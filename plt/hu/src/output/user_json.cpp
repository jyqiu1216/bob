#include "common_func.h"
#include "time_utils.h"
#include "city_base.h"
#include "user_json.h"
#include "game_info.h"
#include "common_base.h"
#include "item_base.h"
#include "common_json.h"
#include "player_base.h"
#include "action_base.h"
#include "game_command.h"
#include "alliance_mapping.h"
#include "quest_logic.h"
#include "common_logic.h"
#include "buffer_base.h"
#include "backpack_logic.h"
#include "wild_info.h"
#include "output_conf.h"

TVOID CUserJson::GenDataJson(SSession* pstSession, Json::Value& rJson)
{
    GenLoginJson(pstSession, rJson);
    GenPlayerJson(pstSession, rJson);
    GenResearchJson(pstSession, rJson);
    GenLordSkillJson(pstSession, rJson);
    GenDragonSkillJson(pstSession, rJson);
    GenDragonMonsterSkillJson(pstSession, rJson);
    GenDailyLoginJson(pstSession, rJson);
    GenKnightInfo(pstSession, rJson);

    GenCityJson(pstSession, rJson);
    GenBagJson(pstSession, rJson);
    GenTopQuestJson(pstSession, rJson);
    GenBlackListJson(pstSession, rJson);
    GenBookMarkListJson(pstSession, rJson);
    GenStatJson(pstSession, rJson);

    GenActionNewJson(pstSession, rJson);

    /*
    if(pstSession->m_dwClientReqMode == EN_CLIENT_REQ_MODE__TCP)
    {
        GenActionNewJson(pstSession, rJson);
    }
    else
    {
        GenActionJson(pstSession, rJson);
    }
    */
    GenTipsJson(pstSession, rJson);
    GenTitleJson(pstSession, rJson);
    GenAllianceJson(pstSession, rJson);
    GenDiplomacyJson(pstSession, rJson);
    GenAlStoreJson(pstSession, rJson);
    GenClientFlag(pstSession, rJson);
    GenRewardWindowJson(pstSession, rJson);
    GenBuffJson(pstSession, rJson);

    GenTimeQuestJson(pstSession, rJson);

    //equip_json.h
    GenBackPackJson(pstSession, rJson);

    //task.h
    CCommJson::GenTaskInfo(&pstSession->m_stUserInfo, rJson);

    CCommJson::GenEventTipsJson(&pstSession->m_stUserInfo, rJson);
    //CCommJson::GenThemeEventTipsJson(&pstSession->m_stUserInfo, rJson);

    //svr_list_json.h
    if((pstSession->m_stReqParam.m_ucLoginStatus == EN_LOGIN_STATUS__LOGIN && strcmp(pstSession->m_stReqParam.m_szCommand, "login_get") == 0) ||
        pstSession->m_stReqParam.m_udwCommandID == EN_CLIENT_REQ_COMMAND__SVR_GET || strcmp(pstSession->m_stReqParam.m_szCommand, "guide_finish") == 0)
    {
        GenSvrListJson(pstSession, rJson);
    }

    //bounty_json.h
    GenBountyInfo(pstSession, rJson);

    //broadcast.h
    GenBroadcastInfo(pstSession, rJson);

    GenEventInfo(pstSession, rJson);

    GenRewardWindowNewInfo(pstSession, rJson);

    GenRandomRewardInfo(pstSession, rJson);

    GenMonsterInfo(pstSession, rJson);

    GenTrialInfo(pstSession, rJson);

    GenComputeResInfo(pstSession, rJson);

    GenGemRechargeInfo(pstSession, rJson);

    GenIdolInfo(pstSession, rJson);

    GenThroneInfo(pstSession, rJson);
}

TVOID CUserJson::GenLoginJson(SSession* pstSession, Json::Value& rjsonResult)
{
    /*
    "svr_login": {
        "uid": int, //用户id
        "did": long, //device id
        "idfa": string, //idfa
        "gem": int, //现有宝石数量
        "gem_seq": int, //购买宝石的seq
        "gem_buy": int, // 购买过的宝石数量
        "gem_cost": int, // 消耗过的宝石数量
        "seq": long, //用户序列号
        "ctime": long, //用户的创建时间
        "utime": long, //用户数据的更新时间
        "last_svr": int, //上一次登录过的svr id
        "is_npc": int, //1 表示npc 0 表示正常玩家
        "guide_flag": // 表示用户已经提示过的指引, 包括新手教学 EGuideFinishStage
        [
            int,
            int
        ],
        "first_al_time": 1384136591 //第一次加入联盟的时间
        "apns_switch": [2, 1, 0, 2, 1] // 下标为类型 ENewNoticType，值0表示关，1表示仅text开，2表示text和sound全开 ENewNoticSwitchType
        "rating_switch": int
        "rating_gem": int
        "dragon_unlock_flag": int
        "login_time": int //上一次登录时间
    }
    */
    TbLogin* ptbLogin = &pstSession->m_stUserInfo.m_tbLogin;
    //if(pstSession->m_dwClientResType != 0 && ptbLogin->IfNeedUpdate() == false)
    //{
    //    return;
    //}

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
        // ONLY ouput the flag which is setted
        if(BITTEST(ptbLogin->m_bGuide_flag[0].m_bitFlag, udwIdx) != 0)
        {
            jsonSvrLogin["guide_flag"][udwJsonIndex] = udwIdx;
            udwJsonIndex++;
        }
    }

    jsonSvrLogin["first_al_time"] = ptbLogin->m_nAl_time;

    jsonSvrLogin["apns_switch"] = ptbLogin->m_jApns_switch;

    //rating
    jsonSvrLogin["rating_switch"] = pstSession->m_stUserInfo.m_bRatingSwitch;
    jsonSvrLogin["rating_gem"] = pstSession->m_stUserInfo.m_udwRatingGem;

    jsonSvrLogin["dragon_unlock_flag"] = pstSession->m_dwDragonUnlockFlag;
    jsonSvrLogin["kf_lv"] = pstSession->m_dwKfLv;
    jsonSvrLogin["login_time"] = pstSession->m_stUserInfo.m_tbLogin.m_nLast_lg_time;
}

TVOID CUserJson::GenPlayerJson(SSession* pstSession, Json::Value& rjsonResult)
{
    /*
    "svr_player": 
    {
        ...
    }
    */

    Json::Value& jsonSvrPlayer = rjsonResult["svr_player"];
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbPlayer* ptbPlayer = &pstSession->m_stUserInfo.m_tbPlayer;

    CCommJson::GenPlayerProfileJson(ptbPlayer, pstSession->m_stUserInfo.m_atbEquip, 
        pstSession->m_stUserInfo.m_udwEquipNum, jsonSvrPlayer, &pstSession->m_stUserInfo.m_stPlayerBuffList);

    //wave@20160415: 增加person guide id
    jsonSvrPlayer["guide_id"] = CPlayerBase::GetCurPersonGuide(pstUser);
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

    if (ptbPlayer->m_nUid != 0 && pstUser->m_jPlayerRankInfo.isObject() && pstUser->m_jPlayerRankInfo.isMember("uid")
        && pstUser->m_jPlayerRankInfo["uid"].asInt() == ptbPlayer->m_nUid)
    {
        Json::Value& rJsonPlayerRankInfo = rjsonResult["svr_rank_info"];
        rJsonPlayerRankInfo = pstUser->m_jPlayerRankInfo;
    }
}

TVOID CUserJson::GenCityJson(SSession* pstSession, Json::Value& rjsonResult)
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

    SCityInfo *pstCity = &pstSession->m_stUserInfo.m_stCityInfo;

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

    jsonSvrCity["total_troop"] = CCommonBase::CalcTroopsNum(&pstSession->m_stUserInfo);

    CCommJson::GenTroopJson(&pstCity->m_stTblData.m_bTroop[0], jsonSvrCity["troop"]);

    SCommonTroop stLiveTroop = pstCity->m_stTblData.m_bTroop[0];
    CCommonBase::CalcMarchActionTroop(&pstSession->m_stUserInfo, stLiveTroop.m_addwNum, FALSE);
    CCommJson::GenTroopJson(&stLiveTroop, jsonSvrCity["live_troop"]);

    CCommJson::GenTroopJson(&pstCity->m_stTblData.m_bHos_wait[0], jsonSvrCity["troop_wounded"]);

    jsonSvrCity["total_fort"] = CCommonBase::CalcFortsNum(&pstSession->m_stUserInfo);

    CCommJson::GenFortJson(&pstCity->m_stTblData.m_bFort[0], jsonSvrCity["fort"]);

    CCommJson::GenFortJson(&pstCity->m_stTblData.m_bDead_fort[0], jsonSvrCity["fort_dead"]);

    GenAltarJson(pstSession, pstCity, jsonSvrCity["altar"]);

    GenPrisonJson(pstSession, jsonSvrCity["prison"]);

    GenProductionJson(pstCity, jsonSvrCity["production"]);

    if (!pstSession->m_bHasPromoteData)
    {
        jsonSvrCity["building_pos"] = pstSession->m_JsonValue; //TODO 不知道啥意思
    }
    else
    {
        jsonSvrCity["building_pos"] = Json::Value(Json::objectValue);
    }

    jsonSvrCityList.append(jsonSvrCity);
}

TVOID CUserJson::GenAltarJson(SSession* pstSession, SCityInfo* pstCity, Json::Value& rjson)
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
    for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_udwTmpAltarHistoryNum; ++udwIdx)
    {
        if(pstSession->m_atbTmpAltarHistory[udwIdx].m_jContent["killer"][0u].asInt64() == pstSession->m_stUserInfo.m_tbPlayer.m_nUid)
        {
            rjson["kill_history"]["kill"].append(pstSession->m_atbTmpAltarHistory[udwIdx].m_jContent);
        }
        else
        {
            rjson["kill_history"]["killed"].append(pstSession->m_atbTmpAltarHistory[udwIdx].m_jContent);
        }
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

TVOID CUserJson::GenAllianceJson(SSession* pstSession, Json::Value& rjsonResult)
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
    SUserInfo* pstUser = &pstSession->m_stUserInfo;
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

    rjsonAlliance["al_join_req_num"] = pstUser->m_udwAllianceReqCurFindNum;
    rjsonAlliance["can_help_action_num"] = pstUser->m_udwCanHelpTaskNum;
    rjsonAlliance["can_help_action_list"] = Json::Value(Json::arrayValue);
    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwAlCanHelpActionNum; ++udwIdx)
    {
        rjsonAlliance["can_help_action_list"].append(pstUser->m_patbAlCanHelpAction[udwIdx]->m_nId);
    }
    rjsonAlliance["wall_msg_num"] = pstUser->m_udwNewAlWallMsgNum;

    CCommJson::GenAlGiftJson(pstUser, rjsonAlliance);

    rjsonAlliance["can_assist_num"] = pstUser->m_udwAllianceCanAssistNum;
    rjsonAlliance["throne_pos"] = ptbAlliance->m_nThrone_pos;
    rjsonAlliance["throne_status"] = ptbAlliance->m_nThrone_status;
    rjsonAlliance["al_star"] = ptbAlliance->m_nAl_star;
    rjsonAlliance["hive_svr"] = ptbAlliance->m_nHive_svr;
    rjsonAlliance["hive_pos"] = ptbAlliance->m_nHive_pos;
    rjsonAlliance["hive_status"] = ptbAlliance->m_nHive_show_flag;
}

