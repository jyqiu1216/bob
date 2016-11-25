#include "json_result_generator.h"
#include "rc4.h"
#include "zlib.h"
#include "hu_work.h"

#include "user_json.h"
#include "alliancelist_json.h"
#include "maillist_json.h"
#include "wall_json.h"
#include "playerlist_json.h"
#include "map_json.h"
#include "reportlist_json.h"
#include "al_reportlist_json.h"
#include "buffer_info_json.h"
#include "assist_list_json.h"
#include "svr_list_json.h"
#include "event_json.h"
#include "op_task_condition_json.h"
#include "rallywar_info_json.h"
#include "rally_history_json.h"
#include "recommend_json.h"
#include "random_reward_json.h"
#include "jsoncpp_utils.h"
#include "global_serv.h"
#include "output_conf.h"
#include "common_logic.h"
#include "idol_info_json.h"
#include "throne_info_json.h"

CJsonResultGenerator::CJsonResultGenerator()
{
    m_jsonResult.clear();

    m_szDataBuffer = new TCHAR[MAX_NETIO_PACKAGE_BUF_LEN];
    m_szCompressBuffer = new TCHAR[MAX_NETIO_PACKAGE_BUF_LEN];
    m_szEncodeBuffer = new TCHAR[MAX_NETIO_PACKAGE_BUF_LEN];
    m_szResultBuffer = new TCHAR[MAX_NETIO_PACKAGE_BUF_LEN];
    m_szMD5Buffer = new TCHAR[MAX_NETIO_PACKAGE_BUF_LEN];

    m_bNeedCompress = FALSE;

    for(TUINT32 udwIdx = 0; udwIdx < EN_JSON_TYPE_END; ++udwIdx)
    {
        m_apJsonGen[udwIdx] = NULL;
    }
    m_apJsonGen[EN_JSON_TYPE_USER_JSON] = new CUserJson();
    m_apJsonGen[EN_JSON_TYPE_MAIL_LIST] = new CMailListJson();
    m_apJsonGen[EN_JSON_TYPE_ALLIANCE_LIST] = new CAllianceListJson();
    m_apJsonGen[EN_JSON_TYPE_PLAYER_LIST] = new CPlayerListJson();
    m_apJsonGen[EN_JSON_TYPE_WALL] = new CWallJson();
    m_apJsonGen[EN_JSON_TYPE_REPORT] = new CReportListJson();
    m_apJsonGen[EN_JSON_TYPE_AL_REPORT] = new CAlReportListJson();
    m_apJsonGen[EN_JSON_TYPE_MAP] = new CMapJson();
	m_apJsonGen[EN_JSON_TYPE_OPERATE_BUFFER_INFO] = new CBufferInfoJson();
    m_apJsonGen[EN_JSON_TYPE_ASSIST_LIST] = new CAssistListJson();
    m_apJsonGen[EN_JSON_TYPE_SVR_LIST] = new CSvrListJson();
    m_apJsonGen[EN_JSON_TYPE_EVENT] = new CEventJson();
    m_apJsonGen[EN_JSON_TYPE_OPERATE_TASK_INFO] = new COpTaskConditionJson();
    m_apJsonGen[EN_JSON_TYPE_RALLY_WAR_INFO] = new CRallyWarInfoJson();
    m_apJsonGen[EN_JSON_TYPE_RALLY_HISTORY] = new CRallyHistoryJson();
    m_apJsonGen[EN_JSON_TYPE_PLAYER_RECOMMEND] = new CRecommendJson();
    m_apJsonGen[EN_JSON_TYPE_RANDOM_REWARD] = new CRandomRewardJson();
    m_apJsonGen[EN_JSON_TYPE_IDOL_INFO] = new CIdolInfoJson();
    m_apJsonGen[EN_JSON_TYPE_THRONE_INFO] = new CThroneInfoJson();
}

