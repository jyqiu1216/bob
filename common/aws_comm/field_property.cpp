#include "field_property.h"
#include <assert.h>

CFieldProperty::CFieldProperty()
{
	m_uwAllFieldNum = 0;
	m_uwALLOpenTypeNum = 0;
	m_puwAliasToFieldNo = NULL;
	// 初始化为0
	memset(m_pastAllFieldProperty,0,sizeof(m_pastAllFieldProperty));
	memset(m_pastAllOpenType,0,sizeof(m_pastAllOpenType));
	memset(m_pastLocalIndex, 0, sizeof(m_pastLocalIndex));
    memset(m_pastGlobalIndex, 0, sizeof(m_pastGlobalIndex));
	m_pstHashKey = NULL;
	m_pstRangeKey = NULL;
	m_uwLocalIndexNum = 0;
    m_uwGlobalIndexNum = 0;

	m_szTableName[0] = 0;
	m_udwReadUnits = 0;
	m_udwWriteUnits = 0;
	m_ucMutleTableFlag = 0;
    m_udwTableType = 0;
    m_udwDependFldNo = 0;
    m_udwSplitSize = 0;

	//andy@20160527: new add for cache module
	m_udwCacheMode = 0;
	m_udwRouteFld = 0;
	m_uddwRouteThrd = 0;


    m_HashFldMap.clear();
    m_RangeFldMap.clear();
}

CFieldProperty::~CFieldProperty()
{
	this->Release();
}

TBOOL	CFieldProperty::Init( const char* szConfFile )
{
	TiXmlDocument    oXmlDoc;
	TBOOL            bRet = FALSE;
	if ( NULL == m_puwAliasToFieldNo )
	{
	    m_puwAliasToFieldNo = new TUINT16[MAX_ALIAS_NUM];
	    if ( NULL == m_puwAliasToFieldNo )
	    {
	        return FALSE;
	    }
	    memset( m_puwAliasToFieldNo,0,sizeof(TUINT16) * MAX_ALIAS_NUM);
	}

	bRet = oXmlDoc.LoadFile( szConfFile );
	// 装载配置文件
	if ( !bRet )
	{
	    return FALSE;
	}
	// 获取根节点
	TiXmlHandle oRootHandle(oXmlDoc.RootElement());

	// 获取表名称
	const TCHAR *pCur = oXmlDoc.RootElement()->Attribute("table_name");
	assert(pCur);
	strcpy(m_szTableName, pCur);
	pCur = oXmlDoc.RootElement()->Attribute("read_capacity_units");
	m_udwReadUnits = atoi(pCur);
	pCur = oXmlDoc.RootElement()->Attribute("write_capacity_units");
	m_udwWriteUnits = atoi(pCur);
    pCur = oXmlDoc.RootElement()->Attribute("table_type");
    m_udwTableType = atoi(pCur);
    pCur = oXmlDoc.RootElement()->Attribute("depend_fld");
    if(pCur)
    {
        m_udwDependFldNo = atoi(pCur);
    }
    else
    {
        m_udwDependFldNo = 0;
    }
    pCur = oXmlDoc.RootElement()->Attribute("split_size");
    if(pCur)
    {
        m_udwSplitSize = atoi(pCur);
    }
    else
    {
        m_udwSplitSize = 0;
    }

	pCur = oXmlDoc.RootElement()->Attribute("multi_flag");
	if(pCur)
	{
		m_ucMutleTableFlag = atoi(pCur);
		if(m_ucMutleTableFlag != 0)
		{
			m_ucMutleTableFlag = 1;
		}
	}
	else
	{
		m_ucMutleTableFlag = 0;
	}
	//andy@20160527: new fields for cache module
	m_udwCacheMode = 0;
	m_udwRouteFld = 0;
	m_uddwRouteThrd = 0;
	//andy@20160527:new add for cache module
	pCur = oXmlDoc.RootElement()->Attribute("cache_mode");
	if(pCur)
	{
		m_udwCacheMode = atoi(pCur);
	}
	else
	{
		m_udwCacheMode = 0;
	}
	//andy@20160527:new add for cache module
	pCur = oXmlDoc.RootElement()->Attribute("route_fld");
	if(pCur)
	{
		m_udwRouteFld = atoi(pCur);
	}
	else
	{
		m_udwRouteFld = 0;
	}
	//andy@20160527:new add for cache module
	pCur = oXmlDoc.RootElement()->Attribute("route_thrd");
	if(pCur)
	{
		m_uddwRouteThrd = strtoull(pCur, NULL, 10);
	}
	else
	{
		m_uddwRouteThrd = 0;
	}

	//////////////////////////////////////////////////////////////////////////
	// 装载所有字段列表
	bRet = LoadFieldList(&oRootHandle);
	if ( !bRet )
	{
	    return FALSE;
	}
	// 装载开启方式列表
	bRet = LoadOpenTypeList(&oRootHandle);
	if ( !bRet )
	{
	    return FALSE;
	}

	// 装载本地二级索引信息
	bRet = LoadLocalIndexList(&oRootHandle);
	if(!bRet)
	{
		return FALSE;
	}

    // 装载Global二级索引信息
    bRet = LoadGlobalIndexList(&oRootHandle);
    if(!bRet)
    {
        return FALSE;
    }

    // init map
    int idx = 0;
    m_HashFldMap.clear();
    m_RangeFldMap.clear();
    m_HashFldMap.insert(make_pair(m_pstHashKey->m_uwFieldNo, INDEX_TYPE_HASH_KEY));
    if(m_pstRangeKey)
    {
        m_RangeFldMap.insert(make_pair(m_pstRangeKey->m_uwFieldNo, INDEX_TYPE_RANGE_KEY));
    }
    for(idx = 0; idx < m_uwGlobalIndexNum; idx++)
    {
        m_HashFldMap.insert(make_pair(m_pastGlobalIndex[idx]->m_wHashIndexField, INDEX_TYPE_HASH_KEY));
        if(m_pastGlobalIndex[idx]->m_wRangeIndexField >= 0)
        {
            m_RangeFldMap.insert(make_pair(m_pastGlobalIndex[idx]->m_wRangeIndexField, INDEX_TYPE_RANGE_KEY));
        }
    }    
    for(idx = 0; idx < m_uwLocalIndexNum; idx++)
    {
        m_RangeFldMap.insert(make_pair(m_pastLocalIndex[idx]->m_uwIndexField, INDEX_TYPE_RANGE_KEY));
    }

	return TRUE;

}

