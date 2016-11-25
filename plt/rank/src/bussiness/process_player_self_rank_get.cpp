#include "process_player_self_rank_get.h"

#include "all_server.h"

TINT32 CProcessPlayerSelfRankGet::requestHandler(SSession* pstSession)
{
    TINT32 ret = 0;

    TINT32 sid = pstSession->m_stReqParam.m_udwSvrId;
    TINT32 self_uid = pstSession->m_stReqParam.m_udwUserId;

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;

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

        pstSession->m_stCommonResInfo.m_oComputeTime.ddwTime = target_server->update_time;
        for (TINT32 type_index = 0; type_index < sizeof(kPlayerRankTypeOrders) / sizeof(kPlayerRankTypeOrders[0]); ++type_index)
        {
            target_server->getPlayerRank(kPlayerRankTypeOrders[type_index], self_uid, &pstSession->m_stCommonResInfo.player_rank_infos[type_index]);
        }
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;

        WrapPlayerSelfRankJson(pstSession);
    }

    return 0;
}

TVOID CProcessPlayerSelfRankGet::WrapPlayerSelfRankJson(SSession *pstSession)
{
    TINT32 self_uid = pstSession->m_stReqParam.m_udwUserId;
    Json::Value& jRootWriter = pstSession->m_stCommonResInfo.m_jResultWriter;

    jRootWriter = Json::Value(Json::objectValue);
    jRootWriter["uid"] = self_uid;
    jRootWriter["compute_time"] = pstSession->m_stCommonResInfo.m_oComputeTime.ddwTime;
    jRootWriter["self_rank"] = Json::Value(Json::arrayValue);

    for (TINT32 type_index = 0; type_index < sizeof(kPlayerRankTypeOrders) / sizeof(kPlayerRankTypeOrders[0]); ++type_index)
    {
        int uid = pstSession->m_stCommonResInfo.player_rank_infos[type_index].uid;
        int type = pstSession->m_stCommonResInfo.player_rank_infos[type_index].rank_type;
        int rank = pstSession->m_stCommonResInfo.player_rank_infos[type_index].rank;
        int value = pstSession->m_stCommonResInfo.player_rank_infos[type_index].value;

        if (uid != self_uid || type != kPlayerRankTypeOrders[type_index])
        {
            type = kPlayerRankTypeOrders[type_index];
            rank = 0;
            value = 0;
        }

        jRootWriter["self_rank"][type_index] = Json::Value(Json::objectValue);
        jRootWriter["self_rank"][type_index]["type"] = type;
        jRootWriter["self_rank"][type_index]["rank"] = rank;
        jRootWriter["self_rank"][type_index]["value"] = value;
    }
}
