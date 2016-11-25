#include "pushdata_action.h"
#include "common_func.h"
#include "base/log/wtselogger.h"
#include "sendmessage_base.h"
#include "procedure_base.h"
#include "json_result_generator.h"
#include "action_base.h"
#include "tool_base.h"
#include "global_serv.h"
#include "map_base.h"
#include "common_json.h"
#include "map_logic.h"
#include "player_base.h"

using namespace wtse::log;


TINT32 CPushDataProcess::SendPushDataRequest_AlHelpReq( SSession *pstSession, TbAlliance_action* ptbAlAction )
{
    TUINT32 udwExceptUid = ptbAlAction->m_nSuid;

    Json::Value rjsonResult(Json::objectValue);
    rjsonResult["svr_al_action_list"] = Json::Value(Json::objectValue);

    // action
    CCommJson::GenAlActionInfoForPush(ptbAlAction, rjsonResult["svr_al_action_list"]);

    // send push data
    return CPushDataBasic::PushDataBasic_TargetAid(pstSession, EN_SERVICE_TYPE__CLIENT__DATA_PUSH_RSP, 
        rjsonResult, pstSession->m_stReqParam.m_udwAllianceId, &udwExceptUid, 1);
}

TINT32 CPushDataProcess::SendPushDataRequest_AlTaskHelpSpeedUp( SSession *pstSession, TbAlliance_action* ptbAlAction, TINT32 dwHelpNum, TINT32 dwHelpTime )
{
    Json::Value rjsonResult(Json::objectValue);
    rjsonResult["svr_action_list"] = Json::Value(Json::objectValue);

    // action
    ptbAlAction->m_nHelped_num += dwHelpNum;
    ptbAlAction->m_nEtime += dwHelpTime;
    CCommJson::GenAlActionInfoForPush(ptbAlAction, rjsonResult["svr_action_list"]);

    // tips
    TbTips tbTmpTips;
    tbTmpTips.Reset();
    SUserInfo& stUserInfo = pstSession->m_stUserInfo;
    TbPlayer& tbPlayer = pstSession->m_stUserInfo.m_tbPlayer;
    if(ptbAlAction->m_nMclass == EN_ACTION_MAIN_CLASS__BUILDING)
    {
        if(ptbAlAction->m_nSclass == EN_ACTION_SEC_CLASS__RESEARCH_UPGRADE)
        {
            CSendMessageBase::GetTips(&tbTmpTips, EN_TIPS_TYPE__AL_HELP, ptbAlAction->m_nSuid, 
                EN_HELP_TYPE__RESEARCH, ptbAlAction->m_bParam[0].m_stBuilding.m_ddwType, ptbAlAction->m_bParam[0].m_stBuilding.m_ddwTargetLevel,
                tbPlayer.m_sUin.c_str(), CCommonFunc::NumToString(dwHelpTime * -1).c_str());
        }
        else if(ptbAlAction->m_nSclass == EN_ACTION_SEC_CLASS__BUILDING_UPGRADE)
        {
            CSendMessageBase::GetTips(&tbTmpTips, EN_TIPS_TYPE__AL_HELP, ptbAlAction->m_nSuid, 
                EN_HELP_TYPE__BUILDING, ptbAlAction->m_bParam[0].m_stBuilding.m_ddwType, ptbAlAction->m_bParam[0].m_stBuilding.m_ddwTargetLevel,
                tbPlayer.m_sUin.c_str(), CCommonFunc::NumToString(dwHelpTime * -1).c_str());
        }
        else if(ptbAlAction->m_nSclass == EN_ACTION_SEC_CLASS__REMOVE_OBSTACLE || ptbAlAction->m_nSclass == EN_ACTION_SEC_CLASS__BUILDING_REMOVE)
        {
            CSendMessageBase::GetTips(&tbTmpTips, EN_TIPS_TYPE__AL_HELP, ptbAlAction->m_nSuid, 
                EN_HELP_TYPE__REMOVE_OBSTCLE, ptbAlAction->m_bParam[0].m_stBuilding.m_ddwType, ptbAlAction->m_bParam[0].m_stBuilding.m_ddwTargetLevel,
                tbPlayer.m_sUin.c_str(), CCommonFunc::NumToString(dwHelpTime * -1).c_str());
        }
    }
    else if(ptbAlAction->m_nMclass == EN_ACTION_MAIN_CLASS__EQUIP && ptbAlAction->m_nSclass == EN_ACTION_SEC_CLASS__EQUIP_UPGRADE)
    {
        CSendMessageBase::GetTips(&tbTmpTips, EN_TIPS_TYPE__AL_HELP, ptbAlAction->m_nSuid, 
            EN_HELP_TYPE__SMELT, ptbAlAction->m_bParam[0].m_stEquip.m_ddwEType, 0, tbPlayer.m_sUin.c_str(), CCommonFunc::NumToString(dwHelpTime * -1).c_str());
    }
    else if (ptbAlAction->m_nMclass == EN_ACTION_MAIN_CLASS__TRAIN_NEW)
    {
        TINT32 dwType = -1;
        if (ptbAlAction->m_nSclass == EN_ACTION_SEC_CLASS__TROOP)
        {
            dwType = EN_HELP_TYPE__TROOP_TRAIN;
        }
        else if (ptbAlAction->m_nSclass == EN_ACTION_SEC_CLASS__FORT)
        {
            dwType = EN_HELP_TYPE__FORT_TRAIN;
        }
        else if (ptbAlAction->m_nSclass == EN_ACTION_SEC_CLASS__HOS_TREAT)
        {
            dwType = EN_HELP_TYPE__TROOP_HEAL;
        }
        else if (ptbAlAction->m_nSclass == EN_ACTION_SEC_CLASS__FORT_REPAIR)
        {
            dwType = EN_HELP_TYPE__FORT_REPAIR;
        }
        if (dwType != -1)
        {
            CSendMessageBase::GetTips(&tbTmpTips, EN_TIPS_TYPE__AL_HELP, ptbAlAction->m_nSuid, 
                dwType, ptbAlAction->m_bParam[0].m_stTrain.m_ddwType, ptbAlAction->m_bParam[0].m_stTrain.m_ddwNum, 
                tbPlayer.m_sUin.c_str(), CCommonFunc::NumToString(dwHelpTime * -1).c_str());
        }
    }
    else if (ptbAlAction->m_nMclass == EN_ACTION_MAIN_CLASS__DRAGON)
    {
        if (ptbAlAction->m_nSclass == EN_ACTION_SEC_CLASS__UNLOCK_DRAGON)
        {
            CSendMessageBase::GetTips(&tbTmpTips, EN_TIPS_TYPE__AL_HELP, ptbAlAction->m_nSuid, 
                EN_HELP_TYPE__UNLOCK_DRAGON, 0, 0,
                tbPlayer.m_sUin.c_str(), CCommonFunc::NumToString(dwHelpTime * -1).c_str());
        }
    }
    if(tbTmpTips.IfNeedUpdate())
    {
        TUINT32 udwJsonIndex = 0;
        Json::Value& jsonTips = rjsonResult["svr_tips"];
        jsonTips = Json::Value(Json::arrayValue);
        jsonTips[udwJsonIndex] = Json::Value(Json::arrayValue);
        jsonTips[udwJsonIndex].append(tbTmpTips.m_nTime);
        jsonTips[udwJsonIndex].append(tbTmpTips.m_nType);
        jsonTips[udwJsonIndex].append(tbTmpTips.m_sContent);
    }

    // send push data
    return CPushDataBasic::PushDataBasic_TargetUid(pstSession, EN_SERVICE_TYPE__CLIENT__DATA_PUSH_RSP, rjsonResult, ptbAlAction->m_nSuid);
}

