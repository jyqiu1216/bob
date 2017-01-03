#include "maillist_json.h"
#include "game_command.h"
#include "common_json.h"
#include "common_func.h"
#include "document.h"

TVOID CMailListJson::GenDataJson(SSession* pstSession, Json::Value& rJson)
{
    if(pstSession->m_udwCommand == EN_CLIENT_REQ_COMMAND__MAIL_GET)
    {
        //"svr_mail_total_list":
        //{
        //    "cur_page": int,        //当前所处页数
        //    "total_num" : int,       //总数量
        //    "unread_num" : int,      //未读数量
        //    "entries" : [            //聚合后的邮件列表
        //    {
        //        "unread_num" : int, //聚合展示后该分类的未读数量
        //        "head_mail" : {     //展示的第一封邮件
        //            "base":[
        //                long,             //mail id
        //                int,              //unix time
        //                int,              //mail send type
        //                long,             //sender uid
        //                string,           //sender name
        //                long,             //receiver uid
        //                long,             //receiver aid
        //                int,              //doc id
        //                int,              //mail display type, 用以区分各种类型的邮件
        //                [string, string], //doc esc label
        //                string,           //sender al nick name
        //                int               //sender player avatar
        //            ],
        //            "text": {
        //                "title": string,
        //                "content" : string,
        //                "jmp_type" : int,
        //                "url" : string,
        //            },
        //            "reward" : [
        //                [int, int, int]   //[type, id, num]
        //            ],
        //            "read" : int,       //0表示未读,1表示已读
        //            "star" : int,       //0表示未收藏,1表示已经收藏
        //            "collect" : int,    //0表示未领奖,1表示已经领奖
        //            "al_info" : {     //这个key不一定有,要做保护
        //                "sid": int,
        //                "oid" : long,
        //                "oname" : string,
        //                "aid" : long,
        //                "alname" : string,
        //                "force" : long,      //force
        //                "force_kill" : long, //force_kill
        //                "member" : int,      //数量
        //                "desc" : string,     //联盟描述
        //                "lang" : int,          //语言
        //                "policy" : int,        //加入政策
        //                "gift_point" : int     //联盟礼物点数
        //            }
        //        }
        //    }]
        //},
        SUserInfo* pstUser = &pstSession->m_stUserInfo;
        Json::Value& rJsonMailList = rJson["svr_mail_total_list"];
        rJsonMailList = Json::Value(Json::objectValue);
        rJsonMailList["cur_page"] = pstSession->m_stReqParam.m_udwPage;
        rJsonMailList["total_num"] = pstUser->m_stMailUserInfo.m_udwMailTotalNum;
        rJsonMailList["unread_num"] = pstUser->m_stMailUserInfo.m_udwMailUnreadNum;
        rJsonMailList["entries"] = Json::Value(Json::arrayValue);
        TUINT32 udwCount = 0;
        for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_stMailUserInfo.m_udwMailEntryNum; ++udwIdx)
        {
            TbMail* ptbMail = NULL;
            for(TUINT32 udwIdy = 0; udwIdy < pstUser->m_udwMailNum; ++udwIdy)
            {
                if(pstUser->m_stMailUserInfo.m_aMailToReturn[udwIdx].ddwMid == pstUser->m_atbMailList[udwIdy].m_nId)
                {
                    ptbMail = &pstUser->m_atbMailList[udwIdy];
                    break;
                }
            }
            if(!ptbMail)
            {
                continue;
            }
            rJsonMailList["entries"][udwCount] = Json::Value(Json::objectValue);
            CMailListJson::GenMailInfo(&pstUser->m_stMailUserInfo.m_aMailToReturn[udwIdx], ptbMail, 1, rJsonMailList["entries"][udwCount]["head_mail"]);
            rJsonMailList["entries"][udwCount]["unread_num"] = pstUser->m_stMailUserInfo.m_aMailToReturn[udwIdx].dwUnreadCount;
            rJsonMailList["entries"][udwCount]["total_num"] = pstUser->m_stMailUserInfo.m_aMailToReturn[udwIdx].dwNum;
            udwCount++;
        }

        Json::Value& rJsonStat = rJson["svr_stat"];
        CCommJson::GenStatJson(pstSession->m_stRecommendTime.ddwTime,
            pstSession->m_udwRecommendNum,
            pstSession->m_bNeedMailMusic,
            &pstSession->m_stUserInfo.m_tbUserStat,
            rJsonStat);
    }
    else if(pstSession->m_udwCommand == EN_CLIENT_REQ_COMMAND__MAIL_DETAIL_GET)
    {
        //mail_detail_get
        //"svr_mail_detail_list":
        //{
        //    "title": string,    //tab标题
        //    "cur_page" : int,    //当前所处页数
        //    "total_num" : int,   //总数目
        //    "unread_num" : int,  //未读数量
        //    "entries" : [        //mail的列表
        //    {
        //        "base":[
        //            long,             //mail id
        //            int,              //unix time
        //            int,              //mail send type
        //            long,             //sender uid
        //            string,           //sender name
        //            long,             //receiver uid
        //            long,             //receiver aid
        //            int,              //doc id
        //            int,              //mail display type, 用以区分各种类型的邮件
        //            [string, string], //doc esc label
        //            string,           //sender al nick name  
        //            int               //sender player avatar
        //        ],
        //        "text": {
        //                "title": string,
        //                "content" : string,
        //                "jmp_type" : int,
        //                "url" : string,
        //        },
        //        "reward" : [
        //            [int, int, int]   //[type, id, num]
        //        ],
        //        "read" : int,       //0表示未读,1表示已读
        //        "star" : int,       //0表示未收藏,1表示已经收藏
        //        "collect" : int,    //0表示未领奖,1表示已经领奖
        //        "al_info" : {     //这个key不一定有,要做保护
        //            "sid": int,
        //            "oid" : long,
        //            "oname" : string,
        //            "aid" : long,
        //            "alname" : string,
        //            "force" : long,      //force
        //            "force_kill" : long, //force_kill
        //            "member" : int,      //数量
        //            "desc" : string,     //联盟描述
        //            "lang" : int,          //语言
        //            "policy" : int,        //加入政策
        //            "gift_point" : int     //联盟礼物点数
        //        }
        //    }]
        //}
        SUserInfo* pstUser = &pstSession->m_stUserInfo;
        Json::Value& rJsonMailList = rJson["svr_mail_detail_list"];
        rJsonMailList = Json::Value(Json::objectValue);
        string strTabName = "Mail";
        rJsonMailList["cur_page"] = pstSession->m_stReqParam.m_udwPage;
        rJsonMailList["total_num"] = pstUser->m_stMailUserInfo.m_udwMailTotalNum;
        rJsonMailList["unread_num"] = pstUser->m_stMailUserInfo.m_udwMailUnreadNum;
        rJsonMailList["entries"] = Json::Value(Json::arrayValue);
        TUINT32 udwCount = 0;
        for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_stMailUserInfo.m_udwMailEntryNum; ++udwIdx)
        {
            TbMail* ptbMail = NULL;
            for(TUINT32 udwIdy = 0; udwIdy < pstUser->m_udwMailNum; ++udwIdy)
            {
                if (pstUser->m_stMailUserInfo.m_aMailToReturn[udwIdx].ddwMid == pstUser->m_atbMailList[udwIdy].m_nId)
                {
                    ptbMail = &pstUser->m_atbMailList[udwIdy];
                    break;
                }
            }
            if(!ptbMail)
            {
                continue;
            }
            if(udwIdx == 0)
            {
                if(pstUser->m_tbPlayer.m_nUid == ptbMail->m_nSuid)
                {
                    strTabName = pstUser->m_stMailUserInfo.m_aMailToReturn[udwIdx].strReceiver;
                }
                else
                {
                    strTabName = ptbMail->m_sSender;
                }
                if (pstUser->m_stMailUserInfo.m_aMailToReturn[udwIdx].dwDisplayType == EN_MAIL_DISPLAY_TYPE_ALLIANCE)
                {
                    strTabName = "(" + pstUser->m_tbAlliance.m_sAl_nick_name + ")"
                               + pstUser->m_tbAlliance.m_sName;
                }
            }
            rJsonMailList["entries"][udwCount] = Json::Value(Json::objectValue);
            CMailListJson::GenMailInfo(&pstUser->m_stMailUserInfo.m_aMailToReturn[udwIdx], ptbMail, 0, rJsonMailList["entries"][udwCount]);
            udwCount++;
        }
        rJsonMailList["title"] = strTabName;

        Json::Value& rJsonStat = rJson["svr_stat"];
        CCommJson::GenStatJson(pstSession->m_stRecommendTime.ddwTime,
            pstSession->m_udwRecommendNum,
            pstSession->m_bNeedMailMusic,
            &pstSession->m_stUserInfo.m_tbUserStat,
            rJsonStat);
    }
    else if(pstSession->m_udwCommand == EN_CLIENT_OPERATE_COMMAND__MAIL_GET)
    {
        SUserInfo* pstUser = &pstSession->m_stUserInfo;
        Json::Value& rJsonMailList = rJson["svr_op_mail_list"];
        rJsonMailList = Json::Value(Json::objectValue);
        SMailEntry stMailEntry;
        CMailListJson::GenMailInfo(&stMailEntry, &pstUser->m_atbMailList[0U], 0, rJsonMailList);
    }
}

