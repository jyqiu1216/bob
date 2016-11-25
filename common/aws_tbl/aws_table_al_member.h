#ifndef _AWS_TABLE_AL_MEMBER_H_
#define _AWS_TABLE_AL_MEMBER_H_

#include "aws_table_common.h"
#include "field_property.h"

#define EN_AWS_TABLE_AL_MEMBER "al_member"

enum ETbAl_memberField
{
	TbAL_MEMBER_FIELD_AID = 0,
	TbAL_MEMBER_FIELD_UID = 1,
	TbAL_MEMBER_FIELD_AL_POS = 2,
	TbAL_MEMBER_FIELD_REQ_TIME = 3,
	TbAL_MEMBER_FIELD_PROFILE = 4,
	TbAL_MEMBER_FIELD_PROFILE_UPDATE_TIME = 5,
	TbAL_MEMBER_FIELD_SEQ = 6,
};

enum ETbAl_memberOpenType
{
	ETbALLIANCEMEMBER_OPEN_TYPE_PRIMARY = 0,
	ETbALLIANCEMEMBER_OPEN_TYPE_AL_POS = 1,
	ETbALLIANCEMEMBER_OPEN_TYPE_REQ_TIME = 2,
};

class TbAl_member : public AwsTable
{
public:
	TINT64 m_nAid;
	TINT64 m_nUid;
	TINT64 m_nAl_pos;
	TINT64 m_nReq_time;
	Json::Value m_jProfile;
	TINT64 m_nProfile_update_time;
	TINT64 m_nSeq;

public:
	static TableDesc oTableDesc;
	Json::Reader oJsonReader;
	Json::FastWriter oJsonWriter;
	static int Init(const string& sConfFile, const string strProjectName);

public:
	TbAl_member():AwsTable(&oTableDesc)
	{
		Reset();
	}
	~TbAl_member()
	{}
	void Reset()
	{
		m_nAid = 0;
		m_nUid = 0;
		m_nAl_pos = 0;
		m_nReq_time = 0;
		m_jProfile.clear();
		m_nProfile_update_time = 0;
		m_nSeq = 0;
		ClearFlag();
	};
	const TINT64& Get_Aid()
	{
		return m_nAid;
	}
	void Set_Aid(const TINT64& nAid)
	{
		m_nAid = nAid;
	}
	const TINT64& Get_Uid()
	{
		return m_nUid;
	}
	void Set_Uid(const TINT64& nUid)
	{
		m_nUid = nUid;
	}
	const TINT64& Get_Al_pos()
	{
		return m_nAl_pos;
	}
	void Set_Al_pos(const TINT64& nAl_pos, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nAl_pos = nAl_pos;
		m_mFlag[TbAL_MEMBER_FIELD_AL_POS] = dwActionType;
	}
	const TINT64& Get_Req_time()
	{
		return m_nReq_time;
	}
	void Set_Req_time(const TINT64& nReq_time, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nReq_time = nReq_time;
		m_mFlag[TbAL_MEMBER_FIELD_REQ_TIME] = dwActionType;
	}
	const Json::Value& Get_Profile()
	{
		return m_jProfile;
	}
	void Set_Profile(const Json::Value& jProfile)
	{
		m_jProfile = jProfile;
		m_mFlag[TbAL_MEMBER_FIELD_PROFILE] = jProfile.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Profile_update_time()
	{
		return m_nProfile_update_time;
	}
	void Set_Profile_update_time(const TINT64& nProfile_update_time, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nProfile_update_time = nProfile_update_time;
		m_mFlag[TbAL_MEMBER_FIELD_PROFILE_UPDATE_TIME] = dwActionType;
	}
	const TINT64& Get_Seq()
	{
		return m_nSeq;
	}
	void Set_Seq(const TINT64& nSeq, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSeq = nSeq;
		m_mFlag[TbAL_MEMBER_FIELD_SEQ] = dwActionType;
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

