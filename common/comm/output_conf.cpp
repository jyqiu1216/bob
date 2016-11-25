#include "output_conf.h"
#include "service_key.h"

COutputConf* COutputConf::m_poOutputConf = NULL;

COutputConf* COutputConf::GetInstace()
{
    if(m_poOutputConf == NULL)
    {
        m_poOutputConf = new COutputConf;
        m_poOutputConf->Init();
    }
    return m_poOutputConf;
}

TBOOL COutputConf::Init()
{
    m_objTblMap.clear();

    InitTableConf();

    return TRUE;
}

TVOID COutputConf::GetTableConf(const TCHAR* pszTable, STableCommonConf &stTableConf)
{
    MapTableConf::iterator it = m_objTblMap.find(pszTable);
    if(it != m_objTblMap.end())
    {
        stTableConf = it->second;
    }
    else
    {
        stTableConf.m_ucOrganizeType = EN_TABLE_ORGANIZE_TYPE__NORMAL;
        stTableConf.m_ucDataType = EN_TABLE_DATA_TYPE__DB;
        stTableConf.m_ucSeqCheckType = EN_TABLE_SEQCHECK_TYPE__NO;
    }
}

TINT32 COutputConf::GetTableOutputType( const TCHAR* pszTable, TINT32 dwResType )
{
    TINT32 dwTblOutputType = EN_CONTENT_UPDATE_TYPE__ITEM_INC;
    MapTableConf::iterator it = m_objTblMap.find(pszTable);
    if(it != m_objTblMap.end())
    {
        dwTblOutputType = it->second.m_ucUpdtType;
    }

    // 如果表支持的级别比要求的级别高，返回要求级别即可
    if(dwTblOutputType > dwResType)
    {
        dwTblOutputType = dwResType;
    }

    return dwTblOutputType;
}

