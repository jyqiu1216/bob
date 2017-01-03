#include "globalres_logic.h"
#include "player_info.h"
#include "game_define.h"
#include "item_base.h"
#include "player_base.h"
#include "game_info.h"
#include "city_base.h"
#include "common_base.h"
#include "item_logic.h"
#include "common_logic.h"
#include "service_key.h"
#include "action_base.h"
#include "backpack_logic.h"
#include "common_func.h"
#include "sendmessage_base.h"

TINT32 CGlobalResLogic::AddGlobalRes(SUserInfo *pstUser, SCityInfo* pstCity, SGlobalRes *pstGlobalRes, TUINT32 udwSecClass /*= 0*/,TINT64 ddwTargetId /* =-1 */, TUINT32 udwSpItemId)
{
    if(NULL == pstGlobalRes || NULL == pstCity)
    {
        return -1;
    }

    TINT32 dwRetCode = 0;
    for(TUINT32 udwIdx = 0; udwIdx < pstGlobalRes->ddwTotalNum; ++udwIdx)
    {
        TSE_LOG_INFO(CGameInfo::GetInstance()->m_poLog, ("CGlobalResLogic:AddGlobalRes[uid=%u cidx=%d cid=%d type=%d id=%d num=%d idx=%d]",
            pstUser->m_tbPlayer.m_nUid,
            0,
            pstCity->m_stTblData.m_nPos,
            pstGlobalRes->aRewardList[udwIdx].ddwType,
            pstGlobalRes->aRewardList[udwIdx].ddwId,
            pstGlobalRes->aRewardList[udwIdx].ddwNum,
            udwIdx));


        dwRetCode = CGlobalResLogic::AddGlobalRes(pstUser, pstCity, 
            pstGlobalRes->aRewardList[udwIdx].ddwType, 
            pstGlobalRes->aRewardList[udwIdx].ddwId,
            pstGlobalRes->aRewardList[udwIdx].ddwNum, 
            udwSecClass,
            ddwTargetId, 
            udwSpItemId);
        if(dwRetCode != 0)
        {
            return dwRetCode;
        }
    }

    return 0;
}


TINT32 CGlobalResLogic::AddSpGlobalRes(SUserInfo *pstUser, SCityInfo* pstCity, SSpGlobalRes *pstGlobalRes, TUINT32 udwSecClass/* = 0*/,TINT64 ddwTargetRes /* =-1 */, TUINT32 udwSpItemId)
{
    if(NULL == pstGlobalRes || NULL == pstCity)
    {
        return -1;
    }

    if(pstGlobalRes->udwTotalNum == 0)
    {
        return 0;
    }
    TINT32 dwRetCode = 0;
    for(TUINT32 udwIdx = 0; udwIdx < pstGlobalRes->udwTotalNum; ++udwIdx)
    {
        TSE_LOG_INFO(CGameInfo::GetInstance()->m_poLog, ("CGlobalResLogic:AddGlobalRes[uid=%u cidx=%d cid=%d type=%d id=%d num=%d idx=%d]",
            pstUser->m_tbPlayer.m_nUid,
            0,
            pstCity->m_stTblData.m_nPos,
            pstGlobalRes->aRewardList[udwIdx].udwType,
            pstGlobalRes->aRewardList[udwIdx].udwId,
            pstGlobalRes->aRewardList[udwIdx].udwNum,
            udwIdx));

        dwRetCode = CGlobalResLogic::AddGlobalRes(pstUser, pstCity, pstGlobalRes->aRewardList[udwIdx].udwType, pstGlobalRes->aRewardList[udwIdx].udwId, pstGlobalRes->aRewardList[udwIdx].udwNum,udwSecClass,ddwTargetRes, udwSpItemId, &pstGlobalRes->aRewardList[udwIdx].stAttrInfo);
        if(dwRetCode != 0)
        {
            return dwRetCode;
        }
    }

    return 0;
}

TINT32 CGlobalResLogic::CostGlobalRes(SUserInfo *pstUser, SCityInfo* pstCity, SGlobalRes *pstGlobalRes)
{
    if(NULL == pstGlobalRes || NULL == pstCity)
    {
        return -1;
    }

    TINT32 dwRetCode = 0;
    for(TUINT32 udwIdx = 0; udwIdx < pstGlobalRes->ddwTotalNum; ++udwIdx)
    {
        TSE_LOG_INFO(CGameInfo::GetInstance()->m_poLog, ("CGlobalResLogic:CostGlobalRes[uid=%u cidx=%d cid=%d type=%d id=%d num=%d idx=%d]",
            pstUser->m_tbPlayer.m_nUid,
            0,
            pstCity->m_stTblData.m_nPos,
            pstGlobalRes->aRewardList[udwIdx].ddwType,
            pstGlobalRes->aRewardList[udwIdx].ddwId,
            pstGlobalRes->aRewardList[udwIdx].ddwNum,
            udwIdx));

        dwRetCode = CGlobalResLogic::CostGlobalRes(pstUser, pstCity, pstGlobalRes->aRewardList[udwIdx].ddwType, pstGlobalRes->aRewardList[udwIdx].ddwId, pstGlobalRes->aRewardList[udwIdx].ddwNum);
        if(dwRetCode != 0)
        {
            return dwRetCode;
        }
    }

    return 0;
}

TBOOL CGlobalResLogic::HaveEnoughGlobalsRes(SUserInfo *pstUser, SCityInfo* pstCity, SGlobalRes *pstGlobalRes)
{
    if(NULL == pstGlobalRes || NULL == pstCity || pstUser == NULL)
    {
        return FALSE;
    }

    TINT32 dwRetCode = 0;
    for(TUINT32 udwIdx = 0; udwIdx < pstGlobalRes->ddwTotalNum; ++udwIdx)
    {
        dwRetCode = CGlobalResLogic::HaveEnoughGlobalsRes(pstUser, pstCity, pstGlobalRes->aRewardList[udwIdx].ddwType, pstGlobalRes->aRewardList[udwIdx].ddwId, pstGlobalRes->aRewardList[udwIdx].ddwNum);
        if(dwRetCode == FALSE)
        {
            TSE_LOG_INFO(CGameInfo::GetInstance()->m_poLog, ("CGlobalResLogic:NotHaveEnoughGlobalsRes[uid=%u cidx=%d cid=%d type=%d id=%d num=%d idx=%d]",
                pstUser->m_tbPlayer.m_nUid,
                0,
                pstCity->m_stTblData.m_nPos,
                pstGlobalRes->aRewardList[udwIdx].ddwType,
                pstGlobalRes->aRewardList[udwIdx].ddwId,
                pstGlobalRes->aRewardList[udwIdx].ddwNum,
                udwIdx));

            return FALSE;
        }
        TSE_LOG_INFO(CGameInfo::GetInstance()->m_poLog, ("CGlobalResLogic:HaveEnoughGlobalsRes[uid=%u cidx=%d cid=%d type=%d id=%d num=%d idx=%d]",
            pstUser->m_tbPlayer.m_nUid,
            0,
            pstCity->m_stTblData.m_nPos,
            pstGlobalRes->aRewardList[udwIdx].ddwType,
            pstGlobalRes->aRewardList[udwIdx].ddwId,
            pstGlobalRes->aRewardList[udwIdx].ddwNum,
            udwIdx));
    }

    return TRUE;
}


