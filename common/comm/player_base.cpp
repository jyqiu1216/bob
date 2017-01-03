#include "player_base.h"
#include "common_base.h"
#include "common_func.h"
#include "activities_logic.h"
#include "service_key.h"
#include "quest_logic.h"
#include "city_base.h"
#include "action_base.h"
#include "item_base.h"
#include "msg_base.h"
#include "conf_base.h"
#include "sendmessage_base.h"

TBOOL CPlayerBase::HasEnoughGem(TbLogin *ptbLogin, TUINT32 udwGemNum)
{
    if (ptbLogin->m_nGem >= udwGemNum)
    {
        return TRUE;
    }
    return FALSE;
}

TVOID CPlayerBase::CostGem(SUserInfo *pstUser, TUINT32 udwGemNum)
{
    TbLogin *ptbLogin = &pstUser->m_tbLogin;
    if (ptbLogin->m_nGem > udwGemNum)
    {
        ptbLogin->m_nGem -= udwGemNum;
    }
    else
    {
        ptbLogin->m_nGem = 0;
    }
    CActivitesLogic::ComputeCostGemScore(pstUser, udwGemNum);

    ptbLogin->m_mFlag[TbLOGIN_FIELD_GEM] = UPDATE_ACTION_TYPE_PUT;
}

TVOID CPlayerBase::AddGem(TbLogin *ptbLogin, TUINT32 udwGemNum)
{
    ptbLogin->m_nGem += udwGemNum;
    ptbLogin->m_mFlag[TbLOGIN_FIELD_GEM] = UPDATE_ACTION_TYPE_PUT;
}

TVOID CPlayerBase::SetGem(TbLogin *ptbLogin, TUINT32 udwGemNum)
{
    ptbLogin->m_nGem = udwGemNum;
    ptbLogin->m_mFlag[TbLOGIN_FIELD_GEM] = UPDATE_ACTION_TYPE_PUT;
}

TBOOL CPlayerBase::IsDeadPlayer(TbPlayer *ptbPlayer, TUINT32 udwTime)
{
    if (udwTime - ptbPlayer->m_nUtime > MAX_PLAYER_DEAD_TIME)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

TBOOL CPlayerBase::HasChangePlayerName(const TbPlayer* ptbPlayer)
{
    if (NULL == strstr(ptbPlayer->m_sUin.c_str(), "Player"))
    {
        return TRUE;
    }
    return FALSE;
}

TVOID CPlayerBase::AddLordExp(SUserInfo *pstUser, SCityInfo *pstCity, TUINT32 udwExp, TBOOL bNeedBuff)
{
    TbPlayer* ptbPlayer = &pstUser->m_tbPlayer;

    if (bNeedBuff)
    {
        udwExp *= 1 + 1.0 * pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_LORD_EXP_BONUS].m_ddwBuffTotal / 10000;
    }

    ptbPlayer->Set_Exp(ptbPlayer->m_nExp + udwExp);
    CQuestLogic::SetTaskCurValue(pstUser, pstCity, EN_TASK_TYPE_ING_ADD_HERO_EXP, udwExp);

    if(ptbPlayer->m_nExp > CPlayerBase::GetMaxLordExp())
    {
        ptbPlayer->Set_Exp(CPlayerBase::GetMaxLordExp());
    }
}

TVOID CPlayerBase::AddDragonExp(SUserInfo *pstUser, TUINT32 udwExp, TBOOL bNeedBuff)
{
    TbPlayer* ptbPlayer = &pstUser->m_tbPlayer;

    TINT64 ddwMaxExp = CPlayerBase::GetMaxDragonExp(ptbPlayer->m_nDragon_max_lv);

    if (ptbPlayer->m_nDragon_exp < ddwMaxExp)
    {
        if (bNeedBuff)
        {
            udwExp *= 1 + 1.0 * pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_DRAGON_EXP_BONUS].m_ddwBuffTotal / 10000;
        }
        ptbPlayer->Set_Dragon_exp(ptbPlayer->m_nDragon_exp + udwExp);
        if (ptbPlayer->m_nDragon_exp > ddwMaxExp)
        {
            ptbPlayer->Set_Dragon_exp(ddwMaxExp);
        }
    }
}

TINT64 CPlayerBase::GetRawVipLevelPoint(TINT32 dwLevel)
{
    //dwLevel = (dwLevel <= 1) ? 1 : dwLevel;
    //const Json::Value& oJsonVip = CGameInfo::GetInstance()->m_oJsonRoot["game_vip"];
    //if(dwLevel > static_cast<TINT32>(oJsonVip["a"].size()))
    //{
    //    return oJsonVip["a"][oJsonVip["a"].size() - 1].asInt64();
    //}
    //return oJsonVip["a"][dwLevel - 1].asInt64();

    dwLevel = (dwLevel <= 1) ? 1 : dwLevel;
    const Json::Value& oJsonVip = CGameInfo::GetInstance()->m_oJsonRoot["game_vip_new"];
    if (dwLevel > static_cast<TINT32>(oJsonVip.size()))
    {
        return oJsonVip[oJsonVip.size() - 1]["a"][0u].asInt64();
    }
    return oJsonVip[dwLevel - 1]["a"][0u].asInt64();
}

TINT32 CPlayerBase::GetRawVipLevel(TbPlayer* ptbPlayer, TINT64 ddwVipPoint)
{
    //const Json::Value& oJsonVip = CGameInfo::GetInstance()->m_oJsonRoot["game_vip"];
    //TUINT32 dwVipLevel = 0;
    //for(dwVipLevel = 0; dwVipLevel < oJsonVip["a"].size(); ++dwVipLevel)
    //{
    //    if(ddwVipPoint < oJsonVip["a"][dwVipLevel].asInt64())
    //    {
    //        break;
    //    }
    //}
    //return dwVipLevel;
    const Json::Value& oJsonVip = CGameInfo::GetInstance()->m_oJsonRoot["game_vip_new"];
    TINT32 dwVipLevel = 0;
    for (dwVipLevel = 0; dwVipLevel < oJsonVip.size(); dwVipLevel++)
    {
        if (ddwVipPoint < oJsonVip[dwVipLevel]["a"][0u].asInt64() || ptbPlayer->m_nVip_stage < oJsonVip[dwVipLevel]["a"][1u].asInt64())
        {
            break;
        }
    }
    return dwVipLevel;
}

