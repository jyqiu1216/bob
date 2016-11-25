#include "common_handle_after.h"
#include "time_utils.h"
#include "production_system.h"
#include "common_base.h"
#include "tool_base.h"
#include "common_json.h"
#include "action_base.h"
#include "buffer_base.h"
#include "quest_logic.h"
#include "bounty_logic.h"
#include "global_serv.h"
#include "msg_base.h"

TINT32 CAuCommonAfter::AuCommonHandleAfter(SSession *pstSession, SUserInfo* pstUser)
{
    TbPlayer* ptbPlayer = &pstUser->m_tbPlayer;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    if(NULL == pstCity || pstCity->m_stTblData.m_nUid == 0 || pstCity->m_stTblData.m_nPos == 0 || ptbPlayer->m_nUid == 0)
    {
        return 0;
    }

    if (pstSession->m_udwReqSvrId == ptbPlayer->m_nSid)
    {
        TUINT32 udwNum = 0;
        if (ptbPlayer->m_nCur_troop_might != static_cast<TINT64>(CCommonBase::ComputeTotalTroopMight(pstUser, udwNum)))
        {
            ptbPlayer->Set_Cur_troop_might(CCommonBase::ComputeTotalTroopMight(pstUser, udwNum));
        }
        if (ptbPlayer->m_nCur_fort_might != static_cast<TINT64>(CCommonBase::ComputeFortMight(pstUser, udwNum)))
        {
            ptbPlayer->Set_Cur_fort_might(CCommonBase::ComputeFortMight(pstUser, udwNum));
        }
        if (ptbPlayer->m_nBuilding_force != CBufferBase::ComputeBuildingForce(pstCity, pstUser))
        {
            ptbPlayer->Set_Building_force(CBufferBase::ComputeBuildingForce(pstCity, pstUser));
        }
        if (ptbPlayer->m_nResearch_force != CBufferBase::ComputeResearchForce(pstCity, pstUser))
        {
            ptbPlayer->Set_Research_force(CBufferBase::ComputeResearchForce(pstCity, pstUser));
        }
        if (ptbPlayer->m_nDragon_force != CBufferBase::ComputeDragonLvForce(pstCity, pstUser))
        {
            ptbPlayer->Set_Dragon_force(CBufferBase::ComputeDragonLvForce(pstCity, pstUser));
        }
        if (ptbPlayer->m_nLord_force != CBufferBase::ComputeLordLvForce(pstCity, pstUser))
        {
            ptbPlayer->Set_Lord_force(CBufferBase::ComputeLordLvForce(pstCity, pstUser));
        }
    }

    CBufferBase::ComputeBuffInfo(pstCity, pstUser, pstSession->m_atbIdol, pstSession->m_udwIdolNum, &pstSession->m_tbThrone, &pstSession->m_stTitleInfoList);
    CAuCommonAfter::DelEmptytBuildingPos(pstSession, pstUser);
    CAuCommonAfter::UpdateCityInfo(pstSession, pstUser);
    CAuCommonAfter::UpdatePlayerInfo(pstSession, pstUser);
    CAuCommonAfter::UpdateUserMight(pstSession, pstUser);
    CAuCommonAfter::UpdateUserInfoToMap(pstSession, pstUser);
    CAuCommonAfter::SyncInfoToMarch(pstSession, pstUser);
    CAuCommonAfter::UpdateNotiTimer(pstSession, pstUser);
    CAuCommonAfter::DelNotiTimer(pstSession, pstUser);


    pstUser->m_bIsSendEventReq = TRUE;
    /*
    TINT32 dwAlGiftLv = 0;
    TINT32 dwMaxAlGemBuy = 0;
    if (ptbPlayer->m_nAlid && ptbPlayer->m_nAlpos)
    {
        dwAlGiftLv = CCommonBase::GetAlGiftLevel(&pstUser->m_tbAlliance);
        dwMaxAlGemBuy = pstUser->m_tbAlliance.m_nMax_gem_buy;

        if (0 == dwMaxAlGemBuy)
        {
            if (13 <= dwAlGiftLv)
            {
                dwMaxAlGemBuy = 33000;
            }
            else if (10 <= dwAlGiftLv)
            {
                dwMaxAlGemBuy = 13000;
            }
            else if (7 <= dwAlGiftLv)
            {
                dwMaxAlGemBuy = 5000;
            }
        }
    }

    if (0 == CCommonBase::GetUserDna(ptbPlayer->m_nUid, CCityBase::GetBuildingLevelById(&pstCity->m_stTblData, 3), 
        pstUser->m_tbLogin.m_nGem_buy, dwAlGiftLv, dwMaxAlGemBuy, pstSession->m_udwReqSvrId, &pstUser->m_stUserEventGoals))
    {
        pstUser->m_bIsSendEventReq = TRUE;
    }
    */

    CBountyLogic::CheckBountyNotic(pstUser, pstCity);

    UpdateLastMight(pstUser, pstSession->m_poServLog);

    CheckPeaceTime(pstSession, pstUser);

    if (pstUser->m_tbPlayer.m_nAlid != 0
        && pstUser->m_tbPlayer.m_nAlpos > EN_ALLIANCE_POS__REQUEST)
    {
        if (pstUser->m_tbPlayer.m_mFlag.size() > 0)
        {
            Json::Value tmpProfile;
            CCommJson::GenAlMemberInfo(&pstUser->m_tbPlayer, tmpProfile);
            if (tmpProfile != pstUser->m_tbSelfAlmember.m_jProfile)
            {
                pstUser->m_tbSelfAlmember.Set_Aid(pstUser->m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET);
                pstUser->m_tbSelfAlmember.Set_Uid(pstUser->m_tbPlayer.m_nUid);
                pstUser->m_tbSelfAlmember.Set_Al_pos(pstUser->m_tbPlayer.m_nAlpos);
                pstUser->m_tbSelfAlmember.Set_Profile(tmpProfile);
                //pstUser->m_tbSelfAlmember.Set_Profile_update_time(CTimeUtils::GetUnixTime()); //只需要更新profile 保证联盟force值的正确性
            }
        }
    }

    if (pstUser->m_tbPlayer.m_nAlpos == EN_ALLIANCE_POS__CHANCELLOR
        && pstUser->m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET == pstUser->m_tbAlliance.m_nAid)
    {
        if (pstUser->m_tbPlayer.m_nCid != pstUser->m_tbAlliance.m_nOwner_cid)
        {
            pstUser->m_tbAlliance.Set_Owner_cid(pstUser->m_tbPlayer.m_nCid);
        }
    }

    return 0;
}

