#include "process_operate.h"
#include "task_process.h"
#include "game_info.h"
#include "procedure_base.h"
#include "common_func.h"
#include "db_request.h"
#include "db_response.h"
#include "global_serv.h"
#include "statistic.h"
#include "sendmessage_base.h"
#include "game_command.h"
#include "common_logic.h"
#include <math.h>
#include "msg_base.h"
#include "conf_base.h"
#include "process_alliance.h"
#include "globalres_logic.h"
#include "process_mailreport.h"
#include "document.h"
#include "iap_white_list.h"
#include "conf_base.h"
#include "quest_logic.h"
#include "common_json.h"
#include "warning_mgr.h"
#include "tool_base.h"
#include "city_base.h"
#include "item_base.h"
#include "common_base.h"
#include "map_base.h"
#include "action_base.h"
#include "player_base.h"
#include "report_svr_request.h"
#include "item_logic.h"
#include "pushdata_action.h"

TINT32 CProcessOperate::ProcessCmd_LoginGet( SSession *pstSession, TBOOL &bNeedResponse )
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;

    TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_LoginGet: wild size[%u] [seq=%u]",
        pstUser->m_udwWildNum, pstSession->m_stUserInfo.m_udwBSeqNo));
    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwWildNum; ++udwIdx)
    {
        TbMap *ptbWild = &pstUser->m_atbWild[udwIdx];
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_LoginGet: wild[%u] id[%ld] type[%ld] uid[%ld] uname[%s] alid[%ld] [seq=%u]",
            udwIdx, ptbWild->m_nId, ptbWild->m_nType, ptbWild->m_nUid, ptbWild->m_sUname.c_str(), ptbWild->m_nAlid, pstSession->m_stUserInfo.m_udwBSeqNo));
    }

    TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_LoginGet: march num[%u] [seq=%u]",
        pstUser->m_udwMarchNum, pstSession->m_stUserInfo.m_udwBSeqNo));
    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
    {
        TbMarch_action *ptbAction = &pstUser->m_atbMarch[udwIdx];
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_LoginGet: march[%u] id[%ld] class[%ld:%ld] stat[%ld] time[%ld:%ld:%ld] suid[%ld] scid[%ld] tuid[%ld] tpos[%ld] sal[%ld] tal[%ld] [seq=%u]",
            udwIdx, ptbAction->m_nId, ptbAction->m_nMclass, ptbAction->m_nSclass, ptbAction->m_nStatus,
            ptbAction->m_nBtime, ptbAction->m_nCtime, ptbAction->m_nEtime,
            ptbAction->m_nSuid, ptbAction->m_nScid, ptbAction->m_nTuid, ptbAction->m_nTpos,
            ptbAction->m_nSal, ptbAction->m_nTal, pstSession->m_stUserInfo.m_udwBSeqNo));
    }
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_LoginGet: p_march num[%u] [seq=%u]",
        pstUser->m_udwPassiveMarchNum, pstSession->m_stUserInfo.m_udwBSeqNo));
    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwPassiveMarchNum; ++udwIdx)
    {
        TbMarch_action *ptbAction = &pstUser->m_atbPassiveMarch[udwIdx];
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_LoginGet: p_march[%u] id[%ld] class[%ld:%ld] stat[%ld] time[%ld:%ld:%ld] suid[%ld] scid[%ld] tuid[%ld] tpos[%ld] sal[%ld] tal[%ld] [seq=%u]",
            udwIdx, ptbAction->m_nId, ptbAction->m_nMclass, ptbAction->m_nSclass, ptbAction->m_nStatus,
            ptbAction->m_nBtime, ptbAction->m_nCtime, ptbAction->m_nEtime,
            ptbAction->m_nSuid, ptbAction->m_nScid, ptbAction->m_nTuid, ptbAction->m_nTpos,
            ptbAction->m_nSal, ptbAction->m_nTal, pstSession->m_stUserInfo.m_udwBSeqNo));
    }

    TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_LoginGet: buff_action num[%u] [seq=%u]",
        pstUser->m_udwActionNum, pstSession->m_stUserInfo.m_udwBSeqNo));
    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwActionNum; ++udwIdx)
    {
        TbAction *ptbAction = &pstUser->m_atbAction[udwIdx];
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_LoginGet: action[%u] id[%ld] class[%ld:%ld] stat[%ld] time[%ld:%ld:%ld] suid[%ld] scid[%ld] [seq=%u]",
            udwIdx, ptbAction->m_nId, ptbAction->m_nMclass, ptbAction->m_nSclass, ptbAction->m_nStatus,
            ptbAction->m_nBtime, ptbAction->m_nCtime, ptbAction->m_nEtime,
            ptbAction->m_nSuid, ptbAction->m_nScid, pstSession->m_stUserInfo.m_udwBSeqNo));
    }

    TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_LoginGet: al_action num[%u] [seq=%u]",
        pstUser->m_udwSelfAlActionNum, pstSession->m_stUserInfo.m_udwBSeqNo));
    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwSelfAlActionNum; ++udwIdx)
    {
        TbAlliance_action *ptbAction = &pstUser->m_atbSelfAlAction[udwIdx];
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_LoginGet: action[%u] id[%ld] class[%ld:%ld] stat[%ld] time[%ld:%ld:%ld] suid[%ld] scid[%ld] [seq=%u]",
            udwIdx, ptbAction->m_nId, ptbAction->m_nMclass, ptbAction->m_nSclass, ptbAction->m_nStatus,
            ptbAction->m_nBtime, ptbAction->m_nCtime, ptbAction->m_nEtime,
            ptbAction->m_nSuid, ptbAction->m_nScid, pstSession->m_stUserInfo.m_udwBSeqNo));
    }

	return 0;
}

TINT32 CProcessOperate::ProcessCmd_OperateLog(SSession *pstSession, TBOOL &bNeedResponse)
{
    return 0;
}

TINT32 CProcessOperate::ProcessCmd_GemRecharge(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;

    TUINT32 udwRechargeGem = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    //TUINT32 udwRechargeGemSeq = strtoul(pstSession->m_stReqParam.m_szKey[1], NULL, 10);
    TINT32 dwRechargeType = atoi(pstSession->m_stReqParam.m_szKey[2]); //(线上版本固定为0)(为1是为线下加gem方便)
    string sTransId = pstSession->m_stReqParam.m_szKey[3]; // 与apple交易的id(必传)
    TINT64 ddwInstantId = strtoll(pstSession->m_stReqParam.m_szKey[4], NULL, 10); //促销id,兼容pid
    TINT64 ddwProjectId = strtoll(pstSession->m_stReqParam.m_szKey[5], NULL, 10); //促销具体方案的idd

    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbLogin& tbLogin = pstSession->m_stUserInfo.m_tbLogin;
    TbPlayer& tbPlayer = pstSession->m_stUserInfo.m_tbPlayer;

    TFLOAT32 fWhiteWeekMaxIap = 0.0;
    TBOOL bIsWhiteUser = CIAPWhiteList::bIsWhitePlayer(tbPlayer.m_nUid, fWhiteWeekMaxIap);
    TFLOAT32 fIapNum = CCommonLogic::GetIapNumByRechargeGem(udwRechargeGem);


    // 1. 发送获取数据的请求
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        if (0 != dwRechargeType && 1 != dwRechargeType)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -1;
        }

        //白名单校验
        TFLOAT32 fWeekGemRecharge = tbLogin.m_jWeek_gem_recharge[0].asFloat();
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_GemRecharge: [uid=%u] [bIsWhiteUser=%u fWeekGemRecharge=%f fIapNum=%f fWhiteWeekMaxIap=%f ] [seq=%u]",
            tbPlayer.m_nUid, bIsWhiteUser, fWeekGemRecharge, fIapNum, fWhiteWeekMaxIap, pstSession->m_udwSeqNo));


        if (bIsWhiteUser && fWeekGemRecharge + fIapNum >= fWhiteWeekMaxIap)
        {
            TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("ProcessCmd_GemRecharge:user is white user and his over week iap num just return [uid=%u max_week_iap=%f cur_week_iap=%f gem_num=%u gem_iap=%f] [seq=%u]",
                tbPlayer.m_nUid,
                fWhiteWeekMaxIap,
                fWeekGemRecharge,
                udwRechargeGem,
                fIapNum,
                pstSession->m_stUserInfo.m_udwBSeqNo));
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__IAP_WHITE_MAX;
            return -1;
        }

        if (1 == dwRechargeType && 100 != udwRechargeGem) //reward 100gem不给联盟gift
        {
            tbLogin.Set_Gem(tbLogin.m_nGem + udwRechargeGem);
            return 0;
        }

        // wave@20140122: 设定充值的默认标记值
        pstSession->m_ucFakeRecharge = TRUE;

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        /*
        if (0 == dwRechargeType)
        {
            Json::FastWriter writer;
            writer.omitEndingLineFeed();
            CJsoncppSeri jSeri;
            Json::Value jTmp = Json::Value(Json::objectValue);
            jTmp["purchase_token"] = pstSession->m_stReqParam.m_szPurchaseToken;
            jTmp["package_name"] = pstSession->m_stReqParam.m_szPackageName;
            jTmp["item_id"] = pstSession->m_stReqParam.m_szItemId;
            jTmp["purchase_uid"] = pstSession->m_stReqParam.m_szPurchaseUid;
            jTmp["platform"] = pstSession->m_stReqParam.m_szPlatForm;
            jTmp["user_id"] = pstSession->m_stUserInfo.m_tbPlayer.m_nUid;
            jTmp["sid"] = pstSession->m_stUserInfo.m_tbPlayer.m_nSid;

            pstSession->m_stPurchaseReq.Reset();
            pstSession->m_stPurchaseRsp.Reset();
            if (pstSession->m_udwContentType == EN_CONTENT_TYPE__STRING)
            {
                pstSession->m_stPurchaseReq.sReqContent = writer.write(jTmp);
            }
            else
            {
                TUINT32 udwLen = 0;
                const TCHAR* szTmp = NULL;
                szTmp = jSeri.serializeToBuffer(jTmp, udwLen);
                pstSession->m_stPurchaseReq.sReqContent.resize(udwLen);
                memcpy((char*)pstSession->m_stPurchaseReq.sReqContent.c_str(), szTmp, udwLen);
            }

            // set next procedure
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__PURCHASE_CHECK;

            // send request
            bNeedResponse = TRUE;
            dwRetCode = CBaseProcedure::SendPurchaseCheckRequest(pstSession, EN_SERVICE_TYPE_PURCHASE_CHECK_REQ);
            if (dwRetCode < 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_GemRecharge: send purchase check req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                return -6;
            }
            return 0;
        }
        */
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        /*
        if (0 == dwRechargeType)
        {
            Json::Reader jReader;
            CJsoncppUnseri jUnseri;
            Json::Value jTmp;
            if (pstSession->m_stPurchaseRsp.udwResType == EN_CONTENT_TYPE__STRING)
            {
                if (jReader.parse(pstSession->m_stPurchaseRsp.sRspContent, jTmp) == FALSE)
                {
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PURCHASE_CHECK_FAIL;
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_GemRecharge: parse purchase check rsp fail [seq=%u]",
                        pstSession->m_stUserInfo.m_udwBSeqNo));
                    return -3;
                }
            }
            else
            {
                if (jUnseri.unserializeToDom(pstSession->m_stPurchaseRsp.sRspContent.c_str(), jTmp, UNSERI_MODE_REPLACE) != 0)
                {
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PURCHASE_CHECK_FAIL;
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_GemRecharge: parse purchase check rsp fail [seq=%u]",
                        pstSession->m_stUserInfo.m_udwBSeqNo));
                    return -3;
                }
            }

            if (!jTmp.isMember("ret_code") || jTmp["ret_code"].asInt() != 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PURCHASE_CHECK_FAIL;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_GemRecharge: purchase check failed[errmsg=%s]. [seq=%u]",
                    jTmp["error_msg"].asString().c_str(), pstSession->m_stUserInfo.m_udwBSeqNo));
                return -3;
            }

            pstSession->m_stReqParam.m_ucIsSandBox = jTmp["sandbox"].asInt();
        }
        */
        /*
        if(0 == dwRechargeType)
        {
        TCHAR szPromoteData[MAX_JSON_LEN];
        TCHAR szUrl[1024];
        memset(szPromoteData, 0, sizeof(szPromoteData));
        memset(szUrl, 0, sizeof(szUrl));
        sprintf(szUrl, "%s?request=command=gem_recharge&sid=%u&uid=%u&instant_id=%ld&project_id=%ld&tran_id=%s&timestamp=%u&gems=%u&is_max_attack=%d", CConfBase::GetString("iap_url_pre", "serv_url").c_str(),
        pstSession->m_stReqParam.m_udwSvrId, pstSession->m_stReqParam.m_udwUserId, ddwInstantId, ddwProjectId,
        sTransId.c_str(), pstSession->m_stReqParam.m_udwInReqTime, udwRechargeGem, CItemLogic::HasTrialItemOrUnlock(pstUser));

        TSE_LOG_INFO(pstSession->m_poServLog, ("Url:%s [seq=%u]", szUrl, pstSession->m_stUserInfo.m_udwBSeqNo));

        if(CURLE_OK != CToolBase::ResFromUrl(szUrl, szPromoteData))
        {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GET_IAP_FAIL;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("get IAPDesc failed. [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
        return -2;
        }

        for(TUINT32 udwIdx = 0; udwIdx < strlen(szPromoteData); ++udwIdx)
        {
        if(szPromoteData[udwIdx] == '\n')
        {
        szPromoteData[udwIdx] = ' ';
        }
        }

        Json::Reader reader;
        pstSession->m_JsonValue.clear();
        if(FALSE == reader.parse(szPromoteData, pstSession->m_JsonValue))
        {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GET_IAP_FAIL;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("parse IAPDesc failed. [seq=%u]", pstSession->m_udwSeqNo));
        TSE_LOG_ERROR(pstSession->m_poServLog, ("IapDesc: %s [seq=%u]", szPromoteData, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -3;
        }
        if(!pstSession->m_JsonValue.isMember("res_data")
        || !pstSession->m_JsonValue["res_data"].isArray()
        || !pstSession->m_JsonValue["res_data"][0u].isMember("data")
        || !pstSession->m_JsonValue["res_data"][0u]["data"].isArray()
        || !pstSession->m_JsonValue["res_data"][0u]["data"][0u].isMember("data")
        || !pstSession->m_JsonValue["res_data"][0u]["data"][0u]["data"].isMember("recharge"))
        {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GET_IAP_FAIL;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("IapDesc error.No gem_recharge![seq=%u]", pstSession->m_udwSeqNo));
        TSE_LOG_ERROR(pstSession->m_poServLog, ("IapDesc: %s [seq=%u]", szPromoteData, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -4;
        }
        if(!pstSession->m_JsonValue.isMember("res_data")
        || !pstSession->m_JsonValue["res_data"].isArray()
        || !pstSession->m_JsonValue["res_data"][1u].isMember("data")
        || !pstSession->m_JsonValue["res_data"][1u]["data"].isArray()
        || !pstSession->m_JsonValue["res_data"][1u]["data"][0u].isMember("data")
        || !pstSession->m_JsonValue["res_data"][1u]["data"][0u]["data"].isMember("promote"))
        {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GET_IAP_FAIL;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("IapDesc error.No iap_promote![seq=%u]", pstSession->m_udwSeqNo));
        TSE_LOG_ERROR(pstSession->m_poServLog, ("IapDesc: %s [seq=%u]", szPromoteData, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -5;
        }
        pstSession->m_JsonValue["res_data"][0u]["data"][0u]["data"]["promote"] = pstSession->m_JsonValue["res_data"][1u]["data"][0u]["data"]["promote"];
        pstSession->m_JsonValue = pstSession->m_JsonValue["res_data"][0u]["data"][0u]["data"];
        }
        */
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        if (0 == dwRechargeType)
        {
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__IAP_SVR;
            IapSvrReqInfo* pstReq = new IapSvrReqInfo;
            pstReq->m_udwType = EN_HU_REQUEST_TYPE__RECHARGE;
            Json::Value rIapReqJson = Json::Value(Json::objectValue);

            rIapReqJson["sid"] = pstUser->m_tbLogin.m_nSid;
            rIapReqJson["uid"] = pstUser->m_tbPlayer.m_nUid;
            rIapReqJson["aid"] = pstUser->m_tbPlayer.m_nAlpos == 0 ? 0 : pstUser->m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET;
            rIapReqJson["instant_id"] = ddwInstantId;
            rIapReqJson["tran_id"] = sTransId;
            rIapReqJson["pid"] = ddwProjectId;
            rIapReqJson["gems"] = udwRechargeGem;

            Json::FastWriter rEventWriter;
            rEventWriter.omitEndingLineFeed();
            pstReq->m_sReqContent = rEventWriter.write(rIapReqJson);
            pstSession->m_vecIapSvrReq.push_back(pstReq);

            TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_GemRecharge: iap svr req: [json=%s] [type=%u] [uid=%ld][seq=%u]",
                pstReq->m_sReqContent.c_str(), \
                pstReq->m_udwType, \
                pstUser->m_tbPlayer.m_nUid, \
                pstSession->m_udwSeqNo));

            bNeedResponse = TRUE;
            TINT32 dwRetCode = CBaseProcedure::SendIapSvrRequest(pstSession);
            if (dwRetCode == 0)
            {
                return 0;
            }
            else
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_IAP_SERVER_REQ_ERR;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_GemRecharge: send request to iap svr failed. [json=%s] [ret=%d] [seq=%u]",
                    pstReq->m_sReqContent.c_str(), dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
                return -3;
            }
        }
    }


    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;
        if (0 == dwRechargeType)
        {
            vector<IapSvrRspInfo*>& vecRsp = pstSession->m_vecIapSvrRsp;
            IapSvrRspInfo *pstIapSvrRsp = NULL;
            if (vecRsp.size() > 0)
            {
                Json::Reader jsonReader;
                Json::Value oRspDataJson;
                pstIapSvrRsp = vecRsp[0];
                if (pstIapSvrRsp->m_udwType == EN_HU_REQUEST_TYPE__RECHARGE)
                {
                    TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_GemRecharge: iap svr get rsp: [json=%s] [uid=%ld][seq=%u]",
                        pstIapSvrRsp->m_sRspJson.c_str(), pstUser->m_tbPlayer.m_nUid, pstSession->m_udwSeqNo));

                    if (FALSE == jsonReader.parse(pstIapSvrRsp->m_sRspJson, pstSession->m_JsonValue))
                    {
                        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_IAP_SERVER_PACKAGE_ERR;
                        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_GemRecharge: prase rsp from iap svr failed. [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                        return -6;
                    }
                }
                else
                {
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GET_IAP_FAIL;
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_GemRecharge: m_udwType[%u] error.[seq=%u]", pstIapSvrRsp->m_udwType, pstSession->m_udwSeqNo));
                    return -5;
                }

                if (!pstSession->m_JsonValue.isMember("recharge"))
                {
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GET_IAP_FAIL;
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_GemRecharge: IapDesc error.No gem_recharge![seq=%u]", pstSession->m_udwSeqNo));
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_GemRecharge: IapDesc: %s [seq=%u]", pstIapSvrRsp->m_sRspJson.c_str(), pstSession->m_stUserInfo.m_udwBSeqNo));
                    return -5;
                }
                if (!pstSession->m_JsonValue.isMember("promote"))
                {
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GET_IAP_FAIL;
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_GemRecharge: IapDesc error.No iap_promote![seq=%u]", pstSession->m_udwSeqNo));
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_GemRecharge: IapDesc: %s [seq=%u]", pstIapSvrRsp->m_sRspJson.c_str(), pstSession->m_stUserInfo.m_udwBSeqNo));
                    return -5;
                }
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessReqCommand_GemRecharge: IapDesc: %s [seq=%u]", pstIapSvrRsp->m_sRspJson.c_str(), pstSession->m_stUserInfo.m_udwBSeqNo));
            }
            else
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GET_IAP_FAIL;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_GemRecharge: vecRsp.size() error.[seq=%u]", pstSession->m_udwSeqNo));
                return -5;
            }
        }
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        // buy
        pstSession->ResetAwsInfo();
        TbIap_transaction tbIapTrans;
        tbIapTrans.Set_Tid(sTransId);
        tbIapTrans.Set_Sid(pstSession->m_stReqParam.m_udwSvrId);
        tbIapTrans.Set_Uid(tbLogin.m_nUid);
        tbIapTrans.Set_Gem(udwRechargeGem);
        tbIapTrans.Set_Time(CTimeUtils::GetUnixTime());
        tbIapTrans.Set_Sandbox(pstSession->m_stReqParam.m_ucIsSandBox);
        ExpectedItem expect_item;
        ExpectedDesc expect_desc;
        expect_item.SetVal(TbIAP_TRANSACTION_FIELD_TID, false); //不存在才update
        expect_desc.push_back(expect_item);
        CAwsRequest::UpdateItem(pstSession, &tbIapTrans,  expect_desc, RETURN_VALUES_ALL_NEW);

        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__4;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_GemRecharge: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -5;
        }

        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__4)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;

        TbIap_transaction tbIapTrans;
        dwRetCode = CAwsResponse::OnUpdateItemRsp(*pstSession->m_vecAwsRsp[0], &tbIapTrans);

        //@rock 2014/01/02 2==dwRechargeType,不需要校验update是否成功
        TBOOL bNeedCheckTrans = TRUE;
        if((2 == dwRechargeType) &&
            (("" == sTransId) || ("0" == sTransId)))
        {
            bNeedCheckTrans = FALSE;
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessReqCommand_GemRecharge:Not need to check trans_id.[type=%d][id=%s][seq=%u]",
                dwRechargeType, sTransId.c_str(), pstSession->m_stUserInfo.m_udwBSeqNo));
        }

        if(bNeedCheckTrans && (dwRetCode <= 0)) //没update成功,说明记录已经存在
        {
            //打log 直接return
            strcpy(pstSession->m_stReqParam.m_szKey[7], "1");
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_GemRecharge: UpdateItem falid. sTransId:%s already exist!! [seq=%u]", sTransId.c_str(), pstSession->m_stUserInfo.m_udwBSeqNo));
            return 0;
        }

        if(-1 == ddwInstantId)
        {
            pstSession->m_dwPromoteTpye = -1;
        }
        if(0 == dwRechargeType)
        {
            //判断是否有reward
            if(pstSession->m_JsonValue["recharge"].isNull())
            {
                TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("ProcessReqCommand_GemRecharge:no iap reward [seq=%u]", pstSession->m_udwSeqNo));
                pstSession->m_dwPromoteTpye = -1;
            }
            else
            {
                dwRetCode = CProcessAccount::AddPromoteReward(pstSession, pstSession->m_JsonValue["recharge"]["bonus"]);
                if(dwRetCode < 0)
                {
                    TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("ProcessReqCommand_GemRecharge: add promote reward failed [ret=%d] %s. [seq=%u]",
                        dwRetCode, pstSession->m_udwSeqNo));
                    pstSession->m_stUserInfo.m_szIapDesc.clear();
                    return -6;
                }
                pstSession->m_ucPromoteReward = 1;
                pstSession->m_dwPromoteTpye = pstSession->m_JsonValue["recharge"]["type"].asInt();
            }
        }
        // 透传promote信息
        Json::Value jIapPromote = Json::Value(Json::objectValue);
        jIapPromote["op_iap_promote"] = Json::Value(Json::objectValue);
        jIapPromote["op_iap_promote"]["promote"] = pstSession->m_JsonValue["promote"];
        pstSession->m_JsonValue = jIapPromote;
        pstSession->m_bHasPromoteData = TRUE;

        tbLogin.Set_Gem_seq(tbLogin.m_nGem_seq + 1);
        tbLogin.Set_Gem(tbLogin.m_nGem + udwRechargeGem);

        if(tbLogin.m_nGem_buy == 0 && udwRechargeGem != 0)
        {
            CMsgBase::SendEncourageMail(&pstSession->m_stUserInfo.m_tbUserStat, pstSession->m_stReqParam.m_udwSvrId, EN_MAIL_ID__FIRST_IAP);
        }

        TINT64 ddwGemAdd = pstSession->m_stUserInfo.m_tbLogin.m_nGem - pstSession->m_ddwGemBegin;
        TFLOAT32 fDollar = CCommonLogic::GetIapNumByRechargeGem(udwRechargeGem);
        CMsgBase::SendOperateMail(tbPlayer.m_nUid,
            EN_MAIL_ID__DELAY_PURCHASE,
            pstSession->m_stReqParam.m_udwSvrId,
            SYSTEM_ENCOURAGE,
            CCommonFunc::NumToString(ddwGemAdd).c_str(),
            CCommonFunc::NumToString(fDollar).c_str(),
            "");

        tbLogin.Set_Gem_buy(tbLogin.m_nGem_buy + udwRechargeGem);
        pstSession->m_udwGembuy = udwRechargeGem;

        if(bIsWhiteUser)
        {
            Json::Value jsonWeekGemRecharge;
            jsonWeekGemRecharge = Json::Value(Json::arrayValue);
            jsonWeekGemRecharge.append(tbLogin.m_jWeek_gem_recharge[0].asFloat() + fIapNum);
            tbLogin.Set_Week_gem_recharge(jsonWeekGemRecharge);
        }

        //wave@20130122: 真正充值标记
        pstSession->m_ucFakeRecharge = FALSE;
        //生成一条iap gift记录
        /* hu不发联盟礼物
        if(tbPlayer.m_nAlid > 0 && tbPlayer.m_nAlpos > EN_ALLIANCE_POS__REQUEST && 100 != udwRechargeGem)
        {
            TINT32 dwRetCode = CCommonLogic::GenIapAlGift(&pstSession->m_stUserInfo.m_tbAlliance, udwRechargeGem,
                tbLogin.m_nUid, pstSession->m_stUserInfo.m_stAlGifts, &pstSession->m_stUserInfo.m_tbLogin);
            if(dwRetCode != 0)
            {
                TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("ProcessCmd_GemRecharge: gen iap al_gift error[retcode=%d] [seq=%u]",
                    dwRetCode, pstSession->m_udwSeqNo));
            }
            SNoticInfo stNoticInfo;
            stNoticInfo.Reset();
            stNoticInfo.SetValue(EN_NOTI_ID__AL_GIFT,
                "", "",
                0, 0,
                0, 0,
                0, "", 0);
            CMsgBase::SendNotificationAlliance(CConfBase::GetString("tbxml_project"), pstSession->m_stReqParam.m_udwSvrId, tbPlayer.m_nUid, tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET, stNoticInfo);
        }
        */
        return 0;
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessOperate::ProcessCmd_AddPersonAlGift(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwPackId = atoi(pstSession->m_stReqParam.m_szKey[0]);

    if(pstSession->m_stUserInfo.m_tbPlayer.m_nAlid == 0 ||
        pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos == EN_ALLIANCE_POS__REQUEST ||
        pstSession->m_stUserInfo.m_tbAlliance.m_nAid == 0)
    {
        pstSession->m_ucAddPersonAlGiftStatus = 1;
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_AddPersonAlGift: player not in alliance [seq=%u]",
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return 0;
    }

    TINT32 dwRetCode = CCommonLogic::GenAlGiftToPerson(&pstSession->m_stUserInfo.m_tbPlayer,
        dwPackId, pstSession->m_stUserInfo.m_stAlGifts, &pstSession->m_stUserInfo.m_tbLogin);
    if(dwRetCode == -1)
    {
        pstSession->m_ucAddPersonAlGiftStatus = 2;
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("CProcessOpAddPersonAlGift: no pack id[seq=%u]", pstSession->m_udwSeqNo));
        return -1;
    }
    if(dwRetCode == -2)
    {
        pstSession->m_ucAddPersonAlGiftStatus = 3;
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("CProcessOpAddPersonAlGift: no reward level[seq=%u]", pstSession->m_udwSeqNo));
        return -2;
    }

    pstSession->m_ucAddPersonAlGiftStatus = 4;

    return 0;
}

