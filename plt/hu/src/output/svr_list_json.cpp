#include "svr_list_json.h"
#include "common_func.h"
#include "common_json.h"
CSvrListJson::CSvrListJson()
{

}

CSvrListJson::~CSvrListJson()
{

}

TVOID CSvrListJson::GenDataJson(SSession* pstSession, Json::Value& rJson)
{
//     {
//         "svr_list": {
//             "5010": 	//	中心位置
//             [
//                 0, 				// svr id
//                 "english", 		// language
//                 1384128000, 	// create time
//                 0, 				// server status: SERVER_STATUS
//                 1, 				// 是否是当前svr
//                 1, 				// 是否有账号
//                 0, 				// 是否保护中
//                 5010, 			// 中心位置
//                 "al_solar", 	// alliance
//                 "als", 			// al nick nam
//                 "solar", 		// king name
//                 101, 			// svr map type: SERVER_MAP_TYPE
//                 2, 				// block size
//                 1 				// alliance flag id
//             ],
//         }
//     }

    Json::Value& jsonSvrList = rJson["svr_list"];
    jsonSvrList = Json::Value(Json::objectValue);

    for(TUINT32 udwSvrIdx = 0; udwSvrIdx < pstSession->m_udwTmpSvrNum; ++udwSvrIdx)
    {
        TUINT32 udwSvrPos = pstSession->m_atbTmpSvrStat[udwSvrIdx].m_nPos;
        TUINT32 dwThroneMapIdx = 0;
        for(dwThroneMapIdx = 0; dwThroneMapIdx < pstSession->m_udwTmpSvrNum; dwThroneMapIdx++)
        {
            if(pstSession->m_atbTmpSvrStat[udwSvrIdx].m_nSid == pstSession->m_atbTmpThroneMap[dwThroneMapIdx].m_nSid)
            {
                break;
            }
        }
        if(dwThroneMapIdx == pstSession->m_udwTmpSvrNum)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("OUTPUT:can't find corresponding throne map [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
        }
        CSvrListJson::GenBaseDataJson(pstSession, jsonSvrList[CCommonFunc::NumToString(udwSvrPos)], udwSvrIdx, dwThroneMapIdx);
    }
    return;
}

TVOID CSvrListJson::GenBaseDataJson(SSession* pstSession, Json::Value& rJson, TUINT32 udwSvrIdx, TUINT32 udwThroneMapIdx)
{
    rJson = Json::Value(Json::arrayValue);
    TUINT32 udwLoginSvrFlag[MAX_GAME_SVR_NUM] = {0};
    TINT32 dwSvrFlag = 0;

    /*
    for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_stUserInfo.m_tbLogin.m_bSvrlist.m_udwNum; udwIdx++)
    {
        udwLoginSvrFlag[pstSession->m_stUserInfo.m_tbLogin.m_bSvrlist.m_astList[udwIdx]] = TRUE;
    }
    */
    udwLoginSvrFlag[pstSession->m_stUserInfo.m_tbLogin.m_nSid] = TRUE; 

    rJson[0U] = pstSession->m_atbTmpSvrStat[udwSvrIdx].m_nSid;
    rJson[1U] = pstSession->m_atbTmpSvrStat[udwSvrIdx].m_sLanguage;
    rJson[2U] = pstSession->m_atbTmpSvrStat[udwSvrIdx].m_nOpen_time;
    rJson[3U] = pstSession->m_atbTmpSvrStat[udwSvrIdx].m_nStatus;
    rJson[4U] = pstSession->m_stUserInfo.m_tbLogin.m_nSid == pstSession->m_atbTmpSvrStat[udwSvrIdx].m_nSid ? 1U : 0U;
    rJson[5U] = udwLoginSvrFlag[pstSession->m_atbTmpSvrStat[udwSvrIdx].m_nSid] ? 1U : 0U;

    dwSvrFlag = pstSession->m_atbTmpSvrStat[udwSvrIdx].m_nSwitch;
    if(udwLoginSvrFlag[pstSession->m_atbTmpSvrStat[udwSvrIdx].m_nSid])
    {
        dwSvrFlag = 1U;
    }
    rJson[6U] = dwSvrFlag == 0 ? 1 : 0;
    rJson[7U] = pstSession->m_atbTmpSvrStat[udwSvrIdx].m_nPos;
    rJson[8U] = pstSession->m_atbTmpThroneMap[udwThroneMapIdx].m_sAlname;
    rJson[9U] = pstSession->m_atbTmpThroneMap[udwThroneMapIdx].m_sAl_nick;
    rJson[10U] = pstSession->m_atbTmpThroneMap[udwThroneMapIdx].m_sUname;
    rJson[11U] = 101U;
    rJson[12U] = pstSession->m_atbTmpSvrStat[udwSvrIdx].m_nBlock_size;
    rJson[13U] = pstSession->m_atbTmpThroneMap[udwThroneMapIdx].m_nAl_flag;
    rJson[14U] = pstSession->m_atbTmpSvrStat[udwSvrIdx].m_nAvatar;

    return;
}
