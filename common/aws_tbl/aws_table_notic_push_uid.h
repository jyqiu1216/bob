#ifndef _AWS_TABLE_NOTIC_PUSH_UID_H_
#define _AWS_TABLE_NOTIC_PUSH_UID_H_

#include "aws_table_common.h"
#include "field_property.h"

#define EN_AWS_TABLE_NOTIC_PUSH_UID "notic_push_uid"

enum ETbNotic_push_uidField
{
	TbNOTIC_PUSH_UID_FIELD_UID = 0,
	TbNOTIC_PUSH_UID_FIELD_SEQ = 1,
	TbNOTIC_PUSH_UID_FIELD_DID = 2,
	TbNOTIC_PUSH_UID_FIELD_DE = 3,
	TbNOTIC_PUSH_UID_FIELD_APNS_TOKEN = 4,
	TbNOTIC_PUSH_UID_FIELD_APNS_NUM = 5,
	TbNOTIC_PUSH_UID_FIELD_UTIME = 6,
	TbNOTIC_PUSH_UID_FIELD_IDFA = 7,
	TbNOTIC_PUSH_UID_FIELD_PLATFORM = 8,
	TbNOTIC_PUSH_UID_FIELD_LANG = 9,
	TbNOTIC_PUSH_UID_FIELD_APNS_SWITCH = 10,
	TbNOTIC_PUSH_UID_FIELD_ARN = 11,
};

enum ETbNotic_push_uidOpenType
{
	ETbNOTIC_PUSH_UID_OPEN_TYPE_PRIMARY = 0,
};

class TbNotic_push_uid : public AwsTable
{
public:
	TINT64 m_nUid;
	TINT64 m_nSeq;
	TINT64 m_nDid;
	string m_sDe;
	string m_sApns_token;
	TINT64 m_nApns_num;
	TINT64 m_nUtime;
	string m_sIdfa;
	string m_sPlatform;
	TINT64 m_nLang;
	Json::Value m_jApns_switch;
	string m_sArn;

public:
	static TableDesc oTableDesc;
	Json::Reader oJsonReader;
	Json::FastWriter oJsonWriter;
	static int Init(const string& sConfFile, const string strProjectName);

public:
	TbNotic_push_uid():AwsTable(&oTableDesc)
	{
		Reset();
	}
	~TbNotic_push_uid()
	{}
	void Reset()
	{
		m_nUid = 0;
		m_nSeq = 0;
		m_nDid = 0;
		m_sDe = "";
		m_sApns_token = "";
		m_nApns_num = 0;
		m_nUtime = 0;
		m_sIdfa = "";
		m_sPlatform = "";
		m_nLang = 0;
		m_jApns_switch.clear();
		m_sArn = "";
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
	const TINT64& Get_Seq()
	{
		return m_nSeq;
	}
	void Set_Seq(const TINT64& nSeq, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSeq = nSeq;
		m_mFlag[TbNOTIC_PUSH_UID_FIELD_SEQ] = dwActionType;
	}
	const TINT64& Get_Did()
	{
		return m_nDid;
	}
	void Set_Did(const TINT64& nDid, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nDid = nDid;
		m_mFlag[TbNOTIC_PUSH_UID_FIELD_DID] = dwActionType;
	}
	const string& Get_De()
	{
		return m_sDe;
	}
	void Set_De(const string& sDe)
	{
		m_sDe = sDe;
		m_mFlag[TbNOTIC_PUSH_UID_FIELD_DE] = sDe.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const string& Get_Apns_token()
	{
		return m_sApns_token;
	}
	void Set_Apns_token(const string& sApns_token)
	{
		m_sApns_token = sApns_token;
		m_mFlag[TbNOTIC_PUSH_UID_FIELD_APNS_TOKEN] = sApns_token.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Apns_num()
	{
		return m_nApns_num;
	}
	void Set_Apns_num(const TINT64& nApns_num, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nApns_num = nApns_num;
		m_mFlag[TbNOTIC_PUSH_UID_FIELD_APNS_NUM] = dwActionType;
	}
	const TINT64& Get_Utime()
	{
		return m_nUtime;
	}
	void Set_Utime(const TINT64& nUtime, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nUtime = nUtime;
		m_mFlag[TbNOTIC_PUSH_UID_FIELD_UTIME] = dwActionType;
	}
	const string& Get_Idfa()
	{
		return m_sIdfa;
	}
	void Set_Idfa(const string& sIdfa)
	{
		m_sIdfa = sIdfa;
		m_mFlag[TbNOTIC_PUSH_UID_FIELD_IDFA] = sIdfa.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const string& Get_Platform()
	{
		return m_sPlatform;
	}
	void Set_Platform(const string& sPlatform)
	{
		m_sPlatform = sPlatform;
		m_mFlag[TbNOTIC_PUSH_UID_FIELD_PLATFORM] = sPlatform.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Lang()
	{
		return m_nLang;
	}
	void Set_Lang(const TINT64& nLang, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nLang = nLang;
		m_mFlag[TbNOTIC_PUSH_UID_FIELD_LANG] = dwActionType;
	}
	const Json::Value& Get_Apns_switch()
	{
		return m_jApns_switch;
	}
	void Set_Apns_switch(const Json::Value& jApns_switch)
	{
		m_jApns_switch = jApns_switch;
		m_mFlag[TbNOTIC_PUSH_UID_FIELD_APNS_SWITCH] = jApns_switch.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const string& Get_Arn()
	{
		return m_sArn;
	}
	void Set_Arn(const string& sArn)
	{
		m_sArn = sArn;
		m_mFlag[TbNOTIC_PUSH_UID_FIELD_ARN] = sArn.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
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

