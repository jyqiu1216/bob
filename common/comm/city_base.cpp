#include "city_base.h"
#include "common_func.h"
#include "game_info.h"
#include "time_utils.h"
#include "game_define.h"
#include <math.h>
#include "service_key.h"
#include "action_base.h"
#include "common_base.h"
#include "global_serv.h"

using namespace wtse::log;
using namespace std;

TINT64 CCityBase::CalcTroopMight(TINT64 *addwTroop)
{
    TINT64 ddwMight = 0;
    TINT32 dwSingleMight = 0;
    CGameInfo *pobjGameInfo = CGameInfo::GetInstance();

    for (TUINT32 udwIdx = EN_TROOP_TYPE__T1_INFANTRY; udwIdx < EN_TROOP_TYPE__END; ++udwIdx)
    {
        if (addwTroop[udwIdx] > 0)
        {
            dwSingleMight = pobjGameInfo->m_oJsonRoot["game_troop"][udwIdx]["a"]["a9"].asInt();
            ddwMight += dwSingleMight * addwTroop[udwIdx];
        }
    }

    return ddwMight;
}

TINT64 CCityBase::CalcFortMight(TINT64 *addwFort)
{
    TINT64 ddwMight = 0;
    TINT32 dwSingleMight = 0;
    CGameInfo *pobjGameInfo = CGameInfo::GetInstance();

    for (TUINT32 idx = 0; idx < EN_FORT_TYPE__END; idx++)
    {
        if (addwFort[idx] > 0)
        {
            dwSingleMight = pobjGameInfo->m_oJsonRoot["game_fort"][idx]["a"]["a9"].asUInt();
            ddwMight += dwSingleMight * addwFort[idx];
        }
    }

    return ddwMight;
}

TBOOL CCityBase::HasEnoughTroop(SCityInfo *pstCity, TUINT8 ucType, TINT64 *addwTroop, TUINT32 udwNum)
{
    TINT64 *pddwUserVal = NULL;
    TBOOL bRetCode = TRUE;
    if (ucType == 0)
    {
        pddwUserVal = pstCity->m_stTblData.m_bTroop.m_astList[0].m_addwNum;
    }
    else
    {
        pddwUserVal = pstCity->m_stTblData.m_bFort.m_astList[0].m_addwNum;
    }
    for (TUINT32 idx = 0; idx < udwNum; idx++)
    {
        if (pddwUserVal[idx] < addwTroop[idx])
        {
            bRetCode = FALSE;
            break;
        }
    }
    return bRetCode;
}

TBOOL CCityBase::HasEnoughTroop(SCityInfo *pstCity, TUINT8 ucType, TUINT32 udwId, TINT64 ddwNum)
{
    TINT64 *pddwUserVal = NULL;
    TBOOL bRetCode = TRUE;
    if(ucType == 0)
    {
        pddwUserVal = pstCity->m_stTblData.m_bTroop.m_astList[0].m_addwNum;
    }
    else
    {
        pddwUserVal = pstCity->m_stTblData.m_bFort.m_astList[0].m_addwNum;
    }

    if(pddwUserVal[udwId] < ddwNum)
    {
        bRetCode = FALSE;
    }
    
    return bRetCode;
}

TVOID CCityBase::CalcCityTroopAndFort(SCityInfo *pstCity, TINT64 *addwTroop, TINT64 *addwFort)
{
    if (addwTroop)
    {
        for (TUINT32 udwIdx = 0; udwIdx < EN_TROOP_TYPE__END; ++udwIdx)
        {
            addwTroop[udwIdx] += pstCity->m_stTblData.m_bTroop.m_astList[0].m_addwNum[udwIdx];
        }
    }

    if (addwFort)
    {
        for (TUINT32 udwIdx = 0; udwIdx < EN_FORT_TYPE__END; ++udwIdx)
        {
            addwFort[udwIdx] += pstCity->m_stTblData.m_bFort.m_astList[0].m_addwNum[udwIdx];
        }
    }
}

TVOID CCityBase::AddTroop(SCityInfo *pstCity, TUINT32 udwTroopIdx, TINT64 ddwTroopNum)
{
    pstCity->m_stTblData.m_bTroop.m_astList[0].m_addwNum[udwTroopIdx] += ddwTroopNum;
    pstCity->m_stTblData.m_mFlag[TbCITY_FIELD_TROOP] = UPDATE_ACTION_TYPE_PUT;
}

