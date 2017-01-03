#include "item_logic.h"
#include "game_info.h"
#include "time_utils.h"
#include "city_info.h"
#include "service_key.h"
#include "action_base.h"
#include "player_info.h"
#include "city_base.h"
#include "player_base.h"
#include "common_base.h"
#include "sendmessage_base.h"
#include "buffer_base.h"
#include "quest_logic.h"
#include "action_base.h"
#include "globalres_logic.h"
#include "game_define.h"
#include "activities_logic.h"
#include "chest_lottery.h"

TINT32 CItemLogic::TakeItemBuffer(TUINT32 udwItemId, SBuffDetail* pstItemBuffer, TINT64 ddwTargetId, SCityInfo* pstCity, SUserInfo* pstUser)
{
    //vip etime
    if(pstItemBuffer->m_udwId == EN_BUFFER_INFO_VIP_ACTIVATE)
    {
        return CCommonBase::AddVipTime(pstUser, pstCity, pstItemBuffer->m_dwTime);
    }

    TbAction *ptbAction = NULL;
    UActionParam stParam;
    stParam.Reset();

    if(pstItemBuffer->m_dwTime == 0 &&
        pstItemBuffer->m_udwId == 0 &&
        pstItemBuffer->m_ddwNum == 0)
    {
        return 0;
    }

    if(pstItemBuffer->m_udwId < EN_BUFFER_INFO_END && pstItemBuffer->m_udwId > EN_BUFFER_INFO_BEGIN)
    {
        ptbAction = CActionBase::GetActionByBufferId(&pstUser->m_atbAction[0], pstUser->m_udwActionNum, pstItemBuffer->m_udwId);
    }
    if(ptbAction == NULL)
    {
        //无此类action
        stParam.m_stItem.SetValue(pstItemBuffer->m_udwId, pstItemBuffer->m_ddwNum, pstItemBuffer->m_dwTime, udwItemId);

        CCommonBase::AddAction(pstUser, pstCity, EN_ACTION_MAIN_CLASS__ITEM, EN_ACTION_SEC_CLASS__ITEM,
            EN_ITEM_STATUS__USING, pstItemBuffer->m_dwTime, &stParam);

        TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("CItemLogic::TakeItemBuffer add action :uid=%u item_id=%u [bufferId=%u bufferNum=%d bufferTime=%u]",
            pstUser->m_tbPlayer.Get_Uid(),
            udwItemId,
            pstItemBuffer->m_udwId,
            pstItemBuffer->m_ddwNum,
            pstItemBuffer->m_dwTime));
    }
    else
    {
        //if(ptbAction->m_bParam[0].m_stItem.m_udwNum == pstItemBuffer->m_udwNum)
        //{
        //    //效果相同 延长时间
        //    ptbAction->Set_Etime(pstItemBuffer->m_dwTime + ptbAction->Get_Etime());
        //    ptbAction->Set_Ctime(pstItemBuffer->m_dwTime + ptbAction->Get_Ctime());
        //    ptbAction->m_bParam[0].m_stItem.SetValue(pstItemBuffer->m_udwId, pstItemBuffer->m_udwNum, pstItemBuffer->m_dwTime, udwItemId);

        //    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("CItemLogic::TakeItemBuffer have same action add action time:uid=%u item_id=%u [bufferId=%u bufferNum=%u bufferTime=%u] [action_param bufferId=%u num=%u time=%d]",
        //        pstUser->m_tbPlayer.Get_Uid(),
        //        udwItemId,
        //        pstItemBuffer->m_udwId,
        //        pstItemBuffer->m_udwNum,
        //        pstItemBuffer->m_dwTime,
        //        ptbAction->m_bParam[0].m_stItem.m_udwBufferId,
        //        ptbAction->m_bParam[0].m_stItem.m_udwNum,
        //        ptbAction->m_bParam[0].m_stItem.m_udwTime));
        //}
        //else
        //{
        //    //效果不同
            ptbAction->m_bParam[0].m_stItem.SetValue(pstItemBuffer->m_udwId, pstItemBuffer->m_ddwNum, pstItemBuffer->m_dwTime, udwItemId);

            ptbAction->Set_Btime(CTimeUtils::GetUnixTime());
            ptbAction->Set_Ctime(pstItemBuffer->m_dwTime);
            ptbAction->Set_Etime(pstItemBuffer->m_dwTime + CTimeUtils::GetUnixTime());

            ptbAction->SetFlag(TbACTION_FIELD_PARAM);

            TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("CItemLogic::TakeItemBuffer have replace action:uid=%u item_id=%u [bufferId=%u bufferNum=%d bufferTime=%u] [action_param bufferId=%u num=%d time=%d]",
                pstUser->m_tbPlayer.Get_Uid(),
                udwItemId,
                pstItemBuffer->m_udwId,
                pstItemBuffer->m_ddwNum,
                pstItemBuffer->m_dwTime,
                ptbAction->m_bParam[0].m_stItem.m_ddwBufferId,
                ptbAction->m_bParam[0].m_stItem.m_ddwNum,
                ptbAction->m_bParam[0].m_stItem.m_ddwTime));

        //}
        TUINT32 udwActionIdx = CActionBase::GetActionIndex(&pstUser->m_atbAction[0], pstUser->m_udwActionNum, ptbAction->m_nId);
        pstUser->m_aucActionFlag[udwActionIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
    }
    return 0;
}

