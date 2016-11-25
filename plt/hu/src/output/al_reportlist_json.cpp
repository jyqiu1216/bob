#include "al_reportlist_json.h"
#include "common_json.h"
#include <math.h>

CAlReportListJson::CAlReportListJson()
{

}

CAlReportListJson::~CAlReportListJson()
{

}

TVOID CAlReportListJson::GenDataJson(SSession* pstSession, Json::Value& rJson)
{
    //"svr_al_report":
    //{
    //    "total_page_num" : 255,
    //    "current_page_count" : 20,
    //    "list" :
    //    [
    //        {
    //            "base":[
    //                6,              //report id
    //                1421323823,     //unix time
    //                3,              //report type
    //                0,              //result 见EReportResult
    //            ],
    //            "from":[
    //                1153387,        //uid
    //                190094,         //cid
    //                7,              //aid
    //                0,              //wild type
    //                5,              //wild level
    //            ],
    //            "to":[
    //                1153387,        //uid
    //                190094,         //cid
    //                7,              //aid
    //                0,              //wild type
    //                5,              //wild level
    //            ],
    //            "name_info":[
    //                "suname",           //发起者名字
    //                "scityname",        //发起者城市名字
    //                "salname",          //发起者联盟名字
    //                "tuname",           //目标玩家名字
    //                "tcityname",        //目标玩家城市名字
    //                "talname",          //目标玩家联盟名字
    //            ],
    //            "content": {        //不同report type 不一样 见下方 
    //            }
    //        }
    //    ]
    //}
    Json::Value& jsonAlReport = rJson["svr_al_report"];
    jsonAlReport = Json::Value(Json::objectValue);
    jsonAlReport["total_page_num"] = ceil(pstSession->m_stUserInfo.m_stReportUserInfo.m_udwReportTotalNum / 20.0);
    jsonAlReport["current_page_count"] = pstSession->m_stUserInfo.m_udwReportNum;
    jsonAlReport["list"] = Json::Value(Json::arrayValue);

    TbReport *ptbReport = NULL;
    for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_stUserInfo.m_udwReportNum; ++udwIdx)
    {
        ptbReport = &pstSession->m_stUserInfo.m_atbReportList[udwIdx];
        CCommJson::GenReportInfo(ptbReport, FALSE, jsonAlReport["list"][udwIdx]);
    }
}

