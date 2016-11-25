#include "user_dyeing_info.h"
#include "game_info.h"

CUserDyeingInfo *CUserDyeingInfo::m_poIns = NULL;
CUserDyeingInfo* CUserDyeingInfo::m_poBackup = NULL;

CUserDyeingInfo* CUserDyeingInfo::GetInstance()
{
    if (m_poIns == NULL)
	{
        assert(0);
	}
    return m_poIns;
}

TINT32 CUserDyeingInfo::Update()
{
    CUserDyeingInfo *pTmp = new CUserDyeingInfo();

    if (pTmp->AddDyeingInfo() != 0)
    {
        delete(pTmp);
        return -1;
    }

    if (m_poBackup != NULL)
    {
        delete(m_poBackup);
        m_poBackup = NULL;
    }

    m_poBackup = m_poIns;

    m_poIns = pTmp;

    return 0;
}

TINT32 CUserDyeingInfo::GetUserDyeingInfo(TINT64 ddwUserId, SDyeingInfo *pstDyeingInfo)
{
    if (m_mapUserDyeingInfo.find(ddwUserId) != m_mapUserDyeingInfo.end())
    {
        SDyeingInfo *pTemp = m_mapUserDyeingInfo[ddwUserId];

        //TSE_LOG_INFO(CGameInfo::GetInstance()->m_poLog, ("CUserDyeingInfo::GetUserDyeingInfo: %ld,%ld,%ld,%ld,%ld",
        //    pTemp->m_ddwUserId, pTemp->m_ddwF1, pTemp->m_ddwF2, pTemp->m_ddwF3, pTemp->m_ddwF4));

        pstDyeingInfo->SetVal(pTemp->m_ddwUserId, pTemp->m_ddwF1, pTemp->m_ddwF2, pTemp->m_ddwF3, pTemp->m_ddwF4);

        return 0;
    }

    return -1;
}

TINT32 CUserDyeingInfo::AddDyeingInfo()
{
    TCHAR szLine[512];
    TCHAR *pCur = NULL;

    SDyeingInfo *pstDyeingInfo;

    TINT64 ddwUserId = 0;
    TINT64 ddwF1 = 0;
    TINT64 ddwF2 = 0;
    TINT64 ddwF3 = 0;
    TINT64 ddwF4 = 0;

    FILE *fp = fopen(USER_DYEING_INFO_FILE, "rt");
    if (fp == NULL)
    {
        return -1;
    }

    while (fgets(szLine, 512, fp))
    {
        pCur = szLine;

        ddwUserId = strtoll(pCur, NULL, 10);

        //TSE_LOG_INFO(CGameInfo::GetInstance()->m_poLog, ("CUserDyeingInfo::AddDyeingInfo: %ld",
        //    ddwUserId));
        if (m_mapUserDyeingInfo.find(ddwUserId) != m_mapUserDyeingInfo.end())
        {
            continue;
        }

        pCur = strchr(pCur, '\t');
        pCur++;
        ddwF1 = strtoll(pCur, NULL, 10);
        pCur = strchr(pCur, '\t');
        pCur++;
        ddwF2 = strtoll(pCur, NULL, 10);
        pCur = strchr(pCur, '\t');
        pCur++;
        ddwF3 = strtoll(pCur, NULL, 10);
        pCur = strchr(pCur, '\t');
        pCur++;
        ddwF4 = strtoll(pCur, NULL, 10);

        pstDyeingInfo = new SDyeingInfo();
        pstDyeingInfo->SetVal(ddwUserId, ddwF1, ddwF2, ddwF3, ddwF4);
        //TSE_LOG_INFO(CGameInfo::GetInstance()->m_poLog, ("CUserDyeingInfo::AddDyeingInfo: %ld,%ld,%ld,%ld,%ld",
        //    ddwUserId, ddwF1, ddwF2, ddwF3, ddwF4));
        m_mapUserDyeingInfo.insert(make_pair(ddwUserId, pstDyeingInfo));
    }

    fclose(fp);

    return 0;
}

