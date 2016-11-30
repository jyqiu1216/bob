#ifndef _PROCESS_ACTIVE_EVENT_H_
#define _PROCESS_ACTIVE_EVENT_H_


#include "session.h"

class CProcessEvent
{
public:
    static TINT32 ProcessCmd_AllEventGet(SSession *pstSession, TBOOL& bNeedResponse);
};
#endif
