#include "pushdata_basic.h"
#include "json_result_generator.h"
#include "global_serv.h"
#include "map_base.h"
#include "map_logic.h"
#include "common_func.h"
#include "common_json.h"

TINT32 CPushDataBasic::PushDataUid_Refresh( SSession *pstSession, TUINT32 udwTargetUid )
{
    Json::Value rjsonResult(Json::objectValue);
    return PushDataBasic_TargetUid(pstSession, EN_SERVICE_TYPE__CLIENT__INC_REFRESH_RSP, rjsonResult, udwTargetUid);
}

TINT32 CPushDataBasic::PushDataUid_Refresh( SSession *pstSession, TUINT32 *pudwUidList, TUINT32 udwUidListNum )
{
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("PushDataUid_Refresh: target=%u target_num=%u [seq=%u]", 
        pudwUidList[0], udwUidListNum, pstSession->m_udwSeqNo));

    Json::Value rjsonResult(Json::objectValue);
    return PushDataBasic_TargetUid(pstSession, EN_SERVICE_TYPE__CLIENT__INC_REFRESH_RSP, rjsonResult, pudwUidList, udwUidListNum);
}

TINT32 CPushDataBasic::PushDataAid_Refresh( SSession *pstSession, TUINT32 udwTargetAid, TUINT32 *pudwExceptUidList /*= NULL*/, TUINT32 udwExceptListNum /*= 0*/ )
{
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("PushDataAid_Refresh: target=%u [seq=%u]", 
        udwTargetAid, pstSession->m_udwSeqNo));

    Json::Value rjsonResult(Json::objectValue);
    return PushDataBasic_TargetAid(pstSession, EN_SERVICE_TYPE__CLIENT__INC_REFRESH_RSP, rjsonResult, udwTargetAid, pudwExceptUidList, udwExceptListNum);
}

TINT32 CPushDataBasic::PushDataAid_KeyRefresh( SSession *pstSession, TUINT32 udwTargetAid, TUINT32 *pudwExceptUidList /*= NULL*/, TUINT32 udwExceptListNum /*= 0*/, const TCHAR *pKey /*= NULL*/ )
{
    if(pKey == NULL)
    {
        return 0;
    }

    // action
    Json::Value rjsonResult(Json::objectValue);
    if (NULL != pKey)
    {
        rjsonResult[pKey] = Json::Value(Json::objectValue);
    }

    // send push data
    return PushDataBasic_TargetAid(pstSession, EN_SERVICE_TYPE__CLIENT__DATA_PUSH_RSP, rjsonResult, udwTargetAid, pudwExceptUidList, udwExceptListNum);
}

TINT32 CPushDataBasic::PushDataAid_Normal( SSession *pstSession, CPushAlData *pobjPushData, TUINT32 *pudwExceptUidList, TUINT32 udwExceptListNum )
{
    Json::Value jsonAlAction;

    GetPushData_AlAction(pobjPushData->m_vecActionList, pobjPushData->m_udwAllianceId, jsonAlAction);

    return PushDataBasic_TargetAid(pstSession, EN_SERVICE_TYPE__CLIENT__DATA_PUSH_RSP, jsonAlAction, pobjPushData->m_udwAllianceId, pudwExceptUidList, udwExceptListNum);
}

TINT32 CPushDataBasic::GetPushData_AlAction( vector<SPushActionNode> &vecActionList, TINT64 ddwAid, Json::Value &jsonAlAction )
{
    TINT32 dwActionNum = 0;
    jsonAlAction.clear();
    if(ddwAid == 0)
    {
        return 0;
    }

    SPushActionNode *pstNode = NULL;
    for(TINT32 idx = 0; idx < vecActionList.size(); idx++)
    {
        pstNode = &vecActionList[idx];
        if(pstNode->m_dwType == EN_AID_ACTION)
        {
            CCommJson::GenAlActionInfoForPush((TbAlliance_action*)pstNode->m_ptbAction, jsonAlAction["svr_al_action_list"], pstNode->m_ucFlag);
            dwActionNum++;
        }
        else if(pstNode->m_dwType == EN_MARCH_ACTION)
        {
            TbMarch_action *ptbAction = (TbMarch_action*)pstNode->m_ptbAction;

            if(pstNode->m_ucFlag == EN_TABLE_UPDT_FLAG__DEL)
            {
                CCommJson::GenMarchInfoForPush(ptbAction, jsonAlAction["svr_al_action_list"], pstNode->m_ucFlag);
                CCommJson::GenMarchInfoForPush(ptbAction, jsonAlAction["svr_al_p_action_list"], pstNode->m_ucFlag);
            }
            else
            {
                if(ptbAction->m_nSal == ddwAid)
                {
                    CCommJson::GenMarchInfoForPush(ptbAction, jsonAlAction["svr_al_action_list"], pstNode->m_ucFlag);
                    dwActionNum++;
                }

                if(ptbAction->m_nTal == ddwAid)
                {
                    CCommJson::GenMarchInfoForPush(ptbAction, jsonAlAction["svr_al_p_action_list"], pstNode->m_ucFlag);
                    dwActionNum++;
                }
            }
        }
        else
        {
            continue;
        }
    }
    return dwActionNum;
}