TVOID CUserJson::GenActionNewJson( SSession* pstSession, Json::Value& rjsonResult )
{
    //"svr_action_list":[ ]// 个人action列表
    //"svr_p_action_list":[], // 个人被动action列表
    //"svr_al_action_list" : [], // 联盟主动action列表
    //"svr_al_p_action_list" : [], // 联盟被动action列表

    SUserInfo* pstUser = &pstSession->m_stUserInfo;
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

    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwActionNum; ++udwIdx)
    {
        //wave@20160709: for 增量更新
        if(FALSE == CCommJson::IfTableDataNeedOutput(pstSession->m_dwClientResType, &pstUser->m_atbAction[udwIdx], pstUser->m_aucActionFlag[udwIdx]))
        {
            continue;
        }
        
        CCommJson::GenActionInfoForPush(&pstUser->m_atbAction[udwIdx], rjsonResult["svr_action_list"], pstUser->m_aucActionFlag[udwIdx]);
        ++udwAIndex;
    }

    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwSelfAlActionNum; ++udwIdx) // 自己al_action都要输出
    {
        //wave@20160709: for 增量更新
        if(FALSE == CCommJson::IfTableDataNeedOutput(pstSession->m_dwClientResType, &pstUser->m_atbSelfAlAction[udwIdx], pstUser->m_aucSelfAlActionFlag[udwIdx]))
        {
            continue;
        }

        if(CActionBase::IsPlayerOwnedAction(ptbPlayer->m_nUid, pstUser->m_atbSelfAlAction[udwIdx].m_nId)) // 自己的都要输出
        {
            CCommJson::GenAlActionInfoForPush(&pstUser->m_atbSelfAlAction[udwIdx], rjsonResult["svr_action_list"], pstUser->m_aucSelfAlActionFlag[udwIdx]);
            ++udwAIndex;

            if(pstUser->m_atbSelfAlAction[udwIdx].m_nCan_help_num > 0)
            {
                CCommJson::GenAlActionInfoForPush(&pstUser->m_atbSelfAlAction[udwIdx], rjsonResult["svr_al_action_list"], pstUser->m_aucSelfAlActionFlag[udwIdx]);
                ++udwAlAIndex;
            }
        }
    }
    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwAlCanHelpActionNum; udwIdx++) //其他人的al_action要看情况输出
    {
        if(pstUser->m_patbAlCanHelpAction[udwIdx]->m_nSuid == ptbPlayer->m_nUid)
        {
            continue;
        }

        //wave@20160709: for 增量更新
        if(FALSE == CCommJson::IfTableDataNeedOutput(pstSession->m_dwClientResType, pstUser->m_patbAlCanHelpAction[udwIdx], EN_TABLE_UPDT_FLAG__UNCHANGE))
        {
            continue;
        }

        CCommJson::GenAlActionInfoForPush(pstUser->m_patbAlCanHelpAction[udwIdx], rjsonResult["svr_al_action_list"], EN_TABLE_UPDT_FLAG__UNCHANGE);
        ++udwAlAIndex;
    }

    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
    {
        if(pstUser->m_atbMarch[udwIdx].m_nMclass == EN_ACTION_MAIN_CLASS__TIMER)
        {
            continue;
        }

        if(pstUser->m_atbMarch[udwIdx].m_nStatus == EN_MARCH_STATUS__RETURNING
            && CActionBase::IsEmptyMarch(&pstUser->m_atbMarch[udwIdx]))
        {
            if(pstUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__RALLY_REINFORCE
                || pstUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_NORMAL
                || pstUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_THRONE)
            {
                continue;
            }
        }

        //wave@20160709: for 增量更新
        if(FALSE == CCommJson::IfTableDataNeedOutput(pstSession->m_dwClientResType, &pstUser->m_atbMarch[udwIdx], pstUser->m_aucMarchFlag[udwIdx]))
        {
            continue;
        }

        TbMarch_action tbTmpMarch = pstUser->m_atbMarch[udwIdx];
        if(CActionBase::IsPlayerOwnedAction(ptbPlayer->m_nUid, pstUser->m_atbMarch[udwIdx].m_nId))
        {
            if(pstUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR
                && pstUser->m_atbMarch[udwIdx].m_nStatus != EN_MARCH_STATUS__RETURNING
                && pstUser->m_atbMarch[udwIdx].m_nStatus != EN_MARCH_STATUS__DELING
                && pstUser->m_atbMarch[udwIdx].m_nStatus != EN_MARCH_STATUS__DELING_ON_FLY)
            {
                CCommJson::GenMarchInfoForPush(&tbTmpMarch, rjsonResult["svr_al_action_list"], pstUser->m_aucMarchFlag[udwIdx]);
                ++udwAlAIndex;
            }

            if (pstUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR_THRONE
                && pstUser->m_atbMarch[udwIdx].m_nStatus != EN_MARCH_STATUS__RETURNING
                && pstUser->m_atbMarch[udwIdx].m_nStatus != EN_MARCH_STATUS__DELING
                && pstUser->m_atbMarch[udwIdx].m_nStatus != EN_MARCH_STATUS__DELING_ON_FLY)
            {
                CCommJson::GenMarchInfoForPush(&tbTmpMarch, rjsonResult["svr_al_action_list"], pstUser->m_aucMarchFlag[udwIdx]);
                ++udwAlAIndex;
            }

            //修正marching状态中reinforce的时间显示
            if(pstUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__RALLY_REINFORCE
                && pstUser->m_atbMarch[udwIdx].m_nStatus == EN_MARCH_STATUS__PREPARING
                && pstUser->m_atbMarch[udwIdx].m_nEtime == INT64_MAX)
            {
                //寻找主rally war action
                for(TUINT32 udwIdz = 0; udwIdz < pstUser->m_udwMarchNum; udwIdz++)
                {
                    if(pstUser->m_aucMarchFlag[udwIdz] == EN_TABLE_UPDT_FLAG__DEL)
                    {
                        continue;
                    }
                    if((pstUser->m_atbMarch[udwIdz].m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR
                        || pstUser->m_atbMarch[udwIdz].m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR_THRONE)
                        &&pstUser->m_atbMarch[udwIdx].m_nTid == pstUser->m_atbMarch[udwIdz].m_nId)
                    {
                        if(pstUser->m_atbMarch[udwIdz].m_nStatus == EN_MARCH_STATUS__MARCHING)
                        {
                            tbTmpMarch.m_nEtime = pstUser->m_atbMarch[udwIdz].m_nEtime;
                        }
                        break;
                    }
                }
            }

//             if (pstUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_THRONE
//                 && pstUser->m_atbMarch[udwIdx].m_nTid != 0
//                 && pstUser->m_atbMarch[udwIdx].m_nEtime == INT64_MAX)
//             {
//                 //寻找主rally war action
//                 for (TUINT32 udwIdz = 0; udwIdz < pstUser->m_udwMarchNum; udwIdz++)
//                 {
//                     if (pstUser->m_aucMarchFlag[udwIdz] == EN_TABLE_UPDT_FLAG__DEL)
//                     {
//                         continue;
//                     }
//                     if (pstUser->m_atbMarch[udwIdz].m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR_THRONE
//                         &&pstUser->m_atbMarch[udwIdx].m_nTid == pstUser->m_atbMarch[udwIdz].m_nId)
//                     {
//                         if (pstUser->m_atbMarch[udwIdz].m_nStatus == EN_MARCH_STATUS__MARCHING)
//                         {
//                             tbTmpMarch.m_nEtime = pstUser->m_atbMarch[udwIdz].m_nEtime;
//                         }
//                         break;
//                     }
//                 }
//             }

            CCommJson::GenMarchInfoForPush(&tbTmpMarch, rjsonResult["svr_action_list"], pstUser->m_aucMarchFlag[udwIdx]);
            ++udwAIndex;
        }
        else
        {
            if(pstUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR
                && pstUser->m_atbMarch[udwIdx].m_nStatus != EN_MARCH_STATUS__RETURNING)
            {
                CCommJson::GenMarchInfoForPush(&tbTmpMarch, rjsonResult["svr_al_action_list"], pstUser->m_aucMarchFlag[udwIdx]);
                ++udwAlAIndex;
            }
            else if (pstUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR_THRONE
                && pstUser->m_atbMarch[udwIdx].m_nStatus != EN_MARCH_STATUS__RETURNING)
            {
                CCommJson::GenMarchInfoForPush(&tbTmpMarch, rjsonResult["svr_al_action_list"], pstUser->m_aucMarchFlag[udwIdx]);
                ++udwAlAIndex;
            }
            else if (pstUser->m_aucMarchFlag[udwIdx] == EN_TABLE_UPDT_FLAG__CHANGE
                && pstUser->m_atbMarch[udwIdx].m_nStatus != EN_MARCH_STATUS__RETURNING)
            {
                //加速了别人的数据...放到action_list做同步用...
                CCommJson::GenMarchInfoForPush(&tbTmpMarch, rjsonResult["svr_al_action_list"], pstUser->m_aucMarchFlag[udwIdx]);
                ++udwAIndex;
            }
        }
    }

    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwPassiveMarchNum; ++udwIdx)
    {
        if(pstUser->m_atbPassiveMarch[udwIdx].m_nMclass == EN_ACTION_MAIN_CLASS__TIMER)
        {
            continue;
        }

        //wave@20160709: for 增量更新
        if(FALSE == CCommJson::IfTableDataNeedOutput(pstSession->m_dwClientResType, &pstUser->m_atbPassiveMarch[udwIdx], pstUser->m_aucPassiveMarchFlag[udwIdx]))
        {
            continue;
        }

        TbMarch_action tbTmpMarch = pstUser->m_atbPassiveMarch[udwIdx];
        //TUINT32 udwMapType = oWildResJson[CCommonFunc::NumToString(tbTmpMarch.m_bParam[0].m_ddwTargetType)]["a0"]["a0"].asUInt();

        if(pstUser->m_atbPassiveMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR)
        {
            CCommJson::GenMarchInfoForPush(&tbTmpMarch, rjsonResult["svr_al_p_action_list"], pstUser->m_aucPassiveMarchFlag[udwIdx]);
            ++udwAlPIndex;
            if (pstUser->m_atbPassiveMarch[udwIdx].m_nTuid == ptbPlayer->m_nUid && pstUser->m_atbPassiveMarch[udwIdx].m_nTpos == ptbPlayer->m_nCid)
            {
                CCommJson::GenMarchInfoForPush(&tbTmpMarch, rjsonResult["svr_p_action_list"], pstUser->m_aucPassiveMarchFlag[udwIdx]);
                ++udwPIndex;
            }
        }
        else if (pstUser->m_atbPassiveMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR_THRONE)
        {
            tbTmpMarch.m_bRally_def_force[0].ddwTotalNum = pstSession->m_tbThrone.m_nReinforce_troop_num;
            tbTmpMarch.m_bRally_def_force[0].ddwTotalForce = pstSession->m_tbThrone.m_nReinforce_troop_force;
            tbTmpMarch.m_bRally_def_force[0].ddwReinforceNum = pstSession->m_tbThrone.m_nReinforce_troop_num;
            tbTmpMarch.m_bRally_def_force[0].ddwReinforceTroopLimit = CCommonBase::GetGameBasicVal(EN_GAME_BASIC_THRONE_TROOP_NUM_LIMIT);
            tbTmpMarch.m_bRally_def_force[0].ddwReinforceForce = pstSession->m_tbThrone.m_nReinforce_troop_force;
            CCommJson::GenMarchInfoForPush(&tbTmpMarch, rjsonResult["svr_al_p_action_list"], pstUser->m_aucPassiveMarchFlag[udwIdx]);
            ++udwAlPIndex;
        }
        else if (pstUser->m_atbPassiveMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__ATTACK_THRONE)
        {
            CCommJson::GenMarchInfoForPush(&tbTmpMarch, rjsonResult["svr_al_p_action_list"], pstUser->m_aucPassiveMarchFlag[udwIdx]);
            ++udwAlPIndex;
        }
        else
        {
            TBOOL bFind = FALSE;
            if(pstUser->m_atbPassiveMarch[udwIdx].m_nTpos == pstUser->m_stCityInfo.m_stTblData.m_nPos)
            {
                bFind = TRUE;
            }
            for(TUINT32 udwWildIndex = 0; udwWildIndex < pstUser->m_udwWildNum; ++udwWildIndex)
            {
                if(pstUser->m_atbPassiveMarch[udwIdx].m_nTpos == pstUser->m_atbWild[udwWildIndex].m_nId)
                {
                    bFind = TRUE;
                }
            }
            if (tbTmpMarch.m_nSclass == EN_ACTION_SEC_CLASS__CATCH_DRAGON && tbTmpMarch.m_nTuid == pstUser->m_tbPlayer.m_nUid)
            {
                bFind = TRUE;
            }
            if (tbTmpMarch.m_nSclass == EN_ACTION_SEC_CLASS__ATTACK)
            {
                for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
                {
                    TbMarch_action *ptbAction = &pstUser->m_atbMarch[udwIdx];
                    if (ptbAction->m_nSclass == EN_ACTION_SEC_CLASS__OCCUPY && ptbAction->m_nTpos == tbTmpMarch.m_nTpos &&
                        ptbAction->m_nStatus == EN_MARCH_STATUS__LOADING && ptbAction->m_nSuid == ptbPlayer->m_nUid)
                    {
                        bFind = TRUE;
                        break;
                    }
                    else if (ptbAction->m_nSclass == EN_ACTION_SEC_CLASS__CAMP && ptbAction->m_nTpos == tbTmpMarch.m_nTpos &&
                        ptbAction->m_nStatus == EN_MARCH_STATUS__CAMPING_NORMAL && ptbAction->m_nSuid == ptbPlayer->m_nUid)
                    {
                        bFind = TRUE;
                        break;
                    }
                }
            }

            if(!bFind)
            {
                continue;
            }

            CCommJson::GenMarchInfoForPush(&tbTmpMarch, rjsonResult["svr_p_action_list"], pstUser->m_aucPassiveMarchFlag[udwIdx]);
            ++udwPIndex;
        }
    }
}

