#include "process_get_al_rank_by_id.h"
#include "all_server.h"

TINT32 CProcessGetAlRankById::requestHandler(SSession *pstSession)
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

    target_server->getAlRank(aid, &pstSession->m_stCommonResInfo.self_al_rank_info);

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    genResponse(pstSession);
    return 0;
}

TVOID CProcessGetAlRankById::genResponse(SSession *pstSession)
{
    Json::Value& jRootWriter = pstSession->m_stCommonResInfo.m_jResultWriter;
    jRootWriter = Json::Value(Json::objectValue);
    jRootWriter["alinfo"] = Json::Value(Json::objectValue);
    jRootWriter["alinfo"]["aid"] = pstSession->m_stCommonResInfo.self_al_rank_info.aid;
    jRootWriter["alinfo"]["alname"] = pstSession->m_stCommonResInfo.self_al_rank_info.alname;
    jRootWriter["alinfo"]["alnick"] = pstSession->m_stCommonResInfo.self_al_rank_info.alnick;
    jRootWriter["alinfo"]["force_rank"] = pstSession->m_stCommonResInfo.self_al_rank_info.rank3;
    jRootWriter["alinfo"]["troop_kill_rank"] = pstSession->m_stCommonResInfo.self_al_rank_info.rank2;
}
