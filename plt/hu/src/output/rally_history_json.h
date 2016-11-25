#ifndef _RALLY_HISTORY_H_
#define _RALLY_HISTORY_H_

#include "base_json.h"

class CRallyHistoryJson : public CBaseJson
{
public:
    CRallyHistoryJson();
    virtual ~CRallyHistoryJson();
    virtual TVOID GenDataJson(SSession* pstSession, Json::Value& rJson);
};

#endif