TINT32 CItemLogic::SpeedUpActionByNum(TINT64 uddwTargetId, TUINT32 udwSecondClass, TUINT32 udwEffectNum, SUserInfo* pstUser)
{
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();

    TUINT32 udwActionType = CActionBase::GetActionTypeBySecondClass(udwSecondClass);
    TINT32 dwActionIdx = -1;

    // 0. 获取action
    switch(udwActionType)
    {
    case EN_ACTION_TYPE_BUFF_NORMAL:
        dwActionIdx = CActionBase::GetActionIndex(pstUser->m_atbAction, pstUser->m_udwActionNum, uddwTargetId);
        break;
    case EN_ACTION_TYPE_AL_CAN_HELP:
        dwActionIdx = CActionBase::GetAlActionIndex(pstUser->m_atbSelfAlAction, pstUser->m_udwSelfAlActionNum, uddwTargetId);
        break;
    case EN_ACTION_TYPE_MARCH:
        dwActionIdx = CActionBase::GetMarchIndex(pstUser->m_atbMarch, pstUser->m_udwMarchNum, uddwTargetId);
        break;
    }
    if(dwActionIdx >= 0)
    {
        switch(udwActionType)
        {
        case EN_ACTION_TYPE_BUFF_NORMAL:
            if(pstUser->m_atbAction[dwActionIdx].m_nEtime - udwEffectNum < udwCurTime)
            {
                pstUser->m_atbAction[dwActionIdx].Set_Etime(udwCurTime);
            }
            else
            {
                pstUser->m_atbAction[dwActionIdx].Set_Etime(pstUser->m_atbAction[dwActionIdx].m_nEtime - udwEffectNum);
            }
            pstUser->m_aucActionFlag[dwActionIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
            break;
        case EN_ACTION_TYPE_AL_CAN_HELP:
            if(pstUser->m_atbSelfAlAction[dwActionIdx].m_nEtime - udwEffectNum < udwCurTime)
            {
                pstUser->m_atbSelfAlAction[dwActionIdx].Set_Etime(udwCurTime);
            }
            else
            {
                pstUser->m_atbSelfAlAction[dwActionIdx].Set_Etime(pstUser->m_atbSelfAlAction[dwActionIdx].m_nEtime - udwEffectNum);
            }
            pstUser->m_aucSelfAlActionFlag[dwActionIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
            break;
        case EN_ACTION_TYPE_MARCH:
            if(pstUser->m_atbMarch[dwActionIdx].m_nEtime - udwEffectNum < udwCurTime)
            {
                pstUser->m_atbMarch[dwActionIdx].Set_Etime(udwCurTime);
            }
            else
            {
                pstUser->m_atbMarch[dwActionIdx].Set_Etime(pstUser->m_atbMarch[dwActionIdx].m_nEtime - udwEffectNum);
            }
            pstUser->m_aucMarchFlag[dwActionIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
            break;
        }
    }
    else
    {
        return -2;
    }

    return 0;

}

TINT32 CItemLogic::SpeedUpActionByPercent(TINT64 uddwTargetId, TUINT32 udwSecondClass, TUINT32 udwEffectNum, SUserInfo* pstUser)
{
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    TUINT32 udwReduceTime = 0;

    TUINT32 udwActionType = CActionBase::GetActionTypeBySecondClass(udwSecondClass);
    TINT32 dwActionIdx = -1;

    //兼容
    if (udwActionType == EN_ACTION_TYPE_AL_CAN_HELP)
    {
        dwActionIdx = CActionBase::GetAlActionIndex(pstUser->m_atbSelfAlAction, pstUser->m_udwSelfAlActionNum, uddwTargetId);
        if (dwActionIdx == -1)
        {
            udwActionType = EN_ACTION_TYPE_BUFF_NORMAL;
        }
    }
    dwActionIdx = -1;

    // 0. 获取action
    switch(udwActionType)
    {
    case EN_ACTION_TYPE_BUFF_NORMAL:
        dwActionIdx = CActionBase::GetActionIndex(pstUser->m_atbAction, pstUser->m_udwActionNum, uddwTargetId);
        break;
    case EN_ACTION_TYPE_AL_CAN_HELP:
        dwActionIdx = CActionBase::GetAlActionIndex(pstUser->m_atbSelfAlAction, pstUser->m_udwSelfAlActionNum, uddwTargetId);
        break;
    case EN_ACTION_TYPE_MARCH:
        dwActionIdx = CActionBase::GetMarchIndex(pstUser->m_atbMarch, pstUser->m_udwMarchNum, uddwTargetId);
        break;
    }
    if(dwActionIdx >= 0)
    {
        switch(udwActionType)
        {
        case EN_ACTION_TYPE_BUFF_NORMAL:
            udwReduceTime = (pstUser->m_atbAction[dwActionIdx].m_nEtime - udwCurTime) * udwEffectNum / 10000;
            if(pstUser->m_atbAction[dwActionIdx].m_nEtime - udwReduceTime < udwCurTime)
            {
                pstUser->m_atbAction[dwActionIdx].Set_Etime(udwCurTime);
            }
            else
            {
                pstUser->m_atbAction[dwActionIdx].Set_Etime(pstUser->m_atbAction[dwActionIdx].m_nEtime - udwReduceTime);
            }
            pstUser->m_aucActionFlag[dwActionIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
            break;
        case EN_ACTION_TYPE_AL_CAN_HELP:
            udwReduceTime = (pstUser->m_atbSelfAlAction[dwActionIdx].m_nEtime - udwCurTime) * udwEffectNum / 10000;
            if(pstUser->m_atbSelfAlAction[dwActionIdx].m_nEtime - udwReduceTime < udwCurTime)
            {
                pstUser->m_atbSelfAlAction[dwActionIdx].Set_Etime(udwCurTime);
            }
            else
            {
                pstUser->m_atbSelfAlAction[dwActionIdx].Set_Etime(pstUser->m_atbSelfAlAction[dwActionIdx].m_nEtime - udwReduceTime);
            }
            pstUser->m_aucSelfAlActionFlag[dwActionIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
            break;
        case EN_ACTION_TYPE_MARCH:
            udwReduceTime = (pstUser->m_atbMarch[dwActionIdx].m_nEtime - udwCurTime) * udwEffectNum / 10000;
            if(pstUser->m_atbMarch[dwActionIdx].m_nEtime - udwReduceTime < udwCurTime)
            {
                pstUser->m_atbMarch[dwActionIdx].Set_Etime(udwCurTime);
            }
            else
            {
                pstUser->m_atbMarch[dwActionIdx].Set_Etime(pstUser->m_atbMarch[dwActionIdx].m_nEtime - udwReduceTime);
            }
            pstUser->m_aucMarchFlag[dwActionIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
            SyncPassMarchAction(&pstUser->m_atbMarch[dwActionIdx], pstUser);
            break;
        }
    }
    else
    {
        return -2;
    }

    return 0;

}

TINT32 CItemLogic::BuyItem(SUserInfo *pstUser, TbBackpack* ptbBackPack, TUINT32 udwItemId, TUINT32 udwItemNum /*= 1*/)
{
    SItemInfo stItemInfo;
    stItemInfo.Reset();

    TbLogin *ptbLogin = &pstUser->m_tbLogin;
    // 1. 获取属性信息
    if(FALSE == CGameInfo::GetInstance()->GetItemInfo(udwItemId, &stItemInfo))
    {
        return EN_RET_CODE__TARGET_NOT_EXIST;
    }

    // 2. 判定资源是否足够
    TUINT32 udwNeedGem = stItemInfo.m_audwProperty[EN_PROP_STATUS__SALE_PRICE] * udwItemNum;
    if(FALSE == CPlayerBase::HasEnoughGem(ptbLogin, udwNeedGem))
    {
        return EN_RET_CODE__GEM_LACK;
    }

    // 3. 消耗资源
    CPlayerBase::CostGem(pstUser, udwNeedGem);

    // 4. 增加物品
    CItemBase::AddItem(ptbBackPack, udwItemId, udwItemNum);

    return 0;
}

TINT32 CItemLogic::BuyItem(SUserInfo *pstUse, TbBackpack* ptbBackPack, TUINT32 udwItemId, TUINT32 udwItemNum, TUINT32 udwTotalNeedGem)
{
    CGameInfo *poGameInfo = CGameInfo::GetInstance();
    SItemInfo stItemInfo;

    TbLogin *ptbLogin = &pstUse->m_tbLogin;
    // 获取属性信息
    if(FALSE == poGameInfo->GetItemInfo(udwItemId, &stItemInfo))
    {
        return EN_RET_CODE__TARGET_NOT_EXIST;
    }

    // 2. 判定资源是否足够
    if(FALSE == CPlayerBase::HasEnoughGem(ptbLogin, udwTotalNeedGem))
    {
        return EN_RET_CODE__GEM_LACK;
    }

    // 消耗资源

    CPlayerBase::CostGem(pstUse, udwTotalNeedGem);

    // 4. 增加物品
    CItemBase::AddItem(ptbBackPack, udwItemId, udwItemNum);

    return 0;
}

TINT32 CItemLogic::UseItem(SUserInfo *pstUser, SCityInfo *pstCity, TUINT32 udwItemId, TUINT32 udwSecClas /* = 0 */, TUINT32 udwItemNum /*= 1*/, TINT64 ddwTargetId /*= 0L*/, TBOOL bIsKing /*= FALSE*/)
{
    TINT32 dwRetCode = 0;
    SSpGlobalRes stEffectInfo;
    stEffectInfo.Reset();
    if((dwRetCode = GetItemreward(udwItemId, &stEffectInfo)) != 0)
    {
        return dwRetCode;
    }

    SBuffDetail stBufferDetail;
    stBufferDetail.Reset();
    if((dwRetCode = GetItemBuffer(udwItemId, &stBufferDetail)) != 0)
    {
        return dwRetCode;
    }

    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("CItemLogic::UseItem:uid=%u item_id=%u [type=%u effect_id=%u effect_num=%d] [buffer_type=%u buffer_time=%u buffer_num=%d]",
        pstUser->m_tbPlayer.Get_Uid(),
        udwItemId,
        stEffectInfo.aRewardList[0].udwType,
        stEffectInfo.aRewardList[0].udwId,
        stEffectInfo.aRewardList[0].udwNum,
        stBufferDetail.m_udwId,
        stBufferDetail.m_dwTime,
        stBufferDetail.m_ddwNum));
    //peacetime 特殊处理
    if(stBufferDetail.m_udwId == EN_BUFFER_INFO_PEACE_TIME)
    {
        if(pstUser->m_tbPlayer.m_nStatus & EN_CITY_STATUS__NEW_PROTECTION)//新手保护期间 不让用peacetime物品
        {
            return EN_RET_CODE__PEACETIME_ON_NEW_PROTECT;
        }
        
        if(pstCity->m_stActionStat.m_bCanPeaceTime == FALSE)
        {
            return EN_RET_CODE__CAN_NOT_PEACETIME;
        }

        if (pstUser->m_tbPlayer.m_nStatus & EN_CITY_STATUS__ON_SPECIAL_WILD) //在特殊地形上 不可以使用peacetime
        {
            return EN_RET_CODE__CAN_NOT_PEACETIME_ON_SPECIAL;
        }

        if (bIsKing)
        {
            return EN_RET_CODE__KING_CANNOT_USE_PEACE_TIME;
        }
    }
    // 龙不在城市中不能加经验、体力
//     if (pstUser->m_tbPlayer.m_nDragon_status != EN_DRAGON_STATUS_NORMAL)
//     {
//         for (TUINT32 udwIdx = 0; udwIdx < stEffectInfo.udwTotalNum; ++udwIdx)
//         {
//             if (stEffectInfo.aRewardList[udwIdx].udwType == EN_GLOBALRES_TYPE_DRAGON_EXP ||
//                 stEffectInfo.aRewardList[udwIdx].udwType == EN_GLOBALRES_TYPE_DRAGON_ENERGY)
//             {
//                 return EN_RET_CODE__DRAGON_NOT_IN_CITY;
//             }
//         }
//     }
    //对经验类物品的使用做保护
    for(TUINT32 udwIdx = 0; udwIdx < stEffectInfo.udwTotalNum; ++udwIdx)
    {
        // vip point
        if (stEffectInfo.aRewardList[udwIdx].udwType == EN_GLOBALRES_TYPE_VIP_POINT && pstUser->m_tbPlayer.m_nVip_point >= CPlayerBase::GetMaxVipPoint())
        {
            return EN_RET_CODE__VIP_LEVEL_REACH_THE_TOP;
        }

        if (pstUser->m_tbPlayer.m_nHas_dragon == 0)
        {
            if (stEffectInfo.aRewardList[udwIdx].udwType == EN_GLOBALRES_TYPE_DRAGON_ENERGY
                || stEffectInfo.aRewardList[udwIdx].udwType == EN_GLOBALRES_TYPE_DRAGON_EXP)
            {
                return EN_RET_CODE__PLAYER_HAVE_NO_DRAGON;
            }
        }
        // hero exp
        if (stEffectInfo.aRewardList[udwIdx].udwType == EN_GLOBALRES_TYPE_DRAGON_EXP)
        {
            if(pstUser->m_tbPlayer.m_nDragon_exp >= CPlayerBase::GetMaxDragonExp(pstUser->m_tbPlayer.m_nDragon_max_lv))
            {
                return EN_RET_CODE__DRAGON_EXP_REACH_THE_TOP;
            }
        }
        if (stEffectInfo.aRewardList[udwIdx].udwType == EN_GLOBALRES_TYPE_DRAGON_EXP
            && pstUser->m_tbPlayer.m_nDragon_status != EN_DRAGON_STATUS_NORMAL
            && pstUser->m_tbPlayer.m_nDragon_status != EN_DRAGON_STATUS_MARCH)
        {
            return EN_RET_CODE__HERO_IS_INVALID_TO_ADD_EXP;
        }

        //这个udwMaxEnergy的逻辑见CPlayerBase::AddHeroEnergy
        if(stEffectInfo.aRewardList[udwIdx].udwType == EN_GLOBALRES_TYPE_DRAGON_ENERGY && pstUser->m_tbPlayer.m_nDragon_cur_energy >= CPlayerBase::GetCurDragonMaxEnergy(pstUser))
        {
            return EN_RET_CODE__HERO_ENERGY_REACH_THE_TOP;
        }
        if (stEffectInfo.aRewardList[udwIdx].udwType == EN_GLOBALRES_TYPE_DRAGON_ENERGY
            && pstUser->m_tbPlayer.m_nDragon_status != EN_DRAGON_STATUS_NORMAL
            && pstUser->m_tbPlayer.m_nDragon_status != EN_DRAGON_STATUS_MARCH)
        {
            return EN_RET_CODE__HERO_IS_INVALID_TO_ADD_ENERGY;
        }

        if (stEffectInfo.aRewardList[udwIdx].udwType == EN_GLOBALRES_TYPE_DRAGON_SHARD
            && pstUser->m_tbPlayer.m_nHas_dragon == 0)
        {
            return EN_RET_CODE__CANNOT_USER_DRAGON_SHARD;
        }
    }

    //take effect
    for(TUINT16 idx = 0; idx < udwItemNum; idx++)
    {
        if(0 != (dwRetCode = CGlobalResLogic::AddSpGlobalRes(pstUser, pstCity, &stEffectInfo, udwSecClas,ddwTargetId, udwItemId)))
        {
            return dwRetCode;
        }
        if(0 != (dwRetCode = CItemLogic::TakeItemBuffer(udwItemId, &stBufferDetail, ddwTargetId, pstCity, pstUser)))
        {
            return dwRetCode;
        }
    }

    //chest
    TCHAR szItemId[64];
    sprintf(szItemId, "%u", udwItemId);

    TUINT32 udwItemCategry = CGameInfo::GetInstance()->m_oJsonRoot["game_item"][szItemId]["a1"].asUInt();
    if(EN_ITEM_CATEGORY__CHEST == udwItemCategry)
    {
        dwRetCode = GetChest(pstUser, pstCity, udwItemId);
    }

    //3 cost item 
    CItemBase::CostItem(&pstUser->m_tbBackpack, udwItemId);

    CItemLogic::ItemUseHanderAfter(pstUser, pstCity, &stBufferDetail, &stEffectInfo, udwItemId, ddwTargetId);

    return dwRetCode;
}

TINT32 CItemLogic::BuyAndUseItem(SUserInfo *pstUser, SCityInfo *pstCity, TUINT32 udwItemId, TUINT32 udwPrice, TUINT32 udwSecClass/* = 0*/, TINT64 ddwTargetId /*= 0L*/, TBOOL bIsKing /*= FALSE*/)
{
    TINT32 dwRetCode = 0;
    //CGameInfo *poGameInfo = CGameInfo::GetInstance();


    SSpGlobalRes stEffectInfo;
    stEffectInfo.Reset();
    if((dwRetCode = GetItemreward(udwItemId, &stEffectInfo)) != 0)
    {
        return dwRetCode;
    }

    SBuffDetail stBufferDetail;
    stBufferDetail.Reset();
    if((dwRetCode = GetItemBuffer(udwItemId, &stBufferDetail)) != 0)
    {
        return dwRetCode;
    }

    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("CItemLogic::UseItem:uid=%u item_id=%u [type=%u effect_id=%u effect_num=%u] [buffer_type=%u buffer_time=%u buffer_num=%d]",
        pstUser->m_tbPlayer.Get_Uid(),
        udwItemId,
        stEffectInfo.aRewardList[0].udwType,
        stEffectInfo.aRewardList[0].udwId,
        stEffectInfo.aRewardList[0].udwNum,
        stBufferDetail.m_udwId,
        stBufferDetail.m_dwTime,
        stBufferDetail.m_ddwNum));

    //各种期间 不让用peacetime物品
    if(stBufferDetail.m_udwId == EN_BUFFER_INFO_PEACE_TIME)
    {
        if (pstUser->m_tbPlayer.m_nStatus & EN_CITY_STATUS__NEW_PROTECTION)
        {
            return EN_RET_CODE__PEACETIME_ON_NEW_PROTECT;
        }

        if (pstCity->m_stActionStat.m_bCanPeaceTime == FALSE)
        {
            return EN_RET_CODE__CAN_NOT_PEACETIME;
        }

        if (pstUser->m_tbPlayer.m_nStatus & EN_CITY_STATUS__ON_SPECIAL_WILD) //在特殊地形上 不可以使用peacetime
        {
            return EN_RET_CODE__CAN_NOT_PEACETIME_ON_SPECIAL;
        }

        if (bIsKing)
        {
            return EN_RET_CODE__KING_CANNOT_USE_PEACE_TIME;
        }
    }
    // 龙不在城市中不能加经验、体力
//     if (pstUser->m_tbPlayer.m_nDragon_status != EN_DRAGON_STATUS_NORMAL)
//     {
//         for (TUINT32 udwIdx = 0; udwIdx < stEffectInfo.udwTotalNum; ++udwIdx)
//         {
//             if (stEffectInfo.aRewardList[udwIdx].udwType == EN_GLOBALRES_TYPE_DRAGON_EXP ||
//                 stEffectInfo.aRewardList[udwIdx].udwType == EN_GLOBALRES_TYPE_DRAGON_ENERGY)
//             {
//                 return EN_RET_CODE__DRAGON_NOT_IN_CITY;
//             }
//         }
//     }
    //对经验类物品的使用做保护
    for(TUINT32 udwIdx = 0; udwIdx < stEffectInfo.udwTotalNum; ++udwIdx)
    {
        // vip point
        if (stEffectInfo.aRewardList[udwIdx].udwType == EN_GLOBALRES_TYPE_VIP_POINT && pstUser->m_tbPlayer.m_nVip_point >= CPlayerBase::GetMaxVipPoint())
        {
            return EN_RET_CODE__VIP_LEVEL_REACH_THE_TOP;
        }

        if (pstUser->m_tbPlayer.m_nHas_dragon == 0)
        {
            if (stEffectInfo.aRewardList[udwIdx].udwType == EN_GLOBALRES_TYPE_DRAGON_ENERGY
                || stEffectInfo.aRewardList[udwIdx].udwType == EN_GLOBALRES_TYPE_DRAGON_EXP)
            {
                return EN_RET_CODE__PLAYER_HAVE_NO_DRAGON;
            }
        }

        // hero exp
        if (stEffectInfo.aRewardList[udwIdx].udwType == EN_GLOBALRES_TYPE_DRAGON_EXP)
        {
            if (pstUser->m_tbPlayer.m_nDragon_exp >= CPlayerBase::GetMaxDragonExp(pstUser->m_tbPlayer.m_nDragon_max_lv))
            {
                return EN_RET_CODE__DRAGON_EXP_REACH_THE_TOP;
            }
        }
        if (stEffectInfo.aRewardList[udwIdx].udwType == EN_GLOBALRES_TYPE_DRAGON_EXP
            && pstUser->m_tbPlayer.m_nDragon_status != EN_DRAGON_STATUS_NORMAL
            && pstUser->m_tbPlayer.m_nDragon_status != EN_DRAGON_STATUS_MARCH)
        {
            return EN_RET_CODE__HERO_IS_INVALID_TO_ADD_EXP;
        }

        //这个udwMaxEnergy的逻辑见CPlayerBase::AddHeroEnergy
        if (stEffectInfo.aRewardList[udwIdx].udwType == EN_GLOBALRES_TYPE_DRAGON_ENERGY && pstUser->m_tbPlayer.m_nDragon_cur_energy >= CPlayerBase::GetCurDragonMaxEnergy(pstUser))
        {
            return EN_RET_CODE__HERO_ENERGY_REACH_THE_TOP;
        }
        if (stEffectInfo.aRewardList[udwIdx].udwType == EN_GLOBALRES_TYPE_DRAGON_ENERGY
            && pstUser->m_tbPlayer.m_nDragon_status != EN_DRAGON_STATUS_NORMAL
            && pstUser->m_tbPlayer.m_nDragon_status != EN_DRAGON_STATUS_MARCH)
        {
            return EN_RET_CODE__HERO_IS_INVALID_TO_ADD_ENERGY;
        }

        if (stEffectInfo.aRewardList[udwIdx].udwType == EN_GLOBALRES_TYPE_DRAGON_SHARD
            && pstUser->m_tbPlayer.m_nHas_dragon == 0)
        {
            return EN_RET_CODE__CANNOT_USER_DRAGON_SHARD;
        }
    }

    if(0 != (dwRetCode = CGlobalResLogic::AddSpGlobalRes(pstUser, pstCity, &stEffectInfo, udwSecClass, ddwTargetId, udwItemId)))
    {
        return dwRetCode;
    }
    if(0 != (dwRetCode = CItemLogic::TakeItemBuffer(udwItemId, &stBufferDetail, ddwTargetId, pstCity, pstUser)))
    {
        return dwRetCode;
    }


    //chest
    TCHAR szItemId[64];
    sprintf(szItemId, "%u", udwItemId);

    TUINT32 udwItemCategry = CGameInfo::GetInstance()->m_oJsonRoot["game_item"][szItemId]["a1"].asUInt();
    if(EN_ITEM_CATEGORY__CHEST == udwItemCategry)
    {
        dwRetCode = GetChest(pstUser, pstCity, udwItemId);
    }

    CPlayerBase::CostGem(pstUser, udwPrice);

    CItemLogic::ItemUseHanderAfter(pstUser, pstCity, &stBufferDetail, &stEffectInfo, udwItemId, ddwTargetId);

    return 0;
}


TINT32 CItemLogic::GetItemreward(TUINT32 udwItemId, SSpGlobalRes *pstItemReward)
{
    if(NULL == pstItemReward)
    {
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("GetItemEffect:pstItemEffect NULL"));
        return -1;
    }

    pstItemReward->Reset();
    CGameInfo * pstGameInfo = CGameInfo::GetInstance();
    TCHAR szItemId[64];
    sprintf(szItemId, "%u", udwItemId);
    if(!pstGameInfo->m_oJsonRoot["game_item"].isMember(szItemId))
    {
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("GetItemEffect:game.json not include item_id=%u [ret=-2]", udwItemId));
        return EN_RET_CODE__REQ_PARAM_ERROR;
    }

    for(TUINT32 udwIdx = 0; udwIdx < pstGameInfo->m_oJsonRoot["game_item"][szItemId]["a14"].size(); ++udwIdx)
    {
        pstItemReward->aRewardList[pstItemReward->udwTotalNum].udwType = pstGameInfo->m_oJsonRoot["game_item"][szItemId]["a14"][udwIdx][0U].asUInt();
        pstItemReward->aRewardList[pstItemReward->udwTotalNum].udwId = pstGameInfo->m_oJsonRoot["game_item"][szItemId]["a14"][udwIdx][1U].asUInt();
        pstItemReward->aRewardList[pstItemReward->udwTotalNum].udwNum = pstGameInfo->m_oJsonRoot["game_item"][szItemId]["a14"][udwIdx][2U].asUInt();
        pstItemReward->udwTotalNum++;

    }

    return 0;
}


TINT32 CItemLogic::GetItemBuffer(TUINT32 udwItemId, SBuffDetail *pstItemBuffer)
{
    if(NULL == pstItemBuffer)
    {
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("GetItemBuffer:pstItemBuffer NULL"));
        return -1;
    }

    pstItemBuffer->Reset();
    CGameInfo * pstGameInfo = CGameInfo::GetInstance();
    TCHAR szItemId[64];
    sprintf(szItemId, "%u", udwItemId);
    if(!pstGameInfo->m_oJsonRoot["game_item"].isMember(szItemId))
    {
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("GetItemBuffer:game.json not include item_id=%u [ret=-2]", udwItemId));
        return EN_RET_CODE__REQ_PARAM_ERROR;
    }

    pstItemBuffer->m_udwId = pstGameInfo->m_oJsonRoot["game_item"][szItemId]["a13"][0U].asUInt();
    pstItemBuffer->m_ddwNum = pstGameInfo->m_oJsonRoot["game_item"][szItemId]["a13"][1U].asInt();
    pstItemBuffer->m_dwTime = pstGameInfo->m_oJsonRoot["game_item"][szItemId]["a13"][2U].asUInt();

    return 0;
}


