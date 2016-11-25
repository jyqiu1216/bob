#include "process_alliance.h"
#include "statistic.h"
#include "game_info.h"
#include "procedure_base.h"
#include "global_serv.h"
#include "db_request.h"
#include "db_response.h"
#include "process_diplomacy.h"
#include "common_base.h"
#include "common_func.h"
#include "globalres_logic.h"
#include "msg_base.h"
#include "sendmessage_base.h"
#include "backpack_info.h"
#include "quest_logic.h"
#include "common_json.h"
#include "encode/src/encode/utf8_util.h"
#include "common_logic.h"
#include "document.h"
#include "conf_base.h"
#include "tool_base.h"
#include "action_base.h"
#include "player_base.h"
#include "map_logic.h"
#include "item_base.h"
#include "city_base.h"
#include "pushdata_action.h"

#include "update_notic_task_mgr.h"
#include "update_notic_task.h"
#include "update_notic_process.h"

#include "wild_info.h"

TINT32 CProcessAlliance::ProcessCmd_AllianceCreate(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    TbAlliance *ptbAlliance = &pstSession->m_stUserInfo.m_tbAlliance;
    TbPlayer *ptbPlayer = &pstSession->m_stUserInfo.m_tbPlayer;
    SCityInfo *pstCity = &pstSession->m_stUserInfo.m_stCityInfo;
    // 0. 输入参数
    TCHAR *pszAllianceName = &pstSession->m_stReqParam.m_szKey[0][0];
    TCHAR *pszAllianceDesc = &pstSession->m_stReqParam.m_szKey[1][0];
    TUINT32 udwAllianceLanguage = atoi(pstSession->m_stReqParam.m_szKey[2]);
    TCHAR *pszAllianceNick = &pstSession->m_stReqParam.m_szKey[3][0];
    TUINT32 udwPolicy = EN_ALLIANCE_JOIN_AUTO;

    // 1.1 使用名称查找该名称是否已经被使用
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        // check param
        if(ptbPlayer->m_nAlpos != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ALREADY_IN_AL_FOR_CREATE_AL;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AllianceCreate: user already has alliance[%u:%s] [seq=%u]", 
                ptbPlayer->m_nAlid, ptbPlayer->m_sAlname.c_str(), pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
        TUINT32 udwAllianceNameLen = strlen(pszAllianceName);
        if(udwAllianceNameLen < 3)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AllianceCreate: req alliance name[%s] [seq=%u]", 
                pszAllianceName, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }

        pstSession->ResetAwsInfo();

        // get check name
        TbUnique_name tbUnique_name;
        tbUnique_name.Set_Type(EN_ALLIANCE_NAME);
        //TODO need check
        tbUnique_name.Set_Name(CToolBase::ToLower(pszAllianceName));
        tbUnique_name.Set_Exist(1);
        tbUnique_name.Set_Id(-1 * ptbPlayer->m_nUid);

        ExpectedDesc expect_desc;
        expect_desc.clear();
        ExpectedItem expect_item;
        expect_item.SetVal(TbUNIQUE_NAME_FIELD_EXIST, FALSE);
        expect_desc.push_back(expect_item);

        CAwsRequest::UpdateItem(pstSession, &tbUnique_name, expect_desc, RETURN_VALUES_ALL_NEW);

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        // send
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AllianceCreate: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }

        return 0;
    }

    // 1.2. 处理响应
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        TbUnique_name tbUniqueName;
        tbUniqueName.Reset();

        dwRetCode = CAwsResponse::OnUpdateItemRsp(*pstSession->m_vecAwsRsp[0], &tbUniqueName);
        if(0 == dwRetCode)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ALLIANCE_NAME_BE_USED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AllianceCreate: alliance name already exist, createfailed [dwRetCode=%d] [seq=%u]", \
                dwRetCode, \
                pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }
    }

    // 1.3 简称是否存在
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        pstSession->ResetAwsInfo();

        // get check name
        TbUnique_name tbUnique_name;
        tbUnique_name.Set_Type(EN_ALLIANCE_NICK_NAME);
        tbUnique_name.Set_Name(CToolBase::ToLower(pszAllianceNick));
        tbUnique_name.Set_Exist(1);
        tbUnique_name.Set_Id(-1 * ptbPlayer->m_nUid);

        ExpectedDesc expect_desc;
        expect_desc.clear();
        ExpectedItem expect_item;
        expect_item.SetVal(TbUNIQUE_NAME_FIELD_EXIST, FALSE);
        expect_desc.push_back(expect_item);

        CAwsRequest::UpdateItem(pstSession, &tbUnique_name, expect_desc, RETURN_VALUES_ALL_NEW);

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        // send
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AllianceCreate: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -4;
        }

        return 0;
    }

    // 1.2. 处理响应
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        TbUnique_name tbUniqueName;
        tbUniqueName.Reset();

        dwRetCode = CAwsResponse::OnUpdateItemRsp(*pstSession->m_vecAwsRsp[0], &tbUniqueName);
        if(0 == dwRetCode)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AllianceCreate: alliance nick name already exist, createfailed [dwRetCode=%d] [seq=%u]", \
                dwRetCode, \
                pstSession->m_stUserInfo.m_udwBSeqNo));
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;
        }
        else
        {
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__5;
        }
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        pstSession->ResetAwsInfo();

        // get check name
        TbUnique_name tbUnique_name;
        tbUnique_name.Set_Type(EN_ALLIANCE_NAME);
        tbUnique_name.Set_Name(CToolBase::ToLower(pszAllianceName));

        CAwsRequest::DeleteItem(pstSession, &tbUnique_name);

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__4;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        // send
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AllianceCreate: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -5;
        }

        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__4)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ALLIANCE_NICK_BE_USED;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AllianceCreate:nick name already exist, createfailed [dwRetCode=%d] [seq=%u]", \
            dwRetCode, \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -6;
    }

    // 2. 发送获取联盟id的请求
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__5)
    {
        // 发送获取联盟id的请求
        pstSession->ResetAwsInfo();
        CAwsRequest::GetGlobalNewId(pstSession, EN_GLOBAL_PARAM__ALLIANCE_ID);
        //CAwsRequest::GetGlobalNewId(pstSession, EN_SVR_PARAM__ALLIANCE_COUNT);

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__6;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AllianceCreate: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -7;
        }
        return 0;
    }

    // 2.2 解析响应
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__6)
    {
        // parse data
        TINT32 dwAlId = 0;
//        TINT32 dwAlCount = 0;
        TbParam tbParamItem;
        for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecAwsRsp.size(); ++udwIdx)
        {
            dwRetCode = CAwsResponse::OnUpdateItemRsp(*pstSession->m_vecAwsRsp[udwIdx], &tbParamItem);
            if(dwRetCode > 0)
            {
                if(tbParamItem.m_nKey == EN_GLOBAL_PARAM__ALLIANCE_ID)
                {
                    dwAlId = tbParamItem.m_nVal;
                }
//                 if(tbParamItem.m_nKey == EN_SVR_PARAM__ALLIANCE_COUNT)
//                 {
//                     dwAlCount = tbParamItem.m_nVal;
//                 }
            }
            else
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_PACKAGE_ERR;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AllianceCreate: get alliance id failed[%d] [seq=%u]", dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
                return -8;
            }
        }

        // update alliance info
        TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
        ptbAlliance->Reset();
        ptbAlliance->Set_Aid(dwAlId);
        ptbAlliance->Set_Name(pszAllianceName);
        ptbAlliance->Set_Oid(ptbPlayer->m_nUid);
        ptbAlliance->Set_Owner_cid(ptbPlayer->m_nCid);
        ptbAlliance->Set_Oname(ptbPlayer->m_sUin);
        ptbAlliance->Set_Member(1);
        ptbAlliance->Set_Might(ptbPlayer->m_nMight);
        ptbAlliance->Set_Force_kill(ptbPlayer->m_nMkill);
        ptbAlliance->Set_Sid(pstSession->m_stReqParam.m_udwSvrId);
        ptbAlliance->Set_Desc(pszAllianceDesc);
        ptbAlliance->Set_Al_nick_name(pszAllianceNick);
//         if(ptbAlliance->m_sDesc.empty())
//         {
//             ptbAlliance->Set_Desc("There is no description now.");
//         }
        ptbAlliance->Set_Language(udwAllianceLanguage);
        ptbAlliance->Set_Policy(udwPolicy*ALLIANCE_POLICY_OFFSET + 1);
        ptbAlliance->Set_Notice("");
        ptbAlliance->Set_Al_star(2);
        ptbAlliance->Set_Hive_svr(ptbPlayer->m_nSid);
        ptbAlliance->Set_Hive_pos(0);
        ptbAlliance->Set_Hive_show_flag(0);
        pstSession->m_stUserInfo.m_ucAllianceFlag = EN_TABLE_UPDT_FLAG__NEW;

        // update player info
        pstSession->m_dwOldRequestAid = ptbPlayer->m_nReq_al >> 32;
        ptbPlayer->Set_Alid(CCommonBase::GetPlayerAllianceId(ptbAlliance->m_nAid, EN_ALLIANCE_POS__CHANCELLOR));
        ptbPlayer->Set_Req_al(0);
        ptbPlayer->Set_Alpos(EN_ALLIANCE_POS__CHANCELLOR);
        ptbPlayer->Set_Alname(pszAllianceName);
        ptbPlayer->Set_Al_nick_name(pszAllianceNick);
        ptbPlayer->Set_Al_time(udwCurTime);
        ptbPlayer->Set_Invite_mail_time(udwCurTime);
        ptbPlayer->Set_Alname_update_time(udwCurTime);

        //gen svr_al record
        pstSession->m_stUserInfo.m_tbSvrAl.Set_Sid(ptbPlayer->m_nSid);
        pstSession->m_stUserInfo.m_tbSvrAl.Set_Alid(ptbAlliance->m_nAid);
        pstSession->m_stUserInfo.m_tbSvrAl.Set_Owner_uid(ptbPlayer->m_nUid);
        pstSession->m_stUserInfo.m_tbSvrAl.Set_Owner_cid(pstCity->m_stTblData.m_nPos);
        pstSession->m_stUserInfo.m_aucSvrAlFlag = EN_TABLE_UPDT_FLAG__NEW;

        if(pstSession->m_stUserInfo.m_tbLogin.m_nAl_time == 0)
        {
            ptbPlayer->Set_Status(ptbPlayer->m_nStatus | EN_CITY_STATUS__FIRST_ALLIANCE_REWARD);
            TUINT32 udwGemReward = CGameInfo::GetInstance()->m_oJsonRoot["game_basic"][EN_GAME_BASIC_FIRST_JOIN_ALLIANCE_GEM_REWARD].asUInt();
            CPlayerBase::AddGem(&pstSession->m_stUserInfo.m_tbLogin, udwGemReward);
            TSE_LOG_INFO(CGameInfo::GetInstance()->m_poLog, ("CProcessAlCreate: uid[%ld]first create alliance, reward gem[%u][seq=%u]", ptbPlayer->m_nUid, udwGemReward, pstSession->m_udwSeqNo));
        }
        else// 发送邮件
        {
            CMsgBase::SendOperateMail(ptbPlayer->m_nUid, EN_MAIL_ID__CREATE_ALLIANCE, ptbPlayer->m_nSid,
                SYSTEM_ENCOURAGE, pszAllianceName, "", "");
        }

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__7;
        CMsgBase::AddAlRank(ptbAlliance);
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__7)
    {
        pstSession->ResetAwsInfo();

        // get check name
        TbUnique_name tbUnique_name;
        tbUnique_name.Set_Type(EN_ALLIANCE_NAME);
        tbUnique_name.Set_Name(CToolBase::ToLower(pszAllianceName));
        tbUnique_name.Set_Exist(1);
        tbUnique_name.Set_Id(ptbAlliance->m_nAid);
        CAwsRequest::UpdateItem(pstSession, &tbUnique_name);

        tbUnique_name.Set_Type(EN_ALLIANCE_NICK_NAME);
        tbUnique_name.Set_Name(CToolBase::ToLower(pszAllianceNick));
        tbUnique_name.Set_Exist(1);
        tbUnique_name.Set_Id(ptbAlliance->m_nAid);
        CAwsRequest::UpdateItem(pstSession, &tbUnique_name);

        TbAl_member tbAlmember;
        tbAlmember.Set_Aid(ptbAlliance->m_nAid);
        tbAlmember.Set_Uid(ptbPlayer->m_nUid);
        tbAlmember.Set_Al_pos(EN_ALLIANCE_POS__CHANCELLOR);
        CCommJson::GenAlMemberInfo(ptbPlayer, tbAlmember.m_jProfile);
        tbAlmember.SetFlag(TbAL_MEMBER_FIELD_PROFILE);
        CAwsRequest::UpdateItem(pstSession, &tbAlmember);

        if(pstSession->m_dwOldRequestAid > 0)
        {
            tbAlmember.Reset();
            tbAlmember.Set_Aid(-1 * pstSession->m_dwOldRequestAid);
            tbAlmember.Set_Uid(ptbPlayer->m_nUid);
            CAwsRequest::DeleteItem(pstSession, &tbAlmember);
        }

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__8;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        // send
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AllianceCreate: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -10;
        }

        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__8)
    {
        //task count
        CQuestLogic::SetTaskCurValue(&pstSession->m_stUserInfo,pstCity, EN_TASK_TYPE_JOIN_ALLIANCE);

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    return 0;
}

