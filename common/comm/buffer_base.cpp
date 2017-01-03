#include "buffer_base.h"
#include "game_info.h"
#include "action_base.h"
#include "time_utils.h"
#include "city_base.h"
#include "player_base.h"
#include "common_base.h"
#include "common_func.h"
#include "tool_base.h"
#include "game_define.h"
#include "backpack_logic.h"
#include "common_logic.h"

TINT32 CBufferBase::ComputeBuffInfo(SCityInfo *pstCity, SUserInfo *pstUser, TbIdol *atbIdol, TUINT32 udwIdolNum, TbThrone *ptbThrone, STitleInfoList *pstTitle)
{
    if(pstCity == NULL || pstUser == NULL)
    {
        return -1;
    }

    TBOOL bComputeDragonBuff = TRUE;
    //英雄被抓或处死 会影响的buff
    if(pstUser->m_tbPlayer.m_nDragon_status == EN_DRAGON_STATUS_BEING_ESCORT ||
        pstUser->m_tbPlayer.m_nDragon_status == EN_DRAGON_STATUS_WAIT_KILL ||
        pstUser->m_tbPlayer.m_nDragon_status == EN_DRAGON_STATUS_WAIT_RELEASE ||
        pstUser->m_tbPlayer.m_nDragon_status == EN_DRAGON_STATUS_DEAD)
    {
        bComputeDragonBuff = FALSE;
    }
    pstUser->m_stPlayerBuffList.Reset();

    //basic
    ComputeBasicBuff(pstCity, pstUser);

    //dragon skill
    if(bComputeDragonBuff)
    {
        ComputeDragonSkillBuff(pstCity, pstUser);
    }    

    //dragon monster skill
    ComputeDragonMonsterSkillBuff(pstCity, pstUser);

    //lord skill
    ComputeLordSkillBuff(pstCity, pstUser);

    //research 
    ComputeResearchBuff(pstCity, pstUser);

    //item
    ComputeItemBuff(pstCity, pstUser);

    //wild
    ComputeWildBuff(pstCity, pstUser);

    //vip
    ComputeVipBuff(pstCity, pstUser);

    //equip
    if(bComputeDragonBuff)
    {
        ComputeEquipBuff(pstCity, pstUser);
    }

    //altar
    ComputeAltarBuff(pstCity, pstUser);

    //fort
    ComputeFortBuff(pstCity, pstUser);

    //troop
    ComputeTroopBuff(pstCity, pstUser);

    //build
    ComputeBuildingBuff(pstCity, pstUser);

    //prison
    ComputeCaptureDragonBuff(pstCity, pstUser);

    //dragon lv
    ComputeDragonLvBuff(pstCity, pstUser);

    //lord lv
    ComputeLordLvBuff(pstCity, pstUser);

    //knight
    ComputeKnightBuff(pstCity, pstUser);

    //idol
    ComputeIdolBuff(atbIdol, udwIdolNum, pstUser);

    //throne
    if (ptbThrone)
    {
        ComputeThroneBuff(ptbThrone, pstUser);
        //title
        if (pstTitle)
        {
            ComputeTitleBuff(pstTitle, ptbThrone, pstUser);
        }
    }

    pstUser->m_stBuffWithoutDragon = pstUser->m_stPlayerBuffList;

    CBufferBase::RemoveDragonBuff(&pstUser->m_stBuffWithoutDragon);

    return 0;
}

TINT32 CBufferBase::ComputeBuildingBuff(SCityInfo *pstCity, SUserInfo *pstUser)
{
    TUINT32 udwType = 0;
    TUINT32 udwLv = 0;
    const Json::Value& oBuildNode = CGameInfo::GetInstance()->m_oJsonRoot["game_building"];
    for(TUINT32 udwIdx = 0; udwIdx < pstCity->m_stTblData.m_bBuilding.m_udwNum; ++udwIdx)
    {
        SCityBuildingNode* pstBuildingNode = &pstCity->m_stTblData.m_bBuilding[udwIdx];
        udwType = pstBuildingNode->m_ddwType;
        udwLv = pstBuildingNode->m_ddwLevel;
        if(udwLv == 0)
        {
            continue;
        }

        TCHAR szBuildType[32];
        snprintf(szBuildType, 32, "%u", udwType);

        const Json::Value& oBuilBuff = oBuildNode[szBuildType]["b"]["b0"][udwLv - 1];
        Json::Value::Members member = oBuilBuff.getMemberNames();
        for(Json::Value::Members::iterator iter = member.begin(); iter != member.end(); ++iter)
        {
            const std::string &key = *iter;
            TUINT32 udwBufferId = oBuilBuff[key][0U].asUInt();
            TINT32 dwBufferNum = oBuilBuff[key][1U].asInt();

            pstUser->m_stPlayerBuffList[udwBufferId].m_udwBuffId = udwBufferId;
            pstUser->m_stPlayerBuffList[udwBufferId].m_astBuffDetail[EN_BUFF_TYPE_BUILDING].m_udwId = udwBufferId;
            pstUser->m_stPlayerBuffList[udwBufferId].m_astBuffDetail[EN_BUFF_TYPE_BUILDING].m_ddwNum += dwBufferNum;
            pstUser->m_stPlayerBuffList[udwBufferId].m_astBuffDetail[EN_BUFF_TYPE_BUILDING].m_dwTime = 0;
            pstUser->m_stPlayerBuffList[udwBufferId].m_ddwBuffTotal += dwBufferNum;
        }

        //if (CCommonLogic::CheckFuncOpen(pstUser, pstCity, EN_FUNC_OPEN_TYPE_TOP_BUFF))
        if(1)
        {
            const Json::Value& oBuildTopBuff = oBuildNode[szBuildType]["b"]["b4"][udwLv - 1];
            member = oBuildTopBuff.getMemberNames();
            for (Json::Value::Members::iterator iter = member.begin(); iter != member.end(); ++iter)
            {
                const std::string &key = *iter;
                TUINT32 udwBufferId = oBuildTopBuff[key][0U].asUInt();
                TINT32 dwBufferNum = oBuildTopBuff[key][1U].asInt();

                pstUser->m_stPlayerBuffList[udwBufferId].m_udwBuffId = udwBufferId;
                pstUser->m_stPlayerBuffList[udwBufferId].m_astBuffDetail[EN_BUFF_TYPE_BUILDING].m_udwId = udwBufferId;
                pstUser->m_stPlayerBuffList[udwBufferId].m_astBuffDetail[EN_BUFF_TYPE_BUILDING].m_ddwNum += dwBufferNum;
                pstUser->m_stPlayerBuffList[udwBufferId].m_astBuffDetail[EN_BUFF_TYPE_BUILDING].m_dwTime = 0;
                pstUser->m_stPlayerBuffList[udwBufferId].m_ddwBuffTotal += dwBufferNum;
            }
        }
    }
    return 0;
}

TINT32 CBufferBase::ComputeCaptureDragonBuff(SCityInfo *pstCity, SUserInfo *pstUser)
{
    TUINT32 udwBuildingLevel = CCityBase::GetBuildingLevelById(&pstCity->m_stTblData, 59);
    if (udwBuildingLevel == 0)
    {
        return 0;
    }

    TINT32 dwMaxLv = 0;
    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwPassiveMarchNum; udwIdx++)
    {
        if (pstUser->m_atbPassiveMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__PRISON_TIMER)
        {
            if (dwMaxLv < pstUser->m_atbPassiveMarch[udwIdx].m_bPrison_param[0].stDragon.m_ddwLevel)
            {
                dwMaxLv = pstUser->m_atbPassiveMarch[udwIdx].m_bPrison_param[0].stDragon.m_ddwLevel;
            }
        }
    }
    
    if (dwMaxLv == 0)
    {
        return 0;
    }

    const Json::Value& jCaptureDragonBuff = CGameInfo::GetInstance()->m_oJsonRoot["game_capture_dragon_buff"][dwMaxLv - 1][udwBuildingLevel-1];
    TINT32 dwBufferNum = 0;
    TINT32 dwBuffId = 0;
    Json::Value::Members members = jCaptureDragonBuff.getMemberNames();
    for (Json::Value::Members::iterator it = members.begin(); it != members.end(); it++)
    {
        dwBuffId = jCaptureDragonBuff[*it][0U].asInt();
        dwBufferNum = jCaptureDragonBuff[*it][1U].asInt();
        pstUser->m_stPlayerBuffList[dwBuffId].m_udwBuffId = dwBuffId;
        pstUser->m_stPlayerBuffList[dwBuffId].m_astBuffDetail[EN_BUFF_TYPE_BUILDING].m_udwId = dwBuffId;
        pstUser->m_stPlayerBuffList[dwBuffId].m_astBuffDetail[EN_BUFF_TYPE_BUILDING].m_ddwNum += dwBufferNum;
        pstUser->m_stPlayerBuffList[dwBuffId].m_astBuffDetail[EN_BUFF_TYPE_BUILDING].m_dwTime = 0;
        pstUser->m_stPlayerBuffList[dwBuffId].m_ddwBuffTotal += dwBufferNum;
    }

    return 0;
}

