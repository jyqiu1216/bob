#include "activities_logic.h"
#include "game_info.h"
#include "time_utils.h"
#include "city_info.h"
#include "service_key.h"
#include "player_info.h"
#include "city_base.h"
#include "player_base.h"
#include "common_base.h"
#include "sendmessage_base.h"
#include "buffer_base.h"
#include "globalres_logic.h"
#include "game_define.h"
#include "tool_base.h"
#include <vector>
#include "common_func.h"


//获得积分计算
TINT32 CActivitesLogic::ComputeTroopKillScore(SUserInfo *pstUser, TUINT32 udwTroopKillForce, TUINT32 udwFortKillForce, TINT64 ddwTUid)
{
    pstUser->m_stScore.audwScoreList[EN_SCORE_TYPE__KILL_FORCE][EN_SCORE_ID_KILL_FORCE__CITY_FORT] += udwFortKillForce;
    if (ddwTUid != 0)  //城里
    {
        pstUser->m_stScore.audwScoreList[EN_SCORE_TYPE__KILL_FORCE][EN_SCORE_ID_KILL_FORCE__CITY_TROOP] += udwTroopKillForce;
    }
    else  //野外
    {
        pstUser->m_stScore.audwScoreList[EN_SCORE_TYPE__KILL_FORCE][EN_SCORE_ID_KILL_FORCE__WILD_TROOP] += udwTroopKillForce;
    }
    return 0;
}

TINT32 CActivitesLogic::ComputeTroopKillScore(SUserInfo *pstUser, TUINT32 udwTroopKillForce, TUINT32 udwFortKillForce, TINT64 ddwTUid, 
                                                TINT64 ddwUid, TUINT32 udwSid, TUINT64 uddwAlid, string strUname)
{
    ComputeStructScoreList(pstUser, EN_SCORE_TYPE__KILL_FORCE, EN_SCORE_ID_KILL_FORCE__CITY_FORT, udwFortKillForce, ddwUid, udwSid, uddwAlid, strUname);
    if(ddwTUid != 0)  //城里
    {
        ComputeStructScoreList(pstUser, EN_SCORE_TYPE__KILL_FORCE, EN_SCORE_ID_KILL_FORCE__CITY_TROOP, udwTroopKillForce, ddwUid, udwSid, uddwAlid, strUname);
    }
    else  //野外
    {
        ComputeStructScoreList(pstUser, EN_SCORE_TYPE__KILL_FORCE, EN_SCORE_ID_KILL_FORCE__WILD_TROOP, udwTroopKillForce, ddwUid, udwSid, uddwAlid, strUname);
    }

    return 0;
}

TINT32 CActivitesLogic::ComputeTrainTroopScore(SUserInfo *pstUser, TUINT32 udwType, TUINT32 udwNum)
{
//     if (IfCalcScore(pstUser) == FALSE)
//     {
//         return 0;
//     }

    //根据force计算
    const Json::Value &rTroopJson = CGameInfo::GetInstance()->m_oJsonRoot["game_troop"];
    if(udwType > rTroopJson.size() - 1)
    {
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("ComputeTrainTroopScourt:troop idx out of range [seq=%u]", pstUser->m_udwBSeqNo));
        return -1;
    }

    //UINT32 udwSingleForce = rTroopJson[udwType]["a"]["a9"].asUInt();
    //TUINT32 udwTotalForce = udwSingleForce * udwNum;

    TUINT32 udwTotalForce = udwNum;
    pstUser->m_stScore.audwScoreList[EN_SCORE_TYPE__TRAIN_TROOP][udwType] += udwTotalForce;
    return 0;
}

TINT32 CActivitesLogic::ComputeTrainFortScore(SUserInfo *pstUser, TUINT32 udwType, TUINT32 udwNum)
{
//     if (IfCalcScore(pstUser) == FALSE)
//     {
//         return 0;
//     }

    //根据force计算
    const Json::Value &rFortJson = CGameInfo::GetInstance()->m_oJsonRoot["game_fort"];
    if(udwType > rFortJson.size() - 1)
    {
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("ComputeTrainFortScourt:troop idx out of range [seq=%u]", pstUser->m_udwBSeqNo));
        return -1;
    }
    //TUINT32 udwSingleForce = rFortJson[udwType]["a"]["a9"].asUInt();

    //TUINT32 udwTotalForce = udwSingleForce * udwNum;
    TUINT32 udwTotalForce = udwNum;
    pstUser->m_stScore.audwScoreList[EN_SCORE_TYPE__TRAIN_FORT][udwType] += udwTotalForce;
    return 0;
}