TINT32 CProcessAlliance::ProcessCmd_AllianceRequestJoin(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    TbPlayer *ptbSelfPlayer = &pstSession->m_stUserInfo.m_tbPlayer;
    SUserInfo *pstUserInfo = &pstSession->m_stUserInfo;
    TbAlliance* ptbTargetAlliance = &pstSession->m_tbTmpAlliance;
    SCityInfo *pstCity = &pstUserInfo->m_stCityInfo;
    // 0. 获取请求参数
    TUINT32 udwAllianceId = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    //TUINT32 udwForce = strtoul(pstSession->m_stReqParam.m_szKey[1], NULL, 10);
    //TUINT32 udwPolicy = strtoul(pstSession->m_stReqParam.m_szKey[2], NULL, 10);

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        // check param
        if(ptbSelfPlayer->m_nAlpos != EN_ALLIANCE_POS__REQUEST)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AllianceRequestJoin: already has alliance [seq=%u]", \
                pstSession->m_stUserInfo.m_udwBSeqNo));
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ALREADY_IN_ALLIANCE;
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
            return -1;
        }
        for(TUINT32 udwIdx = 0; udwIdx < pstUserInfo->m_udwPassiveMarchNum; udwIdx++)
        {
            if(pstUserInfo->m_aucPassiveMarchFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
            {
                continue;
            }
            if(pstUserInfo->m_atbPassiveMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR)
            {
                if(pstUserInfo->m_atbPassiveMarch[udwIdx].m_bParam[0].m_ddwSourceAlliance == udwAllianceId)
                {
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AllianceRequestJoin: trying to join an alliance which is attacking you by rally war [seq=%u]", \
                        pstSession->m_stUserInfo.m_udwBSeqNo));
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__IS_RALLY_WAR_ATTACKED_BY_TARGET_ALLIANCE;
                    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
                    return -9;
                }
            }
        }

        pstSession->ResetAwsInfo();
        CAwsRequest::AllianceGetByAid(pstSession, udwAllianceId);
        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AllianceRequestJoin: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }
        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        ptbTargetAlliance->Reset();
        CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[0], ptbTargetAlliance);
        if(ptbTargetAlliance->m_nAid == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ALLIANCE_NOT_EXIST;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AllianceRequestJoin: get target alliance failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }

        if(ptbTargetAlliance->m_nSid != ptbSelfPlayer->m_nSid)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ALLIANCE_NOT_EXIT_IN_THIS_SVR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AllianceRequestJoin: get target alliance diff sid [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -4;
        }

        if(ptbTargetAlliance->m_nMember >= MAX_ALLIANCE_MEMBER_NUM)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ALLIANCE_MEMBER_OVER_LOAD;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AllianceRequestJoin: alliance[%lld] is full [seq=%u]", ptbTargetAlliance->m_nAid, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -4;
        }

        pstSession->ResetAwsInfo();
        if (ptbTargetAlliance->m_nPolicy / ALLIANCE_POLICY_OFFSET == EN_ALLIANCE_JOIN_NORMAL)
        {
            pstSession->m_dwOldRequestAid = ptbSelfPlayer->m_nReq_al >> 32;
            // set data
            TbPlayer tbPlayerItem = *ptbSelfPlayer;

            tbPlayerItem.Set_Alid(CCommonBase::GetPlayerAllianceId(udwAllianceId, EN_ALLIANCE_POS__REQUEST));
            tbPlayerItem.Set_Alpos(EN_ALLIANCE_POS__REQUEST);
            TINT64 ddwReqAlTime = udwAllianceId;
            ddwReqAlTime = (ddwReqAlTime << 32) + CTimeUtils::GetUnixTime();
            tbPlayerItem.Set_Req_al(ddwReqAlTime);
            ExpectedDesc expect;
            expect.push_back(ExpectedItem(TbPLAYER_FIELD_ALPOS, TRUE, EN_ALLIANCE_POS__REQUEST));

            CAwsRequest::UpdateItem(pstSession, &tbPlayerItem, expect, RETURN_VALUES_ALL_NEW);

            // set next procedure
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
            bNeedResponse = TRUE;
            dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
            if(dwRetCode < 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AllianceRequestJoin: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                return -5;
            }
            return 0;
        }
        else
        {
            pstSession->m_dwOldRequestAid = ptbSelfPlayer->m_nReq_al >> 32;
            // set data
            TbPlayer tbPlayerItem = *ptbSelfPlayer;

            tbPlayerItem.Set_Alid(CCommonBase::GetPlayerAllianceId(udwAllianceId, EN_ALLIANCE_POS__MEMBER));
            tbPlayerItem.Set_Alpos(EN_ALLIANCE_POS__MEMBER);
            tbPlayerItem.Set_Alname(ptbTargetAlliance->m_sName);
            tbPlayerItem.Set_Al_nick_name(ptbTargetAlliance->m_sAl_nick_name);
            tbPlayerItem.Set_Al_time(CTimeUtils::GetUnixTime());
            tbPlayerItem.Set_Invite_mail_time(CTimeUtils::GetUnixTime());
            tbPlayerItem.Set_Req_al(0);
            tbPlayerItem.Set_Alname_update_time(CTimeUtils::GetUnixTime());

            ExpectedDesc expect;
            expect.push_back(ExpectedItem(TbPLAYER_FIELD_ALPOS, TRUE, EN_ALLIANCE_POS__REQUEST));

            CAwsRequest::UpdateItem(pstSession, &tbPlayerItem, expect, RETURN_VALUES_ALL_NEW);

            // set next procedure
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
            bNeedResponse = TRUE;
            dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
            if(dwRetCode < 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AllianceRequestJoin: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                return -6;
            }
            return 0;
        }
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        TbPlayer tbPlayerItem;
        tbPlayerItem.Reset();
        dwRetCode = CAwsResponse::OnUpdateItemRsp(*pstSession->m_vecAwsRsp[0], &tbPlayerItem);
        if(dwRetCode <= 0 || tbPlayerItem.m_nUid == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AllianceRequestJoin: respon fail [ret=%d uid=%u][seq=%u]", dwRetCode, tbPlayerItem.m_nUid, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -7;
        }

        pstSession->ResetAwsInfo();
        if (ptbTargetAlliance->m_nPolicy / ALLIANCE_POLICY_OFFSET == EN_ALLIANCE_JOIN_NORMAL)
        {
            TbAl_member tbAlmember;
            if(pstSession->m_dwOldRequestAid > 0)
            {
                tbAlmember.Set_Aid(-1 * pstSession->m_dwOldRequestAid);
                tbAlmember.Set_Uid(ptbSelfPlayer->m_nUid);
                CAwsRequest::DeleteItem(pstSession, &tbAlmember);
            }
            *ptbSelfPlayer = tbPlayerItem;

            tbAlmember.Reset();
            tbAlmember.Set_Aid(-1 * ptbTargetAlliance->m_nAid);
            tbAlmember.Set_Uid(ptbSelfPlayer->m_nUid);
            tbAlmember.Set_Al_pos(EN_ALLIANCE_POS__REQUEST);
            tbAlmember.Set_Req_time(CTimeUtils::GetUnixTime());
            CCommJson::GenAlMemberInfo(ptbSelfPlayer, tbAlmember.m_jProfile);
            tbAlmember.SetFlag(TbAL_MEMBER_FIELD_PROFILE);

            CAwsRequest::UpdateItem(pstSession, &tbAlmember);
        }
        else
        {
            TbAl_member tbAlmember;
            if(pstSession->m_dwOldRequestAid > 0)
            {
                tbAlmember.Set_Aid(-1 * pstSession->m_dwOldRequestAid);
                tbAlmember.Set_Uid(ptbSelfPlayer->m_nUid);
                CAwsRequest::DeleteItem(pstSession, &tbAlmember);
            }
            *ptbSelfPlayer = tbPlayerItem;

            tbAlmember.Set_Aid(ptbTargetAlliance->m_nAid);
            tbAlmember.Set_Uid(ptbSelfPlayer->m_nUid);
            tbAlmember.Set_Al_pos(EN_ALLIANCE_POS__MEMBER);
            tbAlmember.Set_Req_time(0);
            CCommJson::GenAlMemberInfo(ptbSelfPlayer, tbAlmember.m_jProfile);
            tbAlmember.SetFlag(TbAL_MEMBER_FIELD_PROFILE);

            CAwsRequest::UpdateItem(pstSession, &tbAlmember);

            // updt alliance info
            ptbTargetAlliance->Set_Member(ptbTargetAlliance->m_nMember + 1);
            ptbTargetAlliance->Set_Might(ptbTargetAlliance->m_nMight + ptbSelfPlayer->m_nMight);
            ptbTargetAlliance->Set_Force_kill(ptbTargetAlliance->m_nForce_kill + ptbSelfPlayer->m_nMkill);
            pstUserInfo->m_tbAlliance = *ptbTargetAlliance;

            pstUserInfo->m_tbUserStat.Set_Wall_get_t(CTimeUtils::GetUnixTime());

            for(TUINT32 udwIdx = 0; udwIdx < pstUserInfo->m_udwWildNum; ++udwIdx)
            {
                pstUserInfo->m_atbWild[udwIdx].Set_Alid(ptbTargetAlliance->m_nAid);
                pstUserInfo->m_atbWild[udwIdx].Set_Alname(ptbTargetAlliance->m_sName);
                pstUserInfo->m_atbWild[udwIdx].Set_Al_nick(ptbTargetAlliance->m_sAl_nick_name);
                pstUserInfo->m_atbWild[udwIdx].Set_Al_flag(ptbTargetAlliance->m_nAvatar);
                pstUserInfo->m_atbWild[udwIdx].Set_Name_update_time(CTimeUtils::GetUnixTime());
            }

            //task count
            CQuestLogic::SetTaskCurValue(&pstSession->m_stUserInfo,pstCity, EN_TASK_TYPE_JOIN_ALLIANCE);
        }

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AllianceRequestJoin: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -8;
        }
        return 0;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        if (ptbTargetAlliance->m_nPolicy / ALLIANCE_POLICY_OFFSET == EN_ALLIANCE_JOIN_AUTO)
        {
            if(pstSession->m_stUserInfo.m_tbLogin.m_nAl_time == 0)
            {
                ptbSelfPlayer->Set_Status(ptbSelfPlayer->m_nStatus | EN_CITY_STATUS__FIRST_ALLIANCE_REWARD);
                TUINT32 udwGemReward = CGameInfo::GetInstance()->m_oJsonRoot["game_basic"][EN_GAME_BASIC_FIRST_JOIN_ALLIANCE_GEM_REWARD].asUInt();
                CPlayerBase::AddGem(&pstSession->m_stUserInfo.m_tbLogin, udwGemReward);
                TSE_LOG_INFO(CGameInfo::GetInstance()->m_poLog, ("CProcessAlCreate: uid[%ld]first join alliance, reward gem[%u][seq=%u]", ptbSelfPlayer->m_nUid, udwGemReward, pstSession->m_udwSeqNo));
            }
            else// 发送邮件
            {
                CMsgBase::SendOperateMail(ptbSelfPlayer->m_nUid, EN_MAIL_ID__JOINED_AN_ALLIANCE, ptbSelfPlayer->m_nSid,
                    SYSTEM_ENCOURAGE, ptbTargetAlliance->m_sName.c_str(), "", "");
            }

            pstUserInfo->m_tbUserStat.Set_Al_event_tips_time(CTimeUtils::GetUnixTime());

            CSendMessageBase::AddTips(&pstSession->m_stUserInfo, EN_TIPS_TYPE__JOINED_ALLIANCE, ptbSelfPlayer->m_nUid,
                FALSE, ptbTargetAlliance->m_nAid, 0, 0, ptbTargetAlliance->m_sName.c_str());

            
            ostringstream ossCustomizeParam;
            ossCustomizeParam.str("");
            ossCustomizeParam << EN_CHAT_SYS_MSG_JOIN_TO_ALLIANCE << "#" << ptbSelfPlayer->m_nUid << "#" << ptbSelfPlayer->m_sUin;
            CSendMessageBase::SendSysMsgToChat(ptbSelfPlayer->m_nSid, ptbTargetAlliance->m_nAid, 1, ossCustomizeParam.str().c_str());
        }
        else
        {
            //增加有人申请加入的推送
            SNoticInfo stNoticInfo;
            stNoticInfo.Reset();
            stNoticInfo.m_dwNoticId = EN_NOTI_ID__AL_REQUEST;
            stNoticInfo.SetValue(EN_NOTI_ID__AL_REQUEST,
                "", "",
                0, 0,
                0, 0,
                0, "", 0);
            CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), ptbTargetAlliance->m_nSid, ptbTargetAlliance->m_nOid, stNoticInfo);
        }

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessAlliance::ProcessCmd_AllianceLeave(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbPlayer *ptbPlayer = &pstSession->m_stUserInfo.m_tbPlayer;
    TbAlliance *ptbAlliance = &pstSession->m_stUserInfo.m_tbAlliance;
    SCityInfo *pstCity = &pstSession->m_stUserInfo.m_stCityInfo;

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        if(ptbPlayer->m_nAlpos == EN_ALLIANCE_POS__REQUEST)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AllianceLeave: not in alliance still ret=0 [seq=%u]",
                pstSession->m_stUserInfo.m_udwBSeqNo));
            return 0;
            //pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NOT_IN_ALLIANCE;
            //return -1;
        }

        //wave@20160623: for dead player
        if(CToolBase::IsOpCommand(pstSession->m_stReqParam.m_szCommand))
        {
            TBOOL bDeadPlayerNeedClear = CPlayerBase::CheckDeadPlayerBaseCondForClear(pstUser);
            if(bDeadPlayerNeedClear == FALSE)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__USER_CANNOT_CLEAR;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__ClearUser: uid=%ld dead=%ld utime=%ld, can't clear [seq=%u]", 
                    ptbPlayer->m_nUid, ptbPlayer->m_nDead_flag, ptbPlayer->m_nUtime,                 
                    pstSession->m_stUserInfo.m_udwBSeqNo)); 
                return -7;
            }
        } 

        if (ptbPlayer->m_nAlpos == EN_ALLIANCE_POS__CHANCELLOR && pstSession->m_tbThrone.m_nAlid == ptbAlliance->m_nAid)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__CAN_NOT_LEAVE_ALLIANCE;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AllianceLeave: has throne, can not leave [seq=%u]",
                pstSession->m_stUserInfo.m_udwBSeqNo));
            return -4;
        }

        for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
        {
            TbMarch_action* ptbRallyWar = &pstUser->m_atbMarch[udwIdx];
            if(ptbRallyWar->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR
                && ptbRallyWar->m_nSuid == pstUser->m_tbPlayer.m_nUid
                && ptbRallyWar->m_nStatus == EN_MARCH_STATUS__FIGHTING)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
                return -2;
            }
        }

        // reset req
        pstSession->ResetAwsInfo();

        // set player get
        CAwsRequest::AllAlMemberQuery(pstSession, ptbAlliance->m_nAid);

        // send request
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AllianceMemberGet: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }

        return 0;
    }

    // 2. 解析获取的数据
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        // get data info
        TUINT32 udwMemberNum = 0;
        dwRetCode = CAwsResponse::OnQueryRsp(*pstSession->m_vecAwsRsp[0],
            pstSession->m_atbTmpAlmember, sizeof(TbAl_member), 1000/*MAX_ALLIANCE_MEMBER_NUM*/);
        if(dwRetCode > 0)
        {
            udwMemberNum = dwRetCode;

            //修正联盟成员数目
            if(udwMemberNum != pstSession->m_stUserInfo.m_tbAlliance.m_nMember)
            {
                pstSession->m_stUserInfo.m_tbAlliance.Set_Member(udwMemberNum);
                pstSession->m_stUserInfo.m_ucAllianceFlag = EN_TABLE_UPDT_FLAG__CHANGE;
            }

            // 修正联盟兵力
            TINT64 ddwAllMight = 0;
            TINT64 ddwForceKill = 0;
            for(TUINT32 udwIdx = 0; udwIdx < udwMemberNum; ++udwIdx)
            {
                if(pstSession->m_atbTmpAlmember[udwIdx].m_jProfile.isObject())
                {
                    ddwAllMight += pstSession->m_atbTmpAlmember[udwIdx].m_jProfile["force"].asInt64();
                    ddwForceKill += pstSession->m_atbTmpAlmember[udwIdx].m_jProfile["force_kill"].asInt64();
                }
            }

            if(0 < pstSession->m_stUserInfo.m_tbAlliance.m_nAid &&
                ddwAllMight != pstSession->m_stUserInfo.m_tbAlliance.m_nMight)
            {
                pstSession->m_stUserInfo.m_tbAlliance.Set_Might(ddwAllMight);
            }
            if(0 < pstSession->m_stUserInfo.m_tbAlliance.m_nAid &&
                ddwForceKill != pstSession->m_stUserInfo.m_tbAlliance.m_nForce_kill)
            {
                pstSession->m_stUserInfo.m_tbAlliance.Set_Force_kill(ddwForceKill);
            }
        }
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        if(ptbPlayer->m_nAlpos == EN_ALLIANCE_POS__CHANCELLOR && ptbAlliance->m_nMember > 1)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__CAN_NOT_LEAVE_ALLIANCE;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AllianceLeave: req param error [seq=%u]", 
                pstSession->m_stUserInfo.m_udwBSeqNo));
            return -4;
        }       

        if(CToolBase::IsOpCommand(pstSession->m_stReqParam.m_szCommand) == FALSE)
        {
            CSendMessageBase::AddTips(&pstSession->m_stUserInfo, EN_TIPS_TYPE__LEAVE_ALLIANCE_OK, ptbPlayer->m_nUid,
                FALSE, ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET, 0, 0, ptbPlayer->m_sAlname.c_str());
        }        

        // update user info
        ptbPlayer->Set_Alid(0);
        ptbPlayer->Set_Alpos(0);
        ptbPlayer->Set_Alname("");
        ptbPlayer->Set_Al_nick_name("");
        ptbPlayer->Set_Req_al(0);
        ptbPlayer->Set_Alname_update_time(CTimeUtils::GetUnixTime());

        pstSession->m_stUserInfo.m_ucPlayerFlag = EN_TABLE_UPDT_FLAG__CHANGE;

        for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwWildNum; ++udwIdx)
        {
            pstUser->m_atbWild[udwIdx].Set_Alid(0);
            pstUser->m_atbWild[udwIdx].Set_Alname("");
            pstUser->m_atbWild[udwIdx].Set_Al_nick("");
            pstUser->m_atbWild[udwIdx].Set_Al_flag(0);
            pstUser->m_atbWild[udwIdx].Set_Name_update_time(CTimeUtils::GetUnixTime());
        }

        // update alliance info
        if(ptbAlliance->m_nMember > 0)
        {
            ptbAlliance->Set_Member(ptbAlliance->m_nMember - 1);
        }
        if(ptbAlliance->m_nMight > ptbPlayer->m_nMight)
        {
            ptbAlliance->Set_Might(ptbAlliance->m_nMight - ptbPlayer->m_nMight);
        }
        else
        {
            ptbAlliance->Set_Might(0);
        }
        if(ptbAlliance->m_nForce_kill > ptbPlayer->m_nMkill)
        {
            ptbAlliance->Set_Force_kill(ptbAlliance->m_nForce_kill - ptbPlayer->m_nMkill);
        }
        else
        {
            ptbAlliance->Set_Force_kill(0);
        }
        if(ptbAlliance->m_nMember == 0)
        {
            pstSession->m_stUserInfo.m_ucAllianceFlag = EN_TABLE_UPDT_FLAG__DEL;
            CMsgBase::DelAlRank(ptbAlliance);
            if (pstSession->m_stUserInfo.m_tbSvrAl.m_nAlid == ptbAlliance->m_nAid)
            {
                pstSession->m_stUserInfo.m_aucSvrAlFlag = EN_TABLE_UPDT_FLAG__DEL;
            }
        }
        else
        {
            pstSession->m_stUserInfo.m_ucAllianceFlag = EN_TABLE_UPDT_FLAG__CHANGE;
        }

        // updt action
        PlayerLeaveAllianceUpdtAction(pstSession, pstSession->m_stUserInfo.m_tbPlayer.m_nUid);

        // clear player assist
        for(TUINT32 dwIdx = 0; dwIdx < pstUser->m_udwAlAssistAllNum; dwIdx++)
        {
            if(pstUser->m_atbAlAssistAll[dwIdx].m_nUid == pstSession->m_stUserInfo.m_tbPlayer.m_nUid)
            {
                pstUser->m_aucAlAssistAllFlag[dwIdx] = EN_TABLE_UPDT_FLAG__DEL;
            }
        }

        if(pstSession->m_stUserInfo.m_ucAllianceFlag != EN_TABLE_UPDT_FLAG__DEL)
        {
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__6;
        }
        else
        {
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;
        }
    }

    //解散联盟 删除nick name 和name信息
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        pstSession->ResetAwsInfo();

        TbUnique_name tbUniqueName;
        tbUniqueName.Set_Type(EN_ALLIANCE_NAME);
        tbUniqueName.Set_Name(CToolBase::ToLower(pstSession->m_stUserInfo.m_tbAlliance.m_sName));
        CAwsRequest::DeleteItem(pstSession, &tbUniqueName);

        tbUniqueName.Set_Type(EN_ALLIANCE_NICK_NAME);
        tbUniqueName.Set_Name(CToolBase::ToLower(pstSession->m_stUserInfo.m_tbAlliance.m_sAl_nick_name));
        CAwsRequest::DeleteItem(pstSession, &tbUniqueName);

        // send
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__4;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AllianceCreate: setp4 send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -6;
        }

        return 0;
    }

    //解散联盟时，获取应删除的diplomacy信息
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__4)
    {
        pstSession->ResetAwsInfo();

        //删除本联盟指向别人的diplomacy
        TbDiplomacy tbDiplomacy;
        tbDiplomacy.Set_Src_al(ptbAlliance->m_nAid);
        CAwsRequest::Query(pstSession, &tbDiplomacy, ETbDIPLOMACY_OPEN_TYPE_PRIMARY);

        //send
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__5;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AllianceCreate: setp5 send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -7;
        }
        return 0;
    }

    //根据query到的信息，删除相应diplomacy
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__5)
    {
        TbDiplomacy atbDiplomacyArray[MAX_DIPLOMACY_NUM];
        TUINT32 udwDipNum = 0;

        //解析回包，获得src_to_des字段
        for(TUINT32 idx = 0; idx < pstSession->m_vecAwsRsp.size(); idx++)
        {
            AwsRspInfo *pstAwsRspInfo = pstSession->m_vecAwsRsp[idx];
            dwRetCode = CAwsResponse::OnQueryRsp(*pstAwsRspInfo, atbDiplomacyArray, sizeof(TbDiplomacy), MAX_DIPLOMACY_NUM);
            if (dwRetCode > 0)
            {
                udwDipNum = dwRetCode;
            }
        }

        //根据获取到的src_to_des字段，删除相应的diplomacy
        if (udwDipNum == 0)
        {
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__6;
        }
        else
        {
            pstSession->ResetAwsInfo();
            TbDiplomacy tbDiplomacyItem;
            for (TUINT32 idx = 0; idx < udwDipNum; idx++)
            {
                CAwsRequest::DeleteItem(pstSession, &atbDiplomacyArray[idx]);
                tbDiplomacyItem.Reset();
                tbDiplomacyItem.Set_Src_al(atbDiplomacyArray[idx].m_nDes_al);
                tbDiplomacyItem.Set_Des_al(atbDiplomacyArray[idx].m_nSrc_al);
                CAwsRequest::DeleteItem(pstSession, &tbDiplomacyItem);
            }
            //send
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__6;
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
            bNeedResponse = TRUE;
            dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
            if(dwRetCode < 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AllianceCreate: setp6 send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                return -8;
            }
            return 0;
        }
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__6)
    {
        //al member
        pstSession->ResetAwsInfo();

        TbAl_member tbAlmember;
        tbAlmember.Set_Aid(ptbAlliance->m_nAid);
        tbAlmember.Set_Uid(ptbPlayer->m_nUid);
        CAwsRequest::DeleteItem(pstSession, &tbAlmember);

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__7;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AllianceMemberGet: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -9;
        }

        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__7)
    {
        if(CToolBase::IsOpCommand(pstSession->m_stReqParam.m_szCommand) == FALSE)
        {
//             ostringstream ossCustomizeParam;
//             ossCustomizeParam.str("");
//             ossCustomizeParam << EN_CHAT_SYS_MSG_LEAVE_FROM_ALLIANCE << "#" << ptbPlayer->m_nUid << "#" << ptbPlayer->m_sUin;
//             CSendMessageBase::SendSysMsgToChat(pstSession->m_stReqParam.m_udwSvrId, ptbAlliance->m_nAid, 1, ossCustomizeParam.str().c_str());
        }

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessAlliance::ProcessCmd_AllianceKickout(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    TbPlayer *ptbSelfPlayer = &pstSession->m_stUserInfo.m_tbPlayer;
    TINT64 ddwSelfAid = ptbSelfPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET;

    // 0. 获取输入参数
    TUINT32 udwTargetUserId = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);

    // 1. check param
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        if(ptbSelfPlayer->m_nAlpos == EN_ALLIANCE_POS__REQUEST)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NOT_IN_ALLIANCE;
            return -1;
        }

        // check param
        if(ptbSelfPlayer->m_nAlpos < EN_ALLIANCE_POS__VICE_CHANCELOR
        || ptbSelfPlayer->m_nUid == udwTargetUserId)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AllianceRequestPro: req param error [seq=%u]", \
                pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }

        // get target user info 
        pstSession->ResetAwsInfo();
        CAwsRequest::UserGetByUid(pstSession, udwTargetUserId);
        CAwsRequest::MapQueryByUid(pstSession, pstSession->m_stReqParam.m_udwSvrId, udwTargetUserId);
        CAwsRequest::MarchActionQueryBySuid(pstSession, pstSession->m_stReqParam.m_udwSvrId, udwTargetUserId);
        CAwsRequest::MarchActionQueryByTuid(pstSession, pstSession->m_stReqParam.m_udwSvrId, udwTargetUserId);
        TbAl_member tbAlmember;
        tbAlmember.Set_Aid(ddwSelfAid);
        tbAlmember.Set_Uid(udwTargetUserId);
        CAwsRequest::GetItem(pstSession, &tbAlmember, ETbALLIANCEMEMBER_OPEN_TYPE_PRIMARY);

        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AllianceKickout: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }

        return 0;
    }

    // 3. 处理响应
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        TbAl_member tbAlmember;
        tbAlmember.Reset();
        pstSession->m_tbTmpPlayer.Reset();
        AwsRspInfo *pstRes = NULL;
        for(TUINT32 dwIdx = 0; dwIdx < pstSession->m_vecAwsRsp.size(); dwIdx++)
        {
            pstRes = pstSession->m_vecAwsRsp[dwIdx];
            string strTableRawName = CCommonFunc::GetTableRawName(pstRes->sTableName);
            if(strTableRawName == EN_AWS_TABLE_PLAYER)
            {
                dwRetCode = CAwsResponse::OnGetItemRsp(*pstRes, &pstSession->m_tbTmpPlayer);
                continue;
            }
            if(strTableRawName == EN_AWS_TABLE_MAP)
            {
                dwRetCode = CAwsResponse::OnQueryRsp(*pstRes, pstSession->m_atbTmpWild, sizeof(TbMap), MAX_WILD_RETURN_NUM);
                if(dwRetCode > 0)
                {
                    pstSession->m_udwTmpWildNum = dwRetCode;
                }
                continue;
            }
            if(strTableRawName == EN_AWS_TABLE_MARCH_ACTION)
            {
                dwRetCode = CAwsResponse::OnQueryRsp(*pstRes, pstSession->m_atbTmpMarch + pstSession->m_udwTmpMarchNum, sizeof(TbMarch_action), MAX_USER_MARCH_NUM);
                if(dwRetCode > 0)
                {
                    pstSession->m_udwTmpMarchNum += dwRetCode;
                }
            }
            if(strTableRawName == EN_AWS_TABLE_AL_MEMBER)
            {
                dwRetCode = CAwsResponse::OnGetItemRsp(*pstRes, &tbAlmember);
            }
        }

        if(pstSession->m_tbTmpPlayer.m_nUid == 0
            || pstSession->m_tbTmpPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET != ddwSelfAid
            || pstSession->m_tbTmpPlayer.m_nAlpos >= ptbSelfPlayer->m_nAlpos)
        {
            if(tbAlmember.m_nUid == udwTargetUserId)
            {
                pstSession->ResetAwsInfo();
                CAwsRequest::DeleteItem(pstSession, &tbAlmember);
                // set next procedure
                pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;
                pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
                // send request
                bNeedResponse = TRUE;
                dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
                if(dwRetCode < 0)
                {
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AllianceKickout: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                    return -4;
                }

                return 0;
            }
            else
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
                if (pstSession->m_tbTmpPlayer.m_nUid != 0
                    && pstSession->m_tbTmpPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET != ddwSelfAid)
                {
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PLAYER_HAS_LEAVE_ALLIANCE;
                }
                return -5;
            }
        }
        else
        {
            pstSession->ResetAwsInfo();

            TbAl_member tbAlmember;
            tbAlmember.Set_Aid(ddwSelfAid);
            tbAlmember.Set_Uid(pstSession->m_tbTmpPlayer.m_nUid);
            CAwsRequest::DeleteItem(pstSession, &tbAlmember);

            pstSession->m_tbTmpPlayer.Set_Alid(0);
            pstSession->m_tbTmpPlayer.Set_Alpos(0);
            pstSession->m_tbTmpPlayer.Set_Alname("");
            pstSession->m_tbTmpPlayer.Set_Al_nick_name("");
            pstSession->m_tbTmpPlayer.Set_Req_al(0);
            pstSession->m_tbTmpPlayer.Set_Alname_update_time(CTimeUtils::GetUnixTime());
            CAwsRequest::UpdateItem(pstSession, &pstSession->m_tbTmpPlayer);

            const Json::Value& oWildResJson = CWildInfo::GetWildResInfo(ptbSelfPlayer->m_nSid);

            TUINT32 udwCurTime = CTimeUtils::GetUnixTime();

            for(TUINT32 dwIdx = 0; dwIdx < pstSession->m_udwTmpMarchNum; ++dwIdx)
            {
                TbMarch_action* ptbMarch = &pstSession->m_atbTmpMarch[dwIdx];
                TBOOL bIsChange = FALSE;

                //召回王座相关的reinforce
                if (CActionBase::IsPlayerOwnedAction(udwTargetUserId, ptbMarch->m_nId)
                    && ptbMarch->m_nMclass == EN_ACTION_MAIN_CLASS__MARCH
                    && ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_THRONE
                    && ptbMarch->m_nStatus != EN_MARCH_STATUS__RETURNING)
                {
                    if (ptbMarch->m_nStatus == EN_MARCH_STATUS__MARCHING)
                    {
                        CActionBase::ReturnMarchOnFly(ptbMarch);
                    }
                    else
                    {
                        CActionBase::ReturnMarch(ptbMarch);
                    }
                    bIsChange = TRUE;
                }

                //取消对王座的rally_war
                else if (CActionBase::IsPlayerOwnedAction(udwTargetUserId, ptbMarch->m_nId)
                    && ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR_THRONE
                    && ptbMarch->m_nStatus != EN_MARCH_STATUS__RETURNING)
                {
                    if (ptbMarch->m_nStatus == EN_MARCH_STATUS__MARCHING
                        || ptbMarch->m_nStatus == EN_MARCH_STATUS__FIGHTING)
                    {
                        ptbMarch->Set_Status(EN_MARCH_STATUS__DELING_ON_FLY);
                    }
                    else
                    {
                        ptbMarch->Set_Status(EN_MARCH_STATUS__DELING);
                    }
                    ptbMarch->m_bParam[0].m_ddwLoadTime = ptbMarch->m_nEtime;
                    ptbMarch->SetFlag(TbMARCH_ACTION_FIELD_PARAM);
                    ptbMarch->Set_Etime(udwCurTime);
                    ptbMarch->Set_Sal(0, UPDATE_ACTION_TYPE_DELETE);
                    ptbMarch->Set_Tal(0, UPDATE_ACTION_TYPE_DELETE);
                    //ptbMarch->DeleteField(TbMARCH_ACTION_FIELD_SAL);
                    //ptbMarch->DeleteField(TbMARCH_ACTION_FIELD_TAL);
                    bIsChange = TRUE;
                }
                //召回对盟友的王座rally_war的reinforce
//                 else if (CActionBase::IsPlayerOwnedAction(udwTargetUserId, ptbMarch->m_nId)
//                     && ptbMarch->m_nMclass == EN_ACTION_MAIN_CLASS__MARCH
//                     && ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_THRONE
//                     && ptbMarch->m_nTid != 0
//                     && ptbMarch->m_nStatus != EN_MARCH_STATUS__RETURNING)
//                 {
//                     TbMarch_action* ptbRallyWar = NULL;
//                     TINT32 dwRallyWarIndex = -1;
//                     dwRallyWarIndex = CActionBase::GetMarchIndex(pstSession->m_stUserInfo.m_atbMarch, pstSession->m_stUserInfo.m_udwMarchNum, ptbMarch->m_nTid);
//                     if (dwRallyWarIndex >= 0)
//                     {
//                         ptbRallyWar = &pstSession->m_stUserInfo.m_atbMarch[dwRallyWarIndex];
//                         pstSession->m_stUserInfo.m_aucMarchFlag[dwRallyWarIndex] = EN_TABLE_UPDT_FLAG__CHANGE;
// 
//                         if (ptbRallyWar->m_nStatus == EN_MARCH_STATUS__WAITING
//                             && ptbMarch->m_nStatus == EN_MARCH_STATUS__MARCHING)
//                         {
//                             ptbRallyWar->Set_Etime(udwCurTime);
//                         }
// 
//                         CActionBase::ReleaseSlot(ptbRallyWar, ptbMarch, TRUE);
//                         CActionBase::UpdateRallyForce(ptbRallyWar, ptbMarch, TRUE);
//                     }
// 
//                     if (ptbMarch->m_nStatus == EN_MARCH_STATUS__PREPARING)
//                     {
//                         CActionBase::ReturnMarch(ptbMarch);
//                     }
//                     else if (ptbMarch->m_nStatus == EN_MARCH_STATUS__MARCHING)
//                     {
//                         CActionBase::ReturnMarchOnFly(ptbMarch);
//                     }
//                     bIsChange = TRUE;
//                 }
                //召回自己派遣在王座的英雄
                else if (CActionBase::IsPlayerOwnedAction(udwTargetUserId, ptbMarch->m_nId)
                    && ptbMarch->m_nMclass == EN_ACTION_MAIN_CLASS__MARCH
                    && ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__ASSIGN_THRONE
                    && ptbMarch->m_nStatus != EN_MARCH_STATUS__RETURNING)
                {
                    if (ptbMarch->m_nStatus == EN_MARCH_STATUS__MARCHING)
                    {
                        CActionBase::ReturnMarchOnFly(ptbMarch);
                    }
                    else
                    {
                        CActionBase::ReturnMarch(ptbMarch);
                    }
                    bIsChange = TRUE;
                }

                //召回对王座/神像发起的普通进攻
                else if (CActionBase::IsPlayerOwnedAction(udwTargetUserId, ptbMarch->m_nId)
                    && ptbMarch->m_nMclass == EN_ACTION_MAIN_CLASS__MARCH
                    && (ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__ATTACK_IDOL
                    || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__ATTACK_THRONE)
                    && ptbMarch->m_nStatus != EN_MARCH_STATUS__RETURNING)
                {
                    CActionBase::ReturnMarchOnFly(ptbMarch);
                    bIsChange = TRUE;
                }
                else if (CActionBase::IsPlayerOwnedAction(udwTargetUserId, ptbMarch->m_nId)
                    && ptbMarch->m_nMclass == EN_ACTION_MAIN_CLASS__MARCH
                    && ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_REINFORCE
                    && ptbMarch->m_nStatus != EN_MARCH_STATUS__RETURNING)
                {
                    TbMarch_action* ptbRallyWar = NULL;
                    TINT32 dwRallyWarIndex = -1;
                    dwRallyWarIndex = CActionBase::GetMarchIndex(pstSession->m_stUserInfo.m_atbMarch, pstSession->m_stUserInfo.m_udwMarchNum, ptbMarch->m_nTid);
                    if (dwRallyWarIndex >= 0)
                    {
                        ptbRallyWar = &pstSession->m_stUserInfo.m_atbMarch[dwRallyWarIndex];
                        pstSession->m_stUserInfo.m_aucMarchFlag[dwRallyWarIndex] = EN_TABLE_UPDT_FLAG__CHANGE;

                        if (ptbRallyWar->m_nStatus == EN_MARCH_STATUS__WAITING
                            && ptbMarch->m_nStatus == EN_MARCH_STATUS__MARCHING)
                        {
                            ptbRallyWar->Set_Etime(udwCurTime);
                        }

                        CActionBase::ReleaseSlot(ptbRallyWar, ptbMarch, TRUE);
                        CActionBase::UpdateRallyForce(ptbRallyWar, ptbMarch, TRUE);
                    }

                    if (ptbMarch->m_nStatus == EN_MARCH_STATUS__PREPARING)
                    {
                        CActionBase::ReturnMarch(ptbMarch);
                    }
                    else if (ptbMarch->m_nStatus == EN_MARCH_STATUS__MARCHING)
                    {
                        CActionBase::ReturnMarchOnFly(ptbMarch);
                    }
                    bIsChange = TRUE;
                }
                else if(ptbMarch->m_nMclass == EN_ACTION_MAIN_CLASS__MARCH
                    && (ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_NORMAL
                    || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_REINFORCE
                    || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_THRONE)
                    && ptbMarch->m_nStatus != EN_MARCH_STATUS__RETURNING)
                {
                    TBOOL bIsSend = FALSE;
                    if (ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_NORMAL)
                    {
                        if (bIsSend == FALSE && ptbMarch->m_nTuid == udwTargetUserId)
                        {
                            bIsSend = TRUE;
                            CMsgBase::RefreshUserInfo(ptbMarch->m_nTuid);
                        }
                        else if (ptbMarch->m_nTuid != udwTargetUserId)
                        {
                            CMsgBase::RefreshUserInfo(ptbMarch->m_nTuid);
                        }
                    }
                    if (ptbMarch->m_nStatus == EN_MARCH_STATUS__MARCHING)
                    {
                        CActionBase::ReturnMarchOnFly(ptbMarch);
                    }
                    else
                    {
                        CActionBase::ReturnMarch(ptbMarch);
                    }
                    bIsChange = TRUE;
                }
                else if (CActionBase::IsPlayerOwnedAction(udwTargetUserId, ptbMarch->m_nId)
                    && ptbMarch->m_nMclass == EN_ACTION_MAIN_CLASS__MARCH
                    && ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__OCCUPY)
                {
                    if (ptbMarch->m_nStatus == EN_MARCH_STATUS__LOADING
                        || ptbMarch->m_nStatus == EN_MARCH_STATUS__PRE_LOADING)
                    {
                        TbMap *ptbWild = NULL;
                        for (TUINT32 udwMapIdx = 0; udwMapIdx < pstSession->m_udwTmpMarchNum; udwMapIdx++)
                        {
                            if (pstSession->m_atbTmpWild[udwMapIdx].m_nId == ptbMarch->m_nTpos
                                && pstSession->m_atbTmpWild[udwMapIdx].m_nUid == udwTargetUserId)
                            {
                                ptbWild = &pstSession->m_atbTmpWild[udwMapIdx];
                            }
                            if (ptbWild != NULL)
                            {
                                CCommonBase::UpdateOccupyLairStat(&pstSession->m_stUserInfo, ptbWild, ptbMarch->m_nId);
                            }
                        }
                    }
                    if (ptbMarch->m_nStatus == EN_MARCH_STATUS__LOADING)
                    {
                        if (EN_WILD_CLASS_MONSTER_NEST == oWildResJson[CCommonFunc::NumToString(ptbMarch->m_bParam[0].m_ddwTargetType)]["a0"]["a0"].asUInt())
                        {
                            ptbMarch->Set_Etime(udwCurTime);
                            bIsChange = TRUE;
                        }
                    }
                }
                else if (CActionBase::IsPlayerOwnedAction(udwTargetUserId, ptbMarch->m_nId)
                    && ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR
                    && ptbMarch->m_nStatus != EN_MARCH_STATUS__RETURNING)
                {
                    if (ptbMarch->m_nStatus == EN_MARCH_STATUS__MARCHING)
                    {
                        ptbMarch->Set_Status(EN_MARCH_STATUS__DELING_ON_FLY);
                    }
                    else
                    {
                        ptbMarch->Set_Status(EN_MARCH_STATUS__DELING);
                    }
                    ptbMarch->m_bParam[0].m_ddwLoadTime = ptbMarch->m_nEtime;
                    ptbMarch->SetFlag(TbMARCH_ACTION_FIELD_PARAM);
                    ptbMarch->Set_Etime(udwCurTime);
                    ptbMarch->Set_Sal(0, UPDATE_ACTION_TYPE_DELETE);
                    ptbMarch->Set_Tal(0, UPDATE_ACTION_TYPE_DELETE);
                    //ptbMarch->DeleteField(TbMARCH_ACTION_FIELD_SAL);
                    //ptbMarch->DeleteField(TbMARCH_ACTION_FIELD_TAL);

                    bIsChange = TRUE;
                }
                else if (ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR
                    && ptbMarch->m_nTuid == udwTargetUserId)
                {
                    ptbMarch->m_bParam[0].m_ddwTargetAlliance = 0;
                    ptbMarch->m_bParam[0].m_szTargetAlliance[0] = '\0';
                    ptbMarch->SetFlag(TbMARCH_ACTION_FIELD_PARAM);
                    ptbMarch->Set_Tal(-1 * ptbMarch->m_nTuid);

                    bIsChange = TRUE;
                }

                if (bIsChange)
                {
                    ExpectedItem expect_item;
                    ExpectedDesc expect_desc;
                    expect_item.SetVal(TbMARCH_ACTION_FIELD_ID, true, ptbMarch->m_nId); //加expect防止边界条件(action已经结束又被新建)
                    expect_desc.clear();
                    expect_desc.push_back(expect_item);

                    CAwsRequest::UpdateItem(pstSession, ptbMarch, expect_desc);

                    //wave@push_data
                    CPushDataBasic::PushDataMap_SingleAction(pstSession, ptbMarch, EN_TABLE_UPDT_FLAG__CHANGE);
                    CPushDataBasic::PushDataUid_MarchActionSourceUid(pstSession, ptbMarch->m_nSuid, ptbMarch, EN_TABLE_UPDT_FLAG__CHANGE);
                }
                ptbMarch->Reset();
            }
            pstSession->m_udwTmpMarchNum = 0;

            for (TUINT32 dwIdx = 0; dwIdx < pstSession->m_udwTmpWildNum; ++dwIdx)
            {
                if (pstSession->m_atbTmpWild[dwIdx].m_nUid == udwTargetUserId)
                {
                    pstSession->m_atbTmpWild[dwIdx].Set_Alid(0);
                    pstSession->m_atbTmpWild[dwIdx].Set_Alname("");
                    pstSession->m_atbTmpWild[dwIdx].Set_Al_nick("");
                    pstSession->m_atbTmpWild[dwIdx].Set_Al_flag(0);
                    pstSession->m_atbTmpWild[dwIdx].Set_Name_update_time(CTimeUtils::GetUnixTime());
                }
                CAwsRequest::UpdateItem(pstSession, &pstSession->m_atbTmpWild[dwIdx]);
                //wave@push_data
                CPushDataBasic::PushDataMap_SingleWild(pstSession, &pstSession->m_atbTmpWild[dwIdx]);

                pstSession->m_atbTmpWild[dwIdx].Reset();
            }
            pstSession->m_udwTmpWildNum = 0;

            for(TUINT32 dwIdx = 0; dwIdx < pstSession->m_stUserInfo.m_udwAlAssistAllNum; ++dwIdx)
            {
                if(pstSession->m_stUserInfo.m_atbAlAssistAll[dwIdx].m_nUid == udwTargetUserId)
                {
                    pstSession->m_stUserInfo.m_aucAlAssistAllFlag[dwIdx] = EN_TABLE_UPDT_FLAG__DEL;
                }
            }

            //CProcessAlliance::PlayerLeaveAllianceUpdtAction(pstSession, udwTargetUserId);

            // set next procedure
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
            // send request
            bNeedResponse = TRUE;
            dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
            if(dwRetCode < 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AllianceKickout: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                return -6;
            }

            return 0;
        }
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        if(pstSession->m_stUserInfo.m_tbAlliance.m_nMember > 0)
        {
            pstSession->m_stUserInfo.m_tbAlliance.Set_Member(pstSession->m_stUserInfo.m_tbAlliance.m_nMember - 1);
            pstSession->m_stUserInfo.m_ucAllianceFlag = EN_TABLE_UPDT_FLAG__CHANGE;
        }

        CMsgBase::SendOperateMail(udwTargetUserId, EN_MAIL_ID__BE_DISMISS, ptbSelfPlayer->Get_Sid(),
            SYSTEM_ENCOURAGE, pstSession->m_stUserInfo.m_tbAlliance.m_sName.c_str(), "", "");

        //tips
        CSendMessageBase::AddTips(&pstSession->m_stUserInfo, EN_TIPS_TYPY__REMOVED_FROM_ALLIANCE, udwTargetUserId, TRUE,
            pstSession->m_stUserInfo.m_tbAlliance.m_nAvatar, 0, 0, pstSession->m_stUserInfo.m_tbAlliance.m_sName.c_str());

        //wave@push_data
        CPushDataBasic::PushDataUid_Refresh(pstSession, udwTargetUserId);
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        //do nothing
    }

//     ostringstream ossCustomizeParam;
//     ossCustomizeParam.str("");
//     ossCustomizeParam << EN_CHAT_SYS_MSG_KICK_FROM_ALLIANCE << "#" << udwTargetUserId << "#" << pstSession->m_tbTmpPlayer.m_sUin;
//     CSendMessageBase::SendSysMsgToChat(pstSession->m_stReqParam.m_udwSvrId, ddwSelfAid, 1, ossCustomizeParam.str().c_str());


    pstSession->m_udwCommandStep = EN_COMMAND_STEP__INIT;
    pstSession->m_udwCommand = EN_CLIENT_REQ_COMMAND__ALLIANCE_MEMBER_GET;
    pstSession->m_bGotoOtherCmd = TRUE;
    strcpy(pstSession->m_stReqParam.m_szKey[9], "1");
    return 0;
}

TINT32 CProcessAlliance::ProcessCmd_AllianceGetInfo(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;

    TUINT32 udwAlid = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        if(udwAlid == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -1;
        }

        pstSession->ResetAwsInfo();
        CAwsRequest::AllianceGetByAid(pstSession, udwAlid);

        // send request
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            return -2;
        }

        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        pstSession->m_tbTmpAlliance.Reset();
        CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[0], &pstSession->m_tbTmpAlliance);

        if(pstSession->m_tbTmpAlliance.m_nAid == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ALLIANCE_NOT_EXIST;
            return -3;
        }

        // set next procedure
        pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_ALLIANCE_LIST;
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;

        return 0;
    }

    pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_ALLIANCE_LIST;
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessAlliance::ProcessCmd_AllianceWallMsgTop(SSession *pstSession, TBOOL &bNeedResponse)
{
    //TODO 根据表来重写..
    // key0 = al_comment的id
    TINT64 ddwMsgId = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    // key1 = 是否需要置顶(0: 取消置顶; 1: 设置置顶)
    TUINT32 udwTopFlag = strtoul(pstSession->m_stReqParam.m_szKey[1], NULL, 10);

    CGameInfo *poGameInfo = CGameInfo::GetInstance();

    if(EN_ALLIANCE_POS__REQUEST == pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NOT_IN_ALLIANCE;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AllianceWallMsgTop: player not in alliance [seq=%u]", \
                                                pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }

    if(pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos != EN_ALLIANCE_POS__VICE_CHANCELOR
       && pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos != EN_ALLIANCE_POS__CHANCELLOR)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__HAVE_NO_PERMISSION;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AllianceWallMsgTop: player not a alliance manager [seq=%u]", \
                                                pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }

    

    if(0 != udwTopFlag && 1 != udwTopFlag)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AllianceWallMsgTop: topflag error [topflag=%u] [seq=%u]", \
                                                udwTopFlag, \
                                                pstSession->m_stUserInfo.m_udwBSeqNo));
        return -2;
    }

    TINT32 ddwTopAlCommentNum = 0;
    for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_stUserInfo.m_udwWallNum; ++udwIdx)
    {
        if(1 == pstSession->m_stUserInfo.m_atbWall[udwIdx].Get_Topflag())
        {
            ddwTopAlCommentNum++;
        }
    }
    
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_AllianceWallMsgTop: TopAlCommentNum in game [TopAlCommentNum=%d] [seq=%u]", \
                                            poGameInfo->GetTopAlCommentNum(), \
                                            pstSession->m_stUserInfo.m_udwBSeqNo));
    if(1 == udwTopFlag && ddwTopAlCommentNum >= poGameInfo->GetTopAlCommentNum())
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__AL_COMMENT_IS_FULL;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AllianceWallMsgTop: top al_comment num error [ddwTopAlCommentNum=%d] [seq=%u]", \
                                                ddwTopAlCommentNum, \
                                                pstSession->m_stUserInfo.m_udwBSeqNo));
        return -3;
    }


    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    TbAl_wall *ptbWall = NULL;
    TBOOL bIsFind = FALSE;
    for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_stUserInfo.m_udwWallNum; ++udwIdx)
    {
        ptbWall = &pstSession->m_stUserInfo.m_atbWall[udwIdx];
        if(ptbWall->Get_Wall_id() == ddwMsgId && ptbWall->Get_Topflag() != udwTopFlag)
        {
            ptbWall->Set_Alid(pstSession->m_stUserInfo.m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET);
            if (udwTopFlag == 1)
            {
                ptbWall->Set_Wall_id(ddwMsgId + (WALL_MSG_OFFSET << 32));
            }
            else
            {
                ptbWall->Set_Wall_id(ddwMsgId - (WALL_MSG_OFFSET << 32));
            }
            ptbWall->Set_Topflag(udwTopFlag);
            ptbWall->Set_Toptime(udwCurTime);
            ptbWall->SetFlag(TbAL_WALL_FIELD_TIME);
            ptbWall->SetFlag(TbAL_WALL_FIELD_ALPOS);
            ptbWall->SetFlag(TbAL_WALL_FIELD_UID);
            ptbWall->SetFlag(TbAL_WALL_FIELD_UIN);
            ptbWall->SetFlag(TbAL_WALL_FIELD_CONTENT);
            ptbWall->SetFlag(TbAL_WALL_FIELD_AVATAR);
            ptbWall->SetFlag(TbAL_WALL_FIELD_RAW_LANG);
            ptbWall->SetFlag(TbAL_WALL_FIELD_TRANSLATE_CONTENT);

            pstSession->m_stUserInfo.m_aucWallFlag[udwIdx] = EN_TABLE_UPDT_FLAG__NEW;            
            TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessCmd_AllianceWallMsgTop: al_msg_wall top [msgid=%ld] [seq=%u]", \
                                                   ddwMsgId, \
                                                   pstSession->m_stUserInfo.m_udwBSeqNo));

            pstSession->m_stUserInfo.m_tbTmpWall.Set_Alid(pstSession->m_stUserInfo.m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET);
            pstSession->m_stUserInfo.m_tbTmpWall.Set_Wall_id(ddwMsgId);
            pstSession->m_stUserInfo.m_ucTmpWallFlag = EN_TABLE_UPDT_FLAG__DEL;
        }

        if (ptbWall->Get_Wall_id() == ddwMsgId)
        {
            bIsFind = TRUE;
        }
        else if (udwTopFlag == 1 && ptbWall->Get_Wall_id() - (WALL_MSG_OFFSET << 32) == ddwMsgId)
        {
            bIsFind = TRUE;
        }
        else if (udwTopFlag == 0 && ptbWall->Get_Wall_id() + (WALL_MSG_OFFSET << 32) == ddwMsgId)
        {
            bIsFind = TRUE;
        }
    }

    
    if (bIsFind == FALSE)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__WALL_NOT_EXIST;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AllianceWallMsgTop: al_msg_wall not exist [msgid=%ld] [seq=%u]", 
            ddwMsgId, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }

    pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_WALL;
    return 0;

}

TINT32 CProcessAlliance::ProcessCmd_AllianceWallMagGet(SSession *pstSession, TBOOL &bNeedResponse)
{
    if(pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos == EN_ALLIANCE_POS__REQUEST)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NOT_IN_ALLIANCE;
        return -1;
    }

    TbUser_stat& tbUserStat = pstSession->m_stUserInfo.m_tbUserStat;
    tbUserStat.Set_Wall_get_t(CTimeUtils::GetUnixTime());
    pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_WALL;
    return 0;
}

TINT32 CProcessAlliance::ProcessCmd_AllianceRequestGet(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;

    if(pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos == EN_ALLIANCE_POS__REQUEST)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NOT_IN_ALLIANCE;
        return -1;
    }

    if(pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos < EN_ALLIANCE_POS__VICE_CHANCELOR)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -2;
    }

    // 1. 发送获取数据的请求
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        // get package
        pstSession->ResetAwsInfo();

        TbAl_member tbAlmember;
        tbAlmember.Set_Aid(-1 * pstSession->m_stUserInfo.m_tbAlliance.m_nAid);
        CompareDesc comDesc;
        comDesc.dwCompareType = COMPARE_TYPE_GT;

        CAwsRequest::Query(pstSession, &tbAlmember, ETbALLIANCEMEMBER_OPEN_TYPE_PRIMARY, comDesc, TRUE, TRUE, FALSE, MAX_ALLIANCE_REQUEST_NUM);

        // send request
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AllianceRequestGet: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }
        return 0;
    }

    // 2. 解析获取的数据
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        // get data info
        TINT32 dwResNum = 0;
        dwResNum = CAwsResponse::OnQueryRsp(*pstSession->m_vecAwsRsp[0], pstSession->m_atbTmpAlmember,
            sizeof(TbAl_member), MAX_ALLIANCE_REQUEST_NUM);
        if(dwResNum > 0)
        {
            pstSession->m_udwTmpAlmemberNum = dwResNum;
        }
        else
        {
            pstSession->m_udwTmpAlmemberNum = 0;
        }

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_PLAYER_LIST;
    }
    return 0;
}

