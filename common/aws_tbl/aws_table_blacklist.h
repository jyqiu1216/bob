#ifndef _AWS_TABLE_BLACKLIST_H_
#define _AWS_TABLE_BLACKLIST_H_

#include "aws_table_common.h"
#include "field_property.h"

#define EN_AWS_TABLE_BLACKLIST "blacklist"

enum ETbBlacklistField
{
	TbBLACKLIST_FIELD_UID = 0,
	TbBLACKLIST_FIELD_TARGET_UID = 1,
	TbBLACKLIST_FIELD_TARGET_UNAME = 2,
	TbBLACKLIST_FIELD_TARGET_AVATAR = 3,
	TbBLACKLIST_FIELD_UTIME = 4,
	TbBLACKLIST_FIELD_SEQ = 5,
};

enum ETbBlacklistOpenType
{
	ETbBLACKLIST_OPEN_TYPE_PRIMARY = 0,
};

class TbBlacklist : public AwsTable
{
public:
	TINT64 m_nUid;
	TINT64 m_nTarget_uid;
	string m_sTarget_uname;
	TINT64 m_nTarget_avatar;
	TINT64 m_nUtime;
	TINT64 m_nSeq;

public:
	static TableDesc oTableDesc;
	Json::Reader oJsonReader;
	Json::FastWriter oJsonWriter;
	static int Init(const string& sConfFile, const string strProjectName);

public:
	TbBlacklist():AwsTable(&oTableDesc)
	{
		Reset();
	}
	~TbBlacklist()
	{}
	void Reset()
	{
		m_nUid = 0;
		m_nTarget_uid = 0;
		m_sTarget_uname = "";
		m_nTarget_avatar = 0;
		m_nUtime = 0;
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
	const TINT64& Get_Target_uid()
	{
		return m_nTarget_uid;
	}
	void Set_Target_uid(const TINT64& nTarget_uid)
	{
		m_nTarget_uid = nTarget_uid;
	}
	const string& Get_Target_uname()
	{
		return m_sTarget_uname;
	}
	void Set_Target_uname(const string& sTarget_uname)
	{
		m_sTarget_uname = sTarget_uname;
		m_mFlag[TbBLACKLIST_FIELD_TARGET_UNAME] = sTarget_uname.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Target_avatar()
	{
		return m_nTarget_avatar;
	}
	void Set_Target_avatar(const TINT64& nTarget_avatar, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nTarget_avatar = nTarget_avatar;
		m_mFlag[TbBLACKLIST_FIELD_TARGET_AVATAR] = dwActionType;
	}
	const TINT64& Get_Utime()
	{
		return m_nUtime;
	}
	void Set_Utime(const TINT64& nUtime, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nUtime = nUtime;
		m_mFlag[TbBLACKLIST_FIELD_UTIME] = dwActionType;
	}
	const TINT64& Get_Seq()
	{
		return m_nSeq;
	}
	void Set_Seq(const TINT64& nSeq, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSeq = nSeq;
		m_mFlag[TbBLACKLIST_FIELD_SEQ] = dwActionType;
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