TINT32 CProcessOperate::ProcessCmd_AddAllianceAlGift( SSession *pstSession, TBOOL &bNeedResponse )
{
    TINT32 dwPackId = atoi(pstSession->m_stReqParam.m_szKey[0]);

    if(pstSession->m_stUserInfo.m_tbPlayer.m_nAlid == 0 ||
        pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos == EN_ALLIANCE_POS__REQUEST ||
        pstSession->m_stUserInfo.m_tbAlliance.m_nAid == 0)
    {
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_AddAllianceAlGift: player not in alliance [seq=%u]",
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return 0;
    }

    TINT32 dwRetCode = CCommonLogic::GenAlGiftToAlliance(pstSession->m_stUserInfo.m_tbAlliance.m_nAid, dwPackId, 
        pstSession->m_stUserInfo.m_tbPlayer.m_nUid, pstSession->m_stUserInfo.m_stAlGifts, &pstSession->m_stUserInfo.m_tbLogin);
    if(0 != dwRetCode)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_AddAlRewardList: pack id[%d] get reward error [retcode=%d] [seq=%u]", dwRetCode, dwPackId, pstSession->m_udwSeqNo));
        return -3;
    }
    
    return 0;
}

TINT32 CProcessOperate::ProcessCmd_RecallTest(SSession *pstSession, TBOOL &bNeedResponse)
{
    string strNoticContent = "you got an alliance gift!";
    ostringstream oss;
    oss.str("");
    oss << pstSession->m_stReqParam.m_udwSvrId;
    string strSid = oss.str();
    oss.str("");
    oss << pstSession->m_stReqParam.m_udwUserId;
    string strUid = oss.str();

    string strMsg = "./player_warning.sh "
        + strSid + " "
        + strUid + " "
        + "\"" + strNoticContent + "\"";


    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("[strMsg=%s]", \
        strMsg.c_str()));

    CMsgBase::SendDelaySystemMsg(strMsg.c_str());

    return 0;

}


TINT32 CProcessOperate::ProcessCmd_UserUpdate(SSession *pstSession, TBOOL &bNeedResponse)
{
    return 0;
}


TINT32 CProcessOperate::ProcessCmd_PlayerName(SSession *pstSession, TBOOL &bNeedResponse)
{
    //在使用此OP命令时，设置key1=1
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        pstSession->m_stUserInfo.m_tbLogin.Set_Gem(pstSession->m_stUserInfo.m_tbLogin.m_nGem + 1);
    }
    return CProcessPlayer::ProcessCmd_ChangePlayerName(pstSession, bNeedResponse);
}


TINT32 CProcessOperate::ProcessCmd_CityName(SSession *pstSession, TBOOL &bNeedResponse)
{
    //在使用此OP命令时，设置key1=1
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        pstSession->m_stUserInfo.m_tbLogin.Set_Gem(pstSession->m_stUserInfo.m_tbLogin.m_nGem + 1);
    }
    return CProcessPlayer::ProcessCmd_ChangeBaseName(pstSession, bNeedResponse);
}


TINT32 CProcessOperate::ProcessCmd_AddTroop(SSession *pstSession, TBOOL &bNeedResponse)
{
    TUINT32 udwCid = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TINT32 udwTroopId = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TINT32 udwTroopNum = atoi(pstSession->m_stReqParam.m_szKey[2]);
    TINT32 dwAddGainMightType = atoi(pstSession->m_stReqParam.m_szKey[3]);
    //TINT32 dwAddTroopType = atoi(pstSession->m_stReqParam.m_szKey[4]); // 0-正常练兵 1-治疗兵 PS:满足运营需求。。无相关操作。。只用做标志位
    CGameInfo *poGameInfo = CGameInfo::GetInstance();

    if(udwCid == 0 || udwTroopNum <= 0 || udwTroopId < 0 || udwTroopId >= EN_TROOP_TYPE__END)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }

    SCityInfo *pstCity = &pstSession->m_stUserInfo.m_stCityInfo;

    // add troop
    pstCity->m_stTblData.m_bTroop[0].m_addwNum[udwTroopId] += udwTroopNum;
    pstCity->m_stTblData.SetFlag(TbCITY_FIELD_TROOP);

    if(dwAddGainMightType == 0)
    {
        pstSession->m_stUserInfo.m_tbPlayer.m_nMgain += udwTroopNum *
            poGameInfo->m_oJsonRoot["data"]["troop"][udwTroopId]["a"]["a9"].asUInt();
        pstSession->m_stUserInfo.m_tbPlayer.SetFlag(TbPLAYER_FIELD_MGAIN);
    }

    return 0;
}



TINT32 CProcessOperate::ProcessCmd_AddFort(SSession *pstSession, TBOOL &bNeedResponse)
{
    TUINT32 udwCid = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TINT32 udwFortId = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TINT32 udwFortNum = atoi(pstSession->m_stReqParam.m_szKey[2]);
    TINT32 dwAddGainMightType = atoi(pstSession->m_stReqParam.m_szKey[3]);
    CGameInfo *poGameInfo = CGameInfo::GetInstance();

    if(udwCid == 0 || udwFortNum <= 0 || udwFortId < 0 || udwFortId >= EN_TROOP_TYPE__END)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }

    SCityInfo *pstCity = &pstSession->m_stUserInfo.m_stCityInfo;
    if(pstCity == NULL)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -2;
    }

    // add fort
    pstCity->m_stTblData.m_bFort[0].m_addwNum[udwFortId] += udwFortNum;
    pstCity->m_stTblData.SetFlag(TbCITY_FIELD_FORT);

    if(dwAddGainMightType == 0)
    {
        pstSession->m_stUserInfo.m_tbPlayer.m_nMgain += udwFortNum *
            poGameInfo->m_oJsonRoot["data"]["fort"][udwFortId]["a"]["a9"].asUInt();
        pstSession->m_stUserInfo.m_tbPlayer.SetFlag(TbPLAYER_FIELD_MGAIN);
    }

    return 0;
}


TINT32 CProcessOperate::ProcessCmd_ClearUserResource(SSession *pstSession, TBOOL &bNeedResponse)
{
    //TUINT32 udwCityId = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TINT32 dwResourceType = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TUINT32 udwRate = atoi(pstSession->m_stReqParam.m_szKey[2]);
    SCityInfo *pstCity = &pstSession->m_stUserInfo.m_stCityInfo;

    if(pstSession->m_stUserInfo.m_tbLogin.m_nNpc != pstSession->m_stReqParam.m_ucIsNpc)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__ClearUserResource: npc type not right [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }

    if(dwResourceType >= 0 && dwResourceType < EN_RESOURCE_TYPE__END)
    {
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd__ClearUserResource: clear resource [seq=%u]", pstSession->m_udwSeqNo));
        pstCity->m_stTblData.m_bResource[0].m_addwNum[dwResourceType] = pstCity->m_stTblData.m_bResource[0].m_addwNum[dwResourceType] * udwRate / 100;
        pstCity->m_stTblData.SetFlag(TbCITY_FIELD_RESOURCE);
    }

    return 0;
}



TINT32 CProcessOperate::ProcessCmd_AddResource(SSession *pstSession, TBOOL &bNeedResponse)
{
    TUINT8 ucType = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TUINT32 udwNum = strtoul(pstSession->m_stReqParam.m_szKey[1], NULL, 10);
    //TUINT32 udwCityId = strtoul(pstSession->m_stReqParam.m_szKey[2], NULL, 10);

    SCityInfo *pstCity = &pstSession->m_stUserInfo.m_stCityInfo;
    if(ucType >= EN_RESOURCE_TYPE__END)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }

    // set res
    pstCity->m_stTblData.m_bResource[0].m_addwNum[ucType] += udwNum;
    if(pstCity->m_stTblData.m_bResource[0].m_addwNum[ucType] > MAX_RESOURCE_LIMIT_NUM)
    {
        pstCity->m_stTblData.m_bResource[0].m_addwNum[ucType] = MAX_RESOURCE_LIMIT_NUM;
    }
    pstCity->m_stTblData.SetFlag(TbCITY_FIELD_RESOURCE);

    return 0;
}


TINT32 CProcessOperate::ProcessCmd_GemAdd(SSession *pstSession, TBOOL &bNeedResponse)
{
    TUINT8 ucGemAddType = atoi(pstSession->m_stReqParam.m_szKey[0]); // 0 正常充值，seq要增加的，等同于购买，1表示赠送
    TUINT32 udwRechargeGem = strtoul(pstSession->m_stReqParam.m_szKey[1], NULL, 10);

    TINT32 dwRetCode = 0;
    TbLogin *pstAccount = &pstSession->m_stUserInfo.m_tbLogin;

    switch(ucGemAddType)
    {
    case 0:
        pstAccount->Set_Gem_seq(pstAccount->m_nGem_seq + 1);
        pstAccount->Set_Gem(pstAccount->m_nGem + udwRechargeGem);

        if(pstAccount->m_nGem_buy == 0 && udwRechargeGem != 0)
        {
            //CMsgBase::SendOperateMail(pstPlayer->m_nId, ,pstSession->m_stReqParam.m_udwSvrId, 0, EN_MAIL_SPECIAL__FIRST_GEM_RECHARGE);
        }

        pstAccount->Set_Gem_buy(pstAccount->m_nGem_buy + udwRechargeGem);
        break;
    case 1://reward
        pstAccount->Set_Gem(pstAccount->m_nGem + udwRechargeGem);
        break;
    case 2:
        pstAccount->Set_Gem_seq(pstAccount->m_nGem_seq + 1);
        pstAccount->Set_Gem(pstAccount->m_nGem + udwRechargeGem);

        if(pstAccount->m_nGem_buy == 0 && udwRechargeGem != 0)
        {
            //CSendMessageBase::SendMail(pstPlayer->m_nId, pstSession->m_stReqParam.m_udwSvrId, 0, EN_MAIL_SPECIAL__FIRST_GEM_RECHARGE);
        }

        pstAccount->Set_Gem_buy(pstAccount->m_nGem_buy + udwRechargeGem);

        dwRetCode = CCommonLogic::GenIapAlGift(&pstSession->m_stUserInfo.m_tbAlliance, udwRechargeGem,
            pstAccount->m_nUid, pstSession->m_stUserInfo.m_stAlGifts, &pstSession->m_stUserInfo.m_tbLogin);
        if(dwRetCode != 0)
        {
            TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("ProcessCmd_GemAdd: gen iap al_gift error[retcode=%d] [seq=%u]",
                dwRetCode, pstSession->m_udwSeqNo));
        }
        dwRetCode = 0;
        break;
    default:
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        dwRetCode = -2;
        break;
    }

    return dwRetCode;
}


TINT32 CProcessOperate::ProcessCmd_ItemAdd(SSession *pstSession, TBOOL &bNeedResponse)
{
    TUINT32 udwItemId = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TINT32 dwItemNum = atoi(pstSession->m_stReqParam.m_szKey[1]);
    SUserInfo* pstUser = &pstSession->m_stUserInfo;

    if(dwItemNum == 0 || udwItemId >= EN_ITEM_ID__END)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }

    if(dwItemNum > 0)
    {
        CItemBase::AddItem(&pstUser->m_tbBackpack, udwItemId, dwItemNum);
    }
    else
    {
        CItemBase::CostItem(&pstUser->m_tbBackpack, udwItemId, -1*dwItemNum);
    }   

    return 0;
}



TINT32 CProcessOperate::ProcessCmd_ItemSet(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwItemType = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TINT64 ddwItemNum = strtoll(pstSession->m_stReqParam.m_szKey[1], NULL, 10);

    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbBackpack *ptbBackPack = &pstUser->m_tbBackpack;
    TUINT32 udwIdx = 0;

    TSE_LOG_INFO(pstSession->m_poServLog, ("Processcmd_SetItem: start [dwItemType=%d] [ddwItemNum=%ld] [seq=%u]", \
        dwItemType, \
        ddwItemNum, \
        pstSession->m_stUserInfo.m_udwBSeqNo));

    if(EN_ITEM_ID__PORTAL_OF_REFUGE > dwItemType
        || EN_ITEM_ID__END < dwItemType
        || 0 > ddwItemNum)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_SetItem: parameter is illegal [seq=%u]", \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }

    for(udwIdx = 0; udwIdx < ptbBackPack->m_bItem.m_udwNum; ++udwIdx)  //lucien note:item会是一直连续排列的吗？
    {
        if(ptbBackPack->m_bItem[udwIdx].m_ddwItemId == static_cast<TUINT32>(dwItemType))
        {
            ptbBackPack->m_bItem[udwIdx].m_ddwItemNum = ddwItemNum;
            ptbBackPack->SetFlag(TbBACKPACK_FIELD_ITEM);
            break;
        }
    }
    if(udwIdx == ptbBackPack->m_bItem.m_udwNum && ptbBackPack->m_bItem.m_udwNum < MAX_ITEM_TYPE_NUM)
    {
        ptbBackPack->m_bItem[ptbBackPack->m_bItem.m_udwNum].m_ddwItemId = dwItemType;
        ptbBackPack->m_bItem[ptbBackPack->m_bItem.m_udwNum].m_ddwItemNum = ddwItemNum;
        ptbBackPack->m_bItem.m_udwNum++;
        ptbBackPack->SetFlag(TbBACKPACK_FIELD_ITEM);
    }

    TSE_LOG_INFO(pstSession->m_poServLog, ("Processcmd_SetItem: end, [total item num=%u] [seq=%u]", \
        ptbBackPack->m_bItem.m_udwNum, pstSession->m_stUserInfo.m_udwBSeqNo));
    return 0;
}


TINT32 CProcessOperate::ProcessCmd_AllianceMemberGet(SSession *pstSession, TBOOL &bNeedResponse)
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

    // 1. 获取联盟成员的uid——请求
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        //set next step
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        //get request
        pstSession->ResetAwsReq();

        CAwsRequest::AllAlMemberQuery(pstSession, udwAllianceId);

        //send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AllianceMemberGet: send step init req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }
        return 0;
    }

    // 2. 解析member的uid并发送获取member player表的请求
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        vector<TUINT32> vecUid;

        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        // 解析回包，获得成员uid
        dwRetCode = CAwsResponse::OnQueryRsp(*pstSession->m_vecAwsRsp[0], pstSession->m_atbTmpAlmember, sizeof(TbAl_member), 1000/*MAX_ALLIANCE_MEMBER_NUM*/);
        if(dwRetCode > 0)
        {
            for(TINT32 idx = 0; idx < dwRetCode; idx++)
            {
                vecUid.push_back(pstSession->m_atbTmpAlmember[idx].m_nUid);
            }
            pstSession->m_udwTmpAlmemberNum = dwRetCode;
        }

        // 由拿到的成员uid获取成员player表的信息——请求
        // get request
        pstSession->ResetAwsReq();
        TbPlayer tbPlayerItem;
        for(TUINT32 idx = 0; idx < vecUid.size(); idx++)
        {
            tbPlayerItem.Reset();
            tbPlayerItem.Set_Uid(vecUid.at(idx));
            CAwsRequest::GetItem(pstSession, &tbPlayerItem,ETbPLAYER_OPEN_TYPE_PRIMARY);
        }

        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AllianceMemberGet: send step1 req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }

        return 0;
    }

    // 2. 解析获取的数据
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        // set next procedure
        pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_PLAYER_LIST;
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;

        // get data info
        TUINT32 udwReturnMemberNum = 0;
        for(TUINT32 idx = 0; idx < pstSession->m_vecAwsRsp.size(); idx++)
        {
            AwsRspInfo *pstAwsRspInfo = pstSession->m_vecAwsRsp[idx];
            dwRetCode = CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, pstSession->m_stUserInfo.m_atbAllianceMember);
            if(dwRetCode > 0)
            {
                udwReturnMemberNum += dwRetCode;
            }
        }

        //process data
        if(udwReturnMemberNum > 0)
        {
            // init
            for(TUINT32 idy = 0; idy < udwReturnMemberNum; idy++)
            {
                pstSession->m_stUserInfo.m_ptbAllianceMember[idy] = &pstSession->m_stUserInfo.m_atbAllianceMember[idy];
            }
            // sort
            std::sort(pstSession->m_stUserInfo.m_ptbAllianceMember,
                pstSession->m_stUserInfo.m_ptbAllianceMember + udwReturnMemberNum, TbPlayer_Compare);

            //取消分页支持, 每次都全量返回
            pstSession->m_stUserInfo.m_udwCurFindMemberNum = udwReturnMemberNum;
            for(TUINT32 idy = 0; idy < udwReturnMemberNum; idy++)
            {
                pstSession->m_stUserInfo.m_ptbOutputMember[idy] = &pstSession->m_stUserInfo.m_atbAllianceMember[idy];
            }

            if(udwAllianceId == pstSession->m_stUserInfo.m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET)
            {
                // wave@20130809: 解决pos_change时，用户数据在member-get之后更新，导致member中数据不是最新数据的问题
                for(TUINT32 idy = 0; idy < pstSession->m_stUserInfo.m_udwCurFindMemberNum; idy++)
                {
                    if(pstSession->m_stUserInfo.m_ptbOutputMember[idy]->m_nUid == pstSession->m_stUserInfo.m_tbPlayer.m_nUid)
                    {
                        pstSession->m_stUserInfo.m_ptbOutputMember[idy]->m_nAlpos = pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos;
                        pstSession->m_stUserInfo.m_ptbOutputMember[idy]->m_nAlid = pstSession->m_stUserInfo.m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET;
                        pstSession->m_stUserInfo.m_ptbOutputMember[idy]->m_sAlname = pstSession->m_stUserInfo.m_tbPlayer.m_sAlname;
                        break;
                    }
                }

                // 修正联盟兵力
                if(udwReturnMemberNum != pstSession->m_stUserInfo.m_tbAlliance.m_nMember)
                {
                    pstSession->m_stUserInfo.m_tbAlliance.Set_Member(udwReturnMemberNum);
                    pstSession->m_stUserInfo.m_ucAllianceFlag = EN_TABLE_UPDT_FLAG__CHANGE;
                }
                TINT64 ddwAllMight = 0;
                TINT64 ddwForceKill = 0;
                for(TUINT32 udwIdx = 0; udwIdx < udwReturnMemberNum; ++udwIdx)
                {
                    ddwAllMight += pstSession->m_stUserInfo.m_atbAllianceMember[udwIdx].m_nMight;
                    ddwForceKill += pstSession->m_stUserInfo.m_atbAllianceMember[udwIdx].m_nKfort +
                        pstSession->m_stUserInfo.m_atbAllianceMember[udwIdx].m_nKtroop;
                }

                if(0 < pstSession->m_stUserInfo.m_tbAlliance.m_nAid &&
                    ddwAllMight != pstSession->m_stUserInfo.m_tbAlliance.m_nMight)
                {
                    pstSession->m_stUserInfo.m_tbAlliance.Set_Might(ddwAllMight);
                    pstSession->m_stUserInfo.m_ucAllianceFlag = EN_TABLE_UPDT_FLAG__CHANGE;
                }
                if(0 < pstSession->m_stUserInfo.m_tbAlliance.m_nAid &&
                    ddwForceKill != pstSession->m_stUserInfo.m_tbAlliance.m_nForce_kill)
                {
                    pstSession->m_stUserInfo.m_tbAlliance.Set_Force_kill(ddwForceKill);
                    pstSession->m_stUserInfo.m_ucAllianceFlag = EN_TABLE_UPDT_FLAG__CHANGE;
                }
            }
        }
    }

    return 0;
}



TINT32 CProcessOperate::ProcessCmd_AllianceInfoGet(SSession *pstSession, TBOOL &bNeedResponse)
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

        pstSession->ResetAwsReq();
        CAwsRequest::AllianceGetByAid(pstSession, udwAlid);

        // send request
        bNeedResponse = TRUE;
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
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

TINT32 CProcessOperate::ProcessCmd_AllianceChangeChancellor(SSession *pstSession, TBOOL &bNeedResponse)
{
    TUINT32 udwAllianceId = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TUINT32 udwUserId = strtoul(pstSession->m_stReqParam.m_szKey[1], NULL, 10);

    TINT32 dwRetCode = 0;
    TUINT32 udwIdx = 0;

    TbAlliance tbAllianceItem;
    TbPlayer tbNewChancellor;

    //获取目标alliance和new chancellor——请求
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        if(udwAllianceId == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -1;
        }

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        pstSession->ResetAwsReq();
        //TbPlayer tbNewChancellor;
        tbNewChancellor.Set_Uid(udwUserId);
        CAwsRequest::GetItem(pstSession, &tbNewChancellor,ETbPLAYER_OPEN_TYPE_PRIMARY);

        CAwsRequest::AllianceGetByAid(pstSession, udwAllianceId);
        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            return -2;
        }

        return 0;
    }

    //获取目标alliance和new chancellor——解析
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        TUINT8 ucPlayerNum = 0;
        TUINT8 ucAllianceNum = 0;
        AwsRspInfo *pstRes = NULL;
        for(udwIdx = 0; udwIdx < pstSession->m_vecAwsRsp.size(); udwIdx++)
        {
            pstRes = pstSession->m_vecAwsRsp[udwIdx];
            string udwTableName = CCommonFunc::GetTableRawName(pstRes->sTableName.c_str());
            if(udwTableName == EN_AWS_TABLE_PLAYER)
            {
                dwRetCode = CAwsResponse::OnGetItemRsp(*pstRes, &tbNewChancellor);
                ucPlayerNum = dwRetCode;
                continue;
            }
            if(udwTableName == EN_AWS_TABLE_ALLIANCE)
            {
                dwRetCode = CAwsResponse::OnGetItemRsp(*pstRes, &tbAllianceItem);
                ucAllianceNum = dwRetCode;
                continue;
            }
        }
        if(1 != ucPlayerNum || 1 != ucAllianceNum)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AllianceChangeChancellor: player or alliance number error [player num=%u] [al num=%u] [seq=%u]",
                ucPlayerNum,ucAllianceNum,pstSession->m_stUserInfo.m_udwBSeqNo));
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -3;
        }
    }

    //更新目标alliance表、old chancellor的player表、new chancellor的player表、old chancellor的al member表、new chancellor的al member表——请求
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        // get package
        pstSession->ResetAwsReq();
        if(tbAllianceItem.m_nOid == udwUserId || tbNewChancellor.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET != udwAllianceId)
        {
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd__AllianceChangeChancellor: the player[%u][alid=%u] is already the chancellor or not in this alliance[%u][oid=%u]. [seq=%u]",
                tbNewChancellor.m_nUid, tbNewChancellor.m_nAlid, udwAllianceId, tbAllianceItem.m_nOid, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -4;
        }

        // updt player info
        tbNewChancellor.Set_Alid(udwAllianceId * PLAYER_ALLIANCE_ID_OFFSET + EN_ALLIANCE_POS__CHANCELLOR);
        tbNewChancellor.Set_Alpos(EN_ALLIANCE_POS__CHANCELLOR);
        CAwsRequest::UpdateItem(pstSession, &tbNewChancellor);

        TbPlayer tbOldChancellor;
        tbOldChancellor.Set_Uid(tbAllianceItem.m_nOid);
        tbOldChancellor.Set_Alpos(EN_ALLIANCE_POS__MEMBER);
        tbOldChancellor.Set_Alid(udwAllianceId * PLAYER_ALLIANCE_ID_OFFSET + EN_ALLIANCE_POS__MEMBER);
        CAwsRequest::UpdateItem(pstSession, &tbOldChancellor);

        // update al member
        TbAl_member tbOldOwner;
        tbOldOwner.Set_Aid(udwAllianceId);
        tbOldOwner.Set_Uid(tbAllianceItem.m_nOid);
        tbOldOwner.Set_Al_pos(EN_ALLIANCE_POS__MEMBER);
        CAwsRequest::UpdateItem(pstSession, &tbOldOwner);

        TbAl_member tbNewOwner;
        tbNewOwner.Set_Aid(udwAllianceId);
        tbNewOwner.Set_Uid(udwUserId);
        tbNewOwner.Set_Al_pos(EN_ALLIANCE_POS__CHANCELLOR);
        CAwsRequest::UpdateItem(pstSession, &tbNewOwner);

        // update alliance info
        tbAllianceItem.Set_Oid(tbNewChancellor.m_nUid);
        tbAllianceItem.Set_Owner_cid(tbNewChancellor.m_nCid);
        tbAllianceItem.Set_Oname(tbNewChancellor.m_sUin);
        CAwsRequest::UpdateItem(pstSession, &tbAllianceItem);

        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            return -5;
        }
        return 0;
    }

    //更新五张表——回包
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd__AllianceChangeChancellor:alliance[%u] chancellor[%u] change succ. [seq=%u]",
            udwAllianceId, udwUserId, pstSession->m_stUserInfo.m_udwBSeqNo));
    }

    return 0;
}



TINT32 CProcessOperate::ProcessCmd_ResearchLevelSet(SSession *pstSession, TBOOL &bNeedResponse)
{
    // in param
    TINT32 dwType = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TINT32 dwLevel = atoi(pstSession->m_stReqParam.m_szKey[1]);

    // check param
    if(dwType >= EN_RESEARCH_TYPE__END)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }
    if(dwLevel < 0 || dwLevel > MAX_RESEARCH_LEVEL)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -2;
    }

    // set data
    pstSession->m_stUserInfo.m_stCityInfo.m_stTblData.m_bResearch[0].m_addwLevel[dwType] = dwLevel;
    pstSession->m_stUserInfo.m_stCityInfo.m_stTblData.SetFlag(TbCITY_FIELD_RESEARCH);

    return 0;
}

TINT32 CProcessOperate::ProcessCmd_BuildingLevelSet(SSession *pstSession, TBOOL &bNeedResponse)
{
    //不考虑building的依赖吗？
    // in param
    TINT32 dwPos = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TINT32 dwBuildingType = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TINT32 dwLevel = atoi(pstSession->m_stReqParam.m_szKey[2]);
    TINT32 dwSetType = atoi(pstSession->m_stReqParam.m_szKey[3]); // 0: 判定当时pos中的建筑类型，1：直接set,不验证

    // data prepare
    TbCity& tbCity = pstSession->m_stUserInfo.m_stCityInfo.m_stTblData;

    // check param
    if(dwBuildingType >= EN_BUILDING_TYPE__END)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -2;
    }
    if(dwLevel > MAX_BUILDING_LEVEL)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -3;
    }

    if(dwSetType == 0)
    {
        SCityBuildingNode* pstNode = CCityBase::GetBuildingAtPos(&tbCity, dwPos);
        if(!pstNode || pstNode->m_ddwType != dwBuildingType)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -4;
        }
        if(dwLevel == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -5;
        }

        // set data
        pstNode->m_ddwLevel = dwLevel;
        tbCity.SetFlag(TbCITY_FIELD_BUILDING);
    }
    else
    {
        SCityBuildingNode* pstNode = CCityBase::GetBuildingAtPos(&tbCity, dwPos);
        if(pstNode)
        {
            if(dwLevel == 0)
            {
                CCityBase::DelBuildingAtPos(&tbCity, dwPos);
            }
            else//TODO 碰撞检测
            {
                pstNode->m_ddwType = dwBuildingType;
                pstNode->m_ddwLevel = dwLevel;
            }
            tbCity.SetFlag(TbCITY_FIELD_BUILDING);
        }
        else if(dwLevel > 0)
        {
            CCityBase::AddBuilding(dwPos, dwBuildingType, dwLevel, tbCity);
        }
    }

    // next procedure
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;

    return 0;
}




