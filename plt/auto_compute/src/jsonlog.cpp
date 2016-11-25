#include <stdio.h>
#include "jsonlog.h"

CTseLogger *CJsonLog::m_poJsonLog = NULL;

TVOID CJsonLog::Init(CTseLogger *poJsonLog)
{
    m_poJsonLog = poJsonLog;
}

TVOID CJsonLog::OutLogForStat(SSession *pstSession, SUserInfo *pstUser, const TCHAR *pstBodyContent)
{
    assert(m_poJsonLog != NULL);

    if (pstSession->m_pTmpBuf == NULL)
    {
        return;
    }

    TCHAR *pszCur = pstSession->m_pTmpBuf;
    *pszCur = '\0';

    TUINT32 udwCurLen = 0;

    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    TUINT32 udwAlid = pstUser->m_tbPlayer.m_nAlpos == EN_ALLIANCE_POS__REQUEST ? 0 : pstUser->m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET;

    //1 req url
    udwCurLen = sprintf(pszCur, "\t%u\t%ld\t%ld\t%u\t%d\t", udwCurTime,
        pstUser->m_tbPlayer.m_nSid, pstUser->m_tbPlayer.m_nUid, udwAlid, 
        pstSession->m_stCommonResInfo.m_dwRetCode);
    pszCur += udwCurLen;

    switch (pstSession->m_stActionTable.dwTableType)
    {
    case EN_UID_ACTION:
        udwCurLen = sprintf(pszCur, "%ld\t%ld,%u,%ld,%u,%ld,%u\t", pstSession->m_stReqAction.m_nId,
            pstSession->m_stReqAction.m_nMclass, pstSession->m_ucActionRawSecClass, pstSession->m_stReqAction.m_nSclass,
            pstSession->m_ucActionRawStatus, pstSession->m_stReqAction.m_nStatus, pstSession->m_ucReqActionFlag);
        pszCur += udwCurLen;
        break;
    case EN_AID_ACTION:
        udwCurLen = sprintf(pszCur, "%ld\t%ld,%u,%ld,%u,%ld,%u\t", pstSession->m_stReqAlAction.m_nId,
            pstSession->m_stReqAlAction.m_nMclass, pstSession->m_ucActionRawSecClass, pstSession->m_stReqAlAction.m_nSclass,
            pstSession->m_ucActionRawStatus, pstSession->m_stReqAlAction.m_nStatus, pstSession->m_ucReqAlActionFlag);
        pszCur += udwCurLen;
        break;
    case EN_MARCH_ACTION:
        udwCurLen = sprintf(pszCur, "%ld\t%ld,%u,%ld,%u,%ld,%u\t", pstSession->m_stReqMarch.m_nId,
            pstSession->m_stReqMarch.m_nMclass, pstSession->m_ucActionRawSecClass, pstSession->m_stReqMarch.m_nSclass,
            pstSession->m_ucActionRawStatus, pstSession->m_stReqMarch.m_nStatus, pstSession->m_ucReqMarchFlag);
        pszCur += udwCurLen;
        break;
    default:
        udwCurLen = sprintf(pszCur, "\t\t");
        pszCur += udwCurLen;
    }
    
    //result
    udwCurLen = snprintf(pszCur, 200 << 10, "%s\t", pstBodyContent);
    pszCur += udwCurLen;

    udwCurLen = sprintf(pszCur, "[seq=%u]", pstSession->m_udwSeqNo);
    pszCur += udwCurLen;

    // output
    TSE_LOG_HOUR(m_poJsonLog, ("%s", pstSession->m_pTmpBuf));
}
