#include "recommend_json.h"
#include "common_json.h"

CRecommendJson::CRecommendJson()
{

}

CRecommendJson::~CRecommendJson()
{

}

TVOID CRecommendJson::GenDataJson(SSession* pstSession, Json::Value& rJson)
{
    //"svr_recommend_player":
    //{
    //    "next_time": int,   //下次刷新时间
    //    "list" : [
    //        {
    //            "sid": int,
    //            "uid" : int,
    //            "uname" : string,
    //            "force" : long,
    //            "avatar" : int,    // 国王形象
    //            "lord_lv" : int,
    //            "castle_lv" : int//主城等级
    //        }
    //    ]
    //}

    Json::Value& rJsonNode = rJson["svr_recommend_player"];
    rJsonNode = Json::Value(Json::objectValue);
    TUINT32 udwCurtime = CTimeUtils::GetUnixTime();
    if(pstSession->m_stRecommendTime.ddwTime + EN_INTIVE_LIMIT_TIME < udwCurtime)
    {
        rJsonNode["next_time"] = udwCurtime + EN_INTIVE_LIMIT_TIME;
    }
    else
    {
        rJsonNode["next_time"] = pstSession->m_stRecommendTime.ddwTime + EN_INTIVE_LIMIT_TIME;
    }
    rJsonNode["updt_time"] = pstSession->m_stRecommendTime.ddwTime;
    rJsonNode["list"] = Json::Value(Json::arrayValue);
    for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_udwRecommendNum; ++udwIdx)
    {
        rJsonNode["list"][udwIdx] = Json::Value(Json::objectValue);
        rJsonNode["list"][udwIdx]["sid"] = pstSession->m_aRecommendPlayer[udwIdx].udwSid;
        rJsonNode["list"][udwIdx]["uid"] = pstSession->m_aRecommendPlayer[udwIdx].udwUid;
        rJsonNode["list"][udwIdx]["uname"] = pstSession->m_aRecommendPlayer[udwIdx].sUname;
        rJsonNode["list"][udwIdx]["force"] = pstSession->m_aRecommendPlayer[udwIdx].ddwForce;
        rJsonNode["list"][udwIdx]["avatar"] = pstSession->m_aRecommendPlayer[udwIdx].udwAvatar;
        rJsonNode["list"][udwIdx]["lord_lv"] = pstSession->m_aRecommendPlayer[udwIdx].udwLordLv;
        rJsonNode["list"][udwIdx]["castle_lv"] = pstSession->m_aRecommendPlayer[udwIdx].udwCastleLv;
        //wave@20160127
        rJsonNode["list"][udwIdx]["invited"] = pstSession->m_aRecommendPlayer[udwIdx].udwInvited;
    }

    Json::Value& rJsonStat = rJson["svr_stat"];
    CCommJson::GenStatJson(pstSession->m_stRecommendTime.ddwTime,
        pstSession->m_udwRecommendNum,
        pstSession->m_bNeedMailMusic,
        &pstSession->m_stUserInfo.m_tbUserStat,
        rJsonStat);
}
