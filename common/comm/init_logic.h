#ifndef _INIT_LOGIC_H_
#define _INIT_LOGIC_H_

#include "game_info.h"
#include "game_define.h"
#include "player_info.h"


class CInitLogic
{
public:
    static TVOID InitCityInfo(SCityInfo *pstCity, SUserInfo *pstUser, TBOOL bFake = FALSE);
    static TVOID InitPlayerInfo(SCityInfo *pstCity, SUserInfo *pstUser, TBOOL bFake = FALSE);
    

};


#endif
