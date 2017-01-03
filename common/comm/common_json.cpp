#include "common_json.h"
#include "common_func.h"
#include "time_utils.h"
#include "player_base.h"
#include "common_base.h"
#include "tool_base.h"
#include "alliance_mapping.h"
#include "game_svr.h"
#include "map_base.h"
#include "wild_info.h"
#include "buffer_base.h"
#include "city_base.h"
#include "common_logic.h"
#include "quest_logic.h"
#include "action_base.h"
#include "backpack_logic.h"
#include "service_key.h"
#include "output_conf.h"
#include "map_logic.h"

TVOID CCommJson::GenResearchJson(SCommonResearch *pstResearch, Json::Value& rJson)
{
    rJson = Json::Value(Json::objectValue);
    for(TUINT32 udwIdx = 0; udwIdx < EN_RESEARCH_TYPE__END; ++udwIdx)
    {
        if (pstResearch->m_addwLevel[udwIdx] != 0)
        {
            rJson[CCommonFunc::NumToString(udwIdx)] = pstResearch->m_addwLevel[udwIdx];
        }
    }
}

TVOID CCommJson::GenBuildingJson(SCityBuildingNode* astBuildingNode, TUINT32 udwNodeNum, Json::Value& rJson)
{
    rJson = Json::Value(Json::objectValue);
    for(TUINT32 udwIdx = 0; udwIdx < udwNodeNum; ++udwIdx)
    {
        if(astBuildingNode[udwIdx].m_ddwLevel == 0)
        {
            continue;
        }
        string strKey = CCommonFunc::NumToString(astBuildingNode[udwIdx].m_ddwPos);
        rJson[strKey] = Json::Value(Json::arrayValue);
        rJson[strKey].append(astBuildingNode[udwIdx].m_ddwType);
        rJson[strKey].append(astBuildingNode[udwIdx].m_ddwLevel);
    }
}

TVOID CCommJson::GenReportBuildingJson(SCityBuildingNode* astBuildingNode, TUINT32 udwNodeNum, Json::Value& rJson)
{
    rJson = Json::Value(Json::objectValue);
    for(TUINT32 udwIdx = 0; udwIdx < udwNodeNum; ++udwIdx)
    {
        if(astBuildingNode[udwIdx].m_ddwLevel == 0)
        {
            continue;
        }
        if(CToolBase::IsObstacle(astBuildingNode[udwIdx].m_ddwType))
        {
            continue;
        }
        string strKey = CCommonFunc::NumToString(astBuildingNode[udwIdx].m_ddwPos);
        rJson[strKey] = Json::Value(Json::arrayValue);
        rJson[strKey].append(astBuildingNode[udwIdx].m_ddwType);
        rJson[strKey].append(astBuildingNode[udwIdx].m_ddwLevel);
    }
}

TVOID CCommJson::GenNameInfo(TbReport* ptbReport, Json::Value& rJson)
{
    rJson["name_info"] = Json::Value(Json::arrayValue);
    rJson["name_info"].append(ptbReport->m_bFrom[0].m_szUserName);
    rJson["name_info"].append(ptbReport->m_bFrom[0].m_szOwnedCityName);
    rJson["name_info"].append(ptbReport->m_bFrom[0].m_szAllianceName);
    rJson["name_info"].append(ptbReport->m_bFrom[0].m_szSvrName);
    rJson["name_info"].append(ptbReport->m_bTo[0].m_szUserName);
    rJson["name_info"].append(ptbReport->m_bTo[0].m_szOwnedCityName);
    rJson["name_info"].append(ptbReport->m_bTo[0].m_szAllianceName);
    rJson["name_info"].append(ptbReport->m_bTo[0].m_szSvrName);
}

TVOID CCommJson::GenAllianceInfo(TbAlliance* ptbAlliance, Json::Value& rJson)
{
    //        {
    //            "sid": 1,
    //            "oid" : 126,
    //            "oname" : "Player-3I",
    //            "aid" : 17,
    //            "alname" : "Erry-Test-2",
    //            "force" : 555555,    //force
    //            "force_kill" : 555555,    //force_kill
    //            "member" : 56,       //数量
    //            "desc" : "temp",     //联盟描述
    //            "lang" : 0,          //语言
    //            "policy" : 1,        //加入政策
    //            "gift_point" : 0     //联盟礼物等级
    //            "al_nick": string    //联盟简称
    //            "al_star": int       //联盟星级
    //        }

    rJson = Json::Value(Json::objectValue);
    rJson["sid"] = ptbAlliance->m_nSid;
    rJson["oid"] = ptbAlliance->m_nOid;
    rJson["oname"] = ptbAlliance->m_sOname;
    rJson["aid"] = ptbAlliance->m_nAid;
    rJson["alname"] = ptbAlliance->m_sName;
    rJson["force"] = ptbAlliance->m_nMight;
    rJson["force_kill"] = ptbAlliance->m_nForce_kill > 0 ? ptbAlliance->m_nForce_kill : 0;
    rJson["member"] = ptbAlliance->m_nMember;
    rJson["desc"] = ptbAlliance->m_sDesc;
    rJson["lang"] = ptbAlliance->m_nLanguage;
    rJson["policy"] = ptbAlliance->m_nPolicy / ALLIANCE_POLICY_OFFSET;
    rJson["gift_point"] = CCommonBase::GetAlGiftPoint(ptbAlliance->m_nGift_point);
    rJson["flag"] = ptbAlliance->m_nAvatar;
    rJson["al_nick"] = ptbAlliance->m_sAl_nick_name;
    rJson["al_star"] = ptbAlliance->m_nAl_star;
    rJson["force_rank"] = 0;
    rJson["troop_kill_rank"] = 0;
}

TVOID CCommJson::GenAllianceInfo(AllianceRank* pAlRank, TbAlliance* ptbAlliance, Json::Value& rJson)
{
    //        {
    //            "sid": 1,
    //            "oid" : 126,
    //            "oname" : "Player-3I",
    //            "aid" : 17,
    //            "alname" : "Erry-Test-2",
    //            "force" : 555555,    //force
    //            "force_kill" : 555555,    //force_kill
    //            "member" : 56,       //数量
    //            "desc" : "temp",     //联盟描述
    //            "lang" : 0,          //语言
    //            "policy" : 1,        //加入政策
    //            "gift_point" : 0     //联盟礼物等级
    //            "al_nick": string    //联盟简称
    //            "al_star": int       //联盟星级
    //        }

    rJson = Json::Value(Json::objectValue);
    rJson["sid"] = ptbAlliance->m_nSid;
    rJson["oid"] = ptbAlliance->m_nOid;
    rJson["oname"] = ptbAlliance->m_sOname;
    rJson["aid"] = ptbAlliance->m_nAid;
    rJson["alname"] = ptbAlliance->m_sName;
    rJson["force"] = ptbAlliance->m_nMight;
    rJson["force_kill"] = ptbAlliance->m_nForce_kill > 0 ? ptbAlliance->m_nForce_kill : 0;
    rJson["member"] = ptbAlliance->m_nMember;
    rJson["desc"] = ptbAlliance->m_sDesc;
    rJson["lang"] = ptbAlliance->m_nLanguage;
    rJson["policy"] = ptbAlliance->m_nPolicy / ALLIANCE_POLICY_OFFSET;
    rJson["gift_point"] = CCommonBase::GetAlGiftPoint(ptbAlliance->m_nGift_point);
    rJson["flag"] = ptbAlliance->m_nAvatar;
    rJson["al_nick"] = ptbAlliance->m_sAl_nick_name;
    rJson["al_star"] = ptbAlliance->m_nAl_star;
    rJson["force_rank"] = pAlRank->audwRank[2];
    rJson["troop_kill_rank"] = pAlRank->audwRank[1];
}

TVOID CCommJson::GenAlGiftJson(SUserInfo *pstUser, Json::Value& rJson)
{
    const Json::Value& oAlGift = CGameInfo::GetInstance()->m_oJsonRoot["game_al_gift_new"]["pack"];
    TINT64 ddwCurTime = CTimeUtils::GetCurTimeUs();
    SAlGiftList* pstAlGifts = &pstUser->m_stAlGifts;
    TINT32 dwCanOpenNum = 0;
    rJson["al_gift_list"] = Json::Value(Json::arrayValue);
    TUINT32 udwCount = 0;
    TINT32 dwStatus = 0;
    set<TINT64> setClearAlGift;
    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_tbClearAlGift.m_bClear_al_gift.m_udwNum; udwIdx++)
    {
        setClearAlGift.insert(pstUser->m_tbClearAlGift.m_bClear_al_gift[udwIdx]);
    }
    for(TINT32 dwIdx = 0; dwIdx < pstAlGifts->m_dwGiftNum && dwIdx < MAX_AL_IAP_GIFT_NUM; ++dwIdx)
    {
        TbAl_gift* ptbAlGift = &((*pstAlGifts)[dwIdx]);
        TbAl_gift_reward *ptbAlGiftReward = NULL;

        if (setClearAlGift.count(ptbAlGift->m_nId))
        {
            continue;
        }

        for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwAlGiftRewardNum; udwIdx++)
        {
            if(pstUser->m_atbAlGiftReward[udwIdx].m_nGid == ptbAlGift->m_nId)
            {
                ptbAlGiftReward = &pstUser->m_atbAlGiftReward[udwIdx];
                break;
            }
        }
        if(NULL == ptbAlGiftReward)
        {
            dwStatus = EN_AL_GIFT_STATUS_NORMAL;
            if((ptbAlGift->m_nCtime + AL_IAP_GIFT_EXPIRE_TIME) > ddwCurTime)
            {
                dwCanOpenNum++;
            }
        }
        else
        {
            dwStatus = ptbAlGiftReward->m_nStatus;
            if(EN_AL_GIFT_STATUS_NORMAL == ptbAlGiftReward->m_nStatus && (ptbAlGift->m_nCtime + AL_IAP_GIFT_EXPIRE_TIME) > ddwCurTime)
            {
                dwCanOpenNum++;
            }
        }
        if(EN_AL_GIFT_STATUS_CLEARED != dwStatus)
        {
            rJson["al_gift_list"][udwCount] = Json::Value(Json::arrayValue);
            rJson["al_gift_list"][udwCount][0U] = (ptbAlGift->m_nId);
            rJson["al_gift_list"][udwCount][1U] = (ptbAlGift->m_nPack_id);
            rJson["al_gift_list"][udwCount][2U] = ((ptbAlGift->m_nCtime + AL_IAP_GIFT_EXPIRE_TIME)/1000000);
            rJson["al_gift_list"][udwCount][3U] = (dwStatus);
            rJson["al_gift_list"][udwCount][4U] = (ptbAlGift->m_nGift_point);
            rJson["al_gift_list"][udwCount][5U] = Json::Value(Json::arrayValue);
            if(NULL == ptbAlGiftReward || 0 == ptbAlGiftReward->m_bReward.m_udwNum)
            {
                rJson["al_gift_list"][udwCount][5U][0] = Json::Value(Json::arrayValue);
                rJson["al_gift_list"][udwCount][5U][0].append(0);
                rJson["al_gift_list"][udwCount][5U][0].append(oAlGift[CCommonFunc::NumToString(ptbAlGift->m_nPack_id)]["icon_id"].asInt());
                rJson["al_gift_list"][udwCount][5U][0].append(0);
            }
            else
            {
                for (TUINT32 udwIdy = 0; udwIdy < ptbAlGiftReward->m_bReward.m_udwNum; ++udwIdy)
                {
                    rJson["al_gift_list"][udwCount][5U][udwIdy] = Json::Value(Json::arrayValue);
                    rJson["al_gift_list"][udwCount][5U][udwIdy].append(ptbAlGiftReward->m_bReward[udwIdy].ddwType);
                    rJson["al_gift_list"][udwCount][5U][udwIdy].append(ptbAlGiftReward->m_bReward[udwIdy].ddwId);
                    rJson["al_gift_list"][udwCount][5U][udwIdy].append(ptbAlGiftReward->m_bReward[udwIdy].ddwNum);
                }
            }
            udwCount++;
        }
    }
    rJson["al_gift_can_open_num"] = dwCanOpenNum;
}

TVOID CCommJson::GenStatJson(TINT64 ddwTime, TUINT32 udwNum, TBOOL bNeedMusic, TbUser_stat* ptbStat, Json::Value& rJson)  //和CUserJson::GenStatJson一样
{
    //{
    //    "has_mail": int,                  //是否有新邮件，0表示没有，1表示有
    //    "has_report" : int,               //是否有新report，0表示没有，1表示有
    //    "unread_mail" : int,              //未读邮件数量
    //    "unread_report" : int,            //未读report数量
    //    "bookmark" : int,                 //bookmarknum
    //    "mail_newest_id" : long,          //该用户最新的mailid
    //    "report_newest_id" : long,         //该用户最新的mailid
    //    "recommend_player_num": int       //推荐邀请玩家人数
    //},
    rJson["has_mail"] = ptbStat->m_nNewest_mailid > ptbStat->m_nReturn_mailid? 1 : 0;
    rJson["has_report"] = ptbStat->m_nNewest_reportid > ptbStat->m_nReturn_reportid ? 1 : 0;
    rJson["unread_mail"] = ptbStat->m_nUnread_mail;
    rJson["unread_report"] = ptbStat->m_nUnread_report;
    rJson["mail_newest_id"] = ptbStat->m_nNewest_mailid;
    rJson["report_newest_id"] = ptbStat->m_nNewest_reportid;
    rJson["mail_music"] = bNeedMusic;
    rJson["recommend_player_num"] = udwNum;
    if(ddwTime <= ptbStat->m_nPlayer_recommend_time)
    {
        rJson["recommend_player_num"] = 0;
    }
}

TVOID CCommJson::GenResourceJson(SCommonResource* pstRes, Json::Value& rjson)
{
    rjson = Json::Value(Json::arrayValue);
    for(TUINT32 udwIdx = 0; udwIdx < EN_RESOURCE_TYPE__END; ++udwIdx)
    {
        rjson.append(pstRes->m_addwNum[udwIdx]);
    }
}

TVOID CCommJson::GenTroopJson(SCommonTroop* pstTroop, Json::Value& rjson)
{
    rjson = Json::Value(Json::arrayValue);
    for(TUINT32 udwIdx = 0; udwIdx < EN_TROOP_TYPE__END; ++udwIdx)
    {
        rjson.append(pstTroop->m_addwNum[udwIdx]);
    }
}

TVOID CCommJson::GenFortJson(SCommonFort* pstFort, Json::Value& rjson)
{
    rjson = Json::Value(Json::arrayValue);
    for(TUINT32 udwIdx = 0; udwIdx < EN_FORT_TYPE__END; ++udwIdx)
    {
        rjson.append(pstFort->m_addwNum[udwIdx]);
    }
}

