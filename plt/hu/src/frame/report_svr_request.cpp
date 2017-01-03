#include "report_svr_request.h"

TINT32 CReportSvrRequest::QueryReportIdGet(SSession* pstSession)
{
    ostringstream oss;
    oss.str("");

    ReportReqInfo *pReq = new ReportReqInfo;
    pstSession->m_vecReportReq.push_back(pReq);

    pReq->udwReqType = 0;
    oss << EN_REPORT_USER_OP__REPORT_GET << '\t' << pstSession->m_stReqParam.m_udwUserId << '\t' << pstSession->m_stReqParam.m_udwPage;
    pReq->sReqContent = oss.str();
    return 0;
}

TINT32 CReportSvrRequest::QueryReportDetailIdGet(SSession* pstSession, TINT64 ddwReportType)
{
    ostringstream oss;
    oss.str("");

    ReportReqInfo *pReq = new ReportReqInfo;
    pstSession->m_vecReportReq.push_back(pReq);

    pReq->udwReqType = 0;
    oss << EN_REPORT_USER_OP__REPORT_DETAIL_GET << '\t' << pstSession->m_stReqParam.m_udwUserId << '\t' << ddwReportType << ',' << pstSession->m_stReqParam.m_udwPage;
    pReq->sReqContent = oss.str();
    return 0;
}

TINT32 CReportSvrRequest::UpdateReportUserReq(SSession* pstSession, TINT32 dwStatus, TUINT32 udwOpCount, TUINT32 *audwOptype, TINT64 *addwReportId)
{
    ostringstream oss;
    oss.str("");

    ReportReqInfo *pReq = new ReportReqInfo;
    pstSession->m_vecReportReq.push_back(pReq);
    TUINT32 udwCmd;
    if (dwStatus == EN_MAIL_STATUS_READ)
    {
        udwCmd = EN_REPORT_USER_OP__REPORT_READ;
    }
    else
    {
        udwCmd = EN_REPORT_USER_OP__REPORT_DEL;
    }

    pReq->udwReqType = 0;
    oss << udwCmd << '\t' << pstSession->m_stReqParam.m_udwUserId << '\t';
    for (TUINT32 udwIdx = 0; udwIdx < udwOpCount; udwIdx++)
    {
        if (udwIdx != 0)
        {
            oss << ',';
        }
        oss << audwOptype[udwIdx] << ':' << addwReportId[udwIdx];
    }

    pReq->sReqContent = oss.str();
    return 0;
}

TINT32 CReportSvrRequest::AllianceReportIdGet(SSession* pstSession, TINT32 dwGetType)
{
    ostringstream oss;
    oss.str("");
    TINT64 ddwKey = pstSession->m_stReqParam.m_udwAllianceId;
    ddwKey *= -1;

    ReportReqInfo *pReq = new ReportReqInfo;
    pstSession->m_vecReportReq.push_back(pReq);

    pReq->udwReqType = 0;
    oss << EN_REPORT_USER_OP__ALLIANCE_REPORT_GET << '\t' << ddwKey << '\t' << dwGetType << ',' << pstSession->m_stReqParam.m_udwPage << ',' << pstSession->m_stReqParam.m_udwPerpage;
    pReq->sReqContent = oss.str();
    return 0;
}

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

TINT32 CReportSvrRequest::QueryMailIdGet(SSession* pstSession, TINT32 bIsSupport)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    
    ostringstream oss;
    oss.str("");

    ReportReqInfo *pReq = new ReportReqInfo;
    pstSession->m_vecReportReq.push_back(pReq);

    TINT64 ddwAlid = pstUser->m_tbPlayer.m_nAlpos != EN_ALLIANCE_POS__REQUEST ? pstUser->m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET : 0;

    pReq->udwReqType = 1;
    oss << EN_MAIL_USER_OP__MAIL_GET << ',' << pstUser->m_tbPlayer.m_nUid << ',' << pstUser->m_tbPlayer.m_sUin 
        << ',' << ddwAlid << ',' << pstUser->m_tbUserStat.m_nBirth_m << ',' << pstUser->m_tbPlayer.m_nAl_time 
        << ',' << pstUser->m_tbPlayer.m_nSid << ',' << pstUser->m_tbPlayer.m_nSvr_change_time 
        << ',' << pstUser->m_tbPlayer.m_nInvite_mail_time << ',' << pstSession->m_stReqParam.m_szPlatForm
        << ',' << pstSession->m_stReqParam.m_szVs << ',' << bIsSupport
        << ',' << pstSession->m_stReqParam.m_udwPage;
    pReq->sReqContent = oss.str();
    return 0;
}

TINT32 CReportSvrRequest::QueryMailDetailIdGet(SSession* pstSession, TINT64 ddwMailId, TINT64 ddwDisplayType, TINT64 ddwSuid, TINT64 ddwRuid, TINT32 bIsSupport)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;

    ostringstream oss;
    oss.str("");

    ReportReqInfo *pReq = new ReportReqInfo;
    pstSession->m_vecReportReq.push_back(pReq);

    TINT64 ddwAlid = pstUser->m_tbPlayer.m_nAlpos != EN_ALLIANCE_POS__REQUEST ? pstUser->m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET : 0;

    pReq->udwReqType = 1;
    oss << EN_MAIL_USER_OP__MAIL_DETAIL_GET << ',' << pstUser->m_tbPlayer.m_nUid << ',' << ddwAlid
        << ',' << pstUser->m_tbUserStat.m_nBirth_m << ',' << pstUser->m_tbPlayer.m_nAl_time
        << ',' << pstUser->m_tbPlayer.m_nSid << ',' << pstUser->m_tbPlayer.m_nSvr_change_time
        << ',' << pstUser->m_tbPlayer.m_nInvite_mail_time << ',' << pstSession->m_stReqParam.m_szPlatForm 
        << ',' << pstSession->m_stReqParam.m_szVs << ',' << ddwMailId
        << ',' << ddwDisplayType << ',' << ddwSuid << ',' << ddwRuid << ',' << bIsSupport
        << ',' << pstSession->m_stReqParam.m_udwPage;
    pReq->sReqContent = oss.str();
    return 0;
}

