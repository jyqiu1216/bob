#ifndef _SSC_SESSION_H_
#define _SSC_SESSION_H_

#include "queue_t.h"
#include "session_mgr_t.h"
#include "base/common/wtsetypedef.h"
#include "my_define.h"
#include "key_value.h"
#include "http_utils.h"
#include "player_info.h"
#include "service_key.h"
#include "game_svr.h"
#include "aws_request.h"
#include "aws_response.h"
#include "db_req_info.h"
#include "db_rsp_info.h"
#include <vector>
#include "event_req_info.h"
#include "event_rsp_info.h"
#include "cache_req_info.h"
#include "cache_rsp_info.h"
#include "translate_req_info.h"
#include "translate_rsp_info.h"
#include "data_center_req_info.h"
#include "data_center_rsp_info.h"
#include "db_struct_define.h"
#include "map_svr_req_info.h"
#include "report_req_info.h"
#include "report_rsp_info.h"
#include "purchase_req_info.h"
#include "purchase_rsp_info.h"
#include "output_conf.h"
#include "zkutil/zk_util.h"
#include "conf.h"

#include "iap_svr_req_info.h"
#include "iap_svr_rsp_info.h"
#include "pushdata_define.h"

#include "rank_svr_req_info.h"
#include "rank_svr_rsp_info.h"

using namespace std;

#pragma pack(1)

enum EOutputCmpType
{
    EN_OUTPUT_CMP_TYPE__NO = 0,
    EN_OUTPUT_CMP_TYPE__CMP
};

// 流程
enum EProcedure
{
	// 流程起始状态
	EN_PROCEDURE__INIT              = 0,
	// 返回client结果
	EN_PROCEDURE__CLIENT_REQUEST,
	EN_PROCEDURE__CLIENT_RESOPNSE,
    // 请求获取id
    EN_PROCEDURE__GLOBAL_ID_REQUEST,
    EN_PROCEDURE__GLOBAL_ID_RESPONSE,
	// 请求SVR内信息
	EN_PROCEDURE__USER_INFO_SEARCH_REQUEST,
	EN_PROCEDURE__USER_INFO_SEARCH_RESPONSE,
	// 更新svr内信息
	EN_PROCEDURE__USER_INFO_UPDATE_REQUEST,
	EN_PROCEDURE__USER_INFO_UPDATE_RESPONSE,
	// 请求全局信息检索结果
	EN_PROCEDURE__USERID_GENERATE_REQUEST,
	EN_PROCEDURE__USERID_GENERATE_RESPONSE,     // 10
	EN_PROCEDURE__ACCOUNT_SEARCH_REQUEST,
	EN_PROCEDURE__ACCOUNT_SEARCH_RESPONSE,
	EN_PROCEDURE__ACCOUNT_UPDATE_REQUEST,
	EN_PROCEDURE__ACCOUNT_UPDATE_RESPONSE,
	EN_PROCEDURE__ACCOUNT_INSERT_REQUEST,
	EN_PROCEDURE__ACCOUNT_INSERT_RESPONSE,
    EN_PROCEDURE__REPORTS_ID_GENERATE_REQUEST,
    EN_PROCEDURE__REPORTS_ID_GENERATE_RESPONSE,
    EN_PROCEDURE__REPORTS_INSERT_REQUEST,
    EN_PROCEDURE__REPORTS_INSERT_RESPONSE,      // 20
	// 进行计算
	EN_PROCEDURE__COMMAND_PROCESS,
	EN_PROCEDURE__COMMAND_CLASS_PROCESS,
	// map信息获取
	EN_PROCEDURE__MAP_SEARCH_REQUEST,
	EN_PROCEDURE__MAP_SEARCH_RESPONSE,
    EN_PROCEDURE__MAP_ACTION_SEARCH_RESPONSE,
	EN_PROCEDURE__MAP_UPDATE_REQUEST,
	EN_PROCEDURE__MAP_UPDATE_RESPONSE,
	// CITY 插入
	EN_PROCEDURE__CITY_INSERT_REQUEST,
	EN_PROCEDURE__CITY_INSERT_RESPONSE,
	// alliance 获取
	EN_PROCEDURE__ALLIANCE_SEARCH_REQUEST,      // 30
	EN_PROCEDURE__ALLIANCE_SEARCH_RESPONSE,
	EN_PROCEDURE__ALLIANCE_RANKING_COUNT_RESPONSE,
    EN_PROCEDURE__ALLIANCE_RANKING_SEARCH_RESPONSE,
    EN_PROCEDURE__ALLIANCE_RANKING_UPDATE_RESPONSE,
	EN_PROCEDURE__ALLIANCE_SEARCH_NAME_REQUEST,
	EN_PROCEDURE__ALLIANCE_SEARCH_NAME_RESPONSE,
	EN_PROCEDURE__ALLIANCE_UPDATE_REQUEST,
	EN_PROCEDURE__ALLIANCE_UPDATE_RESPONSE,
    EN_PROCEDURE__TASK_SEARCH_RESPONSE,
    EN_PROCEDURE__AL_TASK_SEARCH_RESPONSE,      // 40
    EN_PROCEDURE__AL_TASK_UPDATE_RESPONSE,
    EN_PROCEDURE__AL_IAP_GIFT_SEARCH_RESPONSE,
    EN_PROCEDURE__AL_GIFT_GET_RESPONSE,

    EN_PROCEDURE__ALLIANCE_SEARCH_NICK_NAME_REQUEST,
    EN_PROCEDURE__ALLIANCE_SEARCH_NICK_NAME_RESPONSE,
    EN_PROCEDURE__ALLIANCE_SEARCH_NICK_NAME_DELETE_REQUEST,
    EN_PROCEDURE__ALLIANCE_SEARCH_NICK_NAME_DELETE_RESPONSE,
    EN_PROCEDURE__ALLIANCE_DELETE_ALLIANCE_NAME_REQUEST,
    EN_PROCEDURE__ALLIANCE_DELETE_ALLIANCE_NAME_RESPONSE,

