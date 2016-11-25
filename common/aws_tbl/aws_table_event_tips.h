#ifndef _AWS_TABLE_EVENT_TIPS_H_
#define _AWS_TABLE_EVENT_TIPS_H_

#include "aws_table_common.h"
#include "field_property.h"

#define EN_AWS_TABLE_EVENT_TIPS "event_tips"

enum ETbEvent_tipsField
{
	TbEVENT_TIPS_FIELD_UID = 0,
	TbEVENT_TIPS_FIELD_ID = 1,
	TbEVENT_TIPS_FIELD_CONTENT = 2,
	TbEVENT_TIPS_FIELD_TIME = 3,
	TbEVENT_TIPS_FIELD_EVENT_END_TIME = 4,
	TbEVENT_TIPS_FIELD_SEQ = 5,
};

enum ETbEvent_tipsOpenType
{
	ETbEVENTTIPS_OPEN_TYPE_PRIMARY = 0,
	ETbEVENTTIPS_OPEN_TYPE_TIME = 1,
};

class TbEvent_tips : public AwsTable
{
public:
	TINT64 m_nUid;
	TINT64 m_nId;
	Json::Value m_jContent;
	TINT64 m_nTime;
	TINT64 m_nEvent_end_time;
	TINT64 m_nSeq;

public:
	static TableDesc oTableDesc;
	Json::Reader oJsonReader;
	Json::FastWriter oJsonWriter;
	static int Init(const string& sConfFile, const string strProjectName);

public:
	TbEvent_tips():AwsTable(&oTableDesc)
	{
		Reset();
	}
	~TbEvent_tips()
	{}
	void Reset()
	{
		m_nUid = 0;
		m_nId = 0;
		m_jContent.clear();
		m_nTime = 0;
		m_nEvent_end_time = 0;
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
	const TINT64& Get_Id()
	{
		return m_nId;
	}
	void Set_Id(const TINT64& nId)
	{
		m_nId = nId;
	}
	const Json::Value& Get_Content()
	{
		return m_jContent;
	}
	void Set_Content(const Json::Value& jContent)
	{
		m_jContent = jContent;
		m_mFlag[TbEVENT_TIPS_FIELD_CONTENT] = jContent.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Time()
	{
		return m_nTime;
	}
	void Set_Time(const TINT64& nTime, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nTime = nTime;
		m_mFlag[TbEVENT_TIPS_FIELD_TIME] = dwActionType;
	}
	const TINT64& Get_Event_end_time()
	{
		return m_nEvent_end_time;
	}
	void Set_Event_end_time(const TINT64& nEvent_end_time, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nEvent_end_time = nEvent_end_time;
		m_mFlag[TbEVENT_TIPS_FIELD_EVENT_END_TIME] = dwActionType;
	}
	const TINT64& Get_Seq()
	{
		return m_nSeq;
	}
	void Set_Seq(const TINT64& nSeq, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSeq = nSeq;
		m_mFlag[TbEVENT_TIPS_FIELD_SEQ] = dwActionType;
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