TINT32 CBufferBase::ComputeResearchBuff(SCityInfo *pstCity, SUserInfo *pstUser)
{
    SCommonResearch* pstResearch = &pstUser->m_stCityInfo.m_stTblData.m_bResearch[0];
    const Json::Value& oJsonResearh = CGameInfo::GetInstance()->m_oJsonRoot["game_research"];
    for(TUINT32 udwIdx = 0; udwIdx < EN_RESEARCH_TYPE__END; ++udwIdx)
    {
        if(pstResearch->m_addwLevel[udwIdx] <= 0)
        {
            continue;
        }
        string strResearchId = CCommonFunc::NumToString(udwIdx);
        TINT32 dwLv = pstResearch->m_addwLevel[udwIdx] - 1;
        const Json::Value& oJsonResearchBuff = oJsonResearh[strResearchId]["b"]["b0"][dwLv];
        Json::Value::Members oBuffIds = oJsonResearchBuff.getMemberNames();
        for(Json::Value::Members::iterator it = oBuffIds.begin(); it != oBuffIds.end(); ++it)
        {
            TUINT32 udwBuffId = oJsonResearchBuff[(*it)][0u].asUInt();
            TINT32 dwBuffNum = oJsonResearchBuff[(*it)][1u].asInt();

            pstUser->m_stPlayerBuffList[udwBuffId].m_udwBuffId = udwBuffId;
            pstUser->m_stPlayerBuffList[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_RESEARCH].m_udwId = udwBuffId;
            pstUser->m_stPlayerBuffList[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_RESEARCH].m_ddwNum += dwBuffNum;
            pstUser->m_stPlayerBuffList[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_RESEARCH].m_dwTime = 0;
            pstUser->m_stPlayerBuffList[udwBuffId].m_ddwBuffTotal += dwBuffNum;
        }
    }
    return 0;
}

TINT32 CBufferBase::ComputeItemBuff(SCityInfo *pstCity, SUserInfo *pstUser)
{
    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwActionNum; ++udwIdx)
    {
        if (pstUser->m_aucActionFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }
        if(pstUser->m_atbAction[udwIdx].m_nMclass == EN_ACTION_MAIN_CLASS__ITEM)
        {
            TINT32 dwBufferNum = pstUser->m_atbAction[udwIdx].m_bParam[0].m_stItem.m_ddwNum;

            TUINT32 udwBufferId = pstUser->m_atbAction[udwIdx].m_bParam[0].m_stItem.m_ddwBufferId;
            TUINT32 udwBufferEndTime = pstUser->m_atbAction[udwIdx].Get_Etime();

            pstUser->m_stPlayerBuffList[udwBufferId].m_udwBuffId = udwBufferId;
            pstUser->m_stPlayerBuffList[udwBufferId].m_astBuffDetail[EN_BUFF_TYPE_ITEM].m_udwId = udwBufferId;
            if(udwBufferEndTime < CTimeUtils::GetUnixTime())
            {
                continue;
            }
            pstUser->m_stPlayerBuffList[udwBufferId].m_astBuffDetail[EN_BUFF_TYPE_ITEM].m_ddwNum = dwBufferNum;
            pstUser->m_stPlayerBuffList[udwBufferId].m_astBuffDetail[EN_BUFF_TYPE_ITEM].m_dwTime = udwBufferEndTime;

            pstUser->m_stPlayerBuffList[udwBufferId].m_ddwBuffTotal += dwBufferNum;
        }
    }
    return 0;

}


TINT32 CBufferBase::ComputeWildBuff(SCityInfo *pstCity, SUserInfo *pstUser)
{
    //TODO
    return 0;
}

TINT32 CBufferBase::ComputeVipBuff(SCityInfo *pstCity, SUserInfo *pstUser)
{
    //TINT32 dwVipLevel = CPlayerBase::GetVipLevel(&pstUser->m_tbPlayer);
    //if(dwVipLevel > 0)
    //{
    //    const Json::Value& oJsonVipBuff = CGameInfo::GetInstance()->m_oJsonRoot["game_vip"]["b"]["b0"][dwVipLevel - 1];
    //    Json::Value::Members oJsonVipBuffIds = oJsonVipBuff.getMemberNames();
    //    for(Json::Value::Members::iterator it = oJsonVipBuffIds.begin(); it != oJsonVipBuffIds.end(); ++it)
    //    {
    //        TUINT32 udwBuffId = oJsonVipBuff[(*it)][0u].asUInt();
    //        TINT32 dwBuffNum = oJsonVipBuff[(*it)][1u].asInt();

    //        pstUser->m_stPlayerBuffList[udwBuffId].m_udwBuffId = udwBuffId;
    //        pstUser->m_stPlayerBuffList[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_VIP].m_udwId = udwBuffId;
    //        pstUser->m_stPlayerBuffList[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_VIP].m_ddwNum += dwBuffNum;
    //        pstUser->m_stPlayerBuffList[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_VIP].m_dwTime = 0;
    //        pstUser->m_stPlayerBuffList[udwBuffId].m_ddwBuffTotal += dwBuffNum;
    //    }
    //}
    //return 0;

    TINT32 dwVipLevel = CPlayerBase::GetVipLevel(&pstUser->m_tbPlayer);
    if (dwVipLevel > 0)
    {
        const Json::Value& oJsonVipBuff = CGameInfo::GetInstance()->m_oJsonRoot["game_vip_new"][dwVipLevel - 1]["b"];
        for (TUINT32 udwIdx = 0; udwIdx < oJsonVipBuff.size(); udwIdx++)
        {
            Json::Value::Members jMember = oJsonVipBuff[udwIdx].getMemberNames();
            for (TUINT32 udwIdy = 0; udwIdy < jMember.size(); udwIdy++)
            {
                TUINT32 udwBuffId = oJsonVipBuff[udwIdx][jMember[udwIdy]][0u].asInt();
                TINT32 dwBuffNum = oJsonVipBuff[udwIdx][jMember[udwIdy]][1u].asInt();
                pstUser->m_stPlayerBuffList[udwBuffId].m_udwBuffId = udwBuffId;
                pstUser->m_stPlayerBuffList[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_VIP].m_udwId = udwBuffId;
                pstUser->m_stPlayerBuffList[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_VIP].m_ddwNum += dwBuffNum;
                pstUser->m_stPlayerBuffList[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_VIP].m_dwTime = 0;
                pstUser->m_stPlayerBuffList[udwBuffId].m_ddwBuffTotal += dwBuffNum;
            }
        }
    }
    return 0;
}

TINT32 CBufferBase::ComputeBasicBuff(SCityInfo *pstCity, SUserInfo *pstUser)
{
    const Json::Value &rBasicBuffJson = CGameInfo::GetInstance()->m_oJsonRoot["game_basic_buff"];
    const Json::Value &jBuffFunInfo = CGameInfo::GetInstance()->m_oJsonRoot["game_buff_func_info"];
    Json::Value::Members oBuffIds = rBasicBuffJson.getMemberNames();
    for(Json::Value::Members::iterator it = oBuffIds.begin(); it != oBuffIds.end(); ++it)
    {
        TUINT32 udwBufferId = rBasicBuffJson[(*it).c_str()][0U].asUInt();
        TINT32 dwBuffNum = rBasicBuffJson[(*it).c_str()][1U].asInt();

        if (pstUser->m_tbPlayer.m_nHas_dragon == 0 && jBuffFunInfo[(*it).c_str()]["a4"].asUInt() == EN_BUFF_PROPERTY__DRAGON)
        {
            continue;
        }

        pstUser->m_stPlayerBuffList[udwBufferId].m_udwBuffId = udwBufferId;
        pstUser->m_stPlayerBuffList[udwBufferId].m_astBuffDetail[EN_BUFF_TYPE_BASIC].m_udwId = udwBufferId;
        pstUser->m_stPlayerBuffList[udwBufferId].m_astBuffDetail[EN_BUFF_TYPE_BASIC].m_ddwNum = dwBuffNum;
        pstUser->m_stPlayerBuffList[udwBufferId].m_astBuffDetail[EN_BUFF_TYPE_BASIC].m_dwTime = 0;

        pstUser->m_stPlayerBuffList[udwBufferId].m_ddwBuffTotal += dwBuffNum;

    }
    return 0;
}

