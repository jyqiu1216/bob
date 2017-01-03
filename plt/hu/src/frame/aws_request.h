#ifndef _AWS_REQUEST_H_
#define _AWS_REQUEST_H_

#include "aws_table_common.h"
#include "time_utils.h"
#include <string>
#include "player_info.h"
#include "aws_req_info.h"

using namespace std;


#define HS_REQ_FIELD_DELIMITER	(',')

struct SSession;
struct SReqParam;

class CAwsRequest
{
public:
	static void UIntToString(unsigned int key, char*pstr);// 数字转字符串——36进制

public:
    static int UpdateItem(SSession *pstSession, AwsTable* pAwsTbl, const ExpectedDesc& expected_desc = ExpectedDesc(), int dwReturnValuesType = RETURN_VALUES_NONE, bool bNeedUnLock = false);

    static int DeleteItem(SSession *pstSession, AwsTable* pAwsTbl, const ExpectedDesc& expected_desc = ExpectedDesc(), int dwReturnValuesType = RETURN_VALUES_NONE);

    static int PutItem(SSession *pstSession,  AwsTable* pAwsTbl);

    static int GetItem(SSession *pstSession,  AwsTable* pAwsTbl, unsigned int udwIdxNo,  bool bConsistentRead = true, bool bNeedLock = false);

    static int Query(SSession *pstSession,  AwsTable* pAwsTbl, unsigned int udwIdxNo, const CompareDesc& comp_desc = CompareDesc(), bool bConsistentRead = true, 
        bool bReturnConsumedCapacity = true, bool bScanIndexForward = true, unsigned int dwLimit = 0);

    static int QueryCount(SSession *pstSession,  AwsTable* pAwsTbl, unsigned int udwIdxNo, const CompareDesc& comp_desc = CompareDesc(), bool bConsistentRead = true, 
        bool bReturnConsumedCapacity = true, bool bScanIndexForward = true, unsigned int dwLimit = 0);

    static int Scan(SSession *pstSession,  AwsTable* pAwsTbl, unsigned int udwIdxNo,  
        bool bHasStartKey = false, bool bReturnConsumedCapacity = true, unsigned int dwLimit = 0, unsigned int dwSegment = 0, unsigned int dwTotalSegments = 0);

    //批量读(只能读同一张表)
    static int BatchGetItem(SSession *pstSession, AwsTable* pAwsTbl, unsigned int udwObjSize, unsigned int udwObjNum, unsigned int udwIdxNo);

public:
    static int LoginGetByUid(SSession *pstSession, TUINT32 udwUid);

    static int UserGetByUid(SSession *pstSession, TUINT32 udwUid);

    static int CityGetByUid(SSession *pstSession, TUINT32 udwUid);

    static int UserStatGet(SSession *pstSession, TUINT32 udwUid);

    static int BlackAccountGet(SSession *pstSession, TINT64 ddwUid);

    static int BlackAccountGet(SSession *pstSession, const string& strDid);

    static int BookmarkQuery(SSession *pstSession, TUINT32 udwUid);

    static int MapQueryByUid(SSession *pstSession, TUINT32 udwSvrId,  TUINT32 udwUid);

    static int MapQueryByBid(SSession *pstSession, TUINT32 udwSvrId, TUINT32 udwBid);

    static int AllianceGetByAid(SSession *pstSession, TUINT32 udwAid);

    static int AllianceGetByName(SSession *pstSession, const string& strName);

    static int AlHelpHistoryQuery(SSession *pstSession, TUINT32 udwUid); //获取帮助过的联盟任务列表

    static int CanHelpTaskQuery(SSession *pstSession, TUINT32 udwAid, TUINT32 udwLimit); //获取可以帮助的任务列表

    static int AlAssistQuery(SSession *pstSession, TUINT32 udwAid);

    static int PlayerAssistQuery(SSession *pstSession, TUINT32 udwSvrId, TUINT32 udwUid, TUINT32 udwLimit);

    static int AlRequestConutQuery(SSession *pstSession, TINT32 dwAid);

    static int AllAlMemberQuery(SSession *pstSession, TINT32 dwAid);

    static int DiplomacyQuery(SSession *pstSession, TUINT32 udwAid); //查询目标alliance指向别人的所有diplomacy

    static int WallMsgCntQuery(SSession *pstSession, TUINT32 udwSvrId, TUINT32 udwAid); //查询未读的联盟公告数量
    static int TopWallMsgCntQuery(SSession *pstSession, TUINT32 udwSvrId, TUINT32 udwAid);

    static int AlConsumeHistoryQuery(SSession *pstSession, TUINT32 udwAid, TUINT32 udwLimit); //获取联盟消费记录
    
    static int TipsQuery(SSession *pstSession, TINT64 ddwUid, TUINT32 udwLimit);

    static int EventTipsQuery(SSession *pstSession, TINT64 ddwUid, TUINT32 udwLimit);