TINT32 CItemLogic::GetChest(SUserInfo *pstUser, SCityInfo *pstCity, TUINT32 udwChestId)
{
    TINT32 dwRetCode = 0;

    SSpGlobalRes stGlobalRes;
    stGlobalRes.Reset();

    //检测是否抽奖箱子
    TCHAR szChestId[32];
    sprintf(szChestId, "%u", udwChestId);
    CGameInfo *poGameInfo = CGameInfo::GetInstance();
    if(!poGameInfo->m_oJsonRoot["game_chest"].isMember(szChestId))
    {
        return -1;
    }

    //获取奖励信息
    // 怪物箱子
//     if ((udwChestId >= 502 && udwChestId <= 506) ||
//         udwChestId == 344)
//     {
//         dwRetCode = CItemLogic::GetChestCandidate(pstUser, pstCity, udwChestId, &stGlobalRes);
//     }
    if (CItemLogic::IsChestLottery(udwChestId))
    {
        dwRetCode = CItemLogic::GetChestLottery(pstUser, udwChestId, &stGlobalRes);
    }
    else
    {
        dwRetCode = CGlobalResLogic::GetLotteryChestGlobalResInfo(udwChestId, &stGlobalRes);
    }
    if(dwRetCode != 0)
    {
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("CProcessItemUse:GetChest: get reward info fail [uid=%ld chestId=%u] [ret=%d]", pstUser->m_tbPlayer.Get_Uid(), udwChestId, dwRetCode));
        return dwRetCode;
    }
    //获取奖励
    dwRetCode = CGlobalResLogic::AddSpGlobalRes(pstUser, pstCity, &stGlobalRes);
    if(dwRetCode != 0)
    {
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("CProcessItemUse:GetChest: take reward fail [uid=%ld chestId=%u] [ret=%d]", pstUser->m_tbPlayer.Get_Uid(), udwChestId, dwRetCode));
        return dwRetCode;
    }

    // 抽奖记录
    pstUser->m_udwLotteryChestItemId = udwChestId;
    pstUser->m_udwLotteryChestItemNum = 1;
    pstUser->m_stRewardWindow = stGlobalRes;

    Json::Value jTmp = Json::Value(Json::objectValue);
    jTmp["chest"] = Json::Value(Json::objectValue);
    jTmp["chest"]["id"] = udwChestId;
    jTmp["chest"]["num"] = 1;

    CCommonBase::AddRewardWindow(pstUser, pstUser->m_tbPlayer.m_nUid, EN_REWARD_WIMDOW_TYPE_CHEST, EN_REWARD_WINDOW_GET_TYPE_OPEN_CHEST,
        0, &stGlobalRes, FALSE, jTmp);

    return 0;
}

