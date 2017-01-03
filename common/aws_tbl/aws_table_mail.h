#ifndef _AWS_TABLE_MAIL_H_
#define _AWS_TABLE_MAIL_H_

#include "aws_table_common.h"
#include "field_property.h"

#define EN_AWS_TABLE_MAIL "mail"

enum ETbMailField
{
	TbMAIL_FIELD_ID = 0,
	TbMAIL_FIELD_TIME = 1,
	TbMAIL_FIELD_SUID = 2,
	TbMAIL_FIELD_SENDER = 3,
	TbMAIL_FIELD_SCID = 4,
	TbMAIL_FIELD_RUID = 5,
	TbMAIL_FIELD_SEND_TYPE = 6,
	TbMAIL_FIELD_TITLE = 7,
	TbMAIL_FIELD_CONTENT = 8,
	TbMAIL_FIELD_LINK = 9,
	TbMAIL_FIELD_URL = 10,
	TbMAIL_FIELD_MAILDOCID = 11,
	TbMAIL_FIELD_DISPLAY_TYPE = 12,
	TbMAIL_FIELD_ESCS = 13,
	TbMAIL_FIELD_SENDER_AL_NICK = 14,
	TbMAIL_FIELD_SENDER_AL_INFO = 15,
	TbMAIL_FIELD_REWARD = 16,
	TbMAIL_FIELD_SENDER_PLAYER_AVATAR = 17,
	TbMAIL_FIELD_RAW_LANG = 18,
	TbMAIL_FIELD_TRANSLATE_CONTENT = 19,
	TbMAIL_FIELD_SEQ = 20,
	TbMAIL_FIELD_JMP_TYPE = 21,
	TbMAIL_FIELD_EVENT_ID = 22,
	TbMAIL_FIELD_EVENT_TYPE = 23,
	TbMAIL_FIELD_EVENT_SCORE = 24,
	TbMAIL_FIELD_EX_REWARD = 25,
};

enum ETbMailOpenType
{
	ETbMAIL_OPEN_TYPE_PRIMARY = 0,
};

#define	TBMAIL_REWARD_MAX_NUM	1
typedef	BinBuf<SGlobalRes, TBMAIL_REWARD_MAX_NUM>		TbMail_Reward;

#define	TBMAIL_EX_REWARD_MAX_NUM	100
typedef	BinBuf<SOneGlobalRes, TBMAIL_EX_REWARD_MAX_NUM>		TbMail_Ex_reward;

