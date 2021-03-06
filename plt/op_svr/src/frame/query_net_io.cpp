#include "query_net_io.h"
#include "session_mgr.h"
#include "conf_base.h"
#include "global_serv.h"

TINT32 CQueryNetIO::Init(CTseLogger *pLog, CTaskQueue *poTaskQueue)
{
    if (NULL == pLog || NULL == poTaskQueue)
    {
        return -1;
    }

    TBOOL bRet = -1;

    // 1. 设置配置对象,日志对象和任务队列
    m_pLog = pLog;
    m_poTaskQueue = poTaskQueue;
    m_bHttpOp = CConfBase::GetInt("tcp_mode") == 0 ? TRUE : FALSE;
    m_udwContentType = CConfBase::GetInt("req_type") == 0 ? EN_CONTENT_TYPE__STRING : EN_CONTENT_TYPE__BJSON;
    // 2. 创建长连接对象
    m_pLongConn = CreateLongConnObj();
    if (m_pLongConn == NULL)
    {
        TSE_LOG_ERROR(m_pLog, ("Create longconn failed!"));
        return -2;
    }

    // 3. 侦听端口
    m_hListenSock = CreateListenSocket(CGlobalServ::m_poConf->m_szModuleIp, CConfBase::GetInt("serv_port"));
    if (m_hListenSock < 0)
    {
        TSE_LOG_ERROR(m_pLog, ("Create listen socket fail"));
        return -3;
    }

    // 4. 初始化长连接
    bRet = m_pLongConn->InitLongConn(this, MAX_NETIO_LONGCONN_SESSION_NUM, m_hListenSock, MAX_NETIO_LONGCONN_TIMEOUT_MS, 0, 0, 0);
    if (bRet == FALSE)
    {
        TSE_LOG_ERROR(m_pLog, ("Init longconn failed!"));
        return -4;
    }

    // 5. 初始化打包/解包工具
    m_pPackTool = new CBaseProtocolPack();
    m_pPackTool->Init();
    m_pUnPackTool = new CBaseProtocolUnpack();
    m_pUnPackTool->Init();

    m_udwSeqno = 1000;

    if (m_bHttpOp == FALSE)
    {
        m_pobjClientReq = new ClientRequest;
    }
    return 0;
}

void * CQueryNetIO::RoutineNetIO(void *pParam)
{
    CQueryNetIO *net_io = (CQueryNetIO *)pParam;
    while (1)
    {
        net_io->m_pLongConn->RoutineLongConn(100);
    }

    return NULL;
}

void CQueryNetIO::OnUserRequest(LongConnHandle hSession, const TUCHAR *pData, TUINT32 uLen, BOOL &bWillResponse)
{
    //TCHAR *pszIp = 0;
    //TUINT16 uwPort = 0;
    //SSession    *pSession = NULL;

    ////防止seq溢出
    //if (m_udwSeqno < 1000)
    //{
    //    m_udwSeqno = 1000;
    //}
    //m_udwSeqno++;

    //// Get src Ip and Port
    //GetIp2PortByHandle(hSession, &uwPort, &pszIp);

    //TSE_LOG_DEBUG(m_pLog, ("New request from [%s:%u] [len=%u] [seq=%u]", pszIp, uwPort, uLen, m_udwSeqno));

    //---------req process begin-------------
    //TODO:
    HRESULT hRes = S_OK;

    if (m_bHttpOp)
    {
        hRes = OnClientCommandRequest_Http(hSession, pData, uLen);
    }
    else
    {
        hRes = OnClientCommandRequest_Binary(hSession, pData, uLen);
    }
    if (hRes != S_OK)
    {
        bWillResponse = FALSE;
    }
    else
    {
        bWillResponse = TRUE;
    }
    //---------req process end---------------

    return;
}

void CQueryNetIO::OnTasksFinishedCallBack(LTasksGroup *pTasksgrp)
{
    //---------req process begin-------------
    //TODO:
    //---------req process end---------------
    return;
}

SOCKET CQueryNetIO::CreateListenSocket(TCHAR *pszListenHost, TUINT16 uwPort)
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
        return -1;
    }

    // 3> 绑定端口
    sockaddr_in sa;
    memset(&sa, 0, sizeof(sockaddr_in));
    sa.sin_port = htons(uwPort);
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = inet_addr(pszListenHost);
    int rv = bind(lSocket, (struct sockaddr *) &sa, sizeof(sa));

    if (rv < 0)
    {
        close(lSocket);
        return -1;
    }

    // 4> 监听
    rv = listen(lSocket, uwPort);

    if (rv < 0)
    {
        close(lSocket);
        return -1;
    }

    return lSocket;
}

TINT32 CQueryNetIO::CloseListenSocket()
{
    if (m_hListenSock >= 0)
    {
        close(m_hListenSock);
    }

    return 0;
}

