#include "sendmessage_base.h"
#include "common_func.h"
#include "game_info.h"
#include "time_utils.h"
#include "http_utils.h"
#include "player_base.h"
#include "tool_base.h"
#include "msg_base.h"

using namespace wtse::log;
using namespace std;

TINT32 CSendMessageBase::AddTips(SUserInfo *pstUser, TUINT8 ucType, TUINT32 udwUid, TBOOL bIsNeedPut,
    TUINT32 udwKey0, TUINT32 udwKey1, TUINT32 udwKey2, const TCHAR *pszKey0, const TCHAR *pszKey1, const TCHAR *pszKey2)
{
    if(pstUser->m_udwTipsNum >= MAX_PLAYER_TIPS_NUM * 2)
    {
        return 1;
    }

    TbTips *ptbTips = &pstUser->m_atbTips[pstUser->m_udwTipsNum];

    GetTips(ptbTips, ucType, udwUid, udwKey0, udwKey1, udwKey2, pszKey0, pszKey1, pszKey2);

    if(bIsNeedPut)
    {
        pstUser->m_aucTipsFlag[pstUser->m_udwTipsNum] = EN_TABLE_UPDT_FLAG__NEW;
    }
    pstUser->m_udwTipsNum++;

    return 0;
}

TINT32 CSendMessageBase::AddTips(SUserInfo* pstUser, TUINT32 udwUid, TBOOL bIsNeedPut, TUINT8 ucType, const string& strContent)
{
    if (pstUser->m_udwTipsNum >= MAX_PLAYER_TIPS_NUM * 2)
    {
        return 1;
    }

    TbTips *ptbTips = &pstUser->m_atbTips[pstUser->m_udwTipsNum];

    GetTips(ptbTips, udwUid, ucType, strContent);

    if (bIsNeedPut)
    {
        pstUser->m_aucTipsFlag[pstUser->m_udwTipsNum] = EN_TABLE_UPDT_FLAG__NEW;
    }
    pstUser->m_udwTipsNum++;

    return 0;
}

TINT32 CSendMessageBase::AddAlTips(SUserInfo *pstUser, TUINT8 ucType, TINT64 ddwAid, TBOOL bIsNeedPut, TUINT32 udwKey0 /*= 0*/, TUINT32 udwKey1 /*= 0*/, TUINT32 udwKey2 /*= 0*/, const TCHAR *pszKey0 /*= ""*/, const TCHAR *pszKey1 /*= ""*/, const TCHAR *pszKey2 /*= ""*/)
{
    if(pstUser->m_udwTipsNum >= MAX_PLAYER_TIPS_NUM * 2)
    {
        return 1;
    }

    if(ddwAid == 0)
    {
        return 2;
    }

    TCHAR szContent[256];
    sprintf(szContent, "%u&%u&%u&%s&%s&%s", udwKey0, udwKey1, udwKey2, pszKey0, pszKey1, pszKey2);

    TbTips *ptbTips = &pstUser->m_atbTips[pstUser->m_udwTipsNum];

    ptbTips->Set_Uid(-1 * ddwAid);
    ptbTips->Set_Time(CTimeUtils::GetCurTimeUs());
    ptbTips->Set_Type(ucType);
    ptbTips->Set_Content(szContent);
    if(bIsNeedPut)
    {
        pstUser->m_aucTipsFlag[pstUser->m_udwTipsNum] = EN_TABLE_UPDT_FLAG__NEW;
    }
    pstUser->m_udwTipsNum++;

    return 0;
}

