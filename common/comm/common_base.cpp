#include "common_base.h"
#include "game_info.h"
#include "time_utils.h"
#include "bussiness_struct.h"
#include "common_func.h"
#include "sendmessage_base.h"
#include "event_rule_info.h"
#include "user_dyeing_info.h"
#include "city_base.h"
#include "action_base.h"
#include "tool_base.h"
#include "player_base.h"
#include "map_base.h"
#include "dragon_trail_control.h"
#include "global_gift.h"
#include "special_map.h"

//新手教学资源保护
TVOID CCommonBase::ProtectGuideResource(SUserInfo *pstUser)
{
    // 新手教学--增加资源保护
    // 科学馆lv=1 并且 没有完成第二步新手教学
    // 兵营lv=1 并且 没有完成第三步新手教学
    TUINT32 udwIdx = 0;

    TUINT8 ucAlLabLv = CCityBase::GetBuildingLevelByFuncType(&pstUser->m_stCityInfo,
        EN_BUILDING_TYPE__ALCHEMY_LAB);

    TUINT8 ucBarracksLv = CCityBase::GetBuildingLevelByFuncType(&pstUser->m_stCityInfo,
        EN_BUILDING_TYPE__BARRACKS);

    if (((1 == ucAlLabLv) && (0 == BITTEST(pstUser->m_tbLogin.m_bGuide_flag[0].m_bitFlag, EN_GUIDE_FINISH_STAGE_1_RESEARCH)))
        || ((1 == ucBarracksLv) && (0 == BITTEST(pstUser->m_tbLogin.m_bGuide_flag[0].m_bitFlag, EN_GUIDE_FINISH_STAGE_2_TRAIN_TROOP))))
    {
        pstUser->m_stCityInfo.m_astResProduction[udwIdx].m_uddwUpkeep = 0;
        TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("CCommonBase::ProtectGuideResource : upkeep = 0"));
    }
}

TVOID CCommonBase::CalcMarchActionTroop(SUserInfo *pstUser, TINT64 *addwTroop, TBOOL bIsRawTroop)
{
    SCommonTroop *pstTroop = NULL;
    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
    {
        if(EN_TABLE_UPDT_FLAG__DEL == pstUser->m_aucMarchFlag[udwIdx])
        {
            continue;
        }

        if(!CActionBase::IsPlayerOwnedAction(pstUser->m_tbPlayer.m_nUid, pstUser->m_atbMarch[udwIdx].m_nId))
        {
            continue;
        }

        if(EN_ACTION_MAIN_CLASS__MARCH == pstUser->m_atbMarch[udwIdx].m_nMclass)
        {
            if(EN_ACTION_SEC_CLASS__SCOUT == pstUser->m_atbMarch[udwIdx].m_nSclass)
            {
                continue;
            }

            if(bIsRawTroop)
            {
                pstTroop = &pstUser->m_atbMarch[udwIdx].m_bParam[0].m_stTroopRaw;
            }
            else
            {
                pstTroop = &pstUser->m_atbMarch[udwIdx].m_bParam[0].m_stTroop;
            }

            for(TUINT32 udwIdy = 0; udwIdy < EN_TROOP_TYPE__END; ++udwIdy)
            {
                addwTroop[udwIdy] += pstTroop->m_addwNum[udwIdy];
            }
        }
    }
}

TVOID CCommonBase::UpdateVipInfo(SUserInfo *pstUserInfo, SCityInfo *pstCityInfo)
{
    TbPlayer *ptbPlayer = &pstUserInfo->m_tbPlayer;
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();

    //vip 已经失效, 直接返回 由au删除action 并做处理
    if(ptbPlayer->m_nVip_etime <= udwCurTime)
    {
        return;
    }
    TbAction* ptbAction = CActionBase::GetActionByBufferId(pstUserInfo->m_atbAction, pstUserInfo->m_udwActionNum, EN_BUFFER_INFO_VIP_ACTIVATE);
    if(ptbAction == NULL)//不存在vip 的action 则生成
    {
        UActionParam stParam;
        stParam.m_stItem.SetValue(EN_BUFFER_INFO_VIP_ACTIVATE, 0, ptbPlayer->m_nVip_etime - udwCurTime);
        AddAction(pstUserInfo, pstCityInfo, EN_ACTION_MAIN_CLASS__ITEM, EN_ACTION_SEC_CLASS__ITEM,
            EN_ITEM_STATUS__USING, ptbPlayer->m_nVip_etime - udwCurTime, &stParam);
    }
    else if(ptbAction->m_nEtime != ptbPlayer->m_nVip_etime)
    {
        ptbAction->Set_Etime(ptbPlayer->m_nVip_etime);
        TINT32 dwActionIdx = CActionBase::GetActionIndex(pstUserInfo->m_atbAction, pstUserInfo->m_udwActionNum, ptbAction->m_nId);
        pstUserInfo->m_aucActionFlag[dwActionIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
    }
}

TVOID CCommonBase::ComputeUserMight(SUserInfo *pstUser)
{
    if(pstUser->m_tbPlayer.Get_Might() != pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_ADD_FORCE].m_ddwBuffTotal)
    {
        pstUser->m_tbPlayer.Set_Might(pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_ADD_FORCE].m_ddwBuffTotal);
    }
}

TINT64 CCommonBase::CalcTroopsNum(SUserInfo *pstUser)
{
    if(pstUser == NULL)
    {
        return 0;
    }

    TINT64 ddwUserTroopsNum = 0;

    // city
    ddwUserTroopsNum = CToolBase::GetTroopSumNum(pstUser->m_stCityInfo.m_stTblData.m_bTroop[0]);

    // action-troop
    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; udwIdx++)
    {
        if(pstUser->m_aucMarchFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }
        if(!CActionBase::IsPlayerOwnedAction(pstUser->m_tbPlayer.m_nUid, pstUser->m_atbMarch[udwIdx].m_nId))
        {
            continue;
        }
        if(pstUser->m_atbMarch[udwIdx].m_nMclass != EN_ACTION_MAIN_CLASS__MARCH)
        {
            continue;
        }
        if(pstUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__SCOUT)
        {
            continue;
        }
        ddwUserTroopsNum += CToolBase::GetTroopSumNum(pstUser->m_atbMarch[udwIdx].m_bParam[0].m_stTroop);
    }

    return ddwUserTroopsNum;
}

TINT64 CCommonBase::CalcFortsNum(SUserInfo *pstUser)
{
    if(pstUser == NULL)
    {
        return 0;
    }

    TINT64 ddwUserFortsNum = 0;

    // city
    ddwUserFortsNum += CToolBase::GetFortSumNum(pstUser->m_stCityInfo.m_stTblData.m_bFort[0]);

    return ddwUserFortsNum;
}

TUINT64 CCommonBase::GetTotalMight(SUserInfo *pstUser, TBOOL bNumOrCoeff)
{
    TUINT64 uddwTotalMight = 0;

    TINT64 addwTroop[EN_TROOP_TYPE__END];
    TINT64 addwFort[EN_FORT_TYPE__END];

    memset((TCHAR*)addwTroop, 0, sizeof(addwTroop));
    memset((TCHAR*)addwFort, 0, sizeof(addwFort));

    // city(city_troop, city_fort)
    CCityBase::CalcCityTroopAndFort(&pstUser->m_stCityInfo, addwTroop, addwFort);

    // action(march_raw_troop)
    CalcMarchActionTroop(pstUser, addwTroop, TRUE);

    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwSelfAlActionNum; ++udwIdx)
    {
        if (pstUser->m_aucSelfAlActionFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }

        if (pstUser->m_atbSelfAlAction[udwIdx].m_nMclass != EN_ACTION_MAIN_CLASS__TRAIN_NEW)
        {
            continue;
        }

        if (!CActionBase::IsPlayerOwnedAction(pstUser->m_tbPlayer.m_nUid, pstUser->m_atbSelfAlAction[udwIdx].m_nId))
        {
            continue;
        }

        SActionTrainParam& stTrainParam = pstUser->m_atbSelfAlAction[udwIdx].m_bParam[0].m_stTrain;

        switch (pstUser->m_atbSelfAlAction[udwIdx].m_nSclass)
        {
        case EN_ACTION_SEC_CLASS__TROOP:
        case EN_ACTION_SEC_CLASS__HOS_TREAT:
            addwTroop[stTrainParam.m_ddwType] += stTrainParam.m_ddwNum;
            break;
        case EN_ACTION_SEC_CLASS__FORT:
        case EN_ACTION_SEC_CLASS__FORT_REPAIR:
            addwFort[stTrainParam.m_ddwType] += stTrainParam.m_ddwNum;
            break;
        default:
            break;
        }
    }

    for (TUINT32 udwIdy = 0; udwIdy < EN_TROOP_TYPE__END; ++udwIdy) //hospital(troop_wait)
    {
        addwTroop[udwIdy] += pstUser->m_stCityInfo.m_stTblData.m_bHos_wait[0].m_addwNum[udwIdy];
    }

    if(TRUE == bNumOrCoeff)
    {
        // total troop 
        uddwTotalMight = CToolBase::GetTroopSumNum(addwTroop);
    }
    else
    {
        // total might
        uddwTotalMight += CToolBase::GetTroopSumForce(addwTroop);
        uddwTotalMight += CToolBase::GetFortSumMight(addwFort);
    }

    return uddwTotalMight;
}

TVOID CCommonBase::GetInCityTroop(SUserInfo* pstUser, SCityInfo* pstCity, SCommonTroop& stInCityTroop)
{
    stInCityTroop.Reset();
    for(TUINT32 udwIdx = 0; udwIdx < EN_TROOP_TYPE__END; ++udwIdx)
    {
        stInCityTroop[udwIdx] += pstCity->m_stTblData.m_bTroop[0][udwIdx];
    }

//     for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwPassiveMarchNum; ++udwIdx)
//     {
//         if(pstUser->m_aucPassiveMarchFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
//         {
//             continue;
//         }
// 
//         if(pstUser->m_atbPassiveMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_NORMAL
//             && pstUser->m_atbPassiveMarch[udwIdx].m_nStatus != EN_MARCH_STATUS__RETURNING
//             && pstUser->m_atbPassiveMarch[udwIdx].m_nTpos == pstCity->m_stTblData.m_nPos)
//         {
//             for (TUINT32 udwIdy = 0; udwIdy < EN_TROOP_TYPE__END; ++udwIdy)
//             {
//                 stInCityTroop[udwIdy] += pstUser->m_atbPassiveMarch[udwIdy].m_bParam[0].m_stTroop[udwIdy];
//             }
//         }
//     }
}

TVOID CCommonBase::GetReinforceTroop(SUserInfo* pstUser, SCityInfo* pstCity, SCommonTroop& stReinforceTroop)
{
    stReinforceTroop.Reset();
    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwPassiveMarchNum; ++udwIdx)
    {
        if (pstUser->m_aucPassiveMarchFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }

        if (pstUser->m_atbPassiveMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_NORMAL
            && pstUser->m_atbPassiveMarch[udwIdx].m_nStatus != EN_MARCH_STATUS__RETURNING
            && pstUser->m_atbPassiveMarch[udwIdx].m_nTpos == pstCity->m_stTblData.m_nPos)
        {
            for (TUINT32 udwTroopIdx = 0; udwTroopIdx < EN_TROOP_TYPE__END; ++udwTroopIdx)
            {
                stReinforceTroop[udwTroopIdx] += pstUser->m_atbPassiveMarch[udwIdx].m_bParam[0].m_stTroop[udwTroopIdx];
            }
        }
    }
}

TINT64 CCommonBase::GetReinforcedMarchNum(SUserInfo* pstUser, SCityInfo* pstCity)
{
    TINT64 ddwTotalNum = 0;
    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwPassiveMarchNum; ++udwIdx)
    {
        if(pstUser->m_aucPassiveMarchFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }

        if(pstUser->m_atbPassiveMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_NORMAL
            && pstUser->m_atbPassiveMarch[udwIdx].m_nStatus != EN_MARCH_STATUS__RETURNING
            && pstUser->m_atbPassiveMarch[udwIdx].m_nTpos == pstCity->m_stTblData.m_nPos)
        {
            ++ddwTotalNum;
        }
    }

    return ddwTotalNum;
}

TINT64 CCommonBase::GetReinforcedTroopNum(SUserInfo* pstUser, SCityInfo* pstCity)
{
    TINT64 ddwTotalNum = 0;
    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwPassiveMarchNum; ++udwIdx)
    {
        if (pstUser->m_aucPassiveMarchFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }

        if (pstUser->m_atbPassiveMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_NORMAL
            && pstUser->m_atbPassiveMarch[udwIdx].m_nStatus != EN_MARCH_STATUS__RETURNING
            && pstUser->m_atbPassiveMarch[udwIdx].m_nTpos == pstCity->m_stTblData.m_nPos)
        {
            ddwTotalNum += GetMarchTroopNum(&pstUser->m_atbPassiveMarch[udwIdx]);
        }
    }

    return ddwTotalNum;
}

TINT64 CCommonBase::GetMarchTroopNum(TbMarch_action *ptbMarch)
{
    TINT64 ddwTroopNum = 0;

    for (TUINT32 udwIdx = 0; udwIdx < EN_TROOP_TYPE__END; ++udwIdx)
    {
        ddwTroopNum += ptbMarch->m_bParam[0].m_stTroopRaw.m_addwNum[udwIdx];
    }

    return ddwTroopNum;
}

TINT64 CCommonBase::GetReinforceMarchLimit(SUserInfo* pstUser)
{
    return CCommonBase::GetGameBasicVal(EN_GAME_BASIC_RALLY_SLOT_LIMIT);
}

TINT64 CCommonBase::GetReinforceTroopLimit(SUserInfo* pstUser)
{
    return pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_REINFORCE_TROOP_NUM].m_ddwBuffTotal;
}

TVOID CCommonBase::OccupyWild(TbPlayer *ptbPlayer, TbMap *ptbMap, TbMarch_action *pstAction)
{
    // 更新野地的信息
    ptbMap->Set_Utime(CTimeUtils::GetUnixTime());
    ptbMap->Set_Might(ptbPlayer->m_nMight);
    ptbMap->Set_Force_kill(ptbPlayer->m_bWar_statistics[0].ddwForceKilled);
    ptbMap->Set_Cid(ptbPlayer->m_nCid);
    ptbMap->Set_Uid(ptbPlayer->m_nUid);
    ptbMap->Set_Ulevel(ptbPlayer->m_nLevel);
    ptbMap->Set_Uname(ptbPlayer->m_sUin);
    ptbMap->Set_Vip_level(CPlayerBase::GetRawVipLevel(ptbPlayer, ptbPlayer->m_nVip_point));
    ptbMap->Set_Vip_etime(ptbPlayer->m_nVip_etime);
    if(ptbPlayer->m_nAlpos)
    {
        ptbMap->Set_Alid(ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET);
        ptbMap->Set_Alname(ptbPlayer->m_sAlname);
        ptbMap->Set_Al_nick(ptbPlayer->m_sAl_nick_name);
    }
    ptbMap->Set_Name_update_time(CTimeUtils::GetUnixTime());
    ptbMap->Set_Bid(CMapBase::GetBlockIdFromPos(ptbMap->m_nId));

    if(pstAction)
    {
        ptbMap->Set_Time_end(pstAction->m_nEtime);
        ptbMap->Set_Res_rate(pstAction->m_bParam[0].m_ddwLoadRate);
        ptbMap->Set_Res_time(pstAction->m_bParam[0].m_ddwLoadTime);
    }
}