TVOID CCommJson::GenPlayerProfileJson(TbPlayer* ptbPlayer, TbEquip *atbEquip, TINT32 dwEquipNum, Json::Value& rJson, SPlayerBuffInfo *pstBuff)
{
    /*
    "svr_player": 
    {
        "sid": int,
        "uid": int,
        "uname": string,
        "alname": string,
        "al_nick_name": string
        "aid": int,
        "alpos": int,
        "ctime": long,
        "utime": long,
        "status": int, // ECityStatus, status & EN_CITY_STATUS__XXX != 0 表示存在该状态
        "exp": long,   //lord 经验
        "level": int,    //lord 等级
        "force": long,
        "force_got": long,
        "force_kill": long,
        "req_al": int,// 请求加入的aid
        "al_stat":[
            1383145033, // loyal time
            0,          // loyal interval
            37950,      // loyal cur
            37950,      // loyal all
            0,          // loyal cost
            0,          // share res
            2448        // share might
        ],
        "vip_point": int,// vip 点数
        "vip_level": int  // vip level
        "vip_etime": long,  // vip 失效时间
        "avatar": int,    // lord形象
        "city_pos": int,
        "age": int      //表示时代
        "title": [int]  //玩家拥有的称号id列表
        "key": // profile
        {
            "0":  //war_data
            [
                int//战争胜利次数
                int//战败次数
                int//进攻战胜次数
                int//进攻战败次数
                int//防守战胜次数
                int//防守战败次数
                int//胜率
                int//侦查玩家次数
                int//杀兵数
                int//杀城防数
                int//损兵数
                int//损失城防数
                int//杀兵和损兵比率
                int//自己伤兵数
                int//敌人伤兵数
                int//自己城市破坏次数
                int//破坏他人城市次数
                int//消灭敌人战力
            ],
            "1":  //strength
            [
                int//兵种战力值
                int//城防战力值
                int//建筑实力
                int//科研实力
                int//龙实力
                int//领主实力
            ],
            "2":  //dragon attribute
            [
                int //抓捕龙数
                int //处决龙数
                int //逃跑龙数
                int //龙被俘次数
                int //龙被处决次数
                int //龙逃跑次数
                int //击杀怪物次数
            ],
            "3":  //resource
            [
                int//赠送粮食
                int//赠送黄金
                int//赠送木头
                int//赠送石头
                int//赠送石矿
            ],
            "4":  //other
            [
                int//完成联盟帮助次数
                int//采集资源量
            ]
        },
        "doubloon": int //紫金币数量
        "can_invite": int//0-不可邀请 1-可邀请
        "has_dragon": int//0:没有 1:有
        "dragon":
        {
            "name": string
            "lv": int
            "exp": long
            "avatar": int 
            "energy": int
            "next_recovery_time": long  //下次恢复时间
            "recovery_count": int       //已经恢复次数
            "status": int 
            "action_id": long
            "capture_info": //借助action输出, 后面可以会移到别处, todo
            {
                "uid": long,
                "al_nick": string,
                "uname": string,
                "join_time": int,
                "can_kill_time": int,
                "auto_release_time": int,
                "pos": int,
                "can_kill": int //0表示可以被杀
                "sid": int, //抓捕者sid
            },
            "equipment":
            [
                ...
            ]
        }
        "reinforce":
        [
            int, //march num
            int, //march limit
            int, //troop num
            int, //troop limit
        ]
    }
    */
    TINT64 udwBuffEffect = 0;
    if(pstBuff == NULL)
    {
        udwBuffEffect = 0;
    }
    else
    {
        udwBuffEffect = pstBuff->m_astPlayerBuffInfo[EN_BUFFER_INFO_CORE_EQUIPMENT_DURATION].m_ddwBuffTotal;
    }
    rJson = Json::Value(Json::objectValue);
    rJson["sid"] = ptbPlayer->m_nSid;
    rJson["uid"] = ptbPlayer->m_nUid;
    rJson["uname"] = ptbPlayer->m_sUin;
    rJson["alname"] = ptbPlayer->m_sAlname;
    rJson["al_nick_name"] = ptbPlayer->m_sAl_nick_name;

    TINT32 dwTrueAid = 0;
    TINT32 dwTrueAlPos = 0;
    TINT32 dwTrueReqAl = 0;
    if(ptbPlayer->m_nAlpos == EN_ALLIANCE_POS__REQUEST)
    {
        dwTrueAid = 0;
        dwTrueReqAl = ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET;
        dwTrueAlPos = EN_ALLIANCE_POS__REQUEST;
    }
    else
    {
        dwTrueAid = ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET;
        dwTrueReqAl = 0;
        dwTrueAlPos = ptbPlayer->m_nAlpos;
    }
    rJson["aid"] = dwTrueAid;
    rJson["alpos"] = dwTrueAlPos;
    rJson["req_al"] = dwTrueReqAl;

    //修正alliance name信息
    const Json::Value &jAlMapping = CAllianceMapping::GetInstance()->m_oJsonRoot["alliance_mapping"];
    if (dwTrueAid > 0 && jAlMapping["update_time"].asInt64() > ptbPlayer->m_nAlname_update_time)
    {
        if (jAlMapping.isMember(CCommonFunc::NumToString(dwTrueAid)))
        {
            rJson["alname"] = jAlMapping[CCommonFunc::NumToString(dwTrueAid)]["al_name"];
            rJson["al_nick_name"] = jAlMapping[CCommonFunc::NumToString(dwTrueAid)]["nick_name"];
        }
    }

    rJson["ctime"] = ptbPlayer->m_nCtime;
    rJson["utime"] = ptbPlayer->m_nUtime;
    rJson["status"] = ptbPlayer->m_nStatus;

    rJson["exp"] = ptbPlayer->m_nExp; //lord exp
    rJson["level"] = ptbPlayer->m_nLevel; //lord lv

    rJson["force"] = ptbPlayer->m_nMight;
    rJson["force_got"] = ptbPlayer->m_nMgain;
    rJson["force_kill"] = ptbPlayer->m_bWar_statistics[0].ddwForceKilled;

    rJson["al_stat"] = Json::Value(Json::arrayValue);
    rJson["al_stat"].append(ptbPlayer->m_nLoy_time);
    rJson["al_stat"].append(ptbPlayer->m_nLoy_itv);
    rJson["al_stat"].append(ptbPlayer->m_nLoy_cur);
    rJson["al_stat"].append(ptbPlayer->m_nLoy_all);
    rJson["al_stat"].append(ptbPlayer->m_nLoy_all - ptbPlayer->m_nLoy_cur);

    rJson["vip_point"] = ptbPlayer->m_nVip_point;
    rJson["vip_level"] = CPlayerBase::GetRawVipLevel(ptbPlayer, ptbPlayer->m_nVip_point);
    rJson["vip_etime"] = ptbPlayer->m_nVip_etime;
    rJson["vip_stage"] = ptbPlayer->m_nVip_stage;

    rJson["avatar"] = ptbPlayer->m_nAvatar;

    rJson["city_pos"] = ptbPlayer->m_nCid;
    rJson["age"] = ptbPlayer->m_nAge;

    rJson["title"] = Json::Value(Json::arrayValue);

    rJson["doubloon"] = ptbPlayer->m_nDoubloon;

    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    if (udwCurTime / 86400 == ptbPlayer->m_nInvited_time / 86400 && ptbPlayer->m_nInvited_num >= 3)
    {
        rJson["can_invite"] = 0;
    }
    else
    {
        rJson["can_invite"] = 1;
    }

    rJson["evil_troop_kill"] = ptbPlayer->m_nEvil_troop_kill;
    rJson["evil_force_kill"] = ptbPlayer->m_nEvil_force_kill;

    //各种统计数据
    rJson["key"] = Json::Value(Json::objectValue);
    Json::Value& jWarData = rJson["key"]["0"];
    jWarData = Json::Value(Json::arrayValue);
    Json::Value& jStrength = rJson["key"]["1"];
    jStrength = Json::Value(Json::arrayValue);
    Json::Value& jHeroAttribute = rJson["key"]["2"];
    jHeroAttribute = Json::Value(Json::arrayValue);
    Json::Value& jResource = rJson["key"]["3"];
    jResource = Json::Value(Json::arrayValue);
    Json::Value& jOther = rJson["key"]["4"];
    jOther = Json::Value(Json::arrayValue);

    //war statistics
    TUINT32 udwFail = ptbPlayer->m_bWar_statistics[0].ddwAtkFail + ptbPlayer->m_bWar_statistics[0].ddwDfnFail;
    TUINT32 udwWin = ptbPlayer->m_bWar_statistics[0].ddwAtkWin + ptbPlayer->m_bWar_statistics[0].ddwDfnWin;
    jWarData.append(udwWin);
    jWarData.append(udwFail);
    jWarData.append(ptbPlayer->m_bWar_statistics[0].ddwAtkWin);
    jWarData.append(ptbPlayer->m_bWar_statistics[0].ddwAtkFail);
    jWarData.append(ptbPlayer->m_bWar_statistics[0].ddwDfnWin);
    jWarData.append(ptbPlayer->m_bWar_statistics[0].ddwDfnFail);
    jWarData.append((udwWin + udwFail) ? (10000L * udwWin / (udwWin + udwFail)) : 0);
    jWarData.append(ptbPlayer->m_bWar_statistics[0].ddwScoutNum);
    jWarData.append(ptbPlayer->m_bWar_statistics[0].ddwDamageTroopNum);
    jWarData.append(ptbPlayer->m_bWar_statistics[0].ddwDamageFortNum);
    jWarData.append(ptbPlayer->m_bWar_statistics[0].ddwMyTroopDamagedNum);
    jWarData.append(ptbPlayer->m_bWar_statistics[0].ddwMyFortDamagedNum);
    jWarData.append(ptbPlayer->m_bWar_statistics[0].ddwMyTroopDamagedNum ? (10000 *(ptbPlayer->m_bWar_statistics[0].ddwDamageTroopNum) / ptbPlayer->m_bWar_statistics[0].ddwMyTroopDamagedNum) : 10000);
    jWarData.append(ptbPlayer->m_bWar_statistics[0].ddwHosTroopNum);
    jWarData.append(ptbPlayer->m_bWar_statistics[0].ddwHurtEnemyTroopNum);
    jWarData.append(ptbPlayer->m_bWar_statistics[0].ddwMyCityDestroyedNum);
    jWarData.append(ptbPlayer->m_bWar_statistics[0].ddwDestroyCityNum);
    jWarData.append(ptbPlayer->m_bWar_statistics[0].ddwForceKilled);

    //strength
    jStrength.append(ptbPlayer->m_nCur_troop_might);
    jStrength.append(ptbPlayer->m_nCur_fort_might);
    jStrength.append(ptbPlayer->m_nBuilding_force);
    jStrength.append(ptbPlayer->m_nResearch_force);
    jStrength.append(ptbPlayer->m_nDragon_force);
    jStrength.append(ptbPlayer->m_nLord_force);

    //dragon statistics
    const SDragonStatistics &sDragonStat = ptbPlayer->m_bDragon_statistics[0];
    jHeroAttribute.append(sDragonStat.ddwCaptureDragonNum);
    jHeroAttribute.append(sDragonStat.ddwExecuteDragonNum);
    jHeroAttribute.append(sDragonStat.ddwEscapeDragonNum);
    jHeroAttribute.append(sDragonStat.ddwMyDragonCapturedNum);
    jHeroAttribute.append(sDragonStat.ddwMyDragonExecutedNum);
    jHeroAttribute.append(sDragonStat.ddwMyDragonEscapedNum);
    jHeroAttribute.append(ptbPlayer->m_bWar_statistics[0].ddwKillMonsterNum);

    //resource
    const SCommonResource &tbResource = ptbPlayer->m_bTransport_resource[0];
    jResource.append(tbResource.m_addwNum[EN_RESOURCE_TYPE__FOOD]);
    jResource.append(tbResource.m_addwNum[EN_RESOURCE_TYPE__GOLD]);
    jResource.append(tbResource.m_addwNum[EN_RESOURCE_TYPE__WOOD]);
    jResource.append(tbResource.m_addwNum[EN_RESOURCE_TYPE__STONE]);
    jResource.append(tbResource.m_addwNum[EN_RESOURCE_TYPE__ORE]);

    //other
    const SCommonResource &tbGainResource = ptbPlayer->m_bGain_resource[0];
    TUINT32 udwTotalGainResource = 0;
    for(TINT32 dwIdx = 0; dwIdx < EN_RESOURCE_TYPE__END; dwIdx++)
    {
        udwTotalGainResource += tbGainResource.m_addwNum[dwIdx];
    }
    jOther.append(ptbPlayer->m_nSend_al_help_num);
    jOther.append(udwTotalGainResource);


    //Dragon
    rJson["has_dragon"] = ptbPlayer->m_nHas_dragon > 0 ? 1 : 0;
    rJson["svr_has_dragon"] = ptbPlayer->m_nDragon_max_lv > 0 ? 1 : 0;
    rJson["dragon"] = Json::Value(Json::objectValue);
    rJson["dragon"]["name"] = ptbPlayer->m_sDragon_name;
    rJson["dragon"]["lv"] = ptbPlayer->m_nDragon_level;
    rJson["dragon"]["exp"] = ptbPlayer->m_nDragon_exp;
    rJson["dragon"]["avatar"] = ptbPlayer->m_nDragon_avatar;
    rJson["dragon"]["energy"] = ptbPlayer->m_nDragon_cur_energy;
    rJson["dragon"]["next_recovery_time"] = ptbPlayer->m_nDragon_recovery_time;
    rJson["dragon"]["recovery_count"] = ptbPlayer->m_nDragon_recovery_count;
    rJson["dragon"]["status"] = ptbPlayer->m_nDragon_status;
    rJson["dragon"]["action_id"] = ptbPlayer->m_nDragon_tid;

    // reinforce limit
    rJson["reinforce"][0U] = ptbPlayer->m_bReinforce_limit[0].udwMarchNum;
    rJson["reinforce"][1U] = ptbPlayer->m_bReinforce_limit[0].udwMarchLimit;
    rJson["reinforce"][2U] = ptbPlayer->m_bReinforce_limit[0].udwTroopNum;
    rJson["reinforce"][3U] = ptbPlayer->m_bReinforce_limit[0].udwTroopLimit;

    Json::Value& rJsonEquipList = rJson["dragon"]["equip"];
    if (ptbPlayer->m_jDragon_equip.isObject())
    {
        rJsonEquipList = ptbPlayer->m_jDragon_equip;
    }
    else
    {
        rJsonEquipList = Json::Value(Json::objectValue);
    }
    /*
    rJsonEquipList = Json::Value(Json::objectValue);

    SEquipMentInfo stEquip;
    for (TINT32 dwIdx = 0; dwIdx < dwEquipNum; dwIdx++)
    {
        if (atbEquip[dwIdx].m_nPut_on_pos == 0 || atbEquip[dwIdx].m_nStatus != EN_EQUIPMENT_STATUS_ON_DRAGON)
        {
            continue;
        }
        stEquip.Reset();
        CBackpack::GetEquipInfoById(atbEquip, dwEquipNum, atbEquip[dwIdx].m_nId, &stEquip);

        Json::Value &JsonPosJson = rJsonEquipList[CCommonFunc::NumToString(atbEquip[dwIdx].m_nPut_on_pos)];
        JsonPosJson = Json::Value(Json::objectValue);

        JsonPosJson["e_id"] = atbEquip[dwIdx].m_nId;
        JsonPosJson["id"] = stEquip.stBaseInfo.udwEType;
        JsonPosJson["lv"] = stEquip.stBaseInfo.udwLv;

        TFLOAT64 ffRate = 0.0001;
        TUINT32 udwBuffEffectTime = stEquip.stBaseInfo.udwEffectTime * (1.0 + ffRate * udwBuffEffect);

        if (stEquip.stBaseInfo.udwCategory == 1 && stEquip.stStatusInfo.udwEquipmentPutOnTime)
        {
            JsonPosJson["end_time"] = stEquip.stStatusInfo.udwEquipmentPutOnTime + udwBuffEffectTime;
        }
        else
        {
            JsonPosJson["end_time"] = 0;
        }

        for (TUINT32 udwIdx = 0; udwIdx < MAX_CRYSTAL_IN_EQUIPMENT; ++udwIdx)
        {
            JsonPosJson["crystal"].append(stEquip.stStatusInfo.audwSlot[udwIdx]);
        }

        Json::Value &EquipBuf = JsonPosJson["buff"];
        EquipBuf = Json::Value(Json::objectValue);
        for (TUINT32 udwIdx = 0; udwIdx < stEquip.stStatusInfo.udwBufferNum; ++udwIdx)
        {
            EquipBuf[CCommonFunc::NumToString(stEquip.stStatusInfo.astBuffInfo[udwIdx].m_udwId)].append(stEquip.stStatusInfo.astBuffInfo[udwIdx].m_udwId);
            EquipBuf[CCommonFunc::NumToString(stEquip.stStatusInfo.astBuffInfo[udwIdx].m_udwId)].append(stEquip.stStatusInfo.astBuffInfo[udwIdx].m_dwNum);
            EquipBuf[CCommonFunc::NumToString(stEquip.stStatusInfo.astBuffInfo[udwIdx].m_udwId)].append(stEquip.stStatusInfo.astBuffInfo[udwIdx].m_udwType);
        }
        Json::Value &EquipMistoryBuf = JsonPosJson["mistery_buff"];
        EquipMistoryBuf = Json::Value(Json::objectValue);
        for (TUINT32 udwIdx = 0; udwIdx < stEquip.stStatusInfo.udwMistoryBufferNum; ++udwIdx)
        {
            EquipMistoryBuf[CCommonFunc::NumToString(stEquip.stStatusInfo.astMisteryBuffInfo[udwIdx].m_udwId)].append(stEquip.stStatusInfo.astMisteryBuffInfo[udwIdx].m_udwId);
            EquipMistoryBuf[CCommonFunc::NumToString(stEquip.stStatusInfo.astMisteryBuffInfo[udwIdx].m_udwId)].append(stEquip.stStatusInfo.astMisteryBuffInfo[udwIdx].m_dwNum);
            EquipMistoryBuf[CCommonFunc::NumToString(stEquip.stStatusInfo.astMisteryBuffInfo[udwIdx].m_udwId)].append(stEquip.stStatusInfo.astMisteryBuffInfo[udwIdx].m_udwType);
        }
    }
    */
}