TVOID CCityBase::SetTroop(SCityInfo *pstCity, TUINT32 udwTroopIdx, TINT64 ddwTroopNum)
{
    pstCity->m_stTblData.m_bTroop.m_astList[0].m_addwNum[udwTroopIdx] = ddwTroopNum;
    pstCity->m_stTblData.m_mFlag[TbCITY_FIELD_TROOP] = UPDATE_ACTION_TYPE_PUT;
}

TVOID CCityBase::AddFort(SCityInfo *pstCity, TUINT32 udwTroopIdx, TINT64 ddwTroopNum)
{
    pstCity->m_stTblData.m_bFort.m_astList[0].m_addwNum[udwTroopIdx] += ddwTroopNum;
    pstCity->m_stTblData.m_mFlag[TbCITY_FIELD_FORT] = UPDATE_ACTION_TYPE_PUT;
}

TVOID CCityBase::SetFort(SCityInfo *pstCity, TUINT32 udwTroopIdx, TINT64 ddwTroopNum)
{
    pstCity->m_stTblData.m_bFort.m_astList[0].m_addwNum[udwTroopIdx] = ddwTroopNum;
    pstCity->m_stTblData.m_mFlag[TbCITY_FIELD_FORT] = UPDATE_ACTION_TYPE_PUT;
}

TBOOL CCityBase::HasEnoughResource(SCityInfo *pstCity, TINT64 *addwResCost)
{
    TBOOL bRet = TRUE;

    for (TUINT32 idx = 0; idx < EN_RESOURCE_TYPE__END; idx++)
    {
        if (pstCity->m_stTblData.m_bResource.m_astList[0].m_addwNum[idx] < addwResCost[idx])
        {
            bRet = FALSE;
            break;
        }
    }

    return bRet;
}

TBOOL CCityBase::HasEnoughGold(SCityInfo *pstCity, TINT64 ddwPrice)
{
    if(pstCity->m_stTblData.m_bResource.m_astList[0].m_addwNum[EN_RESOURCE_TYPE__GOLD] < ddwPrice)
    {
        return FALSE;
    }
    return TRUE;
}

TVOID CCityBase::CostResource(SCityInfo *pstCity, TINT64 *addwResCost)
{
    for (TUINT32 idx = 0; idx < EN_RESOURCE_TYPE__END; idx++)
    {
        if (pstCity->m_stTblData.m_bResource.m_astList[0].m_addwNum[idx] > addwResCost[idx])
        {
            pstCity->m_stTblData.m_bResource.m_astList[0].m_addwNum[idx] -= addwResCost[idx];
        }
        else
        {
            pstCity->m_stTblData.m_bResource.m_astList[0].m_addwNum[idx] = 0;
        }
    }
    pstCity->m_stTblData.m_mFlag[TbCITY_FIELD_RESOURCE] = UPDATE_ACTION_TYPE_PUT;
}

TVOID CCityBase::CostOneResource(SCityInfo *pstCity, TUINT32 udwResourceId, TINT64 ddwResourceNum)
{
    
    if(pstCity->m_stTblData.m_bResource.m_astList[0].m_addwNum[udwResourceId] > ddwResourceNum)
    {
        pstCity->m_stTblData.m_bResource.m_astList[0].m_addwNum[udwResourceId] -= ddwResourceNum;
    }
    else
    {
        pstCity->m_stTblData.m_bResource.m_astList[0].m_addwNum[udwResourceId] = 0;
    }
    
    pstCity->m_stTblData.m_mFlag[TbCITY_FIELD_RESOURCE] = UPDATE_ACTION_TYPE_PUT;
}


TVOID CCityBase::CostGold(SCityInfo *pstCity, TINT64 ddwPrice)
{
    if (pstCity->m_stTblData.m_bResource.m_astList[0].m_addwNum[EN_RESOURCE_TYPE__GOLD] > ddwPrice)
    {
        pstCity->m_stTblData.m_bResource.m_astList[0].m_addwNum[EN_RESOURCE_TYPE__GOLD] -= ddwPrice;
    }
    else
    {
        pstCity->m_stTblData.m_bResource.m_astList[0].m_addwNum[EN_RESOURCE_TYPE__GOLD] = 0;
    }
    pstCity->m_stTblData.m_mFlag[TbCITY_FIELD_RESOURCE] = UPDATE_ACTION_TYPE_PUT;
}

