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
        "num": int,            //���صĵ�ͼ���ݵ����� [int]
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
                    int, //status ͬplayer status
                    long, //force
                    long, //force_kill
                    long, //uid
                    string, //player name
                    int, //alid
                    string, //alliance_name
                    string, //������
                    int, // ��Դʣ����� ���֮һ 5000��ʾʣ��50% ���㵥һ��Դ(����gem)����ʱ ���ý�һ��... 0����δ��ȡ(����Դ)[int]
                    int, // wild class type EWildClass
                    int, // player avatar
                    int, //�Ƿ����Ŀ� 1:�� 2:��ʾĳ�����͵ĵؿ�ķ����Ŀ� 0:��ͨ��
                    string, //al_nick
                    int, //block num
                    int, //alliance flag
					long, //������ð�̽���ʱ��
					int, // monster_life_left
					long, //���ν���ʱ��
                    int��//������id��svr_id
                    string, //����������svr_name
                    int, //0��ʾ����Ϊ��, 1��ʾ��������Ӣ��
					long, //��պ�ð�����ʱ��
                    int, //reinforce��������
                    int, //��ǰreinforce����
					int, //age
					int, //vip lv
					long, //vip end time
					int, //al pos
                    int, //˰�ʵ�id,��������Ч ����˰��ֵ��game.json
                    int, //user lord level
                    long, //protect end time
                    int, //center pos ���䲻�����Ŀ�ʱʹ��
                    int //�Ƿ��Ƴ�, �жϸó����Ƿ�Ҫ�����Ƴ�
                ],
                "attack_info":
                [
                    [long, int, long], // [uid, �´���������, ��������ʱ��]
                    ...
                ],
                "al_attack_info":
                [
                    [int, int], // [aid, �´���������]
                    ...
                ],
                "wild_json":// ����Ӫȡ���Ķ�̬json,�Ƕ�̬���� ����Ϊ��,�ͻ��˱��� 
                {
                    //�˴����ж�λΪ��Ӫ����,���庬�����Ӫ�ĵ�
                    "basic": // ������Ϣ
                    {
                        a0 // Ұ�����Ͷ��壺
                        a1 // Ұ�ؽ�����ʽ��0-����1-��
                        a2 // Ұ���������ͣ�0-��Դ(ָ��һ��Դ)��1-��Ʒ
                        a3 // Ұ�����ݻ�ȡ��ʽ��0-ȫ����ȡ��1-�����ȡ��������߸�����Դ����Ʒ����������2-��ý����б��е�1��
                        a4 // Ұ���н���ȡ��ʽ��0-ȫ����ȡ��1-�����ȡ��������߸�����Դ����Ʒ����������2-��ý����б��е�1��
                        a5 // Ұ������Ӣ��������ʽ��0-�����ģ�1-��������-����ֵ��2-��������-�ٷֱ�

                    },
                    "reward": // ����
                    [
                        {
                            type // type
                            id // id
                            num // num
                            rate //rate
                            min // ��С����
                            max // �������
                        }
                    ]
                    "ui": //UI����
                    {
                        a0 // ͼ��
                        a1 // �����ͼ
                    }
                                
                    "ak_type" //������ʽ
                    [ //����Ԫ�طֱ��ǣ�Ӣ�ۡ�������
                        {
                            a0 //ѡ�0-��ѡ��1-��ѡ��2-����ѡ
                            a1 //���ޣ���ע��Ӣ���������ޣ�������������ָ�ȼ���������������ָ����,���ͱ�����ѡ�������ֻ�Ա���������Ҫ��
                            a2 //����(ֻ���ڼ�����ȡ�ٶȣ����Ʊ�����Ч)
                        }
                    ]
								
                    "dragon_energy_cost":0(int) //v1.2_wave@2015.7.22
                                
                    "monster_conf": //v1.2_wave@2015.7.22��
                    [
                        (int), //�����ǰ�õȼ�����˵ȼ��ֵ�ǰ�ù���ȼ���
                        (int), //������������
                        (int), //����ķ���
                        (int), //force
                    ]

                    "occupy_conf":
                    [
                        (int), //������ȡʱ��
                        (double),   //�����������棬��ʾ�ɱ��������޻������ȼ���������ʱ��ȡ���ٶȼӳ�
                                    //factor=max(0,min(�ɳ������������ȼ�, ����������ȼ�)-����������ȼ�)/ ����������ȼ� * a1����������Ϊ0ʱ���ɶ��ٶ������壬 ����������ȷ��С�����6λ
                                    //������ȡʱ��=a0/(1+factor)
                        (int),//0-��ʾֻ�����ȼ����ٶȵļӳɣ�1-��ʾֻ����������ٶȵļӳ�
                    ]
                                
                    "r":5 //������castle�ȼ�
								
					"research"://�������о�id���±�Ϊwild�ȼ�-1
					[
					]

                    "sub_series":1//���ǳ�Ѩ��ʱ�򣬷��ظó�Ѩ��Ӧ��װ����ϵ��id
                }
            }
        },
        "map_action_list": 
        {
            "ID":
            {
                ...//�μ�user_info
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
    "wild_json":// ����Ӫȡ���Ķ�̬json,�Ƕ�̬���� ����Ϊ��,�ͻ��˱��� 
    {
        //�˴����ж�λΪ��Ӫ����,���庬�����Ӫ�ĵ�
        "basic": // ������Ϣ
        {
            a0 // Ұ�����Ͷ��壺
            a1 // Ұ�ؽ�����ʽ��0-����1-��
            a2 // Ұ���������ͣ�0-��Դ(ָ��һ��Դ)��1-��Ʒ
            a3 // Ұ�����ݻ�ȡ��ʽ��0-ȫ����ȡ��1-�����ȡ��������߸�����Դ����Ʒ����������2-��ý����б��е�1��
            a4 // Ұ���н���ȡ��ʽ��0-ȫ����ȡ��1-�����ȡ��������߸�����Դ����Ʒ����������2-��ý����б��е�1��
            a5 // Ұ������Ӣ��������ʽ��0-�����ģ�1-��������-����ֵ��2-��������-�ٷֱ�

        },
        "reward": // ����
        [
            {
                type // type
                id // id
                num // num
                rate //rate
                min // ��С����
                max // �������
            }
        ]
        "ui": //UI����
        {
            a0 // ͼ��
            a1 // �����ͼ
        }
                                
        "ak_type" //������ʽ
        [ //����Ԫ�طֱ��ǣ�Ӣ�ۡ�������
            {
                a0 //ѡ�0-��ѡ��1-��ѡ��2-����ѡ
                a1 //���ޣ���ע��Ӣ���������ޣ�������������ָ�ȼ���������������ָ����,���ͱ�����ѡ�������ֻ�Ա���������Ҫ��
                a2 //����(ֻ���ڼ�����ȡ�ٶȣ����Ʊ�����Ч)
            }
        ]
								
        "dragon_energy_cost":0(int) //v1.2_wave@2015.7.22
                                
        "monster_conf": //v1.2_wave@2015.7.22��
        [
            (int), //�����ǰ�õȼ�����˵ȼ��ֵ�ǰ�ù���ȼ���
            (int), //������������
            (int), //����ķ���
            (int), //force
        ]

        "occupy_conf":
        [
            (int), //������ȡʱ��
            (double),   //�����������棬��ʾ�ɱ��������޻������ȼ���������ʱ��ȡ���ٶȼӳ�
                        //factor=max(0,min(�ɳ������������ȼ�, ����������ȼ�)-����������ȼ�)/ ����������ȼ� * a1����������Ϊ0ʱ���ɶ��ٶ������壬 ����������ȷ��С�����6λ
                        //������ȡʱ��=a0/(1+factor)
            (int),//0-��ʾֻ�����ȼ����ٶȵļӳɣ�1-��ʾֻ����������ٶȵļӳ�
        ]
                                
        "r":5 //������castle�ȼ�
								
		"research"://�������о�id���±�Ϊwild�ȼ�-1
		[
		]

        "sub_series":1//���ǳ�Ѩ��ʱ�򣬷��ظó�Ѩ��Ӧ��װ����ϵ��id
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
    //TODO ò��û��
    jsonMapWildJsonCastleRely = 1;

    Json::Value& jsonMapWildJsonResearch = jsonMapWildJson["research"];
    jsonMapWildJsonResearch = Json::Value(Json::arrayValue);
    for(TUINT32 udwIdx = 0; udwIdx < oWildResJson[szTmpBuf]["a14"].size();++udwIdx)
    {
        jsonMapWildJsonResearch.append(oWildResJson[szTmpBuf]["a14"][udwIdx].asUInt());
    }
    
    jsonMapWildJson["sub_series"] = oWildResJson[szTmpBuf]["a15"].asInt();
}

