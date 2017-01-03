#ifndef _AWS_TABLE_MARCH_ACTION_H_
#define _AWS_TABLE_MARCH_ACTION_H_

#include "aws_table_common.h"
#include "field_property.h"

#define EN_AWS_TABLE_MARCH_ACTION "march_action"

enum ETbMarch_actionField
{
	TbMARCH_ACTION_FIELD_ID = 0,
	TbMARCH_ACTION_FIELD_SUID = 1,
	TbMARCH_ACTION_FIELD_SCID = 2,
	TbMARCH_ACTION_FIELD_FIXED = 3,
	TbMARCH_ACTION_FIELD_MCLASS = 4,
	TbMARCH_ACTION_FIELD_SCLASS = 5,
	TbMARCH_ACTION_FIELD_STATUS = 6,
	TbMARCH_ACTION_FIELD_BTIME = 7,
	TbMARCH_ACTION_FIELD_CTIME = 8,
	TbMARCH_ACTION_FIELD_ETIME = 9,
	TbMARCH_ACTION_FIELD_PARAM = 10,
	TbMARCH_ACTION_FIELD_RETRY = 11,
	TbMARCH_ACTION_FIELD_SID = 12,
	TbMARCH_ACTION_FIELD_NOTIC_TASK_FLAG = 13,
	TbMARCH_ACTION_FIELD_SAL = 14,
	TbMARCH_ACTION_FIELD_TAL = 15,
	TbMARCH_ACTION_FIELD_TID = 16,
	TbMARCH_ACTION_FIELD_SBID = 17,
	TbMARCH_ACTION_FIELD_TBID = 18,
	TbMARCH_ACTION_FIELD_KILL_TROOP_MIGHT = 19,
	TbMARCH_ACTION_FIELD_KILL_FORT_MIGHT = 20,
	TbMARCH_ACTION_FIELD_BUFF = 21,
	TbMARCH_ACTION_FIELD_EXPIRING_BUFF = 22,
	TbMARCH_ACTION_FIELD_REWARD = 23,
	TbMARCH_ACTION_FIELD_SP_REWARD = 24,
	TbMARCH_ACTION_FIELD_TPOS = 25,
	TbMARCH_ACTION_FIELD_TUID = 26,
	TbMARCH_ACTION_FIELD_IF_MAX_ATTACK = 27,
	TbMARCH_ACTION_FIELD_RALLY_ATK_FORCE = 28,
	TbMARCH_ACTION_FIELD_RALLY_ATK_SLOT = 29,
	TbMARCH_ACTION_FIELD_RALLY_DEF_FORCE = 30,
	TbMARCH_ACTION_FIELD_RALLY_DEF_SLOT = 31,
	TbMARCH_ACTION_FIELD_PRISON_PARAM = 32,
	TbMARCH_ACTION_FIELD_CITY_INFO = 33,
	TbMARCH_ACTION_FIELD_REINFORCE_RESULT = 34,
	TbMARCH_ACTION_FIELD_KILL_TROOP_NUM = 35,
	TbMARCH_ACTION_FIELD_KILL_FORT_NUM = 36,
	TbMARCH_ACTION_FIELD_IS_RECALLED = 37,
	TbMARCH_ACTION_FIELD_SAVATAR = 38,
	TbMARCH_ACTION_FIELD_AL_GIFT_RECORD = 39,
	TbMARCH_ACTION_FIELD_ATK_TOTAL_TROOP = 40,
	TbMARCH_ACTION_FIELD_TRADE_INFO = 41,
	TbMARCH_ACTION_FIELD_TAX_INFO = 42,
	TbMARCH_ACTION_FIELD_TRADE_LIST_IDX = 43,
	TbMARCH_ACTION_FIELD_MONSTER_INFO = 44,
	TbMARCH_ACTION_FIELD_DELAY_REPORT_ID = 45,
	TbMARCH_ACTION_FIELD_SEQ = 46,
	TbMARCH_ACTION_FIELD_DEF_TOTAL_TROOP = 47,
	TbMARCH_ACTION_FIELD_EX_REWARD = 48,
};

enum ETbMarch_actionOpenType
{
	ETbMARCH_OPEN_TYPE_PRIMARY = 0,
	ETbMARCH_OPEN_TYPE_GLB_ETIME = 1,
	ETbMARCH_OPEN_TYPE_GLB_SAL = 2,
	ETbMARCH_OPEN_TYPE_GLB_TAL = 3,
	ETbMARCH_OPEN_TYPE_GLB_SBID = 4,
	ETbMARCH_OPEN_TYPE_GLB_TBID = 5,
	ETbMARCH_OPEN_TYPE_GLB_TUID = 6,
};

