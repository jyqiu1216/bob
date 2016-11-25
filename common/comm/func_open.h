#ifndef _FUNC_OPEN_H_
#define _FUNC_OPEN_H_

#include "game_define.h"
#include "game_data.h"
#include "base/log/wtselogger.h"
#include "jsoncpp/json/json.h"
#include <map>

using namespace std;
using namespace wtse::log;

#define UPDATE_FUNC_OPEN_JSON_FLAG_FILE      ("../data/update_func_open_json_flag")

class CFuncOpen
{
public:
    static CFuncOpen* m_poFuncOpen;
    static CFuncOpen* GetInstance();
    static  TINT32 Update( const TCHAR *pszFileName, CTseLogger *poLog );
public:
	TINT32			Init(const TCHAR *pszFileName, CTseLogger *poLog);

public:
	Json::Value		m_oJsonRoot;
	CTseLogger*		m_poLog;
};

#endif
