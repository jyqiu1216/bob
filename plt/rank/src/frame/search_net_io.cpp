#include "search_net_io.h"
#include "conf_base.h"
#include "global_serv.h"
#include "service_key.h"

TINT32 CSearchNetIO::Init( CConf *pobjConf, CTseLogger *pLog, CTseLogger *pRegLog, CTaskQueue *poTaskQueue )
{
	if( NULL == pobjConf || NULL == pLog || NULL == poTaskQueue)
	{
		return -1;
	}

	TBOOL bRet = -1;

	// 1. 设置配置对象,日志对象和任务队列
	m_poConf = pobjConf;
	m_poServLog = pLog;
	m_poRegLog = pRegLog;
	m_poTaskQueue = poTaskQueue;

	// 2. 创建长连接对象
	m_pLongConn = CreateLongConnObj();
	if (m_pLongConn == NULL)
	{
		TSE_LOG_ERROR(m_poServLog, ("Create longconn failed!"));
		return -2;
	}

	// 3. 侦听端口
    m_hListenSock = CreateListenSocket(CGlobalServ::m_poConf->m_szModuleIp, CConfBase::GetInt("search_port"));
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

    m_pobjClientReq = new ClientRequest;

    m_udwSeqNo = 1000;

	return 0;
}

void * CSearchNetIO::RoutineNetIO( void *pParam )
{
	CSearchNetIO *net_io = (CSearchNetIO *)pParam;
	while (1)
	{
		net_io->m_pLongConn->RoutineLongConn(100);
	}

	return NULL;
}

void CSearchNetIO::OnUserRequest( LongConnHandle hSession, const TUCHAR *pszData, TUINT32 udwDataLen, BOOL &bWillResponse )
{
	TCHAR *pszIp = 0;
	TUINT16 uwPort = 0;
	TUINT16         uwServType      = 0;
	TUINT32         udwReqSeq       = 0;
    SSession        *pSession       = NULL;
    TUCHAR          *pszValBuf      = 0;
    TUINT32         udwValBufLen    = 0;
    TUINT32         udwClientSeq = 0;

	// 1. Get src Ip and Port
	GetIp2PortByHandle(hSession, &uwPort, &pszIp);

    if (m_udwSeqNo < 1000)
    {
        m_udwSeqNo = 1000;
    }
    m_udwSeqNo++;

	// 2. unpack
	m_pUnPackTool->UntachPackage();
	m_pUnPackTool->AttachPackage((TUCHAR *)pszData, udwDataLen);
	uwServType = m_pUnPackTool->GetServiceType();
	udwReqSeq = m_pUnPackTool->GetSeq();
    TSE_LOG_DEBUG(m_poServLog, ("OnUserRequest: get new request from[%s:%u] [seq=%u]", pszIp, uwPort, udwReqSeq));
    if(uwServType != EN_SERVICE_TYPE__CLIENT__RANK_REQ
        && uwServType != EN_SERVICE_TYPE__SVR__RANK_REQ)
    {
        bWillResponse = FALSE;
        TSE_LOG_ERROR(m_poServLog, ("OnUserRequest: error service_type[%d] [seq=%u]", uwServType, udwReqSeq));
        goto ErrCode;
    }

    if (FALSE == m_pUnPackTool->Unpack())
    {
        bWillResponse = FALSE;
        TSE_LOG_ERROR(m_poServLog, ("Client request unpack fail [seq=%u]", udwReqSeq));
        goto ErrCode;
    }

    m_pUnPackTool->GetVal(EN_GLOBAL_KEY__REQ_SEQ, &udwClientSeq);

    if (FALSE == m_pUnPackTool->GetVal(EN_GLOBAL_KEY__REQ_BUF, &pszValBuf, &udwValBufLen))
    {
        bWillResponse = FALSE;
        TSE_LOG_ERROR(m_poServLog, ("Client request get EN_GLOBAL_KEY__REQ_BUF fail [seq=%u]", udwReqSeq));
        goto ErrCode;
    }

    if (uwServType == EN_SERVICE_TYPE__CLIENT__RANK_REQ)
    {
        m_pobjClientReq->Clear();
        if (false == m_pobjClientReq->ParseFromArray(pszValBuf, udwValBufLen))
        {
            TSE_LOG_ERROR(m_poServLog, ("Client request parse pb fail [seq=%u]", udwReqSeq));
            goto ErrCode;
        }
    }
    
    // 2. 获取session对象
    if (CSessionMgr::Instance()->WaitTillSession(&pSession) != 0)
    {
        TSE_LOG_ERROR(m_poServLog, ("Get new session failed! [seq=%u]", udwReqSeq));
        goto ErrCode;
    }
    else
    {
        TSE_LOG_INFO(m_poServLog, ("[NEW_REQ] from[%s:%u], session_empty_size=%u, [seq=%u]", pszIp, uwPort, CSessionMgr::Instance()->GetEmptySessionSize(), udwReqSeq));
    }

    // 3. 设置session信息
    pSession->Reset();
    pSession->m_ucIsUsing = 1;
    pSession->m_udwExpectProcedure = EN_PROCEDURE__CLIENT_REQUEST;
    pSession->m_uddwTimeBeg = CTimeUtils::GetCurTimeUs();
    pSession->m_udwSeqNo = m_udwSeqNo;
    pSession->m_udwPackSeqNo = udwReqSeq;
    pSession->m_udwClientSeqNo = udwClientSeq;
    pSession->m_stClientHandle = hSession;
    pSession->m_dwServiceType = uwServType;
    if (pSession->m_dwServiceType == EN_SERVICE_TYPE__CLIENT__RANK_REQ)
    {
        pSession->m_udwPbSeqNo = m_pobjClientReq->seq();
        pSession->m_udwClientReqBufLen = m_pobjClientReq->req_url().size();
        memcpy(pSession->m_szClientReqBuf, m_pobjClientReq->req_url().c_str(), pSession->m_udwClientReqBufLen);
        pSession->m_szClientReqBuf[pSession->m_udwClientReqBufLen] = 0;
    }
    else
    {
        pSession->m_udwPbSeqNo = 0;
        pSession->m_udwClientReqBufLen = udwValBufLen;
        memcpy(pSession->m_szClientReqBuf, pszValBuf, udwValBufLen);
        pSession->m_szClientReqBuf[udwValBufLen] = 0;
    }
    pSession->m_dwClientReqMode = EN_CLIENT_REQ_MODE__TCP;

    // 4. 插入任务队列
    m_poTaskQueue->WaitTillPush(pSession);
    return;

ErrCode:
    TSE_LOG_DEBUG(m_poServLog, ("req error, SendBackResult [seq=%u]", udwReqSeq));
    bWillResponse = FALSE;
    return;
}

