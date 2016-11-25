#include "time_utils.h"
#include "player_info.h"
#include "bounty_logic.h"
#include "game_info.h"
#include "tool_base.h"
#include "city_base.h"
#include "globalres_logic.h"
#include "player_base.h"
#include "common_func.h"
#include "common_base.h"
#include "game_svr.h"
#include "quest_logic.h"
#include "sendmessage_base.h"
#include "common_logic.h"
#include "msg_base.h"
#include "conf_base.h"

//数据存储格式
/*
"svr_bounty"：
{
	"next_refresh_time": int//下次更新限时任务的时间
	"goal"://总体奖励信息
	{
		"get_star":	int //获得的总星数
		"s_stage":	[]	//总体奖励星星分布 阶段信息
		"reward"://总体进度奖励信息
		[//阶段
			[	
				[   	 
					type //类型
					id   //id
					num  //数量
				]
			]
		]
	}
	"base"://基础节点信息
	{
		"idx":[//小节点 
			[int,int,int,int]//目标值当前数量 总目标值 完成阶段 条件开始值
			int //目标项type
			int //目标项id
			int //目标值 value
			int //0-拥有型 1-操作型
			[//idx 为小节点阶段
				int //目标值 数量
			]
			[//idx 为小节点阶段 reward
				[[
					int,//type
					int,//id
					int,//num
				]]
			]
		]
	}
}
*/
TVOID CBountyLogic::GenBountyInfo(SUserInfo *pstUser, SCityInfo *pstCity)
{
    if(pstUser == NULL || pstCity == NULL )
    {
        return;
    }

    TINT64 ddwSid = pstUser->m_tbLogin.m_nSid;
    if(ddwSid < 0)
    {
        return;
    }
    TBOOL bOpen = CCommonLogic::CheckFuncOpen(pstUser, pstCity, EN_FUNC_OPEN_TYPE_BOUNTY);
    if(!bOpen)
    {
        return;
    }

    //设置为玩家创号时间
    TUINT32 udwSvrOpenTime = pstUser->m_tbLogin.m_nCtime;

    TUINT32 udwSvrLastTime = CTimeUtils::GetUnixTime() - udwSvrOpenTime;
    TUINT32 udwSvrLastDay = udwSvrLastTime / (3600 * 24);

    const Json::Value& jBounty = CGameInfo::GetInstance()->m_oJsonRoot["game_bounty"];
    Json::Value::Members jMember = jBounty.getMemberNames();
    vector<TUINT32> vConfDay;
    vConfDay.clear();
    for(Json::Value::Members::iterator it = jMember.begin(); it < jMember.end();++it)
    {
        vConfDay.push_back(atoi((*it).c_str()));
    }

    if(vConfDay.empty())
    {
        return;
    }
    sort(vConfDay.begin(),vConfDay.end());

    TUINT32 udwSelectId = 0;
    for(vector<TUINT32>::iterator it = vConfDay.begin(); it < vConfDay.end();++it)
    {
        TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("GenBountyInfo:[uid=%u] svr_last_day=%u conf_day=%u [seq=%u]",
            pstUser->m_tbPlayer.m_nUid, udwSvrLastDay, *it, pstUser->m_udwBSeqNo));
        if(udwSvrLastDay < *it)
        {
            udwSelectId = *it;
            break;
        }
    }

    if(udwSelectId == 0)
    {
        return;
    }

    TbBounty *pstBounty = &pstUser->m_tbBounty;

    if(!jBounty[CCommonFunc::NumToString(udwSelectId)].isMember("o"))
    {
        return;
    }

    TUINT32 udwNextRefreshDay = jBounty[CCommonFunc::NumToString(udwSelectId)]["refresh"].asUInt();
    if(udwSelectId >= 1000)
    {
        TUINT32 udwLeftDay = (udwNextRefreshDay - (udwSvrLastDay % udwNextRefreshDay)) * 24 * 3600 + CTimeUtils::GetUnixTime();
        pstBounty->Set_Next_refresh_time(udwLeftDay);
    }
    else
    {
        TUINT32 udwLeftDay = udwSelectId - udwSvrLastDay;
        TUINT32 udwTmp = udwLeftDay * 24 * 3600 + CTimeUtils::GetUnixTime();
        pstBounty->Set_Next_refresh_time(udwTmp);
    }
    
    const Json::Value &jStarStage = jBounty[CCommonFunc::NumToString(udwSelectId)]["a"];
    pstBounty->Set_S_stage(jStarStage);
    const Json::Value &jSReward = jBounty[CCommonFunc::NumToString(udwSelectId)]["r"];
    pstBounty->Set_S_reward(jSReward);
    const Json::Value &jBaseInfo = jBounty[CCommonFunc::NumToString(udwSelectId)]["o"];
    pstBounty->Set_Star(0);
    

    Json::Value jGenBaseInfo;
    jGenBaseInfo = Json::Value(Json::objectValue);
    for(TUINT32 udwIdx = 0; udwIdx < jBaseInfo.size();++udwIdx)
    {
        TUINT32 udwConType = jBaseInfo[udwIdx][0U].asUInt();
        TUINT32 udwConId = jBaseInfo[udwIdx][1U].asUInt();
        TUINT32 udwConValue = jBaseInfo[udwIdx][2U].asUInt();
        TUINT32 udwConIsStand = jBaseInfo[udwIdx][3U].asUInt();
        TUINT32 udwLastGoalNum = jBaseInfo[udwIdx][4U][jBaseInfo[udwIdx][4U].size() - 1U].asUInt();
        TUINT32 udwBeginNum = 0;
        TUINT32 udwCurNum = 0;
        if(udwConIsStand)
        {
            //do nothing
        }
        else
        {
            udwBeginNum = CQuestLogic::GetOwnNum(pstCity, pstUser, udwConType,udwConId,udwConValue);
            udwCurNum = udwBeginNum;
        }
        //检查国王
        if(udwConType == EN_TASK_TYPE_ING_BEEN_KING )
        {
            //TODO
        }
        Json::Value &jNodwInfo = jGenBaseInfo[CCommonFunc::NumToString(udwIdx)];
        jNodwInfo = Json::Value(Json::arrayValue);

        Json::Value &jStatusInfo = jNodwInfo[0U];
        jStatusInfo = Json::Value(Json::arrayValue);
        jStatusInfo.append(udwCurNum);
        jStatusInfo.append(udwLastGoalNum);

        TUINT32 udwFinish = 0;
        TUINT32 udwFinishStage = 0;
        if(udwConIsStand)
        {
            //操作型 do nothing
        }
        else
        {
            udwFinish = udwBeginNum;
        }
        for(TUINT32 udwFinishStageIdx = 0; udwFinishStageIdx < jBaseInfo[udwIdx][4U].size(); ++udwFinishStageIdx)
        {
            if(udwFinish > jBaseInfo[udwIdx][4U][jBaseInfo[udwIdx][4U].size()-1].asUInt())
            {
                udwFinishStage = jBaseInfo[udwIdx][4U].size();
                break;
            }
            if(udwFinish < jBaseInfo[udwIdx][4U][udwFinishStageIdx].asUInt())
            {
                udwFinishStage = udwFinishStageIdx;
                break;
            }
        }
        jStatusInfo.append(udwFinishStage);
        jStatusInfo.append(udwBeginNum);
        if(udwFinishStage == 0)
        {
            jStatusInfo.append(0);
        }
        else
        {
            TUINT32 udwTmp = udwFinishStage - 1;
            jStatusInfo.append(jBaseInfo[udwIdx][5U][udwTmp].asUInt());
        }
        
        jNodwInfo[1U] = udwConType;
        jNodwInfo[2U] = udwConId;
        jNodwInfo[3U] = udwConValue;
        jNodwInfo[4U] = udwConIsStand;
        jNodwInfo[5U] = jBaseInfo[udwIdx][4U];
        jNodwInfo[6U] = jBaseInfo[udwIdx][6U];
        jNodwInfo[7U] = udwIdx;
        jNodwInfo[8U] = jBaseInfo[udwIdx][5U];

        
        //初始完成 增加奖励
        for(TUINT32 udwStageIdx = 0; udwStageIdx < udwFinishStage; ++udwStageIdx)
        {
            pair<TUINT32, TUINT32> pRewardInfo;
            pRewardInfo.first = udwIdx;
            pRewardInfo.second = udwStageIdx;
            pstUser->m_vTmpFinishBountyBase.push_back(pRewardInfo);


            CBountyLogic::GetBaseNodeReward(pstUser, pstCity, udwIdx, udwStageIdx);
        }
    }
    pstBounty->Set_Base(jGenBaseInfo);
    return;
}


