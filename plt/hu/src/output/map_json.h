#ifndef _MAP_JSON_H_
#define _MAP_JSON_H_

#include "json_result_generator.h"
class CMapJson : public CBaseJson
{
public:
    CMapJson();
    virtual ~CMapJson();
    virtual TVOID GenDataJson(SSession* pstSession, Json::Value& rJson);
private:
    TVOID GenMapWildJson(SSession* pstSession, Json::Value& rJson,TUINT32 udwIdx);
    
private:
};

#endif // !_REPORTLIST_JSON_H_
