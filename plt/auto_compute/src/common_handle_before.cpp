#include "common_handle_before.h"
#include "time_utils.h"
#include "production_system.h"
#include "common_base.h"
#include "tool_base.h"
#include "common_json.h"
#include "action_base.h"
#include "buffer_base.h"
#include "quest_logic.h"
#include "bounty_logic.h"
#include "common_func.h"
#include "game_evaluate_logic.h"
#include "global_serv.h"
#include "backpack_logic.h"
#include "player_base.h"
#include "sendmessage_base.h"

TINT32 CAuCommonBefore::AuCommonHandleBefore(SSession *pstSession, SUserInfo* pstUser)
{
    //1. 新手保护状态修正
    if ((pstUser->m_tbPlayer.Get_Status() & EN_CITY_STATUS__NEW_PROTECTION) &&
        (CActionBase::GetActionByBufferId(pstUser->m_atbAction, pstUser->m_udwActionNum, EN_BUFFER_INFO_PEACE_TIME) == NULL))
    {
        pstUser->m_tbPlayer.Set_Status(pstUser->m_tbPlayer.Get_Status() ^ EN_CITY_STATUS__NEW_PROTECTION);
    }


    //2. 日志评估系统的原始数据记录必须在逻辑处理前和拉取数据后添加
    SReqInfo stReqInfo;
    TCHAR szKey[MAX_REQ_PARAM_KEY_NUM][DEFAULT_PARAM_STR_LEN] = { 0 };
    switch (pstSession->m_stActionTable.dwTableType)
    {
    case EN_UID_ACTION:
        // 游戏评估系统需要保存的raw ex_data    
        // source        
        stReqInfo.Reset();
        stReqInfo.SetValue(pstSession->m_stReqAction.m_nSid, 0, pstUser->m_tbPlayer.m_nCid, CCommonFunc::NumToString(pstSession->m_stReqAction.m_nSclass), "", szKey, 0);
        CGameEvaluateLogic::SaveGameEvaluateExData(&stReqInfo, &pstSession->m_stSourceUser, EN_EX_DATA_USER_TYPE_SOURCE, EN_EX_DATA_TYPE_RAW);
        break;
    case EN_AID_ACTION:
        // 游戏评估系统需要保存的raw ex_data    
        // source        
        stReqInfo.Reset();
        stReqInfo.SetValue(pstSession->m_stReqAlAction.m_nSid, 0, pstUser->m_tbPlayer.m_nCid, CCommonFunc::NumToString(pstSession->m_stReqAlAction.m_nSclass), "", szKey, 0);
        CGameEvaluateLogic::SaveGameEvaluateExData(&stReqInfo, &pstSession->m_stSourceUser, EN_EX_DATA_USER_TYPE_SOURCE, EN_EX_DATA_TYPE_RAW);
        break;
    case EN_MARCH_ACTION:
        // 游戏评估系统需要保存的raw ex_data    
        // source        
        stReqInfo.Reset();
        stReqInfo.SetValue(pstSession->m_stReqMarch.m_nSid, 0, pstSession->m_stReqMarch.m_nScid, CCommonFunc::NumToString(pstSession->m_stReqMarch.m_nSclass), "", szKey, 0);
        CGameEvaluateLogic::SaveGameEvaluateExData(&stReqInfo, &pstSession->m_stSourceUser, EN_EX_DATA_USER_TYPE_SOURCE, EN_EX_DATA_TYPE_RAW);

        // target
        if (0 != pstSession->m_stReqMarch.m_nTuid
            && pstSession->m_stReqMarch.m_nTuid != pstSession->m_stReqMarch.m_nSuid)
        {
            stReqInfo.Reset();
            stReqInfo.SetValue(pstSession->m_stReqMarch.m_nSid, 0, pstSession->m_stReqMarch.m_nTpos, CCommonFunc::NumToString(pstSession->m_stReqMarch.m_nSclass), "", szKey, 0);
            CGameEvaluateLogic::SaveGameEvaluateExData(&stReqInfo, &pstSession->m_stTargetUser, EN_EX_DATA_USER_TYPE_TARGET, EN_EX_DATA_TYPE_RAW);
            pstSession->m_stSourceUser.m_bGameEvaluateType = true;
        }
        break;
    default:
        break;
    }


    //3. buffer 计算
    if (pstUser->m_tbPlayer.m_nUid > 0)
    {
//         CBufferBase::ComputeBuffInfo(&pstUser->m_stCityInfo, pstUser, pstSession->m_atbIdol, pstSession->m_udwIdolNum, &pstSession->m_tbThrone, &pstSession->m_stTitleInfoList);
//         //时效装备检查
//         TINT32 dwRetCode = CBackpack::CheckSoulEquip(pstUser, &pstUser->m_tbPlayer);
//         if (dwRetCode != 0)
//         {
//             pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
//             TSE_LOG_ERROR(pstSession->m_poServLog, ("AuCommonHandleBefore :check CheckSoulEquip fail [ret=%d] [seq=%u]", dwRetCode, pstUser->m_udwBSeqNo));
//             return -1;
//         }
        CBufferBase::ComputeBuffInfo(&pstUser->m_stCityInfo, pstUser, pstSession->m_atbIdol, pstSession->m_udwIdolNum, &pstSession->m_tbThrone, &pstSession->m_stTitleInfoList);
    }

    //4. 计算city信息并更新资源
    CProductionSystem::ComputeProductionSystem(pstUser, &pstUser->m_stCityInfo, pstSession->m_stReqAction.m_nEtime);

    //5. 检查任务是否完成
    CQuestLogic::CheckTimeQuestFinish(pstUser, &pstUser->m_stCityInfo, &pstUser->m_tbQuest);

    //6. 恢复龙体力
    if (0 == CPlayerBase::CheckUpdtDragonEnergy(pstUser))
    {
        CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPE__DRAGON_ENERGY_OK, pstUser->m_tbPlayer.m_nUid, TRUE);
    }

    //7. 获取双方might信息
    GetLastTotalMight(pstUser, pstSession->m_poServLog);
    return 0;
}

