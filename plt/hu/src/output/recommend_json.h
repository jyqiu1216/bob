#ifndef _RECOMMEND_JSON_H_
#define _RECOMMEND_JSON_H_

#include "base_json.h"

class CRecommendJson : public CBaseJson
{
public:
    CRecommendJson();
    virtual ~CRecommendJson();
    virtual TVOID GenDataJson(SSession* pstSession, Json::Value& rJson);
};


#endif

