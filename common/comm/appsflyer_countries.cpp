#include "appsflyer_countries.h"
#include <algorithm>
#include <string>

using namespace std;

CAppsflyerCountries *CAppsflyerCountries::m_poIns = NULL;
CAppsflyerCountries* CAppsflyerCountries::m_poBackup = NULL;

CAppsflyerCountries* CAppsflyerCountries::GetInstance()
{
    if (m_poIns == NULL)
	{
        assert(0);
	}
    return m_poIns;
}

TINT32 CAppsflyerCountries::Update()
{
    CAppsflyerCountries *pTmp = new CAppsflyerCountries();

    if (pTmp->GenCountriesSet() != 0)
    {
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

TINT32 CAppsflyerCountries::GenCountriesSet()
{
    m_pIP2Location = IP2Location_open(IP2COUNTRIES_FILE);

    if (m_pIP2Location == NULL)
    {
        return -1;
    }

    m_setCountries.clear();
    TCHAR szLine[512];

    FILE *fp = fopen(COUNTRIES_FILE, "rt");
    if (fp == NULL)
    {
        return -1;
    }

    memset(szLine, 0, 512);
    while (fgets(szLine, 512, fp))
    {
        for (TUINT32 udwIdx = 0; udwIdx < strlen(szLine); udwIdx++)
        {
            tolower(szLine[udwIdx]);
            if (szLine[udwIdx] == '\n'
                || szLine[udwIdx] == '\r')
            {
                szLine[udwIdx] = 0;
                break;
            }
        }
        m_setCountries.insert(szLine);
        memset(szLine, 0, 512);
    }

    fclose(fp);

    return 0;
}

TINT32 CAppsflyerCountries::GetAppsflyerFlag(const char* szIp)
{
    IP2LocationRecord *record = IP2Location_get_all(m_pIP2Location, szIp);

    if (record == NULL)
    {
        return 0;
    }

    for (TUINT32 udwIdx = 0; udwIdx < strlen(record->country_long); udwIdx++)
    {
        record->country_long[udwIdx] = tolower(record->country_long[udwIdx]);
    }

    TINT32 dwFlag = 0;
    if (m_setCountries.count(record->country_long) > 0)
    {
        dwFlag = 1;
    }

    IP2Location_free_record(record);

    return dwFlag;
}