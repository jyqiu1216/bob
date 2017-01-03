#include "game_info.h"
#include <math.h>
#include "tool_base.h"
#include "quest_logic.h"

CGameInfo* CGameInfo::m_poGameInfo = NULL;

CGameInfo* CGameInfo::GetInstance()
{
    if(m_poGameInfo == NULL)
    {
        m_poGameInfo = new CGameInfo;
    }

    return m_poGameInfo;
}

TINT32 CGameInfo::Init(const TCHAR *pszFileName, CTseLogger *poLog)
{
    if(pszFileName == NULL || poLog == NULL)
    {
        return -1;
    }

    // 0. set param
    m_poLog = poLog;

    // 1. 解析json用Json::Reader
    Json::Reader reader;
    Json::Value tmpRawJson;

    // Json::Value是一种很重要的类型，可以代表任意类型。如int, string, object, array...
    //Json::Value root; 
    std::ifstream is;
    is.open(pszFileName, std::ios::binary);
    if(reader.parse(is, tmpRawJson) == false)
    {
        TSE_LOG_ERROR(m_poLog, ("CGameInfo::Init: parse file[%s] failed.", pszFileName));
        is.close();
        return -2;
    }
    else
    {
        m_oJsonRoot.clear();
        //m_oJsonRoot = Json::Value(Json::objectValue);
        //CToolBase::LoadNewJson(tmpRawJson, m_oJsonRoot);
        m_oJsonRoot = tmpRawJson;
        TSE_LOG_INFO(m_poLog, ("CGameInfo::Init: parse file[%s] success.", pszFileName));
        is.close();
    }
    if (LoadSubGame(SUB_GAME_JSON_FILE, m_poLog) < 0)
    {
        return -3;
    }
    
    InitArmyData();
    LoadAttackOrder(m_objAttackFactor);

    LoadTaskList();

    return 0;
}

TINT32 CGameInfo::Update(const TCHAR *pszFileName, CTseLogger *poLog)
{
    TINT32 dwRetCode = 0;
    CGameInfo *poEventInfo = new CGameInfo;
    CGameInfo *poTmpEventInfo = m_poGameInfo;

    dwRetCode = poEventInfo->Init(pszFileName, poLog);
    if(dwRetCode != 0)
    {
        TSE_LOG_ERROR(poLog, (" CGameInfo::Update failed[%d]", dwRetCode));
        delete poEventInfo;
        return -1;
    }

    m_poGameInfo = poEventInfo;

    if(poTmpEventInfo != NULL)
    {
        sleep(3);
        delete poTmpEventInfo;
    }

    TSE_LOG_INFO(poLog, (" CGameInfo::Update ok", dwRetCode));

    return 0;
}

TINT64 CGameInfo::GetItemTotalNum()
{
    return m_oJsonRoot["game_item"].size();
}

TINT64 CGameInfo::GetTopAlCommentNum()
{
    return m_oJsonRoot["game_basic"][106U].asInt();
}


const vector<TINT64>& CGameInfo::GetTaskList()
{
    return m_vecTaskList;
}

TBOOL CGameInfo::GetItemInfo(TUINT32 udwID, SItemInfo *pstRetInfo)
{
    pstRetInfo->Reset();

    TUINT32 idx = 0;
    TCHAR szKey[32];
    sprintf(szKey, "%u", udwID);
    if (!m_oJsonRoot["game_item"].isMember(szKey))
    {
        return FALSE;
    }

    const Json::Value &stItem = m_oJsonRoot["game_item"][szKey];

    // property
    for(idx = 0; idx < EN_PROP_STATUS__END; idx++)
    {
        sprintf(szKey, "a%u", idx);
        pstRetInfo->m_audwProperty[idx] = stItem[szKey].asUInt();
    }

    return TRUE;
}

TINT64 CGameInfo::GetResearchTotalNum()
{
    return m_oJsonRoot["game_research"].size();
}


