#ifndef _USER_DYEING_INFO_H_
#define _USER_DYEING_INFO_H_
#include <stdio.h>
#include "base/common/wtse_std_header.h"
#include <map>

using namespace std;

#define UPDATE_USER_DYEING_INFO_FLAG ("../data/update_user_dyeing_info_flag")
#define USER_DYEING_INFO_FILE ("../data/user_dyeing_info.txt")

struct SDyeingInfo
{
    TINT64 m_ddwUserId;
    TINT64 m_ddwF1;
    TINT64 m_ddwF2;
    TINT64 m_ddwF3;
    TINT64 m_ddwF4;

    SDyeingInfo()
    {
        Reset();
    }

    void SetVal(TINT64 ddwUserId, TINT64 ddwF1, TINT64 ddwF2, TINT64 ddwF3, TINT64 ddwF4)
    {
        m_ddwUserId = ddwUserId;
        m_ddwF1 = ddwF1;
        m_ddwF2 = ddwF2;
        m_ddwF3 = ddwF3;
        m_ddwF4 = ddwF4;
    }

    void Reset()
    {
        memset(this, 0, sizeof(*this));
    }
};

class CUserDyeingInfo
{
private:
    static CUserDyeingInfo* m_poIns;
    static CUserDyeingInfo* m_poBackup;

public:
    static CUserDyeingInfo* GetInstance();
    static TINT32 Update();

public:
    TINT32 GetUserDyeingInfo(TINT64 ddwUserId, SDyeingInfo *pstDyeingInfo);
    ~CUserDyeingInfo()
    {
        for (map<TINT64, SDyeingInfo *>::iterator it = m_mapUserDyeingInfo.begin(); it != m_mapUserDyeingInfo.end(); it++)
        {
            if (it->second != NULL)
            {
                delete(it->second);
            }
        }
    }
private:
    CUserDyeingInfo()
    {
        m_mapUserDyeingInfo.clear();
    }

    TINT32 AddDyeingInfo();

    map<TINT64, SDyeingInfo *> m_mapUserDyeingInfo;
};

#endif


