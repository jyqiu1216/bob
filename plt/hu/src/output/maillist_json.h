#ifndef _MAILLIST_JSON_H_
#define _MAILLIST_JSON_H_

#include "base_json.h"

class CMailListJson : public CBaseJson
{
public:
    CMailListJson();
    virtual ~CMailListJson();
    virtual TVOID GenDataJson(SSession* pstSession, Json::Value& rJson);
private:
    TVOID GenMailInfo(SMailEntry* pstMailEntry, TbMail* ptbMail, Json::Value& rjson);
};


#endif