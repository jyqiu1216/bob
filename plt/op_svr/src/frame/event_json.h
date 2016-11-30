#ifndef _EVENT_JSON_H_
#define _EVENT_JSON_H_

#include "json_result_generator.h"
class CEventJson : public CBaseJson
{
public:
    CEventJson();
    virtual ~CEventJson();
    virtual TVOID GenDataJson(SSession* pstSession, Json::Value& rJson);  

};

#endif // !_REPORTLIST_JSON_H_