TINT32 CGlobalResLogic::CostSpGlobalRes(SUserInfo *pstUser, SCityInfo* pstCity, SSpGlobalRes *pstGlobalRes)
{
    if(NULL == pstGlobalRes || NULL == pstCity)
    {
        return -1;
    }

    TINT32 dwRetCode = 0;
    for(TUINT32 udwIdx = 0; udwIdx < pstGlobalRes->udwTotalNum; ++udwIdx)
    {
        dwRetCode = CGlobalResLogic::CostGlobalRes(pstUser, pstCity, pstGlobalRes->aRewardList[udwIdx].udwType, pstGlobalRes->aRewardList[udwIdx].udwId, pstGlobalRes->aRewardList[udwIdx].udwNum);
        if(dwRetCode != 0)
        {
            return dwRetCode;
        }
    }

    return 0;
}

TBOOL CGlobalResLogic::HaveEnoughSpGlobalsRes(SUserInfo *pstUser, SCityInfo* pstCity, SSpGlobalRes *pstGlobalRes)
{
    if(NULL == pstGlobalRes || NULL == pstCity || pstUser == NULL)
    {
        return FALSE;
    }

    TINT32 dwRetCode = 0;
    for(TUINT32 udwIdx = 0; udwIdx < pstGlobalRes->udwTotalNum; ++udwIdx)
    {
        dwRetCode = CGlobalResLogic::HaveEnoughGlobalsRes(pstUser, pstCity, pstGlobalRes->aRewardList[udwIdx].udwType, pstGlobalRes->aRewardList[udwIdx].udwId, pstGlobalRes->aRewardList[udwIdx].udwNum);
        if(dwRetCode == FALSE)
        {
            TSE_LOG_INFO(CGameInfo::GetInstance()->m_poLog, ("CGlobalResLogic:NotHaveEnoughGlobalsRes[uid=%u cidx=%d cid=%d type=%d id=%d num=%d idx=%d]",
                pstUser->m_tbPlayer.m_nUid,
                0,
                pstCity->m_stTblData.m_nPos,
                pstGlobalRes->aRewardList[udwIdx].udwType,
                pstGlobalRes->aRewardList[udwIdx].udwId,
                pstGlobalRes->aRewardList[udwIdx].udwNum,
                udwIdx));
            return FALSE;
        }
        TSE_LOG_INFO(CGameInfo::GetInstance()->m_poLog, ("CGlobalResLogic:HaveEnoughGlobalsRes[uid=%u cidx=%d cid=%d type=%d id=%d num=%d idx=%d]",
            pstUser->m_tbPlayer.m_nUid,
            0,
            pstCity->m_stTblData.m_nPos,
            pstGlobalRes->aRewardList[udwIdx].udwType,
            pstGlobalRes->aRewardList[udwIdx].udwId,
            pstGlobalRes->aRewardList[udwIdx].udwNum,
            udwIdx));
    }

    return TRUE;
}