/*
TINT32 CSendMessageBase::AddAlEventTips(SUserInfo *pstUser,TINT64 ddwAid, TUINT32 udwEventType,
    TUINT32 udwRewardType, TUINT32 udwGoalorRank, TUINT32 udwScoreGet, string sScoreList, SGlobalRes *pstReward,string sEventInfo)
{
    //{//content
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
    //}
    if(pstUser->m_udwEventTipsNum >= MAX_EVENT_TIPS_NUM * 2)
    {
        return 1;
    }
    if(ddwAid == 0)
    {
        return 2;
    }
    TUINT32 udwEventEndTime = 0;
    TUINT64 uddwEventId = 0;
    Json::Value jEventInfo;
    jEventInfo.clear();
    Json::Reader reader;
    TBOOL bPase = reader.parse(sEventInfo.c_str(), jEventInfo);
    if(!bPase)
    {
        udwEventEndTime = 0;
        uddwEventId = 0;
    }
    else
    {
        udwEventEndTime = jEventInfo["e_t"].asUInt();
        uddwEventId = jEventInfo["e_id"].asUInt64();
    }
    TbEvent_tips *ptbEventTips = &pstUser->m_atbEventTips[pstUser->m_udwEventTipsNum];
    ptbEventTips->Set_Uid(-1 * ddwAid);
    ptbEventTips->Set_Time(CTimeUtils::GetUnixTime());
    ptbEventTips->Set_Event_end_time(udwEventEndTime);

    ptbEventTips->Set_Id(CToolBase::GetPlayerNewTaskId(pstUser->m_tbPlayer.m_nUid, pstUser->m_tbLogin.m_nSeq));
    pstUser->m_tbLogin.Set_Seq(pstUser->m_tbLogin.m_nSeq + 1); //llt add,防止,同一个用户请求中,生成多个tips的情况

    Json::Value jEventTipsContent;
    jEventTipsContent = Json::Value(Json::objectValue);
    jEventTipsContent["event_type"] = udwEventType;
    jEventTipsContent["reward_type"] = udwRewardType;
    jEventTipsContent["num"] = udwGoalorRank;
    jEventTipsContent["score_get"] = udwScoreGet;
    jEventTipsContent["event_id"] = uddwEventId;
    Json::Value &jScoreList = jEventTipsContent["score_list"];
    jScoreList = Json::Value(Json::arrayValue);
    vector<TUINT32> vScoreList;
    CCommonFunc::GetVectorFromString(sScoreList.c_str(), ':', vScoreList);

    for(vector<TUINT32>::iterator it = vScoreList.begin(); it < vScoreList.end(); ++it)
    {
        jScoreList.append(*it);
    }


    Json::Value &jRewardList = jEventTipsContent["reward"];
    jRewardList = Json::Value(Json::arrayValue);
    for(TUINT32 udwIdx = 0; udwIdx < pstReward->ddwTotalNum; ++udwIdx)
    {
        jRewardList[udwIdx].append(pstReward->aRewardList[udwIdx].ddwType);
        jRewardList[udwIdx].append(pstReward->aRewardList[udwIdx].ddwId);
        jRewardList[udwIdx].append(pstReward->aRewardList[udwIdx].ddwNum);
    }
    ptbEventTips->Set_Content(jEventTipsContent);

    pstUser->m_aucEventTipsFlag[pstUser->m_udwEventTipsNum] = EN_TABLE_UPDT_FLAG__NEW;
    pstUser->m_udwEventTipsNum++;

    return 0;
}
*/

