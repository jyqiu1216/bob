#include "aws_request.h"
#include "session.h"
#include "aws_table_include.h"
#include "game_info.h"
#include "init_logic.h"
#include "quest_logic.h"
#include "buffer_base.h"
#include "game_command.h"
#include "activities_logic.h"
#include "common_func.h"
#include "bounty_logic.h"
#include "common_logic.h"
#include "sendmessage_base.h"
#include "game_svr.h"
#include "tool_base.h"
#include "common_base.h"
#include "city_base.h"
#include "action_base.h"

void CAwsRequest::UIntToString(unsigned int key, char*pstr)
{
	const char* pVal = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	char szTmp[20];
	int dwByteNum = 0;
	unsigned int udwIndex = 0;

	while(key)
	{
		udwIndex = key%36;
		szTmp[dwByteNum++] = pVal[udwIndex];
		key = key/36;
	}

	for(int idx = 0; idx < dwByteNum; idx++)
	{
		pstr[dwByteNum-idx-1] = szTmp[idx];
	}
	pstr[dwByteNum] = 0;
}

int CAwsRequest::UpdateItem(SSession *pstSession, AwsTable* pAwsTbl, const ExpectedDesc& expected_desc, int dwReturnValuesType, bool bNeedUnLock)
{
    vector<AwsReqInfo*>& vecReq = pstSession->m_vecAwsReq;
    AwsReqInfo* pAwsReq = new AwsReqInfo(pAwsTbl, pAwsTbl->GetTableName(), "UpdateItem");
    if (bNeedUnLock)
    {
        pAwsReq->dwUnlock = true;
        pAwsReq->bIsDefaultLock = false;
    }
    pAwsTbl->OnUpdateItemReq(pAwsReq->sReqContent, expected_desc, dwReturnValuesType);
    if(pAwsReq->sReqContent.empty()) //不需要更新
    {
        delete pAwsReq;
        return 1;
    }
    vecReq.push_back(pAwsReq);
    return 0;
}

int CAwsRequest::DeleteItem(SSession *pstSession, AwsTable* pAwsTbl, const ExpectedDesc& expected_desc, int dwReturnValuesType)
{
    vector<AwsReqInfo*>& vecReq = pstSession->m_vecAwsReq;
    AwsReqInfo* pAwsReq = new AwsReqInfo(pAwsTbl, pAwsTbl->GetTableName(), "DeleteItem");
    pAwsTbl->OnDeleteItemReq(pAwsReq->sReqContent, expected_desc, dwReturnValuesType);
    vecReq.push_back(pAwsReq);
    return 0;
}

int CAwsRequest::PutItem(SSession *pstSession, AwsTable* pAwsTbl)
{
    vector<AwsReqInfo*>& vecReq = pstSession->m_vecAwsReq;
    AwsReqInfo* pAwsReq = new AwsReqInfo(pAwsTbl, pAwsTbl->GetTableName(), "PutItem");
    pAwsTbl->OnPutItemReq(pAwsReq->sReqContent);
    vecReq.push_back(pAwsReq);
    return 0;
}

int CAwsRequest::GetItem(SSession *pstSession, AwsTable* pAwsTbl, unsigned int udwIdxNo, bool bConsistentRead, bool bNeedLock)
{
    vector<AwsReqInfo*>& vecReq = pstSession->m_vecAwsReq;
    AwsReqInfo* pAwsReq = new AwsReqInfo(pAwsTbl, pAwsTbl->GetTableName(), "GetItem", udwIdxNo);
    if (bNeedLock)
    {
        pAwsReq->dwLock = true;
        pAwsReq->bIsDefaultLock = false;
    }
    pAwsTbl->OnGetItemReq(pAwsReq->sReqContent, udwIdxNo, bConsistentRead);
    vecReq.push_back(pAwsReq);
    return 0;
}


int CAwsRequest::Query(SSession *pstSession, AwsTable* pAwsTbl, unsigned int udwIdxNo, const CompareDesc& comp_desc, bool bConsistentRead,
    bool bReturnConsumedCapacity, bool bScanIndexForward, unsigned int dwLimit)
{
    vector<AwsReqInfo*>& vecReq = pstSession->m_vecAwsReq;
    AwsReqInfo* pAwsReq = new AwsReqInfo(pAwsTbl, pAwsTbl->GetTableName(), "Query", udwIdxNo);
    pAwsTbl->OnQueryReq(pAwsReq->sReqContent, udwIdxNo, comp_desc,
        bConsistentRead, bReturnConsumedCapacity, bScanIndexForward, dwLimit);
    vecReq.push_back(pAwsReq);
    return 0;
}


int CAwsRequest::QueryCount(SSession *pstSession, AwsTable* pAwsTbl, unsigned int udwIdxNo,
    const CompareDesc& comp_desc, bool bConsistentRead, bool bReturnConsumedCapacity, bool bScanIndexForward, unsigned int dwLimit)
{
    vector<AwsReqInfo*>& vecReq = pstSession->m_vecAwsReq;
    AwsReqInfo* pAwsReq = new AwsReqInfo(pAwsTbl, pAwsTbl->GetTableName(), "Query", udwIdxNo);
    pAwsTbl->OnCountReq(pAwsReq->sReqContent, udwIdxNo, comp_desc, bConsistentRead, bReturnConsumedCapacity, bScanIndexForward, dwLimit);
    vecReq.push_back(pAwsReq);
    return 0;
}

int CAwsRequest::Scan(SSession *pstSession, AwsTable* pAwsTbl, unsigned int udwIdxNo, 
    bool bHasStartKey, bool bReturnConsumedCapacity, unsigned int dwLimit, unsigned int dwSegment, unsigned int dwTotalSegments)
{
    vector<AwsReqInfo*>& vecReq = pstSession->m_vecAwsReq;
    AwsReqInfo* pAwsReq = new AwsReqInfo(pAwsTbl, pAwsTbl->GetTableName(), "Scan", udwIdxNo);
    pAwsTbl->OnScanReq(pAwsReq->sReqContent, udwIdxNo,
        bHasStartKey, bReturnConsumedCapacity, dwLimit, dwSegment, dwTotalSegments);
    vecReq.push_back(pAwsReq);
    return 0;
}



int CAwsRequest::BatchGetItem(SSession *pstSession, AwsTable* pAwsTbl, unsigned int udwObjSize, unsigned int udwObjNum, unsigned int udwIdxNo)
{
    if(udwObjNum == 0)
    {
        return 0;
    }
    vector<AwsReqInfo*>& vecReq = pstSession->m_vecAwsReq;
    IndexDesc& idx_desc = pAwsTbl->pTableDesc->mIndexDesc[udwIdxNo];
    assert(idx_desc.sName == "PRIMARY"); //只能通过主键查询

    AwsMap BatchReadItem;
    for(unsigned int i = 0; i < udwObjNum; ++i)
    {
        pAwsTbl[i].OnReadItemReq(&BatchReadItem);
    }
    AwsMap *pReadItem = BatchReadItem.GetAwsMap("RequestItems")->GetAwsMap(pAwsTbl->GetTableName());
    ostringstream oss;
    for(unsigned int i = 0; i < idx_desc.vecRtnFld.size(); ++i)
    {
        oss.str("");
        oss << "/AttributesToGet[" << i << "]";
        FieldDesc& fld_desc = pAwsTbl->pTableDesc->mFieldDesc[idx_desc.vecRtnFld[i]];
        pReadItem->AddValue(oss.str(), fld_desc.sName);
    }
    pReadItem->AddBoolean("/ConsistentRead", true);
    BatchReadItem.AddValue("/ReturnConsumedCapacity", "TOTAL");
    oss.str("");
    BatchReadItem.Dump(oss);
    AwsReqInfo* pAwsReq = new AwsReqInfo(pAwsTbl, pAwsTbl->GetTableName(), "BatchGetItem", udwIdxNo, oss.str());
    vecReq.push_back(pAwsReq);
    return 0;
}