TVOID CCityBase::AddResource(SCityInfo *pstCity, TINT64 *addwResCost)
{
    for (TUINT32 idx = 0; idx < EN_RESOURCE_TYPE__END; idx++)
    {
        pstCity->m_stTblData.m_bResource.m_astList[0].m_addwNum[idx] += addwResCost[idx];
    }
    pstCity->m_stTblData.m_mFlag[TbCITY_FIELD_RESOURCE] = UPDATE_ACTION_TYPE_PUT;
}

TVOID CCityBase::AddGold(SCityInfo *pstCity, TINT64 ddwPrice)
{
    pstCity->m_stTblData.m_bResource.m_astList[0].m_addwNum[EN_RESOURCE_TYPE__GOLD] += ddwPrice;
    pstCity->m_stTblData.m_mFlag[TbCITY_FIELD_RESOURCE] = UPDATE_ACTION_TYPE_PUT;
}

TUINT8 CCityBase::GetBuildingLevelByFuncType(SCityInfo *pstCity, TUINT32 udwFuncType)
{
    if(!pstCity)
    {
        return 0;
    }
    TUINT8 ucLevel = 0;
    for (TUINT32 idx = 0; idx < pstCity->m_stTblData.m_bBuilding.m_udwNum; idx++)
    {
        SCityBuildingNode *pstBuildingNode = &pstCity->m_stTblData.m_bBuilding[idx];
        if(pstBuildingNode->m_ddwLevel == 0)
        {
            continue;
        }
        TINT32 dwFuncType = CCityBase::GetBuildingFuncType(pstBuildingNode->m_ddwType);
        if(dwFuncType == static_cast<TINT32>(udwFuncType))
        {
            if (pstBuildingNode->m_ddwLevel > ucLevel)
            {
                ucLevel = pstBuildingNode->m_ddwLevel;
            }
        }
    }
    return ucLevel;
}

TUINT8 CCityBase::GetBuildingIdByFuncType(TbCity *pstCity, TUINT32 udwFuncType)
{
    for(TUINT32 idx = 0; idx < pstCity->m_bBuilding.m_udwNum; idx++)
    {
        SCityBuildingNode *pstBuildingNode = &pstCity->m_bBuilding[idx];
        if(pstBuildingNode->m_ddwLevel == 0)
        {
            continue;
        }
        TINT32 dwFuncType = CCityBase::GetBuildingFuncType(pstBuildingNode->m_ddwType);
        if(dwFuncType == static_cast<TINT32>(udwFuncType))
        {
            return pstBuildingNode->m_ddwType;;
        }
    }
    return 0;
}

TUINT8 CCityBase::GetBuildingLevelById(TbCity *pstCity, TUINT32 udwBuildId)
{
    TUINT8 ucLevel = 0;
    for(TUINT32 idx = 0; idx < pstCity->m_bBuilding.m_udwNum; idx++)
    {
        SCityBuildingNode *pstBuildingNode = &pstCity->m_bBuilding[idx];
        if(pstBuildingNode->m_ddwLevel == 0)
        {
            continue;
        }
        if(udwBuildId == pstBuildingNode->m_ddwType)
        {
            if(pstBuildingNode->m_ddwLevel > ucLevel)
            {
                ucLevel = pstBuildingNode->m_ddwLevel;
            }
        }
    }
    return ucLevel;
}

TUINT8 CCityBase::GetBuildingNumByFuncType(SCityInfo *pstCity, TUINT32 udwFuncType)
{
    TUINT8 ucNum = 0;
    for (TUINT32 idx = 0; idx < pstCity->m_stTblData.m_bBuilding.m_udwNum; idx++)
    {
        SCityBuildingNode *pstBuildingNode = &pstCity->m_stTblData.m_bBuilding[idx];
        if(pstBuildingNode->m_ddwLevel == 0)
        {
            continue;
        }
        TINT32 dwFuncType = CCityBase::GetBuildingFuncType(pstBuildingNode->m_ddwType);
        if(dwFuncType == static_cast<TINT32>(udwFuncType))
        {
            ucNum++;
        }
    }
    return ucNum;
}

