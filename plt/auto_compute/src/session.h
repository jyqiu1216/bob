#ifndef _SSC_SESSION_H_
#define _SSC_SESSION_H_

#include "queue_t.h"
#include "base/common/wtsetypedef.h"
#include "my_define.h"
#include "player_info.h"
#include "service_key.h"
#include "aws_table_include.h"
#include "aws_req_info.h"
#include "aws_rsp_info.h"
#include "db_req_info.h"
#include "db_rsp_info.h"
#include <vector>

#include "event_req_info.h"
#include "event_rsp_info.h"
#include "svr_list_conf.h"
#include "war_base.h"
#include "data_center_req_info.h"
#include "data_center_rsp_info.h"
#include "map_svr_req_info.h"
#include "report_req_info.h"
#include "report_rsp_info.h"

#include "zkutil/zk_util.h"
#include "pushdata_define.h"

using namespace std;

#pragma pack(1)

// 流程
enum EProcedure
{
    // 流程起始状态
    EN_PROCEDURE__INIT = 0,

    EN_PROCEDURE__LOCK_FOR_FISRT_DATA_GET,
    EN_PROCEDURE__FIRST_DATA_GET_REQUEST,
    EN_PROCEDURE__FIRST_DATA_GET_RESPONSE,
    EN_PROCEDURE__FIRST_ALLIANCE_DATA_GET_REQUEST,
    EN_PROCEDURE__FIRST_ALLIANCE_DATA_GET_RESPONSE,
    EN_PROCEDURE__FIRST_HANDLE_BEFORE,

    EN_PROCEDURE__LOCK_FOR_SECOND_DATA_GET,
    EN_PROCEDUER__SECOND_DATA_GET_REQUEST,
    EN_PROCEDUER__SECOND_DATA_GET_RESPONSE,
    EN_PROCEDURE__SECOND_ALLIANCE_DATA_GET_REQUEST,
    EN_PROCEDURE__SECOND_ALLIANCE_DATA_GET_RESPONSE,
    EN_PROCEDURE__SECOND_HANDLE_BEFORE,

    // 请求各方信息
    EN_PROCEDURE__USER_INFO_SEARCH_REQUEST,
    EN_PROCEDURE__USER_INFO_SEARCH_RESPONSE,

    // 更新各方信息
    EN_PROCEDURE__USER_INFO_UPDATE_REQUEST,
    EN_PROCEDURE__USER_INFO_UPDATE_RESPONSE,

    //异常action处理
    EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_REQUEST,
    EN_PROCEDURE__ABNORMAL_ACTION_HANDLE_RESPONSE,

    // 进行计算
    EN_PROCEDURE__COMMAND_PROCESS,

    // 请求action source user info
    EN_PROCEDURE__ACTION_SOURCE_INFO_SEARCH_REQUEST,
    EN_PROCEDURE__ACTION_SOURCE_INFO_SEARCH_RESPONSE,
    EN_PROCEDURE__ACTION_TARGET_INFO_SEARCH_REQUEST,
    EN_PROCEDURE__ACTION_TARGET_INFO_SEARCH_RESPONSE,

    // alliance 获取
    EN_PROCEDURE__ALLIANCE_SEARCH_REQUEST,
    EN_PROCEDURE__ALLIANCE_SEARCH_RESPONSE,

    // report insert
    EN_PROCEDURE__REPORT_INSERT_REQUEST,
    EN_PROCEDURE__REPORT_INSERT_RESPONSE,

    // report insert
    EN_PROCEDURE__REPORT_USER_INSERT_REQUEST,
    EN_PROCEDURE__REPORT_USER_INSERT_RESPONSE,

    // reports insert
    EN_PROCEDURE__REPORTS_INSERT_REQUEST,
    EN_PROCEDURE__REPORTS_INSERT_RESPONSE,

    EN_PROCEDURE__REPORTS_ID_GENERATE_REQUEST,
    EN_PROCEDURE__REPORTS_ID_GENERATE_RESPONSE,

    // LOCK
    EN_PROCEDURE__PLAYER_LOCK_GET_REQUEST,
    EN_PROCEDURE__PLAYER_LOCK_GET_RESPONSE,
    EN_PROCEDURE__PLAYER_LOCK_RELEASE_REQUEST,
    EN_PROCEDURE__PLAYER_LOCK_RELEASE_RESPONSE,

