#include "process_al_recommend_get.h"
#include "global_serv.h"
#include "procedure_base.h"
#include "time_utils.h"
#include "aws_table_common.h"
#include "alliance_rank.h"
#include "all_server.h"

TINT32 CProcessAlRecommendGet::requestHandler(SSession *pstSession)
{
    TINT32 ret = 0;
    TINT32 dwLanguage = pstSession->m_stReqParam.m_udwLang;
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

        pstSession->m_stCommonResInfo.result_count = 0;
        pstSession->m_stCommonResInfo.total_count = 0;
        target_server->getRecommendAl(0, pstSession->m_stCommonResInfo.result_count, pstSession->m_stCommonResInfo.recommend_al_infos);

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
    }

    //2 get wild info -- response
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        pstSession->ResetAwsInfo();
        TbAlliance tmp_alliance;
        for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_stCommonResInfo.result_count; ++udwIdx)
        {
            tmp_alliance.Reset();
            tmp_alliance.Set_Aid(pstSession->m_stCommonResInfo.recommend_al_infos[udwIdx].aid);
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

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        NewWrapAlRecommendJson(pstSession);
        return 0;
    }

    return 0;
}

TVOID CProcessAlRecommendGet::NewWrapAlRecommendJson(SSession *pstSession)
{
    TINT32 dwRetCode = 0;
    Json::Value& jRootWriter = pstSession->m_stCommonResInfo.m_jResultWriter;
    TUINT32 udwAlliacneNum = 0;
    TbAlliance atbAlliance[DEFAULT_TOP_NUM];
    for(TINT32 udwRspIndex = 0; udwRspIndex < pstSession->m_vecAwsRsp.size(); ++udwRspIndex)
    {
        //最后将值付给了m_atbAllianceRanking
        atbAlliance[udwAlliacneNum].Reset();
        dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[udwRspIndex], &atbAlliance[udwAlliacneNum]);
        if(dwRetCode < 0)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AlRecommendGet: send req failed [seq=%u]", pstSession->m_udwSeqNo));
            continue;
        }
        if(0 == atbAlliance[udwAlliacneNum].Get_Aid() && 0 == atbAlliance[udwAlliacneNum].Get_Oid())
        {
            continue;
        }
        if(atbAlliance[udwAlliacneNum].m_nPolicy / ALLIANCE_POLICY_OFFSET != EN_ALLIANCE_JOIN_AUTO
            || atbAlliance[udwAlliacneNum].m_nMember >= 80
            || atbAlliance[udwAlliacneNum].m_nMember == 0)
        {
            continue;
        }

        atbAlliance[udwAlliacneNum].m_nIs_npc = rand() % 20;
        for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_stCommonResInfo.result_count; ++udwIdx)
        {
            if(atbAlliance[udwAlliacneNum].m_nAid == pstSession->m_stCommonResInfo.recommend_al_infos[udwIdx].aid)
            {
                if(pstSession->m_stCommonResInfo.recommend_al_infos[udwIdx].is_npc == 1)
                {
                    atbAlliance[udwAlliacneNum].m_nIs_npc += 20;
                }
                break;
            }
        }

        udwAlliacneNum++;
        if(udwAlliacneNum >= DEFAULT_TOP_NUM)
        {
            break;
        }
    }

    TbAlliance* patbAlliance[DEFAULT_TOP_NUM];
    for(TUINT32 udwIdx = 0; udwIdx < udwAlliacneNum; ++udwIdx)
    {
        patbAlliance[udwIdx] = &atbAlliance[udwIdx];
    }

    if(udwAlliacneNum > 0)
    {
        std::sort(patbAlliance, patbAlliance + udwAlliacneNum, CProcessAlRecommendGet::AllianceGreater);
    }

    TINT32 udwRankNum = 0;
    for(TUINT32 udwIdx = 0; udwIdx < udwAlliacneNum && udwIdx < DEFAULT_NUM_ONE_PAGE; ++udwIdx)
    {
        TbAlliance& tbAlliance = *patbAlliance[udwIdx];
        if(tbAlliance.m_nAl_star == 0)
        {
            tbAlliance.m_nAl_star = 2;
        }

        jRootWriter["svr_alliance_recommended"]["list"][udwRankNum]["sid"] = tbAlliance.Get_Sid();
        jRootWriter["svr_alliance_recommended"]["list"][udwRankNum]["oid"] = tbAlliance.Get_Oid();
        jRootWriter["svr_alliance_recommended"]["list"][udwRankNum]["oname"] = tbAlliance.Get_Oname();
        jRootWriter["svr_alliance_recommended"]["list"][udwRankNum]["aid"] = tbAlliance.Get_Aid();
        jRootWriter["svr_alliance_recommended"]["list"][udwRankNum]["alname"] = tbAlliance.Get_Name();
        jRootWriter["svr_alliance_recommended"]["list"][udwRankNum]["force"] = tbAlliance.Get_Might();
        jRootWriter["svr_alliance_recommended"]["list"][udwRankNum]["force_kill"] = tbAlliance.m_nForce_kill > 0 ? tbAlliance.m_nForce_kill : 0;
        jRootWriter["svr_alliance_recommended"]["list"][udwRankNum]["member"] = tbAlliance.Get_Member();
        jRootWriter["svr_alliance_recommended"]["list"][udwRankNum]["desc"] = tbAlliance.Get_Desc();
        jRootWriter["svr_alliance_recommended"]["list"][udwRankNum]["lang"] = tbAlliance.Get_Language();
        jRootWriter["svr_alliance_recommended"]["list"][udwRankNum]["policy"] = tbAlliance.Get_Policy() / ALLIANCE_POLICY_OFFSET;
        jRootWriter["svr_alliance_recommended"]["list"][udwRankNum]["gift_point"] = tbAlliance.Get_Gift_point();
        jRootWriter["svr_alliance_recommended"]["list"][udwRankNum]["flag"] = tbAlliance.m_nAvatar;
        jRootWriter["svr_alliance_recommended"]["list"][udwRankNum]["al_nick"] = tbAlliance.m_sAl_nick_name;
        jRootWriter["svr_alliance_recommended"]["list"][udwRankNum]["al_star"] = tbAlliance.m_nAl_star;
        udwRankNum++;
    }

    if(0 == udwRankNum)
    {
        jRootWriter["svr_alliance_recommended"]["list"] = Json::Value(Json::arrayValue);
    }
    jRootWriter["svr_alliance_recommended"]["total_page_num"] = 1;
    jRootWriter["svr_alliance_recommended"]["cur_page_num"] = Json::Value(udwRankNum);
}

TBOOL CProcessAlRecommendGet::AllianceGreater(const TbAlliance* stA, const TbAlliance* stB)
{
    TINT32 dwPriorityA = 0, dwPriorityB = 0;

    dwPriorityA += stA->m_nMember;
    if(stA->m_nMember > 30)
    {
        dwPriorityA += 50;
    }
    if(stA->m_nIs_npc >= 20)
    {
        dwPriorityA -= 70;
    }
    dwPriorityA += stA->m_nIs_npc;

    dwPriorityB += stB->m_nMember;
    if(stB->m_nMember > 30)
    {
        dwPriorityB += 50;
    }
    if(stB->m_nIs_npc >= 20)
    {
        dwPriorityB -= 70;
    }
    dwPriorityB += stB->m_nIs_npc;

    return dwPriorityA > dwPriorityB;
}