CJsonResultGenerator::~CJsonResultGenerator()
{
    if(m_szDataBuffer != NULL)
    {
        delete[] m_szDataBuffer;
        m_szDataBuffer = NULL;
    }
    if(m_szCompressBuffer != NULL)
    {
        delete[] m_szCompressBuffer;
        m_szCompressBuffer = NULL;
    }
    if(m_szEncodeBuffer != NULL)
    {
        delete[] m_szEncodeBuffer;
        m_szEncodeBuffer = NULL;
    }
    if(m_szResultBuffer != NULL)
    {
        delete[] m_szResultBuffer;
        m_szResultBuffer = NULL;
    }
    if(m_szMD5Buffer != NULL)
    {
        delete[] m_szMD5Buffer;
        m_szMD5Buffer = NULL;
    }

    for(TUINT32 udwIdx = 0; udwIdx < EN_JSON_TYPE_END; ++udwIdx)
    {
        if(m_apJsonGen[udwIdx] != NULL)
        {
            delete m_apJsonGen[udwIdx];
            m_apJsonGen[udwIdx] = NULL;
        }
    }
}

TUINT32 CJsonResultGenerator::GetResultJsonLength() const
{
    return dwResultLength;
}

TCHAR* CJsonResultGenerator::GenResult(SSession* pstSession)
{
    m_szResultBuffer[0] = '\0';
    TUINT32 udwLengthAppend = 0;
    TCHAR* pCurPos = m_szResultBuffer;
    TCHAR* pEndPos = pCurPos + MAX_NETIO_PACKAGE_BUF_LEN;

    udwLengthAppend = snprintf(pCurPos, pEndPos - pCurPos, "{");
    pCurPos += udwLengthAppend;

    udwLengthAppend = GenResHeader(pstSession, pCurPos, pEndPos);
    pCurPos += udwLengthAppend;

    udwLengthAppend = snprintf(pCurPos, pEndPos - pCurPos, ",");
    pCurPos += udwLengthAppend;

    udwLengthAppend = GenResData(pstSession, pCurPos, pEndPos);
    pCurPos += udwLengthAppend;

    udwLengthAppend = snprintf(pCurPos, pEndPos - pCurPos, "}");
    pCurPos += udwLengthAppend;

    dwResultLength = pCurPos - m_szResultBuffer;
    return m_szResultBuffer;
}

TCHAR* CJsonResultGenerator::GenNobodyResult(SSession* pstSession)
{
    m_szResultBuffer[0] = '\0';
    TUINT32 udwLengthAppend = 0;
    TCHAR* pCurPos = m_szResultBuffer;
    TCHAR* pEndPos = pCurPos + MAX_NETIO_PACKAGE_BUF_LEN;

    udwLengthAppend = snprintf(pCurPos, pEndPos - pCurPos, "{");
    pCurPos += udwLengthAppend;

    udwLengthAppend = GenResHeader(pstSession, pCurPos, pEndPos);
    pCurPos += udwLengthAppend;

    udwLengthAppend = snprintf(pCurPos, pEndPos - pCurPos, "}");
    pCurPos += udwLengthAppend;

    dwResultLength = pCurPos - m_szResultBuffer;
    return m_szResultBuffer;
}

TVOID CJsonResultGenerator::SetCompress(TBOOL bFilter)
{
    m_bNeedCompress = bFilter;
}

TINT32 CJsonResultGenerator::GenResHeader(SSession* pstSession, TCHAR* pBeginPos, TCHAR* pEndPos)
{
    TUINT32 udwLengthAppend = 0;
    TCHAR* pCurPos = pBeginPos;

    udwLengthAppend = snprintf(pCurPos, pEndPos - pCurPos,
        "\"res_header\":{\"ret_code\":%d,\"refresh_code\":%u,\"cost_time_us\":%lu,\"ret_time_s\":%u,\"sid\":%d,\"uid\":%u,\"module\":\"client_data\"}",
        pstSession->m_stCommonResInfo.m_dwRetCode,
        pstSession->m_stCommonResInfo.m_bRefreshCode,
        pstSession->m_stCommonResInfo.m_uddwCostTime,
        CTimeUtils::GetUnixTime(),
        pstSession->m_stReqParam.m_udwSvrId,
        pstSession->m_stReqParam.m_udwUserId);
    pCurPos += udwLengthAppend;

    return pCurPos - pBeginPos;
}

