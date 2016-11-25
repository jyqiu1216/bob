#include "action_get.h"
#include "service_key.h"
#include "session_mgr.h"
#include "cgi_socket.h"
#include <sys/types.h>
#include <sys/socket.h>
#include "aws_request.h"
#include "aws_response.h"
#include "conf_base.h"
#include "svr_list_conf.h"
#include "global_serv.h"
#include "jsoncpp_utils.h"

TINT32 CActionGet::Init(CConf *poConf, CZkRegConf* poZkConf, CTaskQueue *pTaskQueue, CTseLogger *poServLog)
{
    ///////////////////////////
    m_pTaskQueue = pTaskQueue;
    m_poConf = poConf;
    m_poLog = poServLog;
    m_bStopFlag = FALSE;

    //////////////////////////
    m_udwSeqno = 0;

    m_pstAwsNode = NULL;
    m_dwSocket = 0;
    m_bHsIsOk = FALSE;

    for (TUINT32 udwIdx = 0; udwIdx < EN_ACTION_END; udwIdx++)
    {
        m_adwSocket[udwIdx] = 0;
        m_pastNodeList[udwIdx] = NULL;
    }

    m_oPack.Init();
    m_oUnPack.Init();
    m_udwBufLen = 0;

    m_udwResActionNum = 0;

    m_udwActionSeq = 1000;

    m_poZkConf = poZkConf;

    m_udwReqType = CConfBase::GetInt("req_type") == 0 ? EN_CONTENT_TYPE__STRING : EN_CONTENT_TYPE__BJSON;
    return 0;
}

void * CActionGet::Start(void *pParam)
{
    if(NULL == pParam)
    {
        return NULL;
    }

    CActionGet* poIns = (CActionGet*)pParam;
    poIns->WorkRoutine();
    return NULL;
}

TINT32 CActionGet::WorkRoutine()
{
    TUINT64 uddwInterval = 1000 * CConfBase::GetInt("action_get_interval_ms");
    TUINT64 uddwCurTime = 0;
    TUINT64 uddwLastSearchTime = 0;
    //TINT32  dwRc = 0;
    //TBOOL   bPreLockSucc = FALSE;
    //TINT32  dwPreRet = -1;

    while(m_bStopFlag == FALSE)
    {
        uddwCurTime = CTimeUtils::GetCurTimeUs();
        if(uddwCurTime - uddwLastSearchTime >= uddwInterval)
        {
            // 获取actionlist
            for(TUINT32 udwIdx = 0; udwIdx < CActionTableConf::GetInstance()->m_udwTableNum; udwIdx++)
            {
                TSE_LOG_INFO(m_poLog, ("action_table: table_name=%s, [seq=%u]", CActionTableConf::GetInstance()->m_vecTable[udwIdx].strTableName.c_str(), m_udwSeqno));
                DoWork(CActionTableConf::GetInstance()->m_vecTable[udwIdx]);
            }

            uddwLastSearchTime = uddwCurTime;

            TSE_LOG_DEBUG(m_poLog, ("WorkRoutine: cost_time=%Lu [seq=%u]", \
                CTimeUtils::GetCurTimeUs() - uddwCurTime, m_udwSeqno));

            // clear map
            CComputeMap::GetInstance()->ClearTimeoutAction(CConfBase::GetInt("action_timeout_ms"), m_poLog);
        }
        else
        {
            // 每次休息50ms
            usleep(5000);  //但5000us=5ms...
        }
    }

    return 0;
}

TINT32 CActionGet::DoWork(const SActionTable& stActionTable)
{
    TINT32 dwRetCode = 0;
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();

    m_udwSeqno++;

    // 1. 获取action list
    dwRetCode = GetActionList(stActionTable, udwCurTime);
    if(dwRetCode < 0)
    {
        return -1;
    }

    // 2. 处理请求，则其需要的压入工作队列
    ProcessActionList(stActionTable);
    return 0;
}

