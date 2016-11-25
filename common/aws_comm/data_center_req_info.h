#pragma once
struct DataCenterReqInfo
{
    unsigned int m_udwType;
    string m_sReqContent;
    
    DataCenterReqInfo()
    {
        m_udwType = 0;
        m_sReqContent = "";
    }
};