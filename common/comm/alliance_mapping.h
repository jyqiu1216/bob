#ifndef _ALLIANCE_MAPPING_H_
#define _ALLIANCE_MAPPING_H_

#include "jsoncpp/json/json.h"
#include "game_define.h"
#include "base/log/wtselogger.h"
#include <set>
#include "bin_struct_define.h"

using namespace wtse::log;
using namespace std;

#define UPDATE_ALLIANCE_MAPPING_FLAG_FILE ("../data/update_alliance_mapping_flag")

class CAllianceMapping
{
public:
    static CAllianceMapping* m_poAllianceMapping;
    static CAllianceMapping* GetInstance();
    static TINT32 Update(const TCHAR *pszFileName, CTseLogger *poLog);
public:
    TINT32 Init(const TCHAR* pszFileName, CTseLogger* poLog);
public:
    Json::Value		m_oJsonRoot;
    CTseLogger*		m_poLog;

};

#endif
