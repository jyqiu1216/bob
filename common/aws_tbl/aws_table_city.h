#ifndef _AWS_TABLE_CITY_H_
#define _AWS_TABLE_CITY_H_

#include "aws_table_common.h"
#include "field_property.h"

#define EN_AWS_TABLE_CITY "city"

enum ETbCityField
{
	TbCITY_FIELD_UID = 0,
	TbCITY_FIELD_POS = 1,
	TbCITY_FIELD_NAME = 2,
	TbCITY_FIELD_RESOURCE = 3,
	TbCITY_FIELD_BUILDING = 4,
	TbCITY_FIELD_TROOP = 5,
	TbCITY_FIELD_FORT = 6,
	TbCITY_FIELD_HOS_WAIT = 7,
	TbCITY_FIELD_DEAD_FORT = 8,
	TbCITY_FIELD_RESEARCH = 9,
	TbCITY_FIELD_UTIME = 10,
	TbCITY_FIELD_UNLOCK_BLOCK = 11,
	TbCITY_FIELD_ALTAR_BUFF = 12,
	TbCITY_FIELD_ALTAR_BUFF_BTIME = 13,
	TbCITY_FIELD_ALTAR_BUFF_ETIME = 14,
	TbCITY_FIELD_ALTAR_DRAGON_LV = 15,
	TbCITY_FIELD_ALTAR_DRAGON_NAME = 16,
	TbCITY_FIELD_KNIGHT = 17,
	TbCITY_FIELD_SEQ = 18,
};

enum ETbCityOpenType
{
	ETbCITY_OPEN_TYPE_PRIMARY = 0,
};

#define	TBCITY_RESOURCE_MAX_NUM	1
typedef	BinBuf<SCommonResource, TBCITY_RESOURCE_MAX_NUM>		TbCity_Resource;

#define	TBCITY_BUILDING_MAX_NUM	500
typedef	BinBuf<SCityBuildingNode, TBCITY_BUILDING_MAX_NUM>		TbCity_Building;

#define	TBCITY_TROOP_MAX_NUM	1
typedef	BinBuf<SCommonTroop, TBCITY_TROOP_MAX_NUM>		TbCity_Troop;

#define	TBCITY_FORT_MAX_NUM	1
typedef	BinBuf<SCommonFort, TBCITY_FORT_MAX_NUM>		TbCity_Fort;

#define	TBCITY_HOS_WAIT_MAX_NUM	1
typedef	BinBuf<SCommonTroop, TBCITY_HOS_WAIT_MAX_NUM>		TbCity_Hos_wait;

#define	TBCITY_DEAD_FORT_MAX_NUM	1
typedef	BinBuf<SCommonFort, TBCITY_DEAD_FORT_MAX_NUM>		TbCity_Dead_fort;

#define	TBCITY_RESEARCH_MAX_NUM	1
typedef	BinBuf<SCommonResearch, TBCITY_RESEARCH_MAX_NUM>		TbCity_Research;

#define	TBCITY_ALTAR_BUFF_MAX_NUM	100
typedef	BinBuf<SBuffInfo, TBCITY_ALTAR_BUFF_MAX_NUM>		TbCity_Altar_buff;

#define	TBCITY_KNIGHT_MAX_NUM	30
typedef	BinBuf<SKnightInfo, TBCITY_KNIGHT_MAX_NUM>		TbCity_Knight;

