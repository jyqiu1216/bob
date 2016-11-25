#ifndef _AWS_TABLE_BROADCAST_H_
#define _AWS_TABLE_BROADCAST_H_

#include "aws_table_common.h"
#include "field_property.h"

#define EN_AWS_TABLE_BROADCAST "broadcast"

enum ETbBroadcastField
{
	TbBROADCAST_FIELD_KEY = 0,
	TbBROADCAST_FIELD_CTIME = 1,
	TbBROADCAST_FIELD_TYPE = 2,
	TbBROADCAST_FIELD_CID = 3,
	TbBROADCAST_FIELD_REPLACE_DATA = 4,
	TbBROADCAST_FIELD_PARAM = 5,
	TbBROADCAST_FIELD_STRATEGY = 6,
	TbBROADCAST_FIELD_PRIORITY = 7,
	TbBROADCAST_FIELD_SEQ = 8,
};

enum ETbBroadcastOpenType
{
	ETbBROADCAST_OPEN_TYPE_PRIMARY = 0,
};

class TbBroadcast : public AwsTable
{
public:
	TINT64 m_nKey;
	TINT64 m_nCtime;
	TINT64 m_nType;
	TINT64 m_nCid;
	string m_sReplace_data;
	string m_sParam;
	TINT64 m_nStrategy;
	TINT64 m_nPriority;
	TINT64 m_nSeq;

public:
	static TableDesc oTableDesc;
	Json::Reader oJsonReader;
	Json::FastWriter oJsonWriter;
	static int Init(const string& sConfFile, const string strProjectName);

public:
	TbBroadcast():AwsTable(&oTableDesc)
	{
		Reset();
	}
	~TbBroadcast()
	{}
	void Reset()
	{
		m_nKey = 0;
		m_nCtime = 0;
		m_nType = 0;
		m_nCid = 0;
		m_sReplace_data = "";
		m_sParam = "";
		m_nStrategy = 0;
		m_nPriority = 0;
		m_nSeq = 0;
		ClearFlag();
	};
	const TINT64& Get_Key()
	{
		return m_nKey;
	}
	void Set_Key(const TINT64& nKey)
	{
		m_nKey = nKey;
	}
	const TINT64& Get_Ctime()
	{
		return m_nCtime;
	}
	void Set_Ctime(const TINT64& nCtime)
	{
		m_nCtime = nCtime;
	}
	const TINT64& Get_Type()
	{
		return m_nType;
	}
	void Set_Type(const TINT64& nType, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nType = nType;
		m_mFlag[TbBROADCAST_FIELD_TYPE] = dwActionType;
	}
	const TINT64& Get_Cid()
	{
		return m_nCid;
	}
	void Set_Cid(const TINT64& nCid, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nCid = nCid;
		m_mFlag[TbBROADCAST_FIELD_CID] = dwActionType;
	}
	const string& Get_Replace_data()
	{
		return m_sReplace_data;
	}
	void Set_Replace_data(const string& sReplace_data)
	{
		m_sReplace_data = sReplace_data;
		m_mFlag[TbBROADCAST_FIELD_REPLACE_DATA] = sReplace_data.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const string& Get_Param()
	{
		return m_sParam;
	}
	void Set_Param(const string& sParam)
	{
		m_sParam = sParam;
		m_mFlag[TbBROADCAST_FIELD_PARAM] = sParam.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Strategy()
	{
		return m_nStrategy;
	}
	void Set_Strategy(const TINT64& nStrategy, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nStrategy = nStrategy;
		m_mFlag[TbBROADCAST_FIELD_STRATEGY] = dwActionType;
	}
	const TINT64& Get_Priority()
	{
		return m_nPriority;
	}
	void Set_Priority(const TINT64& nPriority, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nPriority = nPriority;
		m_mFlag[TbBROADCAST_FIELD_PRIORITY] = dwActionType;
	}
	const TINT64& Get_Seq()
	{
		return m_nSeq;
	}
	void Set_Seq(const TINT64& nSeq, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSeq = nSeq;
		m_mFlag[TbBROADCAST_FIELD_SEQ] = dwActionType;
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