// 装载字段列表
TBOOL        CFieldProperty::LoadFieldList( TiXmlHandle *pDocConf )
{
    SFieldProperty*    pstFieldProperty = NULL;
    const TCHAR*    pStr = NULL;
    TBOOL            bError = FALSE;
    TINT32            dwTmp = 0;
    TUINT16            uwIdx = 0;
    TiXmlElement *pElement, *pFieldElem;

    pElement = pDocConf->FirstChildElement("FieldList").Element();
    pFieldElem = pElement->FirstChildElement("Field");
    while (pFieldElem != NULL)
    {
        // 开辟空间
        pstFieldProperty = new SFieldProperty;
        // 初始化属性空间
        memset( pstFieldProperty,0,sizeof(SFieldProperty));
        // 装载字段编号
        if (pFieldElem->QueryIntAttribute("no", &dwTmp ) != TIXML_SUCCESS)
        {
            bError = TRUE;
        }
        pstFieldProperty->m_uwFieldNo  = dwTmp;
        // 获取字段类型
        pStr = pFieldElem->Attribute("type");
        for ( uwIdx = 0; uwIdx < FIELD_TYPE_END ; uwIdx++ )
        {
            if ( 0 == strcmp(pStr,g_aaszFieldTypeStr[uwIdx]) )
            {
                pstFieldProperty->m_enFieldType = (EFieldType)uwIdx;
                break;
            }
        }
        if ( uwIdx >= FIELD_TYPE_END )
        {
            bError = TRUE;
        }
        // 装载数值类型或者文本长度
        switch ( pstFieldProperty->m_enFieldType )
        {
        case FIELD_TYPE_S:
        case FIELD_TYPE_B:
        case FIELD_TYPE_J:
            if (pFieldElem->QueryIntAttribute("maxlen", &dwTmp ) != TIXML_SUCCESS)
            {
                bError = TRUE;
            }
            pstFieldProperty->m_udwFieldLen = dwTmp;
            break;
        case FIELD_TYPE_N:
            pStr = pFieldElem->Attribute("numtype");
            for ( uwIdx = 0; uwIdx < NUM_TYPE_MAX ; uwIdx++ )
            {
                // 判断是否无效
                if ( 0 == strcmp(g_aaszNumTypeStr[uwIdx],""))
                {
                    continue;
                }
                if ( 0 == strcmp(pStr,g_aaszNumTypeStr[uwIdx]) )
                {
                    pstFieldProperty->m_enNumType = (ENumType)uwIdx;
                    break;
                }
            }
            if ( uwIdx >= NUM_TYPE_MAX )
            {
                bError = TRUE;
            }
            break;
        case FIELD_TYPE_UNKNOWN:
            break;
	default:
		break;
        }
		/*
        // 装载字段别名
        pStr = pFieldElem->Attribute("alias");
        strcpy(pstFieldProperty->m_szAlias,pStr);
        // 设置别名对应的字段编号
        m_puwAliasToFieldNo[ *(TUINT16*)pStr ] = pstFieldProperty->m_uwFieldNo;
		*/
		// 装载字段名称
		pStr = pFieldElem->Attribute("title");
		strcpy(pstFieldProperty->m_szTitle, pStr);
		// 装载索引类型
		pStr = pFieldElem->Attribute("index");
		if(pStr)
		{
			pstFieldProperty->m_ucIndexType = atoi(pStr);
		}

        if(pstFieldProperty->m_enFieldType == FIELD_TYPE_B)
        {
            // 装载二进制字段结构体名称
            pStr = pFieldElem->Attribute("bin_struct_name");
            if(pStr)
            {
                strcpy(pstFieldProperty->m_szBinStructName, pStr);
            }
            // 装载二进制字段结构体最大大小
            pStr = pFieldElem->Attribute("bin_struct_num");
            if(pStr)
            {
                pstFieldProperty->m_udwBinStructMaxSize = atoi(pStr);
            }
            else
            {
                pstFieldProperty->m_udwBinStructMaxSize = 1;
            }

            assert(pstFieldProperty->m_udwBinStructMaxSize > 0);
            assert(pstFieldProperty->m_szBinStructName[0]);
        }        

        //////////////////////////////////////////////////////////////////////////
        // 记录字段属性
        if ( bError )
        {
            break;
        }
        m_pastAllFieldProperty[ pstFieldProperty->m_uwFieldNo ] = pstFieldProperty;
        m_uwAllFieldNum++;

		// 保存hash和range key
		if(pstFieldProperty->m_ucIndexType == INDEX_TYPE_HASH_KEY)
		{
			m_pstHashKey = pstFieldProperty;
		}
		else if(pstFieldProperty->m_ucIndexType == INDEX_TYPE_RANGE_KEY)
		{
			m_pstRangeKey = pstFieldProperty;
		}

        pFieldElem = pFieldElem->NextSiblingElement("Field");
    }
    // 出错
    if ( bError )
    {
        delete pstFieldProperty;
        pstFieldProperty = NULL;

        return FALSE;
    }

    return TRUE;
}

