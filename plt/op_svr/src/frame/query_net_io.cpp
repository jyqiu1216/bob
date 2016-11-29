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
    m_hListenSock = CreateListenSocket(CGlobalServ::m_poConf->m_szModuleIp, CConfBase::GetInt("serv_port"));
    if (m_hListenSock < 0)
    {
        TSE_LOG_ERROR(m_poServLog, ("Create listen socket fail"));
        return -3;
    }

    // 4. 初始化长连接
    bRet = m_pLongConn->InitLongConn(this, MAX_NETIO_LONGCONN_SESSION_NUM, m_hListenSock, MAX_NETIO_LONGCONN_TIMEOUT_MS, 0, 0, 0);
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

    m_udwSeqno = 1000;

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
    TCHAR *pszIp = 0;
    TUINT16 uwPort = 0;
    SSession    *pSession = NULL;

    //防止seq溢出
    if (m_udwSeqno < 1000)
    {
        m_udwSeqno = 1000;
    }
    m_udwSeqno++;

    // Get src Ip and Port
    GetIp2PortByHandle(hSession, &uwPort, &pszIp);

    TSE_LOG_DEBUG(m_poServLog, ("New request from [%s:%u] [len=%u] [seq=%u]", pszIp, uwPort, uLen, m_udwSeqno));

    //---------req process begin-------------
    //TODO:
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
