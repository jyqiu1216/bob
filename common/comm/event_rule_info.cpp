#include "event_rule_info.h"

CEventRuleInfo *CEventRuleInfo::m_poIns = NULL;
CEventRuleInfo* CEventRuleInfo::m_poBackup = NULL;

CEventRuleInfo* CEventRuleInfo::GetInstance()
{
    if (m_poIns == NULL)
	{
        assert(0);
	}
    return m_poIns;
}

TINT32 CEventRuleInfo::Update()
{
    CEventRuleInfo *pTmp = new CEventRuleInfo();

    Json::Reader reader; 
    std::ifstream is;
    is.open(USER_EVENT_RULE_INFO_FILE, std::ios::binary);
    if (reader.parse(is, pTmp->m_jEventRule) == false)
    {
        is.close();
        delete(pTmp);
        return -1;
    }

    if (m_poBackup != NULL)
    {
        delete(m_poBackup);
        m_poBackup = NULL;
    }

    m_poBackup = m_poIns;

    m_poIns = pTmp;

    return 0;
}