#define	TBMARCH_ACTION_PARAM_MAX_NUM	1
typedef	BinBuf<SActionMarchParam, TBMARCH_ACTION_PARAM_MAX_NUM>		TbMarch_action_Param;

#define	TBMARCH_ACTION_NOTIC_TASK_FLAG_MAX_NUM	1
typedef	BinBuf<SNotictaskFlag, TBMARCH_ACTION_NOTIC_TASK_FLAG_MAX_NUM>		TbMarch_action_Notic_task_flag;

#define	TBMARCH_ACTION_BUFF_MAX_NUM	100
typedef	BinBuf<SBuffInfo, TBMARCH_ACTION_BUFF_MAX_NUM>		TbMarch_action_Buff;

#define	TBMARCH_ACTION_EXPIRING_BUFF_MAX_NUM	100
typedef	BinBuf<SBuffInfo, TBMARCH_ACTION_EXPIRING_BUFF_MAX_NUM>		TbMarch_action_Expiring_buff;

#define	TBMARCH_ACTION_REWARD_MAX_NUM	1
typedef	BinBuf<SGlobalRes, TBMARCH_ACTION_REWARD_MAX_NUM>		TbMarch_action_Reward;

#define	TBMARCH_ACTION_SP_REWARD_MAX_NUM	1
typedef	BinBuf<SGlobalRes, TBMARCH_ACTION_SP_REWARD_MAX_NUM>		TbMarch_action_Sp_reward;

#define	TBMARCH_ACTION_RALLY_ATK_FORCE_MAX_NUM	1
typedef	BinBuf<SRallyForce, TBMARCH_ACTION_RALLY_ATK_FORCE_MAX_NUM>		TbMarch_action_Rally_atk_force;

#define	TBMARCH_ACTION_RALLY_ATK_SLOT_MAX_NUM	50
typedef	BinBuf<SRallySlot, TBMARCH_ACTION_RALLY_ATK_SLOT_MAX_NUM>		TbMarch_action_Rally_atk_slot;

#define	TBMARCH_ACTION_RALLY_DEF_FORCE_MAX_NUM	1
typedef	BinBuf<SRallyForce, TBMARCH_ACTION_RALLY_DEF_FORCE_MAX_NUM>		TbMarch_action_Rally_def_force;

#define	TBMARCH_ACTION_RALLY_DEF_SLOT_MAX_NUM	50
typedef	BinBuf<SRallySlot, TBMARCH_ACTION_RALLY_DEF_SLOT_MAX_NUM>		TbMarch_action_Rally_def_slot;

#define	TBMARCH_ACTION_PRISON_PARAM_MAX_NUM	1
typedef	BinBuf<SPrisonParam, TBMARCH_ACTION_PRISON_PARAM_MAX_NUM>		TbMarch_action_Prison_param;

#define	TBMARCH_ACTION_REINFORCE_RESULT_MAX_NUM	1
typedef	BinBuf<SReinforceResult, TBMARCH_ACTION_REINFORCE_RESULT_MAX_NUM>		TbMarch_action_Reinforce_result;

#define	TBMARCH_ACTION_AL_GIFT_RECORD_MAX_NUM	1
typedef	BinBuf<SGlobalRes, TBMARCH_ACTION_AL_GIFT_RECORD_MAX_NUM>		TbMarch_action_Al_gift_record;

#define	TBMARCH_ACTION_ATK_TOTAL_TROOP_MAX_NUM	1
typedef	BinBuf<SCommonTroop, TBMARCH_ACTION_ATK_TOTAL_TROOP_MAX_NUM>		TbMarch_action_Atk_total_troop;

#define	TBMARCH_ACTION_TRADE_INFO_MAX_NUM	1
typedef	BinBuf<STradeInfo, TBMARCH_ACTION_TRADE_INFO_MAX_NUM>		TbMarch_action_Trade_info;

#define	TBMARCH_ACTION_MONSTER_INFO_MAX_NUM	1
typedef	BinBuf<SMonsterInfo, TBMARCH_ACTION_MONSTER_INFO_MAX_NUM>		TbMarch_action_Monster_info;

#define	TBMARCH_ACTION_DEF_TOTAL_TROOP_MAX_NUM	1
typedef	BinBuf<SCommonTroop, TBMARCH_ACTION_DEF_TOTAL_TROOP_MAX_NUM>		TbMarch_action_Def_total_troop;