TINT32 CAuCommonAfter::UpdateCityInfo(SSession *pstSession, SUserInfo* pstUser)
{
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    CProductionSystem::ComputeProductionSystem(pstUser, &pstUser->m_stCityInfo, udwCurTime);
    return 0;
}

TINT32 CAuCommonAfter::UpdatePlayerInfo(SSession *pstSession, SUserInfo* pstUser)
{
    SCityInfo* pstCity = &pstUser->m_stCityInfo;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;
    
    SCommonTroop stReinforceTroop;
    CCommonBase::GetReinforceTroop(pstUser, pstCity, stReinforceTroop);

    ptbPlayer->m_bReinforce_limit[0].udwMarchNum = CCommonBase::GetReinforcedMarchNum(pstUser, pstCity);
    ptbPlayer->m_bReinforce_limit[0].udwMarchLimit = CCommonBase::GetReinforceMarchLimit(pstUser);
    ptbPlayer->m_bReinforce_limit[0].udwTroopNum = CToolBase::GetTroopSumNum(stReinforceTroop);
    ptbPlayer->m_bReinforce_limit[0].udwTroopLimit = CCommonBase::GetReinforceTroopLimit(pstUser);
    ptbPlayer->m_bReinforce_limit[0].ddwTroopForce = CToolBase::GetTroopSumForce(stReinforceTroop);

    ptbPlayer->SetFlag(TbPLAYER_FIELD_REINFORCE_LIMIT);

    return 0;
}

TINT32 CAuCommonAfter::UpdateUserMight(SSession *pstSession, SUserInfo* pstUser)
{
    // 计算用户的might值
    CCommonBase::ComputeUserMight(pstUser);

    if(pstUser->m_tbPlayer.m_nMkill != pstUser->m_tbPlayer.m_nKfort + pstUser->m_tbPlayer.m_nKtroop)
    {
        pstUser->m_tbPlayer.Set_Mkill(pstUser->m_tbPlayer.m_nKfort + pstUser->m_tbPlayer.m_nKtroop);
    }

    return 0;
}

