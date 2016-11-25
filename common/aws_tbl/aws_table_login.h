#ifndef _AWS_TABLE_LOGIN_H_
#define _AWS_TABLE_LOGIN_H_

#include "aws_table_common.h"
#include "field_property.h"

#define EN_AWS_TABLE_LOGIN "login"

enum ETbLoginField
{
	TbLOGIN_FIELD_SID = 0,
	TbLOGIN_FIELD_UID = 1,
	TbLOGIN_FIELD_SEQ = 2,
	TbLOGIN_FIELD_DID = 3,
	TbLOGIN_FIELD_CTIME = 4,
	TbLOGIN_FIELD_UTIME = 5,
	TbLOGIN_FIELD_GEM = 6,
	TbLOGIN_FIELD_GEM_SEQ = 7,
	TbLOGIN_FIELD_GEM_BUY = 8,
	TbLOGIN_FIELD_NPC = 9,
	TbLOGIN_FIELD_DE = 10,
	TbLOGIN_FIELD_APNS_TOKEN = 11,
	TbLOGIN_FIELD_APNS_NUM = 12,
	TbLOGIN_FIELD_AL_TIME = 13,
	TbLOGIN_FIELD_LAST_LG_TIME = 14,
	TbLOGIN_FIELD_IDFA = 15,
	TbLOGIN_FIELD_GEM_COST = 16,
	TbLOGIN_FIELD_GUIDE_FLAG = 17,
	TbLOGIN_FIELD_PLATFORM = 18,
	TbLOGIN_FIELD_LANG = 19,
	TbLOGIN_FIELD_ABILITY = 20,
	TbLOGIN_FIELD_RATING_TIME = 21,
	TbLOGIN_FIELD_WEEK_GEM_RECHARGE = 22,
	TbLOGIN_FIELD_APNS_SWITCH = 23,
	TbLOGIN_FIELD_MAX_BUY = 24,
	TbLOGIN_FIELD_LAST_BUY = 25,
	TbLOGIN_FIELD_LAST_BUY_TIME = 26,
	TbLOGIN_FIELD_TOTAL_PAY = 27,
	TbLOGIN_FIELD_MAX_PAY = 28,
	TbLOGIN_FIELD_LAST_PAY = 29,
};

enum ETbLoginOpenType
{
	ETbLOGIN_OPEN_TYPE_PRIMARY = 0,
};

#define	TBLOGIN_GUIDE_FLAG_MAX_NUM	1
typedef	BinBuf<SBitFlag, TBLOGIN_GUIDE_FLAG_MAX_NUM>		TbLogin_Guide_flag;

