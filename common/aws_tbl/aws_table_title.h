#ifndef _AWS_TABLE_TITLE_H_
#define _AWS_TABLE_TITLE_H_

#include "aws_table_common.h"
#include "field_property.h"

#define EN_AWS_TABLE_TITLE "title"

enum ETbTitleField
{
	TbTITLE_FIELD_SID = 0,
	TbTITLE_FIELD_ID = 1,
	TbTITLE_FIELD_UID = 2,
	TbTITLE_FIELD_NAME = 3,
	TbTITLE_FIELD_ALNICK = 4,
	TbTITLE_FIELD_ALID = 5,
	TbTITLE_FIELD_SEQ = 6,
	TbTITLE_FIELD_DUB_TIME = 7,
	TbTITLE_FIELD_CID = 8,
};

enum ETbTitleOpenType
{
	ETbTITLE_OPEN_TYPE_PRIMARY = 0,
};

class TbTitle : public AwsTable
{
public:
	TINT64 m_nSid;
	TINT64 m_nId;
	TINT64 m_nUid;
	string m_sName;
	string m_sAlnick;
	TINT64 m_nAlid;
	TINT64 m_nSeq;
	TINT64 m_nDub_time;
	TINT64 m_nCid;

public:
	static TableDesc oTableDesc;
	Json::Reader oJsonReader;
	Json::FastWriter oJsonWriter;
	static int Init(const string& sConfFile, const string strProjectName);

public:
	TbTitle():AwsTable(&oTableDesc)
	{
		Reset();
	}
	~TbTitle()
	{}
	void Reset()
	{
		m_nSid = 0;
		m_nId = 0;
		m_nUid = 0;
		m_sName = "";
		m_sAlnick = "";
		m_nAlid = 0;
		m_nSeq = 0;
		m_nDub_time = 0;
		m_nCid = 0;
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
	const TINT64& Get_Id()
	{
		return m_nId;
	}
	void Set_Id(const TINT64& nId)
	{
		m_nId = nId;
	}
	const TINT64& Get_Uid()
	{
		return m_nUid;
	}
	void Set_Uid(const TINT64& nUid, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nUid = nUid;
		m_mFlag[TbTITLE_FIELD_UID] = dwActionType;
	}
	const string& Get_Name()
	{
		return m_sName;
	}
	void Set_Name(const string& sName)
	{
		m_sName = sName;
		m_mFlag[TbTITLE_FIELD_NAME] = sName.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const string& Get_Alnick()
	{
		return m_sAlnick;
	}
	void Set_Alnick(const string& sAlnick)
	{
		m_sAlnick = sAlnick;
		m_mFlag[TbTITLE_FIELD_ALNICK] = sAlnick.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Alid()
	{
		return m_nAlid;
	}
	void Set_Alid(const TINT64& nAlid, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nAlid = nAlid;
		m_mFlag[TbTITLE_FIELD_ALID] = dwActionType;
	}
	const TINT64& Get_Seq()
	{
		return m_nSeq;
	}
	void Set_Seq(const TINT64& nSeq, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSeq = nSeq;
		m_mFlag[TbTITLE_FIELD_SEQ] = dwActionType;
	}
	const TINT64& Get_Dub_time()
	{
		return m_nDub_time;
	}
	void Set_Dub_time(const TINT64& nDub_time, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nDub_time = nDub_time;
		m_mFlag[TbTITLE_FIELD_DUB_TIME] = dwActionType;
	}
	const TINT64& Get_Cid()
	{
		return m_nCid;
	}
	void Set_Cid(const TINT64& nCid, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nCid = nCid;
		m_mFlag[TbTITLE_FIELD_CID] = dwActionType;
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

