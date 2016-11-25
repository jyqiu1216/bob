#include "war_base.h"

#include <cmath>
#include <cfloat>
#include "common_base.h"
#include "game_info.h"
#include "common_func.h"
#include "wild_info.h"
#include "msg_base.h"
#include "sendmessage_base.h"
#include "action_base.h"
#include "time_utils.h"
#include "map_logic.h"
#include "player_base.h"
#include "map_base.h"
#include "tool_base.h"
#include "globalres_logic.h"

TVOID CWarBase::AddReinforceArmy(TbMarch_action* ptbMarch, SBattleNode* pstNode)
{
    OneArmy reinforce_army;
    reinforce_army.army_id = ptbMarch->m_nId;
    reinforce_army.raw_troop = ptbMarch->m_bParam[0].m_stTroop;
    reinforce_army.left_troop = ptbMarch->m_bParam[0].m_stTroop;
    reinforce_army.knight = ptbMarch->m_bParam[0].m_stKnight;
    pstNode->m_armys.actors.insert(std::make_pair(reinforce_army.army_id, reinforce_army));
    pstNode->m_pastEncampActionList[pstNode->m_udwEncampNum++] = ptbMarch;
}

TVOID CWarBase::AddAttackArmy(TbMarch_action* ptbMarch, SBattleNode* pstNode)
{
    OneArmy attack_leader;
    attack_leader.army_id = ptbMarch->m_nId;
    attack_leader.raw_troop = ptbMarch->m_bParam[0].m_stTroop;
    attack_leader.left_troop = ptbMarch->m_bParam[0].m_stTroop;
    attack_leader.knight = ptbMarch->m_bParam[0].m_stKnight;
    pstNode->m_armys.actors.insert(std::make_pair(attack_leader.army_id, attack_leader));
}

TVOID CWarBase::AddCityArmy(SCityInfo* pstCity, SBattleNode* pstNode)
{
    OneArmy city_army;
    city_army.army_id = pstCity->m_stTblData.m_nUid;
    city_army.raw_troop = pstCity->m_stTblData.m_bTroop[0];
    city_army.left_troop = pstCity->m_stTblData.m_bTroop[0];
    city_army.raw_fort = pstCity->m_stTblData.m_bFort[0];
    city_army.left_fort = pstCity->m_stTblData.m_bFort[0];
    city_army.is_city = TRUE;
    for (TUINT32 udwIdx = 0; udwIdx < pstCity->m_stTblData.m_bKnight.m_udwNum; udwIdx++)
    {
        if (pstCity->m_stTblData.m_bKnight[udwIdx].ddwPos == EN_KNIGHT_POS__TROOP)
        {
            city_army.knight.ddwId = udwIdx;
            city_army.knight.ddwLevel = CGameInfo::GetInstance()->ComputeKnightLevelByExp(pstCity->m_stTblData.m_bKnight[udwIdx].ddwExp);
            city_army.knight.ddwExpAdd = 0;

            pstNode->m_stKnight = city_army.knight;
            break;
        }
    }
    pstNode->m_armys.actors.insert(std::make_pair(city_army.army_id, city_army));
}

TVOID CWarBase::AddRallyReinforceArmy(TbMarch_action* ptbMarch, SBattleNode* pstNode)
{
    OneArmy reinforce_army;
    reinforce_army.army_id = ptbMarch->m_nId;
    reinforce_army.raw_troop = ptbMarch->m_bParam[0].m_stTroop;
    reinforce_army.left_troop = ptbMarch->m_bParam[0].m_stTroop;
    reinforce_army.knight = ptbMarch->m_bParam[0].m_stKnight;
    pstNode->m_armys.actors.insert(std::make_pair(reinforce_army.army_id, reinforce_army));
    pstNode->m_pastRallyReinforceList[pstNode->m_udwRallyReinforceNum++] = ptbMarch;
}

TVOID CWarBase::AddWildArmy(TbMap* ptbWild, SBattleNode* pstNode)
{
    OneArmy wild_army;
    wild_army.army_id = 0;
    wild_army.raw_troop = ptbWild->m_bTroop[0];
    wild_army.left_troop = ptbWild->m_bTroop[0];
    pstNode->m_armys.actors.insert(std::make_pair(wild_army.army_id, wild_army));
}

TVOID CWarBase::AddWildOccupyArmy(TbMarch_action* ptbMarch, SBattleNode* pstNode)
{
    OneArmy wild_occupyer;
    wild_occupyer.army_id = ptbMarch->m_nId;
    wild_occupyer.raw_troop = ptbMarch->m_bParam[0].m_stTroop;
    wild_occupyer.left_troop = ptbMarch->m_bParam[0].m_stTroop;
    wild_occupyer.knight = ptbMarch->m_bParam[0].m_stKnight;
    pstNode->m_armys.actors.insert(std::make_pair(wild_occupyer.army_id, wild_occupyer));
    pstNode->m_pastEncampActionList[pstNode->m_udwEncampNum++] = ptbMarch;
}

TVOID CWarBase::AddWildCampArmy(TbMarch_action* ptbMarch, SBattleNode* pstNode)
{
    OneArmy wild_camp;
    wild_camp.army_id = ptbMarch->m_nId;
    wild_camp.raw_troop = ptbMarch->m_bParam[0].m_stTroop;
    wild_camp.left_troop = ptbMarch->m_bParam[0].m_stTroop;
    wild_camp.knight = ptbMarch->m_bParam[0].m_stKnight;
    pstNode->m_armys.actors.insert(std::make_pair(wild_camp.army_id, wild_camp));
    pstNode->m_pastEncampActionList[pstNode->m_udwEncampNum++] = ptbMarch;
}

TVOID CWarBase::AddIdolArmy(TbIdol *ptbIdol, SBattleNode* pstNode)
{
    OneArmy idol_army;
    idol_army.army_id = 0;
    idol_army.raw_troop = ptbIdol->m_bTroop[0];
    idol_army.left_troop = ptbIdol->m_bTroop[0];
    pstNode->m_armys.actors.insert(std::make_pair(idol_army.army_id, idol_army));
}

TVOID CWarBase::SetBattleBuff(SPlayerBuffInfo *pstBuff, SBattleNode *pstNode, TBOOL bIsCityDefender, TBOOL bIsRallyAttacker)
{
    pstNode->m_stBattleBuff = *pstBuff;

    CGameInfo *poGameInfo = CGameInfo::GetInstance();
    const Json::Value &jTroop = poGameInfo->m_oJsonRoot["game_troop"];
    for (TUINT32 udwIdx = 0; udwIdx < jTroop.size(); ++udwIdx)
    {
        pstNode->m_armys.troop_buffs[udwIdx].Reset();

        //attack buff
        for(TUINT32 udwAtkBuffIdx = 0; udwAtkBuffIdx < jTroop[udwIdx]["ab"]["attack"].size();++udwAtkBuffIdx)
        {
            TUINT32 udwAtkBuffId = jTroop[udwIdx]["ab"]["attack"][udwAtkBuffIdx].asUInt();
            pstNode->m_armys.troop_buffs[udwIdx].atk += pstBuff->m_astPlayerBuffInfo[udwAtkBuffId].m_ddwBuffTotal;
        }
        //hp buff
        for(TUINT32 udwHpBuffIdx = 0; udwHpBuffIdx < jTroop[udwIdx]["ab"]["life"].size(); ++udwHpBuffIdx)
        {
            TUINT32 udwHpBuffId = jTroop[udwIdx]["ab"]["life"][udwHpBuffIdx].asUInt();
            pstNode->m_armys.troop_buffs[udwIdx].hp += pstBuff->m_astPlayerBuffInfo[udwHpBuffId].m_ddwBuffTotal;
        }
        //defense buff
        for(TUINT32 udwDefenseBuffIdx = 0; udwDefenseBuffIdx < jTroop[udwIdx]["ab"]["defense"].size(); ++udwDefenseBuffIdx)
        {
            TUINT32 udwDefenseBuffId = jTroop[udwIdx]["ab"]["defense"][udwDefenseBuffIdx].asUInt();
            pstNode->m_armys.troop_buffs[udwIdx].def += pstBuff->m_astPlayerBuffInfo[udwDefenseBuffId].m_ddwBuffTotal;
        }
    }

    // 场景没法配置
    if (bIsRallyAttacker) //rally war attack时所有军队加成 包括本身
    {
        for (TUINT32 udwIdx = 0; udwIdx < jTroop.size(); ++udwIdx)
        {
            pstNode->m_armys.troop_buffs[udwIdx].atk += pstBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_RALLY_ATTACK].m_ddwBuffTotal;
            pstNode->m_armys.troop_buffs[udwIdx].hp += pstBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_RALLY_LIFE].m_ddwBuffTotal;
            pstNode->m_armys.troop_buffs[udwIdx].def += pstBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_RALLY_DEFENSE].m_ddwBuffTotal;
        }
    }

    const Json::Value &jFort = poGameInfo->m_oJsonRoot["game_fort"];
    for (TUINT32 udwIdx = 0; udwIdx < jFort.size(); ++udwIdx)
    {
        //attack buff
        for(TUINT32 udwAtkBuffIdx = 0; udwAtkBuffIdx < jFort[udwIdx]["ab"]["attack"].size(); ++udwAtkBuffIdx)
        {
            TUINT32 udwAtkBuffId = jFort[udwIdx]["ab"]["attack"][udwAtkBuffIdx].asUInt();
            pstNode->m_armys.fort_buffs[udwIdx].atk += pstBuff->m_astPlayerBuffInfo[udwAtkBuffId].m_ddwBuffTotal;
        }
        //hp buff
        for(TUINT32 udwHpBuffIdx = 0; udwHpBuffIdx < jFort[udwIdx]["ab"]["life"].size(); ++udwHpBuffIdx)
        {
            TUINT32 udwHpBuffId = jFort[udwIdx]["ab"]["life"][udwHpBuffIdx].asUInt();
            pstNode->m_armys.fort_buffs[udwIdx].hp += pstBuff->m_astPlayerBuffInfo[udwHpBuffId].m_ddwBuffTotal;
        }
        //defense buff
        for(TUINT32 udwDefenseBuffIdx = 0; udwDefenseBuffIdx < jFort[udwIdx]["ab"]["defense"].size(); ++udwDefenseBuffIdx)
        {
            TUINT32 udwDefenseBuffId = jFort[udwIdx]["ab"]["defense"][udwDefenseBuffIdx].asUInt();
            pstNode->m_armys.fort_buffs[udwIdx].def += pstBuff->m_astPlayerBuffInfo[udwDefenseBuffId].m_ddwBuffTotal;
        }
    }
}
//
//TVOID CWarBase::PrepareDefendBattleUnits(SBattleNode* pstDefender)
//{
//    CGameInfo *pobjGameInfo = CGameInfo::GetInstance();
//    TUINT32 udwTroopTypeSize = pobjGameInfo->GetTroopTypeNum();
//    TUINT32 udwFortTypeSize = pobjGameInfo->GetFortTypeNum();
//
//    // 计算军队数量
//    for(std::map<ArmyId, OneArmy>::iterator it = pstDefender->m_armys.actors.begin(); it != pstDefender->m_armys.actors.end(); ++it)
//    {
//        SCommonTroop& stAttendTroop = it->second.raw_troop;
//        for(TUINT32 udwTroopType = 0; udwTroopType < udwTroopTypeSize; ++udwTroopType)
//        {
//            if(stAttendTroop[udwTroopType] == 0)
//            {
//                continue;
//            }
//
//            TUINT32 udwArmyClass = CToolBase::GetArmyClsByTroopId(udwTroopType);            
//            // army class
//            pstDefender->m_armys.units[udwArmyClass].unit_class = (EArmsClass)udwArmyClass;
//            // num
//            pstDefender->m_armys.units[udwArmyClass].raw_num += stAttendTroop[udwTroopType];
//        }
//
//        SCommonFort& stAttendFort = it->second.raw_fort;
//        for(TUINT32 udwFortType = 0; udwFortType < udwFortTypeSize; ++udwFortType)
//        {
//            if(stAttendFort[udwFortType] == 0)
//            {
//                continue;
//            }
//
//            TUINT32 udwArmyClass = CToolBase::GetArmyClsByFortId(udwFortType);
//            // army class
//            pstDefender->m_armys.units[udwArmyClass].unit_class = (EArmsClass)udwArmyClass;
//            // num
//            pstDefender->m_armys.units[udwArmyClass].raw_num += stAttendFort[udwFortType];
//        }
//    }
//
//    // 计算troop相关类型的army数据
//    for(TUINT32 udwArmyClass = 0; udwArmyClass < EN_TROOP_CLS__END; udwArmyClass++)
//    {
//        if(pstDefender->m_armys.units[udwArmyClass].raw_num == 0)
//        {
//            continue;
//        }
//
//        STroopInfo *pstArmyInfo = pobjGameInfo->GetArmyInfo(udwArmyClass);
//        TUINT32 udwTroopType = CToolBase::GetTroopIdByArmyCls(udwArmyClass);
//        TINT64 ddwNum = pstDefender->m_armys.units[udwArmyClass].raw_num;
//        TINT64 ddwTotalVal  = 0;
//
//        //force
//        pstDefender->m_armys.units[udwArmyClass].single_force = pstArmyInfo->m_dwMight;
//        pstDefender->m_armys.units[udwArmyClass].single_hp = pstArmyInfo->m_dwHealth * ((10000 + pstDefender->m_armys.troop_buffs[udwTroopType].hp) * 1.0 / 10000) * ARMY_DATA_EXPAND_RATIO;
//        pstDefender->m_armys.units[udwArmyClass].single_atk = pstArmyInfo->m_dwAttack * ((10000 + pstDefender->m_armys.troop_buffs[udwTroopType].atk) * 1.0 / 10000) * ARMY_DATA_EXPAND_RATIO;
//        pstDefender->m_armys.units[udwArmyClass].single_def = pstArmyInfo->m_dwDefense * ((10000 + pstDefender->m_armys.troop_buffs[udwTroopType].def) * 1.0 / 10000) * ARMY_DATA_EXPAND_RATIO;
//
//        //left num
//        pstDefender->m_armys.units[udwArmyClass].left_num = ddwNum;
//
//        //health
//        ddwTotalVal = ddwNum * pstDefender->m_armys.units[udwArmyClass].single_hp;        
//        pstDefender->m_armys.units[udwArmyClass].raw_hp = ddwTotalVal;
//        pstDefender->m_armys.units[udwArmyClass].hp = ddwTotalVal;
//        
//        //defense
//        ddwTotalVal = ddwNum * pstDefender->m_armys.units[udwArmyClass].single_def;
//        pstDefender->m_armys.units[udwArmyClass].def = ddwTotalVal;
//    }
//
//    // 计算fort相关类型的army数据
//    for(TUINT32 udwArmyClass = EN_ARMS_CLS__TRAPS; udwArmyClass < EN_FORT_CLS__END; udwArmyClass++)
//    {
//        if(pstDefender->m_armys.units[udwArmyClass].raw_num == 0)
//        {
//            continue;
//        }
//
//        STroopInfo *pstArmyInfo = pobjGameInfo->GetArmyInfo(udwArmyClass);
//        TUINT32 udwTroopType = CToolBase::GetFortIdByArmyCls(udwArmyClass);
//        TINT64 ddwNum = pstDefender->m_armys.units[udwArmyClass].raw_num;
//        TINT64 ddwTotalVal  = 0;
//
//        //single
//        pstDefender->m_armys.units[udwArmyClass].single_force = pstArmyInfo->m_dwMight;
//        pstDefender->m_armys.units[udwArmyClass].single_hp = pstArmyInfo->m_dwHealth * ((10000 + pstDefender->m_armys.fort_buffs[udwTroopType].hp) * 1.0 / 10000) * ARMY_DATA_EXPAND_RATIO;
//        pstDefender->m_armys.units[udwArmyClass].single_atk = pstArmyInfo->m_dwAttack * ((10000 + pstDefender->m_armys.fort_buffs[udwTroopType].atk) * 1.0 / 10000) * ARMY_DATA_EXPAND_RATIO;
//        pstDefender->m_armys.units[udwArmyClass].single_def = pstArmyInfo->m_dwDefense * ((10000 + pstDefender->m_armys.fort_buffs[udwTroopType].def) * 1.0 / 10000) * ARMY_DATA_EXPAND_RATIO;
//
//        //left num
//        pstDefender->m_armys.units[udwArmyClass].left_num = ddwNum;
//
//        //health
//        ddwTotalVal = ddwNum * pstDefender->m_armys.units[udwArmyClass].single_hp;        
//        pstDefender->m_armys.units[udwArmyClass].raw_hp = ddwTotalVal;
//        pstDefender->m_armys.units[udwArmyClass].hp = ddwTotalVal;
//
//        //defense
//        ddwTotalVal = ddwNum * pstDefender->m_armys.units[udwArmyClass].single_def;
//        pstDefender->m_armys.units[udwArmyClass].def = ddwTotalVal;
//
//        //attack
//        ddwTotalVal = ddwNum * pstDefender->m_armys.units[udwArmyClass].single_atk;        
//        pstDefender->m_armys.units[udwArmyClass].raw_atk = ddwTotalVal;
//        pstDefender->m_armys.units[udwArmyClass].atk = ddwTotalVal;
//    }
//}

