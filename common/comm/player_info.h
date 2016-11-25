#ifndef _PLAYER_INFO_H_
#define _PLAYER_INFO_H_

#include "base/common/wtse_std_header.h"
#include "game_define.h"
#include "bussiness_struct.h"
#include "aws_table_include.h"
#include "city_info.h"
#include <vector>
#include "game_evaluate_base.h"

using namespace std;

struct SPlayerMightInfo
{
    TINT64      m_ddwBuildingMight;
    TINT64      m_ddwTroopMight;
    TINT64      m_ddwFortMight;
    TINT64      m_ddwKnightMight;

    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }

    TINT64 Sum()
    {
        return m_ddwBuildingMight + m_ddwTroopMight + m_ddwFortMight + m_ddwKnightMight;
    }
};

struct SAlGiftList
{
    TINT32 m_dwGiftNum;
    TbAl_gift m_atbGifts[MAX_AL_IAP_GIFT_NUM_SVR]; //额外保留一些al_gift给gem_recharge用，其中促销可能包含、iap购买还有固定赠送algift
    TUINT8 m_aucUpdateFlag[MAX_AL_IAP_GIFT_NUM_SVR];

    ///////////////////////////////////////////////
    TVOID Reset()
    {
        m_dwGiftNum = 0;
        for(TUINT32 udwIdx = 0; udwIdx < MAX_AL_IAP_GIFT_NUM_SVR; ++udwIdx)
        {
            m_aucUpdateFlag[udwIdx] = EN_TABLE_UPDT_FLAG__UNCHANGE;
        }
    }

    SAlGiftList() : m_dwGiftNum(0)
    {
        for(TUINT32 udwIdx = 0; udwIdx < MAX_AL_IAP_GIFT_NUM_SVR; ++udwIdx)
        {
            m_aucUpdateFlag[udwIdx] = EN_TABLE_UPDT_FLAG__UNCHANGE;
        }
    }

    TbAl_gift& operator[](TINT32 dwNum)
    {
        assert(dwNum >= 0 && dwNum < m_dwGiftNum);
        return m_atbGifts[dwNum];
    }
};

struct SReqParam;

///////////////////////////////////////////////////////////////////
// 用户信息
///////////////////////////////////////////////////////////////////
struct SUserInfo
{
    SUserInfo();

    // seq号,与外层session的seq号一致,外层seq用于框架,userinfo层用于业务逻辑
    TUINT32 m_udwBSeqNo;

    // err_msg回传(用于开发调试阶段)
    string m_strErrMsg;

    TVOID* m_pstReqParam; //llt add //原类型为SReqParam*，由于au hu共用。。故设为TVOID* nemo modify

    //源自数据库//////////////////////////////////////
    TUINT8 m_ucAccountFlag;

    TUINT32 m_udwActionNum;// buffer action only active
    TbAction m_atbAction[MAX_USER_ACTION_NUM]; //存放自己的buffaction 以及不能够被联盟帮助的别的玩家看不到的action
    TUINT8 m_aucActionFlag[MAX_USER_ACTION_NUM];

    TbApns_token m_tbApns_token;

    TbBackpack m_tbBackpack;

    TUINT32 m_udwEquipNum;
    TbEquip m_atbEquip[MAX_USER_EQUIP_NUM];
    TUINT8 m_aucEquipFlag[MAX_USER_EQUIP_NUM];


    TUINT32 m_udwBookmarkNum;
    TbBookmark m_atbBookmark[MAX_BOOKMARK_NUM];
    TUINT8 m_aucBookMarkFlag[MAX_BOOKMARK_NUM];

    SCityInfo m_stCityInfo;

    TbLogin m_tbLogin;

    TUINT32 m_udwWildNum;
    TbMap m_atbWild[MAX_WILD_RETURN_NUM];
    TUINT8 m_aucWildFlag[MAX_WILD_RETURN_NUM];
    TUINT32 m_udwHaveStoneRank;

