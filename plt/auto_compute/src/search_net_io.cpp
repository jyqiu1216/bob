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
    m_hListenSock = CreateListenSocket(CGlobalServ::m_poConf->m_szModuleIp, CConfBase::GetInt("serv_port"));
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
	TSE_LOG_DEBUG(m_poServLog, ("recv res next=%u,exp_serv=%u,session[%p] [seq=%u]", \
		poSession->m_udwNextProcedure, poSession->m_udwExpectProcedure, poSession, poSession->m_udwSeqNo));
	switch (poSession->m_udwExpectProcedure)
	{
    case EN_EXPECT_PROCEDURE__AWS:
		OnAwsResponse(pstTasksGrp, poSession);
		break;
    case  EN_EXPECT_PROCEDURE__DB:
        OnDbResponse(pstTasksGrp, poSession);
        break;
	case EN_EXPECT_PROCEDUER__LOCK_GET:
		OnLockResponse(pstTasksGrp, poSession);
		break;
	case EN_EXPECT_PROCEDUER__LOCK_RELEASE:
        poSession->m_uddwDownRqstTimeEnd = CTimeUtils::GetCurTimeUs();
        poSession->m_uddwLockSumTime += poSession->m_uddwDownRqstTimeEnd - poSession->m_uddwDownRqstTimeBeg;
		break;
    case EN_EXPECT_PROCEDURE__EVENT:
        OnEventResponse(pstTasksGrp, poSession);
        break;
    case EN_EXPECT_PROCEDURE__DATA_CENTER:
        OnDataCenterResponse(pstTasksGrp, poSession);
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

	for(udwIdx = 0; udwIdx < pstTasksGrp->m_uValidTasks; udwIdx++)
	{
		pstTask = &pstTasksGrp->m_Tasks[udwIdx];
		GetIp2PortByHandle(pstTask->hSession, &uwPort, &pszIp);
		
		pAwsRsp = new AwsRspInfo;
		vecRsp.push_back(pAwsRsp);

		TSE_LOG_DEBUG(m_poServLog, ("hs res, [ip=%s] [port=%u] [send=%u], [recv=%u], [timeout=%u], "
			"[donw_busy=%u], [socket_closed=%u], [verify_failed=%u] [recv_data_len=%u] [cost_time=%llu]us [req_type=%u] [seq=%u]",
			pszIp, uwPort, pstTask->_ucIsSendOK, pstTask->_ucIsReceiveOK, pstTask->_ucTimeOutEvent,
			pstTask->_ucIsDownstreamBusy, pstTask->_ucIsSockAlreadyClosed, pstTask->_ucIsVerifyPackFail,
			pstTask->_uReceivedDataLen, poSession->m_uddwDownRqstTimeEnd - poSession->m_uddwDownRqstTimeBeg,
            poSession->m_udwDownRqstType, poSession->m_udwSeqNo));

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
			TSE_LOG_ERROR(m_poServLog, ("hs res, [ip=%s] [port=%u] [send=%u], [recv=%u], [timeout=%u], [cost_time=%llu]us [seq=%u]",
                pszIp, uwPort, pstTask->_ucIsSendOK, pstTask->_ucIsReceiveOK, pstTask->_ucTimeOutEvent,
				poSession->m_uddwDownRqstTimeEnd - poSession->m_uddwDownRqstTimeBeg,
                poSession->m_udwSeqNo));
            if (EN_RET_CODE__SUCCESS == poSession->m_stCommonResInfo.m_dwRetCode)
            {
                poSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__TIMEOUT;
                dwRetCode = -1;
            }
			//break;
		}
		else
		{
			if(pstTask->_uReceivedDataLen > MAX_AWS_RES_DATA_LEN)
			{
				TSE_LOG_ERROR(m_poServLog, ("hs res, [ip=%s] [port=%u] recv_data_len[%u]>MAX_HS_RES_DATA_LEN [cost_time=%llu]us [seq=%u]",
                    pszIp, uwPort, pstTask->_uReceivedDataLen, poSession->m_uddwDownRqstTimeEnd - poSession->m_uddwDownRqstTimeBeg,
					poSession->m_udwSeqNo));
				poSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PACKAGE_LEN_OVERFLOW;
				dwRetCode = -2;
				break;
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
					
					TSE_LOG_ERROR(m_poServLog, ("ParseAwsResponse[%u] failed[%d] [ip=%s] [port=%u] [seq=%u]", 
                        udwIdx, dwRetCode, pszIp, uwPort, poSession->m_udwSeqNo));
                    if (EN_RET_CODE__SUCCESS == poSession->m_stCommonResInfo.m_dwRetCode)
                    {
                        poSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_PACKAGE_ERR;
                        dwRetCode = -3;
                    }
				}
				else
				{
					TSE_LOG_DEBUG(m_poServLog, ("ParseAwsResponse[%u]: service[%u], ret[%d], table_name[%s], buf_len[%u] [seq=%u]", 
						udwIdx, pAwsRsp->udwServiceType, pAwsRsp->dwRetCode,  pAwsRsp->sTableName.c_str(), 
                        pAwsRsp->sRspContent.size(), poSession->m_udwSeqNo));
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
        GetIp2PortByHandle(pstTask->hSession, &uwPort, &pszIp);

        pDbRsp = new DbRspInfo;
        vecRsp.push_back(pDbRsp);

        TSE_LOG_DEBUG(m_poServLog, ("hs res, [ip=%s] [port=%u] [send=%u], [recv=%u], [timeout=%u], "
            "[donw_busy=%u], [socket_closed=%u], [verify_failed=%u] [recv_data_len=%u] [cost_time=%llu]us [req_type=%u] [seq=%u]",
            pszIp, uwPort, pstTask->_ucIsSendOK, pstTask->_ucIsReceiveOK, pstTask->_ucTimeOutEvent,
            pstTask->_ucIsDownstreamBusy, pstTask->_ucIsSockAlreadyClosed, pstTask->_ucIsVerifyPackFail,
            pstTask->_uReceivedDataLen, poSession->m_uddwDownRqstTimeEnd - poSession->m_uddwDownRqstTimeBeg,
            poSession->m_udwDownRqstType, poSession->m_udwSeqNo));

        if(0 == pstTask->_ucIsSendOK || 0 == pstTask->_ucIsReceiveOK || 1 == pstTask->_ucTimeOutEvent )
        {
            // 加入超时统计
            if(poSession->m_bDbProxyNodeExist)
            {
                CDownMgr::Instance()->zk_AddTimeOut(poSession->m_pstDbProxyNode);
            }
            TSE_LOG_ERROR(m_poServLog, ("hs res, [ip=%s] [port=%u] [send=%u], [recv=%u], [timeout=%u], [cost_time=%llu]us [seq=%u]",
                pszIp, uwPort, pstTask->_ucIsSendOK, pstTask->_ucIsReceiveOK, pstTask->_ucTimeOutEvent,
                poSession->m_uddwDownRqstTimeEnd - poSession->m_uddwDownRqstTimeBeg,
                poSession->m_udwSeqNo));
            poSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__TIMEOUT;
            dwRetCode = -1;
            break;
        }
        else
        {
            if(pstTask->_uReceivedDataLen > MAX_AWS_RES_DATA_LEN)
            {
                TSE_LOG_ERROR(m_poServLog, ("hs res, [ip=%s] [port=%u] recv_data_len[%u]>MAX_HS_RES_DATA_LEN [cost_time=%llu]us [seq=%u]",
                    pszIp, uwPort, pstTask->_uReceivedDataLen, poSession->m_uddwDownRqstTimeEnd - poSession->m_uddwDownRqstTimeBeg,
                    poSession->m_udwSeqNo));
                poSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PACKAGE_LEN_OVERFLOW;
                dwRetCode = -2;
                break;
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

                    TSE_LOG_ERROR(m_poServLog, ("ParseDbResponse[%u] failed[%d] [ip=%s] [port=%u] [seq=%u]", 
                        udwIdx, dwRetCode, pszIp, uwPort, poSession->m_udwSeqNo));
                    poSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_PACKAGE_ERR;
                    dwRetCode = -3;
                    break;
                }
                else
                {
                    TSE_LOG_DEBUG(m_poServLog, ("ParseDbResponse[%u]: service[%u], ret[%d], table[%s],  buf_len[%u] [seq=%u]", \
                        udwIdx, pDbRsp->udwServiceType, pDbRsp->dwRetCode, pDbRsp->sTableName.c_str(),pDbRsp->sRspContent.size(), poSession->m_udwSeqNo));
                }
            }
        }
    }

    poSession->m_udwDownRqstType = 0;

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

    TSE_LOG_DEBUG(m_poServLog, ("data center res: OnDataCenterResponse: [vaildtasksnum=%u] [seq=%u]", \
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

    TSE_LOG_DEBUG(m_poServLog, ("event res: ParseEventResponse: [vaildtasksnum=%u] [seq=%u]", \
        pstTasksGrp->m_uValidTasks,
        poSession->m_udwSeqNo));

    for(udwIdx = 0; udwIdx < pstTasksGrp->m_uValidTasks; udwIdx++)
    {
        pstTask = &pstTasksGrp->m_Tasks[udwIdx];

        /**************************************************************************************/
        EventReqInfo OriginEventReq;
        m_pUnPackTool->UntachPackage();
        m_pUnPackTool->AttachPackage(pstTask->pData, pstTask->uValidDataLen);
        if(FALSE == m_pUnPackTool->Unpack())
        {
            TSE_LOG_DEBUG(m_poServLog, ("aws res: taskid[%u]: origin event req unpack error [seq=%u]", \
                udwIdx, \
                poSession->m_udwSeqNo));
        }
        /**************************************************************************************/
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
            poSession->m_udwDownRqstType, \
            poSession->m_udwSeqNo));

        if(0 == pstTask->_ucIsSendOK || 0 == pstTask->_ucIsReceiveOK || 1 == pstTask->_ucTimeOutEvent)
        {
            // 加入超时统计
            if(poSession->m_bEventProxyExist)
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
            break;
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
                    if(poSession->m_bEventProxyExist)
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
                        poSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_PACKAGE_ERR;
                        dwRetCode = -3;
                    }
                    // break;
                }
                else
                {
                    TSE_LOG_DEBUG(m_poServLog, ("event res: buf_len[%u] [seq=%u]", \
                        pEventRsp->sRspContent.size(), \
                        poSession->m_udwSeqNo));
                }
            }
        }
    }

    poSession->m_udwDownRqstType = 0;
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
    if(pEventRspInfo->dwRetCode != 0)
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



