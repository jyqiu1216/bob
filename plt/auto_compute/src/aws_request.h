#ifndef _AWS_REQUEST_H_
#define _AWS_REQUEST_H_

#include "time_utils.h"
#include <string>
#include "player_info.h"
#include "aws_req_info.h"
#include "jsoncpp_utils.h"
using namespace std;


#define HS_REQ_FIELD_DELIMITER	(',')

struct SSession;
struct SReqParam;

class CAwsRequest
{
public:
    static void UIntToString(unsigned int key, char*pstr);// 数字转字符串——36进制

public:
    static int  UpdateItem(SSession *pstSession, AwsTable* pAwsTbl, const ExpectedDesc& expected_desc = ExpectedDesc(), int dwReturnValuesType = RETURN_VALUES_NONE, bool bNeedUnLock = false);

    static int  DeleteItem(SSession *pstSession, AwsTable* pAwsTbl, const ExpectedDesc& expected_desc = ExpectedDesc(), int dwReturnValuesType = RETURN_VALUES_NONE);

    static int  PutItem(SSession *pstSession, AwsTable* pAwsTbl);

    static int GetItem(SSession *pstSession, AwsTable* pAwsTbl, unsigned int udwIdxNo, bool bConsistentRead = true, bool bNeedLock = false);

    static int  Query(SSession *pstSession, AwsTable* pAwsTbl, unsigned int udwIdxNo, const CompareDesc& comp_desc = CompareDesc(), bool bConsistentRead = true,
        bool bReturnConsumedCapacity = true, bool bScanIndexForward = true, unsigned int dwLimit = 0);

    static int BatchGetItem(SSession *pstSession, AwsTable* pAwsTbl, unsigned int udwObjSize, unsigned int udwObjNum, unsigned int udwIdxNo);

public:
    static int LoginGetByUid(SSession *pstSession, TUINT32 udwUid);

    static int QuestGetByUid(SSession *pstSession, TUINT32 udwUid);

    static int BountyGetByUid(SSession *pstSession, TUINT32 udwUid);

    static int GlobalParamNewIdUpdtAndGet(SSession *pstSession, TUINT32 udwSvrId, TUINT32 udwValueAdd, TUINT32 udwKey, TINT32 dwRetType = RETURN_VALUES_UPDATED_NEW);

    static int PlayerGetByUid(SSession *pstSession, TUINT32 udwUid);

    static int CityGetByUid(SSession *pstSession, TUINT32 udwUid);

public:
    static int BufferActionQueryBySuid(SSession *pstSession, TUINT32 udwUid);

    static int AlActionQueryBySuid(SSession *pstSession, TUINT32 udwUid);

    static int MarchActionQueryBySuid(SSession *pstSession, TUINT32 udwSid, TUINT32 udwUid);

    static int MarchActionQueryByTuid(SSession *pstSession, TUINT32 udwSid, TUINT32 udwUid);

    static int MarchActionQueryBySal(SSession *pstSession, TUINT32 udwSid, TUINT32 udwAid);

    static int MarchActionQueryByTal(SSession *pstSession, TUINT32 udwSid, TUINT32 udwAid);

    static int AlActionQueryBySal(SSession* pstSession, TUINT32 udwAid);

    static int AlActionQueryByEtime(AwsReqInfo *pstAwsReq, TINT64 ddwSid, TUINT32 udwEtime);

    static int ActionQueryByEtime(AwsReqInfo *pstAwsReq, TINT64 ddwSid, TUINT32 udwEtime);

    static int MarchQueryByEtime(AwsReqInfo *pstAwsReq, TINT64 ddwSid, TUINT32 udwEtime);

public:
    static int AllianceGetByAid(SSession *pstSession, TUINT32 udwAid);

    static int MapGetByIdWithLock(SSession *pstSession, TUINT32 udwSvrId, TUINT32 udwMapId); //udwCid->pos

    static int MapGet(SSession *pstSession, TUINT32 udwMapId);

    static int MapQueryByUid(SSession *pstSession, TUINT32 udwSvrId, TUINT32 udwUid);

    static int UserStatGet(SSession *pstSession, TINT32 dwId, bool bConsistentRead = true);

    static int UserReportPut(SSession *pstSession, TINT64 ddwUid, TUINT32 udwRid, TUINT8 ucType);

    static int UserReportPut(SSession *pstSession, TINT64 ddwUid, TbReport* ptbReport, TUINT8 ucType);

    static int AlAssistQuery(SSession *pstSession, TUINT32 udwAid);

    static int DiplomacyQuery(SSession *pstSession, TUINT32 udwAid); //查询目标alliance指向别人的所有diplomacy

    static int AlMemberGet(SSession* pstSession, TUINT32 udwAid, TUINT32 udwUid);

    static int BackPackGet(SSession *pstSession, TINT64 ddwUid);

    static int EquipQueryByStatus(SSession *pstSession, TUINT32 udwUid, TUINT32 udwStatus);

    static int EquipQueryByUid(SSession *pstSession, TUINT32 udwUid);

public:
    static int NewCityIdQuery(SSession *pstSession, TUINT32 udwSvrId, TUINT8 ucProvinceId);

public:
    static int SvrAlQuery(SSession *pstSession, TUINT32 udwSvrId);

public:
    static int ThroneGet(SSession *pstSession, TUINT32 udwSid);
    static int IdolQuery(SSession *pstSession, TUINT32 udwSid);
    static int TitleQuery(SSession *pstSession, TUINT32 udwSid);
};

#endif
