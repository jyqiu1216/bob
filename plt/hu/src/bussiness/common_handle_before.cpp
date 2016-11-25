#include "common_handle_before.h"
#include "aws_table_include.h"
#include "game_command.h"
#include "production_system.h"
#include "quest_logic.h"
#include "buffer_base.h"
#include "common_logic.h"
#include "activities_logic.h"
#include "rating_user.h"
#include "game_evaluate_logic.h"
#include "bounty_logic.h"
#include "common_func.h"
#include "global_serv.h"
#include "action_base.h"
#include "common_base.h"
#include "player_base.h"
#include "item_base.h"
#include "tool_base.h"
#include "backpack_logic.h"
#include "msg_base.h"

// 公共命令字需要设置"pstSession->m_stCommonResInfo.m_dwRetCode"
TINT32 CCommonHandleBefore::Process_CommonHandleBefore(SSession *pstSession)
{
    TINT32 dwRetCode = 0;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;

    // 游戏评估系统需要保存的raw ex_data
    // source
    SReqInfo stReqInfo;
    stReqInfo.Reset();
    stReqInfo.SetValue(pstSession->m_stReqParam.m_udwSvrId, 0, pstSession->m_stReqParam.m_udwCityId, pstSession->m_stReqParam.m_szCommand, pstSession->m_stReqParam.m_szIdfa, pstSession->m_stReqParam.m_szKey, pstSession->m_stReqParam.m_ddwReqCost);
    CGameEvaluateLogic::SaveGameEvaluateExData(&stReqInfo, pstUser, EN_EX_DATA_USER_TYPE_SOURCE, EN_EX_DATA_TYPE_RAW);
    
    TUINT64 uddwTmpId = CTimeUtils::GetCurTimeUs();
    uddwTmpId = uddwTmpId / 1000;
    uddwTmpId = uddwTmpId - (uddwTmpId / 10000000) * 10000000;
    pstSession->m_stUserInfo.m_uddwCurEventId = uddwTmpId;

    pstUser->m_bIsSendEventReq = TRUE;

    if(EN_CLIENT_REQ_COMMAND__USER_INFO_CREATE == pstSession->m_udwCommand
        || EN_CLIENT_REQ_COMMAND__USER_INFO_RECOVER == pstSession->m_udwCommand)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("common_handle_before :init user ,so return [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
        return 0;
    }
    
    if (EN_LOGIN_STATUS__LOGIN == pstSession->m_stReqParam.m_ucLoginStatus)
    {
        pstUser->m_tbLogin.Set_Last_lg_time(CTimeUtils::GetUnixTime());
    }

    UpdateNewApnsSwitch(pstSession);

    CActionBase::CheckSeq(&pstSession->m_stUserInfo);

    CActionBase::SyncDragonStatus(pstUser);

    CheckWeekIap(pstSession);

    //联盟
    pstSession->m_stUserInfo.m_udwBeforAlid = pstSession->m_stUserInfo.m_tbPlayer.m_nAlid;
    pstSession->m_stUserInfo.m_udwBeforPlayerExp = pstSession->m_stUserInfo.m_tbPlayer.m_nExp;
    
    // 记录玩家初始vip点数
    pstSession->m_stUserInfo.m_ddwRawVipPoint = pstSession->m_stUserInfo.m_tbPlayer.m_nVip_point;
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // 军队的数量监控
    dwRetCode = TroopMonitor(pstSession);
    if(dwRetCode < 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("common_handle_before step 6 error [retcode=%d] [seq=%u]", 
            dwRetCode, \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -6;
    }
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("common_handle_before step 6 [seq=%u]", \
        pstSession->m_stUserInfo.m_udwBSeqNo));
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    //计算玩家的bonous 
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    pstSession->m_stUserInfo.m_stPlayerBuffList.Reset();
    CBufferBase::ComputeBuffInfo(pstCity, &pstSession->m_stUserInfo, pstSession->m_atbIdol, pstSession->m_udwIdolNum, &pstSession->m_tbThrone, &pstSession->m_stTitleInfoList);

    //时效装备检查
//     //目前没有时效装备
//     dwRetCode = CBackpack::CheckSoulEquip(&pstSession->m_stUserInfo, &pstSession->m_stUserInfo.m_tbPlayer);
//     if(dwRetCode != 0)
//     {
//         TSE_LOG_ERROR(pstSession->m_poServLog, ("common_handle_before :check CheckSoulEquip fail [ret=%d] [seq=%u]", dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
//         return -1;
//     }
// 
//     CBufferBase::ComputeBuffInfo(pstCity, &pstSession->m_stUserInfo, pstSession->m_atbIdol, pstSession->m_udwIdolNum, &pstSession->m_tbThrone, &pstSession->m_stTitleInfoList);

    // city信息的计算
    dwRetCode = UpdateCityInfo(pstSession);
    if(dwRetCode < 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("common_handle_before step 7 error [retcode=%d] [seq=%u]", \
            dwRetCode, \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -7;
    }
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("common_handle_before step 7 [seq=%u]", \
        pstSession->m_stUserInfo.m_udwBSeqNo));
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    CQuestLogic::CheckFirstLoginStatus(pstUser, pstCity, pstSession->m_stReqParam.m_ucLoginStatus);

    // 计算能帮助的联盟action数
    dwRetCode = CCommonLogic::ComputeCanHelpAlAction(&pstSession->m_stUserInfo);
    if(dwRetCode < 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("common_handle_before step 14 error [retcode=%d] [seq=%u]", \
            dwRetCode, \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -14;
    }
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("common_handle_before step 14 [seq=%u]", \
        pstSession->m_stUserInfo.m_udwBSeqNo));
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // 更新用户的al_gift
    dwRetCode = ComputeAllianceGift(pstSession);
    if(dwRetCode < 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("common_handle_before step 15 error [retcode=%d] [seq=%u]", \
            dwRetCode, \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -15;
    }
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("common_handle_before step 15 [seq=%u]", \
        pstSession->m_stUserInfo.m_udwBSeqNo));
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // 计算联盟里用户可援助assistance数量
    dwRetCode = ComputeCanHelpAllianceAssistance(pstSession);
    if(dwRetCode < 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("common_handle_before step 16 error [retcode=%d] [seq=%u]", \
            dwRetCode, \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -16;
    }
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("common_handle_before step 16 [seq=%u]", \
        pstSession->m_stUserInfo.m_udwBSeqNo));
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // 自动清除被盟主清除打星的mark
    dwRetCode = CCommonHandleBefore::AutoClearMarkedAlStoreItem(pstSession);
    if(dwRetCode < 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("common_handle_before step 17 error [retcode=%d] [seq=%u]", \
            dwRetCode, \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -17;
    }
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("common_handle_before step 17 [seq=%u]", \
        pstSession->m_stUserInfo.m_udwBSeqNo));
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // 更新user-stat的tips_time
    dwRetCode = UpdateUserStatTipsTime(pstSession);
    if(dwRetCode < 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("common_handle_before step 18 error [retcode=%d] [seq=%u]", \
            dwRetCode, \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -18;
    }
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("common_handle_before step 18 [seq=%u]", \
        pstSession->m_stUserInfo.m_udwBSeqNo));

    //迁服物品
    CCommonHandleBefore::CheckSvrChangeItem(pstSession);

    TSE_LOG_DEBUG(pstSession->m_poServLog, ("common_handle_before step 19 [seq=%u]", \
        pstSession->m_stUserInfo.m_udwBSeqNo));

    //check top quest to claim
    CQuestLogic::CheckIsClaim(&pstSession->m_stUserInfo, pstCity);

    TSE_LOG_DEBUG(pstSession->m_poServLog, ("common_handle_before step 20 [seq=%u]", \
        pstSession->m_stUserInfo.m_udwBSeqNo));

    //task条件更新
    CQuestLogic::CheckTaskCondition(pstUser,pstCity);

    TSE_LOG_DEBUG(pstSession->m_poServLog, ("common_handle_before step 24 [seq=%u]", \
        pstSession->m_stUserInfo.m_udwBSeqNo));

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

    TSE_LOG_DEBUG(pstSession->m_poServLog, ("common_handle_before step 25 [seq=%u]", \
        pstSession->m_stUserInfo.m_udwBSeqNo));

    // 检测rating状态
    CheckRating(pstSession);
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("common_handle_before step 26 [seq=%u]", \
        pstSession->m_stUserInfo.m_udwBSeqNo));
 
    //获取用户活动染色信息
    //GetUserDna(pstSession);
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("common_handle_before step 27 [seq=%u]", \
        pstSession->m_stUserInfo.m_udwBSeqNo));

	CheckPeaceTime(pstSession);
	TSE_LOG_DEBUG(pstSession->m_poServLog, ("common_handle_before step 32 [seq=%u]", \
		pstSession->m_stUserInfo.m_udwBSeqNo));

    GetLastTotalMight(pstSession);
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("common_handle_before step 33 [seq=%u]", \
        pstSession->m_stUserInfo.m_udwBSeqNo));

    //获取用户的kf等级
    ComputePlayerPriorityForKf(pstSession);

    //TODO 后面去掉该代码...
    for (TUINT32 udwIdx = 0; udwIdx < pstCity->m_stTblData.m_bBuilding.m_udwNum; udwIdx++)
    {
        if (pstCity->m_stTblData.m_bBuilding[udwIdx].m_ddwType == 61)
        {
            pstCity->m_stTblData.m_bBuilding[udwIdx].Reset();
            pstCity->m_stTblData.SetFlag(TbCITY_FIELD_BUILDING);
        }
    }

    return 0;
}

