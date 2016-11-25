#ifndef _AWS_TABLE_MAIL_USER_H_
#define _AWS_TABLE_MAIL_USER_H_

#include "aws_table_common.h"
#include "field_property.h"

#define EN_AWS_TABLE_MAIL_USER "mail_user"

enum ETbMail_userField
{
	TbMAIL_USER_FIELD_UID = 0,
	TbMAIL_USER_FIELD_MID = 1,
	TbMAIL_USER_FIELD_SUID = 2,
	TbMAIL_USER_FIELD_TUID = 3,
	TbMAIL_USER_FIELD_TIME = 4,
	TbMAIL_USER_FIELD_STATUS = 5,
	TbMAIL_USER_FIELD_DISPLAY_TYPE = 6,
	TbMAIL_USER_FIELD_RECEIVER_AVATAR = 7,
	TbMAIL_USER_FIELD_RECEIVER_NAME = 8,
	TbMAIL_USER_FIELD_HAS_REWARD = 9,
	TbMAIL_USER_FIELD_RECEIVER_ALNICK = 10,
	TbMAIL_USER_FIELD_IS_AL = 11,
	TbMAIL_USER_FIELD_IS_SINGLE_SVR = 12,
	TbMAIL_USER_FIELD_SID = 13,
	TbMAIL_USER_FIELD_VERSION = 14,
	TbMAIL_USER_FIELD_PLATFORM = 15,
	TbMAIL_USER_FIELD_SEQ = 16,
};

enum ETbMail_userOpenType
{
	ETbMAILUSER_OPEN_TYPE_PRIMARY = 0,
	ETbMAILUSER_OPEN_TYPE_COUNT = 1,
};

class TbMail_user : public AwsTable
{
public:
	TINT64 m_nUid;
	TINT64 m_nMid;
	TINT64 m_nSuid;
	TINT64 m_nTuid;
	TINT64 m_nTime;
	TINT64 m_nStatus;
	TINT64 m_nDisplay_type;
	TINT64 m_nReceiver_avatar;
	string m_sReceiver_name;
	TINT64 m_nHas_reward;
	string m_sReceiver_alnick;
	TINT64 m_nIs_al;
	TINT64 m_nIs_single_svr;
	TINT64 m_nSid;
	string m_sVersion;
	string m_sPlatform;
	TINT64 m_nSeq;

public:
	static TableDesc oTableDesc;
	Json::Reader oJsonReader;
	Json::FastWriter oJsonWriter;
	static int Init(const string& sConfFile, const string strProjectName);

public:
	TbMail_user():AwsTable(&oTableDesc)
	{
		Reset();
	}
	~TbMail_user()
	{}
	void Reset()
	{
		m_nUid = 0;
		m_nMid = 0;
		m_nSuid = 0;
		m_nTuid = 0;
		m_nTime = 0;
		m_nStatus = 0;
		m_nDisplay_type = 0;
		m_nReceiver_avatar = 0;
		m_sReceiver_name = "";
		m_nHas_reward = 0;
		m_sReceiver_alnick = "";
		m_nIs_al = 0;
		m_nIs_single_svr = 0;
		m_nSid = 0;
		m_sVersion = "";
		m_sPlatform = "";
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
	const TINT64& Get_Mid()
	{
		return m_nMid;
	}
	void Set_Mid(const TINT64& nMid)
	{
		m_nMid = nMid;
	}
	const TINT64& Get_Suid()
	{
		return m_nSuid;
	}
	void Set_Suid(const TINT64& nSuid, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSuid = nSuid;
		m_mFlag[TbMAIL_USER_FIELD_SUID] = dwActionType;
	}
	const TINT64& Get_Tuid()
	{
		return m_nTuid;
	}
	void Set_Tuid(const TINT64& nTuid, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nTuid = nTuid;
		m_mFlag[TbMAIL_USER_FIELD_TUID] = dwActionType;
	}
	const TINT64& Get_Time()
	{
		return m_nTime;
	}
	void Set_Time(const TINT64& nTime, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nTime = nTime;
		m_mFlag[TbMAIL_USER_FIELD_TIME] = dwActionType;
	}
	const TINT64& Get_Status()
	{
		return m_nStatus;
	}
	void Set_Status(const TINT64& nStatus, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nStatus = nStatus;
		m_mFlag[TbMAIL_USER_FIELD_STATUS] = dwActionType;
	}
	const TINT64& Get_Display_type()
	{
		return m_nDisplay_type;
	}
	void Set_Display_type(const TINT64& nDisplay_type, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nDisplay_type = nDisplay_type;
		m_mFlag[TbMAIL_USER_FIELD_DISPLAY_TYPE] = dwActionType;
	}
	const TINT64& Get_Receiver_avatar()
	{
		return m_nReceiver_avatar;
	}
	void Set_Receiver_avatar(const TINT64& nReceiver_avatar, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nReceiver_avatar = nReceiver_avatar;
		m_mFlag[TbMAIL_USER_FIELD_RECEIVER_AVATAR] = dwActionType;
	}
	const string& Get_Receiver_name()
	{
		return m_sReceiver_name;
	}
	void Set_Receiver_name(const string& sReceiver_name)
	{
		m_sReceiver_name = sReceiver_name;
		m_mFlag[TbMAIL_USER_FIELD_RECEIVER_NAME] = sReceiver_name.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Has_reward()
	{
		return m_nHas_reward;
	}
	void Set_Has_reward(const TINT64& nHas_reward, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nHas_reward = nHas_reward;
		m_mFlag[TbMAIL_USER_FIELD_HAS_REWARD] = dwActionType;
	}
	const string& Get_Receiver_alnick()
	{
		return m_sReceiver_alnick;
	}
	void Set_Receiver_alnick(const string& sReceiver_alnick)
	{
		m_sReceiver_alnick = sReceiver_alnick;
		m_mFlag[TbMAIL_USER_FIELD_RECEIVER_ALNICK] = sReceiver_alnick.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Is_al()
	{
		return m_nIs_al;
	}
	void Set_Is_al(const TINT64& nIs_al, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nIs_al = nIs_al;
		m_mFlag[TbMAIL_USER_FIELD_IS_AL] = dwActionType;
	}
	const TINT64& Get_Is_single_svr()
	{
		return m_nIs_single_svr;
	}
	void Set_Is_single_svr(const TINT64& nIs_single_svr, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nIs_single_svr = nIs_single_svr;
		m_mFlag[TbMAIL_USER_FIELD_IS_SINGLE_SVR] = dwActionType;
	}
	const TINT64& Get_Sid()
	{
		return m_nSid;
	}
	void Set_Sid(const TINT64& nSid, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSid = nSid;
		m_mFlag[TbMAIL_USER_FIELD_SID] = dwActionType;
	}
	const string& Get_Version()
	{
		return m_sVersion;
	}
	void Set_Version(const string& sVersion)
	{
		m_sVersion = sVersion;
		m_mFlag[TbMAIL_USER_FIELD_VERSION] = sVersion.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const string& Get_Platform()
	{
		return m_sPlatform;
	}
	void Set_Platform(const string& sPlatform)
	{
		m_sPlatform = sPlatform;
		m_mFlag[TbMAIL_USER_FIELD_PLATFORM] = sPlatform.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Seq()
	{
		return m_nSeq;
	}
	void Set_Seq(const TINT64& nSeq, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSeq = nSeq;
		m_mFlag[TbMAIL_USER_FIELD_SEQ] = dwActionType;
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

