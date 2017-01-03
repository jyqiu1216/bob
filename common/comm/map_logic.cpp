#include "map_logic.h"
#include "wild_info.h"
#include "common_func.h"
#include "tool_base.h"
#include "common_base.h"
#include <math.h>
#include "map_base.h"
#include "common_logic.h"
#include "common_logic.h"
#include "city_base.h"
#include "buffer_base.h"
#include "globalres_logic.h"

TBOOL CMapLogic::IsOccupyWild(TINT32 dwSvrId, TINT32 dwWildType)
{
    const Json::Value& oWildResJson = CWildInfo::GetWildResInfo(dwSvrId);
    if (!oWildResJson.isMember(CCommonFunc::NumToString(dwWildType)))
    {
        return FALSE;
    }
    if (oWildResJson[CCommonFunc::NumToString(dwWildType)]["a0"]["a1"].asInt() == 0)
    {
        return TRUE;
    }
    return FALSE;
}

TBOOL CMapLogic::IsAttackWild(TINT32 dwSvrId, TINT32 dwWildType)
{
    const Json::Value& oWildResJson = CWildInfo::GetWildResInfo(dwSvrId);
    if (!oWildResJson.isMember(CCommonFunc::NumToString(dwWildType)))
    {
        return FALSE;
    }
    if (oWildResJson[CCommonFunc::NumToString(dwWildType)]["a0"]["a1"].asInt() == 1)
    {
        return TRUE;
    }
    return FALSE;
}

