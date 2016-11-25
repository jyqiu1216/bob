#include "buffer_info_json.h"
#include "user_json.h"
#include "time_utils.h"
#include "action_base.h"
#include "player_info.h"
#include "common_json.h"
#include "wild_info.h"
#include "game_define.h"
#include "common_func.h"
#include "game_command.h"

CBufferInfoJson::CBufferInfoJson()
{

}

CBufferInfoJson::~CBufferInfoJson()
{

}

TVOID CBufferInfoJson::GenDataJson(SSession* pstSession, Json::Value& rJson)
{
    /*
        "svr_buffer_info":
        {
        "buffer_id":{
        "total":100000,
        "skill":1000,
        "research":1000,
        "basic":1000,
        "wild":1000,
        "item":1000,
        "knight":1000,
        "vip":1000,
        "building":1000
        "equip":100
        }
        }

        "svr_equip_buffer_info":
        {
        "idx":
        {
        "id",
        "absort",
        "typd_id",
        "status",
        "age",
        "pos",
        "crystal":
        [
        ],
        "base_buff":
        [
        [
        id,
        num
        ]
        ],
        "total_buff":
        [
        [
        id,
        num
        ]
        ]
        }
        }


        */
    if(pstSession->m_stReqParam.m_udwCommandID == EN_OPSELF_GET_BUFFER_INFO)
    {
        TINT32 dwTroopId = atoi(pstSession->m_stReqParam.m_szKey[0]);
        SUserInfo *pstUser = &pstSession->m_stUserInfo;

        Json::Value& jsonBufferInfo = rJson["svr_buffer_info"];
        jsonBufferInfo = Json::Value(Json::objectValue);

        Json::Value& jsonBufferDetail = jsonBufferInfo[CCommonFunc::NumToString(dwTroopId)];
        jsonBufferDetail = Json::Value(Json::objectValue);

        jsonBufferDetail["total"] = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[dwTroopId].m_ddwBuffTotal;
        jsonBufferDetail["basic"] = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[dwTroopId].m_astBuffDetail[EN_BUFF_TYPE_BASIC].m_ddwNum;
        jsonBufferDetail["dragon_skill"] = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[dwTroopId].m_astBuffDetail[EN_BUFF_TYPE_DRAGON_SKILL].m_ddwNum;
        jsonBufferDetail["research"] = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[dwTroopId].m_astBuffDetail[EN_BUFF_TYPE_RESEARCH].m_ddwNum;
        jsonBufferDetail["item"] = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[dwTroopId].m_astBuffDetail[EN_BUFF_TYPE_ITEM].m_ddwNum;
        jsonBufferDetail["vip"] = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[dwTroopId].m_astBuffDetail[EN_BUFF_TYPE_VIP].m_ddwNum;
        jsonBufferDetail["equip"] = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[dwTroopId].m_astBuffDetail[EN_BUFF_TYPE_EQUIP].m_ddwNum;
        jsonBufferDetail["title"] = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[dwTroopId].m_astBuffDetail[EN_BUFF_TYPE_TITLE].m_ddwNum;
        jsonBufferDetail["altar"] = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[dwTroopId].m_astBuffDetail[EN_BUFF_TYPE_ALTAR].m_ddwNum;
        jsonBufferDetail["throne"] = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[dwTroopId].m_astBuffDetail[EN_BUFF_TYPE_THRONE].m_ddwNum;
        jsonBufferDetail["fort"] = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[dwTroopId].m_astBuffDetail[EN_BUFF_TYPE_FORT].m_ddwNum;
        jsonBufferDetail["troop"] = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[dwTroopId].m_astBuffDetail[EN_BUFF_TYPE_TROOP].m_ddwNum;
        jsonBufferDetail["building"] = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[dwTroopId].m_astBuffDetail[EN_BUFF_TYPE_BUILDING].m_ddwNum;
        jsonBufferDetail["dragon_lv"] = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[dwTroopId].m_astBuffDetail[EN_BUFF_TYPE_DRAGON_LV].m_ddwNum;
        jsonBufferDetail["dragon_monster_skill"] = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[dwTroopId].m_astBuffDetail[EN_BUFF_TYPE_DRAGON_MONSTER_SKILL].m_ddwNum;
        jsonBufferDetail["lord_skill"] = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[dwTroopId].m_astBuffDetail[EN_BUFF_TYPE_LORD_SKILL].m_ddwNum;
        jsonBufferDetail["throne_research"] = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[dwTroopId].m_astBuffDetail[EN_BUFF_TYPE_THRONE_RESEARCH].m_ddwNum;
        jsonBufferDetail["knight"] = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[dwTroopId].m_astBuffDetail[EN_BUFF_TYPE_KNIGHT].m_ddwNum;
        jsonBufferDetail["idol"] = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[dwTroopId].m_astBuffDetail[EN_BUFF_TYPE_IDOL].m_ddwNum;
    }

    if(pstSession->m_stReqParam.m_udwCommandID == EN_OPSELF_GET_EQUIP_BUFFER_INFO)
    {
        Json::Value& jsonEquipBufferInfo = rJson["svr_equip_buffer_info"];
        jsonEquipBufferInfo = Json::Value(Json::objectValue);
    }
}