TBOOL CGameInfo::GetBuildingInfo(TUINT32 udwType, TUINT32 udwLevel, SBuildingInfo *pstRetInfo)
{
    if(udwLevel == 0 || udwLevel > MAX_BUILDING_LEVEL || pstRetInfo == NULL)
    {
        TSE_LOG_ERROR(m_poLog, ("GetBuildingInfo: input param invalid"));
        return FALSE;
    }
    pstRetInfo->Reset();

    TUINT32 idx = 0;
    // upgrade base cost

    const Json::Value &pBuildingRoot = m_oJsonRoot["game_building"];


    assert(udwType < pBuildingRoot.size());
    TCHAR szType[32];
    snprintf(szType, 32, "%u", udwType);


    pstRetInfo->m_addwBaseCost[EN_BASE_COST_TYPE__TIME] = pBuildingRoot[szType]["r"]["r0"][udwLevel - 1]["a0"].asInt();
    for(idx = 0; idx < pBuildingRoot[szType]["r"]["r0"][udwLevel - 1]["a1"].size(); ++idx)
    {
        if(pBuildingRoot[szType]["r"]["r0"][udwLevel - 1]["a1"][idx][0U].asInt() == EN_GLOBALRES_TYPE_RESOURCE)
        {
            pstRetInfo->m_addwBaseCost[pBuildingRoot[szType]["r"]["r0"][udwLevel - 1]["a1"][idx][1U].asInt()] =
                pBuildingRoot[szType]["r"]["r0"][udwLevel - 1]["a1"][idx][2U].asInt();

            TSE_LOG_DEBUG(m_poLog, ("GetBuildingInfo:idx=%u type=%u lv=%u [resource type=%u num=%u]",
                idx,
                udwType,
                udwLevel,
                pBuildingRoot[szType]["r"]["r0"][udwLevel - 1]["a1"][idx][1U].asInt(),
                pstRetInfo->m_addwBaseCost[pBuildingRoot[szType]["r"]["r0"][udwLevel - 1]["a1"][idx][1U].asInt()]));
        }
        else if(pBuildingRoot[szType]["r"]["r0"][udwLevel - 1]["a1"][idx][0U].asInt() == EN_GLOBALRES_TYPE_ITEM)
        {
            pstRetInfo->m_ucUpradeNeedItem = 1;
            pstRetInfo->m_stSpecialItem.m_udwId = pBuildingRoot[szType]["r"]["r0"][udwLevel - 1]["a1"][idx][1U].asInt();
            pstRetInfo->m_stSpecialItem.m_udwNum = pBuildingRoot[szType]["r"]["r0"][udwLevel - 1]["a1"][idx][2U].asInt();
        }
    }

    return TRUE;
}

TINT64 CGameInfo::GetFortTypeNum()
{
    return m_oJsonRoot["game_fort"].size();
}

