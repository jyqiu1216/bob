#ifndef _WILD_INFO_H_
#define _WILD_INFO_H_

#include "game_define.h"
#include "game_data.h"
#include "base/log/wtselogger.h"
#include "jsoncpp/json/json.h"


#define UPDATE_WILD_RES_JSON_FLAG_FILE ("../data/wild_res_flag")
#define DEFAULT_WILD_RES_JSON_FILE ("../data/wild_res.json")
#define WILD_RES_JSON_FILE ("../data/wild_res")

using namespace wtse::log;

class CWildInfo
{
public:
    static CWildInfo* m_pWildInfo;
    static CWildInfo* GetInstance();
    // none lazy init Singleton, must init before worker threads begin to work
    static TINT32 Update(CTseLogger *poLog);
    static const Json::Value& GetWildResInfo(TUINT32 udwSvrId);
    TINT32 LoadWildRes(CTseLogger *poLog);
    
    TVOID Wild_Output( TCHAR *pszOut, TUINT32 &udwOutLen, TUINT32 udwSid,TUINT32 udwType,TUINT32 udwLevel);

    Json::Value m_jsWildResA[MAX_GAME_SVR_NUM];
    Json::Value m_jsWildResB[MAX_GAME_SVR_NUM];
    Json::Value* m_pjsMajorWildRes;
    Json::Value* m_pjsBufferWildRes;
    static TBOOL m_bHasInitedWildRes;
private:
};

#endif