// function  ===> 军队的数量监控
TINT32 CCommonHandleBefore::TroopMonitor(SSession *pstSession)
{
    pstSession->m_ddwTroopBegin = CCommonBase::GetTotalMight(&pstSession->m_stUserInfo, TRUE);
    pstSession->m_bGetTroop = TRUE;

    return 0;
}

// function  ===> city信息的计算
TINT32 CCommonHandleBefore::UpdateCityInfo(SSession *pstSession)
{
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();

    SUserInfo *pstUserInfo = &pstSession->m_stUserInfo;
    CProductionSystem::ComputeProductionSystem(pstUserInfo, &pstUserInfo->m_stCityInfo, udwCurTime);

    return 0;
}

// function  ===> 更新用户的al_gift
TINT32 CCommonHandleBefore::ComputeAllianceGift(SSession *pstSession)
{
    SUserInfo* pstUserInfo = &pstSession->m_stUserInfo;
    TbPlayer* ptbPlayer = &pstSession->m_stUserInfo.m_tbPlayer;
    if(ptbPlayer->m_nAlid > 0 && ptbPlayer->m_nAlpos > EN_ALLIANCE_POS__REQUEST)
    {
        if(pstUserInfo->m_stAlGifts.m_dwGiftNum > 0)
        {
            std::sort(pstUserInfo->m_stAlGifts.m_atbGifts,
                pstUserInfo->m_stAlGifts.m_atbGifts + pstUserInfo->m_stAlGifts.m_dwGiftNum,
                CCommonHandleBefore::TbAlGift_CompareRe);
        }

        TbAl_gift_reward *ptbAlGiftReward = NULL;
        TBOOL bIsExisted = FALSE;

        for(TINT32 dwIdx = 0; dwIdx < pstUserInfo->m_stAlGifts.m_dwGiftNum && dwIdx < MAX_AL_IAP_GIFT_NUM; dwIdx++)  //更新Reward记录
        {
            if(pstUserInfo->m_udwAlGiftRewardNum >= MAX_AL_IAP_GIFT_NUM_SVR)
            {
                break;
            }

            bIsExisted = CCommonHandleBefore::IsExisted(pstUserInfo->m_atbAlGiftReward, pstUserInfo->m_udwAlGiftRewardNum, pstUserInfo->m_stAlGifts[dwIdx].m_nId);

            if(FALSE == bIsExisted)  //记录不存在
            {
                ptbAlGiftReward = &pstUserInfo->m_atbAlGiftReward[pstUserInfo->m_udwAlGiftRewardNum];

                ptbAlGiftReward->Set_Uid(pstUserInfo->m_tbPlayer.m_nUid);
                ptbAlGiftReward->Set_Gid(pstUserInfo->m_stAlGifts[dwIdx].m_nId);
                ptbAlGiftReward->Set_Status(EN_AL_GIFT_STATUS_NORMAL);
                pstUserInfo->m_udwAlGiftRewardNum++;
            }
        }
    }

    return 0;
}

