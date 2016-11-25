#include "alliance_mapping.h"

CAllianceMapping* CAllianceMapping::m_poAllianceMapping = NULL;

CAllianceMapping* CAllianceMapping::GetInstance()
{
    if(m_poAllianceMapping == NULL)
    {
        m_poAllianceMapping = new CAllianceMapping;
    }

    return m_poAllianceMapping;
}

TINT32 CAllianceMapping::Init(const TCHAR *pszFileName, CTseLogger *poLog)
{
    if(pszFileName == NULL || poLog == NULL)
    {
        return -1;
    }

    // 0. set param
    m_poLog = poLog;

    // 1. 解析json用Json::Reader
    Json::Reader reader;
    // Json::Value是一种很重要的类型，可以代表任意类型。如int, string, object, array...
    //Json::Value root;
    std::ifstream is;
    is.open(pszFileName, std::ios::binary);
    if(reader.parse(is, m_oJsonRoot) == false)
    {
        TSE_LOG_ERROR(m_poLog, ("CAllianceMapping::Init: parse file[%s] failed.", pszFileName));
        is.close();
        return -2;
    }
    else
    {
        TSE_LOG_INFO(m_poLog, ("CAllianceMapping::Init: parse file[%s] success.", pszFileName));
        is.close();
    }
    return 0;
}

TINT32 CAllianceMapping::Update(const TCHAR *pszFileName, CTseLogger *poLog)
{
    TINT32 dwRetCode = 0;
    CAllianceMapping *poEventInfo = new CAllianceMapping;
    CAllianceMapping *poTmpEventInfo = m_poAllianceMapping;

    dwRetCode = poEventInfo->Init(pszFileName, poLog);
    if(dwRetCode != 0)
    {
        TSE_LOG_ERROR(poLog, (" CAllianceMapping::Update failed. [ret code=%d]", dwRetCode));
        delete poEventInfo;
        return -1;
    }

    m_poAllianceMapping = poEventInfo;

    if(poTmpEventInfo != NULL)
    {
        sleep(2);
        delete poTmpEventInfo;
    }

    TSE_LOG_INFO(poLog, (" CAllianceMapping::Update ok. [ret code=%d]", dwRetCode));

    return 0;
}