#include "user_json.h"
#include "common_json.h"
#include "player_base.h"
#include "action_base.h"
#include "city_base.h"
#include "common_base.h"
#include "common_func.h"
#include "backpack_logic.h"
#include "wild_info.h"

CUserJson::CUserJson()
{
}

CUserJson::~CUserJson()
{
}


TVOID CUserJson::GenDataJson(SSession* pstSession, SUserInfo *pstUser, Json::Value& rJson)
{
    GenLoginJson(pstUser, rJson);
    GenPlayerJson(pstSession, pstUser, rJson);
    GenResearchJson(pstUser, rJson);
    GenKnightInfo(pstUser, rJson);

    GenCityJson(pstUser, rJson);
    GenBagJson(pstUser, rJson);
    GenTopQuestJson(pstUser, rJson);
    GenStatJson(pstUser, rJson);

    GenActionNewJson(pstUser, rJson);

    GenAllianceJson(pstUser, rJson);
    GenDiplomacyJson(pstUser, rJson);
    GenBuffJson(pstUser, rJson);

    GenTimeQuestJson(pstUser, rJson);

    GenBackPackJson(pstUser, rJson);
}

TVOID CUserJson::GenLoginJson(SUserInfo *pstUser, Json::Value& rjsonResult)
{
    TbLogin* ptbLogin = &pstUser->m_tbLogin;

    Json::Value& jsonSvrLogin = rjsonResult["svr_login"];
    jsonSvrLogin = Json::Value(Json::objectValue);
    jsonSvrLogin["uid"] = ptbLogin->m_nUid;
    jsonSvrLogin["did"] = ptbLogin->m_sDe;
    jsonSvrLogin["idfa"] = ptbLogin->m_sIdfa;
    jsonSvrLogin["gem"] = ptbLogin->m_nGem;
    jsonSvrLogin["gem_seq"] = ptbLogin->m_nGem_seq;
    jsonSvrLogin["gem_buy"] = ptbLogin->m_nGem_buy;
    jsonSvrLogin["gem_cost"] = ptbLogin->m_nGem_cost;
    jsonSvrLogin["ctime"] = ptbLogin->m_nCtime;
    jsonSvrLogin["utime"] = ptbLogin->m_nUtime;
    jsonSvrLogin["last_svr"] = ptbLogin->m_nSid;
    jsonSvrLogin["is_npc"] = ptbLogin->m_nNpc;
    jsonSvrLogin["guide_flag"] = Json::Value(Json::arrayValue);
    for(TUINT32 udwIdx = 0, udwJsonIndex = 0; udwIdx < MAX_GUIDE_FLAG_NUM; ++udwIdx)
    {
        if(BITTEST(ptbLogin->m_bGuide_flag[0].m_bitFlag, udwIdx) != 0)
        {
            jsonSvrLogin["guide_flag"][udwJsonIndex] = udwIdx;
            udwJsonIndex++;
        }
    }

    jsonSvrLogin["first_al_time"] = ptbLogin->m_nAl_time;

    jsonSvrLogin["apns_switch"] = ptbLogin->m_jApns_switch;

    //rating
    jsonSvrLogin["rating_switch"] = pstUser->m_bRatingSwitch;
    jsonSvrLogin["rating_gem"] = pstUser->m_udwRatingGem;

    jsonSvrLogin["dragon_unlock_flag"] = -1; 
}

TVOID CUserJson::GenPlayerJson(SSession* pstSession, SUserInfo *pstUser, Json::Value& rjsonResult)
{
    /*
    "svr_player": 
    {
        ...
    }
    */

    Json::Value& jsonSvrPlayer = rjsonResult["svr_player"];
    TbPlayer* ptbPlayer = &pstUser->m_tbPlayer;

    CCommJson::GenPlayerProfileJson(ptbPlayer, pstUser->m_atbEquip,
        pstUser->m_udwEquipNum, jsonSvrPlayer, &pstUser->m_stPlayerBuffList);

    jsonSvrPlayer["guide_id"] = 0;
    jsonSvrPlayer["finish_guide_list"] = Json::Value(Json::arrayValue);
    for (TUINT32 udwIdx = 0; udwIdx < ptbPlayer->m_bFinish_guide_list[0].udwNum; ++udwIdx)
    {
        jsonSvrPlayer["finish_guide_list"][udwIdx] = ptbPlayer->m_bFinish_guide_list[0].addwFinishGuide[udwIdx];
    }

    TbMarch_action* ptbPrison = NULL;
    TBOOL bBeCaptured = FALSE;
    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
    {
        if (pstUser->m_aucMarchFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }
        if (!CActionBase::IsPlayerOwnedAction(pstUser->m_tbPlayer.m_nUid, pstUser->m_atbMarch[udwIdx].m_nId))
        {
            continue;
        }
        if (pstUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__PRISON_TIMER)
        {
            ptbPrison = &pstUser->m_atbMarch[udwIdx];
            bBeCaptured = TRUE;
            continue;
        }
    }
    if (bBeCaptured == TRUE && jsonSvrPlayer.isMember("dragon"))
    {
        jsonSvrPlayer["dragon"]["capture_info"] = Json::Value(Json::objectValue);
        jsonSvrPlayer["dragon"]["capture_info"]["uid"] = ptbPrison->m_nTuid;
        jsonSvrPlayer["dragon"]["capture_info"]["al_nick"] = ptbPrison->m_bPrison_param[0].szTargetAlNick;
        jsonSvrPlayer["dragon"]["capture_info"]["uname"] = ptbPrison->m_bPrison_param[0].szTargetUserName;
        jsonSvrPlayer["dragon"]["capture_info"]["join_time"] = ptbPrison->m_bPrison_param[0].ddwJoinTimeStamp;
        jsonSvrPlayer["dragon"]["capture_info"]["can_kill_time"] = ptbPrison->m_nEtime - ptbPrison->m_bPrison_param[0].ddwReleaseWait + ptbPrison->m_bPrison_param[0].ddwExcuteWait;
        jsonSvrPlayer["dragon"]["capture_info"]["auto_release_time"] = ptbPrison->m_nEtime;
        jsonSvrPlayer["dragon"]["capture_info"]["pos"] = ptbPrison->m_nTpos;
        jsonSvrPlayer["dragon"]["capture_info"]["can_kill"] = ptbPrison->m_bPrison_param[0].stDragon.m_ddwCaptured;
        jsonSvrPlayer["dragon"]["capture_info"]["sid"] = ptbPrison->m_nSid;
    }
}

TVOID CUserJson::GenResearchJson(SUserInfo *pstUser, Json::Value& rJson)
{
    /*"svr_research":
    {
    "0"(int): int // "research_id": lv
    }
    */
    SCommonResearch* pstResearch = &pstUser->m_stCityInfo.m_stTblData.m_bResearch[0];
    Json::Value& rJsonResearch = rJson["svr_research"];
    CCommJson::GenResearchJson(pstResearch, rJsonResearch);
}

TVOID CUserJson::GenKnightInfo(SUserInfo *pstUser, Json::Value& rJson)
{
    /*
    "svr_knight_list":
    [
    [int, int, int, int, long] //[knight_id, 状态, 任职位置, exp, 相关actionid]
    ]
    */
    SCityInfo *pstCity = &pstUser->m_stCityInfo;

    Json::Value& rJsonKnight = rJson["svr_knight_list"];
    rJsonKnight = Json::Value(Json::arrayValue);

    for (TUINT32 udwIdx = 0; udwIdx < pstCity->m_stTblData.m_bKnight.m_udwNum; udwIdx++)
    {
        rJsonKnight[udwIdx] = Json::Value(Json::arrayValue);
        rJsonKnight[udwIdx].append(udwIdx);
        rJsonKnight[udwIdx].append(pstCity->m_stTblData.m_bKnight[udwIdx].ddwStatus);
        rJsonKnight[udwIdx].append(pstCity->m_stTblData.m_bKnight[udwIdx].ddwPos);
        rJsonKnight[udwIdx].append(pstCity->m_stTblData.m_bKnight[udwIdx].ddwExp);
        rJsonKnight[udwIdx].append(pstCity->m_stTblData.m_bKnight[udwIdx].ddwTid);
    }
}

