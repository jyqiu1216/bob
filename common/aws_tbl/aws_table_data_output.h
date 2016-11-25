#ifndef _AWS_TABLE_DATA_OUTPUT_H_
#define _AWS_TABLE_DATA_OUTPUT_H_

#include "aws_table_common.h"
#include "field_property.h"

#define EN_AWS_TABLE_DATA_OUTPUT "data_output"

enum ETbData_outputField
{
	TbDATA_OUTPUT_FIELD_UID = 0,
	TbDATA_OUTPUT_FIELD_UTYPE = 1,
	TbDATA_OUTPUT_FIELD_T_ALL = 2,
	TbDATA_OUTPUT_FIELD_T_INC = 3,
	TbDATA_OUTPUT_FIELD_DATA = 4,
	TbDATA_OUTPUT_FIELD_SEQ = 5,
};

enum ETbData_outputOpenType
{
	ETbDATAOUTPUT_OPEN_TYPE_PRIMARY = 0,
};

class TbData_output : public AwsTable
{
public:
	TINT64 m_nUid;
	TINT64 m_nUtype;
	TINT64 m_nT_all;
	TINT64 m_nT_inc;
	Json::Value m_jData;
	TINT64 m_nSeq;

public:
	static TableDesc oTableDesc;
	Json::Reader oJsonReader;
	Json::FastWriter oJsonWriter;
	static int Init(const string& sConfFile, const string strProjectName);

public:
	TbData_output():AwsTable(&oTableDesc)
	{
		Reset();
	}
	~TbData_output()
	{}
	void Reset()
	{
		m_nUid = 0;
		m_nUtype = 0;
		m_nT_all = 0;
		m_nT_inc = 0;
		m_jData.clear();
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
	const TINT64& Get_Utype()
	{
		return m_nUtype;
	}
	void Set_Utype(const TINT64& nUtype, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nUtype = nUtype;
		m_mFlag[TbDATA_OUTPUT_FIELD_UTYPE] = dwActionType;
	}
	const TINT64& Get_T_all()
	{
		return m_nT_all;
	}
	void Set_T_all(const TINT64& nT_all, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nT_all = nT_all;
		m_mFlag[TbDATA_OUTPUT_FIELD_T_ALL] = dwActionType;
	}
	const TINT64& Get_T_inc()
	{
		return m_nT_inc;
	}
	void Set_T_inc(const TINT64& nT_inc, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nT_inc = nT_inc;
		m_mFlag[TbDATA_OUTPUT_FIELD_T_INC] = dwActionType;
	}
	const Json::Value& Get_Data()
	{
		return m_jData;
	}
	void Set_Data(const Json::Value& jData)
	{
		m_jData = jData;
		m_mFlag[TbDATA_OUTPUT_FIELD_DATA] = jData.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Seq()
	{
		return m_nSeq;
	}
	void Set_Seq(const TINT64& nSeq, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSeq = nSeq;
		m_mFlag[TbDATA_OUTPUT_FIELD_SEQ] = dwActionType;
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

