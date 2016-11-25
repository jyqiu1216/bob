#include "process_report.h"
#include "session.h"
#include "aws_table_include.h"
#include "time_utils.h"
#include "city_base.h"
#include "common_func.h"
#include "common_json.h"
#include "war_base.h"
#include "tool_base.h"
#include "common_base.h"
#include "map_logic.h"
#include "game_svr.h"
#include "buffer_base.h"
#include "player_base.h"
#include "document.h"
#include "wild_info.h"

TINT32 CProcessReport::GenAttackReport(SSession *pstSession, TINT32 dwReportType, TINT32 dwWarResult, SBattleNode *pstAttacker, SBattleNode* pstDefender, TbReport* ptbReport)
{
    TbMarch_action* ptbMarch = &pstSession->m_stReqMarch;
    SActionMarchParam *pstMarchParam = &ptbMarch->m_bParam.m_astList[0];
    SUserInfo* pstTargetUser = &pstSession->m_stTargetUser;
    TbMap *ptbWild = &pstSession->m_stMapItem;

    CProcessReport::GenReportFlag(pstSession, ptbReport);
    ptbReport->Set_Type(dwReportType);
    ptbReport->Set_Result(dwWarResult);
    ptbReport->Set_Time(CTimeUtils::GetUnixTime());
    ptbReport->Set_Sid(pstSession->m_udwReqSvrId);

    CProcessReport::GenFromTo(ptbReport, ptbMarch, ptbMarch->m_nSid);

    Json::Value jsonContent = Json::Value(Json::objectValue);
    jsonContent["scoutlv"] = pstMarchParam->m_ddwScoutLevel;//static_cast<TINT32>(ptbAction->m_bMarch_param[0].m_ucScoutLevel);

    jsonContent["attacker_series_id"] = pstAttacker->m_udwSeriesId;
    jsonContent["defender_series_id"] = pstDefender->m_udwSeriesId;

    jsonContent["attacker_avatar"] = pstAttacker->m_ptbPlayer->m_nAvatar;
    jsonContent["defender_avatar"] = pstDefender->m_ptbPlayer->m_nAvatar;

    CProcessReport::GenMajorPlayer(pstAttacker, ptbMarch->m_nId, jsonContent["attacker"]);
    TINT64 ddwMajorDefenderId = CProcessReport::GetDefenderMajorId(pstDefender, ptbMarch, ptbWild, pstTargetUser);
    CProcessReport::GenMajorPlayer(pstDefender, ddwMajorDefenderId, jsonContent["defender"]);

    CProcessReport::GenAllBattlePlayer(pstAttacker, jsonContent["all_attacker"]);
    CProcessReport::GenAllBattlePlayer(pstDefender, jsonContent["all_defender"]);

    CCommJson::GenResourceJson(&pstAttacker->m_armys.resource, jsonContent["resource"]);
    CProcessReport::GenRewardInfo(pstSession, jsonContent);

    jsonContent["attacker_reinforce"] = Json::Value(Json::arrayValue);
    TUINT32 udwJsonIndex = 0;
    for(TUINT32 udwIdx = 0; udwIdx < pstAttacker->m_udwRallyReinforceNum; ++udwIdx)
    {
        CProcessReport::GenReinforcePlayer(pstAttacker, pstAttacker->m_pastRallyReinforceList[udwIdx], jsonContent["attacker_reinforce"][udwJsonIndex]);
        ++udwJsonIndex;
    }

    jsonContent["defender_reinforce"] = Json::Value(Json::arrayValue);
    if(pstTargetUser->m_tbPlayer.m_nUid > 0)
    {
        TUINT32 udwJsonIndex = 0;

        for(TUINT32 udwIdx = 0; udwIdx < pstDefender->m_udwEncampNum; udwIdx++)
        {
            if(pstDefender->m_pastEncampActionList[udwIdx]->m_nSclass == EN_ACTION_SEC_CLASS__OCCUPY)
            {
                if (CMapLogic::GetWildClass(ptbWild->m_nSid, ptbWild->m_nType) != EN_WILD_CLASS_MONSTER_NEST)
                {
                    continue;
                }
                if (CMapLogic::GetWildClass(ptbWild->m_nSid, ptbWild->m_nType) == EN_WILD_CLASS_MONSTER_NEST &&
                    ddwMajorDefenderId == pstDefender->m_pastEncampActionList[udwIdx]->m_nId)
                {
                    continue;
                }
            }
            if (pstDefender->m_pastEncampActionList[udwIdx]->m_nSclass == EN_ACTION_SEC_CLASS__CAMP)
            {
                continue;
            }

            CProcessReport::GenReinforcePlayer(pstDefender, pstDefender->m_pastEncampActionList[udwIdx], jsonContent["defender_reinforce"][udwJsonIndex]);
            ++udwJsonIndex;
        }

        for(TUINT32 udwIdx = 0; udwIdx < pstDefender->m_udwRallyReinforceNum; ++udwIdx)
        {
            CProcessReport::GenReinforcePlayer(pstDefender, pstDefender->m_pastRallyReinforceList[udwIdx], jsonContent["defender_reinforce"][udwJsonIndex]);
            ++udwJsonIndex;
        }
    }

    Json::FastWriter jsonWriter;
    ptbReport->Set_Content(jsonWriter.write(jsonContent));

    //已经在打战处理中设置了接收者

    return 0;
}

TINT32 CProcessReport::GenTransportReport(SSession* pstSession, TbReport* ptbReport)
{
    TbMarch_action* ptbMarch = &pstSession->m_stReqMarch;

    CProcessReport::GenReportFlag(pstSession, ptbReport);
    ptbReport->Set_Type(EN_REPORT_TYPE_TRANSPORT);
    ptbReport->Set_Result(EN_REPORT_RESULT_WIN);
    ptbReport->Set_Time(CTimeUtils::GetUnixTime());
    ptbReport->Set_Sid(pstSession->m_udwReqSvrId);

    CProcessReport::GenFromTo(ptbReport, ptbMarch, ptbMarch->m_nSid);
    Json::Value jsonContent = Json::Value(Json::objectValue);

    CProcessReport::GenMajorPlayer(ptbMarch, jsonContent["attacker"]);

    CCommJson::GenResourceJson(&ptbMarch->m_bParam[0].m_stResource, jsonContent["resource"]);
    CProcessReport::GenRewardInfo(pstSession, jsonContent);

    Json::FastWriter oTmpWriter;
    ptbReport->Set_Content(oTmpWriter.write(jsonContent));

    CProcessReport::AddReceiver(ptbMarch->m_nSuid, &pstSession->m_stSourceUser);
    CProcessReport::AddReceiver(ptbMarch->m_nTuid, &pstSession->m_stTargetUser);

    return 0;
}

TINT32 CProcessReport::GenReinforceReport(SSession* pstSession, TbReport* ptbReport)
{
    CProcessReport::GenTransportReport(pstSession, ptbReport);
    ptbReport->Set_Type(EN_REPORT_TYPE_REINFORCE);
    return 0;
}

TINT32 CProcessReport::GenThroneReinforceReport(SSession* pstSession, TbReport* ptbReport)
{
    CProcessReport::GenOccupyReport(pstSession, ptbReport);
    ptbReport->Set_Type(EN_REPORT_TYPE_THRONE_REINFORCE);
    return 0;
}

TINT32 CProcessReport::GenThroneStatusReport(SSession* pstSession, TINT32 dwReportType, const Json::Value &jContent, TbReport* ptbReport)
{
    TbMarch_action* ptbMarch = &pstSession->m_stReqMarch;

    ptbReport->Set_Type(dwReportType);
    ptbReport->Set_Result(EN_REPORT_RESULT_WIN);
    ptbReport->Set_Time(CTimeUtils::GetUnixTime());
    ptbReport->Set_Sid(pstSession->m_udwReqSvrId);

    CProcessReport::GenFromTo(ptbReport, ptbMarch, ptbMarch->m_nSid);

    Json::FastWriter oTmpWriter;
    ptbReport->Set_Content(oTmpWriter.write(jContent));

    return 0;
}

TINT32 CProcessReport::GenOccupyReport(SSession* pstSession, TbReport* ptbReport)
{
    TbMarch_action* ptbMarch = &pstSession->m_stReqMarch;

    CProcessReport::GenReportFlag(pstSession, ptbReport);
    ptbReport->Set_Type(EN_REPORT_TYPE_OCCUPY);
    ptbReport->Set_Result(EN_REPORT_RESULT_WIN);
    ptbReport->Set_Time(CTimeUtils::GetUnixTime());
    ptbReport->Set_Sid(pstSession->m_udwReqSvrId);

    CProcessReport::GenFromTo(ptbReport, ptbMarch, ptbMarch->m_nSid);
    Json::Value jsonContent = Json::Value(Json::objectValue);

    CProcessReport::GenMajorPlayer(ptbMarch, jsonContent["attacker"]);

    CCommJson::GenResourceJson(&ptbMarch->m_bParam[0].m_stResource, jsonContent["resource"]);
    CProcessReport::GenRewardInfo(pstSession, jsonContent);

    Json::FastWriter oTmpWriter;
    ptbReport->Set_Content(oTmpWriter.write(jsonContent));

    CProcessReport::AddReceiver(ptbMarch->m_nSuid, &pstSession->m_stSourceUser);

    return 0;
}

