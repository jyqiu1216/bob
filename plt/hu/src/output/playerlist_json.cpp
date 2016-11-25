#include "playerlist_json.h"
#include "game_command.h"
#include "common_func.h"
#include "game_define.h"
#include "player_base.h"
#include "common_json.h"
#include "alliance_mapping.h"
#include "action_base.h"
#include "common_base.h"

CPlayerListJson::CPlayerListJson()
{

}

CPlayerListJson::~CPlayerListJson()
{

}

TVOID CPlayerListJson::GenDataJson(SSession* pstSession, Json::Value& rJson)
{
    ////al_member_get
    ////后台不分页 全部返回
    //"svr_al_member_list":
    //[
    //],
    ////al_request_get
    //"svr_al_request_list":
    //[
    //],
    //player_info_get
    //"svr_player_info_list":
    //[
    //]
    SUserInfo* pstUser = &pstSession->m_stUserInfo;


    if(pstSession->m_udwCommand == EN_CLIENT_REQ_COMMAND__ALLIANCE_MEMBER_GET)
    {
        Json::Value& rJsonMemberList = rJson["svr_al_member_list"];
        rJsonMemberList = Json::Value(Json::arrayValue);
        for(TUINT32 udwIdx = 0, udwIdy = 0; udwIdx < pstSession->m_udwTmpAlmemberNum; ++udwIdx)
        {
            if (pstSession->m_atbTmpAlmember[udwIdx].m_nAl_pos == EN_ALLIANCE_POS__REQUEST)
            {
                continue;
            }
            pstSession->m_atbTmpAlmember[udwIdx].m_jProfile["aid"] = pstSession->m_atbTmpAlmember[udwIdx].m_nAid;
            pstSession->m_atbTmpAlmember[udwIdx].m_jProfile["alpos"] = pstSession->m_atbTmpAlmember[udwIdx].m_nAl_pos;

            //修正alliance name信息
            const Json::Value &jAlMapping = CAllianceMapping::GetInstance()->m_oJsonRoot["alliance_mapping"];
            if(pstSession->m_atbTmpAlmember[udwIdx].m_nAid > 0 && jAlMapping["update_time"].asInt64() > pstSession->m_atbTmpAlmember[udwIdx].m_nProfile_update_time)
            {
                if(jAlMapping.isMember(CCommonFunc::NumToString(pstSession->m_atbTmpAlmember[udwIdx].m_nAid)))
                {
                    //如果有nick name和al flag也要在这里更新
                    pstSession->m_atbTmpAlmember[udwIdx].m_jProfile["alname"] = jAlMapping[CCommonFunc::NumToString(pstSession->m_atbTmpAlmember[udwIdx].m_nAid)]["al_name"];
                }
            }
            //这里如果这个玩家一直不上线，m_atbTmpAlmember[udwIdx].m_jProfile里的被打，被抓英雄等信息都是滞后的

            rJsonMemberList[udwIdy] = pstSession->m_atbTmpAlmember[udwIdx].m_jProfile;
            udwIdy++;
        }

        Json::Value& jsonSvrPlayer = rJson["svr_player"];
        TbPlayer* ptbplayer = &pstSession->m_stUserInfo.m_tbPlayer;

        CCommJson::GenPlayerProfileJson(ptbplayer, pstUser->m_atbEquip, pstUser->m_udwEquipNum, jsonSvrPlayer, &pstSession->m_stUserInfo.m_stPlayerBuffList);

        SUserInfo* pstUser = &pstSession->m_stUserInfo;
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

        rjsonAlliance["can_assist_num"] = pstUser->m_udwAllianceCanAssistNum;
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


    if(pstSession->m_udwCommand == EN_CLIENT_OPERATE_CMD__AL_MEMBER_GET)
    {
        Json::Value& rJsonMemberList = rJson["svr_al_member_list"];
        rJsonMemberList = Json::Value(Json::arrayValue);
        for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwCurFindMemberNum; ++udwIdx)
        {
            pstSession->m_atbTmpAlmember[udwIdx].m_jProfile["aid"] = pstSession->m_atbTmpAlmember[udwIdx].m_nAid;
            pstSession->m_atbTmpAlmember[udwIdx].m_jProfile["alpos"] = pstSession->m_atbTmpAlmember[udwIdx].m_nAl_pos;

            //修正alliance name信息
            const Json::Value &jAlMapping = CAllianceMapping::GetInstance()->m_oJsonRoot["alliance_mapping"];
            if(pstSession->m_atbTmpAlmember[udwIdx].m_nAid > 0 && jAlMapping["update_time"].asInt64() > pstSession->m_atbTmpAlmember[udwIdx].m_nProfile_update_time)
            {
                if(jAlMapping.isMember(CCommonFunc::NumToString(pstSession->m_atbTmpAlmember[udwIdx].m_nAid)))
                {
                    //如果有nick name和al flag也要在这里更新
                    pstSession->m_atbTmpAlmember[udwIdx].m_jProfile["alname"] = jAlMapping[CCommonFunc::NumToString(pstSession->m_atbTmpAlmember[udwIdx].m_nAid)]["al_name"];
                }
            }

            rJsonMemberList[udwIdx] = pstSession->m_atbTmpAlmember[udwIdx].m_jProfile;
        }

        Json::Value& jsonSvrPlayer = rJson["svr_player"];
        TbPlayer* ptbplayer = &pstSession->m_stUserInfo.m_tbPlayer;

        CCommJson::GenPlayerProfileJson(ptbplayer, pstUser->m_atbEquip, pstUser->m_udwEquipNum, jsonSvrPlayer, &pstSession->m_stUserInfo.m_stPlayerBuffList);

        SUserInfo* pstUser = &pstSession->m_stUserInfo;
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

        rjsonAlliance["can_assist_num"] = pstUser->m_udwAllianceCanAssistNum;
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
    else if(pstSession->m_udwCommand == EN_CLIENT_REQ_COMMAND__ALLIANCE_REQUEST_GET)
    {
        Json::Value& rJsonReqList = rJson["svr_al_request_list"];
        rJsonReqList = Json::Value(Json::arrayValue);
        for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_udwTmpAlmemberNum; ++udwIdx)
        {
            pstSession->m_atbTmpAlmember[udwIdx].m_jProfile["alpos"] = pstSession->m_atbTmpAlmember[udwIdx].m_nAl_pos;
            rJsonReqList[udwIdx] = pstSession->m_atbTmpAlmember[udwIdx].m_jProfile;
        }
    }
    else if(pstSession->m_udwCommand == EN_CLIENT_REQ_COMMAND__PLAYER_INFO_GET)
    {
        Json::Value& rJsonPlayInfo = rJson["svr_player_info_list"];
        rJsonPlayInfo = Json::Value(Json::arrayValue);
        CCommJson::GenPlayerProfileJson(&pstSession->m_tbTmpPlayer,
            pstSession->m_atbTmpEquip, pstSession->m_udwTmpEquipNum, rJsonPlayInfo[0u], NULL);

        if (pstSession->m_jTmpPlayerRankInfo.isObject() && pstSession->m_jTmpPlayerRankInfo.isMember("uid")
            && pstSession->m_jTmpPlayerRankInfo["uid"].asInt() == pstSession->m_tbTmpPlayer.m_nUid)
        {
            Json::Value& rJsonPlayerRankInfo = rJson["svr_player_rank_info_list"];
            rJsonPlayerRankInfo = Json::Value(Json::arrayValue);
            rJsonPlayerRankInfo.append(pstSession->m_jTmpPlayerRankInfo);
        }
    }
}
