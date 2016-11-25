#include "query_net_io.h"
#include "session_mgr.h"
#include "conf_base.h"
#include "global_serv.h"

TINT32 CQueryNetIO::Init( CConf *pobjConf, CTseLogger *pLog, CTaskQueue *poTaskQueue )
{
	if( NULL == pobjConf || NULL == pLog || NULL == poTaskQueue)
	{
		return -1;
	}

	TBOOL bRet = -1;

	// 1. 设置配置对象,日志对象和任务队列
	m_poConf = pobjConf;
	m_pLog = pLog;
	m_poTaskQueue = poTaskQueue;

	// 2. 创建长连接对象
	m_pLongConn = CreateLongConnObj();
	if (m_pLongConn == NULL)
	{
		TSE_LOG_ERROR(m_pLog, ("Create longconn failed!"));
		return -2;
	}

	// 3. 侦听端口
    m_hListenSock = CreateListenSocket(CGlobalServ::m_poConf->m_szModuleIp, CConfBase::GetInt("query_port"));
	if (m_hListenSock < 0)
	{
		TSE_LOG_ERROR(m_pLog, ("Create listen socket fail"));
		return -3;
	}

	// 4. 初始化长连接, 初始化为http模式
	bRet = m_pLongConn->InitLongConn(this, MAX_NETIO_LONGCONN_SESSION_NUM, m_hListenSock, MAX_NETIO_LONGCONN_TIMEOUT_MS, 0, 0, 1);
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

	return 0;
}

void * CQueryNetIO::RoutineNetIO( void *pParam )
{
	CQueryNetIO *net_io = (CQueryNetIO *)pParam;
	while (1)
	{
		net_io->m_pLongConn->RoutineLongConn(100);
	}

	return NULL;
}

void CQueryNetIO::OnUserRequest( LongConnHandle hSession, const TUCHAR *pData, TUINT32 uLen, BOOL &bWillResponse )
{
	TCHAR *pszIp = 0;
	TUINT16 uwPort = 0;
	SSession    *pSession = NULL;

	m_udwSeqno++;

	// Get src Ip and Port
	GetIp2PortByHandle(hSession, &uwPort, &pszIp);

	TSE_LOG_DEBUG(m_pLog, ("New request from [%s:%u] [len=%u] [seq=%u]", pszIp, uwPort, uLen, m_udwSeqno));

	// 1. 判定包长
	if(uLen > MAX_HTTP_REQ_LEN)
	{
		TSE_LOG_ERROR(m_pLog, ("Http req len [%u] larger than [%u] [seq=%u]",
			uLen, MAX_HTTP_REQ_LEN, m_udwSeqno));
		goto ErrCode;
	}

	// 2. 获取session对象
	if (CSessionMgr::Instance()->WaitTillSession(&pSession) != 0)
	{
		TSE_LOG_ERROR(m_pLog, ("Get new session failed! [seq=%u]", m_udwSeqno));
		goto ErrCode;
	}
	else
	{
		TSE_LOG_INFO(m_pLog, ("Get new session, empty_size=%u, [seq=%u]", CSessionMgr::Instance()->GetEmptySessionSize(), m_udwSeqno));
	}

	// 3. 设置session信息
	pSession->Reset();
	pSession->m_ucIsUsing = 1;
	pSession->m_udwExpectProcedure = EN_PROCEDURE__CLIENT_REQUEST;
	pSession->m_uddwTimeBeg = CTimeUtils::GetCurTimeUs();
	pSession->m_udwSeqNo = m_udwSeqno;
	//pSession->m_stUserInfo.m_udwBSeqNo = pSession->m_udwSeqNo;
	pSession->m_stClientHandle = hSession;
	memcpy(pSession->m_szClientReqBuf, pData, uLen);
	pSession->m_szClientReqBuf[uLen] = 0;
	pSession->m_udwClientReqBufLen = uLen;
    pSession->m_dwClientReqMode = EN_CLIENT_REQ_MODE__HTTP;

	// 4. 插入任务队列
	m_poTaskQueue->WaitTillPush(pSession);
	return;

ErrCode:
	TSE_LOG_DEBUG(m_pLog, ("req error, SendBackResult [seq=%u]", m_udwSeqno));
	SendHttpBackErr(hSession, m_udwSeqno);
	return;
}

void CQueryNetIO::OnTasksFinishedCallBack( LTasksGroup *pTasksgrp )
{
	//do nothing;
}

SOCKET CQueryNetIO::CreateListenSocket( TCHAR *pszListenHost, TUINT16 uwPort )
{
	// 1> 申请SOCKET
	SOCKET lSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (lSocket < 0)
	{
		return -1;
	}

	// 2> 设置端口可重用
	int option = 1;
	if ( setsockopt ( lSocket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof( option ) ) < 0 ) 
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
	int rv = bind(lSocket, (struct sockaddr *) &sa,  sizeof(sa));

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

TINT32 CQueryNetIO::GetIp2PortByHandle( LongConnHandle stHandle, TUINT16 *puwPort, TCHAR **ppszIp )
{
	TUINT32 udwHost = 0;
	TUINT16 uwPort = 0;

	m_pLongConn->GetPeerName(stHandle, &udwHost, &uwPort);
	*puwPort = ntohs(uwPort);
	*ppszIp = inet_ntoa(*(in_addr *)&udwHost);
	return 0;
}

TINT32 CQueryNetIO::SendHttpBackErr( LongConnHandle stHandle, TUINT32 udwSeqno )
{
	// to do;
	return 0;
}
