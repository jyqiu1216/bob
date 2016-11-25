#pragma once



struct IapSvrRspInfo
{
    TINT32 m_dwRetCode;
    TUINT32 m_udwType;
    string m_sRspJson;

    IapSvrRspInfo()
    {
        Reset();
    }

    void Reset()
    {
        m_dwRetCode = 0;
        m_udwType = 0;
        m_sRspJson = "";
    }
};