TINT32 CProcessReport::GenCampReport(SSession* pstSession, TbReport* ptbReport)
{
    TbMarch_action* ptbMarch = &pstSession->m_stReqMarch;

    CProcessReport::GenReportFlag(pstSession, ptbReport);
    ptbReport->Set_Type(EN_REPORT_TYPE_CAMP);
    ptbReport->Set_Result(EN_REPORT_RESULT_WIN);
    ptbReport->Set_Time(CTimeUtils::GetUnixTime());
    ptbReport->Set_Sid(pstSession->m_udwReqSvrId);

    CProcessReport::GenFromTo(ptbReport, ptbMarch, ptbMarch->m_nSid);
    Json::Value jsonContent = Json::Value(Json::objectValue);

    CProcessReport::GenMajorPlayer(ptbMarch, jsonContent["attacker"]);

    Json::FastWriter oTmpWriter;
    ptbReport->Set_Content(oTmpWriter.write(jsonContent));

    CProcessReport::AddReceiver(ptbMarch->m_nSuid, &pstSession->m_stSourceUser);

    return 0;
}

TINT32 CProcessReport::GenScoutCityReport(SSession* pstSession, TbReport* ptbReport)
{
    TbMarch_action* ptbMarch = &pstSession->m_stReqMarch;
    SUserInfo* pstTargetUser = &pstSession->m_stTargetUser;
    SCityInfo* pstCity = &pstTargetUser->m_stCityInfo;
    TbCity* ptbCity = NULL;
    if(pstCity)
    {
        ptbCity = &pstCity->m_stTblData;
    }

    CProcessReport::GenReportFlag(pstSession, ptbReport);
    ptbReport->Set_Type(EN_REPORT_TYPE_SCOUT);
    ptbReport->Set_Result(EN_REPORT_RESULT_WIN);
    ptbReport->Set_Time(CTimeUtils::GetUnixTime());
    ptbReport->Set_Sid(pstSession->m_udwReqSvrId);

    CProcessReport::GenFromTo(ptbReport, ptbMarch, ptbMarch->m_nSid);
    Json::Value jsonContent = Json::Value(Json::objectValue);

    jsonContent["scoutlv"] = ptbMarch->m_bParam[0].m_ddwScoutLevel;
    jsonContent["info"] = Json::Value(Json::objectValue);
    jsonContent["info"]["series_id"] = 0;
    if(ptbCity)
    {
        CCommJson::GenResourceJson(&ptbCity->m_bResource[0], jsonContent["info"]["resource"]);
        CCommJson::GenReportBuildingJson(&ptbCity->m_bBuilding[0], ptbCity->m_bBuilding.m_udwNum, jsonContent["info"]["building"]);
        CCommJson::GenTroopJson(&ptbCity->m_bTroop[0], jsonContent["info"]["troop"]);
        CCommJson::GenFortJson(&ptbCity->m_bFort[0], jsonContent["info"]["fort"]);

        jsonContent["info"]["knight"] = Json::Value(Json::arrayValue);
        TUINT32 udwKnightIdx = 0;
        TUINT32 udwKnightPosNum = 0;
        for (udwKnightIdx = 0; udwKnightIdx < pstCity->m_stTblData.m_bKnight.m_udwNum; udwKnightIdx++)
        {
            if (pstCity->m_stTblData.m_bKnight[udwKnightIdx].ddwPos != EN_KNIGHT_POS__UNASSIGN)
            {
                jsonContent["info"]["knight"][udwKnightPosNum].append(udwKnightIdx);
                jsonContent["info"]["knight"][udwKnightPosNum].append(CGameInfo::GetInstance()->ComputeKnightLevelByExp(pstCity->m_stTblData.m_bKnight[udwKnightIdx].ddwExp));
                jsonContent["info"]["knight"][udwKnightPosNum].append(0);
                jsonContent["info"]["knight"][udwKnightPosNum].append(pstCity->m_stTblData.m_bKnight[udwKnightIdx].ddwPos);
                udwKnightPosNum++;
            }
        }

        jsonContent["info"]["dragon_status"] = pstTargetUser->m_tbPlayer.m_nDragon_status;
        TUINT32 udwDragonLv = pstTargetUser->m_tbPlayer.m_nDragon_status == EN_DRAGON_STATUS_NORMAL ? pstTargetUser->m_tbPlayer.m_nDragon_level : 0;
        jsonContent["info"]["dragon"] = Json::Value(Json::arrayValue);
        jsonContent["info"]["dragon"].append(pstTargetUser->m_tbPlayer.m_sDragon_name);
        jsonContent["info"]["dragon"].append(udwDragonLv);
        jsonContent["info"]["dragon"].append(pstTargetUser->m_tbPlayer.m_nDragon_avatar);
        jsonContent["info"]["dragon"].append(0);

        jsonContent["info"]["battle_buff"] = Json::Value(Json::arrayValue);
        for(TUINT32 udwIdx = 0, udwJsonIndex = 0; udwIdx < EN_BUFFER_INFO_END; ++udwIdx)
        {
            TINT32 dwBufferId = pstTargetUser->m_stPlayerBuffList[udwIdx].m_udwBuffId;
            TINT64 ddwBuffNum = pstTargetUser->m_stPlayerBuffList[udwIdx].m_ddwBuffTotal;
            if(ddwBuffNum > 0)
            {
                if(!CToolBase::IsScoutShowBuff(dwBufferId))
                {
                    continue;
                }
                jsonContent["info"]["battle_buff"][udwJsonIndex] = Json::Value(Json::arrayValue);
                jsonContent["info"]["battle_buff"][udwJsonIndex].append(dwBufferId);
                jsonContent["info"]["battle_buff"][udwJsonIndex].append(ddwBuffNum);
                ++udwJsonIndex;
            }
        }

        jsonContent["info"]["defender_reinforce"] = Json::Value(Json::arrayValue);
        for(TUINT32 udwIdx = 0, udwJsonIndex = 0; udwIdx < pstTargetUser->m_udwPassiveMarchNum; udwIdx++)
        {
            TbMarch_action* ptbReinforceAction = &pstTargetUser->m_atbPassiveMarch[udwIdx];
            if(ptbReinforceAction->m_nTpos == ptbMarch->m_nTpos &&
                ptbReinforceAction->m_nMclass == EN_ACTION_MAIN_CLASS__MARCH &&
                ptbReinforceAction->m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_NORMAL &&
                ptbReinforceAction->m_nStatus == EN_MARCH_STATUS__DEFENDING)
            {
                CProcessReport::GenReinforcePlayer(ptbReinforceAction, jsonContent["info"]["defender_reinforce"][udwJsonIndex]);
                ++udwJsonIndex;
            }
        }
    }

    Json::FastWriter oTmpWriter;
    ptbReport->Set_Content(oTmpWriter.write(jsonContent));

    CProcessReport::AddReceiver(ptbMarch->m_nSuid, &pstSession->m_stSourceUser);
    if(pstSession->m_stSourceUser.m_tbPlayer.m_nAlid > 0 && pstSession->m_stSourceUser.m_tbPlayer.m_nAlpos > EN_ALLIANCE_POS__REQUEST)
    {
        CProcessReport::AddReceiver(CToolBase::GetAllianceUserReportKey(pstSession->m_stSourceUser.m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET),
            &pstSession->m_stSourceUser);
    }
    CProcessReport::AddReceiver(ptbMarch->m_nTuid, &pstSession->m_stTargetUser);
    if(pstSession->m_stTargetUser.m_tbPlayer.m_nAlid > 0 && pstSession->m_stTargetUser.m_tbPlayer.m_nAlpos > EN_ALLIANCE_POS__REQUEST)
    {
        CProcessReport::AddReceiver(CToolBase::GetAllianceUserReportKey(pstSession->m_stTargetUser.m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET),
            &pstSession->m_stTargetUser);
    }

    return 0;
}