TVOID CCommJson::GenAlMemberInfo(TbPlayer* ptbPlayer, Json::Value& rJson)
{
    rJson = Json::Value(Json::objectValue);
    rJson["sid"] = ptbPlayer->m_nSid;
    rJson["uid"] = ptbPlayer->m_nUid;
    rJson["uname"] = ptbPlayer->m_sUin;
    rJson["avatar"] = ptbPlayer->m_nAvatar;
    rJson["level"] = ptbPlayer->m_nLevel;

    TINT32 dwTrueAid = 0;
    TINT32 dwTrueAlPos = 0;
    TINT32 dwTrueReqAl = 0;
    if(ptbPlayer->m_nAlpos == EN_ALLIANCE_POS__REQUEST)
    {
        dwTrueAid = 0;
        dwTrueReqAl = ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET;
        dwTrueAlPos = EN_ALLIANCE_POS__REQUEST;
    }
    else
    {
        dwTrueAid = ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET;
        dwTrueReqAl = 0;
        dwTrueAlPos = ptbPlayer->m_nAlpos;
    }
    rJson["aid"] = dwTrueAid;
    rJson["alpos"] = dwTrueAlPos;
    rJson["req_al"] = dwTrueReqAl;

    rJson["utime"] = ptbPlayer->m_nUtime;

    rJson["force"] = ptbPlayer->m_nMight;
    rJson["force_kill"] = ptbPlayer->m_bWar_statistics[0].ddwForceKilled;

    rJson["city_pos"] = ptbPlayer->m_nCid;

    rJson["al_stat"] = Json::Value(Json::arrayValue);
    rJson["al_stat"].append(ptbPlayer->m_nLoy_time);
    rJson["al_stat"].append(ptbPlayer->m_nLoy_itv);
    rJson["al_stat"].append(ptbPlayer->m_nLoy_cur);
    rJson["al_stat"].append(ptbPlayer->m_nLoy_all);
    rJson["al_stat"].append(ptbPlayer->m_nLoy_all - ptbPlayer->m_nLoy_cur);
}

TVOID CCommJson::GenUniqueNameInfo(TbPlayer* ptbPlayer, Json::Value& rJson)
{
    rJson = Json::Value(Json::objectValue);
    rJson["sid"] = ptbPlayer->m_nSid;
    rJson["uid"] = ptbPlayer->m_nUid;
    rJson["uname"] = ptbPlayer->m_sUin;
    rJson["avatar"] = ptbPlayer->m_nAvatar;
    rJson["al_nick_name"] = ptbPlayer->m_sAl_nick_name;

    rJson["new_al_status"] = 0;
    TINT32 dwTrueAid = 0;
    TINT32 dwTrueAlPos = 0;
    TINT32 dwTrueReqAl = 0;
    if (ptbPlayer->m_nAlpos == EN_ALLIANCE_POS__REQUEST)
    {
        dwTrueAid = 0;
        dwTrueReqAl = ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET;
        dwTrueAlPos = EN_ALLIANCE_POS__REQUEST;
    }
    else
    {
        dwTrueAid = ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET;
        dwTrueReqAl = 0;
        dwTrueAlPos = ptbPlayer->m_nAlpos;
    }
    rJson["aid"] = dwTrueAid;
    rJson["alpos"] = dwTrueAlPos;
    rJson["req_al"] = dwTrueReqAl;
}

TVOID CCommJson::GenReportInfo(TbReport* ptbReport, TBOOL bReturnLarge, Json::Value& rjson)
{
    //     {
    //     "base":[
    //             6,              //report id
    //             1421323823,     //unix time
    //             3,              //report type
    //             0,              //result 见EReportResult
    //     ],
    //     "from":[
    //         1153387,        //uid
    //             190094,         //cid
    //             7,              //aid
    //             0,              //wild type
    //             5,              //wild level
    //             int             //wild svr type
    //     ],
    //     "to":[
    //         1153387,        //uid
    //             190094,         //cid
    //             7,              //aid
    //             0,              //wild type
    //             5,              //wild level
    //             int             //wild svr type
    //     ],
    //     "name_info":[
    //         "suname",           //发起者名字
    //             "scityname",        //发起者城市名字
    //             "salname",          //发起者联盟名字
    //             "tuname",           //目标玩家名字
    //             "tcityname",        //目标玩家城市名字
    //             "talname",          //目标玩家联盟名字
    //     ],
    //     "content": {        //不同report type 不一样
    //         }
    //     }
    rjson = Json::Value(Json::objectValue);
    rjson["base"] = Json::Value(Json::arrayValue);
    rjson["base"][0U] = ptbReport->m_nId;
    rjson["base"][1U] = ptbReport->m_nTime;
    rjson["base"][2U] = ptbReport->m_nType;
    rjson["base"][3U] = ptbReport->m_nResult;
    rjson["base"][4U] = ptbReport->m_nSid;

    rjson["from"] = Json::Value(Json::arrayValue);
    rjson["from"][0U] = ptbReport->m_bFrom[0].m_ddwUserId;
    rjson["from"][1U] = ptbReport->m_bFrom[0].m_ddwPos;
    rjson["from"][2U] = ptbReport->m_bFrom[0].m_ddwAlId;
    rjson["from"][3U] = ptbReport->m_bFrom[0].m_ddwPosType;
    rjson["from"][4U] = ptbReport->m_bFrom[0].m_ddwPosLevel;
    rjson["from"][5U] = ptbReport->m_bFrom[0].m_ddwSvrWildType;
    rjson["from"][6U] = ptbReport->m_bFrom[0].m_ddwSid;

    rjson["to"] = Json::Value(Json::arrayValue);
    rjson["to"][0U] = ptbReport->m_bTo[0].m_ddwUserId;
    rjson["to"][1U] = ptbReport->m_bTo[0].m_ddwPos;
    rjson["to"][2U] = ptbReport->m_bTo[0].m_ddwAlId;
    rjson["to"][3U] = ptbReport->m_bTo[0].m_ddwPosType;
    rjson["to"][4U] = ptbReport->m_bTo[0].m_ddwPosLevel;
    rjson["to"][5U] = ptbReport->m_bTo[0].m_ddwSvrWildType;
    rjson["to"][6U] = ptbReport->m_bTo[0].m_ddwSid;

    CCommJson::GenNameInfo(ptbReport, rjson);

    Json::Reader jsonReader;
    TBOOL bHasContent = TRUE;
    if(!ptbReport->m_sContent.empty())
    {
        jsonReader.parse(ptbReport->m_sContent, rjson["content"]);
        if (ptbReport->m_nType == EN_REPORT_TYPE_SCOUT)
        {
            if(rjson["content"]["info"].empty())
            {
                rjson["content"]["info"] = Json::Value(Json::objectValue);
                SCommonResource stEmptyResource;
                CCommJson::GenResourceJson(&stEmptyResource, rjson["content"]["info"]["resource"]);
                SCommonTroop stEmptyTroop;
                CCommJson::GenTroopJson(&stEmptyTroop, rjson["content"]["info"]["troop"]);
                SCommonFort stEmptyFort;
                CCommJson::GenFortJson(&stEmptyFort, rjson["content"]["info"]["fort"]);
                rjson["content"]["info"]["building"] = Json::Value(Json::objectValue);
                rjson["content"]["info"]["battle_buff"] = Json::Value(Json::arrayValue);
                rjson["content"]["info"]["defender_reinforce"] = Json::Value(Json::arrayValue);
                rjson["content"]["info"]["dragon_status"] = 0;
                rjson["content"]["info"]["dragon"] = Json::Value(Json::arrayValue);
                rjson["content"]["info"]["dragon"].append("");
                rjson["content"]["info"]["dragon"].append(0);
                rjson["content"]["info"]["dragon"].append(0);
                rjson["content"]["info"]["dragon"].append(0);
            }
        }

        if (bReturnLarge == FALSE)
        {
            if (ptbReport->m_nType == EN_REPORT_TYPE_RALLY_WAR
                || ptbReport->m_nType == EN_REPORT_TYPE_THRONE_RALLY_WAR
                || ptbReport->m_nType == EN_REPORT_TYPE_ATTACK
                || ptbReport->m_nType == EN_REPORT_TYPE_THRONE_ATTACK)
            {
                TUINT32 udwTotalReinforce = 0;
                if (rjson["content"].isMember("attacker_reinforce"))
                {
                    udwTotalReinforce += rjson["content"]["attacker_reinforce"].size();
                }
                if (rjson["content"].isMember("defender_reinforce"))
                {
                    udwTotalReinforce += rjson["content"]["defender_reinforce"].size();
                }
                if (udwTotalReinforce >= 10)
                {
                    bHasContent = FALSE;
                    rjson["content"].clear();
                    rjson["content"] = Json::Value(Json::objectValue);
                }
            }
            else if (ptbReport->m_nType == EN_REPORT_TYPE_SCOUT)
            {
                TUINT32 udwTotalReinforce = 0;
                if (rjson["content"]["info"].isMember("defender_reinforce"))
                {
                    udwTotalReinforce += rjson["content"]["info"]["defender_reinforce"].size();
                }
                if (udwTotalReinforce >= 10)
                {
                    bHasContent = FALSE;
                    rjson["content"].clear();
                    rjson["content"] = Json::Value(Json::objectValue);
                }
            }
        }
    }
    rjson["has_content"] = bHasContent? 1 : 0;
}