TVOID CCommonBase::OccupyWildByMarch(TbMap *ptbMap, TbMarch_action *ptbAction)
{
    ptbMap->Set_Utime(CTimeUtils::GetUnixTime());
    ptbMap->Set_Cid(ptbAction->m_nScid);
    ptbMap->Set_Uid(ptbAction->m_nSuid);
    ptbMap->Set_Uname(ptbAction->m_bParam[0].m_szSourceUserName);
    ptbMap->Set_Alid(ptbAction->m_bParam[0].m_ddwSourceAlliance);
    ptbMap->Set_Alname(ptbAction->m_bParam[0].m_szSourceAlliance);
    ptbMap->Set_Al_nick(ptbAction->m_bParam[0].m_szSourceAlNick);
    ptbMap->Set_Res_rate(ptbAction->m_bParam[0].m_ddwLoadRate);
    ptbMap->Set_Res_time(ptbAction->m_bParam[0].m_ddwLoadTime);
}

TVOID CCommonBase::CampWild(TbPlayer *ptbPlayer, TbMap *ptbMap, TbMarch_action *pstAction)
{
    // 更新野地的信息
    ptbMap->Set_Utime(CTimeUtils::GetUnixTime());
    ptbMap->Set_Might(ptbPlayer->m_nMight);
    ptbMap->Set_Force_kill(ptbPlayer->m_bWar_statistics[0].ddwForceKilled);
    ptbMap->Set_Cid(ptbPlayer->m_nCid);
    ptbMap->Set_Uid(ptbPlayer->m_nUid);
    ptbMap->Set_Ulevel(ptbPlayer->m_nLevel);
    ptbMap->Set_Uname(ptbPlayer->m_sUin);
    ptbMap->Set_Vip_level(CPlayerBase::GetRawVipLevel(ptbPlayer, ptbPlayer->m_nVip_point));
    ptbMap->Set_Vip_etime(ptbPlayer->m_nVip_etime);
    if (ptbPlayer->m_nAlpos)
    {
        ptbMap->Set_Alid(ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET);
        ptbMap->Set_Alname(ptbPlayer->m_sAlname);
        ptbMap->Set_Al_nick(ptbPlayer->m_sAl_nick_name);
    }
    ptbMap->Set_Name_update_time(CTimeUtils::GetUnixTime());
    ptbMap->Set_Bid(CMapBase::GetBlockIdFromPos(ptbMap->m_nId));
    ptbMap->Set_Type(EN_WILD_TYPE__CAMP);

    if (pstAction)
    {
        ptbMap->Set_Time_end(pstAction->m_nEtime);
    }
}

TVOID CCommonBase::AbandonWild(SCityInfo *pstCity, TbMap *pstMap)
{
    // update map info
    pstMap->Set_Utime(CTimeUtils::GetUnixTime());
    if(pstMap->m_nType == EN_WILD_TYPE__CITY || pstMap->m_nType == EN_WILD_TYPE__CAMP)
    {
        pstMap->Set_Type(EN_WILD_TYPE__NORMAL);
        pstMap->Set_Level(1);
        pstMap->Set_Expire_time(0);
    }
    pstMap->Set_Name("");
    pstMap->Set_Status(0);
    pstMap->Set_Might(0);
    pstMap->Set_Force_kill(0);
    pstMap->Set_Cid(0);
    pstMap->Set_Uid(0);
    pstMap->Set_Uname("");
    pstMap->Set_Ulevel(0);
    pstMap->Set_Alid(0);
    pstMap->Set_Alname("");
    pstMap->Set_Npc(0);
    pstMap->Set_Res_rate(0);
    pstMap->Set_Res_time(0);
    pstMap->Set_Em_lv(0);
    pstMap->Set_Vip_level(0);
    pstMap->Set_Vip_etime(0);
    pstMap->Set_Time_end(0);
    pstMap->Set_Age(0);
    pstMap->Set_Burn_end_time(0);
    pstMap->Set_Smoke_end_time(0);
    pstMap->Set_Prison_flag(0);
    pstMap->Set_Al_nick("");
    pstMap->Set_Name_update_time(CTimeUtils::GetUnixTime());
    
    pstMap->m_jCity_info.clear();
    pstMap->SetFlag(TbMAP_FIELD_CITY_INFO);
    pstMap->Set_Avatar(0);
    pstMap->Set_Rally_troop_limit(0);
    pstMap->Set_Showtime(0);
    pstMap->Set_Leader_monster_flag(0);
    pstMap->Set_Occupy_num(0);

    if(pstMap->m_nType == EN_WILD_TYPE__NORMAL)
    {
        pstMap->Set_Bid(0);
    }
}

// enMainClass 是后台分类，enSecClass 是客户端对action的唯一分类
// 只处理普通action，可以alliance help的action用alliance action表，march相关action用march action表
TVOID CCommonBase::AddAction(SUserInfo *pstUser, SCityInfo*pstCity, TUINT8 enMainClass, TUINT8 enSecClass, TUINT8 enStatus, TUINT32 udwCostTime,
    UActionParam *pstParam, TUINT64 uddwNewTaskId)
{
    TbAction *pstAction = &pstUser->m_atbAction[pstUser->m_udwActionNum];
    pstAction->Reset();

    assert(pstUser->m_tbPlayer.m_nUid);

    // 1. set head
    if (uddwNewTaskId)
    {
        pstAction->Set_Id(uddwNewTaskId);
    }
    else
    {
        pstAction->Set_Id(CToolBase::GetPlayerNewTaskId(pstUser->m_tbPlayer.m_nUid, pstUser->m_tbLogin.m_nSeq));
    }
    TbLogin& tbLogin = pstUser->m_tbLogin;
    tbLogin.Set_Seq(tbLogin.m_nSeq + 1); //llt add,防止,同一个用户请求中,生成多个action的情况
    pstAction->Set_Suid(pstUser->m_tbPlayer.m_nUid);
    pstAction->Set_Mclass(enMainClass);
    pstAction->Set_Sclass(enSecClass);
    pstAction->Set_Status(enStatus);
    pstAction->Set_Btime(CTimeUtils::GetUnixTime());
    pstAction->Set_Ctime(udwCostTime);
    pstAction->Set_Etime(pstAction->m_nBtime + udwCostTime);
    pstAction->Set_Retry(0);
    pstAction->Set_Sid(pstUser->m_tbPlayer.m_nSid);

    // 2. set param
    memcpy((char*)&pstAction->m_bParam.m_astList[0], (char*)pstParam, sizeof(UActionParam));
    pstAction->SetFlag(TbACTION_FIELD_PARAM);


    // 3. set table flag
    pstUser->m_aucActionFlag[pstUser->m_udwActionNum] = EN_TABLE_UPDT_FLAG__NEW;

    // 4. auto increase
    pstUser->m_udwActionNum++;

    // 5. for log
    pstUser->m_uddwNewActionId = pstAction->m_nId;
}

TVOID CCommonBase::SetPlayerStateTipsTime(SUserInfo *pstUser)
{
    if(pstUser->m_tbUserStat.m_nUid == 0)
    {
        return;
    }

    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwTipsNum; udwIdx++)
    {
        if(pstUser->m_tbUserStat.m_nTips < pstUser->m_atbTips[udwIdx].m_nTime)
        {
            pstUser->m_tbUserStat.Set_Tips(pstUser->m_atbTips[udwIdx].m_nTime);
        }
    }
}

TVOID CCommonBase::SetPlayerStateEventTipsTime(SUserInfo *pstUser, TBOOL bIsOperateCmd, TBOOL bIsLogin)
{
    if(pstUser->m_tbUserStat.m_nUid == 0)
    {
        return;
    }

    if(bIsOperateCmd)
    {
        return;
    }
    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwEventTipsNum; udwIdx++)
    {
        //联盟event tips
        if(pstUser->m_atbEventTips[udwIdx].m_nUid < 0)
        {
            if(pstUser->m_tbUserStat.m_nAl_event_tips_time < pstUser->m_atbEventTips[udwIdx].m_nTime)
            {
                pstUser->m_tbUserStat.Set_Al_event_tips_time(pstUser->m_atbEventTips[udwIdx].m_nTime);
            }
        }
        else
        {
            if(pstUser->m_tbUserStat.m_nLast_event_win_id < pstUser->m_atbEventTips[udwIdx].m_nId && !bIsLogin)
            {
                pstUser->m_tbUserStat.Set_Last_event_win_id(pstUser->m_atbEventTips[udwIdx].m_nId);
            }
        }
    }

    if (bIsLogin)
    {
        pstUser->m_udwEventTipsNum = 0;
    }
}


TUINT64 CCommonBase::PeriodProcMonitorUserInfo(SUserInfo *pstUser)
{
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    // 1\ production / bonus
    CGameInfo *pGameInfo = CGameInfo::GetInstance();

    const TINT32 k_max_production_bonus = 20000;
    const TINT32 k_max_production = 8000000;
    const TINT32 k_max_base_production = 600000;
    for(TUINT32 udwIdx = 0; udwIdx < EN_RESOURCE_TYPE__END; ++udwIdx)
    {
        if(k_max_production_bonus <= pstCity->m_astResProduction[udwIdx].m_ddwTotalBonus)
        {
            pstUser->m_dwErrProdBonus++;
            TSE_LOG_ERROR(pGameInfo->m_poLog, ("MonitorUserInfo: [%u,%ld]", pstUser->m_tbLogin.m_nUid, pstCity->m_astResProduction[udwIdx].m_ddwTotalBonus));

        }

        if(k_max_production <= pstCity->m_astResProduction[udwIdx].m_ddwCurProduction)
        {
            pstUser->m_dwErrProd++;
            TSE_LOG_ERROR(pGameInfo->m_poLog, ("MonitorUserInfo: [%u,%ld]", pstUser->m_tbLogin.m_nUid, pstCity->m_astResProduction[udwIdx].m_ddwCurProduction));
        }

        if(k_max_base_production <= pstCity->m_astResProduction[udwIdx].m_ddwBaseProduction)
        {
            pstUser->m_dwErrProd++;
            TSE_LOG_ERROR(pGameInfo->m_poLog, ("MonitorUserInfo: [%u,%ld]", pstUser->m_tbLogin.m_nUid, pstCity->m_astResProduction[udwIdx].m_ddwBaseProduction));
        }
    }

    // 2\march bonus
    //const TUINT32 k_max_march_bonus = 20000;

    // 3\ troop
    const TUINT32 k_max_troop_num = 20000000;
    TUINT64 udddwTotalTroop = GetTotalMight(pstUser, TRUE);
    if(k_max_troop_num <= udddwTotalTroop)
    {
        pstUser->m_dwErrTroop++;
        TSE_LOG_ERROR(pGameInfo->m_poLog, ("MonitorUserInfo: [%u,%lu]", pstUser->m_tbLogin.m_nUid, udddwTotalTroop));
    }

    // 4\ fort
    //const TUINT32 k_max_fort_num = 20000000;

    // 5 \ gem
    const TINT32 k_max_cost_gem_num = 500000;
    const TINT32 k_max_own_gem_num = 200000;
    if((k_max_cost_gem_num < pstUser->m_tbLogin.m_nGem_cost - pstUser->m_tbLogin.m_nGem_buy
        || k_max_own_gem_num < pstUser->m_tbLogin.m_nGem - pstUser->m_tbLogin.m_nGem_buy)
        && (0 == pstUser->m_tbLogin.m_nNpc) && (1500 < pstUser->m_tbLogin.m_nUid))
    {
        pstUser->m_dwErrGem++;
        TSE_LOG_ERROR(pGameInfo->m_poLog, ("MonitorUserInfo: [%u,%lu,%lu,%lu]",
            pstUser->m_tbLogin.m_nUid,
            pstUser->m_tbLogin.m_nGem_cost
            , pstUser->m_tbLogin.m_nGem_buy,
            pstUser->m_tbLogin.m_nGem));
    }

    // 6 \ item

    // 7 \build 
    const TUINT32 k_max_build_lv = 30;
    for(TUINT32 idx = 0; idx < pstCity->m_stTblData.m_bBuilding.m_udwNum; idx++)
    {

        if(pstCity->m_stTblData.m_bBuilding[idx].m_ddwLevel > k_max_build_lv)
        {
            pstUser->m_dwErrBuildLv++;
            TSE_LOG_ERROR(pGameInfo->m_poLog, ("MonitorUserInfo: [%u,%u,%lu]",
                pstUser->m_tbLogin.m_nUid,
                pstCity->m_stTblData.m_bBuilding[idx].m_ddwType,
                pstCity->m_stTblData.m_bBuilding[idx].m_ddwLevel));
        }
    }

    // lv

    return 0;
}

//TINT32 CCommonBase::GetMaxMarchNum(SUserInfo *pstUser)
//{
//    return pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_QUEUE_NUM].m_ddwBuffTotal;
//}

TVOID CCommonBase::SetActionDeleteFlag(SUserInfo* pstUser, TINT64 ddwTargetActionId,TUINT32 udwSecondClass)
{
    TUINT32 udwActionType = CActionBase::GetActionTypeBySecondClass(udwSecondClass);
    switch(udwActionType)
    {
    case EN_ACTION_TYPE_AL_CAN_HELP:
        SetAlActionDeleteFlag(pstUser->m_atbSelfAlAction,pstUser->m_aucSelfAlActionFlag,pstUser->m_udwSelfAlActionNum,ddwTargetActionId);
        break;
    case EN_ACTION_TYPE_BUFF_NORMAL:
        SetActionDeleteFlag(pstUser->m_atbAction, pstUser->m_aucActionFlag, pstUser->m_udwActionNum, ddwTargetActionId);
        break;
    case EN_ACTION_TYPE_MARCH:
        SetMarchActionDeleteFlag(pstUser->m_atbMarch,pstUser->m_aucMarchFlag,pstUser->m_udwMarchNum,ddwTargetActionId);
        break;
    }
    
}