TINT32 CBufferBase::ComputeLordSkillBuff(SCityInfo *pstCity, SUserInfo *pstUser)
{
    const Json::Value& oJsonSkill = CGameInfo::GetInstance()->m_oJsonRoot["game_lord_skill"];
    SSkill* pstSkill = &pstUser->m_tbUserStat.m_bLord_skill[0];
    for (TUINT32 udwIdx = 0; udwIdx < EN_SKILL_TYPE__END; ++udwIdx)
    {
        if (pstSkill->m_addwLevel[udwIdx] == 0)
        {
            continue;
        }
        string strSkillId = CCommonFunc::NumToString(udwIdx);
        const Json::Value& oJsonSkillBuff = oJsonSkill[strSkillId]["b"]["b0"][(TUINT32)pstSkill->m_addwLevel[udwIdx] - 1];
        Json::Value::Members oJsonSkillBuffIds = oJsonSkillBuff.getMemberNames();
        for (Json::Value::Members::iterator it = oJsonSkillBuffIds.begin(); it != oJsonSkillBuffIds.end(); ++it)
        {
            TUINT32 udwBuffId = oJsonSkillBuff[(*it)][0u].asUInt();
            TINT32 dwBuffNum = oJsonSkillBuff[(*it)][1u].asInt();

            pstUser->m_stPlayerBuffList[udwBuffId].m_udwBuffId = udwBuffId;
            pstUser->m_stPlayerBuffList[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_LORD_SKILL].m_udwId = udwBuffId;
            pstUser->m_stPlayerBuffList[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_LORD_SKILL].m_ddwNum += dwBuffNum;
            pstUser->m_stPlayerBuffList[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_LORD_SKILL].m_dwTime = 0;
            pstUser->m_stPlayerBuffList[udwBuffId].m_ddwBuffTotal += dwBuffNum;
        }
    }
    return 0;
}

TINT32 CBufferBase::ComputeDragonSkillBuff(SCityInfo *pstCity, SUserInfo *pstUser)
{
    const Json::Value& oJsonSkill = CGameInfo::GetInstance()->m_oJsonRoot["game_dragon_skill"];
    SSkill* pstSkill = &pstUser->m_tbUserStat.m_bDragon_skill[0];
    for (TUINT32 udwIdx = 0; udwIdx < EN_SKILL_TYPE__END; ++udwIdx)
    {
        if (pstSkill->m_addwLevel[udwIdx] == 0)
        {
            continue;
        }
        string strSkillId = CCommonFunc::NumToString(udwIdx);
        const Json::Value& oJsonSkillBuff = oJsonSkill[strSkillId]["b"]["b0"][(TUINT32)pstSkill->m_addwLevel[udwIdx] - 1];
        Json::Value::Members oJsonSkillBuffIds = oJsonSkillBuff.getMemberNames();
        for (Json::Value::Members::iterator it = oJsonSkillBuffIds.begin(); it != oJsonSkillBuffIds.end(); ++it)
        {
            TUINT32 udwBuffId = oJsonSkillBuff[(*it)][0u].asUInt();
            TINT32 dwBuffNum = oJsonSkillBuff[(*it)][1u].asInt();

            pstUser->m_stPlayerBuffList[udwBuffId].m_udwBuffId = udwBuffId;
            pstUser->m_stPlayerBuffList[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_DRAGON_SKILL].m_udwId = udwBuffId;
            pstUser->m_stPlayerBuffList[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_DRAGON_SKILL].m_ddwNum += dwBuffNum;
            pstUser->m_stPlayerBuffList[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_DRAGON_SKILL].m_dwTime = 0;
            pstUser->m_stPlayerBuffList[udwBuffId].m_ddwBuffTotal += dwBuffNum;
        }
    }
    return 0;
}

TINT32 CBufferBase::ComputeDragonMonsterSkillBuff(SCityInfo *pstCity, SUserInfo *pstUser)
{   
    const Json::Value& oJsonSkill = CGameInfo::GetInstance()->m_oJsonRoot["game_dragon_skill_monster"];
    SSkill* pstSkill = &pstUser->m_tbUserStat.m_bDragon_monster_skill[0];
    for (TUINT32 udwIdx = 0; udwIdx < EN_SKILL_TYPE__END; ++udwIdx)
    {
        if (pstSkill->m_addwLevel[udwIdx] == 0)
        {
            continue;
        }
        string strSkillId = CCommonFunc::NumToString(udwIdx);
        const Json::Value& oJsonSkillBuff = oJsonSkill[strSkillId]["b"]["b0"][(TUINT32)pstSkill->m_addwLevel[udwIdx] - 1];
        Json::Value::Members oJsonSkillBuffIds = oJsonSkillBuff.getMemberNames();
        for (Json::Value::Members::iterator it = oJsonSkillBuffIds.begin(); it != oJsonSkillBuffIds.end(); ++it)
        {
            TUINT32 udwBuffId = oJsonSkillBuff[(*it)][0u].asUInt();
            TINT32 dwBuffNum = oJsonSkillBuff[(*it)][1u].asInt();

            pstUser->m_stPlayerBuffList[udwBuffId].m_udwBuffId = udwBuffId;
            pstUser->m_stPlayerBuffList[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_DRAGON_MONSTER_SKILL].m_udwId = udwBuffId;
            pstUser->m_stPlayerBuffList[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_DRAGON_MONSTER_SKILL].m_ddwNum += dwBuffNum;
            pstUser->m_stPlayerBuffList[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_DRAGON_MONSTER_SKILL].m_dwTime = 0;
            pstUser->m_stPlayerBuffList[udwBuffId].m_ddwBuffTotal += dwBuffNum;
        }
    }
    return 0;
}

TINT32 CBufferBase::ComputeTroopBuff(SCityInfo *pstCity, SUserInfo *pstUser)
{
    //user might

    TINT64 addwTroop[EN_TROOP_TYPE__END];
    TINT64 addwFort[EN_FORT_TYPE__END];

    memset((TCHAR*)addwTroop, 0, sizeof(addwTroop));
    memset((TCHAR*)addwFort, 0, sizeof(addwFort));

    // city
    CCityBase::CalcCityTroopAndFort(&pstUser->m_stCityInfo, addwTroop, addwFort);
    // action-troop
    CCommonBase::CalcMarchActionTroop(pstUser, addwTroop);

    //might troop 
    pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_ADD_FORCE].m_astBuffDetail[EN_BUFF_TYPE_TROOP].m_udwId = EN_BUFFER_INFO_ADD_FORCE;
    pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_ADD_FORCE].m_astBuffDetail[EN_BUFF_TYPE_TROOP].m_ddwNum = CToolBase::GetTroopSumForce(addwTroop);

    pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_ADD_FORCE].m_udwBuffId = EN_BUFFER_INFO_ADD_FORCE;
    pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_ADD_FORCE].m_ddwBuffTotal +=
        pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_ADD_FORCE].m_astBuffDetail[EN_BUFF_TYPE_TROOP].m_ddwNum;

    return 0;
}


TINT32 CBufferBase::ComputeFortBuff(SCityInfo *pstCity, SUserInfo *pstUser)
{
    //user might

    TINT64 addwTroop[EN_TROOP_TYPE__END];
    TINT64 addwFort[EN_FORT_TYPE__END];

    memset((TCHAR*)addwTroop, 0, sizeof(addwTroop));
    memset((TCHAR*)addwFort, 0, sizeof(addwFort));

    // city
    CCityBase::CalcCityTroopAndFort(&pstUser->m_stCityInfo, addwTroop, addwFort);

    //might troop 
    pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_ADD_FORCE].m_astBuffDetail[EN_BUFF_TYPE_FORT].m_udwId = EN_BUFFER_INFO_ADD_FORCE;
    pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_ADD_FORCE].m_astBuffDetail[EN_BUFF_TYPE_FORT].m_ddwNum = CToolBase::GetFortSumMight(addwFort);

    pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_ADD_FORCE].m_udwBuffId = EN_BUFFER_INFO_ADD_FORCE;
    pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_ADD_FORCE].m_ddwBuffTotal +=
        pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_ADD_FORCE].m_astBuffDetail[EN_BUFF_TYPE_FORT].m_ddwNum;

    return 0;
}