#define	TBMARCH_ACTION_EX_REWARD_MAX_NUM	100
typedef	BinBuf<SOneGlobalRes, TBMARCH_ACTION_EX_REWARD_MAX_NUM>		TbMarch_action_Ex_reward;

class TbMarch_action : public AwsTable
{
public:
	TINT64 m_nId;
	TINT64 m_nSuid;
	TINT64 m_nScid;
	TINT64 m_nFixed;
	TINT64 m_nMclass;
	TINT64 m_nSclass;
	TINT64 m_nStatus;
	TINT64 m_nBtime;
	TINT64 m_nCtime;
	TINT64 m_nEtime;
	TbMarch_action_Param m_bParam;
	TINT64 m_nRetry;
	TINT64 m_nSid;
	TbMarch_action_Notic_task_flag m_bNotic_task_flag;
	TINT64 m_nSal;
	TINT64 m_nTal;
	TINT64 m_nTid;
	TINT64 m_nSbid;
	TINT64 m_nTbid;
	TINT64 m_nKill_troop_might;
	TINT64 m_nKill_fort_might;
	TbMarch_action_Buff m_bBuff;
	TbMarch_action_Expiring_buff m_bExpiring_buff;
	TbMarch_action_Reward m_bReward;
	TbMarch_action_Sp_reward m_bSp_reward;
	TINT64 m_nTpos;
	TINT64 m_nTuid;
	TINT64 m_nIf_max_attack;
	TbMarch_action_Rally_atk_force m_bRally_atk_force;
	TbMarch_action_Rally_atk_slot m_bRally_atk_slot;
	TbMarch_action_Rally_def_force m_bRally_def_force;
	TbMarch_action_Rally_def_slot m_bRally_def_slot;
	TbMarch_action_Prison_param m_bPrison_param;
	Json::Value m_jCity_info;
	TbMarch_action_Reinforce_result m_bReinforce_result;
	TINT64 m_nKill_troop_num;
	TINT64 m_nKill_fort_num;
	TINT64 m_nIs_recalled;
	TINT64 m_nSavatar;
	TbMarch_action_Al_gift_record m_bAl_gift_record;
	TbMarch_action_Atk_total_troop m_bAtk_total_troop;
	TbMarch_action_Trade_info m_bTrade_info;
	Json::Value m_jTax_info;
	TINT64 m_nTrade_list_idx;
	TbMarch_action_Monster_info m_bMonster_info;
	TINT64 m_nDelay_report_id;
	TINT64 m_nSeq;
	TbMarch_action_Def_total_troop m_bDef_total_troop;
	TbMarch_action_Ex_reward m_bEx_reward;

public:
	static TableDesc oTableDesc;
	Json::Reader oJsonReader;
	Json::FastWriter oJsonWriter;
	static int Init(const string& sConfFile, const string strProjectName);

public:
	TbMarch_action():AwsTable(&oTableDesc)
	{
		Reset();
	}
	~TbMarch_action()
	{}
	void Reset()
	{
		m_nId = 0;
		m_nSuid = 0;
		m_nScid = 0;
		m_nFixed = 0;
		m_nMclass = 0;
		m_nSclass = 0;
		m_nStatus = 0;
		m_nBtime = 0;
		m_nCtime = 0;
		m_nEtime = 0;
		m_bParam.Reset();
		m_nRetry = 0;
		m_nSid = 0;
		m_bNotic_task_flag.Reset();
		m_nSal = 0;
		m_nTal = 0;
		m_nTid = 0;
		m_nSbid = 0;
		m_nTbid = 0;
		m_nKill_troop_might = 0;
		m_nKill_fort_might = 0;
		m_bBuff.Reset();
		m_bExpiring_buff.Reset();
		m_bReward.Reset();
		m_bSp_reward.Reset();
		m_nTpos = 0;
		m_nTuid = 0;
		m_nIf_max_attack = 0;
		m_bRally_atk_force.Reset();
		m_bRally_atk_slot.Reset();
		m_bRally_def_force.Reset();
		m_bRally_def_slot.Reset();
		m_bPrison_param.Reset();
		m_jCity_info.clear();
		m_bReinforce_result.Reset();
		m_nKill_troop_num = 0;
		m_nKill_fort_num = 0;
		m_nIs_recalled = 0;
		m_nSavatar = 0;
		m_bAl_gift_record.Reset();
		m_bAtk_total_troop.Reset();
		m_bTrade_info.Reset();
		m_jTax_info.clear();
		m_nTrade_list_idx = 0;
		m_bMonster_info.Reset();
		m_nDelay_report_id = 0;
		m_nSeq = 0;
		m_bDef_total_troop.Reset();
		m_bEx_reward.Reset();
		ClearFlag();
	};
	const TINT64& Get_Id()
	{
		return m_nId;
	}
	void Set_Id(const TINT64& nId)
	{
		m_nId = nId;
	}
	const TINT64& Get_Suid()
	{
		return m_nSuid;
	}
	void Set_Suid(const TINT64& nSuid)
	{
		m_nSuid = nSuid;
	}
	const TINT64& Get_Scid()
	{
		return m_nScid;
	}
	void Set_Scid(const TINT64& nScid, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nScid = nScid;
		m_mFlag[TbMARCH_ACTION_FIELD_SCID] = dwActionType;
	}
	const TINT64& Get_Fixed()
	{
		return m_nFixed;
	}
	void Set_Fixed(const TINT64& nFixed, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nFixed = nFixed;
		m_mFlag[TbMARCH_ACTION_FIELD_FIXED] = dwActionType;
	}
	const TINT64& Get_Mclass()
	{
		return m_nMclass;
	}
	void Set_Mclass(const TINT64& nMclass, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nMclass = nMclass;
		m_mFlag[TbMARCH_ACTION_FIELD_MCLASS] = dwActionType;
	}
	const TINT64& Get_Sclass()
	{
		return m_nSclass;
	}
	void Set_Sclass(const TINT64& nSclass, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSclass = nSclass;
		m_mFlag[TbMARCH_ACTION_FIELD_SCLASS] = dwActionType;
	}
	const TINT64& Get_Status()
	{
		return m_nStatus;
	}
	void Set_Status(const TINT64& nStatus, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nStatus = nStatus;
		m_mFlag[TbMARCH_ACTION_FIELD_STATUS] = dwActionType;
	}
	const TINT64& Get_Btime()
	{
		return m_nBtime;
	}
	void Set_Btime(const TINT64& nBtime, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nBtime = nBtime;
		m_mFlag[TbMARCH_ACTION_FIELD_BTIME] = dwActionType;
	}
	const TINT64& Get_Ctime()
	{
		return m_nCtime;
	}
	void Set_Ctime(const TINT64& nCtime, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nCtime = nCtime;
		m_mFlag[TbMARCH_ACTION_FIELD_CTIME] = dwActionType;
	}
	const TINT64& Get_Etime()
	{
		return m_nEtime;
	}
	void Set_Etime(const TINT64& nEtime, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nEtime = nEtime;
		m_mFlag[TbMARCH_ACTION_FIELD_ETIME] = dwActionType;
	}
	const TINT64& Get_Retry()
	{
		return m_nRetry;
	}
	void Set_Retry(const TINT64& nRetry, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nRetry = nRetry;
		m_mFlag[TbMARCH_ACTION_FIELD_RETRY] = dwActionType;
	}
	const TINT64& Get_Sid()
	{
		return m_nSid;
	}
	void Set_Sid(const TINT64& nSid, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSid = nSid;
		m_mFlag[TbMARCH_ACTION_FIELD_SID] = dwActionType;
	}
	const TINT64& Get_Sal()
	{
		return m_nSal;
	}
	void Set_Sal(const TINT64& nSal, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSal = nSal;
		m_mFlag[TbMARCH_ACTION_FIELD_SAL] = dwActionType;
	}
	const TINT64& Get_Tal()
	{
		return m_nTal;
	}
	void Set_Tal(const TINT64& nTal, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nTal = nTal;
		m_mFlag[TbMARCH_ACTION_FIELD_TAL] = dwActionType;
	}
	const TINT64& Get_Tid()
	{
		return m_nTid;
	}
	void Set_Tid(const TINT64& nTid, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nTid = nTid;
		m_mFlag[TbMARCH_ACTION_FIELD_TID] = dwActionType;
	}
	const TINT64& Get_Sbid()
	{
		return m_nSbid;
	}
	void Set_Sbid(const TINT64& nSbid, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSbid = nSbid;
		m_mFlag[TbMARCH_ACTION_FIELD_SBID] = dwActionType;
	}
	const TINT64& Get_Tbid()
	{
		return m_nTbid;
	}
	void Set_Tbid(const TINT64& nTbid, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nTbid = nTbid;
		m_mFlag[TbMARCH_ACTION_FIELD_TBID] = dwActionType;
	}
	const TINT64& Get_Kill_troop_might()
	{
		return m_nKill_troop_might;
	}
	void Set_Kill_troop_might(const TINT64& nKill_troop_might, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nKill_troop_might = nKill_troop_might;
		m_mFlag[TbMARCH_ACTION_FIELD_KILL_TROOP_MIGHT] = dwActionType;
	}
	const TINT64& Get_Kill_fort_might()
	{
		return m_nKill_fort_might;
	}
	void Set_Kill_fort_might(const TINT64& nKill_fort_might, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nKill_fort_might = nKill_fort_might;
		m_mFlag[TbMARCH_ACTION_FIELD_KILL_FORT_MIGHT] = dwActionType;
	}
	const TINT64& Get_Tpos()
	{
		return m_nTpos;
	}
	void Set_Tpos(const TINT64& nTpos, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nTpos = nTpos;
		m_mFlag[TbMARCH_ACTION_FIELD_TPOS] = dwActionType;
	}
	const TINT64& Get_Tuid()
	{
		return m_nTuid;
	}
	void Set_Tuid(const TINT64& nTuid, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nTuid = nTuid;
		m_mFlag[TbMARCH_ACTION_FIELD_TUID] = dwActionType;
	}
	const TINT64& Get_If_max_attack()
	{
		return m_nIf_max_attack;
	}
	void Set_If_max_attack(const TINT64& nIf_max_attack, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nIf_max_attack = nIf_max_attack;
		m_mFlag[TbMARCH_ACTION_FIELD_IF_MAX_ATTACK] = dwActionType;
	}
	const Json::Value& Get_City_info()
	{
		return m_jCity_info;
	}
	void Set_City_info(const Json::Value& jCity_info)
	{
		m_jCity_info = jCity_info;
		m_mFlag[TbMARCH_ACTION_FIELD_CITY_INFO] = jCity_info.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Kill_troop_num()
	{
		return m_nKill_troop_num;
	}
	void Set_Kill_troop_num(const TINT64& nKill_troop_num, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nKill_troop_num = nKill_troop_num;
		m_mFlag[TbMARCH_ACTION_FIELD_KILL_TROOP_NUM] = dwActionType;
	}
	const TINT64& Get_Kill_fort_num()
	{
		return m_nKill_fort_num;
	}
	void Set_Kill_fort_num(const TINT64& nKill_fort_num, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nKill_fort_num = nKill_fort_num;
		m_mFlag[TbMARCH_ACTION_FIELD_KILL_FORT_NUM] = dwActionType;
	}
	const TINT64& Get_Is_recalled()
	{
		return m_nIs_recalled;
	}
	void Set_Is_recalled(const TINT64& nIs_recalled, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nIs_recalled = nIs_recalled;
		m_mFlag[TbMARCH_ACTION_FIELD_IS_RECALLED] = dwActionType;
	}
	const TINT64& Get_Savatar()
	{
		return m_nSavatar;
	}
	void Set_Savatar(const TINT64& nSavatar, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSavatar = nSavatar;
		m_mFlag[TbMARCH_ACTION_FIELD_SAVATAR] = dwActionType;
	}
	const Json::Value& Get_Tax_info()
	{
		return m_jTax_info;
	}
	void Set_Tax_info(const Json::Value& jTax_info)
	{
		m_jTax_info = jTax_info;
		m_mFlag[TbMARCH_ACTION_FIELD_TAX_INFO] = jTax_info.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Trade_list_idx()
	{
		return m_nTrade_list_idx;
	}
	void Set_Trade_list_idx(const TINT64& nTrade_list_idx, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nTrade_list_idx = nTrade_list_idx;
		m_mFlag[TbMARCH_ACTION_FIELD_TRADE_LIST_IDX] = dwActionType;
	}
	const TINT64& Get_Delay_report_id()
	{
		return m_nDelay_report_id;
	}
	void Set_Delay_report_id(const TINT64& nDelay_report_id, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nDelay_report_id = nDelay_report_id;
		m_mFlag[TbMARCH_ACTION_FIELD_DELAY_REPORT_ID] = dwActionType;
	}
	const TINT64& Get_Seq()
	{
		return m_nSeq;
	}
	void Set_Seq(const TINT64& nSeq, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSeq = nSeq;
		m_mFlag[TbMARCH_ACTION_FIELD_SEQ] = dwActionType;
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