TVOID CCommonBase::SetActionDeleteFlag(TbAction* ptbActionList, TUINT8* pActionFlagList, TINT32 dwActionNum, TINT64 ddwTargetActionId)
{
    TINT32 dwActionIdx = CActionBase::GetActionIndex(ptbActionList, dwActionNum, ddwTargetActionId);
    if(dwActionIdx >= 0)
    {
        pActionFlagList[dwActionIdx] = EN_TABLE_UPDT_FLAG__DEL;
    }
}

TVOID CCommonBase::SetAlActionDeleteFlag(TbAlliance_action* ptbActionList, TUINT8* pActionFlagList, TINT32 dwActionNum, TINT64 ddwTargetActionId)
{
    TINT32 dwActionIdx = CActionBase::GetAlActionIndex(ptbActionList, dwActionNum, ddwTargetActionId);
    if(dwActionIdx >= 0)
    {
        pActionFlagList[dwActionIdx] = EN_TABLE_UPDT_FLAG__DEL;
    }
}


TVOID CCommonBase::SetMarchActionDeleteFlag(TbMarch_action* ptbActionList, TUINT8* pActionFlagList, TINT32 dwActionNum, TINT64 ddwTargetActionId)
{
    TINT32 dwActionIdx = CActionBase::GetMarchIndex(ptbActionList, dwActionNum, ddwTargetActionId);
    if(dwActionIdx >= 0)
    {
        pActionFlagList[dwActionIdx] = EN_TABLE_UPDT_FLAG__DEL;
    }
}


TVOID CCommonBase::SetMapToNewCity(TbMap *pstMapItem, TbPlayer *pstPlayer, SCityInfo *pstCity, TbAlliance *pstAlliance,TUINT32 udwSTime/* = 0 */ ,TUINT32 udwBTime/* = 0*/,TUINT32 udwPrisonFlag /* = 0*/)
{
    // normal field
    pstMapItem->Set_Utime(CTimeUtils::GetUnixTime());
    pstMapItem->Set_Type(EN_WILD_TYPE__CITY);
    pstMapItem->Set_Name(pstCity->m_stTblData.m_sName);
    pstMapItem->Set_Status(pstPlayer->m_nStatus);
    pstMapItem->Set_Might(pstPlayer->m_nMight);
    pstMapItem->Set_Force_kill(pstPlayer->m_bWar_statistics[0].ddwForceKilled);
    pstMapItem->Set_Cid(pstMapItem->m_nId);
    pstMapItem->Set_Uid(pstPlayer->m_nUid);
    pstMapItem->Set_Uname(pstPlayer->m_sUin);
    pstMapItem->Set_Ulevel(pstPlayer->m_nLevel);
    pstMapItem->Set_Time_end(0); // add nemo 20140224 防止移城到camp的位置产生保护
    if(pstPlayer->m_nAlpos > EN_ALLIANCE_POS__REQUEST)
    {
        pstMapItem->Set_Alid(pstPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET);
        pstMapItem->Set_Alname(pstPlayer->m_sAlname);
    }
    else
    {
        pstMapItem->Set_Alid(0);
        pstMapItem->Set_Alname("");
    }
    pstMapItem->Set_Npc(pstPlayer->m_nNpc);
    pstMapItem->Set_Bid(CMapBase::GetBlockIdFromPos(pstMapItem->m_nId));
    pstMapItem->Set_Res_rate(0);
    pstMapItem->Set_Res_time(0);
    pstMapItem->Set_Vip_level(CPlayerBase::GetRawVipLevel(pstPlayer, pstPlayer->m_nVip_point));
    pstMapItem->Set_Vip_etime(pstPlayer->m_nVip_etime);
    pstMapItem->Set_Avatar(pstPlayer->m_nAvatar);
    pstMapItem->Set_Prison_flag(udwPrisonFlag);
    pstMapItem->Set_Name_update_time(CTimeUtils::GetUnixTime());

    if(pstAlliance != NULL)
    {
        pstMapItem->Set_Al_nick(pstAlliance->m_sAl_nick_name);
        pstMapItem->Set_Al_flag(pstAlliance->m_nAvatar);
    }

    // 建筑level
    TUINT8 ucBuildLevel = CCityBase::GetBuildingLevelByFuncType(pstCity, EN_BUILDING_TYPE__CASTLE);
    pstMapItem->Set_Level(ucBuildLevel);
    ucBuildLevel = CCityBase::GetBuildingLevelByFuncType(pstCity, EN_BUILDING_TYPE__EMBASSY);
    pstMapItem->Set_Em_lv(ucBuildLevel);
    ucBuildLevel = CCityBase::GetBuildingLevelByFuncType(pstCity, EN_BUILDING_TYPE__RALLY_POINT);

    //状态信息
    pstMapItem->Set_Burn_end_time(udwBTime);
    pstMapItem->Set_Smoke_end_time(udwSTime);
    pstMapItem->Set_Expire_time(0);
    pstMapItem->Set_Age(pstPlayer->m_nAge);

    // binary field
    memset((TCHAR*)pstMapItem->m_bResource.m_astList[0].m_addwNum, 0, sizeof(TINT64)*EN_RESOURCE_TYPE__END);
    pstMapItem->SetFlag(TbMAP_FIELD_RESOURCE);
    memset((TCHAR*)pstMapItem->m_bTroop.m_astList[0].m_addwNum, 0, sizeof(TINT64)*EN_TROOP_TYPE__END);
    pstMapItem->SetFlag(TbMAP_FIELD_TROOP);
}

TINT32 CCommonBase::AddVipPoint(SUserInfo *pstUserInfo, SCityInfo *pstCityInfo, TUINT32 udwVipPointAdd)
{
    pstUserInfo->m_tbPlayer.Set_Vip_point(pstUserInfo->m_tbPlayer.m_nVip_point + udwVipPointAdd);
    if (pstUserInfo->m_tbPlayer.m_nVip_point > CPlayerBase::GetMaxVipPoint())
    {
        pstUserInfo->m_tbPlayer.Set_Vip_point(CPlayerBase::GetMaxVipPoint());
    }
    CCommonBase::UpdateVipInfo(pstUserInfo, pstCityInfo);
    return 0;
}

TINT32 CCommonBase::AddVipTime(SUserInfo *pstUserInfo, SCityInfo *pstCityInfo, TUINT32 udwVipTimeAdd)
{
    if(udwVipTimeAdd == 0)
    {
        return 0;
    }

    TbPlayer *ptbPlayer = &pstUserInfo->m_tbPlayer;
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();

    if(ptbPlayer->m_nVip_etime <= udwCurTime)
    {
        ptbPlayer->Set_Vip_etime(udwCurTime + udwVipTimeAdd);
    }
    else
    {
        ptbPlayer->Set_Vip_etime(ptbPlayer->m_nVip_etime + udwVipTimeAdd);
    }

    CCommonBase::UpdateVipInfo(pstUserInfo, pstCityInfo);

    return 0;
}

TINT32 CCommonBase::GetResearchCost(TINT32 dwResearchId, TINT32 dwTargetLv, SSpGlobalRes& rCost)
{
    if(dwTargetLv <= 0)
    {
        return -1;
    }
    rCost.Reset();
    const Json::Value& oJsonResearh = CGameInfo::GetInstance()->m_oJsonRoot["game_research"];
    string strResearchId = CCommonFunc::NumToString(dwResearchId);
    const Json::Value& oJsonResearchCost = oJsonResearh[strResearchId]["r"]["r0"][dwTargetLv - 1]["a1"];
    for(TUINT32 udwIdx = 0; udwIdx < oJsonResearchCost.size(); ++udwIdx)
    {
        rCost.aRewardList[rCost.udwTotalNum].Reset();
        rCost.aRewardList[rCost.udwTotalNum].udwType = oJsonResearchCost[udwIdx][0u].asUInt();
        rCost.aRewardList[rCost.udwTotalNum].udwId = oJsonResearchCost[udwIdx][1u].asUInt();
        rCost.aRewardList[rCost.udwTotalNum].udwNum = oJsonResearchCost[udwIdx][2u].asUInt();
        rCost.udwTotalNum++;
    }
    return 0;
}

TINT32 CCommonBase::GetAlGiftLevel(TbAlliance* ptbAlliance)
{
    TINT32 dwLevel = 0;
    CGameInfo* pstGameInfo = CGameInfo::GetInstance();
    while(dwLevel < (TINT32)pstGameInfo->m_oJsonRoot["game_al_gift_new"]["a0"].size())
    {
        if(ptbAlliance->m_nGift_point < pstGameInfo->m_oJsonRoot["game_al_gift_new"]["a0"][dwLevel].asInt())
        {
            return dwLevel;
        }
        dwLevel++;
    }
    return dwLevel;
}

TINT64 CCommonBase::GetAlGiftPoint(TINT64 ddwPoint)
{
    CGameInfo* pstGameInfo = CGameInfo::GetInstance();
    TINT32 dwMaxLevel = pstGameInfo->m_oJsonRoot["game_al_gift_new"]["a0"].size();
    TINT64 ddwLimitPoint = 0;
    if(dwMaxLevel > 0)
    {
        ddwLimitPoint = pstGameInfo->m_oJsonRoot["game_al_gift_new"]["a0"][dwMaxLevel - 1].asInt64();
    }
    return ddwPoint > ddwLimitPoint ? ddwLimitPoint : ddwPoint;
}

TINT64 CCommonBase::GetConLoginVipPoint(TINT64 ddwConLoginDays)
{
    CGameInfo* pstGameInfo = CGameInfo::GetInstance();
    TINT32 dwIndex = 0;
    if(ddwConLoginDays > pstGameInfo->m_oJsonRoot["game_conn_login"]["vip_point"].size())
    {
        dwIndex = pstGameInfo->m_oJsonRoot["game_conn_login"]["vip_point"].size() - 1;
    }
    else
    {
        dwIndex = ddwConLoginDays - 1;
    }
    if(dwIndex < 0)
    {
        dwIndex = 0;
    }
    return pstGameInfo->m_oJsonRoot["game_conn_login"]["vip_point"][dwIndex].asInt64();
}

TUINT32 CCommonBase::GetVipLevelUpRewardTime(TINT32 dwLevelUp)
{
    return dwLevelUp * CGameInfo::GetInstance()->m_oJsonRoot["game_basic"][EN_GAME_BASIC_VIP_LEVEL_UP_REWARD_TIME].asUInt();
}

TbDiplomacy *CCommonBase::GetDiplomacy(TbDiplomacy *patbDiplomacyList, TUINT32 udwDipNum, TINT32 dwTargetAlid)
{
    TbDiplomacy *ptbDip = NULL;
    for(TUINT32 udwIdx = 0; udwIdx < udwDipNum; ++udwIdx)
    {
        if(patbDiplomacyList[udwIdx].m_nDes_al == dwTargetAlid)
        {
            ptbDip = &patbDiplomacyList[udwIdx];
            break;
        }
    }
    return ptbDip;
}

TINT32 CCommonBase::GetTroopTotalLoad(TbMarch_action *ptbMarch, TUINT32 udwCurTime/* = 0*/)
{
    TUINT64 uddwLoad = 0;
    CGameInfo *poGameInfo = CGameInfo::GetInstance();
    const Json::Value &jTroop = poGameInfo->m_oJsonRoot["game_troop"];
    for(TUINT32 dwIdx = 0; dwIdx < jTroop.size(); ++dwIdx)
    {
        if(ptbMarch->m_bParam[0].m_stTroop[dwIdx] == 0)
        {
            continue;
        }
        uddwLoad += jTroop[dwIdx]["a"]["a4"].asInt() * ptbMarch->m_bParam[0].m_stTroop[dwIdx];
    }

    uddwLoad = uddwLoad *(10000 + CActionBase::GetUserBuffById(ptbMarch, EN_BUFFER_INFO_TROOP_LOAD, udwCurTime)) / 10000;

    return uddwLoad;
}

TINT64 CCommonBase::GetTroopTotalLoad(SCommonTroop *pstTroop)
{
    TINT64 ddwLoad = 0;
    CGameInfo *poGameInfo = CGameInfo::GetInstance();
    const Json::Value &jTroop = poGameInfo->m_oJsonRoot["game_troop"];
    for (TUINT32 dwIdx = 0; dwIdx < jTroop.size(); ++dwIdx)
    {
        if (pstTroop->m_addwNum[dwIdx] == 0)
        {
            continue;
        }
        ddwLoad += jTroop[dwIdx]["a"]["a4"].asInt() * pstTroop->m_addwNum[dwIdx];
    }

    return ddwLoad;
}