// function  ===> 计算联盟里用户可援助assistance数量
TINT32 CCommonHandleBefore::ComputeCanHelpAllianceAssistance(SSession *pstSession)
{
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    SUserInfo *pstUserInfo = &pstSession->m_stUserInfo;
    TbUser_stat &tbUser_stat = pstUserInfo->m_tbUserStat;
    TbPlayer &tbPlayer = pstUserInfo->m_tbPlayer;

    for (unsigned int i = 0; i < pstUserInfo->m_udwAlAssistAllNum; ++i)
    {
        if (pstUserInfo->m_atbAlAssistAll[i].m_nTime <= tbUser_stat.m_nAs_get_t) //已经拉取过的援助,跳过
        {
            continue;
        }
        if (pstUserInfo->m_atbAlAssistAll[i].m_nUid == tbPlayer.m_nUid)//自己的援助,跳过
        {
            continue;
        }
        if(pstUserInfo->m_atbAlAssistAll[i].m_nTime < udwCurTime - CCommonBase::GetGameBasicVal(EN_GAME_BASIC_ASSIST_AVAILABLE_TIME))
        {
            continue;
        }
        pstUserInfo->m_udwAllianceCanAssistNum++;
    }

    return 0;
}

// function  ===> 更新user-stat的tips_time
TINT32 CCommonHandleBefore::UpdateUserStatTipsTime(SSession *pstSession)
{
    SUserInfo *pstUserInfo = &pstSession->m_stUserInfo;
    CCommonBase::SetPlayerStateTipsTime(pstUserInfo);

    return 0;
}

