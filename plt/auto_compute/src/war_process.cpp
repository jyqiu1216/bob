#include "war_process.h"

#include <cmath>
#include <float.h>
#include "game_info.h"
#include "common_func.h"
#include "global_serv.h"
#include "war_base.h"
#include "common_logic.h"
#include "process_report.h"
#include "msg_base.h"
#include "wild_info.h"
#include "buffer_base.h"
#include "sendmessage_base.h"
#include "activities_logic.h"
#include "conf_base.h"
#include "quest_logic.h"
#include "common_base.h"
#include "map_logic.h"
#include "tool_base.h"
#include "player_base.h"
#include "map_base.h"
#include "city_base.h"
#include "backpack_logic.h"

TINT32 CWarProcess::GetBattleType(TINT32 dwSecClass, TINT32 dwWildType, TINT32 dwWildUid, TINT32 dwSid)
{
    TINT32 dwBattleType = EN_BATTLE_TYPE__NO_OP;

    if(dwSecClass == EN_ACTION_SEC_CLASS__ATTACK)
    {
        if(dwWildType == EN_WILD_TYPE__CITY)
        {
            dwBattleType = EN_BATTLE_TYPE__ATTACK_CITY;
        }
        else if(CMapLogic::IsOccupyWild(dwSid, dwWildType) && dwWildUid > 0)
        {
            if (EN_WILD_CLASS_MONSTER_NEST == CMapLogic::GetWildClass(dwSid, dwWildType))
            {
                dwBattleType = EN_BATTLE_TYPE__ATTACK_OCCUPY_LAIR;
            }
            else
            {
                dwBattleType = EN_BATTLE_TYPE__ATTCK_OCCUPY;
            }
        }
        else if (dwWildType == EN_WILD_TYPE__CAMP)
        {
            dwBattleType = EN_BATTLE_TYPE__ATTACK_CAMP;
        }
    }
    else if(dwSecClass == EN_ACTION_SEC_CLASS__RALLY_WAR)
    {
        if(dwWildType == EN_WILD_TYPE__CITY)
        {
            dwBattleType = EN_BATTLE_TYPE__RALLY_CITY;
        }
    }
    else if (dwSecClass == EN_ACTION_SEC_CLASS__ATTACK_IDOL)
    {
        if (dwWildType == EN_WILD_TYPE__IDOL)
        {
            dwBattleType = EN_BATTLE_TYPE__ATTACK_IDOL;
        }
    }
    else if (dwSecClass == EN_ACTION_SEC_CLASS__ATTACK_THRONE
        || dwSecClass == EN_ACTION_SEC_CLASS__RALLY_WAR_THRONE)
    {
        if (dwWildType == EN_WILD_TYPE__THRONE_NEW)
        {
            dwBattleType = EN_BATTLE_TYPE__RALLY_THRONE;
        }
    }

    return dwBattleType;
}

TINT32 CWarProcess::ProcessWar(SSession *pstSession, SDragonNode *pstDragonNode, SMonsterNode *pstMonsterNode)
{
    CWarBase::SetDragonNode(&pstSession->m_stSourceUser, &pstSession->m_stReqMarch, &pstSession->m_stMapItem, pstDragonNode);
    CWarBase::SetMonsterNode(&pstSession->m_stMapItem, pstMonsterNode);

    TINT32 dwWarResult = ProcessAttack(pstDragonNode, pstMonsterNode);

    ComputeBattleResult(pstSession, pstDragonNode, pstMonsterNode);

    Json::Value jHeroNode;
    Json::Value jMonsterNode;
    // Json::Value jMonsterReward;
    Json::FastWriter write;
    CWarBase::DragonNodeOutput(pstDragonNode, jHeroNode);
    CWarBase::MonsterNodeOutput(pstMonsterNode, jMonsterNode);
    // CWarBase::MonsterRewardOutput(&pstSession->m_stReqMarch, jMonsterReward);
    TbPlayer *pstPlayer = &pstSession->m_stSourceUser.m_tbPlayer;
    TINT64 ddwuid = pstPlayer->m_nUid;
    TUINT32 udwSid = pstPlayer->m_nSid;
    TUINT64 uddwAlid = pstPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET;
    string strUname = pstPlayer->m_sUin;

    TSE_LOG_INFO(pstSession->m_poServLog, ("ComputeAttackMonsterScore: uid=%ld sid=%u alid=%lu name=%s [seq=%u]", ddwuid, udwSid, uddwAlid, strUname.c_str(), pstSession->m_udwSeqNo));
    CActivitesLogic::ComputeAttackMonsterScore(&pstSession->m_stSourceUser, pstMonsterNode->m_dwType, pstMonsterNode->m_dwLevel, pstDragonNode->m_dwRealAttackTimes, 
        ddwuid, udwSid, uddwAlid, strUname);
    if (pstMonsterNode->m_bIsDead == TRUE)
    {
        CActivitesLogic::ComputeKillMonsterScore(&pstSession->m_stSourceUser, pstMonsterNode->m_dwType, pstMonsterNode->m_dwLevel, ddwuid, udwSid, uddwAlid, strUname);
    }

    TSE_LOG_INFO(pstSession->m_poServLog, ("[seq=%u] HeroNode : %s", pstSession->m_udwSeqNo, write.write(jHeroNode).c_str()));
    TSE_LOG_INFO(pstSession->m_poServLog, ("[seq=%u] MonsterNode : %s", pstSession->m_udwSeqNo, write.write(jMonsterNode).c_str()));
    // TSE_LOG_INFO(pstSession->m_poServLog, ("[seq=%u] MonsterReward : %s", pstSession->m_udwSeqNo, write.write(jMonsterReward).c_str()));

    return dwWarResult;
}

TINT32 CWarProcess::ProcessWar(SSession *pstSession, SBattleNode *pstAttacker, SBattleNode *pstDefender, TbReport *ptbMarchReport, TUINT32 udwAttackType /*= EN_BATTLE_TYPE__NO_OP*/)
{
    udwAttackType = CWarProcess::GetBattleType(pstSession->m_stReqMarch.m_nSclass, pstSession->m_stMapItem.m_nType, pstSession->m_stMapItem.m_nUid, pstSession->m_udwReqSvrId);

    TUINT32 udwWarResult = EN_REPORT_RESULT_WIN;

    CWarProcess::SetBattleNode(pstSession, pstAttacker, pstDefender, udwAttackType);
    CWarProcess::SetReportBuffer(pstSession, pstAttacker, pstDefender, udwAttackType);

    udwWarResult = CWarProcess::ProcessAttack(pstAttacker, pstDefender);

    Json::Value jAttackerNode;
    Json::Value jDefenderNode;
    Json::FastWriter write;
    CWarBase::RecordBattleNode(pstAttacker, jAttackerNode);
    CWarBase::RecordBattleNode(pstDefender, jDefenderNode);

    TSE_LOG_INFO(pstSession->m_poServLog, ("[seq=%u] AttackerWarNode : %s", pstSession->m_udwSeqNo, write.write(jAttackerNode).c_str()));
    TSE_LOG_INFO(pstSession->m_poServLog, ("[seq=%u] DefenderWarNode : %s", pstSession->m_udwSeqNo, write.write(jDefenderNode).c_str()));

    CWarProcess::OutputBattleNode(pstAttacker);
    CWarProcess::OutputBattleNode(pstDefender);

    CWarProcess::ComputeBattleResult(pstSession, pstAttacker, pstDefender, udwWarResult, udwAttackType);

    //生成cb日志
    // cblog-attacker
    GenWarLog(pstSession, pstAttacker, pstDefender, "attack_finish", 0, udwWarResult);

    // cblog-defender
    if (pstSession->m_stMapItem.m_nType == EN_WILD_TYPE__CITY)
    {
        // city defender
        GenWarLog(pstSession, pstAttacker, pstDefender, "city_attacked", 1, udwWarResult);
        // encamp defender
        GenAllyEncampActionLog(pstSession, pstDefender, "encamp_attacked", udwWarResult);
    }
    else
    {
        // wild defender
        GenWarLog(pstSession, pstAttacker, pstDefender, "wild_attacked", 2, udwWarResult);
    }

    return udwWarResult;
}

TVOID CWarProcess::SetBattleNode(SSession *pstSession, SBattleNode *pstAttacker, SBattleNode *pstDefender, TUINT32 udwAttackType)
{
    pstAttacker->Reset();
    pstDefender->Reset();

    SUserInfo *pstSUser = &pstSession->m_stSourceUser;
    SUserInfo *pstTUser = &pstSession->m_stTargetUser;

    pstAttacker->m_pstUser = pstSUser;
    pstDefender->m_pstUser = pstTUser;

    pstAttacker->m_ptbPlayer = &pstSUser->m_tbPlayer;
    pstDefender->m_ptbPlayer = &pstTUser->m_tbPlayer;

    pstAttacker->m_pstCity = &pstSUser->m_stCityInfo;
    pstDefender->m_pstCity = &pstTUser->m_stCityInfo;

    switch (udwAttackType)
    {
    case EN_BATTLE_TYPE__ATTACK_CITY:
        CWarProcess::SetBattleNodeForAttackCity(pstSession, pstAttacker, pstDefender);
        break;
    case EN_BATTLE_TYPE__ATTCK_OCCUPY:
        CWarProcess::SetBattleNodeForAttackOccupy(pstSession, pstAttacker, pstDefender);
        break;
    case EN_BATTLE_TYPE__ATTCK_WILD:
        CWarProcess::SetBattleNodeForAttackWild(pstSession, pstAttacker, pstDefender);
        break;
    case EN_BATTLE_TYPE__RALLY_CITY:
        CWarProcess::SetBattleNodeForRallyCity(pstSession, pstAttacker, pstDefender);
        break;
    case EN_BATTLE_TYPE__ATTACK_OCCUPY_LAIR:
        CWarProcess::SetBattleNodeForAttackOccupyLair(pstSession, pstAttacker, pstDefender);
        break;
    case EN_BATTLE_TYPE__ATTACK_CAMP:
        CWarProcess::SetBattleNodeForAttackCamp(pstSession, pstAttacker, pstDefender);
        break;
    case EN_BATTLE_TYPE__ATTACK_IDOL:
        CWarProcess::SetBattleNodeForAttackIdol(pstSession, pstAttacker, pstDefender);
        break;
    case EN_BATTLE_TYPE__RALLY_THRONE:
        CWarProcess::SetBattleNodeForRallyThrone(pstSession, pstAttacker, pstDefender);
        break;
    default:
        assert(0);
        break;
    }
}