int CAwsRequest::LoginGetByUid(SSession *pstSession, TUINT32 udwUid)
{
    TbLogin tbLoginItem;
    tbLoginItem.Set_Uid(udwUid);
    return GetItem(pstSession, &tbLoginItem, ETbLOGIN_OPEN_TYPE_PRIMARY);
}

int CAwsRequest::UserGetByUid(SSession *pstSession, TUINT32 udwUid)
{
    TbPlayer tbPlayer;
    tbPlayer.Set_Uid(udwUid);
    return GetItem(pstSession, &tbPlayer, ETbPLAYER_OPEN_TYPE_PRIMARY);
}

int  CAwsRequest::CityGetByUid(SSession *pstSession, TUINT32 udwUid)
{
    TbCity tbCity;
    tbCity.Set_Uid(udwUid);
    return GetItem(pstSession, &tbCity, ETbCITY_OPEN_TYPE_PRIMARY);
}

int CAwsRequest::UserStatGet(SSession *pstSession, TUINT32 udwUid)
{
    TbUser_stat tbUser_stat;
    tbUser_stat.Set_Uid(udwUid);
    return GetItem(pstSession, &tbUser_stat, ETbUSERSTAT_OPEN_TYPE_PRIMARY);
}

int CAwsRequest::BlackAccountGet(SSession *pstSession, TINT64 ddwUid)
{
    TbBlack_account tbBlack_account;
    tbBlack_account.Set_Key(CCommonFunc::NumToString(ddwUid));
    return GetItem(pstSession, &tbBlack_account, ETbBLACKACCOUNT_OPEN_TYPE_PRIMARY);
}

int CAwsRequest::BlackAccountGet(SSession *pstSession, const string& strDid)
{
    TbBlack_account tbBlack_account;
    tbBlack_account.Set_Key(strDid);
    return GetItem(pstSession, &tbBlack_account, ETbBLACKACCOUNT_OPEN_TYPE_PRIMARY);
}

int CAwsRequest::BookmarkQuery(SSession *pstSession, TUINT32 udwUid)
{
    TbBookmark tbBookmark;
    CompareDesc comDesc;
    comDesc.dwCompareType = COMPARE_TYPE_GT;
    tbBookmark.Set_Uid(pstSession->m_stReqParam.m_udwUserId);
    tbBookmark.Set_Pos(0);
    return CAwsRequest::Query(pstSession, &tbBookmark, ETbBOOKMARK_OPEN_TYPE_PRIMARY, comDesc);
}

int  CAwsRequest::MapQueryByUid(SSession *pstSession, TUINT32 udwSvrId, TUINT32 udwUid)
{
    TbMap tbMap;
    tbMap.Set_Sid(udwSvrId);
    tbMap.Set_Uid(udwUid);
    CompareDesc comp_desc;
    comp_desc.dwCompareType = COMPARE_TYPE_EQ;
    return Query(pstSession, &tbMap, ETbMAP_OPEN_TYPE_GLB_UID, comp_desc, false);
}

int  CAwsRequest::MapQueryByBid(SSession *pstSession, TUINT32 udwSvrId, TUINT32 udwBid)
{
    TbMap tbMap;
    tbMap.Set_Sid(udwSvrId);
    tbMap.Set_Bid(udwBid);
    CompareDesc comp_desc;
    comp_desc.dwCompareType = COMPARE_TYPE_EQ;
    return Query(pstSession, &tbMap, ETbMAP_OPEN_TYPE_GLB_BID, comp_desc, false);
}

int CAwsRequest::AllianceGetByAid(SSession *pstSession,  TUINT32 udwAid)
{
    TbAlliance tbAlliance;
    tbAlliance.Set_Aid(udwAid);
    return GetItem(pstSession, &tbAlliance, ETbALLIANCE_OPEN_TYPE_PRIMARY);
}

int CAwsRequest::AllianceGetByName(SSession *pstSession, const string& strName)
{
    TbUnique_name tbAllianceName;
    tbAllianceName.Set_Type(EN_ALLIANCE_NAME);
    tbAllianceName.Set_Name(CToolBase::ToLower(strName));

    return GetItem(pstSession, &tbAllianceName, ETbUNIQUE_NAME_OPEN_TYPE_PRIMARY);
}

int CAwsRequest::AlHelpHistoryQuery(SSession *pstSession, TUINT32 udwUid)
{
    TbAl_help tbAl_help;
    tbAl_help.Set_Uid(udwUid);
    tbAl_help.Set_Idx(0);
    CompareDesc comp_desc;
    comp_desc.dwCompareType = COMPARE_TYPE_GE;
    return Query(pstSession, &tbAl_help, ETbALHELP_OPEN_TYPE_PRIMARY,  comp_desc);
}

int CAwsRequest::CanHelpTaskQuery(SSession *pstSession,TUINT32 udwAid, TUINT32 udwLimit)
{
    TbAlliance_action tbAction;
    tbAction.Set_Sal(udwAid);
    CompareDesc comp_desc;
    comp_desc.dwCompareType = COMPARE_TYPE_EQ;
    return Query(pstSession, &tbAction, ETbALLIANCEACTION_OPEN_TYPE_GLB_SAL, CompareDesc(), false, true, true, udwLimit);
}

int CAwsRequest::AlAssistQuery(SSession *pstSession, TUINT32 udwAid)
{
    TbAl_assist tbAssist;
    tbAssist.Set_Aid(udwAid);
    tbAssist.Set_Id(0);
    CompareDesc comp_desc;
    comp_desc.dwCompareType = COMPARE_TYPE_GT;
    return Query(pstSession, &tbAssist, ETbALASSIST_OPEN_TYPE_PRIMARY, comp_desc, TRUE, TRUE, FALSE, MAX_AL_ASSIST_NUM - 1);
}

int CAwsRequest::PlayerAssistQuery(SSession *pstSession,  TUINT32 udwAid,TUINT32 udwUid, TUINT32 udwLimit)
{
    CompareDesc comp_desc;
    comp_desc.dwCompareType = COMPARE_TYPE_BETWEEN;
    comp_desc.vecN.push_back(udwUid * 10);
    comp_desc.vecN.push_back(udwUid * 10 + EN_ASSIST_TYPE__END);

    TbAl_assist tbAssist;
    tbAssist.Set_Aid(udwAid);
    return CAwsRequest::Query(pstSession, &tbAssist, ETbALASSIST_OPEN_TYPE_PRIMARY, comp_desc, true, true, false, udwLimit);
}

int CAwsRequest::AlRequestConutQuery(SSession *pstSession, TINT32 dwAid)
{
    TbAl_member tbAlmember;
    tbAlmember.Set_Aid(dwAid * -1);
    CompareDesc comp_desc;
    comp_desc.dwCompareType = COMPARE_TYPE_GT;
    return QueryCount(pstSession, &tbAlmember, ETbALLIANCEMEMBER_OPEN_TYPE_PRIMARY, comp_desc, TRUE, TRUE, FALSE, MAX_ALLIANCE_REQUEST_NUM);
}

