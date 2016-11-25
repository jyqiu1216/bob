#ifndef _RALLYWAR_INFO_H_
#define _RALLYWAR_INFO_H_

#include "base_json.h"

class CRallyWarInfoJson : public CBaseJson
{
public:
    CRallyWarInfoJson();
    virtual ~CRallyWarInfoJson();
    virtual TVOID GenDataJson(SSession* pstSession, Json::Value& rJson);
};

#endif

