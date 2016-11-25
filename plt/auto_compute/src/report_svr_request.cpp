#include "report_svr_request.h"

TINT32 CReportSvrRequest::UserReportPut(SSession *pstSession, TINT64 ddwUid, TINT64 ddwReportId, TINT64 ddwReportType, TUINT32 udwType)
{
    ostringstream oss;
    oss.str("");

    ReportReqInfo *pReq = new ReportReqInfo;
    pstSession->m_vecReportReq.push_back(pReq);

    pReq->udwReqType = 0;
    oss << EN_REPORT_USER_OP__REPORT_SEND << '\t' << ddwUid << '\t'
        << ddwReportId << ',' << ddwReportType << ',' << udwType << ',' << 0;
    pReq->sReqContent = oss.str();
    return 0;
}