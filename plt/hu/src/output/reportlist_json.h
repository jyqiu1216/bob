#ifndef _REPORTLIST_JSON_H_
#define _REPORTLIST_JSON_H_

#include "base_json.h"

class CReportListJson : public CBaseJson
{
public:
    CReportListJson();
    virtual ~CReportListJson();
    virtual TVOID GenDataJson(SSession* pstSession, Json::Value& rJson);
};

#endif // !_REPORTLIST_JSON_H_