//
//TVOID CWarBase::PrepareDefendBattleUnits(SBattleNode* pstDefender)
//{
//    Json::Value &jTroop = CGameInfo::GetInstance()->m_oJsonRoot["game_troop"];
//    Json::Value &jFort = CGameInfo::GetInstance()->m_oJsonRoot["game_fort"];
//    for(std::map<ArmyId, OneArmy>::iterator it = pstDefender->m_armys.actors.begin(); it != pstDefender->m_armys.actors.end(); ++it)
//    {
//        SCommonTroop& stAttendTroop = it->second.raw_troop;
//        for(TUINT32 udwTroopType = 0; udwTroopType < jTroop.size(); ++udwTroopType)
//        {
//            if(stAttendTroop[udwTroopType] == 0)
//            {
//                continue;
//            }
//
//            TUINT32 udwArmyClass = CToolBase::GetArmyClsByTroopId(udwTroopType);
//            // army class
//            pstDefender->m_armys.units_a[udwArmyClass].unit_class = (EArmsClass)udwArmyClass;
//            // num
//            pstDefender->m_armys.units_a[udwArmyClass].raw_num += stAttendTroop[udwTroopType];
//            pstDefender->m_armys.units_a[udwArmyClass].left_num += stAttendTroop[udwTroopType];
//            pstDefender->m_armys.raw_troop_num += stAttendTroop[udwTroopType];
//            it->second.part_a_troop[udwTroopType] += stAttendTroop[udwTroopType];
//            // force
//            TUINT32 udwSingleForce = jTroop[udwTroopType]["a"]["a9"].asUInt();
//            pstDefender->m_armys.units_a[udwArmyClass].single_force = udwSingleForce;
//            // health
//            double dSingleVal = jTroop[udwTroopType]["a"]["a1"].asDouble() * ((10000 + pstDefender->m_armys.troop_buffs[udwTroopType].hp) * 1.0 / 10000) * ARMY_DATA_EXPAND_RATIO;
//            pstDefender->m_armys.units_a[udwArmyClass].raw_hp += stAttendTroop[udwTroopType] * dSingleVal;
//            pstDefender->m_armys.units_a[udwArmyClass].hp += stAttendTroop[udwTroopType] * dSingleVal;
//            // attack
//            dSingleVal = jTroop[udwTroopType]["a"]["a2"].asDouble() * ((10000 + pstDefender->m_armys.troop_buffs[udwTroopType].atk) * 1.0 / 10000) * ARMY_DATA_EXPAND_RATIO;
//            pstDefender->m_armys.units_a[udwArmyClass].raw_atk += stAttendTroop[udwTroopType] * dSingleVal;
//            pstDefender->m_armys.units_a[udwArmyClass].atk += stAttendTroop[udwTroopType] * dSingleVal;
//            pstDefender->m_armys.raw_total_atk += stAttendTroop[udwTroopType] * dSingleVal;
//            // defend
//            dSingleVal = jTroop[udwTroopType]["a"]["a17"].asDouble() * ((10000 + pstDefender->m_armys.troop_buffs[udwTroopType].def) * 1.0 / 10000) * ARMY_DATA_EXPAND_RATIO;
//            pstDefender->m_armys.units_a[udwArmyClass].def += stAttendTroop[udwTroopType] * dSingleVal;
//            pstDefender->m_armys.raw_total_def += stAttendTroop[udwTroopType] * dSingleVal;
//        }
//
//        SCommonFort& stAttendFort = it->second.raw_fort;
//        for(TUINT32 udwFortType = 0; udwFortType < jFort.size(); ++udwFortType)
//        {
//            if(stAttendFort[udwFortType] == 0)
//            {
//                continue;
//            }
//
//            TUINT32 udwArmyClass = CToolBase::GetArmyClsByFortId(udwFortType);
//            // army class
//            pstDefender->m_armys.units_b[udwArmyClass].unit_class = (EArmsClass)udwArmyClass;
//            // num
//            pstDefender->m_armys.units_b[udwArmyClass].raw_num += stAttendFort[udwFortType];
//            pstDefender->m_armys.units_b[udwArmyClass].left_num += stAttendFort[udwFortType];
//            pstDefender->m_armys.raw_fort_num += stAttendFort[udwFortType];
//            it->second.part_b_fort[udwFortType] += stAttendFort[udwFortType];
//            // force
//            TUINT32 udwSingleForce = jFort[udwFortType]["a"]["a9"].asUInt();
//            pstDefender->m_armys.units_b[udwArmyClass].single_force = udwSingleForce;
//            // health
//            double dSingleVal = jFort[udwFortType]["a"]["a1"].asDouble() * ((10000 + pstDefender->m_armys.fort_buffs[udwFortType].hp) * 1.0 / 10000) * ARMY_DATA_EXPAND_RATIO;
//            pstDefender->m_armys.units_b[udwArmyClass].raw_hp += stAttendFort[udwFortType] * dSingleVal;
//            pstDefender->m_armys.units_b[udwArmyClass].hp += stAttendFort[udwFortType] * dSingleVal;
//            // attack
//            dSingleVal = jFort[udwFortType]["a"]["a2"].asDouble() * ((10000 + pstDefender->m_armys.fort_buffs[udwFortType].atk) * 1.0 / 10000) * ARMY_DATA_EXPAND_RATIO;
//            pstDefender->m_armys.units_b[udwArmyClass].raw_atk += stAttendFort[udwFortType] * dSingleVal;
//            pstDefender->m_armys.units_b[udwArmyClass].atk += stAttendFort[udwFortType] * dSingleVal;
//            pstDefender->m_armys.raw_total_atk += stAttendFort[udwFortType] * dSingleVal;
//            // defend
//            dSingleVal = jFort[udwFortType]["a"]["a17"].asDouble() * ((10000 + pstDefender->m_armys.fort_buffs[udwFortType].def) * 1.0 / 10000) * ARMY_DATA_EXPAND_RATIO;
//            pstDefender->m_armys.units_b[udwArmyClass].def += stAttendFort[udwFortType] * dSingleVal;
//            pstDefender->m_armys.raw_total_def += stAttendFort[udwFortType] * dSingleVal;
//        }
//    }
//}
//
//TVOID CWarBase::PrepareAttackBattleUnits(const SBattleNode* pstDefender, SBattleNode* pstAttacker)
//{
//    double part_b_rate = 0.0f;
//    if(pstDefender->m_armys.raw_fort_num > 0)
//    {
//        part_b_rate = (double)pstDefender->m_armys.raw_fort_num / ((double)pstDefender->m_armys.raw_fort_num + (double)pstDefender->m_armys.raw_troop_num);
//    }
//
//    TINT32 dwSiegeNum = 0;
//    TINT32 dwNotSiegeNum = 0;
//    const Json::Value &jTroop = CGameInfo::GetInstance()->m_oJsonRoot["game_troop"];
//
//    for(std::map<ArmyId, OneArmy>::iterator it = pstAttacker->m_armys.actors.begin(); it != pstAttacker->m_armys.actors.end(); ++it)
//    {
//        SCommonTroop& stAttendTroop = it->second.raw_troop;
//        for(TUINT32 udwTroopType = 0; udwTroopType < jTroop.size(); ++udwTroopType)
//        {
//            if(stAttendTroop[udwTroopType] == 0)
//            {
//                continue;
//            }
//
//            if(CToolBase::GetTroopCategoryByTroopType(udwTroopType) != EN_TROOP_CATEGORY__SIEGE)
//            {
//                dwNotSiegeNum += stAttendTroop[udwTroopType];
//                continue;
//            }
//
//            TUINT32 udwArmyClass = CToolBase::GetArmyClsByTroopId(udwTroopType);
//            // army class
//            pstAttacker->m_armys.units_b[udwArmyClass].unit_class = (EArmsClass)udwArmyClass;
//            // num
//            pstAttacker->m_armys.units_b[udwArmyClass].raw_num += stAttendTroop[udwTroopType];
//            pstAttacker->m_armys.units_b[udwArmyClass].left_num += stAttendTroop[udwTroopType];
//            pstAttacker->m_armys.raw_troop_num += stAttendTroop[udwTroopType];
//            dwSiegeNum += stAttendTroop[udwTroopType];
//            it->second.part_b_troop[udwTroopType] += stAttendTroop[udwTroopType];
//            // force
//            TUINT32 udwSingleForce = jTroop[udwTroopType]["a"]["a9"].asUInt();
//            pstAttacker->m_armys.units_b[udwArmyClass].single_force = udwSingleForce;
//            // health
//            double dSingleVal = jTroop[udwTroopType]["a"]["a1"].asDouble() * ((10000 + pstAttacker->m_armys.troop_buffs[udwTroopType].hp) * 1.0 / 10000) * ARMY_DATA_EXPAND_RATIO;
//            pstAttacker->m_armys.units_b[udwArmyClass].raw_hp += stAttendTroop[udwTroopType] * dSingleVal;
//            pstAttacker->m_armys.units_b[udwArmyClass].hp += stAttendTroop[udwTroopType] * dSingleVal;
//            // attack
//            dSingleVal = jTroop[udwTroopType]["a"]["a2"].asDouble() * ((10000 + pstAttacker->m_armys.troop_buffs[udwTroopType].atk) * 1.0 / 10000) * ARMY_DATA_EXPAND_RATIO;
//            pstAttacker->m_armys.units_b[udwArmyClass].raw_atk += stAttendTroop[udwTroopType] * dSingleVal;
//            pstAttacker->m_armys.units_b[udwArmyClass].atk += stAttendTroop[udwTroopType] * dSingleVal;
//            pstAttacker->m_armys.raw_total_atk += stAttendTroop[udwTroopType] * dSingleVal;
//            // defend
//            dSingleVal = jTroop[udwTroopType]["a"]["a17"].asDouble() * ((10000 + pstAttacker->m_armys.troop_buffs[udwTroopType].def) * 1.0 / 10000) * ARMY_DATA_EXPAND_RATIO;
//            pstAttacker->m_armys.units_b[udwArmyClass].def += stAttendTroop[udwTroopType] * dSingleVal;
//            pstAttacker->m_armys.raw_total_def += stAttendTroop[udwTroopType] * dSingleVal;
//        }
//    }
//
//    TINT32 part_b_num = (dwSiegeNum + dwNotSiegeNum) * part_b_rate;
//    part_b_num = (part_b_num > dwSiegeNum) ? (part_b_num - dwSiegeNum) : 0;
//    part_b_rate = (double)part_b_num / (double)dwNotSiegeNum;
//
//    for(std::map<ArmyId, OneArmy>::iterator it = pstAttacker->m_armys.actors.begin(); it != pstAttacker->m_armys.actors.end(); ++it)
//    {
//        SCommonTroop& stAttendTroop = it->second.raw_troop;
//        for(TUINT32 udwTroopType = 0; udwTroopType < jTroop.size(); ++udwTroopType)
//        {
//            if(stAttendTroop[udwTroopType] == 0)
//            {
//                continue;
//            }
//
//            if(CToolBase::GetTroopCategoryByTroopType(udwTroopType) == EN_TROOP_CATEGORY__SIEGE)
//            {
//                continue;
//            }
//
//            TUINT32 udwToPartBNum = stAttendTroop[udwTroopType] * part_b_rate;
//            TUINT32 udwToPartANum = stAttendTroop[udwTroopType] - udwToPartBNum;
//
//            TUINT32 udwArmyClass = CToolBase::GetArmyClsByTroopId(udwTroopType);
//            // army class
//            pstAttacker->m_armys.units_a[udwArmyClass].unit_class = (EArmsClass)udwArmyClass;
//            pstAttacker->m_armys.units_b[udwArmyClass].unit_class = (EArmsClass)udwArmyClass;
//            // num
//            pstAttacker->m_armys.units_a[udwArmyClass].raw_num += udwToPartANum;
//            pstAttacker->m_armys.units_a[udwArmyClass].left_num += udwToPartANum;
//            pstAttacker->m_armys.raw_troop_num += udwToPartANum;
//            it->second.part_a_troop[udwTroopType] += udwToPartANum;
//            pstAttacker->m_armys.units_b[udwArmyClass].raw_num += udwToPartBNum;
//            pstAttacker->m_armys.units_b[udwArmyClass].left_num += udwToPartBNum;
//            pstAttacker->m_armys.raw_troop_num += udwToPartBNum;
//            it->second.part_b_troop[udwTroopType] += udwToPartBNum;
//            // force
//            TUINT32 udwSingleForce = jTroop[udwTroopType]["a"]["a9"].asUInt();
//            pstAttacker->m_armys.units_a[udwArmyClass].single_force = udwSingleForce;
//            pstAttacker->m_armys.units_b[udwArmyClass].single_force = udwSingleForce;
//            // health
//            double dSingleVal = jTroop[udwTroopType]["a"]["a1"].asDouble() * ((10000 + pstAttacker->m_armys.troop_buffs[udwTroopType].hp) * 1.0 / 10000) * ARMY_DATA_EXPAND_RATIO;
//            pstAttacker->m_armys.units_a[udwArmyClass].raw_hp += udwToPartANum * dSingleVal;
//            pstAttacker->m_armys.units_a[udwArmyClass].hp += udwToPartANum * dSingleVal;
//            pstAttacker->m_armys.units_b[udwArmyClass].raw_hp += udwToPartBNum * dSingleVal;
//            pstAttacker->m_armys.units_b[udwArmyClass].hp += udwToPartBNum * dSingleVal;
//            // attack
//            dSingleVal = jTroop[udwTroopType]["a"]["a2"].asDouble() * ((10000 + pstAttacker->m_armys.troop_buffs[udwTroopType].atk) * 1.0 / 10000) * ARMY_DATA_EXPAND_RATIO;
//            pstAttacker->m_armys.units_a[udwArmyClass].raw_atk += udwToPartANum * dSingleVal;
//            pstAttacker->m_armys.units_a[udwArmyClass].atk += udwToPartANum * dSingleVal;
//            pstAttacker->m_armys.raw_total_atk += udwToPartANum * dSingleVal;
//            pstAttacker->m_armys.units_b[udwArmyClass].raw_atk += udwToPartBNum * dSingleVal;
//            pstAttacker->m_armys.units_b[udwArmyClass].atk += udwToPartBNum * dSingleVal;
//            pstAttacker->m_armys.raw_total_atk += udwToPartBNum * dSingleVal;
//            // defend
//            dSingleVal = jTroop[udwTroopType]["a"]["a17"].asDouble() * ((10000 + pstAttacker->m_armys.troop_buffs[udwTroopType].def) * 1.0 / 10000) * ARMY_DATA_EXPAND_RATIO;
//            pstAttacker->m_armys.units_a[udwArmyClass].def += udwToPartANum * dSingleVal;
//            pstAttacker->m_armys.raw_total_def += udwToPartANum* dSingleVal;
//            pstAttacker->m_armys.units_b[udwArmyClass].def += udwToPartBNum * dSingleVal;
//            pstAttacker->m_armys.raw_total_def += udwToPartBNum* dSingleVal;
//        }
//    }
//}