TVOID CCommJson::GenReportInfo(TbReport* ptbReport, SReportEntry* pstEntry, TBOOL bReturnLarge, Json::Value& rjson)
{
    //     {
    //     "base":[
    //             6,              //report id
    //             1421323823,     //unix time
    //             3,              //report type
    //             0,              //result 见EReportResult
    //     ],
    //     "from":[
    //         1153387,        //uid
    //             190094,         //cid
    //             7,              //aid
    //             0,              //wild type
    //             5,              //wild level
    //     ],
    //     "to":[
    //         1153387,        //uid
    //             190094,         //cid
    //             7,              //aid
    //             0,              //wild type
    //             5,              //wild level
    //     ],
    //     "name_info":[
    //         "suname",           //发起者名字
    //             "scityname",        //发起者城市名字
    //             "salname",          //发起者联盟名字
    //             "tuname",           //目标玩家名字
    //             "tcityname",        //目标玩家城市名字
    //             "talname",          //目标玩家联盟名字
    //     ],
    //     "content": {        //不同report type 不一样
    //         }
    //     }
    rjson = Json::Value(Json::objectValue);
    rjson["base"] = Json::Value(Json::arrayValue);
    rjson["base"][0U] = ptbReport->m_nId;
    rjson["base"][1U] = ptbReport->m_nTime;
    rjson["base"][2U] = ptbReport->m_nType;
    rjson["base"][3U] = ptbReport->m_nResult;

    rjson["from"] = Json::Value(Json::arrayValue);
    rjson["from"][0U] = ptbReport->m_bFrom[0].m_ddwUserId;
    rjson["from"][1U] = ptbReport->m_bFrom[0].m_ddwPos;
    rjson["from"][2U] = ptbReport->m_bFrom[0].m_ddwAlId;
    rjson["from"][3U] = ptbReport->m_bFrom[0].m_ddwPosType;
    rjson["from"][4U] = ptbReport->m_bFrom[0].m_ddwPosLevel;
    rjson["from"][5U] = ptbReport->m_bFrom[0].m_ddwSvrWildType;
    rjson["from"][6U] = ptbReport->m_bFrom[0].m_ddwSid;

    rjson["to"] = Json::Value(Json::arrayValue);
    rjson["to"][0U] = ptbReport->m_bTo[0].m_ddwUserId;
    rjson["to"][1U] = ptbReport->m_bTo[0].m_ddwPos;
    rjson["to"][2U] = ptbReport->m_bTo[0].m_ddwAlId;
    rjson["to"][3U] = ptbReport->m_bTo[0].m_ddwPosType;
    rjson["to"][4U] = ptbReport->m_bTo[0].m_ddwPosLevel;
    rjson["to"][5U] = ptbReport->m_bTo[0].m_ddwSvrWildType;
    rjson["to"][6U] = ptbReport->m_bTo[0].m_ddwSid;

    CCommJson::GenNameInfo(ptbReport, rjson);

    Json::Reader jsonReader;
    TBOOL bHasContent = TRUE;
    if(!ptbReport->m_sContent.empty())
    {
        jsonReader.parse(ptbReport->m_sContent, rjson["content"]);
        if (ptbReport->m_nType == EN_REPORT_TYPE_SCOUT)
        {
            if (ptbReport->m_nResult != EN_REPORT_RESULT_SCOUT_BE_PREVENT)
            {
                if (rjson["content"]["info"].empty())
                {
                    rjson["content"]["info"] = Json::Value(Json::objectValue);
                    SCommonResource stEmptyResource;
                    CCommJson::GenResourceJson(&stEmptyResource, rjson["content"]["info"]["resource"]);
                    SCommonTroop stEmptyTroop;
                    CCommJson::GenTroopJson(&stEmptyTroop, rjson["content"]["info"]["troop"]);
                    SCommonFort stEmptyFort;
                    CCommJson::GenFortJson(&stEmptyFort, rjson["content"]["info"]["fort"]);
                    rjson["content"]["info"]["building"] = Json::Value(Json::objectValue);
                    rjson["content"]["info"]["battle_buff"] = Json::Value(Json::arrayValue);
                    rjson["content"]["info"]["defender_reinforce"] = Json::Value(Json::arrayValue);
                    rjson["content"]["info"]["dragon_status"] = 0;
                    rjson["content"]["info"]["dragon"] = Json::Value(Json::arrayValue);
                    rjson["content"]["info"]["dragon"].append("");
                    rjson["content"]["info"]["dragon"].append(0);
                    rjson["content"]["info"]["dragon"].append(0);
                    rjson["content"]["info"]["dragon"].append(0);
                }
            }
        }

        if (bReturnLarge == FALSE)
        {
            if (ptbReport->m_nType == EN_REPORT_TYPE_RALLY_WAR
                || ptbReport->m_nType == EN_REPORT_TYPE_THRONE_RALLY_WAR
                || ptbReport->m_nType == EN_REPORT_TYPE_ATTACK
                || ptbReport->m_nType == EN_REPORT_TYPE_THRONE_ATTACK)
            {
                TUINT32 udwTotalReinforce = 0;
                if (rjson["content"].isMember("attacker_reinforce"))
                {
                    udwTotalReinforce += rjson["content"]["attacker_reinforce"].size();
                }
                if (rjson["content"].isMember("defender_reinforce"))
                {
                    udwTotalReinforce += rjson["content"]["defender_reinforce"].size();
                }
                if (udwTotalReinforce >= 10)
                {
                    Json::Value jAttackerList = Json::Value(Json::arrayValue);
                    if (rjson["content"].isMember("attacker_reinforce"))
                    {
                        for (TUINT32 udwIdx = 0; udwIdx < rjson["content"]["attacker_reinforce"].size(); udwIdx++)
                        {
                            jAttackerList.append(rjson["content"]["attacker_reinforce"][udwIdx]["player_uid"].asInt64());
                        }
                    }
                    Json::Value jDefenderList = Json::Value(Json::arrayValue);
                    if (rjson["content"].isMember("defender_reinforce"))
                    {
                        for (TUINT32 udwIdx = 0; udwIdx < rjson["content"]["defender_reinforce"].size(); udwIdx++)
                        {
                            jDefenderList.append(rjson["content"]["defender_reinforce"][udwIdx]["player_uid"].asInt64());
                        }
                    }
                    bHasContent = FALSE;
                    rjson["content"].clear();
                    rjson["content"] = Json::Value(Json::objectValue);
                    rjson["content"]["attacker_reinforce_uid"] = jAttackerList;
                    rjson["content"]["defender_reinforce_uid"] = jDefenderList;
                }
            }
            else if (ptbReport->m_nType == EN_REPORT_TYPE_SCOUT)
            {
                if (ptbReport->m_nResult != EN_REPORT_RESULT_SCOUT_BE_PREVENT)
                {
                    TUINT32 udwTotalReinforce = 0;
                    if (rjson["content"]["info"].isMember("defender_reinforce"))
                    {
                        udwTotalReinforce += rjson["content"]["info"]["defender_reinforce"].size();
                    }
                    if (udwTotalReinforce >= 10)
                    {
                        Json::Value jDefenderList = Json::Value(Json::arrayValue);
                        if (rjson["content"]["info"].isMember("defender_reinforce"))
                        {
                            for (TUINT32 udwIdx = 0; udwIdx < rjson["content"]["info"]["defender_reinforce"].size(); udwIdx++)
                            {
                                jDefenderList.append(rjson["content"]["info"]["defender_reinforce"][udwIdx]["player_uid"].asInt64());
                            }
                        }
                        bHasContent = FALSE;
                        rjson["content"].clear();
                        rjson["content"] = Json::Value(Json::objectValue);
                        rjson["content"]["reinforce_uid"] = jDefenderList;
                    }
                }
            }
        }
    }

    rjson["read"] = (pstEntry->dwStatus & EN_MAIL_STATUS_READ) > 0 ? 1 : 0;
    rjson["has_content"] = bHasContent ? 1 : 0;
}
TVOID CCommJson::GenActionInfo(TbAction* ptbAction, Json::Value& rjson)
{
    rjson = Json::Value(Json::objectValue);
    rjson["basic"] = Json::Value(Json::arrayValue);
    rjson["basic"][0] = ptbAction->m_nId;
    rjson["basic"][1] = ptbAction->m_nSuid;
    rjson["basic"][2] = 0;
    rjson["basic"][3] = 0;
    rjson["basic"][4] = 0;
    rjson["basic"][5] = 0;
    rjson["basic"][6] = 0;
    rjson["basic"][7] = ptbAction->m_nBtime;
    rjson["basic"][8] = ptbAction->m_nCtime;
    TINT64 dwEndTime = ptbAction->m_nEtime;
    TINT64 dwCurTime = CTimeUtils::GetUnixTime();
    if(dwEndTime != (INT64_MAX) && dwCurTime > dwEndTime + 4)
    {
        dwEndTime = dwCurTime + 30 + rand() % 30;
    }
    rjson["basic"][9] = dwEndTime;
    rjson["basic"][10] = ptbAction->m_nSclass;
    rjson["basic"][11] = ptbAction->m_nStatus;
    rjson["basic"][12] = 0;
    rjson["basic"][13] = 0;
    rjson["basic"][14] = 0;
    rjson["basic"][15] = ptbAction->m_nMclass;
    rjson["basic"][16] = 0;
    rjson["basic"][17] = ptbAction->m_nSid;
    rjson["basic"][18] = CActionBase::GetActionDisplayClass(ptbAction->m_nMclass);

    rjson["name_info"] = Json::Value(Json::objectValue);
    rjson["name_info"]["source"] = Json::Value(Json::arrayValue);
    rjson["name_info"]["target"] = Json::Value(Json::arrayValue);
    //source user name
    switch(ptbAction->m_nMclass)
    {
    default:
        rjson["name_info"]["source"].append("");
        break;
    }
    //source city name
    switch(ptbAction->m_nMclass)
    {
    default:
        rjson["name_info"]["source"].append("");
        break;
    }
    //source alliance name
    switch(ptbAction->m_nMclass)
    {
    default:
        rjson["name_info"]["source"].append("");
        break;
    }
    //sorce nick name
    switch (ptbAction->m_nMclass)
    {
    default:
        rjson["name_info"]["source"].append("");
        break;
    }

    //target user name
    switch(ptbAction->m_nMclass)
    {
    default:
        rjson["name_info"]["target"].append("");
        break;
    }
    //target city name
    switch(ptbAction->m_nMclass)
    {
    default:
        rjson["name_info"]["target"].append("");
        break;
    }
    //target alliance name
    switch(ptbAction->m_nMclass)
    {
    default:
        rjson["name_info"]["target"].append("");
        break;
    }
    //target nick name
    switch(ptbAction->m_nMclass)
    {
    default:
        rjson["name_info"]["target"].append("");
        break;
    }

    // 基于客户端class进行区分
    //param
    switch(ptbAction->m_nSclass)
    {
    case EN_ACTION_SEC_CLASS__ITEM:
        rjson["param"] = Json::Value(Json::objectValue);
        rjson["param"]["buffer_id"] = ptbAction->m_bParam.m_astList->m_stItem.m_ddwBufferId;
        rjson["param"]["buffer_num"] = ptbAction->m_bParam.m_astList->m_stItem.m_ddwNum;
        rjson["param"]["buffer_time"] = ptbAction->m_bParam.m_astList->m_stItem.m_ddwTime;
        rjson["param"]["item_id"] = ptbAction->m_bParam.m_astList->m_stItem.m_ddwItemId;
        break;
    case EN_ACTION_SEC_CLASS__ATTACK_MOVE:
        rjson["param"] = Json::Value(Json::objectValue);
        rjson["param"]["city_pos"] = ptbAction->m_bParam.m_astList->m_stAttackMove.m_ddwCityId;
        break;
    default:
        rjson["param"] = Json::Value(Json::objectValue);
        break;
    }

    rjson["map"] = Json::Value(Json::arrayValue);
    rjson["map"].append(1u);
    rjson["map"].append(1u);

    rjson["can_recall"] = FALSE;
    rjson["seq"] = ptbAction->m_nSeq;
}

