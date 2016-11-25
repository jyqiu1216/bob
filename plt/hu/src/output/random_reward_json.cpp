#include "random_reward_json.h"
#include "user_json.h"
#include "time_utils.h"
#include "action_base.h"
#include "player_info.h"
#include "common_json.h"
#include "wild_info.h"
#include "game_define.h"
#include "common_func.h"
#include "game_command.h"

CRandomRewardJson::CRandomRewardJson()
{

}

CRandomRewardJson::~CRandomRewardJson()
{

}

TVOID CRandomRewardJson::GenDataJson(SSession* pstSession, Json::Value& rJson)
{
    Json::Value& jsonRandomReward = rJson["svr_random_reward_info"];
    jsonRandomReward = Json::Value(Json::arrayValue);

    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();

    for (TUINT32 udwIdx = 0, udwIdy = 0; udwIdx < pstSession->m_stUserInfo.m_udwRandomRewardNum; udwIdx++)
    {
        if (pstSession->m_stUserInfo.m_aucRandomRewardFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL
            || udwCurTime >= pstSession->m_stUserInfo.m_atbRandomReward[udwIdx].m_nEtime)
        {
            continue;
        }
        jsonRandomReward[udwIdy] = Json::Value(Json::objectValue);
        jsonRandomReward[udwIdy]["type"] = pstSession->m_stUserInfo.m_atbRandomReward[udwIdx].m_nType;
        jsonRandomReward[udwIdy]["num"] = pstSession->m_stUserInfo.m_atbRandomReward[udwIdx].m_nNum;
        jsonRandomReward[udwIdy]["etime"] = pstSession->m_stUserInfo.m_atbRandomReward[udwIdx].m_nEtime;
        udwIdy++;
    }
}
