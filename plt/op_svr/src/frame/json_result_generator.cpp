#include "json_result_generator.h"
#include "rc4.h"
#include "zlib.h"
#include "hu_work.h"
#include "event_json.h"
#include "jsoncpp_utils.h"
#include "global_serv.h"
#include "output_conf.h"
//#include "common_logic.h"

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
    m_apJsonGen[EN_JSON_TYPE_EVENT] = new CEventJson();
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

TVOID CJsonResultGenerator::GenResult_Pb( SSession* pstSession, TBOOL bNeedContent ) //TODO
{
    m_objPbResponse.Clear();
    m_jsonResult.clear();
    m_apJsonGen[pstSession->m_stCommonResInfo.m_ucJsonType]->GenDataJson(pstSession, m_jsonResult);
    Json::Value::Members jsonDataKeys = m_jsonResult.getMemberNames();
    for (Json::Value::Members::iterator it = jsonDataKeys.begin(); it != jsonDataKeys.end(); ++it)
    {
        //for debug
        m_jsonWriter.omitEndingLineFeed();
        string strData = m_jsonWriter.write(m_jsonResult[*it]);
        TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("GenResult_Pb[tbl=%s]: %s [test_uid=%ld] [seq=%u]",
            (*it).c_str(), strData.c_str(), pstSession->m_stReqParam.m_ddwUserId, pstSession->m_udwSeqNo));
    }

    // header
    m_objPbResponse.set_service_type(EN_SERVICE_TYPE__CLIENT__COMMAND_RSP);
    m_objPbResponse.set_ret_code(pstSession->m_stCommonResInfo.m_dwRetCode);
    m_objPbResponse.set_fresh_code(pstSession->m_stCommonResInfo.m_bRefreshCode);
    m_objPbResponse.set_seq(pstSession->m_udwPbSeq);
    m_objPbResponse.set_svr_seq(pstSession->m_udwSeqNo);
    m_objPbResponse.set_game_time(CTimeUtils::GetUnixTime());
    m_objPbResponse.set_cost_time(pstSession->m_stCommonResInfo.m_uddwCostTime);
    m_objPbResponse.set_uid(pstSession->m_stReqParam.m_ddwUserId);
    m_objPbResponse.set_sid(pstSession->m_stReqParam.m_dwSvrId);

    // serialize
    pstSession->m_dwFinalPackLength = m_objPbResponse.ByteSize();
    assert(pstSession->m_dwFinalPackLength <= MAX_NETIO_PACKAGE_BUF_LEN);
    m_objPbResponse.SerializeToArray(&pstSession->m_szClientRspBuf[0], pstSession->m_dwFinalPackLength);

    TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("GenResult_Pb: total_byte_size=%d [test_uid=%ld][s_type=%d][seq=%u]", 
        pstSession->m_dwFinalPackLength, pstSession->m_stReqParam.m_ddwUserId, EN_SERVICE_TYPE__CLIENT__COMMAND_RSP, pstSession->m_udwSeqNo));
    return;
}

//TBOOL CJsonResultGenerator::IsNeedCompare( SSession* pstSession )
//{
//    return pstSession->m_dwOutputCmpType == EN_OUTPUT_CMP_TYPE__CMP;
//}

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
