#ifndef _UPDATE_NOTIC_TASK_H_
#define _UPDATE_NOTIC_TASK_H_

#include <string>
#include "queue_t.h"
#include "base/common/wtsetypedef.h"

using namespace std;

#pragma pack(1)


struct SUpdateNoticTask
{
    TINT64 m_ddwUid;
    TINT64 m_ddwPid;
    TINT32 m_dwType;
    TINT32 m_dwService;  
	TUINT32 m_udwBSeqNo;

    void Reset()
    {
        m_ddwUid = 0;
        m_ddwPid = 0;
        m_dwType = 0;
        m_dwService = 0;
		m_udwBSeqNo = 0;
    }
};
#pragma pack()

typedef CQueueT<SUpdateNoticTask *> CUpdateNoticTaskQueue;

#endif



