#ifndef _RANK_SVR_RSP_INFO_H_
#define _RANK_SVR_RSP_INFO_H_

#include <string>
using namespace std;

struct RankSvrRspInfo
{
    int m_dwRetCode;
    unsigned int m_udwUid;
    string m_sRspJson;

    RankSvrRspInfo()
    {
        Reset();
    }

    void Reset()
    {
        m_dwRetCode = 0;
        m_udwUid = 0;
        m_sRspJson.clear();
    }
};

#endif