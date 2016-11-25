#ifndef _SSC_SESSION_H_
#define _SSC_SESSION_H_

#include <vector>
#include <map>
#include "base/common/wtsetypedef.h"
#include "jsoncpp/json/json.h"
#include "queue_t.h"
#include "down_mgr.h"
#include "my_define.h"
#include "key_value.h"
#include "http_utils.h"
#include "service_key.h"
#include "aws_request.h"
#include "aws_response.h"
#include "alliance_rank.h"
#include "player_rank.h"
#include "compute_time.h"
#include "server_data.h"
#include "conf.h"

using namespace std;

#pragma pack(1)

// 主业务流程步骤
enum EProcedure
{
    // 流程起始状态
    EN_PROCEDURE__INIT              = 0,

    EN_PROCEDURE__ACCOUNT_SEARCH_REQUEST,
    EN_PROCEDURE__ACCOUNT_SEARCH_RESPONSE,
    

    // 主业务流程
    EN_PROCEDURE__ACCOUNT_CHECK,
    EN_PROCEDURE__ACCOUNT_CREATE,
    EN_PROCEDURE__PLAYER_CREATE,
    EN_PROCEDURE__LOCK_GET,
    EN_PROCEDURE__GET_UID_DATA,
    EN_PROCEDURE__RESPONSE_UID_DATA,
    EN_PROCEDURE__GET_AID_DATA,
    EN_PROCEDURE__RESPONSE_AID_DATA,
    EN_PROCEDURE__BEFORE_PROCESS_CMD,
    EN_PROCEDURE__PROCESS_CMD,
    EN_PROCEDURE__AFTER_PROCESS_CMD,
    EN_PROCEDURE__UPT_DATA,
    EN_PROCEDURE__LOCK_RELEACE,

    // 客户端流程
    EN_PROCEDURE__CLIENT_REQUEST,
    
    // 流程结束标记
    EN_PROCEDURE__END,
    // 回包流程
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

    EN_COMMAND_STEP__END    = 9999,
};

// 数据回调处理流程
enum EExpertProcedure
{
    EN_PROCEDURE__EXPERT_NODE_AWS,
    EN_PROCEDURE__EXPERT_NODE_LOCAL_MYSQL,
    EN_PROCEDURE__EXPERT_NODE_LOCK,
};


/*
 *  结果状态
 */
enum EResult
{
    // 结果状态初始值
    EN_RESULT__INIT                 = 0,
    // 结果可信
    EN_RESULT__TRUST,
    // 结果不可信
    EN_RESULT__UNTRUST,
    // 结果失败
    EN_RESULT__FAIL,
};

struct SReqParam
{
    // req url
    TCHAR               m_szReqUrl[MAX_HTTP_REQ_LEN];

    // common param
    TCHAR               m_szIp[MAX_IP_LEN];
    TUINT64             m_uddwDeviceId;                     // device id
    TUINT32             m_udwSvrId;                         // svr id
    TUINT32             m_udwUserId;                        // user global id
    TUINT32             m_udwCityId;                        // city id;
    TUINT32             m_udwAllianceId;                    // alliance id;
    TUINT32             m_udwSeqNo;
    TFLOAT32            m_udwVersion;
    TUINT8              m_ucLoginStatus;                    //登录时置1，登录后使用是0;
    TUINT32             m_udwLang;                          //用户的语言
    TUINT32             m_udwPage;
    TUINT32             m_udwPerpage;

    TCHAR               m_szCommand[DEFAULT_NAME_STR_LEN];
    TUINT32             m_udwCommandID;
    TCHAR               m_szKey[MAX_REQ_PARAM_KEY_NUM][DEFAULT_PARAM_STR_LEN];
    TCHAR               m_szExKey[MAX_REQ_PARAM_KEY_NUM][DEFAULT_PARAM_STR_LEN];

    TINT32              m_dwTotalNum;
    TINT32              m_dwSelectNum;

    TINT32              m_udwOpEncryptFlag;