TINT32 CProcessOperate::ProcessCmd_IdentityChange(SSession *pstSession, TBOOL &bNeedResponse)
{
    //身份标志 key0 = 1 设为npc 为0 设为普通用户
    TINT32 dwIdentityType = atoi(pstSession->m_stReqParam.m_szKey[0]);

    if(dwIdentityType != 0)
    {
        dwIdentityType = 1;
    }

    if(dwIdentityType == pstSession->m_stUserInfo.m_tbLogin.Get_Npc())
    {
        return 0;
    }

    pstSession->m_stUserInfo.m_tbLogin.Set_Npc(dwIdentityType);

    return 0;
}

TINT32 CProcessOperate::Processcmd_AddRewardList(SSession* pstSession, TBOOL &bNeedResponse)
{
    /*
        活动---增加奖励
        key0=reward(type:id:num,type:id:num....)
        key1=event_type
        key2=reward_type(0:goal 1:rank)
        key3=goal_num or rank_num (depends on key2)
        key4=score//玩家该活动的分数
        key5=rank uname in order list (first at one,use ":" to divide and available when key2=1)
        key6=rank svr_id in order list (first at one,use ":" to divide and available when key2=1, each to key4)

    */
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        pstSession->m_udwCommandStep = EN_PROCEDURE__USER_INFO_UPDATE_REQUEST;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        TUINT32 udwEventType = atoi(pstSession->m_stReqParam.m_szKey[1]);
        TUINT32 udwRewardType = atoi(pstSession->m_stReqParam.m_szKey[2]); // 0: goal 1: rank
        TINT32 dwKey = atoi(pstSession->m_stReqParam.m_szKey[3]);
        string sRankNameList = pstSession->m_stReqParam.m_szKey[5];
        string sRankSvrList = pstSession->m_stReqParam.m_szKey[6];
        TINT32 dwPoint = atoi(pstSession->m_stReqParam.m_szKey[4]);

        string sScoreList = pstSession->m_stReqParam.m_szKey[7];
        string sEventInfo = pstSession->m_stReqParam.m_szKey[8];

        SGlobalRes stGloablres;
        stGloablres.Reset();

        std::vector<string> vecOneReard;

        CCommonFunc::GetVectorFromString(pstSession->m_stReqParam.m_szKey[0], ',', vecOneReard);

        TSE_LOG_DEBUG(pstSession->m_poServLog, ("Processcmd_AddRewardList:key=%s reward size=%u [seq=%u]", pstSession->m_stReqParam.m_szKey[0], vecOneReard.size(), pstSession->m_stUserInfo.m_udwBSeqNo));

        for(TUINT32 udwRewardIdx = 0; udwRewardIdx < vecOneReard.size(); ++udwRewardIdx)
        {
            if(stGloablres.ddwTotalNum >= MAX_REWARD_ITEM_NUM)
            {
                continue;
            }
            TUINT32 audwReward[3] = {0, 0, 0};
            TUINT32 udwNum = 3;
            CCommonFunc::GetArrayFromString(vecOneReard[udwRewardIdx].c_str(), ':', audwReward, udwNum);

            TSE_LOG_DEBUG(pstSession->m_poServLog, ("Processcmd_AddRewardList: reward size=%u idx=%u reward=%s [type=%u id=%u num=%u][seq=%u]",
                vecOneReard.size(),
                udwRewardIdx,
                vecOneReard[udwRewardIdx].c_str(),
                audwReward[0U],
                audwReward[1U],
                audwReward[2U],
                pstSession->m_stUserInfo.m_udwBSeqNo));

            stGloablres.aRewardList[stGloablres.ddwTotalNum].ddwType = audwReward[0U];
            stGloablres.aRewardList[stGloablres.ddwTotalNum].ddwId = audwReward[1U];
            stGloablres.aRewardList[stGloablres.ddwTotalNum].ddwNum = audwReward[2U];
            stGloablres.ddwTotalNum++;
        }

        //mail 
        CProcessMailReport::SendEventMail(pstSession->m_stReqParam.m_udwSvrId,
            pstSession->m_stUserInfo.m_tbPlayer.m_nUid,
            udwEventType,
            udwRewardType,
            dwKey,
            0,
            sRankNameList,
            sRankSvrList,
            FALSE,
            dwPoint,
            &stGloablres,
            CDocument::GetLang(pstSession->m_stUserInfo.m_tbLogin.m_nLang));

        //推送
        SNoticInfo stNoticInfo;
        stNoticInfo.Reset();
        stNoticInfo.SetValue(EN_NOTI_ID__EVENT_REWARD,
            "", "",
            0, 0,
            0, 0,
            0, "", 0, CDocument::GetLang(pstSession->m_stUserInfo.m_tbLogin.m_nLang));
        CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), pstSession->m_stUserInfo.m_tbLogin.m_nSid, pstSession->m_stUserInfo.m_tbLogin.m_nUid,  stNoticInfo);
        
        if(udwRewardType == 0)
        {
            //daemon TODO tips
            CSendMessageBase::AddTips(&pstSession->m_stUserInfo, EN_TIPS_TYPE__EVENT_RANK, pstSession->m_stUserInfo.m_tbPlayer.m_nUid, TRUE, udwEventType);

            CAwsRequest::PutItem(pstSession, &pstSession->m_stUserInfo.m_atbTips[pstSession->m_stUserInfo.m_udwTipsNum - 1]);

            //wave@20160122
            CSendMessageBase::AddEventTips(&pstSession->m_stUserInfo, udwEventType, udwRewardType, dwKey, dwPoint, sScoreList, stGloablres.aRewardList, stGloablres.ddwTotalNum, sEventInfo);
            CAwsRequest::PutItem(pstSession, &pstSession->m_stUserInfo.m_atbEventTips[pstSession->m_stUserInfo.m_udwEventTipsNum - 1]);
        }

        
        CAwsRequest::UpdateItem(pstSession, &pstSession->m_stUserInfo.m_tbLogin);

        if(pstSession->m_vecAwsReq.size() > 0)
        {
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
            bNeedResponse = TRUE;
            TINT32 dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
            if(dwRetCode < 0)
            {

                TSE_LOG_DEBUG(pstSession->m_poServLog, ("Processcmd_AddAlRewardList:[7] [ret=%d][seq=%u]", dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                return -4;
            }
            return 0;
        }

        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NO_ERROR;
        return -1;
        
    }
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NO_ERROR;
        return -1;
    }

    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NO_ERROR;
    return -1;
}

TINT32 CProcessOperate::Processcmd_AddAlRewardList(SSession* pstSession, TBOOL &bNeedResponse)
{
    /*
    活动---增加联盟礼物
    command=op_add_al_reward_list
    key0=reward(type:id:num,type:id:num....)
    key1=event_type
    key2=reward_type(0:goal 1:rank)
    key3=goal_num or rank_num (depends on key2)
    key4=score//玩家该活动的分数
    key5=alid
    key6=rank alname in order list (first at one,use ":" to divide and available when key2=1)
    key7=rank svr_id in order list (first at one,use ":" to divide and available when key2=1, each to key4)

    */
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        TUINT32 udwEventType = atoi(pstSession->m_stReqParam.m_szKey[1]);
        TUINT32 udwRewardType = atoi(pstSession->m_stReqParam.m_szKey[2]);
        TINT32 dwKey = atoi(pstSession->m_stReqParam.m_szKey[3]);
        string sRankNameList = pstSession->m_stReqParam.m_szKey[6];
        string sRankSvrList = pstSession->m_stReqParam.m_szKey[7];
        TINT32 dwAlid = atoi(pstSession->m_stReqParam.m_szKey[5]);
        TINT32 dwPoint = atoi(pstSession->m_stReqParam.m_szKey[4]);
        string sScoreList = pstSession->m_stReqParam.m_szKey[8];
        string sEventInfo = pstSession->m_stReqParam.m_szKey[9];

        if(pstSession->m_stUserInfo.m_tbPlayer.m_nAlid == 0 ||
            pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos == EN_ALLIANCE_POS__REQUEST ||
            pstSession->m_stUserInfo.m_tbAlliance.m_nAid == 0 ||
            dwAlid != pstSession->m_stUserInfo.m_tbAlliance.m_nAid)
        {
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("Processcmd_AddAlRewardList: player[alid:%ld] not in this alliance[%d] [seq=%u]",
                pstSession->m_stUserInfo.m_tbPlayer.m_nAlid, dwAlid, pstSession->m_stUserInfo.m_udwBSeqNo));
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NO_ERROR;

            return -2;
        }
        std::vector<string> vecReward;

        CCommonFunc::GetVectorFromString(pstSession->m_stReqParam.m_szKey[0], ',', vecReward);

        TSE_LOG_DEBUG(pstSession->m_poServLog, ("Processcmd_AddAlRewardList:key=%s reward size=%u [seq=%u]", pstSession->m_stReqParam.m_szKey[0], vecReward.size(), pstSession->m_stUserInfo.m_udwBSeqNo));

        SGlobalRes stGlobalRes;
        stGlobalRes.Reset();
        for(TUINT32 udwRewardIdx = 0; udwRewardIdx < vecReward.size(); ++udwRewardIdx)
        {
            if(stGlobalRes.ddwTotalNum >= MAX_REWARD_ITEM_NUM)
            {
                continue;
            }

            TUINT32 audwReward[3] = {0, 0, 0};
            TUINT32 udwNum = 3;
            CCommonFunc::GetArrayFromString(vecReward[udwRewardIdx].c_str(), ':', audwReward, udwNum);

            TSE_LOG_DEBUG(pstSession->m_poServLog, ("Processcmd_AddAlRewardList: reward size=%u idx=%u reward=%s [type=%u id=%u num=%u][seq=%u]",
                vecReward.size(),
                udwRewardIdx,
                vecReward[udwRewardIdx].c_str(),
                audwReward[0U],
                audwReward[1U],
                audwReward[2U],
                pstSession->m_stUserInfo.m_udwBSeqNo));

            if(audwReward[0U] == EN_GLOBALRES_TYPE_AL_GIFT)
            {
                TUINT32 i = 0;
                while(i < audwReward[2U])
                {
                    TINT32 dwRetCode = CCommonLogic::GenEventAlGift(&pstSession->m_stUserInfo.m_tbPlayer,
                        audwReward[1U], pstSession->m_stUserInfo.m_stAlGifts, audwReward[0U], &pstSession->m_stUserInfo.m_tbLogin);
                    if(0 != dwRetCode)
                    {
                        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
                        TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_AddAlRewardList: pack id[%d] get reward error [retcode=%d] [seq=%u]", dwRetCode, audwReward[1U], pstSession->m_udwSeqNo));
                        return -3;
                    }
                    i++;
                }
                stGlobalRes.aRewardList[stGlobalRes.ddwTotalNum].ddwType = audwReward[0U];
                stGlobalRes.aRewardList[stGlobalRes.ddwTotalNum].ddwId = audwReward[1U];
                stGlobalRes.aRewardList[stGlobalRes.ddwTotalNum].ddwNum = audwReward[2U];
                stGlobalRes.ddwTotalNum++;
            }
        }

        //mail 
        CProcessMailReport::SendEventMail(pstSession->m_stReqParam.m_udwSvrId,
            pstSession->m_stUserInfo.m_tbPlayer.m_nUid,
            udwEventType,
            udwRewardType,
            dwKey,
            1,
            sRankNameList,
            sRankSvrList,
            TRUE,
            dwPoint,
            NULL,
            CDocument::GetLang(pstSession->m_stUserInfo.m_tbLogin.m_nLang));

        if(udwRewardType == 0)
        {
            CSendMessageBase::AddTips(&pstSession->m_stUserInfo, EN_TIPS_TYPE__EVENT_RANK, pstSession->m_stUserInfo.m_tbPlayer.m_nUid, TRUE, udwEventType);
        }

        CSendMessageBase::AddAlEventTips(&pstSession->m_stUserInfo, dwAlid, udwEventType, udwRewardType, dwKey, dwPoint, sScoreList, &stGlobalRes, sEventInfo);
        //推送 
        SNoticInfo stNoticInfo;
        stNoticInfo.Reset();
        stNoticInfo.SetValue(EN_NOTI_ID__EVENT_REWARD,
            "", "",
            0, 0,
            0, 0,
            0, "", 0, CDocument::GetLang(pstSession->m_stUserInfo.m_tbLogin.m_nLang));
        CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), pstSession->m_stUserInfo.m_tbLogin.m_nSid, pstSession->m_stUserInfo.m_tbLogin.m_nUid, stNoticInfo);


        if(pstSession->m_vecAwsReq.size() > 0)
        {
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
            bNeedResponse = TRUE;
            TINT32 dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
            if(dwRetCode < 0)
            {

                TSE_LOG_DEBUG(pstSession->m_poServLog, ("Processcmd_AddAlRewardList:[7] [ret=%d][seq=%u]", dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                return -4;
            }
            return 0;
        }

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessOperate::Processcmd_AddRewardListNew(SSession* pstSession, TBOOL &bNeedResponse)
{
    /*
    活动---增加奖励
    key0=reward(type:id:num,type:id:num....)
    key1=event_type
    key2=reward_type(0:goal 1:rank)
    key3=goal_num or rank_num (depends on key2)
    key4=score//玩家该活动的分数
    key5=rank uname in order list (first at one,use ":" to divide and available when key2=1)
    key6=rank svr_id in order list (first at one,use ":" to divide and available when key2=1, each to key4)
    */

    TUINT32 udwEventType = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TUINT32 udwRewardType = atoi(pstSession->m_stReqParam.m_szKey[2]); // 0: goal 1: rank
    TINT32 dwKey = atoi(pstSession->m_stReqParam.m_szKey[3]);
    TINT32 dwPoint = atoi(pstSession->m_stReqParam.m_szKey[4]);
    string sRankNameList = pstSession->m_stReqParam.m_szKey[5];
    string sRankSvrList = pstSession->m_stReqParam.m_szKey[6];

    string sScoreList = pstSession->m_stReqParam.m_szKey[7];
    string sEventInfo = pstSession->m_stReqParam.m_szKey[8];
    TINT32 dwWindowOpt = atoi(pstSession->m_stReqParam.m_szKey[9]);

    TINT32 dwRetCode = 0;

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        pstSession->ResetAwsInfo();
        CAwsRequest::GetGlobalNewId(pstSession, EN_GLOBAL_PARAM__MAIL_ID);
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_AddRewardList: get mail_id send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
        return 0;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        TbParam tbParamRes;
        dwRetCode = CAwsResponse::OnUpdateItemRsp(*pstSession->m_vecAwsRsp[0], &tbParamRes);
        if (tbParamRes.m_nVal == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_PACKAGE_ERR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_AddRewardList: get mail id failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }
        TINT64 ddwMailId = tbParamRes.m_nVal;
        TSE_LOG_INFO(pstSession->m_poServLog, ("Processcmd_AddRewardList: [new_mail_id=%ld][seq=%u]", ddwMailId, pstSession->m_stUserInfo.m_udwBSeqNo));

        SGlobalRes stGloablres;
        stGloablres.Reset();

        TUINT32 udwRewardNum = 0;
        SOneGlobalRes astRewardList[TBMAIL_EX_REWARD_MAX_NUM];
        memset(astRewardList, 0, sizeof(astRewardList));

        std::vector<string> vecOneReard;

        CCommonFunc::GetVectorFromString(pstSession->m_stReqParam.m_szKey[0], ',', vecOneReard);

        TSE_LOG_DEBUG(pstSession->m_poServLog, ("Processcmd_AddRewardList:key=%s reward size=%u [seq=%u]", pstSession->m_stReqParam.m_szKey[0], vecOneReard.size(), pstSession->m_stUserInfo.m_udwBSeqNo));


        for (TUINT32 udwRewardIdx = 0; udwRewardIdx < vecOneReard.size(); ++udwRewardIdx)
        {
//             if (stGloablres.ddwTotalNum >= MAX_REWARD_ITEM_NUM)
//             {
//                 continue;
//             }
            if (udwRewardNum >= TBMAIL_EX_REWARD_MAX_NUM)
            {
                continue;
            }
            TUINT32 audwReward[3] = { 0, 0, 0 };
            TUINT32 udwNum = 3;
            CCommonFunc::GetArrayFromString(vecOneReard[udwRewardIdx].c_str(), ':', audwReward, udwNum);

            TSE_LOG_DEBUG(pstSession->m_poServLog, ("Processcmd_AddRewardList: reward size=%u idx=%u reward=%s [type=%u id=%u num=%u][seq=%u]",
                vecOneReard.size(),
                udwRewardIdx,
                vecOneReard[udwRewardIdx].c_str(),
                audwReward[0U],
                audwReward[1U],
                audwReward[2U],
                pstSession->m_stUserInfo.m_udwBSeqNo));

//             stGloablres.aRewardList[stGloablres.ddwTotalNum].ddwType = audwReward[0U];
//             stGloablres.aRewardList[stGloablres.ddwTotalNum].ddwId = audwReward[1U];
//             stGloablres.aRewardList[stGloablres.ddwTotalNum].ddwNum = audwReward[2U];
//             stGloablres.ddwTotalNum++;
            astRewardList[udwRewardNum].ddwType = audwReward[0U];
            astRewardList[udwRewardNum].ddwId = audwReward[1U];
            astRewardList[udwRewardNum].ddwNum = audwReward[2U];
            udwRewardNum++;
        }

        TbMail tbMail;
        TbMail_user tbMailUser;
        tbMail.Reset();
        tbMailUser.Reset();
        CProcessMailReport::GenEventMail(pstSession->m_stReqParam.m_udwSvrId, pstSession->m_stUserInfo.m_tbPlayer.m_nUid,
            udwEventType, udwRewardType, dwKey, 0, sRankNameList, sRankSvrList, FALSE, dwPoint,
            astRewardList, udwRewardNum, CDocument::GetLang(pstSession->m_stUserInfo.m_tbLogin.m_nLang), ddwMailId, udwEventType, dwPoint,
            &tbMail, &tbMailUser, sEventInfo);
        TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_AddRewardList: mail event_id[%ld] [seq=%u]", tbMail.m_nEvent_id, pstSession->m_stUserInfo.m_udwBSeqNo));

        //推送
        SNoticInfo stNoticInfo;
        stNoticInfo.Reset();
        stNoticInfo.SetValue(EN_NOTI_ID__EVENT_REWARD,
            "", "",
            0, 0,
            0, 0,
            0, "", 0, CDocument::GetLang(pstSession->m_stUserInfo.m_tbLogin.m_nLang));
        CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), pstSession->m_stUserInfo.m_tbLogin.m_nSid, pstSession->m_stUserInfo.m_tbLogin.m_nUid, stNoticInfo);

        pstSession->ResetAwsInfo();
        if (udwRewardType == 0 || udwRewardType == 1)
        {
            //daemon TODO tips
            CSendMessageBase::AddTips(&pstSession->m_stUserInfo, EN_TIPS_TYPE__EVENT_RANK, pstSession->m_stUserInfo.m_tbPlayer.m_nUid, TRUE, udwEventType);

            CAwsRequest::PutItem(pstSession, &pstSession->m_stUserInfo.m_atbTips[pstSession->m_stUserInfo.m_udwTipsNum - 1]);

            //wave@20160122
            CSendMessageBase::AddEventTips(&pstSession->m_stUserInfo, udwEventType, udwRewardType, dwKey, dwPoint, sScoreList, astRewardList, udwRewardNum, sEventInfo,
                ddwMailId, SYSTEM_EVENT, dwWindowOpt);
            CAwsRequest::PutItem(pstSession, &pstSession->m_stUserInfo.m_atbEventTips[pstSession->m_stUserInfo.m_udwEventTipsNum - 1]);
        }

        pstSession->ResetReportInfo();
        if (tbMail.m_nId == ddwMailId)
        {
            CAwsRequest::UpdateItem(pstSession, &tbMail);
            CReportSvrRequest::MailUserPut(pstSession, &tbMailUser, 1);
        }
        CAwsRequest::UpdateItem(pstSession, &pstSession->m_stUserInfo.m_tbLogin);

        if (pstSession->m_vecAwsReq.size() > 0)
        {
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
            bNeedResponse = TRUE;
            TINT32 dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
            if (dwRetCode < 0)
            {
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("Processcmd_AddRewardList:[7] [ret=%d][seq=%u]", dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                return -3;
            }
            return 0;
        }

        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NO_ERROR;
        return -1;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        if (pstSession->m_vecReportReq.size() > 0)
        {
            //wave@push_data
            CPushDataBasic::PushDataUid_Refresh(pstSession, pstSession->m_stUserInfo.m_tbPlayer.m_nUid);

            // set next procedure
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__REPORT_SVR;
            // send request
            bNeedResponse = TRUE;
            dwRetCode = CBaseProcedure::SendReportSvrRequest(pstSession);
            if (dwRetCode < 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_AddRewardList: insert mail or update stat failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                return -6;
            }
            return 0;
        }
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NO_ERROR;
        return -1;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NO_ERROR;
        return -1;
    }

    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NO_ERROR;
    return -1;
}

TINT32 CProcessOperate::Processcmd_AddThemeRewardList(SSession* pstSession, TBOOL &bNeedResponse)
{
    /*
    活动---增加奖励
    key0=reward(type:id:num,type:id:num....)
    key1=
    key2=reward_type(0:goal 1:rank)
    key3=goal_num or rank_num (depends on key2)
    key4=score//玩家该活动的分数
    key5=rank uname in order list(first at one,use ":" to divide and available when key2=1)
    key6=rank svr_id in order list(first at one,use ":" to divide and available when key2=1, each to key4)
    key7=score_list
    key8={“e_t”://活动结束时间, “e_id”://活动id, “kingdoms”: //是否跨服, “event_ui”:活动label ui id, “e_title”://活动标题}
    key9=window_option	//弹窗设置（0：城内城外弹窗 1：只在城内弹窗）
    */

    TUINT32 udwRewardType = atoi(pstSession->m_stReqParam.m_szKey[2]); // 0: goal 1: rank
    TINT32 dwKey = atoi(pstSession->m_stReqParam.m_szKey[3]);
    TINT32 dwPoint = atoi(pstSession->m_stReqParam.m_szKey[4]);
    string sRankNameList = pstSession->m_stReqParam.m_szKey[5];
    string sRankSvrList = pstSession->m_stReqParam.m_szKey[6];
    string sScoreList = pstSession->m_stReqParam.m_szKey[7];
    string sEventInfo = pstSession->m_stReqParam.m_szKey[8];
    TINT32 dwWindowOpt = atoi(pstSession->m_stReqParam.m_szKey[9]);
    TUINT32 udwEventType = EN_EVENT_TYPE__THEME;

    TINT32 dwRetCode = 0;

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        pstSession->ResetAwsInfo();
        CAwsRequest::GetGlobalNewId(pstSession, EN_GLOBAL_PARAM__MAIL_ID);
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_AddRewardList: get mail_id send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
        return 0;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        TbParam tbParamRes;
        dwRetCode = CAwsResponse::OnUpdateItemRsp(*pstSession->m_vecAwsRsp[0], &tbParamRes);
        if (tbParamRes.m_nVal == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_PACKAGE_ERR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_AddRewardList: get mail id failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }
        TINT64 ddwMailId = tbParamRes.m_nVal;
        TSE_LOG_INFO(pstSession->m_poServLog, ("Processcmd_AddRewardList: [new_mail_id=%ld][seq=%u]", ddwMailId, pstSession->m_stUserInfo.m_udwBSeqNo));

        SGlobalRes stGloablres;
        stGloablres.Reset();

        TUINT32 udwRewardNum = 0;
        SOneGlobalRes astRewardList[TBMAIL_EX_REWARD_MAX_NUM];
        memset(astRewardList, 0, sizeof(astRewardList));

        std::vector<string> vecOneReward;

        CCommonFunc::GetVectorFromString(pstSession->m_stReqParam.m_szKey[0], ',', vecOneReward);

        TSE_LOG_DEBUG(pstSession->m_poServLog, ("Processcmd_AddRewardList:key=%s reward size=%u [seq=%u]", pstSession->m_stReqParam.m_szKey[0], vecOneReward.size(), pstSession->m_stUserInfo.m_udwBSeqNo));


        for (TUINT32 udwRewardIdx = 0; udwRewardIdx < vecOneReward.size(); ++udwRewardIdx)
        {
            //             if (stGloablres.ddwTotalNum >= MAX_REWARD_ITEM_NUM)
            //             {
            //                 continue;
            //             }
            if (udwRewardNum >= TBMAIL_EX_REWARD_MAX_NUM)
            {
                continue;
            }
            TUINT32 audwReward[3] = { 0, 0, 0 };
            TUINT32 udwNum = 3;
            CCommonFunc::GetArrayFromString(vecOneReward[udwRewardIdx].c_str(), ':', audwReward, udwNum);

            TSE_LOG_DEBUG(pstSession->m_poServLog, ("Processcmd_AddRewardList: reward size=%u idx=%u reward=%s [type=%u id=%u num=%u][seq=%u]",
                vecOneReward.size(),
                udwRewardIdx,
                vecOneReward[udwRewardIdx].c_str(),
                audwReward[0U],
                audwReward[1U],
                audwReward[2U],
                pstSession->m_stUserInfo.m_udwBSeqNo));

            //             stGloablres.aRewardList[stGloablres.ddwTotalNum].ddwType = audwReward[0U];
            //             stGloablres.aRewardList[stGloablres.ddwTotalNum].ddwId = audwReward[1U];
            //             stGloablres.aRewardList[stGloablres.ddwTotalNum].ddwNum = audwReward[2U];
            //             stGloablres.ddwTotalNum++;
            astRewardList[udwRewardNum].ddwType = audwReward[0U];
            astRewardList[udwRewardNum].ddwId = audwReward[1U];
            astRewardList[udwRewardNum].ddwNum = audwReward[2U];
            udwRewardNum++;
        }

        TbMail tbMail;
        TbMail_user tbMailUser;
        tbMail.Reset();
        tbMailUser.Reset();
        CProcessMailReport::GenEventMail(pstSession->m_stReqParam.m_udwSvrId, pstSession->m_stUserInfo.m_tbPlayer.m_nUid,
            udwEventType, udwRewardType, dwKey, 0, sRankNameList, sRankSvrList, FALSE, dwPoint,
            astRewardList, udwRewardNum, CDocument::GetLang(pstSession->m_stUserInfo.m_tbLogin.m_nLang), ddwMailId, udwEventType, dwPoint,
            &tbMail, &tbMailUser, sEventInfo);
        TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_AddRewardList: mail event_id[%ld] [seq=%u]", tbMail.m_nEvent_id, pstSession->m_stUserInfo.m_udwBSeqNo));

        //推送
        SNoticInfo stNoticInfo;
        stNoticInfo.Reset();
        stNoticInfo.SetValue(EN_NOTI_ID__EVENT_REWARD,
            "", "",
            0, 0,
            0, 0,
            0, "", 0, CDocument::GetLang(pstSession->m_stUserInfo.m_tbLogin.m_nLang));
        CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), pstSession->m_stUserInfo.m_tbLogin.m_nSid, pstSession->m_stUserInfo.m_tbLogin.m_nUid, stNoticInfo);

        pstSession->ResetAwsInfo();
        if (udwRewardType == 0 || udwRewardType == 1)
        {
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("Processcmd_AddRewardList: [reward type=%u] [seq=%u]", udwRewardType, pstSession->m_stUserInfo.m_udwBSeqNo));
            //daemon TODO tips
            CSendMessageBase::AddTips(&pstSession->m_stUserInfo, EN_TIPS_TYPE__EVENT_RANK, pstSession->m_stUserInfo.m_tbPlayer.m_nUid, TRUE, udwEventType);

            CAwsRequest::PutItem(pstSession, &pstSession->m_stUserInfo.m_atbTips[pstSession->m_stUserInfo.m_udwTipsNum - 1]);

            //wave@20160122
            CSendMessageBase::AddEventTips(&pstSession->m_stUserInfo, udwEventType, udwRewardType, dwKey, dwPoint, sScoreList, astRewardList, udwRewardNum, sEventInfo,
                ddwMailId, SYSTEM_EVENT, dwWindowOpt);

            CAwsRequest::PutItem(pstSession, &pstSession->m_stUserInfo.m_atbEventTips[pstSession->m_stUserInfo.m_udwEventTipsNum - 1]);
        }

        pstSession->ResetReportInfo();
        if (tbMail.m_nId == ddwMailId)
        {
            CAwsRequest::UpdateItem(pstSession, &tbMail);
            CReportSvrRequest::MailUserPut(pstSession, &tbMailUser, 1);
        }
        CAwsRequest::UpdateItem(pstSession, &pstSession->m_stUserInfo.m_tbLogin);

        if (pstSession->m_vecAwsReq.size() > 0)
        {
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
            bNeedResponse = TRUE;
            TINT32 dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
            if (dwRetCode < 0)
            {
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("Processcmd_AddRewardList:[7] [ret=%d][seq=%u]", dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                return -3;
            }
            return 0;
        }

        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NO_ERROR;
        return -1;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        if (pstSession->m_vecReportReq.size() > 0)
        {
            //wave@push_data
            CPushDataBasic::PushDataUid_Refresh(pstSession, pstSession->m_stUserInfo.m_tbPlayer.m_nUid);

            // set next procedure
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__REPORT_SVR;
            // send request
            bNeedResponse = TRUE;
            dwRetCode = CBaseProcedure::SendReportSvrRequest(pstSession);
            if (dwRetCode < 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_AddRewardList: insert mail or update stat failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                return -6;
            }
            return 0;
        }
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NO_ERROR;
        return -1;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NO_ERROR;
        return -1;
    }

    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NO_ERROR;
    return -1;
}