TUINT32 CMapLogic::GetMapResTotalNum(TbMap *ptbWild)
{
    const Json::Value& oWildResJson = CWildInfo::GetWildResInfo(ptbWild->m_nSid);
    if (!oWildResJson.isMember(CCommonFunc::NumToString(ptbWild->m_nType)))
    {
        return 0;
    }
    TUINT32 udwTotalNum = 0;
    TINT32 dwTmpNum = ptbWild->m_nLevel - 1;
    if (oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a0"]["a2"].asInt() != 0)
    {
        udwTotalNum = 0;
    }
    else
    {
        const Json::Value& oRewardJson = oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a1"][dwTmpNum]["a1"];
        for (TUINT32 udwIdx = 0; udwIdx < oRewardJson.size(); ++udwIdx)
        {
            if (oRewardJson[udwIdx][0U].asInt() == EN_GLOBALRES_TYPE_RESOURCE)
            {
                udwTotalNum += oRewardJson[udwIdx][2U].asUInt();
            }
            else if (oRewardJson[udwIdx][0U].asInt() == EN_GLOBALRES_TYPE_GEM)
            {
                udwTotalNum += oRewardJson[udwIdx][2U].asUInt() * 10000;
            }
        }
        udwTotalNum = 1.0 * udwTotalNum / 10000 * ptbWild->m_nReward_left;
    }
    return udwTotalNum;
}

TINT32 CMapLogic::ComputeLoadResTime(TbMarch_action *ptbMarch, TbMap *ptbWild)
{
    const Json::Value& oWildResJson = CWildInfo::GetWildResInfo(ptbMarch->m_nSid);

    if (!oWildResJson.isMember(CCommonFunc::NumToString(ptbWild->m_nType)))
    {
        return 1; //保护
    }

    if (ptbWild->m_nReward_left == 0)
    {
        ptbWild->Set_Reward_left(10000);
    }

    TINT32 dwResTime = 0;
    TINT32 dwMaxNum = 0;
    TINT32 dwMinNum = 0;
    double dFactor = 0.0;
    TINT32 dwTotalNum = 0;
    TINT32 dwTmpNum = ptbWild->m_nLevel - 1;
    if (oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a4"]["a2"].asInt() == 1)
    {
        for (TUINT32 udwIdx = 0; udwIdx < EN_TROOP_TYPE__END; ++udwIdx)
        {
            dwTotalNum += ptbMarch->m_bParam[0].m_stTroop[udwIdx];
        }
    }
    else if (oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a4"]["a2"].asInt() == 0)
    {
        dwTotalNum = ptbMarch->m_bParam[0].m_stDragon.m_ddwLevel;
    }

    dwResTime = oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a4"]["a0"][dwTmpNum].asInt() * 1.0 / 10000 * ptbWild->m_nReward_left;
    dwMaxNum = oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a3"][2U]["a2"].asInt();
    dwMinNum = oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a3"][2U]["a1"].asInt();
    dFactor = oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a4"]["a1"].asDouble();

    if (dwTotalNum > dwMaxNum)
    {
        dwTotalNum = dwMaxNum;
    }
    if (dwTotalNum < dwMinNum)
    {
        dwTotalNum -= dwMinNum;
    }
    else
    {
        dwTotalNum = 0;
    }
    dFactor *= 1.0 * dwTotalNum / dwMaxNum;

    TUINT64 uddwResBonus = 0;
    if (oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a0"]["a1"].asInt() == 0
        && oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a0"]["a2"].asInt() == 0)
    {
        switch (oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a0"]["a6"].asInt())
        {
        case EN_RESOURCE_TYPE__GOLD:
            uddwResBonus = CActionBase::GetUserBuffById(ptbMarch, EN_BUFFER_INFO_GOLD_COLLECT_SPEED);
            break;
        case EN_RESOURCE_TYPE__FOOD:
            uddwResBonus = CActionBase::GetUserBuffById(ptbMarch, EN_BUFFER_INFO_FOOD_COLLECT_SPEED);
            break;
        case EN_RESOURCE_TYPE__ORE:
            uddwResBonus = CActionBase::GetUserBuffById(ptbMarch, EN_BUFFER_INFO_ORE_COLLECT_SPEED);
            break;
        case EN_RESOURCE_TYPE__STONE:
            uddwResBonus = CActionBase::GetUserBuffById(ptbMarch, EN_BUFFER_INFO_STONE_COLLECT_SPEED);
            break;
        case EN_RESOURCE_TYPE__WOOD:
            uddwResBonus = CActionBase::GetUserBuffById(ptbMarch, EN_BUFFER_INFO_WOOD_COLLECT_SPEED);
            break;
        default:
            break;
        }
    }
    
    uddwResBonus += CActionBase::GetUserBuffById(ptbMarch, EN_BUFFER_INFO_COLLECT_SPEED);

    dwResTime /= (1 + dFactor) * (1.0 * (10000 + uddwResBonus) / 10000);

    return dwResTime < 1 ? 1 :dwResTime;
}

TINT32 CMapLogic::GetWildClass(TINT32 dwSvrId, TINT32 dwWildType)
{
    TUINT32 udwWildClass = EN_WILD_CLASS_NORMAL;
    const Json::Value& oWildResJson = CWildInfo::GetWildResInfo(dwSvrId);
    if(oWildResJson.isMember(CCommonFunc::NumToString(dwWildType)))
    {
        udwWildClass = oWildResJson[CCommonFunc::NumToString(dwWildType)]["a0"]["a0"].asUInt();
    }
    else
    {
        switch(dwWildType)
        {
        case EN_WILD_TYPE__NORMAL:
        case EN_WILD_TYPE__LAKE:
            udwWildClass = EN_WILD_CLASS_NORMAL;
            break;
        case EN_WILD_TYPE__CITY:
            udwWildClass = EN_WILD_CLASS_CITY;
            break;
        case EN_WILD_TYPE__CAMP:
            udwWildClass = EN_WILD_CLASS_CAMP;
            break;
        case EN_WILD_TYPE__THRONE_NEW:
            udwWildClass = EN_WILD_CLASS_THRONE;
            break;
        case EN_WILD_TYPE__IDOL:
            udwWildClass = EN_WILD_CLASS_IDOL;
            break;
        }
    }
    return udwWildClass;

}

TINT32 CMapLogic::GetCollectedReward(SUserInfo *pstUser,TbMarch_action *ptbMarch, TbMap *ptbWild, TINT32 dwBeginTime, TINT32 dwLoadTime, TINT64 &ddwLoadNum)
{
    SActionMarchParam *pstParam = &ptbMarch->m_bParam[0];
    const Json::Value& oWildResJson = CWildInfo::GetWildResInfo(ptbWild->m_nSid);
    TUINT32 udwCurtime = CTimeUtils::GetUnixTime();

    if (!oWildResJson.isMember(CCommonFunc::NumToString(ptbWild->m_nType)))
    {
        return 0;
    }
    TINT32 dwTmpNum = ptbWild->m_nLevel - 1;
    const Json::Value& oRewardJson = oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a1"][dwTmpNum]["a1"];
    if (ptbWild->m_nReward_left == 0)
    {
        ptbWild->Set_Reward_left(10000);
    }

    if(dwLoadTime == 0)
    {
        return 0;
    }
    TINT32 dwHasLoadRes = ceil((ptbMarch->m_nEtime - pstParam->m_ddwBeginLoadTime) * 1.0 / dwLoadTime * ptbWild->m_nReward_left);
    TSE_LOG_INFO(CGameInfo::GetInstance()->m_poLog, ("GetCollectedReward: has_load_res[%d] etime[%ld] begin_load_time[%ld] load_time[%d] reward[%ld]",
        dwHasLoadRes, ptbMarch->m_nEtime, pstParam->m_ddwBeginLoadTime, dwLoadTime, ptbWild->m_nReward_left));

    if (dwHasLoadRes > ptbWild->m_nReward_left)
    {
        dwHasLoadRes = ptbWild->m_nReward_left;
    }
    if (dwHasLoadRes < 0)
    {
        dwHasLoadRes = 0;
    }
    TSE_LOG_INFO(CGameInfo::GetInstance()->m_poLog, ("GetCollectedReward: has_load_res[%d]", dwHasLoadRes));

    TUINT32 udwClassType = CMapLogic::GetWildClass(ptbWild->m_nSid,ptbWild->m_nType);
    TINT32 udwLoad = CCommonBase::GetTroopTotalLoad(ptbMarch, dwBeginTime);

    TSE_LOG_INFO(CGameInfo::GetInstance()->m_poLog, ("GetCollectedReward: [uid=%u action_id=%lld map_id=%u troop_load_num=%u rewar_left=%u load_percent=%u",
        ptbMarch->m_nSuid, ptbMarch->m_nId, ptbWild->m_nId, udwLoad, ptbWild->m_nReward_left, dwHasLoadRes));

    if (oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a0"]["a2"].asInt() == 0)
    {
        for (TUINT32 udwIdx = 0; udwIdx < oRewardJson.size(); ++udwIdx)
        {
            if (oRewardJson[udwIdx][0U].asInt() == EN_GLOBALRES_TYPE_RESOURCE)
            {
                TINT64 ddwCollectNum = oRewardJson[udwIdx][2U].asInt64() * dwHasLoadRes / 10000;
                if (static_cast<TINT32>(ddwCollectNum) > udwLoad)
                {
                    ddwCollectNum = udwLoad;
                }
                udwLoad -= ddwCollectNum;
                pstParam->m_stResource[oRewardJson[udwIdx][1U].asUInt()] += ddwCollectNum;
                ddwLoadNum = pstParam->m_stResource[oRewardJson[udwIdx][1U].asUInt()];
            }
            else if (oRewardJson[udwIdx][0U].asInt() == EN_GLOBALRES_TYPE_GEM)
            {
                TINT64 ddwNum = oRewardJson[udwIdx][2U].asInt64();
                TINT32 dwCollectedNum = ddwNum * ptbWild->m_nReward_left / 10000
                    - ddwNum * (ptbWild->m_nReward_left - dwHasLoadRes) / 10000;
                if (dwCollectedNum > udwLoad / 10000)
                {
                    dwCollectedNum = udwLoad / 10000;
                }
                udwLoad -= dwCollectedNum * 10000;
                pstParam->m_ddwLoadGem += dwCollectedNum;
                ddwLoadNum = pstParam->m_stResource[oRewardJson[udwIdx][1U].asUInt()];
            }
        }
        ptbMarch->SetFlag(TbMARCH_ACTION_FIELD_PARAM);

        //针对资源地 阶段掉落机制
        // GenStageResult(ptbWild, ptbMarch, dwLoadTime);

        ptbWild->Set_Reward_left(ptbWild->m_nReward_left - dwHasLoadRes);
//         if (ptbWild->m_nReward_left == 0)
//         {
//             for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
//             {
//                 TbMarch_action tbAction = pstUser->m_atbMarch[udwIdx];
//                 if (tbAction.m_nMclass == EN_ACTION_MAIN_CLASS__MARCH && tbAction.m_nSclass == EN_ACTION_SEC_CLASS__OCCUPY &&
//                     tbAction.m_nTpos == ptbWild->m_nId)
//                 {
//                     pstUser->m_atbMarch[udwIdx].Set_Etime(udwCurtime);
//                     pstUser->m_atbMarch[udwIdx].Set_Status(EN_MARCH_STATUS__RETURNING);
//                 }
//             }
//         }
    }
//     else
//     {
//         if (ptbWild->m_nReward_left - dwHasLoadRes == 0)
//         {
//             GenWildItemReward(ptbWild, ptbMarch);
//         }
//         ptbWild->Set_Reward_left(ptbWild->m_nReward_left - dwHasLoadRes);
//     }

//     if(udwClassType != EN_WILD_CLASS_RES && ptbWild->m_nReward_left == 0)
//     {
//         GenSurpriseResult(ptbWild, ptbMarch);
//     }
    return 0;
}

TBOOL CMapLogic::HaveCollectedOut(SUserInfo *pstUser, TbMarch_action *ptbMainMarch, TbMap *ptbWild)
{
    TbMarch_action *ptbMarch = NULL;
    TUINT32 udwLoadRes = 0;
    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; udwIdx++)
    {
        ptbMarch = &pstUser->m_atbMarch[udwIdx];
        if (ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__OCCUPY
            && ptbMarch->m_nStatus == EN_MARCH_STATUS__LOADING
            && ptbMarch->m_nTpos == ptbWild->m_nId)
        {
            udwLoadRes += ceil((ptbMarch->m_nEtime - ptbMarch->m_bParam[0].m_ddwBeginLoadTime) * 1.0 / ptbMarch->m_bParam[0].m_ddwTotalLoadTime * ptbWild->m_nReward_left);
            if (ptbMarch->m_nEtime > ptbMainMarch->m_nEtime)
            {
                return FALSE;
            }
        }
    }

    if (udwLoadRes >= ptbWild->m_nReward_left)
    {
        return TRUE;
    }

    return FALSE;
}

TINT32 CMapLogic::GetIdxById(TUINT32 udwId, TbMap *patbMap,TUINT32 udwNum)
{
    for(TUINT32 udwIdx = 0; udwIdx < udwNum;++udwIdx)
    {
        if(patbMap[udwIdx].m_nId == udwId)
        {
            return udwIdx;
        }
    }
    return -1;
}

TBOOL CMapLogic::IsPosCanMove(TUINT32 udwPos, TbMap *patbMap, TUINT32 udwNum,TUINT32 udwTypeBlockNum,TUINT32 udwSid)
{
    /*
    //udwTypeBlockNum 是改地形的地块数 定为 NXN
    TUINT32 udwXpos = udwPos / MAP_X_Y_POS_COMPUTE_OFFSET;
    TUINT32 udwYpos = udwPos % MAP_X_Y_POS_COMPUTE_OFFSET;
    TUINT32 udwSize = udwTypeBlockNum;
    assert(udwSize != 0);
    const Json::Value &oWildJson = CWildInfo::GetWildResInfo(udwSid);

    std::set<TINT32> posSet;
    posSet.clear();

    TUINT32 udwCenterPos = CCommonLogic::BuildingPointToPos(udwXpos, udwYpos);
    CCommonLogic::GetBuildingPos(udwCenterPos, udwSize, posSet);

    for(set<TINT32>::iterator it = posSet.begin(); it != posSet.end(); ++it)
    {
        BuildingPoint stBuildingPoing = CCommonLogic::BuildingPosToPoint(*it);

        TUINT32 udwNewPos = stBuildingPoing.x * MAP_X_Y_POS_COMPUTE_OFFSET + stBuildingPoing.y;

        TINT32 dwDataIdx = CMapLogic::GetIdxById(udwNewPos, patbMap, udwNum);
        if(dwDataIdx == -1)
        {
            //数据不足
            TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("IsPosCanMove: [pos=%u side pos info not in tmp_map]",udwNewPos));
            return FALSE;
        }
        
        if(patbMap[dwDataIdx].m_nUid != 0 || 
            patbMap[dwDataIdx].m_nRtype == EN_WILD_TYPE__LAKE ||
            patbMap[dwDataIdx].m_nType == EN_WILD_TYPE__CITY )
        {
            TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("IsPosCanMove: [pos=%u uid=%u type=%u smoe condition not right]", udwNewPos, patbMap[dwDataIdx].m_nUid, patbMap[dwDataIdx].m_nType));
            return FALSE;
        }
        if(oWildJson.isMember(CCommonFunc::NumToString(patbMap[dwDataIdx].m_nType)))
        {
            std::set<TINT32> sProvinceSet;
            CMapLogic::GetProvincePoints(sProvinceSet);
            std::set<TINT32> sThronSet;
            CMapLogic::GetThronePoints(sThronSet);
            TUINT32 udwWildClass = CMapLogic::GetWildClass(patbMap[dwDataIdx].m_nSid, patbMap[dwDataIdx].m_nType);
            if ((EN_WILD_CLASS_HERO_MOSTER == udwWildClass || EN_WILD_CLASS_LEADER_MONSTER == udwWildClass) &&
                patbMap[dwDataIdx].m_nWild_gen_time != 0 && 
                (patbMap[dwDataIdx].m_nWild_gen_time + 3600 * 3 > CTimeUtils::GetUnixTime()))
            {
                TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("IsPosCanMove: [pos=%u type=%u end_time=%u smoe condition not right]",
                    udwNewPos, patbMap[dwDataIdx].m_nType, patbMap[dwDataIdx].m_nWild_gen_time + 3600 * 3));
                return FALSE;
            }
            if(EN_WILD_CLASS_HERO_COLLECT == udwWildClass)
            {
                TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("IsPosCanMove: [pos=%u type=%u  smoe condition not right]",
                    udwNewPos,  patbMap[dwDataIdx].m_nType));
                return FALSE;
            }
            if(udwWildClass == EN_WILD_CLASS_PROVINCE && sProvinceSet.find(CCommonLogic::BuildingPointToPos(udwPos / MAP_X_Y_POS_COMPUTE_OFFSET, udwPos % MAP_X_Y_POS_COMPUTE_OFFSET)) == sProvinceSet.end())
            {
                continue;
            }
            if(udwWildClass == EN_WILD_CLASS_THRONE && sThronSet.find(CCommonLogic::BuildingPointToPos(udwPos / MAP_X_Y_POS_COMPUTE_OFFSET, udwPos % MAP_X_Y_POS_COMPUTE_OFFSET)) == sThronSet.end())
            {
                continue;
            }

            TUINT32 udwTmpBlockNum = CMapBase::GetWildBlockNumByType(udwSid, patbMap[dwDataIdx].m_nType);
            if(udwTmpBlockNum != 1)
            {
                TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("IsPosCanMove: [pos=%u type=%u block_num=%u smoe condition not right]", 
                    udwNewPos, patbMap[dwDataIdx].m_nType, udwTmpBlockNum));
                return FALSE;
            }
        }
    }
    return TRUE;
    */

    return FALSE;
}

TBOOL CMapLogic::IsPosCanMove( TbMap *ptbMap )
{
    /*
    if(ptbMap->m_nUid != 0 || 
        ptbMap->m_nRtype == EN_WILD_TYPE__LAKE)
    {
        return FALSE;
    }

    TUINT32 udwWildClass = GetWildClass(ptbMap->m_nSid, ptbMap->m_nType);
    if(udwWildClass >= EN_WILD_CLASS_HERO_MOSTER)
    {
        if((EN_WILD_CLASS_HERO_MOSTER == udwWildClass || EN_WILD_CLASS_LEADER_MONSTER == udwWildClass) && 
            (ptbMap->m_nWild_gen_time + MAX_MONSTER_SHOW_INTERVAL_TIME < CTimeUtils::GetUnixTime()))
        {
            return TRUE;
        }
        else if (EN_WILD_CLASS_HERO_COLLECT == udwWildClass 
            && ptbMap->m_nExpire_time < CTimeUtils::GetUnixTime())
        {
            return TRUE;
        }

        return FALSE;
    }

    return TRUE;
    */

    return FALSE;
}


TBOOL CMapLogic::IsDragonAttackWild(TINT32 dwSvrId, TINT32 dwWildType)
{
    const Json::Value& oWildResJson = CWildInfo::GetWildResInfo(dwSvrId);
    if(!oWildResJson.isMember(CCommonFunc::NumToString(dwWildType)))
    {
        return FALSE;
    }
    if(oWildResJson[CCommonFunc::NumToString(dwWildType)]["a0"]["a1"].asInt() == 1
        && (oWildResJson[CCommonFunc::NumToString(dwWildType)]["a0"]["a0"].asInt() == EN_WILD_CLASS_MONSTER
        || oWildResJson[CCommonFunc::NumToString(dwWildType)]["a0"]["a0"].asInt() == EN_WILD_CLASS_LEADER_MONSTER))
    {
        return TRUE;
    }
    return FALSE;
}

TBOOL CMapLogic::IsDragonOccupyWild(TINT32 dwSvrId, TINT32 dwWildType)
{
    const Json::Value& oWildResJson = CWildInfo::GetWildResInfo(dwSvrId);
    if(!oWildResJson.isMember(CCommonFunc::NumToString(dwWildType)))
    {
        return FALSE;
    }
    if(oWildResJson[CCommonFunc::NumToString(dwWildType)]["a0"]["a1"].asInt() == 0
        && oWildResJson[CCommonFunc::NumToString(dwWildType)]["a0"]["a0"].asInt() == EN_WILD_CLASS_MONSTER_NEST)
    {
        return TRUE;
    }
    return FALSE;
}

TINT32 CMapLogic::GetResTypeByWildType(TUINT32 udwSid,TINT32 dwWildType,TUINT32 udwLv)
{
    const Json::Value& oWildResJson = CWildInfo::GetWildResInfo(udwSid);

    if(!oWildResJson.isMember(CCommonFunc::NumToString(dwWildType)))
    {
        return -1;
    }
    TINT32 dwTmpNum = udwLv > 0 ? udwLv - 1 : 0;
    if(oWildResJson[CCommonFunc::NumToString(dwWildType)]["a0"]["a2"].asInt() != 0)
    {
        return -1;
    }

    const Json::Value& oRewardJson = oWildResJson[CCommonFunc::NumToString(dwWildType)]["a1"][dwTmpNum]["a1"];
    for(TUINT32 udwIdx = 0; udwIdx < oRewardJson.size(); ++udwIdx)
    {
        if(oRewardJson[udwIdx][0U].asInt() == EN_GLOBALRES_TYPE_RESOURCE)
        {
            return oRewardJson[udwIdx][1U].asInt();
        }
    }
    return -1;
}

TBOOL CMapLogic::IfPlayerCity(SUserInfo *pstUser, TbMap *ptbWild)
{
    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwWildNum; ++udwIdx)
    {
        if (pstUser->m_atbWild[udwIdx].m_nId == ptbWild->m_nId &&
            pstUser->m_tbPlayer.m_nUid == ptbWild->m_nUid &&
            ptbWild->m_nType == EN_WILD_TYPE__CITY)
        {
            return TRUE;
        }
    }
    return FALSE;
}

TBOOL CMapLogic::IsWildNeedToDelete(TbMap *ptbWild)
{
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    if (udwCurTime < ptbWild->m_nShowtime)
    {
        return TRUE;
    }

    TUINT32 udwWildClass = CMapLogic::GetWildClass(ptbWild->m_nSid, ptbWild->m_nType);
    if (udwWildClass == EN_WILD_CLASS_NORMAL)
    {
        return TRUE;
    }

    if ((udwWildClass == EN_WILD_CLASS_MONSTER || udwWildClass == EN_WILD_CLASS_LEADER_MONSTER) &&
        ptbWild->m_nExpire_time < udwCurTime)
    {
        return TRUE;
    }

    if (udwWildClass == EN_WILD_CLASS_MONSTER_NEST
        && ptbWild->m_nExpire_time < udwCurTime)
    {
        return TRUE;
    }

    return FALSE;
}