TINT32 CAuCommonAfter::UpdateUserInfoToMap(SSession *pstSession, SUserInfo* pstUser)
{
    TbPlayer* ptbPlayer = &pstUser->m_tbPlayer;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    if(NULL == pstCity || pstCity->m_stTblData.m_nUid == 0 || pstCity->m_stTblData.m_nPos == 0)
    {
        return 0;
    }

    Json::FastWriter writer;
    Json::Value jsonCityInfo;
    jsonCityInfo.clear();
    jsonCityInfo = Json::Value(Json::objectValue);
    //"city_info":
    //{
    //    "uid": long,
    //    "alnick" : string,
    //    "uname" : string,
    //    "hero" : [string, int, int], //英雄名字,等级,图标  //等级为0表示没有英雄
    //    "troop_num" : long,
    //    "troop_force" : long,
    //    "troop" : [long.....] //下标为troop id，值表示具体数量
    //}
    jsonCityInfo["uid"] = ptbPlayer->m_nUid;
    jsonCityInfo["alnick"] = ptbPlayer->m_sAl_nick_name;
    jsonCityInfo["uname"] = ptbPlayer->m_sUin;
    jsonCityInfo["dragon"] = Json::Value(Json::arrayValue);
    jsonCityInfo["dragon"].append(ptbPlayer->m_sDragon_name);
    if(ptbPlayer->m_nDragon_status == EN_DRAGON_STATUS_NORMAL)
    {
        jsonCityInfo["dragon"].append(ptbPlayer->m_nDragon_level);
    }
    else
    {
        jsonCityInfo["dragon"].append(0);
    }
    jsonCityInfo["dragon"].append(ptbPlayer->m_nDragon_avatar);

    SCommonTroop stTroopInCity;
    CCommonBase::GetInCityTroop(pstUser, pstCity, stTroopInCity);

    jsonCityInfo["troop_num"] = CToolBase::GetTroopSumNum(stTroopInCity);
    jsonCityInfo["troop_force"] = CToolBase::GetTroopSumForce(stTroopInCity);
    CCommJson::GenTroopJson(&stTroopInCity, jsonCityInfo["troop"]);

    jsonCityInfo["reinforced_num"] = ptbPlayer->m_bReinforce_limit[0].udwMarchNum;
    jsonCityInfo["reinforced_limit"] = ptbPlayer->m_bReinforce_limit[0].udwMarchLimit;
    jsonCityInfo["reinforced_troop_num"] = ptbPlayer->m_bReinforce_limit[0].udwTroopNum;
    jsonCityInfo["reinforced_troop_limit"] = ptbPlayer->m_bReinforce_limit[0].udwTroopLimit;
    jsonCityInfo["reinforced_troop_force"] = ptbPlayer->m_bReinforce_limit[0].ddwTroopForce;

    jsonCityInfo["knight"] = Json::Value(Json::arrayValue);
    jsonCityInfo["knight"].append(-1);
    jsonCityInfo["knight"].append(0);
    for (TUINT32 udwIdx = 0; udwIdx < pstCity->m_stTblData.m_bKnight.m_udwNum; udwIdx++)
    {
        if (pstCity->m_stTblData.m_bKnight[udwIdx].ddwPos == EN_KNIGHT_POS__TROOP)
        {
            jsonCityInfo["knight"][0U] = udwIdx;
            jsonCityInfo["knight"][1U] = CGameInfo::GetInstance()->ComputeKnightLevelByExp(pstCity->m_stTblData.m_bKnight[udwIdx].ddwExp);
            break;
        }
    }

    TBOOL bPrisonFlag = FALSE;
    if (pstUser->m_tbPlayer.m_nStatus & EN_CITY_STATUS__AVOID_WAR
        || pstUser->m_tbPlayer.m_nStatus & EN_CITY_STATUS__NEW_PROTECTION)
    {
        // do nothing
    }
    else
    {
        for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwPassiveMarchNum; ++udwIdx)
        {
            if (pstUser->m_aucPassiveMarchFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
            {
                continue;
            }

            if (pstUser->m_atbPassiveMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__PRISON_TIMER
                && pstUser->m_atbPassiveMarch[udwIdx].m_nStatus == EN_MARCH_STATUS__DEFENDING)
            {
                bPrisonFlag = TRUE;
                break;
            }
        }
    }

    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwWildNum; ++udwIdx)
    {
        TbMap* ptbMap = &pstUser->m_atbWild[udwIdx];

        if(EN_WILD_TYPE__CITY != ptbMap->m_nType)
        {
            continue;
        }
        if(ptbMap->m_nUid != pstUser->m_tbPlayer.m_nUid)
        {
            continue;
        }

        if(ptbMap->m_nPrison_flag != bPrisonFlag)
        {
            ptbMap->Set_Prison_flag(bPrisonFlag);
        }
        if (writer.write(ptbMap->m_jCity_info) != writer.write(jsonCityInfo)
            && !jsonCityInfo.empty())
        {
            ptbMap->Set_City_info(jsonCityInfo);
        }

        UpdateMapCityInfo(pstUser, ptbMap);
    }

    //wave@20161124: 更新au的目标地块信息
    if(pstSession->m_stMapItem.m_nUid == ptbPlayer->m_nUid && pstSession->m_stMapItem.m_nType == EN_WILD_TYPE__CITY)
    {
        TbMap* ptbMap = &pstSession->m_stMapItem;
        if(ptbMap->m_nPrison_flag != bPrisonFlag)
        {
            ptbMap->Set_Prison_flag(bPrisonFlag);
        }
        if (writer.write(ptbMap->m_jCity_info) != writer.write(jsonCityInfo)
            && !jsonCityInfo.empty())
        {
            ptbMap->Set_City_info(jsonCityInfo);
        }
        UpdateMapCityInfo(pstUser, ptbMap);
    }

    if(ptbPlayer->m_nCid != pstCity->m_stTblData.m_nPos)
    {
        ptbPlayer->Set_Cid(pstCity->m_stTblData.m_nPos);
    }

    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwWildNum; ++udwIdx)
    {
        TbMap* ptbMap = &pstUser->m_atbWild[udwIdx];
        if (ptbMap->m_nUid != pstUser->m_tbPlayer.m_nUid)
        {
            continue;
        }

        if (EN_WILD_TYPE__CITY == ptbMap->m_nType)
        {
            continue;
        }

        if (ptbMap->m_sUname != ptbPlayer->m_sUin)
        {
            ptbMap->Set_Uname(ptbPlayer->m_sUin);
            ptbMap->Set_Name_update_time(CTimeUtils::GetUnixTime());
        }

        if (ptbMap->m_nAvatar != ptbPlayer->m_nAvatar)
        {
            ptbMap->Set_Avatar(ptbPlayer->m_nAvatar);
        }

        if (ptbMap->m_sAlname != ptbPlayer->m_sAlname)
        {
            if (ptbPlayer->m_nAlid != 0 && ptbPlayer->m_nAlpos != EN_ALLIANCE_POS__REQUEST)
            {
                ptbMap->Set_Alname(ptbPlayer->m_sAlname);
            }
            else
            {
                ptbMap->Set_Alname("");
            }
            ptbMap->Set_Name_update_time(CTimeUtils::GetUnixTime());
        }

        if (ptbMap->m_sAl_nick != pstUser->m_tbAlliance.m_sAl_nick_name)
        {
            if (ptbPlayer->m_nAlid != 0 && ptbPlayer->m_nAlpos != EN_ALLIANCE_POS__REQUEST)
            {
                ptbMap->Set_Al_nick(pstUser->m_tbAlliance.m_sAl_nick_name);
            }
            else
            {
                ptbMap->Set_Al_nick("");
            }
            ptbMap->Set_Name_update_time(CTimeUtils::GetUnixTime());
        }

        if (ptbMap->m_nAl_flag != pstUser->m_tbAlliance.m_nAvatar)
        {
            if (ptbPlayer->m_nAlid != 0 && ptbPlayer->m_nAlpos != EN_ALLIANCE_POS__REQUEST)
            {
                ptbMap->Set_Al_flag(pstUser->m_tbAlliance.m_nAvatar);
            }
            else
            {
                ptbMap->Set_Al_flag(0);
            }
        }
        if (ptbMap->m_nAl_pos != pstUser->m_tbPlayer.m_nAlpos)
        {
            if (ptbPlayer->m_nAlid != 0 && ptbPlayer->m_nAlpos != EN_ALLIANCE_POS__REQUEST)
            {
                ptbMap->Set_Al_pos(pstUser->m_tbPlayer.m_nAlpos);
            }
            else
            {
                ptbMap->Set_Al_pos(0);
            }
        }
        if (ptbMap->m_nAlid != ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET)
        {
            if (ptbPlayer->m_nAlid != 0 && ptbPlayer->m_nAlpos != EN_ALLIANCE_POS__REQUEST)
            {
                ptbMap->Set_Alid(ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET);
            }
            else if (ptbMap->m_nAlid != 0)
            {
                ptbMap->Set_Alid(0);
            }
        }
        if (ptbMap->m_nMight != ptbPlayer->m_nMight)
        {
            ptbMap->Set_Might(ptbPlayer->m_nMight);
        }
    }

    return 0;
}