TVOID CCommJson::GenMarchInfo(TbMarch_action* ptbMarch, Json::Value& rjson)
{
    if(ptbMarch->m_bPrison_param[0].stDragon.m_ddwLevel > 0)
    {
        if (EN_ACTION_SEC_CLASS__CATCH_DRAGON != ptbMarch->m_nSclass)
        {
            ptbMarch->m_bParam[0].m_stDragon = ptbMarch->m_bPrison_param[0].stDragon;
        }
        strncpy(ptbMarch->m_bParam[0].m_szSourceUserName, ptbMarch->m_bPrison_param[0].szSourceUserName, MAX_TABLE_NAME_LEN);
        ptbMarch->m_bParam[0].m_szSourceUserName[MAX_TABLE_NAME_LEN - 1] = '\0';
        strncpy(ptbMarch->m_bParam[0].m_szTargetUserName, ptbMarch->m_bPrison_param[0].szTargetUserName, MAX_TABLE_NAME_LEN);
        ptbMarch->m_bParam[0].m_szTargetUserName[MAX_TABLE_NAME_LEN - 1] = '\0';
        strncpy(ptbMarch->m_bParam[0].m_szTargetCityName, ptbMarch->m_bPrison_param[0].szTargetCityName, MAX_TABLE_NAME_LEN);
        ptbMarch->m_bParam[0].m_szTargetCityName[MAX_TABLE_NAME_LEN - 1] = '\0';

        ptbMarch->m_bParam.m_astList[0].m_ddwTargetType = EN_WILD_TYPE__CITY;
    }

    if(ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR
    && ptbMarch->m_nStatus == EN_MARCH_STATUS__PREPARING)
    {
        ptbMarch->m_nCtime = ptbMarch->m_bParam[0].m_ddwPrepareTime;
    }

    rjson["basic"] = Json::Value(Json::arrayValue);
    rjson["basic"][0] = ptbMarch->m_nId;
    rjson["basic"][1] = ptbMarch->m_nSuid;
    rjson["basic"][2] = ptbMarch->m_nScid;
    rjson["basic"][3] = ptbMarch->m_nSal > 0 ? ptbMarch->m_nSal : ptbMarch->m_bParam[0].m_ddwSourceAlliance;
    rjson["basic"][4] = ptbMarch->m_nTuid;
    rjson["basic"][5] = ptbMarch->m_nTpos;
    rjson["basic"][6] = ptbMarch->m_nTal;
    rjson["basic"][7] = ptbMarch->m_nBtime;
    rjson["basic"][8] = ptbMarch->m_nCtime;
    TINT64 dwEndTime = ptbMarch->m_nEtime;
    TINT64 dwCurTime = CTimeUtils::GetUnixTime();
    if(dwEndTime != (INT64_MAX) && dwCurTime > dwEndTime + 4)
    {
        dwEndTime = dwCurTime + 30 + rand() % 30;
    }
    rjson["basic"][9] = dwEndTime;
    rjson["basic"][10] = ptbMarch->m_nSclass;
    rjson["basic"][11] = ptbMarch->m_nStatus;
    rjson["basic"][12] = 0;
    rjson["basic"][13] = 0;
    rjson["basic"][14] = ptbMarch->m_nTid;
    rjson["basic"][15] = ptbMarch->m_nMclass;
    rjson["basic"][16] = ptbMarch->m_nSavatar;
    rjson["basic"][17] = ptbMarch->m_nSid;
    rjson["basic"][18] = CActionBase::GetActionDisplayClass(ptbMarch->m_nMclass);


    rjson["name_info"] = Json::Value(Json::objectValue);
    rjson["name_info"]["source"] = Json::Value(Json::arrayValue);
    rjson["name_info"]["target"] = Json::Value(Json::arrayValue);

    //source user name
    switch(ptbMarch->m_nMclass)
    {
    case EN_ACTION_MAIN_CLASS__MARCH:
        rjson["name_info"]["source"].append(ptbMarch->m_bParam.m_astList[0].m_szSourceUserName);
        break;
    default:
        rjson["name_info"]["source"].append("");
        break;
    }
    //source city name
    switch(ptbMarch->m_nMclass)
    {
    case EN_ACTION_MAIN_CLASS__MARCH:
        rjson["name_info"]["source"].append(ptbMarch->m_bParam.m_astList[0].m_szSourceCityName);
        break;
    default:
        rjson["name_info"]["source"].append("");
        break;
    }
    //source alliance name
    switch(ptbMarch->m_nMclass)
    {
    case EN_ACTION_MAIN_CLASS__MARCH:
        rjson["name_info"]["source"].append(ptbMarch->m_bParam.m_astList[0].m_szSourceAlliance);
        break;
    default:
        rjson["name_info"]["source"].append("");
        break;
    }
    //sorce nick name
    switch (ptbMarch->m_nMclass)
    {
    case EN_ACTION_MAIN_CLASS__MARCH:
        rjson["name_info"]["source"].append(ptbMarch->m_bParam[0].m_szSourceAlNick);
        break;
    default:
        rjson["name_info"]["source"].append("");
        break;
    }

    //target user name
    switch(ptbMarch->m_nMclass)
    {
    case EN_ACTION_MAIN_CLASS__MARCH:
        rjson["name_info"]["target"].append(ptbMarch->m_bParam.m_astList[0].m_szTargetUserName);
        break;
    default:
        rjson["name_info"]["target"].append("");
        break;
    }
    //target city name
    switch(ptbMarch->m_nMclass)
    {
    case EN_ACTION_MAIN_CLASS__MARCH:
        rjson["name_info"]["target"].append(ptbMarch->m_bParam.m_astList[0].m_szTargetCityName);
        break;
    default:
        rjson["name_info"]["target"].append("");
        break;
    }
    //target alliance name
    switch(ptbMarch->m_nMclass)
    {
    case EN_ACTION_MAIN_CLASS__MARCH:
        rjson["name_info"]["target"].append(ptbMarch->m_bParam.m_astList[0].m_szTargetAlliance);
        break;
    default:
        rjson["name_info"]["target"].append("");
        break;
    }
    //target nick name
    switch(ptbMarch->m_nMclass)
    {
    case EN_ACTION_MAIN_CLASS__MARCH:
        rjson["name_info"]["target"].append(ptbMarch->m_bParam[0].m_szTargetAlNick);
        break;
    default:
        rjson["name_info"]["target"].append("");
        break;
    }

    // 基于客户端class进行区分
    //param
    rjson["param"] = Json::Value(Json::objectValue);
    switch(ptbMarch->m_nSclass)
    {
    case EN_ACTION_SEC_CLASS__ATTACK:
    case EN_ACTION_SEC_CLASS__TRANSPORT:
    case EN_ACTION_SEC_CLASS__REINFORCE_NORMAL:
    case EN_ACTION_SEC_CLASS__SCOUT:
    case EN_ACTION_SEC_CLASS__OCCUPY:
    case EN_ACTION_SEC_CLASS__RALLY_WAR:
    case EN_ACTION_SEC_CLASS__RALLY_REINFORCE:
    case EN_ACTION_SEC_CLASS__DRAGON_ATTACK:
    case EN_ACTION_SEC_CLASS__CATCH_DRAGON:
    case EN_ACTION_SEC_CLASS__RELEASE_DRAGON:
    case EN_ACTION_SEC_CLASS__CAMP:
    case EN_ACTION_SEC_CLASS__ATTACK_IDOL:
    case EN_ACTION_SEC_CLASS__REINFORCE_THRONE:
    case EN_ACTION_SEC_CLASS__ASSIGN_THRONE:
    case EN_ACTION_SEC_CLASS__ATTACK_THRONE:
    case EN_ACTION_SEC_CLASS__RALLY_WAR_THRONE:
        {
             CCommJson::GenTroopJson(&ptbMarch->m_bParam.m_astList[0].m_stTroop, rjson["param"]["troop"]);

             CCommJson::GenResourceJson(&ptbMarch->m_bParam.m_astList[0].m_stResource, rjson["param"]["resource"]);
             
             rjson["param"]["target_wild_type"] = ptbMarch->m_bParam.m_astList[0].m_ddwTargetType;
             rjson["param"]["scout_level"] = ptbMarch->m_bParam.m_astList[0].m_ddwScoutLevel;

             rjson["param"]["load_rate"] = ptbMarch->m_bParam.m_astList[0].m_ddwLoadRate;
             rjson["param"]["load_time"] = ptbMarch->m_bParam.m_astList[0].m_ddwLoadTime;
             rjson["param"]["knight"] = Json::Value(Json::arrayValue);
             rjson["param"]["knight"].append(ptbMarch->m_bParam[0].m_stKnight.ddwId);
             rjson["param"]["knight"].append(ptbMarch->m_bParam[0].m_stKnight.ddwLevel);
             rjson["param"]["knight"].append(ptbMarch->m_bParam[0].m_stKnight.ddwExpAdd);

             rjson["param"]["dragon"] = Json::Value(Json::arrayValue);
             rjson["param"]["dragon"].append(ptbMarch->m_bParam[0].m_stDragon.m_szName);
             rjson["param"]["dragon"].append(ptbMarch->m_bParam[0].m_stDragon.m_ddwLevel);
             rjson["param"]["dragon"].append(ptbMarch->m_bParam[0].m_stDragon.m_ddwIconId);
             rjson["param"]["dragon"].append(ptbMarch->m_bParam[0].m_stDragon.m_ddwExpInc);

             rjson["param"]["side"] = ptbMarch->m_bParam[0].m_ddwSide;
             rjson["param"]["force"] = ptbMarch->m_bParam[0].m_ddwForce;
             rjson["param"]["troop_num"] = ptbMarch->m_bParam[0].m_ddwTroopNum;
             rjson["param"]["captured_dragon"] = ptbMarch->m_bParam[0].m_ddwCaptureDragonFlag;
             rjson["param"]["has_win"] = ptbMarch->m_bParam[0].m_ddwWinFlag;
        }
        break;
    default:
        break;
    }

    rjson["param"]["battle_buff"] = Json::Value(Json::arrayValue);
    if (ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR
        || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__ATTACK
        || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR_THRONE
        || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__ATTACK_THRONE)
    {
        SPlayerBuffInfo stTmpBuff;
        CBufferBase::MarchBuffToPlayerBuff(ptbMarch, &stTmpBuff);
        for (TUINT32 udwIdx = 0, udwJsonIndex = 0; udwIdx < EN_BUFFER_INFO_END; ++udwIdx)
        {
            TINT32 dwBufferId = stTmpBuff[udwIdx].m_udwBuffId;
            TINT64 ddwBuffNum = stTmpBuff[udwIdx].m_ddwBuffTotal;
            if (ddwBuffNum > 0)
            {
                if (!CToolBase::IsWatchTowerShowBuff(dwBufferId))
                {
                    continue;
                }
                rjson["param"]["battle_buff"][udwJsonIndex] = Json::Value(Json::arrayValue);
                rjson["param"]["battle_buff"][udwJsonIndex].append(dwBufferId);
                rjson["param"]["battle_buff"][udwJsonIndex].append(ddwBuffNum);
                ++udwJsonIndex;
            }
        }
    }
    
    if(ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR
        || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR_THRONE)
    {
        rjson["rally_attacker"] = Json::Value(Json::objectValue);
        rjson["rally_attacker"]["reinforce_troop_limit"] = ptbMarch->m_bRally_atk_force[0].ddwReinforceTroopLimit;
        rjson["rally_attacker"]["reinforce_troop_num"] = ptbMarch->m_bRally_atk_force[0].ddwReinforceNum;
        rjson["rally_attacker"]["reinforce_troop_force"] = ptbMarch->m_bRally_atk_force[0].ddwReinforceForce;
        rjson["rally_attacker"]["total_troop_num"] = ptbMarch->m_bRally_atk_force[0].ddwTotalNum;
        rjson["rally_attacker"]["total_troop_force"] = ptbMarch->m_bRally_atk_force[0].ddwTotalForce;
        rjson["rally_attacker"]["slot"] = Json::Value(Json::arrayValue);
        for(TUINT32 udwIdx = 0; udwIdx < ptbMarch->m_bRally_atk_slot.m_udwNum; ++udwIdx)
        {
            rjson["rally_attacker"]["slot"][udwIdx] = Json::Value(Json::arrayValue);
            rjson["rally_attacker"]["slot"][udwIdx].append(ptbMarch->m_bRally_atk_slot[udwIdx].ddwUid);
            rjson["rally_attacker"]["slot"][udwIdx].append(ptbMarch->m_bRally_atk_slot[udwIdx].ddwMarchId);
            rjson["rally_attacker"]["slot"][udwIdx].append(ptbMarch->m_bRally_atk_slot[udwIdx].szUserName);
        }
        CCommJson::GenTroopJson(&ptbMarch->m_bAtk_total_troop[0], rjson["rally_attacker"]["total_troop"]);

        rjson["rally_defender"] = Json::Value(Json::objectValue);
        rjson["rally_defender"]["reinforce_troop_limit"] = ptbMarch->m_bRally_def_force[0].ddwReinforceTroopLimit;
        rjson["rally_defender"]["reinforce_troop_num"] = ptbMarch->m_bRally_def_force[0].ddwReinforceNum;
        rjson["rally_defender"]["reinforce_troop_force"] = ptbMarch->m_bRally_def_force[0].ddwReinforceForce;
        rjson["rally_defender"]["total_troop_num"] = ptbMarch->m_bRally_def_force[0].ddwTotalNum;
        rjson["rally_defender"]["total_troop_force"] = ptbMarch->m_bRally_def_force[0].ddwTotalForce;
        rjson["rally_defender"]["slot"] = Json::Value(Json::arrayValue);
        for(TUINT32 udwIdx = 0; udwIdx < ptbMarch->m_bRally_def_slot.m_udwNum; ++udwIdx)
        {
            rjson["rally_defender"]["slot"][udwIdx] = Json::Value(Json::arrayValue);
            rjson["rally_defender"]["slot"][udwIdx].append(ptbMarch->m_bRally_def_slot[udwIdx].ddwUid);
            rjson["rally_defender"]["slot"][udwIdx].append(ptbMarch->m_bRally_def_slot[udwIdx].ddwMarchId);
            rjson["rally_defender"]["slot"][udwIdx].append(ptbMarch->m_bRally_def_slot[udwIdx].szUserName);
        }
        CCommJson::GenTroopJson(&ptbMarch->m_bDef_total_troop[0], rjson["rally_defender"]["total_troop"]);
    }

    rjson["map"] = Json::Value(Json::arrayValue);
    rjson["map"].append(1u);
    rjson["map"].append(CMapBase::GetWildBlockNumByType(ptbMarch->m_nSid, ptbMarch->m_bParam.m_astList[0].m_ddwTargetType));

    if(ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_REINFORCE)
    {
        if (ptbMarch->m_nStatus != EN_MARCH_STATUS__MARCHING)
        {
            rjson["can_recall"] = FALSE;
        }
        else
        {
            rjson["can_recall"] = TRUE;
        }
    }
    else if(ptbMarch->m_nStatus == EN_MARCH_STATUS__MARCHING
        || ptbMarch->m_nStatus == EN_MARCH_STATUS__DEFENDING
        || ptbMarch->m_nStatus == EN_MARCH_STATUS__LOADING
        || ptbMarch->m_nStatus == EN_MARCH_STATUS__CAMPING_WITH_PEACETIME
        || ptbMarch->m_nStatus == EN_MARCH_STATUS__CAMPING_NORMAL)
    {
        rjson["can_recall"] = TRUE;
    }
    else
    {
        rjson["can_recall"] = FALSE;
    }

    if (ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__DRAGON_ATTACK)
    {
        rjson["param"]["dragon_attack"] = Json::Value(Json::arrayValue);
        rjson["param"]["dragon_attack"].append(ptbMarch->m_bMonster_info[0].ddwSpecailHitType);
        rjson["param"]["dragon_attack"].append(ptbMarch->m_bMonster_info[0].ddwDeadFlag);
        rjson["param"]["dragon_attack"].append(ptbMarch->m_bMonster_info[0].ddwHpLose);
        rjson["param"]["dragon_attack"].append(ptbMarch->m_bMonster_info[0].ddwExpGet);
        Json::Value jReward = Json::Value(Json::arrayValue);
        TUINT32 udwJsonIdx = 0;
        for (TUINT32 udwIdx = 0; udwIdx < ptbMarch->m_bReward[0].ddwTotalNum; udwIdx++)
        {
            jReward[udwJsonIdx] = Json::Value(Json::arrayValue);
            jReward[udwJsonIdx].append(ptbMarch->m_bReward[0].aRewardList[udwIdx].ddwType);
            jReward[udwJsonIdx].append(ptbMarch->m_bReward[0].aRewardList[udwIdx].ddwId);
            jReward[udwJsonIdx].append(ptbMarch->m_bReward[0].aRewardList[udwIdx].ddwNum);
            udwJsonIdx++;
        }
        for (TUINT32 udwIdx = 0; udwIdx < ptbMarch->m_bEx_reward.m_udwNum; udwIdx++)
        {
            jReward[udwJsonIdx] = Json::Value(Json::arrayValue);
            jReward[udwJsonIdx].append(ptbMarch->m_bEx_reward[udwIdx].ddwType);
            jReward[udwJsonIdx].append(ptbMarch->m_bEx_reward[udwIdx].ddwId);
            jReward[udwJsonIdx].append(ptbMarch->m_bEx_reward[udwIdx].ddwNum);
            udwJsonIdx++;
        }
        rjson["param"]["dragon_attack"].append(jReward);
    }

    rjson["seq"] = ptbMarch->m_nSeq;
}

TVOID CCommJson::GenAlActionInfo(TbAlliance_action* ptbAlAction, Json::Value& rjson)
{
    rjson = Json::Value(Json::objectValue);
    rjson["basic"] = Json::Value(Json::arrayValue);
    rjson["basic"][0] = ptbAlAction->m_nId;
    rjson["basic"][1] = ptbAlAction->m_nSuid;
    rjson["basic"][2] = 0;
    rjson["basic"][3] = ptbAlAction->m_nSal;
    rjson["basic"][4] = 0;
    rjson["basic"][5] = 0;
    rjson["basic"][6] = 0;
    rjson["basic"][7] = ptbAlAction->m_nBtime;
    rjson["basic"][8] = ptbAlAction->m_nCtime;
    TINT64 dwEndTime = ptbAlAction->m_nEtime;
    TINT64 dwCurTime = CTimeUtils::GetUnixTime();
    if(dwEndTime != (INT64_MAX) && dwCurTime > dwEndTime + 4)
    {
        dwEndTime = dwCurTime + 30 + rand() % 30;
    }
    rjson["basic"][9] = dwEndTime;
    rjson["basic"][10] = ptbAlAction->m_nSclass;
    rjson["basic"][11] = ptbAlAction->m_nStatus;
    if (ptbAlAction->m_nMclass == EN_ACTION_MAIN_CLASS__TRAIN_NEW && ptbAlAction->m_nCan_help_num == 0)
    {
        rjson["basic"][12] = 1;
    }
    else
    {
        rjson["basic"][12] = ptbAlAction->m_nCan_help_num;
    }
    rjson["basic"][13] = ptbAlAction->m_nHelped_num;
    rjson["basic"][14] = 0;
    rjson["basic"][15] = ptbAlAction->m_nMclass;
    rjson["basic"][16] = 0;
    rjson["basic"][17] = ptbAlAction->m_nSid;
    rjson["basic"][18] = CActionBase::GetActionDisplayClass(ptbAlAction->m_nMclass);

    rjson["name_info"] = Json::Value(Json::objectValue);
    rjson["name_info"]["source"] = Json::Value(Json::arrayValue);
    rjson["name_info"]["target"] = Json::Value(Json::arrayValue);
    //source user name
    switch (ptbAlAction->m_nMclass)
    {
    case EN_ACTION_MAIN_CLASS__BUILDING:
        rjson["name_info"]["source"].append(ptbAlAction->m_bParam.m_astList[0].m_stBuilding.m_szUserName);
        break;
    case EN_ACTION_MAIN_CLASS__TRAIN_NEW:
        rjson["name_info"]["source"].append(ptbAlAction->m_bParam.m_astList[0].m_stTrain.m_szUserName);
        break;
    case EN_ACTION_MAIN_CLASS__EQUIP:
        rjson["name_info"]["source"].append(ptbAlAction->m_bParam.m_astList[0].m_stEquip.m_szUserName);
        break;
    default:
        rjson["name_info"]["source"].append("");
        break;
    }
    //source city name
    switch (ptbAlAction->m_nMclass)
    {
    default:
        rjson["name_info"]["source"].append("");
        break;
    }
    //source alliance name
    switch (ptbAlAction->m_nMclass)
    {
    default:
        rjson["name_info"]["source"].append("");
        break;
    }
    //sorce nick name
    switch (ptbAlAction->m_nMclass)
    {
    default:
        rjson["name_info"]["source"].append("");
        break;
    }

    //target user name
    switch (ptbAlAction->m_nMclass)
    {
    default:
        rjson["name_info"]["target"].append("");
        break;
    }
    //target city name
    switch (ptbAlAction->m_nMclass)
    {
    default:
        rjson["name_info"]["target"].append("");
        break;
    }
    //target alliance name
    switch (ptbAlAction->m_nMclass)
    {
    default:
        rjson["name_info"]["target"].append("");
        break;
    }
    //target nick name
    switch (ptbAlAction->m_nMclass)
    {
    default:
        rjson["name_info"]["target"].append("");
        break;
    }

    // 基于客户端class进行区分
    //param
    switch (ptbAlAction->m_nSclass)
    {
    case EN_ACTION_SEC_CLASS__BUILDING_UPGRADE:
    case EN_ACTION_SEC_CLASS__BUILDING_REMOVE:
    case EN_ACTION_SEC_CLASS__REMOVE_OBSTACLE:
        rjson["param"] = Json::Value(Json::objectValue);
        rjson["param"]["pos"] = ptbAlAction->m_bParam[0].m_stBuilding.m_ddwPos;
        rjson["param"]["id"] = ptbAlAction->m_bParam[0].m_stBuilding.m_ddwType;
        rjson["param"]["target_lv"] = ptbAlAction->m_bParam[0].m_stBuilding.m_ddwTargetLevel;
        break;
    case EN_ACTION_SEC_CLASS__RESEARCH_UPGRADE:
        rjson["param"] = Json::Value(Json::objectValue);
        rjson["param"]["id"] = ptbAlAction->m_bParam[0].m_stBuilding.m_ddwType;
        rjson["param"]["target_lv"] = ptbAlAction->m_bParam[0].m_stBuilding.m_ddwTargetLevel;
        break;
    case EN_ACTION_SEC_CLASS__TROOP:
    case EN_ACTION_SEC_CLASS__FORT:
    case EN_ACTION_SEC_CLASS__HOS_TREAT:
    case EN_ACTION_SEC_CLASS__FORT_REPAIR:
        rjson["param"] = Json::Value(Json::objectValue);
        rjson["param"]["id"] = ptbAlAction->m_bParam[0].m_stTrain.m_ddwType;
        rjson["param"]["num"] = ptbAlAction->m_bParam[0].m_stTrain.m_ddwNum;
        break;
    case EN_ACTION_SEC_CLASS__EQUIP_UPGRADE:
        rjson["param"] = Json::Value(Json::objectValue);
        rjson["param"]["id"] = ptbAlAction->m_bParam[0].m_stEquip.m_uddwId;
        rjson["param"]["eid"] = ptbAlAction->m_bParam[0].m_stEquip.m_ddwEType;
        rjson["param"]["material"] = ptbAlAction->m_bParam[0].m_stEquip.m_szMaterialIdList;
        rjson["param"]["scroll"] = ptbAlAction->m_bParam[0].m_stEquip.m_ddwScrollId;
        rjson["param"]["level"] = ptbAlAction->m_bParam[0].m_stEquip.m_ddwLevel;
        rjson["param"]["soul"] = 0;
        rjson["param"]["part"] = "";
        rjson["param"]["is_mistery"] = 0;
        break;
    default:
        rjson["param"] = Json::Value(Json::objectValue);
        break;
    }

    rjson["map"] = Json::Value(Json::arrayValue);
    rjson["map"].append(1u);
    rjson["map"].append(1u);

    rjson["can_recall"] = FALSE;
    rjson["seq"] = ptbAlAction->m_nSeq;
}

