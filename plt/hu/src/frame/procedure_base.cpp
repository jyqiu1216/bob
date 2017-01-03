#include "procedure_base.h"
#include "aws_request.h"
#include "statistic.h"
#include "global_serv.h"

#include "db_request.h"
#include "db_response.h"
#include "game_info.h"
#include "common_func.h"
#include "city_info.h"

#include "conf_base.h"
#include "quest_logic.h"
#include "bounty_logic.h"
#include "common_logic.h"
#include "common_json.h"
#include "common_base.h"
#include "tool_base.h"
#include "city_base.h"
#include "msg_base.h"

#include "lock_info.h"
#include "report_svr_request.h"

#include "pushdata_action.h"


TINT32 CBaseProcedure::SendAwsRequest(SSession *pstSession, TUINT16 uwReqServiceType)
{
    TUCHAR *pszPack = NULL;
    TUINT32 udwPackLen = 0;
    CDownMgr *pobjDownMgr = CDownMgr::Instance();
    LTasksGroup	stTasksGroup;
    vector<AwsReqInfo*>& vecReq = pstSession->m_vecAwsReq;

    if(vecReq.size() == 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("SendAwsRequest: m_vecAwsReq.size() == 0"));
        return -1;
    }

    pstSession->m_uddwDownRqstTimeBeg = CTimeUtils::GetCurTimeUs();

    // 1. 获取下游——对于一个session来说，每次只需获取一次
    if(pstSession->m_bAwsProxyNodeExist == FALSE)
    {
        pstSession->m_pstAwsProxyNode = NULL;
        if (S_OK == pobjDownMgr->zk_GetNode(DOWN_NODE_TYPE__AWS_PROXY, &pstSession->m_pstAwsProxyNode))
        {
            pstSession->m_bAwsProxyNodeExist = TRUE;
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("Get AwsProx node succ [seq=%u]", pstSession->m_udwSeqNo));
        }
        else
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("Get AwsProxy node fail [seq=%u]", pstSession->m_udwSeqNo));
            return -2;
        }

        if(NULL == pstSession->m_pstAwsProxyNode->m_stDownHandle.handle)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("SendAwsRequest:handle [handlevalue=0x%p] [seq=%u]", \
                pstSession->m_pstAwsProxyNode->m_stDownHandle.handle, pstSession->m_udwSeqNo));
            return -3;
        }
    }
    
    
    if(pstSession->m_bMapSvrExist == FALSE)//wave@20160530
    {
        pstSession->m_pstMapSvrNode = NULL;
        if (S_OK == pobjDownMgr->zk_GetNode(DOWN_NODE_TYPE__MAP_SVR_PROXY, &pstSession->m_pstMapSvrNode))
        {
            pstSession->m_bMapSvrExist = TRUE;
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("Get MapSvr node succ [seq=%u]", pstSession->m_udwSeqNo));
        }
        else
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("Get MapSvr node fail [seq=%u]", pstSession->m_udwSeqNo));
            return -21;
        }
        if(NULL == pstSession->m_pstMapSvrNode->m_stDownHandle.handle)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("SendAwsRequest:handle [handlevalue=0x%p] [seq=%u]", \
                pstSession->m_pstMapSvrNode->m_stDownHandle.handle, pstSession->m_udwSeqNo));
            return -3;
        }
    }

    if (pstSession->m_bBuffActionSvrNodeExist == FALSE)
    {
        pstSession->m_pstBuffActionSvrNode = NULL;
        if (S_OK == pobjDownMgr->zk_GetNode(DOWN_NODE_TYPE__BUFF_ACTION_SVR, &pstSession->m_pstBuffActionSvrNode))
        {
            pstSession->m_bBuffActionSvrNodeExist = TRUE;
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("Get buff action node succ [seq=%u]", pstSession->m_udwSeqNo));
        }
        else
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("Get buff action node fail [seq=%u]", pstSession->m_udwSeqNo));
            return -21;
        }
        if (NULL == pstSession->m_pstBuffActionSvrNode->m_stDownHandle.handle)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("SendAwsRequest:buff action handle [handlevalue=0x%p] [seq=%u]", 
                pstSession->m_pstBuffActionSvrNode->m_stDownHandle.handle, pstSession->m_udwSeqNo));
            return -3;
        }
    }

    if (pstSession->m_bAlActionSvrNodeExist == FALSE)
    {
        pstSession->m_pstAlActionSvrNode = NULL;
        if (S_OK == pobjDownMgr->zk_GetNode(DOWN_NODE_TYPE__AL_ACTION_SVR, &pstSession->m_pstAlActionSvrNode))
        {
            pstSession->m_bAlActionSvrNodeExist = TRUE;
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("Get al action node succ [seq=%u]", pstSession->m_udwSeqNo));
        }
        else
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("Get al action node fail [seq=%u]", pstSession->m_udwSeqNo));
            return -21;
        }
        if (NULL == pstSession->m_pstAlActionSvrNode->m_stDownHandle.handle)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("SendAwsRequest: al action handle [handlevalue=0x%p] [seq=%u]", 
                pstSession->m_pstAlActionSvrNode->m_stDownHandle.handle, pstSession->m_udwSeqNo));
            return -3;
        }
    }

    if (pstSession->m_bMarchActionSvrNodeExist == FALSE)
    {
        pstSession->m_pstMarchActionSvrNode = NULL;
        if (S_OK == pobjDownMgr->zk_GetNode(DOWN_NODE_TYPE__MARCH_ACTION_SVR, &pstSession->m_pstMarchActionSvrNode))
        {
            pstSession->m_bMarchActionSvrNodeExist = TRUE;
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("Get march action node succ [seq=%u]", pstSession->m_udwSeqNo));
        }
        else
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("Get march action node fail [seq=%u]", pstSession->m_udwSeqNo));
            return -21;
        }
        if (NULL == pstSession->m_pstMarchActionSvrNode->m_stDownHandle.handle)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("SendAwsRequest: march action handle [handlevalue=0x%p] [seq=%u]", 
                pstSession->m_pstMarchActionSvrNode->m_stDownHandle.handle, pstSession->m_udwSeqNo));
            return -3;
        }
    }
    
//     if (pstSession->m_bReportUserSvrNodeExist == FALSE)
//     {
//         pstSession->m_pstReportUserSvrNode = NULL;
//         if (S_OK == pobjDownMgr->zk_GetGroupNode(pobjDownMgr->m_pReportUserService, &pstSession->m_pstReportUserSvrNode, 0))
//         {
//             pstSession->m_bReportUserSvrNodeExist = TRUE;
//             TSE_LOG_DEBUG(pstSession->m_poServLog, ("Get report user node succ [seq=%u]", pstSession->m_udwSeqNo));
//         }
//         else
//         {
//             TSE_LOG_ERROR(pstSession->m_poServLog, ("Get report user node fail [seq=%u]", pstSession->m_udwSeqNo));
//             return -21;
//         }
//         if (NULL == pstSession->m_pstReportUserSvrNode->m_stDownHandle.handle)
//         {
//             TSE_LOG_ERROR(pstSession->m_poServLog, ("SendAwsRequest: report user handle [handlevalue=0x%p] [seq=%u]", 
//                 pstSession->m_pstReportUserSvrNode->m_stDownHandle.handle, pstSession->m_udwSeqNo));
//             return -3;
//         }
//     }
    
    // 2. 封装请求、打包、设置task
	for(unsigned int i = 0; i < vecReq.size() && i < MAX_AWS_REQ_TASK_NUM; ++i)
    {
        if(vecReq[i]->sOperatorName == "UpdateItem"
            || vecReq[i]->sOperatorName == "DeleteItem"
            || vecReq[i]->sOperatorName == "PutItem")
        {
            pstSession->m_udwDownRqstType = (pstSession->m_udwDownRqstType == 0 || pstSession->m_udwDownRqstType == 2) ? 2 : 3;
            TSE_LOG_INFO(pstSession->m_poServLog, ("SendAwsRequest:[task_id=%u] [handlevalue=0x%p] [req=%s] [seq=%u]", 
                i, pstSession->m_pstAwsProxyNode->m_stDownHandle.handle, vecReq[i]->sReqContent.c_str(), pstSession->m_udwSeqNo));
        }
        else
        {
            pstSession->m_udwDownRqstType = (pstSession->m_udwDownRqstType == 0 || pstSession->m_udwDownRqstType == 1) ? 1 : 3;
        }
        CBaseProtocolPack* pPack = pstSession->m_ppPackTool[i];
        pPack->ResetContent();
        pPack->SetServiceType(uwReqServiceType);
        pPack->SetSeq(CGlobalServ::GenerateHsReqSeq());
        pPack->SetKey(EN_GLOBAL_KEY__REQ_SEQ, pstSession->m_udwSeqNo);
        pPack->SetKey(EN_GLOBAL_KEY__REQ_TYPE, pstSession->m_udwContentType);
        pPack->SetKey(EN_GLOBAL_KEY__INDEX_NO, vecReq[i]->udwIdxNo);
        pPack->SetKey(EN_GLOBAL_KEY__TABLE_NAME, (unsigned char*)vecReq[i]->sTableName.c_str(), vecReq[i]->sTableName.size());
        pPack->SetKey(EN_GLOBAL_KEY__OPERATOR_NAME, (unsigned char*)vecReq[i]->sOperatorName.c_str(), vecReq[i]->sOperatorName.size());

		pPack->SetKey(EN_GLOBAL_KEY__AWSTBL_CACHE_MODE, vecReq[i]->m_udwCacheMode);
		pPack->SetKey(EN_GLOBAL_KEY__AWSTBL_ROUTE_FLD, (unsigned char*)vecReq[i]->m_strRouteFld.c_str(), vecReq[i]->m_strRouteFld.size() );
		pPack->SetKey(EN_GLOBAL_KEY__AWSTBL_ROUTE_THRD, vecReq[i]->m_uddwRouteThrd);
		
		pPack->SetKey(EN_GLOBAL_KEY__AWSTBL_HASH_KEY, (unsigned char*)vecReq[i]->m_strHashKey.c_str(),vecReq[i]->m_strHashKey.size());	
		if( !vecReq[i]->m_strRangeKey.empty() )
			pPack->SetKey(EN_GOLBAL_KEY__AWSTBL_RANGE_KEY, (unsigned char*)vecReq[i]->m_strRangeKey.c_str(),vecReq[i]->m_strRangeKey.size());	

        pPack->SetKey(EN_KEY_HU2HS__REQ_BUF, (unsigned char*)vecReq[i]->sReqContent.c_str(), vecReq[i]->sReqContent.size());
        if (!vecReq[i]->bIsDefaultLock)
        {
            pPack->SetKey(EN_MAP_SVR_KEY__NEED_LOCK, vecReq[i]->dwLock);
            pPack->SetKey(EN_MAP_SVR_KEY__NEED_UNLOCK, vecReq[i]->dwUnlock);
        }
        
        //wave@20160530
        if (vecReq[i]->sTableName.find("_split_map_") != string::npos)
        {
            stTasksGroup.m_Tasks[i].SetConnSession(pstSession->m_pstMapSvrNode->m_stDownHandle);
        }
        else if (vecReq[i]->sTableName.find("_global_action") != string::npos)
        {
            pPack->SetKey(EN_ACTION_SVR_KEY__SVR_ID, pstSession->m_stReqParam.m_udwSvrId);
            pPack->SetKey(EN_ACTION_SVR_KEY__ACTION_TYPE, EN_UID_ACTION);
            stTasksGroup.m_Tasks[i].SetConnSession(pstSession->m_pstBuffActionSvrNode->m_stDownHandle);
        }
        else if (vecReq[i]->sTableName.find("_global_alliance_action") != string::npos)
        {
            pPack->SetKey(EN_ACTION_SVR_KEY__SVR_ID, pstSession->m_stReqParam.m_udwSvrId);
            pPack->SetKey(EN_ACTION_SVR_KEY__ACTION_TYPE, EN_AID_ACTION);
            stTasksGroup.m_Tasks[i].SetConnSession(pstSession->m_pstAlActionSvrNode->m_stDownHandle);
        }
        else if (vecReq[i]->sTableName.find("_global_march_action") != string::npos)
        {
            pPack->SetKey(EN_ACTION_SVR_KEY__SVR_ID, pstSession->m_stReqParam.m_udwSvrId);
            pPack->SetKey(EN_ACTION_SVR_KEY__ACTION_TYPE, EN_MARCH_ACTION);
            stTasksGroup.m_Tasks[i].SetConnSession(pstSession->m_pstMarchActionSvrNode->m_stDownHandle);
        }
        else
        {
            stTasksGroup.m_Tasks[i].SetConnSession(pstSession->m_pstAwsProxyNode->m_stDownHandle);
        }
        
        pPack->GetPackage(&pszPack, &udwPackLen);

        stTasksGroup.m_Tasks[i].SetSendData(pszPack, udwPackLen);
        stTasksGroup.m_Tasks[i].SetNeedResponse(1);
    }

    // c. 设置task
    stTasksGroup.m_UserData1.ptr = pstSession;
    stTasksGroup.m_UserData2.u32 = pstSession->m_udwSeqNo;//wave@20160104:为解决主程序已处理，但后续有回包返回时的情况

    stTasksGroup.SetValidTasks(vecReq.size());
    stTasksGroup.m_uTaskTimeOutVal = DOWN_SEARCH_NODE_TIMEOUT_MS;

    TSE_LOG_DEBUG(pstSession->m_poServLog, ("aws req: SendAwsRequest: [vaildtasksnum=%u] [seq=%u]", 
        vecReq.size(),
        pstSession->m_udwSeqNo));

    pstSession->ResetAwsInfo();

    // 3. 发送数据    
    pstSession->m_bProcessing = FALSE;    
    pstSession->m_uddwProcessEndTime = CTimeUtils::GetCurTimeUs();
    TSE_LOG_INFO(pstSession->m_poServLog, ("aws_process_cost: [processBtime=%lu] [processEtime=%lu] [processCtime=%lu] [seq=%u]", 
                                           pstSession->m_uddwProcessBegTime, 
                                           pstSession->m_uddwProcessEndTime, 
                                           pstSession->m_uddwProcessEndTime - pstSession->m_uddwProcessBegTime, 
                                           pstSession->m_udwSeqNo));
    if(!pstSession->m_poLongConn->SendData(&stTasksGroup))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("SendGlobalHsRequest send data failed! [seq=%u]", pstSession->m_udwSeqNo));
        return -4;
    }

    return 0;
}

TINT32 CBaseProcedure::SendDbRequest(SSession *pstSession, TUINT16 uwReqServiceType)
{
    TUCHAR *pszPack = NULL;
    TUINT32 udwPackLen = 0;
    CDownMgr *pobjDownMgr = CDownMgr::Instance();
    LTasksGroup	stTasksGroup;
    vector<DbReqInfo*>& vecReq = pstSession->m_vecDbReq;

    if(vecReq.size() == 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("SendDbRequest: m_vecDbReq.size() == 0"));
        return -1;
    }

    pstSession->m_uddwDownRqstTimeBeg = CTimeUtils::GetCurTimeUs();

    // 1. 获取下游——对于一个session来说，每次只需获取一次
    if(pstSession->m_bDbProxyNodeExist == FALSE)
    {
        pstSession->m_pstDbProxyNode = NULL;
        if (S_OK == pobjDownMgr->zk_GetNode(DOWN_NODE_TYPE__DB_PROXY, &pstSession->m_pstDbProxyNode))
        {
            pstSession->m_bDbProxyNodeExist = TRUE;
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("Get DbProx node succ [seq=%u]", pstSession->m_udwSeqNo));
        }
        else
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("Get DbProx node fail [seq=%u]", pstSession->m_udwSeqNo));
            return -2;
        }
    }

    // 2. 封装请求、打包、设置task
    for(unsigned int i = 0; i < vecReq.size(); ++i)
    {
        if(vecReq[i]->sOperatorName == "replace"
            || vecReq[i]->sOperatorName == "delete"
            || vecReq[i]->sOperatorName == "set"
            || vecReq[i]->sOperatorName == "insert"
            || vecReq[i]->sOperatorName == "update")
        {
            pstSession->m_udwDownRqstType = (pstSession->m_udwDownRqstType == 0 || pstSession->m_udwDownRqstType == 2) ? 2 : 3;
        }
        else
        {
            pstSession->m_udwDownRqstType = (pstSession->m_udwDownRqstType == 0 || pstSession->m_udwDownRqstType == 1) ? 1 : 3;
        }
        CBaseProtocolPack* pPack = pstSession->m_ppPackTool[i];
        pPack->ResetContent();
        pPack->SetServiceType(uwReqServiceType);
        pPack->SetSeq(CGlobalServ::GenerateHsReqSeq());
        pPack->SetKey(EN_GLOBAL_KEY__REQ_SEQ, pstSession->m_udwSeqNo);
        pPack->SetKey(EN_GLOBAL_KEY__DB_IDX, vecReq[i]->udwDbIdx);
        pPack->SetKey(EN_GLOBAL_KEY__INDEX_NO, vecReq[i]->udwIdxNo);
        pPack->SetKey(EN_GLOBAL_KEY__TABLE_NAME, (unsigned char*)vecReq[i]->sTableName.c_str(), vecReq[i]->sTableName.size());
        pPack->SetKey(EN_GLOBAL_KEY__OPERATOR_NAME, (unsigned char*)vecReq[i]->sOperatorName.c_str(), vecReq[i]->sOperatorName.size());
        pPack->SetKey(EN_KEY_HU2HS__REQ_BUF, (unsigned char*)vecReq[i]->sReqContent.c_str(), vecReq[i]->sReqContent.size());
        pPack->GetPackage(&pszPack, &udwPackLen);

        if(NULL == pstSession->m_pstDbProxyNode->m_stDownHandle.handle)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("SendDbRequest:handle [task_id=%u] [handlevalue=0x%p] [seq=%u]", \
                i, pstSession->m_pstDbProxyNode->m_stDownHandle.handle, pstSession->m_udwSeqNo));
            return -3;
        }

        stTasksGroup.m_Tasks[i].SetConnSession(pstSession->m_pstDbProxyNode->m_stDownHandle);
        stTasksGroup.m_Tasks[i].SetSendData(pszPack, udwPackLen);
        stTasksGroup.m_Tasks[i].SetNeedResponse(1);
    }

    // c. 设置task
    stTasksGroup.m_UserData1.ptr = pstSession;
    stTasksGroup.m_UserData2.u32 = pstSession->m_udwSeqNo;//wave@20160104:为解决主程序已处理，但后续有回包返回时的情况

    stTasksGroup.SetValidTasks(vecReq.size());
    stTasksGroup.m_uTaskTimeOutVal = DOWN_SEARCH_NODE_TIMEOUT_MS;

    pstSession->ResetDbInfo();

    // 3. 发送数据
    pstSession->m_bProcessing = FALSE;
    pstSession->m_uddwProcessEndTime = CTimeUtils::GetCurTimeUs();
    TSE_LOG_INFO(pstSession->m_poServLog, ("db_process_cost: [processBtime=%lu] [processEtime=%lu] [processCtime=%lu] [seq=%u]", \
                                           pstSession->m_uddwProcessBegTime, \
                                           pstSession->m_uddwProcessEndTime, \
                                           pstSession->m_uddwProcessEndTime - pstSession->m_uddwProcessBegTime, \
                                           pstSession->m_udwSeqNo));
    if(!pstSession->m_poLongConn->SendData(&stTasksGroup))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("SendGlobalHsRequest send data failed! [seq=%u]", pstSession->m_udwSeqNo));
        return -4;
    }

    return 0;
}

TINT32 CBaseProcedure::SendEventRequest(SSession *pstSession, TUINT16 uwReqServiceType)
{
    TUCHAR *pszPack = NULL;
    TUINT32 udwPackLen = 0;
    CDownMgr *pobjDownMgr = CDownMgr::Instance();
    LTasksGroup	stTasksGroup;
    vector<EventReqInfo*>& vecReq = pstSession->m_vecEventReq;
    TUINT32 udwLength = vecReq.size();

    if (udwLength == 0)
    {

        TSE_LOG_DEBUG(pstSession->m_poServLog, ("CBaseProcedure::SendAwsRequest: not event req [seq=%u]", pstSession->m_udwSeqNo));
        return -1;
    }

    pstSession->m_uddwDownRqstTimeBeg = CTimeUtils::GetCurTimeUs();

    // 1. 获取下游——对于一个session来说，每次只需获取一次
    if(pstSession->m_bEventProxyExist == FALSE)
    {
        pstSession->m_pstEventProxyNode = NULL;
        if (S_OK == pobjDownMgr->zk_GetNode(DOWN_NODE_TYPE__EVENT_PROXY, &pstSession->m_pstEventProxyNode))
        {
            pstSession->m_bEventProxyExist = TRUE;
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("CBaseProcedure::SendEventRequest Get EventProx node succ [seq=%u]", pstSession->m_udwSeqNo));
        }
        else
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("CBaseProcedure::SendEventRequest  Get EventProxy node fail [seq=%u]", pstSession->m_udwSeqNo));
            return -2;
        }
    }

    if (pstSession->m_bThemeEventProxyExist == FALSE)
    {
        pstSession->m_pstThemeEventProxyNode = NULL;
        if (S_OK == pobjDownMgr->zk_GetNode(DOWN_NODE_TYPE__THEME_EVENT_PROXY, &pstSession->m_pstThemeEventProxyNode))
        {
            pstSession->m_bThemeEventProxyExist = TRUE;
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("CBaseProcedure::SendEventRequest Get ThemeEventProx node succ [seq=%u]", pstSession->m_udwSeqNo));
        }
        else
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("CBaseProcedure::SendEventRequest  Get ThemeEventProxy node fail [seq=%u]", pstSession->m_udwSeqNo));
            return -2;
        }
    }

    // 2. 封装请求、打包、设置task
    for (TUINT32 udwIdx = 0; udwIdx < udwLength; ++udwIdx)
    {
        EventReqInfo *pstEventReq = vecReq[udwIdx];

        pstSession->m_udwEventRqstType = pstEventReq->m_udwRequestType;

        CBaseProtocolPack* pPack = pstSession->m_ppPackTool[udwIdx];
        pPack->ResetContent();
        pPack->SetServiceType(uwReqServiceType);
        pPack->SetSeq(CGlobalServ::GenerateHsReqSeq());
        pPack->SetKey(EN_GLOBAL_KEY__REQ_SEQ, pstSession->m_udwSeqNo);
        pPack->SetKey(EN_KEY_EVENT_PROXY__REQ_TYPE, pstSession->m_udwEventRqstType);
        pPack->SetKey(EN_KEY_EVENT_PROXY__REQ_JSON, (unsigned char*)pstEventReq->m_sReqContent.c_str(), pstEventReq->m_sReqContent.size());
        pPack->GetPackage(&pszPack, &udwPackLen);

        if (NULL == pstSession->m_pstEventProxyNode->m_stDownHandle.handle
            || NULL == pstSession->m_pstThemeEventProxyNode->m_stDownHandle.handle)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("SendEventRequest:handle [task_id=%u] [handlevalue=0x%p] [seq=%u]", \
                udwIdx, pstSession->m_pstEventProxyNode->m_stDownHandle.handle, pstSession->m_udwSeqNo));
            return -3;
        }
        if (vecReq[udwIdx]->m_udwRequestType != EN_REQUEST_TYPE__UPDATE)
        {
            
            if (pstEventReq->m_udwIsTheme == 0)
            {
                stTasksGroup.m_Tasks[udwIdx].SetConnSession(pstSession->m_pstEventProxyNode->m_stDownHandle);
            }
            else
            {
                stTasksGroup.m_Tasks[udwIdx].SetConnSession(pstSession->m_pstThemeEventProxyNode->m_stDownHandle);
            }
            stTasksGroup.m_Tasks[udwIdx].SetSendData(pszPack, udwPackLen);
            stTasksGroup.m_Tasks[udwIdx].SetNeedResponse(1);
        }
        else
        {
            stTasksGroup.m_Tasks[udwIdx].SetConnSession(pstSession->m_pstEventProxyNode->m_stDownHandle);
            stTasksGroup.m_Tasks[udwIdx].SetSendData(pszPack, udwPackLen);
            stTasksGroup.m_Tasks[udwIdx].SetNeedResponse(1);
            stTasksGroup.m_Tasks[udwLength + udwIdx].SetConnSession(pstSession->m_pstThemeEventProxyNode->m_stDownHandle);
            stTasksGroup.m_Tasks[udwLength + udwIdx].SetSendData(pszPack, udwPackLen);
            stTasksGroup.m_Tasks[udwLength + udwIdx].SetNeedResponse(1);
        }
    }

    // c. 设置task
    stTasksGroup.m_UserData1.ptr = pstSession;
    stTasksGroup.m_UserData2.u32 = pstSession->m_udwSeqNo;//wave@20160104:为解决主程序已处理，但后续有回包返回时的情况
    if (vecReq[0]->m_udwRequestType != EN_REQUEST_TYPE__UPDATE)
    {
        stTasksGroup.SetValidTasks(vecReq.size());
    }
    else
    {
        stTasksGroup.SetValidTasks(vecReq.size() * 2);
    }
    stTasksGroup.m_uTaskTimeOutVal = DOWN_SEARCH_NODE_TIMEOUT_MS;

    pstSession->ResetEventInfo();

    // 3. 发送数据
    pstSession->m_bProcessing = FALSE;
    pstSession->m_uddwProcessEndTime = CTimeUtils::GetCurTimeUs();
    TSE_LOG_INFO(pstSession->m_poServLog, ("event_process_cost: [processBtime=%lu] [processEtime=%lu] [processCtime=%lu] [seq=%u]", \
                                           pstSession->m_uddwProcessBegTime, \
                                           pstSession->m_uddwProcessEndTime, \
                                           pstSession->m_uddwProcessEndTime - pstSession->m_uddwProcessBegTime, \
                                           pstSession->m_udwSeqNo));
    if(!pstSession->m_poLongConn->SendData(&stTasksGroup))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("SendGlobalHsRequest send data failed! [seq=%u]", pstSession->m_udwSeqNo));
        return -4;
    }

    return 0;
}