TVOID CWarProcess::SetReportBuffer(SSession *pstSession, SBattleNode *pstAttacker, SBattleNode *pstDefender, TUINT32 udwAttackType)
{
    SUserInfo *pstSUser = &pstSession->m_stSourceUser;
    SUserInfo *pstTUser = &pstSession->m_stTargetUser;
    TbMarch_action *ptbReqMarch = &pstSession->m_stReqMarch;
    TbCity *pstTCity = &pstTUser->m_stCityInfo.m_stTblData;
    TbPlayer *ptbTPlayer = &pstTUser->m_tbPlayer;

    CGameInfo *poGameInfo = CGameInfo::GetInstance();
    const Json::Value& jKnight = poGameInfo->m_oJsonRoot["game_knight"];
    const Json::Value& jResearch = poGameInfo->m_oJsonRoot["game_research"];
    const Json::Value& jBuff = poGameInfo->m_oJsonRoot["game_buff_func_info"];
    const Json::Value& jKnightPos = poGameInfo->m_oJsonRoot["game_knight_pos"];

    SReportBuffer *pstAtkBuffer = &pstAttacker->m_stReportBuffer;
    SReportBuffer *pstDefBuffer = &pstDefender->m_stReportBuffer;
    pstAtkBuffer->Reset();
    pstDefBuffer->Reset();
    TBOOL bInCity = IsDefenderInCity(udwAttackType);

    TBOOL bAtkDragon = FALSE;
    TBOOL bDefDragon = FALSE;
    if (ptbReqMarch->m_bParam[0].m_stDragon.m_ddwLevel > 0)
    {
        bAtkDragon = TRUE;
    }

    if (pstDefender->m_bIsDragonJoin)
    {
        bDefDragon = TRUE;
    }
    
    // knight
    if (ptbReqMarch->m_bParam[0].m_stKnight.ddwLevel > 0)
    {
        for (TUINT32 udwIdx = 0; udwIdx < jKnight["b"].size(); udwIdx++)
        {
            AddBuffer(pstAtkBuffer->astKnightBuff, pstAtkBuffer->udwKnightBuffNum, MAX_REPORT_BUFFER_NUM,
                jKnight["b"][udwIdx][0U].asInt(), jKnight["b"][udwIdx][1U].asInt() * ptbReqMarch->m_bParam[0].m_stKnight.ddwLevel);
        }
    }
    if (bInCity)
    {
        for (TUINT32 udwIdx = 0; udwIdx < pstTCity->m_bKnight.m_udwNum; ++udwIdx)
        {
            TUINT32 udwKnightPos = pstTCity->m_bKnight[udwIdx].ddwPos;
            if (udwKnightPos >= EN_KNIGHT_POS__RESOURCE && udwKnightPos <= EN_KNIGHT_POS__TROOP)
            {
                TUINT32 udwLv = CGameInfo::GetInstance()->ComputeKnightLevelByExp(pstTCity->m_bKnight[udwIdx].ddwExp);
                const Json::Value& jPosBuff = jKnightPos[CCommonFunc::NumToString(udwKnightPos)]["b"]["b0"][udwLv - 1];
                for (TUINT32 udwIdy = 0; udwIdy < jPosBuff.size(); ++udwIdy)
                {
                    TUINT32 udwBuffId = jPosBuff[udwIdy][0U].asUInt();
                    TUINT32 udwBuffNum = jPosBuff[udwIdy][1U].asUInt();
                    if (jBuff[CCommonFunc::NumToString(udwBuffId)]["a4"].asUInt() == EN_BUFF_PROPERTY__MARCH)
                    {
                        AddBuffer(pstAtkBuffer->astKnightBuff, pstAtkBuffer->udwKnightBuffNum, MAX_REPORT_BUFFER_NUM, udwBuffId, udwBuffNum);
                    }
                }
            }
        }
    }
    else
    {
        if (pstDefender->m_stKnight.ddwLevel > 0)
        {
            for (TUINT32 udwIdx = 0; udwIdx < jKnight["b"].size(); udwIdx++)
            {
                AddBuffer(pstDefBuffer->astKnightBuff, pstDefBuffer->udwKnightBuffNum, MAX_REPORT_BUFFER_NUM,
                    jKnight["b"][udwIdx][0U].asInt(), jKnight["b"][udwIdx][1U].asInt() * pstDefender->m_stKnight.ddwLevel);
            }
        }
    }

    // research
    SCommonResearch *pstSResearch = &pstSUser->m_stCityInfo.m_stTblData.m_bResearch[0];
    for (TUINT32 udwIdx = 0; udwIdx < EN_RESEARCH_TYPE__LIMIT; ++udwIdx)
    {
        if (pstSResearch->m_addwLevel[udwIdx] > 0)
        {
            TUINT32 udwBuffId = jResearch[CCommonFunc::NumToString(udwIdx)]["a"]["a5"].asUInt();
            if (udwBuffId > 0)
            {
                TUINT32 udwLv = pstSResearch->m_addwLevel[udwIdx] - 1;
                TUINT32 udwBuffNum = jResearch[CCommonFunc::NumToString(udwIdx)]["b"]["b0"][udwLv][CCommonFunc::NumToString(udwBuffId)][1U].asUInt();
                if (jBuff[CCommonFunc::NumToString(udwBuffId)]["a4"].asUInt() == EN_BUFF_PROPERTY__MARCH)
                {
                    AddBuffer(pstAtkBuffer->astResearchBuff, pstAtkBuffer->udwResearchBuffNum, MAX_REPORT_BUFFER_NUM, udwBuffId, udwBuffNum);
                }
            }
        }
    }
    if (pstTUser->m_tbPlayer.m_nUid > 0)
    {
        SCommonResearch *pstTResearch = &pstTUser->m_stCityInfo.m_stTblData.m_bResearch[0];
        for (TUINT32 udwIdx = 0; udwIdx < EN_RESEARCH_TYPE__LIMIT; ++udwIdx)
        {
            if (pstTResearch->m_addwLevel[udwIdx] > 0)
            {
                TUINT32 udwBuffId = jResearch[CCommonFunc::NumToString(udwIdx)]["a"]["a5"].asUInt();
                if (udwBuffId > 0)
                {
                    TUINT32 udwLv = pstTResearch->m_addwLevel[udwIdx] - 1;
                    TUINT32 udwBuffNum = jResearch[CCommonFunc::NumToString(udwIdx)]["b"]["b0"][udwLv][CCommonFunc::NumToString(udwBuffId)][1U].asUInt();
                    if (jBuff[CCommonFunc::NumToString(udwBuffId)]["a4"].asUInt() == EN_BUFF_PROPERTY__MARCH)
                    {
                        AddBuffer(pstDefBuffer->astResearchBuff, pstDefBuffer->udwResearchBuffNum, MAX_REPORT_BUFFER_NUM, udwBuffId, udwBuffNum);
                    }
                }
            }
        }
    }

    // dragon
    if (bAtkDragon)
    {
        for (TUINT32 udwIdx = 0; udwIdx < EN_SKILL_TYPE__LIMIT; ++udwIdx)
        {
            TUINT32 udwSkillLv = pstSUser->m_tbUserStat.m_bDragon_skill[0].m_addwLevel[udwIdx];
            if (udwSkillLv > 0)
            {
                const Json::Value& jSkill = poGameInfo->m_oJsonRoot["game_dragon_skill"][CCommonFunc::NumToString(udwIdx)]["b"]["b0"][udwSkillLv - 1];
                Json::Value::Members jMem = jSkill.getMemberNames();
                if (jMem.size() > 0 && jSkill.isMember(jMem[0]))
                {
                    TUINT32 udwBuffId = jSkill[jMem[0]][0U].asUInt();
                    TUINT32 udwBuffNum = jSkill[jMem[0]][1U].asUInt();
                    if (jBuff[CCommonFunc::NumToString(udwBuffId)]["a4"].asUInt() == EN_BUFF_PROPERTY__MARCH)
                    {
                        AddBuffer(pstAtkBuffer->astDragonBuff, pstAtkBuffer->udwDragonBuffNum, MAX_REPORT_BUFFER_NUM, udwBuffId, udwBuffNum);
                    }
                }
            }
        }
    }
    if (bDefDragon)
    {
        for (TUINT32 udwIdx = 0; udwIdx < EN_SKILL_TYPE__LIMIT; ++udwIdx)
        {
            TUINT32 udwSkillLv = pstTUser->m_tbUserStat.m_bDragon_skill[0].m_addwLevel[udwIdx];
            if (udwSkillLv > 0)
            {
                const Json::Value& jSkill = poGameInfo->m_oJsonRoot["game_dragon_skill"][CCommonFunc::NumToString(udwIdx)]["b"]["b0"][udwSkillLv - 1];
                Json::Value::Members jMem = jSkill.getMemberNames();
                if (jMem.size() > 0 && jSkill.isMember(jMem[0]))
                {
                    TUINT32 udwBuffId = jSkill[jMem[0]][0U].asUInt();
                    TUINT32 udwBuffNum = jSkill[jMem[0]][1U].asUInt();
                    if (jBuff[CCommonFunc::NumToString(udwBuffId)]["a4"].asUInt() == EN_BUFF_PROPERTY__MARCH)
                    {
                        AddBuffer(pstDefBuffer->astDragonBuff, pstDefBuffer->udwDragonBuffNum, MAX_REPORT_BUFFER_NUM, udwBuffId, udwBuffNum);
                    }
                }
            }
        }
    }

    // equip
    if (bAtkDragon)
    {
        for (TUINT32 udwIdx = 0; udwIdx < pstSUser->m_udwEquipNum; ++udwIdx)
        {
            if (pstSUser->m_atbEquip[udwIdx].m_nStatus != EN_EQUIPMENT_STATUS_ON_DRAGON ||
                pstSUser->m_atbEquip[udwIdx].m_nPut_on_pos == 0 ||
                pstSUser->m_aucEquipFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
            {
                continue;
            }
            Json::Value::Members jBuffer = pstSUser->m_atbEquip[udwIdx].m_jBuff.getMemberNames();
            for (Json::Value::Members::iterator it = jBuffer.begin(); it != jBuffer.end(); ++it)
            {
                TUINT32 udwBuffId = pstSUser->m_atbEquip[udwIdx].m_jBuff[(*it).c_str()][0U].asUInt();
                TUINT32 dwBufferNum = pstSUser->m_atbEquip[udwIdx].m_jBuff[(*it).c_str()][1U].asInt();
                if (jBuff[CCommonFunc::NumToString(udwBuffId)]["a4"].asUInt() == EN_BUFF_PROPERTY__MARCH)
                {
                    AddBuffer(pstAtkBuffer->astDragonBuff, pstAtkBuffer->udwDragonBuffNum, MAX_REPORT_BUFFER_NUM, udwBuffId, dwBufferNum);
                }
            }
        }
    }
    if (bDefDragon)
    {
        for (TUINT32 udwIdx = 0; udwIdx < pstTUser->m_udwEquipNum; ++udwIdx)
        {
            if (pstTUser->m_atbEquip[udwIdx].m_nStatus != EN_EQUIPMENT_STATUS_ON_DRAGON ||
                pstTUser->m_atbEquip[udwIdx].m_nPut_on_pos == 0 ||
                pstTUser->m_aucEquipFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
            {
                continue;
            }
            Json::Value::Members jBuffer = pstTUser->m_atbEquip[udwIdx].m_jBuff.getMemberNames();
            for (Json::Value::Members::iterator it = jBuffer.begin(); it != jBuffer.end(); ++it)
            {
                TUINT32 udwBuffId = pstTUser->m_atbEquip[udwIdx].m_jBuff[(*it).c_str()][0U].asUInt();
                TUINT32 dwBufferNum = pstTUser->m_atbEquip[udwIdx].m_jBuff[(*it).c_str()][1U].asInt();
                if (jBuff[CCommonFunc::NumToString(udwBuffId)]["a4"].asUInt() == EN_BUFF_PROPERTY__MARCH)
                {
                    AddBuffer(pstDefBuffer->astDragonBuff, pstDefBuffer->udwDragonBuffNum, MAX_REPORT_BUFFER_NUM, udwBuffId, dwBufferNum);
                }
            }
        }
    }
    
    if (udwAttackType == EN_BATTLE_TYPE__ATTACK_IDOL)
    {
        TUINT32 udwBuffId = 0;
        TUINT32 udwBuffNum = 0;

        TUINT32 udwKnightLv = pstDefender->m_ptbIdol->m_jInfo["t"][1U].asInt();
        if (udwKnightLv > 0)
        {
            for (TUINT32 udwIdx = 0; udwIdx < jKnight["b"].size(); udwIdx++)
            {
                udwBuffId = jKnight["b"][udwIdx][0U].asInt();
                if (jBuff[CCommonFunc::NumToString(udwBuffId)]["a4"].asUInt() == EN_BUFF_PROPERTY__MARCH)
                {
                    udwBuffNum = jKnight["b"][udwIdx][1U].asInt() * udwKnightLv;
                    AddBuffer(pstDefBuffer->astKnightBuff, pstDefBuffer->udwKnightBuffNum, MAX_REPORT_BUFFER_NUM,
                        udwBuffId, udwBuffNum);
                }
            }
        }

        for (TUINT32 udwIdx = 0; udwIdx < pstDefender->m_ptbIdol->m_jInfo["t"][6U].size(); udwIdx++)
        {
            udwBuffId = pstDefender->m_ptbIdol->m_jInfo["t"][6U][udwIdx][0U].asUInt();

            if (jBuff[CCommonFunc::NumToString(udwBuffId)]["a4"].asUInt() == EN_BUFF_PROPERTY__MARCH)
            {
                udwBuffNum = pstDefender->m_ptbIdol->m_jInfo["t"][6U][udwIdx][1U].asUInt();
                AddBuffer(pstDefBuffer->astDragonBuff, pstDefBuffer->udwDragonBuffNum, MAX_REPORT_BUFFER_NUM, udwBuffId, udwBuffNum);
            }
        }
    }
}

TBOOL CWarProcess::IsDefenderInCity(TUINT32 udwAttackType)
{
    TBOOL bInCity = FALSE;

    switch (udwAttackType)
    {
    case EN_BATTLE_TYPE__ATTACK_CITY:
    case EN_BATTLE_TYPE__RALLY_CITY:
        bInCity = TRUE;
        break;
    default:
        break;
    }

    return bInCity;
}

TVOID CWarProcess::AddBuffer(SBuffInfo *pstBuff, TUINT32& udwBuffNum, TUINT32 udwMaxBuffNum, TUINT32 udwAddId, TUINT32 udwAddNum)
{
    if (udwAddId <= 0 || udwAddNum == 0)
    {
        return;
    }
    if (udwBuffNum > udwMaxBuffNum)
    {
        udwBuffNum = udwMaxBuffNum;
    }

    TINT32 dwFind = -1;
    for (TUINT32 udwIdx = 0; udwIdx < udwBuffNum; ++udwIdx)
    {
        if (pstBuff[udwIdx].ddwBuffId == udwAddId)
        {
            pstBuff[udwIdx].ddwBuffNum += udwAddNum;
            dwFind = udwIdx;
            break;
        }
    }

    if (dwFind == -1 && udwBuffNum < udwMaxBuffNum)
    {
        pstBuff[udwBuffNum].ddwBuffId = udwAddId;
        pstBuff[udwBuffNum].ddwBuffNum = udwAddNum;
        ++udwBuffNum;
    }
}

TVOID CWarProcess::OutputBattleNode(SBattleNode *pstNode)
{
    CWarProcess::OutputBattleNode(pstNode, pstNode->m_ptbMainAttackAction);
    CWarProcess::OutputBattleNode(pstNode, pstNode->m_pstDefenderCity, pstNode->m_ptbDefendPlayer);
    //CWarProcess::OutputBattleNode(pstNode, pstNode->m_ptbWild);

    TINT64 ddwTroopKill = 0;
    TINT64 ddwFortKill = 0;

    for(TUINT32 udwIdx = 0; udwIdx < pstNode->m_udwEncampNum; ++udwIdx)
    {
        ddwTroopKill = pstNode->m_pastEncampActionList[udwIdx]->m_nKill_troop_might;
        ddwFortKill = pstNode->m_pastEncampActionList[udwIdx]->m_nKill_fort_might;
        CWarProcess::OutputBattleNode(pstNode, pstNode->m_pastEncampActionList[udwIdx]);

        TINT64 ddwTUid = 1; //没有main action, 代表是防守方...
        if (pstNode->m_ptbMainAttackAction)
        {
            ddwTUid = pstNode->m_ptbMainAttackAction->m_nTuid;
        }
        TINT64 ddwUid = pstNode->m_pastEncampActionList[udwIdx]->m_nSuid;
        TUINT32 udwSid = pstNode->m_pastEncampActionList[udwIdx]->m_nSid;
        TUINT64 uddwAlid = pstNode->m_ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET;
        string strUname = pstNode->m_pastEncampActionList[udwIdx]->m_bParam[0].m_szSourceUserName;

        ddwTroopKill = pstNode->m_pastEncampActionList[udwIdx]->m_nKill_troop_might - ddwTroopKill;
        ddwFortKill = pstNode->m_pastEncampActionList[udwIdx]->m_nKill_fort_might - ddwFortKill;
        if (ddwTroopKill < 0)
        {
            ddwTroopKill = 0;
        }
        if (ddwFortKill < 0)
        {
            ddwFortKill = 0;
        }

        CActivitesLogic::ComputeTroopKillScore(pstNode->m_pstUser, ddwTroopKill,
            ddwFortKill, ddwTUid, ddwUid, udwSid, uddwAlid, strUname);
    }

    for(TUINT32 udwIdx = 0; udwIdx < pstNode->m_udwRallyReinforceNum; ++udwIdx)
    {
        ddwTroopKill = pstNode->m_pastRallyReinforceList[udwIdx]->m_nKill_troop_might;
        ddwFortKill = pstNode->m_pastRallyReinforceList[udwIdx]->m_nKill_fort_might;

        CWarProcess::OutputBattleNode(pstNode, pstNode->m_pastRallyReinforceList[udwIdx]);

        TINT64 ddwTUid = 1; //没有main action, 代表是防守方...
        if (pstNode->m_ptbMainAttackAction)
        {
            ddwTUid = pstNode->m_ptbMainAttackAction->m_nTuid;
        }
        TINT64 ddwUid = pstNode->m_pastRallyReinforceList[udwIdx]->m_nSuid;
        TUINT32 udwSid = pstNode->m_pastRallyReinforceList[udwIdx]->m_nSid;
        TUINT64 uddwAlid = pstNode->m_ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET;
        string strUname = pstNode->m_pastRallyReinforceList[udwIdx]->m_bParam[0].m_szSourceUserName;

        ddwTroopKill = pstNode->m_pastRallyReinforceList[udwIdx]->m_nKill_troop_might - ddwTroopKill;
        ddwFortKill = pstNode->m_pastRallyReinforceList[udwIdx]->m_nKill_fort_might - ddwFortKill;
        if (ddwTroopKill < 0)
        {
            ddwTroopKill = 0;
        }
        if (ddwFortKill < 0)
        {
            ddwFortKill = 0;
        }
        CActivitesLogic::ComputeTroopKillScore(pstNode->m_pstUser, ddwTroopKill,
            ddwFortKill, ddwTUid, ddwUid, udwSid, uddwAlid, strUname);
    }
}

TVOID CWarProcess::OutputBattleNode(SBattleNode *pstNode, TbMarch_action *ptbMarch)
{
    TbMap *ptbWild = pstNode->m_ptbWild;

    if(ptbMarch == NULL)
    {
        return;
    }
    TINT64 ddwId = ptbMarch->m_nId;
//     if(ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__OCCUPY && CMapLogic::GetWildClass(ptbWild->m_nSid, ptbWild->m_nType) != EN_WILD_CLASS_MONSTER_NEST)
//     {
//         ddwId = ptbMarch->m_nSuid;
//     }
    std::map<ArmyId, OneArmy>::iterator it = pstNode->m_armys.actors.find(ddwId);
    if(it == pstNode->m_armys.actors.end())
    {
        TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("moondo test: not found encamplist id[%ld]", ddwId));
        return;
    }

    ptbMarch->m_bParam[0].m_stTroop = it->second.left_troop;
    ptbMarch->m_bParam[0].m_ddwTroopNum = CToolBase::GetTroopSumNum(ptbMarch->m_bParam[0].m_stTroop);
    ptbMarch->m_bParam[0].m_ddwForce = CToolBase::GetTroopSumForce(ptbMarch->m_bParam[0].m_stTroop);
    ptbMarch->SetFlag(TbMARCH_ACTION_FIELD_PARAM);
    
    ptbMarch->Set_Kill_troop_might(ptbMarch->m_nKill_troop_might + it->second.kill_troop_force);
    ptbMarch->Set_Kill_fort_might(ptbMarch->m_nKill_fort_might + it->second.kill_fort_force);

    ptbMarch->Set_Kill_troop_num(ptbMarch->m_nKill_troop_num + it->second.kill_troop_num);
    ptbMarch->Set_Kill_fort_num(ptbMarch->m_nKill_fort_num + it->second.kill_fort_num);
}

TVOID CWarProcess::OutputBattleNode(SBattleNode *pstNode, SCityInfo *pstCity, TbPlayer *ptbPlayer)
{
    if(pstCity == NULL || ptbPlayer == NULL)
    {
        return;
    }
    std::map<ArmyId, OneArmy>::iterator it = pstNode->m_armys.actors.find(pstCity->m_stTblData.m_nUid);
    if(it == pstNode->m_armys.actors.end())
    {
        return;
    }

    if(it->second.is_city == FALSE)
    {
        return;
    }

    pstCity->m_stTblData.m_bTroop[0] = it->second.left_troop;
    pstCity->m_stTblData.m_bFort[0] = it->second.left_fort;
    pstCity->m_stTblData.SetFlag(TbCITY_FIELD_TROOP);
    pstCity->m_stTblData.SetFlag(TbCITY_FIELD_FORT);

    ptbPlayer->Set_Ktroop(ptbPlayer->m_nKtroop + it->second.kill_troop_force);
    ptbPlayer->Set_Kfort(ptbPlayer->m_nKfort + it->second.kill_fort_force);
    ptbPlayer->Set_Mkill(ptbPlayer->m_nMkill + it->second.kill_troop_force + it->second.kill_fort_force);
}

TVOID CWarProcess::OutputBattleNode(SBattleNode *pstNode, TbMap *ptbWild)
{
    if(ptbWild == NULL || ptbWild->m_nUid != 0)
    {
        return;
    }
    std::map<ArmyId, OneArmy>::iterator it = pstNode->m_armys.actors.find(0);
    if(it == pstNode->m_armys.actors.end())
    {
        return;
    }

    ptbWild->m_bTroop[0] = it->second.left_troop;
    ptbWild->SetFlag(TbMAP_FIELD_TROOP);
}

TUINT32 CWarProcess::ProcessAttack(SBattleNode *pstAttacker, SBattleNode *pstDefender)
{
    TUINT32 udwRetCode = EN_REPORT_RESULT_WIN;

    //must prepare defender first
    CWarBase::PrepareBattleUnits(pstDefender);
    CWarBase::PrepareBattleUnits(pstAttacker);

    //compute total attack
    CWarBase::ComputeTotalAttack(pstDefender, pstAttacker);
    CWarBase::ComputeTotalAttack(pstAttacker, pstDefender);

    //fight
    CWarBase::FightOneRound(pstAttacker, pstDefender);

    //must calculate score before casualty
    CWarBase::ComputeScore(pstAttacker);
    CWarBase::ComputeScore(pstDefender);

    CWarBase::ComputeCasualty(pstAttacker);
    CWarBase::ComputeCasualty(pstDefender);

    //全死判定
    if(pstAttacker->m_armys.raw_army_num - pstAttacker->m_armys.total_dead_num <= 0)
    {
        pstAttacker->bAllTroopDead = TRUE;
    }
    if(pstDefender->m_armys.raw_army_num - pstDefender->m_armys.total_dead_num <= 0)
    {
        pstDefender->bAllTroopDead = TRUE;
    }

    //胜负判定
    if(pstAttacker->m_armys.total_dead_num > pstDefender->m_armys.total_dead_num)
    {
        udwRetCode = EN_REPORT_RESULT_LOSE;
    }
    
    return udwRetCode;
}

TVOID CWarProcess::ComputeBattleResult(SSession *pstSession, SBattleNode *pstAttacker, SBattleNode *pstDefender, TUINT32 udwBattleResult, TUINT32 udwAttackType)
{
    switch (udwAttackType)
    {
    case EN_BATTLE_TYPE__ATTACK_CITY:
        CWarProcess::ComputeAttackCityResult(pstSession, pstAttacker, pstDefender, udwBattleResult);
        break;
    case EN_BATTLE_TYPE__ATTCK_OCCUPY:
        CWarProcess::ComputeAttackOccupyResult(pstSession, pstAttacker, pstDefender, udwBattleResult);
        break;
    case EN_BATTLE_TYPE__ATTCK_WILD:
        CWarProcess::ComputeAttackWildResult(pstSession, pstAttacker, pstDefender, udwBattleResult);
        break;
    case EN_BATTLE_TYPE__RALLY_CITY:
        CWarProcess::ComputeRallyCityResult(pstSession, pstAttacker, pstDefender, udwBattleResult);
        break;
    case EN_BATTLE_TYPE__ATTACK_OCCUPY_LAIR:
        CWarProcess::ComputeAttackOccupyLairResult(pstSession, pstAttacker, pstDefender, udwBattleResult);
        break;
    case EN_BATTLE_TYPE__ATTACK_CAMP:
        CWarProcess::ComputeAttackCampResult(pstSession, pstAttacker, pstDefender, udwBattleResult);
        break;
    case EN_BATTLE_TYPE__ATTACK_IDOL:
        ComputeAttackIdolResult(pstSession, pstAttacker, pstDefender, udwBattleResult);
        break;
    case EN_BATTLE_TYPE__RALLY_THRONE:
        CWarProcess::ComputeRallyThroneResult(pstSession, pstAttacker, pstDefender, udwBattleResult);
        break;
    default:
        assert(0);
        break;
    }
}

TVOID CWarProcess::RescueDragon(SSession *pstSession, SUserInfo* pstCaptor, TbPlayer* ptbSaver)
{
    CProcessReport::GenPrisonReport(pstCaptor, ptbSaver, EN_REPORT_TYPE_DRAGON_RELEASE, EN_REPORT_RESULT_HERO_RESCUE, &pstSession->m_tbPrisonReport);
    TBOOL bSucc = FALSE;
    for(TUINT32 udwIdx = 0; udwIdx < pstCaptor->m_udwPassiveMarchNum; ++udwIdx)
    {
        if(pstCaptor->m_aucPassiveMarchFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }

        if(pstCaptor->m_atbPassiveMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__PRISON_TIMER
        && pstCaptor->m_atbPassiveMarch[udwIdx].m_nStatus == EN_MARCH_STATUS__DEFENDING)
        {
            //pstCaptor->m_atbPassiveMarch[udwIdx].DeleteField(TbMARCH_ACTION_FIELD_TAL);
            pstCaptor->m_atbPassiveMarch[udwIdx].Set_Tal(0, UPDATE_ACTION_TYPE_DELETE);
            pstCaptor->m_atbPassiveMarch[udwIdx].m_bPrison_param[0].ddwResult = EN_PRISON_RESULT_BE_RELEASED;
            pstCaptor->m_atbPassiveMarch[udwIdx].SetFlag(TbMARCH_ACTION_FIELD_PRISON_PARAM);
            pstCaptor->m_atbPassiveMarch[udwIdx].Set_Status(EN_MARCH_STATUS__CALCING);
            pstCaptor->m_atbPassiveMarch[udwIdx].Set_Etime(CTimeUtils::GetUnixTime());

            //CActionBase::PrisonToMarch(&pstCaptor->m_atbPassiveMarch[udwIdx]);
            pstSession->m_vecPrisonReportReceivers.insert(pstCaptor->m_atbPassiveMarch[udwIdx].m_nSuid);
            if(pstCaptor->m_atbPassiveMarch[udwIdx].m_nSuid != ptbSaver->m_nUid) //被捕者不是营救者时才发被捕者的tip
            {
                //tips
                CSendMessageBase::AddTips(&pstSession->m_stSourceUser, EN_TIPS_TYPY__HERO_IS_RELEASED, pstCaptor->m_atbPassiveMarch[udwIdx].m_nSuid,
                    TRUE, 0, 0, 0, ptbSaver->m_sUin.c_str());//发给被捕者
            }
            bSucc = TRUE;
        }
    }
    if(!bSucc)
    {
        return;
    }
    pstSession->m_vecPrisonReportReceivers.insert(pstCaptor->m_tbPlayer.m_nUid);
    //tips
    CSendMessageBase::AddTips(pstCaptor, EN_TIPS_TYPY__PRISONER_IS_RESCUED, pstCaptor->m_tbPlayer.m_nUid, TRUE, 0, 0, 0, ptbSaver->m_sUin.c_str());//发给抓捕者
    if(ptbSaver)
    {
        pstSession->m_vecPrisonReportReceivers.insert(ptbSaver->m_nUid);
        //tips
        CSendMessageBase::AddTips(&pstSession->m_stSourceUser, EN_TIPS_TYPY__RESCUE_HERO, ptbSaver->m_nUid, TRUE, 0, 0, 0);//发给营救者
    }
    pstSession->m_dwReportFlag |= EN_REPORT_FLAG_PRISON;
}

TVOID CWarProcess::CaptureDragon(SSession *pstSession, SBattleNode *pstAttacker, SBattleNode *pstDefender, TUINT32 udwBattleResult)
{
    TbMarch_action *ptbReqMarch = &pstSession->m_stReqMarch;
    SUserInfo *pstSUser = &pstSession->m_stSourceUser;
    TbPlayer *ptbSPlayer = &pstSUser->m_tbPlayer;
    SUserInfo *pstTUser = &pstSession->m_stTargetUser;
    TbPlayer *ptbTPlayer = &pstTUser->m_tbPlayer;

    if (udwBattleResult == EN_REPORT_RESULT_WIN && pstDefender->bAllTroopDead && pstDefender->m_bIsDragonJoin)
    {
        //进攻方抓捕防守方
        TINT32 dwCaptureResult = CWarProcess::IsCaptureSucc(pstSUser, pstAttacker, pstDefender);
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("CWarProcess::CaptureHero: [Captor=%ld][Owner=%ld][Result=%d][seq=%u]",
            ptbSPlayer->m_nUid, ptbTPlayer->m_nUid,
            dwCaptureResult, pstSession->m_udwSeqNo));

        pstDefender->m_stDragon.m_ddwCaptured = dwCaptureResult;

        if(dwCaptureResult >= 0)
        {
            TbMarch_action* ptbPrisonTimer = CActionBase::AddNewMarch(pstTUser);

            ptbPrisonTimer->Set_Suid(ptbTPlayer->m_nUid);
            ptbPrisonTimer->Set_Scid(ptbReqMarch->m_nTpos);
            ptbPrisonTimer->Set_Tuid(ptbSPlayer->m_nUid);
            ptbPrisonTimer->Set_Mclass(EN_ACTION_MAIN_CLASS__TIMER);
            ptbPrisonTimer->Set_Sclass(EN_ACTION_SEC_CLASS__PRISON_TIMER);
            ptbPrisonTimer->Set_Status(EN_MARCH_STATUS__MARCHING);
            ptbPrisonTimer->Set_Btime(CTimeUtils::GetUnixTime());
            ptbPrisonTimer->Set_Ctime(ptbReqMarch->m_nCtime);

            TUINT32 udwBasicExcuteTime = CCommonBase::GetGameBasicVal(EN_GAME_BASIC_DRAGON_EXECUTED_TIME);
            TUINT32 udwPrisonTime = CCommonBase::GetGameBasicVal(EN_GAME_BASIC_DRAGON_IMPRISON_TIME) 
                + pstDefender->m_stDragon.m_ddwLevel * CCommonBase::GetGameBasicVal(EN_GAME_BASIC_EX_DRAGON_IMPRISON_TIME_PER_LV);
            if (udwPrisonTime >= pstSUser->m_stPlayerBuffList[EN_BUFFER_INFO_ALTAR_EXECUTE_TIME].m_ddwBuffTotal * 3600)
            {
                udwPrisonTime -= pstSUser->m_stPlayerBuffList[EN_BUFFER_INFO_ALTAR_EXECUTE_TIME].m_ddwBuffTotal * 3600;
            }
            else
            {
                udwPrisonTime = 0;
            }

            if(dwCaptureResult > 0)
            {
                udwBasicExcuteTime = 0;
            }

            TSE_LOG_DEBUG(pstSession->m_poServLog, ("CWarProcess::CaptureHero: [Captor=%ld][Owner=%ld][Excute=%u][Prison=%u][seq=%u]",
                ptbSPlayer->m_nUid, ptbTPlayer->m_nUid,
                udwBasicExcuteTime, udwPrisonTime, pstSession->m_udwSeqNo));
            ptbPrisonTimer->Set_Etime(ptbPrisonTimer->m_nBtime + ptbReqMarch->m_nCtime + udwBasicExcuteTime + udwPrisonTime);

            ptbPrisonTimer->m_bPrison_param[0].stDragon = pstDefender->m_stDragon;
            ptbPrisonTimer->m_bPrison_param[0].ddwJoinTimeStamp = ptbPrisonTimer->m_nBtime;
            ptbPrisonTimer->m_bPrison_param[0].ddwReleaseWait = udwBasicExcuteTime + udwPrisonTime;
            ptbPrisonTimer->m_bPrison_param[0].ddwExcuteWait = udwPrisonTime;
            ptbPrisonTimer->m_bPrison_param[0].ddwEscortActionId = ptbReqMarch->m_nId;
            strncpy(ptbPrisonTimer->m_bPrison_param[0].szSourceUserName, ptbReqMarch->m_bParam[0].m_szTargetUserName, MAX_TABLE_NAME_LEN);
            ptbPrisonTimer->m_bPrison_param[0].szSourceUserName[MAX_TABLE_NAME_LEN - 1] = '\0';
            strncpy(ptbPrisonTimer->m_bPrison_param[0].szTargetUserName, ptbReqMarch->m_bParam[0].m_szSourceUserName, MAX_TABLE_NAME_LEN);
            ptbPrisonTimer->m_bPrison_param[0].szTargetUserName[MAX_TABLE_NAME_LEN - 1] = '\0';

            strncpy(ptbPrisonTimer->m_bPrison_param[0].szTargetCityName, ptbReqMarch->m_bParam[0].m_szSourceCityName, MAX_TABLE_NAME_LEN);
            ptbPrisonTimer->m_bPrison_param[0].szTargetCityName[MAX_TABLE_NAME_LEN - 1] = '\0';

            ptbPrisonTimer->m_bPrison_param[0].ddwResult = EN_PRISON_RESULT_ESCAPE;

            ptbPrisonTimer->SetFlag(TbMARCH_ACTION_FIELD_PRISON_PARAM);

            ptbPrisonTimer->Set_Sid(ptbReqMarch->m_nSid);
            ptbPrisonTimer->Set_Tal(-1 * ptbSPlayer->m_nUid);
            ptbPrisonTimer->Set_Tid(ptbReqMarch->m_nId);
            ptbPrisonTimer->Set_Tbid(CMapBase::GetBlockIdFromPos(ptbReqMarch->m_nScid));

            strcpy(ptbPrisonTimer->m_bPrison_param[0].szSourceAlNick, ptbReqMarch->m_bParam[0].m_szTargetAlNick);
            strcpy(ptbPrisonTimer->m_bPrison_param[0].szTargetAlNick, ptbReqMarch->m_bParam[0].m_szSourceAlNick);

            ptbPrisonTimer->Set_Tuid(ptbSPlayer->m_nUid);
            ptbPrisonTimer->Set_Tpos(ptbReqMarch->m_nScid);

            ptbTPlayer->Set_Dragon_status(EN_DRAGON_STATUS_BEING_ESCORT);

            //进攻方加抓捕次数
            ptbSPlayer->m_bDragon_statistics[0].ddwCaptureDragonNum++;
            ptbSPlayer->SetFlag(TbPLAYER_FIELD_DRAGON_STATISTICS);
            //防守方加被抓次数
            ptbTPlayer->m_bDragon_statistics[0].ddwMyDragonCapturedNum++;
            ptbTPlayer->SetFlag(TbPLAYER_FIELD_DRAGON_STATISTICS);

            //tips
            CSendMessageBase::AddTips(&pstSession->m_stSourceUser, EN_TIPS_TYPY__CAPTURE_HERO, ptbSPlayer->m_nUid, TRUE, 0, 0, 0, ptbTPlayer->m_sUin.c_str());//抓捕方
            CSendMessageBase::AddTips(&pstSession->m_stTargetUser, EN_TIPS_TYPY__HERO_IS_CAPTURED, ptbTPlayer->m_nUid, TRUE, 0, 0, 0, ptbSPlayer->m_sUin.c_str());//被抓捕方

            SNoticInfo stNoticInfo;
            stNoticInfo.Reset();
            stNoticInfo.SetValue(EN_NOTI_ID__HERO_BE_CAPTURED,
                ptbSPlayer->m_sUin, "",
                0, 0,
                0, 0,
                0, "", 0);
            CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), ptbReqMarch->m_nSid, ptbTPlayer->m_nUid, stNoticInfo);
        }
    }
    else if (udwBattleResult == EN_REPORT_RESULT_LOSE && pstAttacker->bAllTroopDead && pstAttacker->m_bIsDragonJoin)
    {
        //防守方抓捕成功
        TINT32 dwCaptureResult = CWarProcess::IsCaptureSucc(pstTUser, pstDefender, pstAttacker);
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("CWarProcess::CaptureHero: [Captor=%ld][Owner=%ld][Result=%d][seq=%u]",
            ptbTPlayer->m_nUid, ptbSPlayer->m_nUid,
            dwCaptureResult, pstSession->m_udwSeqNo));

        pstAttacker->m_stDragon.m_ddwCaptured = dwCaptureResult;
        ptbReqMarch->m_bParam[0].m_stDragon.m_ddwCaptured = dwCaptureResult;

        if(dwCaptureResult >= 0)
        {
            ptbReqMarch->m_bParam[0].m_stDragon.m_ddwLevel = 0;
            ptbReqMarch->SetFlag(TbMARCH_ACTION_FIELD_PARAM);

            TbMarch_action* ptbPrisonTimer = CActionBase::AddNewMarch(pstSUser);

            ptbPrisonTimer->Set_Suid(ptbSPlayer->m_nUid);
            ptbPrisonTimer->Set_Scid(ptbReqMarch->m_nScid);
            ptbPrisonTimer->Set_Tuid(ptbTPlayer->m_nUid);
            ptbPrisonTimer->Set_Mclass(EN_ACTION_MAIN_CLASS__TIMER);
            ptbPrisonTimer->Set_Sclass(EN_ACTION_SEC_CLASS__PRISON_TIMER);
            ptbPrisonTimer->Set_Status(EN_MARCH_STATUS__DEFENDING);
            ptbPrisonTimer->Set_Btime(CTimeUtils::GetUnixTime());
            ptbPrisonTimer->Set_Ctime(ptbReqMarch->m_nCtime);

            TUINT32 udwBasicExcuteTime = CCommonBase::GetGameBasicVal(EN_GAME_BASIC_DRAGON_EXECUTED_TIME);
            TUINT32 udwPrisonTime = CCommonBase::GetGameBasicVal(EN_GAME_BASIC_DRAGON_IMPRISON_TIME)
                + pstAttacker->m_stDragon.m_ddwLevel * CCommonBase::GetGameBasicVal(EN_GAME_BASIC_EX_DRAGON_IMPRISON_TIME_PER_LV);
            if (udwPrisonTime >= pstTUser->m_stPlayerBuffList[EN_BUFFER_INFO_ALTAR_EXECUTE_TIME].m_ddwBuffTotal * 3600)
            {
                udwPrisonTime -= pstTUser->m_stPlayerBuffList[EN_BUFFER_INFO_ALTAR_EXECUTE_TIME].m_ddwBuffTotal * 3600;
            }
            else
            {
                udwPrisonTime = 0;
            }

            if (dwCaptureResult > 0)
            {
                udwBasicExcuteTime = 0;
            }

            TSE_LOG_DEBUG(pstSession->m_poServLog, ("CWarProcess::CaptureHero: [Captor=%ld][Owner=%ld][Excute=%u][Prison=%u][seq=%u]",
                ptbSPlayer->m_nUid, ptbTPlayer->m_nUid,
                udwBasicExcuteTime, udwPrisonTime, pstSession->m_udwSeqNo));

            ptbPrisonTimer->Set_Etime(ptbPrisonTimer->m_nBtime + udwPrisonTime + udwBasicExcuteTime);

            ptbPrisonTimer->m_bPrison_param[0].stDragon = pstAttacker->m_stDragon;
            ptbPrisonTimer->m_bPrison_param[0].ddwJoinTimeStamp = ptbPrisonTimer->m_nBtime;
            ptbPrisonTimer->m_bPrison_param[0].ddwReleaseWait = udwPrisonTime + udwBasicExcuteTime;
            ptbPrisonTimer->m_bPrison_param[0].ddwExcuteWait = udwPrisonTime;
            strncpy(ptbPrisonTimer->m_bPrison_param[0].szSourceUserName, ptbReqMarch->m_bParam[0].m_szSourceUserName, MAX_TABLE_NAME_LEN);
            ptbPrisonTimer->m_bPrison_param[0].szSourceUserName[MAX_TABLE_NAME_LEN - 1] = '\0';
            strncpy(ptbPrisonTimer->m_bPrison_param[0].szTargetUserName, ptbReqMarch->m_bParam[0].m_szTargetUserName, MAX_TABLE_NAME_LEN);
            ptbPrisonTimer->m_bPrison_param[0].szTargetUserName[MAX_TABLE_NAME_LEN - 1] = '\0';

            strncpy(ptbPrisonTimer->m_bPrison_param[0].szTargetCityName, ptbReqMarch->m_bParam[0].m_szTargetCityName, MAX_TABLE_NAME_LEN);
            ptbPrisonTimer->m_bPrison_param[0].szTargetCityName[MAX_TABLE_NAME_LEN - 1] = '\0';

            ptbPrisonTimer->m_bPrison_param[0].ddwResult = EN_PRISON_RESULT_ESCAPE;

            ptbPrisonTimer->SetFlag(TbMARCH_ACTION_FIELD_PRISON_PARAM);

            ptbPrisonTimer->Set_Sid(ptbReqMarch->m_nSid);
            ptbPrisonTimer->Set_Tal(-1 * ptbTPlayer->m_nUid);
            ptbPrisonTimer->Set_Tbid(CMapBase::GetBlockIdFromPos(ptbReqMarch->m_nTpos));
            strcpy(ptbPrisonTimer->m_bPrison_param[0].szSourceAlNick, ptbReqMarch->m_bParam[0].m_szSourceAlNick);
            strcpy(ptbPrisonTimer->m_bPrison_param[0].szTargetAlNick, ptbReqMarch->m_bParam[0].m_szTargetAlNick);

            ptbPrisonTimer->Set_Tuid(ptbTPlayer->m_nUid);
            ptbPrisonTimer->Set_Tpos(ptbReqMarch->m_nTpos);

            ptbSPlayer->Set_Dragon_status(EN_DRAGON_STATUS_WAIT_KILL);

            //防守方加抓捕次数
            ptbTPlayer->m_bDragon_statistics[0].ddwCaptureDragonNum++;
            ptbTPlayer->SetFlag(TbPLAYER_FIELD_DRAGON_STATISTICS);
            //进攻方加被抓次数
            ptbSPlayer->m_bDragon_statistics[0].ddwMyDragonCapturedNum++;
            ptbSPlayer->SetFlag(TbPLAYER_FIELD_DRAGON_STATISTICS);

            //tips
            CSendMessageBase::AddTips(&pstSession->m_stTargetUser, EN_TIPS_TYPY__CAPTURE_HERO, ptbTPlayer->m_nUid, TRUE, 0, 0, 0, ptbSPlayer->m_sUin.c_str());//抓捕方
            CSendMessageBase::AddTips(&pstSession->m_stSourceUser, EN_TIPS_TYPY__HERO_IS_CAPTURED, ptbSPlayer->m_nUid, TRUE, 0, 0, 0, ptbTPlayer->m_sUin.c_str());//被抓捕方

            SNoticInfo stNoticInfo;
            stNoticInfo.Reset();
            stNoticInfo.SetValue(EN_NOTI_ID__HERO_BE_CAPTURED,
                ptbTPlayer->m_sUin, "",
                0, 0,
                0, 0,
                0, "", 0);
            CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), ptbReqMarch->m_nSid, ptbSPlayer->m_nUid, stNoticInfo);
        }
    }
}

