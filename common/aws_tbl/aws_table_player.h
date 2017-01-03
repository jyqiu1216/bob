#ifndef _AWS_TABLE_PLAYER_H_
#define _AWS_TABLE_PLAYER_H_

#include "aws_table_common.h"
#include "field_property.h"

#define EN_AWS_TABLE_PLAYER "player"

enum ETbPlayerField
{
	TbPLAYER_FIELD_SID = 0,
	TbPLAYER_FIELD_UID = 1,
	TbPLAYER_FIELD_UIN = 2,
	TbPLAYER_FIELD_AVATAR = 3,
	TbPLAYER_FIELD_AGE = 4,
	TbPLAYER_FIELD_CID = 5,
	TbPLAYER_FIELD_CTIME = 6,
	TbPLAYER_FIELD_UTIME = 7,
	TbPLAYER_FIELD_NPC = 8,
	TbPLAYER_FIELD_STATUS = 9,
	TbPLAYER_FIELD_DEAD_FLAG = 10,
	TbPLAYER_FIELD_ALID = 11,
	TbPLAYER_FIELD_ALNAME = 12,
	TbPLAYER_FIELD_ALPOS = 13,
	TbPLAYER_FIELD_AL_TIME = 14,
	TbPLAYER_FIELD_AL_NICK_NAME = 15,
	TbPLAYER_FIELD_REQ_AL = 16,
	TbPLAYER_FIELD_JOIN_ALLIANCE = 17,
	TbPLAYER_FIELD_LOY_TIME = 18,
	TbPLAYER_FIELD_LOY_ITV = 19,
	TbPLAYER_FIELD_LOY_CUR = 20,
	TbPLAYER_FIELD_LOY_ALL = 21,
	TbPLAYER_FIELD_INVITE_MAIL_TIME = 22,
	TbPLAYER_FIELD_INVITED_NUM = 23,
	TbPLAYER_FIELD_INVITED_TIME = 24,
	TbPLAYER_FIELD_DOUBLOON = 25,
	TbPLAYER_FIELD_VIP_POINT = 26,
	TbPLAYER_FIELD_VIP_ETIME = 27,
	TbPLAYER_FIELD_DRAGON_STATISTICS = 28,
	TbPLAYER_FIELD_WAR_STATISTICS = 29,
	TbPLAYER_FIELD_TRANSPORT_RESOURCE = 30,
	TbPLAYER_FIELD_GAIN_RESOURCE = 31,
	TbPLAYER_FIELD_CUR_TROOP_MIGHT = 32,
	TbPLAYER_FIELD_CUR_FORT_MIGHT = 33,
	TbPLAYER_FIELD_SEND_AL_HELP_NUM = 34,
	TbPLAYER_FIELD_BUILDING_FORCE = 35,
	TbPLAYER_FIELD_RESEARCH_FORCE = 36,
	TbPLAYER_FIELD_DRAGON_FORCE = 37,
	TbPLAYER_FIELD_ALNAME_UPDATE_TIME = 38,
	TbPLAYER_FIELD_HAS_CHANGE_SVR = 39,
	TbPLAYER_FIELD_SVR_CHANGE_TIME = 40,
	TbPLAYER_FIELD_CHEST_LOTTERY = 41,
	TbPLAYER_FIELD_DRAGON = 42,
	TbPLAYER_FIELD_MIGHT = 43,
	TbPLAYER_FIELD_DTROOP = 44,
	TbPLAYER_FIELD_KTROOP = 45,
	TbPLAYER_FIELD_DFORT = 46,
	TbPLAYER_FIELD_KFORT = 47,
	TbPLAYER_FIELD_BAT = 48,
	TbPLAYER_FIELD_BAT_SUC = 49,
	TbPLAYER_FIELD_DCITY = 50,
	TbPLAYER_FIELD_MKILL = 51,
	TbPLAYER_FIELD_MGAIN = 52,
	TbPLAYER_FIELD_EXP = 53,
	TbPLAYER_FIELD_LEVEL = 54,
	TbPLAYER_FIELD_LAST_MIGHT = 55,
	TbPLAYER_FIELD_LAST_TROOP_FORT = 56,
	TbPLAYER_FIELD_SH_RES = 57,
	TbPLAYER_FIELD_SH_MIGHT = 58,
	TbPLAYER_FIELD_RES_COL = 59,
	TbPLAYER_FIELD_DRAGON_NAME = 60,
	TbPLAYER_FIELD_DRAGON_LEVEL = 61,
	TbPLAYER_FIELD_DRAGON_EXP = 62,
	TbPLAYER_FIELD_DRAGON_AVATAR = 63,
	TbPLAYER_FIELD_DRAGON_STATUS = 64,
	TbPLAYER_FIELD_DRAGON_TID = 65,
	TbPLAYER_FIELD_DRAGON_CUR_ENERGY = 66,
	TbPLAYER_FIELD_DRAGON_RECOVERY_TIME = 67,
	TbPLAYER_FIELD_DRAGON_RECOVERY_COUNT = 68,
	TbPLAYER_FIELD_HAS_DRAGON = 69,
	TbPLAYER_FIELD_DRAGON_BEGIN_RECOVERY_TIME = 70,
	TbPLAYER_FIELD_DRAGON_MAX_ENERGY = 71,
	TbPLAYER_FIELD_PERSON_GUIDE = 72,
	TbPLAYER_FIELD_DRAGON_MAX_LV = 73,
	TbPLAYER_FIELD_MONSTER_HIT = 74,
	TbPLAYER_FIELD_LEADER_MONSTER_KILL = 75,
	TbPLAYER_FIELD_LEADER_MONSTER_GEN = 76,
	TbPLAYER_FIELD_DRAGON_EQUIP = 77,
	TbPLAYER_FIELD_SEQ = 78,
	TbPLAYER_FIELD_LORD_FORCE = 79,
	TbPLAYER_FIELD_TRIAL_INIT = 80,
	TbPLAYER_FIELD_DRAGON_SHARD = 81,
	TbPLAYER_FIELD_TRIAL_RAGE_OPEN = 82,
	TbPLAYER_FIELD_TRIAL_RAGE_MODE = 83,
	TbPLAYER_FIELD_TRIAL_MONSTER_NORMAL = 84,
	TbPLAYER_FIELD_TRIAL_MONSTER_RAGE = 85,
	TbPLAYER_FIELD_TRIAL_LUCKY_BAG_NORMAL = 86,
	TbPLAYER_FIELD_TRIAL_LUCKY_BAG_RAGE = 87,
	TbPLAYER_FIELD_TRIAL_GIFT_LAST_ETIME = 88,
	TbPLAYER_FIELD_FINISH_GUIDE_LIST = 89,
	TbPLAYER_FIELD_REINFORCE_LIMIT = 90,
	TbPLAYER_FIELD_EVIL_FORCE_KILL = 91,
	TbPLAYER_FIELD_EVIL_TROOP_KILL = 92,
	TbPLAYER_FIELD_VIP_STAGE = 93,
};

