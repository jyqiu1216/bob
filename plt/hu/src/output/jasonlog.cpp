#include <stdio.h>
#include "jasonlog.h"
#include "global_serv.h"

TVOID CJasonLog::OutLogForStat(SSession *pstSession, const TCHAR *pstBodyContent, TUINT32 udwBodyLen)
{
    if(pstSession->m_pTmpBuf == NULL)
    {
        return;
    }

    TCHAR *pszCur = pstSession->m_pTmpBuf;
    *pszCur = '\0';

    TUINT32 udwCurLen = 0;

    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    //1 req url
    udwCurLen = sprintf(pszCur, "\t%u\t%u\t%u\t%u\t%d\t%s\t%s\t", udwCurTime,
        pstSession->m_stReqParam.m_udwSvrId, pstSession->m_stReqParam.m_udwUserId, pstSession->m_stReqParam.m_udwAllianceId, 
        pstSession->m_stCommonResInfo.m_dwRetCode, pstSession->m_stReqParam.m_szCommand, pstSession->m_stReqParam.m_szReqUrl);
    pszCur += udwCurLen;

    //result
    udwCurLen = snprintf(pszCur, 200 << 10, "%s\t", pstBodyContent);
    pszCur += udwCurLen;

    udwCurLen = sprintf(pszCur, "[seq=%u]", pstSession->m_udwSeqNo);
    pszCur += udwCurLen;

    TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("JASON"));

    // output
    TSE_LOG_HOUR(CGlobalServ::m_poJasonLog, ("%s", pstSession->m_pTmpBuf));
}