TINT32 CProcessAlliance::ProcessCmd_AllianceRequestAllow(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    TbAlliance* ptbSelfAlliance = &pstSession->m_stUserInfo.m_tbAlliance;
    TUINT32 udwAllianceId = ptbSelfAlliance->m_nAid;

    TCHAR *pszPlayerIdList = pstSession->m_stReqParam.m_szKey[0];

    if(pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos == EN_ALLIANCE_POS__REQUEST)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NOT_IN_ALLIANCE;
        return -1;
    }

    // 1. 发送查询兼更新请求
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        // check param
        pstSession->m_udwMailRUidNum = 0;
        TCHAR *pCur = pszPlayerIdList;
        while(*pCur)
        {
            pstSession->m_audwMailRUidList[pstSession->m_udwMailRUidNum] = strtoul(pCur, NULL, 10);
            if(pstSession->m_audwMailRUidList[pstSession->m_udwMailRUidNum] != 0)
            {
                pstSession->m_udwMailRUidNum++;
                if(pstSession->m_udwMailRUidNum >= DEFAULT_PERPAGE_NUM)
                {
                    break;
                }
            }

            pCur = strchr(pCur, ':');
            if(pCur)
            {
                pCur++;
            }
            else
            {
                break;
            }
        }

        // no request
        if(pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos < EN_ALLIANCE_POS__VICE_CHANCELOR
        || pstSession->m_udwMailRUidNum == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AllianceRequestAllow: req param error [seq=%u]", \
                pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }

        if(ptbSelfAlliance->m_nMember + pstSession->m_udwMailRUidNum > MAX_ALLIANCE_MEMBER_NUM)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__YOUR_ALLIANCE_MEMBER_OVER_LOAD;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AllianceRequestAllow: alliance member num will overload [seq=%u]", \
                pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }

        // get package
        pstSession->ResetAwsInfo();
        TbPlayer tbPlayerItem;
        TbAl_member tbAlmember;
        ExpectedDesc expDesc;
        expDesc.vecExpectedItem.push_back(ExpectedItem(TbPLAYER_FIELD_ALID, TRUE, CCommonBase::GetPlayerAllianceId(udwAllianceId, EN_ALLIANCE_POS__REQUEST)));
        expDesc.vecExpectedItem.push_back(ExpectedItem(TbPLAYER_FIELD_SID, TRUE, ptbSelfAlliance->m_nSid));
        for(TUINT32 dwIdx = 0; dwIdx < pstSession->m_udwMailRUidNum; dwIdx++)
        {
            tbPlayerItem.Set_Uid(pstSession->m_audwMailRUidList[dwIdx]);
            tbPlayerItem.Set_Alname(pstSession->m_stUserInfo.m_tbAlliance.m_sName);
            tbPlayerItem.Set_Al_nick_name(pstSession->m_stUserInfo.m_tbAlliance.m_sAl_nick_name);
            tbPlayerItem.Set_Alid(CCommonBase::GetPlayerAllianceId(pstSession->m_stReqParam.m_udwAllianceId, EN_ALLIANCE_POS__MEMBER));
            tbPlayerItem.Set_Alpos(EN_ALLIANCE_POS__MEMBER);
            tbPlayerItem.Set_Al_time(CTimeUtils::GetUnixTime());
            tbPlayerItem.Set_Invite_mail_time(CTimeUtils::GetUnixTime());
            tbPlayerItem.Set_Req_al(0);
            tbPlayerItem.Set_Join_alliance(1);
            tbPlayerItem.Set_Alname_update_time(CTimeUtils::GetUnixTime());
            CAwsRequest::UpdateItem(pstSession, &tbPlayerItem, expDesc, RETURN_VALUES_ALL_NEW);

            tbAlmember.Reset();
            tbAlmember.Set_Aid((TINT32)udwAllianceId * -1);
            tbAlmember.Set_Uid(pstSession->m_audwMailRUidList[dwIdx]);
            CAwsRequest::DeleteItem(pstSession, &tbAlmember);
        }

        // send request
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AllianceRequestAllow: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -4;
        }

        return 0;
    }

    // 2. 解析结果
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        // parse data
        pstSession->m_udwMailRUidNum = 0;
        for(TUINT32 dwIdx = 0; dwIdx < pstSession->m_vecAwsRsp.size(); dwIdx++)
        {
            AwsRspInfo *pRspInfo = pstSession->m_vecAwsRsp[dwIdx];
            string strTableRawName = CCommonFunc::GetTableRawName(pRspInfo->sTableName);
            if (strTableRawName == EN_AWS_TABLE_PLAYER)
            {
                dwRetCode = CAwsResponse::OnUpdateItemRsp(*pRspInfo, &pstSession->m_stUserInfo.m_atbAllianceMember[pstSession->m_udwMailRUidNum]);
                if (dwRetCode > 0 && pstSession->m_stUserInfo.m_atbAllianceMember[pstSession->m_udwMailRUidNum].m_nUid > 0)
                {
                    pstSession->m_audwMailRUidList[pstSession->m_udwMailRUidNum] = pstSession->m_stUserInfo.m_atbAllianceMember[pstSession->m_udwMailRUidNum].m_nUid;
                    pstSession->m_astrNameList[pstSession->m_udwMailRUidNum] = pstSession->m_stUserInfo.m_atbAllianceMember[pstSession->m_udwMailRUidNum].m_sUin;
                    pstSession->m_udwMailRUidNum++;
                }
            }
        }
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_AllianceRequestAllow: updt %u [seq=%u]", pstSession->m_udwMailRUidNum, pstSession->m_stUserInfo.m_udwBSeqNo));
        if(pstSession->m_udwMailRUidNum == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQUEST_CHANGED;
            return -5;
        }

        pstSession->ResetAwsInfo();
        for(TUINT32 dwIdx = 0; dwIdx < pstSession->m_udwMailRUidNum; dwIdx++)
        {
            TbMap tbMapItem;
            tbMapItem.Set_Sid(pstSession->m_stReqParam.m_udwSvrId);
            tbMapItem.Set_Uid(pstSession->m_audwMailRUidList[dwIdx]);
            CAwsRequest::Query(pstSession, &tbMapItem, ETbMAP_OPEN_TYPE_GLB_UID, CompareDesc(), false);

            ptbSelfAlliance->m_nMember++;
            ptbSelfAlliance->m_nMight += pstSession->m_stUserInfo.m_atbAllianceMember[dwIdx].m_nMight;

            CSendMessageBase::AddTips(&pstSession->m_stUserInfo, EN_TIPS_TYPE__JOINED_ALLIANCE, pstSession->m_audwMailRUidList[dwIdx],
                TRUE, udwAllianceId, 0, 0, ptbSelfAlliance->m_sName.c_str());
        }
        ptbSelfAlliance->SetFlag(TbALLIANCE_FIELD_MEMBER);
        ptbSelfAlliance->SetFlag(TbALLIANCE_FIELD_MIGHT);

        // send request
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AllianceRequestAllow: update map send info failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -6;
        }
        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        pstSession->m_udwTmpWildNum = 0;
        for(TUINT32 dwIdx = 0; dwIdx < pstSession->m_vecAwsRsp.size(); dwIdx++)
        {
            AwsRspInfo *pRspInfo = pstSession->m_vecAwsRsp[dwIdx];
            dwRetCode = CAwsResponse::OnQueryRsp(*pRspInfo, &pstSession->m_atbTmpWild[pstSession->m_udwTmpWildNum], sizeof(TbMap), MAX_WILD_RETURN_NUM - pstSession->m_udwTmpWildNum);
            if(dwRetCode > 0)
            {
                pstSession->m_udwTmpWildNum += dwRetCode;
            }
        }

        // updt map
        pstSession->ResetAwsInfo();
        for(TUINT32 dwIdx = 0; dwIdx < pstSession->m_udwTmpWildNum; dwIdx++)
        {
            TbMap* ptbWild = &pstSession->m_atbTmpWild[dwIdx];
            ptbWild->Set_Alid(udwAllianceId);
            ptbWild->Set_Alname(ptbSelfAlliance->m_sName);
            ptbWild->Set_Al_nick(ptbSelfAlliance->m_sAl_nick_name);
            ptbWild->Set_Al_flag(ptbSelfAlliance->m_nAvatar);
            ptbWild->Set_Name_update_time(CTimeUtils::GetUnixTime());

            TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd__AllianceRequestPro: updt wild[%ld, %ld, %s] [seq=%u]",
                ptbWild->m_nId, ptbWild->m_nAlid, ptbWild->m_sAlname.c_str(), pstSession->m_stUserInfo.m_udwBSeqNo));

            CAwsRequest::UpdateItem(pstSession, ptbWild);

            ptbWild->Reset();
        }
        pstSession->m_udwTmpWildNum = 0;

        for(TUINT32 dwIdx = 0; dwIdx < pstSession->m_udwMailRUidNum; dwIdx++)
        {
            TbAl_member tbAlmember;
            tbAlmember.Set_Aid(udwAllianceId);
            tbAlmember.Set_Uid(pstSession->m_audwMailRUidList[dwIdx]);
            tbAlmember.Set_Al_pos(EN_ALLIANCE_POS__MEMBER);
            Json::Value tmpProfile;
            CCommJson::GenAlMemberInfo(&pstSession->m_stUserInfo.m_atbAllianceMember[dwIdx], tmpProfile);
            tbAlmember.Set_Profile(tmpProfile);
            tbAlmember.Set_Req_time(0);
            CAwsRequest::UpdateItem(pstSession, &tbAlmember);

            TbUser_stat tbUserStat;
            tbUserStat.Set_Uid(pstSession->m_audwMailRUidList[dwIdx]);
            tbUserStat.Set_Al_event_tips_time(CTimeUtils::GetUnixTime());
            CAwsRequest::UpdateItem(pstSession, &tbUserStat);
        }

        // send request
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AllianceRequestPro: update map send info failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -7;
        }
        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        for(TUINT32 dwIdx = 0; dwIdx < pstSession->m_udwMailRUidNum; dwIdx++)
        {
            ostringstream ossCustomizeParam;
            ossCustomizeParam.str("");
            ossCustomizeParam << EN_CHAT_SYS_MSG_JOIN_TO_ALLIANCE << "#" << pstSession->m_audwMailRUidList[dwIdx] << "#"<< pstSession->m_astrNameList[dwIdx];            
            CSendMessageBase::SendSysMsgToChat(ptbSelfAlliance->m_nSid, ptbSelfAlliance->m_nAid, 1, ossCustomizeParam.str().c_str());
        }

    
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__INIT;
        pstSession->m_udwCommand = EN_CLIENT_REQ_COMMAND__ALLIANCE_REQUEST_GET;
        pstSession->m_bGotoOtherCmd = TRUE;
        return 0;
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__INIT;
    pstSession->m_udwCommand = EN_CLIENT_REQ_COMMAND__ALLIANCE_REQUEST_GET;
    pstSession->m_bGotoOtherCmd = TRUE;
    return 0;
}

TINT32 CProcessAlliance::ProcessCmd_AllianceRequestReject(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    TbAlliance* ptbSelfAlliance = &pstSession->m_stUserInfo.m_tbAlliance;
    TUINT32 udwAllianceId = ptbSelfAlliance->m_nAid;

    if(pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos == EN_ALLIANCE_POS__REQUEST)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NOT_IN_ALLIANCE;
        return -1;
    }

    // 0. 获取输入参数
    TCHAR *pszPlayerIdList = pstSession->m_stReqParam.m_szKey[0];

    // 1. 发送查询兼更新请求
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        // check param
        pstSession->m_udwMailRUidNum = 0;
        TCHAR *pCur = pszPlayerIdList;
        while(*pCur)
        {
            pstSession->m_audwMailRUidList[pstSession->m_udwMailRUidNum] = strtoul(pCur, NULL, 10);
            if(pstSession->m_audwMailRUidList[pstSession->m_udwMailRUidNum] != 0)
            {
                pstSession->m_udwMailRUidNum++;
                if(pstSession->m_udwMailRUidNum >= DEFAULT_PERPAGE_NUM)
                {
                    break;
                }
            }

            pCur = strchr(pCur, ':');
            if(pCur)
            {
                pCur++;
            }
            else
            {
                break;
            }
        }

        // no request
        if(pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos < EN_ALLIANCE_POS__VICE_CHANCELOR
        || pstSession->m_udwMailRUidNum == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AllianceRequestPro: req param error [seq=%u]", \
                pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }

        // get package
        pstSession->ResetAwsInfo();
        TbPlayer tbPlayerItem;
        ExpectedDesc expDesc;
        expDesc.vecExpectedItem.push_back(ExpectedItem(TbPLAYER_FIELD_ALID, TRUE, CCommonBase::GetPlayerAllianceId(udwAllianceId, EN_ALLIANCE_POS__REQUEST)));
        for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_udwMailRUidNum; udwIdx++)
        {
            tbPlayerItem.Set_Uid(pstSession->m_audwMailRUidList[udwIdx]);
            tbPlayerItem.Set_Alid(0);
            tbPlayerItem.Set_Alpos(0);
            tbPlayerItem.Set_Req_al(0);
            tbPlayerItem.Set_Alname("");
            tbPlayerItem.Set_Al_nick_name("");
            tbPlayerItem.Set_Alname_update_time(CTimeUtils::GetUnixTime());
            CAwsRequest::UpdateItem(pstSession, &tbPlayerItem, expDesc, RETURN_VALUES_ALL_NEW);
        }

        // send request
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AllianceRequestPro: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }

        return 0;
    }

    // 2. 解析结果
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        // 删除已拒绝的用户
        for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_udwMailRUidNum; ++udwIdx)
        {
            TbAl_member tbAlmember;
            tbAlmember.Set_Aid((TINT32)udwAllianceId * -1);
            tbAlmember.Set_Uid(pstSession->m_audwMailRUidList[udwIdx]);
            CAwsRequest::DeleteItem(pstSession, &tbAlmember);
        }
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd__AllianceRequestPro: updt %u [seq=%u]", pstSession->m_udwMailRUidNum, pstSession->m_stUserInfo.m_udwBSeqNo));

        // 查看响应
        pstSession->m_udwMailRUidNum = 0;
        TbPlayer tbTmpPlayer;
        for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecAwsRsp.size(); udwIdx++)
        {
            tbTmpPlayer.Reset();
            AwsRspInfo *pRspInfo = pstSession->m_vecAwsRsp[udwIdx];
            dwRetCode = CAwsResponse::OnUpdateItemRsp(*pRspInfo, &tbTmpPlayer);
            if(dwRetCode > 0 && tbTmpPlayer.m_nUid > 0)
            {
                pstSession->m_audwMailRUidList[pstSession->m_udwMailRUidNum++] = tbTmpPlayer.m_nUid;
            }
        }
        
        for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_udwMailRUidNum; ++udwIdx)
        {
            // 发送邮件
            CMsgBase::SendOperateMail(pstSession->m_audwMailRUidList[udwIdx], EN_MAIL_ID__REQUEST_IS_DECLINED, pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE, ptbSelfAlliance->m_sName.c_str(), "", "");
        }

        pstSession->m_udwMailRUidNum = 0;
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AllianceRequestPro: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }

        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__INIT;
        pstSession->m_udwCommand = EN_CLIENT_REQ_COMMAND__ALLIANCE_REQUEST_GET;
        pstSession->m_bGotoOtherCmd = TRUE;
        return 0;
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__INIT;
    pstSession->m_udwCommand = EN_CLIENT_REQ_COMMAND__ALLIANCE_REQUEST_GET;
    pstSession->m_bGotoOtherCmd = TRUE;
    return 0;
}

TINT32 CProcessAlliance::ProcessCmd_AllianceMemberGet(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;

    // 0. 获取输入参数
    TUINT32 udwAllianceId = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TUINT32 udwOtherCmdJump = atoi(pstSession->m_stReqParam.m_szKey[9]);
    if(udwOtherCmdJump == 1)
    {
        udwAllianceId = 0;
    }

    if(udwAllianceId == 0 && pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos == EN_ALLIANCE_POS__REQUEST)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NOT_IN_ALLIANCE;
        return -1;
    }
    else if(udwAllianceId == 0)
    {
        udwAllianceId = pstSession->m_stUserInfo.m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET;
    }

    // 1. 发送获取数据的请求
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        // reset req
        pstSession->ResetAwsReq();

        // set player get
        CAwsRequest::AllAlMemberQuery(pstSession, udwAllianceId);

        // send request
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AllianceMemberGet: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }

        return 0;
    }

    // 2. 解析获取的数据
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        // get data info
        TUINT32 udwMemberNum = 0;
        dwRetCode = CAwsResponse::OnQueryRsp(*pstSession->m_vecAwsRsp[0],
            pstSession->m_atbTmpAlmember, sizeof(TbAl_member), 1000/*MAX_ALLIANCE_MEMBER_NUM*/);
        if(dwRetCode > 0)
        {
            udwMemberNum = dwRetCode;
            pstSession->m_udwTmpAlmemberNum = dwRetCode;

            if(udwAllianceId == pstSession->m_stUserInfo.m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET
            && pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos > EN_ALLIANCE_POS__REQUEST)
            {
                //修正联盟成员数目
                if(udwMemberNum != pstSession->m_stUserInfo.m_tbAlliance.m_nMember)
                {
                    pstSession->m_stUserInfo.m_tbAlliance.Set_Member(udwMemberNum);
                    pstSession->m_stUserInfo.m_ucAllianceFlag = EN_TABLE_UPDT_FLAG__CHANGE;
                }

                // 修正联盟兵力
                TINT64 ddwAllMight = 0;
                TINT64 ddwForceKill = 0;
                for(TUINT32 udwIdx = 0; udwIdx < udwMemberNum; ++udwIdx)
                {
                    if(pstSession->m_atbTmpAlmember[udwIdx].m_jProfile.isObject())
                    {
                        ddwAllMight += pstSession->m_atbTmpAlmember[udwIdx].m_jProfile["force"].asInt64();
                        ddwForceKill += pstSession->m_atbTmpAlmember[udwIdx].m_jProfile["force_kill"].asInt64();
                    }
                }

                if(0 < pstSession->m_stUserInfo.m_tbAlliance.m_nAid &&
                    ddwAllMight != pstSession->m_stUserInfo.m_tbAlliance.m_nMight)
                {
                    pstSession->m_stUserInfo.m_tbAlliance.Set_Might(ddwAllMight);
                }
                if(0 < pstSession->m_stUserInfo.m_tbAlliance.m_nAid &&
                    ddwForceKill != pstSession->m_stUserInfo.m_tbAlliance.m_nForce_kill)
                {
                    pstSession->m_stUserInfo.m_tbAlliance.Set_Force_kill(ddwForceKill);
                }
            }
        }
        else
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ALLIANCE_NOT_EXIST;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AllianceMemberGet: alliance member is zero [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_PLAYER_LIST;
        return 0;
    }
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessAlliance::ProcessCmd_AlliancePosChange(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    TbPlayer *pstPlayer = &pstSession->m_stUserInfo.m_tbPlayer;
    TbAlliance *pstAlliance = &pstSession->m_stUserInfo.m_tbAlliance;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;

    if(pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos == EN_ALLIANCE_POS__REQUEST)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NOT_IN_ALLIANCE;
        return -1;
    }

    // 0. 获取输入参数
    TUINT32 udwAllianceId = pstSession->m_stReqParam.m_udwAllianceId;
    TUINT32 udwUserId = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TUINT8 ucFromPosition = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TUINT8 ucPosition = atoi(pstSession->m_stReqParam.m_szKey[2]);

    // 1. check param
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        // check param
        if((pstPlayer->m_nAlpos == EN_ALLIANCE_POS__VICE_CHANCELOR && pstPlayer->m_nAlpos <= ucPosition)
            || pstPlayer->m_nAlpos < EN_ALLIANCE_POS__VICE_CHANCELOR
            || pstPlayer->m_nUid == udwUserId)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__HAVE_NO_PERMISSION;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AllianceRequestPro: req param error [seq=%u]", 
                pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
        else if(ucPosition > EN_ALLIANCE_POS__CHANCELLOR)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AllianceRequestPro: req pos error [seq=%u]", 
                pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        if (ucPosition == EN_ALLIANCE_POS__CHANCELLOR && pstAlliance->m_nAid == pstSession->m_tbThrone.m_nAlid)
        {
            //需要判断其peace_time状态
            CAwsRequest::UserGetByUid(pstSession, udwUserId);
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
            bNeedResponse = TRUE;
            dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
            if (dwRetCode < 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AlliancePosChange: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                return -3;
            }
            return 0;
        }
    }

    // 2. 发送查询请求
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        if (ucPosition == EN_ALLIANCE_POS__CHANCELLOR && pstAlliance->m_nAid == pstSession->m_tbThrone.m_nAlid)
        {
            TbPlayer tbTmpPlayer;
            CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[0], &tbTmpPlayer);
            if (tbTmpPlayer.m_nUid != udwUserId)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
                return -1;
            }
            else if ((tbTmpPlayer.m_nStatus & EN_CITY_STATUS__AVOID_WAR)
                || (tbTmpPlayer.m_nStatus & EN_CITY_STATUS__NEW_PROTECTION))
            {
                //替换国王...有peace_time要报错...
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__CANNOT_BE_KING_WHEN_PEACE_TIME;
                return -1;
            }
        }

        // get package
        pstSession->ResetAwsInfo();
        // set expect
        ExpectedItem expect_item;
        ExpectedDesc expect_desc;
        expect_desc.clear();
        expect_item.SetVal(TbPLAYER_FIELD_ALID, TRUE, CCommonBase::GetPlayerAllianceId(udwAllianceId, ucFromPosition)); //加expect防止边界条件
        expect_desc.push_back(expect_item);
        expect_item.SetVal(TbPLAYER_FIELD_ALPOS, TRUE, static_cast<TINT32>(ucFromPosition));
        expect_desc.push_back(expect_item);
        // updt player info
        TbPlayer tbPlayerItem;
        tbPlayerItem.Set_Uid(udwUserId);
        tbPlayerItem.Set_Alid(CCommonBase::GetPlayerAllianceId(udwAllianceId, ucPosition));
        tbPlayerItem.Set_Alpos(ucPosition);
        tbPlayerItem.Set_Alname(pstPlayer->m_sAlname);
        tbPlayerItem.Set_Alname_update_time(CTimeUtils::GetUnixTime());
        CAwsRequest::UpdateItem(pstSession, &tbPlayerItem, expect_desc, RETURN_VALUES_ALL_NEW);

        // send request
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AlliancePosChange: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }

        return 0;
    }

    // 3. 处理响应
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        // parse data
        dwRetCode = CAwsResponse::OnUpdateItemRsp(*pstSession->m_vecAwsRsp[0], &pstSession->m_tbTmpPlayer);

        pstSession->ResetAwsInfo();
        if(dwRetCode > 0 && pstSession->m_tbTmpPlayer.m_nUid > 0)
        {
            TbAl_member tbAlmember;
            // update player and alliance info
            if(ucPosition == EN_ALLIANCE_POS__CHANCELLOR)
            {
                // change player info
                pstPlayer->Set_Alpos(EN_ALLIANCE_POS__MEMBER);
                pstPlayer->Set_Alid(CCommonBase::GetPlayerAllianceId(udwAllianceId, EN_ALLIANCE_POS__MEMBER));
                pstSession->m_stUserInfo.m_ucPlayerFlag = EN_TABLE_UPDT_FLAG__CHANGE;

                // change alliance info
                pstAlliance->Set_Oid(pstSession->m_tbTmpPlayer.m_nUid);
                pstAlliance->Set_Oname(pstSession->m_tbTmpPlayer.m_sUin);
                pstAlliance->Set_Owner_cid(pstSession->m_tbTmpPlayer.m_nCid);
                pstSession->m_stUserInfo.m_ucAllianceFlag = EN_TABLE_UPDT_FLAG__CHANGE;

                tbAlmember.Set_Aid(udwAllianceId);
                tbAlmember.Set_Uid(pstSession->m_stUserInfo.m_tbPlayer.m_nUid);
                tbAlmember.Set_Al_pos(EN_ALLIANCE_POS__MEMBER);
                CAwsRequest::UpdateItem(pstSession, &tbAlmember);

                //change svr_al record
                pstSession->m_stUserInfo.m_tbSvrAl.Set_Owner_uid(pstSession->m_tbTmpPlayer.m_nUid);
                pstSession->m_stUserInfo.m_tbSvrAl.Set_Owner_cid(pstSession->m_tbTmpPlayer.m_nCid);
                pstSession->m_stUserInfo.m_aucSvrAlFlag = EN_TABLE_UPDT_FLAG__CHANGE;

                if(pstAlliance->m_nAid == pstSession->m_tbThrone.m_nAlid)
                {
                    pstSession->m_tbThrone.Set_Owner_id(pstAlliance->m_nOid);
                    pstSession->m_tbThrone.Set_Owner_cid(pstAlliance->m_nOwner_cid);
                    pstSession->m_tbThrone.Set_Occupy_time(CTimeUtils::GetUnixTime());

                    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwWildNum; udwIdx++)
                    {
                        if (pstUser->m_atbWild[udwIdx].m_nType == EN_WILD_TYPE__THRONE_NEW
                            && pstUser->m_atbWild[udwIdx].m_nId == pstSession->m_tbThrone.m_nPos)
                        {
                            pstUser->m_atbWild[udwIdx].Set_Uid(pstAlliance->m_nOid);
                            pstUser->m_atbWild[udwIdx].Set_Uname(pstAlliance->m_sOname);
                            pstUser->m_atbWild[udwIdx].Set_Alname(pstAlliance->m_sName);
                            pstUser->m_atbWild[udwIdx].Set_Al_nick(pstAlliance->m_sAl_nick_name);
                            pstUser->m_atbWild[udwIdx].Set_Al_flag(pstAlliance->m_nAvatar);
                            pstUser->m_atbWild[udwIdx].Set_Name_update_time(CTimeUtils::GetUnixTime());

                            pstUser->m_aucWildFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
                        }
                    }

                    // 召回throne assign
                    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; udwIdx++)
                    {
                        if (pstUser->m_atbMarch[udwIdx].m_nSuid == pstPlayer->m_nUid
                            && pstUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__ASSIGN_THRONE
                            && pstUser->m_atbMarch[udwIdx].m_nStatus != EN_MARCH_STATUS__RETURNING)
                        {
                            if (pstUser->m_atbMarch[udwIdx].m_nStatus == EN_MARCH_STATUS__MARCHING)
                            {
                                CActionBase::ReturnMarchOnFly(&pstUser->m_atbMarch[udwIdx]);
                            }
                            else
                            {
                                CActionBase::ReturnMarch(&pstUser->m_atbMarch[udwIdx]);
                            }
                            pstUser->m_aucMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
                        }
                    }

                    // 转移收税action
                    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; udwIdx++)
                    {
                        if (pstUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__TAX
                            && pstUser->m_atbMarch[udwIdx].m_nSuid == pstUser->m_tbPlayer.m_nUid)
                        {
                            if (pstUser->m_atbMarch[udwIdx].m_jTax_info.isObject())
                            {
                                pstUser->m_atbMarch[udwIdx].m_jTax_info["self_alid"] = pstAlliance->m_nAid;
                            }
                            else
                            {
                                pstUser->m_atbMarch[udwIdx].m_jTax_info = Json::Value(Json::objectValue);
                                pstUser->m_atbMarch[udwIdx].m_jTax_info["self_alid"] = pstAlliance->m_nAid;
                            }
                            pstUser->m_atbMarch[udwIdx].SetFlag(TbMARCH_ACTION_FIELD_TAX_INFO);

                            pstUser->m_atbMarch[udwIdx].Set_Etime(CTimeUtils::GetUnixTime());
                            pstUser->m_aucMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;

                            break;
                        }
                    }

                    CSendMessageBase::SendBroadcast(pstUser, pstAlliance->m_nSid, 0, EN_BROADCAST_CONTENT_ID__BECOME_KING, pstAlliance->m_sOname);
                }
            }

            tbAlmember.Set_Aid(udwAllianceId);
            tbAlmember.Set_Uid(pstSession->m_tbTmpPlayer.m_nUid);
            tbAlmember.Set_Al_pos(ucPosition);
            CAwsRequest::UpdateItem(pstSession, &tbAlmember);

            // send request
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
            bNeedResponse = TRUE;
            dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
            if(dwRetCode < 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AlliancePosChange: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                return -4;
            }
            return 0;
        }
        else
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__AL_POS_NOT_MATCH;
            return -5;
        }
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        //3 发送邮件
        //3-1 降级
        if(ucFromPosition > ucPosition)
        {
            CMsgBase::SendOperateMail(udwUserId, EN_MAIL_ID__BE_DEMOTED, pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE, CCommonFunc::NumToString(ucPosition).c_str(), "", "");
        }
        //3-2 升级
        if(ucFromPosition < ucPosition)
        {
            CMsgBase::SendOperateMail(udwUserId, EN_MAIL_ID__BE_PROMOTED, pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE, CCommonFunc::NumToString(ucPosition).c_str(), "", "");
        }

        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__INIT;
        pstSession->m_udwCommand = EN_CLIENT_REQ_COMMAND__ALLIANCE_MEMBER_GET;
        pstSession->m_bGotoOtherCmd = TRUE;
        strcpy(pstSession->m_stReqParam.m_szKey[9], "1");
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__INIT;
    pstSession->m_udwCommand = EN_CLIENT_REQ_COMMAND__ALLIANCE_MEMBER_GET;
    pstSession->m_bGotoOtherCmd = TRUE;
    strcpy(pstSession->m_stReqParam.m_szKey[9], "1");
    return 0;
}

TINT32 CProcessAlliance::ProcessCmd_AllianceChangePolicy(SSession *pstSession, TBOOL &bNeedResponse)
{
    if(pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos == EN_ALLIANCE_POS__REQUEST)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NOT_IN_ALLIANCE;
        return -1;
    }

    TUINT32 udwPolicy = atoi(pstSession->m_stReqParam.m_szKey[0]);
    if((EN_ALLIANCE_JOIN_NORMAL != udwPolicy && EN_ALLIANCE_JOIN_AUTO != udwPolicy)
    || pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos < EN_ALLIANCE_POS__VICE_CHANCELOR)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AllianceChangePolicy: Invalid udwPolicy[%u] [seq=%u]", udwPolicy, \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -2;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        TbAlliance& tbAlliance = pstSession->m_stUserInfo.m_tbAlliance;
        if(tbAlliance.m_nPolicy / ALLIANCE_POLICY_OFFSET != udwPolicy)
        {
            pstSession->m_stUserInfo.m_ucAllianceFlag = EN_TABLE_UPDT_FLAG__CHANGE;
            tbAlliance.Set_Policy(udwPolicy * ALLIANCE_POLICY_OFFSET);

            //// set next procedure
            //AllianceRank oTmpAllianceRank;
            //oTmpAllianceRank.udwAid = tbAlliance.m_nAid;
            //oTmpAllianceRank.udwPolicy = udwPolicy;
            //CDbRequest::UpdateAlliancePolicy(pstSession, &oTmpAllianceRank);
            //TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_AllianceChangePolicy:try update alliance policy[seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));

            //// send request
            //pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
            //pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__DB;
            //dwRetCode = CBaseProcedure::SendDbRequest(pstSession, EN_SERVICE_TYPE_QUERY_MYSQL_REQ);
            //if(dwRetCode < 0)
            //{
            //    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            //    TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AllianceChangePolicy: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            //    return -3;
            //}
            //return 0;
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
            CMsgBase::ChangeAlPolicy(&tbAlliance);
        }
        else
        {
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
            return 0;
        }
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessAlliance::ProcessCmd_AllianceChangeDesc(SSession *pstSession, TBOOL &bNeedResponse)
{
    if(pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos == EN_ALLIANCE_POS__REQUEST)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NOT_IN_ALLIANCE;
        return -1;
    }

    if(pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos < EN_ALLIANCE_POS__VICE_CHANCELOR)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -2;
    }

    // 0. 请求参数
    TCHAR *pszDesc = pstSession->m_stReqParam.m_szKey[0];

    pstSession->m_stUserInfo.m_tbAlliance.Set_Desc(pszDesc);
//     if(pstSession->m_stUserInfo.m_tbAlliance.m_sDesc.empty())
//     {
//         pstSession->m_stUserInfo.m_tbAlliance.Set_Desc("There is no description now.");
//     }

    pstSession->m_stUserInfo.m_ucAllianceFlag = EN_TABLE_UPDT_FLAG__CHANGE;

    return 0;
}

TINT32 CProcessAlliance::ProcessCmd_AllianceChangeNotic(SSession *pstSession, TBOOL &bNeedResponse)
{
    if(pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos == EN_ALLIANCE_POS__REQUEST)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NOT_IN_ALLIANCE;
        return -1;
    }

    if(pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos < EN_ALLIANCE_POS__VICE_CHANCELOR)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -2;
    }
    // 0. 请求参数
    TCHAR *pszNotice = pstSession->m_stReqParam.m_szKey[0];

    pstSession->m_stUserInfo.m_tbAlliance.Set_Notice(pszNotice);
//     if(pstSession->m_stUserInfo.m_tbAlliance.m_sNotice.empty())
//     {
//         pstSession->m_stUserInfo.m_tbAlliance.Set_Notice("Welcome to join our alliance!");
//     }

    pstSession->m_stUserInfo.m_ucAllianceFlag = EN_TABLE_UPDT_FLAG__CHANGE;

    return 0;
}