TVOID CBountyLogic::CheckBounty(SUserInfo *pstUser, SCityInfo *pstCity)
{
    CBountyLogic::CheckBountyRefresh(pstUser,pstCity);

    if (pstUser->m_tbBounty.m_nNext_refresh_time <= CTimeUtils::GetUnixTime())
    {
        return;
    }

    CBountyLogic::CheckBountyOwnNumUpdate(pstUser, pstCity);

    CBountyLogic::CheckBaseNodeFinish(pstUser, pstCity);

    CBountyLogic::CheckGoalFinish(pstUser,pstCity);

    return;
}

//au notification
TVOID CBountyLogic::CheckBountyNotic(SUserInfo *pstUser, SCityInfo *pstCity)
{
    if (pstUser->m_tbBounty.m_nNext_refresh_time <= CTimeUtils::GetUnixTime())
    {
        return;
    }
    if (CBountyLogic::CheckBountyOwnNumUpdate(pstUser, pstCity))
    {
        CBountyLogic::CheckBaseNodeFinishNotic(pstUser, pstCity);

        CBountyLogic::CheckGoalFinishNotic(pstUser, pstCity);
    }

    return;
}

TVOID CBountyLogic::SetBountyCurValue(SUserInfo *pstUser, SCityInfo *pstCity, TUINT32 udwCondiType, TUINT32 udwNumAdd/* = 1*/, TUINT32 udwId/* = 0*/, TUINT32 Value/* = 0*/)
{
    TBOOL bOpen = CCommonLogic::CheckFuncOpen(pstUser, pstCity, EN_FUNC_OPEN_TYPE_BOUNTY);
    if(!bOpen)
    {
        return;
    }

    TbBounty *pstBounty = &pstUser->m_tbBounty;

    TBOOL bNeedUpdate = FALSE;
    Json::Value &jBase = pstBounty->m_jBase;
    Json::Value::Members jMember = jBase.getMemberNames();
    for(Json::Value::Members::iterator it = jMember.begin(); it < jMember.end();++it)
    {
        TUINT32 udwConType = jBase[(*it).c_str()][1U].asUInt();
        TUINT32 udwConId = jBase[(*it).c_str()][2U].asUInt();
        TUINT32 udwConValue = jBase[(*it).c_str()][3U].asUInt();
        if(udwConType == 0)
        {
            continue;
        }
        if(udwConType == udwCondiType &&
            udwConId == udwId &&
            udwConValue == Value)
        {
            jBase[(*it).c_str()][0U][0U] = jBase[(*it).c_str()][0U][0U].asUInt() + udwNumAdd;
            bNeedUpdate = TRUE;
        }
    }
    if(bNeedUpdate)
    {
        pstBounty->SetFlag(TbBOUNTY_FIELD_BASE);
    }

    return;
}

