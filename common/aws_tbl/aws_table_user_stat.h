#ifndef _AWS_TABLE_USER_STAT_H_
#define _AWS_TABLE_USER_STAT_H_

#include "aws_table_common.h"
#include "field_property.h"

#define EN_AWS_TABLE_USER_STAT "user_stat"

enum ETbUser_statField
{
	TbUSER_STAT_FIELD_UID = 0,
	TbUSER_STAT_FIELD_NEWEST_REPORTID = 1,
	TbUSER_STAT_FIELD_UNREAD_REPORT = 2,
	TbUSER_STAT_FIELD_NEWEST_MAILID = 3,
	TbUSER_STAT_FIELD_UNREAD_MAIL = 4,
	TbUSER_STAT_FIELD_AS_GET_T = 5,
	TbUSER_STAT_FIELD_BIRTH_R = 6,
	TbUSER_STAT_FIELD_BIRTH_M = 7,
	TbUSER_STAT_FIELD_TIPS = 8,
	TbUSER_STAT_FIELD_WALL_GET_T = 9,
	TbUSER_STAT_FIELD_MARK = 10,
	TbUSER_STAT_FIELD_STAR_MAIL = 11,
	TbUSER_STAT_FIELD_RETURN_MAILID = 12,
	TbUSER_STAT_FIELD_RETURN_REPORTID = 13,
	TbUSER_STAT_FIELD_REINFORCE_NUM = 14,
	TbUSER_STAT_FIELD_KILL_TROOP_NUM = 15,
	TbUSER_STAT_FIELD_LAST_EVENT_WIN_ID = 16,
	TbUSER_STAT_FIELD_AL_EVENT_TIPS_TIME = 17,
	TbUSER_STAT_FIELD_PLAYER_RECOMMEND_TIME = 18,
	TbUSER_STAT_FIELD_BROADCAST_TIME = 19,
	TbUSER_STAT_FIELD_MAIL_FLAG = 20,
	TbUSER_STAT_FIELD_TOP_QUEST = 21,
	TbUSER_STAT_FIELD_TOP_QUEST_FINISH = 22,
	TbUSER_STAT_FIELD_LORD_LEVEL_FINISH = 23,
	TbUSER_STAT_FIELD_LAST_HELP_BUBBLE_SET_TIME = 24,
	TbUSER_STAT_FIELD_LAST_HELP_BUBBLE_TIME_OUT = 25,
	TbUSER_STAT_FIELD_DAILY_QUEST_FINISH_NUM = 26,
	TbUSER_STAT_FIELD_AL_QUEST_FINISH_NUM = 27,
	TbUSER_STAT_FIELD_VIP_QUEST_FINISH_NUM = 28,
	TbUSER_STAT_FIELD_LAST_MIGHT = 29,
	TbUSER_STAT_FIELD_LAST_TROOP_FORT = 30,
	TbUSER_STAT_FIELD_CON_LOGIN_DAYS = 31,
	TbUSER_STAT_FIELD_MAX_DEFEAT_MONSTER_LV = 32,
	TbUSER_STAT_FIELD_EQUIP_GRIDE = 33,
	TbUSER_STAT_FIELD_REMOVE_FLAG = 34,
	TbUSER_STAT_FIELD_LORD_SKILL = 35,
	TbUSER_STAT_FIELD_DRAGON_SKILL = 36,
	TbUSER_STAT_FIELD_DRAGON_MONSTER_SKILL = 37,
	TbUSER_STAT_FIELD_DRAGON_LEVEL_FINISH = 38,
	TbUSER_STAT_FIELD_REWARD_WINDOW_TIME = 39,
	TbUSER_STAT_FIELD_SEQ = 40,
};

enum ETbUser_statOpenType
{
	ETbUSERSTAT_OPEN_TYPE_PRIMARY = 0,
};

#define	TBUSER_STAT_MARK_MAX_NUM	5
typedef	BinBuf<SMarkItem, TBUSER_STAT_MARK_MAX_NUM>		TbUser_stat_Mark;

#define	TBUSER_STAT_STAR_MAIL_MAX_NUM	100
typedef	BinBuf<TINT64, TBUSER_STAT_STAR_MAIL_MAX_NUM>		TbUser_stat_Star_mail;

