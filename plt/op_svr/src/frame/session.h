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

// 流程
enum EProcedure
{
    // 流程起始状态
    EN_PROCEDURE__INIT = 0,
    // 返回client结果
    EN_PROCEDURE__CLIENT_REQUEST,
    EN_PROCEDURE__CLIENT_RESOPNSE,
    //命令字处理
    EN_PROCEDURE__COMMAND_PROCESS,
    // 流程结束标记
    EN_PROCEDURE__END,  // 80
    EN_PROCEDURE__SEND_RESULT_BACK,
};


// 命令步骤
enum ECommandStep
{
    EN_COMMAND_STEP__INIT = 0,

    // 命令字步骤
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
    TBOOL               m_bRefreshCode;//TRUE 表示数据正确  FALSE 表示数据错误

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
    TINT32              m_dwOpEncryptFlag;

    // req url
    TCHAR               m_szReqUrl[MAX_HTTP_REQ_LEN];

    // common param
    TCHAR               m_szIp[MAX_IP_LEN];
    TUINT64             m_uddwDeviceId;                     // device id
    TCHAR				m_szDevice[MAX_DEVICE_ID_LEN];
    TINT32              m_dwSvrId;                         // svr id
    TINT64              m_ddwUserId;                        // user global id
    TUINT32             m_udwSeqNo;
    TCHAR               m_szPlatForm[DEFAULT_NAME_STR_LEN];
    TUINT32				m_udwLang;		                    //用户的语言
    TFLOAT32			m_udwVersion;


    TCHAR               m_szCommand[DEFAULT_NAME_STR_LEN];
    TUINT32             m_udwCommandID;
    TCHAR               m_szKey[MAX_REQ_PARAM_KEY_NUM][DEFAULT_PARAM_STR_LEN];
    TCHAR               m_szExKey[MAX_REQ_PARAM_KEY_NUM][DEFAULT_PARAM_STR_LEN];




    void Reset()
    {
        m_dwOpEncryptFlag = 0;
        m_szReqUrl[0] = 0;

        m_szIp[0] = 0;
        m_uddwDeviceId = 0;
        m_dwSvrId = (TUINT32)-1;
        m_ddwUserId = 0;

        m_udwSeqNo = 0;
        m_szPlatForm[0] = '\0';
        m_udwLang = 0;

        m_szCommand[0] = 0;
        m_udwCommandID = 0;
        m_udwVersion = 1.0;

        for (TUINT32 udwIdx = 0; udwIdx < MAX_REQ_PARAM_KEY_NUM; udwIdx++)
        {
            m_szKey[udwIdx][0] = '\0';
            m_szExKey[udwIdx][0] = '\0';
        }
    }
};

struct SSession
{
    /********************************************************************************************
                        外部链接信息和序列号信息
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
    TUINT32             m_udwProcessSeq; // 处理流程序号
    TUINT32             m_udwCommand;
    TUINT32             m_udwCommandStep;
    /********************************************************************************************
                        session本身状态
                        *********************************************************************************************/
    TUINT8              m_ucIsUsing;
    TBOOL               m_bProcessing;

    /********************************************************************************************
                        当前使用的下游节点
                        *********************************************************************************************/
    TUINT32 m_udwContentType;

    SDownNode            *m_pstAwsProxyNode;
    TBOOL               m_bAwsProxyNodeExist;
    SDownNode            *m_pstEventProxyNode; //daemon
    TBOOL               m_bEventProxyExist;
    SDownNode            *m_pstThemeEventProxyNode; //kuro for theme event
    TBOOL               m_bThemeEventProxyExist;

    /********************************************************************************************
                        网络消息内存
                        *********************************************************************************************/
    TUCHAR              m_szClientReqBuf[MAX_HTTP_REQ_LEN];					// client请求Buf
    TUINT32             m_udwClientReqBufLen;								// client请求Buf长度
    TINT32              m_dwClientReqEnType;                                // 请求加密方式：0非加密，1加密
    TINT32              m_dwClientResType;                                  // 响应方式：0全量，1增量
    TINT32              m_dwClientReqMode;                                  // 0:http, 1:tcp-长连接

    TCHAR               m_szClientRspBuf[MAX_NETIO_PACKAGE_BUF_LEN];        // client返包Buf
    TINT32              m_dwFinalPackLength;                                // client返包优化后大小
    TINT32              m_dwOriPackLength;

    /********************************************************************************************
                        内部保留信息(session中间保留信息)
                        *********************************************************************************************/
    SReqParam           m_stReqParam;	// 请求参数
    SCommonResInfo      m_stCommonResInfo;	// 结果相关

    /********************************************************************************************
                        借用的task_process变量
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
                        时间参数------凡是有上下游交互的地方，都需要记录耗时
                        *********************************************************************************************/
    TUINT64             m_uddwTimeBeg;     // 收到请求的时间
    TUINT64             m_uddwTimeEnd;     // 发送前端回应的时间 
    TUINT64             m_uddwDownRqstTimeBeg;
    TUINT64             m_uddwDownRqstTimeEnd;
    TUINT64             m_uddwProcessBegTime;
    TUINT64             m_uddwProcessEndTime;

    TUINT32 m_udwEventRqstType; //1:update 2:get_info 3:get_all_info
    TBOOL m_bGotoOtherCmd;

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
        if (m_bAwsProxyNodeExist)
        {
            poDownMgr->zk_ReleaseNode(DOWN_NODE_TYPE__AWS_PROXY, m_pstAwsProxyNode);
        }
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

        m_ucIsUsing = 0;
        m_bProcessing = FALSE;

        m_bGotoOtherCmd = FALSE;

        m_udwContentType = EN_CONTENT_TYPE__STRING;

        //下游
        m_bAwsProxyNodeExist = FALSE;
        m_pstAwsProxyNode = NULL;
        m_pstEventProxyNode = NULL;
        m_bEventProxyExist = FALSE;
        m_pstThemeEventProxyNode = NULL;
        m_bThemeEventProxyExist = FALSE;

        m_dwClientReqEnType = 0; //是否加密标记
        m_dwClientResType = 0;

        m_szClientRspBuf[0] = '\0';
        m_dwFinalPackLength = 0;
        m_dwOriPackLength = 0;

        m_bAwsProxyNodeExist = FALSE;
        m_pstAwsProxyNode = NULL;


        m_szClientReqBuf[0] = 0;
        m_udwClientReqBufLen = 0;

        m_stReqParam.Reset();
        m_stCommonResInfo.Reset();

        m_uddwTimeBeg = 0;
        m_uddwTimeEnd = 0;
        m_uddwProcessBegTime = 0;
        m_uddwProcessEndTime = 0;

        m_udwEventRqstType = 0;

        // task process 变量
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