    TUINT32 m_udwMarchNum;
    TbMarch_action m_atbMarch[MAX_USER_MARCH_NUM];//存放自己发起的普通march 和 联盟里的rally war以及联盟里rally war 的reinforce
    TUINT8 m_aucMarchFlag[MAX_USER_MARCH_NUM];

    TUINT32 m_udwPassiveMarchNum;
    TbMarch_action m_atbPassiveMarch[MAX_USER_MARCH_NUM];//存放自己被打 的 march 自己被普通reinforce 和 联盟被rallywar
    TUINT8 m_aucPassiveMarchFlag[MAX_USER_MARCH_NUM];

    TbPlayer m_tbPlayer;
    TUINT8 m_ucPlayerFlag;
    TbUnique_name m_tbSelfName;
    Json::Value m_jPlayerRankInfo;

    TbQuest m_tbQuest;

    TbTask m_tbTask;
    set<TUINT32> m_setInsertTask;

    TbUser_stat m_tbUserStat;

    // alliance info sort by table name
    TUINT32 m_udwAlAssistAllNum;     // 联盟里所有援助的数量(不管有没有援助过)
    TbAl_assist m_atbAlAssistAll[MAX_AL_ASSIST_NUM];
    TUINT8 m_aucAlAssistAllFlag[MAX_AL_ASSIST_NUM];

    SAlGiftList m_stAlGifts;//联盟的联盟礼物
    TbAl_gift_reward m_atbAlGiftReward[MAX_AL_IAP_GIFT_NUM_SVR];
    TUINT32 m_udwAlGiftRewardNum;

    TbAl_help m_atbAl_help[MAX_AL_HELP_LIST_NUM];// 帮助过的联盟任务
    TUINT8 m_aucAlHelpFlag[MAX_AL_HELP_LIST_NUM];

    TUINT32 m_udwAlMemberNum;
    TbAl_member m_atbAlMember[MAX_ALLIANCE_MEMBER_NUM];

    TUINT32 m_udwAlConsumeNum;    // al store consume
    TbAl_store_consume m_atbAl_store_consume[MAX_AL_CONSUME_NUM];

    TUINT32 m_udwWallNum;     // 联盟里所有的wall
    TbAl_wall m_atbWall[MAX_WALL_MSG_NUM];
    TUINT8 m_aucWallFlag[MAX_WALL_MSG_NUM];
    TUINT32 m_udwWallDelNum;

    TbAl_wall m_tbTmpWall;
    TUINT8 m_ucTmpWallFlag;

    TbAlliance m_tbAlliance;
    TUINT8 m_ucAllianceFlag;

    TUINT32 m_udwSelfAlActionNum;
    TbAlliance_action m_atbSelfAlAction[MAX_USER_AL_ACTION_NUM];//存放自己的建筑action 和 自己盟友申请帮助的建筑action 以及一切自己联盟内部能看到的action
    TUINT8 m_aucSelfAlActionFlag[MAX_USER_AL_ACTION_NUM];

    // rally_info defence action
    TbMarch_action m_atbRallyMarch[MAX_USER_MARCH_NUM * 2];
    TUINT32 m_udwRallyMarchNum;

    TUINT8 m_ucSelfAlmemberFlag;
    TbAl_member m_tbSelfAlmember;

    TbBounty m_tbBounty;
    //辅助变量
    //联盟帮助
    TUINT32 m_udwAlCanHelpActionNum;
    TbAlliance_action *m_patbAlCanHelpAction[MAX_USER_AL_ACTION_NUM];
    // alliance member
    TUINT32 m_udwCurFindMemberNum;
    TbPlayer m_atbAllianceMember[MAX_ALLIANCE_MEMBER_NUM];
    TbPlayer *m_ptbAllianceMember[MAX_ALLIANCE_MEMBER_NUM];
    TbPlayer *m_ptbOutputMember[MAX_ALLIANCE_MEMBER_NUM];
    // alliance 申请相关
    TUINT32 m_udwAllianceReqTotalNum;
    TUINT32 m_udwAllianceReqCurFindNum;
    TbPlayer m_atbAllianceReqPlayer[MAX_ALLIANCE_REQUEST_NUM];
    // 角标
    TUINT32 m_udwCanHelpTaskNum; //可以帮助的任务数量
    TUINT32 m_udwAllianceCanAssistNum; //联盟里用户可援助数量
    TUINT32 m_udwNewAlWallMsgNum; //未读的联盟公告数量

