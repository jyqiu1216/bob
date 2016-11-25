#ifndef _UPDATE_NOTIC_TASK_MGR_H_
#define _UPDATE_NOTIC_TASK_MGR_H_

#include "base/log/wtselogger.h"
#include "update_notic_task.h"

using namespace wtse::log;

class CUpdateNoticTaskMgr
{
public:
    static CUpdateNoticTaskMgr *GetInstance();
    ~CUpdateNoticTaskMgr();

    int Init(TUINT32 queue_size, CTseLogger *serv_log);
    int Uninit();

    int WaitTillUpdateNoticTask(SUpdateNoticTask **update_notic_task);
    int ReleaseUpdateNoticTask(SUpdateNoticTask *update_notic_task);

	int GetEmptySessionSize();

private:
    CUpdateNoticTaskMgr();

    // 禁止类拷贝和类赋值
    CUpdateNoticTaskMgr(const CUpdateNoticTaskMgr&);
    CUpdateNoticTaskMgr &operator = (const CUpdateNoticTaskMgr&);

    static CUpdateNoticTaskMgr *m_update_notic_task_mgr;

    CUpdateNoticTaskQueue *m_update_notic_task_queue;
    CTseLogger *m_serv_log;
};

#endif

