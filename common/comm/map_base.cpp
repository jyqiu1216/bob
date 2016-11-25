#include "map_base.h"
#include "wild_info.h"
#include "common_func.h"
#include "city_base.h"

// =================================================== public ===================================================== //

TINT64 CMapBase::GetMapMight(TbMap *ptbMap)
{
    TINT64 ddwMapMight = 0;
    TINT64 ddwCoefficient = 0;
    CGameInfo *poCGameInfo = CGameInfo::GetInstance();
    const SCommonTroop &rstSCommonTroop = ptbMap->m_bTroop[0];
    const Json::Value &rJsonTroop = poCGameInfo->m_oJsonRoot["game_troop"];

    for (TINT32 dwIdx = EN_TROOP_TYPE__T1_INFANTRY; dwIdx < EN_TROOP_TYPE__END; ++dwIdx)
    {
        if (rstSCommonTroop.m_addwNum[dwIdx] > 0)
        {
            ddwCoefficient = rJsonTroop[dwIdx]["a"]["a9"].asInt();
            ddwMapMight += ddwCoefficient * rstSCommonTroop.m_addwNum[dwIdx];
        }
    }

    return ddwMapMight;
}

TVOID CMapBase::ResetMap(TbMap *ptbMap)
{

    if (ptbMap->m_nType == EN_WILD_TYPE__CITY)
    {
        return;
    }

    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("ResetMap[pos=%ld]", ptbMap->m_nId));

    // update map info
    ptbMap->Set_Utime(CTimeUtils::GetUnixTime());
    ptbMap->Set_Type(ptbMap->m_nRtype);
    ptbMap->Set_Name("");
    ptbMap->Set_Status(0);
    ptbMap->Set_Might(0);
    ptbMap->Set_Force_kill(0);
    ptbMap->Set_Cid(0);
    ptbMap->Set_Uid(0);
    ptbMap->Set_Uname("");
    ptbMap->Set_Ulevel(0);
    ptbMap->Set_Alid(0);
    ptbMap->Set_Alname("");
    ptbMap->Set_Npc(0);
    ptbMap->Set_Bid(0);
    ptbMap->Set_Res_rate(0);
    ptbMap->Set_Res_time(0);
    ptbMap->Set_Reward_left(0);

    ptbMap->m_bTroop.Reset();
    ptbMap->SetFlag(TbMAP_FIELD_TROOP);

    ptbMap->m_bResource[0].Reset();
    ptbMap->SetFlag(TbMAP_FIELD_RESOURCE);

    ptbMap->Set_Burn_end_time(0);
    ptbMap->Set_Smoke_end_time(0);
    ptbMap->Set_Prison_flag(0);
    ptbMap->Set_Al_nick("");

    ptbMap->m_jCity_info.clear();
    ptbMap->SetFlag(TbMAP_FIELD_CITY_INFO);
    ptbMap->Set_Avatar(0);
    ptbMap->Set_Rally_troop_limit(0);
    ptbMap->Set_Showtime(0);
    ptbMap->Set_Name_update_time(CTimeUtils::GetUnixTime());
    ptbMap->Set_Level(1);
    ptbMap->Set_Age(0);
    ptbMap->Set_Al_pos(0);
    ptbMap->Set_Center_pos(0);
    ptbMap->Set_Name_update_time(0);
    ptbMap->Set_Time_end(0);
    ptbMap->Set_Vip_etime(0);
    ptbMap->Set_Vip_level(0);
    ptbMap->m_bAl_attack_info.Reset();
    ptbMap->m_bAttack_info.Reset();
    ptbMap->SetFlag(TbMAP_FIELD_AL_ATTACK_INFO);
    ptbMap->SetFlag(TbMAP_FIELD_ATTACK_INFO);
    ptbMap->Set_Leader_monster_flag(0);
}

TINT64 CMapBase::GetPflag(TINT64 ddwMapType, TINT64 ddwMapPos)
{
    return (ddwMapType * 100 + GetProvinceFromPos(ddwMapPos)) * 1000 * MAP_X_Y_POS_COMPUTE_OFFSET + ddwMapPos;
}