TBOOL CCommonBase::IsActionDoingOnUser(SUserInfo *pstUserInfo)
{
    for(TUINT32 udwIdx = 0; udwIdx < pstUserInfo->m_udwPassiveMarchNum; ++udwIdx)
    {
        if (pstUserInfo->m_atbPassiveMarch[udwIdx].m_nTuid == pstUserInfo->m_tbPlayer.m_nUid
            && pstUserInfo->m_atbPassiveMarch[udwIdx].m_nTpos == pstUserInfo->m_tbPlayer.m_nCid)
        {
            if (pstUserInfo->m_atbPassiveMarch[udwIdx].m_nStatus == EN_MARCH_STATUS__FIGHTING
                || pstUserInfo->m_atbPassiveMarch[udwIdx].m_nStatus == EN_MARCH_STATUS__SETUP_CAMP
                || pstUserInfo->m_atbPassiveMarch[udwIdx].m_nStatus == EN_MARCH_STATUS__UN_LOADING)
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

TINT32 CCommonBase::AddAllianceFund(TbPlayer *pstPlayer, TbAlliance *pstAlliance, TUINT32 udwNum)
{
    if(pstPlayer->m_nAlid != 0 && pstPlayer->m_nAlpos > EN_ALLIANCE_POS__REQUEST)
    {
        if(pstAlliance->m_nAid == 0)
        {
            pstAlliance->Set_Sid(pstPlayer->m_nSid);
            pstAlliance->Set_Aid(pstPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET);
            pstAlliance->Set_Fund(udwNum, UPDATE_ACTION_TYPE_ADD);
        }
        else
        {
            if(pstAlliance->m_mFlag.find(TbALLIANCE_FIELD_FUND) != pstAlliance->m_mFlag.end())
            {
                pstAlliance->Set_Fund(pstAlliance->m_nFund + udwNum, pstAlliance->m_mFlag[TbALLIANCE_FIELD_FUND]);
            }
            else
            {
                pstAlliance->Set_Fund(pstAlliance->m_nFund + udwNum);
            }
        }
    }

    return 0;
}

TINT32 CCommonBase::CostAllianceFund(TbPlayer *pstPlayer, TbAlliance *pstAlliance, TUINT32 udwNum)
{
    if(pstPlayer->m_nAlid != 0 && pstPlayer->m_nAlpos > EN_ALLIANCE_POS__REQUEST)
    {
        pstAlliance->Set_Fund(pstAlliance->m_nFund > udwNum ? pstAlliance->m_nFund - udwNum : 0);
    }

    return 0;
}


TINT32 CCommonBase::HasEnoughAllianceFund(TbPlayer *pstPlayer, TbAlliance *pstAlliance, TUINT32 udwNum)
{
    if(pstPlayer->m_nAlid != 0 && pstPlayer->m_nAlpos > EN_ALLIANCE_POS__REQUEST)
    {
        if(pstAlliance->m_nFund > udwNum)
        {
            return TRUE;
        }
    }

    return FALSE;
}

TINT32 CCommonBase::GetRewardBoxUiType(SSpGlobalRes *pstGlobalRes)
{
    TINT32 dwUiType = -1;
    //分析reward数据
    TUINT32 audwTypeNum[EN_GLOBALRES_TYPE_END] = {0};

    for(TUINT32 udwIdx = 0; udwIdx < pstGlobalRes->udwTotalNum; ++udwIdx)
    {
        audwTypeNum[pstGlobalRes->aRewardList[udwIdx].udwType]++;
    }

    const Json::Value &oRewardBoxJson = CGameInfo::GetInstance()->m_oJsonRoot["game_reward_box"];
    Json::Value::Members oRewardBoxJsonIdx = oRewardBoxJson.getMemberNames();
    for(Json::Value::Members::iterator it = oRewardBoxJsonIdx.begin(); it != oRewardBoxJsonIdx.end(); ++it)
    {
        TBOOL bSpType = FALSE;
        TBOOL bNormalType = FALSE;
        TINT32 adwRewardListNum[EN_GLOBALRES_TYPE_END];
        for(TUINT32 udwIdx = 0; udwIdx < EN_GLOBALRES_TYPE_END; ++udwIdx)
        {
            adwRewardListNum[udwIdx] = -1;
        }


        TUINT32 udwTempleType = oRewardBoxJson[(*it)]["templet_type"].asUInt();

        const Json::Value &oRewardList = oRewardBoxJson[(*it)]["reward_list"];
        for(TUINT32 udwIdx = 0; udwIdx < oRewardList.size(); ++udwIdx)
        {
            adwRewardListNum[oRewardBoxJson[(*it)]["reward_list"][udwIdx][0U].asUInt()] = oRewardBoxJson[(*it)]["reward_list"][udwIdx][1U].asInt();
        }

        if(udwTempleType == 0)
        {
            bSpType = IsSpUiType(&audwTypeNum[0], &adwRewardListNum[0], EN_GLOBALRES_TYPE_END);
        }
        if(udwTempleType == 1)
        {
            bNormalType = IsNormalUiType(&audwTypeNum[0], &adwRewardListNum[0], EN_GLOBALRES_TYPE_END);
        }
        TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("CCommonBase::GetRewardBoxUiType: [ui_type=%u temple_type=%u bSpType=%u b_NormalType=%u]",
            atoi((*it).c_str()),
            udwTempleType,
            bSpType,
            bNormalType));

        if(bSpType)
        {
            return atoi((*it).c_str());
        }
        else if(bNormalType)
        {
            dwUiType = atoi((*it).c_str());
        }
    }
    return dwUiType;
}

TUINT64 CCommonBase::ComputeTotalMight(SUserInfo *pstUser)
{
    TUINT64 uddwTotalMight = 0;
    TUINT32 udwUnitMight = 0;
    CGameInfo *poGameInfo = CGameInfo::GetInstance();

    TINT64 addwTroop[EN_TROOP_TYPE__END];
    TINT64 addwFort[EN_FORT_TYPE__END];

    memset((TCHAR*)addwTroop, 0, sizeof(addwTroop));
    memset((TCHAR*)addwFort, 0, sizeof(addwFort));

    // city(city_troop, city_fort)
    TbCity* ptbCity = &pstUser->m_stCityInfo.m_stTblData;
    // troop
    for (TUINT32 udwIdx = 0; udwIdx < EN_TROOP_TYPE__END; udwIdx++)
    {
        addwTroop[udwIdx] += ptbCity->m_bTroop.m_astList[0].m_addwNum[udwIdx];
    }
    // fort
    for (TUINT32 udwIdx = 0; udwIdx < EN_FORT_TYPE__END; udwIdx++)
    {
        addwFort[udwIdx] += ptbCity->m_bFort.m_astList[0].m_addwNum[udwIdx];
    }

    // march(march_raw_troop)
    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; udwIdx++)
    {
        if(EN_TABLE_UPDT_FLAG__DEL == pstUser->m_aucMarchFlag[udwIdx])
        {
            continue;
        }
        if(!CActionBase::IsPlayerOwnedAction(pstUser->m_tbPlayer.m_nUid, pstUser->m_atbMarch[udwIdx].m_nId))
        {
            continue;
        }

        if(EN_ACTION_MAIN_CLASS__MARCH == pstUser->m_atbMarch[udwIdx].m_nMclass)
        {
            if(EN_ACTION_SEC_CLASS__SCOUT == pstUser->m_atbMarch[udwIdx].m_nSclass)
            {
                continue;
            }
            // march_troop
            for(TUINT32 udwIdy = 0; udwIdy < EN_TROOP_TYPE__END; udwIdy++)
            {
                addwTroop[udwIdy] += pstUser->m_atbMarch[udwIdx].m_bParam[0].m_stTroop.m_addwNum[udwIdy];
            }
        }
    }

    // troop fort list
    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwSelfAlActionNum; ++udwIdx)
    {
        if (pstUser->m_aucSelfAlActionFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }

        if (pstUser->m_atbSelfAlAction[udwIdx].m_nMclass != EN_ACTION_MAIN_CLASS__TRAIN_NEW)
        {
            continue;
        }

        if (!CActionBase::IsPlayerOwnedAction(pstUser->m_tbPlayer.m_nUid, pstUser->m_atbSelfAlAction[udwIdx].m_nId))
        {
            continue;
        }

        SActionTrainParam& stTrainParam = pstUser->m_atbSelfAlAction[udwIdx].m_bParam[0].m_stTrain;

        switch (pstUser->m_atbSelfAlAction[udwIdx].m_nSclass)
        {
        case EN_ACTION_SEC_CLASS__TROOP:
        case EN_ACTION_SEC_CLASS__HOS_TREAT:
            addwTroop[stTrainParam.m_ddwType] += stTrainParam.m_ddwNum;
            break;
        case EN_ACTION_SEC_CLASS__FORT:
        case EN_ACTION_SEC_CLASS__FORT_REPAIR:
            addwFort[stTrainParam.m_ddwType] += stTrainParam.m_ddwNum;
            break;
        default:
            break;
        }
    }

    for (TUINT32 udwIdx = 0; udwIdx < EN_TROOP_TYPE__END; udwIdx++)// hospital(troop_wait)
    {
        addwTroop[udwIdx] += ptbCity->m_bHos_wait[0].m_addwNum[udwIdx];
    }

    // total might
    for(TUINT32 udwIdx = 0; udwIdx < EN_TROOP_TYPE__END; udwIdx++)
    {
        if(addwTroop[udwIdx] > 0)
        {
            udwUnitMight = poGameInfo->m_oJsonRoot["game_troop"][udwIdx]["a"]["a9"].asUInt();
            uddwTotalMight += udwUnitMight * addwTroop[udwIdx];
        }
    }
    for(TUINT32 udwIdx = 0; udwIdx < EN_FORT_TYPE__END; udwIdx++)
    {
        if(addwFort[udwIdx] > 0)
        {
            udwUnitMight = poGameInfo->m_oJsonRoot["game_fort"][udwIdx]["a"]["a9"].asUInt();
            uddwTotalMight += udwUnitMight * addwFort[udwIdx];
        }
    }

    return uddwTotalMight;
}


TUINT64 CCommonBase::ComputeFortMight(SUserInfo *pstUser,TUINT32 &udwFortNum)
{
    TUINT64 uddwTotalMight = 0;
    TUINT32 udwUnitMight = 0;
    CGameInfo *poGameInfo = CGameInfo::GetInstance();

    TINT64 addwFort[EN_FORT_TYPE__END];
    memset((TCHAR*)addwFort, 0, sizeof(addwFort));

    // city(city_troop, city_fort)
    TbCity* ptbCity = &pstUser->m_stCityInfo.m_stTblData;
    // fort
    for (TUINT32 udwIdx = 0; udwIdx < EN_FORT_TYPE__END; udwIdx++)
    {
        addwFort[udwIdx] += ptbCity->m_bFort.m_astList[0].m_addwNum[udwIdx];
    }

    for(TUINT32 udwIdx = 0; udwIdx < EN_FORT_TYPE__END; udwIdx++)
    {
        if(addwFort[udwIdx] > 0)
        {
            udwUnitMight = poGameInfo->m_oJsonRoot["game_fort"][udwIdx]["a"]["a9"].asUInt();
            uddwTotalMight += udwUnitMight * addwFort[udwIdx];
            udwFortNum += addwFort[udwIdx];
        }
    }

    return uddwTotalMight;
}

TUINT64 CCommonBase::ComputeTotalTroopMight(SUserInfo *pstUser,TUINT32 &udwTroopNum)
{
    TUINT64 uddwTotalMight = 0;
    TUINT32 udwUnitMight = 0;
    CGameInfo *poGameInfo = CGameInfo::GetInstance();

    TINT64 addwTroop[EN_TROOP_TYPE__END];

    memset((TCHAR*)addwTroop, 0, sizeof(addwTroop));

    // city(city_troop, city_fort)
    TbCity* ptbCity = &pstUser->m_stCityInfo.m_stTblData;
    // troop
    for (TUINT32 udwIdx = 0; udwIdx < EN_TROOP_TYPE__END; udwIdx++)
    {
        addwTroop[udwIdx] += ptbCity->m_bTroop.m_astList[0].m_addwNum[udwIdx];
    }

    // march(march_raw_troop)
    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; udwIdx++)
    {
        if(EN_TABLE_UPDT_FLAG__DEL == pstUser->m_aucMarchFlag[udwIdx])
        {
            continue;
        }

        if(!CActionBase::IsPlayerOwnedAction(pstUser->m_tbPlayer.m_nUid, pstUser->m_atbMarch[udwIdx].m_nId))
        {
            continue;
        }

        if(EN_ACTION_MAIN_CLASS__MARCH == pstUser->m_atbMarch[udwIdx].m_nMclass)
        {
            if(EN_ACTION_SEC_CLASS__SCOUT == pstUser->m_atbMarch[udwIdx].m_nSclass)
            {
                continue;
            }
            // march_troop
            for(TUINT32 udwIdy = 0; udwIdy < EN_TROOP_TYPE__END; udwIdy++)
            {
                addwTroop[udwIdy] += pstUser->m_atbMarch[udwIdx].m_bParam[0].m_stTroop.m_addwNum[udwIdy];
            }
        }
    }

    // total might
    for(TUINT32 udwIdx = 0; udwIdx < EN_TROOP_TYPE__END; udwIdx++)
    {
        if(addwTroop[udwIdx] > 0)
        {
            udwUnitMight = poGameInfo->m_oJsonRoot["game_troop"][udwIdx]["a"]["a9"].asUInt();
            uddwTotalMight += udwUnitMight * addwTroop[udwIdx];
            udwTroopNum += addwTroop[udwIdx];
        }
    }
    return uddwTotalMight;
}

TBOOL CCommonBase::ComputeTotalTroop(SUserInfo *pstUser, TINT64 addwTroop[])
{
    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("ComputeTotalTroop"));
    
    memset((TCHAR*)addwTroop, 0, sizeof(TINT64)*EN_TROOP_TYPE__END);

    // city(city_troop, city_fort)
    TbCity* ptbCity = &pstUser->m_stCityInfo.m_stTblData;
    // troop
    for (TUINT32 udwIdx = 0; udwIdx < EN_TROOP_TYPE__END; udwIdx++)
    {
        addwTroop[udwIdx] += ptbCity->m_bTroop.m_astList[0].m_addwNum[udwIdx];
    }

    // march
    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; udwIdx++)
    {
        if(EN_TABLE_UPDT_FLAG__DEL == pstUser->m_aucMarchFlag[udwIdx])
        {
            continue;
        }
        if(!CActionBase::IsPlayerOwnedAction(pstUser->m_tbPlayer.m_nUid, pstUser->m_atbMarch[udwIdx].m_nId))
        {
            continue;
        }

        if(EN_ACTION_MAIN_CLASS__MARCH == pstUser->m_atbMarch[udwIdx].m_nMclass)
        {
            if(EN_ACTION_SEC_CLASS__SCOUT == pstUser->m_atbMarch[udwIdx].m_nSclass)
            {
                continue;
            }
            // march_raw_troop
            for(TUINT32 udwIdy = 0; udwIdy < EN_TROOP_TYPE__END; udwIdy++)
            {
                addwTroop[udwIdy] += pstUser->m_atbMarch[udwIdx].m_bParam[0].m_stTroop.m_addwNum[udwIdy];
            }
        }
    }

    return true;
}