TINT32 CBountyLogic::CheckBountyRefresh(SUserInfo *pstUser, SCityInfo *pstCity)
{
    if(pstUser == NULL || pstCity == NULL)
    {
        return 0;
    }
    TbBounty *pstBounty = &pstUser->m_tbBounty;

    if(pstBounty->m_nNext_refresh_time <= CTimeUtils::GetUnixTime())
    {

        CBountyLogic::GenBountyInfo(pstUser, pstCity);
    }
    return 0;
}

TBOOL CBountyLogic::CheckBountyOwnNumUpdate(SUserInfo *pstUser, SCityInfo *pstCity)
{
    TbBounty *pstBounty = &pstUser->m_tbBounty;

    TBOOL bNeedUpdate = FALSE;
    Json::Value &jBase = pstBounty->m_jBase;
    Json::Value::Members jMember = jBase.getMemberNames();
    for(Json::Value::Members::iterator it = jMember.begin(); it < jMember.end(); ++it)
    {
        TUINT32 udwConType = jBase[(*it).c_str()][1U].asUInt();
        if(udwConType == 0)
        {
            continue;
        }

        TUINT32 udwConId = jBase[(*it).c_str()][2U].asUInt();
        TUINT32 udwConValue = jBase[(*it).c_str()][3U].asUInt();
        TUINT64 uddwOldNum = jBase[(*it).c_str()][0U][0U].asUInt64();
        TUINT64 uddwCurNum = CQuestLogic::GetOwnNum(pstCity, pstUser, udwConType, udwConId, udwConValue);

        if(uddwCurNum > uddwOldNum)
        {
            jBase[(*it).c_str()][0U][0U] = uddwCurNum;
            bNeedUpdate = TRUE;
        }
    }
    if(bNeedUpdate)
    {
        pstBounty->SetFlag(TbBOUNTY_FIELD_BASE);
        return TRUE;
    }

    return FALSE;
}

