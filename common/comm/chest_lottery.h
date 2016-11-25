#ifndef _CHEST_LOTTERY_H_
#define _CHEST_LOTTERY_H_

#include "game_define.h"
#include "game_data.h"
#include "base/log/wtselogger.h"
#include "jsoncpp/json/json.h"
#include <map>

using namespace std;
using namespace wtse::log;

#define UPDATE_CHEST_LOTTERY_JSON_FLAG_FILE      ("../data/update_chest_lottery_flag")

class CChestLottery
{
public:
    static CChestLottery* m_poChestLottery;
    static CChestLottery* GetInstance();
    static  TINT32 Update(const TCHAR *pszFileName, CTseLogger *poLog);
public:
    TINT32			Init(const TCHAR *pszFileName, CTseLogger *poLog);

public:
    Json::Value		m_oJsonRoot;
    CTseLogger*		m_poLog;
};

#endif
