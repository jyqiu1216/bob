#ifndef _NOTIC_TASK_MGR_H_
#define _NOTIC_TASK_MGR_H_

#include "base/log/wtselogger.h"
#include "notic_task.h"

using namespace wtse::log;

class CNoticTaskMgr
{
public:
    static CNoticTaskMgr *GetInstance();
    ~CNoticTaskMgr();

    int Init(TUINT32 queue_size, CTseLogger *serv_log);
    int Uninit();

    int WaitTillNoticTask(SNoticTask **notic_task);
    int ReleaselNoticTask(SNoticTask *notic_task);

	int GetEmptySessionSize();

private:
    CNoticTaskMgr();

    // 禁止类拷贝和类赋值
    CNoticTaskMgr(const CNoticTaskMgr&);
    CNoticTaskMgr &operator = (const CNoticTaskMgr&);

    static CNoticTaskMgr *m_notic_task_mgr;

    CNoticTaskQueue *m_notic_task_queue;
    CTseLogger *m_serv_log;
};

#endif

