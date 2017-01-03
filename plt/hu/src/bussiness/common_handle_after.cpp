#include "common_handle_after.h"
#include "aws_table_include.h"
#include "game_command.h"
#include "production_system.h"
#include "buffer_base.h"
#include "quest_logic.h"
#include "process_mailreport.h"
#include "msg_base.h"
#include "common_logic.h"
#include "quest_notic_logic.h"
#include "process_action.h"
#include "activities_logic.h"
#include "common_json.h"
#include "sendmessage_base.h"
#include "common_func.h"
#include "bounty_logic.h"
#include "global_serv.h"
#include "common_base.h"
#include "tool_base.h"
#include "player_base.h"
#include "action_base.h"
#include "backpack_logic.h"
#include "map_base.h"
#include "map_logic.h"

// 公共命令字需要设置"pstSession->m_stCommonResInfo.m_dwRetCode"
TINT32 CCommonHandleAfter::Process_CommonHandleAfter(SSession *pstSession)
{
    TINT32 dwRetCode = 0;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;

    //wave@20161130: for bug check
    CQuestLogic::CheckPlayerTimeQuestValid(pstUser, "handle_after_beg");

    CCommonHandleAfter::CorrectExp(pstSession);

    SCityInfo *pstCity = &pstUser->m_stCityInfo;

    pstSession->m_udwCastlelv = CCityBase::GetBuildingLevelByFuncType(pstCity, EN_BUILDING_TYPE__CASTLE);

    TUINT32 udwNum = 0;
    if(pstSession->m_stUserInfo.m_tbPlayer.m_nCur_troop_might != static_cast<TINT64>(CCommonBase::ComputeTotalTroopMight(pstUser, udwNum)))
    {
        pstSession->m_stUserInfo.m_tbPlayer.Set_Cur_troop_might(CCommonBase::ComputeTotalTroopMight(pstUser,udwNum));
    }
    if(pstSession->m_stUserInfo.m_tbPlayer.m_nCur_fort_might != static_cast<TINT64>(CCommonBase::ComputeFortMight(pstUser,udwNum)))
    {
        pstSession->m_stUserInfo.m_tbPlayer.Set_Cur_fort_might(CCommonBase::ComputeFortMight(pstUser,udwNum));
    }
    if(pstSession->m_stUserInfo.m_tbPlayer.m_nBuilding_force != CBufferBase::ComputeBuildingForce(pstCity,pstUser))
    {
        pstSession->m_stUserInfo.m_tbPlayer.Set_Building_force(CBufferBase::ComputeBuildingForce(pstCity, pstUser));
    }
    if(pstSession->m_stUserInfo.m_tbPlayer.m_nResearch_force != CBufferBase::ComputeResearchForce(pstCity, pstUser))
    {
        pstSession->m_stUserInfo.m_tbPlayer.Set_Research_force(CBufferBase::ComputeResearchForce(pstCity, pstUser));
    }
    if (pstSession->m_stUserInfo.m_tbPlayer.m_nDragon_force != CBufferBase::ComputeDragonLvForce(pstCity, pstUser))
    {
        pstSession->m_stUserInfo.m_tbPlayer.Set_Dragon_force(CBufferBase::ComputeDragonLvForce(pstCity, pstUser));
    }
    if (pstSession->m_stUserInfo.m_tbPlayer.m_nLord_force != CBufferBase::ComputeLordLvForce(pstCity, pstUser))
    {
        pstSession->m_stUserInfo.m_tbPlayer.Set_Lord_force(CBufferBase::ComputeLordLvForce(pstCity, pstUser));
    }

    // player有更新時就检测一下selfname----这里还没有更新utime
    if(pstSession->m_stUserInfo.m_tbPlayer.m_mFlag.size() > 0)
    {
        Json::Value tmpProfile;
        pstSession->m_stUserInfo.m_tbSelfName.Set_Type(EN_PLAYER_NAME);
        pstSession->m_stUserInfo.m_tbSelfName.Set_Name(CToolBase::ToLower(pstUser->m_tbPlayer.m_sUin));
        pstSession->m_stUserInfo.m_tbSelfName.Set_Id(pstSession->m_stUserInfo.m_tbPlayer.m_nUid);
        pstSession->m_stUserInfo.m_tbSelfName.Set_Exist(1);
        CCommJson::GenUniqueNameInfo(&pstSession->m_stUserInfo.m_tbPlayer, tmpProfile);
        if(tmpProfile != pstSession->m_stUserInfo.m_tbSelfName.m_jProfile)
        {
            pstSession->m_stUserInfo.m_tbSelfName.Set_Profile(tmpProfile);
        }
    }

    // 客户端新手教学直接触发
    CCommonHandleAfter::HandleGuideFinishStage(pstSession);

    //计算玩家的bonous
    pstSession->m_stUserInfo.m_stPlayerBuffList.Reset();
    CBufferBase::ComputeBuffInfo(pstCity, &pstSession->m_stUserInfo, pstSession->m_atbIdol, pstSession->m_udwIdolNum, &pstSession->m_tbThrone, &pstSession->m_stTitleInfoList);

    dwRetCode = CCommonLogic::ComputeCanHelpAlAction(&pstSession->m_stUserInfo);
    if(dwRetCode < 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("common_handle_after step 0 error [retcode=%d] [seq=%u]",
            dwRetCode,
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("common_handle_after step 0 [seq=%u]",
        pstSession->m_stUserInfo.m_udwBSeqNo));
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // 客户端每次主动拉数据时,更新player表的utime和该服的连续登录天数
    dwRetCode = UpdatePlayerUtimeAndLoginDay(pstSession);
    if(dwRetCode < 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("common_handle_after step 1 error [retcode=%d] [seq=%u]",
            dwRetCode,
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("common_handle_after step 1 [seq=%u]",
        pstSession->m_stUserInfo.m_udwBSeqNo));
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // 客户端每次主动拉数据时,更新玩家的vip信息
    dwRetCode = UpdatePlayerVipInfo(pstSession);
    if(dwRetCode < 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("common_handle_after step 2 error [retcode=%d] [seq=%u]",
            dwRetCode,
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -2;
    }
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("common_handle_after step 2 [seq=%u]",
        pstSession->m_stUserInfo.m_udwBSeqNo));

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // 军队的数量监控
    dwRetCode = TroopMonitor(pstSession);
    if(dwRetCode < 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("common_handle_after step 4 error [retcode=%d] [seq=%u]",
            dwRetCode,
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -4;
    }
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("common_handle_after step 4 [seq=%u]",
        pstSession->m_stUserInfo.m_udwBSeqNo));
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // city信息的计算
    dwRetCode = UpdateCityInfo(pstSession);
    if(dwRetCode < 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("common_handle_after step 5 error [retcode=%d] [seq=%u]",
            dwRetCode,
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -5;
    }
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("common_handle_after step 5 [seq=%u]",
        pstSession->m_stUserInfo.m_udwBSeqNo));
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // 计算用户的might值
    dwRetCode = UpdateUserMight(pstSession);
    if(dwRetCode < 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("common_handle_after step 6 error [retcode=%d] [seq=%u]",
            dwRetCode,
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -6;
    }
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("common_handle_after step 6 [seq=%u]",
        pstSession->m_stUserInfo.m_udwBSeqNo));
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // 判定是否要提示用户新手保护已经结束
    dwRetCode = CheckPlayerNewProtectionStatus(pstSession);
    if(dwRetCode < 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("common_handle_after step 8 error [retcode=%d] [seq=%u]",
            dwRetCode,
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -8;
    }
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("common_handle_after step 8 [seq=%u]",
        pstSession->m_stUserInfo.m_udwBSeqNo));
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // 判定是否对玩家首次加入联盟并成功进行奖励
    dwRetCode = CheckPlayerFirstInAllianceReward(pstSession);
    if(dwRetCode < 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("common_handle_after step 9 error [retcode=%d] [seq=%u]",
            dwRetCode,
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -9;
    }
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("common_handle_after step 9 [seq=%u]",
        pstSession->m_stUserInfo.m_udwBSeqNo));
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // 是否需要发鼓励邮件
    CProcessMailReport::AutoSendEncourageMail(pstSession);

    TSE_LOG_DEBUG(pstSession->m_poServLog, ("common_handle_after step 11 [seq=%u]",
        pstSession->m_stUserInfo.m_udwBSeqNo));
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // 监控用户数据
    dwRetCode = MonitorUserData(pstSession);
    if(dwRetCode < 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("common_handle_after step 13 error [retcode=%d] [seq=%u]",
            dwRetCode,
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -13;
    }
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("common_handle_after step 13 [seq=%u]",
        pstSession->m_stUserInfo.m_udwBSeqNo));
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // player信息的计算
    dwRetCode = UpdatePlayerInfo(pstSession);
    if (dwRetCode < 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("common_handle_after step 14 error [retcode=%d] [seq=%u]",
            dwRetCode,
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -14;
    }
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("common_handle_after step 14 [seq=%u]",
        pstSession->m_stUserInfo.m_udwBSeqNo));
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // 同步用户信息到地图
    dwRetCode = UpdateUserInfoToMap(pstSession);
    if(dwRetCode < 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("common_handle_after step 14 error [retcode=%d] [seq=%u]",
            dwRetCode,
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -14;
    }
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("common_handle_after step 14 [seq=%u]",
        pstSession->m_stUserInfo.m_udwBSeqNo));

    dwRetCode = SyncInfoToMarch(pstSession);

    // 记录玩家的语言,方便au做本地化推送
    pstSession->m_stUserInfo.m_tbLogin.Set_Lang(pstSession->m_stReqParam.m_udwLang);

    CCommonHandleAfter::UpdateNotiTimer(pstSession);
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("common_handle_after step 16 [seq=%u]",
        pstSession->m_stUserInfo.m_udwBSeqNo));

    CCommonHandleAfter::DelNotiTimer(pstSession);

    //弹窗  舍去
//     if(pstSession->m_stUserInfo.m_stRewardWindow.udwTotalNum != 0)
//     {
//         TINT32 dwRewardWinType = CCommonBase::GetRewardBoxUiType(&pstSession->m_stUserInfo.m_stRewardWindow);
//         if(dwRewardWinType == -1)
//         {
//             pstSession->m_stUserInfo.m_stRewardWindow.Reset();
//             pstSession->m_stUserInfo.udwRewardWinType = 0;
//             pstSession->m_stUserInfo.udwRewardWinGetType = 0;
//         }
//         else
//         {
//             pstSession->m_stUserInfo.udwRewardWinType = (TUINT32)dwRewardWinType;
//         }
//     }

    //检查任务是否完成
    CQuestLogic::CheckTimeQuestFinish(&pstSession->m_stUserInfo,pstCity, &pstSession->m_stUserInfo.m_tbQuest);
    //检查任务是否可刷新
    // CQuestLogic::CheckTimeQuestRefresh(&pstSession->m_stUserInfo, pstCity, &pstSession->m_stUserInfo.m_tbQuest);
    CQuestLogic::NewCheckTimeQuestRefresh(&pstSession->m_stUserInfo, pstCity, &pstSession->m_stUserInfo.m_tbQuest);

    //check top quest to claim
    CQuestLogic::CheckIsClaim(&pstSession->m_stUserInfo, pstCity);

    //task count
    CQuestLogic::SetTaskCurValue(pstUser,pstCity, EN_TASK_TYPE_USE_GEM_NUM, pstSession->m_udwGemCost);

    CQuestLogic::ProcessSpTask(&pstSession->m_stUserInfo, pstCity);

    //task
    CQuestLogic::CheckTask(&pstSession->m_stUserInfo, pstCity, FALSE);

    CClientCmd *poCClientCmd = CClientCmd::GetInstance();
    struct SFunctionSet stProcessFunctionSet;
    map<EClientReqCommand, struct SFunctionSet>::iterator itCmdFunctionSet;
    itCmdFunctionSet = poCClientCmd->Get_CmdMap()->find((EClientReqCommand)pstSession->m_stReqParam.m_udwCommandID);
    if(itCmdFunctionSet != poCClientCmd->Get_CmdMap()->end())
    {
        stProcessFunctionSet = itCmdFunctionSet->second;
        if(stProcessFunctionSet.dwCmdType == EN_OP)
        {
            //do nothing
        }
        else
        {
            CBountyLogic::CheckBounty(&pstSession->m_stUserInfo, pstCity);
        }
    }

    //更新玩家活力值
    if (0 == CPlayerBase::CheckUpdtDragonEnergy(pstUser))
    {
        CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPE__DRAGON_ENERGY_OK, pstUser->m_tbPlayer.m_nUid, FALSE);
    }

    CCommonHandleAfter::DelEmptytBuildingPos(pstSession);

    //玩家在线 删除移城action
    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwActionNum;++udwIdx)
    {
        TbAction *pstAction = &pstUser->m_atbAction[udwIdx];
        if(pstAction->m_nMclass == EN_ACTION_MAIN_TASK_ATTACK_MOVE && 
            pstAction->m_nSclass == EN_ACTION_SEC_CLASS__ATTACK_MOVE
            && pstUser->m_aucActionFlag[udwIdx] != EN_TABLE_UPDT_FLAG__NEW)
        {
            if (EN_OP == CClientCmd::GetCmdType(pstSession->m_stReqParam.m_udwCommandID)
                || pstAction->m_bParam[0].m_stAttackMove.m_ddwSpecailMoveFlag != 0)
            {
                //do nothing
            }
            else
            {
                pstUser->m_aucActionFlag[udwIdx] = EN_TABLE_UPDT_FLAG__DEL;
            }
        }
    }
    //被动移城标记
    if(pstUser->m_tbUserStat.m_nRemove_flag)
    {
        if (pstUser->m_tbUserStat.m_nRemove_flag == 1 
            && pstSession->m_stReqParam.m_ucLoginStatus == EN_LOGIN_STATUS__LOGIN)
        {
            pstUser->m_dwShowRemoveFlag = 1;
        }
        else if (pstUser->m_tbUserStat.m_nRemove_flag == 2)
        {
            pstUser->m_dwShowRemoveFlag = 2;
        }
        pstUser->m_tbUserStat.Set_Remove_flag(0);
    }

    // 更新用户的al_gift
    dwRetCode = ComputeAllianceGift(pstSession);
    if(dwRetCode < 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("common_handle_after ComputeAllianceGift error [retcode=%d] [seq=%u]", \
            dwRetCode, \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -16;
    }

    //检测英雄是否升级 要在hero exp变动之后更新
    dwRetCode = CompareLordLevel(pstSession);
    if(dwRetCode < 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("common_handle_after CompareHeroLevel error [retcode=%d] [seq=%u]", \
            dwRetCode, \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -17;
    }

    //v1.2_wave@2015.07.27: 务必放在最后更新player的utime
    if(pstSession->m_stUserInfo.m_bUpdtPlayerUtime == TRUE)
    {
        if(pstSession->m_stUserInfo.m_tbPlayer.m_nUid)
        {
            pstSession->m_stUserInfo.m_tbPlayer.Set_Utime(CTimeUtils::GetUnixTime());
        }
    }

    //更新utime之后需要更新——判断是否更新almember相关信息
    //同步玩家联盟昵称
    if(pstSession->m_stUserInfo.m_tbPlayer.m_nAlid != 0
        && pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos > EN_ALLIANCE_POS__REQUEST)
    {
        if(pstSession->m_stUserInfo.m_tbPlayer.m_sAl_nick_name !=
            pstSession->m_stUserInfo.m_tbAlliance.m_sAl_nick_name)
        {
            pstSession->m_stUserInfo.m_tbPlayer.Set_Al_nick_name(pstSession->m_stUserInfo.m_tbAlliance.m_sAl_nick_name);
            pstSession->m_stUserInfo.m_tbPlayer.Set_Alname_update_time(CTimeUtils::GetUnixTime());
        }
        if(pstSession->m_stUserInfo.m_tbPlayer.m_sAlname !=
            pstSession->m_stUserInfo.m_tbAlliance.m_sName)
        {
            pstSession->m_stUserInfo.m_tbPlayer.Set_Alname(pstSession->m_stUserInfo.m_tbAlliance.m_sName);
            pstSession->m_stUserInfo.m_tbPlayer.Set_Alname_update_time(CTimeUtils::GetUnixTime());
        }

        bool need_updt_almember = false;
        if(CTimeUtils::GetUnixTime() - pstSession->m_stUserInfo.m_tbSelfAlmember.m_jProfile["utime"].asUInt() > 120) //2分鬃更新一次
        {
            need_updt_almember = true;
        }
        if(pstSession->m_stUserInfo.m_tbPlayer.m_mFlag.size() > 0 || need_updt_almember)//只更新utime 则不更新
        {
            Json::Value tmpProfile;
            CCommJson::GenAlMemberInfo(&pstSession->m_stUserInfo.m_tbPlayer, tmpProfile);
            if(tmpProfile != pstSession->m_stUserInfo.m_tbSelfAlmember.m_jProfile)
            {
                pstSession->m_stUserInfo.m_tbSelfAlmember.Set_Aid(pstSession->m_stUserInfo.m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET);
                pstSession->m_stUserInfo.m_tbSelfAlmember.Set_Uid(pstSession->m_stUserInfo.m_tbPlayer.m_nUid);
                pstSession->m_stUserInfo.m_tbSelfAlmember.Set_Al_pos(pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos);
                pstSession->m_stUserInfo.m_tbSelfAlmember.Set_Profile(tmpProfile);
                pstSession->m_stUserInfo.m_tbSelfAlmember.Set_Profile_update_time(CTimeUtils::GetUnixTime());
            }
        }

        if (pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos == EN_ALLIANCE_POS__CHANCELLOR)
        {
            if (pstSession->m_stUserInfo.m_tbPlayer.m_nCid != pstSession->m_stUserInfo.m_tbAlliance.m_nOwner_cid)
            {
                pstSession->m_stUserInfo.m_tbAlliance.Set_Owner_cid(pstSession->m_stUserInfo.m_tbPlayer.m_nCid);
            }
        }

        // 更新联盟的购买信息
        if (pstSession->m_stUserInfo.m_tbLogin.m_nGem_buy > pstSession->m_stUserInfo.m_tbAlliance.m_nMax_gem_buy)
        {
            pstSession->m_stUserInfo.m_tbAlliance.Set_Max_gem_buy(pstSession->m_stUserInfo.m_tbLogin.m_nGem_buy);
        }
    }
    else
    {
        pstSession->m_stUserInfo.m_tbPlayer.Set_Al_nick_name("");
        pstSession->m_stUserInfo.m_tbPlayer.Set_Alname("");
        pstSession->m_stUserInfo.m_tbPlayer.Set_Alpos(EN_ALLIANCE_POS__REQUEST);
        pstSession->m_stUserInfo.m_tbPlayer.Set_Alname_update_time(CTimeUtils::GetUnixTime());
    }

    if (pstCity && CCityBase::GetBuildingLevelById(&pstCity->m_stTblData, 29) > 0)
    {
        pstSession->m_stSelfCityInfo.dwSid = pstUser->m_tbPlayer.m_nSid;
        pstSession->m_stSelfCityInfo.dwUid = pstUser->m_tbPlayer.m_nUid;
        pstSession->m_stSelfCityInfo.dwX = pstCity->m_stTblData.m_nPos / MAP_X_Y_POS_COMPUTE_OFFSET;
        pstSession->m_stSelfCityInfo.dwY = pstCity->m_stTblData.m_nPos % MAP_X_Y_POS_COMPUTE_OFFSET;
        pstSession->m_stSelfCityInfo.sCityName = pstCity->m_stTblData.m_sName;
        pstSession->m_stSelfCityInfo.udwUTime = CTimeUtils::GetUnixTime();
    }

    //更新alliance星级
    if(pstSession->m_stUserInfo.m_tbAlliance.m_nAid > 0 && pstSession->m_stUserInfo.m_tbAlliance.m_nAl_star <= 0)
    {
        pstSession->m_stUserInfo.m_tbAlliance.Set_Al_star(1);
    }

    //检查并更新骑士信息
    CPlayerBase::CheckAndUpdtKnightInfo(pstUser);

    CheckDragon(pstSession);

    CheckSvrAlRecord(pstSession);

    CheckTaxAction(pstSession);

    UpdateLastMight(pstSession);

    CheckTitle(pstSession);

    CheckPeaceTime(pstSession);

    GenEventInfo(pstSession);

    CheckRewardWindow(pstSession);

    GenEquipInfo(pstSession);

    // 特定情况下的不产生tips提示
    CleanUpTipsNum(pstSession);

    CheckUserWild(pstSession);

    UpdateEventTipsStat(pstSession);

    UpdateSelfTitle(pstSession);

    UpdateThroneInfo(pstSession);

    UpdateBroadcastStat(pstSession);

    CheckRallyReinforce(pstSession);

    CancelPeaceTimeWhenBecomeKing(pstSession);


    //wave@20161130: for bug check
    CQuestLogic::CheckPlayerTimeQuestValid(pstUser, "handle_after_end");

    return 0;
}

/**************************************************private*********************************************/

// function  ===> 客户端每次主动拉数据时,更新player表的utime和con_login_day
TINT32 CCommonHandleAfter::UpdatePlayerUtimeAndLoginDay(SSession *pstSession)
{
    if(CToolBase::IsOpCommand(pstSession->m_stReqParam.m_szCommand))
    {
        return 0;
    }

    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    SUserInfo *pstUser = &pstSession->m_stUserInfo;

    // 更新player的utime
    // wave@20140409：在日期切换时，也应该更新utime，防止因为日期变化不更新发生的vip point计算错误
    TUINT32 udwCurDay = udwCurTime / (24 * 60 * 60);
    TUINT32 udwPlayerUtimeDay = pstUser->m_tbPlayer.m_nUtime / (24 * 60 * 60);
    if (udwCurTime - pstUser->m_tbPlayer.m_nUtime > 60
        || udwCurDay != udwPlayerUtimeDay)//leyton modify,降低utime的更新频率;
    {
        pstUser->m_bUpdtPlayerUtime = TRUE;
        if(udwCurDay == udwPlayerUtimeDay + 1)
        {
            pstUser->m_tbUserStat.Set_Con_login_days(pstUser->m_tbUserStat.m_nCon_login_days + 1);
            pstUser->m_bTodayFirstLogin = TRUE;
        }
        else if(udwCurDay == udwPlayerUtimeDay)
        {
            pstUser->m_bTodayFirstLogin = FALSE;
        }
        else
        {
            pstUser->m_tbUserStat.Set_Con_login_days(1);
            pstUser->m_bTodayFirstLogin = TRUE;
        }
    }

    return 0;

}

// function  ===> 客户端每次主动拉数据时,更新玩家的vip信息,包括每日登录的奖励
TINT32 CCommonHandleAfter::UpdatePlayerVipInfo(SSession *pstSession)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    if (NULL == pstCity)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("GetCityInfo:req_cid[%u] failed, and player has no city [seq=%u]", 
            pstSession->m_stReqParam.m_udwCityId, 
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }

    if(pstUser->m_bTodayFirstLogin)
    {
        TINT64 ddwPointAdd = CCommonBase::GetConLoginVipPoint(pstUser->m_tbUserStat.m_nCon_login_days);
        CCommonBase::AddVipPoint(pstUser, pstCity, ddwPointAdd);
    }
    if (pstUser->m_tbPlayer.m_nVip_stage == 0)
    {
        pstUser->m_tbPlayer.Set_Vip_stage(1);
    }
    TINT32 dwRawVipLevel = pstUser->m_dwRawVipLevel;
    TINT32 dwNowVipLevel = CPlayerBase::GetRawVipLevel(&pstUser->m_tbPlayer, pstUser->m_tbPlayer.m_nVip_point);
    TINT32 dwLevelUp = dwNowVipLevel - dwRawVipLevel;
    if (dwLevelUp > 0)
    {
        CCommonBase::AddVipTime(pstUser, pstCity, CCommonBase::GetVipLevelUpRewardTime(dwLevelUp));
        CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPY__VIP_LEVEL_UP, pstUser->m_tbPlayer.m_nUid, FALSE, dwNowVipLevel, 0, 0);
    }
    else
    {
        // 更新player的vip info
        CCommonBase::UpdateVipInfo(pstUser, pstCity);
    }

    return 0;
}

