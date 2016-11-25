#include "alliancelist_json.h"
#include "game_command.h"
#include "common_base.h"
#include "common_json.h"

TVOID CAllianceListJson::GenDataJson(SSession* pstSession, Json::Value& rJson)
{
    //"svr_alliance_friend": //友好联盟
    //{
    //    "total_page_num": 15,    //总的页数
    //    "cur_page_num" : 15, //当前这一页返回的数目
    //    "list" :             //数组长度等于cur_page_num
    //    [
    //    ]
    //},
    //"svr_alliance_hostile": //敌对联盟
    //{
    //    "total_page_num": 15,    //总的页数
    //    "cur_page_num" : 15, //当前这一页返回的数目
    //    "list" :             //数组长度等于cur_page_num
    //    [
    //    ]
    //},
    //al_info_get
    //只会返回一个目标联盟
    //"svr_al_info":{
    //}

    if(pstSession->m_udwCommand == EN_CLIENT_REQ_COMMAND__ALLIANCE_DIPLOMACY_GET)
    {
        GenAllianceInfoList(pstSession->m_atbFriendAl, pstSession->m_udwFriendAlNum, rJson["svr_alliance_friend"]);
        GenAllianceInfoList(pstSession->m_atbHostileAl, pstSession->m_udwHostilesAlNum, rJson["svr_alliance_hostile"]);
    }
    else if(pstSession->m_udwCommand == EN_CLIENT_REQ_COMMAND__ALLIANCE_INFO_GET ||
        pstSession->m_udwCommand == EN_CLIENT_OPERATE_CMD__AL_INFO_GET)
    {
        CCommJson::GenAllianceInfo(&pstSession->m_tbTmpAlliance, rJson["svr_al_info"]);
    }
}

CAllianceListJson::CAllianceListJson()
{

}

CAllianceListJson::~CAllianceListJson()
{

}

TVOID CAllianceListJson::GenAllianceInfoList(TbAlliance* ptbAlliance, TUINT32 udwAllianceNum, Json::Value& rJson)
{
    rJson = Json::Value(Json::objectValue);
    rJson["total_page_num"] = 1;
    rJson["cur_page_num"] = udwAllianceNum;
    rJson["list"] = Json::Value(Json::arrayValue);
    for(TUINT32 udwIdx = 0; udwIdx < udwAllianceNum; ++udwIdx)
    {
        CCommJson::GenAllianceInfo(&(ptbAlliance[udwIdx]), rJson["list"][udwIdx]);
    }
}