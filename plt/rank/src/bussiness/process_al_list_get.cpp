#include "global_serv.h"
#include "process_al_list_get.h"
#include "procedure_base.h"
#include "time_utils.h"
#include "aws_table_common.h"
#include "alliance_rank.h"
#include "all_server.h"

TINT32 CProcessAlListGet::requestHandler(SSession *pstSession)
{
    TINT32 ret = 0;

    TINT32 page = pstSession->m_stReqParam.m_udwPage;
    TINT32 offset = (page - 1) * 20;
    if(offset < 0)
    {
        offset = 0;
    }

    TINT32 join_policy = atoi(pstSession->m_stReqParam.m_szKey[0]);

    const TINT32  kMaxBufferLength = 256;
    TCHAR keyword[kMaxBufferLength];
    keyword[0] = '\0';
    snprintf(keyword, kMaxBufferLength, "%s", pstSession->m_stReqParam.m_szKey[1]);

    TINT32 sid = pstSession->m_stReqParam.m_udwSvrId;

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
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

        if(0 == strcmp(keyword, ""))
        {
            target_server->getAlList(join_policy, offset, pstSession->m_stCommonResInfo.result_count,
                pstSession->m_stCommonResInfo.total_count, pstSession->m_stCommonResInfo.al_rank_infos);
        }
        else
        {
            target_server->searchAl(join_policy, keyword, offset, pstSession->m_stCommonResInfo.result_count,
                pstSession->m_stCommonResInfo.total_count, pstSession->m_stCommonResInfo.al_rank_infos);
        }

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        pstSession->ResetAwsInfo();
        TbAlliance tmp_alliance;
        for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_stCommonResInfo.result_count; ++udwIdx)
        {
            tmp_alliance.Reset();
            tmp_alliance.Set_Aid(pstSession->m_stCommonResInfo.al_rank_infos[udwIdx].aid);
            CAwsRequest::GetItem(pstSession, &tmp_alliance, ETbALLIANCE_OPEN_TYPE_PRIMARY);
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
            TSE_LOG_ERROR(pstSession->m_poServLog, ("AlListGet: send req failed [seq=%u]", pstSession->m_udwSeqNo));
            return -3;
        }
        else
        {
            return 0;
        }
    }

    //4 解析并返回
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        NewWrapAlListJson(pstSession);
        return 0;
    }

    return 0;
}

TVOID CProcessAlListGet::NewWrapAlListJson(SSession *pstSession)
{
    TINT32 dwRetCode = 0;
    Json::Value& jRootWriter = pstSession->m_stCommonResInfo.m_jResultWriter;
    TINT32 udwRankNum = 0;

    for(TINT32 udwNum = 0; udwNum < pstSession->m_vecAwsRsp.size(); ++udwNum)
    {
        //最后将值付给了m_atbAllianceRanking
        TbAlliance tbAlliance;
        dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[udwNum], &tbAlliance);
        if(dwRetCode <= 0)
        {
            //pstSession->m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            //TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AlRecommendGet: send req failed [seq=%u]", pstSession->m_udwSeqNo));
            continue;
        }

        if(tbAlliance.m_nAl_star == 0)
        {
            tbAlliance.m_nAl_star = 2;
        }

        jRootWriter["svr_alliance_list"]["list"][udwRankNum]["sid"] = tbAlliance.m_nSid;
        jRootWriter["svr_alliance_list"]["list"][udwRankNum]["oid"] = tbAlliance.m_nOid;
        jRootWriter["svr_alliance_list"]["list"][udwRankNum]["oname"] = tbAlliance.m_sOname;
        jRootWriter["svr_alliance_list"]["list"][udwRankNum]["aid"] = tbAlliance.m_nAid;
        jRootWriter["svr_alliance_list"]["list"][udwRankNum]["alname"] = tbAlliance.m_sName;
        jRootWriter["svr_alliance_list"]["list"][udwRankNum]["force"] = tbAlliance.m_nMight;
        jRootWriter["svr_alliance_list"]["list"][udwRankNum]["force_kill"] = tbAlliance.m_nForce_kill > 0 ? tbAlliance.m_nForce_kill : 0;
        jRootWriter["svr_alliance_list"]["list"][udwRankNum]["member"] = tbAlliance.m_nMember;
        jRootWriter["svr_alliance_list"]["list"][udwRankNum]["desc"] = tbAlliance.m_sDesc;
        jRootWriter["svr_alliance_list"]["list"][udwRankNum]["lang"] = tbAlliance.m_nLanguage;
        jRootWriter["svr_alliance_list"]["list"][udwRankNum]["policy"] = tbAlliance.m_nPolicy / ALLIANCE_POLICY_OFFSET;
        jRootWriter["svr_alliance_list"]["list"][udwRankNum]["gift_point"] = tbAlliance.m_nGift_point;
        jRootWriter["svr_alliance_list"]["list"][udwRankNum]["flag"] = tbAlliance.m_nAvatar;
        jRootWriter["svr_alliance_list"]["list"][udwRankNum]["al_nick"] = tbAlliance.m_sAl_nick_name;
        jRootWriter["svr_alliance_list"]["list"][udwRankNum]["al_star"] = tbAlliance.m_nAl_star;
        udwRankNum++;
    }
    if(0 == udwRankNum)
    {
        jRootWriter["svr_alliance_list"]["list"] = Json::Value(Json::arrayValue);
    }
    jRootWriter["svr_alliance_list"]["total_page_num"] = Json::Value(ceil(pstSession->m_stCommonResInfo.total_count / 20.0));
    jRootWriter["svr_alliance_list"]["cur_page_num"] = Json::Value(udwRankNum);
}