TINT32 CBountyLogic::CheckBaseNodeFinish(SUserInfo *pstUser, SCityInfo *pstCity)
{
    TbBounty *pstBounty = &pstUser->m_tbBounty;

    Json::Value &jBase = pstBounty->m_jBase;
    Json::Value::Members jMember = jBase.getMemberNames();
    for(Json::Value::Members::iterator it = jMember.begin(); it < jMember.end(); ++it)
    {
        TUINT32 udwConType = jBase[(*it).c_str()][1U].asUInt();
        TUINT32 udwConId = jBase[(*it).c_str()][2U].asUInt();
        TUINT32 udwConValue = jBase[(*it).c_str()][3U].asUInt();
        
        if(udwConType == 0)
        {
            continue;
        }
        TUINT64 uddwCurNum = jBase[(*it).c_str()][0U][0U].asUInt64();
        
        TUINT64 udwFinishStage = jBase[(*it).c_str()][0U][2U].asUInt64();//0-3
        if(udwFinishStage >= jBase[(*it).c_str()][5U].size())
        {
            //已完成
            continue;
        }
        for(TUINT32 udwDoneIdx = jBase[(*it).c_str()][0U][2U].asUInt(); udwDoneIdx < jBase[(*it).c_str()][5U].size();++udwDoneIdx)
        {
            TUINT32 udwValue = jBase[(*it).c_str()][5U][udwDoneIdx].asUInt();
            
            //操作型
            TBOOL bFinish = TRUE;
            if(jBase[(*it).c_str()][4U].asUInt())
            {
                if(uddwCurNum - jBase[(*it).c_str()][0U][3U].asUInt64() < udwValue)
                {
                    bFinish = FALSE;
                }
            }
            else
            {
                if(uddwCurNum < udwValue)
                {
                    bFinish = FALSE;
                }
            }
            if(bFinish)
            {
                pair<TUINT32, TUINT32> pRewardInfo;
                pRewardInfo.first = atoi((*it).c_str());
                pRewardInfo.second = udwDoneIdx ;
                pstUser->m_vTmpFinishBountyBase.push_back(pRewardInfo);

                CBountyLogic::GetBaseNodeReward(pstUser, pstCity, atoi((*it).c_str()), udwDoneIdx);
                
                udwFinishStage = udwDoneIdx + 1;
                string sType = CCommonFunc::NumToString(udwConType);
                string sId = CCommonFunc::NumToString(udwConId);
                string sValue = CCommonFunc::NumToString(udwConValue);
                string sNum = CCommonFunc::NumToString(udwValue);

                SNoticInfo stNoticInfo;
                stNoticInfo.Reset();
                stNoticInfo.SetValue(EN_NOTI_ID__BONUTY_FINISH,
                    pstUser->m_tbPlayer.m_sUin, "",
                    0, 0,
                    0, 0,
                    0, "", 0);
                CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), pstUser->m_tbPlayer.m_nSid, pstUser->m_tbPlayer.m_nUid, stNoticInfo);


            }
        }
        if(udwFinishStage != jBase[(*it).c_str()][0U][2U].asUInt64())
        {
            jBase[(*it).c_str()][0U][2U] = udwFinishStage;
            if(udwFinishStage == 0)
            {
                jBase[(*it).c_str()][0U][4U] = 0;
            }
            else
            {
                TUINT32 udwTmp = udwFinishStage - 1U;
                const Json::Value &jSomething = jBase[(*it).c_str()][8U];
                TUINT32 udwValue = jSomething[udwTmp].asUInt();
                jBase[(*it).c_str()][0U][4U] = udwValue;
            }
            pstBounty->SetFlag(TbBOUNTY_FIELD_BASE);
        }
    }
    
    return 0;
}