TINT32 CActionGet::GetActionList(const SActionTable& stActionTable, TUINT32 udwEndTime)
{
    TINT32 dwRetCode = 0;
    TUCHAR *pszPack = NULL;
    TUINT32 udwPackLen = 0;

    //连接下游
    if (m_adwSocket[stActionTable.dwTableType] <= 0)
    {
        TSE_LOG_DEBUG(m_poLog, ("ProcessActionGetRequest: reconnect action[%d] svr [seq=%u]", stActionTable.dwTableType, m_udwSeqno));
        dwRetCode = ReconnetActionSvr(stActionTable.dwTableType);
        if (dwRetCode < 0)
        {
            TSE_LOG_ERROR(m_poLog, ("ProcessActionGetRequest: fail to reconnect action[%d] svr [seq=%u]", stActionTable.dwTableType, m_udwSeqno));
            return -1;
        }
    }

    //组装请求
    string sOperatorName = "GetActionListByEtime";
    string sReqContent;

    Json::Value jTmp = Json::Value(Json::objectValue);
    jTmp["Etime"] = udwEndTime;
    jTmp["Limit"] = MAX_ACTION_NUM_IN_ONE_REQUEST;

    const TCHAR *pszReqContent;
    TUINT32 udwReqLen;
    CJsoncppSeri jSeri;

    Json::FastWriter writer;
    writer.omitEndingLineFeed();

    string sTableName;
    sTableName.clear();
    if (stActionTable.dwTableType == EN_UID_ACTION)
    {
        TbAction tbAction;
        sTableName = tbAction.GetTableName();
    }
    else if (stActionTable.dwTableType == EN_AID_ACTION)
    {
        TbAlliance_action tbAction;
        sTableName = tbAction.GetTableName();
    }
    else if(stActionTable.dwTableType == EN_MARCH_ACTION)
    {
        TbMarch_action tbAction;
        sTableName = tbAction.GetTableName();
    }

    //打包
    m_oPack.ResetContent();
    m_oPack.SetServiceType(EN_SERVICE_TYPE_ACTION_CENTER_REQ);
    TUINT32 udwSeqNo = CGlobalServ::GenerateHsReqSeq();
    m_oPack.SetSeq(udwSeqNo);
    m_oPack.SetKey(EN_GLOBAL_KEY__REQ_SEQ, m_udwSeqno);
    m_oPack.SetKey(EN_GLOBAL_KEY__REQ_TYPE, m_udwReqType);
    m_oPack.SetKey(EN_ACTION_SVR_KEY__SVR_ID, stActionTable.dwTableIdx);
    m_oPack.SetKey(EN_ACTION_SVR_KEY__ACTION_TYPE, stActionTable.dwTableType);
    m_oPack.SetKey(EN_ACTION_SVR_KEY__TABLE_NAME, (unsigned char*)sTableName.c_str(), sTableName.size());
    m_oPack.SetKey(EN_ACTION_SVR_KEY__OPERATE, (unsigned char*)sOperatorName.c_str(), sOperatorName.size());
    if (m_udwReqType == EN_CONTENT_TYPE__STRING)
    {
        sReqContent = writer.write(jTmp); 
        m_oPack.SetKey(EN_ACTION_SVR_KEY__REQ_BUF, (unsigned char*)sReqContent.c_str(), sReqContent.size());
    }
    else
    {
        pszReqContent = jSeri.serializeToBuffer(jTmp, udwReqLen);
        m_oPack.SetKey(EN_ACTION_SVR_KEY__REQ_BUF, (unsigned char*)pszReqContent, udwReqLen);
    }

    m_oPack.GetPackage(&pszPack, &udwPackLen);

    //发送请求
    dwRetCode = CCgiSocket::Send(m_adwSocket[stActionTable.dwTableType], (TCHAR*)pszPack, udwPackLen);
    if (dwRetCode != (TINT32)udwPackLen)
    {
        TSE_LOG_ERROR(m_poLog, ("ProcessActionGetRequest: send request failed[%d] [seq=%u]", dwRetCode, m_udwSeqno));
        if (m_adwSocket[stActionTable.dwTableType] > 0)
        {
            CCgiSocket::Close(m_adwSocket[stActionTable.dwTableType]);
            m_adwSocket[stActionTable.dwTableType] = 0;
        }
        return -2;
    }

    //接收响应
    dwRetCode = CCgiSocket::Recv(m_adwSocket[stActionTable.dwTableType], (TCHAR*)m_pucBuf, MAX_NETIO_PACKAGE_BUF_LEN);
    if (0 >= dwRetCode)
    {
        TSE_LOG_ERROR(m_poLog, ("ProcessActionGetRequest: recv response failed[%d] [seq=%u]", dwRetCode, m_udwSeqno));
        if (m_adwSocket[stActionTable.dwTableType] > 0)
        {
            CCgiSocket::Close(m_adwSocket[stActionTable.dwTableType]);
            m_adwSocket[stActionTable.dwTableType] = 0;
        }
        return -3;
    }
    else
    {
        m_udwBufLen = dwRetCode;
    }

    //parse response
    dwRetCode = ParseActionSvrResponse(m_pucBuf, m_udwBufLen, &m_stAwsResInfo);
    if (dwRetCode < 0)
    {
        TSE_LOG_ERROR(m_poLog, ("ProcessActionGetRequest: ParseActionSvrResponse failed[%d] [seq=%u]", dwRetCode, m_udwSeqno));
        return -4;
    }

    //get action info
    switch(stActionTable.dwTableType)
    {
    case EN_AID_ACTION:
        dwRetCode = CAwsResponse::OnQueryRsp(m_stAwsResInfo, m_astAlAction, sizeof(TbAlliance_action), MAX_ACTION_NUM_IN_ONE_REQUEST);
        break;
    case EN_UID_ACTION:
        dwRetCode = CAwsResponse::OnQueryRsp(m_stAwsResInfo, m_astAction, sizeof(TbAction), MAX_ACTION_NUM_IN_ONE_REQUEST);
        break;
    case EN_MARCH_ACTION:
        dwRetCode = CAwsResponse::OnQueryRsp(m_stAwsResInfo, m_astMarch, sizeof(TbMarch_action), MAX_ACTION_NUM_IN_ONE_REQUEST);
        break;
    default:
        break;
    }

    m_udwAlActionNum = 0;
    m_udwResActionNum = 0;
    m_udwMarchNum = 0;

    if(dwRetCode < 0)
    {
        switch(stActionTable.dwTableType)
        {
        case EN_AID_ACTION:
            m_udwAlActionNum = 0;
            break;
        case EN_UID_ACTION:
            m_udwResActionNum = 0;
            break;
        case EN_MARCH_ACTION:
            m_udwMarchNum = 0;
            break;
        default:
            break;
        }
        TSE_LOG_ERROR(m_poLog, ("ProcessActionGetRequest: get query rsp failed[%d] [seq=%u]", dwRetCode, m_udwSeqno));
        return -5;
    }
    else
    {
        switch(stActionTable.dwTableType)
        {
        case EN_AID_ACTION:
            m_udwAlActionNum = dwRetCode;
            break;
        case EN_UID_ACTION:
            m_udwResActionNum = dwRetCode;
            break;
        case EN_MARCH_ACTION:
            m_udwMarchNum = dwRetCode;
            break;
        default:
            break;
        }
    }
    
    //log
    TSE_LOG_DEBUG(m_poLog, ("GetActionList: table[%s],action_num[%u],etime[%u] [seq=%u]",
        stActionTable.strTableName.c_str(), dwRetCode, udwEndTime, m_udwSeqno));
    return 0;
}

