#ifndef _AWS_TABLE_ALLIANCE_ACTION_H_
#define _AWS_TABLE_ALLIANCE_ACTION_H_

#include "aws_table_common.h"
#include "field_property.h"

#define EN_AWS_TABLE_ALLIANCE_ACTION "alliance_action"

enum ETbAlliance_actionField
{
	TbALLIANCE_ACTION_FIELD_ID = 0,
	TbALLIANCE_ACTION_FIELD_SUID = 1,
	TbALLIANCE_ACTION_FIELD_MCLASS = 2,
	TbALLIANCE_ACTION_FIELD_SCLASS = 3,
	TbALLIANCE_ACTION_FIELD_STATUS = 4,
	TbALLIANCE_ACTION_FIELD_BTIME = 5,
	TbALLIANCE_ACTION_FIELD_CTIME = 6,
	TbALLIANCE_ACTION_FIELD_ETIME = 7,
	TbALLIANCE_ACTION_FIELD_PARAM = 8,
	TbALLIANCE_ACTION_FIELD_RETRY = 9,
	TbALLIANCE_ACTION_FIELD_SAL = 10,
	TbALLIANCE_ACTION_FIELD_CAN_HELP_NUM = 11,
	TbALLIANCE_ACTION_FIELD_HELPED_NUM = 12,
	TbALLIANCE_ACTION_FIELD_SID = 13,
	TbALLIANCE_ACTION_FIELD_NOTI_FLAG = 14,
	TbALLIANCE_ACTION_FIELD_SCID = 15,
	TbALLIANCE_ACTION_FIELD_SEQ = 16,
};

enum ETbAlliance_actionOpenType
{
	ETbALLIANCEACTION_OPEN_TYPE_PRIMARY = 0,
	ETbALLIANCEACTION_OPEN_TYPE_GLB_ETIME = 1,
	ETbALLIANCEACTION_OPEN_TYPE_GLB_SAL = 2,
};

#define	TBALLIANCE_ACTION_PARAM_MAX_NUM	1
typedef	BinBuf<UActionParam, TBALLIANCE_ACTION_PARAM_MAX_NUM>		TbAlliance_action_Param;

