#include "process_del_al_info.h"
#include "all_server.h"

TINT32 CProcessDelAlInfo::requestHandler(SSession *pstSession)
{
    TINT32 ret = 0;

    TINT32 sid = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TINT32 aid = atoi(pstSession->m_stReqParam.m_szKey[1]);

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

    if(target_server->delAlRank(aid) != 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -3;
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    genResponse(pstSession);
    return 0;
}



TVOID CProcessDelAlInfo::genResponse(SSession *pstSession)
{
    pstSession->m_stCommonResInfo.m_jResultWriter = Json::Value(Json::objectValue);
    pstSession->m_stCommonResInfo.m_jResultWriter["ret_code"] = 0;
}