TINT32 CBaseProcedure::SendCacheRequest(SSession *pstSession, TUINT16 uwReqServiceType)
{
    TUCHAR *pszPack = NULL;
    TUINT32 udwPackLen = 0;
    CDownMgr *pobjDownMgr = CDownMgr::Instance();
    LTasksGroup	stTasksGroup;
    vector<CacheReqInfo*>& vecReq = pstSession->m_vecCacheReq;

    if(vecReq.size() == 0)
    {

        TSE_LOG_DEBUG(pstSession->m_poServLog, ("CBaseProcedure::SendAwsRequest: not cache req [seq=%u]", \
                                                pstSession->m_udwSeqNo));
        return -1;
    }


    // 1. 获取下游——对于一个session来说，每次只需获取一次
    if(pstSession->m_bCacheProxyExist == FALSE)
    {
        pstSession->m_pstCacheProxyNode = NULL;
        if (S_OK == pobjDownMgr->zk_GetNode(DOWN_NODE_TYPE__CACHE_PROXY, &pstSession->m_pstCacheProxyNode))
        {
            pstSession->m_bCacheProxyExist = TRUE;
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("SendCacheRequest: Get CacheProx node succ [seq=%u]", \
                                                    pstSession->m_udwSeqNo));
        }
        else
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("SendCacheRequest: Get CacheProxy node fail [seq=%u]", \
                                                    pstSession->m_udwSeqNo));
            return -2;
        }
    }

    // 2. 封装请求、打包、设置task
    for(TUINT32 udwIdx = 0; udwIdx < vecReq.size(); ++udwIdx)
    {
        srand(time(0)); 
        CacheReqInfo *pstCacheReq = vecReq[udwIdx];
        CBaseProtocolPack* pPack = pstSession->m_ppPackTool[udwIdx];
        pPack->ResetContent();
        pPack->SetServiceType(uwReqServiceType);
        pPack->SetSeq(CGlobalServ::GenerateHsReqSeq());
        pPack->SetKey(EN_CACHE__REQ_SEQ, pstSession->m_udwSeqNo);
        pPack->SetKey(EN_CACHE__NAME, (unsigned char*)pstCacheReq->m_strCacheName.c_str(), pstCacheReq->m_strCacheName.size());
        pPack->SetKey(EN_CACHE__OPERATE, (unsigned char*)pstCacheReq->m_strCacheOperate.c_str(), pstCacheReq->m_strCacheOperate.size());
        pPack->SetKey(EN_CACHE__KEY, pstCacheReq->m_uddwCacheKey);
        pPack->SetKey(EN_CACHE__VALUE_DATA, (unsigned char*)pstCacheReq->m_strCacheValue.c_str(), pstCacheReq->m_strCacheValue.size());
        pPack->SetKey(EN_CACHE__VALUE_VAILDTIME, (TINT32)(CACHE_VAILD_TIME_S + rand() % 300));
        pPack->GetPackage(&pszPack, &udwPackLen);
        
        if(NULL == pstSession->m_pstCacheProxyNode->m_stDownHandle.handle)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("SendCacheRequest: handle [task_id=%u] [handlevalue=0x%p] [seq=%u]", \
                                                    udwIdx, \
                                                    pstSession->m_pstCacheProxyNode->m_stDownHandle.handle, \
                                                    pstSession->m_udwSeqNo));
            return -3;
        }

        stTasksGroup.m_Tasks[udwIdx].SetConnSession(pstSession->m_pstCacheProxyNode->m_stDownHandle);
        stTasksGroup.m_Tasks[udwIdx].SetSendData(pszPack, udwPackLen);
        stTasksGroup.m_Tasks[udwIdx].SetNeedResponse(1);
    }

    // c. 设置task
    stTasksGroup.m_UserData1.ptr = pstSession;
    stTasksGroup.m_UserData2.u32 = pstSession->m_udwSeqNo;//wave@20160104:为解决主程序已处理，但后续有回包返回时的情况

    stTasksGroup.SetValidTasks(vecReq.size());
    stTasksGroup.m_uTaskTimeOutVal = DOWN_SEARCH_NODE_TIMEOUT_MS;

    pstSession->ResetCacheInfo();

    // 3. 发送数据
    pstSession->m_bProcessing = FALSE;
    pstSession->m_uddwProcessEndTime = CTimeUtils::GetCurTimeUs();
    TSE_LOG_INFO(pstSession->m_poServLog, ("cache_process_cost: [processBtime=%lu] [processEtime=%lu] [processCtime=%lu] [seq=%u]", \
                                           pstSession->m_uddwProcessBegTime, \
                                           pstSession->m_uddwProcessEndTime, \
                                           pstSession->m_uddwProcessEndTime - pstSession->m_uddwProcessBegTime, \
                                           pstSession->m_udwSeqNo));
    if(!pstSession->m_poLongConn->SendData(&stTasksGroup))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("SendCacheRequest: send data failed! [seq=%u]", pstSession->m_udwSeqNo));
        return -4;
    }

    return 0;
}

TINT32 CBaseProcedure::SendTranslateRequest(SSession *pstSession, TUINT16 uwReqServiceType)
{
    TUCHAR *pszPack = NULL;
    TUINT32 udwPackLen = 0;
    CDownMgr *pobjDownMgr = CDownMgr::Instance();
    LTasksGroup	stTasksGroup;
    vector<TranslateReqInfo*>& vecReq = pstSession->m_vecTranslateReq;

    if(vecReq.size() == 0)
    {

        TSE_LOG_DEBUG(pstSession->m_poServLog, ("SendTranslateRequest: not translate req [seq=%u]", \
                                                pstSession->m_udwSeqNo));
        return -1;
    }


    // 1. 获取下游——对于一个session来说，每次只需获取一次
    if(pstSession->m_bTranslateProxyExist == FALSE)
    {
        pstSession->m_pstTranslateProxyNode = NULL;
        if (S_OK == pobjDownMgr->zk_GetNode(DOWN_NODE_TYPE__TRANSLATE_PROXY, &pstSession->m_pstTranslateProxyNode))
        {
            pstSession->m_bTranslateProxyExist = TRUE;
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("SendTranslateRequest: Get TranslateProxy node succ [seq=%u]", \
                                                    pstSession->m_udwSeqNo));
        }
        else
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("SendTranslateRequest: Get TranslateProxy node fail [seq=%u]", \
                                                    pstSession->m_udwSeqNo));
            return -2;
        }
    }

    // 2. 封装请求、打包、设置task
    for(TUINT32 udwIdx = 0; udwIdx < vecReq.size(); ++udwIdx)
    {
        srand(time(0)); 
        TranslateReqInfo *pstTranslateReq = vecReq[udwIdx];
        CBaseProtocolPack* pPack = pstSession->m_ppPackTool[udwIdx];
        pPack->ResetContent();
        pPack->SetServiceType(uwReqServiceType);
        pPack->SetSeq(CGlobalServ::GenerateHsReqSeq());
        pPack->SetKey(EN_TRANSLATE__REQ_SEQ, pstSession->m_udwSeqNo);
        pPack->SetKey(EN_TRANSLATE__TYPE, (unsigned char*)pstTranslateReq->m_strTranslateType.c_str(), pstTranslateReq->m_strTranslateType.size());
        pPack->SetKey(EN_TRANSLATE__OPERATE, (unsigned char*)pstTranslateReq->m_strTranslateOperate.c_str(), pstTranslateReq->m_strTranslateOperate.size());
        pPack->SetKey(EN_TRANSLATE__CONTENT, (unsigned char*)pstTranslateReq->m_strTranslateContent.c_str(), pstTranslateReq->m_strTranslateContent.size());
        pPack->GetPackage(&pszPack, &udwPackLen);
        
        if(NULL == pstSession->m_pstTranslateProxyNode->m_stDownHandle.handle)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("SendTranslateRequest: handle [task_id=%u] [handlevalue=0x%p] [seq=%u]", \
                                                    udwIdx, \
                                                    pstSession->m_pstTranslateProxyNode->m_stDownHandle.handle, \
                                                    pstSession->m_udwSeqNo));
            return -3;
        }

        stTasksGroup.m_Tasks[udwIdx].SetConnSession(pstSession->m_pstTranslateProxyNode->m_stDownHandle);
        stTasksGroup.m_Tasks[udwIdx].SetSendData(pszPack, udwPackLen);
        stTasksGroup.m_Tasks[udwIdx].SetNeedResponse(1);
    }

    // c. 设置task
    stTasksGroup.m_UserData1.ptr = pstSession;
    stTasksGroup.m_UserData2.u32 = pstSession->m_udwSeqNo;//wave@20160104:为解决主程序已处理，但后续有回包返回时的情况

    stTasksGroup.SetValidTasks(vecReq.size());
    stTasksGroup.m_uTaskTimeOutVal = DOWN_SEARCH_NODE_TIMEOUT_MS;

    pstSession->ResetTranslateInfo();

    // 3. 发送数据
    pstSession->m_bProcessing = FALSE;
    pstSession->m_uddwProcessEndTime = CTimeUtils::GetCurTimeUs();
    TSE_LOG_INFO(pstSession->m_poServLog, ("translate_process_cost: [processBtime=%lu] [processEtime=%lu] [processCtime=%lu] [seq=%u]", \
                                           pstSession->m_uddwProcessBegTime, \
                                           pstSession->m_uddwProcessEndTime, \
                                           pstSession->m_uddwProcessEndTime - pstSession->m_uddwProcessBegTime, \
                                           pstSession->m_udwSeqNo));
    if(!pstSession->m_poLongConn->SendData(&stTasksGroup))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("SendTranslateRequest: send data failed! [seq=%u]", pstSession->m_udwSeqNo));
        return -4;
    }

    return 0;
}

TINT32 CBaseProcedure::SendDataCenterRequest(SSession *pstSession, TUINT16 uwReqServiceType)
{
    TUCHAR *pszPack = NULL;
    TUINT32 udwPackLen = 0;
    CDownMgr *pobjDownMgr = CDownMgr::Instance();
    LTasksGroup	stTasksGroup;
    vector<DataCenterReqInfo*>& vecReq = pstSession->m_vecDataCenterReq;

    if (vecReq.size() == 0)
    {
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("SendTranslateRequest: not translate req [seq=%u]", \
            pstSession->m_udwSeqNo));
        return -1;
    }


    // 1. 获取下游——对于一个session来说，每次只需获取一次
    if (pstSession->m_bDataCenterProxyExist == FALSE)
    {
        pstSession->m_pstDataCenterProxyNode = NULL;
        if (S_OK == pobjDownMgr->zk_GetNode(DOWN_NODE_TYPE__DATA_CENTER_PROXY, &pstSession->m_pstDataCenterProxyNode))
        {
            pstSession->m_bDataCenterProxyExist = TRUE;
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("SendDataCenterRequest: Get DataCenterProxy node succ [seq=%u]", \
                pstSession->m_udwSeqNo));
        }
        else
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("SendDataCenterRequest: Get DataCenterProxy node fail [seq=%u]", \
                pstSession->m_udwSeqNo));
            return -2;
        }
    }

    // 2. 封装请求、打包、设置task
    for (TUINT32 udwIdx = 0; udwIdx < vecReq.size(); ++udwIdx)
    {
        srand(time(0));
        DataCenterReqInfo *pstDataCenterReq = vecReq[udwIdx];
        CBaseProtocolPack* pPack = pstSession->m_ppPackTool[udwIdx];
        pPack->ResetContent();
        pPack->SetServiceType(uwReqServiceType);
        TUINT32 udwHandleSeq = CGlobalServ::GenerateHsReqSeq();
        pPack->SetSeq(udwHandleSeq);
        pPack->SetKey(EN_DATA_CENTER__REQ_SEQ, pstSession->m_udwSeqNo);
        pPack->SetKey(EN_DATA_CENTER__REQ_TYPE, pstDataCenterReq->m_udwType);
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("SendDataCenterRequest: send data: [handleseq=%u][seq=%u][type=%u] [seq=%u]", \
            udwHandleSeq, pstSession->m_udwSeqNo, pstDataCenterReq->m_udwType, pstSession->m_udwSeqNo));
        pPack->SetKey(EN_DATA_CENTER__REQ_JSON, (unsigned char*)pstDataCenterReq->m_sReqContent.c_str(), pstDataCenterReq->m_sReqContent.size());
        ///打包请求到下游
        pPack->GetPackage(&pszPack, &udwPackLen);

        if (NULL == pstSession->m_pstDataCenterProxyNode->m_stDownHandle.handle)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("SendDataCenterRequest: handle [task_id=%u] [handlevalue=0x%p] [seq=%u]", \
                udwIdx, \
                pstSession->m_pstDataCenterProxyNode->m_stDownHandle.handle, \
                pstSession->m_udwSeqNo));
            return -3;
        }

        stTasksGroup.m_Tasks[udwIdx].SetConnSession(pstSession->m_pstDataCenterProxyNode->m_stDownHandle);
        stTasksGroup.m_Tasks[udwIdx].SetSendData(pszPack, udwPackLen);
        stTasksGroup.m_Tasks[udwIdx].SetNeedResponse(1);
    }

    // c. 设置task
    stTasksGroup.m_UserData1.ptr = pstSession;
    stTasksGroup.m_UserData2.u32 = pstSession->m_udwSeqNo;//wave@20160104:为解决主程序已处理，但后续有回包返回时的情况

    stTasksGroup.SetValidTasks(vecReq.size());
    stTasksGroup.m_uTaskTimeOutVal = DOWN_SEARCH_NODE_TIMEOUT_MS;

    pstSession->ResetDataCenterReq();

    // 3. 发送数据
    pstSession->m_bProcessing = FALSE;
    pstSession->m_uddwProcessEndTime = CTimeUtils::GetCurTimeUs();
    TSE_LOG_INFO(pstSession->m_poServLog, ("data_center_process_cost: [processBtime=%lu] [processEtime=%lu] [processCtime=%lu] [seq=%u]", \
        pstSession->m_uddwProcessBegTime, \
        pstSession->m_uddwProcessEndTime, \
        pstSession->m_uddwProcessEndTime - pstSession->m_uddwProcessBegTime, \
        pstSession->m_udwSeqNo));
    if (!pstSession->m_poLongConn->SendData(&stTasksGroup))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("SendDataCenterRequest: send data failed! [seq=%u]", pstSession->m_udwSeqNo));
        return -4;
    }

    return 0;
}

TINT32 CBaseProcedure::SendMapSvrRequest(SSession *pstSession, TUINT16 uwReqServiceType)
{
    TUCHAR *pszPack = NULL;
    TUINT32 udwPackLen = 0;
    CDownMgr *pobjDownMgr = CDownMgr::Instance();
    LTasksGroup	stTasksGroup;
    vector<MapSvrReqInfo*>& vecReq = pstSession->m_vecMapSvrReq;

    if (vecReq.size() == 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("SendMapSvrRequest: m_vecMapSvrReq.size() == 0"));
        return -1;
    }

    pstSession->m_uddwDownRqstTimeBeg = CTimeUtils::GetCurTimeUs();

    // 1. 获取下游——对于一个session来说，每次只需获取一次
    if (pstSession->m_bMapSvrExist == FALSE)
    {
        pstSession->m_pstMapSvrNode = NULL;
        if (S_OK == pobjDownMgr->zk_GetNode(DOWN_NODE_TYPE__MAP_SVR_PROXY, &pstSession->m_pstMapSvrNode))
        {
            pstSession->m_bMapSvrExist = TRUE;
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("SendMapSvrRequest: Get MapSvr node succ [seq=%u]", pstSession->m_udwSeqNo));
        }
        else
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("SendMapSvrRequest: Get MapSvr node fail [seq=%u]", pstSession->m_udwSeqNo));
            return -21;
        }
        if (NULL == pstSession->m_pstMapSvrNode->m_stDownHandle.handle)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("SendMapSvrRequest:handle [handlevalue=0x%p] [seq=%u]", 
                pstSession->m_pstMapSvrNode->m_stDownHandle.handle, pstSession->m_udwSeqNo));
            return -3;
        }
    }


    // 2. 封装请求、打包、设置task
    for (unsigned int i = 0; i < vecReq.size() && i < MAX_AWS_REQ_TASK_NUM; ++i)
    {
        CBaseProtocolPack* pPack = pstSession->m_ppPackTool[i];
        pPack->ResetContent();
        pPack->SetServiceType(uwReqServiceType);
        pPack->SetSeq(CGlobalServ::GenerateHsReqSeq());
        pPack->SetKey(EN_GLOBAL_KEY__REQ_SEQ, pstSession->m_udwSeqNo);
        pPack->SetKey(EN_GLOBAL_KEY__REQ_TYPE, pstSession->m_udwContentType);
        pPack->SetKey(EN_MAP_SVR_KEY__SVR_ID, vecReq[i]->dwSid);
        pPack->SetKey(EN_MAP_SVR_KEY__OPERATE, (unsigned char*)vecReq[i]->sOperator.c_str(), vecReq[i]->sOperator.size());
        pPack->SetKey(EN_MAP_SVR_KEY__REQ_BUF, (unsigned char*)vecReq[i]->sReqContent.c_str(), vecReq[i]->sReqContent.size());
        pPack->GetPackage(&pszPack, &udwPackLen);

        stTasksGroup.m_Tasks[i].SetConnSession(pstSession->m_pstMapSvrNode->m_stDownHandle);

        stTasksGroup.m_Tasks[i].SetSendData(pszPack, udwPackLen);
        stTasksGroup.m_Tasks[i].SetNeedResponse(1);
    }

    // c. 设置task
    stTasksGroup.m_UserData1.ptr = pstSession;
    stTasksGroup.m_UserData2.u32 = pstSession->m_udwSeqNo;//wave@20160104:为解决主程序已处理，但后续有回包返回时的情况

    stTasksGroup.SetValidTasks(vecReq.size());
    stTasksGroup.m_uTaskTimeOutVal = DOWN_SEARCH_NODE_TIMEOUT_MS;

    TSE_LOG_DEBUG(pstSession->m_poServLog, ("SendMapSvrRequest: [vaildtasksnum=%u] [seq=%u]", 
        vecReq.size(), pstSession->m_udwSeqNo));

    pstSession->ResetAwsInfo();

    // 3. 发送数据    
    pstSession->m_bProcessing = FALSE;
    pstSession->m_uddwProcessEndTime = CTimeUtils::GetCurTimeUs();
    TSE_LOG_INFO(pstSession->m_poServLog, ("map_svr_process_cost: [processBtime=%lu] [processEtime=%lu] [processCtime=%lu] [seq=%u]", \
        pstSession->m_uddwProcessBegTime, \
        pstSession->m_uddwProcessEndTime, \
        pstSession->m_uddwProcessEndTime - pstSession->m_uddwProcessBegTime, \
        pstSession->m_udwSeqNo));
    if (!pstSession->m_poLongConn->SendData(&stTasksGroup))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("SendMapSvrRequest send data failed! [seq=%u]", pstSession->m_udwSeqNo));
        return -4;
    }

    return 0;
}

TINT32 CBaseProcedure::SendIapSvrRequest(SSession *pstSession)
{
    TUCHAR *pszPack = NULL;
    TUINT32 udwPackLen = 0;
    CDownMgr *pobjDownMgr = CDownMgr::Instance();
    LTasksGroup	stTasksGroup;
    vector<IapSvrReqInfo*>& vecReq = pstSession->m_vecIapSvrReq;

    if (vecReq.size() == 0)
    {
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("SendTranslateRequest: not translate req [seq=%u]", \
            pstSession->m_udwSeqNo));
        return -1;
    }


    // 1. 获取下游——对于一个session来说，每次只需获取一次
    if (pstSession->m_bIapSvrExist == FALSE)
    {
        pstSession->m_pstIapSvrNode = NULL;
        const string szPriorityHost = CConfBase::GetString("iap_svr_master_ip");
        if (S_OK == pobjDownMgr->zk_GetNode(DOWN_NODE_TYPE__IAP_SVR, &pstSession->m_pstIapSvrNode, szPriorityHost.c_str()))
        {
            pstSession->m_bIapSvrExist = TRUE;
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("SendIapSvrRequest: Get IapSvr node succ [seq=%u]", \
                pstSession->m_udwSeqNo));
        }
        else
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("SendIapSvrRequest: Get IapSvr node fail [seq=%u]", \
                pstSession->m_udwSeqNo));
            return -2;
        }
    }

    // 2. 封装请求、打包、设置task
    for (TUINT32 udwIdx = 0; udwIdx < vecReq.size(); ++udwIdx)
    {
        IapSvrReqInfo *pstIapSvrReq = vecReq[udwIdx];
        CBaseProtocolPack* pPack = pstSession->m_ppPackTool[udwIdx];
        pPack->ResetContent();
        pPack->SetServiceType(pstIapSvrReq->m_udwType);
        TUINT32 udwHandleSeq = CGlobalServ::GenerateHsReqSeq();
        pPack->SetSeq(udwHandleSeq);
        pPack->SetKey(EN_HU_REQUEST_KEY__REQUEST_SEQ, pstSession->m_udwSeqNo);
        pPack->SetKey(EN_HU_REQUEST_KEY__REQUEST_TYPE, pstIapSvrReq->m_udwType);
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("SendIapSvrRequest: send data: [handleseq=%u][seq=%u][type=%u] [seq=%u]", \
            udwHandleSeq, pstSession->m_udwSeqNo, pstIapSvrReq->m_udwType, pstSession->m_udwSeqNo));
        pPack->SetKey(EN_HU_REQUEST_KEY__REQUEST_JSON, (unsigned char*)pstIapSvrReq->m_sReqContent.c_str(), pstIapSvrReq->m_sReqContent.size());
        //打包请求到下游
        pPack->GetPackage(&pszPack, &udwPackLen);

        if (NULL == pstSession->m_pstIapSvrNode->m_stDownHandle.handle)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("SendIapSvrRequest: handle [task_id=%u] [handlevalue=0x%p] [seq=%u]", \
                udwIdx, \
                pstSession->m_pstIapSvrNode->m_stDownHandle.handle, \
                pstSession->m_udwSeqNo));
            return -3;
        }

        stTasksGroup.m_Tasks[udwIdx].SetConnSession(pstSession->m_pstIapSvrNode->m_stDownHandle);
        stTasksGroup.m_Tasks[udwIdx].SetSendData(pszPack, udwPackLen);
        stTasksGroup.m_Tasks[udwIdx].SetNeedResponse(1);
    }

    // c. 设置task
    stTasksGroup.m_UserData1.ptr = pstSession;
    stTasksGroup.m_UserData2.u32 = pstSession->m_udwSeqNo;//wave@20160104:为解决主程序已处理，但后续有回包返回时的情况

    stTasksGroup.SetValidTasks(vecReq.size());
    stTasksGroup.m_uTaskTimeOutVal = DOWN_SEARCH_NODE_TIMEOUT_MS;

    pstSession->ResetIapSvrReq();

    // 3. 发送数据
    pstSession->m_bProcessing = FALSE;
    pstSession->m_uddwProcessEndTime = CTimeUtils::GetCurTimeUs();
    TSE_LOG_INFO(pstSession->m_poServLog, ("iap_server_process_cost: [processBtime=%lu] [processEtime=%lu] [processCtime=%lu] [seq=%u]", \
        pstSession->m_uddwProcessBegTime, \
        pstSession->m_uddwProcessEndTime, \
        pstSession->m_uddwProcessEndTime - pstSession->m_uddwProcessBegTime, \
        pstSession->m_udwSeqNo));
    if (!pstSession->m_poLongConn->SendData(&stTasksGroup))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("SendIapSvrRequest: send data failed! [seq=%u]", pstSession->m_udwSeqNo));
        return -4;
    }

    return 0;
}