    // map
    EN_PROCEDURE__MAP_GET_REQUEST,
    EN_PROCEDURE__MAP_GET_RESPONSE,

    //auction
    EN_PROCEDURE__AUCTION_CHANGE_INFO_REQUEST,
    EN_PROCEDURE__AUCTION_CHANGE_INFO_RESPONSE,

    //event_rank
    EN_PROCEDURE__EVENT_SOURCE_RANK_GET_REQUEST,
    EN_PROCEDURE__EVENT_SOURCE_RANK_GET_RESPONSE,
    EN_PROCEDURE__EVENT_TARGET_RANK_GET_REQUEST,
    EN_PROCEDURE__EVENT_TARGET_RANK_GET_RESPONSE,

    EN_PROCEDURE__EVENT_SOURCE_RANK_UPDATE_REQUEST,
    EN_PROCEDURE__EVENT_SOURCE_RANK_UPDATE_RESPONSE,
    EN_PROCEDURE__EVENT_TARGET_RANK_UPDATE_REQUEST,
    EN_PROCEDURE__EVENT_TARGET_RANK_UPDATE_RESPONSE,

    EN_PROCEDURE__DELAY_UPDATE_REQUEST,
    EN_PROCEDURE__DELAY_UPDATE_RESPONSE,

    //event
    EN_PROCEDURE__EVENT_PROCESS_UPDATE,
    EN_PROCEDURE__EVENT_PROCESS_RESPONSE,

    // 流程结束标记
    EN_PROCEDURE__END,
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
    EN_EXPECT_PROCEDUER__LOCK_GET,
    EN_EXPECT_PROCEDUER__LOCK_RELEASE,
    EN_EXPECT_PROCEDURE__AWS,
    EN_EXPECT_PROCEDURE__DB,
    EN_EXPECT_PROCEDURE__EVENT = 10,
    EN_EXPECT_PROCEDURE__DATA_CENTER = 13,
    EN_EXPECT_PROCEDURE__MAP_SVR = 20,
    EN_EXPECT_PROCEDURE__ACTION_SVR = 21,
    EN_EXPECT_PROCEDURE__REPORT_SVR = 22,
};

enum ELockType
{
    EN_LOCK_TYPE__WILD = 0,
    EN_LOCK_TYPE__USER,
    EN_LOCK_TYPE__ALL,
};

/*
*  结果状态
*/
enum EResult
{
    // 结果状态初始值
    EN_RESULT__INIT = 0,
    // 结果可信
    EN_RESULT__TRUST,
    // 结果不可信
    EN_RESULT__UNTRUST,
    // 结果失败
    EN_RESULT__FAIL,
};

enum EReportFlag
{
    EN_REPORT_FLAG_NORMAL = 1,
    EN_REPORT_FLAG_PRISON = 1 << 1,
    EN_REPORT_FLAG_DRAGON_ATTACK = 1 << 2,
    EN_REPORT_FLAG_CHALLENGER_REWARD = 1 << 3,
};

struct SReqParam
{
    TCHAR m_szCommand[DEFAULT_NAME_STR_LEN];
    TCHAR m_szKey[DEFAULT_PARAM_STR_LEN];
    TUINT8 m_ucIsSuid;
    TUINT32 m_udwResource[EN_RESOURCE_TYPE__END];
    TBOOL m_BIsNeedCb;

    void SetCmd(const TCHAR *pszCmd)
    {
        strncpy(m_szCommand, pszCmd, DEFAULT_NAME_STR_LEN - 1);
        m_szCommand[DEFAULT_NAME_STR_LEN - 1] = 0;
    }

    void Reset()
    {
        SetCmd("error_cmd");
        m_szKey[0] = 0;
        m_ucIsSuid = 1;
        memset(m_udwResource, 0, sizeof(m_udwResource));
        m_BIsNeedCb = FALSE;
    }
};

struct SCommonResInfo
{
    TINT32 m_dwRetCode;
    TUINT64 m_uddwCostTime;
    string m_szErrorMsg;
    void Reset()
    {
        m_dwRetCode = 0;
        m_uddwCostTime = 0;
        m_szErrorMsg.clear();
    }
};

