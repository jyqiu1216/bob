#ifndef _AWS_TABLE_SVR_AL_H_
#define _AWS_TABLE_SVR_AL_H_

#include "aws_table_common.h"
#include "field_property.h"

#define EN_AWS_TABLE_SVR_AL "svr_al"

enum ETbSvr_alField
{
	TbSVR_AL_FIELD_SID = 0,
	TbSVR_AL_FIELD_ALID = 1,
	TbSVR_AL_FIELD_OWNER_UID = 2,
	TbSVR_AL_FIELD_OWNER_CID = 3,
	TbSVR_AL_FIELD_SEQ = 4,
};

enum ETbSvr_alOpenType
{
	ETbSVR_AL_OPEN_TYPE_PRIMARY = 0,
};

class TbSvr_al : public AwsTable
{
public:
	TINT64 m_nSid;
	TINT64 m_nAlid;
	TINT64 m_nOwner_uid;
	TINT64 m_nOwner_cid;
	TINT64 m_nSeq;

public:
	static TableDesc oTableDesc;
	Json::Reader oJsonReader;
	Json::FastWriter oJsonWriter;
	static int Init(const string& sConfFile, const string strProjectName);

public:
	TbSvr_al():AwsTable(&oTableDesc)
	{
		Reset();
	}
	~TbSvr_al()
	{}
	void Reset()
	{
		m_nSid = 0;
		m_nAlid = 0;
		m_nOwner_uid = 0;
		m_nOwner_cid = 0;
		m_nSeq = 0;
		ClearFlag();
	};
	const TINT64& Get_Sid()
	{
		return m_nSid;
	}
	void Set_Sid(const TINT64& nSid)
	{
		m_nSid = nSid;
	}
	const TINT64& Get_Alid()
	{
		return m_nAlid;
	}
	void Set_Alid(const TINT64& nAlid)
	{
		m_nAlid = nAlid;
	}
	const TINT64& Get_Owner_uid()
	{
		return m_nOwner_uid;
	}
	void Set_Owner_uid(const TINT64& nOwner_uid, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nOwner_uid = nOwner_uid;
		m_mFlag[TbSVR_AL_FIELD_OWNER_UID] = dwActionType;
	}
	const TINT64& Get_Owner_cid()
	{
		return m_nOwner_cid;
	}
	void Set_Owner_cid(const TINT64& nOwner_cid, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nOwner_cid = nOwner_cid;
		m_mFlag[TbSVR_AL_FIELD_OWNER_CID] = dwActionType;
	}
	const TINT64& Get_Seq()
	{
		return m_nSeq;
	}
	void Set_Seq(const TINT64& nSeq, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSeq = nSeq;
		m_mFlag[TbSVR_AL_FIELD_SEQ] = dwActionType;
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