TINT32 CBaseProcedure::SendReportSvrRequest(SSession *pstSession)
{
    TUCHAR *pszPack = NULL;
    TUINT32 udwPackLen = 0;
    CDownMgr *pobjDownMgr = CDownMgr::Instance();
    LTasksGroup	stTasksGroup;
    vector<ReportReqInfo*>& vecReq = pstSession->m_vecReportReq;

    if (vecReq.size() == 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("SendReportSvrRequest: req.size() == 0"));
        return -1;
    }

    pstSession->m_uddwDownRqstTimeBeg = CTimeUtils::GetCurTimeUs();

    // 1. 获取下游——对于一个session来说，每次只需获取一次
    if (pstSession->m_bReportUserSvrNodeExist == FALSE)
    {
        pstSession->m_pstReportUserSvrNode = NULL;
        if (S_OK == pobjDownMgr->zk_GetNode(DOWN_NODE_TYPE__REPORT_USER_SVR, &pstSession->m_pstReportUserSvrNode))
        {
            pstSession->m_bReportUserSvrNodeExist = TRUE;
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("SendReportSvrRequest: Get report node succ [seq=%u]", pstSession->m_udwSeqNo));
        }
        else
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("SendReportSvrRequest: Get report node fail [seq=%u]", pstSession->m_udwSeqNo));
            return -21;
        }
        if (NULL == pstSession->m_pstReportUserSvrNode->m_stDownHandle.handle)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("SendReportSvrRequest:handle [handlevalue=0x%p] [seq=%u]",
                pstSession->m_pstReportUserSvrNode->m_stDownHandle.handle, pstSession->m_udwSeqNo));
            return -3;
        }
    }

    if (pstSession->m_bMailUserSvrNodeExist == FALSE)
    {
        pstSession->m_pstMailUserSvrNode = NULL;
        if (S_OK == pobjDownMgr->zk_GetNode(DOWN_NODE_TYPE__MAIL_USER_SVR, &pstSession->m_pstMailUserSvrNode))
        {
            pstSession->m_bMailUserSvrNodeExist = TRUE;
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("SendReportSvrRequest: Get mail node succ [seq=%u]", pstSession->m_udwSeqNo));
        }
        else
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("SendReportSvrRequest: Get mail node fail [seq=%u]", pstSession->m_udwSeqNo));
            return -21;
        }
        if (NULL == pstSession->m_pstMailUserSvrNode->m_stDownHandle.handle)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("SendReportSvrRequest:handle [handlevalue=0x%p] [seq=%u]",
                pstSession->m_pstMailUserSvrNode->m_stDownHandle.handle, pstSession->m_udwSeqNo));
            return -3;
        }
    }


    // 2. 封装请求、打包、设置task
    for (unsigned int i = 0; i < vecReq.size() && i < MAX_AWS_REQ_TASK_NUM; ++i)
    {
        CBaseProtocolPack* pPack = pstSession->m_ppPackTool[i];
        pPack->ResetContent();
        if (vecReq[i]->udwReqType == 0)
        {
            pPack->SetServiceType(EN_SERVICE_TYPE_REPORT_CENTER_REQ);
        }
        else
        {
            pPack->SetServiceType(EN_SERVICE_TYPE_MAIL_CENTER_REQ);
        }
        pPack->SetSeq(CGlobalServ::GenerateHsReqSeq());
        pPack->SetKey(EN_GLOBAL_KEY__REQ_SEQ, pstSession->m_udwSeqNo);
        pPack->SetKey(EN_KEY_HU2HS__REQ_BUF, (unsigned char*)vecReq[i]->sReqContent.c_str(), vecReq[i]->sReqContent.size());
        pPack->GetPackage(&pszPack, &udwPackLen);
        if (vecReq[i]->udwReqType == 0)
        {
            stTasksGroup.m_Tasks[i].SetConnSession(pstSession->m_pstReportUserSvrNode->m_stDownHandle);
        }
        else
        {
            stTasksGroup.m_Tasks[i].SetConnSession(pstSession->m_pstMailUserSvrNode->m_stDownHandle);
        }

        stTasksGroup.m_Tasks[i].SetSendData(pszPack, udwPackLen);
        stTasksGroup.m_Tasks[i].SetNeedResponse(1);
    }

    // c. 设置task
    stTasksGroup.m_UserData1.ptr = pstSession;
    stTasksGroup.m_UserData2.u32 = pstSession->m_udwSeqNo;//wave@20160104:为解决主程序已处理，但后续有回包返回时的情况

    stTasksGroup.SetValidTasks(vecReq.size());
    stTasksGroup.m_uTaskTimeOutVal = DOWN_SEARCH_NODE_TIMEOUT_MS;

    TSE_LOG_DEBUG(pstSession->m_poServLog, ("SendReportSvrRequest: [vaildtasksnum=%u] [seq=%u]",
        vecReq.size(), pstSession->m_udwSeqNo));

    pstSession->ResetReportInfo();

    // 3. 发送数据    
    pstSession->m_bProcessing = FALSE;
    pstSession->m_uddwProcessEndTime = CTimeUtils::GetCurTimeUs();
    TSE_LOG_INFO(pstSession->m_poServLog, ("report_svr_process_cost: [processBtime=%lu] [processEtime=%lu] [processCtime=%lu] [seq=%u]", \
        pstSession->m_uddwProcessBegTime, \
        pstSession->m_uddwProcessEndTime, \
        pstSession->m_uddwProcessEndTime - pstSession->m_uddwProcessBegTime, \
        pstSession->m_udwSeqNo));
    if (!pstSession->m_poLongConn->SendData(&stTasksGroup))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("SendReportSvrRequest send data failed! [seq=%u]", pstSession->m_udwSeqNo));
        return -4;
    }

    return 0;
}

TINT32 CBaseProcedure::SendPurchaseCheckRequest(SSession *pstSession, TUINT16 uwReqServiceType)
{
    TUCHAR *pszPack = NULL;
    TUINT32 udwPackLen = 0;
    CDownMgr *pobjDownMgr = CDownMgr::Instance();
    LTasksGroup	stTasksGroup;
    
    if (pstSession->m_stPurchaseReq.sReqContent.size() == 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("SendPurchaseCheckRequest: req_content.size() == 0"));
        return -1;
    }

    pstSession->m_uddwDownRqstTimeBeg = CTimeUtils::GetCurTimeUs();

    // 1. 获取下游——对于一个session来说，每次只需获取一次
    if (pstSession->m_bPurchaseCheckNodeExist == FALSE)
    {
        pstSession->m_pstPurchaseCheckNode = NULL;
        if (S_OK == pobjDownMgr->zk_GetNode(DOWN_NODE_TYPE__PURCHAES_CHECK, &pstSession->m_pstPurchaseCheckNode))
        {
            pstSession->m_bPurchaseCheckNodeExist = TRUE;
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("SendPurchaseCheckRequest: Get purchase check node succ [seq=%u]", pstSession->m_udwSeqNo));
        }
        else
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("SendPurchaseCheckRequest: Get purchase check node fail [seq=%u]", pstSession->m_udwSeqNo));
            return -21;
        }
        if (NULL == pstSession->m_pstPurchaseCheckNode->m_stDownHandle.handle)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("SendPurchaseCheckRequest:handle [handlevalue=0x%p] [seq=%u]",
                pstSession->m_pstPurchaseCheckNode->m_stDownHandle.handle, pstSession->m_udwSeqNo));
            return -3;
        }
    }

    // 2. 封装请求、打包、设置task
    CBaseProtocolPack* pPack = pstSession->m_ppPackTool[0];
    pPack->ResetContent();
    pPack->SetServiceType(uwReqServiceType);
    pPack->SetSeq(CGlobalServ::GenerateHsReqSeq());
    pPack->SetKey(EN_GLOBAL_KEY__REQ_SEQ, pstSession->m_udwSeqNo);
    pPack->SetKey(EN_GLOBAL_KEY__REQ_TYPE, pstSession->m_udwContentType);
    pPack->SetKey(EN_GLOBAL_KEY__REQ_BUF, (unsigned char*)pstSession->m_stPurchaseReq.sReqContent.c_str(), pstSession->m_stPurchaseReq.sReqContent.size());
    pPack->GetPackage(&pszPack, &udwPackLen);

    stTasksGroup.m_Tasks[0].SetConnSession(pstSession->m_pstPurchaseCheckNode->m_stDownHandle);

    stTasksGroup.m_Tasks[0].SetSendData(pszPack, udwPackLen);
    stTasksGroup.m_Tasks[0].SetNeedResponse(1);

    // c. 设置task
    stTasksGroup.m_UserData1.ptr = pstSession;
    stTasksGroup.m_UserData2.u32 = pstSession->m_udwSeqNo;//wave@20160104:为解决主程序已处理，但后续有回包返回时的情况

    stTasksGroup.SetValidTasks(1);
    stTasksGroup.m_uTaskTimeOutVal = DOWN_SEARCH_NODE_TIMEOUT_MS;

    // 3. 发送数据    
    pstSession->m_bProcessing = FALSE;

    if (!pstSession->m_poLongConn->SendData(&stTasksGroup))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("SendPurchaseCheckRequest send data failed! [seq=%u]", pstSession->m_udwSeqNo));
        return -4;
    }

    return 0;
}

TINT32 CBaseProcedure::SendRankSvrRequest(SSession *pstSession)
{
    TUCHAR *pszPack = NULL;
    TUINT32 udwPackLen = 0;
    CDownMgr *pobjDownMgr = CDownMgr::Instance();
    LTasksGroup	stTasksGroup;
    vector<RankSvrReqInfo*>& vecReq = pstSession->m_vecRankSvrReq;

    string szReqContent;

    if (vecReq.size() == 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("SendRankSvrRequest: req.size() == 0"));
        return -1;
    }

    pstSession->m_uddwDownRqstTimeBeg = CTimeUtils::GetCurTimeUs();

    // 1. 获取下游——对于一个session来说，每次只需获取一次
    if (pstSession->m_bRankSvrExist == FALSE)
    {
        pstSession->m_pstRankSvrNode = NULL;
        if (S_OK == pobjDownMgr->zk_GetNode(DOWN_NODE_TYPE__RANK_SVR, &pstSession->m_pstRankSvrNode))
        {
            pstSession->m_bRankSvrExist = TRUE;
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("SendRankSvrRequest: Get rank node succ [seq=%u]", pstSession->m_udwSeqNo));
        }
        else
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("SendRankSvrRequest: Get rank node fail [seq=%u]", pstSession->m_udwSeqNo));
            return -21;
        }
        if (NULL == pstSession->m_pstRankSvrNode->m_stDownHandle.handle)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("SendRankSvrRequest:handle [handlevalue=0x%p] [seq=%u]",
                pstSession->m_pstRankSvrNode->m_stDownHandle.handle, pstSession->m_udwSeqNo));
            return -3;
        }
    }

    // 2. 封装请求、打包、设置task
    for (unsigned int i = 0; i < vecReq.size() && i < MAX_AWS_REQ_TASK_NUM; ++i)
    {
        szReqContent.clear();
        vecReq[i]->GenReqString(szReqContent);

        CBaseProtocolPack* pPack = pstSession->m_ppPackTool[i];
        pPack->ResetContent();
        pPack->SetServiceType(EN_SERVICE_TYPE__SVR__RANK_REQ);
        pPack->SetSeq(CGlobalServ::GenerateHsReqSeq());
        pPack->SetKey(EN_GLOBAL_KEY__REQ_SEQ, pstSession->m_udwSeqNo);
        pPack->SetKey(EN_GLOBAL_KEY__REQ_BUF, (unsigned char*)szReqContent.c_str(), szReqContent.size());
        pPack->GetPackage(&pszPack, &udwPackLen);

        stTasksGroup.m_Tasks[i].SetConnSession(pstSession->m_pstRankSvrNode->m_stDownHandle);
        stTasksGroup.m_Tasks[i].SetSendData(pszPack, udwPackLen);
        stTasksGroup.m_Tasks[i].SetNeedResponse(1);
    }

    // c. 设置task
    stTasksGroup.m_UserData1.ptr = pstSession;
    stTasksGroup.m_UserData2.u32 = pstSession->m_udwSeqNo;//wave@20160104:为解决主程序已处理，但后续有回包返回时的情况

    stTasksGroup.SetValidTasks(vecReq.size());
    stTasksGroup.m_uTaskTimeOutVal = DOWN_SEARCH_NODE_TIMEOUT_MS;

    TSE_LOG_DEBUG(pstSession->m_poServLog, ("SendRankSvrRequest: [vaildtasksnum=%u] [seq=%u]",
        vecReq.size(), pstSession->m_udwSeqNo));

    pstSession->ResetRankSvrInfo();

    // 3. 发送数据    
    pstSession->m_bProcessing = FALSE;
    pstSession->m_uddwProcessEndTime = CTimeUtils::GetCurTimeUs();
    TSE_LOG_INFO(pstSession->m_poServLog, ("rank_svr_process_cost: [processBtime=%lu] [processEtime=%lu] [processCtime=%lu] [seq=%u]", 
        pstSession->m_uddwProcessBegTime, 
        pstSession->m_uddwProcessEndTime, 
        pstSession->m_uddwProcessEndTime - pstSession->m_uddwProcessBegTime, 
        pstSession->m_udwSeqNo));
    if (!pstSession->m_poLongConn->SendData(&stTasksGroup))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("SendRankSvrRequest send data failed! [seq=%u]", pstSession->m_udwSeqNo));
        return -4;
    }

    return 0;
}

TINT32 CBaseProcedure::ProcessProcedure_LoginUpdtRequest(SSession *pstSession)
{
    TINT32 dwRetCode = 0;
    dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstSession->m_stUserInfo.m_tbLogin);
    if(dwRetCode != 0)
    {
        return 1;
    }

    // log
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("SendAwsRequest: login info update, aws_req_num=%u [seq=%u]",
        pstSession->m_vecAwsReq.size(), pstSession->m_udwSeqNo));

    // send request
    dwRetCode = SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
    if(dwRetCode < 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
        return -2;
    }

    return 0;
}

TINT32 CBaseProcedure::ProcessProcedure_LoginRequest(SSession* pstSession)
{
    TINT32 dwRetCode = 0;
    pstSession->ResetAwsInfo();

    CAwsRequest::LoginGetByUid(pstSession, pstSession->m_stReqParam.m_udwUserId);

    // send request
    dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
    if(dwRetCode < 0)
    {
        if(pstSession->m_stCommonResInfo.m_dwRetCode == EN_RET_CODE__SUCCESS)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
        }
        return -1;
    }

    return 0;
}

TINT32 CBaseProcedure::ProcessProcedure_LoginResponse(SSession* pstSession)
{
    TINT32 dwRetCode = 0;
    TBOOL bLoginSuccess = FALSE;

    TbLogin* ptbLogin = &pstSession->m_stUserInfo.m_tbLogin;
    ptbLogin->Reset();

    // 1. 获取账号信息
    if(pstSession->m_vecAwsRsp.size() == 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_PACKAGE_ERR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessProcedure_LoginResponse: no valid res [seq=%u]", pstSession->m_udwSeqNo));
        return -1;
    }
    else
    {
        dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[0], ptbLogin);
        if(ptbLogin->m_nUid > 0)
        {
            // update user svr
            if(pstSession->m_stReqParam.m_udwSvrId == (TUINT32)-1)
            {
                pstSession->m_stReqParam.m_udwSvrId = ptbLogin->m_nSid;
            }
            bLoginSuccess = TRUE;
        }
    }

    // 2. 如果未找到相应的账户
    if(bLoginSuccess == FALSE)
    {
        if(pstSession->m_stReqParam.m_ucLoginStatus == EN_LOGIN_STATUS__LOGIN && pstSession->m_stReqParam.m_udwCommandID == EN_CLIENT_REQ_COMMAND__LOGIN_GET)
        {
            if(pstSession->m_stReqParam.m_ucIsNpc)
            {
                pstSession->m_udwCommand = EN_CLIENT_REQ_COMMAND__LOGIN_CREATE;
                pstSession->m_stReqParam.m_ucIsGuideFinish = TRUE;
                TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessProcedure_LoginResponse: npc is not exist, convert to create... [seq=%u]", pstSession->m_udwSeqNo));
            }
            else
            {
                pstSession->m_udwCommand = EN_CLIENT_REQ_COMMAND__LOGIN_FAKE;
                TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessProcedure_LoginResponse: the device has not account, convert to fake... [seq=%u]", pstSession->m_udwSeqNo));
            }
            return 0;
        }
        else if(pstSession->m_stReqParam.m_udwCommandID == EN_CLIENT_REQ_COMMAND__GUIDE_FINISH)
        {
            pstSession->m_stReqParam.m_udwCommandID = EN_CLIENT_REQ_COMMAND__LOGIN_GET;
            pstSession->m_stReqParam.m_ucLoginStatus = EN_LOGIN_STATUS__LOGIN;
            pstSession->m_udwCommand = EN_CLIENT_REQ_COMMAND__LOGIN_CREATE;
            pstSession->m_stReqParam.m_ucIsGuideFinish = TRUE;
            pstSession->m_stReqParam.m_udwUserId = 0;
            //pstReqInfo->m_udwSvrId = (TUINT32)-1;
            TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessProcedure_LoginResponse: guide finish, convert to create account... [seq=%u]", pstSession->m_udwSeqNo));
            return 0;
        }
        else
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessProcedure_LoginResponse: need create account, but req status or cmd error [seq=%u]", pstSession->m_udwSeqNo));
            return -2;
        }
    }

    // 3. 判定合法性
    if(pstSession->m_stReqParam.m_ucLoginStatus == EN_LOGIN_STATUS__USING && pstSession->m_stReqParam.m_udwCommandID != EN_CLIENT_REQ_COMMAND__GUIDE_FINISH)
    {
#ifdef	_PRESS_
        //wavewang@20130330: 压测时，将用户id验证暂时去掉，方便压测
        pstReqInfo->m_udwUserId = tbAccountIndex.Get_Uid();
#else
        if(pstSession->m_stReqParam.m_udwUserId != ptbLogin->m_nUid)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessProcedure_LoginResponse: error user id [seq=%u]", pstSession->m_udwSeqNo));
            return -3;
        }
#endif

        if(pstSession->m_stReqParam.m_udwSvrId != ptbLogin->m_nSid)
        {
            //pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_SID_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessProcedure_LoginResponse: error svr id[%u][%ld] [seq=%u]",
                pstSession->m_stReqParam.m_udwSvrId, ptbLogin->m_nSid, pstSession->m_udwSeqNo));
            //return -4;
            if (pstSession->m_stReqParam.m_udwSvrId != ptbLogin->m_nSid)
            {
                pstSession->m_stReqParam.m_udwSvrId = ptbLogin->m_nSid;
            }
        }
    }
    else
    {
        //// 更新请求中的相关id
        //pstReqInfo->m_udwSeqNo = tbLogin.Get_Seq();
        pstSession->m_stReqParam.m_udwUserId = ptbLogin->m_nUid;
        if(pstSession->m_stReqParam.m_udwSvrId == (TUINT32)-1)
        {
            pstSession->m_stReqParam.m_udwSvrId = ptbLogin->m_nSid;
        }
        if(pstSession->m_stReqParam.m_udwSvrId != ptbLogin->m_nSid)
        {
            pstSession->m_stReqParam.m_udwSvrId = ptbLogin->m_nSid;
        }

        //wave@20160323: 合服相关——无效用户触发合服流程
        TINT32 dwTSid = CGameSvrInfo::GetInstance()->GetTargeSid(ptbLogin->m_nSid);
        if(ptbLogin->m_nSid != dwTSid)
        {
            //修改命令字，进行用户迁移
            //todo
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessProcedure_LoginResponse [uid=%u] is in dead list, raw_sid=%ld, target_sid=%d, wait to process [seq=%u]", \
                pstSession->m_stReqParam.m_udwUserId, ptbLogin->m_nSid, dwTSid, pstSession->m_udwSeqNo));
            pstSession->m_udwCommand = EN_CLIENT_REQ_COMMAND__RECOVERY_MERGE_USER;
            //pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SVR_MERGE_TMP;
            return 0;
        }
    }

    // 4. 黑名单判定
    CGameSvrInfo *pobjGameSvr = CGameSvrInfo::GetInstance();
    if(pobjGameSvr->m_objBlackList.count(pstSession->m_stReqParam.m_udwUserId) > 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessProcedure_LoginResponse [uid=%u] is in black list, [seq=%u]", \
            pstSession->m_stReqParam.m_udwUserId, pstSession->m_udwSeqNo));
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -5;
    }

    // 2. 记录命令处理前的gem
    pstSession->m_ddwGemBegin = pstSession->m_stUserInfo.m_tbLogin.m_nGem;
    return 0;
}
/////////////////////////////////////////////////////////////////////////////////////////////

TINT32 CBaseProcedure::ProcessProcedure_UserDataGetRequest(SSession *pstSession)
{
    TUINT32 udwSvrId = pstSession->m_stReqParam.m_udwSvrId;
    TUINT32 udwUserId = pstSession->m_stReqParam.m_udwUserId;
    if(0 == udwUserId)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }
    pstSession->ResetAwsInfo();

    //获取login 信息
    if(pstSession->m_stUserInfo.m_tbLogin.m_nUid == 0)
    {
        CAwsRequest::LoginGetByUid(pstSession, udwUserId);
    }

    // 获取player信息(player)
    CAwsRequest::UserGetByUid(pstSession, udwUserId);

    // 获取city信息(city)
    CAwsRequest::CityGetByUid(pstSession, udwUserId);

    //action 获取
    CAwsRequest::BufferActionQueryBySuid(pstSession, udwUserId);
    CAwsRequest::AlActionQueryBySuid(pstSession, udwUserId);
    //战争主动action 
    CAwsRequest::MarchActionQueryBySuid(pstSession, udwSvrId, udwUserId);
    CAwsRequest::MarchActionQueryByTuid(pstSession, udwSvrId, udwUserId);

    // 获取stat信息(user_stat)
    CAwsRequest::UserStatGet(pstSession, udwUserId);

    // 获取帐号黑名单(black_account)(获取回来做帐号黑名单的检验)
    if(EN_LOGIN_STATUS__LOGIN == pstSession->m_stReqParam.m_ucLoginStatus
        && pstSession->m_dwClientReqEnType)
    {
        CAwsRequest::BlackAccountGet(pstSession, udwUserId);
        CAwsRequest::BlackAccountGet(pstSession, pstSession->m_stReqParam.m_szDevice);
    }

    // 获取bookmark的信息(bookmark)
    CAwsRequest::BookmarkQuery(pstSession, udwUserId);

    // 获取背包(backpack)
    CAwsRequest::BackPackGet(pstSession, udwUserId);

    //map
    CAwsRequest::MapQueryByUid(pstSession, udwSvrId, udwUserId);

    // quest
    CAwsRequest::QuestGetByUid(pstSession, udwUserId);

    //bounty
    CAwsRequest::BountyGetByUid(pstSession, udwUserId);

    //backpack eqip crystal soul parts material 
    CAwsRequest::EquipQueryByUid(pstSession,udwUserId);

    //get help_bubble
    CAwsRequest::GetGlobalId(pstSession, EN_GLOBAL_PARAM__HELP_BUBBLE_TIME);

    CAwsRequest::AltarHistoryQuery(pstSession, udwUserId);

    if((pstSession->m_stReqParam.m_ucLoginStatus == EN_LOGIN_STATUS__LOGIN && strcmp(pstSession->m_stReqParam.m_szCommand, "login_get") == 0) ||
        pstSession->m_stReqParam.m_udwCommandID == EN_CLIENT_REQ_COMMAND__SVR_GET)
    {
        CAwsRequest::SvrListInfoQuery(pstSession);
    }

    CAwsRequest::BlackListQuery(pstSession, udwUserId);

    CAwsRequest::TaskGet(pstSession, udwUserId);

    CAwsRequest::RandomRewardQuery(pstSession, udwUserId);

    CAwsRequest::DataOutputGet(pstSession, udwUserId);

    CAwsRequest::IdolQuery(pstSession, udwSvrId);

    CAwsRequest::ThroneGet(pstSession, udwSvrId);

    CAwsRequest::TitleQuery(pstSession, udwSvrId);
    //decoration
    CAwsRequest::DecorationGet(pstSession, udwUserId);
    //lord image
    CAwsRequest::LordImageGet(pstSession, udwUserId);

    TINT32 dwRetCode = SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
    if(dwRetCode != 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
        return -2;
    }
    return 0;
}

