#include "process_bookmark.h"
#include "encode/src/encode/utf8_util.h"

TINT32 CProcessBookmark::ProcessCmd_BookmarkGet(SSession *pstSession, TBOOL &bNeedResponse)
{
    return 0;
}

TINT32 CProcessBookmark::ProcessCmd_BookmarkAdd(SSession *pstSession, TBOOL &bNeedResponse)
{
    TbBookmark *pstBookmark = NULL;

    // 0. 获取输入参数
    TUINT32 udwBookmarkPos = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TCHAR *pszBookmarkNick = pstSession->m_stReqParam.m_szKey[1];
    TUINT8	ucBookmarkType = atoi(pstSession->m_stReqParam.m_szKey[2]);
    TUINT32 udwSvrId = strtoul(pstSession->m_stReqParam.m_szKey[3], NULL, 10);

    TUINT64 uddwPos = udwSvrId;
    uddwPos = (uddwPos << 32) + udwBookmarkPos;

    // 1. check param
    if(pstSession->m_stUserInfo.m_udwBookmarkNum >= MAX_BOOKMARK_NUM)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__BOOK_MARK_OVERLOAD;  //其实这名字应该是overflow
        return -1;
    }
    for(TUINT32 idx = 0; idx < pstSession->m_stUserInfo.m_udwBookmarkNum; idx++)
    {
        pstBookmark = &pstSession->m_stUserInfo.m_atbBookmark[idx];
        if(pstBookmark->m_nPos == static_cast<TINT64>(uddwPos))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__BOOK_MARK_EXIST;
            return -2;
        }
    }

    // 2. add into list
    pstBookmark = &pstSession->m_stUserInfo.m_atbBookmark[pstSession->m_stUserInfo.m_udwBookmarkNum];

    pstBookmark->Reset();
    pstBookmark->Set_Uid(pstSession->m_stReqParam.m_udwUserId);
    pstBookmark->Set_Pos(uddwPos);
    pstBookmark->Set_Flag(ucBookmarkType);
    pstBookmark->Set_Nick(pszBookmarkNick);
    pstBookmark->Set_Time(CTimeUtils::GetUnixTime());
    pstSession->m_stUserInfo.m_aucBookMarkFlag[pstSession->m_stUserInfo.m_udwBookmarkNum] = EN_TABLE_UPDT_FLAG__NEW;
    pstSession->m_stUserInfo.m_udwBookmarkNum++;

    return 0;
}