TINT32 CPushDataProcess::GetPushTasks_AlTaskHelpSpeedUpAll( SSession *pstSession, LTasksGroup &stTasks)
{
    SUserInfo& stUserInfo = pstSession->m_stUserInfo;
    TbPlayer& tbPlayer = pstSession->m_stUserInfo.m_tbPlayer;
    TbAlliance& tbAlliance = pstSession->m_stUserInfo.m_tbAlliance;
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    SCityInfo *pstCity = &stUserInfo.m_stCityInfo;
    TINT32 dwTaskIdx = 0;
    Json::Value rjsonResult(Json::objectValue);
    

    TbAlliance_action *ptbAlAction = NULL;
    for(TUINT32 udwIdx = 0; udwIdx < stUserInfo.m_udwAlCanHelpActionNum; ++udwIdx)
    {
        ptbAlAction = stUserInfo.m_patbAlCanHelpAction[udwIdx];
        if(CActionBase::IsPlayerOwnedAction(tbPlayer.m_nUid, ptbAlAction->m_nId)) //自己的任务跳过
        {
            continue;
        }

        rjsonResult.clear();
        TINT32 dwHelpTime = CToolBase::Get_AlHelpTime(ptbAlAction->m_nCtime, ptbAlAction->m_nCan_help_num);
        ptbAlAction->m_nEtime += dwHelpTime;
        ptbAlAction->m_nHelped_num++;
        CCommJson::GenAlActionInfoForPush(ptbAlAction, rjsonResult["svr_action_list"]);

        // tips
        TbTips tbTmpTips;
        tbTmpTips.Reset();
        if(ptbAlAction->m_nMclass == EN_ACTION_MAIN_CLASS__BUILDING)
        {
            if(ptbAlAction->m_nSclass == EN_ACTION_SEC_CLASS__RESEARCH_UPGRADE)
            {
                CSendMessageBase::GetTips(&tbTmpTips, EN_TIPS_TYPE__AL_HELP, ptbAlAction->m_nSuid, 
                    EN_HELP_TYPE__RESEARCH, ptbAlAction->m_bParam[0].m_stBuilding.m_ddwType, ptbAlAction->m_bParam[0].m_stBuilding.m_ddwTargetLevel,
                    tbPlayer.m_sUin.c_str(), CCommonFunc::NumToString(dwHelpTime * -1).c_str());
            }
            else if(ptbAlAction->m_nSclass == EN_ACTION_SEC_CLASS__BUILDING_UPGRADE)
            {
                CSendMessageBase::GetTips(&tbTmpTips, EN_TIPS_TYPE__AL_HELP, ptbAlAction->m_nSuid, 
                    EN_HELP_TYPE__BUILDING, ptbAlAction->m_bParam[0].m_stBuilding.m_ddwType, ptbAlAction->m_bParam[0].m_stBuilding.m_ddwTargetLevel,
                    tbPlayer.m_sUin.c_str(), CCommonFunc::NumToString(dwHelpTime * -1).c_str());
            }
            else if(ptbAlAction->m_nSclass == EN_ACTION_SEC_CLASS__REMOVE_OBSTACLE || ptbAlAction->m_nSclass == EN_ACTION_SEC_CLASS__BUILDING_REMOVE)
            {
                CSendMessageBase::GetTips(&tbTmpTips, EN_TIPS_TYPE__AL_HELP, ptbAlAction->m_nSuid, 
                    EN_HELP_TYPE__REMOVE_OBSTCLE, ptbAlAction->m_bParam[0].m_stBuilding.m_ddwType, ptbAlAction->m_bParam[0].m_stBuilding.m_ddwTargetLevel,
                    tbPlayer.m_sUin.c_str(), CCommonFunc::NumToString(dwHelpTime * -1).c_str());
            }
        }
        else if(ptbAlAction->m_nMclass == EN_ACTION_MAIN_CLASS__EQUIP && ptbAlAction->m_nSclass == EN_ACTION_SEC_CLASS__EQUIP_UPGRADE)
        {
            CSendMessageBase::GetTips(&tbTmpTips, EN_TIPS_TYPE__AL_HELP, ptbAlAction->m_nSuid, 
                EN_HELP_TYPE__SMELT, ptbAlAction->m_bParam[0].m_stEquip.m_ddwEType, 0, tbPlayer.m_sUin.c_str(), CCommonFunc::NumToString(dwHelpTime * -1).c_str());
        }
        else if (ptbAlAction->m_nMclass == EN_ACTION_MAIN_CLASS__TRAIN_NEW)
        {
            TINT32 dwType = -1;
            if (ptbAlAction->m_nSclass == EN_ACTION_SEC_CLASS__TROOP)
            {
                dwType = EN_HELP_TYPE__TROOP_TRAIN;
            }
            else if (ptbAlAction->m_nSclass == EN_ACTION_SEC_CLASS__FORT)
            {
                dwType = EN_HELP_TYPE__FORT_TRAIN;
            }
            else if (ptbAlAction->m_nSclass == EN_ACTION_SEC_CLASS__HOS_TREAT)
            {
                dwType = EN_HELP_TYPE__TROOP_HEAL;
            }
            else if (ptbAlAction->m_nSclass == EN_ACTION_SEC_CLASS__FORT_REPAIR)
            {
                dwType = EN_HELP_TYPE__FORT_REPAIR;
            }
            if (dwType != -1)
            {
                CSendMessageBase::GetTips(&tbTmpTips, EN_TIPS_TYPE__AL_HELP, ptbAlAction->m_nSuid, 
                    dwType, ptbAlAction->m_bParam[0].m_stTrain.m_ddwType, ptbAlAction->m_bParam[0].m_stTrain.m_ddwNum, 
                    tbPlayer.m_sUin.c_str(), CCommonFunc::NumToString(dwHelpTime * -1).c_str());
            }
        }
        else if (ptbAlAction->m_nMclass == EN_ACTION_MAIN_CLASS__DRAGON)
        {
            if (ptbAlAction->m_nSclass == EN_ACTION_SEC_CLASS__UNLOCK_DRAGON)
            {
                CSendMessageBase::GetTips(&tbTmpTips, EN_TIPS_TYPE__AL_HELP, ptbAlAction->m_nSuid, 
                    EN_HELP_TYPE__UNLOCK_DRAGON, 0, 0,
                    tbPlayer.m_sUin.c_str(), CCommonFunc::NumToString(dwHelpTime * -1).c_str());
            }
        }
        if(tbTmpTips.IfNeedUpdate())
        {
            TUINT32 udwJsonIndex = 0;
            Json::Value& jsonTips = rjsonResult["svr_tips"];
            jsonTips = Json::Value(Json::arrayValue);
            jsonTips[udwJsonIndex] = Json::Value(Json::arrayValue);
            jsonTips[udwJsonIndex].append(tbTmpTips.m_nTime);
            jsonTips[udwJsonIndex].append(tbTmpTips.m_nType);
            jsonTips[udwJsonIndex].append(tbTmpTips.m_sContent);
        }

        // set task
        SetOneTask_AlTaskHelpSpeedUp(pstSession, stTasks.m_Tasks[dwTaskIdx], dwTaskIdx, EN_SERVICE_TYPE__CLIENT__DATA_PUSH_RSP, rjsonResult, ptbAlAction->m_nSuid);
        dwTaskIdx++;
    }

    if(dwTaskIdx > 0)
    {
        stTasks.SetValidTasks(dwTaskIdx);
    }
    return dwTaskIdx;
}