TINT32 CAuCommonAfter::SyncInfoToMarch(SSession *pstSession, SUserInfo* pstUser)
{
    TbPlayer* ptbPlayer = &pstUser->m_tbPlayer;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    if(NULL == pstCity || pstCity->m_stTblData.m_nUid == 0 || pstCity->m_stTblData.m_nPos == 0 || ptbPlayer->m_nUid == 0)
    {
        return 0;
    }

    //TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    TINT64 ddwCityPos = pstCity->m_stTblData.m_nPos;

    Json::FastWriter writer;
    writer.omitEndingLineFeed();

    Json::Value jsonCityInfo;
    jsonCityInfo.clear();
    jsonCityInfo = Json::Value(Json::objectValue);
    //"city_info":
    //{
    //    "uid": long,
    //    "alnick" : string,
    //    "uname" : string,
    //    "hero" : [string, int, int], //英雄名字,等级,图标  //等级为0表示没有英雄
    //    "troop_num" : long,
    //    "troop_force" : long,
    //    "troop" : [long.....] //下标为troop id，值表示具体数量
    //}
    jsonCityInfo["uid"] = ptbPlayer->m_nUid;
    jsonCityInfo["alnick"] = ptbPlayer->m_sAl_nick_name;
    jsonCityInfo["uname"] = ptbPlayer->m_sUin;
    jsonCityInfo["dragon"] = Json::Value(Json::arrayValue);
    jsonCityInfo["dragon"].append(ptbPlayer->m_sDragon_name);
    if (ptbPlayer->m_nDragon_status == EN_DRAGON_STATUS_NORMAL)
    {
        jsonCityInfo["dragon"].append(ptbPlayer->m_nDragon_level);
    }
    else
    {
        jsonCityInfo["dragon"].append(0);
    }
    jsonCityInfo["dragon"].append(ptbPlayer->m_nDragon_avatar);

    SCommonTroop stTroopInCity;
    CCommonBase::GetInCityTroop(pstUser, pstCity, stTroopInCity);

    jsonCityInfo["troop_num"] = CToolBase::GetTroopSumNum(stTroopInCity);
    jsonCityInfo["troop_force"] = CToolBase::GetTroopSumForce(stTroopInCity);
    CCommJson::GenTroopJson(&stTroopInCity, jsonCityInfo["troop"]);

    jsonCityInfo["reinforced_num"] = ptbPlayer->m_bReinforce_limit[0].udwMarchNum;
    jsonCityInfo["reinforced_limit"] = ptbPlayer->m_bReinforce_limit[0].udwMarchLimit;
    jsonCityInfo["reinforced_troop_num"] = ptbPlayer->m_bReinforce_limit[0].udwTroopNum;
    jsonCityInfo["reinforced_troop_limit"] = ptbPlayer->m_bReinforce_limit[0].udwTroopLimit;
    jsonCityInfo["reinforced_troop_force"] = ptbPlayer->m_bReinforce_limit[0].ddwTroopForce;

    jsonCityInfo["knight"] = Json::Value(Json::arrayValue);
    jsonCityInfo["knight"].append(-1);
    jsonCityInfo["knight"].append(0);
    for (TUINT32 udwIdx = 0; udwIdx < pstCity->m_stTblData.m_bKnight.m_udwNum;udwIdx++)
    {
        if (pstCity->m_stTblData.m_bKnight[udwIdx].ddwPos == EN_KNIGHT_POS__TROOP)
        {
            jsonCityInfo["knight"][0U] = udwIdx;
            jsonCityInfo["knight"][1U] = CGameInfo::GetInstance()->ComputeKnightLevelByExp(pstCity->m_stTblData.m_bKnight[udwIdx].ddwExp);
            break;
        }
    }

    //被别人攻打的rallywar
    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwPassiveMarchNum; ++udwIdx)
    {
        if(pstUser->m_aucPassiveMarchFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }
        TbMarch_action* ptbDefendRallyWar = &pstUser->m_atbPassiveMarch[udwIdx];
        if(ptbDefendRallyWar->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR
            && ptbDefendRallyWar->m_nTuid == pstUser->m_tbPlayer.m_nUid
            && ptbDefendRallyWar->m_nTpos == ddwCityPos)
        {
            if (writer.write(ptbDefendRallyWar->m_jCity_info) != writer.write(jsonCityInfo))
            {
                ptbDefendRallyWar->Set_City_info(jsonCityInfo);

                pstUser->m_aucPassiveMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
            }
            if(pstUser->m_tbPlayer.m_nAlid > 0 && pstUser->m_tbPlayer.m_nAlpos > EN_ALLIANCE_POS__REQUEST)
            {
                if(ptbDefendRallyWar->m_nTal != (pstUser->m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET))
                {
                    ptbDefendRallyWar->Set_Tal(pstUser->m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET);
                    ptbDefendRallyWar->m_bParam[0].m_ddwTargetAlliance = pstUser->m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET;
                    ptbDefendRallyWar->SetFlag(TbMARCH_ACTION_FIELD_PARAM);
                    pstUser->m_aucPassiveMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
                }
            }
        }
    }

    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
    {
        if(pstUser->m_aucMarchFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }

        TbMarch_action* ptbMarch = &pstUser->m_atbMarch[udwIdx];
        if(!CActionBase::IsPlayerOwnedAction(pstUser->m_tbPlayer.m_nUid, ptbMarch->m_nId))
        {
            continue;
        }

        //别人警戒塔里看到的信息
        if(ptbMarch->m_nStatus == EN_MARCH_STATUS__MARCHING)
        {
            if(ptbMarch->m_bParam[0].m_stDragon.m_ddwLevel > 0)
            {
                if (ptbMarch->m_bParam[0].m_stDragon.m_ddwLevel != pstUser->m_tbPlayer.m_nDragon_level)
                {
                    ptbMarch->m_bParam[0].m_stDragon.m_ddwLevel = pstUser->m_tbPlayer.m_nDragon_level;
                    ptbMarch->SetFlag(TbMARCH_ACTION_FIELD_PARAM);
                }

                if(ptbMarch->m_bParam[0].m_stDragon.m_ddwIconId != pstUser->m_tbPlayer.m_nDragon_avatar)
                {
                    ptbMarch->m_bParam[0].m_stDragon.m_ddwIconId = pstUser->m_tbPlayer.m_nDragon_avatar;
                    ptbMarch->SetFlag(TbMARCH_ACTION_FIELD_PARAM);
                }

                if(strcmp(ptbMarch->m_bParam[0].m_stDragon.m_szName, pstUser->m_tbPlayer.m_sDragon_name.c_str()) != 0)
                {
                    strncpy(ptbMarch->m_bParam[0].m_stDragon.m_szName, pstUser->m_tbPlayer.m_sDragon_name.c_str(), MAX_TABLE_NAME_LEN);
                    ptbMarch->m_bParam[0].m_stDragon.m_szName[MAX_TABLE_NAME_LEN] = '\0';
                    ptbMarch->SetFlag(TbMARCH_ACTION_FIELD_PARAM);
                }
            }

            if(ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__ATTACK
                || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR)
            {
                SPlayerBuffInfo stNowBuff;
                if(ptbMarch->m_bParam[0].m_stDragon.m_ddwLevel > 0)
                {
                    CBufferBase::GenBattleBuff(&pstUser->m_stPlayerBuffList, &stNowBuff, ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR, 
                        FALSE, ptbMarch->m_bParam[0].m_stKnight.ddwLevel);
                }
                else
                {
                    CBufferBase::GenBattleBuff(&pstUser->m_stBuffWithoutDragon, &stNowBuff, ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR, 
                        FALSE, ptbMarch->m_bParam[0].m_stKnight.ddwLevel);
                }
                SPlayerBuffInfo stOldBuff;
                CBufferBase::MarchBuffToPlayerBuff(ptbMarch, &stOldBuff);
                if(!CBufferBase::IsBuffSame(stNowBuff, stOldBuff))
                {
                    CBufferBase::SetMarchBuff(&stNowBuff, ptbMarch);
                }
            }
        }

        //别人监狱里看到的信息
        if(ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__PRISON_TIMER)
        {
            if(strcmp(ptbMarch->m_bPrison_param[0].szSourceUserName, pstUser->m_tbPlayer.m_sUin.c_str()) != 0)
            {
                strncpy(ptbMarch->m_bPrison_param[0].szSourceUserName, pstUser->m_tbPlayer.m_sUin.c_str(), MAX_TABLE_NAME_LEN);
                ptbMarch->m_bPrison_param[0].szSourceUserName[MAX_TABLE_NAME_LEN] = '\0';
                ptbMarch->SetFlag(TbMARCH_ACTION_FIELD_PRISON_PARAM);
            }
        }

        if(ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_NORMAL
            || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_REINFORCE)
        {
            if(ptbMarch->m_nSavatar != pstUser->m_tbPlayer.m_nAvatar)
            {
                ptbMarch->Set_Savatar(pstUser->m_tbPlayer.m_nAvatar);
            }

            if(strcmp(ptbMarch->m_bParam[0].m_szSourceUserName, pstUser->m_tbPlayer.m_sUin.c_str()) != 0)
            {
                strncpy(ptbMarch->m_bParam[0].m_szSourceUserName, pstUser->m_tbPlayer.m_sUin.c_str(), MAX_TABLE_NAME_LEN);
                ptbMarch->m_bParam[0].m_szSourceUserName[MAX_TABLE_NAME_LEN - 1] = '\0';
                ptbMarch->SetFlag(TbMARCH_ACTION_FIELD_PARAM);
            }
        }
    }

    return 0;
}

