#ifndef _AWS_TABLE_COMMON_H_
#define _AWS_TABLE_COMMON_H_

#include <string>
#include <vector>
#include <map>
#include <set>
#include <sstream>
#include <assert.h>
#include "util_base64.h"
#include "jsoncpp/json/json.h"
#include "aws_struct.h"
#include "aws_consume_info.h"
#include "base/common/wtse_std_header.h"
#include "game_define.h"

#ifndef TOOL
#include "bin_struct_define.h"
#endif


using namespace std;

typedef __int64_t               TINT64;

#undef  INT64_MIN
#undef  INT64_MAX

#define INT64_MIN 0X8000000000000000

#define INT64_MAX 9223372036854775807LL

// 表类型
enum ETableType
{
    TABLE_TYPE_GLOBAL = 0,
    TABLE_TYPE_SPLIT,
    TABLE_TYPE_MONTH,
};

enum EFieldType
{
	FIELD_TYPE_UNKNOWN = 0,
	FIELD_TYPE_N,
	FIELD_TYPE_S,
	FIELD_TYPE_B,
	FIELD_TYPE_J,
	FIELD_TYPE_END,
};

// 字段类型对应的配置信息
static const char g_aaszFieldTypeStr[][16] =
{
	"other",
	"N",
	"S",
	"B",
	"J",
};

enum EIndexType
{
	INDEX_TYPE_NONE = 0,
	INDEX_TYPE_HASH_KEY,
	INDEX_TYPE_RANGE_KEY,
	INDEX_TYPE_LOCAL_INDEX,
};

static const char g_aaszIndexType[][16] = 
{
	"",
	"HASH",
	"RANGE",
	"RANGE",
};

enum ECompareType
{
	//COMPARE_TYPE_UNKNOWN,
	COMPARE_TYPE_EQ,
	COMPARE_TYPE_LE,
	COMPARE_TYPE_LT,
	COMPARE_TYPE_GE,
	COMPARE_TYPE_GT,
	COMPARE_TYPE_BEGINS_WITH, //暂不支持
	COMPARE_TYPE_BETWEEN, 
};

enum EUpdateActionType
{
	UPDATE_ACTION_TYPE_PUT = 1,
	UPDATE_ACTION_TYPE_ADD,
    UPDATE_ACTION_TYPE_DELETE,
};

enum ELocalProjectionType
{
	LOCAL_PROJECTION_TYPE__KEYS_ONLY = 0,
	LOCAL_PROJECTION_TYPE__INCLUDE,
	LOCAL_PROJECTION_TYPE__ALL,
	LOCAL_PROJECTION_TYPE__END,
};

static const char g_aaLocalProjectionType[][16] = 
{
	"KEYS_ONLY",
	"INCLUDE",
	"ALL",
};

enum EWriteActionType
{
	WRITE_ACTION_TYPE_PUT,
	WRITE_ACTION_TYPE_DELETE,
};

enum EReturnValuesType
{
	RETURN_VALUES_NONE,
	RETURN_VALUES_ALL_OLD,
	RETURN_VALUES_UPDATED_OLD,
	RETURN_VALUES_ALL_NEW,
	RETURN_VALUES_UPDATED_NEW,
};

struct FieldDesc //字段描述
{
	unsigned int udwFldNo; //字段no,
	string sName; //字段名
	int dwType; //数据类型
	int dwIndexType; //索引类型(HASH KEY|RANGE KEY|LOCAL INDEX)
	unsigned int udwMaxLen;

    string sBinStructName;
    int dwBinStructMaxNum;

	FieldDesc()
	{
		Reset();
	}
	FieldDesc(unsigned int udwFldNo, const string& sName, int dwType, int dwIndexType, unsigned int udwMaxLen = 4096, const string sBinStructName = "", int dwBinStructMaxNum = 0)
	{
		this->udwFldNo = udwFldNo;
		this->sName = sName;
		this->dwType = dwType;
		this->dwIndexType = dwIndexType;
		this->udwMaxLen = udwMaxLen;
        this->sBinStructName = sBinStructName;
        this->dwBinStructMaxNum = dwBinStructMaxNum;
	}
	void Reset()
	{
		udwFldNo = (unsigned int)-1;
		sName = "";
		dwType = FIELD_TYPE_UNKNOWN;
		dwIndexType = INDEX_TYPE_NONE;
		udwMaxLen = 4096;
        sBinStructName = "";
        dwBinStructMaxNum = 0;
	}
};