TBOOL CItemLogic::IsChestLottery(TUINT32 udwChestId)
{
    TCHAR szChestId[32];
    sprintf(szChestId, "%u", udwChestId);
    CChestLottery *poChestLottery = CChestLottery::GetInstance();
    if (poChestLottery->m_oJsonRoot.isMember("chest_lottery") &&
        poChestLottery->m_oJsonRoot["chest_lottery"].isMember(szChestId))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

TINT32 CItemLogic::GetChestLottery(SUserInfo *pstUser, TUINT32 udwChestId, SSpGlobalRes *pstGlobalRes)
{
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;

    TCHAR szChestId[MAX_TABLE_NAME_LEN];
    snprintf(szChestId, MAX_TABLE_NAME_LEN, "%u", udwChestId);
    CChestLottery *poChestLottery = CChestLottery::GetInstance();

    if (!poChestLottery->m_oJsonRoot.isMember("chest_lottery") ||
        !poChestLottery->m_oJsonRoot["chest_lottery"].isMember(szChestId) ||
        poChestLottery->m_oJsonRoot["chest_lottery"][szChestId].size() == 0)
    {
        return -1;
    }

    TUINT32 udwSeqNum = poChestLottery->m_oJsonRoot["chest_lottery"][szChestId].size();

    TINT32 dwPos = -1;
    for (TUINT32 udwIdx = 0; udwIdx < ptbPlayer->m_bChest_lottery.m_udwNum; ++udwIdx)
    {
        if (ptbPlayer->m_bChest_lottery.m_astList[udwIdx].ddwChestId == udwChestId)
        {
            dwPos = udwIdx;
            break;
        }
    }
    if (dwPos == -1 && ptbPlayer->m_bChest_lottery.m_udwNum == TBPLAYER_CHEST_LOTTERY_MAX_NUM)
    {
        return -2;
    }


    if (dwPos == -1)
    {
        dwPos = ptbPlayer->m_bChest_lottery.m_udwNum;
        TUINT32 udwSeq = rand() % udwSeqNum;
        if (poChestLottery->m_oJsonRoot["chest_lottery"][szChestId][udwSeq].size() == 0)
        {
            return -3;
        }

        ptbPlayer->m_bChest_lottery.m_astList[dwPos].ddwChestId = udwChestId;
        ptbPlayer->m_bChest_lottery.m_astList[dwPos].ddwSeq = udwSeq;
        ptbPlayer->m_bChest_lottery.m_astList[dwPos].ddwUsed = 1;
        ++ptbPlayer->m_bChest_lottery.m_udwNum;
        ptbPlayer->SetFlag(TbPLAYER_FIELD_CHEST_LOTTERY);
        TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("GetChestLottery: ins: list seq=%d, used = %d", udwSeq, 1));

        pstGlobalRes->Reset();
        const Json::Value &jsonReward = poChestLottery->m_oJsonRoot["chest_lottery"][szChestId][udwSeq][0U];
        pstGlobalRes->aRewardList[0].udwType = jsonReward[0U].asInt();
        pstGlobalRes->aRewardList[0].udwId = jsonReward[1U].asInt();
        pstGlobalRes->aRewardList[0].udwNum = jsonReward[2U].asInt();
        pstGlobalRes->udwTotalNum = 1;
    }
    else
    {
        TUINT32 udwSeq = ptbPlayer->m_bChest_lottery.m_astList[dwPos].ddwSeq;
        TUINT32 udwUsed = ptbPlayer->m_bChest_lottery.m_astList[dwPos].ddwUsed;
        if (poChestLottery->m_oJsonRoot["chest_lottery"][szChestId].size() > udwSeq &&
            poChestLottery->m_oJsonRoot["chest_lottery"][szChestId][udwSeq].size() > udwUsed)
        {
            ++ptbPlayer->m_bChest_lottery.m_astList[dwPos].ddwUsed;
            ptbPlayer->SetFlag(TbPLAYER_FIELD_CHEST_LOTTERY);
            TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("GetChestLottery: cnt: list seq=%d, used = %d", udwSeq, udwUsed + 1));

            pstGlobalRes->Reset();
            const Json::Value &jsonReward = poChestLottery->m_oJsonRoot["chest_lottery"][szChestId][udwSeq][udwUsed];
            pstGlobalRes->aRewardList[0].udwType = jsonReward[0U].asInt();
            pstGlobalRes->aRewardList[0].udwId = jsonReward[1U].asInt();
            pstGlobalRes->aRewardList[0].udwNum = jsonReward[2U].asInt();
            pstGlobalRes->udwTotalNum = 1;
        }
        else
        {
            TUINT32 udwSeq = rand() % udwSeqNum;
            if (poChestLottery->m_oJsonRoot["chest_lottery"][szChestId][udwSeq].size() == 0)
            {
                return -4;
            }

            ptbPlayer->m_bChest_lottery.m_astList[dwPos].ddwSeq = udwSeq;
            ptbPlayer->m_bChest_lottery.m_astList[dwPos].ddwUsed = 1;
            ptbPlayer->SetFlag(TbPLAYER_FIELD_CHEST_LOTTERY);
            TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("GetChestLottery: upt: list seq=%d, used = %d", udwSeq, 1));

            pstGlobalRes->Reset();
            const Json::Value &jsonReward = poChestLottery->m_oJsonRoot["chest_lottery"][szChestId][udwSeq][0U];
            pstGlobalRes->aRewardList[0].udwType = jsonReward[0U].asInt();
            pstGlobalRes->aRewardList[0].udwId = jsonReward[1U].asInt();
            pstGlobalRes->aRewardList[0].udwNum = jsonReward[2U].asInt();
            pstGlobalRes->udwTotalNum = 1;
        }
    }
    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("GetChestLottery: get: [%u,%u,%u]",
        pstGlobalRes->aRewardList[0].udwType, pstGlobalRes->aRewardList[0].udwId, pstGlobalRes->aRewardList[0].udwNum));

    return 0;
}