int CAwsRequest::AllAlMemberQuery(SSession *pstSession, TINT32 dwAid)
{
    TbAl_member tbAlmember;
    tbAlmember.Set_Aid(dwAid);
    CompareDesc comp_desc;
    comp_desc.dwCompareType = COMPARE_TYPE_GT;
    return Query(pstSession, &tbAlmember, ETbALLIANCEMEMBER_OPEN_TYPE_PRIMARY, comp_desc, TRUE, TRUE, FALSE, 1000);
}

int CAwsRequest::DiplomacyQuery(SSession *pstSession, TUINT32 udwAid)
{
    if(udwAid == 0)
    {
        return -1;
    }
    TbDiplomacy tbDiplomacy;

    tbDiplomacy.Set_Src_al(udwAid);
    tbDiplomacy.Set_Des_al(0);
    CompareDesc comp_desc;
    comp_desc.dwCompareType = COMPARE_TYPE_GT;

    return Query(pstSession, &tbDiplomacy, ETbDIPLOMACY_OPEN_TYPE_PRIMARY, comp_desc);
}

int CAwsRequest::WallMsgCntQuery(SSession *pstSession, TUINT32 udwSvrId, TUINT32 udwAid)
{
    TbAl_wall tbWall;
    tbWall.Set_Alid(udwAid);
    CompareDesc comp_desc;
    comp_desc.dwCompareType = COMPARE_TYPE_BETWEEN;
    TINT64 ddwSeq = pstSession->m_stUserInfo.m_tbUserStat.m_nWall_get_t << 32;
    comp_desc.push_back(ddwSeq);
    ddwSeq = (TINT64)CTimeUtils::GetUnixTime() << 32;
    comp_desc.push_back(ddwSeq);
    
    return QueryCount(pstSession, &tbWall, ETbALWALL_OPEN_TYPE_COUNT, comp_desc, TRUE, TRUE, FALSE, MAX_WALL_MSG_NUM);
}

int CAwsRequest::TopWallMsgCntQuery(SSession *pstSession, TUINT32 udwSvrId, TUINT32 udwAid)
{
    TbAl_wall tbWall;
    tbWall.Set_Alid(udwAid);
    CompareDesc comp_desc;
    comp_desc.dwCompareType = COMPARE_TYPE_BETWEEN;
    TINT64 ddwSeq = (pstSession->m_stUserInfo.m_tbUserStat.m_nWall_get_t + WALL_MSG_OFFSET) << 32;
    comp_desc.push_back(ddwSeq);
    ddwSeq = (CTimeUtils::GetUnixTime() + WALL_MSG_OFFSET) << 32;
    comp_desc.push_back(ddwSeq);

    return QueryCount(pstSession, &tbWall, ETbALWALL_OPEN_TYPE_COUNT, comp_desc, TRUE, TRUE, FALSE, MAX_WALL_MSG_NUM);
}

int CAwsRequest::AlConsumeHistoryQuery(SSession *pstSession, TUINT32 udwAid, TUINT32 udwLimit)
{
    TbAl_store_consume tbAlConsume;
    tbAlConsume.Set_Aid(udwAid);
    tbAlConsume.Set_Id(0);
    CompareDesc comp_desc;
    comp_desc.dwCompareType = COMPARE_TYPE_GT;
    return Query(pstSession, &tbAlConsume, ETbALSTORECONSUME_OPEN_TYPE_PRIMARY, comp_desc, TRUE, TRUE, FALSE, udwLimit);
}

int CAwsRequest::TipsQuery(SSession *pstSession, TINT64 ddwUid, TUINT32 udwLimit)
{
    TbUser_stat &tbStat = pstSession->m_stUserInfo.m_tbUserStat;
    TINT64 ddwCurTime = CTimeUtils::GetCurTimeUs() - 40 * 1000 * 1000;
    if(ddwCurTime < tbStat.m_nTips)
    {
        ddwCurTime = tbStat.m_nTips;
    }

    CompareDesc comp_desc;
    comp_desc.dwCompareType = COMPARE_TYPE_GT;

    TbTips tbTips;
    tbTips.Set_Uid(ddwUid);
    tbTips.Set_Time(ddwCurTime);

    return Query(pstSession, &tbTips, ETbTIPS_OPEN_TYPE_PRIMARY, comp_desc, true, true, true, udwLimit);
}


int CAwsRequest::EventTipsQuery(SSession *pstSession, TINT64 ddwUid, TUINT32 udwLimit)
{
    TbUser_stat &tbStat = pstSession->m_stUserInfo.m_tbUserStat;
    TUINT64 uddwLastTipsId = tbStat.m_nLast_event_win_id;
    CompareDesc comp_desc;
    comp_desc.dwCompareType = COMPARE_TYPE_GT;

    TbEvent_tips tbEventTips;
    tbEventTips.Set_Uid(ddwUid);
    tbEventTips.Set_Id(uddwLastTipsId);

    return Query(pstSession, &tbEventTips, ETbEVENTTIPS_OPEN_TYPE_PRIMARY, comp_desc, true, true, true, udwLimit);
}


int CAwsRequest::AlEventTipsQuery(SSession *pstSession, TINT64 ddwAlid, TUINT32 udwLimit)
{
    TbUser_stat &tbStat = pstSession->m_stUserInfo.m_tbUserStat;
    TUINT64 uddwAlTipsTime = tbStat.m_nAl_event_tips_time;
    CompareDesc comp_desc;
    comp_desc.dwCompareType = COMPARE_TYPE_GT;

    TbEvent_tips tbEventTips;
    tbEventTips.Set_Uid(-1 * ddwAlid);
    tbEventTips.Set_Time(uddwAlTipsTime);

    return Query(pstSession, &tbEventTips, ETbEVENTTIPS_OPEN_TYPE_TIME, comp_desc, true, true, true, udwLimit);
}

int CAwsRequest::AllianceWallQuery(SSession *pstSession, TUINT32 udwAlId)
{
    TbAl_wall tbWall;
    tbWall.Set_Alid(udwAlId);
    tbWall.Set_Wall_id(0);
    CompareDesc comp_desc;
    comp_desc.dwCompareType = COMPARE_TYPE_GT;
    return Query(pstSession, &tbWall, ETbALWALL_OPEN_TYPE_PRIMARY, comp_desc, TRUE, TRUE, FALSE, MAX_WALL_MSG_NUM);
}

int CAwsRequest::QuestGetByUid(SSession *pstSession, TUINT32 udwUid)
{
    TbQuest tbQuest;
    tbQuest.Set_Uid(udwUid);
    return GetItem(pstSession, &tbQuest, ETbQUEST_OPEN_TYPE_PRIMARY);
}

int CAwsRequest::BountyGetByUid(SSession *pstSession, TUINT32 udwUid)
{
    TbBounty tbBounty;
    tbBounty.Set_Uid(udwUid);
    return GetItem(pstSession, &tbBounty, ETbBOUNTY_OPEN_TYPE_PRIMARY);
}

int CAwsRequest::GetGlobalNewId(SSession *pstSession, TUINT32 udwKey, TINT32 dwRetType)
{
    TbParam tbParam;
    if(udwKey == EN_GLOBAL_PARAM__USER_ID
    || udwKey == EN_GLOBAL_PARAM__MAIL_ID
    || udwKey == EN_GLOBAL_PARAM__REPORT_ID
    || udwKey == EN_GLOBAL_PARAM__ALLIANCE_ID
    || udwKey == EN_GLOBAL_PARAM__HELP_BUBBLE_TIME)
    {
        tbParam.Set_Key(udwKey);
    }
    else
    {
        tbParam.Set_Key(((1L * pstSession->m_stReqParam.m_udwSvrId) << 32) + udwKey);
    }
    tbParam.Set_Val(1, UPDATE_ACTION_TYPE_ADD); //加1并返回
    return CAwsRequest::UpdateItem(pstSession, &tbParam, ExpectedDesc(), dwRetType);
}

