#ifndef _SSC_TASK_PROCESS_H_
#define _SSC_TASK_PROCESS_H_

#include "std_header.h"
#include "action_map.h"
#include "war_process.h"
#include "user_json.h"
#include "pushdata_action.h"
#include "json_result_generator.h"

#define MAX_ACTION_RELATE_PLAYER_NUM	(2)
#define MAX_ACTION_RELATE_CITY_NUM		(2)
#define MAX_ACTION_RELATE_ACTION_NUM	(30)
#define MAX_ACTION_RELATE_WILD_NUM		(1)


// des-key��ע����ò�Ҫ����8���ֽڣ������ٶȽ���Ӱ��
#define DES_ENCRYPT_KEY_STR					"Ly123!@#"
#define RC4_ENCRYPT_KEY_STR					"Ly1234567!@#$%^&"

class CTaskProcess
{
public:
    // ��ʼ��
    TINT32 Init(CConf *poConf, ILongConn *poSearchLongConn, ILongConn *poQueryLongConn,
        CTaskQueue *pTaskQueue, CTseLogger *poServLog, CTseLogger *poDbLog);
    // �߳���ں���
    static void *Start(void *pParam);
    // ���ԴӶ������ȡ����
    TINT32 WorkRoutine();
public:
    /*****************************************
    ��ͻ�����ص���������Ϣ������
    *****************************************/
    // ����command�������������
    TVOID ProcessReqCommand(SSession *pstSession);
    TVOID UidActionProdure(SSession *pstSession);
    TVOID AidActionProdure(SSession *pstSession);
    TVOID MarchActionProdure(SSession *pstSession);

    TINT32 CheckReqAction(SSession* pstSession, SUserInfo* pstUserInfo);
    //���ݷ������
    TINT32 SendAwsRequest(SSession *pstSession, TUINT16 uwReqServiceType);
    TINT32 SendDbRequest(SSession *pstSession, TUINT16 uwReqServiceType);
    TINT32 SendDataCenterRequest(SSession *pstSession, TUINT16 uwReqServiceType);
    TINT32 SendMapSvrRequest(SSession *pstSession, TUINT16 uwReqServiceType);
    TINT32 SendReportSvrRequest(SSession *pstSession);

    TINT32 ProcessProcedure_LockReq(SSession *pstSession, TUINT16 uwReqServiceType);

    //���ݻ�ȡ���
    TINT32 FirstLockGetRequest(SSession *pstSession);
    TINT32 FirstDataGetRequest(SSession *pstSession);
    TINT32 SecondLockGetRequest(SSession *pstSession);
    TINT32 SecondDataGetRequest(SSession *pstSession);
    TINT32 UserInfoGetRequest(SSession *pstSession, TINT32 dwUid);
    TINT32 UserInfoGetResponse(SSession *pstSession, SUserInfo *pstUserInfo, TBOOL IsSourceUser = TRUE);

    TINT32 AllianceInfoRequest(SSession *pstSession, SUserInfo *pstUser, TINT32 dwAlid, TBOOL IsNeedReport = FALSE);
    TINT32 AllianceInfoGetResponse(SSession *pstSession, SUserInfo *pstUserInfo, TBOOL IsSourceUser);
    //���ݸ������
    TINT32 ProcessProcedure_UserUpdtRequest(SSession *pstSession);
    TINT32 GetUserUpdtRequest(SSession *pstSession, SUserInfo *pstUserInfo);
    TINT32 GetCurActionUpdtRequest(SSession *pstSession);
    TINT32 GetSessionUpdtRequest(SSession* pstSession);
    TINT32 SyncReqActionToUserInfo(SSession *pstSession);

    // ����action����
    TINT32 UpdateAndCheckActionInfo(SSession *pstSession, TbMap *pstWild, SUserInfo *pstSource, SUserInfo *pstTarget);

    TINT32 GenReportIdRequest(SSession *pstSession);
    TINT32 InsertReport(SSession *pstSession);
    TVOID  GenReportRequest(SSession *pstSession);

    TINT32 SendEventRequest(SSession *pstSession, TUINT16 uwReqServiceType);

private:
    TVOID GenFormatLog(SSession *pstSession, TUINT64 uddwCostTime);
    TVOID GenUidAcitonHourLog(SSession *pstSession, TUINT64 uddwCostTime, TUINT32 udwKey0, TUINT32 udwKey1, string strGameEvaluateLog);
    TVOID GenAidAcitonHourLog(SSession *pstSession, TUINT64 uddwCostTime, TUINT32 udwKey0, TUINT32 udwKey1, string strGameEvaluateLog);
    TVOID GenSidAcitonHourLog(SSession *pstSession, TUINT64 uddwCostTime, TUINT32 udwKey0, TUINT32 udwKey1, string strGameEvaluateLog);
    TVOID GetReqActionCmdAndKeys(SSession *pstSession);

private:
    TINT32 AbNormalActionHandle(SSession *pstSession); //�쳣action����

public:
    // �����ļ�����
    CConf               *m_poConf;
    ILongConn           *m_poSearchLongConn;
    ILongConn			*m_poQueryLongConn;

    // ��־����
    CTseLogger          *m_poServLog;
    CTseLogger			*m_poDbLog;
    CTseLogger			*m_poReqLog;

    // �������
    CTaskQueue          *m_pTaskQueue;

    // ���/�����
    CBaseProtocolPack   *m_pPackTool[MAX_LOCAL_HS_TASK_NUM];
    CBaseProtocolUnpack *m_pUnPackTool;

private:
    // log bug
    TCHAR m_szLogBuf[MAX_HTTP_REQ_LEN];
    TCHAR *m_pTmpBuf;

    CUserJson m_oUserJson;
    Json::Value m_jUserJson;
    Json::FastWriter m_jWriter;
    string m_sRspJsonContent;

private:
    CJsonResultGenerator *m_pJsonGenerator;
};

#endif
