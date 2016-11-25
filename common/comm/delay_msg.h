#ifndef _DELAY_MSG_TASK_H_
#define _DELAY_MSG_TASK_H_

#include "queue_t.h"
#include "base/common/wtsetypedef.h"

#define MAX_LEN_DELAY_MSG          (2048)

struct SDelayMsgTask
{
    TUINT32 m_udwSeqNo;
    TCHAR   m_szMsg[MAX_LEN_DELAY_MSG];


    TUINT32 m_udwBeginTime;

    void Reset()
    {
        m_udwSeqNo = 0;
        m_szMsg[0] = '\0';
    }
};

typedef CQueueT<SDelayMsgTask *> CDelayMsgTaskQueue;

#endif
