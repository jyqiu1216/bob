#ifndef _COMM_JSON_H_
#define _COMM_JSON_H_

#include "jsoncpp/json/json.h"
#include "aws_table_include.h"
#include "bussiness_struct.h"
#include "db_struct_define.h"
#include "player_info.h"

class CCommJson 
{
public:
    static TVOID GenPlayerProfileJson(TbPlayer* ptbPlayer, TbEquip *atbEquip, TINT32 dwEquipNum, Json::Value& rJson, SPlayerBuffInfo *pstBuff = NULL);
    static TVOID GenAlMemberInfo(TbPlayer* ptbPlayer, Json::Value& rJson);
    static TVOID GenUniqueNameInfo(TbPlayer* ptbPlayer, Json::Value& rJson);
    static TVOID GenReportInfo(TbReport* ptbReport, TBOOL bReturnLarge, Json::Value& rjson);
    static TVOID GenReportInfo(TbReport* ptbReport, SReportEntry* pstEntry, TBOOL bReturnLarge, Json::Value& rjson);
    static TVOID GenTroopJson(SCommonTroop* pstTroop, Json::Value& rjson);
    static TVOID GenFortJson(SCommonFort* pstFort, Json::Value& rjson);
    static TVOID GenResourceJson(SCommonResource* pstRes, Json::Value& rjson);
    static TVOID GenResearchJson(SCommonResearch* pstResearch, Json::Value& rJson);
    static TVOID GenBuildingJson(SCityBuildingNode* pstBuildingNode, TUINT32 udwNodeNum, Json::Value& rJson);
    static TVOID GenReportBuildingJson(SCityBuildingNode* pstBuildingNode, TUINT32 udwNodeNum, Json::Value& rJson);
    static TVOID GenNameInfo(TbReport* ptbReport, Json::Value& rJson);
    static TVOID GenAllianceInfo(TbAlliance* ptbAlliance, Json::Value& rJson);
    static TVOID GenAllianceInfo(AllianceRank* pAlRank, TbAlliance* ptbAlliance, Json::Value& rJson);
    static TVOID GenStatJson(TINT64 ddwTime, TUINT32 udwNum, TBOOL bNeedMusic, TbUser_stat* ptbStat, Json::Value& rJson);
    static TVOID GenAlGiftJson(SUserInfo *pstUser, Json::Value& rJson);

    static TVOID GenActionInfoForPush(TbAction* ptbAction, Json::Value& rjson, TINT32 dwItemUpdtType = EN_TABLE_UPDT_FLAG__CHANGE);
    static TVOID GenMarchInfoForPush(TbMarch_action* ptbMarch, Json::Value& rjson, TINT32 dwItemUpdtType = EN_TABLE_UPDT_FLAG__CHANGE);
    static TVOID GenAlActionInfoForPush(TbAlliance_action* ptbAlAction, Json::Value& rjson, TINT32 dwItemUpdtType = EN_TABLE_UPDT_FLAG__CHANGE);

    static TVOID GenActionInfo(TbAction* ptbAction, Json::Value& rjson);
    static TVOID GenMarchInfo(TbMarch_action* ptbMarch, Json::Value& rjson);
    static TVOID GenAlActionInfo(TbAlliance_action* ptbAlAction, Json::Value& rjson);

    static TVOID GenMapBaseJson(TbMap* ptbMap, Json::Value& rJson);

    static TVOID GenEventTipsJson(SUserInfo* pstUserInfo, Json::Value& rjsonResult);
    //static TVOID GenThemeEventTipsJson(SUserInfo* pstUserInfo, Json::Value& rjsonResult);

    static vector<SEventPriority> GetSortEventTipsList(TbEvent_tips *atbEventTipsList, TUINT32 udwNum);

    static TBOOL CompareEventTips(SEventPriority sEventTipsA, SEventPriority sEventTipsB);

    static TUINT32 GetEventTipsPriority(SEventPriority sEventTips);

    static TVOID GenBountyInfo(SUserInfo *pstUser, TUINT32 udwCid, Json::Value& rJson);

    static TVOID GenTaskInfo(SUserInfo *pstUser, Json::Value& rJson);

    static TVOID GenSkillInfo(SSkill *pstSkill, Json::Value& rJson);

    static TVOID GenReportBufferInfo(SReportBuffer *pstBuff, Json::Value& rJson);

    static TVOID GenBattleBuffInfo(SPlayerBuffInfo *pstBuff, Json::Value& rJson);

    static TVOID GenTitleInfo(STitleInfoList *pstTitle, TbThrone *ptbThrone, Json::Value& rJson);

    //wave@20160709: for 增量更新
    static TBOOL IfTableDataNeedOutput(TINT32 dwType, AwsTable* ptbTbl, TINT32 dwItemUpdtType = EN_TABLE_UPDT_FLAG__CHANGE);

    //wave@20160823
    static TVOID PushData_GenMapWildJson(TUINT32 udwSvrId, TbMap* ptbMap, Json::Value& rJson);
    static TVOID PushData_GenMapJson(TUINT32 udwSvrId, TbMap* ptbMap, Json::Value& rJson);

};

#endif

