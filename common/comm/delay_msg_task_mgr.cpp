#include "delay_msg_task_mgr.h"

CDelayMsgTaskMgr * CDelayMsgTaskMgr::m_delay_task_mgr = NULL;

CDelayMsgTaskMgr::CDelayMsgTaskMgr()
{
    m_object_pool = NULL;
    m_serv_log = NULL;
    m_com_queue = NULL;
}

CDelayMsgTaskMgr::~CDelayMsgTaskMgr()
{
}

CDelayMsgTaskMgr * CDelayMsgTaskMgr::Instance()
{
    if (NULL == m_delay_task_mgr)
    {
        m_delay_task_mgr = new CDelayMsgTaskMgr();
    }

    return m_delay_task_mgr;
}

int CDelayMsgTaskMgr::Init(TUINT32 queue_size, CTseLogger *serv_log)
{
    if (0 == queue_size || NULL == serv_log)
    {
        return -1;
    }

    SDelayMsgTask *delay_task_list = new SDelayMsgTask[queue_size];
    m_serv_log = serv_log;

    m_object_pool = new CDelayMsgTaskQueue();
    if (m_object_pool->Init(queue_size + 1) != 0)
    {
        return -2;
    }
    for (TUINT32 i = 0; i < queue_size; ++i)
    {
        delay_task_list[i].Reset();
        m_object_pool->WaitTillPush(&(delay_task_list[i]));
    }

    m_com_queue = new CDelayMsgTaskQueue();
    if (m_com_queue->Init(queue_size) != 0)
    {
        return -3;
    }

    TSE_LOG_INFO(m_serv_log, ("CDelayMsgTaskMgr init succ"));

    return 0;
}

int CDelayMsgTaskMgr::WaitTillDelayTask(SDelayMsgTask **delay_task)
{
    if (0 != m_object_pool->WaitTillPop(*delay_task))
    {
        TSE_LOG_ERROR(m_serv_log, ("CDelayMsgTaskMgr wait till delay_task fail"));
        return -1;
    }
    TSE_LOG_DEBUG(m_serv_log, ("CDelayMsgTaskMgr wait till pop delay_task succ, task_size=%u", m_object_pool->Size()));

    return 0;
}

int CDelayMsgTaskMgr::ReleaseDelayTask(SDelayMsgTask *delay_task)
{
    if (NULL == delay_task)
    {
        return 0;
    }

    delay_task->Reset();

    if (NULL == m_object_pool || 0 != m_object_pool->WaitTillPush(delay_task))
    {
        TSE_LOG_ERROR(m_serv_log, ("CDelayMsgTaskMgr release delay_task[%p] fail", delay_task));
        return -1;
    }
    TSE_LOG_DEBUG(m_serv_log, ("CDelayMsgTaskMgr release delay_task[%p] succ", delay_task));

    return 0;
}

int CDelayMsgTaskMgr::Uninit()
{
    // do nothing
    return 0;
}

int CDelayMsgTaskMgr::GetEmptySessionSize()
{
    return m_object_pool->Size();
}

CDelayMsgTaskQueue* CDelayMsgTaskMgr::GetCommQueue()
{
    return m_com_queue;
}
