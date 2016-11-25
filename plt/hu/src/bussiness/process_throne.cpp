#include "process_throne.h"

#include "procedure_base.h"
#include "common_func.h"
#include "common_logic.h"
#include "sendmessage_base.h"
#include "action_base.h"
#include "msg_base.h"
#include "game_info.h"

TINT32 CProcessThrone::ProcessCmd_GetIdolInfo(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;

    TINT32 dwPos = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TINT32 dwSid = atoi(pstSession->m_stReqParam.m_szKey[1]);

    if (strlen(pstSession->m_stReqParam.m_szKey[1]) == 0)
    {
        dwSid = pstSession->m_stReqParam.m_udwSvrId;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        pstSession->ResetAwsInfo();
        CAwsRequest::MapGetByIdAndSid(pstSession, dwPos, dwSid);
        CAwsRequest::IdolQuery(pstSession, dwSid);

        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_GetIdolInfo: send map get req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
        return 0;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecAwsRsp.size(); udwIdx++)
        {
            AwsRspInfo *pstAwsRspInfo = pstSession->m_vecAwsRsp[udwIdx];
            string strTableRawName = CCommonFunc::GetTableRawName(pstAwsRspInfo->sTableName);
            if (strTableRawName == EN_AWS_TABLE_MAP)
            {
                CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &pstSession->m_tbTmpMap);
                continue;
            }
            if (strTableRawName == EN_AWS_TABLE_IDOL)
            {
                dwRetCode = CAwsResponse::OnQueryRsp(*pstAwsRspInfo, pstSession->m_atbIdol, sizeof(TbIdol), MAX_IDOL_NUM);
                if (dwRetCode >= 0)
                {
                    pstSession->m_udwIdolNum = dwRetCode;
                }
                continue;
            }
        }

        if (pstSession->m_tbTmpMap.m_nId != dwPos || pstSession->m_tbTmpMap.m_nType != EN_WILD_TYPE__IDOL)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_GetIdolInfo: target_map error[req_pos:%d, map_pos:%ld, map_type:%ld] [seq=%u]", 
                dwPos, pstSession->m_tbTmpMap.m_nId, pstSession->m_tbTmpMap.m_nType, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }

        TBOOL bIsFind = FALSE;
        for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_udwIdolNum; udwIdx++)
        {
            if (pstSession->m_atbIdol[udwIdx].m_nPos == dwPos)
            {
                bIsFind = TRUE;
                break;
            }
        }
        if (bIsFind == FALSE)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_GetIdolInfo: there is no idol on pos[%d] [idol_num:%u] [seq=%u]",
                dwPos, pstSession->m_udwIdolNum, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
    }

    pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_IDOL_INFO;
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessThrone::ProcessCmd_GetThroneInfo(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;

    TINT32 dwPos = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TINT32 dwSid = atoi(pstSession->m_stReqParam.m_szKey[1]);

    if (strlen(pstSession->m_stReqParam.m_szKey[1]) == 0)
    {
        dwSid = pstSession->m_stReqParam.m_udwSvrId;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        pstSession->ResetAwsInfo();
        CAwsRequest::MapGetByIdAndSid(pstSession, dwPos, dwSid);
        CAwsRequest::ThroneGet(pstSession, dwSid);
        CAwsRequest::TitleQuery(pstSession, dwSid);

        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_GetThroneInfo: send map get req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
        return 0;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecAwsRsp.size(); udwIdx++)
        {
            AwsRspInfo *pstAwsRspInfo = pstSession->m_vecAwsRsp[udwIdx];
            string strTableRawName = CCommonFunc::GetTableRawName(pstAwsRspInfo->sTableName);
            if (strTableRawName == EN_AWS_TABLE_MAP)
            {
                CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &pstSession->m_tbTmpMap);
                continue;
            }
            if (strTableRawName == EN_AWS_TABLE_THRONE)
            {
                CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &pstSession->m_tbThrone);
                continue;
            }
            if (strTableRawName == EN_AWS_TABLE_TITLE)
            {
                dwRetCode =  CAwsResponse::OnQueryRsp(*pstAwsRspInfo, pstSession->m_stTitleInfoList.atbTitle, sizeof(TbTitle), MAX_TITLEINFO_NUM_IN_ONE_SERVER);
                if (dwRetCode > 0)
                {
                    pstSession->m_stTitleInfoList.udwNum = dwRetCode;
                }
                continue;
            }
        }

        if (pstSession->m_tbTmpMap.m_nId != dwPos || pstSession->m_tbTmpMap.m_nType != EN_WILD_TYPE__THRONE_NEW
            || pstSession->m_tbThrone.m_nPos != dwPos)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_GetIdolInfo: target_map error[req_pos:%d, map_pos:%ld, map_type:%ld, throne_pos:%ld] [seq=%u]",
                dwPos, pstSession->m_tbTmpMap.m_nId, pstSession->m_tbTmpMap.m_nType, pstSession->m_tbThrone.m_nPos, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }

        pstSession->ResetAwsInfo();

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        if (pstSession->m_tbThrone.m_nAlid > 0)
        {
            CAwsRequest::UserGetByUid(pstSession, pstSession->m_tbThrone.m_nOwner_id);
            CAwsRequest::AllianceGetByAid(pstSession, pstSession->m_tbThrone.m_nAlid);
            CAwsRequest::MarchActionQueryBySal(pstSession, pstSession->m_tbThrone.m_nSid, pstSession->m_tbThrone.m_nAlid);

            pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
            // send request
            bNeedResponse = TRUE;
            dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
            if (dwRetCode < 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_GetThroneInfo: send map get req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                return -1;
            }
            return 0;
        }
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecAwsRsp.size(); udwIdx++)
        {
            AwsRspInfo *pstAwsRspInfo = pstSession->m_vecAwsRsp[udwIdx];
            string strTableRawName = CCommonFunc::GetTableRawName(pstAwsRspInfo->sTableName);
            if (strTableRawName == EN_AWS_TABLE_PLAYER)
            {
                CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &pstSession->m_tbTmpPlayer);
                continue;
            }
            if (strTableRawName == EN_AWS_TABLE_ALLIANCE)
            {
                CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &pstSession->m_tbTmpAlliance);
                continue;
            }
            if (strTableRawName == EN_AWS_TABLE_MARCH_ACTION)
            {
                TINT32 dwRet = CAwsResponse::OnQueryRsp(*pstAwsRspInfo, pstSession->m_atbTmpMarch, sizeof(TbMarch_action), MAX_USER_MARCH_NUM);
                if (dwRet >= 0)
                {
                    pstSession->m_udwTmpMarchNum = dwRet;
                }
                continue;
            }
        }

        if (pstSession->m_tbTmpAlliance.m_nAid != pstSession->m_tbThrone.m_nAlid
            || pstSession->m_tbTmpPlayer.m_nUid != pstSession->m_tbThrone.m_nOwner_id)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PROCESS_CMD_ERROR;
            return -1;
        }
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    }

    pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_THRONE_INFO;
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessThrone::ProcessCmd_ThroneDismissTitle(SSession *pstSession, TBOOL &bNeedResponse)
{
    TbPlayer *ptbSelfPlayer = &pstSession->m_stUserInfo.m_tbPlayer;

    // 0. 获取输入参数
    TUINT32 udwTargetUserId = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TINT32 dwTitleId = atoi(pstSession->m_stReqParam.m_szKey[1]);

    if (ptbSelfPlayer->m_nAlpos != EN_ALLIANCE_POS__CHANCELLOR
        || pstSession->m_tbThrone.m_nAlid != ptbSelfPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }

    // check param
    if (ptbSelfPlayer->m_nUid == udwTargetUserId)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ThroneDubTitle: cannot dub yourself [seq=%u]",
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -2;
    }

    CCommonLogic::RemoveTitle(udwTargetUserId, dwTitleId, &pstSession->m_stTitleInfoList);

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessThrone::ProcessCmd_ThroneDubTitle(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    TbPlayer *ptbSelfPlayer = &pstSession->m_stUserInfo.m_tbPlayer;

    // 0. 获取输入参数
    TUINT32 udwTargetUserId = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TINT32 dwTitleId = atoi(pstSession->m_stReqParam.m_szKey[1]);

    // 1. check param
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        if (ptbSelfPlayer->m_nAlpos != EN_ALLIANCE_POS__CHANCELLOR
            || pstSession->m_tbThrone.m_nAlid != ptbSelfPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -1;
        }

        // check param
        if (ptbSelfPlayer->m_nUid == udwTargetUserId)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ThroneDubTitle: cannot dub yourself [seq=%u]", 
                pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }

        if (CCommonLogic::HasTitleDub(dwTitleId, &pstSession->m_tbThrone, &pstSession->m_stTitleInfoList))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ThroneDubTitle: title has dub [seq=%u]",
                pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }

        if (CCommonLogic::HasTitle(udwTargetUserId, &pstSession->m_tbThrone, &pstSession->m_stTitleInfoList))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ALREADY_HAS_TITLE;
            return -5;
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
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ThroneDubTitle: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }

        return 0;
    }

    // 3. 处理响应
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        pstSession->m_tbTmpPlayer.Reset();
        dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[0], &pstSession->m_tbTmpPlayer);

        if (pstSession->m_tbTmpPlayer.m_nUid == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -4;
        }

        if (pstSession->m_tbTmpPlayer.m_nSid != ptbSelfPlayer->m_nSid)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PLAYER_CHANGE_SVR;
            return -4;
        }

        CCommonLogic::AddTitle(&pstSession->m_tbTmpPlayer, dwTitleId, &pstSession->m_stTitleInfoList);
        const Json::Value &jTitle = CGameInfo::GetInstance()->m_oJsonRoot["game_title"];
        ostringstream oss;
        oss.str("");
        oss << dwTitleId;
        TINT32 dwExpireTime = -1;
        if (jTitle.isMember(oss.str()))
        {
            dwExpireTime = jTitle[oss.str()]["time"].asInt();
        }
        Json::Value jExInfo = Json::Value(Json::arrayValue);
        //jExInfo.append(ptbSelfPlayer->m_sUin);
        //jExInfo.append(pstSession->m_tbTmpPlayer.m_sUin);
        jExInfo.append(dwTitleId);
        jExInfo.append(dwExpireTime);

        Json::FastWriter writer;
        writer.omitEndingLineFeed();
        CMsgBase::SendOperateMail(pstSession->m_tbTmpPlayer.m_nUid, EN_MAIL_ID__BE_DUBBED_TITLE, 
            pstSession->m_tbTmpPlayer.m_nSid, 0, "", writer.write(jExInfo).c_str(), "");

        if (pstSession->m_tbThrone.m_nStatus != EN_THRONE_STATUS__CONTEST_PERIOD)
        {
            string szTmp = pstSession->m_tbTmpPlayer.m_sUin + "#" + CCommonFunc::NumToString(dwTitleId);
            CSendMessageBase::SendBroadcast(&pstSession->m_stUserInfo, ptbSelfPlayer->m_nSid, 0, EN_BROADCAST_CONTENT_ID__DUB_TITLE, szTmp);
        }

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessThrone::ProcessCmd_ThroneAbandon(SSession *pstSession, TBOOL &bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;
    TbAlliance *ptbAlliance = &pstUser->m_tbAlliance;
    TbThrone *ptbThrone = &pstSession->m_tbThrone;
    TbMap *ptbWild = NULL;

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwWildNum; udwIdx++)
        {
            if (pstUser->m_atbWild[udwIdx].m_nType == EN_WILD_TYPE__THRONE_NEW
                && pstUser->m_atbWild[udwIdx].m_nId == ptbThrone->m_nPos)
            {
                ptbWild = &pstUser->m_atbWild[udwIdx];
                pstUser->m_aucWildFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
            }
        }

        if (ptbWild == NULL || ptbPlayer->m_nAlpos != EN_ALLIANCE_POS__CHANCELLOR
            || pstSession->m_tbThrone.m_nAlid != ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET)
        {
            //pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            //无王座...不报错...
            return 0;
        }

        for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; udwIdx++)
        {
            //召回王座相关的reinforce/dispatch
            if (pstUser->m_atbMarch[udwIdx].m_nSal == ptbAlliance->m_nAid
                && pstUser->m_atbMarch[udwIdx].m_nMclass == EN_ACTION_MAIN_CLASS__MARCH
                && (pstUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_THRONE
                || pstUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__ASSIGN_THRONE)
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

        if (ptbThrone->m_nStatus == EN_THRONE_STATUS__PEACE_TIME)
        {
            CSendMessageBase::SendBroadcast(pstUser, ptbThrone->m_nSid, 0, EN_BROADCAST_CONTENT_ID__THRONE_PEACE_TIME_END);
        }

        CCommonLogic::AbandonThrone(ptbAlliance, ptbThrone, ptbWild);
        ptbThrone->Set_Status(EN_THRONE_STATUS__QUIET_PERIOD);
        ptbThrone->Set_End_time(0);
        ptbWild->Set_Status(ptbThrone->m_nStatus);
        ptbWild->Set_Time_end(0);

        TbMarch_action tbMarch;

        tbMarch.Reset();
        tbMarch.Set_Suid(0);
        tbMarch.Set_Id(CActionBase::GenMapActionId(ptbThrone->m_nSid, ptbThrone->m_nPos));
        tbMarch.Set_Sid(ptbThrone->m_nSid);
        CAwsRequest::DeleteItem(pstSession, &tbMarch);

        TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
        for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_udwIdolNum; udwIdx++)
        {
            tbMarch.Reset();
            tbMarch.Set_Suid(0);
            tbMarch.Set_Id(CActionBase::GenMapActionId(pstSession->m_atbIdol[udwIdx].m_nSid, pstSession->m_atbIdol[udwIdx].m_nPos));
            tbMarch.Set_Sid(pstSession->m_atbIdol[udwIdx].m_nSid);
            tbMarch.Set_Tpos(pstSession->m_atbIdol[udwIdx].m_nPos);
            tbMarch.Set_Mclass(EN_ACTION_MAIN_CLASS__TIMER);
            tbMarch.Set_Sclass(EN_ACTION_SEC_CLASS__IDOL_PERIOD);
            tbMarch.Set_Status(EN_TITMER_IDOL_PERIOD_STATUS__THRONE_PEACE_TIME);
            tbMarch.Set_Etime(udwCurTime + 2); //防止提前执行了...需要在throne数据更新之后才执行
            CAwsRequest::UpdateItem(pstSession, &tbMarch);
        }

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        if (pstSession->m_vecAwsReq.size() > 0)
        {
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
            bNeedResponse = TRUE;
            TINT32 dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
            if (dwRetCode < 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("TableRequest_MapGetByCids: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                return -1;
            }
            return 0;
        }
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessThrone::ProcessCmd_ThroneSetTax(SSession *pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;
    TbAlliance *ptbAlliance = &pstUser->m_tbAlliance;

    TINT32 dwTaxId = atoi(pstSession->m_stReqParam.m_szKey[0]);

    if (pstSession->m_tbThrone.m_nAlid == ptbAlliance->m_nAid
        && ptbAlliance->m_nOid == ptbPlayer->m_nUid)
    {
        pstSession->m_tbThrone.Set_Tax_id(dwTaxId);
    }
    else
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ThroneTaxSet: have no permission[throne_pos=%ld][throne_alid=%ld][oid=%ld][uid=%ld] [seq=%u]",
            pstSession->m_tbThrone.m_nPos, pstSession->m_tbThrone.m_nAlid, ptbAlliance->m_nOid, ptbPlayer->m_nUid, pstUser->m_udwBSeqNo));
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__HAVE_NO_PERMISSION;
        return -1;
    }

    // next procedure
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;

    return 0;
}