int CAwsRequest::UserReportPut(SSession *pstSession, TINT64 ddwUid, TbReport* ptbReport, TUINT8 ucType)
{
    TbReport_user tbReportUserItem;
    tbReportUserItem.Set_Uid(ddwUid);
    tbReportUserItem.Set_Rid(ptbReport->m_nId);
    tbReportUserItem.Set_Type(ucType);
    tbReportUserItem.Set_Report_type(ptbReport->m_nType);
    tbReportUserItem.Set_Time(ptbReport->m_nTime);
    return CAwsRequest::PutItem(pstSession, &tbReportUserItem);
}

int CAwsRequest::BackPackGet(SSession *pstSession, TINT64 ddwUid)
{
    TbBackpack tbBackpack;
    tbBackpack.Set_Uid(ddwUid);
    return GetItem(pstSession, &tbBackpack, ETbBackpack_OPEN_TYPE_PRIMARY, pstSession->m_stReqParam.m_udwSvrId);
}

int CAwsRequest::MailOperateHistoryQuery(SSession *pstSession, TUINT32 udwUid)
{
    TbMail_operate tbMailOperate;
    tbMailOperate.Set_Uid(udwUid);
    return GetItem(pstSession, &tbMailOperate, ETbMAILOPERATE_OPEN_TYPE_PRIMARY);
}

int CAwsRequest::MapGet(SSession *pstSession, TUINT32 udwMapId)
{
    TbMap tbMapItem;
    tbMapItem.Set_Sid(pstSession->m_stReqParam.m_udwSvrId);
    tbMapItem.Set_Id(udwMapId);
    return GetItem(pstSession, &tbMapItem, ETbMAP_OPEN_TYPE_PRIMARY);
}

int CAwsRequest::MapGetByIdAndSid(SSession *pstSession, TUINT32 udwMapId,TUINT32 udwSid)
{
    TbMap tbMapItem;
    tbMapItem.Set_Sid(udwSid);
    tbMapItem.Set_Id(udwMapId);
    return GetItem(pstSession, &tbMapItem, ETbMAP_OPEN_TYPE_PRIMARY);
}

int CAwsRequest::BufferActionQueryBySuid(SSession *pstSession, TUINT32 udwUid)
{
    TbAction tbAction;
    tbAction.Set_Suid(udwUid);
    tbAction.Set_Id(0);
    CompareDesc comp_desc;
    comp_desc.dwCompareType = COMPARE_TYPE_GT;
    return Query(pstSession, &tbAction, ETbACTION_OPEN_TYPE_PRIMARY, comp_desc);
}

int CAwsRequest::AlActionQueryBySuid(SSession *pstSession, TUINT32 udwUid)
{
    CompareDesc comp_desc;
    comp_desc.dwCompareType = COMPARE_TYPE_GT;
    TbAlliance_action tbAction;
    tbAction.Set_Suid(udwUid);
    tbAction.Set_Id(0);
    return Query(pstSession, &tbAction, ETbALLIANCEACTION_OPEN_TYPE_PRIMARY, comp_desc);
}

int CAwsRequest::MarchActionQueryBySuid(SSession *pstSession, TUINT32 udwSid,TUINT32 udwUid)
{
    CompareDesc comp_desc;
    comp_desc.dwCompareType = COMPARE_TYPE_GT;
    TbMarch_action tbAction;
    tbAction.Set_Suid(udwUid);
    tbAction.Set_Id(0);
    return Query(pstSession, &tbAction, ETbMARCH_OPEN_TYPE_PRIMARY, comp_desc);
}

int CAwsRequest::MarchActionQueryByTuid(SSession *pstSession, TUINT32 udwSid, TUINT32 udwUid)
{
    TINT64 ddwKey = udwUid;
    TbMarch_action tbAction;
    tbAction.Set_Tal(-1 * ddwKey);
    return Query(pstSession, &tbAction, ETbMARCH_OPEN_TYPE_GLB_TUID, CompareDesc(), false);
}

int CAwsRequest::MarchActionQueryBySal(SSession *pstSession, TUINT32 udwSid, TUINT32 udwAid)
{
    TINT64 ddwKey = udwAid;
    TbMarch_action tbAction;
    tbAction.Set_Sal(ddwKey);
    return Query(pstSession, &tbAction, ETbMARCH_OPEN_TYPE_GLB_SAL, CompareDesc(), false);
}

int CAwsRequest::MarchActionQueryByTal(SSession *pstSession, TUINT32 udwSid, TUINT32 udwAid)
{
    TINT64 ddwKey = udwAid;
    TbMarch_action tbAction;
    tbAction.Set_Tal(ddwKey);
    return Query(pstSession, &tbAction, ETbMARCH_OPEN_TYPE_GLB_TAL, CompareDesc(), false);
}

int CAwsRequest::AttackThroneMarchQuery(SSession *pstSession, TINT64 ddwSid, TINT64 ddwPos)
{
    TINT64 ddwKey = CActionBase::GenThroneTargetId(ddwSid, ddwPos);
    TbMarch_action tbAction;
    tbAction.Set_Tal(ddwKey);
    return Query(pstSession, &tbAction, ETbMARCH_OPEN_TYPE_GLB_TAL, CompareDesc(), false);
}

int CAwsRequest::UserGetByPlayerName(SSession *pstSession, const string& sUin)
{
    TbUnique_name tbUniqueName;
    tbUniqueName.Set_Name(CToolBase::ToLower(sUin));
    tbUniqueName.Set_Type(EN_PLAYER_NAME);

    return GetItem(pstSession, &tbUniqueName, ETbUNIQUE_NAME_OPEN_TYPE_PRIMARY);
}

int CAwsRequest::AlMemberGet(SSession* pstSession, TUINT32 udwAid, TUINT32 udwUid)
{
    TbAl_member tbAlmember;
    tbAlmember.Set_Aid(udwAid);
    tbAlmember.Set_Uid(udwUid);
    return GetItem(pstSession, &tbAlmember, ETbALLIANCEMEMBER_OPEN_TYPE_PRIMARY);
}

int CAwsRequest::SvrListInfoQuery(SSession* pstSession)
{
    TUINT32 udwSvrNum = CGameSvrInfo::GetInstance()->m_udwSvrNum;
    assert(udwSvrNum > 0);
    pstSession->m_udwTmpSvrNum = udwSvrNum;

    TbSvr_stat tbSvrStat;
    TbMap tbThrone; //王座的map信息
    for(TUINT32 idx = 0; idx < udwSvrNum; idx++)
    {
        //拿到svr_stat表中的信息
        tbSvrStat.Reset();
        tbSvrStat.Set_Sid(idx);
        GetItem(pstSession, &tbSvrStat, ETbSVR_STAT_OPEN_TYPE_PRIMARY);

        //拿到王座的map信息
        tbThrone.Reset();
        tbThrone.Set_Sid(idx);
        tbThrone.Set_Id(THRONE_POS);
        GetItem(pstSession, &tbThrone, ETbMAP_OPEN_TYPE_PRIMARY);
    }

    return 0;
}

int CAwsRequest::BlackListQuery(SSession *pstSession, TUINT32 udwUid)
{
    TbBlacklist tbBlacklist;
    tbBlacklist.Set_Uid(udwUid);
    tbBlacklist.Set_Target_uid(0);

    CompareDesc comp_desc;
    comp_desc.dwCompareType = COMPARE_TYPE_GE;

    return Query(pstSession, &tbBlacklist, ETbBLACKLIST_OPEN_TYPE_PRIMARY, comp_desc);
}

