#ifndef _PROCESS_MAIL_REPORT_H_
#define _PROCESS_MAIL_REPORT_H_


#include "session.h"



class CProcessMailReport
{

public: 
    
    
    /***********************************mail����ز���**************************************/
    static TINT32 ProcessCmd_MailSend(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_MailSendById(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_OpMailSend(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 ProcessCmd_MailGet(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_MailDetailGet(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_OpMailGet(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 ProcessCmd_MailRead(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_MailDel(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_MailStar(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_MailUnstar(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 ProcessCmd_MailRewardCollect(SSession* pstSession, TBOOL& bNeedResponse);

    static TINT32 ProcessCmd_GiftSend(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_GiftPickUp(SSession *pstSession, TBOOL &bNeedResponse);
    /***********************************report����ز���**************************************/
    static TINT32 ProcessCmd_ReportGet(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_ReportDetailGet(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 ProcessCmd_ReportRead(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_ReportDel(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ==> ��ȡ��ǰҳreport��id�б�
    static TINT32 ProcessCmd_AllianceReportGet(SSession *pstSession, TBOOL &bNeedResponse);
    // function  ==> ��ȡ��ǰҳreport��id�б�
    static TINT32 ProcessCmd_OpReportGet(SSession *pstSession, TBOOL &bNeedResponse);

    /***********************************�����ʼ�����ز���**************************************/
    // ��common after �е���, �ж��Ƿ��ɹ����ʼ����͵ĺ���
    static TVOID AutoSendEncourageMail(SSession *pstSession);

    static TINT32 SendEventMail(TINT32 dwSvrId, TUINT64 uddwUid, TUINT32 udwEventType, TUINT32 udwRankOrGoal, TINT32 dwKey,
        TUINT32 udwPersonOrAlliance, string sMsgAdd1, string sMsgAdd2, TBOOL bIsClaim,TUINT32 udwPoint,
        SGlobalRes *pstGlobalRes = NULL, string sLang = "english");

    static TINT32 SendEventMailToAlliance(TINT32 dwSvrId, TUINT64 uddAid, TUINT32 udwEventType, TUINT32 udwRankOrGoal, TINT32 dwKey,
        TUINT32 udwPersonOrAlliance, string sMsgAdd1, string sMsgAdd2, TBOOL bIsClaim, TUINT32 udwPoint,
        SGlobalRes *pstGlobalRes = NULL, string sLang = "english");

    static TINT32 GetEventMailId(TUINT32 udwEventType, TUINT32 udwRankOrGoal, TUINT32 udwPersonOrAlliance);

    static TINT32 GenEventMail(TINT32 dwSvrId, TUINT64 uddwUid, TUINT32 udwEventType, TUINT32 udwRankOrGoal, TINT32 dwKey,
        TUINT32 udwPersonOrAlliance, string sMsgAdd1, string sMsgAdd2, TBOOL bIsClaim, TUINT32 udwPoint,
        /*SGlobalRes *pstGlobalRes, */
        SOneGlobalRes *astRewardList, TUINT32 udwRewardNum,
        string sLang, TINT64 ddwMailId, TINT64 ddwEventType, TINT64 ddwEventScore,
        TbMail *ptbMail, TbMail_user *ptbMailUser, const string sEventInfo);

    //numtostring
    static string NumToSplitString(TINT64 ddwNum);

private:
    /***********************************mail����ز���**************************************/
    static TINT64 GetMailDisplayClass(const TbMail_user& tbMailUser);
    static TINT32 SetMailStatus(TINT32 dwStatus, SSession* pstSession, TBOOL& bNeedResponse, TBOOL bClear = FALSE);

    /***********************************report����ز���**************************************/
    // function  ==> report���¼����Ƚ�(��reportid��ֵ)
    static bool TbReport_Compare_Reverse(const TbReport *pstA, const TbReport *pstB);
    static TINT64 GetReportDisplayClass(const TbReport_user& tbReportUser);
    static TINT32 SetReportStatus(TINT32 dwStatus, SSession* pstSession, TBOOL& bNeedResponse, TBOOL bClear = FALSE);
    static string GetMailTitle(SSession* pstSession, TINT32 dwDocId);
};

#endif