TINT32 CPlayerBase::GetRawVipStage(TINT32 dwLevel)
{
    //const Json::Value& oJsonVip = CGameInfo::GetInstance()->m_oJsonRoot["game_vip_new"];
    //TINT32 dwVipStage;
    //TUINT32 udwIdx;
    //for (udwIdx = 0; udwIdx < oJsonVip.size(); udwIdx++)
    //{
    //    if (ddwVipPoint < oJsonVip[udwIdx]["a"][0u].asInt64())
    //    {
    //        break;
    //    }
    //}
    //dwVipStage = oJsonVip[udwIdx-1]["a"][1u].asUInt();
    //return dwVipStage;
    dwLevel = (dwLevel <= 1) ? 1 : dwLevel;
    const Json::Value& oJsonVip = CGameInfo::GetInstance()->m_oJsonRoot["game_vip_new"];
    if (dwLevel > static_cast<TINT32>(oJsonVip.size()))
    {
        return oJsonVip[oJsonVip.size() - 1]["a"][1u].asInt64();
    }
    return oJsonVip[dwLevel - 1]["a"][1u].asInt64();
}

TINT64 CPlayerBase::GetMaxVipPoint(TbPlayer* ptbPlayer)
{   //当前阶段最大vip点数
    TUINT32 udwVipStage = ptbPlayer->m_nVip_stage;
    TUINT32 udwIdx;
    const Json::Value& oJsonVip = CGameInfo::GetInstance()->m_oJsonRoot["game_vip_new"];
    for (udwIdx = 0; udwIdx < oJsonVip.size(); udwIdx++)
    {
        if (udwVipStage < oJsonVip[udwIdx]["a"][1u].asUInt())
        {
            udwIdx--;
            break;
        }
    }
    return  oJsonVip[udwIdx]["a"][0u].asInt64();
}

TINT64 CPlayerBase::GetMaxVipPoint()
{
    const Json::Value& oJsonVip = CGameInfo::GetInstance()->m_oJsonRoot["game_vip_new"];
    return oJsonVip[oJsonVip.size() - 1]["a"][0u].asInt64();
}

TINT32 CPlayerBase::GetMaxVipStage()
{
    const Json::Value& oJsonVip = CGameInfo::GetInstance()->m_oJsonRoot["game_vip_new"];
    return oJsonVip[oJsonVip.size() - 1]["a"][1u].asInt();
}

TINT32 CPlayerBase::GetVipLevel(TbPlayer* ptbPlayer)
{
    if(ptbPlayer->m_nVip_etime < CTimeUtils::GetUnixTime())
    {
        return 0;
    }
    return CPlayerBase::GetRawVipLevel(ptbPlayer, ptbPlayer->m_nVip_point);
}

TINT32 CPlayerBase::ComputePlayerLevel(TbPlayer* ptbPlayer)
{
    TUINT32 dwLevel = 0;
    const Json::Value& oJsonExp = CGameInfo::GetInstance()->m_oJsonRoot["game_lord_exp"];
    for(; dwLevel < oJsonExp.size(); ++dwLevel)
    {
        if(ptbPlayer->m_nExp < oJsonExp[dwLevel][0u].asInt64())
        {
            break;
        }
    }
    dwLevel = dwLevel > 0 ? dwLevel - 1 : dwLevel;

    return dwLevel;
}

TINT32 CPlayerBase::ComputeDragonLevel(TINT64 ddwExp)
{
    TUINT32 dwLevel = 0;
    const Json::Value& oJsonExp = CGameInfo::GetInstance()->m_oJsonRoot["game_dragon_exp"];
    for (; dwLevel < oJsonExp.size(); ++dwLevel)
    {
        if (ddwExp < oJsonExp[dwLevel][0u].asInt64())
        {
            break;
        }
    }
    dwLevel = dwLevel > 0 ? dwLevel - 1 : dwLevel;

    return dwLevel;
}

TINT32 CPlayerBase::GetLeftLordSkillPoint(TbPlayer* ptbPlayer, TbUser_stat* ptbUserStat)
{
    TUINT32 dwLevel = ptbPlayer->m_nLevel;
    const Json::Value& oJsonExp = CGameInfo::GetInstance()->m_oJsonRoot["game_lord_exp"];
    if (dwLevel >= oJsonExp.size())
    {
        dwLevel = oJsonExp.size() - 1;
    }
    TINT32 dwTotalPoint = oJsonExp[dwLevel][1u].asInt();
    TINT32 dwUsedPoints = 0;
    for (TUINT32 udwIdx = 0; udwIdx < EN_SKILL_TYPE__END; ++udwIdx)
    {
        dwUsedPoints += ptbUserStat->m_bLord_skill[0].m_addwLevel[udwIdx];
    }
    return dwTotalPoint > dwUsedPoints ? dwTotalPoint - dwUsedPoints : 0;

    return 0;
}

TINT32 CPlayerBase::GetLeftLordSkillPoint( SUserInfo *pstUserInfo )
{
    TINT32 dwTotalPoint = pstUserInfo->m_stPlayerBuffList[EN_BUFFER_INFO_INCREACE_LORD_SKILL_POINT].m_ddwBuffTotal;
    TINT32 dwUsedPoints = GetUsedLoadSkillPoint(&pstUserInfo->m_tbUserStat);
    return dwTotalPoint > dwUsedPoints ? dwTotalPoint - dwUsedPoints : 0;
}

TINT32 CPlayerBase::GetUsedLoadSkillPoint( TbUser_stat* ptbUserStat )
{
    TINT32 dwUsedPoints = 0;
    for (TUINT32 udwIdx = 0; udwIdx < EN_SKILL_TYPE__END; ++udwIdx)
    {
        dwUsedPoints += ptbUserStat->m_bLord_skill[0].m_addwLevel[udwIdx];
    }
    return dwUsedPoints;
}

TINT32 CPlayerBase::GetLordSkillPointLimit(TINT32 dwSkillId)
{
    const Json::Value& oJsonSkill = CGameInfo::GetInstance()->m_oJsonRoot["game_lord_skill"];
    string strSkillId = CCommonFunc::NumToString(dwSkillId);
    if (oJsonSkill.isMember(strSkillId) == FALSE)
    {
        return 0;
    }
    return oJsonSkill[strSkillId]["a"]["a0"].asInt();
}

