#ifndef _GLOBAL_GIFT_H_
#define _GLOBAL_GIFT_H_

#include "jsoncpp/json/json.h"
#include "game_define.h"
#include "base/log/wtselogger.h"
#include <set>
#include "bin_struct_define.h"

using namespace wtse::log;
using namespace std;

#define UPDATE_GLOBAL_GIFT_FLAG_FILE ("../data/update_global_gift_flag")

class CGlobalGift
{
public:
    static CGlobalGift* m_poGlobalGift;
    static CGlobalGift* GetInstance();
    static TINT32 Update(const TCHAR *pszFileName, CTseLogger *poLog);
public:
    TINT32 Init(const TCHAR* pszFileName, CTseLogger* poLog);
public:
    Json::Value		m_oJsonRoot;
    CTseLogger*		m_poLog;

};

#endif
