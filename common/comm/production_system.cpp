#include "city_base.h"
#include "common_func.h"
#include "game_info.h"
#include "time_utils.h"
#include "common_base.h"
#include "production_system.h"
#include "player_base.h"
#include "action_base.h"
#include "buffer_base.h"
#include "tool_base.h"

using namespace wtse::log;
using namespace std;

TVOID CProductionSystem::ComputeProductionSystem(SUserInfo *pstUser, SCityInfo *pstCity, TUINT32 udwTime)
{
    TBOOL bDeadPlayer = FALSE;
    
    // 主城动态计算人口和资源状况：判定是否需要更改happiness, 并根据具体情况更改happiness数值及相关资源数值，并更新时间

    bDeadPlayer = CPlayerBase::IsDeadPlayer(&pstUser->m_tbPlayer, udwTime);
    if(bDeadPlayer == TRUE)
    {
        ComputeCityResource(pstCity, pstUser, pstUser->m_tbPlayer.m_nUtime + MAX_PLAYER_DEAD_TIME);
    }
    else
    {
        // 重新计算资源
        ComputeCityResource(pstCity, pstUser, udwTime);
    }

    // 4. 统计action各项
    ComputeActionStat(pstUser, pstCity);
}

TVOID CProductionSystem::ComputeBaseProduction(SCityInfo *pstCity,SUserInfo *pstUser)
{
    TUINT32 idx = 0;
    

    // other
    for(idx = EN_RESOURCE_TYPE__GOLD; idx < EN_RESOURCE_TYPE__END; idx++)
    {
        pstCity->m_astResProduction[idx].m_ddwBaseProduction =
            pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_GOLD_PRODUCTION + idx].m_ddwBuffTotal;
    }
}

TINT32 CProductionSystem::ComputeCityResource(SCityInfo *pstCity, SUserInfo *pstUser, TUINT32 udwTime)
{
    if(CTimeUtils::GetUnixTime() < pstCity->m_stTblData.m_nUtime)
    {
        return -1;
    }

    // 1. 获取生产率\容量\upkeep等值
    ComputeCityProduction(pstCity, pstUser);

    // 2. 计算时差
    TINT32 dwTimeDiff = udwTime - pstCity->m_stTblData.m_nUtime;

    if(dwTimeDiff < 10)
    {
        return 0;
    }
    // 3. 计算资源数据(生产+消耗)
    SResourceProduction *pstResProduction = NULL;
    TINT64 ddwResource = 0;
    TINT64 ddwProduction = 0;
    TINT64 ddwRawResource = 0;
    //TINT32 dwCheckResource = 0;
    for(TINT32 dwIdx = 0; dwIdx < EN_RESOURCE_TYPE__END; dwIdx++)
    {
        pstResProduction = &pstCity->m_astResProduction[dwIdx];
        ddwProduction = (TINT64)pstResProduction->m_ddwCurProduction - pstResProduction->m_uddwUpkeep;

        ddwRawResource = pstCity->m_stTblData.m_bResource.m_astList[0].m_addwNum[dwIdx];

        ddwResource = pstCity->m_stTblData.m_bResource.m_astList[0].m_addwNum[dwIdx];
        ddwResource += ddwProduction * dwTimeDiff / 3600;

        if(ddwResource >= pstCity->m_stTblData.m_bResource.m_astList[0].m_addwNum[dwIdx])
        {
            if(pstCity->m_stTblData.m_bResource.m_astList[0].m_addwNum[dwIdx] > (TUINT32)pstResProduction->m_ddwCapacity)
            {
                continue;
            }
            else if(ddwResource > pstResProduction->m_ddwCapacity)
            {
                pstCity->m_stTblData.m_bResource.m_astList[0].m_addwNum[dwIdx] = pstResProduction->m_ddwCapacity;
            }
            else
            {
                pstCity->m_stTblData.m_bResource.m_astList[0].m_addwNum[dwIdx] = ddwResource;
            }
        }
        else if(ddwResource >= 0)
        {
            pstCity->m_stTblData.m_bResource.m_astList[0].m_addwNum[dwIdx] = ddwResource;
        }
        else
        {
            pstCity->m_stTblData.m_bResource.m_astList[0].m_addwNum[dwIdx] = 0;
        }

        if(dwIdx == EN_RESOURCE_TYPE__GOLD && pstCity->m_stTblData.m_bResource.m_astList[0].m_addwNum[dwIdx] == 0)
        {
            CPlayerBase::AutoUnassignKnight(pstUser);
        }
    }

    

    // 5. 更新时间
    pstCity->m_stTblData.Set_Utime(udwTime);

    // 6. 设置flag
    pstCity->m_stTblData.SetFlag(TbCITY_FIELD_RESOURCE);

    return 0;
}

