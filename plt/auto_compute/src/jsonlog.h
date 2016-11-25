#ifndef _JSON_LOG_H_
#define _JSON_LOG_H_

#include "std_header.h"

class SSession;

class CJsonLog
{
public:
    static TVOID Init(CTseLogger *poJsonLog);
    static TVOID OutLogForStat(SSession *pstSession, SUserInfo *pstUser, const TCHAR *pstBodyContent);

private:
    static CTseLogger *m_poJsonLog;
};

#endif