TVOID CWarProcess::CaptureDragonWild(SSession *pstSession, SBattleNode *pstAttacker, SBattleNode *pstDefender, TUINT32 udwBattleResult)
{
    TbMarch_action *ptbReqMarch = &pstSession->m_stReqMarch;
    TbMap *ptbWild = &pstSession->m_stMapItem;
    SUserInfo *pstSUser = &pstSession->m_stSourceUser;
    TbPlayer *ptbSPlayer = &pstSUser->m_tbPlayer;
    SUserInfo *pstTUser = &pstSession->m_stTargetUser;
    TbPlayer *ptbTPlayer = &pstTUser->m_tbPlayer;

    if (udwBattleResult == EN_REPORT_RESULT_WIN && pstDefender->bAllTroopDead && pstDefender->m_bIsDragonJoin)
    {
        //进攻方抓捕防守方
        TINT32 dwCaptureResult = CWarProcess::IsCaptureSucc(pstSUser, pstAttacker, pstDefender);
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("CWarProcess::CaptureDragonWild: [Captor=%ld][Owner=%ld][Result=%d][seq=%u]",
            ptbSPlayer->m_nUid, ptbTPlayer->m_nUid,
            dwCaptureResult, pstSession->m_udwSeqNo));

        pstDefender->m_stDragon.m_ddwCaptured = dwCaptureResult;

        if (dwCaptureResult >= 0)
        {
            // 防守方清龙
            for (TUINT32 udwIdx = 0; udwIdx < pstTUser->m_udwMarchNum; ++udwIdx)
            {
                TbMarch_action *ptbAction = &pstTUser->m_atbMarch[udwIdx];
                if (ptbAction->m_nTpos == ptbWild->m_nId && ptbAction->m_nSuid == ptbTPlayer->m_nUid &&
                    ptbAction->m_nStatus == EN_MARCH_STATUS__RETURNING)
                {
                    ptbAction->m_bParam[0].m_stDragon.m_ddwLevel = 0;
                    ptbAction->m_bParam[0].m_stDragon.m_ddwCaptured = dwCaptureResult;
                    ptbAction->SetFlag(TbMARCH_ACTION_FIELD_PARAM);
                    break;
                }
            }
            pstDefender->m_stDragon.m_ddwCaptured = dwCaptureResult;

            TbMarch_action* ptbPrisonTimer = CActionBase::AddNewMarch(pstTUser);

            ptbPrisonTimer->Set_Suid(ptbTPlayer->m_nUid);
            ptbPrisonTimer->Set_Scid(ptbReqMarch->m_nTpos);
            ptbPrisonTimer->Set_Tuid(ptbSPlayer->m_nUid);
            ptbPrisonTimer->Set_Mclass(EN_ACTION_MAIN_CLASS__TIMER);
            ptbPrisonTimer->Set_Sclass(EN_ACTION_SEC_CLASS__PRISON_TIMER);
            ptbPrisonTimer->Set_Status(EN_MARCH_STATUS__MARCHING);
            ptbPrisonTimer->Set_Btime(CTimeUtils::GetUnixTime());
            ptbPrisonTimer->Set_Ctime(ptbReqMarch->m_nCtime);

            TUINT32 udwBasicExcuteTime = CCommonBase::GetGameBasicVal(EN_GAME_BASIC_DRAGON_EXECUTED_TIME);
            TUINT32 udwPrisonTime = CCommonBase::GetGameBasicVal(EN_GAME_BASIC_DRAGON_IMPRISON_TIME)
                + pstDefender->m_stDragon.m_ddwLevel * CCommonBase::GetGameBasicVal(EN_GAME_BASIC_EX_DRAGON_IMPRISON_TIME_PER_LV);
            if (udwPrisonTime >= pstSUser->m_stPlayerBuffList[EN_BUFFER_INFO_ALTAR_EXECUTE_TIME].m_ddwBuffTotal * 3600)
            {
                udwPrisonTime -= pstSUser->m_stPlayerBuffList[EN_BUFFER_INFO_ALTAR_EXECUTE_TIME].m_ddwBuffTotal * 3600;
            }
            else
            {
                udwPrisonTime = 0;
            }

            if (dwCaptureResult > 0)
            {
                udwBasicExcuteTime = 0;
            }

            TSE_LOG_DEBUG(pstSession->m_poServLog, ("CWarProcess::CaptureDragonWild: [Captor=%ld][Owner=%ld][Excute=%u][Prison=%u][seq=%u]",
                ptbSPlayer->m_nUid, ptbTPlayer->m_nUid,
                udwBasicExcuteTime, udwPrisonTime, pstSession->m_udwSeqNo));

            ptbPrisonTimer->Set_Etime(ptbPrisonTimer->m_nBtime + ptbReqMarch->m_nCtime + udwPrisonTime + udwBasicExcuteTime);

            ptbPrisonTimer->m_bPrison_param[0].stDragon = pstDefender->m_stDragon;
            ptbPrisonTimer->m_bPrison_param[0].ddwJoinTimeStamp = ptbPrisonTimer->m_nBtime;
            ptbPrisonTimer->m_bPrison_param[0].ddwReleaseWait = udwPrisonTime + udwBasicExcuteTime;
            ptbPrisonTimer->m_bPrison_param[0].ddwExcuteWait = udwPrisonTime;
            ptbPrisonTimer->m_bPrison_param[0].ddwEscortActionId = ptbReqMarch->m_nId;
            strncpy(ptbPrisonTimer->m_bPrison_param[0].szSourceUserName, ptbReqMarch->m_bParam[0].m_szTargetUserName, MAX_TABLE_NAME_LEN);
            ptbPrisonTimer->m_bPrison_param[0].szSourceUserName[MAX_TABLE_NAME_LEN - 1] = '\0';
            strncpy(ptbPrisonTimer->m_bPrison_param[0].szTargetUserName, ptbReqMarch->m_bParam[0].m_szSourceUserName, MAX_TABLE_NAME_LEN);
            ptbPrisonTimer->m_bPrison_param[0].szTargetUserName[MAX_TABLE_NAME_LEN - 1] = '\0';

            strncpy(ptbPrisonTimer->m_bPrison_param[0].szTargetCityName, ptbReqMarch->m_bParam[0].m_szSourceCityName, MAX_TABLE_NAME_LEN);
            ptbPrisonTimer->m_bPrison_param[0].szTargetCityName[MAX_TABLE_NAME_LEN - 1] = '\0';

            ptbPrisonTimer->m_bPrison_param[0].ddwResult = EN_PRISON_RESULT_ESCAPE;

            ptbPrisonTimer->SetFlag(TbMARCH_ACTION_FIELD_PRISON_PARAM);

            ptbPrisonTimer->Set_Sid(ptbReqMarch->m_nSid);
            ptbPrisonTimer->Set_Tal(-1 * ptbSPlayer->m_nUid);
            ptbPrisonTimer->Set_Tid(ptbReqMarch->m_nId);
            ptbPrisonTimer->Set_Tbid(CMapBase::GetBlockIdFromPos(ptbReqMarch->m_nScid));

            strcpy(ptbPrisonTimer->m_bPrison_param[0].szSourceAlNick, ptbReqMarch->m_bParam[0].m_szTargetAlNick);
            strcpy(ptbPrisonTimer->m_bPrison_param[0].szTargetAlNick, ptbReqMarch->m_bParam[0].m_szSourceAlNick);

            ptbPrisonTimer->Set_Tuid(ptbSPlayer->m_nUid);
            ptbPrisonTimer->Set_Tpos(ptbReqMarch->m_nScid);

            ptbTPlayer->Set_Dragon_status(EN_DRAGON_STATUS_BEING_ESCORT);

            //进攻方加抓捕次数
            ptbSPlayer->m_bDragon_statistics[0].ddwCaptureDragonNum++;
            ptbSPlayer->SetFlag(TbPLAYER_FIELD_DRAGON_STATISTICS);
            //防守方加被抓次数
            ptbTPlayer->m_bDragon_statistics[0].ddwMyDragonCapturedNum++;
            ptbTPlayer->SetFlag(TbPLAYER_FIELD_DRAGON_STATISTICS);

            //tips
            CSendMessageBase::AddTips(&pstSession->m_stSourceUser, EN_TIPS_TYPY__CAPTURE_HERO, ptbSPlayer->m_nUid, TRUE, 0, 0, 0, ptbTPlayer->m_sUin.c_str());//抓捕方
            CSendMessageBase::AddTips(&pstSession->m_stTargetUser, EN_TIPS_TYPY__HERO_IS_CAPTURED, ptbTPlayer->m_nUid, TRUE, 0, 0, 0, ptbSPlayer->m_sUin.c_str());//被抓捕方

            SNoticInfo stNoticInfo;
            stNoticInfo.Reset();
            stNoticInfo.SetValue(EN_NOTI_ID__HERO_BE_CAPTURED,
                ptbSPlayer->m_sUin, "",
                0, 0,
                0, 0,
                0, "", 0);
            CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), ptbReqMarch->m_nSid, ptbTPlayer->m_nUid, stNoticInfo);
        }
    }
    else if (udwBattleResult == EN_REPORT_RESULT_LOSE && pstAttacker->bAllTroopDead && pstAttacker->m_bIsDragonJoin)
    {
        //防守方抓捕成功
        TINT32 dwCaptureResult = CWarProcess::IsCaptureSucc(pstTUser, pstDefender, pstAttacker);
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("CWarProcess::CaptureDragonWild: [Captor=%ld][Owner=%ld][Result=%d][seq=%u]",
            ptbTPlayer->m_nUid, ptbSPlayer->m_nUid,
            dwCaptureResult, pstSession->m_udwSeqNo));

        pstAttacker->m_stDragon.m_ddwCaptured = dwCaptureResult;
        ptbReqMarch->m_bParam[0].m_stDragon.m_ddwCaptured = dwCaptureResult;

        if (dwCaptureResult >= 0)
        {
            ptbReqMarch->m_bParam[0].m_stDragon.m_ddwLevel = 0;
            ptbReqMarch->SetFlag(TbMARCH_ACTION_FIELD_PARAM);

            // 押龙march
            TbMarch_action* ptbCatchDragon = CActionBase::AddNewMarch(pstTUser);

            ptbCatchDragon->Set_Sid(ptbReqMarch->m_nSid);
            ptbCatchDragon->Set_Suid(ptbTPlayer->m_nUid);
            ptbCatchDragon->Set_Scid(ptbTPlayer->m_nCid);
            ptbCatchDragon->Set_Tuid(ptbSPlayer->m_nUid);
            ptbCatchDragon->Set_Tal(-1 * ptbSPlayer->m_nUid);
            ptbCatchDragon->Set_Tpos(ptbReqMarch->m_nTpos);
            ptbCatchDragon->Set_Mclass(EN_ACTION_MAIN_CLASS__MARCH);
            ptbCatchDragon->Set_Sclass(EN_ACTION_SEC_CLASS__CATCH_DRAGON);
            ptbCatchDragon->Set_Status(EN_MARCH_STATUS__RETURNING);
            ptbCatchDragon->Set_Btime(CTimeUtils::GetUnixTime());
            ptbCatchDragon->Set_Ctime(ptbReqMarch->m_nCtime);
            for (TUINT32 udwIdx = 0; udwIdx < pstTUser->m_udwMarchNum; ++udwIdx)
            {
                TbMarch_action *ptbAction = &pstTUser->m_atbMarch[udwIdx];
                if (ptbAction->m_nTpos == ptbReqMarch->m_nTpos && ptbAction->m_nSuid == ptbWild->m_nUid &&
                    (ptbAction->m_nStatus == EN_MARCH_STATUS__LOADING || ptbAction->m_nStatus == EN_MARCH_STATUS__DEFENDING))
                {
                    ptbCatchDragon->Set_Ctime(ptbAction->m_bParam[0].m_ddwMarchingTime);
                }
            }
            ptbCatchDragon->Set_Etime(ptbCatchDragon->m_nBtime + ptbCatchDragon->m_nCtime);

            ptbCatchDragon->Set_Sbid(CMapBase::GetBlockIdFromPos(ptbCatchDragon->m_nScid));
            ptbCatchDragon->Set_Tbid(CMapBase::GetBlockIdFromPos(ptbCatchDragon->m_nTpos));

            ptbCatchDragon->m_bPrison_param[0].stDragon = ptbReqMarch->m_bParam[0].m_stDragon;
            ptbCatchDragon->SetFlag(TbMARCH_ACTION_FIELD_PRISON_PARAM);
            ptbCatchDragon->m_bParam[0].m_stDragon = ptbReqMarch->m_bParam[0].m_stDragon;
            ptbCatchDragon->m_bParam[0].m_ddwSourceUserId = ptbTPlayer->m_nUid;
            strncpy(ptbCatchDragon->m_bParam[0].m_szSourceUserName, ptbTPlayer->m_sUin.c_str(), MAX_TABLE_NAME_LEN);
            ptbCatchDragon->m_bParam[0].m_ddwSourceAlliance = ptbTPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET;
            strncpy(ptbCatchDragon->m_bParam[0].m_szSourceAlliance, ptbTPlayer->m_sAlname.c_str(), MAX_TABLE_NAME_LEN);
            strncpy(ptbCatchDragon->m_bParam[0].m_szSourceAlNick, ptbTPlayer->m_sAl_nick_name.c_str(), MAX_TABLE_NAME_LEN);
            ptbCatchDragon->m_bParam[0].m_ddwTargetType = ptbWild->m_nType;
            ptbCatchDragon->SetFlag(TbMARCH_ACTION_FIELD_PARAM);
            TSE_LOG_INFO(pstSession->m_poServLog, ("CaptureDragonWild: gen catch_dragon action[%ld] [seq=%u]",
                ptbCatchDragon->m_nId, pstSession->m_udwSeqNo));

            // prison timer
            TbMarch_action* ptbPrisonTimer = CActionBase::AddNewMarch(pstSUser);

            ptbPrisonTimer->Set_Suid(ptbSPlayer->m_nUid);
            ptbPrisonTimer->Set_Scid(ptbReqMarch->m_nScid);
            ptbPrisonTimer->Set_Tuid(ptbTPlayer->m_nUid);
            ptbPrisonTimer->Set_Mclass(EN_ACTION_MAIN_CLASS__TIMER);
            ptbPrisonTimer->Set_Sclass(EN_ACTION_SEC_CLASS__PRISON_TIMER);
            ptbPrisonTimer->Set_Status(EN_MARCH_STATUS__MARCHING);
            ptbPrisonTimer->Set_Btime(CTimeUtils::GetUnixTime());
            ptbPrisonTimer->Set_Ctime(ptbCatchDragon->m_nCtime);

            TUINT32 udwBasicExcuteTime = CCommonBase::GetGameBasicVal(EN_GAME_BASIC_DRAGON_EXECUTED_TIME);
            TUINT32 udwPrisonTime = CCommonBase::GetGameBasicVal(EN_GAME_BASIC_DRAGON_IMPRISON_TIME)
                + pstAttacker->m_stDragon.m_ddwLevel * CCommonBase::GetGameBasicVal(EN_GAME_BASIC_EX_DRAGON_IMPRISON_TIME_PER_LV);
            if (udwPrisonTime >= pstTUser->m_stPlayerBuffList[EN_BUFFER_INFO_ALTAR_EXECUTE_TIME].m_ddwBuffTotal * 3600)
            {
                udwPrisonTime -= pstTUser->m_stPlayerBuffList[EN_BUFFER_INFO_ALTAR_EXECUTE_TIME].m_ddwBuffTotal * 3600;
            }
            else
            {
                udwPrisonTime = 0;
            }

            if (dwCaptureResult > 0)
            {
                udwBasicExcuteTime = 0;
            }

            TSE_LOG_DEBUG(pstSession->m_poServLog, ("CWarProcess::CaptureDragonWild: [Captor=%ld][Owner=%ld][Excute=%u][Prison=%u][seq=%u]",
                ptbSPlayer->m_nUid, ptbTPlayer->m_nUid,
                udwBasicExcuteTime, udwPrisonTime, pstSession->m_udwSeqNo));

            ptbPrisonTimer->Set_Etime(ptbPrisonTimer->m_nBtime + ptbCatchDragon->m_nCtime + udwPrisonTime + udwBasicExcuteTime);

            ptbPrisonTimer->m_bPrison_param[0].stDragon = pstAttacker->m_stDragon;
            ptbPrisonTimer->m_bPrison_param[0].ddwJoinTimeStamp = ptbPrisonTimer->m_nBtime;
            ptbPrisonTimer->m_bPrison_param[0].ddwReleaseWait = udwPrisonTime + udwBasicExcuteTime;
            ptbPrisonTimer->m_bPrison_param[0].ddwExcuteWait = udwPrisonTime;
            ptbPrisonTimer->m_bPrison_param[0].ddwEscortActionId = ptbCatchDragon->m_nId;
            strncpy(ptbPrisonTimer->m_bPrison_param[0].szSourceUserName, ptbReqMarch->m_bParam[0].m_szSourceUserName, MAX_TABLE_NAME_LEN);
            ptbPrisonTimer->m_bPrison_param[0].szSourceUserName[MAX_TABLE_NAME_LEN - 1] = '\0';
            strncpy(ptbPrisonTimer->m_bPrison_param[0].szTargetUserName, ptbReqMarch->m_bParam[0].m_szTargetUserName, MAX_TABLE_NAME_LEN);
            ptbPrisonTimer->m_bPrison_param[0].szTargetUserName[MAX_TABLE_NAME_LEN - 1] = '\0';

            strncpy(ptbPrisonTimer->m_bPrison_param[0].szTargetCityName, ptbReqMarch->m_bParam[0].m_szTargetCityName, MAX_TABLE_NAME_LEN);
            ptbPrisonTimer->m_bPrison_param[0].szTargetCityName[MAX_TABLE_NAME_LEN - 1] = '\0';

            ptbPrisonTimer->m_bPrison_param[0].ddwResult = EN_PRISON_RESULT_ESCAPE;

            ptbPrisonTimer->SetFlag(TbMARCH_ACTION_FIELD_PRISON_PARAM);

            ptbPrisonTimer->Set_Sid(ptbReqMarch->m_nSid);
            ptbPrisonTimer->Set_Tid(ptbCatchDragon->m_nId);
            ptbPrisonTimer->Set_Tal(-1 * ptbTPlayer->m_nUid);
            ptbPrisonTimer->Set_Tbid(CMapBase::GetBlockIdFromPos(ptbTPlayer->m_nCid));
            strcpy(ptbPrisonTimer->m_bPrison_param[0].szSourceAlNick, ptbReqMarch->m_bParam[0].m_szSourceAlNick);
            strcpy(ptbPrisonTimer->m_bPrison_param[0].szTargetAlNick, ptbReqMarch->m_bParam[0].m_szTargetAlNick);

            ptbPrisonTimer->Set_Tuid(ptbTPlayer->m_nUid);
            ptbPrisonTimer->Set_Tpos(ptbTPlayer->m_nCid);

            ptbSPlayer->Set_Dragon_status(EN_DRAGON_STATUS_BEING_ESCORT);

            //防守方加抓捕次数
            ptbTPlayer->m_bDragon_statistics[0].ddwCaptureDragonNum++;
            ptbTPlayer->SetFlag(TbPLAYER_FIELD_DRAGON_STATISTICS);
            //进攻方加被抓次数
            ptbSPlayer->m_bDragon_statistics[0].ddwMyDragonCapturedNum++;
            ptbSPlayer->SetFlag(TbPLAYER_FIELD_DRAGON_STATISTICS);

            //tips
            CSendMessageBase::AddTips(&pstSession->m_stTargetUser, EN_TIPS_TYPY__CAPTURE_HERO, ptbTPlayer->m_nUid, TRUE, 0, 0, 0, ptbSPlayer->m_sUin.c_str());//抓捕方
            CSendMessageBase::AddTips(&pstSession->m_stSourceUser, EN_TIPS_TYPY__HERO_IS_CAPTURED, ptbSPlayer->m_nUid, TRUE, 0, 0, 0, ptbTPlayer->m_sUin.c_str());//被抓捕方

            SNoticInfo stNoticInfo;
            stNoticInfo.Reset();
            stNoticInfo.SetValue(EN_NOTI_ID__HERO_BE_CAPTURED,
                ptbTPlayer->m_sUin, "",
                0, 0,
                0, 0,
                0, "", 0);
            CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), ptbReqMarch->m_nSid, ptbSPlayer->m_nUid, stNoticInfo);
        }
    }
}

TVOID CWarProcess::CaptureDragonThrone(SSession *pstSession, SBattleNode *pstAttacker, SBattleNode *pstDefender, TUINT32 udwBattleResult)
{
    TbMarch_action *ptbReqMarch = &pstSession->m_stReqMarch;
    TbMap *ptbWild = &pstSession->m_stMapItem;
    SUserInfo *pstSUser = &pstSession->m_stSourceUser;
    TbPlayer *ptbSPlayer = &pstSUser->m_tbPlayer;
    TbAlliance* ptbSAlliance = &pstSUser->m_tbAlliance;
    SUserInfo *pstTUser = &pstSession->m_stTargetUser;
    TbPlayer *ptbTPlayer = &pstTUser->m_tbPlayer;

    if (udwBattleResult == EN_REPORT_RESULT_WIN && pstDefender->bAllTroopDead && pstDefender->m_bIsDragonJoin)
    {
        //进攻方抓捕防守方
        TINT32 dwCaptureResult = CWarProcess::IsCaptureSucc(pstSUser, pstAttacker, pstDefender);
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("CWarProcess::CaptureDragonThrone: [Captor=%ld][Owner=%ld][Result=%d][seq=%u]",
            ptbSPlayer->m_nUid, ptbTPlayer->m_nUid,
            dwCaptureResult, pstSession->m_udwSeqNo));

        pstDefender->m_stDragon.m_ddwCaptured = dwCaptureResult;

        if (dwCaptureResult >= 0)
        {
            // 防守方清龙
            for (TUINT32 udwIdx = 0; udwIdx < pstTUser->m_udwMarchNum; ++udwIdx)
            {
                TbMarch_action *ptbAction = &pstTUser->m_atbMarch[udwIdx];
                if (ptbAction->m_nTpos == ptbWild->m_nId && ptbAction->m_nSuid == ptbTPlayer->m_nUid 
                    && ptbAction->m_nSclass == EN_ACTION_SEC_CLASS__ASSIGN_THRONE
                    && ptbAction->m_nStatus == EN_MARCH_STATUS__DEFENDING)
                {
                    ptbAction->m_bParam[0].m_stDragon.m_ddwLevel = 0;
                    ptbAction->m_bParam[0].m_stDragon.m_ddwCaptured = dwCaptureResult;
                    ptbAction->SetFlag(TbMARCH_ACTION_FIELD_PARAM);
                    break;
                }
            }
            pstDefender->m_stDragon.m_ddwCaptured = dwCaptureResult;

            TINT64 ddwCaptureActionId = 0;
            // 不需要返回时才增加新的action
            if (ptbSAlliance->m_nOid == ptbReqMarch->m_nSuid && ptbReqMarch->m_bParam[0].m_stDragon.m_ddwLevel > 0)
            {
                TbMarch_action* ptbCatchDragon = CActionBase::AddNewMarch(pstSUser);

                ddwCaptureActionId = ptbCatchDragon->m_nId;

                ptbCatchDragon->Set_Sid(ptbReqMarch->m_nSid);
                ptbCatchDragon->Set_Suid(ptbSPlayer->m_nUid);
                ptbCatchDragon->Set_Scid(ptbSPlayer->m_nCid);
                ptbCatchDragon->Set_Tuid(ptbTPlayer->m_nUid);
                ptbCatchDragon->Set_Tal(-1 * ptbTPlayer->m_nUid);
                ptbCatchDragon->Set_Tpos(ptbReqMarch->m_nTpos);
                ptbCatchDragon->Set_Mclass(EN_ACTION_MAIN_CLASS__MARCH);
                ptbCatchDragon->Set_Sclass(EN_ACTION_SEC_CLASS__CATCH_DRAGON);
                ptbCatchDragon->Set_Status(EN_MARCH_STATUS__RETURNING);
                ptbCatchDragon->Set_Btime(CTimeUtils::GetUnixTime());
                ptbCatchDragon->Set_Ctime(ptbReqMarch->m_bParam[0].m_ddwMarchingTime);
                ptbCatchDragon->Set_Etime(ptbCatchDragon->m_nBtime + ptbCatchDragon->m_nCtime);
                ptbCatchDragon->Set_Sbid(CMapBase::GetBlockIdFromPos(ptbCatchDragon->m_nScid));
                ptbCatchDragon->Set_Tbid(CMapBase::GetBlockIdFromPos(ptbCatchDragon->m_nTpos));

                ptbCatchDragon->m_bPrison_param[0].stDragon = pstDefender->m_stDragon;
                ptbCatchDragon->SetFlag(TbMARCH_ACTION_FIELD_PRISON_PARAM);
                ptbCatchDragon->m_bParam[0].m_ddwSourceUserId = ptbSPlayer->m_nUid;
                strncpy(ptbCatchDragon->m_bParam[0].m_szSourceUserName, ptbSPlayer->m_sUin.c_str(), MAX_TABLE_NAME_LEN);
                ptbCatchDragon->m_bParam[0].m_ddwSourceAlliance = ptbSPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET;
                strncpy(ptbCatchDragon->m_bParam[0].m_szSourceAlliance, ptbSPlayer->m_sAlname.c_str(), MAX_TABLE_NAME_LEN);
                strncpy(ptbCatchDragon->m_bParam[0].m_szSourceAlNick, ptbSPlayer->m_sAl_nick_name.c_str(), MAX_TABLE_NAME_LEN);
                ptbCatchDragon->m_bParam[0].m_ddwTargetType = ptbWild->m_nType;
                ptbCatchDragon->SetFlag(TbMARCH_ACTION_FIELD_PARAM);
                TSE_LOG_INFO(pstSession->m_poServLog, ("CaptureDragonThrone: gen catch_dragon action[%ld] [seq=%u]",
                    ptbCatchDragon->m_nId, pstSession->m_udwSeqNo));
            }
            else
            {
                ddwCaptureActionId = ptbReqMarch->m_nId;
                ptbReqMarch->m_bParam[0].m_ddwCaptureDragonFlag = 1;
                ptbReqMarch->m_bParam[0].m_ddwWinFlag = 1;
                ptbReqMarch->SetFlag(TbMARCH_ACTION_FIELD_PARAM);
            }
            
            TbMarch_action* ptbPrisonTimer = CActionBase::AddNewMarch(pstTUser);

            ptbPrisonTimer->Set_Suid(ptbTPlayer->m_nUid);
            ptbPrisonTimer->Set_Scid(ptbReqMarch->m_nTpos);
            ptbPrisonTimer->Set_Tuid(ptbSPlayer->m_nUid);
            ptbPrisonTimer->Set_Mclass(EN_ACTION_MAIN_CLASS__TIMER);
            ptbPrisonTimer->Set_Sclass(EN_ACTION_SEC_CLASS__PRISON_TIMER);
            ptbPrisonTimer->Set_Status(EN_MARCH_STATUS__MARCHING);
            ptbPrisonTimer->Set_Btime(CTimeUtils::GetUnixTime());
            ptbPrisonTimer->Set_Ctime(ptbReqMarch->m_bParam[0].m_ddwMarchingTime);

            TUINT32 udwBasicExcuteTime = CCommonBase::GetGameBasicVal(EN_GAME_BASIC_DRAGON_EXECUTED_TIME);
            TUINT32 udwPrisonTime = CCommonBase::GetGameBasicVal(EN_GAME_BASIC_DRAGON_IMPRISON_TIME)
                + pstDefender->m_stDragon.m_ddwLevel * CCommonBase::GetGameBasicVal(EN_GAME_BASIC_EX_DRAGON_IMPRISON_TIME_PER_LV);
            if (udwPrisonTime >= pstSUser->m_stPlayerBuffList[EN_BUFFER_INFO_ALTAR_EXECUTE_TIME].m_ddwBuffTotal * 3600)
            {
                udwPrisonTime -= pstSUser->m_stPlayerBuffList[EN_BUFFER_INFO_ALTAR_EXECUTE_TIME].m_ddwBuffTotal * 3600;
            }
            else
            {
                udwPrisonTime = 0;
            }

            if (dwCaptureResult > 0)
            {
                udwBasicExcuteTime = 0;
            }

            TSE_LOG_DEBUG(pstSession->m_poServLog, ("CWarProcess::CaptureDragonThrone: [Captor=%ld][Owner=%ld][Excute=%u][Prison=%u][seq=%u]",
                ptbSPlayer->m_nUid, ptbTPlayer->m_nUid,
                udwBasicExcuteTime, udwPrisonTime, pstSession->m_udwSeqNo));

            ptbPrisonTimer->Set_Etime(ptbPrisonTimer->m_nBtime + ptbPrisonTimer->m_nCtime + udwPrisonTime + udwBasicExcuteTime);

            ptbPrisonTimer->m_bPrison_param[0].stDragon = pstDefender->m_stDragon;
            ptbPrisonTimer->m_bPrison_param[0].ddwJoinTimeStamp = ptbPrisonTimer->m_nBtime;
            ptbPrisonTimer->m_bPrison_param[0].ddwReleaseWait = udwPrisonTime + udwBasicExcuteTime;
            ptbPrisonTimer->m_bPrison_param[0].ddwExcuteWait = udwPrisonTime;
            ptbPrisonTimer->m_bPrison_param[0].ddwEscortActionId = ddwCaptureActionId;
            strncpy(ptbPrisonTimer->m_bPrison_param[0].szSourceUserName, ptbTPlayer->m_sUin.c_str(), MAX_TABLE_NAME_LEN);
            ptbPrisonTimer->m_bPrison_param[0].szSourceUserName[MAX_TABLE_NAME_LEN - 1] = '\0';
            strncpy(ptbPrisonTimer->m_bPrison_param[0].szTargetUserName, ptbReqMarch->m_bParam[0].m_szSourceUserName, MAX_TABLE_NAME_LEN);
            ptbPrisonTimer->m_bPrison_param[0].szTargetUserName[MAX_TABLE_NAME_LEN - 1] = '\0';

            strncpy(ptbPrisonTimer->m_bPrison_param[0].szTargetCityName, ptbReqMarch->m_bParam[0].m_szSourceCityName, MAX_TABLE_NAME_LEN);
            ptbPrisonTimer->m_bPrison_param[0].szTargetCityName[MAX_TABLE_NAME_LEN - 1] = '\0';

            ptbPrisonTimer->m_bPrison_param[0].ddwResult = EN_PRISON_RESULT_ESCAPE;

            ptbPrisonTimer->SetFlag(TbMARCH_ACTION_FIELD_PRISON_PARAM);

            ptbPrisonTimer->Set_Sid(ptbReqMarch->m_nSid);
            ptbPrisonTimer->Set_Tal(-1 * ptbSPlayer->m_nUid);
            ptbPrisonTimer->Set_Tid(ddwCaptureActionId);
            ptbPrisonTimer->Set_Tbid(CMapBase::GetBlockIdFromPos(ptbReqMarch->m_nScid));

            strcpy(ptbPrisonTimer->m_bPrison_param[0].szSourceAlNick, ptbReqMarch->m_bParam[0].m_szTargetAlNick);
            strcpy(ptbPrisonTimer->m_bPrison_param[0].szTargetAlNick, ptbReqMarch->m_bParam[0].m_szSourceAlNick);

            ptbPrisonTimer->Set_Tuid(ptbSPlayer->m_nUid);
            ptbPrisonTimer->Set_Tpos(ptbReqMarch->m_nScid);

            ptbTPlayer->Set_Dragon_status(EN_DRAGON_STATUS_BEING_ESCORT);

            //进攻方加抓捕次数
            ptbSPlayer->m_bDragon_statistics[0].ddwCaptureDragonNum++;
            ptbSPlayer->SetFlag(TbPLAYER_FIELD_DRAGON_STATISTICS);
            //防守方加被抓次数
            ptbTPlayer->m_bDragon_statistics[0].ddwMyDragonCapturedNum++;
            ptbTPlayer->SetFlag(TbPLAYER_FIELD_DRAGON_STATISTICS);

            //tips
            CSendMessageBase::AddTips(&pstSession->m_stSourceUser, EN_TIPS_TYPY__CAPTURE_HERO, ptbSPlayer->m_nUid, TRUE, 0, 0, 0, ptbTPlayer->m_sUin.c_str());//抓捕方
            CSendMessageBase::AddTips(&pstSession->m_stTargetUser, EN_TIPS_TYPY__HERO_IS_CAPTURED, ptbTPlayer->m_nUid, TRUE, 0, 0, 0, ptbSPlayer->m_sUin.c_str());//被抓捕方

            SNoticInfo stNoticInfo;
            stNoticInfo.Reset();
            stNoticInfo.SetValue(EN_NOTI_ID__HERO_BE_CAPTURED,
                ptbSPlayer->m_sUin, "",
                0, 0,
                0, 0,
                0, "", 0);
            CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), ptbReqMarch->m_nSid, ptbTPlayer->m_nUid, stNoticInfo);
        }
    }
    else if (udwBattleResult == EN_REPORT_RESULT_LOSE && pstAttacker->bAllTroopDead && pstAttacker->m_bIsDragonJoin)
    {
        //防守方抓捕成功
        TINT32 dwCaptureResult = CWarProcess::IsCaptureSucc(pstTUser, pstDefender, pstAttacker);
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("CWarProcess::CaptureHero: [Captor=%ld][Owner=%ld][Result=%d][seq=%u]",
            ptbTPlayer->m_nUid, ptbSPlayer->m_nUid,
            dwCaptureResult, pstSession->m_udwSeqNo));

        pstAttacker->m_stDragon.m_ddwCaptured = dwCaptureResult;
        
        if (dwCaptureResult >= 0)
        {
            ptbReqMarch->m_bParam[0].m_stDragon.m_ddwCaptured = dwCaptureResult;
            ptbReqMarch->m_bParam[0].m_stDragon.m_ddwLevel = 0;
            ptbReqMarch->SetFlag(TbMARCH_ACTION_FIELD_PARAM);

            // 押龙march
            TbMarch_action* ptbCatchDragon = CActionBase::AddNewMarch(pstTUser);

            ptbCatchDragon->Set_Sid(ptbReqMarch->m_nSid);
            ptbCatchDragon->Set_Suid(ptbTPlayer->m_nUid);
            ptbCatchDragon->Set_Scid(ptbTPlayer->m_nCid);
            ptbCatchDragon->Set_Tuid(ptbSPlayer->m_nUid);
            ptbCatchDragon->Set_Tal(-1 * ptbSPlayer->m_nUid);
            ptbCatchDragon->Set_Tpos(ptbReqMarch->m_nTpos);
            ptbCatchDragon->Set_Mclass(EN_ACTION_MAIN_CLASS__MARCH);
            ptbCatchDragon->Set_Sclass(EN_ACTION_SEC_CLASS__CATCH_DRAGON);
            ptbCatchDragon->Set_Status(EN_MARCH_STATUS__RETURNING);
            ptbCatchDragon->Set_Btime(CTimeUtils::GetUnixTime());
            ptbCatchDragon->Set_Ctime(ptbReqMarch->m_nCtime);
            for (TUINT32 udwIdx = 0; udwIdx < pstTUser->m_udwMarchNum; ++udwIdx)
            {
                TbMarch_action *ptbAction = &pstTUser->m_atbMarch[udwIdx];
                if (ptbAction->m_nTpos == ptbReqMarch->m_nTpos && ptbAction->m_nSuid == ptbWild->m_nUid 
                    && ptbAction->m_nStatus == EN_MARCH_STATUS__DEFENDING)
                {
                    ptbCatchDragon->Set_Ctime(ptbAction->m_bParam[0].m_ddwMarchingTime);
                }
            }

            ptbCatchDragon->Set_Etime(ptbCatchDragon->m_nBtime + ptbCatchDragon->m_nCtime);
            ptbCatchDragon->Set_Sbid(CMapBase::GetBlockIdFromPos(ptbCatchDragon->m_nScid));
            ptbCatchDragon->Set_Tbid(CMapBase::GetBlockIdFromPos(ptbCatchDragon->m_nTpos));

            ptbCatchDragon->m_bPrison_param[0].stDragon = ptbReqMarch->m_bParam[0].m_stDragon;
            ptbCatchDragon->SetFlag(TbMARCH_ACTION_FIELD_PRISON_PARAM);
            ptbCatchDragon->m_bParam[0].m_stDragon = ptbReqMarch->m_bParam[0].m_stDragon;
            ptbCatchDragon->m_bParam[0].m_ddwSourceUserId = ptbTPlayer->m_nUid;
            strncpy(ptbCatchDragon->m_bParam[0].m_szSourceUserName, ptbTPlayer->m_sUin.c_str(), MAX_TABLE_NAME_LEN);
            ptbCatchDragon->m_bParam[0].m_ddwSourceAlliance = ptbTPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET;
            strncpy(ptbCatchDragon->m_bParam[0].m_szSourceAlliance, ptbTPlayer->m_sAlname.c_str(), MAX_TABLE_NAME_LEN);
            strncpy(ptbCatchDragon->m_bParam[0].m_szSourceAlNick, ptbTPlayer->m_sAl_nick_name.c_str(), MAX_TABLE_NAME_LEN);
            ptbCatchDragon->m_bParam[0].m_ddwTargetType = ptbWild->m_nType;
            ptbCatchDragon->SetFlag(TbMARCH_ACTION_FIELD_PARAM);
            TSE_LOG_INFO(pstSession->m_poServLog, ("CaptureDragonWild: gen catch_dragon action[%ld] [seq=%u]",
                ptbCatchDragon->m_nId, pstSession->m_udwSeqNo));

            // prison timer
            TbMarch_action* ptbPrisonTimer = CActionBase::AddNewMarch(pstSUser);

            ptbPrisonTimer->Set_Suid(ptbSPlayer->m_nUid);
            ptbPrisonTimer->Set_Scid(ptbReqMarch->m_nScid);
            ptbPrisonTimer->Set_Tuid(ptbTPlayer->m_nUid);
            ptbPrisonTimer->Set_Mclass(EN_ACTION_MAIN_CLASS__TIMER);
            ptbPrisonTimer->Set_Sclass(EN_ACTION_SEC_CLASS__PRISON_TIMER);
            ptbPrisonTimer->Set_Status(EN_MARCH_STATUS__MARCHING);
            ptbPrisonTimer->Set_Btime(CTimeUtils::GetUnixTime());
            ptbPrisonTimer->Set_Ctime(ptbReqMarch->m_nCtime);

            TUINT32 udwBasicExcuteTime = CCommonBase::GetGameBasicVal(EN_GAME_BASIC_DRAGON_EXECUTED_TIME);
            TUINT32 udwPrisonTime = CCommonBase::GetGameBasicVal(EN_GAME_BASIC_DRAGON_IMPRISON_TIME)
                + pstAttacker->m_stDragon.m_ddwLevel * CCommonBase::GetGameBasicVal(EN_GAME_BASIC_EX_DRAGON_IMPRISON_TIME_PER_LV);
            if (udwPrisonTime >= pstTUser->m_stPlayerBuffList[EN_BUFFER_INFO_ALTAR_EXECUTE_TIME].m_ddwBuffTotal * 3600)
            {
                udwPrisonTime -= pstTUser->m_stPlayerBuffList[EN_BUFFER_INFO_ALTAR_EXECUTE_TIME].m_ddwBuffTotal * 3600;
            }
            else
            {
                udwPrisonTime = 0;
            }

            if (dwCaptureResult > 0)
            {
                udwBasicExcuteTime = 0;
            }

            TSE_LOG_DEBUG(pstSession->m_poServLog, ("CWarProcess::CaptureDragonThrone: [Captor=%ld][Owner=%ld][Excute=%u][Prison=%u][seq=%u]",
                ptbSPlayer->m_nUid, ptbTPlayer->m_nUid,
                udwBasicExcuteTime, udwPrisonTime, pstSession->m_udwSeqNo));
            ptbPrisonTimer->Set_Etime(ptbPrisonTimer->m_nBtime + ptbPrisonTimer->m_nCtime + udwPrisonTime + udwBasicExcuteTime);

            ptbPrisonTimer->m_bPrison_param[0].stDragon = pstAttacker->m_stDragon;
            ptbPrisonTimer->m_bPrison_param[0].ddwJoinTimeStamp = ptbPrisonTimer->m_nBtime;
            ptbPrisonTimer->m_bPrison_param[0].ddwReleaseWait = udwPrisonTime + udwBasicExcuteTime;
            ptbPrisonTimer->m_bPrison_param[0].ddwExcuteWait = udwPrisonTime;
            ptbPrisonTimer->m_bPrison_param[0].ddwEscortActionId = ptbCatchDragon->m_nId;
            strncpy(ptbPrisonTimer->m_bPrison_param[0].szSourceUserName, ptbReqMarch->m_bParam[0].m_szSourceUserName, MAX_TABLE_NAME_LEN);
            ptbPrisonTimer->m_bPrison_param[0].szSourceUserName[MAX_TABLE_NAME_LEN - 1] = '\0';
            strncpy(ptbPrisonTimer->m_bPrison_param[0].szTargetUserName, ptbReqMarch->m_bParam[0].m_szTargetUserName, MAX_TABLE_NAME_LEN);
            ptbPrisonTimer->m_bPrison_param[0].szTargetUserName[MAX_TABLE_NAME_LEN - 1] = '\0';

            strncpy(ptbPrisonTimer->m_bPrison_param[0].szTargetCityName, ptbReqMarch->m_bParam[0].m_szTargetCityName, MAX_TABLE_NAME_LEN);
            ptbPrisonTimer->m_bPrison_param[0].szTargetCityName[MAX_TABLE_NAME_LEN - 1] = '\0';

            ptbPrisonTimer->m_bPrison_param[0].ddwResult = EN_PRISON_RESULT_ESCAPE;

            ptbPrisonTimer->SetFlag(TbMARCH_ACTION_FIELD_PRISON_PARAM);

            ptbPrisonTimer->Set_Sid(ptbReqMarch->m_nSid);
            ptbPrisonTimer->Set_Tid(ptbCatchDragon->m_nId);
            ptbPrisonTimer->Set_Tal(-1 * ptbTPlayer->m_nUid);
            ptbPrisonTimer->Set_Tbid(CMapBase::GetBlockIdFromPos(ptbTPlayer->m_nCid));
            strcpy(ptbPrisonTimer->m_bPrison_param[0].szSourceAlNick, ptbReqMarch->m_bParam[0].m_szSourceAlNick);
            strcpy(ptbPrisonTimer->m_bPrison_param[0].szTargetAlNick, ptbReqMarch->m_bParam[0].m_szTargetAlNick);

            ptbPrisonTimer->Set_Tuid(ptbTPlayer->m_nUid);
            ptbPrisonTimer->Set_Tpos(ptbTPlayer->m_nCid);

            ptbSPlayer->Set_Dragon_status(EN_DRAGON_STATUS_BEING_ESCORT);

            //防守方加抓捕次数
            ptbTPlayer->m_bDragon_statistics[0].ddwCaptureDragonNum++;
            ptbTPlayer->SetFlag(TbPLAYER_FIELD_DRAGON_STATISTICS);
            //进攻方加被抓次数
            ptbSPlayer->m_bDragon_statistics[0].ddwMyDragonCapturedNum++;
            ptbSPlayer->SetFlag(TbPLAYER_FIELD_DRAGON_STATISTICS);

            //tips
            CSendMessageBase::AddTips(&pstSession->m_stTargetUser, EN_TIPS_TYPY__CAPTURE_HERO, ptbTPlayer->m_nUid, TRUE, 0, 0, 0, ptbSPlayer->m_sUin.c_str());//抓捕方
            CSendMessageBase::AddTips(&pstSession->m_stSourceUser, EN_TIPS_TYPY__HERO_IS_CAPTURED, ptbSPlayer->m_nUid, TRUE, 0, 0, 0, ptbTPlayer->m_sUin.c_str());//被抓捕方

            SNoticInfo stNoticInfo;
            stNoticInfo.Reset();
            stNoticInfo.SetValue(EN_NOTI_ID__HERO_BE_CAPTURED,
                ptbTPlayer->m_sUin, "",
                0, 0,
                0, 0,
                0, "", 0);
            CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), ptbReqMarch->m_nSid, ptbSPlayer->m_nUid, stNoticInfo);
        }
    }
}

