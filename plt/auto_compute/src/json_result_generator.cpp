#include "json_result_generator.h"
#include "zlib.h"
#include "global_serv.h"
#include "output_conf.h"
#include "common_logic.h"

CJsonResultGenerator::CJsonResultGenerator()
{
    m_jsonResult.clear();

    m_szDataBuffer = new TCHAR[MAX_NETIO_PACKAGE_BUF_LEN];
    m_szCompressBuffer = new TCHAR[MAX_NETIO_PACKAGE_BUF_LEN];
    m_szEncodeBuffer = new TCHAR[MAX_NETIO_PACKAGE_BUF_LEN];
    m_szResultBuffer = new TCHAR[MAX_NETIO_PACKAGE_BUF_LEN];
    m_szMD5Buffer = new TCHAR[MAX_NETIO_PACKAGE_BUF_LEN];

    m_bNeedCompress = FALSE;

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

}

TVOID CJsonResultGenerator::GenPushData_Pb( SSession *pstSession, TUINT16 uwServiceType, Json::Value& jsonResult )
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
        //ddwSeq = CCommonLogic::GetTableSeq(*it, &pstSession->m_stUserInfo);
        ddwSeq = 0;
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
         TSE_LOG_INFO(CGlobalServ::m_poServLog, ("GenPushData_Pb[tbl=%s][type=%d]: %s [seq=%u]", 
             (*it).c_str(), dwTableOutputType, strData.c_str(), pstSession->m_udwSeqNo));
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
    m_objPbResponse.set_ret_code(0);
    m_objPbResponse.set_fresh_code(1);
    m_objPbResponse.set_seq(pstSession->m_udwSeqNo);
    m_objPbResponse.set_svr_seq(pstSession->m_udwSeqNo);
    m_objPbResponse.set_game_time(CTimeUtils::GetUnixTime());
    m_objPbResponse.set_cost_time(1);
    m_objPbResponse.set_uid(pstSession->m_stSourceUser.m_tbPlayer.m_nUid);
    m_objPbResponse.set_sid(pstSession->m_stSourceUser.m_tbPlayer.m_nSid);

    // serialize
    pstSession->m_udwTmpBufLen = m_objPbResponse.ByteSize();
    assert(pstSession->m_udwTmpBufLen <= MAX_NETIO_PACKAGE_BUF_LEN);
    m_objPbResponse.SerializeToArray(&pstSession->m_pTmpBuf[0], pstSession->m_udwTmpBufLen);

    TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("GenPushData_Pb: total_byte_size=%d [test_uid=%d][s_type=%d][seq=%u]", 
        pstSession->m_udwTmpBufLen, pstSession->m_stSourceUser.m_tbPlayer.m_nUid, uwServiceType, pstSession->m_udwSeqNo));
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
    TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("GenResult_Pb[tbl=%s][type=%d]: %s", pszTableName, EN_CONTENT_UPDATE_TYPE__ITEM_INC, strData.c_str()));
}
