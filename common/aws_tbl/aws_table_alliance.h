#ifndef _AWS_TABLE_ALLIANCE_H_
#define _AWS_TABLE_ALLIANCE_H_

#include "aws_table_common.h"
#include "field_property.h"

#define EN_AWS_TABLE_ALLIANCE "alliance"

enum ETbAllianceField
{
	TbALLIANCE_FIELD_AID = 0,
	TbALLIANCE_FIELD_NAME = 1,
	TbALLIANCE_FIELD_OID = 2,
	TbALLIANCE_FIELD_ONAME = 3,
	TbALLIANCE_FIELD_MEMBER = 4,
	TbALLIANCE_FIELD_MIGHT = 5,
	TbALLIANCE_FIELD_DESC = 6,
	TbALLIANCE_FIELD_NOTICE = 7,
	TbALLIANCE_FIELD_SID = 8,
	TbALLIANCE_FIELD_POLICY = 9,
	TbALLIANCE_FIELD_GIFT_POINT = 10,
	TbALLIANCE_FIELD_LANGUAGE = 11,
	TbALLIANCE_FIELD_FUND = 12,
	TbALLIANCE_FIELD_AL_STORE_ITEM = 13,
	TbALLIANCE_FIELD_FORCE_KILL = 14,
	TbALLIANCE_FIELD_AVATAR = 15,
	TbALLIANCE_FIELD_AL_NICK_NAME = 16,
	TbALLIANCE_FIELD_THRONE_POS = 17,
	TbALLIANCE_FIELD_THRONE_STATUS = 18,
	TbALLIANCE_FIELD_IS_NPC = 19,
	TbALLIANCE_FIELD_MAX_GEM_BUY = 20,
	TbALLIANCE_FIELD_AL_STAR = 21,
	TbALLIANCE_FIELD_HIVE_POS = 22,
	TbALLIANCE_FIELD_HIVE_SHOW_FLAG = 23,
	TbALLIANCE_FIELD_HIVE_SVR = 24,
	TbALLIANCE_FIELD_SEQ = 25,
	TbALLIANCE_FIELD_HAS_OCCUPIED_TIME = 26,
	TbALLIANCE_FIELD_LAST_OCCUPY_TIME = 27,
	TbALLIANCE_FIELD_OWNER_CID = 28,
};

enum ETbAllianceOpenType
{
	ETbALLIANCE_OPEN_TYPE_PRIMARY = 0,
};

#define	TBALLIANCE_AL_STORE_ITEM_MAX_NUM	100
typedef	BinBuf<SAlStoreItem, TBALLIANCE_AL_STORE_ITEM_MAX_NUM>		TbAlliance_Al_store_item;