TBOOL        CFieldProperty::Release()
{
	TUINT16 uwIdx = 0;

    // 释放内存
    for ( uwIdx = 0; uwIdx < MAX_FIELD_NUM; uwIdx++ )
    {
        if ( NULL != m_pastAllFieldProperty[uwIdx])
        {
            delete m_pastAllFieldProperty[uwIdx];
        }
    }
    //
    if ( NULL != m_puwAliasToFieldNo )
    {
        delete[] m_puwAliasToFieldNo;
        m_puwAliasToFieldNo = NULL;
    }
	// 
	for ( uwIdx = 0; uwIdx < MAX_OPEN_TYPE_NUM; uwIdx++ )
	{
		if ( NULL != m_pastAllOpenType[uwIdx])
		{
			delete m_pastAllOpenType[uwIdx];
		}
	}

	for(uwIdx = 0; uwIdx < MAX_LOCAL_SEC_INDEX_NUM; uwIdx++)
	{
		if(NULL != m_pastLocalIndex[uwIdx])
		{
			delete m_pastLocalIndex[uwIdx];
		}
	}

    for(uwIdx = 0; uwIdx < MAX_GLOBAL_SEC_INDEX_NUM; uwIdx++)
    {
        if(NULL != m_pastGlobalIndex[uwIdx])
        {
            delete m_pastGlobalIndex[uwIdx];
        }
    }

    m_uwAllFieldNum = 0;
    m_uwALLOpenTypeNum = 0;
    // 初始化为0
    memset(m_pastAllFieldProperty,0,sizeof(SFieldProperty*) * MAX_FIELD_NUM );
    memset(m_pastAllOpenType,0,sizeof(SOpenIndexType*) * MAX_OPEN_TYPE_NUM );

    return TRUE;
}

