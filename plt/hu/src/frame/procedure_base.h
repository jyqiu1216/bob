#ifndef _PROCEDURE_BASE_H_
#define _PROCEDURE_BASE_H_

#include "session.h"
// #include "model.h"

class CBaseProcedure
{
public:
	static TINT32 SendAwsRequest(SSession *pstSession, TUINT16 uwReqServiceType);
    static TINT32 SendDbRequest(SSession *pstSession, TUINT16 uwReqServiceType);
    static TINT32 SendEventRequest(SSession *pstSession, TUINT16 uwReqServiceType);
    static TINT32 SendCacheRequest(SSession *pstSession, TUINT16 uwReqServiceType);
    static TINT32 SendTranslateRequest(SSession *pstSession, TUINT16 uwReqServiceType);
    static TINT32 SendDataCenterRequest(SSession *pstSession, TUINT16 uwReqServiceType);
    static TINT32 SendMapSvrRequest(SSession *pstSession, TUINT16 uwReqServiceType);
    static TINT32 SendIapSvrRequest(SSession *pstSession);
    static TINT32 SendReportSvrRequest(SSession *pstSession);
    static TINT32 SendPurchaseCheckRequest(SSession *pstSession, TUINT16 uwReqServiceType);
    static TINT32 SendRankSvrRequest(SSession *pstSession);

// 优化版本的数据拉取
public:
    
    // op命令字的帐号数据获取
    static TINT32 ProcessOperate_LoginGet(SSession *pstSession);
    static TINT32 ProcessOperate_LoginRes(SSession *pstSession);
    

    // 获取锁
	static TINT32 ProcessProcedure_LockReq(SSession *pstSession, TUINT16 uwReqServiceType);


    // 用户个人数据
    static TINT32 ProcessProcedure_UserDataGetRequest(SSession *pstSession);
    static TINT32 ProcessProcedure_UserDataGetResponse(SSession *pstSession);

    // 用户联盟数据和联盟数据
    static TINT32 ProcessProcedure_AllianceDataGetRequest(SSession *pstSession);
    static TINT32 ProcessProcedure_AllianceDataGetResponse(SSession *pstSession);

    // 获取用户的活动信息
    static TINT32 ProcessProcedure_EventInfoRequest(SSession *pstSession);
    static TINT32 ProcessProcedure_EventInfoResponse(SSession *pstSession);

    // 访问db proxy获取信息
    static TINT32 ProcessProcedure_DbDataGetRequest(SSession *pstSession);
    static TINT32 ProcessProcedure_DbDataGetResponse(SSession *pstSession);

    static TINT32 ProcessProcedure_DbDataUpdateRequest(SSession *pstSession);

    // function  ==> 用户个人数据/用户联盟数据/联盟数据的更新
    // in_value  ==> pstSession: 业务session
    // out_value ==> 返回请求aws更新数据的个数
    static TINT32 ProcessProcedure_UserAndAllianceDataUpdtRequest(SSession *pstSession);

    // 用户帐号数据更新
	static TINT32 ProcessProcedure_LoginUpdtRequest(SSession *pstSession);

    // 拍卖场的数据更新
    static TINT32 ProcessProcedure_AuctionUpdate(SSession *pstSession);

    static TINT32 ProcessProcedure_LoginRequest(SSession* pstSession);
    static TINT32 ProcessProcedure_LoginResponse(SSession* pstSession);

    // 数据中心的数据请求
    static TINT32 ProcessProcedure_DataCenterRequest(SSession* pstSession);
    static TINT32 ProcessProcedure_DataCenterResponse(SSession* pstSession);

    // 用户report user数据
    static TINT32 ProcessProcedure_ReportSvrRequest(SSession *pstSession);
    static TINT32 ProcessProcedure_ReportSvrResponse(SSession *pstSession);

	// 用户md5数据更新
    static TINT32 ProcessProcedure_DataOutputUpdateRequest(SSession* pstSession);

    // 用户数据推送
    static TINT32 ProcessProcedure_PushDataRequest(SSession* pstSession);
    static TINT32 ProcessProcedure_PushDataResponse(SSession* pstSession);

    // 用户rank信息获取
    static TINT32 ProcessProcedure_RankSvrRequest(SSession* pstSession);
    static TINT32 ProcessProcedure_RankSvrResponse(SSession* pstSession);

//     static TINT32 ProcessProcedure_UpdateUserData(SUserInfo *pstUser, SCityInfo *pstCity, SRefreshData &stRefreshData, TBOOL *abRefreshFlag);
//     static TINT32 GenDataCenterReqJson(SUserInfo *pstUser, SCityInfo *pstCity, TINT32 dwType, Json::Value &rDataReqJson);
//     static TINT32 GenDataCenterReq(SUserInfo * pstUser, SCityInfo * pstCity, vector<DataCenterReqInfo*>& vecReq, TBOOL *abRefreshFlag);
};

#endif
