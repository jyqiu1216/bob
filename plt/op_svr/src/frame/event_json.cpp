#include "event_json.h"
#include "time_utils.h"
#include "action_base.h"
#include "player_info.h"
#include "common_json.h"
#include "game_define.h"
#include "game_command.h"
#include <map>
using namespace std;

CEventJson::CEventJson()
{

}

CEventJson::~CEventJson()
{

}

TVOID CEventJson::GenDataJson(SSession* pstSession, Json::Value& rJson)
{
    /*
    "svr_event_info":
    {
        //event_info
    }
    */
    /*if(pstSession->m_udwCommand == EN_CLIENT_ALL_EVENT_GET ||
        EN_CLIENT_EVENT_INFO_GET == pstSession->m_udwCommand)
    {*/
    if (pstSession->m_udwCommand == EN_CLIENT_REQ_COMMAND__ALL_EVENT_GET)
        {
        
        Json::Value& jsonEventInfo = rJson["svr_event_list"];
        Json::Value& jThemeEventInfo = rJson["svr_theme_event_list"];

        jsonEventInfo = Json::Value(Json::objectValue);
        jThemeEventInfo = Json::Value(Json::objectValue);

        Json::Reader reader;

        Json::Value oEventListJson;
        if (pstSession->m_vecEventRsp[0]->dwRetCode == 0
            && FALSE != reader.parse(pstSession->m_vecEventRsp[0]->sRspContent, oEventListJson))
        {
            jsonEventInfo = oEventListJson["svr_event_list"];
        }
        if (pstSession->m_vecEventRsp[1]->dwRetCode == 0
            && FALSE != reader.parse(pstSession->m_vecEventRsp[1]->sRspContent, oEventListJson))
        {
            jThemeEventInfo = oEventListJson["svr_theme_event_list"];
        }

        return;
    }
}