TINT32 CPlayerBase::GetLeftDragonSkillPoint(TbPlayer* ptbPlayer, TbUser_stat* ptbUserStat)
{
    TUINT32 dwLevel = ptbPlayer->m_nDragon_level;
    const Json::Value& oJsonExp = CGameInfo::GetInstance()->m_oJsonRoot["game_dragon_exp"];
    if(dwLevel >= oJsonExp.size())
    {
        dwLevel = oJsonExp.size() - 1;
    }
    TINT32 dwTotalPoint = oJsonExp[dwLevel][1u].asInt();
    TINT32 dwUsedPoints = 0;
    for(TUINT32 udwIdx = 0; udwIdx < EN_SKILL_TYPE__END; ++udwIdx)
    {
        dwUsedPoints += ptbUserStat->m_bDragon_skill[0].m_addwLevel[udwIdx];
    }
    return dwTotalPoint > dwUsedPoints ? dwTotalPoint - dwUsedPoints : 0;
}

TINT32 CPlayerBase::GetLeftDragonSkillPoint( SUserInfo *pstUserInfo )
{
    TbUser_stat *ptbUserStat = &pstUserInfo->m_tbUserStat;
    TINT32 dwTotalPoint = pstUserInfo->m_stPlayerBuffList[EN_BUFFER_INFO_INCREACE_DRAGON_SKILL_POINT].m_ddwBuffTotal;
    TINT32 dwUsedPoints = 0;
    for(TUINT32 udwIdx = 0; udwIdx < EN_SKILL_TYPE__END; ++udwIdx)
    {
        dwUsedPoints += ptbUserStat->m_bDragon_skill[0].m_addwLevel[udwIdx];
    }
    return dwTotalPoint > dwUsedPoints ? dwTotalPoint - dwUsedPoints : 0;
}

TINT32 CPlayerBase::GetDragonSkillPointLimit(TINT32 dwSkillId)
{
    const Json::Value& oJsonSkill = CGameInfo::GetInstance()->m_oJsonRoot["game_dragon_skill"];
    string strSkillId = CCommonFunc::NumToString(dwSkillId);
    if(oJsonSkill.isMember(strSkillId) == FALSE)
    {
        return 0;
    }
    return oJsonSkill[strSkillId]["a"]["a0"].asInt();
}

TINT32 CPlayerBase::GetLeftDragonMonsterSkillPoint(TbPlayer* ptbPlayer, TbUser_stat* ptbUserStat)
{
    TUINT32 dwLevel = ptbPlayer->m_nDragon_level;
    const Json::Value& oJsonExp = CGameInfo::GetInstance()->m_oJsonRoot["game_dragon_exp"];
    if(dwLevel >= oJsonExp.size())
    {
        dwLevel = oJsonExp.size() - 1;
    }
    TINT32 dwTotalPoint = oJsonExp[dwLevel][2u].asInt();
    TINT32 dwUsedPoints = 0;
    for (TUINT32 udwIdx = 0; udwIdx < EN_SKILL_TYPE__END; ++udwIdx)
    {
        dwUsedPoints += ptbUserStat->m_bDragon_monster_skill[0].m_addwLevel[udwIdx];
    }
    return dwTotalPoint > dwUsedPoints ? dwTotalPoint - dwUsedPoints : 0;
}

TINT32 CPlayerBase::GetLeftDragonMonsterSkillPoint( SUserInfo *pstUserInfo )
{
    TbUser_stat *ptbUserStat = &pstUserInfo->m_tbUserStat;
    TINT32 dwTotalPoint = pstUserInfo->m_stPlayerBuffList[EN_BUFFER_INFO_INCREACE_MONSTER_SKILL_POINT].m_ddwBuffTotal;
    TINT32 dwUsedPoints = 0;
    for (TUINT32 udwIdx = 0; udwIdx < EN_SKILL_TYPE__END; ++udwIdx)
    {
        dwUsedPoints += ptbUserStat->m_bDragon_monster_skill[0].m_addwLevel[udwIdx];
    }
    return dwTotalPoint > dwUsedPoints ? dwTotalPoint - dwUsedPoints : 0;
}

TINT32 CPlayerBase::GetDragonMonsterSkillPointLimit(TINT32 dwSkillId)
{
    const Json::Value& oJsonSkill = CGameInfo::GetInstance()->m_oJsonRoot["game_dragon_skill_monster"];
    string strSkillId = CCommonFunc::NumToString(dwSkillId);
    if(oJsonSkill.isMember(strSkillId) == FALSE)
    {
        return 0;
    }
    return oJsonSkill[strSkillId]["a"]["a0"].asInt();
}

TBOOL CPlayerBase::IsMeetLordSkillReliance(SUserInfo* pstUser, TINT32 dwSkillId, TINT32 dwSKillLevel)
{
    const Json::Value& oJsonSkill = CGameInfo::GetInstance()->m_oJsonRoot["game_lord_skill"];
    string strSkillId = CCommonFunc::NumToString(dwSkillId);
    SSkill* pstSkill = &pstUser->m_tbUserStat.m_bLord_skill[0];
    const Json::Value& oJsonSkillReliance = oJsonSkill[strSkillId]["r"]["r1"][dwSKillLevel - 1];
    for (TUINT32 udwIdx = 0; udwIdx < oJsonSkillReliance.size(); ++udwIdx)
    {
        //可以在此处扩展依赖的类型
        TINT32 dwRelySkillId = oJsonSkillReliance[udwIdx][1u].asInt();
        TINT32 dwRelySkillLevel = oJsonSkillReliance[udwIdx][2u].asInt();
        if (pstSkill->m_addwLevel[dwRelySkillId] < dwRelySkillLevel)
        {
            return FALSE;
        }
    }
    return TRUE;
}

TBOOL CPlayerBase::IsMeetDragonSkillReliance(SUserInfo* pstUser, TINT32 dwSkillId, TINT32 dwSKillLevel)
{
    const Json::Value& oJsonSkill = CGameInfo::GetInstance()->m_oJsonRoot["game_dragon_skill"];
    string strSkillId = CCommonFunc::NumToString(dwSkillId);
    SSkill* pstSkill = &pstUser->m_tbUserStat.m_bDragon_skill[0];
    const Json::Value& oJsonSkillReliance = oJsonSkill[strSkillId]["r"]["r1"][dwSKillLevel - 1];
    for (TUINT32 udwIdx = 0; udwIdx < oJsonSkillReliance.size(); ++udwIdx)
    {
        //可以在此处扩展依赖的类型
        TINT32 dwRelySkillId = oJsonSkillReliance[udwIdx][1u].asInt();
        TINT32 dwRelySkillLevel = oJsonSkillReliance[udwIdx][2u].asInt();
        if (pstSkill->m_addwLevel[dwRelySkillId] < dwRelySkillLevel)
        {
            return FALSE;
        }
    }
    return TRUE;
}