TVOID CUserJson::GenBagJson(SSession* pstSession, Json::Value& rjsonResult)
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
    for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_stUserInfo.m_tbBackpack.m_bItem.m_udwNum; ++udwIdx)
    {
        //ONLY output when item num > 0
        if(pstSession->m_stUserInfo.m_tbBackpack.m_bItem[udwIdx].m_ddwItemNum > 0)
        {
            jsonSvrBag["item_list"][CCommonFunc::NumToString(pstSession->m_stUserInfo.m_tbBackpack.m_bItem[udwIdx].m_ddwItemId)]
                = pstSession->m_stUserInfo.m_tbBackpack.m_bItem[udwIdx].m_ddwItemNum;
        }
    }
}

TVOID CUserJson::GenTopQuestJson(SSession* pstSession, Json::Value& rjsonResult)
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
    STopQuest& stQuest = pstSession->m_stUserInfo.m_tbUserStat.m_bTop_quest[0];
    jsonSvrQuest["top_quest_list"] = Json::Value(Json::arrayValue);

    for(TUINT32 udwIdx = 0; udwIdx < EN_TOP_QUEST_NUM_LIMIT; ++udwIdx)
    {
        if(BITTEST(stQuest.m_bitQuest, udwIdx))
        {
            jsonSvrQuest["top_quest_list"].append(udwIdx);
        }
    }

    jsonSvrQuest["top_quest_list_to_claim"] = Json::Value(Json::arrayValue);
    for(TUINT32 udwIdx = 0; udwIdx < EN_TOP_QUEST_NUM_LIMIT; ++udwIdx)
    {
        if(BITTEST(stQuest.m_bitQuest, udwIdx))
        {
            continue;
        }
        // 是否是待收集任务
        if(!BITTEST(pstSession->m_stUserInfo.m_tbUserStat.m_bTop_quest_finish[0].m_bitQuest, udwIdx))
        {
            continue;
        }

        jsonSvrQuest["top_quest_list_to_claim"].append(udwIdx);
    }
}

TBOOL CompBlacklist(const TbBlacklist *ptbA, const TbBlacklist *ptbB)
{
    return ptbA->m_nUtime > ptbB->m_nUtime;
}

TVOID CUserJson::GenBlackListJson(SSession* pstSession, Json::Value& rjsonResult)
{
    /*
    "svr_blacklist":
    [
        [
            long, // uid
            long, // unix time
            string, // uname
            int // avatar
        ],
        ...
    ]
    */

    SUserInfo* pstUser = &pstSession->m_stUserInfo;
    Json::Value& jsonSvrBlacklist = rjsonResult["svr_blacklist"];
    jsonSvrBlacklist = Json::Value(Json::arrayValue);

    TbBlacklist *pptbBlacklist[MAX_BLACKLIST_NUM + 1];
    TUINT32 udwTmpBlacklistNum = 0;

    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwBlackListNum; udwIdx++)
    {
        if (pstUser->m_aucBlackListFlag[udwIdx] != EN_TABLE_UPDT_FLAG__DEL)
        {
            pptbBlacklist[udwTmpBlacklistNum++] = &pstUser->m_atbBlackList[udwIdx];
        }
    }

    if (udwTmpBlacklistNum > 1)
    {
        std::sort(pptbBlacklist, pptbBlacklist + udwTmpBlacklistNum, CompBlacklist);
    }

    for (TUINT32 udwIdx = 0; udwIdx < udwTmpBlacklistNum; udwIdx++)
    {
        jsonSvrBlacklist[udwIdx] = Json::Value(Json::arrayValue);
        jsonSvrBlacklist[udwIdx].append(pptbBlacklist[udwIdx]->m_nTarget_uid);
        jsonSvrBlacklist[udwIdx].append(pptbBlacklist[udwIdx]->m_nUtime);
        jsonSvrBlacklist[udwIdx].append(pptbBlacklist[udwIdx]->m_sTarget_uname);
        jsonSvrBlacklist[udwIdx].append(pptbBlacklist[udwIdx]->m_nTarget_avatar);
    }
}

TVOID CUserJson::GenStatJson(SSession* pstSession, Json::Value& rjsonResult)
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

    TbUser_stat* ptbStat = &pstSession->m_stUserInfo.m_tbUserStat;
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();

    jsonSvrStat["has_mail"] = ptbStat->m_nNewest_mailid > ptbStat->m_nReturn_mailid ? 1 : 0;
    jsonSvrStat["has_report"] = ptbStat->m_nNewest_reportid > ptbStat->m_nReturn_reportid ? 1 : 0;
    jsonSvrStat["has_help_bubble"] = udwCurTime > ptbStat->m_nLast_help_bubble_time_out ? EN_HELP_BUBBLE_STATUS_OFF : EN_HELP_BUBBLE_STATUS_ON;
    jsonSvrStat["unread_mail"] = ptbStat->m_nUnread_mail;
    jsonSvrStat["unread_report"] = ptbStat->m_nUnread_report;
    jsonSvrStat["mail_newest_id"] = ptbStat->m_nNewest_mailid;
    jsonSvrStat["report_newest_id"] = ptbStat->m_nNewest_reportid;
    jsonSvrStat["mail_music"] = pstSession->m_bNeedMailMusic;
    jsonSvrStat["recommend_player_num"] = pstSession->m_udwRecommendNum;
    if(pstSession->m_stRecommendTime.ddwTime <= ptbStat->m_nPlayer_recommend_time)
    {
        jsonSvrStat["recommend_player_num"] = 0;
    }
}

TVOID CUserJson::GenDiplomacyJson(SSession* pstSession, Json::Value& rjsonResult)
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
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
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

TVOID CUserJson::GenTipsJson(SSession* pstSession, Json::Value& rjsonResult)
{
    /*
    "svr_tips": 
    [
        [long, int, string] // time, type, content content具体定义见tips content
    ]
    */

    Json::Value& jsonTips = rjsonResult["svr_tips"];
    jsonTips = Json::Value(Json::arrayValue);

    //wave@20160804: 出错时不返回tips
    if(pstSession->m_stCommonResInfo.m_dwRetCode != EN_RET_CODE__SUCCESS)
    {
        return;
    }

    for(TUINT32 udwIdx = 0, udwJsonIndex = 0; udwIdx < pstSession->m_stUserInfo.m_udwTipsNum; ++udwIdx)
    {
        if(pstSession->m_stUserInfo.m_aucTipsFlag[udwIdx] == EN_TABLE_UPDT_FLAG__NEW)
        {
            continue;
        }

        if(pstSession->m_stUserInfo.m_aucTipsFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }

        jsonTips[udwJsonIndex] = Json::Value(Json::arrayValue);
        jsonTips[udwJsonIndex].append(pstSession->m_stUserInfo.m_atbTips[udwIdx].m_nTime);
        jsonTips[udwJsonIndex].append(pstSession->m_stUserInfo.m_atbTips[udwIdx].m_nType);
        jsonTips[udwJsonIndex].append(pstSession->m_stUserInfo.m_atbTips[udwIdx].m_sContent);

        if(pstSession->m_stUserInfo.m_atbTips[udwIdx].m_nType == EN_TIPS_TYPE__COMMON_REWARD
            || pstSession->m_stUserInfo.m_atbTips[udwIdx].m_nType == EN_TIPS_TYPE__MYSTEY_GIFT
            || pstSession->m_stUserInfo.m_atbTips[udwIdx].m_nType == EN_TIPS_TYPE__DRAGON_ATTACK_MONSTER_GET_REWARD
            || pstSession->m_stUserInfo.m_atbTips[udwIdx].m_nType == EN_TIPS_TYPE__TRIAL_GIFT)
        {
            Json::Reader tmpReader;
            tmpReader.parse(pstSession->m_stUserInfo.m_atbTips[udwIdx].m_sContent, jsonTips[udwJsonIndex][2U]);
        }
        ++udwJsonIndex;
    }
}