class TbMail : public AwsTable
{
public:
	TINT64 m_nId;
	TINT64 m_nTime;
	TINT64 m_nSuid;
	string m_sSender;
	TINT64 m_nScid;
	TINT64 m_nRuid;
	TINT64 m_nSend_type;
	string m_sTitle;
	string m_sContent;
	TINT64 m_nLink;
	string m_sUrl;
	TINT64 m_nMaildocid;
	TINT64 m_nDisplay_type;
	string m_sEscs;
	string m_sSender_al_nick;
	string m_sSender_al_info;
	TbMail_Reward m_bReward;
	TINT64 m_nSender_player_avatar;
	TINT64 m_nRaw_lang;
	string m_sTranslate_content;
	TINT64 m_nSeq;
	TINT64 m_nJmp_type;
	TINT64 m_nEvent_id;
	TINT64 m_nEvent_type;
	TINT64 m_nEvent_score;
	TbMail_Ex_reward m_bEx_reward;

public:
	static TableDesc oTableDesc;
	Json::Reader oJsonReader;
	Json::FastWriter oJsonWriter;
	static int Init(const string& sConfFile, const string strProjectName);

public:
	TbMail():AwsTable(&oTableDesc)
	{
		Reset();
	}
	~TbMail()
	{}
	void Reset()
	{
		m_nId = 0;
		m_nTime = 0;
		m_nSuid = 0;
		m_sSender = "";
		m_nScid = 0;
		m_nRuid = 0;
		m_nSend_type = 0;
		m_sTitle = "";
		m_sContent = "";
		m_nLink = 0;
		m_sUrl = "";
		m_nMaildocid = 0;
		m_nDisplay_type = 0;
		m_sEscs = "";
		m_sSender_al_nick = "";
		m_sSender_al_info = "";
		m_bReward.Reset();
		m_nSender_player_avatar = 0;
		m_nRaw_lang = 0;
		m_sTranslate_content = "";
		m_nSeq = 0;
		m_nJmp_type = 0;
		m_nEvent_id = 0;
		m_nEvent_type = 0;
		m_nEvent_score = 0;
		m_bEx_reward.Reset();
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
	const TINT64& Get_Time()
	{
		return m_nTime;
	}
	void Set_Time(const TINT64& nTime, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nTime = nTime;
		m_mFlag[TbMAIL_FIELD_TIME] = dwActionType;
	}
	const TINT64& Get_Suid()
	{
		return m_nSuid;
	}
	void Set_Suid(const TINT64& nSuid, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSuid = nSuid;
		m_mFlag[TbMAIL_FIELD_SUID] = dwActionType;
	}
	const string& Get_Sender()
	{
		return m_sSender;
	}
	void Set_Sender(const string& sSender)
	{
		m_sSender = sSender;
		m_mFlag[TbMAIL_FIELD_SENDER] = sSender.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Scid()
	{
		return m_nScid;
	}
	void Set_Scid(const TINT64& nScid, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nScid = nScid;
		m_mFlag[TbMAIL_FIELD_SCID] = dwActionType;
	}
	const TINT64& Get_Ruid()
	{
		return m_nRuid;
	}
	void Set_Ruid(const TINT64& nRuid, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nRuid = nRuid;
		m_mFlag[TbMAIL_FIELD_RUID] = dwActionType;
	}
	const TINT64& Get_Send_type()
	{
		return m_nSend_type;
	}
	void Set_Send_type(const TINT64& nSend_type, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSend_type = nSend_type;
		m_mFlag[TbMAIL_FIELD_SEND_TYPE] = dwActionType;
	}
	const string& Get_Title()
	{
		return m_sTitle;
	}
	void Set_Title(const string& sTitle)
	{
		m_sTitle = sTitle;
		m_mFlag[TbMAIL_FIELD_TITLE] = sTitle.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const string& Get_Content()
	{
		return m_sContent;
	}
	void Set_Content(const string& sContent)
	{
		m_sContent = sContent;
		m_mFlag[TbMAIL_FIELD_CONTENT] = sContent.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Link()
	{
		return m_nLink;
	}
	void Set_Link(const TINT64& nLink, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nLink = nLink;
		m_mFlag[TbMAIL_FIELD_LINK] = dwActionType;
	}
	const string& Get_Url()
	{
		return m_sUrl;
	}
	void Set_Url(const string& sUrl)
	{
		m_sUrl = sUrl;
		m_mFlag[TbMAIL_FIELD_URL] = sUrl.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Maildocid()
	{
		return m_nMaildocid;
	}
	void Set_Maildocid(const TINT64& nMaildocid, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nMaildocid = nMaildocid;
		m_mFlag[TbMAIL_FIELD_MAILDOCID] = dwActionType;
	}
	const TINT64& Get_Display_type()
	{
		return m_nDisplay_type;
	}
	void Set_Display_type(const TINT64& nDisplay_type, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nDisplay_type = nDisplay_type;
		m_mFlag[TbMAIL_FIELD_DISPLAY_TYPE] = dwActionType;
	}
	const string& Get_Escs()
	{
		return m_sEscs;
	}
	void Set_Escs(const string& sEscs)
	{
		m_sEscs = sEscs;
		m_mFlag[TbMAIL_FIELD_ESCS] = sEscs.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const string& Get_Sender_al_nick()
	{
		return m_sSender_al_nick;
	}
	void Set_Sender_al_nick(const string& sSender_al_nick)
	{
		m_sSender_al_nick = sSender_al_nick;
		m_mFlag[TbMAIL_FIELD_SENDER_AL_NICK] = sSender_al_nick.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const string& Get_Sender_al_info()
	{
		return m_sSender_al_info;
	}
	void Set_Sender_al_info(const string& sSender_al_info)
	{
		m_sSender_al_info = sSender_al_info;
		m_mFlag[TbMAIL_FIELD_SENDER_AL_INFO] = sSender_al_info.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Sender_player_avatar()
	{
		return m_nSender_player_avatar;
	}
	void Set_Sender_player_avatar(const TINT64& nSender_player_avatar, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSender_player_avatar = nSender_player_avatar;
		m_mFlag[TbMAIL_FIELD_SENDER_PLAYER_AVATAR] = dwActionType;
	}
	const TINT64& Get_Raw_lang()
	{
		return m_nRaw_lang;
	}
	void Set_Raw_lang(const TINT64& nRaw_lang, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nRaw_lang = nRaw_lang;
		m_mFlag[TbMAIL_FIELD_RAW_LANG] = dwActionType;
	}
	const string& Get_Translate_content()
	{
		return m_sTranslate_content;
	}
	void Set_Translate_content(const string& sTranslate_content)
	{
		m_sTranslate_content = sTranslate_content;
		m_mFlag[TbMAIL_FIELD_TRANSLATE_CONTENT] = sTranslate_content.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Seq()
	{
		return m_nSeq;
	}
	void Set_Seq(const TINT64& nSeq, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSeq = nSeq;
		m_mFlag[TbMAIL_FIELD_SEQ] = dwActionType;
	}
	const TINT64& Get_Jmp_type()
	{
		return m_nJmp_type;
	}
	void Set_Jmp_type(const TINT64& nJmp_type, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nJmp_type = nJmp_type;
		m_mFlag[TbMAIL_FIELD_JMP_TYPE] = dwActionType;
	}
	const TINT64& Get_Event_id()
	{
		return m_nEvent_id;
	}
	void Set_Event_id(const TINT64& nEvent_id, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nEvent_id = nEvent_id;
		m_mFlag[TbMAIL_FIELD_EVENT_ID] = dwActionType;
	}
	const TINT64& Get_Event_type()
	{
		return m_nEvent_type;
	}
	void Set_Event_type(const TINT64& nEvent_type, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nEvent_type = nEvent_type;
		m_mFlag[TbMAIL_FIELD_EVENT_TYPE] = dwActionType;
	}
	const TINT64& Get_Event_score()
	{
		return m_nEvent_score;
	}
	void Set_Event_score(const TINT64& nEvent_score, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nEvent_score = nEvent_score;
		m_mFlag[TbMAIL_FIELD_EVENT_SCORE] = dwActionType;
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