TINT32 CProcessAlliance::ProcessCmd_AllianceChangeLanguage(SSession *pstSession, TBOOL &bNeedResponse)
{
    if(pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos == EN_ALLIANCE_POS__REQUEST)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NOT_IN_ALLIANCE;
        return -1;
    }

    // 0. 请求参数
    TUINT32 udwAllianceLanguage = atoi(pstSession->m_stReqParam.m_szKey[0]);

    if(pstSession->m_stUserInfo.m_tbPlayer.Get_Alpos() < EN_ALLIANCE_POS__VICE_CHANCELOR)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__HAVE_NO_PERMISSION;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AllianceChangeLanguage: req param error [seq=%u]", \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -2;
    }

    // 更新mysql
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        ////实际是上update
        //ostringstream oss;
        //oss << "update alliance_recommand_all set al_language=" << udwAllianceLanguage << " where aid=" << pstSession->m_stUserInfo.m_tbAlliance.m_nAid << ";";
        //CDbRequest::Select(pstSession, oss.str(), "alliance_recommand_all", "update");
        //oss.str("");
        //oss << "update alliance_rank set al_language=" << udwAllianceLanguage << " where aid=" << pstSession->m_stUserInfo.m_tbAlliance.m_nAid << ";";
        //CDbRequest::Select(pstSession, oss.str(), "alliance_rank", "update");

        //// send request
        //pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        //pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__DB;
        //dwRetCode = CBaseProcedure::SendDbRequest(pstSession, EN_SERVICE_TYPE_QUERY_MYSQL_REQ);
        //if(dwRetCode < 0)
        //{
        //    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
        //    TSE_LOG_ERROR(pstSession->m_poServLog, ("send alliance rank update failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
        //    return -3;
        //}
        //return 0;

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_stUserInfo.m_tbAlliance.Set_Language(udwAllianceLanguage);
        CMsgBase::ChangeAlLang(&pstSession->m_stUserInfo.m_tbAlliance);
    }

    // 更新DYNAMODB
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        pstSession->m_stUserInfo.m_tbAlliance.Set_Language(udwAllianceLanguage);
        pstSession->m_stUserInfo.m_ucAllianceFlag = EN_TABLE_UPDT_FLAG__CHANGE;

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessAlliance::ProcessCmd_WallInsert(SSession *pstSession, TBOOL &bNeedResponse)
{
    if(EN_COMMAND_STEP__INIT == pstSession->m_udwCommandStep)
    {
        if(pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos == EN_ALLIANCE_POS__REQUEST)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NOT_IN_ALLIANCE;
            return -1;
        }

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
    }

    if(EN_COMMAND_STEP__1 == pstSession->m_udwCommandStep)
    {  
        TINT32 dwRetCode = 0;
        
        // reset req
        pstSession->ResetTranslateReq();
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;

        if(0 != CDocument::GetInstance()->GetSupportLangNum())
        {        
        
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__TRANSLATE;
            // send request
            bNeedResponse = TRUE;
        	Json::FastWriter oJsonWriter;
        	oJsonWriter.omitEndingLineFeed();
        	Json::Value rTranslateJson = Json::Value(Json::objectValue);
            for(TINT32 dwIdx = 0; dwIdx < CDocument::GetInstance()->GetSupportLangNum(); ++dwIdx)
            {
                rTranslateJson.clear();
                TranslateReqInfo *pstTranslateReq = new TranslateReqInfo;
                rTranslateJson["0"]["from"] = "";
                rTranslateJson["0"]["to"] = CDocument::GetInstance()->GetShortLangName(dwIdx);
            	rTranslateJson["0"]["content"] = pstSession->m_stReqParam.m_szKey[0];
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_TranslateTest: [TranslateType=%s] [TranslateOperate=%s] [TranslateReqContent=%s] [seq:%u]", \
                                                        pstTranslateReq->m_strTranslateType.c_str(), \
                                                        pstTranslateReq->m_strTranslateOperate.c_str(), \
                                                        pstTranslateReq->m_strTranslateContent.c_str(), \
                                                        pstSession->m_stUserInfo.m_udwBSeqNo));
                pstTranslateReq->SetVal("mail", "translate", oJsonWriter.write(rTranslateJson));       
                pstSession->m_vecTranslateReq.push_back(pstTranslateReq);
            }
            
            dwRetCode = CBaseProcedure::SendTranslateRequest(pstSession, EN_SERVICE_TYPE_TRANSLATE_REQ);
            if(dwRetCode != 0)
            {        
                bNeedResponse = FALSE;
                pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TranslateTest: send translate req fail [ret=%d] [seq=%u]", \
                                                        dwRetCode, \
                                                        pstSession->m_stUserInfo.m_udwBSeqNo));
            }
            else
            {
                return 0;
            }             
        }
    }

    if (EN_COMMAND_STEP__2 == pstSession->m_udwCommandStep)
    {
        SCityInfo *pstCity = &pstSession->m_stUserInfo.m_stCityInfo;
        if (pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos == EN_ALLIANCE_POS__REQUEST)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NOT_IN_ALLIANCE;
            return -1;
        }

        TBOOL bTranslateFalg = FALSE;
        if ((TUINT32)CDocument::GetInstance()->GetSupportLangNum() == pstSession->m_vecTranslateRsp.size()
            && 0 != pstSession->m_vecTranslateRsp.size())
        {
            bTranslateFalg = TRUE;
        }
        for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecTranslateRsp.size(); ++udwIdx)
        {
            if (0 != pstSession->m_vecTranslateRsp[udwIdx]->m_dwRetCode)
            {
                bTranslateFalg = FALSE;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TranslateTest: parse translate info fail [seq=%u]", \
                    pstSession->m_stUserInfo.m_udwBSeqNo));
            }
            else
            {
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_TranslateTest: [TranslateType=%s] [TranslateOperate=%s] [TranslateContent=%s] [TranslateResult=%s] [seq=%u]", \
                    pstSession->m_vecTranslateRsp[udwIdx]->m_strTranslateType.c_str(), \
                    pstSession->m_vecTranslateRsp[udwIdx]->m_strTranslateOperate.c_str(), \
                    pstSession->m_vecTranslateRsp[udwIdx]->m_strTranslateContent.c_str(), \
                    pstSession->m_vecTranslateRsp[udwIdx]->m_strTranslateResult.c_str(), \
                    pstSession->m_stUserInfo.m_udwBSeqNo));
            }
        }


        TINT32 dwLanguageId = -1;
        string strTranslateContent = "";
        Json::FastWriter oJsonWriter;
        oJsonWriter.omitEndingLineFeed();
        Json::Value rTranslateJson = Json::Value(Json::objectValue);
        if (TRUE == bTranslateFalg)
        {
            for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecTranslateRsp.size(); ++udwIdx)
            {
                Json::Reader jsonReader;
                Json::Value jResultBody;
                if (jsonReader.parse(pstSession->m_vecTranslateRsp[udwIdx]->m_strTranslateResult.c_str(), jResultBody))
                {
                    if (0 == udwIdx)
                    {
                        dwLanguageId = atoi(CDocument::GetInstance()->GetLanguageId(jResultBody["0"]["from"].asString().c_str(), pstSession->m_stReqParam.m_udwLang).c_str());
                    }
                    Json::Value::Members vecMembers = jResultBody.getMemberNames();
                    for (TUINT32 udwIdy = 0; udwIdy < vecMembers.size(); ++udwIdy)
                    {
                        rTranslateJson[CDocument::GetInstance()->GetLangId(jResultBody[vecMembers[0]]["to"].asString()).c_str()][vecMembers[udwIdy]] = jResultBody[vecMembers[udwIdy]]["content"];
                    }
                }
            }
            strTranslateContent = oJsonWriter.write(rTranslateJson);
        }

        TINT32 dwIndex = pstSession->m_stUserInfo.m_udwWallNum;
        if (pstSession->m_stUserInfo.m_udwWallNum == MAX_WALL_MSG_NUM)
        {
            dwIndex = pstSession->m_stUserInfo.m_udwWallNum - 1;
        }

        TbAl_wall *ptbWall = &pstSession->m_stUserInfo.m_atbWall[dwIndex];
        ptbWall->Reset();

        TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
        TINT64 ddwWallId = udwCurTime;
        ddwWallId = (ddwWallId << 32) + pstSession->m_stUserInfo.m_tbLogin.m_nUid;

        ptbWall->Set_Alid(pstSession->m_stUserInfo.m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET);
        ptbWall->Set_Wall_id(ddwWallId);
        ptbWall->Set_Time(udwCurTime);
        ptbWall->Set_Alpos(pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos);
        ptbWall->Set_Uid(pstSession->m_stUserInfo.m_tbPlayer.m_nUid);
        ptbWall->Set_Uin(pstSession->m_stUserInfo.m_tbPlayer.m_sUin);
        ptbWall->Set_Content(pstSession->m_stReqParam.m_szKey[0]);
        ptbWall->Set_Avatar(pstSession->m_stUserInfo.m_tbPlayer.m_nAvatar);
        ptbWall->Set_Raw_lang(dwLanguageId);
        ptbWall->Set_Translate_content(strTranslateContent);

        pstSession->m_stUserInfo.m_aucWallFlag[dwIndex] = EN_TABLE_UPDT_FLAG__NEW;
        if (pstSession->m_stUserInfo.m_udwWallNum < MAX_WALL_MSG_NUM)
        {
            pstSession->m_stUserInfo.m_udwWallNum++;
        }

        TbUser_stat& tbUserStat = pstSession->m_stUserInfo.m_tbUserStat;
        tbUserStat.Set_Wall_get_t(udwCurTime + 1);

        //task count
        CQuestLogic::SetTaskCurValue(&pstSession->m_stUserInfo, pstCity, EN_TASK_TYPE_ALLIANCE_COMMENT);

        // next procedure
        pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_WALL;

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    return 0;
}

TINT32 CProcessAlliance::ProcessCmd_WallDelete(SSession *pstSession, TBOOL &bNeedResponse)
{
    if(pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos == EN_ALLIANCE_POS__REQUEST)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NOT_IN_ALLIANCE;
        return -1;
    }

    TINT64 ddwId = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 10);

    for(TUINT32 idx = 0; idx < pstSession->m_stUserInfo.m_udwWallNum; idx++)
    {
        if(pstSession->m_stUserInfo.m_atbWall[idx].m_nWall_id == ddwId)
        {
            pstSession->m_stUserInfo.m_aucWallFlag[idx] = EN_TABLE_UPDT_FLAG__DEL;
        }
    }

    TbUser_stat& tbUserStat = pstSession->m_stUserInfo.m_tbUserStat;
    tbUserStat.Set_Wall_get_t(CTimeUtils::GetUnixTime());

    pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_WALL;
    return 0;
}

TINT32 CProcessAlliance::ProcessCmd_AlAssistSend(SSession *pstSession, TBOOL &bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    // 0. 请求参数
    TUINT8 ucHelpType = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TCHAR *pszResource = pstSession->m_stReqParam.m_szKey[1];
    TUINT32 udwCityPos = strtoul(pstSession->m_stReqParam.m_szKey[2], NULL, 10);
    TCHAR *pszDesc = pstSession->m_stReqParam.m_szKey[3];
    TINT64 ddwReqTime = CTimeUtils::GetUnixTime();
    TUINT32 udwParamNum = EN_RESOURCE_TYPE__END;

    // check param
    if(pstUser->m_tbPlayer.m_nAlpos == EN_ALLIANCE_POS__REQUEST)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NOT_IN_ALLIANCE;
        return -1;
    }
    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwAlAssistAllNum; ++udwIdx)
    {
        if(pstUser->m_atbAlAssistAll[udwIdx].m_nUid == pstUser->m_tbLogin.m_nUid
        && pstUser->m_atbAlAssistAll[udwIdx].m_nType == ucHelpType
        && pstUser->m_atbAlAssistAll[udwIdx].m_nTime + CCommonBase::GetGameBasicVal(EN_GAME_BASIC_ASSIST_INTERVAL) > ddwReqTime)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__HAS_POST_ASSIST;
            return -2;
        }
    }

    TbAl_assist tbAssist;
    tbAssist.Reset();
    tbAssist.Set_Sid(pstSession->m_stReqParam.m_udwSvrId);
    tbAssist.Set_Id((ddwReqTime << 32) + pstUser->m_tbPlayer.m_nUid);
    tbAssist.Set_Type(ucHelpType);
    tbAssist.Set_Uid(pstUser->m_tbPlayer.m_nUid);
    tbAssist.Set_Uname(pstUser->m_tbPlayer.m_sUin);
    tbAssist.Set_Cid(udwCityPos);
    tbAssist.Set_Aid(pstUser->m_tbAlliance.m_nAid);
    tbAssist.Set_Time(ddwReqTime);
    tbAssist.Set_Desc(pszDesc);
    switch(ucHelpType)
    {
    case EN_ASSIST_TYPE__RES_REQ:
    case EN_ASSIST_TYPE__RES_SHARE:
        CCommonFunc::GetArrayFromString(pszResource, ':', &tbAssist.m_bParam[0].m_addwNum[0], udwParamNum);
        break;
    }
    tbAssist.SetFlag(TbAL_ASSIST_FIELD_PARAM);
    tbAssist.m_bProgress.Reset();
    tbAssist.SetFlag(TbAL_ASSIST_FIELD_PROGRESS);

    pstUser->m_atbAlAssistAll[pstUser->m_udwAlAssistAllNum] = tbAssist;
    pstUser->m_aucAlAssistAllFlag[pstUser->m_udwAlAssistAllNum] = EN_TABLE_UPDT_FLAG__NEW;
    pstUser->m_udwAlAssistAllNum++;

    // next procedure
    pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_ASSIST_LIST;
    return 0;
}

TINT32 CProcessAlliance::ProcessCmd_AlAssistGet(SSession *pstSession, TBOOL &bNeedResponse)
{
    if(pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos == EN_ALLIANCE_POS__REQUEST)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NOT_IN_ALLIANCE;
        return -1;
    }

    TUINT32 udwTimeEnd = CTimeUtils::GetUnixTime() - CCommonBase::GetGameBasicVal(EN_GAME_BASIC_ASSIST_INTERVAL);
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbUser_stat *ptbUserStat = &pstUser->m_tbUserStat;

    // set stat
    ptbUserStat->Set_As_get_t(CTimeUtils::GetUnixTime());

    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwAlAssistAllNum; ++udwIdx)
    {
        if(pstUser->m_atbAlAssistAll[udwIdx].m_nTime < udwTimeEnd)
        {
            pstUser->m_aucAlAssistAllFlag[udwIdx] = EN_TABLE_UPDT_FLAG__DEL;
        }
    }

    // next procedure
    pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_ASSIST_LIST;
    return 0;
}

TINT32 CProcessAlliance::ProcessCmd_AlAssistDel(SSession *pstSession, TBOOL &bNeedResponse)
{
    if(pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos == EN_ALLIANCE_POS__REQUEST)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NOT_IN_ALLIANCE;
        return -1;
    }

    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    // 0. 请求参数
    TINT64 ddwDelId = strtoull(pstSession->m_stReqParam.m_szKey[0], NULL, 10);

    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwAlAssistAllNum; ++udwIdx)
    {
        if(pstUser->m_atbAlAssistAll[udwIdx].m_nId == ddwDelId)
        {
            pstUser->m_aucAlAssistAllFlag[udwIdx] = EN_TABLE_UPDT_FLAG__DEL;
            break;
        }
    }

    // next procedure
    pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_ASSIST_LIST;
    return 0;
}

TINT32 CProcessAlliance::ProcessCmd_AlTaskHelpGet(SSession *pstSession, TBOOL &bNeedResponse)
{
    if(pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos == EN_ALLIANCE_POS__REQUEST)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NOT_IN_ALLIANCE;
        return -1;
    }

    return 0;
}

TINT32 CProcessAlliance::ProcessCmd_AlTaskHelpSpeedUp(SSession *pstSession, TBOOL &bNeedResponse)
{
    if(pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos == EN_ALLIANCE_POS__REQUEST)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NOT_IN_ALLIANCE;
        return -1;
    }

    SCityInfo *pstCity = &pstSession->m_stUserInfo.m_stCityInfo;

    SReqParam& stReqParam = pstSession->m_stReqParam;
    SUserInfo& stUserInfo = pstSession->m_stUserInfo;
    TbPlayer& tbPlayer = pstSession->m_stUserInfo.m_tbPlayer;

    TINT64 ddwTaskId = strtoull(stReqParam.m_szKey[0], NULL, 10);

    TUINT32 udwAlHelpListIdx = ddwTaskId % MAX_AL_HELP_LIST_NUM;
    TbAl_help& tbAl_help = pstSession->m_stUserInfo.m_atbAl_help[udwAlHelpListIdx];
    SAlHelpList& stAlHelpList = tbAl_help.m_bList[0];
    TbAlliance& tbAlliance = pstSession->m_stUserInfo.m_tbAlliance;

    if(CActionBase::IsPlayerOwnedAction(tbPlayer.m_nUid, ddwTaskId)) //自己的action
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__TARGET_NOT_EXIST;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AlTaskHelpSpeedUp: can't help youself [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }

    TbAlliance_action* ptbAlAction = NULL;
    for(TUINT32 udwIdx = 0; udwIdx < stUserInfo.m_udwAlCanHelpActionNum; ++udwIdx)
    {
        if(stUserInfo.m_patbAlCanHelpAction[udwIdx]->m_nId == ddwTaskId)
        {
            ptbAlAction = stUserInfo.m_patbAlCanHelpAction[udwIdx];
            break;
        }
    }

    stUserInfo.m_tbDataOutput.m_jData["svr_al_action_list"] = 0;
    stUserInfo.m_tbDataOutput.SetFlag(TbDATA_OUTPUT_FIELD_DATA);

    if(!ptbAlAction)
    {
        //llt modify 只打log 不返回错误
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessCmd__AlTaskHelpSpeedUp: task[%llu] not exist [seq=%u]", ddwTaskId, pstSession->m_stUserInfo.m_udwBSeqNo));
        return 0;
    }

    TbAlliance_action tbTmpAlAction = *ptbAlAction;

    ptbAlAction->Set_Helped_num(1, UPDATE_ACTION_TYPE_ADD);
    TINT32 dwHelpTime = CToolBase::Get_AlHelpTime(ptbAlAction->m_nCtime, ptbAlAction->m_nCan_help_num);
    ptbAlAction->Set_Etime(dwHelpTime, UPDATE_ACTION_TYPE_ADD);
    TINT32 dwAcitonIdx = CActionBase::GetAlActionIndex(stUserInfo.m_atbSelfAlAction, stUserInfo.m_udwSelfAlActionNum, ptbAlAction->m_nId);
    if(dwAcitonIdx >= 0)
    {
        stUserInfo.m_aucSelfAlActionFlag[dwAcitonIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
    }

    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    //添加帮助记录
    SAlHelpNode* pstAlHelpNode = stAlHelpList.AddNode();
    pstAlHelpNode->uddwTaskId = ddwTaskId;
    pstAlHelpNode->ddwEndTime = ptbAlAction->m_nEtime;
    tbAl_help.SetFlag(TbAL_HELP_FIELD_LIST);
    pstSession->m_stUserInfo.m_aucAlHelpFlag[udwAlHelpListIdx] = EN_TABLE_UPDT_FLAG__CHANGE;

    //增加忠诚度和联盟基金
    if(tbPlayer.m_nLoy_time / AL_FUND_GET_INTERVAL != udwCurTime / AL_FUND_GET_INTERVAL)
    {
        tbPlayer.Set_Loy_itv(0);
    }
    TUINT32 udwFundAdd = AL_FUND_ADD_PER_TIME;

    TINT64 ddwMaxAlFundGetPerInterval = CGameInfo::GetInstance()->m_oJsonRoot["game_basic"][3U].asInt64();
    if(tbPlayer.m_nLoy_itv + udwFundAdd > ddwMaxAlFundGetPerInterval)
    {
        udwFundAdd = ddwMaxAlFundGetPerInterval - tbPlayer.m_nLoy_itv > 0 ? ddwMaxAlFundGetPerInterval - tbPlayer.m_nLoy_itv : 0;
    }
    pstSession->m_udwLoytalAdd = udwFundAdd;
    tbPlayer.Set_Loy_itv(tbPlayer.m_nLoy_itv + udwFundAdd);
    tbPlayer.Set_Loy_cur(tbPlayer.m_nLoy_cur + udwFundAdd);
    tbPlayer.Set_Loy_all(tbPlayer.m_nLoy_all + udwFundAdd);
    tbPlayer.Set_Loy_time(udwCurTime);
    tbPlayer.Set_Send_al_help_num(tbPlayer.m_nSend_al_help_num +1);
    tbAlliance.Set_Fund(tbAlliance.m_nFund + udwFundAdd);

    //task count
    CQuestLogic::SetTaskCurValue(&pstSession->m_stUserInfo,pstCity, EN_TASK_TYPE_ALLIANCE_CONSTRUCT);

    // 发送help tips
    if(strcmp(pstSession->m_stReqParam.m_szVs, "1.0") == 0)
    {
        if(ptbAlAction->m_nMclass == EN_ACTION_MAIN_CLASS__BUILDING)
        {
            if(ptbAlAction->m_nSclass == EN_ACTION_SEC_CLASS__RESEARCH_UPGRADE)
            {
                //tips
                CSendMessageBase::AddTips(&stUserInfo, EN_TIPS_TYPE__AL_HELP, ptbAlAction->m_nSuid, TRUE,
                    EN_HELP_TYPE__RESEARCH, ptbAlAction->m_bParam[0].m_stBuilding.m_ddwType, ptbAlAction->m_bParam[0].m_stBuilding.m_ddwTargetLevel,
                    tbPlayer.m_sUin.c_str(), CCommonFunc::NumToString(dwHelpTime * -1).c_str());

                TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd__AlTaskHelpSpeedUp: SendHelpTips[%llu] [seq=%u]", ddwTaskId, pstSession->m_stUserInfo.m_udwBSeqNo));
            }
            else if(ptbAlAction->m_nSclass == EN_ACTION_SEC_CLASS__BUILDING_UPGRADE)
            {
                //tips
                CSendMessageBase::AddTips(&stUserInfo, EN_TIPS_TYPE__AL_HELP, ptbAlAction->m_nSuid, TRUE,
                    EN_HELP_TYPE__BUILDING, ptbAlAction->m_bParam[0].m_stBuilding.m_ddwType, ptbAlAction->m_bParam[0].m_stBuilding.m_ddwTargetLevel,
                    tbPlayer.m_sUin.c_str(), CCommonFunc::NumToString(dwHelpTime * -1).c_str());

                TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd__AlTaskHelpSpeedUp: SendHelpTips[%llu] [seq=%u]", ddwTaskId, pstSession->m_stUserInfo.m_udwBSeqNo));
            }
            else if(ptbAlAction->m_nSclass == EN_ACTION_SEC_CLASS__REMOVE_OBSTACLE || ptbAlAction->m_nSclass == EN_ACTION_SEC_CLASS__BUILDING_REMOVE)
            {
                //tips
                CSendMessageBase::AddTips(&stUserInfo, EN_TIPS_TYPE__AL_HELP, ptbAlAction->m_nSuid, TRUE,
                    EN_HELP_TYPE__REMOVE_OBSTCLE, ptbAlAction->m_bParam[0].m_stBuilding.m_ddwType, ptbAlAction->m_bParam[0].m_stBuilding.m_ddwTargetLevel,
                    tbPlayer.m_sUin.c_str(), CCommonFunc::NumToString(dwHelpTime * -1).c_str());

                TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd__AlTaskHelpSpeedUp: SendHelpTips[%llu] [seq=%u]", ddwTaskId, pstSession->m_stUserInfo.m_udwBSeqNo));
            }
        }
        else if(ptbAlAction->m_nMclass == EN_ACTION_MAIN_CLASS__EQUIP && ptbAlAction->m_nSclass == EN_ACTION_SEC_CLASS__EQUIP_UPGRADE)
        {
            //tips
            CSendMessageBase::AddTips(&stUserInfo, EN_TIPS_TYPE__AL_HELP, ptbAlAction->m_nSuid, TRUE,
                EN_HELP_TYPE__SMELT, ptbAlAction->m_bParam[0].m_stEquip.m_ddwEType, 0, tbPlayer.m_sUin.c_str(), CCommonFunc::NumToString(dwHelpTime * -1).c_str());
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd__AlTaskHelpSpeedUp: SendHelpTips[%llu] [seq=%u]", ddwTaskId, pstSession->m_stUserInfo.m_udwBSeqNo));
        }
        else if (ptbAlAction->m_nMclass == EN_ACTION_MAIN_CLASS__TRAIN_NEW)
        {
            TINT32 dwType = -1;
            if (ptbAlAction->m_nSclass == EN_ACTION_SEC_CLASS__TROOP)
            {
                dwType = EN_HELP_TYPE__TROOP_TRAIN;
            }
            else if (ptbAlAction->m_nSclass == EN_ACTION_SEC_CLASS__FORT)
            {
                dwType = EN_HELP_TYPE__FORT_TRAIN;
            }
            else if (ptbAlAction->m_nSclass == EN_ACTION_SEC_CLASS__HOS_TREAT)
            {
                dwType = EN_HELP_TYPE__TROOP_HEAL;
            }
            else if (ptbAlAction->m_nSclass == EN_ACTION_SEC_CLASS__FORT_REPAIR)
            {
                dwType = EN_HELP_TYPE__FORT_REPAIR;
            }
            if (dwType != -1)
            {
                //tips
                CSendMessageBase::AddTips(&stUserInfo, EN_TIPS_TYPE__AL_HELP, ptbAlAction->m_nSuid, TRUE,
                    dwType, ptbAlAction->m_bParam[0].m_stTrain.m_ddwType, ptbAlAction->m_bParam[0].m_stTrain.m_ddwNum, 
                    tbPlayer.m_sUin.c_str(), CCommonFunc::NumToString(dwHelpTime * -1).c_str());
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd__AlTaskHelpSpeedUp: SendHelpTips[%lu] [seq=%u]", ddwTaskId, pstSession->m_stUserInfo.m_udwBSeqNo));
            }
        }
        else if (ptbAlAction->m_nMclass == EN_ACTION_MAIN_CLASS__DRAGON)
        {
            if (ptbAlAction->m_nSclass == EN_ACTION_SEC_CLASS__UNLOCK_DRAGON)
            {
                //tips
                CSendMessageBase::AddTips(&stUserInfo, EN_TIPS_TYPE__AL_HELP, ptbAlAction->m_nSuid, TRUE,
                    EN_HELP_TYPE__UNLOCK_DRAGON, 0, 0,
                    tbPlayer.m_sUin.c_str(), CCommonFunc::NumToString(dwHelpTime * -1).c_str());

                TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd__AlTaskHelpSpeedUp: SendHelpTips[%llu] [seq=%u]", ddwTaskId, pstSession->m_stUserInfo.m_udwBSeqNo));
            }
        }
    }
    else
    {
        //wave@push_data
        CPushDataProcess::SendPushDataRequest_AlTaskHelpSpeedUp(pstSession, &tbTmpAlAction, 1, dwHelpTime);
    }

    return 0;
}

TINT32 CProcessAlliance::ProcessCmd_AlTaskHelpSpeedUpAll(SSession *pstSession, TBOOL &bNeedResponse)
{
    if(pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos == EN_ALLIANCE_POS__REQUEST)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NOT_IN_ALLIANCE;
        return -1;
    }

    SUserInfo& stUserInfo = pstSession->m_stUserInfo;
    TbPlayer& tbPlayer = pstSession->m_stUserInfo.m_tbPlayer;
    TbAlliance& tbAlliance = pstSession->m_stUserInfo.m_tbAlliance;
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    SCityInfo *pstCity = &stUserInfo.m_stCityInfo;
    TUINT32 udwHelpedNum = 0;

    stUserInfo.m_tbDataOutput.m_jData["svr_al_action_list"] = 0;
    stUserInfo.m_tbDataOutput.SetFlag(TbDATA_OUTPUT_FIELD_DATA);

    //wave@20160712:get push data
    LTasksGroup stTasksGroup;
    TINT32 dwTasksNum = CPushDataProcess::GetPushTasks_AlTaskHelpSpeedUpAll(pstSession, stTasksGroup);
    if(dwTasksNum == 0)
    {
        return 0;
    }
    //-----------------------------

    TbAlliance_action *ptbAlAction = NULL;
    for(TUINT32 udwIdx = 0; udwIdx < stUserInfo.m_udwAlCanHelpActionNum; ++udwIdx)
    {
        ptbAlAction = stUserInfo.m_patbAlCanHelpAction[udwIdx];
        if(CActionBase::IsPlayerOwnedAction(tbPlayer.m_nUid, ptbAlAction->m_nId)) //自己的任务跳过
        {
            continue;
        }
        ptbAlAction->Set_Helped_num(1, UPDATE_ACTION_TYPE_ADD);
        TINT32 dwHelpTime = CToolBase::Get_AlHelpTime(ptbAlAction->m_nCtime, ptbAlAction->m_nCan_help_num);
        ptbAlAction->Set_Etime(dwHelpTime, UPDATE_ACTION_TYPE_ADD);
        TINT32 dwAcitonIdx = CActionBase::GetAlActionIndex(stUserInfo.m_atbSelfAlAction, stUserInfo.m_udwSelfAlActionNum, ptbAlAction->m_nId);
        if(dwAcitonIdx >= 0)
        {
            stUserInfo.m_aucSelfAlActionFlag[dwAcitonIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
        }
        udwHelpedNum++;

        TUINT32 udwAlHelpListIdx = ptbAlAction->m_nId % MAX_AL_HELP_LIST_NUM;
        TbAl_help& tbAl_help = stUserInfo.m_atbAl_help[udwAlHelpListIdx];
        SAlHelpList& stAlHelpList = tbAl_help.m_bList[0];
        //添加帮助记录
        SAlHelpNode* pstAlHelpNode = stAlHelpList.AddNode();
        pstAlHelpNode->uddwTaskId = ptbAlAction->m_nId;
        pstAlHelpNode->ddwEndTime = ptbAlAction->m_nEtime;
        tbAl_help.SetFlag(TbAL_HELP_FIELD_LIST);
        pstSession->m_stUserInfo.m_aucAlHelpFlag[udwAlHelpListIdx] = EN_TABLE_UPDT_FLAG__CHANGE;

        // 发送help tips
        if(strcmp(pstSession->m_stReqParam.m_szVs, "1.0") == 0)
        {
            if(ptbAlAction->m_nMclass == EN_ACTION_MAIN_CLASS__BUILDING)
            {
                if(ptbAlAction->m_nSclass == EN_ACTION_SEC_CLASS__RESEARCH_UPGRADE)
                {
                    //tips
                    CSendMessageBase::AddTips(&stUserInfo, EN_TIPS_TYPE__AL_HELP, ptbAlAction->m_nSuid, TRUE,
                        EN_HELP_TYPE__RESEARCH, ptbAlAction->m_bParam[0].m_stBuilding.m_ddwType, ptbAlAction->m_bParam[0].m_stBuilding.m_ddwTargetLevel,
                        tbPlayer.m_sUin.c_str(), CCommonFunc::NumToString(dwHelpTime * -1).c_str());

                    TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd__AlTaskHelpSpeedUp: SendHelpTips[%ld] [seq=%u]", ptbAlAction->m_nId, pstSession->m_stUserInfo.m_udwBSeqNo));
                }
                else if(ptbAlAction->m_nSclass == EN_ACTION_SEC_CLASS__BUILDING_UPGRADE)
                {
                    //tips
                    CSendMessageBase::AddTips(&stUserInfo, EN_TIPS_TYPE__AL_HELP, ptbAlAction->m_nSuid, TRUE,
                        EN_HELP_TYPE__BUILDING, ptbAlAction->m_bParam[0].m_stBuilding.m_ddwType, ptbAlAction->m_bParam[0].m_stBuilding.m_ddwTargetLevel,
                        tbPlayer.m_sUin.c_str(), CCommonFunc::NumToString(dwHelpTime * -1).c_str());

                    TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd__AlTaskHelpSpeedUp: SendHelpTips[%ld] [seq=%u]", ptbAlAction->m_nId, pstSession->m_stUserInfo.m_udwBSeqNo));
                }

                else if(ptbAlAction->m_nSclass == EN_ACTION_SEC_CLASS__REMOVE_OBSTACLE || ptbAlAction->m_nSclass == EN_ACTION_SEC_CLASS__BUILDING_REMOVE)
                {
                    //tips
                    CSendMessageBase::AddTips(&stUserInfo, EN_TIPS_TYPE__AL_HELP, ptbAlAction->m_nSuid, TRUE,
                        EN_HELP_TYPE__REMOVE_OBSTCLE, ptbAlAction->m_bParam[0].m_stBuilding.m_ddwType, ptbAlAction->m_bParam[0].m_stBuilding.m_ddwTargetLevel,
                        tbPlayer.m_sUin.c_str(), CCommonFunc::NumToString(dwHelpTime * -1).c_str());

                    TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd__AlTaskHelpSpeedUp: SendHelpTips[%ld] [seq=%u]", ptbAlAction->m_nId, pstSession->m_stUserInfo.m_udwBSeqNo));
                }
            }
            else if(ptbAlAction->m_nMclass == EN_ACTION_MAIN_CLASS__EQUIP && ptbAlAction->m_nSclass == EN_ACTION_SEC_CLASS__EQUIP_UPGRADE)
            {
                //tips
                CSendMessageBase::AddTips(&stUserInfo, EN_TIPS_TYPE__AL_HELP, ptbAlAction->m_nSuid, TRUE,
                    EN_HELP_TYPE__SMELT, ptbAlAction->m_bParam[0].m_stEquip.m_ddwEType, 0, 
                    tbPlayer.m_sUin.c_str(), CCommonFunc::NumToString(dwHelpTime * -1).c_str());
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd__AlTaskHelpSpeedUp: SendHelpTips[%ld] [seq=%u]", ptbAlAction->m_nId, pstSession->m_stUserInfo.m_udwBSeqNo));
            }
            else if (ptbAlAction->m_nMclass == EN_ACTION_MAIN_CLASS__TRAIN_NEW)
            {
                TINT32 dwType = -1;
                if (ptbAlAction->m_nSclass == EN_ACTION_SEC_CLASS__TROOP)
                {
                    dwType = EN_HELP_TYPE__TROOP_TRAIN;
                }
                else if (ptbAlAction->m_nSclass == EN_ACTION_SEC_CLASS__FORT)
                {
                    dwType = EN_HELP_TYPE__FORT_TRAIN;
                }
                else if (ptbAlAction->m_nSclass == EN_ACTION_SEC_CLASS__HOS_TREAT)
                {
                    dwType = EN_HELP_TYPE__TROOP_HEAL;
                }
                else if (ptbAlAction->m_nSclass == EN_ACTION_SEC_CLASS__FORT_REPAIR)
                {
                    dwType = EN_HELP_TYPE__FORT_REPAIR;
                }
                if (dwType != -1)
                {
                    //tips
                    CSendMessageBase::AddTips(&stUserInfo, EN_TIPS_TYPE__AL_HELP, ptbAlAction->m_nSuid, TRUE,
                        dwType, ptbAlAction->m_bParam[0].m_stTrain.m_ddwType, ptbAlAction->m_bParam[0].m_stTrain.m_ddwNum, 
                        tbPlayer.m_sUin.c_str(), CCommonFunc::NumToString(dwHelpTime * -1).c_str());
                    TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd__AlTaskHelpSpeedUp: SendHelpTips[%lu] [seq=%u]", 
                        ptbAlAction->m_nId, pstSession->m_stUserInfo.m_udwBSeqNo));
                }
            }
        }
    }

    if(udwHelpedNum == 0)
    {
        return 0;
    }

    //增加忠诚度和联盟基金
    if(tbPlayer.m_nLoy_time / AL_FUND_GET_INTERVAL != udwCurTime / AL_FUND_GET_INTERVAL)
    {
        tbPlayer.Set_Loy_itv(0);
    }
    TUINT32 udwFundAdd = AL_FUND_ADD_PER_TIME * udwHelpedNum;

    TINT64 ddwMaxAlFundGetPerInterval = CGameInfo::GetInstance()->m_oJsonRoot["game_basic"][3U].asInt64();
    if(tbPlayer.m_nLoy_itv + udwFundAdd > ddwMaxAlFundGetPerInterval)
    {
        udwFundAdd = ddwMaxAlFundGetPerInterval - tbPlayer.m_nLoy_itv > 0 ? ddwMaxAlFundGetPerInterval - tbPlayer.m_nLoy_itv : 0;
    }
    pstSession->m_udwLoytalAdd = udwFundAdd;
    tbPlayer.Set_Loy_itv(tbPlayer.m_nLoy_itv + udwFundAdd);
    tbPlayer.Set_Loy_cur(tbPlayer.m_nLoy_cur + udwFundAdd);
    tbPlayer.Set_Loy_all(tbPlayer.m_nLoy_all + udwFundAdd);
    tbPlayer.Set_Loy_time(udwCurTime);
    tbAlliance.Set_Fund(tbAlliance.m_nFund + udwFundAdd);

    tbPlayer.Set_Send_al_help_num(tbPlayer.m_nSend_al_help_num + udwHelpedNum);
    //task count
    CQuestLogic::SetTaskCurValue(&pstSession->m_stUserInfo,pstCity, EN_TASK_TYPE_ALLIANCE_CONSTRUCT);


    if(strcmp(pstSession->m_stReqParam.m_szVs, "1.0") != 0)
    {
        //wave@20160712:push_data
        pstSession->m_poLongConn->SendData(&stTasksGroup);
    }

    return 0;
}

