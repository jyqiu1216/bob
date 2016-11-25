/************************************************************
 * Copyright (C), 1998-2009, Tencent Technology Company Limited
 *
 * 文件名称: FieldProperty.h
 * 作者: allenhuang
 * 日期: 2009/04/23
 * 版本: 1.0
 * 模块描述: 字段配置信息
 * 组成类:
 *      CFieldProperty
 * 修改历史:
 *      <author>        <time>       <version >   <desc>
 *      allenhuang   2009/04/23          1.0        创建
 *************************************************************/

#ifndef __KINGDOM_FIELD_PROPERTY_H__
#define __KINGDOM_FIELD_PROPERTY_H__

#include "base/common/wtse_std_header.h"
#include "base/conf/tinyxml.h"
#include "aws_table_common.h"

#pragma pack(1)
// 最大字段数目
#define        MAX_FIELD_NUM    (128)
// 定义最大别名数组
#define        MAX_ALIAS_NUM    (1 << 16)
// 定义最大的开启方式数目
#define			MAX_OPEN_TYPE_NUM	(10)
// 定义最大的联合索引数目
#define			MAX_UNION_INDEX_FLD_NUM	(10)
// 定义最大的本地二次索引数目
#define			MAX_LOCAL_SEC_INDEX_NUM	(5)
#define			MAX_GLOBAL_SEC_INDEX_NUM	(5)
// 定义本地二次索引最大的映射字段数目
#define			MAX_LOCAL_PROJECTION_FIELD_NUM	(20)

#define         MAX_OPEN_TYPE_ENUM_STR_LEN  (64)

//////////////////////////////////////////////////////////////////////////
// 数值类型
enum ENumType
{
    NUM_TYPE_UINT8 = 1,         // TUINT8
    NUM_TYPE_UINT16 = 2,        // TUINT16
    NUM_TYPE_UINT32 = 4,        // TUINT32
    NUM_TYPE_UINT64 = 8,        // TUINT64
    NUM_TYPE_MAX                // 数值类型最大值
};
// 数值类型对应的配置信息
static const TCHAR g_aaszNumTypeStr[][16] =
{
    "",                 // 0
    "TUINT8",           // 1
    "TUINT16",          // 2
    "",                 // 3
    "TUINT32",          // 4
    "",                 // 5
    "",                 // 6
    "",                 // 7
    "TUINT64"           // 8
};
//////////////////////////////////////////////////////////////////////////
// 字段属性
struct SFieldProperty
{
    // 字段编号
    TUINT16     m_uwFieldNo;
    // 字段类型
    EFieldType  m_enFieldType;
    // 数值类型,这个值只有是数值字段时有效
    ENumType    m_enNumType;
    // 字段长度
    TUINT32     m_udwFieldLen;
    // 索引方式
    TUINT8      m_ucIndexType;
	// 字段名称，应该与mysql中名称定义一致
	TCHAR		m_szTitle[64];

    // 二进制字段结构体名称
    TCHAR       m_szBinStructName[64];
    // 二进制字段结构体最大大小
    TUINT32     m_udwBinStructMaxSize;

    SFieldProperty()
    {
        m_uwFieldNo = 0;
        m_enFieldType = FIELD_TYPE_UNKNOWN;
        m_enNumType = NUM_TYPE_MAX;
        m_udwFieldLen = 0;
        m_ucIndexType = INDEX_TYPE_NONE;

		m_szTitle[0] = 0;

        m_szBinStructName[0] = 0;
        m_udwBinStructMaxSize = 0;
    }
};
//////////////////////////////////////////////////////////////////////////
// 索引引用方式
struct SOpenIndexType
{
	// 方式编号
	TUINT16	m_uwTypeNo;
	// 索引名称
	TCHAR	m_szIndexName[64];
	
	// 需要返回的字段编号
	TUINT32 m_udwRetFieldNum;
	TUINT16	m_auwRetFieldNo[MAX_FIELD_NUM];
	TUINT8	m_aucFieldRetFlag[MAX_FIELD_NUM];

    // 打开方式的别名，用于自动生成enum
    TCHAR   m_szEType[MAX_OPEN_TYPE_ENUM_STR_LEN];

	SOpenIndexType()
	{
		m_uwTypeNo = 0;
		m_szIndexName[0] = 0;
		m_udwRetFieldNum = 0;
        m_szEType[0] = 0;
		memset((char*)m_aucFieldRetFlag, 0, sizeof(TUINT8)*MAX_FIELD_NUM);
	}
};

// 本地二级索引信息
struct SLocalIndexType
{
	// 方式编号
	TUINT16	m_uwTypeNo;
	// 索引名称
	TCHAR	m_szIndexName[64];
	// 目标索引字段编号
	TUINT16 m_uwIndexField;

	// 字段映射方式
	TUINT8	m_ucProjectionType;
	// 字段映射列表
	TUINT32	m_udwProjectionFieldNum;
	TUINT16	m_auwProjectionFieldList[MAX_LOCAL_PROJECTION_FIELD_NUM];

	SLocalIndexType()
	{
		m_uwTypeNo = 0;
		m_szIndexName[0] = 0;
		m_uwIndexField = 0;
		m_ucProjectionType = LOCAL_PROJECTION_TYPE__KEYS_ONLY;
		m_udwProjectionFieldNum = 0;
	}
};