TINT32 CWarProcess::IsCaptureSucc(SUserInfo* pstUserA, SBattleNode *pstWinner, SBattleNode *pstLoser)
{
    CGameInfo *pstGameInfo = CGameInfo::GetInstance();
    SMarchDragonInfo* pstDragonB = &pstLoser->m_stDragon;

    if (pstUserA->m_stPlayerBuffList[EN_BUFFER_INFO_JAIL_LIMIT].m_ddwBuffTotal == 0)
    {
        return -1;
    }

    TUINT32 udwPrisonCount = 0;
    for (TUINT32 udwIdx = 0; udwIdx < pstUserA->m_udwPassiveMarchNum; ++udwIdx)
    {
        if (pstUserA->m_aucPassiveMarchFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }
        if (pstUserA->m_atbPassiveMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__PRISON_TIMER)
        {
            udwPrisonCount++;
        }
    }
    if (udwPrisonCount >= pstUserA->m_stPlayerBuffList[EN_BUFFER_INFO_JAIL_LIMIT].m_ddwBuffTotal)
    {
        return -2;
    }

    if (pstDragonB->m_ddwLevel >= pstGameInfo->m_oJsonRoot["game_dragon"]["a17"].asInt())
    {
        return 0;
    }
    else if (pstDragonB->m_ddwLevel >= pstGameInfo->m_oJsonRoot["game_dragon"]["a16"].asInt())
    {
        return 1;
    }

    return -3;
}

TVOID CWarProcess::SetBattleNodeForAttackCity(SSession *pstSession, SBattleNode *pstAttacker, SBattleNode *pstDefender)
{
    TbMarch_action *ptbReqMarch = &pstSession->m_stReqMarch;
    SUserInfo *pstSUser = &pstSession->m_stSourceUser;
    SUserInfo *pstTUser = &pstSession->m_stTargetUser;
    TbMap *ptbWild = &pstSession->m_stMapItem;

    pstAttacker->m_ptbMainAttackAction = ptbReqMarch;

    pstDefender->m_pstDefenderCity = &pstTUser->m_stCityInfo;
    pstDefender->m_ptbDefendPlayer = &pstTUser->m_tbPlayer;

    pstDefender->m_ptbWild = ptbWild;

    //进攻方军队
    CWarProcess::SetLeadAttackerArmy(ptbReqMarch, pstSUser, pstAttacker);

    //联盟report
    CToolBase::AddUserToMailReceiverList(pstSUser->m_adwMailSendUidList, pstSUser->m_udwMailSendNum,
        CToolBase::GetAllianceUserReportKey(ptbReqMarch->m_bParam[0].m_ddwSourceAlliance));

    //防守方军队
    CWarProcess::SetCityDefenderArmy(pstTUser, pstDefender);
    CWarProcess::SetCityReinforceArmy(pstTUser, pstDefender);

    //联盟report
    CToolBase::AddUserToMailReceiverList(pstTUser->m_adwMailSendUidList, pstTUser->m_udwMailSendNum,
        CToolBase::GetAllianceUserReportKey(ptbReqMarch->m_bParam[0].m_ddwTargetAlliance));
}

TVOID CWarProcess::SetBattleNodeForAttackOccupy(SSession *pstSession, SBattleNode *pstAttacker, SBattleNode *pstDefender)
{
    TbMarch_action *ptbReqMarch = &pstSession->m_stReqMarch;
    SUserInfo *pstSUser = &pstSession->m_stSourceUser;
    SUserInfo *pstTUser = &pstSession->m_stTargetUser;
    TbMap *ptbWild = &pstSession->m_stMapItem;

    pstAttacker->m_ptbMainAttackAction = ptbReqMarch;
    /*
    pstDefender->m_pstDefenderCity = CCityBase::GetCityInfo(pstTUser->m_astCityInfo, pstTUser->m_udwCityNum, ptbReqMarch->m_nTcid, EN_WILD_TYPE__NORMAL);
    if (pstDefender->m_pstDefenderCity)
    {
        pstDefender->m_ptbDefendPlayer = &pstTUser->m_tbPlayer;
    }
    */
    pstDefender->m_pstDefenderCity = &pstTUser->m_stCityInfo;
    pstDefender->m_ptbDefendPlayer = &pstTUser->m_tbPlayer;
    pstDefender->m_ptbWild = ptbWild;

    //进攻方军队
    CWarProcess::SetLeadAttackerArmy(ptbReqMarch, pstSUser, pstAttacker);

    CToolBase::AddUserToMailReceiverList(pstSUser->m_adwMailSendUidList, pstSUser->m_udwMailSendNum,
        CToolBase::GetAllianceUserReportKey(ptbReqMarch->m_bParam[0].m_ddwSourceAlliance));

    //防守方军队

    CWarProcess::SetWildDefenderArmy(pstTUser, pstDefender, ptbWild->m_nId);
    CToolBase::AddUserToMailReceiverList(pstTUser->m_adwMailSendUidList, pstTUser->m_udwMailSendNum,
        CToolBase::GetAllianceUserReportKey(ptbReqMarch->m_bParam[0].m_ddwTargetAlliance));
}

TVOID CWarProcess::SetBattleNodeForAttackCamp(SSession *pstSession, SBattleNode *pstAttacker, SBattleNode *pstDefender)
{
    TbMarch_action *ptbReqMarch = &pstSession->m_stReqMarch;
    SUserInfo *pstSUser = &pstSession->m_stSourceUser;
    SUserInfo *pstTUser = &pstSession->m_stTargetUser;
    TbMap *ptbWild = &pstSession->m_stMapItem;

    pstAttacker->m_ptbMainAttackAction = ptbReqMarch;
    /*
    pstDefender->m_pstDefenderCity = CCityBase::GetCityInfo(pstTUser->m_astCityInfo, pstTUser->m_udwCityNum, ptbReqMarch->m_nTcid, EN_WILD_TYPE__NORMAL);
    if (pstDefender->m_pstDefenderCity)
    {
    pstDefender->m_ptbDefendPlayer = &pstTUser->m_tbPlayer;
    }
    */
    pstDefender->m_pstDefenderCity = &pstTUser->m_stCityInfo;
    pstDefender->m_ptbDefendPlayer = &pstTUser->m_tbPlayer;
    pstDefender->m_ptbWild = ptbWild;

    //进攻方军队
    CWarProcess::SetLeadAttackerArmy(ptbReqMarch, pstSUser, pstAttacker);

    CToolBase::AddUserToMailReceiverList(pstSUser->m_adwMailSendUidList, pstSUser->m_udwMailSendNum,
        CToolBase::GetAllianceUserReportKey(ptbReqMarch->m_bParam[0].m_ddwSourceAlliance));

    //防守方军队

    CWarProcess::SetWildCampArmy(pstTUser, pstDefender, ptbWild->m_nId);
    CToolBase::AddUserToMailReceiverList(pstTUser->m_adwMailSendUidList, pstTUser->m_udwMailSendNum,
        CToolBase::GetAllianceUserReportKey(ptbReqMarch->m_bParam[0].m_ddwTargetAlliance));
}

TVOID CWarProcess::SetBattleNodeForAttackOccupyLair(SSession *pstSession, SBattleNode *pstAttacker, SBattleNode *pstDefender)
{
    TbMarch_action *ptbReqMarch = &pstSession->m_stReqMarch;
    SUserInfo *pstSUser = &pstSession->m_stSourceUser;
    SUserInfo *pstTUser = &pstSession->m_stTargetUser;
    TbMap *ptbWild = &pstSession->m_stMapItem;

    pstAttacker->m_ptbMainAttackAction = ptbReqMarch;
    /*
    pstDefender->m_pstDefenderCity = CCityBase::GetCityInfo(pstTUser->m_astCityInfo, pstTUser->m_udwCityNum, ptbReqMarch->m_nTcid, EN_WILD_TYPE__NORMAL);
    if (pstDefender->m_pstDefenderCity)
    {
    pstDefender->m_ptbDefendPlayer = &pstTUser->m_tbPlayer;
    }
    */
    pstDefender->m_pstDefenderCity = &pstTUser->m_stCityInfo;
    pstDefender->m_ptbDefendPlayer = &pstTUser->m_tbPlayer;
    pstDefender->m_ptbWild = ptbWild;

    //进攻方军队
    CWarProcess::SetLeadAttackerArmy(ptbReqMarch, pstSUser, pstAttacker);

    CToolBase::AddUserToMailReceiverList(pstSUser->m_adwMailSendUidList, pstSUser->m_udwMailSendNum,
        CToolBase::GetAllianceUserReportKey(ptbReqMarch->m_bParam[0].m_ddwSourceAlliance));

    //防守方军队
    CWarProcess::SetOccupyLairDefenderArmy(pstTUser, pstDefender, ptbReqMarch, ptbWild);

    CToolBase::AddUserToMailReceiverList(pstTUser->m_adwMailSendUidList, pstTUser->m_udwMailSendNum,
        CToolBase::GetAllianceUserReportKey(ptbReqMarch->m_bParam[0].m_ddwTargetAlliance));
}

TVOID CWarProcess::SetBattleNodeForAttackWild(SSession *pstSession, SBattleNode *pstAttacker, SBattleNode *pstDefender)
{
    TbMarch_action *ptbReqMarch = &pstSession->m_stReqMarch;
    SUserInfo *pstSUser = &pstSession->m_stSourceUser;
    TbMap *ptbWild = &pstSession->m_stMapItem;

    pstAttacker->m_ptbMainAttackAction = ptbReqMarch;

    pstDefender->m_ptbWild = ptbWild;

    //进攻方军队
    CWarProcess::SetLeadAttackerArmy(ptbReqMarch, pstSUser, pstAttacker);

    CToolBase::AddUserToMailReceiverList(pstSUser->m_adwMailSendUidList, pstSUser->m_udwMailSendNum,
        CToolBase::GetAllianceUserReportKey(ptbReqMarch->m_bParam[0].m_ddwSourceAlliance));

    //防守方军队
    CWarProcess::SetWildDefenderNode(ptbWild, pstDefender);
}

TVOID CWarProcess::SetBattleNodeForRallyCity(SSession *pstSession, SBattleNode *pstAttacker, SBattleNode *pstDefender)
{
    TbMarch_action *ptbReqMarch = &pstSession->m_stReqMarch;
    SUserInfo *pstSUser = &pstSession->m_stSourceUser;
    SUserInfo *pstTUser = &pstSession->m_stTargetUser;
    TbMap *ptbWild = &pstSession->m_stMapItem;

    pstAttacker->m_ptbMainAttackAction = ptbReqMarch;

    /*
    pstDefender->m_pstDefenderCity = CCityBase::GetCityInfo(pstTUser->m_astCityInfo, pstTUser->m_udwCityNum, ptbReqMarch->m_nTcid);
    if(pstDefender->m_pstDefenderCity)
    {
        pstDefender->m_ptbDefendPlayer = &pstTUser->m_tbPlayer;
    }
    */
    pstDefender->m_pstDefenderCity = &pstTUser->m_stCityInfo;
    pstDefender->m_ptbDefendPlayer = &pstTUser->m_tbPlayer;
    pstDefender->m_ptbWild = ptbWild;

    //进攻方军队
    CWarProcess::SetLeadAttackerArmy(ptbReqMarch, pstSUser, pstAttacker);
    CWarProcess::SetRallyReinforceArmy(pstSUser, ptbReqMarch, pstAttacker);

    CToolBase::AddUserToMailReceiverList(pstSUser->m_adwMailSendUidList, pstSUser->m_udwMailSendNum,
        CToolBase::GetAllianceUserReportKey(ptbReqMarch->m_bParam[0].m_ddwSourceAlliance));

    //防守方军队
    CWarProcess::SetCityDefenderArmy(pstTUser, pstDefender);
    CWarProcess::SetCityReinforceArmy(pstTUser, pstDefender);

    CToolBase::AddUserToMailReceiverList(pstTUser->m_adwMailSendUidList, pstTUser->m_udwMailSendNum,
        CToolBase::GetAllianceUserReportKey(ptbReqMarch->m_bParam[0].m_ddwTargetAlliance));
}

TVOID CWarProcess::SetBattleNodeForRallyThrone(SSession * pstSession, SBattleNode * pstAttacker, SBattleNode * pstDefender)
{
    TbMarch_action *ptbReqMarch = &pstSession->m_stReqMarch;
    SUserInfo *pstSUser = &pstSession->m_stSourceUser;
    SUserInfo *pstTUser = &pstSession->m_stTargetUser;
    TbMap *ptbWild = &pstSession->m_stMapItem;

    pstAttacker->m_ptbMainAttackAction = ptbReqMarch;
    pstDefender->m_ptbWild = ptbWild;

    //进攻方军队
    CWarProcess::SetLeadAttackerArmy(ptbReqMarch, pstSUser, pstAttacker);
    CWarProcess::SetRallyReinforceArmy(pstSUser, ptbReqMarch, pstAttacker);

    CToolBase::AddUserToMailReceiverList(pstSUser->m_adwMailSendUidList, pstSUser->m_udwMailSendNum,
        CToolBase::GetAllianceUserReportKey(ptbReqMarch->m_bParam[0].m_ddwSourceAlliance));

    //防守方军队
    CWarProcess::SetLeaderAssign(pstTUser, pstDefender, ptbWild->m_nId);
    CWarProcess::SetThroneReinforceArmy(pstTUser, pstDefender, ptbWild->m_nId);

    //发给王座拥有者
    CToolBase::AddUserToMailReceiverList(pstTUser->m_adwMailSendUidList, pstTUser->m_udwMailSendNum,
        ptbWild->m_nUid);
    CToolBase::AddUserToMailReceiverList(pstTUser->m_adwMailSendUidList, pstTUser->m_udwMailSendNum,
        CToolBase::GetAllianceUserReportKey(ptbReqMarch->m_bParam[0].m_ddwTargetAlliance));
}

TINT32 CWarProcess::SetLeadAttackerArmy(TbMarch_action *ptbReqMarch, SUserInfo *pstUserInfo, SBattleNode *pstAttacker)
{
    if(ptbReqMarch->m_bParam[0].m_stDragon.m_ddwLevel > 0)
    {
        pstAttacker->m_bIsDragonJoin = TRUE;
        pstAttacker->m_stDragon = ptbReqMarch->m_bParam[0].m_stDragon;
    }

    pstAttacker->m_stKnight = ptbReqMarch->m_bParam[0].m_stKnight;

    CWarBase::AddAttackArmy(ptbReqMarch, pstAttacker);

    TBOOL bIsRallyAttack = FALSE;
    if (ptbReqMarch->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR
        || ptbReqMarch->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR_THRONE)
    {
        bIsRallyAttack = TRUE;
    }
    SPlayerBuffInfo stBuff;
    stBuff.Reset();
    if(pstAttacker->m_bIsDragonJoin)
    {
        CBufferBase::GenBattleBuff(&pstUserInfo->m_stPlayerBuffList, &stBuff, bIsRallyAttack, FALSE, ptbReqMarch->m_bParam[0].m_stKnight.ddwLevel);
    }
    else
    {
        CBufferBase::GenBattleBuff(&pstUserInfo->m_stBuffWithoutDragon, &stBuff, bIsRallyAttack, FALSE, ptbReqMarch->m_bParam[0].m_stKnight.ddwLevel);
    }

    CWarBase::SetBattleBuff(&stBuff, pstAttacker, FALSE, bIsRallyAttack);

    CToolBase::AddUserToMailReceiverList(pstUserInfo->m_adwMailSendUidList, pstUserInfo->m_udwMailSendNum, ptbReqMarch->m_nSuid);

    return 0;
}

TINT32 CWarProcess::SetWildDefenderArmy(SUserInfo *pstUserInfo, SBattleNode *pstDefender, TUINT32 udwWildPos)
{
    TINT32 dwActionIdx = CWarBase::GetWildAction(pstUserInfo->m_atbMarch, pstUserInfo->m_udwMarchNum, udwWildPos);
    if(dwActionIdx >= 0)
    {
        TbMarch_action *ptbMarch = &pstUserInfo->m_atbMarch[dwActionIdx];
        SPlayerBuffInfo stBuff;
        stBuff.Reset();
        if(ptbMarch->m_bParam[0].m_stDragon.m_ddwLevel > 0)
        {
            pstDefender->m_bIsDragonJoin = TRUE;
            pstDefender->m_stDragon = ptbMarch->m_bParam[0].m_stDragon;
            CBufferBase::GenBattleBuff(&pstUserInfo->m_stPlayerBuffList, &stBuff, FALSE, FALSE, ptbMarch->m_bParam[0].m_stKnight.ddwLevel);
        }
        else
        {
            CBufferBase::GenBattleBuff(&pstUserInfo->m_stBuffWithoutDragon, &stBuff, FALSE, FALSE, ptbMarch->m_bParam[0].m_stKnight.ddwLevel);
        }

        CWarBase::SetBattleBuff(&stBuff, pstDefender);

        CWarBase::AddWildOccupyArmy(ptbMarch, pstDefender);
        pstDefender->m_stKnight = ptbMarch->m_bParam[0].m_stKnight;

        // 添加盟友idlist，用于发送邮件
        CToolBase::AddUserToMailReceiverList(pstUserInfo->m_adwMailSendUidList, pstUserInfo->m_udwMailSendNum, ptbMarch->m_nSuid);
    }

    return 0;
}

TINT32 CWarProcess::SetWildCampArmy(SUserInfo *pstUserInfo, SBattleNode *pstDefender, TUINT32 udwWildPos)
{
    TINT32 dwActionIdx = CWarBase::GetWildAction(pstUserInfo->m_atbMarch, pstUserInfo->m_udwMarchNum, udwWildPos);
    if (dwActionIdx >= 0)
    {
        TbMarch_action *ptbMarch = &pstUserInfo->m_atbMarch[dwActionIdx];
        SPlayerBuffInfo stBuff;
        stBuff.Reset();
        if (ptbMarch->m_bParam[0].m_stDragon.m_ddwLevel > 0)
        {
            pstDefender->m_bIsDragonJoin = TRUE;
            pstDefender->m_stDragon = ptbMarch->m_bParam[0].m_stDragon;
            CBufferBase::GenBattleBuff(&pstUserInfo->m_stPlayerBuffList, &stBuff, FALSE, FALSE, ptbMarch->m_bParam[0].m_stKnight.ddwLevel);
        }
        else
        {
            CBufferBase::GenBattleBuff(&pstUserInfo->m_stBuffWithoutDragon, &stBuff, FALSE, FALSE, ptbMarch->m_bParam[0].m_stKnight.ddwLevel);
        }

        CWarBase::SetBattleBuff(&stBuff, pstDefender);

        CWarBase::AddWildCampArmy(ptbMarch, pstDefender);
        pstDefender->m_stKnight = ptbMarch->m_bParam[0].m_stKnight;

        // 添加盟友idlist，用于发送邮件
        CToolBase::AddUserToMailReceiverList(pstUserInfo->m_adwMailSendUidList, pstUserInfo->m_udwMailSendNum, ptbMarch->m_nSuid);
    }

    return 0;
}

TINT32 CWarProcess::SetWildDefenderNode(TbMap *ptbWildDefender, SBattleNode *pstDefender)
{
    CWarBase::AddWildArmy(ptbWildDefender, pstDefender);

    return 0;
}

TINT32 CWarProcess::SetCityDefenderArmy(SUserInfo *pstUserInfo, SBattleNode *pstDefender)
{
    if (pstUserInfo->m_tbPlayer.m_nDragon_status == EN_DRAGON_STATUS_NORMAL && pstUserInfo->m_tbPlayer.m_nHas_dragon > 0)
    {
        pstDefender->m_bIsDragonJoin = TRUE;
        pstDefender->m_stDragon.m_ddwIconId = pstUserInfo->m_tbPlayer.m_nDragon_avatar;
        pstDefender->m_stDragon.m_ddwLevel = pstUserInfo->m_tbPlayer.m_nDragon_level;
        strcpy(pstDefender->m_stDragon.m_szName, pstUserInfo->m_tbPlayer.m_sDragon_name.c_str());
    }

    CWarBase::AddCityArmy(pstDefender->m_pstDefenderCity, pstDefender);

    SPlayerBuffInfo stBuff;
    stBuff.Reset();
    if (pstDefender->m_bIsDragonJoin)
    {
        CBufferBase::GenBattleBuff(&pstUserInfo->m_stPlayerBuffList, &stBuff);
    }
    else
    {
        CBufferBase::GenBattleBuff(&pstUserInfo->m_stBuffWithoutDragon, &stBuff);
    }

    CWarBase::SetBattleBuff(&stBuff, pstDefender, TRUE);

    CToolBase::AddUserToMailReceiverList(pstUserInfo->m_adwMailSendUidList, pstUserInfo->m_udwMailSendNum, pstUserInfo->m_tbPlayer.m_nUid);

    return 0;
}

TINT32 CWarProcess::SetCityReinforceArmy(SUserInfo *pstUserInfo, SBattleNode *pstDefender)
{
    TbMarch_action *ptbReinforce = NULL;

    for(TUINT32 udwIdx = 0; udwIdx < pstUserInfo->m_udwPassiveMarchNum && pstDefender->m_udwEncampNum < MAX_REINFORCE_NUM; udwIdx++)
    {
        ptbReinforce = &pstUserInfo->m_atbPassiveMarch[udwIdx];
        if(ptbReinforce->m_nTpos == pstDefender->m_pstDefenderCity->m_stTblData.m_nPos
        && ptbReinforce->m_nMclass == EN_ACTION_MAIN_CLASS__MARCH
        && ptbReinforce->m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_NORMAL
        && ptbReinforce->m_nStatus == EN_MARCH_STATUS__DEFENDING)
        {
            CWarBase::AddReinforceArmy(ptbReinforce, pstDefender);
            CToolBase::AddUserToMailReceiverList(pstUserInfo->m_adwMailSendUidList, pstUserInfo->m_udwMailSendNum, ptbReinforce->m_nSuid);
        }
    }

    return 0;
}

TINT32 CWarProcess::SetThroneReinforceArmy(SUserInfo *pstUser, SBattleNode *pstDefender, TUINT32 udwWildPos)
{
    if (pstUser->m_tbPlayer.m_nUid == 0)
    {
        return 0;
    }
    TbMarch_action* ptbReinforce = NULL;
    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
    {
        if (pstUser->m_aucMarchFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }

        ptbReinforce = &pstUser->m_atbMarch[udwIdx];
        if(ptbReinforce->m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_THRONE
            && ptbReinforce->m_nStatus == EN_MARCH_STATUS__DEFENDING
            && ptbReinforce->m_nTpos == udwWildPos)
        {
            CWarBase::AddRallyReinforceArmy(ptbReinforce, pstDefender);

            // 添加盟友idlist，用于发送邮件
            CToolBase::AddUserToMailReceiverList(pstUser->m_adwMailSendUidList, pstUser->m_udwMailSendNum, ptbReinforce->m_nSuid);
        }
    }

    return 0;
}

TINT32 CWarProcess::SetOccupyLairDefenderArmy(SUserInfo *pstUser, SBattleNode *pstDefender, TbMarch_action *ptbReqMarch, TbMap *ptbWild)
{
    TUINT32 udwWildPos = ptbWild->m_nId;

    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
    {
        TbMarch_action *ptbAction = &pstUser->m_atbMarch[udwIdx];
        if (ptbAction->m_nMclass == EN_ACTION_MAIN_CLASS__MARCH && ptbAction->m_nSclass == EN_ACTION_SEC_CLASS__OCCUPY &&
            ptbAction->m_nTpos == udwWildPos &&
            (ptbAction->m_nStatus == EN_MARCH_STATUS__PRE_LOADING || ptbAction->m_nStatus == EN_MARCH_STATUS__LOADING))
        {
            if (ptbWild->m_nUid == ptbAction->m_nSuid) // main defender
            {
                if (ptbAction->m_bParam[0].m_stDragon.m_ddwLevel > 0)
                {
                    pstDefender->m_bIsDragonJoin = TRUE;
                    pstDefender->m_stDragon = ptbAction->m_bParam[0].m_stDragon;
                }
                pstDefender->m_stKnight = ptbAction->m_bParam[0].m_stKnight;

                ptbReqMarch->Set_Tuid(ptbAction->m_nSuid);
                ptbReqMarch->m_bParam[0].m_ddwTargetUserId = ptbAction->m_nSuid;
                strncpy(ptbReqMarch->m_bParam[0].m_szTargetUserName, ptbAction->m_bParam[0].m_szSourceUserName, MAX_TABLE_NAME_LEN);
                ptbReqMarch->SetFlag(TbMARCH_ACTION_FIELD_PARAM);
            }

            CWarBase::AddReinforceArmy(ptbAction, pstDefender);
            CToolBase::AddUserToMailReceiverList(pstUser->m_adwMailSendUidList, pstUser->m_udwMailSendNum, ptbAction->m_nSuid);
        }
    }

    SPlayerBuffInfo stBuff;
    stBuff.Reset();
    if (pstDefender->m_bIsDragonJoin)
    {
        CBufferBase::GenBattleBuff(&pstUser->m_stPlayerBuffList, &stBuff);
    }
    else
    {
        CBufferBase::GenBattleBuff(&pstUser->m_stBuffWithoutDragon, &stBuff);
    }
    CWarBase::SetBattleBuff(&stBuff, pstDefender, TRUE);

    return 0;
}

TINT32 CWarProcess::SetRallyReinforceArmy(SUserInfo *pstUser, TbMarch_action* ptbRallyWar, SBattleNode *pstBattleNode)
{
    if(ptbRallyWar->m_nSclass != EN_ACTION_SEC_CLASS__RALLY_WAR
        && ptbRallyWar->m_nSclass != EN_ACTION_SEC_CLASS__RALLY_WAR_THRONE)
    {
        return 0;
    }

    TbMarch_action *ptbRallyReinforce = NULL;

    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwPassiveMarchNum; udwIdx++)
    {
        ptbRallyReinforce = &pstUser->m_atbPassiveMarch[udwIdx];
        if(ptbRallyReinforce->m_nTid == ptbRallyWar->m_nId
            && ptbRallyReinforce->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_REINFORCE
            && ptbRallyReinforce->m_nStatus == EN_MARCH_STATUS__PREPARING)
        {
            CWarBase::AddRallyReinforceArmy(ptbRallyReinforce, pstBattleNode);

            // 添加盟友idlist，用于发送邮件
            CToolBase::AddUserToMailReceiverList(pstUser->m_adwMailSendUidList, pstUser->m_udwMailSendNum, ptbRallyReinforce->m_nSuid);
        }
//         else if (ptbRallyReinforce->m_nTid == ptbRallyWar->m_nId
//             && ptbRallyReinforce->m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_THRONE
//             && ptbRallyReinforce->m_nStatus == EN_MARCH_STATUS__PREPARING)
//         {
//             CWarBase::AddRallyReinforceArmy(ptbRallyReinforce, pstBattleNode);
// 
//             // 添加盟友idlist，用于发送邮件
//             CToolBase::AddUserToMailReceiverList(pstUser->m_adwMailSendUidList, pstUser->m_udwMailSendNum, ptbRallyReinforce->m_nSuid);
//         }
    }

    return 0;
}

TINT32 CWarProcess::SetLeaderAssign(SUserInfo *pstUserInfo, SBattleNode *pstDefender, TUINT32 udwWildPos)
{
    if (pstUserInfo->m_tbPlayer.m_nUid == 0)
    {
        return 0;
    }
    TbMarch_action* ptbMainAssign = NULL;
    TBOOL bIsFind = FALSE;
    for(TUINT32 udwIdx = 0; udwIdx < pstUserInfo->m_udwMarchNum; ++udwIdx)
    {
        if(pstUserInfo->m_aucMarchFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }

        ptbMainAssign = &pstUserInfo->m_atbMarch[udwIdx];
        if(ptbMainAssign->m_nSclass == EN_ACTION_SEC_CLASS__ASSIGN_THRONE
            && ptbMainAssign->m_nStatus == EN_MARCH_STATUS__DEFENDING
            && ptbMainAssign->m_nTpos == udwWildPos
            && ptbMainAssign->m_nSuid == pstUserInfo->m_tbPlayer.m_nUid)
        {
            pstDefender->m_stKnight = ptbMainAssign->m_bParam[0].m_stKnight;

            if (ptbMainAssign->m_bParam[0].m_stDragon.m_ddwLevel > 0)
            {
                pstDefender->m_bIsDragonJoin = TRUE;
                pstDefender->m_stDragon = ptbMainAssign->m_bParam[0].m_stDragon;
            }

            CWarBase::AddRallyReinforceArmy(ptbMainAssign, pstDefender);

            // 添加盟友idlist，用于发送邮件
            CToolBase::AddUserToMailReceiverList(pstUserInfo->m_adwMailSendUidList, pstUserInfo->m_udwMailSendNum, ptbMainAssign->m_nSuid);

            bIsFind = TRUE;
            break;
        }
    }

    SPlayerBuffInfo stBuff;
    stBuff.Reset();
    if (bIsFind == FALSE)
    {
        CBufferBase::GenBattleBuff(&pstUserInfo->m_stBuffWithoutDragon, &stBuff, FALSE, FALSE, 0);
        CWarBase::SetBattleBuff(&stBuff, pstDefender);
    }
    else
    {
        if (pstDefender->m_bIsDragonJoin)
        {
            CBufferBase::GenBattleBuff(&pstUserInfo->m_stPlayerBuffList, &stBuff, FALSE, FALSE, pstDefender->m_stKnight.ddwLevel);
        }
        else
        {
            CBufferBase::GenBattleBuff(&pstUserInfo->m_stBuffWithoutDragon, &stBuff, FALSE, FALSE, pstDefender->m_stKnight.ddwLevel);
        }
        CWarBase::SetBattleBuff(&stBuff, pstDefender);
    }
    return 0;
}