	// bookmark
	EN_PROCEDURE__BOOKMARK_SEARCH_REQUEST,
	EN_PROCEDURE__BOOKMARK_SEARCH_RESPONSE,
	EN_PROCEDURE__BOOKMARK_UPDATE_REQUEST,
	EN_PROCEDURE__BOOKMARK_UPDATE_RESPONSE,

	// mail_report
	EN_PROCEDURE__MAIL_REPORT_SEARCH_REQUEST,
	EN_PROCEDURE__MAIL_REPORT_SEARCH_RESPONSE,
	EN_PROCEDURE__MAIL_REPORT_INSERT_REQUEST,   // 50
	EN_PROCEDURE__MAIL_REPORT_INSERT_RESPONSE,
	// player
	EN_PROCEDURE__PLAYER_RANK_SEARCH_REQUEST,
	EN_PROCEDURE__PLAYER_RANK_SEARCH_RESPONSE,

	// stat
	EN_PROCEDURE__PLAYER_STAT_SEARCH_REQUEST,
	EN_PROCEDURE__PLAYER_STAT_SEARCH_RESPONSE,

	// LOCK
	EN_PROCEDURE__PLAYER_LOCK_GET_REQUEST,
	EN_PROCEDURE__PLAYER_LOCK_GET_RESPONSE,
	EN_PROCEDURE__PLAYER_LOCK_RELEASE_REQUEST,
	EN_PROCEDURE__PLAYER_LOCK_RELEASE_RESPONSE,

	// wall
	EN_PROCEDURE__WALL_SEARCH_REQUEST,  // 60
	EN_PROCEDURE__WALL_SEARCH_RESPONSE,

    //iap
    EN_PROCEDURE__IAP_TRANSACTION_UPDATE_REQUEST,
    EN_PROCEDURE__IAP_TRANSACTION_UPDATE_RESPONSE,
    EN_PROCEDURE__IAP_IAPPROMOTION_UPDATE_RESPONSE,

    //auction附加过程
    EN_PROCEDURE__AUCTION_FINISH_REQUEST,
    EN_PROCEDURE__AUCTION_FINISH_RESPONSE,
    EN_PROCEDURE__AUCTION_ACTION_SEARCH_REQUEST,
    EN_PROCEDURE__AUCTION_ACTION_SEARCH_RESPONSE,
    EN_PROCEDURE__AUCTION_ACTION_CHANGE_REQUEST,
    EN_PROCEDURE__AUCTION_ACTION_CHANGE_RESPONSE,   // 70
    EN_PROCEDURE__AUCTION_USERINFO_CHANGE_REQUEST,
    EN_PROCEDURE__AUCTION_USERINFO_CHANGE_RESPONSE,
    EN_PROCEDURE__AUCTION_ACTION_FINISH_REQUEST,
    EN_PROCEDURE__AUCTION_ACTION_FINISH_RESPONSE,

    //event_rank
    EN_PROCEDURE__EVENT_RANK_GET_REQUEST,
    EN_PROCEDURE__EVENT_RANK_GET_RESPONSE,
    EN_PROCEDURE__EVENT_RANK_UPDATE_REQUEST,
    EN_PROCEDURE__EVENT_RANK_UPDATE_RESPONSE,

    //Invite
    EN_PROCEDURE__LOGIN_GET_RESPONSE,

    // recall
    EN_PROCEDURE__RECALL_REQUEST,
    EN_PROCEDURE__RECALL_RESPONSE,

    // notic action 
    EN_PROCEDURE__NOTIC_ACTION_REQUEST,
    EN_PROCEDURE__NOTIC_ACTION_RESPONSE,

    // db数据获取     /*获取联盟数据后拉取 联盟推荐成员的角标*/
    EN_PROCEDURE__DB_DATA_REQUEST,
    EN_PROCEDURE__DB_DATA_RESPONSE,

    // 获取数据后的公共命令字
    EN_PROCEDURE__COMMON_HANDLE_BEFORE,
    // 更新数据前的公共命令字
    EN_PROCEDURE__COMMON_HANDLE_AFTER,
    // 拉数据后生成一份错误恢复json
    EN_PROCEDURE__COMMON_HANDLE,

    //event
    EN_PROCEDURE__EVENT_PROCESS_UPDATE,
    EN_PROCEDURE__EVENT_PROCESS_RESPONSE,

    //db更新数据
    EN_PROCEDURE__DB_DATA_UPDATE_REQUEST,
    EN_PROCEDURE__DB_DATA_UPDATE_RESPONSE,

    //数据中心更新数据流程
    EN_PROCEDURE__DATA_CENTER_GET_REQUEST,
    EN_PROCEDURE__DATA_CENTER_GET_RESPONSE,

    //ReportSvr流程
    EN_PROCEDURE__REPORT_SVR_REQUEST,
    EN_PROCEDURE__REPORT_SVR_RESPONSE,

    //rank svr
    EN_PROCEDURE__RANK_SVR_REQUEST,
    EN_PROCEDURE__RANK_SVR_RESPONSE,

	// 流程结束标记
	EN_PROCEDURE__END,  // 80
	EN_PROCEDURE__SEND_RESULT_BACK,
    EN_PROCEDURE__UPDATE_DATA_OUTPUT_RESPONSE,