class TbLogin : public AwsTable
{
public:
	TINT64 m_nSid;
	TINT64 m_nUid;
	TINT64 m_nSeq;
	TINT64 m_nDid;
	TINT64 m_nCtime;
	TINT64 m_nUtime;
	TINT64 m_nGem;
	TINT64 m_nGem_seq;
	TINT64 m_nGem_buy;
	TINT64 m_nNpc;
	string m_sDe;
	string m_sApns_token;
	TINT64 m_nApns_num;
	TINT64 m_nAl_time;
	TINT64 m_nLast_lg_time;
	string m_sIdfa;
	TINT64 m_nGem_cost;
	TbLogin_Guide_flag m_bGuide_flag;
	string m_sPlatform;
	TINT64 m_nLang;
	TINT64 m_nAbility;
	TINT64 m_nRating_time;
	Json::Value m_jWeek_gem_recharge;
	Json::Value m_jApns_switch;
	TINT64 m_nMax_buy;
	TINT64 m_nLast_buy;
	TINT64 m_nLast_buy_time;
	TINT64 m_nTotal_pay;
	TINT64 m_nMax_pay;
	TINT64 m_nLast_pay;

public:
	static TableDesc oTableDesc;
	Json::Reader oJsonReader;
	Json::FastWriter oJsonWriter;
	static int Init(const string& sConfFile, const string strProjectName);

public:
	TbLogin():AwsTable(&oTableDesc)
	{
		Reset();
	}
	~TbLogin()
	{}
	void Reset()
	{
		m_nSid = 0;
		m_nUid = 0;
		m_nSeq = 0;
		m_nDid = 0;
		m_nCtime = 0;
		m_nUtime = 0;
		m_nGem = 0;
		m_nGem_seq = 0;
		m_nGem_buy = 0;
		m_nNpc = 0;
		m_sDe = "";
		m_sApns_token = "";
		m_nApns_num = 0;
		m_nAl_time = 0;
		m_nLast_lg_time = 0;
		m_sIdfa = "";
		m_nGem_cost = 0;
		m_bGuide_flag.Reset();
		m_sPlatform = "";
		m_nLang = 0;
		m_nAbility = 0;
		m_nRating_time = 0;
		m_jWeek_gem_recharge.clear();
		m_jApns_switch.clear();
		m_nMax_buy = 0;
		m_nLast_buy = 0;
		m_nLast_buy_time = 0;
		m_nTotal_pay = 0;
		m_nMax_pay = 0;
		m_nLast_pay = 0;
		ClearFlag();
	};
	const TINT64& Get_Sid()
	{
		return m_nSid;
	}
	void Set_Sid(const TINT64& nSid, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSid = nSid;
		m_mFlag[TbLOGIN_FIELD_SID] = dwActionType;
	}
	const TINT64& Get_Uid()
	{
		return m_nUid;
	}
	void Set_Uid(const TINT64& nUid)
	{
		m_nUid = nUid;
	}
	const TINT64& Get_Seq()
	{
		return m_nSeq;
	}
	void Set_Seq(const TINT64& nSeq, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSeq = nSeq;
		m_mFlag[TbLOGIN_FIELD_SEQ] = dwActionType;
	}
	const TINT64& Get_Did()
	{
		return m_nDid;
	}
	void Set_Did(const TINT64& nDid, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nDid = nDid;
		m_mFlag[TbLOGIN_FIELD_DID] = dwActionType;
	}
	const TINT64& Get_Ctime()
	{
		return m_nCtime;
	}
	void Set_Ctime(const TINT64& nCtime, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nCtime = nCtime;
		m_mFlag[TbLOGIN_FIELD_CTIME] = dwActionType;
	}
	const TINT64& Get_Utime()
	{
		return m_nUtime;
	}
	void Set_Utime(const TINT64& nUtime, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nUtime = nUtime;
		m_mFlag[TbLOGIN_FIELD_UTIME] = dwActionType;
	}
	const TINT64& Get_Gem()
	{
		return m_nGem;
	}
	void Set_Gem(const TINT64& nGem, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nGem = nGem;
		m_mFlag[TbLOGIN_FIELD_GEM] = dwActionType;
	}
	const TINT64& Get_Gem_seq()
	{
		return m_nGem_seq;
	}
	void Set_Gem_seq(const TINT64& nGem_seq, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nGem_seq = nGem_seq;
		m_mFlag[TbLOGIN_FIELD_GEM_SEQ] = dwActionType;
	}
	const TINT64& Get_Gem_buy()
	{
		return m_nGem_buy;
	}
	void Set_Gem_buy(const TINT64& nGem_buy, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nGem_buy = nGem_buy;
		m_mFlag[TbLOGIN_FIELD_GEM_BUY] = dwActionType;
	}
	const TINT64& Get_Npc()
	{
		return m_nNpc;
	}
	void Set_Npc(const TINT64& nNpc, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nNpc = nNpc;
		m_mFlag[TbLOGIN_FIELD_NPC] = dwActionType;
	}
	const string& Get_De()
	{
		return m_sDe;
	}
	void Set_De(const string& sDe)
	{
		m_sDe = sDe;
		m_mFlag[TbLOGIN_FIELD_DE] = sDe.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const string& Get_Apns_token()
	{
		return m_sApns_token;
	}
	void Set_Apns_token(const string& sApns_token)
	{
		m_sApns_token = sApns_token;
		m_mFlag[TbLOGIN_FIELD_APNS_TOKEN] = sApns_token.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Apns_num()
	{
		return m_nApns_num;
	}
	void Set_Apns_num(const TINT64& nApns_num, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nApns_num = nApns_num;
		m_mFlag[TbLOGIN_FIELD_APNS_NUM] = dwActionType;
	}
	const TINT64& Get_Al_time()
	{
		return m_nAl_time;
	}
	void Set_Al_time(const TINT64& nAl_time, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nAl_time = nAl_time;
		m_mFlag[TbLOGIN_FIELD_AL_TIME] = dwActionType;
	}
	const TINT64& Get_Last_lg_time()
	{
		return m_nLast_lg_time;
	}
	void Set_Last_lg_time(const TINT64& nLast_lg_time, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nLast_lg_time = nLast_lg_time;
		m_mFlag[TbLOGIN_FIELD_LAST_LG_TIME] = dwActionType;
	}
	const string& Get_Idfa()
	{
		return m_sIdfa;
	}
	void Set_Idfa(const string& sIdfa)
	{
		m_sIdfa = sIdfa;
		m_mFlag[TbLOGIN_FIELD_IDFA] = sIdfa.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Gem_cost()
	{
		return m_nGem_cost;
	}
	void Set_Gem_cost(const TINT64& nGem_cost, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nGem_cost = nGem_cost;
		m_mFlag[TbLOGIN_FIELD_GEM_COST] = dwActionType;
	}
	const string& Get_Platform()
	{
		return m_sPlatform;
	}
	void Set_Platform(const string& sPlatform)
	{
		m_sPlatform = sPlatform;
		m_mFlag[TbLOGIN_FIELD_PLATFORM] = sPlatform.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Lang()
	{
		return m_nLang;
	}
	void Set_Lang(const TINT64& nLang, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nLang = nLang;
		m_mFlag[TbLOGIN_FIELD_LANG] = dwActionType;
	}
	const TINT64& Get_Ability()
	{
		return m_nAbility;
	}
	void Set_Ability(const TINT64& nAbility, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nAbility = nAbility;
		m_mFlag[TbLOGIN_FIELD_ABILITY] = dwActionType;
	}
	const TINT64& Get_Rating_time()
	{
		return m_nRating_time;
	}
	void Set_Rating_time(const TINT64& nRating_time, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nRating_time = nRating_time;
		m_mFlag[TbLOGIN_FIELD_RATING_TIME] = dwActionType;
	}
	const Json::Value& Get_Week_gem_recharge()
	{
		return m_jWeek_gem_recharge;
	}
	void Set_Week_gem_recharge(const Json::Value& jWeek_gem_recharge)
	{
		m_jWeek_gem_recharge = jWeek_gem_recharge;
		m_mFlag[TbLOGIN_FIELD_WEEK_GEM_RECHARGE] = jWeek_gem_recharge.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const Json::Value& Get_Apns_switch()
	{
		return m_jApns_switch;
	}
	void Set_Apns_switch(const Json::Value& jApns_switch)
	{
		m_jApns_switch = jApns_switch;
		m_mFlag[TbLOGIN_FIELD_APNS_SWITCH] = jApns_switch.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Max_buy()
	{
		return m_nMax_buy;
	}
	void Set_Max_buy(const TINT64& nMax_buy, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nMax_buy = nMax_buy;
		m_mFlag[TbLOGIN_FIELD_MAX_BUY] = dwActionType;
	}
	const TINT64& Get_Last_buy()
	{
		return m_nLast_buy;
	}
	void Set_Last_buy(const TINT64& nLast_buy, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nLast_buy = nLast_buy;
		m_mFlag[TbLOGIN_FIELD_LAST_BUY] = dwActionType;
	}
	const TINT64& Get_Last_buy_time()
	{
		return m_nLast_buy_time;
	}
	void Set_Last_buy_time(const TINT64& nLast_buy_time, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nLast_buy_time = nLast_buy_time;
		m_mFlag[TbLOGIN_FIELD_LAST_BUY_TIME] = dwActionType;
	}
	const TINT64& Get_Total_pay()
	{
		return m_nTotal_pay;
	}
	void Set_Total_pay(const TINT64& nTotal_pay, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nTotal_pay = nTotal_pay;
		m_mFlag[TbLOGIN_FIELD_TOTAL_PAY] = dwActionType;
	}
	const TINT64& Get_Max_pay()
	{
		return m_nMax_pay;
	}
	void Set_Max_pay(const TINT64& nMax_pay, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nMax_pay = nMax_pay;
		m_mFlag[TbLOGIN_FIELD_MAX_PAY] = dwActionType;
	}
	const TINT64& Get_Last_pay()
	{
		return m_nLast_pay;
	}
	void Set_Last_pay(const TINT64& nLast_pay, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nLast_pay = nLast_pay;
		m_mFlag[TbLOGIN_FIELD_LAST_PAY] = dwActionType;
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