TINT32 CProcessReport::GenScoutThroneReport(SSession* pstSession, TbReport* ptbReport)
{
    TbMarch_action* ptbSourceMarch = &pstSession->m_stReqMarch;
    SUserInfo* pstTargetUser = &pstSession->m_stTargetUser;
    TbMap* ptbMap = &pstSession->m_stMapItem;

    CProcessReport::GenReportFlag(pstSession, ptbReport);
    ptbReport->Set_Type(EN_REPORT_TYPE_THRONE_SCOUT);
    ptbReport->Set_Result(EN_REPORT_RESULT_WIN);
    ptbReport->Set_Time(CTimeUtils::GetUnixTime());
    ptbReport->Set_Sid(pstSession->m_udwReqSvrId);

    CProcessReport::GenFromTo(ptbReport, ptbSourceMarch, ptbSourceMarch->m_nSid);
    Json::Value jsonContent = Json::Value(Json::objectValue);

    jsonContent["scoutlv"] = ptbSourceMarch->m_bParam[0].m_ddwScoutLevel;
    jsonContent["info"] = Json::Value(Json::objectValue);
    jsonContent["info"]["series_id"] = 0;
    if(pstTargetUser->m_tbPlayer.m_nUid > 0)
    {
        TBOOL bIsDragonJoin = FALSE;
        TUINT32 udwKnightLv = 0;
        TbMarch_action* ptbThroneAssign = NULL;
        for(TUINT32 udwIdx = 0; udwIdx < pstTargetUser->m_udwMarchNum; ++udwIdx)
        {
            if(pstTargetUser->m_aucMarchFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
            {
                continue;
            }
            ptbThroneAssign = &pstTargetUser->m_atbMarch[udwIdx];
            if(ptbThroneAssign->m_nSclass != EN_ACTION_SEC_CLASS__ASSIGN_THRONE
                || ptbThroneAssign->m_nStatus != EN_MARCH_STATUS__DEFENDING
                || ptbThroneAssign->m_nTpos != ptbMap->m_nId)
            {
                continue;
            }

            if (ptbThroneAssign->m_bParam[0].m_stDragon.m_ddwLevel > 0)
            {
                bIsDragonJoin = TRUE;
            }
            udwKnightLv = ptbThroneAssign->m_bParam[0].m_stKnight.ddwLevel;

            jsonContent["info"]["dragon_status"] = EN_DRAGON_STATUS_MARCH;
            jsonContent["info"]["dragon"] = Json::Value(Json::arrayValue);
            jsonContent["info"]["dragon"].append(ptbThroneAssign->m_bParam[0].m_stDragon.m_szName);
            jsonContent["info"]["dragon"].append(ptbThroneAssign->m_bParam[0].m_stDragon.m_ddwLevel);
            jsonContent["info"]["dragon"].append(ptbThroneAssign->m_bParam[0].m_stDragon.m_ddwIconId);
            jsonContent["info"]["dragon"].append(0);

            jsonContent["info"]["knight"] = Json::Value(Json::arrayValue);
            jsonContent["info"]["knight"][0U] = Json::Value(Json::arrayValue);
            jsonContent["info"]["knight"][0U].append(ptbThroneAssign->m_bParam[0].m_stKnight.ddwId);
            jsonContent["info"]["knight"][0U].append(ptbThroneAssign->m_bParam[0].m_stKnight.ddwLevel);
            jsonContent["info"]["knight"][0U].append(0);
            jsonContent["info"]["knight"][0U].append(0);
        }

        jsonContent["info"]["battle_buff"] = Json::Value(Json::arrayValue);
        SPlayerBuffInfo *pstBuff = NULL;
        if (bIsDragonJoin)
        {
            pstBuff = &pstTargetUser->m_stPlayerBuffList;
        }
        else
        {
            pstBuff = &pstTargetUser->m_stBuffWithoutDragon;
        }

        SPlayerBuffInfo stTmpBuff;
        CBufferBase::GenBattleBuff(pstBuff, &stTmpBuff, FALSE, FALSE, udwKnightLv);
        for (TUINT32 udwIdx = 0, udwJsonIndex = 0; udwIdx < EN_BUFFER_INFO_END; ++udwIdx)
        {
            TINT32 dwBufferId = stTmpBuff[udwIdx].m_udwBuffId;
            TINT64 ddwBuffNum = stTmpBuff[udwIdx].m_ddwBuffTotal;
            if (ddwBuffNum > 0)
            {
                if (!CToolBase::IsScoutShowBuff(dwBufferId))
                {
                    continue;
                }
                jsonContent["info"]["battle_buff"][udwJsonIndex] = Json::Value(Json::arrayValue);
                jsonContent["info"]["battle_buff"][udwJsonIndex].append(dwBufferId);
                jsonContent["info"]["battle_buff"][udwJsonIndex].append(ddwBuffNum);
                ++udwJsonIndex;
            }
        }

        jsonContent["info"]["defender_reinforce"] = Json::Value(Json::arrayValue);
        for(TUINT32 udwIdx = 0, udwJsonIndex = 0; udwIdx < pstTargetUser->m_udwMarchNum; udwIdx++)
        {
            TbMarch_action* ptbReinforceAction = &pstTargetUser->m_atbMarch[udwIdx];
            if(ptbReinforceAction->m_nTpos == ptbMap->m_nId
                && ptbReinforceAction->m_nMclass == EN_ACTION_MAIN_CLASS__MARCH
                && (ptbReinforceAction->m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_THRONE
                || ptbReinforceAction->m_nSclass == EN_ACTION_SEC_CLASS__ASSIGN_THRONE)
                && ptbReinforceAction->m_nStatus == EN_MARCH_STATUS__DEFENDING)
            {
                CProcessReport::GenReinforcePlayer(ptbReinforceAction, jsonContent["info"]["defender_reinforce"][udwJsonIndex]);
                ++udwJsonIndex;
            }
        }
    }
    else
    {
    }

    Json::FastWriter oTmpWriter;
    ptbReport->Set_Content(oTmpWriter.write(jsonContent));

    CProcessReport::AddReceiver(ptbSourceMarch->m_nSuid, &pstSession->m_stSourceUser);
    if(pstSession->m_stSourceUser.m_tbPlayer.m_nAlid > 0 && pstSession->m_stSourceUser.m_tbPlayer.m_nAlpos > EN_ALLIANCE_POS__REQUEST)
    {
        CProcessReport::AddReceiver(CToolBase::GetAllianceUserReportKey(pstSession->m_stSourceUser.m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET),
            &pstSession->m_stSourceUser);
    }

    if(pstSession->m_stTargetUser.m_tbPlayer.m_nAlid > 0 && pstSession->m_stTargetUser.m_tbPlayer.m_nAlpos > EN_ALLIANCE_POS__REQUEST)
    {
        CProcessReport::AddReceiver(CToolBase::GetAllianceUserReportKey(pstSession->m_stTargetUser.m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET),
            &pstSession->m_stTargetUser);
    }

    return 0;
}

TINT32 CProcessReport::GenScoutIdolReport(SSession* pstSession, TbReport* ptbReport)
{
    TbMarch_action* ptbMarch = &pstSession->m_stReqMarch;
    TbMap *ptbWild = &pstSession->m_stMapItem;

    TbIdol *ptbIdol = NULL;
    for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_udwIdolNum; udwIdx++)
    {
        if (pstSession->m_atbIdol[udwIdx].m_nPos == ptbWild->m_nId)
        {
            ptbIdol = &pstSession->m_atbIdol[udwIdx];
            break;
        }
    }

    CProcessReport::GenReportFlag(pstSession, ptbReport);
    ptbReport->Set_Type(EN_REPORT_TYPE_IDOL_SCOUT);
    ptbReport->Set_Result(EN_REPORT_RESULT_WIN);
    ptbReport->Set_Time(CTimeUtils::GetUnixTime());
    ptbReport->Set_Sid(pstSession->m_udwReqSvrId);

    CProcessReport::GenFromTo(ptbReport, ptbMarch, ptbMarch->m_nSid);
    Json::Value jsonContent = Json::Value(Json::objectValue);

    jsonContent["scoutlv"] = ptbMarch->m_bParam[0].m_ddwScoutLevel;
    jsonContent["info"] = Json::Value(Json::objectValue);
    jsonContent["info"]["series_id"] = ptbIdol->m_jInfo["t"][2U].asInt();
    
    CCommJson::GenTroopJson(&ptbIdol->m_bTroop[0], jsonContent["info"]["troop"]);

    jsonContent["info"]["knight"] = Json::Value(Json::arrayValue);
    jsonContent["info"]["knight"][0U] = Json::Value(Json::arrayValue);
    jsonContent["info"]["knight"][0U].append(0);
    jsonContent["info"]["knight"][0U].append(ptbIdol->m_jInfo["t"][1U].asInt());
    jsonContent["info"]["knight"][0U].append(0);
    jsonContent["info"]["knight"][0U].append(0);

    jsonContent["info"]["dragon_status"] = EN_DRAGON_STATUS_NORMAL;
    jsonContent["info"]["dragon"] = Json::Value(Json::arrayValue);
    jsonContent["info"]["dragon"].append("");
    jsonContent["info"]["dragon"].append(ptbIdol->m_jInfo["t"][0U].asInt());
    jsonContent["info"]["dragon"].append(0);
    jsonContent["info"]["dragon"].append(0);

    SPlayerBuffInfo stBuff;
    stBuff.Reset();
    CBufferBase::GetIdolBattleBuff(ptbIdol, &stBuff);

    jsonContent["info"]["battle_buff"] = Json::Value(Json::arrayValue);
    for (TUINT32 udwIdx = 0, udwJsonIndex = 0; udwIdx < EN_BUFFER_INFO_END; ++udwIdx)
    {
        TINT32 dwBufferId = stBuff[udwIdx].m_udwBuffId;
        TINT64 ddwBuffNum = stBuff[udwIdx].m_ddwBuffTotal;
        if (ddwBuffNum > 0)
        {
            if (!CToolBase::IsScoutShowBuff(dwBufferId))
            {
                continue;
            }
            jsonContent["info"]["battle_buff"][udwJsonIndex] = Json::Value(Json::arrayValue);
            jsonContent["info"]["battle_buff"][udwJsonIndex].append(dwBufferId);
            jsonContent["info"]["battle_buff"][udwJsonIndex].append(ddwBuffNum);
            ++udwJsonIndex;
        }
    }

    Json::FastWriter oTmpWriter;
    ptbReport->Set_Content(oTmpWriter.write(jsonContent));

    CProcessReport::AddReceiver(ptbMarch->m_nSuid, &pstSession->m_stSourceUser);
    if (pstSession->m_stSourceUser.m_tbPlayer.m_nAlid > 0 && pstSession->m_stSourceUser.m_tbPlayer.m_nAlpos > EN_ALLIANCE_POS__REQUEST)
    {
        CProcessReport::AddReceiver(CToolBase::GetAllianceUserReportKey(pstSession->m_stSourceUser.m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET),
            &pstSession->m_stSourceUser);
    }

    return 0;
}