TVOID CAuCommonBefore::GetLastTotalMight(SUserInfo * pstUser, CTseLogger * poServLog)
{
    Json::FastWriter rWriter;
    rWriter.omitEndingLineFeed();

    //source user
    pstUser->m_uddwLastMight = pstUser->m_tbUserStat.m_nLast_might;
    pstUser->m_sLastTroopAndFort = rWriter.write(pstUser->m_tbUserStat.m_jLast_troop_fort);

    Json::Value rTroopAndFort_S = Json::Value(Json::objectValue);
    pstUser->m_uddwCurMight = CCommonBase::GetTotalTroopAndFort(pstUser, rTroopAndFort_S);
    pstUser->m_sCurTroopAndFort = rWriter.write(rTroopAndFort_S);

    if (pstUser->m_uddwLastMight != pstUser->m_uddwCurMight)
    {
        TSE_LOG_ERROR(poServLog, ("GetLastTotalMight: Current might and record might not match. [change=%ld] [uid=%ld][seq=%u]",
            (TINT64)pstUser->m_uddwCurMight - (TINT64)pstUser->m_uddwLastMight, pstUser->m_tbPlayer.m_nUid, pstUser->m_udwBSeqNo))
    }

    //log
    TSE_LOG_DEBUG(poServLog, ("GetLastTotalMight: [cur_might=%ld][last_might=%ld][cur_json=%s][last_json=%s] [uid=%ld][seq=%u]",
        pstUser->m_uddwCurMight, pstUser->m_uddwLastMight,
        pstUser->m_sCurTroopAndFort.c_str(), pstUser->m_sLastTroopAndFort.c_str(),
        pstUser->m_tbPlayer.m_nUid, pstUser->m_udwBSeqNo));
}