TVOID CWarProcess::ComputeAttackCityResult(SSession *pstSession, SBattleNode *pstAttacker, SBattleNode *pstDefender, TUINT32 udwBattleResult)
{
    TbMarch_action *ptbReqMarch = &pstSession->m_stReqMarch;
    SUserInfo *pstSUser = &pstSession->m_stSourceUser;
    TbPlayer *ptbSPlayer = &pstSUser->m_tbPlayer;
    SUserInfo *pstTUser = &pstSession->m_stTargetUser;
    TbPlayer *ptbTPlayer = &pstTUser->m_tbPlayer;
    SCityInfo *pstSCity = &pstSUser->m_stCityInfo;
    SCityInfo *pstTCity = &pstTUser->m_stCityInfo;

    CWarProcess::ComputeCityDead(pstTUser, pstDefender, ptbReqMarch->m_nId);

    CWarProcess::ReturnEncampAction(pstDefender);

    CWarProcess::RobResourceFromCity(ptbReqMarch, pstAttacker, pstDefender, udwBattleResult);

    CWarProcess::CaptureDragon(pstSession, pstAttacker, pstDefender, udwBattleResult);

    if(udwBattleResult == EN_REPORT_RESULT_WIN)
    {
        CWarProcess::RescueDragon(pstSession, pstTUser, ptbSPlayer);
    }

    CWarProcess::AddDragonExp(pstSUser, pstSCity, pstAttacker, udwBattleResult);
    CWarProcess::AddDragonExp(pstTUser, pstTCity, pstDefender, udwBattleResult);
    
    SetKnightResult(ptbReqMarch, pstAttacker, udwBattleResult);
    
    ptbSPlayer->Set_Bat(1, UPDATE_ACTION_TYPE_ADD);
    if (udwBattleResult == EN_REPORT_RESULT_WIN)
    {
        ptbSPlayer->Set_Bat_suc(1, UPDATE_ACTION_TYPE_ADD);
        ptbSPlayer->Set_Dcity(1, UPDATE_ACTION_TYPE_ADD);
    }
    ptbTPlayer->Set_Bat(1, UPDATE_ACTION_TYPE_ADD);
    if (udwBattleResult == EN_REPORT_RESULT_LOSE)
    {
        ptbTPlayer->Set_Bat_suc(1, UPDATE_ACTION_TYPE_ADD);
    }
}

TVOID CWarProcess::ComputeAttackOccupyResult(SSession *pstSession, SBattleNode *pstAttacker, SBattleNode *pstDefender, TUINT32 udwBattleResult)
{
    TbMarch_action *ptbReqMarch = &pstSession->m_stReqMarch;
    SUserInfo *pstSUser = &pstSession->m_stSourceUser;
    TbPlayer *ptbSPlayer = &pstSUser->m_tbPlayer;
    SUserInfo *pstTUser = &pstSession->m_stTargetUser;
    TbPlayer *ptbTPlayer = &pstTUser->m_tbPlayer;
    TbMap *ptbWild = &pstSession->m_stMapItem;
    SCityInfo *pstSCity = &pstSUser->m_stCityInfo;
    SCityInfo *pstTCity = &pstTUser->m_stCityInfo;

    if (udwBattleResult == EN_REPORT_RESULT_WIN)
    {
        CWarProcess::ReturnEncampAction(pstDefender, TRUE);
    }

    CWarProcess::ProcessOccupyWild(pstSUser, pstTUser,ptbReqMarch, pstAttacker, pstDefender, ptbWild, udwBattleResult);
    CWarProcess::ComputeWildResult(ptbWild, pstDefender, udwBattleResult);

    CWarProcess::CaptureDragonWild(pstSession, pstAttacker, pstDefender, udwBattleResult);

    CWarProcess::AddDragonExp(pstSUser, pstSCity, pstAttacker, udwBattleResult);
    CWarProcess::AddDragonExp(pstTUser, pstTCity, pstDefender, udwBattleResult);

    SetKnightResult(ptbReqMarch, pstAttacker, udwBattleResult);
    SetDefenderKnightResult(pstTUser, pstDefender, udwBattleResult, ptbWild->m_nId);

    ptbSPlayer->Set_Bat(1, UPDATE_ACTION_TYPE_ADD);
    if (udwBattleResult == EN_REPORT_RESULT_WIN)
    {
        ptbSPlayer->Set_Bat_suc(1, UPDATE_ACTION_TYPE_ADD);
    }
    ptbTPlayer->Set_Bat(1, UPDATE_ACTION_TYPE_ADD);
    if (udwBattleResult == EN_REPORT_RESULT_LOSE)
    {
        ptbTPlayer->Set_Bat_suc(1, UPDATE_ACTION_TYPE_ADD);
    }
}

TVOID CWarProcess::ComputeAttackCampResult(SSession *pstSession, SBattleNode *pstAttacker, SBattleNode *pstDefender, TUINT32 udwBattleResult)
{
    TbMarch_action *ptbReqMarch = &pstSession->m_stReqMarch;
    SUserInfo *pstSUser = &pstSession->m_stSourceUser;
    TbPlayer *ptbSPlayer = &pstSUser->m_tbPlayer;
    SUserInfo *pstTUser = &pstSession->m_stTargetUser;
    TbPlayer *ptbTPlayer = &pstTUser->m_tbPlayer;
    TbMap *ptbWild = &pstSession->m_stMapItem;
    SCityInfo *pstSCity = &pstSUser->m_stCityInfo;
    SCityInfo *pstTCity = &pstTUser->m_stCityInfo;

    CWarProcess::CalcAttackCampAction(pstSession, pstAttacker, pstDefender, udwBattleResult);

    CWarProcess::CaptureDragonWild(pstSession, pstAttacker, pstDefender, udwBattleResult);

    CWarProcess::AddDragonExp(pstSUser, pstSCity, pstAttacker, udwBattleResult);
    CWarProcess::AddDragonExp(pstTUser, pstTCity, pstDefender, udwBattleResult);

    SetKnightResult(ptbReqMarch, pstAttacker, udwBattleResult);
    SetDefenderKnightResult(pstTUser, pstDefender, udwBattleResult, ptbWild->m_nId);

    ptbSPlayer->Set_Bat(1, UPDATE_ACTION_TYPE_ADD);
    if (udwBattleResult == EN_REPORT_RESULT_WIN)
    {
        ptbSPlayer->Set_Bat_suc(1, UPDATE_ACTION_TYPE_ADD);
    }
    ptbTPlayer->Set_Bat(1, UPDATE_ACTION_TYPE_ADD);
    if (udwBattleResult == EN_REPORT_RESULT_LOSE)
    {
        ptbTPlayer->Set_Bat_suc(1, UPDATE_ACTION_TYPE_ADD);
    }
}

TVOID CWarProcess::ComputeAttackOccupyLairResult(SSession *pstSession, SBattleNode *pstAttacker, SBattleNode *pstDefender, TUINT32 udwBattleResult)
{
    TbMarch_action *ptbReqMarch = &pstSession->m_stReqMarch;
    SUserInfo *pstSUser = &pstSession->m_stSourceUser;
    TbPlayer *ptbSPlayer = &pstSUser->m_tbPlayer;
    SUserInfo *pstTUser = &pstSession->m_stTargetUser;
    TbPlayer *ptbTPlayer = &pstTUser->m_tbPlayer;
    TbMap *ptbWild = &pstSession->m_stMapItem;
    SCityInfo *pstSCity = &pstSUser->m_stCityInfo;
    SCityInfo *pstTCity = &pstTUser->m_stCityInfo;

    TSE_LOG_INFO(pstSession->m_poServLog, ("ComputeAttackOccupyLairResult: battle result=%u [seq=%u]",
        udwBattleResult, pstSession->m_udwSeqNo));
    if (udwBattleResult == EN_REPORT_RESULT_WIN)
    {
        CWarProcess::ReturnEncampAction(pstDefender, TRUE);
    }

    CWarProcess::ProcessOccupyLair(pstSUser, pstTUser, ptbReqMarch, pstAttacker, pstDefender, ptbWild, udwBattleResult);
    CWarProcess::ComputeWildResult(ptbWild, pstDefender, udwBattleResult);

    CWarProcess::CaptureDragonWild(pstSession, pstAttacker, pstDefender, udwBattleResult);

    CWarProcess::AddDragonExp(pstSUser, pstSCity, pstAttacker, udwBattleResult);
    CWarProcess::AddDragonExp(pstTUser, pstTCity, pstDefender, udwBattleResult);

    SetKnightResult(ptbReqMarch, pstAttacker, udwBattleResult);
    SetDefenderKnightResult(pstTUser, pstDefender, udwBattleResult, ptbWild->m_nId);

    ptbSPlayer->Set_Bat(1, UPDATE_ACTION_TYPE_ADD);
    if (udwBattleResult == EN_REPORT_RESULT_WIN)
    {
        ptbSPlayer->Set_Bat_suc(1, UPDATE_ACTION_TYPE_ADD);
    }
    ptbTPlayer->Set_Bat(1, UPDATE_ACTION_TYPE_ADD);
    if (udwBattleResult == EN_REPORT_RESULT_LOSE)
    {
        ptbTPlayer->Set_Bat_suc(1, UPDATE_ACTION_TYPE_ADD);
    }
}

TVOID CWarProcess::ComputeAttackWildResult(SSession *pstSession, SBattleNode *pstAttacker, SBattleNode *pstDefender, TUINT32 udwBattleResult)
{
    TbMarch_action *ptbReqMarch = &pstSession->m_stReqMarch;
    SUserInfo *pstSUser = &pstSession->m_stSourceUser;
    TbPlayer *ptbSPlayer = &pstSUser->m_tbPlayer;
    TbMap *ptbWild = &pstSession->m_stMapItem;
    SCityInfo *pstSCity = &pstSUser->m_stCityInfo;
    SUserInfo *pstTUser = &pstSession->m_stTargetUser;
    SCityInfo *pstTCity = &pstTUser->m_stCityInfo;

    CWarProcess::GenWildReward(ptbReqMarch, ptbWild, udwBattleResult);
    CWarProcess::ComputeWildResult(ptbReqMarch, ptbWild, udwBattleResult);

    CWarProcess::AddDragonExp(pstSUser, pstSCity, pstAttacker, udwBattleResult);
    CWarProcess::AddDragonExp(pstTUser, pstTCity, pstDefender, udwBattleResult);

    Json::Value tipContent = Json::Value(Json::arrayValue);
    //普通奖励tips
    for(TUINT32 udwIdx = 0; udwIdx < ptbReqMarch->m_bReward[0].ddwTotalNum; ++udwIdx)
    {
        tipContent[udwIdx] = Json::Value(Json::arrayValue);
        tipContent[udwIdx].append(ptbReqMarch->m_bReward[0].aRewardList[udwIdx].ddwType);
        tipContent[udwIdx].append(ptbReqMarch->m_bReward[0].aRewardList[udwIdx].ddwId);
        tipContent[udwIdx].append(ptbReqMarch->m_bReward[0].aRewardList[udwIdx].ddwNum);
    }
    //惊喜奖励tips
    for(TUINT32 udwIdx = 0; udwIdx < ptbReqMarch->m_bSp_reward[0].ddwTotalNum; ++udwIdx)
    {
        TUINT32 udwJsonIndex = udwIdx + ptbReqMarch->m_bReward[0].ddwTotalNum;
        tipContent[udwJsonIndex] = Json::Value(Json::arrayValue);
        tipContent[udwJsonIndex].append(ptbReqMarch->m_bSp_reward[0].aRewardList[udwIdx].ddwType);
        tipContent[udwJsonIndex].append(ptbReqMarch->m_bSp_reward[0].aRewardList[udwIdx].ddwId);
        tipContent[udwJsonIndex].append(ptbReqMarch->m_bSp_reward[0].aRewardList[udwIdx].ddwNum);
    }
    //wave@20160402:TODO
    //CSendMessageBase::AddTips(pstSUser, ptbSPlayer->m_nUid, TRUE, EN_TIPS_TYPY__GET_WILD_REWARD, tipContent.toStyledString());

    SetKnightResult(ptbReqMarch, pstAttacker, udwBattleResult);

    ptbSPlayer->Set_Bat(1, UPDATE_ACTION_TYPE_ADD);
    if (udwBattleResult == EN_REPORT_RESULT_WIN)
    {
        ptbSPlayer->Set_Bat_suc(1, UPDATE_ACTION_TYPE_ADD);
    }
}

TVOID CWarProcess::ComputeRallyCityResult(SSession *pstSession, SBattleNode *pstAttacker, SBattleNode *pstDefender, TUINT32 udwBattleResult)
{
    TbMarch_action *ptbRallyWar = &pstSession->m_stReqMarch;
    SUserInfo *pstSUser = &pstSession->m_stSourceUser;
    TbPlayer *ptbSPlayer = &pstSUser->m_tbPlayer;
    SUserInfo *pstTUser = &pstSession->m_stTargetUser;
    TbPlayer *ptbTPlayer = &pstTUser->m_tbPlayer;
    TbMap *ptbWild = &pstSession->m_stMapItem;
    SCityInfo *pstSCity = &pstSUser->m_stCityInfo;
    SCityInfo *pstTCity = &pstTUser->m_stCityInfo;
    
    CWarProcess::ComputeCityDead(pstTUser, pstDefender, ptbRallyWar->m_nId);

    CWarProcess::ReturnEncampAction(pstDefender);

    CWarProcess::RobResourceFromCity(ptbRallyWar, pstAttacker, pstDefender, udwBattleResult);

    CWarProcess::CaptureDragon(pstSession, pstAttacker, pstDefender, udwBattleResult);

    if(udwBattleResult == EN_REPORT_RESULT_WIN)
    {
        CWarProcess::RescueDragon(pstSession, pstTUser, ptbSPlayer);
    }

    CWarProcess::AddDragonExp(pstSUser, pstSCity, pstAttacker, udwBattleResult);
    CWarProcess::AddDragonExp(pstTUser, pstTCity, pstDefender, udwBattleResult);

    CWarProcess::ProcessRallyReinforce(pstAttacker, ptbWild->m_nId, ptbWild->m_nType, TRUE, FALSE);
    CWarProcess::ProcessRallyReinforce(pstDefender, 0, -1, TRUE, FALSE);

    SetKnightResult(ptbRallyWar, pstAttacker, udwBattleResult);

    ptbSPlayer->Set_Bat(1, UPDATE_ACTION_TYPE_ADD);
    if(udwBattleResult == EN_REPORT_RESULT_WIN)
    {
        ptbSPlayer->Set_Bat_suc(1, UPDATE_ACTION_TYPE_ADD);
        ptbSPlayer->Set_Dcity(1, UPDATE_ACTION_TYPE_ADD);
    }
    ptbTPlayer->Set_Bat(1, UPDATE_ACTION_TYPE_ADD);
    if(udwBattleResult == EN_REPORT_RESULT_LOSE)
    {
        ptbTPlayer->Set_Bat_suc(1, UPDATE_ACTION_TYPE_ADD);
    }
}

TVOID CWarProcess::ComputeRallyThroneResult(SSession *pstSession, SBattleNode *pstAttacker, SBattleNode *pstDefender, TUINT32 udwBattleResult)
{
    TbMarch_action* ptbRallyWar = &pstSession->m_stReqMarch;

    SUserInfo* pstSUser = &pstSession->m_stSourceUser;
    TbAlliance* ptbSAlliance = &pstSUser->m_tbAlliance;

    SUserInfo* pstTUser = &pstSession->m_stTargetUser;
    TbAlliance* ptbTAlliance = &pstTUser->m_tbAlliance;
    SCityInfo *pstTCity = &pstTUser->m_stCityInfo;

    TbThrone *ptbThrone = &pstSession->m_tbThrone;

    TbMap *ptbWild = &pstSession->m_stMapItem;
    TBOOL bIsWild = pstTUser->m_tbPlayer.m_nUid == 0 ? TRUE : FALSE;

    SCityInfo *pstSCity = &pstSUser->m_stCityInfo;

    CWarProcess::AddDragonExp(pstSUser, pstSCity, pstAttacker, udwBattleResult);
    CWarProcess::AddDragonExp(pstTUser, pstTCity, pstDefender, udwBattleResult);

    SetKnightResult(ptbRallyWar, pstAttacker, udwBattleResult);

    CaptureDragonThrone(pstSession, pstAttacker, pstDefender, udwBattleResult);

    TBOOL bToControl = TRUE;
    if(udwBattleResult == EN_REPORT_RESULT_WIN)
    {
        if(!bIsWild)//有人的占领的野地要直接放弃
        {
            CWarProcess::ProcessRallyReinforce(pstDefender, 0, -1, TRUE, FALSE);
            CCommonLogic::AbandonThrone(ptbTAlliance, ptbThrone, ptbWild);
            // 删除收税action
            for (TUINT32 udwIdx = 0; udwIdx < pstTUser->m_udwMarchNum; udwIdx++)
            {
                if (pstTUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__TAX
                    && pstTUser->m_atbMarch[udwIdx].m_nSuid == pstTUser->m_tbPlayer.m_nUid)
                {
                    pstTUser->m_aucMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__DEL;
                }
            }
            CMsgBase::RefreshUserInfo(ptbThrone->m_nOwner_id);
        }

        CWarProcess::SetThroneOnFight(pstSession, ptbWild);
        CWarProcess::ActiveIdol(pstSession, CTimeUtils::GetUnixTime());

        CWarProcess::ControlThrone(ptbSAlliance, ptbThrone, ptbWild);

        if (pstSUser->m_tbPlayer.m_nUid == ptbThrone->m_nOwner_id)
        {
            CActionBase::GenTaxAction(pstSUser, ptbThrone->m_nPos);
        }
        else
        {
            CMsgBase::RefreshUserInfo(ptbThrone->m_nOwner_id);
        }
        
        pstSession->bKingChanged = TRUE;

        TSE_LOG_DEBUG(pstSession->m_poServLog, ("Source alliacne[aid=%ld] control the province[pos=%ld] [seq=%u]",
            ptbSAlliance->m_nAid, ptbWild->m_nId, pstSession->m_udwSeqNo));
    }
    else
    {
        bToControl = FALSE;
        if(!bIsWild)
        {
            //没有攻打成功 只快速返回防守方的空支援
            CWarProcess::ReturnNoTroopReinforce(pstDefender, 0, -1);
        }
    }

    //处理支援action 
    CWarProcess::ProcessRallyReinforce(pstAttacker, ptbWild->m_nId, ptbWild->m_nType, FALSE, bToControl);

    //处理主action
    if (!bToControl || ptbSAlliance->m_nOid != ptbRallyWar->m_nSuid || ptbRallyWar->m_bParam[0].m_stDragon.m_ddwLevel == 0
        || !CActionBase::IsMarchHasTroop(ptbRallyWar))
    {
        CActionBase::ReturnMarch(ptbRallyWar);
    }
    else
    {
        CActionBase::RallyWarToThroneAssign(ptbRallyWar);
    }
}

TVOID CWarProcess::AddDragonExp(SUserInfo *pstUser, SCityInfo *pstCity, SBattleNode *pstAttacker, TUINT32 udwBattleResult)
{
    if(!pstAttacker->m_bIsDragonJoin)
    {
        return;
    }
    TINT32 dwExpAdd = CCommonBase::GetGameBasicVal(EN_GAME_BASIC_DRAGON_FIGHT_EXP_RATIO) * 1.0 / 10000 *
        (pstAttacker->m_armys.kill_troop_force + pstAttacker->m_armys.kill_fort_force);

    TINT64 ddwExpBefore = pstUser->m_tbPlayer.m_nDragon_exp;
    CPlayerBase::AddDragonExp(pstUser, dwExpAdd);
    pstAttacker->m_stDragon.m_ddwExpInc = pstUser->m_tbPlayer.m_nDragon_exp - ddwExpBefore;
}

TVOID CWarProcess::SetThroneOnFight(SSession* pstSession, TbMap* ptbWild)
{
    TbThrone *ptbThrone = &pstSession->m_tbThrone;

    ptbThrone->Set_Status(EN_THRONE_STATUS__CONTEST_PERIOD);
    ptbWild->Set_Status(EN_THRONE_STATUS__CONTEST_PERIOD);
    ptbThrone->Set_End_time(CTimeUtils::GetUnixTime() + CCommonBase::GetGameBasicVal(EN_GAME_BASIC_THRONE_FIGHT_TIME));
    ptbWild->Set_Time_end(ptbThrone->m_nEnd_time);

    TbMarch_action *ptbMarch = &pstSession->m_atbTmpMarch[pstSession->m_udwTmpMarchNum];
    pstSession->m_audwTmpMarchFlag[pstSession->m_udwTmpMarchNum] = EN_TABLE_UPDT_FLAG__NEW;

    ptbMarch->Set_Id(CActionBase::GenMapActionId(ptbWild->m_nSid, ptbWild->m_nId));
    ptbMarch->Set_Mclass(EN_ACTION_MAIN_CLASS__TIMER);
    ptbMarch->Set_Sclass(EN_ACTION_SEC_CLASS__THRONE_PERIOD);
    ptbMarch->Set_Status(EN_TITMER_THRONE_PERIOD_STATUS__CONTEST);
    ptbMarch->Set_Etime(ptbThrone->m_nEnd_time);
    ptbMarch->Set_Sid(ptbWild->m_nSid);
    ptbMarch->Set_Tpos(ptbWild->m_nId);
    pstSession->m_udwTmpMarchNum++;
}

TVOID CWarProcess::ActiveIdol(SSession* pstSession, TUINT32 udwActiveTime)
{
    for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_udwIdolNum; udwIdx++)
    {
        if (pstSession->m_atbIdol[udwIdx].m_nStatus == EN_IDOL_STATUS__THRONE_PEACE_TIME
            && pstSession->m_atbIdol[udwIdx].m_nEnd_time == 0)
        {
            pstSession->m_audwTmpMarchFlag[pstSession->m_udwTmpMarchNum] = EN_TABLE_UPDT_FLAG__NEW;
            TbMarch_action *ptbAction = &pstSession->m_atbTmpMarch[pstSession->m_udwTmpMarchNum];
            ptbAction->Set_Suid(0);
            ptbAction->Set_Id(CActionBase::GenMapActionId(pstSession->m_atbIdol[udwIdx].m_nSid, pstSession->m_atbIdol[udwIdx].m_nPos));
            ptbAction->Set_Sid(pstSession->m_atbIdol[udwIdx].m_nSid);
            ptbAction->Set_Tpos(pstSession->m_atbIdol[udwIdx].m_nPos);
            ptbAction->Set_Mclass(EN_ACTION_MAIN_CLASS__TIMER);
            ptbAction->Set_Sclass(EN_ACTION_SEC_CLASS__IDOL_PERIOD);
            ptbAction->Set_Status(EN_TITMER_IDOL_PERIOD_STATUS__THRONE_PEACE_TIME);
            ptbAction->Set_Etime(udwActiveTime);
            pstSession->m_udwTmpMarchNum++;
        }
    }
}

TVOID CWarProcess::ControlThrone(TbAlliance* ptbSAlliance, TbThrone *ptbThrone, TbMap* ptbWild)
{
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();

    ptbSAlliance->Set_Last_occupy_time(udwCurTime);

    ptbThrone->Set_Alid(ptbSAlliance->m_nAid);
    ptbThrone->Set_Owner_id(ptbSAlliance->m_nOid);
    ptbThrone->Set_Owner_cid(ptbSAlliance->m_nOwner_cid);

    ptbThrone->m_jInfo = Json::Value(Json::objectValue);
    ptbThrone->m_jInfo["buff"] = CGameInfo::GetInstance()->m_oJsonRoot["game_throne_buff"];
    ptbThrone->SetFlag(TbTHRONE_FIELD_INFO);

    ptbThrone->Set_Occupy_time(udwCurTime);

    ptbWild->Set_Uid(ptbSAlliance->m_nOid);
    ptbWild->Set_Uname(ptbSAlliance->m_sOname);

    ptbWild->Set_Alid(ptbSAlliance->m_nAid);
    ptbWild->Set_Alname(ptbSAlliance->m_sName);
    ptbWild->Set_Al_nick(ptbSAlliance->m_sAl_nick_name);
    ptbWild->Set_Al_flag(ptbSAlliance->m_nAvatar);
    ptbWild->Set_Name_update_time(udwCurTime);
}

TINT32 CWarProcess::ProcessRallyReinforce(SBattleNode *pstNode, TINT64 ddwWildPos, TINT64 ddwWildType, TBOOL bIsForceReturn /*= FALSE*/, TBOOL bToControl /*= FALSE*/)
{
    for(TUINT32 udwIdx = 0; udwIdx < pstNode->m_udwRallyReinforceNum; ++udwIdx)
    {
        TbMarch_action* ptbRallyReinforce = pstNode->m_pastRallyReinforceList[udwIdx];

        if (!CActionBase::IsMarchHasTroop(ptbRallyReinforce))
        {
            CActionBase::ReturnMarch(ptbRallyReinforce, ddwWildPos, ddwWildType);
            continue;
        }

        if (bIsForceReturn)
        {
            CActionBase::ReturnMarch(ptbRallyReinforce, ddwWildPos, ddwWildType);
        }
        else if(bToControl)
        {
            CActionBase::ReinforceToThrone(ptbRallyReinforce, ddwWildPos, ddwWildType);
        }
        else
        {
            CActionBase::ReturnMarch(ptbRallyReinforce, ddwWildPos, ddwWildType);
        }
    }
    return 0;
}

TINT32 CWarProcess::ReturnNoTroopReinforce(SBattleNode *pstNode, TINT64 ddwWildPos, TINT64 ddwWildType)
{
    for (TUINT32 udwIdx = 0; udwIdx < pstNode->m_udwRallyReinforceNum; ++udwIdx)
    {
        TbMarch_action* ptbRallyReinforce = pstNode->m_pastRallyReinforceList[udwIdx];

        if (!CActionBase::IsMarchHasTroop(ptbRallyReinforce))
        {
            CActionBase::ReturnMarch(ptbRallyReinforce, ddwWildPos, ddwWildType);
        }
    }
    return 0;
}

TVOID CWarProcess::ReturnThroneAssign(SUserInfo* pstTUser, TbMap * ptbWild)
{
    for(TUINT32 udwIdx = 0; udwIdx < pstTUser->m_udwMarchNum; ++udwIdx)
    {
        if(pstTUser->m_aucMarchFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }

        TbMarch_action* ptbThroneAssign = &pstTUser->m_atbMarch[udwIdx];
        if(ptbThroneAssign->m_nSclass == EN_ACTION_SEC_CLASS__ASSIGN_THRONE
            && ptbThroneAssign->m_nStatus == EN_MARCH_STATUS__DEFENDING
            && ptbThroneAssign->m_nTpos == ptbWild->m_nId)
        {
            CActionBase::ReturnMarch(ptbThroneAssign);
        }
    }
}

TINT32 CWarProcess::SetKnightResult(TbMarch_action *ptbMarchAction, SBattleNode *pstNode, TUINT32 udwBattleResult)
{
    if (ptbMarchAction->m_bParam[0].m_stKnight.ddwId < 0 || ptbMarchAction->m_bParam[0].m_stKnight.ddwLevel >= CGameInfo::GetInstance()->GetBasicVal(EN_GAME_BASIC_KNIGHT_TOP_LEVEL))
    {
        return 0;
    }
    if (udwBattleResult == EN_REPORT_RESULT_WIN)
    {
        ptbMarchAction->m_bParam[0].m_stKnight.ddwExpAdd = CGameInfo::GetInstance()->m_oJsonRoot["game_knight"]["a"]["a0"].asInt64();
    }
    else
    {
        ptbMarchAction->m_bParam[0].m_stKnight.ddwExpAdd = CGameInfo::GetInstance()->m_oJsonRoot["game_knight"]["a"]["a1"].asInt64();
    }
    pstNode->m_stKnight.ddwExpAdd = ptbMarchAction->m_bParam[0].m_stKnight.ddwExpAdd;
    ptbMarchAction->SetFlag(TbMARCH_ACTION_FIELD_PARAM);

    return 0;
}

TINT32 CWarProcess::SetDefenderKnightResult(SUserInfo *pstUser, SBattleNode *pstNode, TUINT32 udwBattleResult, TUINT32 udwWildPos)
{
    TINT32 dwActionIdx = -1;
    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
    {
        TbMarch_action *ptbMarch = &pstUser->m_atbMarch[udwIdx];
        if (ptbMarch->m_nTpos == udwWildPos && ptbMarch->m_nSuid == pstUser->m_tbPlayer.m_nUid &&
            ptbMarch->m_nMclass == EN_ACTION_MAIN_CLASS__MARCH &&
            (ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__OCCUPY || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__CAMP))
        {
            dwActionIdx = udwIdx;
            break;
        }
    }

    if (dwActionIdx >= 0)
    {
        TbMarch_action *ptbMarch = &pstUser->m_atbMarch[dwActionIdx];
        if (ptbMarch->m_bParam[0].m_stKnight.ddwId >= 0 && ptbMarch->m_bParam[0].m_stKnight.ddwLevel > 0)
        {
            if (udwBattleResult == EN_REPORT_RESULT_WIN)
            {
                ptbMarch->m_bParam[0].m_stKnight.ddwExpAdd = CGameInfo::GetInstance()->m_oJsonRoot["game_knight"]["a"]["a1"].asInt64();
            }
            else
            {
                ptbMarch->m_bParam[0].m_stKnight.ddwExpAdd = CGameInfo::GetInstance()->m_oJsonRoot["game_knight"]["a"]["a0"].asInt64();
            }
            ptbMarch->SetFlag(TbMARCH_ACTION_FIELD_PARAM);
            pstUser->m_aucMarchFlag[dwActionIdx] = EN_TABLE_UPDT_FLAG__NEW;
            pstNode->m_stKnight.ddwExpAdd = ptbMarch->m_bParam[0].m_stKnight.ddwExpAdd;
        }
    }

    return 0;
}