TINT32 CPushDataProcess::SetOneTask_AlTaskHelpSpeedUp( SSession *pstSession, LTask& stTask, TINT32 dwTaskIdx, TUINT16 uwReqServiceType, Json::Value& rJson, TUINT32 udwTargetUid )
{
    CDownMgr *pobjDownMgr = CDownMgr::Instance();
    TUCHAR *pucPackage = NULL;
    TUINT32 udwPackageLen = 0;
    CBaseProtocolPack *pobjPack = pstSession->m_ppPackTool[dwTaskIdx];
    TBOOL bRet = FALSE;
    TINT32 dwCostTime = 1;
    TINT32 dwTargetType = EN_PUSH_DATA_TYPE__UID;
    TINT32 dwListSize = 1;
    CJsonResultGenerator *pJsonGenerator = pstSession->m_pJsonGenerator;
    TUINT8 ucCompressFlag = 0;

    // 1. get down node
    if(pstSession->m_bUserLinkerNodeExist == FALSE)
    {
        pstSession->m_pstUserLinkerNode = NULL;
        if(S_OK == pobjDownMgr->zk_GetNode(DOWN_NODE_TYPE__USER_LINKER, &pstSession->m_pstUserLinkerNode))
        {
            pstSession->m_bUserLinkerNodeExist = TRUE;
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("Get UserLinker node succ [seq=%u]", pstSession->m_udwSeqNo));
        }
        else
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("Get UserLinker node fail [seq=%u]", pstSession->m_udwSeqNo));
            return -1;
        }

        if(NULL == pstSession->m_pstUserLinkerNode->m_stDownHandle.handle)
        {
            pstSession->m_bUserLinkerNodeExist = FALSE;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("SendUserLinkerRequest_TargetUid:handle [handlevalue=0x%p] [seq=%u]", \
                pstSession->m_pstAwsProxyNode->m_stDownHandle.handle, pstSession->m_udwSeqNo));
            return -2;
        }
    }

    // 2. get data
    pJsonGenerator->GenPushData_Pb(pstSession, uwReqServiceType, rJson);

    // 3. get package
    pobjPack->ResetContent();
    pobjPack->SetServiceType(EN_SERVICE_TYPE__CLIENT__DATA_PUSH_RSP);
    pobjPack->SetSeq(CGlobalServ::GenerateHsReqSeq());
    pobjPack->SetKey(EN_GLOBAL_KEY__REQ_SEQ, pstSession->m_udwSeqNo);
    pobjPack->SetKey(EN_GLOBAL_KEY__TARGET_TYPE, dwTargetType);
    pobjPack->SetKey(EN_GLOBAL_KEY__TARGET_LIST_SIZE, dwListSize);
    pobjPack->SetKey(EN_GLOBAL_KEY__TARGET_LIST, (TUCHAR*)&udwTargetUid, sizeof(udwTargetUid));
    dwListSize = 0;
    pobjPack->SetKey(EN_GLOBAL_KEY__EXCEPT_LIST_SIZE, dwListSize);
    pobjPack->SetKey(EN_GLOBAL_KEY__RES_BUF_COMPRESS_FLAG, ucCompressFlag);
    pobjPack->SetKey(EN_GLOBAL_KEY__REQ_BUF, (TUCHAR*)&pstSession->m_szTmpClientRspBuf[0], pstSession->m_dwTmpFinalPackLength);

    pobjPack->GetPackage(&pucPackage, &udwPackageLen);

    TSE_LOG_DEBUG(pstSession->m_poServLog, ("SendUserLinkerRequest_TargetUid: send package_len=%u [seq=%u]", udwPackageLen, pstSession->m_udwSeqNo));

    // 4. set task
    stTask.SetConnSession(pstSession->m_pstUserLinkerNode->m_stDownHandle);
    stTask.SetSendData(pucPackage, udwPackageLen);
    stTask.SetNeedResponse(0);

    return 0;
}

