#ifndef _SSC_SESSION_MGR_H_
#define _SSC_SESSION_MGR_H_

#include "base/common/wtse_std_header.h"
#include "base/log/wtselogger.h"
#include "queue_t.h"

using namespace wtse::log;
/*
 * Session管理器
 */
template <typename T>
class CSessionMgrT
{
public:
    CSessionMgrT();
    ~CSessionMgrT();

    // 初始化
    int Init(TUINT32 queue_size, CTseLogger *serv_log);
    int Uninit();

    // 获取session
    int WaitTillSession(T **session);

    // 释放session
    int ReleaseSession(T *session);
    
    int GetEmptySessionSize();

private:
    CQueueT<T *>  *m_task_queue;
    wtse::log::CTseLogger  *m_serv_log;
};

template <typename T>
CSessionMgrT<T>::CSessionMgrT()
{
    m_task_queue = NULL;
    m_serv_log = NULL;
}

template <typename T>
CSessionMgrT<T>::~CSessionMgrT()
{
    // do nothing
}

template <typename T>
int CSessionMgrT<T>::Uninit()
{
    // do nothing
    return 0;
}

template <typename T>
int CSessionMgrT<T>::Init(TUINT32 queue_size, CTseLogger *serv_log)
{
    if (0 == queue_size || NULL == serv_log)
    {
        return -1;
    }

    T *session_list = new T[queue_size];
    m_serv_log = serv_log;

    m_task_queue = new CQueueT<T *>();
    if (m_task_queue->Init(queue_size + 1) != 0)
    {
        return -2;
    }
    for (TUINT32 i = 0; i < queue_size; ++i)
    {
        session_list[i].Reset();
        m_task_queue->WaitTillPush(&(session_list[i]));
    }
    TSE_LOG_INFO(m_serv_log, ("CSessionMgrT init succ"));

    return 0;
}

template <typename T>
int CSessionMgrT<T>::WaitTillSession(T **session)
{
    if (0 != m_task_queue->WaitTillPop(*session))
    {
        TSE_LOG_ERROR(m_serv_log, ("CSessionMgrT wait till session fail"));
        return -1;
    }
    TSE_LOG_DEBUG(m_serv_log, ("CSessionMgrT wait till pop session succ, task_size=%u", m_task_queue->Size()));

    return 0;
}

template <typename T>
int CSessionMgrT<T>::ReleaseSession(T *session)
{
    if(NULL == session)
    {
        return 0;
    }

    if(session->m_ucIsUsing == FALSE) //调试和校验用，防止反复释放
    {
        TSE_LOG_ERROR(m_serv_log, ("session[%p] is not using or have returned", session));
        return 0;
    }

    TSE_LOG_DEBUG(m_serv_log, ("CSessionMgrT beg to release session[%p] [seq=%u]", session, session->m_udwSeqNo));

    session->Reset();

    if (NULL == m_task_queue || 0 != m_task_queue->WaitTillPush(session))
    {
        TSE_LOG_ERROR(m_serv_log, ("CSessionMgrT release session[%p] fail", session));
        return -1;
    }
    TSE_LOG_DEBUG(m_serv_log, ("CSessionMgrT release session[%p] succ", session));

    return 0;
}

template <typename T>
int CSessionMgrT<T>::GetEmptySessionSize()
{
    return m_task_queue->Size();
}

#endif