int CAwsRequest::BlackListGet(SSession *pstSession, TUINT32 udwSUid, TUINT32 udwTUid)
{
    TbBlacklist tbBlacklist;
    tbBlacklist.Set_Uid(udwSUid);
    tbBlacklist.Set_Target_uid(udwTUid);

    return GetItem(pstSession, &tbBlacklist, ETbBLACKLIST_OPEN_TYPE_PRIMARY);
}

int  CAwsRequest::EquipQueryByStatus(SSession *pstSession, TUINT32 udwUid, TUINT32 udwStatus)
{
    TbEquip tbEquip;
    tbEquip.Set_Uid(udwUid);
    tbEquip.Set_Status(EN_EQUIPMENT_STATUS_ON_DRAGON);
    return Query(pstSession, &tbEquip, ETbEQUIP_OPEN_TYPE_STATUS);
    return 0;
}

int CAwsRequest::EquipQueryByUid(SSession *pstSession,  TUINT32 udwUid)
{
    TINT64 ddwKey = udwUid;
    CompareDesc comp_desc;
    comp_desc.dwCompareType = COMPARE_TYPE_BETWEEN;
    comp_desc.vecN.push_back(ddwKey << 32);
    comp_desc.vecN.push_back((ddwKey << 32) + (unsigned int)-1);
    TbEquip tbEquip;
    tbEquip.Set_Uid(udwUid);
    return Query(pstSession, &tbEquip, ETbEQUIP_OPEN_TYPE_PRIMARY, comp_desc);
    return 0;
}

int CAwsRequest::AltarHistoryQuery(SSession* pstSession, TUINT32 udwUid)
{
    TbAltar_history tbAltarHistory;
    tbAltarHistory.Set_Uid(udwUid);
    tbAltarHistory.Set_Time(0);

    CompareDesc stCompareInfo;
    stCompareInfo.dwCompareType = COMPARE_TYPE_GT;

    CAwsRequest::Query(pstSession, &tbAltarHistory, ETbRALLYHISTORY_OPEN_TYPE_PRIMARY, stCompareInfo, true, true, false, MAX_RALLY_HISTORY);
    return 0;
}

int CAwsRequest::NewCityIdQuery(SSession *pstSession, TUINT32 udwSvrId, TUINT8 ucProvinceId)
{
    TbMap tbMap;
    tbMap.Set_Sid(udwSvrId);
    TUINT32 udwMinX = (ucProvinceId % 4) * 125 + 1;
    TUINT32 udwMaxX = (ucProvinceId % 4 + 1) * 125;
    TUINT32 udwX = CToolBase::GetRandNumber(udwMinX, udwMaxX);

    TUINT32 udwX2 = udwX == udwMaxX ? udwMaxX - 1 : udwX + 1;

    TUINT32 udwMinY = (ucProvinceId / 4) * 250 + 1;
    TUINT32 udwMaxY = (ucProvinceId / 4 + 1) * 250;

    TUINT32 udwMidY = (udwMaxY + udwMinY) / 2;
    if(rand() % 2 == 0)
    {
        udwMaxY = udwMidY;
    }
    else
    {
        udwMinY = udwMidY;
    }
    CompareDesc comp_desc_one;
    comp_desc_one.dwCompareType = COMPARE_TYPE_BETWEEN;
    if(udwMinY <= udwMaxY)
    {
        comp_desc_one.push_back(MAP_X_Y_POS_COMPUTE_OFFSET * udwX + udwMinY);
        comp_desc_one.push_back(MAP_X_Y_POS_COMPUTE_OFFSET * udwX + udwMaxY);
    }
    else
    {
        comp_desc_one.push_back(MAP_X_Y_POS_COMPUTE_OFFSET * udwX + udwMaxY);
        comp_desc_one.push_back(MAP_X_Y_POS_COMPUTE_OFFSET * udwX + udwMinY);
    }
    Query(pstSession, &tbMap, ETbMAP_OPEN_TYPE_GLB_ID, comp_desc_one, false, true, true, MAX_WILD_RETURN_NUM / 2);

    CompareDesc comp_desc_two;
    comp_desc_two.dwCompareType = COMPARE_TYPE_BETWEEN;
    if(udwMinY <= udwMaxY)
    {
        comp_desc_two.push_back(MAP_X_Y_POS_COMPUTE_OFFSET * udwX2 + udwMinY);
        comp_desc_two.push_back(MAP_X_Y_POS_COMPUTE_OFFSET * udwX2 + udwMaxY);
    }
    else
    {
        comp_desc_two.push_back(MAP_X_Y_POS_COMPUTE_OFFSET * udwX2 + udwMaxY);
        comp_desc_two.push_back(MAP_X_Y_POS_COMPUTE_OFFSET * udwX2 + udwMinY);
    }
    
    Query(pstSession, &tbMap, ETbMAP_OPEN_TYPE_GLB_ID, comp_desc_two, false, true, true, MAX_WILD_RETURN_NUM / 2);

    return 0;
}

int CAwsRequest::GetGlobalId(SSession *pstSession, TUINT32 udwKey)
{
    TbParam tbParam;
    if(udwKey == EN_GLOBAL_PARAM__USER_ID
    || udwKey == EN_GLOBAL_PARAM__MAIL_ID
    || udwKey == EN_GLOBAL_PARAM__REPORT_ID
    || udwKey == EN_GLOBAL_PARAM__ALLIANCE_ID
    || udwKey == EN_GLOBAL_PARAM__HELP_BUBBLE_TIME)
    {
        tbParam.Set_Key(udwKey);
    }
    else
    {
        tbParam.Set_Key(((1L * pstSession->m_stReqParam.m_udwSvrId) << 32) + udwKey);
    }
    return CAwsRequest::GetItem(pstSession, &tbParam, ETbPARAM_OPEN_TYPE_PRIMARY);
}

void CAwsRequest::CreateUserInfo(SSession *pstSession, SUserInfo *pstUser, TUINT32 udwSvrId, unsigned int udwCityId)
{
    // 创建player数据
    CreatePlayerInfo(pstSession, pstUser, udwSvrId, udwCityId);

    // 创建city数据
    AddCity(pstUser, udwSvrId, udwCityId);
}

