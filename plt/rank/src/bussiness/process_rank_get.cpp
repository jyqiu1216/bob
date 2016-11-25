#include "aws_table_common.h"
#include "game_define.h"
#include "global_serv.h"
#include "procedure_base.h"
#include "time_utils.h"
#include "alliance_rank.h"
#include "process_rank_get.h"
#include "alliance_mapping.h"
#include "common_func.h"
#include "all_server.h"

TINT32 CProcessRankGet::requestHandler(SSession* pstSession)
{
    TINT32 ret = 0;

    //offset
    const TINT32 kCount = 100;
    TINT32 page = pstSession->m_stReqParam.m_udwPage;
    TINT32 offset = (page - 1) * kCount;
    if(offset < 0)
    {
        offset = 0;
    }

    //target rank type
    TINT32 rank_type = atoi(pstSession->m_stReqParam.m_szKey[0]);

    //search keyword
    const TINT32  kMaxBufferSize = 256;
    TCHAR keyword[kMaxBufferSize];
    keyword[0] = '\0';
    snprintf(keyword, kMaxBufferSize, "%s", pstSession->m_stReqParam.m_szKey[1]);

    TINT32 sid = pstSession->m_stReqParam.m_udwSvrId;
    TINT32 self_aid = pstSession->m_stReqParam.m_udwAllianceId;
    TINT32 self_uid = pstSession->m_stReqParam.m_udwUserId;

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_PROCEDURE__EXPERT_NODE_LOCAL_MYSQL;

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

        //联盟排行
        if(0 == strcmp(keyword, "") && rank_type > EN_RANK_TYPE_ALLIANCE_CHAMPION)
        {
            pstSession->m_stCommonResInfo.m_oComputeTime.ddwTime = target_server->update_time;
            pstSession->m_stCommonResInfo.result_count = 0;
            target_server->getTopAlRank(rank_type, pstSession->m_stCommonResInfo.al_rank_infos, pstSession->m_stCommonResInfo.result_count);
            target_server->getAlRank(rank_type, self_aid, &pstSession->m_stCommonResInfo.self_al_rank_info);
            pstSession->m_stCommonResInfo.total_count = pstSession->m_stCommonResInfo.result_count;
        }
        else if(0 != strcmp(keyword, "") && rank_type > EN_RANK_TYPE_ALLIANCE_CHAMPION)
        {
        }
        else if(rank_type == EN_RANK_TYPE_ALLIANCE_CHAMPION)
        {
            pstSession->m_stCommonResInfo.m_oComputeTime.ddwTime = target_server->update_time;
            pstSession->m_stCommonResInfo.result_count = 0;
            for(TINT32 type_index = 0; type_index < sizeof(kAlRankTypeOrder) / sizeof(kAlRankTypeOrder[0]); ++type_index)
            {
                if(target_server->getTopSingleAlRank(kAlRankTypeOrder[type_index], &pstSession->m_stCommonResInfo.al_rank_infos[type_index]) == 0)
                {
                    pstSession->m_stCommonResInfo.result_count++;
                }
            }
            pstSession->m_stCommonResInfo.total_count = pstSession->m_stCommonResInfo.result_count;
        }
        //玩家排行
        else if(0 == strcmp(keyword, "") && (rank_type > EN_RANK_TYPE_PLAYER_CHAMPION && rank_type < EN_RANK_TYPE_ALLIANCE_CHAMPION))
        {
            pstSession->m_stCommonResInfo.m_oComputeTime.ddwTime = target_server->update_time;
            pstSession->m_stCommonResInfo.result_count = 0;
            target_server->getTopPlayerRank(rank_type, pstSession->m_stCommonResInfo.player_rank_infos, pstSession->m_stCommonResInfo.result_count);
            target_server->getPlayerRank(rank_type, self_uid, &pstSession->m_stCommonResInfo.self_player_rank_info);
            pstSession->m_stCommonResInfo.total_count = pstSession->m_stCommonResInfo.result_count;
        }
        else if(0 != strcmp(keyword, "") && (rank_type > EN_RANK_TYPE_PLAYER_CHAMPION && rank_type < EN_RANK_TYPE_ALLIANCE_CHAMPION))
        {
            target_server->NewsearchPlayer(rank_type, keyword, offset, pstSession->m_stCommonResInfo.result_count, pstSession->m_stCommonResInfo.total_count, pstSession->m_stCommonResInfo.player_rank_infos);
        }
        else if(rank_type == EN_RANK_TYPE_PLAYER_CHAMPION)
        {
            pstSession->m_stCommonResInfo.m_oComputeTime.ddwTime = target_server->update_time;
            pstSession->m_stCommonResInfo.result_count = 0;
            for(TINT32 type_index = 0; type_index < sizeof(kPlayerRankTypeOrders) / sizeof(kPlayerRankTypeOrders[0]); ++type_index)
            {
                if(target_server->getTopSinglePlayerRank(kPlayerRankTypeOrders[type_index], &pstSession->m_stCommonResInfo.player_rank_infos[type_index]) == 0)
                {
                    pstSession->m_stCommonResInfo.result_count++;
                }
            }
            pstSession->m_stCommonResInfo.total_count = pstSession->m_stCommonResInfo.result_count;
        }
        else
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -3;
        }

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        pstSession->ResetAwsInfo();

        if(rank_type >= EN_RANK_TYPE_ALLIANCE_CHAMPION)
        {
            TbAlliance tmp_alliance;
            TBOOL is_find_self = FALSE;
            for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_stCommonResInfo.result_count; ++udwIdx)
            {
                tmp_alliance.Reset();
                tmp_alliance.Set_Aid(pstSession->m_stCommonResInfo.al_rank_infos[udwIdx].aid);
                CAwsRequest::GetItem(pstSession, &tmp_alliance, ETbALLIANCE_OPEN_TYPE_PRIMARY);

                if(pstSession->m_stCommonResInfo.self_al_rank_info.aid ==
                   pstSession->m_stCommonResInfo.al_rank_infos[udwIdx].aid)
                {
                    is_find_self = TRUE;
                }
            }

            if(is_find_self == FALSE &&
               rank_type > EN_RANK_TYPE_ALLIANCE_CHAMPION &&
               pstSession->m_stCommonResInfo.self_al_rank_info.aid > 0)
            {
                tmp_alliance.Reset();
                tmp_alliance.Set_Aid(pstSession->m_stCommonResInfo.self_al_rank_info.aid);
                CAwsRequest::GetItem(pstSession, &tmp_alliance, ETbALLIANCE_OPEN_TYPE_PRIMARY);
            }
        }
        else if(rank_type >= EN_RANK_TYPE_PLAYER_CHAMPION)
        {
            TbPlayer tmp_player;
            TBOOL is_find_self = FALSE;
            for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_stCommonResInfo.result_count; ++udwIdx)
            {
                tmp_player.Reset();
                tmp_player.Set_Uid(pstSession->m_stCommonResInfo.player_rank_infos[udwIdx].uid);
                CAwsRequest::GetItem(pstSession, &tmp_player, ETbPLAYER_OPEN_TYPE_PRIMARY);

                if(pstSession->m_stCommonResInfo.self_player_rank_info.uid ==
                   pstSession->m_stCommonResInfo.player_rank_infos[udwIdx].uid)
                {
                    is_find_self = TRUE;
                }
            }

            if(is_find_self == FALSE &&
               rank_type > EN_RANK_TYPE_PLAYER_CHAMPION &&
               pstSession->m_stCommonResInfo.self_player_rank_info.uid > 0)
            {
                tmp_player.Reset();
                tmp_player.Set_Uid(pstSession->m_stCommonResInfo.self_player_rank_info.uid);
                CAwsRequest::GetItem(pstSession, &tmp_player, ETbPLAYER_OPEN_TYPE_PRIMARY);
            }
        }

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        pstSession->m_udwExpectProcedure = EN_PROCEDURE__EXPERT_NODE_AWS;
        // send request
        ret = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(ret == -1)
        {
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        }
        else if(ret < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NO_VALID_DOWNSTEAM;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("RankGet: send req failed [seq=%u]", pstSession->m_udwSeqNo));
            return -6;
        }
        else
        {
            return 0;
        }
    }

    //4 update map相关信息
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        if(rank_type > EN_RANK_TYPE_ALLIANCE_CHAMPION)
        {
            NewWrapAlRankJson(pstSession);
        }
        else if(rank_type == EN_RANK_TYPE_ALLIANCE_CHAMPION)
        {
            NewWrapAlChampionJson(pstSession);
        }
        else if(rank_type > EN_RANK_TYPE_PLAYER_CHAMPION && rank_type < EN_RANK_TYPE_ALLIANCE_CHAMPION)
        {
            NewWrapPlayerRankJson(pstSession);
        }
        else if(rank_type == EN_RANK_TYPE_PLAYER_CHAMPION)
        {
            NewWrapPlayerChampionJson(pstSession);
        }
        return 0;
    }

    return 0;
}