TVOID CUserJson::GenClientFlag(SSession* pstSession, Json::Value& rjsonResult)
{
    /*
    "svr_client_show_flag":
    [
        int //未上线期间 是否被动移城 0:否 1:是
        int //移城准备时间
    ]
    */

    rjsonResult["svr_client_show_flag"] = Json::Value(Json::arrayValue);
    rjsonResult["svr_client_show_flag"].append(pstSession->m_stUserInfo.m_dwShowRemoveFlag);
    rjsonResult["svr_client_show_flag"].append(pstSession->m_stUserInfo.m_dwTimeForMoveCityPrepare);
}

TVOID CUserJson::GenTimeQuestJson(SSession* pstSession, Json::Value& rjsonResult)
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
    CUserJson::GenQuestNodeJson(&pstSession->m_stUserInfo.m_tbQuest.m_bDaily_quest[0], jsonDailyQuest);

    Json::Value& jsonVipQuest = jsonSvrTimeQuest["vip"];
    jsonVipQuest = Json::Value(Json::objectValue);
    if (pstSession->m_stUserInfo.m_tbPlayer.m_nVip_etime >= CTimeUtils::GetUnixTime())
    {
        CUserJson::GenQuestNodeJson(&pstSession->m_stUserInfo.m_tbQuest.m_bVip_quest[0], jsonVipQuest);
    }

    Json::Value& jsonAllianceQuest = jsonSvrTimeQuest["alliance"];
    jsonAllianceQuest = Json::Value(Json::objectValue);
    if (pstSession->m_stUserInfo.m_tbPlayer.m_nAlid != 0 && pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos != EN_ALLIANCE_POS__REQUEST)
    {
        CUserJson::GenQuestNodeJson(&pstSession->m_stUserInfo.m_tbQuest.m_bAl_quest[0], jsonAllianceQuest);
    }

    TUINT32 udwMisteryGiftEndTime = pstSession->m_stUserInfo.m_tbQuest.m_bTimer_gift[0].m_stQuestCom[0].m_ddwCTime + pstSession->m_stUserInfo.m_tbQuest.m_bTimer_gift[0].m_stQuestCom[0].m_ddwBTime;
    Json::Value& jsonMisteryGift = jsonSvrTimeQuest["mistery_gift"];
    jsonMisteryGift = Json::Value(Json::objectValue);
    jsonMisteryGift["end_time"] = udwMisteryGiftEndTime;

    Json::Value& jsonQuestReward = jsonMisteryGift["reward"];
    jsonQuestReward = Json::Value(Json::arrayValue);
    for (TUINT32 udwRewardIdx = 0; udwRewardIdx < pstSession->m_stUserInfo.m_tbQuest.m_bTimer_gift[0].m_stQuestCom[0].m_stReward.ddwTotalNum; ++udwRewardIdx)
    {
        Json::Value& jsonQuestRewarNode = jsonQuestReward[udwRewardIdx];
        jsonQuestRewarNode = Json::Value(Json::arrayValue);
        jsonQuestRewarNode[0U] = pstSession->m_stUserInfo.m_tbQuest.m_bTimer_gift[0].m_stQuestCom[0].m_stReward.aRewardList[udwRewardIdx].ddwType;
        jsonQuestRewarNode[1U] = pstSession->m_stUserInfo.m_tbQuest.m_bTimer_gift[0].m_stQuestCom[0].m_stReward.aRewardList[udwRewardIdx].ddwId;
        jsonQuestRewarNode[2U] = pstSession->m_stUserInfo.m_tbQuest.m_bTimer_gift[0].m_stQuestCom[0].m_stReward.aRewardList[udwRewardIdx].ddwNum;
    }
}

TVOID CUserJson::GenQuestNodeJson(SQuestNode *pstQuestNode, Json::Value& rjsonResult)
{
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    TINT64 ddwQuestRefreshtime = 0;
    if(pstQuestNode->m_ddwRTime <= udwCurTime)
    {
        TUINT32 udwRefreshTime = CGameInfo::GetInstance()->m_oJsonRoot["game_basic"][EN_GAME_BASIC_QUEST_REFRESH_CYCLE].asUInt();
        TUINT32 udwCount = 1 + (udwCurTime - pstQuestNode->m_ddwRTime)/udwRefreshTime;
        ddwQuestRefreshtime = pstQuestNode->m_ddwRTime + udwCount * udwRefreshTime;
    }
    else
    {
        ddwQuestRefreshtime = pstQuestNode->m_ddwRTime;
    }

    rjsonResult["refresh_time"] = ddwQuestRefreshtime;

    Json::Value& jsonQuestList = rjsonResult["quest_list"];
    jsonQuestList = Json::Value(Json::arrayValue);

    for(TUINT32 udwIdx = 0; udwIdx < pstQuestNode->m_ddwQuestNum; ++udwIdx)
    {
        Json::Value& jsonQuestNode = jsonQuestList[udwIdx];
        jsonQuestNode = Json::Value(Json::objectValue);
        jsonQuestNode["begin_time"] = pstQuestNode->m_stQuestCom[udwIdx].m_ddwBTime;
        jsonQuestNode["cost_time"] = pstQuestNode->m_stQuestCom[udwIdx].m_ddwCTime;
        jsonQuestNode["status"] = pstQuestNode->m_stQuestCom[udwIdx].m_ddwStatus;
        jsonQuestNode["lv"] = pstQuestNode->m_stQuestCom[udwIdx].m_ddwLv;

        //临时屏蔽BUG
        if(pstQuestNode->m_stQuestCom[udwIdx].m_ddwLv == 0)
        {
            jsonQuestNode["lv"] = 1;
            jsonQuestNode["status"] = EN_TIME_QUEST_STATUS_DONE;
        }

        Json::Value& jsonQuestReward = jsonQuestNode["reward"];
        jsonQuestReward = Json::Value(Json::arrayValue);
        for(TUINT32 udwRewardIdx = 0; udwRewardIdx < pstQuestNode->m_stQuestCom[udwIdx].m_stReward.ddwTotalNum && udwRewardIdx < MAX_REWARD_ITEM_NUM; ++udwRewardIdx)
        {
            Json::Value& jsonQuestRewarNode = jsonQuestReward[udwRewardIdx];
            jsonQuestRewarNode = Json::Value(Json::arrayValue);
            jsonQuestRewarNode[0U] = pstQuestNode->m_stQuestCom[udwIdx].m_stReward.aRewardList[udwRewardIdx].ddwType;
            jsonQuestRewarNode[1U] = pstQuestNode->m_stQuestCom[udwIdx].m_stReward.aRewardList[udwRewardIdx].ddwId;
            jsonQuestRewarNode[2U] = pstQuestNode->m_stQuestCom[udwIdx].m_stReward.aRewardList[udwRewardIdx].ddwNum;
        }
    }
}

TVOID CUserJson::GenAlStoreJson(SSession* pstSession, Json::Value& rjsonResult)
{
    /*
    "svr_al_store":
    {
        "marked_item": 
        [
            int //自己标记过的item的id
        ],     
        "store_item": //联盟商店中的商品
        [          
            [int, int, int] //[item id, item num, star num]
        ],
        "purchase_list": //联盟中的玩家的购买记录
        [
            [int, int, int, string, long] //[id. item id, item num, player name, time]
        ]
    }
    */
    TbPlayer* ptbPlayer = &pstSession->m_stUserInfo.m_tbPlayer;
    TbUser_stat* ptbUserStat = &pstSession->m_stUserInfo.m_tbUserStat;
    TbAlliance* ptbAlliance = &pstSession->m_stUserInfo.m_tbAlliance;
    Json::Value& jsonAlStore = rjsonResult["svr_al_store"];
    jsonAlStore = Json::Value(Json::objectValue);

    jsonAlStore["marked_item"] = Json::Value(Json::arrayValue);
    for(TUINT32 udwIdx = 0; udwIdx < ptbUserStat->m_bMark.m_udwNum; ++udwIdx)
    {
        jsonAlStore["marked_item"].append(ptbUserStat->m_bMark[udwIdx].ddwItemId);
    }

    jsonAlStore["store_item"] = Json::Value(Json::arrayValue);
    for(TUINT32 udwIdx = 0, udwJsonIndex = 0; udwIdx < ptbAlliance->m_bAl_store_item.m_udwNum; ++udwIdx)
    {
        if(!(CItemBase::GetItemType(ptbAlliance->m_bAl_store_item[udwIdx].ddwItemId, ptbPlayer->m_nSid) & (1 << EN_ALLIANCE_ITEM_STORE)))
        {
            continue;
        }

        jsonAlStore["store_item"][udwJsonIndex] = Json::Value(Json::arrayValue);
        jsonAlStore["store_item"][udwJsonIndex].append(ptbAlliance->m_bAl_store_item[udwIdx].ddwItemId);
        jsonAlStore["store_item"][udwJsonIndex].append(ptbAlliance->m_bAl_store_item[udwIdx].ddwNum);
        jsonAlStore["store_item"][udwJsonIndex].append(ptbAlliance->m_bAl_store_item[udwIdx].ddwStar);
        jsonAlStore["store_item"][udwJsonIndex].append(0);
        ++udwJsonIndex;
    }

    jsonAlStore["limit_item"] = Json::Value(Json::arrayValue);

    jsonAlStore["purchase_list"] = Json::Value(Json::arrayValue);
    for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_stUserInfo.m_udwAlConsumeNum; ++udwIdx)
    {
        jsonAlStore["purchase_list"][udwIdx] = Json::Value(Json::arrayValue);
        jsonAlStore["purchase_list"][udwIdx].append(pstSession->m_stUserInfo.m_atbAl_store_consume[udwIdx].m_nId);
        jsonAlStore["purchase_list"][udwIdx].append(pstSession->m_stUserInfo.m_atbAl_store_consume[udwIdx].m_nItem_id);
        jsonAlStore["purchase_list"][udwIdx].append(pstSession->m_stUserInfo.m_atbAl_store_consume[udwIdx].m_nItem_num);
        jsonAlStore["purchase_list"][udwIdx].append(pstSession->m_stUserInfo.m_atbAl_store_consume[udwIdx].m_sUname);
        jsonAlStore["purchase_list"][udwIdx].append(pstSession->m_stUserInfo.m_atbAl_store_consume[udwIdx].m_nTime);
    }
    if(pstSession->m_tbTmpAlConsume.m_nAid != 0 && pstSession->m_tbTmpAlConsume.m_nAid == pstSession->m_stReqParam.m_udwAllianceId)
    {
        Json::Value jTmpAlConsume = Json::Value(Json::arrayValue);
        jTmpAlConsume.append(pstSession->m_tbTmpAlConsume.m_nId);
        jTmpAlConsume.append(pstSession->m_tbTmpAlConsume.m_nItem_id);
        jTmpAlConsume.append(pstSession->m_tbTmpAlConsume.m_nItem_num);
        jTmpAlConsume.append(pstSession->m_tbTmpAlConsume.m_sUname);
        jTmpAlConsume.append(pstSession->m_tbTmpAlConsume.m_nTime);

        jsonAlStore["purchase_list"].append(jTmpAlConsume);
    }
}