int CAwsRequest::AddCity(SUserInfo *pstUser, TUINT32 udwSvrId, unsigned int udwCityId)
{
    TCHAR szUIntToString[20];
    // 0. 更新player数据
    pstUser->m_tbPlayer.Set_Cid(udwCityId);

    // 1. 设置city数据
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    SCityInfo& stCityInfo = pstUser->m_stCityInfo;
    TbCity& tbCity = stCityInfo.m_stTblData;
    ostringstream oss;
    oss << CGameInfo::GetInstance()->m_oJsonRoot["game_init_data"]["r"]["a0"].asString().c_str();
    if(pstUser->m_tbLogin.m_nUid)
    {
        UIntToString(pstUser->m_tbLogin.Get_Uid(), szUIntToString);
        oss << szUIntToString;
    }

    tbCity.Reset();
    tbCity.Set_Uid(pstUser->m_tbLogin.m_nUid);
    tbCity.Set_Pos(udwCityId);
    tbCity.Set_Utime(udwCurTime);
    tbCity.Set_Name(oss.str());

    CInitLogic::InitCityInfo(&stCityInfo, pstUser);

    // 设置vip激活状态, 时长24小时
    CCommonBase::AddVipTime(pstUser, &stCityInfo, 24 * 60 * 60);

    pstUser->m_tbQuest.Set_Uid(pstUser->m_tbPlayer.m_nUid);
    pstUser->m_tbTask.Set_Uid(pstUser->m_tbPlayer.m_nUid);

    pstUser->m_tbBounty.Set_Uid(pstUser->m_tbPlayer.m_nUid);

    CBountyLogic::GenBountyInfo(pstUser, &stCityInfo);

    ////刷新玩家的buffer数据
    CBufferBase::ComputeBuffInfo(&stCityInfo, pstUser, NULL, 0, NULL, NULL);

    CQuestLogic::RefreshTimequest(pstUser, &stCityInfo, EN_TIME_QUEST_TYPE_NEW_USER_MISTERY, &pstUser->m_tbQuest.m_bTimer_gift[0]);
    pstUser->m_tbQuest.SetFlag(TbQUEST_FIELD_TIMER_GIFT);


    // CQuestLogic::RefreshTimequest(pstUser, &stCityInfo, EN_TIME_QUEST_TYPE_NEW_USER_DAILY, &pstUser->m_tbQuest.m_bDaily_quest[0], TRUE);
    // pstUser->m_tbQuest.SetFlag(TbQUEST_FIELD_DAILY_QUEST);


    // CQuestLogic::RefreshTimequest(pstUser, &stCityInfo, EN_TIME_QUEST_TYPE_ALLIANCE, &pstUser->m_tbQuest.m_bAl_quest[0]);
    // pstUser->m_tbQuest.SetFlag(TbQUEST_FIELD_AL_QUEST);


    // CQuestLogic::RefreshTimequest(pstUser, &stCityInfo, EN_TIME_QUEST_TYPE_VIP, &pstUser->m_tbQuest.m_bVip_quest[0]);
    // pstUser->m_tbQuest.SetFlag(TbQUEST_FIELD_VIP_QUEST);

    //wave@20160612: 屏蔽bookmark中的王座省会数据
    //CCommonLogic::AddBookMark(pstUser, THRONE_POS, 3, "Throne");

    return 0;
}

void CAwsRequest::CreatePlayerInfo(SSession *pstSession, SUserInfo *pstUser, TUINT32 udwSvrId, TUINT32 udwCityId)
{
    TCHAR szUIntToString[20];
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();

    // 0. 设置player数据
    TbPlayer& tbPlayer = pstUser->m_tbPlayer;

    tbPlayer.Reset();
    tbPlayer.Set_Sid(udwSvrId);
    tbPlayer.Set_Uid(pstUser->m_tbLogin.Get_Uid());
    tbPlayer.Set_Level(1L);

    //玩家名
    ostringstream oss;
    oss << CGameInfo::GetInstance()->m_oJsonRoot["game_init_data"]["r"]["a1"].asString().c_str();
    if(tbPlayer.Get_Uid())
    {
        UIntToString(tbPlayer.Get_Uid(), szUIntToString);
        oss << szUIntToString;
    }
    
    tbPlayer.Set_Uin(oss.str());

    //玩家头像
    TUINT32 udwPlayerUi = 0;
    TUINT32 udwPlayerUiSize = CGameInfo::GetInstance()->m_oJsonRoot["game_init_data"]["r"]["a3"].size();
    if(udwPlayerUiSize != 0)
    {
        TUINT32 udwTmp = pstSession->m_stReqParam.m_uddwDeviceId % udwPlayerUiSize;
        udwPlayerUi = CGameInfo::GetInstance()->m_oJsonRoot["game_init_data"]["r"]["a3"][udwTmp].asUInt();

        TSE_LOG_INFO(pstSession->m_poServLog, ("device=%lu, player_avatar=%u", pstSession->m_stReqParam.m_uddwDeviceId, udwPlayerUi));
    }
    tbPlayer.Set_Avatar(udwPlayerUi);

    TUINT32 udwAge = CGameInfo::GetInstance()->m_oJsonRoot["game_init_data"]["r"]["a6"].asUInt();
    tbPlayer.Set_Age(udwAge);

    tbPlayer.Set_Ctime(udwCurTime);
    tbPlayer.Set_Status(EN_CITY_STATUS__NEW_PROTECTION | EN_CITY_STATUS__NEW_USER_MISTERY); //llt add, 20131212, fake数据也返回新手保护状态
    
    //tbPlayer.m_bEquipment_list.m_udwNum = MAX_EQUIP_ON_PLAYER;
    //for(TUINT32 udwIdx = 0; udwIdx < MAX_EQUIP_ON_PLAYER;++udwIdx)
    //{
    //    tbPlayer.m_bEquipment_list[udwIdx].Reset();
    //}
    //tbPlayer.SetFlag(TbPLAYER_FIELD_EQUIPMENT_LIST);

    TbQuest& tbQuest = pstUser->m_tbQuest;
    tbQuest.Reset();
    tbQuest.Set_Uid(pstUser->m_tbPlayer.m_nUid);

    TbUser_stat &tbUser_stat = pstUser->m_tbUserStat;
    tbUser_stat.Reset();
    tbUser_stat.Set_Uid(pstUser->m_tbPlayer.m_nUid);

    //gamejson数据
    SCityInfo stCityInfo;
    stCityInfo.Reset();
    CInitLogic::InitPlayerInfo(&stCityInfo, pstUser);

    tbPlayer.Set_Cid(udwCityId);

    pstUser->m_tbQuest.Reset();
    pstUser->m_tbQuest.Set_Uid(pstUser->m_tbLogin.Get_Uid());

    pstUser->m_tbBackpack.Set_Uid(pstUser->m_tbLogin.Get_Uid());
    TUINT32 udwEquipGridNum = CGameInfo::GetInstance()->m_oJsonRoot["game_init_data"]["r"]["a5"].asUInt();
    tbUser_stat.Set_Equip_gride(udwEquipGridNum);

    // 1. flag
    pstUser->m_ucPlayerFlag = EN_TABLE_UPDT_FLAG__NEW;
}

void CAwsRequest::CreateFakeUserInfo(SSession *pstSession, SUserInfo *pstUser, TUINT32 udwSvrId, unsigned int udwCityId)
{
    // 创建player数据
    CreateFakePlayerInfo(pstSession, pstUser, udwSvrId, udwCityId);

    // 创建city数据
    AddFakeCity(pstUser, udwSvrId, udwCityId);
}

int CAwsRequest::AddFakeCity(SUserInfo *pstUser, TUINT32 udwSvrId, unsigned int udwCityId)
{
    TCHAR szUIntToString[20];
    // 0. 更新player数据
    pstUser->m_tbPlayer.Set_Cid(udwCityId);

    // 1. 设置city数据
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    SCityInfo& stCityInfo = pstUser->m_stCityInfo;
    TbCity& tbCity = stCityInfo.m_stTblData;

    //拼接城市名
    ostringstream oss;
    oss << CGameInfo::GetInstance()->m_oJsonRoot["game_init_data"]["r"]["a0"].asString().c_str();
    if(pstUser->m_tbLogin.m_nUid)
    {
        UIntToString(pstUser->m_tbLogin.Get_Uid(), szUIntToString);
        oss << szUIntToString;
    }

    tbCity.Reset();
    tbCity.Set_Uid(pstUser->m_tbLogin.m_nUid);
    tbCity.Set_Pos(udwCityId);
    tbCity.Set_Utime(udwCurTime);
    tbCity.Set_Name(oss.str());

    //读取gamejson中配置的数据
    CInitLogic::InitCityInfo(&stCityInfo, pstUser,TRUE);

    CInitLogic::InitPlayerInfo(&stCityInfo, pstUser, TRUE);

    CBountyLogic::GenBountyInfo(pstUser, &stCityInfo);

    //task
    CQuestLogic::GenTaskNormalQuest(pstUser, &stCityInfo);
    CQuestLogic::GenTaskTimeQuest(pstUser, &stCityInfo);

    pstUser->m_tbQuest.Set_Uid(pstUser->m_tbLogin.m_nUid);
    
    return 0;
}

