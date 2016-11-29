#include "session_mgr.h"

CSessionMgr * CSessionMgr::m_session_mgr = NULL;

CSessionMgr::CSessionMgr()
{
}

CSessionMgr::~CSessionMgr()
{
}

CSessionMgr * CSessionMgr::Instance()
{
    if (NULL == m_session_mgr)
    {
        m_session_mgr = new CSessionMgr();
    }

    return m_session_mgr;
}

int CSessionMgr::Init(TUINT32 queue_size, CTseLogger *serv_log)
{
    if (0 == queue_size || NULL == serv_log)
    {
        return -1;
    }

    SSession *session_list = new SSession[queue_size];
    m_serv_log = serv_log;

    m_task_queue = new CTaskQueue();
    if (m_task_queue->Init(queue_size + 1) != 0)
    {
        return -2;
    }
    for (TUINT32 i = 0; i < queue_size; ++i)
    {
        session_list[i].Reset();
        m_task_queue->WaitTillPush(&(session_list[i]));
    }
    TSE_LOG_INFO(m_serv_log, ("CSessionMgr init succ"));

    return 0;
}

int CSessionMgr::WaitTillSession(SSession **session)
{
    if (0 != m_task_queue->WaitTillPop(*session))
    {
        TSE_LOG_ERROR(m_serv_log, ("CSessionMgr wait till session fail"));
        return -1;
    }
    TSE_LOG_DEBUG(m_serv_log, ("SessionMgr wait till pop session succ, task_size=%u", m_task_queue->Size()));

    return 0;
}

int CSessionMgr::ReleaseSession(SSession *session)
{
    if (NULL == session)
    {
        return 0;
    }

    if (session->m_ucIsUsing == 0)
    {
        TSE_LOG_ERROR(m_serv_log, ("session[%p] is not using or have returned", session));
        return 0;
    }

    TSE_LOG_DEBUG(m_serv_log, ("CSessionMgr beg to release session[%p] [seq=%u]", session, session->m_udwSeqNo));

    session->Reset();

    if (NULL == m_task_queue || 0 != m_task_queue->WaitTillPush(session))
    {
        TSE_LOG_ERROR(m_serv_log, ("CSessionMgr release session[%p] fail", session));
        return -1;
    }
    TSE_LOG_DEBUG(m_serv_log, ("CSessionMgr release session[%p] succ", session));

    return 0;
}

int CSessionMgr::Uninit()
{
    // do nothing
    return 0;
}

int CSessionMgr::GetEmptySessionSize()
{
    return m_task_queue->Size();
}