TINT32 CJsonResultGenerator::GenResData(SSession* pstSession, TCHAR* pBeginPos, TCHAR* pEndPos)
{
    TUINT32 udwLengthAppend = 0;
    TCHAR* pCurPos = pBeginPos;
    TINT32 dwRawDataLen = GenData(pstSession);

    if(dwRawDataLen > 512 * 1024)
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("raw package is too large[size=%u][seq=%u]", dwRawDataLen, pstSession->m_udwSeqNo));
    }

    udwLengthAppend = snprintf(pCurPos, pEndPos - pCurPos, "\"res_data\":[");
    pCurPos += udwLengthAppend;

    if(!m_bNeedCompress)
    {
        TUINT64 uddwMD5 = CHuWork::GetStringMD5(m_szDataBuffer, m_szMD5Buffer);

        udwLengthAppend = snprintf(pCurPos, pEndPos - pCurPos,
            "{\"key\":\"[svr:%lu:%d:0:%d]\",\"data\":%s}",
            uddwMD5, CTimeUtils::GetUnixTime(), dwRawDataLen, m_szDataBuffer);
        pCurPos += udwLengthAppend;
    }
    else
    {
        TINT32 dwCompressDataLen = Compress(pstSession->m_stReqParam.m_udwVersion, dwRawDataLen, m_szDataBuffer);
        TUINT64 uddwMD5 = CHuWork::GetStringMD5(m_szEncodeBuffer, m_szMD5Buffer);

        if(dwCompressDataLen > 512 * 1024)
        {
            TSE_LOG_INFO(pstSession->m_poServLog, ("compress package is too large[size=%u][seq=%u]", dwCompressDataLen, pstSession->m_udwSeqNo));
        }

        udwLengthAppend = snprintf(pCurPos, pEndPos - pCurPos,
            "{\"key\":\"[svr:%lu:%d:1:%d]\",\"data\":\"%s\"}",
            uddwMD5, CTimeUtils::GetUnixTime(), dwCompressDataLen + 2, m_szEncodeBuffer);
        pCurPos += udwLengthAppend;
    }

    if(pstSession->m_bHasPromoteData)
    {
        TINT32 dwRawDataLen = GenPromoteData(pstSession);
        if(!m_bNeedCompress)
        {
            TUINT64 uddwMD5 = CHuWork::GetStringMD5(m_szDataBuffer, m_szMD5Buffer);

            udwLengthAppend = snprintf(pCurPos, pEndPos - pCurPos,
                ",{\"key\":\"[iap_promote:%lu:%d:0:%d]\",\"data\":%s}",
                uddwMD5, CTimeUtils::GetUnixTime(), dwRawDataLen, m_szDataBuffer);
            pCurPos += udwLengthAppend;
        }
        else
        {
            TINT32 dwCompressDataLen = Compress(pstSession->m_stReqParam.m_udwVersion, dwRawDataLen, m_szDataBuffer);
            TUINT64 uddwMD5 = CHuWork::GetStringMD5(m_szEncodeBuffer, m_szMD5Buffer);

            udwLengthAppend = snprintf(pCurPos, pEndPos - pCurPos,
                ",{\"key\":\"[iap_promote:%lu:%d:1:%d]\",\"data\":\"%s\"}",
                uddwMD5, CTimeUtils::GetUnixTime(), dwCompressDataLen + 2, m_szEncodeBuffer);
            pCurPos += udwLengthAppend;
        }
    }

    udwLengthAppend = snprintf(pCurPos, pEndPos - pCurPos, "]");
    pCurPos += udwLengthAppend;

    return pCurPos - pBeginPos;
}