//sid字段的设置??
void CAwsRequest::CreateFakePlayerInfo(SSession *pstSession, SUserInfo *pstUser, TUINT32 udwSvrId, TUINT32 udwCityId)
{
    TCHAR szUIntToString[20];
    // 0. 设置player数据
    TbPlayer& tbPlayer = pstUser->m_tbPlayer;

    //拼接用户名

    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();

    tbPlayer.Reset();
    tbPlayer.Set_Sid(udwSvrId);
    tbPlayer.Set_Uid(pstUser->m_tbLogin.Get_Uid());
    tbPlayer.Set_Level(1L);

    //玩家名
    ostringstream oss;
    oss << CGameInfo::GetInstance()->m_oJsonRoot["game_init_data"]["fake"]["a1"].asString().c_str();
    if(tbPlayer.Get_Uid())
    {
        UIntToString(tbPlayer.Get_Uid(), szUIntToString);
        oss << szUIntToString;
    }
    tbPlayer.Set_Uin(oss.str());

    //玩家头像
    TUINT32 udwPlayerUi = 0;
    TUINT32 udwPlayerUiSize = CGameInfo::GetInstance()->m_oJsonRoot["game_init_data"]["fake"]["a3"].size();
    if(udwPlayerUiSize != 0)
    {
        TUINT32 udwTmp = pstSession->m_stReqParam.m_uddwDeviceId % udwPlayerUiSize;
        udwPlayerUi = CGameInfo::GetInstance()->m_oJsonRoot["game_init_data"]["fake"]["a3"][udwTmp].asUInt();

        TSE_LOG_INFO(pstSession->m_poServLog, ("device=%lu, player_avatar=%u", pstSession->m_stReqParam.m_uddwDeviceId, udwPlayerUi));
    }
    tbPlayer.Set_Avatar(udwPlayerUi);

    tbPlayer.Set_Ctime(udwCurTime);
    tbPlayer.Set_Status(EN_CITY_STATUS__NEW_PROTECTION | EN_CITY_STATUS__NEW_USER_MISTERY); //llt add, 20131212, fake数据也返回新手保护状态

    tbPlayer.Set_Cid(udwCityId);

    //gamejson数据
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    CInitLogic::InitPlayerInfo(pstCity, pstUser, TRUE);
}

int CAwsRequest::LoginCreate(SSession *pstSession, TUINT32 udwUserId)
{
    vector<AwsReqInfo*>& vecReq = pstSession->m_vecAwsReq;
    SReqParam *pstReqParam = &pstSession->m_stReqParam;
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    TbLogin& tbLogin = pstSession->m_stUserInfo.m_tbLogin;
    tbLogin.Set_Sid(pstReqParam->m_udwSvrId);
    tbLogin.Set_Uid(udwUserId);

    tbLogin.Set_Seq(1);
    tbLogin.Set_Did(pstReqParam->m_uddwDeviceId);
    tbLogin.Set_Ctime(udwCurTime);
    tbLogin.Set_Utime(udwCurTime);
    tbLogin.Set_Gem(EN_GEM_REWARD__LOGIN_CREATE);
    tbLogin.Set_Npc(pstReqParam->m_ucIsNpc);
    tbLogin.Set_De(pstReqParam->m_szDevice);
    tbLogin.Set_Idfa(pstReqParam->m_szIdfa);
    // 新玩家的推送开关默认全部打开
    Json::Value jNewApnsSwitch = Json::Value(Json::arrayValue);
	jNewApnsSwitch.append(0);
    // notic_timer(1)
    jNewApnsSwitch.append(2);
    // notic_war(2)
    jNewApnsSwitch.append(2);
    // notic_alliance(3)
    jNewApnsSwitch.append(2);
    // notic_social(4)
    jNewApnsSwitch.append(2);
    // notic_gift(5)
    jNewApnsSwitch.append(2);
    // notic_buff(6)
    jNewApnsSwitch.append(2);
    // notic_special(7)
    jNewApnsSwitch.append(2);
    tbLogin.Set_Apns_switch(jNewApnsSwitch);
    
    tbLogin.Set_Last_lg_time(udwCurTime);
    
    AwsReqInfo* pAwsReq = new AwsReqInfo(&tbLogin, tbLogin.GetTableName(), "PutItem");
    tbLogin.OnPutItemReq(pAwsReq->sReqContent);
    vecReq.push_back(pAwsReq);
    return 0;
}

void CAwsRequest::UpdateUserAccountInfo(SSession *pstSession, TbLogin* pTbLogin, SReqParam *pstReqParam)
{
    if(CToolBase::IsOpCommand(pstReqParam->m_szCommand))
    {
        if (0 == strcmp(pstSession->m_stReqParam.m_szCommand, "operate_gem_recharge"))
        {
            pstSession->m_bAccoutUpt = TRUE;
        }
        return;
    }

    // 2. 更新deviceid
    if(pTbLogin->Get_Did() != (TINT64)pstReqParam->m_uddwDeviceId
    && pstReqParam->m_ucLoginStatus == EN_LOGIN_STATUS__LOGIN)
    {
        pTbLogin->Set_Did(pstReqParam->m_uddwDeviceId);
        pTbLogin->Set_De(pstReqParam->m_szDevice);
    }
    if(strcmp(pTbLogin->m_sDe.c_str(), pstReqParam->m_szDevice) != 0
        && pstReqParam->m_ucLoginStatus == EN_LOGIN_STATUS__LOGIN) // 没有填写device时，进行填写
    {
        pTbLogin->Set_De(pstReqParam->m_szDevice);
    }

    if(strcmp(pTbLogin->m_sIdfa.c_str(), pstReqParam->m_szIdfa) != 0
        && pstReqParam->m_ucLoginStatus == EN_LOGIN_STATUS__LOGIN) // 没有填写idfa时，进行填写
    {
        pTbLogin->Set_Idfa(pstReqParam->m_szIdfa);
    }

    // wave@20140217: 限制login表seq、utime、apns_num的更新频率
    TINT64 ddwCurTime = CTimeUtils::GetUnixTime();
    TUINT32 udwCurDay = ddwCurTime / (24 * 60 * 60);
    TUINT32 udwPlayerUtimeDay = pTbLogin->m_nUtime / (24 * 60 * 60);
    TBOOL bUpdtSeqFlag = FALSE;
    if(pstReqParam->m_ucLoginStatus == EN_LOGIN_STATUS__LOGIN)
    {
        bUpdtSeqFlag = TRUE;
    }
    else if(pstReqParam->m_udwCommandID != EN_CLIENT_REQ_COMMAND__LOGIN_GET)
    {
        bUpdtSeqFlag = TRUE;
    }
    else if(ddwCurTime - pTbLogin->m_nUtime > 60 || udwCurDay != udwPlayerUtimeDay)// wave@20140409：在日期切换时，也应该更新utime，防止因为日期变化不更新发生的vip point计算错误
    {
        bUpdtSeqFlag = TRUE;
    }
    else
    {
        bUpdtSeqFlag = FALSE;
    }
    if(bUpdtSeqFlag == TRUE)
    {
        // 4. seq
        pTbLogin->Set_Seq(pTbLogin->Get_Seq() + 1);

        // 5. uptime
        pTbLogin->Set_Utime(ddwCurTime);

        // 6. updt apns num
        pTbLogin->Set_Apns_num(0);
    }

    // 计算此次操作导致的宝石变化
    TINT64 ddwGemAdd = 0;
    if(pstSession->m_udwCommand != EN_CLIENT_REQ_COMMAND__LOGIN_FAKE &&
        pstSession->m_stCommonResInfo.m_dwRetCode == EN_RET_CODE__SUCCESS)
    {
        ddwGemAdd = pstSession->m_stUserInfo.m_tbLogin.m_nGem - pstSession->m_ddwGemBegin;
        if(ddwGemAdd < 0)
        {
            pstSession->m_stUserInfo.m_tbLogin.Set_Gem_cost(-1 * ddwGemAdd + pstSession->m_stUserInfo.m_tbLogin.Get_Gem_cost());
        }
    }
    pstSession->m_bAccoutUpt = TRUE;
}