struct SSession
{
    /********************************************************************************************
    外部链接信息和序列号信息
    *********************************************************************************************/
    TUINT32 m_udwSeqNo;
    TUINT32 m_udwExpectProcedure;
    TUINT32 m_udwNextProcedure;
    TUINT32 m_udwCommandStep; //nemo
    TUINT32 m_udwProcessSeq; // 处理流程序号
    TUINT32 m_udwReqSvrId;	  // 请求的svrid

    TbAction m_stReqAction;	  // 请求的action信息
    TbAction m_stRawReqAction; // 保存action的初始状态
    TUINT8 m_ucReqActionFlag;	// 用于请求action数据的更新等操作

    TbAlliance_action m_stReqAlAction;	  // 请求的action信息
    TbAlliance_action m_stRawReqAlAction; // 保存action的初始状态
    TUINT8 m_ucReqAlActionFlag;	// 用于请求action数据的更新等操作

    TbMarch_action m_stReqMarch;	  // 请求的action信息
    TbMarch_action m_stRawReqMarch; // 保存action的初始状态
    TUINT8 m_ucReqMarchFlag;	// 用于请求action数据的更新等操作

    SActionTable m_stActionTable;

    TUINT8 m_ucMapUpdateCityStatus;
    string m_strGameEvaluateLog;

    /********************************************************************************************
    session本身状态
    *********************************************************************************************/
    TUINT8 m_ucIsUsing;
    TBOOL m_bProcessing;
    
    /********************************************************************************************
    当前使用的下游节点
    *********************************************************************************************/
    TUINT32 m_udwContentType;

    SDownNode *m_pstAwsProxyNode;									// 虚拟服务器svr
    TBOOL m_bAwsProxyNodeExist;								// 
    SDownNode *m_pstDbProxyNode;
    TBOOL m_bDbProxyNodeExist;
    SDownNode *m_pstEventProxyNode; //daemon
    TBOOL m_bEventProxyExist;
    SDownNode *m_pstThemeEventProxyNode;
    TBOOL m_bThemeEventProxyExist;
    
    // datacenter
    SDownNode *m_pstDataCenterProxyNode;  
    TBOOL m_bDataCenterProxyExist;

    SDownNode *m_pstMapSvrNode;
    TBOOL m_bMapSvrExist;

    SDownNode *m_pstBuffActionSvrNode;
    TBOOL m_bBuffActionSvrNodeExist;
    SDownNode *m_pstAlActionSvrNode;
    TBOOL m_bAlActionSvrNodeExist;
    SDownNode *m_pstMarchActionSvrNode;
    TBOOL m_bMarchActionSvrNodeExist;
    SDownNode *m_pstReportUserSvrNode;
    TBOOL m_bReportUserSvrNodeExist;
    SDownNode *m_pstUserLinkerNode;
    TBOOL m_bUserLinkerNodeExist;

    // lock svr
    SDownNode *m_pstLockSvr;
    TBOOL m_bLockSvrExist;
    TINT32 m_dwLockRetCode;
    TUINT32 m_udwLockUid;
    TBOOL m_bIsUidNeedLock;
    TUINT32 m_udwLockUidB;
    TBOOL m_bIsUidBNeedLock;
    TUINT32 m_udwLockWild;
    TBOOL m_bIsWildNeedLock;
    TUINT32 m_udwLockWildB;
    TBOOL m_bIsWildBNeedLock;
    TUINT32 m_udwLockAid;
    TBOOL m_bIsAidNeedLock;
    TUINT32 m_udwLockAidB;
    TBOOL m_bIsAidBNeedLock;
    TBOOL m_bLockedData;

    /********************************************************************************************
    网络消息内存
    *********************************************************************************************/
    //aws info
    vector<AwsReqInfo*> m_vecAwsReq;
    vector<AwsRspInfo*> m_vecAwsRsp;

    vector<AwsReqInfo*> m_vecDelayAwsReq;

    //db info
    vector<DbReqInfo*> m_vecDbReq;
    vector<DbRspInfo*> m_vecDbRsp;