TINT32 CProcessReport::GenScoutWildReport(SSession* pstSession, TbReport* ptbReport)
{
    TbMarch_action* ptbSourceMarch = &pstSession->m_stReqMarch;
    SUserInfo* pstTargetUser = &pstSession->m_stTargetUser;
    TbMap* ptbMap = &pstSession->m_stMapItem;

    CProcessReport::GenReportFlag(pstSession, ptbReport);
    ptbReport->Set_Type(EN_REPORT_TYPE_SCOUT);
    ptbReport->Set_Result(EN_REPORT_RESULT_WIN);
    ptbReport->Set_Time(CTimeUtils::GetUnixTime());
    ptbReport->Set_Sid(pstSession->m_udwReqSvrId);

    CProcessReport::GenFromTo(ptbReport, ptbSourceMarch, ptbSourceMarch->m_nSid);
    Json::Value jsonContent = Json::Value(Json::objectValue);

    jsonContent["scoutlv"] = ptbSourceMarch->m_bParam[0].m_ddwScoutLevel;
    jsonContent["info"] = Json::Value(Json::objectValue);
    CCommJson::GenResourceJson(&ptbMap->m_bResource[0], jsonContent["info"]["resource"]);

    if(pstTargetUser->m_tbPlayer.m_nUid > 0)
    {
        TbMarch_action* ptbOccupyAction = NULL;
        TINT32 dwIndex = CWarBase::GetWildAction(pstTargetUser->m_atbMarch, pstTargetUser->m_udwMarchNum, ptbSourceMarch->m_nTpos);
        if(dwIndex >= 0)
        {
            ptbOccupyAction = &pstTargetUser->m_atbMarch[dwIndex];
        }
        if(ptbOccupyAction)
        {
            SActionMarchParam *pstMarchParam = &ptbOccupyAction->m_bParam[0];

            CCommJson::GenTroopJson(&pstMarchParam->m_stTroop, jsonContent["info"]["troop"]);

            jsonContent["info"]["dragon"] = Json::Value(Json::arrayValue);
            jsonContent["info"]["dragon"].append(pstMarchParam->m_stDragon.m_szName);
            jsonContent["info"]["dragon"].append(pstMarchParam->m_stDragon.m_ddwLevel);
            jsonContent["info"]["dragon"].append(pstMarchParam->m_stDragon.m_ddwIconId);
            jsonContent["info"]["dragon"].append(0);

            if(pstMarchParam->m_stDragon.m_ddwLevel > 0)
            {
                jsonContent["info"]["dragon_status"] = EN_DRAGON_STATUS_MARCH;
            }
            else
            {
                jsonContent["info"]["dragon_status"] = EN_DRAGON_STATUS_NORMAL;
            }
        }
    }
    else
    {
        CCommJson::GenTroopJson(&ptbMap->m_bTroop[0], jsonContent["info"]["troop"]);
    }

    Json::FastWriter oTmpWriter;
    ptbReport->Set_Content(oTmpWriter.write(jsonContent));

    CProcessReport::AddReceiver(ptbSourceMarch->m_nSuid, &pstSession->m_stSourceUser);
    if(pstSession->m_stSourceUser.m_tbPlayer.m_nAlid > 0 && pstSession->m_stSourceUser.m_tbPlayer.m_nAlpos > EN_ALLIANCE_POS__REQUEST)
    {
        CProcessReport::AddReceiver(CToolBase::GetAllianceUserReportKey(pstSession->m_stSourceUser.m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET),
            &pstSession->m_stSourceUser);
    }
    CProcessReport::AddReceiver(ptbSourceMarch->m_nTuid, &pstSession->m_stTargetUser);
    if(pstSession->m_stTargetUser.m_tbPlayer.m_nAlid > 0 && pstSession->m_stTargetUser.m_tbPlayer.m_nAlpos > EN_ALLIANCE_POS__REQUEST)
    {
        CProcessReport::AddReceiver(CToolBase::GetAllianceUserReportKey(pstSession->m_stTargetUser.m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET),
            &pstSession->m_stTargetUser);
    }

    return 0;
}

TINT32 CProcessReport::GenInValidReport(SSession* pstSession, TINT32 dwReportType, TINT32 dwReason, TbReport* ptbReport)
{
    if (dwReason == 0)
    {
        return 0;
    }
    TbMarch_action* ptbMarch = &pstSession->m_stReqMarch;

    CProcessReport::GenReportFlag(pstSession, ptbReport);
    ptbReport->Set_Type(dwReportType);
    ptbReport->Set_Result(dwReason);
    ptbReport->Set_Time(CTimeUtils::GetUnixTime());
    ptbReport->Set_Sid(pstSession->m_udwReqSvrId);

    CProcessReport::GenFromTo(ptbReport, ptbMarch, ptbMarch->m_nSid);

    CProcessReport::AddReceiver(ptbMarch->m_nSuid, &pstSession->m_stSourceUser);
//     if(dwReason == EN_REPORT_RESULT_TARGET_REINFORCE_FULL)
//     {
//         CProcessReport::AddReceiver(ptbMarch->m_nTuid, &pstSession->m_stTargetUser);
//     }

    return 0;
}

TINT32 CProcessReport::GenScoutPreventReport(SSession* pstSession, TINT32 dwReportType, TINT32 dwReason, TbReport* ptbReport)
{
    if (dwReason == 0)
    {
        return 0;
    }
    TbMarch_action* ptbMarch = &pstSession->m_stReqMarch;

    CProcessReport::GenReportFlag(pstSession, ptbReport);
    ptbReport->Set_Type(dwReportType);
    ptbReport->Set_Result(dwReason);
    ptbReport->Set_Time(CTimeUtils::GetUnixTime());
    ptbReport->Set_Sid(pstSession->m_udwReqSvrId);

    CProcessReport::GenFromTo(ptbReport, ptbMarch, ptbMarch->m_nSid);

    Json::Value jsonContent = Json::Value(Json::objectValue);

    jsonContent["info"] = "misaka asked";

    Json::FastWriter oTmpWriter;
    ptbReport->Set_Content(oTmpWriter.write(jsonContent));

    CProcessReport::AddReceiver(ptbMarch->m_nSuid, &pstSession->m_stSourceUser);
    CProcessReport::AddReceiver(ptbMarch->m_nTuid, &pstSession->m_stTargetUser);

    return 0;
}

TVOID CProcessReport::GenReportFlag(SSession * pstSession, TbReport* ptbReport)
{
    //SUserInfo* pstSrcUser = &pstSession->m_stSourceUser;
    pstSession->m_dwReportFlag |= EN_REPORT_FLAG_NORMAL;
    //ptbReport->Set_Id(pstSession->m_dwReportFlag);
}

