#ifndef _ASSIST_LIST_H_ 
#define _ASSIST_LIST_H_ 

#include "base_json.h"
class CAssistListJson : public CBaseJson
{
public:
    CAssistListJson();
    virtual ~CAssistListJson();
    virtual TVOID GenDataJson(SSession* pstSession, Json::Value& rJson);

private:
};

#endif