void CSearchNetIO::OnTasksFinishedCallBack( LTasksGroup *pstTasksGrp )
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

	// 2> copy the msg mem to session wrapper
	TSE_LOG_DEBUG(m_poServLog, ("recv res next=%u,exp_serv=%u,session[%p] [seq=%u]", \
		poSession->m_udwNextProcedure, poSession->m_udwExpectProcedure, poSession, poSession->m_udwSeqNo));
	switch (poSession->m_udwExpectProcedure)
	{
	case EN_PROCEDURE__EXPERT_NODE_AWS:
		OnAwsResponse(pstTasksGrp, poSession);
		break;
	default :
		TSE_LOG_ERROR(m_poServLog, ("Invalid expect service type[%u] [seq=%u]", poSession->m_udwExpectProcedure, poSession->m_udwSeqNo));
		break;
	}

	// 3> push session wrapper to work queue
	m_poTaskQueue->WaitTillPush(poSession);
}

SOCKET CSearchNetIO::CreateListenSocket( TCHAR *pszListenHost, TUINT16 uwPort )
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

	// 2> 绑定端口
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

TINT32 CSearchNetIO::GetIp2PortByHandle( LongConnHandle stHandle, TUINT16 *puwPort, TCHAR **ppszIp )
{
	TUINT32 udwHost = 0;
	TUINT16 uwPort = 0;

	m_pLongConn->GetPeerName(stHandle, &udwHost, &uwPort);
	*puwPort = ntohs(uwPort);
	*ppszIp = inet_ntoa(*(in_addr *)&udwHost);
	return 0;
}

