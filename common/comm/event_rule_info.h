#ifndef _EVENT_RULE_INFO_H_
#define _EVENT_RULE_INFO_H_
#include <stdio.h>
#include "base/common/wtse_std_header.h"
#include "jsoncpp/json/json.h"

#define UPDATE_EVENT_RULE_INFO_FLAG ("../data/update_event_rule_info_flag")
#define USER_EVENT_RULE_INFO_FILE ("../data/event_rule_info.json")

class CEventRuleInfo
{
private:
    static CEventRuleInfo* m_poIns;
    static CEventRuleInfo* m_poBackup;

    CEventRuleInfo()
    {
        m_jEventRule.clear();
    }

public:
    ~CEventRuleInfo()
    {
        m_jEventRule.clear();
    }
    static CEventRuleInfo* GetInstance();
    static TINT32 Update();

public:

    Json::Value m_jEventRule;
};

#endif