TBOOL CPlayerBase::IsMeetDragonMonsterSkillReliance(SUserInfo* pstUser, TINT32 dwSkillId, TINT32 dwSKillLevel)
{
    const Json::Value& oJsonSkill = CGameInfo::GetInstance()->m_oJsonRoot["game_dragon_skill_monster"];
    string strSkillId = CCommonFunc::NumToString(dwSkillId);
    SSkill* pstSkill = &pstUser->m_tbUserStat.m_bDragon_monster_skill[0];
    const Json::Value& oJsonSkillReliance = oJsonSkill[strSkillId]["r"]["r1"][dwSKillLevel - 1];
    for (TUINT32 udwIdx = 0; udwIdx < oJsonSkillReliance.size(); ++udwIdx)
    {
        //可以在此处扩展依赖的类型
        TINT32 dwRelySkillId = oJsonSkillReliance[udwIdx][1u].asInt();
        TINT32 dwRelySkillLevel = oJsonSkillReliance[udwIdx][2u].asInt();
        if (pstSkill->m_addwLevel[dwRelySkillId] < dwRelySkillLevel)
        {
            return FALSE;
        }
    }
    return TRUE;
}

TINT32 CPlayerBase::AddLoyality(TbPlayer *pstPlayer, TUINT32 udwNum)
{
    if(pstPlayer->m_nAlid != 0 && pstPlayer->m_nAlpos != EN_ALLIANCE_POS__REQUEST)
    {
        pstPlayer->Set_Loy_all(udwNum + pstPlayer->Get_Loy_all());
        pstPlayer->Set_Loy_cur(udwNum + pstPlayer->Get_Loy_cur());

    }
    return 0;
}

TINT32 CPlayerBase::CostLoyality(TbPlayer *pstPlayer, TUINT32 udwNum)
{
    pstPlayer->Set_Loy_all(pstPlayer->Get_Loy_all() > udwNum ? pstPlayer->Get_Loy_all() - udwNum : 0);
    pstPlayer->Set_Loy_cur(pstPlayer->Get_Loy_cur() > udwNum ? pstPlayer->Get_Loy_cur() - udwNum : 0);

    return 0;
}

TINT32 CPlayerBase::HasEnoughLoyality(TbPlayer *pstPlayer, TUINT32 udwNum)
{
    if(pstPlayer->Get_Loy_cur() > udwNum)
    {
        return TRUE;
    }
    return FALSE;
}


TINT64 CPlayerBase::GetMaxDragonExp(TINT64 ddwMaxLv)
{
    CGameInfo *poGameInfo = CGameInfo::GetInstance();
    const Json::Value& oJson = poGameInfo->m_oJsonRoot["game_dragon_exp"];

    TUINT32 udwLv = ddwMaxLv;

    if (oJson.size() == 0)
    {
        return 0;
    }

    if (udwLv == 0 || udwLv > oJson.size() - 1)
    {
        udwLv = oJson.size() - 1;
    }

    return oJson[udwLv][0U].asInt64();
}

TINT64 CPlayerBase::GetMaxLordExp()
{
    CGameInfo *poGameInfo = CGameInfo::GetInstance();
    const Json::Value& oJson = poGameInfo->m_oJsonRoot["game_lord_exp"];

    if (oJson.size() > 0)
    {
        return oJson[oJson.size() - 1][0U].asInt64();
    }

    return 0;
}

TUINT32 CPlayerBase::GetDragonExcuteTime(TUINT32 udwLevel)
{
    CGameInfo *pstGameInfo = CGameInfo::GetInstance();
    const Json::Value& oJson = pstGameInfo->m_oJsonRoot["game_dragon_exp"];
    return oJson[udwLevel][4u].asUInt();
}

TUINT32 CPlayerBase::GetAltarBuffTime(TUINT32 udwHerolv)
{
    CGameInfo *pstGameInfo = CGameInfo::GetInstance();
    const Json::Value& oJson = pstGameInfo->m_oJsonRoot["game_dragon_exp"];
    return oJson[udwHerolv][5u].asUInt();
}

TINT32 CPlayerBase::CheckUpdtDragonEnergy(SUserInfo *pstUser)
{
    TINT32 dwRetCode = 1;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;    if(ptbPlayer->m_nHas_dragon == 0) //没有龙时直接返回
    {
        return dwRetCode;
    }

    TINT64 ddwMaxEnergy = GetCurDragonMaxEnergy(pstUser);

    const Json::Value &jDragon = CGameInfo::GetInstance()->m_oJsonRoot["game_dragon"];

    if (ptbPlayer->m_nDragon_max_energy == 0)
    {
        ptbPlayer->Set_Dragon_max_energy(ddwMaxEnergy);
    }
    else if (ptbPlayer->m_nDragon_max_energy != ddwMaxEnergy)
    {
        TINT64 ddwCurEnergy = ptbPlayer->m_nDragon_cur_energy;
        ddwCurEnergy = ceil(ddwCurEnergy * 1.0 / ptbPlayer->m_nDragon_max_energy * ddwMaxEnergy);
        if (ddwCurEnergy > ddwMaxEnergy)
        {
            ddwCurEnergy = ddwMaxEnergy;
        }
        ptbPlayer->Set_Dragon_cur_energy(ddwCurEnergy);
        ptbPlayer->Set_Dragon_max_energy(ddwMaxEnergy);
    }
    TINT64 ddwCurTime = CTimeUtils::GetUnixTime();
    if (ptbPlayer->m_nDragon_recovery_time != 0 && ddwCurTime > ptbPlayer->m_nDragon_recovery_time)
    {
        ptbPlayer->Set_Dragon_cur_energy(ddwMaxEnergy);
        ptbPlayer->Set_Dragon_recovery_time(0);

        //ADD NOTIFICATION
        SNoticInfo stNoticInfo;
        stNoticInfo.Reset();
        stNoticInfo.SetValue(EN_NOTI_ID__HERO_ENERGY_RECOVERED,
            "", "",
            0, 0,
            0, 0,
            0, "", 0);
        CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), pstUser->m_tbPlayer.m_nSid, pstUser->m_tbPlayer.m_nUid, stNoticInfo);

        dwRetCode = 0;
    }

    if (ptbPlayer->m_nDragon_cur_energy < ddwMaxEnergy && ptbPlayer->m_nDragon_recovery_time == 0)
    {
        if (ptbPlayer->m_nDragon_begin_recovery_time + jDragon["a5"].asInt64() < ddwCurTime)
        {
            ptbPlayer->Set_Dragon_recovery_count(0);
        }
        assert(jDragon["a4"].size());
        ptbPlayer->Set_Dragon_recovery_time(CTimeUtils::GetUnixTime() + jDragon["a4"][(TINT32)ptbPlayer->m_nDragon_recovery_count].asInt64());

        ptbPlayer->Set_Dragon_recovery_count(ptbPlayer->m_nDragon_recovery_count + 1);
        if (ptbPlayer->m_nDragon_recovery_count >= jDragon["a4"].size())
        {
            ptbPlayer->m_nDragon_recovery_count = jDragon["a4"].size() - 1;
        }
    }

    return dwRetCode;
}

