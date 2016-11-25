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

// �Ż��汾��������ȡ
public:
    
    // op�����ֵ��ʺ����ݻ�ȡ
    static TINT32 ProcessOperate_LoginGet(SSession *pstSession);
    static TINT32 ProcessOperate_LoginRes(SSession *pstSession);
    

    // ��ȡ��
	static TINT32 ProcessProcedure_LockReq(SSession *pstSession, TUINT16 uwReqServiceType);


    // �û���������
    static TINT32 ProcessProcedure_UserDataGetRequest(SSession *pstSession);
    static TINT32 ProcessProcedure_UserDataGetResponse(SSession *pstSession);

    // �û��������ݺ���������
    static TINT32 ProcessProcedure_AllianceDataGetRequest(SSession *pstSession);
    static TINT32 ProcessProcedure_AllianceDataGetResponse(SSession *pstSession);

    // ��ȡ�û��Ļ��Ϣ
    static TINT32 ProcessProcedure_EventInfoRequest(SSession *pstSession);
    static TINT32 ProcessProcedure_EventInfoResponse(SSession *pstSession);

    // ����db proxy��ȡ��Ϣ
    static TINT32 ProcessProcedure_DbDataGetRequest(SSession *pstSession);
    static TINT32 ProcessProcedure_DbDataGetResponse(SSession *pstSession);

    static TINT32 ProcessProcedure_DbDataUpdateRequest(SSession *pstSession);

    // function  ==> �û���������/�û���������/�������ݵĸ���
    // in_value  ==> pstSession: ҵ��session
    // out_value ==> ��������aws�������ݵĸ���
    static TINT32 ProcessProcedure_UserAndAllianceDataUpdtRequest(SSession *pstSession);

    // �û��ʺ����ݸ���
	static TINT32 ProcessProcedure_LoginUpdtRequest(SSession *pstSession);

    // �����������ݸ���
    static TINT32 ProcessProcedure_AuctionUpdate(SSession *pstSession);

    static TINT32 ProcessProcedure_LoginRequest(SSession* pstSession);
    static TINT32 ProcessProcedure_LoginResponse(SSession* pstSession);

    // �������ĵ���������
    static TINT32 ProcessProcedure_DataCenterRequest(SSession* pstSession);
    static TINT32 ProcessProcedure_DataCenterResponse(SSession* pstSession);

    // �û�report user����
    static TINT32 ProcessProcedure_ReportSvrRequest(SSession *pstSession);
    static TINT32 ProcessProcedure_ReportSvrResponse(SSession *pstSession);

	// �û�md5���ݸ���
    static TINT32 ProcessProcedure_DataOutputUpdateRequest(SSession* pstSession);

    // �û���������
    static TINT32 ProcessProcedure_PushDataRequest(SSession* pstSession);
    static TINT32 ProcessProcedure_PushDataResponse(SSession* pstSession);

    // �û�rank��Ϣ��ȡ
    static TINT32 ProcessProcedure_RankSvrRequest(SSession* pstSession);
    static TINT32 ProcessProcedure_RankSvrResponse(SSession* pstSession);

//     static TINT32 ProcessProcedure_UpdateUserData(SUserInfo *pstUser, SCityInfo *pstCity, SRefreshData &stRefreshData, TBOOL *abRefreshFlag);
//     static TINT32 GenDataCenterReqJson(SUserInfo *pstUser, SCityInfo *pstCity, TINT32 dwType, Json::Value &rDataReqJson);
//     static TINT32 GenDataCenterReq(SUserInfo * pstUser, SCityInfo * pstCity, vector<DataCenterReqInfo*>& vecReq, TBOOL *abRefreshFlag);
};

#endif