#define	TBUSER_STAT_MAIL_FLAG_MAX_NUM	1
typedef	BinBuf<SMailFlag, TBUSER_STAT_MAIL_FLAG_MAX_NUM>		TbUser_stat_Mail_flag;

#define	TBUSER_STAT_TOP_QUEST_MAX_NUM	1
typedef	BinBuf<STopQuest, TBUSER_STAT_TOP_QUEST_MAX_NUM>		TbUser_stat_Top_quest;

#define	TBUSER_STAT_TOP_QUEST_FINISH_MAX_NUM	1
typedef	BinBuf<STopQuest, TBUSER_STAT_TOP_QUEST_FINISH_MAX_NUM>		TbUser_stat_Top_quest_finish;

#define	TBUSER_STAT_LORD_LEVEL_FINISH_MAX_NUM	1
typedef	BinBuf<SLevelQuest, TBUSER_STAT_LORD_LEVEL_FINISH_MAX_NUM>		TbUser_stat_Lord_level_finish;

#define	TBUSER_STAT_LORD_SKILL_MAX_NUM	1
typedef	BinBuf<SSkill, TBUSER_STAT_LORD_SKILL_MAX_NUM>		TbUser_stat_Lord_skill;

#define	TBUSER_STAT_DRAGON_SKILL_MAX_NUM	1
typedef	BinBuf<SSkill, TBUSER_STAT_DRAGON_SKILL_MAX_NUM>		TbUser_stat_Dragon_skill;

#define	TBUSER_STAT_DRAGON_MONSTER_SKILL_MAX_NUM	1
typedef	BinBuf<SSkill, TBUSER_STAT_DRAGON_MONSTER_SKILL_MAX_NUM>		TbUser_stat_Dragon_monster_skill;

#define	TBUSER_STAT_DRAGON_LEVEL_FINISH_MAX_NUM	1
typedef	BinBuf<SLevelQuest, TBUSER_STAT_DRAGON_LEVEL_FINISH_MAX_NUM>		TbUser_stat_Dragon_level_finish;