TVOID CProductionSystem::ComputeCityResUpkeep(SCityInfo *pstCity, SUserInfo *pstUser)
{
    TUINT32 idx = 0, idy = 0;
    CGameInfo *poGameInfo = CGameInfo::GetInstance();
    TINT64 addwTroop[EN_TROOP_TYPE__END];
    const Json::Value &pTroopInfo = poGameInfo->m_oJsonRoot["game_troop"];

    pstCity->m_astResProduction[EN_RESOURCE_TYPE__GOLD].m_addwBonus[EN_GOLD_BONUS_TYPE__KNIGHT_SALARY] = pstCity->m_astResProduction[EN_RESOURCE_TYPE__GOLD].m_uddwUpkeep;

    // city-troop
    memcpy((TCHAR*)addwTroop, (TCHAR*)pstCity->m_stTblData.m_bTroop.m_astList[0].m_addwNum, sizeof(TINT64)*EN_TROOP_TYPE__END);

    // active-action-troop
    for(idx = 0; idx < pstUser->m_udwMarchNum; idx++)
    {
        if(EN_ACTION_MAIN_CLASS__MARCH != pstUser->m_atbMarch[idx].m_nMclass)
        {
            continue;
        }
        if(!CActionBase::IsPlayerOwnedAction(pstUser->m_tbPlayer.m_nUid, pstUser->m_atbMarch[idx].m_nId))
        {
            continue;
        }
        if(EN_ACTION_SEC_CLASS__SCOUT == pstUser->m_atbMarch[idx].m_nSclass)
        {
            continue;
        }
        if(pstUser->m_atbMarch[idx].m_nScid != pstCity->m_stTblData.m_nPos)
        {
            continue;
        }
        // troop
        for(idy = 0; idy < EN_TROOP_TYPE__END; idy++)
        {
            addwTroop[idy] += pstUser->m_atbMarch[idx].m_bParam[0].m_stTroop.m_addwNum[idy];
        }
    }

    // upkeep
    TUINT32 udwSignleUpkeep = 0;
    for(idx = 0; idx < EN_TROOP_TYPE__END; idx++)
    {
        if(addwTroop[idx] > 0)
        {
            udwSignleUpkeep = pTroopInfo[idx]["a"]["a8"].asUInt();
            pstCity->m_astResProduction[EN_RESOURCE_TYPE__FOOD].m_uddwUpkeep +=
                addwTroop[idx] * udwSignleUpkeep;
        }
    }

    // gold_upkeep----for knight
    pstCity->m_uddwKnightCostGold = 0;
    udwSignleUpkeep = CGameInfo::GetInstance()->GetBasicVal(EN_GAME_BASIC_KNIGHT_ASSIGN_COST_GOLD);
    for(idx = 0; idx < pstCity->m_stTblData.m_bKnight.m_udwNum; idx++)
    {
        SKnightInfo *pstKnight = &pstCity->m_stTblData.m_bKnight[idx];
        if(pstKnight->ddwPos != EN_KNIGHT_POS__UNASSIGN)
        {
            TUINT32 udwLevel = CGameInfo::GetInstance()->ComputeKnightLevelByExp(pstKnight->ddwExp);
            pstCity->m_uddwKnightCostGold += udwSignleUpkeep * udwLevel;
        }
    }
    pstCity->m_astResProduction[EN_RESOURCE_TYPE__GOLD].m_uddwUpkeep += pstCity->m_uddwKnightCostGold;

    //buffer_info
    TINT32 dwUpkeepBuffer = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_ALL_TROOP_UPKEEP].m_ddwBuffTotal;
    TFLOAT32 dFBuffer = (10000 - dwUpkeepBuffer * 1.0) / 10000;
    pstCity->m_astResProduction[EN_RESOURCE_TYPE__FOOD].m_uddwUpkeep = pstCity->m_astResProduction[EN_RESOURCE_TYPE__FOOD].m_uddwUpkeep * dFBuffer;

    TSE_LOG_INFO(poGameInfo->m_poLog, ("ComputeCityResUpkeep: upkeep  [gold_upkeep=%u knight_upkeep=%u food_upkeep=%u]",
        pstCity->m_astResProduction[EN_RESOURCE_TYPE__GOLD].m_uddwUpkeep,
        pstCity->m_astResProduction[EN_RESOURCE_TYPE__GOLD].m_addwBonus[EN_GOLD_BONUS_TYPE__KNIGHT_SALARY],
        pstCity->m_astResProduction[EN_RESOURCE_TYPE__FOOD].m_uddwUpkeep));
}