TINT32 CCommonHandleBefore::AutoClearMarkedAlStoreItem(SSession* pstSession)
{
    if(!(pstSession->m_stUserInfo.m_tbPlayer.m_nAlid > 0 &&
       pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos > EN_ALLIANCE_POS__REQUEST))
    {
        return 0;
    }
    TbAlliance& tbAlliance = pstSession->m_stUserInfo.m_tbAlliance;
    TbAlliance_Al_store_item& bAlItem = tbAlliance.m_bAl_store_item;
    TbUser_stat_Mark& bMark = pstSession->m_stUserInfo.m_tbUserStat.m_bMark;

    TUINT32 udwNum = 0;
    TBOOL bFlag = FALSE;
    for(TUINT32 udwIdx = 0; udwIdx < bMark.m_udwNum; udwIdx++)
    {
        bFlag = FALSE;
        for(TUINT32 udwIdy = 0; udwIdy < bAlItem.m_udwNum; udwIdy++)
        {
            if(bMark[udwIdx].ddwItemId == bAlItem[udwIdy].ddwItemId && bMark[udwIdx].ddwMarkTime < bAlItem[udwIdy].ddwClearTime) //打星已经给盟主清除
            {
                bFlag = TRUE;
                break;
            }
        }
        if(!bFlag)
        {
            if(udwNum < udwIdx)
            {
                bMark[udwNum] = bMark[udwIdx];
            }
            udwNum++;
        }
    }
    if(udwNum != bMark.m_udwNum)
    {
        bMark.m_udwNum = udwNum;
        pstSession->m_stUserInfo.m_tbUserStat.SetFlag(TbUSER_STAT_FIELD_MARK);
    }
    return 0;
}

TBOOL CCommonHandleBefore::TbAlGift_CompareRe(const TbAl_gift& stA, const TbAl_gift& stB)
{
    if(stA.m_nCtime > stB.m_nCtime)
    {
        return TRUE;
    }
    return FALSE;
}

TBOOL CCommonHandleBefore::IsExisted(TbAl_gift_reward* patbAlGiftRewardList, TUINT32 udwListSize, TINT64 ddwGiftId)
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

