#include "procedure_base.h"
#include "time_utils.h"
#include "global_serv.h"

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
    if (pstSession->m_bEventProxyExist == FALSE)
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
    if (!pstSession->m_poLongConn->SendData(&stTasksGroup))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("SendGlobalHsRequest send data failed! [seq=%u]", pstSession->m_udwSeqNo));
        return -4;
    }

    return 0;
}