TINT32	CSearchNetIO::ParseDbResponse(TUCHAR *pszPack, TUINT32 udwPackLen,  DbRspInfo* pDbRspInfo)
{
    TUCHAR *pszValBuf = NULL;
    TUINT32 udwValBufLen = 0;

    m_pUnPackTool->UntachPackage();
    m_pUnPackTool->AttachPackage(pszPack, udwPackLen);
    if(FALSE == m_pUnPackTool->Unpack())
    {
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
        return -3;
    }

    return 0;	
}



/*
TINT32 CSearchNetIO::OnHsResponse( LTasksGroup *pstTasksGrp, SSession *poSession, SHsResInfo *pastHsResInfo, TUINT32 &udwResNum)
{
	LTask               *pstTask                = 0;
	TCHAR               *pszIp                  = 0;
	TUINT16             uwPort                  = 0;
	TUINT32				udwIdx					= 0;
	TINT32				dwRetCode				= 0;

	poSession->m_uddwDownRqstTimeEnd = CTimeUtils::GetCurTimeUs();

	udwResNum = pstTasksGrp->m_uValidTasks;

	for(udwIdx = 0; udwIdx < pstTasksGrp->m_uValidTasks; udwIdx++)
	{
		pstTask = &pstTasksGrp->m_Tasks[udwIdx];
		GetIp2PortByHandle(pstTask->hSession, &uwPort, &pszIp);

		pastHsResInfo[udwIdx].Reset();

		TSE_LOG_DEBUG(m_poServLog, ("hs res, [ip=%s] [port=%u] [send=%u], [recv=%u], [timeout=%u], "
			"[donw_busy=%u], [socket_closed=%u], [verify_failed=%u] [recv_data_len=%u] [cost_time=%llu]us [seq=%u]",
			pszIp, uwPort, pstTask->_ucIsSendOK, pstTask->_ucIsReceiveOK, pstTask->_ucTimeOutEvent,
			pstTask->_ucIsDownstreamBusy, pstTask->_ucIsSockAlreadyClosed, pstTask->_ucIsVerifyPackFail,
			pstTask->_uReceivedDataLen, poSession->m_uddwDownRqstTimeEnd - poSession->m_uddwDownRqstTimeBeg,
			poSession->m_udwSeqNo));	

		if(0 == pstTask->_ucIsSendOK || 0 == pstTask->_ucIsReceiveOK || 1 == pstTask->_ucTimeOutEvent )
		{
			// 加入超时统计
			if(poSession->m_udwExpectProcedure == EN_PROCEDURE__ACCOUNT_SEARCH_RESPONSE)
			{
				if(poSession->m_bGlobalHsNodeExist)
				{
					CDownMgr::Instance()->zk_AddTimeOut(poSession->m_pstGlobalHsNode);
				}
			}
			else
			{
				if(poSession->m_bLocalHsNodeExist)
				{
					CDownMgr::Instance()->zk_AddTimeOut(poSession->m_pstLocalHsNode);
				}
			}
			TSE_LOG_ERROR(m_poServLog, ("hs res, [send=%u], [recv=%u], [timeout=%u], [cost_time=%llu]us [seq=%u]",
				pstTask->_ucIsSendOK, pstTask->_ucIsReceiveOK, pstTask->_ucTimeOutEvent,
				poSession->m_uddwDownRqstTimeEnd - poSession->m_uddwDownRqstTimeBeg,
				poSession->m_udwSeqNo));
			poSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__TIMEOUT;
			dwRetCode = -1;
			break;
		}
		else
		{
			if(pstTask->_uReceivedDataLen > MAX_HS_RES_DATA_LEN)
			{
				TSE_LOG_ERROR(m_poServLog, ("hs res, recv_data_len[%u]>MAX_HS_RES_DATA_LEN [cost_time=%llu]us [seq=%u]",
					pstTask->_uReceivedDataLen, poSession->m_uddwDownRqstTimeEnd - poSession->m_uddwDownRqstTimeBeg,
					poSession->m_udwSeqNo));
				poSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PACKAGE_LEN_OVERFLOW;
				dwRetCode = -2;
				break;
			}
			else
			{
				dwRetCode = ParseHsResponse(pstTask->_pReceivedData, pstTask->_uReceivedDataLen, &pastHsResInfo[udwIdx]);
				if(dwRetCode < 0)
				{
					// 加入错误统计
					if(poSession->m_udwExpectProcedure == EN_PROCEDURE__ACCOUNT_SEARCH_RESPONSE)
					{
						if(poSession->m_bGlobalHsNodeExist)
						{
							CDownMgr::Instance()->zk_AddError(poSession->m_pstGlobalHsNode);
						}
					}
					else
					{
						if(poSession->m_bLocalHsNodeExist)
						{
							CDownMgr::Instance()->zk_AddError(poSession->m_pstLocalHsNode);
						}
					}

					TSE_LOG_ERROR(m_poServLog, ("ParseHsResponse[%u] failed[%d] [seq=%u]", \
						udwIdx, dwRetCode, poSession->m_udwSeqNo));
					poSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_PACKAGE_ERR;
					dwRetCode = -3;
					break;
				}
				else
				{
					TSE_LOG_DEBUG(m_poServLog, ("ParseHsResponse[%u]: service[%u],cmd[%d],ret[%d],total_res[%u],cur_res[%u],buf_len[%u] [seq=%u]", \
						udwIdx, pastHsResInfo[udwIdx].m_udwResServiceType, pastHsResInfo[udwIdx].m_enHsCommand,
						dwRetCode, pastHsResInfo[udwIdx].m_udwTotalResNum, 
						pastHsResInfo[udwIdx].m_udwCurResNum, pastHsResInfo[udwIdx].m_udwResDataLen, poSession->m_udwSeqNo));
				}
			}
		}
	}

	return 0;
}

TINT32 CSearchNetIO::ParseHsResponse( TUCHAR *pszPack, TUINT32 udwPackLen, SHsResInfo *pstHsResInfo )
{
	TUCHAR *pszValBuf = NULL;
	TUINT32 udwValBufLen = 0;

	m_pUnPackTool->UntachPackage();
	m_pUnPackTool->AttachPackage(pszPack, udwPackLen);
	if(FALSE == m_pUnPackTool->Unpack())
	{
		return -1;
	}
	pstHsResInfo->m_udwResServiceType = m_pUnPackTool->GetServiceType();
	m_pUnPackTool->GetVal(EN_GLOBAL_KEY__RES_CODE, &pstHsResInfo->m_dwRetCode);
	m_pUnPackTool->GetVal(EN_KEY_HS2HU__RES_COMMAND, &pstHsResInfo->m_enHsCommand);
	m_pUnPackTool->GetVal(EN_GLOBAL_KEY__RES_COST_TIME, &pstHsResInfo->m_udwCostTime);
	m_pUnPackTool->GetVal(EN_KEY_HS2HU__RES_DB_TYPE, &pstHsResInfo->m_udwDbType);
	m_pUnPackTool->GetVal(EN_KEY_HS2HU__RES_TBL_TYPE, &pstHsResInfo->m_udwTableType);
	m_pUnPackTool->GetVal(EN_KEY_HS2HU__RES_INDEX_OPENTYPE, &pstHsResInfo->m_udwIndexOpenType);

	if(pstHsResInfo->m_dwRetCode == EN_RET_CODE__ZERO)
	{
		return 0;
	}
	else if(pstHsResInfo->m_dwRetCode != EN_RET_CODE__SUCCESS)
	{
		return -2;
	}
	
	if(pstHsResInfo->m_udwResServiceType == EN_SERVICE_TYPE_HS2HU__SEARCH_RSP)
	{
		m_pUnPackTool->GetVal(EN_GLOBAL_KEY__RES_TOTAL_RES_NUM, &pstHsResInfo->m_udwTotalResNum);
		m_pUnPackTool->GetVal(EN_GLOBAL_KEY__RES_CUR_RES_NUM, &pstHsResInfo->m_udwCurResNum);
		if(pstHsResInfo->m_udwCurResNum > 0)
		{
			m_pUnPackTool->GetVal(EN_GLOBAL_KEY__RES_BUF, &pszValBuf, &udwValBufLen);
			memcpy(pstHsResInfo->m_szResData, pszValBuf, udwValBufLen);
			pstHsResInfo->m_udwResDataLen = udwValBufLen;
		}
	}

	return 0;
}
*/

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
	
    //2XX的返回码,都是正常的情况
	if(pAwsRspInfo->dwRetCode/100 != 2)
	{
		TSE_LOG_ERROR(m_poServLog, ("pAwsRspInfo->dwRetCode=%d\n", pAwsRspInfo->dwRetCode));
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



TINT32 CSearchNetIO::OnLockResponse( LTasksGroup *pstTasksGrp, SSession *poSession )
{
	// do nothing
	LTask               *pstTask                = 0;
	TCHAR               *pszIp                  = 0;
	TUINT16             uwPort                  = 0;
	TUINT32				udwIdx					= 0;
	TINT32				dwRetCode				= 0;

	poSession->m_uddwDownRqstTimeEnd = CTimeUtils::GetCurTimeUs();

    poSession->m_uddwLockSumTime += poSession->m_uddwDownRqstTimeEnd - poSession->m_uddwDownRqstTimeBeg;

	for(udwIdx = 0; udwIdx < pstTasksGrp->m_uValidTasks && udwIdx < 1; udwIdx++)
	{
		pstTask = &pstTasksGrp->m_Tasks[udwIdx];
		GetIp2PortByHandle(pstTask->hSession, &uwPort, &pszIp);

		TSE_LOG_DEBUG(m_poServLog, ("lock res, [ip=%s] [port=%u] [send=%u], [recv=%u], [timeout=%u], "
			"[donw_busy=%u], [socket_closed=%u], [verify_failed=%u] [recv_data_len=%u] [cost_time=%llu]us [seq=%u]",
			pszIp, uwPort, pstTask->_ucIsSendOK, pstTask->_ucIsReceiveOK, pstTask->_ucTimeOutEvent,
			pstTask->_ucIsDownstreamBusy, pstTask->_ucIsSockAlreadyClosed, pstTask->_ucIsVerifyPackFail,
			pstTask->_uReceivedDataLen, poSession->m_uddwDownRqstTimeEnd - poSession->m_uddwDownRqstTimeBeg,
			poSession->m_udwSeqNo));	

		if(0 == pstTask->_ucIsSendOK || 0 == pstTask->_ucIsReceiveOK || 1 == pstTask->_ucTimeOutEvent )
		{
			// 加入超时统计
			if(poSession->m_bLockSvrExist)
			{
				CDownMgr::Instance()->zk_AddTimeOut(poSession->m_pstLockSvr);
			}
			TSE_LOG_ERROR(m_poServLog, ("lock res, [ip=%s] [port=%u] [send=%u], [recv=%u], [timeout=%u], [cost_time=%llu]us [seq=%u]",
                pszIp, uwPort, pstTask->_ucIsSendOK, pstTask->_ucIsReceiveOK, pstTask->_ucTimeOutEvent,
				poSession->m_uddwDownRqstTimeEnd - poSession->m_uddwDownRqstTimeBeg,
				poSession->m_udwSeqNo));
			poSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__TIMEOUT;
			dwRetCode = -1;
			break;
		}
		else
		{
			m_pUnPackTool->UntachPackage();
			m_pUnPackTool->AttachPackage(pstTask->_pReceivedData, pstTask->_uReceivedDataLen);
			if(FALSE == m_pUnPackTool->Unpack())
			{
                TSE_LOG_ERROR(m_poServLog, ("lock res: [ip=%s] [port=%u] parse fail [seq=%u]",
                    pszIp, uwPort, poSession->m_udwSeqNo));
				poSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_PACKAGE_ERR;
				return -2;
			}
			m_pUnPackTool->GetVal(EN_GLOBAL_KEY__RES_CODE, &poSession->m_dwLockRetCode);

			if(poSession->m_dwLockRetCode != EN_RET_CODE__SUCCESS)
			{
                TSE_LOG_ERROR(m_poServLog, ("lock res: [ip=%s] [port=%u] [lock_ret=%d] [seq=%u]",
                    pszIp, uwPort, poSession->m_dwLockRetCode, poSession->m_udwSeqNo));
				poSession->m_stCommonResInfo.m_dwRetCode = poSession->m_dwLockRetCode;
				return -3;
			}
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

    poSession->ResetMapSvrInfo();
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
            udwIdx, 
            pszIp, 
            uwPort, 
            pstTask->_ucIsSendOK, 
            pstTask->_ucIsReceiveOK, 
            pstTask->_ucTimeOutEvent, 
            pstTask->_ucIsDownstreamBusy, 
            pstTask->_ucIsSockAlreadyClosed, 
            pstTask->_ucIsVerifyPackFail, 
            pstTask->_uReceivedDataLen, 
            poSession->m_uddwDownRqstTimeEnd - poSession->m_uddwDownRqstTimeBeg, 
            poSession->m_udwDownRqstType, 
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
                pszIp, 
                uwPort, 
                pstTask->_ucIsSendOK, 
                pstTask->_ucIsReceiveOK, 
                pstTask->_ucTimeOutEvent, 
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

                TSE_LOG_ERROR(m_poServLog, ("action svr res: [ip=%s] [port=%u] recv_data_len[%u]>MAX_HS_RES_DATA_LEN [cost_time=%llu]us [seq=%u]",
                    pszIp, 
                    uwPort, 
                    pstTask->_uReceivedDataLen, 
                    poSession->m_uddwDownRqstTimeEnd - poSession->m_uddwDownRqstTimeBeg, 
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
            CDownMgr::Instance()->zk_AddTimeOut(poSession->m_pstReportUserSvrNode);
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
                    CDownMgr::Instance()->zk_AddError(poSession->m_pstReportUserSvrNode);

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