    // mail
    TUINT32 m_udwMailEntryNum;
    TUINT32 m_udwTotalMailNum;
    TUINT32 m_udwTotalMailUnreadNum;
    TbMail_operate m_tbMailOperate;
    SMailEntry m_aMailToReturn[MAX_MAIL_PERPAGE_NUM];

    // mail rsp
    SMailUserRspInfo m_stMailUserInfo;
    TUINT32 m_udwMailNum;
    TbMail m_atbMailList[MAX_MAIL_PERPAGE_NUM];

    // mail recv list
    TUINT32 m_udwMailSendNum;
    TINT32 m_adwMailSendUidList[MAX_REINFORCE_NUM * 2];

    // report
    SReportUserRspInfo m_stReportUserInfo;
    TUINT32 m_udwReportNum;
    TbReport m_atbReportList[MAX_REPORT_PERPAGE_NUM];
    TbReport *m_ptbReport[MAX_REPORT_PERPAGE_NUM];

    // diplomacy
    TUINT32 m_udwDiplomacyNum;
    TUINT32 m_udwDiplomacyDelNum;
    TbDiplomacy m_atbDiplomacy[MAX_DIPLOMACY_NUM];
    TUINT32 m_audwDiplomacyTypeNum[EN_DIPLOMACY_TYPE__END];
    TUINT8 m_aucDiplomacyFlag[MAX_DIPLOMACY_NUM];
    TbDiplomacy m_tbTargetDiplomacy;
    TUINT8 m_ucTargetDiplomacyFlag;

    //event tips
    TUINT32 m_udwEventTipsNum;
    TbEvent_tips m_atbEventTips[MAX_EVENT_TIPS_NUM * 2];
    TUINT8 m_aucEventTipsFlag[MAX_EVENT_TIPS_NUM * 2];

    //tips
    TUINT32 m_udwTipsNum;
    TbTips m_atbTips[MAX_PLAYER_TIPS_NUM * 2];
    TUINT8 m_aucTipsFlag[MAX_PLAYER_TIPS_NUM * 2];

    //broadcast
    TUINT32 m_udwBroadcastNum;
    TbBroadcast m_atbBroadcast[MAX_BROADCAST_NUM_TOTAL];
    TUINT8 m_aucBroadcastFlag[MAX_BROADCAST_NUM_TOTAL];

    TUINT32 m_udwLotteryChestItemId;
    TUINT32 m_udwLotteryChestItemNum;

    //返回给客户端的弹窗
    TUINT32 udwRewardWinType;
    SSpGlobalRes m_stRewardWindow;
    TUINT32 udwRewardWinGetType;

    //new reward window nemo
    TINT32 m_dwRewardWindowNum;
    TbReward_window m_atbRewardWindow[MAX_REWARD_WINDOW_NUM];
    TUINT8 m_aucRewardWindowFlag[MAX_REWARD_WINDOW_NUM];

    //Iap促销描述 nemo
    string m_szIapDesc;

    ////////////////////////////////////////////////
    TUINT64 m_uddwNewActionId;
    bool m_bNewBreakFlag;
    bool m_bFirstJoinAlReward;

    // 游戏评估
    SGameEvaluateExData m_astGameEvaluateExDataRaw[2];  // 原始的ex_data(source/target)
    SGameEvaluateExData m_astGameEvaluateExDataNew[2];  // 处理之后的ex_data(source/target)
    TBOOL m_bGameEvaluateType;                          // false: source; true: both