TINT32 CPushDataProcess::SendPushDataRequest_MarchAction( SSession *pstSession, TUINT32 udwActiveUid, TUINT32 udwActiveAid, TbMarch_action* ptbAction, TINT32 dwUpdtType, SPushActionNode *pstRawActionInfo )
{
    if(ptbAction == NULL)
    {
        return 0;
    }

    //过滤不需要push的action
    if(ptbAction->m_nSclass == EN_ACTION_SEC_CLASS__THRONE_TIMER
        || ptbAction->m_nSclass == EN_ACTION_SEC_CLASS__PRISON_TIMER
        || ptbAction->m_nSclass == EN_ACTION_SEC_CLASS__NOTI_TIMER
        || ptbAction->m_nSclass == EN_ACTION_SEC_CLASS__IDOL_PERIOD
        || ptbAction->m_nSclass == EN_ACTION_SEC_CLASS__THRONE_PERIOD
        || ptbAction->m_nSclass == EN_ACTION_SEC_CLASS__TAX)
    {
        return 0;
    }

    TbThrone *ptbThrone = &pstSession->m_tbThrone;

    TSE_LOG_DEBUG(pstSession->m_poServLog, ("SendPushDataRequest_MarchAction: id=%ld, type=%d, sal=%ld, tal=%ld [seq=%u]", 
        ptbAction->m_nId, dwUpdtType, ptbAction->m_nSal, ptbAction->m_nTal, pstSession->m_udwSeqNo));
    if(pstRawActionInfo)
    {
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("SendPushDataRequest_MarchAction--RawInfo: id=%ld, suid=%u, sal=%ld, tuid=%u, tal=%ld [seq=%u]", 
            ptbAction->m_nId, pstRawActionInfo->m_udwSUid, pstRawActionInfo->m_nSal, pstRawActionInfo->m_udwTUid, pstRawActionInfo->m_nTal, pstSession->m_udwSeqNo));
    }

    TUINT32 audwExceptUidList[10];
    TUINT32 udwExceptListNum = 0;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;

    if(udwActiveUid)
    {
        audwExceptUidList[udwExceptListNum++] = udwActiveUid;
    }

    // 处理suid的情况
    if (ptbAction->m_nSuid != udwActiveUid && ptbAction->m_nSuid != 0)
    {
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("SendPushDataRequest_MarchAction: send to suid=%ld [seq=%u]", ptbAction->m_nSuid, pstSession->m_udwSeqNo));
        CPushDataBasic::PushDataUid_MarchActionSourceUid(pstSession, ptbAction->m_nSuid, ptbAction, dwUpdtType);
        audwExceptUidList[udwExceptListNum++] = ptbAction->m_nSuid;
    }

    //处理sal情况
    if(ptbAction->m_nSal > 0)
    {
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("SendPushDataRequest_MarchAction: send to sal [seq=%u]", pstSession->m_udwSeqNo));
        CPushDataBasic::PushDataAid_March(pstSession, ptbAction, dwUpdtType, "svr_al_action_list", ptbAction->m_nSal, &audwExceptUidList[0], udwExceptListNum);
    }

    // 处理tuid的情况
    if (udwActiveUid != ptbAction->m_nTuid && ptbAction->m_nTal != 0 && ptbAction->m_nTuid != 0)
    {
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("SendPushDataRequest_MarchAction: send to tuid [seq=%u]", pstSession->m_udwSeqNo));
        //CPushDataBasic::PushDataUid_MarchActionTargetUid(pstSession, ptbAction->m_nTuid, ptbAction, dwUpdtType);
        //audwExceptUidList[udwExceptListNum++] = ptbAction->m_nSuid;
        CPushDataBasic::PushDataUid_Refresh(pstSession, ptbAction->m_nTuid);
    }

    //处理tal情况
    if (ptbAction->m_nTal > CActionBase::GenThroneTargetId(0, 0))
    {
        if (ptbThrone->m_nAlid != 0 && ptbAction->m_nTal == CActionBase::GenThroneTargetId(ptbThrone->m_nSid, ptbThrone->m_nPos))
        {
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("SendPushDataRequest_MarchAction: send to throne [seq=%u]", pstSession->m_udwSeqNo));
            CPushDataBasic::PushDataAid_March(pstSession, ptbAction, dwUpdtType, "svr_al_p_action_list", ptbThrone->m_nAlid, &audwExceptUidList[0], udwExceptListNum);
        }
    }
    else if(ptbAction->m_nTal > 0)
    {
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("SendPushDataRequest_MarchAction: send to tal [seq=%u]", pstSession->m_udwSeqNo));
        CPushDataBasic::PushDataAid_March(pstSession, ptbAction, dwUpdtType, "svr_al_p_action_list", ptbAction->m_nTal, &audwExceptUidList[0], udwExceptListNum);
    }

    // 处理sal、tal、tuid发生变化的情况, 防止漏处理
    if(pstRawActionInfo)
    {
        if(pstRawActionInfo->m_nSal > 0 && pstRawActionInfo->m_nSal != ptbAction->m_nSal)
        {
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("SendPushDataRequest_MarchAction: raw_info_change send del to sal [seq=%u]", pstSession->m_udwSeqNo));
            CPushDataBasic::PushDataAid_March(pstSession, ptbAction, EN_TABLE_UPDT_FLAG__DEL, "svr_al_action_list", pstRawActionInfo->m_nSal, &audwExceptUidList[0], udwExceptListNum);
        }

        if(pstRawActionInfo->m_nTal > 0 && pstRawActionInfo->m_nTal != ptbAction->m_nTal)
        {
            if (pstRawActionInfo->m_nTal > CActionBase::GenThroneTargetId(0, 0))
            {
                if (ptbThrone->m_nAlid != 0 && pstRawActionInfo->m_nTal == CActionBase::GenThroneTargetId(ptbThrone->m_nSid, ptbThrone->m_nPos))
                {
                    TSE_LOG_DEBUG(pstSession->m_poServLog, ("SendPushDataRequest_MarchAction: raw_info_change send del to throne [seq=%u]", pstSession->m_udwSeqNo));
                    CPushDataBasic::PushDataAid_March(pstSession, ptbAction, EN_TABLE_UPDT_FLAG__DEL, "svr_al_p_action_list", ptbThrone->m_nAlid, &audwExceptUidList[0], udwExceptListNum);
                }
            }
            else
            {
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("SendPushDataRequest_MarchAction: raw_info_change send del to tal [seq=%u]", pstSession->m_udwSeqNo));
                CPushDataBasic::PushDataAid_March(pstSession, ptbAction, EN_TABLE_UPDT_FLAG__DEL, "svr_al_p_action_list", pstRawActionInfo->m_nTal, &audwExceptUidList[0], udwExceptListNum);
            }
        }

        if(pstRawActionInfo->m_nTal < 0 && pstRawActionInfo->m_nTal != ptbAction->m_nTal && pstRawActionInfo->m_udwTUid != udwActiveUid)
        {
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("SendPushDataRequest_MarchAction: raw_info_change send del to tuid [seq=%u]", pstSession->m_udwSeqNo));
            CPushDataBasic::PushDataUid_MarchActionTargetUid(pstSession, pstRawActionInfo->m_udwTUid, ptbAction, EN_TABLE_UPDT_FLAG__DEL);
        }
    }

    // 处理rally_war界面刷新状况
    if(ptbAction->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR)
    {
        if (dwUpdtType != EN_TABLE_UPDT_FLAG__NEW)
        {
            if (pstRawActionInfo && pstRawActionInfo->m_nSal > 0)
            {
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("PushDataAid_KeyRefresh---rallywar_sal: action_id=%ld, suid=%ld sal=%ld tal=%ld tid=%ld [seq=%u]", 
                    ptbAction->m_nId, ptbAction->m_nSuid, 
                    ptbAction->m_nSal, ptbAction->m_nTal, 
                    ptbAction->m_nTid, pstSession->m_udwSeqNo));
                CPushDataBasic::PushDataAid_KeyRefresh(pstSession, pstRawActionInfo->m_nSal, NULL, 0, "svr_rally_war_info");
            }
            if (pstRawActionInfo && pstRawActionInfo->m_nTal > 0)
            {
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("PushDataAid_KeyRefresh---rallywar_tal: action_id=%ld, suid=%ld sal=%ld tal=%ld tid=%ld [seq=%u]", 
                    ptbAction->m_nId, ptbAction->m_nSuid, 
                    ptbAction->m_nSal, ptbAction->m_nTal, 
                    ptbAction->m_nTid, pstSession->m_udwSeqNo));
                CPushDataBasic::PushDataAid_KeyRefresh(pstSession, pstRawActionInfo->m_nTal, NULL, 0, "svr_rally_war_info");
            }
        }
    }
    else if (ptbAction->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_REINFORCE
        || ptbAction->m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_NORMAL)
    {
		if (ptbAction->CheckFlag(TbMARCH_ACTION_FIELD_STATUS) || ptbAction->m_nStatus != EN_MARCH_STATUS__RETURNING)
		{
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("PushDataAid_KeyRefresh---rally_reinforce[aid=%u]: action_id=%ld, suid=%ld sal=%ld tal=%ld tid=%ld [seq=%u]",
                pstSession->m_stReqParam.m_udwAllianceId, ptbAction->m_nId, ptbAction->m_nSuid,
                ptbAction->m_nSal, ptbAction->m_nTal,
                ptbAction->m_nTid, pstSession->m_udwSeqNo));
            CPushDataBasic::PushDataAid_KeyRefresh(pstSession, pstSession->m_stReqParam.m_udwAllianceId, NULL, 0, "svr_rally_war_info");
		}
    }
    else if (ptbAction->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR_THRONE)
    {
        if (dwUpdtType != EN_TABLE_UPDT_FLAG__NEW)
        {
            if (pstRawActionInfo && pstRawActionInfo->m_nSal > 0)
            {
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("PushDataAid_KeyRefresh---rally_throne_sal: action_id=%ld, suid=%ld sal=%ld tal=%ld tid=%ld [seq=%u]",
                    ptbAction->m_nId, ptbAction->m_nSuid,
                    ptbAction->m_nSal, ptbAction->m_nTal,
                    ptbAction->m_nTid, pstSession->m_udwSeqNo));
                CPushDataBasic::PushDataAid_KeyRefresh(pstSession, pstRawActionInfo->m_nSal, NULL, 0, "svr_rally_war_info");
            }
            if (pstRawActionInfo && pstRawActionInfo->m_nTal == CActionBase::GenThroneTargetId(ptbThrone->m_nSid, ptbThrone->m_nPos))
            {
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("PushDataAid_KeyRefresh---rally_throne_tal: action_id=%ld, suid=%ld sal=%ld tal=%ld tid=%ld [seq=%u]",
                    ptbAction->m_nId, ptbAction->m_nSuid,
                    ptbAction->m_nSal, ptbAction->m_nTal,
                    ptbAction->m_nTid, pstSession->m_udwSeqNo));
                CPushDataBasic::PushDataAid_KeyRefresh(pstSession, ptbThrone->m_nAlid, NULL, 0, "svr_rally_war_info");
            }
        }
    }
    else if (ptbAction->m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_THRONE
        || ptbAction->m_nSclass == EN_ACTION_SEC_CLASS__ASSIGN_THRONE)
    {
//         if (ptbAction->m_nTid > 0)
//         { //进攻方
//             if (ptbAction->CheckFlag(TbMARCH_ACTION_FIELD_STATUS) || ptbAction->m_nStatus != EN_MARCH_STATUS__RETURNING)
//             {
//                 TSE_LOG_DEBUG(pstSession->m_poServLog, ("PushDataAid_KeyRefresh---rally_throne_reinforce[aid=%u]: action_id=%ld, suid=%ld sal=%ld tal=%ld tid=%ld [seq=%u]",
//                     pstSession->m_stReqParam.m_udwAllianceId, ptbAction->m_nId, ptbAction->m_nSuid,
//                     ptbAction->m_nSal, ptbAction->m_nTal,
//                     ptbAction->m_nTid, pstSession->m_udwSeqNo));
//                 CPushDataBasic::PushDataAid_KeyRefresh(pstSession, pstSession->m_stReqParam.m_udwAllianceId, NULL, 0, "svr_rally_war_info");
//             }
//         }
//         else
        { //防守方
            if (ptbAction->CheckFlag(TbMARCH_ACTION_FIELD_STATUS) || ptbAction->m_nStatus != EN_MARCH_STATUS__RETURNING)
            {
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("PushDataAid_KeyRefresh---throne_reinforce[aid=%u]: action_id=%ld, suid=%ld sal=%ld tal=%ld tid=%ld [seq=%u]",
                    ptbThrone->m_nAlid, ptbAction->m_nId, ptbAction->m_nSuid,
                    ptbAction->m_nSal, ptbAction->m_nTal,
                    ptbAction->m_nTid, pstSession->m_udwSeqNo));
                if (ptbThrone->m_nAlid != 0)
                {
                    CPushDataBasic::PushDataAid_KeyRefresh(pstSession, ptbThrone->m_nAlid, NULL, 0, "svr_throne_detail_info");
                }
            }
        }
    }

    return 0;
}