TINT32 CProcessBookmark::ProcessCmd_BookmarkUpdate(SSession *pstSession, TBOOL &bNeedResponse)
{
    TUINT32 udwPos = 0;
    TCHAR szNick[1024];
    TUINT8 ucType = 0;
    TUINT32 udwSvrId = 0;
    TUINT64 uddwPosition = 0;

    TUINT32 idx = 0;
    TbBookmark *pstBookmark = NULL;
    TCHAR *pCur = NULL;
    TUINT32 udwNickLen = 0;
    TBOOL bBreak = FALSE;
    TUINT32 udwChangeNum = 0;

    // 0. 获取输入参数
    TCHAR *pPos = pstSession->m_stReqParam.m_szKey[0];
    TCHAR *pNick = pstSession->m_stReqParam.m_szKey[1];
    TCHAR *pType = pstSession->m_stReqParam.m_szKey[2];
    TCHAR *pSvrId = pstSession->m_stReqParam.m_szKey[3];

    // 1. process
    while(pPos && pNick && pType && pSvrId)
    {
        if(*pPos == ',' || *pPos == 0 || *pNick == ',' || *pNick == 0 || *pType == ',' || *pType == 0 || *pSvrId == ',' || *pSvrId == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_BookmarkUpdate: invalid param [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }

        pstBookmark = NULL;

        // pos
        udwPos = atoi(pPos);
        pPos = strchr(pPos, ',');
        if(pPos)
        {
            pPos++;
        }
        else
        {
            bBreak = TRUE;
        }

        // nick
        pCur = strchr(pNick, ',');
        if(pCur)
        {
            udwNickLen = pCur - pNick >= MAX_TABLE_NAME_LEN - 1 ? MAX_TABLE_NAME_LEN - 1 : pCur - pNick;
            strncpy(szNick, pNick, udwNickLen);
            szNick[udwNickLen] = 0;

            pNick = pCur + 1;
        }
        else
        {
            strncpy(szNick, pNick, 1024 - 1);
            szNick[1024 - 1] = 0;
            bBreak = TRUE;
        }

        CUtf8Util::RemoveBrokenPart(szNick);

        // type
        ucType = atoi(pType);
        pType = strchr(pType, ',');
        if(pType)
        {
            pType++;
        }
        else
        {
            bBreak = TRUE;
        }

        //svr id
        udwSvrId = atoi(pSvrId);
        pSvrId = strchr(pSvrId, ',');
        if(pSvrId)
        {
            pSvrId++;
        }
        else
        {
            bBreak = TRUE;
        }

        // update
        uddwPosition = udwSvrId;
        uddwPosition = uddwPosition << 32;
        uddwPosition += udwPos;
        for(idx = 0; idx < pstSession->m_stUserInfo.m_udwBookmarkNum; idx++)
        {
            if(pstSession->m_stUserInfo.m_atbBookmark[idx].m_nPos == static_cast<TINT64>(uddwPosition))
            {
                pstBookmark = &pstSession->m_stUserInfo.m_atbBookmark[idx];
                break;
            }
        }
        if(pstBookmark)
        {
            pstBookmark->Set_Nick(szNick);
            pstBookmark->Set_Flag(ucType);
            pstBookmark->Set_Time(CTimeUtils::GetUnixTime());
            pstSession->m_stUserInfo.m_aucBookMarkFlag[idx] = EN_TABLE_UPDT_FLAG__CHANGE;

            udwChangeNum++;
            if(udwChangeNum >= 20)
            {
                break;
            }
        }
        else
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_BookmarkUpdate: can't find corresponding bookmark [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }

        if(bBreak)
        {
            break;
        }
    }

    return 0;
}

TINT32 CProcessBookmark::ProcessCmd_BookmarkDelete(SSession *pstSession, TBOOL &bNeedResponse)
{
    TUINT32 udwPos = 0;
    TUINT32 udwSvrId = 0;
    TUINT64 uddwPosition = 0;

    TUINT32 idx = 0;
    TbBookmark *pstBookmark = NULL;
    TBOOL bBreak = FALSE;

    // 0. 获取输入参数
    TCHAR *pPos = pstSession->m_stReqParam.m_szKey[0];
    TCHAR *pSvrId = pstSession->m_stReqParam.m_szKey[1];

    // 1. process
    while(pPos && pSvrId)
    {
        if(*pPos == ',' || *pPos == 0 || *pSvrId == ',' || *pSvrId == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_BookmarkDelete: invalid param [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }

        pstBookmark = NULL;

        // pos
        udwPos = atoi(pPos);
        pPos = strchr(pPos, ',');
        if(pPos)
        {
            pPos++;
        }
        else
        {
            bBreak = TRUE;
        }

        //svr id
        udwSvrId = atoi(pSvrId);
        pSvrId = strchr(pSvrId, ',');
        if(pSvrId)
        {
            pSvrId++;
        }
        else
        {
            bBreak = TRUE;
        }

        // delete
        uddwPosition = udwSvrId;
        uddwPosition = uddwPosition << 32;
        uddwPosition += udwPos;
        for(idx = 0; idx < pstSession->m_stUserInfo.m_udwBookmarkNum; idx++)
        {
            if(pstSession->m_stUserInfo.m_atbBookmark[idx].m_nPos == static_cast<TINT64>(uddwPosition))
            {
                pstBookmark = &pstSession->m_stUserInfo.m_atbBookmark[idx];
                break;
            }
        }
        if(pstBookmark)
        {
            pstSession->m_stUserInfo.m_aucBookMarkFlag[idx] = EN_TABLE_UPDT_FLAG__DEL;
        }
        else
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_BookmarkDelete: can't find corresponding bookmark [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }

        if(bBreak)
        {
            break;
        }
    }

    return 0;
}


