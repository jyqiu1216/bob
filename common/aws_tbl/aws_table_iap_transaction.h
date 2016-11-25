#ifndef _AWS_TABLE_IAP_TRANSACTION_H_
#define _AWS_TABLE_IAP_TRANSACTION_H_

#include "aws_table_common.h"
#include "field_property.h"

#define EN_AWS_TABLE_IAP_TRANSACTION "iap_transaction"

enum ETbIap_transactionField
{
	TbIAP_TRANSACTION_FIELD_TID = 0,
	TbIAP_TRANSACTION_FIELD_SID = 1,
	TbIAP_TRANSACTION_FIELD_UID = 2,
	TbIAP_TRANSACTION_FIELD_TIME = 3,
	TbIAP_TRANSACTION_FIELD_GEM = 4,
	TbIAP_TRANSACTION_FIELD_SANDBOX = 5,
	TbIAP_TRANSACTION_FIELD_PLATFORM = 6,
	TbIAP_TRANSACTION_FIELD_IP = 7,
	TbIAP_TRANSACTION_FIELD_SEQ = 8,
};

enum ETbIap_transactionOpenType
{
	ETbIAPTRANSACTION_OPEN_TYPE_PRIMARY = 0,
};

class TbIap_transaction : public AwsTable
{
public:
	string m_sTid;
	TINT64 m_nSid;
	TINT64 m_nUid;
	TINT64 m_nTime;
	TINT64 m_nGem;
	TINT64 m_nSandbox;
	string m_sPlatform;
	string m_sIp;
	TINT64 m_nSeq;

public:
	static TableDesc oTableDesc;
	Json::Reader oJsonReader;
	Json::FastWriter oJsonWriter;
	static int Init(const string& sConfFile, const string strProjectName);

public:
	TbIap_transaction():AwsTable(&oTableDesc)
	{
		Reset();
	}
	~TbIap_transaction()
	{}
	void Reset()
	{
		m_sTid = "";
		m_nSid = 0;
		m_nUid = 0;
		m_nTime = 0;
		m_nGem = 0;
		m_nSandbox = 0;
		m_sPlatform = "";
		m_sIp = "";
		m_nSeq = 0;
		ClearFlag();
	};
	const string& Get_Tid()
	{
		return m_sTid;
	}
	void Set_Tid(const string& sTid)
	{
		m_sTid = sTid;
	}
	const TINT64& Get_Sid()
	{
		return m_nSid;
	}
	void Set_Sid(const TINT64& nSid, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSid = nSid;
		m_mFlag[TbIAP_TRANSACTION_FIELD_SID] = dwActionType;
	}
	const TINT64& Get_Uid()
	{
		return m_nUid;
	}
	void Set_Uid(const TINT64& nUid, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nUid = nUid;
		m_mFlag[TbIAP_TRANSACTION_FIELD_UID] = dwActionType;
	}
	const TINT64& Get_Time()
	{
		return m_nTime;
	}
	void Set_Time(const TINT64& nTime, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nTime = nTime;
		m_mFlag[TbIAP_TRANSACTION_FIELD_TIME] = dwActionType;
	}
	const TINT64& Get_Gem()
	{
		return m_nGem;
	}
	void Set_Gem(const TINT64& nGem, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nGem = nGem;
		m_mFlag[TbIAP_TRANSACTION_FIELD_GEM] = dwActionType;
	}
	const TINT64& Get_Sandbox()
	{
		return m_nSandbox;
	}
	void Set_Sandbox(const TINT64& nSandbox, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSandbox = nSandbox;
		m_mFlag[TbIAP_TRANSACTION_FIELD_SANDBOX] = dwActionType;
	}
	const string& Get_Platform()
	{
		return m_sPlatform;
	}
	void Set_Platform(const string& sPlatform)
	{
		m_sPlatform = sPlatform;
		m_mFlag[TbIAP_TRANSACTION_FIELD_PLATFORM] = sPlatform.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const string& Get_Ip()
	{
		return m_sIp;
	}
	void Set_Ip(const string& sIp)
	{
		m_sIp = sIp;
		m_mFlag[TbIAP_TRANSACTION_FIELD_IP] = sIp.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Seq()
	{
		return m_nSeq;
	}
	void Set_Seq(const TINT64& nSeq, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSeq = nSeq;
		m_mFlag[TbIAP_TRANSACTION_FIELD_SEQ] = dwActionType;
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