    //event
    vector<EventReqInfo*> m_vecEventReq;
    vector<EventRspInfo*> m_vecEventRsp;

    
    // data center
    vector<DataCenterReqInfo*> m_vecDataCenterReq;
    vector<DataCenterRspInfo*> m_vecDataCenterRsp;

    //map_svr
    vector<MapSvrReqInfo*> m_vecMapSvrReq;

    //report_svr
    vector<ReportReqInfo*> m_vecReportReq;
    vector<ReportRspInfo*> m_vecReportRsp;

    /********************************************************************************************
    内部保留信息
    *********************************************************************************************/
    SUserInfo m_stSourceUser;
    SUserInfo m_stTargetUser;		// 目标用户信息 

    TbReport m_tbReport;
    TbReport m_tbChallengerReport;

    vector<TbReport> m_vecAlReport;
    vector<TINT64> m_vecReportAlReceiver;

    TbThrone m_tbThrone;

    TUINT32 m_udwIdolNum;
    TbIdol m_atbIdol[MAX_IDOL_NUM];
    TUINT8 m_ucIdolFlag[MAX_IDOL_NUM];

    TbMap m_stMapItem;		// 野地信息
    TUINT8 m_ucMapItemFlag;

    TbAl_gift m_tbTmpAlGift;

    TbRally_history m_atbTmpRallyHistory[2];
    TBOOL m_bNeedRallyHistory;

    SCommonResInfo m_stCommonResInfo;	// 结果相关
    CTseLogger *m_poServLog;

    TUINT8 m_ucActionRawStatus;
    TUINT8 m_ucActionRawSecClass;
    TUINT32 m_udwTrainRawType;
    TUINT32 m_udwTrainRawNum;
    TUINT32 m_udwRawTuid;
    TUINT8 m_ucRawWildType;
    TUINT8 m_ucRawWildLevel;

    TbMap m_tbTmpMap; //临时变量
    TUINT8 m_ucTmpMapItemFlag;

    TUINT32 m_udwTmpMarchNum;
    TbMarch_action m_atbTmpMarch[10];
    TUINT32 m_audwTmpMarchFlag[10];

    STitleInfoList m_stTitleInfoList;
    TbParam m_tbReportId;

    // raw param
    UActionParam m_uParam;

    SDragonNode m_stDragonNode;
    SMonsterNode m_stMonsterNode;

    CBaseProtocolPack **m_ppPackTool;
    ILongConn *m_poLongConn;

    /********************************************************************************************
    时间参数
    *********************************************************************************************/
    TUINT64 m_uddwTimeBeg;     // 收到请求的时间
    TUINT64 m_uddwTimeEnd;     // 发送前端回应的时间
    TUINT64 m_uddwDownRqstTimeBeg;
    TUINT64 m_uddwDownRqstTimeEnd;

    /*******************for cb log**************************************/
    SReqParam m_stReqParam;
    TINT32 m_dwMarchResult;
    TINT32 m_dwRealReportId;
    TINT32 m_dwReportFlag;
    TbReport m_tbPrisonReport;
    set<TINT64> m_vecPrisonReportReceivers;

    TUINT64 m_uddwAwsReadSumTime;
    TUINT64 m_uddwAwsWriteSumTime;
    TUINT64 m_uddwAwsReadWriteSumTime;
    TUINT64 m_uddwAwsNoOpSumTime;
    TUINT64 m_uddwDbReadSumTime;
    TUINT64 m_uddwDbWriteSumTime;
    TUINT64 m_uddwDbReadWriteSumTime;
    TUINT64 m_uddwDbNoOpSumTime;
    TUINT64 m_uddwLockSumTime;
    TUINT32 m_udwDownRqstType; //1:read 2:write 3:read&write

    TUINT32 m_udwEventRqstType; //1:update 2:get_info 3:get_all_info

    TINT64 m_ddwGemBegin; // for log

	TBOOL m_bCheckValidFlag;
    TINT64 m_ddwCheckUid;

    TCHAR* m_pTmpBuf;
    TUINT32 m_udwTmpBufLen;
    
    //wave@push_data
    CAuPushDataNode m_objAuPushDataNode;
    TVOID* m_pJsonGenerator;

    TBOOL bKingChanged;
    TINT32 dwRawThroneAlid;

