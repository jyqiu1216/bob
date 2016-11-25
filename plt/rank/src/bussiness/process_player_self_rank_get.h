#ifndef _PROCESS_PLAYER_SELF_RANK_GET_H_
#define _PROCESS_PLAYER_SELF_RANK_GET_H_

#include "session.h"

class CProcessPlayerSelfRankGet
{
public:
    static TINT32 requestHandler(SSession* pstSession);

private:
    //player
    static TVOID WrapPlayerSelfRankJson(SSession *pstSession);
};



#endif