TINT32 CItemLogic::AppendGlobalRes(SSpGlobalRes *pstGlobalRes, SSpGlobalRes *pstGlobalResOne)
{
    for (TUINT32 udwIdx = 0; udwIdx < pstGlobalResOne->udwTotalNum && pstGlobalRes->udwTotalNum + udwIdx < MAX_SP_REWARD_ITEM_NUM; ++udwIdx)
    {
        TUINT32 udwNum = pstGlobalRes->udwTotalNum;
        TBOOL bExist = FALSE;
        for (TUINT32 udwIdy = 0; udwIdy < udwNum; ++udwIdy)
        {
            if (pstGlobalRes->aRewardList[udwIdy].udwType == pstGlobalResOne->aRewardList[udwIdx].udwType &&
                pstGlobalRes->aRewardList[udwIdy].udwId == pstGlobalResOne->aRewardList[udwIdx].udwId)
            {
                bExist = TRUE;
                pstGlobalRes->aRewardList[udwIdy].udwNum += pstGlobalResOne->aRewardList[udwIdx].udwNum;
                break;
            }
        }
        if (!bExist)
        {
            pstGlobalRes->aRewardList[udwNum].udwType = pstGlobalResOne->aRewardList[udwIdx].udwType;
            pstGlobalRes->aRewardList[udwNum].udwId = pstGlobalResOne->aRewardList[udwIdx].udwId;
            pstGlobalRes->aRewardList[udwNum].udwNum = pstGlobalResOne->aRewardList[udwIdx].udwNum;
            ++pstGlobalRes->udwTotalNum;
        }
    }

    return 0;
}

