#ifndef _ITEM_LOGIC_H_
#define _ITEM_LOGIC_H_

#include "player_info.h"
#include "item_base.h"
#include "buffer_base.h"

class CItemLogic
{
public:
    static TINT32 UseItem(SUserInfo *pstUser, SCityInfo *pstCity, TUINT32 udwItemId, TUINT32 udwSecClas = 0,TUINT32 udwItemNum = 1, TINT64 ddwTargetId = 0L, TBOOL bIsKing = FALSE);

    static TINT32 BuyItem(SUserInfo *pstUse, TbBackpack* ptbBackPack, TUINT32 udwItemId, TUINT32 udwItemNum = 1);
    static TINT32 BuyItem(SUserInfo *pstUse, TbBackpack* ptbBackPack, TUINT32 udwItemId, TUINT32 udwItemNum, TUINT32 udwTotalNeedGem);
    static TINT32 BuyAndUseItem(SUserInfo *pstUser, SCityInfo *pstCity, TUINT32 udwItemId, TUINT32 udwPrice, TUINT32 udwSecClass = 0, TINT64 ddwTargetId = 0L, TBOOL bIsKing = FALSE);
    static TINT32 GetItemBuffer(TUINT32 udwItemId, SBuffDetail *pstItemBuffer);

    static TINT32 TakeItemBuffer(TUINT32 udwItemId, SBuffDetail* pstItemEffect, TINT64 ddwTargetId, SCityInfo* pstCity, SUserInfo* pstUser);
    static TINT32 GetChest(SUserInfo *pstUser, SCityInfo *pstCity, TUINT32 udwChestId);

    static TBOOL IsChestLottery(TUINT32 udwChestId);
    static TINT32 GetChestLottery(SUserInfo *pstUser, TUINT32 udwChestId, SSpGlobalRes *pstGlobalRes);
    static TINT32 AppendGlobalRes(SSpGlobalRes *pstGlobalRes, SSpGlobalRes *pstGlobalResOne);

    static TINT32 GetItemreward(TUINT32 udwItemId, SSpGlobalRes *pstItemReward);

    static TINT32 SpeedUpActionByNum(TINT64 ddwTargetActionId, TUINT32 udwSecClass,TUINT32 udwEffectNum, SUserInfo* pstUser);
    static TINT32 SpeedUpActionByPercent(TINT64 ddwTargetActionId, TUINT32 udwSecondClass, TUINT32 udwEffectNum, SUserInfo* pstUser);

    static TINT32 ItemUseHanderAfter(SUserInfo * pstUserInfo, SCityInfo *pstCity, SBuffDetail* pstItemEffect, SSpGlobalRes *pstEffectInfo, TUINT32 udwItemId, TINT64 ddwTargetId);

    static TINT32 IsCanMoveCrystal(SUserInfo *pstUser, TUINT32 udwChestId, TUINT32 udwCrystalLv);

    static TINT32 CanMoveCity(SUserInfo *pstUser);
    static TINT32 HasTrialItemOrUnlock(SUserInfo *pstUser);

    static TINT32 SyncPassMarchAction(TbMarch_action *ptbAction, SUserInfo *pstUser);
};
#endif