TINT32 CAuCommonAfter::DelEmptytBuildingPos(SSession* pstSession, SUserInfo* pstUser)
{
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    if(NULL == pstCity || pstCity->m_stTblData.m_nUid == 0 || pstCity->m_stTblData.m_nPos == 0)
    {
        return 0;
    }

    for(TUINT32 udwIdx = 0; udwIdx < pstCity->m_stTblData.m_bBuilding.m_udwNum;)
    {
        SCityBuildingNode& pstNode = pstCity->m_stTblData.m_bBuilding[udwIdx];
        if(pstNode.m_ddwLevel == 0)
        {
            pstNode = pstCity->m_stTblData.m_bBuilding[pstCity->m_stTblData.m_bBuilding.m_udwNum - 1];
            pstCity->m_stTblData.m_bBuilding.m_udwNum--;
            pstCity->m_stTblData.SetFlag(TbCITY_FIELD_BUILDING);
            continue;
        }
        ++udwIdx;
    }
    return 0;
}

TINT32 CAuCommonAfter::UpdateNotiTimer(SSession* pstSession, SUserInfo* pstUser)
{
    TINT32 dwCurTime = CTimeUtils::GetUnixTime();

    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    if(NULL == pstCity || pstCity->m_stTblData.m_nUid == 0 || pstCity->m_stTblData.m_nPos == 0)
    {
        return 0;
    }
    TINT64 ddwCityPos = pstCity->m_stTblData.m_nPos;

    TBOOL bNeedTimer = FALSE;
    TINT32 dwTickTimeStamp = 0;

    TbMarch_action* ptbTimer = NULL;
    TINT32 dwTimerIndex = -1;
    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
    {
        if(pstUser->m_aucMarchFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }
        ptbTimer = &pstUser->m_atbMarch[udwIdx];

        if(!CActionBase::IsPlayerOwnedAction(pstUser->m_tbPlayer.m_nUid, ptbTimer->m_nId))
        {
            continue;
        }

        if(ptbTimer->m_nSclass != EN_ACTION_SEC_CLASS__NOTI_TIMER)
        {
            continue;
        }

        dwTimerIndex = udwIdx;
        break;
    }

    //building research 相关
    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwSelfAlActionNum; ++udwIdx)
    {
        if(pstUser->m_aucSelfAlActionFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }
        TbAlliance_action* ptbCanFreeAction = &pstUser->m_atbSelfAlAction[udwIdx];

        if(!CActionBase::IsPlayerOwnedAction(pstUser->m_tbPlayer.m_nUid, ptbCanFreeAction->m_nId))
        {
            continue;
        }

        if(ptbCanFreeAction->m_nNoti_flag == TRUE)
        {
            continue;
        }

        if(ptbCanFreeAction->m_nSclass != EN_ACTION_SEC_CLASS__BUILDING_UPGRADE
        && ptbCanFreeAction->m_nSclass != EN_ACTION_SEC_CLASS__BUILDING_REMOVE
        && ptbCanFreeAction->m_nSclass != EN_ACTION_SEC_CLASS__RESEARCH_UPGRADE
        && ptbCanFreeAction->m_nSclass != EN_ACTION_SEC_CLASS__EQUIP_UPGRADE)
        {
            continue;
        }

        TINT32 dwCanFreeTimeStamp = ptbCanFreeAction->m_nEtime -
            CCommonBase::GetGameBasicVal(EN_GAME_BASIC_FREE_INSTANT_TIME) -
            pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_FREE_INSTANT_FINISH_TIME].m_ddwBuffTotal;
        if (dwTickTimeStamp == 0 || dwTickTimeStamp > dwCanFreeTimeStamp)
        {
            dwTickTimeStamp = dwCanFreeTimeStamp;
            bNeedTimer = TRUE;
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("NotiTimer[timestamp=%d][seq=%u]", dwTickTimeStamp, pstSession->m_udwSeqNo));
        }
    }

    //buff time 相关
    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwActionNum; ++udwIdx)
    {
        if(pstUser->m_aucActionFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }
        TbAction* ptbPeacetime = &pstUser->m_atbAction[udwIdx];

        if(ptbPeacetime->m_nSclass != EN_ACTION_SEC_CLASS__ITEM)
        {
            continue;
        }

        if(ptbPeacetime->m_nNoti_flag == TRUE)
        {
            continue;
        }

        TUINT32 udwNotiId = EN_NOTI_ID__END;
        switch (ptbPeacetime->m_bParam[0].m_stItem.m_ddwBufferId)
        {
        case EN_BUFFER_INFO_PEACE_TIME:
            udwNotiId = EN_NOTI_ID__PEACETIME_END;
            if (pstUser->m_tbPlayer.m_nStatus & EN_CITY_STATUS__NEW_PROTECTION)
            {
                udwNotiId = EN_NOTI_ID__NEW_PLAYER_END;
            }
            break;
        case EN_BUFFER_INFO_VIP_ACTIVATE:
            udwNotiId = EN_NOTI_ID__VIP_EXPIRED;
            break;
        case EN_BUFFER_INFO_ALL_TROOP_ATTACK:
        case EN_BUFFER_INFO_ALL_TROOP_LIFE:
        case EN_BUFFER_INFO_GOLD_PRODUCTION:
        case EN_BUFFER_INFO_FOOD_PRODUCTION:
        case EN_BUFFER_INFO_WOOD_PRODUCTION:
        case EN_BUFFER_INFO_STONE_PROTECTION:
        case EN_BUFFER_INFO_ORE_PROTECTION:
        case EN_BUFFER_INFO_QUEUE_NUM:
        case EN_BUFFER_INFO_TROOP_SIZE:
            udwNotiId = EN_NOTI_ID__BUFF_EXPIRED;
            break;
        default:
            break;
        }

        if (udwNotiId == EN_NOTI_ID__END)
        {
            continue;
        }

        TINT32 dwNeedNotiTimeStamp = ptbPeacetime->m_nEtime - 3600;
        if(dwTickTimeStamp == 0 || dwTickTimeStamp > dwNeedNotiTimeStamp)
        {
            dwTickTimeStamp = dwNeedNotiTimeStamp;
            bNeedTimer = TRUE;
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("NotiTimer[timestamp=%d][seq=%u]", dwTickTimeStamp, pstSession->m_udwSeqNo));
        }
    }

    /*
    //top quest
    TBOOL IsTopQuestClaim = FALSE;
    if(dwTimerIndex == -1 || (dwTimerIndex >= 0 && 0 == BITTEST(ptbTimer->m_bNotic_task_flag[0].m_bitFlag, EN_TOP_QUEST)))
    {
        CQuestLogic::CheckTopQuestCanClaim(pstUser, IsTopQuestClaim);
        if(TRUE == IsTopQuestClaim)
        {
            TINT32 dwNeedNotiTimeStamp = dwCurTime;
            if(dwTickTimeStamp == 0 || dwTickTimeStamp > dwNeedNotiTimeStamp)
            {
                dwTickTimeStamp = dwNeedNotiTimeStamp;
                bNeedTimer = TRUE;
            }
        }
    }
    */

    TUINT32 udwQuestFinishTime = 0;
    TBOOL IsQuestRunning = FALSE;

    //daily quest 
    if(dwTimerIndex == -1 || (dwTimerIndex >= 0 && 0 == BITTEST(ptbTimer->m_bNotic_task_flag[0].m_bitFlag, EN_DAILY_QUEST)))
    {
        udwQuestFinishTime = CQuestLogic::CheckQuestNodeRunning(&pstUser->m_tbQuest.m_bDaily_quest[0], IsQuestRunning);
        if(TRUE == IsQuestRunning)
        {
            TINT32 dwNeedNotiTimeStamp = udwQuestFinishTime;
            if(dwTickTimeStamp == 0 || dwTickTimeStamp > dwNeedNotiTimeStamp)
            {
                dwTickTimeStamp = dwNeedNotiTimeStamp;
                bNeedTimer = TRUE;
            }
        }
    }

    //alliance quest 
    if(dwTimerIndex == -1 || (dwTimerIndex >= 0 && 0 == BITTEST(ptbTimer->m_bNotic_task_flag[0].m_bitFlag, EN_ALLIANCE_QUEST)))
    {
        udwQuestFinishTime = CQuestLogic::CheckQuestNodeRunning(&pstUser->m_tbQuest.m_bAl_quest[0], IsQuestRunning);
        if(TRUE == IsQuestRunning)
        {
            TINT32 dwNeedNotiTimeStamp = udwQuestFinishTime;
            if(dwTickTimeStamp == 0 || dwTickTimeStamp > dwNeedNotiTimeStamp)
            {
                dwTickTimeStamp = dwNeedNotiTimeStamp;
                bNeedTimer = TRUE;
            }
        }
    }

    //vip quest 
    if(dwTimerIndex == -1 || (dwTimerIndex >= 0 && 0 == BITTEST(ptbTimer->m_bNotic_task_flag[0].m_bitFlag, EN_VIP_QUEST)))
    {
        udwQuestFinishTime = CQuestLogic::CheckQuestNodeRunning(&pstUser->m_tbQuest.m_bVip_quest[0], IsQuestRunning);
        if(TRUE == IsQuestRunning)
        {
            TINT32 dwNeedNotiTimeStamp = udwQuestFinishTime;
            if(dwTickTimeStamp == 0 || dwTickTimeStamp > dwNeedNotiTimeStamp)
            {
                dwTickTimeStamp = dwNeedNotiTimeStamp;
                bNeedTimer = TRUE;
            }
        }
    }

    //mistery
    if(dwTimerIndex == -1 || (dwTimerIndex >= 0 && 0 == BITTEST(ptbTimer->m_bNotic_task_flag[0].m_bitFlag, EN_MISTERY_QUEST)))
    {
        udwQuestFinishTime = CQuestLogic::CheckQuestNodeRunning(&pstUser->m_tbQuest.m_bTimer_gift[0], IsQuestRunning);
        if(TRUE == IsQuestRunning)
        {
            TINT32 dwNeedNotiTimeStamp = udwQuestFinishTime;
            if(dwTickTimeStamp == 0 || dwTickTimeStamp > dwNeedNotiTimeStamp)
            {
                dwTickTimeStamp = dwNeedNotiTimeStamp;
                bNeedTimer = TRUE;
            }
        }
    }

    // wave@20160428: dragon 体力恢复时间计算
    if (pstUser->m_tbPlayer.m_nDragon_recovery_time > dwCurTime)
    {
        TINT32 dwNeedNotiTimeStamp = pstUser->m_tbPlayer.m_nDragon_recovery_time;
        if (dwTickTimeStamp == 0 || dwTickTimeStamp > dwNeedNotiTimeStamp)
        {
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("UpdateNotiTimer:uid=%u dragon_recovery need noti [seq=%u]", pstUser->m_tbPlayer.m_nUid, pstUser->m_udwBSeqNo));
            dwTickTimeStamp = dwNeedNotiTimeStamp;
            bNeedTimer = TRUE;
        }
    }

    // wave@20160428：knight 自动unassign时间计算
    TINT32 dwKnightAutoUnassignTime = CProductionSystem::ComputeKnightUnassignTime(pstUser, &pstUser->m_stCityInfo);
    if (dwKnightAutoUnassignTime)
    {
        if (dwTickTimeStamp == 0 || dwTickTimeStamp > dwKnightAutoUnassignTime)
        {
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("UpdateNotiTimer:uid=%u ComputeKnightUnassignTime need noti [seq=%u]", pstUser->m_tbPlayer.m_nUid, pstUser->m_udwBSeqNo));
            dwTickTimeStamp = dwKnightAutoUnassignTime;
            bNeedTimer = TRUE;
        }
    }

    if (!bNeedTimer || dwTickTimeStamp < dwCurTime)
    {
        if(dwTimerIndex >= 0)
        {
            pstUser->m_aucMarchFlag[dwTimerIndex] = EN_TABLE_UPDT_FLAG__CHANGE;
            ptbTimer->Set_Etime(dwCurTime + 60 * 60 * 24 * 7 + 300);
        }
        return 0;
    }

    if (dwTimerIndex == -1)
    {
        ptbTimer = CActionBase::AddNewMarch(pstUser);
        ptbTimer->Set_Suid(pstUser->m_tbPlayer.m_nUid);
        ptbTimer->Set_Scid(ddwCityPos);
        ptbTimer->Set_Mclass(EN_ACTION_MAIN_CLASS__TIMER);
        ptbTimer->Set_Sclass(EN_ACTION_SEC_CLASS__NOTI_TIMER);
        ptbTimer->Set_Sid(pstUser->m_tbPlayer.m_nSid);
        ptbTimer->Set_Etime(dwTickTimeStamp + 1);
    }
    else if (ptbTimer->m_nEtime != dwTickTimeStamp + 1)
    {
        ptbTimer->Set_Etime(dwTickTimeStamp + 1);
    }
    

    return 0;
}

