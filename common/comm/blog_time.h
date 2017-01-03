#ifndef _BLOG_TIME_H_
#define _BLOG_TIME_H_

#include "jsoncpp/json/json.h"
#include "game_define.h"
#include "base/log/wtselogger.h"

using namespace wtse::log;
using namespace std;


#define UPDATE_BLOG_TIME_FLAG_FILE ("../data/blog_time_update_flag")

class CBlogTime
{
public:
    static CBlogTime* m_poBlogTime;
    static CBlogTime* GetInstance();
    static TINT32 Update(const TCHAR *pszFileName, CTseLogger *poLog);

    TINT32 Init(const TCHAR *pszFileName, CTseLogger *poLog);

public:
    Json::Value	m_jBlogTime;
    CTseLogger*	m_poLog;
};

#endif