// function  ===> 军队的数量监控
TINT32 CCommonHandleAfter::TroopMonitor(SSession *pstSession)
{
    SCityInfo *pstCity = &pstSession->m_stUserInfo.m_stCityInfo;
    if (NULL == pstCity)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("GetCityInfo:req_cid[%u] failed, and player has no city [seq=%u]", 
            pstSession->m_stReqParam.m_udwCityId, 
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }

    if (!pstSession->m_bGetTroop)
    {
        pstSession->m_ddwTroopBegin = CCommonBase::GetTotalMight(&pstSession->m_stUserInfo, TRUE);
        pstSession->m_bGetTroop = TRUE;
    }

    return 0;
}

// function  ===> city信息的计算
TINT32 CCommonHandleAfter::UpdateCityInfo(SSession *pstSession)
{
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    if (NULL == pstCity)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("GetCityInfo:req_cid[%u] failed, and player has no city [seq=%u]", 
            pstSession->m_stReqParam.m_udwCityId, 
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }

    CProductionSystem::ComputeProductionSystem(pstUser, pstCity, udwCurTime);

    return 0;
}

// function  ===> player信息的计算
TINT32 CCommonHandleAfter::UpdatePlayerInfo(SSession *pstSession)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;
    if (NULL == ptbPlayer)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("UpdatePlayerInfo:req_uid[%u] failed, and player has no player [seq=%u]",
            pstSession->m_stReqParam.m_udwUserId, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }

    SCommonTroop stReinforceTroop;
    CCommonBase::GetReinforceTroop(pstUser, pstCity, stReinforceTroop);

    ptbPlayer->m_bReinforce_limit[0].udwMarchNum = CCommonBase::GetReinforcedMarchNum(pstUser, pstCity);
    ptbPlayer->m_bReinforce_limit[0].udwMarchLimit = CCommonBase::GetReinforceMarchLimit(pstUser);
    ptbPlayer->m_bReinforce_limit[0].udwTroopNum = CToolBase::GetTroopSumNum(stReinforceTroop);
    ptbPlayer->m_bReinforce_limit[0].udwTroopLimit = CCommonBase::GetReinforceTroopLimit(pstUser);
    ptbPlayer->m_bReinforce_limit[0].ddwTroopForce = CToolBase::GetTroopSumForce(stReinforceTroop);

    ptbPlayer->SetFlag(TbPLAYER_FIELD_REINFORCE_LIMIT);

    return 0;
}

// function  ===> 计算用户的might值
TINT32 CCommonHandleAfter::UpdateUserMight(SSession *pstSession)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;

    // 计算用户的might值
    CCommonBase::ComputeUserMight(pstUser);

    if(pstUser->m_tbPlayer.m_nMkill != pstUser->m_tbPlayer.m_nKfort + pstUser->m_tbPlayer.m_nKtroop)
    {
        pstUser->m_tbPlayer.Set_Mkill(pstUser->m_tbPlayer.m_nKfort + pstUser->m_tbPlayer.m_nKtroop);
    }

    return 0;
}

// function  ===> 新手教学期间的资源保护
TINT32 CCommonHandleAfter::NewPlayerResourceProtection(SSession *pstSession)
{

    SUserInfo *pstUser = &pstSession->m_stUserInfo;

    CCommonBase::ProtectGuideResource(pstUser);

    return 0;
}

// function  ===> 判定是否要提示用户新手保护已经结束
TINT32 CCommonHandleAfter::CheckPlayerNewProtectionStatus(SSession *pstSession)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbPlayer& tbPlayer = pstSession->m_stUserInfo.m_tbPlayer;

    if (tbPlayer.m_nStatus & EN_CITY_STATUS__NEW_PROTECTION_CANCEL)
    {
        tbPlayer.Set_Status(tbPlayer.m_nStatus - EN_CITY_STATUS__NEW_PROTECTION_CANCEL);
        pstUser->m_bNewBreakFlag = true;
    }
    return 0;
}

