#ifndef _AWS_TABLE_MAP_H_
#define _AWS_TABLE_MAP_H_

#include "aws_table_common.h"
#include "field_property.h"

#define EN_AWS_TABLE_MAP "map"

enum ETbMapField
{
	TbMAP_FIELD_SID = 0,
	TbMAP_FIELD_ID = 1,
	TbMAP_FIELD_UTIME = 2,
	TbMAP_FIELD_TYPE = 3,
	TbMAP_FIELD_RTYPE = 4,
	TbMAP_FIELD_LEVEL = 5,
	TbMAP_FIELD_MIGHT = 6,
	TbMAP_FIELD_BID = 7,
	TbMAP_FIELD_UID = 8,
	TbMAP_FIELD_NAME = 9,
	TbMAP_FIELD_STATUS = 10,
	TbMAP_FIELD_NPC = 11,
	TbMAP_FIELD_CITY_INFO = 12,
	TbMAP_FIELD_EM_LV = 13,
	TbMAP_FIELD_CID = 14,
	TbMAP_FIELD_UNAME = 15,
	TbMAP_FIELD_ULEVEL = 16,
	TbMAP_FIELD_ALID = 17,
	TbMAP_FIELD_ALNAME = 18,
	TbMAP_FIELD_AL_NICK = 19,
	TbMAP_FIELD_AL_FLAG = 20,
	TbMAP_FIELD_VIP_LEVEL = 21,
	TbMAP_FIELD_VIP_ETIME = 22,
	TbMAP_FIELD_CENTER_POS = 23,
	TbMAP_FIELD_NAME_UPDATE_TIME = 24,
	TbMAP_FIELD_AL_POS = 25,
	TbMAP_FIELD_AVATAR = 26,
	TbMAP_FIELD_AGE = 27,
	TbMAP_FIELD_PRISON_FLAG = 28,
	TbMAP_FIELD_TIME_END = 29,
	TbMAP_FIELD_PIC_INDEX = 30,
	TbMAP_FIELD_RALLY_TROOP_LIMIT = 31,
	TbMAP_FIELD_SMOKE_END_TIME = 32,
	TbMAP_FIELD_BURN_END_TIME = 33,
	TbMAP_FIELD_MARCH_STATUS_TIME = 34,
	TbMAP_FIELD_MOVE_CITY = 35,
	TbMAP_FIELD_RESOURCE = 36,
	TbMAP_FIELD_TROOP = 37,
	TbMAP_FIELD_RES_RATE = 38,
	TbMAP_FIELD_RES_TIME = 39,
	TbMAP_FIELD_REWARD_LEFT = 40,
	TbMAP_FIELD_SHOWTIME = 41,
	TbMAP_FIELD_WILD_GEN_TIME = 42,
	TbMAP_FIELD_BOSS_LIFE = 43,
	TbMAP_FIELD_ATTACK_INFO = 44,
	TbMAP_FIELD_TAX_RATE_ID = 45,
	TbMAP_FIELD_FORCE_KILL = 46,
	TbMAP_FIELD_AL_ATTACK_INFO = 47,
	TbMAP_FIELD_LEADER_MONSTER_FLAG = 48,
	TbMAP_FIELD_OCCUPY_NUM = 49,
	TbMAP_FIELD_OCCUPY_CLEAN_FLAG = 50,
	TbMAP_FIELD_SEQ = 51,
	TbMAP_FIELD_EXPIRE_TIME = 52,
};

enum ETbMapOpenType
{
	ETbMAP_OPEN_TYPE_PRIMARY = 0,
	ETbMAP_OPEN_TYPE_GLB_UID = 1,
	ETbMAP_OPEN_TYPE_GLB_BID = 2,
	ETbMAP_OPEN_TYPE_GLB_ID = 3,
};

#define	TBMAP_RESOURCE_MAX_NUM	1
typedef	BinBuf<SCommonResource, TBMAP_RESOURCE_MAX_NUM>		TbMap_Resource;

#define	TBMAP_TROOP_MAX_NUM	1
typedef	BinBuf<SCommonTroop, TBMAP_TROOP_MAX_NUM>		TbMap_Troop;

#define	TBMAP_ATTACK_INFO_MAX_NUM	100
typedef	BinBuf<SAttackTimesInfo, TBMAP_ATTACK_INFO_MAX_NUM>		TbMap_Attack_info;