TINT32  CSearchNetIO::OnAwsResponse(LTasksGroup *pstTasksGrp, SSession *poSession)
{
	LTask               *pstTask                = 0;
	TCHAR               *pszIp                  = 0;
	TUINT16             uwPort                  = 0;
	TUINT32				udwIdx					= 0;
	TINT32				dwRetCode				= 0;

	poSession->m_uddwDownRqstTimeEnd = CTimeUtils::GetCurTimeUs();

    if (poSession->m_udwDownRqstType == 1)
    {
        poSession->m_uddwAwsReadSumTime += poSession->m_uddwDownRqstTimeEnd - poSession->m_uddwDownRqstTimeBeg;
    }
    else if (poSession->m_udwDownRqstType == 2)
    {
        poSession->m_uddwAwsWriteSumTime += poSession->m_uddwDownRqstTimeEnd - poSession->m_uddwDownRqstTimeBeg;
    }
    else if (poSession->m_udwDownRqstType == 3)
    {
        poSession->m_uddwAwsReadWriteSumTime += poSession->m_uddwDownRqstTimeEnd - poSession->m_uddwDownRqstTimeBeg;
    }
    else
    {
        poSession->m_uddwAwsNoOpSumTime += poSession->m_uddwDownRqstTimeEnd - poSession->m_uddwDownRqstTimeBeg;
    }


	//poSession->ResetAwsRsp();
	poSession->ResetAwsInfo();
	vector<AwsRspInfo*>& vecRsp = poSession->m_vecAwsRsp;
	AwsRspInfo* pAwsRsp = NULL;

    TSE_LOG_DEBUG(m_poServLog, ("aws res: ParseAwsResponse: [vaildtasksnum=%u] [seq=%u]", \
                                pstTasksGrp->m_uValidTasks, 
                                poSession->m_udwSeqNo));


	for(udwIdx = 0; udwIdx < pstTasksGrp->m_uValidTasks; udwIdx++)
	{
		pstTask = &pstTasksGrp->m_Tasks[udwIdx];



        /**************************************************************************************/ 
        // 获取原始请求的信息,方便在异常的时候知道是哪条请求出问题,能更好的获取数据层面的信息
        /*
        AwsReqInfo OriginAwsReq;        
        TUCHAR *pszOriginValBuf = NULL;
        TUINT32 udwOriginValBufLen = 0;
        m_pUnPackTool->UntachPackage();
        m_pUnPackTool->AttachPackage(pstTask->pData, pstTask->uValidDataLen);
        if(FALSE == m_pUnPackTool->Unpack())
        {
            TSE_LOG_DEBUG(m_poServLog, ("aws res: taskid[%u]: origin aws req unpack error [seq=%u]", \
                                        udwIdx, \
                                        poSession->m_udwSeqNo));
        }
        else
        {     
            m_pUnPackTool->GetVal(EN_GLOBAL_KEY__INDEX_NO, &OriginAwsReq.udwIdxNo);
            if(m_pUnPackTool->GetVal(EN_GLOBAL_KEY__TABLE_NAME, &pszOriginValBuf, &udwOriginValBufLen))
            {
                OriginAwsReq.sTableName.resize(udwOriginValBufLen);
                memcpy((char*)OriginAwsReq.sTableName.c_str(), pszOriginValBuf, udwOriginValBufLen);
            }            
            if (m_pUnPackTool->GetVal(EN_GLOBAL_KEY__OPERATOR_NAME, &pszOriginValBuf, &udwOriginValBufLen))
            {
                OriginAwsReq.sOperatorName.resize(udwOriginValBufLen);
                memcpy((char*)OriginAwsReq.sOperatorName.c_str(), pszOriginValBuf, udwOriginValBufLen);
            }
        }
        */
        /**************************************************************************************/ 
        
		GetIp2PortByHandle(pstTask->hSession, &uwPort, &pszIp);
		
		pAwsRsp = new AwsRspInfo;
		vecRsp.push_back(pAwsRsp);

		TSE_LOG_DEBUG(m_poServLog, ("aws res: taskid[%u]: [ip=%s] [port=%u] [send=%u], [recv=%u], [timeout=%u], [donw_busy=%u], [socket_closed=%u], [verify_failed=%u] [recv_data_len=%u] [cost_time=%llu]us [req_type=%u] [seq=%u]",
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
                                    poSession->m_udwDownRqstType, \
                                    poSession->m_udwSeqNo));

		if(0 == pstTask->_ucIsSendOK || 0 == pstTask->_ucIsReceiveOK || 1 == pstTask->_ucTimeOutEvent )
		{
			// 加入超时统计
            if(poSession->m_bAwsProxyNodeExist)
            {
                CDownMgr::Instance()->zk_AddTimeOut(poSession->m_pstAwsProxyNode);
            }
			TSE_LOG_ERROR(m_poServLog, ("aws res: [send=%u], [recv=%u], [timeout=%u], [cost_time=%llu]us [seq=%u]",
                           				pstTask->_ucIsSendOK, \
                        				pstTask->_ucIsReceiveOK, \
                        				pstTask->_ucTimeOutEvent, \
                        				poSession->m_uddwDownRqstTimeEnd - poSession->m_uddwDownRqstTimeBeg, \
                        				poSession->m_udwSeqNo));
            if(EN_RET_CODE__SUCCESS == poSession->m_stCommonResInfo.m_dwRetCode)
            {
                poSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__TIMEOUT;
                dwRetCode = -1;
            }
			// break;
		}
		else
		{
			if(pstTask->_uReceivedDataLen > MAX_AWS_RES_DATA_LEN)
			{
				TSE_LOG_ERROR(m_poServLog, ("aws res: recv_data_len[%u]>MAX_HS_RES_DATA_LEN [cost_time=%llu]us [seq=%u]",
                        					pstTask->_uReceivedDataLen, \
                        					poSession->m_uddwDownRqstTimeEnd - poSession->m_uddwDownRqstTimeBeg, \
                        					poSession->m_udwSeqNo));
                if(EN_RET_CODE__SUCCESS == poSession->m_stCommonResInfo.m_dwRetCode)
                {
    				poSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PACKAGE_LEN_OVERFLOW;
    				dwRetCode = -2;
                }
				// break;
			}
			else
			{
				dwRetCode = ParseAwsResponse(pstTask->_pReceivedData, pstTask->_uReceivedDataLen, pAwsRsp);
				if(dwRetCode < 0)
				{
					// 加入错误统计
					if (poSession->m_bAwsProxyNodeExist)
					{
						CDownMgr::Instance()->zk_AddError(poSession->m_pstAwsProxyNode);
					}
					
					TSE_LOG_ERROR(m_poServLog, ("aws res: parse failed[%d] [seq=%u]", \
                                                dwRetCode, \
                                                poSession->m_udwSeqNo));
                    if(EN_RET_CODE__SUCCESS == poSession->m_stCommonResInfo.m_dwRetCode)
                    {
    					poSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_PACKAGE_ERR;
    					dwRetCode = -3;
                    }
					// break;
				}
				else
				{
					TSE_LOG_DEBUG(m_poServLog, ("aws res: parse ok. service[%u], ret[%d], table[%s], buf_len[%u] [seq=%u]", \
						                        pAwsRsp->udwServiceType, \
						                        pAwsRsp->dwRetCode, \
						                        pAwsRsp->sTableName.c_str(), \
						                        pAwsRsp->sRspContent.size(), \
						                        poSession->m_udwSeqNo));
				}
			}
		}
	}

    poSession->m_udwDownRqstType = 0;

	return 0;
}