TVOID CUserJson::GenCityJson(SUserInfo *pstUser, Json::Value& rjsonResult)
{
    /*
    "svr_city_list[":
    {
        "basic": [
            0,              //cidx wot2固定为0?
            102109,         //city position
            "NewCity-3I",   //city name
            1383902544,     //updt time
            1,              //解锁地块数
        ]
        "resource": [
            long,         //gold
            long,         //food
            long,         //wood
            long,           //stone
            long          //ore
        ],
        "building": {
            "0"(int): //position
            [
                int,      //building type
                int       //building level
            ]
        },
        "castle_lv": int
        "total_troop" : long  // 总的troop数量，包括march出去的兵。
        "troop": //下标为troop id，值表示具体数量
        [
            long,
            long,
            ...
        ],
        "live_troop": //同 troop
        [
        ...
        ],
        "troop_wounded": //同 troop
        [
            ...
        ],
        "total_fort" : long
        "fort": //下标为fort id，值表示具体数量
        [
            long,
            long,
            ...
        ],
        "fort_dead"://同 fort
        [
            ...
        ],
        "altar":{ //祭坛
            "kill_buff":{
                "dragon_name" : string, //处决的龙名字
                "dragon_lv" : int,   //处决的龙等级
                "begintime" : int, //unixtime,开始时间
                "endtime" : int, //unixtime,到期时间
                "detail":[
                    [int, int] // buff id, buff num
                ]
            },
            "kill_history":{
                "kill":[
                    {
                        "killer":[int, string, string], //uid , 联盟简称, 玩家名称
                        "owner":[int, stirng, string], //uid , 联盟简称, 玩家名称
                        "bekilled_dragon":[int, stirng, int], //dragon avatar, dragon name, dragon lv
                        "kill_time": int //unixtime
                    }
                ],
                "killed":[
                    {
                        "killer":[int, string, string], //uid , 联盟简称, 玩家名称
                        "owner":[int, stirng, string], //uid , 联盟简称, 玩家名称
                        "bekilled_dragon":[int, stirng, int], //dragon avatar, dragon name, dragon lv
                        "kill_time": int //unixtime
                    }
                ]
            }
        },
        "prison"://监狱
        [
            {
                "can_kill": int, //0表示能够处死, 1表示自身等级不足, 2表示对方等级不足
                "uid": long,
                "al_nick": string,
                "uname": string,
                "dragon_avatar": int,
                "dragon_name": string,
                "dragon_lv": int,
                "dragon_status": int, //EHeroStatus
                "join_time": int,
                "can_kill_time": int,
                "auto_release_time": int,
                "action_id": long
            }
        ],
        "production": 
        {
            "0"(int): //key为资源类型ID
            [
                long, //base production
                long, //cur production
                long, //capacity
                long, //upkeep
                long, //bonus amount
                long, //total bouns
                long, //knight bonus
                long, //research bonus
                long, //item bonus
                long, //dragon bonus
                long, //vip bonus
                long, //lord bonus
                long  //other bouns
            ]
        }
        //"building_pos" 这个实在不知道有什么用 TODO
    }]
    */

    SCityInfo *pstCity = &pstUser->m_stCityInfo;

    Json::Value& jsonSvrCityList = rjsonResult["svr_city_list"];
    jsonSvrCityList = Json::Value(Json::arrayValue);

    Json::Value jsonSvrCity = Json::Value(Json::objectValue);
    jsonSvrCity["base"][0] = 0;
    jsonSvrCity["base"][1] = pstCity->m_stTblData.m_nPos;
    jsonSvrCity["base"][2] = pstCity->m_stTblData.m_sName;
    jsonSvrCity["base"][3] = pstCity->m_stTblData.m_nUtime;
    jsonSvrCity["base"][4] = 64/*TODO 获取最大地块数*/ - CCityBase::GetBuildingNumByFuncType(pstCity, EN_BUILDING_TYPE__GROVE);

    CCommJson::GenResourceJson(&pstCity->m_stTblData.m_bResource[0], jsonSvrCity["resource"]);

    CCommJson::GenBuildingJson(&pstCity->m_stTblData.m_bBuilding[0], pstCity->m_stTblData.m_bBuilding.m_udwNum, jsonSvrCity["building"]);

    jsonSvrCity["castle_lv"] = CCityBase::GetBuildingNumByFuncType(pstCity, EN_BUILDING_TYPE__CASTLE);

    jsonSvrCity["total_troop"] = CCommonBase::CalcTroopsNum(pstUser);

    CCommJson::GenTroopJson(&pstCity->m_stTblData.m_bTroop[0], jsonSvrCity["troop"]);

    SCommonTroop stLiveTroop = pstCity->m_stTblData.m_bTroop[0];
    CCommonBase::CalcMarchActionTroop(pstUser, stLiveTroop.m_addwNum, FALSE);
    CCommJson::GenTroopJson(&stLiveTroop, jsonSvrCity["live_troop"]);

    CCommJson::GenTroopJson(&pstCity->m_stTblData.m_bHos_wait[0], jsonSvrCity["troop_wounded"]);

    jsonSvrCity["total_fort"] = CCommonBase::CalcFortsNum(pstUser);

    CCommJson::GenFortJson(&pstCity->m_stTblData.m_bFort[0], jsonSvrCity["fort"]);

    CCommJson::GenFortJson(&pstCity->m_stTblData.m_bDead_fort[0], jsonSvrCity["fort_dead"]);

    GenAltarJson(pstCity, jsonSvrCity["altar"]);

    GenPrisonJson(pstUser, jsonSvrCity["prison"]);

    GenProductionJson(pstCity, jsonSvrCity["production"]);

    jsonSvrCity["building_pos"] = Json::Value(Json::objectValue);

    jsonSvrCityList.append(jsonSvrCity);
}

TVOID CUserJson::GenAltarJson(SCityInfo* pstCity, Json::Value& rjson)
{
    /*
    "altar":{ //祭坛
        "kill_buff":{
            "dragon_name" : string, //处决的龙名字
            "dragon_lv" : int,   //处决的龙等级
            "begintime" : int, //unixtime,开始时间
            "endtime" : int, //unixtime,到期时间
            "detail":[
                [int, int] // buff id, buff num
            ]
        },
        "kill_history":{
            "kill":[
                {
                    "killer":[int, string, string], //uid , 联盟简称, 玩家名称
                    "owner":[int, stirng, string], //uid , 联盟简称, 玩家名称
                    "bekilled_dragon":[int, stirng, int], //dragon avatar, dragon name, dragon lv
                    "kill_time": int //unixtime
                }
            ],
            "killed":[
                {
                    "killer":[int, string, string], //uid , 联盟简称, 玩家名称
                    "owner":[int, stirng, string], //uid , 联盟简称, 玩家名称
                    "bekilled_dragon":[int, stirng, int], //dragon avatar, dragon name, dragon lv
                    "kill_time": int //unixtime
                }
            ]
        }
    }
    */
    rjson = Json::Value(Json::objectValue);

    rjson["kill_buff"] = Json::Value(Json::objectValue);
    rjson["kill_buff"]["dragon_name"] = pstCity->m_stTblData.m_sAltar_dragon_name;
    rjson["kill_buff"]["dragon_lv"] = pstCity->m_stTblData.m_nAltar_dragon_lv;
    rjson["kill_buff"]["begintime"] = pstCity->m_stTblData.m_nAltar_buff_btime;
    rjson["kill_buff"]["endtime"] = pstCity->m_stTblData.m_nAltar_buff_etime;
    rjson["kill_buff"]["detail"] = Json::Value(Json::arrayValue);
    if(pstCity->m_stTblData.m_nAltar_buff_etime > CTimeUtils::GetUnixTime())
    {
        for(TUINT32 udwIdx = 0; udwIdx < pstCity->m_stTblData.m_bAltar_buff.m_udwNum; ++udwIdx)
        {
            rjson["kill_buff"]["detail"][udwIdx] = Json::Value(Json::arrayValue);
            rjson["kill_buff"]["detail"][udwIdx].append(pstCity->m_stTblData.m_bAltar_buff[udwIdx].ddwBuffId);
            rjson["kill_buff"]["detail"][udwIdx].append(pstCity->m_stTblData.m_bAltar_buff[udwIdx].ddwBuffNum);
        }
    }

    rjson["kill_history"] = Json::Value(Json::objectValue);
    rjson["kill_history"]["kill"] = Json::Value(Json::arrayValue);
    rjson["kill_history"]["killed"] = Json::Value(Json::arrayValue);
}

