#ifndef _BUFFER_BASE_H_
#define _BUFFER_BASE_H_

#include "player_info.h"

class CBufferBase
{

public:
    static TINT32 ComputeBuffInfo(SCityInfo *pstCity, SUserInfo *pstUser, TbIdol *atbIdol, TUINT32 udwIdolNum, TbThrone *ptbThrone, STitleInfoList *pstTitle);

    static TINT32 ComputeBasicBuff(SCityInfo *pstCity, SUserInfo *pstUser);

    static TINT32 ComputeLordSkillBuff(SCityInfo *pstCity, SUserInfo *pstUser);
    static TINT32 ComputeDragonSkillBuff(SCityInfo *pstCity, SUserInfo *pstUser);
    static TINT32 ComputeDragonMonsterSkillBuff(SCityInfo *pstCity, SUserInfo *pstUser);
    static TINT32 ComputeResearchBuff(SCityInfo *pstCity, SUserInfo *pstUser);
    static TINT32 ComputeItemBuff(SCityInfo *pstCity, SUserInfo *pstUser);
    static TINT32 ComputeWildBuff(SCityInfo *pstCity, SUserInfo *pstUser);
    static TINT32 ComputeVipBuff(SCityInfo *pstCity, SUserInfo *pstUser);
    static TINT32 ComputeEquipBuff(SCityInfo *pstCity, SUserInfo *pstUser);
    static TINT32 ComputeAltarBuff(SCityInfo *pstCity, SUserInfo* pstUser);
    static TINT32 ComputeDragonLvBuff(SCityInfo *pstCity, SUserInfo* pstUser);
    static TINT32 ComputeLordLvBuff(SCityInfo *pstCity, SUserInfo* pstUser);
    static TINT32 ComputeFortBuff(SCityInfo *pstCity, SUserInfo *pstUser);
    static TINT32 ComputeTroopBuff(SCityInfo *pstCity, SUserInfo *pstUser);
    static TINT32 ComputeIdolBuff(TbIdol *atbIdol, TUINT32 udwIdolNum, SUserInfo *pstUser);
    static TINT32 ComputeTitleBuff(STitleInfoList *pstTitle, TbThrone *ptbThrone, SUserInfo *pstUser);
    static TINT32 ComputeThroneBuff(TbThrone *ptbThrone, SUserInfo *pstUser);

    static TINT32 ComputeBuildingBuff(SCityInfo *pstCity, SUserInfo *pstUser);
    static TINT32 ComputeCaptureDragonBuff(SCityInfo *pstCity, SUserInfo *pstUser);

    static TINT32 ComputeKnightBuff(SCityInfo *pstCity, SUserInfo* pstUser);

    static TINT32 GetDragonAttackBuff(const SPlayerBuffInfo* pstBuff, TbMarch_action *ptbAction);
    static TINT32 GetDragonOccupyBuff(const SPlayerBuffInfo* pstBuff, TbMarch_action *ptbAction);

    static TINT32 MarchBuffToPlayerBuff(TbMarch_action* ptbMarch, SPlayerBuffInfo* pstBuff);
    static TINT32 GenMarchBuff(const SPlayerBuffInfo* pstBuff, TbMarch_action* ptbMarch, TBOOL bIsRallyAttack = FALSE);
    static TINT32 SetMarchBuff(SPlayerBuffInfo* pstBuff, TbMarch_action* ptbMarch);
    static TINT32 SetOccupyBuff(SPlayerBuffInfo* pstBuff, TbMarch_action* ptbMarch);
    static TINT32 GenBattleBuff(const SPlayerBuffInfo* pstBuff, SPlayerBuffInfo* pstBattleBuff, TBOOL bIsRallyAttack = FALSE, TBOOL bIsInCity = TRUE, TINT32 dwKnightLv = 0);
    static TINT32 GenOccupyBuff(const SPlayerBuffInfo* pstBuff, SPlayerBuffInfo* pstOccupyBuff);
    static TINT32 RemoveDragonBuff(SPlayerBuffInfo *pstBuff);

    static TBOOL IsBuffSame(const SPlayerBuffInfo& stBuffA, const SPlayerBuffInfo& stBuffB);
    static TINT32 ComputeBuildingForce(SCityInfo *pstCity, SUserInfo *pstUser); //for player info
    static TINT32 ComputeResearchForce(SCityInfo *pstCity, SUserInfo *pstUser); //for player info
    static TINT64 ComputeDragonLvForce(SCityInfo *pstCity, SUserInfo *pstUser); //for player info
    static TINT64 ComputeLordLvForce(SCityInfo *pstCity, SUserInfo *pstUser); //for player info

    static TINT32 GetBuffType(TUINT32 udwBuffId);

    static TINT32 GetIdolBattleBuff(TbIdol *ptbIdol, SPlayerBuffInfo* pstBuff);

};
#endif