TVOID CCommJson::GenMapBaseJson(TbMap* ptbMap, Json::Value& rJson)
{
    /*
    [
        int, //pos
        int, //bid
        int, //type
        int, //level
        int, //status 同player status
        long, //force
        long, //force_kill
        long, //uid
        string, //player name
        int, //alid
        string, //alliance_name
        string, //城市名
        int, // 资源剩余比例 万分之一 5000表示剩余50% 计算单一资源(例如gem)数量时 采用进一法... 0代表未拉取(满资源)[int]
        int, // wild class type EWildClass
        int, // player avatar
        int, //是否中心块 1:是 2:表示某种类型的地块的非中心快 0:普通地
        string, //al_nick
        int, //block num
        int, //alliance flag
		long, //攻击后冒烟结束时间
		int, // monster_life_left
		long, //地形结束时间
        int，//服务器id，svr_id
        string, //服务器名，svr_name
        int, //0表示监狱为空, 1表示监狱里有英雄
		long, //清空后冒火结束时间
        int, //reinforce数量限制
        int, //当前reinforce数量
		int, //age
		int, //vip lv
		long, //vip end time
		int, //al pos
        int, //税率档id,仅王座有效 具体税率值见game.json
        int, //user lord level
        long, //protect end time
        int, //center pos 当其不是中心块时使用
        int //是否移城, 判断该城市是否将要被动移城
        int, //如果当前地块是巢穴，正在拉取资源的用户数量
        int, //是否camp
        int, //可接受支援兵上线
        int, //已支援兵数量
    ]
    */
    const Json::Value& oWildResJson = CWildInfo::GetWildResInfo(ptbMap->m_nSid);

    TINT64 ddwStatus = ptbMap->m_nStatus;
    TUINT32 udwCurtime = CTimeUtils::GetUnixTime();
    TINT64 ddwPeatimeEnd = ptbMap->m_nTime_end;

    if((ddwStatus & EN_CITY_STATUS__AVOID_WAR)
        && ptbMap->m_nType != EN_WILD_TYPE__IDOL
        && ptbMap->m_nType != EN_WILD_TYPE__THRONE_NEW)
    {
        if (ptbMap->m_nTime_end < udwCurtime)
        {
            ddwStatus &= (~EN_CITY_STATUS__AVOID_WAR);
        }
        ddwPeatimeEnd = ptbMap->m_nTime_end;
    }

    rJson = Json::Value(Json::arrayValue);
    rJson[0U] = ptbMap->m_nId;
    rJson[1U] = ptbMap->m_nBid;
    rJson[2U] = ptbMap->m_nType;
    rJson[3U] = ptbMap->m_nLevel;
    rJson[4U] = ddwStatus;
    rJson[5U] = ptbMap->m_nMight;
    rJson[6U] = ptbMap->m_nForce_kill;
    rJson[7U] = ptbMap->m_nUid;
    rJson[8U] = ptbMap->m_sUname;
    rJson[9U] = ptbMap->m_nAlid;
    rJson[10U] = ptbMap->m_sAlname;
    rJson[11U] = ptbMap->m_sName;
    rJson[12U] = ptbMap->m_nReward_left;

    TUINT32 udwWildClass = CMapLogic::GetWildClass(ptbMap->m_nSid, ptbMap->m_nType);
    
    rJson[13U] = udwWildClass;
    rJson[14U] = ptbMap->m_nAvatar;
    rJson[15U] = ptbMap->m_nPic_index;
    rJson[16U] = ptbMap->m_sAl_nick;
    rJson[17U] = CMapBase::GetWildBlockNumByType(ptbMap->m_nSid, ptbMap->m_nType);
    rJson[18U] = ptbMap->m_nAl_flag;
    rJson[19U] = ptbMap->m_nSmoke_end_time;
    rJson[20U] = ptbMap->m_nBoss_life;
    if (oWildResJson.isMember(CCommonFunc::NumToString(ptbMap->m_nType))
        && (oWildResJson[CCommonFunc::NumToString(ptbMap->m_nType)]["a0"]["a0"].asUInt() == EN_WILD_CLASS_MONSTER
        || oWildResJson[CCommonFunc::NumToString(ptbMap->m_nType)]["a0"]["a0"].asUInt() == EN_WILD_CLASS_LEADER_MONSTER
        || oWildResJson[CCommonFunc::NumToString(ptbMap->m_nType)]["a0"]["a0"].asUInt() == EN_WILD_CLASS_MONSTER_NEST))
    {
        rJson[21U] = ptbMap->m_nExpire_time;
    }
    else
    {
        rJson[21U] = ptbMap->m_nMarch_status_time;
    }
    rJson[22U] = ptbMap->m_nSid;
    rJson[23U] = CGameSvrInfo::GetInstance()->GetSvrNameBySid(ptbMap->m_nSid);
    rJson[24U] = ptbMap->m_nPrison_flag;
    rJson[25U] = ptbMap->m_nBurn_end_time; 
    if (ptbMap->m_jCity_info.empty())
    {
        rJson[26U] = 0;
        rJson[27U] = 0;
    }
    else
    {
        rJson[26U] = ptbMap->m_jCity_info["reinforced_limit"].asInt();
        rJson[27U] = ptbMap->m_jCity_info["reinforced_num"].asInt();
    }
    rJson[28U] = ptbMap->m_nAge;
    rJson[29U] = ptbMap->m_nVip_level;
    rJson[30U] = ptbMap->m_nVip_etime;
    rJson[31U] = ptbMap->m_nAl_pos;
    rJson[32U] = ptbMap->m_nTax_rate_id;
    rJson[33U] = ptbMap->m_nUlevel;

    rJson[34U] = ddwPeatimeEnd;

    rJson[35U] = ptbMap->m_nCenter_pos;

    rJson[36U] = ptbMap->m_nMove_city;

    rJson[37U] = ptbMap->m_nOccupy_num;
    // jonathan @20160728 预计去掉的字段，兼容线下旧客户端
    rJson[38U] = 0;
    if (ptbMap->m_jCity_info.empty())
    {
        rJson[39U] = 0;
        rJson[40U] = 0;
    }
    else
    {
        rJson[39U] = ptbMap->m_jCity_info["reinforced_troop_limit"].asInt();
        rJson[40U] = ptbMap->m_jCity_info["reinforced_troop_num"].asInt();
    }
}

TVOID CCommJson::GenEventTipsJson(SUserInfo* pstUserInfo, Json::Value& rjsonResult)
{
    //"svr_event_reward_window":
    //[
    //    {
    //      event_type: //event type
    //      reward_type : //(0:goal 1:rank)
    //      num : //goal_num or rank_num 
    //      score_get : //玩家获取的分数
    //      score_list : [1000, 2000, 5000] //活动分数分布
    //      reward :
    //      [
    //          [
    //                //type,
    //                //id
    //                //num
    //          ]
    //      ]
    //      mail:
    //      {
    //          //id,
    //          //suid,
    //      }
    //    }
    //]

    SCityInfo *pstCity = &pstUserInfo->m_stCityInfo;
    if (pstCity == NULL)
    {
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("Processcmd_AddRewardList: city is null [seq=%u]", pstUserInfo->m_udwBSeqNo));
        return;
    }

    TUINT32 udwCastle = CCityBase::GetBuildingLevelById(&pstCity->m_stTblData, 3);
    //wave@20160414:age 屏蔽
    /*
    if(pstUserInfo->m_tbPlayer.m_nAge < 3 )
    {
    return;
    }
    */
    //if (udwCastle < 3)
    //{
    //    return;
    //}

    Json::Value &jEventRewardWindow = rjsonResult["svr_event_reward_window"];
    jEventRewardWindow = Json::Value(Json::arrayValue);

    TUINT32 udwLastLgTime = pstUserInfo->m_tbLogin.m_nLast_lg_time;
    TINT64 dwLastWinId = pstUserInfo->m_tbUserStat.m_nLast_event_win_id;
    TINT32 dwResGoal = -1;
    TINT32 dwIdxGoal = -1;
    TINT32 dwIdxRank = -1;

    TINT32 dwResThGoal = -1;
    TINT32 dwIdxThGoal = -1;
    TINT32 dwIdxThRank = -1;

    for (TUINT32 udwIdx = 0; udwIdx < pstUserInfo->m_udwEventTipsNum; ++udwIdx)
    {
        TUINT32 udwTipsEventType = pstUserInfo->m_atbEventTips[udwIdx].m_jContent["event_type"].asUInt();
        TUINT32 udwTipsRewardType = pstUserInfo->m_atbEventTips[udwIdx].m_jContent["reward_type"].asUInt();
        TUINT32 udwTipsTheme = pstUserInfo->m_atbEventTips[udwIdx].m_jContent["theme_event"].asUInt();
        TUINT32 udwEventEndTime = pstUserInfo->m_atbEventTips[udwIdx].m_nEvent_end_time;

        TUINT32 udwTime = pstUserInfo->m_atbEventTips[udwIdx].m_nTime;
        TINT64 dwId = pstUserInfo->m_atbEventTips[udwIdx].m_nId;

        TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("GenEventTipsJson: [time=%u] [last_login=%u] [id=%ld] [last_win=%ld] [seq=%u]", udwTime, udwLastLgTime, dwId, dwLastWinId, pstUserInfo->m_udwBSeqNo));
        
        if (udwTipsEventType != 1 && udwTipsEventType != 2 && udwTipsTheme != 1)
        {
            continue;
        }
        else
        {
            TINT32 dwCurGoal = pstUserInfo->m_atbEventTips[udwIdx].m_jContent["num"].asUInt();
            TUINT32 udwEventTipsGenTime = pstUserInfo->m_atbEventTips[udwIdx].m_nTime;
            if (udwTipsTheme != 1)
            {
                if (udwTipsRewardType == 0)
                {
                    //普通活动goal，达成了弹，离线达成不弹
                    if (udwEventEndTime < udwLastLgTime || udwTime < udwLastLgTime)
                    {
                        continue;
                    }
                    if (dwCurGoal > dwResGoal)
                    {
                        dwResGoal = dwCurGoal;
                        dwIdxGoal = udwIdx;;
                    }
                }
                else
                {
                    //普通活动rank，达成rank时在线，就弹
                    if (udwTime < udwLastLgTime)
                    {
                        continue;
                    }
                    dwIdxRank = udwIdx;
                }
            }
            else
            {
                if (udwTipsRewardType == 0)
                {
                    //主题活动goal，与普通活动一致
                    if (udwEventEndTime < udwLastLgTime || udwTime < udwLastLgTime)
                    {
                        continue;
                    }
                    if (dwCurGoal > dwResThGoal)
                    {
                        dwResThGoal = dwCurGoal;
                        dwIdxThGoal = udwIdx;
                    }
                }
                else
                {
                    //主题活动rank，什么情况都弹
                    dwIdxThRank = udwIdx;
                }
            }
        }
    }
    if (dwIdxRank >= 0)
    {
        jEventRewardWindow.append(pstUserInfo->m_atbEventTips[dwIdxRank].m_jContent);
    }
    else if (dwIdxGoal >= 0)
    {
        jEventRewardWindow.append(pstUserInfo->m_atbEventTips[dwIdxGoal].m_jContent);
    }
    if (dwIdxThRank >= 0)
    {
        jEventRewardWindow.append(pstUserInfo->m_atbEventTips[dwIdxThRank].m_jContent);
    }
    else if (dwIdxThGoal >= 0)
    {
        jEventRewardWindow.append(pstUserInfo->m_atbEventTips[dwIdxThGoal].m_jContent);
    }
    //for log test
    Json::FastWriter writer;
    writer.omitEndingLineFeed();
    string strEventReward = writer.write(jEventRewardWindow);
    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("GenEventTipsJson: [event num=%u] [Event Reward Window = %s] [seq=%u]", pstUserInfo->m_udwEventTipsNum, strEventReward.c_str(), pstUserInfo->m_udwBSeqNo));
}

vector<SEventPriority> CCommJson::GetSortEventTipsList(TbEvent_tips *atbEventTipsList,TUINT32 udwNum)
{
    vector<SEventPriority> vEventTips;
    vEventTips.clear();

    for(TUINT32 udwIdx = 0; udwIdx < udwNum;++udwIdx)
    {
        SEventPriority sEventPriority;
        sEventPriority.Reset();

        sEventPriority.uddwEventId = atbEventTipsList[udwIdx].m_jContent["event_id"].asUInt64();
        sEventPriority.ddwEventType = atbEventTipsList[udwIdx].m_jContent["event_type"].asUInt();
        sEventPriority.ddwRewardType = atbEventTipsList[udwIdx].m_jContent["reward_type"].asUInt();
        sEventPriority.ddwKey = atbEventTipsList[udwIdx].m_jContent["num"].asUInt();
        sEventPriority.ddwIdx = udwIdx;

        vEventTips.push_back(sEventPriority);
    }
    sort(vEventTips.begin(), vEventTips.end(), CCommJson::CompareEventTips);
    return vEventTips;
}

TBOOL CCommJson::CompareEventTips(SEventPriority sEventTipsA, SEventPriority sEventTipsB)
{
    TUINT32 udwARankType = CCommJson::GetEventTipsPriority(sEventTipsA);
    TUINT32 udwBRankType = CCommJson::GetEventTipsPriority(sEventTipsB);

    if(udwARankType > udwBRankType)
    {
        TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("CompareEventTips: [event_id_A=%lld event_id_B=%lld] [rankType_A=%u rankType_B=%u] return true",
            sEventTipsA.uddwEventId,
            sEventTipsB.uddwEventId,
            udwARankType,
            udwBRankType));

        return TRUE;
    }
    else if(udwARankType == udwBRankType)
    {
        if(sEventTipsA.uddwEventId < sEventTipsB.uddwEventId)
        {
            return TRUE;
        }
        else if(sEventTipsA.uddwEventId == sEventTipsB.uddwEventId)
        {
            if(sEventTipsA.ddwKey < sEventTipsB.ddwKey)
            {
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("CompareEventTips: [event_id_A=%lld event_id_B=%lld] [rankType_A=%u rankType_B=%u] return false",
            sEventTipsA.uddwEventId,
            sEventTipsB.uddwEventId,
            udwARankType,
            udwBRankType));

        return FALSE;
    }
    return FALSE;
}

TUINT32 CCommJson::GetEventTipsPriority(SEventPriority sEventTips)
{
    if(sEventTips.ddwEventType == EN_EVENT_TYPE__PERSONAL_1 ||
        sEventTips.ddwEventType == EN_EVENT_TYPE__PERSONAL_2)
    {
        if(sEventTips.ddwRewardType == 0)
        {
            //最高优先级
            return 1000;
        }
        else
        {
            //第三优先级
            return 500;
        }
    }
    if(sEventTips.ddwEventType == EN_EVENT_TYPE__ALLIANCE)
    {
        if(sEventTips.ddwEventType == 0)
        {
            //第二优先级
            return 750;
        }
        else
        {
            //最低优先级
            return 250;
        }
    }
    return 0;
}


TVOID CCommJson::GenBountyInfo(SUserInfo *pstUser ,TUINT32 udwCid,Json::Value& rJson)
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
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    if(pstCity == NULL)
    {
        return;
    }
    TbBounty *pstBounty = &pstUser->m_tbBounty;
    Json::Value& jsonBounty = rJson["svr_bounty"];
    jsonBounty = Json::Value(Json::objectValue);

    //daemon TODO svr定制功能使用 方案还没有给出
    TBOOL bOpen = CCommonLogic::CheckFuncOpen(pstUser, pstCity, EN_FUNC_OPEN_TYPE_BOUNTY);
    jsonBounty["has_bounty"] = bOpen ? 1 : 0;

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
    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_vTmpFinishBountyBase.size(); ++udwIdx)
    {
        Json::Value &jIdxNodeReward = jsonBountyFinishNode[udwIdx];
        jIdxNodeReward = Json::Value(Json::arrayValue);
        jIdxNodeReward.append(pstUser->m_vTmpFinishBountyBase[udwIdx].first);
        jIdxNodeReward.append(pstUser->m_vTmpFinishBountyBase[udwIdx].second);
    }

    Json::Value &jsonBountyFinish = jsonBounty["finish_bounty"];
    jsonBountyFinish = Json::Value(Json::arrayValue);
    if(pstUser->m_udwTmpFinishBountyGoal != 0)
    {
        jsonBountyFinish.append(pstUser->m_udwTmpFinishBountyGoal);
    }
}

