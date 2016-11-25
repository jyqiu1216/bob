#include "search_net_io.h"
#include "conf_base.h"
#include "global_serv.h"

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
	TUINT32         udwRegSeq       = 0;

	// 1. Get src Ip and Port
	GetIp2PortByHandle(hSession, &uwPort, &pszIp);

	// 2. Get Service Type
	m_pUnPackTool->UntachPackage();
	m_pUnPackTool->AttachPackage((TUCHAR *)pszData, udwDataLen);
	uwServType = m_pUnPackTool->GetServiceType();
	udwRegSeq = m_pUnPackTool->GetSeq();

	// 3> check the message
	switch (uwServType)
	{
	default :
		TSE_LOG_ERROR(m_poServLog, ("Unknown service type=[%u] [reqseq=%u]", uwServType, udwRegSeq));
		bWillResponse = FALSE;
	}
	
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

    // 1.2> check seqno
    if(poSession->m_udwSeqNo != pstTasksGrp->m_UserData2.u32)
    {
        TSE_LOG_ERROR(m_poServLog, ("CSearchNetIO::OnTasksFinishedCallBack, session[%p],sseq=%u [seq=%u]", poSession, poSession->m_udwSeqNo, pstTasksGrp->m_UserData2.u32));
        //return;
    }

    if(TRUE == poSession->m_bProcessing)
    {
        TSE_LOG_ERROR(m_poServLog, ("CSearchNetIO::OnTasksFinishedCallBack is processing, session[%p] [seq=%u]", poSession, poSession->m_udwSeqNo));
        return;
    }
    else
    {
        poSession->m_bProcessing = TRUE;
    }

	// 2> copy the msg mem to session wrapper
	TSE_LOG_DEBUG(m_poServLog, ("recv res next=%u,exp_serv=%u,session[%p] [seq=%u]", 
		poSession->m_udwNextProcedure, poSession->m_udwExpectProcedure, poSession, poSession->m_udwSeqNo));
	switch (poSession->m_udwExpectProcedure)
	{
    case EN_EXPECT_PROCEDURE__AWS:
        OnAwsResponse(pstTasksGrp, poSession);
        break;
    case EN_EXPECT_PROCEDURE__DB:
        OnDbResponse(pstTasksGrp, poSession);
        break;
    case EN_EXPECT_PROCEDURE__LOCK_GET:
        OnLockResponse(pstTasksGrp, poSession);
        break;
    case EN_EXPECT_PROCEDURE__LOCK_RELEASE:
        poSession->m_uddwDownRqstTimeEnd = CTimeUtils::GetCurTimeUs();
        poSession->m_uddwLockSumTime += poSession->m_uddwDownRqstTimeEnd - poSession->m_uddwDownRqstTimeBeg;
        break;
    case EN_EXPECT_PROCEDURE__EVENT:
        OnEventResponse(pstTasksGrp, poSession);
        break;
    case EN_EXPECT_PROCEDURE__CACHE:
        OnCacheResponse(pstTasksGrp, poSession);
        break;
    case EN_EXPECT_PROCEDURE__TRANSLATE:
        OnTranslateResponse(pstTasksGrp, poSession);
        break;
    case EN_EXPECT_PROCEDURE__DATA_CENTER:
        OnDataCenterResponse(pstTasksGrp, poSession);
        break;
    case EN_EXPECT_PROCEDURE__IAP_SVR:
        OnIapSvrResponse(pstTasksGrp, poSession);
        break;
    case EN_EXPECT_PROCEDURE__MAP_SVR:
        OnMapSvrResponse(pstTasksGrp, poSession);
        break;
    case EN_EXPECT_PROCEDURE__ACTION_SVR:
        OnActionSvrResponse(pstTasksGrp, poSession);
        break;
    case EN_EXPECT_PROCEDURE__REPORT_SVR:
        OnReportSvrResponse(pstTasksGrp, poSession);
        break;
    case EN_EXPECT_PROCEDURE__PURCHASE_CHECK:
        OnPurchaseCheckResponse(pstTasksGrp, poSession);
        break;
    case EN_EXPECT_PROCEDURE__RANK_SVR:
        OnRankSvrResponse(pstTasksGrp, poSession);
        break;
    case EN_EXPECT_PROCEDURE__USER_LINKER:
        assert(0);
        break;
    default:
        TSE_LOG_ERROR(m_poServLog, ("Invalid expect service type[%u] [seq=%u]", poSession->m_udwExpectProcedure, poSession->m_udwSeqNo));
        break;
	}

	// 3> push session wrapper to work queue
    TUINT64 uddwTimeBeg = CTimeUtils::GetCurTimeUs();
	m_poTaskQueue->WaitTillPush(poSession);
    TSE_LOG_INFO(m_poServLog, ("m_poTaskQueue->WaitTillPush: session[%p], push_cost_time[%ld] [seq=%u]", poSession, CTimeUtils::GetCurTimeUs()-uddwTimeBeg,  poSession->m_udwSeqNo));
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
            if (poSession->m_bAwsProxyNodeExist && pstTask->hSession == poSession->m_pstAwsProxyNode->m_stDownHandle)
            {
                CDownMgr::Instance()->zk_AddTimeOut(poSession->m_pstAwsProxyNode);
            }
            else if (poSession->m_bMapSvrExist && pstTask->hSession == poSession->m_pstMapSvrNode->m_stDownHandle)
            {
                CDownMgr::Instance()->zk_AddTimeOut(poSession->m_pstMapSvrNode);
            }
            else if (poSession->m_bBuffActionSvrNodeExist && pstTask->hSession == poSession->m_pstBuffActionSvrNode->m_stDownHandle)
            {
                CDownMgr::Instance()->zk_AddTimeOut(poSession->m_pstBuffActionSvrNode);
            }
            else if (poSession->m_bAlActionSvrNodeExist && pstTask->hSession == poSession->m_pstAlActionSvrNode->m_stDownHandle)
            {
                CDownMgr::Instance()->zk_AddTimeOut(poSession->m_pstAlActionSvrNode);
            }
            else if (poSession->m_bMarchActionSvrNodeExist && pstTask->hSession == poSession->m_pstMarchActionSvrNode->m_stDownHandle)
            {
                CDownMgr::Instance()->zk_AddTimeOut(poSession->m_pstMarchActionSvrNode);
            }
            else if (poSession->m_bReportUserSvrNodeExist && pstTask->hSession == poSession->m_pstReportUserSvrNode->m_stDownHandle)
            {
                CDownMgr::Instance()->zk_AddTimeOut(poSession->m_pstReportUserSvrNode);
            }
            /*
			TSE_LOG_ERROR(m_poServLog, ("aws res: [otablename=%s] [ooperation=%s] [oopentype=%u] [send=%u], [recv=%u], [timeout=%u], [cost_time=%llu]us [seq=%u]",
                                        OriginAwsReq.sTableName.c_str(), \
                                        OriginAwsReq.sOperatorName.c_str(), \
                                        OriginAwsReq.udwIdxNo, \
                        				pstTask->_ucIsSendOK, \
                        				pstTask->_ucIsReceiveOK, \
                        				pstTask->_ucTimeOutEvent, \
                        				poSession->m_uddwDownRqstTimeEnd - poSession->m_uddwDownRqstTimeBeg, \
                        				poSession->m_udwSeqNo));
            */
            TSE_LOG_ERROR(m_poServLog, ("aws res: [ip=%s] [port=%u] [send=%u], [recv=%u], [timeout=%u], [cost_time=%llu]us [seq=%u]",
                pszIp, \
                uwPort, \
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
			
				TSE_LOG_ERROR(m_poServLog, ("aws res: [ip=%s] [port=%u] recv_data_len[%u]>MAX_HS_RES_DATA_LEN [cost_time=%llu]us [seq=%u]",
                    pszIp, \
                    uwPort, \
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
                    if (poSession->m_bAwsProxyNodeExist && pstTask->hSession == poSession->m_pstAwsProxyNode->m_stDownHandle)
                    {
                        CDownMgr::Instance()->zk_AddError(poSession->m_pstAwsProxyNode);
                    }
                    else if (poSession->m_bMapSvrExist && pstTask->hSession == poSession->m_pstMapSvrNode->m_stDownHandle)
                    {
                        CDownMgr::Instance()->zk_AddError(poSession->m_pstMapSvrNode);
                    }
                    else if (poSession->m_bBuffActionSvrNodeExist && pstTask->hSession == poSession->m_pstBuffActionSvrNode->m_stDownHandle)
                    {
                        CDownMgr::Instance()->zk_AddError(poSession->m_pstBuffActionSvrNode);
                    }
                    else if (poSession->m_bAlActionSvrNodeExist && pstTask->hSession == poSession->m_pstAlActionSvrNode->m_stDownHandle)
                    {
                        CDownMgr::Instance()->zk_AddError(poSession->m_pstAlActionSvrNode);
                    }
                    else if (poSession->m_bMarchActionSvrNodeExist && pstTask->hSession == poSession->m_pstMarchActionSvrNode->m_stDownHandle)
                    {
                        CDownMgr::Instance()->zk_AddError(poSession->m_pstMarchActionSvrNode);
                    }
                    else if (poSession->m_bReportUserSvrNodeExist && pstTask->hSession == poSession->m_pstReportUserSvrNode->m_stDownHandle)
                    {
                        CDownMgr::Instance()->zk_AddError(poSession->m_pstReportUserSvrNode);
                    }
					
					TSE_LOG_ERROR(m_poServLog, ("aws res: [ip=%s] [port=%u] failed[%d] [seq=%u]", 
                        pszIp, \
                        uwPort, \
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
					TSE_LOG_DEBUG(m_poServLog, ("aws res: service[%u], ret[%d], table[%s], buf_len[%u] [seq=%u]", 
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

TINT32  CSearchNetIO::OnDbResponse(LTasksGroup *pstTasksGrp, SSession *poSession)
{
    LTask               *pstTask                = 0;
    TCHAR               *pszIp                  = 0;
    TUINT16             uwPort                  = 0;
    TUINT32				udwIdx					= 0;
    TINT32				dwRetCode				= 0;

    poSession->m_uddwDownRqstTimeEnd = CTimeUtils::GetCurTimeUs();

    if (poSession->m_udwDownRqstType == 1)
    {
        poSession->m_uddwDbReadSumTime += poSession->m_uddwDownRqstTimeEnd - poSession->m_uddwDownRqstTimeBeg;
    }
    else if (poSession->m_udwDownRqstType == 2)
    {
        poSession->m_uddwDbWriteSumTime += poSession->m_uddwDownRqstTimeEnd - poSession->m_uddwDownRqstTimeBeg;
    }
    else if (poSession->m_udwDownRqstType == 3)
    {
        poSession->m_uddwDbReadWriteSumTime += poSession->m_uddwDownRqstTimeEnd - poSession->m_uddwDownRqstTimeBeg;
    }
    else
    {
        poSession->m_uddwDbNoOpSumTime += poSession->m_uddwDownRqstTimeEnd - poSession->m_uddwDownRqstTimeBeg;
    }

    poSession->ResetDbInfo();
    vector<DbRspInfo*>& vecRsp = poSession->m_vecDbRsp;
    DbRspInfo* pDbRsp = NULL;

    for(udwIdx = 0; udwIdx < pstTasksGrp->m_uValidTasks; udwIdx++)
    {
        pstTask = &pstTasksGrp->m_Tasks[udwIdx];

        /**************************************************************************************/ 
        // 获取原始请求的信息,方便在异常的时候知道是哪条请求出问题,能更好的获取数据层面的信息
        /*
        DbRspInfo OriginDbReq;        
        TUCHAR *pszOriginValBuf = NULL;
        TUINT32 udwOriginValBufLen = 0;
        m_pUnPackTool->UntachPackage();
        m_pUnPackTool->AttachPackage(pstTask->pData, pstTask->uValidDataLen);
        if(FALSE == m_pUnPackTool->Unpack())
        {
            TSE_LOG_DEBUG(m_poServLog, ("db res: taskid[%u]: origin db req unpack error [seq=%u]", \
                                        udwIdx, \
                                        poSession->m_udwSeqNo));
        }
        else
        {     
            m_pUnPackTool->GetVal(EN_GLOBAL_KEY__INDEX_NO, &OriginDbReq.udwIdxNo);
            if(m_pUnPackTool->GetVal(EN_GLOBAL_KEY__TABLE_NAME, &pszOriginValBuf, &udwOriginValBufLen))
            {
                OriginDbReq.sTableName.resize(udwOriginValBufLen);
                memcpy((char*)OriginDbReq.sTableName.c_str(), pszOriginValBuf, udwOriginValBufLen);
            }            
            if (m_pUnPackTool->GetVal(EN_GLOBAL_KEY__OPERATOR_NAME, &pszOriginValBuf, &udwOriginValBufLen))
            {
                OriginDbReq.sOperatorName.resize(udwOriginValBufLen);
                memcpy((char*)OriginDbReq.sOperatorName.c_str(), pszOriginValBuf, udwOriginValBufLen);
            }
        }
        */
        /**************************************************************************************/ 
        
        GetIp2PortByHandle(pstTask->hSession, &uwPort, &pszIp);

        pDbRsp = new DbRspInfo;
        vecRsp.push_back(pDbRsp);

        TSE_LOG_DEBUG(m_poServLog, ("db res: taskid[%u]: [ip=%s] [port=%u] [send=%u], [recv=%u], [timeout=%u], [donw_busy=%u], [socket_closed=%u], [verify_failed=%u] [recv_data_len=%u] [cost_time=%llu]us [req_type=%u] [seq=%u]",
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
            if(poSession->m_bDbProxyNodeExist)
            {
                CDownMgr::Instance()->zk_AddTimeOut(poSession->m_pstDbProxyNode);
            }
            TSE_LOG_ERROR(m_poServLog, ("db res: [ip=%s] [port=%u] [send=%u], [recv=%u], [timeout=%u], [cost_time=%llu]us [seq=%u]",
                pszIp, \
                uwPort, \
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
                TSE_LOG_ERROR(m_poServLog, ("db res: [ip=%s] [port=%u] recv_data_len[%u]>MAX_HS_RES_DATA_LEN [cost_time=%llu]us [seq=%u]",
                    pszIp, \
                    uwPort, \
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
                dwRetCode = ParseDbResponse(pstTask->_pReceivedData, pstTask->_uReceivedDataLen, pDbRsp);
                if(dwRetCode < 0)
                {
                    // 加入错误统计
                    if (poSession->m_bDbProxyNodeExist)
                    {
                        CDownMgr::Instance()->zk_AddError(poSession->m_pstDbProxyNode);
                    }

                    TSE_LOG_ERROR(m_poServLog, ("db res: [ip=%s] [port=%u] failed[%d] [seq=%u]", 
                        pszIp, \
                        uwPort, \
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
                    TSE_LOG_DEBUG(m_poServLog, ("db res: service[%u], ret[%d], table[%s], buf_len[%u] [seq=%u]", 
                        pDbRsp->udwServiceType, \
                        pDbRsp->dwRetCode, \
                        pDbRsp->sTableName.c_str(), \
                        pDbRsp->sRspContent.size(), \
                        poSession->m_udwSeqNo));
                }
            }
        }
    }

    poSession->m_udwDownRqstType = 0;

    return 0;
}


TINT32 CSearchNetIO::OnTranslateResponse(LTasksGroup *pstTasksGrp, SSession *poSession)
{
    LTask               *pstTask = 0;
    TCHAR               *pszIp = 0;
    TUINT16             uwPort = 0;
    TUINT32				udwIdx = 0;
    TINT32				dwRetCode = 0;

    poSession->ResetTranslateInfo();
    vector<TranslateRspInfo*>& vecRsp = poSession->m_vecTranslateRsp;
    TranslateRspInfo* pTranslateRsp = NULL;

    TSE_LOG_DEBUG(m_poServLog, ("translate res: OnTranslateResponse: [vaildtasksnum=%u] [seq=%u]", \
                                pstTasksGrp->m_uValidTasks, \
                                poSession->m_udwSeqNo));

    for(udwIdx = 0; udwIdx < pstTasksGrp->m_uValidTasks; udwIdx++)
    {
        pstTask = &pstTasksGrp->m_Tasks[udwIdx];

        GetIp2PortByHandle(pstTask->hSession, &uwPort, &pszIp);

        pTranslateRsp = new TranslateRspInfo;
        vecRsp.push_back(pTranslateRsp);

        TSE_LOG_DEBUG(m_poServLog, ("translate res: taskid[%u]: [ip=%s] [port=%u] [send=%u], [recv=%u], [timeout=%u], [donw_busy=%u], [socket_closed=%u], [verify_failed=%u] [recv_data_len=%u] [cost_time=%llu]us [req_type=%u] [seq=%u]",
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

        if(0 == pstTask->_ucIsSendOK || 0 == pstTask->_ucIsReceiveOK || 1 == pstTask->_ucTimeOutEvent)
        {
            // 加入超时统计
            if(poSession->m_bTranslateProxyExist)
            {
                CDownMgr::Instance()->zk_AddTimeOut(poSession->m_pstTranslateProxyNode);
            }
            TSE_LOG_ERROR(m_poServLog, ("translate res: [ip=%s] [port=%u] [send=%u], [recv=%u], [timeout=%u], [cost_time=%llu]us [seq=%u]",
                pszIp, \
                uwPort, \
                pstTask->_ucIsSendOK, \
                pstTask->_ucIsReceiveOK, \
                pstTask->_ucTimeOutEvent, \
                poSession->m_uddwDownRqstTimeEnd - poSession->m_uddwDownRqstTimeBeg, \
                poSession->m_udwSeqNo));
            if(EN_RET_CODE__SUCCESS == poSession->m_stCommonResInfo.m_dwRetCode)
            {
                // poSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__TIMEOUT;
                dwRetCode = -1;
            }
            break;
        }
        else
        {
            if(pstTask->_uReceivedDataLen > MAX_AWS_RES_DATA_LEN)
            {
                TSE_LOG_ERROR(m_poServLog, ("translate res: [ip=%s] [port=%u] recv_data_len[%u]>MAX_HS_RES_DATA_LEN [cost_time=%llu]us [seq=%u]",
                    pszIp, \
                    uwPort, \
                    pstTask->_uReceivedDataLen, \
                    poSession->m_uddwDownRqstTimeEnd - poSession->m_uddwDownRqstTimeBeg, \
                    poSession->m_udwSeqNo));
                if(EN_RET_CODE__SUCCESS == poSession->m_stCommonResInfo.m_dwRetCode)
                {
                    // poSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PACKAGE_LEN_OVERFLOW;
                    dwRetCode = -2;
                }
                break;
            }
            else
            {
                dwRetCode = ParseTranslateResponse(pstTask->_pReceivedData, pstTask->_uReceivedDataLen, pTranslateRsp);
                if(dwRetCode < 0)
                {
                    // 加入错误统计
                    if(poSession->m_bTranslateProxyExist)
                    {
                        CDownMgr::Instance()->zk_AddError(poSession->m_pstTranslateProxyNode);
                    }

                    TSE_LOG_ERROR(m_poServLog, ("translate res: [ip=%s] [port=%u] failed[%d] [seq=%u]", 
                        pszIp, \
                        uwPort, \
                        dwRetCode, \
                        poSession->m_udwSeqNo));
                    if(EN_RET_CODE__SUCCESS == poSession->m_stCommonResInfo.m_dwRetCode)
                    {
                        // poSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_CACHE_PACKAGE_ERR;
                        dwRetCode = -3;
                    }
                    break;
                }
                else
                {
                    TSE_LOG_DEBUG(m_poServLog, ("translate res: [seq=%u]", poSession->m_udwSeqNo));
                }
            }
        }
    }

    poSession->m_udwDownRqstType = 0;
    return 0;

}


TINT32 CSearchNetIO::OnDataCenterResponse(LTasksGroup *pstTasksGrp, SSession *poSession)
{
    LTask               *pstTask = 0;
    TCHAR               *pszIp = 0;
    TUINT16             uwPort = 0;
    TUINT32				udwIdx = 0;
    TINT32				dwRetCode = 0;

    poSession->ResetDataCenterInfo();
    vector<DataCenterRspInfo*>& vecRsp = poSession->m_vecDataCenterRsp;
    DataCenterRspInfo* pDataCenterRsp = NULL;

    TSE_LOG_DEBUG(m_poServLog, ("data center res: OnDataCenterResponse: [vaildtasksnum=%u] [seq=%u]", 
        pstTasksGrp->m_uValidTasks, \
        poSession->m_udwSeqNo));

    for (udwIdx = 0; udwIdx < pstTasksGrp->m_uValidTasks; udwIdx++)
    {
        pstTask = &pstTasksGrp->m_Tasks[udwIdx];

        GetIp2PortByHandle(pstTask->hSession, &uwPort, &pszIp);

        pDataCenterRsp = new DataCenterRspInfo;
        vecRsp.push_back(pDataCenterRsp);

        TSE_LOG_DEBUG(m_poServLog, ("data center res: taskid[%u]: [ip=%s] [port=%u] [send=%u], [recv=%u], [timeout=%u], [donw_busy=%u], [socket_closed=%u], [verify_failed=%u] [recv_data_len=%u] [cost_time=%llu]us [req_type=%u] [seq=%u]",
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
            if (poSession->m_bDataCenterProxyExist)
            {
                CDownMgr::Instance()->zk_AddTimeOut(poSession->m_pstDataCenterProxyNode);
            }
            TSE_LOG_ERROR(m_poServLog, ("data center res: [ip=%s] [port=%u] [send=%u], [recv=%u], [timeout=%u], [cost_time=%llu]us [seq=%u]",
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
            break;
        }
        else
        {
            if (pstTask->_uReceivedDataLen > MAX_AWS_RES_DATA_LEN)
            {
                TSE_LOG_ERROR(m_poServLog, ("data center res: [ip=%s] [port=%u] recv_data_len[%u]>MAX_HS_RES_DATA_LEN [cost_time=%llu]us [seq=%u]",
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
                break;
            }
            else
            {
                dwRetCode = ParseDataCenterResponse(pstTask->_pReceivedData, pstTask->_uReceivedDataLen, pDataCenterRsp);
                if (dwRetCode < 0)
                {
                    // 加入错误统计
                    if (poSession->m_bDataCenterProxyExist)
                    {
                        CDownMgr::Instance()->zk_AddError(poSession->m_pstDataCenterProxyNode);
                    }

                    TSE_LOG_ERROR(m_poServLog, ("data center res: [ip=%s] [port=%u] failed[%d] [seq=%u]", 
                        pszIp, \
                        uwPort, \
                        dwRetCode, \
                        poSession->m_udwSeqNo));
                    if (EN_RET_CODE__SUCCESS == poSession->m_stCommonResInfo.m_dwRetCode)
                    {
                        poSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_DATA_CENTER_PACKAGE_ERR;
                        dwRetCode = -3;
                    }
                    break;
                }
                else
                {
                    TSE_LOG_DEBUG(m_poServLog, ("data center res: [seq=%u]", 
                        poSession->m_udwSeqNo));
                }
            }
        }
    }
    poSession->m_udwDownRqstType = 0;
    return 0;
}

TINT32 CSearchNetIO::OnIapSvrResponse(LTasksGroup *pstTasksGrp, SSession *poSession)
{
    LTask               *pstTask = 0;
    TCHAR               *pszIp = 0;
    TUINT16             uwPort = 0;
    TUINT32				udwIdx = 0;
    TINT32				dwRetCode = 0;

    poSession->ResetIapSvrInfo();
    vector<IapSvrRspInfo*>& vecRsp = poSession->m_vecIapSvrRsp;
    //DataCenterRspInfo* pDataCenterRsp = NULL;
    IapSvrRspInfo* pIapSvrRsp = NULL;

    TSE_LOG_DEBUG(m_poServLog, ("iap server res: OnDataCenterResponse: [vaildtasksnum=%u] [seq=%u]", 
        pstTasksGrp->m_uValidTasks, \
        poSession->m_udwSeqNo));

    for (udwIdx = 0; udwIdx < pstTasksGrp->m_uValidTasks; udwIdx++)
    {
        pstTask = &pstTasksGrp->m_Tasks[udwIdx];

        GetIp2PortByHandle(pstTask->hSession, &uwPort, &pszIp);

        pIapSvrRsp = new IapSvrRspInfo;
        vecRsp.push_back(pIapSvrRsp);

        TSE_LOG_DEBUG(m_poServLog, ("iap server res: taskid[%u]: [ip=%s] [port=%u] [send=%u], [recv=%u], [timeout=%u], [donw_busy=%u], [socket_closed=%u], [verify_failed=%u] [recv_data_len=%u] [cost_time=%llu]us [req_type=%u] [seq=%u]",
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
            if (poSession->m_bIapSvrExist)
            {
                CDownMgr::Instance()->zk_AddTimeOut(poSession->m_pstIapSvrNode);
            }
            TSE_LOG_ERROR(m_poServLog, ("iap server res: [ip=%s] [port=%u] [send=%u], [recv=%u], [timeout=%u], [cost_time=%llu]us [seq=%u]",
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
            break;
        }
        else
        {
            if (pstTask->_uReceivedDataLen > MAX_AWS_RES_DATA_LEN)
            {
                TSE_LOG_ERROR(m_poServLog, ("iap server res: [ip=%s] [port=%u] recv_data_len[%u]>MAX_HS_RES_DATA_LEN [cost_time=%llu]us [seq=%u]",
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
                break;
            }
            else
            {
                dwRetCode = ParseIapSvrResponse(pstTask->_pReceivedData, pstTask->_uReceivedDataLen, pIapSvrRsp);
                if (dwRetCode < 0)
                {
                    // 加入错误统计
                    if (poSession->m_bIapSvrExist)
                    {
                        CDownMgr::Instance()->zk_AddError(poSession->m_pstIapSvrNode);
                    }

                    TSE_LOG_ERROR(m_poServLog, ("iap server res: [ip=%s] [port=%u] failed[%d] [seq=%u]", 
                        pszIp, \
                        uwPort, \
                        dwRetCode, \
                        poSession->m_udwSeqNo));
                    if (EN_RET_CODE__SUCCESS == poSession->m_stCommonResInfo.m_dwRetCode)
                    {
                        poSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_IAP_SERVER_REQ_ERR;
                        dwRetCode = -3;
                    }
                    break;
                }
                else
                {
                    TSE_LOG_DEBUG(m_poServLog, ("iap server res: [seq=%u]", 
                        poSession->m_udwSeqNo));
                }
            }
        }
    }
    poSession->m_udwDownRqstType = 0;
    return 0;
}

TINT32 CSearchNetIO::ParseTranslateResponse(TUCHAR *pszPack, TUINT32 udwPackLen, TranslateRspInfo* pTranslateRspInfo)
{
    TUCHAR *pszValBuf = NULL;
    TUINT32 udwValBufLen = 0;

    m_pUnPackTool->UntachPackage();
    m_pUnPackTool->AttachPackage(pszPack, udwPackLen);
    if(FALSE == m_pUnPackTool->Unpack())
    {
        return -1;
    }

    pTranslateRspInfo->m_udwServiceType = m_pUnPackTool->GetServiceType();

    m_pUnPackTool->GetVal(EN_TRANSLATE__RES_CODE, &pTranslateRspInfo->m_dwRetCode);


    //2XX的返回码,都是正常的情况
    if(pTranslateRspInfo->m_dwRetCode != 0)
    {
        TSE_LOG_ERROR(m_poServLog, ("cache res: pTranslateRspInfo->m_dwRetCode=%d\n", pTranslateRspInfo->m_dwRetCode));
        return -2;
    }

    if(pTranslateRspInfo->m_udwServiceType == EN_SERVICE_TYPE_TRANSLATE_RSP)
    {
      if(m_pUnPackTool->GetVal(EN_TRANSLATE__TYPE, &pszValBuf, &udwValBufLen))
      {
          pTranslateRspInfo->m_strTranslateType.resize(udwValBufLen);
          memcpy((char*)pTranslateRspInfo->m_strTranslateType.c_str(), pszValBuf, udwValBufLen);
      }
      
      if(m_pUnPackTool->GetVal(EN_TRANSLATE__OPERATE, &pszValBuf, &udwValBufLen))
      {
          pTranslateRspInfo->m_strTranslateOperate.resize(udwValBufLen);
          memcpy((char*)pTranslateRspInfo->m_strTranslateOperate.c_str(), pszValBuf, udwValBufLen);
      }
        

      if(m_pUnPackTool->GetVal(EN_TRANSLATE__CONTENT, &pszValBuf, &udwValBufLen))
      {
          pTranslateRspInfo->m_strTranslateContent.resize(udwValBufLen);
          memcpy((char*)pTranslateRspInfo->m_strTranslateContent.c_str(), pszValBuf, udwValBufLen);
      }    
      
      if(m_pUnPackTool->GetVal(EN_TRANSLATE__RESULT, &pszValBuf, &udwValBufLen))
      {
          pTranslateRspInfo->m_strTranslateResult.resize(udwValBufLen);
          memcpy((char*)pTranslateRspInfo->m_strTranslateResult.c_str(), pszValBuf, udwValBufLen);
      }
    }
    else
    {
      return -3;
    }

    return 0;    
}



TINT32 CSearchNetIO::ParseDataCenterResponse(TUCHAR *pszPack, TUINT32 udwPackLen, DataCenterRspInfo* pDataCenterRspInfo)
{
    TUCHAR *pszValBuf = NULL;
    TUINT32 udwValBufLen = 0;

    m_pUnPackTool->UntachPackage();
    m_pUnPackTool->AttachPackage(pszPack, udwPackLen);
    if (FALSE == m_pUnPackTool->Unpack())
    {
        return -1;
    }

    pDataCenterRspInfo->m_udwServiceType = m_pUnPackTool->GetServiceType();

    m_pUnPackTool->GetVal(EN_DATA_CENTER__RSP_RET_CODE, &pDataCenterRspInfo->m_dwRetCode);
    m_pUnPackTool->GetVal(EN_DATA_CENTER__RSP_TYPE, &pDataCenterRspInfo->m_udwType);
    
    if (pDataCenterRspInfo->m_dwRetCode != 0)
    {
        TSE_LOG_ERROR(m_poServLog, ("data center res: pDataCenterRspInfo->m_dwRetCode=%d\n", pDataCenterRspInfo->m_dwRetCode));
        return -2;
    }

    if (pDataCenterRspInfo->m_udwServiceType == EN_SERVICE_TYPE_DATA_CENTER_RSP)
    {
        ///CHARLES:添加返包解析
        if (m_pUnPackTool->GetVal(EN_DATA_CENTER__RSP_JSON, &pszValBuf, &udwValBufLen))
        {
            pDataCenterRspInfo->m_sRspJson.resize(udwValBufLen);
            memcpy((char*)pDataCenterRspInfo->m_sRspJson.c_str(), pszValBuf, udwValBufLen);
            TSE_LOG_DEBUG(m_poServLog, ("data center res: [rsp_json=%s]", pDataCenterRspInfo->m_sRspJson.c_str()));
        }
    }
    else
    {
        return -3;
    }

    return 0;
}

TINT32 CSearchNetIO::ParseIapSvrResponse(TUCHAR *pszPack, TUINT32 udwPackLen, IapSvrRspInfo* pIapSvrRspInfo)
{
    TUCHAR *pszValBuf = NULL;
    TUINT32 udwValBufLen = 0;

    m_pUnPackTool->UntachPackage();
    m_pUnPackTool->AttachPackage(pszPack, udwPackLen);
    if (FALSE == m_pUnPackTool->Unpack())
    {
        return -1;
    }

    //pDataCenterRspInfo->m_udwServiceType = m_pUnPackTool->GetServiceType();
    //pIapSvrRspInfo->m_udwServiceType = m_pUnPackTool->GetServiceType();

    m_pUnPackTool->GetVal(EN_HU_RESPONSE_KEY__RET_CODE, &pIapSvrRspInfo->m_dwRetCode);
    m_pUnPackTool->GetVal(EN_HU_RESPONSE_KEY__REQUEST_TYPE, &pIapSvrRspInfo->m_udwType);

    if (pIapSvrRspInfo->m_dwRetCode != 0)
    {
        TSE_LOG_ERROR(m_poServLog, ("iap server res: pIapSvrRspInfo->m_dwRetCode=%d\n", pIapSvrRspInfo->m_dwRetCode));
        return -2;
    }


    ///CHARLES:添加返包解析
    if (m_pUnPackTool->GetVal(EN_HU_RESPONSE_KEY__JSON, &pszValBuf, &udwValBufLen))
    {
        pIapSvrRspInfo->m_sRspJson.resize(udwValBufLen);
        memcpy((char*)pIapSvrRspInfo->m_sRspJson.c_str(), pszValBuf, udwValBufLen);
        TSE_LOG_DEBUG(m_poServLog, ("iap server res: [rsp_json=%s]", pIapSvrRspInfo->m_sRspJson.c_str()));
    }

    return 0;
}

TINT32 CSearchNetIO::OnCacheResponse(LTasksGroup *pstTasksGrp, SSession *poSession)
{
    LTask               *pstTask = 0;
    TCHAR               *pszIp = 0;
    TUINT16             uwPort = 0;
    TUINT32				udwIdx = 0;
    TINT32				dwRetCode = 0;

    poSession->ResetCacheInfo();
    vector<CacheRspInfo*>& vecRsp = poSession->m_vecCacheRsp;
    CacheRspInfo* pCacheRsp = NULL;

    TSE_LOG_DEBUG(m_poServLog, ("cache res: ParseCacheResponse: [vaildtasksnum=%u] [seq=%u]", 
                                pstTasksGrp->m_uValidTasks, \
                                poSession->m_udwSeqNo));

    for(udwIdx = 0; udwIdx < pstTasksGrp->m_uValidTasks; udwIdx++)
    {
        pstTask = &pstTasksGrp->m_Tasks[udwIdx];

        GetIp2PortByHandle(pstTask->hSession, &uwPort, &pszIp);

        pCacheRsp = new CacheRspInfo;
        vecRsp.push_back(pCacheRsp);

        TSE_LOG_DEBUG(m_poServLog, ("cache res: taskid[%u]: [ip=%s] [port=%u] [send=%u], [recv=%u], [timeout=%u], [donw_busy=%u], [socket_closed=%u], [verify_failed=%u] [recv_data_len=%u] [cost_time=%llu]us [req_type=%u] [seq=%u]",
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

        if(0 == pstTask->_ucIsSendOK || 0 == pstTask->_ucIsReceiveOK || 1 == pstTask->_ucTimeOutEvent)
        {
            // 加入超时统计
            if(poSession->m_bCacheProxyExist)
            {
                CDownMgr::Instance()->zk_AddTimeOut(poSession->m_pstCacheProxyNode);
            }
            TSE_LOG_ERROR(m_poServLog, ("cache res: [ip=%s] [port=%u] [send=%u], [recv=%u], [timeout=%u], [cost_time=%llu]us [seq=%u]",
                pszIp, \
                uwPort, \
                pstTask->_ucIsSendOK, \
                pstTask->_ucIsReceiveOK, \
                pstTask->_ucTimeOutEvent, \
                poSession->m_uddwDownRqstTimeEnd - poSession->m_uddwDownRqstTimeBeg, \
                poSession->m_udwSeqNo));
            if(EN_RET_CODE__SUCCESS == poSession->m_stCommonResInfo.m_dwRetCode)
            {
                // poSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__TIMEOUT;
                dwRetCode = -1;
            }
            break;
        }
        else
        {
            if(pstTask->_uReceivedDataLen > MAX_AWS_RES_DATA_LEN)
            {
                TSE_LOG_ERROR(m_poServLog, ("cache res: [ip=%s] [port=%u] recv_data_len[%u]>MAX_HS_RES_DATA_LEN [cost_time=%llu]us [seq=%u]",
                    pszIp, \
                    uwPort, \
                    pstTask->_uReceivedDataLen, \
                    poSession->m_uddwDownRqstTimeEnd - poSession->m_uddwDownRqstTimeBeg, \
                    poSession->m_udwSeqNo));
                if(EN_RET_CODE__SUCCESS == poSession->m_stCommonResInfo.m_dwRetCode)
                {
                    // poSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PACKAGE_LEN_OVERFLOW;
                    dwRetCode = -2;
                }
                break;
            }
            else
            {
                dwRetCode = ParseCacheResponse(pstTask->_pReceivedData, pstTask->_uReceivedDataLen, pCacheRsp);
                if(dwRetCode < 0)
                {
                    // 加入错误统计
                    if(poSession->m_bCacheProxyExist)
                    {
                        CDownMgr::Instance()->zk_AddError(poSession->m_pstCacheProxyNode);
                    }

                    TSE_LOG_ERROR(m_poServLog, ("cache res: [ip=%s] [port=%u] failed[%d] [seq=%u]", 
                        pszIp, \
                        uwPort, \
                        dwRetCode, \
                        poSession->m_udwSeqNo));
                    if(EN_RET_CODE__SUCCESS == poSession->m_stCommonResInfo.m_dwRetCode)
                    {
                        // poSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_CACHE_PACKAGE_ERR;
                        dwRetCode = -3;
                    }
                    break;
                }
                else
                {
                    TSE_LOG_DEBUG(m_poServLog, ("cache res: [seq=%u]", poSession->m_udwSeqNo));
                }
            }
        }
    }

    poSession->m_udwDownRqstType = 0;
    return 0;
}


TINT32 CSearchNetIO::ParseCacheResponse(TUCHAR *pszPack, TUINT32 udwPackLen, CacheRspInfo* pCacheRspInfo)
{
    TUCHAR *pszValBuf = NULL;
    TUINT32 udwValBufLen = 0;

    m_pUnPackTool->UntachPackage();
    m_pUnPackTool->AttachPackage(pszPack, udwPackLen);
    if(FALSE == m_pUnPackTool->Unpack())
    {
        return -1;
    }

    pCacheRspInfo->udwServiceType = m_pUnPackTool->GetServiceType();
    
    m_pUnPackTool->GetVal(EN_CACHE__RES_CODE, &pCacheRspInfo->dwRetCode);


    //2XX的返回码,都是正常的情况
    if(pCacheRspInfo->dwRetCode  != 0)
    {
        TSE_LOG_ERROR(m_poServLog, ("cache res: pCacheRspInfo->dwRetCode=%d\n", pCacheRspInfo->dwRetCode));
        return -2;
    }

    if(pCacheRspInfo->udwServiceType == EN_SERVICE_TYPE_CACHE_RSP)
    {
        if(m_pUnPackTool->GetVal(EN_CACHE__NAME, &pszValBuf, &udwValBufLen))
        {
            pCacheRspInfo->m_strCacheName.resize(udwValBufLen);
            memcpy((char*)pCacheRspInfo->m_strCacheName.c_str(), pszValBuf, udwValBufLen);
        }
        
        if(m_pUnPackTool->GetVal(EN_CACHE__OPERATE, &pszValBuf, &udwValBufLen))
        {
            pCacheRspInfo->m_strCacheOperate.resize(udwValBufLen);
            memcpy((char*)pCacheRspInfo->m_strCacheOperate.c_str(), pszValBuf, udwValBufLen);
        }
        
        m_pUnPackTool->GetVal(EN_CACHE__KEY, &pCacheRspInfo->m_uddwCacheKey);

        m_pUnPackTool->GetVal(EN_CACHE__KEY_EXIST, &pCacheRspInfo->m_bExist);
        
        if(m_pUnPackTool->GetVal(EN_CACHE__VALUE_DATA, &pszValBuf, &udwValBufLen))
        {
            pCacheRspInfo->m_strCacheValue.resize(udwValBufLen);
            memcpy((char*)pCacheRspInfo->m_strCacheValue.c_str(), pszValBuf, udwValBufLen);
        }
    }
    else
    {
        return -3;
    }

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

    for(udwIdx = 0; udwIdx < pstTasksGrp->m_uValidTasks; udwIdx++)
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

        if(0 == pstTask->_ucIsSendOK || 0 == pstTask->_ucIsReceiveOK || 1 == pstTask->_ucTimeOutEvent)
        {
            // 加入超时统计
            if(poSession->m_bEventProxyExist && pstTask->hSession == poSession->m_pstEventProxyNode->m_stDownHandle)
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
                TSE_LOG_ERROR(m_poServLog, ("event res: [ip=%s] [port=%u] recv_data_len[%u]>MAX_HS_RES_DATA_LEN [cost_time=%llu]us [seq=%u]",
                    pszIp, \
                    uwPort, \
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
                dwRetCode = ParseEventResponse(pstTask->_pReceivedData, pstTask->_uReceivedDataLen, pEventRsp);
                if(dwRetCode < 0)
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
                    if(EN_RET_CODE__SUCCESS == poSession->m_stCommonResInfo.m_dwRetCode)
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
    if(FALSE == m_pUnPackTool->Unpack())
    {
        return -1;
    }
    pEventRspInfo->udwServiceType = m_pUnPackTool->GetServiceType();
    m_pUnPackTool->GetVal(EN_GLOBAL_KEY__RES_CODE, &pEventRspInfo->dwRetCode);
    m_pUnPackTool->GetVal(EN_GLOBAL_KEY__RES_COST_TIME, &pEventRspInfo->udwCostTime);
    m_pUnPackTool->GetVal(EN_GLOBAL_KEY__INDEX_NO, &pEventRspInfo->udwIdxNo);
    m_pUnPackTool->GetVal(EN_KEY_EVENT_PROXY__REQ_TYPE, &pEventRspInfo->udwReqType);

    //2XX的返回码,都是正常的情况
    if(pEventRspInfo->dwRetCode  != 0)
    {
        TSE_LOG_ERROR(m_poServLog, ("event res: pAwsRspInfo->dwRetCode=%d\n", pEventRspInfo->dwRetCode));
        return -2;
    }

    if(pEventRspInfo->udwServiceType == EN_SERVICE_TYPE_QUERY_EVENT_RSP)
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
    if (!m_pUnPackTool->GetVal(EN_GLOBAL_KEY__RES_TYPE, &pAwsRspInfo->udwResType))
    {
        pAwsRspInfo->udwResType = EN_CONTENT_TYPE__STRING;
    }
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

TINT32	CSearchNetIO::ParseDbResponse(TUCHAR *pszPack, TUINT32 udwPackLen,  DbRspInfo* pDbRspInfo)
{
    TUCHAR *pszValBuf = NULL;
    TUINT32 udwValBufLen = 0;

    m_pUnPackTool->UntachPackage();
    m_pUnPackTool->AttachPackage(pszPack, udwPackLen);
    if(FALSE == m_pUnPackTool->Unpack())
    {
        TSE_LOG_ERROR(m_poServLog, ("ParseDbResponse: Unpack failed. [%s]", pszPack));
        return -1;
    }
    pDbRspInfo->udwServiceType = m_pUnPackTool->GetServiceType();
    m_pUnPackTool->GetVal(EN_GLOBAL_KEY__RES_CODE, &pDbRspInfo->dwRetCode);
    m_pUnPackTool->GetVal(EN_GLOBAL_KEY__RES_COST_TIME, &pDbRspInfo->udwCostTime);
    m_pUnPackTool->GetVal(EN_GLOBAL_KEY__INDEX_NO, &pDbRspInfo->udwIdxNo);
    if (m_pUnPackTool->GetVal(EN_GLOBAL_KEY__TABLE_NAME, &pszValBuf, &udwValBufLen))
    {
        pDbRspInfo->sTableName.resize(udwValBufLen);
        memcpy((char*)pDbRspInfo->sTableName.c_str(), pszValBuf, udwValBufLen);
    }
    if (m_pUnPackTool->GetVal(EN_GLOBAL_KEY__OPERATOR_NAME, &pszValBuf, &udwValBufLen))
    {
        pDbRspInfo->sOperatorName.resize(udwValBufLen);
        memcpy((char*)pDbRspInfo->sOperatorName.c_str(), pszValBuf, udwValBufLen);
    }

    if(pDbRspInfo->dwRetCode != 0)
    {
        TSE_LOG_ERROR(m_poServLog, ("pDbRspInfo->dwRetCode=%d\n", pDbRspInfo->dwRetCode));
        return -2;
    }

    if(pDbRspInfo->udwServiceType == EN_SERVICE_TYPE_QUERY_MYSQL_RSP)
    {
        m_pUnPackTool->GetVal(EN_GLOBAL_KEY__RES_BUF, &pszValBuf, &udwValBufLen);
        pDbRspInfo->sRspContent.resize(udwValBufLen);
        memcpy((char*)pDbRspInfo->sRspContent.c_str(), pszValBuf, udwValBufLen);
    }
    else
    {
        TSE_LOG_ERROR(m_poServLog, ("ParseDbResponse: ServiceType not SQL. [type=%u]", pDbRspInfo->udwServiceType));
        return -3;
    }

    return 0;	
}

TINT32 CSearchNetIO::OnLockResponse( LTasksGroup *pstTasksGrp, SSession *poSession )
{
    // do nothing
    LTask               *pstTask                = 0;
    TCHAR               *pszIp                  = 0;
    TUINT16             uwPort                  = 0;
    TUINT32             udwIdx                  = 0;


    poSession->m_uddwDownRqstTimeEnd = CTimeUtils::GetCurTimeUs();

    poSession->m_uddwLockSumTime += poSession->m_uddwDownRqstTimeEnd - poSession->m_uddwDownRqstTimeBeg;

    // lock请求的时候不会有多个task,所以response也就只有一个
    for(udwIdx = 0; udwIdx < pstTasksGrp->m_uValidTasks && udwIdx < 1; udwIdx++)
    {
        pstTask = &pstTasksGrp->m_Tasks[udwIdx];

        /**************************************************************************************/ 
        // 获取原始请求的信息,方便在异常的时候知道是哪条请求出问题,能更好的获取数据层面的信息
        /*
        TUINT32 udwOriginLockNum = 0;         
        TUINT64 uddwOriginTimeOut = 0;
        TUINT64 auddwOriginKey[MAX_LOCK_KEY_NUM_IN_ONE_REQ];
        TUCHAR *pszOriginValBuf = NULL;
        TUINT32 udwOriginValBufLen = 0;
        m_pUnPackTool->UntachPackage();
        m_pUnPackTool->AttachPackage(pstTask->pData, pstTask->uValidDataLen);
        if(FALSE == m_pUnPackTool->Unpack())
        {
            TSE_LOG_DEBUG(m_poServLog, ("lock res: taskid[%u]: origin lock req unpack error [seq=%u]", \
                                        udwIdx, \
                                        poSession->m_udwSeqNo));
        }
        else
        {     
            m_pUnPackTool->GetVal(EN_KEY_HU2LOCK__REQ_KEY_NUM, &udwOriginLockNum);           
            m_pUnPackTool->GetVal(EN_KEY_HU2LOCK__REQ_TIMEOUT_US, &uddwOriginTimeOut);
            m_pUnPackTool->GetVal(EN_KEY_HU2LOCK__REQ_KEY_LIST, &pszOriginValBuf, &udwOriginValBufLen);
            memcpy((char*)&auddwOriginKey, pszOriginValBuf, udwOriginLockNum * sizeof(TUINT64));
        }
        */
        /**************************************************************************************/ 
         
		GetIp2PortByHandle(pstTask->hSession, &uwPort, &pszIp);

		TSE_LOG_DEBUG(m_poServLog, ("lock res: taskid[%u]: [ip=%s] [port=%u] [send=%u], [recv=%u], [timeout=%u], [donw_busy=%u], [socket_closed=%u], [verify_failed=%u] [recv_data_len=%u] [cost_time=%llu]us [seq=%u]",
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
            poSession->m_udwSeqNo));

		if(0 == pstTask->_ucIsSendOK || 0 == pstTask->_ucIsReceiveOK || 1 == pstTask->_ucTimeOutEvent )
		{
			// 加入超时统计
			if(poSession->m_bLockSvrExist)
			{
				CDownMgr::Instance()->zk_AddTimeOut(poSession->m_pstLockSvr);
			}
			TSE_LOG_ERROR(m_poServLog, ("lock res: [ip=%s] [port=%u] [send=%u], [recv=%u], [timeout=%u], [cost_time=%llu]us [seq=%u]",
                pszIp, \
                uwPort, \
                pstTask->_ucIsSendOK, \
                pstTask->_ucIsReceiveOK, \
                pstTask->_ucTimeOutEvent, \
                poSession->m_uddwDownRqstTimeEnd - poSession->m_uddwDownRqstTimeBeg, \
                poSession->m_udwSeqNo));
			poSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__TIMEOUT;
			
			break;
		}
		else
		{
			m_pUnPackTool->UntachPackage();
			m_pUnPackTool->AttachPackage(pstTask->_pReceivedData, pstTask->_uReceivedDataLen);
			if(FALSE == m_pUnPackTool->Unpack())
			{
                TSE_LOG_ERROR(m_poServLog, ("lock res: [ip=%s] [port=%u] unpack error [seq=%u]", 
                    pszIp, \
                    uwPort, \
                    poSession->m_udwSeqNo));
            
				poSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_PACKAGE_ERR;
				return -2;
			}
			m_pUnPackTool->GetVal(EN_GLOBAL_KEY__RES_CODE, &poSession->m_dwLockRetCode);

			if(poSession->m_dwLockRetCode != EN_RET_CODE__SUCCESS)
			{
                TSE_LOG_ERROR(m_poServLog, ("lock res: [ip=%s] [port=%u] lock proxy ret err [ret=%d] [seq=%u]", 
                    pszIp, \
                    uwPort, \
                    poSession->m_dwLockRetCode, \
                    poSession->m_udwSeqNo));
            
                poSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_PACKAGE_ERR;
				return -3;
			}

            TSE_LOG_DEBUG(m_poServLog, ("lock res: [ret=%d] [seq=%u]", 
                poSession->m_dwLockRetCode, \
                poSession->m_udwSeqNo));
		}
	}
	return 0;
}

TINT32 CSearchNetIO::OnMapSvrResponse(LTasksGroup *pstTasksGrp, SSession *poSession)
{
    LTask               *pstTask = 0;
    TCHAR               *pszIp = 0;
    TUINT16             uwPort = 0;
    TUINT32				udwIdx = 0;
    TINT32				dwRetCode = 0;

    poSession->m_uddwDownRqstTimeEnd = CTimeUtils::GetCurTimeUs();

    poSession->ResetMapSvrInfo();
    vector<AwsRspInfo*>& vecRsp = poSession->m_vecAwsRsp;
    AwsRspInfo* pAwsRsp = NULL;

    TSE_LOG_DEBUG(m_poServLog, ("OnMapSvrResponse: [vaildtasksnum=%u] [seq=%u]", 
        pstTasksGrp->m_uValidTasks,
        poSession->m_udwSeqNo));

    for (udwIdx = 0; udwIdx < pstTasksGrp->m_uValidTasks; udwIdx++)
    {
        pstTask = &pstTasksGrp->m_Tasks[udwIdx];

        GetIp2PortByHandle(pstTask->hSession, &uwPort, &pszIp);

        pAwsRsp = new AwsRspInfo;
        vecRsp.push_back(pAwsRsp);

        TSE_LOG_DEBUG(m_poServLog, ("map svr res: taskid[%u]: [ip=%s] [port=%u] [send=%u], [recv=%u], [timeout=%u], [donw_busy=%u], [socket_closed=%u], [verify_failed=%u] [recv_data_len=%u] [cost_time=%llu]us [req_type=%u] [seq=%u]",
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

        if (0 == pstTask->_ucIsSendOK || 0 == pstTask->_ucIsReceiveOK || 1 == pstTask->_ucTimeOutEvent)
        {
            // 加入超时统计
            if (poSession->m_bMapSvrExist)
            {
                CDownMgr::Instance()->zk_AddTimeOut(poSession->m_pstMapSvrNode);
            }
            TSE_LOG_ERROR(m_poServLog, ("map svr res: [ip=%s] [port=%u] [send=%u], [recv=%u], [timeout=%u], [cost_time=%llu]us [seq=%u]",
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

                TSE_LOG_ERROR(m_poServLog, ("map svr res: [ip=%s] [port=%u] recv_data_len[%u]>MAX_HS_RES_DATA_LEN [cost_time=%llu]us [seq=%u]",
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
                dwRetCode = ParseMapSvrResponse(pstTask->_pReceivedData, pstTask->_uReceivedDataLen, pAwsRsp);
                if (dwRetCode < 0)
                {
                    // 加入错误统计
                    if (poSession->m_bMapSvrExist)
                    {
                        CDownMgr::Instance()->zk_AddError(poSession->m_pstMapSvrNode);
                    }

                    TSE_LOG_ERROR(m_poServLog, ("map svr res: [ip=%s] [port=%u] failed[%d] [seq=%u]", 
                        pszIp, \
                        uwPort, \
                        dwRetCode, \
                        poSession->m_udwSeqNo));
                    if (EN_RET_CODE__SUCCESS == poSession->m_stCommonResInfo.m_dwRetCode)
                    {
                        poSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_PACKAGE_ERR;
                        dwRetCode = -3;
                    }
                    // break;
                }
                else
                {
                    TSE_LOG_DEBUG(m_poServLog, ("map svr res: service[%u], ret[%d], table[%s], buf_len[%u] [seq=%u]", 
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

TINT32 CSearchNetIO::ParseMapSvrResponse(TUCHAR *pszPack, TUINT32 udwPackLen, AwsRspInfo* pAwsRspInfo)
{
    TUCHAR *pszValBuf = NULL;
    TUINT32 udwValBufLen = 0;

    m_pUnPackTool->UntachPackage();
    m_pUnPackTool->AttachPackage(pszPack, udwPackLen);
    if (FALSE == m_pUnPackTool->Unpack())
    {
        return -1;
    }
    pAwsRspInfo->udwServiceType = m_pUnPackTool->GetServiceType();
    m_pUnPackTool->GetVal(EN_GLOBAL_KEY__RES_CODE, &pAwsRspInfo->dwRetCode);
    m_pUnPackTool->GetVal(EN_GLOBAL_KEY__RES_COST_TIME, &pAwsRspInfo->udwCostTime);
    if (!m_pUnPackTool->GetVal(EN_GLOBAL_KEY__RES_TYPE, &pAwsRspInfo->udwResType))
    {
        pAwsRspInfo->udwResType = EN_CONTENT_TYPE__STRING;
    }
    if (m_pUnPackTool->GetVal(EN_MAP_SVR_KEY__OPERATE, &pszValBuf, &udwValBufLen))
    {
        pAwsRspInfo->sOperatorName.resize(udwValBufLen);
        memcpy((char*)pAwsRspInfo->sOperatorName.c_str(), pszValBuf, udwValBufLen);
    }

    //2XX的返回码,都是正常的情况
    if (pAwsRspInfo->dwRetCode / 100 != 2)
    {
        TSE_LOG_ERROR(m_poServLog, ("aws res: pAwsRspInfo->dwRetCode=%d\n", pAwsRspInfo->dwRetCode));
        return -2;
    }

    if (pAwsRspInfo->udwServiceType == EN_SERVICE_TYPE_MAP_SVR_PROXY_RSP)
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

TINT32 CSearchNetIO::OnActionSvrResponse(LTasksGroup *pstTasksGrp, SSession *poSession)
{
    LTask               *pstTask = 0;
    TCHAR               *pszIp = 0;
    TUINT16             uwPort = 0;
    TUINT32				udwIdx = 0;
    TINT32				dwRetCode = 0;

    poSession->m_uddwDownRqstTimeEnd = CTimeUtils::GetCurTimeUs();

    poSession->ResetAwsRsp();
    vector<AwsRspInfo*>& vecRsp = poSession->m_vecAwsRsp;
    AwsRspInfo* pAwsRsp = NULL;

    TSE_LOG_DEBUG(m_poServLog, ("OnActionSvrResponse: [vaildtasksnum=%u] [seq=%u]",
        pstTasksGrp->m_uValidTasks,
        poSession->m_udwSeqNo));

    for (udwIdx = 0; udwIdx < pstTasksGrp->m_uValidTasks; udwIdx++)
    {
        pstTask = &pstTasksGrp->m_Tasks[udwIdx];

        GetIp2PortByHandle(pstTask->hSession, &uwPort, &pszIp);

        pAwsRsp = new AwsRspInfo;
        vecRsp.push_back(pAwsRsp);

        TSE_LOG_DEBUG(m_poServLog, ("action svr res: taskid[%u]: [ip=%s] [port=%u] [send=%u], [recv=%u], [timeout=%u], [donw_busy=%u], [socket_closed=%u], [verify_failed=%u] [recv_data_len=%u] [cost_time=%llu]us [req_type=%u] [seq=%u]",
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

        if (0 == pstTask->_ucIsSendOK || 0 == pstTask->_ucIsReceiveOK || 1 == pstTask->_ucTimeOutEvent)
        {
            // 加入超时统计
            if (poSession->m_bBuffActionSvrNodeExist && pstTask->hSession == poSession->m_pstBuffActionSvrNode->m_stDownHandle)
            {
                CDownMgr::Instance()->zk_AddTimeOut(poSession->m_pstBuffActionSvrNode);
            }
            else if (poSession->m_bAlActionSvrNodeExist && pstTask->hSession == poSession->m_pstAlActionSvrNode->m_stDownHandle)
            {
                CDownMgr::Instance()->zk_AddTimeOut(poSession->m_pstAlActionSvrNode);
            }
            else if (poSession->m_bMarchActionSvrNodeExist && pstTask->hSession == poSession->m_pstMarchActionSvrNode->m_stDownHandle)
            {
                CDownMgr::Instance()->zk_AddTimeOut(poSession->m_pstMarchActionSvrNode);
            }
            TSE_LOG_ERROR(m_poServLog, ("action svr res: [ip=%s] [port=%u] [send=%u], [recv=%u], [timeout=%u], [cost_time=%llu]us [seq=%u]",
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

                TSE_LOG_ERROR(m_poServLog, ("action svr res: [ip=%s] [port=%u] recv_data_len[%u]>MAX_HS_RES_DATA_LEN [cost_time=%llu]us [seq=%u]",
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
                dwRetCode = ParseActionSvrResponse(pstTask->_pReceivedData, pstTask->_uReceivedDataLen, pAwsRsp);
                if (dwRetCode < 0)
                {
                    // 加入错误统计
                    if (poSession->m_bBuffActionSvrNodeExist && pstTask->hSession == poSession->m_pstBuffActionSvrNode->m_stDownHandle)
                    {
                        CDownMgr::Instance()->zk_AddError(poSession->m_pstBuffActionSvrNode);
                    }
                    else if (poSession->m_bAlActionSvrNodeExist && pstTask->hSession == poSession->m_pstAlActionSvrNode->m_stDownHandle)
                    {
                        CDownMgr::Instance()->zk_AddError(poSession->m_pstAlActionSvrNode);
                    }
                    else if (poSession->m_bMarchActionSvrNodeExist && pstTask->hSession == poSession->m_pstMarchActionSvrNode->m_stDownHandle)
                    {
                        CDownMgr::Instance()->zk_AddError(poSession->m_pstMarchActionSvrNode);
                    }

                    TSE_LOG_ERROR(m_poServLog, ("action svr res: [ip=%s] [port=%u] failed[%d] [seq=%u]", 
                        pszIp, 
                        uwPort, 
                        dwRetCode,
                        poSession->m_udwSeqNo));
                    if (EN_RET_CODE__SUCCESS == poSession->m_stCommonResInfo.m_dwRetCode)
                    {
                        poSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_PACKAGE_ERR;
                        dwRetCode = -3;
                    }
                    // break;
                }
                else
                {
                    TSE_LOG_DEBUG(m_poServLog, ("action svr res: service[%u], ret[%d], table[%s], op[%s], buf_len[%u] [seq=%u]",
                        pAwsRsp->udwServiceType,
                        pAwsRsp->dwRetCode,
                        pAwsRsp->sTableName.c_str(),
                        pAwsRsp->sOperatorName.c_str(),
                        pAwsRsp->sRspContent.size(), 
                        poSession->m_udwSeqNo));
                }
            }
        }
    }

    poSession->m_udwDownRqstType = 0;

    return 0;
}

TINT32 CSearchNetIO::ParseActionSvrResponse(TUCHAR *pszPack, TUINT32 udwPackLen, AwsRspInfo* pAwsRspInfo)
{
    TUCHAR *pszValBuf = NULL;
    TUINT32 udwValBufLen = 0;

    m_pUnPackTool->UntachPackage();
    m_pUnPackTool->AttachPackage(pszPack, udwPackLen);
    if (FALSE == m_pUnPackTool->Unpack())
    {
        return -1;
    }
    pAwsRspInfo->udwServiceType = m_pUnPackTool->GetServiceType();
    m_pUnPackTool->GetVal(EN_GLOBAL_KEY__RES_CODE, &pAwsRspInfo->dwRetCode);
    m_pUnPackTool->GetVal(EN_GLOBAL_KEY__RES_COST_TIME, &pAwsRspInfo->udwCostTime);
    if (!m_pUnPackTool->GetVal(EN_GLOBAL_KEY__RES_TYPE, &pAwsRspInfo->udwResType))
    {
        pAwsRspInfo->udwResType = EN_CONTENT_TYPE__STRING;
    }
    if (m_pUnPackTool->GetVal(EN_ACTION_SVR_KEY__OPERATE, &pszValBuf, &udwValBufLen))
    {
        pAwsRspInfo->sOperatorName.resize(udwValBufLen);
        memcpy((char*)pAwsRspInfo->sOperatorName.c_str(), pszValBuf, udwValBufLen);
    }

    if (m_pUnPackTool->GetVal(EN_ACTION_SVR_KEY__TABLE_NAME, &pszValBuf, &udwValBufLen))
    {
        pAwsRspInfo->sTableName.resize(udwValBufLen);
        memcpy((char*)pAwsRspInfo->sTableName.c_str(), pszValBuf, udwValBufLen);
    }

    //2XX的返回码,都是正常的情况
    if (pAwsRspInfo->dwRetCode / 100 != 2)
    {
        TSE_LOG_ERROR(m_poServLog, ("action svr res: pAwsRspInfo->dwRetCode=%d\n", pAwsRspInfo->dwRetCode));
        return -2;
    }

    if (pAwsRspInfo->udwServiceType == EN_SERVICE_TYPE_ACTION_CENTER_RSP)
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

TINT32 CSearchNetIO::OnReportSvrResponse(LTasksGroup *pstTasksGrp, SSession *poSession)
{
    LTask               *pstTask = 0;
    TCHAR               *pszIp = 0;
    TUINT16             uwPort = 0;
    TUINT32				udwIdx = 0;
    TINT32				dwRetCode = 0;

    poSession->m_uddwDownRqstTimeEnd = CTimeUtils::GetCurTimeUs();

    poSession->ResetReportInfo();
    vector<ReportRspInfo*>& vecRsp = poSession->m_vecReportRsp;
    ReportRspInfo* pReportRsp = NULL;

    TSE_LOG_DEBUG(m_poServLog, ("OnReportSvrResponse: [vaildtasksnum=%u] [seq=%u]",
        pstTasksGrp->m_uValidTasks,
        poSession->m_udwSeqNo));

    for (udwIdx = 0; udwIdx < pstTasksGrp->m_uValidTasks; udwIdx++)
    {
        pstTask = &pstTasksGrp->m_Tasks[udwIdx];

        GetIp2PortByHandle(pstTask->hSession, &uwPort, &pszIp);

        pReportRsp = new ReportRspInfo;
        vecRsp.push_back(pReportRsp);

        TSE_LOG_DEBUG(m_poServLog, ("report svr res: taskid[%u]: [ip=%s] [port=%u] [send=%u], [recv=%u], [timeout=%u], [donw_busy=%u], [socket_closed=%u], [verify_failed=%u] [recv_data_len=%u] [cost_time=%llu]us [req_type=%u] [seq=%u]",
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

        if (0 == pstTask->_ucIsSendOK || 0 == pstTask->_ucIsReceiveOK || 1 == pstTask->_ucTimeOutEvent)
        {
            // 加入超时统计
            if (poSession->m_bReportUserSvrNodeExist && pstTask->hSession == poSession->m_pstReportUserSvrNode->m_stDownHandle)
            {
                CDownMgr::Instance()->zk_AddTimeOut(poSession->m_pstReportUserSvrNode);
            }
            else if (poSession->m_bMailUserSvrNodeExist && pstTask->hSession == poSession->m_pstMailUserSvrNode->m_stDownHandle)
            {
                CDownMgr::Instance()->zk_AddTimeOut(poSession->m_pstMailUserSvrNode);
            }
            TSE_LOG_ERROR(m_poServLog, ("report svr res: [ip=%s] [port=%u] [send=%u], [recv=%u], [timeout=%u], [cost_time=%llu]us [seq=%u]",
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
                TSE_LOG_ERROR(m_poServLog, ("report svr res: [ip=%s] [port=%u] recv_data_len[%u]>MAX_HS_RES_DATA_LEN [cost_time=%llu]us [seq=%u]",
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
                dwRetCode = ParseReportSvrResponse(pstTask->_pReceivedData, pstTask->_uReceivedDataLen, pReportRsp);
                if (dwRetCode < 0)
                {
                    // 加入错误统计
                    if (poSession->m_bReportUserSvrNodeExist && pstTask->hSession == poSession->m_pstReportUserSvrNode->m_stDownHandle)
                    {
                        CDownMgr::Instance()->zk_AddError(poSession->m_pstReportUserSvrNode);
                    }
                    else if (poSession->m_bMailUserSvrNodeExist && pstTask->hSession == poSession->m_pstMailUserSvrNode->m_stDownHandle)
                    {
                        CDownMgr::Instance()->zk_AddError(poSession->m_pstMailUserSvrNode);
                    }

                    TSE_LOG_ERROR(m_poServLog, ("report svr res: [ip=%s] [port=%u] failed[%d] [seq=%u]",
                        pszIp, 
                        uwPort, 
                        dwRetCode,
                        poSession->m_udwSeqNo));
                    if (EN_RET_CODE__SUCCESS == poSession->m_stCommonResInfo.m_dwRetCode)
                    {
                        poSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_PACKAGE_ERR;
                        dwRetCode = -3;
                    }
                    // break;
                }
                else
                {
                    TSE_LOG_DEBUG(m_poServLog, ("report svr res: service[%u], ret[%d], buf_len[%u] [seq=%u]",
                        pReportRsp->udwServiceType,
                        pReportRsp->dwRetCode,
                        pReportRsp->sRspContent.size(),
                        poSession->m_udwSeqNo));
                }
            }
        }
    }

    poSession->m_udwDownRqstType = 0;

    return 0;
}

TINT32 CSearchNetIO::ParseReportSvrResponse(TUCHAR *pszPack, TUINT32 udwPackLen, ReportRspInfo* pReportRspInfo)
{
    TUCHAR *pszValBuf = NULL;
    TUINT32 udwValBufLen = 0;

    m_pUnPackTool->UntachPackage();
    m_pUnPackTool->AttachPackage(pszPack, udwPackLen);
    if (FALSE == m_pUnPackTool->Unpack())
    {
        return -1;
    }
    pReportRspInfo->udwServiceType = m_pUnPackTool->GetServiceType();
    m_pUnPackTool->GetVal(EN_GLOBAL_KEY__RES_CODE, &pReportRspInfo->dwRetCode);

    //2XX的返回码,都是正常的情况
    if (pReportRspInfo->dwRetCode / 100 != 2)
    {
        TSE_LOG_ERROR(m_poServLog, ("report svr res: pReportRspInfo->dwRetCode=%d\n", pReportRspInfo->dwRetCode));
        return -2;
    }

    if (pReportRspInfo->udwServiceType == EN_SERVICE_TYPE_REPORT_CENTER_RSP)
    {
        pReportRspInfo->dwRspType = 0;
        m_pUnPackTool->GetVal(EN_GLOBAL_KEY__RES_BUF, &pszValBuf, &udwValBufLen);
        pReportRspInfo->sRspContent.resize(udwValBufLen);
        memcpy((char*)pReportRspInfo->sRspContent.c_str(), pszValBuf, udwValBufLen);
    }
    else if (pReportRspInfo->udwServiceType == EN_SERVICE_TYPE_MAIL_CENTER_RSP)
    {
        pReportRspInfo->dwRspType = 1;
        m_pUnPackTool->GetVal(EN_GLOBAL_KEY__RES_BUF, &pszValBuf, &udwValBufLen);
        pReportRspInfo->sRspContent.resize(udwValBufLen);
        memcpy((char*)pReportRspInfo->sRspContent.c_str(), pszValBuf, udwValBufLen);
    }
    else
    {
        return -3;
    }

    return 0;
}

TINT32 CSearchNetIO::OnPurchaseCheckResponse(LTasksGroup *pstTasksGrp, SSession *poSession)
{
    LTask               *pstTask = 0;
    TCHAR               *pszIp = 0;
    TUINT16             uwPort = 0;
    TUINT32				udwIdx = 0;
    TINT32				dwRetCode = 0;

    poSession->m_uddwDownRqstTimeEnd = CTimeUtils::GetCurTimeUs();

    poSession->m_stPurchaseRsp.Reset();

    TSE_LOG_DEBUG(m_poServLog, ("OnPurchaseCheckResponse: [vaildtasksnum=%u] [seq=%u]",
        pstTasksGrp->m_uValidTasks,
        poSession->m_udwSeqNo));

    for (udwIdx = 0; udwIdx < pstTasksGrp->m_uValidTasks; udwIdx++)
    {
        pstTask = &pstTasksGrp->m_Tasks[udwIdx];

        GetIp2PortByHandle(pstTask->hSession, &uwPort, &pszIp);

        TSE_LOG_DEBUG(m_poServLog, ("purchase_check res: taskid[%u]: [ip=%s] [port=%u] [send=%u], [recv=%u], [timeout=%u], "
            "[donw_busy=%u], [socket_closed=%u], [verify_failed=%u] [recv_data_len=%u] [cost_time=%llu]us [req_type=%u] [seq=%u]",
            udwIdx, pszIp, uwPort, pstTask->_ucIsSendOK, pstTask->_ucIsReceiveOK, pstTask->_ucTimeOutEvent, 
            pstTask->_ucIsDownstreamBusy, pstTask->_ucIsSockAlreadyClosed, pstTask->_ucIsVerifyPackFail, 
            pstTask->_uReceivedDataLen, poSession->m_uddwDownRqstTimeEnd - poSession->m_uddwDownRqstTimeBeg, 
            poSession->m_udwDownRqstType, poSession->m_udwSeqNo));

        if (0 == pstTask->_ucIsSendOK || 0 == pstTask->_ucIsReceiveOK || 1 == pstTask->_ucTimeOutEvent)
        {
            // 加入超时统计
            CDownMgr::Instance()->zk_AddTimeOut(poSession->m_pstReportUserSvrNode);
            TSE_LOG_ERROR(m_poServLog, ("purchase_check res: [ip=%s] [port=%u] [send=%u], [recv=%u], [timeout=%u], [cost_time=%llu]us [seq=%u]",
                pszIp, uwPort, pstTask->_ucIsSendOK, pstTask->_ucIsReceiveOK, pstTask->_ucTimeOutEvent,
                poSession->m_uddwDownRqstTimeEnd - poSession->m_uddwDownRqstTimeBeg, 
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

                TSE_LOG_ERROR(m_poServLog, ("purchase_check res: [ip=%s] [port=%u] recv_data_len[%u]>MAX_HS_RES_DATA_LEN [cost_time=%llu]us [seq=%u]",
                    pszIp, uwPort, pstTask->_uReceivedDataLen, poSession->m_uddwDownRqstTimeEnd - poSession->m_uddwDownRqstTimeBeg,
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
                dwRetCode = ParsePurchaseCheckResponse(pstTask->_pReceivedData, pstTask->_uReceivedDataLen, &poSession->m_stPurchaseRsp);
                if (dwRetCode < 0)
                {
                    // 加入错误统计
                    CDownMgr::Instance()->zk_AddError(poSession->m_pstReportUserSvrNode);

                    TSE_LOG_ERROR(m_poServLog, ("purchase_check res: [ip=%s] [port=%u] failed[%d] [seq=%u]",
                        pszIp, uwPort, dwRetCode, poSession->m_udwSeqNo));
                    if (EN_RET_CODE__SUCCESS == poSession->m_stCommonResInfo.m_dwRetCode)
                    {
                        poSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_PACKAGE_ERR;
                        dwRetCode = -3;
                    }
                    // break;
                }
                else
                {
                    TSE_LOG_DEBUG(m_poServLog, ("purchase_check res: service[%u], ret[%d] [seq=%u]",
                        poSession->m_stPurchaseRsp.udwServiceType,
                        poSession->m_stPurchaseRsp.dwRetCode,
                        poSession->m_udwSeqNo));
                    break;
                }
            }
        }
    }

    poSession->m_udwDownRqstType = 0;

    return 0;
}

TINT32 CSearchNetIO::ParsePurchaseCheckResponse(TUCHAR *pszPack, TUINT32 udwPackLen, PurchaseRspInfo* pPurchaseRspInfo)
{
    TUCHAR *pszValBuf = NULL;
    TUINT32 udwValBufLen = 0;

    m_pUnPackTool->UntachPackage();
    m_pUnPackTool->AttachPackage(pszPack, udwPackLen);
    if (FALSE == m_pUnPackTool->Unpack())
    {
        return -1;
    }
    pPurchaseRspInfo->udwServiceType = m_pUnPackTool->GetServiceType();
    m_pUnPackTool->GetVal(EN_GLOBAL_KEY__RES_CODE, &pPurchaseRspInfo->dwRetCode);
    m_pUnPackTool->GetVal(EN_GLOBAL_KEY__RES_COST_TIME, &pPurchaseRspInfo->udwCostTime);
    if (!m_pUnPackTool->GetVal(EN_GLOBAL_KEY__RES_TYPE, &pPurchaseRspInfo->udwResType))
    {
        pPurchaseRspInfo->udwResType = EN_CONTENT_TYPE__STRING;
    }
    //2XX的返回码,都是正常的情况
    if (pPurchaseRspInfo->dwRetCode != 200)
    {
        TSE_LOG_ERROR(m_poServLog, ("purchase_check res: dwRetCode=%d\n", pPurchaseRspInfo->dwRetCode));
        return -2;
    }

    m_pUnPackTool->GetVal(EN_GLOBAL_KEY__RES_BUF, &pszValBuf, &udwValBufLen);
    pPurchaseRspInfo->sRspContent.resize(udwValBufLen);
    memcpy((char*)pPurchaseRspInfo->sRspContent.c_str(), pszValBuf, udwValBufLen);

    return 0;
}

TINT32 CSearchNetIO::OnRankSvrResponse(LTasksGroup *pstTasksGrp, SSession *poSession)
{
    LTask               *pstTask = 0;
    TCHAR               *pszIp = 0;
    TUINT16             uwPort = 0;
    TUINT32				udwIdx = 0;
    TINT32				dwRetCode = 0;

    poSession->m_uddwDownRqstTimeEnd = CTimeUtils::GetCurTimeUs();

    poSession->ResetRankSvrInfo();
    vector<RankSvrRspInfo*>& vecRsp = poSession->m_vecRankSvrRsp;
    RankSvrRspInfo* pRankRsp = NULL;

    TSE_LOG_DEBUG(m_poServLog, ("OnRankSvrResponse: [vaildtasksnum=%u] [seq=%u]",
        pstTasksGrp->m_uValidTasks,
        poSession->m_udwSeqNo));

    for (udwIdx = 0; udwIdx < pstTasksGrp->m_uValidTasks; udwIdx++)
    {
        pstTask = &pstTasksGrp->m_Tasks[udwIdx];

        GetIp2PortByHandle(pstTask->hSession, &uwPort, &pszIp);

        pRankRsp = new RankSvrRspInfo;
        vecRsp.push_back(pRankRsp);

        TSE_LOG_DEBUG(m_poServLog, ("rank svr res: taskid[%u]: [ip=%s] [port=%u] [send=%u], [recv=%u], [timeout=%u], [donw_busy=%u], [socket_closed=%u], [verify_failed=%u] [recv_data_len=%u] [cost_time=%llu]us [req_type=%u] [seq=%u]",
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

        if (0 == pstTask->_ucIsSendOK || 0 == pstTask->_ucIsReceiveOK || 1 == pstTask->_ucTimeOutEvent)
        {
            // 加入超时统计
            if (poSession->m_bRankSvrExist && pstTask->hSession == poSession->m_pstRankSvrNode->m_stDownHandle)
            {
                CDownMgr::Instance()->zk_AddTimeOut(poSession->m_pstRankSvrNode);
            }
            TSE_LOG_ERROR(m_poServLog, ("rank svr res: [ip=%s] [port=%u] [send=%u], [recv=%u], [timeout=%u], [cost_time=%llu]us [seq=%u]",
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
                TSE_LOG_ERROR(m_poServLog, ("rank svr res: [ip=%s] [port=%u] recv_data_len[%u]>MAX_AWS_RES_DATA_LEN [cost_time=%llu]us [seq=%u]",
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
                dwRetCode = ParseRankSvrResponse(pstTask->_pReceivedData, pstTask->_uReceivedDataLen, pRankRsp);
                if (dwRetCode < 0)
                {
                    // 加入错误统计
                    if (poSession->m_bRankSvrExist && pstTask->hSession == poSession->m_pstRankSvrNode->m_stDownHandle)
                    {
                        CDownMgr::Instance()->zk_AddError(poSession->m_pstRankSvrNode);
                    }

                    TSE_LOG_ERROR(m_poServLog, ("rank svr res: [ip=%s] [port=%u] failed[%d] [seq=%u]",
                        pszIp, 
                        uwPort, 
                        dwRetCode,
                        poSession->m_udwSeqNo));
                    if (EN_RET_CODE__SUCCESS == poSession->m_stCommonResInfo.m_dwRetCode)
                    {
                        poSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_PACKAGE_ERR;
                        dwRetCode = -3;
                    }
                    // break;
                }
                else
                {
                    TSE_LOG_DEBUG(m_poServLog, ("rank svr res: uid[%u], ret[%d], buf_len[%u] [seq=%u]",
                        pRankRsp->m_udwUid,
                        pRankRsp->m_dwRetCode,
                        pRankRsp->m_sRspJson.size(),
                        poSession->m_udwSeqNo));
                }
            }
        }
    }

    poSession->m_udwDownRqstType = 0;

    return 0;
}

TINT32 CSearchNetIO::ParseRankSvrResponse(TUCHAR *pszPack, TUINT32 udwPackLen, RankSvrRspInfo* pRankRspInfo)
{
    TUCHAR *pszValBuf = NULL;
    TUINT32 udwValBufLen = 0;
    TUINT32 udwServiceType = 0;

    m_pUnPackTool->UntachPackage();
    m_pUnPackTool->AttachPackage(pszPack, udwPackLen);
    if (FALSE == m_pUnPackTool->Unpack())
    {
        return -1;
    }
    udwServiceType = m_pUnPackTool->GetServiceType();
    m_pUnPackTool->GetVal(EN_GLOBAL_KEY__RES_CODE, &pRankRspInfo->m_dwRetCode);

    //2XX的返回码,都是正常的情况
    if (pRankRspInfo->m_dwRetCode != 0)
    {
        TSE_LOG_ERROR(m_poServLog, ("report svr res: pRankRspInfo->dwRetCode=%d\n", pRankRspInfo->m_dwRetCode));
        return -2;
    }

    if (udwServiceType == EN_SERVICE_TYPE__SVR__RANK_RSP)
    {
        m_pUnPackTool->GetVal(EN_GLOBAL_KEY__TARGET_UID, &pRankRspInfo->m_udwUid);
        m_pUnPackTool->GetVal(EN_GLOBAL_KEY__RES_BUF, &pszValBuf, &udwValBufLen);
        pRankRspInfo->m_sRspJson.resize(udwValBufLen);
        memcpy((char*)pRankRspInfo->m_sRspJson.c_str(), pszValBuf, udwValBufLen);
    }
    else
    {
        return -3;
    }

    return 0;
}