TINT32 CPushDataBasic::PushDataMap_SingleWild( SSession *pstSession, TbMap *ptbWild )
{
    TUINT32 udwSBlockId = 0;
    TINT32 dwSid = ptbWild->m_nSid;
    Json::Value jsonMap;
    Json::Value jsonTmp;
    if (CMapLogic::IsWildNeedToDelete(ptbWild))
    {
        udwSBlockId = CMapBase::GetBlockIdFromPos(ptbWild->m_nId);
        jsonMap[CCommonFunc::NumToString(udwSBlockId)]["svr_map_inc"]["list"][CCommonFunc::NumToString(ptbWild->m_nId)] = Json::Value(Json::nullValue);
        jsonMap[CCommonFunc::NumToString(udwSBlockId)]["svr_map_inc"]["svr_id"] = dwSid;
    }
    else
    {
        jsonTmp.clear();
        CCommJson::PushData_GenMapJson(ptbWild->m_nSid, ptbWild, jsonTmp);

        udwSBlockId = CMapBase::GetBlockIdFromPos(ptbWild->m_nId);
        jsonMap[CCommonFunc::NumToString(udwSBlockId)]["svr_map_inc"]["list"][CCommonFunc::NumToString(ptbWild->m_nId)] = jsonTmp;
        jsonMap[CCommonFunc::NumToString(udwSBlockId)]["svr_map_inc"]["svr_id"] = dwSid;
    }

    return PushDataBasic_Map(pstSession, EN_SERVICE_TYPE__CLIENT__DATA_PUSH_RSP, jsonMap, dwSid);
}

TINT32 CPushDataBasic::PushDataMap_SingleAction( SSession *pstSession, TbMarch_action *ptbMarch, TUINT8 ucFlag )
{
    //过滤不需要push的action
    if(ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__THRONE_TIMER
        || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__PRISON_TIMER
        || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__NOTI_TIMER
        || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__IDOL_PERIOD
        || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__THRONE_PERIOD
        || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__TAX)
    {
        return 0;
    }

    TINT32 dwSid = ptbMarch->m_nSid;
    Json::Value jsonMap;
    Json::Value jsonTmp;
    TUINT32 udwSBlockId = 0, udwTBlockId = 0;

    CCommJson::GenMarchInfoForPush(ptbMarch, jsonTmp, ucFlag);

    udwSBlockId = CMapBase::GetBlockIdFromPos(ptbMarch->m_nScid);
    udwTBlockId = CMapBase::GetBlockIdFromPos(ptbMarch->m_nTpos);
    jsonMap[CCommonFunc::NumToString(udwSBlockId)]["svr_map_inc"]["map_action_list"] = jsonTmp;
    jsonMap[CCommonFunc::NumToString(udwTBlockId)]["svr_map_inc"]["map_action_list"] = jsonTmp;
    jsonMap[CCommonFunc::NumToString(udwSBlockId)]["svr_map_inc"]["svr_id"] = dwSid;

    return PushDataBasic_Map(pstSession, EN_SERVICE_TYPE__CLIENT__DATA_PUSH_RSP, jsonMap, dwSid);
}

TINT32 CPushDataBasic::PushDataMap_Normal( SSession *pstSession, vector<SPushActionNode> &vecActionList, vector<SPushWildNode> &vecWildList, TINT32 dwSid )
{
    Json::Value jsonMap;

    GetPushData_BlockMap(vecActionList, vecWildList, jsonMap, dwSid);

    return PushDataBasic_Map(pstSession, EN_SERVICE_TYPE__CLIENT__DATA_PUSH_RSP, jsonMap, dwSid);
}