TBOOL CGameInfo::GetFortInfo(TUINT32 udwType, STroopInfo *pstRetInfo)
{
    pstRetInfo->Reset();

    if (m_oJsonRoot["game_fort"].size() <= udwType)
    {
        return FALSE;
    }

    const Json::Value &pFortRoot = m_oJsonRoot["game_fort"][udwType];
    if(pFortRoot.isNull())
    {
        return FALSE;
    };

    // property
    pstRetInfo->m_dwId = pFortRoot["a"]["a0"].asInt();
    pstRetInfo->m_fHealth = pFortRoot["a"]["a1"].asDouble();
    pstRetInfo->m_fAttack = pFortRoot["a"]["a2"].asDouble();
    pstRetInfo->m_dwSpeed = pFortRoot["a"]["a3"].asInt();
    pstRetInfo->m_dwLoad = pFortRoot["a"]["a4"].asInt();
    pstRetInfo->m_dwCategory = pFortRoot["a"]["a5"].asInt();
    pstRetInfo->m_dwTier = pFortRoot["a"]["a6"].asInt();
    pstRetInfo->m_bTrainable = pFortRoot["a"]["a7"].asInt();
    pstRetInfo->m_dwUpkeep = pFortRoot["a"]["a8"].asInt();
    pstRetInfo->m_dwMight = pFortRoot["a"]["a9"].asInt();
    pstRetInfo->m_fDefense = pFortRoot["a"]["a17"].asDouble();

    pstRetInfo->m_audwBaseCost[EN_BASE_COST_TYPE__TIME] = pFortRoot["r"]["r0"][0U]["a0"].asUInt();
    for(TUINT32 udwIdx = 0; udwIdx < pFortRoot["r"]["r0"][0U]["a1"].size(); ++udwIdx)
    {
        if(pFortRoot["r"]["r0"][0U]["a1"][udwIdx][0U].asUInt() == EN_GLOBALRES_TYPE_RESOURCE)
        {
            pstRetInfo->m_audwBaseCost[pFortRoot["r"]["r0"][0U]["a1"][udwIdx][1U].asUInt()] = pFortRoot["r"]["r0"][0U]["a1"][udwIdx][2U].asUInt();
        }
        else if(pFortRoot["r"]["r0"][0U]["a1"][udwIdx][0U].asUInt() == EN_GLOBALRES_TYPE_ITEM)
        {
            pstRetInfo->m_astItemCost[pstRetInfo->m_udwRelyNum].m_udwId = pFortRoot["r"]["r0"][0U]["a1"][udwIdx][1U].asUInt();
            pstRetInfo->m_astItemCost[pstRetInfo->m_udwRelyNum].m_udwNum = pFortRoot["r"]["r0"][0U]["a1"][udwIdx][2U].asUInt();
            pstRetInfo->m_udwRelyNum++;
        }
        else if(pFortRoot["r"]["r0"][0U]["a1"][udwIdx][0U].asUInt() == EN_GLOBALRES_TYPE_POPULATION)
        {
            pstRetInfo->m_audwBaseCost[EN_BASE_COST_TYPE__POPULATION] = pFortRoot["r"]["r0"][0U]["a1"][udwIdx][2U].asUInt();
        }
    }
    pstRetInfo->m_bValid = TRUE;

    return TRUE;
}

STroopInfo* CGameInfo::GetFortInfo( TUINT32 udwType )
{
    if(m_aFort[udwType].m_bValid)
    {
        return &m_aFort[udwType];
    }
    return NULL;
}


TINT64 CGameInfo::GetTroopTypeNum()
{
    return m_oJsonRoot["game_troop"].size();
}

