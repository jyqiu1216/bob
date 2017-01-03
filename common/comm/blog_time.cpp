#include "blog_time.h"

CBlogTime* CBlogTime::m_poBlogTime = NULL;

CBlogTime* CBlogTime::GetInstance()
{
    if (m_poBlogTime == NULL)
    {
        m_poBlogTime = new CBlogTime;
    }
    return m_poBlogTime;
}

TINT32 CBlogTime::Init(const TCHAR *pszFileName, CTseLogger *poLog)
{
    if (pszFileName == NULL || poLog == NULL)
    {
        return -1;
    }

    // 0. set param
    m_poLog = poLog;

    // 1. ½âÎöjson
    Json::Reader reader;
    Json::Value tmpRawJson;

    std::ifstream is;
    is.open(pszFileName, std::ios::binary);
    if (reader.parse(is, tmpRawJson) == false)
    {
        TSE_LOG_ERROR(m_poLog, ("CBlogTime::Init: parse file[%s] failed.", pszFileName));
        is.close();
        return -2;
    }
    else
    {
        m_jBlogTime.clear();
        m_jBlogTime = tmpRawJson;
        TSE_LOG_INFO(m_poLog, ("CBlogTime::Init: parse file[%s] success.", pszFileName));
        is.close();
    }
    return 0;
}

TINT32 CBlogTime::Update(const TCHAR *pszFileName, CTseLogger *poLog)
{
    TINT32 dwRetCode = 0;
    CBlogTime *poBlogTime = new CBlogTime;
    CBlogTime *poTmpBlogTime = m_poBlogTime;

    dwRetCode = poBlogTime->Init(pszFileName, poLog);
    if (dwRetCode != 0)
    {
        TSE_LOG_ERROR(poLog, (" CBlogTime::Update failed[%d]", dwRetCode));
        delete poBlogTime;
        return -1;
    }

    m_poBlogTime = poBlogTime;

    if (poTmpBlogTime != NULL)
    {
        sleep(3);
        delete poTmpBlogTime;
    }
    TSE_LOG_INFO(poLog, ("CBlogTime::Update ok", dwRetCode));
    return 0;
}