// 全局二级索引信息
struct SGlobalIndexType
{
    // 方式编号
    TUINT16	m_uwTypeNo;
    // 索引名称
    TCHAR	m_szIndexName[64];
    // 目标索引字段编号
    TINT16 m_wHashIndexField;
    TINT16 m_wRangeIndexField;

    // 字段映射方式
    TUINT8	m_ucProjectionType;
    // 字段映射列表
    TUINT32	m_udwProjectionFieldNum;
    TUINT16	m_auwProjectionFieldList[MAX_LOCAL_PROJECTION_FIELD_NUM];

    // index的读写阈值
    TUINT32				m_udwReadUnits;
    TUINT32				m_udwWriteUnits;

    SGlobalIndexType()
    {
        m_uwTypeNo = 0;
        m_szIndexName[0] = 0;
        m_wHashIndexField = -1;
        m_wRangeIndexField = -1;
        m_ucProjectionType = LOCAL_PROJECTION_TYPE__KEYS_ONLY;
        m_udwProjectionFieldNum = 0;
        m_udwReadUnits = 0;
        m_udwWriteUnits = 0;
    }
};

//////////////////////////////////////////////////////////////////////////
/************************************************************************
  功能描述：
  该模块提供字段配置信息的读取。
 ************************************************************************/
class CFieldProperty
{
public:
    CFieldProperty();
    ~CFieldProperty();
public:
    /************************************************************************
      功能：
      初始化函数
      输入：
		szConfFile        :配置文件
		输出：
		TRUE    ：成功
		FALSE   ：失败
		Example：

     ************************************************************************/
    TBOOL        Init( const char* szConfFile );
    /************************************************************************
      功能：
      释放资源
      输入：

      输出：
      TRUE    ：成功
      FALSE   ：失败
      Example：

     ************************************************************************/
    TBOOL        Release();
    /************************************************************************
     * 功能：
     *  获取指定字段信息
     * 输入：
     *  uwFieldNo        :字段编号
     *  pstField        ：字段属性
     * 输出：
     *  TRUE    ：成功
     *  FALSE   ：失败
     * Example：
     ***********************************************************************/
    TBOOL        GetFieldInfo( TUINT16 uwFieldNo, SFieldProperty* &pstField );

	/************************************************************************
     * 功能：
     *  获取指定打开索引方式的信息
     * 输入：
     *  uwType				:方式编号
     *  pstOpenType         :方式属性
     * 输出：
     *  TRUE    ：成功
     *  FALSE   ：失败
     * Example：
     ***********************************************************************/
    TBOOL        GetOpenTypeInfo( TUINT16 uwType, SOpenIndexType* &pstOpenType );
   
    /************************************************************************
     * 功能：
     *  获取所有字段属性
     * 输入：
     *  ppstField        ：字段属性列表
     *  uwFieldNum        ：字段数目
     * 输出：
     *  TRUE    ：成功
     *  FALSE   ：失败
     * Example：
     ************************************************************************/
    TBOOL        GetAllField( SFieldProperty** &ppstField,TUINT16& uwFieldNum );
    /************************************************************************
     * 功能：
     *  获取字段别名对应的字段编号
     * 输入：
     *  pszAlias        ：字段别名
     * 输出：
     *  字段编号
     * Example：
     ************************************************************************/
    TUINT16        AliasToFieldNo( const TCHAR* pszAlias );
public:
    // 装载字段列表
    TBOOL        LoadFieldList( TiXmlHandle *pDocConf );
	// 装载方式列表
	TBOOL		 LoadOpenTypeList( TiXmlHandle *pDocConf );
	// 装载本地二级索引列表
	TBOOL		 LoadLocalIndexList( TiXmlHandle *pDocConf );
    // 装载Global二级索引列表
    TBOOL		 LoadGlobalIndexList( TiXmlHandle *pDocConf );
public:
    // 所有字段
    TUINT16                m_uwAllFieldNum;
    SFieldProperty*        m_pastAllFieldProperty[MAX_FIELD_NUM];

	// 所有的open_index类型
	TUINT16					m_uwALLOpenTypeNum;
	SOpenIndexType*			m_pastAllOpenType[MAX_OPEN_TYPE_NUM];
    
    // 别名对应的字段编号
    TUINT16*            m_puwAliasToFieldNo;

	// table name
	TCHAR				m_szTableName[64];
	TUINT32				m_udwReadUnits;
	TUINT32				m_udwWriteUnits;
	TUINT8				m_ucMutleTableFlag;
    TUINT32             m_udwTableType;
    TUINT32             m_udwDependFldNo;
    TUINT32             m_udwSplitSize;

	//andy@20160527: new add for cache module
	TUINT32				m_udwCacheMode;
	TUINT32				m_udwRouteFld;
	TUINT64				m_uddwRouteThrd;

	// hash_key
	SFieldProperty*			m_pstHashKey;
	SFieldProperty*			m_pstRangeKey;

	// local index
	TUINT16					m_uwLocalIndexNum;
	SLocalIndexType*		m_pastLocalIndex[MAX_LOCAL_SEC_INDEX_NUM];

    // global index
    TUINT16					m_uwGlobalIndexNum;
    SGlobalIndexType*		m_pastGlobalIndex[MAX_GLOBAL_SEC_INDEX_NUM];

    // 记录索引过的字段值
    map<unsigned int, int> m_HashFldMap;
    map<unsigned int, int> m_RangeFldMap;

public:
	int	GetTableInfo(TableDesc *poTableInfo);
};

#pragma pack()

#endif