TINT32 CSendMessageBase::AddEventTips(SUserInfo *pstUser,TUINT32 udwEventType,TUINT32 udwRewardType,TUINT32 udwGoalorRank,
    TUINT32 udwScoreGet, string sScoreList, SOneGlobalRes *astRewardList, TUINT32 udwRewardNum, string sEventInfo,
    TINT64 ddwMailId /*= 0*/, TINT64 ddwSuid /*= 0*/, TINT32 dwWindowOpt)
{
    //{//content
    //      event_type: //event type
    //      reward_type : //(0:goal 1:rank)
    //      num : //goal_num or rank_num 
    //      score_get : //玩家获取的分数
    //      score_list : [1000, 2000, 5000] //活动分数分布
    //      event_id:
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
    //          id:   // mail id
    //          suid : // sender uid
    //      }
    //}
    /*{
        event_type: //event type
        theme_event: //(0:no 1:yes)
        reward_type: //(0:goal 1:rank)
        num: //goal_num or rank_num
        event_ui: //活动label ui
        score_get: //玩家获取的分数
        score_list:[1000,2000,5000] //活动分数分布
        event_id:
        window_option:  //(0:城内城外弹窗 1：只在城内弹窗)
        reward:
        [
        [
        //type,
        //id
        //num
        ]
        ],
        mail:
        {
        id:   // mail id
        suid: // sender uid
        }
    }*/
    if(pstUser->m_udwEventTipsNum >= MAX_EVENT_TIPS_NUM * 2)
    {
        return 1;
    }
    TUINT32 udwEventEndTime = 0;
    TUINT64 uddwEventId = 0;
    TUINT32 udwEventUI = 0;

    Json::Value jEventInfo;
    jEventInfo.clear();
    Json::Reader reader;
    TBOOL bPase = reader.parse(sEventInfo.c_str(), jEventInfo);
    if(!bPase)
    {
        udwEventEndTime = 0;
        uddwEventId = 0;
    }
    else if (udwEventType != EN_EVENT_TYPE__THEME)
    {
        udwEventEndTime = jEventInfo["e_t"].asUInt();
        uddwEventId = jEventInfo["e_id"].asUInt64();
    }
    else
    {
        udwEventEndTime = jEventInfo["e_t"].asUInt();
        uddwEventId = jEventInfo["e_id"].asUInt64();
        udwEventUI = jEventInfo["event_ui"].asUInt();
    }
    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("CSendMessageBase::AddEventTips: [e_id=%ld] [e_time=%u] [seq=%u]", uddwEventId, udwEventEndTime, pstUser->m_udwBSeqNo));

    TbEvent_tips *ptbEventTips = &pstUser->m_atbEventTips[pstUser->m_udwEventTipsNum];
    ptbEventTips->Set_Uid(pstUser->m_tbPlayer.m_nUid);

    ptbEventTips->Set_Time(CTimeUtils::GetUnixTime());
    ptbEventTips->Set_Event_end_time(udwEventEndTime);

    Json::Value jEventTipsContent;
    jEventTipsContent = Json::Value(Json::objectValue);
    jEventTipsContent["theme_event"] = 0;
    jEventTipsContent["event_type"] = udwEventType;
    jEventTipsContent["reward_type"] = udwRewardType;
    jEventTipsContent["num"] = udwGoalorRank;
    jEventTipsContent["score_get"] = udwScoreGet;
    jEventTipsContent["event_id"] = uddwEventId;
    jEventTipsContent["window_option"] = dwWindowOpt;
    if (udwEventType == EN_EVENT_TYPE__THEME)
    {
        jEventTipsContent["theme_event"] = 1;
        jEventTipsContent["event_ui"] = udwEventUI;
    }

    ptbEventTips->Set_Id(CToolBase::GetPlayerNewTaskId(pstUser->m_tbPlayer.m_nUid, pstUser->m_tbLogin.m_nSeq + jEventTipsContent["theme_event"].asInt64()));
    pstUser->m_tbLogin.Set_Seq(pstUser->m_tbLogin.m_nSeq + 1); //llt add,防止,同一个用户请求中,生成多个action的情况

    Json::Value &jScoreList = jEventTipsContent["score_list"];
    jScoreList = Json::Value(Json::arrayValue);
    vector<TUINT32> vScoreList;
    CCommonFunc::GetVectorFromString(sScoreList.c_str(), ':', vScoreList);
    for (vector<TUINT32>::iterator it = vScoreList.begin(); it < vScoreList.end(); ++it)
    {
        jScoreList.append(*it);
    }
    TSE_LOG_INFO(CGameInfo::GetInstance()->m_poLog, ("uid=%u score_list=%s vector_size=%u [seq=%u]", pstUser->m_tbPlayer.m_nUid, sScoreList.c_str(), vScoreList.size(), pstUser->m_udwBSeqNo));

    Json::Value &jRewardList = jEventTipsContent["reward"];
    jRewardList = Json::Value(Json::arrayValue);
    for (TUINT32 udwIdx = 0; udwIdx < udwRewardNum; ++udwIdx)
    {
        jRewardList[udwIdx].append(astRewardList[udwIdx].ddwType);
        jRewardList[udwIdx].append(astRewardList[udwIdx].ddwId);
        jRewardList[udwIdx].append(astRewardList[udwIdx].ddwNum);
    }
    Json::Value &jMail = jEventTipsContent["mail"];
    jMail = Json::Value(Json::objectValue);
    jMail["id"] = ddwMailId;
    jMail["suid"] = ddwSuid;

    //for log
    Json::FastWriter writer;
    writer.omitEndingLineFeed();
    string jContent = writer.write(jEventTipsContent);
    TSE_LOG_INFO(CGameInfo::GetInstance()->m_poLog, ("AddEventTips: [json content=%s] [seq=%u]", jContent.c_str(), pstUser->m_udwBSeqNo));

    ptbEventTips->Set_Content(jEventTipsContent);

    pstUser->m_aucEventTipsFlag[pstUser->m_udwEventTipsNum] = EN_TABLE_UPDT_FLAG__NEW;
    pstUser->m_udwEventTipsNum++;

    return 0;
}