    //static int AlEventTipsQuery(SSession *pstSession, TINT64 ddwAlid, TUINT32 udwLimit);

    static int AllianceWallQuery(SSession *pstSession, TUINT32 udwAlId);

    static int QuestGetByUid(SSession *pstSession, TUINT32 udwUid);

    static int BountyGetByUid(SSession *pstSession, TUINT32 udwUid);

    static int MailOperateHistoryQuery(SSession *pstSession, TUINT32 udwUid);
    
    static int MapGet(SSession *pstSession, TUINT32 udwMapId);

    static int MapGetByIdAndSid(SSession *pstSession, TUINT32 udwMapId, TUINT32 udwSid);

    static int UserGetByPlayerName(SSession *pstSession, const string& sUin);

    static int GetGlobalNewId(SSession *pstSession, TUINT32 udwKey, TINT32 dwRetType = RETURN_VALUES_ALL_NEW);
    
    static int BackPackGet(SSession *pstSession, TINT64 ddwUid);
    
    static int GetGlobalId(SSession *pstSession, TUINT32 udwKey);

    static int BufferActionQueryBySuid(SSession *pstSession, TUINT32 udwUid);

    static int AlActionQueryBySuid(SSession *pstSession, TUINT32 udwUid);

    static int MarchActionQueryBySuid(SSession *pstSession, TUINT32 udwSid, TUINT32 udwUid);

    static int MarchActionQueryByTuid(SSession *pstSession, TUINT32 udwSid, TUINT32 udwUid);

    static int MarchActionQueryBySal(SSession *pstSession, TUINT32 udwSid, TUINT32 udwAid);

    static int MarchActionQueryByTal(SSession *pstSession, TUINT32 udwSid, TUINT32 udwAid);

    static int AttackThroneMarchQuery(SSession *pstSession, TINT64 ddwSid, TINT64 ddwPos);

    static int AlMemberGet(SSession* pstSession, TUINT32 udwAid, TUINT32 udwUid);

    static int EquipQueryByStatus(SSession *pstSession, TUINT32 udwUid,TUINT32 udwStatus);
    
    static int EquipQueryByUid(SSession *pstSession, TUINT32 udwUid);

    static int AltarHistoryQuery(SSession* pstSession, TUINT32 udwUid);

    static int SvrListInfoQuery(SSession* pstSession);

    static int BlackListQuery(SSession *pstSession, TUINT32 udwUid);

    static int BlackListGet(SSession *pstSession, TUINT32 udwSUid, TUINT32 udwTUid);

    static int DataOutputGet(SSession *pstSession, TUINT32 udwUid);

    static int LordImageGet(SSession *pstSession, TUINT32 udwUid);

    static int DecorationGet(SSession *pstSession, TUINT32 udwUid);

//logic
public:
    static int NewCityIdQuery(SSession *pstSession, TUINT32 udwSvrId, TUINT8 ucProvinceId);

    static int AddCity(SUserInfo *pstUser, TUINT32 udwSvrId, unsigned int udwCityId);

    static void CreateUserInfo(SSession *pstSession, SUserInfo *pstUser, TUINT32 udwSvrId, unsigned int udwCityId);

    static void CreateFakeUserInfo(SSession *pstSession, SUserInfo *pstUser, TUINT32 udwSvrId, unsigned int udwCityId);

    static int AddFakeCity(SUserInfo *pstUser, TUINT32 udwSvrId, unsigned int udwCityId);

    static void CreateFakePlayerInfo(SSession *pstSession, SUserInfo *pstUser, TUINT32 udwSvrId, TUINT32 udwCityId);

    static int LoginCreate(SSession *pstSession, TUINT32 udwUserId);

    static void UpdateUserAccountInfo(SSession *pstSession, TbLogin* pTbLogin, SReqParam *pstReqParam);

    static void CreatePlayerInfo(SSession *pstSession, SUserInfo *pstUser, TUINT32 udwSvrId, TUINT32 udwCityId);

    static int UserReportPut(SSession *pstSession, TINT64 ddwUid, TbReport* ptbReport, TUINT8 ucType);

    static int BroadcastQuery(SSession *pstSession);

public:
    static int TaskGet(SSession *pstSession, TUINT32 udwUid);

    static int ReqDataCenterQuestInfo(SSession *pstSession, TUINT32 udwQuestType);

public:
    static int RewardWindowQuery(SSession *pstSession, TINT32 ddwUid, TINT64 ddwQueryTime);

    static int RandomRewardQuery(SSession *pstSession, TINT64 ddwUid);

public:
    static int ThroneGet(SSession *pstSession, TUINT32 udwSid);
    static int IdolQuery(SSession *pstSession, TUINT32 udwSid);
    static int TitleQuery(SSession *pstSession, TUINT32 udwSid);
};
 


#endif