TINT32 CBufferBase::ComputeEquipBuff(SCityInfo *pstCity, SUserInfo *pstUser)
{
    TINT64 udwBuffEffect = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_CORE_EQUIPMENT_DURATION].m_ddwBuffTotal;
    SEquipMentInfo stEquip;
    TUINT64 uddwEquipId = 0;

    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwEquipNum; udwIdx++)
    {
        if (pstUser->m_atbEquip[udwIdx].m_nStatus != EN_EQUIPMENT_STATUS_ON_DRAGON
            || pstUser->m_atbEquip[udwIdx].m_nPut_on_pos == 0
            || pstUser->m_aucEquipFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }

        uddwEquipId = pstUser->m_atbEquip[udwIdx].m_nId;
        stEquip.Reset();

        CBackpack::GetEquipInfoById(pstUser->m_atbEquip, pstUser->m_udwEquipNum, uddwEquipId, &stEquip);
        for (TUINT32 udwIdx = 0; udwIdx < stEquip.stStatusInfo.udwBufferNum; ++udwIdx)
        {
            TUINT32 udwBuffId = stEquip.stStatusInfo.astBuffInfo[udwIdx].m_udwId;
            TINT32 dwBufferNum = stEquip.stStatusInfo.astBuffInfo[udwIdx].m_dwNum;
            TUINT32 udwBufferTime = 0;

            TFLOAT64 ffRate = 0.0001;
            TUINT32 udwBuffEffectTime = stEquip.stBaseInfo.udwEffectTime * (1.0 + ffRate*udwBuffEffect);

            if (stEquip.stBaseInfo.udwCategory == EN_EQUIP_CATEGORY_SOUL&&stEquip.stStatusInfo.udwEquipmentPutOnTime != 0)
            {
                udwBufferTime = udwBuffEffectTime + stEquip.stStatusInfo.udwEquipmentPutOnTime;
            }

            pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[udwBuffId].m_udwBuffId = udwBuffId;
            pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_EQUIP].m_udwId = udwBuffId;
            if (stEquip.stBaseInfo.udwEffectTime != 0 && udwBufferTime < CTimeUtils::GetUnixTime())
            {
                continue;
            }
            pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_EQUIP].m_ddwNum += dwBufferNum;
            pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_EQUIP].m_dwTime = udwBufferTime;

            pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[udwBuffId].m_ddwBuffTotal += dwBufferNum;
        }
        for (TUINT32 udwIdx = 0; udwIdx < stEquip.stStatusInfo.udwMistoryBufferNum; ++udwIdx)
        {
            TUINT32 udwBuffId = stEquip.stStatusInfo.astMisteryBuffInfo[udwIdx].m_udwId;
            TINT32 dwBufferNum = stEquip.stStatusInfo.astMisteryBuffInfo[udwIdx].m_dwNum;
            TUINT32 udwBufferTime = 0;

            TFLOAT64 ffRate = 0.0001;
            TUINT32 udwBuffEffectTime = stEquip.stBaseInfo.udwEffectTime * (1.0 + ffRate*udwBuffEffect);

            if (stEquip.stBaseInfo.udwCategory == EN_EQUIP_CATEGORY_SOUL && stEquip.stStatusInfo.udwEquipmentPutOnTime != 0)
            {
                udwBufferTime = udwBuffEffectTime + stEquip.stStatusInfo.udwEquipmentPutOnTime;
            }

            pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[udwBuffId].m_udwBuffId = udwBuffId;
            pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_EQUIP].m_udwId = udwBuffId;
            if (stEquip.stBaseInfo.udwEffectTime != 0 && udwBufferTime < CTimeUtils::GetUnixTime())
            {
                continue;
            }
            pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_EQUIP].m_ddwNum += dwBufferNum;
            pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_EQUIP].m_dwTime = udwBufferTime;

            pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[udwBuffId].m_ddwBuffTotal += dwBufferNum;
        }
    }
    return 0;
}

TINT32 CBufferBase::GetDragonAttackBuff(const SPlayerBuffInfo* pstBuff, TbMarch_action *ptbAction)
{
    CGameInfo *poGameInfo = CGameInfo::GetInstance();
    SBuffInfo astTotalUserInfo[EN_BUFFER_INFO_END];

    for(TUINT32 udwIdx = 0; udwIdx < EN_BUFF_TYPE_END; ++udwIdx)
    {
        for (TUINT32 udwIdy = 0; udwIdy < EN_BUFFER_INFO_END; ++udwIdy)
        {
            if (pstBuff->m_astPlayerBuffInfo[udwIdy].m_astBuffDetail[udwIdx].m_ddwNum == 0)
            {
                continue;
            }

            if (poGameInfo->m_oJsonRoot["game_buff_func_info"][CCommonFunc::NumToString(pstBuff->m_astPlayerBuffInfo[udwIdy].m_astBuffDetail[udwIdx].m_udwId)]["a4"].asInt() == EN_BUFF_PROPERTY__DRAGON)
            {
                astTotalUserInfo[pstBuff->m_astPlayerBuffInfo[udwIdy].m_astBuffDetail[udwIdx].m_udwId].ddwBuffNum += pstBuff->m_astPlayerBuffInfo[udwIdy].m_astBuffDetail[udwIdx].m_ddwNum;
                astTotalUserInfo[pstBuff->m_astPlayerBuffInfo[udwIdy].m_astBuffDetail[udwIdx].m_udwId].ddwBuffId = pstBuff->m_astPlayerBuffInfo[udwIdy].m_astBuffDetail[udwIdx].m_udwId;

                if (pstBuff->m_astPlayerBuffInfo[udwIdy].m_astBuffDetail[udwIdx].m_dwTime > 0)
                {
                    ptbAction->m_bExpiring_buff[ptbAction->m_bExpiring_buff.m_udwNum].ddwBuffId = pstBuff->m_astPlayerBuffInfo[udwIdy].m_astBuffDetail[udwIdx].m_udwId;
                    ptbAction->m_bExpiring_buff[ptbAction->m_bExpiring_buff.m_udwNum].ddwBuffNum = pstBuff->m_astPlayerBuffInfo[udwIdy].m_astBuffDetail[udwIdx].m_ddwNum;
                    ptbAction->m_bExpiring_buff[ptbAction->m_bExpiring_buff.m_udwNum].ddwBuffExpiredTime = pstBuff->m_astPlayerBuffInfo[udwIdy].m_astBuffDetail[udwIdx].m_dwTime;
                    ptbAction->m_bExpiring_buff.m_udwNum++;
                }
            }
        }
    }

    for(TUINT32 udwIdx = 0; udwIdx < EN_BUFFER_INFO_END; ++udwIdx)
    {
        if(astTotalUserInfo[udwIdx].ddwBuffNum != 0)
        {
            ptbAction->m_bBuff[ptbAction->m_bBuff.m_udwNum++] = astTotalUserInfo[udwIdx];
            //TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("GetActionMarchBuffer: id:%d num:%d time:%d",
            //    astTotalUserInfo[udwIdx].ddwBuffId, astTotalUserInfo[udwIdx].ddwBuffNum, astTotalUserInfo[udwIdx].ddwBuffExpiredTime));
        }
    }

    ptbAction->SetFlag(TbMARCH_ACTION_FIELD_BUFF);
    ptbAction->SetFlag(TbMARCH_ACTION_FIELD_EXPIRING_BUFF);

    return 0;
}

TINT32 CBufferBase::GetDragonOccupyBuff(const SPlayerBuffInfo* pstBuff, TbMarch_action *ptbAction)
{
    return GetDragonAttackBuff(pstBuff, ptbAction);
}

TINT32 CBufferBase::ComputeAltarBuff(SCityInfo *pstCity, SUserInfo* pstUser)
{
    if(pstCity->m_stTblData.m_nAltar_buff_etime < CTimeUtils::GetUnixTime())
    {
        return 0;
    }

    for(TUINT32 udwIdx = 0; udwIdx < pstCity->m_stTblData.m_bAltar_buff.m_udwNum; ++udwIdx)
    {
        TUINT32 udwBuffId = pstCity->m_stTblData.m_bAltar_buff[udwIdx].ddwBuffId;
        TINT32 dwBuffNum = pstCity->m_stTblData.m_bAltar_buff[udwIdx].ddwBuffNum;

        pstUser->m_stPlayerBuffList[udwBuffId].m_udwBuffId = udwBuffId;
        pstUser->m_stPlayerBuffList[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_ALTAR].m_udwId = udwBuffId;
        pstUser->m_stPlayerBuffList[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_ALTAR].m_ddwNum += dwBuffNum;
        pstUser->m_stPlayerBuffList[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_ALTAR].m_dwTime = 0;
        pstUser->m_stPlayerBuffList[udwBuffId].m_ddwBuffTotal += dwBuffNum;
    }

    return 0;
}

