#ifndef _PROCESS_AL_LIST_GET_H_
#define _PROCESS_AL_LIST_GET_H_

#include "session.h"

class CProcessAlListGet
{
public:
    static TINT32 requestHandler(SSession *pstSession);

private:
    static TVOID NewWrapAlListJson(SSession *pstSession);
};



#endif
