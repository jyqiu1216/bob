#ifndef _PROCESS_GET_AL_RANL_BY_ID_
#define _PROCESS_GET_AL_RANL_BY_ID_

#include "session.h"

class CProcessGetAlRankById
{
public:
    static TINT32 requestHandler(SSession *pstSession);

private:
    static TVOID genResponse(SSession *pstSession);

};

#endif