class TbAlliance : public AwsTable
{
public:
	TINT64 m_nAid;
	string m_sName;
	TINT64 m_nOid;
	string m_sOname;
	TINT64 m_nMember;
	TINT64 m_nMight;
	string m_sDesc;
	string m_sNotice;
	TINT64 m_nSid;
	TINT64 m_nPolicy;
	TINT64 m_nGift_point;
	TINT64 m_nLanguage;
	TINT64 m_nFund;
	TbAlliance_Al_store_item m_bAl_store_item;
	TINT64 m_nForce_kill;
	TINT64 m_nAvatar;
	string m_sAl_nick_name;
	TINT64 m_nThrone_pos;
	TINT64 m_nThrone_status;
	TINT64 m_nIs_npc;
	TINT64 m_nMax_gem_buy;
	TINT64 m_nAl_star;
	TINT64 m_nHive_pos;
	TINT64 m_nHive_show_flag;
	TINT64 m_nHive_svr;
	TINT64 m_nSeq;
	TINT64 m_nHas_occupied_time;
	TINT64 m_nLast_occupy_time;
	TINT64 m_nOwner_cid;

public:
	static TableDesc oTableDesc;
	Json::Reader oJsonReader;
	Json::FastWriter oJsonWriter;
	static int Init(const string& sConfFile, const string strProjectName);

public:
	TbAlliance():AwsTable(&oTableDesc)
	{
		Reset();
	}
	~TbAlliance()
	{}
	void Reset()
	{
		m_nAid = 0;
		m_sName = "";
		m_nOid = 0;
		m_sOname = "";
		m_nMember = 0;
		m_nMight = 0;
		m_sDesc = "";
		m_sNotice = "";
		m_nSid = 0;
		m_nPolicy = 0;
		m_nGift_point = 0;
		m_nLanguage = 0;
		m_nFund = 0;
		m_bAl_store_item.Reset();
		m_nForce_kill = 0;
		m_nAvatar = 0;
		m_sAl_nick_name = "";
		m_nThrone_pos = 0;
		m_nThrone_status = 0;
		m_nIs_npc = 0;
		m_nMax_gem_buy = 0;
		m_nAl_star = 0;
		m_nHive_pos = 0;
		m_nHive_show_flag = 0;
		m_nHive_svr = 0;
		m_nSeq = 0;
		m_nHas_occupied_time = 0;
		m_nLast_occupy_time = 0;
		m_nOwner_cid = 0;
		ClearFlag();
	};
	const TINT64& Get_Aid()
	{
		return m_nAid;
	}
	void Set_Aid(const TINT64& nAid)
	{
		m_nAid = nAid;
	}
	const string& Get_Name()
	{
		return m_sName;
	}
	void Set_Name(const string& sName)
	{
		m_sName = sName;
		m_mFlag[TbALLIANCE_FIELD_NAME] = sName.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Oid()
	{
		return m_nOid;
	}
	void Set_Oid(const TINT64& nOid, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nOid = nOid;
		m_mFlag[TbALLIANCE_FIELD_OID] = dwActionType;
	}
	const string& Get_Oname()
	{
		return m_sOname;
	}
	void Set_Oname(const string& sOname)
	{
		m_sOname = sOname;
		m_mFlag[TbALLIANCE_FIELD_ONAME] = sOname.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Member()
	{
		return m_nMember;
	}
	void Set_Member(const TINT64& nMember, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nMember = nMember;
		m_mFlag[TbALLIANCE_FIELD_MEMBER] = dwActionType;
	}
	const TINT64& Get_Might()
	{
		return m_nMight;
	}
	void Set_Might(const TINT64& nMight, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nMight = nMight;
		m_mFlag[TbALLIANCE_FIELD_MIGHT] = dwActionType;
	}
	const string& Get_Desc()
	{
		return m_sDesc;
	}
	void Set_Desc(const string& sDesc)
	{
		m_sDesc = sDesc;
		m_mFlag[TbALLIANCE_FIELD_DESC] = sDesc.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const string& Get_Notice()
	{
		return m_sNotice;
	}
	void Set_Notice(const string& sNotice)
	{
		m_sNotice = sNotice;
		m_mFlag[TbALLIANCE_FIELD_NOTICE] = sNotice.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Sid()
	{
		return m_nSid;
	}
	void Set_Sid(const TINT64& nSid, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSid = nSid;
		m_mFlag[TbALLIANCE_FIELD_SID] = dwActionType;
	}
	const TINT64& Get_Policy()
	{
		return m_nPolicy;
	}
	void Set_Policy(const TINT64& nPolicy, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nPolicy = nPolicy;
		m_mFlag[TbALLIANCE_FIELD_POLICY] = dwActionType;
	}
	const TINT64& Get_Gift_point()
	{
		return m_nGift_point;
	}
	void Set_Gift_point(const TINT64& nGift_point, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nGift_point = nGift_point;
		m_mFlag[TbALLIANCE_FIELD_GIFT_POINT] = dwActionType;
	}
	const TINT64& Get_Language()
	{
		return m_nLanguage;
	}
	void Set_Language(const TINT64& nLanguage, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nLanguage = nLanguage;
		m_mFlag[TbALLIANCE_FIELD_LANGUAGE] = dwActionType;
	}
	const TINT64& Get_Fund()
	{
		return m_nFund;
	}
	void Set_Fund(const TINT64& nFund, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nFund = nFund;
		m_mFlag[TbALLIANCE_FIELD_FUND] = dwActionType;
	}
	const TINT64& Get_Force_kill()
	{
		return m_nForce_kill;
	}
	void Set_Force_kill(const TINT64& nForce_kill, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nForce_kill = nForce_kill;
		m_mFlag[TbALLIANCE_FIELD_FORCE_KILL] = dwActionType;
	}
	const TINT64& Get_Avatar()
	{
		return m_nAvatar;
	}
	void Set_Avatar(const TINT64& nAvatar, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nAvatar = nAvatar;
		m_mFlag[TbALLIANCE_FIELD_AVATAR] = dwActionType;
	}
	const string& Get_Al_nick_name()
	{
		return m_sAl_nick_name;
	}
	void Set_Al_nick_name(const string& sAl_nick_name)
	{
		m_sAl_nick_name = sAl_nick_name;
		m_mFlag[TbALLIANCE_FIELD_AL_NICK_NAME] = sAl_nick_name.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Throne_pos()
	{
		return m_nThrone_pos;
	}
	void Set_Throne_pos(const TINT64& nThrone_pos, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nThrone_pos = nThrone_pos;
		m_mFlag[TbALLIANCE_FIELD_THRONE_POS] = dwActionType;
	}
	const TINT64& Get_Throne_status()
	{
		return m_nThrone_status;
	}
	void Set_Throne_status(const TINT64& nThrone_status, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nThrone_status = nThrone_status;
		m_mFlag[TbALLIANCE_FIELD_THRONE_STATUS] = dwActionType;
	}
	const TINT64& Get_Is_npc()
	{
		return m_nIs_npc;
	}
	void Set_Is_npc(const TINT64& nIs_npc, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nIs_npc = nIs_npc;
		m_mFlag[TbALLIANCE_FIELD_IS_NPC] = dwActionType;
	}
	const TINT64& Get_Max_gem_buy()
	{
		return m_nMax_gem_buy;
	}
	void Set_Max_gem_buy(const TINT64& nMax_gem_buy, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nMax_gem_buy = nMax_gem_buy;
		m_mFlag[TbALLIANCE_FIELD_MAX_GEM_BUY] = dwActionType;
	}
	const TINT64& Get_Al_star()
	{
		return m_nAl_star;
	}
	void Set_Al_star(const TINT64& nAl_star, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nAl_star = nAl_star;
		m_mFlag[TbALLIANCE_FIELD_AL_STAR] = dwActionType;
	}
	const TINT64& Get_Hive_pos()
	{
		return m_nHive_pos;
	}
	void Set_Hive_pos(const TINT64& nHive_pos, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nHive_pos = nHive_pos;
		m_mFlag[TbALLIANCE_FIELD_HIVE_POS] = dwActionType;
	}
	const TINT64& Get_Hive_show_flag()
	{
		return m_nHive_show_flag;
	}
	void Set_Hive_show_flag(const TINT64& nHive_show_flag, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nHive_show_flag = nHive_show_flag;
		m_mFlag[TbALLIANCE_FIELD_HIVE_SHOW_FLAG] = dwActionType;
	}
	const TINT64& Get_Hive_svr()
	{
		return m_nHive_svr;
	}
	void Set_Hive_svr(const TINT64& nHive_svr, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nHive_svr = nHive_svr;
		m_mFlag[TbALLIANCE_FIELD_HIVE_SVR] = dwActionType;
	}
	const TINT64& Get_Seq()
	{
		return m_nSeq;
	}
	void Set_Seq(const TINT64& nSeq, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSeq = nSeq;
		m_mFlag[TbALLIANCE_FIELD_SEQ] = dwActionType;
	}
	const TINT64& Get_Has_occupied_time()
	{
		return m_nHas_occupied_time;
	}
	void Set_Has_occupied_time(const TINT64& nHas_occupied_time, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nHas_occupied_time = nHas_occupied_time;
		m_mFlag[TbALLIANCE_FIELD_HAS_OCCUPIED_TIME] = dwActionType;
	}
	const TINT64& Get_Last_occupy_time()
	{
		return m_nLast_occupy_time;
	}
	void Set_Last_occupy_time(const TINT64& nLast_occupy_time, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nLast_occupy_time = nLast_occupy_time;
		m_mFlag[TbALLIANCE_FIELD_LAST_OCCUPY_TIME] = dwActionType;
	}
	const TINT64& Get_Owner_cid()
	{
		return m_nOwner_cid;
	}
	void Set_Owner_cid(const TINT64& nOwner_cid, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nOwner_cid = nOwner_cid;
		m_mFlag[TbALLIANCE_FIELD_OWNER_CID] = dwActionType;
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