TINT32 CPushDataBasic::GetPushData_BlockMap( vector<SPushActionNode> &vecActionList, vector<SPushWildNode> &vecWildList, Json::Value &jsonMap, TINT32 dwSid )
{
    jsonMap.clear();

    Json::Value jsonTmp;
    TUINT32 udwSBlockId = 0, udwTBlockId = 0;

    SPushActionNode *pstActionNode = NULL;
    TbMarch_action *ptbAction = NULL;
    for(TINT32 idx = 0; idx < vecActionList.size(); idx++)
    {
        pstActionNode = &vecActionList[idx];
        if(pstActionNode->m_dwType != EN_MARCH_ACTION)
        {
            continue;
        }

        jsonTmp.clear();
        ptbAction = (TbMarch_action*)pstActionNode->m_ptbAction;

        TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("[wavetest] march action: block_map gen json id=%ld, flag=%d", 
            ptbAction->m_nId, pstActionNode->m_ucFlag));

        CCommJson::GenMarchInfoForPush(ptbAction, jsonTmp, pstActionNode->m_ucFlag);

        udwSBlockId = CMapBase::GetBlockIdFromPos(ptbAction->m_nScid);
        udwTBlockId = CMapBase::GetBlockIdFromPos(ptbAction->m_nTpos);
        jsonMap[CCommonFunc::NumToString(udwSBlockId)]["svr_map_inc"]["map_action_list"] = jsonTmp;
        jsonMap[CCommonFunc::NumToString(udwTBlockId)]["svr_map_inc"]["map_action_list"] = jsonTmp;
        jsonMap[CCommonFunc::NumToString(udwSBlockId)]["svr_map_inc"]["svr_id"] = dwSid;
    }

    TbMap *ptbWild = NULL;
    for(TINT32 idx = 0; idx < vecWildList.size(); idx++)
    {
        ptbWild = vecWildList[idx].m_ptbWild;

        if(CMapLogic::IsWildNeedToDelete(ptbWild))
        {
            udwSBlockId = CMapBase::GetBlockIdFromPos(ptbWild->m_nId);
            jsonMap[CCommonFunc::NumToString(udwSBlockId)]["svr_map_inc"]["list"][CCommonFunc::NumToString(ptbWild->m_nId)] = Json::Value(Json::nullValue);
            jsonMap[CCommonFunc::NumToString(udwSBlockId)]["svr_map_inc"]["svr_id"] = dwSid;
        }
        else
        {
            jsonTmp.clear();
            CCommJson::PushData_GenMapJson(ptbWild->m_nSid, ptbWild, jsonTmp);

            udwSBlockId = CMapBase::GetBlockIdFromPos(ptbWild->m_nId);
            jsonMap[CCommonFunc::NumToString(udwSBlockId)]["svr_map_inc"]["list"][CCommonFunc::NumToString(ptbWild->m_nId)] = jsonTmp;
            jsonMap[CCommonFunc::NumToString(udwSBlockId)]["svr_map_inc"]["svr_id"] = dwSid;
        }
    }

    return 0;
}

TINT32 CPushDataBasic::PushDataBasic_Map( SSession *pstSession, TUINT16 uwReqServiceType, Json::Value& rJson, TINT32 dwSid )
{
    CJsonResultGenerator *pJsonGenerator = (CJsonResultGenerator *)pstSession->m_pJsonGenerator;
    CBaseProtocolPack *pobjPack = NULL;
    TUCHAR *pucPackage = NULL;
    TUINT32 udwPackageLen = 0;
    TINT32 dwListSize = 1;
    TINT32 dwRet = 0;
    TINT32 dwTargetType = EN_PUSH_DATA_TYPE__MAP;
    TUINT8 ucCompressFlag = 0;

    // 0. prepare
    Json::Value::Members jsonDataKeys = rJson.getMemberNames();
    TUINT32 udwValidTaskNum = jsonDataKeys.size();
    if(udwValidTaskNum == 0)
    {
        return 0;
    }

    // 1. get down node
    dwRet = GetDownNode(pstSession, pstSession->m_pstUserLinkerNode, pstSession->m_bUserLinkerNodeExist, DOWN_NODE_TYPE__USER_LINKER);
    if(dwRet < 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("PushDataBasic_Map: get down node failed[%d] [seq=%u]", dwRet, pstSession->m_udwSeqNo));
        return -1;
    }

    // 2. get data
    LTasksGroup        stTasks;
    TUINT32 udwTargetBlockId = 0;
    
    for (TINT32 idx = 0; idx < udwValidTaskNum; idx++)
    {
        udwTargetBlockId = atoi(jsonDataKeys[idx].c_str());
        pobjPack = pstSession->m_ppPackTool[idx];

        // get pb data
        pJsonGenerator->GenPushData_Pb(pstSession, uwReqServiceType, rJson[jsonDataKeys[idx]], dwSid);

        // get package
        pobjPack->ResetContent();
        pobjPack->SetServiceType(uwReqServiceType);
        pobjPack->SetSeq(CGlobalServ::GenerateHsReqSeq());
        pobjPack->SetKey(EN_GLOBAL_KEY__REQ_SEQ, pstSession->m_udwSeqNo);
        pobjPack->SetKey(EN_GLOBAL_KEY__TARGET_TYPE, dwTargetType);
        pobjPack->SetKey(EN_GLOBAL_KEY__TARGET_SID, dwSid);
        dwListSize = 1;
        pobjPack->SetKey(EN_GLOBAL_KEY__TARGET_LIST_SIZE, dwListSize);
        pobjPack->SetKey(EN_GLOBAL_KEY__TARGET_LIST, (TUCHAR*)&udwTargetBlockId, sizeof(udwTargetBlockId));
        pobjPack->SetKey(EN_GLOBAL_KEY__RES_BUF_COMPRESS_FLAG, ucCompressFlag);
        pobjPack->SetKey(EN_GLOBAL_KEY__REQ_BUF, (TUCHAR*)&pstSession->m_szTmpClientRspBuf[0], pstSession->m_dwTmpFinalPackLength);

        pobjPack->GetPackage(&pucPackage, &udwPackageLen);

        TSE_LOG_DEBUG(pstSession->m_poServLog, ("PushDataBasic_Map: service=%u, pklen=%u, blockid=%u [seq=%u]", \
            uwReqServiceType, udwPackageLen, udwTargetBlockId, pstSession->m_udwSeqNo));

        // set task
        stTasks.m_Tasks[idx].SetConnSession(pstSession->m_pstUserLinkerNode->m_stDownHandle);
        stTasks.m_Tasks[idx].SetSendData(pucPackage, udwPackageLen);
        stTasks.m_Tasks[idx].SetNeedResponse(0);
    }
    stTasks.SetValidTasks(udwValidTaskNum);

    // 3. send request    
    if(!pstSession->m_poLongConn->SendData(&stTasks))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("PushDataBasic_Map: send request failed [seq=%u]", pstSession->m_udwSeqNo));
        return -2;
    }

    return 0;
}

