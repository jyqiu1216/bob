#include "chest_lottery.h"
#include <math.h>
#include "tool_base.h"

CChestLottery* CChestLottery::m_poChestLottery = NULL;


CChestLottery* CChestLottery::GetInstance()
{
    if (m_poChestLottery == NULL)
    {
        m_poChestLottery = new CChestLottery;
    }

    return m_poChestLottery;
}

TINT32 CChestLottery::Init(const TCHAR *pszFileName, CTseLogger *poLog)
{
    if (pszFileName == NULL || poLog == NULL)
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
    if (reader.parse(is, tmpRawJson) == false)
    {
        TSE_LOG_ERROR(m_poLog, ("CChestLottery::Init: parse file[%s] failed.", pszFileName));
        is.close();
        return -2;
    }
    else
    {
        m_oJsonRoot = tmpRawJson; // not a new json

        TSE_LOG_INFO(m_poLog, ("CChestLottery::Init: parse file[%s] success.", pszFileName));
        is.close();
    }
    return 0;
}

TINT32 CChestLottery::Update(const TCHAR *pszFileName, CTseLogger *poLog)
{
    TINT32 dwRetCode = 0;
    CChestLottery *poEventInfo = new CChestLottery;
    CChestLottery *poTmpEventInfo = m_poChestLottery;

    dwRetCode = poEventInfo->Init(pszFileName, poLog);
    if (dwRetCode != 0)
    {
        TSE_LOG_ERROR(poLog, (" CChestLottery::Update failed[%d]", dwRetCode));
        delete poEventInfo;
        return -1;
    }

    m_poChestLottery = poEventInfo;

    if (poTmpEventInfo != NULL)
    {
        sleep(3);
        delete poTmpEventInfo;
    }

    TSE_LOG_INFO(poLog, (" CChestLottery::Update ok", dwRetCode));

    return 0;
}

