#ifndef _THRONE_INFO_JSON_H_
#define _THRONE_INFO_JSON_H_

#include "json_result_generator.h"
class CThroneInfoJson : public CBaseJson
{
public:
    CThroneInfoJson();
    virtual ~CThroneInfoJson();
    virtual TVOID GenDataJson(SSession* pstSession, Json::Value& rJson);

private:
    TVOID GenThroneJson(SSession* pstSession, Json::Value& rJson);
    TVOID GenTitleJson(SSession* pstSession, Json::Value& rJson);

};

#endif // !_IDOL_INFO_JSON_H_
