#include "event_json.h"
#include "time_utils.h"
#include "action_base.h"
#include "player_info.h"
#include "common_json.h"
#include "game_define.h"
#include "game_command.h"
#include <map>
using namespace std;

CEventJson::CEventJson()
{

}

CEventJson::~CEventJson()
{

}

TVOID CEventJson::GenDataJson(SSession* pstSession, Json::Value& rJson)
{
    /*
    "svr_event_info":
    {
        //event_info
    }
    */
    /*if(pstSession->m_udwCommand == EN_CLIENT_ALL_EVENT_GET ||
        EN_CLIENT_EVENT_INFO_GET == pstSession->m_udwCommand)
    {*/
    if (pstSession->m_udwCommand == EN_CLIENT_REQ_COMMAND__ALL_EVENT_GET)
        {
        
        Json::Value& jsonEventInfo = rJson["svr_event_list"];
        Json::Value& jThemeEventInfo = rJson["svr_theme_event_list"];

        jsonEventInfo = Json::Value(Json::objectValue);

        Json::Reader reader;

        Json::Value oEventListJson;
        /*
        if(pstSession->m_vecEventRsp.size() != 0
            && pstSession->m_vecEventRsp[0]->dwRetCode == 0
            && FALSE != reader.parse(pstSession->m_vecEventRsp[0]->sRspContent, oEventListJson))
        {
            Json::Value &oJson = oEventListJson["svr_event_list"];

            //解析出下游返回数据后  对数据进行修正
            Json::Value::Members oJsonMembers = oJson.getMemberNames();

            TUINT64 uddwBTime = CTimeUtils::GetCurTimeUs();
            TSE_LOG_INFO(pstSession->m_poServLog, ("CEventJson:BTime=%lu, seq=%u", uddwBTime, pstSession->m_udwSeqNo));
            for(Json::Value::Members::iterator it = oJsonMembers.begin(); it != oJsonMembers.end();++it)
            {
                if(oJson[(*it).c_str()].isNull())
                {
                    continue;
                }
               
                if(!oJson[(*it).c_str()].isMember("basic"))
                {
                    continue;
                }
                if(oJson[(*it).c_str()]["result"].isMember("self_result"))
                {
                    //oJson[(*it).c_str()]["result"]["self_result"]["name"] = pstSession->m_stUserInfo.m_tbPlayer.m_sUin;
                    //oJson[(*it).c_str()]["result"]["self_result"]["al_short_name"] = pstSession->m_stUserInfo.m_tbPlayer.m_sAl_nick_name;
                    oJson[(*it).c_str()]["result"]["self_result"]["name"] = "";
                    oJson[(*it).c_str()]["result"]["self_result"]["al_short_name"] = "";
                }

                if(oJson[(*it).c_str()]["basic"]["type"].asUInt() != 0 &&
                    !oJson[(*it).c_str()]["result"].isMember("self_alliance_member"))
                {
                    continue;
                }
                else
                {
                    Json::Value &jAlMemberResult = oJson[(*it).c_str()]["result"]["self_alliance_member"];

                    map<TUINT64, TUINT64> mapAlmemberResult;
                    map<TUINT64, TUINT64>::iterator itmapAlmemberResult;
                    if(0 != jAlMemberResult.size())
                    {
                        for(TUINT32 udwIdx = 0; udwIdx < jAlMemberResult.size(); ++udwIdx)
                        {
                            mapAlmemberResult.insert(make_pair(jAlMemberResult[udwIdx]["uid"].asUInt(), jAlMemberResult[udwIdx]["score"].asUInt()));
                        }
                    }

                    set<TUINT64> setAlmember;
                    set<TUINT64>::iterator itsetAlmember;
                    if(pstSession->m_udwTmpAlmemberNum > 0)
                    {
                        for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_udwTmpAlmemberNum; ++udwIdx)
                        {
                            TbAl_member tbAlMember = pstSession->m_atbTmpAlmember[udwIdx];
                            setAlmember.insert(tbAlMember.m_nUid);
                        }
                    }

                    Json::Value oAlEventListJson;
                    
                    if(pstSession->m_udwTmpAlmemberNum > 0)
                    {
                        for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_udwTmpAlmemberNum; ++udwIdx)
                        {
                            TbAl_member tbAlMember = pstSession->m_atbTmpAlmember[udwIdx];

                            Json::Value OneUserInfo;
                            OneUserInfo = Json::Value(Json::objectValue);
                            OneUserInfo["uid"] = tbAlMember.m_nUid;
                            itmapAlmemberResult = mapAlmemberResult.find(tbAlMember.m_nUid);
                            if(itmapAlmemberResult == mapAlmemberResult.end())
                            {
                                OneUserInfo["score"] = 0;
                                OneUserInfo["exist"] = 1;
                            }
                            else
                            {
                                OneUserInfo["score"] = itmapAlmemberResult->second;
                                OneUserInfo["exist"] = 1;
                            }
                            OneUserInfo["name"] = tbAlMember.m_jProfile["uname"];
                            OneUserInfo["alpos"] = tbAlMember.m_jProfile["alpos"];
                            
                            oAlEventListJson.append(OneUserInfo);
                        }
                    }

                    if(0 != jAlMemberResult.size())
                    {
                        for(TUINT32 udwIdx = 0; udwIdx < jAlMemberResult.size(); ++udwIdx)
                        {
                            itsetAlmember = setAlmember.find(jAlMemberResult[udwIdx]["uid"].asUInt());
                            
                            if(itsetAlmember == setAlmember.end())
                            {
                                oAlEventListJson.append(jAlMemberResult[udwIdx]);
                            }
                        }
                    }
                    jAlMemberResult = oAlEventListJson;
              }
            }
            jsonEventInfo = oJson;
            //reader.parse(pstSession->m_vecEventRsp[0]->sRspContent, jsonEventInfo);
            TUINT64 uddwETime = CTimeUtils::GetCurTimeUs();
            TSE_LOG_INFO(pstSession->m_poServLog, ("CEventJson:ETime=%lu, CTime=%lu, seq=%u", uddwETime, uddwETime - uddwBTime, pstSession->m_udwSeqNo));
        }
        */
        if (pstSession->m_vecEventRsp[0]->dwRetCode == 0
            && FALSE != reader.parse(pstSession->m_vecEventRsp[0]->sRspContent, oEventListJson))
        {
            jThemeEventInfo = oEventListJson["svr_event_list"];
        }
        if (pstSession->m_vecEventRsp[1]->dwRetCode == 0
            && FALSE != reader.parse(pstSession->m_vecEventRsp[1]->sRspContent, oEventListJson))
        {
            jThemeEventInfo = oEventListJson["svr_theme_event_list"];
        }

        return;
    }
}

