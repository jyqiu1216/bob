#ifndef _LOCK_INFO_H_
#define _LOCK_INFO_H_

#include "jsoncpp/json/json.h"
#include "game_define.h"
#include "base/log/wtselogger.h"
#include <set>
#include "bin_struct_define.h"

using namespace wtse::log;
using namespace std;

#define UPDATE_LOCK_INFO_FLAG_FILE ("../data/update_lock_info_flag")

class CLockInfo
{
public:
    static CLockInfo* m_poLockInfo;
    static CLockInfo* GetInstance();
    static TINT32 Update(const TCHAR *pszFileName, CTseLogger *poLog);
public:
    TINT32 Init(const TCHAR* pszFileName, CTseLogger* poLog);
public:
    Json::Value        m_oJsonRoot;
    CTseLogger*        m_poLog;

};

#endif