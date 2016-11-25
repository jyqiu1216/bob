#ifndef _AWS_TABLE_BACKPACK_H_
#define _AWS_TABLE_BACKPACK_H_

#include "aws_table_common.h"
#include "field_property.h"

#define EN_AWS_TABLE_BACKPACK "backpack"

enum ETbBackpackField
{
	TbBACKPACK_FIELD_UID = 0,
	TbBACKPACK_FIELD_ITEM = 1,
	TbBACKPACK_FIELD_CRYSTAL = 2,
	TbBACKPACK_FIELD_MATERIAL = 3,
	TbBACKPACK_FIELD_SOUL = 4,
	TbBACKPACK_FIELD_PARTS = 5,
	TbBACKPACK_FIELD_SP_CRYSTAL = 6,
	TbBACKPACK_FIELD_SCROLL = 7,
	TbBACKPACK_FIELD_SCROLL_GET_TIME = 8,
	TbBACKPACK_FIELD_SEQ = 9,
};

enum ETbBackpackOpenType
{
	ETbBackpack_OPEN_TYPE_PRIMARY = 0,
};

#define	TBBACKPACK_ITEM_MAX_NUM	2000
typedef	BinBuf<SPlayerItem, TBBACKPACK_ITEM_MAX_NUM>		TbBackpack_Item;

class TbBackpack : public AwsTable
{
public:
	TINT64 m_nUid;
	TbBackpack_Item m_bItem;
	Json::Value m_jCrystal;
	Json::Value m_jMaterial;
	Json::Value m_jSoul;
	Json::Value m_jParts;
	Json::Value m_jSp_crystal;
	Json::Value m_jScroll;
	Json::Value m_jScroll_get_time;
	TINT64 m_nSeq;

public:
	static TableDesc oTableDesc;
	Json::Reader oJsonReader;
	Json::FastWriter oJsonWriter;
	static int Init(const string& sConfFile, const string strProjectName);

public:
	TbBackpack():AwsTable(&oTableDesc)
	{
		Reset();
	}
	~TbBackpack()
	{}
	void Reset()
	{
		m_nUid = 0;
		m_bItem.Reset();
		m_jCrystal.clear();
		m_jMaterial.clear();
		m_jSoul.clear();
		m_jParts.clear();
		m_jSp_crystal.clear();
		m_jScroll.clear();
		m_jScroll_get_time.clear();
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
	const Json::Value& Get_Crystal()
	{
		return m_jCrystal;
	}
	void Set_Crystal(const Json::Value& jCrystal)
	{
		m_jCrystal = jCrystal;
		m_mFlag[TbBACKPACK_FIELD_CRYSTAL] = jCrystal.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const Json::Value& Get_Material()
	{
		return m_jMaterial;
	}
	void Set_Material(const Json::Value& jMaterial)
	{
		m_jMaterial = jMaterial;
		m_mFlag[TbBACKPACK_FIELD_MATERIAL] = jMaterial.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const Json::Value& Get_Soul()
	{
		return m_jSoul;
	}
	void Set_Soul(const Json::Value& jSoul)
	{
		m_jSoul = jSoul;
		m_mFlag[TbBACKPACK_FIELD_SOUL] = jSoul.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const Json::Value& Get_Parts()
	{
		return m_jParts;
	}
	void Set_Parts(const Json::Value& jParts)
	{
		m_jParts = jParts;
		m_mFlag[TbBACKPACK_FIELD_PARTS] = jParts.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const Json::Value& Get_Sp_crystal()
	{
		return m_jSp_crystal;
	}
	void Set_Sp_crystal(const Json::Value& jSp_crystal)
	{
		m_jSp_crystal = jSp_crystal;
		m_mFlag[TbBACKPACK_FIELD_SP_CRYSTAL] = jSp_crystal.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const Json::Value& Get_Scroll()
	{
		return m_jScroll;
	}
	void Set_Scroll(const Json::Value& jScroll)
	{
		m_jScroll = jScroll;
		m_mFlag[TbBACKPACK_FIELD_SCROLL] = jScroll.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const Json::Value& Get_Scroll_get_time()
	{
		return m_jScroll_get_time;
	}
	void Set_Scroll_get_time(const Json::Value& jScroll_get_time)
	{
		m_jScroll_get_time = jScroll_get_time;
		m_mFlag[TbBACKPACK_FIELD_SCROLL_GET_TIME] = jScroll_get_time.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Seq()
	{
		return m_nSeq;
	}
	void Set_Seq(const TINT64& nSeq, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSeq = nSeq;
		m_mFlag[TbBACKPACK_FIELD_SEQ] = dwActionType;
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