TINT32 CCommonBase::GetUserDna(TINT64 ddwUserId, TINT32 dwCastleLv, TINT32 dwGemBuy, TINT32 dwAlGiftLv,
        TINT32 dwMaxAlGemBuy, TUINT32 udwSid, SUserEventGoals* pOutUserEventGoals)
{
    if (dwCastleLv == 0)
    {
        dwCastleLv = 1;
    }
    if (dwAlGiftLv == 0)
    {
        dwAlGiftLv = 1;
    }

    const Json::Value &jsonEvent = CEventRuleInfo::GetInstance()->m_jEventRule; // 活动配置

    SDyeingInfo stDyeingInfo;
    CUserDyeingInfo::GetInstance()->GetUserDyeingInfo(ddwUserId, &stDyeingInfo);
    if (stDyeingInfo.m_ddwUserId == 0)
    {
        stDyeingInfo.SetVal(ddwUserId, 0, 0, 0, 0);
    }
    TINT64 ddwDyeingId = stDyeingInfo.m_ddwF1; // 染色id
    bitset<sizeof(ddwDyeingId)* 8> bitFlag(ddwDyeingId);

    // 地狱个人活动
    TUINT32 udwEventType = EN_EVENT_TYPE__PERSONAL_1; // 活动类型
    TCHAR szEventType[32];
    snprintf(szEventType, 32, "%u", udwEventType);
    TUINT32 udwStrategyId = 0;
    for (TINT32 dwBit = 0; dwBit < 16; ++dwBit)
    {
        if (bitFlag[dwBit] != 0)
        {
            udwStrategyId = dwBit;
            break;
        }
    }
    udwStrategyId -= 0;

    udwStrategyId = ddwDyeingId;

    TCHAR szStrategyId[32];
    snprintf(szStrategyId, 32, "%u", udwStrategyId); // 策略ID
    if (!jsonEvent["basic"].isMember(szEventType) || !jsonEvent["basic"][szEventType].isMember(szStrategyId))
    {
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("CCommonBase::GetUserDna: event_type=%u, strategy_id=%u, uid=%d, city_lv=%d, gem_buy=%d, al_gift_lv=%d",
            udwEventType, udwStrategyId, ddwUserId, dwCastleLv, dwGemBuy, dwAlGiftLv));
        return -1;
    }
    TBOOL bFindGoal = FALSE;
    TUINT32 udwGoalSize = jsonEvent["basic"][szEventType][szStrategyId].size();
    for (TUINT32 udwIdx = 0; udwIdx < udwGoalSize; ++udwIdx) // for a goal
    {
        const Json::Value &jsonGoal = jsonEvent["basic"][szEventType][szStrategyId][udwIdx];
        TUINT32 udwGoalsId = jsonGoal[0U].asInt();
        TUINT32 udwRuleSize = jsonGoal[1U].size();
        TBOOL bReachGoal = TRUE;
        for (TUINT32 udwIdy = 0; udwIdy < udwRuleSize; ++udwIdy) // for a rule
        {
            const Json::Value &jsonRule = jsonGoal[1U][udwIdy];
            TINT32 dwMin = jsonRule[1U].asInt();
            TINT32 dwMax = jsonRule[2U].asInt();
            if (jsonRule[0U].asInt() == 0) // castle
            {
                if (dwMin == dwMax && dwCastleLv != dwMin)
                {
                    bReachGoal = FALSE;
                }
                else
                {
                    if (!(dwCastleLv > dwMin))
                    {
                        bReachGoal = FALSE;
                    }
                    if (dwMax != -1 && !(dwCastleLv <= dwMax)) // -1 mean maxi
                    {
                        bReachGoal = FALSE;
                    }
                }
            }
            else if (jsonRule[0U].asInt() == 1) // gem
            {
                if (dwMin == 0 && dwMax == 0 && dwGemBuy == 0)
                {

                }
                else
                {
                    if (!(dwGemBuy > dwMin))
                    {
                        bReachGoal = FALSE;
                    }
                    if (dwMax != -1 && !(dwGemBuy <= dwMax)) // -1 mean maxi
                    {
                        bReachGoal = FALSE;
                    }
                }
            }
            else if (jsonRule[0U].asInt() == 2) // al gift lv
            {
                if (dwMin == dwMax && dwAlGiftLv != dwMin)
                {
                    bReachGoal = FALSE;
                }
                else
                {
                    if (!(dwAlGiftLv > dwMin))
                    {
                        bReachGoal = FALSE;
                    }
                    if (dwMax != -1 && !(dwAlGiftLv <= dwMax)) // -1 mean maxi
                    {
                        bReachGoal = FALSE;
                    }
                }
            }
        }
        if (bReachGoal) // reach goal
        {
            bFindGoal = TRUE;
            pOutUserEventGoals->asGoalList[udwEventType].dwFirstGroup = (udwGoalsId / 1000) % 10;
            pOutUserEventGoals->asGoalList[udwEventType].dwSecGroup = udwGoalsId % 1000;
            pOutUserEventGoals->asGoalList[udwEventType].dwRand = ((CTimeUtils::GetUnixTime()/3600)%3);//udwGoalsId % 3;
            pOutUserEventGoals->asGoalList[udwEventType].dwGoalId = udwGoalsId;
            break;
        }
    }
    if (!bFindGoal)
    {
        return -1;
    }

    // 联盟活动
    udwEventType = EN_EVENT_TYPE__ALLIANCE; // 活动类型
    snprintf(szEventType, 32, "%u", udwEventType);
    udwStrategyId = 16;
    
    if (udwSid >= 3 )
    {
        udwStrategyId = 1;
    }
    else
    {
        udwStrategyId = 0;
    }
     
    snprintf(szStrategyId, 32, "%u", udwStrategyId); // 策略ID
    if (!jsonEvent["basic"].isMember(szEventType) || !jsonEvent["basic"][szEventType].isMember(szStrategyId))
    {
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("CCommonBase::GetUserDna: event_type=%u, strategy_id=%u, uid=%d, city_lv=%d, gem_buy=%d, al_gift_lv=%d",
            udwEventType, udwStrategyId, ddwUserId, dwCastleLv, dwMaxAlGemBuy, dwAlGiftLv));
        return -1;
    }
    bFindGoal = FALSE;
    udwGoalSize = jsonEvent["basic"][szEventType][szStrategyId].size();
    for (TUINT32 udwIdx = 0; udwIdx < udwGoalSize; ++udwIdx) // for a goal
    {
        const Json::Value &jsonGoal = jsonEvent["basic"][szEventType][szStrategyId][udwIdx];
        TUINT32 udwGoalsId = jsonGoal[0U].asInt();
        TUINT32 udwRuleSize = jsonGoal[1U].size();
        TBOOL bReachGoal = TRUE;
        for (TUINT32 udwIdy = 0; udwIdy < udwRuleSize; ++udwIdy) // for a rule
        {
            const Json::Value &jsonRule = jsonGoal[1U][udwIdy];
            TINT32 dwMin = jsonRule[1U].asInt();
            TINT32 dwMax = jsonRule[2U].asInt();
            if (jsonRule[0U].asInt() == 0) // castle
            {
                if (dwMin == dwMax && dwCastleLv != dwMin)
                {
                    bReachGoal = FALSE;
                }
                else
                {
                    if (!(dwCastleLv > dwMin))
                    {
                        bReachGoal = FALSE;
                    }
                    if (dwMax != -1 && !(dwCastleLv <= dwMax)) // -1 mean maxi
                    {
                        bReachGoal = FALSE;
                    }
                }
            }
            else if (jsonRule[0U].asInt() == 1) // gem
            {
                if (dwMin == dwMax && dwMaxAlGemBuy != dwMin)
                {
                    bReachGoal = FALSE;
                }
                else
                {
                    if (!(dwMaxAlGemBuy > dwMin))
                    {
                        bReachGoal = FALSE;
                    }
                    if (dwMax != -1 && !(dwMaxAlGemBuy <= dwMax)) // -1 mean maxi
                    {
                        bReachGoal = FALSE;
                    }
                }
            }
            else if (jsonRule[0U].asInt() == 2) // al gift lv
            {
                if (dwMin == dwMax && dwAlGiftLv != dwMin)
                {
                    bReachGoal = FALSE;
                }
                else
                {
                    if (!(dwAlGiftLv > dwMin))
                    {
                        bReachGoal = FALSE;
                    }
                    if (dwMax != -1 && !(dwAlGiftLv <= dwMax)) // -1 mean maxi
                    {
                        bReachGoal = FALSE;
                    }
                }
            }
        }
        if (bReachGoal) // reach goal
        {
            bFindGoal = TRUE;
            pOutUserEventGoals->asGoalList[udwEventType].dwFirstGroup = (udwGoalsId / 1000) % 10;
            pOutUserEventGoals->asGoalList[udwEventType].dwSecGroup = udwGoalsId % 1000;
            pOutUserEventGoals->asGoalList[udwEventType].dwRand = 0;//udwGoalsId % 3;
            pOutUserEventGoals->asGoalList[udwEventType].dwGoalId = udwGoalsId;
            break;
        }
    }



    if (!bFindGoal)
    {
        return -1;
    }

    // 个人活动
    udwEventType = EN_EVENT_TYPE__PERSONAL_2;
	snprintf(szEventType, 32, "%u", udwEventType);
    udwStrategyId = 0;

    //TCHAR szStrategyId[32];
    snprintf(szStrategyId, 32, "%u", udwStrategyId); // 
    if (!jsonEvent["basic"].isMember(szEventType) || !jsonEvent["basic"][szEventType].isMember(szStrategyId))
    {
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("CCommonBase::GetUserDna: event_type=%u, strategy_id=%u, uid=%d, city_lv=%d, gem_buy=%d, al_gift_lv=%d",
            udwEventType, udwStrategyId, ddwUserId, dwCastleLv, dwGemBuy, dwAlGiftLv));
        return -1;
    }
    bFindGoal = FALSE;
    udwGoalSize = jsonEvent["basic"][szEventType][szStrategyId].size();
    for (TUINT32 udwIdx = 0; udwIdx < udwGoalSize; ++udwIdx) // for a goal
    {
        const Json::Value &jsonGoal = jsonEvent["basic"][szEventType][szStrategyId][udwIdx];
        TUINT32 udwGoalsId = jsonGoal[0U].asInt();
        TUINT32 udwRuleSize = jsonGoal[1U].size();
        TBOOL bReachGoal = TRUE;
        for (TUINT32 udwIdy = 0; udwIdy < udwRuleSize; ++udwIdy) // for a rule
        {
            const Json::Value &jsonRule = jsonGoal[1U][udwIdy];
            TINT32 dwMin = jsonRule[1U].asInt();
            TINT32 dwMax = jsonRule[2U].asInt();
            if (jsonRule[0U].asInt() == 0) // castle
            {
                if (dwMin == dwMax && dwCastleLv != dwMin)
                {
                    bReachGoal = FALSE;
                }
                else
                {
                    if (!(dwCastleLv > dwMin))
                    {
                        bReachGoal = FALSE;
                    }
                    if (dwMax != -1 && !(dwCastleLv <= dwMax)) // -1 mean maxi
                    {
                        bReachGoal = FALSE;
                    }
                }
            }
            else if (jsonRule[0U].asInt() == 1) // gem
            {
                if (dwMin == 0 && dwMax == 0 && dwGemBuy == 0)
                {

                }
                else
                {
                    if (!(dwGemBuy > dwMin))
                    {
                        bReachGoal = FALSE;
                    }
                    if (dwMax != -1 && !(dwGemBuy <= dwMax)) // -1 mean maxi
                    {
                        bReachGoal = FALSE;
                    }
                }
            }
            else if (jsonRule[0U].asInt() == 2) // al gift lv
            {
                if (dwMin == dwMax && dwAlGiftLv != dwMin)
                {
                    bReachGoal = FALSE;
                }
                else
                {
                    if (!(dwAlGiftLv > dwMin))
                    {
                        bReachGoal = FALSE;
                    }
                    if (dwMax != -1 && !(dwAlGiftLv <= dwMax)) // -1 mean maxi
                    {
                        bReachGoal = FALSE;
                    }
                }
            }
        }
        if (bReachGoal) // reach goal
        {
            bFindGoal = TRUE;
            pOutUserEventGoals->asGoalList[udwEventType].dwFirstGroup = (udwGoalsId / 1000) % 10;
            pOutUserEventGoals->asGoalList[udwEventType].dwSecGroup = udwGoalsId % 1000;
            pOutUserEventGoals->asGoalList[udwEventType].dwRand = ((CTimeUtils::GetUnixTime()/3600)%3);//udwGoalsId % 3;
            pOutUserEventGoals->asGoalList[udwEventType].dwGoalId = udwGoalsId;
            break;
        }
    }
    if (!bFindGoal)
    {
        return -1;
    }

    pOutUserEventGoals->ddwF1 = stDyeingInfo.m_ddwF1;
    pOutUserEventGoals->ddwF2 = stDyeingInfo.m_ddwF2;
    pOutUserEventGoals->ddwF3 = stDyeingInfo.m_ddwF3;
    pOutUserEventGoals->ddwF4 = stDyeingInfo.m_ddwF4;


    return 0;
}

TINT32 CCommonBase::ComputeSpeedUpGem(TINT32 dwSpeedUpSec)
{
    if (dwSpeedUpSec <= 0)
    {
        return 0;
    }
    CGameInfo *poGameInfo = CGameInfo::GetInstance();

    vector<pair<TINT32, TINT32> > vecItemList;
    const Json::Value &jItem = poGameInfo->m_oJsonRoot["game_item"];
    Json::Value::Members member = jItem.getMemberNames();
    for (Json::Value::Members::iterator it = member.begin(); it != member.end(); ++it)
    {
        if (jItem[*it]["a2"].asInt() == 10001 && jItem[*it]["a3"].asInt() == 1)
        {
            vecItemList.push_back(make_pair(jItem[*it]["a0"].asInt(), jItem[*it]["a14"][0U][2U].asInt()));
        }
    }

    std::sort(vecItemList.begin(), vecItemList.end(), BaseCompare);

    TINT32 dwGemCost = GetTimeOptimizeGem(dwSpeedUpSec, 0, vecItemList);

    TSE_LOG_INFO(CGameInfo::GetInstance()->m_poLog, ("ComputeSpeedUpGem: [time=%d] [gem=%d]", dwSpeedUpSec, dwGemCost));

    return dwGemCost;
}

TINT32 CCommonBase::GetTimeOptimizeGem(TINT32 dwTime, TINT32 dwIdx, vector<pair<TINT32, TINT32> > &vecList)
{
    int total_gem = 0;
    if (dwTime > 0 && dwIdx >= 0 && dwIdx < (TINT32)vecList.size())
    {
        TINT32 dwItemSpeedTime = vecList[dwIdx].second;
        TINT32 dwItemGem = vecList[dwIdx].first;
        while (dwTime >= dwItemSpeedTime)
        {
            dwTime -= dwItemSpeedTime;
            total_gem += dwItemGem;
        }
        if (dwTime > 0)
        {
            if (dwIdx < (TINT32)vecList.size())
            {
                total_gem += min(GetTimeOptimizeGem(dwTime, dwIdx + 1, vecList), dwItemGem);
            }
            else
            {
                total_gem += dwItemGem;
            }
        }
    }
    return total_gem;
}

TBOOL CCommonBase::IsCorrectTime(TINT32 dwGivenTime, TINT32 dwCalcTime)
{
    TSE_LOG_INFO(CGameInfo::GetInstance()->m_poLog, ("IsCorrectTime: given_time:%u calc_time:%d",
        dwGivenTime, dwCalcTime));
    TINT32 dwTmp = dwCalcTime * 0.95;
    if (dwGivenTime >= dwTmp)
    {
        return TRUE;
    }
    return FALSE;
}

TINT32 CCommonBase::CheckInstantGem(TINT32 dwGivenCost, TINT32 dwSpeedUpTime)
{
    TINT32 dwGem = 0;
    dwGem += ComputeSpeedUpGem(dwSpeedUpTime);

    TSE_LOG_INFO(CGameInfo::GetInstance()->m_poLog, ("CheckInstantGem: speed_up_time:%d, given_gem:%d calc_gem:%d",
        dwSpeedUpTime, dwGivenCost, dwGem));
    TINT32 dwTmp = dwGem * 0.95;
    if (dwGivenCost >= dwTmp)
    {
        return TRUE;
    }
    return FALSE;
}