    void Reset()
    {
        m_szReqUrl[0] = 0;

        m_szIp[0] = 0;
        m_uddwDeviceId = 0;
        m_udwSvrId = (TUINT32)-1;
        m_udwUserId = 0;
        m_udwCityId = 0;
        m_dwTotalNum = 0;
        m_dwSelectNum = 0;
        m_udwOpEncryptFlag = 0;

        m_udwAllianceId = 0;

        m_udwSeqNo = 0;
        m_udwVersion = 1.0;
        m_ucLoginStatus = EN_LOGIN_STATUS__USING;
        m_udwLang = 0;
        m_udwPage = 1;
        m_udwPerpage = DEFAULT_PERPAGE_NUM;

        m_szCommand[0] = 0;
        m_udwCommandID = 0;
        for(TUINT32 udwIdx = 0; udwIdx < MAX_REQ_PARAM_KEY_NUM; udwIdx++)
        {
            m_szKey[udwIdx][0] = '\0';
            m_szExKey[udwIdx][0] = '\0';
        }
    }
};

struct SCommonResInfo
{
    TINT32              m_dwRetCode;
    TUINT64             m_uddwCostTime;
    Json::Value         m_jResultWriter;
    AllianceRank        m_aobjAllianceRank[100];
    AllianceRank        m_aobjAllianceChampion[20];
    AllianceRank        m_objMyAllianceRank;
    PlayerRank          m_aobjPlayerRank[100];
    PlayerRank          m_aobjPlayerChampion[20];
    PlayerRank          m_objPlayerRank;
    ComputeTime         m_oComputeTime;
    AllianceRankInfo    al_rank_infos[DEFAULT_TOP_NUM];
    AllianceRankInfo    self_al_rank_info;
    PlayerRankInfo      player_rank_infos[DEFAULT_TOP_NUM];
    PlayerRankInfo      self_player_rank_info;
    RecommendAllianceInfo recommend_al_infos[DEFAULT_TOP_NUM];
    TINT32              result_count;
    TINT32              total_count;

    void Reset()
    {
        m_dwRetCode = 0;
        m_uddwCostTime = 0;
        m_jResultWriter.clear();
        result_count = 0;
        total_count = 0;
        self_al_rank_info.reset();
        self_player_rank_info.reset();
    }
    SCommonResInfo()
    {
    }
};

struct SSession
{
    /********************************************************************************************
                        外部链接信息和序列号信息
    *********************************************************************************************/
    TUINT32             m_udwSeqNo;
    TUINT32             m_udwPbSeqNo;
    TUINT32             m_udwPackSeqNo;
    TUINT32             m_udwClientSeqNo;
    LongConnHandle      m_stClientHandle;
    TUINT32             m_udwExpectProcedure;
    TUINT32             m_udwNextProcedure;
    TUINT32             m_udwWaitFinishProcedure;
    TUINT32             m_udwCommand;
    TUINT32             m_udwCommandStep;
    TUINT32             m_udwProcessSeq; // 处理流程序号
    TINT32              m_dwClientReqMode;
    TINT32              m_dwServiceType;

    /********************************************************************************************
                        session本身状态
    *********************************************************************************************/
    TUINT8              m_ucIsUsing;

    /********************************************************************************************
                        当前使用的下游节点
    *********************************************************************************************/
    SDownNode            *m_pstAwsProxyNode;
    TBOOL               m_bAwsProxyNodeExist;

    // lock svr
    SDownNode            *m_pstLockSvr;
    TBOOL               m_bLockSvrExist;
    TINT32              m_dwLockRetCode;
    TBOOL               m_bLockedData; // 是否有加锁
    TINT64              m_ddwLockedTaskId;

    /********************************************************************************************
                        网络消息内存
    *********************************************************************************************/
    TUCHAR              m_szClientReqBuf[MAX_HTTP_REQ_LEN];                 // client请求Buf
    TUINT32             m_udwClientReqBufLen;                               // client请求Buf长度

    /********************************************************************************************
                        内部保留信息
    *********************************************************************************************/
    SReqParam           m_stReqParam;   // 请求参数
    SCommonResInfo      m_stCommonResInfo;  // 结果相关


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
    TUINT32             m_udwTmpBufLen;