//TVOID CWarBase::LoadAttackOrder(AttackOrders& attack_orders)
//{
//    CGameInfo *pobjGameInfo = CGameInfo::GetInstance();
//
//    // init atk_factor to zero
//    for(TUINT32 udwSource = 0; udwSource < EN_ARMS_CLS__END; ++udwSource)
//    {
//        for(TUINT32 udwTarget = 0; udwTarget < EN_ARMS_CLS__END; ++udwTarget)
//        {
//            attack_orders.orders[udwSource][udwTarget].atk_factor = 0.0f;
//        }
//    }
//
//    // load troop attack order
//    for(TUINT32 udwTroopType = 0; udwTroopType < pobjGameInfo->GetTroopTypeNum(); ++udwTroopType)
//    {
//        LoadTroopAttackOrder(udwTroopType, attack_orders);
//    }
//
//    // load fort attack order
//    for(TUINT32 udwFortType = 0; udwFortType < pobjGameInfo->GetFortTypeNum(); ++udwFortType)
//    {
//        LoadFortAttackOrder(udwFortType, attack_orders);
//    }
//}
//
//TVOID CWarBase::LoadTroopAttackOrder(TUINT32 udwSourceTroopId, AttackOrders& attack_orders)
//{
//    assert(udwSourceTroopId < CGameInfo::GetInstance()->GetTroopTypeNum());
//    Json::Value jTroop = CGameInfo::GetInstance()->m_oJsonRoot["game_troop"][udwSourceTroopId]["a"];
//    TUINT32 udwSrcArmyCls = CToolBase::GetArmyClsByTroopId(udwSourceTroopId);
//    TUINT32 udwTargetArmyCls = 0;
//    TUINT32 order = 0;
//    ostringstream oss1, oss2;
//    oss1 << "troop[" << udwSourceTroopId << ":" << udwSrcArmyCls << "][";
//    oss2 << "troop[" << udwSourceTroopId << ":" << udwSrcArmyCls << "][";
//    //fort
//    for(TUINT32 target_fort = 0; target_fort < jTroop["a14"].size(); ++target_fort)
//    {
//        udwTargetArmyCls = CToolBase::GetArmyClsByFortId(jTroop["a14"][target_fort].asUInt());
//        attack_orders.orders[udwSrcArmyCls][order].target = (EArmsClass)udwTargetArmyCls;
//        attack_orders.orders[udwSrcArmyCls][order].atk_factor = jTroop["a15"][target_fort].asDouble();
//        ++order;
//    }
//    //troop
//    for(TUINT32 target_troop = 0; target_troop < jTroop["a12"].size(); ++target_troop)
//    {
//        udwTargetArmyCls = CToolBase::GetArmyClsByTroopId(jTroop["a12"][target_troop].asUInt());
//        attack_orders.orders[udwSrcArmyCls][order].target = (EArmsClass)udwTargetArmyCls;
//        attack_orders.orders[udwSrcArmyCls][order].atk_factor = jTroop["a13"][target_troop].asDouble();
//        ++order;
//    }
//
//    for(TUINT32 udwIdx = 0; udwIdx < order; ++udwIdx)
//    {
//        oss1 << attack_orders.orders[udwSrcArmyCls][order].target << ",";
//        oss2 << attack_orders.orders[udwSrcArmyCls][order].atk_factor << ",";
//    }
//    oss1 << "]";
//    oss2 << "]";
//    //TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("SetTroopAttackPro:%s", oss1.str().c_str()));
//    //TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("SetTroopAttackPro:%s", oss2.str().c_str()));
//}
//
//TVOID CWarBase::LoadFortAttackOrder(TUINT32 udwSourceFortId, AttackOrders& attack_orders)
//{
//    assert(udwSourceFortId < CGameInfo::GetInstance()->GetFortTypeNum());
//
//    Json::Value jFort = CGameInfo::GetInstance()->m_oJsonRoot["game_fort"][udwSourceFortId]["a"];
//    TUINT32 udwSrcArmyCls = CToolBase::GetArmyClsByFortId(udwSourceFortId);
//    TUINT32 udwTargetArmyCls = 0;
//    TUINT32 order = 0;
//    ostringstream oss1, oss2;
//    oss1 << "fort[" << udwSourceFortId << ":" << udwSrcArmyCls << "][";
//    oss2 << "fort[" << udwSourceFortId << ":" << udwSrcArmyCls << "][";
//
//    //若本方有fort参战  对方不会有fort by nemo
//    //troop
//    for(TUINT32 target_troop = 0; target_troop < jFort["a12"].size(); ++target_troop)
//    {
//        udwTargetArmyCls = CToolBase::GetArmyClsByTroopId(jFort["a12"][target_troop].asUInt());
//        attack_orders.orders[udwSrcArmyCls][order].target = (EArmsClass)udwTargetArmyCls;
//        attack_orders.orders[udwSrcArmyCls][order].atk_factor = jFort["a13"][target_troop].asDouble();
//        ++order;
//    }
//
//    for(TUINT32 udwIdx = 0; udwIdx < order; ++udwIdx)
//    {
//        oss1 << attack_orders.orders[udwSrcArmyCls][order].target << ",";
//        oss2 << attack_orders.orders[udwSrcArmyCls][order].atk_factor << ",";;
//    }
//    oss1 << "]";
//    oss2 << "]";
//    //TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("SetTroopAttackPro:%s", oss1.str().c_str()));
//    //TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("SetTroopAttackPro:%s", oss2.str().c_str()));
//}

//TVOID CWarBase::UnitsFightUnits(const AttackOrders& attack_orders, BattleUnit* atk_units, BattleUnit* def_units)
//{
//    for(TUINT32 atk_step = 0; atk_step < EN_ARMS_CLS__END; ++atk_step)
//    {
//        for(TUINT32 army_class = 0; army_class < EN_ARMS_CLS__END; ++army_class)
//        {
//            if(0 == atk_units[army_class].atk)
//            {
//                continue;
//            }
//
//            EArmsClass target_army_class = attack_orders.orders[army_class][atk_step].target;
//            if(std::fabs(attack_orders.orders[army_class][atk_step].atk_factor - 0.0f) < DBL_EPSILON)
//            {
//                continue;
//            }
//            if(0 == def_units[target_army_class].hp)
//            {
//                continue;
//            }
//
//            TINT64 final_atk = atk_units[army_class].atk * attack_orders.orders[army_class][atk_step].atk_factor;
//
//            TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("CWarBase::UnitsFightUnits:[src=%u][tar=%u][def=%ld][atk=%ld][atk_with_factor=%ld][factor=%f]",
//                army_class,
//                target_army_class,
//                def_units[target_army_class].def,
//                atk_units[army_class].atk,
//                final_atk,
//                attack_orders.orders[army_class][atk_step].atk_factor));
//
//            if(final_atk > def_units[target_army_class].def)
//            {
//                final_atk = final_atk - def_units[target_army_class].def;
//                def_units[target_army_class].def = 0;
//            }
//            else
//            {
//                def_units[target_army_class].def -= final_atk;
//                final_atk = 0;
//            }
//
//            if(final_atk <= 0)
//            {
//                continue;
//            }
//
//            TINT64 origin_hp = def_units[target_army_class].hp;
//            if(final_atk <= origin_hp)
//            {
//                atk_units[army_class].atk = 0;
//                def_units[target_army_class].hp -= final_atk;
//            }
//            else
//            {
//                atk_units[army_class].atk -= origin_hp / attack_orders.orders[army_class][atk_step].atk_factor + 1;
//                def_units[target_army_class].hp = 0;
//            }
//
//            TINT64 now_hp = def_units[target_army_class].hp;
//            TINT64 lose_hp = origin_hp - now_hp;
//            double dead_ratio = (double)lose_hp / (double)def_units[target_army_class].raw_hp;
//            TINT64 dead_num = def_units[target_army_class].raw_num * dead_ratio;
//            dead_num = (dead_num > def_units[target_army_class].left_num) ? def_units[target_army_class].left_num : dead_num;
//
//            def_units[target_army_class].left_num -= dead_num;
//            if(now_hp == 0 && def_units[target_army_class].left_num > 0)
//            {
//                dead_num += def_units[target_army_class].left_num;
//                def_units[target_army_class].left_num = 0;
//            }
//
//            if(CToolBase::IsArmyClsTroop(target_army_class))
//            {
//                atk_units[army_class].kill_troop_num += dead_num;
//                atk_units[army_class].kill_troop_force += dead_num * def_units[target_army_class].single_force;
//            }
//            else if(CToolBase::IsArmyClsFort(target_army_class))
//            {
//                atk_units[army_class].kill_fort_num += dead_num;
//                atk_units[army_class].kill_fort_force += dead_num * def_units[target_army_class].single_force;
//            }
//        }
//    }
//}


TVOID CWarBase::UnitsFightUnits( SBattleNode* pstAttacker, SBattleNode* pstDefender )
{
    CGameInfo* poGameInfo = CGameInfo::GetInstance();
    TUINT32 udwArmyClass = 0;
    TINT64 ddwRealAttack = 0;
    TINT64 ddwCurAttack = 0;
    TINT64 ddwLostHealth = 0;
    TINT64 ddwDeadNum = 0;
    TINT32 dwArmyCount = 0;

    if(pstAttacker->m_armys.raw_total_atk > pstDefender->m_armys.raw_total_def)
    {
        ddwRealAttack = pstAttacker->m_armys.raw_total_atk - pstDefender->m_armys.raw_total_def + pstDefender->m_armys.raw_total_def*0.08;
    }
    else
    {
        ddwRealAttack = pstAttacker->m_armys.raw_total_atk*0.08;
    }

    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("[wavetest]CWarBase::UnitsFightUnits[begin]: src_atk=%ld tar_def=%d src_real_atk=%ld [seq=%u]",
        pstAttacker->m_armys.raw_total_atk, pstDefender->m_armys.raw_total_def, ddwRealAttack, pstAttacker->m_pstUser->m_udwBSeqNo));

    for(TINT32 idx = 0; idx < MAX_ARMY_TIER_LIMIT; idx++)
    {
        if(pstDefender->m_armys.tier_health[idx] == 0)
        {
            continue;
        }

        dwArmyCount = poGameInfo->m_avecTierList[idx].size();
        if(ddwRealAttack >= pstDefender->m_armys.tier_health[idx])
        {
            TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("[wavetest]CWarBase::UnitsFightUnits[tier_death]:target_tier=%d----left_atk=%ld tier_lif=%ld, all_death  [seq=%u]",
                idx, ddwRealAttack, pstDefender->m_armys.tier_health[idx], pstAttacker->m_pstUser->m_udwBSeqNo));

            ddwRealAttack -= pstDefender->m_armys.tier_health[idx];

            for(TINT32 idy = 0; idy < dwArmyCount; idy++)
            {
                udwArmyClass = poGameInfo->m_avecTierList[idx][idy];

                if(CToolBase::IsArmyClsTroop(udwArmyClass))
                {
                    pstAttacker->m_armys.kill_troop_num += pstDefender->m_armys.units[udwArmyClass].left_num;
                    pstAttacker->m_armys.kill_troop_force += pstDefender->m_armys.units[udwArmyClass].left_num * pstDefender->m_armys.units[udwArmyClass].single_force;
                }
                else
                {
                    pstAttacker->m_armys.kill_fort_num += pstDefender->m_armys.units[udwArmyClass].left_num;
                    pstAttacker->m_armys.kill_fort_force += pstDefender->m_armys.units[udwArmyClass].left_num * pstDefender->m_armys.units[udwArmyClass].single_force;
                }                

                pstDefender->m_armys.units[udwArmyClass].left_num = 0;
                pstDefender->m_armys.units[udwArmyClass].hp = 0;
            }
        }
        else
        {
            TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("[wavetest]CWarBase::UnitsFightUnits[normal]:target_tier=%d----left_atk=%ld tier_lif=%ld  [seq=%u]",
                idx, ddwRealAttack, pstDefender->m_armys.tier_health[idx], pstAttacker->m_pstUser->m_udwBSeqNo));

            TINT64 ddwRawAttack = ddwRealAttack;
            //TINT64 ddwLeftAttack = 0;
            TINT32  dwCount = 0;
            TINT32  dwFailedNum = 0;
            while(ddwRealAttack)
            {
                dwCount++;
                dwFailedNum = 0;
                ddwRawAttack = ddwRealAttack;

                for(TINT32 idy = 0; idy < dwArmyCount; idy++)
                {
                    udwArmyClass = poGameInfo->m_avecTierList[idx][idy];
                    if(pstDefender->m_armys.units[udwArmyClass].left_num == 0)
                    {
                        dwFailedNum++;
                        continue;
                    }
    
                    ddwCurAttack = ddwRawAttack * pstDefender->m_armys.units[udwArmyClass].raw_atked_rate / pstDefender->m_armys.tier_atked_rate[idx];
                    if(ddwCurAttack < pstDefender->m_armys.units[udwArmyClass].single_hp)
                    {
                        dwFailedNum++;
                        continue;
                    }

                    if(ddwCurAttack >= pstDefender->m_armys.units[udwArmyClass].hp)
                    {
                        ddwDeadNum = pstDefender->m_armys.units[udwArmyClass].left_num;
                        ddwRealAttack -= pstDefender->m_armys.units[udwArmyClass].hp;

                        if(CToolBase::IsArmyClsTroop(udwArmyClass))
                        {
                            pstAttacker->m_armys.kill_troop_num += pstDefender->m_armys.units[udwArmyClass].left_num;
                            pstAttacker->m_armys.kill_troop_force += pstDefender->m_armys.units[udwArmyClass].left_num * pstDefender->m_armys.units[udwArmyClass].single_force;
                        }
                        else
                        {
                            pstAttacker->m_armys.kill_fort_num += pstDefender->m_armys.units[udwArmyClass].left_num;
                            pstAttacker->m_armys.kill_fort_force += pstDefender->m_armys.units[udwArmyClass].left_num * pstDefender->m_armys.units[udwArmyClass].single_force;
                        }                        
                       
                        TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("[wavetest]CWarBase::UnitsFightUnits[count=%d][normal-all]:target_tier=%d,target_army=%d----src_atk(%ld->%lld) tar_hp(%ld->0) tar_troop(%ld->0)  [seq=%u]",
                            dwCount, idx, udwArmyClass, ddwRawAttack, ddwRealAttack, pstDefender->m_armys.units[udwArmyClass].hp, ddwDeadNum, pstAttacker->m_pstUser->m_udwBSeqNo));

                        pstDefender->m_armys.units[udwArmyClass].left_num = 0;
                        pstDefender->m_armys.units[udwArmyClass].hp = 0;

                        pstDefender->m_armys.tier_atked_rate[idx] -= pstDefender->m_armys.units[udwArmyClass].raw_atked_rate;
                        pstDefender->m_armys.units[udwArmyClass].raw_atked_rate = 0;                        
                    }
                    else
                    {
                        ddwDeadNum = ddwCurAttack/pstDefender->m_armys.units[udwArmyClass].single_hp;
                        ddwLostHealth = ddwDeadNum*pstDefender->m_armys.units[udwArmyClass].single_hp;

                        ddwRealAttack -= ddwLostHealth;

                        if(CToolBase::IsArmyClsTroop(udwArmyClass))
                        {
                            pstAttacker->m_armys.kill_troop_num += ddwDeadNum;
                            pstAttacker->m_armys.kill_troop_force += ddwDeadNum * pstDefender->m_armys.units[udwArmyClass].single_force;
                        }
                        else
                        {
                            pstAttacker->m_armys.kill_fort_num += ddwDeadNum;
                            pstAttacker->m_armys.kill_fort_force += ddwDeadNum * pstDefender->m_armys.units[udwArmyClass].single_force;
                        }                        

                        TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("[wavetest]CWarBase::UnitsFightUnits[count=%d][normal-tmp]:target_tier=%d,target_army=%d----src_atk(%ld->%lld) tar_hp(%ld->%ld) tar_troop(%ld->%ld)  [seq=%u]",
                            dwCount, idx, udwArmyClass, ddwRawAttack, ddwRealAttack, pstDefender->m_armys.units[udwArmyClass].hp,
                            pstDefender->m_armys.units[udwArmyClass].hp - ddwLostHealth, pstDefender->m_armys.units[udwArmyClass].left_num, 
                            pstDefender->m_armys.units[udwArmyClass].left_num - ddwDeadNum, pstAttacker->m_pstUser->m_udwBSeqNo));

                        pstDefender->m_armys.units[udwArmyClass].left_num -= ddwDeadNum;
                        pstDefender->m_armys.units[udwArmyClass].hp -= ddwLostHealth;
                    }
                }

                if(dwFailedNum >= dwArmyCount)
                {
                    break;
                }
            }
        }
    }
}

TVOID CWarBase::FightOneRound(SBattleNode* pstAttacker, SBattleNode* pstDefender)
{
    // attacker to defender
    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("[wavetest]CWarBase::FightOneRound-----A  to  D [seq=%u]", pstAttacker->m_pstUser->m_udwBSeqNo));
    CWarBase::UnitsFightUnits(pstAttacker, pstDefender);

    // attacker to defender
    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("[wavetest]CWarBase::FightOneRound-----D  to  A [seq=%u]", pstAttacker->m_pstUser->m_udwBSeqNo));
    CWarBase::UnitsFightUnits(pstDefender, pstAttacker);

    return;
}

TVOID CWarBase::ComputeCasualty(SBattleNode* pstNode)
{
    pstNode->m_armys.total_dead_num = 0;

    double dead_ratio[EN_ARMS_CLS__END] = {0.0f};
    TINT64 dead_left[EN_ARMS_CLS__END] = {0};

    TUINT32 army_class = 0;
    for(TINT32 idx = 0; idx < pstNode->m_armys.valid_units_num; ++idx)
    {
        army_class = pstNode->m_armys.valid_units_list[idx];

        // calculate dead rate
        if(pstNode->m_armys.units[army_class].left_num == 0)
        {
            dead_ratio[army_class] = 1.0;
        }
        else
        {
            dead_ratio[army_class] = 1.0 - ((double)pstNode->m_armys.units[army_class].left_num / (double)pstNode->m_armys.units[army_class].raw_num);
        }
        // total dead num
        dead_left[army_class] = pstNode->m_armys.units[army_class].raw_num - pstNode->m_armys.units[army_class].left_num;
        pstNode->m_armys.total_dead_num += dead_left[army_class];
    }

    for(TINT32 idx = 0; idx < pstNode->m_armys.valid_units_num; ++idx)
    {
        army_class = pstNode->m_armys.valid_units_list[idx];
        if(!CToolBase::IsArmyClsTroop(army_class))
        {
            continue;
        }

        TUINT32 troop_type = CToolBase::GetTroopIdByArmyCls(army_class);

        for(std::map<ArmyId, OneArmy>::iterator it = pstNode->m_armys.actors.begin(); it != pstNode->m_armys.actors.end(); ++it)
        {
            if(it->second.raw_troop[troop_type] == 0)
            {
                continue;
            }

            TUINT32 dead_num = it->second.raw_troop[troop_type] * dead_ratio[army_class];
            if(dead_num > it->second.raw_troop[troop_type])
            {
                dead_num = it->second.raw_troop[troop_type];
            }
            if(dead_num > dead_left[army_class])
            {
                dead_num = dead_left[army_class];
            }

            //it->second.raw_troop[troop_type] -= dead_num;
            it->second.left_troop[troop_type] -= dead_num;
            it->second.dead_troop[troop_type] += dead_num;
            dead_left[army_class] -= dead_num;
        }

        if(dead_left[army_class] > 0)
        {
            for(std::map<ArmyId, OneArmy>::iterator it = pstNode->m_armys.actors.begin(); it != pstNode->m_armys.actors.end(); ++it)
            {
                if(it->second.raw_troop[troop_type] == 0)
                {
                    continue;
                }

                if(it->second.raw_troop[troop_type] >= dead_left[army_class])
                {
                    //it->second.raw_troop[troop_type] -= dead_left[army_class];
                    it->second.left_troop[troop_type] -= dead_left[army_class];
                    it->second.dead_troop[troop_type] += dead_left[army_class];
                    break;
                }
                else
                {
                    dead_left[army_class] -= it->second.raw_troop[troop_type];
                    it->second.left_troop[troop_type] -= it->second.raw_troop[troop_type];
                    it->second.dead_troop[troop_type] += it->second.raw_troop[troop_type];
                    //it->second.raw_troop[troop_type] = 0;
                    continue;
                }
            }
        }
    }


    for(TINT32 idx = 0; idx < pstNode->m_armys.valid_units_num; ++idx)
    {
        army_class = pstNode->m_armys.valid_units_list[idx];
        if(!CToolBase::IsArmyClsFort(army_class))
        {
            continue;
        }

        TUINT32 fort_type = CToolBase::GetFortIdByArmyCls(army_class);

        for(std::map<ArmyId, OneArmy>::iterator it = pstNode->m_armys.actors.begin(); it != pstNode->m_armys.actors.end(); ++it)
        {
            if(it->second.raw_fort[fort_type] == 0)
            {
                continue;
            }

            TUINT32 dead_num = it->second.raw_fort[fort_type] * dead_ratio[army_class];
            if(dead_num > it->second.raw_fort[fort_type])
            {
                dead_num = it->second.raw_fort[fort_type];
            }
            if(dead_num > dead_left[army_class])
            {
                dead_num = dead_left[army_class];
            }

            //it->second.raw_fort[fort_type] -= dead_num;
            it->second.left_fort[fort_type] -= dead_num;
            it->second.dead_fort[fort_type] += dead_num;
            dead_left[army_class] -= dead_num;
        }

        if(dead_left[army_class] > 0)
        {
            for(std::map<ArmyId, OneArmy>::iterator it = pstNode->m_armys.actors.begin(); it != pstNode->m_armys.actors.end(); ++it)
            {
                if(it->second.raw_fort[fort_type] == 0)
                {
                    continue;
                }

                if(it->second.raw_fort[fort_type] >= dead_left[army_class])
                {
                    //it->second.raw_fort[fort_type] -= dead_left[army_class];
                    it->second.left_fort[fort_type] -= dead_left[army_class];
                    it->second.dead_fort[fort_type] += dead_left[army_class];
                    break;
                }
                else
                {
                    dead_left[army_class] -= it->second.raw_fort[fort_type];
                    it->second.left_fort[fort_type] -= it->second.raw_fort[fort_type];
                    it->second.dead_fort[fort_type] += it->second.raw_fort[fort_type];
                    //it->second.raw_fort[fort_type] = 0;
                    continue;
                }
            }
        }
    }

    return;
}