// function  ===> 判定是否对玩家首次加入联盟并成功进行奖励
TINT32 CCommonHandleAfter::CheckPlayerFirstInAllianceReward(SSession *pstSession)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbPlayer& tbPlayer = pstSession->m_stUserInfo.m_tbPlayer;
    TbLogin& tbLogin = pstSession->m_stUserInfo.m_tbLogin;

    if(tbPlayer.m_nJoin_alliance == 1)
    {
        tbPlayer.Set_Join_alliance(0);
        pstUser->m_tbUserStat.Set_Wall_get_t(CTimeUtils::GetUnixTime());
        if( !(tbPlayer.m_nAlid > 0 && tbPlayer.m_nAlpos > EN_ALLIANCE_POS__REQUEST) )
        {
            return 0;
        }
        if(tbLogin.m_nAl_time == 0)
        {
            TUINT32 udwGemReward = CGameInfo::GetInstance()->m_oJsonRoot["game_basic"][EN_GAME_BASIC_FIRST_JOIN_ALLIANCE_GEM_REWARD].asUInt();
            CPlayerBase::AddGem(&pstSession->m_stUserInfo.m_tbLogin, udwGemReward);
            tbPlayer.Set_Status(tbPlayer.m_nStatus | EN_CITY_STATUS__FIRST_ALLIANCE_REWARD);
            pstUser->m_bFirstJoinAlReward = TRUE;
            TSE_LOG_INFO(CGameInfo::GetInstance()->m_poLog, ("CheckPlayerFirstInAllianceReward: uid[%ld]first join alliance, reward gem[%u][seq=%u]", tbPlayer.m_nUid, udwGemReward, pstSession->m_udwSeqNo));
        }
        else
        {
            CMsgBase::SendOperateMail(tbPlayer.m_nUid, EN_MAIL_ID__JOINED_AN_ALLIANCE, tbPlayer.m_nSid,
                SYSTEM_ENCOURAGE, tbPlayer.m_sAlname.c_str(), "", "");
        }
    }

    return 0;
}

// function  ===> 特定情况下的不产生tips提示
TINT32 CCommonHandleAfter::CleanUpTipsNum(SSession *pstSession)
{
    if (EN_CLIENT_REQ_COMMAND__LOGIN_CREATE == pstSession->m_udwCommand
        || EN_CLIENT_REQ_COMMAND__USER_INFO_CREATE == pstSession->m_udwCommand
        || EN_CLIENT_REQ_COMMAND__GUIDE_FINISH_STAGE == pstSession->m_stReqParam.m_udwCommandID
        || EN_CLIENT_REQ_COMMAND__GUIDE_FINISH == pstSession->m_stReqParam.m_udwCommandID)
    {
        pstSession->m_stUserInfo.m_udwTipsNum = 0;
    }
    return 0;
}

// function  ===> 监控用户数据
TINT32 CCommonHandleAfter::MonitorUserData(SSession *pstSession)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;

    CCommonBase::PeriodProcMonitorUserInfo(pstUser);

    return 0;
}

// function  ===> 同步用户信息到地图
TINT32 CCommonHandleAfter::UpdateUserInfoToMap(SSession *pstSession)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbPlayer* ptbPlayer = &pstUser->m_tbPlayer;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    if(NULL == pstCity)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("GetCityInfo:req_cid[%u] failed, and player has no city [seq=%u]", 
            pstSession->m_stReqParam.m_udwCityId, 
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }

    Json::FastWriter writer;
    Json::Value& jsonCityInfo = pstSession->m_jCityInfo;
    jsonCityInfo.clear();
    jsonCityInfo = Json::Value(Json::objectValue);
    //"city_info":
    //{
    //    "uid": long,
    //    "alnick" : string,
    //    "uname" : string,
    //    "dragon" : [string, int, int], //英雄名字,等级,图标  //等级为0表示没有英雄
    //    "troop_num" : long,
    //    "troop_force" : long,
    //    "troop" : [long.....] //下标为troop id，值表示具体数量
    //}
    jsonCityInfo["uid"] = ptbPlayer->m_nUid;
    jsonCityInfo["alnick"] = ptbPlayer->m_sAl_nick_name;
    jsonCityInfo["uname"] = ptbPlayer->m_sUin;
    jsonCityInfo["dragon"] = Json::Value(Json::arrayValue);
    jsonCityInfo["dragon"].append(ptbPlayer->m_sDragon_name);
    if (ptbPlayer->m_nDragon_status == EN_DRAGON_STATUS_NORMAL)
    {
        jsonCityInfo["dragon"].append(ptbPlayer->m_nDragon_level);
    }
    else
    {
        jsonCityInfo["dragon"].append(0);
    }
    jsonCityInfo["dragon"].append(ptbPlayer->m_nDragon_avatar);

    SCommonTroop stTroopInCity;
    CCommonBase::GetInCityTroop(pstUser, pstCity, stTroopInCity);

    jsonCityInfo["troop_num"] = CToolBase::GetTroopSumNum(stTroopInCity);
    jsonCityInfo["troop_force"] = CToolBase::GetTroopSumForce(stTroopInCity);
    CCommJson::GenTroopJson(&stTroopInCity, jsonCityInfo["troop"]);

    jsonCityInfo["reinforced_num"] = ptbPlayer->m_bReinforce_limit[0].udwMarchNum;
    jsonCityInfo["reinforced_limit"] = ptbPlayer->m_bReinforce_limit[0].udwMarchLimit;
    jsonCityInfo["reinforced_troop_num"] = ptbPlayer->m_bReinforce_limit[0].udwTroopNum;
    jsonCityInfo["reinforced_troop_limit"] = ptbPlayer->m_bReinforce_limit[0].udwTroopLimit;
    jsonCityInfo["reinforced_troop_force"] = ptbPlayer->m_bReinforce_limit[0].ddwTroopForce;

    jsonCityInfo["knight"] = Json::Value(Json::arrayValue);
    jsonCityInfo["knight"].append(-1);
    jsonCityInfo["knight"].append(0);
    for (TUINT32 udwIdx = 0; udwIdx < pstCity->m_stTblData.m_bKnight.m_udwNum; udwIdx++)
    {
        if (pstCity->m_stTblData.m_bKnight[udwIdx].ddwPos == EN_KNIGHT_POS__TROOP)
        {
            jsonCityInfo["knight"][0U] = udwIdx;
            jsonCityInfo["knight"][1U] = CGameInfo::GetInstance()->ComputeKnightLevelByExp(pstCity->m_stTblData.m_bKnight[udwIdx].ddwExp);
            break;
        }
    }

    TBOOL bPrisonFlag = FALSE;
    if (pstUser->m_tbPlayer.m_nStatus & EN_CITY_STATUS__AVOID_WAR
        || pstUser->m_tbPlayer.m_nStatus & EN_CITY_STATUS__NEW_PROTECTION)
    {
        //do nothing
    }
    else
    {
        for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwPassiveMarchNum; ++udwIdx)
        {
            if (pstUser->m_aucPassiveMarchFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
            {
                continue;
            }

            if (pstUser->m_atbPassiveMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__PRISON_TIMER
                && pstUser->m_atbPassiveMarch[udwIdx].m_nStatus == EN_MARCH_STATUS__DEFENDING)
            {
                bPrisonFlag = TRUE;
                break;
            }
        }
    }
    
    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwWildNum; ++udwIdx)
    {
        TbMap* ptbMap = &pstUser->m_atbWild[udwIdx];
        if(EN_WILD_TYPE__CITY != ptbMap->m_nType)
        {
            continue;
        }
        if(ptbMap->m_nUid != pstUser->m_tbPlayer.m_nUid)
        {
            continue;
        }
        if(pstUser->m_tbPlayer.m_nStatus & EN_CITY_STATUS__BEEN_ATTACK &&
            ptbMap->Get_Smoke_end_time() < CTimeUtils::GetUnixTime())
        {
            //只有中心坐标记录了状态时间
            pstUser->m_tbPlayer.Set_Status(pstUser->m_tbPlayer.m_nStatus & (~EN_CITY_STATUS__BEEN_ATTACK));
        }
        if(pstUser->m_tbPlayer.m_nStatus & EN_CITY_STATUS__BEEN_REMOVE &&
            ptbMap->Get_Burn_end_time() < CTimeUtils::GetUnixTime())
        {
            //只有中心坐标记录了状态时间
            pstUser->m_tbPlayer.Set_Status(pstUser->m_tbPlayer.m_nStatus & (~EN_CITY_STATUS__BEEN_REMOVE));
        }
        if(ptbMap->m_nMove_city == 1 &&
            EN_OP != CClientCmd::GetCmdType(pstSession->m_stReqParam.m_udwCommandID))
        {
            ptbMap->Set_Move_city(0);
        }
        if(ptbMap->m_sUname != ptbPlayer->m_sUin)
        {
            ptbMap->Set_Uname(ptbPlayer->m_sUin);
            ptbMap->Set_Name_update_time(CTimeUtils::GetUnixTime());
        }
        if(ptbMap->m_sAlname != ptbPlayer->m_sAlname)
        {
            if(ptbPlayer->m_nAlid != 0 && ptbPlayer->m_nAlpos != EN_ALLIANCE_POS__REQUEST)
            {
                ptbMap->Set_Alname(ptbPlayer->m_sAlname);
            }
            else
            {
                ptbMap->Set_Alname("");
            }
            ptbMap->Set_Name_update_time(CTimeUtils::GetUnixTime());
        }
        if(ptbMap->m_nAlid != ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET)
        {
            if(ptbPlayer->m_nAlid != 0 && ptbPlayer->m_nAlpos != EN_ALLIANCE_POS__REQUEST)
            {
                ptbMap->Set_Alid(ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET);
            }
            else if (ptbMap->m_nAlid != 0)
            {
                ptbMap->Set_Alid(0);
            }
            
        }
        if(ptbMap->m_nRally_troop_limit != pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_RALLY_TROOP_LIMIT].m_ddwBuffTotal)
        {
            ptbMap->Set_Rally_troop_limit(pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_RALLY_TROOP_LIMIT].m_ddwBuffTotal);
        }

        if(ptbMap->m_nUlevel != ptbPlayer->m_nLevel)
        {
            ptbMap->Set_Ulevel(ptbPlayer->m_nLevel);
        }
        if(ptbMap->m_nMight != ptbPlayer->m_nMight)
        {
            ptbMap->Set_Might(ptbPlayer->m_nMight);
        }
        if (ptbMap->m_nForce_kill != ptbPlayer->m_bWar_statistics[0].ddwForceKilled)
        {
            ptbMap->Set_Force_kill(ptbPlayer->m_bWar_statistics[0].ddwForceKilled);
        }
        if(ptbMap->m_nStatus != ptbPlayer->m_nStatus)
        {
            ptbMap->Set_Status(ptbPlayer->m_nStatus);
        }
        if(ptbMap->m_nTime_end != pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_PEACE_TIME].m_astBuffDetail[EN_BUFF_TYPE_ITEM].m_dwTime)
        {
            ptbMap->Set_Time_end(pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_PEACE_TIME].m_astBuffDetail[EN_BUFF_TYPE_ITEM].m_dwTime);
        }
        TINT32 dwCastleLv = CCityBase::GetBuildingLevelByFuncType(pstCity, EN_BUILDING_TYPE__CASTLE);
        if(ptbMap->m_nLevel != dwCastleLv)
        {
            ptbMap->Set_Level(dwCastleLv);
        }
        // 大使馆level
        TINT32 dwEmLv = CCityBase::GetBuildingLevelByFuncType(pstCity, EN_BUILDING_TYPE__EMBASSY);
        if(ptbMap->m_nEm_lv != dwEmLv)
        {
            ptbMap->Set_Em_lv(dwEmLv);
        }
        if(ptbMap->m_nAvatar != ptbPlayer->m_nAvatar)
        {
            ptbMap->Set_Avatar(ptbPlayer->m_nAvatar);
        }
        if(ptbMap->m_sAl_nick != pstUser->m_tbAlliance.m_sAl_nick_name)
        {
            if(ptbPlayer->m_nAlid != 0 && ptbPlayer->m_nAlpos != EN_ALLIANCE_POS__REQUEST)
            {
                ptbMap->Set_Al_nick(pstUser->m_tbAlliance.m_sAl_nick_name);
            }
            else
            {
                ptbMap->Set_Al_nick("");
            }
            ptbMap->Set_Name_update_time(CTimeUtils::GetUnixTime());
        }
        if(ptbMap->m_nAl_flag != pstUser->m_tbAlliance.m_nAvatar)
        {
            if(ptbPlayer->m_nAlid != 0 && ptbPlayer->m_nAlpos != EN_ALLIANCE_POS__REQUEST)
            {
                ptbMap->Set_Al_flag(pstUser->m_tbAlliance.m_nAvatar);
            }
            else
            {
                ptbMap->Set_Al_flag(0);
            }
        }
        if(ptbMap->m_nPrison_flag != bPrisonFlag)
        {
            ptbMap->Set_Prison_flag(bPrisonFlag);
        }
        if (writer.write(ptbMap->m_jCity_info) != writer.write(jsonCityInfo)
            && !jsonCityInfo.empty())
        {
            ptbMap->Set_City_info(jsonCityInfo);
        }
        if(ptbPlayer->m_nAge != ptbMap->m_nAge)
        {
            ptbMap->Set_Age(ptbPlayer->m_nAge);
        }
        if(ptbMap->m_nAl_pos != pstUser->m_tbPlayer.m_nAlpos)
        {
            if(ptbPlayer->m_nAlid != 0 && ptbPlayer->m_nAlpos != EN_ALLIANCE_POS__REQUEST)
            {
                ptbMap->Set_Al_pos(pstUser->m_tbPlayer.m_nAlpos);
            }
            else
            {
                ptbMap->Set_Al_pos(0);
            }
        }
        if (ptbMap->m_nVip_level != CPlayerBase::GetRawVipLevel(ptbPlayer, ptbPlayer->m_nVip_point))
        {
            ptbMap->Set_Vip_level(CPlayerBase::GetRawVipLevel(ptbPlayer, ptbPlayer->m_nVip_point));
        }
        if(ptbMap->m_nVip_etime != ptbPlayer->m_nVip_etime)
        {
            ptbMap->Set_Vip_etime(ptbPlayer->m_nVip_etime);
        }
        if (ptbMap->m_sName != pstCity->m_stTblData.m_sName)
        {
            ptbMap->Set_Name(pstCity->m_stTblData.m_sName);
        }
    }

    if (ptbPlayer->m_nCid != pstCity->m_stTblData.m_nPos)
    {
        ptbPlayer->Set_Cid(pstCity->m_stTblData.m_nPos);
    }

    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwWildNum; ++udwIdx)
    {
        TbMap* ptbMap = &pstUser->m_atbWild[udwIdx];

        if(ptbMap->m_nUid != pstUser->m_tbPlayer.m_nUid)
        {
            continue;
        }

        if (EN_WILD_TYPE__CITY == ptbMap->m_nType)
        {
            continue;
        }

        TUINT32 udwWildClass = CMapLogic::GetWildClass(ptbMap->m_nSid, ptbMap->m_nType);
        if ((udwWildClass == EN_WILD_CLASS_MONSTER || udwWildClass == EN_WILD_CLASS_LEADER_MONSTER) &&
            ptbMap->m_nExpire_time < CTimeUtils::GetUnixTime())
        {
            CMapBase::ResetMap(ptbMap);
            continue;
        }

        if(ptbMap->m_sUname != ptbPlayer->m_sUin)
        {
            ptbMap->Set_Uname(ptbPlayer->m_sUin);
            ptbMap->Set_Name_update_time(CTimeUtils::GetUnixTime());
        }

        if(ptbMap->m_nAvatar != ptbPlayer->m_nAvatar)
        {
            ptbMap->Set_Avatar(ptbPlayer->m_nAvatar);
        }

        if(ptbMap->m_sAlname != ptbPlayer->m_sAlname)
        {
            if(ptbPlayer->m_nAlid != 0 && ptbPlayer->m_nAlpos != EN_ALLIANCE_POS__REQUEST)
            {
                ptbMap->Set_Alname(ptbPlayer->m_sAlname);
            }
            else
            {
                ptbMap->Set_Alname("");
            }
            ptbMap->Set_Name_update_time(CTimeUtils::GetUnixTime());
        }

        if(ptbMap->m_sAl_nick != pstUser->m_tbAlliance.m_sAl_nick_name)
        {
            if(ptbPlayer->m_nAlid != 0 && ptbPlayer->m_nAlpos != EN_ALLIANCE_POS__REQUEST)
            {
                ptbMap->Set_Al_nick(pstUser->m_tbAlliance.m_sAl_nick_name);
            }
            else
            {
                ptbMap->Set_Al_nick("");
            }
            ptbMap->Set_Name_update_time(CTimeUtils::GetUnixTime());
        }

        if(ptbMap->m_nAl_flag != pstUser->m_tbAlliance.m_nAvatar)
        {
            if(ptbPlayer->m_nAlid != 0 && ptbPlayer->m_nAlpos != EN_ALLIANCE_POS__REQUEST)
            {
                ptbMap->Set_Al_flag(pstUser->m_tbAlliance.m_nAvatar);
            }
            else
            {
                ptbMap->Set_Al_flag(0);
            }
        }
        if(ptbMap->m_nAl_pos != pstUser->m_tbPlayer.m_nAlpos)
        {
            if(ptbPlayer->m_nAlid != 0 && ptbPlayer->m_nAlpos != EN_ALLIANCE_POS__REQUEST)
            {
                ptbMap->Set_Al_pos(pstUser->m_tbPlayer.m_nAlpos);
            }
            else
            {
                ptbMap->Set_Al_pos(0);
            }
        }
        if (ptbMap->m_nAlid != ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET)
        {
            if (ptbPlayer->m_nAlid != 0 && ptbPlayer->m_nAlpos != EN_ALLIANCE_POS__REQUEST)
            {
                ptbMap->Set_Alid(ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET);
            }
            else if (ptbMap->m_nAlid != 0)
            {
                ptbMap->Set_Alid(0);
            }
        }
        if (ptbMap->m_nMight != ptbPlayer->m_nMight)
        {
            ptbMap->Set_Might(ptbPlayer->m_nMight);
        }
    }

    return 0;
}