int CAwsRequest::BroadcastQuery(SSession *pstSession)
{
    SUserInfo* pstUserInfo = &pstSession->m_stUserInfo;
    TbUser_stat& tbUserStat = pstUserInfo->m_tbUserStat;

    if (tbUserStat.m_nBroadcast_time == 0)
    {
        return 0;
    }

    TbBroadcast tbBroadcast;
    TINT64 ddwQueryTime = tbUserStat.m_nBroadcast_time;

    CompareDesc stCompareInfo;
    stCompareInfo.dwCompareType = COMPARE_TYPE_GT;
    //即时类
    //全服广播
    tbBroadcast.Set_Key(CSendMessageBase::GetBroadcastKeyBySid(pstSession->m_stReqParam.m_udwSvrId));
    tbBroadcast.Set_Ctime(ddwQueryTime);
    CAwsRequest::Query(pstSession, &tbBroadcast, ETbBROADCAST_OPEN_TYPE_PRIMARY, stCompareInfo, TRUE, TRUE, FALSE, MAX_BROADCAST_NUM_EACH_TYPE);

    return 0;
}

int CAwsRequest::TaskGet( SSession *pstSession, TUINT32 udwUid )
{
    TbTask tbTask;
    tbTask.Set_Uid(udwUid);
    return GetItem(pstSession, &tbTask, ETbTASK_OPEN_TYPE_PRIMARY);
}

int CAwsRequest::RewardWindowQuery(SSession *pstSession, TINT32 ddwUid, TINT64 ddwQueryTime)
{
    TbReward_window tbRewardWindow;
    tbRewardWindow.Reset();
    CompareDesc comp_desc;
    comp_desc.dwCompareType = COMPARE_TYPE_GT;

    tbRewardWindow.Set_Uid(ddwUid);
    tbRewardWindow.Set_Id(ddwQueryTime);

    return Query(pstSession, &tbRewardWindow, ETbREWARDWINDOW_OPEN_TYPE_PRIMARY, comp_desc, true, true, true, MAX_REWARD_WINDOW_NUM);
}

int CAwsRequest::RandomRewardQuery(SSession *pstSession, TINT64 ddwUid)
{
    TbRandom_reward tbRandomReward;
    tbRandomReward.Set_Uid(ddwUid);

    CompareDesc comp_desc;
    comp_desc.dwCompareType = COMPARE_TYPE_GE;

    return Query(pstSession, &tbRandomReward, ETbRANDOMREWARD_OPEN_TYPE_PRIMARY, comp_desc, true, true, true, MAX_RANDOM_REWARD_NUM);
}

int CAwsRequest::ReqDataCenterQuestInfo(SSession *pstSession, TUINT32 udwQuestType)
{
    SUserInfo *pstUserInfo = &pstSession->m_stUserInfo;

    TINT32 dwRefreshQuestNum = 0;
    if(EN_TIME_QUEST_TYPE_DAILY == udwQuestType
       && 0 != pstUserInfo->m_dwRefreshDailyQuestNum)
    {
        dwRefreshQuestNum = pstUserInfo->m_dwRefreshDailyQuestNum;
    }
    if(EN_TIME_QUEST_TYPE_ALLIANCE == udwQuestType
       && 0 != pstUserInfo->m_dwRefreshAllianceQuestNum)
    {
        dwRefreshQuestNum = pstUserInfo->m_dwRefreshAllianceQuestNum;
    }
    if(EN_TIME_QUEST_TYPE_VIP == udwQuestType
       && 0 != pstUserInfo->m_dwRefreshVipQuestNum)
    {
        dwRefreshQuestNum = pstUserInfo->m_dwRefreshVipQuestNum;
    }

    if(0 == dwRefreshQuestNum)
    {
        return 0;
    }
    
    DataCenterReqInfo* pstReq = new DataCenterReqInfo;
    pstReq->m_udwType = EN_REFRESH_DATA_TYPE__QUEST;

    Json::Value rDataReqJson = Json::Value(Json::objectValue);
    rDataReqJson["sid"] = pstUserInfo->m_tbLogin.m_nSid;
    rDataReqJson["uid"] = pstUserInfo->m_tbPlayer.m_nUid;
    rDataReqJson["castle_lv"] = CCityBase::GetBuildingLevelByFuncType(&pstUserInfo->m_stCityInfo, EN_BUILDING_TYPE__CASTLE);
    rDataReqJson["request"] = Json::Value(Json::objectValue);
    rDataReqJson["request"]["quest_type"] = udwQuestType;
    rDataReqJson["request"]["quest_num"] = dwRefreshQuestNum;
    rDataReqJson["request"]["refresh_type"] = pstUserInfo->m_dwRefreshpQuesType;
    rDataReqJson["request"]["request_type"] = 0L;

    Json::FastWriter rEventWriter;
    pstReq->m_sReqContent = rEventWriter.write(rDataReqJson);
    
    pstSession->m_vecDataCenterReq.push_back(pstReq);

    
    return 0;
}

int CAwsRequest::DataOutputGet( SSession *pstSession, TUINT32 udwUid )
{
    TbData_output tbOutput;
    tbOutput.Set_Uid(udwUid);
    return GetItem(pstSession, &tbOutput, ETbDATAOUTPUT_OPEN_TYPE_PRIMARY);
}


int CAwsRequest::ThroneGet(SSession *pstSession, TUINT32 udwSid)
{
    TbThrone tbThrone;
    tbThrone.Set_Sid(udwSid);
    tbThrone.Set_Id(0);

    return GetItem(pstSession, &tbThrone, ETbTHRONE_OPEN_TYPE_PRIMARY);
}

int CAwsRequest::IdolQuery(SSession *pstSession, TUINT32 udwSid)
{
    TbIdol tbIdol;
    tbIdol.Set_Sid(udwSid);
    tbIdol.Set_Id(0);

    CompareDesc comp;
    comp.dwCompareType = COMPARE_TYPE_GE;

    return Query(pstSession, &tbIdol, ETbIDOL_OPEN_TYPE_PRIMARY, comp);
}

int CAwsRequest::TitleQuery(SSession *pstSession, TUINT32 udwSid)
{
    CompareDesc stCompareInfo;
    stCompareInfo.dwCompareType = COMPARE_TYPE_GE;
    TbTitle tbTitle;
    tbTitle.Set_Sid(udwSid);
    tbTitle.Set_Id(0);

    CAwsRequest::Query(pstSession, &tbTitle, ETbTITLE_OPEN_TYPE_PRIMARY, stCompareInfo);
    return 0;
}