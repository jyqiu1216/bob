#ifndef _IAP_WHITE_LIST_H_
#define _IAP_WHITE_LIST_H_

#include "jsoncpp/json/json.h"
#include "game_define.h"
#include "base/log/wtselogger.h"
#include <set>

using namespace wtse::log;
using namespace std;

#define UPDATE_IAP_WHITE_LIST_FLAG_FILE ("../data/iap_white_list_flag")
#define DEFAULT_IAP_WHITE_LIST_FILE ("../data/iap_white_list.json")
#define IAP_WHITE_LIST_FILE ("../data/iap_white_list")

struct SIAPWhiteList
{
    TUINT64 uddwUId;
    TUINT64 uddwWeekIapNum;

    TVOID Reset()
    {
        memset((char*)this, 0, sizeof(*this));
    }
};

class CIAPWhiteList
{
public:
    static CIAPWhiteList* m_poCIAPWhiteList;
    static CIAPWhiteList* GetInstance();
    static TINT32 Update(CTseLogger *poLog);
    static TBOOL bIsWhitePlayer(TINT64 ddwUid,TFLOAT32 &fMaxWeekIap);
public:
    TINT32 Init(CTseLogger *poLog);

public:
    Json::Value m_oJsonRoot;
    CTseLogger* m_poLog;

};

#endif
