#ifndef _SVR_LIST_JSON_H_
#define _SVR_LIST_JSON_H_

#include "base_json.h"

class CSvrListJson : public CBaseJson
{
public:
    CSvrListJson();
    virtual ~CSvrListJson();
    virtual TVOID GenDataJson(SSession* pstSession, Json::Value& rJson);

private:
    TVOID GenBaseDataJson(SSession* pstSession, Json::Value& rJson, TUINT32 udwSvrIdx,TUINT32 udwThroneMapIdx);
};

#endif