TVOID CCommJson::GenTaskInfo(SUserInfo *pstUser, Json::Value& rJson)
{
    /*
    "svr_task":
        {
            "finished_task" : [ //刚刚完成的task id, 客户端用于展示弹窗
                int,        //task id    
                int
            ]
            "doing_task": {  // 进行中的task
                stirng : {   //key是task id
                    "basic": 
					[
                        int,    //进度最近更新时间
                        int,    //0表示非新增,1表示新增,用于客户端展示
                        int,    //0表示无进度,1表示有进度,用于客户端展示
                        int     //结束时间, 0表示不限时
                    ],
                    "progress":
					[  //idx跟game.json对应
						int  //目标值当前数量
					],
                    "can_finish":
                    [ //idx跟game.json对应
                        int //0不可以继续完成， 1可以继续完成
                    ],
					"end_time":
					[  //idx跟game.json对应
						int //条件进度结束时间
					]
            }
            "recent_update": int  //最近更新task id,可以在doing_task中找到, 用于客户端
            "next_refresh_time": int//下次更新限时任务的时间
            "display_order":[123, 456]//doing_task 中已经排序的id  recommed为0  v1.1
            "overall_status": int//0无 1加 2勾    v1.1
			"check_task_id": int //打勾详情id     v1.1
        }
    }
    */
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TbTask *ptbTask = &pstUser->m_tbTask;
    TUINT32 udwNewestTaskId = 0;
    TUINT32 udwNewestTime = 0;

    Json::Value& rTaskJson = rJson["svr_task"];
    rTaskJson = Json::Value(Json::objectValue);

    //finished_task
    Json::Value& rFinishTaskJson = rTaskJson["finished_task"];
    rFinishTaskJson = Json::Value(Json::arrayValue);
    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwTmpFinishNum; ++udwIdx)
    {
        TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("TaskFinish:CheckTaskFinish:uid=%u finish task_id=%u [tmp_finish_num=%u finish_id=%u] [seq=%u]",
            pstUser->m_tbPlayer.m_nUid,
            pstUser->m_audwTmpFinishTaskId[udwIdx],
            pstUser->m_udwTmpFinishNum,
            pstUser->m_audwTmpFinishTaskId[udwIdx],
            pstUser->m_udwBSeqNo));

        //新手教学已经fake 的数据不返回 都是george的错
        if (pstUser->m_audwTmpFinishTaskId[udwIdx] == 1 ||
            pstUser->m_audwTmpFinishTaskId[udwIdx] == 6 ||
            pstUser->m_audwTmpFinishTaskId[udwIdx] == 36 ||
            pstUser->m_audwTmpFinishTaskId[udwIdx] == 37)
        {
            continue;
        }
        rFinishTaskJson.append(pstUser->m_audwTmpFinishTaskId[udwIdx]);
    }

    //doing_task
    Json::Value& rDoingTaskJson = rTaskJson["doing_task"];
    rDoingTaskJson = Json::Value(Json::objectValue);

    //time task
    for (TUINT32 udwIdx = 0; udwIdx < ptbTask->m_bTask_time.m_udwNum; ++udwIdx)
    {
        TUINT32 udwTaskId = ptbTask->m_bTask_time[udwIdx].m_ddwId;
        if (udwTaskId == 0)
        {
            continue;
        }
        Json::Value& rTaskIdInfo = rDoingTaskJson[CCommonFunc::NumToString(udwTaskId)];
        rTaskIdInfo = Json::Value(Json::objectValue);

        //basic
        Json::Value& rTaskIdBaseInfo = rTaskIdInfo["basic"];
        rTaskIdBaseInfo = Json::Value(Json::arrayValue);

        rTaskIdBaseInfo.append(ptbTask->m_bTask_time[udwIdx].m_ddwUpdateTime);
        rTaskIdBaseInfo.append(ptbTask->m_bTask_time[udwIdx].m_bIsNew);
        rTaskIdBaseInfo.append(ptbTask->m_bTask_time[udwIdx].m_bIsProgress);
        rTaskIdBaseInfo.append(ptbTask->m_bTask_time[udwIdx].m_ddwEndTime);

        //progress
        Json::Value& rTaskIdProgressInfo = rTaskIdInfo["progress"];
        rTaskIdProgressInfo = Json::Value(Json::arrayValue);

        for (TUINT32 udwCondiIdx = 0; udwCondiIdx < MAX_TASK_CONDITION_LIMIT; ++udwCondiIdx)
        {
            rTaskIdProgressInfo.append(ptbTask->m_bTask_time[udwIdx].astFinishCondition[udwCondiIdx].m_uddwCurrValue);
        }

        //progress
        Json::Value& rTaskCanFinish = rTaskIdInfo["can_finish"];
        rTaskCanFinish = Json::Value(Json::arrayValue);

        Json::Value& rTaskIdEndTime = rTaskIdInfo["end_time"];
        rTaskIdEndTime = Json::Value(Json::arrayValue);

        for (TUINT32 udwCondiIdx = 0; udwCondiIdx < MAX_TASK_CONDITION_LIMIT; ++udwCondiIdx)
        {
            TUINT32 udwConditionType = ptbTask->m_bTask_time[udwIdx].astFinishCondition[udwCondiIdx].m_ddwTaskType;
            TUINT32 udwConditionId = ptbTask->m_bTask_time[udwIdx].astFinishCondition[udwCondiIdx].m_ddwId;
            TUINT32 udwConditionValue = ptbTask->m_bTask_time[udwIdx].astFinishCondition[udwCondiIdx].m_ddwValue;

            TUINT32 udwTime = CQuestLogic::GetTaskTypeEndTime(pstUser, udwConditionType, udwConditionId, udwConditionValue);
            rTaskIdEndTime.append(udwTime);
            rTaskCanFinish.append(CQuestLogic::IsTaskCanStart(pstUser, pstCity, udwConditionType, udwConditionId, udwConditionValue));

        }

        if (ptbTask->m_bTask_time[udwIdx].m_ddwUpdateTime > udwNewestTime)
        {
            udwNewestTime = ptbTask->m_bTask_time[udwIdx].m_ddwUpdateTime;
            udwNewestTaskId = udwTaskId;
        }
    }
    //normal task
    for (TUINT32 udwIdx = 0; udwIdx < ptbTask->m_bTask_normal.m_udwNum; ++udwIdx)
    {
        TUINT32 udwTaskId = ptbTask->m_bTask_normal[udwIdx].m_ddwId;
        if (udwTaskId == 0)
        {
            continue;
        }
        Json::Value& rTaskIdInfo = rDoingTaskJson[CCommonFunc::NumToString(udwTaskId)];
        rTaskIdInfo = Json::Value(Json::objectValue);

        //basic
        Json::Value& rTaskIdBaseInfo = rTaskIdInfo["basic"];
        rTaskIdBaseInfo = Json::Value(Json::arrayValue);

        rTaskIdBaseInfo.append(ptbTask->m_bTask_normal[udwIdx].m_ddwUpdateTime);
        rTaskIdBaseInfo.append(ptbTask->m_bTask_normal[udwIdx].m_bIsNew);
        rTaskIdBaseInfo.append(ptbTask->m_bTask_normal[udwIdx].m_bIsProgress);
        rTaskIdBaseInfo.append(0);

        //progress
        Json::Value& rTaskIdProgressInfo = rTaskIdInfo["progress"];
        rTaskIdProgressInfo = Json::Value(Json::arrayValue);

        //progress
        Json::Value& rTaskCanFinish = rTaskIdInfo["can_finish"];
        rTaskCanFinish = Json::Value(Json::arrayValue);

        for (TUINT32 udwCondiIdx = 0; udwCondiIdx < MAX_TASK_CONDITION_LIMIT; ++udwCondiIdx)
        {
            rTaskIdProgressInfo.append(ptbTask->m_bTask_normal[udwIdx].astFinishCondition[udwCondiIdx].m_uddwCurrValue);
        }
        Json::Value& rTaskIdEndTime = rTaskIdInfo["end_time"];
        rTaskIdEndTime = Json::Value(Json::arrayValue);

        for (TUINT32 udwCondiIdx = 0; udwCondiIdx < MAX_TASK_CONDITION_LIMIT; ++udwCondiIdx)
        {
            TUINT32 udwConditionType = ptbTask->m_bTask_normal[udwIdx].astFinishCondition[udwCondiIdx].m_ddwTaskType;
            TUINT32 udwConditionId = ptbTask->m_bTask_normal[udwIdx].astFinishCondition[udwCondiIdx].m_ddwId;
            TUINT32 udwConditionValue = ptbTask->m_bTask_normal[udwIdx].astFinishCondition[udwCondiIdx].m_ddwValue;

            TUINT32 udwTime = CQuestLogic::GetTaskTypeEndTime(pstUser, udwConditionType, udwConditionId, udwConditionValue);
            rTaskIdEndTime.append(udwTime);
            rTaskCanFinish.append(CQuestLogic::IsTaskCanStart(pstUser, pstCity, udwConditionType, udwConditionId, udwConditionValue));
        }

        if (ptbTask->m_bTask_normal[udwIdx].m_ddwUpdateTime > udwNewestTime)
        {
            udwNewestTime = ptbTask->m_bTask_normal[udwIdx].m_ddwUpdateTime;
            udwNewestTaskId = udwTaskId;
        }
    }

    //recent_update
    rTaskJson["recent_update"] = udwNewestTaskId;

    //next_refresh_time
    rTaskJson["next_refresh_time"] = (1 + CTimeUtils::GetUnixTime() / (24 * 3600)) * 24 * 3600;

    //overall_status
    if (ptbTask->m_nTask_status <= EN_TASK_SHOW_STATUS_ADD)
    {
        rTaskJson["overall_status"] = EN_TASK_SHOW_STATUS_NORMAL;
    }
    else
    {
        rTaskJson["overall_status"] = ptbTask->m_nTask_status;
    }
    rTaskJson["force_update"] = pstUser->m_bTaskUpdate;

    //display_order
    Json::Value& rDisplayOrderJson = rTaskJson["display_order"];
    rDisplayOrderJson = Json::Value(Json::arrayValue);

    for (vector<TUINT32>::iterator it = pstUser->m_vecTaskList.begin(); it < pstUser->m_vecTaskList.end(); ++it)
    {
        rDisplayOrderJson.append(*it);
    }

    //check_task_id
    rTaskJson["check_task_id"] = ptbTask->m_nTask_check_id;

    //TODO 不知道干啥的...
    Json::Value &rFinishJson = rTaskJson["test_finish"];
    rFinishJson = Json::Value(Json::arrayValue);
    for (TUINT32 udwIdx = 0; udwIdx < EN_TOP_QUEST_NUM_LIMIT; ++udwIdx)
    {
        if (BITTEST(ptbTask->m_bTask_finish[0].m_bitTask, udwIdx))
        {
            rFinishJson.append(udwIdx);
        }
    }
}

TVOID CCommJson::GenSkillInfo(SSkill *pstSkill, Json::Value& rJson)
{
    /*
    {
        "0"(int): int // "skill_id": lv
    }
    */
    rJson = Json::Value(Json::objectValue);
    for (TUINT32 udwIdx = 0; udwIdx < EN_SKILL_TYPE__END; ++udwIdx)
    {
        if (pstSkill->m_addwLevel[udwIdx] == 0)
        {
            continue;
        }
        rJson[CCommonFunc::NumToString(udwIdx)] = pstSkill->m_addwLevel[udwIdx];
    }
}

TBOOL CCommJson::IfTableDataNeedOutput( TINT32 dwType, AwsTable* ptbTbl, TINT32 dwItemUpdtType)
{
    if(dwItemUpdtType == EN_TABLE_UPDT_FLAG__DEL)
    {
        if(dwType != EN_CONTENT_UPDATE_TYPE__ALL && dwType != EN_CONTENT_UPDATE_TYPE__TABLE_INC)
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        if(dwType == EN_CONTENT_UPDATE_TYPE__ITEM_INC && ptbTbl->IfNeedUpdate() == FALSE)
        {
            return FALSE;
        }
    }
    return TRUE;
}

TVOID CCommJson::GenActionInfoForPush( TbAction* ptbAction, Json::Value& rjson, TINT32 dwItemUpdtType /*= EN_TABLE_UPDT_FLAG__CHANGE*/ )
{
    // action
    string key = CCommonFunc::NumToString(ptbAction->m_nId);
    if(dwItemUpdtType == EN_TABLE_UPDT_FLAG__DEL)
    {
        rjson[key] = Json::Value(Json::nullValue);
    }
    else
    {
        ptbAction->m_nSeq++;
        CCommJson::GenActionInfo(ptbAction, rjson[key]);
        ptbAction->m_nSeq--;
    } 
}

TVOID CCommJson::GenMarchInfoForPush( TbMarch_action* ptbAction, Json::Value& rjson, TINT32 dwItemUpdtType /*= EN_TABLE_UPDT_FLAG__CHANGE*/ )
{
    // action
    string key = CCommonFunc::NumToString(ptbAction->m_nId);
    if(dwItemUpdtType == EN_TABLE_UPDT_FLAG__DEL)
    {
        rjson[key] = Json::Value(Json::nullValue);
    }
    else
    {
        ptbAction->m_nSeq++;
        CCommJson::GenMarchInfo(ptbAction, rjson[key]);
        ptbAction->m_nSeq--;
    } 
}

TVOID CCommJson::GenAlActionInfoForPush( TbAlliance_action* ptbAction, Json::Value& rjson, TINT32 dwItemUpdtType /*= EN_TABLE_UPDT_FLAG__CHANGE*/ )
{
    // action
    string key = CCommonFunc::NumToString(ptbAction->m_nId);
    if(dwItemUpdtType == EN_TABLE_UPDT_FLAG__DEL)
    {
        rjson[key] = Json::Value(Json::nullValue);
    }
    else
    {
        ptbAction->m_nSeq++;
        CCommJson::GenAlActionInfo(ptbAction, rjson[key]);
        ptbAction->m_nSeq--;
    } 
}

TVOID CCommJson::GenReportBufferInfo(SReportBuffer *pstBuff, Json::Value& rJson)
{
    rJson = Json::Value(Json::objectValue);

    rJson["dragon"] = Json::Value(Json::arrayValue);
    for (TUINT32 udwIdx = 0; udwIdx < pstBuff->udwDragonBuffNum; ++udwIdx)
    {
        Json::Value rBuff = Json::Value(Json::arrayValue);
        rBuff[0U] = pstBuff->astDragonBuff[udwIdx].ddwBuffId;
        rBuff[1U] = pstBuff->astDragonBuff[udwIdx].ddwBuffNum;
        rJson["dragon"].append(rBuff);
    }

    rJson["knight"] = Json::Value(Json::arrayValue);
    for (TUINT32 udwIdx = 0; udwIdx < pstBuff->udwKnightBuffNum; ++udwIdx)
    {
        Json::Value rBuff = Json::Value(Json::arrayValue);
        rBuff[0U] = pstBuff->astKnightBuff[udwIdx].ddwBuffId;
        rBuff[1U] = pstBuff->astKnightBuff[udwIdx].ddwBuffNum;
        rJson["knight"].append(rBuff);
    }

    rJson["research"] = Json::Value(Json::arrayValue);
    for (TUINT32 udwIdx = 0; udwIdx < pstBuff->udwResearchBuffNum; ++udwIdx)
    {
        Json::Value rBuff = Json::Value(Json::arrayValue);
        rBuff[0U] = pstBuff->astResearchBuff[udwIdx].ddwBuffId;
        rBuff[1U] = pstBuff->astResearchBuff[udwIdx].ddwBuffNum;
        rJson["research"].append(rBuff);
    }
}