TINT32 CSendMessageBase::AddCommonTips(SUserInfo* pstUser, TUINT32 udwUid, TBOOL bIsNeedPut, SBuffDetail* pstItemEffect, SSpGlobalRes *pstEffectInfo, TINT32 dwItemId, TINT64 ddwTargetId)
{
    if(pstUser->m_udwTipsNum >= MAX_PLAYER_TIPS_NUM * 2)
    {
        return 1;
    }

    
    Json::Value jsonContent;
    jsonContent = Json::Value(Json::objectValue);
    if(pstItemEffect->m_dwTime == 0
    && pstItemEffect->m_udwId == 0
    && pstItemEffect->m_ddwNum == 0)
    {
        //Do nothing
    }
    else
    {
        jsonContent["buff"] = Json::Value(Json::arrayValue);
        jsonContent["buff"].append(pstItemEffect->m_udwId);
        jsonContent["buff"].append(pstItemEffect->m_ddwNum);
        jsonContent["buff"].append(pstItemEffect->m_dwTime);
    }

    if(pstEffectInfo->udwTotalNum > 0)
    {
        jsonContent["reward"] = Json::Value(Json::arrayValue);
        for(TUINT32 udwIdx = 0; udwIdx < pstEffectInfo->udwTotalNum; ++udwIdx)
        {
            jsonContent["reward"][udwIdx] = Json::Value(Json::arrayValue);
            jsonContent["reward"][udwIdx].append(pstEffectInfo->aRewardList[udwIdx].udwType);
            jsonContent["reward"][udwIdx].append(pstEffectInfo->aRewardList[udwIdx].udwId);
            jsonContent["reward"][udwIdx].append(pstEffectInfo->aRewardList[udwIdx].udwNum);
        }
    }

    jsonContent["item_id"] = dwItemId;
    jsonContent["target_id"] = ddwTargetId;


    TbTips *ptbTips = &pstUser->m_atbTips[pstUser->m_udwTipsNum];

    ptbTips->Set_Uid(udwUid);
    ptbTips->Set_Time(CTimeUtils::GetCurTimeUs());
    ptbTips->Set_Type(EN_TIPS_TYPE__COMMON_REWARD);
    ptbTips->Set_Content(jsonContent.toStyledString());
    if(bIsNeedPut)
    {
        pstUser->m_aucTipsFlag[pstUser->m_udwTipsNum] = EN_TABLE_UPDT_FLAG__NEW;
    }
    pstUser->m_udwTipsNum++;

    return 0;
}

TVOID CSendMessageBase::SendSysMsgToChat(TINT32 dwSid, TINT64 ddwAlid, TINT32 dwMsgType, string strCustomizeParam)
{    
    const TINT32 k_tmp_len = 1024;
    TCHAR szScriptStr[k_tmp_len];

    
    TCHAR szEncodeBuf[256];
    CHttpUtils::url_encode(strCustomizeParam.c_str(), szEncodeBuf, 256);


    sprintf(szScriptStr, "./send_sys_msg_to_chat.sh \"%d\" \"%ld\" \"%d\" \"%s\"", \
                         dwSid, ddwAlid, dwMsgType, szEncodeBuf);

    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("SendSysMsgToChat: [%s]",
                                                      szScriptStr));

    CMsgBase::SendDelaySystemMsg(szScriptStr);

    return;
}

