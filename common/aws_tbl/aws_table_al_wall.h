#ifndef _AWS_TABLE_AL_WALL_H_
#define _AWS_TABLE_AL_WALL_H_

#include "aws_table_common.h"
#include "field_property.h"

#define EN_AWS_TABLE_AL_WALL "al_wall"

enum ETbAl_wallField
{
	TbAL_WALL_FIELD_ALID = 0,
	TbAL_WALL_FIELD_WALL_ID = 1,
	TbAL_WALL_FIELD_TIME = 2,
	TbAL_WALL_FIELD_ALPOS = 3,
	TbAL_WALL_FIELD_UID = 4,
	TbAL_WALL_FIELD_UIN = 5,
	TbAL_WALL_FIELD_CONTENT = 6,
	TbAL_WALL_FIELD_AVATAR = 7,
	TbAL_WALL_FIELD_TOPFLAG = 8,
	TbAL_WALL_FIELD_TOPTIME = 9,
	TbAL_WALL_FIELD_RAW_LANG = 10,
	TbAL_WALL_FIELD_TRANSLATE_CONTENT = 11,
	TbAL_WALL_FIELD_SEQ = 12,
};

enum ETbAl_wallOpenType
{
	ETbALWALL_OPEN_TYPE_PRIMARY = 0,
	ETbALWALL_OPEN_TYPE_COUNT = 1,
	ETbALWALL_OPEN_TYPE_TOPFLAG = 2,
};

class TbAl_wall : public AwsTable
{
public:
	TINT64 m_nAlid;
	TINT64 m_nWall_id;
	TINT64 m_nTime;
	TINT64 m_nAlpos;
	TINT64 m_nUid;
	string m_sUin;
	string m_sContent;
	TINT64 m_nAvatar;
	TINT64 m_nTopflag;
	TINT64 m_nToptime;
	TINT64 m_nRaw_lang;
	string m_sTranslate_content;
	TINT64 m_nSeq;

public:
	static TableDesc oTableDesc;
	Json::Reader oJsonReader;
	Json::FastWriter oJsonWriter;
	static int Init(const string& sConfFile, const string strProjectName);

public:
	TbAl_wall():AwsTable(&oTableDesc)
	{
		Reset();
	}
	~TbAl_wall()
	{}
	void Reset()
	{
		m_nAlid = 0;
		m_nWall_id = 0;
		m_nTime = 0;
		m_nAlpos = 0;
		m_nUid = 0;
		m_sUin = "";
		m_sContent = "";
		m_nAvatar = 0;
		m_nTopflag = 0;
		m_nToptime = 0;
		m_nRaw_lang = 0;
		m_sTranslate_content = "";
		m_nSeq = 0;
		ClearFlag();
	};
	const TINT64& Get_Alid()
	{
		return m_nAlid;
	}
	void Set_Alid(const TINT64& nAlid)
	{
		m_nAlid = nAlid;
	}
	const TINT64& Get_Wall_id()
	{
		return m_nWall_id;
	}
	void Set_Wall_id(const TINT64& nWall_id)
	{
		m_nWall_id = nWall_id;
	}
	const TINT64& Get_Time()
	{
		return m_nTime;
	}
	void Set_Time(const TINT64& nTime, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nTime = nTime;
		m_mFlag[TbAL_WALL_FIELD_TIME] = dwActionType;
	}
	const TINT64& Get_Alpos()
	{
		return m_nAlpos;
	}
	void Set_Alpos(const TINT64& nAlpos, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nAlpos = nAlpos;
		m_mFlag[TbAL_WALL_FIELD_ALPOS] = dwActionType;
	}
	const TINT64& Get_Uid()
	{
		return m_nUid;
	}
	void Set_Uid(const TINT64& nUid, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nUid = nUid;
		m_mFlag[TbAL_WALL_FIELD_UID] = dwActionType;
	}
	const string& Get_Uin()
	{
		return m_sUin;
	}
	void Set_Uin(const string& sUin)
	{
		m_sUin = sUin;
		m_mFlag[TbAL_WALL_FIELD_UIN] = sUin.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const string& Get_Content()
	{
		return m_sContent;
	}
	void Set_Content(const string& sContent)
	{
		m_sContent = sContent;
		m_mFlag[TbAL_WALL_FIELD_CONTENT] = sContent.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Avatar()
	{
		return m_nAvatar;
	}
	void Set_Avatar(const TINT64& nAvatar, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nAvatar = nAvatar;
		m_mFlag[TbAL_WALL_FIELD_AVATAR] = dwActionType;
	}
	const TINT64& Get_Topflag()
	{
		return m_nTopflag;
	}
	void Set_Topflag(const TINT64& nTopflag, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nTopflag = nTopflag;
		m_mFlag[TbAL_WALL_FIELD_TOPFLAG] = dwActionType;
	}
	const TINT64& Get_Toptime()
	{
		return m_nToptime;
	}
	void Set_Toptime(const TINT64& nToptime, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nToptime = nToptime;
		m_mFlag[TbAL_WALL_FIELD_TOPTIME] = dwActionType;
	}
	const TINT64& Get_Raw_lang()
	{
		return m_nRaw_lang;
	}
	void Set_Raw_lang(const TINT64& nRaw_lang, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nRaw_lang = nRaw_lang;
		m_mFlag[TbAL_WALL_FIELD_RAW_LANG] = dwActionType;
	}
	const string& Get_Translate_content()
	{
		return m_sTranslate_content;
	}
	void Set_Translate_content(const string& sTranslate_content)
	{
		m_sTranslate_content = sTranslate_content;
		m_mFlag[TbAL_WALL_FIELD_TRANSLATE_CONTENT] = sTranslate_content.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Seq()
	{
		return m_nSeq;
	}
	void Set_Seq(const TINT64& nSeq, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSeq = nSeq;
		m_mFlag[TbAL_WALL_FIELD_SEQ] = dwActionType;
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