    // PUSH DATA
    EN_PROCEDURE__PUSH_DATA_REQUEST = 200,
    EN_PROCEDURE__PUSH_DATA_RESPONSE, 
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
    EN_EXPECT_PROCEDURE__THEME_EVENT = 14,
    EN_EXPECT_PROCEDURE__MAP_SVR = 20,
    EN_EXPECT_PROCEDURE__ACTION_SVR = 21,
    EN_EXPECT_PROCEDURE__REPORT_SVR = 22,
	EN_EXPECT_PROCEDURE__USER_LINKER = 23,
    EN_EXPECT_PROCEDURE__PURCHASE_CHECK = 24,
    EN_EXPECT_PROCEDURE__IAP_SVR = 25,
    EN_EXPECT_PROCEDURE__RANK_SVR = 26,
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

enum ECreateUserInfoFlag
{
	EN_CREATE_FLAG__NO_CREATE = 0,
	EN_CREATE_FLAG__PLAYER = 1,
	EN_CREATE_FLAG__CITY = 2,
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
    TINT64             m_ddwReportId;                      //客户端拉取过的最大的report id
    TINT64             m_ddwMailId;                         //客户端拉取过的最大的mail id
	TUINT32				m_udwAllianceId;					// alliance id;
    TUINT8              m_ucAlliancePos;                    // alliance pos;
	TUINT32				m_udwSeqNo;
	TFLOAT32			m_udwVersion;
	TUINT8				m_ucLoginStatus;					//登录时置1，登录后使用是0;
    TUINT32				m_udwLang;		                    //用户的语言
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
    TINT64              m_ddwReqCost;                       // 客户端的处理时间


	// 创建用户所属城市时使用
	TUINT8				m_ucProvince;
	TUINT32				m_udwSkip;
	TUINT8				m_ucIsNpc;
    TUINT8              m_ucIsSandBox;
    TBOOL               m_bNeedLoginCheck; //是否账号验证，如果不需要，只要uid正确即可，一般用于op cmd

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
    TUINT32             m_udwInReqTime; //客户端带过来的时间参数

	//charles@20150709
	TBOOL				m_bLotteryRefresh;

    string m_szPurchaseToken;
    string m_szPackageName;
    string m_szItemId;
    string m_szPurchaseUid;

