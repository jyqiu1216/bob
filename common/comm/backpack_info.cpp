#include "backpack_info.h"
#include <math.h>
#include "tool_base.h"

CBackpackInfo* CBackpackInfo::m_poBackpackInfo = NULL;


CBackpackInfo* CBackpackInfo::GetInstance()
{
    if(m_poBackpackInfo == NULL)
    {
        m_poBackpackInfo = new CBackpackInfo;
    }

    return m_poBackpackInfo;
}

TINT32 CBackpackInfo::Init(const TCHAR *pszFileName, CTseLogger *poLog)
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
    if(reader.parse(is, tmpRawJson) == false)
    {
        TSE_LOG_ERROR(m_poLog, ("CBackpackInfo::Init: parse file[%s] failed.", pszFileName));
        is.close();
        return -2;
    }
    else
    {
        m_oJsonRoot.clear();
        //m_oJsonRoot = Json::Value(Json::objectValue);
        //CToolBase::LoadNewJson(tmpRawJson, m_oJsonRoot);
        m_oJsonRoot = tmpRawJson;

        TSE_LOG_INFO(m_poLog, ("CBackpackInfo::Init: parse file[%s] success.", pszFileName));
        is.close();
    }
    return 0;
}

TINT32 CBackpackInfo::Update(const TCHAR *pszFileName, CTseLogger *poLog)
{
    TINT32 dwRetCode = 0;
    CBackpackInfo *poEventInfo = new CBackpackInfo;
    CBackpackInfo *poTmpEventInfo = m_poBackpackInfo;

    dwRetCode = poEventInfo->Init(pszFileName, poLog);
    if(dwRetCode != 0)
    {
        TSE_LOG_ERROR(poLog, (" CBackpackInfo::Update failed[%d]", dwRetCode));
        delete poEventInfo;
        return -1;
    }

    m_poBackpackInfo = poEventInfo;

    if(poTmpEventInfo != NULL)
    {
        sleep(3);
        delete poTmpEventInfo;
    }

    TSE_LOG_INFO(poLog, (" CBackpackInfo::Update ok", dwRetCode));

    return 0;
}

