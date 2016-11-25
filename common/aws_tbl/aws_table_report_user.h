#ifndef _AWS_TABLE_REPORT_USER_H_
#define _AWS_TABLE_REPORT_USER_H_

#include "aws_table_common.h"
#include "field_property.h"

#define EN_AWS_TABLE_REPORT_USER "report_user"

enum ETbReport_userField
{
	TbREPORT_USER_FIELD_UID = 0,
	TbREPORT_USER_FIELD_RID = 1,
	TbREPORT_USER_FIELD_TYPE = 2,
	TbREPORT_USER_FIELD_REPORT_TYPE = 3,
	TbREPORT_USER_FIELD_TIME = 4,
	TbREPORT_USER_FIELD_STATUS = 5,
	TbREPORT_USER_FIELD_SEQ = 6,
};

enum ETbReport_userOpenType
{
	ETbREPORTUSER_OPEN_TYPE_PRIMARY = 0,
};

class TbReport_user : public AwsTable
{
public:
	TINT64 m_nUid;
	TINT64 m_nRid;
	TINT64 m_nType;
	TINT64 m_nReport_type;
	TINT64 m_nTime;
	TINT64 m_nStatus;
	TINT64 m_nSeq;

public:
	static TableDesc oTableDesc;
	Json::Reader oJsonReader;
	Json::FastWriter oJsonWriter;
	static int Init(const string& sConfFile, const string strProjectName);

public:
	TbReport_user():AwsTable(&oTableDesc)
	{
		Reset();
	}
	~TbReport_user()
	{}
	void Reset()
	{
		m_nUid = 0;
		m_nRid = 0;
		m_nType = 0;
		m_nReport_type = 0;
		m_nTime = 0;
		m_nStatus = 0;
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
	const TINT64& Get_Rid()
	{
		return m_nRid;
	}
	void Set_Rid(const TINT64& nRid)
	{
		m_nRid = nRid;
	}
	const TINT64& Get_Type()
	{
		return m_nType;
	}
	void Set_Type(const TINT64& nType, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nType = nType;
		m_mFlag[TbREPORT_USER_FIELD_TYPE] = dwActionType;
	}
	const TINT64& Get_Report_type()
	{
		return m_nReport_type;
	}
	void Set_Report_type(const TINT64& nReport_type, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nReport_type = nReport_type;
		m_mFlag[TbREPORT_USER_FIELD_REPORT_TYPE] = dwActionType;
	}
	const TINT64& Get_Time()
	{
		return m_nTime;
	}
	void Set_Time(const TINT64& nTime, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nTime = nTime;
		m_mFlag[TbREPORT_USER_FIELD_TIME] = dwActionType;
	}
	const TINT64& Get_Status()
	{
		return m_nStatus;
	}
	void Set_Status(const TINT64& nStatus, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nStatus = nStatus;
		m_mFlag[TbREPORT_USER_FIELD_STATUS] = dwActionType;
	}
	const TINT64& Get_Seq()
	{
		return m_nSeq;
	}
	void Set_Seq(const TINT64& nSeq, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSeq = nSeq;
		m_mFlag[TbREPORT_USER_FIELD_SEQ] = dwActionType;
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

