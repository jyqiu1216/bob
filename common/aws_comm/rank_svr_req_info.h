#ifndef _RANK_SVR_REQ_INFO_H_
#define _RANK_SVR_REQ_INFO_H_

#include <string>
#include <sstream>
using namespace std;

struct RankSvrReqInfo
{
    unsigned int m_udwUid;
    unsigned int m_udwSvrId;
    string m_szCmd;

    RankSvrReqInfo()
    {
        Reset();
    }

    void Reset()
    {
        m_udwUid = 0;
        m_udwSvrId = 0;
        m_szCmd.clear();
    }

    void SetVal(unsigned int sid, unsigned int uid, const string& cmd)
    {
        m_udwUid = uid;
        m_udwSvrId = sid;
        m_szCmd = cmd;
    }

    void GenReqString(string &szOut)
    {
        ostringstream oss;
        oss.str("");
        oss << "hu_send?op_en_flag=0&sid=" << m_udwSvrId << "&uid=" << m_udwUid << "&command=" << m_szCmd;

        szOut = oss.str();
    }
};

#endif