TVOID CUserJson::GenPrisonJson(SUserInfo *pstUser, Json::Value& rJson)
{
    /*
    "prison"://监狱
    [
    {
    "can_kill": int, //0表示能够处死, 1表示自身等级不足, 2表示对方等级不足
    "uid": long,
    "al_nick": string,
    "uname": string,
    "dragon_avatar": int,
    "dragon_name": string,
    "dragon_lv": int,
    "dragon_status": int, //EHeroStatus
    "join_time": int,
    "can_kill_time": int,
    "auto_release_time": int,
    "action_id": long
    }
    ]
    */

    rJson = Json::Value(Json::arrayValue);
    Json::Value jsonOne = Json::Value(Json::objectValue);
    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwPassiveMarchNum; ++udwIdx)
    {
        if (pstUser->m_aucPassiveMarchFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }

        if (pstUser->m_atbPassiveMarch[udwIdx].m_nSclass != EN_ACTION_SEC_CLASS__PRISON_TIMER)
        {
            continue;
        }

        if (pstUser->m_atbPassiveMarch[udwIdx].m_mFlag.find(TbMARCH_ACTION_FIELD_TAL) != pstUser->m_atbPassiveMarch[udwIdx].m_mFlag.end()
            && pstUser->m_atbPassiveMarch[udwIdx].m_mFlag[TbMARCH_ACTION_FIELD_TAL] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }

        jsonOne.clear();
        jsonOne["can_kill"] = pstUser->m_atbPassiveMarch[udwIdx].m_bPrison_param[0].stDragon.m_ddwCaptured;
        jsonOne["uid"] = pstUser->m_atbPassiveMarch[udwIdx].m_nSuid;
        jsonOne["al_nick"] = pstUser->m_atbPassiveMarch[udwIdx].m_bPrison_param[0].szSourceAlNick;
        jsonOne["uname"] = pstUser->m_atbPassiveMarch[udwIdx].m_bPrison_param[0].szSourceUserName;
        jsonOne["dragon_avatar"] = pstUser->m_atbPassiveMarch[udwIdx].m_bPrison_param[0].stDragon.m_ddwIconId;
        jsonOne["dragon_name"] = pstUser->m_atbPassiveMarch[udwIdx].m_bPrison_param[0].stDragon.m_szName;
        jsonOne["dragon_lv"] = pstUser->m_atbPassiveMarch[udwIdx].m_bPrison_param[0].stDragon.m_ddwLevel;

        TUINT32 udwDragonStatus = EN_DRAGON_STATUS_WAIT_KILL;
        if (pstUser->m_atbPassiveMarch[udwIdx].m_bPrison_param[0].ddwEscortActionId > 0)
        {
            udwDragonStatus = EN_DRAGON_STATUS_BEING_ESCORT;
        }
        else if (CTimeUtils::GetUnixTime() > (pstUser->m_atbPassiveMarch[udwIdx].m_nEtime - pstUser->m_atbPassiveMarch[udwIdx].m_bPrison_param[0].ddwReleaseWait + pstUser->m_atbPassiveMarch[udwIdx].m_bPrison_param[0].ddwExcuteWait)
            || pstUser->m_atbPassiveMarch[udwIdx].m_bPrison_param[0].stDragon.m_ddwCaptured != 0)
        {
            udwDragonStatus = EN_DRAGON_STATUS_WAIT_RELEASE;
        }
        jsonOne["dragon_status"] = udwDragonStatus;

        jsonOne["join_time"] = pstUser->m_atbPassiveMarch[udwIdx].m_bPrison_param[0].ddwJoinTimeStamp;
        jsonOne["can_kill_time"] = pstUser->m_atbPassiveMarch[udwIdx].m_nEtime - pstUser->m_atbPassiveMarch[udwIdx].m_bPrison_param[0].ddwReleaseWait + pstUser->m_atbPassiveMarch[udwIdx].m_bPrison_param[0].ddwExcuteWait;
        jsonOne["auto_release_time"] = pstUser->m_atbPassiveMarch[udwIdx].m_nEtime;
        jsonOne["action_id"] = pstUser->m_atbPassiveMarch[udwIdx].m_bPrison_param[0].ddwEscortActionId;

        rJson.append(jsonOne);
    }
}

TVOID CUserJson::GenProductionJson(SCityInfo* pstCity, Json::Value& rjson)
{
    rjson = Json::Value(Json::objectValue);
    for(TUINT32 udwIdx = 0; udwIdx < EN_RESOURCE_TYPE__END; ++udwIdx)
    {
        string strKey = CCommonFunc::NumToString(udwIdx);
        rjson[strKey] = Json::Value(Json::arrayValue);
        rjson[strKey].append(pstCity->m_astResProduction[udwIdx].m_ddwBaseProduction);
        rjson[strKey].append(pstCity->m_astResProduction[udwIdx].m_ddwCurProduction);
        rjson[strKey].append(pstCity->m_astResProduction[udwIdx].m_ddwCapacity);
        rjson[strKey].append(pstCity->m_astResProduction[udwIdx].m_uddwUpkeep);
        rjson[strKey].append(pstCity->m_astResProduction[udwIdx].m_ddwTotalBonusAmount);
        rjson[strKey].append(pstCity->m_astResProduction[udwIdx].m_ddwTotalBonus);
        rjson[strKey].append(pstCity->m_astResProduction[udwIdx].m_addwBonus[EN_BONUS_TYPE__KNIGHT]);
        rjson[strKey].append(pstCity->m_astResProduction[udwIdx].m_addwBonus[EN_BONUS_TYPE__RESEARCH]);
        rjson[strKey].append(pstCity->m_astResProduction[udwIdx].m_addwBonus[EN_BONUS_TYPE__ITEM]);
        rjson[strKey].append(pstCity->m_astResProduction[udwIdx].m_addwBonus[EN_BONUS_TYPE__DRAGON]);
        rjson[strKey].append(pstCity->m_astResProduction[udwIdx].m_addwBonus[EN_BONUS_TYPE__VIP]);
        rjson[strKey].append(pstCity->m_astResProduction[udwIdx].m_addwBonus[EN_BONUS_TYPE__LORD]);
        rjson[strKey].append(pstCity->m_astResProduction[udwIdx].m_addwBonus[EN_BONUS_TYPE__OTHER]);
    }
}

