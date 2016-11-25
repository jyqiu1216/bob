#include "map_json.h"
//#include "wild_info.h"
#include "user_json.h"
#include "time_utils.h"
#include "action_base.h"
#include "player_info.h"
#include "common_json.h"
#include "wild_info.h"
#include "game_define.h"
#include "map_logic.h"
#include "common_func.h"
#include "game_svr.h"
#include "map_base.h"
#include "game_command.h"
#include "map_logic.h"
#include "common_logic.h"
#include "msg_base.h"

CMapJson::CMapJson()
{

}

CMapJson::~CMapJson()
{

}

TVOID CMapJson::GenDataJson(SSession* pstSession, Json::Value& rJson)
{
    if(pstSession->m_udwCommand == EN_CLIENT_REQ_COMMAND__MANOR_INFO)
    {
        rJson = Json::Value(Json::objectValue);
        rJson["svr_manor_info"] = pstSession->m_JsonValue;
        return;
    }
    if(pstSession->m_udwCommand == EN_CLIENT_REQ_COMMAND__PRISON_INFO)
    {
        rJson = Json::Value(Json::objectValue);
        rJson["svr_prison_info"] = pstSession->m_JsonValue;
        return;
    }

    /*
    "svr_map":
    {
        "num": int,            //返回的地图数据的数量 [int]
        "svr_id": int,         //server id
		"bid_list":
		[
            int,
            int,
            ...
		],
        "list":
        {
            "id"://map id
			{
                "basic":[
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
                ],
                "attack_info":
                [
                    [long, int, long], // [uid, 下次连击次数, 连击结束时间]
                    ...
                ],
                "al_attack_info":
                [
                    [int, int], // [aid, 下次连击次数]
                    ...
                ],
                "wild_json":// 从运营取出的动态json,非动态地形 可能为空,客户端保护 
                {
                    //此处所有定位为运营给出,具体含义见运营文档
                    "basic": // 基础信息
                    {
                        a0 // 野地类型定义：
                        a1 // 野地交互方式：0-拉，1-打
                        a2 // 野地内容类型：0-资源(指单一资源)，1-物品
                        a3 // 野地内容获取方式：0-全部获取，1-随机获取：随机拿走各种资源或物品若干数量，2-获得奖励列表中的1种
                        a4 // 野地中奖获取方式：0-全部获取，1-随机获取：随机拿走各种资源或物品若干数量，2-获得奖励列表中的1种
                        a5 // 野地消耗英雄体力方式：0-不消耗，1-消耗体力-绝对值，2-消耗体力-百分比

                    },
                    "reward": // 奖励
                    [
                        {
                            type // type
                            id // id
                            num // num
                            rate //rate
                            min // 最小数量
                            max // 最大数量
                        }
                    ]
                    "ui": //UI方案
                    {
                        a0 // 图标
                        a1 // 弹框底图
                    }
                                
                    "ak_type" //攻击方式
                    [ //三个元素分别是：英雄、龙、兵
                        {
                            a0 //选项：0-必选，1-可选，2-不可选
                            a1 //下限（备注：英雄无上下限，龙的上下限是指等级，兵的上下限是指数量,龙和兵都必选的情况下只对兵做上下限要求）
                            a2 //上限(只用于计算拉取速度，限制本身不生效)
                        }
                    ]
								
                    "dragon_energy_cost":0(int) //v1.2_wave@2015.7.22
                                
                    "monster_conf": //v1.2_wave@2015.7.22：
                    [
                        (int), //怪物的前置等级（打此等级怪的前置怪物等级）
                        (int), //怪物的最大生命
                        (int), //怪物的防御
                        (int), //force
                    ]

                    "occupy_conf":
                    [
                        (int), //基础拉取时间
                        (double),   //攻击军队增益，表示派兵超过下限或者龙等级超过下限时拉取的速度加成
                                    //factor=max(0,min(派出兵数量或龙等级, 上线数量或等级)-下限数量或等级)/ 上限数量或等级 * a1，当此增益为0时多派对速度无意义， 浮点数，精确到小数点后6位
                                    //最终拉取时间=a0/(1+factor)
                        (int),//0-表示只算龙等级对速度的加成，1-表示只算兵数量对速度的加成
                    ]
                                
                    "r":5 //依赖的castle等级
								
					"research"://依赖的研究id，下标为wild等级-1
					[
					]

                    "sub_series":1//当是巢穴的时候，返回该巢穴对应的装备子系列id
                }
            }
        },
        "map_action_list": 
        {
            "ID":
            {
                ...//参见user_info
            }
        }
    }
    */
    TUINT32 udwSvrId = strtoul(pstSession->m_stReqParam.m_szKey[1], NULL, 10);
    Json::Value& jsonMapList = rJson["svr_map"];
    jsonMapList = Json::Value(Json::objectValue);
    jsonMapList["num"] = pstSession->m_udwTmpWildNum;//m_stMapList.m_dwWildNum;
    if(pstSession->m_udwTmpWildNum > 0)
    {
        jsonMapList["svr_id"] = pstSession->m_atbTmpWild[0].m_nSid;
    }
    else
    {
        TINT32 dwSid = strtoul(pstSession->m_stReqParam.m_szKey[1], NULL, 10);
        jsonMapList["svr_id"] = dwSid;
    }
    for(map<TUINT64, TUINT32>::iterator it = pstSession->m_mBlockId.begin(); it != pstSession->m_mBlockId.end();++it)
    {
        jsonMapList["bid_list"].append(it->first);
    }

    jsonMapList["list"] = Json::Value(Json::objectValue);
    Json::Value& jsonMapActionList = jsonMapList["map_action_list"];
    jsonMapActionList = Json::Value(Json::objectValue);
    TUINT32 udwJsonIndex = 0;

    for(TUINT32  udwMapIdx = 0;udwMapIdx < pstSession->m_udwTmpWildNum; ++udwMapIdx)
    {
        if(pstSession->m_aucTmpWildFlag[udwMapIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }
        if(CTimeUtils::GetUnixTime() < pstSession->m_atbTmpWild[udwMapIdx].m_nShowtime)
        {
            continue;
        }

        string strMapId = CCommonFunc::NumToString(pstSession->m_atbTmpWild[udwMapIdx].m_nId);
        jsonMapList["list"][strMapId] = Json::Value(Json::objectValue);

        TUINT32 udwWildClass = CMapLogic::GetWildClass(udwSvrId, pstSession->m_atbTmpWild[udwMapIdx].m_nType);
        if((udwWildClass == EN_WILD_CLASS_MONSTER || udwWildClass == EN_WILD_CLASS_LEADER_MONSTER) &&
            pstSession->m_atbTmpWild[udwMapIdx].m_nExpire_time < CTimeUtils::GetUnixTime())
        {
            if (pstSession->m_atbTmpWild[udwMapIdx].m_nUid > 0)
            {
                CMsgBase::RefreshUserInfo(pstSession->m_atbTmpWild[udwMapIdx].m_nUid);
            }
            continue;
        }

        if (pstSession->m_atbTmpWild[udwMapIdx].m_nUid > 0
            && pstSession->m_atbTmpWild[udwMapIdx].m_nType == EN_WILD_TYPE__CITY)
        {
            if (pstSession->m_atbTmpWild[udwMapIdx].m_nMight == 0)
            {
                CMsgBase::ClearNoPlayerMap(pstSession->m_atbTmpWild[udwMapIdx].m_nUid, pstSession->m_atbTmpWild[udwMapIdx].m_nSid, pstSession->m_atbTmpWild[udwMapIdx].m_nId);
            }
            else if (pstSession->m_atbTmpWild[udwMapIdx].m_nTime_end < CTimeUtils::GetUnixTime()
                && ((pstSession->m_atbTmpWild[udwMapIdx].m_nStatus & EN_CITY_STATUS__AVOID_WAR)
                || (pstSession->m_atbTmpWild[udwMapIdx].m_nStatus & EN_CITY_STATUS__NEW_PROTECTION)))
            {
                CMsgBase::RefreshUserInfo(pstSession->m_atbTmpWild[udwMapIdx].m_nUid);
            }
        }

        if (udwWildClass == EN_WILD_CLASS_MONSTER_NEST
            && pstSession->m_atbTmpWild[udwMapIdx].m_nExpire_time < CTimeUtils::GetUnixTime())
        {
            continue;
        }

        GenMapWildJson(pstSession, jsonMapList["list"][strMapId], udwMapIdx);

        CCommJson::GenMapBaseJson(&pstSession->m_atbTmpWild[udwMapIdx], jsonMapList["list"][strMapId]["basic"]);

        if (udwWildClass == EN_WILD_CLASS_MONSTER_NEST)
        {
            for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_udwTmpMarchNum; ++udwIdx)
            {
                TbMarch_action *ptbAction = &pstSession->m_atbTmpMarch[udwIdx];
                if (ptbAction->m_nSuid == pstSession->m_stReqParam.m_udwUserId &&
                    ptbAction->m_nTpos == pstSession->m_atbTmpWild[udwMapIdx].m_nId &&
                    ptbAction->m_nStatus == EN_MARCH_STATUS__LOADING)
                {
                    jsonMapList["list"][strMapId]["basic"][7U] = ptbAction->m_nSuid;
                    jsonMapList["list"][strMapId]["basic"][8U] = ptbAction->m_bParam[0].m_szSourceUserName;
                    TSE_LOG_INFO(pstSession->m_poServLog, ("CMapJson::GenDataJson: set tmp_map uid[%ld] uname[%s] [seq=%u]",
                        ptbAction->m_nSuid, ptbAction->m_bParam[0].m_szSourceUserName, pstSession->m_udwSeqNo));
                }
            }
        }

        Json::Value& jsonAttackInfoList = jsonMapList["list"][strMapId]["attack_info"];
        jsonAttackInfoList = Json::Value(Json::arrayValue);

        for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_atbTmpWild[udwMapIdx].m_bAttack_info.m_udwNum; ++udwIdx)
        {
            Json::Value &jTmp = jsonAttackInfoList[udwIdx];
            jTmp = Json::arrayValue;
            jTmp.append(pstSession->m_atbTmpWild[udwMapIdx].m_bAttack_info[udwIdx].m_ddwId);
            jTmp.append(pstSession->m_atbTmpWild[udwMapIdx].m_bAttack_info[udwIdx].m_ddwTimes);
            jTmp.append(pstSession->m_atbTmpWild[udwMapIdx].m_bAttack_info[udwIdx].m_ddwAttackTime);
        }

        Json::Value& jsonAlAttackInfoList = jsonMapList["list"][strMapId]["al_attack_info"];
        jsonAlAttackInfoList = Json::Value(Json::arrayValue);

        for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_atbTmpWild[udwMapIdx].m_bAl_attack_info.m_udwNum; ++udwIdx)
        {
            Json::Value &jTmp = jsonAlAttackInfoList[udwIdx];
            jTmp = Json::arrayValue;
            jTmp.append(pstSession->m_atbTmpWild[udwMapIdx].m_bAl_attack_info[udwIdx].m_ddwId);
            jTmp.append(pstSession->m_atbTmpWild[udwMapIdx].m_bAl_attack_info[udwIdx].m_ddwTimes);
            jTmp.append(pstSession->m_atbTmpWild[udwMapIdx].m_bAl_attack_info[udwIdx].m_ddwAttackTime);
        }
    }

    for(TUINT32 udwIndex = 0; udwIndex < pstSession->m_udwTmpMarchNum; ++udwIndex)
    {
        if(pstSession->m_atbTmpMarch[udwIndex].m_nStatus == EN_MARCH_STATUS__RETURNING
            && CActionBase::IsEmptyMarch(&pstSession->m_atbTmpMarch[udwIndex]))
        {
            if(pstSession->m_atbTmpMarch[udwIndex].m_nSclass == EN_ACTION_SEC_CLASS__RALLY_REINFORCE
                || pstSession->m_atbTmpMarch[udwIndex].m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_NORMAL
                || pstSession->m_atbTmpMarch[udwIndex].m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_THRONE)
            {
                continue;
            }
        }

        if(EN_ACTION_MAIN_CLASS__MARCH == pstSession->m_atbTmpMarch[udwIndex].m_nMclass)
        {
            string strActionId = CCommonFunc::NumToString(pstSession->m_atbTmpMarch[udwIndex].m_nId);
            CCommJson::GenMarchInfo(&pstSession->m_atbTmpMarch[udwIndex], jsonMapActionList[strActionId]);
            ++udwJsonIndex;
        }
    }
}