TINT32 CProcessAlliance::ProcessCmd_AlGiftOpen(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT64 ddwGiftId = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 10);

    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbAl_gift_reward *ptbAlGiftReward = NULL;
    TbAl_gift* ptbAlGift = NULL;
    SCityInfo* pstCity = &pstUser->m_stCityInfo;
    if(pstUser->m_tbPlayer.m_nAlid == 0 || pstUser->m_tbPlayer.m_nAlpos == EN_ALLIANCE_POS__REQUEST)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NOT_IN_ALLIANCE;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AlGiftOpen: not in alliance[seq=%u]",
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }

    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwAlGiftRewardNum; udwIdx++)  //取回GiftReward
    {
        if (pstUser->m_atbAlGiftReward[udwIdx].m_nGid == ddwGiftId)
        {
            ptbAlGiftReward = &pstUser->m_atbAlGiftReward[udwIdx];
            break;
        }
    }

    if (NULL == ptbAlGiftReward)
    {
        ptbAlGiftReward = &pstUser->m_atbAlGiftReward[pstUser->m_udwAlGiftRewardNum];
        pstUser->m_udwAlGiftRewardNum++;
        ptbAlGiftReward->Set_Uid(pstUser->m_tbPlayer.m_nUid);
        ptbAlGiftReward->Set_Gid(ddwGiftId);
        ptbAlGiftReward->Set_Status(EN_AL_GIFT_STATUS_NORMAL);
    }
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        TINT32 dwStatus = EN_AL_GIFT_STATUS_CLEARED;
        dwStatus = ptbAlGiftReward->m_nStatus;
        if (EN_AL_GIFT_STATUS_NORMAL != dwStatus)  //状态错误
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AlGiftOpen: gift status error. [id=%ld][status=%d][seq=%u]",
                ddwGiftId, dwStatus, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }
        else
        {
            for (TINT32 dwIdx = 0; dwIdx < pstSession->m_stUserInfo.m_stAlGifts.m_dwGiftNum; ++dwIdx)
            {
                if (pstSession->m_stUserInfo.m_stAlGifts[dwIdx].m_nId == ddwGiftId)
                {
                    ptbAlGift = &pstSession->m_stUserInfo.m_stAlGifts[dwIdx];
                    break;
                }
            }
            if (NULL == ptbAlGift)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AlGiftOpen: no such gift[id=%ld] [seq=%u]",
                    ddwGiftId, pstSession->m_stUserInfo.m_udwBSeqNo));
                return -3;
            }
            else if (ptbAlGift->m_nCtime + AL_IAP_GIFT_EXPIRE_TIME < static_cast<TINT64>(CTimeUtils::GetCurTimeUs()))
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AlGiftOpen: gift could not open[id=%ld][endtime=%ld] [seq=%u]",
                    ddwGiftId, (ptbAlGift->m_nCtime + AL_IAP_GIFT_EXPIRE_TIME), pstSession->m_stUserInfo.m_udwBSeqNo));
                return -4;
            }
            else
            {
                //开始定位reward
                TINT32 dwPackId = ptbAlGift->m_nPack_id;  //拿到pack id
                pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
                pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__DATA_CENTER;
                pstSession->ResetDataCenterReq();

                DataCenterReqInfo* pstReq = new DataCenterReqInfo;
                pstReq->m_udwType = EN_REFRESH_DATA_TYPE__AL_GIFT_NEW;
                Json::Value rDataReqJson = Json::Value(Json::objectValue);

                rDataReqJson["sid"] = pstUser->m_tbLogin.m_nSid;
                rDataReqJson["uid"] = pstUser->m_tbPlayer.m_nUid;
                rDataReqJson["aid"] = pstUser->m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET;
                rDataReqJson["gift_lv"] = CCommonBase::GetAlGiftLevel(&pstUser->m_tbAlliance);
                rDataReqJson["castle_lv"] = CCityBase::GetBuildingLevelByFuncType(&pstUser->m_stCityInfo, EN_BUILDING_TYPE__CASTLE);
                rDataReqJson["request"] = Json::Value(Json::objectValue);
                rDataReqJson["request"]["num"] = 1;
                rDataReqJson["request"]["flag"] = 0;
                rDataReqJson["request"]["list"] = Json::Value(Json::arrayValue);
                Json::Value jsonReward = Json::Value(Json::objectValue);
                jsonReward["pack_id"] = dwPackId;
                jsonReward["request_type"] = EN_REFRESH_DATA_REQ_TYPE__ALLIANCE_GIFT_ALL;
                rDataReqJson["request"]["list"][0] = jsonReward;

                Json::FastWriter rEventWriter;
                pstReq->m_sReqContent = rEventWriter.write(rDataReqJson);
                pstSession->m_vecDataCenterReq.push_back(pstReq);

                TSE_LOG_DEBUG(pstSession->m_poServLog, ("AlGiftOpen: data center req: [type=%u] [uid=%ld][seq=%u] [json=%s]",
                                                        pstReq->m_udwType, pstUser->m_tbPlayer.m_nUid, pstSession->m_udwSeqNo, pstReq->m_sReqContent.c_str()));
                bNeedResponse = TRUE;
                TINT32 dwRetCode = CBaseProcedure::SendDataCenterRequest(pstSession, EN_SERVICE_TYPE_DATA_CENTER_REQ);
                if (dwRetCode == 0)
                {   
                    return 0;
                }
                else
                {
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_DATA_CENTER_REQ_ERR;
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AlGiftOpen: send request to data center failed. [id=%ld][packid=%d][ret=%d] [seq=%u]",
                        ddwGiftId, dwPackId, dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
                    return -5;
                }
            }
        }
    }
    
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        TINT32 dwCurAlGiftIdx = 0;
        for (TINT32 dwIdx = 0; dwIdx < pstSession->m_stUserInfo.m_stAlGifts.m_dwGiftNum; ++dwIdx)
        {
            if (pstSession->m_stUserInfo.m_stAlGifts[dwIdx].m_nId == ddwGiftId)
            {
                ptbAlGift = &pstSession->m_stUserInfo.m_stAlGifts[dwIdx];
                dwCurAlGiftIdx = dwIdx;
                break;
            }
        }
        if (NULL == ptbAlGift)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AlGiftOpen: no such gift[id=%ld] [seq=%u]",
                ddwGiftId, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }
        else if (ptbAlGift->m_nCtime + AL_IAP_GIFT_EXPIRE_TIME < static_cast<TINT64>(CTimeUtils::GetCurTimeUs()))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AlGiftOpen: gift could not open[id=%ld][endtime=%ld] [seq=%u]",
                ddwGiftId, (ptbAlGift->m_nCtime + AL_IAP_GIFT_EXPIRE_TIME), pstSession->m_stUserInfo.m_udwBSeqNo));
            return -4;
        }
        SRefreshData stRefreshData;
        vector<DataCenterRspInfo*>& vecRsp = pstSession->m_vecDataCenterRsp;
        DataCenterRspInfo *pstDataCenterRsp = NULL;
        if (vecRsp.size() > 0)
        {
            stRefreshData.Reset();
            Json::Reader jsonReader;
            Json::Value oRspDataJson;
            for (TUINT32 udwIdx = 0; udwIdx < vecRsp.size(); ++udwIdx)
            {
                pstDataCenterRsp = vecRsp[udwIdx];
                if (pstDataCenterRsp->m_udwType == EN_REFRESH_DATA_TYPE__AL_GIFT_NEW)
                {
                    TSE_LOG_DEBUG(pstSession->m_poServLog, ("AlGiftOpen: data center get rsp: [uid=%ld] [seq=%u] [json=%s]",
                        pstUser->m_tbPlayer.m_nUid, pstSession->m_udwSeqNo, pstDataCenterRsp->m_sRspJson.c_str()));

                    if (FALSE == jsonReader.parse(pstDataCenterRsp->m_sRspJson, oRspDataJson))
                    {
                        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_DATA_CENTER_PACKAGE_ERR;
                        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AlGiftOpen: prase rsp from data center failed. [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                        return -6;
                    }

                    TINT32 dwRetCode = stRefreshData.m_stAllianceGiftRsp.setVal(oRspDataJson);
                    if (0 != dwRetCode)
                    {
                        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__DATA_CENTER_PACKAGE_FORMAT_ERR;
                        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AlGiftOpen: response data format error. [ret=%d][seq=%u]", dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
                        return -7;
                    }
                    if (stRefreshData.m_stAllianceGiftRsp.m_nGiftNum != 1)
                    {
                        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_DATA_CENTER_PACKAGE_ERR;
                        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AlGiftOpen: algift rewards num != 1. [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                        return -8;
                    }
                    break;
                }
            }
            SGlobalRes stGlobalRes;
            stGlobalRes.Reset();
            for (TUINT32 udwIdx = 0; udwIdx < MAX_REWARD_ITEM_NUM && udwIdx < stRefreshData.m_stAllianceGiftRsp.m_sAllianceGift[0].m_vecReward.size(); ++udwIdx)
            {
                stGlobalRes[stGlobalRes.ddwTotalNum].ddwType = stRefreshData.m_stAllianceGiftRsp.m_sAllianceGift[0].m_vecReward[udwIdx]->ddwType;
                stGlobalRes[stGlobalRes.ddwTotalNum].ddwId = stRefreshData.m_stAllianceGiftRsp.m_sAllianceGift[0].m_vecReward[udwIdx]->ddwId;
                stGlobalRes[stGlobalRes.ddwTotalNum].ddwNum = stRefreshData.m_stAllianceGiftRsp.m_sAllianceGift[0].m_vecReward[udwIdx]->ddwNum;
                stGlobalRes.ddwTotalNum++;
                ptbAlGiftReward->m_bReward[ptbAlGiftReward->m_bReward.m_udwNum].ddwType = stRefreshData.m_stAllianceGiftRsp.m_sAllianceGift[0].m_vecReward[udwIdx]->ddwType;
                ptbAlGiftReward->m_bReward[ptbAlGiftReward->m_bReward.m_udwNum].ddwId = stRefreshData.m_stAllianceGiftRsp.m_sAllianceGift[0].m_vecReward[udwIdx]->ddwId;
                ptbAlGiftReward->m_bReward[ptbAlGiftReward->m_bReward.m_udwNum].ddwNum = stRefreshData.m_stAllianceGiftRsp.m_sAllianceGift[0].m_vecReward[udwIdx]->ddwNum;
                ptbAlGiftReward->m_bReward.m_udwNum++;
            }
            ptbAlGiftReward->SetFlag(TbAL_GIFT_REWARD_FIELD_REWARD);
            CGlobalResLogic::AddGlobalRes(&pstSession->m_stUserInfo, pstCity, &stGlobalRes);
            pstSession->m_stUserInfo.m_tbAlliance.Set_Gift_point(pstSession->m_stUserInfo.m_tbAlliance.m_nGift_point + stRefreshData.m_stAllianceGiftRsp.m_sAllianceGift[0].ddwGiftPoint);
            ptbAlGiftReward->Set_Status(EN_AL_GIFT_STATUS_OPENED);

            ptbAlGift->Set_Gift_point(stRefreshData.m_stAllianceGiftRsp.m_sAllianceGift[0].ddwGiftPoint);
            pstSession->m_stUserInfo.m_stAlGifts.m_aucUpdateFlag[dwCurAlGiftIdx] = EN_TABLE_UPDT_FLAG__CHANGE;


        }

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        CQuestLogic::SetTaskCurValue(&pstSession->m_stUserInfo, pstCity, EN_TASK_TYPE_COLLECT_ALLIANCE_GIFT);
    }

    
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessAlliance::ProcessCmd_AlGiftGet(SSession *pstSession, TBOOL &bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo* pstCity = &pstUser->m_stCityInfo;
    TINT64 ddwCurTime = CTimeUtils::GetCurTimeUs();
    SAlGiftList* pstAlGifts = &pstSession->m_stUserInfo.m_stAlGifts;

    if(pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos == EN_ALLIANCE_POS__REQUEST)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NOT_IN_ALLIANCE;
        return -1;
    }

    TUINT32 audwExpired[AL_GIFT_OPEN_RSP_NUM];
    TUINT32 audwAlReward[AL_GIFT_OPEN_RSP_NUM];
    TUINT32 udwExpiredNum = 0;

    for (TINT32 dwIdx = 0; dwIdx < pstAlGifts->m_dwGiftNum; ++dwIdx)
    {
        TbAl_gift* ptbAlGift = &((*pstAlGifts)[dwIdx]);
        TbAl_gift_reward *ptbAlGiftReward = NULL;
        TINT32 dwRewardIdx = -1;
        for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_stUserInfo.m_udwAlGiftRewardNum; udwIdx++)
        {
            if (pstSession->m_stUserInfo.m_atbAlGiftReward[udwIdx].m_nGid == ptbAlGift->m_nId)
            {
                ptbAlGiftReward = &pstSession->m_stUserInfo.m_atbAlGiftReward[udwIdx];
                dwRewardIdx = udwIdx;
                break;
            }
        }
        if (NULL == ptbAlGiftReward)
        {
            continue;
        }

        if (ptbAlGiftReward->m_nStatus == EN_AL_GIFT_STATUS_NORMAL && (ptbAlGift->m_nCtime + AL_IAP_GIFT_EXPIRE_TIME) < ddwCurTime &&
            dwRewardIdx >= 0)
        {
            audwExpired[udwExpiredNum] = dwIdx;
            audwAlReward[udwExpiredNum] = dwRewardIdx;
            ++udwExpiredNum;
            if (udwExpiredNum >= AL_GIFT_OPEN_RSP_NUM)
            {
                break;
            }
        }
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        if (udwExpiredNum > 0)
        {
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__DATA_CENTER;
            pstSession->ResetDataCenterReq();

            DataCenterReqInfo* pstReq = new DataCenterReqInfo;
            pstReq->m_udwType = EN_REFRESH_DATA_TYPE__AL_GIFT_NEW;
            Json::Value rDataReqJson = Json::Value(Json::objectValue);

            rDataReqJson["sid"] = pstUser->m_tbLogin.m_nSid;
            rDataReqJson["uid"] = pstUser->m_tbPlayer.m_nUid;
            rDataReqJson["aid"] = pstUser->m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET;
            rDataReqJson["gift_lv"] = CCommonBase::GetAlGiftLevel(&pstUser->m_tbAlliance);
            rDataReqJson["castle_lv"] = CCityBase::GetBuildingLevelByFuncType(&pstUser->m_stCityInfo, EN_BUILDING_TYPE__CASTLE);
            rDataReqJson["request"] = Json::Value(Json::objectValue);
            rDataReqJson["request"]["num"] = udwExpiredNum;
            rDataReqJson["request"]["flag"] = 1;
            rDataReqJson["request"]["list"] = Json::Value(Json::arrayValue);
            for (TUINT32 udwIdx = 0; udwIdx < udwExpiredNum; ++udwIdx)
            {
                Json::Value jsonReward = Json::Value(Json::objectValue);
                jsonReward["pack_id"] = pstAlGifts->m_atbGifts[audwExpired[udwIdx]].m_nPack_id;
                jsonReward["request_type"] = EN_REFRESH_DATA_REQ_TYPE__ALLIANCE_GIFT_ALL;
                rDataReqJson["request"]["list"][udwIdx] = jsonReward;
            }

            Json::FastWriter rEventWriter;
            pstReq->m_sReqContent = rEventWriter.write(rDataReqJson);
            pstSession->m_vecDataCenterReq.push_back(pstReq);

            TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_AlGiftGet: data center req: [type=%u] [uid=%ld] [seq=%u] [json=%s]",
                pstReq->m_udwType, pstUser->m_tbPlayer.m_nUid, pstSession->m_udwSeqNo, pstReq->m_sReqContent.c_str()));
            bNeedResponse = TRUE;
            TINT32 dwRetCode = CBaseProcedure::SendDataCenterRequest(pstSession, EN_SERVICE_TYPE_DATA_CENTER_REQ);
            if (dwRetCode == 0)
            {
                return 0;
            }
            else
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_DATA_CENTER_REQ_ERR;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AlGiftGet: send request to data center failed. [ret=%d] [seq=%u]",
                    dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
                return -5;
            }
        }
        else
        {
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
            return 0;
        }
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        SRefreshData stRefreshData;
        vector<DataCenterRspInfo*>& vecRsp = pstSession->m_vecDataCenterRsp;
        DataCenterRspInfo *pstDataCenterRsp = NULL;

        if (vecRsp.size() > 0)
        {
            stRefreshData.Reset();
            Json::Reader jsonReader;
            Json::Value oRspDataJson;
            for (TUINT32 udwIdx = 0; udwIdx < vecRsp.size(); ++udwIdx)
            {
                pstDataCenterRsp = vecRsp[udwIdx];
                if (pstDataCenterRsp->m_udwType == EN_REFRESH_DATA_TYPE__AL_GIFT_NEW)
                {
                    TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_AlGiftGet: data center get rsp: [json=%s] [uid=%ld][seq=%u]",
                        pstDataCenterRsp->m_sRspJson.c_str(), pstUser->m_tbPlayer.m_nUid, pstSession->m_udwSeqNo));

                    if (FALSE == jsonReader.parse(pstDataCenterRsp->m_sRspJson, oRspDataJson))
                    {
                        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_DATA_CENTER_PACKAGE_ERR;
                        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AlGiftGet: prase rsp from data center failed. [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                        return -6;
                    }
                    TINT32 dwRetCode = stRefreshData.m_stAllianceGiftRsp.setVal(oRspDataJson);
                    if (0 != dwRetCode)
                    {
                        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__DATA_CENTER_PACKAGE_FORMAT_ERR;
                        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AlGiftGet: response data format error. [ret=%d][seq=%u]", dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
                        return -7;
                    }
                    if (stRefreshData.m_stAllianceGiftRsp.m_nGiftNum != udwExpiredNum)
                    {
                        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_DATA_CENTER_PACKAGE_ERR;
                        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AlGiftGet: algift rewards num != expireNum[%u]. [seq=%u]",
                            udwExpiredNum, pstSession->m_stUserInfo.m_udwBSeqNo));
                        return -8;
                    }
                    break;
                }
            }

            for (TUINT32 udwGiftIdx = 0; udwGiftIdx < udwExpiredNum; ++udwGiftIdx)
            {
                TbAl_gift_reward *ptbAlGiftReward = &pstSession->m_stUserInfo.m_atbAlGiftReward[audwAlReward[udwGiftIdx]];
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_AlGiftGet: rsp al_reward gid[%ld]. [seq=%u]",
                    ptbAlGiftReward->m_nGid, pstSession->m_stUserInfo.m_udwBSeqNo));

                for (TUINT32 udwIdx = 0; udwIdx < MAX_REWARD_ITEM_NUM && udwIdx < stRefreshData.m_stAllianceGiftRsp.m_sAllianceGift[udwGiftIdx].m_vecReward.size(); ++udwIdx)
                {
                    SOneGlobalRes *pstReward = stRefreshData.m_stAllianceGiftRsp.m_sAllianceGift[udwGiftIdx].m_vecReward[udwIdx];
                    ptbAlGiftReward->m_bReward[ptbAlGiftReward->m_bReward.m_udwNum].ddwType = pstReward->ddwType;
                    ptbAlGiftReward->m_bReward[ptbAlGiftReward->m_bReward.m_udwNum].ddwId = pstReward->ddwId;
                    ptbAlGiftReward->m_bReward[ptbAlGiftReward->m_bReward.m_udwNum].ddwNum = pstReward->ddwNum;
                    ptbAlGiftReward->m_bReward.m_udwNum++;
                }
                ptbAlGiftReward->SetFlag(TbAL_GIFT_REWARD_FIELD_REWARD);
                ptbAlGiftReward->Set_Status(EN_AL_GIFT_STATUS_EXPIRED);
            }

            pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        }
        else
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AlGiftGet: get datacenter rsp vec size[%u]. [seq=%u]",
                vecRsp.size(), pstSession->m_stUserInfo.m_udwBSeqNo));
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        }
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessAlliance::ProcessCmd_AlGiftDel(SSession *pstSession, TBOOL &bNeedResponse)
{
    TbAl_gift_reward *ptbAlGiftReward = NULL;
    TbAl_gift* ptbAlGift = NULL;

    if(pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos == EN_ALLIANCE_POS__REQUEST)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NOT_IN_ALLIANCE;
        return -1;
    }
    TINT64 ddwGiftId = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 0);

    for(TINT32 dwIdx = 0; dwIdx < pstSession->m_stUserInfo.m_stAlGifts.m_dwGiftNum; ++dwIdx)
    {
        if(pstSession->m_stUserInfo.m_stAlGifts[dwIdx].m_nId == ddwGiftId)
        {
            ptbAlGift = &pstSession->m_stUserInfo.m_stAlGifts[dwIdx];
            break;
        }
    }
    if(NULL == ptbAlGift)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AlGiftDel: no such gift[id=%ld] [seq=%u]",
            ddwGiftId, pstSession->m_stUserInfo.m_udwBSeqNo));
        return 0;
    }
    for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_stUserInfo.m_udwAlGiftRewardNum; ++udwIdx)
    {
        if(pstSession->m_stUserInfo.m_atbAlGiftReward[udwIdx].m_nGid == ddwGiftId)
        {
            ptbAlGiftReward = &pstSession->m_stUserInfo.m_atbAlGiftReward[udwIdx];
            break;
        }
    }

    if(NULL == ptbAlGiftReward)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AlGiftOpen: gift status error. [id=%ld][status=%d] [seq=%u]",
            ddwGiftId, 0, pstSession->m_stUserInfo.m_udwBSeqNo));
        return 0;
    }
    // set data package
    ptbAlGiftReward->Set_Status(EN_AL_GIFT_STATUS_CLEARED);

    return 0;
}

TINT32 CProcessAlliance::ProcessCmd_AlGiftDelAll(SSession *pstSession, TBOOL &bNeedResponse)
{
    TbAl_gift_reward* ptbAlGift = NULL;

    if(pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos == EN_ALLIANCE_POS__REQUEST)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NOT_IN_ALLIANCE;
        return -1;
    }
    TBOOL bNeedDelete = FALSE;
    for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_stUserInfo.m_udwAlGiftRewardNum && udwIdx < MAX_AL_IAP_GIFT_NUM * 2; udwIdx++)
    {
        ptbAlGift = &pstSession->m_stUserInfo.m_atbAlGiftReward[udwIdx];
        bNeedDelete = FALSE;
        if(ptbAlGift->m_nStatus == EN_AL_GIFT_STATUS_OPENED)
        {
            bNeedDelete = TRUE;
        }
        else
        {
            for(TINT32 dwIdx = 0; dwIdx < pstSession->m_stUserInfo.m_stAlGifts.m_dwGiftNum; dwIdx++)
            {
                if(pstSession->m_stUserInfo.m_stAlGifts[dwIdx].m_nId == ptbAlGift->m_nGid
                    && (TUINT64)(pstSession->m_stUserInfo.m_stAlGifts[dwIdx].m_nCtime + AL_IAP_GIFT_EXPIRE_TIME) < CTimeUtils::GetCurTimeUs())
                {
                    bNeedDelete = TRUE;
                }
            }
        }

        if(TRUE == bNeedDelete)
        {
            ptbAlGift->Set_Status(EN_AL_GIFT_STATUS_CLEARED);
        }
    }

    return 0;
}

/************************************************private**********************************************/

TVOID CProcessAlliance::PlayerLeaveAllianceUpdtAction(SSession *pstSession, TINT64 ddwSrcUid)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    CActionBase::UpdtActionWhenLeaveAlliance(pstUser, pstSession->m_stReqParam.m_udwSvrId, ddwSrcUid);
}

TINT32 CProcessAlliance::GetAidArrayByDiplomacy(SSession *pstSession, TUINT32 udwSvrId, TbDiplomacy *pstList, TUINT32 udwListSize, TUINT8 *pucFlag, TUINT8 ucDiplomacy, vector<TUINT32>& vecAid)
{
    // select 
    for(TUINT32 idx = 0; idx < udwListSize; idx++)
    {
        if(pucFlag[idx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }

        if(ucDiplomacy == EN_DIPLOMACY_TYPE__HOSTILE)
        {
            if(pstList[idx].m_nType == ucDiplomacy)
            {
                vecAid.push_back(pstList[idx].m_nDes_al);
            }
        }
        else
        {
            if(pstList[idx].m_nType > EN_DIPLOMACY_TYPE__HOSTILE)
            {
                vecAid.push_back(pstList[idx].m_nDes_al);
            }
        }
    }

    return 0;
}

TINT32 CProcessAlliance::ParseAid(TUINT64 src_to_des, TINT32 type) 
{
    if(type == EN_ALLIANCE_PARSE_TYPE__SOURCE)
    {
        TINT32 ret=0;
        ret = static_cast<TINT32> (src_to_des >> 32);
        return ret;
    }
    else if(type == EN_ALLIANCE_PARSE_TYPE__DESTINATION)
    {
        TINT32 ret=0;
        ret = static_cast<TINT32> (src_to_des % (1UL << 32));
        return ret;
    }
    else
    {
        return -1;
    }
}

TUINT64 CProcessAlliance::GetSrcToDes(TUINT32 source, TUINT32 destination)
{
    TUINT64 ret = source;
    ret = (ret << 32) + destination;
    return ret;
}

TUINT64 CProcessAlliance::GetDesToSrc(TUINT32 source, TUINT32 destination)
{
    TUINT64 ret = destination;
    ret = (ret << 32) + source;
    return ret;
}

TINT32 CProcessAlliance::ProcessCmd_AlItemMark(SSession *pstSession, TBOOL &bNeedResponse)
{
    if(pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos == EN_ALLIANCE_POS__REQUEST)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NOT_IN_ALLIANCE;
        return -1;
    }

    SReqParam& stReqParam = pstSession->m_stReqParam;
    TbUser_stat& tbStat = pstSession->m_stUserInfo.m_tbUserStat;
    TbUser_stat_Mark& bMark = tbStat.m_bMark;
    TbAlliance& tbAlliance = pstSession->m_stUserInfo.m_tbAlliance;
    TbAlliance_Al_store_item& bAlIStoreItem = tbAlliance.m_bAl_store_item;
    TUINT32 udwItemId = atoi(stReqParam.m_szKey[0]);

    CGameInfo *pGameInfo = CGameInfo::GetInstance();

    ostringstream ossItemId;
    ossItemId << udwItemId;

    ostringstream ossSvrId;
    ossSvrId << pstSession->m_stReqParam.m_udwSvrId;

    const Json::Value &stGameJson = pGameInfo->m_oJsonRoot["game_item"];
    TUINT32 udwItemType = 0;


    if(!stGameJson.isMember(ossItemId.str()))
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AlItemMark: not such item[%u]. [seq=%u]", udwItemId, pstSession->m_udwSeqNo));
        return -2;
    }

    udwItemType = stGameJson[ossItemId.str()]["a8"].asUInt();
    TSE_LOG_INFO(pstSession->m_poServLog, (" ProcessCmd__AlItemMark type=%u,itemid=%u [seq=%u]",
        udwItemType, udwItemId, pstSession->m_udwSeqNo));

    //非联盟store item
    if((udwItemType & (1 << EN_ALLIANCE_ITEM_STORE)) == 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AlItemMark: not alliance store item[%u]. [seq=%u]", udwItemId, pstSession->m_udwSeqNo));
        return -3;
    }

    TBOOL bNewMarkFlag = TRUE;
    for(unsigned int i = 0; i < bMark.m_udwNum; ++i)
    {
        if(bMark[i].ddwItemId == udwItemId)
        {
            bMark[i].ddwMarkTime = CTimeUtils::GetUnixTime();
            bNewMarkFlag = FALSE;
            break;
        }
    }

    if(bNewMarkFlag)
    {
        //超过mark上限时 清除最旧的那个
        if(bMark.m_udwNum >= TBUSER_STAT_MARK_MAX_NUM)
        {
            for(unsigned int i = 0; i < bAlIStoreItem.m_udwNum; ++i)
            {
                if(bAlIStoreItem[i].ddwItemId == bMark[0].ddwItemId)
                {
                    //减少自己最早mark的那个,并且没有被盟主清除mark
                    if(bAlIStoreItem[i].ddwClearTime<bMark[0].ddwMarkTime && bAlIStoreItem[i].ddwStar>0)
                    {
                        bAlIStoreItem[i].ddwStar--;
                    }
                    //如果已经无人mark并且,没有剩余 则清除
                    if(0 == bAlIStoreItem[i].ddwNum && 0 == bAlIStoreItem[i].ddwStar) //从物品列表中删除
                    {
                        bAlIStoreItem[i] = bAlIStoreItem[bAlIStoreItem.m_udwNum - 1];
                        bAlIStoreItem.m_udwNum--;
                    }
                    break;
                }
            }
            memcpy(&bMark[0], &bMark[1], sizeof(bMark[0])*(TBUSER_STAT_MARK_MAX_NUM - 1));
            bMark.m_udwNum--;
        }
        //新增mark
        bMark[bMark.m_udwNum].ddwItemId = udwItemId;
        bMark[bMark.m_udwNum].ddwMarkTime = CTimeUtils::GetUnixTime();
        bMark.m_udwNum++;
    }
    tbStat.SetFlag(TbUSER_STAT_FIELD_MARK);

    TBOOL bFlag = FALSE;
    for(unsigned int i = 0; i < bAlIStoreItem.m_udwNum; ++i)
    {
        if(bAlIStoreItem[i].ddwItemId == udwItemId)
        {
            bAlIStoreItem[i].ddwStar++;
            bFlag = TRUE;
            break;
        }
    }
    if(!bFlag) //新增store item
    {
        bAlIStoreItem[bAlIStoreItem.m_udwNum].ddwItemId = udwItemId;
        bAlIStoreItem[bAlIStoreItem.m_udwNum].ddwNum = 0;
        bAlIStoreItem[bAlIStoreItem.m_udwNum].ddwStar = 1;
        bAlIStoreItem.m_udwNum++;
    }

    tbAlliance.SetFlag(TbALLIANCE_FIELD_AL_STORE_ITEM);

    return 0;
}

TINT32 CProcessAlliance::ProcessCmd_AlItemUnmark(SSession *pstSession, TBOOL &bNeedResponse)
{
    if(pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos == EN_ALLIANCE_POS__REQUEST)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NOT_IN_ALLIANCE;
        return -1;
    }

    SReqParam& stReqParam = pstSession->m_stReqParam;
    TbUser_stat& tbStat = pstSession->m_stUserInfo.m_tbUserStat;
    TbUser_stat_Mark& bMark = tbStat.m_bMark;
    TbAlliance& tbAlliance = pstSession->m_stUserInfo.m_tbAlliance;
    TbAlliance_Al_store_item& bAlStoreItem = tbAlliance.m_bAl_store_item;
    TUINT32 udwItemId = atoi(stReqParam.m_szKey[0]);

    CGameInfo *pGameInfo = CGameInfo::GetInstance();

    TUINT32 udwItemType = 0;

    TCHAR szSid[128];
    TCHAR szItemId[256];
    snprintf(szSid, 128, "%u", pstSession->m_stReqParam.m_udwSvrId);
    snprintf(szItemId, 256, "%u", udwItemId);

    const Json::Value &stGameJson = pGameInfo->m_oJsonRoot["game_item"];

    if(!stGameJson.isMember(szItemId))
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AlItemUnmark: not such item[%u]. [seq=%u]", udwItemId, pstSession->m_udwSeqNo));
        return -2;
    }

    udwItemType = stGameJson[szItemId]["a8"].asUInt();
    TSE_LOG_INFO(pstSession->m_poServLog, (" ProcessCmd__AlItemUnmark type=%u,itemid=%u [seq=%u]",
        udwItemType, udwItemId, pstSession->m_udwSeqNo));

    TBOOL bFlag = FALSE;
    TUINT32 udwMarkTime = 0;
    for(unsigned int i = 0; i < bMark.m_udwNum; ++i)
    {
        if(bMark[i].ddwItemId == udwItemId)
        {
            udwMarkTime = bMark[i].ddwMarkTime;
            if(i < bMark.m_udwNum - 1)
            {
                memcpy(&bMark[i], &bMark[i + 1], sizeof(bMark[0])*(bMark.m_udwNum - i - 1));
            }
            bMark.m_udwNum--;
            bFlag = TRUE;
            break;
        }
    }
    if(!bFlag)
    {
        return 0;
    }


    tbStat.SetFlag(TbUSER_STAT_FIELD_MARK);
    bFlag = FALSE;
    for(unsigned int i = 0; i < bAlStoreItem.m_udwNum; ++i)
    {
        if(bAlStoreItem[i].ddwItemId == udwItemId)
        {
            if(bAlStoreItem[i].ddwClearTime<udwMarkTime && bAlStoreItem[i].ddwStar>0)
            {
                bAlStoreItem[i].ddwStar--;
            }
            if(0 == bAlStoreItem[i].ddwNum && 0 == bAlStoreItem[i].ddwStar) //从物品列表中删除
            {
                bAlStoreItem[i] = bAlStoreItem[bAlStoreItem.m_udwNum - 1];
                bAlStoreItem.m_udwNum--;
            }
            bFlag = TRUE;
            break;
        }
    }
    if(bFlag)
    {
        tbAlliance.SetFlag(TbALLIANCE_FIELD_AL_STORE_ITEM);
    }

    return 0;
}