TVOID CMapBase::ComputeWildInfo(TbMap *pstWild)
{
    if (pstWild->m_nUid)
    {
        return;
    }

    // wave@20130708: 避免地图出错情况
    if (pstWild->m_nType == EN_WILD_TYPE__CITY && pstWild->m_nUid == 0)
    {
        pstWild->Set_Type(EN_WILD_TYPE__NORMAL);
        pstWild->Set_Level(1);
        pstWild->Set_Name("");
        pstWild->Set_Status(0);
        pstWild->Set_Might(0);
        pstWild->Set_Force_kill(0);
        pstWild->Set_Cid(0);
        pstWild->Set_Uid(0);
        pstWild->Set_Uname("");
        pstWild->Set_Ulevel(0);
        pstWild->Set_Alid(0);
        pstWild->Set_Alname("");
        pstWild->Set_Npc(0);
        pstWild->Set_Bid(0, UPDATE_ACTION_TYPE_DELETE);
        //pstWild->DeleteField(TbMAP_FIELD_BID);
        pstWild->Set_Name_update_time(CTimeUtils::GetUnixTime());
        pstWild->Set_Leader_monster_flag(0);
    }
}

TINT64 CMapBase::GetBlockIdFromPos(TINT64 ddwMapPos)
{
    TINT64 ddwIdx = ddwMapPos / MAP_X_Y_POS_COMPUTE_OFFSET;
    TINT64 ddwIdy = ddwMapPos % MAP_X_Y_POS_COMPUTE_OFFSET;

    if (0 == ddwIdx || 0 == ddwIdy)
    {
        return 0;
    }

    return ((ddwIdx - 1) / 10) * 1000 + ((ddwIdy - 1) / 10) + 1;
}

// =================================================== private ===================================================== //
TINT64 CMapBase::GetProvinceFromPos(TINT64 ddwMapPos)
{
    TINT64 ddwIdx = ddwMapPos / MAP_X_Y_POS_COMPUTE_OFFSET;
    TINT64 ddwIdy = ddwMapPos % MAP_X_Y_POS_COMPUTE_OFFSET;

    return ((ddwIdx - 1) / 125) + 4 * ((ddwIdy - 1) / 250);
}

TVOID CMapBase::SetMonseterNestMap(TbMap *ptbMap)
{
    const Json::Value& oWildResJson = CWildInfo::GetWildResInfo(ptbMap->m_nSid);
    if (oWildResJson.isMember(CCommonFunc::NumToString(ptbMap->m_nType)) == FALSE)
    {
        return;
    }
    const Json::Value& jTypeJson = oWildResJson[CCommonFunc::NumToString(ptbMap->m_nType)];
    if(jTypeJson["a0"]["a0"].asUInt() != EN_WILD_CLASS_MONSTER
        && jTypeJson["a0"]["a0"].asUInt() != EN_WILD_CLASS_LEADER_MONSTER)
    {
        return;
    }
    TUINT32 udwWildType = 0;
    TUINT32 udwTotalRate = 0;
    TUINT32 udwLevel = ptbMap->m_nLevel;

    for (TUINT32 udwIdx = 0; udwIdx < jTypeJson["a0"]["a13"][udwLevel - 1].size(); udwIdx++)
    {
        udwTotalRate += jTypeJson["a0"]["a13"][udwLevel - 1][udwIdx][1U].asUInt();
    }

    if (udwTotalRate != 0)
    {
        TUINT32 udwRate = rand() % udwTotalRate;
        for (TUINT32 udwIdx = 0; udwIdx < jTypeJson["a0"]["a13"][udwLevel - 1].size(); udwIdx++)
        {
            if (udwRate < jTypeJson["a0"]["a13"][udwLevel - 1][udwIdx][1U].asUInt())
            {
                udwWildType = jTypeJson["a0"]["a13"][udwLevel - 1][udwIdx][0U].asUInt();
                break;
            }
            else
            {
                udwRate -= jTypeJson["a0"]["a13"][udwLevel - 1][udwIdx][1U].asUInt();
            }
        }
    }

    if (udwWildType == 0)
    {
        CMapBase::ResetMap(ptbMap);
        return;
    }

    if (oWildResJson.isMember(CCommonFunc::NumToString(udwWildType)) == FALSE)
    {
        return;
    }

    TUINT32 udwContinueTime = oWildResJson[CCommonFunc::NumToString(udwWildType)]["a0"]["a12"][(TUINT32)ptbMap->m_nLevel - 1].asUInt();

    // update map info
    ptbMap->Set_Utime(CTimeUtils::GetUnixTime());
    
    ptbMap->Set_Type(udwWildType);
    ptbMap->Set_Name("");
    ptbMap->Set_Status(0);
    ptbMap->Set_Cid(0);
    ptbMap->Set_Uid(0);
    ptbMap->Set_Uname("");
    ptbMap->Set_Ulevel(0);
    ptbMap->Set_Alid(0);
    ptbMap->Set_Alname("");
    ptbMap->Set_Npc(0);
    ptbMap->Set_Bid(CMapBase::GetBlockIdFromPos(ptbMap->m_nId));
    ptbMap->Set_Name_update_time(CTimeUtils::GetUnixTime());

    ptbMap->Set_Reward_left(10000);
    
    ptbMap->m_bTroop.Reset();
    ptbMap->SetFlag(TbMAP_FIELD_TROOP);

    ptbMap->Set_Might(0);
    ptbMap->Set_Force_kill(0);
    ptbMap->m_bAl_attack_info.Reset();
    ptbMap->m_bAttack_info.Reset();
    ptbMap->SetFlag(TbMAP_FIELD_AL_ATTACK_INFO);
    ptbMap->SetFlag(TbMAP_FIELD_ATTACK_INFO);
    ptbMap->Set_Leader_monster_flag(0);

    ptbMap->Set_Wild_gen_time(ptbMap->m_nUtime);
    ptbMap->Set_Expire_time(ptbMap->m_nUtime + udwContinueTime);
}

