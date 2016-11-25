#include "item_base.h"
#include "game_info.h"
#include "service_key.h"
#include "time_utils.h"
#include "common_func.h"


TINT32 CItemBase::AddItem(TbBackpack *ptbBackPack, TUINT32 udwItemId, TUINT32 udwItemNum /*= 1*/)
{
    for(TUINT32 udwIdx = 0; udwIdx < ptbBackPack->m_bItem.m_udwNum; ++udwIdx)
    {
        if(ptbBackPack->m_bItem[udwIdx].m_ddwItemId == udwItemId)
        {
            ptbBackPack->m_bItem[udwIdx].m_ddwItemNum += udwItemNum;
            ptbBackPack->SetFlag(TbBACKPACK_FIELD_ITEM);
            return 0;
        }
    }
    if(ptbBackPack->m_bItem.m_udwNum < MAX_ITEM_TYPE_NUM)
    {
        ptbBackPack->m_bItem[ptbBackPack->m_bItem.m_udwNum].m_ddwItemId = udwItemId;
        ptbBackPack->m_bItem[ptbBackPack->m_bItem.m_udwNum].m_ddwItemNum = udwItemNum;
        ptbBackPack->m_bItem.m_udwNum++;
        ptbBackPack->SetFlag(TbBACKPACK_FIELD_ITEM);
        return 0;
    }
    return -1;
}

TINT32 CItemBase::CostItem(TbBackpack *ptbBackPack, TUINT32 udwItemId, TUINT32 udwItemNum /*= 1*/)
{
    for(TUINT32 udwIdx = 0; udwIdx < ptbBackPack->m_bItem.m_udwNum; ++udwIdx)
    {
        if(ptbBackPack->m_bItem[udwIdx].m_ddwItemId == udwItemId)
        {
            if(ptbBackPack->m_bItem[udwIdx].m_ddwItemNum <= udwItemNum)
            {
                ptbBackPack->m_bItem[udwIdx] = ptbBackPack->m_bItem[ptbBackPack->m_bItem.m_udwNum - 1];
                ptbBackPack->m_bItem.m_udwNum--;
                ptbBackPack->SetFlag(TbBACKPACK_FIELD_ITEM);
                return 0;
            }
            else
            {
                ptbBackPack->m_bItem[udwIdx].m_ddwItemNum -= udwItemNum;
            }
            ptbBackPack->SetFlag(TbBACKPACK_FIELD_ITEM);
            return 0;
        }
    }
    return -1;
}

TINT32 CItemBase::SetItem(TbBackpack *ptbBackPack, TUINT32 udwItemId, TUINT32 udwItemNum)
{
    for (TUINT32 udwIdx = 0; udwIdx < ptbBackPack->m_bItem.m_udwNum; ++udwIdx)
    {
        if (ptbBackPack->m_bItem[udwIdx].m_ddwItemId == udwItemId)
        {
            ptbBackPack->m_bItem[udwIdx].m_ddwItemNum = udwItemNum;
            ptbBackPack->SetFlag(TbBACKPACK_FIELD_ITEM);
            return 0;
        }
    }
    if (ptbBackPack->m_bItem.m_udwNum < MAX_ITEM_TYPE_NUM)
    {
        ptbBackPack->m_bItem[ptbBackPack->m_bItem.m_udwNum].m_ddwItemId = udwItemId;
        ptbBackPack->m_bItem[ptbBackPack->m_bItem.m_udwNum].m_ddwItemNum = udwItemNum;
        ptbBackPack->m_bItem.m_udwNum++;
        ptbBackPack->SetFlag(TbBACKPACK_FIELD_ITEM);
        return 0;
    }
    return -1;
}

TBOOL CItemBase::HasEnoughItem(TbBackpack *ptbBackPack, TUINT32 udwItemId, TUINT32 udwItemNum /*= 1*/)
{
    if (udwItemNum == 0)
    {
        return TRUE;
    }
    for(TUINT32 udwIdx = 0; udwIdx < ptbBackPack->m_bItem.m_udwNum; ++udwIdx)
    {
        if(ptbBackPack->m_bItem[udwIdx].m_ddwItemId == udwItemId
        && ptbBackPack->m_bItem[udwIdx].m_ddwItemNum >= udwItemNum)
        {
            return TRUE;
        }
    }
    return FALSE;
}

TUINT32 CItemBase::GetItemTypeFromGameJson(TUINT16 uwItemId)
{
    TUINT32 ItemType = 0;
    CGameInfo *poGameInfo = CGameInfo::GetInstance();
    TCHAR szItemId[32];
    sprintf(szItemId, "%u", uwItemId);
    if(poGameInfo->m_oJsonRoot["game_item"].isMember(szItemId))
    {
        ItemType = poGameInfo->m_oJsonRoot["game_item"][szItemId]["a8"].asUInt();
    }
    return ItemType;
}

TUINT32 CItemBase::GetItemFuncFromGameJson( TUINT16 uwItemId )
{
    TUINT32 ItemType = 0;
    CGameInfo *poGameInfo = CGameInfo::GetInstance();
    TCHAR szItemId[32];
    sprintf(szItemId, "%u", uwItemId);
    if(poGameInfo->m_oJsonRoot["game_item"].isMember(szItemId))
    {
        ItemType = poGameInfo->m_oJsonRoot["game_item"][szItemId]["a2"].asUInt();
    }
    return ItemType;
}

TUINT32 CItemBase::GetItemType(TUINT16 uwItemId, TUINT32 udwSid)
{
    TUINT32 udwItemTypeGameJson = CItemBase::GetItemTypeFromGameJson(uwItemId);

    return udwItemTypeGameJson;
}

TBOOL CItemBase::IsChest(TUINT32 udwItemId)
{
    CGameInfo *poGameInfo = CGameInfo::GetInstance();
    if(poGameInfo->m_oJsonRoot["game_chest"].isMember(CCommonFunc::NumToString(udwItemId)))
    {
        return TRUE;
    }
    return FALSE;
}

TINT32 CItemBase::CheckItemPrice( TUINT32 udwItemId, TUINT32 udwItemNum, TUINT32 udwTotalClientGem )
{
    TINT32 udwPrice = GetItemPrice(udwItemId);
    if(udwPrice < 0)
    {
        return -1;
    }

    if(udwItemNum*udwPrice != udwTotalClientGem)
    {
        return -2;
    }

    return 0;
}

TINT32 CItemBase::GetItemPrice( TUINT32 udwItemId )
{
    SItemInfo stItemInfo;
    stItemInfo.Reset();

    // 1. 获取属性信息
    if(FALSE == CGameInfo::GetInstance()->GetItemInfo(udwItemId, &stItemInfo))
    {
        return -1;
    }

    return stItemInfo.m_audwProperty[EN_PROP_STATUS__PRICE];
}

TUINT32 CItemBase::GetItemNum( TbBackpack *ptbBackPack, TUINT32 udwItemId )
{
    TUINT32 udwItemNum = 0;
    for(TUINT32 udwIdx = 0; udwIdx < ptbBackPack->m_bItem.m_udwNum; ++udwIdx)
    {
        if(ptbBackPack->m_bItem[udwIdx].m_ddwItemId == udwItemId)
        {
            udwItemNum =  ptbBackPack->m_bItem[udwIdx].m_ddwItemNum;
            break;
        }
    }
    return udwItemNum;
}
