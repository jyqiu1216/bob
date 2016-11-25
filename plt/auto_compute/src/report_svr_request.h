#ifndef _REPORT_SVR_REQUEST_H_
#define _REPORT_SVR_REQUEST_H_

#include "session.h"

class CReportSvrRequest
{
public:
    static TINT32 UserReportPut(SSession *pstSession, TINT64 ddwUid, TINT64 ddwReportId, TINT64 ddwReportType, TUINT32 udwType);
};
 


#endif