TINT32 CWarProcess::ComputeCityDead(SUserInfo *pstUserInfo, SBattleNode *pstDefender, TINT64 ddwAlterId)
{
    SCityInfo *pstCity = pstDefender->m_pstDefenderCity;
    std::map<ArmyId, OneArmy>::iterator it = pstDefender->m_armys.actors.find(pstCity->m_stTblData.m_nUid);
    if(it == pstDefender->m_armys.actors.end())
    {
        return 0;
    }

    TBOOL bNeedAltar = FALSE;
    TUINT32 udwHosTroopNum = CCityBase::GetHosTroopNum(pstCity, pstUserInfo->m_atbSelfAlAction, pstUserInfo->m_aucSelfAlActionFlag, pstUserInfo->m_udwSelfAlActionNum);

    TINT64 ddwHosCapacity = pstUserInfo->m_stPlayerBuffList[EN_BUFFER_INFO_HOSPITAL_CAPICATY].m_ddwBuffTotal > udwHosTroopNum ?
        pstUserInfo->m_stPlayerBuffList[EN_BUFFER_INFO_HOSPITAL_CAPICATY].m_ddwBuffTotal - udwHosTroopNum : 0;

    TUINT32 udwHosFortNum = CCityBase::GetHosFortNum(pstCity, pstUserInfo->m_atbSelfAlAction, pstUserInfo->m_aucSelfAlActionFlag, pstUserInfo->m_udwSelfAlActionNum);

    TINT64 ddwHosFortCapacity = pstUserInfo->m_stPlayerBuffList[EN_BUFFER_INFO_FORT_HOSPITAL_CAPICATY].m_ddwBuffTotal > udwHosFortNum ?
        pstUserInfo->m_stPlayerBuffList[EN_BUFFER_INFO_FORT_HOSPITAL_CAPICATY].m_ddwBuffTotal - udwHosFortNum : 0;

    SCommonTroop stTroop = it->second.dead_troop;
    SCommonFort stFort = it->second.dead_fort;

    pstDefender->m_stDefenderCityWoundedTroop.Reset();

    CGameInfo *poGameInfo = CGameInfo::GetInstance();

    TINT64 addwTierTotalDeadNum[MAX_ARMY_TIER_LIMIT];
    vector<TINT32> avecTierIdList[MAX_ARMY_TIER_LIMIT];
    TINT32 dwMaxTier = -1;

    memset(addwTierTotalDeadNum, 0, sizeof(addwTierTotalDeadNum));
    for (TUINT32 udwIdx = 0; udwIdx < MAX_ARMY_TIER_LIMIT; udwIdx++)
    {
        avecTierIdList[udwIdx].clear();
    }

    for (TUINT32 udwIdx = 0; udwIdx < poGameInfo->m_oJsonRoot["game_troop"].size(); ++udwIdx)
    {
        if (stTroop[udwIdx] == 0)
        {
            continue;
        }
        addwTierTotalDeadNum[poGameInfo->m_aTroop[udwIdx].m_dwTier] += stTroop[udwIdx];
        avecTierIdList[poGameInfo->m_aTroop[udwIdx].m_dwTier].push_back(udwIdx);
        if (poGameInfo->m_aTroop[udwIdx].m_dwTier > dwMaxTier)
        {
            dwMaxTier = poGameInfo->m_aTroop[udwIdx].m_dwTier;
        }
    }

    for (TINT32 dwTierIdx = dwMaxTier; dwTierIdx >= 0; dwTierIdx--)
    {
        if (addwTierTotalDeadNum[dwTierIdx] <= ddwHosCapacity)
        {
            pstUserInfo->m_tbPlayer.m_bWar_statistics[0].ddwHosTroopNum += addwTierTotalDeadNum[dwTierIdx];
            pstUserInfo->m_tbPlayer.SetFlag(TbPLAYER_FIELD_WAR_STATISTICS);

            ddwHosCapacity -= addwTierTotalDeadNum[dwTierIdx];

            TINT32 dwTroopId = 0;
            for (TUINT32 udwTroopIdx = 0; udwTroopIdx < avecTierIdList[dwTierIdx].size(); udwTroopIdx++)
            {
                dwTroopId = avecTierIdList[dwTierIdx][udwTroopIdx];
                if (stTroop[dwTroopId] == 0)
                {
                    continue;
                }

                pstCity->m_stTblData.m_bHos_wait[0].m_addwNum[dwTroopId] += stTroop[dwTroopId];
                pstCity->m_stTblData.SetFlag(TbCITY_FIELD_HOS_WAIT);
                pstDefender->m_stDefenderCityWoundedTroop[dwTroopId] = stTroop[dwTroopId];
                it->second.wounded_troop[dwTroopId] = stTroop[dwTroopId];
                it->second.dead_troop[dwTroopId] -= stTroop[dwTroopId];
            }
        }
        else
        {
            bNeedAltar = TRUE;

            if (ddwHosCapacity == 0)
            {
                break;
            }

            pstUserInfo->m_tbPlayer.m_bWar_statistics[0].ddwHosTroopNum += ddwHosCapacity;
            pstUserInfo->m_tbPlayer.SetFlag(TbPLAYER_FIELD_WAR_STATISTICS);

            TINT32 dwTroopId = 0;
            TINT64 ddwRealNum = 0;
            TINT64 ddwTotalNum = 0;
            for (TUINT32 udwTroopIdx = 0; udwTroopIdx < avecTierIdList[dwTierIdx].size(); udwTroopIdx++)
            {
                dwTroopId = avecTierIdList[dwTierIdx][udwTroopIdx];
                if (stTroop[dwTroopId] == 0)
                {
                    continue;
                }

                ddwRealNum = 1.0 * stTroop[dwTroopId] / addwTierTotalDeadNum[dwTierIdx] * ddwHosCapacity;

                pstCity->m_stTblData.m_bHos_wait[0].m_addwNum[dwTroopId] += ddwRealNum;
                pstCity->m_stTblData.SetFlag(TbCITY_FIELD_HOS_WAIT);
                pstDefender->m_stDefenderCityWoundedTroop[dwTroopId] = ddwRealNum;
                it->second.wounded_troop[dwTroopId] = ddwRealNum;
                it->second.dead_troop[dwTroopId] -= ddwRealNum;

                ddwTotalNum += ddwRealNum;
            }

            TFLOAT64 dRealNum = 0.0;
            if (ddwHosCapacity > ddwTotalNum)
            {
                for (TUINT32 udwTroopIdx = 0; udwTroopIdx < avecTierIdList[dwTierIdx].size(); udwTroopIdx++)
                {
                    if (ddwHosCapacity <= ddwTotalNum)
                    {
                        break;
                    }
                    dwTroopId = avecTierIdList[dwTierIdx][udwTroopIdx];
                    if (stTroop[dwTroopId] == 0)
                    {
                        continue;
                    }

                    dRealNum = 1.0 * stTroop[dwTroopId] / addwTierTotalDeadNum[dwTierIdx] * ddwHosCapacity;
                    ddwRealNum = dRealNum;
                    if (dRealNum - ddwRealNum > 0.00001 && stTroop[dwTroopId] > ddwRealNum)
                    {
                        pstCity->m_stTblData.m_bHos_wait[0].m_addwNum[dwTroopId] += 1;
                        pstCity->m_stTblData.SetFlag(TbCITY_FIELD_HOS_WAIT);
                        pstDefender->m_stDefenderCityWoundedTroop[dwTroopId] += 1;
                        it->second.wounded_troop[dwTroopId] += 1;
                        it->second.dead_troop[dwTroopId] -= 1;

                        ddwTotalNum += 1;
                    }
                }
            }
            break;
        }
    }

    if(bNeedAltar)
    {
        CMsgBase::SendEncourageMail(&pstUserInfo->m_tbUserStat, pstUserInfo->m_tbPlayer.m_nSid, EN_MAIL_ID__FIRST_HOSPITAL_OVER);
    }

    dwMaxTier = -1;

    memset(addwTierTotalDeadNum, 0, sizeof(addwTierTotalDeadNum));
    for (TUINT32 udwIdx = 0; udwIdx < MAX_ARMY_TIER_LIMIT; udwIdx++)
    {
        avecTierIdList[udwIdx].clear();
    }

    for (TUINT32 udwIdx = 0; udwIdx < poGameInfo->m_oJsonRoot["game_fort"].size(); ++udwIdx)
    {
        if (stFort[udwIdx] == 0)
        {
            continue;
        }
        addwTierTotalDeadNum[poGameInfo->m_aFort[udwIdx].m_dwTier] += stFort[udwIdx];
        avecTierIdList[poGameInfo->m_aFort[udwIdx].m_dwTier].push_back(udwIdx);
        if (poGameInfo->m_aFort[udwIdx].m_dwTier > dwMaxTier)
        {
            dwMaxTier = poGameInfo->m_aFort[udwIdx].m_dwTier;
        }
    }

    for (TINT32 dwTierIdx = dwMaxTier; dwTierIdx >= 0; dwTierIdx--)
    {
        if (ddwHosFortCapacity == 0)
        {
            break;
        }

        if (addwTierTotalDeadNum[dwTierIdx] <= ddwHosFortCapacity)
        {
            ddwHosFortCapacity -= addwTierTotalDeadNum[dwTierIdx];

            TINT32 dwFortId = 0;
            for (TUINT32 udwFortIdx = 0; udwFortIdx < avecTierIdList[dwTierIdx].size(); udwFortIdx++)
            {
                dwFortId = avecTierIdList[dwTierIdx][udwFortIdx];
                if (stFort[dwFortId] == 0)
                {
                    continue;
                }

                pstCity->m_stTblData.m_bDead_fort[0].m_addwNum[dwFortId] += stFort[dwFortId];
                pstCity->m_stTblData.SetFlag(TbCITY_FIELD_DEAD_FORT);
                pstDefender->m_stDefenderCityWoundedFort[dwFortId] = stFort[dwFortId];
                it->second.wounded_fort[dwFortId] = stFort[dwFortId];
                it->second.dead_fort[dwFortId] -= stFort[dwFortId];
            }
        }
        else
        {
            TINT32 dwFortId = 0;
            TINT64 ddwRealNum = 0;
            TINT64 ddwTotalNum = 0;
            for (TUINT32 udwFortIdx = 0; udwFortIdx < avecTierIdList[dwTierIdx].size(); udwFortIdx++)
            {
                dwFortId = avecTierIdList[dwTierIdx][udwFortIdx];
                if (stFort[dwFortId] == 0)
                {
                    continue;
                }

                ddwRealNum = 1.0 * stFort[dwFortId] / addwTierTotalDeadNum[dwTierIdx] * ddwHosFortCapacity;

                pstCity->m_stTblData.m_bDead_fort[0].m_addwNum[dwFortId] += ddwRealNum;
                pstCity->m_stTblData.SetFlag(TbCITY_FIELD_DEAD_FORT);
                pstDefender->m_stDefenderCityWoundedFort[dwFortId] = ddwRealNum;
                it->second.wounded_fort[dwFortId] = ddwRealNum;
                it->second.dead_fort[dwFortId] -= ddwRealNum;

                ddwTotalNum += ddwRealNum;
            }

            TFLOAT64 dRealNum = 0.0;
            if (ddwHosFortCapacity > ddwTotalNum)
            {
                for (TUINT32 udwFortIdx = 0; udwFortIdx < avecTierIdList[dwTierIdx].size(); udwFortIdx++)
                {
                    if (ddwHosFortCapacity <= ddwTotalNum)
                    {
                        break;
                    }
                    dwFortId = avecTierIdList[dwTierIdx][udwFortIdx];
                    if (stFort[dwFortId] == 0)
                    {
                        continue;
                    }

                    dRealNum = 1.0 * stFort[dwFortId] / addwTierTotalDeadNum[dwTierIdx] * ddwHosFortCapacity;
                    ddwRealNum = dRealNum;
                    if (dRealNum - ddwRealNum > 0.00001 && stFort[dwFortId] > ddwRealNum)
                    {
                        pstCity->m_stTblData.m_bDead_fort[0].m_addwNum[dwFortId] += 1;
                        pstCity->m_stTblData.SetFlag(TbCITY_FIELD_DEAD_FORT);
                        pstDefender->m_stDefenderCityWoundedFort[dwFortId] += 1;
                        it->second.wounded_fort[dwFortId] += 1;
                        it->second.dead_fort[dwFortId] -= 1;

                        ddwTotalNum += 1;
                    }
                }
            }
            break;
        }
    }
    
    return 0;
}

TINT32 CWarProcess::RobResourceFromCity(TbMarch_action *ptbReqMarch, SBattleNode *pstAttacker, SBattleNode *pstDefender, TUINT32 udwBattleResult)
{
    if (udwBattleResult == EN_REPORT_RESULT_LOSE)
    {
        return 0;
    }

    CWarBase::ComputeTotalLoad(pstAttacker);

    if(pstAttacker->m_armys.total_load == 0)
    {
        return 0;
    }

    SCommonResource stActionResource;
    stActionResource.Reset();
    SCommonResource *pstCityResource = &pstDefender->m_pstDefenderCity->m_stTblData.m_bResource[0];
    TINT64 ddwTotalResource = 0;
    TINT64 ddwProtectionLimit = 0;
    TFLOAT32 fRate = 0.0;
    TUINT32 udwIdx = 0;
    SCommonResource stCityResource;
    stCityResource.Reset();

    pstDefender->m_pstDefenderCity->m_stTblData.SetFlag(TbCITY_FIELD_RESOURCE);

    TFLOAT64 dfTotalAddBuff = (10000.0 + pstDefender->m_pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_ALL_RESOURCE_PROTECTION].m_ddwBuffTotal)/10000;

    if (udwBattleResult == EN_REPORT_RESULT_WIN)
    {
        ddwProtectionLimit = pstDefender->m_pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_GOLD_PROTECTION].m_ddwBuffTotal * dfTotalAddBuff;
        stCityResource[EN_RESOURCE_TYPE__GOLD] = pstCityResource->m_addwNum[EN_RESOURCE_TYPE__GOLD] > ddwProtectionLimit ?
            pstCityResource->m_addwNum[EN_RESOURCE_TYPE__GOLD] - ddwProtectionLimit : 0;
        ddwTotalResource += stCityResource[EN_RESOURCE_TYPE__GOLD];

        ddwProtectionLimit = pstDefender->m_pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_FOOD_PROTECTION].m_ddwBuffTotal * dfTotalAddBuff;
        stCityResource[EN_RESOURCE_TYPE__FOOD] = pstCityResource->m_addwNum[EN_RESOURCE_TYPE__FOOD] > ddwProtectionLimit ?
            pstCityResource->m_addwNum[EN_RESOURCE_TYPE__FOOD] - ddwProtectionLimit : 0;
        ddwTotalResource += stCityResource[EN_RESOURCE_TYPE__FOOD];

        ddwProtectionLimit = pstDefender->m_pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_WODD_PROTECTION].m_ddwBuffTotal * dfTotalAddBuff;
        stCityResource[EN_RESOURCE_TYPE__WOOD] = pstCityResource->m_addwNum[EN_RESOURCE_TYPE__WOOD] > ddwProtectionLimit ?
            pstCityResource->m_addwNum[EN_RESOURCE_TYPE__WOOD] - ddwProtectionLimit : 0;
        ddwTotalResource += stCityResource[EN_RESOURCE_TYPE__WOOD];

        ddwProtectionLimit = pstDefender->m_pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_STONE_PROTECTION].m_ddwBuffTotal * dfTotalAddBuff;
        stCityResource[EN_RESOURCE_TYPE__STONE] = pstCityResource->m_addwNum[EN_RESOURCE_TYPE__STONE] > ddwProtectionLimit ?
            pstCityResource->m_addwNum[EN_RESOURCE_TYPE__STONE] - ddwProtectionLimit : 0;
        ddwTotalResource += stCityResource[EN_RESOURCE_TYPE__STONE];

        ddwProtectionLimit = pstDefender->m_pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_ORE_PROTECTION].m_ddwBuffTotal * dfTotalAddBuff;
        stCityResource[EN_RESOURCE_TYPE__ORE] = pstCityResource->m_addwNum[EN_RESOURCE_TYPE__ORE] > ddwProtectionLimit ?
            pstCityResource->m_addwNum[EN_RESOURCE_TYPE__ORE] - ddwProtectionLimit : 0;
        ddwTotalResource += stCityResource[EN_RESOURCE_TYPE__ORE];
    }

    if (pstAttacker->m_armys.total_load < ddwTotalResource)
    {
        fRate = pstAttacker->m_armys.total_load * 1.0 / ddwTotalResource;
    }
    else
    {
        fRate = 1.0000000;
    }
    
    for(udwIdx = 0; udwIdx < EN_RESOURCE_TYPE__END; udwIdx++)
    {
        if (stCityResource[udwIdx] > 0)
        {
            stActionResource.m_addwNum[udwIdx] = stCityResource[udwIdx] * fRate;
            pstCityResource->m_addwNum[udwIdx] = pstCityResource->m_addwNum[udwIdx] > stActionResource.m_addwNum[udwIdx] ?
                pstCityResource->m_addwNum[udwIdx] - stActionResource.m_addwNum[udwIdx] : 0;
        }
    }

    CWarBase::DistributeResource(pstAttacker, &stActionResource);

    return 0;
}

TINT32 CWarProcess::ReturnEncampAction(SBattleNode *pstDefender, TBOOL bIsForceReturn /*= FALSE*/)
{
    TBOOL bNoTroopFlag = TRUE;
    TBOOL bNoKnightFlag = TRUE;
    for(TUINT32 udwIdx = 0; udwIdx < pstDefender->m_udwEncampNum; ++udwIdx)
    {
        bNoTroopFlag = TRUE;
        bNoKnightFlag = TRUE;
        if(pstDefender->m_pastEncampActionList[udwIdx]->m_nStatus == EN_MARCH_STATUS__DEFENDING
            || pstDefender->m_pastEncampActionList[udwIdx]->m_nStatus == EN_MARCH_STATUS__LOADING)
        {
            for(TUINT32 udwIdy = 0; udwIdy < EN_TROOP_TYPE__END; ++udwIdy)
            {
                if(pstDefender->m_pastEncampActionList[udwIdx]->m_bParam[0].m_stTroop.m_addwNum[udwIdy] > 0)
                {
                    bNoTroopFlag = FALSE;
                    break;
                }
            }
            if (pstDefender->m_pastEncampActionList[udwIdx]->m_bParam[0].m_stKnight.ddwLevel > 0)
            {
                bNoKnightFlag = FALSE;
            }
            if (bNoTroopFlag == TRUE || bIsForceReturn)
            {
                // 更新状态
                if(bNoTroopFlag == FALSE || bNoKnightFlag == FALSE)
                {
                    CActionBase::ReturnMarch(pstDefender->m_pastEncampActionList[udwIdx]);
                }
                else
                {
                    CActionBase::DeleteMarch(pstDefender->m_pastEncampActionList[udwIdx]);
                }
            }
        }
    }

    return 0;
}

TINT32 CWarProcess::ProcessOccupyWild(SUserInfo *pstSUser, SUserInfo *pstTarget, TbMarch_action *ptbReqMarch, SBattleNode *pstAttacker, SBattleNode *pstDefender, TbMap *ptbWild, TUINT32 udwBattleResult)
{
    TbMarch_action *ptbDefendAction = NULL;
    for (TUINT32 udwIdx = 0; udwIdx < pstDefender->m_udwEncampNum; ++udwIdx)
    {
        if (pstDefender->m_pastEncampActionList[udwIdx]->m_nTpos == pstDefender->m_ptbWild->m_nId)
        {
            ptbDefendAction = pstDefender->m_pastEncampActionList[udwIdx];
            break;
        }
    }
    if(ptbDefendAction == NULL)
    {
        return 0;
    }

    const Json::Value& oWildResJson = CWildInfo::GetWildResInfo(ptbReqMarch->m_nSid);

    //是否带英雄 会影响buff
    if (ptbReqMarch->m_bParam[0].m_stDragon.m_ddwLevel != 0)
    {
        CBufferBase::GenMarchBuff(&pstSUser->m_stPlayerBuffList, ptbReqMarch);
    }
    else
    {
        CBufferBase::GenMarchBuff(&pstSUser->m_stBuffWithoutDragon, ptbReqMarch);
    }
    pstAttacker->m_armys.total_load = CCommonBase::GetTroopTotalLoad(ptbReqMarch);

    //是否带英雄 会影响buff
    SPlayerBuffInfo stOccupyBuff;
    stOccupyBuff.Reset();
    if (ptbDefendAction->m_bParam[0].m_stDragon.m_ddwLevel != 0)
    {
        CBufferBase::GenOccupyBuff(&pstTarget->m_stPlayerBuffList, &stOccupyBuff);
    }
    else
    {
        CBufferBase::GenOccupyBuff(&pstTarget->m_stBuffWithoutDragon, &stOccupyBuff);
    }
    CBufferBase::SetOccupyBuff(&stOccupyBuff, ptbDefendAction);

    pstDefender->m_armys.total_load = CCommonBase::GetTroopTotalLoad(ptbDefendAction);

    TINT32 dwHasLoadRes = 0;
    TINT32 dwResType = 0;
    TINT32 dwResNum = 0;
    TBOOL bIsGem = FALSE;
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    SActionMarchParam *pstParam = &ptbDefendAction->m_bParam[0];
    dwHasLoadRes = (udwCurTime - pstParam->m_ddwBeginLoadTime) * 1.0 / pstParam->m_ddwTotalLoadTime * ptbWild->m_nReward_left;

    if(dwHasLoadRes > ptbWild->m_nReward_left)
    {
        dwHasLoadRes = ptbWild->m_nReward_left;
    }

    if (oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a0"]["a1"].asInt() == 0
        && oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a0"]["a2"].asInt() == 0)
    {
        if (oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a1"][(TUINT32)(ptbWild->m_nLevel - 1)]["a1"][0U][0U].asInt() == EN_GLOBALRES_TYPE_RESOURCE)
        {
            dwResType = oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a0"]["a6"].asInt();
            dwResNum = oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a1"][(TUINT32)(ptbWild->m_nLevel - 1)]["a1"][0U][2U].asInt() * 1.0 / 10000 * dwHasLoadRes;
        }
        else
        {
            bIsGem = TRUE;
            dwResNum = oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a1"][(TUINT32)(ptbWild->m_nLevel - 1)]["a1"][0U][2U].asInt() * 1.0 * ptbWild->m_nReward_left
                - oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a1"][(TUINT32)(ptbWild->m_nLevel - 1)]["a1"][0U][2U].asInt() * 1.0 * (ptbWild->m_nReward_left - dwHasLoadRes);
            dwResNum /= 10000;
        }
    }
    ptbWild->Set_Reward_left(ptbWild->m_nReward_left - dwHasLoadRes);
    ptbWild->Set_Res_time(0);
    ptbWild->Set_Res_rate(0);

    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("ProcessOccupyWild:[has_load=%u dwResNum=%u] [seq=%u]", dwHasLoadRes, dwResNum,pstSUser->m_udwBSeqNo));
    if (bIsGem)
    {
        pstParam->m_ddwLoadGem += dwResNum;
        dwResNum = pstParam->m_ddwLoadGem;
        pstDefender->m_armys.total_load /= 10000;
        pstAttacker->m_armys.total_load /= 10000;
    }
    else
    {
        pstParam->m_stResource[dwResType] += dwResNum;
        dwResNum = pstParam->m_stResource[dwResType];
    }
    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("ProcessOccupyWild:[has_load=%u dwResNum=%u] [seq=%u]", dwHasLoadRes, dwResNum, pstSUser->m_udwBSeqNo));

    if (udwBattleResult == EN_REPORT_RESULT_LOSE)
    {
        if (pstDefender->m_armys.total_load < dwResNum)
        {
            dwResNum = pstDefender->m_armys.total_load;
            if (bIsGem)
            {
                pstParam->m_ddwLoadGem = pstDefender->m_armys.total_load;
            }
            else
            {
                pstParam->m_stResource[dwResType] = pstDefender->m_armys.total_load;
            }
        }
        pstDefender->m_armys.total_load = CCommonBase::GetTroopTotalLoad(ptbDefendAction);
        pstDefender->m_armys.total_load -= pstParam->m_ddwLoadGem * 10000 + pstParam->m_stResource[dwResType];
        
        TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("ProcessOccupyWild:pstDefender->m_armys.total_load=%u  [seq=%u]",
            pstDefender->m_armys.total_load,
            pstSUser->m_udwBSeqNo));

        pstParam->m_ddwLoadTime = CMapLogic::ComputeLoadResTime(ptbReqMarch, ptbWild);
        pstParam->m_ddwTotalLoadTime = pstParam->m_ddwLoadTime;
        pstParam->m_ddwBeginLoadTime = CTimeUtils::GetUnixTime();
        pstParam->m_ddwLoadResTotalNum = CMapLogic::GetMapResTotalNum(ptbWild);
        pstParam->m_ddwLoadRate = pstParam->m_ddwLoadResTotalNum * 1.0 / pstParam->m_ddwLoadTime * 3600;

        TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("ProcessOccupyWild:m_udwLoadTime=%u  m_udwTotalLoadTime=%u  m_udwBeginLoadTime=%u m_udwLoadResTotalNum=%u m_udwLoadRate=%u [seq=%u]",
            pstParam->m_ddwLoadTime,
            pstParam->m_ddwTotalLoadTime,
            pstParam->m_ddwBeginLoadTime,
            pstParam->m_ddwLoadResTotalNum,
            pstParam->m_ddwLoadRate, pstSUser->m_udwBSeqNo));

        if (pstParam->m_ddwLoadResTotalNum > pstDefender->m_armys.total_load)
        {
            pstParam->m_ddwLoadResTotalNum = pstDefender->m_armys.total_load;
            pstParam->m_ddwLoadTime = ceil(pstParam->m_ddwLoadResTotalNum * 1.0 / pstParam->m_ddwLoadRate * 3600);
        }

        TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("ProcessOccupyWild:m_udwLoadTime=%u  m_udwTotalLoadTime=%u  m_udwBeginLoadTime=%u m_udwLoadResTotalNum=%u m_udwLoadRate=%u [seq=%u]",
            pstParam->m_ddwLoadTime,
            pstParam->m_ddwTotalLoadTime,
            pstParam->m_ddwBeginLoadTime,
            pstParam->m_ddwLoadResTotalNum,
            pstParam->m_ddwLoadRate, pstSUser->m_udwBSeqNo));
        ptbWild->Set_Res_time(pstParam->m_ddwLoadTime);
        ptbWild->Set_Res_rate(pstParam->m_ddwLoadRate);
        ptbDefendAction->SetFlag(TbMARCH_ACTION_FIELD_PARAM);
        ptbDefendAction->Set_Btime(pstParam->m_ddwBeginLoadTime);
        ptbDefendAction->Set_Etime(pstParam->m_ddwBeginLoadTime + pstParam->m_ddwLoadTime);
    }
    else
    {
        if(pstAttacker->m_armys.total_load < dwResNum)
        {
            if (bIsGem)
            {
                ptbReqMarch->m_bParam[0].m_ddwLoadGem = pstAttacker->m_armys.total_load;
            }
            else
            {
                ptbReqMarch->m_bParam[0].m_stResource[dwResType] = pstAttacker->m_armys.total_load;
            }
        }
        else
        {
            if (bIsGem)
            {
                ptbReqMarch->m_bParam[0].m_ddwLoadGem = dwResNum;
            }
            else
            {
                ptbReqMarch->m_bParam[0].m_stResource[dwResType] = dwResNum;
            }
        }
        pstParam->m_stResource.Reset();
        ptbDefendAction->SetFlag(TbMARCH_ACTION_FIELD_PARAM);
        ptbReqMarch->SetFlag(TbMARCH_ACTION_FIELD_PARAM);
    }

    return 0;
}

TINT32 CWarProcess::ProcessOccupyLair(SUserInfo *pstSUser, SUserInfo *pstTarget, TbMarch_action *ptbReqMarch, SBattleNode *pstAttacker, SBattleNode *pstDefender, TbMap *ptbWild, TUINT32 udwBattleResult)
{
    const Json::Value& oWildResJson = CWildInfo::GetWildResInfo(ptbReqMarch->m_nSid);
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();

    //是否带英雄 会影响buff
    if (ptbReqMarch->m_bParam[0].m_stDragon.m_ddwLevel != 0)
    {
        CBufferBase::GenMarchBuff(&pstSUser->m_stPlayerBuffList, ptbReqMarch);
    }
    else
    {
        CBufferBase::GenMarchBuff(&pstSUser->m_stBuffWithoutDragon, ptbReqMarch);
    }
    pstAttacker->m_armys.total_load = CCommonBase::GetTroopTotalLoad(ptbReqMarch);

    if (udwBattleResult == EN_REPORT_RESULT_WIN)
    {
        ptbReqMarch->m_bParam[0].m_ddwLoadGem = 0;
        ptbReqMarch->m_bParam[0].m_stResource.Reset();
    }

    if (udwBattleResult == EN_REPORT_RESULT_WIN)
    {
        TINT32 dwResNum = 0;
        TINT32 dwHasLoadRes = 0;
        TINT32 dwResType = 0;
        TBOOL bIsGem = FALSE;

        for (TUINT32 udwIdx = 0; udwIdx < pstDefender->m_udwEncampNum; ++udwIdx)
        {
            dwHasLoadRes += (udwCurTime - pstDefender->m_pastEncampActionList[udwIdx]->m_bParam[0].m_ddwBeginLoadTime) * 1.0 
                / pstDefender->m_pastEncampActionList[udwIdx]->m_bParam[0].m_ddwTotalLoadTime * ptbWild->m_nReward_left;
        }

        if (dwHasLoadRes > ptbWild->m_nReward_left)
        {
            dwHasLoadRes = ptbWild->m_nReward_left;
        }

        ptbWild->Set_Reward_left(ptbWild->m_nReward_left - dwHasLoadRes);
        ptbWild->Set_Res_time(0);
        ptbWild->Set_Res_rate(0);

        if (oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a0"]["a1"].asInt() == 0
            && oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a0"]["a2"].asInt() == 0)
        {
            if (oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a1"][(TUINT32)(ptbWild->m_nLevel - 1)]["a1"][0U][0U].asInt() == EN_GLOBALRES_TYPE_RESOURCE)
            {
                dwResType = oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a0"]["a6"].asInt();
                dwResNum = oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a1"][(TUINT32)(ptbWild->m_nLevel - 1)]["a1"][0U][2U].asInt() * 1.0 / 10000 * dwHasLoadRes;
            }
            else
            {
                bIsGem = TRUE;
                dwResNum = oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a1"][(TUINT32)(ptbWild->m_nLevel - 1)]["a1"][0U][2U].asInt() * 1.0 * ptbWild->m_nReward_left
                    - oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a1"][(TUINT32)(ptbWild->m_nLevel - 1)]["a1"][0U][2U].asInt() * 1.0 * (ptbWild->m_nReward_left - dwHasLoadRes);
                dwResNum /= 10000;
            }
        }

        if (bIsGem)
        {
            if (pstAttacker->m_armys.total_load / 10000 < dwResNum)
            {
                dwResNum = pstAttacker->m_armys.total_load / 10000;
            }
            ptbReqMarch->m_bParam[0].m_ddwLoadGem += dwResNum;
        }
        else
        {
            if (pstAttacker->m_armys.total_load < dwResNum)
            {
                dwResNum = pstAttacker->m_armys.total_load;
            }
            ptbReqMarch->m_bParam[0].m_stResource[dwResType] += dwResNum;
        }

        ptbReqMarch->SetFlag(TbMARCH_ACTION_FIELD_PARAM);
    }

    return 0;
}

TINT32 CWarProcess::ComputeWildResult(TbMap *ptbWild, SBattleNode *pstDefender, TUINT32 udwBattleResult)
{
    TUINT32 udwMapType = CMapLogic::GetWildClass(ptbWild->m_nSid, ptbWild->m_nType);

    if(udwBattleResult == EN_REPORT_RESULT_LOSE)
    {
        return 0;
    }

    if(EN_WILD_CLASS_MONSTER_NEST != udwMapType && ptbWild->m_nUid && ptbWild->m_nUid == pstDefender->m_ptbDefendPlayer->m_nUid)
    {
        CCommonBase::AbandonWild(pstDefender->m_pstDefenderCity, ptbWild);
    }
    if (EN_WILD_CLASS_MONSTER_NEST == udwMapType && udwBattleResult == EN_REPORT_RESULT_WIN)
    {
        CCommonBase::AbandonWild(pstDefender->m_pstDefenderCity, ptbWild);
    }

    if(ptbWild->m_nReward_left == 0)
    {
        CMapBase::ResetMap(ptbWild);
    }

    return 0;
}

TINT32 CWarProcess::ComputeWildResult(TbMarch_action *ptbMarch, TbMap *ptbWild, TUINT32 udwBattleResult)
{
    if(udwBattleResult == EN_REPORT_RESULT_WIN)
    {
        CMapBase::ResetMap(ptbWild);
    }
    else
    {
        TINT64 ddwOriginMapMight = ptbWild->m_nMight;
        TINT64 ddwMapMight = CMapBase::GetMapMight(ptbWild);
        if(ddwMapMight != ddwOriginMapMight)
        {
            ptbWild->Set_Might(ddwMapMight);
        }
        if(ptbWild->m_nMight <= 0)
        {
            CMapBase::ResetMap(ptbWild);
        }
    }

    return 0;
}

TINT32 CWarProcess::GenWildReward(TbMarch_action *ptbMarch, TbMap *ptbWild, TUINT32 udwBattleResult)
{
    if(!CMapLogic::IsAttackWild(ptbWild->m_nSid, ptbWild->m_nType))
    {
        return 0;
    }

    if(udwBattleResult == EN_REPORT_RESULT_WIN)
    {
        CMapLogic::GetAttackReward(ptbMarch, ptbWild);
    }

    return 0;
}

TVOID CWarProcess::CalcAttackCampAction(SSession *pstSession, SBattleNode *pstAttacker, SBattleNode *pstDefender, TUINT32 udwBattleResult)
{
    TbMarch_action *ptbReqMarch = &pstSession->m_stReqMarch;
    TbMap *ptbWild = &pstSession->m_stMapItem;
    SUserInfo *pstTUser = &pstSession->m_stTargetUser;
    TbPlayer *ptbTPlayer = &pstTUser->m_tbPlayer;
    SCityInfo *pstCity = &pstTUser->m_stCityInfo;

    if (udwBattleResult == EN_REPORT_RESULT_WIN)
    {
        for (TUINT32 udwIdx = 0; udwIdx < pstTUser->m_udwMarchNum; ++udwIdx)
        {
            TbMarch_action *ptbCampAction = &pstTUser->m_atbMarch[udwIdx];
            if (ptbCampAction->m_nSclass == EN_ACTION_SEC_CLASS__CAMP && ptbCampAction->m_nSuid == ptbTPlayer->m_nUid &&
                ptbCampAction->m_nTpos == ptbReqMarch->m_nTpos)
            {
                CActionBase::ReturnMarch(ptbCampAction);
                CCommonBase::AbandonWild(pstCity, ptbWild);
            }
        }
    }
}

TVOID CWarProcess::GenWarLog(SSession *pstSession, SBattleNode *pstAttacker, SBattleNode *pstDefender, const TCHAR *pszCmd, TUINT8 ucType, TUINT32 udwResultCode)
{
    TCHAR szTmpBuf[100 << 10];
    TCHAR *pCur = szTmpBuf;
    TUINT32 udwCurLen = 0;
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    TbMarch_action *ptbMarch = &pstSession->m_stReqMarch;
    SBattleNode *pstNode = NULL;
    SUserInfo *pstUser = NULL;
    SCityInfo *pstCity = NULL;
    TbPlayer *ptbPlayer = NULL;

    if (ucType == 0)
    {
        pstNode = pstAttacker;
    }
    else
    {
        pstNode = pstDefender;
    }

    pstUser = pstNode->m_pstUser;
    ptbPlayer = pstNode->m_ptbPlayer;
    pstCity = pstNode->m_pstCity;

    if (ptbPlayer->m_nUid == 0 || NULL == pstCity)
    {
        return;
    }

    TbCity *ptbCity = &pstCity->m_stTblData;

    // 1. head & keys
    udwCurLen = sprintf(pCur, "%f\t%u\t%s\t%s\t%ld\t%u\t%ld\t",
        1.0, udwCurTime, "", pszCmd,
        ptbPlayer->m_nUid, pstSession->m_udwReqSvrId,
        ptbMarch->m_nTpos);
    pCur += udwCurLen;

    // 2. key
    if (ucType == 0) // attacker
    {
        GenAttackerKeys(pstAttacker, pstDefender, ptbMarch, pCur, udwCurLen, udwResultCode);
    }
    else if (ucType == 1) //city defender
    {
        GenDefenderKeys(pstAttacker, pstDefender, ptbMarch, pCur, udwCurLen, udwResultCode);
    }
    else // wild defender
    {
        GenDefenderKeys(pstAttacker, pstDefender, ptbMarch, pCur, udwCurLen, udwResultCode);
    }
    pCur += udwCurLen;

    // 3. seqno\ret_code\timecost\gid\aid\tid
    TINT32 dwAid = ptbPlayer->m_nAlpos ? ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET : 0;
    udwCurLen = sprintf(pCur, "\t%u\t%d\t%u\t%s\t%d\t%d",
        pstSession->m_udwSeqNo, pstSession->m_stCommonResInfo.m_dwRetCode,
        0, "", dwAid, 2);
    pCur += udwCurLen;

    // 4. options
    udwCurLen = sprintf(pCur, "\t%u|%u|%d|%ld|%ld|%s", pstSession->m_dwReportFlag, 0, udwResultCode, ptbMarch->m_nId,
        ptbPlayer->m_nNpc, pstUser->m_tbLogin.m_sPlatform.c_str());
    pCur += udwCurLen;

    // level
    udwCurLen = sprintf(pCur, "\t%ld", ptbPlayer->m_nDragon_level);
    pCur += udwCurLen;

    // force
    udwCurLen = sprintf(pCur, "\t%ld|%ld|%ld|%ld|%ld|%ld|%ld",
        ptbPlayer->m_nMight, 
        pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_ADD_FORCE].m_astBuffDetail[EN_BUFF_TYPE_BUILDING].m_ddwNum,
        pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_ADD_FORCE].m_astBuffDetail[EN_BUFF_TYPE_RESEARCH].m_ddwNum,
        pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_ADD_FORCE].m_astBuffDetail[EN_BUFF_TYPE_TROOP].m_ddwNum,
        pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_ADD_FORCE].m_astBuffDetail[EN_BUFF_TYPE_FORT].m_ddwNum,
        0L,
        0L);
    pCur += udwCurLen;

    // gem
    udwCurLen = sprintf(pCur, "\t|||||%ld", ptbMarch->m_bParam[0].m_ddwLoadGem);
    pCur += udwCurLen;

    // basic
    udwCurLen = sprintf(pCur, "\t%s|%s|%s|%ld|%ld|%u|%u|%u|%d|%d|%ld|%ld|%ld|%u",
        ptbPlayer->m_sUin.c_str(), ptbCity->m_sName.c_str(), ptbPlayer->m_sAlname.c_str(),
        ptbPlayer->m_nCtime, ptbPlayer->m_nUtime,
        0, 100,
        0, 0,
        0, ptbPlayer->m_nStatus,
        pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_POPULATION_CAPICATY].m_ddwBuffTotal,
        pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_HOSPITAL_CAPICATY].m_ddwBuffTotal,
        0);
    pCur += udwCurLen;

    // 6. output
    TSE_LOG_HOUR(CGlobalServ::m_poFormatLog, ("\t%s", szTmpBuf));
}