TINT32 CQueryNetIO::GetIp2PortByHandle(LongConnHandle stHandle, TUINT16 *puwPort, TCHAR **ppszIp)
{
    TUINT32 udwHost = 0;
    TUINT16 uwPort = 0;

    m_pLongConn->GetPeerName(stHandle, &udwHost, &uwPort);
    *puwPort = ntohs(uwPort);
    *ppszIp = inet_ntoa(*(in_addr *)&udwHost);
    return 0;
}

TINT32 CQueryNetIO::SendHttpBackErr(LongConnHandle stHandle, TUINT32 udwSeqno)
{
    // TODO;
    return 0;
}

HRESULT CQueryNetIO::OnClientCommandRequest_Http(LongConnHandle stHandle, const TUCHAR *pszData, TUINT32 udwDataLen)
{
    TCHAR *pszIp = 0;
    TUINT16 uwPort = 0;
    TUINT16 uwServType = 0;
    TUINT32 udwRegSeq = 0;
    SSession    *pSession = NULL;

    if (m_udwSeqno < 1000)
    {
        m_udwSeqno = 1000;
    }
    m_udwSeqno++;

    // 1. Get src Ip and Port
    GetIp2PortByHandle(stHandle, &uwPort, &pszIp);
    TSE_LOG_DEBUG(m_pLog, ("QueryNetIO: New request from [%s:%u] [len=%u] [seq=%u]", pszIp, uwPort, udwDataLen, m_udwSeqno));


    // 1. 判定包长
    if (udwDataLen > MAX_HTTP_REQ_LEN)
    {
        TSE_LOG_ERROR(m_pLog, ("Http req len [%u] larger than [%u] [seq=%u]",
            udwDataLen, MAX_HTTP_REQ_LEN, m_udwSeqno));
        return S_FAIL;
    }

    // 2. 获取session对象
    if (CGlobalServ::m_poSessionMgr->WaitTillSession(&pSession) != 0)
    {
        TSE_LOG_ERROR(m_pLog, ("Get new session failed! [seq=%u]", m_udwSeqno));
        return S_FAIL;
    }
    else
    {
        TSE_LOG_INFO(m_pLog, ("[FOR_MEMORY] Get new session[%p], empty_size=%u [seq=%u]", pSession, CGlobalServ::m_poSessionMgr->GetEmptySessionSize(), m_udwSeqno));
    }

    // 3. 设置session信息
    //pSession->Reset();//----初始化和release时已重置
    pSession->m_ucIsUsing = 1;
    pSession->m_udwContentType = m_udwContentType;
    pSession->m_udwRequestType = EN_PROCEDURE__CLIENT_REQUEST;
    pSession->m_uddwTimeBeg = CTimeUtils::GetCurTimeUs();
    pSession->m_udwSeqNo = m_udwSeqno;
    //pSession->m_stUserInfo.m_udwBSeqNo = pSession->m_udwSeqNo;
    pSession->m_stClientHandle = stHandle;
    memcpy(pSession->m_szClientReqBuf, pszData, udwDataLen);
    pSession->m_szClientReqBuf[udwDataLen] = 0;
    pSession->m_udwClientReqBufLen = udwDataLen;
    pSession->m_dwClientReqEnType = CConfBase::GetInt("NeedEncodeAndCompress");
    pSession->m_dwClientReqMode = EN_CLIENT_REQ_MODE__HTTP;

    // 4. 插入任务队列
    TSE_LOG_INFO(m_pLog, ("m_poTaskQueue->WaitTillPush: session[%p] [seq=%u]", pSession, m_udwSeqno));
    m_poTaskQueue->WaitTillPush(pSession);

    return S_OK;
}