TINT32 CProcessAlliance::ProcessCmd_AlItemMarkClear(SSession *pstSession, TBOOL &bNeedResponse)
{
    if(pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos == EN_ALLIANCE_POS__REQUEST)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NOT_IN_ALLIANCE;
        return -1;
    }

    TUINT32 udwItemId = atoi(pstSession->m_stReqParam.m_szKey[0]);

    TbPlayer& tbPlayer = pstSession->m_stUserInfo.m_tbPlayer;
    TbAlliance& tbAlliance = pstSession->m_stUserInfo.m_tbAlliance;
    TbAlliance_Al_store_item& bAlStoreItem = tbAlliance.m_bAl_store_item;
    //权限不足
    if(tbPlayer.m_nAlpos < EN_ALLIANCE_POS__VICE_CHANCELOR)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AlItemMarkClear: have not authority. [seq=%u]", pstSession->m_udwSeqNo));
        return -2;
    }

    for(unsigned int i = 0; i < bAlStoreItem.m_udwNum; ++i)
    {
        if(bAlStoreItem[i].ddwItemId == udwItemId)
        {
            bAlStoreItem[i].ddwStar = 0;
            bAlStoreItem[i].ddwClearTime = CTimeUtils::GetUnixTime();
            tbAlliance.SetFlag(TbALLIANCE_FIELD_AL_STORE_ITEM);
            break;
        }
    }

    return 0;
}

TINT32 CProcessAlliance::ProcessCmd_AlItemExchange(SSession *pstSession, TBOOL &bNeedResponse)
{
    if(pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos == EN_ALLIANCE_POS__REQUEST)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NOT_IN_ALLIANCE;
        return -1;
    }

    TUINT32 udwItemId = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TUINT32 udwItemNum = atoi(pstSession->m_stReqParam.m_szKey[1]);

    TbPlayer& tbPlayer = pstSession->m_stUserInfo.m_tbPlayer;
    TbAlliance& tbAlliance = pstSession->m_stUserInfo.m_tbAlliance;
    TbAlliance_Al_store_item& bAlStoreItem = tbAlliance.m_bAl_store_item;
    TbAl_store_consume& tbTmpAlConsume = pstSession->m_tbTmpAlConsume;

    CGameInfo *pGameInfo = CGameInfo::GetInstance();

    TCHAR szSid[128];
    TCHAR szItemId[256];
    snprintf(szSid, 128, "%u", pstSession->m_stReqParam.m_udwSvrId);
    snprintf(szItemId, 256, "%u", udwItemId);

    TUINT32 udwItemType = 0;
    TUINT32 udwCostLoyalty = 0;

    const Json::Value &stGameJson = pGameInfo->m_oJsonRoot["game_item"];
    if(!stGameJson.isMember(szItemId))
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AlItemExchange: not such item[%u]. [seq=%u]", udwItemId, pstSession->m_udwSeqNo));
        return -2;
    }

    udwItemType = stGameJson[szItemId]["a8"].asUInt();
    udwCostLoyalty = stGameJson[szItemId]["a10"].asUInt(); //消耗的忠诚度

    //忠诚度不够
    if(udwItemNum*udwCostLoyalty > tbPlayer.m_nLoy_cur)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AlItemExchange: costLoyalty=%u,ownloyalty[%u] not enough. [seq=%u]", udwCostLoyalty, tbPlayer.m_nLoy_cur, pstSession->m_udwSeqNo));
        return -3;
    }
    
    //非联盟商店的物品
    if ((udwItemType&(1 << EN_ALLIANCE_ITEM_STORE)) == 0 && (udwItemType&(1 << EN_ALLIANCE_ITEM_BASIC)) == 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AlItemExchange: not alliance item[%u]. [seq=%u]", udwItemId, pstSession->m_udwSeqNo));
        return -4;
    }
    if (udwItemType&(1 << EN_ALLIANCE_ITEM_BASIC))
    {
        //无限买
    }
    else if (udwItemType&(1 << EN_ALLIANCE_ITEM_STORE))
    {
        SAlStoreItem* pstOneAlItem = NULL;
        for (unsigned int i = 0; i < bAlStoreItem.m_udwNum; ++i)
        {
            if (bAlStoreItem[i].ddwItemId == udwItemId)
            {
                pstOneAlItem = &bAlStoreItem[i];
                break;
            }
        }
        if(pstOneAlItem == NULL)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AlItemExchange: alliance store item[%d] not exist yet. [seq=%u]", udwItemId, pstSession->m_udwSeqNo));
            return -5;
        }
        if (pstOneAlItem->ddwNum < udwItemNum)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AlItemExchange: alliance store item[%d] not enough. do nothing but return suc [seq=%u]", udwItemId, pstSession->m_udwSeqNo));
            return 0;
            //pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__STORE_ITEM_NOT_ENOUGH;
            //TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AlItemExchange: alliance store item[%d] not enough. [seq=%u]", udwItemId, pstSession->m_udwSeqNo));
            //return -5;
        }
        pstOneAlItem->ddwNum -= udwItemNum;
        //llt add, 打星的用户兑换走之后, 减少打星数量

        tbAlliance.SetFlag(TbALLIANCE_FIELD_AL_STORE_ITEM);
    }

    CItemBase::AddItem(&pstSession->m_stUserInfo.m_tbBackpack, udwItemId, udwItemNum);
    tbPlayer.Set_Loy_cur(tbPlayer.m_nLoy_cur - udwItemNum*udwCostLoyalty);

    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    TINT64 ddwConsumeId = udwCurTime;
    ddwConsumeId = (ddwConsumeId << 32) + tbPlayer.m_nUid;
    tbTmpAlConsume.Set_Aid(pstSession->m_stReqParam.m_udwAllianceId);
    tbTmpAlConsume.Set_Id(ddwConsumeId);
    tbTmpAlConsume.Set_Uid(tbAlliance.m_nAid);
    tbTmpAlConsume.Set_Item_id(udwItemId);
    tbTmpAlConsume.Set_Item_num(udwItemNum);
    tbTmpAlConsume.Set_Time(udwCurTime);
    tbTmpAlConsume.Set_Loyalty(udwItemNum*udwCostLoyalty);
    tbTmpAlConsume.Set_Uname(tbPlayer.m_sUin);
    pstSession->m_ucTmpAlConsumeFlag = EN_TABLE_UPDT_FLAG__NEW;

    return 0;
}

TINT32 CProcessAlliance::ProcessCmd_AlItemBuy(SSession *pstSession, TBOOL &bNeedResponse)
{
    if(pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos == EN_ALLIANCE_POS__REQUEST)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NOT_IN_ALLIANCE;
        return -1;
    }

    TUINT32 udwItemId = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TUINT32 udwItemNum = atoi(pstSession->m_stReqParam.m_szKey[1]);

    TbPlayer& tbPlayer = pstSession->m_stUserInfo.m_tbPlayer;
    TbAlliance& tbAlliance = pstSession->m_stUserInfo.m_tbAlliance;
    TbAlliance_Al_store_item& bAlStoreItem = tbAlliance.m_bAl_store_item;

    CGameInfo *pGameInfo = CGameInfo::GetInstance();

    TCHAR szSid[128];
    TCHAR szItemId[256];
    snprintf(szSid, 128, "%u", pstSession->m_stReqParam.m_udwSvrId);
    snprintf(szItemId, 256, "%u", udwItemId);
    TUINT32 udwItemType = 0;
    TUINT32 udwCostFund = 0;

    const Json::Value &stGameJson = pGameInfo->m_oJsonRoot["game_item"];

    if(!stGameJson.isMember(szItemId))
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AlItemBuy: not such item[%u]. [seq=%u]", udwItemId, pstSession->m_udwSeqNo));
        return -2;
    }

    udwItemType = stGameJson[szItemId]["a8"].asUInt();
    udwCostFund = stGameJson[szItemId]["a9"].asUInt(); //消耗的fund

    //非联盟store item
    if((udwItemType&(1 << EN_ALLIANCE_ITEM_STORE)) == 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AlItemBuy: not alliance store item[%u]. [seq=%u]", udwItemId, pstSession->m_udwSeqNo));
        return -3;
    }

    //权限不足
    if(tbPlayer.m_nAlpos < EN_ALLIANCE_POS__VICE_CHANCELOR)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__HAVE_NO_PERMISSION;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AlItemBuy: have not authority. [seq=%u]", pstSession->m_udwSeqNo));
        return -4;
    }

    //联盟基金不足
    if(tbAlliance.m_nFund < udwCostFund*udwItemNum)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__AL_FUND_NOT_ENOUGH;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AlItemBuy: have not authority. [seq=%u]", pstSession->m_udwSeqNo));
        return -5;
    }

    bool bFlag = false;
    for(unsigned int i = 0; i < bAlStoreItem.m_udwNum; ++i)
    {
        if(bAlStoreItem[i].ddwItemId == udwItemId)
        {
            bAlStoreItem[i].ddwNum += udwItemNum;
            if(bAlStoreItem[i].ddwStar <= udwItemNum)
            {
                bAlStoreItem[i].ddwStar = 0;
            }
            else
            {
                bAlStoreItem[i].ddwStar -= udwItemNum;
            }
            bFlag = true;
            break;
        }
    }
    if(!bFlag) //新增store item
    {
        bAlStoreItem[bAlStoreItem.m_udwNum].ddwItemId = udwItemId;
        bAlStoreItem[bAlStoreItem.m_udwNum].ddwNum = udwItemNum;
        bAlStoreItem[bAlStoreItem.m_udwNum].ddwStar = 0;
        bAlStoreItem.m_udwNum++;
    }
    tbAlliance.SetFlag(TbALLIANCE_FIELD_AL_STORE_ITEM);
    tbAlliance.Set_Fund(tbAlliance.m_nFund - udwCostFund*udwItemNum);

    return 0;
}

TINT32 CProcessAlliance::ProcessCmd_AlDiplomacyGet(SSession *pstSession, TBOOL &bNeedResponse)
{
    if(pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos == EN_ALLIANCE_POS__REQUEST)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NOT_IN_ALLIANCE;
        return -1;
    }

    TINT32 dwRetCode = 0;

    //获取参数，要拉回的列表类型，friendly or hostile
    TUINT8 udwSelectType = atoi(pstSession->m_stReqParam.m_szKey[0]);
    if(udwSelectType != EN_DIPLOMACY_TYPE__FRIENDLY && udwSelectType != EN_DIPLOMACY_TYPE__HOSTILE)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }

    // 1. 获取相关联盟信息——请求
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        vector<TUINT32> vecAid;
        GetAidArrayByDiplomacy(pstSession, pstSession->m_stReqParam.m_udwSvrId, pstSession->m_stUserInfo.m_atbDiplomacy,
            pstSession->m_stUserInfo.m_udwDiplomacyNum, pstSession->m_stUserInfo.m_aucDiplomacyFlag, udwSelectType, vecAid);
        if(vecAid.size() == 0)
        {
            // set next procedure
            pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_ALLIANCE_LIST;
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
            return 0;
        }

        pstSession->ResetAwsInfo();
        for(TUINT32 udwIdx = 0; udwIdx < vecAid.size(); ++udwIdx)
        {
            CAwsRequest::AllianceGetByAid(pstSession, vecAid.at(udwIdx));
        }

        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("CProcessAlliance::ProcessCmd__AlDiplomacyGet send aws req failed [seq=%u]", pstSession->m_udwSeqNo));
            return -1;
        }
        return 0;
    }

    // 2. 获取相关联盟信息——解析
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        if(udwSelectType == EN_DIPLOMACY_TYPE__HOSTILE)
        {
            // parse data
            pstSession->m_udwHostilesAlNum = 0;
            for(TUINT32 idx = 0, udwAlIndex = 0; idx < pstSession->m_vecAwsRsp.size(); idx++)
            {
                AwsRspInfo *pstAwsRspInfo = pstSession->m_vecAwsRsp[idx];
                dwRetCode = CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &pstSession->m_atbHostileAl[udwAlIndex]);
                if(dwRetCode > 0)
                {
                    pstSession->m_udwHostilesAlNum += dwRetCode;
                    udwAlIndex += dwRetCode;
                }
                else
                {
                    pstSession->m_atbHostileAl[udwAlIndex].Reset();
                }
            }
            for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_stUserInfo.m_udwDiplomacyNum; udwIdx++)
            {
                if (pstSession->m_stUserInfo.m_aucDiplomacyFlag[udwIdx] != EN_TABLE_UPDT_FLAG__DEL
                    && pstSession->m_stUserInfo.m_atbDiplomacy[udwIdx].m_nType == EN_DIPLOMACY_TYPE__HOSTILE)
                {
                    TBOOL bIsFind = FALSE;
                    TUINT32 udwIdy = 0;
                    for (udwIdy = 0; udwIdy < pstSession->m_udwHostilesAlNum; udwIdy++)
                    {
                        if (pstSession->m_atbHostileAl[udwIdy].m_nAid == pstSession->m_stUserInfo.m_atbDiplomacy[udwIdx].m_nDes_al)
                        {
                            bIsFind = TRUE;
                            break;
                        }
                    }
                    if (!bIsFind)
                    {
                        pstSession->m_stUserInfo.m_aucDiplomacyFlag[udwIdx] = EN_TABLE_UPDT_FLAG__DEL;
                    }
                }
            }
        }
        else if(udwSelectType == EN_DIPLOMACY_TYPE__FRIENDLY)
        {
            // parse data
            pstSession->m_udwFriendAlNum = 0;
            for(TUINT32 idx = 0, udwAlIndex = 0; idx < pstSession->m_vecAwsRsp.size(); idx++)
            {
                AwsRspInfo *pstAwsRspInfo = pstSession->m_vecAwsRsp[idx];
                dwRetCode = CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &pstSession->m_atbFriendAl[udwAlIndex]);
                if(dwRetCode > 0)
                {
                    pstSession->m_udwFriendAlNum += dwRetCode;
                    udwAlIndex += dwRetCode;
                }
                else
                {
                    pstSession->m_atbFriendAl[udwAlIndex].Reset();
                }
            }
            for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_stUserInfo.m_udwDiplomacyNum; udwIdx++)
            {
                if (pstSession->m_stUserInfo.m_aucDiplomacyFlag[udwIdx] != EN_TABLE_UPDT_FLAG__DEL
                    && pstSession->m_stUserInfo.m_atbDiplomacy[udwIdx].m_nType > EN_DIPLOMACY_TYPE__HOSTILE)
                {
                    TBOOL bIsFind = FALSE;
                    TUINT32 udwIdy = 0;
                    for (udwIdy = 0; udwIdy < pstSession->m_udwFriendAlNum; udwIdy++)
                    {
                        if (pstSession->m_atbFriendAl[udwIdy].m_nAid == pstSession->m_stUserInfo.m_atbDiplomacy[udwIdx].m_nDes_al)
                        {
                            bIsFind = TRUE;
                            break;
                        }
                    }
                    if (!bIsFind)
                    {
                        pstSession->m_stUserInfo.m_aucDiplomacyFlag[udwIdx] = EN_TABLE_UPDT_FLAG__DEL;
                    }
                }
            }
        }

        // set next procedure
        pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_ALLIANCE_LIST;
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    }

    return 0;
}

TINT32 CProcessAlliance::ProcessCmd_AlDiplomacySet(SSession *pstSession, TBOOL &bNeedResponse)
{
    if(pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos == EN_ALLIANCE_POS__REQUEST)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NOT_IN_ALLIANCE;
        return -1;
    }

    if(pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos < EN_ALLIANCE_POS__VICE_CHANCELOR)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }
    TUINT32 udwTargetAid = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TUINT8	ucTargetDiplomacy = atoi(pstSession->m_stReqParam.m_szKey[1]);

    Json::Value jsonSelfAlInfo;
    CCommJson::GenAllianceInfo(&pstSession->m_stUserInfo.m_tbAlliance, jsonSelfAlInfo);
    Json::FastWriter tmpWriter;
    string strSelfAlInfo = tmpWriter.write(jsonSelfAlInfo);
    string strSelfAlName = "(" + pstSession->m_stUserInfo.m_tbAlliance.m_sAl_nick_name + ")"
        + pstSession->m_stUserInfo.m_tbAlliance.m_sName;

    TINT32 dwRetCode = 0;

    if(ucTargetDiplomacy == EN_DIPLOMACY_TYPE__PEDDING)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        if(udwTargetAid == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -2;
        }

        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        pstSession->ResetAwsReq();
        //获取目标联盟指向自身联盟的外交关系
        CDiplomacyProcess::TableRequest_GetDiplomacyByTwoAid(pstSession, udwTargetAid, pstSession->m_stReqParam.m_udwAllianceId);
        //获取目标联盟信息
        CAwsRequest::AllianceGetByAid(pstSession, udwTargetAid);

        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            return -3;
        }
        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        TUINT8 ucTargetDiplomacyFlag = 0;
        TbDiplomacy *pstDiplomacy = NULL;
        TbDiplomacy *pstTargetDiplomacy = &pstSession->m_stUserInfo.m_tbTargetDiplomacy;
        TUINT32 udwIndex = 0;

        TbAlliance tbTargetAlliance;
        Json::Value jsonTargetAlInfo;
        string strTargetAlInfo = "";
        string strTargetAlName = "";

        TBOOL bIsDirtyData = FALSE;

        // 获取结果
        for(TUINT32 dwIdx = 0; dwIdx < pstSession->m_vecAwsRsp.size(); ++dwIdx)
        {
            AwsRspInfo *pstAwsRspInfo = pstSession->m_vecAwsRsp[dwIdx];
            string udwTableName = CCommonFunc::GetTableRawName(pstAwsRspInfo->sTableName.c_str());
            if(udwTableName == EN_AWS_TABLE_DIPLOMACY)
            {
                dwRetCode = CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, pstTargetDiplomacy);
                if(dwRetCode <= 0)
                {
                    pstTargetDiplomacy->Reset();
                    pstTargetDiplomacy->m_nType = EN_DIPLOMACY_TYPE__NORMAL;
                }
                continue;
            }
            if(udwTableName == EN_AWS_TABLE_ALLIANCE)
            {
                dwRetCode = CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &tbTargetAlliance);
                // 不存在联盟记录
                if(dwRetCode <= 0)
                {
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
                    return -4;
                }
                else if (tbTargetAlliance.m_sName.empty())
                {
                    if (ucTargetDiplomacy != EN_DIPLOMACY_TYPE__NORMAL)
                    {
                        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ALLIANCE_NOT_EXIT_IN_THIS_SVR;
                        return -4;
                    }
                    bIsDirtyData = TRUE;
                }
                else if(tbTargetAlliance.m_nSid != pstSession->m_stUserInfo.m_tbAlliance.m_nSid)
                {
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ALLIANCE_NOT_EXIT_IN_THIS_SVR;
                    return -4;
                }

                strTargetAlName = "(" + tbTargetAlliance.m_sAl_nick_name + ")" + tbTargetAlliance.m_sName;
                CCommJson::GenAllianceInfo(&tbTargetAlliance, jsonTargetAlInfo);
                strTargetAlInfo = tmpWriter.write(jsonTargetAlInfo);
                continue;
            }
        }

        // 检查是否已经在现有队列当中 
        for(TUINT32 idx = 0; idx < pstSession->m_stUserInfo.m_udwDiplomacyNum; idx++)
        {
            if(pstSession->m_stUserInfo.m_atbDiplomacy[idx].m_nDes_al == static_cast<TINT64>(udwTargetAid))
            {
                pstDiplomacy = &pstSession->m_stUserInfo.m_atbDiplomacy[idx];
                udwIndex = idx;
                break;
            }
        }

        if(pstDiplomacy == NULL)
        {
            if(ucTargetDiplomacy == EN_DIPLOMACY_TYPE__NORMAL || ucTargetDiplomacy == EN_DIPLOMACY_TYPE__PEDDING)
            {
                pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
                return 0;
            }
        }

        if (bIsDirtyData && pstDiplomacy)
        {
            pstSession->m_stUserInfo.m_aucDiplomacyFlag[udwIndex] = EN_TABLE_UPDT_FLAG__DEL;
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
            return 0;
        }

        // 检查是否超过上限
        if(ucTargetDiplomacy == EN_DIPLOMACY_TYPE__FRIENDLY)
        {
            TUINT32 friendly_num = 0;
            for(TUINT32 idx = 0; idx < pstSession->m_stUserInfo.m_udwDiplomacyNum; idx++)
            {
                if(pstSession->m_stUserInfo.m_atbDiplomacy[idx].m_nType == EN_DIPLOMACY_TYPE__FRIENDLY ||
                    pstSession->m_stUserInfo.m_atbDiplomacy[idx].m_nType == EN_DIPLOMACY_TYPE__PEDDING)
                {
                    friendly_num++;
                }
            }
            if(friendly_num >= MAX_FRIENDLY_DIPLOMACY_NUM)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__FRIEND_ALLIANCE_FULL;
                return -2;
            }
        }

        if(ucTargetDiplomacy == EN_DIPLOMACY_TYPE__HOSTILE)
        {
            TUINT32 hostile_num = 0;
            for(TUINT32 idx = 0; idx < pstSession->m_stUserInfo.m_udwDiplomacyNum; idx++)
            {
                if(pstSession->m_stUserInfo.m_atbDiplomacy[idx].m_nType == EN_DIPLOMACY_TYPE__HOSTILE)
                {
                    hostile_num++;
                }
            }
            if(hostile_num >= MAX_HOSTILE_DIPLOMACY_NUM)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__HOSTILE_ALLIANCE_FULL;
                return -3;
            }
        }

        //新增
        if(pstDiplomacy == NULL)
        {
            udwIndex = pstSession->m_stUserInfo.m_udwDiplomacyNum;
            pstDiplomacy = &pstSession->m_stUserInfo.m_atbDiplomacy[udwIndex];
            pstDiplomacy->Reset();

            pstSession->m_stUserInfo.m_aucDiplomacyFlag[udwIndex] = EN_TABLE_UPDT_FLAG__NEW;
            pstSession->m_stUserInfo.m_udwDiplomacyNum++;
        }
        else
        {
            pstSession->m_stUserInfo.m_aucDiplomacyFlag[udwIndex] = EN_TABLE_UPDT_FLAG__CHANGE;
        }

        if(pstDiplomacy->m_nType == static_cast<TINT64>(ucTargetDiplomacy) ||
            (pstDiplomacy->m_nType == EN_DIPLOMACY_TYPE__PEDDING && ucTargetDiplomacy == EN_DIPLOMACY_TYPE__FRIENDLY))
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AlDiplomacySet: requiring diplomacy is current diplomacy [cur dip=%ld] [req dip=%d] [seq=%u]",
                pstDiplomacy->m_nType, ucTargetDiplomacy, pstSession->m_stUserInfo.m_udwBSeqNo));
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
            return 0;
        }

        if(pstDiplomacy->m_nType == EN_DIPLOMACY_TYPE__NORMAL)
        {
            dwRetCode = CProcessAlliance::ProcessDiplomacyIfHeIsNormal(pstDiplomacy, pstTargetDiplomacy, ucTargetDiplomacy, pstSession->m_stUserInfo.m_tbAlliance, tbTargetAlliance, pstSession->m_stUserInfo.m_aucDiplomacyFlag[udwIndex], ucTargetDiplomacyFlag, pstSession);
        }
        else if(pstDiplomacy->m_nType == EN_DIPLOMACY_TYPE__FRIENDLY)
        {
            dwRetCode = CProcessAlliance::ProcessDiplomacyIfHeIsFriendly(pstDiplomacy, pstTargetDiplomacy, ucTargetDiplomacy, pstSession->m_stUserInfo.m_tbAlliance, tbTargetAlliance, pstSession->m_stUserInfo.m_aucDiplomacyFlag[udwIndex], ucTargetDiplomacyFlag, pstSession);
        }
        else if(pstDiplomacy->m_nType == EN_DIPLOMACY_TYPE__HOSTILE)
        {
            dwRetCode = CProcessAlliance::ProcessDiplomacyIfHeIsHostile(pstDiplomacy, pstTargetDiplomacy, ucTargetDiplomacy, pstSession->m_stUserInfo.m_tbAlliance, tbTargetAlliance, pstSession->m_stUserInfo.m_aucDiplomacyFlag[udwIndex], ucTargetDiplomacyFlag, pstSession);
        }
        else if(pstDiplomacy->m_nType == EN_DIPLOMACY_TYPE__PEDDING)
        {
            dwRetCode = CProcessAlliance::ProcessDiplomacyIfHeIsPedding(pstDiplomacy, pstTargetDiplomacy, ucTargetDiplomacy, pstSession->m_stUserInfo.m_tbAlliance, tbTargetAlliance, pstSession->m_stUserInfo.m_aucDiplomacyFlag[udwIndex], ucTargetDiplomacyFlag, pstSession);
        }
        else
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AlDiplomacySet: diplomacy error [source dip=%ld] [target dip=%ld] [seq=%u]", \
                pstDiplomacy->m_nType, pstTargetDiplomacy->m_nType, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -4;
        }

        if(dwRetCode != 0)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AlDiplomacySet: error [my dip=%ld] [ret=%d] [seq=%u]", pstDiplomacy->m_nType, dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -5;
        }

        // 统一更新TargetDiplomacy
        pstSession->ResetAwsReq();
        dwRetCode = CDiplomacyProcess::TableRequest_DiplomacyChange(pstSession, pstTargetDiplomacy, ucTargetDiplomacyFlag);
        if(pstSession->m_vecAwsReq.size() > 0)
        {
            bNeedResponse = TRUE;
            // next procedure
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
            dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
            if(dwRetCode < 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                return -5;
            }
            return 0;
        }
        else
        {
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
            return 0;
        }
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessAlliance::ProcessCmd_AlTaskHelpReq(SSession *pstSession, TBOOL &bNeedResponse)
{
    SReqParam& stReqParam = pstSession->m_stReqParam;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbPlayer& tbPlayer = pstSession->m_stUserInfo.m_tbPlayer;
    SCityInfo *pstCity = NULL;
    TINT64 ddwTaskId = strtoll(stReqParam.m_szKey[0], NULL, 10);
    TINT32 dwRetCode = 0;

    if(0 == tbPlayer.m_nAlid || EN_ALLIANCE_POS__REQUEST == tbPlayer.m_nAlpos)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NOT_IN_ALLIANCE;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AlTaskHelpReq: you are not in alliance [seq=%u]", \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }

    // 获取action
    TINT32 dwActionIdx = CActionBase::GetAlActionIndex(pstUser->m_atbSelfAlAction, pstUser->m_udwSelfAlActionNum, ddwTaskId);
    if(dwActionIdx < 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AlTaskHelpReq: aciton[%llu] not found [seq=%u]", \
            ddwTaskId, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -2;
    }

    TbAlliance_action& tbAction = pstUser->m_atbSelfAlAction[dwActionIdx];
    pstCity = &pstUser->m_stCityInfo;

    //只有build，research，hospital的等待时间可以免费加速
    if(EN_ACTION_MAIN_CLASS__BUILDING == tbAction.m_nMclass)
    {
    }
    else if(EN_ACTION_MAIN_CLASS__TRAIN_NEW == tbAction.m_nMclass)
    {
    }
    else if(tbAction.m_nMclass == EN_ACTION_MAIN_CLASS__EQUIP && tbAction.m_nSclass == EN_ACTION_SEC_CLASS__EQUIP_UPGRADE)
    {
    }
    else if (tbAction.m_nMclass == EN_ACTION_MAIN_CLASS__DRAGON)
    {
    }
    else
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AlTaskHelpReq: mclass[%lld] can't help. [seq=%u]", \
            tbAction.m_nMclass, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -4;
    }

    //已经请求过了
    if(tbAction.m_nSal != 0 || tbAction.m_nCan_help_num != 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AlTaskHelpReq: task[%llu] already request help. [seq=%u]", ddwTaskId, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -5;
    }

    // updt action info
    TINT32 dwCanHelpTimes = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_ALLIANCE_HELP_NUM].m_ddwBuffTotal;

    tbAction.Set_Sal(tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET);
    tbAction.Set_Can_help_num(dwCanHelpTimes);
    tbAction.Set_Helped_num(0);
    pstUser->m_aucSelfAlActionFlag[dwActionIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
    
    return 0;
}

TINT32 CProcessAlliance::ProcessCmd_AlAvatarChange( SSession *pstSession, TBOOL &bNeedResponse )
{
    if(pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos == EN_ALLIANCE_POS__REQUEST)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NOT_IN_ALLIANCE;
        return -1;
    }

    bNeedResponse = FALSE;

    SUserInfo *pstUser = &pstSession->m_stUserInfo;

    // 0. 请求参数
    TINT32 dwAvatar = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TINT32 dwType = atoi(pstSession->m_stReqParam.m_szKey[1]); //0 消耗道具, 1消耗宝石
    TINT32 dwGemNum = atoi(pstSession->m_stReqParam.m_szKey[2]);
    TINT32 dwItemId = atoi(pstSession->m_stReqParam.m_szKey[3]);

    // 1.0 检验权限
    if(pstUser->m_tbPlayer.m_nAlpos < EN_ALLIANCE_POS__VICE_CHANCELOR)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }
    // 1.1 检验资源是否足够
    if(dwType == 1)
    {
        if(FALSE == CPlayerBase::HasEnoughGem(&pstUser->m_tbLogin, dwGemNum))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GEM_LACK;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AlFlagChange: gem not enough [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
    }
    else if(dwType == 0)
    {
        if(FALSE == CItemBase::HasEnoughItem(&pstUser->m_tbBackpack, dwItemId))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ITEM_NOT_ENOUGH;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AlFlagChange: item not enough [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }
    }
    else
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AlFlagChange: req item and gem all empty [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
        return -3;
    }

    // 2.1 消耗资源
    if(dwType == 1)
    {
        pstSession->m_udwGemCost = dwGemNum;
        CPlayerBase::CostGem(pstUser, dwGemNum);
    }
    else if(dwType == 0)
    {
        CItemBase::CostItem(&pstSession->m_stUserInfo.m_tbBackpack, dwItemId);
    }

    pstUser->m_tbAlliance.Set_Avatar(dwAvatar);

    return 0;
}

TINT32 CProcessAlliance::ProcessCmd_AllianceNickChange(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;

    // 0. 请求参数
    TCHAR *pszAlliance = pstSession->m_stReqParam.m_szKey[0];
    TINT32 dwGemNum = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TINT32 dwItemId = atoi(pstSession->m_stReqParam.m_szKey[2]);

    // 1. 第一步操作
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        TCHAR szTmpName[MAX_TABLE_NAME_LEN];
        strcpy(szTmpName, pszAlliance);
        CUtf8Util::strtolower(szTmpName);

        if(!CToolBase::IsValidName(szTmpName, EN_ALLIANCE_NICK_NAME))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__OTHER_NAME_INVALID;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_ChangePlayerName: reserve name [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -7;
        }
        // 1.1 检验资源是否足够
        if(dwGemNum > 0)
        {
            if(FALSE == CPlayerBase::HasEnoughGem(&pstUser->m_tbLogin, dwGemNum))
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GEM_LACK;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_ChangePlayerName: gem not enough [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                return -1;
            }
        }
        else if(dwItemId >= 0)
        {
            if(FALSE == CItemBase::HasEnoughItem(&pstUser->m_tbBackpack, dwItemId))
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ITEM_NOT_ENOUGH;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_ChangePlayerName: item not enough [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                return -2;
            }
        }
        else
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_ChangePlayerName: req item and gem all empty [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }

        // update data first
        pstSession->ResetAwsInfo();

        TbUnique_name tbUniqueName;
        tbUniqueName.Set_Type(EN_ALLIANCE_NICK_NAME);
        tbUniqueName.Set_Name(CToolBase::ToLower(szTmpName));
        tbUniqueName.Set_Exist(1);
        tbUniqueName.Set_Id(pstUser->m_tbAlliance.m_nAid);
        ExpectedDesc expectedDesc;
        expectedDesc.clear();
        ExpectedItem expectedItem;
        expectedItem.SetVal(TbUNIQUE_NAME_FIELD_EXIST, FALSE);
        expectedDesc.push_back(expectedItem);

        CAwsRequest::UpdateItem(pstSession, &tbUniqueName, expectedDesc, RETURN_VALUES_ALL_NEW);

        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_ChangePlayerName: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -4;
        }

        return 0;
    }

    // 2. 处理查询响应
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        TbUnique_name tbUniqueName;
        tbUniqueName.Reset();
        int dwCnt = CAwsResponse::OnUpdateItemRsp(*pstSession->m_vecAwsRsp[0], &tbUniqueName);
        if(dwCnt <= 0 || tbUniqueName.m_nId != pstUser->m_tbAlliance.m_nAid)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ALLIANCE_NICK_BE_USED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_ChangePlayerName: name has been used [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -5;
        }

        pstSession->ResetAwsInfo();
        tbUniqueName.Reset();
        tbUniqueName.Set_Type(EN_ALLIANCE_NICK_NAME);
        tbUniqueName.Set_Name(CToolBase::ToLower(pstUser->m_tbAlliance.m_sAl_nick_name));
        CAwsRequest::DeleteItem(pstSession, &tbUniqueName);

        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_ChangePlayerName: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -6;
        }

        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        // 2.1 消耗资源
        if(dwGemNum > 0)
        {
            pstSession->m_udwGemCost = dwGemNum;
            CPlayerBase::CostGem(pstUser, dwGemNum);
        }
        else if(dwItemId > 0)
        {
            CItemBase::CostItem(&pstSession->m_stUserInfo.m_tbBackpack, dwItemId);
        }

        // 2.2. 设置用户名
        char szName[MAX_TABLE_NAME_LEN];
        snprintf(szName, sizeof(szName), "%s", pszAlliance);
        pstUser->m_tbAlliance.Set_Al_nick_name(szName);

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;
        CMsgBase::ChangeAlNick(&pstUser->m_tbAlliance);
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    return 0;
}