TVOID CWarProcess::GenDefenderKeys(SBattleNode *pstAttacker, SBattleNode *pstDefender, TbMarch_action *ptbMarch, TCHAR *pszOut, TUINT32 &udwOutLen, TUINT32 udwResultCode)
{
    TCHAR *pCur = pszOut;
    TUINT32 udwCurLen = 0;
    SActionMarchParam *pstMarchParam = &ptbMarch->m_bParam[0];

    // head
    udwCurLen = sprintf(pCur, "%ld|%ld|%ld|%ld|%ld|%ld|%ld|%ld|%ld",
        pstMarchParam->m_ddwSourceUserId, pstMarchParam->m_ddwSourceCityId, pstMarchParam->m_ddwSourceAlliance,
        pstMarchParam->m_ddwTargetUserId, pstMarchParam->m_ddwTargetCityId, pstMarchParam->m_ddwTargetAlliance,
        ptbMarch->m_nTpos, pstMarchParam->m_ddwTargetType, ptbMarch->m_nSclass);
    pCur += udwCurLen;

    SCommonTroop stTroop;
    SCommonFort  stFort;
    SCommonTroop stRawTroop;
    SCommonFort  stRawFort;
    stTroop.Reset();
    stRawTroop.Reset();
    CWarBase::GetTotalTroop(*pstAttacker, &stTroop);
    CWarBase::GetRawTotalTroop(*pstAttacker, &stRawTroop);

    // resource
    CCommonFunc::CbArrayOutput(pCur, udwCurLen, pstMarchParam->m_stResource.m_addwNum, EN_RESOURCE_TYPE__END, "|", FALSE);
    pCur += udwCurLen;
    // attacker troop raw
    CCommonFunc::CbArrayOutput(pCur, udwCurLen, stRawTroop.m_addwNum, EN_TROOP_TYPE__END, "|", FALSE);
    pCur += udwCurLen;
    // attacker troop end
    CCommonFunc::CbArrayOutput(pCur, udwCurLen, stTroop.m_addwNum, EN_TROOP_TYPE__END, "|", FALSE);
    pCur += udwCurLen;

    // attacker_dead_knight_id
    udwCurLen = sprintf(pCur, "|-1");
    pCur += udwCurLen;

    //status
    udwCurLen = sprintf(pCur, "|%u", udwResultCode);
    pCur += udwCurLen;

    stTroop.Reset();
    stRawTroop.Reset();
    stFort.Reset();
    stRawFort.Reset();
    CWarBase::GetTotalTroop(*pstDefender, &stTroop);
    CWarBase::GetTotalFort(*pstDefender, &stFort);
    CWarBase::GetRawTotalTroop(*pstDefender, &stRawTroop);
    CWarBase::GetRawTotalFort(*pstDefender, &stRawFort);

    // defender troop raw
    CCommonFunc::CbArrayOutput(pCur, udwCurLen, stRawTroop.m_addwNum, EN_TROOP_TYPE__END, "|", FALSE);
    pCur += udwCurLen;
    // defender troop end
    CCommonFunc::CbArrayOutput(pCur, udwCurLen, stTroop.m_addwNum, EN_TROOP_TYPE__END, "|", FALSE);
    pCur += udwCurLen;
    // defender troop wound
    CCommonFunc::CbArrayOutput(pCur, udwCurLen, pstDefender->m_stDefenderCityWoundedTroop.m_addwNum, EN_TROOP_TYPE__END, "|", FALSE);
    pCur += udwCurLen;
    // defender fort beg
    CCommonFunc::CbArrayOutput(pCur, udwCurLen, stRawFort.m_addwNum, EN_FORT_TYPE__END, "|", FALSE);
    pCur += udwCurLen;
    // defender fort end
    CCommonFunc::CbArrayOutput(pCur, udwCurLen, stFort.m_addwNum, EN_FORT_TYPE__END, "|", FALSE);
    pCur += udwCurLen;

    udwOutLen = pCur - pszOut;
}

TVOID CWarProcess::GenAttackerKeys(SBattleNode *pstAttacker, SBattleNode *pstDefender, TbMarch_action *ptbMarch, TCHAR *pszOut, TUINT32 &udwOutLen, TUINT32 udwResultCode)
{
    TCHAR *pCur = pszOut;
    TUINT32 udwCurLen = 0;
    SActionMarchParam *pstMarchParam = &ptbMarch->m_bParam[0];
    SUserInfo *pstSUser = pstAttacker->m_pstUser;
    //SUserInfo *pstTUser = pstDefender->m_pstUser;
    TbPlayer *ptbSPlayer = &pstSUser->m_tbPlayer;
    TbPlayer *ptbTPlayer = &pstSUser->m_tbPlayer;

    SCityInfo *pstSCity = pstAttacker->m_pstCity;
    SCityInfo *pstTCity = pstDefender->m_pstCity;

    // head
    udwCurLen = sprintf(pCur, "%ld|%ld|%ld|%ld|%ld|%ld|%ld|%ld|%ld",
        pstMarchParam->m_ddwSourceUserId, pstMarchParam->m_ddwSourceCityId, pstMarchParam->m_ddwSourceAlliance,
        pstMarchParam->m_ddwTargetUserId, pstMarchParam->m_ddwTargetCityId, pstMarchParam->m_ddwTargetAlliance,
        ptbMarch->m_nTpos, pstMarchParam->m_ddwTargetType, ptbMarch->m_nSclass);
    pCur += udwCurLen;

    SCommonTroop stTroop;
    SCommonFort  stFort;
    SCommonTroop stRawTroop;
    SCommonFort  stRawFort;
    stTroop.Reset();
    stRawTroop.Reset();
    CWarBase::GetTotalTroop(*pstAttacker, &stTroop);
    CWarBase::GetRawTotalTroop(*pstAttacker, &stRawTroop);

    // resource
    CCommonFunc::CbArrayOutput(pCur, udwCurLen, pstMarchParam->m_stResource.m_addwNum, EN_RESOURCE_TYPE__END, "|", FALSE);
    pCur += udwCurLen;
    // attacker troop raw
    CCommonFunc::CbArrayOutput(pCur, udwCurLen, stRawTroop.m_addwNum, EN_TROOP_TYPE__END, "|", FALSE);
    pCur += udwCurLen;
    // attacker troop end
    CCommonFunc::CbArrayOutput(pCur, udwCurLen, stTroop.m_addwNum, EN_TROOP_TYPE__END, "|", FALSE);
    pCur += udwCurLen;

    // attacker_dead_knight_id
    udwCurLen = sprintf(pCur, "|-1");
    pCur += udwCurLen;

    //status
    udwCurLen = sprintf(pCur, "|%u", udwResultCode);
    pCur += udwCurLen;

    stTroop.Reset();
    stRawTroop.Reset();
    stFort.Reset();
    stRawFort.Reset();
    CWarBase::GetTotalTroop(*pstDefender, &stTroop);
    CWarBase::GetTotalFort(*pstDefender, &stFort);
    CWarBase::GetRawTotalTroop(*pstDefender, &stRawTroop);
    CWarBase::GetRawTotalFort(*pstDefender, &stRawFort);

    // defender troop raw
    CCommonFunc::CbArrayOutput(pCur, udwCurLen, stRawTroop.m_addwNum, EN_TROOP_TYPE__END, "|", FALSE);
    pCur += udwCurLen;
    // defender troop end
    CCommonFunc::CbArrayOutput(pCur, udwCurLen, stTroop.m_addwNum, EN_TROOP_TYPE__END, "|", FALSE);
    pCur += udwCurLen;
    // defender troop wound
    CCommonFunc::CbArrayOutput(pCur, udwCurLen, pstDefender->m_stDefenderCityWoundedTroop.m_addwNum, EN_TROOP_TYPE__END, "|", FALSE);
    pCur += udwCurLen;
    // defender fort beg
    CCommonFunc::CbArrayOutput(pCur, udwCurLen, stRawFort.m_addwNum, EN_FORT_TYPE__END, "|", FALSE);
    pCur += udwCurLen;
    // defender fort end
    CCommonFunc::CbArrayOutput(pCur, udwCurLen, stFort.m_addwNum, EN_FORT_TYPE__END, "|", FALSE);
    pCur += udwCurLen;

    // defender name
    udwCurLen = sprintf(pCur, "|%s", ptbTPlayer->m_nUid ? ptbTPlayer->m_sUin.c_str() : "");
    pCur += udwCurLen;

    // attack knight lv
    udwCurLen = sprintf(pCur, "|%d", 0);
    pCur += udwCurLen;

    // defend knight lv
    udwCurLen = sprintf(pCur, "|%u", 0);
    pCur += udwCurLen;

    // wild level
    udwCurLen = sprintf(pCur, "|%ld", pstMarchParam->m_ddwTargetLevel);
    pCur += udwCurLen;

    //nothing
    udwCurLen = sprintf(pCur, "|||||||");
    pCur += udwCurLen;

    // player info
    TUINT8 ucAkCastleLv = 0;
    TUINT8 ucDfCastleLv = 0;
    TUINT32 udwAkPlayerLv = 0;
    TUINT32 udwDfPlayerLv = 0;

    if (NULL != pstSCity)
    {
        ucAkCastleLv = CCityBase::GetBuildingLevelByFuncType(pstSCity, EN_BUILDING_TYPE__CASTLE);
    }
    if (NULL != pstTCity)
    {
        ucDfCastleLv = CCityBase::GetBuildingLevelByFuncType(pstTCity, EN_BUILDING_TYPE__CASTLE);
    }
    if (ptbSPlayer->m_nUid != 0)
    {
        udwAkPlayerLv = ptbSPlayer->m_nLevel;
    }
    if (ptbTPlayer->m_nUid != 0)
    {
        udwDfPlayerLv = ptbTPlayer->m_nLevel;
    }

    udwCurLen = sprintf(pCur, "|%u#%u#%u#%u", ucAkCastleLv, udwAkPlayerLv, ucDfCastleLv, udwDfPlayerLv);
    pCur += udwCurLen;

    udwOutLen = pCur - pszOut;
}

TVOID CWarProcess::GenAllyEncampActionLog(SSession *pstSession, SBattleNode *pstNode, const TCHAR *pszCmd, TUINT32 udwResultCode)
{
    TCHAR szTmpBuf[MAX_REQ_BUF_LEN];
    TCHAR *pCur = szTmpBuf;
    TUINT32 udwCurLen = 0;
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();

    TbMarch_action *ptbMarch = NULL;
    SActionMarchParam *pstMarchParam = NULL;

    for (TUINT32 idx = 0; idx < pstNode->m_udwEncampNum; idx++)
    {
        pCur = szTmpBuf;
        ptbMarch = pstNode->m_pastEncampActionList[idx];
        pstMarchParam = &ptbMarch->m_bParam[0];

        // 1. head & keys
        udwCurLen = sprintf(pCur, "%f\t%u\t%s\t%s\t%ld\t%u\t%ld\t",
            1.0, udwCurTime, "", pszCmd,
            ptbMarch->m_nSuid, pstSession->m_udwReqSvrId,
            ptbMarch->m_nScid);
        pCur += udwCurLen;

        // 2. key
        GenAllyEncampActionKeys(pstNode, idx, pCur, udwCurLen);
        pCur += udwCurLen;

        // 3. seqno\ret_code\timecost\gid\aid\tid
        udwCurLen = sprintf(pCur, "\t%u\t%d\t%u\t%s\t%ld\t%ld",
            pstSession->m_udwSeqNo, pstSession->m_stCommonResInfo.m_dwRetCode,
            0, "", pstMarchParam->m_ddwSourceAlliance,
            ptbMarch->m_nId);
        pCur += udwCurLen;

        // 4. options
        udwCurLen = sprintf(pCur, "\t%u|%u|%d|%ld", pstSession->m_dwReportFlag, 0, udwResultCode, ptbMarch->m_nId);
        pCur += udwCurLen;

        // level-force-gem
        udwCurLen = sprintf(pCur, "\t\t\t|||||%u", 0);
        pCur += udwCurLen;

        // basic
        udwCurLen = sprintf(pCur, "\t%s|%s|%s|0|0|0|0|0|0|0|0|0|0|0",
            pstMarchParam->m_szSourceUserName, pstMarchParam->m_szSourceCityName, pstMarchParam->m_szSourceAlliance);
        pCur += udwCurLen;

        // 5. output
        TSE_LOG_HOUR(CGlobalServ::m_poFormatLog, ("\t%s", szTmpBuf));
    }
}

TVOID CWarProcess::GenAllyEncampActionKeys(SBattleNode *pstNode, TUINT32 idx, TCHAR *pszOut, TUINT32 &udwOutLen)
{
    TCHAR *pCur = pszOut;
    TUINT32 udwCurLen = 0;
    TbMarch_action *ptbMarch = pstNode->m_pastEncampActionList[idx];
    SActionMarchParam *pstMarchParam = &ptbMarch->m_bParam[0];

    // head
    udwCurLen = sprintf(pCur, "%ld|%ld|%ld|%ld|%ld|%ld|%ld|%ld|%ld",
        pstMarchParam->m_ddwSourceUserId, pstMarchParam->m_ddwSourceCityId, pstMarchParam->m_ddwSourceAlliance,
        pstMarchParam->m_ddwTargetUserId, pstMarchParam->m_ddwTargetCityId, pstMarchParam->m_ddwTargetAlliance,
        ptbMarch->m_nTpos, pstMarchParam->m_ddwTargetType, ptbMarch->m_nSclass);
    pCur += udwCurLen;

    // troop raw
    std::map<ArmyId, OneArmy>::iterator it = pstNode->m_armys.actors.find(ptbMarch->m_nId);
    if(it == pstNode->m_armys.actors.end())
    {
        return;
    }

    CCommonFunc::CbArrayOutput(pCur, udwCurLen, it->second.raw_troop.m_addwNum, EN_TROOP_TYPE__END, "|", FALSE);
    pCur += udwCurLen;
    // troop end
    CCommonFunc::CbArrayOutput(pCur, udwCurLen, it->second.left_troop.m_addwNum, EN_TROOP_TYPE__END, "|", FALSE);
    pCur += udwCurLen;

    udwOutLen = pCur - pszOut;
}

TUINT32 CWarProcess::ProcessAttack(SDragonNode *pstDragonNode, SMonsterNode *pstMonsterNode)
{
    CGameInfo *poGameInfo = CGameInfo::GetInstance();
    TINT64 ddwRealDefend = 0;
    pstDragonNode->m_dwMultiAttackTimes = pstDragonNode->m_dwBeginMultiAttackTimes;
    if(pstDragonNode->m_dwMultiAttackTimes > pstDragonNode->m_dwMaxMultiAttackTimes)
    {
        //当连击次数超过玩家当前最大连击次数（科技决定），连击次数从0开始计算
        if(pstDragonNode->m_dwMaxMultiAttackTimes == 0)
        {
            pstDragonNode->m_dwMultiAttackTimes = 0;
        }
        else
        {
            pstDragonNode->m_dwMultiAttackTimes = pstDragonNode->m_dwMultiAttackTimes % (pstDragonNode->m_dwMaxMultiAttackTimes + 1);
        }
    }

    TINT32 dwMultiNextAttackTimes = 0;
    if (pstDragonNode->m_dwMultiAttackTimes + 1 > pstDragonNode->m_dwMaxMultiAttackTimes)
    {
        dwMultiNextAttackTimes = 0;
    }
    else
    {
        dwMultiNextAttackTimes = pstDragonNode->m_dwMultiAttackTimes + 1;
    }

    //基础攻击
    TINT32 dwAttackBase = pstDragonNode->m_ddwAttack;

    //连击buff
    TINT32 dwComboBuff = 0;
    if(pstDragonNode->m_dwMultiAttackTimes > 0)
    {
        dwComboBuff = poGameInfo->m_oJsonRoot["game_dragon_combo"][pstDragonNode->m_dwMultiAttackTimes - 1][1U].asInt();
    }
    TINT32 dwNextComboBuff = 0;
    if (dwMultiNextAttackTimes > 0)
    {
        dwNextComboBuff = poGameInfo->m_oJsonRoot["game_dragon_combo"][dwMultiNextAttackTimes - 1][1U].asInt();
    }
    pstDragonNode->m_dwMultiAttackBuff = dwComboBuff;
    pstDragonNode->m_dwNextMultiAttackBuff = dwNextComboBuff;

    pstDragonNode->m_dwAlMultiAttackTimes = pstDragonNode->m_dwBeginAlMultiAttackTimes;
    //TINT64 ddwAlComboBuff = pstDragonNode->m_dwAlMultiAttackTimes * CCommonBase::GetGameBasicVal(EN_GAME_BASIC_Al_COMBO_BUFF);
    TINT64 ddwAlComboBuff = 0;
    if(pstDragonNode->m_dwAlMultiAttackTimes > 0)
    {
         if(pstDragonNode->m_dwAlMultiAttackTimes > (TINT32)poGameInfo->m_oJsonRoot["game_dragon_ally_combo"].size())
         {
             ddwAlComboBuff = poGameInfo->m_oJsonRoot["game_dragon_ally_combo"][poGameInfo->m_oJsonRoot["game_dragon_ally_combo"].size() - 1][1U].asInt();
         }
         else
         {
             ddwAlComboBuff = poGameInfo->m_oJsonRoot["game_dragon_ally_combo"][pstDragonNode->m_dwAlMultiAttackTimes - 1][1U].asInt();
         }
    }
    pstDragonNode->m_dwAlMultiAttackBuff = ddwAlComboBuff;
    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("ProcessAttack WAVETEST: m_dwAlMultiAttackTimes=%d, ddwAlComboBuff=%ld",
        pstDragonNode->m_dwAlMultiAttackTimes, ddwAlComboBuff));

    //单次特殊攻击伤害
    TINT64 ddwCritAttack = dwAttackBase * ((10000.0 + dwComboBuff + ddwAlComboBuff) / 10000) * ((10000.0 + pstDragonNode->m_dwCritBuff) / 10000);
    TINT64 ddwRaidAttack = dwAttackBase * ((10000.0 + dwComboBuff + ddwAlComboBuff) / 10000) * ((10000.0 + pstDragonNode->m_dwRaidBuff) / 10000);
    TINT64 ddwHeavyAttack = dwAttackBase * ((10000.0 + dwComboBuff + ddwAlComboBuff) / 10000) * ((10000.0 + pstDragonNode->m_dwHeavyBuff) / 10000);
    TINT64 ddwDoubleAttack = dwAttackBase * ((10000.0 + dwComboBuff + ddwAlComboBuff) / 10000) * ((10000.0 + pstDragonNode->m_dwDoubleBuff) / 10000);

    //单次非暴击伤害
    TINT64 ddwNormalAttack = dwAttackBase * ((10000.0 + dwComboBuff + ddwAlComboBuff) / 10000);
    //防御值
    ddwRealDefend = pstMonsterNode->m_ddwDefence / ((10000.0 + pstDragonNode->m_dwMonsterDefenceReduce) / 10000);

    //计算次数
    TINT32 dwIdx = 0;
    TINT64 ddwTmpAttack = 0;    //累计输出
    TINT64 ddwTmpDamage = 0;    //造成累计伤害
    string szMonsterLv = CCommonFunc::NumToString(pstMonsterNode->m_dwLevel);

    pstDragonNode->m_dwEMonsterHit = pstDragonNode->m_dwBMonsterHit;

    while(dwIdx < pstDragonNode->m_dwCanAttackTimes)
    {
        pstDragonNode->m_astAttackInfo[dwIdx].m_dwMultiAttackNum = pstDragonNode->m_dwMultiAttackTimes;
        pstDragonNode->m_astAttackInfo[dwIdx].m_dwMultiAttackBuff = dwComboBuff;

        pstDragonNode->m_dwRealAttackTimes = dwIdx + 1;

        TINT32 dwTmpRate = rand() % 10000;
        if(dwTmpRate < pstDragonNode->m_dwCritChance)
        {
            pstDragonNode->m_astAttackInfo[dwIdx].m_dwAttackType = 1; //暴击
            ddwTmpAttack = ddwCritAttack;
            pstDragonNode->m_astAttackInfo[dwIdx].m_ddwAttackDamage = ddwTmpAttack;
        }
        else if (poGameInfo->m_oJsonRoot["game_monster_hit"][szMonsterLv][pstDragonNode->m_dwEMonsterHit][0U].asInt() != 0)
        {
            TINT64 ddwSpecailAttack = 0;
            pstDragonNode->m_astAttackInfo[dwIdx].m_dwAttackType = poGameInfo->m_oJsonRoot["game_monster_hit"][szMonsterLv][pstDragonNode->m_dwEMonsterHit][0U].asInt();
            switch (pstDragonNode->m_astAttackInfo[dwIdx].m_dwAttackType)
            {
            case 1:
                ddwSpecailAttack = ddwCritAttack;
                break;
            case 2:
                ddwSpecailAttack = ddwHeavyAttack;
                break;
            case 3:
                ddwSpecailAttack = ddwDoubleAttack;
                break;
            case 4:
                ddwSpecailAttack = ddwRaidAttack;
                break;
            default:
                ddwSpecailAttack = ddwNormalAttack;
            }
            ddwTmpAttack = ddwSpecailAttack;
            pstDragonNode->m_astAttackInfo[dwIdx].m_ddwAttackDamage = ddwSpecailAttack;
        }
        else
        {
            pstDragonNode->m_astAttackInfo[dwIdx].m_dwAttackType = 0;
            ddwTmpAttack = ddwNormalAttack;
            pstDragonNode->m_astAttackInfo[dwIdx].m_ddwAttackDamage = ddwNormalAttack;
        }
        if(pstDragonNode->m_astAttackInfo[dwIdx].m_ddwAttackDamage < 0)
        {
            pstDragonNode->m_astAttackInfo[dwIdx].m_ddwAttackDamage = 0;
        }

        if (pstDragonNode->m_dwEMonsterHit == 0)
        {
            pstDragonNode->m_dwHitToZero = 1;
        }
        ++pstDragonNode->m_dwEMonsterHit;
        if (pstDragonNode->m_dwEMonsterHit >= (TINT32)poGameInfo->m_oJsonRoot["game_monster_hit"][szMonsterLv].size())
        {
            pstDragonNode->m_dwEMonsterHit = 0;
        }

        //该次攻击造成累计伤害
        pstDragonNode->m_astAttackInfo[dwIdx].m_ddwRealDamage = ddwTmpAttack > ddwRealDefend ? ddwTmpAttack - ddwRealDefend : 0;
        if (pstDragonNode->m_astAttackInfo[dwIdx].m_ddwRealDamage > 0)
        {
            pstDragonNode->m_ddwRealDamageNum++;
        }
        pstDragonNode->m_ddwTotalDamage += pstDragonNode->m_astAttackInfo[dwIdx].m_ddwRealDamage;
        if (pstDragonNode->m_ddwTotalDamage > pstMonsterNode->m_ddwHp)
        {
            break;
        }
        
        dwIdx++;
    }
    pstMonsterNode->m_ddwLostHp = pstMonsterNode->m_ddwHp > pstDragonNode->m_ddwTotalDamage ? pstDragonNode->m_ddwTotalDamage : pstMonsterNode->m_ddwHp;
    pstMonsterNode->m_ddwHp = pstMonsterNode->m_ddwHp > pstDragonNode->m_ddwTotalDamage ? pstMonsterNode->m_ddwHp - pstDragonNode->m_ddwTotalDamage : 0;

    //pstDragonNode->m_dwELeaderMonsterKillNum = pstDragonNode->m_dwBLeaderMonsterKillNum;
    if(pstMonsterNode->m_ddwHp <= 0)
    {
        pstMonsterNode->m_bIsDead = TRUE;
        if (pstMonsterNode->m_ddwLeader)
        {
            //pstDragonNode->m_dwELeaderMonsterKillNum = (pstDragonNode->m_dwELeaderMonsterKillNum + 1) % poGameInfo->m_oJsonRoot["game_leader_monstor_kill"][szMonsterLv].size();
        }
    }

    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("ProcessAttack base_attack=%u [crit_change=%u crit=%u crit_buff=%u] [comb_lv=%u combo_buff=%u] [can_attack_time=%u real_attack_time=%u][real_attack=%u real_defend=%u damage=%u][monster_hp=%u lost_hp=%u is_dead=%u] [cri_attack=%u normal_attack=%u]",
                    dwAttackBase, pstDragonNode->m_dwCritChance,pstDragonNode->m_astAttackInfo[0].m_dwAttackType, pstDragonNode->m_dwCritBuff,
                    pstDragonNode->m_dwMultiAttackTimes, dwComboBuff,
                    pstDragonNode->m_dwCanAttackTimes, pstDragonNode->m_dwRealAttackTimes,
                    ddwTmpAttack, ddwRealDefend, ddwTmpDamage,
                    pstMonsterNode->m_ddwHp, pstMonsterNode->m_ddwLostHp, pstMonsterNode->m_bIsDead,
                    ddwCritAttack, ddwNormalAttack));

    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("ProcessAttack NEMO: %d %d %d %d %d %d",
        pstMonsterNode->m_dwLevel, pstMonsterNode->m_dwType,
        pstDragonNode->m_dwBMonsterHit, pstDragonNode->m_dwEMonsterHit, 
        pstDragonNode->m_dwBLeaderMonsterKillNum, pstDragonNode->m_dwELeaderMonsterKillNum));

    return EN_REPORT_RESULT_WIN;
}

TVOID CWarProcess::ComputeBattleResult(SSession *pstSession, SDragonNode *pstDragonNode, SMonsterNode *pstMonsterNode)
{
    TbMarch_action *ptbReqAction = &pstSession->m_stReqMarch;
    SUserInfo *pstSUser = &pstSession->m_stSourceUser;
    SUserInfo *pstChallenger = NULL;
    TbPlayer *ptbPlayer = &pstSUser->m_tbPlayer;
    TbMap *ptbWild = &pstSession->m_stMapItem;
    TbPlayer *ptbChallengerPlayer = NULL;

    if (ptbWild->m_nUid != pstSUser->m_tbPlayer.m_nUid)
    {
        pstChallenger = &pstSession->m_stTargetUser;
        ptbChallengerPlayer = &pstChallenger->m_tbPlayer;
    }
    else
    {
        pstChallenger = pstSUser;
    }

    CWarBase::SetDragonResult(ptbReqAction, pstSUser, ptbWild, pstDragonNode, pstMonsterNode);

    // CWarBase::SetMonsterRewardResult(pstSUser, ptbReqAction, ptbWild, &pstSession->m_tbTmpAlGift, pstMonsterNode, pstDragonNode);
    // CWarBase::SetChallengerResult(pstChallenger, pstMonsterNode, pstDragonNode);

    CWarBase::SetMonsterResult(ptbReqAction, ptbPlayer, ptbChallengerPlayer, ptbWild, pstDragonNode, pstMonsterNode);
}