TINT32 CActionGet::ProcessActionList(const SActionTable& stActionTable)
{
    switch(stActionTable.dwTableType)
    {
    case EN_UID_ACTION:
        return CActionGet::ProcessBuffActionList(stActionTable);
        break;
    case EN_AID_ACTION:
        return CActionGet::ProcessAlActionList(stActionTable);
        break;
    case EN_MARCH_ACTION:
        return CActionGet::ProcessMarchActionList(stActionTable);
        break;
    default:
        break;
    }

    return -2;
}

TINT32 CActionGet::ProcessBuffActionList(const SActionTable& stActionTable)
{
    SSession *pSession = NULL;
    TUINT64 uddwInsertTime = 0;
    TBOOL bFind = TRUE;
    for(TUINT32 udwIdx = 0; udwIdx < m_udwResActionNum; udwIdx++)
    {
        if(m_astAction[udwIdx].m_nId == 0)
        {
            TSE_LOG_DEBUG(m_poLog, ("ProcessActionList: error action[id=%ld][etime=%ld] [seq=%u]",
                m_astAction[udwIdx].m_nId, m_astAction[udwIdx].m_nEtime, m_udwSeqno));
            continue;
        }

        // a 查询现在是否可以计算
//        if (stActionTable.setTableSid.count(m_astAction[udwIdx].m_nSid) == 0)
//        {
//            continue;
//        }
        bFind = CComputeMap::GetInstance()->FindInMap(0, &m_astAction[udwIdx]);
        if(bFind == TRUE)
        {
            TSE_LOG_DEBUG(m_poLog, ("ProcessActionList: map task[%ld,%ld,%u,%lu] [seq=%u]", \
                m_astAction[udwIdx].m_nId, m_astAction[udwIdx].m_nEtime, uddwInsertTime, \
                0, m_udwSeqno));
            continue;
        }

        // b 插入map
        CComputeMap::GetInstance()->InsertIntoMap(0, &m_astAction[udwIdx]);

        // c. 如果是新的数据，则先获取session，并将其压入工作队列
        // c.1 获取session
        pSession = NULL;
        if(CSessionMgr::Instance()->WaitTillSession(&pSession) != 0)
        {
            TSE_LOG_ERROR(m_poLog, ("Get new session failed! [seq=%u]", m_udwSeqno));
            continue;
        }
        // c.2 设置session
        pSession->Reset();
        pSession->m_udwSeqNo = m_udwActionSeq++;
        pSession->m_udwExpectProcedure = EN_PROCEDURE__INIT;
        pSession->m_udwNextProcedure = EN_PROCEDURE__INIT;
        pSession->m_udwReqSvrId = m_astAction[udwIdx].m_nSid;
        pSession->m_ucActionRawStatus = m_astAction[udwIdx].m_nStatus;
        pSession->m_ucActionRawSecClass = m_astAction[udwIdx].m_nSclass;
        //pSession->m_udwRawTuid = m_astAction[idx].m_nTuid;
        pSession->m_udwRawTuid = 0;//TODO

        pSession->m_stReqAction = m_astAction[udwIdx];
        pSession->m_ucIsUsing = 1;
        pSession->m_udwContentType = m_udwReqType;
        pSession->m_uddwTimeBeg = CTimeUtils::GetCurTimeUs();

        pSession->m_stActionTable = stActionTable;

        // c.3 压入工作队列
        m_pTaskQueue->WaitTillPush(pSession);

        // log
        TSE_LOG_DEBUG(m_poLog, ("ProcessActionList: gen work task[%ld,%ld,%u,%lu] [seq=%u]",
            m_astAction[udwIdx].m_nId, m_astAction[udwIdx].m_nEtime, uddwInsertTime,
            pSession->m_uddwTimeBeg, m_udwSeqno));
    }

    return 0;
}