TINT32 CPushDataBasic::PushDataBasic_TargetUid( SSession *pstSession, TUINT16 uwReqServiceType, Json::Value& rJson, TUINT32 *pudwTargetUidList, TUINT32 udwTargetNum /*= 1*/ )
{
    CJsonResultGenerator *pJsonGenerator = (CJsonResultGenerator *)pstSession->m_pJsonGenerator;
    CBaseProtocolPack *pobjPack = pstSession->m_ppPackTool[0];
    TUCHAR *pucPackage = NULL;
    TUINT32 udwPackageLen = 0;
    TINT32 dwListSize = 1;
    TINT32 dwRet = 0;
    TINT32 dwTargetType = EN_PUSH_DATA_TYPE__UID;
    TUINT8 ucCompressFlag = 0;

    // 1. get down node
    dwRet = GetDownNode(pstSession, pstSession->m_pstUserLinkerNode, pstSession->m_bUserLinkerNodeExist, DOWN_NODE_TYPE__USER_LINKER);
    if(dwRet < 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("PushDataBasic_TargetUid: get down node failed[%d] [seq=%u]", dwRet, pstSession->m_udwSeqNo));
        return -1;
    }

    // 2. get data
    pJsonGenerator->GenPushData_Pb(pstSession, uwReqServiceType, rJson);

    // 3. get package
    pobjPack->ResetContent();
    pobjPack->SetServiceType(uwReqServiceType);
    pobjPack->SetSeq(CGlobalServ::GenerateHsReqSeq());
    pobjPack->SetKey(EN_GLOBAL_KEY__REQ_SEQ, pstSession->m_udwSeqNo);
    pobjPack->SetKey(EN_GLOBAL_KEY__TARGET_TYPE, dwTargetType);
    dwListSize = udwTargetNum;
    pobjPack->SetKey(EN_GLOBAL_KEY__TARGET_LIST_SIZE, dwListSize);
    pobjPack->SetKey(EN_GLOBAL_KEY__TARGET_LIST, (TUCHAR*)pudwTargetUidList, sizeof(TUINT32)*dwListSize);
    dwListSize = 0;
    pobjPack->SetKey(EN_GLOBAL_KEY__EXCEPT_LIST_SIZE, dwListSize);
    pobjPack->SetKey(EN_GLOBAL_KEY__RES_BUF_COMPRESS_FLAG, ucCompressFlag);
    pobjPack->SetKey(EN_GLOBAL_KEY__REQ_BUF, (TUCHAR*)&pstSession->m_szTmpClientRspBuf[0], pstSession->m_dwTmpFinalPackLength);

    pobjPack->GetPackage(&pucPackage, &udwPackageLen);

    TSE_LOG_DEBUG(pstSession->m_poServLog, ("PushDataBasic_TargetUid: service=%u, pklen=%u, target_uid_0=%u, target_uid_num=%u [seq=%u]", \
        uwReqServiceType, udwPackageLen, pudwTargetUidList[0], udwTargetNum, pstSession->m_udwSeqNo));

    // 4. send request
    LTasksGroup        stTasks;
    stTasks.m_Tasks[0].SetConnSession(pstSession->m_pstUserLinkerNode->m_stDownHandle);
    stTasks.m_Tasks[0].SetSendData(pucPackage, udwPackageLen);
    stTasks.m_Tasks[0].SetNeedResponse(0);
    stTasks.SetValidTasks(1);
    if(!pstSession->m_poLongConn->SendData(&stTasks))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("PushDataBasic_TargetUid: send request failed [seq=%u]", pstSession->m_udwSeqNo));
        return -2;
    }

    return 0;
}

