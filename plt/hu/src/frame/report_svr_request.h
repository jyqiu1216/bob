#ifndef _REPORT_SVR_REQUEST_H_
#define _REPORT_SVR_REQUEST_H_

#include "session.h"

class CReportSvrRequest
{
public:
    static TINT32 QueryReportIdGet(SSession* pstSession);
    static TINT32 QueryReportDetailIdGet(SSession* pstSession, TINT64 ddwReportType);
    static TINT32 UpdateReportUserReq(SSession* pstSession, TINT32 dwStatus, TUINT32 udwOpCount, TUINT32 *audwOptype, TINT64 *addwReportId);
    static TINT32 AllianceReportIdGet(SSession* pstSession, TINT32 dwGetType);

public:
    static TINT32 UserReportPut(SSession *pstSession, TINT64 ddwUid, TINT64 ddwReportId, TINT64 ddwReportType, TUINT32 udwType);

public:
    static TINT32 QueryMailIdGet(SSession* pstSession, TINT32 bIsSupport = 0);
    static TINT32 QueryMailDetailIdGet(SSession* pstSession, TINT64 ddwMailId, TINT64 ddwDisplayType, TINT64 ddwSuid, TINT64 ddwRuid, TINT32 bIsSupport = 0);
    static TINT32 SetMailStatus(SSession* pstSession, TINT32 dwStatus, TBOOL bIsClear, const TCHAR* pOpMailList);
    static TINT32 SetMailRewardCollect(SSession* pstSession, TINT64 ddwMailId);
    static TINT32 SetMailRewardCollect(SSession* pstSession, TbPlayer *ptbPlayer, TbUser_stat *ptbUserStat, TINT64 ddwMailId);
    static TINT32 MailUserPut(SSession* pstSession, TbMail_user *atbMailUser, TUINT32 udwMailUserNum);
    static TINT32 MailUserDelete(SSession* pstSession, TINT64 ddwUid, TINT64 ddwMid);
};
 


#endif


