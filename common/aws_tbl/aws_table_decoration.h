#ifndef _AWS_TABLE_DECORATION_H_
#define _AWS_TABLE_DECORATION_H_

#include "aws_table_common.h"
#include "field_property.h"

#define EN_AWS_TABLE_DECORATION "decoration"

enum ETbDecorationField
{
	TbDECORATION_FIELD_UID = 0,
	TbDECORATION_FIELD_OPEN_TIME = 1,
	TbDECORATION_FIELD_SERIES_LIST = 2,
	TbDECORATION_FIELD_DECORATION_LIST = 3,
	TbDECORATION_FIELD_SEQ = 4,
};

enum ETbDecorationOpenType
{
	ETbDECORATION_OPEN_TYPE_PRIMARY = 0,
};

class TbDecoration : public AwsTable
{
public:
	TINT64 m_nUid;
	TINT64 m_nOpen_time;
	Json::Value m_jSeries_list;
	Json::Value m_jDecoration_list;
	TINT64 m_nSeq;

public:
	static TableDesc oTableDesc;
	Json::Reader oJsonReader;
	Json::FastWriter oJsonWriter;
	static int Init(const string& sConfFile, const string strProjectName);

public:
	TbDecoration():AwsTable(&oTableDesc)
	{
		Reset();
	}
	~TbDecoration()
	{}
	void Reset()
	{
		m_nUid = 0;
		m_nOpen_time = 0;
		m_jSeries_list.clear();
		m_jDecoration_list.clear();
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
	const TINT64& Get_Open_time()
	{
		return m_nOpen_time;
	}
	void Set_Open_time(const TINT64& nOpen_time, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nOpen_time = nOpen_time;
		m_mFlag[TbDECORATION_FIELD_OPEN_TIME] = dwActionType;
	}
	const Json::Value& Get_Series_list()
	{
		return m_jSeries_list;
	}
	void Set_Series_list(const Json::Value& jSeries_list)
	{
		m_jSeries_list = jSeries_list;
		m_mFlag[TbDECORATION_FIELD_SERIES_LIST] = jSeries_list.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const Json::Value& Get_Decoration_list()
	{
		return m_jDecoration_list;
	}
	void Set_Decoration_list(const Json::Value& jDecoration_list)
	{
		m_jDecoration_list = jDecoration_list;
		m_mFlag[TbDECORATION_FIELD_DECORATION_LIST] = jDecoration_list.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Seq()
	{
		return m_nSeq;
	}
	void Set_Seq(const TINT64& nSeq, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSeq = nSeq;
		m_mFlag[TbDECORATION_FIELD_SEQ] = dwActionType;
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