TINT32 CJsonResultGenerator::GenData(SSession* pstSession)
{
    TbData_output *ptbDataOutput = &pstSession->m_stUserInfo.m_tbDataOutput;

    //wave@20160714
    pstSession->m_dwClientResType = EN_CONTENT_UPDATE_TYPE__ALL;

    m_jsonResult.clear();
    if (pstSession->m_stCommonResInfo.m_bIsProSysData == FALSE)
    {
        m_apJsonGen[pstSession->m_stCommonResInfo.m_ucJsonType]->GenDataJson(pstSession, m_jsonResult);
    }
    else
    {
        m_jsonResult = pstSession->m_JsonValue;
    }

    Json::FastWriter writer;
    writer.omitEndingLineFeed();
    pstSession->m_sRspJsonContent = writer.write(m_jsonResult);

    m_szDataBuffer[0] = '\0';
    TUINT32 udwLengthAppend = 0;
    TCHAR* pCurPos = m_szDataBuffer;
    TCHAR* pEndPos = pCurPos + MAX_NETIO_PACKAGE_BUF_LEN;

    udwLengthAppend = snprintf(pCurPos, pEndPos - pCurPos, "[");
    pCurPos += udwLengthAppend;

    TUINT32 udwKeyCount = 0;
    Json::Value::Members jsonDataKeys = m_jsonResult.getMemberNames();
    for(Json::Value::Members::iterator it = jsonDataKeys.begin(); it != jsonDataKeys.end(); ++it)
    {
        TUINT64 uddwRawMd5 = ptbDataOutput->m_jData[*it].asUInt64();
        TUINT64 uddwTmpMd5 = uddwRawMd5;
        udwLengthAppend = JsonToKeyDataNew(m_jsonResult[(*it)], (*it), pCurPos, pEndPos, uddwRawMd5, pstSession->m_dwClientResType);

        if(udwLengthAppend > 0)
        {
            pCurPos += udwLengthAppend;
            udwKeyCount++;

            TSE_LOG_INFO(pstSession->m_poServLog, ("GenData[%u]: %s --- %u", udwKeyCount, (*it).c_str(), udwLengthAppend));

            if(udwKeyCount != 0)
            {
                udwLengthAppend = snprintf(pCurPos, pEndPos - pCurPos, ",");
                pCurPos += udwLengthAppend;
            }
        }        

        //if(uddwTmpMd5 != uddwRawMd5)
        //{
        //    // updt md5
        //    ptbDataOutput->m_jData[*it] = uddwRawMd5;
        //    ptbDataOutput->SetFlag(TbDATA_OUTPUT_FIELD_DATA);
        //}
    }

    if(udwKeyCount)
    {
        pCurPos--;
    }

    udwLengthAppend = snprintf(pCurPos, pEndPos - pCurPos, "]");
    pCurPos += udwLengthAppend;

    (*pCurPos) = '\0';

    // update tbl_output
    //if(ptbDataOutput->IfNeedUpdate())
    //{
    //    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    //    if(ptbDataOutput->m_nUid != pstSession->m_stReqParam.m_udwUserId)
    //    {
    //        ptbDataOutput->Set_Uid(pstSession->m_stReqParam.m_udwUserId);
    //    }        
    //    ptbDataOutput->Set_Utype(pstSession->m_dwClientResType);
    //    if(pstSession->m_dwClientResType == EN_CONTENT_UPDATE_TYPE__ALL)
    //    {
    //        ptbDataOutput->Set_T_all(udwCurTime);
    //    }
    //    else
    //    {
    //        ptbDataOutput->Set_T_inc(udwCurTime);
    //    }
    //}

    return pCurPos - m_szDataBuffer;
}

TINT32 CJsonResultGenerator::GenPromoteData(SSession* pstSession)
{
    m_szDataBuffer[0] = '\0';
    TUINT32 udwLengthAppend = 0;
    TCHAR* pCurPos = m_szDataBuffer;
    TCHAR* pEndPos = pCurPos + MAX_NETIO_PACKAGE_BUF_LEN;

    udwLengthAppend = snprintf(pCurPos, pEndPos - pCurPos, "[");
    pCurPos += udwLengthAppend;

    TUINT32 udwKeyCount = 0;
    Json::Value::Members jsonDataKeys = pstSession->m_JsonValue.getMemberNames();
    for(Json::Value::Members::iterator it = jsonDataKeys.begin(); it != jsonDataKeys.end(); ++it)
    {
        if(udwKeyCount != 0)
        {
            udwLengthAppend = snprintf(pCurPos, pEndPos - pCurPos, ",");
            pCurPos += udwLengthAppend;
        }
        udwLengthAppend = JsonToKeyData(pstSession->m_JsonValue[(*it)], (*it), pCurPos, pEndPos);
        pCurPos += udwLengthAppend;
        udwKeyCount++;
    }

    udwLengthAppend = snprintf(pCurPos, pEndPos - pCurPos, "]");
    pCurPos += udwLengthAppend;

    (*pCurPos) = '\0';

    return pCurPos - m_szDataBuffer;
}

TINT32 CJsonResultGenerator::JsonToKeyData(Json::Value& jsonData, string& strKey, TCHAR* pBeginPos, TCHAR* pEndPos)
{
    TUINT32 udwLengthAppend = 0;
    m_jsonWriter.omitEndingLineFeed();
    string strData = m_jsonWriter.write(jsonData);
    TINT32 dwDataLen = strData.length();
    TUINT64 uddwMD5 = CHuWork::GetStringMD5(strData.c_str(), m_szMD5Buffer);

    udwLengthAppend = snprintf(pBeginPos, pEndPos - pBeginPos,
        "{\"key\":\"[%s:%lu:0:0:%d]\",\"data\":%s}",
        strKey.c_str(), uddwMD5, dwDataLen, strData.c_str());

    return udwLengthAppend;
}

