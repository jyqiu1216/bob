#ifndef _SSC_SESSION_H_
#define _SSC_SESSION_H_

#include "queue_t.h"
#include "base/common/wtsetypedef.h"
#include "down_mgr.h"
#include "my_define.h"
#include "service_key.h"
#include <vector>
#include "event_req_info.h"
#include "event_rsp_info.h"
#include "conf.h"
#include "key_value.h"

using namespace std;

#pragma pack(1)

// ����
enum EProcedure
{
    // ������ʼ״̬
    EN_PROCEDURE__INIT = 0,
    // ����client���
    EN_PROCEDURE__CLIENT_REQUEST,
    EN_PROCEDURE__CLIENT_RESOPNSE,
    //�����ִ���
    EN_PROCEDURE__COMMAND_PROCESS,
    // ���̽������
    EN_PROCEDURE__END,  // 80
    EN_PROCEDURE__SEND_RESULT_BACK,
};


// �����
enum ECommandStep
{
    EN_COMMAND_STEP__INIT = 0,

    // �����ֲ���
    EN_COMMAND_STEP__1,
    EN_COMMAND_STEP__2,
    EN_COMMAND_STEP__3,
    EN_COMMAND_STEP__4,
    EN_COMMAND_STEP__5,
    EN_COMMAND_STEP__6,
    EN_COMMAND_STEP__7,
    EN_COMMAND_STEP__8,
    EN_COMMAND_STEP__9,
    EN_COMMAND_STEP__10,
    EN_COMMAND_STEP__11,

    EN_COMMAND_STEP__END = 9999,

};


enum EExpectProcedure
{
    EN_EXPECT_PROCEDURE__INIT = 0,
    EN_EXPECT_PROCEDURE__LOCK_GET,
    EN_EXPECT_PROCEDURE__LOCK_RELEASE,
    EN_EXPECT_PROCEDURE__AWS,
    EN_EXPECT_PROCEDURE__DB,
    EN_EXPECT_PROCEDURE__EVENT = 10,
    EN_EXPECT_PROCEDURE__CACHE = 11,
    EN_EXPECT_PROCEDURE__TRANSLATE = 12,
    EN_EXPECT_PROCEDURE__DATA_CENTER = 13,
};

struct SCommonResInfo
{
    TINT32              m_dwRetCode;
    TUINT64             m_uddwCostTime;
    TUINT8              m_ucJsonType;
    TBOOL               m_bRefreshCode;//TRUE ��ʾ������ȷ  FALSE ��ʾ���ݴ���

    void Reset()
    {
        m_dwRetCode = 0;
        m_uddwCostTime = 0;
        m_ucJsonType = EN_JSON_TYPE_USER_JSON;
        m_bRefreshCode = FALSE;
    }
};

struct SReqParam
{
    // req url
    TCHAR				m_szReqUrl[MAX_HTTP_REQ_LEN];

    // common param
    TUINT8              m_ucResType;                        // res type
    TCHAR				m_szIp[MAX_IP_LEN];
    TUINT64				m_uddwDeviceId;						// device id
    TUINT32				m_udwSvrId;							// svr id
    TUINT32				m_udwUserId;						// user global id
    TUINT32				m_udwCityId;						// city id;
    TINT64             m_ddwReportId;                      //�ͻ�����ȡ��������report id
    TINT64             m_ddwMailId;                         //�ͻ�����ȡ��������mail id
    TUINT32				m_udwAllianceId;					// alliance id;
    TUINT8              m_ucAlliancePos;                    // alliance pos;
    TUINT32				m_udwSeqNo;
    TFLOAT32			m_udwVersion;
    TUINT8				m_ucLoginStatus;					//��¼ʱ��1����¼��ʹ����0;
    TUINT32				m_udwLang;		                    //�û�������
    TUINT32				m_udwPage;
    TUINT32				m_udwPerpage;
    TCHAR				m_szGameCenterID[MAX_GAME_CENTER_ID_LEN];
    TUINT64				m_uddwGId;
    TCHAR				m_szDevice[MAX_DEVICE_ID_LEN];
    TCHAR               m_szIdfa[MAX_DEVICE_ID_LEN];
    TCHAR               m_szIosVer[MAX_OS_VER_LEN];
    TCHAR               m_szPid[MAX_PRODUCT_ID_LEN];
    TCHAR               m_szVs[MAX_PRODUCT_ID_LEN];
    TCHAR               m_szSy[MAX_PRODUCT_ID_LEN];
    TCHAR               m_szPlatForm[MAX_PRODUCT_ID_LEN];
    TINT64              m_ddwReqCost;                       // �ͻ��˵Ĵ���ʱ��


    // �����û���������ʱʹ��
    TUINT8				m_ucProvince;
    TUINT32				m_udwSkip;
    TUINT8				m_ucIsNpc;
    TUINT8              m_ucIsSandBox;
    TBOOL               m_bNeedLoginCheck; //�Ƿ��˺���֤���������Ҫ��ֻҪuid��ȷ���ɣ�һ������op cmd

