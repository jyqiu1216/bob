#ifndef _JSON_GEN_INTERFACE_H_
#define _JSON_GEN_INTERFACE_H_

#include "jsoncpp/json/json.h"
#include "session.h"

class CBaseJson
{
public:
    CBaseJson() {};
    virtual ~CBaseJson() {};

    virtual TVOID GenDataJson(SSession* pstSession, Json::Value& rJson) = 0;
};

#endif