TUINT8 CCityBase::GetBuildingNumById(SCityInfo *pstCity, TUINT32 udwBuildId)
{
    TUINT8 ucNum = 0;
    for(TUINT32 idx = 0; idx < pstCity->m_stTblData.m_bBuilding.m_udwNum; idx++)
    {
        SCityBuildingNode *pstBuildingNode = &pstCity->m_stTblData.m_bBuilding[idx];
        if(pstBuildingNode->m_ddwLevel == 0)
        {
            continue;
        }
        if(udwBuildId == pstBuildingNode->m_ddwType)
        {
            ucNum++;
        }
    }
    return ucNum;
}

TUINT32 CCityBase::GetBuildingNumByCategory(TbCity *pstCity, TUINT32 udwCategory)
{
    CGameInfo *pobjGameInfo = CGameInfo::GetInstance();
    TUINT32 udwNum = 0;

    for (TUINT32 udwIdx = 0; udwIdx < pstCity->m_bBuilding.m_udwNum; ++udwIdx)
    {
        SCityBuildingNode *pstBuildingNode = &pstCity->m_bBuilding[udwIdx];
        if (pstBuildingNode->m_ddwLevel == 0)
        {
            continue;
        }
        TUINT32 udwBuildId = pstBuildingNode->m_ddwType;
        if (pobjGameInfo->m_oJsonRoot["game_building"].isMember(CCommonFunc::NumToString(udwBuildId)))
        {
            TUINT32 udwBuildCategory = pobjGameInfo->m_oJsonRoot["game_building"][CCommonFunc::NumToString(udwBuildId)]["a"]["a4"].asInt();
            if (udwCategory == udwBuildCategory)
            {
                ++udwNum;
            }
        }
    }

    return udwNum;
}

TBOOL CCityBase::HasBuildCategoryCapacity(SUserInfo *pstUser, TUINT32 udwBuildId)
{
    TbCity *pstCity = &pstUser->m_stCityInfo.m_stTblData;

    CGameInfo *pobjGameInfo = CGameInfo::GetInstance();

    if (pobjGameInfo->m_oJsonRoot["game_building"].isMember(CCommonFunc::NumToString(udwBuildId)))
    {
        TUINT32 udwCategory = pobjGameInfo->m_oJsonRoot["game_building"][CCommonFunc::NumToString(udwBuildId)]["a"]["a4"].asInt();
        TUINT32 udwCategoryNum = GetBuildingNumByCategory(pstCity, udwCategory);
        TUINT32 udwCategoryCapacity = 0;

        switch (udwCategory)
        {
        case EN_BUILDING_CATEGORY__OUT_CITY:
            udwCategoryCapacity = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_OUT_CITY_BLOCK].m_ddwBuffTotal;
            break;
        case EN_BUILDING_CATEGORY__IN_CITY:
            udwCategoryCapacity = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_IN_CITY_BLOCK].m_ddwBuffTotal;
            break;
        case EN_BUILDING_CATEGORY__SPECIAL:
            udwCategoryCapacity = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_SPECIAL_BLOCK].m_ddwBuffTotal;
            break;
        case EN_BUILDING_CATEGORY__DRAGON:
            udwCategoryCapacity = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_DRAGON_CITY_BLOCK].m_ddwBuffTotal;
        default:
            break;
        }
        TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("check build category: buildid[%u], category[%u] category_num[%u] category_cap[%u]",
            udwBuildId, udwCategory, udwCategoryNum, udwCategoryCapacity));
        if (udwCategoryNum < udwCategoryCapacity)
        {
            return TRUE;
        }
    }

    return FALSE;
}

TINT32 CCityBase::GetBuildingFuncType(TUINT32 udwBuildingId)
{
    CGameInfo *pobjGameInfo = CGameInfo::GetInstance();
    return pobjGameInfo->m_oJsonRoot["game_building"][CCommonFunc::NumToString(udwBuildingId)]["a"]["a5"].asInt();
}