TINT32 CCommonHandleAfter::SyncInfoToMarch(SSession *pstSession)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;

    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();

    Json::FastWriter writer;
    writer.omitEndingLineFeed();

    //被别人攻打的rallywar
    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwPassiveMarchNum; ++udwIdx)
    {
        if(pstUser->m_aucPassiveMarchFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }
        TbMarch_action* ptbPassiveMarch = &pstUser->m_atbPassiveMarch[udwIdx];
        if(ptbPassiveMarch->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR
            && ptbPassiveMarch->m_nTuid == pstUser->m_tbPlayer.m_nUid
            && ptbPassiveMarch->m_nTpos == pstSession->m_stReqParam.m_udwCityId)
        {
            if (writer.write(ptbPassiveMarch->m_jCity_info) != writer.write(pstSession->m_jCityInfo)
                && pstSession->m_udwCommand != EN_CLIENT_REQ_COMMAND__MARCH_RALLY_INFO)
            {
                ptbPassiveMarch->Set_City_info(pstSession->m_jCityInfo);
                pstUser->m_aucPassiveMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
            }

            if(pstUser->m_tbPlayer.m_nAlid > 0 && pstUser->m_tbPlayer.m_nAlpos > EN_ALLIANCE_POS__REQUEST)
            {
                if(ptbPassiveMarch->m_nTal != (pstUser->m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET) ||
                    strcmp(ptbPassiveMarch->m_bParam[0].m_szTargetAlNick, pstUser->m_tbAlliance.m_sAl_nick_name.c_str()) != 0)
                {
                    ptbPassiveMarch->Set_Tal(pstUser->m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET);
                    strncpy(ptbPassiveMarch->m_bParam[0].m_szTargetAlNick, pstUser->m_tbAlliance.m_sAl_nick_name.c_str(), MAX_TABLE_NAME_LEN);
                    ptbPassiveMarch->m_bParam[0].m_ddwTargetAlliance = pstUser->m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET;
                    ptbPassiveMarch->SetFlag(TbMARCH_ACTION_FIELD_PARAM);
                    pstUser->m_aucPassiveMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
                }
            }
        }
        if(ptbPassiveMarch->m_nSclass == EN_ACTION_SEC_CLASS__PRISON_TIMER
            && ptbPassiveMarch->m_nTuid == pstUser->m_tbPlayer.m_nUid)
        {
            if(strcmp(ptbPassiveMarch->m_bPrison_param[0].szTargetUserName, pstUser->m_tbPlayer.m_sUin.c_str()) != 0)
            {
                strncpy(ptbPassiveMarch->m_bPrison_param[0].szTargetUserName, pstUser->m_tbPlayer.m_sUin.c_str(), MAX_TABLE_NAME_LEN);
                ptbPassiveMarch->m_bPrison_param[0].szTargetUserName[MAX_TABLE_NAME_LEN - 1] = '\0';
                ptbPassiveMarch->SetFlag(TbMARCH_ACTION_FIELD_PRISON_PARAM);
                pstUser->m_aucPassiveMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
            }

            if(pstCity != NULL
            && strcmp(ptbPassiveMarch->m_bPrison_param[0].szTargetCityName, pstCity->m_stTblData.m_sName.c_str()) != 0)
            {
                strncpy(ptbPassiveMarch->m_bPrison_param[0].szTargetCityName, pstCity->m_stTblData.m_sName.c_str(), MAX_TABLE_NAME_LEN);
                ptbPassiveMarch->m_bPrison_param[0].szTargetCityName[MAX_TABLE_NAME_LEN - 1] = '\0';
                ptbPassiveMarch->SetFlag(TbMARCH_ACTION_FIELD_PRISON_PARAM);
                pstUser->m_aucPassiveMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
            }

            if (strcmp(ptbPassiveMarch->m_bPrison_param[0].szTargetAlNick, pstUser->m_tbPlayer.m_sAl_nick_name.c_str()) != 0)
            {
                strncpy(ptbPassiveMarch->m_bPrison_param[0].szTargetAlNick, pstUser->m_tbPlayer.m_sAl_nick_name.c_str(), MAX_TABLE_NAME_LEN);
                ptbPassiveMarch->SetFlag(TbMARCH_ACTION_FIELD_PRISON_PARAM);
                pstUser->m_aucPassiveMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
            }
        }
    }

    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
    {
        TBOOL bIsNew = FALSE;
        if(pstUser->m_aucMarchFlag[udwIdx] == EN_TABLE_UPDT_FLAG__NEW)
        {
            bIsNew = TRUE;
        }

        if(pstUser->m_aucMarchFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }

        TbMarch_action* ptbMarch = &pstUser->m_atbMarch[udwIdx];
        if(!CActionBase::IsPlayerOwnedAction(pstUser->m_tbPlayer.m_nUid, ptbMarch->m_nId))
        {
            continue;
        }

        //别人警戒塔里看到的信息
        if(ptbMarch->m_nStatus == EN_MARCH_STATUS__MARCHING)
        {
            if(ptbMarch->m_bParam[0].m_stDragon.m_ddwLevel > 0)
            {
                if (ptbMarch->m_bParam[0].m_stDragon.m_ddwLevel != pstUser->m_tbPlayer.m_nDragon_level)
                {
                    ptbMarch->m_bParam[0].m_stDragon.m_ddwLevel = pstUser->m_tbPlayer.m_nDragon_level;
                    ptbMarch->SetFlag(TbMARCH_ACTION_FIELD_PARAM);
                    pstUser->m_aucMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
                }

                if(ptbMarch->m_bParam[0].m_stDragon.m_ddwIconId != pstUser->m_tbPlayer.m_nDragon_avatar)
                {
                    ptbMarch->m_bParam[0].m_stDragon.m_ddwIconId = pstUser->m_tbPlayer.m_nDragon_avatar;
                    ptbMarch->SetFlag(TbMARCH_ACTION_FIELD_PARAM);
                    pstUser->m_aucMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
                }

                if (strcmp(ptbMarch->m_bParam[0].m_stDragon.m_szName, pstUser->m_tbPlayer.m_sDragon_name.c_str()) != 0)
                {
                    strncpy(ptbMarch->m_bParam[0].m_stDragon.m_szName, pstUser->m_tbPlayer.m_sDragon_name.c_str(), MAX_TABLE_NAME_LEN);
                    ptbMarch->m_bParam[0].m_stDragon.m_szName[MAX_TABLE_NAME_LEN] = '\0';
                    ptbMarch->SetFlag(TbMARCH_ACTION_FIELD_PARAM);
                    pstUser->m_aucMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
                }
            }

            if(ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__ATTACK
                || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR)
            {
                SPlayerBuffInfo stNowBuff;
                if(ptbMarch->m_bParam[0].m_stDragon.m_ddwLevel > 0)
                {
                    CBufferBase::GenBattleBuff(&pstUser->m_stPlayerBuffList, &stNowBuff, ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR, 
                        FALSE, ptbMarch->m_bParam[0].m_stKnight.ddwLevel);
                }
                else
                {
                    CBufferBase::GenBattleBuff(&pstUser->m_stBuffWithoutDragon, &stNowBuff, ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR, 
                        FALSE, ptbMarch->m_bParam[0].m_stKnight.ddwLevel);
                }
                SPlayerBuffInfo stOldBuff;
                CBufferBase::MarchBuffToPlayerBuff(ptbMarch, &stOldBuff);
                if(!CBufferBase::IsBuffSame(stNowBuff, stOldBuff))
                {
                    CBufferBase::SetMarchBuff(&stNowBuff, ptbMarch);
                    pstUser->m_aucMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
                }
            }
        }

        if (ptbMarch->m_nStatus == EN_MARCH_STATUS__WAITING)
        {
            if (ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR
                || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR_THRONE)
            {
                TBOOL bNeedWait = FALSE;
                TINT64 ddwEtime = 0;
                for (TUINT32 udwIdy = 0; udwIdy < pstUser->m_udwPassiveMarchNum; ++udwIdy)
                {
                    if (pstUser->m_atbPassiveMarch[udwIdy].m_nSclass == EN_ACTION_SEC_CLASS__RALLY_REINFORCE
                        && pstUser->m_atbPassiveMarch[udwIdy].m_nTid == ptbMarch->m_nId
                        && pstUser->m_atbPassiveMarch[udwIdy].m_nStatus == EN_MARCH_STATUS__MARCHING)
                    {
                        bNeedWait = TRUE;
                        if (ddwEtime < pstUser->m_atbPassiveMarch[udwIdy].m_nEtime)
                        {
                            ddwEtime = pstUser->m_atbPassiveMarch[udwIdy].m_nEtime;
                        }
                    }
                }

                if (bNeedWait == TRUE && ptbMarch->m_nEtime != ddwEtime)
                {
                    ptbMarch->Set_Etime(ddwEtime);
                    pstUser->m_aucMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
                }
                else if (bNeedWait == FALSE)
                {
                    ptbMarch->Set_Etime(CTimeUtils::GetUnixTime());
                    pstUser->m_aucMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
                }
            }
        }
        
        //别人监狱里看到的信息
        if(ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__PRISON_TIMER)
        {
            if(strcmp(ptbMarch->m_bPrison_param[0].szSourceUserName, pstUser->m_tbPlayer.m_sUin.c_str()) != 0)
            {
                strncpy(ptbMarch->m_bPrison_param[0].szSourceUserName, pstUser->m_tbPlayer.m_sUin.c_str(), MAX_TABLE_NAME_LEN);
                ptbMarch->m_bPrison_param[0].szSourceUserName[MAX_TABLE_NAME_LEN - 1] = '\0';
                ptbMarch->SetFlag(TbMARCH_ACTION_FIELD_PRISON_PARAM);
                pstUser->m_aucMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
            }
            if (strcmp(ptbMarch->m_bPrison_param[0].szSourceAlNick, pstUser->m_tbPlayer.m_sAl_nick_name.c_str()) != 0)
            {
                strncpy(ptbMarch->m_bPrison_param[0].szSourceAlNick, pstUser->m_tbPlayer.m_sAl_nick_name.c_str(), MAX_TABLE_NAME_LEN);
                ptbMarch->m_bPrison_param[0].szSourceAlNick[MAX_TABLE_NAME_LEN - 1] = '\0';
                ptbMarch->SetFlag(TbMARCH_ACTION_FIELD_PRISON_PARAM);
                pstUser->m_aucMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
            }
        }

        if(ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_NORMAL
            || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_REINFORCE)
        {
            if(ptbMarch->m_nSavatar != pstUser->m_tbPlayer.m_nAvatar)
            {
                ptbMarch->Set_Savatar(pstUser->m_tbPlayer.m_nAvatar);
                pstUser->m_aucMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
            }

            if(strcmp(ptbMarch->m_bParam[0].m_szSourceUserName, pstUser->m_tbPlayer.m_sUin.c_str()) != 0)
            {
                strncpy(ptbMarch->m_bParam[0].m_szSourceUserName, pstUser->m_tbPlayer.m_sUin.c_str(), MAX_TABLE_NAME_LEN);
                ptbMarch->m_bParam[0].m_szSourceUserName[MAX_TABLE_NAME_LEN - 1] = '\0';
                ptbMarch->SetFlag(TbMARCH_ACTION_FIELD_PARAM);
                pstUser->m_aucMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
            }
        }

        if(pstUser->m_atbMarch[udwIdx].m_nStatus == EN_MARCH_STATUS__RETURNING
            && CActionBase::IsEmptyMarch(&pstUser->m_atbMarch[udwIdx]))
        {
            if (pstUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__RALLY_REINFORCE
                || pstUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_NORMAL
                || pstUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_THRONE)
            {
                if(pstUser->m_atbMarch[udwIdx].m_nEtime > (udwCurTime + 30))
                {
                    pstUser->m_atbMarch[udwIdx].Set_Etime(udwCurTime);
                    pstUser->m_aucMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
                }
            }
        }

        if(bIsNew)
        {
            pstUser->m_aucMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__NEW;
        }
    }

    return 0;
}

