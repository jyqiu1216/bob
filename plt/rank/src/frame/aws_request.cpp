#include "aws_request.h"
#include "session.h"
#include "aws_table_include.h"
//#include "game_info.h"
//#include "game_command.h"
#include "tool_base.h"

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

int CAwsRequest::UpdateItem(SSession *pstSession, AwsTable* pAwsTbl,
    const ExpectedDesc& expected_desc, int dwReturnValuesType)
{
    vector<AwsReqInfo*>& vecReq = pstSession->m_vecAwsReq;
    AwsReqInfo* pAwsReq = new AwsReqInfo(pAwsTbl, pAwsTbl->GetTableName(), "UpdateItem");
    pAwsTbl->OnUpdateItemReq(pAwsReq->sReqContent, expected_desc, dwReturnValuesType);
    if(pAwsReq->sReqContent.empty()) //不需要更新
    {
        delete pAwsReq;
        return 1;
    }
    vecReq.push_back(pAwsReq);
    return 0;
}

int CAwsRequest::DeleteItem(SSession *pstSession, AwsTable* pAwsTbl,
    const ExpectedDesc& expected_desc, int dwReturnValuesType)
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

int CAwsRequest::GetItem(SSession *pstSession, AwsTable* pAwsTbl, unsigned int udwIdxNo, bool bConsistentRead)
{
    vector<AwsReqInfo*>& vecReq = pstSession->m_vecAwsReq;
    AwsReqInfo* pAwsReq = new AwsReqInfo(pAwsTbl, pAwsTbl->GetTableName(), "GetItem", udwIdxNo);
    pAwsTbl->OnGetItemReq(pAwsReq->sReqContent, udwIdxNo, bConsistentRead);
    vecReq.push_back(pAwsReq);
    return 0;
}

int CAwsRequest::Query(SSession *pstSession, AwsTable* pAwsTbl, unsigned int udwIdxNo,
    const CompareDesc& comp_desc, bool bConsistentRead,
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
