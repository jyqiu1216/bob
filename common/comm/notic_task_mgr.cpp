#include "notic_task_mgr.h"

CNoticTaskMgr * CNoticTaskMgr::m_notic_task_mgr = NULL;

CNoticTaskMgr::CNoticTaskMgr()
{
}

CNoticTaskMgr::~CNoticTaskMgr()
{
}

CNoticTaskMgr *CNoticTaskMgr::GetInstance()
{
    if (NULL == m_notic_task_mgr)
    {
        m_notic_task_mgr = new CNoticTaskMgr();
    }

    return m_notic_task_mgr;
}

int CNoticTaskMgr::Init(TUINT32 queue_size, CTseLogger *serv_log)
{
    if (0 == queue_size || NULL == serv_log)
    {
        return -1;
    }

    SNoticTask *notic_task_list = new SNoticTask[queue_size];
    m_serv_log = serv_log;

    m_notic_task_queue = new CNoticTaskQueue();
    if (m_notic_task_queue->Init(queue_size + 1) != 0)
    {
        return -2;
    }
    for(TUINT32 udwIdx = 0; udwIdx < queue_size; ++udwIdx)
    {
        notic_task_list[udwIdx].Reset();
        m_notic_task_queue->WaitTillPush(&(notic_task_list[udwIdx]));
    }

    TSE_LOG_INFO(m_serv_log, ("CNoticTaskMgr init succ"));

    return 0;
}

int CNoticTaskMgr::WaitTillNoticTask(SNoticTask **notic_task)
{
    if (0 != m_notic_task_queue->WaitTillPop(*notic_task))
    {
        TSE_LOG_ERROR(m_serv_log, ("CNoticTaskMgr wait till notic_task fail"));
        return -1;
    }
    TSE_LOG_DEBUG(m_serv_log, ("CNoticTaskMgr wait till pop notic_task succ, task_size=%u", m_notic_task_queue->Size()));

    return 0;
}

int CNoticTaskMgr::ReleaselNoticTask(SNoticTask *notic_task)
{
    if (NULL == notic_task)
	{
		return 0;
	}

    notic_task->Reset();

    if (NULL == m_notic_task_queue || 0 != m_notic_task_queue->WaitTillPush(notic_task))
    {
        TSE_LOG_ERROR(m_serv_log, ("CNoticTaskMgr release notic_task[%p] fail", notic_task));
        return -1;
    }
    TSE_LOG_DEBUG(m_serv_log, ("CNoticTaskMgr release notic_task[%p] succ", notic_task));

    return 0;
}

int CNoticTaskMgr::Uninit()
{
    // do nothing
    return 0;
}

int CNoticTaskMgr::GetEmptySessionSize()
{
	return m_notic_task_queue->Size();
}