TINT32 CPushDataProcess::SendPushDataRequest_AlAction( SSession *pstSession, TUINT32 udwActiveUid, TbAlliance_action* ptbAction, TINT32 dwUpdtType, SPushActionNode *pstRawActionInfo )
{
    if(ptbAction == NULL)
    {
        return 0;
    }

    TINT64 ddwRawSal = 0;
    if(pstRawActionInfo)
    {
        ddwRawSal = pstRawActionInfo->m_nSal;
    }

    TSE_LOG_DEBUG(pstSession->m_poServLog, ("SendPushDataRequest_AlAction: id=%ld, type=%d, sal=%ld, raw_sal=%ld [seq=%u]", \
        ptbAction->m_nId, dwUpdtType, ptbAction->m_nSal, ddwRawSal, pstSession->m_udwSeqNo));

    if(ptbAction->m_nSal > 0)
    {
        SendPushDataRequest_AlActionAid(pstSession, ptbAction, dwUpdtType, "svr_al_action_list", ptbAction->m_nSal, udwActiveUid);
    }

    if(ddwRawSal > 0 && ptbAction->m_nSal != ddwRawSal)
    {
        SendPushDataRequest_AlActionAid(pstSession, ptbAction, EN_TABLE_UPDT_FLAG__DEL, "svr_al_action_list", ddwRawSal, udwActiveUid);
    }

    return 0;
}