TINT32 CAuCommonAfter::DelNotiTimer(SSession* pstSession, SUserInfo* pstUser)
{
    if(pstUser->m_tbPlayer.m_nUid == 0)
    {
        return 0;
    }

    TUINT32 udwCount = 0;
    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
    {
        if(pstUser->m_aucMarchFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }
        TbMarch_action* ptbTimer = &pstUser->m_atbMarch[udwIdx];

        if(!CActionBase::IsPlayerOwnedAction(pstUser->m_tbPlayer.m_nUid, ptbTimer->m_nId))
        {
            continue;
        }

        if(ptbTimer->m_nSclass == EN_ACTION_SEC_CLASS__NOTI_TIMER)
        {
            udwCount++;
            if(udwCount > 1)
            {
                pstUser->m_aucMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__DEL;
            }
            continue;
        }
    }

    return 0;
}

TVOID CAuCommonAfter::UpdateLastMight(SUserInfo * pstUser, CTseLogger * poServLog)
{
    Json::FastWriter rWriter;
    rWriter.omitEndingLineFeed();

    Json::Value rTroopAndFort = Json::Value(Json::objectValue);
    TUINT64 uddwTotalMight = CCommonBase::GetTotalTroopAndFort(pstUser, rTroopAndFort);
    string sJson = rWriter.write(rTroopAndFort);
    
    pstUser->m_tbUserStat.Set_Last_might(uddwTotalMight);
    pstUser->m_tbUserStat.Set_Last_troop_fort(rTroopAndFort);

    TSE_LOG_DEBUG(poServLog, ("UpdateLastMight: Source: [uid=%ld][now_might=%lu][now_json=%s] [seq=%u]", 
        pstUser->m_tbPlayer.m_nUid, uddwTotalMight, sJson.c_str(), pstUser->m_udwBSeqNo));
}