	void Reset()
	{
		m_szReqUrl[0] = 0;

        m_ucResType = EN_CONTENT_UPDATE_TYPE__TABLE_INC;

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
        m_szIdfa[0]     = '\0';
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
		for(TUINT32 udwIdx = 0; udwIdx < MAX_REQ_PARAM_KEY_NUM; udwIdx++)
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

struct SCommonResInfo
{
	TINT32				m_dwRetCode;
	TUINT64				m_uddwCostTime;
    TUINT8              m_ucJsonType;
    TBOOL               m_bRefreshCode;//TRUE 表示数据正确  FALSE 表示数据错误
    TBOOL               m_bIsProSysData;

	void Reset()
	{
		m_dwRetCode = 0;
		m_uddwCostTime = 0;
        m_ucJsonType = EN_JSON_TYPE_USER_JSON;
        m_bRefreshCode = FALSE;
        m_bIsProSysData = FALSE;
	}
};

struct SChatResInfo
{
	TUINT32				m_udwResServiceType;
	TINT32				m_dwRetCode;
	TUINT32				m_udwCostTime;
	TUINT32				m_udwTotalResNum;
	TUINT32				m_udwCurResNum;
	TUINT32				m_udwResDataLen;
	TUCHAR				m_szResData[MAX_CHAT_PACKAGE_BUF_LEN];

	TVOID Reset()
	{
		m_udwResServiceType = 0;
		m_dwRetCode = EN_RET_CODE__UNEXPECTED;
		m_udwCostTime = 0;
		m_udwTotalResNum = 0;
		m_udwCurResNum = 0;
		m_udwResDataLen = 0;
	}
};

struct SSession
{
    /********************************************************************************************
                        外部链接信息和序列号信息
                        *********************************************************************************************/
    TUINT32             m_udwSeqNo;
    TUINT32             m_udwPbSeq;
    TUINT32             m_udwClientSeqNo;
    TCHAR               m_szClientIp[32];
    TUINT32             m_udwLinkerCmdRef;
    LongConnHandle      m_stClientHandle;
    TUINT32             m_udwRequestType;
    TUINT32             m_udwExpectProcedure;
    TUINT32             m_udwNextProcedure;
    TUINT32             m_udwWaitFinishProcedure;
    TUINT32             m_udwCommand;
    TUINT32             m_udwCommandStep;
    TINT32              m_dwOutputCmpType;
    TUINT32             m_udwPackSeq;

    TUINT32             m_udwProcessSeq; // 处理流程序号
    TUINT32             m_udwGemCost; // for log
    TUINT8              m_ucFakeRecharge; // for log
    TINT64              m_ddwGemBegin; // for log
    TUINT8              m_ucPromoteReward;
    TINT32              m_dwPromoteTpye;        // gem促销的cb_log(表示gem促销的购买类型)

    TUINT8              m_ucAddPersonAlGiftStatus; // 增加个人的联盟礼物状态(1:没有联盟;2:no pack id;3:no reward level;4:成功赠送)

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
    SDownNode            *m_pstDbProxyNode;
    TBOOL               m_bDbProxyNodeExist;
    SDownNode            *m_pstEventProxyNode; //daemon
    TBOOL               m_bEventProxyExist;
    SDownNode            *m_pstThemeEventProxyNode; //kuro for theme event
    TBOOL               m_bThemeEventProxyExist;
    SDownNode            *m_pstCacheProxyNode;
    TBOOL               m_bCacheProxyExist;
    SDownNode            *m_pstTranslateProxyNode;
    TBOOL               m_bTranslateProxyExist;
    SDownNode            *m_pstDataCenterProxyNode;  //charles
    TBOOL               m_bDataCenterProxyExist;
    SDownNode           *m_pstMapSvrNode;
    TBOOL               m_bMapSvrExist;
    SDownNode            *m_pstBuffActionSvrNode;
    TBOOL               m_bBuffActionSvrNodeExist;
    SDownNode            *m_pstAlActionSvrNode;
    TBOOL               m_bAlActionSvrNodeExist;
    SDownNode            *m_pstMarchActionSvrNode;
    TBOOL               m_bMarchActionSvrNodeExist;
    SDownNode            *m_pstReportUserSvrNode;
    TBOOL               m_bReportUserSvrNodeExist;
    SDownNode            *m_pstUserLinkerNode;
    TBOOL               m_bUserLinkerNodeExist;
    SDownNode            *m_pstPurchaseCheckNode;
    TBOOL               m_bPurchaseCheckNodeExist;
    SDownNode            *m_pstMailUserSvrNode;
    TBOOL               m_bMailUserSvrNodeExist;
    SDownNode           *m_pstIapSvrNode;   //kuro
    TBOOL               m_bIapSvrExist;
    SDownNode           *m_pstRankSvrNode;
    TBOOL               m_bRankSvrExist;
    
    // lock svr
    SDownNode            *m_pstLockSvr;
    TBOOL               m_bLockSvrExist;
    TINT32              m_dwLockRetCode;
    TBOOL               m_bLockedData; // 是否有加锁
    TINT64              m_ddwLockedTaskId;

    /********************************************************************************************
                        网络消息内存
                        *********************************************************************************************/
    TUCHAR              m_szClientReqBuf[MAX_HTTP_REQ_LEN];					// client请求Buf
    TUINT32             m_udwClientReqBufLen;								// client请求Buf长度
    TINT32              m_dwClientReqEnType;                                // 请求加密方式：0非加密，1加密
    TINT32              m_dwClientResType;                                  // 响应方式：0全量，1增量
    TINT32              m_dwClientReqMode;                                  // 0:http, 1:tcp-长连接

    SChatResInfo        m_stChatResInfo;
    TBOOL               m_bChatResFlag;
    TCHAR               m_szClientRspBuf[MAX_NETIO_PACKAGE_BUF_LEN];        // client返包Buf
    TINT32              m_dwFinalPackLength;                                // client返包优化后大小
    TINT32              m_dwOriPackLength;

    TCHAR               m_szTmpClientRspBuf[MAX_NETIO_PACKAGE_BUF_LEN];        // 命令字出错后 返回包buff
    TINT32              m_dwTmpFinalPackLength;                                // 命令字出错后 client返包优化后大小
    TINT32              m_dwTmpOriPackLength;

    /********************************************************************************************
                        内部保留信息
                        *********************************************************************************************/
    SReqParam           m_stReqParam;	// 请求参数
    SUserInfo           m_stUserInfo;	// 用户信息结构体

    TbThrone m_tbThrone;
    TINT64 m_ddwRawThroneAid;

    TUINT32 m_udwIdolNum;
    TbIdol m_atbIdol[MAX_IDOL_NUM];
    TUINT8 m_ucIdolFlag[MAX_IDOL_NUM];

    // 临时变量
    TUINT32             m_udwTmpWildNum;
    TbMap               m_atbTmpWild[MAX_WILD_RETURN_NUM];
    TUINT8              m_aucTmpWildFlag[MAX_WILD_RETURN_NUM];
    TUINT32             m_udwTmpActionNum;
    TbAction            m_atbTmpAction[MAX_USER_ACTION_NUM];

    TUINT32             m_udwTmpMarchNum;
    TbMarch_action      m_atbTmpMarch[MAX_USER_MARCH_NUM];

    TUINT32             m_udwTmpSvrNum;
    TUINT32             m_udwTmpSvrStatNum;
    TbSvr_stat          m_atbTmpSvrStat[MAX_GAME_SVR_NUM];
    TUINT32             m_udwTmpThroneMapNum;
    TbMap               m_atbTmpThroneMap[MAX_GAME_SVR_NUM];

    SCommonResInfo      m_stCommonResInfo;	// 结果相关
    TBOOL               m_bNeedCreatAccountFlag;
    TBOOL               m_bAccoutUpt;

    TbMap	            m_tbTmpMap;

    TINT64              m_ddwNewMailId;
    TBOOL               m_bNeedMailMusic;
    TINT32              m_dwMailReceiverId;
    TbMail	            m_tbMail;
    TUINT8              m_ucMailTargeType; // 0表示私人、1目标联盟全体、2表示给目标联盟的盟主和副盟主
    TUINT32             m_audwMailRUidList[DEFAULT_PERPAGE_NUM];
    TbPlayer            m_atbTmpPlayer[DEFAULT_PERPAGE_NUM];
    TUINT32             m_udwTmpPlayerNum;
    TUINT32             m_udwMailRUidNum;
    TUINT32             m_udwMailSendStep;
    TbUnique_name       m_atbMailReceiver[DEFAULT_PERPAGE_NUM];

    string              m_astrNameList[DEFAULT_PERPAGE_NUM];

    TbAl_store_consume  m_tbTmpAlConsume;
    TUINT8              m_ucTmpAlConsumeFlag;

    TbPlayer            m_tbTmpPlayer;
    TUINT8              m_ucTmpPlayerFlag;
    Json::Value         m_jTmpPlayerRankInfo;

    TbAction            m_tbTmpAction;
    TUINT8              m_ucTmpActionFlag;

    TbAlliance          m_tbTmpAlliance;

    TUINT8              m_ucTmpMapItemFlag;
    map<TUINT64, TUINT32>   m_mBlockId;

    TbAl_member         m_atbTmpAlmember[1000];
    TUINT32             m_udwTmpAlmemberNum;

    TbEquip             m_atbTmpEquip[MAX_USER_EQUIP_NUM];
    TUINT32             m_udwTmpEquipNum;

    TbAlliance          m_atbFriendAl[MAX_DIPLOMACY_NUM];
    TUINT32             m_udwFriendAlNum;
    TbAlliance          m_atbHostileAl[MAX_DIPLOMACY_NUM];
    TUINT32             m_udwHostilesAlNum;

    TbRally_history     m_atbTmpRallyHistory[MAX_RALLY_HISTORY];
    TUINT32             m_udwTmpRallyHistoryNum;

    TbAltar_history     m_atbTmpAltarHistory[MAX_RALLY_HISTORY];
    TUINT32             m_udwTmpAltarHistoryNum;

    STitleInfoList      m_stTitleInfoList;

    TbReport            m_tbTmpReport;

    ComputeTime         m_stRecommendTime;
    PlayerRecommend     m_aRecommendPlayer[MAX_RECOMMEND_NUM];
    TUINT32             m_udwRecommendNum;

    TUINT32             m_udwGembuy;
    TUINT32             m_udwCastlelv;

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

    TUINT32             m_nCreateRefindTime;

    /********************************************************************************************
                        时间参数
                        *********************************************************************************************/
    TUINT64             m_uddwTimeBeg;     // 收到请求的时间
    TUINT64             m_uddwTimeEnd;     // 发送前端回应的时间
    TUINT64             m_uddwDownRqstTimeBeg;
    TUINT64             m_uddwDownRqstTimeEnd;
    TUINT64             m_uddwProcessBegTime;
    TUINT64             m_uddwProcessEndTime;

    TUINT32             m_udwLoytalAdd; //for cb log, 本次请求获得的忠诚度
    TUINT32             m_udwDailyQuestLevel; //for cb log, 本次请求收集的quest level
    TUINT32             m_udwAllianceQuestLevel; //for cb log, 本次请求收集的quest level

    // troop 监控
    TBOOL               m_bGetTroop;
    TINT64              m_ddwTroopBegin; // for log
    TINT64              m_ddwTroopChange; // for log

    //aws info
    vector<AwsReqInfo*> m_vecAwsReq;
    vector<AwsRspInfo*> m_vecAwsRsp;

    //db info
    vector<DbReqInfo*> m_vecDbReq;
    vector<DbRspInfo*> m_vecDbRsp;

    //event
    vector<EventReqInfo*> m_vecEventReq;
    vector<EventRspInfo*> m_vecEventRsp;

    // cache
    vector<CacheReqInfo*> m_vecCacheReq;
    vector<CacheRspInfo*> m_vecCacheRsp;

    // translate
    vector<TranslateReqInfo*> m_vecTranslateReq;
    vector<TranslateRspInfo*> m_vecTranslateRsp;

    // data center
    vector<DataCenterReqInfo*> m_vecDataCenterReq;
    vector<DataCenterRspInfo*> m_vecDataCenterRsp;

    //map_svr
    vector<MapSvrReqInfo*> m_vecMapSvrReq;

    //Iap_svr
    vector<IapSvrReqInfo *> m_vecIapSvrReq;
    vector<IapSvrRspInfo*> m_vecIapSvrRsp;

    //report_svr
    vector<ReportReqInfo*> m_vecReportReq;
    vector<ReportRspInfo*> m_vecReportRsp;

    //purchase
    PurchaseReqInfo m_stPurchaseReq;
    PurchaseRspInfo m_stPurchaseRsp;

    //report_svr
    vector<RankSvrReqInfo*> m_vecRankSvrReq;
    vector<RankSvrRspInfo*> m_vecRankSvrRsp;

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
    TBOOL   m_bBreakMapEndTime;

    TUINT32 m_udwEventRqstType; //1:update 2:get_info 3:get_all_info

    TBOOL m_bGotoOtherCmd;
    Json::Value m_JsonValue;
    TBOOL m_bHasPromoteData;
    Json::Value m_jCityInfo;
    TINT32 m_dwOldRequestAid;

    TBOOL m_bIsNeedCheck;
    TbParam m_tbTmpGlobalParam;
    TUINT32 m_udwLastBubbleTime;

    TINT32 m_udwNewPlayerAlLeaveFlag;

    TUINT32 m_udwTradeInRangeNum;
    TradeCityInfo m_astTradeCityInfoInRange[MAX_TRADE_NUM];
    TUINT32 m_udwTradeInRandomNum;
    TradeCityInfo m_astTradeCityInfoInRandom[MAX_TRADE_NUM];

    TradeCityInfo m_stRawCityInfo;
    TradeCityInfo m_stSelfCityInfo;

    TbBlacklist m_tbTmpBlackList;

    TBOOL m_bNeedRefreshData;  //是否需要从数据中心拉取数据回来更新
    TBOOL m_abRefreshFlag[EN_REFRESH_DATA_TYPE__END];  //标记需要拉取哪些数据

    TINT32 m_dwDragonUnlockFlag;
    TINT32 m_dwKfLv;

    TBOOL m_bEventInfoOk;
    string m_sEventInfo;

    string m_sRspJsonContent;

    //wave@push_data
    CHuPushDataNode m_objAuPushDataNode;
    TVOID   *m_pJsonGenerator;

    void ResetAwsReq()
    {
        for(unsigned int i = 0; i < m_vecAwsReq.size(); ++i)
        {
            delete m_vecAwsReq[i];
        }
        m_vecAwsReq.clear();
    }
    void ResetAwsRsp()
    {
        for(unsigned int i = 0; i < m_vecAwsRsp.size(); ++i)
        {
            delete m_vecAwsRsp[i];
        }
        m_vecAwsRsp.clear();
    }
    void ResetDbReq()
    {
        for(unsigned int i = 0; i < m_vecDbReq.size(); ++i)
        {
            delete m_vecDbReq[i];
        }
        m_vecDbReq.clear();
    }
    void ResetDbRsp()
    {
        for(unsigned int i = 0; i < m_vecDbRsp.size(); ++i)
        {
            delete m_vecDbRsp[i];
        }
        m_vecDbRsp.clear();
    }
    void ResetAwsInfo()
    {
        ResetAwsReq();
        ResetAwsRsp();
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

    void ResetCacheReq()
    {
        for(unsigned int i = 0; i < m_vecCacheReq.size(); ++i)
        {
            delete m_vecCacheReq[i];
        }
        m_vecCacheReq.clear();
    }
    void ResetCacheRsp()
    {
        for(unsigned int i = 0; i < m_vecCacheRsp.size(); ++i)
        {
            delete m_vecCacheRsp[i];
        }
        m_vecCacheRsp.clear();
    }
    void ResetCacheInfo()
    {
        ResetCacheReq();
        ResetCacheRsp();
    }


    void ResetTranslateReq()
    {
        for(unsigned int i = 0; i < m_vecTranslateReq.size(); ++i)
        {
            delete m_vecTranslateReq[i];
        }
        m_vecTranslateReq.clear();
    }
    void ResetTranslateRsp()
    {
        for(unsigned int i = 0; i < m_vecTranslateRsp.size(); ++i)
        {
            delete m_vecTranslateRsp[i];
        }
        m_vecTranslateRsp.clear();
    }
    void ResetTranslateInfo()
    {
        ResetTranslateReq();
        ResetTranslateRsp();
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
    void ResetIapSvrReq()
    {
        for (unsigned int i = 0; i < m_vecIapSvrReq.size(); ++i)
        {
            delete m_vecIapSvrReq[i];
        }
        m_vecIapSvrReq.clear();
    }
    void ResetIapSvrRsp()
    {
        for (unsigned int i = 0; i < m_vecIapSvrRsp.size(); ++i)
        {
            delete m_vecIapSvrRsp[i];
        }
        m_vecIapSvrRsp.clear();
    }
    void ResetIapSvrInfo()
    {
        ResetIapSvrReq();
        ResetIapSvrRsp();
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

    void ResetRankSvrReq()
    {
        for (unsigned int i = 0; i < m_vecRankSvrReq.size(); ++i)
        {
            delete m_vecRankSvrReq[i];
        }
        m_vecRankSvrReq.clear();
    }
    void ResetRankSvrRsp()
    {
        for (unsigned int i = 0; i < m_vecRankSvrRsp.size(); ++i)
        {
            delete m_vecRankSvrRsp[i];
        }
        m_vecRankSvrRsp.clear();
    }
    void ResetRankSvrInfo()
    {
        ResetRankSvrReq();
        ResetRankSvrRsp();
    }

    void ReleaseDownService()
    {
        CDownMgr *poDownMgr = CDownMgr::Instance();
        if(m_bLockSvrExist)
        {
            poDownMgr->zk_ReleaseNode(DOWN_NODE_TYPE__LOCK_SVR, m_pstLockSvr);
        }
        if(m_bAwsProxyNodeExist)
        {
            poDownMgr->zk_ReleaseNode(DOWN_NODE_TYPE__AWS_PROXY, m_pstAwsProxyNode);
        }
        if (m_bDbProxyNodeExist)
        {
            poDownMgr->zk_ReleaseNode(DOWN_NODE_TYPE__DB_PROXY, m_pstDbProxyNode);
        }
        if (m_bEventProxyExist)
        {
            poDownMgr->zk_ReleaseNode(DOWN_NODE_TYPE__EVENT_PROXY, m_pstEventProxyNode);
        }
        if (m_bThemeEventProxyExist)
        {
            poDownMgr->zk_ReleaseNode(DOWN_NODE_TYPE__THEME_EVENT_PROXY, m_pstThemeEventProxyNode);
        }
        if (m_bCacheProxyExist)
        {
            poDownMgr->zk_ReleaseNode(DOWN_NODE_TYPE__CACHE_PROXY, m_pstCacheProxyNode);
        }
        if (m_bTranslateProxyExist)
        {
            poDownMgr->zk_ReleaseNode(DOWN_NODE_TYPE__TRANSLATE_PROXY, m_pstTranslateProxyNode);
        }
        if (m_bDataCenterProxyExist)
        {
            poDownMgr->zk_ReleaseNode(DOWN_NODE_TYPE__DATA_CENTER_PROXY, m_pstDataCenterProxyNode);
        }
        if (m_bMapSvrExist)
        {
            poDownMgr->zk_ReleaseNode(DOWN_NODE_TYPE__MAP_SVR_PROXY, m_pstMapSvrNode);
        }
        if (m_bBuffActionSvrNodeExist)
        {
            poDownMgr->zk_ReleaseNode(DOWN_NODE_TYPE__BUFF_ACTION_SVR, m_pstBuffActionSvrNode);
        }
        if (m_bAlActionSvrNodeExist)
        {
            poDownMgr->zk_ReleaseNode(DOWN_NODE_TYPE__AL_ACTION_SVR, m_pstAlActionSvrNode);
        }
        if (m_bMarchActionSvrNodeExist)
        {
            poDownMgr->zk_ReleaseNode(DOWN_NODE_TYPE__MARCH_ACTION_SVR, m_pstMarchActionSvrNode);
        }
        if (m_bReportUserSvrNodeExist)
        {
            poDownMgr->zk_ReleaseNode(DOWN_NODE_TYPE__REPORT_USER_SVR, m_pstReportUserSvrNode);
        }
        if (m_bUserLinkerNodeExist)
        {
            poDownMgr->zk_ReleaseNode(DOWN_NODE_TYPE__USER_LINKER, m_pstUserLinkerNode);
        }
        if (m_bPurchaseCheckNodeExist)
        {
            poDownMgr->zk_ReleaseNode(DOWN_NODE_TYPE__PURCHAES_CHECK, m_pstPurchaseCheckNode);
        }
        if (m_bMailUserSvrNodeExist)
        {
            poDownMgr->zk_ReleaseNode(DOWN_NODE_TYPE__PURCHAES_CHECK, m_pstMailUserSvrNode);
        }
        if (m_bIapSvrExist)
        {
            poDownMgr->zk_ReleaseNode(DOWN_NODE_TYPE__IAP_SVR, m_pstIapSvrNode);
        }
        if (m_bRankSvrExist)
        {
            poDownMgr->zk_ReleaseNode(DOWN_NODE_TYPE__RANK_SVR, m_pstRankSvrNode);
        }
    }

    void Reset()
    {
        ReleaseDownService();

        m_udwSeqNo = 0;
        m_udwPbSeq = 0;
        m_udwClientSeqNo = 0;
        m_szClientIp[0] = 0;
        m_udwLinkerCmdRef = 0;
        memset((char*)&m_stClientHandle, 0, sizeof(LongConnHandle));
        m_udwRequestType = 0;
        m_udwExpectProcedure = 0;
        m_udwNextProcedure = 0;
        m_udwWaitFinishProcedure = 0; //nemo
        m_udwCommandStep = 0;
        m_udwProcessSeq = 0;
        m_udwGemCost = 0;
        m_ddwGemBegin = 0;

        m_dwOutputCmpType = EN_OUTPUT_CMP_TYPE__NO;

        m_udwTmpWildNum = 0;
        m_udwTmpActionNum = 0;

        m_ucIsUsing = 0;
        m_bProcessing = FALSE;

        m_udwContentType = EN_CONTENT_TYPE__STRING;

        m_bAwsProxyNodeExist = FALSE;
        m_pstAwsProxyNode = NULL;
        m_bDbProxyNodeExist = FALSE;
        m_pstDbProxyNode = NULL;
        m_bAccoutUpt = FALSE;
        m_pstEventProxyNode = NULL;
        m_bEventProxyExist = FALSE;
        m_pstThemeEventProxyNode = NULL;
        m_bThemeEventProxyExist = FALSE;
        m_pstCacheProxyNode = NULL;
        m_bCacheProxyExist = FALSE;
        m_pstTranslateProxyNode = NULL;
        m_bTranslateProxyExist = FALSE;
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
        m_pstPurchaseCheckNode = NULL;
        m_bPurchaseCheckNodeExist = FALSE;
        m_pstMailUserSvrNode = NULL;
        m_bMailUserSvrNodeExist = FALSE;
        m_pstIapSvrNode = NULL;
        m_bIapSvrExist = FALSE;
        m_bRankSvrExist = FALSE;
        m_pstRankSvrNode = NULL;

        m_szClientReqBuf[0] = 0;
        m_udwClientReqBufLen = 0;
        m_dwClientReqEnType = 0; //是否加密标记
        m_dwClientResType = 0;

        m_bChatResFlag = FALSE;
        m_szClientRspBuf[0] = '\0';
        m_dwFinalPackLength = 0;
        m_dwOriPackLength = 0;

        m_szTmpClientRspBuf[0] = '\0';
        m_dwTmpFinalPackLength = 0;
		m_dwTmpOriPackLength = 0;
        m_stReqParam.Reset();
        m_stCommonResInfo.Reset();
        m_stUserInfo.Reset();
        m_bNeedCreatAccountFlag = FALSE;

        m_stUserInfo.m_pstReqParam = &m_stReqParam; //llt add

        m_uddwTimeBeg = 0;
        m_uddwTimeEnd = 0;

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

        // lock svr
        m_pstLockSvr = NULL;
        m_bLockSvrExist = 0;
        m_dwLockRetCode = 0;
        m_bLockedData = FALSE;
        m_ddwLockedTaskId = 0;

        m_udwLoytalAdd = 0;
        m_udwDailyQuestLevel = 0;
        m_udwAllianceQuestLevel = 0;

        //aws info
        ResetAwsInfo();
        //db info
        ResetDbInfo();

        //event info 
        ResetEventInfo();

        // cache info
        ResetCacheInfo();

        // translate info
        ResetTranslateInfo();

        // data center info
        ResetDataCenterInfo();

        ResetReportInfo();

        ResetIapSvrInfo();

        m_ucFakeRecharge = 0;
        m_ucPromoteReward = 0;
        m_dwPromoteTpye = 0;

        m_ddwTroopBegin = 0;
        m_ddwTroopChange = 0;
        m_bGetTroop = FALSE;

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
        m_ucTmpMapItemFlag = 0;
        m_bBreakMapEndTime = FALSE;

        m_udwEventRqstType = 0;

        for(TUINT32 udwIdx = 0; udwIdx < MAX_DIPLOMACY_NUM; ++udwIdx)
        {
            m_atbFriendAl[udwIdx].Reset();
            m_atbHostileAl[udwIdx].Reset();
        }
        m_udwFriendAlNum = 0;
        m_udwHostilesAlNum = 0;

        m_tbTmpMap.Reset();
        m_mBlockId.clear();

        m_ddwNewMailId = 0;
        m_bNeedMailMusic = FALSE;
        m_dwMailReceiverId = 0;
        m_tbMail.Reset();
        m_ucMailTargeType = 0;
        m_udwMailRUidNum = 0;
        m_udwMailSendStep = 0;
        for(TUINT32 udwIdx = 0; udwIdx < DEFAULT_PERPAGE_NUM; ++udwIdx)
        {
            m_atbMailReceiver[udwIdx].Reset();
        }
        for(TUINT32 udwIdx = 0; udwIdx < DEFAULT_PERPAGE_NUM; ++udwIdx)
        {
            m_atbTmpPlayer[udwIdx].Reset();
        }
        m_udwTmpPlayerNum = 0;
        
        for(TUINT32 udwIdx = 0; udwIdx < DEFAULT_PERPAGE_NUM; ++udwIdx)
        {
           m_astrNameList[udwIdx].clear();
        }

        //for(TUINT32 udwIdx = 0; udwIdx < MAX_TMP_AL_GIFT_NUM; ++udwIdx)
        //{
        //    m_tbTmpAlGift[udwIdx].Reset();
        //    m_ucTmpAlGiftFlag[udwIdx] = 0;
        //}
        //m_udwTmpAlGiftNum = 0;

        m_tbTmpAlConsume.Reset();
        m_ucTmpAlConsumeFlag = 0;

        m_tbTmpPlayer.Reset();
        m_ucTmpPlayerFlag = 0;
        m_jTmpPlayerRankInfo.clear();

        m_tbTmpAction.Reset();
        m_ucTmpActionFlag = 0;
        memset(m_aucTmpWildFlag, 0, sizeof(m_aucTmpWildFlag));

        m_tbTmpAlliance.Reset();

        for(TUINT32 udwIdx = 0; udwIdx < 1000/*MAX_ALLIANCE_MEMBER_NUM*/; ++udwIdx)
        {
            m_atbTmpAlmember[udwIdx].Reset();
        }
        m_udwTmpAlmemberNum = 0;

        for(TUINT32 udwIdx = 0; udwIdx < MAX_WILD_RETURN_NUM; ++udwIdx)
        {
            m_atbTmpWild[udwIdx].Reset();
        }
        for(TUINT32 udwIdx = 0; udwIdx < MAX_USER_ACTION_NUM; ++udwIdx)
        {
            m_atbTmpAction[udwIdx].Reset();
        }
        m_JsonValue.clear();
        m_bHasPromoteData = FALSE;
        m_bGotoOtherCmd = FALSE;

        for(TUINT32 udwIdx = 0; udwIdx < MAX_USER_MARCH_NUM; ++udwIdx)
        {
            m_atbTmpMarch[udwIdx].Reset();
        }
        m_udwTmpMarchNum = 0;

        for(TUINT32 udwIdx = 0; udwIdx < MAX_RALLY_HISTORY; ++udwIdx)
        {
            m_atbTmpRallyHistory[udwIdx].Reset();
        }
        m_udwTmpRallyHistoryNum = 0;

        for(TUINT32 udwIdx = 0; udwIdx < MAX_RALLY_HISTORY; ++udwIdx)
        {
            m_atbTmpAltarHistory[udwIdx].Reset();
        }
        m_udwTmpAltarHistoryNum = 0;

        m_stTitleInfoList.Reset();
        m_udwTmpEquipNum = 0;
        for(TUINT32 udwIdx = 0; udwIdx < MAX_USER_EQUIP_NUM;++udwIdx)
        {
            m_atbTmpEquip[udwIdx].Reset();
        }

        m_tbTmpReport.Reset();
        m_jCityInfo.clear();

        m_udwTmpSvrNum = 0;
        m_udwTmpSvrStatNum = 0;
        m_udwTmpThroneMapNum = 0;
        for(TUINT32 udwIdx = 0; udwIdx < MAX_GAME_SVR_NUM; udwIdx++)
        {
            m_atbTmpSvrStat[udwIdx].Reset();
            m_atbTmpThroneMap[udwIdx].Reset();
        }

        m_dwOldRequestAid = 0;
        m_nCreateRefindTime = 0;

        m_pJsonGenerator = NULL;
        m_objAuPushDataNode.Reset();

        m_uddwProcessBegTime = 0;
        m_uddwProcessEndTime = 0;

        m_bIsNeedCheck = FALSE;
        m_tbTmpGlobalParam.Reset();
        m_udwLastBubbleTime = 0;

        m_udwNewPlayerAlLeaveFlag = 0;

        m_stRecommendTime.Reset();
        for(TUINT32 udwIdx = 0; udwIdx < MAX_RECOMMEND_NUM; ++udwIdx)
        {
            m_aRecommendPlayer[udwIdx].Reset();
        }
        m_udwRecommendNum = 0;

        m_udwGembuy = 0;
        m_udwCastlelv = 0;

        m_udwTradeInRangeNum = 0;
        for (TUINT32 udwIdx = 0; udwIdx < MAX_TRADE_NUM; udwIdx++)
        {
            m_astTradeCityInfoInRange[udwIdx].Reset();
        }

        m_udwTradeInRandomNum = 0;
        for (TUINT32 udwIdx = 0; udwIdx < MAX_TRADE_NUM; udwIdx++)
        {
            m_astTradeCityInfoInRandom[udwIdx].Reset();
        }

        m_stRawCityInfo.Reset();
        m_stSelfCityInfo.Reset();

        m_tbTmpBlackList.Reset();

        m_bNeedRefreshData = FALSE;
        for (TUINT32 udwIdx = 0; udwIdx < EN_REFRESH_DATA_TYPE__END; ++udwIdx)
        {
            m_abRefreshFlag[udwIdx] = FALSE;
        }

        m_tbThrone.Reset();
        m_ddwRawThroneAid = 0;

        m_dwDragonUnlockFlag = 0;
        m_dwKfLv = 0;

        m_sEventInfo.clear();

        m_bEventInfoOk = FALSE;

        ResetMapSvrInfo();

        m_stPurchaseReq.Reset();
        m_stPurchaseRsp.Reset();

        ResetRankSvrInfo();

        m_sRspJsonContent.clear();

        m_udwIdolNum = 0;
        for (TUINT32 udwIdx = 0; udwIdx < MAX_IDOL_NUM; udwIdx++)
        {
            m_atbIdol[udwIdx].Reset();
            m_ucIdolFlag[udwIdx] = EN_TABLE_UPDT_FLAG__UNCHANGE;
        }
    }
};

#pragma pack()

typedef CQueueT<SSession *> CTaskQueue;
typedef CSessionMgrT<SSession> CSessionMgr;

#endif