TINT32 CCommonHandleAfter::DelEmptytBuildingPos(SSession* pstSession)
{
    SCityInfo *pstCity = &pstSession->m_stUserInfo.m_stCityInfo;
    if (0 == pstCity->m_stTblData.m_nUid)
    {
        return 0;
    }

    for(TUINT32 udwIdx = 0; udwIdx < pstCity->m_stTblData.m_bBuilding.m_udwNum;)
    {
        SCityBuildingNode& pstNode = pstCity->m_stTblData.m_bBuilding[udwIdx];
        if(pstNode.m_ddwLevel == 0)
        {
            pstNode = pstCity->m_stTblData.m_bBuilding[pstCity->m_stTblData.m_bBuilding.m_udwNum - 1];
            pstCity->m_stTblData.m_bBuilding.m_udwNum--;
            pstCity->m_stTblData.SetFlag(TbCITY_FIELD_BUILDING);
            continue;
        }
        ++udwIdx;
    }
    return 0;
}

TINT32 CCommonHandleAfter::HandleGuideFinishStage(SSession *pstSession)
{
    SCityInfo *pstCity = &pstSession->m_stUserInfo.m_stCityInfo;

    // 客户端新手教学直接触发
    // 科技教学未完成，又存在科技馆建造
    if(0 == BITTEST(pstSession->m_stUserInfo.m_tbLogin.m_bGuide_flag[0].m_bitFlag, EN_GUIDE_FINISH_STAGE_1_RESEARCH))
    {
        TUINT32 udwBuildingNum = CCityBase::GetBuildingNumByFuncType(pstCity, EN_BUILDING_TYPE__ALCHEMY_LAB);
        for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_stUserInfo.m_udwSelfAlActionNum; ++udwIdx)
        {
            TbAlliance_action *ptbAlAction = &pstSession->m_stUserInfo.m_atbSelfAlAction[udwIdx];
            if(!CActionBase::IsPlayerOwnedAction(pstSession->m_stUserInfo.m_tbPlayer.m_nUid, ptbAlAction->m_nId))
            {
                continue;
            }

            SActionBuildingParam *pstActionParam = &ptbAlAction->m_bParam[0].m_stBuilding;

            if(ptbAlAction->m_nMclass == EN_ACTION_MAIN_CLASS__BUILDING &&
                ptbAlAction->m_nSclass == EN_ACTION_SEC_CLASS__BUILDING_UPGRADE &&
                CCityBase::GetBuildingFuncType(pstActionParam->m_ddwType) == EN_BUILDING_TYPE__ALCHEMY_LAB &&
                pstActionParam->m_ddwTargetLevel == 1 &&
                udwBuildingNum == 0)
            {
                CProcessAction::BuildingActionDone(pstSession, &pstSession->m_stUserInfo, pstCity, (EActionSecClass)ptbAlAction->m_nSclass, pstActionParam, TRUE);
                pstSession->m_stUserInfo.m_aucSelfAlActionFlag[udwIdx] = EN_TABLE_UPDT_FLAG__DEL;
            }
        }
    }

    // 兵营教学未完成，又存在兵营建造
    if (0 == BITTEST(pstSession->m_stUserInfo.m_tbLogin.m_bGuide_flag[0].m_bitFlag, EN_GUIDE_FINISH_STAGE_2_TRAIN_TROOP))
    {
        TUINT32 udwBuildingNum = CCityBase::GetBuildingNumByFuncType(pstCity, EN_BUILDING_TYPE__INFANTRY_BARRACKS);
        for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_stUserInfo.m_udwSelfAlActionNum; ++udwIdx)
        {
            TbAlliance_action *ptbAlAction = &pstSession->m_stUserInfo.m_atbSelfAlAction[udwIdx];
            if(!CActionBase::IsPlayerOwnedAction(pstSession->m_stUserInfo.m_tbPlayer.m_nUid, ptbAlAction->m_nId))
            {
                continue;
            }

            SActionBuildingParam *pstActionParam = &ptbAlAction->m_bParam[0].m_stBuilding;

            if(ptbAlAction->m_nMclass == EN_ACTION_MAIN_CLASS__BUILDING &&
                ptbAlAction->m_nSclass == EN_ACTION_SEC_CLASS__BUILDING_UPGRADE &&
                CCityBase::GetBuildingFuncType(pstActionParam->m_ddwType) == EN_BUILDING_TYPE__INFANTRY_BARRACKS &&
                pstActionParam->m_ddwTargetLevel == 1 &&
                udwBuildingNum == 0)
            {
                CProcessAction::BuildingActionDone(pstSession, &pstSession->m_stUserInfo, pstCity, (EActionSecClass)ptbAlAction->m_nSclass, pstActionParam, TRUE);
                pstSession->m_stUserInfo.m_aucSelfAlActionFlag[udwIdx] = EN_TABLE_UPDT_FLAG__DEL;
            }
        }
    }

    //rally field 教学未完成 又存在relly field建造
//     if (0 == BITTEST(pstSession->m_stUserInfo.m_tbLogin.m_bGuide_flag[0].m_bitFlag, EN_GUIDE_FINISH_STAGE_4_OCCUPY_RESOURCE))
//     {
//         TUINT32 udwBuildingNum = CCityBase::GetBuildingNumByFuncType(pstCity, EN_BUILDING_TYPE__RALLY_POINT);
//         for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_stUserInfo.m_udwSelfAlActionNum; ++udwIdx)
//         {
//             TbAlliance_action *ptbAlAction = &pstSession->m_stUserInfo.m_atbSelfAlAction[udwIdx];
//             if(!CActionBase::IsPlayerOwnedAction(pstSession->m_stUserInfo.m_tbPlayer.m_nUid, ptbAlAction->m_nId))
//             {
//                 continue;
//             }
// 
//             SActionBuildingParam *pstActionParam = &ptbAlAction->m_bParam[0].m_stBuilding;
// 
//             if(ptbAlAction->m_nMclass == EN_ACTION_MAIN_CLASS__BUILDING &&
//                 ptbAlAction->m_nSclass == EN_ACTION_SEC_CLASS__BUILDING_UPGRADE &&
//                 CCityBase::GetBuildingFuncType(pstActionParam->m_ddwType) == EN_BUILDING_TYPE__RALLY_POINT &&
//                 pstActionParam->m_ddwTargetLevel == 1 &&
//                 udwBuildingNum == 0)
//             {
//                 CProcessAction::BuildingActionDone(pstSession, &pstSession->m_stUserInfo, pstCity, (EActionSecClass)ptbAlAction->m_nSclass, pstActionParam, TRUE);
//                 pstSession->m_stUserInfo.m_aucSelfAlActionFlag[udwIdx] = EN_TABLE_UPDT_FLAG__DEL;
//             }
//         }
//     }

    return 0;
}