TINT32 CBufferBase::ComputeDragonLvBuff(SCityInfo *pstCity, SUserInfo* pstUser)
{
    const Json::Value &rBasicBuffJson = CGameInfo::GetInstance()->m_oJsonRoot["game_dragon_exp"];
    TUINT32 udwDragonLv = pstUser->m_tbPlayer.m_nDragon_level;

    //force
    TINT64 ddwForce = rBasicBuffJson[udwDragonLv][3U].asInt64();
    pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_ADD_FORCE].m_udwBuffId = EN_BUFFER_INFO_ADD_FORCE;
    pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_ADD_FORCE].m_astBuffDetail[EN_BUFF_TYPE_DRAGON_LV].m_udwId = EN_BUFFER_INFO_ADD_FORCE;
    pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_ADD_FORCE].m_astBuffDetail[EN_BUFF_TYPE_DRAGON_LV].m_ddwNum += ddwForce;
    pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_ADD_FORCE].m_astBuffDetail[EN_BUFF_TYPE_DRAGON_LV].m_dwTime = 0;
    pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_ADD_FORCE].m_ddwBuffTotal += ddwForce;

    //dragon skill
    TINT64 ddwSkillNum = rBasicBuffJson[udwDragonLv][1U].asInt64();
    pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_INCREACE_DRAGON_SKILL_POINT].m_udwBuffId = EN_BUFFER_INFO_INCREACE_DRAGON_SKILL_POINT;
    pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_INCREACE_DRAGON_SKILL_POINT].m_astBuffDetail[EN_BUFF_TYPE_DRAGON_LV].m_udwId = EN_BUFFER_INFO_INCREACE_DRAGON_SKILL_POINT;
    pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_INCREACE_DRAGON_SKILL_POINT].m_astBuffDetail[EN_BUFF_TYPE_DRAGON_LV].m_ddwNum += ddwSkillNum;
    pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_INCREACE_DRAGON_SKILL_POINT].m_astBuffDetail[EN_BUFF_TYPE_DRAGON_LV].m_dwTime = 0;
    pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_INCREACE_DRAGON_SKILL_POINT].m_ddwBuffTotal += ddwSkillNum;

    //monster skill
    ddwSkillNum = rBasicBuffJson[udwDragonLv][2U].asInt64();
    pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_INCREACE_MONSTER_SKILL_POINT].m_udwBuffId = EN_BUFFER_INFO_INCREACE_MONSTER_SKILL_POINT;
    pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_INCREACE_MONSTER_SKILL_POINT].m_astBuffDetail[EN_BUFF_TYPE_DRAGON_LV].m_udwId = EN_BUFFER_INFO_INCREACE_MONSTER_SKILL_POINT;
    pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_INCREACE_MONSTER_SKILL_POINT].m_astBuffDetail[EN_BUFF_TYPE_DRAGON_LV].m_ddwNum += ddwSkillNum;
    pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_INCREACE_MONSTER_SKILL_POINT].m_astBuffDetail[EN_BUFF_TYPE_DRAGON_LV].m_dwTime = 0;
    pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_INCREACE_MONSTER_SKILL_POINT].m_ddwBuffTotal += ddwSkillNum;
    return 0;
}

TINT32 CBufferBase::ComputeLordLvBuff(SCityInfo *pstCity, SUserInfo* pstUser)
{
    const Json::Value &rBasicBuffJson = CGameInfo::GetInstance()->m_oJsonRoot["game_lord_exp"];
    TUINT32 udwLordLv = pstUser->m_tbPlayer.m_nLevel;

    //force
    TINT64 ddwForce = rBasicBuffJson[udwLordLv][2U].asInt64();
    pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_ADD_FORCE].m_udwBuffId = EN_BUFFER_INFO_ADD_FORCE;
    pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_ADD_FORCE].m_astBuffDetail[EN_BUFF_TYPE_LORD_LV].m_udwId = EN_BUFFER_INFO_ADD_FORCE;
    pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_ADD_FORCE].m_astBuffDetail[EN_BUFF_TYPE_LORD_LV].m_ddwNum += ddwForce;
    pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_ADD_FORCE].m_astBuffDetail[EN_BUFF_TYPE_LORD_LV].m_dwTime = 0;
    pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_ADD_FORCE].m_ddwBuffTotal += ddwForce;

    //skill
    TINT64 ddwSkillNum = rBasicBuffJson[udwLordLv][1U].asInt64();
    pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_INCREACE_LORD_SKILL_POINT].m_udwBuffId = EN_BUFFER_INFO_INCREACE_LORD_SKILL_POINT;
    pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_INCREACE_LORD_SKILL_POINT].m_astBuffDetail[EN_BUFF_TYPE_LORD_LV].m_udwId = EN_BUFFER_INFO_INCREACE_LORD_SKILL_POINT;
    pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_INCREACE_LORD_SKILL_POINT].m_astBuffDetail[EN_BUFF_TYPE_LORD_LV].m_ddwNum += ddwSkillNum;
    pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_INCREACE_LORD_SKILL_POINT].m_astBuffDetail[EN_BUFF_TYPE_LORD_LV].m_dwTime = 0;
    pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_INCREACE_LORD_SKILL_POINT].m_ddwBuffTotal += ddwSkillNum;

    return 0;
}

TINT32 CBufferBase::MarchBuffToPlayerBuff(TbMarch_action* ptbMarch, SPlayerBuffInfo* pstBuff)
{
    pstBuff->Reset();
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();

    for(TUINT32 udwIdx = 0; udwIdx < ptbMarch->m_bBuff.m_udwNum; ++udwIdx)
    {
        TUINT32 udwBuffId = ptbMarch->m_bBuff[udwIdx].ddwBuffId;
        TUINT32 udwBuffNum = ptbMarch->m_bBuff[udwIdx].ddwBuffNum;
        pstBuff->m_astPlayerBuffInfo[udwBuffId].m_udwBuffId = udwBuffId;
        pstBuff->m_astPlayerBuffInfo[udwBuffId].m_ddwBuffTotal += udwBuffNum;
    }

    for(TUINT32 udwIdx = 0; udwIdx < ptbMarch->m_bExpiring_buff.m_udwNum; ++udwIdx)
    {
        TUINT32 udwBuffId = ptbMarch->m_bExpiring_buff[udwIdx].ddwBuffId;
        TUINT32 udwBuffNum = ptbMarch->m_bExpiring_buff[udwIdx].ddwBuffNum;
        TUINT32 udwExpiredTime = ptbMarch->m_bExpiring_buff[udwIdx].ddwBuffExpiredTime;
        if(udwExpiredTime > 0 && udwExpiredTime < udwCurTime)
        {
            pstBuff->m_astPlayerBuffInfo[udwBuffId].m_udwBuffId = udwBuffId;
            pstBuff->m_astPlayerBuffInfo[udwBuffId].m_ddwBuffTotal -= udwBuffNum;
        }
    }

    return 0;
}

TINT32 CBufferBase::GenMarchBuff(const SPlayerBuffInfo* pstBuff, TbMarch_action* ptbMarch, TBOOL bIsRallyAttack)
{

    SPlayerBuffInfo stBattleBuff;
    
    CBufferBase::GenBattleBuff(pstBuff, &stBattleBuff, bIsRallyAttack, FALSE, ptbMarch->m_bParam[0].m_stKnight.ddwLevel);

    CBufferBase::SetMarchBuff(&stBattleBuff, ptbMarch);

    return 0;
}

TINT32 CBufferBase::SetMarchBuff(SPlayerBuffInfo* pstBattleBuff, TbMarch_action* ptbMarch)
{
    ptbMarch->m_bBuff.Reset();
    ptbMarch->m_bExpiring_buff.Reset();

    for(TUINT32 udwBufferId = 0; udwBufferId < EN_BUFFER_INFO_END; ++udwBufferId)
    {
        if(pstBattleBuff->m_astPlayerBuffInfo[udwBufferId].m_ddwBuffTotal == 0)
        {
            continue;
        }

        ptbMarch->m_bBuff[ptbMarch->m_bBuff.m_udwNum].ddwBuffId = udwBufferId;
        ptbMarch->m_bBuff[ptbMarch->m_bBuff.m_udwNum].ddwBuffNum = pstBattleBuff->m_astPlayerBuffInfo[udwBufferId].m_ddwBuffTotal;
        ptbMarch->m_bBuff[ptbMarch->m_bBuff.m_udwNum].ddwBuffExpiredTime = 0;
        ptbMarch->m_bBuff.m_udwNum++;
        ptbMarch->SetFlag(TbMARCH_ACTION_FIELD_BUFF);

        for(TUINT32 udwBuffType = 0; udwBuffType < EN_BUFF_TYPE_END; ++udwBuffType)
        {
            if(pstBattleBuff->m_astPlayerBuffInfo[udwBufferId].m_astBuffDetail[udwBuffType].m_dwTime > 0)
            {
                ptbMarch->m_bExpiring_buff[ptbMarch->m_bExpiring_buff.m_udwNum].ddwBuffId = udwBufferId;
                ptbMarch->m_bExpiring_buff[ptbMarch->m_bExpiring_buff.m_udwNum].ddwBuffNum =
                    pstBattleBuff->m_astPlayerBuffInfo[udwBufferId].m_astBuffDetail[udwBuffType].m_ddwNum;
                ptbMarch->m_bExpiring_buff[ptbMarch->m_bExpiring_buff.m_udwNum].ddwBuffExpiredTime =
                    pstBattleBuff->m_astPlayerBuffInfo[udwBufferId].m_astBuffDetail[udwBuffType].m_dwTime;
                ptbMarch->m_bExpiring_buff.m_udwNum++;
                ptbMarch->SetFlag(TbMARCH_ACTION_FIELD_EXPIRING_BUFF);
            }

            if(ptbMarch->m_bExpiring_buff.m_udwNum >= TBMARCH_ACTION_EXPIRING_BUFF_MAX_NUM)
            {
                break;
            }
        }

        if(ptbMarch->m_bBuff.m_udwNum >= TBMARCH_ACTION_BUFF_MAX_NUM)
        {
            break;
        }
    }

    return 0;
}

TINT32 CBufferBase::SetOccupyBuff(SPlayerBuffInfo* pstOccupyBuff, TbMarch_action* ptbMarch)
{
    return SetMarchBuff(pstOccupyBuff, ptbMarch);
}