TVOID CProcessRankGet::NewWrapAlRankJson(SSession *pstSession)
{
    TINT32 rank_type = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TINT32 self_aid = pstSession->m_stReqParam.m_udwAllianceId;
    Json::Value& jRootWriter = pstSession->m_stCommonResInfo.m_jResultWriter;

    jRootWriter["svr_alliance_rank"] = Json::Value(Json::objectValue);
    jRootWriter["svr_alliance_rank"]["compute_time"] = pstSession->m_stCommonResInfo.m_oComputeTime.ddwTime;
    jRootWriter["svr_alliance_rank"]["rank_list"] = Json::Value(Json::arrayValue);

    TINT32 dwRankNum = 0;
    TINT32 dwSelectNum = pstSession->m_stCommonResInfo.result_count;
    for(dwRankNum = 0; dwRankNum < dwSelectNum; dwRankNum++)
    {
        TINT32 aid = pstSession->m_stCommonResInfo.al_rank_infos[dwRankNum].aid;
        TINT32 rank = pstSession->m_stCommonResInfo.al_rank_infos[dwRankNum].rank;
        TINT64 value = pstSession->m_stCommonResInfo.al_rank_infos[dwRankNum].value;

        TBOOL bFindInDynamoDB = FALSE;
        for(TINT32 dwAwsNum = 0; dwAwsNum < pstSession->m_vecAwsRsp.size(); ++dwAwsNum)
        {
            TbAlliance tbAlliance;
            TINT32 dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[dwAwsNum], &tbAlliance);
            if(dwRetCode < 0)
            {
                continue;
            }
            if(tbAlliance.m_nAid == aid)
            {
                jRootWriter["svr_alliance_rank"]["rank_list"][dwRankNum]["type"] = Json::Value(rank_type);
                jRootWriter["svr_alliance_rank"]["rank_list"][dwRankNum]["rank"] = Json::Value(rank);
                jRootWriter["svr_alliance_rank"]["rank_list"][dwRankNum]["value"] = value > 0 ? value : 0;
                jRootWriter["svr_alliance_rank"]["rank_list"][dwRankNum]["extra_info"]["aid"] = Json::Value(tbAlliance.m_nAid);
                jRootWriter["svr_alliance_rank"]["rank_list"][dwRankNum]["extra_info"]["alname"] = Json::Value(tbAlliance.Get_Name());
                jRootWriter["svr_alliance_rank"]["rank_list"][dwRankNum]["extra_info"]["al_nick_name"] = tbAlliance.m_sAl_nick_name;

                bFindInDynamoDB = TRUE;
                break;
            }
        }

        if(bFindInDynamoDB == FALSE)
        {
            jRootWriter["svr_alliance_rank"]["rank_list"][dwRankNum]["type"] = Json::Value(rank_type);
            jRootWriter["svr_alliance_rank"]["rank_list"][dwRankNum]["rank"] = Json::Value(rank);
            jRootWriter["svr_alliance_rank"]["rank_list"][dwRankNum]["value"] = value > 0 ? value : 0;
            jRootWriter["svr_alliance_rank"]["rank_list"][dwRankNum]["extra_info"]["aid"] = aid;
            jRootWriter["svr_alliance_rank"]["rank_list"][dwRankNum]["extra_info"]["alname"] =
                pstSession->m_stCommonResInfo.al_rank_infos[dwRankNum].alname;
            jRootWriter["svr_alliance_rank"]["rank_list"][dwRankNum]["extra_info"]["al_nick_name"] =
                pstSession->m_stCommonResInfo.al_rank_infos[dwRankNum].alnick;
        }
    }

    TBOOL bHasFindSelf = FALSE;
    if(self_aid > 0 && pstSession->m_stCommonResInfo.self_al_rank_info.aid == self_aid)
    {
        TINT32 rank = pstSession->m_stCommonResInfo.self_al_rank_info.rank;
        TINT64 value = pstSession->m_stCommonResInfo.self_al_rank_info.value;

        for(TINT32 dwAwsNum = 0; dwAwsNum < pstSession->m_vecAwsRsp.size(); ++dwAwsNum)
        {
            TbAlliance tbAlliance;
            TINT32 dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[dwAwsNum], &tbAlliance);
            if(dwRetCode < 0)
            {
                continue;
            }
            if(tbAlliance.m_nAid == self_aid)
            {
                jRootWriter["svr_alliance_rank"]["self_rank"] = Json::Value(Json::objectValue);
                jRootWriter["svr_alliance_rank"]["self_rank"]["type"] = Json::Value(rank_type);
                jRootWriter["svr_alliance_rank"]["self_rank"]["rank"] = rank;
                jRootWriter["svr_alliance_rank"]["self_rank"]["value"] = value > 0 ? value : 0;
                jRootWriter["svr_alliance_rank"]["self_rank"]["extra_info"]["aid"] = Json::Value(tbAlliance.m_nAid);
                jRootWriter["svr_alliance_rank"]["self_rank"]["extra_info"]["alname"] = tbAlliance.m_sName;
                jRootWriter["svr_alliance_rank"]["self_rank"]["extra_info"]["al_nick_name"] = tbAlliance.m_sAl_nick_name;

                bHasFindSelf = TRUE;
                break;
            }
        }
    }

    if(!bHasFindSelf)
    {
        jRootWriter["svr_alliance_rank"]["self_rank"] = Json::Value(Json::objectValue);
        jRootWriter["svr_alliance_rank"]["self_rank"]["type"] = Json::Value(rank_type);
        jRootWriter["svr_alliance_rank"]["self_rank"]["rank"] = Json::Value(0);
        jRootWriter["svr_alliance_rank"]["self_rank"]["value"] = Json::Value(0);
        jRootWriter["svr_alliance_rank"]["self_rank"]["extra_info"]["aid"] = Json::Value(0);
        jRootWriter["svr_alliance_rank"]["self_rank"]["extra_info"]["alname"] = "";
        jRootWriter["svr_alliance_rank"]["self_rank"]["extra_info"]["al_nick_name"] = "";
    }

    jRootWriter["svr_alliance_rank"]["total_num"] = Json::Value(1);
    jRootWriter["svr_alliance_rank"]["cur_page_num"] = Json::Value(dwRankNum);
}

