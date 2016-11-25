#include "rallywar_info_json.h"
#include "action_base.h"
#include "common_json.h"

CRallyWarInfoJson::CRallyWarInfoJson()
{

}

CRallyWarInfoJson::~CRallyWarInfoJson()
{

}

TVOID CRallyWarInfoJson::GenDataJson(SSession* pstSession, Json::Value& rJson)
{
    //"svr_rally_war_info":
    //{
        //"city_info":
        //{
        //    "uid": long,
        //    "alnick" : string,
        //    "uname" : string,
        //    "hero" : [string, int, int], //英雄名字,等级,图标  //等级为0表示没有英雄
        //    "troop_num" : long,
        //    "troop_force" : long,
        //    "troop" : [long.....] //下标为troop id，值表示具体数量
        //},
    //    "main_action" : {},
    //    "reinforce_attacker" : [],
    //    "reinforce_defender" : []
    //}
    SUserInfo* pstUser = &pstSession->m_stUserInfo;
    TINT64 ddwRallyWarId = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TINT32 dwSide = atoi(pstSession->m_stReqParam.m_szKey[1]);

    TbMarch_action* ptbRallyWar = NULL;
    if(dwSide == EN_RALLY_SIDE_ATTACK)
    {
        ptbRallyWar = CActionBase::GetMarch(pstUser->m_atbMarch, pstUser->m_udwMarchNum, ddwRallyWarId);

    }
    else
    {
        ptbRallyWar = CActionBase::GetMarch(pstUser->m_atbPassiveMarch, pstUser->m_udwPassiveMarchNum, ddwRallyWarId);
    }

    rJson = Json::Value(Json::objectValue);
    if(ptbRallyWar == NULL)
    {
        return;
    }

    Json::Value& jsonRallyWarInfo = rJson["svr_rally_war_info"];
    jsonRallyWarInfo = Json::Value(Json::objectValue);

    if(!ptbRallyWar->m_jCity_info.empty())
    {
        jsonRallyWarInfo["city_info"] = ptbRallyWar->m_jCity_info;

        //wave@20160804: 处理bug
        if (jsonRallyWarInfo["city_info"].isMember("knight") == false)
        {
            jsonRallyWarInfo["city_info"]["knight"] = Json::Value(Json::arrayValue);
            jsonRallyWarInfo["city_info"]["knight"].append(-1);
            jsonRallyWarInfo["city_info"]["knight"].append(0);
        }
        if (!jsonRallyWarInfo["city_info"].isMember("troop"))
        {
            jsonRallyWarInfo["city_info"]["troop"] = Json::Value(Json::arrayValue);
        }
    }
    else
    {
        jsonRallyWarInfo["city_info"] = Json::Value(Json::nullValue);
    }

    //修正rally war等待时间
//     if(ptbRallyWar->m_nStatus == EN_MARCH_STATUS__WAITING && ptbRallyWar->m_nEtime == INT64_MAX
//         && dwSide == EN_RALLY_SIDE_ATTACK)
//     {
//         TINT64 ddwEndTime = 0;
//         for(TUINT32 udwIdy = 0; udwIdy < pstUser->m_udwRallyMarchNum; udwIdy++)
//         {
//             //reinforce rally / reinforce throne
//             if(pstUser->m_atbRallyMarch[udwIdy].m_nTuid = ptbRallyWar->m_nSuid
//                 && pstUser->m_atbRallyMarch[udwIdy].m_nTid == ddwRallyWarId
//                 && pstUser->m_atbRallyMarch[udwIdy].m_nStatus == EN_MARCH_STATUS__MARCHING)
//             {
//                 if (pstUser->m_atbRallyMarch[udwIdy].m_nEtime > ddwEndTime)
//                 {
//                     ddwEndTime = pstUser->m_atbRallyMarch[udwIdy].m_nEtime;
//                 }
//             }
//         }
//         ptbRallyWar->m_nEtime = ddwEndTime;  //不会更新到AWS
//     }

    CCommJson::GenMarchInfo(ptbRallyWar, jsonRallyWarInfo["main_action"]);

    if(dwSide == EN_RALLY_SIDE_ATTACK)
    {
        jsonRallyWarInfo["reinforce_attacker"] = Json::Value(Json::arrayValue);
        TUINT32 udwJsonIndex = 0;
        for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwRallyMarchNum; ++udwIdx)
        {
            //reinforce rally / reinforce throne
            if (pstUser->m_atbRallyMarch[udwIdx].m_nTuid == ptbRallyWar->m_nSuid
                && pstUser->m_atbRallyMarch[udwIdx].m_nTid == ddwRallyWarId
                && pstUser->m_atbRallyMarch[udwIdx].m_nStatus != EN_MARCH_STATUS__RETURNING)
            {
                CCommJson::GenMarchInfo(&pstUser->m_atbRallyMarch[udwIdx], jsonRallyWarInfo["reinforce_attacker"][udwJsonIndex]);
                ++udwJsonIndex;
            }
        }
    }
    else
    {
        jsonRallyWarInfo["reinforce_defender"] = Json::Value(Json::arrayValue);
        TUINT32 udwJsonIndex = 0;
        for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwRallyMarchNum; ++udwIdx)
        {
            TbMarch_action *ptbReinforce = &pstUser->m_atbRallyMarch[udwIdx];
            if (ptbReinforce->m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_NORMAL 
                && ptbReinforce->m_nStatus != EN_MARCH_STATUS__RETURNING 
                && ptbReinforce->m_nTuid == ptbRallyWar->m_nTuid
                && ptbReinforce->m_nTpos == ptbRallyWar->m_nTpos)
            {
                CCommJson::GenMarchInfo(ptbReinforce, jsonRallyWarInfo["reinforce_defender"][udwJsonIndex]);
                ++udwJsonIndex;
            }
        }
    }
}
