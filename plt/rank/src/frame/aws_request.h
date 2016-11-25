#ifndef _AWS_REQUEST_H_
#define _AWS_REQUEST_H_

//#include "hs_define.h"
#include "time_utils.h"
#include <string>
#include "player_info.h"
#include "aws_req_info.h"

// #include "model.h"


using namespace std;



#define HS_REQ_FIELD_DELIMITER	(',')

struct SSession;
struct SReqParam;

class CAwsRequest
{
public:
    static void GetMapIdList(SUserInfo *pstUser, vector<unsigned int>& vecMapId);
public:
    static void UIntToString(unsigned int key, char*pstr);// ����ת�ַ�������36����

public:
    static int UpdateItem(SSession *pstSession, AwsTable* pAwsTbl,
        const ExpectedDesc& expected_desc = ExpectedDesc(), int dwReturnValuesType = RETURN_VALUES_NONE);

    static int DeleteItem(SSession *pstSession, AwsTable* pAwsTbl,
        const ExpectedDesc& expected_desc = ExpectedDesc(), int dwReturnValuesType = RETURN_VALUES_NONE);

    static int PutItem(SSession *pstSession, AwsTable* pAwsTbl);

    static int GetItem(SSession *pstSession, AwsTable* pAwsTbl, unsigned int udwIdxNo,
        bool bConsistentRead = true);

    static int Query(SSession *pstSession, AwsTable* pAwsTbl, unsigned int udwIdxNo,
        const CompareDesc& comp_desc = CompareDesc(), bool bConsistentRead = true,
        bool bReturnConsumedCapacity = true, bool bScanIndexForward = true, unsigned int dwLimit = 0);

    static int QueryCount(SSession *pstSession, AwsTable* pAwsTbl, unsigned int udwIdxNo,
        const CompareDesc& comp_desc = CompareDesc(), bool bConsistentRead = true,
        bool bReturnConsumedCapacity = true, bool bScanIndexForward = true, unsigned int dwLimit = 0);

    static int Scan(SSession *pstSession, AwsTable* pAwsTbl, unsigned int udwIdxNo,
        bool bHasStartKey = false, bool bReturnConsumedCapacity = true, unsigned int dwLimit = 0, unsigned int dwSegment = 0, unsigned int dwTotalSegments = 0);

    //������(ֻ�ܶ�ͬһ�ű�)
    static int BatchGetItem(SSession *pstSession, AwsTable* pAwsTbl, unsigned int udwObjSize, unsigned int udwObjNum, unsigned int udwIdxNo);
 
};

#endif