TVOID CMapBase::SetLeaderMonsterMap(TbMap *ptbMap)
{
    const Json::Value& oWildResJson = CWildInfo::GetWildResInfo(ptbMap->m_nSid);
    if (oWildResJson.isMember(CCommonFunc::NumToString(ptbMap->m_nType)) == FALSE)
    {
        return;
    }
    const Json::Value& jTypeJson = oWildResJson[CCommonFunc::NumToString(ptbMap->m_nType)];
    if (jTypeJson["a0"]["a0"].asUInt() != EN_WILD_CLASS_MONSTER)
    {
        return;
    }

    TUINT32 udwLevel = ptbMap->m_nLevel;
    TUINT32 udwType = jTypeJson["a0"]["a15"].asUInt();

    ptbMap->Set_Level(udwLevel);
    ptbMap->Set_Type(udwType);
    ptbMap->Set_Utime(CTimeUtils::GetUnixTime());
    ptbMap->Set_Bid(CMapBase::GetBlockIdFromPos(ptbMap->m_nId));
    ptbMap->Set_Showtime(CTimeUtils::GetUnixTime());
    ptbMap->Set_Pic_index(1);
    ptbMap->Set_Boss_life(oWildResJson[CCommonFunc::NumToString(ptbMap->m_nType)]["a11"][udwLevel - 1][1U].asUInt());
    ptbMap->Set_Wild_gen_time(CTimeUtils::GetUnixTime());
    ptbMap->Set_Expire_time(ptbMap->m_nWild_gen_time + MAX_MONSTER_SHOW_INTERVAL_TIME);
    ptbMap->Set_Might(0);
    ptbMap->Set_Force_kill(0);
    ptbMap->m_bAl_attack_info.Reset();
    ptbMap->m_bAttack_info.Reset();
    ptbMap->SetFlag(TbMAP_FIELD_AL_ATTACK_INFO);
    ptbMap->SetFlag(TbMAP_FIELD_ATTACK_INFO);

    ptbMap->Set_Leader_monster_flag(1);
}

TUINT32 CMapBase::GetWildBlockNumByType(TUINT32 udwSid, TUINT32 udwType)
{
    const Json::Value& oWildJson = CWildInfo::GetWildResInfo(udwSid);
    TUINT32 udwTmpBlockNum = 0;

    if(oWildJson.isMember(CCommonFunc::NumToString(udwType)) && oWildJson[CCommonFunc::NumToString(udwType)]["a0"].isMember("a8"))
    {
        udwTmpBlockNum = oWildJson[CCommonFunc::NumToString(udwType)]["a0"]["a8"].asUInt();
    }
    else
    {
        switch(udwType)
        {
        case EN_WILD_TYPE__CITY:
            udwTmpBlockNum = 1;
            break;
        case EN_WILD_TYPE__THRONE_NEW:
            udwTmpBlockNum = 5;
            break;
        case EN_WILD_TYPE__IDOL:
            udwTmpBlockNum = 3;
            break;
        default:
            udwTmpBlockNum = 1;
            break;
        }
    }

    return udwTmpBlockNum;
}
