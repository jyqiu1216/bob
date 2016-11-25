#include "time_utils.h"
#include "player_info.h"
#include "quest_logic.h"
#include "game_info.h"
#include "tool_base.h"
#include "city_base.h"
#include "action_base.h"
#include "globalres_logic.h"
#include "player_base.h"
#include "activities_logic.h"
#include "common_func.h"
#include "common_base.h"
#include "map_logic.h"
#include "sendmessage_base.h"
#include "bounty_logic.h"
#include "wild_info.h"

TVOID CQuestLogic::CheckTimeQuestFinish(SUserInfo *pstUser, SCityInfo *pstCity, TbQuest *pstQuest)
{
    TBOOL IsFinish = FALSE;

    //auto 
    TINT32 dwIsDailyAutoFinish = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_AUTO_FINISH_DAILY].m_ddwBuffTotal;
    TINT32 dwIsAllianceAutoFinish = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_AUTO_FINISH_ALLIANCE].m_ddwBuffTotal;

    //daily quest
    CQuestLogic::CheckQuestNodeFinish(pstUser, &pstQuest->m_bDaily_quest[0], pstCity, EN_TIME_QUEST_TYPE_DAILY, dwIsDailyAutoFinish, IsFinish);
    if(IsFinish)
    {
        pstQuest->SetFlag(TbQUEST_FIELD_DAILY_QUEST);
    }

    //alliance quest 
    CQuestLogic::CheckQuestNodeFinish(pstUser, &pstQuest->m_bAl_quest[0], pstCity, EN_TIME_QUEST_TYPE_ALLIANCE,dwIsAllianceAutoFinish, IsFinish);
    if(IsFinish)
    {
        pstQuest->SetFlag(TbQUEST_FIELD_AL_QUEST);
    }

    //mistery quest
    CQuestLogic::CheckQuestNodeFinish(pstUser, &pstQuest->m_bTimer_gift[0], pstCity, EN_TIME_QUEST_TYPE_MISTERY, 0, IsFinish);
    if(IsFinish)
    {
        pstQuest->SetFlag(TbQUEST_FIELD_TIMER_GIFT);
    }

    //vip quest
    CQuestLogic::CheckQuestNodeFinish(pstUser, &pstQuest->m_bVip_quest[0], pstCity, EN_TIME_QUEST_TYPE_VIP, 0, IsFinish);
    if(IsFinish)
    {
        pstQuest->SetFlag(TbQUEST_FIELD_VIP_QUEST);
    }
}

TVOID CQuestLogic::CheckQuestNodeFinish(SUserInfo *pstUser, SQuestNode* pstQuestNode, SCityInfo *pstCity, TUINT32 udwType, TINT32 udwAuto, TBOOL &bFinish)
{
    TUINT32 udwQuestFinishTime = 0;
    bFinish = FALSE;
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();

    for(TUINT32 udwIdx = 0; udwIdx < pstQuestNode->m_ddwQuestNum; ++udwIdx)
    {
        if(udwAuto && (EN_TIME_QUEST_STATUS_DONE != pstQuestNode->m_stQuestCom[udwIdx].m_ddwStatus &&
            EN_TIME_QUEST_STATUS_AUTO_FINISH != pstQuestNode->m_stQuestCom[udwIdx].m_ddwStatus))
        {
            if(udwType == EN_TIME_QUEST_TYPE_DAILY)
            {
                pstUser->m_tbUserStat.Set_Daily_quest_finish_num(pstUser->m_tbUserStat.m_nDaily_quest_finish_num + 1);
                CQuestLogic::SetTaskCurValue(pstUser, pstCity, EN_TASK_TYPE_DAILY_QUEST_FINISH);
            }
            else if(udwType == EN_TIME_QUEST_TYPE_ALLIANCE)
            {
                pstUser->m_tbUserStat.Set_Al_quest_finish_num(pstUser->m_tbUserStat.m_nAl_quest_finish_num + 1);
                CQuestLogic::SetTaskCurValue(pstUser, pstCity, EN_TASK_TYPE_ALLIANCE_QUEST_FINISH);
            }
            else if(udwType == EN_TIME_QUEST_TYPE_VIP)
            {
                pstUser->m_tbUserStat.Set_Vip_quest_finish_num(pstUser->m_tbUserStat.m_nVip_quest_finish_num + 1);
                CQuestLogic::SetTaskCurValue(pstUser, pstCity, EN_TASK_TYPE_VIP_QUEST_FINISH);
            }

            pstQuestNode->m_stQuestCom[udwIdx].m_ddwStatus = EN_TIME_QUEST_STATUS_AUTO_FINISH;
            bFinish = TRUE;
            continue;
        }

        if(EN_TIME_QUEST_STATUS_WAIT_START == pstQuestNode->m_stQuestCom[udwIdx].m_ddwStatus
            || EN_TIME_QUEST_STATUS_DONE == pstQuestNode->m_stQuestCom[udwIdx].m_ddwStatus
            || EN_TIME_QUEST_STATUS_UNKOWN == pstQuestNode->m_stQuestCom[udwIdx].m_ddwStatus
            || EN_TIME_QUEST_STATUS_FINISH == pstQuestNode->m_stQuestCom[udwIdx].m_ddwStatus
            || EN_TIME_QUEST_STATUS_AUTO_FINISH == pstQuestNode->m_stQuestCom[udwIdx].m_ddwStatus)
        {
            continue;
        }
        udwQuestFinishTime = pstQuestNode->m_stQuestCom[udwIdx].m_ddwBTime + pstQuestNode->m_stQuestCom[udwIdx].m_ddwCTime;

        if(udwCurTime >= udwQuestFinishTime)
        {
            pstQuestNode->m_stQuestCom[udwIdx].m_ddwStatus = EN_TIME_QUEST_STATUS_FINISH;
            if(udwType == EN_TIME_QUEST_TYPE_DAILY)
            {
                pstUser->m_tbUserStat.Set_Daily_quest_finish_num(pstUser->m_tbUserStat.m_nDaily_quest_finish_num + 1);
                CQuestLogic::SetTaskCurValue(pstUser, pstCity, EN_TASK_TYPE_DAILY_QUEST_FINISH);
            }
            else if(udwType == EN_TIME_QUEST_TYPE_ALLIANCE)
            {
                pstUser->m_tbUserStat.Set_Al_quest_finish_num(pstUser->m_tbUserStat.m_nAl_quest_finish_num + 1);
                CQuestLogic::SetTaskCurValue(pstUser, pstCity, EN_TASK_TYPE_ALLIANCE_QUEST_FINISH);
            }
            else if(udwType == EN_TIME_QUEST_TYPE_VIP)
            {
                pstUser->m_tbUserStat.Set_Vip_quest_finish_num(pstUser->m_tbUserStat.m_nVip_quest_finish_num + 1);
                CQuestLogic::SetTaskCurValue(pstUser, pstCity, EN_TASK_TYPE_VIP_QUEST_FINISH);
            }

            if(FALSE == bFinish)
            {
                bFinish = TRUE;
            }
        }
    }
}


TVOID CQuestLogic::NewCheckTimeQuestRefresh(SUserInfo *pstUserInfo, SCityInfo *pstCityInfo, TbQuest *pstQuest)
{
    if(pstCityInfo == NULL 
       || pstUserInfo == NULL)
    {
        return;
    }

    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    
    // check daily
    // 刷新时间已到
    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("NewCheckTimeQuestRefresh: [RefreshDailyTime=%lld] [udwCurTime=%d] [questnum=%lld] [seq=%u]",
                                                      pstQuest->m_bDaily_quest[0].m_ddwRTime, \
                                                      udwCurTime, \
                                                      pstQuest->m_bDaily_quest[0].m_ddwQuestNum, \
                                                      pstUserInfo->m_udwBSeqNo));
    
    if (pstQuest->m_bDaily_quest[0].m_ddwRTime < static_cast<TINT64>(udwCurTime))
    {
        CQuestLogic::NewRefreshTimequest(pstUserInfo, pstCityInfo, EN_TIME_QUEST_TYPE_DAILY, &pstQuest->m_bDaily_quest[0], EN_QUEST_REFRESH_TYPE_NORMAL);
        TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("NewCheckTimeQuestRefresh: RefreshDaily uid=%ld, type=%d, num=%d [seq=%u]",
            pstUserInfo->m_tbLogin.m_nUid, EN_TIME_QUEST_TYPE_DAILY, pstUserInfo->m_dwRefreshDailyQuestNum, pstUserInfo->m_udwBSeqNo));
    }

    
    // check alliance
    // in_alliance/刷新时间已到
    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("NewCheckTimeQuestRefresh: [RefreshAllianceTime=%lld] [udwCurTime=%d] [questnum=%lld] [seq=%u]",
                                                      pstQuest->m_bAl_quest[0].m_ddwRTime, \
                                                      udwCurTime, \
                                                      pstQuest->m_bAl_quest[0].m_ddwQuestNum, \
                                                      pstUserInfo->m_udwBSeqNo));
    if (pstUserInfo->m_tbPlayer.m_nAlpos > 0 
        && pstQuest->m_bAl_quest[0].m_ddwRTime < static_cast<TINT64>(udwCurTime))
    {        
        CQuestLogic::NewRefreshTimequest(pstUserInfo, pstCityInfo, EN_TIME_QUEST_TYPE_ALLIANCE, &pstQuest->m_bAl_quest[0], EN_QUEST_REFRESH_TYPE_NORMAL);
        TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("NewCheckTimeQuestRefresh: RefreshDaily uid=%ld, type=%d, num=%d [seq=%u]",
            pstUserInfo->m_tbLogin.m_nUid, EN_TIME_QUEST_TYPE_ALLIANCE, pstUserInfo->m_dwRefreshAllianceQuestNum, pstUserInfo->m_udwBSeqNo));
    }
    
    // check vip
    // vip/vip_quest为0/刷新时间已到
    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("NewCheckTimeQuestRefresh: [RefreshVipTime=%lld] [udwCurTime=%d] [questnum=%lld] [seq=%u]",
                                                      pstQuest->m_bVip_quest[0].m_ddwRTime, \
                                                      udwCurTime, \
                                                      pstQuest->m_bVip_quest[0].m_ddwQuestNum, \
                                                      pstUserInfo->m_udwBSeqNo));
    if ((CPlayerBase::GetVipLevel(&pstUserInfo->m_tbPlayer) > 0 && pstQuest->m_bVip_quest[0].m_ddwRTime < static_cast<TINT64>(udwCurTime))
       || (CPlayerBase::GetVipLevel(&pstUserInfo->m_tbPlayer) > 0 && pstQuest->m_bVip_quest[0].m_ddwQuestNum == 0))
    {
        CQuestLogic::NewRefreshTimequest(pstUserInfo, pstCityInfo, EN_TIME_QUEST_TYPE_VIP, &pstQuest->m_bVip_quest[0], EN_QUEST_REFRESH_TYPE_NORMAL);
        TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("NewCheckTimeQuestRefresh: RefreshDaily uid=%ld, type=%d, num=%d [seq=%u]",
            pstUserInfo->m_tbLogin.m_nUid, EN_TIME_QUEST_TYPE_VIP, pstUserInfo->m_dwRefreshVipQuestNum, pstUserInfo->m_udwBSeqNo));
    }

}

TVOID CQuestLogic::NewRefreshTimequest(SUserInfo *pstUser, SCityInfo * pstCityInfo, TUINT32 udwQuestType, SQuestNode* pstQuestNode, TINT32 dwRefreshType)
{
    //TINT32 dwIsDailyAutoFinish = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_AUTO_FINISH_DAILY].m_ddwBuffTotal;
    //TINT32 dwIsAllianceAutoFinish = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_AUTO_FINISH_ALLIANCE].m_ddwBuffTotal;

    TUINT32 udwRefreshNum = 0;
    if(EN_TIME_QUEST_TYPE_MISTERY == udwQuestType
        || EN_TIME_QUEST_TYPE_NEW_USER_MISTERY == udwQuestType)
    {
        udwRefreshNum = 1;
    }
    else
    {
        udwRefreshNum = MAX_DAILY_ALLIANCE_VIP_QUEST_NUM;
    }

    pstUser->m_dwRefreshpQuesType = dwRefreshType;
    
    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("NewRefreshTimequest: [udwRefreshNum=%ld] [seq=%u]",
                                                      udwRefreshNum, \
                                                      pstUser->m_udwBSeqNo));
    for(TUINT32 udwIdx = 0; udwIdx < udwRefreshNum; ++udwIdx)
    {

        TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("NewRefreshTimequest: [QuestType=%ld] [QuestStatus=%ld] [seq=%u]",
                                                           udwQuestType, \
                                                           pstQuestNode->m_stQuestCom[udwIdx].m_ddwStatus, \
                                                           pstUser->m_udwBSeqNo));
    
        //对于running 和 finish的questnode 不刷新
        if(pstQuestNode->m_stQuestCom[udwIdx].m_ddwStatus == EN_TIME_QUEST_STATUS_RUNNING
           || pstQuestNode->m_stQuestCom[udwIdx].m_ddwStatus == EN_TIME_QUEST_STATUS_FINISH
           || pstQuestNode->m_stQuestCom[udwIdx].m_ddwStatus == EN_TIME_QUEST_STATUS_AUTO_FINISH)
        {
            continue;
        }
        
        if(EN_TIME_QUEST_TYPE_DAILY == udwQuestType 
           || EN_TIME_QUEST_TYPE_NEW_USER_DAILY == udwQuestType)
        {
            pstUser->m_dwRefreshDailyQuestNum++;
        }
        else if(EN_TIME_QUEST_TYPE_ALLIANCE == udwQuestType)
        {
            pstUser->m_dwRefreshAllianceQuestNum++;
        }
        else if(EN_TIME_QUEST_TYPE_VIP == udwQuestType)
        {
            pstUser->m_dwRefreshVipQuestNum++;
        }
    }
    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("NewRefreshTimequest: [RefreshDailyQuestNum=%ld], [RefreshAllianceQuestNum=%ld], [RefreshVipQuestNum=%ld] [seq=%u]",
                                                      pstUser->m_dwRefreshDailyQuestNum, \
                                                      pstUser->m_dwRefreshAllianceQuestNum, \
                                                      pstUser->m_dwRefreshVipQuestNum, \
                                                      pstUser->m_udwBSeqNo));

    
    return;

}


TVOID CQuestLogic::NewGenQuestNum(SQuestNode* pstQuestNode, SQuestListInfo *pstQuestListInfo, TINT32 dwCurQuestNum, TUINT32 udwCurTime, TUINT32 udwUserCreateTime)
{   
    TUINT32 udwRefreshTime = CGameInfo::GetInstance()->m_oJsonRoot["game_basic"][EN_GAME_BASIC_QUEST_REFRESH_CYCLE].asUInt();
    TUINT32 udwCount = 1 + (udwCurTime - udwUserCreateTime)/udwRefreshTime;

    pstQuestNode->m_ddwRTime = udwUserCreateTime + udwCount * udwRefreshTime;

    pstQuestNode->m_ddwQuestNum = dwCurQuestNum;
    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("NewGenQuestNum: [dwCurQuestNum:%ld]",
                                                      pstQuestNode->m_ddwQuestNum));

    
    for(TUINT32 udwIdx = 0, udwIdy = 0; udwIdx < MAX_DAILY_ALLIANCE_VIP_QUEST_NUM; ++udwIdx)
    {    

        //对于running 和 finish的questnode 不刷新
        if(pstQuestNode->m_stQuestCom[udwIdx].m_ddwStatus == EN_TIME_QUEST_STATUS_RUNNING 
           || pstQuestNode->m_stQuestCom[udwIdx].m_ddwStatus == EN_TIME_QUEST_STATUS_FINISH
           || pstQuestNode->m_stQuestCom[udwIdx].m_ddwStatus == EN_TIME_QUEST_STATUS_AUTO_FINISH)
        {
            continue;
        }
        else
        {   

            TUINT32 udwRewardIdx = 0;
            if(udwIdy < pstQuestListInfo->m_vecQuestList.size())
            {
                udwRewardIdx = udwIdy;
            }
            else
            {
                udwRewardIdx = pstQuestListInfo->m_vecQuestList.size() - 1;
            }
            
            TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("NewGenQuestNum: [QuestLevel:%ld] [CostTime=%ld]", \
                                                              pstQuestListInfo->m_vecQuestList[udwRewardIdx]->m_dwLevel, \
                                                              pstQuestListInfo->m_vecQuestList[udwRewardIdx]->m_dwCostTime));
            

            pstQuestNode->m_stQuestCom[udwIdx].m_ddwBTime = CTimeUtils::GetUnixTime();
            pstQuestNode->m_stQuestCom[udwIdx].m_ddwStatus = EN_TIME_QUEST_STATUS_WAIT_START;

              
            pstQuestNode->m_stQuestCom[udwIdx].m_ddwLv = pstQuestListInfo->m_vecQuestList[udwRewardIdx]->m_dwLevel;
            pstQuestNode->m_stQuestCom[udwIdx].m_ddwCTime = pstQuestListInfo->m_vecQuestList[udwRewardIdx]->m_dwCostTime;
            pstQuestNode->m_stQuestCom[udwIdx].m_stReward.ddwTotalNum = 0;
            for (TUINT32 udwIdz = 0; udwIdz < pstQuestListInfo->m_vecQuestList[udwRewardIdx]->m_vecReward.size(); ++udwIdz)
            {
                pstQuestNode->m_stQuestCom[udwIdx].m_stReward.aRewardList[udwIdz].ddwType = pstQuestListInfo->m_vecQuestList[udwRewardIdx]->m_vecReward[udwIdz]->ddwType;
                pstQuestNode->m_stQuestCom[udwIdx].m_stReward.aRewardList[udwIdz].ddwId = pstQuestListInfo->m_vecQuestList[udwRewardIdx]->m_vecReward[udwIdz]->ddwId;
                pstQuestNode->m_stQuestCom[udwIdx].m_stReward.aRewardList[udwIdz].ddwNum = pstQuestListInfo->m_vecQuestList[udwRewardIdx]->m_vecReward[udwIdz]->ddwNum;
                pstQuestNode->m_stQuestCom[udwIdx].m_stReward.ddwTotalNum++;
                if(MAX_REWARD_ITEM_NUM <= pstQuestNode->m_stQuestCom[udwIdx].m_stReward.ddwTotalNum)
                {
                    break;
                }
            }
            ++udwIdy;
        }
    }



    for(TUINT32 udwIdx = pstQuestNode->m_ddwQuestNum, udwIdy = 0; udwIdx < MAX_DAILY_ALLIANCE_VIP_QUEST_NUM;)
    {
        if(udwIdy < pstQuestNode->m_ddwQuestNum)
        {        
            if(pstQuestNode->m_stQuestCom[udwIdx].m_ddwStatus == EN_TIME_QUEST_STATUS_FINISH
                || pstQuestNode->m_stQuestCom[udwIdx].m_ddwStatus == EN_TIME_QUEST_STATUS_AUTO_FINISH)
            {
                if(pstQuestNode->m_stQuestCom[udwIdy].m_ddwStatus == EN_TIME_QUEST_STATUS_WAIT_START)
                {
                    pstQuestNode->m_stQuestCom[udwIdy] = pstQuestNode->m_stQuestCom[udwIdx];
                    pstQuestNode->m_stQuestCom[udwIdx].m_ddwStatus = EN_TIME_QUEST_STATUS_DONE;
                    udwIdx++;
                    udwIdy++;
                }
                else
                {
                    udwIdy++;
                }
            }
            else
            {
                udwIdx++;
            }
        }
        else
        {
            pstQuestNode->m_stQuestCom[udwIdx].m_ddwStatus = EN_TIME_QUEST_STATUS_DONE;
            udwIdx++;
        }
    }


    /*
    for(TUINT32 udwIdx = pstQuestNode->m_ddwQuestNum, udwIdy = 0; udwIdx < MAX_DAILY_ALLIANCE_VIP_QUEST_NUM && udwIdy < pstQuestNode->m_ddwQuestNum;)
    {
        if(pstQuestNode->m_stQuestCom[udwIdx].m_ddwStatus == EN_TIME_QUEST_STATUS_FINISH
            || pstQuestNode->m_stQuestCom[udwIdx].m_ddwStatus == EN_TIME_QUEST_STATUS_AUTO_FINISH)
        {
            if(pstQuestNode->m_stQuestCom[udwIdy].m_ddwStatus == EN_TIME_QUEST_STATUS_WAIT_START)
            {
                pstQuestNode->m_stQuestCom[udwIdy] = pstQuestNode->m_stQuestCom[udwIdx];
                pstQuestNode->m_stQuestCom[udwIdx].m_ddwStatus = EN_TIME_QUEST_STATUS_DONE;
                udwIdx++;
                udwIdy++;
            }
            else
            {
                udwIdy++;
            }
        }
        else
        {
            udwIdx++;
        }
    }
    */
}