TVOID CWarBase::ComputeScore(SBattleNode* pstNode)
{
    TUINT32 army_class = 0;
    TINT64 total_left_kill_troop_num = pstNode->m_armys.kill_troop_num;
    TINT64 total_left_kill_troop_force = pstNode->m_armys.kill_troop_force;
    TINT64 total_left_kill_fort_num = pstNode->m_armys.kill_fort_num;
    TINT64 total_left_kill_fort_force = pstNode->m_armys.kill_fort_force;

    for(TINT32 idx = 0; idx < pstNode->m_armys.valid_units_num; ++idx)
    {
        army_class = pstNode->m_armys.valid_units_list[idx];

        double ratio = 0.0f;
        TINT64 left_kill_troop_num = pstNode->m_armys.kill_troop_num * pstNode->m_armys.units[army_class].raw_atk_rate;
        TINT64 left_kill_troop_force = pstNode->m_armys.kill_troop_force * pstNode->m_armys.units[army_class].raw_atk_rate;
        TINT64 left_kill_fort_num = pstNode->m_armys.kill_fort_num * pstNode->m_armys.units[army_class].raw_atk_rate;
        TINT64 left_kill_fort_force = pstNode->m_armys.kill_fort_force * pstNode->m_armys.units[army_class].raw_atk_rate;

//         total_left_kill_troop_num -= left_kill_troop_num;
//         total_left_kill_troop_force -= left_kill_troop_force;
//         total_left_kill_fort_num -= left_kill_fort_num;
//         total_left_kill_fort_force -= left_kill_fort_force;

        if(left_kill_troop_num == 0 && left_kill_fort_num == 0)
        {
            continue;
        }

        for(std::map<ArmyId, OneArmy>::iterator it = pstNode->m_armys.actors.begin(); it != pstNode->m_armys.actors.end(); ++it)
        {
            if(CToolBase::IsArmyClsTroop(army_class))
            {
                TUINT32 troop_type = CToolBase::GetTroopIdByArmyCls(army_class);
                if(it->second.raw_troop[troop_type] == 0)
                {
                    continue;
                }
                ratio = (double)it->second.raw_troop[troop_type] / (double)pstNode->m_armys.units[army_class].raw_num;
            }
            else if(CToolBase::IsArmyClsFort(army_class))
            {
                TUINT32 fort_type = CToolBase::GetFortIdByArmyCls(army_class);
                if(it->second.raw_fort[fort_type] == 0)
                {
                    continue;
                }
                ratio = (double)it->second.raw_fort[fort_type] / (double)pstNode->m_armys.units[army_class].raw_num;
            }
            else
            {
                continue;
            }

            TINT64 kill_troop_num = left_kill_troop_num * ratio;
            TINT64 kill_troop_force = left_kill_troop_force * ratio;
            TINT64 kill_fort_num = left_kill_fort_num * ratio;
            TINT64 kill_fort_force = left_kill_fort_force * ratio;

//             kill_troop_num = (kill_troop_num > left_kill_troop_num) ? left_kill_troop_num : kill_troop_num;
//             kill_troop_force = (kill_troop_force > left_kill_troop_force) ? left_kill_troop_force : kill_troop_force;
//             kill_fort_num = (kill_fort_num > left_kill_fort_num) ? left_kill_fort_num : kill_fort_num;
//             kill_fort_force = (kill_fort_force > left_kill_fort_force) ? left_kill_fort_force : kill_fort_force;

            TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("[wavetest]CWarBase::ComputeScore[uid=%ld]:%f %ld %ld %ld %ld [seq=%u]",
                pstNode->m_pstUser->m_tbPlayer.m_nUid, ratio,
                kill_troop_num, kill_troop_force, kill_fort_num, kill_fort_force,
                pstNode->m_pstUser->m_udwBSeqNo));

            it->second.kill_troop_num += kill_troop_num;
            it->second.kill_troop_force += kill_troop_force;
            it->second.kill_fort_num += kill_fort_num;
            it->second.kill_fort_force += kill_fort_force;

            total_left_kill_troop_num -= kill_troop_num;
            total_left_kill_troop_force -= kill_troop_force;
            total_left_kill_fort_num -= kill_fort_num;
            total_left_kill_fort_force -= kill_fort_force;
//             left_kill_troop_num -= kill_troop_num;
//             left_kill_troop_force -= kill_troop_force;
//             left_kill_fort_num -= kill_fort_num;
//             left_kill_fort_force -= kill_fort_force;
        }

//         if(left_kill_troop_num > 0 || left_kill_troop_force > 0 || left_kill_fort_num > 0 || left_kill_fort_force > 0)
//         {
//             if(pstNode->m_armys.pstSelectArmyForProcessLeft)
//             {
//                 pstNode->m_armys.pstSelectArmyForProcessLeft->kill_troop_num += left_kill_troop_num;
//                 pstNode->m_armys.pstSelectArmyForProcessLeft->kill_troop_force += left_kill_troop_force;
//                 pstNode->m_armys.pstSelectArmyForProcessLeft->kill_fort_num += left_kill_fort_num;
//                 pstNode->m_armys.pstSelectArmyForProcessLeft->kill_fort_force += left_kill_fort_force;
//             }           
//         }
    }

    if(total_left_kill_troop_num > 0 || total_left_kill_troop_force > 0 || total_left_kill_fort_num > 0 || total_left_kill_fort_force > 0)
    {
        if(pstNode->m_armys.pstSelectArmyForProcessLeft)
        {
            pstNode->m_armys.pstSelectArmyForProcessLeft->kill_troop_num += total_left_kill_troop_num;
            pstNode->m_armys.pstSelectArmyForProcessLeft->kill_troop_force += total_left_kill_troop_force;
            pstNode->m_armys.pstSelectArmyForProcessLeft->kill_fort_num += total_left_kill_fort_num;
            pstNode->m_armys.pstSelectArmyForProcessLeft->kill_fort_force += total_left_kill_fort_force;
        }
    }
}

TVOID CWarBase::RecordBattleNode(SBattleNode* pstNode, Json::Value& record)
{
    record = Json::Value(Json::objectValue);

    record["army"] = Json::Value(Json::arrayValue);
    TUINT32 army_count = 0;
    for(std::map<ArmyId, OneArmy>::iterator it = pstNode->m_armys.actors.begin(); it != pstNode->m_armys.actors.end(); ++it)
    {
        CWarBase::RecordArmy(&(it->second), record["army"][army_count++]);
    }

    record["total_attack"] = pstNode->m_armys.raw_total_atk;
    record["total_defend"] = pstNode->m_armys.raw_total_def;

    CWarBase::RecordUnits(pstNode->m_armys.units, EN_ARMS_CLS__END, record["partA"]);
    CWarBase::RecordUnits(pstNode->m_armys.units, EN_ARMS_CLS__END, record["partB"]);

    CWarBase::RecordBuff(pstNode->m_armys.troop_buffs, EN_TROOP_TYPE__END, record["troop_buff"]);
    CWarBase::RecordBuff(pstNode->m_armys.fort_buffs, EN_FORT_TYPE__END, record["fort_buff"]);
}

TVOID CWarBase::RecordArmy(OneArmy* army, Json::Value& record)
{
    CGameInfo *poGameInfo = CGameInfo::GetInstance();

    const Json::Value &jTroop = poGameInfo->m_oJsonRoot["game_troop"];
    const Json::Value &jFort = poGameInfo->m_oJsonRoot["game_fort"];

    record = Json::Value(Json::objectValue);

    record["army_id"] = army->army_id;

    record["raw_troop"] = Json::Value(Json::arrayValue);
    ArrayOutput(army->raw_troop.m_addwNum, jTroop.size(), record["raw_troop"]);

    record["raw_fort"] = Json::Value(Json::arrayValue);
    ArrayOutput(army->raw_fort.m_addwNum, jFort.size(), record["raw_fort"]);

    record["left_troop"] = Json::Value(Json::arrayValue);
    ArrayOutput(army->left_troop.m_addwNum, jTroop.size(), record["left_troop"]);

    record["left_fort"] = Json::Value(Json::arrayValue);
    ArrayOutput(army->left_fort.m_addwNum, jFort.size(), record["left_fort"]);

    record["dead_troop"] = Json::Value(Json::arrayValue);
    ArrayOutput(army->dead_troop.m_addwNum, jTroop.size(), record["dead_troop"]);

    record["dead_fort"] = Json::Value(Json::arrayValue);
    ArrayOutput(army->dead_fort.m_addwNum, jFort.size(), record["dead_fort"]);
}

TVOID CWarBase::RecordUnits(BattleUnit* units, TUINT32 unit_num, Json::Value& record)
{
    record = Json::Value(Json::objectValue);

    record["raw_attack"] = Json::Value(Json::arrayValue);
    record["left_attack"] = Json::Value(Json::arrayValue);
    record["raw_life"] = Json::Value(Json::arrayValue);
    record["left_life"] = Json::Value(Json::arrayValue);
    record["raw_num"] = Json::Value(Json::arrayValue);
    record["left_num"] = Json::Value(Json::arrayValue);
    record["kill_troop_num"] = Json::Value(Json::arrayValue);
    record["kill_fort_num"] = Json::Value(Json::arrayValue);

    for(TUINT32 udwIdx = 0; udwIdx < unit_num; ++udwIdx)
    {
        record["raw_attack"].append(units[udwIdx].raw_atk);
        record["left_attack"].append(units[udwIdx].atk);
        record["raw_life"].append(units[udwIdx].raw_hp);
        record["left_life"].append(units[udwIdx].hp);
        record["raw_num"].append(units[udwIdx].raw_num);
        record["left_num"].append(units[udwIdx].left_num);
        record["kill_troop_num"].append(units[udwIdx].kill_troop_num);
        record["kill_fort_num"].append(units[udwIdx].kill_fort_num);
    }
}

TVOID CWarBase::RecordBuff(BattleBuff* buffs, TUINT32 buff_num, Json::Value& record)
{
    record = Json::Value(Json::objectValue);

    record["attack"] = Json::Value(Json::arrayValue);
    for(TUINT32 udwIdx = 0; udwIdx < buff_num; ++udwIdx)
    {
        record["attack"][udwIdx] = buffs[udwIdx].atk;
    }
    record["life"] = Json::Value(Json::arrayValue);
    for(TUINT32 udwIdx = 0; udwIdx < buff_num; ++udwIdx)
    {
        record["life"][udwIdx] = buffs[udwIdx].hp;
    }
    record["defend"] = Json::Value(Json::arrayValue);
    for(TUINT32 udwIdx = 0; udwIdx < buff_num; ++udwIdx)
    {
        record["defend"][udwIdx] = buffs[udwIdx].def;
    }
}