HRESULT CQueryNetIO::OnClientCommandRequest_Binary(LongConnHandle stHandle, const TUCHAR *pszData, TUINT32 udwDataLen)
{
    SSession        *pSession = NULL;
    TUCHAR          *pszValBuf = 0;
    TUINT32         udwValBufLen = 0;
    TCHAR           *pszIp = 0;
    TUINT16         uwPort = 0;
    TUINT16         uwServType = 0;
    TUINT32         udwLinkerCmdRef = 100;
    TCHAR           *pszClientIp = 0;
    TUINT32         udwClientIpLen = 0;

    // 1. 解包
    m_pUnPackTool->UntachPackage();
    m_pUnPackTool->AttachPackage((TUCHAR *)pszData, udwDataLen);
    uwServType = m_pUnPackTool->GetServiceType();
    TUINT32 udwPackSeq = m_pUnPackTool->GetSeq();
    TUINT32 udwClientSeq = 0;
    if (m_udwSeqno < 1000)
    {
        m_udwSeqno = 1000;
    }
    m_udwSeqno++;

    GetIp2PortByHandle(stHandle, &uwPort, &pszIp);
    TSE_LOG_DEBUG(m_pLog, ("QueryNetIO: New request from [%s:%u] [len=%u] [seq=%u]", pszIp, uwPort, udwDataLen, m_udwSeqno));

    if (FALSE == m_pUnPackTool->Unpack())
    {
        TSE_LOG_ERROR(m_pLog, ("Client request unpack fail [seq=%u]", m_udwSeqno));
        return S_FAIL;
    }

    if (FALSE == m_pUnPackTool->GetVal(EN_GLOBAL_KEY__REQ_BUF, &pszValBuf, &udwValBufLen))
    {
        TSE_LOG_ERROR(m_pLog, ("Client request get EN_GLOBAL_KEY__REQ_BUF fail [seq=%u]", m_udwSeqno));
        return S_FAIL;
    }

    if (FALSE == m_pUnPackTool->GetVal(EN_GLOBAL_KEY__CLIENT_IP, &pszClientIp, &udwClientIpLen))
    {
        TSE_LOG_ERROR(m_pLog, ("Client request get EN_GLOBAL_KEY__CLIENT_IP fail [seq=%u]", m_udwSeqno));
    }

    if (FALSE == m_pUnPackTool->GetVal(EN_GLOBAL_KEY__LINKER_CMD_REF, &udwLinkerCmdRef))
    {
        TSE_LOG_ERROR(m_pLog, ("Client request get EN_GLOBAL_KEY__LINKER_CMD_REF fail [seq=%u]", m_udwSeqno));
    }

    m_pobjClientReq->Clear();
    if (false == m_pobjClientReq->ParseFromArray(pszValBuf, udwValBufLen))
    {
        TSE_LOG_ERROR(m_pLog, ("Client request parse pb fail [seq=%u]", m_udwSeqno));
        return S_FAIL;
    }

    if (FALSE == m_pUnPackTool->GetVal(EN_GLOBAL_KEY__REQ_SEQ, &udwClientSeq))
    {
        TSE_LOG_ERROR(m_pLog, ("Client request get EN_GLOBAL_KEY__REQ_SEQ fail [seq=%u]", m_udwSeqno));
    }

    // 2. 获取session对象
    if (CGlobalServ::m_poSessionMgr->WaitTillSession(&pSession) != 0)
    {
        TSE_LOG_ERROR(m_pLog, ("Get new session failed! [seq=%u]", m_udwSeqno));
        return S_FAIL;
    }
    else
    {
        TSE_LOG_DEBUG(m_pLog, ("[FOR_MEMORY] Get new session[%p], empty_size=%u, linker_cmd_ref=%u [seq=%u]", pSession, CGlobalServ::m_poSessionMgr->GetEmptySessionSize(), udwLinkerCmdRef, m_udwSeqno));
    }

    // 3. 设置session信息
    //pSession->Reset();//----初始化和release时已重置
    pSession->m_ucIsUsing = 1;
    pSession->m_udwContentType = m_udwContentType;
    pSession->m_udwRequestType = EN_PROCEDURE__CLIENT_REQUEST;
    pSession->m_uddwTimeBeg = CTimeUtils::GetCurTimeUs();
    pSession->m_udwSeqNo = m_udwSeqno;
    pSession->m_udwPackSeq = udwPackSeq;
    pSession->m_udwClientSeqNo = udwClientSeq;
    if (pszClientIp)
    {
        strncpy(pSession->m_szClientIp, pszClientIp, 31);
        pSession->m_szClientIp[31] = 0;
    }
    pSession->m_udwPbSeq = m_pobjClientReq->seq();
    pSession->m_udwLinkerCmdRef = udwLinkerCmdRef;
    //pSession->m_stUserInfo.m_udwBSeqNo = pSession->m_udwSeqNo;
    pSession->m_stClientHandle = stHandle;
    TUINT32 udwReqLen = m_pobjClientReq->req_url().size();
    memcpy(pSession->m_szClientReqBuf, m_pobjClientReq->req_url().c_str(), udwReqLen);
    pSession->m_szClientReqBuf[udwReqLen] = 0;
    pSession->m_udwClientReqBufLen = udwReqLen;
    pSession->m_dwClientReqEnType = CConfBase::GetInt("NeedEncodeAndCompress");
    pSession->m_dwClientReqMode = EN_CLIENT_REQ_MODE__TCP;

    // 4. 插入任务队列
    TSE_LOG_INFO(m_pLog, ("m_poTaskQueue->WaitTillPush: session[%p] [seq=%u]", pSession, m_udwSeqno));
    m_poTaskQueue->WaitTillPush(pSession);

    return S_OK;
}