TVOID CProcessRankGet::NewWrapAlChampionJson(SSession *pstSession)
{
    Json::Value& jRootWriter = pstSession->m_stCommonResInfo.m_jResultWriter;

    jRootWriter["svr_alliance_champion"] = Json::Value(Json::objectValue);
    jRootWriter["svr_alliance_champion"]["compute_time"] = pstSession->m_stCommonResInfo.m_oComputeTime.ddwTime;
    jRootWriter["svr_alliance_champion"]["rank_list"] = Json::Value(Json::arrayValue);

    TINT32 dwRankNum = 0;
   // TINT32 dwSelectNum = pstSession->m_stCommonResInfo.result_count;
    TINT32 dwSelectNum = sizeof(kAlRankTypeOrder) / sizeof(kAlRankTypeOrder[0]);
    for(dwRankNum = 0; dwRankNum < dwSelectNum; dwRankNum++)
    {
        TINT32 rank_type = pstSession->m_stCommonResInfo.al_rank_infos[dwRankNum].rank_type;
        TINT32 aid = pstSession->m_stCommonResInfo.al_rank_infos[dwRankNum].aid;
        TINT32 rank = pstSession->m_stCommonResInfo.al_rank_infos[dwRankNum].rank;
        TINT64 value = pstSession->m_stCommonResInfo.al_rank_infos[dwRankNum].value;
        if (aid == 0)
        {
            rank_type = kAlRankTypeOrder[dwRankNum];
        }

        TBOOL bFindInDynamoDB = FALSE;
        for(TINT32 udwAwsIndex = 0; udwAwsIndex < pstSession->m_vecAwsRsp.size(); ++udwAwsIndex)
        {
            TbAlliance tbAlliance;
            TINT32 dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[udwAwsIndex], &tbAlliance);
            if(dwRetCode < 0)
            {
                continue;
            }
            if(aid == tbAlliance.m_nAid)
            {
                jRootWriter["svr_alliance_champion"]["rank_list"][dwRankNum]["type"] = rank_type;
                jRootWriter["svr_alliance_champion"]["rank_list"][dwRankNum]["rank"] = rank;
                jRootWriter["svr_alliance_champion"]["rank_list"][dwRankNum]["value"] = value > 0 ? value : 0;
                jRootWriter["svr_alliance_champion"]["rank_list"][dwRankNum]["extra_info"]["aid"] = tbAlliance.m_nAid;
                jRootWriter["svr_alliance_champion"]["rank_list"][dwRankNum]["extra_info"]["alname"] = tbAlliance.m_sName;
                jRootWriter["svr_alliance_champion"]["rank_list"][dwRankNum]["extra_info"]["al_nick_name"] = tbAlliance.m_sAl_nick_name;
                bFindInDynamoDB = TRUE;
                break;
            }
        }

        if(bFindInDynamoDB == FALSE)
        {
            jRootWriter["svr_alliance_champion"]["rank_list"][dwRankNum]["type"] = rank_type;
            jRootWriter["svr_alliance_champion"]["rank_list"][dwRankNum]["rank"] = rank;
            jRootWriter["svr_alliance_champion"]["rank_list"][dwRankNum]["value"] = value > 0 ? value : 0;
            jRootWriter["svr_alliance_champion"]["rank_list"][dwRankNum]["extra_info"]["aid"] = aid;
            jRootWriter["svr_alliance_champion"]["rank_list"][dwRankNum]["extra_info"]["alname"] =
                pstSession->m_stCommonResInfo.al_rank_infos[dwRankNum].alname;
            jRootWriter["svr_alliance_champion"]["rank_list"][dwRankNum]["extra_info"]["al_nick_name"] =
                pstSession->m_stCommonResInfo.al_rank_infos[dwRankNum].alnick;
        }
    }
}