TBOOL CGameInfo::GetTroopInfo(TUINT32 udwType, STroopInfo* pstRetInfo)
{
    pstRetInfo->Reset();

    if (m_oJsonRoot["game_troop"].size() <= udwType)
    {
        return FALSE;
    }

    const Json::Value &pTroopRoot = m_oJsonRoot["game_troop"][udwType];
    if(pTroopRoot.isNull())
    {
        return FALSE;
    }

    // property
    pstRetInfo->m_dwId = pTroopRoot["a"]["a0"].asInt();
    pstRetInfo->m_fHealth = pTroopRoot["a"]["a1"].asDouble();
    pstRetInfo->m_fAttack = pTroopRoot["a"]["a2"].asDouble();
    pstRetInfo->m_dwSpeed = pTroopRoot["a"]["a3"].asInt();
    pstRetInfo->m_dwLoad = pTroopRoot["a"]["a4"].asInt();
    pstRetInfo->m_dwCategory = pTroopRoot["a"]["a5"].asInt();
    pstRetInfo->m_dwTier = pTroopRoot["a"]["a6"].asInt();
    pstRetInfo->m_bTrainable = pTroopRoot["a"]["a7"].asInt();
    pstRetInfo->m_dwUpkeep = pTroopRoot["a"]["a8"].asInt();
    pstRetInfo->m_dwMight = pTroopRoot["a"]["a9"].asInt();
    pstRetInfo->m_fDefense = pTroopRoot["a"]["a17"].asDouble();

    pstRetInfo->m_audwBaseCost[EN_BASE_COST_TYPE__TIME] = pTroopRoot["r"]["r0"][0U]["a0"].asUInt();
    for(TUINT32 udwIdx = 0; udwIdx < pTroopRoot["r"]["r0"][0U]["a1"].size(); ++udwIdx)
    {
        if(pTroopRoot["r"]["r0"][0U]["a1"][udwIdx][0U].asUInt() == EN_GLOBALRES_TYPE_RESOURCE)
        {
            pstRetInfo->m_audwBaseCost[pTroopRoot["r"]["r0"][0U]["a1"][udwIdx][1U].asUInt()] = pTroopRoot["r"]["r0"][0U]["a1"][udwIdx][2U].asUInt();
        }
        else if(pTroopRoot["r"]["r0"][0U]["a1"][udwIdx][0U].asUInt() == EN_GLOBALRES_TYPE_ITEM)
        {
            pstRetInfo->m_astItemCost[pstRetInfo->m_udwRelyNum].m_udwId = pTroopRoot["r"]["r0"][0U]["a1"][udwIdx][1U].asUInt();
            pstRetInfo->m_astItemCost[pstRetInfo->m_udwRelyNum].m_udwNum = pTroopRoot["r"]["r0"][0U]["a1"][udwIdx][2U].asUInt();
            pstRetInfo->m_udwRelyNum++;
        }
        else if(pTroopRoot["r"]["r0"][0U]["a1"][udwIdx][0U].asUInt() == EN_GLOBALRES_TYPE_POPULATION)
        {
            pstRetInfo->m_audwBaseCost[EN_BASE_COST_TYPE__POPULATION] = pTroopRoot["r"]["r0"][0U]["a1"][udwIdx][2U].asUInt();

        }

        TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("CProcessPlayer::Train cost:type=%u id=%u num=%u ret_info=%u",
            pTroopRoot["r"]["r0"][0U]["a1"][udwIdx][0U].asUInt(),
            pTroopRoot["r"]["r0"][0U]["a1"][udwIdx][1U].asUInt(),
            pTroopRoot["r"]["r0"][0U]["a1"][udwIdx][2U].asUInt(),
            pstRetInfo->m_audwBaseCost[EN_BASE_COST_TYPE__POPULATION]
            ));
    }

    pstRetInfo->m_bValid = TRUE;

    return TRUE;
}

STroopInfo* CGameInfo::GetTroopInfo( TUINT32 udwType )
{
    if(m_aTroop[udwType].m_bValid)
    {
        return &m_aTroop[udwType];
    }
    return NULL;
}


TBOOL CGameInfo::GetHealTroopInfo(TUINT32 udwType, STroopInfo* pstRetInfo)
{
    pstRetInfo->Reset();

    if (m_oJsonRoot["game_heal_troop"].size() <= udwType)
    {
        return FALSE;
    }

    const Json::Value &pTroopRoot = m_oJsonRoot["game_heal_troop"][udwType];
    if(pTroopRoot.isNull())
    {
        return FALSE;
    }

    // property
    pstRetInfo->m_dwId = pTroopRoot["a"]["a0"].asInt();
    pstRetInfo->m_fHealth = pTroopRoot["a"]["a1"].asDouble();
    pstRetInfo->m_fAttack = pTroopRoot["a"]["a2"].asDouble();
    pstRetInfo->m_dwSpeed = pTroopRoot["a"]["a3"].asInt();
    pstRetInfo->m_dwLoad = pTroopRoot["a"]["a4"].asInt();
    pstRetInfo->m_dwCategory = pTroopRoot["a"]["a5"].asInt();
    pstRetInfo->m_dwTier = pTroopRoot["a"]["a6"].asInt();
    pstRetInfo->m_bTrainable = pTroopRoot["a"]["a7"].asInt();
    pstRetInfo->m_dwUpkeep = pTroopRoot["a"]["a8"].asInt();
    pstRetInfo->m_dwMight = pTroopRoot["a"]["a9"].asInt();
    pstRetInfo->m_fDefense = pTroopRoot["a"]["a17"].asDouble();

    pstRetInfo->m_audwBaseCost[EN_BASE_COST_TYPE__TIME] = pTroopRoot["r"]["r0"][0U]["a0"].asUInt();
    for(TUINT32 udwIdx = 0; udwIdx < pTroopRoot["r"]["r0"][0U]["a1"].size(); ++udwIdx)
    {
        if(pTroopRoot["r"]["r0"][0U]["a1"][udwIdx][0U].asUInt() == EN_GLOBALRES_TYPE_RESOURCE)
        {
            pstRetInfo->m_audwBaseCost[pTroopRoot["r"]["r0"][0U]["a1"][udwIdx][1U].asUInt()] = pTroopRoot["r"]["r0"][0U]["a1"][udwIdx][2U].asUInt();
        }
        else if(pTroopRoot["r"]["r0"][0U]["a1"][udwIdx][0U].asUInt() == EN_GLOBALRES_TYPE_ITEM)
        {
            pstRetInfo->m_astItemCost[pstRetInfo->m_udwRelyNum].m_udwId = pTroopRoot["r"]["r0"][0U]["a1"][udwIdx][1U].asUInt();
            pstRetInfo->m_astItemCost[pstRetInfo->m_udwRelyNum].m_udwNum = pTroopRoot["r"]["r0"][0U]["a1"][udwIdx][2U].asUInt();
            pstRetInfo->m_udwRelyNum++;
        }
        else if(pTroopRoot["r"]["r0"][0U]["a1"][udwIdx][0U].asUInt() == EN_GLOBALRES_TYPE_POPULATION)
        {
            pstRetInfo->m_audwBaseCost[EN_BASE_COST_TYPE__POPULATION] = pTroopRoot["r"]["r0"][0U]["a1"][udwIdx][2U].asUInt();
        }

    }
    pstRetInfo->m_bValid = TRUE;
    return TRUE;
}