TINT32 CProcessOperate::Processcmd_AddAlRewardListNew(SSession* pstSession, TBOOL &bNeedResponse)
{
    /*
        活动---增加联盟礼物
        command=op_add_al_reward_list
        key0=reward(type:id:num,type:id:num....)
        key1=event_type
        key2=reward_type(0:goal 1:rank)
        key3=goal_num or rank_num (depends on key2)
        key4=score//玩家该活动的分数
        key5=alid
        key6=rank alname in order list (first at one,use ":" to divide and available when key2=1)
        key7=rank svr_id in order list (first at one,use ":" to divide and available when key2=1, each to key4)

    */

    TUINT32 udwEventType = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TUINT32 udwRewardType = atoi(pstSession->m_stReqParam.m_szKey[2]);
    //TINT32 dwKey = atoi(pstSession->m_stReqParam.m_szKey[3]);
    string sRankNameList = pstSession->m_stReqParam.m_szKey[6];
    string sRankSvrList = pstSession->m_stReqParam.m_szKey[7];
    TINT32 dwAlid = atoi(pstSession->m_stReqParam.m_szKey[5]);
    //TINT32 dwPoint = atoi(pstSession->m_stReqParam.m_szKey[4]);
    string sScoreList = pstSession->m_stReqParam.m_szKey[8];
    string sEventInfo = pstSession->m_stReqParam.m_szKey[9];

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        std::vector<string> vecReward;

        CCommonFunc::GetVectorFromString(pstSession->m_stReqParam.m_szKey[0], ',', vecReward);

        TSE_LOG_DEBUG(pstSession->m_poServLog, ("Processcmd_AddAlRewardList:key=%s reward size=%u [seq=%u]", pstSession->m_stReqParam.m_szKey[0], vecReward.size(), pstSession->m_stUserInfo.m_udwBSeqNo));

        SGlobalRes stGlobalRes;
        stGlobalRes.Reset();
        for(TUINT32 udwRewardIdx = 0; udwRewardIdx < vecReward.size(); ++udwRewardIdx)
        {
            if(stGlobalRes.ddwTotalNum >= MAX_REWARD_ITEM_NUM)
            {
                continue;
            }

            TUINT32 audwReward[3] = {0, 0, 0};
            TUINT32 udwNum = 3;
            CCommonFunc::GetArrayFromString(vecReward[udwRewardIdx].c_str(), ':', audwReward, udwNum);

            TSE_LOG_DEBUG(pstSession->m_poServLog, ("Processcmd_AddAlRewardList: reward size=%u idx=%u reward=%s [type=%u id=%u num=%u][seq=%u]",
                vecReward.size(),
                udwRewardIdx,
                vecReward[udwRewardIdx].c_str(),
                audwReward[0U],
                audwReward[1U],
                audwReward[2U],
                pstSession->m_stUserInfo.m_udwBSeqNo));

            if(audwReward[0U] == EN_GLOBALRES_TYPE_AL_GIFT)
            {
                TUINT32 i = 0;
                while(i < audwReward[2U])
                {
                    TINT32 dwRetCode = CCommonLogic::GenAlGiftToAlliance(dwAlid, audwReward[1U], pstSession->m_stUserInfo.m_tbPlayer.m_nUid,
                        pstSession->m_stUserInfo.m_stAlGifts, &pstSession->m_stUserInfo.m_tbLogin);
                    if(0 != dwRetCode)
                    {
                        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
                        TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_AddAlRewardList: pack id[%d] get reward error [retcode=%d] [seq=%u]", dwRetCode, audwReward[1U], pstSession->m_udwSeqNo));
                        return -3;
                    }
                    i++;
                }
                stGlobalRes.aRewardList[stGlobalRes.ddwTotalNum].ddwType = audwReward[0U];
                stGlobalRes.aRewardList[stGlobalRes.ddwTotalNum].ddwId = audwReward[1U];
                stGlobalRes.aRewardList[stGlobalRes.ddwTotalNum].ddwNum = audwReward[2U];
                stGlobalRes.ddwTotalNum++;
            }
        }

        if (udwRewardType == 0 || udwRewardType == 1)
        {
            CSendMessageBase::AddAlTips(&pstSession->m_stUserInfo, EN_TIPS_TYPE__EVENT_RANK, pstSession->m_stUserInfo.m_tbAlliance.m_nAid, TRUE, udwEventType);
        }

        //推送 
        SNoticInfo stNoticInfo;
        stNoticInfo.Reset();
        stNoticInfo.SetValue(EN_NOTI_ID__EVENT_REWARD,
            "", "",
            0, 0,
            0, 0,
            0, "", 0, CDocument::GetLang(pstSession->m_stUserInfo.m_tbLogin.m_nLang));
        CMsgBase::SendNotificationAlliance(CConfBase::GetString("tbxml_project"), pstSession->m_stUserInfo.m_tbLogin.m_nSid, pstSession->m_stUserInfo.m_tbLogin.m_nUid, dwAlid, stNoticInfo);


        if(0 == udwRewardType)
        {
            pstSession->ResetAwsInfo();

            TBOOL bNeedUpdate = FALSE;
            TbAlliance tbAllianceTmp;
            tbAllianceTmp.Reset();
            
            ExpectedItem expect_item;
            ExpectedDesc expect_desc;
            expect_desc.clear();
            expect_item.SetVal(TbALLIANCE_FIELD_AID, TRUE, dwAlid);
            expect_desc.push_back(expect_item);
            
            tbAllianceTmp.Set_Aid(dwAlid);

            if(bNeedUpdate)
            {
                CAwsRequest::UpdateItem(pstSession, &tbAllianceTmp, expect_desc, RETURN_VALUES_ALL_NEW);
            }
        }
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("Processcmd_AddAlRewardList: [m_vecAwsReq.size=%d] [seq=%u]", \
                                                pstSession->m_vecAwsReq.size(), pstSession->m_stUserInfo.m_udwBSeqNo));
        

        if(pstSession->m_vecAwsReq.size() > 0)
        {
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
            bNeedResponse = TRUE;
            TINT32 dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
            if(dwRetCode < 0)
            {

                TSE_LOG_DEBUG(pstSession->m_poServLog, ("Processcmd_AddAlRewardList:[7] [ret=%d][seq=%u]", dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                return -5;
            }
            return 0;
        }

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        //wave@push_data
        CPushDataBasic::PushDataAid_Refresh(pstSession, dwAlid);

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessOperate::Processcmd_AddThemeAlRewardList(SSession* pstSession, TBOOL &bNeedResponse)
{
    /*
    command=op_add_theme_al_reward_list
    key0=reward(type:id:num,type:id:num....)
    key1=
    key2=reward_type(0:goal 1:rank)
    key3=goal_num or rank_num
    key4=score//玩家该活动的分数
    key5=alid
    key6=rank alname in order list
    key7=rank svr_id in order list
    key8=score_list
    key9={“e_t”://活动结束时间, “e_id”://活动id, “kingdoms”: //是否跨服, “event_ui”:活动label ui id, “e_title”://活动标题}
    key10=window_option	//弹窗设置（0：城内城外弹窗 1：只在城内弹窗）
    */
    TUINT32 udwEventType = EN_EVENT_TYPE__THEME;
    TUINT32 udwRewardType = atoi(pstSession->m_stReqParam.m_szKey[2]);
    //TINT32 dwKey = atoi(pstSession->m_stReqParam.m_szKey[3]);
    string sRankNameList = pstSession->m_stReqParam.m_szKey[6];
    string sRankSvrList = pstSession->m_stReqParam.m_szKey[7];
    TINT32 dwAlid = atoi(pstSession->m_stReqParam.m_szKey[5]);
    //TINT32 dwPoint = atoi(pstSession->m_stReqParam.m_szKey[4]);
    string sScoreList = pstSession->m_stReqParam.m_szKey[8];
    string sEventInfo = pstSession->m_stReqParam.m_szKey[9];
    TINT32 dwWindowOpt = atoi(pstSession->m_stReqParam.m_szKey[10]);

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        std::vector<string> vecReward;

        CCommonFunc::GetVectorFromString(pstSession->m_stReqParam.m_szKey[0], ',', vecReward);

        TSE_LOG_DEBUG(pstSession->m_poServLog, ("Processcmd_AddAlRewardList:key=%s reward size=%u [seq=%u]", pstSession->m_stReqParam.m_szKey[0], vecReward.size(), pstSession->m_stUserInfo.m_udwBSeqNo));

        SGlobalRes stGlobalRes;
        stGlobalRes.Reset();
        for (TUINT32 udwRewardIdx = 0; udwRewardIdx < vecReward.size(); ++udwRewardIdx)
        {
            if (stGlobalRes.ddwTotalNum >= MAX_REWARD_ITEM_NUM)
            {
                continue;
            }

            TUINT32 audwReward[3] = { 0, 0, 0 };
            TUINT32 udwNum = 3;
            CCommonFunc::GetArrayFromString(vecReward[udwRewardIdx].c_str(), ':', audwReward, udwNum);

            TSE_LOG_DEBUG(pstSession->m_poServLog, ("Processcmd_AddAlRewardList: reward size=%u idx=%u reward=%s [type=%u id=%u num=%u][seq=%u]",
                vecReward.size(),
                udwRewardIdx,
                vecReward[udwRewardIdx].c_str(),
                audwReward[0U],
                audwReward[1U],
                audwReward[2U],
                pstSession->m_stUserInfo.m_udwBSeqNo));

            if (audwReward[0U] == EN_GLOBALRES_TYPE_AL_GIFT)
            {
                TUINT32 i = 0;
                while (i < audwReward[2U])
                {
                    TINT32 dwRetCode = CCommonLogic::GenAlGiftToAlliance(dwAlid, audwReward[1U], pstSession->m_stUserInfo.m_tbPlayer.m_nUid,
                        pstSession->m_stUserInfo.m_stAlGifts, &pstSession->m_stUserInfo.m_tbLogin);
                    if (0 != dwRetCode)
                    {
                        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
                        TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_AddAlRewardList: pack id[%d] get reward error [retcode=%d] [seq=%u]", dwRetCode, audwReward[1U], pstSession->m_udwSeqNo));
                        return -3;
                    }
                    i++;
                }
                stGlobalRes.aRewardList[stGlobalRes.ddwTotalNum].ddwType = audwReward[0U];
                stGlobalRes.aRewardList[stGlobalRes.ddwTotalNum].ddwId = audwReward[1U];
                stGlobalRes.aRewardList[stGlobalRes.ddwTotalNum].ddwNum = audwReward[2U];
                stGlobalRes.ddwTotalNum++;
            }
        }

        if (udwRewardType == 0 || udwRewardType == 1)
        {
            CSendMessageBase::AddAlTips(&pstSession->m_stUserInfo, EN_TIPS_TYPE__EVENT_RANK, pstSession->m_stUserInfo.m_tbAlliance.m_nAid, TRUE, udwEventType);
        }

        //推送 
        SNoticInfo stNoticInfo;
        stNoticInfo.Reset();
        stNoticInfo.SetValue(EN_NOTI_ID__EVENT_REWARD,
            "", "",
            0, 0,
            0, 0,
            0, "", 0, CDocument::GetLang(pstSession->m_stUserInfo.m_tbLogin.m_nLang));
        CMsgBase::SendNotificationAlliance(CConfBase::GetString("tbxml_project"), pstSession->m_stUserInfo.m_tbLogin.m_nSid, pstSession->m_stUserInfo.m_tbLogin.m_nUid, dwAlid, stNoticInfo);


        if (0 == udwRewardType)
        {
            pstSession->ResetAwsInfo();

            TBOOL bNeedUpdate = FALSE;
            TbAlliance tbAllianceTmp;
            tbAllianceTmp.Reset();

            ExpectedItem expect_item;
            ExpectedDesc expect_desc;
            expect_desc.clear();
            expect_item.SetVal(TbALLIANCE_FIELD_AID, TRUE, dwAlid);
            expect_desc.push_back(expect_item);

            tbAllianceTmp.Set_Aid(dwAlid);

            if (bNeedUpdate)
            {
                CAwsRequest::UpdateItem(pstSession, &tbAllianceTmp, expect_desc, RETURN_VALUES_ALL_NEW);
            }
        }
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("Processcmd_AddAlRewardList: [m_vecAwsReq.size=%d] [seq=%u]", \
            pstSession->m_vecAwsReq.size(), pstSession->m_stUserInfo.m_udwBSeqNo));


        if (pstSession->m_vecAwsReq.size() > 0)
        {
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
            bNeedResponse = TRUE;
            TINT32 dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
            if (dwRetCode < 0)
            {

                TSE_LOG_DEBUG(pstSession->m_poServLog, ("Processcmd_AddAlRewardList:[7] [ret=%d][seq=%u]", dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                return -5;
            }
            return 0;
        }

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        //wave@push_data
        CPushDataBasic::PushDataAid_Refresh(pstSession, dwAlid);

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessOperate::Processcmd_AddLoyaltyAndFund(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT64 ddwLoyalty = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TINT64 ddwFundNum = strtoll(pstSession->m_stReqParam.m_szKey[1], NULL, 10);

    TSE_LOG_INFO(pstSession->m_poServLog, ("Processcmd_AddLoyaltyAndFund: start [Loyalty=%ld] [FundNum=%ld] [seq=%u]",
        ddwLoyalty, ddwFundNum, pstSession->m_stUserInfo.m_udwBSeqNo));

    if(0 > ddwLoyalty || 0 > ddwFundNum)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_AddLoyaltyAndFund: parameter is illegal [seq=%u]",
            pstSession->m_stUserInfo.m_udwBSeqNo));
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }

    if(pstSession->m_stUserInfo.m_tbAlliance.m_nAid == 0
        || pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos == 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_AddLoyaltyAndFund: player[alid: %ld, alpos: %ld] in not in alliance [seq=%u]",
            pstSession->m_stUserInfo.m_tbPlayer.m_nAlid, pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -2;
    }

    if(ddwLoyalty > 0)
    {
        pstSession->m_stUserInfo.m_tbPlayer.Set_Loy_cur(pstSession->m_stUserInfo.m_tbPlayer.m_nLoy_cur + ddwLoyalty);
        pstSession->m_stUserInfo.m_tbPlayer.Set_Loy_all(pstSession->m_stUserInfo.m_tbPlayer.m_nLoy_all + ddwLoyalty);
    }

    if(ddwFundNum > 0)
    {
        pstSession->m_stUserInfo.m_tbAlliance.Set_Fund(ddwFundNum, UPDATE_ACTION_TYPE_ADD);
    }

    TSE_LOG_INFO(pstSession->m_poServLog, ("Processcmd_AddLoyaltyAndFund: end [seq=%u]",
        pstSession->m_stUserInfo.m_udwBSeqNo));
    return 0;
}

TINT32 CProcessOperate::Processcmd_ListBuildingPos(SSession* pstSession, TBOOL& bNeedResponse)
{
    SCityInfo *pstCity = &pstSession->m_stUserInfo.m_stCityInfo;

    pstSession->m_JsonValue = Json::Value(Json::objectValue);
    pstSession->m_JsonValue["building"] = Json::Value(Json::arrayValue);
    set<TINT32> setPosSet;
    TBOOL bConflict = FALSE;
    for(TUINT32 udwIdx = 0; udwIdx < pstCity->m_stTblData.m_bBuilding.m_udwNum; ++udwIdx)
    {
        SCityBuildingNode* pstNode = &pstCity->m_stTblData.m_bBuilding[udwIdx];
        pstSession->m_JsonValue["building"][udwIdx] = Json::Value(Json::arrayValue);
        pstSession->m_JsonValue["building"][udwIdx].append(pstNode->m_ddwPos);
        pstSession->m_JsonValue["building"][udwIdx].append(pstNode->m_ddwType);
        pstSession->m_JsonValue["building"][udwIdx].append(pstNode->m_ddwLevel);
        if(pstNode->m_ddwLevel == 0)
        {
            continue;
        }
        TINT32 dwCenterPos = pstNode->m_ddwPos;
        TINT32 dwSize = CToolBase::GetBuildingSize(pstNode->m_ddwType);
        bConflict = CCommonLogic::GetBuildingPos(dwCenterPos, dwSize, setPosSet);
    }

    pstSession->m_JsonValue["conflict"] = bConflict;
    pstSession->m_JsonValue["pos"] = Json::Value(Json::arrayValue);

    for(set<TINT32>::iterator it = setPosSet.begin(); it != setPosSet.end(); ++it)
    {
        pstSession->m_JsonValue["pos"].append(*it);
    }

    return 0;
}


TINT32 CProcessOperate::ProcessCmd_RecoverPlayerCity(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        CProcessOperate::RecoverPlayerCity(pstSession, &pstSession->m_stUserInfo,
            pstSession->m_stUserInfo.m_atbWild, pstSession->m_stUserInfo.m_udwWildNum);

        // send request
        if(pstSession->m_vecAwsReq.size() > 0)
        {
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
            bNeedResponse = TRUE;
            dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
            if(dwRetCode < 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                return -1;
            }
            return 0;
        }

        pstSession->m_stUserInfo.m_udwWildNum = 0;
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        pstSession->m_stUserInfo.m_udwWildNum = 0;
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

/***********************************************************private************************************************************************/

TBOOL CProcessOperate::TbPlayer_Compare(const TbPlayer *pstA, const TbPlayer *pstB)
{
    if(pstA->m_nMight > pstB->m_nMight)
    {
        return true;
    }
    else if(pstA->m_nMight == pstB->m_nMight)
    {
        if(pstA->m_nUtime > pstB->m_nUtime)
        {
            return true;
        }
    }
    return false;
}

TINT32 OpenAlGift(int pack_id)
{
    CGameInfo* poGameInfo = CGameInfo::GetInstance();
    SGlobalRes stRewardsRes;
    stRewardsRes.Reset();

    //开始定位reward
    TINT32 dwPackId = pack_id;  //拿到pack id
    if(!poGameInfo->m_oJsonRoot["game_al_gift_new"]["pack"].isMember(CCommonFunc::NumToString(dwPackId)))
    {
        TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("OpenAlGift:%d -1", pack_id));
        return -1;
    }

    //根据pack id，找到rewardlist
    const Json::Value &jRewardList = poGameInfo->m_oJsonRoot["game_al_gift_new"]["pack"][CCommonFunc::NumToString(dwPackId)]["reward_list"];
    TINT32 dwRewardSize = jRewardList.size();
    if(dwRewardSize == 0)
    {
        TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("OpenAlGift:%d -2", pack_id));
        return -2;
    }
    //根据概率随机一个reward id
    TUINT32 dwRewardId = CGlobalResLogic::GetAlGiftRewarId(poGameInfo->m_oJsonRoot["game_al_gift_new"]["pack"][CCommonFunc::NumToString(dwPackId)]);
    if(!poGameInfo->m_oJsonRoot["game_al_gift_new"]["reward"].isMember(CCommonFunc::NumToString(dwRewardId)))
    {
        TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("OpenAlGift:%d -3", pack_id));
        return -3;
    }
    //拿到了reward id，开始生成对应礼品并获取礼品
    TINT32 dwRewardType = poGameInfo->m_oJsonRoot["game_al_gift_new"]["reward"][CCommonFunc::NumToString(dwRewardId)]["r0"].asInt();  //获取方式，0-全部获取，1-随机若干，2-随机一种
    const Json::Value& oRewardJson = poGameInfo->m_oJsonRoot["game_al_gift_new"]["reward"][CCommonFunc::NumToString(dwRewardId)]["r1"];
    CGlobalResLogic::GetGlobalResInfo(oRewardJson, dwRewardType, &stRewardsRes);

    // log:todo
    string rewards_str = "";
    char tmp[100];
    for(TUINT32 i = 0; i < stRewardsRes.ddwTotalNum; i++)
    {
        if(i == 0)
        {
            sprintf(tmp, "%ld,%ld,%ld", stRewardsRes.aRewardList[i].ddwType, stRewardsRes.aRewardList[i].ddwId, stRewardsRes.aRewardList[i].ddwNum);
        }
        else
        {
            sprintf(tmp, "#%ld,%ld,%ld", stRewardsRes.aRewardList[i].ddwType, stRewardsRes.aRewardList[i].ddwId, stRewardsRes.aRewardList[i].ddwNum);
        }
        rewards_str.append(tmp);
    }
    TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("OpenAlGift__RES:%d %d %d %s", pack_id, dwRewardId, dwRewardType, rewards_str.c_str()));

    return 0;
}

TINT32 CProcessOperate::ProcessCmd_StatisticalOpenAlGiftWave(SSession *pstSession, TBOOL &bNeedResponse)
{
    int ret = 0;
    int gift_id_list[20] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
    for(int idx = 0; idx < 20; idx++)
    {
        for(int idy = 0; idy < 1000; idy++)
        {
            ret = OpenAlGift(gift_id_list[idx]);
        }
    }
    return 0;
}

TINT32 CProcessOperate::ProcessCmd_CheckPlayerMap(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;

    // get city info --request
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        // set package
        pstSession->ResetAwsInfo();
        CAwsRequest::MapQueryByUid(pstSession, pstSession->m_stReqParam.m_udwSvrId, pstSession->m_stReqParam.m_udwUserId);
        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            return -1;
        }
        return 0;
    }

    // get wild info -- response
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;

        // parse data
        pstSession->m_stUserInfo.m_udwWildNum = 0;
        pstSession->m_udwTmpWildNum = 0;
        dwRetCode = CAwsResponse::OnQueryRsp(*pstSession->m_vecAwsRsp[0],
            pstSession->m_atbTmpWild, sizeof(TbMap), MAX_WILD_RETURN_NUM);

        if(dwRetCode > 0)
        {
            pstSession->m_udwTmpWildNum = dwRetCode;
        }

        // check status——只检测不处理
        CProcessOperate::CheckPlayerMap(pstSession, &pstSession->m_stUserInfo,
            pstSession->m_atbTmpWild, pstSession->m_udwTmpWildNum);
    }

    // wild info ___updt
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        // set package
        pstSession->ResetAwsInfo();

        // 更新地图有的，而用户信息没有的
        for(TUINT32 idx = 0; idx < pstSession->m_udwTmpWildNum; idx++)
        {
            ExpectedDesc expDesc;
            expDesc.vecExpectedItem.push_back(ExpectedItem(TbMAP_FIELD_UID, true, pstSession->m_stReqParam.m_udwUserId));
            dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstSession->m_atbTmpWild[idx], expDesc);
            if(dwRetCode == 0)
            {
                TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessCmd__CheckPlayerMap: del wild or city from map, pos=%ld, uid=%u, sid=%u [seq=%u]",
                    pstSession->m_atbTmpWild[idx].m_nId, pstSession->m_stReqParam.m_udwUserId,
                    pstSession->m_stReqParam.m_udwSvrId, pstSession->m_stUserInfo.m_udwBSeqNo));
            }
        }

        // send request
        if(pstSession->m_vecAwsReq.size())
        {
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;
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

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    return 0;
}

TINT32 CProcessOperate::ProcessCmd_CheckPlayerSvrMap(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwSid = atoi(pstSession->m_stReqParam.m_szKey[0]);

    TINT32 dwRetCode = 0;

    // get city info --request
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        // set package
        pstSession->ResetAwsInfo();
        CAwsRequest::MapQueryByUid(pstSession, dwSid, pstSession->m_stReqParam.m_udwUserId);
        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            return -1;
        }
        return 0;
    }

    // get wild info -- response
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;

        // parse data
        pstSession->m_udwTmpWildNum = 0;
        dwRetCode = CAwsResponse::OnQueryRsp(*pstSession->m_vecAwsRsp[0],
            pstSession->m_atbTmpWild, sizeof(TbMap), MAX_WILD_RETURN_NUM);

        if(dwRetCode > 0)
        {
            pstSession->m_udwTmpWildNum = dwRetCode;
        }
    }

    // wild info ___updt
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        // set package
        pstSession->ResetAwsInfo();

        // 更新地图有的，而用户信息没有的
        for(TUINT32 idx = 0; idx < pstSession->m_udwTmpWildNum; idx++)
        {
            if(pstSession->m_atbTmpWild[idx].m_nSid == pstSession->m_stUserInfo.m_tbPlayer.m_nSid)
            {
                continue;
            }

            CCommonBase::AbandonWild(NULL, &pstSession->m_atbTmpWild[idx]);
            ExpectedDesc expDesc;
            expDesc.vecExpectedItem.push_back(ExpectedItem(TbMAP_FIELD_UID, true, pstSession->m_stReqParam.m_udwUserId));
            dwRetCode = CAwsRequest::UpdateItem(pstSession, &pstSession->m_atbTmpWild[idx], expDesc);
            if(dwRetCode == 0)
            {
                TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessCmd__CheckPlayerMap: del wild or city from map, pos=%ld, uid=%u, sid=%u [seq=%u]",
                    pstSession->m_atbTmpWild[idx].m_nId, pstSession->m_stReqParam.m_udwUserId,
                    pstSession->m_stReqParam.m_udwSvrId, pstSession->m_stUserInfo.m_udwBSeqNo));
            }
        }

        // send request
        if(pstSession->m_vecAwsReq.size())
        {
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;
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

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    return 0;
}

