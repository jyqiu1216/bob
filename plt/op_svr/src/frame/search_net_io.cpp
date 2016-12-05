#include "search_net_io.h"
#include "conf_base.h"
#include "global_serv.h"

TINT32 CSearchNetIO::Init(CConf *pobjConf, CTseLogger *pLog, CTaskQueue *poTaskQueue)
{
    if (NULL == pobjConf || NULL == pLog || NULL == poTaskQueue)
    {
        return -1;
    }

    TBOOL bRet = -1;

    // 1. 设置配置对象,日志对象和任务队列
    m_poConf = pobjConf;
    m_poServLog = pLog;
    m_poTaskQueue = poTaskQueue;

    // 2. 创建长连接对象
    m_pLongConn = CreateLongConnObj();
    if (m_pLongConn == NULL)
    {
        TSE_LOG_ERROR(m_poServLog, ("Create longconn failed!"));
        return -2;
    }

    // 3. 侦听端口
    TSE_LOG_ERROR(m_poServLog, ("listen socket: ip=%s port=%d", CGlobalServ::m_poConf->m_szModuleIp, CConfBase::GetInt("reg_port")));
    m_hListenSock = CreateListenSocket(CGlobalServ::m_poConf->m_szModuleIp, CConfBase::GetInt("search_port"));
    if (m_hListenSock < 0)
    {
        TSE_LOG_ERROR(m_poServLog, ("Create listen socket fail ret=%d", m_hListenSock));
        return -3;
    }

    // 4. 初始化长连接
    bRet = m_pLongConn->InitLongConn(this, MAX_NETIO_LONGCONN_SESSION_NUM, m_hListenSock, MAX_NETIO_LONGCONN_TIMEOUT_MS);
    if (bRet == FALSE)
    {
        TSE_LOG_ERROR(m_poServLog, ("Init longconn failed!"));
        return -4;
    }

    // 5. 初始化打包/解包工具
    m_pPackTool = new CBaseProtocolPack();
    m_pPackTool->Init();
    m_pUnPackTool = new CBaseProtocolUnpack();
    m_pUnPackTool->Init();

    return 0;
}

void * CSearchNetIO::RoutineNetIO(void *pParam)
{
    CSearchNetIO *net_io = (CSearchNetIO *)pParam;
    while (1)
    {
        net_io->m_pLongConn->RoutineLongConn(100);
    }

    return NULL;
}

void CSearchNetIO::OnUserRequest(LongConnHandle hSession, const TUCHAR *pszData, TUINT32 udwDataLen, BOOL &bWillResponse)
{
    //---------req process begin-------------
    //TODO:
    //---------req process end---------------
    return;
}

void CSearchNetIO::OnTasksFinishedCallBack(LTasksGroup *pstTasksGrp)
{
    SSession *poSession = 0;

    if (0 == pstTasksGrp)
    {
        TSE_LOG_DEBUG(m_poServLog, ("pstTasksGrp is null"));
        return;
    }

    if (0 == pstTasksGrp->m_Tasks[0].ucNeedRecvResponse)
    {
        TSE_LOG_DEBUG(m_poServLog, ("Invalid call back"));
        return;
    }

    // 1> get session wrapper
    poSession = (SSession *)(pstTasksGrp->m_UserData1.ptr);
    pstTasksGrp->m_UserData1.ptr = 0;
    if (0 == poSession)
    {
        TSE_LOG_ERROR(m_poServLog, ("No session attached in task_grp"));
        return;
    }

    // 1.2> check seqno
    if (poSession->m_udwSeqNo != pstTasksGrp->m_UserData2.u32)
    {
        TSE_LOG_ERROR(m_poServLog, ("CSearchNetIO::OnTasksFinishedCallBack, session[%p],sseq=%u [seq=%u]", poSession, poSession->m_udwSeqNo, pstTasksGrp->m_UserData2.u32));
        //return;
    }

    if (TRUE == poSession->m_bProcessing)
    {
        TSE_LOG_ERROR(m_poServLog, ("CSearchNetIO::OnTasksFinishedCallBack is processing, session[%p] [seq=%u]", poSession, poSession->m_udwSeqNo));
        return;
    }
    else
    {
        poSession->m_bProcessing = TRUE;
    }

    //---------req process begin-------------
    //TODO:
    TSE_LOG_DEBUG(m_poServLog, ("recv res next=%u,exp_serv=%u,session[%p] [seq=%u]",
        poSession->m_udwNextProcedure, poSession->m_udwExpectProcedure, poSession, poSession->m_udwSeqNo));
    switch (poSession->m_udwExpectProcedure)
    {
    case EN_EXPECT_PROCEDURE__EVENT:
        OnEventResponse(pstTasksGrp, poSession);
        break;
    default:
        TSE_LOG_ERROR(m_poServLog, ("Invalid expect service type[%u] [seq=%u]", poSession->m_udwExpectProcedure, poSession->m_udwSeqNo));
        break;
    }
    //---------req process end---------------

    // 3> push session wrapper to work queue
    TUINT64 uddwTimeBeg = CTimeUtils::GetCurTimeUs();
    m_poTaskQueue->WaitTillPush(poSession);
    TSE_LOG_INFO(m_poServLog, ("m_poTaskQueue->WaitTillPush: session[%p], push_cost_time[%ld] [seq=%u]", poSession, CTimeUtils::GetCurTimeUs() - uddwTimeBeg, poSession->m_udwSeqNo));
    return;
}