TINT32 CCommonHandleAfter::UpdateNotiTimer(SSession* pstSession)
{
    TINT32 dwCurTime = CTimeUtils::GetUnixTime();

    SUserInfo *pstUser = &pstSession->m_stUserInfo;

    TBOOL bNeedTimer = FALSE;
    TINT32 dwTickTimeStamp = 0;
    TbMarch_action* ptbTimer = NULL;
    TINT32 dwTimerIndex = -1;

    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
    {
        if(pstUser->m_aucMarchFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }
        ptbTimer = &pstUser->m_atbMarch[udwIdx];

        if(!CActionBase::IsPlayerOwnedAction(pstUser->m_tbPlayer.m_nUid, ptbTimer->m_nId))
        {
            continue;
        }

        if(ptbTimer->m_nSclass != EN_ACTION_SEC_CLASS__NOTI_TIMER)
        {
            continue;
        }

        dwTimerIndex = udwIdx;
        break;
    }

    if(dwTimerIndex != -1)
    {
        CQuestNoticLogic::ShowNoticFlag(pstUser, ptbTimer);
        CQuestNoticLogic::CheckTaskNotic(pstUser, ptbTimer);
    }

    //building research 相关
    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwSelfAlActionNum; ++udwIdx)
    {
        if(pstUser->m_aucSelfAlActionFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }
        TbAlliance_action* ptbCanFreeAction = &pstUser->m_atbSelfAlAction[udwIdx];

        if(!CActionBase::IsPlayerOwnedAction(pstUser->m_tbPlayer.m_nUid, ptbCanFreeAction->m_nId))
        {
            continue;
        }

        if(ptbCanFreeAction->m_nNoti_flag == TRUE)
        {
            continue;
        }

        if(ptbCanFreeAction->m_nSclass != EN_ACTION_SEC_CLASS__BUILDING_UPGRADE
        && ptbCanFreeAction->m_nSclass != EN_ACTION_SEC_CLASS__BUILDING_REMOVE
        && ptbCanFreeAction->m_nSclass != EN_ACTION_SEC_CLASS__RESEARCH_UPGRADE
        && ptbCanFreeAction->m_nSclass != EN_ACTION_SEC_CLASS__EQUIP_UPGRADE)
        {
            continue;
        }

        TINT32 dwCanFreeTimeStamp = ptbCanFreeAction->m_nEtime -
            CCommonBase::GetGameBasicVal(EN_GAME_BASIC_FREE_INSTANT_TIME) -
            pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_FREE_INSTANT_FINISH_TIME].m_ddwBuffTotal;
        if (dwTickTimeStamp == 0 || dwTickTimeStamp > dwCanFreeTimeStamp)
        {
            dwTickTimeStamp = dwCanFreeTimeStamp;
            bNeedTimer = TRUE;
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("NotiTimer[timestamp=%d][seq=%u]", dwTickTimeStamp, pstSession->m_udwSeqNo));
        }
    }

    //buff time相关
    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwActionNum; ++udwIdx)
    {
        if(pstUser->m_aucActionFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }
        TbAction* ptbPeacetime = &pstUser->m_atbAction[udwIdx];

        if(ptbPeacetime->m_nSclass != EN_ACTION_SEC_CLASS__ITEM)
        {
            continue;
        }

        if(ptbPeacetime->m_nNoti_flag == TRUE)
        {
            continue;
        }

        TUINT32 udwNotiId = EN_NOTI_ID__END;
        switch(ptbPeacetime->m_bParam[0].m_stItem.m_ddwBufferId)
        {
        case EN_BUFFER_INFO_PEACE_TIME:
            udwNotiId = EN_NOTI_ID__PEACETIME_END;
            if(pstUser->m_tbPlayer.m_nStatus & EN_CITY_STATUS__NEW_PROTECTION)
            {
                udwNotiId = EN_NOTI_ID__NEW_PLAYER_END;
            }
            break;
        case EN_BUFFER_INFO_VIP_ACTIVATE:
            udwNotiId = EN_NOTI_ID__VIP_EXPIRED;
            break;
        case EN_BUFFER_INFO_ALL_TROOP_ATTACK:
        case EN_BUFFER_INFO_ALL_TROOP_LIFE:
        case EN_BUFFER_INFO_GOLD_PRODUCTION:
        case EN_BUFFER_INFO_FOOD_PRODUCTION:
        case EN_BUFFER_INFO_WOOD_PRODUCTION:
        case EN_BUFFER_INFO_STONE_PROTECTION:
        case EN_BUFFER_INFO_ORE_PROTECTION:
        case EN_BUFFER_INFO_QUEUE_NUM:
        case EN_BUFFER_INFO_TROOP_SIZE:
            udwNotiId = EN_NOTI_ID__BUFF_EXPIRED;
            break;
        default:
            break;
        }

        if (udwNotiId == EN_NOTI_ID__END)
        {
            continue;
        }

        TINT32 dwNeedNotiTimeStamp = ptbPeacetime->m_nEtime - 3600;
        if(dwTickTimeStamp == 0 || dwTickTimeStamp > dwNeedNotiTimeStamp)
        {
            dwTickTimeStamp = dwNeedNotiTimeStamp;
            bNeedTimer = TRUE;
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("NotiTimer[timestamp=%d][seq=%u]", dwTickTimeStamp, pstSession->m_udwSeqNo));
        }
    }

    //top quest
    //TBOOL IsTopQuestClaim = FALSE;
    //if(dwTimerIndex == -1 || (dwTimerIndex >= 0 && 0 == BITTEST(ptbTimer->m_bNotic_task_flag[0].m_bitFlag, EN_TOP_QUEST)))
    //{
    //    TSE_LOG_DEBUG(pstSession->m_poServLog, ("UpdateNotiTimer:uid=%u check top_quest noti or not[seq=%u]", pstUser->m_tbPlayer.m_nUid, pstUser->m_udwBSeqNo));
    //    CQuestLogic::CheckTopQuestCanClaim(pstUser, IsTopQuestClaim);
    //    if(TRUE == IsTopQuestClaim)
    //    {
    //        TSE_LOG_DEBUG(pstSession->m_poServLog, ("UpdateNotiTimer:uid=%u top_quest need noti [seq=%u]",pstUser->m_tbPlayer.m_nUid,pstUser->m_udwBSeqNo));
    //        TINT32 dwNeedNotiTimeStamp = dwCurTime;
    //        if(dwTickTimeStamp == 0 || dwTickTimeStamp > dwNeedNotiTimeStamp)
    //        {
    //            dwTickTimeStamp = dwNeedNotiTimeStamp;
    //            bNeedTimer = TRUE;
    //        }
    //    }
    //}

    TUINT32 udwQuestFinishTime = 0;
    TBOOL IsQuestRunning = FALSE;

    //daily quest 
    if(dwTimerIndex == -1 || (dwTimerIndex >= 0 && 0 == BITTEST(ptbTimer->m_bNotic_task_flag[0].m_bitFlag, EN_DAILY_QUEST)))
    {
        udwQuestFinishTime = CQuestLogic::CheckQuestNodeRunning(&pstUser->m_tbQuest.m_bDaily_quest[0], IsQuestRunning);
        if(TRUE == IsQuestRunning)
        {
            TINT32 dwNeedNotiTimeStamp = udwQuestFinishTime;
            if(dwTickTimeStamp == 0 || dwTickTimeStamp > dwNeedNotiTimeStamp)
            {
                dwTickTimeStamp = dwNeedNotiTimeStamp;
                bNeedTimer = TRUE;
            }
        }
    }

    //alliance quest 
    if(dwTimerIndex == -1 || (dwTimerIndex >= 0 && 0 == BITTEST(ptbTimer->m_bNotic_task_flag[0].m_bitFlag, EN_ALLIANCE_QUEST)))
    {
        udwQuestFinishTime = CQuestLogic::CheckQuestNodeRunning(&pstUser->m_tbQuest.m_bAl_quest[0], IsQuestRunning);
        if(TRUE == IsQuestRunning)
        {
            TINT32 dwNeedNotiTimeStamp = udwQuestFinishTime;
            if(dwTickTimeStamp == 0 || dwTickTimeStamp > dwNeedNotiTimeStamp)
            {
                dwTickTimeStamp = dwNeedNotiTimeStamp;
                bNeedTimer = TRUE;
            }
        }
    }
        
    //vip quest 
    if(dwTimerIndex == -1 || (dwTimerIndex >= 0 && 0 == BITTEST(ptbTimer->m_bNotic_task_flag[0].m_bitFlag, EN_VIP_QUEST)))
    {
        udwQuestFinishTime = CQuestLogic::CheckQuestNodeRunning(&pstUser->m_tbQuest.m_bVip_quest[0], IsQuestRunning);
        if(TRUE == IsQuestRunning)
        {
            TINT32 dwNeedNotiTimeStamp = udwQuestFinishTime;
            if(dwTickTimeStamp == 0 || dwTickTimeStamp > dwNeedNotiTimeStamp)
            {
                dwTickTimeStamp = dwNeedNotiTimeStamp;
                bNeedTimer = TRUE;
            }
        }
    }
        
    //mistery
    if(dwTimerIndex == -1 || (dwTimerIndex >= 0 && 0 == BITTEST(ptbTimer->m_bNotic_task_flag[0].m_bitFlag, EN_MISTERY_QUEST)))
    {
        udwQuestFinishTime = CQuestLogic::CheckQuestNodeRunning(&pstUser->m_tbQuest.m_bTimer_gift[0], IsQuestRunning);
        if(TRUE == IsQuestRunning)
        {
            TINT32 dwNeedNotiTimeStamp = udwQuestFinishTime;
            if(dwTickTimeStamp == 0 || dwTickTimeStamp > dwNeedNotiTimeStamp)
            {
                dwTickTimeStamp = dwNeedNotiTimeStamp;
                bNeedTimer = TRUE;
            }
        }
    }

    // wave@20160428: dragon 体力恢复时间计算
    if (pstUser->m_tbPlayer.m_nDragon_recovery_time > dwCurTime)
    {
        TINT32 dwNeedNotiTimeStamp = pstUser->m_tbPlayer.m_nDragon_recovery_time;
        if (dwTickTimeStamp == 0 || dwTickTimeStamp > dwNeedNotiTimeStamp)
        {
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("UpdateNotiTimer:uid=%u dragon_recovery need noti [seq=%u]", pstUser->m_tbPlayer.m_nUid, pstUser->m_udwBSeqNo));
            dwTickTimeStamp = dwNeedNotiTimeStamp;
            bNeedTimer = TRUE;
        }
    }

    // wave@20160428：knight 自动unassign时间计算
    TINT32 dwKnightAutoUnassignTime = CProductionSystem::ComputeKnightUnassignTime(pstUser, &pstUser->m_stCityInfo);
    if (dwKnightAutoUnassignTime)
    {
        if (dwTickTimeStamp == 0 || dwTickTimeStamp > dwKnightAutoUnassignTime)
        {
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("UpdateNotiTimer:uid=%u ComputeKnightUnassignTime need noti [seq=%u]", pstUser->m_tbPlayer.m_nUid, pstUser->m_udwBSeqNo));
            dwTickTimeStamp = dwKnightAutoUnassignTime;
            bNeedTimer = TRUE;
        }
    }

    if(!bNeedTimer)
    {
        if(dwTimerIndex >= 0)
        {
            pstUser->m_aucMarchFlag[dwTimerIndex] = EN_TABLE_UPDT_FLAG__CHANGE;
            ptbTimer->Set_Etime(dwCurTime + 60 * 60 * 24 * 7 + 300);
        }
        return 0;
    }

    if(dwTimerIndex == -1)
    {
        ptbTimer = CActionBase::AddNewMarch(pstUser);
        ptbTimer->Set_Suid(pstUser->m_tbPlayer.m_nUid);
        ptbTimer->Set_Scid(pstSession->m_stReqParam.m_udwCityId);
        ptbTimer->Set_Mclass(EN_ACTION_MAIN_CLASS__TIMER);
        ptbTimer->Set_Sclass(EN_ACTION_SEC_CLASS__NOTI_TIMER);
        ptbTimer->Set_Sid(pstUser->m_tbPlayer.m_nSid);
    }

    if(dwCurTime + 60 > dwTickTimeStamp)
    {
        dwTickTimeStamp = dwCurTime + 60;
    }

    if (dwTickTimeStamp != ptbTimer->m_nEtime)
    {
        ptbTimer->Set_Etime(dwTickTimeStamp);
        pstUser->m_aucMarchFlag[dwTimerIndex] = EN_TABLE_UPDT_FLAG__CHANGE;
    }

    if(dwTimerIndex != -1)
    {
        CQuestNoticLogic::ShowNoticFlag(pstUser, ptbTimer);
    }
    return 0;
}

// function  ===> 更新用户的al_gift
TINT32 CCommonHandleAfter::ComputeAllianceGift(SSession *pstSession)
{
    //do nothing
    return 0;
}

TBOOL CCommonHandleAfter::TbAlGift_CompareRe(const TbAl_gift& stA, const TbAl_gift& stB)
{
    if(stA.m_nCtime > stB.m_nCtime)
    {
        return TRUE;
    }
    return FALSE;
}

TBOOL CCommonHandleAfter::IsExisted(TbAl_gift_reward* patbAlGiftRewardList, TUINT32 udwListSize, TINT64 ddwGiftId)
{
    TBOOL bIsExisted = FALSE;
    for(TUINT32 udwIdx = 0; udwIdx < udwListSize; udwIdx++)
    {
        if(patbAlGiftRewardList[udwIdx].m_nGid == ddwGiftId)
        {
            bIsExisted = TRUE;
            break;
        }
    }
    return bIsExisted;
}

TINT32 CCommonHandleAfter::CorrectExp(SSession* pstSession)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    const Json::Value &oDragonExpJson = CGameInfo::GetInstance()->m_oJsonRoot["game_dragon_exp"];

    TUINT32 udwLv = pstUser->m_tbPlayer.m_nDragon_level;

    TBOOL bExpError = FALSE;
    if(udwLv > 1 && pstUser->m_tbPlayer.m_nDragon_exp == 0)
    {
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("Dragon exp not match level[seq=%u]", pstSession->m_udwSeqNo));
        bExpError = TRUE;
    }

    if(udwLv >= oDragonExpJson.size())
    {
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("Dragon level overflow[lv=%u][seq=%u]", udwLv, pstSession->m_udwSeqNo));
        udwLv = oDragonExpJson.size() - 1;
        bExpError = TRUE;
    }

    if(bExpError == TRUE)
    {
        TUINT32 udwExp = oDragonExpJson[udwLv][0U].asUInt();

        pstSession->m_stUserInfo.m_tbPlayer.Set_Dragon_exp(udwExp);
        pstSession->m_stUserInfo.m_tbPlayer.Set_Dragon_level(udwLv);
    }

    const Json::Value &oLordExpJson = CGameInfo::GetInstance()->m_oJsonRoot["game_lord_exp"];

    udwLv = pstUser->m_tbPlayer.m_nLevel;

    bExpError = FALSE;
    if (udwLv > 1 && pstUser->m_tbPlayer.m_nExp == 0)
    {
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("Lord exp not match level[seq=%u]", pstSession->m_udwSeqNo));
        bExpError = TRUE;
    }

    if (udwLv >= oLordExpJson.size())
    {
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("Lord level overflow[lv=%u][seq=%u]", udwLv, pstSession->m_udwSeqNo));
        udwLv = oLordExpJson.size() - 1;
        bExpError = TRUE;
    }

    if (bExpError == TRUE)
    {
        TUINT32 udwExp = oLordExpJson[udwLv][0U].asUInt();

        pstSession->m_stUserInfo.m_tbPlayer.Set_Exp(udwExp);
        pstSession->m_stUserInfo.m_tbPlayer.Set_Level(udwLv);
    }

    return 0;
}

TINT32 CCommonHandleAfter::CompareLordLevel(SSession *pstSession)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;

    TINT32 dwBeforeLevel = 0;
    const Json::Value& oJsonExp = CGameInfo::GetInstance()->m_oJsonRoot["game_lord_exp"];
    for(dwBeforeLevel = 0; dwBeforeLevel < (TINT32)oJsonExp.size(); ++dwBeforeLevel)
    {
        if(pstUser->m_udwBeforPlayerExp < oJsonExp[dwBeforeLevel][0u].asInt64())
        {
            break;
        }
    }
    dwBeforeLevel = dwBeforeLevel > 0 ? dwBeforeLevel - 1 : dwBeforeLevel;

    TINT32 dwAfterLevel = CPlayerBase::ComputePlayerLevel(&pstUser->m_tbPlayer);

    return 0;
}

TINT32 CCommonHandleAfter::DelNotiTimer(SSession* pstSession)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    if(pstUser->m_tbPlayer.m_nUid == 0)
    {
        return 0;
    }

    TUINT32 udwCount = 0;
    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
    {
        if(pstUser->m_aucMarchFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }
        TbMarch_action* ptbTimer = &pstUser->m_atbMarch[udwIdx];

        if(!CActionBase::IsPlayerOwnedAction(pstUser->m_tbPlayer.m_nUid, ptbTimer->m_nId))
        {
            continue;
        }

        if(ptbTimer->m_nSclass == EN_ACTION_SEC_CLASS__NOTI_TIMER)
        {
            udwCount++;
            if(udwCount > 1)
            {
                pstUser->m_aucMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__DEL;
            }
            continue;
        }
    }

    return 0;
}

TINT32 CCommonHandleAfter::CheckSvrAlRecord(SSession *pstSession)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;

    if (ptbPlayer->m_nAlpos == EN_ALLIANCE_POS__CHANCELLOR && ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET > 0
        && pstUser->m_aucSvrAlFlag == EN_TABLE_UPDT_FLAG__UNCHANGE)
    {
        if (pstSession->m_stReqParam.m_ucLoginStatus == EN_LOGIN_STATUS__LOGIN
            && pstSession->m_stReqParam.m_udwCommandID == EN_CLIENT_REQ_COMMAND__LOGIN_GET)
        {
            if (pstUser->m_tbSvrAl.m_nAlid == 0)
            {
                pstUser->m_tbSvrAl.Set_Sid(ptbPlayer->m_nSid);
                pstUser->m_tbSvrAl.Set_Alid(ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET);
            }
            pstUser->m_tbSvrAl.Set_Owner_uid(ptbPlayer->m_nUid);
            pstUser->m_tbSvrAl.Set_Owner_cid(pstSession->m_stReqParam.m_udwCityId);
            pstUser->m_aucSvrAlFlag = EN_TABLE_UPDT_FLAG__CHANGE;
        }
        else
        {
            if (pstUser->m_tbSvrAl.m_nOwner_uid == ptbPlayer->m_nUid &&
                pstUser->m_tbSvrAl.m_nOwner_cid != pstSession->m_stReqParam.m_udwCityId)
            {
                if (pstUser->m_tbSvrAl.m_nAlid == 0)
                {
                    pstUser->m_tbSvrAl.Set_Sid(ptbPlayer->m_nSid);
                    pstUser->m_tbSvrAl.Set_Alid(ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET);
                }
                pstUser->m_tbSvrAl.Set_Owner_uid(ptbPlayer->m_nUid);
                pstUser->m_tbSvrAl.Set_Owner_cid(pstSession->m_stReqParam.m_udwCityId);
                pstUser->m_aucSvrAlFlag = EN_TABLE_UPDT_FLAG__CHANGE;
            }
        }
    }

    return 0;
}