TINT32 CProcessOperate::ProcessCmd_ClearUser(SSession* pstSession, TBOOL& bNeedResponse)
{
    TINT32 dwRetCode = 0;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstSession->m_stUserInfo.m_stCityInfo;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
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

        if(ptbPlayer->m_nNpc != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__USER_CANNOT_CLEAR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__ClearUser: uid=%ld is npc, can't clear [seq=%u]", 
                ptbPlayer->m_nUid, pstSession->m_stUserInfo.m_udwBSeqNo)); 
            return -7;
        }

        //if(pstUser->m_tbLogin.m_nGem_buy > 0)
        //{
        //    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__USER_CANNOT_CLEAR;
        //    TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__ClearUser: [uid=%ld] is a paid user, can't clear!!! [seq=%u]", pstUser->m_tbLogin.m_nUid,
        //        pstSession->m_stUserInfo.m_udwBSeqNo));
        //    return -1;
        //}

        //if(ptbPlayer->m_nAlid > 0)
        //{
        //    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__USER_CANNOT_CLEAR;
        //    TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__ClearUser:  [uid=%ld]'s [aid=%ld], can't clear!!! [seq=%u]",
        //        ptbPlayer->m_nUid, ptbPlayer->m_nAlid, pstSession->m_stUserInfo.m_udwBSeqNo));
        //    return -2;
        //}

        for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
        {
            TbMarch_action* ptbMarch = &pstUser->m_atbMarch[udwIdx];
            if(!CActionBase::IsPlayerOwnedAction(ptbPlayer->m_nUid, ptbMarch->m_nId))
            {
                continue;
            }

            if(ptbMarch->m_nSclass != EN_ACTION_SEC_CLASS__NOTI_TIMER)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__USER_CANNOT_CLEAR;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__ClearUser:  [uid=%ld] has march [id=%ld][sec_class=%ld], can't clear!!! [seq=%u]",
                    ptbPlayer->m_nUid, ptbMarch->m_nId, ptbMarch->m_nSclass, pstSession->m_stUserInfo.m_udwBSeqNo));
                return -3;
            }
            else
            {
                pstUser->m_aucMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__DEL;
            }
        }

        for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwSelfAlActionNum; ++udwIdx)
        {
            TbAlliance_action* ptbAlAction = &pstUser->m_atbSelfAlAction[udwIdx];
            if(!CActionBase::IsPlayerOwnedAction(ptbPlayer->m_nUid, ptbAlAction->m_nId))
            {
                continue;
            }

            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__USER_CANNOT_CLEAR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__ClearUser:  [uid=%ld] has alaction [id=%ld][sec_class=%ld], can't clear!!! [seq=%u]",
                ptbPlayer->m_nUid, ptbAlAction->m_nId, ptbAlAction->m_nSclass, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -4;
        }

        for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwActionNum; ++udwIdx)
        {
            TbAction* ptbAction = &pstUser->m_atbAction[udwIdx];
            if(!CActionBase::IsPlayerOwnedAction(ptbPlayer->m_nUid, ptbAction->m_nId))
            {
                continue;
            }

            if(ptbAction->m_nMclass != EN_ACTION_MAIN_CLASS__ITEM)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__USER_CANNOT_CLEAR;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__ClearUser:  [uid=%ld] has action [id=%ld][sec_class=%ld], can't clear!!! [seq=%u]",
                    ptbPlayer->m_nUid, ptbAction->m_nId, ptbAction->m_nSclass, pstSession->m_stUserInfo.m_udwBSeqNo));
                return -5;
            }
            else
            {
                pstUser->m_aucActionFlag[udwIdx] = EN_TABLE_UPDT_FLAG__DEL;
            }
        }

        if(pstUser->m_udwPassiveMarchNum > 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__USER_CANNOT_CLEAR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__ClearUser: [uid=%ld] has passive march, can't clear!!! [seq=%u]",
                ptbPlayer->m_nUid, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -6;
        }

        /*
        TCHAR szHttpResponse[MAX_JSON_LEN];
        TCHAR szUrl[1024];
        memset(szHttpResponse, 0, sizeof(szHttpResponse));

        memset(szUrl, 0, sizeof(szUrl));
        sprintf(szUrl, "%s?request=command=clear_account&key0=%ld&key1=1100537975&key2=IOS&key3=1&time=%u",
            CConfBase::GetString("account_url_pre", "serv_url").c_str(),
            ptbPlayer->m_nUid, udwCurTime);

        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessCmd_ClearUser: Url:%s [seq=%u]", szUrl, pstSession->m_stUserInfo.m_udwBSeqNo));

        CURLcode res = CToolBase::ResFromUrl(szUrl, szHttpResponse);
        if(CURLE_OK != res)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__USER_CANNOT_CLEAR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ClearUser: curl failed[%d]. [seq=%u]", res, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -8;
        }

        Json::Reader reader;
        pstSession->m_JsonValue.clear();
        if(FALSE == reader.parse(szHttpResponse, pstSession->m_JsonValue))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__USER_CANNOT_CLEAR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ClearUser: parse http response failed. [seq=%u]", pstSession->m_udwSeqNo));
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ClearUser: http response [response=%s] [seq=%u]",
                szHttpResponse, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -9;
        }

        if(pstSession->m_JsonValue.isObject()
        && pstSession->m_JsonValue.isMember("res_header")
        && pstSession->m_JsonValue["res_header"].isObject()
        && pstSession->m_JsonValue["res_header"].isMember("ret_code")
        && pstSession->m_JsonValue["res_header"]["ret_code"].asInt() == 0)
        {
            pstSession->m_JsonValue.clear();
            TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessCmd_ClearUser: clear user account succ [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
        }
        else
        {
            pstSession->m_JsonValue.clear();
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__USER_CANNOT_CLEAR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ClearUser: parse http response failed. [seq=%u]", pstSession->m_udwSeqNo));
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ClearUser: http response [response=%s] [seq=%u]",
                szHttpResponse, pstSession->m_stUserInfo.m_udwBSeqNo));;
            return -10;
        }
        */
        pstSession->ResetAwsInfo();
        ptbPlayer->Set_Dead_flag(1);

        CAwsRequest::UpdateItem(pstSession, &pstUser->m_tbLogin);
        CAwsRequest::UpdateItem(pstSession, ptbPlayer);
        CAwsRequest::UpdateItem(pstSession, &pstCity->m_stTblData);

        for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwWildNum; ++udwIdx)
        {
            if(pstUser->m_atbWild[udwIdx].m_nUid != ptbPlayer->m_nUid)
            {
                continue;
            }

            CCommonBase::AbandonWild(pstCity, &pstUser->m_atbWild[udwIdx]);
            ExpectedItem expect_item;
            ExpectedDesc expect_desc;
            expect_item.SetVal(TbMAP_FIELD_UID, TRUE, ptbPlayer->m_nUid);
            expect_desc.push_back(expect_item);
            CAwsRequest::UpdateItem(pstSession, &pstUser->m_atbWild[udwIdx], expect_desc);
        }

        for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
        {
            if(pstUser->m_aucMarchFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
            {
                CAwsRequest::DeleteItem(pstSession, &pstUser->m_atbMarch[udwIdx]);
            }
        }

        for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwActionNum; ++udwIdx)
        {
            if(pstUser->m_aucActionFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
            {
                CAwsRequest::DeleteItem(pstSession, &pstUser->m_atbAction[udwIdx]);
            }
        }

        pstUser->m_udwWildNum = 0;
        pstUser->m_udwActionNum = 0;
        pstUser->m_udwMarchNum = 0;
        pstUser->m_udwSelfAlActionNum = 0;

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            return -1;
        }
        return 0; 
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessOperate::ProcessCmd_BlockHacker(SSession* pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;
    ptbPlayer->Set_Dead_flag(1);

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TVOID CProcessOperate::CheckPlayerMap(SSession *pstSession, SUserInfo *pstUser, TbMap *ptbWildList, const TUINT32 udwWildNum)
{
    std::set<TINT32> sSpPointSet;
    sSpPointSet.clear();

    //以用户 city表数据为准 生成 正确city坐标集合
    TUINT32 udwRightCenterPos = pstUser->m_stCityInfo.m_stTblData.m_nPos;
    TUINT32 udwCenterPos = CCommonLogic::BuildingPointToPos(udwRightCenterPos / MAP_X_Y_POS_COMPUTE_OFFSET, udwRightCenterPos % MAP_X_Y_POS_COMPUTE_OFFSET);
    CCommonLogic::GetBuildingPos(udwCenterPos, 2, sSpPointSet);

    for(TUINT32 udwMapIndex = 0; udwMapIndex < udwWildNum; udwMapIndex++)
    {
        TbMap* ptbWild = &ptbWildList[udwMapIndex];
        if(ptbWild == NULL)
        {
            break;
        }

        //以用户city表数据为准 删除 世界地图上多余的城市 
        if(ptbWild->m_nType == EN_WILD_TYPE__CITY)
        {
            TBOOL bFindInPlayerData = FALSE;
            if(sSpPointSet.find(CCommonLogic::BuildingPointToPos(ptbWild->m_nId / MAP_X_Y_POS_COMPUTE_OFFSET, ptbWild->m_nId % MAP_X_Y_POS_COMPUTE_OFFSET)) != sSpPointSet.end())
            {
                bFindInPlayerData = TRUE;
            }

            if(bFindInPlayerData == FALSE)
            {
                TSE_LOG_ERROR(pstSession->m_poServLog, ("CheckMapCity: MapErrorInfo-City: sid=%u, uid=%u, deadflag=%lld, ctime=%lld, utime=%lld, pos=%lld [seq=%u]",
                    pstSession->m_stReqParam.m_udwSvrId, pstSession->m_stReqParam.m_udwUserId,
                    pstUser->m_tbPlayer.m_nDead_flag, pstUser->m_tbPlayer.m_nCtime, pstUser->m_tbPlayer.m_nUtime,
                    ptbWild->m_nId, pstSession->m_stUserInfo.m_udwBSeqNo));

                CCommonBase::AbandonWild(NULL, ptbWild);
            }
        }
        else
        {
            //以用户march 数据 为准 删除 世界地图上多余的野地
            if(ptbWild->m_nType == EN_WILD_TYPE__THRONE_NEW)
            {
                continue;
            }

            TBOOL bFindMarchPos = FALSE;
            for(TUINT32 udwMarchIndex = 0; udwMarchIndex < pstUser->m_udwMarchNum; udwMarchIndex++)
            {
                TbMarch_action* ptbMarch = &pstUser->m_atbMarch[udwMarchIndex];
                if(ptbMarch->m_nMclass != EN_ACTION_MAIN_CLASS__MARCH)
                {
                    continue;
                }
                if(ptbMarch->m_nStatus == EN_MARCH_STATUS__LOADING || ptbMarch->m_nStatus == EN_MARCH_STATUS__DEFENDING)
                {
                    if(ptbWild->m_nId == ptbMarch->m_nTpos)
                    {
                        bFindMarchPos = TRUE;
                        break;
                    }
                }
            }

            if(bFindMarchPos == FALSE)
            {
                TSE_LOG_ERROR(pstSession->m_poServLog, ("CheckMapCity: MapErrorInfo-Wild: sid=%u, uid=%u, deadflag=%lld, ctime=%lld, utime=%lld, pos=%lld [seq=%u]",
                    pstSession->m_stReqParam.m_udwSvrId, pstSession->m_stReqParam.m_udwUserId,
                    pstUser->m_tbPlayer.m_nDead_flag, pstUser->m_tbPlayer.m_nCtime, pstUser->m_tbPlayer.m_nUtime,
                    ptbWild->m_nId, pstSession->m_stUserInfo.m_udwBSeqNo));
                SCityInfo* pstCity = &pstUser->m_stCityInfo;

                CCommonBase::AbandonWild(pstCity, ptbWild);
            }
        }
    }
}

TVOID CProcessOperate::RecoverPlayerCity(SSession *pstSession, SUserInfo *pstUser, TbMap *ptbWildList, const TUINT32 udwWildNum)
{
    pstSession->ResetAwsInfo();
    SCityInfo* pstCity = &pstUser->m_stCityInfo;
    if(pstCity == NULL)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("RecoverPlayerCity: no city info![seq=%u]", pstSession->m_udwSeqNo));
        return;
    }
    
    TBOOL bCityInWorldMap = FALSE;
    TBOOL bUidRight = FALSE;
    for (TUINT32 udwMapIndex = 0; udwMapIndex < udwWildNum; udwMapIndex++)
    {
        TbMap* ptbWild = &ptbWildList[udwMapIndex];
        if (ptbWild == NULL)
        {
            break;
        }

        if (ptbWild->m_nId == pstCity->m_stTblData.m_nPos)
        {
            bUidRight = TRUE;
            if (ptbWild->m_nType == EN_WILD_TYPE__CITY)
            {
                bCityInWorldMap = TRUE;
                break;
            }
        }
    }

    if (bCityInWorldMap == FALSE)
    {
        TbMap tbTmpMap;
        tbTmpMap.Reset();
        tbTmpMap.Set_Id(pstCity->m_stTblData.m_nPos);
        tbTmpMap.Set_Utime(CTimeUtils::GetUnixTime());
        tbTmpMap.Set_Type(EN_WILD_TYPE__CITY);
        tbTmpMap.Set_Name(pstCity->m_stTblData.m_sName);
        tbTmpMap.Set_Status(pstUser->m_tbPlayer.m_nStatus);
        tbTmpMap.Set_Cid(pstCity->m_stTblData.m_nPos);
        tbTmpMap.Set_Uid(pstUser->m_tbPlayer.m_nUid);
        tbTmpMap.Set_Uname(pstUser->m_tbPlayer.m_sUin);
        tbTmpMap.Set_Ulevel(pstUser->m_tbPlayer.m_nLevel);
        tbTmpMap.Set_Alid(pstUser->m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET);
        tbTmpMap.Set_Alname(pstUser->m_tbPlayer.m_sAlname);
        tbTmpMap.Set_Al_nick(pstUser->m_tbPlayer.m_sAl_nick_name);
        tbTmpMap.Set_Npc(pstUser->m_tbPlayer.m_nNpc);
        tbTmpMap.Set_Name_update_time(CTimeUtils::GetUnixTime());
        tbTmpMap.Set_Bid(CMapBase::GetBlockIdFromPos(pstCity->m_stTblData.m_nPos));
        tbTmpMap.Set_Prison_flag(0);

        ExpectedItem expect_item;
        ExpectedDesc expect_desc;
        if (bUidRight)
        {
            expect_item.SetVal(TbMAP_FIELD_UID, TRUE, pstUser->m_tbPlayer.m_nUid);
        }
        else
        {
            expect_item.SetVal(TbMAP_FIELD_UID, TRUE, 0);
        }
        expect_desc.clear();
        expect_desc.push_back(expect_item);

        CAwsRequest::UpdateItem(pstSession, &tbTmpMap, expect_desc);
    }
}

TINT32 CProcessOperate::ResetMap(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwMapId = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TINT32 dwSid = atoi(pstSession->m_stReqParam.m_szKey[1]);

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        pstSession->ResetAwsInfo();
        TbMap tbMap;
        tbMap.Set_Id(dwMapId);
        tbMap.Set_Sid(dwSid);
        tbMap.Set_Rtype(0);
        CMapBase::ResetMap(&tbMap);
        CAwsRequest::UpdateItem(pstSession, &tbMap);
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        bNeedResponse = TRUE;
        TINT32 dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            return -1;
        }
        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessOperate::ClearNoPlayerMap(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwMapId = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TINT32 dwSid = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TINT32 dwUid = atoi(pstSession->m_stReqParam.m_szKey[2]);

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        if (dwUid == 0 || dwMapId == 0)
        {
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
            return 0;
        }

        pstSession->ResetAwsInfo();
        CAwsRequest::MapGetByIdAndSid(pstSession, dwMapId, dwSid);
        CAwsRequest::UserGetByUid(pstSession, dwUid);

        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        bNeedResponse = TRUE;
        TINT32 dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            return -1;
        }
        return 0;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        AwsRspInfo *pstRes = NULL;
        TbPlayer tbPlayer;
        TbMap tbMap;
        tbPlayer.Reset();
        tbMap.Reset();

        for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecAwsRsp.size(); ++udwIdx)
        {
            pstRes = pstSession->m_vecAwsRsp[udwIdx];

            string strTableRawName = CCommonFunc::GetTableRawName(pstRes->sTableName);
            if (strTableRawName == EN_AWS_TABLE_MAP)
            {
                CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[udwIdx],
                    &tbMap);
            }
            else if (strTableRawName == EN_AWS_TABLE_PLAYER)
            {
                CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[udwIdx],
                    &tbPlayer);
            }
        }

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        if (tbPlayer.m_nUid == 0 && tbMap.m_nUid == dwUid && tbMap.m_nId == dwMapId && tbMap.m_nType == EN_WILD_TYPE__CITY)
        {
            pstSession->ResetAwsInfo();
            tbMap.Reset();
            tbMap.Set_Id(dwMapId);
            tbMap.Set_Sid(dwSid);
            tbMap.Set_Rtype(0);
            CMapBase::ResetMap(&tbMap);
            CAwsRequest::UpdateItem(pstSession, &tbMap);

            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
            bNeedResponse = TRUE;
            TINT32 dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
            if (dwRetCode < 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                return -1;
            }
            return 0;
        }
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessOperate::ProcessCmd_HuPressureMeasure(SSession* pstSession, TBOOL& bNeedResponse)
{
    Json::Value NullJson;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TINT32 dwKey = atoi(pstSession->m_stReqParam.m_szKey[0]);
    ostringstream oss;
    oss << pstUser->m_tbPlayer.m_nUid;
    string sName1 = oss.str() + "asdf";
    string sName2 = oss.str();
    string sNesName = sName2;
    if(pstUser->m_tbPlayer.m_sUin == sName2)
    {
        sNesName = sName1;
    }

    TUINT32 udwProvinceId = rand() % 16;

    switch(dwKey % 6)
    {
    case 0:
        // next procedure
        pstSession->m_udwCommand = EN_CLIENT_REQ_COMMAND__REFRESH_TIME_QUEST;
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__INIT;

        //quest_start 
        CPlayerBase::AddGem(&pstUser->m_tbLogin, 1);
        strncpy(&pstSession->m_stReqParam.m_szKey[0][0], CCommonFunc::NumToString(1).c_str(), DEFAULT_PARAM_STR_LEN - 1);
        pstSession->m_stReqParam.m_szKey[0][DEFAULT_PARAM_STR_LEN - 1] = 0;

        strncpy(&pstSession->m_stReqParam.m_szKey[1][0], CCommonFunc::NumToString(1).c_str(), DEFAULT_PARAM_STR_LEN - 1);
        pstSession->m_stReqParam.m_szKey[1][DEFAULT_PARAM_STR_LEN - 1] = 0;

        pstSession->m_bGotoOtherCmd = TRUE;
        return 0;
        break;
    case 1:
        // next procedure
        pstSession->m_udwCommand = EN_CLIENT_REQ_COMMAND__ITEM_BUY_AND_USE;
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__INIT;

        //item buy and use 
        CPlayerBase::AddGem(&pstUser->m_tbLogin,1);
        strncpy(&pstSession->m_stReqParam.m_szKey[0][0], CCommonFunc::NumToString(170).c_str(), DEFAULT_PARAM_STR_LEN - 1);
        pstSession->m_stReqParam.m_szKey[0][DEFAULT_PARAM_STR_LEN - 1] = 0;

        strncpy(&pstSession->m_stReqParam.m_szKey[1][0], CCommonFunc::NumToString(0).c_str(), DEFAULT_PARAM_STR_LEN - 1);
        pstSession->m_stReqParam.m_szKey[1][DEFAULT_PARAM_STR_LEN - 1] = 0;

        strncpy(&pstSession->m_stReqParam.m_szKey[2][0], CCommonFunc::NumToString(1).c_str(), DEFAULT_PARAM_STR_LEN - 1);
        pstSession->m_stReqParam.m_szKey[2][DEFAULT_PARAM_STR_LEN - 1] = 0;

        strncpy(&pstSession->m_stReqParam.m_szKey[3][0], CCommonFunc::NumToString(0).c_str(), DEFAULT_PARAM_STR_LEN - 1);
        pstSession->m_stReqParam.m_szKey[3][DEFAULT_PARAM_STR_LEN - 1] = 0;

        strncpy(&pstSession->m_stReqParam.m_szKey[4][0], CCommonFunc::NumToString(0).c_str(), DEFAULT_PARAM_STR_LEN - 1);
        pstSession->m_stReqParam.m_szKey[4][DEFAULT_PARAM_STR_LEN - 1] = 0;

        strncpy(&pstSession->m_stReqParam.m_szKey[5][0], CCommonFunc::NumToString(pstSession->m_stReqParam.m_udwSvrId).c_str(), DEFAULT_PARAM_STR_LEN - 1);
        pstSession->m_stReqParam.m_szKey[5][DEFAULT_PARAM_STR_LEN - 1] = 0;

        pstSession->m_bGotoOtherCmd = TRUE;
        return 0;
        break;
    case 2:
        // next procedure
        pstSession->m_udwCommand = EN_CLIENT_REQ_COMMAND__CHANGE_BASE_NAME;
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__INIT;

        //change city name
        CPlayerBase::AddGem(&pstUser->m_tbLogin, 1);
        strncpy(&pstSession->m_stReqParam.m_szKey[0][0], "Xesiafa", DEFAULT_PARAM_STR_LEN - 1);
        pstSession->m_stReqParam.m_szKey[0][DEFAULT_PARAM_STR_LEN - 1] = 0;

        strncpy(&pstSession->m_stReqParam.m_szKey[1][0], CCommonFunc::NumToString(1).c_str(), DEFAULT_PARAM_STR_LEN - 1);
        pstSession->m_stReqParam.m_szKey[1][DEFAULT_PARAM_STR_LEN - 1] = 0;

        strncpy(&pstSession->m_stReqParam.m_szKey[2][0], CCommonFunc::NumToString(0).c_str(), DEFAULT_PARAM_STR_LEN - 1);
        pstSession->m_stReqParam.m_szKey[2][DEFAULT_PARAM_STR_LEN - 1] = 0;

        pstSession->m_bGotoOtherCmd = TRUE;
        return 0;
        break;
    case 3:
        // next procedure
        pstSession->m_udwCommand = EN_CLIENT_REQ_COMMAND__CHANGE_PLAYER_NAME;
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__INIT;

        //change player name
        CPlayerBase::AddGem(&pstUser->m_tbLogin, 1);
        
        strncpy(&pstSession->m_stReqParam.m_szKey[0][0], sNesName.c_str(), DEFAULT_PARAM_STR_LEN - 1);
        pstSession->m_stReqParam.m_szKey[0][DEFAULT_PARAM_STR_LEN - 1] = 0;

        strncpy(&pstSession->m_stReqParam.m_szKey[1][0], CCommonFunc::NumToString(1).c_str(), DEFAULT_PARAM_STR_LEN - 1);
        pstSession->m_stReqParam.m_szKey[1][DEFAULT_PARAM_STR_LEN - 1] = 0;

        strncpy(&pstSession->m_stReqParam.m_szKey[2][0], CCommonFunc::NumToString(0).c_str(), DEFAULT_PARAM_STR_LEN - 1);
        pstSession->m_stReqParam.m_szKey[2][DEFAULT_PARAM_STR_LEN - 1] = 0;

        pstSession->m_bGotoOtherCmd = TRUE;
        return 0;
        break;
    case 4:
    case 5:
        // next procedure
        pstSession->m_udwCommand = EN_CLIENT_REQ_COMMAND__RANDOM_MOVE_CITY;
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__INIT;

        CItemBase::AddItem(&pstUser->m_tbBackpack, EN_ITEM_ID__PORTAL_OF_REFUGE);
        //randon move city
        strncpy(&pstSession->m_stReqParam.m_szKey[0][0], CCommonFunc::NumToString(udwProvinceId).c_str(), DEFAULT_PARAM_STR_LEN - 1);
        pstSession->m_stReqParam.m_szKey[0][DEFAULT_PARAM_STR_LEN - 1] = 0;

        pstSession->m_bGotoOtherCmd = TRUE;
        return 0;
        break;
    default:
        break;
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessOperate::ProcessCmd_AuPressureMeasure(SSession* pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    UActionParam stParam;
    stParam.Reset();
    SCityInfo *pstCity = &pstSession->m_stUserInfo.m_stCityInfo;
    if(pstCity == NULL)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -2;
    }

    //status 复用march status 
    CCommonBase::AddAction(pstUser, pstCity, EN_ACTION_MAIN_CLASS__AU_PRESSURE_MEASURE, EN_ACTION_SEC_CLASS__AU_PRESSURE_MEASURE,
        EN_MARCH_STATUS__MARCHING, 5, &stParam);

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessOperate::ProcessCmd_CleanAuPressureAction(SSession* pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwActionNum;++udwIdx)
    {
        TbAction *pstAction = &pstUser->m_atbAction[udwIdx];
        if(pstAction->m_nMclass == EN_ACTION_MAIN_CLASS__AU_PRESSURE_MEASURE &&
            pstAction->m_nSclass == EN_ACTION_SEC_CLASS__AU_PRESSURE_MEASURE)
        {
            pstUser->m_aucActionFlag[udwIdx] = EN_TABLE_UPDT_FLAG__DEL;
        }
    }
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}



TINT32 CProcessOperate::ProcessCmd_CacheTest(SSession* pstSession, TBOOL& bNeedResponse)
{
    TINT32 dwRetCode = 0;
    TINT32 dwOperateType = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TINT32 dwAid = atoi(pstSession->m_stReqParam.m_szKey[1]);


    // 1.信息
    if(EN_COMMAND_STEP__INIT == pstSession->m_udwCommandStep)
    {

        if(0 == dwOperateType)
        {
            // reset req
            pstSession->ResetCacheReq();
            // next procedure
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__CACHE;

        
            // send request
            bNeedResponse = TRUE;
            CacheReqInfo *pstCacheReq = new CacheReqInfo;
            pstCacheReq->SetVal("al_member", "get", dwAid, "");       
            pstSession->m_vecCacheReq.push_back(pstCacheReq);

            //log
            TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("ProcessCmd_CacheTest: [CacheName=%s] [CacheOperate=%s] [uddwCacheKey=%ld] [CacheValue=%s] [seq=%u]",
                                                               pstCacheReq->m_strCacheName.c_str(), \
                                                               pstCacheReq->m_strCacheOperate.c_str(), \
                                                               pstCacheReq->m_uddwCacheKey, \
                                                               pstCacheReq->m_strCacheValue.c_str(), \
                                                               pstSession->m_udwSeqNo));

            dwRetCode = CBaseProcedure::SendCacheRequest(pstSession, EN_SERVICE_TYPE_CACHE_REQ);
            if(dwRetCode != 0)
            {        
                bNeedResponse = FALSE;
                pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
                // pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_CACHE_REQ_ERR;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_CacheTest: send cache req fail [ret=%d] [seq=%u]", dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
                // return -1;
                return 0;
            }

            return 0;
        }
        else if(1 == dwOperateType)
        {   
            // 获取al_member表
            // reset req
            pstSession->ResetAwsReq();
            // next procedure
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

            // set player get
            CAwsRequest::AllAlMemberQuery(pstSession, dwAid);

            // send request
            bNeedResponse = TRUE;
            dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
            if (dwRetCode < 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_CacheTest: send aws req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                return -1;
            }
            return 0;

        }
        else
        {
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
            // pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_CACHE_REQ_ERR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_CacheTest: error operate type [dwOperateType=%d] [seq=%u]", dwOperateType, pstSession->m_stUserInfo.m_udwBSeqNo));
            // return -1;
            
            return 0;
        }
    }


    if(EN_COMMAND_STEP__1 == pstSession->m_udwCommandStep)
    {
        // reset req
        pstSession->ResetCacheReq();
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__CACHE;
        
        // send request
        bNeedResponse = TRUE;
        CacheReqInfo *pstCacheReq = new CacheReqInfo;
        pstCacheReq->SetVal("al_member", "set", dwAid, pstSession->m_vecAwsRsp[0]->sRspContent);       
        pstSession->m_vecCacheReq.push_back(pstCacheReq);

        //log
        TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("ProcessCmd_CacheTest: [CacheName=%s] [CacheOperate=%s] [uddwCacheKey=%lu] [CacheValue=%s] [seq=%u]",
                                                           pstCacheReq->m_strCacheName.c_str(), \
                                                           pstCacheReq->m_strCacheOperate.c_str(), \
                                                           pstCacheReq->m_uddwCacheKey, \
                                                           pstCacheReq->m_strCacheValue.c_str(), \
                                                           pstSession->m_udwSeqNo));

        dwRetCode = CBaseProcedure::SendCacheRequest(pstSession, EN_SERVICE_TYPE_CACHE_REQ);
        if(dwRetCode != 0)
        {
            bNeedResponse = FALSE;
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
            // pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_CACHE_REQ_ERR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_CacheTest: send cache req fail [ret=%d] [seq=%u]", dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
            // return -1;
            return 0;
        }

        return 0;
    }




    if(EN_COMMAND_STEP__2 == pstSession->m_udwCommandStep)
    {
        if(pstSession->m_vecCacheRsp.size() > 0)
        {
            if(pstSession->m_vecCacheRsp[0]->dwRetCode != 0)
            {
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_CacheTest: parse cache info fail [seq=%u]", \
                                                        pstSession->m_stUserInfo.m_udwBSeqNo));
            }
            else
            {
                TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessCmd_CacheTest: [ServiceType=%u] [CacheName=%s] [CacheOperate=%s] [CacheKey=%lld] [CacheValue=%s] [CacheExist=%d] [seq=%u]", \
                                                       pstSession->m_vecCacheRsp[0]->udwServiceType, \
                                                       pstSession->m_vecCacheRsp[0]->m_strCacheName.c_str(), \
                                                       pstSession->m_vecCacheRsp[0]->m_strCacheOperate.c_str(), \
                                                       pstSession->m_vecCacheRsp[0]->m_uddwCacheKey, \
                                                       pstSession->m_vecCacheRsp[0]->m_strCacheValue.c_str(), \
                                                       pstSession->m_vecCacheRsp[0]->m_bExist, \
                                                       pstSession->m_stUserInfo.m_udwBSeqNo));
                if("al_member" == pstSession->m_vecCacheRsp[0]->m_strCacheName)
                {
                    if("get" == pstSession->m_vecCacheRsp[0]->m_strCacheOperate)
                    {
                        if(TRUE == pstSession->m_vecCacheRsp[0]->m_bExist)
                        {
                            TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessCmd_CacheTest: cache info exist [seq=%u]", \
                                                                    pstSession->m_stUserInfo.m_udwBSeqNo));
                        }
                        else
                        {
                            // 获取al_member表
                            // reset req
                            pstSession->ResetAwsReq();
                            // next procedure
                            pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;
                            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

                            // set player get
                            CAwsRequest::AllAlMemberQuery(pstSession, dwAid);

                            // send request
                            bNeedResponse = TRUE;
                            dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
                            if (dwRetCode < 0)
                            {
                                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_CacheTest: send aws req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                                return -1;
                            }
                            return 0;
                        }
                    }
                    else if("set" == pstSession->m_vecCacheRsp[0]->m_strCacheOperate)
                    {
                        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessCmd_CacheTest: set cache info success [seq=%u]", \
                                                                pstSession->m_stUserInfo.m_udwBSeqNo));
                    }
                    else
                    {
                        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_CacheTest: error cache operate [CacheOperate=%s] [seq=%u]", \
                                                                pstSession->m_vecCacheRsp[0]->m_strCacheOperate.c_str(), \
                                                                pstSession->m_stUserInfo.m_udwBSeqNo));
                    }
                }
                else
                {
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_CacheTest: error cache name [CacheName=%s] [seq=%u]", \
                                                            pstSession->m_vecCacheRsp[0]->m_strCacheName.c_str(), \
                                                            pstSession->m_stUserInfo.m_udwBSeqNo));
                }

                
            }
        }
        else
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_CacheTest: not cache response [seq=%u]", \
                                                    pstSession->m_stUserInfo.m_udwBSeqNo));

        }
    }

    if(EN_COMMAND_STEP__3 == pstSession->m_udwCommandStep)
    {
        // reset req
        pstSession->ResetCacheReq();
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__CACHE;
        
        // send request
        bNeedResponse = TRUE;
        CacheReqInfo *pstCacheReq = new CacheReqInfo;
        pstCacheReq->SetVal("al_member", "set", dwAid, pstSession->m_vecAwsRsp[0]->sRspContent);       
        pstSession->m_vecCacheReq.push_back(pstCacheReq);

        //log
        TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("ProcessCmd_CacheTest: [CacheName=%s] [CacheOperate=%s] [uddwCacheKey=%ldd] [CacheValue=%s] [seq=%u]",
                                                           pstCacheReq->m_strCacheName.c_str(), \
                                                           pstCacheReq->m_strCacheOperate.c_str(), \
                                                           pstCacheReq->m_uddwCacheKey, \
                                                           pstCacheReq->m_strCacheValue.c_str(), \
                                                           pstSession->m_udwSeqNo));

        dwRetCode = CBaseProcedure::SendCacheRequest(pstSession, EN_SERVICE_TYPE_CACHE_REQ);
        if(dwRetCode != 0)
        {
            bNeedResponse = FALSE;
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
            // pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_CACHE_REQ_ERR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_CacheTest: send cache req fail [ret=%d] [seq=%u]", dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
            // return -1;
            return 0;
        }

        return 0;
    }


    
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;

}


