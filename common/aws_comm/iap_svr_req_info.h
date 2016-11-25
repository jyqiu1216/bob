#pragma once
struct IapSvrReqInfo
{
    unsigned int m_udwType;
    string m_sReqContent;
    
    IapSvrReqInfo()
    {
        m_udwType = 0;
        m_sReqContent = "";
    }
};