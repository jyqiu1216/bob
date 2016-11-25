#include "lock_info.h"

CLockInfo* CLockInfo::m_poLockInfo = NULL;

CLockInfo* CLockInfo::GetInstance()
{
    if (m_poLockInfo == NULL)
    {
        m_poLockInfo = new CLockInfo;
    }

    return m_poLockInfo;
}

TINT32 CLockInfo::Init(const TCHAR *pszFileName, CTseLogger *poLog)
{
    if (pszFileName == NULL || poLog == NULL)
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
    if (reader.parse(is, m_oJsonRoot) == false)
    {
        TSE_LOG_ERROR(m_poLog, ("CLockInfo::Init: parse file[%s] failed.", pszFileName));
        is.close();
        return -2;
    }
    else
    {
        TSE_LOG_INFO(m_poLog, ("CLockInfo::Init: parse file[%s] success.", pszFileName));
        is.close();
    }
    return 0;
}

TINT32 CLockInfo::Update(const TCHAR *pszFileName, CTseLogger *poLog)
{
    TINT32 dwRetCode = 0;
    CLockInfo *poLockInfo = new CLockInfo;
    CLockInfo *poTmpLockInfo = m_poLockInfo;

    dwRetCode = poLockInfo->Init(pszFileName, poLog);
    if (dwRetCode != 0)
    {
        TSE_LOG_ERROR(poLog, (" CLockInfo::Update failed. [ret code=%d]", dwRetCode));
        delete poLockInfo;
        return -1;
    }

    m_poLockInfo = poLockInfo;

    if (poTmpLockInfo != NULL)
    {
        sleep(2);
        delete poTmpLockInfo;
    }

    TSE_LOG_INFO(poLog, (" CLockInfo::Update ok. [ret code=%d]", dwRetCode));

    return 0;
}