TINT32 CItemLogic::ItemUseHanderAfter(SUserInfo * pstUserInfo, SCityInfo *pstCity, SBuffDetail* pstItemEffect, SSpGlobalRes *pstEffectInfo, TUINT32 udwItemId, TINT64 ddwTargetId)
{
    //1 peacetime
    if(pstItemEffect->m_udwId == EN_BUFFER_INFO_PEACE_TIME)
    {
        //同步player信息
        pstUserInfo->m_tbPlayer.Set_Status(pstUserInfo->m_tbPlayer.Get_Status() | EN_CITY_STATUS__AVOID_WAR);

    }

    TBOOL bKnightExpItem = FALSE;
    TBOOL bHeroExpItem = FALSE;
    TBOOL bUserSpeedUp = FALSE;
    TBOOL bUseResource = FALSE;
    TINT64 ddwTotalKnightExp = 0;
    for(TUINT32 udwIdx = 0; udwIdx < pstEffectInfo->udwTotalNum; ++udwIdx)
    {
        if(pstEffectInfo->aRewardList[udwIdx].udwType == EN_GLOBALRES_TYPE_KNIGHT_EXP)
        {
            bKnightExpItem = TRUE;
            ddwTotalKnightExp += pstEffectInfo->aRewardList[udwIdx].udwNum;
        }

        if(pstEffectInfo->aRewardList[udwIdx].udwType == EN_GLOBALRES_TYPE_LORD_EXP)
        {
            bHeroExpItem = TRUE;
        }
        if(pstEffectInfo->aRewardList[udwIdx].udwType == EN_GLOBALRES_TYPE_SPEEDUP_TIME ||
            pstEffectInfo->aRewardList[udwIdx].udwType == EN_GLOBALRES_TYPE_SPEEDUP_PERCENT)
        {
            bUserSpeedUp = TRUE;
        }
        if(EN_GLOBALRES_TYPE_RESOURCE == pstEffectInfo->aRewardList[udwIdx].udwType)
        {
            bUseResource = TRUE;
        }
    }

    if (bKnightExpItem)
    {
        TUINT32 udwBLv = CGameInfo::GetInstance()->ComputeKnightLevelByExp(pstCity->m_stTblData.m_bKnight[ddwTargetId].ddwExp - ddwTotalKnightExp);
        TUINT32 udwELv = CGameInfo::GetInstance()->ComputeKnightLevelByExp(pstCity->m_stTblData.m_bKnight[ddwTargetId].ddwExp);
        if (udwELv > udwBLv)
        {
            CSendMessageBase::AddTips(pstUserInfo, EN_TIPS_TYPE__KNIGHT_LV_UP, pstUserInfo->m_tbPlayer.m_nUid, FALSE, ddwTargetId);
        }
    }

    if(bHeroExpItem)
    {
        //活动积分统计
        CActivitesLogic::ComputeHeroExpItemScore(pstUserInfo, udwItemId);
    }

    if(bUserSpeedUp)
    {
        //活动积分统计
        CActivitesLogic::ComputeSpeedUpItemScore(pstUserInfo, udwItemId);
        //task count
        CQuestLogic::SetTaskCurValue(pstUserInfo,pstCity, EN_TASK_TYPE_USE_SPEED_UP_ITEMS);
    }
    
    if(bUseResource)
    {
        //task count
        CQuestLogic::SetTaskCurValue(pstUserInfo, pstCity, EN_TASK_TYPE_USE_RESOURCE_ITEMS);
    }

    //task count
    CQuestLogic::SetTaskCurValue(pstUserInfo, pstCity, EN_TASK_TYPE_USE_ITEMS, 1, udwItemId);

    if(!CItemBase::IsChest(udwItemId)
        || udwItemId == EN_ITEM_ID__DRAGONS_STOMP)
    {
        CSendMessageBase::AddCommonTips(pstUserInfo, pstUserInfo->m_tbPlayer.m_nUid, FALSE, pstItemEffect, pstEffectInfo, udwItemId, ddwTargetId);
    }
    return 0;
}