TINT32 CPushDataProcess::SendPushDataRequest_AlActionAid( SSession *pstSession, TbAlliance_action* ptbAction, TINT32 dwUpdtType, const TCHAR* pszTable, TUINT32 udwTargetAid, TUINT32 udwExceptUid /*= 0*/ )
{
    Json::Value rjsonResult(Json::objectValue);    
    if(ptbAction->m_nSal > 0)
    {
        rjsonResult["svr_al_action_list"] = Json::Value(Json::objectValue);
        CCommJson::GenAlActionInfoForPush(ptbAction, rjsonResult["svr_al_action_list"], dwUpdtType);
    }

    // send push data
    return CPushDataBasic::PushDataBasic_TargetAid(pstSession, EN_SERVICE_TYPE__CLIENT__DATA_PUSH_RSP, 
        rjsonResult, udwTargetAid, &udwExceptUid, 1);
}

TINT32 CPushDataProcess::SendPushDataRequest_ForHuActionComm( SSession *pstSession )
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TUINT32 udwUid = pstUser->m_tbPlayer.m_nUid;
    CPushMapData *pobjMapData = &pstSession->m_objAuPushDataNode.m_objPushDataMap;
    CPushAlData *pobjActionData = &pstSession->m_objAuPushDataNode.m_objPushDataSourceAl;

    TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("[wavetest---HuPushData]: push_data beg--------------- [seq=%u]", pstSession->m_udwSeqNo));

    if(pstSession->m_stReqParam.m_udwCommandID != EN_CLIENT_REQ_COMMAND__ALLIANCE_TASK_HELP_SPEED_UP
        && pstSession->m_stReqParam.m_udwCommandID != EN_CLIENT_REQ_COMMAND__ALLIANCE_TASK_HELP_SPEED_UP_ALL
        && pstSession->m_stReqParam.m_udwCommandID != EN_CLIENT_REQ_COMMAND__SVR_CHANGE)
    {
        // push action
        PushData_HuActionProcess(pstSession, pstUser, pobjActionData, pobjMapData);

        //push map(action & wild)
        CPushDataBasic::PushDataMap_Normal(pstSession, pobjMapData->m_vecActionList, pobjMapData->m_vecWild, pstUser->m_tbPlayer.m_nSid);

        // push tips
        if(pstUser->m_ptbPushTips)
        {
            CPushDataBasic::PushDataUid_Tips(pstSession, pstUser->m_ptbPushTips->m_nUid, pstUser->m_ptbPushTips);
        }
    }

    if (pstSession->m_udwCommand == EN_CLIENT_REQ_COMMAND__SVR_CHANGE)
    {
        //全局相关的数据需要重新拉取...
        CPushDataBasic::PushDataUid_Refresh(pstSession, udwUid);
    }

    if (pstSession->m_tbThrone.CheckFlag(TbTHRONE_FIELD_OWNER_ID))
    {
        Json::Value jTitle = Json::Value(Json::objectValue);
        CCommJson::GenTitleInfo(&pstSession->m_stTitleInfoList, &pstSession->m_tbThrone, jTitle["svr_title_new"]);
        CPushDataBasic::PushDataBasic_Sid(pstSession, EN_SERVICE_TYPE__CLIENT__DATA_PUSH_RSP, jTitle, pstUser->m_tbPlayer.m_nSid);
    }

    TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("[wavetest---HuPushData]: push_data end--------------- [seq=%u]", pstSession->m_udwSeqNo));
    return 0;
}