TVOID CUserJson::GenBagJson(SUserInfo *pstUser, Json::Value& rjsonResult)
{
    /*
    "svr_bag":
    {
    "item_list":
    {
    "6"(int): int, //item_id:item_num
    ...
    }
    }
    */

    Json::Value& jsonSvrBag = rjsonResult["svr_bag"];
    jsonSvrBag["item_list"] = Json::Value(Json::objectValue);
    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_tbBackpack.m_bItem.m_udwNum; ++udwIdx)
    {
        //ONLY output when item num > 0
        if (pstUser->m_tbBackpack.m_bItem[udwIdx].m_ddwItemNum > 0)
        {
            jsonSvrBag["item_list"][CCommonFunc::NumToString(pstUser->m_tbBackpack.m_bItem[udwIdx].m_ddwItemId)]
                = pstUser->m_tbBackpack.m_bItem[udwIdx].m_ddwItemNum;
        }
    }
}

TVOID CUserJson::GenTopQuestJson(SUserInfo *pstUser, Json::Value& rjsonResult)
{
    /*
    "svr_quest":
    {
    "top_quest_list": //已经完成的国王quest的Idlist,并且已经收集
    [
    int
    ],
    "top_quest_list_to_claim": //待收集的国王quest
    [
    int
    ],
    }
    */
    Json::Value& jsonSvrQuest = rjsonResult["svr_quest"];
    STopQuest& stQuest = pstUser->m_tbUserStat.m_bTop_quest[0];
    jsonSvrQuest["top_quest_list"] = Json::Value(Json::arrayValue);

    for (TUINT32 udwIdx = 0; udwIdx < EN_TOP_QUEST_NUM_LIMIT; ++udwIdx)
    {
        if (BITTEST(stQuest.m_bitQuest, udwIdx))
        {
            jsonSvrQuest["top_quest_list"].append(udwIdx);
        }
    }

    jsonSvrQuest["top_quest_list_to_claim"] = Json::Value(Json::arrayValue);
    for (TUINT32 udwIdx = 0; udwIdx < EN_TOP_QUEST_NUM_LIMIT; ++udwIdx)
    {
        if (BITTEST(stQuest.m_bitQuest, udwIdx))
        {
            continue;
        }
        // 是否是待收集任务
        if (!BITTEST(pstUser->m_tbUserStat.m_bTop_quest_finish[0].m_bitQuest, udwIdx))
        {
            continue;
        }

        jsonSvrQuest["top_quest_list_to_claim"].append(udwIdx);
    }
}

TVOID CUserJson::GenStatJson(SUserInfo *pstUser, Json::Value& rjsonResult)
{
    /*
    "svr_stat":
    {
    "has_mail": int,                  //是否有新邮件，0表示没有，1表示有
    "has_report": int,                //是否有新report，0表示没有，1表示有
    "has_help_bubble": int,			  //是否有帮助神像气泡，0表示没有，1表示有
    "unread_mail": int,               //未读邮件数量
    "unread_report": int,             //未读report数量
    "mail_newest_id":long,	          //该用户最新的mailid
    "report_newest_id":long,	      //该用户最新的reportid
    "mail_music": int,                //结合mail_newest_id来控制新邮件音乐 1播放 0不播放
    "recommend_player_num": int       //推荐邀请玩家人数
    }
    */

    Json::Value& jsonSvrStat = rjsonResult["svr_stat"];

    TbUser_stat* ptbStat = &pstUser->m_tbUserStat;
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();

    jsonSvrStat["has_mail"] = ptbStat->m_nNewest_mailid > ptbStat->m_nReturn_mailid ? 1 : 0;
    jsonSvrStat["has_report"] = ptbStat->m_nNewest_reportid > ptbStat->m_nReturn_reportid ? 1 : 0;
    jsonSvrStat["has_help_bubble"] = udwCurTime > ptbStat->m_nLast_help_bubble_time_out ? EN_HELP_BUBBLE_STATUS_OFF : EN_HELP_BUBBLE_STATUS_ON;
    jsonSvrStat["unread_mail"] = ptbStat->m_nUnread_mail;
    jsonSvrStat["unread_report"] = ptbStat->m_nUnread_report;
    jsonSvrStat["mail_newest_id"] = ptbStat->m_nNewest_mailid;
    jsonSvrStat["report_newest_id"] = ptbStat->m_nNewest_reportid;
    jsonSvrStat["mail_music"] = FALSE;
    jsonSvrStat["recommend_player_num"] = 0;
    jsonSvrStat["recommend_player_num"] = 0;
}

TVOID CUserJson::GenAllianceJson(SUserInfo *pstUser, Json::Value& rjsonResult)
{
    /*
    "svr_alliance": 
    {
        "base": [
            int,  //aid
            string, //alname
            long, //owner_id
            string, //owner_name
            string, //desc
            string, //notice
            int,   //member num
            long,  //force
            int, //目前的联盟加入政策
            int, //联盟语言
            long, //联盟礼物点数
            long, //联盟基金
            int, //联盟旗帜
            string //联盟简称
        ],
        "al_join_req_num": int, //申请加入联盟的用户数量
        "can_help_action_num": int, // 可以帮助的action数目
        "can_help_action_list": // 可以帮助的action的 id 列表 具体action从svr_al_action_list获取
        [
            long,
            ...
        ],
        "wall_msg_num": int,
        "al_gift_can_open_num": int, //可以打开的联盟礼物数量
        "al_gift_list":
        [
            [int, int, long, int, int, [[int,int,int],...]]        //[gift_id, pack_id, end_time, status, gift_point, [[type, id, num]]]
        ],
        "can_assist_num" : int //未读的可以assist的数量
        "throne_pos": int, //占领或者控制中的王座/省会坐标
        "throne_status": int, //王座/省会占领状态
        "al_star": int //联盟活跃程度
        "hive_svr": int //联盟据点服号
        "hive_pos": int //联盟据点坐标
        "hive_status": int //据点开关是否开启，1为开启
    }
    */

    TbAlliance* ptbAlliance = &pstUser->m_tbAlliance;

    Json::Value& rjsonAlliance = rjsonResult["svr_alliance"];
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

    rjsonAlliance["al_join_req_num"] = 0;
    rjsonAlliance["can_help_action_num"] = 0;
    rjsonAlliance["can_help_action_list"] = Json::Value(Json::arrayValue);

    rjsonAlliance["wall_msg_num"] = 0;

    rjsonAlliance["al_gift_list"] = Json::Value(Json::arrayValue);
    rjsonAlliance["al_gift_can_open_num"] = 0;

    rjsonAlliance["can_assist_num"] = 0;
    rjsonAlliance["throne_pos"] = ptbAlliance->m_nThrone_pos;
    rjsonAlliance["throne_status"] = ptbAlliance->m_nThrone_status;
    rjsonAlliance["al_star"] = ptbAlliance->m_nAl_star;
    rjsonAlliance["hive_svr"] = ptbAlliance->m_nHive_svr;
    rjsonAlliance["hive_pos"] = ptbAlliance->m_nHive_pos;
    rjsonAlliance["hive_status"] = ptbAlliance->m_nHive_show_flag;
}

