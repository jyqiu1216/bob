#ifndef _RATING_USER_INFO_H_
#define _RATING_USER_INFO_H_

#include "jsoncpp/json/json.h"
#include "game_define.h"
#include "base/log/wtselogger.h"
#include <set>

using namespace wtse::log;
using namespace std;

#define UPDATE_RATING_USER_INFO_FLAG_FILE ("../data/rating_user_info_flag")
#define DEFAULT_RATING_USER_INFO_EVENT_FILE ("../data/rating_user.json")
#define RATING_USER_INFO_EVENT_FILE ("../data/rating_user")

struct SRatingUserInfo
{
    TUINT64 uddwUId;
    TUINT64 uddwRatingTime;
    TUINT32 udwGem;

    TVOID Reset()
    {
        memset((char*)this, 0, sizeof(*this));
    }
};

class CRatingUserInfo
{
public:
    static CRatingUserInfo* m_poCRatingUserInfo;
    static CRatingUserInfo* GetInstance();
    static TINT32 Update(CTseLogger *poLog);
public:
    TINT32 Init(CTseLogger *poLog);

public:
    Json::Value m_oJsonRoot;
    CTseLogger* m_poLog;

};

#endif