TINT32 CSendMessageBase::SendBroadcast(SUserInfo *pstUser, TUINT32 udwSid, TUINT32 udwUid, TINT32 dwContentId, const string& sReplaceData, const string& sParam)
{
    const Json::Value& jBroadcast = CGameInfo::GetInstance()->m_oJsonRoot["game_broadcast"];
    if(!jBroadcast.isMember(CCommonFunc::NumToString(dwContentId)))
    {
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("SendBroadcast: can't find such cid in game json [cid=%d] [seq=%u]",
            dwContentId, pstUser->m_udwBSeqNo));
        return -1;
    }
    TINT64 ddwStrategy = jBroadcast[CCommonFunc::NumToString(dwContentId)]["strategy"].asInt64();
    TINT32 dwTarget = jBroadcast[CCommonFunc::NumToString(dwContentId)]["target"].asInt();

    if(ddwStrategy == EN_BROADCAST_STRATEGY__REAL_TIME)
    {
        if(pstUser->m_udwBroadcastNum >= MAX_BROADCAST_NUM_TOTAL)
        {
            TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("SendBroadcast: broadcast full [broadcast num=%u] [seq=%u]",
                pstUser->m_udwBroadcastNum, pstUser->m_udwBSeqNo));
            return -2;
        }
        TbBroadcast *ptbBroadcast = &pstUser->m_atbBroadcast[pstUser->m_udwBroadcastNum];
        if (dwTarget == EN_BROADCAST_TARGET__SERVER)
        {
            ptbBroadcast->Set_Key(CSendMessageBase::GetBroadcastKeyBySid(udwSid));
        }
        else
        {
            TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("SendBroadcast: target error [target=%d] [seq=%u]",
                dwTarget, pstUser->m_udwBSeqNo));
            return -3;
        }
        ptbBroadcast->Set_Ctime(CTimeUtils::GetCurTimeUs());
        ptbBroadcast->Set_Cid(dwContentId);
        ptbBroadcast->Set_Replace_data(sReplaceData);
        ptbBroadcast->Set_Param(sParam);
        pstUser->m_aucBroadcastFlag[pstUser->m_udwBroadcastNum] = EN_TABLE_UPDT_FLAG__NEW;
        pstUser->m_udwBroadcastNum++;

        TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("SendBroadcast: [key=%ld] [ctime=%ld] [type=%ld] [strategy=%ld] [priority=%ld] [cid=%ld] [replace:%s] [param:%s] [seq=%u]",
            ptbBroadcast->m_nKey, ptbBroadcast->m_nCtime, ptbBroadcast->m_nType,
            ptbBroadcast->m_nStrategy, ptbBroadcast->m_nPriority, ptbBroadcast->m_nCid,
            ptbBroadcast->m_sReplace_data.c_str(), ptbBroadcast->m_sParam.c_str(),
            pstUser->m_udwBSeqNo));
    }
    else
    {
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("SendBroadcast: strategy error [strategy=%ld] [seq=%u]",
            ddwStrategy, pstUser->m_udwBSeqNo));
        return -6;
    }

    return 0;
}

TINT64 CSendMessageBase::GetBroadcastKeyBySid(TINT32 dwSid)
{
    TUINT64 uddwResult = 0U;
    uddwResult = static_cast<TUINT32>(dwSid);
    uddwResult = uddwResult << 32;
    return static_cast<TINT64>(uddwResult);
}

TINT32 CSendMessageBase::GetTips( TbTips *ptbTips, TUINT8 ucType, TUINT32 udwUid, TUINT32 udwKey0 /*= 0*/, TUINT32 udwKey1 /*= 0*/, TUINT32 udwKey2 /*= 0*/, const TCHAR *pszKey0 /*= ""*/, const TCHAR *pszKey1 /*= ""*/, const TCHAR *pszKey2 /*= ""*/ )
{
    TCHAR szContent[256];
    sprintf(szContent, "%u&%u&%u&%s&%s&%s", udwKey0, udwKey1, udwKey2, pszKey0, pszKey1, pszKey2);

    ptbTips->Set_Uid(udwUid);
    ptbTips->Set_Time(CTimeUtils::GetCurTimeUs());
    ptbTips->Set_Type(ucType);
    ptbTips->Set_Content(szContent);

    return 0;
}

TINT32 CSendMessageBase::GetTips( TbTips *ptbTips, TUINT32 udwUid, TUINT8 ucType, const string& strContent )
{
    ptbTips->Set_Uid(udwUid);
    ptbTips->Set_Time(CTimeUtils::GetCurTimeUs());
    ptbTips->Set_Type(ucType);
    ptbTips->Set_Content(strContent);

    return 0;
}
