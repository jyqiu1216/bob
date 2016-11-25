#ifndef _DELAY_MSG_TASK_MGR_H_
#define _DELAY_MSG_TASK_MGR_H_

#include "base/log/wtselogger.h"
#include "delay_msg.h"

using namespace wtse::log;

class CDelayMsgTaskMgr
{
public:
    static CDelayMsgTaskMgr * Instance();

    // 初始化
    int Init(TUINT32 queue_size, CTseLogger *serv_log);

    int WaitTillDelayTask(SDelayMsgTask **delay_task);

    int ReleaseDelayTask(SDelayMsgTask *delay_task);

    int Uninit();
    ~CDelayMsgTaskMgr();

	int GetEmptySessionSize();

    CDelayMsgTaskQueue* GetCommQueue();

private:
    CDelayMsgTaskMgr();
    CDelayMsgTaskMgr(const CDelayMsgTaskMgr &);
    CDelayMsgTaskMgr & operator =(const CDelayMsgTaskMgr &);

    static CDelayMsgTaskMgr *m_delay_task_mgr;

    CDelayMsgTaskQueue  *m_object_pool;
    CTseLogger  *m_serv_log;

    // 外围的公共队列
    CDelayMsgTaskQueue  *m_com_queue;
};

#endif