TINT32 CBaseProcedure::ProcessProcedure_AllianceDataGetRequest(SSession *pstSession)
{
    TUINT32 udwSvrId = pstSession->m_stReqParam.m_udwSvrId;
    TUINT32 udwUserId = pstSession->m_stReqParam.m_udwUserId;
    TUINT32 udwAllianceId = pstSession->m_stReqParam.m_udwAllianceId;
    pstSession->ResetAwsInfo();

    if(udwAllianceId && pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos)
    {
        // 获取用户所在联盟的信息
        CAwsRequest::AllianceGetByAid(pstSession, udwAllianceId);

        // 获取用户帮助过的联盟任务
        CAwsRequest::AlHelpHistoryQuery(pstSession, udwUserId);

        // 获取能帮助的联盟action_list(其实就是通过sal拉取)(top100)
        CAwsRequest::CanHelpTaskQuery(pstSession, udwAllianceId, MAX_USER_AL_ACTION_NUM / 4);

        CAwsRequest::MarchActionQueryBySal(pstSession, udwSvrId, udwAllianceId);
        CAwsRequest::MarchActionQueryByTal(pstSession, udwSvrId, udwAllianceId);

        // 当用户是盟主或者副盟主的时候，获取请求加入联盟的用户数（设置了验证加入联盟的条件）
        if(pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos >= EN_ALLIANCE_POS__VICE_CHANCELOR)
        {
            CAwsRequest::AlRequestConutQuery(pstSession, udwAllianceId);
        }

        // 获取联盟全部人assistance的记录(包括所有的type:申请资源/分享资源/申请援兵(目前只用到前两种type))
        CAwsRequest::AlAssistQuery(pstSession, pstSession->m_stReqParam.m_udwAllianceId);

        // 获取联盟的外交政策
        CAwsRequest::DiplomacyQuery(pstSession, udwAllianceId);

        // 获取用户未读的联盟公告数量
        CAwsRequest::WallMsgCntQuery(pstSession, udwSvrId, udwAllianceId);
        CAwsRequest::TopWallMsgCntQuery(pstSession, udwSvrId, udwAllianceId);

        // 获取联盟里全部人发布的公告
        CAwsRequest::AllianceWallQuery(pstSession, udwAllianceId);

        // 获取联盟商店购买记录
        CAwsRequest::AlConsumeHistoryQuery(pstSession, udwAllianceId, MAX_AL_CONSUME_NUM);

        // 获取自己的联盟成员信息
        CAwsRequest::AlMemberGet(pstSession, udwAllianceId, udwUserId);

        // 联盟礼物
        TINT64 ddwTimePoint = CTimeUtils::GetCurTimeUs() - AL_IAP_GIFT_EXPIRE_TIME * 2;
        ddwTimePoint = (ddwTimePoint > (pstSession->m_stUserInfo.m_tbPlayer.m_nAl_time*1000*1000)) ? ddwTimePoint : pstSession->m_stUserInfo.m_tbPlayer.m_nAl_time*1000*1000;
        TbAl_gift tbAl_gift;
        tbAl_gift.Set_Aid(udwAllianceId);
        tbAl_gift.Set_Id(ddwTimePoint);
        CompareDesc comp_desc;
        comp_desc.dwCompareType = COMPARE_TYPE_GE;
        CAwsRequest::Query(pstSession, &tbAl_gift, ETbALGIFT_OPEN_TYPE_PRIMARY, comp_desc, TRUE, TRUE, FALSE, MAX_AL_IAP_GIFT_NUM);

        TbAl_gift_reward tbAlGiftReward;
        tbAlGiftReward.Set_Uid(pstSession->m_stUserInfo.m_tbPlayer.m_nUid);
        tbAlGiftReward.Set_Gid(ddwTimePoint);
        CAwsRequest::Query(pstSession, &tbAlGiftReward, ETbALGIFTREWARD_OPEN_TYPE_PRIMARY, comp_desc, true, true, false, MAX_AL_IAP_GIFT_NUM);

        TbClear_al_gift tbClearAlGift;
        tbClearAlGift.Set_Uid(pstSession->m_stUserInfo.m_tbPlayer.m_nUid);
        CAwsRequest::GetItem(pstSession, &tbClearAlGift, ETbCLEARALGIFT_OPEN_TYPE_PRIMARY);

        CAwsRequest::TipsQuery(pstSession, -1 * static_cast<TINT64>(udwAllianceId), MAX_PLAYER_TIPS_NUM - 1);

        //CAwsRequest::AlEventTipsQuery(pstSession, static_cast<TINT64>(udwAllianceId), MAX_EVENT_TIPS_NUM - 1);

        if (pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos == EN_ALLIANCE_POS__CHANCELLOR)
        {
            TbSvr_al tbSvrAl;
            tbSvrAl.Reset();

            tbSvrAl.Set_Sid(udwSvrId);
            tbSvrAl.Set_Alid(udwAllianceId);
            CAwsRequest::GetItem(pstSession, &tbSvrAl, ETbSVR_AL_OPEN_TYPE_PRIMARY);
        }

        if (udwAllianceId == pstSession->m_tbThrone.m_nAlid)
        {
            CAwsRequest::AttackThroneMarchQuery(pstSession, pstSession->m_tbThrone.m_nSid, pstSession->m_tbThrone.m_nPos);
        }
    }

    if(EN_CLIENT_REQ_COMMAND__ALLIANCE_JOIN == pstSession->m_stReqParam.m_udwCommandID)
    {
        pstSession->m_stReqParam.m_udwAllianceId = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
        udwAllianceId = pstSession->m_stReqParam.m_udwAllianceId;
    }

    /************ 这部分是tips的 ************/
    // 由于需要先拉user_stat表,所有才放这里
    // 获取用户的tips
    CAwsRequest::TipsQuery(pstSession, udwUserId, MAX_PLAYER_TIPS_NUM - 1);
    if(!pstSession->m_stUserInfo.m_tbPlayer.m_sUin.empty())
    {
        CAwsRequest::UserGetByPlayerName(pstSession, pstSession->m_stUserInfo.m_tbPlayer.m_sUin);
    }

    CAwsRequest::EventTipsQuery(pstSession, udwUserId, MAX_EVENT_TIPS_NUM - 1);

    /************ 这部分是mail和report的 ************/
    // todo: 正常的联盟数据获取流程是不存在mail和report的流程,并且没有联盟数据拉取时,直接跳转process层
    CompareDesc stCompareInfo;
    stCompareInfo.dwCompareType = COMPARE_TYPE_GT;

    //拉取广播，由于要先拉user stat，所以放在这里
    CAwsRequest::BroadcastQuery(pstSession);

    CAwsRequest::RewardWindowQuery(pstSession, udwUserId, pstSession->m_stUserInfo.m_tbUserStat.m_nReward_window_time - MAX_DELAY_TIME_REWARD_WINDOW * 1000);

    TINT32 dwRetCode = SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
    if(dwRetCode != 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
        return -3;
    }

    return 0;
}

TINT32 CBaseProcedure::ProcessProcedure_UserDataGetResponse(SSession *pstSession)
{
    TUINT32 udwIdx = 0;
    TINT32 dwRet;
    AwsRspInfo *pstAwsRspInfo = NULL;

    SUserInfo *pstUserInfo = &pstSession->m_stUserInfo;
    TbLogin& tbLogin = pstUserInfo->m_tbLogin;
    TbPlayer& tbPlayer = pstUserInfo->m_tbPlayer;
    TbUser_stat& tbUser_stat = pstUserInfo->m_tbUserStat;
    TbCity& tbCity = pstUserInfo->m_stCityInfo.m_stTblData;
    TbLord_image& tbLord_image = pstUserInfo->m_tbLordImage;
    TbDecoration& tbDecoration = pstUserInfo->m_tbDecoration;
    TbBackpack* ptbBackpack = &pstUserInfo->m_tbBackpack;
    ptbBackpack->Reset();
    TbBlack_account tbBlack_accout;
    TbParam& tbParam = pstSession->m_tbTmpGlobalParam;

    // 重置各关键值
    pstUserInfo->m_udwActionNum = 0;
    pstUserInfo->m_udwSelfAlActionNum = 0;
    pstUserInfo->m_udwMarchNum = 0;
    pstUserInfo->m_udwPassiveMarchNum = 0;
    pstUserInfo->m_udwDiplomacyNum = 0;
    pstUserInfo->m_udwAllianceReqCurFindNum = 0;
    pstUserInfo->m_udwAllianceCanAssistNum = 0;

    for(udwIdx = 0; udwIdx < pstSession->m_vecAwsRsp.size(); ++udwIdx)
    {
        pstAwsRspInfo = pstSession->m_vecAwsRsp[udwIdx];
        string strTableRawName = CCommonFunc::GetTableRawName(pstAwsRspInfo->sTableName);
        if(strTableRawName == EN_AWS_TABLE_LOGIN)
        {
            CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &tbLogin);
            continue;
        }
        if(strTableRawName == EN_AWS_TABLE_PLAYER)
        {
            CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &tbPlayer);
            continue;
        }
        if(strTableRawName == EN_AWS_TABLE_CITY)
        {
            CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &tbCity);
            continue;
        }
        if(strTableRawName == EN_AWS_TABLE_ACTION)
        {
            dwRet = CAwsResponse::OnQueryRsp(*pstAwsRspInfo, pstUserInfo->m_atbAction, sizeof(TbAction), MAX_USER_ACTION_NUM);
            if(dwRet >= 0)
            {
                pstUserInfo->m_udwActionNum = dwRet;
            }
            continue;
        }
        if(strTableRawName == EN_AWS_TABLE_ALLIANCE_ACTION)
        {
            dwRet = CAwsResponse::OnQueryRsp(*pstAwsRspInfo, pstUserInfo->m_atbSelfAlAction, sizeof(TbAlliance_action), MAX_USER_AL_ACTION_NUM);
            if(dwRet >= 0)
            {
                pstUserInfo->m_udwSelfAlActionNum = dwRet;
            }
            continue;
        }
        if(strTableRawName == EN_AWS_TABLE_MARCH_ACTION)
        {
            switch(pstAwsRspInfo->udwIdxNo)
            {
            case ETbMARCH_OPEN_TYPE_PRIMARY:
                dwRet = CAwsResponse::OnQueryRsp(*pstAwsRspInfo, pstUserInfo->m_atbMarch, sizeof(TbMarch_action), MAX_USER_MARCH_NUM);
                if(dwRet >= 0)
                {
                    pstUserInfo->m_udwMarchNum = dwRet;
                }
                break;
            case ETbMARCH_OPEN_TYPE_GLB_TUID:
                dwRet = CAwsResponse::OnQueryRsp(*pstAwsRspInfo, pstUserInfo->m_atbPassiveMarch, sizeof(TbMarch_action), MAX_USER_MARCH_NUM);
                if(dwRet >= 0)
                {
                    pstUserInfo->m_udwPassiveMarchNum = dwRet;
                }
                break;
            default:
                break;
            }
            continue;
        }
        if(strTableRawName == EN_AWS_TABLE_USER_STAT)
        {
            CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &tbUser_stat);
            continue;
        }
        if (strTableRawName == EN_AWS_TABLE_LORD_IMAGE)
        {
            CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &tbLord_image);
            continue;
        }
        if (strTableRawName == EN_AWS_TABLE_DECORATION)
        {
            CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &tbDecoration);
            continue;
        }
        if(strTableRawName == EN_AWS_TABLE_BLACK_ACCOUNT)
        {
            dwRet = CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &tbBlack_accout);
            if(dwRet > 0) //命中黑名单
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__BLACK_ACCOUNT;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessUserInfoGetResponse: black account!! : uid[%ld], did[%s], key[%s]",
                    tbLogin.m_nUid, pstSession->m_stReqParam.m_szDevice, tbBlack_accout.m_sKey.c_str()));
                return -1;
            }
            continue;
        }
        if(strTableRawName == EN_AWS_TABLE_BOOKMARK)
        {
            dwRet = CAwsResponse::OnQueryRsp(*pstAwsRspInfo, pstUserInfo->m_atbBookmark, sizeof(TbBookmark), MAX_BOOKMARK_NUM);
            if(dwRet >= 0)
            {
                pstUserInfo->m_udwBookmarkNum = dwRet;
            }
            continue;
        }
        if(strTableRawName == EN_AWS_TABLE_BACKPACK)
        {
            CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, ptbBackpack);
            continue;
        }
        if(strTableRawName == EN_AWS_TABLE_MAP)
        {
            switch(pstAwsRspInfo->udwIdxNo)
            {
            case ETbMAP_OPEN_TYPE_GLB_UID:
                dwRet = CAwsResponse::OnQueryRsp(*pstAwsRspInfo, pstUserInfo->m_atbWild, sizeof(TbMap), MAX_WILD_RETURN_NUM);
                if(dwRet >= 0)
                {
                    pstUserInfo->m_udwWildNum = dwRet;
                }
                break;
            case ETbMAP_OPEN_TYPE_PRIMARY:
                dwRet = CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &pstSession->m_atbTmpThroneMap[pstSession->m_udwTmpThroneMapNum]);
                if(dwRet >= 0)
                {
                    pstSession->m_udwTmpThroneMapNum += dwRet;
                }
                break;
            default:
                break;
            }
            continue;
        }
        if(strTableRawName == EN_AWS_TABLE_QUEST)
        {
            CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &pstUserInfo->m_tbQuest);

            continue;
        }
        if(strTableRawName == EN_AWS_TABLE_BOUNTY)
        {
            CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &pstUserInfo->m_tbBounty);

            continue;
        }
        if(strTableRawName == EN_AWS_TABLE_EQUIP)
        {
            dwRet = CAwsResponse::OnQueryRsp(*pstAwsRspInfo, pstUserInfo->m_atbEquip, sizeof(TbEquip), MAX_USER_EQUIP_NUM);
            if(dwRet >= 0)
            {
                pstUserInfo->m_udwEquipNum = dwRet;
            }
            continue;
        }
        if(strTableRawName == EN_AWS_TABLE_ALTAR_HISTORY)
        {
            dwRet = CAwsResponse::OnQueryRsp(*pstAwsRspInfo, pstSession->m_atbTmpAltarHistory, sizeof(TbAltar_history), MAX_RALLY_HISTORY);
            if(dwRet >= 0)
            {
                pstSession->m_udwTmpAltarHistoryNum = dwRet;
            }
            continue;
        }
        if(strTableRawName == EN_AWS_TABLE_SVR_STAT)
        {
            dwRet = CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &pstSession->m_atbTmpSvrStat[pstSession->m_udwTmpSvrStatNum]);
            if(dwRet >= 0)
            {
                pstSession->m_udwTmpSvrStatNum += dwRet;
            }
            continue;
        }
        if (strTableRawName == EN_AWS_TABLE_BLACKLIST)
        {
            dwRet = CAwsResponse::OnQueryRsp(*pstAwsRspInfo, pstSession->m_stUserInfo.m_atbBlackList, sizeof(TbBlacklist), MAX_BLACKLIST_NUM);
            if (dwRet >= 0)
            {
                pstSession->m_stUserInfo.m_udwBlackListNum = dwRet;
            }
            continue;
        }
        if (strTableRawName == EN_AWS_TABLE_PARAM)
        {
            dwRet = CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &tbParam);
            continue;
        }

        if(strTableRawName == EN_AWS_TABLE_TASK)
        {
            dwRet = CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &pstSession->m_stUserInfo.m_tbTask);
            continue;
        }

        if (strTableRawName == EN_AWS_TABLE_RANDOM_REWARD)
        {
            dwRet = CAwsResponse::OnQueryRsp(*pstAwsRspInfo, pstSession->m_stUserInfo.m_atbRandomReward, sizeof(TbRandom_reward), MAX_RANDOM_REWARD_NUM);
            if (dwRet >= 0)
            {
                pstSession->m_stUserInfo.m_udwRandomRewardNum = dwRet;
            }
            continue;
        }

        if(strTableRawName == EN_AWS_TABLE_DATA_OUTPUT)
        {
            dwRet = CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &pstSession->m_stUserInfo.m_tbDataOutput);
            pstSession->m_dwOutputCmpType = EN_OUTPUT_CMP_TYPE__CMP;
            continue;
        }

        if (strTableRawName == EN_AWS_TABLE_IDOL)
        {
            dwRet = CAwsResponse::OnQueryRsp(*pstAwsRspInfo, pstSession->m_atbIdol, sizeof(TbIdol), MAX_IDOL_NUM);
            if (dwRet >= 0)
            {
                pstSession->m_udwIdolNum = dwRet;
            }
            continue;
        }

        if (strTableRawName == EN_AWS_TABLE_THRONE)
        {
            CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &pstSession->m_tbThrone);
            pstSession->m_ddwRawThroneAid = pstSession->m_tbThrone.m_nAlid;
            continue;
        }

        if (strTableRawName == EN_AWS_TABLE_TITLE)
        {
            dwRet = CAwsResponse::OnQueryRsp(*pstAwsRspInfo, pstSession->m_stTitleInfoList.atbTitle, sizeof(TbTitle), MAX_TITLEINFO_NUM_IN_ONE_SERVER);
            if (dwRet >= 0)
            {
                pstSession->m_stTitleInfoList.udwNum = dwRet;
            }
            continue;
        }
    }

    //wave@20161130: for bug check
    CQuestLogic::CheckPlayerTimeQuestValid(&pstSession->m_stUserInfo, "user_data_response");

    TSE_LOG_DEBUG(pstSession->m_poServLog, ("LoginGetResponse: get global key. [val=%ld] [seq=%u]", tbParam.m_nVal, pstSession->m_udwSeqNo))

    // 这些操作必须在获取完用户数据之后就执行,不然会导致联盟数据的拉取问题
    // 主要是userstat表和player表的aid
    // 拉取完数据之后必须要有数据的校验和修正(修正的情况会发生在用户登录的时候)
    // todo: 如果想统一的数据拉取流程,那么最起码需要把uid和aid在帐号拉取过程中就需要拿到,但目前现在貌似也没啥问题
    // 这些内容更符合帐号流程中做

    // 3. 判定合法性
    if(pstSession->m_stReqParam.m_ucLoginStatus == EN_LOGIN_STATUS__USING && pstSession->m_stReqParam.m_udwCommandID != EN_CLIENT_REQ_COMMAND__GUIDE_FINISH)
    {
        if(!CToolBase::IsOpCommand(pstSession->m_stReqParam.m_szCommand) && pstSession->m_stReqParam.m_bNeedLoginCheck == TRUE)
        {
            if((TINT64)pstSession->m_stReqParam.m_uddwDeviceId != tbLogin.m_nDid)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__HAS_LOG_IN;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("LoginGetResponse: error device id [seq=%u]", pstSession->m_udwSeqNo));
                return -2;
            }
        }
    }
    else
    {
        // 更新请求中的相关id
        //pstSession->m_stReqParam.m_udwSeqNo = tbLogin.Get_Seq();
        pstSession->m_stReqParam.m_udwUserId = tbLogin.Get_Uid();
        if(pstSession->m_stReqParam.m_udwSvrId == (TUINT32)-1)
        {
            pstSession->m_stReqParam.m_udwSvrId = tbLogin.m_nSid;
        }

        if((TINT64)pstSession->m_stReqParam.m_uddwDeviceId != tbLogin.Get_Did())
        {
            TSE_LOG_INFO(pstSession->m_poServLog, ("change_device: svr[%u],user[%u],old_did[%ld],new_did[%lu] [seq=%u]", \
                pstSession->m_stReqParam.m_udwSvrId, pstSession->m_stReqParam.m_udwUserId, tbLogin.Get_Did(), pstSession->m_stReqParam.m_uddwDeviceId, pstSession->m_stReqParam.m_udwSeqNo));
        }
    }
    // 4. 黑名单判定
    if(pstSession->m_dwClientReqEnType)
    {
        CGameSvrInfo *pobjGameSvr = CGameSvrInfo::GetInstance();
        if(pobjGameSvr->m_pobjMasterBlackAccount->count(CCommonFunc::NumToString(pstSession->m_stReqParam.m_udwUserId)) > 0)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("uid[%u] is in black list, [seq=%u]",
                pstSession->m_stReqParam.m_udwUserId, pstSession->m_udwSeqNo));
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__BLACK_ACCOUNT;
            return -3;
        }

        if(pobjGameSvr->m_pobjMasterBlackAccount->count(pstSession->m_stReqParam.m_szDevice) > 0)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("did[%s] is in black list, [seq=%u]",
                pstSession->m_stReqParam.m_szDevice, pstSession->m_udwSeqNo));
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__BLACK_ACCOUNT;
            return -3;
        }

        if(pobjGameSvr->m_pobjMasterBlackAccount->count(pstSession->m_stReqParam.m_szIdfa) > 0)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("idfa[%s] is in black list, [seq=%u]",
                pstSession->m_stReqParam.m_szIdfa, pstSession->m_udwSeqNo));
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__BLACK_ACCOUNT;
            return -3;
        }
    }

    // 5. 记录命令处理前的gem
    pstSession->m_ddwGemBegin = pstSession->m_stUserInfo.m_tbLogin.m_nGem;

    if((0 == ptbBackpack->m_nUid) && (0 != tbPlayer.m_nUid))
    {
        TUINT32 udwEquipGridNum = CGameInfo::GetInstance()->m_oJsonRoot["game_init_data"]["r"]["a5"].asUInt();

        TSE_LOG_ERROR(pstSession->m_poServLog, ("[Equipment]ProcessUserInfoGetResponse: uid[%ld],player[%ld],city_pos[%ld], Ready to crate back_pack[seq=%u]", 
            tbLogin.m_nUid, tbPlayer.m_nUid, tbCity.m_nPos, pstSession->m_udwSeqNo));
        ptbBackpack->Reset();
        ptbBackpack->Set_Uid(tbPlayer.m_nUid);
        tbUser_stat.Set_Equip_gride(udwEquipGridNum);
    }

    if((0 == pstUserInfo->m_tbUserStat.m_nUid) && (0 != tbPlayer.m_nUid))
    {
        pstUserInfo->m_tbUserStat.Set_Uid(tbPlayer.m_nUid);
        
    }

    if((0 == pstUserInfo->m_tbBounty.m_nUid) && (0 != tbPlayer.m_nUid))
    {
        pstUserInfo->m_tbBounty.Set_Uid(tbPlayer.m_nUid);
        CBountyLogic::GenBountyInfo(pstUserInfo, &pstUserInfo->m_stCityInfo);
    }

    if ((0 == pstUserInfo->m_tbLordImage.m_nUid) && (0 != tbPlayer.m_nUid))
    {
        pstUserInfo->m_tbLordImage.Set_Uid(tbPlayer.m_nUid);
    }
    if ((0 == pstUserInfo->m_tbDecoration.m_nUid) && (0 != tbPlayer.m_nUid))
    {
        pstUserInfo->m_tbDecoration.Set_Uid(tbPlayer.m_nUid);
    }
    //1.被删除的死用户数据恢复(leyton add@20140125)
    if(tbPlayer.m_nDead_flag > 0)
    {
        if(pstSession->m_stReqParam.m_ucLoginStatus == EN_LOGIN_STATUS__LOGIN && pstSession->m_stReqParam.m_udwCommandID == EN_CLIENT_REQ_COMMAND__LOGIN_GET)
        {
            TSE_LOG_WARN(pstSession->m_poServLog, ("ProcessUserInfoGetResponse: dead user come back, go to recover user data... [seq=%u]", pstSession->m_udwSeqNo));
            // 转入用户信息恢复流程
            pstSession->m_udwCommandStep = EN_PROCEDURE__INIT;
            pstSession->m_udwCommand = EN_CLIENT_REQ_COMMAND__USER_INFO_RECOVER;
            return 0;
            //pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__USER_INFO_INCOMPLETE;
            //TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessUserInfoGetResponse: dead user !!! [seq=%u]", pstSession->m_udwSeqNo));
            //return -4;
        }
        else if (EN_OP != CClientCmd::GetCmdType(pstSession->m_stReqParam.m_udwCommandID))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__USER_INFO_INCOMPLETE;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessUserInfoGetResponse: unexpected dead user !!! [seq=%u]", pstSession->m_udwSeqNo));
            return -4;
        }
    }

    // 2. 检查数据完整性
    if(pstSession->m_stReqParam.m_udwCommandID == EN_CLIENT_REQ_COMMAND__LOGIN_GET && pstSession->m_stReqParam.m_ucLoginStatus == EN_LOGIN_STATUS__LOGIN)
    {
        if(tbPlayer.m_nUid == 0 && pstUserInfo->m_stCityInfo.m_stTblData.m_nPos == 0)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessUserInfoGetResponse: user info incomplete: uid[%ld],player[%ld],city_pos[%ld] [seq=%u]",
                tbLogin.m_nUid, tbPlayer.m_nUid, pstUserInfo->m_stCityInfo.m_stTblData.m_nPos, pstSession->m_udwSeqNo));

            // 转入用户信息创建流程
            pstSession->m_udwCommandStep = EN_PROCEDURE__INIT;
            pstSession->m_udwCommand = EN_CLIENT_REQ_COMMAND__USER_INFO_CREATE;
            return 0;
        }
        else if(tbPlayer.m_nUid == 0)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessUserInfoGetResponse: user info incomplete: uid[%ld],stat_uid[%ld],player[%ld],city_pos[%ld] [seq=%u]",
                tbLogin.m_nUid, tbUser_stat.m_nUid, tbPlayer.m_nUid, pstUserInfo->m_stCityInfo.m_stTblData.m_nPos, pstSession->m_udwSeqNo));
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessUserInfoGetResponse: CreatePlayerInfo [seq=%u]", pstSession->m_udwSeqNo));

            // create player info
            CAwsRequest::CreatePlayerInfo(pstSession, pstUserInfo, pstSession->m_stReqParam.m_udwSvrId, 0);

            tbPlayer.Set_Cid(pstUserInfo->m_stCityInfo.m_stTblData.m_nPos);

        }
        else if (pstUserInfo->m_stCityInfo.m_stTblData.m_nPos == 0)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessUserInfoGetResponse: user info incomplete: uid[%ld],player[%ld],city_pos[%ld] [seq=%u]",
                tbLogin.m_nUid, tbPlayer.m_nUid, pstUserInfo->m_stCityInfo.m_stTblData.m_nPos, pstSession->m_udwSeqNo));

            TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessUserInfoGetResponse: convert to command[ADD_CITY] [seq=%u]", pstSession->m_udwSeqNo));

            // reset player city info
            tbPlayer.Set_Cid(0);

            // 转入用户数据创建流程
            pstSession->m_udwCommandStep = EN_PROCEDURE__INIT;
            pstSession->m_udwCommand = EN_CLIENT_REQ_COMMAND__USER_INFO_CREATE;

            return 0;
        }
        else
        {
            if (pstUserInfo->m_stCityInfo.m_stTblData.m_nPos != tbPlayer.m_nCid)
            {
                tbPlayer.Set_Cid(pstUserInfo->m_stCityInfo.m_stTblData.m_nPos);
            }
        }
    }
    else
    {
        if (tbPlayer.m_nUid == 0 || pstUserInfo->m_stCityInfo.m_stTblData.m_nPos == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__USER_INFO_INCOMPLETE;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessUserInfoGetResponse: user info incomplete: uid[%ld],player[%ld],city_pos[%ld] [seq=%u]",
                tbLogin.m_nUid, tbPlayer.m_nUid, pstUserInfo->m_stCityInfo.m_stTblData.m_nPos, pstSession->m_udwSeqNo));
            return -5;
        }
    }

    //  设置初始城市
    if(pstSession->m_stReqParam.m_ucLoginStatus == EN_LOGIN_STATUS__LOGIN ||
        pstSession->m_stReqParam.m_udwCityId == 0)
    {
        pstSession->m_stReqParam.m_udwCityId = tbPlayer.m_nCid;
    }

    SCityInfo *pstCity = &pstUserInfo->m_stCityInfo;

    if (CCityBase::GetBuildingLevelById(&pstCity->m_stTblData, 29) > 0)
    {
        pstSession->m_stRawCityInfo.dwSid = pstUserInfo->m_tbPlayer.m_nSid;
        pstSession->m_stRawCityInfo.dwUid = pstUserInfo->m_tbPlayer.m_nUid;
        pstSession->m_stRawCityInfo.dwX = pstCity->m_stTblData.m_nPos / MAP_X_Y_POS_COMPUTE_OFFSET;
        pstSession->m_stRawCityInfo.dwY = pstCity->m_stTblData.m_nPos % MAP_X_Y_POS_COMPUTE_OFFSET;
        pstSession->m_stRawCityInfo.sCityName = pstCity->m_stTblData.m_sName;
    }

    // 修正请求中的联盟id
    pstSession->m_stReqParam.m_udwAllianceId = tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET;

    if(CCommonBase::IsActionDoingOnUser(pstUserInfo)
        && (pstSession->m_udwCommand == EN_CLIENT_REQ_COMMAND__RANDOM_MOVE_CITY
        || pstSession->m_udwCommand == EN_CLIENT_REQ_COMMAND__MOVE_CITY))
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ACTION_DOING_ON_YOU;
        return -7;
    }

    return 0;
}