TINT32 CPushDataProcess::SendUserLinkerRequest_DataRefreshUid(SSession *pstSession, TUINT16 uwReqServiceType, TUINT32 udwTargetUid, const TCHAR *pKey)
{
    // action
    Json::Value rjsonResult(Json::objectValue);
    if (NULL != pKey)
    {
        rjsonResult[pKey] = Json::Value(Json::objectValue);
    }

    TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("SendUserLinkerRequest_DataRefreshUid: target=%u [seq=%u]",
        udwTargetUid, pstSession->m_udwSeqNo));

    // send push data
    return CPushDataBasic::PushDataBasic_TargetUid(pstSession, EN_SERVICE_TYPE__CLIENT__DATA_PUSH_RSP, rjsonResult, udwTargetUid);
}

TINT32 CPushDataProcess::SendUserLinkerRequest_DataRefreshAid(SSession *pstSession, TUINT16 uwReqServiceType, TUINT32 udwTargetAid, TUINT32 *pudwExceptUidList, TUINT32 udwExceptListNum, const TCHAR *pKey)
{
    // action
    Json::Value rjsonResult(Json::objectValue);
    if (NULL != pKey)
    {
        rjsonResult[pKey] = Json::Value(Json::objectValue);
    }

    TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("SendUserLinkerRequest_DataRefreshAid: target=%u [seq=%u]",
        udwTargetAid, pstSession->m_udwSeqNo));

    // send push data
    return CPushDataBasic::PushDataBasic_TargetAid(pstSession, EN_SERVICE_TYPE__CLIENT__DATA_PUSH_RSP, rjsonResult, udwTargetAid, pudwExceptUidList, udwExceptListNum);
}

TINT32 CPushDataProcess::InitRawInfo( SUserInfo *pstUser, CPushAlData *pobjPushData )
{
    pobjPushData->m_udwAllianceId = CPlayerBase::GetAllianceId(&pstUser->m_tbPlayer);
//     if(pobjPushData->m_udwAllianceId == 0)
//     {
//         return 0;
//     }

    TUINT8 ucFlag = EN_TABLE_UPDT_FLAG__UNCHANGE;
    map<TINT64, SPushActionNode>::iterator it;
    for(TINT32 idx = 0; idx < pstUser->m_udwSelfAlActionNum; idx++)
    {
        TbAlliance_action *ptbAlAction = &pstUser->m_atbSelfAlAction[idx];
        if(ptbAlAction->m_nSal > 0 && ptbAlAction->m_nSal == pobjPushData->m_udwAllianceId)
        {
            it = pobjPushData->m_mapAlActionRaw.find(ptbAlAction->m_nId);
            if(it == pobjPushData->m_mapAlActionRaw.end())
            {
                pobjPushData->m_mapAlActionRaw.insert(make_pair(ptbAlAction->m_nId, SPushActionNode(ptbAlAction, EN_AID_ACTION, ucFlag, TRUE)));
            }
        }
    }

    for(TINT32 idx = 0; idx < pstUser->m_udwMarchNum; idx++)
    {
        TbMarch_action *ptbMarch = &pstUser->m_atbMarch[idx];

        //过滤不需要push的action
        if(ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__THRONE_TIMER
            || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__PRISON_TIMER
            || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__NOTI_TIMER
            || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__IDOL_PERIOD
            || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__THRONE_PERIOD
            || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__TAX)
        {
            continue;
        }

        if((ptbMarch->m_nSal > 0 && ptbMarch->m_nSal == pobjPushData->m_udwAllianceId)
            || ptbMarch->m_nSuid == pstUser->m_tbPlayer.m_nUid)
        {
            it = pobjPushData->m_mapAlActionRaw.find(ptbMarch->m_nId);
            if(it == pobjPushData->m_mapAlActionRaw.end())
            {
                pobjPushData->m_mapAlActionRaw.insert(make_pair(ptbMarch->m_nId, SPushActionNode(ptbMarch, EN_MARCH_ACTION, ucFlag, TRUE)));
            }
        }
    }

    for(TINT32 idx = 0; idx < pstUser->m_udwPassiveMarchNum; idx++)
    {
        TbMarch_action *ptbMarch = &pstUser->m_atbPassiveMarch[idx];
        if(ptbMarch->m_nTal != 0)
        {
            it = pobjPushData->m_mapAlActionRaw.find(ptbMarch->m_nId);
            if(it == pobjPushData->m_mapAlActionRaw.end())
            {
                pobjPushData->m_mapAlActionRaw.insert(make_pair(ptbMarch->m_nId, SPushActionNode(ptbMarch, EN_MARCH_ACTION, ucFlag, FALSE)));
            }
        }
    }

    TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("[wavetest] InitRawInfo uid=%ld alid=%u, alaction_num=%u", 
        pstUser->m_tbPlayer.m_nUid, pobjPushData->m_udwAllianceId, pobjPushData->m_mapAlActionRaw.size()));

    return 0;
}