TVOID CCommonHandleBefore::CheckSvrChangeItem(SSession *pstSession)
{
    TbBackpack *pstBackpack = &pstSession->m_stUserInfo.m_tbBackpack;
    //TUINT32 udwCastleLevel = CCityBase::GetBuildingLevelByFuncType(&pstSession->m_stUserInfo.m_stCityInfo, 3);
    for(TUINT32 udwIdx = 0; udwIdx < pstBackpack->m_bItem.m_udwNum;++udwIdx)
    {
        TUINT32 udwItemId = pstBackpack->m_bItem[udwIdx].m_ddwItemId;
        //第四个时代 不允许出现333号item
        if(udwItemId == EN_ITEM_ID__SVR_CHANGE_AUTO_DEL && pstSession->m_stUserInfo.m_tbPlayer.m_nAge >= 3) //6级castle后不能再跨服移城
        {
            if(pstSession->m_stUserInfo.m_tbPlayer.m_nHas_change_svr)
            {
                //do nothing
            }
            else
            {
                CItemBase::CostItem(pstBackpack, udwItemId, 1);
                pstSession->m_stUserInfo.m_tbPlayer.Set_Has_change_svr(1);
            }
        }
    }
}

TVOID CCommonHandleBefore::CheckRating( SSession* pstSession )
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbLogin * pstLogin = &pstUser->m_tbLogin;
    CRatingUserInfo *pRatingUsreInfo = CRatingUserInfo::GetInstance();

    if (!pRatingUsreInfo->m_oJsonRoot.isMember("rating_user_list") || !pRatingUsreInfo->m_oJsonRoot["rating_user_list"].isObject())
    {
        return;
    }

    const Json::Value &stRatingRoot = pRatingUsreInfo->m_oJsonRoot["rating_user_list"];
    //TUINT32 udwRatingUserNum = stRatingRoot.size();
    //TINT32 dwRatingIdx = -1;
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();

    if (pstLogin->Get_Last_lg_time() == 0)
    {
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("CheckRating: [uid=%u] last_lg_time equal zero [lg_time=%u cur_time=%u] [seq=%u]",
            pstLogin->Get_Uid(), pstLogin->Get_Last_lg_time(), udwCurTime, pstSession->m_udwSeqNo));
        return;
    }
    if (udwCurTime < pstLogin->Get_Last_lg_time() + 120)
    {
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("CheckRating: [uid=%u] cur_time < last_lg_time+120s [lg_time=%u cur_time=%u] [seq=%u]",
            pstLogin->Get_Uid(), pstLogin->Get_Last_lg_time(), udwCurTime, pstSession->m_udwSeqNo));
        return;

    }
//     for (TUINT32 udwIdx = 0; udwIdx < udwRatingUserNum; udwIdx++)
//     {
//         if (pstLogin->Get_Uid() == stRatingRoot[udwIdx][0U].asUInt())
//         {
//             dwRatingIdx = udwIdx;
//             break;
//         }
//     }

//    if (dwRatingIdx >= 0)
    string szUid = CCommonFunc::NumToString(pstLogin->m_nUid);
    if (stRatingRoot.isMember(szUid))
    {
        TINT64 ddwNoticTime = stRatingRoot[szUid][0U].asInt();

        TSE_LOG_DEBUG(pstSession->m_poServLog, ("CheckRating:[uid=%u],[rating_switch=%u] [last_lg_time=%u] [rating_time=%u] [NoticTime=%u] [seq=%u]",
            pstLogin->Get_Uid(), pstUser->m_bRatingSwitch, pstLogin->Get_Last_lg_time(), pstLogin->Get_Rating_time(), ddwNoticTime, pstSession->m_udwSeqNo));


        if (ddwNoticTime != pstLogin->Get_Rating_time() && ddwNoticTime != 0)
        {
            pstUser->m_bRatingSwitch = TRUE;
            pstUser->m_udwRatingGem = stRatingRoot[szUid][1U].asUInt();
        }
    }

    TSE_LOG_INFO(pstSession->m_poServLog, ("CheckRating:[uid=%u],[rating_switch=%u] [last_lg_time=%u] [rewardtime=%u] [switch=%u] [seq=%u]",
        pstLogin->Get_Uid(), pstUser->m_bRatingSwitch, pstLogin->Get_Last_lg_time(), pstLogin->Get_Rating_time(), pstUser->m_bRatingSwitch, pstSession->m_udwSeqNo));
}

