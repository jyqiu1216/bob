#ifndef _AWS_TABLE_SVR_STAT_H_
#define _AWS_TABLE_SVR_STAT_H_

#include "aws_table_common.h"
#include "field_property.h"

#define EN_AWS_TABLE_SVR_STAT "svr_stat"

enum ETbSvr_statField
{
	TbSVR_STAT_FIELD_SID = 0,
	TbSVR_STAT_FIELD_NAME = 1,
	TbSVR_STAT_FIELD_LANGUAGE = 2,
	TbSVR_STAT_FIELD_OPEN_TIME = 3,
	TbSVR_STAT_FIELD_STATUS = 4,
	TbSVR_STAT_FIELD_PLAIN_NUM = 5,
	TbSVR_STAT_FIELD_CITY_NUM = 6,
	TbSVR_STAT_FIELD_THRESHOLD = 7,
	TbSVR_STAT_FIELD_SWITCH = 8,
	TbSVR_STAT_FIELD_SCALE = 9,
	TbSVR_STAT_FIELD_POS = 10,
	TbSVR_STAT_FIELD_AVATAR = 11,
	TbSVR_STAT_FIELD_BLOCK_SIZE = 12,
	TbSVR_STAT_FIELD_SHOW_FLAG = 13,
	TbSVR_STAT_FIELD_DRAGON_FLAG = 14,
	TbSVR_STAT_FIELD_SEQ = 15,
};

enum ETbSvr_statOpenType
{
	ETbSVR_STAT_OPEN_TYPE_PRIMARY = 0,
};

class TbSvr_stat : public AwsTable
{
public:
	TINT64 m_nSid;
	string m_sName;
	string m_sLanguage;
	TINT64 m_nOpen_time;
	TINT64 m_nStatus;
	TINT64 m_nPlain_num;
	TINT64 m_nCity_num;
	TINT64 m_nThreshold;
	TINT64 m_nSwitch;
	TINT64 m_nScale;
	TINT64 m_nPos;
	TINT64 m_nAvatar;
	TINT64 m_nBlock_size;
	TINT64 m_nShow_flag;
	TINT64 m_nDragon_flag;
	TINT64 m_nSeq;

public:
	static TableDesc oTableDesc;
	Json::Reader oJsonReader;
	Json::FastWriter oJsonWriter;
	static int Init(const string& sConfFile, const string strProjectName);

public:
	TbSvr_stat():AwsTable(&oTableDesc)
	{
		Reset();
	}
	~TbSvr_stat()
	{}
	void Reset()
	{
		m_nSid = 0;
		m_sName = "";
		m_sLanguage = "";
		m_nOpen_time = 0;
		m_nStatus = 0;
		m_nPlain_num = 0;
		m_nCity_num = 0;
		m_nThreshold = 0;
		m_nSwitch = 0;
		m_nScale = 0;
		m_nPos = 0;
		m_nAvatar = 0;
		m_nBlock_size = 0;
		m_nShow_flag = 0;
		m_nDragon_flag = 0;
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
	const string& Get_Name()
	{
		return m_sName;
	}
	void Set_Name(const string& sName)
	{
		m_sName = sName;
		m_mFlag[TbSVR_STAT_FIELD_NAME] = sName.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const string& Get_Language()
	{
		return m_sLanguage;
	}
	void Set_Language(const string& sLanguage)
	{
		m_sLanguage = sLanguage;
		m_mFlag[TbSVR_STAT_FIELD_LANGUAGE] = sLanguage.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Open_time()
	{
		return m_nOpen_time;
	}
	void Set_Open_time(const TINT64& nOpen_time, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nOpen_time = nOpen_time;
		m_mFlag[TbSVR_STAT_FIELD_OPEN_TIME] = dwActionType;
	}
	const TINT64& Get_Status()
	{
		return m_nStatus;
	}
	void Set_Status(const TINT64& nStatus, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nStatus = nStatus;
		m_mFlag[TbSVR_STAT_FIELD_STATUS] = dwActionType;
	}
	const TINT64& Get_Plain_num()
	{
		return m_nPlain_num;
	}
	void Set_Plain_num(const TINT64& nPlain_num, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nPlain_num = nPlain_num;
		m_mFlag[TbSVR_STAT_FIELD_PLAIN_NUM] = dwActionType;
	}
	const TINT64& Get_City_num()
	{
		return m_nCity_num;
	}
	void Set_City_num(const TINT64& nCity_num, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nCity_num = nCity_num;
		m_mFlag[TbSVR_STAT_FIELD_CITY_NUM] = dwActionType;
	}
	const TINT64& Get_Threshold()
	{
		return m_nThreshold;
	}
	void Set_Threshold(const TINT64& nThreshold, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nThreshold = nThreshold;
		m_mFlag[TbSVR_STAT_FIELD_THRESHOLD] = dwActionType;
	}
	const TINT64& Get_Switch()
	{
		return m_nSwitch;
	}
	void Set_Switch(const TINT64& nSwitch, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSwitch = nSwitch;
		m_mFlag[TbSVR_STAT_FIELD_SWITCH] = dwActionType;
	}
	const TINT64& Get_Scale()
	{
		return m_nScale;
	}
	void Set_Scale(const TINT64& nScale, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nScale = nScale;
		m_mFlag[TbSVR_STAT_FIELD_SCALE] = dwActionType;
	}
	const TINT64& Get_Pos()
	{
		return m_nPos;
	}
	void Set_Pos(const TINT64& nPos, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nPos = nPos;
		m_mFlag[TbSVR_STAT_FIELD_POS] = dwActionType;
	}
	const TINT64& Get_Avatar()
	{
		return m_nAvatar;
	}
	void Set_Avatar(const TINT64& nAvatar, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nAvatar = nAvatar;
		m_mFlag[TbSVR_STAT_FIELD_AVATAR] = dwActionType;
	}
	const TINT64& Get_Block_size()
	{
		return m_nBlock_size;
	}
	void Set_Block_size(const TINT64& nBlock_size, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nBlock_size = nBlock_size;
		m_mFlag[TbSVR_STAT_FIELD_BLOCK_SIZE] = dwActionType;
	}
	const TINT64& Get_Show_flag()
	{
		return m_nShow_flag;
	}
	void Set_Show_flag(const TINT64& nShow_flag, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nShow_flag = nShow_flag;
		m_mFlag[TbSVR_STAT_FIELD_SHOW_FLAG] = dwActionType;
	}
	const TINT64& Get_Dragon_flag()
	{
		return m_nDragon_flag;
	}
	void Set_Dragon_flag(const TINT64& nDragon_flag, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nDragon_flag = nDragon_flag;
		m_mFlag[TbSVR_STAT_FIELD_DRAGON_FLAG] = dwActionType;
	}
	const TINT64& Get_Seq()
	{
		return m_nSeq;
	}
	void Set_Seq(const TINT64& nSeq, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSeq = nSeq;
		m_mFlag[TbSVR_STAT_FIELD_SEQ] = dwActionType;
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