TINT32 CActivitesLogic::ComputeBuildUpgradeScore(SUserInfo *pstUser, TUINT32 udwBuildType, TUINT32 udwBuildLv)
{
//     if (IfCalcScore(pstUser) == FALSE)
//     {
//         return 0;
//     }

    //根据force计算
    const Json::Value &rBuildJson = CGameInfo::GetInstance()->m_oJsonRoot["game_building"];

    string sBuildingType = CCommonFunc::NumToString(udwBuildType);
    if(!rBuildJson.isMember(sBuildingType.c_str()))
    {
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("ComputeBuildUpgradeScourt:game json not such building type [seq=%u]", pstUser->m_udwBSeqNo));
        return -1;
    }

    //force
    const Json::Value &oRewardJson = rBuildJson[sBuildingType.c_str()]["b"]["b0"];
    if(udwBuildLv > oRewardJson.size())
    {
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("ComputeBuildUpgradeScourt:building lv out of range [seq=%u]", pstUser->m_udwBSeqNo));
        return -2;
    }

    string ForceBufferId = CCommonFunc::NumToString(EN_BUFFER_INFO_ADD_FORCE);

    if(udwBuildLv == 1)
    {
        if(oRewardJson[udwBuildLv - 1].isMember(ForceBufferId.c_str()))
        {
            pstUser->m_stScore.audwScoreList[EN_SCORE_TYPE__BUILDING_UPGRADE][udwBuildType] += oRewardJson[udwBuildLv - 1][ForceBufferId][1U].asUInt();
        }

    }
    else if(udwBuildLv > 1)
    {
        TUINT32 udwCurLvForce = 0;
        TUINT32 udwTarLvForce = 0;
        if(oRewardJson[udwBuildLv - 2].isMember(ForceBufferId.c_str()))
        {
            udwCurLvForce = oRewardJson[udwBuildLv - 2][ForceBufferId][1U].asUInt();
        }
        if(oRewardJson[udwBuildLv - 1].isMember(ForceBufferId.c_str()))
        {
            udwTarLvForce = oRewardJson[udwBuildLv - 1][ForceBufferId][1U].asUInt();
        }

        TUINT32 udwAddForce = udwTarLvForce > udwCurLvForce ? udwTarLvForce - udwCurLvForce : 0;
        pstUser->m_stScore.audwScoreList[EN_SCORE_TYPE__BUILDING_UPGRADE][udwBuildType] += udwAddForce;
    }

    return 0;
}

TINT32 CActivitesLogic::ComputeResearchUpgradeScore(SUserInfo *pstUser, TUINT32 udwType, TUINT32 udwLv)
{
//     if (IfCalcScore(pstUser) == FALSE)
//     {
//         return 0;
//     }

    //根据force计算
    const Json::Value &rResearchJson = CGameInfo::GetInstance()->m_oJsonRoot["game_research"];

    string sResearchType = CCommonFunc::NumToString(udwType);
    if(!rResearchJson.isMember(sResearchType.c_str()))
    {
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("ComputeBuildUpgradeScourt:game json not such research type [seq=%u]", pstUser->m_udwBSeqNo));
        return -1;
    }
    //force
    const Json::Value &oRewardJson = rResearchJson[sResearchType.c_str()]["b"]["b0"];
    if(udwLv > oRewardJson.size())
    {
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("ComputeBuildUpgradeScourt:research lv out of range [seq=%u]", pstUser->m_udwBSeqNo));
        return -2;
    }

    string ForceBufferId = CCommonFunc::NumToString(EN_BUFFER_INFO_ADD_FORCE);

    if(udwLv == 1)
    {
        if(oRewardJson[udwLv - 1].isMember(ForceBufferId.c_str()))
        {
            pstUser->m_stScore.audwScoreList[EN_SCORE_TYPE__RESEARCH_UPGRADE][udwType] += oRewardJson[udwLv - 1][ForceBufferId][1U].asUInt();
        }

    }
    else if(udwLv > 1)
    {
        TUINT32 udwCurLvForce = 0;
        TUINT32 udwTarLvForce = 0;
        if(oRewardJson[udwLv - 2].isMember(ForceBufferId.c_str()))
        {
            udwCurLvForce = oRewardJson[udwLv - 2][ForceBufferId][1U].asUInt();
        }
        if(oRewardJson[udwLv - 1].isMember(ForceBufferId.c_str()))
        {
            udwTarLvForce = oRewardJson[udwLv - 1][ForceBufferId][1U].asUInt();
        }

        TUINT32 udwAddForce = udwTarLvForce > udwCurLvForce ? udwTarLvForce - udwCurLvForce : 0;
        pstUser->m_stScore.audwScoreList[EN_SCORE_TYPE__RESEARCH_UPGRADE][udwType] += udwAddForce;
    }

    return 0;
}

