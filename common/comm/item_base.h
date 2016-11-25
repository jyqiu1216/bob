#ifndef _ITEM_BASE_H_
#define _ITEM_BASE_H_

#include "base/common/wtse_std_header.h"
#include "game_define.h"
#include "aws_table_include.h"

class CItemBase
{
public:
    static TINT32 AddItem(TbBackpack *ptbBackPack, TUINT32 udwItemId, TUINT32 udwItemNum = 1);
    static TINT32 CostItem(TbBackpack *ptbBackPack, TUINT32 udwItemId, TUINT32 udwItemNum = 1);
    static TINT32 SetItem(TbBackpack *ptbBackPack, TUINT32 udwItemId, TUINT32 udwItemNum);
    static TBOOL HasEnoughItem(TbBackpack *ptbBackPack, TUINT32 udwItemId, TUINT32 udwItemNum = 1);

    static TUINT32 GetItemTypeFromGameJson(TUINT16 uwItemId);
    static TUINT32 GetItemFuncFromGameJson(TUINT16 uwItemId);
    static TUINT32 GetItemType(TUINT16 uwItemId, TUINT32 udwSid);
    static TBOOL IsChest(TUINT32 udwItemId);

    //wave
    static TINT32 CheckItemPrice(TUINT32 udwItemId, TUINT32 udwItemNum, TUINT32 udwTotalClientGem);
    static TINT32 GetItemPrice(TUINT32 udwItemId);

    //wave
    static TUINT32 GetItemNum(TbBackpack *ptbBackPack, TUINT32 udwItemId);
};

#endif
