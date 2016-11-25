#include "assist_list_json.h"
#include "game_info.h"
#include "common_base.h"
#include "common_json.h"

CAssistListJson::CAssistListJson()
{

}

CAssistListJson::~CAssistListJson()
{

}

TVOID CAssistListJson::GenDataJson(SSession* pstSession, Json::Value& rJson)
{
    //"svr_al_assist_list":[//联盟内玩家申请的协助
    //    [
    //        1, //id
    //        0, //type
    //        1, //uid
    //        "uname", //uname
    //        "desc", //desc
    //        100100, //cid
    //        1,      //aid
    //        100000, //time
    //        [
    //            670800,         //gold
    //            790853,         //food
    //            734369,         //wood
    //            6281,           //stone
    //            990491          //ore
    //        ]
    //    ]
    //]
    SUserInfo* pstUser = &pstSession->m_stUserInfo;
    Json::Value& jsonAssistList = rJson["svr_al_assist_list"];
    TUINT32 udwCurtime = CTimeUtils::GetUnixTime();

    jsonAssistList = Json::Value(Json::arrayValue);
    TUINT32 udwTimeEnd = udwCurtime - CCommonBase::GetGameBasicVal(EN_GAME_BASIC_ASSIST_INTERVAL);
    for(TUINT32 udwIdx = 0, udwJsonIndex = 0; udwIdx < pstUser->m_udwAlAssistAllNum; ++udwIdx)
    {
        if(pstUser->m_aucAlAssistAllFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }
        if(pstUser->m_atbAlAssistAll[udwIdx].m_nTime <= udwTimeEnd)
        {
            continue;
        }
        if(pstUser->m_atbAlAssistAll[udwIdx].m_nUid != pstUser->m_tbPlayer.m_nUid
        && pstUser->m_atbAlAssistAll[udwIdx].m_nTime < udwCurtime - CCommonBase::GetGameBasicVal(EN_GAME_BASIC_ASSIST_AVAILABLE_TIME))
        {
            continue;
        }

        jsonAssistList[udwJsonIndex] = Json::Value(Json::arrayValue);
        jsonAssistList[udwJsonIndex][0u] = (pstUser->m_atbAlAssistAll[udwIdx].m_nId);
        jsonAssistList[udwJsonIndex][1u] = (pstUser->m_atbAlAssistAll[udwIdx].m_nType);
        jsonAssistList[udwJsonIndex][2u] = (pstUser->m_atbAlAssistAll[udwIdx].m_nUid);
        jsonAssistList[udwJsonIndex][3u] = (pstUser->m_atbAlAssistAll[udwIdx].m_sUname);
        jsonAssistList[udwJsonIndex][4u] = (pstUser->m_atbAlAssistAll[udwIdx].m_sDesc);
        jsonAssistList[udwJsonIndex][5u] = (pstUser->m_atbAlAssistAll[udwIdx].m_nCid);
        jsonAssistList[udwJsonIndex][6u] = (pstUser->m_atbAlAssistAll[udwIdx].m_nAid);
        jsonAssistList[udwJsonIndex][7u] = (pstUser->m_atbAlAssistAll[udwIdx].m_nTime);
        jsonAssistList[udwJsonIndex][8u] = Json::Value(Json::arrayValue);

        TINT32 adwResourceNeed[EN_RESOURCE_TYPE__END];
        for(TUINT32 udwResType = 0; udwResType < EN_RESOURCE_TYPE__END; udwResType++)
        {
            adwResourceNeed[udwResType] = pstUser->m_atbAlAssistAll[udwIdx].m_bParam[0].m_addwNum[udwResType] - pstUser->m_atbAlAssistAll[udwIdx].m_bProgress[0].m_addwNum[udwResType];
            if(adwResourceNeed[udwResType] < 0)
            {
                adwResourceNeed[udwResType] = 0;
            }
            jsonAssistList[udwJsonIndex][8u].append(adwResourceNeed[udwResType]);
        }

        jsonAssistList[udwJsonIndex][9u] = Json::Value(Json::arrayValue);
        for(TUINT32 udwResType = 0; udwResType < EN_RESOURCE_TYPE__END; udwResType++)
        {
            jsonAssistList[udwJsonIndex][9u].append(pstUser->m_atbAlAssistAll[udwIdx].m_bParam[0].m_addwNum[udwResType]);
        }

        ++udwJsonIndex;
    }

    //"svr_alliance": {
    //    "base":[
    //        10,  //aid
    //        "Rule_the_World", //alname
    //        248886, //owner_id
    //        "KnightKing", //owner_name
    //        "desc",
    //        "Notice",
    //        78,   //member num
    //        2111,  //force
    //        0, //目前的联盟加入政策
    //        0, //联盟语言
    //        1111111, //公告修改的时间
    //        1, //联盟礼物点数
    //        1  //联盟基金
    //        2, //联盟旗帜@v1.1
    //        string //联盟简称
    //    ],
    //    "al_join_req_num": 0,       //申请加入联盟的用户数量
    //    "can_help_action_num" : 0,   // 可以帮助的action数目 
    //    "can_help_action_list" : [    // 可以帮助的action的 id 列表 具体action从svr_al_action_list获取
    //    ],
    //    "wall_msg_num" : 0,
    //    "al_gift_can_open_num" : 0,  //可以打开的联盟礼物数量
    //    "al_gift_list" : [
    //         [111, 111, 111, 0, 500, [1, 1, 1]]        //[gift_id, pack_id, end_time, status, gift_point, [type, id, num]]
    //    ],
    //    "can_assist_num" : 0        //未读的可以assist的数量
    //}

    TbAlliance* ptbAlliance = &pstUser->m_tbAlliance;

    Json::Value& rjsonAlliance = rJson["svr_alliance"];
    rjsonAlliance = Json::Value(Json::objectValue);
    rjsonAlliance["base"] = Json::Value(Json::arrayValue);
    rjsonAlliance["base"].append(ptbAlliance->m_nAid);
    rjsonAlliance["base"].append(ptbAlliance->m_sName);
    rjsonAlliance["base"].append(ptbAlliance->m_nOid);
    rjsonAlliance["base"].append(ptbAlliance->m_sOname);
    rjsonAlliance["base"].append(ptbAlliance->m_sDesc);
    rjsonAlliance["base"].append(ptbAlliance->m_sNotice);
    rjsonAlliance["base"].append(ptbAlliance->m_nMember);
    rjsonAlliance["base"].append(ptbAlliance->m_nMight);
    rjsonAlliance["base"].append(ptbAlliance->m_nPolicy / ALLIANCE_POLICY_OFFSET);
    rjsonAlliance["base"].append(ptbAlliance->m_nLanguage);
    rjsonAlliance["base"].append(CCommonBase::GetAlGiftPoint(ptbAlliance->m_nGift_point));
    rjsonAlliance["base"].append(ptbAlliance->m_nFund);
    rjsonAlliance["base"].append(ptbAlliance->m_nAvatar);
    rjsonAlliance["base"].append(ptbAlliance->m_sAl_nick_name);

    rjsonAlliance["can_assist_num"] = 0;
    rjsonAlliance["al_join_req_num"] = pstUser->m_udwAllianceReqCurFindNum;
    rjsonAlliance["can_help_action_num"] = pstUser->m_udwCanHelpTaskNum;
    rjsonAlliance["can_help_action_list"] = Json::Value(Json::arrayValue);
    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwAlCanHelpActionNum; ++udwIdx)
    {
        rjsonAlliance["can_help_action_list"].append(pstUser->m_patbAlCanHelpAction[udwIdx]->m_nId);
    }

    rjsonAlliance["wall_msg_num"] = pstUser->m_udwNewAlWallMsgNum;

    rjsonAlliance["throne_pos"] = ptbAlliance->m_nThrone_pos;
    rjsonAlliance["throne_status"] = ptbAlliance->m_nThrone_status;

    rjsonAlliance["al_star"] = ptbAlliance->m_nAl_star;

    CCommJson::GenAlGiftJson(pstUser, rjsonAlliance);
}


