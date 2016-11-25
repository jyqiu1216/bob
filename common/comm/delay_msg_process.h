#ifndef _DELAY_MSG_PROCESS_H_
#define _DELAY_MSG_PROCESS_H_

#include "delay_msg.h"
#include "delay_msg_task_mgr.h"

class CDelayMsgProcess
{
public:
    TINT32 Init(CDelayMsgTaskQueue *pTaskQueue, CTseLogger *poServLog);
    static void *Start(void *pParam);
    TINT32 WorkRoutine();

public:
    TINT32 DelayUpdate(SDelayMsgTask *pstDelayTask);


public:
    CTseLogger *m_poServLog;
    CDelayMsgTaskQueue *m_pTaskQueue;

    TUINT32 m_udwSeqno;
};

#endif