TINT32 CJsonResultGenerator::JsonToKeyDataNew( Json::Value& jsonData, string& strKey, TCHAR* pBeginPos, TCHAR* pEndPos, TUINT64 &uddwRawMD5, TINT32 dwOutputType )
{
    TUINT32 udwLengthAppend = 0;
    m_jsonWriter.omitEndingLineFeed();
    string strData = m_jsonWriter.write(jsonData);
    TINT32 dwDataLen = strData.length();
    TUINT64 uddwMD5 = CHuWork::GetStringMD5(strData.c_str(), m_szMD5Buffer);

    if((uddwMD5 != uddwRawMD5) || dwOutputType == EN_CONTENT_UPDATE_TYPE__ALL)
    {
        udwLengthAppend = snprintf(pBeginPos, pEndPos - pBeginPos, "{\"key\":\"[%s:%lu:0:0:%d]\",\"data\":%s}", strKey.c_str(), uddwMD5, dwDataLen, strData.c_str());
        uddwRawMD5 = uddwMD5;

        TSE_LOG_INFO(CGlobalServ::m_poServLog, ("JsonToKeyDataNew[1]: %s --- %u", strKey.c_str(), udwLengthAppend));
    }
    else
    {
        TSE_LOG_INFO(CGlobalServ::m_poServLog, ("JsonToKeyDataNew[2]: %s --- %u", strKey.c_str(), udwLengthAppend));
    }

    return udwLengthAppend;
}

TINT32 CJsonResultGenerator::Compress(TUINT32 udwVersion, TUINT32 udwRawDataLen, TCHAR* szRawData)
{
    CEncryptCR4 *poEncrypt = CEncryptCR4Mgr::get_instance()->GetEncrypt(udwVersion);

    TINT32 dwBodyBufLen = MAX_NETIO_PACKAGE_BUF_LEN;
    unsigned long udwCompressBufLen = MAX_NETIO_PACKAGE_BUF_LEN;

    m_szCompressBuffer[0] = '\0';
    m_szEncodeBuffer[0] = '\0';

    compress((Bytef*)m_szCompressBuffer, &udwCompressBufLen, (Bytef*)szRawData, udwRawDataLen);//zip
    poEncrypt->rc4((unsigned char*)m_szCompressBuffer, udwCompressBufLen);//encrypt
    CUtilBase64::encode(m_szCompressBuffer, m_szCompressBuffer + udwCompressBufLen, m_szEncodeBuffer, dwBodyBufLen);//encode

    m_szEncodeBuffer[dwBodyBufLen] = '\0';

    return dwBodyBufLen;
}

TINT32 CJsonResultGenerator::CompressZip( TUINT32 udwRawDataLen, TCHAR* szRawData )
{
    unsigned long udwCompressBufLen = MAX_NETIO_PACKAGE_BUF_LEN;
    compress((Bytef*)m_szEncodeBuffer, &udwCompressBufLen, (Bytef*)szRawData, udwRawDataLen);//zip
    return udwCompressBufLen;
}