TINT32 CPlayerBase::AddLordImage(TbLord_image *ptbLord_image, TUINT32 udwImageId)
{
    Json::Value& jTmp = ptbLord_image->m_jLord_image;
    for (TUINT32 udwIdx = 0; udwIdx < jTmp.size(); udwIdx++)
    {
        if (udwImageId == jTmp[udwIdx]["id"].asUInt())
        {
            return 0;
        }
    }
    Json::Value jImage = Json::Value(Json::objectValue);
    jImage["id"] = udwImageId;
    jTmp.append(jImage);
    ptbLord_image->SetFlag(TbLORD_IMAGE_FIELD_LORD_IMAGE);
    return 0;
}

TINT32 CPlayerBase::AddDecoration(TbDecoration *ptbDecoration, TUINT32 udwDecoId, TUINT32 udwItemNum /*=1*/)
{
    Json::Value& jTmp = ptbDecoration->m_jDecoration_list;
    string strDecoid;
    ostringstream oss;
    oss << udwDecoId;
    strDecoid = oss.str();
    if (jTmp.isMember(strDecoid))
    {
        jTmp[strDecoid]["still_have_num"] = jTmp[strDecoid]["still_have_num"].asInt() + udwItemNum;
        jTmp[strDecoid]["total_num"] = jTmp[strDecoid]["total_num"].asInt() + udwItemNum;
    }
    else
    {
        //种类数量限制
        TUINT32 udwMax = CCommonBase::GetGameBasicVal(EN_GAME_BASIC_DECORATION_UPPER_LIMIT);
        //TUINT32 udwMax = 3;
        Json::Value::Members jMember = ptbDecoration->m_jDecoration_list.getMemberNames();
        if (jMember.size() + udwItemNum > udwMax)
        {
            return EN_RET_CODE__DECORATION_OVREFLOW;
        }
        Json::Value jNewDeco = Json::Value(Json::objectValue);
        jNewDeco["get_time"] = CTimeUtils::GetUnixTime();
        jNewDeco["still_have_num"] = udwItemNum;
        jNewDeco["total_num"] = udwItemNum;
        jTmp[strDecoid] = jNewDeco;
    }
    ptbDecoration->SetFlag(TbDECORATION_FIELD_DECORATION_LIST);
    return 0;
}

TINT32 CPlayerBase::SetDecoration(TbDecoration *ptbDecoration, string strDecoId, TUINT32 udwItemNum)
{
    Json::Value jTmp = ptbDecoration->m_jDecoration_list;
    if (!jTmp.isMember(strDecoId) || jTmp[strDecoId]["total_num"].asUInt() < udwItemNum)
    {
        return EN_RET_CODE__REQ_PARAM_ERROR;
    }
    jTmp[strDecoId]["still_have_num"] = jTmp[strDecoId]["total_num"].asUInt() - udwItemNum;
    ptbDecoration->Set_Decoration_list(jTmp);
    return 0;
}

TINT32 CPlayerBase::HasEnoughDecoration(TbDecoration *ptbDecoration, TUINT32 udwDecoId, TUINT32 udwNum /*=1*/)
{
    Json::Value& jTmp = ptbDecoration->m_jDecoration_list;
    string strDecoid;
    ostringstream oss;
    oss << udwDecoId;
    strDecoid = oss.str();
    if (!jTmp.isMember(strDecoid))
    {
        return -1;
    }
    if (jTmp[strDecoid]["still_have_num"].asInt() < udwNum)
    {
        return -2;
    }
    return 0;
}

TINT32 CPlayerBase::CostDecoration(TbDecoration *ptbDecoration, TUINT32 udwDecoId)
{
    Json::Value& jTmp = ptbDecoration->m_jDecoration_list;
    string strDecoid;
    ostringstream oss;
    oss << udwDecoId;
    strDecoid = oss.str();
    if (!jTmp.isMember(strDecoid))
    {
        return -1;
    }
    if (jTmp[strDecoid]["still_have_num"].asInt() <= 0)
    {
        return -2;
    }
    jTmp[strDecoid]["still_have_num"] = jTmp[strDecoid]["still_have_num"].asInt() - 1;
    ptbDecoration->Set_Decoration_list(jTmp);
    return 0;
}

TINT32 CPlayerBase::PickUpDecoration(TbDecoration *ptbDecoration, TUINT32 udwDecoId)
{
    Json::Value& jTmp = ptbDecoration->m_jDecoration_list;
    string strDecoid;
    ostringstream oss;
    oss << udwDecoId;
    strDecoid = oss.str();
    if (!jTmp.isMember(strDecoid))
    {
        return -1;
    }
    jTmp[strDecoid]["still_have_num"] = jTmp[strDecoid]["still_have_num"].asInt() + 1;
    ptbDecoration->Set_Decoration_list(jTmp);
    return 0;
}

