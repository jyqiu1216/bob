#ifndef _PROCESS_CHANGE_AL_INFO_H_
#define _PROCESS_CHANGE_AL_INFO_H_

#include "session.h"

class CProcessChangeAlInfo
{
public:
    static TINT32 requestHandler(SSession *pstSession);

private:
    static TVOID genResponse(SSession *pstSession);

};

#endif