SOCKET CSearchNetIO::CreateListenSocket(TCHAR *pszListenHost, TUINT16 uwPort)
{
    // 1> 申请SOCKET
    SOCKET lSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (lSocket < 0)
    {
        return -1;
    }

    // 2> 设置端口可重用
    int option = 1;
    if (setsockopt(lSocket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) < 0)
    {
        close(lSocket);
        return -2;
    }

    // 2> 绑定端口
    sockaddr_in sa;
    memset(&sa, 0, sizeof(sockaddr_in));
    sa.sin_port = htons(uwPort);
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = inet_addr(pszListenHost);
    int rv = bind(lSocket, (struct sockaddr *) &sa, sizeof(sa));

    if (rv < 0)
    {
        close(lSocket);
        return -3;
    }

    // 3> 监听
    rv = listen(lSocket, uwPort);

    if (rv < 0)
    {
        close(lSocket);
        return -4;
    }

    return lSocket;
}

TINT32 CSearchNetIO::CloseListenSocket()
{
    if (m_hListenSock >= 0)
    {
        close(m_hListenSock);
    }

    return 0;
}

TINT32 CSearchNetIO::GetIp2PortByHandle(LongConnHandle stHandle, TUINT16 *puwPort, TCHAR **ppszIp)
{
    TUINT32 udwHost = 0;
    TUINT16 uwPort = 0;

    m_pLongConn->GetPeerName(stHandle, &udwHost, &uwPort);
    *puwPort = ntohs(uwPort);
    *ppszIp = inet_ntoa(*(in_addr *)&udwHost);
    return 0;
}