TVOID CQuestLogic::CheckTimeQuestRefresh(SUserInfo *pstUserInfo, SCityInfo *pstCityInfo, TbQuest *pstQuest)
{
    if(pstCityInfo == NULL || pstUserInfo == NULL)
    {
        return;
    }
    //get refresh time
    TUINT32 udwRefreshTime = CGameInfo::GetInstance()->m_oJsonRoot["game_basic"][EN_GAME_BASIC_QUEST_REFRESH_CYCLE].asUInt();
    //check daily
    if (pstQuest->m_bDaily_quest[0].m_ddwRTime < CTimeUtils::GetUnixTime())
    {
        //打破init daily 等级
        TUINT32 udwBreakInitDailyLv = CGameInfo::GetInstance()->m_oJsonRoot["game_basic"][30U].asUInt();
        TUINT32 udwCastleLv = CCityBase::GetBuildingLevelByFuncType(pstCityInfo, EN_BUILDING_TYPE__CASTLE);
        if(udwCastleLv < udwBreakInitDailyLv)
        {
            CQuestLogic::RefreshTimequest(pstUserInfo, pstCityInfo, EN_TIME_QUEST_TYPE_NEW_USER_DAILY, &pstQuest->m_bDaily_quest[0], TRUE);
        }
        else
        {
            CQuestLogic::RefreshTimequest(pstUserInfo, pstCityInfo, EN_TIME_QUEST_TYPE_DAILY, &pstQuest->m_bDaily_quest[0]);
        }
        pstQuest->m_bDaily_quest[0].m_ddwRTime = CTimeUtils::GetUnixTime() + udwRefreshTime;
        pstQuest->SetFlag(TbQUEST_FIELD_DAILY_QUEST);

    }
    //check alliance
    if (pstUserInfo->m_tbPlayer.m_nAlpos > 0 && pstQuest->m_bAl_quest[0].m_ddwRTime < CTimeUtils::GetUnixTime())
    {
        CQuestLogic::RefreshTimequest(pstUserInfo, pstCityInfo, EN_TIME_QUEST_TYPE_ALLIANCE, &pstQuest->m_bAl_quest[0]);

        pstQuest->m_bAl_quest[0].m_ddwRTime = CTimeUtils::GetUnixTime() + udwRefreshTime;
        pstQuest->SetFlag(TbQUEST_FIELD_AL_QUEST);

    }
    //check mistery
    if(pstQuest->m_bTimer_gift[0].m_stQuestCom[0].m_ddwStatus < EN_TIME_QUEST_STATUS_RUNNING)
    {
        if(pstUserInfo->m_tbPlayer.m_nStatus & EN_CITY_STATUS__NEW_USER_MISTERY)
        {
            CQuestLogic::RefreshTimequest(pstUserInfo, pstCityInfo, EN_TIME_QUEST_TYPE_NEW_USER_MISTERY, &pstQuest->m_bTimer_gift[0]);
        }
        else
        {
            // CQuestLogic::RefreshTimequest(pstUserInfo, pstCityInfo, EN_TIME_QUEST_TYPE_MISTERY, &pstQuest->m_bTimer_gift[0]);
        }

        pstQuest->m_bTimer_gift[0].m_ddwRTime = CTimeUtils::GetUnixTime();
        pstQuest->SetFlag(TbQUEST_FIELD_TIMER_GIFT);
    }

    //check vip
    if ((CPlayerBase::GetVipLevel(&pstUserInfo->m_tbPlayer) > 0 && pstQuest->m_bVip_quest[0].m_ddwRTime < CTimeUtils::GetUnixTime()) ||
        (CPlayerBase::GetVipLevel(&pstUserInfo->m_tbPlayer) > 0 && pstQuest->m_bVip_quest[0].m_ddwQuestNum == 0))
    {
        CQuestLogic::RefreshTimequest(pstUserInfo, pstCityInfo, EN_TIME_QUEST_TYPE_VIP, &pstQuest->m_bVip_quest[0]);
        pstQuest->m_bVip_quest[0].m_ddwRTime = CTimeUtils::GetUnixTime() + udwRefreshTime;
        pstQuest->SetFlag(TbQUEST_FIELD_VIP_QUEST);
    }

    return;
}

TVOID CQuestLogic::RefreshTimequest(SUserInfo *pstUser, SCityInfo * pstCityInfo, TUINT32 udwQuestType, SQuestNode* pstQuestNode, TBOOL bIsInitDaily /* = FALSE*/)
{
    //1 get quest num
    TINT32 dwQuestNum = 0;
    if(udwQuestType == EN_TIME_QUEST_TYPE_DAILY || EN_TIME_QUEST_TYPE_NEW_USER_DAILY == udwQuestType)
    {

        dwQuestNum = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_DAILY_QUEST_NUM].m_ddwBuffTotal;

    }
    else if(udwQuestType == EN_TIME_QUEST_TYPE_ALLIANCE)
    {
        dwQuestNum = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_ALLIANCE_NUM].m_ddwBuffTotal;

    }
    else if(udwQuestType == EN_TIME_QUEST_TYPE_VIP)
    {
        dwQuestNum = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_VIP_QUEST_NUM].m_ddwBuffTotal;
    }
    else if(udwQuestType == EN_TIME_QUEST_TYPE_MISTERY || udwQuestType == EN_TIME_QUEST_TYPE_NEW_USER_MISTERY)
    {
        dwQuestNum = 1;
    }

    if(dwQuestNum > MAX_DAILY_ALLIANCE_VIP_QUEST_NUM)
    {
        dwQuestNum = MAX_DAILY_ALLIANCE_VIP_QUEST_NUM;
    }

    pstQuestNode->m_ddwQuestNum = dwQuestNum;

    TUINT32 udwRefreshNum = 0;
    if(udwQuestType == EN_TIME_QUEST_TYPE_MISTERY || udwQuestType == EN_TIME_QUEST_TYPE_NEW_USER_MISTERY)
    {
        udwRefreshNum = 1;
    }
    else
    {
        udwRefreshNum = MAX_DAILY_ALLIANCE_VIP_QUEST_NUM;
    }
    //移动
    
    TUINT32 udwCastleLv = CCityBase::GetBuildingLevelByFuncType(pstCityInfo, EN_BUILDING_TYPE__CASTLE);
    for(TUINT32 udwIdx = 0; udwIdx < udwRefreshNum; ++udwIdx)
    {
        //对于running 和 finish的questnode 不刷新
        if(pstQuestNode->m_stQuestCom[udwIdx].m_ddwStatus == EN_TIME_QUEST_STATUS_RUNNING ||
            pstQuestNode->m_stQuestCom[udwIdx].m_ddwStatus == EN_TIME_QUEST_STATUS_FINISH ||
            pstQuestNode->m_stQuestCom[udwIdx].m_ddwStatus == EN_TIME_QUEST_STATUS_AUTO_FINISH)
        {
            continue;
        }
        if(!bIsInitDaily)
        {
            CQuestLogic::GenQuestNode(udwQuestType, udwCastleLv, pstQuestNode->m_ddwCollectNum, &pstQuestNode->m_stQuestCom[udwIdx]);
        }
        else
        {
            CQuestLogic::GenInitDailyQuestNode(udwQuestType, udwCastleLv, pstQuestNode->m_ddwCollectNum, &pstQuestNode->m_stQuestCom[udwIdx], udwIdx);
        }

    }

    for(TUINT32 udwIdx = pstQuestNode->m_ddwQuestNum, udwIdy = 0; udwIdx < MAX_DAILY_ALLIANCE_VIP_QUEST_NUM && udwIdy < pstQuestNode->m_ddwQuestNum;)
    {
        if(pstQuestNode->m_stQuestCom[udwIdx].m_ddwStatus == EN_TIME_QUEST_STATUS_FINISH
            || pstQuestNode->m_stQuestCom[udwIdx].m_ddwStatus == EN_TIME_QUEST_STATUS_AUTO_FINISH)
        {
            if(pstQuestNode->m_stQuestCom[udwIdy].m_ddwStatus == EN_TIME_QUEST_STATUS_WAIT_START)
            {
                pstQuestNode->m_stQuestCom[udwIdy] = pstQuestNode->m_stQuestCom[udwIdx];
                pstQuestNode->m_stQuestCom[udwIdx].m_ddwStatus = EN_TIME_QUEST_STATUS_DONE;
                udwIdx++;
                udwIdy++;
            }
            else
            {
                udwIdy++;
            }
        }
        else
        {
            udwIdx++;
        }
    }
    return;
}

TVOID CQuestLogic::GenQuestNode(TUINT32 udwQuestType, TUINT32 udwCastleLv, TUINT32 udwTimeGiftCollectNum, SQuestComm *pstQuestComm)
{
    CGameInfo *pstGameInfo = CGameInfo::GetInstance();

    pstQuestComm->Reset();
    pstQuestComm->m_ddwStatus = EN_TIME_QUEST_STATUS_WAIT_START;
    pstQuestComm->m_ddwBTime = 0;

    TCHAR szQuestType[64];
    snprintf(szQuestType, 64, "%u", udwQuestType);
    if(!pstGameInfo->m_oJsonRoot["game_sub_quest"].isMember(szQuestType))
    {
        return;
    }
    const Json::Value &oQuestNode = pstGameInfo->m_oJsonRoot["game_sub_quest"][szQuestType];

    //quest lv 从一级开始
    TUINT32 udwQuestLv = 0;
    TUINT32 udwTotalLvRate = 0;
    for(TUINT32 udwIdx = 0; udwIdx < oQuestNode["a1"].size(); ++udwIdx)
    {
        udwTotalLvRate += oQuestNode["a1"][udwIdx].asUInt();
    }
    assert(udwTotalLvRate != 0);

    TUINT32 udwRandon = 0;
    if(udwQuestType == EN_TIME_QUEST_TYPE_MISTERY || udwQuestType == EN_TIME_QUEST_TYPE_NEW_USER_MISTERY)
    {
        udwRandon = udwTimeGiftCollectNum;
        pstQuestComm->m_ddwBTime = CTimeUtils::GetUnixTime();
        pstQuestComm->m_ddwStatus = EN_TIME_QUEST_STATUS_RUNNING;
    }
    else
    {
        udwRandon = rand() % udwTotalLvRate;
    }

    udwTotalLvRate = 0;
    for(TUINT32 udwIdx = 0; udwIdx < oQuestNode["a1"].size(); ++udwIdx)
    {
        udwTotalLvRate += oQuestNode["a1"][udwIdx].asUInt();
        if(udwRandon < udwTotalLvRate)
        {
            udwQuestLv = udwIdx;
            break;
        }
    }
    pstQuestComm->m_ddwLv = udwQuestLv + 1;

    //虽然配置上保证了这种情况的出现 但加上保护
    if(udwQuestType == EN_TIME_QUEST_TYPE_MISTERY && udwRandon >= udwTotalLvRate)
    {
        pstQuestComm->m_ddwLv = oQuestNode["a1"].size();
    }

    //quest cost time
    TUINT32 udwMinTime = oQuestNode["a2"][(TUINT32)pstQuestComm->m_ddwLv - 1]["a0"][0U].asUInt();
    TUINT32 udwMaxTime = oQuestNode["a2"][(TUINT32)pstQuestComm->m_ddwLv - 1]["a0"][1U].asUInt();
    TUINT32 udwCostTime = CToolBase::GetRandNumber(udwMinTime, udwMaxTime);
    pstQuestComm->m_ddwCTime = udwCostTime;

    //reward
    CGlobalResLogic::GetTimeQuestReward(udwQuestType, pstQuestComm->m_ddwLv, udwCastleLv, &pstQuestComm->m_stReward);
}

TVOID CQuestLogic::GenInitDailyQuestNode(TUINT32 udwQuestType, TUINT32 udwCastleLv, TUINT32 udwTimeGiftCollectNum, SQuestComm *pstQuestComm, TUINT32 udwQuestIdx)
{
    CGameInfo *pstGameInfo = CGameInfo::GetInstance();

    pstQuestComm->Reset();
    pstQuestComm->m_ddwStatus = EN_TIME_QUEST_STATUS_WAIT_START;
    pstQuestComm->m_ddwBTime = 0;

    TCHAR szQuestType[64];
    snprintf(szQuestType, 64, "%u", udwQuestType);
    if(!pstGameInfo->m_oJsonRoot["game_sub_quest"].isMember(szQuestType))
    {
        return;
    }
    const Json::Value &oQuestNode = pstGameInfo->m_oJsonRoot["game_sub_quest"][szQuestType];


    //根据idx计算出此时的任务节点
    TUINT32 udwTotal = 0;
    TUINT32 udwTmp = 0;
    for(TUINT32 udwIdxReward = 0; udwIdxReward < oQuestNode["a0"].size(); ++udwIdxReward)
    {

        if(oQuestNode["a0"][udwIdxReward].asUInt() == 0)
        {
            continue;
        }
        udwTotal += oQuestNode["a0"][udwIdxReward].asUInt();
        if(udwQuestIdx < udwTotal)
        {
            udwTmp = udwIdxReward;
            break;
        }
    }
    pstQuestComm->m_ddwLv = udwTmp + 1;

    //quest cost time
    TUINT32 udwMinTime = oQuestNode["a2"][(TUINT32)pstQuestComm->m_ddwLv - 1]["a0"][0U].asUInt();
    TUINT32 udwMaxTime = oQuestNode["a2"][(TUINT32)pstQuestComm->m_ddwLv - 1]["a0"][1U].asUInt();
    TUINT32 udwCostTime = CToolBase::GetRandNumber(udwMinTime, udwMaxTime);
    pstQuestComm->m_ddwCTime = udwCostTime;


    //reward
    CGlobalResLogic::GetTimeQuestReward(udwQuestType, pstQuestComm->m_ddwLv, udwCastleLv, &pstQuestComm->m_stReward);
}

TVOID CQuestLogic::TestFinishBeginQuest(SUserInfo *pstUser, TUINT32 udwQuestType, TUINT32 udwTargetId, TUINT32 udwLv)
{
    //判断是否完成某个任务
    for(TUINT32 udwIdx = 0; udwIdx < EN_TOP_QUEST_NUM_LIMIT; ++udwIdx)
    {
        // 0.判断任务是否已经收集
        if(BITTEST(pstUser->m_tbUserStat.m_bTop_quest[0].m_bitQuest, udwIdx))
        {
            continue;
        }
        //1 判断是否已完成
        if (BITTEST(pstUser->m_tbUserStat.m_bTop_quest_finish[0].m_bitQuest, udwIdx))
        {
            continue;
        }

        TCHAR szBuf[1024];
        snprintf(szBuf, 1024, "%u", udwIdx);
        if(!CGameInfo::GetInstance()->m_oJsonRoot["game_quest"].isMember(szBuf))
        {
            continue;
        }
        if(CGameInfo::GetInstance()->m_oJsonRoot["game_quest"][szBuf].isNull())
        {
            continue;
        }

        TUINT32 udwType = CGameInfo::GetInstance()->m_oJsonRoot["game_quest"][szBuf]["o"]["a0"].asUInt();
        TUINT32 udwId = CGameInfo::GetInstance()->m_oJsonRoot["game_quest"][szBuf]["o"]["a1"].asUInt();
        if(udwType != udwQuestType)
        {
            continue;
        }
        if(udwId != udwTargetId)
        {
            continue;
        }
        TUINT32 udwTargetValue = CGameInfo::GetInstance()->m_oJsonRoot["game_quest"][szBuf]["o"]["a2"].asUInt();

        if(udwTargetValue <= udwLv)
        {
            BITSET(pstUser->m_tbUserStat.m_bTop_quest_finish[0].m_bitQuest, udwIdx);
            pstUser->m_tbUserStat.SetFlag(TbUSER_STAT_FIELD_TOP_QUEST_FINISH);
        }
    }
}

TVOID CQuestLogic::HasFinishQuestNode(SQuestNode* pstQuestNode, TBOOL &bFinish)
{
    bFinish = FALSE;

    for (TUINT32 udwIdx = 0; udwIdx < pstQuestNode->m_ddwQuestNum; ++udwIdx)
    {
        if (pstQuestNode->m_stQuestCom[udwIdx].m_ddwStatus == EN_TIME_QUEST_STATUS_FINISH ||
            pstQuestNode->m_stQuestCom[udwIdx].m_ddwStatus == EN_TIME_QUEST_STATUS_AUTO_FINISH)
        {
            bFinish = TRUE;
        }
    }
}

TUINT32 CQuestLogic::GetDoneQuestNum(SUserInfo *pstUser, TUINT32 udwType)
{
    TUINT32 udwKingdomQuestNum = 0;
    for(TUINT32 udwIdx = 0; udwIdx<EN_TOP_QUEST_NUM_LIMIT; ++udwIdx)
    {
        // 0.判断任务是否已经收集
        if(BITTEST(pstUser->m_tbUserStat.m_bTop_quest[0].m_bitQuest, udwIdx))
        {
            udwKingdomQuestNum++;
        }
    }
    //1表示daily quest，2 表示alliance quest，3kingdom quest加上daily quest加上alliance quest的总和
    switch(udwType)
    {
    case EN_COLLECT_QUEST_DAILY:
        return pstUser->m_tbQuest.m_bDaily_quest[0].m_ddwCollectNum;
        break;
    case EN_COLLECT_QUEST_ALLIANCE:
        return pstUser->m_tbQuest.m_bAl_quest[0].m_ddwCollectNum;
        break;
    case EN_COLLECT_QUEST_KINGDOMQUEST:
        return udwKingdomQuestNum + pstUser->m_tbQuest.m_bDaily_quest[0].m_ddwCollectNum + pstUser->m_tbQuest.m_bAl_quest[0].m_ddwCollectNum;;
        break;
    default:
        return 0;
    }
    return 0;
}

TVOID CQuestLogic::CheckIsClaim(SUserInfo *pstUserInfo, SCityInfo *pstCity)
{
    STopQuest& stQuest = pstUserInfo->m_tbUserStat.m_bTop_quest[0];
    for(TUINT32 udwIdx = 0; udwIdx < EN_TOP_QUEST_NUM_LIMIT; ++udwIdx)
    {
        if(!CGameInfo::GetInstance()->m_oJsonRoot["game_quest"].isMember(CCommonFunc::NumToString(udwIdx)))
        {
            BITCLEAR(stQuest.m_bitQuest, udwIdx);
            BITCLEAR(pstUserInfo->m_tbUserStat.m_bTop_quest_finish[0].m_bitQuest, udwIdx);
            pstUserInfo->m_tbUserStat.SetFlag(TbUSER_STAT_FIELD_TOP_QUEST);
            pstUserInfo->m_tbUserStat.SetFlag(TbUSER_STAT_FIELD_TOP_QUEST_FINISH);
            continue;
        }
        if(BITTEST(stQuest.m_bitQuest, udwIdx))
        {
            continue;
        }
        // 是否是待收集任务
        if(!CQuestLogic::IsToClaim(pstUserInfo, pstCity, udwIdx))
        {
            continue;
        }
    }
}