TVOID CJsonResultGenerator::GenResult_Pb( SSession* pstSession, TBOOL bNeedContent ) //TODO
{
    TbData_output *ptbDataOutput = &pstSession->m_stUserInfo.m_tbDataOutput;
    COutputConf *poOutputConf = COutputConf::GetInstace();
    TINT32 dwTableOutputType = 0;
    TINT64 ddwSeq = 0;
    TINT32 dwSeqCheckType = 0;

    const TCHAR *pszData = NULL;
    TUINT32 udwDataLen = 0;

    TBOOL bLoginTableOutFlag = FALSE;
    TBOOL bActionTableOutFlag = FALSE;

    m_objPbResponse.Clear();

    // table content
    if(bNeedContent)
    {
        m_jsonResult.clear();
        if (pstSession->m_stCommonResInfo.m_bIsProSysData == FALSE)
        {
            m_apJsonGen[pstSession->m_stCommonResInfo.m_ucJsonType]->GenDataJson(pstSession, m_jsonResult);
            if (pstSession->m_bHasPromoteData)
            {
                Json::Value::Members jIapKeys = pstSession->m_JsonValue.getMemberNames();
                for (Json::Value::Members::iterator it = jIapKeys.begin(); it != jIapKeys.end(); ++it)
                {
                    m_jsonResult[*it] = pstSession->m_JsonValue[*it];
                }
            }
        }
        else
        {
            m_jsonResult = pstSession->m_JsonValue;
        }
        Json::FastWriter writer;
        writer.omitEndingLineFeed();

        if (pstSession->m_stCommonResInfo.m_ucJsonType == EN_JSON_TYPE_USER_JSON)
        {
            pstSession->m_sRspJsonContent = writer.write(m_jsonResult);
        }

        Json::Value::Members jsonDataKeys = m_jsonResult.getMemberNames();

        
        for(Json::Value::Members::iterator it = jsonDataKeys.begin(); it != jsonDataKeys.end(); ++it)
        {
            pszData = NULL;
            pszData = m_jSeri.serializeToBuffer(m_jsonResult[*it], udwDataLen);

            //wave@20160709: 控制表级别的增量更新——按表做增量，表内全量
            TBOOL bNeedOutput = TRUE;
            TUINT64 uddwKey = 0;
            TUINT64 uddwKeyRaw = 0;
            if(IsNeedCompare(pstSession))
            {
                uddwKeyRaw = ptbDataOutput->m_jData[*it].asUInt64();
                uddwKey = CHuWork::GetRawDataMd5((const TUCHAR*)pszData, udwDataLen);
                if(uddwKey == ptbDataOutput->m_jData[*it].asUInt64())
                {
                    if(pstSession->m_dwClientResType >= EN_CONTENT_UPDATE_TYPE__TABLE_INC) 
                    {
                        bNeedOutput = FALSE;
                    }
                }
                else
                {
                    ptbDataOutput->m_jData[*it] = uddwKey;
                    ptbDataOutput->SetFlag(TbDATA_OUTPUT_FIELD_DATA);
                    bNeedOutput = TRUE;
                }
            }
            if(bNeedOutput == FALSE)
            {
                continue;
            }

            if(pstSession->m_stCommonResInfo.m_ucJsonType == EN_JSON_TYPE_USER_JSON)
            {
                if(strcmp((*it).c_str(),  "svr_login") == 0)
                {
                    bLoginTableOutFlag = TRUE;
                }
                if(strcmp((*it).c_str(), "svr_action_list") == 0)
                {
                    bActionTableOutFlag = TRUE;
                }
            }            
            
            dwTableOutputType = poOutputConf->GetTableOutputType((*it).c_str(), pstSession->m_dwClientResType);
            dwSeqCheckType = poOutputConf->GetTableSeqCheckType((*it).c_str());
            ddwSeq = CCommonLogic::GetTableSeq(*it, &pstSession->m_stUserInfo);
            TableDomData *pTblData = m_objPbResponse.add_table_dom_data();
            pTblData->set_table_name(*it);
            pTblData->set_updt_type(dwTableOutputType);
            pTblData->set_seq_type(dwSeqCheckType);
            pTblData->set_seq(ddwSeq);
            pTblData->set_dom_type(EN_DOMDATA_TYPE__BJSON);
            pTblData->set_dom_data(pszData, udwDataLen);

            //for debug
            m_jsonWriter.omitEndingLineFeed();
            string strData = m_jsonWriter.write(m_jsonResult[*it]);
            TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("GenResult_Pb[tbl=%s][type=%d]: %s [test_uid=%d]  [raw_md5=%lu][now_md5=%lu]  [seq=%u]", 
                (*it).c_str(), dwTableOutputType, strData.c_str(), pstSession->m_stReqParam.m_udwUserId, uddwKeyRaw, uddwKey, pstSession->m_udwSeqNo));
        }
    }

    //wave@20160721: 为客户端数据刷新做兼容
    if(pstSession->m_stCommonResInfo.m_ucJsonType == EN_JSON_TYPE_USER_JSON && bLoginTableOutFlag == FALSE && bNeedContent == TRUE)
    {
        GenEmptyTableFroClientUpdt_Pb("svr_login");
    }
    if(pstSession->m_stCommonResInfo.m_ucJsonType == EN_JSON_TYPE_USER_JSON && bActionTableOutFlag == FALSE && bNeedContent == TRUE)
    {
        GenEmptyTableFroClientUpdt_Pb("svr_action_list");
    }

    // header
    m_objPbResponse.set_service_type(EN_SERVICE_TYPE__CLIENT__COMMAND_RSP);
    m_objPbResponse.set_ret_code(pstSession->m_stCommonResInfo.m_dwRetCode);
    m_objPbResponse.set_fresh_code(pstSession->m_stCommonResInfo.m_bRefreshCode);
    m_objPbResponse.set_seq(pstSession->m_udwPbSeq);
    m_objPbResponse.set_svr_seq(pstSession->m_udwSeqNo);
    m_objPbResponse.set_game_time(CTimeUtils::GetUnixTime());
    m_objPbResponse.set_cost_time(pstSession->m_stCommonResInfo.m_uddwCostTime);
    m_objPbResponse.set_uid(pstSession->m_stReqParam.m_udwUserId);
    if(pstSession->m_stCommonResInfo.m_ucJsonType == EN_JSON_TYPE_USER_JSON && bNeedContent == TRUE)
    {
        m_objPbResponse.set_sid(pstSession->m_stUserInfo.m_tbLogin.m_nSid);
    }
    else
    {
        m_objPbResponse.set_sid(pstSession->m_stReqParam.m_udwSvrId);
    }    

    // serialize
    pstSession->m_dwFinalPackLength = m_objPbResponse.ByteSize();
    assert(pstSession->m_dwFinalPackLength <= MAX_NETIO_PACKAGE_BUF_LEN);
    m_objPbResponse.SerializeToArray(&pstSession->m_szClientRspBuf[0], pstSession->m_dwFinalPackLength);

    TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("GenResult_Pb: total_byte_size=%d [test_uid=%d][s_type=%d][seq=%u]", 
        pstSession->m_dwFinalPackLength, pstSession->m_stReqParam.m_udwUserId, EN_SERVICE_TYPE__CLIENT__COMMAND_RSP, pstSession->m_udwSeqNo));

    // update tbl_output
    if(ptbDataOutput->IfNeedUpdate())
    {
        TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
        if(ptbDataOutput->m_nUid != pstSession->m_stReqParam.m_udwUserId)
        {
            ptbDataOutput->Set_Uid(pstSession->m_stReqParam.m_udwUserId);
        }
        ptbDataOutput->Set_Utype(pstSession->m_dwClientResType);
        if(pstSession->m_dwClientResType == EN_CONTENT_UPDATE_TYPE__ALL)
        {
            ptbDataOutput->Set_T_all(udwCurTime);
        }
        else
        {
            ptbDataOutput->Set_T_inc(udwCurTime);
        }
    }

    return;
}