TBOOL        CFieldProperty::GetFieldInfo( TUINT16 uwFieldNo, SFieldProperty* &pstField )
{
    if ( uwFieldNo >= m_uwAllFieldNum )
    {
        return FALSE;
    }

    pstField = m_pastAllFieldProperty[uwFieldNo];

    return TRUE;
}

TBOOL        CFieldProperty::GetAllField( SFieldProperty** &ppstField,TUINT16& uwFieldNum )
{
    ppstField = m_pastAllFieldProperty;
    uwFieldNum = m_uwAllFieldNum;

    return TRUE;
}

TUINT16        CFieldProperty::AliasToFieldNo( const TCHAR* pszAlias )
{
    if ( NULL == m_puwAliasToFieldNo )
    {
        return -1;
    }
    return m_puwAliasToFieldNo[ *(TUINT16*)pszAlias ];
}

TBOOL CFieldProperty::GetOpenTypeInfo( TUINT16 uwType, SOpenIndexType* &pstOpenType )
{
	if(uwType >= m_uwALLOpenTypeNum)
	{
		return FALSE;
	}

	pstOpenType = m_pastAllOpenType[uwType];
	return TRUE;
}

TBOOL CFieldProperty::LoadOpenTypeList( TiXmlHandle *pDocConf )
{
	SOpenIndexType*    pstType = NULL;
	const TCHAR*    pStr = NULL;
	TBOOL            bError = FALSE;
	TINT32            dwTmp = 0;
	TUINT16            uwIdx = 0;
	TiXmlElement *pElement, *pItemElem;
	const TCHAR*	pFld = NULL;
	//TCHAR			*pCur = NULL;
	//TUINT32			udwCurLen = 0;

	pElement = pDocConf->FirstChildElement("OpenType").Element();
	pItemElem = pElement->FirstChildElement("Index");
	while (pItemElem != NULL)
	{
		// 开辟空间
		pstType = new SOpenIndexType;
		// 获取编号
		if (pItemElem->QueryIntAttribute("no", &dwTmp ) != TIXML_SUCCESS)
		{
			bError = TRUE;
		}
		pstType->m_uwTypeNo  = dwTmp;
		// 获取索引名称
		pStr = pItemElem->Attribute("idx_name");
		if(pStr == NULL || pStr[0] == 0)
		{
			bError = TRUE;
		}
		strcpy(pstType->m_szIndexName, pStr);
		
		// 获取需要返回的字段
		pStr = pItemElem->Attribute("ret_fld");
		if(pStr == NULL || strcmp(pStr, "all") == 0)
		{
			pstType->m_udwRetFieldNum = m_uwAllFieldNum;
			for(uwIdx = 0; uwIdx < m_uwAllFieldNum; uwIdx++)
			{
				pstType->m_auwRetFieldNo[uwIdx] = uwIdx;
				pstType->m_aucFieldRetFlag[uwIdx] = TRUE;
			}
		}
		else
		{
			pFld = pStr;
			while(pFld)
			{
				assert(pstType->m_udwRetFieldNum < m_uwAllFieldNum);
				pstType->m_auwRetFieldNo[pstType->m_udwRetFieldNum] = atoi(pFld);
				assert(pstType->m_auwRetFieldNo[pstType->m_udwRetFieldNum] < m_uwAllFieldNum);
				pstType->m_aucFieldRetFlag[pstType->m_auwRetFieldNo[pstType->m_udwRetFieldNum]] = TRUE;
				pstType->m_udwRetFieldNum++;

				pFld = strchr(pFld+1, ',');
				if(pFld)
				{
					pFld++;
				}
			}
		}

        // 获取enum名称
        pStr = pItemElem->Attribute("enum_name");
        if(pStr == NULL || pStr[0] == 0)
        {
            bError = TRUE;
        }
        strcpy(pstType->m_szEType, pStr);

		m_pastAllOpenType[m_uwALLOpenTypeNum++] = pstType;

		pItemElem = pItemElem->NextSiblingElement("Index");
	}
	// 出错
	if ( bError )
	{
		delete pstType;
		pstType = NULL;

		return FALSE;
	}

	return TRUE;
}