TINT32 CBaseProcedure::ProcessProcedure_AllianceDataGetResponse(SSession *pstSession)
{
    TINT32 dwRet = 0;

    AwsRspInfo *pstAwsRspInfo = NULL;

    SUserInfo *pstUserInfo = &pstSession->m_stUserInfo;
    TbPlayer &tbPlayer = pstUserInfo->m_tbPlayer;

//    TbReport_user astReportUser[10];
//    TbMail_user astMailUser[30];
//    TINT32 dwNewReportNum = 0;
//    TINT32 dwNewMailNum = 0;

    pstUserInfo->m_udwAlAssistAllNum = 0;
    pstUserInfo->m_udwAlConsumeNum = 0;

    //  response
    for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecAwsRsp.size(); udwIdx++)
    {
        pstAwsRspInfo = pstSession->m_vecAwsRsp[udwIdx];
        string strTableRawName = CCommonFunc::GetTableRawName(pstAwsRspInfo->sTableName);
        if(strTableRawName == EN_AWS_TABLE_AL_HELP)
        {
            CAwsResponse::OnQueryRsp(*pstAwsRspInfo, pstUserInfo->m_atbAl_help, sizeof(TbAl_help), MAX_AL_HELP_LIST_NUM);
            continue;
        }
        if(strTableRawName == EN_AWS_TABLE_ALLIANCE_ACTION)
        {
            TbAlliance_action atbTmpAlAction[MAX_USER_AL_ACTION_NUM];
            dwRet = CAwsResponse::OnQueryRsp(*pstAwsRspInfo, atbTmpAlAction, sizeof(TbAlliance_action), MAX_USER_AL_ACTION_NUM - pstUserInfo->m_udwSelfAlActionNum);
            if(dwRet >= 0)
            {
                std::set<TINT64> setAlActionId;
                for(TUINT32 udwIdx = 0; udwIdx < pstUserInfo->m_udwSelfAlActionNum; ++udwIdx)
                {
                    setAlActionId.insert(pstUserInfo->m_atbSelfAlAction[udwIdx].m_nId);
                }
                for(TINT32 dwIdx = 0; dwIdx < dwRet; ++dwIdx)
                {
                    if(setAlActionId.count(atbTmpAlAction[dwIdx].m_nId) > 0)
                    {
                        continue;
                    }
                    pstUserInfo->m_atbSelfAlAction[pstUserInfo->m_udwSelfAlActionNum++] = atbTmpAlAction[dwIdx];
                }
            }
            continue;
        }
        if(strTableRawName == EN_AWS_TABLE_MARCH_ACTION)
        {
            TbMarch_action atbTmpMarch[MAX_USER_MARCH_NUM];
            switch(pstAwsRspInfo->udwIdxNo)
            {
            case ETbMARCH_OPEN_TYPE_GLB_SAL:
                dwRet = CAwsResponse::OnQueryRsp(*pstAwsRspInfo, atbTmpMarch, sizeof(TbMarch_action), MAX_USER_MARCH_NUM);
                if(dwRet >= 0)
                {
                    std::set<TINT64> setMarchId;
                    for(TUINT32 udwIdx = 0; udwIdx < pstUserInfo->m_udwMarchNum; ++udwIdx)
                    {
                        setMarchId.insert(pstUserInfo->m_atbMarch[udwIdx].m_nId);
                    }
                    for(TINT32 dwIdx = 0; dwIdx < dwRet; ++dwIdx)
                    {
                        if(pstUserInfo->m_udwMarchNum >= MAX_USER_MARCH_NUM)
                        {
                            break;
                        }
                        if(setMarchId.count(atbTmpMarch[dwIdx].m_nId) > 0)
                        {
                            continue;
                        }
                        pstUserInfo->m_atbMarch[pstUserInfo->m_udwMarchNum++] = atbTmpMarch[dwIdx];
                    }
                }
                break;
            case ETbMARCH_OPEN_TYPE_GLB_TAL:
                dwRet = CAwsResponse::OnQueryRsp(*pstAwsRspInfo, atbTmpMarch, sizeof(TbMarch_action), MAX_USER_MARCH_NUM);
                if(dwRet >= 0)
                {
                    std::set<TINT64> setPassiveMarchId;
                    for(TUINT32 udwIdx = 0; udwIdx < pstUserInfo->m_udwPassiveMarchNum; ++udwIdx)
                    {
                        setPassiveMarchId.insert(pstUserInfo->m_atbPassiveMarch[udwIdx].m_nId);
                    }
                    for(TINT32 dwIdx = 0; dwIdx < dwRet; ++dwIdx)
                    {
                        if(pstUserInfo->m_udwPassiveMarchNum >= MAX_USER_MARCH_NUM)
                        {
                            break;
                        }
                        if(setPassiveMarchId.count(atbTmpMarch[dwIdx].m_nId) > 0)
                        {
                            continue;
                        }
                        pstUserInfo->m_atbPassiveMarch[pstUserInfo->m_udwPassiveMarchNum++] = atbTmpMarch[dwIdx];
                    }
                }
                break;
            default:
                break;
            }
            continue;
        }
        if(strTableRawName == EN_AWS_TABLE_ALLIANCE)
        {
            dwRet = CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &pstUserInfo->m_tbAlliance);
            continue;
        }
        if(strTableRawName == EN_AWS_TABLE_AL_MEMBER)
        {
            dwRet = CAwsResponse::OnQueryCountRsp(*pstAwsRspInfo);
            if(dwRet > 0)
            {
                pstUserInfo->m_udwAllianceReqCurFindNum = dwRet;
            }
            continue;
        }
        if(strTableRawName == EN_AWS_TABLE_AL_ASSIST)
        {
            dwRet = CAwsResponse::OnQueryRsp(*pstAwsRspInfo, pstUserInfo->m_atbAlAssistAll, sizeof(TbAl_assist), MAX_AL_ASSIST_NUM - 1);
            if(dwRet > 0)
            {
                pstUserInfo->m_udwAlAssistAllNum = dwRet;
            }
            continue;
        }
        if(strTableRawName == EN_AWS_TABLE_AL_WALL)
        {
            switch(pstAwsRspInfo->udwIdxNo)
            {
            case ETbALWALL_OPEN_TYPE_COUNT:
                dwRet = CAwsResponse::OnQueryCountRsp(*pstAwsRspInfo);
                if(dwRet > 0)
                {
                    pstUserInfo->m_udwNewAlWallMsgNum += dwRet;
                }
                break;
            case ETbALWALL_OPEN_TYPE_PRIMARY:
                dwRet = CAwsResponse::OnQueryRsp(*pstAwsRspInfo, pstUserInfo->m_atbWall, sizeof(TbAl_wall), MAX_WALL_MSG_NUM);
                if(dwRet > 0)
                {
                    pstUserInfo->m_udwWallNum = dwRet;
                }
                break;
            default:
                break;
            }
            continue;
        }
        if(strTableRawName == EN_AWS_TABLE_AL_GIFT)
        {
            dwRet = CAwsResponse::OnQueryRsp(*pstAwsRspInfo, pstUserInfo->m_stAlGifts.m_atbGifts, sizeof(TbAl_gift), MAX_AL_IAP_GIFT_NUM);
            if(dwRet > 0)
            {
                pstUserInfo->m_stAlGifts.m_dwGiftNum = dwRet;
            }
            continue;
        }
        if(strTableRawName == EN_AWS_TABLE_TIPS)
        {
            dwRet = CAwsResponse::OnQueryRsp(*pstAwsRspInfo, pstUserInfo->m_atbTips + pstUserInfo->m_udwTipsNum, sizeof(TbTips), MAX_PLAYER_TIPS_NUM);
            if(dwRet > 0)
            {
                pstSession->m_stUserInfo.m_udwTipsNum += dwRet;
            }
            continue;
        }
        if(strTableRawName == EN_AWS_TABLE_EVENT_TIPS)
        {
            dwRet = CAwsResponse::OnQueryRsp(*pstAwsRspInfo, pstUserInfo->m_atbEventTips + pstUserInfo->m_udwEventTipsNum, sizeof(TbEvent_tips), MAX_EVENT_TIPS_NUM);
            if(dwRet > 0)
            {
                pstSession->m_stUserInfo.m_udwEventTipsNum += dwRet;
            }
            continue;
        }
        if(strTableRawName == EN_AWS_TABLE_AL_STORE_CONSUME)
        {
            dwRet = CAwsResponse::OnQueryRsp(*pstAwsRspInfo, pstUserInfo->m_atbAl_store_consume, sizeof(TbAl_store_consume), MAX_AL_CONSUME_NUM);
            if(dwRet > 0)
            {
                pstUserInfo->m_udwAlConsumeNum = dwRet;
            }
            continue;
        }
        if(strTableRawName == EN_AWS_TABLE_AL_MEMBER)
        {
            dwRet = CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &pstUserInfo->m_tbSelfAlmember);
            continue;
        }
        if(strTableRawName == EN_AWS_TABLE_UNIQUE_NAME)
        {
            dwRet = CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &pstUserInfo->m_tbSelfName);
            continue;
        }
//         if(strTableRawName == EN_AWS_TABLE_MAIL_USER)
//         {
//             dwRet = CAwsResponse::OnQueryRsp(*pstAwsRspInfo, &astMailUser[dwNewMailNum], sizeof(TbMail_user), MAX_MAIL_RES_NUM - dwNewMailNum);
//             if(dwRet > 0)
//             {
//                 dwNewMailNum += dwRet;
//             }
//             continue;
//         }
//         if(strTableRawName == EN_AWS_TABLE_MAIL_OPERATE)
//         {
//             dwRet = CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &pstUserInfo->m_tbMailOperate);
//             continue;
//         }
//         if(strTableRawName == EN_AWS_TABLE_REPORT_USER)
//         {
//             dwRet = CAwsResponse::OnQueryRsp(*pstAwsRspInfo, &astReportUser[dwNewReportNum], sizeof(TbReport_user), MAX_MAIL_RES_NUM - dwNewReportNum);
//             if(dwRet > 0)
//             {
//                 dwNewReportNum += dwRet;
//             }
//             continue;
//         }
        if(strTableRawName == EN_AWS_TABLE_DIPLOMACY)
        {
            dwRet = CAwsResponse::OnQueryRsp(*pstAwsRspInfo, pstSession->m_stUserInfo.m_atbDiplomacy, sizeof(TbDiplomacy), MAX_DIPLOMACY_NUM);
            if(dwRet > 0)
            {
                pstSession->m_stUserInfo.m_udwDiplomacyNum = dwRet;
            }
            continue;
        }
        if(strTableRawName == EN_AWS_TABLE_AL_GIFT_REWARD)
        {
            dwRet = CAwsResponse::OnQueryRsp(*pstAwsRspInfo, pstUserInfo->m_atbAlGiftReward, 
                sizeof(TbAl_gift_reward), MAX_AL_IAP_GIFT_NUM_SVR);
            if(dwRet > 0)
            {
                pstUserInfo->m_udwAlGiftRewardNum = dwRet;
            }
            continue;
        }
        if(strTableRawName == EN_AWS_TABLE_BROADCAST)
        {
            dwRet = CAwsResponse::OnQueryRsp(*pstAwsRspInfo, &pstUserInfo->m_atbBroadcast[pstUserInfo->m_udwBroadcastNum], sizeof(TbBroadcast), MAX_BROADCAST_NUM_TOTAL - pstUserInfo->m_udwBroadcastNum);
            if(dwRet > 0)
            {
                pstUserInfo->m_udwBroadcastNum += dwRet;
            }
            continue;
        }
        if (strTableRawName == EN_AWS_TABLE_SVR_AL)
        {
            CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &pstUserInfo->m_tbSvrAl);
            continue;
        }
        if (strTableRawName == EN_AWS_TABLE_REWARD_WINDOW)
        {
            dwRet = CAwsResponse::OnQueryRsp(*pstAwsRspInfo, pstUserInfo->m_atbRewardWindow, sizeof(TbReward_window), MAX_REWARD_WINDOW_NUM);
            if (dwRet > 0)
            {
                pstUserInfo->m_dwRewardWindowNum = dwRet;
            }
            continue;
        }
        if (strTableRawName == EN_AWS_TABLE_CLEAR_AL_GIFT)
        {
            CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &pstUserInfo->m_tbClearAlGift);
            continue;
        }
    }

//     if(0 == pstUserInfo->m_tbMailOperate.m_nUid)
//     {
//         pstUserInfo->m_tbMailOperate.Set_Uid(pstUserInfo->m_tbPlayer.m_nUid);
//     }

    // mail:updt user stat
//     TINT64 ddwMaxMailId = 0;
//     TINT32 dwNewUnreadMailNum = 0;
//     pstSession->m_bNeedMailMusic = FALSE;
//     for(TINT32 dwIdx = 0; dwIdx < dwNewMailNum; dwIdx++)
//     {
//         if(ddwMaxMailId < astMailUser[dwIdx].m_nMid)
//         {
//             ddwMaxMailId = astMailUser[dwIdx].m_nMid;
//         }
// 
//         if(astMailUser[dwIdx].m_nIs_single_svr == TRUE)
//         {
//             if(astMailUser[dwIdx].m_nSid != pstUserInfo->m_tbPlayer.m_nSid)
//             {
//                 continue;
//             }
//             if(astMailUser[dwIdx].m_nTime < pstUserInfo->m_tbPlayer.m_nSvr_change_time)
//             {
//                 continue;
//             }
//         }
// 
//         if(!astMailUser[dwIdx].m_sPlatform.empty())
//         {
//             if(astMailUser[dwIdx].m_sPlatform != "all")
//             {
//                 if(strcmp(astMailUser[dwIdx].m_sPlatform.c_str(), pstSession->m_stReqParam.m_szPlatForm) != 0)
//                 {
//                     continue;
//                 }
//             }
//         }
// 
//         if(!astMailUser[dwIdx].m_sVersion.empty())
//         {
//             if(strcmp(astMailUser[dwIdx].m_sVersion.c_str(), pstSession->m_stReqParam.m_szVs) != 0)
//             {
//                 continue;
//             }
//         }
// 
//         if(astMailUser[dwIdx].m_nMid > pstUserInfo->m_tbUserStat.m_nNewest_mailid)
//         {
//             dwNewUnreadMailNum++;
//         }
// 
//         if(astMailUser[dwIdx].m_nSuid != pstUserInfo->m_tbPlayer.m_nUid)
//         {
//             pstSession->m_bNeedMailMusic = TRUE;
//         }
//     }
// 
//     if(ddwMaxMailId > pstSession->m_stUserInfo.m_tbUserStat.m_nNewest_mailid)
//     {
//         TSE_LOG_DEBUG(pstSession->m_poServLog, ("Newest Mail id:[%ld] [return_mailid=%ld] [unread_mail=%ld] [seq=%u]",
//             ddwMaxMailId,
//             pstSession->m_stUserInfo.m_tbUserStat.m_nReturn_mailid,
//             pstSession->m_stUserInfo.m_tbUserStat.m_nUnread_mail,
//             pstSession->m_udwSeqNo));
//         pstSession->m_stUserInfo.m_tbUserStat.Set_Newest_mailid(ddwMaxMailId);
//     }
//     if(dwNewUnreadMailNum > 0)
//     {
//         pstSession->m_stUserInfo.m_tbUserStat.Set_Unread_mail(pstSession->m_stUserInfo.m_tbUserStat.m_nUnread_mail + dwNewUnreadMailNum);
//         TSE_LOG_DEBUG(pstSession->m_poServLog, ("unread mail num: [%ld]", pstSession->m_stUserInfo.m_tbUserStat.m_nUnread_mail));
//     }

    // report : updt user stat
//     TINT64 ddwMaxReportId = 0;
//     for(TINT32 dwIdx = 0; dwIdx < dwNewReportNum; dwIdx++)
//     {
//         if(ddwMaxReportId < astReportUser[dwIdx].m_nRid)
//         {
//             ddwMaxReportId = astReportUser[dwIdx].m_nRid;
//         }
//     }
//     if(ddwMaxReportId > pstSession->m_stUserInfo.m_tbUserStat.m_nNewest_reportid)
//     {
//         pstSession->m_stUserInfo.m_tbUserStat.Set_Newest_reportid(ddwMaxReportId);
//         if(dwNewReportNum > 0)
//         {
//             pstSession->m_stUserInfo.m_tbUserStat.Set_Unread_report(pstSession->m_stUserInfo.m_tbUserStat.m_nUnread_report + dwNewReportNum);
//         }
//     }

    //纠正位置
    for(int i = MAX_AL_HELP_LIST_NUM - 1; i >= 0; --i)
    {
        TbAl_help& tbAl_help = pstUserInfo->m_atbAl_help[i];
        if(tbAl_help.m_nIdx > i)//放到正确的位置
        {
            pstUserInfo->m_atbAl_help[tbAl_help.m_nIdx] = tbAl_help;
        }
    }

    if(tbPlayer.m_nLoy_time / AL_FUND_GET_INTERVAL != CTimeUtils::GetUnixTime() / AL_FUND_GET_INTERVAL)
    {
        if(tbPlayer.m_nLoy_itv)
        {
            tbPlayer.Set_Loy_itv(0);
        }
    }

    //广播逆序拉取的  需要正序一下...
    for (unsigned int i = 0; i < (pstUserInfo->m_udwBroadcastNum / 2); i++)
    {
        TbBroadcast tbTmpBroadCast;
        tbTmpBroadCast = pstUserInfo->m_atbBroadcast[i];
        pstUserInfo->m_atbBroadcast[i] = pstUserInfo->m_atbBroadcast[pstUserInfo->m_udwBroadcastNum - i - 1];
        pstUserInfo->m_atbBroadcast[pstUserInfo->m_udwBroadcastNum - i - 1] = tbTmpBroadCast;
    }

    return 0;
}

TINT32 CBaseProcedure::ProcessProcedure_DbDataGetRequest(SSession *pstSession)
{
    pstSession->ResetDbInfo();

    //SUserInfo* pstUser = &pstSession->m_stUserInfo;
    //SCityInfo *pstCity = &pstUser->m_stCityInfo;

    //wave@20160414:屏蔽推荐用户和trade功能
    /*
    if(pstUser->m_tbPlayer.m_nAlpos >= EN_ALLIANCE_POS__VICE_CHANCELOR
    && pstUser->m_tbPlayer.m_nAlid > 0)
    {
        CDbRequest::SelectRecommendCount(pstSession, pstUser->m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET);
        CDbRequest::SelectRecommendTime(pstSession);
    }
    */

    //if (pstCity)
    //{
    //    if ((pstUser->m_tbPlayer.m_nAge == 3 && pstUser->m_tbTrade.m_nRefresh_time == 0)
    //        || (CTimeUtils::GetUnixTime() >= pstUser->m_tbTrade.m_nRefresh_time && pstUser->m_tbTrade.m_nRefresh_time != 0))
    //    {
    //        TINT32 dwDistance = 0;
    //        TINT32 dwMaxMarchTime = 0;

    //        dwMaxMarchTime = CCommonBase::GetGameBasicVal(EN_GAME_BASIC_TRADE_REFRESH_INTERVAL) / 6/*市场20级时的最大生成数量:同时最大队列数*/;
    //        dwMaxMarchTime -= CCommonBase::GetGameBasicVal(EN_GAME_BASIC_TRADE_STAYING_TIME);
    //        dwMaxMarchTime /= 2;
    //        dwDistance = 1.0 * dwMaxMarchTime / 6000 * CCommonBase::GetGameBasicVal(EN_GAME_BASIC_TRADE_SPEED);

    //        TINT32 dwX1 = 1, dwX2 = 800, dwY1 = 1, dwY2 = 800;
    //        TINT32 dwPos = pstCity->m_stTblData.m_nPos;
    //        if (dwPos / 1000 - dwDistance > dwX1)
    //        {
    //            dwX1 = dwPos / 1000 - dwDistance;
    //        }
    //        if (dwPos / 1000 + dwDistance < dwX2)
    //        {
    //            dwX2 = dwPos / 1000 + dwDistance;
    //        }
    //        if (dwPos % 1000 - dwDistance > dwY1)
    //        {
    //            dwY1 = dwPos % 1000 - dwDistance;
    //        }
    //        if (dwPos % 1000 + dwDistance < dwY2)
    //        {
    //            dwY2 = dwPos % 1000 + dwDistance;
    //        }

    //        CDbRequest::SelectTradeCityInfoInRandom(pstSession, pstSession->m_stReqParam.m_udwSvrId, MAX_TRADE_NUM);
    //        CDbRequest::SelectTradeCityInfoInRange(pstSession, pstSession->m_stReqParam.m_udwSvrId, dwPos / 1000, dwPos % 1000, dwX1, dwX2, dwY1, dwY2, MAX_TRADE_NUM);
    //    }
    //}

    if (pstSession->m_vecDbReq.size() > 0)
    {
        TINT32 dwRetCode = CBaseProcedure::SendDbRequest(pstSession, EN_SERVICE_TYPE_QUERY_MYSQL_REQ);
        if (dwRetCode != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            return -1;
        }
        return 0;
    }
    
    return -2;
}

TINT32 CBaseProcedure::ProcessProcedure_DbDataGetResponse(SSession *pstSession)
{
    pstSession->m_udwRecommendNum = 0;
    for(unsigned int i = 0; i < pstSession->m_vecDbRsp.size(); ++i)
    {
        DbRspInfo& rspInfo = *pstSession->m_vecDbRsp[i];
        if(rspInfo.sTableName == "player_recommend")
        {
            TINT32 dwRetCode = CDbResponse::OnCountResponse(rspInfo);
            if(dwRetCode < 0)
            {
                TSE_LOG_ERROR(pstSession->m_poServLog, ("OnSelectResponse failed [seq=%u]", pstSession->m_udwSeqNo));
            }
            else
            {
                pstSession->m_udwRecommendNum = dwRetCode;
            }
            continue;
        }
        if(rspInfo.sTableName == "compute_time")
        {
            TINT32 dwRetCode = CDbResponse::OnSelectResponse(rspInfo, &pstSession->m_stRecommendTime, 1);
            if(dwRetCode < 0)
            {
                TSE_LOG_ERROR(pstSession->m_poServLog, ("OnSelectResponse failed [seq=%u]", pstSession->m_udwSeqNo));
            }
            continue;
        }
        if (rspInfo.sTableName == "trade_city_list")
        {
            if (rspInfo.sOperatorName == "select_in_range")
            {
                TINT32 dwRetCode = CDbResponse::OnSelectResponse(rspInfo, pstSession->m_astTradeCityInfoInRange, MAX_TRADE_NUM);
                if (dwRetCode < 0)
                {
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("OnSelectResponse failed [seq=%u]", pstSession->m_udwSeqNo));
                }
                else
                {
                    pstSession->m_udwTradeInRangeNum = dwRetCode;
                }
            }
            if (rspInfo.sOperatorName == "select_in_random")
            {
                TINT32 dwRetCode = CDbResponse::OnSelectResponse(rspInfo, pstSession->m_astTradeCityInfoInRandom, MAX_TRADE_NUM);
                if (dwRetCode < 0)
                {
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("OnSelectResponse failed [seq=%u]", pstSession->m_udwSeqNo));
                }
                else
                {
                    pstSession->m_udwTradeInRandomNum = dwRetCode;
                }
            }
            continue;
        }
    }
    return 0;
}