enum ETbPlayerOpenType
{
	ETbPLAYER_OPEN_TYPE_PRIMARY = 0,
};

#define	TBPLAYER_DRAGON_STATISTICS_MAX_NUM	1
typedef	BinBuf<SDragonStatistics, TBPLAYER_DRAGON_STATISTICS_MAX_NUM>		TbPlayer_Dragon_statistics;

#define	TBPLAYER_WAR_STATISTICS_MAX_NUM	1
typedef	BinBuf<SWarStatistics, TBPLAYER_WAR_STATISTICS_MAX_NUM>		TbPlayer_War_statistics;

#define	TBPLAYER_TRANSPORT_RESOURCE_MAX_NUM	1
typedef	BinBuf<SCommonResource, TBPLAYER_TRANSPORT_RESOURCE_MAX_NUM>		TbPlayer_Transport_resource;

#define	TBPLAYER_GAIN_RESOURCE_MAX_NUM	1
typedef	BinBuf<SCommonResource, TBPLAYER_GAIN_RESOURCE_MAX_NUM>		TbPlayer_Gain_resource;

#define	TBPLAYER_CHEST_LOTTERY_MAX_NUM	20
typedef	BinBuf<SChestLottery, TBPLAYER_CHEST_LOTTERY_MAX_NUM>		TbPlayer_Chest_lottery;

#define	TBPLAYER_DRAGON_MAX_NUM	1
typedef	BinBuf<SDragonInfo, TBPLAYER_DRAGON_MAX_NUM>		TbPlayer_Dragon;

#define	TBPLAYER_PERSON_GUIDE_MAX_NUM	1
typedef	BinBuf<SPersonGuideFinish, TBPLAYER_PERSON_GUIDE_MAX_NUM>		TbPlayer_Person_guide;

#define	TBPLAYER_MONSTER_HIT_MAX_NUM	1
typedef	BinBuf<SMonsterHit, TBPLAYER_MONSTER_HIT_MAX_NUM>		TbPlayer_Monster_hit;

#define	TBPLAYER_LEADER_MONSTER_KILL_MAX_NUM	1
typedef	BinBuf<SLeaderMonsterKill, TBPLAYER_LEADER_MONSTER_KILL_MAX_NUM>		TbPlayer_Leader_monster_kill;

#define	TBPLAYER_LEADER_MONSTER_GEN_MAX_NUM	1
typedef	BinBuf<SLeaderMonsterGen, TBPLAYER_LEADER_MONSTER_GEN_MAX_NUM>		TbPlayer_Leader_monster_gen;

#define	TBPLAYER_TRIAL_MONSTER_NORMAL_MAX_NUM	1
typedef	BinBuf<STrialMonster, TBPLAYER_TRIAL_MONSTER_NORMAL_MAX_NUM>		TbPlayer_Trial_monster_normal;

#define	TBPLAYER_TRIAL_MONSTER_RAGE_MAX_NUM	1
typedef	BinBuf<STrialMonster, TBPLAYER_TRIAL_MONSTER_RAGE_MAX_NUM>		TbPlayer_Trial_monster_rage;

#define	TBPLAYER_TRIAL_LUCKY_BAG_NORMAL_MAX_NUM	1
typedef	BinBuf<STrialLuckyBagNormal, TBPLAYER_TRIAL_LUCKY_BAG_NORMAL_MAX_NUM>		TbPlayer_Trial_lucky_bag_normal;

#define	TBPLAYER_TRIAL_LUCKY_BAG_RAGE_MAX_NUM	1
typedef	BinBuf<STrialLuckyBagRage, TBPLAYER_TRIAL_LUCKY_BAG_RAGE_MAX_NUM>		TbPlayer_Trial_lucky_bag_rage;