TBOOL CFieldProperty::LoadLocalIndexList( TiXmlHandle *pDocConf )
{
	SLocalIndexType*    pstType = NULL;
	const TCHAR*    pStr = NULL;
	TBOOL            bError = FALSE;
	TINT32            dwTmp = 0;
	//TUINT16            uwIdx = 0;
	TiXmlElement *pElement, *pItemElem;
	const TCHAR*	pFld = NULL;
	//TCHAR			*pCur = NULL;
	//TUINT32			udwCurLen = 0;
	//TINT32			dwProjectionType = 0;

	pElement = pDocConf->FirstChildElement("LocalIndex").Element();
	pItemElem = pElement->FirstChildElement("Index");
	while (pItemElem != NULL)
	{
		// 开辟空间
		pstType = new SLocalIndexType;
		// 获取编号
		if (pItemElem->QueryIntAttribute("no", &dwTmp ) != TIXML_SUCCESS)
		{
			bError = TRUE;
		}
		pstType->m_uwTypeNo  = dwTmp;
		// 获取索引名称
		pStr = pItemElem->Attribute("idx_name");
		if(pStr == NULL || pStr[0] == 0)
		{
			bError = TRUE;
		}
		strcpy(pstType->m_szIndexName, pStr);

		// 获取对应的索引字段
		pStr = pItemElem->Attribute("idx_fld");
		assert(pStr);
		pstType->m_uwIndexField = atoi(pStr);

		// 获取projection type
		pStr = pItemElem->Attribute("projection_type");
		if(pStr)
		{
			pstType->m_ucProjectionType = atoi(pStr);
		}
		if(pstType->m_ucProjectionType >= LOCAL_PROJECTION_TYPE__END)
		{
			pstType->m_ucProjectionType = LOCAL_PROJECTION_TYPE__KEYS_ONLY;
		}

		// 获取projection list
		if(pstType->m_ucProjectionType != LOCAL_PROJECTION_TYPE__INCLUDE)
		{
			pstType->m_udwProjectionFieldNum = 0;
		}
		else
		{
			pStr = pItemElem->Attribute("projection_fld");
			assert(pStr);
			pFld = pStr;
			while(pFld)
			{
				assert(pstType->m_udwProjectionFieldNum < MAX_LOCAL_PROJECTION_FIELD_NUM);
				pstType->m_auwProjectionFieldList[pstType->m_udwProjectionFieldNum] = atoi(pFld);
				assert(pstType->m_auwProjectionFieldList[pstType->m_udwProjectionFieldNum] < m_uwAllFieldNum);
				pstType->m_udwProjectionFieldNum++;

				pFld = strchr(pFld+1, ',');
				if(pFld)
				{
					pFld++;
				}
			}
		}
		m_pastLocalIndex[m_uwLocalIndexNum++] = pstType;

		pItemElem = pItemElem->NextSiblingElement("Index");
	}
	// 出错
	if ( bError )
	{
		delete pstType;
		pstType = NULL;

		return FALSE;
	}

	return TRUE;
}

