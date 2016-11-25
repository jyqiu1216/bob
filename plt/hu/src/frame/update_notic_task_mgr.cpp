#include "update_notic_task_mgr.h"

CUpdateNoticTaskMgr * CUpdateNoticTaskMgr::m_update_notic_task_mgr = NULL;

CUpdateNoticTaskMgr::CUpdateNoticTaskMgr()
{
}

CUpdateNoticTaskMgr::~CUpdateNoticTaskMgr()
{
}

CUpdateNoticTaskMgr *CUpdateNoticTaskMgr::GetInstance()
{
    if (NULL == m_update_notic_task_mgr)
    {
        m_update_notic_task_mgr = new CUpdateNoticTaskMgr();
    }

    return m_update_notic_task_mgr;
}

int CUpdateNoticTaskMgr::Init(TUINT32 queue_size, CTseLogger *serv_log)
{
    if (0 == queue_size || NULL == serv_log)
    {
        return -1;
    }

    SUpdateNoticTask *update_notic_task_list = new SUpdateNoticTask[queue_size];
    m_serv_log = serv_log;

    m_update_notic_task_queue = new CUpdateNoticTaskQueue();
    if (m_update_notic_task_queue->Init(queue_size + 1) != 0)
    {
        return -2;
    }
    for(TUINT32 udwIdx = 0; udwIdx < queue_size; ++udwIdx)
    {
        update_notic_task_list[udwIdx].Reset();
        m_update_notic_task_queue->WaitTillPush(&(update_notic_task_list[udwIdx]));
    }

    TSE_LOG_INFO(m_serv_log, ("CUpdateNoticTaskMgr init succ"));

    return 0;
}

int CUpdateNoticTaskMgr::WaitTillUpdateNoticTask(SUpdateNoticTask **update_notic_task)
{
    if (0 != m_update_notic_task_queue->WaitTillPop(*update_notic_task))
    {
        TSE_LOG_ERROR(m_serv_log, ("CUpdateNoticTaskMgr wait till update_notic_task fail "));
        return -1;
    }
    TSE_LOG_DEBUG(m_serv_log, ("CUpdateNoticTaskMgr wait till pop update_notic_task succ, [task_size=%u]", \
							m_update_notic_task_queue->Size()));

    return 0;
}

int CUpdateNoticTaskMgr::ReleaseUpdateNoticTask(SUpdateNoticTask *update_notic_task)
{
    if (NULL == update_notic_task)
	{
		return 0;
	}

    update_notic_task->Reset();

    if (NULL == m_update_notic_task_queue || 0 != m_update_notic_task_queue->WaitTillPush(update_notic_task))
    {
        TSE_LOG_ERROR(m_serv_log, ("CUpdateNoticTaskMgr release update_notic_task[%p] fail [seq=%u]", update_notic_task, update_notic_task->m_udwBSeqNo));
        return -1;
    }
    TSE_LOG_DEBUG(m_serv_log, ("CUpdateNoticTaskMgr release update_notic_task[%p] succ [seq=%u]", update_notic_task, update_notic_task->m_udwBSeqNo));

    return 0;
}

int CUpdateNoticTaskMgr::Uninit()
{
    // do nothing
    return 0;
}

int CUpdateNoticTaskMgr::GetEmptySessionSize()
{
	return m_update_notic_task_queue->Size();
}