    TCHAR				m_szCommand[DEFAULT_NAME_STR_LEN];
    TUINT32				m_udwCommandID;
    TCHAR				m_szKey[MAX_REQ_PARAM_KEY_NUM][DEFAULT_PARAM_STR_LEN];
    TCHAR				m_szExKey[MAX_REQ_PARAM_KEY_NUM][DEFAULT_PARAM_STR_LEN];

    TUINT32				m_udwLoginType;
    TUINT32             m_udwLoginSuccType;

    TUINT8				m_ucIsNewPlayer;
    TUINT8				m_ucIsNewSvrPlayer;
    TUINT8				m_ucIsGuideFinish;

    //wave@20140522
    TUINT32             m_udwInReqTime; //�ͻ��˴�������ʱ�����

    //charles@20150709
    TBOOL				m_bLotteryRefresh;

    string m_szPurchaseToken;
    string m_szPackageName;
    string m_szItemId;
    string m_szPurchaseUid;

    void Reset()
    {
        m_szReqUrl[0] = 0;

        m_ucResType = 0;

        m_szIp[0] = 0;
        m_uddwDeviceId = 0;
        m_udwSvrId = (TUINT32)-1;
        m_udwUserId = 0;
        m_udwCityId = 0;

        m_ddwReportId = 0;
        m_ddwMailId = 0;

        m_udwAllianceId = 0;
        m_ucAlliancePos = 0;

        m_szGameCenterID[0] = 0;
        m_uddwGId = 0;
        m_szDevice[0] = 0;
        m_szIdfa[0] = '\0';
        m_szSy[0] = '\0';
        m_szVs[0] = '\0';
        m_szPlatForm[0] = '\0';
        m_ddwReqCost = 0;

        m_ucProvince = 0;
        m_udwSkip = 0;
        m_ucIsNpc = 0;
        m_ucIsSandBox = 0;
        m_bNeedLoginCheck = TRUE;

        m_udwSeqNo = 0;
        m_udwVersion = 1.0;
        m_ucLoginStatus = EN_LOGIN_STATUS__USING;
        m_udwLang = 0;
        m_udwPage = 1;
        m_udwPerpage = DEFAULT_PERPAGE_NUM;

        m_szCommand[0] = 0;
        m_udwCommandID = 0;
        for (TUINT32 udwIdx = 0; udwIdx < MAX_REQ_PARAM_KEY_NUM; udwIdx++)
        {
            m_szKey[udwIdx][0] = 0;
            m_szExKey[udwIdx][0] = 0;
        }

        m_udwLoginType = EN_LOGIN_TYPE__GID;
        m_udwLoginSuccType = EN_LOGIN_TYPE__GID;

        m_ucIsNewPlayer = EN_NEW_PLAYER__NORMAL;
        m_ucIsNewSvrPlayer = 0;
        m_ucIsGuideFinish = 0;

        m_udwInReqTime = 0;
        m_szIosVer[0] = 0;
        m_szPid[0] = 0;

        m_bLotteryRefresh = FALSE;

        m_szPurchaseToken.clear();
        m_szPurchaseUid.clear();
        m_szPackageName.clear();
        m_szItemId.clear();
    }
};

struct SSession
{
    /********************************************************************************************
                        �ⲿ������Ϣ�����к���Ϣ
                        *********************************************************************************************/
    TUINT32             m_udwSeqNo;
    LongConnHandle      m_stClientHandle;
    TUINT32             m_udwRequestType;

    TCHAR               m_szClientIp[32];
    TUINT32             m_udwPackSeq;
    TUINT32             m_udwClientSeqNo;
    TUINT32             m_udwPbSeq;
    TUINT32             m_udwLinkerCmdRef;

    TUINT32             m_udwExpectProcedure;
    TUINT32             m_udwNextProcedure;
    TUINT32             m_udwProcessSeq; // �����������
    TUINT32             m_udwCommand;
    TUINT32             m_udwCommandStep;
    /********************************************************************************************
                        session����״̬
                        *********************************************************************************************/
    TUINT8              m_ucIsUsing;
    TBOOL               m_bProcessing;

    /********************************************************************************************
                        ��ǰʹ�õ����νڵ�
                        *********************************************************************************************/
    TUINT32             m_udwContentType;

    SDownNode            *m_pstEventProxyNode; //daemon
    TBOOL               m_bEventProxyExist;
    SDownNode            *m_pstThemeEventProxyNode; //kuro for theme event
    TBOOL               m_bThemeEventProxyExist;

    /********************************************************************************************
                        ������Ϣ�ڴ�
                        *********************************************************************************************/
    TUCHAR              m_szClientReqBuf[MAX_HTTP_REQ_LEN];					// client����Buf
    TUINT32             m_udwClientReqBufLen;								// client����Buf����
    TINT32              m_dwClientReqEnType;                                // ������ܷ�ʽ��0�Ǽ��ܣ�1����
    TINT32              m_dwClientResType;                                  // ��Ӧ��ʽ��0ȫ����1����
    TINT32              m_dwClientReqMode;                                  // 0:http, 1:tcp-������

    TCHAR               m_szClientRspBuf[MAX_NETIO_PACKAGE_BUF_LEN_UL];        // client����Buf
    TINT32              m_dwFinalPackLength;                                // client�����Ż����С

    /********************************************************************************************
                        �ڲ�������Ϣ(session�м䱣����Ϣ)
                        *********************************************************************************************/
    SReqParam           m_stReqParam;	// �������
    SCommonResInfo      m_stCommonResInfo;	// ������

    TUINT32             m_udwDownRqstType; //1:read 2:write 3:read&write

    /********************************************************************************************
                        ���õ�task_process����
                        *********************************************************************************************/
    CConf               *m_poConf;
    CTseLogger          *m_poServLog;
    CTseLogger          *m_poDbLog;
    ILongConn           *m_poLongConn;
    CBaseProtocolPack   **m_ppPackTool;
    RequestParam        *m_pstHttpParam;
    TUCHAR              *m_pszReqBuf;
    TUINT32             m_udwReqBufLen;
    TCHAR*              m_pTmpBuf;

    /********************************************************************************************
                        ʱ�����------�����������ν����ĵط�������Ҫ��¼��ʱ
                        *********************************************************************************************/
    TUINT64             m_uddwTimeBeg;     // �յ������ʱ��
    TUINT64             m_uddwTimeEnd;     // ����ǰ�˻�Ӧ��ʱ�� 
    TUINT64             m_uddwDownRqstTimeBeg;
    TUINT64             m_uddwDownRqstTimeEnd;
    TUINT64             m_uddwProcessBegTime;
    TUINT64             m_uddwProcessEndTime;

    TUINT32             m_udwEventRqstType; //1:update 2:get_info 3:get_all_info
    TBOOL               m_bGotoOtherCmd;

    //event
    vector<EventReqInfo*> m_vecEventReq;
    vector<EventRspInfo*> m_vecEventRsp;

    void ResetEventReq()
    {
        for (unsigned int i = 0; i < m_vecEventReq.size(); ++i)
        {
            delete m_vecEventReq[i];
        }
        m_vecEventReq.clear();
    }
    void ResetEventRsp()
    {
        for (unsigned int i = 0; i < m_vecEventRsp.size(); ++i)
        {
            delete m_vecEventRsp[i];
        }
        m_vecEventRsp.clear();
    }
    void ResetEventInfo()
    {
        ResetEventReq();
        ResetEventRsp();
    }

    void ReleaseDownService()
    {
        CDownMgr *poDownMgr = CDownMgr::Instance();
        if (m_bEventProxyExist)
        {
            poDownMgr->zk_ReleaseNode(DOWN_NODE_TYPE__EVENT_PROXY, m_pstEventProxyNode);
        }
        if (m_bThemeEventProxyExist)
        {
            poDownMgr->zk_ReleaseNode(DOWN_NODE_TYPE__THEME_EVENT_PROXY, m_pstThemeEventProxyNode);
        }
    }
    void Reset()
    {
        ReleaseDownService();

        m_udwSeqNo = 0;
        memset((char*)&m_stClientHandle, 0, sizeof(LongConnHandle));
        m_udwRequestType = 0;
        m_szClientIp[0] = 0;
        m_udwPackSeq = 0;
        m_udwClientSeqNo = 0;
        m_udwCommand = 0;
        m_udwCommandStep = 0;
        m_udwPbSeq = 0;
        m_udwLinkerCmdRef = 0;
        m_udwExpectProcedure = 0;
        m_udwNextProcedure = 0;
        m_udwProcessSeq = 0;
        m_udwDownRqstType = 0;

        m_ucIsUsing = 0;
        m_bProcessing = FALSE;

        m_bGotoOtherCmd = FALSE;

        m_udwContentType = EN_CONTENT_TYPE__STRING;

        //����
        m_pstEventProxyNode = NULL;
        m_bEventProxyExist = FALSE;
        m_pstThemeEventProxyNode = NULL;
        m_bThemeEventProxyExist = FALSE;

        m_dwClientReqEnType = 0; //�Ƿ���ܱ��
        m_dwClientResType = 0;

        m_szClientRspBuf[0] = '\0';
        m_dwFinalPackLength = 0;

        m_szClientReqBuf[0] = 0;
        m_udwClientReqBufLen = 0;

        m_stReqParam.Reset();
        m_stCommonResInfo.Reset();

        m_uddwTimeBeg = 0;
        m_uddwTimeEnd = 0;
        m_uddwProcessBegTime = 0;
        m_uddwProcessEndTime = 0;

        m_udwEventRqstType = 0;

        // task process ����
        m_poConf = NULL;
        m_poServLog = NULL;
        m_poDbLog = NULL;
        m_poLongConn = NULL;
        m_ppPackTool = NULL;
        m_pstHttpParam = NULL;
        m_pszReqBuf = NULL;
        m_udwReqBufLen = 0;
        m_pTmpBuf = NULL;
    }
};

#pragma pack()

typedef CQueueT<SSession *> CTaskQueue;

#endif
