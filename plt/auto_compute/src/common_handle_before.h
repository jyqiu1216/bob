#ifndef _AU_COMMON_HANDLE_BEFORE_H_
#define _AU_COMMON_HANDLE_BEFORE_H_

#include "session.h"

class CAuCommonBefore
{
public:
    static TINT32 AuCommonHandleBefore(SSession *pstSession, SUserInfo* pstUser);
    static TVOID GetLastTotalMight(SUserInfo * pstUser, CTseLogger * poServLog);
};

#endif