STroopInfo* CGameInfo::GetHealTroopInfo( TUINT32 udwType )
{
    if(m_aTroopHeal[udwType].m_bValid)
    {
        return &m_aTroopHeal[udwType];
    }
    return NULL;
}


TBOOL CGameInfo::GetHealFortInfo(TUINT32 udwType, STroopInfo* pstRetInfo)
{
    pstRetInfo->Reset();

    if (m_oJsonRoot["game_heal_fort"].size() <= udwType)
    {
        return FALSE;
    }

    const Json::Value &pTroopRoot = m_oJsonRoot["game_heal_fort"][udwType];
    if(pTroopRoot.isNull())
    {
        return FALSE;
    }

    // property
    pstRetInfo->m_dwId = pTroopRoot["a"]["a0"].asInt();
    pstRetInfo->m_fHealth = pTroopRoot["a"]["a1"].asDouble();
    pstRetInfo->m_fAttack = pTroopRoot["a"]["a2"].asDouble();
    pstRetInfo->m_dwSpeed = pTroopRoot["a"]["a3"].asInt();
    pstRetInfo->m_dwLoad = pTroopRoot["a"]["a4"].asInt();
    pstRetInfo->m_dwCategory = pTroopRoot["a"]["a5"].asInt();
    pstRetInfo->m_dwTier = pTroopRoot["a"]["a6"].asInt();
    pstRetInfo->m_bTrainable = pTroopRoot["a"]["a7"].asInt();
    pstRetInfo->m_dwUpkeep = pTroopRoot["a"]["a8"].asInt();
    pstRetInfo->m_dwMight = pTroopRoot["a"]["a9"].asInt();
    pstRetInfo->m_fDefense = pTroopRoot["a"]["a17"].asDouble();

    pstRetInfo->m_audwBaseCost[EN_BASE_COST_TYPE__TIME] = pTroopRoot["r"]["r0"][0U]["a0"].asUInt();
    for(TUINT32 udwIdx = 0; udwIdx < pTroopRoot["r"]["r0"][0U]["a1"].size(); ++udwIdx)
    {
        if(pTroopRoot["r"]["r0"][0U]["a1"][udwIdx][0U].asUInt() == EN_GLOBALRES_TYPE_RESOURCE)
        {
            pstRetInfo->m_audwBaseCost[pTroopRoot["r"]["r0"][0U]["a1"][udwIdx][1U].asUInt()] = pTroopRoot["r"]["r0"][0U]["a1"][udwIdx][2U].asUInt();
        }
        else if(pTroopRoot["r"]["r0"][0U]["a1"][udwIdx][0U].asUInt() == EN_GLOBALRES_TYPE_ITEM)
        {
            pstRetInfo->m_astItemCost[pstRetInfo->m_udwRelyNum].m_udwId = pTroopRoot["r"]["r0"][0U]["a1"][udwIdx][1U].asUInt();
            pstRetInfo->m_astItemCost[pstRetInfo->m_udwRelyNum].m_udwNum = pTroopRoot["r"]["r0"][0U]["a1"][udwIdx][2U].asUInt();
            pstRetInfo->m_udwRelyNum++;
        }
        else if(pTroopRoot["r"]["r0"][0U]["a1"][udwIdx][0U].asUInt() == EN_GLOBALRES_TYPE_POPULATION)
        {
            pstRetInfo->m_audwBaseCost[EN_BASE_COST_TYPE__POPULATION] = pTroopRoot["r"]["r0"][0U]["a1"][udwIdx][2U].asUInt();
        }

    }
    pstRetInfo->m_bValid = TRUE;
    return TRUE;
}

