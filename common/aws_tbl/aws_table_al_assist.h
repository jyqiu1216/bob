#ifndef _AWS_TABLE_AL_ASSIST_H_
#define _AWS_TABLE_AL_ASSIST_H_

#include "aws_table_common.h"
#include "field_property.h"

#define EN_AWS_TABLE_AL_ASSIST "al_assist"

enum ETbAl_assistField
{
	TbAL_ASSIST_FIELD_SID = 0,
	TbAL_ASSIST_FIELD_ID = 1,
	TbAL_ASSIST_FIELD_TYPE = 2,
	TbAL_ASSIST_FIELD_UID = 3,
	TbAL_ASSIST_FIELD_UNAME = 4,
	TbAL_ASSIST_FIELD_CID = 5,
	TbAL_ASSIST_FIELD_AID = 6,
	TbAL_ASSIST_FIELD_TIME = 7,
	TbAL_ASSIST_FIELD_PARAM = 8,
	TbAL_ASSIST_FIELD_PROGRESS = 9,
	TbAL_ASSIST_FIELD_DESC = 10,
	TbAL_ASSIST_FIELD_SEQ = 11,
};

enum ETbAl_assistOpenType
{
	ETbALASSIST_OPEN_TYPE_PRIMARY = 0,
};

#define	TBAL_ASSIST_PARAM_MAX_NUM	1
typedef	BinBuf<SCommonResource, TBAL_ASSIST_PARAM_MAX_NUM>		TbAl_assist_Param;

#define	TBAL_ASSIST_PROGRESS_MAX_NUM	1
typedef	BinBuf<SCommonResource, TBAL_ASSIST_PROGRESS_MAX_NUM>		TbAl_assist_Progress;

class TbAl_assist : public AwsTable
{
public:
	TINT64 m_nSid;
	TINT64 m_nId;
	TINT64 m_nType;
	TINT64 m_nUid;
	string m_sUname;
	TINT64 m_nCid;
	TINT64 m_nAid;
	TINT64 m_nTime;
	TbAl_assist_Param m_bParam;
	TbAl_assist_Progress m_bProgress;
	string m_sDesc;
	TINT64 m_nSeq;

public:
	static TableDesc oTableDesc;
	Json::Reader oJsonReader;
	Json::FastWriter oJsonWriter;
	static int Init(const string& sConfFile, const string strProjectName);

public:
	TbAl_assist():AwsTable(&oTableDesc)
	{
		Reset();
	}
	~TbAl_assist()
	{}
	void Reset()
	{
		m_nSid = 0;
		m_nId = 0;
		m_nType = 0;
		m_nUid = 0;
		m_sUname = "";
		m_nCid = 0;
		m_nAid = 0;
		m_nTime = 0;
		m_bParam.Reset();
		m_bProgress.Reset();
		m_sDesc = "";
		m_nSeq = 0;
		ClearFlag();
	};
	const TINT64& Get_Sid()
	{
		return m_nSid;
	}
	void Set_Sid(const TINT64& nSid, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSid = nSid;
		m_mFlag[TbAL_ASSIST_FIELD_SID] = dwActionType;
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
		m_mFlag[TbAL_ASSIST_FIELD_TYPE] = dwActionType;
	}
	const TINT64& Get_Uid()
	{
		return m_nUid;
	}
	void Set_Uid(const TINT64& nUid, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nUid = nUid;
		m_mFlag[TbAL_ASSIST_FIELD_UID] = dwActionType;
	}
	const string& Get_Uname()
	{
		return m_sUname;
	}
	void Set_Uname(const string& sUname)
	{
		m_sUname = sUname;
		m_mFlag[TbAL_ASSIST_FIELD_UNAME] = sUname.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Cid()
	{
		return m_nCid;
	}
	void Set_Cid(const TINT64& nCid, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nCid = nCid;
		m_mFlag[TbAL_ASSIST_FIELD_CID] = dwActionType;
	}
	const TINT64& Get_Aid()
	{
		return m_nAid;
	}
	void Set_Aid(const TINT64& nAid)
	{
		m_nAid = nAid;
	}
	const TINT64& Get_Time()
	{
		return m_nTime;
	}
	void Set_Time(const TINT64& nTime, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nTime = nTime;
		m_mFlag[TbAL_ASSIST_FIELD_TIME] = dwActionType;
	}
	const string& Get_Desc()
	{
		return m_sDesc;
	}
	void Set_Desc(const string& sDesc)
	{
		m_sDesc = sDesc;
		m_mFlag[TbAL_ASSIST_FIELD_DESC] = sDesc.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Seq()
	{
		return m_nSeq;
	}
	void Set_Seq(const TINT64& nSeq, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSeq = nSeq;
		m_mFlag[TbAL_ASSIST_FIELD_SEQ] = dwActionType;
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