TVOID CWarBase::SelectBattleNode(TUINT32 udwTotalSelectNum, SCommonTroop *pstRawTroop, SCommonFort* pstRawFort, SCommonTroop *pstSelectTroop, SCommonFort *pstSelectFort)
{
    TUINT32 udwOwnTotalNum = 0;
    TUINT32 udwOwnTotalFort = 0;
    TUINT32 audwGradeTropNum[EN_TROOP_FORT_GRADE_TYPE_END] = {0};
    TUINT32 udwSelectGrade = 0;

    //for log
    TCHAR szBuffMsg[2048];
    TBOOL bHead = TRUE;
    TUINT32 udwLen = 0;
    for(TUINT32 udwIdx = 0; udwIdx < EN_TROOP_TYPE__END && udwIdx < CGameInfo::GetInstance()->m_oJsonRoot["game_troop"].size(); ++udwIdx)
    {
        if(bHead)
        {
            udwLen = snprintf(szBuffMsg, 2048, "%ld", pstRawTroop->m_addwNum[udwIdx]);
            bHead = FALSE;
        }
        else
        {
            udwLen += snprintf(szBuffMsg + udwLen, 2048 - udwLen, " %ld", pstRawTroop->m_addwNum[udwIdx]);
        }

    }
    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("CWarBase::SelectBattleNode:TotalSelectNum=%u  raw_troop[%s]", udwTotalSelectNum, szBuffMsg));

    memset(szBuffMsg,0,sizeof(TCHAR)*2048);
    bHead = TRUE;
    udwLen = 0;
    for(TUINT32 udwIdx = 0; udwIdx < EN_FORT_TYPE__END && udwIdx < CGameInfo::GetInstance()->m_oJsonRoot["game_fort"].size(); ++udwIdx)
    {
        if(bHead)
        {
            udwLen = snprintf(szBuffMsg, 2048, "%ld", pstRawFort->m_addwNum[udwIdx]);
            bHead = FALSE;
        }
        else
        {
            udwLen += snprintf(szBuffMsg + udwLen, 2048 - udwLen, " %ld", pstRawFort->m_addwNum[udwIdx]);
        }
    }
    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("CWarBase::SelectBattleNode:TotalSelectNum=%u  raw_fort[%s]", udwTotalSelectNum,szBuffMsg));


    //统计数据
    for(TUINT32 udwIdx = 0; udwIdx < EN_TROOP_TYPE__END && udwIdx < CGameInfo::GetInstance()->m_oJsonRoot["game_troop"].size(); ++udwIdx)
    {
        udwOwnTotalNum += pstRawTroop->m_addwNum[udwIdx];
        TUINT32 udwGrade = CGameInfo::GetInstance()->m_oJsonRoot["game_troop"][udwIdx]["a"]["a6"].asUInt();

        if(udwGrade == 0 || udwGrade >= EN_TROOP_FORT_GRADE_TYPE_END)
        {
            continue;
        }
        audwGradeTropNum[udwGrade] += pstRawTroop->m_addwNum[udwIdx];

    }
    for(TUINT32 udwIdx = 0; udwIdx < EN_FORT_TYPE__END && udwIdx < CGameInfo::GetInstance()->m_oJsonRoot["game_fort"].size(); ++udwIdx)
    {
        udwOwnTotalFort += pstRawFort->m_addwNum[udwIdx];

    }
    udwTotalSelectNum = udwTotalSelectNum - udwOwnTotalFort;
    if(udwOwnTotalNum <= udwTotalSelectNum)
    {
        //总兵数小于上限 不需要选择
        for(TUINT32 udwIdx = 0; udwIdx < EN_TROOP_TYPE__END; ++udwIdx)
        {
            pstSelectTroop->m_addwNum[udwIdx] = pstRawTroop->m_addwNum[udwIdx];
        }
        for(TUINT32 udwIdx = 0; udwIdx < EN_FORT_TYPE__END; ++udwIdx)
        {
            pstSelectFort->m_addwNum[udwIdx] = pstRawFort->m_addwNum[udwIdx];
        }


        //for log
        memset(szBuffMsg, 0, sizeof(TCHAR)* 2048);
        bHead = TRUE;
        udwLen = 0;
        for(TUINT32 udwIdx = 0; udwIdx < EN_TROOP_TYPE__END && udwIdx < CGameInfo::GetInstance()->m_oJsonRoot["game_troop"].size(); ++udwIdx)
        {
            if(bHead)
            {
                udwLen = snprintf(szBuffMsg, 2048, "%ld", pstSelectTroop->m_addwNum[udwIdx]);
                bHead = FALSE;
            }
            else
            {
                udwLen += snprintf(szBuffMsg + udwLen, 2048 - udwLen, " %ld", pstSelectTroop->m_addwNum[udwIdx]);
            }

        }
        TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("CWarBase::SelectBattleNode: select_troop[%s]", szBuffMsg));

        memset(szBuffMsg, 0, sizeof(TCHAR)* 2048);
        bHead = TRUE;
        udwLen = 0;
        for(TUINT32 udwIdx = 0; udwIdx < EN_FORT_TYPE__END && udwIdx < CGameInfo::GetInstance()->m_oJsonRoot["game_fort"].size(); ++udwIdx)
        {
            if(bHead)
            {
                udwLen = snprintf(szBuffMsg, 2048, "%ld", pstSelectFort->m_addwNum[udwIdx]);
                bHead = FALSE;
            }
            else
            {
                udwLen += snprintf(szBuffMsg + udwLen, 2048 - udwLen, " %ld", pstSelectFort->m_addwNum[udwIdx]);
            }
        }
        TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("CWarBase::SelectBattleNode: select_fort[%s]", szBuffMsg));

        return;
    }
    else
    {
        //查看在哪个梯度里进行选择
        TUINT32 udwTmpNum = 0;
        for(TUINT32 udwIdx = EN_TROOP_FORT_GRADE_TYPE_T4; udwIdx >= EN_TROOP_FORT_GRADE_TYPE_T1;--udwIdx)
        {
            udwTmpNum += audwGradeTropNum[udwIdx];
            if(udwTmpNum > udwTotalSelectNum)
            {
                udwSelectGrade = udwIdx;
                break;
            }
        }
        assert(udwSelectGrade != 0);
    }

    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, (" CWarBase::SelectBattleNode:limit=%u udwSelectGrade=%u", udwTotalSelectNum, udwSelectGrade));

    TUINT32 udwSelectTmpNum = 0;
    //序列化选取梯度之前梯度的数据
    for(TUINT32 udwGradeIdx = EN_TROOP_FORT_GRADE_TYPE_T4; udwGradeIdx > udwSelectGrade; --udwGradeIdx)
    {
        for(TUINT32 udwIdx = 0; udwIdx < EN_TROOP_TYPE__END && udwIdx < CGameInfo::GetInstance()->m_oJsonRoot["game_troop"].size(); ++udwIdx)
        {
            TUINT32 udwGrade = CGameInfo::GetInstance()->m_oJsonRoot["game_troop"][udwIdx]["a"]["a6"].asUInt();
            if(udwGrade != udwGradeIdx)
            {
                continue;
            }
            if(pstRawTroop->m_addwNum[udwIdx] == 0)
            {
                continue;
            }
            pstSelectTroop->m_addwNum[udwIdx] = pstRawTroop->m_addwNum[udwIdx];
            udwSelectTmpNum += pstRawTroop->m_addwNum[udwIdx];
        }
    }
    //在选取梯度中选取数据
    TUINT32 udwTmpHaveSelect = 0;
    
    while(udwSelectTmpNum < udwTotalSelectNum)
    {
        TUINT32 udwMinNum = 1<<31;
        TUINT32 udwNum = 0;
        for(TUINT32 udwIdx = 0; udwIdx < EN_TROOP_TYPE__END && udwIdx < CGameInfo::GetInstance()->m_oJsonRoot["game_troop"].size(); ++udwIdx)
        {
            TUINT32 udwGrade = CGameInfo::GetInstance()->m_oJsonRoot["game_troop"][udwIdx]["a"]["a6"].asUInt();
            if(udwGrade != udwSelectGrade)
            {
                continue;
            }
            if(pstRawTroop->m_addwNum[udwIdx] == 0)
            {
                continue;
            }
            if(pstRawTroop->m_addwNum[udwIdx] <= udwTmpHaveSelect)
            {
                continue;
            }
            udwNum++;
            udwMinNum = udwMinNum < pstRawTroop->m_addwNum[udwIdx] - udwTmpHaveSelect ? udwMinNum : pstRawTroop->m_addwNum[udwIdx] - udwTmpHaveSelect;
        }

        if(udwMinNum * udwNum > udwTotalSelectNum - udwSelectTmpNum)
        {
            //已选完
            udwTmpHaveSelect += (udwTotalSelectNum - udwSelectTmpNum) / udwNum;
            udwSelectTmpNum = udwTotalSelectNum;

            break;
        }
        else
        {
            udwSelectTmpNum += udwNum * udwMinNum;
            udwTmpHaveSelect += udwMinNum;
        }
    }
    //挑选
    for(TUINT32 udwIdx = 0; udwIdx < EN_TROOP_TYPE__END && udwIdx < CGameInfo::GetInstance()->m_oJsonRoot["game_troop"].size(); ++udwIdx)
    {
        TUINT32 udwGrade = CGameInfo::GetInstance()->m_oJsonRoot["game_troop"][udwIdx]["a"]["a6"].asUInt();
        if(udwGrade != udwSelectGrade)
        {
            continue;
        }
        if(pstRawTroop->m_addwNum[udwIdx] == 0)
        {
            continue;
        }
        if(pstRawTroop->m_addwNum[udwIdx] <= udwTmpHaveSelect)
        {
            pstSelectTroop->m_addwNum[udwIdx] = pstRawTroop->m_addwNum[udwIdx];
        }
        else
        {
            pstSelectTroop->m_addwNum[udwIdx] = udwTmpHaveSelect;
        }
    }
    for(TUINT32 udwIdx = 0; udwIdx < EN_FORT_TYPE__END; ++udwIdx)
    {
        pstSelectFort->m_addwNum[udwIdx] = pstRawFort->m_addwNum[udwIdx];
    }

    //for log
    memset(szBuffMsg, 0, sizeof(TCHAR)* 2048);
    bHead = TRUE;
    udwLen = 0;
    for(TUINT32 udwIdx = 0; udwIdx < EN_TROOP_TYPE__END && udwIdx < CGameInfo::GetInstance()->m_oJsonRoot["game_troop"].size(); ++udwIdx)
    {
        if(bHead)
        {
            udwLen = snprintf(szBuffMsg, 2048, "%ld", pstSelectTroop->m_addwNum[udwIdx]);
            bHead = FALSE;
        }
        else
        {
            udwLen += snprintf(szBuffMsg + udwLen, 2048 - udwLen, " %ld", pstSelectTroop->m_addwNum[udwIdx]);
        }

    }
    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("CWarBase::SelectBattleNode: select_troop[%s]", szBuffMsg));

    memset(szBuffMsg, 0, sizeof(TCHAR)* 2048);
    bHead = TRUE;
    udwLen = 0;
    for(TUINT32 udwIdx = 0; udwIdx < EN_FORT_TYPE__END && udwIdx < CGameInfo::GetInstance()->m_oJsonRoot["game_fort"].size(); ++udwIdx)
    {
        if(bHead)
        {
            udwLen = snprintf(szBuffMsg, 2048, "%ld", pstSelectFort->m_addwNum[udwIdx]);
            bHead = FALSE;
        }
        else
        {
            udwLen += snprintf(szBuffMsg + udwLen, 2048 - udwLen, " %ld", pstSelectFort->m_addwNum[udwIdx]);
        }
    }
    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("CWarBase::SelectBattleNode: select_fort[%s]", szBuffMsg));

    return;
}

TVOID CWarBase::ComputeTotalLoad(SBattleNode *pstNode)
{
    if (pstNode == NULL || pstNode->m_pstUser == NULL || pstNode->m_ptbMainAttackAction == NULL)
    {
        return;
    }

    TINT64 ddwLoadBuff = pstNode->m_pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_TROOP_LOAD].m_ddwBuffTotal;

    if (!pstNode->m_bIsDragonJoin)
    {
        ddwLoadBuff = pstNode->m_pstUser->m_stBuffWithoutDragon[EN_BUFFER_INFO_TROOP_LOAD].m_ddwBuffTotal;
    }

    pstNode->m_armys.total_load = 0;
    std::map<ArmyId, OneArmy>::iterator it = pstNode->m_armys.actors.find(pstNode->m_ptbMainAttackAction->m_nId);
    if (it != pstNode->m_armys.actors.end())
    {
        it->second.load = CCommonBase::GetTroopTotalLoad(&pstNode->m_ptbMainAttackAction->m_bParam[0].m_stTroop) * (1 + 1.0 * ddwLoadBuff / 10000);
        pstNode->m_armys.total_load += it->second.load;
    }
    for (TUINT32 udwIdx = 0; udwIdx < pstNode->m_udwRallyReinforceNum; ++udwIdx)
    {
        TINT64 ddwId = pstNode->m_pastRallyReinforceList[udwIdx]->m_nId;
        it = pstNode->m_armys.actors.find(ddwId);
        if (it == pstNode->m_armys.actors.end())
        {
            continue;
        }
        it->second.load = CCommonBase::GetTroopTotalLoad(&pstNode->m_pastRallyReinforceList[udwIdx]->m_bParam[0].m_stTroop) * (1 + 1.0 * ddwLoadBuff / 10000);
        pstNode->m_armys.total_load += it->second.load;
    }
}

TVOID CWarBase::DistributeResource(SBattleNode *pstNode, SCommonResource *pstResource)
{
    if (pstNode == NULL || pstNode->m_ptbMainAttackAction == NULL || pstNode->m_armys.total_load == 0)
    {
        return;
    }

    map<TINT64, TINT64> mapHasResNum;
    mapHasResNum.clear();

    TFLOAT32 fRate = 0.0;

    SCommonResource stTmpResource = *pstResource;
    
    pstNode->m_armys.resource = stTmpResource;

    std::map<ArmyId, OneArmy>::iterator it = pstNode->m_armys.actors.find(pstNode->m_ptbMainAttackAction->m_nId);
    if (it != pstNode->m_armys.actors.end())
    {
        pstNode->m_ptbMainAttackAction->SetFlag(TbMARCH_ACTION_FIELD_PARAM);
        pstNode->m_ptbMainAttackAction->m_bParam[0].m_stResource.Reset();
        fRate = 1.0 * it->second.load / pstNode->m_armys.total_load;
        mapHasResNum[pstNode->m_ptbMainAttackAction->m_nId] = 0;
        for (TUINT32 udwIdx = 0; udwIdx < EN_RESOURCE_TYPE__END; udwIdx++)
        {
            if (pstResource->m_addwNum[udwIdx] == 0)
            {
                continue;
            }
            pstNode->m_ptbMainAttackAction->m_bParam[0].m_stResource[udwIdx] = pstResource->m_addwNum[udwIdx] * fRate;
            mapHasResNum[pstNode->m_ptbMainAttackAction->m_nId] += pstNode->m_ptbMainAttackAction->m_bParam[0].m_stResource[udwIdx];
            stTmpResource[udwIdx] -= pstNode->m_ptbMainAttackAction->m_bParam[0].m_stResource[udwIdx];
        }
        it->second.resource = pstNode->m_ptbMainAttackAction->m_bParam[0].m_stResource;
    }
    for (TUINT32 udwIdx = 0; udwIdx < pstNode->m_udwRallyReinforceNum; ++udwIdx)
    {
        it = pstNode->m_armys.actors.find(pstNode->m_pastRallyReinforceList[udwIdx]->m_nId);
        if (it != pstNode->m_armys.actors.end())
        {
            pstNode->m_pastRallyReinforceList[udwIdx]->SetFlag(TbMARCH_ACTION_FIELD_PARAM);
            pstNode->m_pastRallyReinforceList[udwIdx]->m_bParam[0].m_stResource.Reset();
            fRate = 1.0 * it->second.load / pstNode->m_armys.total_load;
            mapHasResNum[pstNode->m_pastRallyReinforceList[udwIdx]->m_nId] = 0;
            for (TUINT32 udwIdy = 0; udwIdy < EN_RESOURCE_TYPE__END; udwIdy++)
            {
                if (pstResource->m_addwNum[udwIdy] == 0)
                {
                    continue;
                }
                pstNode->m_pastRallyReinforceList[udwIdx]->m_bParam[0].m_stResource[udwIdy] = pstResource->m_addwNum[udwIdy] * fRate;
                mapHasResNum[pstNode->m_pastRallyReinforceList[udwIdx]->m_nId] += pstNode->m_pastRallyReinforceList[udwIdx]->m_bParam[0].m_stResource[udwIdy];
                stTmpResource[udwIdy] -= pstNode->m_pastRallyReinforceList[udwIdx]->m_bParam[0].m_stResource[udwIdy];
            }
            it->second.resource = pstNode->m_pastRallyReinforceList[udwIdx]->m_bParam[0].m_stResource;
        }
    }

    TBOOL bEmpty = TRUE;
    for (TUINT32 udwIdx = 0; udwIdx < EN_RESOURCE_TYPE__END; udwIdx++)
    {
        if (stTmpResource[udwIdx] > 0)
        {
            bEmpty = FALSE;
            break;
        }
    }

    if (bEmpty)
    {
        return;
    }

    TUINT32 udwResIdx = 0;
    it = pstNode->m_armys.actors.find(pstNode->m_ptbMainAttackAction->m_nId);
    if (it != pstNode->m_armys.actors.end())
    {
        if (mapHasResNum[pstNode->m_ptbMainAttackAction->m_nId] < it->second.load)
        {
            TINT64 dwLeft = it->second.load - mapHasResNum[pstNode->m_ptbMainAttackAction->m_nId];
            for (udwResIdx; udwResIdx < EN_RESOURCE_TYPE__END; udwResIdx++)
            {
                if (dwLeft == 0)
                {
                    break;
                }
                if (stTmpResource[udwResIdx] <= dwLeft)
                {
                    pstNode->m_ptbMainAttackAction->m_bParam[0].m_stResource[udwResIdx] += stTmpResource[udwResIdx];
                    stTmpResource[udwResIdx] = 0;
                    dwLeft -= stTmpResource[udwResIdx];
                }
                else
                {
                    pstNode->m_ptbMainAttackAction->m_bParam[0].m_stResource[udwResIdx] += dwLeft;
                    dwLeft = 0;
                    stTmpResource[udwResIdx] -= dwLeft;
                    break;
                }
            }
            it->second.resource = pstNode->m_ptbMainAttackAction->m_bParam[0].m_stResource;
        }
    }
    for (TUINT32 udwIdx = 0; udwIdx < pstNode->m_udwRallyReinforceNum; ++udwIdx)
    {
        TINT64 ddwId = pstNode->m_pastRallyReinforceList[udwIdx]->m_nId;
        it = pstNode->m_armys.actors.find(ddwId);
        if (it != pstNode->m_armys.actors.end())
        {
            if (mapHasResNum[ddwId] < it->second.load)
            {
                TINT64 dwLeft = it->second.load - mapHasResNum[ddwId];
                for (udwResIdx; udwResIdx < EN_RESOURCE_TYPE__END; udwResIdx++)
                {
                    if (dwLeft == 0)
                    {
                        break;
                    }
                    if (stTmpResource[udwResIdx] <= dwLeft)
                    {
                        pstNode->m_pastRallyReinforceList[udwIdx]->m_bParam[0].m_stResource[udwResIdx] += stTmpResource[udwResIdx];
                        stTmpResource[udwResIdx] = 0;
                        dwLeft -= stTmpResource[udwResIdx];
                    }
                    else
                    {
                        pstNode->m_pastRallyReinforceList[udwIdx]->m_bParam[0].m_stResource[udwResIdx] += dwLeft;
                        dwLeft = 0;
                        stTmpResource[udwResIdx] -= dwLeft;
                        break;
                    }
                }
                it->second.resource = pstNode->m_pastRallyReinforceList[udwIdx]->m_bParam[0].m_stResource;
            }
        }
    }
}

