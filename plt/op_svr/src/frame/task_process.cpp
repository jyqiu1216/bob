#include "task_process.h"
#include "global_serv.h"
#include "base/os/wtsesocket.h"
#include "session_mgr.h"
#include "statistic.h"
#include "common_func.h"
#include "conf_base.h"

TINT32 CTaskProcess::Init(CConf *poConf, ILongConn *poSearchLongConn, ILongConn *poQueryLongConn,
    CTaskQueue *pTaskQueue, CTseLogger *poServLog, CTseLogger *poDayLog)
{
    if (NULL == poConf || NULL == poSearchLongConn || NULL == pTaskQueue || NULL == poServLog)
    {
        return -1;
    }

    TUINT32 idx = 0;
    TINT32 dwRetCode = 0;

    m_poConf = poConf;
    m_poSearchLongConn = poSearchLongConn;
    m_poQueryLongConn = poQueryLongConn;
    m_pTaskQueue = pTaskQueue;
    m_poServLog = poServLog;
    m_poClientReqLog = poDayLog;

    for (idx = 0; idx < MAX_LOCAL_HS_TASK_NUM; idx++)
    {
        m_pPackTool[idx] = new CBaseProtocolPack();
        m_pPackTool[idx]->Init();
    }
    m_pUnPackTool = new CBaseProtocolUnpack();
    m_pUnPackTool->Init();

    return 0;
}

void * CTaskProcess::Start(void *pParam)
{
    if (NULL == pParam)
    {
        return NULL;
    }

    CTaskProcess* poIns = (CTaskProcess*)pParam;
    poIns->WorkRoutine();
    return NULL;
}

TINT32 CTaskProcess::WorkRoutine()
{
    SSession *pstSession = NULL;
    while (1)
    {
        if (m_pTaskQueue->WaitTillPop(pstSession) != 0)
        {
            continue;
        }

        pstSession->m_uddwTimeBeg_Process = CTimeUtils::GetCurTimeUs();
        TSE_LOG_INFO(m_poServLog, ("WorkRoutine:WaitTillPop:session[%p],is_using[%u] task_size[%u] [seq=%u]", \
            pstSession, pstSession->m_ucIsUsing, m_pTaskQueue->Size(), pstSession->m_udwSeqNo));

        if (pstSession->m_ucIsUsing == 0)
        {
            CSessionMgr::Instance()->ReleaseSession(pstSession);
            continue;
        }

        // process by command and procedure
        //---------req process begin-------------
        //TODO:
        //---------req process end---------------
    }

    return 0;
}