TINT32 CActionGet::ProcessAlActionList(const SActionTable& stActionTable)
{
    SSession *pSession = NULL;
    TUINT64 uddwInsertTime = 0;
    TBOOL bFind = TRUE;
    for(TUINT32 udwIdx = 0; udwIdx < m_udwAlActionNum; udwIdx++)
    {
        if(m_astAlAction[udwIdx].m_nId == 0)
        {
            TSE_LOG_DEBUG(m_poLog, ("ProcessActionList: error action[id=%ld][etime=%ld] [seq=%u]",
                m_astAlAction[udwIdx].m_nId, m_astAlAction[udwIdx].m_nEtime, m_udwSeqno));
            continue;
        }

        // a 查询现在是否可以计算
        bFind = CComputeMap::GetInstance()->FindInMap(0, &m_astAlAction[udwIdx]);
        if(bFind == TRUE)
        {
            TSE_LOG_DEBUG(m_poLog, ("ProcessActionList: map task[%ld,%ld,%u,%llu] [seq=%u]", \
                m_astAlAction[udwIdx].m_nId, m_astAlAction[udwIdx].m_nEtime, uddwInsertTime, \
                0, m_udwSeqno));
            continue;
        }

        // b 插入map
        CComputeMap::GetInstance()->InsertIntoMap(0, &m_astAlAction[udwIdx]);

        // c. 如果是新的数据，则先获取session，并将其压入工作队列
        // c.1 获取session
        pSession = NULL;
        if(CSessionMgr::Instance()->WaitTillSession(&pSession) != 0)
        {
            TSE_LOG_ERROR(m_poLog, ("Get new session failed! [seq=%u]", m_udwSeqno));
            continue;
        }
        // c.2 设置session
        pSession->Reset();
        pSession->m_udwSeqNo = m_udwActionSeq++;
        pSession->m_udwExpectProcedure = EN_PROCEDURE__INIT;
        pSession->m_udwNextProcedure = EN_PROCEDURE__INIT;
        pSession->m_udwReqSvrId = m_astAlAction[udwIdx].m_nSid;
        pSession->m_ucActionRawStatus = m_astAlAction[udwIdx].m_nStatus;
        pSession->m_ucActionRawSecClass = m_astAlAction[udwIdx].m_nSclass;
        //pSession->m_udwRawTuid = m_astAction[idx].m_nTuid;
        pSession->m_udwRawTuid = 0;//TODO

        //TODO
        if(m_astAlAction[udwIdx].m_nMclass == EN_ACTION_MAIN_CLASS__TRAIN_NEW)
        {
            pSession->m_udwTrainRawType = m_astAction[udwIdx].m_bParam[0].m_stTrain.m_ddwType;
            pSession->m_udwTrainRawNum = m_astAction[udwIdx].m_bParam[0].m_stTrain.m_ddwNum;
        }

        pSession->m_stReqAlAction = m_astAlAction[udwIdx];
        pSession->m_ucIsUsing = 1;
        pSession->m_udwContentType = m_udwReqType;
        pSession->m_uddwTimeBeg = CTimeUtils::GetCurTimeUs();

        pSession->m_stActionTable = stActionTable;
        // c.3 压入工作队列
        m_pTaskQueue->WaitTillPush(pSession);

        // log
        TSE_LOG_DEBUG(m_poLog, ("ProcessActionList: gen work task[%ld,%ld,%u,%lu] [seq=%u]",
            m_astAlAction[udwIdx].m_nId, m_astAlAction[udwIdx].m_nEtime, uddwInsertTime,
            pSession->m_uddwTimeBeg, m_udwSeqno));
    }

    return 0;
}