TINT32 CCommonBase::GetTroopMarchTime(SCommonTroop *pstTroop, TINT32 dwFrom, TINT32 dwTo, SPlayerBuffInfo *pstBuff)
{
    CGameInfo *poGameInfo = CGameInfo::GetInstance();

    TINT32 dwLowestSpeed = 0;
    for (TUINT32 udwIdx = 0; udwIdx < poGameInfo->m_oJsonRoot["game_troop"].size(); ++udwIdx)
    {
        if (pstTroop->m_addwNum[udwIdx] == 0)
        {
            continue;
        }
        TINT32 dwSpeed = poGameInfo->m_oJsonRoot["game_troop"][udwIdx]["a"]["a3"].asInt();
        TINT32 dwCategory = poGameInfo->m_oJsonRoot["game_troop"][udwIdx]["a"]["a5"].asInt();
        TINT32 dwBuff = 10000 + pstBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_MARCH_SPEED].m_ddwBuffTotal;

        if (dwCategory == EN_TROOP_CATEGORY__INFANTRY)
        {
            dwBuff += pstBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_INFANTRY_MARCH_SPEED_ONLY].m_ddwBuffTotal;
        }
        else if (dwCategory == EN_TROOP_CATEGORY__REMOTE)
        {
            dwBuff += pstBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_REMOTE_MARCH_SPEED_ONLY].m_ddwBuffTotal;
        }
        else if (dwCategory == EN_TROOP_CATEGORY__SOWAR)
        {
            dwBuff += pstBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_SOWAR_MARCH_SPEED_ONLY].m_ddwBuffTotal;
        }
        else if (dwCategory == EN_TROOP_CATEGORY__SIEGE)
        {
            dwBuff += pstBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_SIEGE_MARCH_SPEED_ONLY].m_ddwBuffTotal;
        }

        dwSpeed = dwSpeed * (1.0 * dwBuff / 10000);
        if (dwLowestSpeed == 0 || dwLowestSpeed > dwSpeed)
        {
            dwLowestSpeed = dwSpeed;
        }
    }

    if (dwLowestSpeed == 0)
    {
        return 0x7FFFFFFF;
    }

    TINT32 x = dwFrom / MAP_X_Y_POS_COMPUTE_OFFSET - dwTo / MAP_X_Y_POS_COMPUTE_OFFSET;
    TINT32 y = dwFrom % MAP_X_Y_POS_COMPUTE_OFFSET - dwTo % MAP_X_Y_POS_COMPUTE_OFFSET;
    TINT32 dwDistance = sqrt(x * x + y * y);

    TINT32 dwTime = 1.0 * dwDistance / dwLowestSpeed * 6000;

    if (dwTime < 30)
    {
        dwTime = 30;
    }

    return dwTime;
}

TINT32 CCommonBase::GetTroopReinforceTime(SCommonTroop *pstTroop, TINT32 dwFrom, TINT32 dwTo, SPlayerBuffInfo *pstBuff)
{
    CGameInfo *poGameInfo = CGameInfo::GetInstance();

    TINT32 dwLowestSpeed = 0;
    for (TUINT32 udwIdx = 0; udwIdx < poGameInfo->m_oJsonRoot["game_troop"].size(); ++udwIdx)
    {
        if (pstTroop->m_addwNum[udwIdx] == 0)
        {
            continue;
        }
        TINT32 dwSpeed = poGameInfo->m_oJsonRoot["game_troop"][udwIdx]["a"]["a3"].asInt();
        TINT32 dwCategory = poGameInfo->m_oJsonRoot["game_troop"][udwIdx]["a"]["a5"].asInt();
        TINT32 dwBuff = 10000 + pstBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_MARCH_SPEED].m_ddwBuffTotal
            + pstBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_REINFORCE_SPEED_UP_PERCENT].m_ddwBuffTotal;

        if (dwCategory == EN_TROOP_CATEGORY__INFANTRY)
        {
            dwBuff += pstBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_INFANTRY_MARCH_SPEED_ONLY].m_ddwBuffTotal;
        }
        else if (dwCategory == EN_TROOP_CATEGORY__REMOTE)
        {
            dwBuff += pstBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_REMOTE_MARCH_SPEED_ONLY].m_ddwBuffTotal;
        }
        else if (dwCategory == EN_TROOP_CATEGORY__SOWAR)
        {
            dwBuff += pstBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_SOWAR_MARCH_SPEED_ONLY].m_ddwBuffTotal;
        }
        else if (dwCategory == EN_TROOP_CATEGORY__SIEGE)
        {
            dwBuff += pstBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_SIEGE_MARCH_SPEED_ONLY].m_ddwBuffTotal;
        }

        dwSpeed = dwSpeed * (1.0 * dwBuff / 10000);
        if (dwLowestSpeed == 0 || dwLowestSpeed > dwSpeed)
        {
            dwLowestSpeed = dwSpeed;
        }
        if (dwLowestSpeed == 0 || dwLowestSpeed > dwSpeed)
        {
            dwLowestSpeed = dwSpeed;
        }
    }

    if (dwLowestSpeed == 0)
    {
        return 0x7FFFFFFF;
    }

    TINT32 x = dwFrom / MAP_X_Y_POS_COMPUTE_OFFSET - dwTo / MAP_X_Y_POS_COMPUTE_OFFSET;
    TINT32 y = dwFrom % MAP_X_Y_POS_COMPUTE_OFFSET - dwTo % MAP_X_Y_POS_COMPUTE_OFFSET;
    TINT32 dwDistance = sqrt(x * x + y * y);

    TINT32 dwTime = 1.0 * dwDistance / dwLowestSpeed * 6000;

    if (dwTime < 30)
    {
        dwTime = 30;
    }

    return dwTime;
}

TINT32 CCommonBase::GetTroopTransportTime(TINT32 dwFrom, TINT32 dwTo, SPlayerBuffInfo *pstBuff)
{
    TINT32 dwSpeed = CCommonBase::GetGameBasicVal(EN_GAME_BASIC_DEFAULT_TRANSPORT_SPEED);
    TINT32 dwBuff = 10000 + pstBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_MARCH_SPEED].m_ddwBuffTotal
        + pstBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_TRANSPORT_SPEED].m_ddwBuffTotal
        + pstBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_REINFORCE_SPEED_UP_PERCENT].m_ddwBuffTotal;
    
    dwSpeed *= dwBuff * 1.0 / 10000;

    if (dwSpeed == 0)
    {
        return 0x7FFFFFFF;
    }

    TINT32 x = dwFrom / MAP_X_Y_POS_COMPUTE_OFFSET - dwTo / MAP_X_Y_POS_COMPUTE_OFFSET;
    TINT32 y = dwFrom % MAP_X_Y_POS_COMPUTE_OFFSET - dwTo % MAP_X_Y_POS_COMPUTE_OFFSET;
    TINT32 dwDistance = sqrt(x * x + y * y);

    TINT32 dwTime = 1.0 * dwDistance / dwSpeed * 6000;

    if (dwTime < 30)
    {
        dwTime = 30;
    }

    return dwTime;
}

TINT32 CCommonBase::GetTroopRallyWarTime(SCommonTroop *pstTroop, TINT32 dwFrom, TINT32 dwTo, SPlayerBuffInfo *pstBuff)
{
    CGameInfo *poGameInfo = CGameInfo::GetInstance();

    TINT32 dwLowestSpeed = 0;
    for (TUINT32 udwIdx = 0; udwIdx < poGameInfo->m_oJsonRoot["game_troop"].size(); ++udwIdx)
    {
        if (pstTroop->m_addwNum[udwIdx] == 0)
        {
            continue;
        }
        TINT32 dwSpeed = poGameInfo->m_oJsonRoot["game_troop"][udwIdx]["a"]["a3"].asInt();
        TINT32 dwCategory = poGameInfo->m_oJsonRoot["game_troop"][udwIdx]["a"]["a5"].asInt();
        TINT32 dwBuff = 10000 + pstBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_MARCH_SPEED].m_ddwBuffTotal
            + pstBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_RALLY_WAR_SPEED].m_ddwBuffTotal;

        if (dwCategory == EN_TROOP_CATEGORY__INFANTRY)
        {
            dwBuff += pstBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_INFANTRY_MARCH_SPEED_ONLY].m_ddwBuffTotal;
            dwBuff += pstBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_RALLY_WAR_INFANTRY_SPEED].m_ddwBuffTotal;
        }
        else if (dwCategory == EN_TROOP_CATEGORY__REMOTE)
        {
            dwBuff += pstBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_REMOTE_MARCH_SPEED_ONLY].m_ddwBuffTotal;
            dwBuff += pstBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_RALLY_WAR_REMOTE_SPEED].m_ddwBuffTotal;
        }
        else if (dwCategory == EN_TROOP_CATEGORY__SOWAR)
        {
            dwBuff += pstBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_SOWAR_MARCH_SPEED_ONLY].m_ddwBuffTotal;
            dwBuff += pstBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_RALLY_WAR_SOWAR_SPEED].m_ddwBuffTotal;
        }
        else if (dwCategory == EN_TROOP_CATEGORY__SIEGE)
        {
            dwBuff += pstBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_SIEGE_MARCH_SPEED_ONLY].m_ddwBuffTotal;
            dwBuff += pstBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_RALLY_WAR_SIEGE_SPEED].m_ddwBuffTotal;
        }

        dwSpeed = dwSpeed * (1.0 * dwBuff / 10000);
        if (dwLowestSpeed == 0 || dwLowestSpeed > dwSpeed)
        {
            dwLowestSpeed = dwSpeed;
        }
    }

    if (dwLowestSpeed == 0)
    {
        return 0x7FFFFFFF;
    }

    TINT32 x = dwFrom / MAP_X_Y_POS_COMPUTE_OFFSET - dwTo / MAP_X_Y_POS_COMPUTE_OFFSET;
    TINT32 y = dwFrom % MAP_X_Y_POS_COMPUTE_OFFSET - dwTo % MAP_X_Y_POS_COMPUTE_OFFSET;
    TINT32 dwDistance = sqrt(x * x + y * y);

    TINT32 dwTime = 1.0 * dwDistance / dwLowestSpeed * 6000;

    if (dwTime < 30)
    {
        dwTime = 30;
    }

    return dwTime;
}

TINT32 CCommonBase::GetScoutMarchTime(TINT32 dwFrom, TINT32 dwTo, TUINT32 udwBuff)
{
    TINT32 dwSpeed = 2000;
    TINT32 x = dwFrom / MAP_X_Y_POS_COMPUTE_OFFSET - dwTo / MAP_X_Y_POS_COMPUTE_OFFSET;
    TINT32 y = dwFrom % MAP_X_Y_POS_COMPUTE_OFFSET - dwTo % MAP_X_Y_POS_COMPUTE_OFFSET;
    TINT32 dwDistance = sqrt(x * x + y * y);

    TINT32 dwTime = 1.0 * dwDistance / dwSpeed / (udwBuff * 1.0 / 10000 + 1.0) * 6000;

    if (dwTime < 30)
    {
        dwTime = 30;
    }

    return dwTime;
}

TINT32 CCommonBase::GetHeroMarchTime(TINT32 dwFrom, TINT32 dwTo, TUINT32 udwBuff)
{
    TINT32 dwSpeed = GetGameBasicVal(EN_GAME_BASIC_HERO_MARCH_SPEED);

    if (dwSpeed == 0)
    {
        return 0x7FFFFFFF;
    }

    TINT32 x = dwFrom / MAP_X_Y_POS_COMPUTE_OFFSET - dwTo / MAP_X_Y_POS_COMPUTE_OFFSET;
    TINT32 y = dwFrom % MAP_X_Y_POS_COMPUTE_OFFSET - dwTo % MAP_X_Y_POS_COMPUTE_OFFSET;
    TINT32 dwDistance = sqrt(x * x + y * y);

    TINT32 dwTime = 1.0 * dwDistance / dwSpeed / (udwBuff * 1.0 / 10000 + 1.0) * 6000;

//     if (dwTime < 30)
//     {
//         dwTime = 30;
//     }

    return dwTime;
}

TUINT32 CCommonBase::GetPlayerAllianceId(TUINT32 udwAlId, TUINT8 ucAlPos)
{
    return udwAlId * PLAYER_ALLIANCE_ID_OFFSET + ucAlPos;
}

TBOOL CCommonBase::HasEnoughDoubloon(TbPlayer *ptbPlayer, TINT32 dwPrice)
{
    return ptbPlayer->m_nDoubloon >= dwPrice;
}

TINT32 CCommonBase::CostDoubloon(TbPlayer *ptbPlayer, TINT32 dwPrice)
{
    if (ptbPlayer->m_nDoubloon > dwPrice)
    {
        ptbPlayer->Set_Doubloon(ptbPlayer->m_nDoubloon - dwPrice);
    }
    else
    {
        ptbPlayer->Set_Doubloon(0);
    }

    return 0;
}