TINT32 CPushDataBasic::PushDataBasic_TargetUid( SSession *pstSession, TUINT16 uwReqServiceType, Json::Value& rJson, TUINT32 udwTargetUid )
{
    return PushDataBasic_TargetUid(pstSession, uwReqServiceType, rJson, &udwTargetUid, 1);
}

TINT32 CPushDataBasic::PushDataBasic_TargetAid( SSession *pstSession, TUINT16 uwReqServiceType, Json::Value& rJson, TUINT32 udwTargetAid, TUINT32 *pudwExceptUidList /*= NULL*/, TUINT32 udwExceptListNum /*= 0*/ )
{
    CJsonResultGenerator *pJsonGenerator = (CJsonResultGenerator *)pstSession->m_pJsonGenerator;
    CBaseProtocolPack *pobjPack = pstSession->m_ppPackTool[0];
    TUCHAR *pucPackage = NULL;
    TUINT32 udwPackageLen = 0;
    TINT32 dwListSize = 1;
    TINT32 dwRet = 0;
    TINT32 dwTargetType = EN_PUSH_DATA_TYPE__AID;
    TUINT8 ucCompressFlag = 0;

    // 1. get down node
    dwRet = GetDownNode(pstSession, pstSession->m_pstUserLinkerNode, pstSession->m_bUserLinkerNodeExist, DOWN_NODE_TYPE__USER_LINKER);
    if(dwRet < 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("PushDataBasic_TargetAid: get down node failed[%d] [seq=%u]", dwRet, pstSession->m_udwSeqNo));
        return -1;
    }

    // 2. get data
    pJsonGenerator->GenPushData_Pb(pstSession, uwReqServiceType, rJson);

    // 3. get package
    pobjPack->ResetContent();
    pobjPack->SetServiceType(uwReqServiceType);
    pobjPack->SetSeq(CGlobalServ::GenerateHsReqSeq());
    pobjPack->SetKey(EN_GLOBAL_KEY__REQ_SEQ, pstSession->m_udwSeqNo);
    pobjPack->SetKey(EN_GLOBAL_KEY__TARGET_TYPE, dwTargetType);
    dwListSize = 1;
    pobjPack->SetKey(EN_GLOBAL_KEY__TARGET_LIST_SIZE, dwListSize);
    pobjPack->SetKey(EN_GLOBAL_KEY__TARGET_LIST, (TUCHAR*)&udwTargetAid, sizeof(udwTargetAid));
    dwListSize = udwExceptListNum;
    pobjPack->SetKey(EN_GLOBAL_KEY__EXCEPT_LIST_SIZE, dwListSize);
    if(dwListSize > 0)
    {
        dwTargetType = EN_PUSH_DATA_TYPE__UID;
        pobjPack->SetKey(EN_GLOBAL_KEY__EXCEPT_TYPE, dwTargetType);
        pobjPack->SetKey(EN_GLOBAL_KEY__EXCEPT_LIST, (TUCHAR*)pudwExceptUidList, sizeof(TUINT32)*dwListSize);
    }
    pobjPack->SetKey(EN_GLOBAL_KEY__RES_BUF_COMPRESS_FLAG, ucCompressFlag);
    pobjPack->SetKey(EN_GLOBAL_KEY__REQ_BUF, (TUCHAR*)&pstSession->m_szTmpClientRspBuf[0], pstSession->m_dwTmpFinalPackLength);

    pobjPack->GetPackage(&pucPackage, &udwPackageLen);

    TSE_LOG_DEBUG(pstSession->m_poServLog, ("PushDataBasic_TargetAid: service=%u, pklen=%u, target_aid=%u, except_uid_0=%u, except_uid_num=%u [seq=%u]", \
        uwReqServiceType, udwPackageLen, udwTargetAid, udwExceptListNum == 0 ? 0 : pudwExceptUidList[0], udwExceptListNum, pstSession->m_udwSeqNo));

    // 4. send request
    LTasksGroup        stTasks;
    stTasks.m_Tasks[0].SetConnSession(pstSession->m_pstUserLinkerNode->m_stDownHandle);
    stTasks.m_Tasks[0].SetSendData(pucPackage, udwPackageLen);
    stTasks.m_Tasks[0].SetNeedResponse(0);
    stTasks.SetValidTasks(1);
    if(!pstSession->m_poLongConn->SendData(&stTasks))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("PushDataBasic_TargetAid: send request failed [seq=%u]", pstSession->m_udwSeqNo));
        return -2;
    }

    return 0;
}