TVOID CUserJson::GenDiplomacyJson(SUserInfo *pstUser, Json::Value& rjsonResult)
{
    /*
    "svr_diplomacy_list": //联盟间外交关系
    [
        {
            "alid": int,
            "type": int //EDiplomacyType
        }
    ]
    */

    Json::Value& jsonDiplomacyList = rjsonResult["svr_diplomacy_list"];
    jsonDiplomacyList = Json::Value(Json::arrayValue);
    if(pstUser->m_udwDiplomacyNum > pstUser->m_udwDiplomacyDelNum && pstUser->m_tbPlayer.m_nAlpos != EN_ALLIANCE_POS__REQUEST)
    {
        for(TUINT32 udwIdx = 0, udwJsonIndex = 0; udwIdx < pstUser->m_udwDiplomacyNum; ++udwIdx)
        {
            if(pstUser->m_aucDiplomacyFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
            {
                continue;
            }
            jsonDiplomacyList[udwJsonIndex] = Json::Value(Json::objectValue);
            jsonDiplomacyList[udwJsonIndex]["alid"] = pstUser->m_atbDiplomacy[udwIdx].m_nDes_al;
            jsonDiplomacyList[udwJsonIndex]["type"] = pstUser->m_atbDiplomacy[udwIdx].m_nType;
            ++udwJsonIndex;
        }
    }
}

TVOID CUserJson::GenBuffJson(SUserInfo *pstUser, Json::Value& rJson)
{
    /*
    "svr_buff": // idx is buff_id
    [
    long//buff_num
    ],
    "svr_buff_without_dragon": // idx is buff_id
    [
    long//buff_num
    ]
    */

    Json::Value& rJsonBuffer = rJson["svr_buff"];
    rJsonBuffer = Json::Value(Json::arrayValue);
    for (TUINT32 udwIdx = 0; udwIdx < EN_BUFFER_INFO_USER_JSON_OUTPUT_END; ++udwIdx)
    {
        rJsonBuffer.append(pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[udwIdx].m_ddwBuffTotal);
    }

    Json::Value& rJsonBufferNew = rJson["svr_buff_without_dragon"];
    rJsonBufferNew = Json::Value(Json::arrayValue);
    for (TUINT32 udwIdx = 0; udwIdx < EN_BUFFER_INFO_USER_JSON_OUTPUT_END; ++udwIdx)
    {
        rJsonBufferNew.append(pstUser->m_stBuffWithoutDragon.m_astPlayerBuffInfo[udwIdx].m_ddwBuffTotal);
    }
}

TVOID CUserJson::GenTimeQuestJson(SUserInfo *pstUser, Json::Value& rjsonResult)
{
    //"svr_time_quest":{
    //    "daily":{
    //        "refresh_time":1383895880,
    //        "quest_list" : [
    //            {
    //                "begin_time":1383896040, //begin time
    //                "cost_time" : 255,        //cost time
    //                "status" : 4,          	//
    //                "lv" : 0,          		//level
    //                "reward" : [
    //                    {
    //                        12,
    //                        43,
    //                        2
    //                    }
    //                ]
    //             }
    //        ]
    //    },
    //    "alliance":{
    //        "refresh_time":1383895880,
    //        "quest_list" : [
    //             {
    //                "begin_time":1383896040, //begin time
    //                "cost_time" : 255,        //cost time
    //                "status" : 4,          	//
    //                "lv" : 0,          		//level
    //                "reward" : [
    //                    {
    //                        12,
    //                        43,
    //                        2
    //                    }
    //                ]
    //            }
    //        ]
    //    },
    //    "mistery_gift":{
    //        "end_time"         	//
    //        "reward":[
    //             {
    //                12,
    //                43,
    //                2
    //             }
    //        ]
    //    },
    //    "vip":{
    //        "refresh_time":1383895880,
    //        "quest_list" : [
    //             {
    //                "begin_time":1383896040, //begin time
    //                "cost_time" : 255,        //cost time
    //                "status" : 4,          	//
    //                "lv" : 0,          		//level
    //                "reward" : [
    //                    {
    //                        12,
    //                        43,
    //                        2
    //                    }
    //                ]
    //            }
    //        ]
    //    },
    //}

    Json::Value& jsonSvrTimeQuest = rjsonResult["svr_time_quest"];
    jsonSvrTimeQuest = Json::Value(Json::objectValue);

    Json::Value& jsonDailyQuest = jsonSvrTimeQuest["daily"];
    jsonDailyQuest = Json::Value(Json::objectValue);
    CUserJson::GenQuestNodeJson(&pstUser->m_tbQuest.m_bDaily_quest[0], jsonDailyQuest);

    Json::Value& jsonVipQuest = jsonSvrTimeQuest["vip"];
    jsonVipQuest = Json::Value(Json::objectValue);
    if (pstUser->m_tbPlayer.m_nVip_etime >= CTimeUtils::GetUnixTime())
    {
        CUserJson::GenQuestNodeJson(&pstUser->m_tbQuest.m_bVip_quest[0], jsonVipQuest);
    }

    Json::Value& jsonAllianceQuest = jsonSvrTimeQuest["alliance"];
    jsonAllianceQuest = Json::Value(Json::objectValue);
    if (pstUser->m_tbPlayer.m_nAlid != 0 && pstUser->m_tbPlayer.m_nAlpos != EN_ALLIANCE_POS__REQUEST)
    {
        CUserJson::GenQuestNodeJson(&pstUser->m_tbQuest.m_bAl_quest[0], jsonAllianceQuest);
    }

    TUINT32 udwMisteryGiftEndTime = pstUser->m_tbQuest.m_bTimer_gift[0].m_stQuestCom[0].m_ddwCTime + pstUser->m_tbQuest.m_bTimer_gift[0].m_stQuestCom[0].m_ddwBTime;
    Json::Value& jsonMisteryGift = jsonSvrTimeQuest["mistery_gift"];
    jsonMisteryGift = Json::Value(Json::objectValue);
    jsonMisteryGift["end_time"] = udwMisteryGiftEndTime;

    Json::Value& jsonQuestReward = jsonMisteryGift["reward"];
    jsonQuestReward = Json::Value(Json::arrayValue);
    for (TUINT32 udwRewardIdx = 0; udwRewardIdx < pstUser->m_tbQuest.m_bTimer_gift[0].m_stQuestCom[0].m_stReward.ddwTotalNum; ++udwRewardIdx)
    {
        if (MAX_REWARD_ITEM_NUM <= udwRewardIdx)
        {
            break;
        }
        Json::Value& jsonQuestRewarNode = jsonQuestReward[udwRewardIdx];
        jsonQuestRewarNode = Json::Value(Json::arrayValue);
        jsonQuestRewarNode[0U] = pstUser->m_tbQuest.m_bTimer_gift[0].m_stQuestCom[0].m_stReward.aRewardList[udwRewardIdx].ddwType;
        jsonQuestRewarNode[1U] = pstUser->m_tbQuest.m_bTimer_gift[0].m_stQuestCom[0].m_stReward.aRewardList[udwRewardIdx].ddwId;
        jsonQuestRewarNode[2U] = pstUser->m_tbQuest.m_bTimer_gift[0].m_stQuestCom[0].m_stReward.aRewardList[udwRewardIdx].ddwNum;
    }
}

TVOID CUserJson::GenQuestNodeJson(SQuestNode *pstQuestNode, Json::Value& rjsonResult)
{
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    TINT64 ddwQuestRefreshtime = 0;
    if (pstQuestNode->m_ddwRTime <= udwCurTime)
    {
        TUINT32 udwRefreshTime = CGameInfo::GetInstance()->m_oJsonRoot["game_basic"][EN_GAME_BASIC_QUEST_REFRESH_CYCLE].asUInt();
        TUINT32 udwCount = 1 + (udwCurTime - pstQuestNode->m_ddwRTime) / udwRefreshTime;
        ddwQuestRefreshtime = pstQuestNode->m_ddwRTime + udwCount * udwRefreshTime;
    }
    else
    {
        ddwQuestRefreshtime = pstQuestNode->m_ddwRTime;
    }

    rjsonResult["refresh_time"] = ddwQuestRefreshtime;

    Json::Value& jsonQuestList = rjsonResult["quest_list"];
    jsonQuestList = Json::Value(Json::arrayValue);

    for (TUINT32 udwIdx = 0; udwIdx < pstQuestNode->m_ddwQuestNum; ++udwIdx)
    {
        Json::Value& jsonQuestNode = jsonQuestList[udwIdx];
        jsonQuestNode = Json::Value(Json::objectValue);
        jsonQuestNode["begin_time"] = pstQuestNode->m_stQuestCom[udwIdx].m_ddwBTime;
        jsonQuestNode["cost_time"] = pstQuestNode->m_stQuestCom[udwIdx].m_ddwCTime;
        jsonQuestNode["status"] = pstQuestNode->m_stQuestCom[udwIdx].m_ddwStatus;
        jsonQuestNode["lv"] = pstQuestNode->m_stQuestCom[udwIdx].m_ddwLv;

        //临时屏蔽BUG
        if (pstQuestNode->m_stQuestCom[udwIdx].m_ddwLv == 0)
        {
            jsonQuestNode["lv"] = 1;
            jsonQuestNode["status"] = EN_TIME_QUEST_STATUS_DONE;
        }

        Json::Value& jsonQuestReward = jsonQuestNode["reward"];
        jsonQuestReward = Json::Value(Json::arrayValue);
        for (TUINT32 udwRewardIdx = 0; udwRewardIdx < pstQuestNode->m_stQuestCom[udwIdx].m_stReward.ddwTotalNum; ++udwRewardIdx)
        {
            if (MAX_REWARD_ITEM_NUM <= udwRewardIdx)
            {
                break;
            }
            Json::Value& jsonQuestRewarNode = jsonQuestReward[udwRewardIdx];
            jsonQuestRewarNode = Json::Value(Json::arrayValue);
            jsonQuestRewarNode[0U] = pstQuestNode->m_stQuestCom[udwIdx].m_stReward.aRewardList[udwRewardIdx].ddwType;
            jsonQuestRewarNode[1U] = pstQuestNode->m_stQuestCom[udwIdx].m_stReward.aRewardList[udwRewardIdx].ddwId;
            jsonQuestRewarNode[2U] = pstQuestNode->m_stQuestCom[udwIdx].m_stReward.aRewardList[udwRewardIdx].ddwNum;
        }
    }
}

TVOID CUserJson::GenBackPackJson(SUserInfo *pstUser, Json::Value& rJson)
{
    /*
    "svr_equip_gride"：
	{
		"limit"://int 玩家拥有装备格子上限 
		"cost"://int  玩家已用装备格子数
	}
    "svr_equip": {
		id:{		//唯一Id
			id:		//game json id
			end_time:
			status: //0：普通状态 1：合成中 2：穿戴中
			pos:	//穿戴位置的pos（1-7）
			crystal:[
			],
			buff
			{
				buff_id
				[
					// buff id
					// num
					// time  持续时间，单位秒，-1表示持续有效
					// type 0-basic 基础值, 1-bonus 加成比; 0-basic 对应的num 是整数;1-bonus 对应的num，万位表示，比如10000 表示100%,主要用作展示，客户端需要
				]
			}
			mistery_buff
			{
				buff_id
				[
					// buff id
					// num
					// time  持续时间，单位秒，-1表示持续有效
					// type 0-basic 基础值, 1-bonus 加成比; 0-basic 对应的num 是整数;1-bonus 对应的num，万位表示，比如10000 表示100%,主要用作展示，客户端需要
				]
			}
		}
	}
	svr_crystal:
	{
		"id":num
	}

	svr_material
	{
		"id":num
	}

	svr_soul
	{
		"id":num
	}

	svr_parts
	{
		"id":num
	}
	svr_sp_crystal:
	{
		"id":num
	}
    */

    TINT64 udwBuffEffect = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_CORE_EQUIPMENT_DURATION].m_ddwBuffTotal;
    //equip grid 
    Json::Value &rOutPutEquipGrid = rJson["svr_equip_gride"];
    rOutPutEquipGrid = Json::Value(Json::objectValue);
    rOutPutEquipGrid["limit"] = pstUser->m_tbUserStat.m_nEquip_gride;

    TUINT32 udwCost = 0;
    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwEquipNum; ++udwIdx)
    {
        if (pstUser->m_aucEquipFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }
        udwCost++;
    }
    rOutPutEquipGrid["cost"] = udwCost;

    //equip
    Json::Value& rOutPutEquip = rJson["svr_equip"];
    rOutPutEquip = Json::Value(Json::objectValue);

    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwEquipNum; ++udwIdx)
    {
        if (pstUser->m_aucEquipFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL ||
            pstUser->m_atbEquip[udwIdx].m_nId == 0)
        {
            continue;
        }
        SEquipMentInfo stEquipInfo;
        stEquipInfo.Reset();
        TUINT64 uddwEquipId = pstUser->m_atbEquip[udwIdx].m_nId;
        CBackpack::GetEquipInfoById(pstUser->m_atbEquip, pstUser->m_udwEquipNum, uddwEquipId, &stEquipInfo);
        if(stEquipInfo.stStatusInfo.udwStatus == EN_EQUIPMENT_STATUS_UPGRADING)
        {
            continue;
        }
        Json::Value& rOutPutEquipInfo = rOutPutEquip[CCommonFunc::NumToString(uddwEquipId)];
        rOutPutEquipInfo = Json::Value(Json::objectValue);
        rOutPutEquipInfo["id"] = stEquipInfo.stBaseInfo.udwEType;
        rOutPutEquipInfo["lv"] = stEquipInfo.stBaseInfo.udwLv;

        TFLOAT64 ffRate = 0.0001;
        TUINT32 udwBuffEffectTime = stEquipInfo.stBaseInfo.udwEffectTime * (1.0 + ffRate*udwBuffEffect);

        if(stEquipInfo.stBaseInfo.udwCategory == 1 && stEquipInfo.stStatusInfo.udwEquipmentPutOnTime)
        {
            rOutPutEquipInfo["end_time"] = stEquipInfo.stStatusInfo.udwEquipmentPutOnTime + udwBuffEffectTime;
        }
        else
        {
            rOutPutEquipInfo["end_time"] = 0;
        }
        rOutPutEquipInfo["pos"] = stEquipInfo.stStatusInfo.udwPotOnPos;
        rOutPutEquipInfo["status"] = stEquipInfo.stStatusInfo.udwStatus;
        rOutPutEquipInfo["get_time"] = pstUser->m_atbEquip[udwIdx].m_nGet_time;

        for(TUINT32 udwIdx = 0; udwIdx < MAX_CRYSTAL_IN_EQUIPMENT;++udwIdx)
        {
            rOutPutEquipInfo["crystal"].append(stEquipInfo.stStatusInfo.audwSlot[udwIdx]);
        }
        Json::Value &oBufferJson = rOutPutEquipInfo["buff"];
        oBufferJson = Json::Value(Json::objectValue);
        for(TUINT32 udwIdx = 0; udwIdx < stEquipInfo.stStatusInfo.udwBufferNum;++udwIdx)
        {
            Json::Value& rBufferInfo = oBufferJson[CCommonFunc::NumToString(stEquipInfo.stStatusInfo.astBuffInfo[udwIdx].m_udwId)];
            rBufferInfo = Json::Value(Json::arrayValue);
            rBufferInfo.append(stEquipInfo.stStatusInfo.astBuffInfo[udwIdx].m_udwId);
            rBufferInfo.append(stEquipInfo.stStatusInfo.astBuffInfo[udwIdx].m_dwNum);
            rBufferInfo.append(-1);
            rBufferInfo.append(stEquipInfo.stStatusInfo.astBuffInfo[udwIdx].m_udwType);
        }
        Json::Value &oMistoryBufferJson = rOutPutEquipInfo["mistery_buff"];
        oMistoryBufferJson = Json::Value(Json::objectValue);
        for(TUINT32 udwIdx = 0; udwIdx < stEquipInfo.stStatusInfo.udwMistoryBufferNum; ++udwIdx)
        {
            Json::Value& rBufferInfo = oMistoryBufferJson[CCommonFunc::NumToString(stEquipInfo.stStatusInfo.astMisteryBuffInfo[udwIdx].m_udwId)];
            rBufferInfo = Json::Value(Json::arrayValue);
            rBufferInfo.append(stEquipInfo.stStatusInfo.astMisteryBuffInfo[udwIdx].m_udwId);
            rBufferInfo.append(stEquipInfo.stStatusInfo.astMisteryBuffInfo[udwIdx].m_dwNum);
            rBufferInfo.append(-1);
            rBufferInfo.append(stEquipInfo.stStatusInfo.astMisteryBuffInfo[udwIdx].m_udwType);
        }
    }

    //crystal
    Json::Value& rOutPutCrystal = rJson["svr_crystal"];
    rOutPutCrystal = Json::Value(Json::objectValue);
    rOutPutCrystal = pstUser->m_tbBackpack.m_jCrystal;

    //material
    Json::Value& rOutPutMaterial = rJson["svr_material"];
    rOutPutMaterial = Json::Value(Json::objectValue);
    rOutPutMaterial = pstUser->m_tbBackpack.m_jMaterial;

    //soul
    Json::Value& rOutPutSoul = rJson["svr_soul"];
    rOutPutSoul = Json::Value(Json::objectValue);
    rOutPutSoul = pstUser->m_tbBackpack.m_jSoul;

    //parts
    Json::Value& rOutPutParts = rJson["svr_parts"];
    rOutPutParts = Json::Value(Json::objectValue);
    rOutPutParts = pstUser->m_tbBackpack.m_jParts;

    //sp_crystal
    Json::Value& rOutPuSptCrystal = rJson["svr_sp_crystal"];
    rOutPuSptCrystal = Json::Value(Json::objectValue);
    rOutPuSptCrystal = pstUser->m_tbBackpack.m_jSp_crystal;

    //scroll old v1.0
    Json::Value& rOutPutScroll = rJson["svr_scrolls"];
    rOutPutScroll = Json::Value(Json::objectValue);
    rOutPutScroll = pstUser->m_tbBackpack.m_jScroll;

    //scroll new v1.1
    Json::Value& rOutPutScrollNew = rJson["svr_scrolls_new"];
    rOutPutScrollNew = Json::Value(Json::objectValue);
    Json::Value::Members members = pstUser->m_tbBackpack.m_jScroll.getMemberNames();
    for (Json::Value::Members::iterator it = members.begin(); it != members.end(); it++)
    {
        rOutPutScrollNew[*it] = Json::Value(Json::objectValue);
        rOutPutScrollNew[*it]["num"] = pstUser->m_tbBackpack.m_jScroll[*it];
        if (pstUser->m_tbBackpack.m_jScroll_get_time.isMember(*it))
        {
            rOutPutScrollNew[*it]["get_time"] = pstUser->m_tbBackpack.m_jScroll_get_time[*it];
        }
        else
        {
            rOutPutScrollNew[*it]["get_time"] = 0;
        }
    }
}

