#ifndef _AWS_TABLE_THRONE_H_
#define _AWS_TABLE_THRONE_H_

#include "aws_table_common.h"
#include "field_property.h"

#define EN_AWS_TABLE_THRONE "throne"

enum ETbThroneField
{
	TbTHRONE_FIELD_SID = 0,
	TbTHRONE_FIELD_ID = 1,
	TbTHRONE_FIELD_POS = 2,
	TbTHRONE_FIELD_ALID = 3,
	TbTHRONE_FIELD_OWNER_ID = 4,
	TbTHRONE_FIELD_INFO = 5,
	TbTHRONE_FIELD_TAX_ID = 6,
	TbTHRONE_FIELD_STATUS = 7,
	TbTHRONE_FIELD_END_TIME = 8,
	TbTHRONE_FIELD_SEQ = 9,
	TbTHRONE_FIELD_DEFENDING_NUM = 10,
	TbTHRONE_FIELD_DEFENDING_TROOP_NUM = 11,
	TbTHRONE_FIELD_DEFENDING_TROOP_FORCE = 12,
	TbTHRONE_FIELD_REINFORCE_NUM = 13,
	TbTHRONE_FIELD_REINFORCE_TROOP_NUM = 14,
	TbTHRONE_FIELD_REINFORCE_TROOP_FORCE = 15,
	TbTHRONE_FIELD_OCCUPY_TIME = 16,
	TbTHRONE_FIELD_OWNER_CID = 17,
	TbTHRONE_FIELD_RANK_INFO = 18,
};

enum ETbThroneOpenType
{
	ETbTHRONE_OPEN_TYPE_PRIMARY = 0,
};