TINT32 CProcessOperate::Processcmd_AccountOperate(SSession* pstSession, TBOOL& bNeedResponse)
{
    return 0;
}

TINT32 CProcessOperate::ProcessCmd_CreateNewPlayerAlliance(SSession* pstSession, TBOOL& bNeedResponse)
{
    /*
    TINT32 dwRetCode = 0;
    TbAlliance *ptbAlliance = &pstSession->m_stUserInfo.m_tbAlliance;
    TbPlayer *ptbPlayer = &pstSession->m_stUserInfo.m_tbPlayer;
    SCityInfo *pstCity = &pstSession->m_stUserInfo.m_stCityInfo;

    // 0. 输入参数
    TCHAR *pszAllianceName = &pstSession->m_stReqParam.m_szKey[0][0];
    TCHAR *pszAllianceNick = &pstSession->m_stReqParam.m_szKey[1][0];
    TCHAR *pszAllianceDesc = &pstSession->m_stReqParam.m_szKey[2][0];
    TUINT32 udwPolicy = EN_ALLIANCE_JOIN_AUTO;

    // 1.1 使用名称查找该名称是否已经被使用
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        // check param
        if (ptbPlayer->m_nAlpos != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ALREADY_IN_ALLIANCE;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_CreateNewPlayerAlliance: user already has alliance[%u:%s] [seq=%u]", 
                ptbPlayer->m_nAlid, ptbPlayer->m_sAlname.c_str(), pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
        TUINT32 udwAllianceNameLen = strlen(pszAllianceName);
        if (udwAllianceNameLen < 3)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_CreateNewPlayerAlliance: req alliance name[%s] [seq=%u]", 
                pszAllianceName, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }

        pstSession->ResetAwsInfo();

        // get check name
        TbUnique_name tbUnique_name;
        tbUnique_name.Set_Type(EN_ALLIANCE_NAME);
        tbUnique_name.Set_Name(pszAllianceName);
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
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_CreateNewPlayerAlliance: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }

        return 0;
    }

    // 1.2. 处理响应
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        TbUnique_name tbUniqueName;
        tbUniqueName.Reset();

        dwRetCode = CAwsResponse::OnUpdateItemRsp(*pstSession->m_vecAwsRsp[0], &tbUniqueName);
        if (0 == dwRetCode)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ALLIANCE_NAME_BE_USED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_CreateNewPlayerAlliance: alliance name already exist, createfailed [dwRetCode=%d] [seq=%u]", 
                dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }
    }

    // 1.3 简称是否存在
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
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
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_CreateNewPlayerAlliance: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -4;
        }

        return 0;
    }

    // 1.2. 处理响应
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        TbUnique_name tbUniqueName;
        tbUniqueName.Reset();

        dwRetCode = CAwsResponse::OnUpdateItemRsp(*pstSession->m_vecAwsRsp[0], &tbUniqueName);
        if (0 == dwRetCode)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_CreateNewPlayerAlliance: alliance nick name already exist, createfailed [dwRetCode=%d] [seq=%u]", 
                dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;
        }
        else
        {
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__5;
        }
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        pstSession->ResetAwsInfo();

        // get check name
        TbUnique_name tbUnique_name;
        tbUnique_name.Set_Type(EN_ALLIANCE_NAME);
        tbUnique_name.Set_Name(pszAllianceName);

        CAwsRequest::DeleteItem(pstSession, &tbUnique_name);

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__4;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        // send
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_CreateNewPlayerAlliance: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -5;
        }

        return 0;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__4)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ALLIANCE_NICK_BE_USED;
        return -6;
    }

    // 2. 发送获取联盟id的请求
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__5)
    {
        // 发送获取联盟id的请求
        pstSession->ResetAwsInfo();
        CAwsRequest::GetGlobalNewId(pstSession, EN_GLOBAL_PARAM__ALLIANCE_ID);
        //CAwsRequest::GetGlobalNewId(pstSession, EN_SVR_PARAM__ALLIANCE_COUNT);

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__6;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_CreateNewPlayerAlliance: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -7;
        }
        return 0;
    }

    // 2.2 解析响应
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__6)
    {
        // parse data
        TINT32 dwAlId = 0;
        TINT32 dwAlCount = 0;
        TbParam tbParamItem;
        for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecAwsRsp.size(); ++udwIdx)
        {
            dwRetCode = CAwsResponse::OnUpdateItemRsp(*pstSession->m_vecAwsRsp[udwIdx], &tbParamItem);
            if (dwRetCode > 0)
            {
                if (tbParamItem.m_nKey == EN_GLOBAL_PARAM__ALLIANCE_ID)
                {
                    dwAlId = tbParamItem.m_nVal;
                }
                if (tbParamItem.m_nKey == EN_SVR_PARAM__ALLIANCE_COUNT)
                {
                    dwAlCount = tbParamItem.m_nVal;
                }
            }
            else
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_PACKAGE_ERR;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_CreateNewPlayerAlliance: get alliance id failed[%d] [seq=%u]", dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
                return -8;
            }
        }

        // update alliance info
        ptbAlliance->Reset();
        ptbAlliance->Set_Aid(dwAlId);
        ptbAlliance->Set_Name(pszAllianceName);
        ptbAlliance->Set_Oid(ptbPlayer->m_nUid);
        ptbAlliance->Set_Oname(ptbPlayer->m_sUin);
        ptbAlliance->Set_Member(1);
        ptbAlliance->Set_Might(ptbPlayer->m_nMight);
        ptbAlliance->Set_Force_kill(ptbPlayer->m_nMkill);
        ptbAlliance->Set_Sid(pstSession->m_stReqParam.m_udwSvrId);
        ptbAlliance->Set_Desc(pszAllianceDesc);
        ptbAlliance->Set_Al_nick_name(pszAllianceNick);
        ptbAlliance->Set_Al_star(3);

//         if (ptbAlliance->m_sDesc.empty())
//         {
//             ptbAlliance->Set_Desc("There is no description now.");
//         }
        ptbAlliance->Set_Language(0);
        ptbAlliance->Set_Policy(udwPolicy*ALLIANCE_POLICY_OFFSET + 1);
        //ptbAlliance->Set_Notice("Welcome to our Alliance!");
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
        ptbPlayer->Set_Al_time(CTimeUtils::GetUnixTime());
        ptbPlayer->Set_Invite_mail_time(CTimeUtils::GetUnixTime());
        ptbPlayer->Set_Alname_update_time(CTimeUtils::GetUnixTime());

        if (pstSession->m_stUserInfo.m_tbLogin.m_nAl_time == 0)
        {
            ptbPlayer->Set_Status(ptbPlayer->m_nStatus | EN_CITY_STATUS__FIRST_ALLIANCE_REWARD);
        }

        CMsgBase::AddAlRank(ptbAlliance);
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__7;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__7)
    {
        pstSession->ResetAwsInfo();

        // get check name
        TbUnique_name tbUnique_name;
        tbUnique_name.Set_Type(EN_ALLIANCE_NAME);
        tbUnique_name.Set_Name(pszAllianceName);
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

        if (pstSession->m_dwOldRequestAid > 0)
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
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__AllianceCreate: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -10;
        }

        return 0;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__8)
    {
        //task count
        CQuestLogic::SetTaskCurValue(&pstSession->m_stUserInfo, pstCity, EN_TASK_TYPE_JOIN_ALLIANCE);

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    */
    return -1;
}

TINT32 CProcessOperate::ProcessCmd_SendRallyPridict(SSession* pstSession, TBOOL& bNeedResponse)
{
    //废弃
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return -1;
}

TINT32 CProcessOperate::ProcessCmd_SetHelpBubble(SSession* pstSession, TBOOL& bNeedResponse)
{
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();

    pstSession->ResetAwsInfo();

    TbParam tbParam;
    tbParam.Reset();
    tbParam.Set_Key(EN_GLOBAL_PARAM__HELP_BUBBLE_TIME);
    tbParam.Set_Val(udwCurTime);

    CAwsRequest::UpdateItem(pstSession, &tbParam, ExpectedDesc());
    pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
    TINT32 dwRet = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);

    if (dwRet < 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_SetHelpBubble: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }

    TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_SetHelpBubble: Help bubble status on. [time=%u][ret=%d] [seq=%u]", udwCurTime, dwRet, pstSession->m_udwSeqNo));

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessOperate::ProcessCmd_WarningTest(SSession *pstSession, TBOOL &bNeedResponse)
{
    ostringstream ossContent;

    // endpoint
    ossContent.str("");
    ossContent << CGlobalServ::m_poConf->m_szModuleIp << ":" << CConfBase::GetInt("query_port");
    string strEndpoint = ossContent.str().c_str();

    // tags
    ossContent.str("");    
    ossContent << "proj=" << CConfBase::GetString("project") << "," << "mod=" << CGlobalServ::m_poConf->m_szModuleName;
    string strTags = ossContent.str().c_str();

    
    Json::FastWriter jsonWriter;
    Json::Value jPostDataBody = Json::Value(Json::arrayValue);
    Json::Value jSignalPostDataBody = Json::Value(Json::objectValue);
        
    jSignalPostDataBody["metric"] = "snow";
    jSignalPostDataBody["endpoint"] = strEndpoint;    
    jSignalPostDataBody["tags"] = strTags;    
    jSignalPostDataBody["value"] = 10;   
    jSignalPostDataBody["step"] = 60;
    jSignalPostDataBody["counterType"] = "GAUGE";
    jSignalPostDataBody["timestamp"] = CTimeUtils::GetUnixTime();    

    jPostDataBody.append(jSignalPostDataBody);

    string strPostData = jsonWriter.write(jPostDataBody);
    
    CWarningMgr::GetInstance()->Send_Warning(strPostData);
    
    TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessCmd_WarningTest: warning post data [%s] [seq=%u]", \
                                            strPostData.c_str(), \
                                            pstSession->m_stUserInfo.m_udwBSeqNo));

    
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}


