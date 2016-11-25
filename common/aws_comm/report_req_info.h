#ifndef _REPORT_REQ_INFO_H_
#define _REPORT_REQ_INFO_H_
#include <string>

using namespace std;

enum EReportUserProcessCmd
{
    EN_REPORT_USER_OP__REPORT_GET = 0,
    EN_REPORT_USER_OP__REPORT_DETAIL_GET,
    EN_REPORT_USER_OP__REPORT_READ,
    EN_REPORT_USER_OP__REPORT_DEL,
    EN_REPORT_USER_OP__ALLIANCE_REPORT_GET,
    EN_REPORT_USER_OP__OP_REPORT_GET,
    EN_REPORT_USER_OP__REPORT_SEND,
};

enum EMailUserProcessCmd
{
    EN_MAIL_USER_OP__MAIL_GET = 0,
    EN_MAIL_USER_OP__MAIL_DETAIL_GET,
    EN_MAIL_USER_OP__MAIL_READ,
    EN_MAIL_USER_OP__MAIL_DEL,
    EN_MAIL_USER_OP__MAIL_STAR,
    EN_MAIL_USER_OP__MAIL_UNSTAR,
    EN_MAIL_USER_OP__MAIL_REWARDCOLLECT,
    EN_MAIL_USER_OP__MAIL_SEND, //暂不使用
    EN_MAIL_USER_OP__MAIL_SEND_BY_ID,
    EN_MAIL_USER_OP__OPMAIL_SEND, //暂不使用
    EN_MAIL_USER_OP__SET_INVITEMAIL_TIME,
    EN_MAIL_USER_OP__SET_MAIL_STATUS,
    EN_MAIL_USER_OP__DELETE_MAIL_USER,
};

struct ReportReqInfo
{
    TUINT32 udwReqType; //0: report 1: mail
	string sReqContent;

    ReportReqInfo()
    {
        Reset();
    }

    void Reset()
    {
        udwReqType = 0;
        sReqContent.clear();
    }
};

#endif
