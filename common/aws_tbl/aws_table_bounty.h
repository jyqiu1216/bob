#ifndef _AWS_TABLE_BOUNTY_H_
#define _AWS_TABLE_BOUNTY_H_

#include "aws_table_common.h"
#include "field_property.h"

#define EN_AWS_TABLE_BOUNTY "bounty"

enum ETbBountyField
{
	TbBOUNTY_FIELD_UID = 0,
	TbBOUNTY_FIELD_NEXT_REFRESH_TIME = 1,
	TbBOUNTY_FIELD_STAR = 2,
	TbBOUNTY_FIELD_S_STAGE = 3,
	TbBOUNTY_FIELD_S_REWARD = 4,
	TbBOUNTY_FIELD_BASE = 5,
	TbBOUNTY_FIELD_SEQ = 6,
};

enum ETbBountyOpenType
{
	ETbBOUNTY_OPEN_TYPE_PRIMARY = 0,
};

class TbBounty : public AwsTable
{
public:
	TINT64 m_nUid;
	TINT64 m_nNext_refresh_time;
	TINT64 m_nStar;
	Json::Value m_jS_stage;
	Json::Value m_jS_reward;
	Json::Value m_jBase;
	TINT64 m_nSeq;

public:
	static TableDesc oTableDesc;
	Json::Reader oJsonReader;
	Json::FastWriter oJsonWriter;
	static int Init(const string& sConfFile, const string strProjectName);

public:
	TbBounty():AwsTable(&oTableDesc)
	{
		Reset();
	}
	~TbBounty()
	{}
	void Reset()
	{
		m_nUid = 0;
		m_nNext_refresh_time = 0;
		m_nStar = 0;
		m_jS_stage.clear();
		m_jS_reward.clear();
		m_jBase.clear();
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
	const TINT64& Get_Next_refresh_time()
	{
		return m_nNext_refresh_time;
	}
	void Set_Next_refresh_time(const TINT64& nNext_refresh_time, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nNext_refresh_time = nNext_refresh_time;
		m_mFlag[TbBOUNTY_FIELD_NEXT_REFRESH_TIME] = dwActionType;
	}
	const TINT64& Get_Star()
	{
		return m_nStar;
	}
	void Set_Star(const TINT64& nStar, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nStar = nStar;
		m_mFlag[TbBOUNTY_FIELD_STAR] = dwActionType;
	}
	const Json::Value& Get_S_stage()
	{
		return m_jS_stage;
	}
	void Set_S_stage(const Json::Value& jS_stage)
	{
		m_jS_stage = jS_stage;
		m_mFlag[TbBOUNTY_FIELD_S_STAGE] = jS_stage.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const Json::Value& Get_S_reward()
	{
		return m_jS_reward;
	}
	void Set_S_reward(const Json::Value& jS_reward)
	{
		m_jS_reward = jS_reward;
		m_mFlag[TbBOUNTY_FIELD_S_REWARD] = jS_reward.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const Json::Value& Get_Base()
	{
		return m_jBase;
	}
	void Set_Base(const Json::Value& jBase)
	{
		m_jBase = jBase;
		m_mFlag[TbBOUNTY_FIELD_BASE] = jBase.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Seq()
	{
		return m_nSeq;
	}
	void Set_Seq(const TINT64& nSeq, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSeq = nSeq;
		m_mFlag[TbBOUNTY_FIELD_SEQ] = dwActionType;
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