TINT32 CPushDataProcess::PushData_HuActionProcess( SSession *pstSession, SUserInfo *pstUser, CPushAlData *pobjPushDataAl, CPushMapData *pobjPushDataMap )
{
    TUINT8 ucFlag = EN_TABLE_UPDT_FLAG__UNCHANGE;
    TUINT32 udwActiveUid = pstSession->m_stReqParam.m_udwUserId;
    TUINT32 udwAlId = pobjPushDataAl->m_udwAllianceId;
    map<TINT64, SPushActionNode>::iterator it;
    SPushActionNode *pstRawActionInfo = NULL;
    TINT32 dwPushDataActionNum = 0;

    for(TINT32 idx = 0; idx < pstUser->m_udwSelfAlActionNum; idx++)
    {
        pstRawActionInfo = NULL;
        TbAlliance_action *ptbAlAction = &pstUser->m_atbSelfAlAction[idx];

        if(EN_TABLE_UPDT_FLAG__UNCHANGE != pstUser->m_aucSelfAlActionFlag[idx])
        {
            if(pobjPushDataAl->m_setActionId.count(ptbAlAction->m_nId) == 0)
            {
                it = pobjPushDataAl->m_mapAlActionRaw.find(ptbAlAction->m_nId);
                if(it != pobjPushDataAl->m_mapAlActionRaw.end())
                {
                    pstRawActionInfo = &it->second;
                }
                if (ptbAlAction->m_nHelped_num > 0)
                {
                    continue;
                }
                SendPushDataRequest_AlAction(pstSession, udwActiveUid, ptbAlAction, pstUser->m_aucSelfAlActionFlag[idx], pstRawActionInfo);
                dwPushDataActionNum++;

                pobjPushDataAl->m_setActionId.insert(ptbAlAction->m_nId);
                
            }
        }
    }

    for(TINT32 idx = 0; idx < pstUser->m_udwMarchNum; idx++)
    {
        pstRawActionInfo = NULL;
        TbMarch_action *ptbMarch = &pstUser->m_atbMarch[idx];

        //过滤不需要push的action
        if(ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__THRONE_TIMER
            || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__PRISON_TIMER
            || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__NOTI_TIMER
            || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__IDOL_PERIOD
            || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__THRONE_PERIOD
            || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__TAX)
        {
            continue;
        }

        if(EN_TABLE_UPDT_FLAG__UNCHANGE != pstUser->m_aucMarchFlag[idx])
        {
            if(pobjPushDataAl->m_setActionId.count(ptbMarch->m_nId) == 0)
            {
                it = pobjPushDataAl->m_mapAlActionRaw.find(ptbMarch->m_nId);
                if(it != pobjPushDataAl->m_mapAlActionRaw.end())
                {
                    pstRawActionInfo = &it->second;
                }
                SendPushDataRequest_MarchAction(pstSession, udwActiveUid, pobjPushDataAl->m_udwAllianceId, ptbMarch, pstUser->m_aucMarchFlag[idx], pstRawActionInfo);

                dwPushDataActionNum++;

                //add action to map
                AddPushData_MapAction(pobjPushDataMap, ptbMarch, pstUser->m_aucMarchFlag[idx]);

                pobjPushDataAl->m_setActionId.insert(ptbMarch->m_nId);
            }
        }
    }

    for(TINT32 idx = 0; idx < pstUser->m_udwPassiveMarchNum; idx++)
    {
        pstRawActionInfo = NULL;
        TbMarch_action *ptbMarch = &pstUser->m_atbPassiveMarch[idx];
        //过滤不需要push的action
        if(ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__THRONE_TIMER
            || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__PRISON_TIMER
            || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__NOTI_TIMER
            || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__IDOL_PERIOD
            || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__THRONE_PERIOD
            || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__TAX)
        {
            continue;
        }

        if(EN_TABLE_UPDT_FLAG__UNCHANGE != pstUser->m_aucPassiveMarchFlag[idx])
        {
            if(pobjPushDataAl->m_setActionId.count(ptbMarch->m_nId) == 0)
            {
                it = pobjPushDataAl->m_mapAlActionRaw.find(ptbMarch->m_nId);
                if(it != pobjPushDataAl->m_mapAlActionRaw.end())
                {
                    pstRawActionInfo = &it->second;
                }
                SendPushDataRequest_MarchAction(pstSession, udwActiveUid, pobjPushDataAl->m_udwAllianceId, ptbMarch, pstUser->m_aucPassiveMarchFlag[idx], pstRawActionInfo);
                
                dwPushDataActionNum++;

                //add action to map
                AddPushData_MapAction(pobjPushDataMap, ptbMarch, pstUser->m_aucPassiveMarchFlag[idx]);

                pobjPushDataAl->m_setActionId.insert(ptbMarch->m_nId);
            }
        }
    }

    if (dwPushDataActionNum > 0 && pstSession->m_stCommonResInfo.m_ucJsonType != EN_JSON_TYPE_USER_JSON && udwActiveUid > 0)
    {
        CPushDataBasic::PushDataUid_Refresh(pstSession, udwActiveUid);
    }

    TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("[wavetest] PushData_ComputeResult uid=%ld alid=%u, alaction_num=%u, mapacion_num=%u [seq=%u]",
        pstUser->m_tbPlayer.m_nUid, pobjPushDataAl->m_udwAllianceId, dwPushDataActionNum, pobjPushDataAl->m_udwUidListNum, pobjPushDataMap->m_vecActionList.size(), pstSession->m_udwSeqNo));

    return 0;
}

TINT32 CPushDataProcess::AddPushData_Wild( CPushMapData *pobjPushDataMap, TbMap *ptbWild )
{
    if(ptbWild->m_nId == 0)
    {
        return 0;
    }

    if(pobjPushDataMap->m_setWild.count(ptbWild->m_nId) == 0)
    {
        pobjPushDataMap->m_vecWild.push_back(SPushWildNode(ptbWild));
        pobjPushDataMap->m_setWild.insert(ptbWild->m_nId);
    }

    return 0;
}

TINT32 CPushDataProcess::AddPushData_MapAction( CPushMapData *pobjPushDataMap, TbMarch_action *ptbMarch, TINT32 dwTableFlag )
{
    TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("[wavetest] pmarch action: push_back_to_map id=%ld, flag=%d, count=%u", 
        ptbMarch->m_nId, dwTableFlag, pobjPushDataMap->m_setAction.count(ptbMarch->m_nId)));

    if(pobjPushDataMap->m_setAction.count(ptbMarch->m_nId) == 0)
    {
        pobjPushDataMap->m_vecActionList.push_back(SPushActionNode(ptbMarch, EN_MARCH_ACTION, dwTableFlag));
        pobjPushDataMap->m_setAction.insert(ptbMarch->m_nId);
    }
    return 0;
}