TINT64 CCityBase::GetHosTroopNum(SCityInfo *pstCity, TbAlliance_action* patbAction, TUINT8* paucFlag, TUINT32 udwActionNum)
{
    TINT64 ddwNum = 0;
    //hos troop wait
    for (TUINT32 udwWaitTroopIdx = 0; udwWaitTroopIdx < EN_TROOP_TYPE__END; ++udwWaitTroopIdx)
    {
        ddwNum += pstCity->m_stTblData.m_bHos_wait[0].m_addwNum[udwWaitTroopIdx];
    }
    //hos troop treat
    for (TUINT32 udwIdx = 0; udwIdx < udwActionNum; ++udwIdx)
    {
        if (paucFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }
        if (patbAction[udwIdx].m_nMclass != EN_ACTION_MAIN_CLASS__TRAIN_NEW
            || patbAction[udwIdx].m_nSclass != EN_ACTION_SEC_CLASS__HOS_TREAT)
        {
            continue;
        }
        if (!CActionBase::IsPlayerOwnedAction(pstCity->m_stTblData.m_nUid, patbAction[udwIdx].m_nId))
        {
            continue;
        }
        ddwNum += patbAction[udwIdx].m_bParam[0].m_stTrain.m_ddwNum;
    }
    return ddwNum;
}

TINT64 CCityBase::GetHosFortNum(SCityInfo *pstCity, TbAlliance_action* patbAction, TUINT8* paucFlag, TUINT32 udwActionNum)
{
    TINT64 ddwNum = 0;
    for (TUINT32 udwIdx = 0; udwIdx < EN_FORT_TYPE__END; ++udwIdx)
    {
        ddwNum += pstCity->m_stTblData.m_bDead_fort[0][udwIdx];
    }
    for (TUINT32 udwIdx = 0; udwIdx < udwActionNum; ++udwIdx)
    {
        if (paucFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }
        if (patbAction[udwIdx].m_nMclass != EN_ACTION_MAIN_CLASS__TRAIN_NEW
            || patbAction[udwIdx].m_nSclass != EN_ACTION_SEC_CLASS__FORT_REPAIR)
        {
            continue;
        }
        if (!CActionBase::IsPlayerOwnedAction(pstCity->m_stTblData.m_nUid, patbAction[udwIdx].m_nId))
        {
            continue;
        }
        ddwNum += patbAction[udwIdx].m_bParam[0].m_stTrain.m_ddwNum;
    }
    return ddwNum;
}

TBOOL CCityBase::HasChangeCityName(SCityInfo* pstCity)
{
    stringstream oss;
    TCHAR szUIntToString[20];
    oss << CGameInfo::GetInstance()->m_oJsonRoot["game_init_data"]["r"]["a0"].asString().c_str();
    if(pstCity->m_stTblData.m_nUid)
    {
        CCommonFunc::UIntToString(pstCity->m_stTblData.m_nUid, szUIntToString);
        oss << szUIntToString;
    }

    if(pstCity->m_stTblData.m_sName.c_str() != oss.str())
    {
        return TRUE;
    }
    return FALSE;
}

TVOID CCityBase::AddFood(SCityInfo *pstCity, TINT64 ddwNum)
{
    pstCity->m_stTblData.m_bResource.m_astList[0].m_addwNum[EN_RESOURCE_TYPE__FOOD] += ddwNum;   
    pstCity->m_stTblData.m_mFlag[TbCITY_FIELD_RESOURCE] = UPDATE_ACTION_TYPE_PUT;
}


TVOID CCityBase::AddWood(SCityInfo *pstCity, TINT64 ddwNum)
{
    pstCity->m_stTblData.m_bResource.m_astList[0].m_addwNum[EN_RESOURCE_TYPE__WOOD] += ddwNum;
    pstCity->m_stTblData.m_mFlag[TbCITY_FIELD_RESOURCE] = UPDATE_ACTION_TYPE_PUT;
}


TVOID CCityBase::AddStone(SCityInfo *pstCity, TINT64 ddwNum)
{
    pstCity->m_stTblData.m_bResource.m_astList[0].m_addwNum[EN_RESOURCE_TYPE__STONE] += ddwNum;
    pstCity->m_stTblData.m_mFlag[TbCITY_FIELD_RESOURCE] = UPDATE_ACTION_TYPE_PUT;
}


