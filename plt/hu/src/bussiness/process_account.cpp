#include "process_account.h"
#include "statistic.h"
#include "game_info.h"
#include "procedure_base.h"
#include "global_serv.h"
#include "sendmessage_base.h"
#include "common_logic.h"
#include "conf_base.h"
#include "globalres_logic.h"
#include "activities_logic.h"
#include "quest_logic.h"
#include "common_func.h"
#include "msg_base.h"
#include "iap_white_list.h"
#include "tool_base.h"
#include "common_base.h"
#include "item_logic.h"

TINT32 CProcessAccount::ProcessCmd_GemRecharge(SSession *pstSession, TBOOL &bNeedResponse)
{
    SUserInfo *pstUserInfo = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUserInfo->m_stCityInfo;
    TINT32 dwRetCode = 0;
    TUINT32 udwRechargeGem = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    //TUINT32 udwRechargeGemSeq = strtoul(pstSession->m_stReqParam.m_szKey[1], NULL, 10);
    TINT32 dwRechargeType = atoi(pstSession->m_stReqParam.m_szKey[2]); //(线上版本固定为0)(为1是为线下加gem方便)
    string sTransId = pstSession->m_stReqParam.m_szKey[3]; // 与apple交易的id(必传)
    TINT64 ddwInstantId = strtoll(pstSession->m_stReqParam.m_szKey[4], NULL, 10); //促销id,兼容pid
    TINT64 ddwProjectId = strtoll(pstSession->m_stReqParam.m_szKey[5], NULL, 10); //促销具体方案的id

    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbLogin& tbLogin = pstSession->m_stUserInfo.m_tbLogin;
    TbPlayer& tbPlayer = pstSession->m_stUserInfo.m_tbPlayer;

    TFLOAT32 fWhiteWeekMaxIap = 0.0;
    TBOOL bIsWhiteUser = CIAPWhiteList::bIsWhitePlayer(tbPlayer.m_nUid, fWhiteWeekMaxIap);
    TFLOAT32 fIapNum = CCommonLogic::GetIapPayCent(pstSession->m_stReqParam.m_szItemId) / 100.0;

    //wave@20140312: 添加transaction id保护
    if(sTransId.length() == 0)
    {
        sTransId = "0";
    }

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
            pstUserInfo->m_tbPlayer.m_nUid, bIsWhiteUser, fWeekGemRecharge, fIapNum, fWhiteWeekMaxIap, pstSession->m_udwSeqNo));


        if (bIsWhiteUser && fWeekGemRecharge + fIapNum >= fWhiteWeekMaxIap)
        {
            TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("ProcessCmd_GemRecharge:user is white user and his over week iap num just return [uid=%u max_week_iap=%f cur_week_iap=%f gem_num=%u gem_iap=%f] [seq=%u]",
                pstUserInfo->m_tbPlayer.m_nUid,
                fWhiteWeekMaxIap,
                fWeekGemRecharge,
                udwRechargeGem,
                fIapNum,
                pstUserInfo->m_udwBSeqNo));
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__IAP_WHITE_MAX;
            return -1;
        }

        /******************************************* 线下测试环境需要的加gem功能 *******************************************/
        if (1 == dwRechargeType && 100 != udwRechargeGem) //reward 100gem不给联盟gift
        {
            tbLogin.Set_Gem(tbLogin.m_nGem + udwRechargeGem);
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;

            //更新玩家的购买力标记
            TUINT32 udwAbility = CCommonLogic::GetPurchaseAbility(udwRechargeGem);
            if (udwAbility > tbLogin.m_nAbility)
            {
                tbLogin.Set_Ability(udwAbility);
            }
            return 0;
        }
        // wave@20140122: 设定有效充值标记的默认值
        pstSession->m_ucFakeRecharge = TRUE;

        //wave@20140312: 加密+购买情况下，要验证transaction id
        if (pstSession->m_dwClientReqEnType && 0 == dwRechargeType)
        {
            if (sTransId.length() <= 2)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("gem_recharge invalid transaction [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                return -2;
            }
        }

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
            jTmp["user_id"] = pstUserInfo->m_tbPlayer.m_nUid;
            jTmp["sid"] = pstUserInfo->m_tbPlayer.m_nSid;

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
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_GemRecharge: send purchase check req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
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
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_GemRecharge: parse purchase check rsp fail [seq=%u]",
                        pstSession->m_stUserInfo.m_udwBSeqNo));
                    return -3;
                }
            }
            else
            {
                if (jUnseri.unserializeToDom(pstSession->m_stPurchaseRsp.sRspContent.c_str(), jTmp, UNSERI_MODE_REPLACE) != 0)
                {
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PURCHASE_CHECK_FAIL;
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_GemRecharge: parse purchase check rsp fail [seq=%u]",
                        pstSession->m_stUserInfo.m_udwBSeqNo));
                    return -3;
                }
            }

            if (!jTmp.isMember("ret_code") || jTmp["ret_code"].asInt() != 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PURCHASE_CHECK_FAIL;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_GemRecharge: purchase check failed[errmsg=%s]. [seq=%u]",
                    jTmp["error_msg"].asString().c_str(), pstSession->m_stUserInfo.m_udwBSeqNo));
                return -3;
            }

            if (pstSession->m_stReqParam.m_ucIsSandBox == 0)
            {
                pstSession->m_stReqParam.m_ucIsSandBox = jTmp["sandbox"].asInt();
            }
        }
        */

        /******************************************* 正常购买iap情况 *******************************************/
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;

        if (!bIsWhiteUser && pstSession->m_stReqParam.m_ucIsSandBox != 0)
        {
            TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("ProcessCmd_GemRecharge: uid=%ld is not in white list but use sandbox[%u] [seq=%u]",
                pstUserInfo->m_tbPlayer.m_nUid, pstSession->m_stReqParam.m_ucIsSandBox, pstUserInfo->m_udwBSeqNo));
            //pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__IAP_WHITE_MAX;
            //return -1;
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
            return 0;
        }

        if (0 == dwRechargeType)
        {
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__IAP_SVR;
            IapSvrReqInfo* pstReq = new IapSvrReqInfo;
            pstReq->m_udwType = EN_HU_REQUEST_TYPE__RECHARGE;
            Json::Value rIapReqJson = Json::Value(Json::objectValue);

            rIapReqJson["sid"] = pstUserInfo->m_tbLogin.m_nSid;
            rIapReqJson["uid"] = pstUserInfo->m_tbPlayer.m_nUid;
            rIapReqJson["aid"] = pstUserInfo->m_tbPlayer.m_nAlpos == 0 ? 0 : pstUserInfo->m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET;
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
                pstUserInfo->m_tbPlayer.m_nUid, \
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
                        pstIapSvrRsp->m_sRspJson.c_str(), pstUserInfo->m_tbPlayer.m_nUid, pstSession->m_udwSeqNo));

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
        // 把该transid记录到iap_transaction表中
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
            return -6;
        }

        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__4)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__5;

        TbIap_transaction tbIapTrans;
        dwRetCode = CAwsResponse::OnUpdateItemRsp(*pstSession->m_vecAwsRsp[0], &tbIapTrans);

        // 没update成功,说明记录已经存在(写表导致超限的错误怎样处理)
        if(dwRetCode <= 0)
        {
            //打log 直接return
            strcpy(pstSession->m_stReqParam.m_szKey[7], "1");
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_GemRecharge: UpdateItem falid. sTransId:%s already exist!! [seq=%u]", sTransId.c_str(), pstSession->m_stUserInfo.m_udwBSeqNo));
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
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
                TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("ProcessReqCommand_GemRecharge:no promote reward [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
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
                    return -1;
                }
                else
                {
                    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("ProcessReqCommand_GemRecharge:add promote succ [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                }
                pstSession->m_ucPromoteReward = 1;
                pstSession->m_dwPromoteTpye = pstSession->m_JsonValue["recharge"]["type"].asInt();
            }
        }
        //透传promote信息
        Json::Value jIapPromote = Json::Value(Json::objectValue);
        jIapPromote["op_iap_promote"] = Json::Value(Json::objectValue);
        jIapPromote["op_iap_promote"]["promote"] = pstSession->m_JsonValue["promote"];
        pstSession->m_JsonValue = jIapPromote;
        pstSession->m_bHasPromoteData = TRUE;

        tbLogin.Set_Gem_seq(tbLogin.m_nGem_seq + 1);
        tbLogin.Set_Gem(tbLogin.m_nGem + udwRechargeGem);

        if (strcmp(pstSession->m_stReqParam.m_szItemId.c_str(), "1600gems9") == 0
            && ddwProjectId != 0)
        {
            TINT64 ddwExGem = CCommonBase::GetGameBasicVal(EN_GAME_BASIC_IAP_PROMOTE_EX_GEM);
            /*
            if (tbLogin.m_nIap_promote_gem_num == 0 && ddwExGem != 0)
            {
                TCHAR szTitle[128];
                sprintf(szTitle, "Accumulate FREE Gems Gifts");
                TCHAR szContent[4096];
                sprintf(szContent, "My Lord,\n"
                    "\n"
                    "Congratulations on your purchase of this great gems pack valued USD 99.99!\n"
                    "\n"
                    "To show our gratitude to your continued support, from now on, you will earn 100 more Gems for every purchase of USD 99.99 Pack. For example, if you buy 18,000 gems with USD 99.99 in your First Purchase, you'll get 18,100 Gems on your Second Purchase, and 18,200 gems for your Third Purchase...That is to say, with your accumulated purchases, you will receive a pack of greater value with accumulative free gems gifts.\n"
                    "\n"
                    "Gems are like magic. They are like your right-hand man to boost your city faster and stronger! Also they are like sharp swords to help you revenge hard and quick.\n"
                    "\n"
                    "Be a wise wizard and wield the Gems Magic right! And don't forget to check the gems store where surprises await!\n"
                    "\n"
                    "Cheers,\n"
                    "Blaze of Battle");
                CMsgBase::SendOperateMail(tbLogin.m_nUid, 0, tbLogin.m_nSid, SYSTEM_ACTIVITY, 2, szTitle, szContent, "", "");
            }
            */
            tbLogin.Set_Gem(tbLogin.m_nGem + tbLogin.m_nIap_promote_gem_num);
            tbLogin.Set_Iap_promote_gem_num(tbLogin.m_nIap_promote_gem_num + ddwExGem);
        }

        // 首次充值发送运营邮件
        if(tbLogin.m_nGem_buy == 0 && udwRechargeGem != 0)
        {
            CMsgBase::SendEncourageMail(&pstUserInfo->m_tbUserStat, pstSession->m_stReqParam.m_udwSvrId, EN_MAIL_ID__FIRST_IAP);
        }
        tbLogin.Set_Gem_buy(tbLogin.m_nGem_buy + udwRechargeGem);
        pstSession->m_udwGembuy = udwRechargeGem;

        if (udwRechargeGem >= tbLogin.m_nMax_buy)
        {
            tbLogin.Set_Max_buy(udwRechargeGem);
        }
        tbLogin.Set_Last_buy(udwRechargeGem);
        tbLogin.Set_Last_buy_time(CTimeUtils::GetUnixTime());

        TUINT32 udwPay = CCommonLogic::GetIapPay(pstSession->m_stReqParam.m_szItemId);
        TUINT32 udwPayCent = CCommonLogic::GetIapPayCent(pstSession->m_stReqParam.m_szItemId);
        tbLogin.Set_Total_pay(tbLogin.m_nTotal_pay + udwPay);
        if (udwPay > tbLogin.m_nMax_pay)
        {
            tbLogin.Set_Max_pay(udwPay);
        }
        tbLogin.Set_Last_pay(udwPay);

        if(bIsWhiteUser)
        {
            Json::Value jsonWeekGemRecharge;
            jsonWeekGemRecharge = Json::Value(Json::arrayValue);
            jsonWeekGemRecharge.append(tbLogin.m_jWeek_gem_recharge[0].asFloat() + fIapNum);
            tbLogin.Set_Week_gem_recharge(jsonWeekGemRecharge);
        }

        // wave@20130122: 真正充值标记(到这一步的证明上面的充值处理已经成功)
        pstSession->m_ucFakeRecharge = FALSE;

        //更新玩家的购买力标记
        TUINT32 udwAbility = CCommonLogic::GetPurchaseAbility(udwRechargeGem);
        if(udwAbility > tbLogin.m_nAbility)
        {
            tbLogin.Set_Ability(udwAbility);
        }

        //event add gem
        CActivitesLogic::ComputeBuyIapScore(pstUserInfo, udwPayCent);
    }

    if(EN_COMMAND_STEP__5 == pstSession->m_udwCommandStep)
    {
        //task count
        CQuestLogic::SetTaskCurValue(pstUserInfo,pstCity, EN_TASK_TYPE_BUY_GEMS, udwRechargeGem);

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessAccount::ProcessCmd_SvrInfoGet(SSession *pstSession, TBOOL &bNeedResponse)
{
    return 0;
}

TINT32 CProcessAccount::ProcessCmd_ApnsUpdate(SSession *pstSession, TBOOL &bNeedResponse)
{
    TCHAR *pszToken = pstSession->m_stReqParam.m_szKey[0];
    TUINT8 ucSwitch = atoi(pstSession->m_stReqParam.m_szKey[1]); // 0 close; 1 open
    TbLogin *pstLogin = &pstSession->m_stUserInfo.m_tbLogin;
    TbApns_token& tbApnsToken = pstSession->m_stUserInfo.m_tbApns_token;

    pstLogin->Set_Apns_token(pszToken);
    pstLogin->Set_Apns_switch(ucSwitch);
    tbApnsToken.Set_Token(pszToken);
    tbApnsToken.Set_Uid(pstLogin->m_nUid);

    if(strcmp(pstLogin->m_sPlatform.c_str(), pstSession->m_stReqParam.m_szPlatForm) != 0)
    {
        pstLogin->Set_Platform(pstSession->m_stReqParam.m_szPlatForm);
    }

    return 0;
}

TINT32 CProcessAccount::ProcessCmd_ApnsToken(SSession *pstSession, TBOOL &bNeedResponse)
{
    TCHAR *pszToken = pstSession->m_stReqParam.m_szKey[0];
    TbLogin *pstLogin = &pstSession->m_stUserInfo.m_tbLogin;
    TbApns_token& tbApnsToken = pstSession->m_stUserInfo.m_tbApns_token;

    pstLogin->Set_Apns_token(pszToken);
    tbApnsToken.Set_Token(pszToken);
    tbApnsToken.Set_Uid(pstLogin->m_nUid);

    if(strcmp(pstLogin->m_sPlatform.c_str(), pstSession->m_stReqParam.m_szPlatForm) != 0)
    {
        pstLogin->Set_Platform(pstSession->m_stReqParam.m_szPlatForm);
    }

    return 0;
}


TINT32 CProcessAccount::ProcessCmd_ApnsSwitch(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwNoticType = atoi(pstSession->m_stReqParam.m_szKey[0]); // 推送类型
    TINT32 dwSwitch = atoi(pstSession->m_stReqParam.m_szKey[1]); // 0表示关，1表示仅text开，2表示text和sound全开
    TbLogin *pstLogin = &pstSession->m_stUserInfo.m_tbLogin;

    if (dwNoticType >= EN_NOTIC_TYPE__END || dwNoticType <= EN_NOTIC_TYPE__UNKNOWN)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ApnsSwitch: notic type error [notictype=%u] [seq=%u]", 
            dwNoticType,
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }

    if(dwSwitch > EN_NOTIC_SWITCH_TYPE__TEXT_SOUND || dwSwitch < EN_NOTIC_SWITCH_TYPE__CLOSE)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ApnsSwitch: notic switch error [noticswitch=%u] [seq=%u]", 
            dwSwitch,
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -2;
    
    }

    Json::Value jNewApnsSwitch = Json::Value(Json::arrayValue);
    jNewApnsSwitch = pstLogin->m_jApns_switch;
    jNewApnsSwitch[dwNoticType] = dwSwitch;

    pstLogin->Set_Apns_switch(jNewApnsSwitch);

    return 0;
}



TINT32 CProcessAccount::AddPromoteReward(SSession *pstSession, const Json::Value &jReward)
{
    TINT32 dwRetCode = 0;
    SUserInfo* pstUser = &pstSession->m_stUserInfo;

    SCityInfo *pstCity = &pstUser->m_stCityInfo;

    //获取奖励信息
    SSpGlobalRes stReward;
    stReward.Reset();
    dwRetCode = CGlobalResLogic::GetSpGlobalResInfo(jReward, EREWARD_TYPE_ALL, &stReward);
    if(dwRetCode != 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GET_IAP_FAIL;
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("CProcessAccount::AddPromoteReward: get promote reward failed [ret=%d][seq=%u]",
            dwRetCode, pstSession->m_udwSeqNo));
        return -1;
    }

    //获得奖励
    dwRetCode = CGlobalResLogic::AddSpGlobalRes(pstUser, pstCity, &stReward);
    if(dwRetCode != 0)
    {
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("CProcessAccount::AddPromoteReward: add reward fail [uid=%ld][ret=%d][seq=%u]",
            pstUser->m_tbPlayer.m_nUid, dwRetCode, pstSession->m_udwSeqNo));
        return dwRetCode;
    }
    return 0;
}