TINT32 CPlayerBase::AddDragonEnergy(SUserInfo *pstUser, TUINT32 udwNum)
{
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;
    TINT64 ddwMaxEnergy = CPlayerBase::GetCurDragonMaxEnergy(pstUser);
    TINT64 ddwCurEnergy = ptbPlayer->m_nDragon_cur_energy;

    ddwCurEnergy += udwNum;
    if(ddwCurEnergy > ddwMaxEnergy)
    {
        ddwCurEnergy = ddwMaxEnergy;
    }
    ptbPlayer->Set_Dragon_cur_energy(ddwCurEnergy);

    return 0;
}

TINT32 CPlayerBase::AddDragonShard(SUserInfo *pstUser, TUINT32 udwNum)
{
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;

    ptbPlayer->Set_Dragon_shard(ptbPlayer->m_nDragon_shard + udwNum);

    return 0;
}

TINT32 CPlayerBase::SetDragonShard(SUserInfo *pstUser, TUINT32 udwNum)
{
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;

    ptbPlayer->Set_Dragon_shard(udwNum);

    return 0;
}

TINT32 CPlayerBase::CostDragonShard(SUserInfo *pstUser, TUINT32 udwNum)
{
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;

    if (udwNum == 0)
    {
        return 0;
    }

    TINT64 ddwDragonShard = ptbPlayer->m_nDragon_shard;
    if (udwNum > ddwDragonShard)
    {
        ptbPlayer->Set_Dragon_shard(0);
    }
    else
    {
        ptbPlayer->Set_Dragon_shard(ddwDragonShard - udwNum);
    }

    return 0;
}

TBOOL CPlayerBase::HasEnoughDragonShard(SUserInfo *pstUser, TUINT32 udwNum)
{
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;

    if (ptbPlayer->m_nDragon_shard >= udwNum)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

TVOID CPlayerBase::GetTrialAttackCost(SUserInfo *pstUser, TUINT32 udwAtkNum, TUINT32 &udwItemNum, TUINT32 &udwShardNum)
{
    udwItemNum = 0;
    udwShardNum = 0;

    TUINT32 udwItemId = 836; // 免费攻击道具
    TUINT32 udwHasItemNum = CItemBase::GetItemNum(&pstUser->m_tbBackpack, udwItemId);

    TUINT32 udwShardCost = CCommonBase::GetGameBasicVal(EN_GAME_BASIC_TRIAL_NORMAL_DRAGON_SHARD_COST);
    if (pstUser->m_tbPlayer.m_nTrial_rage_mode == 1)
    {
        udwShardCost = CCommonBase::GetGameBasicVal(EN_GAME_BASIC_TRIAL_RAGE_DRAGON_SHARD_COST);
    }

    if (udwHasItemNum >= udwAtkNum)
    {
        udwItemNum = udwAtkNum;
    }
    else
    {
        udwItemNum = udwHasItemNum;
        udwShardNum = (udwAtkNum - udwHasItemNum) * udwShardCost;
    }
}

TINT64 CPlayerBase::GetCurDragonMaxEnergy(SUserInfo *pstUser)
{
    TINT64 ddwMaxEnergy = CCommonBase::GetGameBasicVal(EN_GAME_BASIC_HERO_ENERGY_DEFAULT);
    TFLOAT64 fHeroStaminaLimitRate = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_HERO_STAMINA_LIMIT].m_ddwBuffTotal / 10000.0;
    ddwMaxEnergy = ceil(ddwMaxEnergy*(1.0 + fHeroStaminaLimitRate));
    return ddwMaxEnergy;
}

TVOID CPlayerBase::CheckAndUpdtKnightInfo( SUserInfo *pstUser )
{
    TbCity_Knight& knight = pstUser->m_stCityInfo.m_stTblData.m_bKnight;
    TUINT32 udwNum = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_KNIGHT_NUM].m_ddwBuffTotal;
    //TUINT32 udwNum = CCityBase::GetBuildingLevelByFuncType(&pstUser->m_stCityInfo, EN_BUILDING_TYPE__KNIGHT_HALL);
    if(udwNum > knight.m_udwNum)
    {
        for(TUINT32 idx = knight.m_udwNum; idx < udwNum; idx++)
        {
            knight[idx].Reset();
        }
        knight.m_udwNum = udwNum;
        pstUser->m_stCityInfo.m_stTblData.SetFlag(TbCITY_FIELD_KNIGHT);
    }

    //容错处理
    for(TUINT32 idx = 0; idx < knight.m_udwNum; idx++)
    {
        if(knight[idx].ddwPos == EN_KNIGHT_POS__UNASSIGN)
        {
            if(knight[idx].ddwStatus == EN_KNIGHT_STATUS__ASSIGNING)
            {
                knight[idx].ddwStatus = EN_KNIGHT_STATUS__NORMAL;
                pstUser->m_stCityInfo.m_stTblData.SetFlag(TbCITY_FIELD_KNIGHT);
            }
        }

        if(knight[idx].ddwPos != EN_KNIGHT_POS__UNASSIGN)
        {
            if(knight[idx].ddwStatus != EN_KNIGHT_STATUS__ASSIGNING)
            {
                knight[idx].ddwStatus = EN_KNIGHT_STATUS__ASSIGNING;
                pstUser->m_stCityInfo.m_stTblData.SetFlag(TbCITY_FIELD_KNIGHT);
            }            
        }

        if(knight[idx].ddwStatus == EN_KNIGHT_STATUS__MARCHING)
        {
            if(knight[idx].ddwTid == 0)
            {
                knight[idx].ddwStatus = EN_KNIGHT_STATUS__NORMAL;
                pstUser->m_stCityInfo.m_stTblData.SetFlag(TbCITY_FIELD_KNIGHT);
            }
            else
            {
                TINT32 dwIndex = CActionBase::GetMarchIndex(pstUser->m_atbMarch, pstUser->m_udwMarchNum, knight[idx].ddwTid);
                if(dwIndex < 0)
                {
                    knight[idx].ddwStatus = EN_KNIGHT_STATUS__NORMAL;
                    pstUser->m_stCityInfo.m_stTblData.SetFlag(TbCITY_FIELD_KNIGHT);
                }
            }
        }
    }
}

