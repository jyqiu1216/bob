#include "func_open.h"
#include <math.h>
#include "tool_base.h"

CFuncOpen* CFuncOpen::m_poFuncOpen = NULL;


CFuncOpen* CFuncOpen::GetInstance()
{
    if(m_poFuncOpen == NULL)
    {
        m_poFuncOpen = new CFuncOpen;
    }

    return m_poFuncOpen;
}

TINT32 CFuncOpen::Init(const TCHAR *pszFileName, CTseLogger *poLog)
{
    if(pszFileName == NULL || poLog == NULL)
    {
        return -1;
    }

    // 0. set param
    m_poLog = poLog;

    // 1. 解析json用Json::Reader
    Json::Reader reader;
    Json::Value tmpRawJson;

    // Json::Value是一种很重要的类型，可以代表任意类型。如int, string, object, array...
    //Json::Value root; 
    std::ifstream is;
    is.open(pszFileName, std::ios::binary);
    if(reader.parse(is, m_oJsonRoot) == false)
    {
        TSE_LOG_ERROR(m_poLog, ("CFuncOpen::Init: parse file[%s] failed.", pszFileName));
        is.close();
        return -2;
    }
    else
    {
        //m_oJsonRoot.clear();
        //m_oJsonRoot = Json::Value(Json::objectValue);
        //CToolBase::LoadNewJson(tmpRawJson, m_oJsonRoot);

        TSE_LOG_INFO(m_poLog, ("CFuncOpen::Init: parse file[%s] success.", pszFileName));
        is.close();
    }
    return 0;
}

TINT32 CFuncOpen::Update(const TCHAR *pszFileName, CTseLogger *poLog)
{
    TINT32 dwRetCode = 0;
    CFuncOpen *poEventInfo = new CFuncOpen;
    CFuncOpen *poTmpEventInfo = m_poFuncOpen;

    dwRetCode = poEventInfo->Init(pszFileName, poLog);
    if(dwRetCode != 0)
    {
        TSE_LOG_ERROR(poLog, (" CFuncOpen::Update failed[%d]", dwRetCode));
        delete poEventInfo;
        return -1;
    }

    m_poFuncOpen = poEventInfo;

    if(poTmpEventInfo != NULL)
    {
        sleep(3);
        delete poTmpEventInfo;
    }

    TSE_LOG_INFO(poLog, (" CFuncOpen::Update ok", dwRetCode));

    return 0;
}