CUserJson::CUserJson()
{

}

CUserJson::~CUserJson()
{

}

TVOID CUserJson::GenBookMarkListJson(SSession* pstSession, Json::Value& rjsonResult)
{
    /*
    "svr_bookmark_list":
    [
        [int, int, int, string, long] // sid, pos, flag, nickname, unixtime
    ]
    */

    Json::Value& rJsonBookMarkList = rjsonResult["svr_bookmark_list"];
    rJsonBookMarkList = Json::Value(Json::arrayValue);
    for(TUINT32 udwIdx = 0, udwJsonIndex = 0; udwIdx < pstSession->m_stUserInfo.m_udwBookmarkNum; ++udwIdx)
    {
        if(pstSession->m_stUserInfo.m_aucBookMarkFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }
        rJsonBookMarkList[udwJsonIndex] = Json::Value(Json::arrayValue);
        TbBookmark &tbOneBookmark = pstSession->m_stUserInfo.m_atbBookmark[udwIdx];
        rJsonBookMarkList[udwJsonIndex].append(tbOneBookmark.m_nPos >> 32);
        rJsonBookMarkList[udwJsonIndex].append(tbOneBookmark.m_nPos & 0xFFFFFFFF);
        rJsonBookMarkList[udwJsonIndex].append(tbOneBookmark.m_nFlag);
        rJsonBookMarkList[udwJsonIndex].append(tbOneBookmark.m_sNick);
        rJsonBookMarkList[udwJsonIndex].append(tbOneBookmark.m_nTime);
        ++udwJsonIndex;
    }
}

TVOID CUserJson::GenLordSkillJson(SSession* pstSession, Json::Value& rJson)
{
    /*
    "svr_lord_skill":
    {
    "0"(int): int // "skill_id": lv
    }
    */
    SSkill* pstSkill = &pstSession->m_stUserInfo.m_tbUserStat.m_bLord_skill[0];
    Json::Value& rJsonSkill = rJson["svr_lord_skill"];
    CCommJson::GenSkillInfo(pstSkill, rJsonSkill);
}

TVOID CUserJson::GenDragonSkillJson(SSession* pstSession, Json::Value& rJson)
{
    /*
    "svr_dragon_skill":
    {
        "0"(int): int // "skill_id": lv
    }
    */

    SSkill* pstSkill = &pstSession->m_stUserInfo.m_tbUserStat.m_bDragon_skill[0];
    Json::Value& rJsonSkill = rJson["svr_dragon_skill"];
    CCommJson::GenSkillInfo(pstSkill, rJsonSkill);
}

TVOID CUserJson::GenDragonMonsterSkillJson(SSession* pstSession, Json::Value& rJson)
{
    /*
    "svr_dragon_monster_skill":
    {
    "0"(int): int // "skill_id": lv
    }
    */
    SSkill* pstSkill = &pstSession->m_stUserInfo.m_tbUserStat.m_bDragon_monster_skill[0];
    Json::Value& rJsonSkill = rJson["svr_dragon_monster_skill"];
    CCommJson::GenSkillInfo(pstSkill, rJsonSkill);
}

TVOID CUserJson::GenDailyLoginJson(SSession* pstSession, Json::Value& rJson)
{
    /*
    "svr_daily_login":
    {
        "con_login_days": int, //连续登录的天数
        "today_vip_point": int, //今天获得的vip点数
        "tomorrow_vip_point": int// 明天可以获得的vip点数
    }
    */

    Json::Value& rJsonDailyLogin = rJson["svr_daily_login"];
    rJsonDailyLogin = Json::Value(Json::objectValue);
    rJsonDailyLogin["con_login_days"] = pstSession->m_stUserInfo.m_tbUserStat.m_nCon_login_days;
    rJsonDailyLogin["today_vip_point"] = CCommonBase::GetConLoginVipPoint(pstSession->m_stUserInfo.m_tbUserStat.m_nCon_login_days);
    rJsonDailyLogin["tomorrow_vip_point"] = CCommonBase::GetConLoginVipPoint(pstSession->m_stUserInfo.m_tbUserStat.m_nCon_login_days + 1);
}

TVOID CUserJson::GenResearchJson(SSession* pstSession, Json::Value& rJson)
{
    /*"svr_research":
    {
        "0"(int): int // "research_id": lv
    }
    */
    SCommonResearch* pstResearch = &pstSession->m_stUserInfo.m_stCityInfo.m_stTblData.m_bResearch[0];
    Json::Value& rJsonResearch = rJson["svr_research"];
    CCommJson::GenResearchJson(pstResearch, rJsonResearch);
}

TVOID CUserJson::GenBuffJson(SSession* pstSession, Json::Value& rJson)
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
        rJsonBuffer.append(pstSession->m_stUserInfo.m_stPlayerBuffList.m_astPlayerBuffInfo[udwIdx].m_ddwBuffTotal);
    }

    Json::Value& rJsonBufferNew = rJson["svr_buff_without_dragon"];
    rJsonBufferNew = Json::Value(Json::arrayValue);
    for (TUINT32 udwIdx = 0; udwIdx < EN_BUFFER_INFO_USER_JSON_OUTPUT_END; ++udwIdx)
    {
        rJsonBufferNew.append(pstSession->m_stUserInfo.m_stBuffWithoutDragon.m_astPlayerBuffInfo[udwIdx].m_ddwBuffTotal);
    }
}

TVOID CUserJson::GenBackPackJson(SSession* pstSession, Json::Value& rJson)
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

    TINT64 udwBuffEffect = pstSession->m_stUserInfo.m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_CORE_EQUIPMENT_DURATION].m_ddwBuffTotal;
    //equip grid 
    Json::Value &rOutPutEquipGrid = rJson["svr_equip_gride"];
    rOutPutEquipGrid = Json::Value(Json::objectValue);
    rOutPutEquipGrid["limit"] = pstSession->m_stUserInfo.m_tbUserStat.m_nEquip_gride;

    TUINT32 udwCost = 0;
    for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_stUserInfo.m_udwEquipNum;++udwIdx)
    {
        if(pstSession->m_stUserInfo.m_aucEquipFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }
        udwCost++;
    }
    rOutPutEquipGrid["cost"] = udwCost;

    //equip
    Json::Value& rOutPutEquip = rJson["svr_equip"];
    rOutPutEquip = Json::Value(Json::objectValue);

    for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_stUserInfo.m_udwEquipNum;++udwIdx)
    {
        if(pstSession->m_stUserInfo.m_aucEquipFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL || 
            pstSession->m_stUserInfo.m_atbEquip[udwIdx].m_nId == 0)
        {
            continue;
        }
        SEquipMentInfo stEquipInfo;
        stEquipInfo.Reset();
        TUINT64 uddwEquipId = pstSession->m_stUserInfo.m_atbEquip[udwIdx].m_nId;
        CBackpack::GetEquipInfoById(pstSession->m_stUserInfo.m_atbEquip, pstSession->m_stUserInfo.m_udwEquipNum, uddwEquipId, &stEquipInfo);
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
        rOutPutEquipInfo["get_time"] = pstSession->m_stUserInfo.m_atbEquip[udwIdx].m_nGet_time;

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
    rOutPutCrystal = pstSession->m_stUserInfo.m_tbBackpack.m_jCrystal;

    //material
    Json::Value& rOutPutMaterial = rJson["svr_material"];
    rOutPutMaterial = Json::Value(Json::objectValue);
    rOutPutMaterial = pstSession->m_stUserInfo.m_tbBackpack.m_jMaterial;

    //soul
    Json::Value& rOutPutSoul = rJson["svr_soul"];
    rOutPutSoul = Json::Value(Json::objectValue);
    rOutPutSoul = pstSession->m_stUserInfo.m_tbBackpack.m_jSoul;

    //parts
    Json::Value& rOutPutParts = rJson["svr_parts"];
    rOutPutParts = Json::Value(Json::objectValue);
    rOutPutParts = pstSession->m_stUserInfo.m_tbBackpack.m_jParts;

    //sp_crystal
    Json::Value& rOutPuSptCrystal = rJson["svr_sp_crystal"];
    rOutPuSptCrystal = Json::Value(Json::objectValue);
    rOutPuSptCrystal = pstSession->m_stUserInfo.m_tbBackpack.m_jSp_crystal;

    //scroll old v1.0
    Json::Value& rOutPutScroll = rJson["svr_scrolls"];
    rOutPutScroll = Json::Value(Json::objectValue);
    rOutPutScroll = pstSession->m_stUserInfo.m_tbBackpack.m_jScroll;

    //scroll new v1.1
    Json::Value& rOutPutScrollNew = rJson["svr_scrolls_new"];
    rOutPutScrollNew = Json::Value(Json::objectValue);
    Json::Value::Members members = pstSession->m_stUserInfo.m_tbBackpack.m_jScroll.getMemberNames();
    for (Json::Value::Members::iterator it = members.begin(); it != members.end(); it++)
    {
        rOutPutScrollNew[*it] = Json::Value(Json::objectValue);
        rOutPutScrollNew[*it]["num"] = pstSession->m_stUserInfo.m_tbBackpack.m_jScroll[*it];
        if (pstSession->m_stUserInfo.m_tbBackpack.m_jScroll_get_time.isMember(*it))
        {
            rOutPutScrollNew[*it]["get_time"] = pstSession->m_stUserInfo.m_tbBackpack.m_jScroll_get_time[*it];
        }
        else
        {
            rOutPutScrollNew[*it]["get_time"] = 0;
        }
    }
}