TINT32 CPushDataBasic::PushDataBasic_Sid(SSession *pstSession, TUINT16 uwReqServiceType, Json::Value& rJson, TUINT32 udwSid, TUINT32 *pudwExceptUidList = NULL, TUINT32 udwExceptListNum = 0)
{
    CJsonResultGenerator *pJsonGenerator = (CJsonResultGenerator *)pstSession->m_pJsonGenerator;
    CBaseProtocolPack *pobjPack = pstSession->m_ppPackTool[0];
    TUCHAR *pucPackage = NULL;
    TUINT32 udwPackageLen = 0;
    TINT32 dwListSize = 1;
    TINT32 dwRet = 0;
    TINT32 dwTargetType = EN_PUSH_DATA_TYPE__SID;
    TUINT8 ucCompressFlag = 0;

    // 1. get down node
    dwRet = GetDownNode(pstSession, pstSession->m_pstUserLinkerNode, pstSession->m_bUserLinkerNodeExist, DOWN_NODE_TYPE__USER_LINKER);
    if (dwRet < 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("PushDataBasic_TargetAid: get down node failed[%d] [seq=%u]", dwRet, pstSession->m_udwSeqNo));
        return -1;
    }

    // 2. get data
    pJsonGenerator->GenPushData_Pb(pstSession, uwReqServiceType, rJson);

    // 3. get package
    pobjPack->ResetContent();
    pobjPack->SetServiceType(uwReqServiceType);
    pobjPack->SetSeq(CGlobalServ::GenerateHsReqSeq());

    pobjPack->SetKey(EN_GLOBAL_KEY__REQ_SEQ, pstSession->m_udwSeqNo);
    pobjPack->SetKey(EN_GLOBAL_KEY__TARGET_TYPE, dwTargetType);

    pobjPack->SetKey(EN_GLOBAL_KEY__TARGET_LIST_SIZE, dwListSize);
    pobjPack->SetKey(EN_GLOBAL_KEY__TARGET_LIST, (TUCHAR*)&udwSid, sizeof(udwSid));

    dwListSize = udwExceptListNum;
    pobjPack->SetKey(EN_GLOBAL_KEY__EXCEPT_LIST_SIZE, dwListSize);
    if (dwListSize > 0)
    {
        dwTargetType = EN_PUSH_DATA_TYPE__UID;
        pobjPack->SetKey(EN_GLOBAL_KEY__EXCEPT_TYPE, dwTargetType);
        pobjPack->SetKey(EN_GLOBAL_KEY__EXCEPT_LIST, (TUCHAR*)pudwExceptUidList, sizeof(TUINT32)*dwListSize);
    }

    pobjPack->SetKey(EN_GLOBAL_KEY__RES_BUF_COMPRESS_FLAG, ucCompressFlag);
    pobjPack->SetKey(EN_GLOBAL_KEY__REQ_BUF, (TUCHAR*)&pstSession->m_szTmpClientRspBuf[0], pstSession->m_dwTmpFinalPackLength);

    pobjPack->GetPackage(&pucPackage, &udwPackageLen);

    TSE_LOG_DEBUG(pstSession->m_poServLog, ("PushDataBasic_TargetAid: service=%u, pklen=%u, target_sid=%u, except_uid_0=%u, except_uid_num=%u [seq=%u]", 
        uwReqServiceType, udwPackageLen, udwSid, udwExceptListNum == 0 ? 0 : pudwExceptUidList[0], udwExceptListNum, pstSession->m_udwSeqNo));

    // 4. send request
    LTasksGroup stTasks;
    stTasks.m_Tasks[0].SetConnSession(pstSession->m_pstUserLinkerNode->m_stDownHandle);
    stTasks.m_Tasks[0].SetSendData(pucPackage, udwPackageLen);
    stTasks.m_Tasks[0].SetNeedResponse(0);
    stTasks.SetValidTasks(1);
    if (!pstSession->m_poLongConn->SendData(&stTasks))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("PushDataBasic_TargetAid: send request failed [seq=%u]", pstSession->m_udwSeqNo));
        return -2;
    }

    return 0;
}

