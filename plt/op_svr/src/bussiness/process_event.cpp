#include "process_event.h"
#include "procedure_base.h"
#include "common_func.h"
#include "game_define.h"
#include "common_base.h"
#include "city_base.h"

TINT32 CProcessEvent::ProcessCmd_AllEventGet(SSession *pstSession, TBOOL& bNeedResponse)
{
    TINT32 dwRetCode = 0;

    string szEventInfo = pstSession->m_stReqParam.m_szKey[0];

    Json::Value jTmp;
    Json::Reader reader;
    if (reader.parse(szEventInfo, jTmp) == FALSE)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GET_EVENT_FAIL;
        return -1;
    }

    TUINT32 udwAlid = jTmp["alid"].asUInt();
    TUINT32 udwSid = jTmp["sid"].asUInt();
    TUINT32 udwUid = jTmp["uid"].asUInt();
    string strUname = jTmp["uname"].asString();
    string strAlNick = jTmp["alnick"].asString();
    TUINT32 udwTrialLv = jTmp["trial_lv"].asUInt();

    // 1.пео╒
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_AllEventInfoGet: event_info=%s [seq=%u]",
            szEventInfo.c_str(), pstSession->m_udwSeqNo));
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__EVENT;

        // send request
        bNeedResponse = TRUE;
        EventReqInfo *pstEventReq = new EventReqInfo;
        EventReqInfo *pstThemeEvenReq = new EventReqInfo;

        pstSession->m_vecEventReq.push_back(pstEventReq);
        pstEventReq->m_udwIdxNo = 0;
        pstEventReq->m_udwRequestType = EN_REQUEST_TYPE__GET_ALL_INFO;
        pstEventReq->m_sReqContent = szEventInfo;
        pstEventReq->m_udwIsTheme = 0;
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_AllEventInfoGet: EventReqInfo req_string=%s IsTheme=%u [seq=%u]",
            pstEventReq->m_sReqContent.c_str(), pstEventReq->m_udwIsTheme, pstSession->m_udwSeqNo));
        //theme event req
        pstSession->m_vecEventReq.push_back(pstThemeEvenReq);
        pstThemeEvenReq->SetVal(udwSid, udwUid, udwAlid, strUname, strAlNick, udwTrialLv, 1, EN_REQUEST_TYPE__GET_ALL_INFO, 0);
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_AllEventInfoGet: EventReqInfo req_string=%s IsTheme=%u [seq=%u]",
            pstThemeEvenReq->m_sReqContent.c_str(), pstThemeEvenReq->m_udwIsTheme, pstSession->m_udwSeqNo));
        dwRetCode = CBaseProcedure::SendEventRequest(pstSession, EN_SERVICE_TYPE_QUERY_EVENT_REQ);
        if (dwRetCode != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_EVENT_REQ_ERR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AllEventInfoGet: send event req fail [ret=%d] [seq=%u]", dwRetCode, pstSession->m_udwSeqNo));
            return -1;
        }

        return 0;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        Json::FastWriter writer;
        writer.omitEndingLineFeed();
        for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecEventRsp.size(); udwIdx++)
        {
            Json::Reader reader;
            Json::Value stEventDesc;

            if (pstSession->m_vecEventRsp[udwIdx]->dwRetCode != 0 || FALSE == reader.parse(pstSession->m_vecEventRsp[udwIdx]->sRspContent, stEventDesc))
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GET_EVENT_FAIL;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AllEventInfoGet: pase event info fail [seq=%u]", pstSession->m_udwSeqNo));
                return -2;
            }
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_AllEventInfoGet: [json_value=%s] [seq=%u]", pstSession->m_vecEventRsp[udwIdx]->sRspContent.c_str(), pstSession->m_udwSeqNo));
        }
    }
    pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_EVENT;
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}
