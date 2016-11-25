#ifndef _AWS_TABLE_AL_STORE_CONSUME_H_
#define _AWS_TABLE_AL_STORE_CONSUME_H_

#include "aws_table_common.h"
#include "field_property.h"

#define EN_AWS_TABLE_AL_STORE_CONSUME "al_store_consume"

enum ETbAl_store_consumeField
{
	TbAL_STORE_CONSUME_FIELD_AID = 0,
	TbAL_STORE_CONSUME_FIELD_ID = 1,
	TbAL_STORE_CONSUME_FIELD_UID = 2,
	TbAL_STORE_CONSUME_FIELD_ITEM_ID = 3,
	TbAL_STORE_CONSUME_FIELD_ITEM_NUM = 4,
	TbAL_STORE_CONSUME_FIELD_TIME = 5,
	TbAL_STORE_CONSUME_FIELD_LOYALTY = 6,
	TbAL_STORE_CONSUME_FIELD_UNAME = 7,
	TbAL_STORE_CONSUME_FIELD_SEQ = 8,
};

enum ETbAl_store_consumeOpenType
{
	ETbALSTORECONSUME_OPEN_TYPE_PRIMARY = 0,
};

class TbAl_store_consume : public AwsTable
{
public:
	TINT64 m_nAid;
	TINT64 m_nId;
	TINT64 m_nUid;
	TINT64 m_nItem_id;
	TINT64 m_nItem_num;
	TINT64 m_nTime;
	TINT64 m_nLoyalty;
	string m_sUname;
	TINT64 m_nSeq;

public:
	static TableDesc oTableDesc;
	Json::Reader oJsonReader;
	Json::FastWriter oJsonWriter;
	static int Init(const string& sConfFile, const string strProjectName);

public:
	TbAl_store_consume():AwsTable(&oTableDesc)
	{
		Reset();
	}
	~TbAl_store_consume()
	{}
	void Reset()
	{
		m_nAid = 0;
		m_nId = 0;
		m_nUid = 0;
		m_nItem_id = 0;
		m_nItem_num = 0;
		m_nTime = 0;
		m_nLoyalty = 0;
		m_sUname = "";
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
		m_mFlag[TbAL_STORE_CONSUME_FIELD_UID] = dwActionType;
	}
	const TINT64& Get_Item_id()
	{
		return m_nItem_id;
	}
	void Set_Item_id(const TINT64& nItem_id, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nItem_id = nItem_id;
		m_mFlag[TbAL_STORE_CONSUME_FIELD_ITEM_ID] = dwActionType;
	}
	const TINT64& Get_Item_num()
	{
		return m_nItem_num;
	}
	void Set_Item_num(const TINT64& nItem_num, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nItem_num = nItem_num;
		m_mFlag[TbAL_STORE_CONSUME_FIELD_ITEM_NUM] = dwActionType;
	}
	const TINT64& Get_Time()
	{
		return m_nTime;
	}
	void Set_Time(const TINT64& nTime, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nTime = nTime;
		m_mFlag[TbAL_STORE_CONSUME_FIELD_TIME] = dwActionType;
	}
	const TINT64& Get_Loyalty()
	{
		return m_nLoyalty;
	}
	void Set_Loyalty(const TINT64& nLoyalty, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nLoyalty = nLoyalty;
		m_mFlag[TbAL_STORE_CONSUME_FIELD_LOYALTY] = dwActionType;
	}
	const string& Get_Uname()
	{
		return m_sUname;
	}
	void Set_Uname(const string& sUname)
	{
		m_sUname = sUname;
		m_mFlag[TbAL_STORE_CONSUME_FIELD_UNAME] = sUname.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Seq()
	{
		return m_nSeq;
	}
	void Set_Seq(const TINT64& nSeq, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSeq = nSeq;
		m_mFlag[TbAL_STORE_CONSUME_FIELD_SEQ] = dwActionType;
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