TVOID CCommonHandleBefore::CheckWeekIap(SSession* pstSession)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbLogin * pstLogin = &pstUser->m_tbLogin;
    
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    TUINT32 udwCurWDay = CTimeUtils::GetWDay(udwCurTime);
    //本周起始时间
    TUINT32 udwDay = udwCurTime /( 24 * 3600);
    TUINT32 udwDayBeginTime = udwDay * 24 * 3600;
    TUINT32 udwWDayBeginTime = udwDayBeginTime - udwCurWDay * 3600 * 24;
    
    if(pstLogin->Get_Last_lg_time() == 0)
    {
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("CheckWeekIap: [uid=%u] last_lg_time equal zero [lg_time=%u cur_time=%u] [seq=%u]",
            pstLogin->Get_Uid(), pstLogin->Get_Last_lg_time(), udwCurTime, pstSession->m_udwSeqNo));
        return;
    }
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("CheckWeekIap: [uid=%u] [lg_time=%u cur_time=%u w_begin_time=%u w_day=%u week_gem=%f] [seq=%u]",
        pstLogin->Get_Uid(), pstLogin->Get_Last_lg_time(), udwCurTime, udwWDayBeginTime, udwCurWDay, pstLogin->m_jWeek_gem_recharge[0].asFloat(), pstSession->m_udwSeqNo));

    if(pstLogin->m_nLast_lg_time < udwWDayBeginTime)
    {
        Json::Value jsonWeekGemRecharge = Json::Value(arrayValue);
        jsonWeekGemRecharge.append(0);
        pstLogin->Set_Week_gem_recharge(jsonWeekGemRecharge);
    }
    return;

}

TINT32 CCommonHandleBefore::GetUserDna(SSession* pstSession)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;
    TbAlliance *ptbAlliance = &pstUser->m_tbAlliance;
    TbLogin *ptbLogin = &pstUser->m_tbLogin;

    SCityInfo *pstCity = &pstUser->m_stCityInfo;

    TINT32 dwAlGiftLv = 0;
    TINT32 dwMaxAlGemBuy = 0;
    if (ptbPlayer->m_nAlid && ptbPlayer->m_nAlpos)
    {
        dwAlGiftLv = CCommonBase::GetAlGiftLevel(ptbAlliance);
        dwMaxAlGemBuy = ptbAlliance->m_nMax_gem_buy;

        if (0 == dwMaxAlGemBuy)
        {
            if (13 <= dwAlGiftLv)
            {
                dwMaxAlGemBuy = 33000;
            }
            else if (10 <= dwAlGiftLv)
            {
                dwMaxAlGemBuy = 13000;
            }
            else if (7 <= dwAlGiftLv)
            {
                dwMaxAlGemBuy = 5000;
            }
        }
    }


    TINT32 dwCastleLv = CCityBase::GetBuildingLevelById(&pstCity->m_stTblData, 3);
    

    TSE_LOG_DEBUG(pstSession->m_poServLog, ("CCommonHandleBefore::GetUserDna uid=%u castle_lv=%d gem_buy=%ld al_gift_lv=%d [seq=%u]", 
        ptbPlayer->m_nUid, dwCastleLv, ptbLogin->m_nGem_buy, dwAlGiftLv, pstSession->m_stUserInfo.m_udwBSeqNo));
    if (0 != CCommonBase::GetUserDna(ptbPlayer->m_nUid, dwCastleLv, ptbLogin->m_nGem_buy, dwAlGiftLv, dwMaxAlGemBuy, pstSession->m_stReqParam.m_udwSvrId, &pstUser->m_stUserEventGoals))
    {
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("CCommonHandleBefore::GetUserDna get user dna failed [seq=%u]",
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }
    pstUser->m_bIsSendEventReq = TRUE;
    return 0;
}