//*************************PLAYER************************************

TVOID CProcessRankGet::NewWrapPlayerRankJson(SSession *pstSession)
{
    TINT32 rank_type = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TINT32 self_uid = pstSession->m_stReqParam.m_udwUserId;
    Json::Value& jRootWriter = pstSession->m_stCommonResInfo.m_jResultWriter;

    jRootWriter["svr_player_rank"] = Json::Value(Json::objectValue);
    jRootWriter["svr_player_rank"]["compute_time"] = pstSession->m_stCommonResInfo.m_oComputeTime.ddwTime;
    jRootWriter["svr_player_rank"]["rank_list"] = Json::Value(Json::arrayValue);

    TINT32 dwRankNum = 0;
    TINT32 dwSelectNum = pstSession->m_stCommonResInfo.result_count;
    for(dwRankNum = 0; dwRankNum < dwSelectNum; dwRankNum++)
    {
        TINT32 uid = pstSession->m_stCommonResInfo.player_rank_infos[dwRankNum].uid;
        TINT32 rank = pstSession->m_stCommonResInfo.player_rank_infos[dwRankNum].rank;
        TINT64 value = pstSession->m_stCommonResInfo.player_rank_infos[dwRankNum].value;

        TBOOL bFindInDynamoDB = FALSE;
        for(TINT32 dwNum = 0; dwNum < pstSession->m_vecAwsRsp.size(); ++dwNum)
        {
            TbPlayer tbPlayer;
            TINT32 dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[dwNum], &tbPlayer);
            if(dwRetCode < 0)
            {
                continue;
            }

            //修正alliance name等信息
            Json::Value &jAlMapping = CAllianceMapping::GetInstance()->m_oJsonRoot["alliance_mapping"];
            if(tbPlayer.m_nAlid > 0 && jAlMapping["update_time"].asInt64() > tbPlayer.m_nAlname_update_time)
            {
                if(jAlMapping.isMember(CCommonFunc::NumToString(tbPlayer.m_nAlid)))
                {
                    //这里不要改AWS表里的信息，因为alliance_mapping的内容可能滞后
                    tbPlayer.Set_Alname(jAlMapping[CCommonFunc::NumToString(tbPlayer.m_nAlid)]["al_name"].asString());
                    tbPlayer.Set_Al_nick_name(jAlMapping[CCommonFunc::NumToString(tbPlayer.m_nAlid)]["nick_name"].asString());
                    //lucien note:Player表中没有联盟旗帜，以后若有，应当也在此更新
                }
            }

            if(tbPlayer.m_nUid == uid)
            {
                jRootWriter["svr_player_rank"]["rank_list"][dwRankNum] = Json::Value(Json::objectValue);
                jRootWriter["svr_player_rank"]["rank_list"][dwRankNum]["type"] = Json::Value(rank_type);
                jRootWriter["svr_player_rank"]["rank_list"][dwRankNum]["rank"] = Json::Value(rank);
                jRootWriter["svr_player_rank"]["rank_list"][dwRankNum]["value"] = value > 0 ? value : 0;

                if(strcmp(tbPlayer.Get_Alname().c_str(), "") == 0)
                {
                    jRootWriter["svr_player_rank"]["rank_list"][dwRankNum]["extra_info"]["alname"] = Json::Value("");
                }
                else
                {
                    jRootWriter["svr_player_rank"]["rank_list"][dwRankNum]["extra_info"]["alname"] = Json::Value(tbPlayer.Get_Alname());
                }

                if(strcmp(tbPlayer.Get_Uin().c_str(), "") == 0)
                {
                    jRootWriter["svr_player_rank"]["rank_list"][dwRankNum]["extra_info"]["uname"] = Json::Value("");
                }
                else
                {
                    jRootWriter["svr_player_rank"]["rank_list"][dwRankNum]["extra_info"]["uname"] = Json::Value(tbPlayer.Get_Uin());
                }


                jRootWriter["svr_player_rank"]["rank_list"][dwRankNum]["extra_info"]["uid"] = Json::Value(tbPlayer.m_nUid);
                jRootWriter["svr_player_rank"]["rank_list"][dwRankNum]["extra_info"]["ulevel"] = Json::Value(tbPlayer.m_nLevel);
                TINT32 dwTrueAid = 0;
                if(tbPlayer.m_nAlpos > EN_ALLIANCE_POS__REQUEST)
                {
                    dwTrueAid = tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET;
                }
                jRootWriter["svr_player_rank"]["rank_list"][dwRankNum]["extra_info"]["aid"] = Json::Value(dwTrueAid);
                jRootWriter["svr_player_rank"]["rank_list"][dwRankNum]["extra_info"]["alposition"] = Json::Value(tbPlayer.Get_Alpos());
                jRootWriter["svr_player_rank"]["rank_list"][dwRankNum]["extra_info"]["force"] = Json::Value(tbPlayer.Get_Might());
                jRootWriter["svr_player_rank"]["rank_list"][dwRankNum]["extra_info"]["al_nick_name"] = tbPlayer.m_sAl_nick_name;

                bFindInDynamoDB = TRUE;
                break;
            }
        }

        if(bFindInDynamoDB == FALSE)
        {
            jRootWriter["svr_player_rank"]["rank_list"][dwRankNum] = Json::Value(Json::objectValue);
            jRootWriter["svr_player_rank"]["rank_list"][dwRankNum]["type"] = Json::Value(rank_type);
            jRootWriter["svr_player_rank"]["rank_list"][dwRankNum]["rank"] = Json::Value(rank);
            jRootWriter["svr_player_rank"]["rank_list"][dwRankNum]["value"] = value > 0 ? value : 0;

            jRootWriter["svr_player_rank"]["rank_list"][dwRankNum]["extra_info"]["alname"] =
                pstSession->m_stCommonResInfo.m_aobjPlayerRank[dwRankNum].sAlName;

            jRootWriter["svr_player_rank"]["rank_list"][dwRankNum]["extra_info"]["uname"] =
                pstSession->m_stCommonResInfo.m_aobjPlayerRank[dwRankNum].sUname;

            jRootWriter["svr_player_rank"]["rank_list"][dwRankNum]["extra_info"]["uid"] = uid;

            jRootWriter["svr_player_rank"]["rank_list"][dwRankNum]["extra_info"]["ulevel"] =
                pstSession->m_stCommonResInfo.m_aobjPlayerRank[dwRankNum].udwLevel;

            jRootWriter["svr_player_rank"]["rank_list"][dwRankNum]["extra_info"]["aid"] =
                pstSession->m_stCommonResInfo.m_aobjPlayerRank[dwRankNum].udwAid;

            jRootWriter["svr_player_rank"]["rank_list"][dwRankNum]["extra_info"]["alposition"] = EN_ALLIANCE_POS__MEMBER;
            jRootWriter["svr_player_rank"]["rank_list"][dwRankNum]["extra_info"]["force"] = 0;
            jRootWriter["svr_player_rank"]["rank_list"][dwRankNum]["extra_info"]["al_nick_name"] = "";
        }
    }

    TBOOL bHasFindSelf = FALSE;
    if(self_uid > 0 && self_uid == pstSession->m_stCommonResInfo.self_player_rank_info.uid)
    {
        TINT32 rank = pstSession->m_stCommonResInfo.self_player_rank_info.rank;
        TINT64 value = pstSession->m_stCommonResInfo.self_player_rank_info.value;

        for(TINT32 dwNum = 0; dwNum < pstSession->m_vecAwsRsp.size(); ++dwNum)
        {
            TbPlayer tbPlayer;
            TINT32 dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[dwNum], &tbPlayer);
            if(dwRetCode < 0)
            {
                continue;
            }

            //修正alliance name等信息
            Json::Value &jAlMapping = CAllianceMapping::GetInstance()->m_oJsonRoot["alliance_mapping"];
            if(tbPlayer.m_nAlid > 0 && jAlMapping["update_time"].asInt64() > tbPlayer.m_nAlname_update_time)
            {
                if(jAlMapping.isMember(CCommonFunc::NumToString(tbPlayer.m_nAlid)))
                {
                    //这里不要改AWS表里的信息，因为alliance_mapping的内容可能滞后
                    tbPlayer.Set_Alname(jAlMapping[CCommonFunc::NumToString(tbPlayer.m_nAlid)]["al_name"].asString());
                    tbPlayer.Set_Al_nick_name(jAlMapping[CCommonFunc::NumToString(tbPlayer.m_nAlid)]["nick_name"].asString());
                    //lucien note:Player表中没有联盟旗帜，以后若有，应当也在此更新
                }
            }

            if(tbPlayer.m_nUid == self_uid)
            {
                jRootWriter["svr_player_rank"]["self_rank"] = Json::Value(Json::objectValue);
                jRootWriter["svr_player_rank"]["self_rank"]["type"] = Json::Value(rank_type);
                jRootWriter["svr_player_rank"]["self_rank"]["rank"] = rank;
                jRootWriter["svr_player_rank"]["self_rank"]["value"] = value > 0 ? value : 0;
                jRootWriter["svr_player_rank"]["self_rank"]["extra_info"]["uid"] = Json::Value(tbPlayer.m_nUid);
                jRootWriter["svr_player_rank"]["self_rank"]["extra_info"]["ulevel"] = Json::Value(tbPlayer.m_nLevel);
                TINT32 dwTrueAid = 0;
                if(tbPlayer.m_nAlpos > EN_ALLIANCE_POS__REQUEST)
                {
                    dwTrueAid = tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET;
                }
                jRootWriter["svr_player_rank"]["self_rank"]["extra_info"]["aid"] = Json::Value(dwTrueAid);

                if(strcmp(tbPlayer.Get_Alname().c_str(), "") == 0)
                {
                    jRootWriter["svr_player_rank"]["self_rank"]["extra_info"]["alname"] = Json::Value("");
                }
                else
                {
                    jRootWriter["svr_player_rank"]["self_rank"]["extra_info"]["alname"] = Json::Value(tbPlayer.Get_Alname());
                }
                if(strcmp(tbPlayer.Get_Uin().c_str(), "") == 0)
                {
                    jRootWriter["svr_player_rank"]["self_rank"]["extra_info"]["uname"] = Json::Value("");
                }
                else
                {
                    jRootWriter["svr_player_rank"]["self_rank"]["extra_info"]["uname"] = Json::Value(tbPlayer.Get_Uin());
                }

                jRootWriter["svr_player_rank"]["self_rank"]["extra_info"]["alposition"] = Json::Value(tbPlayer.Get_Alpos());
                jRootWriter["svr_player_rank"]["self_rank"]["extra_info"]["force"] = Json::Value(tbPlayer.Get_Might());
                jRootWriter["svr_player_rank"]["self_rank"]["extra_info"]["al_nick_name"] = tbPlayer.m_sAl_nick_name;

                bHasFindSelf = TRUE;
                break;
            }
        }
    }

    if(!bHasFindSelf)
    {
        jRootWriter["svr_player_rank"]["self_rank"] = Json::Value(Json::objectValue);

        jRootWriter["svr_player_rank"]["self_rank"]["type"] = Json::Value(rank_type);

        jRootWriter["svr_player_rank"]["self_rank"]["rank"] = 0;
        jRootWriter["svr_player_rank"]["self_rank"]["value"] = 0;

        jRootWriter["svr_player_rank"]["self_rank"]["extra_info"]["uid"] = Json::Value(pstSession->m_stReqParam.m_udwUserId);
        jRootWriter["svr_player_rank"]["self_rank"]["extra_info"]["alname"] = Json::Value("");
        jRootWriter["svr_player_rank"]["self_rank"]["extra_info"]["uname"] = Json::Value("");
        jRootWriter["svr_player_rank"]["self_rank"]["extra_info"]["ulevel"] = Json::Value(0);
        jRootWriter["svr_player_rank"]["self_rank"]["extra_info"]["aid"] = Json::Value(0);
        jRootWriter["svr_player_rank"]["self_rank"]["extra_info"]["alposition"] = Json::Value(0);
        jRootWriter["svr_player_rank"]["self_rank"]["extra_info"]["force"] = Json::Value(0);
        jRootWriter["svr_player_rank"]["self_rank"]["extra_info"]["al_nick_name"] = Json::Value("");
    }

    jRootWriter["svr_player_rank"]["total_num"] = Json::Value(1);
    jRootWriter["svr_player_rank"]["cur_page_num"] = Json::Value(dwRankNum);
}