    TINT32 m_dwReqMapPushFlag; //1: 强行不推 2: 强行推...

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

    void ResetDbReq()
    {
        for (unsigned int i = 0; i < m_vecDbReq.size(); ++i)
        {
            delete m_vecDbReq[i];
        }
        m_vecDbReq.clear();
    }
    void ResetDbRsp()
    {
        for (unsigned int i = 0; i < m_vecDbRsp.size(); ++i)
        {
            delete m_vecDbRsp[i];
        }
        m_vecDbRsp.clear();
    }

    void ResetDbInfo()
    {
        ResetDbReq();
        ResetDbRsp();
    }

    void ResetEventReq()
    {
        for(unsigned int i = 0; i < m_vecEventReq.size(); ++i)
        {
            delete m_vecEventReq[i];
        }
        m_vecEventReq.clear();
    }
    void ResetEventRsp()
    {
        for(unsigned int i = 0; i < m_vecEventRsp.size(); ++i)
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


    void ResetDataCenterReq()
    {
        for (unsigned int i = 0; i < m_vecDataCenterReq.size(); ++i)
        {
            delete m_vecDataCenterReq[i];
        }
        m_vecDataCenterReq.clear();
    }
    void ResetDataCenterRsp()
    {
        for (unsigned int i = 0; i < m_vecDataCenterRsp.size(); ++i)
        {
            delete m_vecDataCenterRsp[i];
        }
        m_vecDataCenterRsp.clear();
    }
    void ResetDataCenterInfo()
    {
        ResetDataCenterReq();
        ResetDataCenterRsp();
    }

    void ResetMapSvrReq()
    {
        for (unsigned int i = 0; i < m_vecMapSvrReq.size(); ++i)
        {
            delete m_vecMapSvrReq[i];
        }
        m_vecMapSvrReq.clear();
    }
    void ResetMapSvrInfo()
    {
        ResetMapSvrReq();
        ResetAwsRsp();
    }

    void ResetReportReq()
    {
        for (unsigned int i = 0; i < m_vecReportReq.size(); ++i)
        {
            delete m_vecReportReq[i];
        }
        m_vecReportReq.clear();
    }
    void ResetReportRsp()
    {
        for (unsigned int i = 0; i < m_vecReportRsp.size(); ++i)
        {
            delete m_vecReportRsp[i];
        }
        m_vecReportRsp.clear();
    }
    void ResetReportInfo()
    {
        ResetReportReq();
        ResetReportRsp();
    }
    
    void Reset()
    {
        m_udwSeqNo = 0;
        m_udwExpectProcedure = 0;
        m_udwNextProcedure = 0;
        m_udwCommandStep = 0;
        m_udwProcessSeq = 0;
        m_udwReqSvrId = 0;

        m_ucIsUsing = 0;
        m_bProcessing = FALSE;

        m_udwContentType = EN_CONTENT_TYPE__STRING;

        m_pstAwsProxyNode = NULL;
        m_bAwsProxyNodeExist = FALSE;
        m_pstDbProxyNode = NULL;
        m_bDbProxyNodeExist = FALSE;
        m_pstEventProxyNode = NULL;
        m_bEventProxyExist = FALSE;
        m_pstThemeEventProxyNode = NULL;
        m_bThemeEventProxyExist = FALSE;
        m_pstDataCenterProxyNode = NULL;
        m_bDataCenterProxyExist = FALSE;
        m_pstMapSvrNode = NULL;
        m_bMapSvrExist = FALSE;
        m_pstBuffActionSvrNode = NULL;
        m_bBuffActionSvrNodeExist = FALSE;
        m_pstAlActionSvrNode = NULL;
        m_bAlActionSvrNodeExist = FALSE;
        m_pstMarchActionSvrNode = NULL;
        m_bMarchActionSvrNodeExist = FALSE;
        m_pstReportUserSvrNode = NULL;
        m_bReportUserSvrNodeExist = FALSE;
        m_pstUserLinkerNode = NULL;
        m_bUserLinkerNodeExist = FALSE;

        ResetAwsInfo();
        ResetDbInfo();
        ResetEventInfo();
        ResetDataCenterInfo();
        ResetMapSvrReq();
        m_stSourceUser.Reset();
        m_stTargetUser.Reset();
        m_tbReport.Reset();

        m_stMapItem.Reset();
        m_ucMapItemFlag = EN_TABLE_UPDT_FLAG__CHANGE;
        m_tbTmpAlGift.Reset();

        m_stCommonResInfo.Reset();
        m_poServLog = NULL;

        m_ucActionRawSecClass = 0;
        m_ucActionRawStatus = 0;
        m_udwTrainRawType = 0;
        m_udwTrainRawNum = 0;
        m_udwRawTuid = 0;
        m_uddwTimeBeg = 0;
        m_uddwTimeEnd = 0;
        m_uddwDownRqstTimeBeg = 0;
        m_uddwDownRqstTimeEnd = 0;

        m_poLongConn = NULL;
        m_ppPackTool = NULL;


        m_uParam.Reset();
        m_tbTmpMap.Reset();
        // lock svr
        m_pstLockSvr = NULL;
        m_bLockSvrExist = 0;
        m_dwLockRetCode = 0;
        m_udwLockUid = 0;
        m_bIsUidNeedLock = FALSE;
        m_udwLockUidB = 0;
        m_bIsUidBNeedLock = FALSE;
        m_udwLockWild = 0;
        m_bIsWildNeedLock = FALSE;
        m_udwLockWildB = 0;
        m_bIsWildBNeedLock = FALSE;
        m_udwLockAid = 0;
        m_bIsAidNeedLock = FALSE;
        m_udwLockAidB = 0;
        m_bIsAidBNeedLock = FALSE;
        m_bLockedData = FALSE;

        m_stReqParam.Reset();
        m_dwMarchResult = 0;
        m_dwRealReportId = 0;
        m_dwReportFlag = 0;

        m_uddwAwsReadSumTime = 0;
        m_uddwAwsWriteSumTime = 0;
        m_uddwAwsReadWriteSumTime = 0;
        m_uddwAwsNoOpSumTime = 0;
        m_uddwDbReadSumTime = 0;
        m_uddwDbWriteSumTime = 0;
        m_uddwDbReadWriteSumTime = 0;
        m_uddwDbNoOpSumTime = 0;
        m_uddwLockSumTime = 0;

        m_stActionTable.Reset();

        m_bNeedRallyHistory = FALSE;
        for(TUINT32 udwIdx = 0; udwIdx < 2; ++udwIdx)
        {
            m_atbTmpRallyHistory[udwIdx].Reset();
        }

        m_udwTmpMarchNum = 0;
        for (TUINT32 udwIdx = 0; udwIdx < 10; udwIdx++)
        {
            m_atbTmpMarch[udwIdx].Reset();
            m_audwTmpMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__UNCHANGE;
        }

        m_stTitleInfoList.Reset();
        m_tbReportId.Reset();

        m_tbPrisonReport.Reset();
        m_vecPrisonReportReceivers.clear();

        m_ddwGemBegin = 0;
        m_strGameEvaluateLog = "";

		m_bCheckValidFlag = TRUE;
        m_ddwCheckUid = 0;

        m_ucReqActionFlag = EN_TABLE_UPDT_FLAG__CHANGE;
        m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;
        m_ucReqAlActionFlag = EN_TABLE_UPDT_FLAG__CHANGE;

        m_tbThrone.Reset();

        m_stDragonNode.Reset();
        m_stMonsterNode.Reset();

        m_ucTmpMapItemFlag = 0;

        m_pTmpBuf = NULL;
        m_udwTmpBufLen = 0;

        m_udwIdolNum = 0;
        for (TUINT32 udwIdx = 0; udwIdx < MAX_IDOL_NUM; udwIdx++)
        {
            m_atbIdol[udwIdx].Reset();
            m_ucIdolFlag[udwIdx] = EN_TABLE_UPDT_FLAG__UNCHANGE;
        }

        m_vecAlReport.clear();
        m_vecReportAlReceiver.clear();
        

        m_objAuPushDataNode.Reset();
        m_pJsonGenerator = NULL;

        bKingChanged = FALSE;
        dwRawThroneAlid = 0;
        m_dwReqMapPushFlag = 0;
    }
};

#pragma pack()

typedef CQueueT<SSession *> CTaskQueue;

#endif