TINT32 CWarBase::GetWildAction(TbMarch_action *patbMarchList, TUINT32 udwMarchNum, TUINT32 udwWildPos)
{
    for (TUINT32 udwIdx = 0; udwIdx < udwMarchNum; udwIdx++)
    {
        if (patbMarchList[udwIdx].m_nTpos == udwWildPos
            && patbMarchList[udwIdx].m_nMclass == EN_ACTION_MAIN_CLASS__MARCH
            && (patbMarchList[udwIdx].m_nStatus == EN_MARCH_STATUS__LOADING || patbMarchList[udwIdx].m_nStatus == EN_MARCH_STATUS__CAMPING_WITH_PEACETIME || patbMarchList[udwIdx].m_nStatus == EN_MARCH_STATUS__CAMPING_NORMAL)
            && (patbMarchList[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__OCCUPY || patbMarchList[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__CAMP))
        {
            return udwIdx;
        }
    }

    return -1;
}

template<typename T>
TVOID CWarBase::ArrayOutput(T *NumNode, TUINT32 udwNodeNum, Json::Value &jNode)
{
    for (TUINT32 udwIdx = 0; udwIdx < udwNodeNum; ++udwIdx)
    {
        jNode[udwIdx] = NumNode[udwIdx];
    }
}

TVOID CWarBase::GetTotalTroop(const SBattleNode &stNode, SCommonTroop *pTroop)
{
    for(std::map<ArmyId, OneArmy>::const_iterator it = stNode.m_armys.actors.begin(); it != stNode.m_armys.actors.end(); ++it)
    {
        for(TUINT32 udwIdx = 0; udwIdx < EN_TROOP_TYPE__END; ++udwIdx)
        {
            pTroop->m_addwNum[udwIdx] += it->second.left_troop.m_addwNum[udwIdx];
        }
    }
}

TVOID CWarBase::GetTotalFort(const SBattleNode &stNode, SCommonFort *pFort)
{
    for(std::map<ArmyId, OneArmy>::const_iterator it = stNode.m_armys.actors.begin(); it != stNode.m_armys.actors.end(); ++it)
    {
        for(TUINT32 udwIdx = 0; udwIdx < EN_FORT_TYPE__END; ++udwIdx)
        {
            pFort->m_addwNum[udwIdx] += it->second.left_fort.m_addwNum[udwIdx];
        }
    }
}

TVOID CWarBase::GetRawTotalTroop(const SBattleNode &stNode, SCommonTroop *pRawTroop)
{
    for(std::map<ArmyId, OneArmy>::const_iterator it = stNode.m_armys.actors.begin(); it != stNode.m_armys.actors.end(); ++it)
    {
        for(TUINT32 udwIdx = 0; udwIdx < EN_TROOP_TYPE__END; ++udwIdx)
        {
            pRawTroop->m_addwNum[udwIdx] += it->second.raw_troop.m_addwNum[udwIdx];
        }
    }
}

TVOID CWarBase::GetRawTotalFort(const SBattleNode &stNode, SCommonFort *pRawFort)
{
    for(std::map<ArmyId, OneArmy>::const_iterator it = stNode.m_armys.actors.begin(); it != stNode.m_armys.actors.end(); ++it)
    {
        for(TUINT32 udwIdx = 0; udwIdx < EN_FORT_TYPE__END; ++udwIdx)
        {
            pRawFort->m_addwNum[udwIdx] += it->second.raw_fort.m_addwNum[udwIdx];
        }
    }
}

TVOID CWarBase::SetDragonNode(SUserInfo *pstUser, TbMarch_action *ptbReqAction, TbMap *ptbWild, SDragonNode *pstDragonNode)
{
    CGameInfo *poGameInfo = CGameInfo::GetInstance();

    pstDragonNode->m_ddwAttack = (10000 + pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_HERO_ATTACK].m_ddwBuffTotal) * 1.0 / 10000
        * poGameInfo->m_oJsonRoot["game_basic"][EN_GAME_BASIC_HERO_ATTACK_DEFAULT].asInt64();

    pstDragonNode->m_dwMaxMultiAttackTimes = poGameInfo->m_oJsonRoot["game_basic"][EN_GAME_BASIC_HERO_CONN_DEFAULT].asInt()
        + pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_HERO_COMBO].m_ddwBuffTotal;

    pstDragonNode->m_dwCritBuff = poGameInfo->m_oJsonRoot["game_basic"][EN_GAME_BASIC_HERO_BAOJI_BUFF].asInt();
    pstDragonNode->m_dwRaidBuff = poGameInfo->m_oJsonRoot["game_dragon"]["a10"].asInt();
    pstDragonNode->m_dwHeavyBuff = poGameInfo->m_oJsonRoot["game_dragon"]["a11"].asInt();
    pstDragonNode->m_dwDoubleBuff = poGameInfo->m_oJsonRoot["game_dragon"]["a12"].asInt();

    pstDragonNode->m_dwMonsterDefenceReduce = pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_MONSTER_DEFENSE_DOWN].m_ddwBuffTotal;

    pstDragonNode->m_dwCritChance = pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_HERO_CRIT].m_ddwBuffTotal;

    pstDragonNode->m_dwBeginMultiAttackTimes = 0;
    for(TUINT32 udwIdx = 0; udwIdx < ptbWild->m_bAttack_info.m_udwNum; ++udwIdx)
    {
        if(ptbWild->m_bAttack_info[udwIdx].m_ddwId == ptbReqAction->m_nSuid
            && CTimeUtils::GetUnixTime() < ptbWild->m_bAttack_info[udwIdx].m_ddwAttackTime)
        {
            pstDragonNode->m_dwBeginMultiAttackTimes = ptbWild->m_bAttack_info[udwIdx].m_ddwTimes;
            break;
        }
    }
    pstDragonNode->m_dwAlMutiAttackExpire = 1;
    for (TUINT32 udwIdx = 0; udwIdx < ptbWild->m_bAl_attack_info.m_udwNum; ++udwIdx)
    {
        if (pstUser->m_tbPlayer.m_nAlpos != 0
            && ptbWild->m_bAl_attack_info[udwIdx].m_ddwId == pstUser->m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET
            && CTimeUtils::GetUnixTime() < ptbWild->m_bAl_attack_info[udwIdx].m_ddwAttackTime)
        {
            pstDragonNode->m_dwBeginAlMultiAttackTimes = ptbWild->m_bAl_attack_info[udwIdx].m_ddwTimes;
            pstDragonNode->m_dwAlMutiAttackExpire = 0;
            break;
        }
    }

    pstDragonNode->m_dwEnergyCost = GetEnergyCost(pstUser, ptbReqAction, ptbWild);
    TINT32 dwCanAttackTimes = 0;
    if(ptbReqAction->m_nIf_max_attack)
    {
        if(pstDragonNode->m_dwEnergyCost != 0)
        {
            dwCanAttackTimes = pstUser->m_tbPlayer.m_nDragon_cur_energy / pstDragonNode->m_dwEnergyCost;
        }
        else
        {
            dwCanAttackTimes = MAX_ATTACK_TIMES;
        }
        if (dwCanAttackTimes > pstDragonNode->m_dwMaxMultiAttackTimes)
        {
            dwCanAttackTimes = pstDragonNode->m_dwMaxMultiAttackTimes;
        }
    }
    else
    {
        dwCanAttackTimes = 1;
    }
    pstDragonNode->m_dwCanAttackTimes = dwCanAttackTimes;

    pstDragonNode->m_dwRawExp = pstUser->m_tbPlayer.m_nDragon_exp;

    pstDragonNode->m_dwBMonsterHit = pstUser->m_tbPlayer.m_bMonster_hit[0].addwNum[ptbWild->m_nLevel];
    //pstDragonNode->m_dwBLeaderMonsterKillNum = pstUser->m_tbPlayer.m_bLeader_monster_kill[0].addwNum[ptbWild->m_nLevel];

    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("AttackMoster SetHeroNode: hero in node info\
        [uid=%u attack=%u can_attack_time=%u begin_attack_lv=%u max_attack_lv=%u crit_chance=%u crit_buff=%u defense=%u energy_cost=%u ] [seq=%u]",
        pstUser->m_tbPlayer.m_nUid, 
        pstDragonNode->m_ddwAttack, 
        pstDragonNode->m_dwCanAttackTimes, 
        pstDragonNode->m_dwBeginMultiAttackTimes,
        pstDragonNode->m_dwMaxMultiAttackTimes,
        pstDragonNode->m_dwCritChance,
        pstDragonNode->m_dwCritBuff,
        pstDragonNode->m_dwMonsterDefenceReduce,
        pstDragonNode->m_dwEnergyCost,
        pstUser->m_udwBSeqNo));
}

TVOID CWarBase::SetMonsterNode(TbMap *ptbWild, SMonsterNode *pstMonsterNode)
{
    const Json::Value& oWildResJson = CWildInfo::GetWildResInfo(ptbWild->m_nSid);
    TINT32 dwWildType = ptbWild->m_nType;
    TINT32 dwWildLevel = ptbWild->m_nLevel;
    if(!oWildResJson.isMember(CCommonFunc::NumToString(dwWildType)))
    {
        return;
    }

    pstMonsterNode->m_dwType = dwWildType;
    pstMonsterNode->m_dwLevel = dwWildLevel;
    pstMonsterNode->m_ddwHp = ptbWild->m_nBoss_life;
    pstMonsterNode->m_ddwRawHp = ptbWild->m_nBoss_life;
    pstMonsterNode->m_ddwMaxHp = oWildResJson[CCommonFunc::NumToString(dwWildType)]["a11"][dwWildLevel - 1][1u].asInt();
    pstMonsterNode->m_ddwDefence = oWildResJson[CCommonFunc::NumToString(dwWildType)]["a11"][dwWildLevel - 1][2u].asInt();
    pstMonsterNode->m_ddwLeader = ptbWild->m_nLeader_monster_flag;
}

TINT32 CWarBase::GetEnergyCost(SUserInfo *pstUser, TbMarch_action *ptbReqAction, TbMap *ptbWild)
{
    CGameInfo *poGameInfo = CGameInfo::GetInstance();
    const Json::Value& oWildResJson = CWildInfo::GetWildResInfo(ptbWild->m_nSid);
    TINT32 dwWildType = ptbWild->m_nType;
    TINT32 dwWildLevel = ptbWild->m_nLevel;
    if(!oWildResJson.isMember(CCommonFunc::NumToString(dwWildType)))
    {
        return 0;
    }

    TINT32 dwCostType = oWildResJson[CCommonFunc::NumToString(dwWildType)]["a0"]["a5"].asInt();
    TINT32 dwEnergyCost = 0;
    TFLOAT64 ffRate = 0.0001;
    switch(dwCostType)
    {
    case 0:
        dwEnergyCost = 0;
        break;
    case 1:
        dwEnergyCost = oWildResJson[CCommonFunc::NumToString(dwWildType)]["a10"][dwWildLevel - 1].asInt()
            / ((1.0 + ffRate * pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_HERO_STAMINA_USE].m_ddwBuffTotal));
        break;
    case 2:
        dwEnergyCost = CPlayerBase::GetCurDragonMaxEnergy(pstUser)
            * ((oWildResJson[CCommonFunc::NumToString(dwWildType)]["a10"][dwWildLevel - 1].asInt()
            / ((1.0  + ffRate * pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_HERO_STAMINA_USE].m_ddwBuffTotal))));
        break;
    default:
        break;
    }

    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("ProcessAttack:GetEnergyCost:cost_type=%u buff_127=%u base_59=%u buff_128=%u wild_a10=%u per_cost=%u [seq=%u]",
        dwCostType, 
        pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_HERO_STAMINA_USE].m_ddwBuffTotal,
        poGameInfo->m_oJsonRoot["op_game_data"]["basic"][EN_GAME_BASIC_HERO_ENERGY_DEFAULT].asInt64(),
        pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_HERO_STAMINA_LIMIT].m_ddwBuffTotal / 10000,
        oWildResJson[CCommonFunc::NumToString(dwWildType)]["a10"][dwWildLevel - 1].asInt(), 
        dwEnergyCost,
        pstUser->m_udwBSeqNo));
    return dwEnergyCost;
}

TVOID CWarBase::DragonNodeOutput(SDragonNode *pstNode, Json::Value &jNode)
{
    jNode = Json::Value(Json::objectValue);
    jNode["attack"] = pstNode->m_ddwAttack;
    jNode["can_attack_times"] = pstNode->m_dwCanAttackTimes;
    jNode["b_times"] = pstNode->m_dwBeginMultiAttackTimes;
    jNode["max_times"] = pstNode->m_dwMaxMultiAttackTimes;
    jNode["crit_chance"] = pstNode->m_dwCritChance;
    jNode["crit_buff"] = pstNode->m_dwCritBuff;
    jNode["defence_reduce"] = pstNode->m_dwMonsterDefenceReduce;
    jNode["energy_cost"] = pstNode->m_dwEnergyCost;
    jNode["total_damage"] = pstNode->m_ddwTotalDamage;
    jNode["next_times"] = pstNode->m_dwMultiAttackTimes;
    jNode["real_attack_times"] = pstNode->m_dwRealAttackTimes;
    jNode["exp"] = pstNode->m_dwExp;

    jNode["attack_info"] = Json::arrayValue;
    for(TINT32 dwIdx = 0; dwIdx < pstNode->m_dwRealAttackTimes; ++dwIdx)
    {
        jNode["attack_info"][dwIdx] = Json::arrayValue;
        jNode["attack_info"][dwIdx].append(pstNode->m_astAttackInfo[dwIdx].m_dwAttackType);
        jNode["attack_info"][dwIdx].append(pstNode->m_astAttackInfo[dwIdx].m_dwMultiAttackNum);
        jNode["attack_info"][dwIdx].append(pstNode->m_astAttackInfo[dwIdx].m_dwMultiAttackBuff);
        jNode["attack_info"][dwIdx].append(pstNode->m_astAttackInfo[dwIdx].m_ddwAttackDamage);
    }
    
}

TVOID CWarBase::MonsterNodeOutput(SMonsterNode *pstNode, Json::Value &jNode)
{
    jNode = Json::Value(Json::objectValue);
    jNode["raw_hp"] = pstNode->m_ddwRawHp;
    jNode["cur_hp"] = pstNode->m_ddwHp;
    jNode["defence"] = pstNode->m_ddwDefence;
    jNode["lost_hp"] = pstNode->m_ddwLostHp;
    jNode["is_dead"] = pstNode->m_bIsDead;

}


TVOID CWarBase::MonsterRewardOutput(TbMarch_action *ptbReqAction, Json::Value &jNode)
{
    jNode = Json::Value(Json::arrayValue);
    for(TINT32 dwIdx = 0; dwIdx <ptbReqAction->m_bReward[0].ddwTotalNum; ++dwIdx)
    {
        jNode[dwIdx] = Json::arrayValue;
        jNode[dwIdx].append(ptbReqAction->m_bReward[0].aRewardList[dwIdx].ddwType);
        jNode[dwIdx].append(ptbReqAction->m_bReward[0].aRewardList[dwIdx].ddwId);
        jNode[dwIdx].append(ptbReqAction->m_bReward[0].aRewardList[dwIdx].ddwNum);
    }

}

TINT32 CWarBase::SetDragonResult(TbMarch_action *ptbReqAction, SUserInfo *pstUser, TbMap *ptbWild, SDragonNode *pstDragonNode, SMonsterNode *pstMonsterNode)
{
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;

    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("ProcessAttack:dragon_energy=%u",ptbPlayer->m_nDragon_cur_energy));
    const Json::Value& oWildResJson = CWildInfo::GetWildResInfo(ptbWild->m_nSid);
    const Json::Value &jDragon = CGameInfo::GetInstance()->m_oJsonRoot["game_dragon"];
    TINT64 ddwCurTime = CTimeUtils::GetUnixTime();
    TINT32 dwWildType = ptbWild->m_nType;
    TINT32 dwWildLv = ptbWild->m_nLevel;
    if(!oWildResJson.isMember(CCommonFunc::NumToString(dwWildType)))
    {
        return 0;
    } 
    TUINT32 udwExpBefor = ptbPlayer->m_nDragon_exp;
    if (pstMonsterNode->m_ddwLostHp > 0)
    {
        TINT64 ddwExp = pstDragonNode->m_ddwTotalDamage + pstMonsterNode->m_ddwDefence * pstDragonNode->m_ddwRealDamageNum;
        ddwExp *= jDragon["a6"][dwWildLv - 1].asDouble();

        CPlayerBase::AddDragonExp(pstUser, ddwExp);
        pstDragonNode->m_dwExp = ptbPlayer->m_nDragon_exp - udwExpBefor;
    }

    TINT64 ddwBeforEnergy = ptbPlayer->m_nDragon_cur_energy;
    ptbPlayer->Set_Dragon_cur_energy(ptbPlayer->m_nDragon_cur_energy - pstDragonNode->m_dwEnergyCost * pstDragonNode->m_dwRealAttackTimes);
    if (ptbPlayer->m_nDragon_cur_energy < 0)
    {
        ptbPlayer->m_nDragon_cur_energy = 0;
    }
    if (ptbPlayer->m_nDragon_begin_recovery_time + jDragon["a5"].asInt64() <= ddwCurTime)
    {
        ptbPlayer->Set_Dragon_recovery_count(0);
    }
    if (ptbPlayer->m_nDragon_recovery_time == 0 && pstDragonNode->m_dwEnergyCost * pstDragonNode->m_dwRealAttackTimes > 0)
    {
        assert(jDragon["a4"].size());
        if (ptbPlayer->m_nDragon_recovery_count == 0)
        {
            ptbPlayer->Set_Dragon_begin_recovery_time(ddwCurTime);
        }
        ptbPlayer->Set_Dragon_recovery_time(CTimeUtils::GetUnixTime() + jDragon["a4"][(TINT32)ptbPlayer->m_nDragon_recovery_count].asInt64());
        if (ptbPlayer->m_nDragon_recovery_time > ptbPlayer->m_nDragon_begin_recovery_time + jDragon["a5"].asInt64())
        {
            ptbPlayer->m_nDragon_recovery_time = ptbPlayer->m_nDragon_begin_recovery_time + jDragon["a5"].asInt64();
        }
        ptbPlayer->Set_Dragon_recovery_count(ptbPlayer->m_nDragon_recovery_count + 1);
        if (ptbPlayer->m_nDragon_recovery_count >= jDragon["a4"].size())
        {
            ptbPlayer->m_nDragon_recovery_count = jDragon["a4"].size() - 1;
        }
    }

    ptbPlayer->m_bMonster_hit[0].addwNum[pstMonsterNode->m_dwLevel] = pstDragonNode->m_dwEMonsterHit;
    //ptbPlayer->m_bLeader_monster_kill[0].addwNum[pstMonsterNode->m_dwLevel] = pstDragonNode->m_dwELeaderMonsterKillNum;
    ptbPlayer->SetFlag(TbPLAYER_FIELD_MONSTER_HIT);
    //ptbPlayer->SetFlag(TbPLAYER_FIELD_LEADER_MONSTER_KILL);

    if (pstDragonNode->m_dwHitToZero)
    {
        ptbPlayer->m_bLeader_monster_gen[0].addwNum[pstMonsterNode->m_dwLevel] = 0;
        ptbPlayer->SetFlag(TbPLAYER_FIELD_LEADER_MONSTER_GEN);
    }

    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("ProcessAttack:[monster_lost_hp=%u befor_exp=%u after_exp=%u]\
                                                      [per_energy_cost=%u can_attack_time=%u real_attack_time=%u cost_energy=%u ]\
                                                      [befor_energy=%ld after_energy=%u]",
                                                      pstMonsterNode->m_ddwLostHp, udwExpBefor, ptbPlayer->m_nDragon_exp,
                                                      pstDragonNode->m_dwEnergyCost, pstDragonNode->m_dwCanAttackTimes,pstDragonNode->m_dwRealAttackTimes, pstDragonNode->m_dwEnergyCost * pstDragonNode->m_dwCanAttackTimes,
                                                      ddwBeforEnergy, ptbPlayer->m_nDragon_cur_energy));
    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("ProcessAttack:dragon_energy=%u", ptbPlayer->m_nDragon_cur_energy));

    return 0;
}

