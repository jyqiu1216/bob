#ifndef _COMMON_OUTPUT_CONF_H_
#define _COMMON_OUTPUT_CONF_H_

#include "jsoncpp/json/json.h"
#include "base/common/wtsetypedef.h"
#include <map>

using namespace std;

enum EContentUpdateType
{
    EN_CONTENT_UPDATE_TYPE__ALL = 0,
    EN_CONTENT_UPDATE_TYPE__TABLE_INC,
    EN_CONTENT_UPDATE_TYPE__ITEM_INC,
    EN_CONTENT_UPDATE_TYPE__FIELD_ADD,
};

enum EPushDataType
{
    EN_PUSH_DATA_TYPE__UID = 1,
    EN_PUSH_DATA_TYPE__SID = 2,
    EN_PUSH_DATA_TYPE__AID = 3,
    EN_PUSH_DATA_TYPE__MAP = 4,
};

enum ETableOgzType //dom表数据组织方式
{
    EN_TABLE_ORGANIZE_TYPE__NORMAL = 0,
    EN_TABLE_ORGANIZE_TYPE__TEMP,
    EN_TABLE_ORGANIZE_TYPE__COMPLEX,
};

enum ETableDataType //dom表数据类型
{
    EN_TABLE_DATA_TYPE__DB = 0,
    EN_TABLE_DATA_TYPE__COMPUTE,
};

enum ETableSeqCheckType //dom表seq check方式
{
    EN_TABLE_SEQCHECK_TYPE__NO = 0,
    EN_TABLE_SEQCHECK_TYPE__TABLE,
    EN_TABLE_SEQCHECK_TYPE__ITEM,
};

enum EDomDataType
{
    EN_DOMDATA_TYPE__BJSON = 0,
    EN_DOMDATA_TYPE__JSON = 1,
    EN_DOMDATA_TYPE__BINARY = 2,
};

struct STableCommonConf
{
    TINT8 m_ucOrganizeType;
    TINT8 m_ucDataType;
    TINT8 m_ucSeqCheckType;
    TINT8 m_ucUpdtType;

    STableCommonConf()
    {
        m_ucOrganizeType = EN_TABLE_ORGANIZE_TYPE__NORMAL;
        m_ucDataType = EN_TABLE_DATA_TYPE__DB;
        m_ucSeqCheckType = EN_TABLE_SEQCHECK_TYPE__NO;
        m_ucUpdtType = EN_CONTENT_UPDATE_TYPE__ALL;
    }

    STableCommonConf(TINT8 ucOrganizeType, TINT8 ucDataType, TINT8 ucSeqType, TINT8 ucUpdtType)
    {
        m_ucOrganizeType = ucOrganizeType;
        m_ucDataType = ucDataType;
        m_ucSeqCheckType = ucSeqType;
        m_ucUpdtType = ucUpdtType;
    }
};

typedef map<string, STableCommonConf> MapTableConf;

class COutputConf
{
public:
    static COutputConf* GetInstace();
    TBOOL Init();
    
    TINT32 GetTableOutputType(const TCHAR* pszTable, TINT32 dwResType);
    TVOID GetTableConf(const TCHAR* pszTable, STableCommonConf &stTableConf);
    TINT32 GetTableSeqCheckType(const TCHAR* pszTable);

private:
    TVOID InitTableConf();
    TVOID AddTableConf(const TCHAR* pszTable, TINT8 ucOrganizeType = EN_TABLE_ORGANIZE_TYPE__NORMAL, 
        TINT8 ucDataType = EN_TABLE_DATA_TYPE__DB, TINT8 ucSeqType = EN_TABLE_SEQCHECK_TYPE__NO, TINT8 ucUpdtType = EN_CONTENT_UPDATE_TYPE__ALL);

private:   
    static COutputConf* m_poOutputConf;
    MapTableConf m_objTblMap;
};

#endif

