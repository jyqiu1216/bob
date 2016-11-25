#ifndef _AWS_TABLE_UNIQUE_NAME_H_
#define _AWS_TABLE_UNIQUE_NAME_H_

#include "aws_table_common.h"
#include "field_property.h"

#define EN_AWS_TABLE_UNIQUE_NAME "unique_name"

enum ETbUnique_nameField
{
	TbUNIQUE_NAME_FIELD_TYPE = 0,
	TbUNIQUE_NAME_FIELD_NAME = 1,
	TbUNIQUE_NAME_FIELD_EXIST = 2,
	TbUNIQUE_NAME_FIELD_ID = 3,
	TbUNIQUE_NAME_FIELD_PROFILE = 4,
	TbUNIQUE_NAME_FIELD_SEQ = 5,
};

enum ETbUnique_nameOpenType
{
	ETbUNIQUE_NAME_OPEN_TYPE_PRIMARY = 0,
};

class TbUnique_name : public AwsTable
{
public:
	TINT64 m_nType;
	string m_sName;
	TINT64 m_nExist;
	TINT64 m_nId;
	Json::Value m_jProfile;
	TINT64 m_nSeq;

public:
	static TableDesc oTableDesc;
	Json::Reader oJsonReader;
	Json::FastWriter oJsonWriter;
	static int Init(const string& sConfFile, const string strProjectName);

public:
	TbUnique_name():AwsTable(&oTableDesc)
	{
		Reset();
	}
	~TbUnique_name()
	{}
	void Reset()
	{
		m_nType = 0;
		m_sName = "";
		m_nExist = 0;
		m_nId = 0;
		m_jProfile.clear();
		m_nSeq = 0;
		ClearFlag();
	};
	const TINT64& Get_Type()
	{
		return m_nType;
	}
	void Set_Type(const TINT64& nType)
	{
		m_nType = nType;
	}
	const string& Get_Name()
	{
		return m_sName;
	}
	void Set_Name(const string& sName)
	{
		m_sName = sName;
	}
	const TINT64& Get_Exist()
	{
		return m_nExist;
	}
	void Set_Exist(const TINT64& nExist, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nExist = nExist;
		m_mFlag[TbUNIQUE_NAME_FIELD_EXIST] = dwActionType;
	}
	const TINT64& Get_Id()
	{
		return m_nId;
	}
	void Set_Id(const TINT64& nId, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nId = nId;
		m_mFlag[TbUNIQUE_NAME_FIELD_ID] = dwActionType;
	}
	const Json::Value& Get_Profile()
	{
		return m_jProfile;
	}
	void Set_Profile(const Json::Value& jProfile)
	{
		m_jProfile = jProfile;
		m_mFlag[TbUNIQUE_NAME_FIELD_PROFILE] = jProfile.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Seq()
	{
		return m_nSeq;
	}
	void Set_Seq(const TINT64& nSeq, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSeq = nSeq;
		m_mFlag[TbUNIQUE_NAME_FIELD_SEQ] = dwActionType;
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