TINT32 CWarBase::SetMonsterResult(TbMarch_action *ptbReqAction, TbPlayer *ptbPlayer, TbPlayer *ptbChallenger, TbMap *ptbWild, SDragonNode *pstDragonNode, SMonsterNode *pstMonsterNode)
{
    CGameInfo *poGameInfo = CGameInfo::GetInstance();
    string szLv = CCommonFunc::NumToString(pstMonsterNode->m_dwLevel);
    const Json::Value& jMonsterHit = poGameInfo->m_oJsonRoot["game_monster_hit"][szLv];

    TUINT32 udwEndHit = (ptbPlayer->m_bMonster_hit[0].addwNum[pstMonsterNode->m_dwLevel] + jMonsterHit.size() - 1) % jMonsterHit.size();
    if (pstMonsterNode->m_ddwLeader == 0 && ptbPlayer->m_bLeader_monster_gen[0].addwNum[pstMonsterNode->m_dwLevel] < jMonsterHit[udwEndHit][1U].asInt64())
    {
        if (ptbPlayer->m_nUid != ptbWild->m_nUid)
        {
            CCommonBase::OccupyWild(ptbPlayer, ptbWild);
        }
    }

    TINT64 ddwChallengerId = 0;
    if(pstMonsterNode->m_bIsDead)
    {
        TBOOL bIsGenLeader = FALSE;
        if (pstMonsterNode->m_ddwLeader == 1)
        {
            ddwChallengerId = ptbWild->m_nUid;
        }
        if (pstMonsterNode->m_ddwLeader == 0 && ptbWild->m_nUid != 0)
        {
            if (ptbWild->m_nUid == ptbPlayer->m_nUid)
            {
                if (ptbPlayer->m_bLeader_monster_gen[0].addwNum[pstMonsterNode->m_dwLevel] < jMonsterHit[udwEndHit][1U].asInt64())
                {
                    CMapBase::SetLeaderMonsterMap(ptbWild);
                    ptbPlayer->m_bLeader_monster_gen[0].addwNum[pstMonsterNode->m_dwLevel]++;
                    ptbPlayer->SetFlag(TbPLAYER_FIELD_LEADER_MONSTER_GEN);
                    bIsGenLeader = TRUE;
                }
            }
            else if (ptbWild->m_nUid == ptbChallenger->m_nUid)
            {
                udwEndHit = (ptbChallenger->m_bMonster_hit[0].addwNum[pstMonsterNode->m_dwLevel] + jMonsterHit.size() - 1) % jMonsterHit.size();
                if (ptbChallenger->m_bLeader_monster_gen[0].addwNum[pstMonsterNode->m_dwLevel] < jMonsterHit[udwEndHit][1U].asInt64())
                {
                    CMapBase::SetLeaderMonsterMap(ptbWild);
                    ptbChallenger->m_bLeader_monster_gen[0].addwNum[pstMonsterNode->m_dwLevel]++;
                    ptbChallenger->SetFlag(TbPLAYER_FIELD_LEADER_MONSTER_GEN);
                    bIsGenLeader = TRUE;
                }
            }
        }
        
        if (bIsGenLeader == FALSE)
        {
            CMapBase::SetMonseterNestMap(ptbWild);
        }

        TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("SetMonseterNestMap:set nest map[sid=%u id=%u type=%u lv=%u action_id=%lld reward_left=%u]",
            ptbReqAction->m_nSid,
            ptbWild->m_nId,
            ptbWild->m_nType,
            ptbWild->m_nLevel,
            ptbReqAction->m_nId,
            ptbWild->m_nReward_left));
    }
    else
    {
        ptbWild->Set_Boss_life(pstMonsterNode->m_ddwHp);
        TUINT32 udwIdx = 0;
        for(udwIdx = 0; udwIdx < ptbWild->m_bAttack_info.m_udwNum; ++udwIdx)
        {
            if(ptbWild->m_bAttack_info[udwIdx].m_ddwId == ptbPlayer->m_nUid)
            {
                ptbWild->m_bAttack_info[udwIdx].m_ddwAttackTime = CTimeUtils::GetUnixTime()
                    + poGameInfo->m_oJsonRoot["game_basic"][EN_GAME_BASIC_HERO_MONSTER_CONN_TIME_INTERVAL].asUInt();

                pstDragonNode->m_dwMultiAttackTimes++;
                ptbWild->m_bAttack_info[udwIdx].m_ddwTimes = pstDragonNode->m_dwMultiAttackTimes;
                if(ptbWild->m_bAttack_info[udwIdx].m_ddwTimes > pstDragonNode->m_dwMaxMultiAttackTimes)
                {
                    ptbWild->m_bAttack_info[udwIdx].m_ddwTimes = 
                        ptbWild->m_bAttack_info[udwIdx].m_ddwTimes % (pstDragonNode->m_dwMaxMultiAttackTimes + 1);
                }

                TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("AttackMoster SetMonsterResult:monster out node info [uid=%u attack_lv=%u attack_time=%u]",
                                    ptbWild->m_bAttack_info[udwIdx].m_ddwId,
                                    ptbWild->m_bAttack_info[udwIdx].m_ddwTimes,
                                    ptbWild->m_bAttack_info[udwIdx].m_ddwAttackTime));
                break;
            }
        }
        if(udwIdx >= ptbWild->m_bAttack_info.m_udwNum)
        {
            ptbWild->m_bAttack_info[ptbWild->m_bAttack_info.m_udwNum].m_ddwId = ptbPlayer->m_nUid;
            ptbWild->m_bAttack_info[ptbWild->m_bAttack_info.m_udwNum].m_ddwAttackTime = CTimeUtils::GetUnixTime()
                + poGameInfo->m_oJsonRoot["game_basic"][EN_GAME_BASIC_HERO_MONSTER_CONN_TIME_INTERVAL].asUInt();

            pstDragonNode->m_dwMultiAttackTimes++;
            ptbWild->m_bAttack_info[ptbWild->m_bAttack_info.m_udwNum].m_ddwTimes = pstDragonNode->m_dwMultiAttackTimes;
            if(ptbWild->m_bAttack_info[ptbWild->m_bAttack_info.m_udwNum].m_ddwTimes > pstDragonNode->m_dwMaxMultiAttackTimes)
            {
                ptbWild->m_bAttack_info[ptbWild->m_bAttack_info.m_udwNum].m_ddwTimes = 
                    ptbWild->m_bAttack_info[ptbWild->m_bAttack_info.m_udwNum].m_ddwTimes % (pstDragonNode->m_dwMaxMultiAttackTimes + 1);
            }
            TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("AttackMoster SetMonsterResult:monster out node info[uid=%u attack_lv=%u attack_time=%u]",
                                    ptbWild->m_bAttack_info[ptbWild->m_bAttack_info.m_udwNum].m_ddwId,
                                    ptbWild->m_bAttack_info[ptbWild->m_bAttack_info.m_udwNum].m_ddwTimes,
                                    ptbWild->m_bAttack_info[ptbWild->m_bAttack_info.m_udwNum].m_ddwAttackTime));
            ptbWild->m_bAttack_info.m_udwNum++;
        }
        ptbWild->SetFlag(TbMAP_FIELD_ATTACK_INFO);

        if (ptbPlayer->m_nAlpos != 0 && pstMonsterNode->m_ddwLostHp != 0)
        {
            for (udwIdx = 0; udwIdx < ptbWild->m_bAl_attack_info.m_udwNum; ++udwIdx)
            {
                if (ptbWild->m_bAl_attack_info[udwIdx].m_ddwId == ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET)
                {
                    pstDragonNode->m_dwAlMultiAttackTimes++;
                    ptbWild->m_bAl_attack_info[udwIdx].m_ddwTimes = pstDragonNode->m_dwAlMultiAttackTimes;

                    if (pstDragonNode->m_dwAlMutiAttackExpire == 1)
                    {
                        ptbWild->m_bAl_attack_info[udwIdx].m_ddwAttackTime = CTimeUtils::GetUnixTime() +
                            poGameInfo->m_oJsonRoot["game_dragon"]["a7"].asUInt();
                    }

                    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("AttackMoster SetMonsterResult:monster out node info [aid=%u attack_lv=%u attack_time=%u]",
                        ptbWild->m_bAl_attack_info[udwIdx].m_ddwId,
                        ptbWild->m_bAl_attack_info[udwIdx].m_ddwTimes,
                        ptbWild->m_bAl_attack_info[udwIdx].m_ddwAttackTime));
                    break;
                }
            }
            if (udwIdx >= ptbWild->m_bAl_attack_info.m_udwNum)
            {
                ptbWild->m_bAl_attack_info[ptbWild->m_bAl_attack_info.m_udwNum].m_ddwId = ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET;

                pstDragonNode->m_dwAlMultiAttackTimes++;
                ptbWild->m_bAl_attack_info[ptbWild->m_bAl_attack_info.m_udwNum].m_ddwTimes = pstDragonNode->m_dwAlMultiAttackTimes;
                ptbWild->m_bAl_attack_info[ptbWild->m_bAl_attack_info.m_udwNum].m_ddwAttackTime = CTimeUtils::GetUnixTime() + poGameInfo->m_oJsonRoot["game_dragon"]["a7"].asUInt();

                TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("AttackMoster SetMonsterResult:monster out node info[uid=%u attack_lv=%u attack_time=%u]",
                    ptbWild->m_bAl_attack_info[ptbWild->m_bAl_attack_info.m_udwNum].m_ddwId,
                    ptbWild->m_bAl_attack_info[ptbWild->m_bAl_attack_info.m_udwNum].m_ddwTimes,
                    ptbWild->m_bAl_attack_info[ptbWild->m_bAl_attack_info.m_udwNum].m_ddwAttackTime));
                ptbWild->m_bAl_attack_info.m_udwNum++;
            }
            ptbWild->SetFlag(TbMAP_FIELD_AL_ATTACK_INFO);
        }
    }

    //set monster info
    SMonsterInfo *pstMonsterInfo = &ptbReqAction->m_bMonster_info[0];
    pstMonsterInfo->ddwDeadFlag = pstMonsterNode->m_bIsDead;
    pstMonsterInfo->ddwExpGet = pstDragonNode->m_dwExp;
    pstMonsterInfo->ddwSpecailHitType = 0;
    for (TINT32 dwIdx = 0; dwIdx < pstDragonNode->m_dwRealAttackTimes; dwIdx++)
    {
        if (pstDragonNode->m_astAttackInfo[dwIdx].m_dwAttackType != 0)
        {
            pstMonsterInfo->ddwSpecailHitType = pstDragonNode->m_astAttackInfo[dwIdx].m_dwAttackType;
            break;
        }
    }
    pstMonsterInfo->ddwHpLose = pstMonsterNode->m_ddwLostHp * 10000 / pstMonsterNode->m_ddwMaxHp;
    pstMonsterInfo->ddwLv = pstMonsterNode->m_dwLevel;
    pstMonsterInfo->ddwType = pstMonsterNode->m_dwType;
    pstMonsterInfo->ddwRawHp = pstMonsterNode->m_ddwRawHp;
    pstMonsterInfo->ddwLead = pstMonsterNode->m_ddwLeader;
    pstMonsterInfo->ddwChallengerId = ddwChallengerId;
    ptbReqAction->SetFlag(TbMARCH_ACTION_FIELD_MONSTER_INFO);

    return 0;
}

TVOID CWarBase::GetDropRewardResult(SMonsterNode *pstMonsterNode, TBOOL &bNormalReward, TBOOL &bEliteReward)
{

    bNormalReward = FALSE;
    bEliteReward = FALSE;

    if(0 == pstMonsterNode->m_ddwLostHp)
    {
        return;
    }

    bNormalReward = TRUE;
    if(pstMonsterNode->m_ddwLeader 
        && pstMonsterNode->m_bIsDead)
    {
        bEliteReward = TRUE;    
    }
    return;
}



TINT32 CWarBase::SetMonsterRewardResult(SUserInfo *pstUser, TbMarch_action *ptbReqAction, TbMap *ptbWild, TbAl_gift *ptbAlGift, SMonsterNode *pstMonsterNode, SDragonNode *pstDragon)
{
    if (pstMonsterNode->m_ddwLostHp == 0)
    {
        return 0;
    }

    if(pstMonsterNode->m_bIsDead)
    {
        CMapLogic::GenMosterAllianceGift(pstUser, ptbWild, ptbReqAction);
    }

    CGameInfo *pstGameInfo = CGameInfo::GetInstance();
    string szMonsterLv = CCommonFunc::NumToString(pstMonsterNode->m_dwLevel);
    
    TUINT32 udwMonsterHitSize = pstGameInfo->m_oJsonRoot["game_monster_hit"][szMonsterLv].size();

    for (TINT32 dwIdx = 0; dwIdx < pstDragon->m_dwRealAttackTimes; dwIdx++)
    {
        if (pstDragon->m_astAttackInfo[dwIdx].m_ddwRealDamage > 0)
        {
            const Json::Value &jReward = pstGameInfo->m_oJsonRoot["game_monster_hit"][szMonsterLv][(pstDragon->m_dwBMonsterHit + dwIdx) % udwMonsterHitSize][2U];
            SetMarchReward(ptbReqAction, jReward);
        }
    }

    return 0;
}

TINT32 CWarBase::SetChallengerResult(SUserInfo *pstChallenger, SMonsterNode *pstMonsterNode, SDragonNode *pstDragon)
{
    if (pstMonsterNode->m_ddwLeader && pstMonsterNode->m_bIsDead)
    {
        CGameInfo *pstGameInfo = CGameInfo::GetInstance();
        string szMonsterLv = CCommonFunc::NumToString(pstMonsterNode->m_dwLevel);
        TbPlayer *ptbPlayer = &pstChallenger->m_tbPlayer;
        const Json::Value& jReward = pstGameInfo->m_oJsonRoot["game_leader_monstor_kill"][szMonsterLv][(TUINT32)ptbPlayer->m_bLeader_monster_kill[0].addwNum[pstMonsterNode->m_dwLevel]];
        ptbPlayer->m_bLeader_monster_kill[0].addwNum[pstMonsterNode->m_dwLevel] = (ptbPlayer->m_bLeader_monster_kill[0].addwNum[pstMonsterNode->m_dwLevel] + 1) 
            % pstGameInfo->m_oJsonRoot["game_leader_monstor_kill"][szMonsterLv].size();
        ptbPlayer->SetFlag(TbPLAYER_FIELD_LEADER_MONSTER_KILL);

        //reward
        for (TUINT32 udwIdx = 0; udwIdx < jReward.size(); ++udwIdx)
        {
            if (jReward[udwIdx][0U].asUInt() >= EN_GLOBALRES_TYPE_END)
            {
                continue;
            }

            TUINT32 udwType = jReward[udwIdx][0U].asUInt();
            TUINT32 udwId = jReward[udwIdx][1U].asInt();
            TUINT32 udwNum = jReward[udwIdx][2U].asUInt();
            TBOOL bExit = FALSE;
            for (TUINT32 udwOwnIdx = 0; udwOwnIdx < pstDragon->m_stChallengerReward.ddwTotalNum; ++udwOwnIdx)
            {
                if (pstDragon->m_stChallengerReward.aRewardList[udwOwnIdx].ddwType == udwType &&
                    pstDragon->m_stChallengerReward.aRewardList[udwOwnIdx].ddwId == udwId)
                {
                    pstDragon->m_stChallengerReward.aRewardList[udwOwnIdx].ddwNum += udwNum;
                    bExit = TRUE;
                }
            }

            if (!bExit)
            {
                if (pstDragon->m_stChallengerReward.ddwTotalNum >= MAX_REWARD_ITEM_NUM)
                {
                    continue;
                }
                pstDragon->m_stChallengerReward.aRewardList[pstDragon->m_stChallengerReward.ddwTotalNum].ddwType = udwType;
                pstDragon->m_stChallengerReward.aRewardList[pstDragon->m_stChallengerReward.ddwTotalNum].ddwId = udwId;
                pstDragon->m_stChallengerReward.aRewardList[pstDragon->m_stChallengerReward.ddwTotalNum].ddwNum = udwNum;
                ++pstDragon->m_stChallengerReward.ddwTotalNum;
            }
        }

        CGlobalResLogic::AddGlobalRes(pstChallenger, &pstChallenger->m_stCityInfo, &pstDragon->m_stChallengerReward);
    }

    return 0;
}