TINT32 CCommonHandleBefore::CheckPeaceTime(SSession *pstSession)
{
	SUserInfo *pstUser = &pstSession->m_stUserInfo;
	TbPlayer *ptbPlayer = &pstSession->m_stUserInfo.m_tbPlayer;

    if (pstSession->m_stReqParam.m_ucLoginStatus != EN_LOGIN_STATUS__LOGIN)
	{
		return 0;
	}

	if (CTimeUtils::GetUnixTime() > pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_PEACE_TIME].m_astBuffDetail[EN_BUFF_TYPE_ITEM].m_dwTime + 60)
	{
		if (ptbPlayer->m_nStatus & EN_CITY_STATUS__AVOID_WAR)
		{
			ptbPlayer->Set_Status(ptbPlayer->m_nStatus ^ EN_CITY_STATUS__AVOID_WAR);
		}
		if (ptbPlayer->m_nStatus & EN_CITY_STATUS__NEW_PROTECTION)
		{
			ptbPlayer->Set_Status(ptbPlayer->m_nStatus ^ EN_CITY_STATUS__NEW_PROTECTION);
		}
	}
	
	TSE_LOG_INFO(pstSession->m_poServLog, ("common_handle_before:CheckPeaceTime: uid=%u, time=[now=%u, end=%d] [seq=%u]", 
		ptbPlayer->m_nUid, CTimeUtils::GetUnixTime(), 
		pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_PEACE_TIME].m_astBuffDetail[EN_BUFF_TYPE_ITEM].m_dwTime, 
		pstSession->m_stUserInfo.m_udwBSeqNo));

	return 0;
}

TINT32 CCommonHandleBefore::UpdateNewApnsSwitch(SSession* pstSession)
{
    TbLogin *pstLogin = &pstSession->m_stUserInfo.m_tbLogin;

    if(pstSession->m_stReqParam.m_ucLoginStatus == EN_LOGIN_STATUS__LOGIN && pstSession->m_stReqParam.m_udwCommandID == EN_CLIENT_REQ_COMMAND__LOGIN_GET)
    {
        if (pstLogin->m_jApns_switch.empty())
        {
            pstLogin->m_jApns_switch = Json::Value(Json::arrayValue);
        }
        if (pstLogin->m_jApns_switch.size() < EN_NOTIC_TYPE__END)
        {
            for (TUINT32 udwIdx = pstLogin->m_jApns_switch.size(); udwIdx < EN_NOTIC_TYPE__END; udwIdx++)
            {
                pstLogin->m_jApns_switch[udwIdx] = EN_NOTIC_SWITCH_TYPE__TEXT_SOUND;
            }
            pstLogin->SetFlag(TbLOGIN_FIELD_APNS_SWITCH);
        }
    }
    return 0;
}

TVOID CCommonHandleBefore::GetLastTotalMight(SSession * pstSession)
{    
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    Json::FastWriter rWriter;
    rWriter.omitEndingLineFeed();

    pstUser->m_uddwLastMight = pstUser->m_tbUserStat.m_nLast_might;
    pstUser->m_sLastTroopAndFort = rWriter.write(pstUser->m_tbUserStat.m_jLast_troop_fort);

    Json::Value rTroopAndFort = Json::Value(Json::objectValue);
    pstUser->m_uddwCurMight = CCommonBase::GetTotalTroopAndFort(pstUser, rTroopAndFort);
    pstUser->m_sCurTroopAndFort = rWriter.write(rTroopAndFort);

    if (pstUser->m_uddwLastMight != pstUser->m_uddwCurMight)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("GetLastTotalMight: Current might and record might not match. [change=%ld] [uid=%ld][seq=%u]",
            (TINT64)pstUser->m_uddwCurMight - (TINT64)pstUser->m_uddwLastMight, pstUser->m_tbPlayer.m_nUid, pstSession->m_udwSeqNo))
    }

    TSE_LOG_DEBUG(pstSession->m_poServLog, ("GetLastTotalMight: [cur_might=%ld][last_might=%ld][cur_json=%s][last_json=%s] [uid=%ld][seq=%u]",
        pstUser->m_uddwCurMight, pstUser->m_uddwLastMight,
        pstUser->m_sCurTroopAndFort.c_str(),pstUser->m_sLastTroopAndFort.c_str(),
        pstUser->m_tbPlayer.m_nUid, pstSession->m_udwSeqNo));
}

TINT32 CCommonHandleBefore::ComputePlayerPriorityForKf( SSession * pstSession )
{
    TbLogin *ptbLogin = &pstSession->m_stUserInfo.m_tbLogin;
    if(ptbLogin->m_nUid == 0)
    {
        return 0;
    }

    if(ptbLogin->m_nTotal_pay > 0)
    {
        pstSession->m_dwKfLv = 1001;
    }

    //todo: 需要运营提供标记
}