TINT32 CProcessOperate::ProcessCmd_TranslateTest(SSession *pstSession, TBOOL &bNeedResponse)
{
    if(EN_COMMAND_STEP__INIT == pstSession->m_udwCommandStep)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
    }

    if(EN_COMMAND_STEP__1 == pstSession->m_udwCommandStep)
    {  
        TINT32 dwRetCode = 0;
        
        // reset req
        pstSession->ResetTranslateReq();
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__TRANSLATE;
        
        // send request
        bNeedResponse = TRUE;
    	Json::FastWriter oJsonWriter;
    	oJsonWriter.omitEndingLineFeed();
    	Json::Value rTranslateJson = Json::Value(Json::objectValue);

        for(TINT32 dwIdx = 0; dwIdx < CDocument::GetInstance()->GetSupportLangNum(); ++dwIdx)
        {
            TranslateReqInfo *pstTranslateReq = new TranslateReqInfo;
            rTranslateJson.clear();
            rTranslateJson["0"]["from"] = "";
            rTranslateJson["0"]["to"] = CDocument::GetInstance()->GetShortLangName(dwIdx);
            rTranslateJson["0"]["content"] = "hi";
            rTranslateJson["1"]["from"] = "";
            rTranslateJson["1"]["to"] = CDocument::GetInstance()->GetShortLangName(dwIdx);
            rTranslateJson["1"]["content"] = "good";
            pstTranslateReq->SetVal("mail", "translate", oJsonWriter.write(rTranslateJson));       
            pstSession->m_vecTranslateReq.push_back(pstTranslateReq);

            //log    
            TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessCmd_TranslateTest: [TranslateType=%s] [TranslateOperate=%s] [TranslateReqContent=%s] [seq:%u]", \
                                                  pstTranslateReq->m_strTranslateType.c_str(), \
                                                  pstTranslateReq->m_strTranslateOperate.c_str(), \
                                                  pstTranslateReq->m_strTranslateContent.c_str(), \
                                                  pstSession->m_stUserInfo.m_udwBSeqNo));
                           
        }

        

        dwRetCode = CBaseProcedure::SendTranslateRequest(pstSession, EN_SERVICE_TYPE_TRANSLATE_REQ);
        if(dwRetCode != 0)
        {        
            bNeedResponse = FALSE;
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
            pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_USER_JSON;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_TranslateTest: send translate req fail [ret=%d] [seq=%u]", \
                                                    dwRetCode, \
                                                    pstSession->m_stUserInfo.m_udwBSeqNo));
         }
        
        return 0;

    }

    if(EN_COMMAND_STEP__2 == pstSession->m_udwCommandStep)
    {

        TBOOL bTranslateFalg = FALSE;
        if((TUINT32)CDocument::GetInstance()->GetSupportLangNum() == pstSession->m_vecTranslateRsp.size()
           && 0 != pstSession->m_vecTranslateRsp.size())
        {
            bTranslateFalg = TRUE;
        }
        for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecTranslateRsp.size(); ++udwIdx)
        {
            if(0 != pstSession->m_vecTranslateRsp[udwIdx]->m_dwRetCode)
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
        if(TRUE == bTranslateFalg)
        {
            for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecTranslateRsp.size(); ++udwIdx)
            {
                Json::Reader jsonReader;
                Json::Value jResultBody;
                if(jsonReader.parse(pstSession->m_vecTranslateRsp[udwIdx]->m_strTranslateResult.c_str(), jResultBody))
                { 
                    if(0 == udwIdx)
                    {
                        dwLanguageId = atoi(CDocument::GetInstance()->GetLanguageId(jResultBody["0"]["from"].asString().c_str(), pstSession->m_stReqParam.m_udwLang).c_str());
                    }
                    Json::Value::Members vecMembers = jResultBody.getMemberNames();
                    for(TUINT32 udwIdy = 0; udwIdy < vecMembers.size(); ++udwIdy)
                    {
                        rTranslateJson[CDocument::GetInstance()->GetLangId(jResultBody[vecMembers[0]]["to"].asString()).c_str()][vecMembers[udwIdy]] = jResultBody[vecMembers[udwIdy]]["content"];
                    }
                }
                
            }
            strTranslateContent = oJsonWriter.write(rTranslateJson);
        }


        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessCmd_TranslateTest: [strTranslateContent=%s] [seq=%u]", \
                                              strTranslateContent.c_str(), \
                                              pstSession->m_stUserInfo.m_udwBSeqNo));

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessOperate::ProcessGenTaxAction(SSession *pstSession, TBOOL &bNeedResponse)
{
    if (pstSession->m_stUserInfo.m_tbAlliance.m_nAid != 0
        && pstSession->m_stUserInfo.m_tbAlliance.m_nAid == pstSession->m_tbThrone.m_nAlid
        && pstSession->m_stUserInfo.m_tbAlliance.m_nOid == pstSession->m_stUserInfo.m_tbPlayer.m_nUid)
    {
        TBOOL bIsFind = FALSE;
        for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_stUserInfo.m_udwMarchNum; udwIdx++)
        {
            if (pstSession->m_stUserInfo.m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__TAX
                && pstSession->m_stUserInfo.m_atbMarch[udwIdx].m_nSuid == pstSession->m_stUserInfo.m_tbPlayer.m_nUid)
            {
                bIsFind = TRUE;
                break;
            }
        }

        if (bIsFind == FALSE)
        {
            CActionBase::GenTaxAction(&pstSession->m_stUserInfo, pstSession->m_tbThrone.m_nPos);
        }
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessOperate::ProcessSetIapKey(SSession *pstSession, TBOOL &bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbLogin *ptbLogin = &pstUser->m_tbLogin;

    TINT32 dwMode = atoi(pstSession->m_stReqParam.m_szKey[0]); // 0:设置 1：清空
    TINT32 dwGemBuy = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TINT32 dwMaxBuy = atoi(pstSession->m_stReqParam.m_szKey[2]);
    TINT32 dwLastBuy = atoi(pstSession->m_stReqParam.m_szKey[3]);
    TINT32 dwTotalPay = atoi(pstSession->m_stReqParam.m_szKey[4]);
    TINT32 dwMaxPay = atoi(pstSession->m_stReqParam.m_szKey[5]);
    TINT32 dwLastPay = atoi(pstSession->m_stReqParam.m_szKey[6]);
    TINT32 dwLastBuyTime = atoi(pstSession->m_stReqParam.m_szKey[7]);

    if (dwMode == 0)
    {
        if (dwGemBuy != 0)
        {
            ptbLogin->Set_Gem_buy(dwGemBuy);
        }
        if (dwMaxBuy != 0)
        {
            ptbLogin->Set_Max_buy(dwMaxBuy);
        }
        if (dwLastBuy != 0)
        {
            ptbLogin->Set_Last_buy(dwLastBuy);
        }
        if (dwTotalPay != 0)
        {
            ptbLogin->Set_Total_pay(dwTotalPay);
        }
        if (dwMaxPay != 0)
        {
            ptbLogin->Set_Max_pay(dwMaxPay);
        }
        if (dwLastPay != 0)
        {
            ptbLogin->Set_Last_pay(dwLastPay);
        }
        if (dwLastBuyTime != 0)
        {
            ptbLogin->Set_Last_buy_time(dwLastBuyTime);
        }
    }
    else if (dwMode == 1)
    {
        if (dwGemBuy != 0)
        {
            ptbLogin->Set_Gem_buy(0);
        }
        if (dwMaxBuy != 0)
        {
            ptbLogin->Set_Max_buy(0);
        }
        if (dwLastBuy != 0)
        {
            ptbLogin->Set_Last_buy(0);
        }
        if (dwTotalPay != 0)
        {
            ptbLogin->Set_Total_pay(0);
        }
        if (dwMaxPay != 0)
        {
            ptbLogin->Set_Max_pay(0);
        }
        if (dwLastPay != 0)
        {
            ptbLogin->Set_Last_pay(0);
        }
        if (dwLastBuyTime != 0)
        {
            ptbLogin->Set_Last_buy_time(0);
        }
    }

    return 0;
}

TINT32 CProcessOperate::ProcessCmd_ChangeSvr(SSession *pstSession, TBOOL &bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;

    TbLogin *ptbLogin = &pstUser->m_tbLogin;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;

    SCityInfo *pstCity = &pstUser->m_stCityInfo;

    TbAlliance *ptbAlliance = &pstUser->m_tbAlliance;

    TbSvr_al *ptbSvrAl = &pstUser->m_tbSvrAl;

    TINT32 dwNewSvrId = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TINT32 dwNewCityPos = atoi(pstSession->m_stReqParam.m_szKey[1]);

    if (dwNewSvrId == ptbLogin->m_nSid)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    /*
    al_assist:
        只处理自己的
        删除
    */
    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwAlAssistAllNum; udwIdx++)
    {
        if (pstUser->m_atbAlAssistAll[udwIdx].m_nUid == ptbPlayer->m_nUid)
        {
            pstUser->m_aucAlAssistAllFlag[udwIdx] = EN_TABLE_UPDT_FLAG__DEL;
        }
    }

    /*
    player:
        重设sid
        重设citylist
        修改svr_change_time
        英雄恢复normal
    */
    ptbPlayer->Set_Sid(dwNewSvrId);
    ptbPlayer->Set_Invite_mail_time(CTimeUtils::GetUnixTime());
    ptbPlayer->Set_Svr_change_time(CTimeUtils::GetUnixTime());
    ptbPlayer->Set_Status(pstUser->m_tbPlayer.m_nStatus & (~EN_CITY_STATUS__ON_SPECIAL_WILD));

    if (ptbPlayer->m_nDragon_status != EN_DRAGON_STATUS_DEAD
        && ptbPlayer->m_nDragon_status != EN_DRAGON_STATUS_NORMAL)
    {
        ptbPlayer->Set_Dragon_status(EN_DRAGON_STATUS_NORMAL);
        ptbPlayer->Set_Dragon_tid(0);
    }
    ptbPlayer->m_nCid = dwNewCityPos;

    /*
    alliance:
        盟主执行
        重设sid
        重设throne_pos, throne_status, hive_show_flag, hive_pos, hive_svr
    */
    if (ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET > 0
        && ptbPlayer->m_nAlpos == EN_ALLIANCE_POS__CHANCELLOR)
    {
        ptbAlliance->Set_Sid(dwNewSvrId);
        ptbAlliance->Set_Hive_pos(0);
        ptbAlliance->Set_Hive_svr(dwNewSvrId);
        ptbAlliance->Set_Hive_show_flag(0);
        ptbAlliance->Set_Throne_pos(0);
        ptbAlliance->Set_Throne_status(EN_ALLIANCE_THRONE_STATUS_NONE);
    }

    /*
    login:
        重设sid
    */
    ptbLogin->Set_Sid(dwNewSvrId);

    /*
    svr_al:
        重建, 旧的不管
    */
    if (ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET > 0
        && ptbPlayer->m_nAlpos == EN_ALLIANCE_POS__CHANCELLOR)
    {
        ptbSvrAl->Set_Sid(dwNewSvrId);
        ptbSvrAl->Set_Alid(ptbAlliance->m_nAid);
        ptbSvrAl->Set_Owner_uid(ptbPlayer->m_nUid);
        ptbSvrAl->Set_Owner_cid(dwNewCityPos);
        pstUser->m_aucSvrAlFlag = EN_TABLE_UPDT_FLAG__CHANGE;
    }

    /*
    city:
        做移城
        清空旧wild
    */
    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwWildNum; udwIdx++)
    {
        if (pstUser->m_atbWild[udwIdx].m_nUid == ptbPlayer->m_nUid)
        {
            pstUser->m_atbWild[udwIdx].Set_Type(EN_WILD_TYPE__NORMAL);
            CMapBase::ResetMap(&pstUser->m_atbWild[udwIdx]);
            pstUser->m_aucWildFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
        }
    }
    pstSession->m_stReqParam.m_udwCityId = dwNewCityPos;
    pstCity->m_stTblData.Set_Pos(dwNewCityPos);

    TbMap *ptbMap = NULL;
    ptbMap = &pstUser->m_atbWild[pstUser->m_udwWildNum];
    pstUser->m_aucWildFlag[pstUser->m_udwWildNum] = EN_TABLE_UPDT_FLAG__CHANGE;
    pstUser->m_udwWildNum++;
    ptbMap->Set_Sid(dwNewSvrId);
    ptbMap->Set_Id(dwNewCityPos);
    CCommonBase::SetMapToNewCity(ptbMap, &pstUser->m_tbPlayer, pstCity, ptbAlliance);

    /*
    action:
        sid重设
    */
    for (TUINT32 idx = 0; idx < pstUser->m_udwActionNum; idx++)
    {
        TbAction *ptbAction = &pstUser->m_atbAction[idx];
        //是否存在被动移城action
        if (ptbAction->m_nMclass == EN_ACTION_SEC_CLASS__ATTACK_MOVE)
        {
            pstUser->m_aucActionFlag[idx] = EN_TABLE_UPDT_FLAG__DEL;
            continue;
        }
        if (ptbAction->m_nSuid == ptbPlayer->m_nUid)
        {
            if (ptbAction->m_nSclass == EN_ACTION_SEC_CLASS__ITEM)
            {
                ptbAction->Set_Ctime(ptbAction->m_nCtime + 6 * 3600);
                ptbAction->Set_Etime(ptbAction->m_nEtime + 6 * 3600);
                ptbAction->m_bParam[0].m_stItem.m_ddwTime += 6 * 3600;
                ptbAction->SetFlag(TbACTION_FIELD_PARAM);
            }
            ptbAction->Set_Sid(dwNewSvrId);
            pstUser->m_aucActionFlag[idx] = EN_TABLE_UPDT_FLAG__CHANGE;
        }
    }

    /*
    alliance_action:
        sid重设
    */
    for (TUINT32 idx = 0; idx < pstUser->m_udwSelfAlActionNum; idx++)
    {
        if (!CActionBase::IsPlayerOwnedAction(ptbPlayer->m_nUid, pstUser->m_atbSelfAlAction[idx].m_nId))
        {
            continue;
        }
        TbAlliance_action *ptbAlAction = &pstUser->m_atbSelfAlAction[idx];

        ptbAlAction->Set_Sid(dwNewSvrId);
        pstUser->m_aucSelfAlActionFlag[idx] = EN_TABLE_UPDT_FLAG__CHANGE;
    }

    /*
    march_action:
        删除王座相关action(单独脚本处理)
        scout删除, attack_move删除
        己方prison_timer删除, tax删除, notic删除
        出征全部召回
        (处理完后保证此表为空)
    */
    for (TUINT32 idx = 0; idx < pstUser->m_udwMarchNum; idx++)
    {
        if (!CActionBase::IsPlayerOwnedAction(pstUser->m_tbPlayer.m_nUid, pstUser->m_atbMarch[idx].m_nId))
        {
            continue;
        }
        TbMarch_action *ptbMarch = &pstUser->m_atbMarch[idx];
        if (ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__NOTI_TIMER)
        {
            pstUser->m_aucMarchFlag[idx] = EN_TABLE_UPDT_FLAG__DEL;
        }
        else if (ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__PRISON_TIMER)
        {
            pstUser->m_aucMarchFlag[idx] = EN_TABLE_UPDT_FLAG__DEL;
        }
        else if (ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__TAX)
        {
            pstUser->m_aucMarchFlag[idx] = EN_TABLE_UPDT_FLAG__DEL;
        }
        else if (ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__SCOUT)
        {
            pstUser->m_aucMarchFlag[idx] = EN_TABLE_UPDT_FLAG__DEL;
        }
        else
        {
            //troop
            for (TUINT32 udwIdx = 0; udwIdx < EN_TROOP_TYPE__END; ++udwIdx)
            {
                pstCity->m_stTblData.m_bTroop[0].m_addwNum[udwIdx] += ptbMarch->m_bParam[0].m_stTroop[udwIdx];
            }
            pstCity->m_stTblData.SetFlag(TbCITY_FIELD_TROOP);

            //resource 
            for (TUINT32 udwIdx = 0; udwIdx < EN_RESOURCE_TYPE__END; ++udwIdx)
            {
                pstCity->m_stTblData.m_bResource[0].m_addwNum[udwIdx] += ptbMarch->m_bParam[0].m_stResource[udwIdx];
                if (pstCity->m_stTblData.m_bResource[0].m_addwNum[udwIdx] > MAX_RESOURCE_LIMIT_NUM)
                {
                    pstCity->m_stTblData.m_bResource[0].m_addwNum[udwIdx] = MAX_RESOURCE_LIMIT_NUM;
                }
            }
            pstCity->m_stTblData.SetFlag(TbCITY_FIELD_RESOURCE);

            //reward
            CGlobalResLogic::AddGlobalRes(pstUser, pstCity, &ptbMarch->m_bReward[0]);
            CGlobalResLogic::AddGlobalRes(pstUser, pstCity, &ptbMarch->m_bSp_reward[0]);

            pstUser->m_aucMarchFlag[idx] = EN_TABLE_UPDT_FLAG__DEL;
        }
    }

    //TODO 删除称号

    /*
    bookmark:
        全删
        添加新svr的王座省会
    */
    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwBookmarkNum; ++udwIdx)
    {
        pstUser->m_aucBookMarkFlag[udwIdx] = EN_TABLE_UPDT_FLAG__DEL;
    }
    //CCommonLogic::AddBookMark(pstUser, THRONE_POS, 3, "Throne");

    pstSession->m_stReqParam.m_udwSvrId = dwNewSvrId;

    TCHAR szSvrChangeResponse[MAX_JSON_LEN];
    TCHAR szUrl[1024];
    memset(szSvrChangeResponse, 0, sizeof(szSvrChangeResponse));
    const char *pszUrlBeg = strchr(pstSession->m_stReqParam.m_szReqUrl, '?');
    string strRequest = pszUrlBeg;
    if (strRequest.find("pid=") == std::string::npos)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ACCOUNT_CHANGE_SVR_FAIL;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_SvrChange: there is no pid [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
        return -4;
    }
    std::size_t end = strRequest.find("command=");
    if (end != std::string::npos)
    {
        strRequest.resize(end + strlen("command="));
    }
    CMsgBase::StringReplace(strRequest, "command=", "command=change_sid&key0=");

    memset(szUrl, 0, sizeof(szUrl));
    sprintf(szUrl, "%s%s%d", CConfBase::GetString("account_url_pre", "serv_url").c_str(), strRequest.c_str(), dwNewSvrId);

    TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessCmd_SvrChange: Url:%s [seq=%u]", szUrl, pstSession->m_stUserInfo.m_udwBSeqNo));

    CURLcode res = CToolBase::ResFromUrl(szUrl, szSvrChangeResponse);
    if (CURLE_OK != res)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ACCOUNT_CHANGE_SVR_FAIL;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_SvrChange: change_sid failed[%d][seq=%u]", res, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -4;
    }

    for (TUINT32 udwIdx = 0; udwIdx < strlen(szSvrChangeResponse); ++udwIdx)
    {
        if (szSvrChangeResponse[udwIdx] == '\n')
        {
            szSvrChangeResponse[udwIdx] = ' ';
        }
    }
    TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessCmd_SvrChange: account_svr change_sid response: %s. [seq=%u]", szSvrChangeResponse, pstSession->m_udwSeqNo));

    Json::Reader reader;
    pstSession->m_JsonValue.clear();
    if (FALSE == reader.parse(szSvrChangeResponse, pstSession->m_JsonValue))
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ACCOUNT_CHANGE_SVR_FAIL;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_SvrChange: parse change_sid response failed. [seq=%u]", pstSession->m_udwSeqNo));
        return -5;
    }

    if (pstSession->m_JsonValue.isObject()
        && pstSession->m_JsonValue.isMember("res_header")
        && pstSession->m_JsonValue["res_header"].isObject()
        && pstSession->m_JsonValue["res_header"].isMember("ret_code")
        && pstSession->m_JsonValue["res_header"]["ret_code"].asInt() == 0)
    {
        pstSession->m_JsonValue.clear();
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessCmd_SvrChange: change_sid succ [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
    }
    else
    {
        pstSession->m_JsonValue.clear();
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ACCOUNT_CHANGE_SVR_FAIL;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_SvrChange: parse change_sid failed. [seq=%u]", pstSession->m_udwSeqNo));
        return -6;
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessOperate::Processcmd_AddRewardListForOp( SSession *pstSession, TBOOL &bNeedResponse )
{
    std::vector<string> vecOneReard;

    CCommonFunc::GetVectorFromString(pstSession->m_stReqParam.m_szKey[0], ',', vecOneReard);

    TSE_LOG_DEBUG(pstSession->m_poServLog, ("Processcmd_AddRewardListForOp:key=%s reward size=%u [seq=%u]", pstSession->m_stReqParam.m_szKey[0], vecOneReard.size(), pstSession->m_stUserInfo.m_udwBSeqNo));

    for(TUINT32 udwRewardIdx = 0; udwRewardIdx < vecOneReard.size(); ++udwRewardIdx)
    {
        TINT32 adwReward[3] = {0, 0, 0};
        TUINT32 udwNum = 3;
        CCommonFunc::GetArrayFromString(vecOneReard[udwRewardIdx].c_str(), ':', adwReward, udwNum);

        TSE_LOG_DEBUG(pstSession->m_poServLog, ("Processcmd_AddRewardListForOp: reward size=%u idx=%u reward=%s [type=%u id=%u num=%u][seq=%u]",
            vecOneReard.size(),
            udwRewardIdx,
            vecOneReard[udwRewardIdx].c_str(),
            adwReward[0U],
            adwReward[1U],
            adwReward[2U],
            pstSession->m_stUserInfo.m_udwBSeqNo));

        if(adwReward[2] > 0)
        {
            CGlobalResLogic::AddGlobalRes(&pstSession->m_stUserInfo, &pstSession->m_stUserInfo.m_stCityInfo, adwReward[0], adwReward[1], adwReward[2]);
        }
        else
        {
            CGlobalResLogic::CostGlobalRes(&pstSession->m_stUserInfo, &pstSession->m_stUserInfo.m_stCityInfo, adwReward[0], adwReward[1], -1*adwReward[2]);
        }
    }

    return 0;
}

TINT32 CProcessOperate::ProcessCmd_SetDragonFlag(SSession *pstSession, TBOOL &bNeedResponse)
{
    /*
    TINT32 dwSid = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TINT32 dwRetCode = 0;

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        TbMap tbMap;
        tbMap.Reset();
        tbMap.Set_Sid(dwSid);
        tbMap.Set_Id(THRONE_POS);

        CAwsRequest::GetItem(pstSession, &tbMap, ETbMAP_OPEN_TYPE_PRIMARY);

        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_SetDragonFlag: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
        return 0;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        TbMap tbMap;
        dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[0], &tbMap);
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessCmd_SetDragonFlag: map data sid=%ld id=%ld alid=%ld uid=%ld [seq=%u]",
            tbMap.m_nSid, tbMap.m_nId, tbMap.m_nAlid, tbMap.m_nUid, pstSession->m_stUserInfo.m_udwBSeqNo));
        if (dwRetCode <= 0 || tbMap.m_nId != THRONE_POS || tbMap.m_nSid != dwSid || tbMap.m_nAlid == 0 || tbMap.m_nUid == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_SetDragonFlag: map data error sid=%ld id=%ld alid=%ld uid=%ld [seq=%u]", 
                tbMap.m_nSid, tbMap.m_nId, tbMap.m_nAlid, tbMap.m_nUid, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }

        TbAlliance tbAlliance;
        tbAlliance.Reset();
        tbAlliance.Set_Aid(tbMap.m_nAlid);
        CAwsRequest::GetItem(pstSession, &tbAlliance, ETbALLIANCE_OPEN_TYPE_PRIMARY);

        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_SetDragonFlag: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
        return 0;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        TbAlliance tbAlliance;
        dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[0], &tbAlliance);
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessCmd_SetDragonFlag: alliance date sid=%ld aid=%ld throne_pos=%ld throne_status=%ld [seq=%u]",
            tbAlliance.m_nSid, tbAlliance.m_nAid, tbAlliance.m_nThrone_pos, tbAlliance.m_nThrone_status, pstSession->m_stUserInfo.m_udwBSeqNo));
        if (dwRetCode <= 0 || tbAlliance.m_nThrone_pos != THRONE_POS || tbAlliance.m_nThrone_status != EN_ALLIANCE_THRONE_STATUS_OCCUPY)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_SetDragonFlag: alliance date error sid=%ld aid=%ld throne_pos=%ld throne_status=%ld [seq=%u]",
                tbAlliance.m_nSid, tbAlliance.m_nAid, tbAlliance.m_nThrone_pos, tbAlliance.m_nThrone_status, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }

        TbSvr_stat tbSvrStat;
        tbSvrStat.Reset();
        tbSvrStat.Set_Sid(dwSid);
        tbSvrStat.Set_Dragon_flag(1);

        ExpectedDesc expect_desc;
        ExpectedItem expect_item;
        expect_item.SetVal(TbSVR_STAT_FIELD_SID, TRUE, dwSid);
        expect_desc.push_back(expect_item);

        CAwsRequest::UpdateItem(pstSession, &tbSvrStat, expect_desc, RETURN_VALUES_ALL_NEW);

        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_SetDragonFlag: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
        return 0;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        TbSvr_stat tbSvrStat;
        dwRetCode = CAwsResponse::OnUpdateItemRsp(*pstSession->m_vecAwsRsp[0], &tbSvrStat);
        if (dwRetCode <= 0 || tbSvrStat.m_nSid != dwSid || tbSvrStat.m_nDragon_flag != 1)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_SetDragonFlag: update svr stat ret=%d sid=%ld dragon_flag=%ld [seq=%u]",
                dwRetCode, tbSvrStat.m_nSid, tbSvrStat.m_nDragon_flag, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }

        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessCmd_SetDragonFlag: succ [seq=%u]",
            pstSession->m_stUserInfo.m_udwBSeqNo));
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;

    */
    return 0;
}

TINT32 CProcessOperate::ProcessCmd_RecoveryDeadPlayer(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbPlayer* ptbPlayer = &pstUser->m_tbPlayer;
    TbLogin* ptbLogin = &pstUser->m_tbLogin;
    TbMap* ptbTmpMap = &pstSession->m_tbTmpMap;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;

    TINT32 dwNemoFlag = atoi(pstSession->m_stReqParam.m_szKey[0]);

    // 1. 从map中获取未被占领的plain作为新城市的id——发送请求
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        if (dwNemoFlag == 77398022 && ptbPlayer->m_nDead_flag == 0)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_RecoveryDeadPlayer: not dead_player [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
            return 0;
        }

        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__MAP_SVR;

        pstSession->ResetMapSvrInfo();
        // set package
        MapSvrReqInfo* pMapSvrReq = new MapSvrReqInfo(pstSession->m_stReqParam.m_udwSvrId, "GetNewCityMap");
        Json::FastWriter writer;
        writer.omitEndingLineFeed();

        CJsoncppSeri jSeri;

        Json::Value jTmp = Json::Value(Json::objectValue);
        jTmp["type"] = 2; // 0: new user 1: random move 2: attack move
        jTmp["zone"] = -1;
        jTmp["province"] = -1;

        if (pstSession->m_udwContentType == EN_CONTENT_TYPE__STRING)
        {
            pMapSvrReq->sReqContent = writer.write(jTmp);
        }
        else
        {
            const TCHAR *pszRes = NULL;
            TUINT32 udwResLen = 0;
            pszRes = jSeri.serializeToBuffer(jTmp, udwResLen);
            pMapSvrReq->sReqContent.resize(udwResLen);
            memcpy((char*)pMapSvrReq->sReqContent.c_str(), pszRes, udwResLen);
        }
        pstSession->m_vecMapSvrReq.push_back(pMapSvrReq);

        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendMapSvrRequest(pstSession, EN_SERVICE_TYPE_MAP_SVR_PROXY_REQ);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_RecoveryDeadPlayer: send get map item request failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }

        return 0;
    }

    // 2. 解析获取的新city，并进行信息设置
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        // c.set procedure
        TINT32 dwParseCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[0], ptbTmpMap);
        if (dwParseCode <= 0 || ptbTmpMap->m_nId == 0 || ptbTmpMap->m_nUid != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_RecoveryDeadPlayer: map update failed, get again.[ret_code=%d res_info_ret_code=%u tbTmpMap_uid=%u login_uid=%u] [seq=%u]",
                dwRetCode,
                pstSession->m_stCommonResInfo.m_dwRetCode,
                ptbTmpMap->m_nUid,
                ptbLogin->m_nUid,
                pstSession->m_stUserInfo.m_udwBSeqNo));
            return -4;
        }

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;
    }

    // 3. 优先修改map数据，方便后续兼容插入失败情况——发送请求
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__4;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        if (pstUser->m_tbPlayer.m_nStatus & EN_CITY_STATUS__ON_SPECIAL_WILD)
        {
            pstUser->m_tbPlayer.Set_Status(pstUser->m_tbPlayer.m_nStatus & (~EN_CITY_STATUS__ON_SPECIAL_WILD));
        }

        //新手保护状态恢复@wave：20160923
        TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
        TUINT32 udwNewProtectTime = CGameInfo::GetInstance()->m_oJsonRoot["game_basic"][EN_GAME_BASIC_NEW_PROTECT_TIME].asUInt();
        if(udwCurTime - pstUser->m_tbLogin.m_nCtime < udwNewProtectTime) // 还在保护期内
        {
            TUINT32 udwBreakProtectLv = CGameInfo::GetInstance()->m_oJsonRoot["game_basic"][29U].asUInt();
            TUINT32 udwCastleLevel = CCityBase::GetBuildingLevelByFuncType(pstCity, EN_BUILDING_TYPE__CASTLE);
            if(udwCastleLevel < udwBreakProtectLv)
            {
                udwNewProtectTime = udwNewProtectTime - (udwCurTime - pstUser->m_tbLogin.m_nCtime);
                UActionParam stParam;
                stParam.m_stItem.SetValue(EN_BUFFER_INFO_PEACE_TIME, 0, udwNewProtectTime);
                CCommonBase::AddAction(pstUser, pstCity, EN_ACTION_MAIN_CLASS__ITEM, EN_ACTION_SEC_CLASS__ITEM,
                    EN_ITEM_STATUS__USING, udwNewProtectTime, &stParam);
                pstUser->m_tbPlayer.Set_Status(pstUser->m_tbPlayer.m_nStatus | EN_CITY_STATUS__NEW_PROTECTION);
            }            
        }

        CCommonBase::SetMapToNewCity(&pstSession->m_tbTmpMap, &pstUser->m_tbPlayer, pstCity, NULL);

        // get data
        pstSession->ResetAwsInfo();
        CAwsRequest::UpdateItem(pstSession, ptbTmpMap, ExpectedDesc(), 0, true);

        //wave@push_data
        CPushDataBasic::PushDataMap_SingleWild(pstSession, &pstSession->m_tbTmpMap);


        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_RecoveryDeadPlayer:[map_pos=%ld][seq=%u]",
            ptbTmpMap->m_nId,
            pstSession->m_udwSeqNo));

        // send request 
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_RecoveryDeadPlayer: map update request failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }

        return 0;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__4)
    {
        ptbPlayer->Set_Dead_flag(0);
        ptbPlayer->Set_Cid(ptbTmpMap->m_nId);
        pstCity->m_stTblData.Set_Pos(ptbTmpMap->m_nId);
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessOperate::ProcessCmd_ProSysGetData(SSession *pstSession, TBOOL &bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        pstSession->m_stCommonResInfo.m_bIsProSysData = TRUE;
        pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_UNKNOWN;
        pstSession->m_JsonValue.clear();
        pstSession->m_JsonValue = Json::Value(Json::objectValue);
        /*
        TCHAR szPromoteData[MAX_JSON_LEN];
        TCHAR szUrl[1024];
        memset(szPromoteData, 0, sizeof(szPromoteData));
        memset(szUrl, 0, sizeof(szUrl));
        const char *pszUrlBeg = strchr(pstSession->m_stReqParam.m_szReqUrl, '?');

        sprintf(szUrl, "%s%s&is_max_attack=%d", CConfBase::GetString("iap_url_pre", "serv_url").c_str(), pszUrlBeg,
            CItemLogic::HasTrialItemOrUnlock(pstUser));

        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessCmd_ProSysGetData: Url:%s [seq=%u]", szUrl, pstSession->m_stUserInfo.m_udwBSeqNo));

        if (CURLE_OK != CToolBase::ResFromUrl(szUrl, szPromoteData))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PRO_SYS_GET_DATA_FAIL;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ProSysGetData: get data failed. [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }

        for (TUINT32 udwIdx = 0; udwIdx < strlen(szPromoteData); ++udwIdx)
        {
            if (szPromoteData[udwIdx] == '\n')
            {
                szPromoteData[udwIdx] = ' ';
            }
        }

        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_ProSysGetData: date: %s [seq=%u]", szPromoteData, pstSession->m_stUserInfo.m_udwBSeqNo));

        Json::Reader reader;
        Json::Value jTmp;
        if (FALSE == reader.parse(szPromoteData, jTmp))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PRO_SYS_GET_DATA_FAIL;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("parse pro sys data failed. [seq=%u]", pstSession->m_udwSeqNo));
            return -3;
        }
        */

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__IAP_SVR;
        IapSvrReqInfo* pstReq = new IapSvrReqInfo;
        pstReq->m_udwType = EN_HU_REQUEST_TYPE__GET_IAP_PROMOTE;
        Json::Value rIapReqJson = Json::Value(Json::objectValue);
        rIapReqJson["sid"] = pstUser->m_tbLogin.m_nSid;
        rIapReqJson["uid"] = pstUser->m_tbPlayer.m_nUid;
        rIapReqJson["aid"] = pstUser->m_tbPlayer.m_nAlpos == 0 ? 0 : pstUser->m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET;
        Json::FastWriter rEventWriter;
        rEventWriter.omitEndingLineFeed();
        pstReq->m_sReqContent = rEventWriter.write(rIapReqJson);
        pstSession->m_vecIapSvrReq.push_back(pstReq);

        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_ProSysGetData: iap svr req: [json=%s] [type=%u] [uid=%ld][seq=%u]",
            pstReq->m_sReqContent.c_str(), \
            pstReq->m_udwType, \
            pstUser->m_tbPlayer.m_nUid, \
            pstSession->m_udwSeqNo));

        bNeedResponse = TRUE;
        TINT32 dwRetCode = CBaseProcedure::SendIapSvrRequest(pstSession);
        if (dwRetCode == 0)
        {
            return 0;
        }
        else
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_IAP_SERVER_REQ_ERR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ProSysGetData: send request to iap svr failed. [json=%s] [ret=%d] [seq=%u]",
                pstReq->m_sReqContent.c_str(), dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }
    }
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        vector<IapSvrRspInfo*>& vecRsp = pstSession->m_vecIapSvrRsp;
        IapSvrRspInfo *pstIapSvrRsp = NULL;
        if (vecRsp.size() > 0)
        {
            Json::Reader jsonReader;
            Json::Value oRspDataJson;
            pstIapSvrRsp = vecRsp[0];
            if (pstIapSvrRsp->m_udwType == EN_HU_REQUEST_TYPE__GET_IAP_PROMOTE)
            {
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_ProSysGetData: iap svr get rsp: [json=%s] [uid=%ld][seq=%u]",
                    pstIapSvrRsp->m_sRspJson.c_str(), pstUser->m_tbPlayer.m_nUid, pstSession->m_udwSeqNo));

                if (FALSE == jsonReader.parse(pstIapSvrRsp->m_sRspJson, pstSession->m_JsonValue))
                {
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_IAP_SERVER_PACKAGE_ERR;
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ProSysGetData: prase rsp from iap svr failed. [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                    return -6;
                }
            }
            else
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GET_IAP_FAIL;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ProSysGetData: m_udwType error.[seq=%u]", pstSession->m_udwSeqNo));
                return -5;
            }

            TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_ProSysGetData: IapDesc: %s [seq=%u]", pstIapSvrRsp->m_sRspJson.c_str(), pstSession->m_stUserInfo.m_udwBSeqNo));
        }
        else
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GET_IAP_FAIL;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ProSysGetData: vecRsp.size() error.[seq=%u]", pstSession->m_udwSeqNo));
            return -5;
        }

        Json::Value jIapPromote = Json::Value(Json::objectValue);
        jIapPromote["op_iap_promote"] = Json::Value(Json::objectValue);
        jIapPromote["op_iap_promote"]["promote"] = pstSession->m_JsonValue["promote"];
        pstSession->m_JsonValue = jIapPromote;

        //pstSession->m_stCommonResInfo.m_dwRetCode = pstSession->m_JsonValue["res_header"]["ret_code"].asInt();
    }
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessOperate::ProcessCmd_GenAttackMoveAction(SSession *pstSession, TBOOL &bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;

    UActionParam stParam;
    stParam.m_stAttackMove.SetValue(pstCity->m_stTblData.m_nPos, 2);
    CCommonBase::AddAction(pstUser, pstCity, EN_ACTION_MAIN_TASK_ATTACK_MOVE,
        EN_ACTION_SEC_CLASS__ATTACK_MOVE,
        EN_MARCH_STATUS__MOVE_CITY_PREPARE, 1, &stParam);

    return 0;
}

TINT32 CProcessOperate::ProcessCmd_GenIdol(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    TUINT32 udwType = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10); //0: add 1:del
    TUINT32 udwIdolId = strtoul(pstSession->m_stReqParam.m_szKey[1], NULL, 10);
    TUINT32 udwPos = strtoul(pstSession->m_stReqParam.m_szKey[2], NULL, 10);
    TUINT32 udwBeginTime = strtoul(pstSession->m_stReqParam.m_szKey[3], NULL, 10);
    TUINT32 udwXYSize = CMapBase::GetWildBlockNumByType(pstSession->m_stReqParam.m_udwSvrId, EN_WILD_TYPE__IDOL);

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        if (!CGameInfo::GetInstance()->m_oJsonRoot["game_guardian"].isMember(CCommonFunc::NumToString(udwIdolId)))
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_GenIdol: there is no such idol[id=%u] [seq=%u]",
                udwIdolId, pstSession->m_udwSeqNo));
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -1;
        }

        //拉取地图和神像
        // set request package
        pstSession->ResetAwsInfo();

        TUINT32 udwXPos = udwPos / MAP_X_Y_POS_COMPUTE_OFFSET;
        TUINT32 udwYPos = udwPos % MAP_X_Y_POS_COMPUTE_OFFSET;

        std::set<TINT32> posSet;
        posSet.clear();

        TUINT32 udwCenterPos = CCommonLogic::BuildingPointToPos(udwXPos, udwYPos);
        CCommonLogic::GetWildPos(udwCenterPos, udwXYSize, posSet);

        for (set<TINT32>::iterator it = posSet.begin(); it != posSet.end(); ++it)
        {
            BuildingPoint stBuildingPoing = CCommonLogic::BuildingPosToPoint(*it);
            if ((stBuildingPoing.x + stBuildingPoing.y) % 2 != 0)
            {
                continue;
            }
            TUINT32 udwNewPos = stBuildingPoing.x * MAP_X_Y_POS_COMPUTE_OFFSET + stBuildingPoing.y;
            CAwsRequest::MapGet(pstSession, udwNewPos);
        }

        CAwsRequest::IdolQuery(pstSession, pstSession->m_stReqParam.m_udwSvrId);

        // send request
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("TableRequest_MapGetByCids: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
        return 0;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;

        pstSession->m_udwIdolNum = 0;

        AwsRspInfo *pstRes = NULL;
        for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecAwsRsp.size(); ++udwIdx)
        {
            pstRes = pstSession->m_vecAwsRsp[udwIdx];

            string strTableRawName = CCommonFunc::GetTableRawName(pstRes->sTableName);
            if (strTableRawName == EN_AWS_TABLE_MAP)
            {
                dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[udwIdx],
                    &pstSession->m_atbTmpWild[pstSession->m_udwTmpWildNum]);

                if (dwRetCode > 0)
                {
                    pstSession->m_udwTmpWildNum++;
                }
            }
            else if (strTableRawName == EN_AWS_TABLE_IDOL)
            {
                dwRetCode = CAwsResponse::OnQueryRsp(*pstSession->m_vecAwsRsp[udwIdx],
                    pstSession->m_atbIdol, sizeof(TbIdol), MAX_IDOL_NUM);
                if (dwRetCode > 0)
                {
                    pstSession->m_udwIdolNum = dwRetCode;
                }
            }
        }
    }

    // 4. 处理
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        // set package
        pstSession->ResetAwsInfo();

        for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_udwTmpWildNum; udwIdx++)
        {
            pstSession->m_atbTmpWild[udwIdx].Set_Type(EN_WILD_TYPE__NORMAL);
            CMapBase::ResetMap(&pstSession->m_atbTmpWild[udwIdx]);
            if (udwType == 0)
            {
                //set data
                pstSession->m_atbTmpWild[udwIdx].Set_Type(EN_WILD_TYPE__IDOL);
                pstSession->m_atbTmpWild[udwIdx].Set_Utime(CTimeUtils::GetUnixTime());
                pstSession->m_atbTmpWild[udwIdx].Set_Bid(CMapBase::GetBlockIdFromPos(udwPos));
                pstSession->m_atbTmpWild[udwIdx].Set_Expire_time(INT64_MAX);
                if (pstSession->m_atbTmpWild[udwIdx].m_nId == udwPos)
                {
                    pstSession->m_atbTmpWild[udwIdx].Set_Pic_index(1);
                    pstSession->m_atbTmpWild[udwIdx].Set_Status(EN_IDOL_STATUS__THRONE_PEACE_TIME);
                    pstSession->m_atbTmpWild[udwIdx].Set_Time_end(0);
                }
                else
                {
                    pstSession->m_atbTmpWild[udwIdx].Set_Pic_index(2);
                }
            }
            // set package
            CAwsRequest::UpdateItem(pstSession, &pstSession->m_atbTmpWild[udwIdx]);
        }

        TbIdol *ptbIdol = NULL;
        for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_udwIdolNum; udwIdx++)
        {
            if (pstSession->m_atbIdol[udwIdx].m_nId == udwIdolId)
            {
                ptbIdol = &pstSession->m_atbIdol[udwIdx];
            }
        }

        if (ptbIdol == NULL && udwType == 0)
        {
            //add an Idol
            TbIdol tbIdol;
            tbIdol.Reset();
            tbIdol.Set_Sid(pstSession->m_stReqParam.m_udwSvrId);
            tbIdol.Set_Id(udwIdolId);
            tbIdol.Set_Pos(udwPos);
            tbIdol.Set_Status(EN_IDOL_STATUS__THRONE_PEACE_TIME);
            CAwsRequest::UpdateItem(pstSession, &tbIdol);
        }
        else if (ptbIdol && udwType == 1)
        {
            //del an Idol
            CAwsRequest::DeleteItem(pstSession, ptbIdol);
        }

        if (udwType == 0)
        {
            TbMarch_action tbAction;
            tbAction.Set_Suid(0);
            tbAction.Set_Id(CActionBase::GenMapActionId(pstSession->m_stReqParam.m_udwSvrId, udwPos));
            tbAction.Set_Sid(pstSession->m_stReqParam.m_udwSvrId);
            tbAction.Set_Tpos(udwPos);
            tbAction.Set_Mclass(EN_ACTION_MAIN_CLASS__TIMER);
            tbAction.Set_Sclass(EN_ACTION_SEC_CLASS__IDOL_PERIOD);
            tbAction.Set_Status(EN_TITMER_IDOL_PERIOD_STATUS__THRONE_PEACE_TIME);
            tbAction.Set_Etime(udwBeginTime);
            CAwsRequest::UpdateItem(pstSession, &tbAction);
        }

        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("TableRequest_CityChangeId: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -5;
        }
        return 0;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    return 0;
}