TINT32 CWarBase::SetMarchReward(TbMarch_action *ptbReqAction, const Json::Value& jReward)
{
    for (TUINT32 udwIdx = 0; udwIdx < jReward.size(); ++udwIdx)
    {
        if (jReward[udwIdx][0U].asUInt() >= EN_GLOBALRES_TYPE_END)
        {
            continue;
        }

        TUINT32 udwType = jReward[udwIdx][0U].asUInt();
        TUINT32 udwId = jReward[udwIdx][1U].asInt();
        TUINT32 udwNum = jReward[udwIdx][2U].asUInt();
        TBOOL bExit = FALSE;
        for (TUINT32 udwOwnIdx = 0; udwOwnIdx < ptbReqAction->m_bReward[0].ddwTotalNum; ++udwOwnIdx)
        {
            if (ptbReqAction->m_bReward[0].aRewardList[udwOwnIdx].ddwType == udwType &&
                ptbReqAction->m_bReward[0].aRewardList[udwOwnIdx].ddwId == udwId)
            {
                ptbReqAction->m_bReward[0].aRewardList[udwOwnIdx].ddwNum += udwNum;
                bExit = TRUE;
            }
        }

        if (!bExit)
        {
            if (ptbReqAction->m_bReward[0].ddwTotalNum >= MAX_REWARD_ITEM_NUM)
            {
                continue;
            }
            ptbReqAction->m_bReward[0].aRewardList[ptbReqAction->m_bReward[0].ddwTotalNum].ddwType = udwType;
            ptbReqAction->m_bReward[0].aRewardList[ptbReqAction->m_bReward[0].ddwTotalNum].ddwId = udwId;
            ptbReqAction->m_bReward[0].aRewardList[ptbReqAction->m_bReward[0].ddwTotalNum].ddwNum = udwNum;
            ++ptbReqAction->m_bReward[0].ddwTotalNum;
        }
        ptbReqAction->SetFlag(TbMARCH_ACTION_FIELD_REWARD);
    }

    return 0;
}



TINT32 CWarBase::NewSetMarchReward(TbMarch_action *ptbReqAction, const vector<SOneGlobalRes *> &vecReward)
{
    for (TUINT32 udwIdx = 0; udwIdx < vecReward.size(); ++udwIdx)
    {
        if(vecReward[udwIdx]->ddwType >= EN_GLOBALRES_TYPE_END)
        {
            continue;
        }

        TUINT32 udwType = vecReward[udwIdx]->ddwType;
        TUINT32 udwId = vecReward[udwIdx]->ddwId;
        TUINT32 udwNum = vecReward[udwIdx]->ddwNum;
        
        TBOOL bExit = FALSE;
        for (TUINT32 udwOwnIdx = 0; udwOwnIdx < ptbReqAction->m_bReward[0].ddwTotalNum; ++udwOwnIdx)
        {
            if (ptbReqAction->m_bReward[0].aRewardList[udwOwnIdx].ddwType == udwType 
                && ptbReqAction->m_bReward[0].aRewardList[udwOwnIdx].ddwId == udwId)
            {
                ptbReqAction->m_bReward[0].aRewardList[udwOwnIdx].ddwNum += udwNum;
                bExit = TRUE;
            }
        }

        if (!bExit)
        {
            if (ptbReqAction->m_bReward[0].ddwTotalNum >= MAX_REWARD_ITEM_NUM)
            {
                continue;
            }
            ptbReqAction->m_bReward[0].aRewardList[ptbReqAction->m_bReward[0].ddwTotalNum].ddwType = udwType;
            ptbReqAction->m_bReward[0].aRewardList[ptbReqAction->m_bReward[0].ddwTotalNum].ddwId = udwId;
            ptbReqAction->m_bReward[0].aRewardList[ptbReqAction->m_bReward[0].ddwTotalNum].ddwNum = udwNum;
            ++ptbReqAction->m_bReward[0].ddwTotalNum;
        }
        ptbReqAction->SetFlag(TbMARCH_ACTION_FIELD_REWARD);
    }

    return 0;
}

TINT32 CWarBase::NewSetChallengerReward(SUserInfo *pstChallenger, SDragonNode *pstDragon, const vector<SOneGlobalRes *> &vecReward)
{

    //reward
    for (TUINT32 udwIdx = 0; udwIdx < vecReward.size(); ++udwIdx)
    {
        if(vecReward[udwIdx]->ddwType >= EN_GLOBALRES_TYPE_END)
        {
            continue;
        }

        TUINT32 udwType = vecReward[udwIdx]->ddwType;
        TUINT32 udwId = vecReward[udwIdx]->ddwId;
        TUINT32 udwNum = vecReward[udwIdx]->ddwNum;
        
        TBOOL bExit = FALSE;
        for (TUINT32 udwOwnIdx = 0; udwOwnIdx < pstDragon->m_stChallengerReward.ddwTotalNum; ++udwOwnIdx)
        {
            if (pstDragon->m_stChallengerReward.aRewardList[udwOwnIdx].ddwType == udwType 
                && pstDragon->m_stChallengerReward.aRewardList[udwOwnIdx].ddwId == udwId)
            {
                pstDragon->m_stChallengerReward.aRewardList[udwOwnIdx].ddwNum += udwNum;
                bExit = TRUE;
            }
        }

        if (!bExit)
        {
            if (pstDragon->m_stChallengerReward.ddwTotalNum >= MAX_REWARD_ITEM_NUM)
            {
                continue;
            }
            pstDragon->m_stChallengerReward.aRewardList[pstDragon->m_stChallengerReward.ddwTotalNum].ddwType = udwType;
            pstDragon->m_stChallengerReward.aRewardList[pstDragon->m_stChallengerReward.ddwTotalNum].ddwId = udwId;
            pstDragon->m_stChallengerReward.aRewardList[pstDragon->m_stChallengerReward.ddwTotalNum].ddwNum = udwNum;
            ++pstDragon->m_stChallengerReward.ddwTotalNum;
        }
    }

    CGlobalResLogic::AddGlobalRes(pstChallenger, &pstChallenger->m_stCityInfo, &pstDragon->m_stChallengerReward);

    return 0;
}

TVOID CWarBase::PrepareBattleUnits( SBattleNode* pstNode )
{
    CGameInfo *pobjGameInfo = CGameInfo::GetInstance();
    TUINT32 udwTroopTypeSize = pobjGameInfo->GetTroopTypeNum();
    TUINT32 udwFortTypeSize = pobjGameInfo->GetFortTypeNum();
    TUINT32 udwArmyClass = 0;
    TUINT32 udwTroopType = 0;
    STroopInfo *pstArmyInfo = NULL;

    // 计算army数量
    for(std::map<ArmyId, OneArmy>::iterator it = pstNode->m_armys.actors.begin(); it != pstNode->m_armys.actors.end(); ++it)
    {
        SCommonTroop& stAttendTroop = it->second.raw_troop;
        for(TUINT32 udwTroopType = 0; udwTroopType < udwTroopTypeSize; ++udwTroopType)
        {
            if(stAttendTroop[udwTroopType] == 0)
            {
                continue;
            }

            TUINT32 udwArmyClass = CToolBase::GetArmyClsByTroopId(udwTroopType);
            // num
            pstNode->m_armys.units[udwArmyClass].raw_num += stAttendTroop[udwTroopType];
            pstNode->m_armys.raw_troop_num += stAttendTroop[udwTroopType];

            //actors内部统计
            it->second.raw_army_num += stAttendTroop[udwTroopType];
        }

        if(it->second.is_city)
        {
            SCommonFort& stAttendFort = it->second.raw_fort;
            for(TUINT32 udwFortType = 0; udwFortType < udwFortTypeSize; ++udwFortType)
            {
                if(stAttendFort[udwFortType] == 0)
                {
                    continue;
                }

                TUINT32 udwArmyClass = CToolBase::GetArmyClsByFortId(udwFortType);
                // num
                pstNode->m_armys.units[udwArmyClass].raw_num += stAttendFort[udwFortType];
                pstNode->m_armys.raw_fort_num += stAttendFort[udwFortType];

                //actors内部统计
                it->second.raw_army_num += stAttendFort[udwFortType];
            }
        }

        // 筛选可用于做异常结算用的army(处理计算剩余的问题)
        if(pstNode->m_armys.pstSelectArmyForProcessLeft == NULL && it->second.raw_army_num > 0)
        {
            pstNode->m_armys.pstSelectArmyForProcessLeft = &it->second;
        }
    }

    // 计算army数据
    pstNode->m_armys.raw_army_num = pstNode->m_armys.raw_fort_num + pstNode->m_armys.raw_troop_num;
    for(udwArmyClass = 0; udwArmyClass < EN_ARMS_CLS__END; udwArmyClass++)
    {
        if(pstNode->m_armys.units[udwArmyClass].raw_num == 0)
        {
            continue;
        }

        pstArmyInfo = pobjGameInfo->GetArmyInfo(udwArmyClass);
        if(pstArmyInfo == NULL) //wave@20160613: 保护
        {
            continue;
        }
        TINT64 ddwNum = pstNode->m_armys.units[udwArmyClass].raw_num;
        TINT64 ddwTotalVal  = 0;

        //force
        pstNode->m_armys.units[udwArmyClass].single_force = pstArmyInfo->m_dwMight;

        //single
        if(udwArmyClass < EN_TROOP_CLS__END)
        {
            udwTroopType = CToolBase::GetTroopIdByArmyCls(udwArmyClass);
            pstNode->m_armys.units[udwArmyClass].single_hp = pstArmyInfo->m_fHealth * ((10000 + pstNode->m_armys.troop_buffs[udwTroopType].hp) * 1.0 / 10000) * ARMY_DATA_EXPAND_RATIO;
            pstNode->m_armys.units[udwArmyClass].single_atk = pstArmyInfo->m_fAttack * ((10000 + pstNode->m_armys.troop_buffs[udwTroopType].atk) * 1.0 / 10000) * ARMY_DATA_EXPAND_RATIO;
            pstNode->m_armys.units[udwArmyClass].single_def = pstArmyInfo->m_fDefense * ((10000 + pstNode->m_armys.troop_buffs[udwTroopType].def) * 1.0 / 10000) * ARMY_DATA_EXPAND_RATIO;
        }
        else
        {
            udwTroopType = CToolBase::GetFortIdByArmyCls(udwArmyClass);
            pstNode->m_armys.units[udwArmyClass].single_force = pstArmyInfo->m_dwMight;
            pstNode->m_armys.units[udwArmyClass].single_hp = pstArmyInfo->m_fHealth * ((10000 + pstNode->m_armys.fort_buffs[udwTroopType].hp) * 1.0 / 10000) * ARMY_DATA_EXPAND_RATIO;
            pstNode->m_armys.units[udwArmyClass].single_atk = pstArmyInfo->m_fAttack * ((10000 + pstNode->m_armys.fort_buffs[udwTroopType].atk) * 1.0 / 10000) * ARMY_DATA_EXPAND_RATIO;
            pstNode->m_armys.units[udwArmyClass].single_def = pstArmyInfo->m_fDefense * ((10000 + pstNode->m_armys.fort_buffs[udwTroopType].def) * 1.0 / 10000) * ARMY_DATA_EXPAND_RATIO;
        }

        //left num
        pstNode->m_armys.units[udwArmyClass].left_num = ddwNum;

        //health
        ddwTotalVal = ddwNum * pstNode->m_armys.units[udwArmyClass].single_hp;        
        pstNode->m_armys.units[udwArmyClass].raw_hp = ddwTotalVal;
        pstNode->m_armys.units[udwArmyClass].hp = ddwTotalVal;
        

        //defense
        ddwTotalVal = ddwNum * pstNode->m_armys.units[udwArmyClass].single_def;
        pstNode->m_armys.units[udwArmyClass].def = ddwTotalVal;
        pstNode->m_armys.raw_total_def += ddwTotalVal;

        //attack
        ddwTotalVal = ddwNum * pstNode->m_armys.units[udwArmyClass].single_atk;        
        pstNode->m_armys.units[udwArmyClass].raw_atk = ddwTotalVal;
        pstNode->m_armys.units[udwArmyClass].atk = ddwTotalVal;
        //pstNode->m_armys.raw_total_atk += ddwTotalVal;

        //war param---tier
        pstNode->m_armys.units[udwArmyClass].raw_rate = 1.0 * pstNode->m_armys.units[udwArmyClass].raw_num/pstNode->m_armys.raw_army_num;
        pstNode->m_armys.tier_num_rate[pstArmyInfo->m_dwTier] += pstNode->m_armys.units[udwArmyClass].raw_rate;
        pstNode->m_armys.tier_health[pstArmyInfo->m_dwTier] += pstNode->m_armys.units[udwArmyClass].hp;        

        //记录有效的udwArmyClass
        pstNode->m_armys.valid_units_list[pstNode->m_armys.valid_units_num] = udwArmyClass;
        pstNode->m_armys.valid_units_num++;

        //log
        TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("[wavetest]CWarBase::PrepareBattleUnits[uid=%ld]: class=%d num=%ld rate=%f hp=%ld(%f) def=%ld(%f) atk=%ld(%f) [seq=%u]",
            pstNode->m_pstUser->m_tbPlayer.m_nUid, udwArmyClass, ddwNum, pstNode->m_armys.units[udwArmyClass].raw_rate,
            pstNode->m_armys.units[udwArmyClass].hp, pstNode->m_armys.units[udwArmyClass].single_hp*1.0/pstArmyInfo->m_fHealth, 
            pstNode->m_armys.units[udwArmyClass].def, pstNode->m_armys.units[udwArmyClass].single_def*1.0/pstArmyInfo->m_fDefense, 
            pstNode->m_armys.units[udwArmyClass].atk, pstNode->m_armys.units[udwArmyClass].single_atk*1.0/pstArmyInfo->m_fAttack, pstNode->m_pstUser->m_udwBSeqNo));
    }
}

TVOID CWarBase::ComputeTotalAttack( SBattleNode* pstDefender, SBattleNode* pstAttacker )
{
    TUINT32 udwAtkArmyClass = 0;
    TUINT32 udwDefArmyClass = 0;
    CGameInfo *pobjGameInfo = CGameInfo::GetInstance();
    STroopInfo *pstArmyInfo = NULL;
    AttackOrders* pstAttackOrder = &CGameInfo::GetInstance()->m_objAttackFactor;
    TFLOAT64 dfTmpRate = 0.0;
    for(TINT32 idx = 0; idx < pstAttacker->m_armys.valid_units_num; idx++)//进攻方的army
    {
        udwAtkArmyClass = pstAttacker->m_armys.valid_units_list[idx];
        for(TINT32 idy = 0; idy < pstDefender->m_armys.valid_units_num; idy++)//防守方的army
        {
            udwDefArmyClass = pstDefender->m_armys.valid_units_list[idy];
            pstAttacker->m_armys.raw_total_atk += pstDefender->m_armys.units[udwDefArmyClass].raw_rate * 
                pstAttacker->m_armys.units[udwAtkArmyClass].atk * pstAttackOrder->orders[udwAtkArmyClass][udwDefArmyClass].atk_factor;

            TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("[wavetest]CWarBase::ComputeTotalAttack[uid=%ld]:total_atk=%ld atk_type=%u, def_type=%u [%f,%ld,%f] [seq=%u]",
                pstAttacker->m_pstUser->m_tbPlayer.m_nUid, pstAttacker->m_armys.raw_total_atk, udwAtkArmyClass, udwDefArmyClass, 
                pstDefender->m_armys.units[udwDefArmyClass].raw_rate,
                pstAttacker->m_armys.units[udwAtkArmyClass].atk,
                pstAttackOrder->orders[udwAtkArmyClass][udwDefArmyClass].atk_factor,
                pstAttacker->m_pstUser->m_udwBSeqNo));
        }
    }

    for(TINT32 idx = 0; idx < pstAttacker->m_armys.valid_units_num; idx++)//进攻方的army
    {
        udwAtkArmyClass = pstAttacker->m_armys.valid_units_list[idx];
        for(TINT32 idy = 0; idy < pstDefender->m_armys.valid_units_num; idy++)//防守方的army
        {
            udwDefArmyClass = pstDefender->m_armys.valid_units_list[idy];
            pstArmyInfo = pobjGameInfo->GetArmyInfo(udwDefArmyClass);
            dfTmpRate = pstDefender->m_armys.units[udwDefArmyClass].raw_rate * 
                pstAttacker->m_armys.units[udwAtkArmyClass].atk * pstAttackOrder->orders[udwAtkArmyClass][udwDefArmyClass].atk_factor/pstAttacker->m_armys.raw_total_atk;
            pstDefender->m_armys.units[udwDefArmyClass].raw_atked_rate += dfTmpRate;
            pstDefender->m_armys.tier_atked_rate[pstArmyInfo->m_dwTier] += dfTmpRate;
            pstAttacker->m_armys.units[udwAtkArmyClass].raw_atk_rate += dfTmpRate;
            TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("[wavetest]CWarBase::ComputeTotalAttack[uid=%ld]:total_atk=%ld atk_type=%u, atk_rate=%f [seq=%u]",
                pstAttacker->m_pstUser->m_tbPlayer.m_nUid, pstAttacker->m_armys.raw_total_atk, udwAtkArmyClass, 
                pstAttacker->m_armys.units[udwAtkArmyClass].raw_atk_rate,
                pstAttacker->m_pstUser->m_udwBSeqNo));
        }
    }
}




