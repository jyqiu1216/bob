#include "iap_white_list.h"
#include <math.h>

CIAPWhiteList* CIAPWhiteList::m_poCIAPWhiteList = NULL;


CIAPWhiteList* CIAPWhiteList::GetInstance()
{
    if(m_poCIAPWhiteList == NULL)
    {
        m_poCIAPWhiteList = new CIAPWhiteList;
    }

    return m_poCIAPWhiteList;
}

TINT32 CIAPWhiteList::Init(CTseLogger *poLog)
{
    if(poLog == NULL)
    {
        return -1;
    }

    // 0. set param
    m_poLog = poLog;

    TCHAR szFileName[1024];
    szFileName[0] = '\0';
    
    // 1.判断默认的配置文件，必须存在
    if(0 != access(DEFAULT_IAP_WHITE_LIST_FILE, F_OK))
    {
        assert(0);
    }
    

    // 2.加载配置文件
    TBOOL bUseDefault = FALSE;

    snprintf(szFileName, 1024, "%s.json", IAP_WHITE_LIST_FILE);
    if(0 != access(szFileName, F_OK))
    {
        TSE_LOG_ERROR(m_poLog, ("CIAPWhiteList::Init: file[%s] is not exist, use default file.", szFileName));
        bUseDefault = TRUE;
    }

    Json::Reader reader;
    std::ifstream is;
    if(bUseDefault)
    {
        is.open(DEFAULT_IAP_WHITE_LIST_FILE, std::ios::binary);
        if(reader.parse(is, m_oJsonRoot) == false)
        {
            // 默认文件也加载失败
            assert(0);
        }
        else
        {
            TSE_LOG_INFO(m_poLog, ("CIAPWhiteList::Init: parse default file[%s] success.", DEFAULT_IAP_WHITE_LIST_FILE));
            is.close();
        }
    }
    else
    {
        is.open(szFileName, std::ios::binary);
        Json::Value oJsonTmp;
        if(reader.parse(is, oJsonTmp) == false)
        {
            TSE_LOG_ERROR(m_poLog, ("CIAPWhiteList::Init: parse file[%s] failed.", szFileName));
            is.close();
        }
        else
        {
            m_oJsonRoot = oJsonTmp;
            TSE_LOG_INFO(m_poLog, ("CIAPWhiteList::Init: parse file[%s] success.", szFileName));
            is.close();
        }
    }       

    return 0;
}



TINT32 CIAPWhiteList::Update(CTseLogger *poLog)
{
    TINT32 dwRetCode = 0;
    CIAPWhiteList *poIapWhiteList = new CIAPWhiteList;
    CIAPWhiteList *poTmpRatingUserInfo = m_poCIAPWhiteList;

    dwRetCode = poIapWhiteList->Init(poLog);
    if (dwRetCode != 0)
    {
        TSE_LOG_ERROR(poLog, (" CIAPWhiteList::Update failed[%d]", dwRetCode));
        delete poIapWhiteList;
        return -1;
    }

    m_poCIAPWhiteList = poIapWhiteList;
    if (poTmpRatingUserInfo != NULL)
    {
        sleep(3);
        delete poTmpRatingUserInfo;
    }
    

    TSE_LOG_INFO(poLog, (" CIAPWhiteList::Update ok", dwRetCode));

    return 0;
}


TBOOL CIAPWhiteList::bIsWhitePlayer(TINT64 ddwUid, TFLOAT32 &fMaxWeekIap)
{
    fMaxWeekIap = 0.0;
    Json::Value::Members oUidMember = CIAPWhiteList::GetInstance()->m_oJsonRoot.getMemberNames();
    for(Json::Value::Members::iterator it = oUidMember.begin(); it != oUidMember.end();++it)
    {
        TINT64 ddwWhiteUid = atoi((*it).c_str());
        if(ddwWhiteUid == ddwUid)
        {
            fMaxWeekIap = CIAPWhiteList::GetInstance()->m_oJsonRoot[(*it)]["tmax"].asFloat();
            return TRUE;
        }
    }
    return FALSE;
}