TVOID CProcessReport::GenChallengerRewardInfo(SDragonNode *pstDragon, Json::Value& rJsonContent)
{
    rJsonContent["reward"] = Json::Value(Json::arrayValue);
    TUINT32 udwJsonIndex = 0;
    for (TUINT32 udwIdx = 0; udwIdx < pstDragon->m_stChallengerReward.ddwTotalNum; ++udwIdx)
    {
        rJsonContent["reward"][udwJsonIndex] = Json::Value(Json::arrayValue);
        rJsonContent["reward"][udwJsonIndex].append(pstDragon->m_stChallengerReward[udwIdx].ddwType);
        rJsonContent["reward"][udwJsonIndex].append(pstDragon->m_stChallengerReward[udwIdx].ddwId);
        rJsonContent["reward"][udwJsonIndex].append(pstDragon->m_stChallengerReward[udwIdx].ddwNum);
        udwJsonIndex++;
    }
}

TVOID CProcessReport::GenRewardInfo(SSession* pstSession, Json::Value& rJsonContent)
{
    TbMarch_action* ptbMarch = &pstSession->m_stReqMarch;
    rJsonContent["reward"] = Json::Value(Json::arrayValue);
    TUINT32 udwJsonIndex = 0;
    for(TUINT32 udwIdx = 0; udwIdx < ptbMarch->m_bReward[0].ddwTotalNum; ++udwIdx)
    {
        rJsonContent["reward"][udwJsonIndex] = Json::Value(Json::arrayValue);
        rJsonContent["reward"][udwJsonIndex].append(ptbMarch->m_bReward[0][udwIdx].ddwType);
        rJsonContent["reward"][udwJsonIndex].append(ptbMarch->m_bReward[0][udwIdx].ddwId);
        rJsonContent["reward"][udwJsonIndex].append(ptbMarch->m_bReward[0][udwIdx].ddwNum);
        udwJsonIndex++;
    }
    for(TUINT32 udwIdx = 0; udwIdx < ptbMarch->m_bAl_gift_record[0].ddwTotalNum; ++udwIdx)
    {
        rJsonContent["reward"][udwJsonIndex] = Json::Value(Json::arrayValue);
        rJsonContent["reward"][udwJsonIndex].append(ptbMarch->m_bAl_gift_record[0][udwIdx].ddwType);
        rJsonContent["reward"][udwJsonIndex].append(ptbMarch->m_bAl_gift_record[0][udwIdx].ddwId);
        rJsonContent["reward"][udwJsonIndex].append(ptbMarch->m_bAl_gift_record[0][udwIdx].ddwNum);
        udwJsonIndex++;
    }
    rJsonContent["surprise"] = Json::Value(Json::arrayValue);
    for(TUINT32 udwIdx = 0; udwIdx < ptbMarch->m_bSp_reward[0].ddwTotalNum; ++udwIdx)
    {
        rJsonContent["surprise"][udwIdx] = Json::Value(Json::arrayValue);
        rJsonContent["surprise"][udwIdx].append(ptbMarch->m_bSp_reward[0][udwIdx].ddwType);
        rJsonContent["surprise"][udwIdx].append(ptbMarch->m_bSp_reward[0][udwIdx].ddwId);
        rJsonContent["surprise"][udwIdx].append(ptbMarch->m_bSp_reward[0][udwIdx].ddwNum);
    }
}

TVOID CProcessReport::GenFromTo(TbReport* ptbReport, TbMarch_action* ptbMarch, TINT32 dwSvrId)
{
    ptbReport->m_bFrom[0].m_ddwUserId = ptbMarch->m_nSuid;
    strcpy(ptbReport->m_bFrom[0].m_szUserName, ptbMarch->m_bParam[0].m_szSourceUserName);

    ptbReport->m_bFrom[0].m_ddwPos = ptbMarch->m_nScid;
    ptbReport->m_bFrom[0].m_ddwPosType = EN_WILD_TYPE__CITY;
    ptbReport->m_bFrom[0].m_ddwPosLevel = 1; //TODO
    ptbReport->m_bFrom[0].m_ddwSvrWildType = CMapLogic::GetWildClass(dwSvrId, EN_WILD_TYPE__CITY);
    ptbReport->m_bFrom[0].m_ddwSid = dwSvrId;

    string strSvrName = CDocument::GetInstance()->GetSvrName(ptbMarch->m_nSid);
    strncpy(ptbReport->m_bFrom[0].m_szSvrName, strSvrName.c_str(), strSvrName.size());

    ptbReport->m_bFrom[0].m_ddwOwnedCityId = ptbMarch->m_nScid;
    strcpy(ptbReport->m_bFrom[0].m_szOwnedCityName, ptbMarch->m_bParam[0].m_szSourceCityName);

    ptbReport->m_bFrom[0].m_ddwAlId = ptbMarch->m_bParam[0].m_ddwSourceAlliance;
    strcpy(ptbReport->m_bFrom[0].m_szAllianceName, ptbMarch->m_bParam[0].m_szSourceAlNick);

    ptbReport->SetFlag(TbREPORT_FIELD_FROM);

    ptbReport->m_bTo[0].m_ddwUserId = ptbMarch->m_bParam[0].m_ddwTargetUserId;
    strcpy(ptbReport->m_bTo[0].m_szUserName, ptbMarch->m_bParam[0].m_szTargetUserName);

    ptbReport->m_bTo[0].m_ddwPos = ptbMarch->m_nTpos;
    ptbReport->m_bTo[0].m_ddwPosType = ptbMarch->m_bParam[0].m_ddwTargetType;
    ptbReport->m_bTo[0].m_ddwPosLevel = ptbMarch->m_bParam[0].m_ddwTargetLevel;
    ptbReport->m_bTo[0].m_ddwSvrWildType = CMapLogic::GetWildClass(dwSvrId, ptbMarch->m_bParam[0].m_ddwTargetType);
    ptbReport->m_bTo[0].m_ddwSid = dwSvrId;

    strSvrName = CDocument::GetInstance()->GetSvrName(ptbMarch->m_nSid);
    strncpy(ptbReport->m_bTo[0].m_szSvrName, strSvrName.c_str(), strSvrName.size());

    ptbReport->m_bTo[0].m_ddwOwnedCityId = ptbMarch->m_bParam[0].m_ddwTargetCityId;
    strcpy(ptbReport->m_bTo[0].m_szOwnedCityName, ptbMarch->m_bParam[0].m_szTargetCityName);

    ptbReport->m_bTo[0].m_ddwAlId = ptbMarch->m_bParam[0].m_ddwTargetAlliance;
    strcpy(ptbReport->m_bTo[0].m_szAllianceName, ptbMarch->m_bParam[0].m_szTargetAlNick);

    ptbReport->SetFlag(TbREPORT_FIELD_TO);

}

TVOID CProcessReport::GenMajorPlayer(SBattleNode* pstNode, TINT64 ddwId, Json::Value& rjson)
{
    rjson = Json::Value(Json::objectValue);

    std::map<ArmyId, OneArmy>::iterator it = pstNode->m_armys.actors.find(ddwId);
    if(it != pstNode->m_armys.actors.end())
    {
        CCommJson::GenTroopJson(&it->second.raw_troop, rjson["rawtroop"]);
        CCommJson::GenTroopJson(&it->second.left_troop, rjson["lefttroop"]);
        CCommJson::GenTroopJson(&it->second.dead_troop, rjson["deadtroop"]);
        CCommJson::GenTroopJson(&it->second.wounded_troop, rjson["woundedtroop"]);

        CCommJson::GenFortJson(&it->second.raw_fort, rjson["rawfort"]);
        CCommJson::GenFortJson(&it->second.left_fort, rjson["leftfort"]);
        CCommJson::GenFortJson(&it->second.dead_fort, rjson["deadfort"]);
        CCommJson::GenFortJson(&it->second.wounded_fort, rjson["woundedfort"]);

        rjson["kill_troop_num"] = it->second.kill_troop_num;
        rjson["kill_fort_num"] = it->second.kill_fort_num;

        CCommJson::GenResourceJson(&it->second.resource, rjson["resource"]);
    }
    else
    {
        SCommonTroop stEmptyTroop;
        stEmptyTroop.Reset();
        SCommonFort stEmptyFort;
        stEmptyFort.Reset();
        SCommonResource stEmptyRes;
        stEmptyRes.Reset();

        CCommJson::GenTroopJson(&stEmptyTroop, rjson["rawtroop"]);
        CCommJson::GenTroopJson(&stEmptyTroop, rjson["lefttroop"]);
        CCommJson::GenTroopJson(&stEmptyTroop, rjson["deadtroop"]);
        CCommJson::GenTroopJson(&stEmptyTroop, rjson["woundedtroop"]);

        CCommJson::GenFortJson(&stEmptyFort, rjson["rawfort"]);
        CCommJson::GenFortJson(&stEmptyFort, rjson["leftfort"]);
        CCommJson::GenFortJson(&stEmptyFort, rjson["deadfort"]);
        CCommJson::GenFortJson(&stEmptyFort, rjson["woundedfort"]);

        rjson["kill_troop_num"] = 0;
        rjson["kill_fort_num"] = 0;

        CCommJson::GenResourceJson(&stEmptyRes, rjson["resource"]);
    }

    rjson["knight"] = Json::Value(Json::arrayValue);
    rjson["knight"].append(pstNode->m_stKnight.ddwId);
    rjson["knight"].append(pstNode->m_stKnight.ddwLevel);
    rjson["knight"].append(pstNode->m_stKnight.ddwExpAdd);

    rjson["dragon"] = Json::Value(Json::arrayValue);
    rjson["dragon"].append(pstNode->m_stDragon.m_szName);
    rjson["dragon"].append(pstNode->m_stDragon.m_ddwLevel);
    rjson["dragon"].append(pstNode->m_stDragon.m_ddwIconId);
    rjson["dragon"].append(pstNode->m_stDragon.m_ddwExpInc);
    rjson["dragon_be_captured"] = pstNode->m_stDragon.m_ddwCaptured >= 0 ? 1 : 0;

    CCommJson::GenReportBufferInfo(&pstNode->m_stReportBuffer, rjson["buff"]);

    CCommJson::GenBattleBuffInfo(&pstNode->m_stBattleBuff, rjson["troop_buff"]);
}