TUINT64 CCommonBase::GetTotalTroopAndFort(SUserInfo *pstUser, Json::Value &jsonRet)
{
    CGameInfo *poGameInfo = CGameInfo::GetInstance();

    map<int, long> m_TroopNum;  //<type, num>
    map<int, long> m_FortNum;  //<type, num>
    m_TroopNum.clear();
    m_FortNum.clear();

    TUINT32 udwTroopSize = poGameInfo->m_oJsonRoot["game_troop"].size();
    TUINT32 udwFortSize = poGameInfo->m_oJsonRoot["game_fort"].size();

    //现有
    // city(city_troop, city_fort)
    TbCity* ptbCity = &pstUser->m_stCityInfo.m_stTblData;
    // troop
    for (TUINT32 udwIdx = 0; udwIdx < udwTroopSize && udwIdx < EN_TROOP_TYPE__END; udwIdx++)
    {
        if (0 < ptbCity->m_bTroop.m_astList[0].m_addwNum[udwIdx])
        {
            if (m_TroopNum.find(udwIdx) == m_TroopNum.end())
            {
                m_TroopNum[udwIdx] = 0;
            }
            m_TroopNum[udwIdx] += ptbCity->m_bTroop.m_astList[0].m_addwNum[udwIdx];
        }
    }
    // fort
    for (TUINT32 udwIdx = 0; udwIdx < udwFortSize && udwIdx < EN_FORT_TYPE__END; udwIdx++)
    {
        if (0 < ptbCity->m_bFort.m_astList[0].m_addwNum[udwIdx])
        {
            if (m_FortNum.find(udwIdx) == m_FortNum.end())
            {
                m_FortNum[udwIdx] = 0;
            }
            m_FortNum[udwIdx] += ptbCity->m_bFort.m_astList[0].m_addwNum[udwIdx];
        }
    }

    //March
    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; udwIdx++)
    {
        if (EN_TABLE_UPDT_FLAG__DEL == pstUser->m_aucMarchFlag[udwIdx])
        {
            continue;
        }
        if (!CActionBase::IsPlayerOwnedAction(pstUser->m_tbPlayer.m_nUid, pstUser->m_atbMarch[udwIdx].m_nId))
        {
            continue;
        }

        if (EN_ACTION_MAIN_CLASS__MARCH == pstUser->m_atbMarch[udwIdx].m_nMclass)
        {
            if (EN_ACTION_SEC_CLASS__SCOUT == pstUser->m_atbMarch[udwIdx].m_nSclass)
            {
                continue;
            }
            // march_troop
            for (TUINT32 udwIdy = 0; udwIdy < udwTroopSize && udwIdy < EN_TROOP_TYPE__END; udwIdy++)
            {
                if (0 < pstUser->m_atbMarch[udwIdx].m_bParam[0].m_stTroop.m_addwNum[udwIdy])
                {
                    if (m_TroopNum.find(udwIdx) == m_TroopNum.end())
                    {
                        m_TroopNum[udwIdx] = 0;
                    }
                    m_TroopNum[udwIdy] += pstUser->m_atbMarch[udwIdx].m_bParam[0].m_stTroop.m_addwNum[udwIdy];
                }
            }
        }
    }

    //训练
    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwSelfAlActionNum; ++udwIdx)
    {
        if (pstUser->m_aucSelfAlActionFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }

        if (pstUser->m_atbSelfAlAction[udwIdx].m_nMclass != EN_ACTION_MAIN_CLASS__TRAIN_NEW)
        {
            continue;
        }

        if (!CActionBase::IsPlayerOwnedAction(pstUser->m_tbPlayer.m_nUid, pstUser->m_atbSelfAlAction[udwIdx].m_nId))
        {
            continue;
        }

        SActionTrainParam& stTrainParam = pstUser->m_atbSelfAlAction[udwIdx].m_bParam[0].m_stTrain;

        switch (pstUser->m_atbSelfAlAction[udwIdx].m_nSclass)
        {
        case EN_ACTION_SEC_CLASS__TROOP:
        case EN_ACTION_SEC_CLASS__HOS_TREAT:
            if (0 < stTrainParam.m_ddwNum)
            {
                if (m_TroopNum.find(stTrainParam.m_ddwType) == m_TroopNum.end())
                {
                    m_TroopNum[stTrainParam.m_ddwType] = 0;
                }
                m_TroopNum[stTrainParam.m_ddwType] += stTrainParam.m_ddwNum;
            }
            break;
        case EN_ACTION_SEC_CLASS__FORT:
        case EN_ACTION_SEC_CLASS__FORT_REPAIR:
            if (0 < stTrainParam.m_ddwNum)
            {
                if (m_FortNum.find(stTrainParam.m_ddwType) == m_FortNum.end())
                {
                    m_FortNum[stTrainParam.m_ddwType] = 0;
                }
                m_FortNum[stTrainParam.m_ddwType] += stTrainParam.m_ddwNum;
            }
            break;
        default:
            break;
        }
    }

    //医院 祭坛
    for (TUINT32 udwIdx = 0; udwIdx < EN_TROOP_TYPE__END; udwIdx++)// hospital(troop_wait)
    {
        if (0 < ptbCity->m_bHos_wait[0].m_addwNum[udwIdx])
        {
            if (m_TroopNum.find(udwIdx) == m_TroopNum.end())
            {
                m_TroopNum[udwIdx] = 0;
            }
            m_TroopNum[udwIdx] += ptbCity->m_bHos_wait[0].m_addwNum[udwIdx];
        }
    }

    TUINT64 uddwTotalMight = 0;
    TUINT32 udwUnitMight = 0;
    jsonRet["troop"] = Json::Value(Json::arrayValue);
    jsonRet["fort"] = Json::Value(Json::arrayValue);

    for (TUINT32 udwIdx = 0; udwIdx < udwTroopSize; ++udwIdx)
    {
        map<int, long>::iterator iter_troop = m_TroopNum.find(udwIdx);
        if (m_TroopNum.end() == iter_troop)
        {
            jsonRet["troop"].append(0);
        }
        else
        {
            udwUnitMight = poGameInfo->m_oJsonRoot["game_troop"][udwIdx]["a"]["a9"].asUInt();
            uddwTotalMight += udwUnitMight * iter_troop->second;
            jsonRet["troop"].append(iter_troop->second);
        }
    }

    for (TUINT32 udwIdx = 0; udwIdx < udwFortSize; ++udwIdx)
    {
        map<int, long>::iterator iter_fort = m_FortNum.find(udwIdx);
        if (m_FortNum.end() == iter_fort)
        {
            jsonRet["fort"].append(0);
        }
        else
        {
            udwUnitMight = poGameInfo->m_oJsonRoot["game_fort"][udwIdx]["a"]["a9"].asUInt();
            uddwTotalMight += udwUnitMight * iter_fort->second;
            jsonRet["fort"].append(iter_fort->second);
        }
    }

    return uddwTotalMight;
}

TUINT64 CCommonBase::GenLockIdByLockInfo(TUINT64 uddwKey, TINT32 dwType, TUINT32 udwSid)
{
    if (uddwKey == 0)
    {
        return 0;
    }
    TUINT64 uddwLockId = 0;
    switch (dwType)
    {
    case EN_LOCK_ID_TYPE__UID:  //[100,000,001,000, 1000,000,000,000]
        uddwLockId = USER_ID_LOCK_OFFSET + uddwKey;
        break;
    case EN_LOCK_ID_TYPE__AID:
        uddwLockId = ALLIANCE_ID_LOCK_OFFSET + uddwKey; //(1000,000,000,000, 1000 << 32]
        break;
    case EN_LOCK_ID_TYPE__TASK_ID:  //(1000 << 32, MAX]
        uddwLockId = TASK_ID_LOCK_OFFSET + uddwKey;
        break;
    case EN_LOCK_ID_TYPE__WILD:
        uddwLockId = udwSid * WILD_ID_LOCK_OFFSET + uddwKey;
        break;
    case EN_LOCK_ID_TYPE__THRONE:
        uddwLockId = udwSid * WILD_ID_LOCK_OFFSET + uddwKey; // throne 的key值为1
    default:
        break;
    }
    return uddwLockId;
}

TBOOL CCommonBase::IsSpUiType(TUINT32 *audwTypeNum, TINT32 *adwListTypeNum, TUINT32 udwNum)
{
    for(TUINT32 udwIdx = 0; udwIdx < udwNum; ++udwIdx)
    {
        if(adwListTypeNum[udwIdx] == -1 && audwTypeNum[udwIdx] != 0)
        {
            return FALSE;
        }
        if(adwListTypeNum[udwIdx] == 0 && audwTypeNum[udwIdx] != 1)
        {
            return FALSE;
        }
        if(adwListTypeNum[udwIdx] == 1 && audwTypeNum[udwIdx] <= 1)
        {
            return FALSE;
        }
    }
    return TRUE;
}

TBOOL CCommonBase::IsNormalUiType(TUINT32 *audwTypeNum, TINT32 *adwListTypeNum, TUINT32 udwNum)
{
    for(TUINT32 udwIdx = 0; udwIdx < udwNum; ++udwIdx)
    {
        if(audwTypeNum[udwIdx] == 0)
        {
            continue;
        }
        if(audwTypeNum[udwIdx] >= 1 && adwListTypeNum[udwIdx] == -1)
        {
            return FALSE;
        }
        
    }
    return TRUE;
}

