#ifndef _CACHE_RSP_INFO_H_
#define _CACHE_RSP_INFO_H_

#include <string>
using namespace std;


struct CacheRspInfo
{
    int dwRetCode;
    unsigned int udwServiceType;
    string m_strCacheName;
    string m_strCacheOperate;
    TUINT64 m_uddwCacheKey;    
    string m_strCacheValue;
    TBOOL m_bExist;

    CacheRspInfo()
    {
        Reset();
    }

    void Reset()
    {
        dwRetCode = 0;
        udwServiceType = 0;
        m_strCacheName = "";
        m_strCacheOperate = "";
        m_uddwCacheKey = 0;
        m_strCacheValue = "";
        m_bExist = FALSE;
    }
};

#endif      