TBOOL CFieldProperty::LoadGlobalIndexList( TiXmlHandle *pDocConf )
{
    SGlobalIndexType*    pstType = NULL;
    const TCHAR*    pStr = NULL;
    TBOOL            bError = FALSE;
    TINT32            dwTmp = 0;
    //TUINT16            uwIdx = 0;
    TiXmlElement *pElement, *pItemElem;
    const TCHAR*	pFld = NULL;
    //TCHAR			*pCur = NULL;
    //TUINT32			udwCurLen = 0;
    //TINT32			dwProjectionType = 0;

    pElement = pDocConf->FirstChildElement("GlobalIndex").Element();
    if(pElement == NULL)
    {
        return true;
    }
    pItemElem = pElement->FirstChildElement("Index");
    while (pItemElem != NULL)
    {
        // 开辟空间
        pstType = new SGlobalIndexType;
        // 获取编号
        if (pItemElem->QueryIntAttribute("no", &dwTmp ) != TIXML_SUCCESS)
        {
            bError = TRUE;
        }
        pstType->m_uwTypeNo  = dwTmp;
        // 获取索引名称
        pStr = pItemElem->Attribute("idx_name");
        if(pStr == NULL || pStr[0] == 0)
        {
            bError = TRUE;
        }
        strcpy(pstType->m_szIndexName, pStr);

        // 获取对应的索引字段
        pStr = pItemElem->Attribute("hash_fld");
        assert(pStr);
        pstType->m_wHashIndexField = atoi(pStr);

        pStr = pItemElem->Attribute("range_fld");
        assert(pStr);
        pstType->m_wRangeIndexField = atoi(pStr);

        // 获取读写阈值
        pStr = pItemElem->Attribute("read");
        assert(pStr);
        pstType->m_udwReadUnits = atoi(pStr);
        pStr = pItemElem->Attribute("write");
        assert(pStr);
        pstType->m_udwWriteUnits = atoi(pStr);

        // 获取projection type
        pStr = pItemElem->Attribute("projection_type");
        if(pStr)
        {
            pstType->m_ucProjectionType = atoi(pStr);
        }
        if(pstType->m_ucProjectionType >= LOCAL_PROJECTION_TYPE__END)
        {
            pstType->m_ucProjectionType = LOCAL_PROJECTION_TYPE__KEYS_ONLY;
        }

        // 获取projection list
        if(pstType->m_ucProjectionType != LOCAL_PROJECTION_TYPE__INCLUDE)
        {
            pstType->m_udwProjectionFieldNum = 0;
        }
        else
        {
            pStr = pItemElem->Attribute("projection_fld");
            assert(pStr);
            pFld = pStr;
            while(pFld)
            {
                assert(pstType->m_udwProjectionFieldNum < MAX_LOCAL_PROJECTION_FIELD_NUM);
                pstType->m_auwProjectionFieldList[pstType->m_udwProjectionFieldNum] = atoi(pFld);
                assert(pstType->m_auwProjectionFieldList[pstType->m_udwProjectionFieldNum] < m_uwAllFieldNum);
                pstType->m_udwProjectionFieldNum++;

                pFld = strchr(pFld+1, ',');
                if(pFld)
                {
                    pFld++;
                }
            }
        }
        m_pastGlobalIndex[m_uwGlobalIndexNum++] = pstType;

        pItemElem = pItemElem->NextSiblingElement("Index");
    }
    // 出错
    if ( bError )
    {
        delete pstType;
        pstType = NULL;

        return FALSE;
    }

    return TRUE;
}