TINT32 CPushDataBasic::GetDownNode( SSession *pstSession, SDownNode* &pstNode, TBOOL &bExist, TINT32 dwNodeType )
{
    if(bExist == FALSE)
    {
        CDownMgr *pobjDownMgr = CDownMgr::Instance();
        pstNode = NULL;

        if (S_OK == pobjDownMgr->zk_GetNode(dwNodeType, &pstNode))
        {
            bExist = TRUE;
            TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("GetDownNode: get node[type=%d][ip=%s:%d] succ [seq=%u]", \
                dwNodeType, pstNode->m_szIP, pstNode->m_uwPort, pstSession->m_udwSeqNo));
        }
        else
        {
            TSE_LOG_ERROR(CGlobalServ::m_poServLog, ("GetDownNode: get node[type=%d] failed [seq=%u]", \
                dwNodeType, pstSession->m_udwSeqNo));
            return -1;
        }

        if(NULL == pstNode->m_stDownHandle.handle)
        {
            bExist = FALSE;
            TSE_LOG_ERROR(CGlobalServ::m_poServLog, ("GetDownNode: get node[type=%d][ip=%s:%d] failed, handle=NULL [seq=%u]", \
                dwNodeType, pstNode->m_szIP, pstNode->m_uwPort, pstSession->m_udwSeqNo));
            return -2;
        }
    }

    return 0;
}

TINT32 CPushDataBasic::PushDataUid_Tips( SSession *pstSession, TUINT32 udwTargetUid, TbTips *ptbTips )
{
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("PushDataUid_Tips: target=%u [seq=%u]", 
        udwTargetUid, pstSession->m_udwSeqNo));

    TUINT32 udwJsonIndex = 0;
    Json::Value rjsonResult(Json::objectValue);
    rjsonResult["svr_tips"] = Json::Value(Json::arrayValue);
    rjsonResult["svr_tips"][udwJsonIndex] = Json::Value(Json::arrayValue);
    rjsonResult["svr_tips"][udwJsonIndex].append(ptbTips->m_nTime);
    rjsonResult["svr_tips"][udwJsonIndex].append(ptbTips->m_nType);
    rjsonResult["svr_tips"][udwJsonIndex].append(ptbTips->m_sContent);

    // send push data
    return PushDataBasic_TargetUid(pstSession, EN_SERVICE_TYPE__CLIENT__DATA_PUSH_RSP, rjsonResult, udwTargetUid);
}

TINT32 CPushDataBasic::PushDataUid_AlAction( SSession *pstSession, TUINT32 udwTargetUid, TbAlliance_action* ptbAction, TINT32 dwUpdtType )
{
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("PushDataUid_AlAction:target=%u action_id=%ld, suid=%ld sal=%ld, [main=%ld,sec=%ld,status=%d] [seq=%u]", 
        udwTargetUid, ptbAction->m_nId, ptbAction->m_nSuid, ptbAction->m_nSal, 
        ptbAction->m_nMclass, ptbAction->m_nSclass, ptbAction->m_nStatus, pstSession->m_udwSeqNo));

    Json::Value rjsonResult(Json::objectValue);    
    if(ptbAction->m_nSal > 0)
    {
        rjsonResult["svr_al_action_list"] = Json::Value(Json::objectValue);
        CCommJson::GenAlActionInfoForPush(ptbAction, rjsonResult["svr_al_action_list"], dwUpdtType);
    }

    if(udwTargetUid == ptbAction->m_nSuid)
    {
        rjsonResult["svr_action_list"] = Json::Value(Json::objectValue);
        CCommJson::GenAlActionInfoForPush(ptbAction, rjsonResult["svr_action_list"], dwUpdtType);
    }

    // send push data
    return PushDataBasic_TargetUid(pstSession, EN_SERVICE_TYPE__CLIENT__DATA_PUSH_RSP, rjsonResult, udwTargetUid);
}

