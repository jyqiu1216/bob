#ifndef _DRAGON_TRAIL_CONTROL_H_
#define _DRAGON_TRAIL_CONTROL_H_

#include "jsoncpp/json/json.h"
#include "game_define.h"
#include "base/log/wtselogger.h"
#include <set>
#include "bin_struct_define.h"

using namespace wtse::log;
using namespace std;

#define UPDATE_DRAGON_TRAIL_CONTROL_FLAG_FILE ("../data/update_dragon_trail_control_flag")

class CDragonTrailControl
{
public:
    static CDragonTrailControl* m_poDragonTrailControl;
    static CDragonTrailControl* GetInstance();
    static TINT32 Update(const TCHAR *pszFileName, CTseLogger *poLog);
public:
    TINT32 Init(const TCHAR* pszFileName, CTseLogger* poLog);
public:
    Json::Value		m_oJsonRoot;
    CTseLogger*		m_poLog;

};

#endif
