#include "process_change_al_info.h"
#include "all_server.h"

TINT32 CProcessChangeAlInfo::requestHandler(SSession *pstSession)
{
    TINT32 ret = 0;

    TINT32 sid = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TINT32 aid = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TINT32 key = atoi(pstSession->m_stReqParam.m_szKey[2]);

    AllServer* all_server = AllServer::instance();
    if(!all_server)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NO_VALID_DOWNSTEAM;
        return -1;
    }

    SingleServer* target_server = all_server->getSingleServer(sid);
    if(!target_server)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NO_VALID_DOWNSTEAM;
        return -2;
    }

    if(key == 0)
    {
        TINT32 lang = atoi(pstSession->m_stReqParam.m_szKey[3]);
        target_server->changeAlLang(aid, lang);
    }
    else if(key == 1)
    {
        TINT32 policy = atoi(pstSession->m_stReqParam.m_szKey[3]);
        target_server->changeAlPolicy(aid, policy);
    }
    else if(key == 2)
    {
        string alname = pstSession->m_stReqParam.m_szKey[3];
        target_server->changeAlName(aid, alname);
    }
    else if(key == 3)
    {
        string alnick = pstSession->m_stReqParam.m_szKey[3];
        target_server->changeAlNick(aid, alnick);
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    genResponse(pstSession);
    return 0;
}

TVOID CProcessChangeAlInfo::genResponse(SSession *pstSession)
{
    pstSession->m_stCommonResInfo.m_jResultWriter = Json::Value(Json::objectValue);
    pstSession->m_stCommonResInfo.m_jResultWriter["ret_code"] = 0;
}