TVOID CUserJson::GenRewardWindowJson(SSession* pstSession, Json::Value& rJson)
{
    /*
    "svr_reward_window":{
		"type"://弹窗类型 ERewardWinbdowType
		"get_type"://从何种渠道获得 ERewardWindowGetType
		"reward":
		[
			[
				int, //type
                int, //id
                int  //num
			]
		]
	}
    */
    Json::Value& rJsonRewardResult = rJson["svr_reward_window"];
    rJsonRewardResult = Json::Value(Json::objectValue);
    rJsonRewardResult["type"] = pstSession->m_stUserInfo.udwRewardWinType;
    rJsonRewardResult["chest_id"] = pstSession->m_stUserInfo.m_udwLotteryChestItemId; //TODO 到底有啥用
    rJsonRewardResult["chest_num"] = pstSession->m_stUserInfo.m_udwLotteryChestItemNum; //TODO 到底有啥用
    rJsonRewardResult["get_type"] = pstSession->m_stUserInfo.udwRewardWinGetType;
    Json::Value& rJsonRewardDetail = rJsonRewardResult["reward"];
    rJsonRewardDetail = Json::Value(Json::arrayValue);

    for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_stUserInfo.m_stRewardWindow.udwTotalNum; ++udwIdx)
    {
        rJsonRewardDetail[udwIdx].append(pstSession->m_stUserInfo.m_stRewardWindow.aRewardList[udwIdx].udwType);
        rJsonRewardDetail[udwIdx].append(pstSession->m_stUserInfo.m_stRewardWindow.aRewardList[udwIdx].udwId);
        rJsonRewardDetail[udwIdx].append(pstSession->m_stUserInfo.m_stRewardWindow.aRewardList[udwIdx].udwNum);

    }
}

TVOID CUserJson::GenTitleJson(SSession* pstSession, Json::Value& rjsonResult)
{
    Json::Value& jsonSvrTitle = rjsonResult["svr_title_new"];
    CCommJson::GenTitleInfo(&pstSession->m_stTitleInfoList, &pstSession->m_tbThrone, jsonSvrTitle);
}

TVOID CUserJson::GenPrisonJson(SSession* pstSession, Json::Value& rJson)
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
    SUserInfo *pstUser = &pstSession->m_stUserInfo;

    rJson = Json::Value(Json::arrayValue);
    Json::Value jsonOne = Json::Value(Json::objectValue);
    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwPassiveMarchNum; ++udwIdx)
    {
        if(pstUser->m_aucPassiveMarchFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }

        if(pstUser->m_atbPassiveMarch[udwIdx].m_nSclass != EN_ACTION_SEC_CLASS__PRISON_TIMER)
        {
            continue;
        }

        if(pstUser->m_atbPassiveMarch[udwIdx].m_mFlag.find(TbMARCH_ACTION_FIELD_TAL) != pstUser->m_atbPassiveMarch[udwIdx].m_mFlag.end()
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
        if(pstUser->m_atbPassiveMarch[udwIdx].m_bPrison_param[0].ddwEscortActionId > 0)
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

TVOID CUserJson::GenSvrListJson(SSession* pstSession, Json::Value& rJson)
{
    /*
    "svr_list": 
    {
        "5010": 	//	中心位置
        [
            0, 				// svr id
            "english", 		// language
            1384128000, 	// create time
            0, 				// server status: SERVER_STATUS
            1, 				// 是否是当前svr
            1, 				// 是否有账号
            0, 				// 是否保护中
            5010, 			// 中心位置
            "al_solar", 	// alliance
            "als", 			// al nick nam
            "solar", 		// king name
            101, 			// svr map type: SERVER_MAP_TYPE
            2, 				// block size
            1 				// alliance flag id
        ],
    }
    */

    if(pstSession->m_udwTmpSvrNum != pstSession->m_udwTmpSvrStatNum || pstSession->m_udwTmpSvrNum != pstSession->m_udwTmpThroneMapNum)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("wrong data number [svr_stat_num=%u] [throne_map_num=%u] [svr_num=%u] [seq=%u]",
            pstSession->m_udwTmpSvrStatNum, pstSession->m_udwTmpThroneMapNum, pstSession->m_udwTmpSvrNum, pstSession->m_stUserInfo.m_udwBSeqNo));
        return;
    }

    Json::Value& jsonSvrList = rJson["svr_list"];
    jsonSvrList = Json::Value(Json::objectValue);

    for(TUINT32 udwSvrIdx = 0; udwSvrIdx < pstSession->m_udwTmpSvrNum; ++udwSvrIdx)
    {
        TUINT32 udwSvrPos = pstSession->m_atbTmpSvrStat[udwSvrIdx].m_nPos;
        TUINT32 dwThroneMapIdx = 0;
        for(dwThroneMapIdx = 0; dwThroneMapIdx < pstSession->m_udwTmpSvrNum; dwThroneMapIdx++)
        {
            if(pstSession->m_atbTmpSvrStat[udwSvrIdx].m_nSid == pstSession->m_atbTmpThroneMap[dwThroneMapIdx].m_nSid)
            {
                break;
            }
        }
        if(dwThroneMapIdx == pstSession->m_udwTmpSvrNum)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("OUTPUT:can't find corresponding throne map [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
        }

        SGameSvrInfo *pstSvrInfo = CGameSvrInfo::GetInstance()->GetSidInfo(pstSession->m_atbTmpSvrStat[udwSvrIdx].m_nSid);
        if (pstSession->m_atbTmpSvrStat[udwSvrIdx].m_nShow_flag == 0 || pstSvrInfo->m_dwMergeStatus == EN_SVR_MERGE_STATUS__DONE) //当show_flag=0时，不在这个svr当中的人是看不到这个svr的
        {
            if(pstSession->m_stUserInfo.m_tbLogin.m_nSid != pstSession->m_atbTmpSvrStat[udwSvrIdx].m_nSid)
            {
                continue;
            }
        }
        GenSvrJson(pstSession, jsonSvrList[CCommonFunc::NumToString(udwSvrPos)], udwSvrIdx, dwThroneMapIdx);
    }
    return;
}

TVOID CUserJson::GenSvrJson(SSession* pstSession, Json::Value& rJson, TUINT32 udwSvrIdx, TUINT32 udwThroneMapIdx)
{
    rJson = Json::Value(Json::arrayValue);
    TUINT32 udwLoginSvrFlag[MAX_GAME_SVR_NUM] = {0};
    TINT32 dwSvrFlag = 0;

    udwLoginSvrFlag[pstSession->m_stUserInfo.m_tbLogin.m_nSid] = TRUE;

    //修正map中的alliance name信息
    const Json::Value &jAlMapping = CAllianceMapping::GetInstance()->m_oJsonRoot["alliance_mapping"];
    TbMap *pstMap = &pstSession->m_atbTmpThroneMap[udwThroneMapIdx];
    if(pstMap->m_nAlid > 0 && jAlMapping["update_time"].asInt64() > pstMap->m_nName_update_time)
    {
        if(jAlMapping.isMember(CCommonFunc::NumToString(pstMap->m_nAlid)))
        {
            //这里不要改AWS表里的信息，因为alliance_mapping的内容可能滞后
            pstMap->Set_Alname(jAlMapping[CCommonFunc::NumToString(pstMap->m_nAlid)]["al_name"].asString());
            pstMap->Set_Al_nick(jAlMapping[CCommonFunc::NumToString(pstMap->m_nAlid)]["nick_name"].asString());
            pstMap->Set_Al_flag(jAlMapping[CCommonFunc::NumToString(pstMap->m_nAlid)]["avatar"].asInt64());
            pstMap->Set_Uname(jAlMapping[CCommonFunc::NumToString(pstMap->m_nAlid)]["oname"].asString());
        }
    }

    rJson[0U] = pstSession->m_atbTmpSvrStat[udwSvrIdx].m_nSid;
    rJson[1U] = pstSession->m_atbTmpSvrStat[udwSvrIdx].m_sLanguage;
    rJson[2U] = pstSession->m_atbTmpSvrStat[udwSvrIdx].m_nOpen_time;
    rJson[3U] = pstSession->m_atbTmpSvrStat[udwSvrIdx].m_nStatus;
    rJson[4U] = pstSession->m_stUserInfo.m_tbLogin.m_nSid == pstSession->m_atbTmpSvrStat[udwSvrIdx].m_nSid ? 1U : 0U;
    rJson[5U] = udwLoginSvrFlag[pstSession->m_atbTmpSvrStat[udwSvrIdx].m_nSid] ? 1U : 0U;

    dwSvrFlag = pstSession->m_atbTmpSvrStat[udwSvrIdx].m_nSwitch;
    if(udwLoginSvrFlag[pstSession->m_atbTmpSvrStat[udwSvrIdx].m_nSid])
    {
        dwSvrFlag = 1U;
    }
    rJson[6U] = dwSvrFlag == 0 ? 1 : 0;
    rJson[7U] = pstSession->m_atbTmpSvrStat[udwSvrIdx].m_nPos;
    rJson[8U] = pstSession->m_atbTmpThroneMap[udwThroneMapIdx].m_sAlname;
    rJson[9U] = pstSession->m_atbTmpThroneMap[udwThroneMapIdx].m_sAl_nick;
    rJson[10U] = pstSession->m_atbTmpThroneMap[udwThroneMapIdx].m_sUname;
    rJson[11U] = 101U;
    rJson[12U] = pstSession->m_atbTmpSvrStat[udwSvrIdx].m_nBlock_size;
    rJson[13U] = pstSession->m_atbTmpThroneMap[udwThroneMapIdx].m_nAl_flag;
    rJson[14U] = pstSession->m_atbTmpSvrStat[udwSvrIdx].m_nAvatar;

    return;
}

TVOID CUserJson::GenBountyInfo(SSession* pstSession, Json::Value& rJson)
{
    /*
    "svr_bounty"：
    {
        "next_refresh_time": int//下次更新限时任务的时间
        "goal" ://总体奖励信息
        {
            "get_star":	int //获得的总星数
                "s_stage" : []	//总体奖励星星分布 阶段信息
                "reward" ://总体进度奖励信息
                [//阶段
                    [
                        [
                            type //类型
                            id   //id
                            num  //数量
                        ]
                    ]
                ]
        }
        "base"://基础节点信息
        {
            "idx":[//小节点 
                [int, int, int, int]//目标值当前数量 总目标值 完成阶段 条件开始值
                int //目标项type
                    int //目标项id
                    int //目标值 value
                    int //0-拥有型 1-操作型
                    [//idx 为小节点阶段
                        int //目标值 数量
                    ]
                [//idx 为小节点阶段 reward
                    [[
                        int,//type
                            int,//id
                            int,//num
                    ]]
                ]
                int //displayer order
            ]
        }
        "finish_node":[		//小节点奖励信息
            [
                int,	//node idx 节点idx
                int,	//stage Idx 第几阶段
            ]
        ]
        "finish_bounty": [int]//总体进度完成奖励信息
    */
    TbBounty *pstBounty = &pstSession->m_stUserInfo.m_tbBounty;
    Json::Value& jsonBounty = rJson["svr_bounty"];
    jsonBounty = Json::Value(Json::objectValue);

    if(pstBounty->m_nNext_refresh_time < CTimeUtils::GetUnixTime())
    {
        jsonBounty["has_bounty"] = 0;
    }
    else
    {
        jsonBounty["has_bounty"] = 1;
    }
    
    jsonBounty["next_refresh_time"] = pstBounty->m_nNext_refresh_time;
    
    Json::Value &jsonBountyGoal = jsonBounty["goal"];
    jsonBountyGoal = Json::Value(Json::objectValue);
    jsonBountyGoal["get_star"] = pstBounty->m_nStar;
    jsonBountyGoal["s_stage"] = pstBounty->m_jS_stage;
    jsonBountyGoal["reward"] = pstBounty->m_jS_reward;

    Json::Value &jsonBountyBase = jsonBounty["base"];
    jsonBountyBase = Json::Value(Json::objectValue);
    jsonBountyBase = pstBounty->m_jBase;

    Json::Value &jsonBountyFinishNode = jsonBounty["finish_node"];
    jsonBountyFinishNode = Json::Value(Json::arrayValue);
    for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_stUserInfo.m_vTmpFinishBountyBase.size();++udwIdx)
    {
        Json::Value &jIdxNodeReward = jsonBountyFinishNode[udwIdx];
        jIdxNodeReward = Json::Value(Json::arrayValue);
        jIdxNodeReward.append(pstSession->m_stUserInfo.m_vTmpFinishBountyBase[udwIdx].first);
        jIdxNodeReward.append(pstSession->m_stUserInfo.m_vTmpFinishBountyBase[udwIdx].second);
    }

    Json::Value &jsonBountyFinish = jsonBounty["finish_bounty"];
    jsonBountyFinish = Json::Value(Json::arrayValue);
    if(pstSession->m_stUserInfo.m_udwTmpFinishBountyGoal != 0)
    {
        jsonBountyFinish.append(pstSession->m_stUserInfo.m_udwTmpFinishBountyGoal);
    }
}

