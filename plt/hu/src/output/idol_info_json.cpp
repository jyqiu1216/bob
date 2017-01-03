#include "idol_info_json.h"

#include "common_func.h"

CIdolInfoJson::CIdolInfoJson()
{

}

CIdolInfoJson::~CIdolInfoJson()
{

}

TVOID CIdolInfoJson::GenDataJson(SSession* pstSession, Json::Value& rJson)
{
    Json::Value& jsonIdolInfo = rJson["svr_idol_detail_info"];
    jsonIdolInfo = Json::Value(Json::objectValue);

    TbIdol *ptbIdol = NULL;
//    TUINT32 udwPos = pstSession->m_tbTmpMap.m_nId;

    for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_udwIdolNum; udwIdx++)
    {
//         if (udwPos == pstSession->m_atbIdol[udwIdx].m_nPos)
//         {
//             ptbIdol = &pstSession->m_atbIdol[udwIdx];
//             break;
//         }
    
        ptbIdol = &pstSession->m_atbIdol[udwIdx];
        Json::Value& jOneIdol = jsonIdolInfo[CCommonFunc::NumToString(ptbIdol->m_nPos)];
        jOneIdol = Json::Value(Json::objectValue);
        jOneIdol["status"] = ptbIdol->m_nStatus;
        jOneIdol["end_time"] = ptbIdol->m_nEnd_time;

        jOneIdol["buff"] = Json::Value(Json::arrayValue);
        Json::Value::Members members = ptbIdol->m_jInfo["b"].getMemberNames();
        for (Json::Value::Members::iterator it = members.begin(); it != members.end(); it++)
        {
            jOneIdol["buff"].append(ptbIdol->m_jInfo["b"][*it]);
        }

        jOneIdol["rank"] = ptbIdol->m_jRank;
        if (!jOneIdol["rank"].isArray())
        {
            jOneIdol["rank"] = Json::Value(Json::arrayValue);
        }
        TBOOL bIsFind = FALSE;
        for (TUINT32 udwIdx = 0; udwIdx < ptbIdol->m_jRank.size(); udwIdx++)
        {
            if (pstSession->m_stReqParam.m_udwAllianceId == ptbIdol->m_jRank[udwIdx]["alid"].asUInt())
            {
                jOneIdol["self_rank"] = ptbIdol->m_jRank[udwIdx];
                bIsFind = TRUE;
                break;
            }
        }
        if (bIsFind == FALSE)
        {
            jOneIdol["self_rank"] = Json::Value(Json::objectValue);
            jOneIdol["self_rank"]["rank"] = 0;
            jOneIdol["self_rank"]["alid"] = -1;
            jOneIdol["self_rank"]["point"] = 0;
            jOneIdol["self_rank"]["al_nick"] = "";
            jOneIdol["self_rank"]["al_name"] = "";
        }

        jOneIdol["alliance"] = Json::Value(Json::objectValue);
        if (ptbIdol->m_nStatus == EN_IDOL_STATUS__BUFF_PERIOD && ptbIdol->m_jRank.size() > 0)
        {
            jOneIdol["alliance"]["alid"] = ptbIdol->m_jRank[0U]["alid"];
            jOneIdol["alliance"]["al_nick"] = ptbIdol->m_jRank[0U]["al_nick"];
            jOneIdol["alliance"]["al_name"] = ptbIdol->m_jRank[0U]["al_name"];
        }
        else
        {
            jOneIdol["alliance"]["alid"] = -1;
            jOneIdol["alliance"]["al_nick"] = "";
            jOneIdol["alliance"]["al_name"] = "";
        }
    }
}
