#include "rating_user.h"
#include <math.h>

CRatingUserInfo* CRatingUserInfo::m_poCRatingUserInfo = NULL;


CRatingUserInfo* CRatingUserInfo::GetInstance()
{
    if(m_poCRatingUserInfo == NULL)
    {
        m_poCRatingUserInfo = new CRatingUserInfo;
    }

    return m_poCRatingUserInfo;
}

TINT32 CRatingUserInfo::Init(CTseLogger *poLog )
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
    if(0 != access(DEFAULT_RATING_USER_INFO_EVENT_FILE, F_OK))
    {
        assert(0);
    }
    

    // 2.加载配置文件
    TBOOL bUseDefault = FALSE;

    snprintf(szFileName,1024,"%s.json", RATING_USER_INFO_EVENT_FILE);
    if(0 != access(szFileName, F_OK))
    {
        TSE_LOG_ERROR(m_poLog, ("CRatingUserInfo::Init: file[%s] is not exist, use default file.", szFileName));
        bUseDefault = TRUE;
    }

    Json::Reader reader;
    std::ifstream is;
    if(bUseDefault)
    {
        is.open(DEFAULT_RATING_USER_INFO_EVENT_FILE, std::ios::binary);
        if(reader.parse(is, m_oJsonRoot) == false)
        {
            // 默认文件也加载失败
            assert(0);
        }
        else
        {
            TSE_LOG_INFO(m_poLog, ("CRatingUserInfo::Init: parse default file[%s] success.", DEFAULT_RATING_USER_INFO_EVENT_FILE));
            is.close();
        }
    }
    else
    {
        is.open(szFileName, std::ios::binary);
        Json::Value oJsonTmp;
        if(reader.parse(is, oJsonTmp) == false)
        {
            TSE_LOG_ERROR(m_poLog, ("CRatingUserInfo::Init: parse file[%s] failed.", szFileName));
            is.close();
        }
        else
        {
            m_oJsonRoot = oJsonTmp;
            TSE_LOG_INFO(m_poLog, ("CRatingUserInfo::Init: parse file[%s] success.", szFileName));
            is.close();
        }
    }       

    return 0;
}



TINT32 CRatingUserInfo::Update(CTseLogger *poLog)
{
    TINT32 dwRetCode = 0;
    CRatingUserInfo *poRatingUserInfo = new CRatingUserInfo;
    CRatingUserInfo *poTmpRatingUserInfo = m_poCRatingUserInfo;

    dwRetCode = poRatingUserInfo->Init(poLog);
    if (dwRetCode != 0)
    {
        TSE_LOG_ERROR(poLog, (" CRatingUserInfo::Update failed[%d]", dwRetCode));
        delete poRatingUserInfo;
        return -1;
    }

    m_poCRatingUserInfo=poRatingUserInfo;
    if (poTmpRatingUserInfo != NULL)
    {
        sleep(3);
        delete poTmpRatingUserInfo;
    }
    

    TSE_LOG_INFO(poLog, (" CRatingUserInfo::Update ok", dwRetCode));

    return 0;
}





