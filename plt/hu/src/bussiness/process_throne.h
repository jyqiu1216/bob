#ifndef _PROCESS_THRONE_H_
#define _PROCESS_THRONE_H_

#include "session.h"

class CProcessThrone
{
public: 
    static TINT32 ProcessCmd_GetIdolInfo(SSession *pstSession, TBOOL &bNeedResponse);

public:
    static TINT32 ProcessCmd_GetThroneInfo(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_ThroneDismissTitle(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_ThroneDubTitle(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_ThroneAbandon(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_ThroneSetTax(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_GetTitleInfo(SSession *pstSession, TBOOL &bNeedResponse);
};

#endif
