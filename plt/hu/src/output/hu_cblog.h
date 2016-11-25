#ifndef _HU_CBLOG_H_
#define _HU_CBLOG_H_
#include "session.h"

class CHuCBLog
{
public:
    static TVOID OutLogCbLog(SSession *pstSession, string strCommand = "", string strKey = "");

    static TVOID OutOperateLogCbLog(SSession *pstSession, string strCommand = "", string strKey = "");

private:
    template<class T>
    static TVOID SetCbLogKey(ostringstream &oss, const T &tVariable);

    static TVOID GenProductionCBlog(SResourceProduction* pstProduction, ostringstream& CBlog);
};

#endif