TBOOL CQuestLogic::IsToClaim(SUserInfo *pstUser, SCityInfo *pstCity, TUINT32 udwQuestId)
{
    STopQuest& stQuest = pstUser->m_tbUserStat.m_bTop_quest[0];

    // 0.判断任务是否已经收集
    if(BITTEST(stQuest.m_bitQuest, udwQuestId))
    {
        return FALSE;;
    }
    if(CQuestLogic::IsTopQuestFinish(pstUser, pstCity, udwQuestId))
    {
        return TRUE;
    }

    return FALSE;
}

TBOOL CQuestLogic::IsTopQuestFinish(SUserInfo *pstUser, SCityInfo *pstCity, TUINT32 udwQuestId)
{
    TBOOL bMiscFinish = FALSE;

    CGameInfo *poGameInfo = CGameInfo::GetInstance();

    TCHAR szBuf[1024];
    snprintf(szBuf, 1024, "%u", udwQuestId);
    if(!poGameInfo->m_oJsonRoot["game_quest"].isMember(szBuf))
    {
        return FALSE;
    }

    if (BITTEST(pstUser->m_tbUserStat.m_bTop_quest_finish[0].m_bitQuest, udwQuestId))
    {
        return TRUE;
    }
    TUINT32 udwType = poGameInfo->m_oJsonRoot["game_quest"][szBuf]["o"]["a0"].asUInt();
    TUINT32 udwId = poGameInfo->m_oJsonRoot["game_quest"][szBuf]["o"]["a1"].asUInt();
    TINT64 ddwTargetValue = poGameInfo->m_oJsonRoot["game_quest"][szBuf]["o"]["a2"].asInt64();
    TINT64 ddwCurValue = 0;

    TINT32 dwHasTips = poGameInfo->m_oJsonRoot["game_quest"][szBuf]["a"]["a7"].asInt();

    switch(udwType)
    {
    case EN_QUEST_TYPE_UPGRADE_BUILDING:
        ddwCurValue = CCityBase::GetBuildingLevelById(&pstCity->m_stTblData, udwId);
        break;
    case EN_QUEST_TYPE_CONSTRUCT_BUILDING:
        ddwCurValue = CCityBase::GetBuildingNumById(pstCity, udwId);
        break;
    case EN_QUEST_TYPE_UPGRADE_RESEARCH:
        ddwCurValue = pstCity->m_stTblData.m_bResearch[0].m_addwLevel[udwId];
        break;
    case EN_QUEST_TYPE_INCREASE_PRODUCTION:
        ddwCurValue = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_GOLD_PRODUCTION + udwId].m_ddwBuffTotal;
        break;
    case EN_QUEST_TYPE_TROOP:
        ddwCurValue = pstCity->m_stTblData.m_bTroop[0].m_addwNum[udwId];
        break;
    case EN_QUEST_TYPE_FORT:
        ddwCurValue = pstCity->m_stTblData.m_bFort[0].m_addwNum[udwId];
        break;
    case EN_QUEST_TYPE_LEVEL_UP:
        ddwCurValue = pstUser->m_tbPlayer.m_nLevel;
        break;
    case EN_QUEST_TYPE_MISC:
        switch(udwQuestId)
        {
        case 204: // join alliance
            if(0 < pstUser->m_tbAlliance.m_nAid && EN_ALLIANCE_POS__REQUEST != pstUser->m_tbPlayer.m_nAlpos)
            {
                bMiscFinish = TRUE;

            }
            break;
        case 208: // change player name
            if(NULL == strstr(pstUser->m_tbPlayer.m_sUin.c_str(), CGameInfo::GetInstance()->m_oJsonRoot["game_init_data"]["r"]["a1"].asString().c_str()))
            {
                bMiscFinish = TRUE;
            }
            break;
        case 209: // change city name
            if(NULL == strstr(pstCity->m_stTblData.m_sName.c_str(), CGameInfo::GetInstance()->m_oJsonRoot["game_init_data"]["r"]["a0"].asString().c_str()))
            {
                bMiscFinish = TRUE;

            }
            break;
        case 203://开始一个daily quest
            if(CQuestLogic::HasRuningTimeQuest(&pstUser->m_tbQuest, EN_TIME_QUEST_TYPE_DAILY))
            {
                bMiscFinish = TRUE;
            }
            break;
        case 205:
            if(CQuestLogic::HasRuningTimeQuest(&pstUser->m_tbQuest, EN_TIME_QUEST_TYPE_ALLIANCE))
            {
                bMiscFinish = TRUE;
            }
            break;//开始一个alliance quest
        default:
            break;
        }
        break;
    case EN_QUEST_TYPE_QUEST_NUM:
        ddwCurValue = GetDoneQuestNum(pstUser, udwId);
        break;
    default:
        break;
    }

    if((ddwTargetValue <= ddwCurValue && EN_QUEST_TYPE_MISC != udwType) || bMiscFinish)
    {
        TSE_LOG_DEBUG(poGameInfo->m_poLog, ("[IsFinish TRUE][uid=%u,questid=%u,id=%u,type=%u,cur=%u,target=%u,misfinish=%u]",
            pstUser->m_tbPlayer.m_nUid, udwQuestId, udwId, udwType, ddwCurValue, ddwTargetValue, bMiscFinish));

        BITSET(pstUser->m_tbUserStat.m_bTop_quest_finish[0].m_bitQuest, udwQuestId);
        pstUser->m_tbUserStat.SetFlag(TbUSER_STAT_FIELD_TOP_QUEST_FINISH);

        if (dwHasTips == 1)
        {
            CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPE__TOP_QUEST_FINISH, pstUser->m_tbPlayer.m_nUid, FALSE, udwQuestId);
        }

        return TRUE;
    }
    else
    {
        return FALSE;
    }
    return FALSE;
}

TBOOL CQuestLogic::HasRuningTimeQuest(TbQuest* ptbQuest, TUINT8 ucType)
{
    SQuestNode* pstTimeQuest = NULL;
    switch(ucType)
    {
    case EN_TIME_QUEST_TYPE_DAILY:
        pstTimeQuest = &ptbQuest->m_bDaily_quest[0];
        break;
    case EN_TIME_QUEST_TYPE_ALLIANCE:
        pstTimeQuest = &ptbQuest->m_bAl_quest[0];
        break;
    case EN_TIME_QUEST_TYPE_VIP:
        pstTimeQuest = &ptbQuest->m_bVip_quest[0];
        break;
    default:
        break;
    }

    if(pstTimeQuest == NULL)
    {
        return FALSE;
    }

    for(TUINT32 udwIdx = 0; udwIdx < pstTimeQuest->m_ddwQuestNum; ++udwIdx)
    {
        if(EN_TIME_QUEST_STATUS_RUNNING == pstTimeQuest->m_stQuestCom[udwIdx].m_ddwStatus)
        {
            return TRUE;
        }
    }

    return FALSE;
}



TUINT32 CQuestLogic::CheckQuestNodeRunning(SQuestNode* pstQuestNode, TBOOL &bRunning)
{
    TUINT32 udwQuestFinishTime = 0;
    bRunning = FALSE;
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();

    for(TUINT32 udwIdx = 0; udwIdx < pstQuestNode->m_ddwQuestNum; ++udwIdx)
    {
        if(pstQuestNode->m_stQuestCom[udwIdx].m_ddwStatus == EN_TIME_QUEST_STATUS_RUNNING
            && udwCurTime < pstQuestNode->m_stQuestCom[udwIdx].m_ddwBTime + pstQuestNode->m_stQuestCom[udwIdx].m_ddwCTime)
        {
            udwQuestFinishTime = pstQuestNode->m_stQuestCom[udwIdx].m_ddwBTime + pstQuestNode->m_stQuestCom[udwIdx].m_ddwCTime;
            bRunning = TRUE;
            return udwQuestFinishTime;
        }
    }

    return udwQuestFinishTime;
}


TVOID CQuestLogic::CheckTopQuestCanClaim(SUserInfo *pstUser, TBOOL &bClaim)
{
    bClaim = FALSE;

    for(TUINT32 udwIdx = 0; udwIdx < MAX_QUEST_NUM; ++udwIdx)
    {
        // 判断任务是否已经收集
        if(BITTEST(pstUser->m_tbUserStat.m_bTop_quest[0].m_bitQuest, udwIdx))
        {
            continue;
        }

        if(IsTopQuestFinish(pstUser, &pstUser->m_stCityInfo, udwIdx))
        {
            bClaim = TRUE;
            break;
        }
    }

}

TBOOL CQuestLogic::CheckPeacetimeRemind(SUserInfo *pstUser)
{
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    TBOOL bIsRemind = FALSE;
    TINT64 ddwBreakTime = 0;

    if(pstUser->m_tbPlayer.m_nStatus & EN_CITY_STATUS__AVOID_WAR)
    {
        ddwBreakTime = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_PEACE_TIME].m_astBuffDetail[EN_BUFF_TYPE_ITEM].m_dwTime;

        if(ddwBreakTime - (TINT64)udwCurTime > 0
            && ddwBreakTime - (TINT64)udwCurTime < 60 * 60)
        {
        }
    }

    return bIsRemind;
}

TBOOL CQuestLogic::CheckNewProtectRemind(SUserInfo *pstUser)
{
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    TUINT32 udwProtectTime = CGameInfo::GetInstance()->m_oJsonRoot["game_basic"][EN_GAME_BASIC_NEW_PROTECT_TIME].asUInt();
    TBOOL bIsRemind = FALSE;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;

    if(pstUser->m_tbPlayer.m_nStatus & EN_CITY_STATUS__NEW_PROTECTION)
    {
        if((TUINT32)ptbPlayer->Get_Ctime() + udwProtectTime - udwCurTime > 0
            && (TUINT32)ptbPlayer->Get_Ctime() + udwProtectTime - udwCurTime < 2 * 60 * 60
            && ptbPlayer->Get_Status() & EN_CITY_STATUS__NEW_PROTECTION)
        {
            bIsRemind = TRUE;
        }
    }
    return bIsRemind;
}

TVOID CQuestLogic::RemoveAgeTask(SUserInfo *pstUser,SCityInfo *pstCity, TUINT32 udwAge)
{
    TbTask *pstTask = &pstUser->m_tbTask;

    const Json::Value &rTaskJson = CGameInfo::GetInstance()->m_oJsonRoot["game_task"];
    //普通任务
    TUINT32 udwIdx = 0;
    while(udwIdx < pstTask->m_bTask_normal.m_udwNum)
    {
        TUINT32 udwTaskId = pstTask->m_bTask_normal[udwIdx].m_ddwId;
        TINT32 dwTaskAge = rTaskJson[CCommonFunc::NumToString(udwTaskId)]["a"]["a9"].asInt();
        if(dwTaskAge == -1)
        {
            dwTaskAge = 9999;
        }
        if(dwTaskAge <= static_cast<TINT32>(udwAge))
        {
            //GetTaskRewardById(pstUser, pstCity, udwTaskId);
            BITSET(pstTask->m_bTask_finish[0].m_bitTask, udwTaskId);

            //daemon 清除上个时代的任务时  检查打勾状态是否有上个时代的任务
            if(pstTask->m_nTask_check_id == udwTaskId)
            {
                pstTask->Set_Task_check_id(0);
                pstTask->Set_Task_status(EN_TASK_SHOW_STATUS_NORMAL);
            }
            pstTask->m_bTask_normal[udwIdx].Reset();
            if(udwIdx != pstTask->m_bTask_normal.m_udwNum - 1)
            {
                memcpy((char*)&pstTask->m_bTask_normal[udwIdx], (char*)&pstTask->m_bTask_normal[pstTask->m_bTask_normal.m_udwNum - 1], sizeof(STaskNodeNow));
            }
            pstTask->m_bTask_normal.m_udwNum--;
            //pstUser->m_audwTmpFinishTaskId[pstUser->m_udwTmpFinishNum] = udwTaskId;
            //pstUser->m_udwTmpFinishNum++;

            pstTask->SetFlag(TbTASK_FIELD_TASK_FINISH);
            pstTask->SetFlag(TbTASK_FIELD_TASK_NORMAL);
        }
        else
        {
            ++udwIdx;
        }
    }
    Json::Value::Members oTaskId = rTaskJson.getMemberNames();
    for(Json::Value::Members::iterator it = oTaskId.begin(); it != oTaskId.end(); ++it)
    {
        TUINT32 udwTaskId = atoi((*it).c_str());
        TINT32 dwTaskAge = rTaskJson[(*it).c_str()]["a"]["a9"].asUInt();
        if(BITTEST(pstTask->m_bTask_finish[0].m_bitTask, udwTaskId))
        {
            continue;
        }
        if(dwTaskAge == -1)
        {
            continue;
        }
        else if(dwTaskAge <= static_cast<TINT32>(udwAge))
        {
            //GetTaskRewardById(pstUser, pstCity, udwTaskId);
            BITSET(pstTask->m_bTask_finish[0].m_bitTask, udwTaskId);
            pstTask->SetFlag(TbTASK_FIELD_TASK_FINISH);
        }
    }
    return;
}

TVOID CQuestLogic::RemoveAgeTopQuest(SUserInfo *pstUser, TUINT32 udwAge)
{
    TbUser_stat *pstStat = &pstUser->m_tbUserStat;
    const Json::Value &rQuestJson = CGameInfo::GetInstance()->m_oJsonRoot["game_quest"];
    Json::Value::Members member = rQuestJson.getMemberNames();

    for(Json::Value::Members::iterator iter = member.begin(); iter != member.end(); ++iter)
    {
        TUINT32 udwQuestId = atoi((*iter).c_str());
        TUINT32 udwQuestAge = rQuestJson[CCommonFunc::NumToString(udwQuestId)]["a"]["a6"].asUInt();
        if(udwQuestAge <= udwAge)
        {
            if(!BITTEST(pstStat->m_bTop_quest_finish[0].m_bitQuest, udwQuestId))
            {
                BITSET(pstStat->m_bTop_quest[0].m_bitQuest, udwQuestId);
                pstStat->SetFlag(TbUSER_STAT_FIELD_TOP_QUEST);
            }
        }
    }

    return;
}

//gt task
TBOOL CQuestLogic::IsTaskFinish(SUserInfo *pstUser, SCityInfo *pstCity, TINT32 dwTaskIdx, TBOOL bIsRepeatType)
{
    STaskNodeNow *pstTaskNodeNow = NULL;
    if(bIsRepeatType)
    {
        pstTaskNodeNow = &pstUser->m_tbTask.m_bTask_time[dwTaskIdx];
    }
    else
    {
        pstTaskNodeNow = &pstUser->m_tbTask.m_bTask_normal[dwTaskIdx];
    }
    TBOOL bFinish = TRUE;
    for(TUINT32 udwIdx = 0; udwIdx < MAX_TASK_CONDITION_LIMIT; ++udwIdx)
    {
        if(pstTaskNodeNow->astFinishCondition[udwIdx].m_ddwTaskType == 0)
        {
            continue;
        }

        //操作型
        if(pstTaskNodeNow->astFinishCondition[udwIdx].m_bIsStand)
        {
            if (pstTaskNodeNow->astFinishCondition[udwIdx].m_uddwCurrValue - pstTaskNodeNow->astFinishCondition[udwIdx].m_uddwBeginValue 
                < pstTaskNodeNow->astFinishCondition[udwIdx].m_uddwNum)
            {
                bFinish = FALSE;
                break;
            }
        }
        else
        {
            if (pstTaskNodeNow->astFinishCondition[udwIdx].m_uddwCurrValue < pstTaskNodeNow->astFinishCondition[udwIdx].m_uddwNum)
            {
                bFinish = FALSE;
                break;
            }
        }
    }
    return bFinish;
}

TBOOL CQuestLogic::IsSubTaskFinish(SUserInfo *pstUser, SCityInfo *pstCity, TINT32 dwTaskIdx, TINT32 dwSubIdx, TBOOL bIsRepeatType)
{
    STaskNodeNow *pstTaskNodeNow = NULL;
    if(bIsRepeatType)
    {
        pstTaskNodeNow = &pstUser->m_tbTask.m_bTask_time[dwTaskIdx];
    }
    else
    {
        pstTaskNodeNow = &pstUser->m_tbTask.m_bTask_normal[dwTaskIdx];
    }
    
    return IsSubTaskFinish(pstUser, pstCity, pstTaskNodeNow, dwSubIdx);
}

TBOOL CQuestLogic::IsSubTaskFinish( SUserInfo *pstUser, SCityInfo *pstCity, STaskNodeNow *pstTaskNodeNow, TINT32 dwSubIdx )
{
    TBOOL bFinish = TRUE;
    if(pstTaskNodeNow->astFinishCondition[dwSubIdx].m_ddwTaskType == 0)
    {
        return bFinish;
    }

    //操作型
    if(pstTaskNodeNow->astFinishCondition[dwSubIdx].m_bIsStand)
    {
        if (pstTaskNodeNow->astFinishCondition[dwSubIdx].m_uddwCurrValue - pstTaskNodeNow->astFinishCondition[dwSubIdx].m_uddwBeginValue
            < pstTaskNodeNow->astFinishCondition[dwSubIdx].m_uddwNum)
        {
            bFinish = FALSE;
        }
    }
    else
    {
        if (pstTaskNodeNow->astFinishCondition[dwSubIdx].m_uddwCurrValue < pstTaskNodeNow->astFinishCondition[dwSubIdx].m_uddwNum)
        {
            bFinish = FALSE;
        }
    }

    return bFinish;
}

