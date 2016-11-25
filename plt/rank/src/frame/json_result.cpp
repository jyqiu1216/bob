#include "json_result.h"

#include "rc4.h"
#include "zlib.h"
#include "hu_work.h"
#include "output_conf.h"

CJsonResult::CJsonResult()
{
    m_jsonResult.clear();

    m_szDataBuffer = new TCHAR[MAX_NETIO_PACKAGE_BUF_LEN];
    m_szCompressBuffer = new TCHAR[MAX_NETIO_PACKAGE_BUF_LEN];
    m_szEncodeBuffer = new TCHAR[MAX_NETIO_PACKAGE_BUF_LEN];
    m_szResultBuffer = new TCHAR[MAX_NETIO_PACKAGE_BUF_LEN];
    m_szMD5Buffer = new TCHAR[MAX_NETIO_PACKAGE_BUF_LEN];

    m_bNeedCompress = FALSE;

}

CJsonResult::~CJsonResult()
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
}

TUINT32 CJsonResult::GetResultJsonLength() const
{
    return dwResultLength;
}

TCHAR* CJsonResult::GenResult(SSession* pstSession)
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

TVOID CJsonResult::SetCompress(TBOOL bFilter)
{
    m_bNeedCompress = bFilter;
}

TINT32 CJsonResult::GenResHeader(SSession* pstSession, TCHAR* pBeginPos, TCHAR* pEndPos)
{
    TUINT32 udwLengthAppend = 0;
    TCHAR* pCurPos = pBeginPos;

    udwLengthAppend = snprintf(pCurPos, pEndPos - pCurPos,
        "\"res_header\":{\"ret_code\":%d,\"cost_time_us\":%lu,\"ret_time_s\":%u,\"sid\":%d,\"uid\":%u,\"module\":\"client_data\"}",
        pstSession->m_stCommonResInfo.m_dwRetCode,
        pstSession->m_stCommonResInfo.m_uddwCostTime,
        CTimeUtils::GetUnixTime(),
        pstSession->m_stReqParam.m_udwSvrId,
        pstSession->m_stReqParam.m_udwUserId);
    pCurPos += udwLengthAppend;

    return pCurPos - pBeginPos;
}

TINT32 CJsonResult::GenResData(SSession* pstSession, TCHAR* pBeginPos, TCHAR* pEndPos)
{
    TUINT32 udwLengthAppend = 0;
    TCHAR* pCurPos = pBeginPos;
    TINT32 dwRawDataLen = GenData(pstSession);

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

        udwLengthAppend = snprintf(pCurPos, pEndPos - pCurPos,
            "{\"key\":\"[svr:%lu:%d:1:%d]\",\"data\":\"%s\"}",
            uddwMD5, CTimeUtils::GetUnixTime(), dwCompressDataLen + 2, m_szEncodeBuffer);
        pCurPos += udwLengthAppend;
    }

    udwLengthAppend = snprintf(pCurPos, pEndPos - pCurPos, "]");
    pCurPos += udwLengthAppend;

    return pCurPos - pBeginPos;
}

TINT32 CJsonResult::GenData(SSession* pstSession)
{
    m_jsonResult.clear();
    m_jsonResult = pstSession->m_stCommonResInfo.m_jResultWriter;

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
        if(udwKeyCount != 0)
        {
            udwLengthAppend = snprintf(pCurPos, pEndPos - pCurPos, ",");
            pCurPos += udwLengthAppend;
        }
        udwLengthAppend = JsonToKeyData(m_jsonResult[(*it)], (*it), pCurPos, pEndPos);
        pCurPos += udwLengthAppend;
        udwKeyCount++;
    }

    udwLengthAppend = snprintf(pCurPos, pEndPos - pCurPos, "]");
    pCurPos += udwLengthAppend;

    (*pCurPos) = '\0';

    return pCurPos - m_szDataBuffer;
}

TINT32 CJsonResult::JsonToKeyData(Json::Value& jsonData, string& strKey, TCHAR* pBeginPos, TCHAR* pEndPos)
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

TINT32 CJsonResult::Compress(TUINT32 udwVersion, TUINT32 udwRawDataLen, TCHAR* szRawData)
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

TVOID CJsonResult::GenResult_Pb( SSession* pstSession, TBOOL bNeedContent /*= TRUE*/ )
{
    m_objPbResponse.Clear();

    // table content
    if(bNeedContent)
    {
        m_jsonResult.clear();
        m_jsonResult = pstSession->m_stCommonResInfo.m_jResultWriter;
        Json::Value::Members jsonDataKeys = m_jsonResult.getMemberNames();

        const TCHAR *pszData = NULL;
        TUINT32 udwDataLen = 0;
        for(Json::Value::Members::iterator it = jsonDataKeys.begin(); it != jsonDataKeys.end(); ++it)
        {
            pszData = NULL;
            pszData = m_jSeri.serializeToBuffer(m_jsonResult[*it], udwDataLen);

            TableDomData *pTblData = m_objPbResponse.add_table_dom_data();
            pTblData->set_table_name(*it);
            pTblData->set_updt_type(EN_CONTENT_UPDATE_TYPE__ALL);
            pTblData->set_seq_type(EN_TABLE_SEQCHECK_TYPE__NO);
            pTblData->set_seq(0);
            pTblData->set_dom_type(EN_DOMDATA_TYPE__BJSON);
            pTblData->set_dom_data(pszData, udwDataLen);
        }
    }

    // header
    m_objPbResponse.set_service_type(EN_SERVICE_TYPE__CLIENT__RANK_RSP);
    m_objPbResponse.set_ret_code(pstSession->m_stCommonResInfo.m_dwRetCode);
    m_objPbResponse.set_fresh_code(1);
    m_objPbResponse.set_seq(pstSession->m_udwPbSeqNo);
    m_objPbResponse.set_svr_seq(pstSession->m_udwSeqNo);
    m_objPbResponse.set_game_time(CTimeUtils::GetUnixTime());
    m_objPbResponse.set_cost_time(pstSession->m_stCommonResInfo.m_uddwCostTime);
    m_objPbResponse.set_uid(pstSession->m_stReqParam.m_udwUserId);
    m_objPbResponse.set_sid(pstSession->m_stReqParam.m_udwSvrId);

    // serialize
    pstSession->m_udwTmpBufLen = m_objPbResponse.ByteSize();
    assert(pstSession->m_udwTmpBufLen <= MAX_NETIO_PACKAGE_BUF_LEN);
    m_objPbResponse.SerializeToArray(&pstSession->m_pTmpBuf[0], pstSession->m_udwTmpBufLen); 

    return;
}