CMailListJson::CMailListJson()
{

}

CMailListJson::~CMailListJson()
{

}

TVOID CMailListJson::GenMailInfo(SMailEntry* pstMailEntry, TbMail* ptbMail, TUINT32 udwHeadFlag, Json::Value& rjson)
{
    //op_mail_get
    //"svr_op_mail_list":
    //{
    //    "base":[
    //        long,             //mail id
    //        int,              //unix time
    //        int,              //mail send type
    //        long,             //sender uid
    //        string,           //sender name
    //        long,             //receiver uid
    //        long,             //receiver aid
    //        int,              //doc id
    //        int,              //mail display type, 用以区分各种类型的邮件
    //        [string, string], //doc esc label
    //        string,           //sender al nick name
    //        int               //sender player avatar
    //    ],
    //    "text": {
    //        "title": string,
    //        "content" : string,
    //        "jmp_type" : int,
    //        "url" : string,
    //    },
    //    "reward" : [
    //        [int, int, int]   //[type, id, num]
    //    ],
    //    "read" : int,       //0表示未读,1表示已读
    //    "star" : int,       //0表示未收藏,1表示已经收藏
    //    "collect" : int,    //0表示未领奖,1表示已经领奖
    //    "available": int,   //0表示失效,1表示未失效
    //    "al_info" : {     //这个key不一定有,要做保护
    //        "sid": int,
    //        "oid" : long,
    //        "oname" : string,
    //        "aid" : long,
    //        "alname" : string,
    //        "force" : long,      //force
    //        "force_kill" : long, //force_kill
    //        "member" : int,      //数量
    //        "desc" : string,     //联盟描述
    //        "lang" : int,          //语言
    //        "policy" : int,        //加入政策
    //        "gift_point" : int     //联盟礼物点数
    //    }
    //}

    ostringstream ossTranslateContent;
    ossTranslateContent.str("");


    rjson = Json::Value(Json::objectValue);
    rjson["base"] = Json::Value(Json::arrayValue);
    rjson["base"].append(ptbMail->m_nId);
    rjson["base"].append(ptbMail->m_nTime);
    rjson["base"].append(ptbMail->m_nSend_type);
    rjson["base"].append(ptbMail->m_nSuid);
    rjson["base"].append(ptbMail->m_sSender);
    rjson["base"].append(ptbMail->m_nRuid);
    if(ptbMail->m_nRuid < 0)
    {
        rjson["base"].append(-1 * ptbMail->m_nRuid);
    }
    else
    {
        rjson["base"].append(0);
    }
    rjson["base"].append(ptbMail->m_nMaildocid);
    rjson["base"].append(ptbMail->m_nDisplay_type);

    Json::Value jsonEscs;
    if(ptbMail->m_sEscs.empty())
    {
        jsonEscs = Json::Value(Json::arrayValue);
    }
    else
    {
        Json::Reader jsonReader;
        jsonReader.parse(ptbMail->m_sEscs, jsonEscs);
    }
    rjson["base"].append(jsonEscs);
    rjson["base"].append(ptbMail->m_sSender_al_nick);
    rjson["base"].append(ptbMail->m_nSender_player_avatar);
    rjson["base"].append(pstMailEntry->dwReceiverAvatar);
    rjson["base"].append(pstMailEntry->strReceiver);
    rjson["base"].append(pstMailEntry->strReceiverAlnick);
    rjson["base"].append(ptbMail->m_nEvent_id);
    rjson["base"].append(ptbMail->m_nEvent_type);
    rjson["base"].append(ptbMail->m_nEvent_score);

    rjson["text"] = Json::Value(Json::objectValue);
    rjson["text"]["title"] = ptbMail->m_sTitle;
    rjson["text"]["content"] = ptbMail->m_sContent;
    rjson["text"]["jmp_type"] = ptbMail->m_nJmp_type;

    TBOOL bIsExistDocument = CDocument::GetInstance()->IsSupportLang(CDocument::GetLang(0));

    if((0 == ptbMail->m_nSend_type || 1 == ptbMail->m_nSend_type || 4 == ptbMail->m_nSend_type)
       && TRUE == bIsExistDocument
       && -1 != ptbMail->m_nRaw_lang)
    {
    
        Json::Value rJsonMailTranslate = Json::Value(Json::objectValue);
    
        rJsonMailTranslate["raw_lang"] = ptbMail->m_nRaw_lang;            
 
        const Json::Value &stDocumentJson = CDocument::GetInstance()->GetDocumentJsonByLang(CDocument::GetLang(0));

        for(TUINT32 udwIdx = 0; udwIdx < stDocumentJson["doc_language"].size(); ++udwIdx)
        {
            if(1 == stDocumentJson["doc_language"][CCommonFunc::NumToString(udwIdx)]["status"].asInt())
            {

                Json::Reader jsonReader;
                Json::Value jResultBody;
                if(jsonReader.parse(ptbMail->m_sTranslate_content.c_str(), jResultBody))
                {          
                    /*
                    ossTranslateContent.str("");
                    ossTranslateContent << jResultBody[CCommonFunc::NumToString(dwIdx)]["0"];
                    rJsonMailTranslate[CCommonFunc::NumToString(dwIdx)]["title"] = ossTranslateContent.str(); 
                    */
                    rJsonMailTranslate[CCommonFunc::NumToString(udwIdx)]["title"] = ""; 

                    ossTranslateContent.str("");
                    ossTranslateContent << jResultBody[CCommonFunc::NumToString(udwIdx)]["0"].asString();
                    rJsonMailTranslate[CCommonFunc::NumToString(udwIdx)]["content"] = ossTranslateContent.str(); 
                }
            }
        }
        
        rjson["text"]["translate"] = rJsonMailTranslate;
    }
    
    rjson["text"]["url"] = ptbMail->m_sUrl;

    rjson["reward"] = Json::Value(Json::arrayValue);
    TUINT32 udwJsonIdx = 0;
    for(TUINT32 udwIdx = 0; udwIdx < ptbMail->m_bReward[0].ddwTotalNum; ++udwIdx)
    {
        if(udwIdx >= MAX_REWARD_ITEM_NUM)
        {
            continue;
        }
        rjson["reward"][udwJsonIdx] = Json::Value(Json::arrayValue);
        rjson["reward"][udwJsonIdx].append(ptbMail->m_bReward[0][udwIdx].ddwType);
        rjson["reward"][udwJsonIdx].append(ptbMail->m_bReward[0][udwIdx].ddwId);
        rjson["reward"][udwJsonIdx].append(ptbMail->m_bReward[0][udwIdx].ddwNum);
        udwJsonIdx++;
    }
    for (TUINT32 udwIdx = 0; udwIdx < ptbMail->m_bEx_reward.m_udwNum; ++udwIdx)
    {
        rjson["reward"][udwJsonIdx] = Json::Value(Json::arrayValue);
        rjson["reward"][udwJsonIdx].append(ptbMail->m_bEx_reward[udwIdx].ddwType);
        rjson["reward"][udwJsonIdx].append(ptbMail->m_bEx_reward[udwIdx].ddwId);
        rjson["reward"][udwJsonIdx].append(ptbMail->m_bEx_reward[udwIdx].ddwNum);
        udwJsonIdx++;
    }
    if(pstMailEntry->dwHasReward)
    {
        if(rjson["reward"].empty())
        {
            rjson["reward"][0u] = Json::Value(Json::arrayValue);
            rjson["reward"][0u].append(0);
            rjson["reward"][0u].append(1);
            rjson["reward"][0u].append(1);
        }
    }
    else if (udwHeadFlag)
    {
        rjson["reward"] = Json::Value(Json::arrayValue);
    }

    rjson["read"] = (pstMailEntry->dwStatus & EN_MAIL_STATUS_READ) > 0 ? 1 : 0;
    rjson["star"] = (pstMailEntry->dwStatus & EN_MAIL_STATUS_MARK) > 0 ? 1 : 0;
    rjson["collect"] = (pstMailEntry->dwStatus & EN_MAIL_STATUS_COLLECT) > 0 ? 1 : 0;
    if(pstMailEntry->dwHasReward)
    {
        rjson["collect"] = 0;
    }
    rjson["available"] = pstMailEntry->bAvailable;

    if(!ptbMail->m_sSender_al_info.empty())
    {
        Json::Reader jsonReader;
        jsonReader.parse(ptbMail->m_sSender_al_info, rjson["al_info"]);
    }
}
