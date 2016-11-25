#ifndef _CACHE_REQ_INFO_H_
#define _CACHE_REQ_INFO_H_

#include <string>
using namespace std;


struct CacheReqInfo
{
    string m_strCacheName;
    string m_strCacheOperate;
    TUINT64 m_uddwCacheKey;    
    string m_strCacheValue;

    CacheReqInfo(const string &strCacheName = "", const string &strCacheOperate = "", TUINT64 uddwCacheKey = 0, const string &strCacheValue = ""):m_strCacheName(strCacheName), m_strCacheOperate(strCacheOperate), m_uddwCacheKey(uddwCacheKey), m_strCacheValue(strCacheValue)
    {
    }

    TVOID SetVal(string strCacheName, string strCacheOperate, TUINT64 uddwCacheKey, string strCacheValue)
    {
        m_strCacheName = strCacheName;
        m_strCacheOperate = strCacheOperate;
        m_uddwCacheKey = uddwCacheKey;
        m_strCacheValue = strCacheValue;
    }

};

#endif
