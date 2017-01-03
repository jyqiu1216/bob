#ifndef _APPSFLYER_COUNTRIES_H_
#define _APPSFLYER_COUNTRIES_H_
#include <stdio.h>
#include "base/common/wtse_std_header.h"
#include <set>
#include "libIP2Location/IP2Location.h"

using namespace std;

#define UPDATE_COUNTRIES_FLAG ("../data/appsflyer_countries_update_flag")
#define COUNTRIES_FILE ("../data/appsflyer_countries.file")
#define IP2COUNTRIES_FILE ("../data/ip-country.bin")

class CAppsflyerCountries
{
private:
    static CAppsflyerCountries* m_poIns;
    static CAppsflyerCountries* m_poBackup;

public:
    static CAppsflyerCountries* GetInstance();
    static TINT32 Update();

public:
    TINT32 GetAppsflyerFlag(const char* szIp);
    ~CAppsflyerCountries()
    {
        if (m_pIP2Location != NULL)
        {
            IP2Location_close(m_pIP2Location);
        }
    }
private:
    CAppsflyerCountries()
    {
        m_setCountries.clear();
        m_pIP2Location = NULL;
    }

    TINT32 GenCountriesSet();

    IP2Location *m_pIP2Location;

    set<string> m_setCountries;
};

#endif