TINT64 CQuestLogic::GetOwnNum(SCityInfo *pstCity, SUserInfo *pstUser, TUINT32 udwTaskType, TUINT32 udwId, TUINT32 udwValue)
{
    TINT64 ddwOwnNum = 0;
    switch(udwTaskType)
    {

    case EN_TASK_TYPE_CONSTRUCT_BUILDING:
        ddwOwnNum = CCityBase::GetBuildingNumByFuncType(pstCity, udwId);
        break;
    case EN_TASK_TYPE_UPGRADE_BUILDING:
        ddwOwnNum = CCityBase::GetBuildingNumByLvAndType(pstCity, udwId, udwValue);
        break;
    case EN_TASK_TYPE_RESEARCH:
        ddwOwnNum = pstCity->m_stTblData.m_bResearch[0].m_addwLevel[udwId];
        break;
    case EN_TASK_TYPE_UNLOCK_PLACE:
        if(TRUE)
        {
            ddwOwnNum = 64 - CCityBase::GetBuildingNumByFuncType(pstCity, EN_BUILDING_TYPE__GROVE);
        }
        break;
    case EN_TASK_TYPE_DAILY_QUEST_FINISH:
        ddwOwnNum = pstUser->m_tbUserStat.m_nDaily_quest_finish_num;
        break;
    case EN_TASK_TYPE_ALLIANCE_QUEST_FINISH:
        ddwOwnNum = pstUser->m_tbUserStat.m_nAl_quest_finish_num;
        break;
    case EN_TASK_TYPE_VIP_QUEST_FINISH:
        ddwOwnNum = pstUser->m_tbUserStat.m_nVip_quest_finish_num;
        break;
    case EN_TASK_TYPE_TRAIN_TROOPS:
        ddwOwnNum = pstCity->m_stTblData.m_bTroop[0].m_addwNum[udwId];
        // march(march_raw_troop)
        for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; udwIdx++)
        {
            if(EN_TABLE_UPDT_FLAG__DEL == pstUser->m_aucMarchFlag[udwIdx])
            {
                continue;
            }

            if(!CActionBase::IsPlayerOwnedAction(pstUser->m_tbPlayer.m_nUid, pstUser->m_atbMarch[udwIdx].m_nId))
            {
                continue;
            }

            if(EN_ACTION_MAIN_CLASS__MARCH == pstUser->m_atbMarch[udwIdx].m_nMclass)
            {
                if(EN_ACTION_SEC_CLASS__SCOUT == pstUser->m_atbMarch[udwIdx].m_nSclass)
                {
                    continue;
                }
                ddwOwnNum += pstUser->m_atbMarch[udwIdx].m_bParam[0].m_stTroop.m_addwNum[udwId];
            }
        }
        break;
    case EN_TASK_TYPE_TRAIN_CITYWALL:
        ddwOwnNum = pstCity->m_stTblData.m_bFort[0].m_addwNum[udwId];
        break;
    case EN_TASK_TYPE_ALLIANCE_TRANSPORT:
        ddwOwnNum = pstUser->m_tbPlayer.m_bTransport_resource[0].m_addwNum[udwId];
        break;
    case EN_TASK_TYPE_ALLIANCE_REINFORCE:
        ddwOwnNum = pstUser->m_tbUserStat.m_nReinforce_num;
        break;
    case EN_TASK_TYPE_GET_ALLIANCE_CONTRIBUTION:
        ddwOwnNum = pstUser->m_tbPlayer.m_nLoy_all;
        break;
    case EN_TASK_TYPE_GET_RESOURCE:
        ddwOwnNum = pstUser->m_tbPlayer.m_bGain_resource[0].m_addwNum[udwId];
        break;
    case EN_TASK_TYPE_GET_ALL_RESOURCE:
        for(TUINT32 udwIdx = 0; udwIdx < EN_RESOURCE_TYPE__END;++udwIdx)
        {
            ddwOwnNum += pstUser->m_tbPlayer.m_bGain_resource[0].m_addwNum[udwIdx];
        }
        break;
    case EN_TASK_TYPE_UPGRADE_HERO:
        ddwOwnNum = pstUser->m_tbPlayer.m_nDragon_level;
        break;
    case EN_TASK_TYPE_ASSIGN_HERO_SKILLS:
        for(TUINT32 udwIdx = 0; udwIdx < EN_SKILL_TYPE__LIMIT; ++udwIdx)
        {
            if(pstUser->m_tbUserStat.m_bDragon_skill[0].m_addwLevel[udwIdx] != 0)
            {
                ddwOwnNum += pstUser->m_tbUserStat.m_bDragon_skill[0].m_addwLevel[udwIdx];
            }
            if (pstUser->m_tbUserStat.m_bDragon_monster_skill[0].m_addwLevel[udwIdx] != 0)
            {
                ddwOwnNum += pstUser->m_tbUserStat.m_bDragon_monster_skill[0].m_addwLevel[udwIdx];
            }
        }
        break;
    case EN_TASK_TYPE_HERO_WEAR_EQUIPMENT:
        //for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_tbPlayer.m_bEquipment_list.m_udwNum; ++udwIdx)
        //{
        //    if(pstUser->m_tbPlayer.m_bEquipment_list[0].uddwId != 0)
        //    {
        //        dwOwnNum++;
        //    }
        //}
        break;
    case EN_TASK_TYPE_RAISE_POPULATION:
        ddwOwnNum = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_POPULATION_CAPICATY].m_ddwBuffTotal;
        break;
    case EN_TASK_TYPE_INCREASE_HOSPITAL_CAPCITY:
        ddwOwnNum = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_HOSPITAL_CAPICATY].m_ddwBuffTotal;
        break;
    case EN_TASK_TYPE_RESEARCH_SUM:
        for(TUINT32 udwIdx = 0; udwIdx < EN_RESEARCH_TYPE__LIMIT; ++udwIdx)
        {
            if (pstCity->m_stTblData.m_bResearch[0].m_addwLevel[udwIdx] != 0)
            {
                ddwOwnNum += pstCity->m_stTblData.m_bResearch[0].m_addwLevel[udwIdx];
            }
        }
        break;
    case EN_TASK_TYPE_INCREASE_RESOURCE_CAPCITY:
        if(TRUE)
        {
            TFLOAT64 dfTotalAddBuff = (10000.0 + pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_ALL_RESOURCE_PROTECTION].m_ddwBuffTotal)/10000;
            switch(udwId)
            {
            case EN_RESOURCE_TYPE__FOOD:
                ddwOwnNum = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_FOOD_PROTECTION].m_ddwBuffTotal * dfTotalAddBuff;
                break;
            case EN_RESOURCE_TYPE__WOOD:
                ddwOwnNum = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_WODD_PROTECTION].m_ddwBuffTotal  * dfTotalAddBuff;
                break;
            case EN_RESOURCE_TYPE__STONE:
                ddwOwnNum = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_STONE_PROTECTION].m_ddwBuffTotal  * dfTotalAddBuff;
                break;
            case EN_RESOURCE_TYPE__ORE:
                ddwOwnNum = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_ORE_PROTECTION].m_ddwBuffTotal  * dfTotalAddBuff;
                break;
            case EN_RESOURCE_TYPE__GOLD:
                ddwOwnNum = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_GOLD_PROTECTION].m_ddwBuffTotal  * dfTotalAddBuff;
                break;
            default:
                ddwOwnNum = 0;
            }
        }
        break;
    case EN_TASK_TYPE_RAISE_ALLIANCE_MARCH_SPEED:
        ddwOwnNum = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_REINFORCE_SPEED_UP_PERCENT].m_ddwBuffTotal;
        break;
    case EN_TASK_TYPE_RAISE_MARCH_SUM:
        ddwOwnNum = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_QUEUE_NUM].m_ddwBuffTotal;
        break;
    case EN_TASK_TYPE_RAISE_MARCH_TROOPS:
        ddwOwnNum = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_TROOP_SIZE].m_ddwBuffTotal;
        break;
    case EN_TASK_TYPE_INCREASE_FOOD_PRODUCTION:
        ddwOwnNum = pstCity->m_astResProduction[EN_RESOURCE_TYPE__FOOD].m_ddwCurProduction;
        break;
    case EN_TASK_TYPE_INCREASE_WOOD_PRODUCTION:
        ddwOwnNum = pstCity->m_astResProduction[EN_RESOURCE_TYPE__WOOD].m_ddwCurProduction;
        break;
    case EN_TASK_TYPE_INCREASE_STONE_PRODUCTION:
        ddwOwnNum = pstCity->m_astResProduction[EN_RESOURCE_TYPE__STONE].m_ddwCurProduction;
        break;
    case EN_TASK_TYPE_INCREASE_IRON_PRODUCTION:
        ddwOwnNum = pstCity->m_astResProduction[EN_RESOURCE_TYPE__ORE].m_ddwCurProduction;
        break;
    case EN_TASK_TYPE_INCREASE_GOLD_PRODUCTION:
        ddwOwnNum = pstCity->m_astResProduction[EN_RESOURCE_TYPE__GOLD].m_ddwCurProduction;
        break;
    case EN_TASK_TYPE_RESOURCE_PRODUCTION_BUFF:
        ddwOwnNum = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_GOLD_PRODUCTION_PERCENT + udwId].m_ddwBuffTotal +
            pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_ALL_RESOURCE_PRODUCTION_PERCENT].m_ddwBuffTotal;
        break;
    case EN_TASK_TYPE_TROOPS_SUM:
        for(TUINT32 udwIdx = 0; udwIdx < EN_TROOP_TYPE__END; ++udwIdx)
        {
            if(pstCity->m_stTblData.m_bTroop[0].m_addwNum[udwIdx] != 0)
            {
                ddwOwnNum += pstCity->m_stTblData.m_bTroop[0].m_addwNum[udwIdx];
            }
        }
        break;
    case EN_TASK_TYPE_KILL_TROOPS_SUM:
        ddwOwnNum = pstUser->m_tbUserStat.m_nKill_troop_num;
        break;
    case EN_TASK_TYPE_BUILDING_NUM:
        for(TUINT32 udwIdx = 0; udwIdx < pstCity->m_stTblData.m_bBuilding.m_udwNum; ++udwIdx)
        {
            TUINT32 udwBuildingId = pstCity->m_stTblData.m_bBuilding[udwIdx].m_ddwType;
            if(CGameInfo::GetInstance()->m_oJsonRoot["game_building"][CCommonFunc::NumToString(udwBuildingId)]["a"]["a12"].asUInt() == 1)
            {
                continue;
            }
            if(pstCity->m_stTblData.m_bBuilding[udwIdx].m_ddwLevel != 0)
            {
                ddwOwnNum++;
            }
        }
        break;
    case EN_TASK_TYPE_AGE_UPGRADE:
        ddwOwnNum = pstUser->m_tbPlayer.m_nAge;
        break;
    case EN_TASK_TYPE_VIP_UPGRADE:
        ddwOwnNum = CPlayerBase::GetRawVipLevel(pstUser->m_tbPlayer.m_nVip_point);
        break;
    case EN_TASK_TYPE_GET_GOLD_RESOURCE:
        ddwOwnNum = pstUser->m_tbPlayer.m_bGain_resource[0].m_addwNum[EN_RESOURCE_TYPE__GOLD];
        break;
    case EN_TASK_TYPE_GET_FOOD_RESOURCE:
        ddwOwnNum = pstUser->m_tbPlayer.m_bGain_resource[0].m_addwNum[EN_RESOURCE_TYPE__FOOD];
        break;
    case EN_TASK_TYPE_GET_WOOD_RESOURCE:
        ddwOwnNum = pstUser->m_tbPlayer.m_bGain_resource[0].m_addwNum[EN_RESOURCE_TYPE__WOOD];
        break;
    case EN_TASK_TYPE_GET_STONE_RESOURCE:
        ddwOwnNum = pstUser->m_tbPlayer.m_bGain_resource[0].m_addwNum[EN_RESOURCE_TYPE__STONE];
        break;
    case EN_TASK_TYPE_GET_IRON_RESOURCE:
        ddwOwnNum = pstUser->m_tbPlayer.m_bGain_resource[0].m_addwNum[EN_RESOURCE_TYPE__ORE];
        break;
    case EN_TASK_TYPE_CHANGE_PLAYER_NAME:
        ddwOwnNum = CPlayerBase::HasChangePlayerName(&pstUser->m_tbPlayer) ? 1 : 0;
        break;
    case EN_TASK_TYPE_CHANGE_CITY_NAME:
        ddwOwnNum = CCityBase::HasChangeCityName(pstCity) ? 1:0;
        break;
    case EN_TASK_TYPE_ING_BUILD_X_LV_BUILDING:
        for(TUINT32 udwIdx = 0; udwIdx < pstCity->m_stTblData.m_bBuilding.m_udwNum; ++udwIdx)
        {
            TUINT32 udwBuildingId = pstCity->m_stTblData.m_bBuilding[udwIdx].m_ddwType;
            if(CGameInfo::GetInstance()->m_oJsonRoot["game_building"][CCommonFunc::NumToString(udwBuildingId)]["a"]["a12"].asUInt() == 1)
            {
                continue;
            }
            if(pstCity->m_stTblData.m_bBuilding[udwIdx].m_ddwLevel == 0)
            {
                continue;
            }
            if(pstCity->m_stTblData.m_bBuilding[udwIdx].m_ddwLevel >= udwValue)
            {
                ddwOwnNum++;
            }
        }
        break;
    
    case EN_TASK_TYPE_OWN_EQUIP_NUM:
        for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwEquipNum;++udwIdx)
        {
            if(pstUser->m_aucEquipFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
            {
                continue;
            }
            TbEquip *pstEquip = &pstUser->m_atbEquip[udwIdx];

            if(pstEquip->m_nStatus == EN_EQUIPMENT_STATUS_UPGRADING)
            {
                continue;
            }
            ddwOwnNum++;
        }
        break;
    default:
        ddwOwnNum = 0;
    }
    return ddwOwnNum;
}

TBOOL CQuestLogic::IsTaskShowNow(SUserInfo *pstUser, TUINT32 udwTaskId)
{
    TbTask *ptbTask = &pstUser->m_tbTask;
    const Json::Value &rTaskJson = CGameInfo::GetInstance()->m_oJsonRoot["game_task"];

    //等级
    if(rTaskJson[CCommonFunc::NumToString(udwTaskId)]["a"]["a2"].asUInt() > pstUser->m_tbPlayer.m_nLevel)
    {
        return FALSE;
    }

    //前置任务
    for(TUINT32 udwIdx = 0; udwIdx < rTaskJson[CCommonFunc::NumToString(udwTaskId)]["a"]["a1"].size(); ++udwIdx)
    {
        TUINT32 udwFixQuestid = rTaskJson[CCommonFunc::NumToString(udwTaskId)]["a"]["a1"][udwIdx].asUInt();
        if(!BITTEST(ptbTask->m_bTask_finish[0].m_bitTask, udwFixQuestid))
        {
            return FALSE;
        }
    }
    //任务链
    for(TUINT32 udwIdx = 0; udwIdx < rTaskJson[CCommonFunc::NumToString(udwTaskId)]["a"]["a4"].size(); ++udwIdx)
    {
        TUINT32 udwTaskListId = rTaskJson[CCommonFunc::NumToString(udwTaskId)]["a"]["a4"][udwIdx].asUInt();
        if(udwTaskListId == udwTaskId)
        {
            break;
        }
        if(!BITTEST(ptbTask->m_bTask_finish[0].m_bitTask, udwTaskListId))
        {
            return FALSE;
        }
    }
    
    //非重复任务
    if(0 == rTaskJson[CCommonFunc::NumToString(udwTaskId)]["a"]["a7"].asUInt())
    {
        //时代
        if(rTaskJson[CCommonFunc::NumToString(udwTaskId)]["a"]["a9"].asInt() != -1 &&
            rTaskJson[CCommonFunc::NumToString(udwTaskId)]["a"]["a9"].asInt() < pstUser->m_tbPlayer.m_nAge)
        {
            return FALSE;
        }

        //是否在当前任务中
        for(TUINT32 udwIdx = 0; udwIdx < ptbTask->m_bTask_normal.m_udwNum; ++udwIdx)
        {
            if(udwTaskId == ptbTask->m_bTask_normal[udwIdx].m_ddwId)
            {
                return FALSE;
            }
        }

        //是否已完成该任务
        if(BITTEST(ptbTask->m_bTask_finish[0].m_bitTask, udwTaskId))
        {
            return FALSE;
        }
    }
    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("IsTaskShowNow:uid=%u TRUE  [%u]", pstUser->m_tbPlayer.m_nUid, udwTaskId));
    return  TRUE;
}

TVOID CQuestLogic::InsertQuest(SUserInfo *pstUser, SCityInfo *pstCity, TUINT32 udwTaskId, TUINT32 udwIdx)
{
    TbTask *ptbTask = &pstUser->m_tbTask;
    const Json::Value &rTaskJson = CGameInfo::GetInstance()->m_oJsonRoot["game_task"];


    STaskCondiStatusNode *pstCondition = NULL;
    const Json::Value &rCondiJson = rTaskJson[CCommonFunc::NumToString(udwTaskId)]["o"];
    //非重复任务
    if(0 == rTaskJson[CCommonFunc::NumToString(udwTaskId)]["a"]["a7"].asUInt())
    {
        pstCondition = &ptbTask->m_bTask_normal[udwIdx].astFinishCondition[0];
        ptbTask->m_bTask_normal[udwIdx].m_ddwId = udwTaskId;
        ptbTask->m_bTask_normal[udwIdx].m_bIsNew = TRUE;
        ptbTask->m_bTask_normal[udwIdx].m_bIsProgress = FALSE;
        ptbTask->m_bTask_normal[udwIdx].m_ddwEndTime = 0;
        ptbTask->m_bTask_normal[udwIdx].m_ddwUpdateTime = CTimeUtils::GetUnixTime();
    }
    else
    {
        pstCondition = &ptbTask->m_bTask_time[udwIdx].astFinishCondition[0];
        ptbTask->m_bTask_time[udwIdx].m_ddwId = udwTaskId;
        ptbTask->m_bTask_time[udwIdx].m_bIsNew = TRUE;
        ptbTask->m_bTask_time[udwIdx].m_bIsProgress = FALSE;
        ptbTask->m_bTask_time[udwIdx].m_ddwEndTime = (1 + CTimeUtils::GetUnixTime() / (24 * 3600)) * 24 * 3600;
        ptbTask->m_bTask_time[udwIdx].m_ddwUpdateTime = CTimeUtils::GetUnixTime();
    }

    //condition
    for(TUINT32 udwConditionIdx = 0; udwConditionIdx < MAX_TASK_CONDITION_LIMIT; ++udwConditionIdx)
    {
        pstCondition[udwConditionIdx].m_ddwId = rCondiJson[udwConditionIdx][1U].asUInt();
        pstCondition[udwConditionIdx].m_bIsStand = rCondiJson[udwConditionIdx][4U].asUInt();
        pstCondition[udwConditionIdx].m_ddwTaskType = rCondiJson[udwConditionIdx][0U].asUInt();
        pstCondition[udwConditionIdx].m_uddwNum = rCondiJson[udwConditionIdx][2U].asUInt();
        pstCondition[udwConditionIdx].m_ddwValue = rCondiJson[udwConditionIdx][3U].asUInt();
        if(pstCondition[udwConditionIdx].m_bIsStand)
        {
            pstCondition[udwConditionIdx].m_uddwBeginValue = 0;
            pstCondition[udwConditionIdx].m_uddwCurrValue = 0;
        }
        else
        {
            pstCondition[udwConditionIdx].m_uddwBeginValue = GetOwnNum(pstCity, pstUser,
                rCondiJson[udwConditionIdx][0U].asUInt(),
                rCondiJson[udwConditionIdx][1U].asUInt(),
                rCondiJson[udwConditionIdx][3U].asUInt());

            pstCondition[udwConditionIdx].m_uddwCurrValue = pstCondition[udwConditionIdx].m_uddwBeginValue;
        }

        TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("InsertQuest:[udwTaskId=%u condition:id=%u is_stand=%u task_type=%u num=%u value=%u begin=%u cur_value=%u] [seq=%u]",
            udwTaskId,
            pstCondition[udwConditionIdx].m_ddwId,
            pstCondition[udwConditionIdx].m_bIsStand,
            pstCondition[udwConditionIdx].m_ddwTaskType,
            pstCondition[udwConditionIdx].m_uddwNum,
            pstCondition[udwConditionIdx].m_ddwValue,
            pstCondition[udwConditionIdx].m_uddwBeginValue,
            pstCondition[udwConditionIdx].m_uddwCurrValue,
            pstUser->m_udwBSeqNo));
    }
}