TINT32 CCommonHandleAfter::CheckTaxAction(SSession *pstSession)
{
    if (pstSession->m_stReqParam.m_ucLoginStatus == EN_LOGIN_STATUS__LOGIN
        && pstSession->m_stReqParam.m_udwCommandID == EN_CLIENT_REQ_COMMAND__LOGIN_GET
        && pstSession->m_stUserInfo.m_tbAlliance.m_nAid != 0
        && pstSession->m_stUserInfo.m_tbAlliance.m_nAid == pstSession->m_tbThrone.m_nAlid
        && pstSession->m_stUserInfo.m_tbAlliance.m_nOid == pstSession->m_stUserInfo.m_tbPlayer.m_nUid)
    {
        TBOOL bIsFind = FALSE;
        for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_stUserInfo.m_udwMarchNum; udwIdx++)
        {
            if (pstSession->m_stUserInfo.m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__TAX
                && pstSession->m_stUserInfo.m_atbMarch[udwIdx].m_nSuid == pstSession->m_stUserInfo.m_tbPlayer.m_nUid)
            {
                if (bIsFind == FALSE)
                {
                    bIsFind = TRUE;
                }
                else
                {
                    pstSession->m_stUserInfo.m_aucMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__DEL;
                }
            }
        }

        if (bIsFind == FALSE)
        {
            CActionBase::GenTaxAction(&pstSession->m_stUserInfo, pstSession->m_tbThrone.m_nPos);
        }
    }
    else if (pstSession->m_stUserInfo.m_tbAlliance.m_nAid == 0
        || pstSession->m_stUserInfo.m_tbAlliance.m_nAid != pstSession->m_tbThrone.m_nAlid
        || pstSession->m_stUserInfo.m_tbAlliance.m_nOid != pstSession->m_stUserInfo.m_tbPlayer.m_nUid)
    {
        for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_stUserInfo.m_udwMarchNum; udwIdx++)
        {
            if (pstSession->m_stUserInfo.m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__TAX
                && pstSession->m_stUserInfo.m_atbMarch[udwIdx].m_nSuid == pstSession->m_stUserInfo.m_tbPlayer.m_nUid)
            {
                if (pstSession->m_stUserInfo.m_atbMarch[udwIdx].m_jTax_info.empty()
                    && pstSession->m_stUserInfo.m_atbMarch[udwIdx].m_nStatus == EN_TAX_STATUS__PREPARING)
                {
                    pstSession->m_stUserInfo.m_aucMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__DEL;
                }
            }
        }
    }

    return 0;
}

TVOID CCommonHandleAfter::UpdateLastMight(SSession * pstSession)
{
    if (pstSession->m_stReqParam.m_udwSvrId != pstSession->m_stUserInfo.m_tbLogin.m_nSid)
    {
        return;
    }

    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    Json::FastWriter rWriter;
    rWriter.omitEndingLineFeed();

    Json::Value rTroopAndFort = Json::Value(Json::objectValue);
    TUINT64 uddwTotalMight = CCommonBase::GetTotalTroopAndFort(pstUser, rTroopAndFort);

    pstUser->m_tbUserStat.Set_Last_might(uddwTotalMight);
    pstUser->m_tbUserStat.Set_Last_troop_fort(rTroopAndFort);

    //log
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("UpdateLastMight: update current might. [uid=%ld][now_might=%lu][json=%s] [seq=%u]", 
        pstUser->m_tbPlayer.m_nUid, uddwTotalMight, rWriter.write(rTroopAndFort).c_str(), pstSession->m_udwSeqNo));
}

TVOID CCommonHandleAfter::CheckDragon(SSession *pstSession)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;
    
    if (CToolBase::IsOpCommand(pstSession->m_stReqParam.m_szCommand))
    {
        return;
    }

    if (ptbPlayer->m_nDragon_max_lv < pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_UNLOCK_DRAGON_LV].m_ddwBuffTotal)
    {
        ptbPlayer->Set_Dragon_max_lv(pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_UNLOCK_DRAGON_LV].m_ddwBuffTotal);
    }
    else if (ptbPlayer->m_nHas_dragon == 0 && ptbPlayer->m_nDragon_max_lv != pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_UNLOCK_DRAGON_LV].m_ddwBuffTotal)
    {
        ptbPlayer->Set_Dragon_max_lv(pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_UNLOCK_DRAGON_LV].m_ddwBuffTotal);
    }

    if (ptbPlayer->m_nHas_dragon == 2)
    {
        ptbPlayer->Set_Has_dragon(1);
        pstSession->m_dwDragonUnlockFlag = 1;
    }

//     TUINT32 udwCastleLv = CCityBase::GetBuildingLevelByFuncType(&pstUser->m_stCityInfo, EN_BUILDING_TYPE__CASTLE);
// 
//     if (ptbPlayer->m_nDragon_max_lv > 0 && ptbPlayer->m_nHas_dragon == 0 
//         && udwCastleLv >= CCommonBase::GetGameBasicVal(EN_GAME_BASIC_CITY_LV_DRAGON_UNLOCK))
//     {
//         pstSession->m_dwDragonUnlockFlag = 1;
//         ptbPlayer->Set_Has_dragon(1L);
//         ptbPlayer->Set_Dragon_exp(0);
//         ptbPlayer->Set_Dragon_level(1);
//         ptbPlayer->Set_Dragon_cur_energy(CCommonBase::GetGameBasicVal(EN_GAME_BASIC_HERO_ENERGY_DEFAULT));
//         ptbPlayer->Set_Dragon_recovery_time(0);
//         ptbPlayer->Set_Dragon_recovery_count(0);
//         ptbPlayer->Set_Dragon_begin_recovery_time(0);
//         ptbPlayer->Set_Dragon_tid(0);
//         ptbPlayer->Set_Dragon_status(EN_DRAGON_STATUS_NORMAL);
// 
//         ostringstream oss;
//         TCHAR szUIntToString[MAX_TABLE_NAME_LEN];
//         //龙名字
//         oss.str("");
//         oss << CGameInfo::GetInstance()->m_oJsonRoot["game_init_data"]["r"]["a2"].asString().c_str();
//         if (ptbPlayer->m_nUid)
//         {
//             CAwsRequest::UIntToString(ptbPlayer->m_nUid, szUIntToString);
//             oss << szUIntToString;
//         }
//         ptbPlayer->Set_Dragon_name(oss.str());
// 
//         //龙头像
//         TUINT32 udwDragonUi = 0;
//         TUINT32 udwDragonUiSize = CGameInfo::GetInstance()->m_oJsonRoot["game_init_data"]["r"]["a4"].size();
//         if (udwDragonUiSize != 0)
//         {
//             udwDragonUi = rand() % udwDragonUiSize;
//         }
//         ptbPlayer->Set_Dragon_avatar(udwDragonUi);
//     }
//     else if (ptbPlayer->m_nHas_dragon == 1)
//     {
//         //TODO check 等级和经验...超出要减掉
//     }
}

TVOID CCommonHandleAfter::CheckTitle(SSession *pstSession)
{
    if (CToolBase::IsOpCommand(pstSession->m_stReqParam.m_szCommand))
    {
        return;
    }

    //TODO
    /*
    STitleInfoList* pTitleInfoList = &pstSession->m_stTitleInfoList;
    TbPlayer *ptbPlayer = &pstSession->m_stUserInfo.m_tbPlayer;
    for (TUINT32 udwIdx = 0; udwIdx < pTitleInfoList->udwNum; ++udwIdx)
    {
        TbTitle* ptbTitle = &pTitleInfoList->atbTitle[udwIdx];
        for (TUINT32 udwIdy = 0; udwIdy < ptbTitle->m_bTitles.m_udwNum; ++udwIdy)
        {
            TBOOL bChange = FALSE;

            if (pTitleInfoList->aucFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
            {
                continue;
            }

            if (ptbTitle->m_bTitles[udwIdy].ddwOid == ptbPlayer->m_nUid)
            {
                if (strcmp(ptbTitle->m_bTitles[udwIdy].szUserName, ptbPlayer->m_sUin.c_str()) != 0)
                {
                    strncpy(ptbTitle->m_bTitles[udwIdy].szUserName, ptbPlayer->m_sUin.c_str(), MAX_TABLE_NAME_LEN);
                    ptbTitle->m_bTitles[udwIdy].szUserName[MAX_TABLE_NAME_LEN - 1] = '\0';
                    bChange = TRUE;
                }
                if (strcmp(ptbTitle->m_bTitles[udwIdy].szAlnick, ptbPlayer->m_sAl_nick_name.c_str()) != 0)
                {
                    strncpy(ptbTitle->m_bTitles[udwIdy].szAlnick, ptbPlayer->m_sAl_nick_name.c_str(), MAX_TABLE_NAME_LEN);
                    ptbTitle->m_bTitles[udwIdy].szAlnick[MAX_TABLE_NAME_LEN - 1] = '\0';
                    bChange = TRUE;
                }
                TINT32 dwReadAid = ptbPlayer->m_nAlpos > EN_ALLIANCE_POS__REQUEST ? ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET : 0;
                if (ptbTitle->m_bTitles[udwIdy].ddwAid != dwReadAid)
                {
                    ptbTitle->m_bTitles[udwIdy].ddwAid = dwReadAid;
                    bChange = TRUE;
                }

                if (bChange)
                {
                    ptbTitle->SetFlag(TbTITLE_FIELD_TITLES);
                    pTitleInfoList->aucFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
                }
            }
        }
    }
    */
}

TVOID CCommonHandleAfter::CheckPeaceTime(SSession *pstSession)
{
    if (CToolBase::IsOpCommand(pstSession->m_stReqParam.m_szCommand))
    {
        return;
    }

    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    if (pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_PEACE_TIME].m_astBuffDetail[EN_BUFF_TYPE_ITEM].m_dwTime > CTimeUtils::GetUnixTime())
    {
        TbMarch_action* ptbPrisonTimer = NULL;
        ostringstream oss;
        for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwPassiveMarchNum; udwIdx++)
        {
            if (pstUser->m_aucPassiveMarchFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
            {
                continue;
            }
            ptbPrisonTimer = &pstUser->m_atbPassiveMarch[udwIdx];
            if (ptbPrisonTimer->m_nSclass == EN_ACTION_SEC_CLASS__PRISON_TIMER
                && ptbPrisonTimer->m_nStatus == EN_MARCH_STATUS__DEFENDING)
            {
                oss.str("");
                oss << "./send_release_dragon_req.sh " << pstUser->m_tbPlayer.m_nUid << " " << ptbPrisonTimer->m_nSuid;

                CMsgBase::SendDelaySystemMsg(oss.str().c_str());
            }
        }
    }
}

TVOID CCommonHandleAfter::GenEventInfo(SSession *pstSession)
{
    TbPlayer *pstPlayer = &pstSession->m_stUserInfo.m_tbPlayer;
    TUINT32 udwAllianceId = pstSession->m_stUserInfo.m_tbAlliance.m_nAid;
    TbAlliance *pstAlliance = &pstSession->m_stUserInfo.m_tbAlliance;
    TbLogin *ptbLogin = &pstSession->m_stUserInfo.m_tbLogin;

    TINT64 ddwAlid = 0;
    TINT64 ddwAlMight = 0;
    string sAlName = "";
    string sAlNickName = "";
    TUINT32 udwAlPos = 0;
    TUINT32 udwAlGiftLv = 0;

    SCityInfo *pstCity = &pstSession->m_stUserInfo.m_stCityInfo;

    if (udwAllianceId && pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos)
    {
        ddwAlid = udwAllianceId;
        ddwAlMight = pstAlliance->m_nMight;
        sAlName = pstAlliance->m_sName;
        sAlNickName = pstAlliance->m_sAl_nick_name;
        udwAlPos = pstPlayer->m_nAlpos;
        udwAlGiftLv = CCommonBase::GetAlGiftLevel(pstAlliance);
    }

    if (pstSession->m_stUserInfo.m_bIsSendEventReq == FALSE)
    {
        return;
    }

    EventReqInfo stEventReq;

    stEventReq.SetVal(pstPlayer->m_nSid, pstPlayer->m_nUid, pstPlayer->m_sUin, pstPlayer->m_nMight,
        ddwAlid, sAlName, ddwAlMight, sAlNickName, udwAlPos,
        ptbLogin->m_nGem_buy, ptbLogin->m_nMax_buy, ptbLogin->m_nLast_buy, ptbLogin->m_nLast_buy_time, 
        ptbLogin->m_nTotal_pay, ptbLogin->m_nMax_pay, ptbLogin->m_nLast_pay,
        udwAlGiftLv, ptbLogin->m_nCtime, CCityBase::GetBuildingLevelById(&pstCity->m_stTblData, 3),
        CCityBase::GetBuildingLevelByFuncType(pstCity, EN_BUILDING_TYPE__TRIAL),
        EN_REQUEST_TYPE__GET_ALL_INFO);

    pstSession->m_sEventInfo = stEventReq.m_sReqContent;
    pstSession->m_bEventInfoOk = TRUE;
}