TVOID CJsonResultGenerator::GenPushData_Pb( SSession *pstSession, TUINT16 uwServiceType, Json::Value& jsonResult, TINT32 dwSid )
{
    COutputConf *poOutputConf = COutputConf::GetInstace();
    TINT32 dwTableOutputType = 0;
    TINT64 ddwSeq = 0;
    TINT32 dwSeqCheckType = 0;

    TBOOL bLoginTableOutFlag = FALSE;
    TBOOL bActionTableOutFlag = FALSE;

    m_objPbResponse.Clear();

    // table content
    Json::Value::Members jsonDataKeys = jsonResult.getMemberNames();

    const TCHAR *pszData = NULL;
    TUINT32 udwDataLen = 0;
    for(Json::Value::Members::iterator it = jsonDataKeys.begin(); it != jsonDataKeys.end(); ++it)
    {
        pszData = NULL;
        pszData = m_jSeri.serializeToBuffer(jsonResult[*it], udwDataLen);

        dwTableOutputType = poOutputConf->GetTableOutputType((*it).c_str(), EN_CONTENT_UPDATE_TYPE__ITEM_INC);
        dwSeqCheckType = poOutputConf->GetTableSeqCheckType((*it).c_str());
        ddwSeq = CCommonLogic::GetTableSeq(*it, &pstSession->m_stUserInfo);
        TableDomData *pTblData = m_objPbResponse.add_table_dom_data();
        pTblData->set_table_name(*it);
        pTblData->set_updt_type(dwTableOutputType);
        pTblData->set_seq_type(dwSeqCheckType);
        pTblData->set_seq(ddwSeq);
        pTblData->set_dom_type(EN_DOMDATA_TYPE__BJSON);
        pTblData->set_dom_data(pszData, udwDataLen);

        if(strcmp((*it).c_str(),  "svr_login") == 0)
        {
            bLoginTableOutFlag = TRUE;
        }
        if(strcmp((*it).c_str(), "svr_action_list") == 0)
        {
            bActionTableOutFlag = TRUE;
        }

        //for debug
        m_jsonWriter.omitEndingLineFeed();
        string strData = m_jsonWriter.write(jsonResult[*it]);
        TSE_LOG_INFO(CGlobalServ::m_poServLog, ("GenPushData_Pb[tbl=%s][type=%d]: %s [test_uid=%d][seq=%u]", 
            (*it).c_str(), dwTableOutputType, strData.c_str(), pstSession->m_stReqParam.m_udwUserId, pstSession->m_udwSeqNo));
    }

    //wave@20160721: 为客户端数据刷新做兼容
    if(bLoginTableOutFlag == FALSE && jsonResult.isMember("svr_map_inc") == false)
    {
        GenEmptyTableFroClientUpdt_Pb("svr_login");
    }
    if(bActionTableOutFlag == FALSE && jsonResult.isMember("svr_map_inc") == false)
    {
        GenEmptyTableFroClientUpdt_Pb("svr_action_list");
    }

    // header
    m_objPbResponse.set_service_type(uwServiceType);
    m_objPbResponse.set_ret_code(pstSession->m_stCommonResInfo.m_dwRetCode);
    m_objPbResponse.set_fresh_code(pstSession->m_stCommonResInfo.m_bRefreshCode);
    m_objPbResponse.set_seq(pstSession->m_udwSeqNo);
    m_objPbResponse.set_svr_seq(pstSession->m_udwSeqNo);
    m_objPbResponse.set_game_time(CTimeUtils::GetUnixTime());
    m_objPbResponse.set_cost_time(1);
    m_objPbResponse.set_uid(pstSession->m_stReqParam.m_udwUserId);
    if(dwSid == -1)
    {
        m_objPbResponse.set_sid(pstSession->m_stReqParam.m_udwSvrId);
    }
    else
    {
        m_objPbResponse.set_sid(dwSid);
    }
    

    // serialize
    pstSession->m_dwTmpFinalPackLength = m_objPbResponse.ByteSize();
    assert(pstSession->m_dwTmpFinalPackLength <= MAX_NETIO_PACKAGE_BUF_LEN);
    m_objPbResponse.SerializeToArray(&pstSession->m_szTmpClientRspBuf[0], pstSession->m_dwTmpFinalPackLength); 

    TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("GenPushData_Pb: total_byte_size=%d [test_uid=%d][s_type=%d][seq=%u]", 
        pstSession->m_dwTmpFinalPackLength, pstSession->m_stReqParam.m_udwUserId, uwServiceType, pstSession->m_udwSeqNo));
}