TVOID CPlayerBase::AutoUnassignKnight( SUserInfo *pstUser )
{
    TINT32 dwUnassignId = -1;
    TbCity_Knight& knight = pstUser->m_stCityInfo.m_stTblData.m_bKnight;   
    for(TUINT32 idx = 0; idx < knight.m_udwNum; idx++)
    {
        if(knight[idx].ddwPos != EN_KNIGHT_POS__UNASSIGN)
        {
            knight[idx].ddwStatus = EN_KNIGHT_STATUS__NORMAL;
            knight[idx].ddwPos = EN_KNIGHT_POS__UNASSIGN;
            dwUnassignId = idx;
            pstUser->m_stCityInfo.m_stTblData.SetFlag(TbCITY_FIELD_KNIGHT);
        }
    }

    if(dwUnassignId >= 0)
    {
        //ADD NOTIFICATION
        SNoticInfo stNoticInfo;
        stNoticInfo.Reset();
        stNoticInfo.SetValue(EN_NOTI_ID__KNIGHT_BE_DISMISSED,
            "", "",
            0, 0,
            0, 0,
            0, "", 0);
        CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), pstUser->m_tbPlayer.m_nSid, pstUser->m_tbPlayer.m_nUid, stNoticInfo);

        // tips
        CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPE__KNIGHT_DISMISS, pstUser->m_tbPlayer.m_nUid, FALSE,
            dwUnassignId, 0, 0, pstUser->m_tbPlayer.m_sUin.c_str());
    }
    
}

TINT32 CPlayerBase::GetCurPersonGuide( SUserInfo *pstUser )
{
    // vip道具累计5个以上,三天没有使用
    //if(CheckPersonGuideVIP(pstUser, EN_PERSON_GUIDE_VIP))
    //{
    //    return EN_PERSON_GUIDE_VIP;
    //}

    // 领主到了8级一直没有分配过技能点
    //if(CheckPersonGuidePlayerSkill(pstUser, EN_PERSON_GUIDE_PLAYER_SKILL))
    //{
    //    return EN_PERSON_GUIDE_PLAYER_SKILL;    
    //}

    // 清除所有树木,TODO:新建建筑没有空间的情况下
    //if(CheckPersonGuideBuilding(pstUser, EN_PERSON_GUIDE_BUILDING))
    //{
    //    return EN_PERSON_GUIDE_BUILDING;    
    //}

    // 当有箱子未开启,指引玩家开箱
    if(CheckPersonGuideChest(pstUser, EN_PERSON_GUIDE_CHEST))
    {
        return EN_PERSON_GUIDE_CHEST;
    }

    // 玩家还没有加入联盟
    if(CheckPersonGuideJoinAlliance(pstUser, EN_PERSON_GUIDE_JOIN_ALLIANCE))
    {
        return EN_PERSON_GUIDE_JOIN_ALLIANCE;
    }
    
    //// 用户开始了一份队列时间大于1小时的任务
    //if(CheckPersonGuideNeverUseSpeedItem(pstUser, EN_PERSON_GUIDE_NEVER_USE_SPEED_ITEM))
    //{
    //    return EN_PERSON_GUIDE_NEVER_USE_SPEED_ITEM;    
    //}
    return 0;
}

TBOOL CPlayerBase::CheckPersonGuideVIP( SUserInfo *pstUser, TUINT32 udwKey )
{
    if(BITTEST(pstUser->m_tbPlayer.m_bPerson_guide[0].m_bitGuide, udwKey))
    {
        return FALSE;
    }

    TUINT32 udwItemNum = 0;
    CGameInfo *poGameInfo = CGameInfo::GetInstance();
    for(TUINT32 idx = 0; idx < pstUser->m_tbBackpack.m_bItem.m_udwNum; idx++)
    {
        TUINT32 udwFuncType = poGameInfo->m_oJsonRoot["game_item"][CCommonFunc::NumToString(pstUser->m_tbBackpack.m_bItem[idx].m_ddwItemId)]["a2"].asUInt();
        if(udwFuncType == 10024)
        {
            udwItemNum += pstUser->m_tbBackpack.m_bItem[idx].m_ddwItemNum;
        }
    }
    
    if(udwItemNum > 5)
    {
        return TRUE;
    }

    return FALSE;
}

