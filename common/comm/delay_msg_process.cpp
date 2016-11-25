#include "delay_msg_task_mgr.h"
#include "delay_msg_process.h"
#include "time_utils.h"
#include <fstream>

using namespace std;

TINT32 CDelayMsgProcess::Init(CDelayMsgTaskQueue *pTaskQueue, CTseLogger *poServLog)
{
    if (NULL == pTaskQueue || NULL == poServLog)
    {
        return -1;
    }

    m_udwSeqno = 0;
    m_poServLog = poServLog;
    m_pTaskQueue = pTaskQueue;

    return 0;
}

void *CDelayMsgProcess::Start(void *pParam)
{
    if (NULL == pParam)
    {
        return NULL;
    }

    CDelayMsgProcess *poIns = (CDelayMsgProcess *)pParam;
    poIns->WorkRoutine();
    return NULL;
}

TINT32 CDelayMsgProcess::WorkRoutine()
{
    SDelayMsgTask *pstDelayTask = NULL;
    while (1)
    {
        if (m_pTaskQueue->WaitTillPop(pstDelayTask) != 0)
        {
            TSE_LOG_DEBUG(m_poServLog, ("WaitTillPop: pop null"));
            continue;
        }

        while (0 != DelayUpdate(pstDelayTask))
        {
        }

        CDelayMsgTaskMgr::Instance()->ReleaseDelayTask(pstDelayTask);
    }

    return 0;
}

TINT32 CDelayMsgProcess::DelayUpdate(SDelayMsgTask *pstDelayTask)
{
    m_udwSeqno++;
    TSE_LOG_DEBUG(m_poServLog, ("DelayUpdate Begin: [delay_seq=%u,empty_size=%u]",m_udwSeqno, m_pTaskQueue->Size()));

    TCHAR szFileName[1024];
    snprintf(szFileName, 1024, "./msg/msg.%lu",CTimeUtils::GetCurTimeUs());
    ofstream fp(szFileName);
    if (fp)
    {
        fp<<pstDelayTask->m_szMsg<<endl;
        fp.close();
    }

    TSE_LOG_DEBUG(m_poServLog, ("DelayUpdate end: [delay_seq=%u,empty_size=%u]", m_udwSeqno, m_pTaskQueue->Size()));

    return 0;
}
