#include "aws_request.h"
#include "session.h"
#include "tool_base.h"
#include "tbl_comm.h"

void CAwsRequest::UIntToString(unsigned int key, char*pstr)
{
    const char* pVal = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    char szTmp[20];
    int dwByteNum = 0;
    unsigned int udwIndex = 0;

    while(key)
    {
        udwIndex = key % 36;
        szTmp[dwByteNum++] = pVal[udwIndex];
        key = key / 36;
    }

    for(int idx = 0; idx < dwByteNum; idx++)
    {
        pstr[dwByteNum - idx - 1] = szTmp[idx];
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
    pAwsTbl->OnQueryReq(pAwsReq->sReqContent, udwIdxNo, comp_desc, bConsistentRead, bReturnConsumedCapacity, bScanIndexForward, dwLimit);
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

    AwsMap BatchGetItem;
    for(unsigned int i = 0; i < udwObjNum; ++i)
    {
        pAwsTbl[i].OnReadItemReq(&BatchGetItem);
    }
    AwsMap *pReadItem = BatchGetItem.GetAwsMap("RequestItems")->GetAwsMap(pAwsTbl->GetTableName());
    ostringstream oss;
    for(unsigned int i = 0; i < idx_desc.vecRtnFld.size(); ++i)
    {
        oss.str("");
        oss << "/AttributesToGet[" << i << "]";
        FieldDesc& fld_desc = pAwsTbl->pTableDesc->mFieldDesc[idx_desc.vecRtnFld[i]];
        pReadItem->AddValue(oss.str(), fld_desc.sName);
    }
    pReadItem->AddBoolean("/ConsistentRead", true);
    BatchGetItem.AddValue("/ReturnConsumedCapacity", "TOTAL");
    oss.str("");
    BatchGetItem.Dump(oss);
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

int CAwsRequest::GlobalParamNewIdUpdtAndGet(SSession *pstSession, TUINT32 udwSvrId, TUINT32 udwValueAdd, TUINT32 udwKey, TINT32 dwRetType)
{
    TbParam tbParam;
    TINT64 ddwRealKey = CTableComm::TblParam_GetRealKey(udwSvrId, udwKey);
    tbParam.Set_Key(ddwRealKey);
    tbParam.Set_Val(udwValueAdd, UPDATE_ACTION_TYPE_ADD); //加1并返回
    return CAwsRequest::UpdateItem(pstSession, &tbParam, ExpectedDesc(), dwRetType);
}

int CAwsRequest::PlayerGetByUid(SSession *pstSession, TUINT32 udwUid)
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

int CAwsRequest::BufferActionQueryBySuid(SSession *pstSession, TUINT32 udwUid)
{
    TbAction tbAction;
    tbAction.Set_Suid(udwUid);
    tbAction.Set_Id(0);
    CompareDesc comDesc;
    comDesc.dwCompareType = COMPARE_TYPE_GT;
    return  Query(pstSession, &tbAction, ETbACTION_OPEN_TYPE_PRIMARY, comDesc);
}

int CAwsRequest::AlActionQueryBySuid(SSession *pstSession, TUINT32 udwUid)
{
    TbAlliance_action tbAction;
    tbAction.Set_Suid(udwUid);
    tbAction.Set_Id(0);
    CompareDesc comDesc;
    comDesc.dwCompareType = COMPARE_TYPE_GT;
    return  Query(pstSession, &tbAction, ETbALLIANCEACTION_OPEN_TYPE_PRIMARY, comDesc);
}

int CAwsRequest::MarchActionQueryBySuid(SSession *pstSession, TUINT32 udwSid, TUINT32 udwUid)
{
    TbMarch_action tbAction;
    tbAction.Set_Suid(udwUid);
    tbAction.Set_Id(0);
    CompareDesc comDesc;
    comDesc.dwCompareType = COMPARE_TYPE_GT;
    return  Query(pstSession, &tbAction, ETbMARCH_OPEN_TYPE_PRIMARY, comDesc);
}

int CAwsRequest::MarchActionQueryByTuid(SSession *pstSession, TUINT32 udwSid, TUINT32 udwUid)
{
    TINT64 ddwKey = udwUid;
    TbMarch_action tbAction;
    tbAction.Set_Tal(-1 * ddwKey); //no range key
    return  Query(pstSession, &tbAction, ETbMARCH_OPEN_TYPE_GLB_TUID, CompareDesc(), false);
}

int CAwsRequest::MarchActionQueryBySal(SSession *pstSession, TUINT32 udwSid, TUINT32 udwAid)
{
    TINT64 ddwKey = udwAid;
    TbMarch_action tbAction;
    tbAction.Set_Sal(ddwKey);//no range key
    return  Query(pstSession, &tbAction, ETbMARCH_OPEN_TYPE_GLB_SAL, CompareDesc(), false);
}

int CAwsRequest::MarchActionQueryByTal(SSession *pstSession, TUINT32 udwSid, TUINT32 udwAid)
{
    TINT64 ddwKey = udwAid;
    TbMarch_action tbAction;
    tbAction.Set_Tal(ddwKey);//no range key
    return  Query(pstSession, &tbAction, ETbMARCH_OPEN_TYPE_GLB_TAL, CompareDesc(), false);
}

int CAwsRequest::ActionQueryByEtime(AwsReqInfo *pstAwsReq, TINT64 ddwSid, TUINT32 udwEtime)
{
    TbAction tbAction;
    tbAction.Set_Sid(ddwSid);
    tbAction.Set_Etime(udwEtime);
    CompareDesc comDesc;
    comDesc.dwCompareType = COMPARE_TYPE_LE;

    pstAwsReq->SetVal(&tbAction, tbAction.GetTableName(), "Query", ETbACTION_OPEN_TYPE_GLB_ETIME);
    tbAction.OnQueryReq(pstAwsReq->sReqContent, ETbACTION_OPEN_TYPE_GLB_ETIME, comDesc, false, true, true, 50);

    return 0;
}

int CAwsRequest::MarchQueryByEtime(AwsReqInfo *pstAwsReq, TINT64 ddwSid, TUINT32 udwEtime)
{
    TbMarch_action tbMarch;
    tbMarch.Set_Sid(ddwSid);
    tbMarch.Set_Etime(udwEtime);
    CompareDesc comDesc;
    comDesc.dwCompareType = COMPARE_TYPE_LE;

    pstAwsReq->SetVal(&tbMarch, tbMarch.GetTableName(), "Query", ETbMARCH_OPEN_TYPE_GLB_ETIME);
    tbMarch.OnQueryReq(pstAwsReq->sReqContent, ETbMARCH_OPEN_TYPE_GLB_ETIME, comDesc, false, true, true, 50);

    return 0;
}

int CAwsRequest::AlActionQueryByEtime(AwsReqInfo *pstAwsReq, TINT64 ddwSid, TUINT32 udwEtime)
{
    TbAlliance_action tbAlAction;
    tbAlAction.Set_Sid(ddwSid);
    tbAlAction.Set_Etime(udwEtime);
    CompareDesc comDesc;
    comDesc.dwCompareType = COMPARE_TYPE_LE;

    pstAwsReq->SetVal(&tbAlAction, tbAlAction.GetTableName(), "Query", ETbALLIANCEACTION_OPEN_TYPE_GLB_ETIME);
    tbAlAction.OnQueryReq(pstAwsReq->sReqContent, ETbALLIANCEACTION_OPEN_TYPE_GLB_ETIME, comDesc, false, true, true, 50);

    return 0;
}

int CAwsRequest::AllianceGetByAid(SSession *pstSession, TUINT32 udwAid)
{
    TbAlliance tbAlliance;
    tbAlliance.Set_Aid(udwAid);
    return GetItem(pstSession, &tbAlliance, ETbALLIANCE_OPEN_TYPE_PRIMARY);
}

int CAwsRequest::MapGetByIdWithLock(SSession *pstSession, TUINT32 udwSvrId, TUINT32 udwCid)
{
    TbMap tbMap;
    tbMap.Set_Sid(udwSvrId);
    tbMap.Set_Id(udwCid);
    return GetItem(pstSession, &tbMap, ETbMAP_OPEN_TYPE_PRIMARY, true, true);
}

int CAwsRequest::MapQueryByUid(SSession *pstSession, TUINT32 udwSvrId, TUINT32 udwUid)
{
    TbMap tbMap;
    tbMap.Set_Uid(udwUid);
    tbMap.Set_Sid(udwSvrId);
    return Query(pstSession, &tbMap, ETbMAP_OPEN_TYPE_GLB_UID, CompareDesc(), false);
}

int CAwsRequest::UserStatGet(SSession *pstSession, TINT32 dwId, bool bConsistentRead)
{
    TbUser_stat tbStat;
    tbStat.Set_Uid(dwId);
    return CAwsRequest::GetItem(pstSession, &tbStat, ETbUSERSTAT_OPEN_TYPE_PRIMARY, bConsistentRead);
}

int CAwsRequest::UserReportPut(SSession *pstSession, TINT64 ddwUid, TUINT32 udwRid, TUINT8 ucType)
{
    TbReport_user tbReportUserItem;
    tbReportUserItem.Set_Uid(ddwUid);
    tbReportUserItem.Set_Rid(udwRid);
    tbReportUserItem.Set_Type(ucType);
    return CAwsRequest::PutItem(pstSession, &tbReportUserItem);
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

int CAwsRequest::AlAssistQuery(SSession *pstSession, TUINT32 udwAid)
{

    TbAl_assist tbAssist;
    tbAssist.Set_Aid(udwAid);
    tbAssist.Set_Id(0);
    CompareDesc comp_desc;
    comp_desc.dwCompareType = COMPARE_TYPE_GT;
    return Query(pstSession, &tbAssist, ETbALASSIST_OPEN_TYPE_PRIMARY, comp_desc, TRUE, TRUE, FALSE, MAX_AL_ASSIST_NUM - 1);
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

int CAwsRequest::AlMemberGet(SSession* pstSession, TUINT32 udwAid, TUINT32 udwUid)
{
    TbAl_member tbAlmember;
    tbAlmember.Set_Aid(udwAid);
    tbAlmember.Set_Uid(udwUid);
    return GetItem(pstSession, &tbAlmember, ETbALLIANCEMEMBER_OPEN_TYPE_PRIMARY);
}

int CAwsRequest::BackPackGet(SSession *pstSession, TINT64 ddwUid)
{
    TbBackpack tbBackpack;
    tbBackpack.Set_Uid(ddwUid);
    return GetItem(pstSession, &tbBackpack, ETbBackpack_OPEN_TYPE_PRIMARY);
}

int  CAwsRequest::EquipQueryByStatus(SSession *pstSession, TUINT32 udwUid, TUINT32 udwStatus)
{
    TbEquip tbEquip;
    tbEquip.Set_Uid(udwUid);
    tbEquip.Set_Status(EN_EQUIPMENT_STATUS_ON_DRAGON);
    CompareDesc comp_desc;
    comp_desc.dwCompareType = COMPARE_TYPE_EQ;
    return Query(pstSession, &tbEquip, ETbEQUIP_OPEN_TYPE_STATUS, comp_desc, false, true, false, MAX_USER_EQUIP_NUM / 2);
}

int CAwsRequest::EquipQueryByUid(SSession *pstSession, TUINT32 udwUid)
{
    TINT64 ddwKey = udwUid;
    CompareDesc comp_desc;
    comp_desc.dwCompareType = COMPARE_TYPE_BETWEEN;
    comp_desc.vecN.push_back(ddwKey << 32);
    comp_desc.vecN.push_back((ddwKey << 32) + (unsigned int)-1);
    TbEquip tbEquip;
    tbEquip.Set_Uid(udwUid);
    return  Query(pstSession, &tbEquip, ETbEQUIP_OPEN_TYPE_PRIMARY, comp_desc);
}

int CAwsRequest::MapGet(SSession *pstSession, TUINT32 udwMapId)
{
    TbMap tbMapItem;
    tbMapItem.Set_Sid(pstSession->m_udwReqSvrId);
    tbMapItem.Set_Id(udwMapId);
    return GetItem(pstSession, &tbMapItem, ETbMAP_OPEN_TYPE_PRIMARY);
}

int CAwsRequest::NewCityIdQuery(SSession *pstSession, TUINT32 udwSvrId, TUINT8 ucProvinceId)
{
    TbMap tbMap;
    tbMap.Set_Sid(udwSvrId);
    TUINT32 udwMinX = (ucProvinceId % 4) * 200 + 1;
    TUINT32 udwMaxX = (ucProvinceId % 4 + 1) * 200;
    TUINT32 udwX = CToolBase::GetRandNumber(udwMinX, udwMaxX);

    TUINT32 udwX2 = udwX == udwMaxX ? udwMaxX - 1 : udwX + 1;

    TUINT32 udwMinY = (ucProvinceId / 4) * 200 + 1;
    TUINT32 udwMaxY = (ucProvinceId / 4 + 1) * 200;

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
    comp_desc_one.push_back(MAP_X_Y_POS_COMPUTE_OFFSET * udwX + udwMinY);
    comp_desc_one.push_back(MAP_X_Y_POS_COMPUTE_OFFSET * udwX + udwMaxY);
    Query(pstSession, &tbMap, ETbMAP_OPEN_TYPE_GLB_ID, comp_desc_one, false, true, true, MAX_WILD_RETURN_NUM / 2);

    CompareDesc comp_desc_two;
    comp_desc_two.dwCompareType = COMPARE_TYPE_BETWEEN;
    comp_desc_two.push_back(MAP_X_Y_POS_COMPUTE_OFFSET * udwX2 + udwMinY);
    comp_desc_two.push_back(MAP_X_Y_POS_COMPUTE_OFFSET * udwX2 + udwMaxY);
    Query(pstSession, &tbMap, ETbMAP_OPEN_TYPE_GLB_ID, comp_desc_two, false, true, true, MAX_WILD_RETURN_NUM / 2);

    return 0;
}

int CAwsRequest::SvrAlQuery(SSession *pstSession, TUINT32 udwSvrId)
{
    TbSvr_al tbSvrAl;
    tbSvrAl.Set_Sid(udwSvrId);
    tbSvrAl.Set_Alid(0);
    CompareDesc comp_desc;
    comp_desc.dwCompareType = COMPARE_TYPE_GT;
    return Query(pstSession, &tbSvrAl, ETbSVR_AL_OPEN_TYPE_PRIMARY, comp_desc);
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
{    CompareDesc stCompareInfo;
    stCompareInfo.dwCompareType = COMPARE_TYPE_GE;
    TbTitle tbTitle;
    tbTitle.Set_Sid(udwSid);
    tbTitle.Set_Id(0);

    CAwsRequest::Query(pstSession, &tbTitle, ETbTITLE_OPEN_TYPE_PRIMARY, stCompareInfo);    return 0;
}