struct IndexDesc //索引描述
{
	unsigned int udwIdxNo; //索引no, 这个是key
	string sName; //索引名称
    string sEnumName; //生成索引枚举量的名字
	vector<unsigned int> vecIdxFld; //hash key或者hash key+range key 或者hash key + local index
	vector<unsigned int> vecRtnFld; //vecRtnFld为空时表示返回全部
	IndexDesc()
	{
		Reset();
	}
	void Reset()
	{
		udwIdxNo = (unsigned int)-1;
		sName = "";
		vecIdxFld.clear();
		vecRtnFld.clear();
	}
};

struct TableDesc //表描述
{
    string m_strProjectName; // 该表的项目名
	string sName; //表名(这里是类名,与实际存储到云平台的表名有差异)
    unsigned int udwTableType;
    unsigned int udwDependFld;
    unsigned int udwSplitSize;
	bool bMultiple; //是否有多张表(mail和report按时间分表)
	unsigned int udwHashKeyFldNo; //HASH KEY
	unsigned int udwRangeKeyFldNo; //RANGE KEY
	map<unsigned int, int> mIndexFldNo;
	map<unsigned int, FieldDesc> mFieldDesc; //FldNo<->FieldDesc
	map<unsigned int, IndexDesc> mIndexDesc; //IdxNo<->IndexDesc

    map<unsigned int, int> mHashFldMap;
    map<unsigned int, int> mRangeFldMap;


	//andy@20160527: new add for cache module
	TUINT32		udwCacheMode;
	TUINT32		udwRouteFld;
	TUINT64		uddwRouteThrd;

	TableDesc()
	{
		Reset();
	}
	void Reset()
	{
	    m_strProjectName = "";
		sName = "";
        udwTableType = TABLE_TYPE_GLOBAL;
        udwDependFld = 0;
        udwSplitSize = 1;
		bMultiple = false;
		udwHashKeyFldNo = (unsigned int)-1;
		udwRangeKeyFldNo = (unsigned int)-1;
		mIndexFldNo.clear();
		mFieldDesc.clear();
		mIndexDesc.clear();
        mHashFldMap.clear();
        mRangeFldMap.clear();

		//andy@20160527: new add for cache module
		udwCacheMode = 0;
		udwRouteFld = 0;
		uddwRouteThrd = 0;
	}
};


struct CompareDesc
{
	int dwCompareType;
	//--下面字段当dwCompareType为COMPARE_TYPE_BEGINS_WITH或者COMPARE_TYPE_BETWEEN时使用
	vector<string> vecS;
	//vector<string> vecB;
	vector<TINT64> vecN;
	CompareDesc()
	{
		Reset();
	}
	void Reset()
	{
		dwCompareType= COMPARE_TYPE_EQ;
		vecS.clear();
		vecN.clear();
	}
    void push_back(TINT64 nValue)
    {
        vecN.push_back(nValue);
    }
    void push_back(const string& sValue)
    {
        vecS.push_back(sValue);
    }
};

struct ExpectedItem
{
	unsigned int udwFldNo;
	//string sName;
	string sValue;
	//string bValue;
	TINT64 nValue;
	bool bExists; //bExists为false时,sValue/nValue无效
	ExpectedItem()
	{
		udwFldNo = (unsigned int)-1;
		//sName = "";
		sValue = "";
		//bValue = "";
		nValue = 0;
		bExists = false;
	}

    void SetVal(unsigned int fld, bool exist, TINT64 val = 0)
    {
        udwFldNo = fld;
        bExists = exist;
        nValue = val;
    }

    void SetVal(unsigned int fld, bool exist, string str)
    {
        udwFldNo = fld;
        bExists = exist;
        sValue = str;
    }