#define	TBPLAYER_FINISH_GUIDE_LIST_MAX_NUM	1
typedef	BinBuf<SFinishGuideList, TBPLAYER_FINISH_GUIDE_LIST_MAX_NUM>		TbPlayer_Finish_guide_list;

#define	TBPLAYER_REINFORCE_LIMIT_MAX_NUM	1
typedef	BinBuf<SReinforceLimit, TBPLAYER_REINFORCE_LIMIT_MAX_NUM>		TbPlayer_Reinforce_limit;

class TbPlayer : public AwsTable
{
public:
	TINT64 m_nSid;
	TINT64 m_nUid;
	string m_sUin;
	TINT64 m_nAvatar;
	TINT64 m_nAge;
	TINT64 m_nCid;
	TINT64 m_nCtime;
	TINT64 m_nUtime;
	TINT64 m_nNpc;
	TINT64 m_nStatus;
	TINT64 m_nDead_flag;
	TINT64 m_nAlid;
	string m_sAlname;
	TINT64 m_nAlpos;
	TINT64 m_nAl_time;
	string m_sAl_nick_name;
	TINT64 m_nReq_al;
	TINT64 m_nJoin_alliance;
	TINT64 m_nLoy_time;
	TINT64 m_nLoy_itv;
	TINT64 m_nLoy_cur;
	TINT64 m_nLoy_all;
	TINT64 m_nInvite_mail_time;
	TINT64 m_nInvited_num;
	TINT64 m_nInvited_time;
	TINT64 m_nDoubloon;
	TINT64 m_nVip_point;
	TINT64 m_nVip_etime;
	TbPlayer_Dragon_statistics m_bDragon_statistics;
	TbPlayer_War_statistics m_bWar_statistics;
	TbPlayer_Transport_resource m_bTransport_resource;
	TbPlayer_Gain_resource m_bGain_resource;
	TINT64 m_nCur_troop_might;
	TINT64 m_nCur_fort_might;
	TINT64 m_nSend_al_help_num;
	TINT64 m_nBuilding_force;
	TINT64 m_nResearch_force;
	TINT64 m_nDragon_force;
	TINT64 m_nAlname_update_time;
	TINT64 m_nHas_change_svr;
	TINT64 m_nSvr_change_time;
	TbPlayer_Chest_lottery m_bChest_lottery;
	TbPlayer_Dragon m_bDragon;
	TINT64 m_nMight;
	TINT64 m_nDtroop;
	TINT64 m_nKtroop;
	TINT64 m_nDfort;
	TINT64 m_nKfort;
	TINT64 m_nBat;
	TINT64 m_nBat_suc;
	TINT64 m_nDcity;
	TINT64 m_nMkill;
	TINT64 m_nMgain;
	TINT64 m_nExp;
	TINT64 m_nLevel;
	TINT64 m_nLast_might;
	Json::Value m_jLast_troop_fort;
	TINT64 m_nSh_res;
	TINT64 m_nSh_might;
	TINT64 m_nRes_col;
	string m_sDragon_name;
	TINT64 m_nDragon_level;
	TINT64 m_nDragon_exp;
	TINT64 m_nDragon_avatar;
	TINT64 m_nDragon_status;
	TINT64 m_nDragon_tid;
	TINT64 m_nDragon_cur_energy;
	TINT64 m_nDragon_recovery_time;
	TINT64 m_nDragon_recovery_count;
	TINT64 m_nHas_dragon;
	TINT64 m_nDragon_begin_recovery_time;
	TINT64 m_nDragon_max_energy;
	TbPlayer_Person_guide m_bPerson_guide;
	TINT64 m_nDragon_max_lv;
	TbPlayer_Monster_hit m_bMonster_hit;
	TbPlayer_Leader_monster_kill m_bLeader_monster_kill;
	TbPlayer_Leader_monster_gen m_bLeader_monster_gen;
	Json::Value m_jDragon_equip;
	TINT64 m_nSeq;
	TINT64 m_nLord_force;
	TINT64 m_nTrial_init;
	TINT64 m_nDragon_shard;
	TINT64 m_nTrial_rage_open;
	TINT64 m_nTrial_rage_mode;
	TbPlayer_Trial_monster_normal m_bTrial_monster_normal;
	TbPlayer_Trial_monster_rage m_bTrial_monster_rage;
	TbPlayer_Trial_lucky_bag_normal m_bTrial_lucky_bag_normal;
	TbPlayer_Trial_lucky_bag_rage m_bTrial_lucky_bag_rage;
	TINT64 m_nTrial_gift_last_etime;
	TbPlayer_Finish_guide_list m_bFinish_guide_list;
	TbPlayer_Reinforce_limit m_bReinforce_limit;
	TINT64 m_nEvil_force_kill;
	TINT64 m_nEvil_troop_kill;
	TINT64 m_nVip_stage;

public:
	static TableDesc oTableDesc;
	Json::Reader oJsonReader;
	Json::FastWriter oJsonWriter;
	static int Init(const string& sConfFile, const string strProjectName);

public:
	TbPlayer():AwsTable(&oTableDesc)
	{
		Reset();
	}
	~TbPlayer()
	{}
	void Reset()
	{
		m_nSid = 0;
		m_nUid = 0;
		m_sUin = "";
		m_nAvatar = 0;
		m_nAge = 0;
		m_nCid = 0;
		m_nCtime = 0;
		m_nUtime = 0;
		m_nNpc = 0;
		m_nStatus = 0;
		m_nDead_flag = 0;
		m_nAlid = 0;
		m_sAlname = "";
		m_nAlpos = 0;
		m_nAl_time = 0;
		m_sAl_nick_name = "";
		m_nReq_al = 0;
		m_nJoin_alliance = 0;
		m_nLoy_time = 0;
		m_nLoy_itv = 0;
		m_nLoy_cur = 0;
		m_nLoy_all = 0;
		m_nInvite_mail_time = 0;
		m_nInvited_num = 0;
		m_nInvited_time = 0;
		m_nDoubloon = 0;
		m_nVip_point = 0;
		m_nVip_etime = 0;
		m_bDragon_statistics.Reset();
		m_bWar_statistics.Reset();
		m_bTransport_resource.Reset();
		m_bGain_resource.Reset();
		m_nCur_troop_might = 0;
		m_nCur_fort_might = 0;
		m_nSend_al_help_num = 0;
		m_nBuilding_force = 0;
		m_nResearch_force = 0;
		m_nDragon_force = 0;
		m_nAlname_update_time = 0;
		m_nHas_change_svr = 0;
		m_nSvr_change_time = 0;
		m_bChest_lottery.Reset();
		m_bDragon.Reset();
		m_nMight = 0;
		m_nDtroop = 0;
		m_nKtroop = 0;
		m_nDfort = 0;
		m_nKfort = 0;
		m_nBat = 0;
		m_nBat_suc = 0;
		m_nDcity = 0;
		m_nMkill = 0;
		m_nMgain = 0;
		m_nExp = 0;
		m_nLevel = 0;
		m_nLast_might = 0;
		m_jLast_troop_fort.clear();
		m_nSh_res = 0;
		m_nSh_might = 0;
		m_nRes_col = 0;
		m_sDragon_name = "";
		m_nDragon_level = 0;
		m_nDragon_exp = 0;
		m_nDragon_avatar = 0;
		m_nDragon_status = 0;
		m_nDragon_tid = 0;
		m_nDragon_cur_energy = 0;
		m_nDragon_recovery_time = 0;
		m_nDragon_recovery_count = 0;
		m_nHas_dragon = 0;
		m_nDragon_begin_recovery_time = 0;
		m_nDragon_max_energy = 0;
		m_bPerson_guide.Reset();
		m_nDragon_max_lv = 0;
		m_bMonster_hit.Reset();
		m_bLeader_monster_kill.Reset();
		m_bLeader_monster_gen.Reset();
		m_jDragon_equip.clear();
		m_nSeq = 0;
		m_nLord_force = 0;
		m_nTrial_init = 0;
		m_nDragon_shard = 0;
		m_nTrial_rage_open = 0;
		m_nTrial_rage_mode = 0;
		m_bTrial_monster_normal.Reset();
		m_bTrial_monster_rage.Reset();
		m_bTrial_lucky_bag_normal.Reset();
		m_bTrial_lucky_bag_rage.Reset();
		m_nTrial_gift_last_etime = 0;
		m_bFinish_guide_list.Reset();
		m_bReinforce_limit.Reset();
		m_nEvil_force_kill = 0;
		m_nEvil_troop_kill = 0;
		m_nVip_stage = 0;
		ClearFlag();
	};
	const TINT64& Get_Sid()
	{
		return m_nSid;
	}
	void Set_Sid(const TINT64& nSid, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSid = nSid;
		m_mFlag[TbPLAYER_FIELD_SID] = dwActionType;
	}
	const TINT64& Get_Uid()
	{
		return m_nUid;
	}
	void Set_Uid(const TINT64& nUid)
	{
		m_nUid = nUid;
	}
	const string& Get_Uin()
	{
		return m_sUin;
	}
	void Set_Uin(const string& sUin)
	{
		m_sUin = sUin;
		m_mFlag[TbPLAYER_FIELD_UIN] = sUin.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Avatar()
	{
		return m_nAvatar;
	}
	void Set_Avatar(const TINT64& nAvatar, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nAvatar = nAvatar;
		m_mFlag[TbPLAYER_FIELD_AVATAR] = dwActionType;
	}
	const TINT64& Get_Age()
	{
		return m_nAge;
	}
	void Set_Age(const TINT64& nAge, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nAge = nAge;
		m_mFlag[TbPLAYER_FIELD_AGE] = dwActionType;
	}
	const TINT64& Get_Cid()
	{
		return m_nCid;
	}
	void Set_Cid(const TINT64& nCid, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nCid = nCid;
		m_mFlag[TbPLAYER_FIELD_CID] = dwActionType;
	}
	const TINT64& Get_Ctime()
	{
		return m_nCtime;
	}
	void Set_Ctime(const TINT64& nCtime, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nCtime = nCtime;
		m_mFlag[TbPLAYER_FIELD_CTIME] = dwActionType;
	}
	const TINT64& Get_Utime()
	{
		return m_nUtime;
	}
	void Set_Utime(const TINT64& nUtime, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nUtime = nUtime;
		m_mFlag[TbPLAYER_FIELD_UTIME] = dwActionType;
	}
	const TINT64& Get_Npc()
	{
		return m_nNpc;
	}
	void Set_Npc(const TINT64& nNpc, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nNpc = nNpc;
		m_mFlag[TbPLAYER_FIELD_NPC] = dwActionType;
	}
	const TINT64& Get_Status()
	{
		return m_nStatus;
	}
	void Set_Status(const TINT64& nStatus, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nStatus = nStatus;
		m_mFlag[TbPLAYER_FIELD_STATUS] = dwActionType;
	}
	const TINT64& Get_Dead_flag()
	{
		return m_nDead_flag;
	}
	void Set_Dead_flag(const TINT64& nDead_flag, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nDead_flag = nDead_flag;
		m_mFlag[TbPLAYER_FIELD_DEAD_FLAG] = dwActionType;
	}
	const TINT64& Get_Alid()
	{
		return m_nAlid;
	}
	void Set_Alid(const TINT64& nAlid, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nAlid = nAlid;
		m_mFlag[TbPLAYER_FIELD_ALID] = dwActionType;
	}
	const string& Get_Alname()
	{
		return m_sAlname;
	}
	void Set_Alname(const string& sAlname)
	{
		m_sAlname = sAlname;
		m_mFlag[TbPLAYER_FIELD_ALNAME] = sAlname.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Alpos()
	{
		return m_nAlpos;
	}
	void Set_Alpos(const TINT64& nAlpos, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nAlpos = nAlpos;
		m_mFlag[TbPLAYER_FIELD_ALPOS] = dwActionType;
	}
	const TINT64& Get_Al_time()
	{
		return m_nAl_time;
	}
	void Set_Al_time(const TINT64& nAl_time, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nAl_time = nAl_time;
		m_mFlag[TbPLAYER_FIELD_AL_TIME] = dwActionType;
	}
	const string& Get_Al_nick_name()
	{
		return m_sAl_nick_name;
	}
	void Set_Al_nick_name(const string& sAl_nick_name)
	{
		m_sAl_nick_name = sAl_nick_name;
		m_mFlag[TbPLAYER_FIELD_AL_NICK_NAME] = sAl_nick_name.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Req_al()
	{
		return m_nReq_al;
	}
	void Set_Req_al(const TINT64& nReq_al, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nReq_al = nReq_al;
		m_mFlag[TbPLAYER_FIELD_REQ_AL] = dwActionType;
	}
	const TINT64& Get_Join_alliance()
	{
		return m_nJoin_alliance;
	}
	void Set_Join_alliance(const TINT64& nJoin_alliance, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nJoin_alliance = nJoin_alliance;
		m_mFlag[TbPLAYER_FIELD_JOIN_ALLIANCE] = dwActionType;
	}
	const TINT64& Get_Loy_time()
	{
		return m_nLoy_time;
	}
	void Set_Loy_time(const TINT64& nLoy_time, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nLoy_time = nLoy_time;
		m_mFlag[TbPLAYER_FIELD_LOY_TIME] = dwActionType;
	}
	const TINT64& Get_Loy_itv()
	{
		return m_nLoy_itv;
	}
	void Set_Loy_itv(const TINT64& nLoy_itv, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nLoy_itv = nLoy_itv;
		m_mFlag[TbPLAYER_FIELD_LOY_ITV] = dwActionType;
	}
	const TINT64& Get_Loy_cur()
	{
		return m_nLoy_cur;
	}
	void Set_Loy_cur(const TINT64& nLoy_cur, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nLoy_cur = nLoy_cur;
		m_mFlag[TbPLAYER_FIELD_LOY_CUR] = dwActionType;
	}
	const TINT64& Get_Loy_all()
	{
		return m_nLoy_all;
	}
	void Set_Loy_all(const TINT64& nLoy_all, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nLoy_all = nLoy_all;
		m_mFlag[TbPLAYER_FIELD_LOY_ALL] = dwActionType;
	}
	const TINT64& Get_Invite_mail_time()
	{
		return m_nInvite_mail_time;
	}
	void Set_Invite_mail_time(const TINT64& nInvite_mail_time, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nInvite_mail_time = nInvite_mail_time;
		m_mFlag[TbPLAYER_FIELD_INVITE_MAIL_TIME] = dwActionType;
	}
	const TINT64& Get_Invited_num()
	{
		return m_nInvited_num;
	}
	void Set_Invited_num(const TINT64& nInvited_num, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nInvited_num = nInvited_num;
		m_mFlag[TbPLAYER_FIELD_INVITED_NUM] = dwActionType;
	}
	const TINT64& Get_Invited_time()
	{
		return m_nInvited_time;
	}
	void Set_Invited_time(const TINT64& nInvited_time, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nInvited_time = nInvited_time;
		m_mFlag[TbPLAYER_FIELD_INVITED_TIME] = dwActionType;
	}
	const TINT64& Get_Doubloon()
	{
		return m_nDoubloon;
	}
	void Set_Doubloon(const TINT64& nDoubloon, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nDoubloon = nDoubloon;
		m_mFlag[TbPLAYER_FIELD_DOUBLOON] = dwActionType;
	}
	const TINT64& Get_Vip_point()
	{
		return m_nVip_point;
	}
	void Set_Vip_point(const TINT64& nVip_point, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nVip_point = nVip_point;
		m_mFlag[TbPLAYER_FIELD_VIP_POINT] = dwActionType;
	}
	const TINT64& Get_Vip_etime()
	{
		return m_nVip_etime;
	}
	void Set_Vip_etime(const TINT64& nVip_etime, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nVip_etime = nVip_etime;
		m_mFlag[TbPLAYER_FIELD_VIP_ETIME] = dwActionType;
	}
	const TINT64& Get_Cur_troop_might()
	{
		return m_nCur_troop_might;
	}
	void Set_Cur_troop_might(const TINT64& nCur_troop_might, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nCur_troop_might = nCur_troop_might;
		m_mFlag[TbPLAYER_FIELD_CUR_TROOP_MIGHT] = dwActionType;
	}
	const TINT64& Get_Cur_fort_might()
	{
		return m_nCur_fort_might;
	}
	void Set_Cur_fort_might(const TINT64& nCur_fort_might, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nCur_fort_might = nCur_fort_might;
		m_mFlag[TbPLAYER_FIELD_CUR_FORT_MIGHT] = dwActionType;
	}
	const TINT64& Get_Send_al_help_num()
	{
		return m_nSend_al_help_num;
	}
	void Set_Send_al_help_num(const TINT64& nSend_al_help_num, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSend_al_help_num = nSend_al_help_num;
		m_mFlag[TbPLAYER_FIELD_SEND_AL_HELP_NUM] = dwActionType;
	}
	const TINT64& Get_Building_force()
	{
		return m_nBuilding_force;
	}
	void Set_Building_force(const TINT64& nBuilding_force, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nBuilding_force = nBuilding_force;
		m_mFlag[TbPLAYER_FIELD_BUILDING_FORCE] = dwActionType;
	}
	const TINT64& Get_Research_force()
	{
		return m_nResearch_force;
	}
	void Set_Research_force(const TINT64& nResearch_force, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nResearch_force = nResearch_force;
		m_mFlag[TbPLAYER_FIELD_RESEARCH_FORCE] = dwActionType;
	}
	const TINT64& Get_Dragon_force()
	{
		return m_nDragon_force;
	}
	void Set_Dragon_force(const TINT64& nDragon_force, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nDragon_force = nDragon_force;
		m_mFlag[TbPLAYER_FIELD_DRAGON_FORCE] = dwActionType;
	}
	const TINT64& Get_Alname_update_time()
	{
		return m_nAlname_update_time;
	}
	void Set_Alname_update_time(const TINT64& nAlname_update_time, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nAlname_update_time = nAlname_update_time;
		m_mFlag[TbPLAYER_FIELD_ALNAME_UPDATE_TIME] = dwActionType;
	}
	const TINT64& Get_Has_change_svr()
	{
		return m_nHas_change_svr;
	}
	void Set_Has_change_svr(const TINT64& nHas_change_svr, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nHas_change_svr = nHas_change_svr;
		m_mFlag[TbPLAYER_FIELD_HAS_CHANGE_SVR] = dwActionType;
	}
	const TINT64& Get_Svr_change_time()
	{
		return m_nSvr_change_time;
	}
	void Set_Svr_change_time(const TINT64& nSvr_change_time, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSvr_change_time = nSvr_change_time;
		m_mFlag[TbPLAYER_FIELD_SVR_CHANGE_TIME] = dwActionType;
	}
	const TINT64& Get_Might()
	{
		return m_nMight;
	}
	void Set_Might(const TINT64& nMight, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nMight = nMight;
		m_mFlag[TbPLAYER_FIELD_MIGHT] = dwActionType;
	}
	const TINT64& Get_Dtroop()
	{
		return m_nDtroop;
	}
	void Set_Dtroop(const TINT64& nDtroop, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nDtroop = nDtroop;
		m_mFlag[TbPLAYER_FIELD_DTROOP] = dwActionType;
	}
	const TINT64& Get_Ktroop()
	{
		return m_nKtroop;
	}
	void Set_Ktroop(const TINT64& nKtroop, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nKtroop = nKtroop;
		m_mFlag[TbPLAYER_FIELD_KTROOP] = dwActionType;
	}
	const TINT64& Get_Dfort()
	{
		return m_nDfort;
	}
	void Set_Dfort(const TINT64& nDfort, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nDfort = nDfort;
		m_mFlag[TbPLAYER_FIELD_DFORT] = dwActionType;
	}
	const TINT64& Get_Kfort()
	{
		return m_nKfort;
	}
	void Set_Kfort(const TINT64& nKfort, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nKfort = nKfort;
		m_mFlag[TbPLAYER_FIELD_KFORT] = dwActionType;
	}
	const TINT64& Get_Bat()
	{
		return m_nBat;
	}
	void Set_Bat(const TINT64& nBat, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nBat = nBat;
		m_mFlag[TbPLAYER_FIELD_BAT] = dwActionType;
	}
	const TINT64& Get_Bat_suc()
	{
		return m_nBat_suc;
	}
	void Set_Bat_suc(const TINT64& nBat_suc, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nBat_suc = nBat_suc;
		m_mFlag[TbPLAYER_FIELD_BAT_SUC] = dwActionType;
	}
	const TINT64& Get_Dcity()
	{
		return m_nDcity;
	}
	void Set_Dcity(const TINT64& nDcity, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nDcity = nDcity;
		m_mFlag[TbPLAYER_FIELD_DCITY] = dwActionType;
	}
	const TINT64& Get_Mkill()
	{
		return m_nMkill;
	}
	void Set_Mkill(const TINT64& nMkill, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nMkill = nMkill;
		m_mFlag[TbPLAYER_FIELD_MKILL] = dwActionType;
	}
	const TINT64& Get_Mgain()
	{
		return m_nMgain;
	}
	void Set_Mgain(const TINT64& nMgain, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nMgain = nMgain;
		m_mFlag[TbPLAYER_FIELD_MGAIN] = dwActionType;
	}
	const TINT64& Get_Exp()
	{
		return m_nExp;
	}
	void Set_Exp(const TINT64& nExp, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nExp = nExp;
		m_mFlag[TbPLAYER_FIELD_EXP] = dwActionType;
	}
	const TINT64& Get_Level()
	{
		return m_nLevel;
	}
	void Set_Level(const TINT64& nLevel, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nLevel = nLevel;
		m_mFlag[TbPLAYER_FIELD_LEVEL] = dwActionType;
	}
	const TINT64& Get_Last_might()
	{
		return m_nLast_might;
	}
	void Set_Last_might(const TINT64& nLast_might, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nLast_might = nLast_might;
		m_mFlag[TbPLAYER_FIELD_LAST_MIGHT] = dwActionType;
	}
	const Json::Value& Get_Last_troop_fort()
	{
		return m_jLast_troop_fort;
	}
	void Set_Last_troop_fort(const Json::Value& jLast_troop_fort)
	{
		m_jLast_troop_fort = jLast_troop_fort;
		m_mFlag[TbPLAYER_FIELD_LAST_TROOP_FORT] = jLast_troop_fort.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Sh_res()
	{
		return m_nSh_res;
	}
	void Set_Sh_res(const TINT64& nSh_res, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSh_res = nSh_res;
		m_mFlag[TbPLAYER_FIELD_SH_RES] = dwActionType;
	}
	const TINT64& Get_Sh_might()
	{
		return m_nSh_might;
	}
	void Set_Sh_might(const TINT64& nSh_might, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSh_might = nSh_might;
		m_mFlag[TbPLAYER_FIELD_SH_MIGHT] = dwActionType;
	}
	const TINT64& Get_Res_col()
	{
		return m_nRes_col;
	}
	void Set_Res_col(const TINT64& nRes_col, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nRes_col = nRes_col;
		m_mFlag[TbPLAYER_FIELD_RES_COL] = dwActionType;
	}
	const string& Get_Dragon_name()
	{
		return m_sDragon_name;
	}
	void Set_Dragon_name(const string& sDragon_name)
	{
		m_sDragon_name = sDragon_name;
		m_mFlag[TbPLAYER_FIELD_DRAGON_NAME] = sDragon_name.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Dragon_level()
	{
		return m_nDragon_level;
	}
	void Set_Dragon_level(const TINT64& nDragon_level, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nDragon_level = nDragon_level;
		m_mFlag[TbPLAYER_FIELD_DRAGON_LEVEL] = dwActionType;
	}
	const TINT64& Get_Dragon_exp()
	{
		return m_nDragon_exp;
	}
	void Set_Dragon_exp(const TINT64& nDragon_exp, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nDragon_exp = nDragon_exp;
		m_mFlag[TbPLAYER_FIELD_DRAGON_EXP] = dwActionType;
	}
	const TINT64& Get_Dragon_avatar()
	{
		return m_nDragon_avatar;
	}
	void Set_Dragon_avatar(const TINT64& nDragon_avatar, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nDragon_avatar = nDragon_avatar;
		m_mFlag[TbPLAYER_FIELD_DRAGON_AVATAR] = dwActionType;
	}
	const TINT64& Get_Dragon_status()
	{
		return m_nDragon_status;
	}
	void Set_Dragon_status(const TINT64& nDragon_status, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nDragon_status = nDragon_status;
		m_mFlag[TbPLAYER_FIELD_DRAGON_STATUS] = dwActionType;
	}
	const TINT64& Get_Dragon_tid()
	{
		return m_nDragon_tid;
	}
	void Set_Dragon_tid(const TINT64& nDragon_tid, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nDragon_tid = nDragon_tid;
		m_mFlag[TbPLAYER_FIELD_DRAGON_TID] = dwActionType;
	}
	const TINT64& Get_Dragon_cur_energy()
	{
		return m_nDragon_cur_energy;
	}
	void Set_Dragon_cur_energy(const TINT64& nDragon_cur_energy, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nDragon_cur_energy = nDragon_cur_energy;
		m_mFlag[TbPLAYER_FIELD_DRAGON_CUR_ENERGY] = dwActionType;
	}
	const TINT64& Get_Dragon_recovery_time()
	{
		return m_nDragon_recovery_time;
	}
	void Set_Dragon_recovery_time(const TINT64& nDragon_recovery_time, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nDragon_recovery_time = nDragon_recovery_time;
		m_mFlag[TbPLAYER_FIELD_DRAGON_RECOVERY_TIME] = dwActionType;
	}
	const TINT64& Get_Dragon_recovery_count()
	{
		return m_nDragon_recovery_count;
	}
	void Set_Dragon_recovery_count(const TINT64& nDragon_recovery_count, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nDragon_recovery_count = nDragon_recovery_count;
		m_mFlag[TbPLAYER_FIELD_DRAGON_RECOVERY_COUNT] = dwActionType;
	}
	const TINT64& Get_Has_dragon()
	{
		return m_nHas_dragon;
	}
	void Set_Has_dragon(const TINT64& nHas_dragon, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nHas_dragon = nHas_dragon;
		m_mFlag[TbPLAYER_FIELD_HAS_DRAGON] = dwActionType;
	}
	const TINT64& Get_Dragon_begin_recovery_time()
	{
		return m_nDragon_begin_recovery_time;
	}
	void Set_Dragon_begin_recovery_time(const TINT64& nDragon_begin_recovery_time, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nDragon_begin_recovery_time = nDragon_begin_recovery_time;
		m_mFlag[TbPLAYER_FIELD_DRAGON_BEGIN_RECOVERY_TIME] = dwActionType;
	}
	const TINT64& Get_Dragon_max_energy()
	{
		return m_nDragon_max_energy;
	}
	void Set_Dragon_max_energy(const TINT64& nDragon_max_energy, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nDragon_max_energy = nDragon_max_energy;
		m_mFlag[TbPLAYER_FIELD_DRAGON_MAX_ENERGY] = dwActionType;
	}
	const TINT64& Get_Dragon_max_lv()
	{
		return m_nDragon_max_lv;
	}
	void Set_Dragon_max_lv(const TINT64& nDragon_max_lv, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nDragon_max_lv = nDragon_max_lv;
		m_mFlag[TbPLAYER_FIELD_DRAGON_MAX_LV] = dwActionType;
	}
	const Json::Value& Get_Dragon_equip()
	{
		return m_jDragon_equip;
	}
	void Set_Dragon_equip(const Json::Value& jDragon_equip)
	{
		m_jDragon_equip = jDragon_equip;
		m_mFlag[TbPLAYER_FIELD_DRAGON_EQUIP] = jDragon_equip.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Seq()
	{
		return m_nSeq;
	}
	void Set_Seq(const TINT64& nSeq, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSeq = nSeq;
		m_mFlag[TbPLAYER_FIELD_SEQ] = dwActionType;
	}
	const TINT64& Get_Lord_force()
	{
		return m_nLord_force;
	}
	void Set_Lord_force(const TINT64& nLord_force, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nLord_force = nLord_force;
		m_mFlag[TbPLAYER_FIELD_LORD_FORCE] = dwActionType;
	}
	const TINT64& Get_Trial_init()
	{
		return m_nTrial_init;
	}
	void Set_Trial_init(const TINT64& nTrial_init, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nTrial_init = nTrial_init;
		m_mFlag[TbPLAYER_FIELD_TRIAL_INIT] = dwActionType;
	}
	const TINT64& Get_Dragon_shard()
	{
		return m_nDragon_shard;
	}
	void Set_Dragon_shard(const TINT64& nDragon_shard, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nDragon_shard = nDragon_shard;
		m_mFlag[TbPLAYER_FIELD_DRAGON_SHARD] = dwActionType;
	}
	const TINT64& Get_Trial_rage_open()
	{
		return m_nTrial_rage_open;
	}
	void Set_Trial_rage_open(const TINT64& nTrial_rage_open, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nTrial_rage_open = nTrial_rage_open;
		m_mFlag[TbPLAYER_FIELD_TRIAL_RAGE_OPEN] = dwActionType;
	}
	const TINT64& Get_Trial_rage_mode()
	{
		return m_nTrial_rage_mode;
	}
	void Set_Trial_rage_mode(const TINT64& nTrial_rage_mode, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nTrial_rage_mode = nTrial_rage_mode;
		m_mFlag[TbPLAYER_FIELD_TRIAL_RAGE_MODE] = dwActionType;
	}
	const TINT64& Get_Trial_gift_last_etime()
	{
		return m_nTrial_gift_last_etime;
	}
	void Set_Trial_gift_last_etime(const TINT64& nTrial_gift_last_etime, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nTrial_gift_last_etime = nTrial_gift_last_etime;
		m_mFlag[TbPLAYER_FIELD_TRIAL_GIFT_LAST_ETIME] = dwActionType;
	}
	const TINT64& Get_Evil_force_kill()
	{
		return m_nEvil_force_kill;
	}
	void Set_Evil_force_kill(const TINT64& nEvil_force_kill, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nEvil_force_kill = nEvil_force_kill;
		m_mFlag[TbPLAYER_FIELD_EVIL_FORCE_KILL] = dwActionType;
	}
	const TINT64& Get_Evil_troop_kill()
	{
		return m_nEvil_troop_kill;
	}
	void Set_Evil_troop_kill(const TINT64& nEvil_troop_kill, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nEvil_troop_kill = nEvil_troop_kill;
		m_mFlag[TbPLAYER_FIELD_EVIL_TROOP_KILL] = dwActionType;
	}
	const TINT64& Get_Vip_stage()
	{
		return m_nVip_stage;
	}
	void Set_Vip_stage(const TINT64& nVip_stage, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nVip_stage = nVip_stage;
		m_mFlag[TbPLAYER_FIELD_VIP_STAGE] = dwActionType;
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