TVOID CProcessReport::GenMajorPlayer(TbMarch_action* ptbMarch, Json::Value& rjson)
{
    rjson = Json::Value(Json::objectValue);
    SActionMarchParam* pstMarchParam = &ptbMarch->m_bParam[0];
    
    CCommJson::GenTroopJson(&pstMarchParam->m_stTroopRaw, rjson["rawtroop"]);
    CCommJson::GenTroopJson(&pstMarchParam->m_stTroop, rjson["lefttroop"]);
    SCommonTroop stEmptyTroop;
    stEmptyTroop.Reset();
    CCommJson::GenTroopJson(&stEmptyTroop, rjson["deadtroop"]);
    CCommJson::GenTroopJson(&stEmptyTroop, rjson["woundedtroop"]);;

    rjson["knight"] = Json::Value(Json::arrayValue);
    rjson["knight"].append(pstMarchParam->m_stKnight.ddwId);
    rjson["knight"].append(pstMarchParam->m_stKnight.ddwLevel);
    rjson["knight"].append(pstMarchParam->m_stKnight.ddwExpAdd);

    rjson["dragon"] = Json::Value(Json::arrayValue);
    rjson["dragon"].append(pstMarchParam->m_stDragon.m_szName);
    rjson["dragon"].append(pstMarchParam->m_stDragon.m_ddwLevel);
    rjson["dragon"].append(pstMarchParam->m_stDragon.m_ddwIconId);
    rjson["dragon"].append(pstMarchParam->m_stDragon.m_ddwExpInc);
    rjson["dragon_be_captured"] = pstMarchParam->m_stDragon.m_ddwCaptured >= 0 ? 1 : 0;

    rjson["kill_troop_num"] = 0;
    rjson["kill_fort_num"] = 0;
}

TVOID CProcessReport::GenAllBattlePlayer(SBattleNode* pstNode, Json::Value& rjson)
{
    rjson = Json::Value(Json::objectValue);

    SCommonTroop stRawTroop;
    stRawTroop.Reset();
    SCommonTroop stLeftTroop;
    stLeftTroop.Reset();
    SCommonTroop stDeadTroop;
    stDeadTroop.Reset();
    SCommonTroop stWoundTroop;
    stWoundTroop.Reset();

    SCommonFort stRawFort;
    stRawFort.Reset();
    SCommonFort stLeftFort;
    stLeftFort.Reset();
    SCommonFort stDeadFort;
    stDeadFort.Reset();
    SCommonFort stWoundFort;
    stWoundFort.Reset();

    for(std::map<ArmyId, OneArmy>::iterator it = pstNode->m_armys.actors.begin(); it != pstNode->m_armys.actors.end(); ++it)
    {
        for(TUINT32 udwTroopType = 0; udwTroopType < EN_TROOP_TYPE__END; ++udwTroopType)
        {
            stRawTroop[udwTroopType] += it->second.raw_troop[udwTroopType];
            stLeftTroop[udwTroopType] += it->second.left_troop[udwTroopType];
            stDeadTroop[udwTroopType] += it->second.dead_troop[udwTroopType];
            stWoundTroop[udwTroopType] += it->second.wounded_troop[udwTroopType];
        }
        for(TUINT32 udwFortType = 0; udwFortType < EN_FORT_TYPE__END; ++udwFortType)
        {
            stRawFort[udwFortType] += it->second.raw_fort[udwFortType];
            stLeftFort[udwFortType] += it->second.left_fort[udwFortType];
            stDeadFort[udwFortType] += it->second.dead_fort[udwFortType];
            stWoundFort[udwFortType] += it->second.wounded_fort[udwFortType];
        }
    }

    CCommJson::GenTroopJson(&stRawTroop, rjson["rawtroop"]);
    CCommJson::GenTroopJson(&stLeftTroop, rjson["lefttroop"]);
    CCommJson::GenTroopJson(&stDeadTroop, rjson["deadtroop"]);
    CCommJson::GenTroopJson(&stWoundTroop, rjson["woundedtroop"]);

    CCommJson::GenFortJson(&stRawFort, rjson["rawfort"]);
    CCommJson::GenFortJson(&stLeftFort, rjson["leftfort"]);
    CCommJson::GenFortJson(&stDeadFort, rjson["deadfort"]);
    CCommJson::GenFortJson(&stWoundFort, rjson["woundedfort"]);

    CCommJson::GenResourceJson(&pstNode->m_armys.resource, rjson["resource"]);
}

TVOID CProcessReport::GenReinforcePlayer(SBattleNode* pstNode, TbMarch_action* ptbMarch, Json::Value& rjson)
{
    std::map<ArmyId, OneArmy>::iterator it = pstNode->m_armys.actors.find(ptbMarch->m_nId);
    if(it == pstNode->m_armys.actors.end())
    {
        return;
    }

    rjson = Json::Value(Json::objectValue);

    CCommJson::GenTroopJson(&it->second.raw_troop, rjson["rawtroop"]);
    CCommJson::GenTroopJson(&it->second.left_troop, rjson["lefttroop"]);
    CCommJson::GenTroopJson(&it->second.dead_troop, rjson["deadtroop"]);
    CCommJson::GenTroopJson(&it->second.wounded_troop, rjson["woundedtroop"]);

    CCommJson::GenFortJson(&it->second.raw_fort, rjson["rawfort"]);
    CCommJson::GenFortJson(&it->second.left_fort, rjson["leftfort"]);
    CCommJson::GenFortJson(&it->second.dead_fort, rjson["deadfort"]);
    CCommJson::GenFortJson(&it->second.wounded_fort, rjson["woundedfort"]);

    CCommJson::GenResourceJson(&it->second.resource, rjson["resource"]);

    rjson["player_name"] = ptbMarch->m_bParam[0].m_szSourceUserName;
    rjson["player_uid"] = ptbMarch->m_bParam[0].m_ddwSourceUserId;
    rjson["player_spos"] = ptbMarch->m_bParam[0].m_ddwSourceCityId;

    rjson["kill_troop_num"] = it->second.kill_troop_num;
    rjson["kill_fort_num"] = it->second.kill_fort_num;

    rjson["knight"] = Json::Value(Json::arrayValue);
    rjson["knight"].append(it->second.knight.ddwId);
    rjson["knight"].append(it->second.knight.ddwLevel);
    rjson["knight"].append(it->second.knight.ddwExpAdd);
}

TVOID CProcessReport::GenReinforcePlayer(TbMarch_action* ptbMarch, Json::Value& rjson)
{
    rjson = Json::Value(Json::objectValue);
    SActionMarchParam* pstMarchParam = &ptbMarch->m_bParam[0];

    CCommJson::GenTroopJson(&pstMarchParam->m_stTroopRaw, rjson["rawtroop"]);
    CCommJson::GenTroopJson(&pstMarchParam->m_stTroop, rjson["lefttroop"]);
    SCommonTroop stTmpTroop;
    stTmpTroop.Reset();
    CCommJson::GenTroopJson(&stTmpTroop, rjson["deadtroop"]);
    CCommJson::GenTroopJson(&stTmpTroop, rjson["woundedtroop"]);;

    rjson["player_name"] = pstMarchParam->m_szSourceUserName;
    rjson["player_uid"] = pstMarchParam->m_ddwSourceUserId;
    rjson["player_spos"] = pstMarchParam->m_ddwSourceCityId;

    rjson["kill_troop_num"] = 0;
    rjson["kill_fort_num"] = 0;

    rjson["knight"] = Json::Value(Json::arrayValue);
    rjson["knight"].append(pstMarchParam->m_stKnight.ddwId);
    rjson["knight"].append(pstMarchParam->m_stKnight.ddwLevel);
    rjson["knight"].append(pstMarchParam->m_stKnight.ddwExpAdd);
}