TVOID CUserJson::GenActionNewJson(SUserInfo *pstUser, Json::Value& rjsonResult)
{
    //"svr_action_list":[ ]// 个人action列表
    //"svr_p_action_list":[], // 个人被动action列表
    //"svr_al_action_list" : [], // 联盟主动action列表
    //"svr_al_p_action_list" : [], // 联盟被动action列表

    TbPlayer* ptbPlayer = &pstUser->m_tbPlayer;

    const Json::Value& oWildResJson = CWildInfo::GetWildResInfo(ptbPlayer->m_nSid);

    rjsonResult["svr_action_list"] = Json::Value(Json::objectValue);
    rjsonResult["svr_p_action_list"] = Json::Value(Json::objectValue);
    rjsonResult["svr_al_action_list"] = Json::Value(Json::objectValue);
    rjsonResult["svr_al_p_action_list"] = Json::Value(Json::objectValue);

    TINT64 ddwThroneTroopNum = 0;
    TINT64 ddwThroneTroopForce = 0;

    TUINT32 udwAIndex = 0;
    TUINT32 udwPIndex = 0;
    TUINT32 udwAlAIndex = 0;
    TUINT32 udwAlPIndex = 0;

    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwActionNum; ++udwIdx)
    {
        if (EN_TABLE_UPDT_FLAG__DEL == pstUser->m_aucActionFlag[udwIdx])
        {
            continue;
        }

        CCommJson::GenActionInfoForPush(&pstUser->m_atbAction[udwIdx], rjsonResult["svr_action_list"], pstUser->m_aucActionFlag[udwIdx]);
        ++udwAIndex;
    }

    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwSelfAlActionNum; ++udwIdx)
    {
        if (EN_TABLE_UPDT_FLAG__DEL == pstUser->m_aucSelfAlActionFlag[udwIdx])
        {
            continue;
        }

        if (CActionBase::IsPlayerOwnedAction(ptbPlayer->m_nUid, pstUser->m_atbSelfAlAction[udwIdx].m_nId))
        {
            CCommJson::GenAlActionInfoForPush(&pstUser->m_atbSelfAlAction[udwIdx], rjsonResult["svr_action_list"], pstUser->m_aucSelfAlActionFlag[udwIdx]);
            ++udwAIndex;

            if (pstUser->m_atbSelfAlAction[udwIdx].m_nCan_help_num > 0)
            {
                CCommJson::GenAlActionInfoForPush(&pstUser->m_atbSelfAlAction[udwIdx], rjsonResult["svr_al_action_list"], pstUser->m_aucSelfAlActionFlag[udwIdx]);
                ++udwAlAIndex;
            }
        }
        else
        {
            CCommJson::GenAlActionInfoForPush(&pstUser->m_atbSelfAlAction[udwIdx], rjsonResult["svr_al_action_list"], pstUser->m_aucSelfAlActionFlag[udwIdx]);
            ++udwAlAIndex;
        }
    }

    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
    {
        if (pstUser->m_atbMarch[udwIdx].m_nMclass == EN_ACTION_MAIN_CLASS__TIMER)
        {
            continue;
        }

        if (pstUser->m_atbMarch[udwIdx].m_nStatus == EN_MARCH_STATUS__RETURNING
            && CActionBase::IsEmptyMarch(&pstUser->m_atbMarch[udwIdx]))
        {
            if (pstUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__RALLY_REINFORCE
                || pstUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_NORMAL
                || pstUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_THRONE)
            {
                continue;
            }
        }

        if (EN_TABLE_UPDT_FLAG__DEL == pstUser->m_aucMarchFlag[udwIdx])
        {
            continue;
        }

        TbMarch_action tbTmpMarch = pstUser->m_atbMarch[udwIdx];
        if (CActionBase::IsPlayerOwnedAction(ptbPlayer->m_nUid, pstUser->m_atbMarch[udwIdx].m_nId))
        {
            if (pstUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR
                && pstUser->m_atbMarch[udwIdx].m_nStatus != EN_MARCH_STATUS__RETURNING
                && pstUser->m_atbMarch[udwIdx].m_nStatus != EN_MARCH_STATUS__DELING
                && pstUser->m_atbMarch[udwIdx].m_nStatus != EN_MARCH_STATUS__DELING_ON_FLY)
            {
//                 if (pstUser->m_atbMarch[udwIdx].m_nStatus == EN_MARCH_STATUS__WAITING && pstUser->m_atbMarch[udwIdx].m_nEtime == INT64_MAX)
//                 {
//                     TINT64 ddwEndTime = 0;
//                     for (TUINT32 udwIdy = 0; udwIdy < pstUser->m_udwPassiveMarchNum; udwIdy++)
//                     {
//                         if (pstUser->m_aucPassiveMarchFlag[udwIdy] == EN_TABLE_UPDT_FLAG__DEL)
//                         {
//                             continue;
//                         }
//                         if (pstUser->m_atbPassiveMarch[udwIdy].m_nSclass == EN_ACTION_SEC_CLASS__RALLY_REINFORCE
//                             && pstUser->m_atbPassiveMarch[udwIdy].m_nTid == pstUser->m_atbMarch[udwIdx].m_nId
//                             && pstUser->m_atbPassiveMarch[udwIdy].m_nStatus == EN_MARCH_STATUS__MARCHING)
//                         {
//                             if (pstUser->m_atbPassiveMarch[udwIdy].m_nEtime > ddwEndTime)
//                             {
//                                 ddwEndTime = pstUser->m_atbPassiveMarch[udwIdy].m_nEtime;
//                             }
//                         }
//                     }
//                     tbTmpMarch.m_nEtime = ddwEndTime;
//                 }

                CCommJson::GenMarchInfoForPush(&tbTmpMarch, rjsonResult["svr_al_action_list"], pstUser->m_aucMarchFlag[udwIdx]);
                ++udwAlAIndex;
            }

            //修正marching状态中reinforce的时间显示
            if (pstUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__RALLY_REINFORCE
                && pstUser->m_atbMarch[udwIdx].m_nStatus == EN_MARCH_STATUS__PREPARING
                && pstUser->m_atbMarch[udwIdx].m_nEtime == INT64_MAX)
            {
                //寻找主rally war action
                for (TUINT32 udwIdz = 0; udwIdz < pstUser->m_udwMarchNum; udwIdz++)
                {
                    if (pstUser->m_aucMarchFlag[udwIdz] == EN_TABLE_UPDT_FLAG__DEL)
                    {
                        continue;
                    }
                    if (pstUser->m_atbMarch[udwIdz].m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR
                        &&pstUser->m_atbMarch[udwIdx].m_nTid == pstUser->m_atbMarch[udwIdz].m_nId)
                    {
                        if (pstUser->m_atbMarch[udwIdz].m_nStatus == EN_MARCH_STATUS__MARCHING)
                        {
                            tbTmpMarch.m_nEtime = pstUser->m_atbMarch[udwIdz].m_nEtime;
                        }
                        break;
                    }
                }
            }

            //TODO
//             if (pstUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_NORMAL
//                 && pstUser->m_atbMarch[udwIdx].m_nStatus == EN_MARCH_STATUS__DEFENDING
//                 && pstUser->m_atbMarch[udwIdx].m_nTpos == pstUser->m_tbAlliance.m_nThrone_pos)
//             {
//                 ddwThroneTroopNum += pstUser->m_atbMarch[udwIdx].m_bParam[0].m_ddwTroopNum;
//                 ddwThroneTroopForce += pstUser->m_atbMarch[udwIdx].m_bParam[0].m_ddwForce;
//             }

            CCommJson::GenMarchInfoForPush(&tbTmpMarch, rjsonResult["svr_action_list"], pstUser->m_aucMarchFlag[udwIdx]);
            ++udwAIndex;
        }
        else
        {
            //TODO
//             if (pstUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_NORMAL
//                 && pstUser->m_atbMarch[udwIdx].m_nStatus == EN_MARCH_STATUS__DEFENDING
//                 && pstUser->m_atbMarch[udwIdx].m_nTpos == pstUser->m_tbAlliance.m_nThrone_pos)
//             {
//                 ddwThroneTroopNum += pstUser->m_atbMarch[udwIdx].m_bParam[0].m_ddwTroopNum;
//                 ddwThroneTroopForce += pstUser->m_atbMarch[udwIdx].m_bParam[0].m_ddwForce;
//             }

            //rally reinforce 不输出
            if (pstUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR
                && pstUser->m_atbMarch[udwIdx].m_nStatus != EN_MARCH_STATUS__RETURNING)
            {
//                 if (pstUser->m_atbMarch[udwIdx].m_nStatus == EN_MARCH_STATUS__WAITING && pstUser->m_atbMarch[udwIdx].m_nEtime == INT64_MAX)
//                 {
//                     TINT64 ddwEndTime = 0;
//                     for (TUINT32 udwIdy = 0; udwIdy < pstUser->m_udwMarchNum; udwIdy++)
//                     {
//                         if (pstUser->m_aucMarchFlag[udwIdy] == EN_TABLE_UPDT_FLAG__DEL)
//                         {
//                             continue;
//                         }
//                         if (pstUser->m_atbMarch[udwIdy].m_nSclass == EN_ACTION_SEC_CLASS__RALLY_REINFORCE
//                             && pstUser->m_atbMarch[udwIdy].m_nTid == pstUser->m_atbMarch[udwIdx].m_nId
//                             && pstUser->m_atbMarch[udwIdy].m_nStatus == EN_MARCH_STATUS__MARCHING)
//                         {
//                             if (pstUser->m_atbMarch[udwIdy].m_nEtime > ddwEndTime)
//                             {
//                                 ddwEndTime = pstUser->m_atbMarch[udwIdy].m_nEtime;
//                             }
//                         }
//                     }
//                     tbTmpMarch.m_nEtime = ddwEndTime;
//                 }
                CCommJson::GenMarchInfoForPush(&tbTmpMarch, rjsonResult["svr_al_action_list"], pstUser->m_aucMarchFlag[udwIdx]);
                ++udwAlAIndex;
            }
        }
    }

    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwPassiveMarchNum; ++udwIdx)
    {
        if (pstUser->m_atbPassiveMarch[udwIdx].m_nMclass == EN_ACTION_MAIN_CLASS__TIMER)
        {
            continue;
        }

        if (EN_TABLE_UPDT_FLAG__DEL == pstUser->m_aucPassiveMarchFlag[udwIdx])
        {
            continue;
        }

        TbMarch_action tbTmpMarch = pstUser->m_atbPassiveMarch[udwIdx];
        TUINT32 udwMapType = oWildResJson[CCommonFunc::NumToString(tbTmpMarch.m_bParam[0].m_ddwTargetType)]["a0"]["a0"].asUInt();

        if (pstUser->m_atbPassiveMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR)
        {
            CCommJson::GenMarchInfoForPush(&tbTmpMarch, rjsonResult["svr_al_p_action_list"], pstUser->m_aucPassiveMarchFlag[udwIdx]);
            ++udwAlPIndex;
            if (pstUser->m_atbPassiveMarch[udwIdx].m_nTuid == ptbPlayer->m_nUid)
            {
                CCommJson::GenMarchInfoForPush(&tbTmpMarch, rjsonResult["svr_p_action_list"], pstUser->m_aucPassiveMarchFlag[udwIdx]);
                ++udwPIndex;
            }
        }
        else
        {
            TBOOL bFind = FALSE;
            if (pstUser->m_atbPassiveMarch[udwIdx].m_nTpos == pstUser->m_stCityInfo.m_stTblData.m_nPos)
            {
                bFind = TRUE;
            }
            for (TUINT32 udwWildIndex = 0; udwWildIndex < pstUser->m_udwWildNum; ++udwWildIndex)
            {
                if (pstUser->m_atbPassiveMarch[udwIdx].m_nTpos == pstUser->m_atbWild[udwWildIndex].m_nId)
                {
                    bFind = TRUE;
                }
            }
            if (pstUser->m_atbPassiveMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__CATCH_DRAGON && pstUser->m_atbPassiveMarch[udwIdx].m_nTuid == pstUser->m_tbPlayer.m_nUid)
            {
                bFind = TRUE;
            }
            if (pstUser->m_atbPassiveMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__ATTACK && udwMapType == EN_WILD_CLASS_MONSTER_NEST)
            {
                for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
                {
                    TbMarch_action *ptbAction = &pstUser->m_atbMarch[udwIdx];
                    if (ptbAction->m_nSclass == EN_ACTION_SEC_CLASS__OCCUPY && ptbAction->m_nTpos == tbTmpMarch.m_nTpos &&
                        ptbAction->m_nStatus == EN_MARCH_STATUS__LOADING && ptbAction->m_nSuid == ptbPlayer->m_nUid)
                    {
                        bFind = TRUE;
                    }
                }
            }

            if (!bFind)
            {
                continue;
            }

            CCommJson::GenMarchInfoForPush(&tbTmpMarch, rjsonResult["svr_p_action_list"], pstUser->m_aucPassiveMarchFlag[udwIdx]);
            ++udwPIndex;
        }
    }
}