TINT32 CActivitesLogic::ComputeHealTroopScore(SUserInfo *pstUser, TUINT32 udwType, TUINT32 udwNum)
{
//     if (IfCalcScore(pstUser) == FALSE)
//     {
//         return 0;
//     }

    TSE_LOG_INFO(CGameInfo::GetInstance()->m_poLog, ("EventReqInfo: Heal. [udwType=%d][udwNum=%d][seq=%u]", udwType, udwNum, pstUser->m_udwBSeqNo));
    const Json::Value &rTroopJson = CGameInfo::GetInstance()->m_oJsonRoot["game_troop"];
    if(udwType > rTroopJson.size() - 1)
    {
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("ComputeTrainTroopScourt:troop idx out of range [seq=%u]", pstUser->m_udwBSeqNo));
        return -1;
    }
    TUINT32 udwSingleForce = rTroopJson[udwType]["a"]["a9"].asUInt();

    TUINT32 udwTotalForce = udwSingleForce * udwNum;

    pstUser->m_stScore.audwScoreList[EN_SCORE_TYPE__HEAL_TROOP][udwType] += (TUINT32)round(1.0 * udwTotalForce / 5);
    return 0;
}


TINT32 CActivitesLogic::ComputeHealFortScore( SUserInfo *pstUser, TUINT32 udwType, TUINT32 udwNum )
{
    TSE_LOG_INFO(CGameInfo::GetInstance()->m_poLog, ("EventReqInfo: Heal Fort. [udwType=%d][udwNum=%d][seq=%u]", udwType, udwNum, pstUser->m_udwBSeqNo));
    const Json::Value &rTroopJson = CGameInfo::GetInstance()->m_oJsonRoot["game_fort"];
    if(udwType > rTroopJson.size() - 1)
    {
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("ComputeHealFortScore:fort idx out of range [seq=%u]", pstUser->m_udwBSeqNo));
        return -1;
    }
    TUINT32 udwSingleForce = rTroopJson[udwType]["a"]["a9"].asUInt();

    TUINT32 udwTotalForce = udwSingleForce * udwNum;

    pstUser->m_stScore.audwScoreList[EN_SCORE_TYPE__HEAL_TROOP][udwType] += (TUINT32)round(1.0 * udwTotalForce / 5);
    return 0;
}

TINT32 CActivitesLogic::ComputeHeroExpMarchScore(SUserInfo *pstUser, TUINT32 udwExp)
{
//     if (IfCalcScore(pstUser) == FALSE)
//     {
//         return 0;
//     }
    pstUser->m_stScore.audwScoreList[EN_SCORE_TYPE__HERO_EXP][EN_SCORE_ID_HERO_EXP__MARCH] += (TUINT32)round(1.0 * udwExp / 450);
    return 0;
}

TINT32 CActivitesLogic::ComputeHeroExpItemScore(SUserInfo *pstUser, TUINT32 udwItemId)
{
//     if (IfCalcScore(pstUser) == FALSE)
//     {
//         return 0;
//     }

    const Json::Value &rItemJson = CGameInfo::GetInstance()->m_oJsonRoot["game_item"];

    string sItemId = CCommonFunc::NumToString(udwItemId);
    if(!rItemJson.isMember(sItemId.c_str()))
    {
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("ComputeKnightExpItemScourt:not such item id [id=%u] [seq=%u]", udwItemId, pstUser->m_udwBSeqNo));
        return -1;
    }
    TUINT32 udwItemExp = rItemJson[sItemId.c_str()]["a14"][0U][2U].asUInt();
    pstUser->m_stScore.audwScoreList[EN_SCORE_TYPE__HERO_EXP][EN_SCORE_ID_HERO_EXP__ITEM] += (TUINT32)round(1.0 * udwItemExp / 450);
    return 0;
}


TINT32 CActivitesLogic::ComputeHeroExpResearchScore(SUserInfo *pstUser, TUINT32 udwExp)
{
//     if (IfCalcScore(pstUser) == FALSE)
//     {
//         return 0;
//     }
    pstUser->m_stScore.audwScoreList[EN_SCORE_TYPE__HERO_EXP][EN_SCORE_ID_HERO_EXP__RESEARCH] += (TUINT32)round(1.0 * udwExp / 450);
    return 0;
}


