#ifndef _AL_REPORTLIST_JSON_H
#define _AL_REPORTLIST_JSON_H

#include "base_json.h"

class CAlReportListJson : public CBaseJson
{
public:
    CAlReportListJson();
    virtual ~CAlReportListJson();
    virtual TVOID GenDataJson(SSession* pstSession, Json::Value& rJson);
};

#endif