TINT32 CProcessAlliance::ProcessCmd_AllianceNameChange(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;

    // 0. 请求参数
    TCHAR *pszAlliance = pstSession->m_stReqParam.m_szKey[0];
    TINT32 dwGemNum = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TINT32 dwItemId = atoi(pstSession->m_stReqParam.m_szKey[2]);

    // 1. 第一步操作
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        TCHAR szTmpName[MAX_TABLE_NAME_LEN];
        strcpy(szTmpName, pszAlliance);

        if(!CToolBase::IsValidName(szTmpName, EN_ALLIANCE_NAME))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__OTHER_NAME_INVALID;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_ChangePlayerName: reserve name [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -7;
        }
        // 1.1 检验资源是否足够
        if(dwGemNum > 0)
        {
            if(FALSE == CPlayerBase::HasEnoughGem(&pstUser->m_tbLogin, dwGemNum))
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GEM_LACK;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_ChangePlayerName: gem not enough [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                return -1;
            }
        }
        else if(dwItemId >= 0)
        {
            if(FALSE == CItemBase::HasEnoughItem(&pstUser->m_tbBackpack, dwItemId))
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ITEM_NOT_ENOUGH;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_ChangePlayerName: item not enough [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                return -2;
            }
        }
        else
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_ChangePlayerName: req item and gem all empty [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }

        // update data first
        pstSession->ResetAwsInfo();

        TbUnique_name tbUniqueName;
        tbUniqueName.Set_Type(EN_ALLIANCE_NAME);
        tbUniqueName.Set_Name(CToolBase::ToLower(szTmpName));
        tbUniqueName.Set_Exist(1);
        tbUniqueName.Set_Id(pstUser->m_tbAlliance.m_nAid);
        ExpectedDesc expectedDesc;
        expectedDesc.clear();
        ExpectedItem expectedItem;
        expectedItem.SetVal(TbUNIQUE_NAME_FIELD_EXIST, FALSE);
        expectedDesc.push_back(expectedItem);

        CAwsRequest::UpdateItem(pstSession, &tbUniqueName, expectedDesc, RETURN_VALUES_ALL_NEW);

        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_ChangePlayerName: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -4;
        }

        return 0;
    }

    // 2. 处理查询响应
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        TbUnique_name tbUniqueName;
        tbUniqueName.Reset();
        int dwCnt = CAwsResponse::OnUpdateItemRsp(*pstSession->m_vecAwsRsp[0], &tbUniqueName);
        if(dwCnt <= 0 || tbUniqueName.m_nId != pstUser->m_tbAlliance.m_nAid)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ALLIANCE_NAME_BE_USED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_ChangePlayerName: name has been used [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -5;
        }

        pstSession->ResetAwsInfo();
        tbUniqueName.Reset();
        tbUniqueName.Set_Type(EN_ALLIANCE_NAME);
        tbUniqueName.Set_Name(CToolBase::ToLower(pstUser->m_tbAlliance.m_sName));
        CAwsRequest::DeleteItem(pstSession, &tbUniqueName);

        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_ChangePlayerName: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -6;
        }

        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        // 2.1 消耗资源
        if(dwGemNum > 0)
        {
            pstSession->m_udwGemCost = dwGemNum;
            CPlayerBase::CostGem(pstUser, dwGemNum);
        }
        else if(dwItemId > 0)
        {
            CItemBase::CostItem(&pstSession->m_stUserInfo.m_tbBackpack, dwItemId);
        }

        // 2.2. 设置用户名
        char szName[MAX_TABLE_NAME_LEN];
        snprintf(szName, sizeof(szName), "%s", pszAlliance);
        pstUser->m_tbAlliance.Set_Name(szName);

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;
        CMsgBase::ChangeAlName(&pstUser->m_tbAlliance);
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessAlliance::ProcessCmd_DubTitle(SSession* pstSession, TBOOL& bNeedResponse)
{
    //废弃
    /*
    TINT32 dwRetCode = 0;
    TbPlayer *ptbSelfPlayer = &pstSession->m_stUserInfo.m_tbPlayer;

    // 0. 获取输入参数
    TUINT32 udwTargetUserId = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TINT32 dwTitleId = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TINT32 dwType = atoi(pstSession->m_stReqParam.m_szKey[2]);

    // 1. check param
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        if(ptbSelfPlayer->m_nAlpos != EN_ALLIANCE_POS__CHANCELLOR)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -1;
        }

        // check param
        if(ptbSelfPlayer->m_nUid == udwTargetUserId)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_DubTitle: req param error [seq=%u]", \
                pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }

        if(pstSession->m_stUserInfo.m_tbAlliance.m_nThrone_status != EN_ALLIANCE_THRONE_STATUS_OCCUPY)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_DubTitle: player not occupy throne [seq=%u]", \
                pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }

        // get target user info 
        pstSession->ResetAwsInfo();
        CAwsRequest::UserGetByUid(pstSession, udwTargetUserId);

        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_DubTitle: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }

        return 0;
    }

    // 3. 处理响应
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        pstSession->m_tbTmpPlayer.Reset();
        dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[0], &pstSession->m_tbTmpPlayer);

        if(pstSession->m_tbTmpPlayer.m_nUid == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -4;
        }

        if(pstSession->m_tbTmpPlayer.m_nSid != pstSession->m_stUserInfo.m_tbAlliance.m_nSid)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PLAYER_CHANGE_SVR;
            return -4;
        }

        if(CCommonLogic::IsAlTitle(dwTitleId))
        {
            if(pstSession->m_tbTmpPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET != ptbSelfPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET
            || pstSession->m_tbTmpPlayer.m_nAlpos == EN_ALLIANCE_POS__REQUEST)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PLAYER_HAS_LEAVE_ALLIANCE;
                return -4;
            }
        }

        if(dwType == 1)
        {
            if(CCommonLogic::HasTitle(ptbSelfPlayer->m_nUid, pstSession->m_atbTmpPlayer->m_nUid, &pstSession->m_stTitleInfoList))
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ALREADY_HAS_TITLE;
                return -5;
            }

            CCommonLogic::AddTitle(ptbSelfPlayer, &pstSession->m_tbTmpPlayer, dwTitleId, &pstSession->m_stTitleInfoList);
            //broadcast
            string sReplace = pstSession->m_tbTmpPlayer.m_sUin + '#' + CCommonFunc::NumToString(dwTitleId);
            if (pstSession->m_stUserInfo.m_tbAlliance.m_nThrone_pos == THRONE_POS)
            {
                CSendMessageBase::SendBroadcast(&pstSession->m_stUserInfo, pstSession->m_tbTmpPlayer.m_nSid, 
                    pstSession->m_tbTmpPlayer.m_nUid, EN_BROADCAST_CONTENT_ID__THRONE_HONOR, sReplace);
            }
        }
        else
        {
            CCommonLogic::RemoveTitle(ptbSelfPlayer->m_nUid, pstSession->m_tbTmpPlayer.m_nUid, dwTitleId, &pstSession->m_stTitleInfoList);
        }

        CSendMessageBase::AddTips(&pstSession->m_stUserInfo, EN_TIPS_TYPE__KING_DUB, pstSession->m_stUserInfo.m_tbPlayer.m_nUid, FALSE,
            dwType, 0, 0, pstSession->m_tbTmpPlayer.m_sUin.c_str());

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    */
    return 0;
}

TINT32 CProcessAlliance::ProcessCmd_Invite(SSession* pstSession, TBOOL& bNeedResponse)
{
    TINT32 dwRetCode = 0;
    TbAlliance* ptbSelfAlliance = &pstSession->m_stUserInfo.m_tbAlliance;
    TUINT32 udwAllianceId = ptbSelfAlliance->m_nAid;
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();

    TCHAR *pszPlayerIdList = pstSession->m_stReqParam.m_szKey[0];
    TINT32 dwType = atoi(pstSession->m_stReqParam.m_szKey[1]); //1不会报错

    if(pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos == EN_ALLIANCE_POS__REQUEST)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NOT_IN_ALLIANCE;
        return -1;
    }

    if(pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos < EN_ALLIANCE_POS__VICE_CHANCELOR)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__HAVE_NO_PERMISSION;
        return -2;
    }

    TUINT32 audwTargetUid[MAX_RECOMMEND_NUM] = {0};
    TUINT32 udwCount = MAX_RECOMMEND_NUM;
    CCommonFunc::GetArrayFromString(pszPlayerIdList, ':', audwTargetUid, udwCount);
    if(udwCount == 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -3;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        pstSession->ResetAwsInfo();
        for(TUINT32 udwIdx = 0; udwIdx < udwCount; ++udwIdx)
        {
            CAwsRequest::UserGetByUid(pstSession, audwTargetUid[udwIdx]);
        }
        // send request
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_Invite: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -4;
        }
        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        pstSession->m_udwTmpPlayerNum = 0;
        for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecAwsRsp.size(); ++udwIdx)
        {
            AwsRspInfo* pstAwsRspInfo = pstSession->m_vecAwsRsp[udwIdx];
            string strTableRawName = CCommonFunc::GetTableRawName(pstAwsRspInfo->sTableName);
            if(strTableRawName == EN_AWS_TABLE_PLAYER)
            {
                dwRetCode = CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &pstSession->m_atbTmpPlayer[pstSession->m_udwTmpPlayerNum]);
                if(dwRetCode > 0)
                {
                    pstSession->m_udwTmpPlayerNum++;
                }
            }
        }

        if(pstSession->m_udwTmpPlayerNum != udwCount)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -5;
        }
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        
        for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_udwTmpPlayerNum; ++udwIdx)
        {
            if(dwType == 0
            && pstSession->m_atbTmpPlayer[udwIdx].m_nAlid > 0
            && pstSession->m_atbTmpPlayer[udwIdx].m_nAlpos > EN_ALLIANCE_POS__REQUEST)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__CAN_NOT_INVITE_TARGET;
                return -6;
            }

            if(pstSession->m_atbTmpPlayer[udwIdx].m_nSid != ptbSelfAlliance->m_nSid)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PLAYER_CHANGE_SVR_NORMAL;
                return -7;
            }

            // chat部分的邀请需要校验次数
            TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
            if(dwType == 0 && pstSession->m_atbTmpPlayer[udwIdx].m_nInvited_time/86400 == udwCurTime/86400
                && pstSession->m_atbTmpPlayer[udwIdx].m_nInvited_num >= 3)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PLAYER_HAS_REACH_INVITED_LIMIT;
                return -7;
            }
        }
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        pstSession->ResetAwsInfo();
        TbAl_invite_record tbOneRecord;
        for(TUINT32 udwIdx = 0; udwIdx < udwCount; ++udwIdx)
        {
            tbOneRecord.Reset();
            tbOneRecord.Set_Aid(udwAllianceId);
            tbOneRecord.Set_Uid(audwTargetUid[udwIdx]);
            CAwsRequest::GetItem(pstSession, &tbOneRecord, ETbALINVITERECORD_OPEN_TYPE_PRIMARY);
        }

        // send request
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__4;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_Invite: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -8;
        }
        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__4)
    {
        TbAl_invite_record tbOneRecord;
        for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecAwsRsp.size(); ++udwIdx)
        {
            tbOneRecord.Reset();
            AwsRspInfo* pstAwsRspInfo = pstSession->m_vecAwsRsp[udwIdx];
            string strTableRawName = CCommonFunc::GetTableRawName(pstAwsRspInfo->sTableName);
            if(strTableRawName == EN_AWS_TABLE_AL_INVITE_RECORD)
            {
                CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &tbOneRecord);
                if(tbOneRecord.m_nUid == 0)
                {
                    continue;
                }
                if(dwType == 0)
                {
                    for(TUINT32 udwPlayerIndex = 0; udwPlayerIndex < udwCount; ++udwPlayerIndex)
                    {
                        if(tbOneRecord.m_nUid == audwTargetUid[udwPlayerIndex]
                        && tbOneRecord.m_nTime + EN_INTIVE_LIMIT_TIME > udwCurTime)
                        {
                            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__HAS_INVITED_IN_LIMIT_TIME;
                            return -9;
                        }
                    }
                }
            }
        }

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__5;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__5)
    {
        pstSession->ResetAwsInfo();
        TbAl_invite_record tbOneRecord;
        for(TUINT32 udwIdx = 0; udwIdx < udwCount; ++udwIdx)
        {
            tbOneRecord.Reset();
            tbOneRecord.Set_Aid(udwAllianceId);
            tbOneRecord.Set_Uid(audwTargetUid[udwIdx]);
            tbOneRecord.Set_Time(udwCurTime);
            CAwsRequest::UpdateItem(pstSession, &tbOneRecord);
        }

        // update player info
        for(TUINT32 idx = 0; idx < pstSession->m_udwTmpPlayerNum; idx++)
        {
            TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
            if(pstSession->m_atbTmpPlayer[idx].m_nInvited_time/86400 == udwCurTime/86400)
            {
                pstSession->m_atbTmpPlayer[idx].Set_Invited_time(udwCurTime);
                pstSession->m_atbTmpPlayer[idx].Set_Invited_num(pstSession->m_atbTmpPlayer[idx].m_nInvited_num+1);
            }
            else
            {
                pstSession->m_atbTmpPlayer[idx].Set_Invited_time(udwCurTime);
                pstSession->m_atbTmpPlayer[idx].Set_Invited_num(1);
            }
            CAwsRequest::UpdateItem(pstSession, &pstSession->m_atbTmpPlayer[idx]);
        }

        // send request
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__6;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_Invite: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -10;
        }
        return 0;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__6)
    {
        AllianceRank stAlRank;

        TCHAR szResponseData[1024];
        TCHAR szUrl[1024];
        memset(szResponseData, 0, sizeof(szResponseData));
        memset(szUrl, 0, sizeof(szUrl));
        sprintf(szUrl, "%s?request=op_en_flag=0&did=system&sid=0&uid=0&aid=0&command=get_al_rank_by_id&key0=%u&key1=%ld",
            CGlobalServ::m_poConf->m_szRankUrlPre,
            pstSession->m_stReqParam.m_udwSvrId,
            pstSession->m_stUserInfo.m_tbAlliance.m_nAid);

        CURLcode res = CToolBase::ResFromUrl(szUrl, szResponseData);
        if(CURLE_OK != res)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("get al rank failed. [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
        }
        for(TUINT32 udwIdx = 0; udwIdx < strlen(szResponseData); ++udwIdx)
        {
            if(szResponseData[udwIdx] == '\n')
            {
                szResponseData[udwIdx] = ' ';
            }
        }
        Json::Reader reader;
        pstSession->m_JsonValue.clear();
        if(TRUE == reader.parse(szResponseData, pstSession->m_JsonValue))
        {
            if(pstSession->m_JsonValue.isMember("res_data")
            && pstSession->m_JsonValue["res_data"].isArray()
            && pstSession->m_JsonValue["res_data"][0u].isMember("data")
            && pstSession->m_JsonValue["res_data"][0u]["data"][0u].isMember("data"))
            {
                stAlRank.audwRank[2] = pstSession->m_JsonValue["res_data"][0u]["data"][0u]["data"]["force_rank"].asUInt64();
                stAlRank.audwRank[1] = pstSession->m_JsonValue["res_data"][0u]["data"][0u]["data"]["troop_kill_rank"].asUInt64();
            }
        }

        TbAlliance& tbMyAlliance = pstSession->m_stUserInfo.m_tbAlliance;
        string strMyName = pstSession->m_stUserInfo.m_tbPlayer.m_sUin;
        string strMyAlInfo = "";
        Json::Value jMyAlInfo;
        Json::FastWriter tmpWriter;

        CCommJson::GenAllianceInfo(&stAlRank, &tbMyAlliance, jMyAlInfo);
        strMyAlInfo = tmpWriter.write(jMyAlInfo);

        SNoticInfo stNoticInfo;
        stNoticInfo.Reset();
        stNoticInfo.m_dwNoticId = EN_NOTI_ID__AL_INVITE;
        stNoticInfo.m_strSName = ptbSelfAlliance->m_sName;
        stNoticInfo.m_strLang = CDocument::GetLang(pstSession->m_stUserInfo.m_tbLogin.m_nLang);;

        for(TUINT32 udwIdx = 0; udwIdx < udwCount; ++udwIdx)
        {
            //v1.2@wave:兼容性屏蔽————联盟邀请邮件
            CMsgBase::SendOperateMail(audwTargetUid[udwIdx],
                EN_MAIL_ID__INVITE,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_INVITE,
                strMyName.c_str(),
                strMyAlInfo.c_str(),
                "");

            CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), ptbSelfAlliance->m_nSid, audwTargetUid[udwIdx], stNoticInfo);
       }

        if(dwType == 1)
        {
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__INIT;
            pstSession->m_udwCommand = EN_CLIENT_REQ_COMMAND__PLAYER_RECOMMAND_GET;
            pstSession->m_bGotoOtherCmd = TRUE;
            return 0;
        }
        else
        {
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
            return 0;
        }
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessAlliance::ProcessCmd_PlayerRecommendGet(SSession* pstSession, TBOOL& bNeedResponse)
{
    TINT32 dwRetCode = 0;
    if(pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos == EN_ALLIANCE_POS__REQUEST)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NOT_IN_ALLIANCE;
        return -1;
    }

    if(pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos < EN_ALLIANCE_POS__VICE_CHANCELOR)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__HAVE_NO_PERMISSION;
        return -2;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        pstSession->ResetDbInfo();

        CDbRequest::SelectRecommendTime(pstSession);
        CDbRequest::SelectRecommendPlayer(pstSession, pstSession->m_stUserInfo.m_tbAlliance.m_nAid);

        // send request
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__DB;
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendDbRequest(pstSession, EN_SERVICE_TYPE_QUERY_MYSQL_REQ);
        if(dwRetCode < 0)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_PlayerRecommendGet: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }
        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        pstSession->m_stRecommendTime.Reset();
        pstSession->m_udwRecommendNum = 0;
        for(unsigned int i = 0; i < pstSession->m_vecDbRsp.size(); ++i)
        {
            DbRspInfo& rspInfo = *pstSession->m_vecDbRsp[i];
            if(rspInfo.sTableName == "compute_time")
            {
                dwRetCode = CDbResponse::OnSelectResponse(rspInfo, &pstSession->m_stRecommendTime, 1);
                if(dwRetCode < 0)
                {
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("OnSelectResponse failed [seq=%u]", pstSession->m_udwSeqNo));
                    return -4;
                }
                continue;
            }
            if(rspInfo.sTableName == "player_recommend")
            {
                dwRetCode = CDbResponse::OnSelectResponse(rspInfo, pstSession->m_aRecommendPlayer, MAX_RECOMMEND_NUM);
                if(dwRetCode < 0)
                {
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("OnSelectResponse failed [seq=%u]", pstSession->m_udwSeqNo));
                    return -5;
                }
                pstSession->m_udwRecommendNum = dwRetCode;
                continue;
            }
        }

        pstSession->m_stUserInfo.m_tbUserStat.Set_Player_recommend_time(CTimeUtils::GetUnixTime());
        pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_PLAYER_RECOMMEND;
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    pstSession->m_stUserInfo.m_tbUserStat.Set_Player_recommend_time(CTimeUtils::GetUnixTime());
    pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_PLAYER_RECOMMEND;
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}


TINT32 CProcessAlliance::ProcessCmd_AlHivePosShow(SSession *pstSession, TBOOL &bNeedResponse)
{
    TUINT32 udwAlHivePosShowFlag = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);    

    if(EN_ALLIANCE_POS__REQUEST == pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NOT_IN_ALLIANCE;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AlHivePosShow: player not in alliance [seq=%u]", \
                                                pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }

    if(EN_ALLIANCE_POS__CHANCELLOR != pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos
       && EN_ALLIANCE_POS__VICE_CHANCELOR != pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AlHivePosShow: hive pos show set must alliance manager [seq=%u]", \
                                                pstSession->m_stUserInfo.m_udwBSeqNo));
        return -2;   
    }
    
    if(0 != udwAlHivePosShowFlag && 1 != udwAlHivePosShowFlag)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AlHivePosShow: hive pos show set error [udwAlHivePosShowFlag=%u] [seq=%u]", \
                                                udwAlHivePosShowFlag, \
                                                pstSession->m_stUserInfo.m_udwBSeqNo));
        return -2;   
    }
    
    TbAlliance *ptbAlliance = &pstSession->m_stUserInfo.m_tbAlliance;
    ptbAlliance->Set_Hive_show_flag(udwAlHivePosShowFlag);
    pstSession->m_stUserInfo.m_ucAllianceFlag = EN_TABLE_UPDT_FLAG__CHANGE;

    return 0;

}