TINT32 CItemLogic::IsCanMoveCrystal(SUserInfo *pstUser, TUINT32 udwItemId, TUINT32 udwCrystalLv)
{
    TBOOL bCanUse = FALSE;
    if(udwItemId == 218 && udwCrystalLv <= 3)
    {
        return TRUE;
    }
    if(udwItemId == 219 && udwCrystalLv <= 4)
    {
        return TRUE;
    }
    if(udwItemId == 220 && udwCrystalLv <= 5)
    {
        return TRUE;
    }
    if(udwItemId == 221 && udwCrystalLv <= 6)
    {
        return TRUE;
    }
    return bCanUse;
}

TINT32 CItemLogic::CanMoveCity(SUserInfo *pstUser)
{
    TINT32 dwRetCode = 0;

    TUINT32 udwMarchNum = 0;
    TBOOL bIsRallyWar = FALSE;
    TINT64 ddwRallyWarId = 0;
    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
    {
        if (!CActionBase::IsPlayerOwnedAction(pstUser->m_tbPlayer.m_nUid, pstUser->m_atbMarch[udwIdx].m_nId))
        {
            continue;
        }
        if (pstUser->m_atbMarch[udwIdx].m_nMclass == EN_ACTION_MAIN_CLASS__TIMER)
        {
            continue;
        }
        if (pstUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR
            || pstUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR_THRONE)
        {
            ddwRallyWarId = pstUser->m_atbMarch[udwIdx].m_nId;
            if (pstUser->m_atbMarch[udwIdx].m_nStatus == EN_MARCH_STATUS__MARCHING
                || pstUser->m_atbMarch[udwIdx].m_nStatus == EN_MARCH_STATUS__FIGHTING)
            {
                bIsRallyWar = TRUE;
                break;
            }
        }

        if (pstUser->m_atbMarch[udwIdx].m_nStatus == EN_MARCH_STATUS__MARCHING ||
            pstUser->m_atbMarch[udwIdx].m_nStatus == EN_MARCH_STATUS__FIGHTING ||
            pstUser->m_atbMarch[udwIdx].m_nStatus == EN_MARCH_STATUS__RETURNING)
        {
            udwMarchNum++;
        }
    }
    if (udwMarchNum != 0 || bIsRallyWar)
    {
        dwRetCode = EN_RET_CODE__CANNOT_MOVE_CITY_BY_MARCH_ACTION;
        if (bIsRallyWar)
        {
            dwRetCode = EN_RET_CODE__CANNOT_MOVE_CITY_ON_RALLY_WAR;
        }
    }

    if (dwRetCode == 0 && pstUser->m_tbPlayer.m_nDragon_status == EN_DRAGON_STATUS_MARCH && pstUser->m_tbPlayer.m_nDragon_tid != ddwRallyWarId)
    {
        dwRetCode = EN_RET_CODE__CANNOT_MOVE_CITY_WHEN_DRAGON_NOT_IN_CITY;
    }

    return dwRetCode;
}