TINT32 CBaseProcedure::ProcessProcedure_UserAndAllianceDataUpdtRequest(SSession *pstSession)
{
    TINT32 dwRetCode = 0;
    unsigned int udwSvrId = pstSession->m_stReqParam.m_udwSvrId;
    SUserInfo *pstUserInfo = &pstSession->m_stUserInfo;
    TUINT32 dwIdx = 0;

    pstSession->ResetAwsReq();

    // 不区分新建和更新,统一使用UpdateItem命令
    // todo: 把更新的数据以log的形式打印出来
    TSE_LOG_INFO(pstSession->m_poServLog, ("TableUpdate: begin to update user and alliance data [seq=%u]",
        pstSession->m_udwSeqNo));

    /********************************************************************************************/
    // player表数据
    dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_tbPlayer);
    if(0 == dwRetCode)
    {
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: player need to update [uid=%ld] [seq=%u]",
            pstUserInfo->m_tbPlayer.m_nUid, pstSession->m_udwSeqNo));
    }

    /********************************************************************************************/
    // 创建账号时玩家名字
    if(pstUserInfo->m_ucPlayerFlag == EN_TABLE_UPDT_FLAG__NEW)
    {
        TbUnique_name tbUniqueName;
        tbUniqueName.Set_Name(CToolBase::ToLower(pstUserInfo->m_tbPlayer.m_sUin));
        tbUniqueName.Set_Type(EN_PLAYER_NAME);
        tbUniqueName.Set_Exist(1);
        tbUniqueName.Set_Id(pstUserInfo->m_tbPlayer.m_nUid);
        Json::Value tmpProfile;
        CCommJson::GenUniqueNameInfo(&pstSession->m_stUserInfo.m_tbPlayer, tmpProfile);
        tbUniqueName.Set_Profile(tmpProfile);

        dwRetCode = CAwsRequest::UpdateItem(pstSession, &tbUniqueName);
        if(0 == dwRetCode)
        {
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: player name need to put [uid=%ld][name=%s] [seq=%u]",
                pstUserInfo->m_tbPlayer.m_nUid, pstUserInfo->m_tbPlayer.m_sUin.c_str(), pstSession->m_udwSeqNo));
        }
    }
    else if(pstUserInfo->m_tbSelfName.m_nId > 0 && pstUserInfo->m_tbSelfName.m_nType == EN_PLAYER_NAME)
    {
        dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_tbSelfName);
        if(0 == dwRetCode)
        {
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: self unique name need to update [uid=%ld] [seq=%u]",
                pstUserInfo->m_tbSelfName.m_nId, pstSession->m_udwSeqNo));
        }
    }

    /********************************************************************************************/
    // city表数据
    dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_stCityInfo.m_stTblData);
    if (0 == dwRetCode)
    {
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: city need to update [cityid=%ld] [seq=%u]",
            pstUserInfo->m_stCityInfo.m_stTblData.m_nPos, pstSession->m_udwSeqNo));
    }

    /********************************************************************************************/
    // map表, 不支持删除
    for(dwIdx = 0; dwIdx < pstUserInfo->m_udwWildNum; dwIdx++)
    {
        if(pstUserInfo->m_aucWildFlag[dwIdx] == EN_TABLE_UPDT_FLAG__CHANGE)
        {
            dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_atbWild[dwIdx]);
            if(dwRetCode == 0)
            {
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: map need to update[pos=%ld][seq=%u]",
                    pstUserInfo->m_atbWild[dwIdx].m_nId, pstSession->m_udwSeqNo));

                //wave@push_data
                CPushDataProcess::AddPushData_Wild(&pstSession->m_objAuPushDataNode.m_objPushDataMap, &pstUserInfo->m_atbWild[dwIdx]);
            }            
        }
    }

    /********************************************************************************************/
    // action表数据
    // 个人action
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: [activeactionnum=%ld] [seq=%u]",
        pstUserInfo->m_udwActionNum, pstSession->m_udwSeqNo));
    for(dwIdx = 0; dwIdx < pstUserInfo->m_udwActionNum; dwIdx++)
    {
        if(pstUserInfo->m_aucActionFlag[dwIdx] > EN_TABLE_UPDT_FLAG__UNCHANGE)
        {
            if(EN_TABLE_UPDT_FLAG__DEL == pstUserInfo->m_aucActionFlag[dwIdx])
            {
                CAwsRequest::DeleteItem(pstSession, &pstUserInfo->m_atbAction[dwIdx]);
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: active action need to delete [actionid=%ld] [seq=%u]",
                    pstUserInfo->m_atbAction[dwIdx].m_nId, pstSession->m_udwSeqNo));
            }
            else if(EN_TABLE_UPDT_FLAG__NEW == pstUserInfo->m_aucActionFlag[dwIdx])
            {
                CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_atbAction[dwIdx]);
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: active action need to put [actionid=%ld] [seq=%u]",
                    pstUserInfo->m_atbAction[dwIdx].m_nId, pstSession->m_udwSeqNo));
            }
            else if(EN_TABLE_UPDT_FLAG__CHANGE == pstUserInfo->m_aucActionFlag[dwIdx])
            {
                ExpectedItem expect_item;
                ExpectedDesc expect_desc;
                expect_item.SetVal(TbACTION_FIELD_ID, true, pstUserInfo->m_atbAction[dwIdx].m_nId); //加expect防止边界条件(action已经结束又被新建)
                expect_desc.clear();
                expect_desc.push_back(expect_item);

                dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_atbAction[dwIdx], expect_desc);
                if(0 == dwRetCode)
                {
                    TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: active action need to update [actionid=%ld] [seq=%u]",
                        pstUserInfo->m_atbAction[dwIdx].m_nId, pstSession->m_udwSeqNo));
                }
            }
        }
    }

    // 主动联盟action
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: [alactionnum=%ld] [seq=%u]",
        pstUserInfo->m_udwSelfAlActionNum, pstSession->m_udwSeqNo));
    for(dwIdx = 0; dwIdx < pstUserInfo->m_udwSelfAlActionNum; dwIdx++)
    {
        if(EN_TABLE_UPDT_FLAG__DEL == pstUserInfo->m_aucSelfAlActionFlag[dwIdx])
        {
            CAwsRequest::DeleteItem(pstSession, &pstUserInfo->m_atbSelfAlAction[dwIdx]);
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: active action need to delete [actionid=%ld] [seq=%u]",
                pstUserInfo->m_atbSelfAlAction[dwIdx].m_nId, pstSession->m_udwSeqNo));
        }
        else if(EN_TABLE_UPDT_FLAG__NEW == pstUserInfo->m_aucSelfAlActionFlag[dwIdx])
        {
            CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_atbSelfAlAction[dwIdx]);
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: active action need to put [actionid=%ld] [seq=%u]",
                pstUserInfo->m_atbSelfAlAction[dwIdx].m_nId, pstSession->m_udwSeqNo));
        }
        else if(EN_TABLE_UPDT_FLAG__CHANGE == pstUserInfo->m_aucSelfAlActionFlag[dwIdx])
        {
            ExpectedItem expect_item;
            ExpectedDesc expect_desc;
            expect_item.SetVal(TbACTION_FIELD_ID, true, pstUserInfo->m_atbSelfAlAction[dwIdx].m_nId); //加expect防止边界条件(action已经结束又被新建)
            expect_desc.clear();
            expect_desc.push_back(expect_item);

            dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_atbSelfAlAction[dwIdx], expect_desc);
            if(0 == dwRetCode)
            {
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: active al action need to update [actionid=%ld] [seq=%u]",
                    pstUserInfo->m_atbSelfAlAction[dwIdx].m_nId, pstSession->m_udwSeqNo));
            }
        }
    }

    // 主动march action
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: [march_num=%ld] [seq=%u]",
        pstUserInfo->m_udwMarchNum, pstSession->m_udwSeqNo));
    for(dwIdx = 0; dwIdx < pstUserInfo->m_udwMarchNum; dwIdx++)
    {
        if(EN_TABLE_UPDT_FLAG__DEL == pstUserInfo->m_aucMarchFlag[dwIdx])
        {
            CAwsRequest::DeleteItem(pstSession, &pstUserInfo->m_atbMarch[dwIdx]);
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: active action need to delete [actionid=%ld] [seq=%u]",
                pstUserInfo->m_atbMarch[dwIdx].m_nId, pstSession->m_udwSeqNo));
        }
        else if(EN_TABLE_UPDT_FLAG__NEW == pstUserInfo->m_aucMarchFlag[dwIdx])
        {
            CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_atbMarch[dwIdx]);
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: active action need to put [actionid=%ld] [seq=%u]",
                pstUserInfo->m_atbMarch[dwIdx].m_nId, pstSession->m_udwSeqNo));
        }
        else if(EN_TABLE_UPDT_FLAG__CHANGE == pstUserInfo->m_aucMarchFlag[dwIdx])
        {
            ExpectedItem expect_item;
            ExpectedDesc expect_desc;
            expect_item.SetVal(TbACTION_FIELD_ID, true, pstUserInfo->m_atbMarch[dwIdx].m_nId); //加expect防止边界条件(action已经结束又被新建)
            expect_desc.clear();
            expect_desc.push_back(expect_item);

            dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_atbMarch[dwIdx], expect_desc);
            if(0 == dwRetCode)
            {
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: active march action need to update [actionid=%ld] [seq=%u]",
                    pstUserInfo->m_atbMarch[dwIdx].m_nId, pstSession->m_udwSeqNo));
            }
        }
    }

    // 被动march action
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: [passive_march_num=%ld] [seq=%u]",
        pstUserInfo->m_udwPassiveMarchNum, pstSession->m_udwSeqNo));
    for(dwIdx = 0; dwIdx < pstUserInfo->m_udwPassiveMarchNum; dwIdx++)
    {
        if(EN_TABLE_UPDT_FLAG__DEL == pstUserInfo->m_aucPassiveMarchFlag[dwIdx])
        {
            CAwsRequest::DeleteItem(pstSession, &pstUserInfo->m_atbPassiveMarch[dwIdx]);
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: passive active action need to delete [actionid=%ld] [seq=%u]",
                pstUserInfo->m_atbPassiveMarch[dwIdx].m_nId, pstSession->m_udwSeqNo));
        }
        else if(pstUserInfo->m_aucPassiveMarchFlag[dwIdx] == EN_TABLE_UPDT_FLAG__CHANGE)
        {
            ExpectedItem expect_item;
            ExpectedDesc expect_desc;
            expect_item.SetVal(TbACTION_FIELD_ID, true, pstUserInfo->m_atbPassiveMarch[dwIdx].m_nId);
            expect_desc.clear();
            expect_desc.push_back(expect_item);

            dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_atbPassiveMarch[dwIdx], expect_desc);
            if(0 == dwRetCode)
            {
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: passive march action need to update [actionid=%ld] [seq=%u]",
                    pstUserInfo->m_atbPassiveMarch[dwIdx].m_nId, pstSession->m_udwSeqNo));
            }
        }
    }

    /********************************************************************************************/
    // user_stat表数据
    dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_tbUserStat);
    if(0 == dwRetCode)
    {
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: user_stat need to update [uid=%ld] [seq=%u]",
            pstUserInfo->m_tbUserStat.m_nUid, pstSession->m_udwSeqNo));
    }
    /********************************************************************************************/
    // lord_image表数据
    dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_tbLordImage);
    if (0 == dwRetCode)
    {
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: lord_image need to update [uid=%ld] [seq=%u]",
            pstUserInfo->m_tbLordImage.m_nUid, pstSession->m_udwSeqNo));
    }
    /********************************************************************************************/
    // decoration表数据
    dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_tbDecoration);
    if (0 == dwRetCode)
    {
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: decoration need to update [uid=%ld] [seq=%u]",
            pstUserInfo->m_tbDecoration.m_nUid, pstSession->m_udwSeqNo));
    }

    /********************************************************************************************/
    // 更新操作过的iap gift列表(al_gift_reward)
    for(TUINT32 dwIdx = 0; dwIdx < pstUserInfo->m_udwAlGiftRewardNum; dwIdx++)
    {
        if (pstUserInfo->m_aucAlGiftRewardFlag[dwIdx] != EN_TABLE_UPDT_FLAG__UNCHANGE)
        {
            dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_atbAlGiftReward[dwIdx]);
            if (0 == dwRetCode)
            {
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: al_gift_reward need to update [uid=%ld] [seq=%u]",
                    pstUserInfo->m_atbAlGiftReward[dwIdx].m_nUid,
                    pstSession->m_udwSeqNo));
            }
        }
    }

    /********************************************************************************************/
    // quest表数据
    dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_tbQuest);
    if(0 == dwRetCode)
    {
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: quest need to update [uid=%ld] [seq=%u]", 
            pstUserInfo->m_tbQuest.m_nUid, 
            pstSession->m_udwSeqNo));
    }

    /********************************************************************************************/
    // bounty表数据
    dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_tbBounty);
    if(0 == dwRetCode)
    {
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: bounty need to update [uid=%ld] [seq=%u]", 
            pstUserInfo->m_tbBounty.m_nUid, 
            pstSession->m_udwSeqNo));
    }

    /********************************************************************************************/
    // backpack表数据
    dwRetCode = CAwsRequest::UpdateItem(pstSession, &(pstSession->m_stUserInfo.m_tbBackpack));
    if(0 == dwRetCode)
    {
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: backpack need to update [uid=%ld], [seq=%u]", 
            pstSession->m_stUserInfo.m_tbBackpack.m_nUid, 
            pstSession->m_udwSeqNo));
    }

    /********************************************************************************************/

    // token表数据
    dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_tbApns_token);
    if(0 == dwRetCode)
    {
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: token need to update [uid=%ld] [seq=%u]",
            pstUserInfo->m_tbApns_token.m_nUid,
            pstSession->m_udwSeqNo));
    }

    /********************************************************************************************/
    // 联盟数据更新(alliance表)
    if(EN_TABLE_UPDT_FLAG__DEL == pstUserInfo->m_ucAllianceFlag)
    {
        dwRetCode = CAwsRequest::DeleteItem(pstSession, &pstUserInfo->m_tbAlliance);
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: alliance need to delete [aid=%ld] [seq=%u]",
            pstUserInfo->m_tbAlliance.m_nAid, pstSession->m_udwSeqNo));
    }
    else if(pstUserInfo->m_tbAlliance.m_nAid > 0)
    {
        dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_tbAlliance);
        if(0 == dwRetCode)
        {
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: alliance need to update [aid=%ld] [seq=%u]",
                pstUserInfo->m_tbAlliance.m_nAid, pstSession->m_udwSeqNo));
        }
    }

    /********************************************************************************************/
    // 更新帮助过的任务列表(al_help表数据)
    for(unsigned int i = 0; i < MAX_AL_HELP_LIST_NUM; ++i)
    {
        if(pstSession->m_stUserInfo.m_aucAlHelpFlag[i] > EN_TABLE_UPDT_FLAG__UNCHANGE)
        {
            //设置正确的hash key和range key
            TbAl_help& tbAl_help = pstSession->m_stUserInfo.m_atbAl_help[i];
            tbAl_help.Set_Uid(pstSession->m_stUserInfo.m_tbPlayer.m_nUid);
            tbAl_help.Set_Idx(i);
            dwRetCode = CAwsRequest::UpdateItem(pstSession, &tbAl_help);
            if(0 == dwRetCode)
            {
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: al_help need to update [uid=%ld] [actionid=%ld] [seq=%u]",
                    tbAl_help.m_nUid, tbAl_help.m_nIdx, pstSession->m_udwSeqNo));
            }
        }
    }

    /********************************************************************************************/
    // alliance-wall(update and delete)(wall表数据)
    for(dwIdx = 0; dwIdx < pstUserInfo->m_udwWallNum; dwIdx++)
    {
        if(pstSession->m_stUserInfo.m_aucWallFlag[dwIdx] == EN_TABLE_UPDT_FLAG__NEW
        || pstSession->m_stUserInfo.m_aucWallFlag[dwIdx] == EN_TABLE_UPDT_FLAG__CHANGE)
        {
            dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_atbWall[dwIdx]);
            if(0 == dwRetCode)
            {
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: wall need to update [aid=%ld] [seq=%u]",
                    pstUserInfo->m_atbWall[dwIdx].m_nAlid, pstUserInfo->m_udwBSeqNo));
            }
        }
        else if(pstUserInfo->m_aucWallFlag[dwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            CAwsRequest::DeleteItem(pstSession, &pstUserInfo->m_atbWall[dwIdx]);
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: wall need to delete [aid=%ld] [seq=%u]",
                pstUserInfo->m_atbWall[dwIdx].m_nAlid, pstUserInfo->m_udwBSeqNo));
        }
    }
    if (pstUserInfo->m_ucTmpWallFlag == EN_TABLE_UPDT_FLAG__DEL)
    {
        CAwsRequest::DeleteItem(pstSession, &pstUserInfo->m_tbTmpWall);
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: wall need to delete [aid=%ld] [seq=%u]",
            pstUserInfo->m_tbTmpWall.m_nAlid, pstUserInfo->m_udwBSeqNo));
    }
    /********************************************************************************************/
    // alliance-diplomacy(diplomacy表数据)
    for(dwIdx = 0; dwIdx < pstUserInfo->m_udwDiplomacyNum; dwIdx++)
    {
        TbDiplomacy *ptbDiplomacy = &pstUserInfo->m_atbDiplomacy[dwIdx];
        if(pstUserInfo->m_aucDiplomacyFlag[dwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            dwRetCode = CAwsRequest::DeleteItem(pstSession, ptbDiplomacy);
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: diplomacy need to delete [src=%ld] [desc=%ld] [type=%ld] [seq=%u]",
                ptbDiplomacy->m_nSrc_al, ptbDiplomacy->m_nDes_al, ptbDiplomacy->m_nType, pstUserInfo->m_udwBSeqNo));
        }
        else if(pstUserInfo->m_aucDiplomacyFlag[dwIdx] == EN_TABLE_UPDT_FLAG__CHANGE
            || pstUserInfo->m_aucDiplomacyFlag[dwIdx] == EN_TABLE_UPDT_FLAG__NEW)
        {
            dwRetCode = CAwsRequest::UpdateItem(pstSession, ptbDiplomacy);
            if(0 == dwRetCode)
            {
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: diplomacy need to update [src=%ld] [desc=%ld] [type=%ld] [seq=%u]",
                    ptbDiplomacy->m_nSrc_al, ptbDiplomacy->m_nDes_al, ptbDiplomacy->m_nType, pstUserInfo->m_udwBSeqNo));
            }
        }
    }

    /********************************************************************************************/
    // 更新用户身的assistance信息(al_assist表数据)
    for(TUINT32 dwIdx = 0; dwIdx < pstUserInfo->m_udwAlAssistAllNum; ++dwIdx)
    {
        pstUserInfo->m_atbAlAssistAll[dwIdx].Set_Sid(udwSvrId);
        if(pstUserInfo->m_aucAlAssistAllFlag[dwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            CAwsRequest::DeleteItem(pstSession, &pstSession->m_stUserInfo.m_atbAlAssistAll[dwIdx]);
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: al_assist need to delete [seq=%u]",
                pstSession->m_udwSeqNo));
        }
        else if(pstUserInfo->m_aucAlAssistAllFlag[dwIdx] == EN_TABLE_UPDT_FLAG__NEW)
        {
            CAwsRequest::PutItem(pstSession, &pstSession->m_stUserInfo.m_atbAlAssistAll[dwIdx]);
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: al_assist need to put [seq=%u]",
                pstSession->m_udwSeqNo));
        }
        else if(pstUserInfo->m_aucAlAssistAllFlag[dwIdx] == EN_TABLE_UPDT_FLAG__CHANGE)
        {
            dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstSession->m_stUserInfo.m_atbAlAssistAll[dwIdx]);
            if(0 == dwRetCode)
            {
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: al_assist need to update [seq=%u]",
                    pstSession->m_udwSeqNo));
            }
        }
    }

    /********************************************************************************************/
    // 更新tips信息(tip表数据)
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: [tipsnum=%ld] [seq=%u]",
        pstUserInfo->m_udwTipsNum, pstSession->m_udwSeqNo));
    for(dwIdx = 0; dwIdx < pstUserInfo->m_udwTipsNum; dwIdx++)
    {
        if(pstUserInfo->m_aucTipsFlag[dwIdx] == EN_TABLE_UPDT_FLAG__NEW)
        {
            if(pstUserInfo->m_ptbPushTips == NULL)
            {
                //wave@push_data
                pstUserInfo->m_ptbPushTips = &pstUserInfo->m_atbTips[dwIdx];
            }
            else
            {
                CAwsRequest::PutItem(pstSession, &pstUserInfo->m_atbTips[dwIdx]);
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: tips need to update [uid=%ld] [time=%ld] [seq=%u]",
                    pstUserInfo->m_atbTips[dwIdx].m_nUid, pstUserInfo->m_atbTips[dwIdx].m_nTime,
                    pstSession->m_udwSeqNo));
            }            
        }
    }

    /********************************************************************************************/
    // 更新tips信息(event_tip表数据)
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: [event_tipsnum=%ld] [seq=%u]",
        pstUserInfo->m_udwEventTipsNum, pstSession->m_udwSeqNo));
    for(dwIdx = 0; dwIdx < pstUserInfo->m_udwEventTipsNum; dwIdx++)
    {
        if(pstUserInfo->m_aucEventTipsFlag[dwIdx] == EN_TABLE_UPDT_FLAG__NEW)
        {
            CAwsRequest::PutItem(pstSession, &pstUserInfo->m_atbEventTips[dwIdx]);
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: event tips need to update [uid=%ld] [id=%ld] [seq=%u]",
                pstUserInfo->m_atbEventTips[dwIdx].m_nUid, pstUserInfo->m_atbEventTips[dwIdx].m_nId,
                pstSession->m_udwSeqNo));
        }
    }

    /********************************************************************************************/
    // 更新broadcast信息(broadcast表数据)  广播不能删除
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: [broadcast=%ld] [seq=%u]",
        pstUserInfo->m_udwBroadcastNum, pstSession->m_udwSeqNo));
    for(dwIdx = 0; dwIdx < pstUserInfo->m_udwBroadcastNum; dwIdx++)
    {
        if(pstUserInfo->m_aucBroadcastFlag[dwIdx] == EN_TABLE_UPDT_FLAG__NEW)
        {
            CAwsRequest::PutItem(pstSession, &pstUserInfo->m_atbBroadcast[dwIdx]);
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: broadcast need to update [ctime=%ld] [seq=%u]",
                pstUserInfo->m_atbBroadcast[dwIdx].m_nCtime,
                pstSession->m_udwSeqNo));
        }
    }

    /********************************************************************************************/
    // set update and insert package(bookmark)
    TbBookmark *pstBookmark = NULL;
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: [bookmarknum=%ld] [seq=%u]",
        pstUserInfo->m_udwBookmarkNum,
        pstSession->m_udwSeqNo));
    for(dwIdx = 0; dwIdx < pstSession->m_stUserInfo.m_udwBookmarkNum; dwIdx++)
    {
        pstBookmark = &pstSession->m_stUserInfo.m_atbBookmark[dwIdx];

        if(pstSession->m_stUserInfo.m_aucBookMarkFlag[dwIdx] == EN_TABLE_UPDT_FLAG__CHANGE)
        {
            dwRetCode = CAwsRequest::UpdateItem(pstSession, pstBookmark);
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: bookmark need to update [seq=%u]",
                pstSession->m_udwSeqNo));
        }
        else if(pstSession->m_stUserInfo.m_aucBookMarkFlag[dwIdx] == EN_TABLE_UPDT_FLAG__NEW)
        {
            dwRetCode = CAwsRequest::UpdateItem(pstSession, pstBookmark);
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: bookmark need to update [seq=%u]",
                pstSession->m_udwSeqNo));
        }
        else if(pstSession->m_stUserInfo.m_aucBookMarkFlag[dwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            CAwsRequest::DeleteItem(pstSession, pstBookmark);
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: bookmark need to delete [seq=%u]",
                pstSession->m_udwSeqNo));
        }
    }

    // 更新map
    if(pstSession->m_ucTmpMapItemFlag == EN_TABLE_UPDT_FLAG__CHANGE)
    {
        pstSession->m_tbTmpMap.Set_Sid(udwSvrId);
        dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstSession->m_tbTmpMap);
	    if(dwRetCode == 0)
        {
            TSE_LOG_INFO(pstSession->m_poServLog, ("TableUpdate: tmp map[%ld] need to update [seq=%u]", pstSession->m_tbTmpMap.m_nId, pstSession->m_udwSeqNo));
            CPushDataProcess::AddPushData_Wild(&pstSession->m_objAuPushDataNode.m_objPushDataMap, &pstSession->m_tbTmpMap);
        }
    }
    for(dwIdx = 0; dwIdx < pstSession->m_udwTmpWildNum; ++dwIdx)
    {
        TbMap *pstTmpMap = &pstSession->m_atbTmpWild[dwIdx];
        if(pstSession->m_aucTmpWildFlag[dwIdx] == EN_TABLE_UPDT_FLAG__CHANGE)
        {
            dwRetCode = CAwsRequest::UpdateItem(pstSession, pstTmpMap);
	        if(dwRetCode == 0)
            {
                TSE_LOG_INFO(pstSession->m_poServLog, ("TableUpdate:tmp wild[id=%ld] need to update [seq=%u]", pstTmpMap->m_nId, pstSession->m_udwSeqNo));
                CPushDataProcess::AddPushData_Wild(&pstSession->m_objAuPushDataNode.m_objPushDataMap, pstTmpMap);
            }
        }
    }

    /********************************************************************************************/
    // 更新al gift
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: [al_gift_num=%u] [seq=%u]",
        pstUserInfo->m_stAlGifts.m_dwGiftNum, pstSession->m_udwSeqNo));
    for(TINT32 dwIdx = 0; dwIdx < pstUserInfo->m_stAlGifts.m_dwGiftNum; dwIdx++)
    {
        if(pstUserInfo->m_stAlGifts.m_aucUpdateFlag[dwIdx] > EN_TABLE_UPDT_FLAG__UNCHANGE)
        {
            if(EN_TABLE_UPDT_FLAG__DEL == pstUserInfo->m_stAlGifts.m_aucUpdateFlag[dwIdx])
            {
                CAwsRequest::DeleteItem(pstSession, &pstUserInfo->m_stAlGifts.m_atbGifts[dwIdx]);
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: al gift need to be deleted [aid=%ld] [gift id=%ld] [seq=%u]",
                    pstUserInfo->m_stAlGifts.m_atbGifts[dwIdx].m_nAid,
                    pstUserInfo->m_stAlGifts.m_atbGifts[dwIdx].m_nId, pstSession->m_udwSeqNo));
            }
            else if(EN_TABLE_UPDT_FLAG__NEW == pstUserInfo->m_stAlGifts.m_aucUpdateFlag[dwIdx])
            {
                CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_stAlGifts.m_atbGifts[dwIdx]);
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: al gift need to be added [aid=%ld] [gift id=%ld] [seq=%u]",
                    pstUserInfo->m_stAlGifts.m_atbGifts[dwIdx].m_nAid,
                    pstUserInfo->m_stAlGifts.m_atbGifts[dwIdx].m_nId, pstSession->m_udwSeqNo));
            }
            else if(EN_TABLE_UPDT_FLAG__CHANGE == pstUserInfo->m_stAlGifts.m_aucUpdateFlag[dwIdx])
            {
                ExpectedItem expect_item;
                ExpectedDesc expect_desc;
                expect_item.SetVal(TbAL_GIFT_FIELD_ID, true, pstUserInfo->m_stAlGifts.m_atbGifts[dwIdx].m_nId); //加expect防止明明没有这个礼包却去修改它！
                expect_desc.clear();
                expect_desc.push_back(expect_item);

                dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_stAlGifts.m_atbGifts[dwIdx], expect_desc);
                if(0 == dwRetCode)
                {
                    TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: al gift need to be changed [aid=%ld] [gift id=%ld] [seq=%u]",
                        pstUserInfo->m_stAlGifts.m_atbGifts[dwIdx].m_nAid,
                        pstUserInfo->m_stAlGifts.m_atbGifts[dwIdx].m_nId, pstSession->m_udwSeqNo));
                }
            }
        }
    }

    if (pstUserInfo->m_tbClearAlGift.m_nUid > 0)
    {
        CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_tbClearAlGift);
    }

    //联盟成员购买物品,生成消费记录
    if(pstSession->m_ucTmpAlConsumeFlag > EN_TABLE_UPDT_FLAG__UNCHANGE)
    {
        CAwsRequest::UpdateItem(pstSession, &pstSession->m_tbTmpAlConsume);
    }

    //更新mail操作记录
