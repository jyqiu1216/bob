#ifndef _AWS_TABLE_EQUIP_H_
#define _AWS_TABLE_EQUIP_H_

#include "aws_table_common.h"
#include "field_property.h"

#define EN_AWS_TABLE_EQUIP "equip"

enum ETbEquipField
{
	TbEQUIP_FIELD_UID = 0,
	TbEQUIP_FIELD_ID = 1,
	TbEQUIP_FIELD_EQUIP_TYPE = 2,
	TbEQUIP_FIELD_STATUS = 3,
	TbEQUIP_FIELD_PUT_ON_TIME = 4,
	TbEQUIP_FIELD_CRYSTAL = 5,
	TbEQUIP_FIELD_BUFF = 6,
	TbEQUIP_FIELD_PUT_ON_POS = 7,
	TbEQUIP_FIELD_MISTERY_BUFF = 8,
	TbEQUIP_FIELD_EQUIP_LV = 9,
	TbEQUIP_FIELD_GET_TIME = 10,
	TbEQUIP_FIELD_SEQ = 11,
};

enum ETbEquipOpenType
{
	ETbEQUIP_OPEN_TYPE_PRIMARY = 0,
	ETbEQUIP_OPEN_TYPE_STATUS = 1,
};

#define	TBEQUIP_CRYSTAL_MAX_NUM	1
typedef	BinBuf<SCrystal, TBEQUIP_CRYSTAL_MAX_NUM>		TbEquip_Crystal;

class TbEquip : public AwsTable
{
public:
	TINT64 m_nUid;
	TINT64 m_nId;
	TINT64 m_nEquip_type;
	TINT64 m_nStatus;
	TINT64 m_nPut_on_time;
	TbEquip_Crystal m_bCrystal;
	Json::Value m_jBuff;
	TINT64 m_nPut_on_pos;
	Json::Value m_jMistery_buff;
	TINT64 m_nEquip_lv;
	TINT64 m_nGet_time;
	TINT64 m_nSeq;

public:
	static TableDesc oTableDesc;
	Json::Reader oJsonReader;
	Json::FastWriter oJsonWriter;
	static int Init(const string& sConfFile, const string strProjectName);

public:
	TbEquip():AwsTable(&oTableDesc)
	{
		Reset();
	}
	~TbEquip()
	{}
	void Reset()
	{
		m_nUid = 0;
		m_nId = 0;
		m_nEquip_type = 0;
		m_nStatus = 0;
		m_nPut_on_time = 0;
		m_bCrystal.Reset();
		m_jBuff.clear();
		m_nPut_on_pos = 0;
		m_jMistery_buff.clear();
		m_nEquip_lv = 0;
		m_nGet_time = 0;
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
	const TINT64& Get_Id()
	{
		return m_nId;
	}
	void Set_Id(const TINT64& nId)
	{
		m_nId = nId;
	}
	const TINT64& Get_Equip_type()
	{
		return m_nEquip_type;
	}
	void Set_Equip_type(const TINT64& nEquip_type, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nEquip_type = nEquip_type;
		m_mFlag[TbEQUIP_FIELD_EQUIP_TYPE] = dwActionType;
	}
	const TINT64& Get_Status()
	{
		return m_nStatus;
	}
	void Set_Status(const TINT64& nStatus, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nStatus = nStatus;
		m_mFlag[TbEQUIP_FIELD_STATUS] = dwActionType;
	}
	const TINT64& Get_Put_on_time()
	{
		return m_nPut_on_time;
	}
	void Set_Put_on_time(const TINT64& nPut_on_time, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nPut_on_time = nPut_on_time;
		m_mFlag[TbEQUIP_FIELD_PUT_ON_TIME] = dwActionType;
	}
	const Json::Value& Get_Buff()
	{
		return m_jBuff;
	}
	void Set_Buff(const Json::Value& jBuff)
	{
		m_jBuff = jBuff;
		m_mFlag[TbEQUIP_FIELD_BUFF] = jBuff.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Put_on_pos()
	{
		return m_nPut_on_pos;
	}
	void Set_Put_on_pos(const TINT64& nPut_on_pos, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nPut_on_pos = nPut_on_pos;
		m_mFlag[TbEQUIP_FIELD_PUT_ON_POS] = dwActionType;
	}
	const Json::Value& Get_Mistery_buff()
	{
		return m_jMistery_buff;
	}
	void Set_Mistery_buff(const Json::Value& jMistery_buff)
	{
		m_jMistery_buff = jMistery_buff;
		m_mFlag[TbEQUIP_FIELD_MISTERY_BUFF] = jMistery_buff.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Equip_lv()
	{
		return m_nEquip_lv;
	}
	void Set_Equip_lv(const TINT64& nEquip_lv, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nEquip_lv = nEquip_lv;
		m_mFlag[TbEQUIP_FIELD_EQUIP_LV] = dwActionType;
	}
	const TINT64& Get_Get_time()
	{
		return m_nGet_time;
	}
	void Set_Get_time(const TINT64& nGet_time, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nGet_time = nGet_time;
		m_mFlag[TbEQUIP_FIELD_GET_TIME] = dwActionType;
	}
	const TINT64& Get_Seq()
	{
		return m_nSeq;
	}
	void Set_Seq(const TINT64& nSeq, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSeq = nSeq;
		m_mFlag[TbEQUIP_FIELD_SEQ] = dwActionType;
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