TVOID CProductionSystem::ComputeCityProduction(SCityInfo *pstCity, SUserInfo *pstUser)
{
    // 0. reset
    for(TUINT32 idx = 0; idx < EN_RESOURCE_TYPE__END; idx++)
    {
        pstCity->m_astResProduction[idx].Reset();
    }
    // 1. base production
    ComputeBaseProduction(pstCity, pstUser);
    // 3. research bonus
    ComputeResearchBonus(pstCity, pstUser);

    // 5. item bonus
    ComputeItemBonus(pstCity, pstUser);
    // 6. other bonus
    ComputeOtherBonus(pstCity, pstUser);

    ComputeVipBonus(pstCity, pstUser);

    ComputeLordBonus(pstCity, pstUser);

    ComputeDragonBonus(pstCity, pstUser);

    // 7. total bonus\total amount\cur production
    ComputeTotalProduction(pstCity, pstUser);
    // 8. upkeep
    ComputeCityResUpkeep(pstCity, pstUser);
    // 9. capacity
    ComputeCityResCapacity(pstCity, pstUser);
}

TVOID CProductionSystem::ComputeCityResCapacity(SCityInfo *pstCity, SUserInfo *pstUser)
{
    // other
    for(TUINT32 udwIdx = EN_RESOURCE_TYPE__GOLD; udwIdx < EN_RESOURCE_TYPE__END; udwIdx++)
    {
        pstCity->m_astResProduction[udwIdx].m_ddwCapacity =
            pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_GOLD_CAPICATY + udwIdx].m_ddwBuffTotal
            + pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_ALL_RESOURCE_CAPACITY].m_ddwBuffTotal;
    }
}

TVOID CProductionSystem::ComputeTotalProduction(SCityInfo *pstCity, SUserInfo *pstUser)
{
    for(TUINT32 udwSourceIdx = EN_RESOURCE_TYPE__GOLD; udwSourceIdx < EN_RESOURCE_TYPE__END; ++udwSourceIdx)
    {
        pstCity->m_astResProduction[udwSourceIdx].m_ddwTotalBonus = 
            pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_GOLD_PRODUCTION_PERCENT + udwSourceIdx].m_ddwBuffTotal;
        pstCity->m_astResProduction[udwSourceIdx].m_ddwTotalBonus += 
            pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_ALL_RESOURCE_PRODUCTION_PERCENT].m_ddwBuffTotal;

        pstCity->m_astResProduction[udwSourceIdx].m_ddwTotalBonusAmount =
            (pstCity->m_astResProduction[udwSourceIdx].m_ddwTotalBonus) / 10000.0 * pstCity->m_astResProduction[udwSourceIdx].m_ddwBaseProduction;

        pstCity->m_astResProduction[udwSourceIdx].m_ddwCurProduction =
            pstCity->m_astResProduction[udwSourceIdx].m_ddwBaseProduction + pstCity->m_astResProduction[udwSourceIdx].m_ddwTotalBonusAmount;
    }
}