class TbUser_stat : public AwsTable
{
public:
	TINT64 m_nUid;
	TINT64 m_nNewest_reportid;
	TINT64 m_nUnread_report;
	TINT64 m_nNewest_mailid;
	TINT64 m_nUnread_mail;
	TINT64 m_nAs_get_t;
	TINT64 m_nBirth_r;
	TINT64 m_nBirth_m;
	TINT64 m_nTips;
	TINT64 m_nWall_get_t;
	TbUser_stat_Mark m_bMark;
	TbUser_stat_Star_mail m_bStar_mail;
	TINT64 m_nReturn_mailid;
	TINT64 m_nReturn_reportid;
	TINT64 m_nReinforce_num;
	TINT64 m_nKill_troop_num;
	TINT64 m_nLast_event_win_id;
	TINT64 m_nAl_event_tips_time;
	TINT64 m_nPlayer_recommend_time;
	TINT64 m_nBroadcast_time;
	TbUser_stat_Mail_flag m_bMail_flag;
	TbUser_stat_Top_quest m_bTop_quest;
	TbUser_stat_Top_quest_finish m_bTop_quest_finish;
	TbUser_stat_Lord_level_finish m_bLord_level_finish;
	TINT64 m_nLast_help_bubble_set_time;
	TINT64 m_nLast_help_bubble_time_out;
	TINT64 m_nDaily_quest_finish_num;
	TINT64 m_nAl_quest_finish_num;
	TINT64 m_nVip_quest_finish_num;
	TINT64 m_nLast_might;
	Json::Value m_jLast_troop_fort;
	TINT64 m_nCon_login_days;
	TINT64 m_nMax_defeat_monster_lv;
	TINT64 m_nEquip_gride;
	TINT64 m_nRemove_flag;
	TbUser_stat_Lord_skill m_bLord_skill;
	TbUser_stat_Dragon_skill m_bDragon_skill;
	TbUser_stat_Dragon_monster_skill m_bDragon_monster_skill;
	TbUser_stat_Dragon_level_finish m_bDragon_level_finish;
	TINT64 m_nReward_window_time;
	TINT64 m_nSeq;

public:
	static TableDesc oTableDesc;
	Json::Reader oJsonReader;
	Json::FastWriter oJsonWriter;
	static int Init(const string& sConfFile, const string strProjectName);

public:
	TbUser_stat():AwsTable(&oTableDesc)
	{
		Reset();
	}
	~TbUser_stat()
	{}
	void Reset()
	{
		m_nUid = 0;
		m_nNewest_reportid = 0;
		m_nUnread_report = 0;
		m_nNewest_mailid = 0;
		m_nUnread_mail = 0;
		m_nAs_get_t = 0;
		m_nBirth_r = 0;
		m_nBirth_m = 0;
		m_nTips = 0;
		m_nWall_get_t = 0;
		m_bMark.Reset();
		m_bStar_mail.Reset();
		m_nReturn_mailid = 0;
		m_nReturn_reportid = 0;
		m_nReinforce_num = 0;
		m_nKill_troop_num = 0;
		m_nLast_event_win_id = 0;
		m_nAl_event_tips_time = 0;
		m_nPlayer_recommend_time = 0;
		m_nBroadcast_time = 0;
		m_bMail_flag.Reset();
		m_bTop_quest.Reset();
		m_bTop_quest_finish.Reset();
		m_bLord_level_finish.Reset();
		m_nLast_help_bubble_set_time = 0;
		m_nLast_help_bubble_time_out = 0;
		m_nDaily_quest_finish_num = 0;
		m_nAl_quest_finish_num = 0;
		m_nVip_quest_finish_num = 0;
		m_nLast_might = 0;
		m_jLast_troop_fort.clear();
		m_nCon_login_days = 0;
		m_nMax_defeat_monster_lv = 0;
		m_nEquip_gride = 0;
		m_nRemove_flag = 0;
		m_bLord_skill.Reset();
		m_bDragon_skill.Reset();
		m_bDragon_monster_skill.Reset();
		m_bDragon_level_finish.Reset();
		m_nReward_window_time = 0;
		m_nSeq = 0;
		ClearFlag();
	};
	const TINT64& Get_Uid()
	{
		return m_nUid;
	}
	void Set_Uid(const TINT64& nUid)
	{
		m_nUid = nUid;
	}
	const TINT64& Get_Newest_reportid()
	{
		return m_nNewest_reportid;
	}
	void Set_Newest_reportid(const TINT64& nNewest_reportid, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nNewest_reportid = nNewest_reportid;
		m_mFlag[TbUSER_STAT_FIELD_NEWEST_REPORTID] = dwActionType;
	}
	const TINT64& Get_Unread_report()
	{
		return m_nUnread_report;
	}
	void Set_Unread_report(const TINT64& nUnread_report, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nUnread_report = nUnread_report;
		m_mFlag[TbUSER_STAT_FIELD_UNREAD_REPORT] = dwActionType;
	}
	const TINT64& Get_Newest_mailid()
	{
		return m_nNewest_mailid;
	}
	void Set_Newest_mailid(const TINT64& nNewest_mailid, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nNewest_mailid = nNewest_mailid;
		m_mFlag[TbUSER_STAT_FIELD_NEWEST_MAILID] = dwActionType;
	}
	const TINT64& Get_Unread_mail()
	{
		return m_nUnread_mail;
	}
	void Set_Unread_mail(const TINT64& nUnread_mail, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nUnread_mail = nUnread_mail;
		m_mFlag[TbUSER_STAT_FIELD_UNREAD_MAIL] = dwActionType;
	}
	const TINT64& Get_As_get_t()
	{
		return m_nAs_get_t;
	}
	void Set_As_get_t(const TINT64& nAs_get_t, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nAs_get_t = nAs_get_t;
		m_mFlag[TbUSER_STAT_FIELD_AS_GET_T] = dwActionType;
	}
	const TINT64& Get_Birth_r()
	{
		return m_nBirth_r;
	}
	void Set_Birth_r(const TINT64& nBirth_r, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nBirth_r = nBirth_r;
		m_mFlag[TbUSER_STAT_FIELD_BIRTH_R] = dwActionType;
	}
	const TINT64& Get_Birth_m()
	{
		return m_nBirth_m;
	}
	void Set_Birth_m(const TINT64& nBirth_m, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nBirth_m = nBirth_m;
		m_mFlag[TbUSER_STAT_FIELD_BIRTH_M] = dwActionType;
	}
	const TINT64& Get_Tips()
	{
		return m_nTips;
	}
	void Set_Tips(const TINT64& nTips, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nTips = nTips;
		m_mFlag[TbUSER_STAT_FIELD_TIPS] = dwActionType;
	}
	const TINT64& Get_Wall_get_t()
	{
		return m_nWall_get_t;
	}
	void Set_Wall_get_t(const TINT64& nWall_get_t, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nWall_get_t = nWall_get_t;
		m_mFlag[TbUSER_STAT_FIELD_WALL_GET_T] = dwActionType;
	}
	const TINT64& Get_Return_mailid()
	{
		return m_nReturn_mailid;
	}
	void Set_Return_mailid(const TINT64& nReturn_mailid, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nReturn_mailid = nReturn_mailid;
		m_mFlag[TbUSER_STAT_FIELD_RETURN_MAILID] = dwActionType;
	}
	const TINT64& Get_Return_reportid()
	{
		return m_nReturn_reportid;
	}
	void Set_Return_reportid(const TINT64& nReturn_reportid, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nReturn_reportid = nReturn_reportid;
		m_mFlag[TbUSER_STAT_FIELD_RETURN_REPORTID] = dwActionType;
	}
	const TINT64& Get_Reinforce_num()
	{
		return m_nReinforce_num;
	}
	void Set_Reinforce_num(const TINT64& nReinforce_num, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nReinforce_num = nReinforce_num;
		m_mFlag[TbUSER_STAT_FIELD_REINFORCE_NUM] = dwActionType;
	}
	const TINT64& Get_Kill_troop_num()
	{
		return m_nKill_troop_num;
	}
	void Set_Kill_troop_num(const TINT64& nKill_troop_num, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nKill_troop_num = nKill_troop_num;
		m_mFlag[TbUSER_STAT_FIELD_KILL_TROOP_NUM] = dwActionType;
	}
	const TINT64& Get_Last_event_win_id()
	{
		return m_nLast_event_win_id;
	}
	void Set_Last_event_win_id(const TINT64& nLast_event_win_id, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nLast_event_win_id = nLast_event_win_id;
		m_mFlag[TbUSER_STAT_FIELD_LAST_EVENT_WIN_ID] = dwActionType;
	}
	const TINT64& Get_Al_event_tips_time()
	{
		return m_nAl_event_tips_time;
	}
	void Set_Al_event_tips_time(const TINT64& nAl_event_tips_time, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nAl_event_tips_time = nAl_event_tips_time;
		m_mFlag[TbUSER_STAT_FIELD_AL_EVENT_TIPS_TIME] = dwActionType;
	}
	const TINT64& Get_Player_recommend_time()
	{
		return m_nPlayer_recommend_time;
	}
	void Set_Player_recommend_time(const TINT64& nPlayer_recommend_time, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nPlayer_recommend_time = nPlayer_recommend_time;
		m_mFlag[TbUSER_STAT_FIELD_PLAYER_RECOMMEND_TIME] = dwActionType;
	}
	const TINT64& Get_Broadcast_time()
	{
		return m_nBroadcast_time;
	}
	void Set_Broadcast_time(const TINT64& nBroadcast_time, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nBroadcast_time = nBroadcast_time;
		m_mFlag[TbUSER_STAT_FIELD_BROADCAST_TIME] = dwActionType;
	}
	const TINT64& Get_Last_help_bubble_set_time()
	{
		return m_nLast_help_bubble_set_time;
	}
	void Set_Last_help_bubble_set_time(const TINT64& nLast_help_bubble_set_time, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nLast_help_bubble_set_time = nLast_help_bubble_set_time;
		m_mFlag[TbUSER_STAT_FIELD_LAST_HELP_BUBBLE_SET_TIME] = dwActionType;
	}
	const TINT64& Get_Last_help_bubble_time_out()
	{
		return m_nLast_help_bubble_time_out;
	}
	void Set_Last_help_bubble_time_out(const TINT64& nLast_help_bubble_time_out, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nLast_help_bubble_time_out = nLast_help_bubble_time_out;
		m_mFlag[TbUSER_STAT_FIELD_LAST_HELP_BUBBLE_TIME_OUT] = dwActionType;
	}
	const TINT64& Get_Daily_quest_finish_num()
	{
		return m_nDaily_quest_finish_num;
	}
	void Set_Daily_quest_finish_num(const TINT64& nDaily_quest_finish_num, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nDaily_quest_finish_num = nDaily_quest_finish_num;
		m_mFlag[TbUSER_STAT_FIELD_DAILY_QUEST_FINISH_NUM] = dwActionType;
	}
	const TINT64& Get_Al_quest_finish_num()
	{
		return m_nAl_quest_finish_num;
	}
	void Set_Al_quest_finish_num(const TINT64& nAl_quest_finish_num, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nAl_quest_finish_num = nAl_quest_finish_num;
		m_mFlag[TbUSER_STAT_FIELD_AL_QUEST_FINISH_NUM] = dwActionType;
	}
	const TINT64& Get_Vip_quest_finish_num()
	{
		return m_nVip_quest_finish_num;
	}
	void Set_Vip_quest_finish_num(const TINT64& nVip_quest_finish_num, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nVip_quest_finish_num = nVip_quest_finish_num;
		m_mFlag[TbUSER_STAT_FIELD_VIP_QUEST_FINISH_NUM] = dwActionType;
	}
	const TINT64& Get_Last_might()
	{
		return m_nLast_might;
	}
	void Set_Last_might(const TINT64& nLast_might, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nLast_might = nLast_might;
		m_mFlag[TbUSER_STAT_FIELD_LAST_MIGHT] = dwActionType;
	}
	const Json::Value& Get_Last_troop_fort()
	{
		return m_jLast_troop_fort;
	}
	void Set_Last_troop_fort(const Json::Value& jLast_troop_fort)
	{
		m_jLast_troop_fort = jLast_troop_fort;
		m_mFlag[TbUSER_STAT_FIELD_LAST_TROOP_FORT] = jLast_troop_fort.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Con_login_days()
	{
		return m_nCon_login_days;
	}
	void Set_Con_login_days(const TINT64& nCon_login_days, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nCon_login_days = nCon_login_days;
		m_mFlag[TbUSER_STAT_FIELD_CON_LOGIN_DAYS] = dwActionType;
	}
	const TINT64& Get_Max_defeat_monster_lv()
	{
		return m_nMax_defeat_monster_lv;
	}
	void Set_Max_defeat_monster_lv(const TINT64& nMax_defeat_monster_lv, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nMax_defeat_monster_lv = nMax_defeat_monster_lv;
		m_mFlag[TbUSER_STAT_FIELD_MAX_DEFEAT_MONSTER_LV] = dwActionType;
	}
	const TINT64& Get_Equip_gride()
	{
		return m_nEquip_gride;
	}
	void Set_Equip_gride(const TINT64& nEquip_gride, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nEquip_gride = nEquip_gride;
		m_mFlag[TbUSER_STAT_FIELD_EQUIP_GRIDE] = dwActionType;
	}
	const TINT64& Get_Remove_flag()
	{
		return m_nRemove_flag;
	}
	void Set_Remove_flag(const TINT64& nRemove_flag, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nRemove_flag = nRemove_flag;
		m_mFlag[TbUSER_STAT_FIELD_REMOVE_FLAG] = dwActionType;
	}
	const TINT64& Get_Reward_window_time()
	{
		return m_nReward_window_time;
	}
	void Set_Reward_window_time(const TINT64& nReward_window_time, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nReward_window_time = nReward_window_time;
		m_mFlag[TbUSER_STAT_FIELD_REWARD_WINDOW_TIME] = dwActionType;
	}
	const TINT64& Get_Seq()
	{
		return m_nSeq;
	}
	void Set_Seq(const TINT64& nSeq, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSeq = nSeq;
		m_mFlag[TbUSER_STAT_FIELD_SEQ] = dwActionType;
	}
	//Scan命令使用,返回完整的请求包
	AwsMap* OnScanReq(unsigned int udwIdxNo, bool bHasStartKey = false, bool bReturnConsumedCapacity = true,
		unsigned int dwLimit = 0, unsigned int dwSegment = 0, unsigned int dwTotalSegments = 0);
	int OnScanReq(string& sPostData, unsigned int udwIdxNo, bool bHasStartKey = false, bool bReturnConsumedCapacity = true,
		unsigned int dwLimit = 0, unsigned int dwSegment = 0, unsigned int dwTotalSegments = 0);
	int OnScanRsp(const Json::Value& item);
	//Query命令使用,返回完整的请求包
	AwsMap* OnQueryReq(unsigned int udwIdxNo, const CompareDesc& comp_desc = CompareDesc(), 
		bool bConsistentRead = true, bool bReturnConsumedCapacity = true, bool bScanIndexForward = true, unsigned int dwLimit = 0, bool bCount = false);
	int OnQueryReq(string& sPostData, unsigned int udwIdxNo, const CompareDesc& comp_desc = CompareDesc(), 
		bool bConsistentRead = true, bool bReturnConsumedCapacity = true, bool bScanIndexForward = true, unsigned int dwLimit = 0);
	int OnQueryRsp(const Json::Value& item);
	//查找记录数量
	int OnCountReq(string& sPostData, unsigned int udwIdxNo, const CompareDesc& comp_desc = CompareDesc(), 
		bool bConsistentRead = true, bool bReturnConsumedCapacity = true, bool bScanIndexForward = true, unsigned int dwLimit = 0);	//UpdateItem命令使用,返回完整的请求包
	AwsMap* OnUpdateItemReq(
		const ExpectedDesc& expected_desc = ExpectedDesc(), int dwReturnValuesType = RETURN_VALUES_NONE, bool bReturnConsumedCapacity = true);
	int OnUpdateItemReq(string& sPostData,
		const ExpectedDesc& expected_desc = ExpectedDesc(), int dwReturnValuesType = RETURN_VALUES_NONE, bool bReturnConsumedCapacity = true);
	int OnUpdateItemRsp(const Json::Value& item);
	//BatchWriteItem命令使用
	AwsMap* OnWriteItemReq(int dwActionType = WRITE_ACTION_TYPE_PUT);
	void OnWriteItemReq(AwsMap* pWriteItem, int dwActionType = WRITE_ACTION_TYPE_PUT);
	//BatchGetItem命令使用
	AwsMap* OnReadItemReq(unsigned int udwIdxNo);
	void OnReadItemReq(AwsMap*pReadItem);
	int OnReadItemRsp(const Json::Value& item);
	//DeleteItem命令使用,返回完整的请求包
	AwsMap* OnDeleteItemReq(
		const ExpectedDesc& expected_desc = ExpectedDesc(), int dwReturnValuesType = RETURN_VALUES_NONE, bool bReturnConsumedCapacity = true);
	int OnDeleteItemReq(string& sPostData,
		const ExpectedDesc& expected_desc = ExpectedDesc(), int dwReturnValuesType = RETURN_VALUES_NONE, bool bReturnConsumedCapacity = true);
	int OnDeleteItemRsp(const Json::Value& item);
	//GetItem命令使用,返回完整的请求包
	AwsMap* OnGetItemReq(unsigned int udwIdxNo,
		bool bConsistentRead = true, bool bReturnConsumedCapacity = true);
	int OnGetItemReq(string& sPostData, unsigned int udwIdxNo,
		bool bConsistentRead = true, bool bReturnConsumedCapacity = true);
	int OnGetItemRsp(const Json::Value& item);
	//PutItem命令使用,返回完整的请求包
	AwsMap* OnPutItemReq(
		const ExpectedDesc& expected_desc = ExpectedDesc(), int dwReturnValuesType = RETURN_VALUES_NONE, bool bReturnConsumedCapacity = true);
	int OnPutItemReq(string& sPostData,
		const ExpectedDesc& expected_desc = ExpectedDesc(), int dwReturnValuesType = RETURN_VALUES_NONE, bool bReturnConsumedCapacity = true);
	int OnPutItemRsp(const Json::Value& item);
	int OnResponse(const Json::Value& item);
public: 
	static AwsTable* NewObject();
	string GetTableName();
	TINT32 GetTableIdx();
public: 
	TINT64 GetSeq();
};

#endif 