TINT32 CPushDataBasic::PushDataUid_MarchActionSourceUid( SSession *pstSession, TUINT32 udwTargetUid, TbMarch_action* ptbAction, TINT32 dwUpdtType )
{
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("PushDataUid_MarchActionSourceUid: target=%u action_id=%ld, suid=%ld sal=%ld tal=%ld tid=%ld, [main=%ld,sec=%ld,status=%d] [seq=%u]", 
        udwTargetUid, ptbAction->m_nId, ptbAction->m_nSuid, ptbAction->m_nSal, ptbAction->m_nTal, ptbAction->m_nTid, 
        ptbAction->m_nMclass, ptbAction->m_nSclass, ptbAction->m_nStatus, pstSession->m_udwSeqNo));

    Json::Value rjsonResult(Json::objectValue);
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbMarch_action tbTmpMarch = *ptbAction;

    //修正marching状态中reinforce的时间显示
    if(ptbAction->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_REINFORCE
        && ptbAction->m_nStatus == EN_MARCH_STATUS__PREPARING
        && ptbAction->m_nEtime == INT64_MAX)
    {
        //寻找主rally war action
        for(TUINT32 udwIdz = 0; udwIdz < pstUser->m_udwMarchNum; udwIdz++)
        {
            if(pstUser->m_aucMarchFlag[udwIdz] == EN_TABLE_UPDT_FLAG__DEL)
            {
                continue;
            }
            if(pstUser->m_atbMarch[udwIdz].m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR
                &&ptbAction->m_nTid == pstUser->m_atbMarch[udwIdz].m_nId)
            {
                if(pstUser->m_atbMarch[udwIdz].m_nStatus == EN_MARCH_STATUS__MARCHING)
                {
                    tbTmpMarch.m_nEtime = pstUser->m_atbMarch[udwIdz].m_nEtime;
                }
                break;
            }
        }
    }

    // 生成action数据
    CCommJson::GenMarchInfoForPush(&tbTmpMarch, rjsonResult["svr_action_list"], dwUpdtType);

    // send push data
    return PushDataBasic_TargetUid(pstSession, EN_SERVICE_TYPE__CLIENT__DATA_PUSH_RSP, rjsonResult, udwTargetUid);
}

TINT32 CPushDataBasic::PushDataUid_MarchActionTargetUid( SSession *pstSession, TUINT32 udwTargetUid, TbMarch_action* ptbAction, TINT32 dwUpdtType )
{
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("PushDataUid_MarchActionTargetUid: target=%u action_id=%ld, suid=%ld sal=%ld tal=%ld tid=%ld, [main=%ld,sec=%ld,status=%d] [seq=%u]", 
        udwTargetUid, ptbAction->m_nId, ptbAction->m_nSuid, ptbAction->m_nSal, ptbAction->m_nTal, ptbAction->m_nTid, 
        ptbAction->m_nMclass, ptbAction->m_nSclass, ptbAction->m_nStatus, pstSession->m_udwSeqNo));

    Json::Value rjsonResult(Json::objectValue);

    CCommJson::GenMarchInfoForPush(ptbAction, rjsonResult["svr_p_action_list"], dwUpdtType);

    // send push data
    return PushDataBasic_TargetUid(pstSession, EN_SERVICE_TYPE__CLIENT__DATA_PUSH_RSP, rjsonResult, udwTargetUid);
}

TINT32 CPushDataBasic::PushDataAid_March( SSession *pstSession, TbMarch_action* ptbAction, TINT32 dwUpdtType, const TCHAR* pszTable, TUINT32 udwTargetAid, TUINT32 *pudwExceptUidList, TUINT32 udwExceptListNum )
{
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("PushDataAid_March: target=%u action_id=%ld, suid=%ld sal=%ld tal=%ld tid=%ld, [main=%ld,sec=%ld,status=%d] [seq=%u]", 
        udwTargetAid, ptbAction->m_nId, ptbAction->m_nSuid, ptbAction->m_nSal, ptbAction->m_nTal, ptbAction->m_nTid, 
        ptbAction->m_nMclass, ptbAction->m_nSclass, ptbAction->m_nStatus, pstSession->m_udwSeqNo));

    // action
    Json::Value rjsonResult(Json::objectValue);
    CCommJson::GenMarchInfoForPush(ptbAction, rjsonResult[pszTable], dwUpdtType);

    // send push data
    return PushDataBasic_TargetAid(pstSession, EN_SERVICE_TYPE__CLIENT__DATA_PUSH_RSP, rjsonResult, udwTargetAid, pudwExceptUidList, udwExceptListNum);
}

TINT32 CPushDataBasic::PushDataAid_AlAction( SSession *pstSession, TUINT32 udwTargetAid, TbAlliance_action* ptbAction, TINT32 dwUpdtType, TUINT32 *pudwExceptUidList /*= NULL*/, TUINT32 udwExceptListNum /*= 0*/ )
{
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("PushDataAid_AlAction:target=%u action_id=%ld, suid=%ld sal=%ld, [main=%ld,sec=%ld,status=%d] [seq=%u]", 
        udwTargetAid, ptbAction->m_nId, ptbAction->m_nSuid, ptbAction->m_nSal, 
        ptbAction->m_nMclass, ptbAction->m_nSclass, ptbAction->m_nStatus, pstSession->m_udwSeqNo));

    Json::Value rjsonResult;
    rjsonResult["svr_al_action_list"] = Json::Value(Json::objectValue);
    CCommJson::GenAlActionInfoForPush(ptbAction, rjsonResult["svr_al_action_list"], dwUpdtType);

    // send push data
    return PushDataBasic_TargetAid(pstSession, EN_SERVICE_TYPE__CLIENT__DATA_PUSH_RSP, rjsonResult, udwTargetAid, pudwExceptUidList, udwExceptListNum);
}