int CFieldProperty::GetTableInfo( TableDesc *poTableInfo )
{
	unsigned int idx = 0, idy = 0;
	// table name
	poTableInfo->sName = m_szTableName;
    poTableInfo->udwTableType = m_udwTableType;
    poTableInfo->udwDependFld = m_udwDependFldNo;
    poTableInfo->udwSplitSize = m_udwSplitSize;
	poTableInfo->bMultiple = m_ucMutleTableFlag == 0 ? false : true;
    poTableInfo->mIndexFldNo.clear();
    poTableInfo->mHashFldMap = m_HashFldMap;
    poTableInfo->mRangeFldMap = m_RangeFldMap;

	//andy@20160527: new fields for cache module
	poTableInfo->udwCacheMode	= m_udwCacheMode;
	poTableInfo->udwRouteFld	= m_udwRouteFld;
	poTableInfo->uddwRouteThrd	= m_uddwRouteThrd;

	// table hash & range key
	poTableInfo->udwHashKeyFldNo = m_pstHashKey->m_uwFieldNo;
	if(m_pstRangeKey)
	{
		poTableInfo->udwRangeKeyFldNo = m_pstRangeKey->m_uwFieldNo;
	}
	else
	{
		poTableInfo->udwRangeKeyFldNo = (TUINT32)-1;
	}
	
	// table local sec index field
	for(idx = 0; idx < m_uwLocalIndexNum; idx++)
	{
		poTableInfo->mIndexFldNo.insert(make_pair(m_pastLocalIndex[idx]->m_uwIndexField, INDEX_TYPE_LOCAL_INDEX));
	}

	// table field info
	poTableInfo->mFieldDesc.clear();
	for(idx = 0; idx < m_uwAllFieldNum; idx++)
	{
		FieldDesc field(m_pastAllFieldProperty[idx]->m_uwFieldNo, m_pastAllFieldProperty[idx]->m_szTitle, 
			m_pastAllFieldProperty[idx]->m_enFieldType, m_pastAllFieldProperty[idx]->m_ucIndexType,
			m_pastAllFieldProperty[idx]->m_udwFieldLen, m_pastAllFieldProperty[idx]->m_szBinStructName, 
            m_pastAllFieldProperty[idx]->m_udwBinStructMaxSize);
		poTableInfo->mFieldDesc.insert(make_pair(m_pastAllFieldProperty[idx]->m_uwFieldNo, field));
	}

	// table index info
	IndexDesc index_info;
	SLocalIndexType *pstLocalIndex = NULL;
    SGlobalIndexType *pstGlobaIndex = NULL;
	for(idx = 0; idx < m_uwALLOpenTypeNum; idx++)
	{
		index_info.Reset();
		pstLocalIndex = NULL;
        pstGlobaIndex = NULL;

		// base
		index_info.udwIdxNo = idx;
		index_info.sName = m_pastAllOpenType[idx]->m_szIndexName;
        index_info.sEnumName = m_pastAllOpenType[idx]->m_szEType;

		// idx field-----顺序必须是hash-key，range-key
		if(strcmp(m_pastAllOpenType[idx]->m_szIndexName, "PRIMARY") == 0)
		{
			index_info.vecIdxFld.push_back(m_pstHashKey->m_uwFieldNo);
			if(m_pstRangeKey)
			{
				index_info.vecIdxFld.push_back(m_pstRangeKey->m_uwFieldNo);
			}
		}
        else if(strncmp(m_pastAllOpenType[idx]->m_szIndexName, "glb_", 4) == 0)
        {
            for(idy = 0; idy < m_uwGlobalIndexNum; idy++)
            {
                if(strcmp(m_pastAllOpenType[idx]->m_szIndexName, m_pastGlobalIndex[idy]->m_szIndexName) == 0)
                {
                    pstGlobaIndex = m_pastGlobalIndex[idy];
                    break;
                }
            }
            assert(pstGlobaIndex);
            index_info.vecIdxFld.push_back(pstGlobaIndex->m_wHashIndexField);
            if(pstGlobaIndex->m_wRangeIndexField >= 0)
            {
                index_info.vecIdxFld.push_back(pstGlobaIndex->m_wRangeIndexField);
            }
        }
		else if(strncmp(m_pastAllOpenType[idx]->m_szIndexName, "idx_", 4) == 0)
		{
			for(idy = 0; idy < m_uwLocalIndexNum; idy++)
			{
				if(strcmp(m_pastAllOpenType[idx]->m_szIndexName, m_pastLocalIndex[idy]->m_szIndexName) == 0)
				{
					pstLocalIndex = m_pastLocalIndex[idy];
					break;
				}
			}
			assert(pstLocalIndex);
			index_info.vecIdxFld.push_back(m_pstHashKey->m_uwFieldNo);
			index_info.vecIdxFld.push_back(pstLocalIndex->m_uwIndexField);
		}
        else
        {
            assert(0);
        }

		// ret field
		if(m_pastAllOpenType[idx]->m_udwRetFieldNum != m_uwAllFieldNum)
		{
			for(idy = 0; idy < m_pastAllOpenType[idx]->m_udwRetFieldNum; idy++)
			{
				index_info.vecRtnFld.push_back(m_pastAllOpenType[idx]->m_auwRetFieldNo[idy]);
			}
		}
		// add to map
		poTableInfo->mIndexDesc.insert(make_pair(idx, index_info));
	}

	return 0;
}