TVOID CCityBase::AddOre(SCityInfo *pstCity, TINT64 ddwNum)
{
    pstCity->m_stTblData.m_bResource.m_astList[0].m_addwNum[EN_RESOURCE_TYPE__ORE] += ddwNum;
    pstCity->m_stTblData.m_mFlag[TbCITY_FIELD_RESOURCE] = UPDATE_ACTION_TYPE_PUT;
}

TVOID CCityBase::AddResource(SCityInfo *pstCity, TUINT32 udwId, TINT64 ddwNum)
{
    pstCity->m_stTblData.m_bResource.m_astList[0].m_addwNum[udwId] += ddwNum;
    pstCity->m_stTblData.m_mFlag[TbCITY_FIELD_RESOURCE] = UPDATE_ACTION_TYPE_PUT;
}

TVOID CCityBase::SetResource(SCityInfo *pstCity, TUINT32 udwId, TINT64 ddwNum)
{
    if (udwId >= EN_RESOURCE_TYPE__GOLD && udwId < EN_RESOURCE_TYPE__END)
    {
        pstCity->m_stTblData.m_bResource.m_astList[0].m_addwNum[udwId] = ddwNum;
        pstCity->m_stTblData.m_mFlag[TbCITY_FIELD_RESOURCE] = UPDATE_ACTION_TYPE_PUT;
    }
}

TVOID CCityBase::CostTroop(SCityInfo *pstCity, TUINT32 udwTroopId, TINT64 ddwNum)
{
    if(pstCity->m_stTblData.m_bTroop[0].m_addwNum[udwTroopId] <= ddwNum)
    {
        pstCity->m_stTblData.m_bTroop[0].m_addwNum[udwTroopId] = 0;
    }
    else
    {
        pstCity->m_stTblData.m_bTroop[0].m_addwNum[udwTroopId] -= ddwNum;
    }
    pstCity->m_stTblData.SetFlag(TbCITY_FIELD_TROOP);
}


TVOID CCityBase::CostFort(SCityInfo *pstCity, TUINT32 udwTroopId, TINT64 ddwNum)
{
    if(pstCity->m_stTblData.m_bFort[0].m_addwNum[udwTroopId] <= ddwNum)
    {
        pstCity->m_stTblData.m_bFort[0].m_addwNum[udwTroopId] = 0;
    }
    else
    {
        pstCity->m_stTblData.m_bFort[0].m_addwNum[udwTroopId] -= ddwNum;
    }
    pstCity->m_stTblData.SetFlag(TbCITY_FIELD_FORT);

}

TBOOL CCityBase::HasEnoughResource(SCityInfo *pstCity, TUINT32 udwId, TINT64 ddwNum)
{
    TBOOL bRet = TRUE;
    if(pstCity->m_stTblData.m_bResource.m_astList[0].m_addwNum[udwId] < ddwNum)
    {
        bRet = FALSE;
    
    }
    return bRet;
}

SCityBuildingNode* CCityBase::GetBuildingAtPos(TbCity* ptbCity, TUINT32 udwPos)
{
    for(TUINT32 udwIdx = 0; udwIdx < ptbCity->m_bBuilding.m_udwNum; ++udwIdx)
    {
        SCityBuildingNode* pstNode = &ptbCity->m_bBuilding[udwIdx];
        if(pstNode->m_ddwPos == udwPos && pstNode->m_ddwLevel > 0)
        {
            return pstNode;
        }
    }
    return NULL;
}

TINT32 CCityBase::DelBuildingAtPos(TbCity* ptbCity, TUINT32 udwPos)
{
    for(TUINT32 udwIdx = 0; udwIdx < ptbCity->m_bBuilding.m_udwNum; ++udwIdx)
    {
        SCityBuildingNode* pstNode = &ptbCity->m_bBuilding[udwIdx];
        if(pstNode->m_ddwPos == udwPos && pstNode->m_ddwLevel > 0)
        {
            pstNode->m_ddwLevel = 0;
            ptbCity->SetFlag(TbCITY_FIELD_BUILDING);
            return 0;
        }
    }
    return -1;
}