TINT32 CProcessOperate::ProcessCmd_GenThrone(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    TUINT32 udwType = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10); //0: add 1:del
    TUINT32 udwPos = strtoul(pstSession->m_stReqParam.m_szKey[2], NULL, 10);

    TUINT32 udwXYSize = CMapBase::GetWildBlockNumByType(pstSession->m_stReqParam.m_udwSvrId, EN_WILD_TYPE__THRONE_NEW);

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        //拉取地图和王座
        // set request package
        pstSession->ResetAwsInfo();

        TUINT32 udwXPos = udwPos / MAP_X_Y_POS_COMPUTE_OFFSET;
        TUINT32 udwYPos = udwPos % MAP_X_Y_POS_COMPUTE_OFFSET;

        std::set<TINT32> posSet;
        posSet.clear();

        TUINT32 udwCenterPos = CCommonLogic::BuildingPointToPos(udwXPos, udwYPos);
        CCommonLogic::GetWildPos(udwCenterPos, udwXYSize, posSet);

        for (set<TINT32>::iterator it = posSet.begin(); it != posSet.end(); ++it)
        {
            BuildingPoint stBuildingPoing = CCommonLogic::BuildingPosToPoint(*it);
            if ((stBuildingPoing.x + stBuildingPoing.y) % 2 != 0)
            {
                continue;
            }
            TUINT32 udwNewPos = stBuildingPoing.x * MAP_X_Y_POS_COMPUTE_OFFSET + stBuildingPoing.y;
            CAwsRequest::MapGet(pstSession, udwNewPos);
        }

        CAwsRequest::ThroneGet(pstSession, pstSession->m_stReqParam.m_udwSvrId);

        // send request
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("TableRequest_MapGetByCids: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
        return 0;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;

        pstSession->m_udwIdolNum = 0;

        AwsRspInfo *pstRes = NULL;
        for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecAwsRsp.size(); ++udwIdx)
        {
            pstRes = pstSession->m_vecAwsRsp[udwIdx];

            string strTableRawName = CCommonFunc::GetTableRawName(pstRes->sTableName);
            if (strTableRawName == EN_AWS_TABLE_MAP)
            {
                dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[udwIdx],
                    &pstSession->m_atbTmpWild[pstSession->m_udwTmpWildNum]);

                if (dwRetCode > 0)
                {
                    pstSession->m_udwTmpWildNum++;
                }
            }
            else if (strTableRawName == EN_AWS_TABLE_THRONE)
            {
                CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[udwIdx], &pstSession->m_tbThrone);
            }
        }
    }

    // 4. 处理
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        // set package
        pstSession->ResetAwsInfo();

        for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_udwTmpWildNum; udwIdx++)
        {
            pstSession->m_atbTmpWild[udwIdx].Set_Type(EN_WILD_TYPE__NORMAL);
            CMapBase::ResetMap(&pstSession->m_atbTmpWild[udwIdx]);
            if (udwType == 0)
            {
                //set data
                pstSession->m_atbTmpWild[udwIdx].Set_Type(EN_WILD_TYPE__THRONE_NEW);
                pstSession->m_atbTmpWild[udwIdx].Set_Utime(CTimeUtils::GetUnixTime());
                pstSession->m_atbTmpWild[udwIdx].Set_Bid(CMapBase::GetBlockIdFromPos(udwPos));
                pstSession->m_atbTmpWild[udwIdx].Set_Expire_time(INT64_MAX);
                if (pstSession->m_atbTmpWild[udwIdx].m_nId == udwPos)
                {
                    pstSession->m_atbTmpWild[udwIdx].Set_Pic_index(1);
                    pstSession->m_atbTmpWild[udwIdx].Set_Status(EN_THRONE_STATUS__QUIET_PERIOD);
                    pstSession->m_atbTmpWild[udwIdx].Set_Time_end(0);
                }
                else
                {
                    pstSession->m_atbTmpWild[udwIdx].Set_Pic_index(2);
                }
            }
            // set package
            CAwsRequest::UpdateItem(pstSession, &pstSession->m_atbTmpWild[udwIdx]);
        }

        if (pstSession->m_tbThrone.m_nPos != udwPos && udwType == 0)
        {
            //add an Throne
            pstSession->m_tbThrone.Reset();
            pstSession->m_tbThrone.Set_Sid(pstSession->m_stReqParam.m_udwSvrId);
            pstSession->m_tbThrone.Set_Id(0);
            pstSession->m_tbThrone.Set_Pos(udwPos);
            pstSession->m_tbThrone.Set_Status(EN_THRONE_STATUS__QUIET_PERIOD);
            pstSession->m_tbThrone.Set_End_time(0);
            pstSession->m_tbThrone.m_jInfo = Json::Value(Json::objectValue);
            pstSession->m_tbThrone.m_jInfo["buff"] = CGameInfo::GetInstance()->m_oJsonRoot["game_throne_buff"];
            pstSession->m_tbThrone.SetFlag(TbTHRONE_FIELD_INFO);
            pstSession->m_tbThrone.Set_Occupy_time(CTimeUtils::GetUnixTime());

            CAwsRequest::UpdateItem(pstSession, &pstSession->m_tbThrone);
        }
        else if (pstSession->m_tbThrone.m_nPos == udwPos && udwType == 1)
        {
            //del an Idol
            CAwsRequest::DeleteItem(pstSession, &pstSession->m_tbThrone);
        }

        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("TableRequest_CityChangeId: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -5;
        }
        return 0;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    return 0;
}

TINT32 CProcessOperate::ProcessCmd_UpgradeNpcToLv10( SSession *pstSession, TBOOL &bNeedResponse )
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();

    //用户校验——npc、无联盟、用户等级
    if(pstUser->m_tbPlayer.m_nNpc == 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }
    if(pstUser->m_tbPlayer.m_nAlid != 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -2;
    }
    if(pstUser->m_tbPlayer.m_nLevel >= 10)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -3;
    }

    //设置guide标记
    BITSET(pstUser->m_tbLogin.m_bGuide_flag[0].m_bitFlag, 1);
    BITSET(pstUser->m_tbLogin.m_bGuide_flag[0].m_bitFlag, 2);
    BITSET(pstUser->m_tbLogin.m_bGuide_flag[0].m_bitFlag, 3);
    pstUser->m_tbLogin.SetFlag(TbLOGIN_FIELD_GUIDE_FLAG);

    //用户等级+exp+age
    pstUser->m_tbPlayer.Set_Exp(638509);
    pstUser->m_tbPlayer.Set_Level(10);
    pstUser->m_tbPlayer.Set_Age(3);

    //打破新手保护————由au负责打破
    for(int idx = 0; idx < pstUser->m_udwActionNum; idx++)
    {
        if(pstUser->m_atbAction[idx].m_nMclass == EN_ACTION_MAIN_CLASS__ITEM
            && pstUser->m_atbAction[idx].m_nSclass == EN_ACTION_SEC_CLASS__ITEM)
        {
            pstUser->m_atbAction[idx].Set_Etime(udwCurTime);
            pstUser->m_aucActionFlag[idx] = EN_TABLE_UPDT_FLAG__CHANGE;
        }
    }

    //用户building设置
    string str_building = "{\"16\":[25,1],\"1016\":[26,1],\"10006\":[7,1],\"10025\":[57,1],\"10035\":[57,1],\"11003\":[3,5],\"11035\":[57,1],\"21012\":[43,3],\"21020\":[40,1],\"30003\":[14,4],\"30011\":[38,4],\"30020\":[56,1],\"30030\":[58,1],\"31030\":[58,1],\"40006\":[17,1],\"41008\":[10,4],\"50000\":[41,4],\"60015\":[57,1],\"60025\":[58,1],\"61025\":[58,1],\"70005\":[19,1],\"71005\":[27,1],\"80010\":[58,1],\"80020\":[56,1],\"81020\":[58,1],\"91001\":[23,1],\"110005\":[57,1],\"110015\":[56,1],\"111015\":[56,1],\"130000\":[58,1],\"130010\":[58,1],\"131010\":[57,1],\"160005\":[58,1],\"161005\":[57,1],\"180000\":[56,1],\"10011022\":[29,4],\"10020006\":[19,5],\"10020012\":[39,1],\"10020030\":[58,1],\"10021012\":[23,4],\"10030001\":[26,3],\"10030009\":[20,2],\"10031018\":[35,1],\"10040025\":[56,1],\"10041008\":[27,2],\"10041021\":[20,1],\"10050005\":[25,3],\"10051014\":[31,4],\"10061019\":[14,1],\"10070000\":[44,5],\"10070020\":[58,1],\"10071009\":[33,4],\"10090006\":[63,1],\"10090015\":[56,1],\"10101006\":[17,2],\"10120010\":[56,1],\"10121010\":[56,1],\"10140005\":[56,1],\"10141005\":[57,1],\"10170000\":[56,1]}";
    Json::Reader reader;
    Json::Value jsonBuilding;
    if(reader.parse(str_building, jsonBuilding) == false)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -4;
    }
    Json::Value::Members jsonDataKeys = jsonBuilding.getMemberNames();
    TUINT32 udwPos = 0;
    pstCity->m_stTblData.m_bBuilding.Reset();
    for(Json::Value::Members::iterator it = jsonDataKeys.begin(); it != jsonDataKeys.end(); ++it)
    {
        udwPos = strtoul((*it).c_str(), NULL, 10);
        CCityBase::AddBuilding(udwPos, jsonBuilding[*it][0].asInt(), jsonBuilding[*it][1].asInt(), pstCity->m_stTblData);
    }

    //用户科技设置
    pstCity->m_stTblData.m_bResearch[0].m_addwLevel[0] = 2;
    pstCity->m_stTblData.m_bResearch[0].m_addwLevel[1] = 1;
    pstCity->m_stTblData.m_bResearch[0].m_addwLevel[2] = 2;
    pstCity->m_stTblData.m_bResearch[0].m_addwLevel[3] = 1;
    pstCity->m_stTblData.SetFlag(TbCITY_FIELD_RESEARCH);

    return 0;
}

TINT32 CProcessOperate::ProcessCmd_SvrChange(SSession *pstSession, TBOOL &bNeedResponse)
{
    /*
    key0=target_svr
    key1=targe_pos
    */

    TINT32 dwRetCode = 0;
    TUINT32 udwTargetSvr = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TUINT32 udwPos = atoi(pstSession->m_stReqParam.m_szKey[1]);

    SUserInfo* pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TbAlliance *pstAlliance = &pstUser->m_tbAlliance;

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        //如果服务器处于合服保护状态，则阻止用户进行svr change
        SGameSvrInfo *pstSvrInfo = CGameSvrInfo::GetInstance()->GetSidInfo(udwTargetSvr);
        if (pstSvrInfo->m_dwMergeStatus != EN_SVR_MERGE_STATUS__NORMAL)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SVR_MERGE_PROTECTED;
            return -1;
        }
        pstSvrInfo = CGameSvrInfo::GetInstance()->GetSidInfo(pstSession->m_stReqParam.m_udwSvrId);
        if (pstSvrInfo->m_dwMergeStatus != EN_SVR_MERGE_STATUS__NORMAL)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SVR_MERGE_PROTECTED;
            return -1;
        }

        if (pstAlliance->m_nAid > 0 && pstUser->m_tbPlayer.m_nAlpos > 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__CANNOT_CHANGE_SVR_IN_ALLIANCE;
            return -1;
        }

        if (pstUser->m_tbPlayer.m_nDragon_status != EN_DRAGON_STATUS_NORMAL
            && pstUser->m_tbPlayer.m_nDragon_status != EN_DRAGON_STATUS_DEAD)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -2;
        }

        for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwPassiveMarchNum; ++udwIdx)
        {
            if (pstUser->m_atbPassiveMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__PRISON_TIMER)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__CAPTURE_HERO;
                return -3;
            }
        }

        TUINT32 udwMarchNum = 0;
        TBOOL bIsRallyWar = FALSE;
        for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
        {
            if (!CActionBase::IsPlayerOwnedAction(pstUser->m_tbPlayer.m_nUid, pstUser->m_atbMarch[udwIdx].m_nId))
            {
                continue;
            }
            if (pstUser->m_atbMarch[udwIdx].m_nMclass == EN_ACTION_MAIN_CLASS__TIMER)
            {
                continue;
            }
            if (pstUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR)
            {
                bIsRallyWar = TRUE;
                break;
            }
            udwMarchNum++;
        }
        if (udwMarchNum || bIsRallyWar)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__CANNOT_MOVE_CITY_BY_MARCH_ACTION;
            if (bIsRallyWar)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__CANNOT_MOVE_CITY_ON_RALLY_WAR;
            }
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MoveCity: march action nut null [num=%u] [rally_war=%d] [seq=%u]",
                udwMarchNum, bIsRallyWar, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -6;

        }
        TUINT32 udwCanNotMoveNum = 0;
        for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwPassiveMarchNum; ++udwIdx)
        {
            if (pstUser->m_atbPassiveMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_NORMAL &&
                (pstUser->m_atbPassiveMarch[udwIdx].m_nStatus == EN_MARCH_STATUS__DEFENDING || pstUser->m_atbPassiveMarch[udwIdx].m_nStatus == EN_MARCH_STATUS__SETUP_CAMP))
            {
                udwCanNotMoveNum++;
            }
            if (pstUser->m_atbPassiveMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__TRANSPORT &&
                pstUser->m_atbPassiveMarch[udwIdx].m_nStatus == EN_MARCH_STATUS__UN_LOADING)
            {
                udwCanNotMoveNum++;
            }
        }

        if (udwCanNotMoveNum)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SVR_CHANGE_ACTION_ERR;

            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MoveCity: passive action not null [num=%u] [seq=%u]", udwCanNotMoveNum,
                pstSession->m_stUserInfo.m_udwBSeqNo));
            return -7;
        }

        pstSession->ResetMapSvrReq();
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__MAP_SVR;

        pstSession->ResetMapSvrReq();

        MapSvrReqInfo* pMapSvrReq = new MapSvrReqInfo(udwTargetSvr, "GetNewCityMap");
        Json::FastWriter writer;
        writer.omitEndingLineFeed();
        CJsoncppSeri jSeri;
        Json::Value jTmp = Json::Value(Json::objectValue);
        jTmp["type"] = 3; // 0: new user 1: random move 2: attack move 3: move city
        jTmp["id"] = udwPos;

        if (pstSession->m_udwContentType == EN_CONTENT_TYPE__STRING)
        {
            pMapSvrReq->sReqContent = writer.write(jTmp);
        }
        else
        {
            const TCHAR *pszRes = NULL;
            TUINT32 udwResLen = 0;
            pszRes = jSeri.serializeToBuffer(jTmp, udwResLen);
            pMapSvrReq->sReqContent.resize(udwResLen);
            memcpy((char*)pMapSvrReq->sReqContent.c_str(), pszRes, udwResLen);
        }
        pstSession->m_vecMapSvrReq.push_back(pMapSvrReq);

        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendMapSvrRequest(pstSession, EN_SERVICE_TYPE_MAP_SVR_PROXY_REQ);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MoveCity: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }

        return 0;
    }

    if (EN_COMMAND_STEP__1 == pstSession->m_udwCommandStep)
    {
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[0],
            &pstSession->m_tbTmpMap);

        if (dwRetCode <= 0 || pstSession->m_tbTmpMap.m_nId == 0 || pstSession->m_tbTmpMap.m_nUid != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__MOVE_CITY_COORD_INVALID;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("GetMapInfo: wild belong someone else. [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -4;
        }
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        if (!pstCity)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__MoveCity: city id error [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -10;
        }

        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        if (pstSession->m_tbTmpMap.m_nType == EN_WILD_TYPE__SPECIAL_LAKE)
        {
            if (pstUser->m_tbPlayer.m_nStatus & EN_CITY_STATUS__NEW_PROTECTION)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NEW_PROTECT_CANNOT_MOVE_TO_SPECAIL;
                return -10;
            }

            CCommonBase::UpdatePlayerStatusOnSpecailWild(pstUser);
        }
        else
        {
            if (pstUser->m_tbPlayer.m_nStatus & EN_CITY_STATUS__ON_SPECIAL_WILD)
            {
                pstUser->m_tbPlayer.Set_Status(pstUser->m_tbPlayer.m_nStatus & (~EN_CITY_STATUS__ON_SPECIAL_WILD));
            }
        }

        // 设置新map数据
        CCommonBase::SetMapToNewCity(&pstSession->m_tbTmpMap, &pstUser->m_tbPlayer, pstCity, pstAlliance);
        for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwWildNum; ++udwIdx)
        {
            TbMap* ptbMap = &pstUser->m_atbWild[udwIdx];
            if (EN_WILD_TYPE__CITY != ptbMap->m_nType)
            {
                continue;
            }
            pstSession->m_tbTmpMap.Set_City_info(ptbMap->m_jCity_info);
            break;
        }

        //设置保护时间
        pstSession->m_tbTmpMap.Set_Time_end(pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_PEACE_TIME].m_astBuffDetail[EN_BUFF_TYPE_ITEM].m_dwTime);

        pstSession->ResetAwsReq();
        CAwsRequest::UpdateItem(pstSession, &pstSession->m_tbTmpMap, ExpectedDesc(), 0, true);

        //wave@push_data
        CPushDataBasic::PushDataMap_SingleWild(pstSession, &pstSession->m_tbTmpMap);

        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_SvrChange: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -5;
        }
        return 0;
    }

    // 5. update action
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__4;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        // set package
        pstSession->ResetAwsReq();

        TUINT32 idx = 0;
        // 更新action数据
        for (idx = 0; idx < pstUser->m_udwActionNum; idx++)
        {
            TbAction *ptbAction = &pstUser->m_atbAction[idx];
            //是否存在被动移城action
            if (ptbAction->m_nMclass == EN_ACTION_SEC_CLASS__ATTACK_MOVE)
            {
                pstUser->m_aucActionFlag[idx] = EN_TABLE_UPDT_FLAG__DEL;
                continue;
            }
            if (ptbAction->m_nSuid == pstSession->m_stUserInfo.m_tbPlayer.m_nUid)
            {
                ptbAction->Set_Sid(udwTargetSvr);
                //ptbAction->Set_Scid(pstSession->m_tbTmpMap.m_nId);
                CAwsRequest::UpdateItem(pstSession, ptbAction);
            }
        }

        for (idx = 0; idx < pstUser->m_udwSelfAlActionNum; idx++)
        {
            TbAlliance_action *ptbAlAction = &pstUser->m_atbSelfAlAction[idx];
            ptbAlAction->Set_Sid(udwTargetSvr);
            ptbAlAction->Set_Scid(pstSession->m_tbTmpMap.m_nId);
            CAwsRequest::UpdateItem(pstSession, ptbAlAction);
        }

        pstSession->m_dwOldRequestAid = pstUser->m_tbPlayer.m_nReq_al >> 32;
        if (pstSession->m_dwOldRequestAid > 0)
        {
            TbAl_member tbAlmember;
            tbAlmember.Set_Aid(-1 * pstSession->m_dwOldRequestAid);
            tbAlmember.Set_Uid(pstUser->m_tbPlayer.m_nUid);
            CAwsRequest::DeleteItem(pstSession, &tbAlmember);
        }

        // send request
        if (pstSession->m_vecAwsReq.size() > 0)
        {
            bNeedResponse = TRUE;
            dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
            if (dwRetCode < 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_SvrChange: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                return -6;
            }
            return 0;
        }
        else
        {
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_SvrChange: no action and wild need update [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
        }
    }

    // 6. 最后更新本地city、city所在map和player信息
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__4)
    {
        TINT64 ddwOldSid = pstUser->m_tbPlayer.m_nSid;

        pstUser->m_tbPlayer.Set_Invite_mail_time(CTimeUtils::GetUnixTime());
        pstUser->m_tbPlayer.Set_Svr_change_time(CTimeUtils::GetUnixTime());
        pstUser->m_tbPlayer.Set_Sid(udwTargetSvr);
        pstUser->m_tbLogin.Set_Sid(udwTargetSvr);

        pstUser->m_tbPlayer.Set_Req_al(0);

        TUINT32 udwRawCityId = pstSession->m_stReqParam.m_udwCityId;
        TUINT32 udwNewCityId = pstSession->m_tbTmpMap.m_nId;

        // 更新成功之后设置新city数据
        pstSession->m_stReqParam.m_udwCityId = udwNewCityId;

        // 更新city的id
        pstCity->m_stTblData.Set_Pos(udwNewCityId);

        // 更新player数据
        pstUser->m_tbPlayer.Set_Cid(udwNewCityId);

        // 更新原始map数据
        pstSession->m_ucTmpMapItemFlag = EN_TABLE_UPDT_FLAG__CHANGE;
        pstSession->m_tbTmpMap.Reset();
        pstSession->m_tbTmpMap.Set_Sid(pstSession->m_stReqParam.m_udwSvrId);
        pstSession->m_tbTmpMap.Set_Id(udwRawCityId);
        pstSession->m_tbTmpMap.Set_Type(EN_WILD_TYPE__CITY);

        CCommonBase::AbandonWild(pstCity, &pstSession->m_tbTmpMap);

        for (TUINT32 udwIdx = 0; udwIdx < MAX_WILD_RETURN_NUM; ++udwIdx)
        {
            pstSession->m_atbTmpWild[udwIdx].Reset();
            pstSession->m_aucTmpWildFlag[udwIdx] = 0;
        }

        //wave@push_data
        CPushDataBasic::PushDataMap_SingleWild(pstSession, &pstSession->m_tbTmpMap);

        //禁止 after更新
        pstUser->m_udwWildNum = 0;

        TCHAR szSvrChangeResponse[MAX_JSON_LEN];
        TCHAR szUrl[1024];
        memset(szSvrChangeResponse, 0, sizeof(szSvrChangeResponse));
        const char *pszUrlBeg = strchr(pstSession->m_stReqParam.m_szReqUrl, '?');
        string strRequest = pszUrlBeg;
        std::size_t end = strRequest.find("command=");
        if (end != std::string::npos)
        {
            strRequest.resize(end + strlen("command="));
        }
        CMsgBase::StringReplace(strRequest, "command=", "command=change_sid&key0=");

        memset(szUrl, 0, sizeof(szUrl));
        sprintf(szUrl, "%s%s%u", CConfBase::GetString("account_url_pre", "serv_url").c_str(), strRequest.c_str(), udwTargetSvr);

        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessCmd_SvrChange: Url:%s [seq=%u]", szUrl, pstSession->m_stUserInfo.m_udwBSeqNo));

        CURLcode res = CToolBase::ResFromUrl(szUrl, szSvrChangeResponse);
        if (CURLE_OK != res)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ACCOUNT_CHANGE_SVR_FAIL;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_SvrChange: change_sid failed[%d][seq=%u]", res, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -4;
        }

        for (TUINT32 udwIdx = 0; udwIdx < strlen(szSvrChangeResponse); ++udwIdx)
        {
            if (szSvrChangeResponse[udwIdx] == '\n')
            {
                szSvrChangeResponse[udwIdx] = ' ';
            }
        }
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessCmd_SvrChange: account_svr change_sid response: %s. [seq=%u]", szSvrChangeResponse, pstSession->m_udwSeqNo));

        Json::Reader reader;
        pstSession->m_JsonValue.clear();
        if (FALSE == reader.parse(szSvrChangeResponse, pstSession->m_JsonValue))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ACCOUNT_CHANGE_SVR_FAIL;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_SvrChange: parse change_sid response failed. [seq=%u]", pstSession->m_udwSeqNo));
            return -5;
        }

        if (pstSession->m_JsonValue.isObject()
            && pstSession->m_JsonValue.isMember("res_header")
            && pstSession->m_JsonValue["res_header"].isObject()
            && pstSession->m_JsonValue["res_header"].isMember("ret_code")
            && pstSession->m_JsonValue["res_header"]["ret_code"].asInt() == 0)
        {
            pstSession->m_JsonValue.clear();
            TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessCmd_SvrChange: change_sid succ [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
        }
        else
        {
            pstSession->m_JsonValue.clear();
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ACCOUNT_CHANGE_SVR_FAIL;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_SvrChange: parse change_sid failed. [seq=%u]", pstSession->m_udwSeqNo));
            return -6;
        }

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessOperate::ProcessCmd_GetRank(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    string szCmd = pstSession->m_stReqParam.m_szKey[0];
    TUINT32 udwUid = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TUINT32 udwSid = atoi(pstSession->m_stReqParam.m_szKey[2]);

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        pstSession->ResetRankSvrInfo();
        
        RankSvrReqInfo *ptbRankReq = new RankSvrReqInfo;

        pstSession->m_vecRankSvrReq.push_back(ptbRankReq);

        ptbRankReq->SetVal(udwSid, udwUid, szCmd);

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__RANK_SVR;
        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendRankSvrRequest(pstSession);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_GetRank: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }

        return 0;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecRankSvrRsp.size(); udwIdx++)
        {
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_GetRank: idx=%u ret=%d uid=%u content=%s [seq=%u]", 
                udwIdx, pstSession->m_vecRankSvrRsp[udwIdx]->m_dwRetCode, pstSession->m_vecRankSvrRsp[udwIdx]->m_udwUid, 
                pstSession->m_vecRankSvrRsp[udwIdx]->m_sRspJson.c_str(),
                pstSession->m_stUserInfo.m_udwBSeqNo));
        }
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}