#ifndef _BACKPACK_INFO_H_
#define _BACKPACK_INFO_H_

#include "game_define.h"
#include "game_data.h"
#include "base/log/wtselogger.h"
#include "jsoncpp/json/json.h"
#include <map>

using namespace std;
using namespace wtse::log;

#define UPDATE_EQUIP_JSON_FLAG_FILE      ("../data/update_equip_json_flag")

class CBackpackInfo
{
public:
	static CBackpackInfo* m_poBackpackInfo;
	static CBackpackInfo* GetInstance();
    static  TINT32 Update( const TCHAR *pszFileName, CTseLogger *poLog );
public:
	TINT32 Init(const TCHAR *pszFileName, CTseLogger *poLog);

public:
	Json::Value		m_oJsonRoot;
	CTseLogger*		m_poLog;
};

#endif