TVOID CProcessRankGet::NewWrapPlayerChampionJson(SSession *pstSession)
{
    Json::Value& jRootWriter = pstSession->m_stCommonResInfo.m_jResultWriter;

    jRootWriter["svr_player_champion"] = Json::Value(Json::objectValue);
    jRootWriter["svr_player_champion"]["compute_time"] = pstSession->m_stCommonResInfo.m_oComputeTime.ddwTime;
    jRootWriter["svr_player_champion"]["rank_list"] = Json::Value(Json::arrayValue);

    //TINT32 dwSelectNum = pstSession->m_stCommonResInfo.result_count;
    TINT32 dwSelectNum = sizeof(kPlayerRankTypeOrders) / sizeof(kPlayerRankTypeOrders[0]);
    for(TINT32 udwRankNum = 0; udwRankNum < dwSelectNum; udwRankNum++)
    {
        TINT32 rank_type = pstSession->m_stCommonResInfo.player_rank_infos[udwRankNum].rank_type;
        TINT32 uid = pstSession->m_stCommonResInfo.player_rank_infos[udwRankNum].uid;
        TINT32 rank = pstSession->m_stCommonResInfo.player_rank_infos[udwRankNum].rank;
        TINT64 value = pstSession->m_stCommonResInfo.player_rank_infos[udwRankNum].value;
        if (uid == 0)
        {
            rank_type = kPlayerRankTypeOrders[udwRankNum];
        }
        TBOOL bFindInDynamoDB = FALSE;
        for(TINT32 udwAwsNum = 0; udwAwsNum < pstSession->m_vecAwsRsp.size(); ++udwAwsNum)
        {
            TbPlayer tbPlayer;
            TINT32 dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[udwAwsNum], &tbPlayer);
            if (dwRetCode < 0)
            {
                continue;
            }

            //修正alliance name等信息
            Json::Value &jAlMapping = CAllianceMapping::GetInstance()->m_oJsonRoot["alliance_mapping"];
            if(tbPlayer.m_nAlid > 0 && jAlMapping["update_time"].asInt64() > tbPlayer.m_nAlname_update_time)
            {
                if(jAlMapping.isMember(CCommonFunc::NumToString(tbPlayer.m_nAlid)))
                {
                    //这里不要改AWS表里的信息，因为alliance_mapping的内容可能滞后
                    tbPlayer.Set_Alname(jAlMapping[CCommonFunc::NumToString(tbPlayer.m_nAlid)]["al_name"].asString());
                    tbPlayer.Set_Al_nick_name(jAlMapping[CCommonFunc::NumToString(tbPlayer.m_nAlid)]["nick_name"].asString());
                    //lucien note:Player表中没有联盟旗帜，以后若有，应当也在此更新
                }
            }

            if(uid == tbPlayer.m_nUid)
            {
                jRootWriter["svr_player_champion"]["rank_list"][udwRankNum]["type"] = Json::Value(rank_type);
                jRootWriter["svr_player_champion"]["rank_list"][udwRankNum]["rank"] = rank;
                jRootWriter["svr_player_champion"]["rank_list"][udwRankNum]["value"] = value > 0 ? value : 0;

                if(strcmp(tbPlayer.Get_Alname().c_str(), "") == 0)
                {
                    jRootWriter["svr_player_champion"]["rank_list"][udwRankNum]["extra_info"]["alname"] = Json::Value("");
                }
                else
                {
                    jRootWriter["svr_player_champion"]["rank_list"][udwRankNum]["extra_info"]["alname"] = Json::Value(tbPlayer.Get_Alname());
                }

                if(strcmp(tbPlayer.Get_Uin().c_str(), "") == 0)
                {
                    jRootWriter["svr_player_champion"]["rank_list"][udwRankNum]["extra_info"]["uname"] = Json::Value("");
                }
                else
                {
                    jRootWriter["svr_player_champion"]["rank_list"][udwRankNum]["extra_info"]["uname"] = Json::Value(tbPlayer.Get_Uin());
                }

                jRootWriter["svr_player_champion"]["rank_list"][udwRankNum]["extra_info"]["uid"] = Json::Value(tbPlayer.m_nUid);
                jRootWriter["svr_player_champion"]["rank_list"][udwRankNum]["extra_info"]["ulevel"] = Json::Value(tbPlayer.m_nLevel);
                TINT32 dwTrueAid = 0;
                if(tbPlayer.m_nAlpos > EN_ALLIANCE_POS__REQUEST)
                {
                    dwTrueAid = tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET;
                }
                jRootWriter["svr_player_champion"]["rank_list"][udwRankNum]["extra_info"]["aid"] = Json::Value(dwTrueAid);
                jRootWriter["svr_player_champion"]["rank_list"][udwRankNum]["extra_info"]["alposition"] = Json::Value(tbPlayer.Get_Alpos());
                jRootWriter["svr_player_champion"]["rank_list"][udwRankNum]["extra_info"]["force"] = Json::Value(tbPlayer.Get_Might());
                jRootWriter["svr_player_champion"]["rank_list"][udwRankNum]["extra_info"]["al_nick_name"] = tbPlayer.m_sAl_nick_name;

                bFindInDynamoDB = TRUE;

                break;
            }
        }

        if(bFindInDynamoDB == FALSE)
        {
            jRootWriter["svr_player_champion"]["rank_list"][udwRankNum]["type"] = Json::Value(rank_type);
            jRootWriter["svr_player_champion"]["rank_list"][udwRankNum]["rank"] = rank;
            jRootWriter["svr_player_champion"]["rank_list"][udwRankNum]["value"] = value > 0 ? value : 0;

            jRootWriter["svr_player_champion"]["rank_list"][udwRankNum]["extra_info"]["alname"] =
                pstSession->m_stCommonResInfo.player_rank_infos[udwRankNum].alname;

            jRootWriter["svr_player_champion"]["rank_list"][udwRankNum]["extra_info"]["uname"] =
                pstSession->m_stCommonResInfo.player_rank_infos[udwRankNum].uname;

            jRootWriter["svr_player_champion"]["rank_list"][udwRankNum]["extra_info"]["uid"] =
                pstSession->m_stCommonResInfo.player_rank_infos[udwRankNum].uid;

            jRootWriter["svr_player_champion"]["rank_list"][udwRankNum]["extra_info"]["ulevel"] = 1;

            jRootWriter["svr_player_champion"]["rank_list"][udwRankNum]["extra_info"]["aid"] =
                pstSession->m_stCommonResInfo.player_rank_infos[udwRankNum].aid;

            jRootWriter["svr_player_champion"]["rank_list"][udwRankNum]["extra_info"]["alposition"] = EN_ALLIANCE_POS__MEMBER;
            jRootWriter["svr_player_champion"]["rank_list"][udwRankNum]["extra_info"]["force"] = 0;
            jRootWriter["svr_player_champion"]["rank_list"][udwRankNum]["extra_info"]["al_nick_name"] = "";
        }
    }
}