TINT32  CSearchNetIO::OnEventResponse(LTasksGroup *pstTasksGrp, SSession *poSession)
{
    LTask               *pstTask = 0;
    TCHAR               *pszIp = 0;
    TUINT16             uwPort = 0;
    TUINT32				udwIdx = 0;
    TINT32				dwRetCode = 0;

    poSession->m_uddwDownRqstTimeEnd = CTimeUtils::GetCurTimeUs();

    poSession->ResetEventInfo();
    vector<EventRspInfo*>& vecRsp = poSession->m_vecEventRsp;
    EventRspInfo* pEventRsp = NULL;

    TSE_LOG_DEBUG(m_poServLog, ("event res: ParseEventResponse: [vaildtasksnum=%u] [seq=%u]",
        pstTasksGrp->m_uValidTasks,
        poSession->m_udwSeqNo));

    for (udwIdx = 0; udwIdx < pstTasksGrp->m_uValidTasks; udwIdx++)
    {
        pstTask = &pstTasksGrp->m_Tasks[udwIdx];

        GetIp2PortByHandle(pstTask->hSession, &uwPort, &pszIp);

        pEventRsp = new EventRspInfo;
        vecRsp.push_back(pEventRsp);

        TSE_LOG_DEBUG(m_poServLog, ("event res: taskid[%u]: [ip=%s] [port=%u] [send=%u], [recv=%u], [timeout=%u], [donw_busy=%u], [socket_closed=%u], [verify_failed=%u] [recv_data_len=%u] [cost_time=%llu]us [req_type=%u] [seq=%u]",
            udwIdx, \
            pszIp, \
            uwPort, \
            pstTask->_ucIsSendOK, \
            pstTask->_ucIsReceiveOK, \
            pstTask->_ucTimeOutEvent, \
            pstTask->_ucIsDownstreamBusy, \
            pstTask->_ucIsSockAlreadyClosed, \
            pstTask->_ucIsVerifyPackFail, \
            pstTask->_uReceivedDataLen, \
            poSession->m_uddwDownRqstTimeEnd - poSession->m_uddwDownRqstTimeBeg, \
            poSession->m_udwEventRqstType, \
            poSession->m_udwSeqNo));

        if (0 == pstTask->_ucIsSendOK || 0 == pstTask->_ucIsReceiveOK || 1 == pstTask->_ucTimeOutEvent)
        {
            // 加入超时统计
            if (poSession->m_bEventProxyExist && pstTask->hSession == poSession->m_pstEventProxyNode->m_stDownHandle)
            {
                CDownMgr::Instance()->zk_AddTimeOut(poSession->m_pstEventProxyNode);
            }
            else if (poSession->m_bThemeEventProxyExist && pstTask->hSession == poSession->m_pstThemeEventProxyNode->m_stDownHandle)
            {
                CDownMgr::Instance()->zk_AddTimeOut(poSession->m_pstThemeEventProxyNode);
            }
            TSE_LOG_ERROR(m_poServLog, ("event res: [ip=%s] [port=%u] [send=%u], [recv=%u], [timeout=%u], [cost_time=%llu]us [seq=%u]",
                pszIp, \
                uwPort, \
                pstTask->_ucIsSendOK, \
                pstTask->_ucIsReceiveOK, \
                pstTask->_ucTimeOutEvent, \
                poSession->m_uddwDownRqstTimeEnd - poSession->m_uddwDownRqstTimeBeg, \
                poSession->m_udwSeqNo));
            if (EN_RET_CODE__SUCCESS == poSession->m_stCommonResInfo.m_dwRetCode)
            {
                poSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__TIMEOUT;
                dwRetCode = -1;
            }
            // break;
        }
        else
        {
            if (pstTask->_uReceivedDataLen > MAX_AWS_RES_DATA_LEN)
            {
                TSE_LOG_ERROR(m_poServLog, ("event res: [ip=%s] [port=%u] recv_data_len[%u]>MAX_HS_RES_DATA_LEN [cost_time=%llu]us [seq=%u]",
                    pszIp, \
                    uwPort, \
                    pstTask->_uReceivedDataLen, \
                    poSession->m_uddwDownRqstTimeEnd - poSession->m_uddwDownRqstTimeBeg, \
                    poSession->m_udwSeqNo));
                if (EN_RET_CODE__SUCCESS == poSession->m_stCommonResInfo.m_dwRetCode)
                {
                    poSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PACKAGE_LEN_OVERFLOW;
                    dwRetCode = -2;
                }
                // break;
            }
            else
            {
                dwRetCode = ParseEventResponse(pstTask->_pReceivedData, pstTask->_uReceivedDataLen, pEventRsp);
                if (dwRetCode < 0)
                {
                    // 加入错误统计
                    if (poSession->m_bEventProxyExist && pstTask->hSession == poSession->m_pstEventProxyNode->m_stDownHandle)
                    {
                        CDownMgr::Instance()->zk_AddError(poSession->m_pstEventProxyNode);
                    }
                    else if (poSession->m_bThemeEventProxyExist && pstTask->hSession == poSession->m_pstThemeEventProxyNode->m_stDownHandle)
                    {
                        CDownMgr::Instance()->zk_AddError(poSession->m_pstThemeEventProxyNode);
                    }
                    TSE_LOG_ERROR(m_poServLog, ("event res: [ip=%s] [port=%u] failed[%d] [seq=%u]",
                        pszIp, \
                        uwPort, \
                        dwRetCode, \
                        poSession->m_udwSeqNo));
                    if (EN_RET_CODE__SUCCESS == poSession->m_stCommonResInfo.m_dwRetCode)
                    {
                        poSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_EVENT_PACKAGE_ERR;
                        dwRetCode = -3;
                    }
                    // break;
                }
                else
                {
                    TSE_LOG_DEBUG(m_poServLog, ("event res: buf_len[%u] [seq=%u]",
                        pEventRsp->sRspContent.size(), \
                        poSession->m_udwSeqNo));
                }
            }
        }
    }

    poSession->m_udwEventRqstType = 0;
    return 0;
}

TINT32 CSearchNetIO::ParseEventResponse(TUCHAR *pszPack, TUINT32 udwPackLen, EventRspInfo* pEventRspInfo)
{
    TUCHAR *pszValBuf = NULL;
    TUINT32 udwValBufLen = 0;

    m_pUnPackTool->UntachPackage();
    m_pUnPackTool->AttachPackage(pszPack, udwPackLen);
    if (FALSE == m_pUnPackTool->Unpack())
    {
        return -1;
    }
    pEventRspInfo->udwServiceType = m_pUnPackTool->GetServiceType();
    m_pUnPackTool->GetVal(EN_GLOBAL_KEY__RES_CODE, &pEventRspInfo->dwRetCode);
    m_pUnPackTool->GetVal(EN_GLOBAL_KEY__RES_COST_TIME, &pEventRspInfo->udwCostTime);
    m_pUnPackTool->GetVal(EN_GLOBAL_KEY__INDEX_NO, &pEventRspInfo->udwIdxNo);
    m_pUnPackTool->GetVal(EN_KEY_EVENT_PROXY__REQ_TYPE, &pEventRspInfo->udwReqType);

    //2XX的返回码,都是正常的情况
    if (pEventRspInfo->dwRetCode != 0)
    {
        TSE_LOG_ERROR(m_poServLog, ("event res: pAwsRspInfo->dwRetCode=%d\n", pEventRspInfo->dwRetCode));
        return -2;
    }

    if (pEventRspInfo->udwServiceType == EN_SERVICE_TYPE_QUERY_EVENT_RSP)
    {
        m_pUnPackTool->GetVal(EN_GLOBAL_KEY__RES_BUF, &pszValBuf, &udwValBufLen);
        pEventRspInfo->sRspContent.resize(udwValBufLen);
        memcpy((char*)pEventRspInfo->sRspContent.c_str(), pszValBuf, udwValBufLen);
    }
    else
    {
        return -3;
    }

    return 0;
}