#ifndef _PLAYERLIST_JSON_H_
#define _PLAYERLIST_JSON_H_

#include "base_json.h"

class CPlayerListJson : public CBaseJson
{
public:
    CPlayerListJson();
    virtual ~CPlayerListJson();
    virtual TVOID GenDataJson(SSession* pstSession, Json::Value& rJson);
};

#endif