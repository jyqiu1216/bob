#ifndef _PROCEDURE_BASE_H_
#define _PROCEDURE_BASE_H_

#include "session.h"
// #include "model.h"

class CBaseProcedure
{
public:
    static TINT32 SendEventRequest(SSession *pstSession, TUINT16 uwReqServiceType);
};

#endif