STroopInfo* CGameInfo::GetHealFortInfo( TUINT32 udwType )
{
    if(m_aFortHeal[udwType].m_bValid)
    {
        return &m_aFortHeal[udwType];
    }
    return NULL;
}


TINT64 CGameInfo::GetKnightExpByLevel( TUINT32 udwLevel )
{
    return m_oJsonRoot["game_knight_exp"][udwLevel][0].asInt64();
}

TINT64 CGameInfo::GetBasicVal( TUINT32 udwKey )
{
    return m_oJsonRoot["game_basic"][udwKey].asInt64();
}

TUINT32 CGameInfo::ComputeKnightLevelByExp( TINT64 ddwExp )
{
    TUINT32 udwLevel = 0;
    for(unsigned int idx = 0; idx < m_oJsonRoot["game_knight_exp"].size(); idx++)
    {
        if(ddwExp >= m_oJsonRoot["game_knight_exp"][idx][0].asInt64())
        {
            udwLevel = idx;
        }
    }
    return udwLevel;
}

TINT32 CGameInfo::InitArmyData()
{
    TINT32 idx = 0;
    TINT32 dwArmyId = 0;

    m_mArmy.clear();
    for(idx = 0; idx <MAX_ARMY_TIER_LIMIT; idx++)
    {
        m_avecTierList[idx].clear();
    }

    for(idx = 0; idx < EN_TROOP_TYPE__END; idx++)
    {
        m_aTroop[idx].Reset();
        GetTroopInfo(idx, &m_aTroop[idx]);

        dwArmyId = CToolBase::GetArmyClsByTroopId(idx);
        m_mArmy.insert(make_pair(dwArmyId, &m_aTroop[idx]));

        if(m_aTroop[idx].m_bValid)
        {
            assert(m_aTroop[idx].m_dwTier < MAX_ARMY_TIER_LIMIT);
            m_avecTierList[m_aTroop[idx].m_dwTier].push_back(dwArmyId);
        }
        
    }

    for(idx = 0; idx < EN_FORT_TYPE__END; idx++)
    {
        m_aFort[idx].Reset();
        GetFortInfo(idx, &m_aFort[idx]);

        dwArmyId = CToolBase::GetArmyClsByFortId(idx);
        m_mArmy.insert(make_pair(dwArmyId, &m_aFort[idx]));

        if(m_aFort[idx].m_bValid)
        {
            assert(m_aFort[idx].m_dwTier < MAX_ARMY_TIER_LIMIT);
            m_avecTierList[m_aFort[idx].m_dwTier].push_back(dwArmyId);
        }
    }

    for(idx = 0; idx < EN_TROOP_TYPE__END; idx++)
    {
        m_aTroopHeal[idx].Reset();
        GetHealTroopInfo(idx, &m_aTroopHeal[idx]);
    }

    for(idx = 0; idx < EN_FORT_TYPE__END; idx++)
    {
        m_aFortHeal[idx].Reset();
        GetHealFortInfo(idx, &m_aFortHeal[idx]);
    }

    return 0;
}

