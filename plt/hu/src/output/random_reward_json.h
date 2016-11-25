#ifndef _RANDOM_REWARD_JSON_H_
#define _RANDOM_REWARD_JSON_H_

#include "json_result_generator.h"
class CRandomRewardJson : public CBaseJson
{
public:
    CRandomRewardJson();
    virtual ~CRandomRewardJson();
    virtual TVOID GenDataJson(SSession* pstSession, Json::Value& rJson);

private:
};

#endif // !_RANDOM_REWARD_JSON_H_