TINT32 CActionGet::ProcessMarchActionList(const SActionTable& stActionTable)
{
    SSession *pSession = NULL;
    TUINT64 uddwInsertTime = 0;
    TBOOL bFind = TRUE;
    for(TUINT32 udwIdx = 0; udwIdx < m_udwMarchNum; udwIdx++)
    {
        if(m_astMarch[udwIdx].m_nId == 0)
        {
            TSE_LOG_DEBUG(m_poLog, ("ProcessActionList: error action[id=%ld][etime=%ld] [seq=%u]",
                m_astMarch[udwIdx].m_nId, m_astMarch[udwIdx].m_nEtime, m_udwSeqno));
            continue;
        }

        // a 查询现在是否可以计算
        bFind = CComputeMap::GetInstance()->FindInMap(0, &m_astMarch[udwIdx]);
        if(bFind == TRUE)
        {
            TSE_LOG_DEBUG(m_poLog, ("ProcessActionList: map task[%ld,%ld,%u,%llu] [seq=%u]", \
                m_astMarch[udwIdx].m_nId, m_astMarch[udwIdx].m_nEtime, uddwInsertTime, \
                0, m_udwSeqno));
            continue;
        }

        // b 插入map
        CComputeMap::GetInstance()->InsertIntoMap(0, &m_astMarch[udwIdx]);

        // c. 如果是新的数据，则先获取session，并将其压入工作队列
        // c.1 获取session
        pSession = NULL;
        if(CSessionMgr::Instance()->WaitTillSession(&pSession) != 0)
        {
            TSE_LOG_ERROR(m_poLog, ("Get new session failed! [seq=%u]", m_udwSeqno));
            continue;
        }
        // c.2 设置session
        pSession->Reset();
        pSession->m_udwSeqNo = m_udwActionSeq++;
        pSession->m_udwExpectProcedure = EN_PROCEDURE__INIT;
        pSession->m_udwNextProcedure = EN_PROCEDURE__INIT;
        pSession->m_udwReqSvrId = m_astMarch[udwIdx].m_nSid;
        pSession->m_ucActionRawStatus = m_astMarch[udwIdx].m_nStatus;
        pSession->m_ucActionRawSecClass = m_astMarch[udwIdx].m_nSclass;
        pSession->m_udwRawTuid = m_astMarch[udwIdx].m_nTuid;

        pSession->m_stReqMarch = m_astMarch[udwIdx];
        pSession->m_ucIsUsing = 1;
        pSession->m_udwContentType = m_udwReqType;
        pSession->m_uddwTimeBeg = CTimeUtils::GetCurTimeUs();

        pSession->m_stActionTable = stActionTable;
        // c.3 压入工作队列
        m_pTaskQueue->WaitTillPush(pSession);

        // log
        TSE_LOG_DEBUG(m_poLog, ("ProcessActionList: gen work task[%ld,%ld,%u,%lu] [seq=%u]",
            m_astMarch[udwIdx].m_nId, m_astMarch[udwIdx].m_nEtime, uddwInsertTime,
            pSession->m_uddwTimeBeg, m_udwSeqno));
    }

    return 0;
}