STroopInfo* CGameInfo::GetArmyInfo( TUINT32 udwType )
{
    map<TINT32, STroopInfo*>::iterator it = m_mArmy.find(udwType);
    if(it != m_mArmy.end())
    {
        if(m_mArmy[udwType]->m_bValid)
        {
            return m_mArmy[udwType];
        }
    }
    return NULL;
}


TVOID CGameInfo::LoadAttackOrder( AttackOrders& attack_orders )
{
    // init atk_factor to zero
    for(TUINT32 udwSource = 0; udwSource < EN_ARMS_CLS__END; ++udwSource)
    {
        for(TUINT32 udwTarget = 0; udwTarget < EN_ARMS_CLS__END; ++udwTarget)
        {
            attack_orders.orders[udwSource][udwTarget].atk_factor = 0.0;
        }
    }

    // load troop attack order
    for(TUINT32 udwTroopType = 0; udwTroopType < GetTroopTypeNum(); ++udwTroopType)
    {
        LoadTroopAttackOrder(udwTroopType, attack_orders);
    }

    // load fort attack order
    for(TUINT32 udwFortType = 0; udwFortType < GetFortTypeNum(); ++udwFortType)
    {
        LoadFortAttackOrder(udwFortType, attack_orders);
    }
}

TVOID CGameInfo::LoadTroopAttackOrder( TUINT32 udwSourceTroopId, AttackOrders& attack_orders )
{
    assert(udwSourceTroopId < GetTroopTypeNum());
    const Json::Value &jTroop = m_oJsonRoot["game_troop"][udwSourceTroopId]["a"];
    TUINT32 udwSrcArmyCls = CToolBase::GetArmyClsByTroopId(udwSourceTroopId);
    TUINT32 udwTargetArmyCls = 0;

    //fort
    for(TUINT32 target_fort = 0; target_fort < jTroop["a15"].size(); ++target_fort)
    {
        udwTargetArmyCls = CToolBase::GetArmyClsByFortId(target_fort);
        attack_orders.orders[udwSrcArmyCls][udwTargetArmyCls].atk_factor = jTroop["a15"][target_fort].asDouble();

        TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("LoadTroopAttackOrder[fort]:[%u-%u]->[%u-%u] atk_factor=%f", 
            udwSourceTroopId, udwSrcArmyCls, target_fort, udwTargetArmyCls, attack_orders.orders[udwSrcArmyCls][udwTargetArmyCls].atk_factor));
    }
    //troop
    for(TUINT32 target_troop = 0; target_troop < jTroop["a13"].size(); ++target_troop)
    {
        udwTargetArmyCls = CToolBase::GetArmyClsByTroopId(target_troop);
        attack_orders.orders[udwSrcArmyCls][udwTargetArmyCls].atk_factor = jTroop["a13"][target_troop].asDouble();

        TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("LoadTroopAttackOrder[troop]:[%u-%u]->[%u-%u] atk_factor=%f", 
            udwSourceTroopId, udwSrcArmyCls, target_troop, udwTargetArmyCls, attack_orders.orders[udwSrcArmyCls][udwTargetArmyCls].atk_factor));
    }
}

TVOID CGameInfo::LoadFortAttackOrder( TUINT32 udwSourceFortId, AttackOrders& attack_orders )
{
    assert(udwSourceFortId < GetFortTypeNum());

    const Json::Value &jFort = m_oJsonRoot["game_fort"][udwSourceFortId]["a"];
    TUINT32 udwSrcArmyCls = CToolBase::GetArmyClsByFortId(udwSourceFortId);
    TUINT32 udwTargetArmyCls = 0;

    //若本方有fort参战  对方不会有fort by nemo
    for(TUINT32 target_troop = 0; target_troop < jFort["a13"].size(); ++target_troop)
    {
        udwTargetArmyCls = CToolBase::GetArmyClsByTroopId(target_troop);
        attack_orders.orders[udwSrcArmyCls][udwTargetArmyCls].atk_factor = jFort["a13"][target_troop].asDouble();

        TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("LoadFortAttackOrder[troop]:[%u-%u]->[%u-%u] atk_factor=%f", 
            udwSourceFortId, udwSrcArmyCls, target_troop, udwTargetArmyCls, attack_orders.orders[udwSrcArmyCls][udwTargetArmyCls].atk_factor));
    }
}