TBOOL CJsonResultGenerator::IsNeedCompare( SSession* pstSession )
{
    return pstSession->m_dwOutputCmpType == EN_OUTPUT_CMP_TYPE__CMP;
}

TVOID CJsonResultGenerator::GenEmptyTableFroClientUpdt_Pb( const TCHAR *pszTableName )
{
    const TCHAR *pszData = NULL;
    TUINT32 udwDataLen = 0;

    Json::Value jTmpJson(Json::objectValue);
    jTmpJson[pszTableName] = Json::Value(Json::objectValue);

    pszData = m_jSeri.serializeToBuffer(jTmpJson[pszTableName], udwDataLen);

    TableDomData *pTblData = m_objPbResponse.add_table_dom_data();
    pTblData->set_table_name(pszTableName);
    pTblData->set_updt_type(EN_CONTENT_UPDATE_TYPE__ITEM_INC);
    pTblData->set_seq_type(EN_TABLE_SEQCHECK_TYPE__NO);
    pTblData->set_seq(0);
    pTblData->set_dom_type(EN_DOMDATA_TYPE__BJSON);
    pTblData->set_dom_data(pszData, udwDataLen);

    //for debug
    m_jsonWriter.omitEndingLineFeed();
    string strData = m_jsonWriter.write(jTmpJson[pszTableName]);
    TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("GenPushDataResult_Pb[tbl=%s][type=%d]: %s", pszTableName, EN_CONTENT_UPDATE_TYPE__ITEM_INC, strData.c_str()));
}
