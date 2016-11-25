#include "rally_history_json.h"
#include "common_json.h"
CRallyHistoryJson::CRallyHistoryJson()
{

}

CRallyHistoryJson::~CRallyHistoryJson()
{

}

TVOID CRallyHistoryJson::GenDataJson(SSession* pstSession, Json::Value& rJson)
{
    //"svr_rally_history":[
    //    {
    //        "attacker":[int, string, string, int, int] // ����id, ���˼��, �������, 0��ʾӮ 1��ʾ��, Ұ������
    //        "defender":[int, string, string, int, int] // ����id, ���˼��, �������, 0��ʾӮ 1��ʾ��, Ұ������
    //        "report" : long,                   // ��ص�reportid
    //        "time" : int,                      // unixtime
    //    }
    //]
    rJson = Json::Value(Json::objectValue);
    Json::Value& jsonRallyHistory = rJson["svr_rally_history"];
    jsonRallyHistory = Json::Value(Json::arrayValue);

    for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_udwTmpRallyHistoryNum; ++udwIdx)
    {
        pstSession->m_atbTmpRallyHistory[udwIdx].m_jContent["report"] =
            pstSession->m_atbTmpRallyHistory[udwIdx].m_nRid;
        jsonRallyHistory.append(pstSession->m_atbTmpRallyHistory[udwIdx].m_jContent);
    }
}