TINT32 CQuestLogic::GenTaskTimeQuest(SUserInfo *pstUser, SCityInfo *pstCity)
{
    const Json::Value &rTaskJson = CGameInfo::GetInstance()->m_oJsonRoot["game_task"];
    Json::Value::Members oTaskId = rTaskJson.getMemberNames();
    
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    
    TbTask *ptbTask = &pstUser->m_tbTask;
    
    TUINT32 audwTimeQuestSet[EN_TASK_NUM_LIMIT];
    memset(audwTimeQuestSet, 0, sizeof(audwTimeQuestSet));
    TUINT32 udwTimeQuestNum = 0;
    
    for(Json::Value::Members::iterator it = oTaskId.begin(); it != oTaskId.end(); ++it)
    {
        if(rTaskJson[(*it).c_str()]["a"]["a7"].asUInt() == 0)
        {
            continue;
        }
        if(CQuestLogic::IsTaskShowNow(pstUser, atoi((*it).c_str())))
        {
            audwTimeQuestSet[udwTimeQuestNum] = atoi((*it).c_str());
            udwTimeQuestNum++;
        }
    }
    
    //assert(udwTimeQuestNum != 0);
    TBOOL bHaveUpdate = FALSE;
    for(TUINT32 udwIdx = 0; udwIdx < MAX_TASK_CURR_TIME_LIMIT && udwIdx < udwTimeQuestNum; ++udwIdx)
    {
        if(udwTimeQuestNum == 0)
        {
            ptbTask->m_bTask_time[udwIdx].Reset();
            continue;
        }
        TUINT32 udwQuestIdx = rand() % udwTimeQuestNum;
        TUINT32 udwQuestId = audwTimeQuestSet[udwQuestIdx];
    
        TBOOL bExist = FALSE;
        for(TUINT32 udwTimeQuestIdx = 0; udwTimeQuestIdx < MAX_TASK_CURR_TIME_LIMIT; ++udwTimeQuestIdx)
        {
            if(udwQuestId == ptbTask->m_bTask_time[udwTimeQuestIdx].m_ddwId)
            {
                bExist = TRUE;
                break;
            }
        }
    
        if(bExist)
        {
            udwIdx--;
            continue;
        }
        CQuestLogic::InsertQuest(pstUser, pstCity, udwQuestId, udwIdx);
        ptbTask->m_bTask_time.m_udwNum++;
        bHaveUpdate = TRUE;
    }
    ptbTask->Set_Task_refresh_time(udwCurTime);
    if(bHaveUpdate)
    {
        //有新任务加入 检查是否更新展示状态
        if(ptbTask->m_nTask_open_window_flag == FALSE && ptbTask->m_nTask_status < EN_TASK_SHOW_STATUS_CHECK)
        {
            ptbTask->Set_Task_status(EN_TASK_SHOW_STATUS_ADD);
        }
        else if(ptbTask->m_nTask_open_window_flag == TRUE && ptbTask->m_nTask_status < EN_TASK_SHOW_STATUS_CHECK)
        {
            ptbTask->Set_Task_status(EN_TASK_SHOW_STATUS_ADD);
            ptbTask->Set_Task_open_window_flag(FALSE);
        }
    }
    ptbTask->SetFlag(TbTASK_FIELD_TASK_TIME);
    return bHaveUpdate;
}

TINT32 CQuestLogic::GenTaskNormalQuest(SUserInfo *pstUser, SCityInfo *pstCity)
{
    if(pstUser->m_tbTask.m_bTask_normal.m_udwNum >= MAX_TASK_CURR_NORMAL_LIMIT) //to加速
    {
        return 0;
    }

    const Json::Value &rTaskJson = CGameInfo::GetInstance()->m_oJsonRoot["game_task"];
    
    vector<STaskPriority> vecTask;
    vecTask.clear();
    TINT32 dwTmpTaskNum = 0;
    TINT32 dwAddTaskNum = 0;

    TbTask *ptbTask = &pstUser->m_tbTask;
    if(ptbTask->m_nUid == 0)
    {
        ptbTask->Set_Uid(pstUser->m_tbPlayer.m_nUid);
    }

    const vector<TINT64>& vecTaskList = CGameInfo::GetInstance()->GetTaskList();

    TBOOL bUpdate = FALSE;
    ostringstream oss;
    for (TUINT32 udwIdx = 0; udwIdx < vecTaskList.size(); udwIdx++)
    {
        oss.str("");
        oss << vecTaskList[udwIdx];
        if (!rTaskJson.isMember(oss.str()) || rTaskJson[oss.str()]["a"]["a7"].asUInt() == 1)
        {
            continue;
        }

        if (!CQuestLogic::IsTaskShowNow(pstUser, vecTaskList[udwIdx]))
        {
            continue;
        }

        //记录到临时任务列表
        if(dwTmpTaskNum < 30)
        {
            STaskPriority stTaskTmp;
            stTaskTmp.ddwTaskId = vecTaskList[udwIdx];
            stTaskTmp.bCanStart = TRUE;
            stTaskTmp.dwPriority = rTaskJson[oss.str()]["a"]["a0"].asInt();
            vecTask.push_back(stTaskTmp);
            dwTmpTaskNum++;
        }
        else
        {
            break;
        }
    }

    
    if(dwTmpTaskNum > 0)
    {
        //插入任务
        for(TUINT32 idx = 0; ptbTask->m_bTask_normal.m_udwNum < MAX_TASK_CURR_NORMAL_LIMIT && idx < vecTask.size(); idx++)
        {
            CQuestLogic::InsertQuest(pstUser, pstCity, vecTask[idx].ddwTaskId, ptbTask->m_bTask_normal.m_udwNum);
            if (!IsTaskFinish(pstUser, pstCity, ptbTask->m_bTask_normal.m_udwNum, FALSE))
            {
                ptbTask->m_bTask_normal.m_udwNum++;
                dwAddTaskNum++;
                //pstUser->m_setInsertTask.insert(vecTask[idx].ddwTaskId); //wave@20160428:用于防止新加入的task，在after检测时判定完成，展示标记
            }
            else
            {
                BITSET(ptbTask->m_bTask_finish[0].m_bitTask, vecTask[idx].ddwTaskId);
            }
        }
        bUpdate = TRUE;
    }

    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("GenTaskNormalQuest: select_task_num=%d add_task_num=%d real_task_num=%d [seq=%u]", 
        dwTmpTaskNum, dwAddTaskNum, ptbTask->m_bTask_normal.m_udwNum, pstUser->m_udwBSeqNo));
    
    if(bUpdate)
    {
        ptbTask->SetFlag(TbTASK_FIELD_TASK_NORMAL);
        ptbTask->SetFlag(TbTASK_FIELD_TASK_FINISH);

        //有新任务加入 检查是否更新展示状态
        if(ptbTask->m_nTask_open_window_flag == FALSE && ptbTask->m_nTask_status < EN_TASK_SHOW_STATUS_CHECK)
        {
            ptbTask->Set_Task_status(EN_TASK_SHOW_STATUS_ADD);
        }
        else if(ptbTask->m_nTask_open_window_flag == TRUE && ptbTask->m_nTask_status < EN_TASK_SHOW_STATUS_CHECK)
        {
            ptbTask->Set_Task_status(EN_TASK_SHOW_STATUS_ADD);
            ptbTask->Set_Task_open_window_flag(FALSE);
        }
    }
    return dwAddTaskNum;
}

TINT32 CQuestLogic::GetTaskRewardById(SUserInfo *pstUser, SCityInfo *pstCity, TUINT32 udwTaskId)
{
    TINT32 dwRetCode = 0;

    STaskNodeNow stStatus;
    stStatus.Reset();

    const Json::Value &rTaskJson = CGameInfo::GetInstance()->m_oJsonRoot["game_task"];

    if(!rTaskJson.isMember(CCommonFunc::NumToString(udwTaskId)))
    {
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("GetTaskRewardById:not such task id [task_id=%u] [seq=%u]", udwTaskId, pstUser->m_udwBSeqNo));
        return -1;
    }

    const Json::Value &oReward = rTaskJson[CCommonFunc::NumToString(udwTaskId)]["r"]["a1"];
    TINT32 dwRewardType = rTaskJson[CCommonFunc::NumToString(udwTaskId)]["r"]["a0"].asInt();
    SSpGlobalRes stResInfo;
    stResInfo.Reset();
    dwRetCode = CGlobalResLogic::GetSpGlobalResInfo(oReward, dwRewardType, &stResInfo);
    if(dwRetCode != 0)
    {
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("GetTaskRewardById: get reward info fail [uid=%ld task_id=%u] [ret=%d]", pstUser->m_tbPlayer.m_nUid, udwTaskId, dwRetCode));
        return dwRetCode;
    }

    // 3、获取奖励
    dwRetCode = CGlobalResLogic::AddSpGlobalRes(pstUser, pstCity, &stResInfo);
    if(dwRetCode != 0)
    {
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("CProcessQuestClaim: take reward fail [uid=%ld task_id=%u] [ret=%d]", pstUser->m_tbPlayer.m_nUid, udwTaskId, dwRetCode));
        return dwRetCode;
    }
    return 0;
}

TINT32 CQuestLogic::CheckTaskRefresh(SUserInfo *pstUser, SCityInfo *pstCity)
{
    TUINT32 udwNowDay = CTimeUtils::GetUnixTime() / (60 * 60 * 24);
    TUINT32 udwPlayerLastFreshDay = pstUser->m_tbTask.m_nTask_refresh_time / (60 * 60 * 24);
    TINT32 dwRefreshNum = 0;

    if(udwNowDay != udwPlayerLastFreshDay)
    {
        dwRefreshNum = GenTaskTimeQuest(pstUser, pstCity);
    }
    dwRefreshNum += GenTaskNormalQuest(pstUser, pstCity);

    if(dwRefreshNum > 0)
    {
        pstUser->m_bTaskUpdate = TRUE;
    }
    return dwRefreshNum;
}

TINT32 CQuestLogic::CheckTaskFinish(SUserInfo *pstUser, SCityInfo *pstCity)
{
    TbTask *ptbTask = &pstUser->m_tbTask;
    TUINT32 udwIdx = 0;

    //普通任务
    udwIdx = 0;
    while(udwIdx < ptbTask->m_bTask_normal.m_udwNum)
    {
        TUINT32 udwTaskId = ptbTask->m_bTask_normal[udwIdx].m_ddwId;
        if(IsTaskFinish(pstUser, pstCity, udwIdx, FALSE))
        {
            pstUser->m_bTaskUpdate = TRUE;

            // 优先从列表中删除
            ptbTask->m_bTask_normal[udwIdx].Reset();
            if(udwIdx != ptbTask->m_bTask_normal.m_udwNum - 1)
            {
                memcpy((char*)&ptbTask->m_bTask_normal[udwIdx], (char*)&ptbTask->m_bTask_normal[ptbTask->m_bTask_normal.m_udwNum - 1], sizeof(STaskNodeNow));
            }
            ptbTask->m_bTask_normal.m_udwNum--;
            ptbTask->SetFlag(TbTASK_FIELD_TASK_NORMAL);

            if(IsSpecialTask(udwTaskId))//--------所有special task不管是否出现在列表当中，都要去判断是否完成
            {
                udwIdx++;
                continue;
            }

            //设置标记
            BITSET(ptbTask->m_bTask_finish[0].m_bitTask, udwTaskId);
            ptbTask->SetFlag(TbTASK_FIELD_TASK_FINISH);
            if (udwTaskId == ptbTask->m_nRecommand_task_id)
            {
                CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPE__TASK_FINISH, pstUser->m_tbPlayer.m_nUid, FALSE, udwTaskId);
            }

            //如有task完成，清理check标记
            ptbTask->Set_Task_check_id(0);
            ptbTask->Set_Task_status(EN_TASK_SHOW_STATUS_NORMAL);
        }
        else
        {
            ++udwIdx;
        }
    }

    //限时任务
    udwIdx = 0;
    while(udwIdx < ptbTask->m_bTask_time.m_udwNum)
    {
        TUINT32 udwTaskId = ptbTask->m_bTask_time[udwIdx].m_ddwId;
        if(IsTaskFinish(pstUser, pstCity, udwIdx, TRUE))
        {
            pstUser->m_bTaskUpdate = TRUE;
            //GetTaskRewardById(pstUser, pstCity, udwTaskId);
            
            if(udwIdx != ptbTask->m_bTask_time.m_udwNum - 1)
            {
                memcpy((char*)&ptbTask->m_bTask_time[udwIdx], (char*)&ptbTask->m_bTask_time[ptbTask->m_bTask_time.m_udwNum - 1], sizeof(STaskNodeNow));
            }
            ptbTask->m_bTask_time.m_udwNum--;
            ptbTask->SetFlag(TbTASK_FIELD_TASK_TIME);

            if (udwTaskId == ptbTask->m_nRecommand_task_id)
            {
                CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPE__TASK_FINISH, pstUser->m_tbPlayer.m_nUid, FALSE, udwTaskId);
            }

            //如有task完成，清理check和status标记
            ptbTask->Set_Task_check_id(0);
            ptbTask->Set_Task_status(EN_TASK_SHOW_STATUS_NORMAL);
        }
        else
        {
            ++udwIdx;
        }
    }

    //special task
    CheckSpecialTaskFinish(pstUser, pstCity);

    return 0;
}

TVOID CQuestLogic::CheckOwnNumUpdate(SUserInfo *pstUser, SCityInfo *pstCity)
{
    TbTask *ptbTask = &pstUser->m_tbTask;
    TBOOL bNormalUpdate = FALSE;
    TBOOL bTimeUpdate = FALSE;
    //普通任务
    for(TUINT32 udwIdx = 0; udwIdx < ptbTask->m_bTask_normal.m_udwNum; ++udwIdx)
    {
        TUINT32 udwTaskId = ptbTask->m_bTask_normal[udwIdx].m_ddwId;
        if(udwTaskId == 0)
        {
            continue;
        }
        TBOOL bUpdateProgress = FALSE;
        for(TUINT32 udwCondiIdx = 0; udwCondiIdx < MAX_TASK_CONDITION_LIMIT; ++udwCondiIdx)
        {
            if(ptbTask->m_bTask_normal[udwIdx].astFinishCondition[udwCondiIdx].m_ddwTaskType == 0)
            {
                continue;
            }
            if(!ptbTask->m_bTask_normal[udwIdx].astFinishCondition[udwCondiIdx].m_bIsStand)
            {
                TUINT32 udwCondiType = ptbTask->m_bTask_normal[udwIdx].astFinishCondition[udwCondiIdx].m_ddwTaskType;
                TUINT32 udwCondiId = ptbTask->m_bTask_normal[udwIdx].astFinishCondition[udwCondiIdx].m_ddwId;
                TUINT32 udwCondiValue = ptbTask->m_bTask_normal[udwIdx].astFinishCondition[udwCondiIdx].m_ddwValue;
                TUINT64 uddwOldNum = ptbTask->m_bTask_normal[udwIdx].astFinishCondition[udwCondiIdx].m_uddwCurrValue;
                
                TUINT64 uddwCurNum = GetOwnNum(pstCity, pstUser, udwCondiType, udwCondiId, udwCondiValue);
                ptbTask->m_bTask_normal[udwIdx].astFinishCondition[udwCondiIdx].m_uddwCurrValue = uddwCurNum;

                if(uddwCurNum > uddwOldNum)
                {                 
                    bNormalUpdate = TRUE;
                    bUpdateProgress = TRUE;
                }
                if(ptbTask->m_bTask_normal[udwIdx].astFinishCondition[udwCondiIdx].m_uddwCurrValue >= ptbTask->m_bTask_normal[udwIdx].astFinishCondition[udwCondiIdx].m_uddwNum)
                {
                    ptbTask->m_bTask_normal[udwIdx].m_bIsProgress = TRUE;
                    ptbTask->m_bTask_normal[udwIdx].m_ddwUpdateTime = CTimeUtils::GetUnixTime();
                    bNormalUpdate = TRUE;                    
                }

            }
        }
        if(bUpdateProgress)
        {
            for(TUINT32 udwCondiIdx = 0; udwCondiIdx < MAX_TASK_CONDITION_LIMIT; ++udwCondiIdx)
            {
                if(ptbTask->m_bTask_normal[udwIdx].astFinishCondition[udwCondiIdx].m_ddwTaskType == 0)
                {
                    continue;
                }

                TUINT32 udwCondiType = ptbTask->m_bTask_normal[udwIdx].astFinishCondition[udwCondiIdx].m_ddwTaskType;
                TUINT32 udwCondiId = ptbTask->m_bTask_normal[udwIdx].astFinishCondition[udwCondiIdx].m_ddwId;
                TUINT32 udwCondiValue = ptbTask->m_bTask_normal[udwIdx].astFinishCondition[udwCondiIdx].m_ddwValue;

                if(!IsSubTaskFinish(pstUser, pstCity, udwIdx, udwCondiIdx, FALSE))
                {
                    TBOOL bCanStart = CQuestLogic::IsTaskCanStart(pstUser, pstCity, udwCondiType, udwCondiId, udwCondiValue);

                    if(bCanStart)
                    {
                        ptbTask->Set_Task_check_id(udwTaskId);
                        ptbTask->Set_Task_status(EN_TASK_SHOW_STATUS_CHECK);
                        pstUser->m_bTaskUpdate = TRUE;

                        TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("CheckOwnNumUpdate_WAVE:uid=%u finish task_id=%u status=%u [seq=%u]",
                            pstUser->m_tbPlayer.m_nUid,
                            udwTaskId, ptbTask->m_nTask_status,
                            pstUser->m_udwBSeqNo));
                    }
                }
            }
        }
    }

    //限时任务
    for(TUINT32 udwIdx = 0; udwIdx < ptbTask->m_bTask_time.m_udwNum; ++udwIdx)
    {
        TUINT32 udwTaskId = ptbTask->m_bTask_time[udwIdx].m_ddwId;
        if(udwTaskId == 0)
        {
            continue;
        }
        TBOOL bUpdateProgress = FALSE;
        for(TUINT32 udwCondiIdx = 0; udwCondiIdx < MAX_TASK_CONDITION_LIMIT; ++udwCondiIdx)
        {
            if(ptbTask->m_bTask_time[udwIdx].astFinishCondition[udwCondiIdx].m_ddwTaskType == 0)
            {
                continue;
            }
            if(!ptbTask->m_bTask_time[udwIdx].astFinishCondition[udwCondiIdx].m_bIsStand)
            {
                TUINT32 udwCondiType = ptbTask->m_bTask_time[udwIdx].astFinishCondition[udwCondiIdx].m_ddwTaskType;
                TUINT32 udwCondiId = ptbTask->m_bTask_time[udwIdx].astFinishCondition[udwCondiIdx].m_ddwId;
                TUINT32 udwCondiValue = ptbTask->m_bTask_time[udwIdx].astFinishCondition[udwCondiIdx].m_ddwValue;
                TUINT32 udwOldNum = ptbTask->m_bTask_time[udwIdx].astFinishCondition[udwCondiIdx].m_uddwCurrValue;

                TUINT32 udwCurNum = GetOwnNum(pstCity, pstUser, udwCondiType, udwCondiId, udwCondiValue);
                ptbTask->m_bTask_time[udwIdx].astFinishCondition[udwCondiIdx].m_uddwCurrValue = udwCurNum;

                if(udwCurNum > udwOldNum)
                {
                    bTimeUpdate = TRUE;
                    bUpdateProgress = TRUE;
                }
                if(ptbTask->m_bTask_time[udwIdx].astFinishCondition[udwCondiIdx].m_uddwCurrValue >= ptbTask->m_bTask_time[udwIdx].astFinishCondition[udwCondiIdx].m_uddwNum)
                {
                    ptbTask->m_bTask_time[udwIdx].m_bIsProgress = TRUE;
                    ptbTask->m_bTask_time[udwIdx].m_ddwUpdateTime = CTimeUtils::GetUnixTime();
                    bTimeUpdate = TRUE;
                }
            }
        }
        if(bUpdateProgress)
        {
            for(TUINT32 udwCondiIdx = 0; udwCondiIdx < MAX_TASK_CONDITION_LIMIT; ++udwCondiIdx)
            {
                if(ptbTask->m_bTask_time[udwIdx].astFinishCondition[udwCondiIdx].m_ddwTaskType == 0)
                {
                    continue;
                }
                TUINT32 udwCondiType = ptbTask->m_bTask_time[udwIdx].astFinishCondition[udwCondiIdx].m_ddwTaskType;
                TUINT32 udwCondiId = ptbTask->m_bTask_time[udwIdx].astFinishCondition[udwCondiIdx].m_ddwId;
                TUINT32 udwCondiValue = ptbTask->m_bTask_time[udwIdx].astFinishCondition[udwCondiIdx].m_ddwValue;

                if(!IsSubTaskFinish(pstUser, pstCity, udwIdx, udwCondiIdx, TRUE))
                {
                    TBOOL bCanStart = CQuestLogic::IsTaskCanStart(pstUser, pstCity, udwCondiType, udwCondiId, udwCondiValue);

                    if(bCanStart)
                    {
                        ptbTask->Set_Task_check_id(udwTaskId);
                        ptbTask->Set_Task_status(EN_TASK_SHOW_STATUS_CHECK);
                        pstUser->m_bTaskUpdate = TRUE;
                        TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("CheckOwnNumUpdate_WAVE:uid=%u finish task_id=%u status=%u [seq=%u]",
                            pstUser->m_tbPlayer.m_nUid,
                            udwTaskId, ptbTask->m_nTask_status,
                            pstUser->m_udwBSeqNo));
                    }
                }
            }
        }
    }

    if(bTimeUpdate)
    {
        ptbTask->SetFlag(TbTASK_FIELD_TASK_TIME);
    }
    if(bNormalUpdate)
    {
        ptbTask->SetFlag(TbTASK_FIELD_TASK_NORMAL);
    }
    return;
}

