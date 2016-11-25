#include "quest_notic_logic.h"
#include "game_info.h"
#include "quest_logic.h"
#include "time_utils.h"
#include "action_base.h"
#include "city_base.h"
#include "document.h"
#include "msg_base.h"


TbMarch_action *CQuestNoticLogic::GetTaskNoticAction(TbMarch_action *pstActionList, TUINT32 udwActionNum)
{
    for(TUINT32 dwIdx = 0; dwIdx < udwActionNum; ++dwIdx)
    {
        if(EN_ACTION_SEC_CLASS__NOTI_TIMER == pstActionList[dwIdx].m_nSclass)
        {
            return &pstActionList[dwIdx];
        }
    }
    return NULL;
}

TVOID CQuestNoticLogic::CheckTaskNotic(SUserInfo *pstUserInfo, TbMarch_action *ptbAction)
{
    //TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    TbLogin *ptbLogin = &pstUserInfo->m_tbLogin;
    TbQuest *ptbQuest = &pstUserInfo->m_tbQuest;
    TBOOL IsQuestRunning = FALSE;
    
    //TBOOL IsTopQuestClaim = FALSE;
    
    // top_quest
    //CQuestLogic::CheckTopQuestCanClaim(pstUserInfo, IsTopQuestClaim);
    //if(FALSE == IsTopQuestClaim)
    //{   
    //    TSE_LOG_INFO(CGameInfo::GetInstance()->m_poLog, ("notictask: top quest can not claim [uid=%lld] [seq=%u]", 
    //                                                     ptbLogin->Get_Uid(), 
    //                                                     pstUserInfo->m_udwBSeqNo));
    //    BITCLEAR(ptbAction->m_bNotic_task_flag[0].m_bitFlag, EN_TOP_QUEST);
    //    ptbAction->SetFlag(TbMARCH_ACTION_FIELD_NOTIC_TASK_FLAG);
    //}
    //daily
    TUINT32 udwQuestFinishTime = CQuestLogic::CheckQuestNodeRunning(&ptbQuest->m_bDaily_quest[0], IsQuestRunning);
    if(TRUE == IsQuestRunning)
    {
        TSE_LOG_INFO(CGameInfo::GetInstance()->m_poLog, ("notictask: daily quest running [uid=%lld] [seq=%u]", \
                                                         ptbLogin->Get_Uid(), \
                                                         pstUserInfo->m_udwBSeqNo));
        BITCLEAR(ptbAction->m_bNotic_task_flag[0].m_bitFlag, EN_DAILY_QUEST);
        ptbAction->SetFlag(TbMARCH_ACTION_FIELD_NOTIC_TASK_FLAG);
    }  

    // alliance quest 
    udwQuestFinishTime = CQuestLogic::CheckQuestNodeRunning(&ptbQuest->m_bAl_quest[0], IsQuestRunning); 
    if(TRUE == IsQuestRunning)
    {
        TSE_LOG_INFO(CGameInfo::GetInstance()->m_poLog, ("notictask: alliance quest running [uid=%lld] [seq=%u]", \
                                                         ptbLogin->Get_Uid(), \
                                                         pstUserInfo->m_udwBSeqNo));
        BITCLEAR(ptbAction->m_bNotic_task_flag[0].m_bitFlag, EN_ALLIANCE_QUEST);
        ptbAction->SetFlag(TbMARCH_ACTION_FIELD_NOTIC_TASK_FLAG);
    }
    
    // vip quest 
    udwQuestFinishTime = CQuestLogic::CheckQuestNodeRunning(&ptbQuest->m_bVip_quest[0], IsQuestRunning); 
    if(TRUE == IsQuestRunning)
    {
        TSE_LOG_INFO(CGameInfo::GetInstance()->m_poLog, ("notictask: vip quest running [uid=%lld] [seq=%u]", \
                                                         ptbLogin->Get_Uid(), \
                                                         pstUserInfo->m_udwBSeqNo));
        BITCLEAR(ptbAction->m_bNotic_task_flag[0].m_bitFlag, EN_VIP_QUEST);
        ptbAction->SetFlag(TbMARCH_ACTION_FIELD_NOTIC_TASK_FLAG);
    }

    // mistery quest
    udwQuestFinishTime = CQuestLogic::CheckQuestNodeRunning(&ptbQuest->m_bTimer_gift[0], IsQuestRunning);
    if(TRUE == IsQuestRunning)
    {
        TSE_LOG_INFO(CGameInfo::GetInstance()->m_poLog, ("notictask: mistery quest running [uid=%lld] [seq=%u]", \
                                                         ptbLogin->Get_Uid(), \
                                                         pstUserInfo->m_udwBSeqNo));
        BITCLEAR(ptbAction->m_bNotic_task_flag[0].m_bitFlag, EN_MISTERY_QUEST);
        ptbAction->SetFlag(TbMARCH_ACTION_FIELD_NOTIC_TASK_FLAG);
    }
}

TVOID CQuestNoticLogic::ShowNoticFlag(SUserInfo *pstUserInfo, TbMarch_action *ptbAction)
{
    for(TINT32 dwIdx = 0; dwIdx < 10; ++dwIdx)
    {
        TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("notictask:[action_id=%lld] notic task action flag [%ld] [seq=%u]", \
                            ptbAction->m_nId, BITTEST(ptbAction->m_bNotic_task_flag[0].m_bitFlag, dwIdx), \
                                                          pstUserInfo->m_udwBSeqNo));
    }
}