TINT32 CItemLogic::HasTrialItemOrUnlock(SUserInfo *pstUser)
{
    TbBackpack *ptbBackPack = &pstUser->m_tbBackpack;
    for (TUINT32 udwIdx = 0; udwIdx < ptbBackPack->m_bItem.m_udwNum; ++udwIdx)
    {
        if (ptbBackPack->m_bItem[udwIdx].m_ddwItemId == 835 &&
            ptbBackPack->m_bItem[udwIdx].m_ddwItemNum > 0)
        {
            return 1;
        }
    }

    if (pstUser->m_tbPlayer.m_nTrial_rage_open == 1)
    {
        return 1;
    }

    return 0;
}

TINT32 CItemLogic::SyncPassMarchAction(TbMarch_action *ptbAction, SUserInfo *pstUser)
{
    for (TUINT32 udwIdy = 0; udwIdy < pstUser->m_udwPassiveMarchNum; ++udwIdy)
    {
        TbMarch_action *ptbPAction = &pstUser->m_atbPassiveMarch[udwIdy];
        if (ptbAction->m_nId == ptbPAction->m_nId)
        {
            ptbPAction->m_nTpos = ptbAction->m_nTpos;
            ptbPAction->m_nTuid = ptbAction->m_nTuid;
            ptbPAction->m_nEtime = ptbAction->m_nEtime;
        }
    }

    return 0;
}
TINT32 CItemLogic::CanUnlockVipStage(SUserInfo *pstUser)
{
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;
    TINT64 ddwVipPoint = ptbPlayer->m_nVip_point;
    TINT32 dwRetCode = 0;

    TINT64 ddwMaxPoint = CPlayerBase::GetMaxVipPoint(ptbPlayer);
    if (ddwVipPoint < ddwMaxPoint || ptbPlayer->m_nVip_stage >= CPlayerBase::GetMaxVipStage())
    {
        dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return dwRetCode;
    }
    return dwRetCode;
}