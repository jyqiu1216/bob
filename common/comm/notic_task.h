#ifndef _NOTIC_TASK_H_
#define _NOTIC_TASK_H_

#include <string>
#include "queue_t.h"
#include "base/common/wtsetypedef.h"

using namespace std;

struct SNoticTask
{
    string m_strTitle;
    TINT64 m_ddwUid;
    TINT64 m_ddwAid;
    TINT64 m_ddwSid;
    TINT64 m_ddwNotiType;
    string m_strContent;
    string m_strProject;
    TINT64 m_ddwNoticFlag;  // 倖繁容僕:0, 選男容僕:1
    TINT64 m_ddwNewNoticType;
    string m_strSound;

    void Reset()
    {
        m_strTitle = "";
        m_ddwUid = 0;
        m_ddwAid = 0;
        m_ddwSid = 0;
        m_ddwNotiType = 0;
        m_strContent =  "";
        m_strProject = "";
        m_ddwNoticFlag = 0;
        m_ddwNewNoticType = 0;
        m_strSound = "";
    }
};

typedef CQueueT<SNoticTask *> CNoticTaskQueue;

#endif