TVOID CUserJson::GenBroadcastInfo(SSession *pstSession, Json::Value& rJson)
{
    /*
    "svr_broadcast_new":
    {
		"broadcast_list":
		[
			[
				int 	//broadcast category (广播类型：0 初级 1：初级 2：高级 )
				long	//unique upt_time(us)
				int 	//content_id
				["aa","bb"...]	//label param array
                ["cc","dd"...]  //action param array
			]
		]
	}
    */

    const Json::Value& jBroadcastInfo = CGameInfo::GetInstance()->m_oJsonRoot["game_broadcast"];

    Json::Value& jBroadcast = rJson["svr_broadcast_new"];
    jBroadcast = Json::Value(Json::objectValue);
    Json::Value& jsonBroadcastList = jBroadcast["broadcast_list"];
    jsonBroadcastList = Json::Value(Json::arrayValue);
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TUINT64 uddwCurTimeUs = CTimeUtils::GetCurTimeUs();

    ostringstream oss;
    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwBroadcastNum; udwIdx++)
    {
        Json::Value jBroadcastItem = Json::Value(Json::arrayValue);
        TbBroadcast& tbBroadcast = pstUser->m_atbBroadcast[udwIdx];
        if(pstUser->m_aucBroadcastFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }
        //自己发出的广播自己还是要看到的...排重由CCommonHandleAfter::UpdateBroadcastStat来保证
//         if(pstUser->m_aucBroadcastFlag[udwIdx] == EN_TABLE_UPDT_FLAG__NEW)
//         {
//             continue;
//         }

        if(tbBroadcast.m_nStrategy == EN_BROADCAST_STRATEGY__REAL_TIME)
        {
            oss.str("");
            oss << tbBroadcast.m_nCid;
            TUINT64 uddwDuration = jBroadcastInfo[oss.str()]["duration"].asUInt64();
            if (uddwDuration <= 60 && pstSession->m_stReqParam.m_ucLoginStatus == EN_LOGIN_STATUS__LOGIN)
            {
                continue;
            }
            if (tbBroadcast.m_nCtime + uddwDuration * 1000000 < uddwCurTimeUs)
            {
                continue;
            }
            if (jBroadcastInfo[oss.str()]["hold"].asInt() == 1)
            {
                TBOOL bIsFind = FALSE;
                for (TUINT32 udwIdy = udwIdx + 1; udwIdy < pstUser->m_udwBroadcastNum; udwIdy++)
                {
                    if (pstUser->m_aucBroadcastFlag[udwIdy] == EN_TABLE_UPDT_FLAG__DEL)
                    {
                        continue;
                    }
//                     if (pstUser->m_aucBroadcastFlag[udwIdy] == EN_TABLE_UPDT_FLAG__NEW)
//                     {
//                         continue;
//                     }
                    if (pstUser->m_atbBroadcast[udwIdy].m_nCid == tbBroadcast.m_nCid)
                    {
                        bIsFind = TRUE;
                        break;
                    }
                }
                if (bIsFind)
                {
                    continue;
                }
            }
            jBroadcastItem.append(jBroadcastInfo[oss.str()]["category"].asInt());
            jBroadcastItem.append(tbBroadcast.m_nCtime);
            jBroadcastItem.append(tbBroadcast.m_nCid);
            //replace array
            Json::Value jReplaceArray = Json::Value(Json::arrayValue);
            if(!tbBroadcast.m_sReplace_data.empty())
            {
                vector<string> vecReplaceList;
                CCommonFunc::GetVectorFromString(tbBroadcast.m_sReplace_data.c_str(), '#', vecReplaceList);
                for(TUINT32 udwIdy = 0; udwIdy < vecReplaceList.size(); udwIdy++)
                {
                    jReplaceArray.append(vecReplaceList[udwIdy]);
                }
            }
            jBroadcastItem.append(jReplaceArray);
            //param array
            Json::Value jParamArray = Json::Value(Json::arrayValue);
            if(!tbBroadcast.m_sParam.empty())
            {
                vector<string> vecParamList;
                CCommonFunc::GetVectorFromString(tbBroadcast.m_sParam.c_str(), '#', vecParamList);
                for(TUINT32 udwIdy = 0; udwIdy < vecParamList.size(); udwIdy++)
                {
                    jParamArray.append(vecParamList[udwIdy]);
                }
            }
            jBroadcastItem.append(jParamArray);

            jsonBroadcastList.append(jBroadcastItem);
        }
    }
}

