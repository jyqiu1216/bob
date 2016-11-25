#ifndef _PROCESS_ADD_AL_INFO_H_
#define _PROCESS_ADD_AL_INFO_H_

#include "session.h"

class CProcessAddAlInfo
{
public:
    static TINT32 requestHandler(SSession *pstSession);
private:
    static TVOID genResponse(SSession *pstSession);
};

#endif

