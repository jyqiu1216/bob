#ifndef _AWS_TABLE_AL_GIFT_H_
#define _AWS_TABLE_AL_GIFT_H_

#include "aws_table_common.h"
#include "field_property.h"

#define EN_AWS_TABLE_AL_GIFT "al_gift"

enum ETbAl_giftField
{
	TbAL_GIFT_FIELD_AID = 0,
	TbAL_GIFT_FIELD_ID = 1,
	TbAL_GIFT_FIELD_TYPE = 2,
	TbAL_GIFT_FIELD_UNAME = 3,
	TbAL_GIFT_FIELD_PACK_ID = 4,
	TbAL_GIFT_FIELD_GIFT_POINT = 5,
	TbAL_GIFT_FIELD_CTIME = 6,
	TbAL_GIFT_FIELD_SEQ = 7,
};

enum ETbAl_giftOpenType
{
	ETbALGIFT_OPEN_TYPE_PRIMARY = 0,
};

class TbAl_gift : public AwsTable
{
public:
	TINT64 m_nAid;
	TINT64 m_nId;
	TINT64 m_nType;
	string m_sUname;
	TINT64 m_nPack_id;
	TINT64 m_nGift_point;
	TINT64 m_nCtime;
	TINT64 m_nSeq;

public:
	static TableDesc oTableDesc;
	Json::Reader oJsonReader;
	Json::FastWriter oJsonWriter;
	static int Init(const string& sConfFile, const string strProjectName);

public:
	TbAl_gift():AwsTable(&oTableDesc)
	{
		Reset();
	}
	~TbAl_gift()
	{}
	void Reset()
	{
		m_nAid = 0;
		m_nId = 0;
		m_nType = 0;
		m_sUname = "";
		m_nPack_id = 0;
		m_nGift_point = 0;
		m_nCtime = 0;
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
	const TINT64& Get_Type()
	{
		return m_nType;
	}
	void Set_Type(const TINT64& nType, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nType = nType;
		m_mFlag[TbAL_GIFT_FIELD_TYPE] = dwActionType;
	}
	const string& Get_Uname()
	{
		return m_sUname;
	}
	void Set_Uname(const string& sUname)
	{
		m_sUname = sUname;
		m_mFlag[TbAL_GIFT_FIELD_UNAME] = sUname.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Pack_id()
	{
		return m_nPack_id;
	}
	void Set_Pack_id(const TINT64& nPack_id, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nPack_id = nPack_id;
		m_mFlag[TbAL_GIFT_FIELD_PACK_ID] = dwActionType;
	}
	const TINT64& Get_Gift_point()
	{
		return m_nGift_point;
	}
	void Set_Gift_point(const TINT64& nGift_point, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nGift_point = nGift_point;
		m_mFlag[TbAL_GIFT_FIELD_GIFT_POINT] = dwActionType;
	}
	const TINT64& Get_Ctime()
	{
		return m_nCtime;
	}
	void Set_Ctime(const TINT64& nCtime, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nCtime = nCtime;
		m_mFlag[TbAL_GIFT_FIELD_CTIME] = dwActionType;
	}
	const TINT64& Get_Seq()
	{
		return m_nSeq;
	}
	void Set_Seq(const TINT64& nSeq, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSeq = nSeq;
		m_mFlag[TbAL_GIFT_FIELD_SEQ] = dwActionType;
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