TINT32 CActivitesLogic::ComputeHeroExpBuildingScore(SUserInfo *pstUser, TUINT32 udwExp)
{
//     if (IfCalcScore(pstUser) == FALSE)
//     {
//         return 0;
//     }
    pstUser->m_stScore.audwScoreList[EN_SCORE_TYPE__HERO_EXP][EN_SCORE_ID_HERO_EXP__BUILDING] += (TUINT32)round(1.0 * udwExp / 450);
    return 0;
}

TINT32 CActivitesLogic::ComputeSpeedUpItemScore(SUserInfo *pstUser, TUINT32 udwItemId)
{
//     if (IfCalcScore(pstUser) == FALSE)
//     {
//         return 0;
//     }

    const Json::Value &rItemJson = CGameInfo::GetInstance()->m_oJsonRoot["game_item"];
    string sItemId = CCommonFunc::NumToString(udwItemId);
    if(!rItemJson.isMember(sItemId.c_str()))
    {
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("ComputeKnightExpItemScourt:not such item id [id=%u] [seq=%u]", udwItemId, pstUser->m_udwBSeqNo));
        return -1;
    }
    TUINT32 udwItemTime = rItemJson[sItemId.c_str()]["a14"][0U][2U].asUInt();
    TUINT32 udwItemEffect = rItemJson[sItemId.c_str()]["a2"].asUInt();
    TUINT32 udwItemPrice = rItemJson[sItemId.c_str()]["a5"].asUInt();
    if(udwItemEffect == 10001)
    {
        pstUser->m_stScore.audwScoreList[EN_SCORE_TYPE__SPEED_UP][EN_SCORE_ID_SPEED_UP__ITEM] += (TUINT32)round(1.0 * udwItemTime / 60);
    }
    else
    {
        pstUser->m_stScore.audwScoreList[EN_SCORE_TYPE__SPEED_UP][EN_SCORE_ID_SPEED_UP__ITEM] += (TUINT32)round(udwItemPrice * 0.75);
    }
    return 0;
}

TINT32 CActivitesLogic::ComputeSpeedUpGemScore(SUserInfo *pstUser, TUINT32 udwTime)
{
//     if (IfCalcScore(pstUser) == FALSE)
//     {
//         return 0;
//     }
    pstUser->m_stScore.audwScoreList[EN_SCORE_TYPE__SPEED_UP][EN_SCORE_ID_SPEED_UP__GEM] += (TUINT32)round(1.0 * udwTime / 60);
    return 0;
}

TINT32 CActivitesLogic::ComputeBuyGemScore(SUserInfo *pstUser, TUINT32 udwGemNum)
{
//     if (IfCalcScore(pstUser) == FALSE)
//     {
//         return 0;
//     }
    pstUser->m_stScore.audwScoreList[EN_SCORE_TYPE__BUY_GEM][EN_SCORE_ID__NORMAL] += udwGemNum * 1.5;
    return 0;
}

TINT32 CActivitesLogic::ComputeCostGemScore(SUserInfo *pstUser, TUINT32 udwGemNum)
{
//     if (IfCalcScore(pstUser) == FALSE)
//     {
//         return 0;
//     }
    pstUser->m_stScore.audwScoreList[EN_SCORE_TYPE__COST_GEM][EN_SCORE_ID__NORMAL] += udwGemNum * 1.5;
    return 0;
}

TINT32 CActivitesLogic::ComputeEquipUpgradeScore(SUserInfo *pstUser, TUINT32 udwTargetLv)
{
//     if (IfCalcScore(pstUser) == FALSE)
//     {
//         return 0;
//     }

    TUINT32 audwEquipUpgradeCost[MAX_EQUIP_LV + 1] = {0, 224, 296, 416, 584, 800,
        1064};
    TUINT32 udwToTalScore = audwEquipUpgradeCost[udwTargetLv] * 1.5;
    pstUser->m_stScore.audwScoreList[EN_SCORE_TYPE__EQUIP_UPGRADE][udwTargetLv] += udwToTalScore;
    return 0;
}