TBOOL CPlayerBase::CheckPersonGuidePlayerSkill(SUserInfo *pstUser, TUINT32 udwKey)
{
    if(BITTEST(pstUser->m_tbPlayer.m_bPerson_guide[0].m_bitGuide, udwKey))
    {
        return FALSE;
    }

    TINT32 dwUsedPoints = CPlayerBase::GetUsedLoadSkillPoint(&pstUser->m_tbUserStat);
    if(pstUser->m_tbPlayer.Get_Level() >= 8
       && 0 == dwUsedPoints)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

TBOOL CPlayerBase::CheckPersonGuideBuilding(SUserInfo *pstUser, TUINT32 udwKey)
{
    if(BITTEST(pstUser->m_tbPlayer.m_bPerson_guide[0].m_bitGuide, udwKey))
    {
        return FALSE;
    }

    TBOOL bRetCode = TRUE;
    SCityBuildingNode *pstBuilding = NULL;

    //check还有无树木
    for(TUINT32 idx = 0; idx < pstUser->m_stCityInfo.m_stTblData.m_bBuilding.m_udwNum; idx++)
    {
        pstBuilding = &pstUser->m_stCityInfo.m_stTblData.m_bBuilding[idx];
        if(pstBuilding->m_ddwType >= 46 && pstBuilding->m_ddwType <= 58)
        {
            bRetCode = FALSE;
            break;
        }
    }

    //check 有无空间放建筑
    //todo

    return bRetCode;
}

TBOOL CPlayerBase::CheckPersonGuideChest(SUserInfo *pstUser, TUINT32 udwKey)
{
    if(BITTEST(pstUser->m_tbPlayer.m_bPerson_guide[0].m_bitGuide, udwKey))
    {
        return FALSE;
    }

    TUINT32 udwItemNum = 0;
    CGameInfo *poGameInfo = CGameInfo::GetInstance();
    for(TUINT32 idx = 0; idx < pstUser->m_tbBackpack.m_bItem.m_udwNum; idx++)
    {
        TUINT32 udwFuncType = poGameInfo->m_oJsonRoot["game_item"][CCommonFunc::NumToString(pstUser->m_tbBackpack.m_bItem[idx].m_ddwItemId)]["a1"].asUInt();
        if(udwFuncType == EN_ITEM_CATEGORY__CHEST && pstUser->m_tbBackpack.m_bItem[idx].m_ddwItemId == 403) //chest
        {
            udwItemNum += pstUser->m_tbBackpack.m_bItem[idx].m_ddwItemNum;
            break;
        }
    }

    //有箱子, 且大于等于第三个时代
    if(udwItemNum >= 1 && pstUser->m_tbPlayer.m_nAge >= 2)
    {
        return TRUE;
    }

    return FALSE;
}

TBOOL CPlayerBase::CheckPersonGuideJoinAlliance(SUserInfo *pstUser, TUINT32 udwKey)
{
    if(BITTEST(pstUser->m_tbPlayer.m_bPerson_guide[0].m_bitGuide, udwKey))
    {
        return FALSE;
    }

    //TUINT8 ucCastleLv = CCityBase::GetBuildingLevelByFuncType(&pstUser->m_stCityInfo, EN_BUILDING_TYPE__CASTLE);
    TUINT8 ucEmbassyLv = CCityBase::GetBuildingLevelByFuncType(&pstUser->m_stCityInfo, EN_BUILDING_TYPE__EMBASSY);
    if(ucEmbassyLv >= 1
       && 0 == pstUser->m_tbLogin.m_nAl_time
       && 0 == pstUser->m_tbPlayer.m_nAlid
       && EN_ALLIANCE_POS__REQUEST == pstUser->m_tbPlayer.m_nAlpos)
    {
        return TRUE;
    }
    
    return FALSE;
}

TBOOL CPlayerBase::CheckPersonGuideNeverUseSpeedItem(SUserInfo *pstUser, TUINT32 udwKey)
{
    if(BITTEST(pstUser->m_tbPlayer.m_bPerson_guide[0].m_bitGuide, udwKey))
    {
        return FALSE;
    }

    TUINT32 udwSpeedupItem = 11; //15fenzhong jiasu
    if(CItemBase::GetItemNum(&pstUser->m_tbBackpack, udwSpeedupItem) == 0) 
    {
        return FALSE;
    }

    TBOOL bRetCode = FALSE;
    TUINT32 idx = 0;
    const TUINT32 udwTimeThrd = 3600;
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    for(idx = 0; idx < pstUser->m_udwSelfAlActionNum; idx++)
    {
        if(pstUser->m_atbSelfAlAction[idx].m_nSuid != pstUser->m_tbPlayer.m_nUid)
        {
            continue;
        }
        if(pstUser->m_atbSelfAlAction[idx].m_nEtime > udwCurTime + udwTimeThrd)
        {
            bRetCode = TRUE;
            break;
        }
    }

    return bRetCode;
}

TINT32 CPlayerBase::AddDragon(TbPlayer *ptbPlayer, TINT64 ddwMaxEnergy)
{
    ptbPlayer->Set_Has_dragon(2L);
    ptbPlayer->Set_Dragon_exp(0);
    ptbPlayer->Set_Dragon_level(1);
    ptbPlayer->Set_Dragon_cur_energy(ddwMaxEnergy);
    ptbPlayer->Set_Dragon_max_energy(ddwMaxEnergy);
    ptbPlayer->Set_Dragon_recovery_time(0);
    ptbPlayer->Set_Dragon_recovery_count(0);
    ptbPlayer->Set_Dragon_begin_recovery_time(0);
    ptbPlayer->Set_Dragon_tid(0);
    ptbPlayer->Set_Dragon_status(EN_DRAGON_STATUS_NORMAL);

    ostringstream oss;
    TCHAR szUIntToString[MAX_TABLE_NAME_LEN];
    //龙名字
    oss.str("");
    oss << CGameInfo::GetInstance()->m_oJsonRoot["game_init_data"]["r"]["a2"].asString().c_str();
    if (ptbPlayer->m_nUid)
    {
        CCommonFunc::UIntToString(ptbPlayer->m_nUid, szUIntToString);
        oss << szUIntToString;
    }
    ptbPlayer->Set_Dragon_name(oss.str());

    //龙头像
    TUINT32 udwDragonUi = 0;
    TUINT32 udwDragonUiSize = CGameInfo::GetInstance()->m_oJsonRoot["game_init_data"]["r"]["a4"].size();
    if (udwDragonUiSize != 0)
    {
        udwDragonUi = rand() % udwDragonUiSize;
    }
    ptbPlayer->Set_Dragon_avatar(udwDragonUi);

    return 0;
}

TBOOL CPlayerBase::CheckDeadPlayerBaseCondForClear( SUserInfo *pstUser )
{
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();

    if(ptbPlayer->m_nDead_flag > 0)
    {
        return FALSE;
    }

    //if((ptbPlayer->m_nStatus & EN_CITY_STATUS__NEW_PROTECTION) || (ptbPlayer->m_nStatus & EN_CITY_STATUS__AVOID_WAR))
    if(ptbPlayer->m_nStatus & EN_CITY_STATUS__AVOID_WAR)
    {
        return FALSE;
    }

    TUINT32 udwCastleLevel = CCityBase::GetBuildingLevelByFuncType(pstCity, EN_BUILDING_TYPE__CASTLE);
    TBOOL bDeadTime = FALSE;

    if(udwCastleLevel <= 2)
    {
        if(udwCurTime - ptbPlayer->m_nUtime > 8 * 3600)
        {
            bDeadTime = TRUE;
        }
    }
    else if(udwCastleLevel <= 4)
    {
        if(udwCurTime - ptbPlayer->m_nUtime > 24 * 3600)
        {
            bDeadTime = TRUE;
        }
    }
    else if(udwCastleLevel <= 7)
    {
        if(udwCurTime - ptbPlayer->m_nUtime > 36 * 3600)
        {
            bDeadTime = TRUE;
        }
    }
    else if(udwCastleLevel <= 13)
    {
        if(udwCurTime - ptbPlayer->m_nUtime > 96 * 3600)
        {
            bDeadTime = TRUE;
        }
    }
    else
    {
        if(udwCurTime - ptbPlayer->m_nUtime > 192 * 3600)
        {
            bDeadTime = TRUE;
        }
    }
    if(bDeadTime == FALSE)
    {
        return FALSE;
    }

    return TRUE;
}

TUINT32 CPlayerBase::GetAllianceId( TbPlayer *ptbPlayer )
{
    TUINT32 udwAlId = 0;
    if(ptbPlayer->m_nUid && ptbPlayer->m_nAlpos && ptbPlayer->m_nAlid)
    {
        udwAlId = ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET;
    }
    return udwAlId;
}