TVOID CCommJson::GenBattleBuffInfo(SPlayerBuffInfo *pstBuff, Json::Value& rJson)
{
    rJson = Json::Value(Json::objectValue);

    rJson["ground"] = Json::Value(Json::objectValue);
    rJson["ground"]["research"] = Json::Value(Json::arrayValue);
    rJson["ground"]["dragon"] = Json::Value(Json::arrayValue);
    rJson["ground"]["equipment"] = Json::Value(Json::arrayValue);
    rJson["ground"]["others"] = Json::Value(Json::arrayValue);
    rJson["ranged"] = Json::Value(Json::objectValue);
    rJson["ranged"]["research"] = Json::Value(Json::arrayValue);
    rJson["ranged"]["dragon"] = Json::Value(Json::arrayValue);
    rJson["ranged"]["equipment"] = Json::Value(Json::arrayValue);
    rJson["ranged"]["others"] = Json::Value(Json::arrayValue);
    rJson["mounted"] = Json::Value(Json::objectValue);
    rJson["mounted"]["research"] = Json::Value(Json::arrayValue);
    rJson["mounted"]["dragon"] = Json::Value(Json::arrayValue);
    rJson["mounted"]["equipment"] = Json::Value(Json::arrayValue);
    rJson["mounted"]["others"] = Json::Value(Json::arrayValue);
    rJson["siege"] = Json::Value(Json::objectValue);
    rJson["siege"]["research"] = Json::Value(Json::arrayValue);
    rJson["siege"]["dragon"] = Json::Value(Json::arrayValue);
    rJson["siege"]["equipment"] = Json::Value(Json::arrayValue);
    rJson["siege"]["others"] = Json::Value(Json::arrayValue);

    TUINT32 udwBuffId = 0;
    TINT32 dwResearchBuffNum = 0;
    TINT32 dwDragonBuffNum = 0;
    TINT32 dwEquipmentBuffNum = 0;
    TINT32 dwOtherBuffNum = 0;
    string szKey;
    for (TUINT32 udwCategoryIdx = 0; udwCategoryIdx < EN_TROOP_CATEGORY__END; udwCategoryIdx++)
    {
        szKey.clear();
        switch (udwCategoryIdx)
        {
        case EN_TROOP_CATEGORY__INFANTRY:
            szKey = "ground";
            break;
        case EN_TROOP_CATEGORY__REMOTE:
            szKey = "ranged";
            break;
        case EN_TROOP_CATEGORY__SOWAR:
            szKey = "mounted";
            break;
        case EN_TROOP_CATEGORY__SIEGE:
            szKey = "siege";
            break;
        }

        if (szKey.empty())
        {
            continue;
        }

        const Json::Value &jTroop = CGameInfo::GetInstance()->m_oJsonRoot["game_troop"];
        TUINT32 udwTroopIdx = 0;
        for (udwTroopIdx = 0; udwTroopIdx < jTroop.size(); ++udwTroopIdx)
        {
            if (jTroop[udwTroopIdx]["a"]["a5"].asUInt() == udwCategoryIdx)
            {
                break;
            }
        }

        Json::Value &jTmp = rJson[szKey];
        for (TUINT32 udwBuffType = 0; udwBuffType < 3; udwBuffType++)
        {
            szKey.clear();
            switch (udwBuffType)
            {
            case 0:
                szKey = "attack";
                break;
            case 1:
                szKey = "life";
                break;
            case 2:
                szKey = "defense";
                break;
            }

            dwResearchBuffNum = 0;
            dwDragonBuffNum = 0;
            dwEquipmentBuffNum = 0;
            dwOtherBuffNum = 0;
            if (!szKey.empty() && udwTroopIdx < jTroop.size())
            {
                for (TUINT32 udwBuffIdx = 0; udwBuffIdx < jTroop[udwTroopIdx]["ab"][szKey].size(); ++udwBuffIdx)
                {
                    udwBuffId = jTroop[udwTroopIdx]["ab"][szKey][udwBuffIdx].asUInt();
                    if (pstBuff->m_astPlayerBuffInfo[udwBuffId].m_udwBuffId == 0)
                    {
                        continue;
                    }
                    dwResearchBuffNum += pstBuff->m_astPlayerBuffInfo[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_RESEARCH].m_ddwNum;
                    dwDragonBuffNum += pstBuff->m_astPlayerBuffInfo[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_DRAGON_SKILL].m_ddwNum +
                        pstBuff->m_astPlayerBuffInfo[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_DRAGON_LV].m_ddwNum +
                        pstBuff->m_astPlayerBuffInfo[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_DRAGON_MONSTER_SKILL].m_ddwNum;
                    dwEquipmentBuffNum += pstBuff->m_astPlayerBuffInfo[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_EQUIP].m_ddwNum;
                    dwOtherBuffNum += pstBuff->m_astPlayerBuffInfo[udwBuffId].m_ddwBuffTotal -
                        pstBuff->m_astPlayerBuffInfo[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_RESEARCH].m_ddwNum -
                        pstBuff->m_astPlayerBuffInfo[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_DRAGON_SKILL].m_ddwNum -
                        pstBuff->m_astPlayerBuffInfo[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_DRAGON_LV].m_ddwNum -
                        pstBuff->m_astPlayerBuffInfo[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_DRAGON_MONSTER_SKILL].m_ddwNum -
                        pstBuff->m_astPlayerBuffInfo[udwBuffId].m_astBuffDetail[EN_BUFF_TYPE_EQUIP].m_ddwNum;
                }
            }
            
            jTmp["research"].append(dwResearchBuffNum);
            jTmp["dragon"].append(dwDragonBuffNum);
            jTmp["equipment"].append(dwEquipmentBuffNum);
            jTmp["others"].append(dwOtherBuffNum);
        }
    }
}

TVOID CCommJson::GenTitleInfo(STitleInfoList *pstTitle, TbThrone *ptbThrone, Json::Value& rJson)
{
    rJson = Json::Value(Json::arrayValue);

    const Json::Value &jTitle = CGameInfo::GetInstance()->m_oJsonRoot["game_title"];

    TUINT32 udwJsonIdx = 0;
    TINT64 ddwCurTime = CTimeUtils::GetUnixTime();
    ostringstream oss;
    if (pstTitle)
    {
        for (TUINT32 udwIdx = 0; udwIdx < pstTitle->udwNum; udwIdx++)
        {
            if (pstTitle->aucFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
            {
                continue;
            }
            oss.str("");
            oss << pstTitle->atbTitle[udwIdx].m_nId;
            TINT32 dwExpireTime = 0;
            if (jTitle.isMember(oss.str()))
            {
                dwExpireTime = jTitle[oss.str()]["time"].asInt();
            }
            if (ptbThrone->m_nOccupy_time >= pstTitle->atbTitle[udwIdx].m_nDub_time
                || ddwCurTime - pstTitle->atbTitle[udwIdx].m_nDub_time > dwExpireTime)
            {
                continue;
            }
            rJson[udwJsonIdx] = Json::Value(Json::arrayValue);
            rJson[udwJsonIdx].append(pstTitle->atbTitle[udwIdx].m_nId);
            rJson[udwJsonIdx].append(pstTitle->atbTitle[udwIdx].m_nUid);
            rJson[udwJsonIdx].append(pstTitle->atbTitle[udwIdx].m_nAlid);
            rJson[udwJsonIdx].append(pstTitle->atbTitle[udwIdx].m_sAlnick);
            rJson[udwJsonIdx].append(pstTitle->atbTitle[udwIdx].m_sName);
            rJson[udwJsonIdx].append(pstTitle->atbTitle[udwIdx].m_nDub_time);
            rJson[udwJsonIdx].append(pstTitle->atbTitle[udwIdx].m_nCid);
            udwJsonIdx++;
        }
    }
    
    if (ptbThrone && ptbThrone->m_nAlid > 0)
    {
        // 国王称号, 默认使用id为1
        rJson[udwJsonIdx] = Json::Value(Json::arrayValue);
        rJson[udwJsonIdx].append(1L);
        rJson[udwJsonIdx].append(ptbThrone->m_nOwner_id);
        rJson[udwJsonIdx].append(ptbThrone->m_nAlid);
        rJson[udwJsonIdx].append("");
        rJson[udwJsonIdx].append("");
        rJson[udwJsonIdx].append(0);
        rJson[udwJsonIdx].append(ptbThrone->m_nOwner_cid);
    }
}

TVOID CCommJson::PushData_GenMapWildJson( TUINT32 udwSvrId, TbMap* ptbMap, Json::Value& rJson )
{
    Json::Value& jsonMapWildJson = rJson["wild_json"];
    if(ptbMap->Get_Type()  <= EN_WILD_TYPE_RES_INIT)
    {
        return ;
    }
    jsonMapWildJson = Json::Value(Json::objectValue);
    TUINT32 udwWildType = ptbMap->Get_Type();
    TUINT32 udwWildlv = ptbMap->Get_Level();

    const Json::Value& oWildResJson = CWildInfo::GetWildResInfo(udwSvrId);

    Json::Value& jsonMapWildJsonBase = jsonMapWildJson["basic"];
    jsonMapWildJsonBase = Json::Value(Json::objectValue);

    const TUINT32 k_max_tmp_len = 1024;
    TCHAR szTmpBuf[k_max_tmp_len];
    snprintf(szTmpBuf, k_max_tmp_len, "%u", udwWildType);

    jsonMapWildJsonBase = oWildResJson[szTmpBuf]["a0"];

    Json::Value& jsonMapWildJsonReward = jsonMapWildJson["reward"];
    jsonMapWildJsonReward = Json::Value(Json::arrayValue);
    jsonMapWildJsonReward = oWildResJson[szTmpBuf]["a1"][udwWildlv-1]["a1"];
    if(oWildResJson[szTmpBuf]["a0"]["a0"].asUInt() == EN_WILD_CLASS_MONSTER_NEST)
    {
        for(TUINT32 udwIdx = 0; udwIdx < oWildResJson[szTmpBuf]["a1"][udwWildlv - 1]["a2"].size();++udwIdx)
        {
            jsonMapWildJsonReward.append(oWildResJson[szTmpBuf]["a1"][udwWildlv - 1]["a2"][udwIdx]);
        }
    }

    Json::Value& jsonMapWildJsonUI = jsonMapWildJson["ui"];
    jsonMapWildJsonUI = Json::Value(Json::objectValue);
    jsonMapWildJsonUI = oWildResJson[szTmpBuf]["a6"];

    Json::Value& jsonMapWildJsonAk = jsonMapWildJson["ak_type"];
    jsonMapWildJsonAk = Json::Value(Json::arrayValue);
    jsonMapWildJsonAk = oWildResJson[szTmpBuf]["a3"];

    Json::Value& jsonMapWildJsonHe = jsonMapWildJson["dragon_energy_cost"];
    jsonMapWildJsonHe = Json::Value(Json::objectValue);
    TUINT32 udwWildClass = CMapLogic::GetWildClass(udwSvrId, udwWildType);
    if(udwWildClass == EN_WILD_CLASS_MONSTER || udwWildClass == EN_WILD_CLASS_LEADER_MONSTER)
    {
        assert(oWildResJson[szTmpBuf]["a10"].size() >= udwWildlv);
        jsonMapWildJsonHe = oWildResJson[szTmpBuf]["a10"][udwWildlv - 1];
    }
    else
    {
        jsonMapWildJsonHe = 0;
    }

    Json::Value& jsonMapWildJsonMc = jsonMapWildJson["monster_conf"];
    jsonMapWildJsonMc = Json::Value(Json::arrayValue);
    if(oWildResJson[szTmpBuf].isMember("a11"))
    {
        jsonMapWildJsonMc = oWildResJson[szTmpBuf]["a11"][udwWildlv - 1];
    }
    else
    {
        jsonMapWildJsonMc.append(0);
        jsonMapWildJsonMc.append(100);
        jsonMapWildJsonMc.append(0);
        jsonMapWildJsonMc.append(0);
        jsonMapWildJsonMc.append(0);
    }

    Json::Value& jsonMapWildJsonOc = jsonMapWildJson["occupy_conf"];
    jsonMapWildJsonOc = Json::Value(Json::arrayValue);
    if(udwWildClass == EN_WILD_CLASS_MONSTER || udwWildClass == EN_WILD_CLASS_LEADER_MONSTER)
    {
        assert(oWildResJson[szTmpBuf]["a4"]["a0"].size() >= udwWildlv);
        jsonMapWildJsonOc.append(oWildResJson[szTmpBuf]["a4"]["a0"][udwWildlv - 1].asUInt());
        jsonMapWildJsonOc.append(oWildResJson[szTmpBuf]["a4"]["a1"].asDouble());
        jsonMapWildJsonOc.append(oWildResJson[szTmpBuf]["a4"]["a2"].asUInt());
    }
    else
    {
        jsonMapWildJsonOc.append(0);
        jsonMapWildJsonOc.append(0);
        jsonMapWildJsonOc.append(0);
    }


    Json::Value& jsonMapWildJsonCastleRely = jsonMapWildJson["r"];
    jsonMapWildJsonCastleRely = Json::Value(Json::objectValue);
    //TODO 貌似没用
    jsonMapWildJsonCastleRely = 1;

    Json::Value& jsonMapWildJsonResearch = jsonMapWildJson["research"];
    jsonMapWildJsonResearch = Json::Value(Json::arrayValue);
    for(TUINT32 udwIdx = 0; udwIdx < oWildResJson[szTmpBuf]["a14"].size();++udwIdx)
    {
        jsonMapWildJsonResearch.append(oWildResJson[szTmpBuf]["a14"][udwIdx].asUInt());
    }

    jsonMapWildJson["sub_series"] = oWildResJson[szTmpBuf]["a15"].asInt();
}

TVOID CCommJson::PushData_GenMapJson( TUINT32 udwSvrId, TbMap* ptbMap, Json::Value& rJson )
{
    CCommJson::GenMapBaseJson(ptbMap, rJson["basic"]);

    Json::Value& jsonAttackInfoList = rJson["attack_info"];
    jsonAttackInfoList = Json::Value(Json::arrayValue);

    for(TUINT32 udwIdx = 0; udwIdx < ptbMap->m_bAttack_info.m_udwNum; ++udwIdx)
    {
        Json::Value &jTmp = jsonAttackInfoList[udwIdx];
        jTmp = Json::arrayValue;
        jTmp.append(ptbMap->m_bAttack_info[udwIdx].m_ddwId);
        jTmp.append(ptbMap->m_bAttack_info[udwIdx].m_ddwTimes);
        jTmp.append(ptbMap->m_bAttack_info[udwIdx].m_ddwAttackTime);
    }

    Json::Value& jsonAlAttackInfoList = rJson["al_attack_info"];
    jsonAlAttackInfoList = Json::Value(Json::arrayValue);
    for (TUINT32 udwIdx = 0; udwIdx < ptbMap->m_bAl_attack_info.m_udwNum; ++udwIdx)
    {
        Json::Value &jTmp = jsonAlAttackInfoList[udwIdx];
        jTmp = Json::arrayValue;
        jTmp.append(ptbMap->m_bAl_attack_info[udwIdx].m_ddwId);
        jTmp.append(ptbMap->m_bAl_attack_info[udwIdx].m_ddwTimes);
        jTmp.append(ptbMap->m_bAl_attack_info[udwIdx].m_ddwAttackTime);
    }

    PushData_GenMapWildJson(udwSvrId, ptbMap, rJson);
}