    ExpectedItem(unsigned int fld, bool exist, TINT64 val = 0)
    {
        udwFldNo = fld;
        bExists = exist;
        nValue = val;
        sValue = "";
    }

    ExpectedItem(unsigned int fld, bool exist, string str)
    {
        udwFldNo = fld;
        bExists = exist;
        sValue = str;
        nValue = 0;
    }
};

struct ExpectedDesc
{
	vector<ExpectedItem> vecExpectedItem;
    void push_back(const ExpectedItem& item)
    {
        vecExpectedItem.push_back(item);
    }
    void clear()
    {
        vecExpectedItem.clear();
    }
};

template<class T, unsigned int N>
class BinBuf
{
public:
	unsigned int m_udwNum;
	T            m_astList[N];

public:
	void Reset()
	{
        if (N == 1)
        {
            m_udwNum = 1;
            memset(m_astList, 0, sizeof(m_astList));
        }
        else
        {
		    m_udwNum = 0;
        }
	}
	bool empty()
	{
		return m_udwNum==0;
	}
	unsigned int size()
	{
		return m_udwNum*sizeof(T);
	}
    char* data()
    {
        return (char*)m_astList;
    }
	T& operator[](unsigned int n)
	{	
		assert(n < N);
		return m_astList[n];
	}
};

class AwsTable
{
public:
	map<unsigned int, int> m_mFlag;
	
public:
	//由表名获得类名
	//static string GetClassName(const string& sTableName, int dwSvrIdx = -1, int dwTableIdx = -1);
	//根据类名创建对象
	static AwsTable* NewObject(const string& sClassName);
	static void AddNewObjectFunc(const string& sClassName, AwsTable* (*pFunc)());
public:
	static map<string, AwsTable* (*)()> mNewObjFunc; //类名到新建对象函数的映射
public:
	TableDesc*  pTableDesc;
	AwsTable(TableDesc* pDesc) : pTableDesc(pDesc)
	{
	}
	virtual ~AwsTable()
	{}
public:
	//Scan命令,返回完整的请求包
	virtual AwsMap* OnScanReq(unsigned int udwIdxNo, bool bHasStartKey = false, bool bReturnConsumedCapacity = true,
		 unsigned int dwLimit = 0, unsigned int dwSegment = 0, unsigned int dwTotalSegments = 0)=0;
	virtual int OnScanReq(string& sPostData, unsigned int udwIdxNo, bool bHasStartKey = false, bool bReturnConsumedCapacity = true,
		unsigned int dwLimit = 0, unsigned int dwSegment = 0, unsigned int dwTotalSegments = 0)=0;
	virtual int OnScanRsp(const Json::Value& item) = 0;
	//Query命令使用,返回完整的请求包
	virtual AwsMap* OnQueryReq(unsigned int udwIdxNo, const CompareDesc& comp_desc = CompareDesc(), 
		bool bConsistentRead = true, bool bReturnConsumedCapacity = true, bool bScanIndexForward = true, unsigned int dwLimit = 0, bool bCount = false)=0;
	virtual int OnQueryReq(string& sPostData, unsigned int udwIdxNo, const CompareDesc& comp_desc = CompareDesc(), 
		bool bConsistentRead = true, bool bReturnConsumedCapacity = true, bool bScanIndexForward = true, unsigned int dwLimit = 0)=0;
	virtual int OnQueryRsp(const Json::Value& item)=0;
    //查找记录数量
    virtual int OnCountReq(string& sPostData, unsigned int udwIdxNo, const CompareDesc& comp_desc = CompareDesc(), 
        bool bConsistentRead = true, bool bReturnConsumedCapacity = true, bool bScanIndexForward = true, unsigned int dwLimit = 0)=0;
	//UpdateItem命令使用,返回完整的请求包
	virtual AwsMap* OnUpdateItemReq(
		const ExpectedDesc& expected_desc = ExpectedDesc(), int dwReturnValuesType = RETURN_VALUES_NONE, bool bReturnConsumedCapacity = true)=0;
	virtual int OnUpdateItemReq(string& sPostData,
		const ExpectedDesc& expected_desc = ExpectedDesc(), int dwReturnValuesType = RETURN_VALUES_NONE, bool bReturnConsumedCapacity = true)=0;
	virtual int OnUpdateItemRsp(const Json::Value& item)=0;
	//BatchWriteItem命令使用
	virtual AwsMap* OnWriteItemReq(int dwActionType = WRITE_ACTION_TYPE_PUT)=0;
	virtual void OnWriteItemReq(AwsMap* pWriteItem, int dwActionType = WRITE_ACTION_TYPE_PUT)=0;
	//BatchGetItem命令使用
	virtual AwsMap* OnReadItemReq(unsigned int udwIdxNo)=0;
	virtual void OnReadItemReq(AwsMap* pReadItem)=0;
	virtual int OnReadItemRsp(const Json::Value& item)=0;
	//DeleteItem命令使用,返回完整的请求包
	virtual AwsMap* OnDeleteItemReq(
		const ExpectedDesc& expected_desc = ExpectedDesc(), int dwReturnValuesType = RETURN_VALUES_NONE, bool bReturnConsumedCapacity = true)=0;
	virtual int OnDeleteItemReq(string& sPostData,
		const ExpectedDesc& expected_desc = ExpectedDesc(), int dwReturnValuesType = RETURN_VALUES_NONE, bool bReturnConsumedCapacity = true)=0;
	virtual int OnDeleteItemRsp(const Json::Value& item)=0;
	//GetItem命令使用,返回完整的请求包
	virtual AwsMap* OnGetItemReq(unsigned int udwIdxNo,
		bool bConsistentRead = true, bool bReturnConsumedCapacity = true)=0;
	virtual int OnGetItemReq(string& sPostData, unsigned int udwIdxNo,
		bool bConsistentRead = true, bool bReturnConsumedCapacity = true)=0;
	virtual int OnGetItemRsp(const Json::Value& item)=0;
	//PutItem命令使用,返回完整的请求包
	virtual AwsMap* OnPutItemReq(
		const ExpectedDesc& expected_desc = ExpectedDesc(), int dwReturnValuesType = RETURN_VALUES_NONE, bool bReturnConsumedCapacity = true)=0;
	virtual int OnPutItemReq(string& sPostData,
		const ExpectedDesc& expected_desc = ExpectedDesc(), int dwReturnValuesType = RETURN_VALUES_NONE, bool bReturnConsumedCapacity = true)=0;
	virtual int OnPutItemRsp(const Json::Value& item)=0;
	virtual int OnResponse(const Json::Value& item)=0;