TINT32 CCommonHandleAfter::CheckRewardWindow(SSession *pstSession)
{
    if (pstSession->m_stCommonResInfo.m_ucJsonType != EN_JSON_TYPE_USER_JSON)
    {
        return 0;
    }

    TUINT64 uddwCurTimeMs = CTimeUtils::GetCurTimeUs() / 1000;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TUINT64 uddwLastestTime = pstUser->m_tbUserStat.m_nReward_window_time;
    for (TINT32 dwIdx = 0; dwIdx < pstUser->m_dwRewardWindowNum; dwIdx++)
    {
        if (pstUser->m_aucRewardWindowFlag[dwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            TUINT64 uddwTmpTime = pstUser->m_atbRewardWindow[dwIdx].m_nId;
            if (pstUser->m_atbRewardWindow[dwIdx].m_nShow_time != 0)
            {
                uddwTmpTime = pstUser->m_atbRewardWindow[dwIdx].m_nShow_time * 1000;
            }

            if (uddwTmpTime <= uddwCurTimeMs && uddwTmpTime > uddwLastestTime)
            {
                uddwLastestTime = uddwTmpTime;
            }

            if (uddwTmpTime > (TUINT64)pstUser->m_tbUserStat.m_nReward_window_time && uddwTmpTime <= uddwCurTimeMs)
            {
                pstUser->m_aucRewardWindowFlag[dwIdx] = EN_TABLE_UPDT_FLAG__UNCHANGE;
            }
        }
    }

    if (uddwLastestTime > (TUINT64)pstUser->m_tbUserStat.m_nReward_window_time)
    {
        pstUser->m_tbUserStat.Set_Reward_window_time(uddwLastestTime);
    }

    return 0;
}

TINT32 CCommonHandleAfter::GenEquipInfo(SSession *pstSession)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SEquipMentInfo stEquip;
    Json::Value JsonEquipList = Json::Value(Json::objectValue);
    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwEquipNum; udwIdx++)
    {
        if(pstUser->m_aucEquipFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }

        if (pstUser->m_atbEquip[udwIdx].m_nPut_on_pos == 0 || pstUser->m_atbEquip[udwIdx].m_nStatus != EN_EQUIPMENT_STATUS_ON_DRAGON)
        {
            continue;
        }
        stEquip.Reset();
        CBackpack::GetEquipInfoById(&pstUser->m_atbEquip[udwIdx], 1, pstUser->m_atbEquip[udwIdx].m_nId, &stEquip);

        Json::Value &JsonPosJson = JsonEquipList[CCommonFunc::NumToString(pstUser->m_atbEquip[udwIdx].m_nPut_on_pos)];
        JsonPosJson = Json::Value(Json::objectValue);

        JsonPosJson["e_id"] = pstUser->m_atbEquip[udwIdx].m_nId;
        JsonPosJson["id"] = stEquip.stBaseInfo.udwEType;
        JsonPosJson["lv"] = stEquip.stBaseInfo.udwLv;

        JsonPosJson["end_time"] = 0;

        for (TUINT32 udwIdx = 0; udwIdx < MAX_CRYSTAL_IN_EQUIPMENT; ++udwIdx)
        {
            JsonPosJson["crystal"].append(stEquip.stStatusInfo.audwSlot[udwIdx]);
        }

        Json::Value &EquipBuf = JsonPosJson["buff"];
        EquipBuf = Json::Value(Json::objectValue);
        for (TUINT32 udwIdx = 0; udwIdx < stEquip.stStatusInfo.udwBufferNum; ++udwIdx)
        {
            EquipBuf[CCommonFunc::NumToString(stEquip.stStatusInfo.astBuffInfo[udwIdx].m_udwId)].append(stEquip.stStatusInfo.astBuffInfo[udwIdx].m_udwId);
            EquipBuf[CCommonFunc::NumToString(stEquip.stStatusInfo.astBuffInfo[udwIdx].m_udwId)].append(stEquip.stStatusInfo.astBuffInfo[udwIdx].m_dwNum);
            EquipBuf[CCommonFunc::NumToString(stEquip.stStatusInfo.astBuffInfo[udwIdx].m_udwId)].append(stEquip.stStatusInfo.astBuffInfo[udwIdx].m_udwType);
        }
        Json::Value &EquipMistoryBuf = JsonPosJson["mistery_buff"];
        EquipMistoryBuf = Json::Value(Json::objectValue);
        for (TUINT32 udwIdx = 0; udwIdx < stEquip.stStatusInfo.udwMistoryBufferNum; ++udwIdx)
        {
            EquipMistoryBuf[CCommonFunc::NumToString(stEquip.stStatusInfo.astMisteryBuffInfo[udwIdx].m_udwId)].append(stEquip.stStatusInfo.astMisteryBuffInfo[udwIdx].m_udwId);
            EquipMistoryBuf[CCommonFunc::NumToString(stEquip.stStatusInfo.astMisteryBuffInfo[udwIdx].m_udwId)].append(stEquip.stStatusInfo.astMisteryBuffInfo[udwIdx].m_dwNum);
            EquipMistoryBuf[CCommonFunc::NumToString(stEquip.stStatusInfo.astMisteryBuffInfo[udwIdx].m_udwId)].append(stEquip.stStatusInfo.astMisteryBuffInfo[udwIdx].m_udwType);
        }
    }

    Json::FastWriter writer;
    writer.omitEndingLineFeed();
    if (writer.write(pstUser->m_tbPlayer.m_jDragon_equip) != writer.write(JsonEquipList))
    {
        pstUser->m_tbPlayer.Set_Dragon_equip(JsonEquipList);
    }

    return 0;
}

TINT32 CCommonHandleAfter::CheckUserWild(SSession *pstSession)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;

    // 检查更新wild的联盟信息
    TUINT32 udwRealAlid = ptbPlayer->m_nAlpos > 0 ? ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET : 0;
    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwWildNum; ++udwIdx)
    {
        TbMap *ptbWild = &pstUser->m_atbWild[udwIdx];
        if (ptbWild->m_nUid == ptbPlayer->m_nUid && ptbWild->m_nAlid != udwRealAlid)
        {
            ptbWild->Set_Alid(udwRealAlid);
            ptbWild->Set_Alname(ptbPlayer->m_sAlname);
            ptbWild->Set_Al_nick(ptbPlayer->m_sAl_nick_name);
            pstUser->m_aucWildFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
        }
    }

    // check city pos
    if (pstSession->m_stReqParam.m_ucLoginStatus == EN_LOGIN_STATUS__LOGIN && pstSession->m_stReqParam.m_udwCommandID == EN_CLIENT_REQ_COMMAND__LOGIN_GET)
    {
        TUINT32 udwCityNum = 0;
        TUINT32 udwCityPos = 0;

        for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwWildNum; ++udwIdx)
        {
            TbMap *ptbMap = &pstUser->m_atbWild[udwIdx];
            if (ptbMap->m_nUid == ptbPlayer->m_nUid && ptbMap->m_nType == EN_WILD_TYPE__CITY &&
                ptbMap->m_nId != 0)
            {
                ++udwCityNum;
                udwCityPos = ptbMap->m_nId;
            }
        }

        if (udwCityPos != 0)
        {
            if (pstCity->m_stTblData.m_nPos != udwCityPos)
            {
                pstCity->m_stTblData.Set_Pos(udwCityPos);
            }
            if (ptbPlayer->m_nCid != udwCityPos)
            {
                ptbPlayer->Set_Cid(udwCityPos);
            }
        }

        if (udwCityNum > 1)
        {
            for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwWildNum; ++udwIdx)
            {
                TbMap *ptbMap = &pstUser->m_atbWild[udwIdx];
                CCommonBase::AbandonWild(pstCity, ptbMap);
                CActionBase::UpdtPassiveActionWhenAbandonWild(pstUser, ptbMap);
                pstUser->m_aucWildFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
            }
        }
    }

    return 0;
}

TINT32 CCommonHandleAfter::UpdateEventTipsStat(SSession *pstSession)
{
    TBOOL bIsOp = TRUE;
    TBOOL bIsLogin = FALSE;

    CClientCmd *poCClientCmd = CClientCmd::GetInstance();
    struct SFunctionSet stProcessFunctionSet;
    map<EClientReqCommand, struct SFunctionSet>::iterator itCmdFunctionSet;
    itCmdFunctionSet = poCClientCmd->Get_CmdMap()->find((EClientReqCommand)pstSession->m_stReqParam.m_udwCommandID);
    if (itCmdFunctionSet != poCClientCmd->Get_CmdMap()->end())
    {
        stProcessFunctionSet = itCmdFunctionSet->second;

        if (stProcessFunctionSet.dwCmdType != EN_OP)
        {
            bIsOp = FALSE;
        }
    }

    if (!bIsOp && pstSession->m_stReqParam.m_ucLoginStatus == EN_LOGIN_STATUS__LOGIN)
    {
        bIsLogin = TRUE;
    }

    if (pstSession->m_stCommonResInfo.m_ucJsonType == EN_JSON_TYPE_USER_JSON)
    {
        CCommonBase::SetPlayerStateEventTipsTime(&pstSession->m_stUserInfo, bIsOp, bIsLogin);
    }

    return 0;
}

TINT32 CCommonHandleAfter::UpdateSelfTitle(SSession *pstSession)
{
    CCommonLogic::UpdateTitle(&pstSession->m_stUserInfo.m_tbPlayer, &pstSession->m_stTitleInfoList);

    return 0;
}

TINT32 CCommonHandleAfter::UpdateThroneInfo(SSession *pstSession)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbThrone *ptbThrone = &pstSession->m_tbThrone;

    if (ptbThrone->m_nAlid != 0 && pstUser->m_tbPlayer.m_nAlpos != 0
        && ptbThrone->m_nAlid == pstUser->m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET)
    {
        if (pstUser->m_tbPlayer.m_nAlpos == EN_ALLIANCE_POS__CHANCELLOR)
        {
            if (ptbThrone->m_nOwner_id != pstUser->m_tbPlayer.m_nUid)
            {
                ptbThrone->Set_Owner_id(pstUser->m_tbPlayer.m_nUid);
            }
            if (ptbThrone->m_nOwner_cid != pstUser->m_tbPlayer.m_nCid)
            {
                ptbThrone->Set_Owner_cid(pstUser->m_tbPlayer.m_nCid);
            }
        }
        TINT64 ddwReinforceNum = 0;
        TINT64 ddwReinforceTroopNum = 0;
        TINT64 ddwReinforceTroopForce = 0;
        SCommonTroop stReinforceTroop;
        stReinforceTroop.Reset();
        TbMarch_action *ptbMarch = NULL;
        for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; udwIdx++)
        {
            if (pstUser->m_aucMarchFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
            {
                continue;
            }
            ptbMarch = &pstUser->m_atbMarch[udwIdx];
            if (ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_THRONE
                && ptbMarch->m_nTpos == ptbThrone->m_nPos
                && ptbMarch->m_nStatus != EN_MARCH_STATUS__RETURNING)
            {
                ddwReinforceNum++;
                for (TUINT32 udwIdy = 0; udwIdy < EN_TROOP_TYPE__END; udwIdy++)
                {
                    stReinforceTroop[udwIdy] += ptbMarch->m_bParam[0].m_stTroop[udwIdy];
                }
            }
            else if (ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__ASSIGN_THRONE
                && ptbMarch->m_nTpos == ptbThrone->m_nPos
                && ptbMarch->m_nStatus != EN_MARCH_STATUS__RETURNING)
            {
                ddwReinforceNum++;
                for (TUINT32 udwIdy = 0; udwIdy < EN_TROOP_TYPE__END; udwIdy++)
                {
                    stReinforceTroop[udwIdy] += ptbMarch->m_bParam[0].m_stTroop[udwIdy];
                }
            }
        }
        ddwReinforceTroopNum = CToolBase::GetTroopSumNum(stReinforceTroop);
        ddwReinforceTroopForce = CToolBase::GetTroopSumForce(stReinforceTroop);

        if (ptbThrone->m_nReinforce_num != ddwReinforceNum)
        {
            ptbThrone->Set_Reinforce_num(ddwReinforceNum);
        }
        if (ptbThrone->m_nReinforce_troop_num != ddwReinforceTroopNum)
        {
            ptbThrone->Set_Reinforce_troop_num(ddwReinforceTroopNum);
        }
        if (ptbThrone->m_nReinforce_troop_force != ddwReinforceTroopForce)
        {
            ptbThrone->Set_Reinforce_troop_force(ddwReinforceTroopForce);
        }
    }

    return  0;
}

TINT32 CCommonHandleAfter::UpdateBroadcastStat(SSession* pstSession)
{
    TUINT64 uddwCurTime = 0;
    if (pstSession->m_stCommonResInfo.m_ucJsonType != EN_JSON_TYPE_USER_JSON)
    {
        return 0;
    }

    CClientCmd *poCClientCmd = CClientCmd::GetInstance();
    struct SFunctionSet stProcessFunctionSet;
    map<EClientReqCommand, struct SFunctionSet>::iterator itCmdFunctionSet;
    itCmdFunctionSet = poCClientCmd->Get_CmdMap()->find((EClientReqCommand)pstSession->m_stReqParam.m_udwCommandID);
    if (itCmdFunctionSet != poCClientCmd->Get_CmdMap()->end())
    {
        stProcessFunctionSet = itCmdFunctionSet->second;
        if (stProcessFunctionSet.dwCmdType == EN_OP)
        {
            return 0;
        }
    }

    for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_stUserInfo.m_udwBroadcastNum; udwIdx++)
    {
        TbBroadcast& tbBroadcast = pstSession->m_stUserInfo.m_atbBroadcast[udwIdx];
        if ((TUINT64)tbBroadcast.m_nCtime > uddwCurTime)
        {
            uddwCurTime = tbBroadcast.m_nCtime;
        }
    }

    if (uddwCurTime == 0)
    {
        uddwCurTime = CTimeUtils::GetCurTimeUs();
    }

    pstSession->m_stUserInfo.m_tbUserStat.Set_Broadcast_time(uddwCurTime);

    return 0;
}

TINT32 CCommonHandleAfter::CheckRallyReinforce(SSession* pstSession)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbMarch_action *ptbMarch = NULL;
    TbMarch_action *ptbRallyWar = NULL;
    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; udwIdx++)
    {
        ptbMarch = &pstUser->m_atbMarch[udwIdx];
        if (ptbMarch->m_nSuid == pstUser->m_tbPlayer.m_nUid
            && ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_REINFORCE
            && ptbMarch->m_nStatus != EN_MARCH_STATUS__RETURNING)
        {
            ptbRallyWar = CActionBase::GetMarch(pstUser->m_atbMarch, pstUser->m_udwMarchNum, ptbMarch->m_nTid);
            if (ptbRallyWar == NULL)
            {
                continue;
            }
            TBOOL bIsFind = FALSE;
            for (TUINT32 udwIdy = 0; udwIdy < ptbRallyWar->m_bRally_atk_slot.m_udwNum; udwIdy++)
            {
                if (ptbRallyWar->m_bRally_atk_slot[udwIdy].ddwMarchId == ptbMarch->m_nId)
                {
                    bIsFind = TRUE;
                    break;
                }
            }
            if (bIsFind == FALSE)
            {
                if (ptbMarch->m_nStatus == EN_MARCH_STATUS__MARCHING)
                {
                    CActionBase::ReturnMarch(ptbMarch);
                }
                else
                {
                    CActionBase::ReturnMarchOnFly(ptbMarch);
                }
                if (pstUser->m_aucMarchFlag[udwIdx] == EN_TABLE_UPDT_FLAG__UNCHANGE)
                {
                    pstUser->m_aucMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
                }
            }
        }
    }

    return 0;
}

TVOID CCommonHandleAfter::CancelPeaceTimeWhenBecomeKing(SSession *pstSession)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    if (pstSession->m_tbThrone.m_nAlid > 0 && pstSession->m_tbThrone.m_nOwner_id == pstUser->m_tbPlayer.m_nUid)
    {
        TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
        for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwActionNum; udwIdx++)
        {
            if (pstUser->m_aucActionFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
            {
                continue;
            }
            if (pstUser->m_atbAction[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__ITEM
                && pstUser->m_atbAction[udwIdx].m_bParam[0].m_stItem.m_ddwBufferId == EN_BUFFER_INFO_PEACE_TIME
                && pstUser->m_atbAction[udwIdx].m_nEtime > udwCurTime)
            {
                pstUser->m_atbAction[udwIdx].Set_Etime(udwCurTime);
                pstUser->m_aucActionFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
            }
        }
    }
}