TVOID CMapJson::GenMapWildJson(SSession* pstSession, Json::Value& rJson,TUINT32 udwIdx)
{

    /*
    "wild_json":// 从运营取出的动态json,非动态地形 可能为空,客户端保护 
    {
        //此处所有定位为运营给出,具体含义见运营文档
        "basic": // 基础信息
        {
            a0 // 野地类型定义：
            a1 // 野地交互方式：0-拉，1-打
            a2 // 野地内容类型：0-资源(指单一资源)，1-物品
            a3 // 野地内容获取方式：0-全部获取，1-随机获取：随机拿走各种资源或物品若干数量，2-获得奖励列表中的1种
            a4 // 野地中奖获取方式：0-全部获取，1-随机获取：随机拿走各种资源或物品若干数量，2-获得奖励列表中的1种
            a5 // 野地消耗英雄体力方式：0-不消耗，1-消耗体力-绝对值，2-消耗体力-百分比

        },
        "reward": // 奖励
        [
            {
                type // type
                id // id
                num // num
                rate //rate
                min // 最小数量
                max // 最大数量
            }
        ]
        "ui": //UI方案
        {
            a0 // 图标
            a1 // 弹框底图
        }
                                
        "ak_type" //攻击方式
        [ //三个元素分别是：英雄、龙、兵
            {
                a0 //选项：0-必选，1-可选，2-不可选
                a1 //下限（备注：英雄无上下限，龙的上下限是指等级，兵的上下限是指数量,龙和兵都必选的情况下只对兵做上下限要求）
                a2 //上限(只用于计算拉取速度，限制本身不生效)
            }
        ]
								
        "dragon_energy_cost":0(int) //v1.2_wave@2015.7.22
                                
        "monster_conf": //v1.2_wave@2015.7.22：
        [
            (int), //怪物的前置等级（打此等级怪的前置怪物等级）
            (int), //怪物的最大生命
            (int), //怪物的防御
            (int), //force
        ]

        "occupy_conf":
        [
            (int), //基础拉取时间
            (double),   //攻击军队增益，表示派兵超过下限或者龙等级超过下限时拉取的速度加成
                        //factor=max(0,min(派出兵数量或龙等级, 上线数量或等级)-下限数量或等级)/ 上限数量或等级 * a1，当此增益为0时多派对速度无意义， 浮点数，精确到小数点后6位
                        //最终拉取时间=a0/(1+factor)
            (int),//0-表示只算龙等级对速度的加成，1-表示只算兵数量对速度的加成
        ]
                                
        "r":5 //依赖的castle等级
								
		"research"://依赖的研究id，下标为wild等级-1
		[
		]

        "sub_series":1//当是巢穴的时候，返回该巢穴对应的装备子系列id
    }
    */
    TUINT32 udwSvrId = strtoul(pstSession->m_stReqParam.m_szKey[1], NULL, 10);
    Json::Value& jsonMapWildJson = rJson["wild_json"];
    if(pstSession->m_atbTmpWild[udwIdx].Get_Type()  <= EN_WILD_TYPE_RES_INIT)
    {
        return ;
    }
    jsonMapWildJson = Json::Value(Json::objectValue);
    TUINT32 udwWildType = pstSession->m_atbTmpWild[udwIdx].Get_Type();
    TUINT32 udwWildlv = pstSession->m_atbTmpWild[udwIdx].Get_Level();

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