TINT32 CGlobalResLogic::AddGlobalRes(SUserInfo *pstUser, SCityInfo* pstCity, TUINT32 udwType, TUINT32 udwItemId, TUINT32 udwItemNum,TUINT32 udwSecClas,
    TINT64 ddwTargetId /* =-1 */, TUINT32 udwSpItemId, SAttrInfo *pstAttrInfo)
{
    string sReplace = pstUser->m_tbPlayer.m_sUin + "#" + CCommonFunc::NumToString(udwItemNum);
    string sParam = CCommonFunc::NumToString(pstUser->m_tbPlayer.m_nUid);

    TINT32 dwRetcode = 0;
    TINT32 dwComputeLevel = 0;
    TUINT32 udwIdx = 0;
    switch(udwType)
    {
    case EN_GLOBALRES_TYPE_ITEM:
        dwRetcode = CItemBase::AddItem(&pstUser->m_tbBackpack, udwItemId, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_GEM:
        CPlayerBase::AddGem(&pstUser->m_tbLogin, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_RESOURCE:
        CCityBase::AddResource(pstCity, udwItemId, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_TROOP:
        CCityBase::AddTroop(pstCity, udwItemId, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_LORD_EXP:
        CPlayerBase::AddLordExp(pstUser, pstCity, udwItemNum, udwSpItemId == 0);
        break;
    case EN_GLOBALRES_TYPE_LORD_LV:
        dwComputeLevel = CPlayerBase::ComputePlayerLevel(&pstUser->m_tbPlayer);
        if(static_cast<TINT32>(udwItemNum) <= dwComputeLevel)
        {
            pstUser->m_tbPlayer.Set_Level(udwItemNum);
        }
        else
        {
            TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("CGlobalResLogic::LEVEL_UP [uid=%ld, level=%u, compute_level=%d, exp=%ld]",
                pstUser->m_tbPlayer.m_nUid, udwItemNum, dwComputeLevel, pstUser->m_tbPlayer.m_nExp));
            return -1;
        }
        break;
    case EN_GLOBALRES_TYPE_AL_GIFT:
        for(udwIdx = 0; udwIdx < udwItemNum; udwIdx++)
        {
            CCommonLogic::GenAlGiftToAlliance(pstUser->m_tbAlliance.m_nAid, udwItemId, pstUser->m_tbPlayer.m_nUid, pstUser->m_stAlGifts, &pstUser->m_tbLogin);
        }
        break;
    case EN_GLOBALRES_TYPE_VIP_POINT:
        dwRetcode = CCommonBase::AddVipPoint(pstUser, pstCity, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_VIP_TIME:
        dwRetcode = CCommonBase::AddVipTime(pstUser, pstCity, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_FORT:
        CCityBase::AddFort(pstCity, udwItemId, udwItemNum);
        break;
    case  EN_GLOBALRES_TYPE_KNIGHT_EXP:
        dwRetcode = CCityBase::AddKnightExp(pstCity, ddwTargetId, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_SPEEDUP_TIME:
        dwRetcode = CItemLogic::SpeedUpActionByNum(ddwTargetId, udwSecClas,udwItemNum, pstUser);
        break;
    case EN_GLOBALRES_TYPE_SPEEDUP_PERCENT:
        dwRetcode = CItemLogic::SpeedUpActionByPercent(ddwTargetId, udwSecClas, udwItemNum, pstUser);
        break;
    case EN_GLOBALRES_TYPE_EQUIP_MATERIAL:
        CBackpack::AddMaterial(pstUser, udwItemId, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_EQUIP_CRYSTAL:
        CBackpack::AddCrystal(pstUser, udwItemId, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_LOYLTY:
        dwRetcode = CPlayerBase::AddLoyality(&pstUser->m_tbPlayer, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_FUND:
        dwRetcode = CCommonBase::AddAllianceFund(&pstUser->m_tbPlayer, &pstUser->m_tbAlliance, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_EQUIP_SOUL:
        CBackpack::AddSoul(pstUser, udwItemId, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_EQUIP_PARTS:
        CBackpack::AddParts(pstUser, udwItemId, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_DRAGON_ENERGY:
        CPlayerBase::AddDragonEnergy(pstUser, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_SPECIAL_CRYSTAL:
        CBackpack::AddSpCrystal(pstUser, udwItemId, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_DRAGON_LV:
        dwComputeLevel = CPlayerBase::ComputeDragonLevel(pstUser->m_tbPlayer.m_nDragon_exp);
        if (static_cast<TINT32>(udwItemNum) <= dwComputeLevel)
        {
            pstUser->m_tbPlayer.Set_Dragon_level(udwItemNum);
        }
        else
        {
            TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("CGlobalResLogic:: dragon LEVEL_UP [uid=%ld, level=%u, compute_level=%d, exp=%ld]",
                pstUser->m_tbPlayer.m_nUid, udwItemNum, dwComputeLevel, pstUser->m_tbPlayer.m_nDragon_exp));
            return -1;
        }
        break;
    case EN_GLOBALRES_TYPE_DRAGON_EXP:
        CPlayerBase::AddDragonExp(pstUser, udwItemNum, udwSpItemId == 0);
        break;
    case EN_GLOBALRES_TYPE_SCROLL:
        CBackpack::AddScroll(pstUser, udwItemId, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_DRAGON_SHARD:
        CPlayerBase::AddDragonShard(pstUser, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_EQUIP:
        dwRetcode = CBackpack::AddNormalEquip(pstUser, udwItemId / 10, udwItemId % 10, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_LORD_IMAGE:
        dwRetcode = CPlayerBase::AddLordImage(&pstUser->m_tbLordImage, udwItemId);
        break;
    case EN_GLOBALRES_TYPE_DECORATION:
        dwRetcode = CPlayerBase::AddDecoration(&pstUser->m_tbDecoration, udwItemId, udwItemNum);
        break;
    default:
        dwRetcode = -1;
        break;
    }

    return dwRetcode;
}

TINT32 CGlobalResLogic::CostGlobalRes(SUserInfo *pstUser, SCityInfo* pstCity, TUINT32 udwType, TUINT32 udwItemId, TUINT32 udwItemNum)
{
    TINT32 dwRetcode = 0;

    switch(udwType)
    {
    case EN_GLOBALRES_TYPE_ITEM:
        dwRetcode = CItemBase::CostItem(&pstUser->m_tbBackpack, udwItemId, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_GEM:
        CPlayerBase::CostGem(pstUser, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_HONOR:
        break;
    case EN_GLOBALRES_TYPE_RESOURCE:
        CCityBase::CostOneResource(pstCity, udwItemId, udwItemNum);
        pstUser->m_udwCostResource += udwItemNum;
        break;
    case EN_GLOBALRES_TYPE_TROOP:
        CCityBase::CostTroop(pstCity, udwItemId, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_LORD_EXP:
        break;
    case EN_GLOBALRES_TYPE_BUILDING_LV:
        break;
    case EN_GLOBALRES_TYPE_RESEARC_LV:
        break;
    case EN_GLOBALRES_TYPE_LORD_LV:
        break;
    case EN_GLOBALRES_TYPE_AL_GIFT:
        break;
    case EN_GLOBALRES_TYPE_VIP_POINT:
        break;
    case EN_GLOBALRES_TYPE_RATE:
        break;
    case EN_GLOBALRES_TYPE_HAPPYNESS:
        break;
    case EN_GLOBALRES_TYPE_VIP_LV:
        break;
    case EN_GLOBALRES_TYPE_VIP_TIME:
        break;
    case EN_GLOBALRES_TYPE_FORT:
        CCityBase::CostFort(pstCity, udwItemId, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_EQUIP_MATERIAL:
        CBackpack::CostMaterial(pstUser, udwItemId, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_EQUIP_CRYSTAL:
        CBackpack::CostCrystal(pstUser, udwItemId, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_LOYLTY:
        dwRetcode = CPlayerBase::CostLoyality(&pstUser->m_tbPlayer, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_FUND:
        dwRetcode = CCommonBase::CostAllianceFund(&pstUser->m_tbPlayer, &pstUser->m_tbAlliance, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_EQUIP_SOUL:
        CBackpack::CostSoul(pstUser, udwItemId, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_EQUIP_PARTS:
        CBackpack::CostParts(pstUser, udwItemId, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_SPECIAL_CRYSTAL:
        CBackpack::CostSpCrystal(pstUser, udwItemId, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_SCROLL:
        CBackpack::CostScroll(pstUser, udwItemId, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_DRAGON_SHARD:
        CPlayerBase::CostDragonShard(pstUser, udwItemNum);
        break;
    default:
        dwRetcode = -1;
        break;
    }

    return dwRetcode;
}

TINT32 CGlobalResLogic::HaveEnoughGlobalsRes(SUserInfo *pstUser, SCityInfo* pstCity, TUINT32 udwType, TUINT32 udwItemId, TUINT32 udwItemNum)
{
    TBOOL bHaveEnough = FALSE;

    switch(udwType)
    {
    case EN_GLOBALRES_TYPE_ITEM:
        bHaveEnough = CItemBase::HasEnoughItem(&pstUser->m_tbBackpack, udwItemId, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_GEM:
        bHaveEnough = CPlayerBase::HasEnoughGem(&pstUser->m_tbLogin, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_HONOR:
        break;
    case EN_GLOBALRES_TYPE_RESOURCE:
        bHaveEnough = CCityBase::HasEnoughResource(pstCity, udwItemId, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_TROOP:
        bHaveEnough = CCityBase::HasEnoughTroop(pstCity, 0, udwItemId, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_LORD_EXP:
        break;
    case EN_GLOBALRES_TYPE_BUILDING_LV:
        if(udwItemNum <= CCityBase::GetBuildingLevelById(&pstCity->m_stTblData, udwItemId))
        {
            bHaveEnough = TRUE;
        }
        break;
    case EN_GLOBALRES_TYPE_RESEARC_LV:
        if (udwItemNum <= pstCity->m_stTblData.m_bResearch[0].m_addwLevel[udwItemId])
        {
            bHaveEnough = TRUE;
        }
        break;
    case EN_GLOBALRES_TYPE_LORD_LV:
        if(udwItemNum <= pstUser->m_tbPlayer.m_nLevel)
        {
            bHaveEnough = TRUE;
        }
        break;
    case EN_GLOBALRES_TYPE_AL_GIFT: //这个暂时不用，以后出现小礼包合成大礼包的类似逻辑时再加
        break;
    case EN_GLOBALRES_TYPE_VIP_POINT:
        break;
    case EN_GLOBALRES_TYPE_RATE:
        break;
    case EN_GLOBALRES_TYPE_HAPPYNESS:
        break;
    case EN_GLOBALRES_TYPE_VIP_LV:
        if(static_cast<TINT32>(udwItemNum) <= CPlayerBase::GetVipLevel(&pstUser->m_tbPlayer))
        {
            bHaveEnough = TRUE;
        }
        break;
    case EN_GLOBALRES_TYPE_VIP_TIME:
        break;
    case EN_GLOBALRES_TYPE_FORT:
        bHaveEnough = CCityBase::HasEnoughTroop(pstCity, 1, udwItemId, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_EQUIP_MATERIAL:
        bHaveEnough = CBackpack::HasEnoughMaterial(pstUser, udwItemId, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_EQUIP_CRYSTAL:
        bHaveEnough = CBackpack::HasEnoughCrystal(pstUser, udwItemId, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_LOYLTY:
        bHaveEnough = CPlayerBase::HasEnoughLoyality(&pstUser->m_tbPlayer, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_FUND:
        bHaveEnough = CCommonBase::HasEnoughAllianceFund(&pstUser->m_tbPlayer, &pstUser->m_tbAlliance, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_BUILD_LEVEL_FUNC:
        if(udwItemNum <= CCityBase::GetBuildingLevelByFuncType(pstCity, udwItemId))
        {
            bHaveEnough = TRUE;
        }
        break;
    case EN_GLOBALRES_TYPE_EQUIP_SOUL:
        bHaveEnough = CBackpack::HasEnoughSoul(pstUser, udwItemId, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_EQUIP_PARTS:
        bHaveEnough = CBackpack::HasEnoughParts(pstUser, udwItemId, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_SPECIAL_CRYSTAL:
        bHaveEnough = CBackpack::HasEnoughSpCrystal(pstUser, udwItemId, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_SCROLL:
        bHaveEnough = CBackpack::HasEnoughScroll(pstUser, udwItemId, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_DRAGON_SHARD:
        bHaveEnough = CPlayerBase::HasEnoughDragonShard(pstUser, udwItemNum);
        break;
    default:
        bHaveEnough = FALSE;
        break;
    }

    return bHaveEnough;
}

TINT32 CGlobalResLogic::GetSpGlobalResInfo(const Json::Value& oJsonRoot, TINT32 dwRewardType, SSpGlobalRes *pstGlobalRes)
{
    TINT32 dwRewardItemSize = oJsonRoot.size();
    TINT32 dwTotalRate = 0;
    TINT32 dwItemRate = 0;
    TINT32 udwRand = 0;
    TINT32 dwRewardIdx = 0;
    TBOOL bExist = FALSE;
    TUINT32 udwType = 0;
    TUINT32 udwId = 0;
    TUINT32 udwNum = 0;

    switch(dwRewardType)
    {
        //全部获取
    case EREWARD_TYPE_ALL:
        for(TINT32 dwIdx = 0; dwIdx < dwRewardItemSize; ++dwIdx)
        {
            udwType = oJsonRoot[dwIdx][0U].asUInt();
            udwId = oJsonRoot[dwIdx][1U].asUInt();
            udwNum = oJsonRoot[dwIdx][2U].asUInt();

            bExist = FALSE;
            for(TUINT32 udwExistIdx = 0; udwExistIdx < pstGlobalRes->udwTotalNum;++udwExistIdx)
            {
                if(pstGlobalRes->aRewardList[udwExistIdx].udwType == udwType && 
                    pstGlobalRes->aRewardList[udwExistIdx].udwId == udwId)
                {
                    pstGlobalRes->aRewardList[udwExistIdx].udwNum += udwNum;
                    bExist = TRUE;
                    break;
                }
            }
            if(!bExist)
            {
                pstGlobalRes->aRewardList[pstGlobalRes->udwTotalNum].udwType = udwType;
                pstGlobalRes->aRewardList[pstGlobalRes->udwTotalNum].udwId = udwId;
                pstGlobalRes->aRewardList[pstGlobalRes->udwTotalNum].udwNum = udwNum;
                pstGlobalRes->udwTotalNum++;
            }
            TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("CGlobalResLogic::GetGlobalResInfo[type=%d id=%d num=%d ItemNum=%d ", udwType,udwId,udwNum,pstGlobalRes->udwTotalNum));
        }
        break;
        //获取其中一部分
    case EREWARD_TYPE_SOME:
        dwItemRate = 0;
        for(TINT32 dwIdx = 0; dwIdx < dwRewardItemSize; ++dwIdx)
        {
            TINT32 udwRand = rand() % 10000;
            dwItemRate = oJsonRoot[dwIdx][3U].asUInt();
            if(udwRand < dwItemRate)
            {
                udwType = oJsonRoot[dwIdx][0U].asUInt();
                udwId = oJsonRoot[dwIdx][1U].asUInt();
                udwNum = oJsonRoot[dwIdx][2U].asUInt();

                bExist = FALSE;
                for(TUINT32 udwExistIdx = 0; udwExistIdx < pstGlobalRes->udwTotalNum; ++udwExistIdx)
                {
                    if(pstGlobalRes->aRewardList[udwExistIdx].udwType == udwType &&
                        pstGlobalRes->aRewardList[udwExistIdx].udwId == udwId)
                    {
                        pstGlobalRes->aRewardList[udwExistIdx].udwNum += udwNum;
                        bExist = TRUE;
                        break;
                    }
                }
                if(!bExist)
                {
                    pstGlobalRes->aRewardList[pstGlobalRes->udwTotalNum].udwType = udwType;
                    pstGlobalRes->aRewardList[pstGlobalRes->udwTotalNum].udwId = udwId;
                    pstGlobalRes->aRewardList[pstGlobalRes->udwTotalNum].udwNum = udwNum;
                    pstGlobalRes->udwTotalNum++;
                }
                TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("CGlobalResLogic::GetGlobalResInfo[type=%d id=%d num=%d ItemNum=%d ", udwType, udwId, udwNum, pstGlobalRes->udwTotalNum));
            }
        }
        break;
        //获取其中一个奖励
    case EREWARD_TYPE_ONE:
        dwTotalRate = 0;
        for(TINT32 dwIdx = 0; dwIdx < dwRewardItemSize; ++dwIdx)
        {
            dwTotalRate += oJsonRoot[dwIdx][3U].asUInt();
        }
        TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("CGlobalResLogic::GetGlobalResInfo [ dwTotalRate=%d dwRewardItemSize=%d]", dwTotalRate, dwRewardItemSize));
        if(dwTotalRate == 0)
        {
            break;
        }
        udwRand = rand() % dwTotalRate;
        TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("CGlobalResLogic::GetGlobalResInfo [udwRand=%d dwTotalRate=%d dwRewardItemSize=%d]", udwRand, dwTotalRate, dwRewardItemSize));

        dwTotalRate = 0;
        for(TINT32 dwIdx = 0; dwIdx < dwRewardItemSize; ++dwIdx)
        {

            dwTotalRate += oJsonRoot[dwIdx][3U].asUInt();
            TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("CGlobalResLogic::GetGlobalResInfo step%d dwTotalRate=%d ", dwIdx, dwTotalRate));
            if(udwRand < dwTotalRate)
            {
                dwRewardIdx = dwIdx;
                break;
            }
        }

        udwType = oJsonRoot[dwRewardIdx][0U].asUInt();
        udwId = oJsonRoot[dwRewardIdx][1U].asUInt();
        udwNum = oJsonRoot[dwRewardIdx][2U].asUInt();

        bExist = FALSE;
        for(TUINT32 udwExistIdx = 0; udwExistIdx < pstGlobalRes->udwTotalNum; ++udwExistIdx)
        {
            if(pstGlobalRes->aRewardList[udwExistIdx].udwType == udwType &&
                pstGlobalRes->aRewardList[udwExistIdx].udwId == udwId)
            {
                pstGlobalRes->aRewardList[udwExistIdx].udwNum += udwNum;
                bExist = TRUE;
                break;
            }
        }
        if(!bExist)
        {
            pstGlobalRes->aRewardList[pstGlobalRes->udwTotalNum].udwType = udwType;
            pstGlobalRes->aRewardList[pstGlobalRes->udwTotalNum].udwId = udwId;
            pstGlobalRes->aRewardList[pstGlobalRes->udwTotalNum].udwNum = udwNum;
            pstGlobalRes->udwTotalNum++;
        }
        TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("CGlobalResLogic::GetGlobalResInfo[type=%d id=%d num=%d ItemNum=%d ", udwType, udwId, udwNum, pstGlobalRes->udwTotalNum));
        break;
    default:
        return -1;

    }
    return 0;
}


TINT32 CGlobalResLogic::GetGlobalResInfo(const Json::Value& oJsonRoot, TINT32 dwRewardType, SGlobalRes *pstGlobalRes)
{
    TINT32 dwRewardItemSize = oJsonRoot.size();
    TINT32 dwTotalRate = 0;
    TINT32 dwItemRate = 0;
    TINT32 udwRand = 0;
    TINT32 dwRewardIdx = 0;

    switch(dwRewardType)
    {
        //全部获取
    case EREWARD_TYPE_ALL:
        for(TINT32 dwIdx = 0; dwIdx < dwRewardItemSize; ++dwIdx)
        {

            pstGlobalRes->aRewardList[pstGlobalRes->ddwTotalNum].ddwType = oJsonRoot[dwIdx][0U].asUInt();
            pstGlobalRes->aRewardList[pstGlobalRes->ddwTotalNum].ddwId = oJsonRoot[dwIdx][1U].asUInt();
            pstGlobalRes->aRewardList[pstGlobalRes->ddwTotalNum].ddwNum = oJsonRoot[dwIdx][2U].asUInt();

            TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("CGlobalResLogic::GetGlobalResInfo[type=%d id=%d num=%d ItemNum=%d]",
                pstGlobalRes->aRewardList[pstGlobalRes->ddwTotalNum].ddwType,
                pstGlobalRes->aRewardList[pstGlobalRes->ddwTotalNum].ddwId,
                pstGlobalRes->aRewardList[pstGlobalRes->ddwTotalNum].ddwNum,
                pstGlobalRes->ddwTotalNum));

            pstGlobalRes->ddwTotalNum++;
        }
        break;
        //获取其中一部分
    case EREWARD_TYPE_SOME:
        dwItemRate = 0;
        for(TINT32 dwIdx = 0; dwIdx < dwRewardItemSize; ++dwIdx)
        {
            TINT32 udwRand = rand() % 10000;
            dwItemRate = oJsonRoot[dwIdx][3U].asUInt();
            if(udwRand < dwItemRate)
            {
                pstGlobalRes->aRewardList[pstGlobalRes->ddwTotalNum].ddwType = oJsonRoot[dwIdx][0U].asUInt();
                pstGlobalRes->aRewardList[pstGlobalRes->ddwTotalNum].ddwId = oJsonRoot[dwIdx][1U].asUInt();
                pstGlobalRes->aRewardList[pstGlobalRes->ddwTotalNum].ddwNum = oJsonRoot[dwIdx][2U].asUInt();

                TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("CGlobalResLogic::GetGlobalResInfo[idx=%u rand=%u rate=%u type=%d id=%d num=%d ItemNum=%d]",
                    dwIdx,
                    udwRand,
                    dwItemRate,
                    pstGlobalRes->aRewardList[pstGlobalRes->ddwTotalNum].ddwType,
                    pstGlobalRes->aRewardList[pstGlobalRes->ddwTotalNum].ddwId,
                    pstGlobalRes->aRewardList[pstGlobalRes->ddwTotalNum].ddwNum,
                    pstGlobalRes->ddwTotalNum));

                pstGlobalRes->ddwTotalNum++;

            }
        }
        break;
        //获取其中一个奖励
    case EREWARD_TYPE_ONE:
        dwTotalRate = 0;
        for(TINT32 dwIdx = 0; dwIdx < dwRewardItemSize; ++dwIdx)
        {
            dwTotalRate += oJsonRoot[dwIdx][3U].asUInt();
        }
        TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("CGlobalResLogic::GetGlobalResInfo [ dwTotalRate=%d dwRewardItemSize=%d]", dwTotalRate, dwRewardItemSize));
        if(dwTotalRate == 0)
        {
            break;
        }
        udwRand = rand() % dwTotalRate;
        TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("CGlobalResLogic::GetGlobalResInfo [udwRand=%d dwTotalRate=%d dwRewardItemSize=%d]", udwRand, dwTotalRate, dwRewardItemSize));

        dwTotalRate = 0;
        for(TINT32 dwIdx = 0; dwIdx < dwRewardItemSize; ++dwIdx)
        {

            dwTotalRate += oJsonRoot[dwIdx][3U].asUInt();
            TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("CGlobalResLogic::GetGlobalResInfo step%d dwTotalRate=%d ", dwIdx, dwTotalRate));
            if(udwRand < dwTotalRate)
            {
                dwRewardIdx = dwIdx;
                break;
            }


        }
        pstGlobalRes->aRewardList[pstGlobalRes->ddwTotalNum].ddwType = oJsonRoot[dwRewardIdx][0U].asUInt();
        pstGlobalRes->aRewardList[pstGlobalRes->ddwTotalNum].ddwId = oJsonRoot[dwRewardIdx][1U].asUInt();
        pstGlobalRes->aRewardList[pstGlobalRes->ddwTotalNum].ddwNum = oJsonRoot[dwRewardIdx][2U].asUInt();


        TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("CGlobalResLogic::GetGlobalResInfo:[type=%d id=%d num=%d ItemNum=%d]",
            pstGlobalRes->aRewardList[pstGlobalRes->ddwTotalNum].ddwType,
            pstGlobalRes->aRewardList[pstGlobalRes->ddwTotalNum].ddwId,
            pstGlobalRes->aRewardList[pstGlobalRes->ddwTotalNum].ddwNum,
            pstGlobalRes->ddwTotalNum));
        pstGlobalRes->ddwTotalNum++;
        break;
    default:
        return -1;

    }
    return 0;
}


TINT32 CGlobalResLogic::GetLotteryChestGlobalResInfo(TINT32 dwChestId, SSpGlobalRes *pstGlobalRes)
{
    if(pstGlobalRes == NULL)
    {
        return -1;
    }
    pstGlobalRes->Reset();

    TINT32 dwRetCode = 0;
    TCHAR szChestId[32];
    sprintf(szChestId, "%u", dwChestId);
    CGameInfo *poGameInfo = CGameInfo::GetInstance();
    if(!poGameInfo->m_oJsonRoot["game_chest"].isMember(szChestId))
    {
        return -6;
    }
    const Json::Value &oChestReward = poGameInfo->m_oJsonRoot["game_chest"][szChestId]["a1"];
    TINT32 dwRewardType = poGameInfo->m_oJsonRoot["game_chest"][szChestId]["a0"].asInt();


    dwRetCode = CGlobalResLogic::GetSpGlobalResInfo(oChestReward, dwRewardType, pstGlobalRes);
    if(dwRetCode != 0)
    {
        return -1;
    }
    return 0;

}

TINT32 CGlobalResLogic::GetTimeQuestReward(TUINT32 udwQuestType, TUINT32 udwQuestLv, TUINT32 udwCastleLv, SGlobalRes *pstGlobalRes)
{
    CGameInfo* pstGameInfo = CGameInfo::GetInstance();
    //quest 的reward是两个节点和平而成的


    pstGlobalRes->Reset();
    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("CGlobalResLogic:get time quest reward [type=%u quest_lv=%u cast_lv=%u]", udwQuestType, udwQuestLv, udwCastleLv));
    TCHAR szQuestType[64];
    snprintf(szQuestType, 64, "%u", udwQuestType);
    const Json::Value &oRewardRaw = pstGameInfo->m_oJsonRoot["game_sub_quest"][szQuestType]["a2"][udwQuestLv - 1]["r"];

    //随机得到奖励idx
    TUINT32 udwCastleRewardIdx = 0;
    TUINT32 udwCastleTotalRate = 0;
    for(TUINT32 udwIdx = 0; udwIdx < oRewardRaw["r0"].size(); ++udwIdx)
    {
        udwCastleTotalRate += oRewardRaw["r0"][udwIdx].asUInt();
    }

    if(udwCastleTotalRate != 0)
    {
        TUINT32 udwRand = rand() % udwCastleTotalRate;
        udwCastleTotalRate = 0;
        for(TUINT32 udwIdx = 0; udwIdx < oRewardRaw["r0"].size(); ++udwIdx)
        {
            udwCastleTotalRate += oRewardRaw["r0"][udwIdx].asUInt();
            if(udwCastleTotalRate > udwRand)
            {
                udwCastleRewardIdx = udwIdx;
                break;
            }
        }
    }
    //材料/宝石/灵魂/部件
    TUINT32 udwTotalRate = 0;
    TUINT32 udwRewardIdx = 0;
    for(TUINT32 udwIdx = 0; udwIdx < oRewardRaw["r3"].size(); ++udwIdx)
    {
        udwTotalRate += oRewardRaw["r3"][udwIdx][3U].asInt();
    }
    if(udwTotalRate != 0)
    {
        TUINT32 udwRand = rand() % udwTotalRate;
        udwTotalRate = 0;
        for(TUINT32 udwIdx = 0; udwIdx < oRewardRaw["r3"].size(); ++udwIdx)
        {
            udwTotalRate += oRewardRaw["r3"][udwIdx][3U].asUInt();
            if(udwTotalRate > udwRand)
            {
                udwRewardIdx = udwIdx;
                break;
            }
        }
        TBOOL bHasAdd = FALSE;
        for(TUINT32 udwIdY = 0; udwIdY < pstGlobalRes->ddwTotalNum; ++udwIdY)
        {
            if(pstGlobalRes->aRewardList[udwIdY].ddwType == oRewardRaw["r3"][udwRewardIdx][0U].asUInt() &&
                pstGlobalRes->aRewardList[udwIdY].ddwId == oRewardRaw["r3"][udwRewardIdx][1U].asUInt())
            {
                pstGlobalRes->aRewardList[udwIdY].ddwNum += oRewardRaw["r3"][udwRewardIdx][2U].asUInt();
                bHasAdd = TRUE;
                break;
            }
        }

        if(!bHasAdd)
        {
            pstGlobalRes->aRewardList[pstGlobalRes->ddwTotalNum].ddwType = oRewardRaw["r3"][udwRewardIdx][0U].asUInt();
            pstGlobalRes->aRewardList[pstGlobalRes->ddwTotalNum].ddwId = oRewardRaw["r3"][udwRewardIdx][1U].asUInt();
            pstGlobalRes->aRewardList[pstGlobalRes->ddwTotalNum].ddwNum = oRewardRaw["r3"][udwRewardIdx][2U].asUInt();
            pstGlobalRes->ddwTotalNum++;
        }
    }
    //经验/fund/loyalty
    for(TUINT32 udwIdx = 0; udwIdx < oRewardRaw["r2"].size(); ++udwIdx)
    {
        TBOOL bHasAdd = FALSE;
        for(TUINT32 udwIdY = 0; udwIdY < pstGlobalRes->ddwTotalNum; ++udwIdY)
        {
            if(pstGlobalRes->aRewardList[udwIdY].ddwType == oRewardRaw["r2"][udwIdx][0U].asUInt() &&
                pstGlobalRes->aRewardList[udwIdY].ddwId == oRewardRaw["r2"][udwIdx][1U].asUInt())
            {
                pstGlobalRes->aRewardList[udwIdY].ddwNum += oRewardRaw["r2"][udwIdx][2U].asUInt();
                bHasAdd = TRUE;
                break;
            }
        }

        if(!bHasAdd)
        {
            pstGlobalRes->aRewardList[pstGlobalRes->ddwTotalNum].ddwType = oRewardRaw["r2"][udwIdx][0U].asUInt();
            pstGlobalRes->aRewardList[pstGlobalRes->ddwTotalNum].ddwId = oRewardRaw["r2"][udwIdx][1U].asUInt();
            pstGlobalRes->aRewardList[pstGlobalRes->ddwTotalNum].ddwNum = oRewardRaw["r2"][udwIdx][2U].asUInt();
            pstGlobalRes->ddwTotalNum++;
        }

    }
    //跟castle lv相关的reward(资源)
    for(TUINT32 udwIdx = 0; udwIdx < oRewardRaw["r1"][udwCastleRewardIdx].size(); ++udwIdx)
    {
        pstGlobalRes->aRewardList[pstGlobalRes->ddwTotalNum].ddwType = oRewardRaw["r1"][udwCastleRewardIdx][udwIdx][0U].asUInt();
        pstGlobalRes->aRewardList[pstGlobalRes->ddwTotalNum].ddwId = oRewardRaw["r1"][udwCastleRewardIdx][udwIdx][1U].asUInt();
        pstGlobalRes->aRewardList[pstGlobalRes->ddwTotalNum].ddwNum = oRewardRaw["r1"][udwCastleRewardIdx][udwIdx][2U].asUInt() * udwCastleLv;
        pstGlobalRes->ddwTotalNum++;
    }
    
    return 0;
}



TINT32 CGlobalResLogic::GetBuildingReward(TUINT32 udwBuildingType, TUINT32 udwBuildLv, SSpGlobalRes *pstGlobalRes)
{
    TCHAR szBuildingType[64];
    snprintf(szBuildingType, 64, "%u", udwBuildingType);

    if(pstGlobalRes == NULL)
    {
        return -1;
    }

    if(udwBuildingType > EN_BUILDING_TYPE__END)
    {
        return -2;
    }

    if(udwBuildLv > MAX_BUILDING_LEVEL)
    {
        return -3;
    }
    const Json::Value &oRewardRaw = CGameInfo::GetInstance()->m_oJsonRoot["game_building"][szBuildingType]["b"]["b1"][udwBuildLv - 1];

    for(TUINT32 udwIdx = 0; udwIdx < oRewardRaw.size(); ++udwIdx)
    {
        pstGlobalRes->aRewardList[pstGlobalRes->udwTotalNum].udwType = oRewardRaw[udwIdx][0U].asUInt();
        pstGlobalRes->aRewardList[pstGlobalRes->udwTotalNum].udwId = oRewardRaw[udwIdx][1U].asUInt();
        pstGlobalRes->aRewardList[pstGlobalRes->udwTotalNum].udwNum = oRewardRaw[udwIdx][2U].asUInt();
        pstGlobalRes->udwTotalNum++;
    }
    return 0;
}

TINT32 CGlobalResLogic::GetObstleReward(TUINT32 udwBuildingType, TUINT32 udwBuildLv, SSpGlobalRes *pstGlobalRes)
{
    TCHAR szBuildingType[64];
    snprintf(szBuildingType, 64, "%u", udwBuildingType);

    if(pstGlobalRes == NULL)
    {
        return -1;
    }

    if(udwBuildingType > CGameInfo::GetInstance()->m_oJsonRoot["game_building"].size())
    {
        return -2;
    }

    if(udwBuildLv > MAX_BUILDING_LEVEL)
    {
        return -3;
    }
    const Json::Value &oRewardRaw = CGameInfo::GetInstance()->m_oJsonRoot["game_building"][szBuildingType]["b"]["b1"][udwBuildLv - 1];

    for(TUINT32 udwIdx = 0; udwIdx < oRewardRaw.size(); ++udwIdx)
    {
        pstGlobalRes->aRewardList[pstGlobalRes->udwTotalNum].udwType = oRewardRaw[udwIdx][0U].asUInt();
        pstGlobalRes->aRewardList[pstGlobalRes->udwTotalNum].udwId = oRewardRaw[udwIdx][1U].asUInt();
        pstGlobalRes->aRewardList[pstGlobalRes->udwTotalNum].udwNum = oRewardRaw[udwIdx][2U].asUInt();
        pstGlobalRes->udwTotalNum++;
    }
    return 0;
}

TINT32 CGlobalResLogic::GetResearchReward(TUINT32 udwResearchType, TUINT32 udwResearchLv, SSpGlobalRes *pstGlobalRes)
{
    TCHAR szBuildingType[64];
    snprintf(szBuildingType, 64, "%u", udwResearchType);

    if(pstGlobalRes == NULL)
    {
        return -1;
    }

    if(udwResearchType > EN_RESEARCH_TYPE__END)
    {
        return -2;
    }

    if(udwResearchLv > MAX_RESEARCH_LEVEL)
    {
        return -3;
    }
    const Json::Value &oRewardRaw = CGameInfo::GetInstance()->m_oJsonRoot["game_research"][szBuildingType]["b"]["b1"][udwResearchLv - 1];

    for(TUINT32 udwIdx = 0; udwIdx < oRewardRaw.size(); ++udwIdx)
    {
        pstGlobalRes->aRewardList[pstGlobalRes->udwTotalNum].udwType = oRewardRaw[udwIdx][0U].asUInt();
        pstGlobalRes->aRewardList[pstGlobalRes->udwTotalNum].udwId = oRewardRaw[udwIdx][1U].asUInt();
        pstGlobalRes->aRewardList[pstGlobalRes->udwTotalNum].udwNum = oRewardRaw[udwIdx][2U].asUInt();
        pstGlobalRes->udwTotalNum++;
    }
    return 0;
}

TINT32 CGlobalResLogic::GetAlGiftRewarId(Json::Value& oJsonRoot)
{
    TUINT32  udwId = 0;
    const Json::Value &rRewardList = oJsonRoot["reward_list"];
    TUINT32 udwTotalRate = 0;
    for(TUINT32 udwIdx = 0; udwIdx < rRewardList.size(); ++udwIdx)
    {
        udwTotalRate += rRewardList[udwIdx][1U].asUInt();
    }
    assert(udwTotalRate != 0);
    TUINT32 udwRandomNum = rand()%udwTotalRate;
    
    udwTotalRate = 0;
    for(TUINT32 udwIdx = 0; udwIdx < rRewardList.size(); ++udwIdx)
    {
        udwTotalRate += rRewardList[udwIdx][1U].asUInt();
        if(udwRandomNum <= udwTotalRate)
        {
            udwId = rRewardList[udwIdx][0U].asUInt();
            break;
        }
    }
    return udwId;
}

TVOID CGlobalResLogic::ComputeBuffRewarRate(Json::Value& oJsonRoot, TbMarch_action* ptbMarch,TBOOL bRateType)
{
    //rate type: 0:10000表示1    1:精确到小数点6位
    //reward 格式
    //[
    //    [
    //        type(int) // 
    //        id(int) //id 当type为3时id为(0-gold, 1-food, 2-wood, 3-stone, 4-ore)
    //        num(int) //num
    //        rate(double) //rate,当获取方式为0、1时，该值浮点数，精确到小数点后6位，当获取方式为2时，该值为整数
    //    ]
    //]

    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    TINT64 ddwSoulGetRateBuff = CActionBase::GetUserBuffById(ptbMarch, EN_BUFFER_INFO_CORE_GATHERING_RATE, udwCurTime);
    TINT64 ddwPartsGetRateBuff = CActionBase::GetUserBuffById(ptbMarch, EN_BUFFER_INFO_PARTS_GATHERING_RATE, udwCurTime);
    TINT64 ddwCrystalGetRateBuff = CActionBase::GetUserBuffById(ptbMarch, EN_BUFFER_INFO_CRYSTAL_GATHERING_RATE, udwCurTime);

    TFLOAT64 ffRate = 0.0001;
    for(TUINT32 udwIdx = 0; udwIdx < oJsonRoot.size();++udwIdx)
    {
        TUINT32 udwRewardType = oJsonRoot[udwIdx][0U].asUInt();
        switch(udwRewardType)
        {
        case EN_GLOBALRES_TYPE_EQUIP_SOUL:
            if(bRateType)
            {
                double dfRate = oJsonRoot[udwIdx][3U].asDouble();
                TINT64 ddwRate = ((dfRate * 1000000) * (1.0 + ffRate * ddwSoulGetRateBuff));
                oJsonRoot[udwIdx][3U] = ddwRate*1.0 / 1000000;

            }
            else
            {
                TUINT32 udwRate = oJsonRoot[udwIdx][3U].asUInt();
                oJsonRoot[udwIdx][3U] = udwRate * (1.0 + ffRate * ddwSoulGetRateBuff);
               
            }
            
            break;
        case EN_GLOBALRES_TYPE_EQUIP_PARTS:
            if(bRateType)
            {
                double dfRate = oJsonRoot[udwIdx][3U].asDouble();
                TINT64 ddwRate = ((dfRate * 1000000) * (1.0 + ffRate * ddwPartsGetRateBuff));
                oJsonRoot[udwIdx][3U] = ddwRate*1.0 / 1000000;

            }
            else
            {
                TUINT32 udwRate = oJsonRoot[udwIdx][3U].asUInt();
                oJsonRoot[udwIdx][3U] = udwRate * (1.0 + ffRate * ddwPartsGetRateBuff);
                
            }
            break;
        case EN_GLOBALRES_TYPE_EQUIP_CRYSTAL:
            if(bRateType)
            {
                double dfRate = oJsonRoot[udwIdx][3U].asDouble();
                TINT64 ddwRate = ((dfRate * 1000000) * (1.0 + ffRate * ddwCrystalGetRateBuff));
                oJsonRoot[udwIdx][3U] = ddwRate*1.0 / 1000000;
                
            }
            else
            {
                TUINT32 udwRate = oJsonRoot[udwIdx][3U].asUInt();
                oJsonRoot[udwIdx][3U] = udwRate * (1.0 + ffRate * ddwCrystalGetRateBuff);
                
            }
            break;
        default:
            break;
        }
    }
}

TINT32 CGlobalResLogic::SetGlobalRes(SUserInfo *pstUser, SCityInfo* pstCity, TUINT32 udwType, TUINT32 udwItemId, TUINT32 udwItemNum)
{
    TbLogin *ptbLogin = &pstUser->m_tbLogin;

    TINT32 dwRetcode = 0;

    switch (udwType)
    {
    case EN_GLOBALRES_TYPE_ITEM:
        dwRetcode = CItemBase::SetItem(&pstUser->m_tbBackpack, udwItemId, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_GEM:
        CPlayerBase::SetGem(ptbLogin, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_RESOURCE:
        CCityBase::SetResource(pstCity, udwItemId, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_TROOP:
        CCityBase::SetTroop(pstCity, udwItemId, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_FORT:
        CCityBase::SetFort(pstCity, udwItemId, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_EQUIP_MATERIAL:
        CBackpack::OpSetMaterial(pstUser, udwItemId, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_EQUIP_CRYSTAL:
        CBackpack::SetCrystal(pstUser, udwItemId, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_SCROLL:
        CBackpack::SetScroll(pstUser, udwItemId, udwItemNum);
        break;
    case EN_GLOBALRES_TYPE_DRAGON_SHARD:
        CPlayerBase::SetDragonShard(pstUser, udwItemNum);
        break;
    default:
        dwRetcode = -1;
        break;
    }

    return dwRetcode;
}
