#include "update_notic_task_mgr.h"
#include "update_notic_process.h"
#include "cgi_socket.h"
#include "conf_base.h"
#include "jsoncpp/json/json.h"

CTseLogger *CUpdateNoticProcess::m_poServLog = NULL;
CUpdateNoticTaskQueue *CUpdateNoticProcess::m_pTaskQueue = NULL;

TINT32 CUpdateNoticProcess::Init(CUpdateNoticTaskQueue *pTaskQueue, CTseLogger *poServLog, TINT32 dwIpNum, UpdateSvrInfo *sUpdateSvrInfoList )
{
    if (NULL == pTaskQueue || NULL == poServLog)
    {
        return -1;
    }
	TSE_LOG_DEBUG(poServLog, ("CUpdateNoticProcess::Init "));
    m_poServLog 			 = poServLog;
    m_pTaskQueue 			 = pTaskQueue;
	m_dwIpNum				 = dwIpNum;
	m_sUpdateSvrInfoList	 = sUpdateSvrInfoList; 

    return 0;
}

void *CUpdateNoticProcess::Start(void *pParam)
{
    if (NULL == pParam)
    {
        return NULL;
    }

    CUpdateNoticProcess *poIns = (CUpdateNoticProcess *)pParam;
    poIns->WorkRoutine();
    return NULL;
}

TINT32 CUpdateNoticProcess::WorkRoutine()
{
    SUpdateNoticTask *pstUpdateNoticTask = NULL;
    while (1)
    {
    
        TSE_LOG_DEBUG(m_poServLog, ("CUpdateNoticProcess::WorkRoutine"));
    
        if (m_pTaskQueue->WaitTillPop(pstUpdateNoticTask) != 0)
        {
            TSE_LOG_DEBUG(m_poServLog, ("WaitTillPop: pop null"));
            continue;
        }

        if(0 != UpdateNotic_Process(pstUpdateNoticTask))
        {
            TSE_LOG_ERROR(m_poServLog, ("CUpdateNoticProcess failed [seq=%u]", pstUpdateNoticTask->m_udwBSeqNo));
        }

        CUpdateNoticTaskMgr::GetInstance()->ReleaseUpdateNoticTask(pstUpdateNoticTask);
    }

    return 0;
}


TINT32 CUpdateNoticProcess::UpdateNotic_Process(SUpdateNoticTask *pstUpdateNoticTask)
{

    TINT32 dwRetCode = 0;

    Json::Value jUpdateNoticRoot = Json::Value(Json::objectValue);
    jUpdateNoticRoot["uid"] = pstUpdateNoticTask->m_ddwUid;
    jUpdateNoticRoot["pid"] = pstUpdateNoticTask->m_ddwPid;
    jUpdateNoticRoot["type"] = pstUpdateNoticTask->m_dwType;
    jUpdateNoticRoot["service"] = pstUpdateNoticTask->m_dwService;
	jUpdateNoticRoot["seq"] = pstUpdateNoticTask->m_udwBSeqNo;

    // return ossGameEvaluateLogOut.str();
    Json::FastWriter write;
	write.omitEndingLineFeed();

	string strUpdateNotic = write.write(jUpdateNoticRoot);
	
	strUpdateNotic = strUpdateNotic + "\r\n\r\n";
  
    dwRetCode = SendUpdateNoticServer(pstUpdateNoticTask, strUpdateNotic);
    if (dwRetCode <= 0)
    {
        return -1;
    }


    TSE_LOG_INFO(m_poServLog, ("UpdateNotic_Process end: [strUpdateNotic=%s] [empty_size=%u] [seq=%u]", \
                               strUpdateNotic.c_str(), \
                               m_pTaskQueue->Size(),
                               pstUpdateNoticTask->m_udwBSeqNo));

    return 0;
}




TINT32 CUpdateNoticProcess::SendUpdateNoticServer(SUpdateNoticTask *pstUpdateNoticTask, string strUpdateNotic)
{
	
	
	T_SOCKET dwSocket;
	TINT32 ret=0;
//	TCHAR *ip="172.31.10.231";
	TINT64 ddwUid = pstUpdateNoticTask->m_ddwUid;
	TINT32 dwIndex = ddwUid % m_dwIpNum;
	string strIp = m_sUpdateSvrInfoList[dwIndex].m_strIp;
	TUINT16 uwPort = m_sUpdateSvrInfoList[dwIndex].m_uwPort;
	dwSocket = CCgiSocket::Connect( strIp.c_str(),  uwPort, 5000); 
	if( dwSocket <= 0 )
	{
		TSE_LOG_ERROR(m_poServLog, ("SendUpdateNoticServer socket fail [errMsg=%s] [dwSocket=%d] [ip=%s] [port=%d] [seq=%u]", \
							strerror(errno), dwSocket, strIp.c_str(), uwPort,  pstUpdateNoticTask->m_udwBSeqNo ));
		return -1;
	}
	ret = CCgiSocket::Send( dwSocket, strUpdateNotic.c_str(), strUpdateNotic.size());
	
	CCgiSocket::Close( dwSocket );

	if( ret <= 0 )
	{
		TSE_LOG_ERROR(m_poServLog, ("SendUpdateNoticServer error [ret=%d] [errMsg=%s] [dwSocket=%d] [ip=%s] [port=%d] [seq=%u]",\
			ret,  strerror(errno), dwSocket, strIp.c_str(), uwPort, pstUpdateNoticTask->m_udwBSeqNo));
	}

	TSE_LOG_DEBUG(m_poServLog, ("SendUpdateNoticServer succ [ret=%d] [errMsg=%s] [dwSocket=%d] [ip=%s] [port=%d] [seq=%u]",\
		ret,  strerror(errno), dwSocket, strIp.c_str(), uwPort, pstUpdateNoticTask->m_udwBSeqNo));

    return ret;
}



