#include "search_net_io.h"
#include "conf_base.h"
#include "global_serv.h"

TINT32 CSearchNetIO::Init(CTseLogger *pLog, CTaskQueue *poTaskQueue)
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

    m_hListenSock = CreateListenSocket(CGlobalServ::m_poConf->m_szModuleIp, CConfBase::GetInt("reg_port"));
    if (m_hListenSock < 0)
    {
        TSE_LOG_ERROR(m_poServLog, ("Create listen socket fail"));
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
        return -1;
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
        return -1;
    }

    // 3> 监听
    rv = listen(lSocket, uwPort);

    if (rv < 0)
    {
        close(lSocket);
        return -1;
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