TVOID CWarProcess::GenMoveCityAction(SSession *pstSession, TbMap *ptbWild, SBattleNode *pstDefend)
{
    //被动移城
    TINT64 ddwSpecialMoveFlag = (ptbWild->m_nStatus & EN_CITY_STATUS__ON_SPECIAL_WILD? 1 : 0);
    TBOOL bUpdate = TRUE;
    for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_stTargetUser.m_udwActionNum; ++udwIdx)
    {
        if(pstSession->m_stTargetUser.m_atbAction[udwIdx].m_nMclass == EN_ACTION_MAIN_TASK_ATTACK_MOVE &&
            pstSession->m_stTargetUser.m_atbAction[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__ATTACK_MOVE)
        {
            //若已经存在强制踢飞, 则直接返回..
            if (pstSession->m_stTargetUser.m_atbAction[udwIdx].m_bParam[0].m_stAttackMove.m_ddwSpecailMoveFlag == 1)
            {
                return;
            }
            bUpdate = FALSE;
            if (pstDefend->bAllTroopDead && ddwSpecialMoveFlag == 1)
            {
                pstSession->m_stTargetUser.m_aucActionFlag[udwIdx] = EN_TABLE_UPDT_FLAG__DEL;
            }
        }
    }

    //王座区域踢飞...直接生成action...
    if (ddwSpecialMoveFlag == 1)
    {
        //着火
        if (pstDefend->bAllTroopDead && bUpdate)
        {
            pstSession->m_stTargetUser.m_tbPlayer.Set_Status(pstSession->m_stTargetUser.m_tbPlayer.m_nStatus | EN_CITY_STATUS__BEEN_REMOVE);
            ptbWild->Set_Burn_end_time(CTimeUtils::GetUnixTime() + 60 * 60 * 6);

            ptbWild->Set_Status(pstSession->m_stTargetUser.m_tbPlayer.Get_Status());
            pstSession->m_ucMapItemFlag = EN_TABLE_UPDT_FLAG__CHANGE;
        }

        UActionParam stParam;
        stParam.m_stAttackMove.SetValue(ptbWild->m_nId, ddwSpecialMoveFlag);
        CCommonBase::AddAction(&pstSession->m_stTargetUser, pstDefend->m_pstCity, EN_ACTION_MAIN_TASK_ATTACK_MOVE,
            EN_ACTION_SEC_CLASS__ATTACK_MOVE,
            EN_MARCH_STATUS__MOVE_CITY_PREPARE, 1, &stParam);
        return;
    }

    //着火
    if (pstDefend->bAllTroopDead && bUpdate)
    {
        pstSession->m_stTargetUser.m_tbPlayer.Set_Status(pstSession->m_stTargetUser.m_tbPlayer.m_nStatus | EN_CITY_STATUS__BEEN_REMOVE);
        ptbWild->Set_Burn_end_time(CTimeUtils::GetUnixTime() + 60 * 60 * 6);

        ptbWild->Set_Status(pstSession->m_stTargetUser.m_tbPlayer.Get_Status());
        ptbWild->Set_Move_city(1);
        pstSession->m_ucMapItemFlag = EN_TABLE_UPDT_FLAG__CHANGE;
    }

    //被动移城action
    if (pstDefend->bAllTroopDead && bUpdate)
    {
        UActionParam stParam;
        stParam.m_stAttackMove.SetValue(ptbWild->m_nId, ddwSpecialMoveFlag);
        CCommonBase::AddAction(&pstSession->m_stTargetUser, pstDefend->m_pstCity, EN_ACTION_MAIN_TASK_ATTACK_MOVE,
            EN_ACTION_SEC_CLASS__ATTACK_MOVE,
            EN_MARCH_STATUS__MOVE_CITY_PREPARE, 60 * 60 * 6, &stParam);

        pstSession->m_stTargetUser.m_tbPlayer.m_bWar_statistics[0].ddwMyCityDestroyedNum++;
        pstSession->m_stTargetUser.m_tbPlayer.SetFlag(TbPLAYER_FIELD_WAR_STATISTICS);

        pstSession->m_stSourceUser.m_tbPlayer.m_bWar_statistics[0].ddwDestroyCityNum++;
        pstSession->m_stSourceUser.m_tbPlayer.SetFlag(TbPLAYER_FIELD_WAR_STATISTICS);

        TSE_LOG_INFO(pstSession->m_poServLog, ("GenMoveCityAction :gen move city action[uid=%u action_id=%lld map_id=%u] [player_status=%u map_status=%u burn_time_end=%u move=%u] [seq=%u] ", 
            pstSession->m_stTargetUser.m_tbPlayer.m_nUid,
            (pstSession->m_stTargetUser.m_tbPlayer.m_nUid << 32) + pstSession->m_stTargetUser.m_tbLogin.m_nSeq - 1,
            ptbWild->m_nId,
            pstSession->m_stTargetUser.m_tbPlayer.m_nStatus,
            ptbWild->m_nStatus,
            ptbWild->m_nBurn_end_time,
            ptbWild->m_nMove_city,
            pstSession->m_udwSeqNo));
    }
}

TVOID CWarProcess::ComputeWarResult(SSession *pstSession, SBattleNode *pstAttacker, SBattleNode *pstDefender, TUINT32 udwResultCode, TUINT32 udwHostNum)
{
    SUserInfo *pstSUser = &pstSession->m_stSourceUser;
    SUserInfo *pstTUser = &pstSession->m_stTargetUser;
    TbMarch_action *ptbReqMarch = &pstSession->m_stReqMarch;
    SCityInfo *pstSCity = &pstSUser->m_stCityInfo;

    TbMap *ptbWild = &pstSession->m_stMapItem;

    if(udwResultCode != EN_REPORT_RESULT_WIN &&
        EN_REPORT_RESULT_LOSE != udwResultCode)
    {
        return;
    }

    TbPlayer *pstSPlayer = &pstSUser->m_tbPlayer;
    TbPlayer *pstTPlayer = NULL;
    SWarStatistics *pstTWarStatis = NULL;
    if(ptbReqMarch->m_nTuid != 0 
        && ptbWild->m_nType != EN_WILD_TYPE__THRONE_NEW)
    {
        pstTPlayer = &pstTUser->m_tbPlayer;
        pstTWarStatis = &pstTPlayer->m_bWar_statistics[0];
    }

    TUINT32 udwWildRawUId = ptbWild->m_nUid;
    
    if (udwResultCode == EN_REPORT_RESULT_LOSE)
    {
        for (TUINT32 udwIdx = 0; udwIdx < pstAttacker->m_udwRallyReinforceNum; ++udwIdx)
        {
            pstAttacker->m_pastRallyReinforceList[udwIdx]->m_bReinforce_result[0].ddwReinforceAtkFail++;
            pstAttacker->m_pastRallyReinforceList[udwIdx]->SetFlag(TbMARCH_ACTION_FIELD_REINFORCE_RESULT);
        }

        for (TUINT32 udwIdx = 0; udwIdx < pstDefender->m_udwEncampNum; udwIdx++)
        {
            pstDefender->m_pastEncampActionList[udwIdx]->m_bReinforce_result[0].ddwReinforceDefendWin++;
            pstDefender->m_pastEncampActionList[udwIdx]->SetFlag(TbMARCH_ACTION_FIELD_REINFORCE_RESULT);
        }

        for (TUINT32 udwIdx = 0; udwIdx < pstDefender->m_udwRallyReinforceNum; ++udwIdx)
        {
            pstDefender->m_pastRallyReinforceList[udwIdx]->m_bReinforce_result[0].ddwReinforceDefendWin++;
            pstDefender->m_pastRallyReinforceList[udwIdx]->SetFlag(TbMARCH_ACTION_FIELD_REINFORCE_RESULT);
        }
    }
    else if (udwResultCode == EN_REPORT_RESULT_WIN)
    {
        for (TUINT32 udwIdx = 0; udwIdx < pstAttacker->m_udwRallyReinforceNum; ++udwIdx)
        {
            pstAttacker->m_pastRallyReinforceList[udwIdx]->m_bReinforce_result[0].ddwReinforceAtkWin++;
            pstAttacker->m_pastRallyReinforceList[udwIdx]->SetFlag(TbMARCH_ACTION_FIELD_REINFORCE_RESULT);
        }

        for (TUINT32 udwIdx = 0; udwIdx < pstDefender->m_udwEncampNum; udwIdx++)
        {
            pstDefender->m_pastEncampActionList[udwIdx]->m_bReinforce_result[0].ddwReinforceDefendFail++;
            pstDefender->m_pastEncampActionList[udwIdx]->SetFlag(TbMARCH_ACTION_FIELD_REINFORCE_RESULT);
        }

        for (TUINT32 udwIdx = 0; udwIdx < pstDefender->m_udwRallyReinforceNum; ++udwIdx)
        {
            pstDefender->m_pastRallyReinforceList[udwIdx]->m_bReinforce_result[0].ddwReinforceDefendFail++;
            pstDefender->m_pastRallyReinforceList[udwIdx]->SetFlag(TbMARCH_ACTION_FIELD_REINFORCE_RESULT);
        }
    }

    //source info 
    SWarStatistics *pstSWarStatis = &pstSPlayer->m_bWar_statistics[0];
    TUINT32 udwSDeadTroopNum = 0;
    TUINT32 udwSDeadFortNum = 0;
    std::map<ArmyId, OneArmy>::iterator SourceIt = pstAttacker->m_armys.actors.find(ptbReqMarch->m_nId);
    if(SourceIt != pstAttacker->m_armys.actors.end())
    {
        for(TUINT32 udwIdx = 0; udwIdx < EN_TROOP_TYPE__END; ++udwIdx)
        {
            udwSDeadTroopNum += SourceIt->second.dead_troop.m_addwNum[udwIdx];
        }
        for(TUINT32 udwIdx = 0; udwIdx < EN_FORT_TYPE__END; ++udwIdx)
        {
            udwSDeadFortNum += SourceIt->second.dead_fort.m_addwNum[udwIdx];
        }
        if(udwResultCode == EN_REPORT_RESULT_WIN)
        {
            pstSWarStatis->ddwAtkWin++;
        }
        else if(udwResultCode == EN_REPORT_RESULT_LOSE)
        {
            pstSWarStatis->ddwAtkFail++;
        }
        pstSWarStatis->ddwMyTroopDamagedNum += udwSDeadTroopNum;
        pstSWarStatis->ddwMyFortDamagedNum += udwSDeadFortNum;

        pstSWarStatis->ddwDamageTroopNum += SourceIt->second.kill_troop_num;
        pstSWarStatis->ddwDamageFortNum += SourceIt->second.kill_fort_num;

        CQuestLogic::SetTaskCurValue(pstSUser, pstSCity, EN_TASK_TYPE_ING_KILL_TROP_NUM, SourceIt->second.kill_troop_num);
        CQuestLogic::SetTaskCurValue(pstSUser, pstSCity, EN_TASK_TYPE_ING_KILL_FORT_NUM, SourceIt->second.kill_fort_num);

        pstSWarStatis->ddwHurtEnemyTroopNum += udwHostNum;
        pstSWarStatis->ddwForceKilled += SourceIt->second.kill_fort_force + SourceIt->second.kill_troop_force;
        pstSPlayer->SetFlag(TbPLAYER_FIELD_WAR_STATISTICS);

        CActivitesLogic::ComputeTroopKillScore(&pstSession->m_stSourceUser,
            SourceIt->second.kill_troop_force,
            SourceIt->second.kill_fort_force,
            pstTPlayer == NULL ? 0 : pstTPlayer->m_nUid);
    }

    if (pstTPlayer != NULL && pstTPlayer->m_nUid && pstTWarStatis != NULL)
    {
        TINT64 ddwTargetId = -1;
        if (ptbWild->m_nType == EN_WILD_TYPE__CITY)
        {
            ddwTargetId = pstTPlayer->m_nUid;
        }
        if (ddwTargetId != -1)
        {
            SCityInfo *pstTCity = &pstTUser->m_stCityInfo;
            
            //target info
            std::map<ArmyId, OneArmy>::iterator TargetIt = pstDefender->m_armys.actors.find(ddwTargetId);
            if(TargetIt != pstDefender->m_armys.actors.end())
            {
                TUINT32 udwTDeadTroopNum = 0;
                TUINT32 udwTDeadFortNum = 0;
                TUINT32 udwTWoundTroopNum = 0;
                TUINT32 udwTWoundFortNum = 0;
                for(TUINT32 udwIdx = 0; udwIdx < EN_TROOP_TYPE__END; ++udwIdx)
                {
                    udwTDeadTroopNum += TargetIt->second.dead_troop.m_addwNum[udwIdx];
                    udwTWoundTroopNum += TargetIt->second.wounded_troop.m_addwNum[udwIdx];
                }
                for(TUINT32 udwIdx = 0; udwIdx < EN_FORT_TYPE__END; ++udwIdx)
                {
                    udwTDeadFortNum += TargetIt->second.dead_fort.m_addwNum[udwIdx];
                    udwTWoundFortNum += TargetIt->second.wounded_fort.m_addwNum[udwIdx];
                }
                pstTWarStatis->ddwMyTroopDamagedNum += udwTDeadTroopNum + udwTWoundTroopNum;
                pstTWarStatis->ddwMyFortDamagedNum += udwTDeadFortNum + udwTWoundFortNum;
                pstTWarStatis->ddwDamageTroopNum += TargetIt->second.kill_troop_num;
                pstTWarStatis->ddwDamageFortNum += TargetIt->second.kill_fort_num;
                pstTWarStatis->ddwForceKilled += TargetIt->second.kill_fort_force + TargetIt->second.kill_troop_force;
                pstTPlayer->SetFlag(TbPLAYER_FIELD_WAR_STATISTICS);

                CQuestLogic::SetTaskCurValue(pstTUser, pstTCity, EN_TASK_TYPE_ING_KILL_TROP_NUM, TargetIt->second.kill_troop_num);
                CQuestLogic::SetTaskCurValue(pstTUser, pstTCity, EN_TASK_TYPE_ING_KILL_FORT_NUM, TargetIt->second.kill_fort_num);

                CActivitesLogic::ComputeTroopKillScore(&pstSession->m_stTargetUser,
                    TargetIt->second.kill_troop_force,
                    TargetIt->second.kill_fort_force,
                    pstSPlayer->m_nUid);

                if (udwResultCode == EN_REPORT_RESULT_LOSE)
                {
                    pstTWarStatis->ddwDfnWin++;
                }
                else if (udwResultCode == EN_REPORT_RESULT_WIN)
                {
                    pstTWarStatis->ddwDfnFail++;
                }
            }
        }
    }
    return;
}

TVOID CWarProcess::UpdateOccupyLairAction(SSession *pstSession, SUserInfo *pstUser, TbMap *ptbWild)
{
    TbMarch_action *ptbReqMarch = &pstSession->m_stReqMarch;
    TUINT32 udwCurtime = CTimeUtils::GetUnixTime();

    TUINT32 udwActionNum = 0;
    TUINT32 audwActionIdx[MAX_USER_ACTION_NUM];
    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
    {
        TbMarch_action *ptbAction = &pstUser->m_atbMarch[udwIdx];
        TSE_LOG_INFO(pstSession->m_poServLog, ("UpdateOccupyLairAction: act[%u]: class[%ld:%ld] stat[%ld] suid[%ld] tpos[%ld] [seq=%u]",
            udwIdx, ptbAction->m_nMclass, ptbAction->m_nSclass, ptbAction->m_nStatus, ptbAction->m_nSuid, ptbAction->m_nTpos, pstSession->m_udwSeqNo));
        TSE_LOG_INFO(pstSession->m_poServLog, ("UpdateOccupyLairAction:          btime[%ld] ctime[%ld] etime[%ld] [seq=%u]",
            ptbAction->m_nBtime, ptbAction->m_nCtime, ptbAction->m_nEtime, pstSession->m_udwSeqNo));

        if (ptbAction->m_nIs_recalled)
        {
            continue;
        }

        if (ptbReqMarch->m_nId != ptbAction->m_nId &&
            ptbAction->m_nMclass == EN_ACTION_MAIN_CLASS__MARCH && ptbAction->m_nSclass == EN_ACTION_SEC_CLASS__OCCUPY &&
            ptbAction->m_nTpos == ptbWild->m_nId && ptbAction->m_nStatus == EN_MARCH_STATUS__LOADING)
        {
            audwActionIdx[udwActionNum++] = udwIdx;
        }
        else if (ptbReqMarch->m_nId == ptbAction->m_nId &&
            ptbReqMarch->m_nStatus == EN_MARCH_STATUS__LOADING)
        {
            audwActionIdx[udwActionNum++] = udwIdx;

            // sync stat from reqmarch
//             TbMarch_action *ptbMarch = &pstUser->m_atbMarch[udwIdx];
//             ptbMarch->Set_Status(ptbReqMarch->m_nStatus);
//             ptbMarch->Set_Btime(ptbReqMarch->m_nBtime);
//             ptbMarch->Set_Ctime(ptbReqMarch->m_nCtime);
//             ptbMarch->Set_Etime(ptbReqMarch->m_nEtime);
//             ptbMarch->m_bParam[0].m_ddwLoadTime = ptbReqMarch->m_bParam[0].m_ddwLoadTime;
//             ptbMarch->m_bParam[0].m_ddwLoadRate = ptbReqMarch->m_bParam[0].m_ddwLoadRate;
//             ptbMarch->Set_Status(ptbReqMarch->m_nStatus);
//             pstUser->m_aucMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
        }
    }
    TSE_LOG_INFO(pstSession->m_poServLog, ("UpdateOccupyLairAction: occupy_num=%u [seq=%u]",
        udwActionNum, pstSession->m_udwSeqNo));

    TUINT32 audwLoadNum[MAX_USER_ACTION_NUM]; // troop max load num
    double adfLoadRate[MAX_USER_ACTION_NUM]; // per second
    TUINT32 audwLoadTime[MAX_USER_ACTION_NUM]; // max load time
    TUINT32 audwTotalLoadTime[MAX_USER_ACTION_NUM]; // max load time

    double dfRate = 0.0;
    TUINT32 udwResNum = CMapLogic::GetMapResTotalNum(ptbWild);
    TSE_LOG_INFO(pstSession->m_poServLog, ("UpdateOccupyLairAction: wild_res[%u] [seq=%u]", udwResNum, pstSession->m_udwSeqNo));
    for (TUINT32 udwIdx = 0; udwIdx < udwActionNum; ++udwIdx)
    {
        TbMarch_action *ptbAction = &pstUser->m_atbMarch[audwActionIdx[udwIdx]];
        if (ptbAction->m_nId == ptbReqMarch->m_nId)
        {
            ptbAction = ptbReqMarch;
        }
        if (ptbAction->m_bParam[0].m_ddwLoadRate == 0)
        {
            TINT64 ddwLoadTime = CMapLogic::ComputeLoadResTime(ptbAction, ptbWild);
            ptbAction->m_bParam[0].m_ddwLoadRate = udwResNum * 1.0 / ddwLoadTime * 3600;
            adfLoadRate[udwIdx] = ptbAction->m_bParam[0].m_ddwLoadRate * 1.0 / 3600;
        }
        else
        {
            adfLoadRate[udwIdx] = ptbAction->m_bParam[0].m_ddwLoadRate * 1.0 / 3600;
        }

        dfRate += adfLoadRate[udwIdx];
        audwLoadNum[udwIdx] = CCommonBase::GetTroopTotalLoad(ptbAction);
        audwLoadTime[udwIdx] = ceil(audwLoadNum[udwIdx] * 1.0 / adfLoadRate[udwIdx]);
        audwTotalLoadTime[udwIdx] = ceil(udwResNum * 1.0 / adfLoadRate[udwIdx]);
        TSE_LOG_INFO(pstSession->m_poServLog, ("UpdateOccupyLairAction: action[%u] load_rate[%.2lf/s] load_num[%u] load_time[%u] total_load_time[%u] [seq=%u]",
            udwIdx, adfLoadRate[udwIdx], audwLoadNum[udwIdx], audwLoadTime[udwIdx], audwTotalLoadTime[udwIdx], pstSession->m_udwSeqNo));
    }

    if (udwActionNum == 0)
    {
        return;
    }

    TUINT32 udwResTotal = 0;
    for (TUINT32 udwIdx = 0; udwIdx < udwActionNum; ++udwIdx)
    {
        udwResTotal += audwLoadNum[udwIdx];
    }

    if (udwResTotal <= udwResNum) // wild res enough
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("UpdateOccupyLairAction: res enough [seq=%u]", pstSession->m_udwSeqNo));
        for (TUINT32 udwIdx = 0; udwIdx < udwActionNum; ++udwIdx)
        {
            TbMarch_action *ptbAction = &pstUser->m_atbMarch[audwActionIdx[udwIdx]];
            if (ptbAction->m_nId == ptbReqMarch->m_nId)
            {
                ptbAction = ptbReqMarch;
                pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;
            }
            else
            {
                pstUser->m_aucMarchFlag[audwActionIdx[udwIdx]] = EN_TABLE_UPDT_FLAG__CHANGE;
            }
            TUINT32 udwEtime = ptbAction->m_nBtime + audwLoadTime[udwIdx];
            if (ptbWild->m_nExpire_time > 0 && udwEtime > ptbWild->m_nExpire_time)
            {
                udwEtime = ptbWild->m_nExpire_time;
            }
            ptbAction->Set_Etime(udwEtime);
            ptbAction->Set_Ctime(udwEtime - ptbAction->m_nBtime);
            ptbAction->m_bParam[0].m_ddwLoadTime = udwEtime - ptbAction->m_nBtime;
            ptbAction->m_bParam[0].m_ddwTotalLoadTime = audwTotalLoadTime[udwIdx];
            ptbAction->SetFlag(TbMARCH_ACTION_FIELD_PARAM);
        }
    }
    else // wild res not enough
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("UpdateOccupyLairAction: res not enough [seq=%u]", pstSession->m_udwSeqNo));
        TINT64 ddwResLeft = udwResNum;
        for (TUINT32 udwIdx = 0; udwIdx < udwActionNum; ++udwIdx)
        {
            TbMarch_action *ptbAction = &pstUser->m_atbMarch[audwActionIdx[udwIdx]];
            if (ptbAction->m_nId == ptbReqMarch->m_nId)
            {
                ptbAction = ptbReqMarch;
                pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;
            }
            ddwResLeft -= floor((udwCurtime - ptbAction->m_nBtime) * adfLoadRate[udwIdx]);
        }
        if (ddwResLeft < 0)
        {
            ddwResLeft = 0;
        }

        TUINT32 udwTotalEtime = udwCurtime;
        if (dfRate > 0)
        {
            udwTotalEtime += ceil(ddwResLeft / dfRate);
        }
        if (ptbWild->m_nExpire_time > 0 && udwTotalEtime > ptbWild->m_nExpire_time)
        {
            udwTotalEtime = ptbWild->m_nExpire_time;
        }

        TSE_LOG_INFO(pstSession->m_poServLog, ("UpdateOccupyLairAction: total_etime[%u] [seq=%u]", udwTotalEtime, pstSession->m_udwSeqNo));
        for (TUINT32 udwIdx = 0; udwIdx < udwActionNum; ++udwIdx)
        {
            TbMarch_action *ptbAction = &pstUser->m_atbMarch[audwActionIdx[udwIdx]];
            if (ptbAction->m_nId == ptbReqMarch->m_nId)
            {
                ptbAction = ptbReqMarch;
                pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;
            }
            else
            {
                pstUser->m_aucMarchFlag[audwActionIdx[udwIdx]] = EN_TABLE_UPDT_FLAG__CHANGE;
            }
            TUINT32 udwEtime = ptbAction->m_nBtime + audwLoadTime[udwIdx];
            if (udwTotalEtime < udwEtime)
            {
                udwEtime = udwTotalEtime;
            }
            TSE_LOG_INFO(pstSession->m_poServLog, ("UpdateOccupyLairAction: act[%u] calc_etime[%u] ori_etime[%u] [seq=%u]",
                udwIdx, udwEtime, ptbAction->m_nEtime, pstSession->m_udwSeqNo));
            ptbAction->Set_Etime(udwEtime);
            ptbAction->Set_Ctime(udwEtime - ptbAction->m_nBtime);
            ptbAction->m_bParam[0].m_ddwLoadTime = udwEtime - ptbAction->m_nBtime;
            //ptbAction->m_bParam[0].m_ddwTotalLoadTime = ptbAction->m_bParam[0].m_ddwLoadTime;
            ptbAction->m_bParam[0].m_ddwTotalLoadTime = audwTotalLoadTime[udwIdx];
            ptbAction->SetFlag(TbMARCH_ACTION_FIELD_PARAM);
            TSE_LOG_INFO(pstSession->m_poServLog, ("UpdateOccupyLairAction: act[%u] update final: btime[%ld] ctime[%ld] etime[%ld] [seq=%u]",
                udwIdx, ptbAction->m_nBtime, ptbAction->m_nCtime, ptbAction->m_nEtime, pstSession->m_udwSeqNo));
        }
    }

    // set reqmarch
//     for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
//     {
//         TbMarch_action *ptbAction = &pstUser->m_atbMarch[udwIdx];
//         if (ptbAction->m_nId == ptbReqMarch->m_nId)
//         {
//             pstUser->m_aucMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__UNCHANGE;
// 
//             ptbReqMarch->Set_Etime(ptbAction->m_nEtime);
//             ptbReqMarch->Set_Ctime(ptbAction->m_nCtime);
//             ptbReqMarch->m_bParam[0].m_ddwLoadTime = ptbAction->m_bParam[0].m_ddwLoadTime;
//             ptbReqMarch->SetFlag(TbMARCH_ACTION_FIELD_PARAM);
//             pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;
//         }
//     }

    for (TUINT32 udwIdx = 0; udwIdx < udwActionNum; ++udwIdx)
    {
        TbMarch_action *tbAction = &pstUser->m_atbMarch[audwActionIdx[udwIdx]];
        TSE_LOG_INFO(pstSession->m_poServLog, ("UpdateOccupyLairAction: end: %u: suid[%ld] etime[%ld] [seq=%u]",
            udwIdx, tbAction->m_nSuid, tbAction->m_nEtime, pstSession->m_udwSeqNo));
    }
}

TVOID CWarProcess::UpdateOccupyLairMap(SSession *pstSession, SUserInfo *pstUser, TbMap *ptbWild)
{
    TbMarch_action *ptbReqMarch = &pstSession->m_stReqMarch;

    // sync stat from reqmarch
    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
    {
        TbMarch_action *ptbMarch = &pstUser->m_atbMarch[udwIdx];
        if (ptbReqMarch->m_nId == ptbMarch->m_nId)
        {
            ptbMarch->Set_Status(ptbReqMarch->m_nStatus);
            ptbMarch->Set_Btime(ptbReqMarch->m_nBtime);
            ptbMarch->Set_Ctime(ptbReqMarch->m_nCtime);
            ptbMarch->Set_Etime(ptbReqMarch->m_nEtime);
            ptbMarch->m_bParam[0].m_ddwLoadTime = ptbReqMarch->m_bParam[0].m_ddwLoadTime;
            ptbMarch->m_bParam[0].m_ddwLoadRate = ptbReqMarch->m_bParam[0].m_ddwLoadRate;
            ptbMarch->Set_Status(ptbReqMarch->m_nStatus);
        }
    }

    CCommonBase::UpdateOccupyLairStat(pstUser, ptbWild);
    
    if (ptbWild->m_nOccupy_num == 0 && ptbWild->m_nUid == pstUser->m_tbPlayer.m_nUid)
    {
        CCommonBase::AbandonWild(&pstUser->m_stCityInfo, ptbWild);
    }
}

TVOID CWarProcess::SetBattleNodeForAttackIdol(SSession *pstSession, SBattleNode *pstAttacker, SBattleNode *pstDefender)
{
    TbMarch_action *ptbReqMarch = &pstSession->m_stReqMarch;
    SUserInfo *pstSUser = &pstSession->m_stSourceUser;
    TbMap *ptbWild = &pstSession->m_stMapItem;

    pstAttacker->m_ptbMainAttackAction = ptbReqMarch;

    pstDefender->m_ptbWild = ptbWild;

    for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_udwIdolNum; udwIdx++)
    {
        if (ptbWild->m_nId == pstSession->m_atbIdol[udwIdx].m_nPos)
        {
            pstDefender->m_ptbIdol = &pstSession->m_atbIdol[udwIdx];
            break;
        }
    }

    //进攻方军队
    CWarProcess::SetLeadAttackerArmy(ptbReqMarch, pstSUser, pstAttacker);

    CToolBase::AddUserToMailReceiverList(pstSUser->m_adwMailSendUidList, pstSUser->m_udwMailSendNum,
        CToolBase::GetAllianceUserReportKey(ptbReqMarch->m_bParam[0].m_ddwSourceAlliance));

    //防守方军队
    CWarProcess::SetIdolDefenderNode(pstDefender->m_ptbIdol, pstDefender);
}

TINT32 CWarProcess::SetIdolDefenderNode(TbIdol *ptbIdolDefender, SBattleNode *pstDefender)
{
    pstDefender->m_udwSeriesId = ptbIdolDefender->m_jInfo["t"][2U].asInt();

    if (ptbIdolDefender->m_jInfo["t"][0U].asInt() > 0)
    {
        pstDefender->m_bIsDragonJoin = TRUE;
        pstDefender->m_stDragon.m_ddwLevel = ptbIdolDefender->m_jInfo["t"][0U].asInt();
    }

    pstDefender->m_stKnight.ddwId = 0;
    pstDefender->m_stKnight.ddwLevel = ptbIdolDefender->m_jInfo["t"][1U].asInt();

    CWarBase::AddIdolArmy(ptbIdolDefender, pstDefender);

    SPlayerBuffInfo stBuff;
    stBuff.Reset();
    CBufferBase::GetIdolBattleBuff(ptbIdolDefender, &stBuff);

    CWarBase::SetBattleBuff(&stBuff, pstDefender);

    return 0;
}

TVOID CWarProcess::ComputeAttackIdolResult(SSession *pstSession, SBattleNode *pstAttacker, SBattleNode *pstDefender, TUINT32 udwBattleResult)
{
    TbMarch_action *ptbReqMarch = &pstSession->m_stReqMarch;
    SUserInfo *pstSUser = &pstSession->m_stSourceUser;
    TbPlayer *ptbSPlayer = &pstSUser->m_tbPlayer;
    SCityInfo *pstSCity = &pstSUser->m_stCityInfo;

    CWarProcess::AddDragonExp(pstSUser, pstSCity, pstAttacker, udwBattleResult);

    SetKnightResult(ptbReqMarch, pstAttacker, udwBattleResult);

    SetIdolResult(pstDefender->m_ptbIdol, ptbSPlayer, udwBattleResult);

}

TINT32 CWarProcess::SetIdolResult(TbIdol *ptbIdol, TbPlayer *ptbPlayer, TUINT32 udwBattleResult)
{
    if (udwBattleResult == EN_REPORT_RESULT_WIN && ptbPlayer->m_nAlpos != 0)
    {
        TUINT32 udwAlid = ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET;
        
        TBOOL bIsFind = FALSE;
        for (TUINT32 udwIdx = 0; udwIdx < ptbIdol->m_jRank.size(); udwIdx++)
        {
            if (ptbIdol->m_jRank[udwIdx]["alid"].asUInt() == udwAlid)
            {
                ptbIdol->m_jRank[udwIdx]["point"] = ptbIdol->m_jRank[udwIdx]["point"].asUInt() + 1;
                ptbIdol->m_jRank[udwIdx]["al_nick"] = ptbPlayer->m_sAl_nick_name;
                ptbIdol->m_jRank[udwIdx]["al_name"] = ptbPlayer->m_sAlname;
                ptbIdol->m_jRank[udwIdx]["time"] = CTimeUtils::GetUnixTime();
                ptbIdol->m_jRank[udwIdx]["rank"] = 0;
                bIsFind = TRUE;
                break;
            }
        }
        if (bIsFind == FALSE)
        {
            Json::Value jTmp = Json::Value(Json::objectValue);
            jTmp["alid"] = udwAlid;
            jTmp["point"] = 1;
            jTmp["al_nick"] = ptbPlayer->m_sAl_nick_name;
            jTmp["al_name"] = ptbPlayer->m_sAlname;
            jTmp["time"] = CTimeUtils::GetUnixTime();
            jTmp["rank"] = 0;
            ptbIdol->m_jRank.append(jTmp);
        }
        ptbIdol->SetFlag(TbIDOL_FIELD_RANK);
    }

    return 0;
}