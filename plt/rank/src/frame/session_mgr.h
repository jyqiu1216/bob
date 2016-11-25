#ifndef _SSC_SESSION_MGR_H_
#define _SSC_SESSION_MGR_H_

#include "std_header.h"
#include "session.h"

/*
 * Session管理器
 */
class CSessionMgr
{
public:
    // 获取session管理器对象
    static CSessionMgr * Instance();

    // 初始化
    int Init(TUINT32 queue_size, CTseLogger *serv_log);

    // 获取session
    int WaitTillSession(SSession **session);

    // 释放session
    int ReleaseSession(SSession *session);

    int Uninit();
    ~CSessionMgr();

	int GetEmptySessionSize();

private:
    CSessionMgr();
    CSessionMgr(const CSessionMgr &);
    CSessionMgr & operator =(const CSessionMgr &);

    static CSessionMgr *m_session_mgr;

    CTaskQueue  *m_task_queue;
    CTseLogger  *m_serv_log;
};

#endif