TVOID CAuCommonAfter::CheckPeaceTime(SSession *pstSession, SUserInfo* pstUser)
{
    if (pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_PEACE_TIME].m_astBuffDetail[EN_BUFF_TYPE_ITEM].m_dwTime > CTimeUtils::GetUnixTime())
    {
        TbMarch_action* ptbPrisonTimer = NULL;
        ostringstream oss;
        for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwPassiveMarchNum; udwIdx++)
        {
            if (pstUser->m_aucPassiveMarchFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
            {
                continue;
            }
            ptbPrisonTimer = &pstUser->m_atbPassiveMarch[udwIdx];
            if (ptbPrisonTimer->m_nSclass == EN_ACTION_SEC_CLASS__PRISON_TIMER
                && ptbPrisonTimer->m_nStatus == EN_MARCH_STATUS__DEFENDING)
            {
                oss.str("");
                oss << "./send_release_dragon_req.sh " << pstUser->m_tbPlayer.m_nUid << " " << ptbPrisonTimer->m_nSuid;

                CMsgBase::SendDelaySystemMsg(oss.str().c_str());
            }
        }
    }
}

TVOID CAuCommonAfter::UpdateMapCityInfo( SUserInfo* pstUser, TbMap *ptbMap )
{
    TbPlayer* ptbPlayer = &pstUser->m_tbPlayer;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;

    if(ptbMap->m_sUname != ptbPlayer->m_sUin)
    {
        ptbMap->Set_Uname(ptbPlayer->m_sUin);
    }
    if(ptbMap->m_sAlname != ptbPlayer->m_sAlname)
    {
        if(ptbPlayer->m_nAlid != 0 && ptbPlayer->m_nAlpos != EN_ALLIANCE_POS__REQUEST)
        {
            ptbMap->Set_Alname(ptbPlayer->m_sAlname);
        }
        else
        {
            ptbMap->Set_Alname("");
        }
    }
    if(ptbMap->m_nAlid != ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET)
    {
        if(ptbPlayer->m_nAlid != 0 && ptbPlayer->m_nAlpos != EN_ALLIANCE_POS__REQUEST)
        {
            ptbMap->Set_Alid(ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET);
        }
        else
        {
            ptbMap->Set_Alid(0);
        }
    }
    if(ptbMap->m_nRally_troop_limit != pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_RALLY_TROOP_LIMIT].m_ddwBuffTotal)
    {
        ptbMap->Set_Rally_troop_limit(pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_RALLY_TROOP_LIMIT].m_ddwBuffTotal);
    }
    if(ptbMap->m_nUlevel != ptbPlayer->m_nLevel)
    {
        ptbMap->Set_Ulevel(ptbPlayer->m_nLevel);
    }
    if(ptbMap->m_nMight != ptbPlayer->m_nMight)
    {
        ptbMap->Set_Might(ptbPlayer->m_nMight);
    }
    if (ptbMap->m_nForce_kill != ptbPlayer->m_bWar_statistics[0].ddwForceKilled)
    {
        ptbMap->Set_Force_kill(ptbPlayer->m_bWar_statistics[0].ddwForceKilled);
    }
    if(ptbMap->m_nStatus != ptbPlayer->m_nStatus)
    {
        ptbMap->Set_Status(ptbPlayer->m_nStatus);
    }
    if(ptbMap->m_nTime_end != pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_PEACE_TIME].m_astBuffDetail[EN_BUFF_TYPE_ITEM].m_dwTime)
    {
        ptbMap->Set_Time_end(pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_PEACE_TIME].m_astBuffDetail[EN_BUFF_TYPE_ITEM].m_dwTime);
    }
    TINT32 dwCastleLv = CCityBase::GetBuildingLevelByFuncType(pstCity, EN_BUILDING_TYPE__CASTLE);
    if(ptbMap->m_nLevel != dwCastleLv)
    {
        ptbMap->Set_Level(dwCastleLv);
    }
    // 大使馆level
    TINT32 dwEmLv = CCityBase::GetBuildingLevelByFuncType(pstCity, EN_BUILDING_TYPE__EMBASSY);
    if(ptbMap->m_nEm_lv != dwEmLv)
    {
        ptbMap->Set_Em_lv(dwEmLv);
    }
    if(ptbMap->m_nAvatar != ptbPlayer->m_nAvatar)
    {
        ptbMap->Set_Avatar(ptbPlayer->m_nAvatar);
    }
    if(ptbMap->m_sAl_nick != pstUser->m_tbAlliance.m_sAl_nick_name)
    {
        if(ptbPlayer->m_nAlid != 0 && ptbPlayer->m_nAlpos != EN_ALLIANCE_POS__REQUEST)
        {
            ptbMap->Set_Al_nick(pstUser->m_tbAlliance.m_sAl_nick_name);
        }
        else
        {
            ptbMap->Set_Al_nick("");
        }

    }
    if(ptbMap->m_nAl_flag != pstUser->m_tbAlliance.m_nAvatar)
    {
        if(ptbPlayer->m_nAlid != 0 && ptbPlayer->m_nAlpos != EN_ALLIANCE_POS__REQUEST)
        {
            ptbMap->Set_Al_flag(pstUser->m_tbAlliance.m_nAvatar);
        }
        else
        {
            ptbMap->Set_Al_flag(0);
        }
    }
    if(ptbMap->m_nAge != pstUser->m_tbPlayer.m_nAge)
    {
        ptbMap->Set_Age(pstUser->m_tbPlayer.m_nAge);
    }
}