TVOID CProductionSystem::ComputeResearchBonus(SCityInfo *pstCity, SUserInfo *pstUser)
{
	for(TUINT32 udwIdx = 0; udwIdx < EN_RESOURCE_TYPE__END; ++udwIdx)
	{
		pstCity->m_astResProduction[udwIdx].m_addwBonus[EN_BONUS_TYPE__RESEARCH] =
			pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_GOLD_PRODUCTION_PERCENT + udwIdx].m_astBuffDetail[EN_BUFF_TYPE_RESEARCH].m_ddwNum;
        pstCity->m_astResProduction[udwIdx].m_addwBonus[EN_BONUS_TYPE__RESEARCH] +=
            pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_ALL_RESOURCE_PRODUCTION_PERCENT].m_astBuffDetail[EN_BUFF_TYPE_RESEARCH].m_ddwNum;
	}
}

TVOID CProductionSystem::ComputeItemBonus(SCityInfo *pstCity, SUserInfo *pstUser)
{
	for(TUINT32 udwIdx = 0; udwIdx < EN_RESOURCE_TYPE__END; ++udwIdx)
    {
        pstCity->m_astResProduction[udwIdx].m_addwBonus[EN_BONUS_TYPE__ITEM] =
            pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_GOLD_PRODUCTION_PERCENT + udwIdx].m_astBuffDetail[EN_BUFF_TYPE_ITEM].m_ddwNum;
        pstCity->m_astResProduction[udwIdx].m_addwBonus[EN_BONUS_TYPE__ITEM] +=
            pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_ALL_RESOURCE_PRODUCTION_PERCENT].m_astBuffDetail[EN_BUFF_TYPE_ITEM].m_ddwNum;
	}
}

TVOID CProductionSystem::ComputeVipBonus(SCityInfo *pstCity, SUserInfo *pstUser)
{
	for(TUINT32 udwIdx = 0; udwIdx < EN_RESOURCE_TYPE__END; ++udwIdx)
    {
        pstCity->m_astResProduction[udwIdx].m_addwBonus[EN_BONUS_TYPE__VIP] =
            pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_GOLD_PRODUCTION_PERCENT + udwIdx].m_astBuffDetail[EN_BUFF_TYPE_VIP].m_ddwNum;
        pstCity->m_astResProduction[udwIdx].m_addwBonus[EN_BONUS_TYPE__VIP] +=
            pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_ALL_RESOURCE_PRODUCTION_PERCENT].m_astBuffDetail[EN_BUFF_TYPE_VIP].m_ddwNum;
	}
}

TVOID CProductionSystem::ComputeLordBonus(SCityInfo *pstCity, SUserInfo *pstUser)
{
    for (TUINT32 udwIdx = 0; udwIdx < EN_RESOURCE_TYPE__END; ++udwIdx)
    {
        pstCity->m_astResProduction[udwIdx].m_addwBonus[EN_BONUS_TYPE__LORD] +=
            pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_GOLD_PRODUCTION_PERCENT + udwIdx].m_astBuffDetail[EN_BUFF_TYPE_LORD_SKILL].m_ddwNum;
        pstCity->m_astResProduction[udwIdx].m_addwBonus[EN_BONUS_TYPE__LORD] +=
            pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_ALL_RESOURCE_PRODUCTION_PERCENT].m_astBuffDetail[EN_BUFF_TYPE_LORD_SKILL].m_ddwNum;
    }
}

