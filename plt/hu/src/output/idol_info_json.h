#ifndef _IDOL_INFO_JSON_H_
#define _IDOL_INFO_JSON_H_

#include "json_result_generator.h"
class CIdolInfoJson : public CBaseJson
{
public:
    CIdolInfoJson();
    virtual ~CIdolInfoJson();
    virtual TVOID GenDataJson(SSession* pstSession, Json::Value& rJson);

private:
};

#endif // !_IDOL_INFO_JSON_H_