class TbCity : public AwsTable
{
public:
	TINT64 m_nUid;
	TINT64 m_nPos;
	string m_sName;
	TbCity_Resource m_bResource;
	TbCity_Building m_bBuilding;
	TbCity_Troop m_bTroop;
	TbCity_Fort m_bFort;
	TbCity_Hos_wait m_bHos_wait;
	TbCity_Dead_fort m_bDead_fort;
	TbCity_Research m_bResearch;
	TINT64 m_nUtime;
	TINT64 m_nUnlock_block;
	TbCity_Altar_buff m_bAltar_buff;
	TINT64 m_nAltar_buff_btime;
	TINT64 m_nAltar_buff_etime;
	TINT64 m_nAltar_dragon_lv;
	string m_sAltar_dragon_name;
	TbCity_Knight m_bKnight;
	TINT64 m_nSeq;

public:
	static TableDesc oTableDesc;
	Json::Reader oJsonReader;
	Json::FastWriter oJsonWriter;
	static int Init(const string& sConfFile, const string strProjectName);

public:
	TbCity():AwsTable(&oTableDesc)
	{
		Reset();
	}
	~TbCity()
	{}
	void Reset()
	{
		m_nUid = 0;
		m_nPos = 0;
		m_sName = "";
		m_bResource.Reset();
		m_bBuilding.Reset();
		m_bTroop.Reset();
		m_bFort.Reset();
		m_bHos_wait.Reset();
		m_bDead_fort.Reset();
		m_bResearch.Reset();
		m_nUtime = 0;
		m_nUnlock_block = 0;
		m_bAltar_buff.Reset();
		m_nAltar_buff_btime = 0;
		m_nAltar_buff_etime = 0;
		m_nAltar_dragon_lv = 0;
		m_sAltar_dragon_name = "";
		m_bKnight.Reset();
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
	const TINT64& Get_Pos()
	{
		return m_nPos;
	}
	void Set_Pos(const TINT64& nPos, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nPos = nPos;
		m_mFlag[TbCITY_FIELD_POS] = dwActionType;
	}
	const string& Get_Name()
	{
		return m_sName;
	}
	void Set_Name(const string& sName)
	{
		m_sName = sName;
		m_mFlag[TbCITY_FIELD_NAME] = sName.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Utime()
	{
		return m_nUtime;
	}
	void Set_Utime(const TINT64& nUtime, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nUtime = nUtime;
		m_mFlag[TbCITY_FIELD_UTIME] = dwActionType;
	}
	const TINT64& Get_Unlock_block()
	{
		return m_nUnlock_block;
	}
	void Set_Unlock_block(const TINT64& nUnlock_block, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nUnlock_block = nUnlock_block;
		m_mFlag[TbCITY_FIELD_UNLOCK_BLOCK] = dwActionType;
	}
	const TINT64& Get_Altar_buff_btime()
	{
		return m_nAltar_buff_btime;
	}
	void Set_Altar_buff_btime(const TINT64& nAltar_buff_btime, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nAltar_buff_btime = nAltar_buff_btime;
		m_mFlag[TbCITY_FIELD_ALTAR_BUFF_BTIME] = dwActionType;
	}
	const TINT64& Get_Altar_buff_etime()
	{
		return m_nAltar_buff_etime;
	}
	void Set_Altar_buff_etime(const TINT64& nAltar_buff_etime, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nAltar_buff_etime = nAltar_buff_etime;
		m_mFlag[TbCITY_FIELD_ALTAR_BUFF_ETIME] = dwActionType;
	}
	const TINT64& Get_Altar_dragon_lv()
	{
		return m_nAltar_dragon_lv;
	}
	void Set_Altar_dragon_lv(const TINT64& nAltar_dragon_lv, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nAltar_dragon_lv = nAltar_dragon_lv;
		m_mFlag[TbCITY_FIELD_ALTAR_DRAGON_LV] = dwActionType;
	}
	const string& Get_Altar_dragon_name()
	{
		return m_sAltar_dragon_name;
	}
	void Set_Altar_dragon_name(const string& sAltar_dragon_name)
	{
		m_sAltar_dragon_name = sAltar_dragon_name;
		m_mFlag[TbCITY_FIELD_ALTAR_DRAGON_NAME] = sAltar_dragon_name.empty() ? UPDATE_ACTION_TYPE_DELETE : UPDATE_ACTION_TYPE_PUT;
	}
	const TINT64& Get_Seq()
	{
		return m_nSeq;
	}
	void Set_Seq(const TINT64& nSeq, int dwActionType=UPDATE_ACTION_TYPE_PUT)
	{
		m_nSeq = nSeq;
		m_mFlag[TbCITY_FIELD_SEQ] = dwActionType;
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