TVOID CProductionSystem::ComputeOtherBonus(SCityInfo *pstCity, SUserInfo *pstUser)
{
    for(TUINT32 udwIdx = 0; udwIdx < EN_RESOURCE_TYPE__END; ++udwIdx)
    {
        pstCity->m_astResProduction[udwIdx].m_addwBonus[EN_BONUS_TYPE__OTHER] +=
            pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_GOLD_PRODUCTION_PERCENT + udwIdx].m_astBuffDetail[EN_BUFF_TYPE_BASIC].m_ddwNum;
        pstCity->m_astResProduction[udwIdx].m_addwBonus[EN_BONUS_TYPE__OTHER] +=
            pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_ALL_RESOURCE_PRODUCTION_PERCENT].m_astBuffDetail[EN_BUFF_TYPE_BASIC].m_ddwNum;

        pstCity->m_astResProduction[udwIdx].m_addwBonus[EN_BONUS_TYPE__OTHER] =
            pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_GOLD_PRODUCTION_PERCENT + udwIdx].m_astBuffDetail[EN_BUFF_TYPE_TITLE].m_ddwNum;
        pstCity->m_astResProduction[udwIdx].m_addwBonus[EN_BONUS_TYPE__OTHER] +=
            pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_ALL_RESOURCE_PRODUCTION_PERCENT].m_astBuffDetail[EN_BUFF_TYPE_TITLE].m_ddwNum;

        pstCity->m_astResProduction[udwIdx].m_addwBonus[EN_BONUS_TYPE__OTHER] +=
            pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_GOLD_PRODUCTION_PERCENT + udwIdx].m_astBuffDetail[EN_BUFF_TYPE_ALTAR].m_ddwNum;
        pstCity->m_astResProduction[udwIdx].m_addwBonus[EN_BONUS_TYPE__OTHER] +=
            pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_ALL_RESOURCE_PRODUCTION_PERCENT].m_astBuffDetail[EN_BUFF_TYPE_ALTAR].m_ddwNum;

        pstCity->m_astResProduction[udwIdx].m_addwBonus[EN_BONUS_TYPE__OTHER] +=
            pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_GOLD_PRODUCTION_PERCENT + udwIdx].m_astBuffDetail[EN_BUFF_TYPE_THRONE].m_ddwNum;
        pstCity->m_astResProduction[udwIdx].m_addwBonus[EN_BONUS_TYPE__OTHER] +=
            pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_ALL_RESOURCE_PRODUCTION_PERCENT].m_astBuffDetail[EN_BUFF_TYPE_THRONE].m_ddwNum;

        pstCity->m_astResProduction[udwIdx].m_addwBonus[EN_BONUS_TYPE__OTHER] +=
            pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_GOLD_PRODUCTION_PERCENT + udwIdx].m_astBuffDetail[EN_BUFF_TYPE_FORT].m_ddwNum;
        pstCity->m_astResProduction[udwIdx].m_addwBonus[EN_BONUS_TYPE__OTHER] +=
            pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_ALL_RESOURCE_PRODUCTION_PERCENT].m_astBuffDetail[EN_BUFF_TYPE_FORT].m_ddwNum;

        pstCity->m_astResProduction[udwIdx].m_addwBonus[EN_BONUS_TYPE__OTHER] +=
            pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_GOLD_PRODUCTION_PERCENT + udwIdx].m_astBuffDetail[EN_BUFF_TYPE_TROOP].m_ddwNum;
        pstCity->m_astResProduction[udwIdx].m_addwBonus[EN_BONUS_TYPE__OTHER] +=
            pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_ALL_RESOURCE_PRODUCTION_PERCENT].m_astBuffDetail[EN_BUFF_TYPE_TROOP].m_ddwNum;

        pstCity->m_astResProduction[udwIdx].m_addwBonus[EN_BONUS_TYPE__OTHER] +=
            pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_GOLD_PRODUCTION_PERCENT + udwIdx].m_astBuffDetail[EN_BUFF_TYPE_BUILDING].m_ddwNum;
        pstCity->m_astResProduction[udwIdx].m_addwBonus[EN_BONUS_TYPE__OTHER] +=
            pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_ALL_RESOURCE_PRODUCTION_PERCENT].m_astBuffDetail[EN_BUFF_TYPE_BUILDING].m_ddwNum;

        pstCity->m_astResProduction[udwIdx].m_addwBonus[EN_BONUS_TYPE__OTHER] +=
            pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_GOLD_PRODUCTION_PERCENT + udwIdx].m_astBuffDetail[EN_BUFF_TYPE_DRAGON_LV].m_ddwNum;
        pstCity->m_astResProduction[udwIdx].m_addwBonus[EN_BONUS_TYPE__OTHER] +=
            pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_ALL_RESOURCE_PRODUCTION_PERCENT].m_astBuffDetail[EN_BUFF_TYPE_DRAGON_LV].m_ddwNum;

        pstCity->m_astResProduction[udwIdx].m_addwBonus[EN_BONUS_TYPE__OTHER] +=
            pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_GOLD_PRODUCTION_PERCENT + udwIdx].m_astBuffDetail[EN_BUFF_TYPE_THRONE_RESEARCH].m_ddwNum;
        pstCity->m_astResProduction[udwIdx].m_addwBonus[EN_BONUS_TYPE__OTHER] +=
            pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_ALL_RESOURCE_PRODUCTION_PERCENT].m_astBuffDetail[EN_BUFF_TYPE_THRONE_RESEARCH].m_ddwNum;

        pstCity->m_astResProduction[udwIdx].m_addwBonus[EN_BONUS_TYPE__OTHER] +=
            pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_GOLD_PRODUCTION_PERCENT + udwIdx].m_astBuffDetail[EN_BUFF_TYPE_KNIGHT].m_ddwNum;
        pstCity->m_astResProduction[udwIdx].m_addwBonus[EN_BONUS_TYPE__OTHER] +=
            pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_ALL_RESOURCE_PRODUCTION_PERCENT].m_astBuffDetail[EN_BUFF_TYPE_KNIGHT].m_ddwNum;
    }
}