TVOID CQuestLogic::CheckTask(SUserInfo *pstUser, SCityInfo *pstCity, TBOOL IsHanderBefore)
{
    //刷新完成度
    CQuestLogic::CheckOwnNumUpdate(pstUser, pstCity);

    //检测是否已完成
    CQuestLogic::CheckTaskFinish(pstUser, pstCity);

    //补充已完成的task
    CQuestLogic::CheckTaskRefresh(pstUser, pstCity);

    //判断check id和flag是否输出
    CQuestLogic::CheckTaskResetFlag(pstUser,pstCity);

    //给task输出排序, 并记录推荐task_id
    TBOOL bRecommandTaskRealCanStart = FALSE;
    pstUser->m_vecTaskList = CQuestLogic::GetSortTaskList(pstUser, pstCity, pstUser->m_vCheckTaskId, bRecommandTaskRealCanStart);
    if (pstUser->m_vecTaskList.size() > 0 && pstUser->m_tbTask.m_nRecommand_task_id != pstUser->m_vecTaskList[0])
    {
        pstUser->m_tbTask.Set_Recommand_task_id(pstUser->m_vecTaskList[0]);
        pstUser->m_bTaskUpdate = TRUE;
    }
    else if (pstUser->m_vecTaskList.size() == 0 && pstUser->m_tbTask.m_nRecommand_task_id != 0)
    {
        pstUser->m_tbTask.Set_Recommand_task_id(0);
        pstUser->m_bTaskUpdate = TRUE;
    }

    //if(pstUser->m_bTaskClearCmd == FALSE)
    {
        // 修正task的状态标记
        if(bRecommandTaskRealCanStart == FALSE)
        {
            if(pstUser->m_tbTask.m_nTask_status == EN_TASK_SHOW_STATUS_RECOMMAND_CAN_START)
            {
                pstUser->m_tbTask.Set_Task_status(EN_TASK_SHOW_STATUS_NORMAL);
            }
        }
        else
        {
            if(pstUser->m_tbTask.m_nTask_status == EN_TASK_SHOW_STATUS_NORMAL
                || pstUser->m_tbTask.m_nTask_status == EN_TASK_SHOW_STATUS_ADD)
            {
                pstUser->m_tbTask.Set_Task_status(EN_TASK_SHOW_STATUS_RECOMMAND_CAN_START);
            }
        }
    }    

    return;
}

TVOID CQuestLogic::SetTaskCurValue(SUserInfo *pstUser, SCityInfo *pstCity, TUINT32 udwConditionType, TUINT32 udwNumAdd, TUINT32 udwConditionId, TUINT32 udwConditionValue)
{
    if(pstUser == NULL || pstCity == NULL)
    {
        return;
    }
    CBountyLogic::SetBountyCurValue(pstUser,pstCity,udwConditionType,udwNumAdd,udwConditionId,udwConditionValue);

    TbTask *ptbTask = &pstUser->m_tbTask;
    for(TUINT32 udwIdx = 0; udwIdx < ptbTask->m_bTask_time.m_udwNum;++udwIdx)
    {
        TBOOL bUpdate = FALSE;
        TBOOL bUpdateProgress = FALSE;
        TUINT32 udwTaskId = ptbTask->m_bTask_time[udwIdx].m_ddwId;
        for(TUINT32 udwConditionIdx = 0; udwConditionIdx < MAX_TASK_CONDITION_LIMIT;++udwConditionIdx)
        {
            STaskCondiStatusNode *pstCondi = ptbTask->m_bTask_time[udwIdx].astFinishCondition;
            if(pstCondi[udwConditionIdx].m_ddwTaskType == 0)
            {
                continue;
            }
            
            if(pstCondi[udwConditionIdx].m_ddwTaskType == udwConditionType &&
                pstCondi[udwConditionIdx].m_ddwId == udwConditionId &&
                pstCondi[udwConditionIdx].m_ddwValue == udwConditionValue)
            {
                pstCondi[udwConditionIdx].m_uddwCurrValue += udwNumAdd;
                bUpdate = TRUE;
            }
            if(bUpdate)
            {
                bUpdateProgress = TRUE;    
                ptbTask->SetFlag(TbTASK_FIELD_TASK_TIME);
            }
            if(pstCondi[udwConditionIdx].m_uddwCurrValue - pstCondi[udwConditionIdx].m_uddwBeginValue >= pstCondi[udwConditionIdx].m_uddwNum)
            {
                ptbTask->m_bTask_time[udwIdx].m_bIsProgress = TRUE;
                ptbTask->m_bTask_time[udwIdx].m_ddwUpdateTime = CTimeUtils::GetUnixTime();
                ptbTask->SetFlag(TbTASK_FIELD_TASK_TIME);
            }
        }
        if(bUpdateProgress)
        {
            for(TUINT32 udwCondiIdx = 0; udwCondiIdx < MAX_TASK_CONDITION_LIMIT; ++udwCondiIdx)
            {
                if(ptbTask->m_bTask_time[udwIdx].astFinishCondition[udwCondiIdx].m_ddwTaskType == 0)
                {
                    continue;
                }
                TUINT32 udwCondiType = ptbTask->m_bTask_time[udwIdx].astFinishCondition[udwCondiIdx].m_ddwTaskType;
                TUINT32 udwCondiId = ptbTask->m_bTask_time[udwIdx].astFinishCondition[udwCondiIdx].m_ddwId;
                TUINT32 udwCondiValue = ptbTask->m_bTask_time[udwIdx].astFinishCondition[udwCondiIdx].m_ddwValue;

                if(!IsSubTaskFinish(pstUser, pstCity, udwIdx, udwCondiIdx, TRUE))
                {
                    TBOOL bCanStart = CQuestLogic::IsTaskCanStart(pstUser, pstCity, udwCondiType, udwCondiId, udwCondiValue);

                    if(bCanStart)
                    {
                        ptbTask->Set_Task_check_id(udwTaskId);
                        ptbTask->Set_Task_status(EN_TASK_SHOW_STATUS_CHECK);
                        TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("CheckOwnNumUpdate_WAVE:uid=%u finish task_id=%u status=%u [seq=%u]",
                            pstUser->m_tbPlayer.m_nUid,
                            udwTaskId, ptbTask->m_nTask_status,
                            pstUser->m_udwBSeqNo));
                    }
                }
            }
        }
    }
    for(TUINT32 udwIdx = 0; udwIdx < ptbTask->m_bTask_normal.m_udwNum;++udwIdx)
    {
        TBOOL bUpdateProgress = FALSE;
        TUINT32 udwTaskId = ptbTask->m_bTask_normal[udwIdx].m_ddwId;
        for(TUINT32 udwConditionIdx = 0; udwConditionIdx < MAX_TASK_CONDITION_LIMIT;++udwConditionIdx)
        {
            TBOOL bUpdate = FALSE;
            STaskCondiStatusNode *pstCondi = ptbTask->m_bTask_normal[udwIdx].astFinishCondition;
            if(pstCondi[udwConditionIdx].m_ddwTaskType == 0)
            {
                continue;
            }

            if(pstCondi[udwConditionIdx].m_ddwTaskType == udwConditionType &&
                pstCondi[udwConditionIdx].m_ddwId == udwConditionId &&
                pstCondi[udwConditionIdx].m_ddwValue == udwConditionValue)
            {
                pstCondi[udwConditionIdx].m_uddwCurrValue += udwNumAdd;
                bUpdate = TRUE;
            }
            if(bUpdate)
            {
                bUpdateProgress = TRUE;       
                ptbTask->SetFlag(TbTASK_FIELD_TASK_NORMAL);
            }
            if(pstCondi[udwConditionIdx].m_uddwCurrValue - pstCondi[udwConditionIdx].m_uddwBeginValue >= pstCondi[udwConditionIdx].m_uddwNum)
            {
                ptbTask->m_bTask_normal[udwIdx].m_bIsProgress = TRUE;
                ptbTask->m_bTask_normal[udwIdx].m_ddwUpdateTime = CTimeUtils::GetUnixTime();
                ptbTask->SetFlag(TbTASK_FIELD_TASK_NORMAL);
            }
        }
        if(bUpdateProgress)
        {
            for(TUINT32 udwCondiIdx = 0; udwCondiIdx < MAX_TASK_CONDITION_LIMIT; ++udwCondiIdx)
            {
                if(ptbTask->m_bTask_normal[udwIdx].astFinishCondition[udwCondiIdx].m_ddwTaskType == 0)
                {
                    continue;
                }
                TUINT32 udwCondiType = ptbTask->m_bTask_normal[udwIdx].astFinishCondition[udwCondiIdx].m_ddwTaskType;
                TUINT32 udwCondiId = ptbTask->m_bTask_normal[udwIdx].astFinishCondition[udwCondiIdx].m_ddwId;
                TUINT32 udwCondiValue = ptbTask->m_bTask_normal[udwIdx].astFinishCondition[udwCondiIdx].m_ddwValue;

                if(!IsSubTaskFinish(pstUser, pstCity, udwIdx, udwCondiIdx, FALSE))
                {
                    TBOOL bCanStart = CQuestLogic::IsTaskCanStart(pstUser, pstCity, udwCondiType, udwCondiId, udwCondiValue);
                    
                    if(bCanStart)
                    {
                        ptbTask->Set_Task_check_id(udwTaskId);
                        ptbTask->Set_Task_status(EN_TASK_SHOW_STATUS_CHECK);
                        TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("CheckOwnNumUpdate_WAVE:uid=%u finish task_id=%u status=%u [seq=%u]",
                            pstUser->m_tbPlayer.m_nUid,
                            udwTaskId, ptbTask->m_nTask_status,
                            pstUser->m_udwBSeqNo));
                    }
                }   
            }
        }
    }
    return;
}

TVOID CQuestLogic::ProcessSpTask(SUserInfo *pstUser, SCityInfo *pstCity)
{
    TbTask *ptbTask = &pstUser->m_tbTask;
    //1 加入联盟任务
    for(TUINT32 udwIdx = 0; udwIdx < ptbTask->m_bTask_time.m_udwNum; ++udwIdx)
    {
        TBOOL bUpdate = FALSE;
        for(TUINT32 udwConditionIdx = 0; udwConditionIdx < MAX_TASK_CONDITION_LIMIT; ++udwConditionIdx)
        {
            STaskCondiStatusNode *pstCondi = ptbTask->m_bTask_time[udwIdx].astFinishCondition;
            if(pstCondi[udwConditionIdx].m_ddwTaskType == 0)
            {
                continue;
            }

            if(pstCondi[udwConditionIdx].m_ddwTaskType == EN_TASK_TYPE_JOIN_ALLIANCE &&
                pstUser->m_tbPlayer.m_nAlid != 0 &&
                pstUser->m_tbPlayer.m_nAlpos != EN_ALLIANCE_POS__REQUEST &&
                pstCondi[udwConditionIdx].m_uddwCurrValue == 0)
            {
                pstCondi[udwConditionIdx].m_uddwCurrValue++;
                bUpdate = TRUE;
            }
            if(bUpdate)
            {
                ptbTask->m_bTask_time[udwIdx].m_bIsProgress = TRUE;
                ptbTask->m_bTask_time[udwIdx].m_ddwUpdateTime = CTimeUtils::GetUnixTime();
                ptbTask->SetFlag(TbTASK_FIELD_TASK_TIME);
            }
        }
    }
    for(TUINT32 udwIdx = 0; udwIdx < ptbTask->m_bTask_normal.m_udwNum; ++udwIdx)
    {
        for(TUINT32 udwConditionIdx = 0; udwConditionIdx < MAX_TASK_CONDITION_LIMIT; ++udwConditionIdx)
        {
            TBOOL bUpdate = FALSE;
            STaskCondiStatusNode *pstCondi = ptbTask->m_bTask_normal[udwIdx].astFinishCondition;
            if(pstCondi[udwConditionIdx].m_ddwTaskType == 0)
            {
                continue;
            }
            if(pstCondi[udwConditionIdx].m_ddwTaskType == EN_TASK_TYPE_JOIN_ALLIANCE &&
                pstUser->m_tbPlayer.m_nAlid != 0 &&
                pstUser->m_tbPlayer.m_nAlpos != EN_ALLIANCE_POS__REQUEST &&
                pstCondi[udwConditionIdx].m_uddwCurrValue == 0)
            {
                pstCondi[udwConditionIdx].m_uddwCurrValue++;
                bUpdate = TRUE;
            }
            if(bUpdate)
            {
                ptbTask->m_bTask_normal[udwIdx].m_bIsProgress = TRUE;
                ptbTask->m_bTask_normal[udwIdx].m_ddwUpdateTime = CTimeUtils::GetUnixTime();
                ptbTask->SetFlag(TbTASK_FIELD_TASK_NORMAL);
            }
        }
    }
    return;
}

TVOID CQuestLogic::CheckTaskCondition(SUserInfo *pstUser,SCityInfo *pstCity)
{
    TbTask *ptbTask = &pstUser->m_tbTask;
    const Json::Value &rTaskJson = CGameInfo::GetInstance()->m_oJsonRoot["game_task"];

    //task normal
    TUINT32 udwIdx = 0;
    while(udwIdx < ptbTask->m_bTask_normal.m_udwNum)
    {
        TBOOL bResetFlag = FALSE;
        TUINT32 udwTaskId = ptbTask->m_bTask_normal[udwIdx].m_ddwId;
        if(udwTaskId == 0)
        {
            ++udwIdx;
            continue;
        }
        TUINT32 udwJsonAge = rTaskJson[CCommonFunc::NumToString(udwTaskId)]["a"]["a9"].asUInt();
        if(udwJsonAge < pstUser->m_tbPlayer.m_nAge)
        {
            BITSET(ptbTask->m_bTask_finish[0].m_bitTask,udwTaskId);
            ptbTask->SetFlag(TbTASK_FIELD_TASK_FINISH);
            bResetFlag = TRUE;
        }
        else
        {
            const Json::Value &rCondiJson = rTaskJson[CCommonFunc::NumToString(udwTaskId)]["o"];
            for(TUINT32 udwJsonConIdx = 0; udwJsonConIdx < rCondiJson.size() && udwJsonConIdx < MAX_TASK_CONDITION_LIMIT; ++udwJsonConIdx)
            {
                TUINT32 udwJsonConType = rCondiJson[udwJsonConIdx][0U].asUInt();
                TUINT32 udwJsonConId = rCondiJson[udwJsonConIdx][1U].asUInt();
                TUINT32 udwJsonConNum = rCondiJson[udwJsonConIdx][2U].asUInt();
                TUINT32 udwJsonConValue = rCondiJson[udwJsonConIdx][3U].asUInt();
                TINT32 dwJsonStand = rCondiJson[udwJsonConIdx][4U].asInt();;

                STaskCondiStatusNode *pstCondition = &ptbTask->m_bTask_normal[udwIdx].astFinishCondition[udwJsonConIdx];

                if(udwJsonConType != pstCondition->m_ddwTaskType ||
                    udwJsonConId != pstCondition->m_ddwId ||
                    udwJsonConNum != pstCondition->m_uddwNum ||
                    udwJsonConValue != pstCondition->m_ddwValue ||
                    dwJsonStand != pstCondition->m_bIsStand)
                {
                    bResetFlag = TRUE;
                    break;
                }
            }
        }
        TUINT32 udwConNum = 0;
        for(TUINT32 udwJsonConIdx = 0; udwJsonConIdx < MAX_TASK_CONDITION_LIMIT; ++udwJsonConIdx)
        {
            STaskCondiStatusNode *pstCondition = &ptbTask->m_bTask_normal[udwIdx].astFinishCondition[udwJsonConIdx];
            if(pstCondition->m_ddwTaskType != 0)
            {
                udwConNum++;
            }
        }
        if(udwConNum != rTaskJson[CCommonFunc::NumToString(udwTaskId)]["o"].size())
        {
            bResetFlag = TRUE;
        }
        if(bResetFlag)
        {
            ptbTask->m_bTask_normal[udwIdx].Reset();
            if(udwIdx != ptbTask->m_bTask_normal.m_udwNum - 1)
            {
                memcpy((char*)&ptbTask->m_bTask_normal[udwIdx], (char*)&ptbTask->m_bTask_normal[ptbTask->m_bTask_normal.m_udwNum - 1], sizeof(STaskNodeNow));
            }
            ptbTask->m_bTask_normal.m_udwNum--;
            ptbTask->SetFlag(TbTASK_FIELD_TASK_NORMAL);
        }
        else
        {
            ++udwIdx;
        }
    }
    return;
}