//     dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_tbMailOperate);
//     if(0 == dwRetCode)
//     {
//         TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: mail operate need to update [uid=%ld] [seq=%u]", 
//             pstUserInfo->m_tbQuest.m_nUid, 
//             pstSession->m_udwSeqNo));
//     }

    //请求联盟帮助
    if(EN_TABLE_UPDT_FLAG__CHANGE == pstSession->m_ucTmpActionFlag)
    {
        ExpectedItem expect_item;
        ExpectedDesc expect_desc;
        expect_item.SetVal(TbACTION_FIELD_ID, true, pstSession->m_tbTmpAction.m_nId); //加expect防止边界条件(action已经结束又被新建)
        expect_desc.clear();
        expect_desc.push_back(expect_item);
        CAwsRequest::UpdateItem(pstSession, &pstSession->m_tbTmpAction, expect_desc);
    }

    //自身的联盟成员信息
    if(pstUserInfo->m_tbSelfAlmember.m_nUid > 0)
    {
        if (EN_TABLE_UPDT_FLAG__DEL == pstUserInfo->m_ucSelfAlmemberFlag)
        {
            CAwsRequest::DeleteItem(pstSession, &pstUserInfo->m_tbSelfAlmember);
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: self al_member need to delete [aid=%ld][uid=%ld] [seq=%u]",
                pstUserInfo->m_tbSelfAlmember.m_nAid, pstUserInfo->m_tbSelfAlmember.m_nUid, pstSession->m_udwSeqNo));
        }
        else
        {
            dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_tbSelfAlmember);
            if (0 == dwRetCode)
            {
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: self al_member need to update [aid=%ld][uid=%ld] [seq=%u]",
                    pstUserInfo->m_tbSelfAlmember.m_nAid, pstUserInfo->m_tbSelfAlmember.m_nUid, pstSession->m_udwSeqNo));
            }
        }
    }

    //equip crystal soul material parts
    for(TUINT32 dwIdx = 0; dwIdx < pstUserInfo->m_udwEquipNum;++dwIdx)
    {
        if(pstUserInfo->m_aucEquipFlag[dwIdx] > EN_TABLE_UPDT_FLAG__UNCHANGE)
        {
            if(EN_TABLE_UPDT_FLAG__DEL == pstUserInfo->m_aucEquipFlag[dwIdx])
            {
                CAwsRequest::DeleteItem(pstSession, &pstUserInfo->m_atbEquip[dwIdx]);
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: equip need to be deleted [uid=%u] [equip_id=%ld] [seq=%u]",
                    pstUserInfo->m_atbEquip[dwIdx].m_nUid,
                    pstUserInfo->m_atbEquip[dwIdx].m_nId, pstSession->m_udwSeqNo));
            }
            else if(EN_TABLE_UPDT_FLAG__NEW == pstUserInfo->m_aucEquipFlag[dwIdx])
            {
                CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_atbEquip[dwIdx]);
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: equip need to be added [uid=%u] [equip_id=%ld] [seq=%u]",
                    pstUserInfo->m_atbEquip[dwIdx].m_nUid,
                    pstUserInfo->m_atbEquip[dwIdx].m_nId, pstSession->m_udwSeqNo));
            }
            else if(EN_TABLE_UPDT_FLAG__CHANGE == pstUserInfo->m_aucEquipFlag[dwIdx])
            {
                ExpectedItem expect_item;
                ExpectedDesc expect_desc;
                expect_item.SetVal(TbEQUIP_FIELD_ID, true, pstUserInfo->m_atbEquip[dwIdx].m_nId); //加expect防止明明没有装备修改它！
                expect_desc.clear();
                expect_desc.push_back(expect_item);

                dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_atbEquip[dwIdx], expect_desc);
                if(0 == dwRetCode)
                {
                    TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: equip need to be changed  [uid=%u] [equip_id=%ld] [seq=%u]",
                        pstUserInfo->m_atbEquip[dwIdx].m_nUid,
                        pstUserInfo->m_atbEquip[dwIdx].m_nId, pstSession->m_udwSeqNo));
                }
            }
        }
    }

    for (TUINT32 dwIdx = 0; dwIdx < pstUserInfo->m_udwBlackListNum; dwIdx++)
    {
        if (pstUserInfo->m_aucBlackListFlag[dwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            CAwsRequest::DeleteItem(pstSession, &pstUserInfo->m_atbBlackList[dwIdx]);
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate:blacklist need to be deleted[target=%ld] [seq=%u]",
                pstUserInfo->m_atbBlackList[dwIdx].m_nTarget_uid,
                pstSession->m_udwSeqNo));
        }
        else if (pstUserInfo->m_aucBlackListFlag[dwIdx] != EN_TABLE_UPDT_FLAG__UNCHANGE)
        {
            dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_atbBlackList[dwIdx]);
            if (dwRetCode == 0)
            {
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate:blacklist need to update[target=%ld] [seq=%u]",
                    pstUserInfo->m_atbBlackList[dwIdx].m_nTarget_uid,
                    pstSession->m_udwSeqNo));
            }
        }
    }

    if (pstUserInfo->m_aucSvrAlFlag == EN_TABLE_UPDT_FLAG__DEL)
    {
        CAwsRequest::DeleteItem(pstSession, &pstUserInfo->m_tbSvrAl);
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate:svr_al need to be deleted[sid=%ld, alid=%ld] [seq=%u]",
            pstUserInfo->m_tbSvrAl.m_nSid, pstUserInfo->m_tbSvrAl.m_nAlid,
            pstSession->m_udwSeqNo));
    }
    else if (pstUserInfo->m_aucSvrAlFlag != EN_TABLE_UPDT_FLAG__UNCHANGE)
    {
        dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_tbSvrAl);
        if (dwRetCode == 0)
        {
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate:svr_al need to update[sid=%ld, alid=%ld, uid=%ld, cid=%ld] [seq=%u]",
                pstUserInfo->m_tbSvrAl.m_nSid, pstUserInfo->m_tbSvrAl.m_nAlid, pstUserInfo->m_tbSvrAl.m_nOwner_uid, 
                pstUserInfo->m_tbSvrAl.m_nOwner_cid, pstSession->m_udwSeqNo));
        }
    }

    /********************************************************************************************/
    if(pstUserInfo->m_tbTask.m_nUid)
    {
        dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_tbTask);
    }

    for (TINT32 dwIdx = 0; dwIdx < pstUserInfo->m_dwRewardWindowNum; dwIdx++)
    {
        if (pstUserInfo->m_aucRewardWindowFlag[dwIdx] == EN_TABLE_UPDT_FLAG__NEW)
        {
            dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstUserInfo->m_atbRewardWindow[dwIdx]);
            if (0 == dwRetCode)
            {
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: reward window need to update[uid: %ld id: %ld] [seq=%u]",
                    pstUserInfo->m_atbRewardWindow[dwIdx].m_nUid, pstUserInfo->m_atbRewardWindow[dwIdx].m_nId,
                    pstSession->m_udwSeqNo));
            }
        }
    }

    for (TUINT32 udwIdx = 0; udwIdx < pstUserInfo->m_udwRandomRewardNum; udwIdx++)
    {
        if (pstUserInfo->m_aucRandomRewardFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            CAwsRequest::DeleteItem(pstSession, &pstUserInfo->m_atbRandomReward[udwIdx]);
        }
    }

    for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_udwIdolNum; udwIdx++)
    {
        if (pstSession->m_ucIdolFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            dwRetCode = CAwsRequest::DeleteItem(pstSession, &pstSession->m_atbIdol[udwIdx]);
            if (dwRetCode == 0)
            {
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: idol[id:%ld pos:%ld] need to del [seq=%u]",
                    pstSession->m_atbIdol[udwIdx].m_nId, pstSession->m_atbIdol[udwIdx].m_nPos, pstSession->m_udwSeqNo));
            }
        }
        else if (pstSession->m_ucIdolFlag[udwIdx] == EN_TABLE_UPDT_FLAG__CHANGE)
        {
            dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstSession->m_atbIdol[udwIdx]);
            if (dwRetCode == 0)
            {
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: idol[id:%ld pos:%ld] need to update [seq=%u]",
                    pstSession->m_atbIdol[udwIdx].m_nId, pstSession->m_atbIdol[udwIdx].m_nPos, pstSession->m_udwSeqNo));
            }
        }
    }

    if (pstSession->m_tbThrone.m_nPos > 0)
    {
        //alid不变才可修改...
        ExpectedItem expect_item;
        ExpectedDesc expect_desc;
        expect_item.SetVal(TbTHRONE_FIELD_ALID, true, pstSession->m_ddwRawThroneAid);
        expect_desc.clear();
        expect_desc.push_back(expect_item);
        dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstSession->m_tbThrone, expect_desc);
        if (dwRetCode == 0)
        {
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate:throne need to update[sid=%ld, alid=%ld, pos=%ld] [seq=%u]",
                pstSession->m_tbThrone.m_nSid, pstSession->m_tbThrone.m_nAlid, pstSession->m_tbThrone.m_nPos,
                pstSession->m_udwSeqNo));
        }
    }

    for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_stTitleInfoList.udwNum; udwIdx++)
    {
        if (pstSession->m_stTitleInfoList.aucFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            dwRetCode = CAwsRequest::DeleteItem(pstSession, &pstSession->m_stTitleInfoList.atbTitle[udwIdx]);
            if (dwRetCode == 0)
            {
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: title [id:%ld] need to del [seq=%u]",
                    pstSession->m_stTitleInfoList.atbTitle[udwIdx].m_nId, pstSession->m_udwSeqNo));
            }
        }
        else if (pstSession->m_stTitleInfoList.aucFlag[udwIdx] == EN_TABLE_UPDT_FLAG__NEW)
        {
            dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstSession->m_stTitleInfoList.atbTitle[udwIdx]);
            if (dwRetCode == 0)
            {
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: title[id:%ld] need to put [seq=%u]",
                    pstSession->m_stTitleInfoList.atbTitle[udwIdx].m_nId, pstSession->m_udwSeqNo));
            }
        }
        else if (pstSession->m_stTitleInfoList.aucFlag[udwIdx] == EN_TABLE_UPDT_FLAG__CHANGE)
        {
            ExpectedItem expect_item;
            ExpectedDesc expect_desc;
            expect_item.SetVal(TbTITLE_FIELD_UID, true, pstSession->m_stTitleInfoList.atbTitle[udwIdx].m_nUid);
            expect_desc.clear();
            expect_desc.push_back(expect_item);
            dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstSession->m_stTitleInfoList.atbTitle[udwIdx], expect_desc);
            if (dwRetCode == 0)
            {
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("TableUpdate: title[id:%ld] need to update [seq=%u]",
                    pstSession->m_stTitleInfoList.atbTitle[udwIdx].m_nId, pstSession->m_udwSeqNo));
            }
        }
    }

    /********************************************************************************************/
    // send aws request
    TINT32 dwSize = pstSession->m_vecAwsReq.size();

    TSE_LOG_INFO(pstSession->m_poServLog, ("TableUpdate: end to update user and alliance data [aws_req_num=%d] [seq=%u]",
        dwSize, pstSession->m_udwSeqNo));

    if(pstSession->m_vecAwsReq.size() > 0)
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("Main_flow:normal: data need to update [update_req_num=%d] [seq=%u]",
            dwRetCode, pstSession->m_udwSeqNo));
        // send request
        dwRetCode = SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            return -1;
        }
    }

    return dwSize;
}

TINT32 CBaseProcedure::ProcessOperate_LoginGet(SSession *pstSession)
{
    TINT32 dwRetCode = 0;
    pstSession->ResetAwsReq();

    if(pstSession->m_stReqParam.m_udwUserId == 0)
    {
        return -1;
    }

    CAwsRequest::LoginGetByUid(pstSession, pstSession->m_stReqParam.m_udwUserId);

    // send request
    dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
    if(dwRetCode < 0)
    {
        if(pstSession->m_stCommonResInfo.m_dwRetCode == EN_RET_CODE__SUCCESS)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
        }
        return -2;
    }

    return 0;
}

TINT32 CBaseProcedure::ProcessOperate_LoginRes(SSession *pstSession)
{
    AwsRspInfo *pstHsRes = NULL;
    TbLogin *pstAccount = &pstSession->m_stUserInfo.m_tbLogin;
    TINT32 dwRetCode = 0;

    // 1. 获取账号信息
    if(pstSession->m_vecAwsRsp.size() == 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_PACKAGE_ERR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("LoginGetResponse: no valid res [seq=%u]", pstSession->m_udwSeqNo));
        return -1;
    }
    else
    {
        pstHsRes = pstSession->m_vecAwsRsp[0];
        dwRetCode = CAwsResponse::OnGetItemRsp(*pstHsRes, pstAccount);
        if(dwRetCode <= 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ZERO;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("LoginGetResponse: no such data [seq=%u]", pstSession->m_udwSeqNo));
            return -2;
        }

        // update user svr
        if(pstSession->m_stReqParam.m_udwSvrId == (TUINT32)-1)
        {
            pstSession->m_stReqParam.m_udwSvrId = pstAccount->m_nSid;
        }
    }

    // 2. 记录命令处理前的gem
    pstSession->m_ddwGemBegin = pstSession->m_stUserInfo.m_tbLogin.m_nGem;
    return 0;
}

TINT32 CBaseProcedure::ProcessProcedure_LockReq(SSession *pstSession, TUINT16 uwReqServiceType)
{
    TUCHAR *pszPack = NULL;
    TUINT32 udwPackLen = 0;
    CDownMgr *pobjDownMgr = CDownMgr::Instance();
    TUINT32 idx = 0;
    LTasksGroup	stTasksGroup;
    TUINT32 udwKeyNum = 0;
    TUINT64 auddwKeyList[MAX_LOCK_KEY_NUM_IN_ONE_REQ];
    const Json::Value& rLockJson = CLockInfo::GetInstance()->m_oJsonRoot;
    
    // 0. 选取svrid和uid
    TUINT32 udwSvrId = pstSession->m_stReqParam.m_udwSvrId;
    TUINT32 udwUserId = pstSession->m_stReqParam.m_udwUserId;
    TUINT32 udwAllianceId = pstSession->m_stReqParam.m_udwAllianceId;
    if(udwUserId == 0)
    {
        return -1;
    }
    pstSession->m_uddwDownRqstTimeBeg = CTimeUtils::GetCurTimeUs();

    auddwKeyList[udwKeyNum++] = CCommonBase::GenLockIdByLockInfo(udwUserId, EN_LOCK_ID_TYPE__UID, udwSvrId);  //替代旧逻辑
    //auddwKeyList[udwKeyNum++] = udwSvrId * 1000000000L + udwUserId;

    if(pstSession->m_ddwLockedTaskId)
    {
        auddwKeyList[udwKeyNum++] = CCommonBase::GenLockIdByLockInfo(pstSession->m_ddwLockedTaskId, EN_LOCK_ID_TYPE__TASK_ID, udwSvrId);  //替代旧逻辑
        //auddwKeyList[udwKeyNum++] = pstSession->m_ddwLockedTaskId;
    }

    if (rLockJson.isMember(pstSession->m_stReqParam.m_szCommand))
    {
        vector<string> vecMember = rLockJson[pstSession->m_stReqParam.m_szCommand].getMemberNames();

        if (vecMember.size() + udwKeyNum >= MAX_LOCK_KEY_NUM_IN_ONE_REQ)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("LockReq: key num out of range. [cmd=%s][key_num=%u][cur_num=%u][max=%u] [uid=%u][seq=%u]",
                pstSession->m_stReqParam.m_szCommand, vecMember.size(), udwKeyNum, MAX_LOCK_KEY_NUM_IN_ONE_REQ, udwUserId, pstSession->m_udwSeqNo));
        }
        else
        {
            for (vector<string>::iterator iter = vecMember.begin(); iter != vecMember.end(); iter++)
            {
                TUINT32 udwIdentifier = 0;
                string sKey = *iter;
                TINT32 dwValue = rLockJson[pstSession->m_stReqParam.m_szCommand][*iter].asInt();
                
                if (CCommonFunc::isNum(sKey))  //从key里面取值
                {
                    TINT32 dwKey = atoi(sKey.c_str());
                    if (dwKey < 0 || dwKey >= MAX_REQ_PARAM_KEY_NUM)  //越界则报错并忽略
                    {
                        TSE_LOG_ERROR(pstSession->m_poServLog, ("LockReq: key out of range. [cmd=%s][key=key%s] [uid=%u][seq=%u]",
                            pstSession->m_stReqParam.m_szCommand, sKey.c_str(), udwUserId, pstSession->m_udwSeqNo));
                        continue;
                    }
                    udwIdentifier = (TUINT32)atoi(pstSession->m_stReqParam.m_szKey[dwKey]);
                }
                else if (0 == strcmp(sKey.c_str(), "aid"))  //以后需要扩展字段则直接添加对应的分支进行处理即可
                {
                    udwIdentifier = udwAllianceId;
                }
                else
                {
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("LockReq: Prase lock key failed. [cmd=%s][key=%s][val=%d] [uid=%u][seq=%u]",
                        pstSession->m_stReqParam.m_szCommand, sKey.c_str(), dwValue, udwUserId, pstSession->m_udwSeqNo));
                    continue;
                }
                if (udwIdentifier == 0)
                {
                    continue;
                }
                TUINT64 uddwLockId = CCommonBase::GenLockIdByLockInfo(udwIdentifier, dwValue, udwSvrId);
                if (uddwLockId != 0)
                {
                    auddwKeyList[udwKeyNum++] = uddwLockId;
                }
            }
        }
    }

    string printList = "";
    for (TUINT32 udwIdx = 0; udwIdx < udwKeyNum && udwIdx < MAX_LOCK_KEY_NUM_IN_ONE_REQ; ++udwIdx)
    {
        printList.append(CCommonFunc::NumToString(auddwKeyList[udwIdx]));
        printList.append(":");
    }
    printList.resize(printList.size() - 1);
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("LockReq: [cmd=%s][list=%s][len=%u] [uid=%u][seq=%u]\n",
        pstSession->m_stReqParam.m_szCommand, printList.c_str(), udwKeyNum, udwUserId, pstSession->m_udwSeqNo));

    // 1. 获取下游——对于一个session来说，每次只需获取一次
    if(pstSession->m_bLockSvrExist == FALSE)
    {
        pstSession->m_pstLockSvr = NULL;
        // 获取下游服务器
        if(S_OK == pobjDownMgr->zk_GetNode(DOWN_NODE_TYPE__LOCK_SVR, &pstSession->m_pstLockSvr))
        {
            pstSession->m_bLockSvrExist = TRUE;
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("Get lock_svr node succ [seq=%u]", pstSession->m_udwSeqNo));
        }
        else
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("Get lock_svr node fail [seq=%u]", pstSession->m_udwSeqNo));
            return -2;
        }
    }

    // 2. 封装请求、打包、设置task
    TUINT32 udwTimeOutUs = 1000 * (DOWN_LOCK_NODE_TIMEOUT_MS);//转换为微妙
    idx = 0;
    pstSession->m_ppPackTool[idx]->ResetContent();
    pstSession->m_ppPackTool[idx]->SetServiceType(uwReqServiceType);
    pstSession->m_ppPackTool[idx]->SetSeq(pstSession->m_udwSeqNo);
    pstSession->m_ppPackTool[idx]->SetKey(EN_KEY_HU2LOCK__REQ_KEY_NUM, udwKeyNum);
    pstSession->m_ppPackTool[idx]->SetKey(EN_KEY_HU2LOCK__REQ_KEY_LIST, (TUCHAR*)&auddwKeyList[0], udwKeyNum*sizeof(TUINT64));
    pstSession->m_ppPackTool[idx]->SetKey(EN_KEY_HU2LOCK__REQ_TIMEOUT_US, udwTimeOutUs);
    pstSession->m_ppPackTool[idx]->GetPackage(&pszPack, &udwPackLen);

    if(NULL == pstSession->m_pstLockSvr->m_stDownHandle.handle)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessProcedure_LockReq:handle [handlevalue=0x%p] [seq=%u]", \
            pstSession->m_pstLockSvr->m_stDownHandle.handle, pstSession->m_udwSeqNo));
        return -3;
    }

    stTasksGroup.m_Tasks[idx].SetConnSession(pstSession->m_pstLockSvr->m_stDownHandle);
    stTasksGroup.m_Tasks[idx].SetSendData(pszPack, udwPackLen);
    stTasksGroup.m_Tasks[idx].SetNeedResponse(1);
    stTasksGroup.SetValidTasks(1);

    stTasksGroup.m_UserData1.ptr = pstSession;
    stTasksGroup.m_UserData2.u32 = pstSession->m_udwSeqNo;
    stTasksGroup.m_uTaskTimeOutVal = DOWN_SEARCH_NODE_TIMEOUT_MS;

    // 设置加锁标记
    if(uwReqServiceType == EN_SERVICE_TYPE_HU2LOCK__GET_REQ)
    {
        pstSession->m_bLockedData = TRUE;
    }

    TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessProcedure_LockReq:[keynum=%u][address=%p][seq=%u]", 
        udwKeyNum, auddwKeyList, pstSession->m_udwSeqNo))

    // 3. 发送数据
    pstSession->m_bProcessing = FALSE;
    pstSession->m_uddwProcessEndTime = CTimeUtils::GetCurTimeUs();
    TSE_LOG_INFO(pstSession->m_poServLog, ("lock_process_cost: [processBtime=%lu] [processEtime=%lu] [processCtime=%lu] [seq=%u]", \
                                           pstSession->m_uddwProcessBegTime, \
                                           pstSession->m_uddwProcessEndTime, \
                                           pstSession->m_uddwProcessEndTime - pstSession->m_uddwProcessBegTime, \
                                           pstSession->m_udwSeqNo));
    if(!pstSession->m_poLongConn->SendData(&stTasksGroup))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessProcedure_LockReq send data failed!, service[%u] [seq=%u]", uwReqServiceType, pstSession->m_udwSeqNo));
        return -4;
    }

    return 0;
}