TINT32 CBountyLogic::CheckGoalFinish(SUserInfo *pstUser, SCityInfo *pstCity)
{
    TbBounty *pstBounty = &pstUser->m_tbBounty;

    TUINT32 udwCurStar = 0;
    TUINT32 udwOldStar = pstBounty->m_nStar;
    if(udwOldStar >= pstBounty->m_jS_stage[pstBounty->m_jS_stage.size() - 1U].asUInt())
    {
        //已经完成了所有goal的收集 do nothing
        return 0;
    }
    const Json::Value &jBase = pstBounty->m_jBase;
    Json::Value::Members jMember = jBase.getMemberNames();
    for(Json::Value::Members::iterator it = jMember.begin(); it < jMember.end(); ++it)
    {
        udwCurStar += jBase[(*it).c_str()][0U][4U].asUInt();
    }

    for(TUINT32 udwIdx = 0; udwIdx < pstBounty->m_jS_stage.size();++udwIdx)
    {
        if(pstBounty->m_jS_stage[udwIdx].asUInt() <= udwOldStar)
        {
            continue;
        }
        if(udwCurStar < pstBounty->m_jS_stage[udwIdx].asUInt())
        {
            break;
        }
        pstUser->m_udwTmpFinishBountyGoal = udwIdx + 1;
        CBountyLogic::GetGoalReward(pstUser, pstCity, udwIdx);

        SNoticInfo stNoticInfo;
        stNoticInfo.Reset();
        stNoticInfo.SetValue(EN_NOTI_ID__BONUTY_FINISH,
            pstUser->m_tbPlayer.m_sUin, "",
            0, 0,
            0, 0,
            0, "", 0);
        CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), pstUser->m_tbPlayer.m_nSid, pstUser->m_tbPlayer.m_nUid, stNoticInfo);


    }
    if(udwCurStar != udwOldStar)
    {
        pstBounty->Set_Star(udwCurStar);
    }
    return 0;
}

TINT32 CBountyLogic::GetBaseNodeReward(SUserInfo *pstUser, SCityInfo *pstCity,TUINT32 udwNodeIdx ,TUINT32 udwStageIdx)
{
    TbBounty *pstBounty = &pstUser->m_tbBounty;

    const Json::Value &jBase = pstBounty->m_jBase;

    const Json::Value &jReward = jBase[CCommonFunc::NumToString(udwNodeIdx)][6U][udwStageIdx];
    
    SSpGlobalRes stGlobalres ;
    stGlobalres.Reset();
    TINT32 dwRetCode = 0;
    dwRetCode = CGlobalResLogic::GetSpGlobalResInfo(jReward, EREWARD_TYPE_ALL, &stGlobalres);
    if(dwRetCode != 0)
    {
        //TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("GetTaskRewardById: get reward info fail [uid=%ld task_id=%u] [ret=%d]", pstUser->m_tbPlayer.m_nUid, udwTaskId, dwRetCode));
        return dwRetCode;
    }

    dwRetCode = CGlobalResLogic::AddSpGlobalRes(pstUser, pstCity, &stGlobalres);
    if(dwRetCode != 0)
    {
        //TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("CProcessQuestClaim: take reward fail [uid=%ld task_id=%u] [ret=%d]", pstUser->m_tbPlayer.m_nUid, udwTaskId, dwRetCode));
        return dwRetCode;
    }
    return dwRetCode;
}

TINT32 CBountyLogic::GetGoalReward(SUserInfo *pstUser, SCityInfo *pstCity, TUINT32 udwStageIdx)
{
    TbBounty *pstBounty = &pstUser->m_tbBounty;

    const Json::Value &jGoal = pstBounty->m_jS_reward;

    const Json::Value &jReward = jGoal[udwStageIdx];

    SSpGlobalRes stGlobalres;
    stGlobalres.Reset();
    TINT32 dwRetCode = 0;
    dwRetCode = CGlobalResLogic::GetSpGlobalResInfo(jReward, EREWARD_TYPE_ALL, &stGlobalres);
    if(dwRetCode != 0)
    {
        //TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("GetTaskRewardById: get reward info fail [uid=%ld task_id=%u] [ret=%d]", pstUser->m_tbPlayer.m_nUid, udwTaskId, dwRetCode));
        return dwRetCode;
    }

    dwRetCode = CGlobalResLogic::AddSpGlobalRes(pstUser, pstCity, &stGlobalres);
    if(dwRetCode != 0)
    {
        //TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("CProcessQuestClaim: take reward fail [uid=%ld task_id=%u] [ret=%d]", pstUser->m_tbPlayer.m_nUid, udwTaskId, dwRetCode));
        return dwRetCode;
    }
    return dwRetCode;
}

