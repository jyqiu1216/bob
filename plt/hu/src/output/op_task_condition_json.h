#ifndef _OP_TASK_CONDITION_JSON_H_
#define _OP_TASK_CONDITION_JSON_H_

#include "json_result_generator.h"
class COpTaskConditionJson : public CBaseJson
{
public:
    COpTaskConditionJson();
    virtual ~COpTaskConditionJson();
    virtual TVOID GenDataJson(SSession* pstSession, Json::Value& rJson);

private:
};

#endif // !_REPORTLIST_JSON_H_