TUINT32 CQuestLogic::GetTaskTypeEndTime(SUserInfo* pstUser, TUINT32 udwTaskType,TUINT32 udwId, TUINT32 udwValue)
{
    TUINT32 udwEndTime = 0;
    TUINT32 udwType = 0;
    TUINT32 udwLv = 0;

    TbAlliance_action * pstAction = NULL;
    TbMarch_action * pstMarch = NULL;

    switch(udwTaskType)
    {
    case EN_TASK_TYPE_CONSTRUCT_BUILDING:
        for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwSelfAlActionNum; ++udwIdx)
        {
            pstAction = &pstUser->m_atbSelfAlAction[udwIdx];
            if(pstAction->m_nSclass == EN_ACTION_SEC_CLASS__BUILDING_UPGRADE)
            {
                udwType = pstAction->m_bParam[0].m_stBuilding.m_ddwType;
                if(udwId == udwType)
                {
                    udwEndTime = pstAction->m_nEtime;
                    break;
                }
            }
        }
        break;
    case EN_TASK_TYPE_UPGRADE_BUILDING:
        for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwSelfAlActionNum; ++udwIdx)
        {
            pstAction = &pstUser->m_atbSelfAlAction[udwIdx];
            if(pstAction->m_nSclass == EN_ACTION_SEC_CLASS__BUILDING_UPGRADE)
            {
                udwType = pstAction->m_bParam[0].m_stBuilding.m_ddwType;
                udwLv = pstAction->m_bParam[0].m_stBuilding.m_ddwTargetLevel;
                if(udwId == udwType && udwLv == udwValue)
                {
                    udwEndTime = pstAction->m_nEtime;
                    break;
                }
            }
        }
        break;
    case EN_TASK_TYPE_RESEARCH:
        for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwSelfAlActionNum; ++udwIdx)
        {
            pstAction = &pstUser->m_atbSelfAlAction[udwIdx];
            if(pstAction->m_nSclass == EN_ACTION_SEC_CLASS__RESEARCH_UPGRADE)
            {
                udwType = pstAction->m_bParam[0].m_stBuilding.m_ddwType;
                if(udwId == udwType)
                {
                    udwEndTime = pstAction->m_nEtime;
                    break;
                }
            }
        }
        break;
    case EN_TASK_TYPE_TRAIN_TROOPS:
        for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwSelfAlActionNum; ++udwIdx)
        {
            pstAction = &pstUser->m_atbSelfAlAction[udwIdx];
            if(pstAction->m_nSclass == EN_ACTION_SEC_CLASS__TROOP)
            {
                udwType = pstAction->m_bParam[0].m_stTrain.m_ddwType;
                if(udwId == udwType)
                {
                    udwEndTime = pstAction->m_nEtime;
                    break;
                }
            }
        }
        break;
    case EN_TASK_TYPE_TREAT_TROOPS:
        for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwSelfAlActionNum; ++udwIdx)
        {
            pstAction = &pstUser->m_atbSelfAlAction[udwIdx];
            if(pstAction->m_nSclass == EN_ACTION_SEC_CLASS__HOS_TREAT)
            {
                udwType = pstAction->m_bParam[0].m_stTrain.m_ddwType;
                if(udwId == udwType)
                {
                    udwEndTime = pstAction->m_nEtime;
                    break;
                }
            }
        }
        break;
    case EN_TASK_TYPE_TRAIN_CITYWALL:
        for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwSelfAlActionNum; ++udwIdx)
        {
            pstAction = &pstUser->m_atbSelfAlAction[udwIdx];
            if(pstAction->m_nSclass == EN_ACTION_SEC_CLASS__FORT)
            {
                udwType = pstAction->m_bParam[0].m_stTrain.m_ddwType;
                if(udwId == udwType)
                {
                    udwEndTime = pstAction->m_nEtime;
                    break;
                }
            }
        }
        break;
    case EN_TASK_TYPE_TREAT_CITYWALL:
        for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwSelfAlActionNum; ++udwIdx)
        {
            pstAction = &pstUser->m_atbSelfAlAction[udwIdx];
            if(pstAction->m_nSclass == EN_ACTION_SEC_CLASS__FORT_REPAIR)
            {
                udwType = pstAction->m_bParam[0].m_stTrain.m_ddwType;
                if(udwId == udwType)
                {
                    udwEndTime = pstAction->m_nEtime;
                    break;
                }
            }
        }
        break;
    case EN_TASK_TYPE_GET_RESOURCE:
        udwEndTime = 0;
        for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum;++udwIdx)
        {
            pstMarch = &pstUser->m_atbMarch[udwIdx];
            udwType = CMapLogic::GetResTypeByWildType(pstUser->m_tbPlayer.m_nSid, pstMarch->m_bParam[0].m_ddwTargetType, pstMarch->m_bParam[0].m_ddwTargetLevel);
            
            if(pstMarch->m_nSclass == EN_ACTION_SEC_CLASS__OCCUPY &&
                udwType == udwId)
            {
                if(pstMarch->m_nEtime > udwEndTime)
                {
                    udwEndTime = pstMarch->m_nEtime;
                }
            }
        }
        break;
    case EN_TASK_TYPE_GET_ALL_RESOURCE:
        udwEndTime = 0;
        for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
        {
            pstMarch = &pstUser->m_atbMarch[udwIdx];
            if(pstMarch->m_nSclass == EN_ACTION_SEC_CLASS__OCCUPY)
            {
                if(pstMarch->m_nEtime > udwEndTime)
                {
                    udwEndTime = pstMarch->m_nEtime;
                }
            }
        }
        break;
    case EN_TASK_TYPE_ROB_RESOURCE:
        udwEndTime = 0;
        for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
        {
            pstMarch = &pstUser->m_atbMarch[udwIdx];
            if(pstMarch->m_nSclass == EN_ACTION_SEC_CLASS__ATTACK && 
                pstMarch->m_bParam[0].m_ddwTargetType == EN_WILD_TYPE__CITY)
            {
                if(pstMarch->m_nEtime > udwEndTime)
                {
                    udwEndTime = pstMarch->m_nEtime;
                }
            }
        }
        break;
    case EN_TASK_TYPE_RESEARCH_SUM:
        for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwSelfAlActionNum; ++udwIdx)
        {
            pstAction = &pstUser->m_atbSelfAlAction[udwIdx];
            if(pstAction->m_nSclass == EN_ACTION_SEC_CLASS__RESEARCH_UPGRADE)
            {
                udwEndTime = pstAction->m_nEtime;
                break;
            }
        }
        break;
    case EN_TASK_TYPE_TROOPS_SUM:
        udwEndTime = 0;
        for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwSelfAlActionNum; ++udwIdx)
        {
            pstAction = &pstUser->m_atbSelfAlAction[udwIdx];
            if(pstAction->m_nSclass == EN_ACTION_SEC_CLASS__TROOP || 
                EN_ACTION_SEC_CLASS__FORT == pstAction->m_nSclass)
            {
                if(udwEndTime < pstAction->m_nEtime)
                {
                    udwEndTime = pstAction->m_nEtime;
                }
            }
        }
        break;
    case EN_TASK_TYPE_GET_GOLD_RESOURCE:
        udwEndTime = 0;
        for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
        {
            pstMarch = &pstUser->m_atbMarch[udwIdx];
            udwType = CMapLogic::GetResTypeByWildType(pstUser->m_tbPlayer.m_nSid, pstMarch->m_bParam[0].m_ddwTargetType, pstMarch->m_bParam[0].m_ddwTargetLevel);
            
            if(pstMarch->m_nSclass == EN_ACTION_SEC_CLASS__OCCUPY &&
                udwType == EN_RESOURCE_TYPE__GOLD)
            {
                if(pstMarch->m_nEtime > udwEndTime)
                {
                    udwEndTime = pstMarch->m_nEtime;
                }
            }
        }
        break;
    case EN_TASK_TYPE_GET_FOOD_RESOURCE:
        udwEndTime = 0;
        for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
        {
            pstMarch = &pstUser->m_atbMarch[udwIdx];
            udwType = CMapLogic::GetResTypeByWildType(pstUser->m_tbPlayer.m_nSid, pstMarch->m_bParam[0].m_ddwTargetType, pstMarch->m_bParam[0].m_ddwTargetLevel);

            if(pstMarch->m_nSclass == EN_ACTION_SEC_CLASS__OCCUPY &&
                udwType == EN_RESOURCE_TYPE__FOOD)
            {
                if(pstMarch->m_nEtime > udwEndTime)
                {
                    udwEndTime = pstMarch->m_nEtime;
                }
            }
        }
        break;
    case EN_TASK_TYPE_GET_WOOD_RESOURCE:
        udwEndTime = 0;
        for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
        {
            pstMarch = &pstUser->m_atbMarch[udwIdx];
            udwType = CMapLogic::GetResTypeByWildType(pstUser->m_tbPlayer.m_nSid, pstMarch->m_bParam[0].m_ddwTargetType, pstMarch->m_bParam[0].m_ddwTargetLevel);

            if(pstMarch->m_nSclass == EN_ACTION_SEC_CLASS__OCCUPY &&
                udwType == EN_RESOURCE_TYPE__WOOD)
            {
                if(pstMarch->m_nEtime > udwEndTime)
                {
                    udwEndTime = pstMarch->m_nEtime;
                }
            }
        }
        break;
    case EN_TASK_TYPE_GET_STONE_RESOURCE:
        udwEndTime = 0;
        for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
        {
            pstMarch = &pstUser->m_atbMarch[udwIdx];
            udwType = CMapLogic::GetResTypeByWildType(pstUser->m_tbPlayer.m_nSid, pstMarch->m_bParam[0].m_ddwTargetType, pstMarch->m_bParam[0].m_ddwTargetLevel);

            if(pstMarch->m_nSclass == EN_ACTION_SEC_CLASS__OCCUPY &&
                udwType == EN_RESOURCE_TYPE__STONE)
            {
                if(pstMarch->m_nEtime > udwEndTime)
                {
                    udwEndTime = pstMarch->m_nEtime;
                }
            }
        }
        break;
    case EN_TASK_TYPE_GET_IRON_RESOURCE:
        udwEndTime = 0;
        for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
        {
            pstMarch = &pstUser->m_atbMarch[udwIdx];
            udwType = CMapLogic::GetResTypeByWildType(pstUser->m_tbPlayer.m_nSid, pstMarch->m_bParam[0].m_ddwTargetType, pstMarch->m_bParam[0].m_ddwTargetLevel);

            if(pstMarch->m_nSclass == EN_ACTION_SEC_CLASS__OCCUPY &&
                udwType == EN_RESOURCE_TYPE__ORE)
            {
                if(pstMarch->m_nEtime > udwEndTime)
                {
                    udwEndTime = pstMarch->m_nEtime;
                }
            }
        }
        break;
    case EN_TASK_TYPE_BUILDING_NUM:
        udwEndTime = 0;
        for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwSelfAlActionNum; ++udwIdx)
        {
            pstAction = &pstUser->m_atbSelfAlAction[udwIdx];
            if(pstAction->m_nSclass == EN_ACTION_SEC_CLASS__BUILDING_UPGRADE)
            {
                if(udwEndTime < pstAction->m_nEtime)
                {
                    udwEndTime = pstAction->m_nEtime;
                }   
            }
        }
        break;
    default:
        break;
    }
    return udwEndTime;
}

TVOID CQuestLogic::SetBuildingProgress(SUserInfo *pstUser, SCityInfo *pstCity)
{

}

