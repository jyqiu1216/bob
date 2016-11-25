#ifndef _PROCESS_REPORT_H
#define _PROCESS_REPORT_H

#include "aws_table_report.h"
#include "aws_table_action.h"
#include "aws_table_march_action.h"
#include "war_base.h"

class SSession;
class SBattleNode;
class SUserInfo;

class CProcessReport
{
public:
    static TINT32 GenAttackReport(SSession *pstSession, TINT32 dwReportType, TINT32 dwWarResult, SBattleNode *pAttack, SBattleNode* pDefender, TbReport* ptbReport);
    static TINT32 GenDragonAttackReport(SSession *pstSession, TINT32 dwWarResult, SDragonNode *pstHeroNode, SMonsterNode *pstMonsterNode, TbReport* ptbReport);
    static TINT32 GenChallengerReport(SSession *pstSession, TINT64 ddwChallengerId, SDragonNode *pstHeroNode, SMonsterNode *pstMonsterNode, TbReport* ptbReport);
    static TINT32 GenTransportReport(SSession* pstSession, TbReport* ptbReport);
    static TINT32 GenReinforceReport(SSession* pstSession, TbReport* ptbReport);
    static TINT32 GenOccupyReport(SSession* pstSession, TbReport* ptbReport);
    static TINT32 GenCampReport(SSession* pstSession, TbReport* ptbReport);
    static TINT32 GenScoutCityReport(SSession* pstSession, TbReport* ptbReport);
    static TINT32 GenScoutThroneReport(SSession* pstSession, TbReport* ptbReport);
    static TINT32 GenScoutWildReport(SSession* pstSession, TbReport* ptbReport);
    static TINT32 GenScoutIdolReport(SSession* pstSession, TbReport* ptbReport);
    static TINT32 GenInValidReport(SSession* pstSession, TINT32 dwReportType, TINT32 dwReason, TbReport* ptbReport);
    static TINT32 GenScoutPreventReport(SSession* pstSession, TINT32 dwReportType, TINT32 dwReason, TbReport* ptbReport);
    static TINT32 GenDragonOccupyReport(SSession* pstSession, TbReport* ptbReport);
    static TINT32 GenThroneAssignReport(SSession* pstSession, TbReport* ptbReport);
    static TINT32 GenThroneReinforceReport(SSession* pstSession, TbReport* ptbReport);

    static TINT32 GenThroneStatusReport(SSession* pstSession, TINT32 dwReportType, const Json::Value &jContent, TbReport* ptbReport);

    static TVOID GenReportFlag(SSession* pstSession, TbReport* ptbReport);
    static TVOID GenRewardInfo(SSession* pstSession, Json::Value& rJsonContent);
    static TVOID GenChallengerRewardInfo(SDragonNode *pstDragon, Json::Value& rJsonContent);
    static TVOID GenFromTo(TbReport* ptbReport, TbMarch_action* ptbMarch, TINT32 dwSvrId);

    static TVOID GenMajorPlayer(SBattleNode* pstNode, TINT64 ddwId, Json::Value& rjson);
    static TVOID GenMajorPlayer(TbMarch_action* ptbMarch, Json::Value& rjson);
    static TVOID GenAllBattlePlayer(SBattleNode* pstNode, Json::Value& rjson);
    static TVOID GenReinforcePlayer(SBattleNode* pstNode, TbMarch_action* ptbMarch, Json::Value& rjson);
    static TVOID GenReinforcePlayer(TbMarch_action* ptbMarch, Json::Value& rjson);

    static TVOID GenPrisonReport(SUserInfo* pstCaptor, TbPlayer* ptbSaver, TINT32 dwReportType, TINT32 dwReportResult, TbReport* ptbReport);

    static TVOID AddReceiver(TINT64 ddwReceiverId, SUserInfo* pstUser);

    static TVOID GenDragonInfo(SDragonNode* pstNode, SActionMarchParam* pstMarch, Json::Value& rjson);
    static TVOID GenMonsterInfo(SMonsterNode* pstNode, Json::Value& rjson);

    static TVOID GenTradeReport(SSession *pstSession, STradeInfo *pstTrade, TbReport *ptbReport);

    static TVOID GenPayTaxReport(SSession *pstSession, TUINT32 *audwResource, TbReport* ptbReport);
    static TVOID GenCollectTaxReport(SSession *pstSession, TbReport* ptbReport);

    static TINT64 GetDefenderMajorId(SBattleNode* pstDefender, TbMarch_action *ptbMarch, TbMap *ptbWild, SUserInfo *pstTUser);
};
#endif