    virtual TINT64 GetSeq() = 0;

public:
	static char* CompareType2Str(int  dwCompareType);
	static char* DataType2Str(int dwDataType);
	static char* UpdateActionType2Str(int dwUpdateActionType);
	static char* ReturnValuesType2Str(int dwReturnValuesType);
	static const string& Base64Encode(const string& sSrc, string& sDest);
	static const string& Base64Decode(const string& sSrc, string& sDest);

    static const string& Base64Encode(char* pszSrc, unsigned int udwSrcLen, string& sDest);
    static void Base64Decode(const string& sSrc, char* pszDest, int &udwDstLen);

    const string& JsonEncode(const string& sInput, string& sOutPut);

public:

	bool CheckFlag(unsigned int udwFldNo)//判断某个字段是否有变化(调用过Set函数)
	{
		return m_mFlag.find(udwFldNo)!=m_mFlag.end();
	}
	void ClearFlag()
	{
		m_mFlag.clear();
	}
	void SetFlag(unsigned int udwFldNo) //二进制数据由使用者自己setflag
	{
		m_mFlag[udwFldNo] = UPDATE_ACTION_TYPE_PUT;
	}
    void DeleteField(unsigned int udwFldNo)
    {
        m_mFlag[udwFldNo] = UPDATE_ACTION_TYPE_DELETE;
    }
	bool IfNeedUpdate()
	{
		return !m_mFlag.empty();
	}

public:
    static int GetTipsTableOffset(TINT64 ddwTime);
    static int GetMailReportTableOffset(TINT64 ddwId);
    virtual string GetTableName() = 0;
};


#endif

