#ifndef _AWS_TABLE_IDOL_H_
#define _AWS_TABLE_IDOL_H_

#include "aws_table_common.h"
#include "field_property.h"

#define EN_AWS_TABLE_IDOL "idol"

enum ETbIdolField
{
	TbIDOL_FIELD_SID = 0,
	TbIDOL_FIELD_ID = 1,
	TbIDOL_FIELD_POS = 2,
	TbIDOL_FIELD_ALID = 3,
	TbIDOL_FIELD_TROOP = 4,
	TbIDOL_FIELD_INFO = 5,
	TbIDOL_FIELD_STATUS = 6,
	TbIDOL_FIELD_END_TIME = 7,
	TbIDOL_FIELD_LAST_TIME = 8,
	TbIDOL_FIELD_RANK = 9,
	TbIDOL_FIELD_ACTIVE = 10,
	TbIDOL_FIELD_SEQ = 11,
};

enum ETbIdolOpenType
{
	ETbIDOL_OPEN_TYPE_PRIMARY = 0,
};

#define	TBIDOL_TROOP_MAX_NUM	1
typedef	BinBuf<SCommonTroop, TBIDOL_TROOP_MAX_NUM>		TbIdol_Troop;

class TbIdol : public AwsTable
{
public:
	TINT64 m_nSid;
	TINT64 m_nId;
	TINT64 m_nPos;
	TINT64 m_nAlid;
	TbIdol_Troop m_bTroop;
	Json::Value m_jInfo;
	TINT64 m_nStatus;
	TINT64 m_nEnd_time;
	TINT64 m_nLast_time;
	Json::Value m_jRank;
	TINT64 m_nActive;
	TINT64 m_nSeq;

public:
	static TableDesc oTableDesc;
	Json::Reader oJsonReader;
	Json::FastWriter oJsonWriter;
	static int Init(const string& sConfFile, const string strProjectName);

public:
	TbIdol():AwsTable(&oTableDesc)
	{
		Reset();
	}
	~TbIdol()
	{}
	void Reset()
	{
		m_nSid = 0;
		m_nId = 0;
		m_nPos = 0;
		m_nAlid = 0;
		m_bTroop.Reset();
		m_jInfo.clear();
		m_nStatus = 0;
		m_nEnd_time = 0;
		m_nLast_time = 0;
		m_jRank.clear();
		m_nActive = 0;
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
	const TINT64& Get_Id()
	{
		return m_nId;
	}
	void Set_Id(const TINT64& nId)
	{
		m_nId = nId;
	}
	const TINT64& Get_Pos()
	{
		return m_nPos;
	}
	void Set_Pos(const TINT64& nPos, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nPos = nPos;
		m_mFlag[TbIDOL_FIELD_POS] = dwActionType;
	}
	const TINT64& Get_Alid()
	{
		return m_nAlid;
	}
	void Set_Alid(const TINT64& nAlid, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nAlid = nAlid;
		m_mFlag[TbIDOL_FIELD_ALID] = dwActionType;
	}
	const Json::Value& Get_Info()
	{
		return m_jInfo;
	}
	void Set_Info(const Json::Value& jInfo)
	{
		m_jInfo = jInfo;
		m_mFlag[TbIDOL_FIELD_INFO] = jInfo.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Status()
	{
		return m_nStatus;
	}
	void Set_Status(const TINT64& nStatus, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nStatus = nStatus;
		m_mFlag[TbIDOL_FIELD_STATUS] = dwActionType;
	}
	const TINT64& Get_End_time()
	{
		return m_nEnd_time;
	}
	void Set_End_time(const TINT64& nEnd_time, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nEnd_time = nEnd_time;
		m_mFlag[TbIDOL_FIELD_END_TIME] = dwActionType;
	}
	const TINT64& Get_Last_time()
	{
		return m_nLast_time;
	}
	void Set_Last_time(const TINT64& nLast_time, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nLast_time = nLast_time;
		m_mFlag[TbIDOL_FIELD_LAST_TIME] = dwActionType;
	}
	const Json::Value& Get_Rank()
	{
		return m_jRank;
	}
	void Set_Rank(const Json::Value& jRank)
	{
		m_jRank = jRank;
		m_mFlag[TbIDOL_FIELD_RANK] = jRank.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Active()
	{
		return m_nActive;
	}
	void Set_Active(const TINT64& nActive, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nActive = nActive;
		m_mFlag[TbIDOL_FIELD_ACTIVE] = dwActionType;
	}
	const TINT64& Get_Seq()
	{
		return m_nSeq;
	}
	void Set_Seq(const TINT64& nSeq, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSeq = nSeq;
		m_mFlag[TbIDOL_FIELD_SEQ] = dwActionType;
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