TINT32 CActionGet::ParseAwsResponse(TUCHAR *pszPack, TUINT32 udwPackLen, AwsRspInfo *pAwsRspInfo)
{
    TUCHAR *pszValBuf = NULL;
    TUINT32 udwValBufLen = 0;

    pAwsRspInfo->Reset();

    m_oUnPack.UntachPackage();
    m_oUnPack.AttachPackage(pszPack, udwPackLen);
    if(FALSE == m_oUnPack.Unpack())
    {
        return -1;
    }
    pAwsRspInfo->udwServiceType = m_oUnPack.GetServiceType();
    //TUINT32 udwSeqNo = m_oUnPack.GetSeq();
    //TSE_LOG_ERROR(m_poLog, ("ParseAwsResponse: send_seq:%u [seq=%u]", udwSeqNo, m_udwSeqno));
    m_oUnPack.GetVal(EN_GLOBAL_KEY__RES_CODE, &pAwsRspInfo->dwRetCode);
    m_oUnPack.GetVal(EN_GLOBAL_KEY__RES_COST_TIME, &pAwsRspInfo->udwCostTime);
    m_oUnPack.GetVal(EN_GLOBAL_KEY__INDEX_NO, &pAwsRspInfo->udwIdxNo);
    if(m_oUnPack.GetVal(EN_GLOBAL_KEY__TABLE_NAME, &pszValBuf, &udwValBufLen))
    {
        pAwsRspInfo->sTableName.resize(udwValBufLen);
        memcpy((char*)pAwsRspInfo->sTableName.c_str(), pszValBuf, udwValBufLen);
    }

    //2XX的返回码,都是正常的情况
    if(pAwsRspInfo->dwRetCode / 100 != 2)
    {
        TSE_LOG_ERROR(m_poLog, ("ParseAwsResponse: pAwsRspInfo->dwRetCode=%d, [seq=%u]", pAwsRspInfo->dwRetCode, m_udwSeqno));
        return -2;
    }

    if(pAwsRspInfo->udwServiceType == EN_SERVICE_TYPE_QUERY_DYNAMODB_RSP)
    {
        m_oUnPack.GetVal(EN_GLOBAL_KEY__RES_BUF, &pszValBuf, &udwValBufLen);
        pAwsRspInfo->sRspContent.resize(udwValBufLen);
        memcpy((char*)pAwsRspInfo->sRspContent.c_str(), pszValBuf, udwValBufLen);
    }
    else
    {
        TSE_LOG_ERROR(m_poLog, ("ParseAwsResponse: pAwsRspInfo->udwServiceType[%u] != EN_SERVICE_TYPE_QUERY_DYNAMODB_RSP, [seq=%u]", pAwsRspInfo->udwServiceType, m_udwSeqno));
        return -3;
    }

    return 0;
}

