#ifndef _USER_JSON_H_
#define _USER_JSON_H_

#include "session.h"

class CUserJson
{
public:
    CUserJson();
    ~CUserJson();
    TVOID GenDataJson(SSession* pstSession, SUserInfo *pstUser, Json::Value& rJson);

private:
    TVOID GenLoginJson(SUserInfo *pstUser, Json::Value& rjsonResult);
    TVOID GenPlayerJson(SSession* pstSession, SUserInfo *pstUser, Json::Value& rjsonResult);
    TVOID GenResearchJson(SUserInfo *pstUser, Json::Value& rJson);

    TVOID GenKnightInfo(SUserInfo *pstUser, Json::Value& rJson);

    TVOID GenCityJson(SUserInfo *pstUser, Json::Value& rjsonResult);
    TVOID GenBagJson(SUserInfo *pstUser, Json::Value& rjsonResult);
    TVOID GenTopQuestJson(SUserInfo *pstUser, Json::Value& rjsonResult);

    TVOID GenStatJson(SUserInfo *pstUser, Json::Value& rjsonResult);

    TVOID GenActionNewJson(SUserInfo *pstUser, Json::Value& rjsonResult);

    TVOID GenAllianceJson(SUserInfo *pstUser, Json::Value& rjsonResult);
    TVOID GenDiplomacyJson(SUserInfo *pstUser, Json::Value& rjsonResult);

    TVOID GenTimeQuestJson(SUserInfo *pstUser, Json::Value& rjsonResult);
    TVOID GenQuestNodeJson(SQuestNode *pstQuestNode, Json::Value& rjsonResult);
    
    TVOID GenAltarJson(SCityInfo* pstCity, Json::Value& rjson);
    TVOID GenProductionJson(SCityInfo* pstCity, Json::Value& rjson);

    TVOID GenBuffJson(SUserInfo *pstUser, Json::Value& rJson);
    TVOID GenBackPackJson(SUserInfo *pstUser, Json::Value& rJson);

    TVOID GenPrisonJson(SUserInfo *pstUser, Json::Value& rJson);
};

#endif