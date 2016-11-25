#ifndef _PROCESS_RANK_GET_H_
#define _PROCESS_RANK_GET_H_

#include "session.h"

class CProcessRankGet
{
public:
    static TINT32 requestHandler(SSession* pstSession);

private:
    //alliance
    static TVOID NewWrapAlRankJson(SSession *pstSession);
    static TVOID NewWrapAlChampionJson(SSession *pstSession);

    //player
    static TVOID NewWrapPlayerRankJson(SSession *pstSession);
    static TVOID NewWrapPlayerChampionJson(SSession *pstSession);
};



#endif
