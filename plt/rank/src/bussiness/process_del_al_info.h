#ifndef _PROCESS_DEL_AL_INFO_H_
#define _PROCESS_DEL_AL_INFO_H_

#include "session.h"

class CProcessDelAlInfo
{
public:
    static TINT32 requestHandler(SSession *pstSession);

private:
    static TVOID genResponse(SSession *pstSession);
};

#endif

