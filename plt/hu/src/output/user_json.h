#ifndef _USER_JSON_H_
#define _USER_JSON_H_

#include "base_json.h"

class CUserJson : public CBaseJson
{
public:
    CUserJson();
    virtual ~CUserJson();
    virtual TVOID GenDataJson(SSession* pstSession, Json::Value& rJson);

private:
    TVOID GenLoginJson(SSession* pstSession, Json::Value& rjsonResult);
    TVOID GenPlayerJson(SSession* pstSession, Json::Value& rjsonResult);
    TVOID GenResearchJson(SSession* pstSession, Json::Value& rJson);

    TVOID GenLordSkillJson(SSession* pstSession, Json::Value& rJson);
    TVOID GenDragonSkillJson(SSession* pstSession, Json::Value& rJson);
    TVOID GenDragonMonsterSkillJson(SSession* pstSession, Json::Value& rJson);

    TVOID GenDailyLoginJson(SSession* pstSession, Json::Value& rJson);
    TVOID GenCityJson(SSession* pstSession, Json::Value& rjsonResult);
    TVOID GenBagJson(SSession* pstSession, Json::Value& rjsonResult);
    TVOID GenTopQuestJson(SSession* pstSession, Json::Value& rjsonResult);
    TVOID GenBlackListJson(SSession* pstSession, Json::Value& rjsonResult);
    TVOID GenBookMarkListJson(SSession* pstSession, Json::Value& rjsonResult);
    TVOID GenStatJson(SSession* pstSession, Json::Value& rjsonResult);
    TVOID GenActionNewJson(SSession* pstSession, Json::Value& rjsonResult);
    TVOID GenTipsJson(SSession* pstSession, Json::Value& rjsonResult);
    TVOID GenAllianceJson(SSession* pstSession, Json::Value& rjsonResult);
    TVOID GenDiplomacyJson(SSession* pstSession, Json::Value& rjsonResult);
    TVOID GenAlStoreJson(SSession* pstSession, Json::Value& rjsonResult);
    TVOID GenClientFlag(SSession* pstSession, Json::Value& rjsonResult);
    TVOID GenTimeQuestJson(SSession* pstSession, Json::Value& rjsonResult);
    TVOID GenQuestNodeJson(SQuestNode *pstQuestNode, Json::Value& rjsonResult);
    
    TVOID GenAltarJson(SSession* pstSession, SCityInfo* pstCity, Json::Value& rjson);
    TVOID GenProductionJson(SCityInfo* pstCity, Json::Value& rjson);

    //TVOID GenQuestNodeJson(SQuestNode *pstQuestNode, Json::Value& rjsonResult);
    TVOID GenBuffJson(SSession* pstSession, Json::Value& rJson);
    TVOID GenBackPackJson(SSession* pstSession, Json::Value& rJson);
    TVOID GenRewardWindowJson(SSession* pstSession, Json::Value& rJson);

    TVOID GenTitleJson(SSession* pstSession, Json::Value& rjsonResult);
    TVOID GenPrisonJson(SSession* pstSession, Json::Value& rJson);

    TVOID GenSvrListJson(SSession* pstSession, Json::Value& rJson);
    TVOID GenSvrJson(SSession* pstSession, Json::Value& rJson, TUINT32 udwSvrIdx, TUINT32 udwThroneMapIdx);

    TVOID GenBountyInfo(SSession* pstSession, Json::Value& rJson);

    TVOID GenBroadcastInfo(SSession *pstSession, Json::Value& rJson);

    TVOID GenKnightInfo(SSession *pstSession, Json::Value& rJson);

    TVOID GenEventInfo(SSession *pstSession, Json::Value& rJson);

    TVOID GenRewardWindowNewInfo(SSession *pstSession, Json::Value& rJson);

    TVOID GenRandomRewardInfo(SSession *pstSession, Json::Value& rJson);

    TVOID GenMonsterInfo(SSession *pstSession, Json::Value& rJson);

    TVOID GenTrialInfo(SSession *pstSession, Json::Value& rJson);

    TVOID GenComputeResInfo(SSession *pstSession, Json::Value& rJson);

    TVOID GenGemRechargeInfo(SSession *pstSession, Json::Value& rJson);

    TVOID GenIdolInfo(SSession *pstSession, Json::Value& rJson);

    TVOID GenThroneInfo(SSession *pstSession, Json::Value& rJson);
};

#endif