TINT32 CCommonBase::UpdateResourceHelpInfo(SUserInfo *pstUser, TbMarch_action *ptbTransportMarch, TUINT8 ucAssistType)
{
    TINT32 idx = 0;
    TbAl_assist *pstHelp = NULL;
    TINT32 dwHelpIdx = -1;
    SActionMarchParam *pstTransport = &ptbTransportMarch->m_bParam[0];

    // 查找联盟请求
    for(idx = 0; idx < static_cast<TINT32>(pstUser->m_udwAlAssistAllNum); idx++)
    {
        if(pstUser->m_atbAlAssistAll[idx].m_nType == ucAssistType && pstUser->m_atbAlAssistAll[idx].m_nUid == pstUser->m_tbPlayer.m_nUid)
        {
            pstHelp = &pstUser->m_atbAlAssistAll[idx];
            dwHelpIdx = idx;
            break;
        }
    }
    if(pstHelp == NULL)
    {
        return 0;
    }

    if(ptbTransportMarch->m_nBtime < pstHelp->m_nTime
    || ptbTransportMarch->m_nBtime > pstHelp->m_nTime + CCommonBase::GetGameBasicVal(EN_GAME_BASIC_ASSIST_AVAILABLE_TIME))
    {
        return 0;
    }

    // 查找成功更新请求信息
    for(idx = EN_RESOURCE_TYPE__GOLD; idx < EN_RESOURCE_TYPE__END; idx++)
    {
        pstHelp->m_bProgress[0].m_addwNum[idx] += pstTransport->m_stResource.m_addwNum[idx];
        pstHelp->SetFlag(TbAL_ASSIST_FIELD_PROGRESS);
    }

    // 更新联盟请求的标记
    pstUser->m_aucAlAssistAllFlag[dwHelpIdx] = EN_TABLE_UPDT_FLAG__DEL;
    for(idx = EN_RESOURCE_TYPE__GOLD; idx < EN_RESOURCE_TYPE__END; idx++)
    {
        if(pstHelp->m_bProgress[0].m_addwNum[idx] < pstHelp->m_bParam[0].m_addwNum[idx])
        {
            pstUser->m_aucAlAssistAllFlag[dwHelpIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
            break;
        }
    }

    return 0;
}

TINT64 CCommonBase::GetGameBasicVal(TINT32 dwKey)
{
    CGameInfo *pstGameInfo = CGameInfo::GetInstance();
    return pstGameInfo->m_oJsonRoot["game_basic"][dwKey].asInt64();
}

TBOOL CCommonBase::IsUnlockTroop(TINT32 dwResearchId)
{
    TBOOL bIsUnlockTroop = FALSE;
    switch(dwResearchId)
    {
    case 18:
    case 19:
    case 20:
    case 24:
    case 25:
    case 36:
    case 37:
    case 38:
    case 42:
    case 43:
    case 54:
    case 55:
    case 56:
    case 60:
    case 61:
    case 73:
    case 74:
    case 75:
        bIsUnlockTroop = TRUE;
        break;
    default:
        break;
    }
    return bIsUnlockTroop;
}

TINT32 CCommonBase::AddRewardWindow(SUserInfo *pstUser, TINT64 ddwTargetId, TUINT32 udwRewardType, TUINT32 udwRewardGetType, TINT64 ddwShowTime, SGlobalRes *pstReward, TBOOL bIsNeedPut, const Json::Value& jInfo)
{
    if (pstUser->m_dwRewardWindowNum >= MAX_REWARD_WINDOW_NUM)
    {
        return -1;
    }

    pstUser->m_atbRewardWindow[pstUser->m_dwRewardWindowNum].Reset();

    pstUser->m_atbRewardWindow[pstUser->m_dwRewardWindowNum].Set_Uid(ddwTargetId);
    pstUser->m_atbRewardWindow[pstUser->m_dwRewardWindowNum].Set_Id(CTimeUtils::GetCurTimeUs() / 1000);
    pstUser->m_atbRewardWindow[pstUser->m_dwRewardWindowNum].Set_Type(udwRewardType);
    pstUser->m_atbRewardWindow[pstUser->m_dwRewardWindowNum].Set_Get_type(udwRewardGetType);
    if (ddwShowTime <= pstUser->m_atbRewardWindow[pstUser->m_dwRewardWindowNum].m_nId / 1000)
    {
        ddwShowTime = 0;
    }
    else if (ddwShowTime - pstUser->m_atbRewardWindow[pstUser->m_dwRewardWindowNum].m_nId / 1000 > MAX_DELAY_TIME_REWARD_WINDOW)
    {
        ddwShowTime = pstUser->m_atbRewardWindow[pstUser->m_dwRewardWindowNum].m_nId / 1000 + MAX_DELAY_TIME_REWARD_WINDOW;
    }
    pstUser->m_atbRewardWindow[pstUser->m_dwRewardWindowNum].Set_Show_time(ddwShowTime);

    Json::Value jReward = Json::Value(Json::arrayValue);
    for (TUINT32 udwIdx = 0; udwIdx < pstReward->ddwTotalNum; udwIdx++)
    {
        jReward[udwIdx] = Json::Value(Json::arrayValue);
        jReward[udwIdx].append(pstReward->aRewardList[udwIdx].ddwType);
        jReward[udwIdx].append(pstReward->aRewardList[udwIdx].ddwId);
        jReward[udwIdx].append(pstReward->aRewardList[udwIdx].ddwNum);
    }

    pstUser->m_atbRewardWindow[pstUser->m_dwRewardWindowNum].Set_Reward(jReward);
    pstUser->m_atbRewardWindow[pstUser->m_dwRewardWindowNum].Set_Info(jInfo);

    if (bIsNeedPut)
    {
        pstUser->m_aucRewardWindowFlag[pstUser->m_dwRewardWindowNum] = EN_TABLE_UPDT_FLAG__NEW;
    }

    pstUser->m_dwRewardWindowNum++;

    return 0;
}

TINT32 CCommonBase::AddRewardWindow(SUserInfo *pstUser, TINT64 ddwTargetId, TUINT32 udwRewardType, TUINT32 udwRewardGetType, TINT64 ddwShowTime, SSpGlobalRes *pstReward, TBOOL bIsNeedPut, const Json::Value& jInfo)
{
    if (pstUser->m_dwRewardWindowNum >= MAX_REWARD_WINDOW_NUM)
    {
        return -1;
    }

    pstUser->m_atbRewardWindow[pstUser->m_dwRewardWindowNum].Reset();

    pstUser->m_atbRewardWindow[pstUser->m_dwRewardWindowNum].Set_Uid(ddwTargetId);
    pstUser->m_atbRewardWindow[pstUser->m_dwRewardWindowNum].Set_Id(CTimeUtils::GetCurTimeUs() / 1000);
    pstUser->m_atbRewardWindow[pstUser->m_dwRewardWindowNum].Set_Type(udwRewardType);
    pstUser->m_atbRewardWindow[pstUser->m_dwRewardWindowNum].Set_Get_type(udwRewardGetType);
    if (ddwShowTime <= pstUser->m_atbRewardWindow[pstUser->m_dwRewardWindowNum].m_nId / 1000)
    {
        ddwShowTime = 0;
    }
    else if (ddwShowTime - pstUser->m_atbRewardWindow[pstUser->m_dwRewardWindowNum].m_nId / 1000 > MAX_DELAY_TIME_REWARD_WINDOW)
    {
        ddwShowTime = pstUser->m_atbRewardWindow[pstUser->m_dwRewardWindowNum].m_nId / 1000 + MAX_DELAY_TIME_REWARD_WINDOW;
    }
    pstUser->m_atbRewardWindow[pstUser->m_dwRewardWindowNum].Set_Show_time(ddwShowTime);

    Json::Value jReward = Json::Value(Json::arrayValue);
    for (TUINT32 udwIdx = 0; udwIdx < pstReward->udwTotalNum; udwIdx++)
    {
        jReward[udwIdx] = Json::Value(Json::arrayValue);
        jReward[udwIdx].append(pstReward->aRewardList[udwIdx].udwType);
        jReward[udwIdx].append(pstReward->aRewardList[udwIdx].udwId);
        jReward[udwIdx].append(pstReward->aRewardList[udwIdx].udwNum);
    }

    pstUser->m_atbRewardWindow[pstUser->m_dwRewardWindowNum].Set_Reward(jReward);
    pstUser->m_atbRewardWindow[pstUser->m_dwRewardWindowNum].Set_Info(jInfo);

    if (bIsNeedPut)
    {
        pstUser->m_aucRewardWindowFlag[pstUser->m_dwRewardWindowNum] = EN_TABLE_UPDT_FLAG__NEW;
    }

    pstUser->m_dwRewardWindowNum++;

    return 0;
}

TINT32 CCommonBase::UpdatePlayerStatusOnSpecailWild(SUserInfo *pstUser)
{
    if (pstUser->m_tbPlayer.m_nStatus & EN_CITY_STATUS__AVOID_WAR)
    {
        pstUser->m_tbPlayer.Set_Status(pstUser->m_tbPlayer.m_nStatus & (~EN_CITY_STATUS__AVOID_WAR));
        // 删除peacetime的action
        TbAction *pstAction = CActionBase::GetActionByBufferId(pstUser->m_atbAction, pstUser->m_udwActionNum, EN_BUFFER_INFO_PEACE_TIME);
        if (pstAction)
        {
            TINT32 dwActionIdx = CActionBase::GetActionIndex(pstUser->m_atbAction, pstUser->m_udwActionNum, pstAction->m_nId);
            if (dwActionIdx >= 0)
            {
                pstUser->m_aucActionFlag[dwActionIdx] = EN_TABLE_UPDT_FLAG__DEL;
            }
        }
    }
    if (!(pstUser->m_tbPlayer.m_nStatus & EN_CITY_STATUS__ON_SPECIAL_WILD))
    {
        pstUser->m_tbPlayer.Set_Status(pstUser->m_tbPlayer.m_nStatus | EN_CITY_STATUS__ON_SPECIAL_WILD);
    }

    return 0;
}

TVOID CCommonBase::FindTrialOpenTime(SUserInfo *pstUser, TUINT32 udwTime, TUINT32& udwOpen, TUINT32& udwBtime, TUINT32& udwEtime)
{
    udwOpen = 0;
    udwBtime = 0;
    udwEtime = 0;

    const Json::Value &jControl = CDragonTrailControl::GetInstance()->m_oJsonRoot["dragon_trail_control"];

    for (TUINT32 udwIdx = 0; udwIdx < jControl.size(); ++udwIdx)
    {
        if (udwTime >= jControl[udwIdx]["forcast_time"].asUInt() && udwTime < jControl[udwIdx]["end_time"].asUInt())
        {
            udwBtime = jControl[udwIdx]["begin_time"].asUInt();
            udwEtime = jControl[udwIdx]["end_time"].asUInt();
            if (udwTime >= jControl[udwIdx]["begin_time"].asUInt())
            {
                udwOpen = 2; // 进行中
            }
            else
            {
                udwOpen = 1; // 预告
            }
        }
    }

    if (pstUser->m_tbPlayer.m_nDragon_level == 0)
    {
        udwOpen = 0;
        udwBtime = 0;
        udwEtime = 0;
    }
}

TVOID CCommonBase::GetTrialGift(TbPlayer *ptbPlayer, TUINT32 udwTime, TUINT32& udwHasGift, TUINT32& udwEndTime, SOneGlobalRes& sReward)
{
    udwHasGift = 0;
    udwEndTime = 0;
    sReward.Reset();

    const Json::Value &jGift = CGlobalGift::GetInstance()->m_oJsonRoot["global_gift"];

    TUINT32 udwLatestEtime = 0;
    for (TUINT32 udwIdx = 0; udwIdx < jGift.size(); ++udwIdx)
    {
        if (jGift[udwIdx]["end_time"].asUInt() == ptbPlayer->m_nTrial_gift_last_etime)
        {
            continue;
        }
        if (udwTime >= jGift[udwIdx]["begin_time"].asUInt() && udwTime <= jGift[udwIdx]["end_time"].asUInt())
        {
            if (jGift[udwIdx]["end_time"].asUInt() > udwLatestEtime)
            {
                udwHasGift = 1;
                udwEndTime = jGift[udwIdx]["end_time"].asUInt();
                sReward.ddwType = jGift[udwIdx]["reward"][0U].asInt();
                sReward.ddwId = jGift[udwIdx]["reward"][1U].asInt();
                sReward.ddwNum = jGift[udwIdx]["reward"][2U].asInt();
                udwLatestEtime = jGift[udwIdx]["end_time"].asUInt();
            }
        }
    }
}

TVOID CCommonBase::UpdateOccupyLairStat(SUserInfo *pstUser, TbMap *ptbWild, TINT64 ddwExceptActionId)
{
    TINT32 dwOccupyIdx = -1;
    TUINT32 udwOccupyNum = 0;

    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
    {
        TbMarch_action *ptbAction = &pstUser->m_atbMarch[udwIdx];
        if (ptbAction->m_nMclass == EN_ACTION_MAIN_CLASS__MARCH && ptbAction->m_nSclass == EN_ACTION_SEC_CLASS__OCCUPY &&
            ptbAction->m_nTpos == ptbWild->m_nId && ptbAction->m_nId != ddwExceptActionId &&
            (ptbAction->m_nStatus == EN_MARCH_STATUS__PRE_LOADING || ptbAction->m_nStatus == EN_MARCH_STATUS__LOADING))
        {
            if (dwOccupyIdx == -1)
            {
                dwOccupyIdx = udwIdx;
            }
            else
            {
                TINT64 ddwDraLv = ptbAction->m_bParam[0].m_stDragon.m_ddwLevel;
                TINT64 ddwKniLv = ptbAction->m_bParam[0].m_stKnight.ddwLevel;
                TINT64 ddwDraLvMax = pstUser->m_atbMarch[dwOccupyIdx].m_bParam[0].m_stDragon.m_ddwLevel;
                TINT64 ddwKniLvMax = pstUser->m_atbMarch[dwOccupyIdx].m_bParam[0].m_stKnight.ddwLevel;
                if (ddwDraLv > ddwDraLvMax || (ddwDraLv == ddwDraLvMax && ddwKniLv > ddwKniLvMax))
                {
                    dwOccupyIdx = udwIdx;
                }
            }

            ++udwOccupyNum;
        }
    }

    // check map data
    if (ptbWild->m_nOccupy_num != udwOccupyNum)
    {
        ptbWild->Set_Occupy_num(udwOccupyNum);
    }

    if (dwOccupyIdx >= 0)
    {
        TbMarch_action *ptbAction = &pstUser->m_atbMarch[(TUINT32)dwOccupyIdx];

        ptbWild->Set_Uid(ptbAction->m_nSuid);
        ptbWild->Set_Uname(ptbAction->m_bParam[0].m_szSourceUserName);
    }
}

TVOID CCommonBase::GetRoadList(TUINT32 udwSourcePos, TUINT32 udwTargetPos, vector<TUINT32> &vecRoadList)
{
    if (udwSourcePos == udwTargetPos)
    {
        return;
    }

    TBOOL bLastSpecialFlag = CSpecailMap::bIsSpecialMap(udwSourcePos);
    TBOOL bSpecialFlag = 0;
    
    vecRoadList.push_back(udwSourcePos);
    TINT32 dwSourceX = udwSourcePos / MAP_X_Y_POS_COMPUTE_OFFSET;
    TINT32 dwSourceY = udwSourcePos % MAP_X_Y_POS_COMPUTE_OFFSET;
    TINT32 dwTargetX = udwTargetPos / MAP_X_Y_POS_COMPUTE_OFFSET;
    TINT32 dwTargetY = udwTargetPos % MAP_X_Y_POS_COMPUTE_OFFSET;


    if (abs(dwSourceY - dwTargetY) > abs(dwSourceX - dwTargetX))
    {//x相等，或者斜率的绝对值大于1，则从y轴上选择路径
        int sign = dwSourceY > dwTargetY ? -1 : 1;

        for (int y = dwSourceY; y != dwTargetY + sign; y += sign)//一直逼近目标位置
        {
            int tmp = (dwSourceX - dwTargetX) * y + dwSourceY * dwTargetX - dwSourceX * dwTargetY;
            float x1 = 1.0 * tmp / (dwSourceY - dwTargetY);
            int x = ceil(x1);//上取整
            if ((x + y) % 2 != 0)//合法坐标
            {
                x = floor(x1);
            }

            bSpecialFlag = CSpecailMap::bIsSpecialMap(x * MAP_X_Y_POS_COMPUTE_OFFSET + y);
            if (bSpecialFlag != bLastSpecialFlag)
            {
                vecRoadList.push_back(x * MAP_X_Y_POS_COMPUTE_OFFSET + y);
                bLastSpecialFlag = bSpecialFlag;
            }
        }
    }
    else
    {
        int sign = dwSourceX > dwTargetX ? -1 : 1;

        for (int x = dwSourceX; x != dwTargetX + sign; x += sign)//一直逼近目标位置
        {
            int tmp = (dwSourceY - dwTargetY) * x + dwSourceX * dwTargetY - dwSourceY * dwTargetX;
            float y1 = 1.0 * tmp / (dwSourceX - dwTargetX);
            int y = ceil(y1);//上取整
            if ((x + y) % 2 != 0)
            {
                y = floor(y1);
            }

            bSpecialFlag = CSpecailMap::bIsSpecialMap(x * MAP_X_Y_POS_COMPUTE_OFFSET + y);
            if (bSpecialFlag != bLastSpecialFlag)
            {
                vecRoadList.push_back(x * MAP_X_Y_POS_COMPUTE_OFFSET + y);
                bLastSpecialFlag = bSpecialFlag;
            }
        }
    }

    if (vecRoadList[vecRoadList.size() - 1] / 10 != udwTargetPos)
    {//终点未被加入，则加入终点
        bSpecialFlag = CSpecailMap::bIsSpecialMap(udwTargetPos);
        vecRoadList.push_back(udwTargetPos);
    }
}

TINT32 CCommonBase::GetTroopRallyWarTimeNew(SCommonTroop *pstTroop, TINT32 dwFrom, TINT32 dwTo, SPlayerBuffInfo *pstBuff)
{
    CGameInfo *poGameInfo = CGameInfo::GetInstance();

    TFLOAT32 fLowestSpeed = 0.0;
    TBOOL bFirst = TRUE;
    for (TUINT32 udwIdx = 0; udwIdx < poGameInfo->m_oJsonRoot["game_troop"].size(); ++udwIdx)
    {
        if (pstTroop->m_addwNum[udwIdx] == 0)
        {
            continue;
        }
        TINT32 dwSpeed = poGameInfo->m_oJsonRoot["game_troop"][udwIdx]["a"]["a3"].asInt();
        TINT32 dwBuff = 10000 + pstBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_RALLY_WAR_SPEED].m_ddwBuffTotal;

        for (TUINT32 udwIdy = 0; udwIdy < poGameInfo->m_oJsonRoot["game_troop"][udwIdx]["ab"]["speed"].size(); udwIdy++)
        {
            dwBuff += pstBuff->m_astPlayerBuffInfo[poGameInfo->m_oJsonRoot["game_troop"][udwIdx]["ab"]["speed"][udwIdy].asUInt()].m_ddwBuffTotal;
        }

        TFLOAT32 fSpeed = dwSpeed * (1.0 * dwBuff / 10000);
        if (bFirst || fLowestSpeed > fSpeed)
        {
            fLowestSpeed = fSpeed;
            bFirst = FALSE;
        }
    }

    if (fLowestSpeed < 0.000001)
    {
        return 0;
    }

    vector<TUINT32> vecRoadList;
    vecRoadList.clear();
    GetRoadList(dwFrom, dwTo, vecRoadList);

    TFLOAT32 fDistance = 0.0;

    float swamp_march_speed_buff = 10000.0 / GetGameBasicVal(EN_GAME_BASIC_DEBUFF_FOR_MARCH_SPEED_ON_SWAMP);
    bool has_swamp = false;
    for (int i = 1; i < vecRoadList.size(); i++)
    {
        TUINT32 udwTpos = vecRoadList[i];
        TUINT32 udwSpos = vecRoadList[i - 1];

        TINT32 x1 = udwSpos / MAP_X_Y_POS_COMPUTE_OFFSET;
        TINT32 y1 = udwSpos % MAP_X_Y_POS_COMPUTE_OFFSET;
        TINT32 x2 = udwTpos / MAP_X_Y_POS_COMPUTE_OFFSET;
        TINT32 y2 = udwTpos % MAP_X_Y_POS_COMPUTE_OFFSET;

        float cur_dis = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));

        if (CSpecailMap::bIsSpecialMap(udwSpos))
        {
            has_swamp = true;
            cur_dis *= swamp_march_speed_buff;
        }

        fDistance += cur_dis;
    }

    if (fLowestSpeed < 0.000001)
    {
        if (has_swamp)
        {
            return GetGameBasicVal(EN_GAME_BASIC_MIN_MARCH_TIME_ON_SWAMP);
        }
        else
        {
            return GetGameBasicVal(EN_GAME_BASIC_MIN_MARCH_TIME);
        }
    }

    TINT32 dwTime = ceil(fDistance / fLowestSpeed * GetGameBasicVal(EN_GAME_BASIC_MARCH_BASE_SPEED));

    if (has_swamp)
    {
        if (dwTime < GetGameBasicVal(EN_GAME_BASIC_MIN_MARCH_TIME_ON_SWAMP))
        {
            dwTime = GetGameBasicVal(EN_GAME_BASIC_MIN_MARCH_TIME_ON_SWAMP);
        }
    }
    else
    {
        if (dwTime < GetGameBasicVal(EN_GAME_BASIC_MIN_MARCH_TIME))
        {
            dwTime = GetGameBasicVal(EN_GAME_BASIC_MIN_MARCH_TIME);
        }
    }

    return dwTime;
}