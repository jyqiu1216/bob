#include "throne_info_json.h"

#include "common_func.h"
#include "game_info.h"
#include "player_base.h"
#include "common_json.h"
#include "tool_base.h"
#include "game_command.h"
#include "common_base.h"

CThroneInfoJson::CThroneInfoJson()
{

}

CThroneInfoJson::~CThroneInfoJson()
{

}

TVOID CThroneInfoJson::GenDataJson(SSession* pstSession, Json::Value& rJson)
{
    if (pstSession->m_udwCommand == EN_CLIENT_REQ_COMMAND__GET_TITLE_INFO)
    {
        GenTitleJson(pstSession, rJson);
    }
    else
    {
        GenThroneJson(pstSession, rJson);
    }
}

TVOID CThroneInfoJson::GenThroneJson(SSession* pstSession, Json::Value& rJson)
{
    CGameInfo *pGameInfo = CGameInfo::GetInstance();

    Json::Value& jsonThroneInfo = rJson["svr_throne_detail_info"];
    jsonThroneInfo = Json::Value(Json::objectValue);

    TbThrone *ptbThrone = &pstSession->m_tbThrone;

    Json::Value& jOneThrone = jsonThroneInfo[CCommonFunc::NumToString(ptbThrone->m_nPos)];
    jOneThrone = Json::Value(Json::objectValue);

    jOneThrone["status"] = ptbThrone->m_nStatus;
    jOneThrone["end_time"] = ptbThrone->m_nEnd_time;
    if (ptbThrone->m_nTax_id == 0)
    {
        jOneThrone["tax"] = pGameInfo->m_oJsonRoot["game_throne_tax"]["default"].asInt();
    }
    else
    {
        jOneThrone["tax"] = ptbThrone->m_nTax_id;
    }

    jOneThrone["buff"] = Json::Value(Json::arrayValue);
    const Json::Value& jThroneBuff = pGameInfo->m_oJsonRoot["game_throne_buff"];
    Json::Value::Members members = jThroneBuff.getMemberNames();
    TUINT32 udwBuffId = 0;
    TUINT32 udwBuffNum = 0;
    TUINT32 udwBuffIdx = 0;
    for (Json::Value::Members::iterator it = members.begin(); it != members.end(); it++)
    {
        udwBuffId = jThroneBuff[*it][0U].asUInt();
        udwBuffNum = jThroneBuff[*it][1U].asUInt();
        jOneThrone["buff"][udwBuffIdx] = Json::Value(Json::arrayValue);
        jOneThrone["buff"][udwBuffIdx].append(udwBuffId);
        jOneThrone["buff"][udwBuffIdx].append(udwBuffNum);
        udwBuffIdx++;
    }

    jOneThrone["al_info"] = Json::Value(Json::objectValue);
    jOneThrone["al_info"]["alid"] = pstSession->m_tbTmpAlliance.m_nAid;
    jOneThrone["al_info"]["al_nick"] = pstSession->m_tbTmpAlliance.m_sAl_nick_name;
    jOneThrone["al_info"]["al_name"] = pstSession->m_tbTmpAlliance.m_sName;
    jOneThrone["al_info"]["al_force"] = pstSession->m_tbTmpAlliance.m_nMight;
    jOneThrone["al_info"]["member"] = pstSession->m_tbTmpAlliance.m_nMember;
    jOneThrone["al_info"]["flag"] = pstSession->m_tbTmpAlliance.m_nAvatar;

    jOneThrone["owner"] = Json::Value(Json::objectValue);
    jOneThrone["owner"]["uid"] = pstSession->m_tbTmpPlayer.m_nUid;
    jOneThrone["owner"]["avatar"] = pstSession->m_tbTmpPlayer.m_nAvatar;
    jOneThrone["owner"]["vip_level"] = CPlayerBase::GetVipLevel(&pstSession->m_tbTmpPlayer);
    jOneThrone["owner"]["city_pos"] = pstSession->m_tbTmpPlayer.m_nCid;
    jOneThrone["owner"]["name"] = pstSession->m_tbTmpPlayer.m_sUin;
    jOneThrone["owner"]["sid"] = pstSession->m_tbTmpPlayer.m_nSid;

    SCommonTroop stReinForceTroop;
    stReinForceTroop.Reset();
    TUINT32 udwReinforceNum = 0;

    jOneThrone["main_assign"] = Json::Value(Json::objectValue);
    jOneThrone["reinforce_detail"] = Json::Value(Json::arrayValue);

    TbMarch_action *ptbMarch = NULL;
    TBOOL bHasMain = FALSE;
    for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_udwTmpMarchNum; udwIdx++)
    {
        ptbMarch = &pstSession->m_atbTmpMarch[udwIdx];
        if (ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__ASSIGN_THRONE
            && ptbMarch->m_nStatus != EN_MARCH_STATUS__RETURNING
            && ptbMarch->m_nSuid == ptbThrone->m_nOwner_id)
        {
            bHasMain = TRUE;
            CCommJson::GenMarchInfo(ptbMarch, jOneThrone["main_assign"]);
            udwReinforceNum++;
            for (TUINT32 udwIdy = 0; udwIdy < EN_TROOP_TYPE__END; udwIdy++)
            {
                stReinForceTroop[udwIdy] += ptbMarch->m_bParam[0].m_stTroop[udwIdy];
            }
            break;
        }
    }

    TUINT32 udwMaxReinforceNum = CCommonBase::GetGameBasicVal(EN_GAME_BASIC_RALLY_SLOT_LIMIT);
    for (TUINT32 udwIdx = 0, udwJsonIdx = 0; udwIdx < pstSession->m_udwTmpMarchNum; udwIdx++)
    {
        ptbMarch = &pstSession->m_atbTmpMarch[udwIdx];
        if (ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_THRONE
            && ptbMarch->m_nStatus != EN_MARCH_STATUS__RETURNING
            && ptbMarch->m_nTpos == ptbThrone->m_nPos)
        {
            if (pstSession->m_udwTmpMarchNum >= udwMaxReinforceNum 
                && bHasMain == FALSE 
                && (ptbThrone->m_nOwner_id == ptbMarch->m_nSuid
                || udwIdx == pstSession->m_udwTmpMarchNum - 1))
            {
                bHasMain = TRUE;
                CCommJson::GenMarchInfo(ptbMarch, jOneThrone["main_assign"]);
                udwReinforceNum++;
                for (TUINT32 udwIdy = 0; udwIdy < EN_TROOP_TYPE__END; udwIdy++)
                {
                    stReinForceTroop[udwIdy] += ptbMarch->m_bParam[0].m_stTroop[udwIdy];
                }
            }
            else
            {
                CCommJson::GenMarchInfo(ptbMarch, jOneThrone["reinforce_detail"][udwJsonIdx++]);
                udwReinforceNum++;
                for (TUINT32 udwIdy = 0; udwIdy < EN_TROOP_TYPE__END; udwIdy++)
                {
                    stReinForceTroop[udwIdy] += ptbMarch->m_bParam[0].m_stTroop[udwIdy];
                }
            }
        }
    }

    jOneThrone["troop_total_num"] = CToolBase::GetTroopSumNum(stReinForceTroop);
    jOneThrone["troop_total_force"] = CToolBase::GetTroopSumForce(stReinForceTroop);

    CCommJson::GenTroopJson(&stReinForceTroop, jOneThrone["total_troop"]);

    CCommJson::GenTitleInfo(&pstSession->m_stTitleInfoList, ptbThrone, jOneThrone["title_info"]);
}

TVOID CThroneInfoJson::GenTitleJson(SSession* pstSession, Json::Value& rJson)
{
    Json::Value& jsonThroneInfo = rJson["svr_other_title_info"];
    jsonThroneInfo = Json::Value(Json::objectValue);
    string szSid = pstSession->m_stReqParam.m_szKey[0];

    if (pstSession->m_tbThrone.m_nPos == 0)
    {
        rJson["svr_other_title_info"][szSid] = Json::Value(Json::arrayValue);
    }
    else
    {
        CCommJson::GenTitleInfo(&pstSession->m_stTitleInfoList, &pstSession->m_tbThrone, rJson["svr_other_title_info"][szSid]);
    }
}