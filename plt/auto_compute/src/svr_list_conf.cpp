#include "svr_list_conf.h"

using wtse::conf::CTseIniConfiger;

string CActionTableConf::m_strIp = "";
CActionTableConf* CActionTableConf::m_poSvrListConf = NULL;

CActionTableConf* CActionTableConf::GetInstance()
{
    if(m_poSvrListConf == NULL)
    {
        m_poSvrListConf = new CActionTableConf;
    }

    return m_poSvrListConf;
}

TINT32 CActionTableConf::Init(const TCHAR *pszConfFile, const string strIp /*= ""*/)
{
    CTseIniConfiger config_obj;
    TCHAR szKeyStr[1024];
    TBOOL bRetVval = FALSE;

    if(strIp != "")
    {
        m_strIp = strIp;
    }

    m_vecTable.clear();
    m_udwTableNum = 0;

    bRetVval = config_obj.LoadConfig(pszConfFile);
    assert(bRetVval == TRUE);

    bRetVval = config_obj.GetValue("SERV_INFO", "ActionTableNum", m_udwTableNum);
    assert(bRetVval == TRUE);
    for(TUINT32 udwIdx = 0; udwIdx < m_udwTableNum; udwIdx++)
    {
        SActionTable stActionTable;
        string strtSvrIp;
        sprintf(szKeyStr, "table_%u_ip", udwIdx);
        bRetVval = config_obj.GetValue("SERV_INFO", szKeyStr, strtSvrIp);
        assert(bRetVval == TRUE);
        if(strtSvrIp != m_strIp)
        {
            continue;
        }
        sprintf(szKeyStr, "table_%u_type", udwIdx);
        bRetVval = config_obj.GetValue("SERV_INFO", szKeyStr, stActionTable.dwTableType);
        assert(bRetVval == TRUE);
        sprintf(szKeyStr, "table_%u_name", udwIdx);
        bRetVval = config_obj.GetValue("SERV_INFO", szKeyStr, stActionTable.strTableName);
        sprintf(szKeyStr, "table_%u_idx", udwIdx);
        bRetVval = config_obj.GetValue("SERV_INFO", szKeyStr, stActionTable.dwTableIdx);
        assert(bRetVval == TRUE);
/*
        sprintf(szKeyStr, "table_%u_svr_num", udwIdx);
        TINT32 dwSvrNum = 0;
        bRetVval = config_obj.GetValue("SERV_INFO", szKeyStr, dwSvrNum);
        assert(bRetVval == TRUE);

        TINT32 dwSvrId = 0;
        for (TINT32 dwIdx = 0; dwIdx < dwSvrNum; dwIdx++)
        {
            sprintf(szKeyStr, "table_%u_svr_idx_%d", udwIdx, dwIdx);
            bRetVval = config_obj.GetValue("SERV_INFO", szKeyStr, dwSvrId);
            assert(bRetVval == TRUE);
            stActionTable.setTableSid.insert(dwSvrId);
        }
*/
        m_vecTable.push_back(stActionTable);
    }
    m_udwTableNum = m_vecTable.size();

    return 0;
}

TINT32 CActionTableConf::Update(const TCHAR *pszConfFile, CTseLogger *poLog)
{
    TINT32 dwRetCode = 0;
    CActionTableConf *poSvrListConf = new CActionTableConf;
    CActionTableConf *poSvrListTmpConf = m_poSvrListConf;

    dwRetCode = poSvrListConf->Init(pszConfFile);
    if(dwRetCode != 0)
    {
        TSE_LOG_ERROR(poLog, (" CConf::Update failed[%d]", dwRetCode));
        delete poSvrListConf;
        return -1;
    }

    m_poSvrListConf = poSvrListConf;

    if(poSvrListTmpConf != NULL)
    {
        sleep(5);
        delete poSvrListTmpConf;
    }

    TSE_LOG_INFO(poLog, (" CSvrListConf::Update ok", dwRetCode));

    return 0;
}