TINT32 CReportSvrRequest::SetMailStatus(SSession* pstSession, TINT32 dwStatus, TBOOL bIsClear, const TCHAR* pOpMailList)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;

    ostringstream oss;
    oss.str("");

    ReportReqInfo *pReq = new ReportReqInfo;
    pstSession->m_vecReportReq.push_back(pReq);

    TINT64 ddwAlid = pstUser->m_tbPlayer.m_nAlpos != EN_ALLIANCE_POS__REQUEST ? pstUser->m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET : 0;

    pReq->udwReqType = 1;
    oss << EN_MAIL_USER_OP__SET_MAIL_STATUS << ',' << pstUser->m_tbPlayer.m_nUid << ',' << ddwAlid
        << ',' << pstUser->m_tbUserStat.m_nBirth_m << ',' << dwStatus << ',' << (TINT32)bIsClear << ',' << pOpMailList;
    pReq->sReqContent = oss.str();
    return 0;
}

TINT32 CReportSvrRequest::SetMailRewardCollect(SSession* pstSession, TINT64 ddwMailId)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;

    ostringstream oss;
    oss.str("");

    ReportReqInfo *pReq = new ReportReqInfo;
    pstSession->m_vecReportReq.push_back(pReq);

    TINT64 ddwAlid = pstUser->m_tbPlayer.m_nAlpos != EN_ALLIANCE_POS__REQUEST ? pstUser->m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET : 0;

    pReq->udwReqType = 1;
    oss << EN_MAIL_USER_OP__MAIL_REWARDCOLLECT << ',' << pstUser->m_tbPlayer.m_nUid << ',' << ddwAlid
        << ',' << pstUser->m_tbUserStat.m_nBirth_m << ',' << ddwMailId;
    pReq->sReqContent = oss.str();
    return 0;
}

TINT32 CReportSvrRequest::SetMailRewardCollect(SSession* pstSession, TbPlayer *ptbPlayer, TbUser_stat *ptbUserStat, TINT64 ddwMailId)
{
    ostringstream oss;
    oss.str("");

    ReportReqInfo *pReq = new ReportReqInfo;
    pstSession->m_vecReportReq.push_back(pReq);

    TINT64 ddwAlid = ptbPlayer->m_nAlpos != EN_ALLIANCE_POS__REQUEST ? ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET : 0;

    pReq->udwReqType = 1;
    oss << EN_MAIL_USER_OP__MAIL_REWARDCOLLECT << ',' << ptbPlayer->m_nUid << ',' << ddwAlid
        << ',' << ptbUserStat->m_nBirth_m << ',' << ddwMailId;
    pReq->sReqContent = oss.str();
    return 0;
}

TINT32 CReportSvrRequest::MailUserPut(SSession* pstSession, TbMail_user *atbMailUser, TUINT32 udwMailUserNum)
{
    ostringstream oss;
    oss.str("");

    ReportReqInfo *pReq = new ReportReqInfo;
    pstSession->m_vecReportReq.push_back(pReq);

    pReq->udwReqType = 1;
    oss << EN_MAIL_USER_OP__MAIL_SEND_BY_ID;
    for (TUINT32 udwIdx = 0; udwIdx < udwMailUserNum; udwIdx++)
    {
        oss << ',' << atbMailUser[udwIdx].m_nUid << ':' << atbMailUser[udwIdx].m_nMid << ':' << atbMailUser[udwIdx].m_nSuid
            << ':' << atbMailUser[udwIdx].m_nTuid << ':' << atbMailUser[udwIdx].m_nTime << ':' << atbMailUser[udwIdx].m_nStatus
            << ':' << atbMailUser[udwIdx].m_nDisplay_type << ':' << atbMailUser[udwIdx].m_nReceiver_avatar
            << ':' << atbMailUser[udwIdx].m_sReceiver_name << ':' << atbMailUser[udwIdx].m_sReceiver_alnick
            << ':' << atbMailUser[udwIdx].m_nHas_reward << ':' << atbMailUser[udwIdx].m_nIs_al
            << ':' << atbMailUser[udwIdx].m_nIs_single_svr << ':' << atbMailUser[udwIdx].m_nSid
            << ':' << atbMailUser[udwIdx].m_sPlatform << ':' << atbMailUser[udwIdx].m_sVersion << ':';
    }

    pReq->sReqContent = oss.str();
    return 0;
}

TINT32 CReportSvrRequest::MailUserDelete(SSession* pstSession, TINT64 ddwUid, TINT64 ddwMid)
{
    ostringstream oss;
    oss.str("");

    ReportReqInfo *pReq = new ReportReqInfo;
    pstSession->m_vecReportReq.push_back(pReq);

    pReq->udwReqType = 1;
    oss << EN_MAIL_USER_OP__DELETE_MAIL_USER << ',' << ddwUid << ',' << ddwMid;
    pReq->sReqContent = oss.str();
    return 0;
}