class TbThrone : public AwsTable
{
public:
	TINT64 m_nSid;
	TINT64 m_nId;
	TINT64 m_nPos;
	TINT64 m_nAlid;
	TINT64 m_nOwner_id;
	Json::Value m_jInfo;
	TINT64 m_nTax_id;
	TINT64 m_nStatus;
	TINT64 m_nEnd_time;
	TINT64 m_nSeq;
	TINT64 m_nDefending_num;
	TINT64 m_nDefending_troop_num;
	TINT64 m_nDefending_troop_force;
	TINT64 m_nReinforce_num;
	TINT64 m_nReinforce_troop_num;
	TINT64 m_nReinforce_troop_force;
	TINT64 m_nOccupy_time;
	TINT64 m_nOwner_cid;
	Json::Value m_jRank_info;

public:
	static TableDesc oTableDesc;
	Json::Reader oJsonReader;
	Json::FastWriter oJsonWriter;
	static int Init(const string& sConfFile, const string strProjectName);

public:
	TbThrone():AwsTable(&oTableDesc)
	{
		Reset();
	}
	~TbThrone()
	{}
	void Reset()
	{
		m_nSid = 0;
		m_nId = 0;
		m_nPos = 0;
		m_nAlid = 0;
		m_nOwner_id = 0;
		m_jInfo.clear();
		m_nTax_id = 0;
		m_nStatus = 0;
		m_nEnd_time = 0;
		m_nSeq = 0;
		m_nDefending_num = 0;
		m_nDefending_troop_num = 0;
		m_nDefending_troop_force = 0;
		m_nReinforce_num = 0;
		m_nReinforce_troop_num = 0;
		m_nReinforce_troop_force = 0;
		m_nOccupy_time = 0;
		m_nOwner_cid = 0;
		m_jRank_info.clear();
		ClearFlag();
	};
	const TINT64& Get_Sid()
	{
		return m_nSid;
	}
	void Set_Sid(const TINT64& nSid)
	{
		m_nSid = nSid;
	}
	const TINT64& Get_Id()
	{
		return m_nId;
	}
	void Set_Id(const TINT64& nId)
	{
		m_nId = nId;
	}
	const TINT64& Get_Pos()
	{
		return m_nPos;
	}
	void Set_Pos(const TINT64& nPos, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nPos = nPos;
		m_mFlag[TbTHRONE_FIELD_POS] = dwActionType;
	}
	const TINT64& Get_Alid()
	{
		return m_nAlid;
	}
	void Set_Alid(const TINT64& nAlid, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nAlid = nAlid;
		m_mFlag[TbTHRONE_FIELD_ALID] = dwActionType;
	}
	const TINT64& Get_Owner_id()
	{
		return m_nOwner_id;
	}
	void Set_Owner_id(const TINT64& nOwner_id, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nOwner_id = nOwner_id;
		m_mFlag[TbTHRONE_FIELD_OWNER_ID] = dwActionType;
	}
	const Json::Value& Get_Info()
	{
		return m_jInfo;
	}
	void Set_Info(const Json::Value& jInfo)
	{
		m_jInfo = jInfo;
		m_mFlag[TbTHRONE_FIELD_INFO] = jInfo.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Tax_id()
	{
		return m_nTax_id;
	}
	void Set_Tax_id(const TINT64& nTax_id, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nTax_id = nTax_id;
		m_mFlag[TbTHRONE_FIELD_TAX_ID] = dwActionType;
	}
	const TINT64& Get_Status()
	{
		return m_nStatus;
	}
	void Set_Status(const TINT64& nStatus, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nStatus = nStatus;
		m_mFlag[TbTHRONE_FIELD_STATUS] = dwActionType;
	}
	const TINT64& Get_End_time()
	{
		return m_nEnd_time;
	}
	void Set_End_time(const TINT64& nEnd_time, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nEnd_time = nEnd_time;
		m_mFlag[TbTHRONE_FIELD_END_TIME] = dwActionType;
	}
	const TINT64& Get_Seq()
	{
		return m_nSeq;
	}
	void Set_Seq(const TINT64& nSeq, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSeq = nSeq;
		m_mFlag[TbTHRONE_FIELD_SEQ] = dwActionType;
	}
	const TINT64& Get_Defending_num()
	{
		return m_nDefending_num;
	}
	void Set_Defending_num(const TINT64& nDefending_num, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nDefending_num = nDefending_num;
		m_mFlag[TbTHRONE_FIELD_DEFENDING_NUM] = dwActionType;
	}
	const TINT64& Get_Defending_troop_num()
	{
		return m_nDefending_troop_num;
	}
	void Set_Defending_troop_num(const TINT64& nDefending_troop_num, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nDefending_troop_num = nDefending_troop_num;
		m_mFlag[TbTHRONE_FIELD_DEFENDING_TROOP_NUM] = dwActionType;
	}
	const TINT64& Get_Defending_troop_force()
	{
		return m_nDefending_troop_force;
	}
	void Set_Defending_troop_force(const TINT64& nDefending_troop_force, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nDefending_troop_force = nDefending_troop_force;
		m_mFlag[TbTHRONE_FIELD_DEFENDING_TROOP_FORCE] = dwActionType;
	}
	const TINT64& Get_Reinforce_num()
	{
		return m_nReinforce_num;
	}
	void Set_Reinforce_num(const TINT64& nReinforce_num, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nReinforce_num = nReinforce_num;
		m_mFlag[TbTHRONE_FIELD_REINFORCE_NUM] = dwActionType;
	}
	const TINT64& Get_Reinforce_troop_num()
	{
		return m_nReinforce_troop_num;
	}
	void Set_Reinforce_troop_num(const TINT64& nReinforce_troop_num, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nReinforce_troop_num = nReinforce_troop_num;
		m_mFlag[TbTHRONE_FIELD_REINFORCE_TROOP_NUM] = dwActionType;
	}
	const TINT64& Get_Reinforce_troop_force()
	{
		return m_nReinforce_troop_force;
	}
	void Set_Reinforce_troop_force(const TINT64& nReinforce_troop_force, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nReinforce_troop_force = nReinforce_troop_force;
		m_mFlag[TbTHRONE_FIELD_REINFORCE_TROOP_FORCE] = dwActionType;
	}
	const TINT64& Get_Occupy_time()
	{
		return m_nOccupy_time;
	}
	void Set_Occupy_time(const TINT64& nOccupy_time, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nOccupy_time = nOccupy_time;
		m_mFlag[TbTHRONE_FIELD_OCCUPY_TIME] = dwActionType;
	}
	const TINT64& Get_Owner_cid()
	{
		return m_nOwner_cid;
	}
	void Set_Owner_cid(const TINT64& nOwner_cid, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nOwner_cid = nOwner_cid;
		m_mFlag[TbTHRONE_FIELD_OWNER_CID] = dwActionType;
	}
	const Json::Value& Get_Rank_info()
	{
		return m_jRank_info;
	}
	void Set_Rank_info(const Json::Value& jRank_info)
	{
		m_jRank_info = jRank_info;
		m_mFlag[TbTHRONE_FIELD_RANK_INFO] = jRank_info.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
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