TINT32 CSearchNetIO::ParseAwsResponse(TUCHAR *pszPack, TUINT32 udwPackLen, AwsRspInfo* pAwsRspInfo)
{
	TUCHAR *pszValBuf = NULL;
	TUINT32 udwValBufLen = 0;


	m_pUnPackTool->UntachPackage();
	m_pUnPackTool->AttachPackage(pszPack, udwPackLen);
	if(FALSE == m_pUnPackTool->Unpack())
	{
		return -1;
	}
	pAwsRspInfo->udwServiceType = m_pUnPackTool->GetServiceType();
	m_pUnPackTool->GetVal(EN_GLOBAL_KEY__RES_CODE, &pAwsRspInfo->dwRetCode);
	m_pUnPackTool->GetVal(EN_GLOBAL_KEY__RES_COST_TIME, &pAwsRspInfo->udwCostTime);
    m_pUnPackTool->GetVal(EN_GLOBAL_KEY__INDEX_NO, &pAwsRspInfo->udwIdxNo);
    if (m_pUnPackTool->GetVal(EN_GLOBAL_KEY__TABLE_NAME, &pszValBuf, &udwValBufLen))
    {
        pAwsRspInfo->sTableName.resize(udwValBufLen);
        memcpy((char*)pAwsRspInfo->sTableName.c_str(), pszValBuf, udwValBufLen);
    }
    if (m_pUnPackTool->GetVal(EN_GLOBAL_KEY__OPERATOR_NAME, &pszValBuf, &udwValBufLen))
    {
        pAwsRspInfo->sOperatorName.resize(udwValBufLen);
        memcpy((char*)pAwsRspInfo->sOperatorName.c_str(), pszValBuf, udwValBufLen);
    }
	
    //2XX的返回码,都是正常的情况
	if(pAwsRspInfo->dwRetCode/100 != 2)
	{
		TSE_LOG_ERROR(m_poServLog, ("aws res: pAwsRspInfo->dwRetCode=%d\n", pAwsRspInfo->dwRetCode));
		return -2;
	}
	
	if(pAwsRspInfo->udwServiceType == EN_SERVICE_TYPE_QUERY_DYNAMODB_RSP)
	{
		m_pUnPackTool->GetVal(EN_GLOBAL_KEY__RES_BUF, &pszValBuf, &udwValBufLen);
		pAwsRspInfo->sRspContent.resize(udwValBufLen);
		memcpy((char*)pAwsRspInfo->sRspContent.c_str(), pszValBuf, udwValBufLen);
	}
	else
	{
		return -3;
	}

	return 0;	
}