    // stat err
    TINT32 m_dwErrTroop;
    TINT32 m_dwErrFort;
    TINT32 m_dwErrGem;
    TINT32 m_dwErrProdBonus;
    TINT32 m_dwErrMarchBonus;
    TINT32 m_dwErrProd;
    TINT32 m_dwErrItem;
    TINT32 m_dwErrBuildLv;

    // 客户端展示标记
    TINT32 m_dwShowGemFlag;
    TINT32 m_dwShowItemFlag;
    TINT32 m_dwShowChestFlag;
    TINT32 m_dwShowRemoveFlag;//被动移城flag
    TINT32 m_dwTimeForMoveCityPrepare;

    //active quest
    TUINT32 m_udwCostResource;

    //玩家buffer
    SPlayerBuffInfo m_stPlayerBuffList;
    SPlayerBuffInfo m_stBuffWithoutDragon;

    //玩家积分详情
    SActiveScore m_stScore;

    //连续登录相关和vip相关
    TINT64 m_ddwRawVipPoint;
    TBOOL m_bTodayFirstLogin;//表示本次操作是否当日第一次登录的变量, 在CCommonHandleAfter::UpdatePlayerUtimeAndLoginDay调用之后可以直接使用

    TUINT32 m_udwBeforAlid;

    TUINT32 m_udwBeforPlayerExp;

    TBOOL m_bUpdtPlayerUtime;
    TUINT32 m_udwTmpFinishNum;
    TUINT32 m_audwTmpFinishTaskId[MAX_TASK_CURR_NORMAL_LIMIT + MAX_TASK_CURR_TIME_LIMIT];

    TUINT32 m_udwTmpFinishBountyGoal;

    TUINT64 m_uddwLastMight;  //上次请求最终的Might值
    string m_sLastTroopAndFort;

    TUINT64 m_uddwCurMight;  //本次请求开始的Might值
    string m_sCurTroopAndFort;

    vector< pair<TUINT32, TUINT32> > m_vTmpFinishBountyBase;


    //rating
    TBOOL m_bRatingSwitch;
    TUINT32 m_udwRatingGem;
    TUINT64 m_uddwCurEventId;
    TUINT32 m_udwMoveNewCityId;

    //task
    vector<TUINT32> m_vCheckTaskId;
    vector<TUINT32> m_vecTaskList;

    //活动染色信息
    SUserEventGoals m_stUserEventGoals;
    TBOOL m_bIsSendEventReq;

    TbBlacklist m_atbBlackList[MAX_BLACKLIST_NUM];
    TUINT8 m_aucBlackListFlag[MAX_BLACKLIST_NUM];
    TUINT32 m_udwBlackListNum;

    TUINT8 m_aucSvrAlFlag;
    TbSvr_al m_tbSvrAl;

    TbThrone *ptbThrone;
    
    TINT32 m_dwRefreshDailyQuestNum;
    TINT32 m_dwRefreshAllianceQuestNum;
    TINT32 m_dwRefreshVipQuestNum;
    TINT32 m_dwRefreshpQuesType; // 刷新daily/alliance/vip的类型


    TbRandom_reward m_atbRandomReward[MAX_RANDOM_REWARD_NUM];
    TUINT8 m_aucRandomRewardFlag[MAX_RANDOM_REWARD_NUM];
    TUINT32 m_udwRandomRewardNum;

    //task clear flag
    TBOOL  m_bTaskClearCmd;
    TBOOL  m_bTaskUpdate;

	//data output
    TbData_output m_tbDataOutput;

    //wave@20160712: push_data
    TbMarch_action *m_ptbPushMarchAction;
    TINT32 dwPushMarchActionType;
    TbAlliance_action *m_ptbPushAlAction;
    TINT32 dwPushAlActionType;
    TbTips *m_ptbPushTips;
	
    TVOID Reset();
};

#endif