TINT32 CActivitesLogic::ComputeTrialAttackScore(SUserInfo *pstUser, TUINT32 udwTrialMode, TUINT32 udwAtkTime)
{
//     if (IfCalcScore(pstUser) == FALSE)
//     {
//         return 0;
//     }

    TUINT32 udwPoint = 0;
    switch (udwTrialMode)
    {
    case EN_TRIAL_MODE__NORMAL:
        udwPoint = 1;
        break;
    case EN_TRIAL_MODE__RAGE:
        udwPoint = 50;
        break;
    default:
        break;
    }
    pstUser->m_stScore.audwScoreList[EN_SCORE_TYPE__TRIAL_ATTACK][EN_SCORE_ID_TRIAL_ATTACK__ONE_TIME] += udwAtkTime * udwPoint;
    return 0;
}

TINT32 CActivitesLogic::ComputeBuyIapScore(SUserInfo *pstUser, TUINT32 udwPayCent)
{
//     if (IfCalcScore(pstUser) == FALSE)
//     {
//         return 0;
//     }
    pstUser->m_stScore.audwScoreList[EN_SCORE_TYPE__BUY_IAP][EN_SCORE_ID_BUY_IAP__ONE_CENT] += udwPayCent;
    return 0;
}

TINT32 CActivitesLogic::ComputeAttackMonsterScore(SUserInfo *pstUser, TINT32 udwMonsterId, TINT32 udwMonsterLv, TINT32 dwAttackNum,
                                                    TINT64 ddwUid, TUINT32 udwSid, TUINT64 uddwAlid, string strUname)
{
    TINT32 dwScoreId = 10 * udwMonsterId + udwMonsterLv;

    ComputeStructScoreList(pstUser, EN_SCORE_TYPE__ATTACK_MONSTER, dwScoreId, dwAttackNum, ddwUid, udwSid, uddwAlid, strUname);

    return 0;
}

TINT32 CActivitesLogic::ComputeKillMonsterScore(SUserInfo *pstUser, TINT32 udwMonsterId, TINT32 udwMonsterLv, 
                                                TINT64 ddwUid, TUINT32 udwSid, TUINT64 uddwAlid, string strUname)
{
    TINT32 dwScoreId = 10 * udwMonsterId + udwMonsterLv;

    ComputeStructScoreList(pstUser, EN_SCORE_TYPE__KILL_MONSTER, dwScoreId, 1, ddwUid, udwSid, uddwAlid, strUname);


    return 0;
}

TUINT32 CActivitesLogic::ComputeStructScoreList(SUserInfo *pstUser, TUINT32 udwScoreType, TUINT32 udwScoreId, TUINT32 udwBaseScore,
                                                TINT64 ddwUid, TUINT32 udwSid, TUINT64 uddwAlid, string strUname)
{
    if (udwBaseScore == 0)
    {
        return 0;
    }
    for (TUINT32 udwIdx = 0; udwIdx < MAX_SCORE_LIST_LENGTH; udwIdx++)
    {
        if (pstUser->m_stScore.sScoreList[udwIdx].ddwUid == 0)
        {
            pstUser->m_stScore.sScoreList[udwIdx].udwScoreType = udwScoreType;
            pstUser->m_stScore.sScoreList[udwIdx].udwScoreId = udwScoreId;
            pstUser->m_stScore.sScoreList[udwIdx].udwScore += udwBaseScore;
            pstUser->m_stScore.sScoreList[udwIdx].ddwUid = ddwUid;
            pstUser->m_stScore.sScoreList[udwIdx].udwSid = udwSid;
            pstUser->m_stScore.sScoreList[udwIdx].uddwAlid = uddwAlid;
            pstUser->m_stScore.sScoreList[udwIdx].strUname = strUname;
            break;
        }
        else if (pstUser->m_tbPlayer.m_nUid == pstUser->m_stScore.sScoreList[udwIdx].ddwUid)
        {
            if (pstUser->m_stScore.sScoreList[udwIdx].udwScoreType == udwScoreType
                && pstUser->m_stScore.sScoreList[udwIdx].udwScoreId == udwScoreId)
            {
                pstUser->m_stScore.sScoreList[udwIdx].udwScore += udwBaseScore;
                break;
            }
        }
    }
    return 0;
}

TINT32 CActivitesLogic::ComputeResCollectScore(SUserInfo *pstUser, TUINT32 udwType, TUINT32 udwNum)
{
    pstUser->m_stScore.audwScoreList[EN_SCORE_TYPE__RES_COLLECT][udwType] += udwNum / 1000;
    return 0;
}

TBOOL CActivitesLogic::IfCalcScore(SUserInfo *pstUser)
{
    if (CCityBase::GetBuildingLevelByFuncType(&pstUser->m_stCityInfo, EN_BUILDING_TYPE__TRIAL) == 0)
    {
        return FALSE;
    }

    return TRUE;
}