TINT32 CProcessAlliance::ProcessCmd_AlSetHivePos(SSession *pstSession, TBOOL &bNeedResponse)
{
    TUINT32 udwAlHiveSvr = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TUINT32 udwAlHivePos = strtoul(pstSession->m_stReqParam.m_szKey[1], NULL, 10);


    if(EN_ALLIANCE_POS__REQUEST == pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NOT_IN_ALLIANCE;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AlHivePosShow: player not in alliance [seq=%u]", \
                                                pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }
    
    if(EN_ALLIANCE_POS__CHANCELLOR != pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos
       && EN_ALLIANCE_POS__VICE_CHANCELOR != pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AlHivePosShow: hive pos show set must alliance manager [seq=%u]", \
                                                pstSession->m_stUserInfo.m_udwBSeqNo));
        return -2;
        
    }
    
    TbAlliance *ptbAlliance = &pstSession->m_stUserInfo.m_tbAlliance;    
    ptbAlliance->Set_Hive_svr(udwAlHiveSvr);
    ptbAlliance->Set_Hive_pos(udwAlHivePos);
    pstSession->m_stUserInfo.m_ucAllianceFlag = EN_TABLE_UPDT_FLAG__CHANGE;

    return 0;
}

TINT32 CProcessAlliance::ProcessCmd_InvitedJoin(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    TbPlayer *ptbSelfPlayer = &pstSession->m_stUserInfo.m_tbPlayer;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbAlliance* ptbTargetAlliance = &pstSession->m_tbTmpAlliance;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;

    // 0. 获取请求参数
    TUINT32 udwAllianceId = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        if(ptbSelfPlayer->m_nAlid > 0 && ptbSelfPlayer->m_nAlpos == EN_ALLIANCE_POS__CHANCELLOR)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__CHANCELLOR_CAN_NOT_ACCEPT_INVITE;
            return -1;
        }

        pstSession->ResetAwsInfo();
        CAwsRequest::AllianceGetByAid(pstSession, udwAllianceId);
        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_InvitedJoin: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }
        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        ptbTargetAlliance->Reset();
        CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[0], ptbTargetAlliance);
        if(ptbTargetAlliance->m_nAid == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ALLIANCE_NOT_EXIST;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_InvitedJoin: get target alliance failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }

        if(ptbTargetAlliance->m_nSid != ptbSelfPlayer->m_nSid)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ALLIANCE_NOT_EXIST;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_InvitedJoin: get target alliance diff sid [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -4;
        }

        if(ptbTargetAlliance->m_nMember >= MAX_ALLIANCE_MEMBER_NUM)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ALLIANCE_MEMBER_OVER_LOAD;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_InvitedJoin: alliance[%lld] is full [seq=%u]",
                ptbTargetAlliance->m_nAid, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -5;
        }

        pstSession->ResetAwsInfo();
        if(ptbSelfPlayer->m_nAlpos > EN_ALLIANCE_POS__REQUEST
            && ptbSelfPlayer->m_nAlid > 0)
        {
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        }
        else
        {
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;
        }
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        TbAlliance *ptbAlliance = &pstSession->m_stUserInfo.m_tbAlliance;
        // update alliance info
        if(ptbAlliance->m_nMember > 0)
        {
            ptbAlliance->Set_Member(ptbAlliance->m_nMember - 1);
        }
        if(ptbAlliance->m_nMight > ptbSelfPlayer->m_nMight)
        {
            ptbAlliance->Set_Might(ptbAlliance->m_nMight - ptbSelfPlayer->m_nMight);
        }
        else
        {
            ptbAlliance->Set_Might(0);
        }
        if(ptbAlliance->m_nForce_kill > ptbSelfPlayer->m_nMkill)
        {
            ptbAlliance->Set_Force_kill(ptbAlliance->m_nForce_kill - ptbSelfPlayer->m_nMkill);
        }
        else
        {
            ptbAlliance->Set_Force_kill(0);
        }

        // updt action
        PlayerLeaveAllianceUpdtAction(pstSession, ptbSelfPlayer->m_nUid);

        // clear player assist
        for(TUINT32 dwIdx = 0; dwIdx < pstUser->m_udwAlAssistAllNum; dwIdx++)
        {
            if(pstUser->m_atbAlAssistAll[dwIdx].m_nUid == pstSession->m_stUserInfo.m_tbPlayer.m_nUid)
            {
                pstUser->m_aucAlAssistAllFlag[dwIdx] = EN_TABLE_UPDT_FLAG__DEL;
            }
        }

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        pstSession->m_dwOldRequestAid = ptbSelfPlayer->m_nReq_al >> 32;
        // set data
        TbPlayer tbPlayerItem = *ptbSelfPlayer;

        tbPlayerItem.Set_Alid(CCommonBase::GetPlayerAllianceId(udwAllianceId, EN_ALLIANCE_POS__MEMBER));
        tbPlayerItem.Set_Alpos(EN_ALLIANCE_POS__MEMBER);
        tbPlayerItem.Set_Alname(ptbTargetAlliance->m_sName);
        tbPlayerItem.Set_Al_nick_name(ptbTargetAlliance->m_sAl_nick_name);
        tbPlayerItem.Set_Al_time(CTimeUtils::GetUnixTime());
        tbPlayerItem.Set_Invite_mail_time(CTimeUtils::GetUnixTime());
        tbPlayerItem.Set_Req_al(0);
        tbPlayerItem.Set_Alname_update_time(CTimeUtils::GetUnixTime());

        CAwsRequest::UpdateItem(pstSession, &tbPlayerItem, ExpectedDesc(), RETURN_VALUES_ALL_NEW);

        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__4;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AllianceRequestJoin: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -6;
        }
        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__4)
    {
        TbPlayer tbPlayerItem;
        tbPlayerItem.Reset();
        dwRetCode = CAwsResponse::OnUpdateItemRsp(*pstSession->m_vecAwsRsp[0], &tbPlayerItem);
        if(dwRetCode <= 0 || tbPlayerItem.m_nUid == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AllianceRequestJoin: respon fail [ret=%d uid=%u][seq=%u]", dwRetCode, tbPlayerItem.m_nUid, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -7;
        }

        pstSession->ResetAwsInfo();

        if(pstSession->m_stUserInfo.m_tbAlliance.m_nAid > 0)
        {
            TbAl_member tbOldAlmember;
            tbOldAlmember.Set_Aid(pstSession->m_stUserInfo.m_tbAlliance.m_nAid);
            tbOldAlmember.Set_Uid(ptbSelfPlayer->m_nUid);
            CAwsRequest::DeleteItem(pstSession, &tbOldAlmember);
        }

        TbAl_member tbAlmember;
        if(pstSession->m_dwOldRequestAid > 0)
        {
            tbAlmember.Set_Aid(-1 * pstSession->m_dwOldRequestAid);
            tbAlmember.Set_Uid(ptbSelfPlayer->m_nUid);
            CAwsRequest::DeleteItem(pstSession, &tbAlmember);
        }
        *ptbSelfPlayer = tbPlayerItem;

        tbAlmember.Set_Aid(ptbTargetAlliance->m_nAid);
        tbAlmember.Set_Uid(ptbSelfPlayer->m_nUid);
        tbAlmember.Set_Al_pos(EN_ALLIANCE_POS__MEMBER);
        tbAlmember.Set_Req_time(0);
        CCommJson::GenAlMemberInfo(ptbSelfPlayer, tbAlmember.m_jProfile);
        tbAlmember.SetFlag(TbAL_MEMBER_FIELD_PROFILE);

        CAwsRequest::UpdateItem(pstSession, &tbAlmember);

        // updt alliance info
        ptbTargetAlliance->Set_Member(ptbTargetAlliance->m_nMember + 1);
        ptbTargetAlliance->Set_Might(ptbTargetAlliance->m_nMight + ptbSelfPlayer->m_nMight);
        ptbTargetAlliance->Set_Force_kill(ptbTargetAlliance->m_nForce_kill + ptbSelfPlayer->m_nMkill);
        //pstUser->m_tbAlliance = *ptbTargetAlliance;
        CAwsRequest::UpdateItem(pstSession, ptbTargetAlliance);

        pstUser->m_tbUserStat.Set_Wall_get_t(CTimeUtils::GetUnixTime());

        for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwWildNum; ++udwIdx)
        {
            pstUser->m_atbWild[udwIdx].Set_Alid(ptbTargetAlliance->m_nAid);
            pstUser->m_atbWild[udwIdx].Set_Alname(ptbTargetAlliance->m_sName);
            pstUser->m_atbWild[udwIdx].Set_Al_nick(ptbTargetAlliance->m_sAl_nick_name);
            pstUser->m_atbWild[udwIdx].Set_Al_flag(ptbTargetAlliance->m_nAvatar);
            pstUser->m_atbWild[udwIdx].Set_Name_update_time(CTimeUtils::GetUnixTime());
        }

        //task count
        CQuestLogic::SetTaskCurValue(&pstSession->m_stUserInfo,pstCity, EN_TASK_TYPE_JOIN_ALLIANCE);

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__5;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AllianceRequestJoin: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -8;
        }
        return 0;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__5)
    {
        if(pstSession->m_stUserInfo.m_tbLogin.m_nAl_time == 0)
        {
            ptbSelfPlayer->Set_Status(ptbSelfPlayer->m_nStatus | EN_CITY_STATUS__FIRST_ALLIANCE_REWARD);
            TUINT32 udwGemReward = CGameInfo::GetInstance()->m_oJsonRoot["game_basic"][EN_GAME_BASIC_FIRST_JOIN_ALLIANCE_GEM_REWARD].asUInt();
            CPlayerBase::AddGem(&pstSession->m_stUserInfo.m_tbLogin, udwGemReward);
            TSE_LOG_INFO(CGameInfo::GetInstance()->m_poLog, ("CProcessAlCreate: uid[%ld]first join alliance, reward gem[%u][seq=%u]", ptbSelfPlayer->m_nUid, udwGemReward, pstSession->m_udwSeqNo));
        }
        else// 发送邮件
        {
            CMsgBase::SendOperateMail(ptbSelfPlayer->m_nUid, EN_MAIL_ID__JOINED_AN_ALLIANCE, ptbSelfPlayer->m_nSid,
                SYSTEM_ENCOURAGE, ptbTargetAlliance->m_sName.c_str(), "", "");
        }

        pstUser->m_tbUserStat.Set_Al_event_tips_time(CTimeUtils::GetUnixTime());

        CSendMessageBase::AddTips(&pstSession->m_stUserInfo, EN_TIPS_TYPE__JOINED_ALLIANCE, ptbSelfPlayer->m_nUid,
            FALSE, ptbTargetAlliance->m_nAid, 0, 0, ptbTargetAlliance->m_sName.c_str());

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__6;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__6)
    {
        ostringstream ossCustomizeParam;
        ossCustomizeParam.str("");
        ossCustomizeParam << EN_CHAT_SYS_MSG_JOIN_TO_ALLIANCE << "#" << ptbSelfPlayer->m_nUid << "#" << ptbSelfPlayer->m_sUin;
        CSendMessageBase::SendSysMsgToChat(pstSession->m_stReqParam.m_udwSvrId, ptbTargetAlliance->m_nAid, 1, ossCustomizeParam.str().c_str());

        if(pstSession->m_stUserInfo.m_tbAlliance.m_nAid > 0)
        {
            ostringstream ossCustomizeParam;
            ossCustomizeParam.str("");
            ossCustomizeParam << EN_CHAT_SYS_MSG_LEAVE_FROM_ALLIANCE << "#" << ptbSelfPlayer->m_nUid << "#" << ptbSelfPlayer->m_sUin;
            CSendMessageBase::SendSysMsgToChat(pstSession->m_stReqParam.m_udwSvrId, pstSession->m_stUserInfo.m_tbAlliance.m_nAid, 1, ossCustomizeParam.str().c_str());            
        }

    
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessAlliance::ProcessDiplomacyIfHeIsNormal(TbDiplomacy* ptbMyDiplomacy, TbDiplomacy* ptbHisDiplomacy, TUINT8 ucTargetDiplomacy, TbAlliance& tbMyAlliance, TbAlliance& tbHisAlliance, TUINT8& ucMyDipFlag, TUINT8& ucHisDipFlag, SSession *pstSession)
{
    if(ptbMyDiplomacy->m_nType != EN_DIPLOMACY_TYPE__NORMAL)
    {
        return -1;
    }
    string strMyAlName = "";
    string strHisAlName = "";
    Json::Value jMyAlInfo;
    Json::Value jHisAlInfo;
    Json::FastWriter tmpWriter;
    string strMyAlInfo = "";
    string strHisAlInfo = "";

    strMyAlName = "(" + tbMyAlliance.m_sAl_nick_name + ")" + tbMyAlliance.m_sName;
    strHisAlName = "(" + tbHisAlliance.m_sAl_nick_name + ")" + tbHisAlliance.m_sName;

    AllianceRank stAlRank;

    TCHAR szResponseData[1024];
    TCHAR szUrl[1024];
    memset(szResponseData, 0, sizeof(szResponseData));
    memset(szUrl, 0, sizeof(szUrl));
    sprintf(szUrl, "%s?request=op_en_flag=0&did=system&sid=0&uid=0&aid=0&command=get_al_rank_by_id&key0=%ld&key1=%ld",
        CGlobalServ::m_poConf->m_szRankUrlPre,
        tbMyAlliance.m_nSid,
        tbMyAlliance.m_nAid);

    CURLcode res = CToolBase::ResFromUrl(szUrl, szResponseData);
    if (CURLE_OK != res)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("get al rank failed. [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
    }
    for (TUINT32 udwIdx = 0; udwIdx < strlen(szResponseData); ++udwIdx)
    {
        if (szResponseData[udwIdx] == '\n')
        {
            szResponseData[udwIdx] = ' ';
        }
    }
    Json::Reader reader;
    Json::Value jRes;
    if (TRUE == reader.parse(szResponseData, jRes))
    {
        if (jRes.isMember("res_data")
            && jRes["res_data"].isArray()
            && jRes["res_data"][0u].isMember("data")
            && jRes["res_data"][0u]["data"][0u].isMember("data"))
        {
            stAlRank.audwRank[2] = jRes["res_data"][0u]["data"][0u]["data"]["force_rank"].asUInt64();
            stAlRank.audwRank[1] = jRes["res_data"][0u]["data"][0u]["data"]["troop_kill_rank"].asUInt64();
        }
    }

    CCommJson::GenAllianceInfo(&stAlRank, &tbMyAlliance, jMyAlInfo);
    strMyAlInfo = tmpWriter.write(jMyAlInfo);
    CCommJson::GenAllianceInfo(&tbHisAlliance, jHisAlInfo);
    strHisAlInfo = tmpWriter.write(jHisAlInfo);

    //ucMyDipFlag除非是delete，否则不需要更改
    if(ptbHisDiplomacy->m_nType == EN_DIPLOMACY_TYPE__NORMAL)
    {
        if(ucTargetDiplomacy == EN_DIPLOMACY_TYPE__FRIENDLY)
        {
            CDiplomacyProcess::TbDiplomacy_SetKey(ptbMyDiplomacy, tbMyAlliance.m_nAid, tbHisAlliance.m_nAid, EN_DIPLOMACY_TYPE__PEDDING);
            CMsgBase::SendOperateMail(tbHisAlliance.m_nOid,
                EN_MAIL_ID__BE_FRIEND,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strMyAlName.c_str(),
                strMyAlInfo.c_str(),
                "");
        }
        else if(ucTargetDiplomacy == EN_DIPLOMACY_TYPE__HOSTILE)
        {
            CDiplomacyProcess::TbDiplomacy_SetKey(ptbMyDiplomacy, tbMyAlliance.m_nAid, tbHisAlliance.m_nAid, EN_DIPLOMACY_TYPE__HOSTILE);
            CMsgBase::SendOperateAlMail(tbMyAlliance.m_nAid,
                EN_MAIL_ID__CONFIRM_HOSTILE,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strHisAlName.c_str(),
                "",
                "");
            CMsgBase::SendOperateMail(tbHisAlliance.m_nOid,
                EN_MAIL_ID__BE_HOSTILE,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strMyAlName.c_str(),
                "",
                "");
        }
    }
    else if(ptbHisDiplomacy->m_nType == EN_DIPLOMACY_TYPE__FRIENDLY)  //此情况理应不存在，容错
    {
        if(ucTargetDiplomacy == EN_DIPLOMACY_TYPE__FRIENDLY)
        {
            CDiplomacyProcess::TbDiplomacy_SetKey(ptbMyDiplomacy, tbMyAlliance.m_nAid, tbHisAlliance.m_nAid, EN_DIPLOMACY_TYPE__FRIENDLY);
            CMsgBase::SendOperateAlMail(tbMyAlliance.m_nAid,
                EN_MAIL_ID__CONFIRM_FRIEND,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strHisAlName.c_str(),
                "",
                "");
        }
        else if(ucTargetDiplomacy == EN_DIPLOMACY_TYPE__HOSTILE)
        {
            CDiplomacyProcess::TbDiplomacy_SetKey(ptbMyDiplomacy, tbMyAlliance.m_nAid, tbHisAlliance.m_nAid, EN_DIPLOMACY_TYPE__HOSTILE);
            CMsgBase::SendOperateAlMail(tbMyAlliance.m_nAid,
                EN_MAIL_ID__CONFIRM_HOSTILE,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strHisAlName.c_str(),
                "",
                "");
            CDiplomacyProcess::TbDiplomacy_SetKey(ptbHisDiplomacy, tbHisAlliance.m_nAid, tbMyAlliance.m_nAid, EN_DIPLOMACY_TYPE__NORMAL);
            ucHisDipFlag = EN_TABLE_UPDT_FLAG__DEL;
            CMsgBase::SendOperateMail(tbHisAlliance.m_nOid,
                EN_MAIL_ID__BE_HOSTILE,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strMyAlName.c_str(),
                "",
                "");
        }
    }
    else if(ptbHisDiplomacy->m_nType == EN_DIPLOMACY_TYPE__HOSTILE)
    {
        if(ucTargetDiplomacy == EN_DIPLOMACY_TYPE__FRIENDLY)
        {
            CDiplomacyProcess::TbDiplomacy_SetKey(ptbMyDiplomacy, tbMyAlliance.m_nAid, tbHisAlliance.m_nAid, EN_DIPLOMACY_TYPE__PEDDING);
            CMsgBase::SendOperateMail(tbHisAlliance.m_nOid,
                EN_MAIL_ID__BE_FRIEND,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strMyAlName.c_str(),
                strMyAlInfo.c_str(),
                "");
        }
        else if(ucTargetDiplomacy == EN_DIPLOMACY_TYPE__HOSTILE)
        {
            CDiplomacyProcess::TbDiplomacy_SetKey(ptbMyDiplomacy, tbMyAlliance.m_nAid, tbHisAlliance.m_nAid, EN_DIPLOMACY_TYPE__HOSTILE);
            CMsgBase::SendOperateAlMail(tbMyAlliance.m_nAid,
                EN_MAIL_ID__CONFIRM_HOSTILE,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strHisAlName.c_str(),
                "",
                "");
            CMsgBase::SendOperateMail(tbHisAlliance.m_nOid,
                EN_MAIL_ID__BE_HOSTILE,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strMyAlName.c_str(),
                "",
                "");
        }
    }
    else if(ptbHisDiplomacy->m_nType == EN_DIPLOMACY_TYPE__PEDDING)
    {
        if(ucTargetDiplomacy == EN_DIPLOMACY_TYPE__FRIENDLY)
        {
            CDiplomacyProcess::TbDiplomacy_SetKey(ptbMyDiplomacy, tbMyAlliance.m_nAid, tbHisAlliance.m_nAid, EN_DIPLOMACY_TYPE__FRIENDLY);
            CMsgBase::SendOperateAlMail(tbMyAlliance.m_nAid,
                EN_MAIL_ID__CONFIRM_FRIEND,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strHisAlName.c_str(),
                "",
                "");

            CDiplomacyProcess::TbDiplomacy_SetKey(ptbHisDiplomacy, tbHisAlliance.m_nAid, tbMyAlliance.m_nAid, EN_DIPLOMACY_TYPE__FRIENDLY);
            ucHisDipFlag = EN_TABLE_UPDT_FLAG__CHANGE;
            CMsgBase::SendOperateAlMail(tbHisAlliance.m_nAid,
                EN_MAIL_ID__CONFIRM_FRIEND,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strMyAlName.c_str(),
                "",
                "");
        }
        else if(ucTargetDiplomacy == EN_DIPLOMACY_TYPE__HOSTILE)
        {
            CDiplomacyProcess::TbDiplomacy_SetKey(ptbMyDiplomacy, tbMyAlliance.m_nAid, tbHisAlliance.m_nAid, EN_DIPLOMACY_TYPE__HOSTILE);
            CMsgBase::SendOperateAlMail(tbMyAlliance.m_nAid,
                EN_MAIL_ID__CONFIRM_HOSTILE,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strHisAlName.c_str(),
                "",
                "");
            CMsgBase::SendOperateMail(tbHisAlliance.m_nOid,
                EN_MAIL_ID__BE_HOSTILE,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strMyAlName.c_str(),
                "",
                "");
        }
    }
    else
    {
        return -3;
    }

    return 0;
}

TINT32 CProcessAlliance::ProcessDiplomacyIfHeIsFriendly(TbDiplomacy* ptbMyDiplomacy, TbDiplomacy* ptbHisDiplomacy, TUINT8 ucTargetDiplomacy, TbAlliance& tbMyAlliance, TbAlliance& tbHisAlliance, TUINT8& ucMyDipFlag, TUINT8& ucHisDipFlag, SSession *pstSession)
{
    if(ptbMyDiplomacy->m_nType != EN_DIPLOMACY_TYPE__FRIENDLY)
    {
        return -1;
    }
    string strMyAlName = "";
    string strHisAlName = "";
    Json::Value jMyAlInfo;
    Json::Value jHisAlInfo;
    Json::FastWriter tmpWriter;
    string strMyAlInfo = "";
    string strHisAlInfo = "";

    strMyAlName = "(" + tbMyAlliance.m_sAl_nick_name + ")" + tbMyAlliance.m_sName;
    strHisAlName = "(" + tbHisAlliance.m_sAl_nick_name + ")" + tbHisAlliance.m_sName;
    CCommJson::GenAllianceInfo(&tbMyAlliance, jMyAlInfo);
    strMyAlInfo = tmpWriter.write(jMyAlInfo);
    CCommJson::GenAllianceInfo(&tbHisAlliance, jHisAlInfo);
    strHisAlInfo = tmpWriter.write(jHisAlInfo);

    //ucMyDipFlag除非是delete，否则不需要更改
    if(ptbHisDiplomacy->m_nType == EN_DIPLOMACY_TYPE__NORMAL)  //容错
    {
        if(ucTargetDiplomacy == EN_DIPLOMACY_TYPE__NORMAL)
        {
            ucMyDipFlag = EN_TABLE_UPDT_FLAG__DEL;
            pstSession->m_stUserInfo.m_udwDiplomacyDelNum++;
            CMsgBase::SendOperateAlMail(tbMyAlliance.m_nAid,
                EN_MAIL_ID__CONFIRM_NORMAL,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strHisAlName.c_str(),
                "",
                "");
        }
        else if(ucTargetDiplomacy == EN_DIPLOMACY_TYPE__HOSTILE)
        {
            CDiplomacyProcess::TbDiplomacy_SetKey(ptbMyDiplomacy, tbMyAlliance.m_nAid, tbHisAlliance.m_nAid, EN_DIPLOMACY_TYPE__HOSTILE);
            CMsgBase::SendOperateAlMail(tbMyAlliance.m_nAid,
                EN_MAIL_ID__CONFIRM_HOSTILE,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strHisAlName.c_str(),
                "",
                "");
            CMsgBase::SendOperateMail(tbHisAlliance.m_nOid,
                EN_MAIL_ID__BE_HOSTILE,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strMyAlName.c_str(),
                "",
                "");
        }
    }
    else if(ptbHisDiplomacy->m_nType == EN_DIPLOMACY_TYPE__FRIENDLY)
    {
        if(ucTargetDiplomacy == EN_DIPLOMACY_TYPE__NORMAL)
        {
            ucMyDipFlag = EN_TABLE_UPDT_FLAG__DEL;
            pstSession->m_stUserInfo.m_udwDiplomacyDelNum++;
            CMsgBase::SendOperateAlMail(tbMyAlliance.m_nAid,
                EN_MAIL_ID__CONFIRM_NORMAL,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strHisAlName.c_str(),
                "",
                "");

            CDiplomacyProcess::TbDiplomacy_SetKey(ptbHisDiplomacy, tbHisAlliance.m_nAid, tbMyAlliance.m_nAid, EN_DIPLOMACY_TYPE__NORMAL);
            ucHisDipFlag = EN_TABLE_UPDT_FLAG__DEL;
            CMsgBase::SendOperateAlMail(tbHisAlliance.m_nAid,
                EN_MAIL_ID__CONFIRM_NORMAL,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strMyAlName.c_str(),
                "",
                "");
        }
        else if(ucTargetDiplomacy == EN_DIPLOMACY_TYPE__HOSTILE)
        {
            CDiplomacyProcess::TbDiplomacy_SetKey(ptbMyDiplomacy, tbMyAlliance.m_nAid, tbHisAlliance.m_nAid, EN_DIPLOMACY_TYPE__HOSTILE);
            CMsgBase::SendOperateAlMail(tbMyAlliance.m_nAid,
                EN_MAIL_ID__CONFIRM_HOSTILE,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strHisAlName.c_str(),
                "",
                "");
            CDiplomacyProcess::TbDiplomacy_SetKey(ptbHisDiplomacy, tbHisAlliance.m_nAid, tbMyAlliance.m_nAid, EN_DIPLOMACY_TYPE__PEDDING);
            ucHisDipFlag = EN_TABLE_UPDT_FLAG__CHANGE;
            CMsgBase::SendOperateAlMail(tbHisAlliance.m_nAid,
                EN_MAIL_ID__CONFIRM_NORMAL,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strMyAlName.c_str(),
                "",
                "");
            CMsgBase::SendOperateMail(tbHisAlliance.m_nOid,
                EN_MAIL_ID__BE_HOSTILE,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strMyAlName.c_str(),
                "",
                "");
        }
    }
    else if(ptbHisDiplomacy->m_nType == EN_DIPLOMACY_TYPE__HOSTILE)  //容错
    {
        if(ucTargetDiplomacy == EN_DIPLOMACY_TYPE__NORMAL)
        {
            ucMyDipFlag = EN_TABLE_UPDT_FLAG__DEL;
            pstSession->m_stUserInfo.m_udwDiplomacyDelNum++;
            CMsgBase::SendOperateAlMail(tbMyAlliance.m_nAid,
                EN_MAIL_ID__CONFIRM_NORMAL,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strHisAlName.c_str(),
                "",
                "");
        }
        else if(ucTargetDiplomacy == EN_DIPLOMACY_TYPE__HOSTILE)
        {
            CDiplomacyProcess::TbDiplomacy_SetKey(ptbMyDiplomacy, tbMyAlliance.m_nAid, tbHisAlliance.m_nAid, EN_DIPLOMACY_TYPE__HOSTILE);
            CMsgBase::SendOperateAlMail(tbMyAlliance.m_nAid,
                EN_MAIL_ID__CONFIRM_HOSTILE,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strHisAlName.c_str(),
                "",
                "");
            CMsgBase::SendOperateMail(tbHisAlliance.m_nOid,
                EN_MAIL_ID__BE_HOSTILE,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strMyAlName.c_str(),
                "",
                "");
        }
    }
    else if(ptbHisDiplomacy->m_nType == EN_DIPLOMACY_TYPE__PEDDING)  //容错
    {
        if(ucTargetDiplomacy == EN_DIPLOMACY_TYPE__NORMAL)
        {
            ucMyDipFlag = EN_TABLE_UPDT_FLAG__DEL;
            pstSession->m_stUserInfo.m_udwDiplomacyDelNum++;
            CMsgBase::SendOperateAlMail(tbMyAlliance.m_nAid,
                EN_MAIL_ID__CONFIRM_NORMAL,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strHisAlName.c_str(),
                "",
                "");
        }
        else if(ucTargetDiplomacy == EN_DIPLOMACY_TYPE__HOSTILE)
        {
            CDiplomacyProcess::TbDiplomacy_SetKey(ptbMyDiplomacy, tbMyAlliance.m_nAid, tbHisAlliance.m_nAid, EN_DIPLOMACY_TYPE__HOSTILE);
            CMsgBase::SendOperateAlMail(tbMyAlliance.m_nAid,
                EN_MAIL_ID__CONFIRM_HOSTILE,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strHisAlName.c_str(),
                "",
                "");
            CMsgBase::SendOperateMail(tbHisAlliance.m_nOid,
                EN_MAIL_ID__BE_HOSTILE,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strMyAlName.c_str(),
                "",
                "");
        }
    }
    else
    {
        return -2;
    }
    return 0;
}

TINT32 CProcessAlliance::ProcessDiplomacyIfHeIsHostile(TbDiplomacy* ptbMyDiplomacy, TbDiplomacy* ptbHisDiplomacy, TUINT8 ucTargetDiplomacy, TbAlliance& tbMyAlliance, TbAlliance& tbHisAlliance, TUINT8& ucMyDipFlag, TUINT8& ucHisDipFlag, SSession *pstSession)
{
    if(ptbMyDiplomacy->m_nType != EN_DIPLOMACY_TYPE__HOSTILE)
    {
        return -1;
    }
    string strMyAlName = "";
    string strHisAlName = "";
    Json::Value jMyAlInfo;
    Json::Value jHisAlInfo;
    Json::FastWriter tmpWriter;
    string strMyAlInfo = "";
    string strHisAlInfo = "";

    strMyAlName = "(" + tbMyAlliance.m_sAl_nick_name + ")" + tbMyAlliance.m_sName;
    strHisAlName = "(" + tbHisAlliance.m_sAl_nick_name + ")" + tbHisAlliance.m_sName;

    AllianceRank stAlRank;

    TCHAR szResponseData[1024];
    TCHAR szUrl[1024];
    memset(szResponseData, 0, sizeof(szResponseData));
    memset(szUrl, 0, sizeof(szUrl));
    sprintf(szUrl, "%s?request=op_en_flag=0&did=system&sid=0&uid=0&aid=0&command=get_al_rank_by_id&key0=%ld&key1=%ld",
        CGlobalServ::m_poConf->m_szRankUrlPre,
        tbMyAlliance.m_nSid,
        tbMyAlliance.m_nAid);

    CURLcode res = CToolBase::ResFromUrl(szUrl, szResponseData);
    if (CURLE_OK != res)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("get al rank failed. [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
    }
    for (TUINT32 udwIdx = 0; udwIdx < strlen(szResponseData); ++udwIdx)
    {
        if (szResponseData[udwIdx] == '\n')
        {
            szResponseData[udwIdx] = ' ';
        }
    }
    Json::Reader reader;
    Json::Value jRes;
    if (TRUE == reader.parse(szResponseData, jRes))
    {
        if (jRes.isMember("res_data")
            && jRes["res_data"].isArray()
            && jRes["res_data"][0u].isMember("data")
            && jRes["res_data"][0u]["data"][0u].isMember("data"))
        {
            stAlRank.audwRank[2] = jRes["res_data"][0u]["data"][0u]["data"]["force_rank"].asUInt64();
            stAlRank.audwRank[1] = jRes["res_data"][0u]["data"][0u]["data"]["troop_kill_rank"].asUInt64();
        }
    }

    CCommJson::GenAllianceInfo(&stAlRank, &tbMyAlliance, jMyAlInfo);
    strMyAlInfo = tmpWriter.write(jMyAlInfo);
    CCommJson::GenAllianceInfo(&tbHisAlliance, jHisAlInfo);
    strHisAlInfo = tmpWriter.write(jHisAlInfo);

    //ucMyDipFlag除非是delete，否则不需要更改
    if(ptbHisDiplomacy->m_nType == EN_DIPLOMACY_TYPE__NORMAL)
    {
        if(ucTargetDiplomacy == EN_DIPLOMACY_TYPE__NORMAL)
        {
            ucMyDipFlag = EN_TABLE_UPDT_FLAG__DEL;
            pstSession->m_stUserInfo.m_udwDiplomacyDelNum++;
            CMsgBase::SendOperateAlMail(tbMyAlliance.m_nAid,
                EN_MAIL_ID__CONFIRM_NORMAL,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strHisAlName.c_str(),
                "",
                "");
            CMsgBase::SendOperateMail(tbHisAlliance.m_nOid,
                EN_MAIL_ID__CANCEL_HOSTILE,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strMyAlName.c_str(),
                strMyAlInfo.c_str(),
                "");
        }
        else if(ucTargetDiplomacy == EN_DIPLOMACY_TYPE__FRIENDLY)
        {
            CDiplomacyProcess::TbDiplomacy_SetKey(ptbMyDiplomacy, tbMyAlliance.m_nAid, tbHisAlliance.m_nAid, EN_DIPLOMACY_TYPE__PEDDING);
            CMsgBase::SendOperateAlMail(tbMyAlliance.m_nAid,
                EN_MAIL_ID__CONFIRM_NORMAL,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strHisAlName.c_str(),
                "",
                "");
            CMsgBase::SendOperateMail(tbHisAlliance.m_nOid,
                EN_MAIL_ID__BE_FRIEND,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strMyAlName.c_str(),
                strMyAlInfo.c_str(),
                "");
        }
    }
    else if(ptbHisDiplomacy->m_nType == EN_DIPLOMACY_TYPE__FRIENDLY)  //容错
    {
        if(ucTargetDiplomacy == EN_DIPLOMACY_TYPE__NORMAL)
        {
            ucMyDipFlag = EN_TABLE_UPDT_FLAG__DEL;
            pstSession->m_stUserInfo.m_udwDiplomacyDelNum++;
            CMsgBase::SendOperateAlMail(tbMyAlliance.m_nAid,
                EN_MAIL_ID__CONFIRM_NORMAL,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strHisAlName.c_str(),
                "",
                "");
            CDiplomacyProcess::TbDiplomacy_SetKey(ptbHisDiplomacy, tbHisAlliance.m_nAid, tbMyAlliance.m_nAid, EN_DIPLOMACY_TYPE__PEDDING);
            ucHisDipFlag = EN_TABLE_UPDT_FLAG__CHANGE;
            CMsgBase::SendOperateAlMail(tbHisAlliance.m_nAid,
                EN_MAIL_ID__CONFIRM_NORMAL,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strMyAlName.c_str(),
                "",
                "");
        }
        else if(ucTargetDiplomacy == EN_DIPLOMACY_TYPE__FRIENDLY)
        {
            CDiplomacyProcess::TbDiplomacy_SetKey(ptbMyDiplomacy, tbMyAlliance.m_nAid, tbHisAlliance.m_nAid, EN_DIPLOMACY_TYPE__FRIENDLY);
            CMsgBase::SendOperateAlMail(tbMyAlliance.m_nAid,
                EN_MAIL_ID__CONFIRM_FRIEND,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strHisAlName.c_str(),
                "",
                "");
        }
    }
    else if(ptbHisDiplomacy->m_nType == EN_DIPLOMACY_TYPE__HOSTILE)
    {
        if(ucTargetDiplomacy == EN_DIPLOMACY_TYPE__NORMAL)
        {
            ucMyDipFlag = EN_TABLE_UPDT_FLAG__DEL;
            pstSession->m_stUserInfo.m_udwDiplomacyDelNum++;
            CMsgBase::SendOperateAlMail(tbMyAlliance.m_nAid,
                EN_MAIL_ID__CONFIRM_NORMAL,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strHisAlName.c_str(),
                "",
                "");
            CMsgBase::SendOperateMail(tbHisAlliance.m_nOid,
                EN_MAIL_ID__CANCEL_HOSTILE,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strMyAlName.c_str(),
                strMyAlInfo.c_str(),
                "");
        }
        else if(ucTargetDiplomacy == EN_DIPLOMACY_TYPE__FRIENDLY)
        {
            CDiplomacyProcess::TbDiplomacy_SetKey(ptbMyDiplomacy, tbMyAlliance.m_nAid, tbHisAlliance.m_nAid, EN_DIPLOMACY_TYPE__PEDDING);
            CMsgBase::SendOperateAlMail(tbMyAlliance.m_nAid,
                EN_MAIL_ID__CONFIRM_NORMAL,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strHisAlName.c_str(),
                "",
                "");
            CMsgBase::SendOperateMail(tbHisAlliance.m_nOid,
                EN_MAIL_ID__BE_FRIEND,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strMyAlName.c_str(),
                strMyAlInfo.c_str(),
                "");
        }
    }
    else if(ptbHisDiplomacy->m_nType == EN_DIPLOMACY_TYPE__PEDDING)
    {
        if(ucTargetDiplomacy == EN_DIPLOMACY_TYPE__NORMAL)
        {
            ucMyDipFlag = EN_TABLE_UPDT_FLAG__DEL;
            pstSession->m_stUserInfo.m_udwDiplomacyDelNum++;
            CMsgBase::SendOperateAlMail(tbMyAlliance.m_nAid,
                EN_MAIL_ID__CONFIRM_NORMAL,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strHisAlName.c_str(),
                "",
                "");
            CMsgBase::SendOperateMail(tbHisAlliance.m_nOid,
                EN_MAIL_ID__CANCEL_HOSTILE,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strMyAlName.c_str(),
                strMyAlInfo.c_str(),
                "");
        }
        else if(ucTargetDiplomacy == EN_DIPLOMACY_TYPE__FRIENDLY)
        {
            CDiplomacyProcess::TbDiplomacy_SetKey(ptbMyDiplomacy, tbMyAlliance.m_nAid, tbHisAlliance.m_nAid, EN_DIPLOMACY_TYPE__FRIENDLY);
            CMsgBase::SendOperateAlMail(tbMyAlliance.m_nAid,
                EN_MAIL_ID__CONFIRM_FRIEND,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strHisAlName.c_str(),
                "",
                "");

            CDiplomacyProcess::TbDiplomacy_SetKey(ptbHisDiplomacy, tbHisAlliance.m_nAid, tbMyAlliance.m_nAid, EN_DIPLOMACY_TYPE__FRIENDLY);
            ucHisDipFlag = EN_TABLE_UPDT_FLAG__CHANGE;
            CMsgBase::SendOperateAlMail(tbHisAlliance.m_nAid,
                EN_MAIL_ID__CONFIRM_FRIEND,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strMyAlName.c_str(),
                "",
                "");
        }
    }
    else
    {
        return -4;
    }
    return 0;
}

TINT32 CProcessAlliance::FakeOpenAlGift(TbAl_gift_reward *ptbAlGiftReward, TbAl_gift* ptbAlGift)
{
    if(ptbAlGiftReward == NULL || ptbAlGift == NULL)
    {
        return -1;
    }

    CGameInfo* poGameInfo = CGameInfo::GetInstance();

    //开始定位reward
    TINT32 dwPackId = ptbAlGift->m_nPack_id;  //拿到pack id
    if(!poGameInfo->m_oJsonRoot["game_al_gift_new"]["pack"].isMember(CCommonFunc::NumToString(dwPackId)))
    {
        return -2;
    }
    //根据pack id，找到rewardlist
    const Json::Value &jRewardList = poGameInfo->m_oJsonRoot["game_al_gift_new"]["pack"][CCommonFunc::NumToString(dwPackId)]["reward_list"];
    TINT32 dwRewardSize = jRewardList.size();
    if(dwRewardSize == 0)
    {
        return -3;
    }
    //根据概率随机一个reward id
    TUINT32 dwRewardId = CGlobalResLogic::GetAlGiftRewarId(poGameInfo->m_oJsonRoot["game_al_gift_new"]["pack"][CCommonFunc::NumToString(dwPackId)]);
    if(!poGameInfo->m_oJsonRoot["game_al_gift_new"]["reward"].isMember(CCommonFunc::NumToString(dwRewardId)))
    {
        return -4;
    }
    //拿到了reward id，开始生成对应礼品
    TINT32 dwRewardType = poGameInfo->m_oJsonRoot["game_al_gift_new"]["reward"][CCommonFunc::NumToString(dwRewardId)]["r0"].asInt();  //获取方式，0-全部获取，1-随机若干，2-随机一种
    const Json::Value& oRewardJson = poGameInfo->m_oJsonRoot["game_al_gift_new"]["reward"][CCommonFunc::NumToString(dwRewardId)]["r1"];
    SGlobalRes stRes;
    CGlobalResLogic::GetGlobalResInfo(oRewardJson, dwRewardType, &stRes);
    for (TINT32 dwIdx = 0; dwIdx < stRes.ddwTotalNum; dwIdx++)
    {
        ptbAlGiftReward->m_bReward[ptbAlGiftReward->m_bReward.m_udwNum].ddwType = stRes[dwIdx].ddwType;
        ptbAlGiftReward->m_bReward[ptbAlGiftReward->m_bReward.m_udwNum].ddwId = stRes[dwIdx].ddwId;
        ptbAlGiftReward->m_bReward[ptbAlGiftReward->m_bReward.m_udwNum].ddwNum = stRes[dwIdx].ddwNum;
        ptbAlGiftReward->m_bReward.m_udwNum++;
    }
    ptbAlGiftReward->SetFlag(TbAL_GIFT_REWARD_FIELD_REWARD);
    //记录为过期状态
    ptbAlGiftReward->Set_Status(EN_AL_GIFT_STATUS_EXPIRED);

    return 0;
}

TINT32 CProcessAlliance::ProcessDiplomacyIfHeIsPedding(TbDiplomacy* ptbMyDiplomacy, TbDiplomacy* ptbHisDiplomacy, TUINT8 ucTargetDiplomacy, TbAlliance& tbMyAlliance, TbAlliance& tbHisAlliance, TUINT8& ucMyDipFlag, TUINT8& ucHisDipFlag, SSession *pstSession)
{
    if(ptbMyDiplomacy->m_nType != EN_DIPLOMACY_TYPE__PEDDING)
    {
        return -1;
    }
    string strMyAlName = "";
    string strHisAlName = "";
    Json::Value jMyAlInfo;
    Json::Value jHisAlInfo;
    Json::FastWriter tmpWriter;
    string strMyAlInfo = "";
    string strHisAlInfo = "";

    strMyAlName = "(" + tbMyAlliance.m_sAl_nick_name + ")" + tbMyAlliance.m_sName;
    strHisAlName = "(" + tbHisAlliance.m_sAl_nick_name + ")" + tbHisAlliance.m_sName;
    CCommJson::GenAllianceInfo(&tbMyAlliance, jMyAlInfo);
    strMyAlInfo = tmpWriter.write(jMyAlInfo);
    CCommJson::GenAllianceInfo(&tbHisAlliance, jHisAlInfo);
    strHisAlInfo = tmpWriter.write(jHisAlInfo);

    //ucMyDipFlag除非是delete，否则不需要更改
    if(ptbHisDiplomacy->m_nType == EN_DIPLOMACY_TYPE__NORMAL)
    {
        if(ucTargetDiplomacy == EN_DIPLOMACY_TYPE__NORMAL)
        {
            ucMyDipFlag = EN_TABLE_UPDT_FLAG__DEL;
            pstSession->m_stUserInfo.m_udwDiplomacyDelNum++;
        }
        else if(ucTargetDiplomacy == EN_DIPLOMACY_TYPE__HOSTILE)
        {
            CDiplomacyProcess::TbDiplomacy_SetKey(ptbMyDiplomacy, tbMyAlliance.m_nAid, tbHisAlliance.m_nAid, EN_DIPLOMACY_TYPE__HOSTILE);
            CMsgBase::SendOperateAlMail(tbMyAlliance.m_nAid,
                EN_MAIL_ID__CONFIRM_HOSTILE,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strHisAlName.c_str(),
                "",
                "");
            CMsgBase::SendOperateMail(tbHisAlliance.m_nOid,
                EN_MAIL_ID__BE_HOSTILE,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strMyAlName.c_str(),
                "",
                "");
        }
    }
    else if(ptbHisDiplomacy->m_nType == EN_DIPLOMACY_TYPE__FRIENDLY)  //容错
    {
        if(ucTargetDiplomacy == EN_DIPLOMACY_TYPE__NORMAL)
        {
            ucMyDipFlag = EN_TABLE_UPDT_FLAG__DEL;
            pstSession->m_stUserInfo.m_udwDiplomacyDelNum++;

            CDiplomacyProcess::TbDiplomacy_SetKey(ptbHisDiplomacy, tbHisAlliance.m_nAid, tbMyAlliance.m_nAid, EN_DIPLOMACY_TYPE__NORMAL);
            ucHisDipFlag = EN_TABLE_UPDT_FLAG__DEL;
            CMsgBase::SendOperateAlMail(tbHisAlliance.m_nAid,
                EN_MAIL_ID__CONFIRM_NORMAL,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strMyAlName.c_str(),
                "",
                "");
        }
        else if(ucTargetDiplomacy == EN_DIPLOMACY_TYPE__HOSTILE)
        {
            CDiplomacyProcess::TbDiplomacy_SetKey(ptbMyDiplomacy, tbMyAlliance.m_nAid, tbHisAlliance.m_nAid, EN_DIPLOMACY_TYPE__HOSTILE);
            CMsgBase::SendOperateAlMail(tbMyAlliance.m_nAid,
                EN_MAIL_ID__CONFIRM_HOSTILE,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strHisAlName.c_str(),
                "",
                "");

            CDiplomacyProcess::TbDiplomacy_SetKey(ptbHisDiplomacy, tbHisAlliance.m_nAid, tbMyAlliance.m_nAid, EN_DIPLOMACY_TYPE__NORMAL);
            ucHisDipFlag = EN_TABLE_UPDT_FLAG__DEL;
            CMsgBase::SendOperateMail(tbHisAlliance.m_nOid,
                EN_MAIL_ID__BE_HOSTILE,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strMyAlName.c_str(),
                "",
                "");
        }
    }
    else if(ptbHisDiplomacy->m_nType == EN_DIPLOMACY_TYPE__HOSTILE)  //容错
    {
        if(ucTargetDiplomacy == EN_DIPLOMACY_TYPE__NORMAL)
        {
            ucMyDipFlag = EN_TABLE_UPDT_FLAG__DEL;
            pstSession->m_stUserInfo.m_udwDiplomacyDelNum++;
        }
        else if(ucTargetDiplomacy == EN_DIPLOMACY_TYPE__HOSTILE)
        {
            CDiplomacyProcess::TbDiplomacy_SetKey(ptbMyDiplomacy, tbMyAlliance.m_nAid, tbHisAlliance.m_nAid, EN_DIPLOMACY_TYPE__HOSTILE);
            CMsgBase::SendOperateAlMail(tbMyAlliance.m_nAid,
                EN_MAIL_ID__CONFIRM_HOSTILE,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strHisAlName.c_str(),
                "",
                "");
            CMsgBase::SendOperateMail(tbHisAlliance.m_nOid,
                EN_MAIL_ID__BE_HOSTILE,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strMyAlName.c_str(),
                "",
                "");
        }
    }
    else if(ptbHisDiplomacy->m_nType == EN_DIPLOMACY_TYPE__PEDDING)  //容错
    {
        if(ucTargetDiplomacy == EN_DIPLOMACY_TYPE__NORMAL)
        {
            ucMyDipFlag = EN_TABLE_UPDT_FLAG__DEL;
            pstSession->m_stUserInfo.m_udwDiplomacyDelNum++;
        }
        else if(ucTargetDiplomacy == EN_DIPLOMACY_TYPE__HOSTILE)
        {
            CDiplomacyProcess::TbDiplomacy_SetKey(ptbMyDiplomacy, tbMyAlliance.m_nAid, tbHisAlliance.m_nAid, EN_DIPLOMACY_TYPE__HOSTILE);
            CMsgBase::SendOperateAlMail(tbMyAlliance.m_nAid,
                EN_MAIL_ID__CONFIRM_HOSTILE,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strHisAlName.c_str(),
                "",
                "");
            CMsgBase::SendOperateMail(tbHisAlliance.m_nOid,
                EN_MAIL_ID__BE_HOSTILE,
                pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE,
                strMyAlName.c_str(),
                "",
                "");
        }
    }
    else
    {
        return -2;
    }
    return 0;
}