TINT32 CBaseProcedure::ProcessProcedure_DbDataUpdateRequest(SSession *pstSession)
{
    return -2;

    pstSession->ResetDbInfo();

    //更新贸易城市信息
//     if (pstSession->m_stReqParam.m_ucLoginStatus == EN_LOGIN_STATUS__LOGIN
//         || pstSession->m_stRawCityInfo.dwSid != pstSession->m_stSelfCityInfo.dwSid
//         || pstSession->m_stRawCityInfo.dwUid != pstSession->m_stSelfCityInfo.dwUid
//         || pstSession->m_stRawCityInfo.dwX != pstSession->m_stSelfCityInfo.dwX
//         || pstSession->m_stRawCityInfo.dwY != pstSession->m_stSelfCityInfo.dwY
//         || pstSession->m_stRawCityInfo.sCityName != pstSession->m_stSelfCityInfo.sCityName)
//     {
//         if (pstSession->m_stSelfCityInfo.dwUid != 0)
//         {
//             CDbRequest::UpdateTradeCityInfo(pstSession, pstSession->m_stSelfCityInfo.dwSid, &pstSession->m_stSelfCityInfo);
//             if (pstSession->m_stRawCityInfo.dwSid != pstSession->m_stSelfCityInfo.dwSid && pstSession->m_stRawCityInfo.dwUid != 0)
//             {
//                 CDbRequest::DeleteTradeCityInfo(pstSession, pstSession->m_stRawCityInfo.dwSid, pstSession->m_stRawCityInfo.dwUid);
//             }
//         }
//     }

    if (pstSession->m_vecDbReq.size() > 0)
    {
        TINT32 dwRetCode = CBaseProcedure::SendDbRequest(pstSession, EN_SERVICE_TYPE_QUERY_MYSQL_REQ);
        if (dwRetCode != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            return -1;
        }
        return 0;
    }

    return -2;
}


TINT32 CBaseProcedure::ProcessProcedure_DataCenterRequest(SSession* pstSession)
{    
    TINT32 dwRetCode = 0;
    pstSession->ResetDataCenterInfo();

    CAwsRequest::ReqDataCenterQuestInfo(pstSession, EN_TIME_QUEST_TYPE_DAILY);
    CAwsRequest::ReqDataCenterQuestInfo(pstSession, EN_TIME_QUEST_TYPE_ALLIANCE);
    CAwsRequest::ReqDataCenterQuestInfo(pstSession, EN_TIME_QUEST_TYPE_VIP);

    // send request
    dwRetCode = CBaseProcedure::SendDataCenterRequest(pstSession, EN_SERVICE_TYPE_DATA_CENTER_REQ);
    if(dwRetCode < 0)//wave@20160506:无需报错，直接忽略dc即可
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessProcedure_DataCenterRequest: SendDataCenterRequest failed. ret=%d [seq=%u]", dwRetCode, pstSession->m_udwSeqNo));
        return -1;
    }
    /*
    if(-1 == dwRetCode)
    {
        return -1;
    }
    else if(dwRetCode < 0)
    {
        if(pstSession->m_stCommonResInfo.m_dwRetCode == EN_RET_CODE__SUCCESS)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
        }
        return -2;
    }*/

    return 0;
}


TINT32 CBaseProcedure::ProcessProcedure_DataCenterResponse(SSession* pstSession)
{
    TINT32 dwRetCode = 0;
    SUserInfo *pstUserInfo = &pstSession->m_stUserInfo;
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    TUINT32 udwUserCreateTime = pstUserInfo->m_tbLogin.m_nCtime;

    //下游容错
    if(pstSession->m_stCommonResInfo.m_dwRetCode != EN_RET_CODE__SUCCESS)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessProcedure_DataCenterResponse: ret_code=%d, no process [seq=%u]", pstSession->m_stCommonResInfo.m_dwRetCode, pstSession->m_udwSeqNo));
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SUCCESS;
        return 0;
    }

    if(pstSession->m_vecDataCenterRsp.size() == 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_PACKAGE_ERR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessProcedure_DataCenterResponse: no valid res [seq=%u]", pstSession->m_udwSeqNo));
        return -1;
    }

    SQuestNode *pstDailyQuset = &pstUserInfo->m_tbQuest.m_bDaily_quest[0];
    SQuestNode *pstAllianceQuset = &pstUserInfo->m_tbQuest.m_bDaily_quest[0];
    SQuestNode *pstVipQuset = &pstUserInfo->m_tbQuest.m_bDaily_quest[0];
    TBOOL bIsFindError = FALSE;
    for (TUINT32 udwIdx = 0; udwIdx < MAX_DAILY_ALLIANCE_VIP_QUEST_NUM; udwIdx++)
    {
        if (pstDailyQuset->m_stQuestCom[udwIdx].m_ddwLv > 10
            || pstDailyQuset->m_stQuestCom[udwIdx].m_ddwLv < 0
            || pstDailyQuset->m_stQuestCom[udwIdx].m_stReward.ddwTotalNum > 3)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessProcedure_DataCenterResponse: before daily quest error lv[%ld] reward_num[%ld] [seq=%u]", 
                pstDailyQuset->m_stQuestCom[udwIdx].m_ddwLv, pstDailyQuset->m_stQuestCom[udwIdx].m_stReward.ddwTotalNum, pstSession->m_udwSeqNo));
            bIsFindError = TRUE;
            break;
        }
        if (pstAllianceQuset->m_stQuestCom[udwIdx].m_ddwLv > 10
            || pstAllianceQuset->m_stQuestCom[udwIdx].m_ddwLv < 0
            || pstAllianceQuset->m_stQuestCom[udwIdx].m_stReward.ddwTotalNum > 3)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessProcedure_DataCenterResponse: before alliance quest error lv[%ld] reward_num[%ld] [seq=%u]",
                pstAllianceQuset->m_stQuestCom[udwIdx].m_ddwLv, pstAllianceQuset->m_stQuestCom[udwIdx].m_stReward.ddwTotalNum, pstSession->m_udwSeqNo));
            bIsFindError = TRUE;
            break;
        }
        if (pstVipQuset->m_stQuestCom[udwIdx].m_ddwLv > 10
            || pstVipQuset->m_stQuestCom[udwIdx].m_ddwLv < 0
            || pstVipQuset->m_stQuestCom[udwIdx].m_stReward.ddwTotalNum > 3)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessProcedure_DataCenterResponse: before vip quest error lv[%ld] reward_num[%ld] [seq=%u]",
                pstVipQuset->m_stQuestCom[udwIdx].m_ddwLv, pstVipQuset->m_stQuestCom[udwIdx].m_stReward.ddwTotalNum, pstSession->m_udwSeqNo));
            bIsFindError = TRUE;
            break;
        }
    }

    SRefreshData stRefreshData;    
    stRefreshData.Reset();
    vector<DataCenterRspInfo*> &vecRsp = pstSession->m_vecDataCenterRsp;
    DataCenterRspInfo *pstDataCenterRsp = NULL;
    
    
    for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecDataCenterRsp.size(); ++udwIdx)
    {
        stRefreshData.Reset();
        Json::Reader jsonReader;
        Json::Value oRspDataJson;
        for (TUINT32 udwIdx = 0; udwIdx < vecRsp.size(); ++udwIdx)
        {
            pstDataCenterRsp = vecRsp[udwIdx];
            if(EN_REFRESH_DATA_TYPE__QUEST == pstDataCenterRsp->m_udwType)
            {
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessProcedure_DataCenterResponse: data center get rsp: [json=%s] [uid=%ld][seq=%u]",
                                                        pstDataCenterRsp->m_sRspJson.c_str(), pstUserInfo->m_tbPlayer.m_nUid, pstSession->m_udwSeqNo));
    
                if(FALSE == jsonReader.parse(pstDataCenterRsp->m_sRspJson, oRspDataJson))
                {
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_DATA_CENTER_PACKAGE_ERR;
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessProcedure_DataCenterResponse: prase rsp from data center failed. [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                    return -2;
                }
                if(EN_TIME_QUEST_TYPE_DAILY == oRspDataJson["quest"]["quest_type"].asInt())
                {
                    stRefreshData.m_stQuestRsp.m_stDaliyQuest.setVal(oRspDataJson);
    
                }
                else if(EN_TIME_QUEST_TYPE_ALLIANCE == oRspDataJson["quest"]["quest_type"].asInt())
                {
                    stRefreshData.m_stQuestRsp.m_stAllianceQuest.setVal(oRspDataJson);
                }
                else if(EN_TIME_QUEST_TYPE_VIP == oRspDataJson["quest"]["quest_type"].asInt())
                {
                    stRefreshData.m_stQuestRsp.m_stVipQuest.setVal(oRspDataJson);                    
                }
                else
                {  
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessProcedure_DataCenterResponse: error quest type from data_center: [json=%s] [uid=%ld][seq=%u]",
                                                            pstDataCenterRsp->m_sRspJson.c_str(), \
                                                            pstUserInfo->m_tbPlayer.m_nUid, \
                                                            pstSession->m_udwSeqNo));
                }
            }
            else
            {
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessProcedure_DataCenterResponse: error datacenter service type from data_center: [json=%s] [uid=%ld][seq=%u]",
                                                        pstDataCenterRsp->m_sRspJson.c_str(), \
                                                        pstUserInfo->m_tbPlayer.m_nUid, \
                                                        pstSession->m_udwSeqNo));
            }
        }
    }

    // 自动刷新daily/alliance/vip quest逻辑
    for(TUINT32 udwIdx = 0; udwIdx < stRefreshData.m_stQuestRsp.m_stDaliyQuest.m_vecQuestList.size(); ++udwIdx)
    {
        SQuestNode *pstQusetNode = &pstUserInfo->m_tbQuest.m_bDaily_quest[0];
        SQuestListInfo *pstQuestListInfo = &stRefreshData.m_stQuestRsp.m_stDaliyQuest;

        TINT32 dwCurQuestNum = pstUserInfo->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_DAILY_QUEST_NUM].m_ddwBuffTotal;
        CQuestLogic::NewGenQuestNum(pstQusetNode, pstQuestListInfo, dwCurQuestNum, udwCurTime, udwUserCreateTime);
        
        pstUserInfo->m_tbQuest.SetFlag(TbQUEST_FIELD_DAILY_QUEST);
    }

    for(TUINT32 udwIdx = 0; udwIdx < stRefreshData.m_stQuestRsp.m_stAllianceQuest.m_vecQuestList.size(); ++udwIdx)
    {
        SQuestNode *pstQusetNode = &pstUserInfo->m_tbQuest.m_bAl_quest[0];
        SQuestListInfo *pstQuestListInfo = &stRefreshData.m_stQuestRsp.m_stAllianceQuest;
        
        TINT32 dwCurQuestNum = pstUserInfo->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_ALLIANCE_NUM].m_ddwBuffTotal;
        CQuestLogic::NewGenQuestNum(pstQusetNode, pstQuestListInfo, dwCurQuestNum, udwCurTime, udwUserCreateTime);
        
        pstUserInfo->m_tbQuest.SetFlag(TbQUEST_FIELD_AL_QUEST);
    }
    
    for(TUINT32 udwIdx = 0; udwIdx < stRefreshData.m_stQuestRsp.m_stVipQuest.m_vecQuestList.size(); ++udwIdx)
    {
        SQuestNode *pstQusetNode = &pstUserInfo->m_tbQuest.m_bVip_quest[0];
        SQuestListInfo *pstQuestListInfo = &stRefreshData.m_stQuestRsp.m_stVipQuest;
        
        TINT32 dwCurQuestNum = pstUserInfo->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_VIP_QUEST_NUM].m_ddwBuffTotal;
        CQuestLogic::NewGenQuestNum(pstQusetNode, pstQuestListInfo, dwCurQuestNum, udwCurTime, udwUserCreateTime);

        pstUserInfo->m_tbQuest.SetFlag(TbQUEST_FIELD_VIP_QUEST);
    }

    SCityInfo *pstCityInfo = &pstSession->m_stUserInfo.m_stCityInfo;
    if(NULL != pstCityInfo)
    {
        //检查任务是否完成
        CQuestLogic::CheckTimeQuestFinish(&pstSession->m_stUserInfo, pstCityInfo, &pstSession->m_stUserInfo.m_tbQuest);
    }
    else
    {
        return -2;
    }
    
    for (TUINT32 udwIdx = 0; udwIdx < MAX_DAILY_ALLIANCE_VIP_QUEST_NUM; udwIdx++)
    {
        if (pstDailyQuset->m_stQuestCom[udwIdx].m_ddwLv > 10
            || pstDailyQuset->m_stQuestCom[udwIdx].m_ddwLv < 0
            || pstDailyQuset->m_stQuestCom[udwIdx].m_stReward.ddwTotalNum > 3)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessProcedure_DataCenterResponse: after daily quest error lv[%ld] reward_num[%ld] [seq=%u]",
                pstDailyQuset->m_stQuestCom[udwIdx].m_ddwLv, pstDailyQuset->m_stQuestCom[udwIdx].m_stReward.ddwTotalNum, pstSession->m_udwSeqNo));
            if (bIsFindError == FALSE)
            {
                TCHAR szMsg[1024];

                sprintf(szMsg, "./send_warning.sh \"uid=%ld player daily quest refresh error seq=[%u]\" quest",
                    pstUserInfo->m_tbPlayer.m_nUid, pstSession->m_udwSeqNo);

                CMsgBase::SendDelaySystemMsg(szMsg);
            }
            break;
        }
        if (pstAllianceQuset->m_stQuestCom[udwIdx].m_ddwLv > 10
            || pstAllianceQuset->m_stQuestCom[udwIdx].m_ddwLv < 0
            || pstAllianceQuset->m_stQuestCom[udwIdx].m_stReward.ddwTotalNum > 3)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessProcedure_DataCenterResponse: after alliance quest error lv[%ld] reward_num[%ld] [seq=%u]",
                pstAllianceQuset->m_stQuestCom[udwIdx].m_ddwLv, pstAllianceQuset->m_stQuestCom[udwIdx].m_stReward.ddwTotalNum, pstSession->m_udwSeqNo));
            if (bIsFindError == FALSE)
            {
                TCHAR szMsg[1024];

                sprintf(szMsg, "./send_warning.sh \"uid=%ld player alliance quest refresh error seq=[%u]\" quest",
                    pstUserInfo->m_tbPlayer.m_nUid, pstSession->m_udwSeqNo);

                CMsgBase::SendDelaySystemMsg(szMsg);
            }
            break;
        }
        if (pstVipQuset->m_stQuestCom[udwIdx].m_ddwLv > 10
            || pstVipQuset->m_stQuestCom[udwIdx].m_ddwLv < 0
            || pstVipQuset->m_stQuestCom[udwIdx].m_stReward.ddwTotalNum > 3)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessProcedure_DataCenterResponse: after vip quest error lv[%ld] reward_num[%ld] [seq=%u]",
                pstVipQuset->m_stQuestCom[udwIdx].m_ddwLv, pstVipQuset->m_stQuestCom[udwIdx].m_stReward.ddwTotalNum, pstSession->m_udwSeqNo));
            if (bIsFindError == FALSE)
            {
                TCHAR szMsg[1024];

                sprintf(szMsg, "./send_warning.sh \"uid=%ld player vip quest refresh error seq=[%u]\" quest",
                    pstUserInfo->m_tbPlayer.m_nUid, pstSession->m_udwSeqNo);

                CMsgBase::SendDelaySystemMsg(szMsg);
            }
            break;
        }
    }

    //wave@20161130: for bug check
    CQuestLogic::CheckPlayerTimeQuestValid(pstUserInfo, "datacenter_response_end");

    return dwRetCode;
    
}

TINT32 CBaseProcedure::ProcessProcedure_ReportSvrRequest(SSession* pstSession)
{
    TINT32 dwRetCode = 0;
    pstSession->ResetReportInfo();

    CReportSvrRequest::QueryReportIdGet(pstSession);
    CReportSvrRequest::QueryMailIdGet(pstSession);

    // send request
    dwRetCode = CBaseProcedure::SendReportSvrRequest(pstSession);
    if (dwRetCode < 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessProcedure_ReportSvrRequest: SendData failed. ret=%d [seq=%u]", dwRetCode, pstSession->m_udwSeqNo));
        return -1;
    }

    return 0;
}

TINT32 CBaseProcedure::ProcessProcedure_ReportSvrResponse(SSession* pstSession)
{
    SUserInfo *pstUserInfo = &pstSession->m_stUserInfo;
    vector<ReportRspInfo*> &vecRsp = pstSession->m_vecReportRsp;
    ReportRspInfo *pstReportRsp = NULL;

    //下游容错
    if (pstSession->m_stCommonResInfo.m_dwRetCode != EN_RET_CODE__SUCCESS)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessProcedure_ReportSvrResponse: ret_code=%d, no process [seq=%u]", pstSession->m_stCommonResInfo.m_dwRetCode, pstSession->m_udwSeqNo));
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SUCCESS;
        return 0;
    }

    if (vecRsp.size() == 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessProcedure_DataCenterResponse: no valid res [seq=%u]", pstSession->m_udwSeqNo));
        return 0;
    }

    for (TUINT32 udwIdx = 0; udwIdx < vecRsp.size(); ++udwIdx)
    {
        pstReportRsp = vecRsp[udwIdx];
        if (pstReportRsp->dwRspType == 0)
        {
            memcpy(&pstUserInfo->m_stReportUserInfo, pstReportRsp->sRspContent.c_str(), pstReportRsp->sRspContent.size());
        }
        else
        {
            memcpy(&pstUserInfo->m_stMailUserInfo, pstReportRsp->sRspContent.c_str(), pstReportRsp->sRspContent.size());
        }
    }

    TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessProcedure_DataCenterResponse:  %ld %ld %u %ld %ld %ld %u %ld[seq=%u]", 
        pstUserInfo->m_stReportUserInfo.m_ddwNewestRid, pstUserInfo->m_tbUserStat.m_nNewest_reportid,
        pstUserInfo->m_stReportUserInfo.m_udwReportUnreadNum, pstUserInfo->m_tbUserStat.m_nUnread_report,
        pstUserInfo->m_stMailUserInfo.m_ddwNewestMid, pstUserInfo->m_tbUserStat.m_nNewest_mailid,
        pstUserInfo->m_stMailUserInfo.m_udwMailUnreadNum, pstUserInfo->m_tbUserStat.m_nUnread_mail,
        pstSession->m_udwSeqNo));

    if (pstUserInfo->m_stReportUserInfo.m_ddwNewestRid > pstUserInfo->m_tbUserStat.m_nNewest_reportid)
    {
        pstUserInfo->m_tbUserStat.Set_Newest_reportid(pstUserInfo->m_stReportUserInfo.m_ddwNewestRid);
        if (pstUserInfo->m_stReportUserInfo.m_udwReportUnreadNum != pstUserInfo->m_tbUserStat.m_nUnread_report)
        {
            pstUserInfo->m_tbUserStat.Set_Unread_report(pstUserInfo->m_stReportUserInfo.m_udwReportUnreadNum);
        }
    }

    pstSession->m_bNeedMailMusic = FALSE;
    if (pstUserInfo->m_stMailUserInfo.m_ddwNewestMid > pstSession->m_stUserInfo.m_tbUserStat.m_nNewest_mailid)
    {
        for (TUINT32 udwIdx = 0; udwIdx < pstUserInfo->m_stMailUserInfo.m_udwMailEntryNum; udwIdx++)
        {
            if (pstUserInfo->m_stMailUserInfo.m_aMailToReturn[udwIdx].ddwMid > pstSession->m_stUserInfo.m_tbUserStat.m_nNewest_mailid
                && pstUserInfo->m_stMailUserInfo.m_aMailToReturn[udwIdx].ddwSuid != pstUserInfo->m_tbPlayer.m_nUid)
            {
                pstSession->m_bNeedMailMusic = TRUE;
                break;
            }
        }
        pstSession->m_stUserInfo.m_tbUserStat.Set_Newest_mailid(pstUserInfo->m_stMailUserInfo.m_ddwNewestMid);
        if (pstUserInfo->m_stMailUserInfo.m_udwMailUnreadNum != pstSession->m_stUserInfo.m_tbUserStat.m_nUnread_mail)
        {
            pstSession->m_stUserInfo.m_tbUserStat.Set_Unread_mail(pstUserInfo->m_stMailUserInfo.m_udwMailUnreadNum);
        }
    }
    
    return 0;
}

TINT32 CBaseProcedure::ProcessProcedure_DataOutputUpdateRequest( SSession* pstSession )
{
    TINT32 dwRetCode = 0;
    dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstSession->m_stUserInfo.m_tbDataOutput);
    if(pstSession->m_vecAwsReq.size() == 0)
    {
        return 1;
    }

    TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessProcedure_DataOutputUpdateRequest: data_output info update, aws_req_num=%u [seq=%u]",
        pstSession->m_vecAwsReq.size(), pstSession->m_udwSeqNo));

    // send request
    dwRetCode = SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
    if(dwRetCode < 0)
    {
        return -2;
    }

    return 0;
}

TINT32 CBaseProcedure::ProcessProcedure_PushDataRequest( SSession* pstSession )
{
    //TODO
    return 1;
}

TINT32 CBaseProcedure::ProcessProcedure_PushDataResponse( SSession* pstSession )
{
    //TODO
    return 1;
}

TINT32 CBaseProcedure::ProcessProcedure_RankSvrRequest(SSession* pstSession)
{
    TbPlayer *ptbPlayer = &pstSession->m_stUserInfo.m_tbPlayer;

    if (ptbPlayer->m_nUid == 0)
    {
        return 1;
    }

    TINT32 dwRetCode = 0;
    pstSession->ResetRankSvrInfo();
    
    RankSvrReqInfo *ptbRankReq = new RankSvrReqInfo;

    pstSession->m_vecRankSvrReq.push_back(ptbRankReq);

    ptbRankReq->SetVal(ptbPlayer->m_nSid, ptbPlayer->m_nUid, "get_player_self_rank");

    // send request
    dwRetCode = CBaseProcedure::SendRankSvrRequest(pstSession);
    if (dwRetCode < 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessProcedure_RankSvrRequest: SendData failed. ret=%d [seq=%u]", dwRetCode, pstSession->m_udwSeqNo));
        return -1;
    }

    return 0;
}

TINT32 CBaseProcedure::ProcessProcedure_RankSvrResponse(SSession* pstSession)
{
    SUserInfo *pstUserInfo = &pstSession->m_stUserInfo;
    vector<RankSvrRspInfo*> &vecRsp = pstSession->m_vecRankSvrRsp;
    RankSvrRspInfo *pstRankRsp = NULL;

    //下游容错
    if (pstSession->m_stCommonResInfo.m_dwRetCode != EN_RET_CODE__SUCCESS)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessProcedure_RankSvrResponse: ret_code=%d, no process [seq=%u]", pstSession->m_stCommonResInfo.m_dwRetCode, pstSession->m_udwSeqNo));
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SUCCESS;
        return 0;
    }

    if (vecRsp.size() == 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessProcedure_RankSvrResponse: no valid res [seq=%u]", pstSession->m_udwSeqNo));
        return 0;
    }

    Json::Reader reader;
    for (TUINT32 udwIdx = 0; udwIdx < vecRsp.size(); ++udwIdx)
    {
        pstRankRsp = vecRsp[udwIdx];
        
        if (pstRankRsp->m_udwUid != 0 && pstRankRsp->m_udwUid == pstUserInfo->m_tbPlayer.m_nUid)
        {
            if (reader.parse(pstRankRsp->m_sRspJson, pstUserInfo->m_jPlayerRankInfo) == TRUE)
            {
                break;
            }
        }
    }

    return 0;
}
