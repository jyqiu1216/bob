#include "reportlist_json.h"
#include "common_json.h"
#include "game_command.h"

CReportListJson::CReportListJson()
{

}

CReportListJson::~CReportListJson()
{

}

TVOID CReportListJson::GenDataJson(SSession* pstSession, Json::Value& rJson)
{
    if(pstSession->m_udwCommand == EN_CLIENT_REQ_COMMAND__REPORT_GET)
    {
        //report_get
        //"svr_report_total_list":
        //{
        //    "cur_page": int,        //��ǰ����ҳ��
        //    "total_num" : int,       //������
        //    "unread_num" : int,      //δ������
        //    "entries" : [            //�ۺϺ�report���б�
        //    {
        //        "total_num" : int,  //�ۺ�չʾ��÷����������
        //        "unread_num" : int, //�ۺ�չʾ��÷����δ������
        //        "head_report" : {       //չʾ�ĵ�һ��report
        //            "base":[
        //                long,              //report id
        //                int,               //unix time
        //                int,               //report type
        //                int,               //result ��EReportResult
        //            ],
        //            "from":[
        //                long,        //uid
        //                int,         //cid
        //                long,        //aid
        //                int,         //wild type
        //                int,         //wild level
        //            ],
        //            "to":[
        //                long,        //uid
        //                int,         //cid
        //                long,        //aid
        //                int,         //wild type
        //                int,         //wild level
        //            ],
        //            "name_info":[
        //                string,           //����������
        //                string,           //�����߳�������
        //                string,           //��������������
        //                string,           //Ŀ���������
        //                string,           //Ŀ����ҳ�������
        //                string,           //Ŀ�������������
        //            ],
        //            "content": {        //��ͬreport type ��һ�� ���·� 
        //            },
        //            "read" : int        //0��ʾδ��,1��ʾ�Ѷ�
        //        }
        //    }]
        //}
        SUserInfo* pstUser = &pstSession->m_stUserInfo;
        SReportUserRspInfo *pstReportUserInfo = &pstUser->m_stReportUserInfo;
        Json::Value& jsonReportList = rJson["svr_report_total_list"];
        jsonReportList = Json::Value(Json::objectValue);

        jsonReportList["cur_page"] = pstSession->m_stReqParam.m_udwPage;
        jsonReportList["total_num"] = pstReportUserInfo->m_udwReportTotalNum;
        jsonReportList["unread_num"] = pstReportUserInfo->m_udwReportUnreadNum;

        jsonReportList["entries"] = Json::Value(Json::arrayValue);
        TUINT32 udwCount = 0;

        for (TUINT32 udwIdx = 0; udwIdx < pstReportUserInfo->m_udwReportEntryNum; ++udwIdx)
        {
            TbReport* ptbReport = NULL;
            for(TUINT32 udwIdy = 0; udwIdy < pstUser->m_udwReportNum; ++udwIdy)
            {
                if (pstUser->m_atbReportList[udwIdy].m_nId == pstReportUserInfo->m_aReportToReturn[udwIdx].ddwRid)
                {
                    ptbReport = &pstUser->m_atbReportList[udwIdy];
                    break;
                }
            }
            if(!ptbReport)
            {
                continue;
            }
            jsonReportList["entries"][udwCount] = Json::Value(Json::objectValue);
            CCommJson::GenReportInfo(ptbReport, &pstReportUserInfo->m_aReportToReturn[udwIdx], FALSE, jsonReportList["entries"][udwCount]["head_report"]);
            jsonReportList["entries"][udwCount]["unread_num"] = pstReportUserInfo->m_aReportToReturn[udwIdx].dwUnread;
            jsonReportList["entries"][udwCount]["total_num"] = pstReportUserInfo->m_aReportToReturn[udwIdx].dwNum;
            udwCount++;
        }

        Json::Value& rJsonStat = rJson["svr_stat"];
        CCommJson::GenStatJson(pstSession->m_stRecommendTime.ddwTime,
            pstSession->m_udwRecommendNum,
            pstSession->m_bNeedMailMusic,
            &pstSession->m_stUserInfo.m_tbUserStat,
            rJsonStat);
    }
    if(pstSession->m_udwCommand == EN_CLIENT_REQ_COMMAND__REPORT_DETAIL_GET)
    {
        //"svr_report_detail_list":
        //{
        //    "cur_page": int,        //��ǰ����ҳ��
        //    "total_num" : int,       //������
        //    "unread_num" : int,      //δ������
        //    "entries" : [            //report���б�
        //    {
        //        "base":[
        //            long,              //report id
        //            int,               //unix time
        //            int,               //report type
        //            int,               //result ��EReportResult
        //        ],
        //        "from":[
        //            long,        //uid
        //            int,         //cid
        //            long,        //aid
        //            int,         //wild type
        //            int,         //wild level
        //        ],
        //        "to":[
        //            long,        //uid
        //            int,         //cid
        //            long,        //aid
        //            int,         //wild type
        //            int,         //wild level
        //        ],
        //        "name_info":[
        //            string,           //����������
        //            string,           //�����߳�������
        //            string,           //��������������
        //            string,           //Ŀ���������
        //            string,           //Ŀ����ҳ�������
        //            string,           //Ŀ�������������
        //        ],
        //        "content": {        //��ͬreport type ��һ�� ���·� 
        //        },
        //        "read" : int        //0��ʾδ��,1��ʾ�Ѷ�
        //    }]
        //}
        SUserInfo* pstUser = &pstSession->m_stUserInfo;
        SReportUserRspInfo *pstReportUserInfo = &pstUser->m_stReportUserInfo;
        Json::Value& jsonReportList = rJson["svr_report_detail_list"];
        jsonReportList = Json::Value(Json::objectValue);

        jsonReportList["cur_page"] = pstSession->m_stReqParam.m_udwPage;
        jsonReportList["total_num"] = pstReportUserInfo->m_udwReportTotalNum;
        jsonReportList["unread_num"] = pstReportUserInfo->m_udwReportUnreadNum;

        jsonReportList["entries"] = Json::Value(Json::arrayValue);
        TUINT32 udwCount = 0;

        for (TUINT32 udwIdx = 0; udwIdx < pstReportUserInfo->m_udwReportEntryNum; ++udwIdx)
        {
            TbReport* ptbReport = NULL;
            for(TUINT32 udwIdy = 0; udwIdy < pstUser->m_udwReportNum; ++udwIdy)
            {
                if (pstUser->m_atbReportList[udwIdy].m_nId == pstReportUserInfo->m_aReportToReturn[udwIdx].ddwRid)
                {
                    ptbReport = &pstUser->m_atbReportList[udwIdy];
                    break;
                }
            }
            if(!ptbReport)
            {
                continue;
            }
            jsonReportList["entries"][udwCount] = Json::Value(Json::objectValue);
            CCommJson::GenReportInfo(ptbReport, &pstReportUserInfo->m_aReportToReturn[udwIdx], FALSE, jsonReportList["entries"][udwCount]);
            udwCount++;
        }

        Json::Value& rJsonStat = rJson["svr_stat"];
        CCommJson::GenStatJson(pstSession->m_stRecommendTime.ddwTime,
            pstSession->m_udwRecommendNum,
            pstSession->m_bNeedMailMusic,
            &pstSession->m_stUserInfo.m_tbUserStat,
            rJsonStat);
    }
    if(pstSession->m_udwCommand == EN_CLIENT_OPERATE_COMMAND__REPORT_GET)
    {
        //"svr_report_info":
        //{
        //    "base":[
        //        long,              //report id
        //        int,               //unix time
        //        int,               //report type
        //        int,               //result ��EReportResult
        //    ],
        //    "from":[
        //        long,        //uid
        //        int,         //cid
        //        long,        //aid
        //        int,         //wild type
        //        int,         //wild level
        //    ],
        //    "to":[
        //        long,        //uid
        //        int,         //cid
        //        long,        //aid
        //        int,         //wild type
        //        int,         //wild level
        //    ],
        //    "name_info":[
        //        string,           //����������
        //        string,           //�����߳�������
        //        string,           //��������������
        //        string,           //Ŀ���������
        //        string,           //Ŀ����ҳ�������
        //        string,           //Ŀ�������������
        //    ],
        //    "content": {        //��ͬreport type ��һ�� ���·� 
        //        },
        //    "read" : int        //0��ʾδ��,1��ʾ�Ѷ�,�˴�������
        //}
        Json::Value& jsonReportInfo = rJson["svr_report_info"];
        jsonReportInfo = Json::Value(Json::objectValue);
        if(pstSession->m_stUserInfo.m_udwReportNum > 0)
        {
            SReportEntry stTmp;
            stTmp.Reset();
            CCommJson::GenReportInfo(pstSession->m_stUserInfo.m_ptbReport[0], &stTmp, TRUE, jsonReportInfo);
        }
    }
}