#define	TBMAP_AL_ATTACK_INFO_MAX_NUM	100
typedef	BinBuf<SAttackTimesInfo, TBMAP_AL_ATTACK_INFO_MAX_NUM>		TbMap_Al_attack_info;

class TbMap : public AwsTable
{
public:
	TINT64 m_nSid;
	TINT64 m_nId;
	TINT64 m_nUtime;
	TINT64 m_nType;
	TINT64 m_nRtype;
	TINT64 m_nLevel;
	TINT64 m_nMight;
	TINT64 m_nBid;
	TINT64 m_nUid;
	string m_sName;
	TINT64 m_nStatus;
	TINT64 m_nNpc;
	Json::Value m_jCity_info;
	TINT64 m_nEm_lv;
	TINT64 m_nCid;
	string m_sUname;
	TINT64 m_nUlevel;
	TINT64 m_nAlid;
	string m_sAlname;
	string m_sAl_nick;
	TINT64 m_nAl_flag;
	TINT64 m_nVip_level;
	TINT64 m_nVip_etime;
	TINT64 m_nCenter_pos;
	TINT64 m_nName_update_time;
	TINT64 m_nAl_pos;
	TINT64 m_nAvatar;
	TINT64 m_nAge;
	TINT64 m_nPrison_flag;
	TINT64 m_nTime_end;
	TINT64 m_nPic_index;
	TINT64 m_nRally_troop_limit;
	TINT64 m_nSmoke_end_time;
	TINT64 m_nBurn_end_time;
	TINT64 m_nMarch_status_time;
	TINT64 m_nMove_city;
	TbMap_Resource m_bResource;
	TbMap_Troop m_bTroop;
	TINT64 m_nRes_rate;
	TINT64 m_nRes_time;
	TINT64 m_nReward_left;
	TINT64 m_nShowtime;
	TINT64 m_nWild_gen_time;
	TINT64 m_nBoss_life;
	TbMap_Attack_info m_bAttack_info;
	TINT64 m_nTax_rate_id;
	TINT64 m_nForce_kill;
	TbMap_Al_attack_info m_bAl_attack_info;
	TINT64 m_nLeader_monster_flag;
	TINT64 m_nOccupy_num;
	TINT64 m_nOccupy_clean_flag;
	TINT64 m_nSeq;
	TINT64 m_nExpire_time;

public:
	static TableDesc oTableDesc;
	Json::Reader oJsonReader;
	Json::FastWriter oJsonWriter;
	static int Init(const string& sConfFile, const string strProjectName);

public:
	TbMap():AwsTable(&oTableDesc)
	{
		Reset();
	}
	~TbMap()
	{}
	void Reset()
	{
		m_nSid = 0;
		m_nId = 0;
		m_nUtime = 0;
		m_nType = 0;
		m_nRtype = 0;
		m_nLevel = 0;
		m_nMight = 0;
		m_nBid = 0;
		m_nUid = 0;
		m_sName = "";
		m_nStatus = 0;
		m_nNpc = 0;
		m_jCity_info.clear();
		m_nEm_lv = 0;
		m_nCid = 0;
		m_sUname = "";
		m_nUlevel = 0;
		m_nAlid = 0;
		m_sAlname = "";
		m_sAl_nick = "";
		m_nAl_flag = 0;
		m_nVip_level = 0;
		m_nVip_etime = 0;
		m_nCenter_pos = 0;
		m_nName_update_time = 0;
		m_nAl_pos = 0;
		m_nAvatar = 0;
		m_nAge = 0;
		m_nPrison_flag = 0;
		m_nTime_end = 0;
		m_nPic_index = 0;
		m_nRally_troop_limit = 0;
		m_nSmoke_end_time = 0;
		m_nBurn_end_time = 0;
		m_nMarch_status_time = 0;
		m_nMove_city = 0;
		m_bResource.Reset();
		m_bTroop.Reset();
		m_nRes_rate = 0;
		m_nRes_time = 0;
		m_nReward_left = 0;
		m_nShowtime = 0;
		m_nWild_gen_time = 0;
		m_nBoss_life = 0;
		m_bAttack_info.Reset();
		m_nTax_rate_id = 0;
		m_nForce_kill = 0;
		m_bAl_attack_info.Reset();
		m_nLeader_monster_flag = 0;
		m_nOccupy_num = 0;
		m_nOccupy_clean_flag = 0;
		m_nSeq = 0;
		m_nExpire_time = 0;
		ClearFlag();
	};
	const TINT64& Get_Sid()
	{
		return m_nSid;
	}
	void Set_Sid(const TINT64& nSid, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSid = nSid;
		m_mFlag[TbMAP_FIELD_SID] = dwActionType;
	}
	const TINT64& Get_Id()
	{
		return m_nId;
	}
	void Set_Id(const TINT64& nId)
	{
		m_nId = nId;
	}
	const TINT64& Get_Utime()
	{
		return m_nUtime;
	}
	void Set_Utime(const TINT64& nUtime, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nUtime = nUtime;
		m_mFlag[TbMAP_FIELD_UTIME] = dwActionType;
	}
	const TINT64& Get_Type()
	{
		return m_nType;
	}
	void Set_Type(const TINT64& nType, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nType = nType;
		m_mFlag[TbMAP_FIELD_TYPE] = dwActionType;
	}
	const TINT64& Get_Rtype()
	{
		return m_nRtype;
	}
	void Set_Rtype(const TINT64& nRtype, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nRtype = nRtype;
		m_mFlag[TbMAP_FIELD_RTYPE] = dwActionType;
	}
	const TINT64& Get_Level()
	{
		return m_nLevel;
	}
	void Set_Level(const TINT64& nLevel, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nLevel = nLevel;
		m_mFlag[TbMAP_FIELD_LEVEL] = dwActionType;
	}
	const TINT64& Get_Might()
	{
		return m_nMight;
	}
	void Set_Might(const TINT64& nMight, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nMight = nMight;
		m_mFlag[TbMAP_FIELD_MIGHT] = dwActionType;
	}
	const TINT64& Get_Bid()
	{
		return m_nBid;
	}
	void Set_Bid(const TINT64& nBid, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nBid = nBid;
		m_mFlag[TbMAP_FIELD_BID] = dwActionType;
	}
	const TINT64& Get_Uid()
	{
		return m_nUid;
	}
	void Set_Uid(const TINT64& nUid, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nUid = nUid;
		m_mFlag[TbMAP_FIELD_UID] = dwActionType;
	}
	const string& Get_Name()
	{
		return m_sName;
	}
	void Set_Name(const string& sName)
	{
		m_sName = sName;
		m_mFlag[TbMAP_FIELD_NAME] = sName.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Status()
	{
		return m_nStatus;
	}
	void Set_Status(const TINT64& nStatus, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nStatus = nStatus;
		m_mFlag[TbMAP_FIELD_STATUS] = dwActionType;
	}
	const TINT64& Get_Npc()
	{
		return m_nNpc;
	}
	void Set_Npc(const TINT64& nNpc, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nNpc = nNpc;
		m_mFlag[TbMAP_FIELD_NPC] = dwActionType;
	}
	const Json::Value& Get_City_info()
	{
		return m_jCity_info;
	}
	void Set_City_info(const Json::Value& jCity_info)
	{
		m_jCity_info = jCity_info;
		m_mFlag[TbMAP_FIELD_CITY_INFO] = jCity_info.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Em_lv()
	{
		return m_nEm_lv;
	}
	void Set_Em_lv(const TINT64& nEm_lv, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nEm_lv = nEm_lv;
		m_mFlag[TbMAP_FIELD_EM_LV] = dwActionType;
	}
	const TINT64& Get_Cid()
	{
		return m_nCid;
	}
	void Set_Cid(const TINT64& nCid, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nCid = nCid;
		m_mFlag[TbMAP_FIELD_CID] = dwActionType;
	}
	const string& Get_Uname()
	{
		return m_sUname;
	}
	void Set_Uname(const string& sUname)
	{
		m_sUname = sUname;
		m_mFlag[TbMAP_FIELD_UNAME] = sUname.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Ulevel()
	{
		return m_nUlevel;
	}
	void Set_Ulevel(const TINT64& nUlevel, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nUlevel = nUlevel;
		m_mFlag[TbMAP_FIELD_ULEVEL] = dwActionType;
	}
	const TINT64& Get_Alid()
	{
		return m_nAlid;
	}
	void Set_Alid(const TINT64& nAlid, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nAlid = nAlid;
		m_mFlag[TbMAP_FIELD_ALID] = dwActionType;
	}
	const string& Get_Alname()
	{
		return m_sAlname;
	}
	void Set_Alname(const string& sAlname)
	{
		m_sAlname = sAlname;
		m_mFlag[TbMAP_FIELD_ALNAME] = sAlname.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const string& Get_Al_nick()
	{
		return m_sAl_nick;
	}
	void Set_Al_nick(const string& sAl_nick)
	{
		m_sAl_nick = sAl_nick;
		m_mFlag[TbMAP_FIELD_AL_NICK] = sAl_nick.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Al_flag()
	{
		return m_nAl_flag;
	}
	void Set_Al_flag(const TINT64& nAl_flag, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nAl_flag = nAl_flag;
		m_mFlag[TbMAP_FIELD_AL_FLAG] = dwActionType;
	}
	const TINT64& Get_Vip_level()
	{
		return m_nVip_level;
	}
	void Set_Vip_level(const TINT64& nVip_level, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nVip_level = nVip_level;
		m_mFlag[TbMAP_FIELD_VIP_LEVEL] = dwActionType;
	}
	const TINT64& Get_Vip_etime()
	{
		return m_nVip_etime;
	}
	void Set_Vip_etime(const TINT64& nVip_etime, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nVip_etime = nVip_etime;
		m_mFlag[TbMAP_FIELD_VIP_ETIME] = dwActionType;
	}
	const TINT64& Get_Center_pos()
	{
		return m_nCenter_pos;
	}
	void Set_Center_pos(const TINT64& nCenter_pos, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nCenter_pos = nCenter_pos;
		m_mFlag[TbMAP_FIELD_CENTER_POS] = dwActionType;
	}
	const TINT64& Get_Name_update_time()
	{
		return m_nName_update_time;
	}
	void Set_Name_update_time(const TINT64& nName_update_time, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nName_update_time = nName_update_time;
		m_mFlag[TbMAP_FIELD_NAME_UPDATE_TIME] = dwActionType;
	}
	const TINT64& Get_Al_pos()
	{
		return m_nAl_pos;
	}
	void Set_Al_pos(const TINT64& nAl_pos, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nAl_pos = nAl_pos;
		m_mFlag[TbMAP_FIELD_AL_POS] = dwActionType;
	}
	const TINT64& Get_Avatar()
	{
		return m_nAvatar;
	}
	void Set_Avatar(const TINT64& nAvatar, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nAvatar = nAvatar;
		m_mFlag[TbMAP_FIELD_AVATAR] = dwActionType;
	}
	const TINT64& Get_Age()
	{
		return m_nAge;
	}
	void Set_Age(const TINT64& nAge, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nAge = nAge;
		m_mFlag[TbMAP_FIELD_AGE] = dwActionType;
	}
	const TINT64& Get_Prison_flag()
	{
		return m_nPrison_flag;
	}
	void Set_Prison_flag(const TINT64& nPrison_flag, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nPrison_flag = nPrison_flag;
		m_mFlag[TbMAP_FIELD_PRISON_FLAG] = dwActionType;
	}
	const TINT64& Get_Time_end()
	{
		return m_nTime_end;
	}
	void Set_Time_end(const TINT64& nTime_end, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nTime_end = nTime_end;
		m_mFlag[TbMAP_FIELD_TIME_END] = dwActionType;
	}
	const TINT64& Get_Pic_index()
	{
		return m_nPic_index;
	}
	void Set_Pic_index(const TINT64& nPic_index, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nPic_index = nPic_index;
		m_mFlag[TbMAP_FIELD_PIC_INDEX] = dwActionType;
	}
	const TINT64& Get_Rally_troop_limit()
	{
		return m_nRally_troop_limit;
	}
	void Set_Rally_troop_limit(const TINT64& nRally_troop_limit, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nRally_troop_limit = nRally_troop_limit;
		m_mFlag[TbMAP_FIELD_RALLY_TROOP_LIMIT] = dwActionType;
	}
	const TINT64& Get_Smoke_end_time()
	{
		return m_nSmoke_end_time;
	}
	void Set_Smoke_end_time(const TINT64& nSmoke_end_time, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSmoke_end_time = nSmoke_end_time;
		m_mFlag[TbMAP_FIELD_SMOKE_END_TIME] = dwActionType;
	}
	const TINT64& Get_Burn_end_time()
	{
		return m_nBurn_end_time;
	}
	void Set_Burn_end_time(const TINT64& nBurn_end_time, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nBurn_end_time = nBurn_end_time;
		m_mFlag[TbMAP_FIELD_BURN_END_TIME] = dwActionType;
	}
	const TINT64& Get_March_status_time()
	{
		return m_nMarch_status_time;
	}
	void Set_March_status_time(const TINT64& nMarch_status_time, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nMarch_status_time = nMarch_status_time;
		m_mFlag[TbMAP_FIELD_MARCH_STATUS_TIME] = dwActionType;
	}
	const TINT64& Get_Move_city()
	{
		return m_nMove_city;
	}
	void Set_Move_city(const TINT64& nMove_city, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nMove_city = nMove_city;
		m_mFlag[TbMAP_FIELD_MOVE_CITY] = dwActionType;
	}
	const TINT64& Get_Res_rate()
	{
		return m_nRes_rate;
	}
	void Set_Res_rate(const TINT64& nRes_rate, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nRes_rate = nRes_rate;
		m_mFlag[TbMAP_FIELD_RES_RATE] = dwActionType;
	}
	const TINT64& Get_Res_time()
	{
		return m_nRes_time;
	}
	void Set_Res_time(const TINT64& nRes_time, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nRes_time = nRes_time;
		m_mFlag[TbMAP_FIELD_RES_TIME] = dwActionType;
	}
	const TINT64& Get_Reward_left()
	{
		return m_nReward_left;
	}
	void Set_Reward_left(const TINT64& nReward_left, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nReward_left = nReward_left;
		m_mFlag[TbMAP_FIELD_REWARD_LEFT] = dwActionType;
	}
	const TINT64& Get_Showtime()
	{
		return m_nShowtime;
	}
	void Set_Showtime(const TINT64& nShowtime, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nShowtime = nShowtime;
		m_mFlag[TbMAP_FIELD_SHOWTIME] = dwActionType;
	}
	const TINT64& Get_Wild_gen_time()
	{
		return m_nWild_gen_time;
	}
	void Set_Wild_gen_time(const TINT64& nWild_gen_time, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nWild_gen_time = nWild_gen_time;
		m_mFlag[TbMAP_FIELD_WILD_GEN_TIME] = dwActionType;
	}
	const TINT64& Get_Boss_life()
	{
		return m_nBoss_life;
	}
	void Set_Boss_life(const TINT64& nBoss_life, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nBoss_life = nBoss_life;
		m_mFlag[TbMAP_FIELD_BOSS_LIFE] = dwActionType;
	}
	const TINT64& Get_Tax_rate_id()
	{
		return m_nTax_rate_id;
	}
	void Set_Tax_rate_id(const TINT64& nTax_rate_id, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nTax_rate_id = nTax_rate_id;
		m_mFlag[TbMAP_FIELD_TAX_RATE_ID] = dwActionType;
	}
	const TINT64& Get_Force_kill()
	{
		return m_nForce_kill;
	}
	void Set_Force_kill(const TINT64& nForce_kill, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nForce_kill = nForce_kill;
		m_mFlag[TbMAP_FIELD_FORCE_KILL] = dwActionType;
	}
	const TINT64& Get_Leader_monster_flag()
	{
		return m_nLeader_monster_flag;
	}
	void Set_Leader_monster_flag(const TINT64& nLeader_monster_flag, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nLeader_monster_flag = nLeader_monster_flag;
		m_mFlag[TbMAP_FIELD_LEADER_MONSTER_FLAG] = dwActionType;
	}
	const TINT64& Get_Occupy_num()
	{
		return m_nOccupy_num;
	}
	void Set_Occupy_num(const TINT64& nOccupy_num, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nOccupy_num = nOccupy_num;
		m_mFlag[TbMAP_FIELD_OCCUPY_NUM] = dwActionType;
	}
	const TINT64& Get_Occupy_clean_flag()
	{
		return m_nOccupy_clean_flag;
	}
	void Set_Occupy_clean_flag(const TINT64& nOccupy_clean_flag, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nOccupy_clean_flag = nOccupy_clean_flag;
		m_mFlag[TbMAP_FIELD_OCCUPY_CLEAN_FLAG] = dwActionType;
	}
	const TINT64& Get_Seq()
	{
		return m_nSeq;
	}
	void Set_Seq(const TINT64& nSeq, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSeq = nSeq;
		m_mFlag[TbMAP_FIELD_SEQ] = dwActionType;
	}
	const TINT64& Get_Expire_time()
	{
		return m_nExpire_time;
	}
	void Set_Expire_time(const TINT64& nExpire_time, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nExpire_time = nExpire_time;
		m_mFlag[TbMAP_FIELD_EXPIRE_TIME] = dwActionType;
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