TINT32 CBufferBase::GenBattleBuff(const SPlayerBuffInfo* pstBuff, SPlayerBuffInfo* pstBattleBuff, TBOOL bIsRallyAttack, TBOOL bIsInCity, TINT32 dwKnightLv)
{
    pstBattleBuff->Reset();

    SPlayerBuffInfo stBuff = *pstBuff;

    CGameInfo *poGameInfo = CGameInfo::GetInstance();
    const Json::Value& jTroop = poGameInfo->m_oJsonRoot["game_troop"];

    if (!bIsInCity)
    {
        for (TINT32 dwIdx = 0; dwIdx < EN_BUFFER_INFO_END; dwIdx++)
        {
            if (stBuff.m_astPlayerBuffInfo[dwIdx].m_astBuffDetail[EN_BUFF_TYPE_KNIGHT].m_ddwNum > 0)
            {
                stBuff.m_astPlayerBuffInfo[dwIdx].m_ddwBuffTotal -= stBuff.m_astPlayerBuffInfo[dwIdx].m_astBuffDetail[EN_BUFF_TYPE_KNIGHT].m_ddwNum;
                stBuff.m_astPlayerBuffInfo[dwIdx].m_astBuffDetail[EN_BUFF_TYPE_KNIGHT].Reset();
                if (stBuff.m_astPlayerBuffInfo[dwIdx].m_ddwBuffTotal <= 0)
                {
                    stBuff.m_astPlayerBuffInfo[dwIdx].Reset();
                }
            }
        }

        const Json::Value& jKnight = poGameInfo->m_oJsonRoot["game_knight"];

        if (dwKnightLv > 0)
        {
            for (TUINT32 udwIdx = 0; udwIdx < jKnight["b"].size(); udwIdx++)
            {
                TINT32 dwBuffId = jKnight["b"][udwIdx][0U].asInt();
                TINT32 dwBuffNum = jKnight["b"][udwIdx][1U].asInt() * dwKnightLv;
                stBuff.m_astPlayerBuffInfo[dwBuffId].m_udwBuffId = dwBuffId;
                stBuff.m_astPlayerBuffInfo[dwBuffId].m_ddwBuffTotal += dwBuffNum;

                stBuff.m_astPlayerBuffInfo[dwBuffId].m_astBuffDetail[EN_BUFF_TYPE_KNIGHT].m_udwId = dwBuffId;
                stBuff.m_astPlayerBuffInfo[dwBuffId].m_astBuffDetail[EN_BUFF_TYPE_KNIGHT].m_ddwNum += dwBuffNum;
            }
        }
    }

    for(TUINT32 udwTroopType = 0; udwTroopType < jTroop.size(); ++udwTroopType)
    {
        for(TUINT32 udwAttackBuffIdx = 0; udwAttackBuffIdx < jTroop[udwTroopType]["ab"]["attack"].size(); ++udwAttackBuffIdx)
        {
            TUINT32 udwBufferId = jTroop[udwTroopType]["ab"]["attack"][udwAttackBuffIdx].asUInt();
            pstBattleBuff->m_astPlayerBuffInfo[udwBufferId] = stBuff.m_astPlayerBuffInfo[udwBufferId];
        }
        for(TUINT32 udwDefenseBuffIdx = 0; udwDefenseBuffIdx < jTroop[udwTroopType]["ab"]["defense"].size(); ++udwDefenseBuffIdx)
        {
            TUINT32 udwBufferId = jTroop[udwTroopType]["ab"]["defense"][udwDefenseBuffIdx].asUInt();
            pstBattleBuff->m_astPlayerBuffInfo[udwBufferId] = stBuff.m_astPlayerBuffInfo[udwBufferId];
        }
        for(TUINT32 udwHpBuffIdx = 0; udwHpBuffIdx < jTroop[udwTroopType]["ab"]["life"].size(); ++udwHpBuffIdx)
        {
            TUINT32 udwBufferId = jTroop[udwTroopType]["ab"]["life"][udwHpBuffIdx].asUInt();
            pstBattleBuff->m_astPlayerBuffInfo[udwBufferId] = stBuff.m_astPlayerBuffInfo[udwBufferId];
        }
    }

    //rally war buff
    if (bIsRallyAttack)
    {
        pstBattleBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_RALLY_ATTACK] = stBuff.m_astPlayerBuffInfo[EN_BUFFER_INFO_RALLY_ATTACK];
        pstBattleBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_RALLY_DEFENSE] = stBuff.m_astPlayerBuffInfo[EN_BUFFER_INFO_RALLY_DEFENSE];
        pstBattleBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_RALLY_LIFE] = stBuff.m_astPlayerBuffInfo[EN_BUFFER_INFO_RALLY_LIFE];
    }

    const Json::Value& jFort = poGameInfo->m_oJsonRoot["game_fort"];
    for(TUINT32 udwFortType = 0; udwFortType < jFort.size(); ++udwFortType)
    {
        for(TUINT32 udwAttackBuffIdx = 0; udwAttackBuffIdx < jFort[udwFortType]["ab"]["attack"].size(); ++udwAttackBuffIdx)
        {
            TUINT32 udwBufferId = jFort[udwFortType]["ab"]["attack"][udwAttackBuffIdx].asUInt();
            pstBattleBuff->m_astPlayerBuffInfo[udwBufferId] = stBuff.m_astPlayerBuffInfo[udwBufferId];
        }
        for(TUINT32 udwDefenseBuffIdx = 0; udwDefenseBuffIdx < jFort[udwFortType]["ab"]["defense"].size(); ++udwDefenseBuffIdx)
        {
            TUINT32 udwBufferId = jFort[udwFortType]["ab"]["defense"][udwDefenseBuffIdx].asUInt();
            pstBattleBuff->m_astPlayerBuffInfo[udwBufferId] = stBuff.m_astPlayerBuffInfo[udwBufferId];
        }
        for(TUINT32 udwHpBuffIdx = 0; udwHpBuffIdx < jFort[udwFortType]["ab"]["life"].size(); ++udwHpBuffIdx)
        {
            TUINT32 udwBufferId = jFort[udwFortType]["ab"]["life"][udwHpBuffIdx].asUInt();
            pstBattleBuff->m_astPlayerBuffInfo[udwBufferId] = stBuff.m_astPlayerBuffInfo[udwBufferId];
        }
    }

    //load
    pstBattleBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_TROOP_LOAD] = stBuff.m_astPlayerBuffInfo[EN_BUFFER_INFO_TROOP_LOAD];

    //collect speed
    pstBattleBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_FOOD_COLLECT_SPEED] = stBuff.m_astPlayerBuffInfo[EN_BUFFER_INFO_FOOD_COLLECT_SPEED];
    pstBattleBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_WOOD_COLLECT_SPEED] = stBuff.m_astPlayerBuffInfo[EN_BUFFER_INFO_WOOD_COLLECT_SPEED];
    pstBattleBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_STONE_COLLECT_SPEED] = stBuff.m_astPlayerBuffInfo[EN_BUFFER_INFO_STONE_COLLECT_SPEED];
    pstBattleBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_ORE_COLLECT_SPEED] = stBuff.m_astPlayerBuffInfo[EN_BUFFER_INFO_ORE_COLLECT_SPEED];
    pstBattleBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_GOLD_COLLECT_SPEED] = stBuff.m_astPlayerBuffInfo[EN_BUFFER_INFO_GOLD_COLLECT_SPEED];
    pstBattleBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_COLLECT_SPEED] = stBuff.m_astPlayerBuffInfo[EN_BUFFER_INFO_COLLECT_SPEED];

    //equipment geting rate
    pstBattleBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_CORE_GATHERING_RATE] = stBuff.m_astPlayerBuffInfo[EN_BUFFER_INFO_CORE_GATHERING_RATE];
    pstBattleBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_PARTS_GATHERING_RATE] = stBuff.m_astPlayerBuffInfo[EN_BUFFER_INFO_PARTS_GATHERING_RATE];
    pstBattleBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_MATERIAL_GATHERING_RATE] = stBuff.m_astPlayerBuffInfo[EN_BUFFER_INFO_MATERIAL_GATHERING_RATE];
    pstBattleBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_CRYSTAL_GATHERING_RATE] = stBuff.m_astPlayerBuffInfo[EN_BUFFER_INFO_CRYSTAL_GATHERING_RATE];

    return 0;
}

