#ifndef _BOUNTY_LOGIC_H_
#define _BOUNTY_LOGIC_H_

#include "game_info.h"
#include "game_define.h"
#include "player_info.h"


class CBountyLogic
{
public:
    static TVOID GenBountyInfo(SUserInfo *pstUser, SCityInfo *pstCity);

    static TVOID CheckBounty(SUserInfo *pstUser, SCityInfo *pstCity);

    static TVOID CheckBountyNotic(SUserInfo *pstUser, SCityInfo *pstCity);

    static TVOID SetBountyCurValue(SUserInfo *pstUser, SCityInfo *pstCity, TUINT32 udwCondiType, TUINT32 udwNumAdd = 1, TUINT32 udwId = 0, TUINT32 Value = 0);

    static TINT32 CheckBountyRefresh(SUserInfo *pstUser, SCityInfo *pstCity);

    static TBOOL CheckBountyOwnNumUpdate(SUserInfo *pstUser, SCityInfo *pstCity);

    static TINT32 CheckBaseNodeFinish(SUserInfo *pstUser, SCityInfo *pstCity);

    static TINT32 CheckGoalFinish(SUserInfo *pstUser, SCityInfo *pstCity);

    static TINT32 GetBaseNodeReward(SUserInfo *pstUser, SCityInfo *pstCity, TUINT32 udwNodeIdx, TUINT32 udwStageIdx);

    static TINT32 GetGoalReward(SUserInfo *pstUser, SCityInfo *pstCity, TUINT32 udwStageIdx);

    static TINT32 CheckBaseNodeFinishNotic(SUserInfo *pstUser, SCityInfo *pstCity);

    static TINT32 CheckGoalFinishNotic(SUserInfo *pstUser, SCityInfo *pstCity);
};


#endif