TINT32 CProcessThrone::ProcessCmd_GetTitleInfo(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;

    TINT32 dwSid = atoi(pstSession->m_stReqParam.m_szKey[0]);

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        pstSession->ResetAwsInfo();
        CAwsRequest::ThroneGet(pstSession, dwSid);
        CAwsRequest::TitleQuery(pstSession, dwSid);

        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_GetTitleInfo: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
        return 0;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecAwsRsp.size(); udwIdx++)
        {
            AwsRspInfo *pstAwsRspInfo = pstSession->m_vecAwsRsp[udwIdx];
            string strTableRawName = CCommonFunc::GetTableRawName(pstAwsRspInfo->sTableName);
            if (strTableRawName == EN_AWS_TABLE_THRONE)
            {
                CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &pstSession->m_tbThrone);
                continue;
            }
            if (strTableRawName == EN_AWS_TABLE_TITLE)
            {
                dwRetCode = CAwsResponse::OnQueryRsp(*pstAwsRspInfo, pstSession->m_stTitleInfoList.atbTitle, sizeof(TbTitle), MAX_TITLEINFO_NUM_IN_ONE_SERVER);
                if (dwRetCode > 0)
                {
                    pstSession->m_stTitleInfoList.udwNum = dwRetCode;
                }
                continue;
            }
        }

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    }

    pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_THRONE_INFO;
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}