    /********************************************************************************************
                        时间参数
    *********************************************************************************************/
    TUINT64             m_uddwTimeBeg;     // 收到请求的时间
    TUINT64             m_uddwTimeEnd;     // 发送前端回应的时间
    TUINT64             m_uddwDownRqstTimeBeg;
    TUINT64             m_uddwDownRqstTimeEnd;

    //aws info
    vector<AwsReqInfo*> m_vecAwsReq;
    vector<AwsRspInfo*> m_vecAwsRsp;

    TUINT64 m_uddwAwsReadSumTime;
    TUINT64 m_uddwAwsWriteSumTime;
    TUINT64 m_uddwAwsReadWriteSumTime;
    TUINT64 m_uddwAwsNoOpSumTime;
    TUINT64 m_uddwDbReadSumTime;
    TUINT64 m_uddwDbWriteSumTime;
    TUINT64 m_uddwDbReadWriteSumTime;
    TUINT64 m_uddwDbNoOpSumTime;
    TUINT64 m_uddwAuctionSumTime;
    TUINT64 m_uddwLockSumTime;
    TUINT32 m_udwDownRqstType; //1:read 2:write 3:read&write

    TBOOL m_bGotoOtherCmd;

    void ResetAwsReq()
    {
        for (unsigned int i = 0; i < m_vecAwsReq.size(); ++i)
        {
            delete m_vecAwsReq[i];
        }
        m_vecAwsReq.clear();
    }
    void ResetAwsRsp()
    {
        for (unsigned int i = 0; i < m_vecAwsRsp.size(); ++i)
        {
            delete m_vecAwsRsp[i];
        }
        m_vecAwsRsp.clear();
    }
    void ResetAwsInfo()
    {
        ResetAwsReq();
        ResetAwsRsp();
    }

    void Reset()
    {
        m_udwSeqNo = 0;
        m_udwPbSeqNo = 0;
        m_udwPackSeqNo = 0;
        m_dwClientReqMode = EN_CLIENT_REQ_MODE__HTTP;
        m_dwServiceType = 0;

        memset((char*)&m_stClientHandle, 0, sizeof(LongConnHandle));
        m_udwExpectProcedure = 0;
        m_udwNextProcedure = 0;
        m_udwWaitFinishProcedure = 0; //nemo
        m_udwCommandStep = 0;
        m_udwProcessSeq = 0;

        m_ucIsUsing = 0;

        m_bAwsProxyNodeExist = FALSE;
        m_pstAwsProxyNode = NULL;

        m_szClientReqBuf[0] = 0;
        m_udwClientReqBufLen = 0;

        m_stReqParam.Reset();

        m_stCommonResInfo.Reset();

        m_uddwTimeBeg = 0;
        m_uddwTimeEnd = 0;
        
        // task process 变量
        m_poConf = NULL;
        m_poServLog = NULL;
        m_poDbLog = NULL;
        m_poLongConn = NULL;
        m_ppPackTool = NULL;
        //m_pPackTool = NULL;
        m_pstHttpParam = NULL;
        m_pszReqBuf = NULL;
        m_udwReqBufLen = 0;
        m_pTmpBuf = NULL;

        // lock svr
        m_pstLockSvr = NULL;
        m_bLockSvrExist = 0;
        m_dwLockRetCode = 0;
        m_bLockedData = FALSE;
        m_ddwLockedTaskId = 0;

        //aws info
        ResetAwsInfo();

        m_uddwAwsReadSumTime = 0;
        m_uddwAwsWriteSumTime = 0;
        m_uddwAwsReadWriteSumTime = 0;
        m_uddwAwsNoOpSumTime = 0;
        m_uddwDbReadSumTime = 0;
        m_uddwDbWriteSumTime = 0;
        m_uddwDbReadWriteSumTime = 0;
        m_uddwDbNoOpSumTime = 0;
        m_uddwAuctionSumTime = 0;
        m_uddwLockSumTime = 0;
        m_udwDownRqstType = 0;
        m_bGotoOtherCmd = FALSE;
    }
};

#pragma pack()

typedef CQueueT<SSession *> CTaskQueue;

#endif