TINT32 CCityBase::AddBuilding(TUINT32 udwPos, TUINT8 ucType, TUINT8 ucLevel, TbCity& tbCity)
{
    if(tbCity.m_bBuilding.m_udwNum == MAX_BUILDING_NUM_IN_ONE_CITY)
    {
        return -1;
    }
    tbCity.m_bBuilding[tbCity.m_bBuilding.m_udwNum].m_ddwPos = udwPos;
    tbCity.m_bBuilding[tbCity.m_bBuilding.m_udwNum].m_ddwType = ucType;
    tbCity.m_bBuilding[tbCity.m_bBuilding.m_udwNum].m_ddwLevel = ucLevel;
    tbCity.m_bBuilding.m_udwNum++;
    tbCity.SetFlag(TbCITY_FIELD_BUILDING);
    return 0;
}

TUINT8 CCityBase::GetBuildingNumByLvAndType(SCityInfo *pstCity, TUINT32 udwFuncType,TUINT32 udwLv)
{
    TUINT8 ucNum = 0;
    for(TUINT32 idx = 0; idx < pstCity->m_stTblData.m_bBuilding.m_udwNum; idx++)
    {
        SCityBuildingNode *pstBuildingNode = &pstCity->m_stTblData.m_bBuilding[idx];
        if(pstBuildingNode->m_ddwLevel == 0)
        {
            continue;
        }
        TINT32 dwFuncType = CCityBase::GetBuildingFuncType(pstBuildingNode->m_ddwType);
        if(dwFuncType != static_cast<TINT32>(udwFuncType))
        {
            continue;
        }
        if(udwLv <= pstBuildingNode->m_ddwLevel)
        {
            ucNum++;
        }
    }
    return ucNum;
}

TUINT32 CCityBase::GetBuildingLimitLv(TUINT8 ucType)
{
    return CGameInfo::GetInstance()->m_oJsonRoot["game_building"][CCommonFunc::NumToString(ucType)]["r"]["r0"].size();
}

TINT64 CCityBase::GetHosCurNum(SCityInfo *pstCity)
{
    TINT64 ddwHosNum = 0;
    if(pstCity == NULL)
    {
        ddwHosNum = 0;
    }
    else
    {
        for(TUINT32 udwIdx = 0; udwIdx < EN_TROOP_TYPE__END; ++udwIdx)
        {
            ddwHosNum += pstCity->m_stTblData.m_bHos_wait[0].m_addwNum[udwIdx];
        }
    }
    return ddwHosNum;
}

TVOID CCityBase::ComputeDeadTroopMight( TbMarch_action *ptbMarch, TINT64& ddwDeadMight, TINT64& ddwDeadNum )
{
    SCommonTroop stTroop;
    ddwDeadMight = 0;
    ddwDeadNum = 0;
    for (TUINT32 udwIdx = 0; udwIdx < EN_TROOP_TYPE__END; ++udwIdx)
    {
        stTroop[udwIdx] = ptbMarch->m_bParam[0].m_stTroopRaw[udwIdx]
            - ptbMarch->m_bParam[0].m_stTroop[udwIdx];
        ddwDeadNum += stTroop[udwIdx];
    }
    ddwDeadMight =  CalcTroopMight(stTroop.m_addwNum);
}

TINT32 CCityBase::AddKnightExp( SCityInfo* pstCity, TUINT32 udwTargetKnight, TUINT32 udwAddExp )
{
    TINT32 dwRetCode = 0;
    TUINT32 udwTopLevel = CGameInfo::GetInstance()->GetBasicVal(EN_GAME_BASIC_KNIGHT_TOP_LEVEL);
    TINT64 ddwTopExp = CGameInfo::GetInstance()->GetKnightExpByLevel(udwTopLevel);
    if(udwTargetKnight < pstCity->m_stTblData.m_bKnight.m_udwNum)
    {
        if(pstCity->m_stTblData.m_bKnight[udwTargetKnight].ddwExp >= ddwTopExp)
        {
            dwRetCode = EN_RET_CODE__KNIGHT_EXP_OVERLOAD;
        }
        else
        {
            pstCity->m_stTblData.m_bKnight[udwTargetKnight].ddwExp += udwAddExp;
            if(pstCity->m_stTblData.m_bKnight[udwTargetKnight].ddwExp > ddwTopExp)
            {
                pstCity->m_stTblData.m_bKnight[udwTargetKnight].ddwExp = ddwTopExp;
            }
            pstCity->m_stTblData.SetFlag(TbCITY_FIELD_KNIGHT);
        }
    }
    return dwRetCode;
}