TINT32 CActionGet::ReconnetActionSvr(TINT32 dwActionType)
{
    CDownMgr *pobjDownMgr = CDownMgr::Instance();

    // 0. reset
    if (m_adwSocket[dwActionType] > 0)
    {
        CCgiSocket::Close(m_adwSocket[dwActionType]);
    }
    TINT32 dwServiceType = -1;
    switch (dwActionType)
    {
    case EN_UID_ACTION:
        dwServiceType = DOWN_NODE_TYPE__BUFF_ACTION_SVR;
        break;
    case EN_AID_ACTION:
        dwServiceType = DOWN_NODE_TYPE__AL_ACTION_SVR;
        break;
    case EN_MARCH_ACTION:
        dwServiceType = DOWN_NODE_TYPE__MARCH_ACTION_SVR;
        break;
    default:
        TSE_LOG_ERROR(m_poLog, ("Error action type[%d] [seq=%u]",
            dwActionType, m_udwSeqno));
        return -1;
    }
    if (m_pastNodeList[dwActionType])
    {
        pobjDownMgr->zk_ReleaseNode(dwServiceType, m_pastNodeList[dwActionType]);
        m_pastNodeList[dwActionType] = NULL;
    }

    // 1. 获取下游
    if (0 == pobjDownMgr->zk_GetNode(dwServiceType, &m_pastNodeList[dwActionType]))
    {
        TSE_LOG_DEBUG(m_poLog, ("Get action[%d] node succ, ip[%s:%u] [seq=%u]",
            dwActionType, m_pastNodeList[dwActionType]->m_szIP, m_pastNodeList[dwActionType]->m_uwPort, m_udwSeqno));
    }
    else
    {
        TSE_LOG_ERROR(m_poLog, ("Get action[%d] node fail [seq=%u]",
            dwActionType, m_udwSeqno));
        return -2;
    }

    // 2. 连接下游
    m_adwSocket[dwActionType] = CCgiSocket::Connect(m_pastNodeList[dwActionType]->m_szIP, m_pastNodeList[dwActionType]->m_uwPort, CConfBase::GetInt("action_timeout_ms"));
    if (m_adwSocket[dwActionType] <= 0)
    {
        TSE_LOG_ERROR(m_poLog, ("fail to connect action[%d][%s:%u],ret=%d [seq=%u]", dwActionType,
            m_pastNodeList[dwActionType]->m_szIP, m_pastNodeList[dwActionType]->m_uwPort, m_adwSocket[dwActionType], m_udwSeqno));
        return -3;
    }
    else
    {
        int nNetTimeout = 3000;//3秒，
        setsockopt(m_adwSocket[dwActionType], SOL_SOCKET, SO_SNDTIMEO, (char *)&nNetTimeout, sizeof(int));
        setsockopt(m_adwSocket[dwActionType], SOL_SOCKET, SO_RCVTIMEO, (char *)&nNetTimeout, sizeof(int));
    }
    
    return 0;
}

TINT32 CActionGet::ParseActionSvrResponse(TUCHAR *pszPack, TUINT32 udwPackLen, AwsRspInfo *pAwsRspInfo)
{
    TUCHAR *pszValBuf = NULL;
    TUINT32 udwValBufLen = 0;

    pAwsRspInfo->Reset();

    m_oUnPack.UntachPackage();
    m_oUnPack.AttachPackage(pszPack, udwPackLen);
    if (FALSE == m_oUnPack.Unpack())
    {
        return -1;
    }
    pAwsRspInfo->udwServiceType = m_oUnPack.GetServiceType();


    m_oUnPack.GetVal(EN_GLOBAL_KEY__RES_CODE, &pAwsRspInfo->dwRetCode);
    m_oUnPack.GetVal(EN_GLOBAL_KEY__RES_COST_TIME, &pAwsRspInfo->udwCostTime);
    if (!m_oUnPack.GetVal(EN_GLOBAL_KEY__RES_TYPE, &pAwsRspInfo->udwResType))
    {
        pAwsRspInfo->udwResType = EN_CONTENT_TYPE__STRING;
    }
    
    if (m_oUnPack.GetVal(EN_ACTION_SVR_KEY__TABLE_NAME, &pszValBuf, &udwValBufLen))
    {
        pAwsRspInfo->sTableName.resize(udwValBufLen);
        memcpy((char*)pAwsRspInfo->sTableName.c_str(), pszValBuf, udwValBufLen);
    }
    if (m_oUnPack.GetVal(EN_ACTION_SVR_KEY__OPERATE, &pszValBuf, &udwValBufLen))
    {
        pAwsRspInfo->sOperatorName.resize(udwValBufLen);
        memcpy((char*)pAwsRspInfo->sOperatorName.c_str(), pszValBuf, udwValBufLen);
    }

    //2XX的返回码,都是正常的情况
    if (pAwsRspInfo->dwRetCode / 100 != 2)
    {
        TSE_LOG_ERROR(m_poLog, ("ParseAwsResponse: action svr res ret=%d\n", pAwsRspInfo->dwRetCode));
        return -2;
    }

    if (pAwsRspInfo->udwServiceType == EN_SERVICE_TYPE_ACTION_CENTER_RSP)
    {
        m_oUnPack.GetVal(EN_GLOBAL_KEY__RES_BUF, &pszValBuf, &udwValBufLen);
        pAwsRspInfo->sRspContent.resize(udwValBufLen);
        memcpy((char*)pAwsRspInfo->sRspContent.c_str(), pszValBuf, udwValBufLen);
    }
    else
    {
        return -3;
    }

    return 0;
}

void CActionGet::StopQuery()
{
    m_bStopFlag = TRUE;
}