TVOID CUserJson::GenKnightInfo(SSession *pstSession, Json::Value& rJson)
{
    /*
    "svr_knight_list":
    [
        [int, int, int, int, long] //[knight_id, 状态, 任职位置, exp, 相关actionid]
    ]
    */
    SCityInfo *pstCity = &pstSession->m_stUserInfo.m_stCityInfo;

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

TVOID CUserJson::GenEventInfo(SSession *pstSession, Json::Value& rJson)
{
    if (pstSession->m_bEventInfoOk)
    {
        Json::Value& oJsonEventInfo = rJson["svr_event_info"];

        oJsonEventInfo = Json::Value(Json::objectValue);
        oJsonEventInfo["info"] = pstSession->m_sEventInfo;
    }
}

TVOID CUserJson::GenRewardWindowNewInfo(SSession *pstSession, Json::Value& rJson)
{
    Json::Value& oJsonRewardWindow = rJson["svr_reward_window_new"];
    oJsonRewardWindow = Json::Value(Json::arrayValue);

    for (TINT32 dwIdx = 0, dwIdy = 0; dwIdx < pstSession->m_stUserInfo.m_dwRewardWindowNum; dwIdx++)
    {
        if (pstSession->m_stUserInfo.m_aucRewardWindowFlag[dwIdx] == EN_TABLE_UPDT_FLAG__UNCHANGE)
        {
            oJsonRewardWindow[dwIdy] = Json::Value(Json::objectValue);
            oJsonRewardWindow[dwIdy]["id"] = pstSession->m_stUserInfo.m_atbRewardWindow[dwIdx].m_nId;
            oJsonRewardWindow[dwIdy]["type"] = pstSession->m_stUserInfo.m_atbRewardWindow[dwIdx].m_nType;
            oJsonRewardWindow[dwIdy]["get_type"] = pstSession->m_stUserInfo.m_atbRewardWindow[dwIdx].m_nGet_type;

            if (pstSession->m_stUserInfo.m_atbRewardWindow[dwIdx].m_jReward.empty())
            {
                oJsonRewardWindow[dwIdy]["reward"] = Json::Value(Json::arrayValue);
            }
            else
            {
                oJsonRewardWindow[dwIdy]["reward"] = pstSession->m_stUserInfo.m_atbRewardWindow[dwIdx].m_jReward;
            }

            if (pstSession->m_stUserInfo.m_atbRewardWindow[dwIdx].m_jInfo.empty())
            {
                oJsonRewardWindow[dwIdy]["info"] = Json::Value(Json::objectValue);
            }
            else
            {
                oJsonRewardWindow[dwIdy]["info"] = pstSession->m_stUserInfo.m_atbRewardWindow[dwIdx].m_jInfo;
            }
            dwIdy++;
        }
    }
}

TVOID CUserJson::GenRandomRewardInfo(SSession* pstSession, Json::Value& rJson)
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

TVOID CUserJson::GenMonsterInfo(SSession *pstSession, Json::Value& rJson)
{
    Json::Value& jsonMonster = rJson["svr_monster_info"];
    jsonMonster = Json::Value(Json::objectValue);

    Json::Value& jMonsterHit = jsonMonster["monster_hit"];
    jMonsterHit = Json::Value(Json::arrayValue);
    Json::Value& jLeaderMonsterGen = jsonMonster["leader_monster_gen"];
    jLeaderMonsterGen = Json::Value(Json::arrayValue);
    Json::Value& jLeaderMonsterKill = jsonMonster["leader_monster_kill"];
    jLeaderMonsterKill = Json::Value(Json::arrayValue);

    for (TUINT32 udwIdx = 1; udwIdx <= MAX_MONSTER_LV; udwIdx++)
    {
        jMonsterHit.append(pstSession->m_stUserInfo.m_tbPlayer.m_bMonster_hit[0].addwNum[udwIdx]);
        jLeaderMonsterGen.append(pstSession->m_stUserInfo.m_tbPlayer.m_bLeader_monster_gen[0].addwNum[udwIdx]);
        jLeaderMonsterKill.append(pstSession->m_stUserInfo.m_tbPlayer.m_bLeader_monster_kill[0].addwNum[udwIdx]);
    }
}

TVOID CUserJson::GenTrialInfo(SSession *pstSession, Json::Value& rJson)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbPlayer* ptbPlayer = &pstUser->m_tbPlayer;

    Json::Value& jTrial = rJson["svr_trial"];
    jTrial = Json::Value(Json::objectValue);

    Json::Value& jTrialOpen = jTrial["open"];
    jTrialOpen = Json::Value(Json::objectValue);
    TUINT32 udwOpen = 0;
    TUINT32 udwBtime = 0;
    TUINT32 udwEtime = 0;
    CCommonBase::FindTrialOpenTime(pstUser, CTimeUtils::GetUnixTime(), udwOpen, udwBtime, udwEtime);
    jTrialOpen["open_flag"] = udwOpen;
    jTrialOpen["btime"] = udwBtime;
    jTrialOpen["etime"] = udwEtime;

    jTrial["init"] = ptbPlayer->m_nTrial_init;
    jTrial["dragon_shard"] = ptbPlayer->m_nDragon_shard;

    Json::Value& jTrialMonster = jTrial["monster"];
    jTrialMonster = Json::Value(Json::objectValue);
    jTrialMonster["rage_open"] = ptbPlayer->m_nTrial_rage_open;
    jTrialMonster["rage_mode"] = ptbPlayer->m_nTrial_rage_mode;
    jTrialMonster["normal"] = Json::Value(Json::objectValue);
    jTrialMonster["normal"]["atk_time"] = ptbPlayer->m_bTrial_monster_normal[0].udwAtkTime;
    jTrialMonster["normal"]["reward"] = Json::Value(Json::arrayValue);
    for (TUINT32 udwIdx = 0; udwIdx < MAX_TRIAL_ATK_TIME; ++udwIdx)
    {
        Json::Value jReward = Json::Value(Json::arrayValue);
        jReward[0U] = ptbPlayer->m_bTrial_monster_normal[0].aRewardList[udwIdx].ddwType;
        jReward[1U] = ptbPlayer->m_bTrial_monster_normal[0].aRewardList[udwIdx].ddwId;
        jReward[2U] = ptbPlayer->m_bTrial_monster_normal[0].aRewardList[udwIdx].ddwNum;
        jTrialMonster["normal"]["reward"][udwIdx] = jReward;
    }
    jTrialMonster["rage"] = Json::Value(Json::objectValue);
    jTrialMonster["rage"]["atk_time"] = ptbPlayer->m_bTrial_monster_rage[0].udwAtkTime;
    jTrialMonster["rage"]["reward"] = Json::Value(Json::arrayValue);
    for (TUINT32 udwIdx = 0; udwIdx < MAX_TRIAL_ATK_TIME; ++udwIdx)
    {
        Json::Value jReward = Json::Value(Json::arrayValue);
        jReward[0U] = ptbPlayer->m_bTrial_monster_rage[0].aRewardList[udwIdx].ddwType;
        jReward[1U] = ptbPlayer->m_bTrial_monster_rage[0].aRewardList[udwIdx].ddwId;
        jReward[2U] = ptbPlayer->m_bTrial_monster_rage[0].aRewardList[udwIdx].ddwNum;
        jTrialMonster["rage"]["reward"][udwIdx] = jReward;
    }

    Json::Value& jTrialBag = jTrial["lucky_bag"];
    jTrialBag = Json::Value(Json::objectValue);
    jTrialBag["normal"] = Json::Value(Json::objectValue);
    jTrialBag["normal"]["reward"] = Json::Value(Json::arrayValue);
    for (TUINT32 udwIdx = 0; udwIdx < MAX_TRIAL_LUCKY_BAG_NORMAL_NUM; ++udwIdx)
    {
        Json::Value jReward = Json::Value(Json::arrayValue);
        jReward[0U] = ptbPlayer->m_bTrial_lucky_bag_normal[0].aRewardList[udwIdx].ddwType;
        jReward[1U] = ptbPlayer->m_bTrial_lucky_bag_normal[0].aRewardList[udwIdx].ddwId;
        jReward[2U] = ptbPlayer->m_bTrial_lucky_bag_normal[0].aRewardList[udwIdx].ddwNum;
        jTrialBag["normal"]["reward"][udwIdx] = jReward;
    }
    jTrialBag["rage"] = Json::Value(Json::objectValue);
    jTrialBag["rage"]["reward"] = Json::Value(Json::arrayValue);
    for (TUINT32 udwIdx = 0; udwIdx < MAX_TRIAL_LUCKY_BAG_RAGE_NUM; ++udwIdx)
    {
        Json::Value jReward = Json::Value(Json::arrayValue);
        jReward[0U] = ptbPlayer->m_bTrial_lucky_bag_rage[0].aRewardList[udwIdx].ddwType;
        jReward[1U] = ptbPlayer->m_bTrial_lucky_bag_rage[0].aRewardList[udwIdx].ddwId;
        jReward[2U] = ptbPlayer->m_bTrial_lucky_bag_rage[0].aRewardList[udwIdx].ddwNum;
        jTrialBag["rage"]["reward"][udwIdx] = jReward;
    }
    jTrialBag["rage"]["time"] = ptbPlayer->m_bTrial_lucky_bag_rage[0].udwOpenNum;

    Json::Value& jTrialGift = jTrial["trial_gift"];
    jTrialGift = Json::Value(Json::objectValue);
    TUINT32 udwHasGift = 0;
    TUINT32 udwEndTime = 0;
    SOneGlobalRes sReward;
    sReward.Reset();
    CCommonBase::GetTrialGift(ptbPlayer, CTimeUtils::GetUnixTime(), udwHasGift, udwEndTime, sReward);
    jTrialGift["has_gift"] = udwHasGift;
    jTrialGift["end_time"] = udwEndTime;
    jTrialGift["reward"] = Json::Value(Json::arrayValue);
    jTrialGift["reward"][0U] = sReward.ddwType;
    jTrialGift["reward"][1U] = sReward.ddwId;
    jTrialGift["reward"][2U] = sReward.ddwNum;
}

TVOID CUserJson::GenComputeResInfo( SSession *pstSession, Json::Value& rJson )
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;

    Json::Value& jJson = rJson["svr_compute_res"];
    jJson = Json::Value(Json::objectValue);

    if(pstUser->m_uddwNewActionId)
    {
        jJson["new_action_id"] = pstUser->m_uddwNewActionId;
    }
}

TVOID CUserJson::GenGemRechargeInfo(SSession *pstSession, Json::Value& rJson)
{
    if (strcmp(pstSession->m_stReqParam.m_szCommand, "gem_recharge") != 0
        && strcmp(pstSession->m_stReqParam.m_szCommand, "operate_gem_recharge") != 0)
    {
        return;
    }

    Json::Value& jGemRecharge = rJson["svr_gem_recharge"];
    jGemRecharge = Json::Value(Json::objectValue);

    TUINT32 udwRechargeGem = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TINT64 ddwProjectId = strtoll(pstSession->m_stReqParam.m_szKey[5], NULL, 10);

    jGemRecharge["fake_gem_recharge"] = pstSession->m_ucFakeRecharge;
    jGemRecharge["platform"] = pstSession->m_stReqParam.m_szPlatForm;
    jGemRecharge["sandbox"] = pstSession->m_stReqParam.m_ucIsSandBox;
    jGemRecharge["recharge_num"] = udwRechargeGem;
    jGemRecharge["project_id"] = ddwProjectId;
}

TVOID CUserJson::GenIdolInfo(SSession *pstSession, Json::Value& rJson)
{
    Json::Value& jIdolInfo = rJson["svr_idol_info"];
    jIdolInfo = Json::Value(Json::arrayValue);
    for (TUINT32 udwIdx = 0, udwIdy = 0; udwIdx < pstSession->m_udwIdolNum; udwIdx++)
    {
        if (pstSession->m_ucIdolFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }
        jIdolInfo[udwIdy]["id"] = pstSession->m_atbIdol[udwIdx].m_nId;
        jIdolInfo[udwIdy]["pos"] = pstSession->m_atbIdol[udwIdx].m_nPos;
        jIdolInfo[udwIdy]["alid"] = pstSession->m_atbIdol[udwIdx].m_nAlid == 0 ? -1 : pstSession->m_atbIdol[udwIdx].m_nAlid;
        jIdolInfo[udwIdy]["buff_total_time"] = pstSession->m_atbIdol[udwIdx].m_nLast_time;
        jIdolInfo[udwIdy]["buff_end_time"] = pstSession->m_atbIdol[udwIdx].m_nEnd_time;
        jIdolInfo[udwIdy]["buff"] = Json::Value(Json::arrayValue);

        if (pstSession->m_atbIdol[udwIdx].m_nStatus == EN_IDOL_STATUS__BUFF_PERIOD 
            && pstSession->m_atbIdol[udwIdx].m_jInfo.isObject() 
            && pstSession->m_atbIdol[udwIdx].m_jInfo.isMember("b"))
        {
            Json::Value::Members members = pstSession->m_atbIdol[udwIdx].m_jInfo["b"].getMemberNames();
            TUINT32 udwBuffId = 0;
            TINT32 dwBuffNum = 0;
            TUINT32 udwBuffIdx = 0;
            for (Json::Value::Members::iterator it = members.begin(); it != members.end(); it++)
            {
                udwBuffId = pstSession->m_atbIdol[udwIdx].m_jInfo["b"][*it][0U].asUInt();
                dwBuffNum = pstSession->m_atbIdol[udwIdx].m_jInfo["b"][*it][1U].asUInt();
                jIdolInfo[udwIdy]["buff"][udwBuffIdx] = Json::Value(Json::arrayValue);
                jIdolInfo[udwIdy]["buff"][udwBuffIdx][0U] = udwBuffId;
                jIdolInfo[udwIdy]["buff"][udwBuffIdx][1U] = dwBuffNum;
                udwBuffIdx++;
            }
        }
        
        udwIdy++;
    }
}

TVOID CUserJson::GenThroneInfo(SSession *pstSession, Json::Value& rJson)
{
    TbThrone *ptbThrone = &pstSession->m_tbThrone;

    Json::Value& jThroneInfo = rJson["svr_throne_info_new"];
    jThroneInfo = Json::Value(Json::objectValue);

    jThroneInfo["pos"] = ptbThrone->m_nPos;
    jThroneInfo["alid"] = ptbThrone->m_nAlid == 0 ? -1 : ptbThrone->m_nAlid;
    jThroneInfo["buff"] = Json::Value(Json::arrayValue);

    const Json::Value& jThroneBuff = ptbThrone->m_jInfo["buff"];
    Json::Value::Members members = jThroneBuff.getMemberNames();
    TUINT32 udwBuffId = 0;
    TINT32 dwBuffNum = 0;
    TUINT32 udwBuffIdx = 0;
    for (Json::Value::Members::iterator it = members.begin(); it != members.end(); it++)
    {
        udwBuffId = jThroneBuff[*it][0U].asUInt();
        dwBuffNum = jThroneBuff[*it][1U].asInt();
        jThroneInfo["buff"][udwBuffIdx] = Json::Value(Json::arrayValue);
        jThroneInfo["buff"][udwBuffIdx][0U] = udwBuffId;
        jThroneInfo["buff"][udwBuffIdx][1U] = dwBuffNum;
        udwBuffIdx++;
    }

    jThroneInfo["reinforce_limit"] = CCommonBase::GetGameBasicVal(EN_GAME_BASIC_RALLY_SLOT_LIMIT);
    jThroneInfo["reinforce_num"] = ptbThrone->m_nReinforce_num;
    jThroneInfo["reinforce_troop_limit"] = CCommonBase::GetGameBasicVal(EN_GAME_BASIC_THRONE_TROOP_NUM_LIMIT);
    jThroneInfo["reinforce_troop_num"] = ptbThrone->m_nReinforce_troop_num;
}