TINT32 CBufferBase::GenOccupyBuff(const SPlayerBuffInfo* pstBuff, SPlayerBuffInfo* pstOccupyBuff)
{
    pstOccupyBuff->Reset();

    //load
    pstOccupyBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_TROOP_LOAD] = pstBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_TROOP_LOAD];

    //collect speed
    pstOccupyBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_FOOD_COLLECT_SPEED] = pstBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_FOOD_COLLECT_SPEED];
    pstOccupyBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_WOOD_COLLECT_SPEED] = pstBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_WOOD_COLLECT_SPEED];
    pstOccupyBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_STONE_COLLECT_SPEED] = pstBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_STONE_COLLECT_SPEED];
    pstOccupyBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_ORE_COLLECT_SPEED] = pstBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_ORE_COLLECT_SPEED];
    pstOccupyBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_GOLD_COLLECT_SPEED] = pstBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_GOLD_COLLECT_SPEED];
    pstOccupyBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_COLLECT_SPEED] = pstBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_COLLECT_SPEED];

    //equipment geting rate
    pstOccupyBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_CORE_GATHERING_RATE] = pstBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_CORE_GATHERING_RATE];
    pstOccupyBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_PARTS_GATHERING_RATE] = pstBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_PARTS_GATHERING_RATE];
    pstOccupyBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_MATERIAL_GATHERING_RATE] = pstBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_MATERIAL_GATHERING_RATE];
    pstOccupyBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_CRYSTAL_GATHERING_RATE] = pstBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_CRYSTAL_GATHERING_RATE];

    return 0;
}

TINT32 CBufferBase::RemoveDragonBuff(SPlayerBuffInfo *pstBuff)
{
    const Json::Value &jBuffFunInfo = CGameInfo::GetInstance()->m_oJsonRoot["game_buff_func_info"];
    for(TUINT32 udwBuffId = 0; udwBuffId < EN_BUFFER_INFO_END; ++udwBuffId)
    {
        pstBuff->m_astPlayerBuffInfo[udwBuffId].m_ddwBuffTotal -= pstBuff->m_astPlayerBuffInfo[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_EQUIP].m_ddwNum;
        pstBuff->m_astPlayerBuffInfo[udwBuffId].m_ddwBuffTotal -= pstBuff->m_astPlayerBuffInfo[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_DRAGON_SKILL].m_ddwNum;
        pstBuff->m_astPlayerBuffInfo[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_EQUIP].m_ddwNum = 0;
        pstBuff->m_astPlayerBuffInfo[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_DRAGON_SKILL].m_ddwNum = 0;

        TBOOL bIsMove = jBuffFunInfo[CCommonFunc::NumToString(udwBuffId)]["a11"].asBool();
        if(bIsMove)
        {
            pstBuff->m_astPlayerBuffInfo[udwBuffId].Reset();
        }
    }
    return 0;
}

TBOOL CBufferBase::IsBuffSame(const SPlayerBuffInfo& stBuffA, const SPlayerBuffInfo& stBuffB)
{
    for(TUINT32 udwBufferId = 0; udwBufferId < EN_BUFFER_INFO_END; ++udwBufferId)
    {
        if(stBuffA.m_astPlayerBuffInfo[udwBufferId].m_ddwBuffTotal != stBuffB.m_astPlayerBuffInfo[udwBufferId].m_ddwBuffTotal)
        {
            return FALSE;
        }
    }
    return TRUE;
}

TINT32 CBufferBase::ComputeBuildingForce(SCityInfo *pstCity, SUserInfo *pstUser)
{
    TUINT32 udwType = 0;
    TUINT32 udwLv = 0;
    TINT32 dwTotalForce = 0;
    const Json::Value& oBuildNode = CGameInfo::GetInstance()->m_oJsonRoot["game_building"];
    for(TUINT32 udwIdx = 0; udwIdx < pstCity->m_stTblData.m_bBuilding.m_udwNum; ++udwIdx)
    {
        SCityBuildingNode* pstBuildingNode = &pstCity->m_stTblData.m_bBuilding[udwIdx];
        udwType = pstBuildingNode->m_ddwType;
        udwLv = pstBuildingNode->m_ddwLevel;
        if(udwLv == 0)
        {
            continue;
        }

        TCHAR szBuildType[32];
        snprintf(szBuildType, 32, "%u", udwType);

        const Json::Value& oBuilBuff = oBuildNode[szBuildType]["b"]["b0"][udwLv - 1];

        Json::Value::Members member = oBuilBuff.getMemberNames();
        for(Json::Value::Members::iterator iter = member.begin(); iter != member.end(); ++iter)
        {
            const std::string &key = *iter;
            TUINT32 udwBufferId = oBuilBuff[key][0U].asUInt();
            TINT32 dwBufferNum = oBuilBuff[key][1U].asInt();
            if(udwBufferId == EN_BUFFER_INFO_ADD_FORCE)
            {
                dwTotalForce += dwBufferNum;
            }
        }
    }
    return dwTotalForce;
}


TINT32 CBufferBase::ComputeResearchForce(SCityInfo *pstCity, SUserInfo *pstUser)
{
    TINT32 dwTotalForce = 0;
    SCommonResearch* pstResearch = &pstUser->m_stCityInfo.m_stTblData.m_bResearch[0];
    const Json::Value& oJsonResearh = CGameInfo::GetInstance()->m_oJsonRoot["game_research"];
    for(TUINT32 udwIdx = 0; udwIdx < EN_RESEARCH_TYPE__END; ++udwIdx)
    {
        if(pstResearch->m_addwLevel[udwIdx] == 0)
        {
            continue;
        }
        string strResearchId = CCommonFunc::NumToString(udwIdx);
        TINT32 dwLv = pstResearch->m_addwLevel[udwIdx] - 1;
        const Json::Value& oJsonResearchBuff = oJsonResearh[strResearchId]["b"]["b0"][dwLv];
        Json::Value::Members oBuffIds = oJsonResearchBuff.getMemberNames();
        for(Json::Value::Members::iterator it = oBuffIds.begin(); it != oBuffIds.end(); ++it)
        {
            TUINT32 udwBuffId = oJsonResearchBuff[(*it)][0u].asUInt();
            TINT32 dwBuffNum = oJsonResearchBuff[(*it)][1u].asInt();
            if(udwBuffId == EN_BUFFER_INFO_ADD_FORCE)
            {
                dwTotalForce += dwBuffNum;
            }
        }
    }
    return dwTotalForce;
}

TINT64 CBufferBase::ComputeDragonLvForce(SCityInfo *pstCity, SUserInfo *pstUser)
{
    const Json::Value &rBasicBuffJson = CGameInfo::GetInstance()->m_oJsonRoot["game_dragon_exp"];
    TINT32 dwDragonLv = pstUser->m_tbPlayer.m_nDragon_level;
    TINT64 ddwForce = rBasicBuffJson[dwDragonLv][3U].asInt64();

    return ddwForce;
}

TINT64 CBufferBase::ComputeLordLvForce(SCityInfo *pstCity, SUserInfo *pstUser)
{
    const Json::Value &rBasicBuffJson = CGameInfo::GetInstance()->m_oJsonRoot["game_lord_exp"];
    TINT32 dwLordLv = pstUser->m_tbPlayer.m_nLevel;
    TINT64 ddwForce = rBasicBuffJson[dwLordLv][2U].asInt64();

    return ddwForce;
}

TINT32 CBufferBase::ComputeKnightBuff(SCityInfo *pstCity, SUserInfo* pstUser)
{
    TUINT32 udwLevel = 0;
    const Json::Value& oPosition = CGameInfo::GetInstance()->m_oJsonRoot["game_knight_pos"];
    TINT64 ddwBaseVal = pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_KNIGHT_HALL_BASE_BUFF].m_ddwBuffTotal; //wave@v1.2:先计算建筑buf

    for (TUINT32 udwIdx = 0; udwIdx < pstCity->m_stTblData.m_bKnight.m_udwNum; udwIdx++)
    {
        if (pstCity->m_stTblData.m_bKnight[udwIdx].ddwStatus != EN_KNIGHT_STATUS__ASSIGNING)
        {
            continue;
        }
        TUINT32 udwKnightPos = pstCity->m_stTblData.m_bKnight.m_astList[udwIdx].ddwPos;
        udwLevel = CGameInfo::GetInstance()->ComputeKnightLevelByExp(pstCity->m_stTblData.m_bKnight[udwIdx].ddwExp);
        string szTmp = CCommonFunc::NumToString(udwKnightPos);
        if (!oPosition.isMember(szTmp))
        {
            continue;
        }
        if (udwLevel == 0)
        {
            continue;
        }

        for (TUINT32 udwBufferIdx = 0; udwBufferIdx < oPosition[szTmp]["b"]["b0"][udwLevel - 1].size(); ++udwBufferIdx)
        {
            TUINT32 udwBufferId = oPosition[szTmp]["b"]["b0"][udwLevel - 1][udwBufferIdx][0U].asUInt();
            TUINT32 udwBufferNum = oPosition[szTmp]["b"]["b0"][udwLevel - 1][udwBufferIdx][1U].asUInt() * (10000 + ddwBaseVal) / 10000;

            pstUser->m_stPlayerBuffList[udwBufferId].m_udwBuffId = udwBufferId;
            pstUser->m_stPlayerBuffList[udwBufferId].m_astBuffDetail[EN_BUFF_TYPE_KNIGHT].m_udwId = udwBufferId;
            pstUser->m_stPlayerBuffList[udwBufferId].m_astBuffDetail[EN_BUFF_TYPE_KNIGHT].m_ddwNum += udwBufferNum;
            pstUser->m_stPlayerBuffList[udwBufferId].m_astBuffDetail[EN_BUFF_TYPE_KNIGHT].m_dwTime = 0;
            pstUser->m_stPlayerBuffList[udwBufferId].m_ddwBuffTotal += udwBufferNum;
        }
    }

    return 0;
}