TINT32 CBountyLogic::CheckBaseNodeFinishNotic(SUserInfo *pstUser, SCityInfo *pstCity)
{
    TbBounty *pstBounty = &pstUser->m_tbBounty;

    const Json::Value &jBase = pstBounty->m_jBase;
    Json::Value::Members jMember = jBase.getMemberNames();
    for(Json::Value::Members::iterator it = jMember.begin(); it < jMember.end(); ++it)
    {
        TUINT32 udwConType = jBase[(*it).c_str()][1U].asUInt();
        //TUINT32 udwConId = jBase[(*it).c_str()][2U].asUInt();
        //TUINT32 udwConValue = jBase[(*it).c_str()][3U].asUInt();

        if(udwConType == 0)
        {
            continue;
        }
        TUINT64 uddwCurNum = jBase[(*it).c_str()][0U][0U].asUInt64();

        TUINT64 udwFinishStage = jBase[(*it).c_str()][0U][2U].asUInt64();//0-3
        if(udwFinishStage >= jBase[(*it).c_str()][5U].size())
        {
            //已完成
            continue;
        }
        for(TUINT32 udwDoneIdx = jBase[(*it).c_str()][0U][2U].asUInt(); udwDoneIdx < jBase[(*it).c_str()][5U].size(); ++udwDoneIdx)
        {
            TUINT32 udwValue = jBase[(*it).c_str()][5U][udwDoneIdx].asUInt();

            //操作型
            TBOOL bFinish = TRUE;
            if(jBase[(*it).c_str()][4U].asUInt())
            {
                if(uddwCurNum - jBase[(*it).c_str()][0U][3U].asUInt64() < udwValue)
                {
                    bFinish = FALSE;
                }
            }
            else
            {
                if(uddwCurNum < udwValue)
                {
                    bFinish = FALSE;
                }
            }
            if(bFinish)
            {
                SNoticInfo stNoticInfo;
                stNoticInfo.Reset();
                stNoticInfo.SetValue(EN_NOTI_ID__BONUTY_FINISH,
                    pstUser->m_tbPlayer.m_sUin, "",
                    0, 0,
                    0, 0,
                    0, "", 0);
                CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), pstUser->m_tbPlayer.m_nSid, pstUser->m_tbPlayer.m_nUid, stNoticInfo);

            }
        }
    }

    return 0;
}

TINT32 CBountyLogic::CheckGoalFinishNotic(SUserInfo *pstUser, SCityInfo *pstCity)
{
    TbBounty *pstBounty = &pstUser->m_tbBounty;

    TUINT32 udwCurStar = 0;
    TUINT32 udwOldStar = pstBounty->m_nStar;
    if(udwOldStar >= pstBounty->m_jS_stage[pstBounty->m_jS_stage.size() - 1U].asUInt())
    {
        //已经完成了所有goal的收集 do nothing
        return 0;
    }
    const Json::Value &jBase = pstBounty->m_jBase;
    Json::Value::Members jMember = jBase.getMemberNames();
    for(Json::Value::Members::iterator it = jMember.begin(); it < jMember.end(); ++it)
    {
        udwCurStar += jBase[(*it).c_str()][0U][4U].asUInt();
    }

    for(TUINT32 udwIdx = 0; udwIdx < pstBounty->m_jS_stage.size(); ++udwIdx)
    {
        if(pstBounty->m_jS_stage[udwIdx].asUInt() <= udwOldStar)
        {
            continue;
        }
        if(udwCurStar < pstBounty->m_jS_stage[udwIdx].asUInt())
        {
            break;
        }
        
        SNoticInfo stNoticInfo;
        stNoticInfo.Reset();
        stNoticInfo.SetValue(EN_NOTI_ID__BONUTY_FINISH,
            pstUser->m_tbPlayer.m_sUin, "",
            0, 0,
            0, 0,
            0, "", 0);
        CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), pstUser->m_tbPlayer.m_nSid, pstUser->m_tbPlayer.m_nUid, stNoticInfo);

    }
    return 0;
}
