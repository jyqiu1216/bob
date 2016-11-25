#include "report_logic.h"
#include "game_svr.h"
#include "map_logic.h"
#include "common_json.h"
#include "document.h"

CReportLogic::CReportLogic()
{

}

CReportLogic::~CReportLogic()
{

}

TVOID CReportLogic::GenFromTo(TbReport* ptbReport, TbMarch_action* ptbMarch, TINT32 dwSvrId)
{
    ptbReport->m_bFrom[0].m_ddwUserId = ptbMarch->m_nSuid;
    strcpy(ptbReport->m_bFrom[0].m_szUserName, ptbMarch->m_bParam[0].m_szSourceUserName);

    ptbReport->m_bFrom[0].m_ddwPos = ptbMarch->m_nScid;
    ptbReport->m_bFrom[0].m_ddwPosType = EN_WILD_TYPE__CITY;
    ptbReport->m_bFrom[0].m_ddwPosLevel = 1;//TODO
    ptbReport->m_bFrom[0].m_ddwSvrWildType = CMapLogic::GetWildClass(dwSvrId, EN_WILD_TYPE__CITY);
    ptbReport->m_bFrom[0].m_ddwSid = dwSvrId;
    //string strSvrName = CGameSvrInfo::GetInstance()->GetSvrNameBySid(ptbMarch->m_nSid);
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
    //strSvrName = CGameSvrInfo::GetInstance()->GetSvrNameBySid(ptbMarch->m_nSid);
    strSvrName = CDocument::GetInstance()->GetSvrName(ptbMarch->m_nSid);
    strncpy(ptbReport->m_bTo[0].m_szSvrName, strSvrName.c_str(), strSvrName.size());

    ptbReport->m_bTo[0].m_ddwOwnedCityId = ptbMarch->m_bParam[0].m_ddwTargetCityId;
    strcpy(ptbReport->m_bTo[0].m_szOwnedCityName, ptbMarch->m_bParam[0].m_szTargetCityName);

    ptbReport->m_bTo[0].m_ddwAlId = ptbMarch->m_bParam[0].m_ddwTargetAlliance;
    strcpy(ptbReport->m_bTo[0].m_szAllianceName, ptbMarch->m_bParam[0].m_szTargetAlNick);

    ptbReport->SetFlag(TbREPORT_FIELD_TO);
}

TVOID CReportLogic::GenMajorPlayer(TbMarch_action* ptbMarch, Json::Value& rjson)
{
    rjson = Json::Value(Json::objectValue);
    SActionMarchParam* pstMarchParam = &ptbMarch->m_bParam[0];

    CCommJson::GenTroopJson(&pstMarchParam->m_stTroopRaw, rjson["rawtroop"]);
    CCommJson::GenTroopJson(&pstMarchParam->m_stTroop, rjson["lefttroop"]);
    SCommonTroop stEmptyTroop;
    stEmptyTroop.Reset();
    CCommJson::GenTroopJson(&stEmptyTroop, rjson["deadtroop"]);
    CCommJson::GenTroopJson(&stEmptyTroop, rjson["woundedtroop"]);;

    rjson["dragon"] = Json::Value(Json::arrayValue);
    rjson["dragon"].append(pstMarchParam->m_stDragon.m_szName);
    rjson["dragon"].append(pstMarchParam->m_stDragon.m_ddwLevel);
    rjson["dragon"].append(pstMarchParam->m_stDragon.m_ddwIconId);
    rjson["dragon"].append(pstMarchParam->m_stDragon.m_ddwExpInc);
    rjson["dragon_be_captured"] = pstMarchParam->m_stDragon.m_ddwCaptured >= 0 ? 1 : 0;

    rjson["kill_troop_num"] = 0;
    rjson["kill_fort_num"] = 0;
}

TVOID CReportLogic::GenDisMissReport(TbMarch_action* ptbMarch, TbReport* ptbReport)
{
    //TODO
    //É¾³ý
}