class TbAlliance_action : public AwsTable
{
public:
	TINT64 m_nId;
	TINT64 m_nSuid;
	TINT64 m_nMclass;
	TINT64 m_nSclass;
	TINT64 m_nStatus;
	TINT64 m_nBtime;
	TINT64 m_nCtime;
	TINT64 m_nEtime;
	TbAlliance_action_Param m_bParam;
	TINT64 m_nRetry;
	TINT64 m_nSal;
	TINT64 m_nCan_help_num;
	TINT64 m_nHelped_num;
	TINT64 m_nSid;
	TINT64 m_nNoti_flag;
	TINT64 m_nScid;
	TINT64 m_nSeq;

public:
	static TableDesc oTableDesc;
	Json::Reader oJsonReader;
	Json::FastWriter oJsonWriter;
	static int Init(const string& sConfFile, const string strProjectName);

public:
	TbAlliance_action():AwsTable(&oTableDesc)
	{
		Reset();
	}
	~TbAlliance_action()
	{}
	void Reset()
	{
		m_nId = 0;
		m_nSuid = 0;
		m_nMclass = 0;
		m_nSclass = 0;
		m_nStatus = 0;
		m_nBtime = 0;
		m_nCtime = 0;
		m_nEtime = 0;
		m_bParam.Reset();
		m_nRetry = 0;
		m_nSal = 0;
		m_nCan_help_num = 0;
		m_nHelped_num = 0;
		m_nSid = 0;
		m_nNoti_flag = 0;
		m_nScid = 0;
		m_nSeq = 0;
		ClearFlag();
	};
	const TINT64& Get_Id()
	{
		return m_nId;
	}
	void Set_Id(const TINT64& nId)
	{
		m_nId = nId;
	}
	const TINT64& Get_Suid()
	{
		return m_nSuid;
	}
	void Set_Suid(const TINT64& nSuid)
	{
		m_nSuid = nSuid;
	}
	const TINT64& Get_Mclass()
	{
		return m_nMclass;
	}
	void Set_Mclass(const TINT64& nMclass, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nMclass = nMclass;
		m_mFlag[TbALLIANCE_ACTION_FIELD_MCLASS] = dwActionType;
	}
	const TINT64& Get_Sclass()
	{
		return m_nSclass;
	}
	void Set_Sclass(const TINT64& nSclass, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSclass = nSclass;
		m_mFlag[TbALLIANCE_ACTION_FIELD_SCLASS] = dwActionType;
	}
	const TINT64& Get_Status()
	{
		return m_nStatus;
	}
	void Set_Status(const TINT64& nStatus, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nStatus = nStatus;
		m_mFlag[TbALLIANCE_ACTION_FIELD_STATUS] = dwActionType;
	}
	const TINT64& Get_Btime()
	{
		return m_nBtime;
	}
	void Set_Btime(const TINT64& nBtime, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nBtime = nBtime;
		m_mFlag[TbALLIANCE_ACTION_FIELD_BTIME] = dwActionType;
	}
	const TINT64& Get_Ctime()
	{
		return m_nCtime;
	}
	void Set_Ctime(const TINT64& nCtime, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nCtime = nCtime;
		m_mFlag[TbALLIANCE_ACTION_FIELD_CTIME] = dwActionType;
	}
	const TINT64& Get_Etime()
	{
		return m_nEtime;
	}
	void Set_Etime(const TINT64& nEtime, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nEtime = nEtime;
		m_mFlag[TbALLIANCE_ACTION_FIELD_ETIME] = dwActionType;
	}
	const TINT64& Get_Retry()
	{
		return m_nRetry;
	}
	void Set_Retry(const TINT64& nRetry, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nRetry = nRetry;
		m_mFlag[TbALLIANCE_ACTION_FIELD_RETRY] = dwActionType;
	}
	const TINT64& Get_Sal()
	{
		return m_nSal;
	}
	void Set_Sal(const TINT64& nSal, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSal = nSal;
		m_mFlag[TbALLIANCE_ACTION_FIELD_SAL] = dwActionType;
	}
	const TINT64& Get_Can_help_num()
	{
		return m_nCan_help_num;
	}
	void Set_Can_help_num(const TINT64& nCan_help_num, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nCan_help_num = nCan_help_num;
		m_mFlag[TbALLIANCE_ACTION_FIELD_CAN_HELP_NUM] = dwActionType;
	}
	const TINT64& Get_Helped_num()
	{
		return m_nHelped_num;
	}
	void Set_Helped_num(const TINT64& nHelped_num, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nHelped_num = nHelped_num;
		m_mFlag[TbALLIANCE_ACTION_FIELD_HELPED_NUM] = dwActionType;
	}
	const TINT64& Get_Sid()
	{
		return m_nSid;
	}
	void Set_Sid(const TINT64& nSid, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSid = nSid;
		m_mFlag[TbALLIANCE_ACTION_FIELD_SID] = dwActionType;
	}
	const TINT64& Get_Noti_flag()
	{
		return m_nNoti_flag;
	}
	void Set_Noti_flag(const TINT64& nNoti_flag, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nNoti_flag = nNoti_flag;
		m_mFlag[TbALLIANCE_ACTION_FIELD_NOTI_FLAG] = dwActionType;
	}
	const TINT64& Get_Scid()
	{
		return m_nScid;
	}
	void Set_Scid(const TINT64& nScid, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nScid = nScid;
		m_mFlag[TbALLIANCE_ACTION_FIELD_SCID] = dwActionType;
	}
	const TINT64& Get_Seq()
	{
		return m_nSeq;
	}
	void Set_Seq(const TINT64& nSeq, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSeq = nSeq;
		m_mFlag[TbALLIANCE_ACTION_FIELD_SEQ] = dwActionType;
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