TVOID CProcessReport::AddReceiver(TINT64 ddwReceiverId, SUserInfo* pstUser)
{
    if(ddwReceiverId != 0)
    {
        CToolBase::AddUserToMailReceiverList(pstUser->m_adwMailSendUidList, pstUser->m_udwMailSendNum, ddwReceiverId);
    }
}

TINT32 CProcessReport::GenDragonAttackReport(SSession *pstSession, TINT32 dwWarResult, SDragonNode *pstHeroNode, SMonsterNode *pstMonsterNode, TbReport* ptbReport)
{
    TbMarch_action* ptbAction = &pstSession->m_stReqMarch;
    SActionMarchParam *pstMarch = &ptbAction->m_bParam.m_astList[0];

    pstSession->m_dwReportFlag = EN_REPORT_FLAG_DRAGON_ATTACK;
    ptbReport->Set_Type(EN_REPORT_TYPE_DRAGON_MONSTER);
    ptbReport->Set_Result(dwWarResult);
    ptbReport->Set_Time(CTimeUtils::GetUnixTime());
    ptbReport->Set_Sid(pstSession->m_udwReqSvrId);

    CProcessReport::GenFromTo(ptbReport, ptbAction, ptbAction->m_nSid);

    Json::Value jsonContent = Json::Value(Json::objectValue);

    CProcessReport::GenDragonInfo(pstHeroNode, pstMarch, jsonContent["attacker"]);
    CProcessReport::GenMonsterInfo(pstMonsterNode, jsonContent["defender"]);

    CProcessReport::GenRewardInfo(pstSession, jsonContent);

    jsonContent["kill_notice_owner"] = 0;

    Json::FastWriter jsonWriter;
    ptbReport->Set_Content(jsonWriter.write(jsonContent));

    //CProcessReport::AddReceiver(ptbAction->m_nSuid, &pstSession->m_stSourceUser);

    return 0;
}

TINT32 CProcessReport::GenChallengerReport(SSession *pstSession, TINT64 ddwChallengerId, SDragonNode *pstHeroNode, SMonsterNode *pstMonsterNode, TbReport* ptbReport)
{
    TbMarch_action* ptbAction = &pstSession->m_stReqMarch;

    ptbReport->Set_Type(EN_REPORT_TYPE_DRAGON_MONSTER);
    ptbReport->Set_Result(EN_REPORT_RESULT_WIN);
    ptbReport->Set_Time(CTimeUtils::GetUnixTime());
    ptbReport->Set_Sid(pstSession->m_udwReqSvrId);

    CProcessReport::GenFromTo(ptbReport, ptbAction, ptbAction->m_nSid);

    Json::Value jsonContent = Json::Value(Json::objectValue);

    CProcessReport::GenMonsterInfo(pstMonsterNode, jsonContent["defender"]);

    CProcessReport::GenChallengerRewardInfo(pstHeroNode, jsonContent);

    jsonContent["kill_notice_owner"] = ddwChallengerId;

    Json::FastWriter jsonWriter;
    ptbReport->Set_Content(jsonWriter.write(jsonContent));

    return 0;
}

TVOID CProcessReport::GenDragonInfo(SDragonNode* pstNode, SActionMarchParam* pstMarch, Json::Value& rjson)
{
    rjson = Json::Value(Json::objectValue);
    rjson["dragon"] = Json::Value(Json::arrayValue);
    rjson["dragon"].append(pstMarch->m_stDragon.m_szName);
    rjson["dragon"].append(pstMarch->m_stDragon.m_ddwLevel);
    rjson["dragon"].append(pstMarch->m_stDragon.m_ddwIconId);
    rjson["dragon"].append(pstNode->m_dwExp);
    rjson["dragon"].append(pstNode->m_dwRawExp + pstNode->m_dwExp);

    rjson["info"] = Json::Value(Json::objectValue);
    rjson["info"]["attack_num"] = pstNode->m_dwRealAttackTimes;

    rjson["info"]["attack_info"] = Json::Value(Json::arrayValue);
    TINT32 dwCriticHitNum = 0;
    for(TINT32 dwIdx = 0; dwIdx < pstNode->m_dwRealAttackTimes; ++dwIdx)
    {
        if (pstNode->m_astAttackInfo[dwIdx].m_dwAttackType == 1)
        {
            dwCriticHitNum++;
        }
    }
    rjson["info"]["attack_info"].append(pstNode->m_dwRealAttackTimes - dwCriticHitNum);
    rjson["info"]["attack_info"].append(dwCriticHitNum); 
    rjson["info"]["attack_info"].append(pstNode->m_dwBeginMultiAttackTimes);
    rjson["info"]["attack_info"].append(pstNode->m_dwMaxMultiAttackTimes);
    rjson["info"]["attack_info"].append(pstNode->m_dwMultiAttackBuff);
    rjson["info"]["attack_info"].append(pstNode->m_dwNextMultiAttackBuff);
    rjson["info"]["attack_info"].append(pstNode->m_dwAlMultiAttackBuff);
}

TVOID CProcessReport::GenMonsterInfo(SMonsterNode* pstNode, Json::Value& rjson)
{
    rjson = Json::Value(Json::objectValue);
    rjson["monster"] = Json::Value(Json::arrayValue);
    rjson["monster"].append(pstNode->m_dwType);
    rjson["monster"].append(pstNode->m_dwLevel);
    rjson["monster"].append(pstNode->m_ddwMaxHp);
    rjson["monster"].append(pstNode->m_ddwDefence);
    rjson["monster"].append(pstNode->m_ddwHp);
    rjson["monster"].append(pstNode->m_ddwLostHp);
}

TINT32 CProcessReport::GenDragonOccupyReport(SSession* pstSession, TbReport* ptbReport)
{
    CProcessReport::GenTransportReport(pstSession, ptbReport);
    ptbReport->Set_Type(EN_REPORT_TYPE_DRAGON_COLLECT);
    return 0;
}

TINT32 CProcessReport::GenThroneAssignReport(SSession* pstSession, TbReport* ptbReport)
{
    CProcessReport::GenTransportReport(pstSession, ptbReport);
    ptbReport->Set_Type(EN_REPORT_TYPE_THRONE_DISPATCH);
    return 0;
}

