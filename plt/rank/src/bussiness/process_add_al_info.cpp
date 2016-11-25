#include "process_add_al_info.h"
#include "all_server.h"

TINT32 CProcessAddAlInfo::requestHandler(SSession *pstSession)
{
    TINT32 ret = 0;

    TINT32 sid = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TINT32 aid = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TINT32 policy = atoi(pstSession->m_stReqParam.m_szKey[2]);
    string alname = pstSession->m_stReqParam.m_szKey[3];
    string alnick = pstSession->m_stReqParam.m_szKey[4];
    TINT64 value3 = strtoull(pstSession->m_stReqParam.m_szKey[5], NULL, 10);
    TINT64 value9 = strtoull(pstSession->m_stReqParam.m_szKey[6], NULL, 10);
    TINT32 new_player_flag = atoi(pstSession->m_stReqParam.m_szKey[7]);

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

    if(target_server->addAlRank(aid, policy, alname, alnick, value3, value9, new_player_flag) != 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -3;
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    genResponse(pstSession);
    return 0;
}



TVOID CProcessAddAlInfo::genResponse(SSession *pstSession)
{
    pstSession->m_stCommonResInfo.m_jResultWriter = Json::Value(Json::objectValue);
    pstSession->m_stCommonResInfo.m_jResultWriter["ret_code"] = 0;
}

