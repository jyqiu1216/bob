#ifndef _PROCESS_ACTIVE_EVENT_H_
#define _PROCESS_ACTIVE_EVENT_H_


#include "session.h"

class CProcessEvent
{

public: 

    static TINT32 ProcessCmd_AllEventInfoGet(SSession *pstSession, TBOOL& bNeedResponse);
    static TINT32 ProcessCmd_EventInfoGet(SSession *pstSession, TBOOL& bNeedResponse);
    static TINT32 ProcessCmd_HistoryEventInfoGet(SSession *pstSession, TBOOL& bNeedResponse);
    static TINT32 ProcessCmd_ThemeHistoryEventInfoGet(SSession *pstSession, TBOOL& bNeedResponse);
};



#endif
