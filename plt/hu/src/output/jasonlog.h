#ifndef _JASON_LOG_H_
#define _JASON_LOG_H_

#include "std_header.h"

class SSession;

class CJasonLog
{
public:
    static TVOID OutLogForStat(SSession *pstSession ,const TCHAR *pstBodyContent, TUINT32 udwBodyLen);
};

#endif