TBOOL CQuestLogic::IsTaskCanStart(SUserInfo *pstUser, SCityInfo *pstCity, TUINT32 udwTaskType, TUINT32 udwId, TUINT32 udwValue, TBOOL bStrictCheck)
{
    const Json::Value& oWildResJson = CWildInfo::GetWildResInfo(pstUser->m_tbPlayer.m_nSid);

    switch(udwTaskType)
    {
    //建筑类
    case EN_TASK_TYPE_CONSTRUCT_BUILDING:
    case EN_TASK_TYPE_BUILDING_NUM:
    case EN_TASK_TYPE_UPGRADE_BUILDING:
    case EN_TASK_TYPE_UNLOCK_PLACE:
        if(bStrictCheck == TRUE)
        {
            for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwSelfAlActionNum; udwIdx++)
            {
                if (pstUser->m_atbSelfAlAction[udwIdx].m_nSuid != pstUser->m_tbPlayer.m_nUid
                    || pstUser->m_aucSelfAlActionFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
                {
                    continue;
                }
                if (pstUser->m_atbSelfAlAction[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__BUILDING_UPGRADE
                    || pstUser->m_atbSelfAlAction[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__BUILDING_REMOVE
                    || pstUser->m_atbSelfAlAction[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__REMOVE_OBSTACLE)
                {
                    return FALSE;
                }
            }
        }
        return TRUE;
    case EN_TASK_TYPE_RESEARCH:
        if(bStrictCheck == TRUE)
        {
            for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwSelfAlActionNum; udwIdx++)
            {
                if (pstUser->m_atbSelfAlAction[udwIdx].m_nSuid != pstUser->m_tbPlayer.m_nUid
                    || pstUser->m_aucSelfAlActionFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
                {
                    continue;
                }
                if (pstUser->m_atbSelfAlAction[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__RESEARCH_UPGRADE)
                {
                    return FALSE;
                }
            }
        }
        else
        {
            for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwSelfAlActionNum; udwIdx++)
            {
                if (pstUser->m_atbSelfAlAction[udwIdx].m_nSuid != pstUser->m_tbPlayer.m_nUid
                    || pstUser->m_aucSelfAlActionFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
                {
                    continue;
                }
                if (pstUser->m_atbSelfAlAction[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__RESEARCH_UPGRADE
                    && pstUser->m_atbSelfAlAction[udwIdx].m_bParam[0].m_stBuilding.m_ddwType == udwId)
                {
                    return FALSE;
                }
            }
        }
        return TRUE;
    //quest 类
    case EN_TASK_TYPE_DAILY_QUEST_FINISH:
        for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_tbQuest.m_bDaily_quest[0].m_ddwQuestNum;++udwIdx)
        {
            if(pstUser->m_tbQuest.m_bDaily_quest[0].m_stQuestCom[udwIdx].m_ddwStatus == EN_TIME_QUEST_STATUS_RUNNING)
            {
                return FALSE;
            }
        }
        return TRUE;
    case EN_TASK_TYPE_ALLIANCE_QUEST_FINISH:
        for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_tbQuest.m_bAl_quest[0].m_ddwQuestNum; ++udwIdx)
        {
            if(pstUser->m_tbQuest.m_bAl_quest[0].m_stQuestCom[udwIdx].m_ddwStatus == EN_TIME_QUEST_STATUS_RUNNING)
            {
                return FALSE;
            }
        }
        return TRUE;
    case EN_TASK_TYPE_VIP_QUEST_FINISH:
        for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_tbQuest.m_bVip_quest[0].m_ddwQuestNum; ++udwIdx)
        {
            if(pstUser->m_tbQuest.m_bVip_quest[0].m_stQuestCom[udwIdx].m_ddwStatus == EN_TIME_QUEST_STATUS_RUNNING)
            {
                return FALSE;
            }
        }
        return TRUE;

    //训练类
    case EN_TASK_TYPE_TRAIN_TROOPS:
        for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwSelfAlActionNum; ++udwIdx)
        {
            if (pstUser->m_aucSelfAlActionFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
            {
                continue;
            }
            if (CActionBase::IsPlayerOwnedAction(pstUser->m_tbPlayer.m_nUid, pstUser->m_atbSelfAlAction[udwIdx].m_nId) &&
                pstUser->m_atbSelfAlAction[udwIdx].m_nMclass == EN_ACTION_MAIN_CLASS__TRAIN_NEW &&
                pstUser->m_atbSelfAlAction[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__TROOP)
            {
                return FALSE;
            }
        }
        return TRUE;
    case EN_TASK_TYPE_TRAIN_CITYWALL:
        for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwSelfAlActionNum; ++udwIdx)
        {
            if (pstUser->m_aucSelfAlActionFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
            {
                continue;
            }
            if (CActionBase::IsPlayerOwnedAction(pstUser->m_tbPlayer.m_nUid, pstUser->m_atbSelfAlAction[udwIdx].m_nId) &&
                pstUser->m_atbSelfAlAction[udwIdx].m_nMclass == EN_ACTION_MAIN_CLASS__TRAIN_NEW &&
                pstUser->m_atbSelfAlAction[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__FORT)
            {
                return FALSE;
            }
            if (CActionBase::IsPlayerOwnedAction(pstUser->m_tbPlayer.m_nUid, pstUser->m_atbSelfAlAction[udwIdx].m_nId) &&
                pstUser->m_atbSelfAlAction[udwIdx].m_nMclass == EN_ACTION_MAIN_CLASS__TRAIN_NEW &&
                pstUser->m_atbSelfAlAction[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__FORT_REPAIR)
            {
                return FALSE;
            }
        }
        return TRUE;

    //战争类
    case EN_TASK_TYPE_GET_GOLD_RESOURCE:
    case EN_TASK_TYPE_GET_FOOD_RESOURCE:
    case EN_TASK_TYPE_GET_WOOD_RESOURCE:
    case EN_TASK_TYPE_GET_STONE_RESOURCE:
    case EN_TASK_TYPE_GET_IRON_RESOURCE:
    case EN_TASK_TYPE_GET_RESOURCE:
    case EN_TASK_TYPE_GET_ALL_RESOURCE:
        for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; udwIdx++)
        {
            if (pstUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__OCCUPY
                && pstUser->m_atbMarch[udwIdx].m_nStatus != EN_MARCH_STATUS__RETURNING)
            {
                ostringstream oss;
                oss.str("");
                oss << (TINT32)pstUser->m_atbMarch[udwIdx].m_bParam[0].m_ddwTargetType;

                if (oWildResJson.isMember(oss.str()) && oWildResJson[oss.str()]["a0"]["a1"].asUInt() == 0
                    && oWildResJson[oss.str()]["a0"]["a2"].asUInt() == 0)
                {
                    TUINT32 udwResType = oWildResJson[oss.str()]["a0"]["a6"].asUInt();
                    if (udwTaskType == EN_TASK_TYPE_GET_ALL_RESOURCE || udwResType == udwId)
                    {
                        return FALSE;
                    }
                }
            }
        }
    case EN_TASK_TYPE_ALLIANCE_TRANSPORT:
    case EN_TASK_TYPE_ALLIANCE_REINFORCE:
    case EN_TASK_TYPE_OCCUPY_RESOURCE_PATCH:
    case EN_TASK_TYPE_ROB_RESOURCE:
        if(pstCity)
        {
            if (pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_QUEUE_NUM].m_ddwBuffTotal > pstCity->m_stActionStat.m_ucDoingMarchNum)
            {
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }
        else
        {
            return FALSE;
        }
    //不可跳转开始
    case EN_TASK_TYPE_RANDOM_REWARD_RESOURCE_NUM:
    case EN_TASK_TYPE_RANDOM_REWARD_GEM_NUM:
        return FALSE;
        
    case EN_TASK_TYPE_CHANGE_PLAYER_NAME:
    case EN_TASK_TYPE_CHANGE_CITY_NAME:
    case EN_TASK_TYPE_CHANGE_PLAYER_ICON:
            return TRUE;
    case EN_TASK_TYPE_JOIN_ALLIANCE:
    case EN_TASK_TYPE_CHECK_ALLIANCE_HELP:
    case EN_TASK_TYPE_ALLIANCE_COMMENT:
    case EN_TASK_TYPE_VISIT_WORLD_MAP:
    case EN_TASK_TYPE_VIP_UPGRADE:
    case EN_TASK_TYPE_CHECK_MAIL:
    case EN_TASK_TYPE_SEND_MAIL:
    case EN_TASK_TYPE_CHECK_REPORT:
    case EN_TASK_TYPE_SEND_ALLIANCE_MAIL:
    case EN_TASK_TYPE_ASSIGN_HERO_SKILLS:
    case EN_TASK_TYPE_ALLIANCE_CHAT:
    case EN_TASK_TYPE_EDIT_CITY:
    case EN_TASK_TYPE_UPGRADE_HERO:
    case EN_TASK_TYPE_INCREASE_FOOD_PRODUCTION:
    case EN_TASK_TYPE_INCREASE_WOOD_PRODUCTION:
    case EN_TASK_TYPE_INCREASE_STONE_PRODUCTION:
    case EN_TASK_TYPE_INCREASE_IRON_PRODUCTION:
    case EN_TASK_TYPE_INCREASE_GOLD_PRODUCTION:
    case EN_TASK_TYPE_RESOURCE_PRODUCTION_BUFF:
        return TRUE;
    //普通类 
    default:
        return FALSE;
    }
}

TVOID CQuestLogic::CheckFirstLoginStatus(SUserInfo *pstUser, SCityInfo *pstCity,TBOOL bIsFirstLogin)
{
    //TUINT64 udwStatus = EN_TASK_SHOW_STATUS_NORMAL;
    //TBOOL bHaveNew = FALSE;
    //TBOOL bHaveTaskCanStart = FALSE;
    //TBOOL bHaveProgress = FALSE;

    if(bIsFirstLogin)
    {
        //if (pstUser->m_tbTask.m_bTask_normal.m_udwNum)
        //{
        //    pstUser->m_tbTask.Set_Task_status(EN_TASK_SHOW_STATUS_RECOMMAND_CAN_START);
        //}
        pstUser->m_tbTask.Set_Task_open_window_flag(0);
    }
}

TUINT32 CQuestLogic::GetPriorityTask(vector<TUINT32> vTaskId)
{
    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("GetPriorityTask:vector_size=%u", vTaskId.size()));
    if(vTaskId.size() == 0)
    {
        return 0;
    }
    const Json::Value &rTaskJson = CGameInfo::GetInstance()->m_oJsonRoot["game_task"];

    TUINT32 udwMinPriorityId = 0;
    TUINT32 udwMinPriority = INT_MAX;
    for(vector<TUINT32>::iterator it = vTaskId.begin(); it != vTaskId.end();++it)
    {
        if(rTaskJson[CCommonFunc::NumToString(*it)]["a"]["a0"].asUInt() < udwMinPriority)
        {
            udwMinPriorityId = (*it);
            udwMinPriority = rTaskJson[CCommonFunc::NumToString(*it)]["a"]["a0"].asUInt();
        }
    }
    return udwMinPriorityId;
}

TBOOL CQuestLogic::CompareTaskPriority(STaskPriority sTaskA, STaskPriority sTaskB)
{
    TBOOL bCompare = TRUE;
    if(sTaskB.bCanStart && !sTaskA.bCanStart)
    {
        bCompare =  FALSE;
    }
    if(sTaskA.bCanStart && !sTaskB.bCanStart)
    {
        bCompare = TRUE;
    }
    if((!sTaskA.bCanStart && !sTaskB.bCanStart) || (sTaskA.bCanStart && sTaskB.bCanStart))
    {
        const Json::Value &rTaskJson = CGameInfo::GetInstance()->m_oJsonRoot["game_task"];
        if(rTaskJson[CCommonFunc::NumToString(sTaskA.ddwTaskId)]["a"]["a0"].asUInt() < rTaskJson[CCommonFunc::NumToString(sTaskB.ddwTaskId)]["a"]["a0"].asUInt())
        {
            bCompare = TRUE;
        }
        else
        {
            bCompare = FALSE;
        }
    }
    return bCompare;
}

TBOOL CQuestLogic::CompareTaskPriorityNoState( STaskPriority sTaskA, STaskPriority sTaskB )
{
    if(sTaskA.dwPriority < sTaskB.dwPriority)
    {
        return TRUE;
    }
    return FALSE;
}

vector<TUINT32> CQuestLogic::GetSortTaskList(SUserInfo *pstUser, SCityInfo *pstCity,vector<TUINT32> &vTaskCanStart, TBOOL &bTopCanStart)
{
    TbTask *ptbTask = &pstUser->m_tbTask;
    vector<STaskPriority> vTaskPriority;
    STaskPriority sTaskPriority;
    vTaskCanStart.clear();

    bTopCanStart = FALSE;

    //普通任务
    for(TUINT32 udwIdx = 0; udwIdx < ptbTask->m_bTask_normal.m_udwNum; ++udwIdx)
    {
        TBOOL bCanStart = FALSE;
        TBOOL bFinish = TRUE;
        sTaskPriority.Reset();
        TUINT32 udwTaskId = ptbTask->m_bTask_normal[udwIdx].m_ddwId;
        if(udwTaskId == 0)
        {
            continue;
        }
        
        for(TUINT32 udwCondiIdx = 0; udwCondiIdx < MAX_TASK_CONDITION_LIMIT; ++udwCondiIdx)
        {
            if(ptbTask->m_bTask_normal[udwIdx].astFinishCondition[udwCondiIdx].m_ddwTaskType == 0)
            {
                continue;
            }
            TUINT32 udwCondiType = ptbTask->m_bTask_normal[udwIdx].astFinishCondition[udwCondiIdx].m_ddwTaskType;
            TUINT32 udwCondiId = ptbTask->m_bTask_normal[udwIdx].astFinishCondition[udwCondiIdx].m_ddwId;
            TUINT32 udwCondiValue = ptbTask->m_bTask_normal[udwIdx].astFinishCondition[udwCondiIdx].m_ddwValue;

            //操作型
            bFinish = CQuestLogic::IsSubTaskFinish(pstUser, pstCity, udwIdx, udwCondiIdx, FALSE);
            if(bFinish == TRUE)
            {
                continue;
            }

            bCanStart = CQuestLogic::IsTaskCanStart(pstUser, pstCity, udwCondiType, udwCondiId, udwCondiValue);
            if(bCanStart == TRUE)
            {
                break;
            }
        }
        if(bCanStart)
        {
            vTaskCanStart.push_back(udwTaskId);
        }

        sTaskPriority.ddwTaskId = udwTaskId;
        sTaskPriority.bCanStart = bCanStart;
        sTaskPriority.pstTask = &ptbTask->m_bTask_normal[udwIdx];
        vTaskPriority.push_back(sTaskPriority);

        TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("GetSortTaskList:one [uid=%u] [ task_id=%u can_start=%u b_finish=%u] [seq=%u]",
            pstUser->m_tbPlayer.m_nUid,
            udwTaskId,
            bCanStart,
            bFinish,
            pstUser->m_udwBSeqNo));

    }
    //限时任务
    for(TUINT32 udwIdx = 0; udwIdx < ptbTask->m_bTask_time.m_udwNum; ++udwIdx)
    {
        TBOOL bCanStart = FALSE;
        sTaskPriority.Reset();
        TUINT32 udwTaskId = ptbTask->m_bTask_time[udwIdx].m_ddwId;
        if(udwTaskId == 0)
        {
            continue;
        }
        
        for(TUINT32 udwCondiIdx = 0; udwCondiIdx < MAX_TASK_CONDITION_LIMIT; ++udwCondiIdx)
        {
            if(ptbTask->m_bTask_time[udwIdx].astFinishCondition[udwCondiIdx].m_ddwTaskType == 0)
            {
                continue;
            }
            TUINT32 udwCondiType = ptbTask->m_bTask_time[udwIdx].astFinishCondition[udwCondiIdx].m_ddwTaskType;
            TUINT32 udwCondiId = ptbTask->m_bTask_time[udwIdx].astFinishCondition[udwCondiIdx].m_ddwId;
            TUINT32 udwCondiValue = ptbTask->m_bTask_time[udwIdx].astFinishCondition[udwCondiIdx].m_ddwValue;

            TBOOL bFinish = TRUE;
            //操作型
            bFinish = CQuestLogic::IsSubTaskFinish(pstUser, pstCity, udwIdx, udwCondiIdx, TRUE);
            if(bFinish == TRUE)
            {
                continue;
            }

            bCanStart = CQuestLogic::IsTaskCanStart(pstUser, pstCity, udwCondiType, udwCondiId, udwCondiValue);
            if(bCanStart == TRUE)
            {
                break;
            }
            
        }
        if(bCanStart)
        {
            vTaskCanStart.push_back(udwTaskId);
        }
        sTaskPriority.ddwTaskId = udwTaskId;
        sTaskPriority.bCanStart = bCanStart;
        sTaskPriority.pstTask = &ptbTask->m_bTask_time[udwIdx];
        vTaskPriority.push_back(sTaskPriority);
    }

    sort(vTaskPriority.begin(), vTaskPriority.end(), CQuestLogic::CompareTaskPriority);

    //wave@20160614
    if(vTaskPriority.size())
    {
        bTopCanStart = CheckTaskRealCanStart(pstUser, pstCity, vTaskPriority[0].pstTask);
    }

    vector<TUINT32> vSortTask;
    for(vector<STaskPriority>::iterator it = vTaskPriority.begin(); it < vTaskPriority.end();++it)
    {
        vSortTask.push_back(it->ddwTaskId);
    }
    return vSortTask;
}

TVOID CQuestLogic::CheckTaskResetFlag(SUserInfo *pstUser, SCityInfo *pstCity)
{
    TbTask *ptbTask = &pstUser->m_tbTask;
    const Json::Value &rTaskJson = CGameInfo::GetInstance()->m_oJsonRoot["game_task"];

    TUINT32 udwCheckTaskId = ptbTask->Get_Task_check_id();
    if(udwCheckTaskId == 0 || ptbTask->m_nTask_status < EN_TASK_SHOW_STATUS_CHECK)
    {
        return;
    }
    
    //非重复任务
    if(0 == rTaskJson[CCommonFunc::NumToString(udwCheckTaskId)]["a"]["a7"].asUInt())
    {
        for(TUINT32 udwIdx = 0; udwIdx < ptbTask->m_bTask_normal.m_udwNum;++udwIdx)
        {
            if(ptbTask->m_bTask_normal[udwIdx].m_ddwId == 0)
            {
                continue;
            }
            if(ptbTask->m_bTask_normal[udwIdx].m_ddwId != udwCheckTaskId)
            {
                continue;
            }
            
            TBOOL bCanStar = FALSE;
            for(TUINT32 udwCondiIdx = 0; udwCondiIdx < MAX_TASK_CONDITION_LIMIT; ++udwCondiIdx)
            {
                TUINT32 udwConType = ptbTask->m_bTask_normal[udwIdx].astFinishCondition[udwCondiIdx].m_ddwTaskType;
                TUINT32 udwConId = ptbTask->m_bTask_normal[udwIdx].astFinishCondition[udwCondiIdx].m_ddwId;
                TUINT32 udwConValue = ptbTask->m_bTask_normal[udwIdx].astFinishCondition[udwCondiIdx].m_ddwValue;
                if(udwConType == 0)
                {
                    continue;
                }
                if(IsSubTaskFinish(pstUser, pstCity, udwIdx, udwCondiIdx, FALSE))
                {
                    continue;
                }
                
                if(CQuestLogic::IsTaskCanStart(pstUser, pstCity, udwConType, udwConId, udwConValue))
                {
                    bCanStar = TRUE;
                    break;
                }
            }
            if(bCanStar == FALSE)
            {
                ptbTask->Set_Task_check_id(0);
                ptbTask->Set_Task_status(EN_TASK_SHOW_STATUS_NORMAL);
                pstUser->m_bTaskUpdate = TRUE;
            }
        }
    }
    else
    {
        for(TUINT32 udwIdx = 0; udwIdx < ptbTask->m_bTask_time.m_udwNum; ++udwIdx)
        {
            if(ptbTask->m_bTask_time[udwIdx].m_ddwId != udwCheckTaskId)
            {
                continue;
            }
            if(ptbTask->m_bTask_time[udwIdx].m_ddwId != udwCheckTaskId)
            {
                continue;
            }

            TBOOL bCanStar = FALSE;
            for(TUINT32 udwCondiIdx = 0; udwCondiIdx < MAX_TASK_CONDITION_LIMIT; ++udwCondiIdx)
            {
                TUINT32 udwConType = ptbTask->m_bTask_time[udwIdx].astFinishCondition[udwCondiIdx].m_ddwTaskType;
                TUINT32 udwConId = ptbTask->m_bTask_time[udwIdx].astFinishCondition[udwCondiIdx].m_ddwId;
                TUINT32 udwConValue = ptbTask->m_bTask_time[udwIdx].astFinishCondition[udwCondiIdx].m_ddwValue;
                if(udwConType == 0)
                {
                    continue;
                }
                if(IsSubTaskFinish(pstUser, pstCity, udwIdx, udwCondiIdx, TRUE))
                {
                    continue;
                }
                if(CQuestLogic::IsTaskCanStart(pstUser, pstCity, udwConType, udwConId, udwConValue))
                {
                    bCanStar = TRUE;
                    break;
                }
            }
            if(bCanStar == FALSE)
            {
                ptbTask->Set_Task_check_id(0);
                ptbTask->Set_Task_status(EN_TASK_SHOW_STATUS_NORMAL);
                pstUser->m_bTaskUpdate = TRUE;
            }
        }
    }
}

TBOOL CQuestLogic::IsNormalTaskCheckNow( SUserInfo *pstUser, TUINT32 udwTaskId )
{
    TbTask *ptbTask = &pstUser->m_tbTask;
    const Json::Value &rTaskJson = CGameInfo::GetInstance()->m_oJsonRoot["game_task"];

    //非重复任务
    if(rTaskJson[CCommonFunc::NumToString(udwTaskId)]["a"]["a7"].asUInt())
    {
        return FALSE;
    }
    
    //等级
    if(rTaskJson[CCommonFunc::NumToString(udwTaskId)]["a"]["a2"].asUInt() > pstUser->m_tbPlayer.m_nLevel)
    {
        return FALSE;
    }

    /*
    //时代
    if(rTaskJson[CCommonFunc::NumToString(udwTaskId)]["a"]["a9"].asInt() != -1 &&
        rTaskJson[CCommonFunc::NumToString(udwTaskId)]["a"]["a9"].asInt() < pstUser->m_tbPlayer.m_nAge)
    {
        return FALSE;
    }
    */

    //是否已完成该任务
    if(BITTEST(ptbTask->m_bTask_finish[0].m_bitTask, udwTaskId))
    {
        return FALSE;
    }

    return TRUE;
}

TINT32 CQuestLogic::GetIdxInNowTaskList( SUserInfo *pstUser, TUINT32 udwTaskId, STaskNodeNow* pstTaskList, TUINT32 udwListNum )
{
    TINT32 dwRetIdx = -1;
    //是否在当前任务中
    for(TUINT32 udwIdx = 0; udwIdx < udwListNum; ++udwIdx)
    {
        if(udwTaskId == pstTaskList[udwIdx].m_ddwId)
        {
            dwRetIdx = udwIdx;
            break;
        }
    }
    
    return dwRetIdx;
}

TBOOL CQuestLogic::IsSpecialTaskFinish( SUserInfo *pstUser, SCityInfo *pstCity, TUINT32 udwTaskId )
{
    if(CGameInfo::GetInstance()->m_oJsonRoot["game_task"].isMember(CCommonFunc::NumToString(udwTaskId)) == false)
    {
        return FALSE;
    }

    TBOOL bFinish = TRUE;
    const Json::Value &rTaskJson = CGameInfo::GetInstance()->m_oJsonRoot["game_task"][CCommonFunc::NumToString(udwTaskId)];
    STaskCondiStatusNode stCondition;
    for(TUINT32 udwIdx = 0; udwIdx < rTaskJson["o"].size(); ++udwIdx)
    {
        stCondition.Reset();
        stCondition.m_ddwTaskType = rTaskJson["o"][udwIdx][0U].asUInt();
        stCondition.m_ddwId = rTaskJson["o"][udwIdx][1U].asUInt();
        stCondition.m_uddwNum = rTaskJson["o"][udwIdx][2U].asUInt();
        stCondition.m_ddwValue = rTaskJson["o"][udwIdx][3U].asUInt();
        stCondition.m_bIsStand = rTaskJson["o"][udwIdx][4U].asUInt();

        if(stCondition.m_ddwTaskType == 0)
        {
            continue;
        }

        //操作型
        if(stCondition.m_bIsStand)
        {
            return FALSE;
        }
        else
        {
            stCondition.m_uddwCurrValue = GetOwnNum(pstCity, pstUser, stCondition.m_ddwTaskType, stCondition.m_ddwId, stCondition.m_ddwValue);
            if (stCondition.m_uddwCurrValue < stCondition.m_uddwNum)
            {
                bFinish = FALSE;
                break;
            }
        }
    }
    return bFinish;
}

TINT32 CQuestLogic::CheckSpecialTaskFinish( SUserInfo *pstUser, SCityInfo *pstCity )
{
    TUINT32 udwTaskId = 0;
    const Json::Value &rTaskJson = CGameInfo::GetInstance()->m_oJsonRoot["game_special_task"]["a1"];
    TbTask *ptbTask = &pstUser->m_tbTask;

    for(TUINT32 idx = 0; idx < rTaskJson.size(); idx++)
    {
        udwTaskId = rTaskJson[idx].asUInt();

        //是否已完成该任务
        if(BITTEST(ptbTask->m_bTask_finish[0].m_bitTask, udwTaskId))
        {
            TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("TaskFinish:CheckSpecialTaskFinish:uid=%u task_id=%u has finished [seq=%u]", 
                pstUser->m_tbPlayer.m_nUid, udwTaskId, pstUser->m_udwBSeqNo));
            continue;
        }
        
        //检测是否完成
        if(IsSpecialTaskFinish(pstUser, pstCity, udwTaskId))
        {
            if(pstUser->m_udwTmpFinishNum >= MAX_TASK_CURR_NORMAL_LIMIT + MAX_TASK_CURR_TIME_LIMIT)
            {
                break;
            }
            pstUser->m_audwTmpFinishTaskId[pstUser->m_udwTmpFinishNum] = udwTaskId;
            pstUser->m_udwTmpFinishNum++;

            GetTaskRewardById(pstUser, pstCity, udwTaskId);
            BITSET(ptbTask->m_bTask_finish[0].m_bitTask, udwTaskId);
            ptbTask->SetFlag(TbTASK_FIELD_TASK_FINISH);

            CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPE__TASK_FINISH, pstUser->m_tbPlayer.m_nUid, FALSE, udwTaskId);

            //如有task完成，清理check和status标记
            ptbTask->Set_Task_check_id(0);
            ptbTask->Set_Task_status(EN_TASK_SHOW_STATUS_NORMAL);

            TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("TaskFinish:CheckSpecialTaskFinish:uid=%u task_id=%u finish success [tmp_finish_num=%u finish_id=%u] status=%ld [seq=%u]", 
                pstUser->m_tbPlayer.m_nUid,
                udwTaskId,
                pstUser->m_udwTmpFinishNum,
                pstUser->m_audwTmpFinishTaskId[pstUser->m_udwTmpFinishNum-1],
                ptbTask->m_nTask_status,
                pstUser->m_udwBSeqNo));

            break;
        }
        else
        {
            TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("TaskFinish:CheckSpecialTaskFinish:uid=%u task_id=%u not finish [seq=%u]", 
                pstUser->m_tbPlayer.m_nUid, udwTaskId, pstUser->m_udwBSeqNo));
        }
    }

    return 0;
}

TBOOL CQuestLogic::IsSpecialTask( TUINT32 udwTaskId )
{
    TBOOL bRetCode = FALSE;
    const Json::Value &rTaskJson = CGameInfo::GetInstance()->m_oJsonRoot["game_special_task"]["a1"];

    for(TUINT32 idx = 0; idx < rTaskJson.size(); idx++)
    {
        if(udwTaskId == rTaskJson[idx].asUInt())
        {
            bRetCode = TRUE;
            break;
        }
    }
    return bRetCode;
}

TBOOL CQuestLogic::IsTaskNeedFinishShow( TUINT32 udwTaskId )
{
    TUINT32 udwFlag = CGameInfo::GetInstance()->m_oJsonRoot["game_task"][CCommonFunc::NumToString(udwTaskId)]["a"]["a11"].asInt();
    if(udwFlag)
    {
        return TRUE;
    }
    return FALSE;
}

TBOOL CQuestLogic::CheckTaskRealCanStart( SUserInfo *pstUser, SCityInfo *pstCity, STaskNodeNow *pstTask )
{
    if(pstTask == NULL || pstTask->m_ddwId == 0)
    {
        return FALSE;
    }
    
    TBOOL bFinish = FALSE;
    TBOOL bCanStart = FALSE;
    for(TUINT32 udwCondiIdx = 0; udwCondiIdx < MAX_TASK_CONDITION_LIMIT; ++udwCondiIdx)
    {
        if(pstTask->astFinishCondition[udwCondiIdx].m_ddwTaskType == 0)
        {
            continue;
        }
        TUINT32 udwCondiType = pstTask->astFinishCondition[udwCondiIdx].m_ddwTaskType;
        TUINT32 udwCondiId = pstTask->astFinishCondition[udwCondiIdx].m_ddwId;
        TUINT32 udwCondiValue = pstTask->astFinishCondition[udwCondiIdx].m_ddwValue;

        //操作型
        bFinish = CQuestLogic::IsSubTaskFinish(pstUser, pstCity, pstTask, udwCondiIdx);
        if(bFinish == TRUE)
        {
            continue;
        }

        bCanStart = CQuestLogic::IsTaskCanStart(pstUser, pstCity, udwCondiType, udwCondiId, udwCondiValue, TRUE);
        if(bCanStart == TRUE)
        {
            break;
        }
    }

    return bCanStart;
}