TVOID CProcessReport::GenPrisonReport(SUserInfo* pstCaptor, TbPlayer* ptbSaver, TINT32 dwReportType, TINT32 dwReportResult, TbReport* ptbReport)
{
    ptbReport->Set_Type(dwReportType);
    ptbReport->Set_Result(dwReportResult);
    ptbReport->Set_Time(CTimeUtils::GetUnixTime());
    ptbReport->Set_Sid(pstCaptor->m_tbPlayer.m_nSid);

    Json::Value jsonContent = Json::Value(Json::objectValue);

    jsonContent["captor"] = Json::Value(Json::objectValue);
    jsonContent["captor"]["uid"] = pstCaptor->m_tbPlayer.m_nUid;
    jsonContent["captor"]["al_nick"] = pstCaptor->m_tbPlayer.m_sAl_nick_name;
    jsonContent["captor"]["uname"] = pstCaptor->m_tbPlayer.m_sUin;

    if(ptbSaver)
    {
        jsonContent["saver"] = Json::Value(Json::objectValue);
        jsonContent["saver"]["uid"] = ptbSaver->m_nUid;
        jsonContent["saver"]["al_nick"] = ptbSaver->m_sAl_nick_name;
        jsonContent["saver"]["uname"] = ptbSaver->m_sUin;
    }
    else
    {
        jsonContent["saver"] = Json::Value(Json::objectValue);
        jsonContent["saver"]["uid"] = 0;
        jsonContent["saver"]["al_nick"] = "";
        jsonContent["saver"]["uname"] = "";
    }

    jsonContent["dragon_list"] = Json::Value(Json::objectValue);
    for(TUINT32 udwIdx = 0; udwIdx < pstCaptor->m_udwPassiveMarchNum; ++udwIdx)
    {
        if(pstCaptor->m_atbPassiveMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__PRISON_TIMER
        && pstCaptor->m_atbPassiveMarch[udwIdx].m_nStatus == EN_MARCH_STATUS__DEFENDING)
        {
            if(pstCaptor->m_aucPassiveMarchFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
            {
                continue;
            }

            Json::Value jsonOne = Json::Value(Json::objectValue);
            jsonOne["al_nick"] = pstCaptor->m_atbPassiveMarch[udwIdx].m_bPrison_param[0].szSourceAlNick;
            jsonOne["uname"] = pstCaptor->m_atbPassiveMarch[udwIdx].m_bPrison_param[0].szSourceUserName;
            jsonOne["dragon_lv"] = pstCaptor->m_atbPassiveMarch[udwIdx].m_bPrison_param[0].stDragon.m_ddwLevel;
            jsonOne["dragon_name"] = pstCaptor->m_atbPassiveMarch[udwIdx].m_bPrison_param[0].stDragon.m_szName;

            jsonContent["dragon_list"][CCommonFunc::NumToString(pstCaptor->m_atbPassiveMarch[udwIdx].m_nSuid)] = jsonOne;
        }
    }

    Json::FastWriter jsonWriter;
    ptbReport->Set_Content(jsonWriter.write(jsonContent));
}

TVOID CProcessReport::GenTradeReport(SSession *pstSession, STradeInfo *pstTrade, TbReport *ptbReport)
{
    TbMarch_action* ptbMarch = &pstSession->m_stReqMarch;
    TbPlayer *ptbSPlayer = &pstSession->m_stSourceUser.m_tbPlayer;
    TbPlayer *ptbTPlayer = &pstSession->m_stTargetUser.m_tbPlayer;

    CProcessReport::GenReportFlag(pstSession, ptbReport);
    ptbReport->Set_Type(EN_REPORT_TYPE_TRADE);
    ptbReport->Set_Result(EN_REPORT_RESULT_WIN);
    ptbReport->Set_Time(CTimeUtils::GetUnixTime());
    ptbReport->Set_Sid(pstSession->m_udwReqSvrId);

    CProcessReport::GenFromTo(ptbReport, ptbMarch, ptbMarch->m_nSid);
    Json::Value jsonContent = Json::Value(Json::objectValue);

    jsonContent["source_avatar"] = ptbSPlayer->m_nAvatar;
    jsonContent["target_avatar"] = ptbTPlayer->m_nAvatar;
    jsonContent["visit_result"] = pstTrade->m_ddwStatus == EN_TRADE_STATUS__RECEPTION ? 1 : 0;
    jsonContent["source_reward_num"] = pstTrade->m_ddwRewardNum;
    jsonContent["target_reward_num"] = pstTrade->m_ddwReceptionRewardNum;

    Json::FastWriter oTmpWriter;
    ptbReport->Set_Content(oTmpWriter.write(jsonContent));

    CProcessReport::AddReceiver(ptbMarch->m_nSuid, &pstSession->m_stSourceUser);
    CProcessReport::AddReceiver(ptbMarch->m_nTuid, &pstSession->m_stTargetUser);

}

TVOID CProcessReport::GenPayTaxReport(SSession *pstSession, TUINT32 *audwResource, TbReport* ptbReport)
{
    TbMarch_action* ptbTax = &pstSession->m_stReqMarch;
    TbPlayer *ptbSPlayer = &pstSession->m_stSourceUser.m_tbPlayer;
    SUserInfo *pstTUser = &pstSession->m_stTargetUser;
    SCityInfo *pstTCity = &pstTUser->m_stCityInfo;

    CProcessReport::GenReportFlag(pstSession, ptbReport);
    ptbReport->Set_Type(EN_REPORT_TYPE_PAY_TAX);
    ptbReport->Set_Result(EN_REPORT_RESULT_WIN);
    ptbReport->Set_Time(CTimeUtils::GetUnixTime());
    ptbReport->Set_Sid(pstSession->m_udwReqSvrId);

    Json::Value jsonContent = Json::Value(Json::objectValue);

    jsonContent["king_info"]["uid"] = ptbSPlayer->m_nUid;
    jsonContent["king_info"]["avatar"] = ptbSPlayer->m_nAvatar;
    jsonContent["king_info"]["vip_level"] = CPlayerBase::GetVipLevel(ptbSPlayer);
    jsonContent["king_info"]["city_pos"] = ptbTax->m_nScid;
    jsonContent["king_info"]["alnick"] = ptbSPlayer->m_sAl_nick_name;
    jsonContent["king_info"]["uname"] = ptbSPlayer->m_sUin;

    jsonContent["tax_resource"] = Json::Value(Json::arrayValue);
    for (TUINT32 udwIdx = 0; udwIdx < EN_RESOURCE_TYPE__END; udwIdx++)
    {
        jsonContent["tax_resource"][udwIdx] = audwResource[udwIdx];
    }

    jsonContent["tax_rate"] = ptbTax->m_jTax_info["tax_rate"];
    jsonContent["gold_production"] = pstTCity->m_astResProduction[EN_RESOURCE_TYPE__GOLD].m_ddwCurProduction;
    jsonContent["pay_tax_uid"] = ptbTax->m_nTuid;

    Json::FastWriter jsonWriter;
    ptbReport->Set_Content(jsonWriter.write(jsonContent));

    CProcessReport::AddReceiver(ptbTax->m_nTuid, &pstSession->m_stTargetUser);
}

TVOID CProcessReport::GenCollectTaxReport(SSession *pstSession, TbReport* ptbReport)
{
    TbMarch_action* ptbTax = &pstSession->m_stReqMarch;

    CProcessReport::GenReportFlag(pstSession, ptbReport);
    ptbReport->Set_Type(EN_REPORT_TYPE_COLLECT_TAX);
    ptbReport->Set_Result(EN_REPORT_RESULT_WIN);
    ptbReport->Set_Time(CTimeUtils::GetUnixTime());
    ptbReport->Set_Sid(pstSession->m_udwReqSvrId);

    Json::Value jsonContent = Json::Value(Json::objectValue);

    jsonContent["tax_resource"] = ptbTax->m_jTax_info["resource"];

    jsonContent["tax_detail"] = ptbTax->m_jTax_info["tax_detail"];

    jsonContent["tax_rate"] = ptbTax->m_jTax_info["tax_rate"];

    Json::FastWriter jsonWriter;
    ptbReport->Set_Content(jsonWriter.write(jsonContent));

    CProcessReport::AddReceiver(ptbTax->m_nSuid, &pstSession->m_stSourceUser);
}

TINT64 CProcessReport::GetDefenderMajorId(SBattleNode* pstDefender, TbMarch_action *ptbMarch, TbMap *ptbWild, SUserInfo *pstTUser)
{
    TINT64 ddwMajorId = ptbMarch->m_nTuid > 0 ? ptbMarch->m_nTuid : 0;
    if (ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__ATTACK &&
        (CMapLogic::GetWildClass(ptbWild->m_nSid, ptbWild->m_nType) == EN_WILD_CLASS_MONSTER_NEST
        || CMapLogic::GetWildClass(ptbWild->m_nSid, ptbWild->m_nType) == EN_WILD_CLASS_RES))
    {
        for (TUINT32 udwIdx = 0; udwIdx < pstDefender->m_udwEncampNum; ++udwIdx)
        {
            TbMarch_action *ptbAction = pstDefender->m_pastEncampActionList[udwIdx];
            if (ptbAction->m_nSclass == EN_ACTION_SEC_CLASS__OCCUPY && ptbAction->m_nTpos == ptbWild->m_nId &&
                ptbAction->m_nSuid == pstTUser->m_tbPlayer.m_nUid)
            {
                ddwMajorId = pstDefender->m_pastEncampActionList[udwIdx]->m_nId;
                break;
            }
        }
    }
    else if (ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__ATTACK_THRONE
        || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR_THRONE)
    {
//         for (TUINT32 udwIdx = 0; udwIdx < pstDefender->m_udwEncampNum; ++udwIdx)
//         {
//             TbMarch_action *ptbAction = pstDefender->m_pastEncampActionList[udwIdx];
//             if (ptbAction->m_nSclass == EN_ACTION_SEC_CLASS__ASSIGN_THRONE && ptbAction->m_nTpos == ptbWild->m_nId &&
//                 ptbAction->m_nSuid == pstTUser->m_tbPlayer.m_nUid)
//             {
//                 ddwMajorId = pstDefender->m_pastEncampActionList[udwIdx]->m_nId;
//                 break;
//             }
//         }
    }
    else
    {
        for (TUINT32 udwIdx = 0; udwIdx < pstTUser->m_udwMarchNum; ++udwIdx)
        {
            TbMarch_action *ptbAction = &pstTUser->m_atbMarch[udwIdx];
            if (ptbAction->m_nMclass == EN_ACTION_MAIN_CLASS__MARCH
                && ptbAction->m_nSclass == EN_ACTION_SEC_CLASS__CAMP
                && ptbAction->m_nTpos == ptbWild->m_nId)
            {
                ddwMajorId = ptbAction->m_nId;
                break;
            }
        }
    }

    return ddwMajorId;
}
