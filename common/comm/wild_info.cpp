#include "wild_info.h"
#include "game_info.h"

CWildInfo* CWildInfo::m_pWildInfo = NULL;
TBOOL CWildInfo::m_bHasInitedWildRes = FALSE;

CWildInfo* CWildInfo::GetInstance()
{
    if(m_pWildInfo == NULL)
    {
        m_pWildInfo = new CWildInfo;
    }

    return m_pWildInfo;
}

TINT32 CWildInfo::Update(CTseLogger *poLog)
{
    return CWildInfo::GetInstance()->LoadWildRes(poLog);
}

TINT32 CWildInfo::LoadWildRes(CTseLogger *poLog)
{
    TCHAR szFileName[1024];
    szFileName[0] = '\0';
    Json::Reader reader;
    Json::Value oJsonTmp;
    std::ifstream is;

    if(!m_bHasInitedWildRes)
    {
        m_pjsMajorWildRes = m_jsWildResA;
        m_pjsBufferWildRes = m_jsWildResB;
        m_bHasInitedWildRes = TRUE;
    }

    for(TUINT32 udwIdx = 0; udwIdx < MAX_SVR_NUM; udwIdx++)
    {    
        TBOOL bUseDefault = FALSE;

        snprintf(szFileName, 1024, "%s.%u.json", WILD_RES_JSON_FILE, udwIdx);
        if(0 != access(szFileName, F_OK))
        {
            TSE_LOG_INFO(poLog, ("CWildInfo::Init: file[%s] is not exist, use default file.", szFileName));
            bUseDefault = TRUE;
        }

        if(bUseDefault)
        {
            is.open(DEFAULT_WILD_RES_JSON_FILE, std::ios::binary);
            if(reader.parse(is, m_pjsBufferWildRes[udwIdx]) == false)
            {
                // Ä¬ÈÏÎÄ¼þÒ²¼ÓÔØÊ§°Ü
                assert(0);
            }
            else
            {
                TSE_LOG_INFO(poLog, ("CWildInfo::Init: parse default file[%s] success.", DEFAULT_WILD_RES_JSON_FILE));
                is.close();
            }
        
        }
        else
        {

            oJsonTmp.clear();
            is.open(szFileName, std::ios::binary);
            if(reader.parse(is, oJsonTmp) == false)
            {
                TSE_LOG_ERROR(poLog, ("LoadWildRes:: svr %u parse file[%s] failed.", udwIdx, szFileName));
                is.close();
                if(!m_bHasInitedWildRes)
                {
                    return -2;// stop the init process
                }
                else
                {
                    TSE_LOG_ERROR(poLog, ("LoadWildRes:: update all svr's wild res json failed. Use last time's~"));
                    return -3;
                }
            }
            else
            {
                m_pjsBufferWildRes[udwIdx].clear();
                m_pjsBufferWildRes[udwIdx] = oJsonTmp;
                TSE_LOG_INFO(poLog, ("LoadWildRes:: svr %u parse file[%s] success.", udwIdx, szFileName));
                is.close();
            }
        }

    }

    TSE_LOG_INFO(poLog, ("LoadWildRes:: update all svr's wild res json success."));
    std::swap(m_pjsBufferWildRes, m_pjsMajorWildRes);

    return 0;
}

const Json::Value& CWildInfo::GetWildResInfo(TUINT32 udwSvrId)
{
    assert(udwSvrId <= MAX_SVR_NUM);
    return CWildInfo::GetInstance()->m_pjsMajorWildRes[udwSvrId]["data"]["wild_res"];
}


void CWildInfo::Wild_Output( TCHAR *pszOut, TUINT32 &udwOutLen, TUINT32 udwSid,TUINT32 udwType,TUINT32 udwLevel)
{
    TCHAR szGameIdx[256];
	snprintf(szGameIdx,256,"%u",udwType);

    TCHAR *PszCur = pszOut;

    TUINT32 udwLen = 0;
    if (MAX_SVR_NUM<=udwSid)
    {
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("Wild_Output::param err[udwSid=%u],so set sid=0!", udwSid));
        udwSid = 0;
    }

    //base
    udwLen = sprintf(PszCur, "\"basic\":");
    PszCur += udwLen;

    Json::FastWriter oJsonWriter ;
    
    const Json::Value &oBaseJaon =m_pjsMajorWildRes[udwSid]["data"]["wild_res"][szGameIdx]["a0"];
    udwLen = sprintf(PszCur, "%s",oBaseJaon.toStyledString().c_str());
    PszCur += udwLen;

    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("base:%s",oJsonWriter.write(oBaseJaon).c_str()));

    //reward
    udwLen = sprintf(PszCur, ",\"reward\":");
    PszCur += udwLen;

    const Json::Value &oRewardJaon = m_pjsMajorWildRes[udwSid]["data"]["wild_res"][szGameIdx]["a1"][udwLevel-1]["a0"];
    udwLen = sprintf(PszCur, "%s",oRewardJaon.toStyledString().c_str());
    PszCur += udwLen;

    //2.8 new reward
    udwLen = sprintf(PszCur, ",\"new_reward\":");
    PszCur += udwLen;

    const Json::Value &oRewardJaonNew = m_pjsMajorWildRes[udwSid]["data"]["wild_res"][szGameIdx]["a1"][udwLevel-1]["a3"];
    udwLen = sprintf(PszCur, "%s",oRewardJaonNew.toStyledString().c_str());
    PszCur += udwLen;

    //ak_type
    udwLen = sprintf(PszCur, ",\"ak_type\":");
    PszCur += udwLen;

    
    const Json::Value &oAkTypeJaon =m_pjsMajorWildRes[udwSid]["data"]["wild_res"][szGameIdx]["a3"];
    udwLen = sprintf(PszCur, "%s",oAkTypeJaon.toStyledString().c_str());
    PszCur += udwLen;

    //ui
    udwLen = sprintf(PszCur, ",\"ui\":");
    PszCur += udwLen;

    
    const Json::Value &oUiJaon =m_pjsMajorWildRes[udwSid]["data"]["wild_res"][szGameIdx]["a6"];
    udwLen = sprintf(PszCur, "%s",oUiJaon.toStyledString().c_str());
    PszCur += udwLen;
    
    udwOutLen = PszCur - pszOut;
    
}