TVOID COutputConf::InitTableConf()
{
    // EN_TABLE_ORGANIZE_TYPE__NORMAL
    AddTableConf("svr_player", EN_TABLE_ORGANIZE_TYPE__NORMAL, EN_TABLE_DATA_TYPE__DB, EN_TABLE_SEQCHECK_TYPE__TABLE, EN_CONTENT_UPDATE_TYPE__TABLE_INC);
    AddTableConf("svr_alliance", EN_TABLE_ORGANIZE_TYPE__NORMAL, EN_TABLE_DATA_TYPE__DB, EN_TABLE_SEQCHECK_TYPE__TABLE, EN_CONTENT_UPDATE_TYPE__TABLE_INC);

    AddTableConf("svr_title_new", EN_TABLE_ORGANIZE_TYPE__NORMAL, EN_TABLE_DATA_TYPE__DB, EN_TABLE_SEQCHECK_TYPE__NO, EN_CONTENT_UPDATE_TYPE__TABLE_INC);

    // EN_TABLE_ORGANIZE_TYPE__COMPLEX
    AddTableConf("svr_action_list", EN_TABLE_ORGANIZE_TYPE__COMPLEX, EN_TABLE_DATA_TYPE__DB, EN_TABLE_SEQCHECK_TYPE__ITEM, EN_CONTENT_UPDATE_TYPE__ITEM_INC);
    AddTableConf("svr_p_action_list", EN_TABLE_ORGANIZE_TYPE__COMPLEX, EN_TABLE_DATA_TYPE__DB, EN_TABLE_SEQCHECK_TYPE__ITEM, EN_CONTENT_UPDATE_TYPE__ITEM_INC);
    AddTableConf("svr_al_action_list", EN_TABLE_ORGANIZE_TYPE__COMPLEX, EN_TABLE_DATA_TYPE__DB, EN_TABLE_SEQCHECK_TYPE__ITEM, EN_CONTENT_UPDATE_TYPE__ITEM_INC);
    AddTableConf("svr_al_p_action_list", EN_TABLE_ORGANIZE_TYPE__COMPLEX, EN_TABLE_DATA_TYPE__DB, EN_TABLE_SEQCHECK_TYPE__ITEM, EN_CONTENT_UPDATE_TYPE__ITEM_INC);
    
    // map
    AddTableConf("svr_map", EN_TABLE_ORGANIZE_TYPE__COMPLEX, EN_TABLE_DATA_TYPE__DB, EN_TABLE_SEQCHECK_TYPE__NO, EN_CONTENT_UPDATE_TYPE__ALL);

    // EN_TABLE_SEQCHECK_TYPE__NO ----else
    AddTableConf("svr_client_show_flag", EN_TABLE_ORGANIZE_TYPE__TEMP, EN_TABLE_DATA_TYPE__DB, EN_TABLE_SEQCHECK_TYPE__NO, EN_CONTENT_UPDATE_TYPE__ALL);
    AddTableConf("svr_reward_window", EN_TABLE_ORGANIZE_TYPE__TEMP, EN_TABLE_DATA_TYPE__DB, EN_TABLE_SEQCHECK_TYPE__NO, EN_CONTENT_UPDATE_TYPE__ALL);
    AddTableConf("svr_event_reward_window", EN_TABLE_ORGANIZE_TYPE__TEMP, EN_TABLE_DATA_TYPE__DB, EN_TABLE_SEQCHECK_TYPE__NO, EN_CONTENT_UPDATE_TYPE__ALL);
    AddTableConf("svr_broadcast", EN_TABLE_ORGANIZE_TYPE__TEMP, EN_TABLE_DATA_TYPE__DB, EN_TABLE_SEQCHECK_TYPE__NO, EN_CONTENT_UPDATE_TYPE__ALL);
    AddTableConf("svr_reward_window_new", EN_TABLE_ORGANIZE_TYPE__TEMP, EN_TABLE_DATA_TYPE__DB, EN_TABLE_SEQCHECK_TYPE__NO, EN_CONTENT_UPDATE_TYPE__ALL);
    AddTableConf("svr_random_reward_info", EN_TABLE_ORGANIZE_TYPE__TEMP, EN_TABLE_DATA_TYPE__DB, EN_TABLE_SEQCHECK_TYPE__NO, EN_CONTENT_UPDATE_TYPE__ALL);
    AddTableConf("svr_compute_res", EN_TABLE_ORGANIZE_TYPE__TEMP, EN_TABLE_DATA_TYPE__DB, EN_TABLE_SEQCHECK_TYPE__NO, EN_CONTENT_UPDATE_TYPE__ALL);

    AddTableConf("svr_map_inc", EN_TABLE_ORGANIZE_TYPE__TEMP, EN_TABLE_DATA_TYPE__DB, EN_TABLE_SEQCHECK_TYPE__NO, EN_CONTENT_UPDATE_TYPE__ALL);
    AddTableConf("svr_tips", EN_TABLE_ORGANIZE_TYPE__TEMP, EN_TABLE_DATA_TYPE__DB, EN_TABLE_SEQCHECK_TYPE__NO, EN_CONTENT_UPDATE_TYPE__ALL);

}

TVOID COutputConf::AddTableConf( const TCHAR* pszTable, TINT8 ucOrganizeType, TINT8 ucDataType, TINT8 ucSeqType, TINT8 ucUpdtType )
{
    STableCommonConf stTblConf(ucOrganizeType, ucDataType, ucSeqType, ucUpdtType);
    m_objTblMap.insert(make_pair(pszTable, stTblConf));
}

TINT32 COutputConf::GetTableSeqCheckType( const TCHAR* pszTable )
{
    MapTableConf::iterator it = m_objTblMap.find(pszTable);
    if(it != m_objTblMap.end())
    {
        return it->second.m_ucSeqCheckType;
    }
    
    return EN_TABLE_SEQCHECK_TYPE__NO;
}