TVOID CProductionSystem::ComputeDragonBonus(SCityInfo *pstCity, SUserInfo *pstUser)
{
    for(TUINT32 udwIdx = 0; udwIdx < EN_RESOURCE_TYPE__END; ++udwIdx)
    {
        pstCity->m_astResProduction[udwIdx].m_addwBonus[EN_BONUS_TYPE__DRAGON] =
            pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_GOLD_PRODUCTION_PERCENT + udwIdx].m_astBuffDetail[EN_BUFF_TYPE_DRAGON_SKILL].m_ddwNum;
        pstCity->m_astResProduction[udwIdx].m_addwBonus[EN_BONUS_TYPE__DRAGON] +=
            pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_ALL_RESOURCE_PRODUCTION_PERCENT].m_astBuffDetail[EN_BUFF_TYPE_DRAGON_SKILL].m_ddwNum;

        pstCity->m_astResProduction[udwIdx].m_addwBonus[EN_BONUS_TYPE__DRAGON] +=
            pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_GOLD_PRODUCTION_PERCENT + udwIdx].m_astBuffDetail[EN_BUFF_TYPE_EQUIP].m_ddwNum;
        pstCity->m_astResProduction[udwIdx].m_addwBonus[EN_BONUS_TYPE__DRAGON] +=
            pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_ALL_RESOURCE_PRODUCTION_PERCENT].m_astBuffDetail[EN_BUFF_TYPE_EQUIP].m_ddwNum;
    }
}

