#ifndef _AWS_TABLE_QUEST_H_
#define _AWS_TABLE_QUEST_H_

#include "aws_table_common.h"
#include "field_property.h"

#define EN_AWS_TABLE_QUEST "quest"

enum ETbQuestField
{
	TbQUEST_FIELD_UID = 0,
	TbQUEST_FIELD_TIMER_GIFT = 1,
	TbQUEST_FIELD_DAILY_QUEST = 2,
	TbQUEST_FIELD_AL_QUEST = 3,
	TbQUEST_FIELD_VIP_QUEST = 4,
	TbQUEST_FIELD_SEQ = 5,
};

enum ETbQuestOpenType
{
	ETbQUEST_OPEN_TYPE_PRIMARY = 0,
};

#define	TBQUEST_TIMER_GIFT_MAX_NUM	1
typedef	BinBuf<SQuestNode, TBQUEST_TIMER_GIFT_MAX_NUM>		TbQuest_Timer_gift;

#define	TBQUEST_DAILY_QUEST_MAX_NUM	1
typedef	BinBuf<SQuestNode, TBQUEST_DAILY_QUEST_MAX_NUM>		TbQuest_Daily_quest;

#define	TBQUEST_AL_QUEST_MAX_NUM	1
typedef	BinBuf<SQuestNode, TBQUEST_AL_QUEST_MAX_NUM>		TbQuest_Al_quest;

#define	TBQUEST_VIP_QUEST_MAX_NUM	1
typedef	BinBuf<SQuestNode, TBQUEST_VIP_QUEST_MAX_NUM>		TbQuest_Vip_quest;

class TbQuest : public AwsTable
{
public:
	TINT64 m_nUid;
	TbQuest_Timer_gift m_bTimer_gift;
	TbQuest_Daily_quest m_bDaily_quest;
	TbQuest_Al_quest m_bAl_quest;
	TbQuest_Vip_quest m_bVip_quest;
	TINT64 m_nSeq;

public:
	static TableDesc oTableDesc;
	Json::Reader oJsonReader;
	Json::FastWriter oJsonWriter;
	static int Init(const string& sConfFile, const string strProjectName);

public:
	TbQuest():AwsTable(&oTableDesc)
	{
		Reset();
	}
	~TbQuest()
	{}
	void Reset()
	{
		m_nUid = 0;
		m_bTimer_gift.Reset();
		m_bDaily_quest.Reset();
		m_bAl_quest.Reset();
		m_bVip_quest.Reset();
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
	const TINT64& Get_Seq()
	{
		return m_nSeq;
	}
	void Set_Seq(const TINT64& nSeq, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSeq = nSeq;
		m_mFlag[TbQUEST_FIELD_SEQ] = dwActionType;
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

