#ifndef _ALLIANCELIST_JSON_H_
#define _ALLIANCELIST_JSON_H_

#include "base_json.h"

class CAllianceListJson : public CBaseJson
{
public:
    CAllianceListJson();
    virtual ~CAllianceListJson();
    virtual TVOID GenDataJson(SSession* pstSession, Json::Value& rJson);
private:
    TVOID GenAllianceInfoList(TbAlliance* ptbAlliance, TUINT32 udwAllianceNum, Json::Value& rjson);
};


#endif