TVOID CGameInfo::LoadTaskList()
{
    const Json::Value &rTaskJson = m_oJsonRoot["game_task"];
    Json::Value::Members oTaskId = rTaskJson.getMemberNames();
    vector<STaskPriority> vecTask;

    ostringstream oss;
    oss.str("");
    for (Json::Value::Members::iterator it = oTaskId.begin(); it != oTaskId.end(); ++it)
    {
        if (rTaskJson[(*it).c_str()]["a"]["a7"].asUInt() == 1)
        {
            continue;
        }

        STaskPriority stTaskTmp;
        stTaskTmp.ddwTaskId = atoi((*it).c_str());
        stTaskTmp.dwPriority = rTaskJson[(*it).c_str()]["a"]["a0"].asInt();
        vecTask.push_back(stTaskTmp);
    }

    if (vecTask.size() > 0)
    {
        sort(vecTask.begin(), vecTask.end(), CQuestLogic::CompareTaskPriorityNoState);
        for (TUINT32 udwIdx = 0; udwIdx < vecTask.size(); udwIdx++)
        {
            m_vecTaskList.push_back(vecTask[udwIdx].ddwTaskId);
            oss << vecTask[udwIdx].ddwTaskId << ",";
        }
    }

    //TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("NEMO TASK_LIST: %s",
    //    oss.str().c_str()));
}

TINT32 CGameInfo::LoadSubGame(const TCHAR *pszFileName, CTseLogger *poLog)
{
    if (pszFileName == NULL || poLog == NULL)
    {
        return -1;
    }
    // 1. 解析json用Json::Reader
    Json::Reader reader;
    Json::Value tmpRawJson;

    // Json::Value是一种很重要的类型，可以代表任意类型。如int, string, object, array...
    //Json::Value root; 
    std::ifstream is;
    is.open(pszFileName, std::ios::binary);
    if (!is)
    {
        return 0;
    }
    if (reader.parse(is, tmpRawJson) == false)
    {
        TSE_LOG_ERROR(m_poLog, ("CGameInfo::Init: parse file[%s] failed.", pszFileName));
        is.close();
        return -2;
    }
    else
    {
        //Load sub_game.json
        Json::Value::Members jMembers = tmpRawJson.getMemberNames();
        for (TUINT32 udwIdx = 0; udwIdx < jMembers.size(); udwIdx++)
        {
            TCHAR *pSubGame = jMembers[udwIdx].c_str();
            pSubGame = strchr(pSubGame, '_');
            pSubGame++;
            if (!m_oJsonRoot.isMember(pSubGame))
            {
                m_oJsonRoot[pSubGame] = tmpRawJson[jMembers[udwIdx]];
                continue;
            }
            Json::Value::Members jSubMembs = tmpRawJson[jMembers[udwIdx]].getMemberNames();
            for (TUINT32 udwIdy = 0; udwIdy < jSubMembs.size(); udwIdy++)
            {
                if (m_oJsonRoot[pSubGame].isMember(jSubMembs[udwIdy]))
                {
                    continue;
                }
                m_oJsonRoot[pSubGame][jSubMembs[udwIdy]] = tmpRawJson[jMembers[udwIdx]][jSubMembs[udwIdy]];
            }
        }
        TSE_LOG_INFO(m_poLog, ("CGameInfo::LoadSubGame: parse file[%s] success.", pszFileName));
        is.close();
        //for debug
        //std::ofstream tmp;
        //tmp.open("../tmp.txt");
        //if (!tmp)
        //{
        //    return -3;
        //}
        //string strJson;
        //Json::FastWriter writer;
        //writer.omitEndingLineFeed();
        //strJson = writer.write(m_oJsonRoot);
        //tmp << strJson.c_str() << endl;
        //tmp.close();
    }
    return 0;
}
