#ifndef _AWS_TABLE_REWARD_WINDOW_H_
#define _AWS_TABLE_REWARD_WINDOW_H_

#include "aws_table_common.h"
#include "field_property.h"

#define EN_AWS_TABLE_REWARD_WINDOW "reward_window"

enum ETbReward_windowField
{
	TbREWARD_WINDOW_FIELD_UID = 0,
	TbREWARD_WINDOW_FIELD_ID = 1,
	TbREWARD_WINDOW_FIELD_TYPE = 2,
	TbREWARD_WINDOW_FIELD_REWARD = 3,
	TbREWARD_WINDOW_FIELD_INFO = 4,
	TbREWARD_WINDOW_FIELD_SHOW_TIME = 5,
	TbREWARD_WINDOW_FIELD_GET_TYPE = 6,
	TbREWARD_WINDOW_FIELD_SEQ = 7,
};

enum ETbReward_windowOpenType
{
	ETbREWARDWINDOW_OPEN_TYPE_PRIMARY = 0,
};

class TbReward_window : public AwsTable
{
public:
	TINT64 m_nUid;
	TINT64 m_nId;
	TINT64 m_nType;
	Json::Value m_jReward;
	Json::Value m_jInfo;
	TINT64 m_nShow_time;
	TINT64 m_nGet_type;
	TINT64 m_nSeq;

public:
	static TableDesc oTableDesc;
	Json::Reader oJsonReader;
	Json::FastWriter oJsonWriter;
	static int Init(const string& sConfFile, const string strProjectName);

public:
	TbReward_window():AwsTable(&oTableDesc)
	{
		Reset();
	}
	~TbReward_window()
	{}
	void Reset()
	{
		m_nUid = 0;
		m_nId = 0;
		m_nType = 0;
		m_jReward.clear();
		m_jInfo.clear();
		m_nShow_time = 0;
		m_nGet_type = 0;
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
	const TINT64& Get_Id()
	{
		return m_nId;
	}
	void Set_Id(const TINT64& nId)
	{
		m_nId = nId;
	}
	const TINT64& Get_Type()
	{
		return m_nType;
	}
	void Set_Type(const TINT64& nType, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nType = nType;
		m_mFlag[TbREWARD_WINDOW_FIELD_TYPE] = dwActionType;
	}
	const Json::Value& Get_Reward()
	{
		return m_jReward;
	}
	void Set_Reward(const Json::Value& jReward)
	{
		m_jReward = jReward;
		m_mFlag[TbREWARD_WINDOW_FIELD_REWARD] = jReward.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const Json::Value& Get_Info()
	{
		return m_jInfo;
	}
	void Set_Info(const Json::Value& jInfo)
	{
		m_jInfo = jInfo;
		m_mFlag[TbREWARD_WINDOW_FIELD_INFO] = jInfo.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Show_time()
	{
		return m_nShow_time;
	}
	void Set_Show_time(const TINT64& nShow_time, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nShow_time = nShow_time;
		m_mFlag[TbREWARD_WINDOW_FIELD_SHOW_TIME] = dwActionType;
	}
	const TINT64& Get_Get_type()
	{
		return m_nGet_type;
	}
	void Set_Get_type(const TINT64& nGet_type, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nGet_type = nGet_type;
		m_mFlag[TbREWARD_WINDOW_FIELD_GET_TYPE] = dwActionType;
	}
	const TINT64& Get_Seq()
	{
		return m_nSeq;
	}
	void Set_Seq(const TINT64& nSeq, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSeq = nSeq;
		m_mFlag[TbREWARD_WINDOW_FIELD_SEQ] = dwActionType;
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