TINT32 CBufferBase::GetBuffType(TUINT32 udwBuffId)
{
    const Json::Value& jBuff = CGameInfo::GetInstance()->m_oJsonRoot["game_buff_func_info"];

    TINT32 dwBuffType = -1;

    if (jBuff.isMember(CCommonFunc::NumToString(udwBuffId)))
    {
        dwBuffType = jBuff[CCommonFunc::NumToString(udwBuffId)]["a4"].asInt();
    }

    return dwBuffType;
}

TINT32 CBufferBase::GetIdolBattleBuff(TbIdol *ptbIdol, SPlayerBuffInfo* pstBuff)
{
    TUINT32 udwBuffId = 0;
    TUINT32 udwBuffNum = 0;

    CGameInfo *poGameInfo = CGameInfo::GetInstance();
    const Json::Value& jKnight = poGameInfo->m_oJsonRoot["game_knight"];

    TUINT32 udwKnightLv = ptbIdol->m_jInfo["t"][1U].asInt();
    if (udwKnightLv > 0)
    {
        for (TUINT32 udwIdx = 0; udwIdx < jKnight["b"].size(); udwIdx++)
        {
            udwBuffId = jKnight["b"][udwIdx][0U].asInt();
            udwBuffNum = jKnight["b"][udwIdx][1U].asInt() * udwKnightLv;

            pstBuff->m_astPlayerBuffInfo[udwBuffId].m_udwBuffId = udwBuffId;
            pstBuff->m_astPlayerBuffInfo[udwBuffId].m_ddwBuffTotal += udwBuffNum;
            pstBuff->m_astPlayerBuffInfo[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_KNIGHT].m_udwId = udwBuffId;
            pstBuff->m_astPlayerBuffInfo[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_KNIGHT].m_ddwNum += udwBuffNum;
        }
    }

    for (TUINT32 udwIdx = 0; udwIdx < ptbIdol->m_jInfo["t"][6U].size(); udwIdx++)
    {
        udwBuffId = ptbIdol->m_jInfo["t"][6U][udwIdx][0U].asUInt();
        udwBuffNum = ptbIdol->m_jInfo["t"][6U][udwIdx][1U].asUInt();

        pstBuff->m_astPlayerBuffInfo[udwBuffId].m_udwBuffId = udwBuffId;
        pstBuff->m_astPlayerBuffInfo[udwBuffId].m_ddwBuffTotal += udwBuffNum;
        pstBuff->m_astPlayerBuffInfo[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_DRAGON_SKILL].m_udwId = udwBuffId;
        pstBuff->m_astPlayerBuffInfo[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_DRAGON_SKILL].m_ddwNum += udwBuffNum;
    }


    return 0;
}

TINT32 CBufferBase::ComputeIdolBuff(TbIdol *atbIdol, TUINT32 udwIdolNum, SUserInfo *pstUser)
{
    if (pstUser->m_tbPlayer.m_nAlpos == 0)
    {
        return 0;
    }
    TUINT32 udwAlid = pstUser->m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET;

    for (TUINT32 udwIdx = 0; udwIdx < udwIdolNum; udwIdx++)
    {
        if (atbIdol[udwIdx].m_nAlid == udwAlid && atbIdol[udwIdx].m_nStatus == EN_IDOL_STATUS__BUFF_PERIOD)
        {
            Json::Value::Members members = atbIdol[udwIdx].m_jInfo["b"].getMemberNames();
            TUINT32 udwBuffId = 0;
            TINT32 dwBuffNum = 0;
            for (Json::Value::Members::iterator it = members.begin(); it != members.end(); it++)
            {
                udwBuffId = atbIdol[udwIdx].m_jInfo["b"][*it][0U].asUInt();
                dwBuffNum = atbIdol[udwIdx].m_jInfo["b"][*it][1U].asInt();

                pstUser->m_stPlayerBuffList[udwBuffId].m_udwBuffId = udwBuffId;
                pstUser->m_stPlayerBuffList[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_IDOL].m_udwId = udwBuffId;
                pstUser->m_stPlayerBuffList[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_IDOL].m_ddwNum += dwBuffNum;
                pstUser->m_stPlayerBuffList[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_IDOL].m_dwTime = 0;
                pstUser->m_stPlayerBuffList[udwBuffId].m_ddwBuffTotal += dwBuffNum;
            }
        }
    }

    return 0;
}

TINT32 CBufferBase::ComputeTitleBuff(STitleInfoList *pstTitle, TbThrone *ptbThrone, SUserInfo *pstUser)
{
    TINT32 dwTitleId = -1;

    const Json::Value &jTitle = CGameInfo::GetInstance()->m_oJsonRoot["game_title"];

    if (ptbThrone->m_nOwner_id != 0 && ptbThrone->m_nOwner_id == pstUser->m_tbPlayer.m_nUid)
    { //国王称号 不存表 每次根据王座的拥有者id去算 且国王称号id默认为1
        dwTitleId = 1;
    }
    else
    {
        ostringstream oss;
        TINT64 ddwCurTime = CTimeUtils::GetUnixTime();
        for (TUINT32 udwIdx = 0; udwIdx < pstTitle->udwNum; udwIdx++)
        {
            if (pstTitle->aucFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
            {
                continue;
            }

            oss.str("");
            oss << pstTitle->atbTitle[udwIdx].m_nId;
            TINT32 dwExpireTime = 0;
            if (jTitle.isMember(oss.str()))
            {
                dwExpireTime = jTitle[oss.str()]["time"].asInt();
            }

            if (ptbThrone->m_nOccupy_time >= pstTitle->atbTitle[udwIdx].m_nDub_time
                || ddwCurTime - pstTitle->atbTitle[udwIdx].m_nDub_time > dwExpireTime)
            {
                continue;
            }
            if (pstTitle->atbTitle[udwIdx].m_nUid == pstUser->m_tbPlayer.m_nUid)
            {
                dwTitleId = pstTitle->atbTitle[udwIdx].m_nId;
                break;
            }
        }
    }

    if (dwTitleId == -1)
    {
        return 0;
    }

    const Json::Value& jTitleBuff = jTitle[CCommonFunc::NumToString(dwTitleId)]["buff"];
    Json::Value::Members members = jTitleBuff.getMemberNames();
    TUINT32 udwBuffId = 0;
    TINT32 dwBuffNum = 0;
    for (Json::Value::Members::iterator it = members.begin(); it != members.end(); it++)
    {
        udwBuffId = jTitleBuff[*it][0U].asUInt();
        dwBuffNum = jTitleBuff[*it][1U].asInt();
        pstUser->m_stPlayerBuffList[udwBuffId].m_udwBuffId = udwBuffId;
        pstUser->m_stPlayerBuffList[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_TITLE].m_udwId = udwBuffId;
        pstUser->m_stPlayerBuffList[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_TITLE].m_ddwNum += dwBuffNum;
        pstUser->m_stPlayerBuffList[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_TITLE].m_dwTime = 0;
        pstUser->m_stPlayerBuffList[udwBuffId].m_ddwBuffTotal += dwBuffNum;
    }

    return 0;
}

TINT32 CBufferBase::ComputeThroneBuff(TbThrone *ptbThrone, SUserInfo *pstUser)
{
    if (pstUser->m_tbPlayer.m_nAlpos == 0
        || ptbThrone->m_nAlid != pstUser->m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET
        || !ptbThrone->m_jInfo.isObject() || !ptbThrone->m_jInfo.isMember("buff"))
    {
        return 0;
    }

    const Json::Value& jThroneBuff = ptbThrone->m_jInfo["buff"];
    Json::Value::Members members = jThroneBuff.getMemberNames();
    TUINT32 udwBuffId = 0;
    TINT32 dwBuffNum = 0;
    for (Json::Value::Members::iterator it = members.begin(); it != members.end(); it++)
    {
        udwBuffId = jThroneBuff[*it][0U].asUInt();
        dwBuffNum = jThroneBuff[*it][1U].asInt();
        pstUser->m_stPlayerBuffList[udwBuffId].m_udwBuffId = udwBuffId;
        pstUser->m_stPlayerBuffList[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_THRONE].m_udwId = udwBuffId;
        pstUser->m_stPlayerBuffList[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_THRONE].m_ddwNum += dwBuffNum;
        pstUser->m_stPlayerBuffList[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_THRONE].m_dwTime = 0;
        pstUser->m_stPlayerBuffList[udwBuffId].m_ddwBuffTotal += dwBuffNum;
    }

    return 0;
}