TVOID CProductionSystem::ComputeActionStat(SUserInfo *pstUser, SCityInfo *pstCity)
{
    TUINT32 idy = 0;

    pstCity->m_stActionStat.Reset();

    pstCity->m_stActionStat.m_bCanPeaceTime = TRUE;

    for(idy = 0; idy < pstUser->m_udwSelfAlActionNum; idy++)
    {
        TbAlliance_action *pstAction = &pstUser->m_atbSelfAlAction[idy];
        
        if(pstUser->m_aucActionFlag[idy] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }

        if (!CActionBase::IsPlayerOwnedAction(pstUser->m_tbPlayer.m_nUid, pstUser->m_atbSelfAlAction[idy].m_nId))
        {
            continue;
        }

        switch(pstAction->m_nMclass)
        {
        case EN_ACTION_MAIN_CLASS__BUILDING:
            if(EN_ACTION_SEC_CLASS__BUILDING_UPGRADE == pstAction->m_nSclass ||
                EN_ACTION_SEC_CLASS__BUILDING_REMOVE == pstAction->m_nSclass ||
                EN_ACTION_SEC_CLASS__REMOVE_OBSTACLE == pstAction->m_nSclass)
            {
                pstCity->m_stActionStat.m_ucDoingBuildingNum++;
            }
            if(EN_ACTION_SEC_CLASS__RESEARCH_UPGRADE == pstAction->m_nSclass)
            {
                pstCity->m_stActionStat.m_ucDoingResearchNum++;
            }
            break;
        case EN_ACTION_MAIN_CLASS__EQUIP:
            pstCity->m_stActionStat.m_ucDoingEquipUpgradeNum++;
            break;
        case EN_ACTION_MAIN_CLASS__TRAIN_NEW:
            if (pstAction->m_nSclass == EN_ACTION_SEC_CLASS__FORT
                || pstAction->m_nSclass == EN_ACTION_SEC_CLASS__FORT_REPAIR)
            {
                pstCity->m_stActionStat.m_ucDoingFortNum++;
            }
            if (pstAction->m_nSclass == EN_ACTION_SEC_CLASS__TROOP)
            {
                TUINT32 udwCategory = CToolBase::GetTroopCategoryByTroopType(pstAction->m_bParam[0].m_stTrain.m_ddwType);
                if (udwCategory < EN_TROOP_CATEGORY__END)
                {
                    pstCity->m_stActionStat.m_aucDoingTroopNum[udwCategory]++;
                }
            }
        }
    }

    for(idy = 0; idy < pstUser->m_udwMarchNum; idy++)
    {
        TbMarch_action *pstAction = &pstUser->m_atbMarch[idy];

        if(pstAction->m_nScid != pstCity->m_stTblData.m_nPos)
        {
            continue;
        }
        if(pstUser->m_aucActionFlag[idy] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }

        switch(pstAction->m_nMclass)
        {
        case EN_ACTION_MAIN_CLASS__MARCH:
            if (pstAction->m_nSclass == EN_ACTION_SEC_CLASS__TRANSPORT)
            {
                pstCity->m_stActionStat.m_ucDoingTransportNum++;
            }
            else
            {
                pstCity->m_stActionStat.m_ucDoingMarchNum++;
                if(pstAction->m_nSclass == EN_ACTION_SEC_CLASS__ATTACK
                    || pstAction->m_nSclass == EN_ACTION_SEC_CLASS__ATTACK_IDOL
                    || pstAction->m_nSclass == EN_ACTION_SEC_CLASS__SCOUT
                    || pstAction->m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_NORMAL
                    || pstAction->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR
                    || pstAction->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_REINFORCE
                    || pstAction->m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_THRONE
                    || pstAction->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR_THRONE
                    || pstAction->m_nSclass == EN_ACTION_SEC_CLASS__ASSIGN_THRONE
                    || pstAction->m_nSclass == EN_ACTION_SEC_CLASS__ATTACK_THRONE)
                {
                    pstCity->m_stActionStat.m_bCanPeaceTime = FALSE;
                }
            }
            break;

        }
    }

    // 被动的action：被攻击、被驻兵等
    for(idy = 0; idy < pstUser->m_udwPassiveMarchNum; idy++)
    {
        TbMarch_action *pstAction = &pstUser->m_atbPassiveMarch[idy];
        if(pstAction->m_nScid == pstCity->m_stTblData.m_nPos) // 本城给本城市所属的wild驻兵，不算encamp
        {
            continue;
        }

        if(pstAction->m_nMclass == EN_ACTION_MAIN_CLASS__MARCH)
        {
            if (pstAction->m_nStatus == EN_MARCH_STATUS__DEFENDING || pstAction->m_nStatus == EN_MARCH_STATUS__SETUP_CAMP)
            {
                if(pstAction->m_bParam.m_astList[0].m_ddwTargetCityId == pstCity->m_stTblData.m_nPos
                    || pstAction->m_nTid == pstCity->m_stTblData.m_nPos)
                {
                    pstCity->m_stActionStat.m_ucDoingEncampNum++;
                }
            }
        }
    }
}

TUINT32 CProductionSystem::ComputeKnightUnassignTime( SUserInfo *pstUser, SCityInfo *pstCity )
{
    if(pstCity->m_uddwKnightCostGold == 0)
    {
        return 0;
    }
    
    if((TUINT64)pstCity->m_astResProduction[EN_RESOURCE_TYPE__GOLD].m_ddwCurProduction >= pstCity->m_astResProduction[EN_RESOURCE_TYPE__GOLD].m_uddwUpkeep)
    {
        return 0;
    }

    if(pstCity->m_stTblData.m_bResource[0].m_addwNum[EN_RESOURCE_TYPE__GOLD] == 0)
    {
        return 0;
    }

    TUINT32 udwRealCost = pstCity->m_astResProduction[EN_RESOURCE_TYPE__GOLD].m_uddwUpkeep - pstCity->m_astResProduction[EN_RESOURCE_TYPE__GOLD].m_ddwCurProduction;
    TUINT32 udwCostTime = 3600 * pstCity->m_stTblData.m_bResource[0].m_addwNum[EN_RESOURCE_TYPE__GOLD]/udwRealCost;

    return CTimeUtils::GetUnixTime() + udwCostTime;
}
