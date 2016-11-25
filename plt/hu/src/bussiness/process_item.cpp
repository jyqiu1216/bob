#include "process_item.h"
#include "statistic.h"
#include "game_info.h"
#include "procedure_base.h"
#include "global_serv.h"
#include "common_func.h"
#include "common_base.h"
#include "city_info.h"
#include "city_base.h"
#include "item_base.h"
#include "item_logic.h"
#include "action_base.h"
#include "player_base.h"
#include "sendmessage_base.h"
#include "process_action.h"
#include "wild_info.h"
#include "common_logic.h"
#include "common_func.h"
#include "globalres_logic.h"
#include "map_logic.h"
#include "map_base.h"
#include "pushdata_action.h"
#include "tool_base.h"

TINT32 CProcessItem::ProcessCmd_ItemUse(SSession *pstSession, TBOOL& bNeedResponse)
{
    TINT32 dwRetCode = 0;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;

    TUINT32 udwItemId = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TUINT64 uddwTargetId = strtoull(pstSession->m_stReqParam.m_szKey[1], NULL, 10);
    TUINT32 udwSecondClass = strtoull(pstSession->m_stReqParam.m_szKey[2], NULL, 10);
    TUINT32 udwItemNum = 1;

    TINT64 ddwRallyWarId = strtoll(pstSession->m_stReqParam.m_szKey[3], NULL, 10);
    TINT32 dwSide = atoi(pstSession->m_stReqParam.m_szKey[4]);


    if(ddwRallyWarId > 0)
    {
        if(dwSide == EN_RALLY_SIDE_ATTACK)
        {
            if(CActionBase::GetMarch(pstUser->m_atbMarch, pstUser->m_udwMarchNum, ddwRallyWarId) == NULL)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__TARGET_NOT_EXIST;
                return -1;
            }
        }
        else
        {
            if(CActionBase::GetMarch(pstUser->m_atbPassiveMarch, pstUser->m_udwPassiveMarchNum, ddwRallyWarId) == NULL)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__TARGET_NOT_EXIST;
                return -2;
            }
        }
    }
    
    if(FALSE == CItemBase::HasEnoughItem(&pstUser->m_tbBackpack, udwItemId, udwItemNum))
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ITEM_NOT_ENOUGH;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ItemUse: item not enough [seq=%u]", \
                                                pstSession->m_stUserInfo.m_udwBSeqNo));
        return -3;
    }


    // 水晶箱子和材料箱子除外使用物品逻辑
    if((EN_ITEM_FUNC_TYPE_CRYSTAL != CItemBase::GetItemFuncFromGameJson(udwItemId)
       && EN_ITEM_FUNC_TYPE_MATERIALS != CItemBase::GetItemFuncFromGameJson(udwItemId)
       && EN_ITEM_FUNC_TYPE_CHALLENGER != CItemBase::GetItemFuncFromGameJson(udwItemId)
       && EN_ITEM_FUNC_TYPE_MONSTER != CItemBase::GetItemFuncFromGameJson(udwItemId))
       || CItemLogic::IsChestLottery(udwItemId))
    {
        TBOOL bIsKing = FALSE;
        if (pstSession->m_tbThrone.m_nAlid > 0 && pstSession->m_tbThrone.m_nOwner_id == ptbPlayer->m_nUid)
        {
            bIsKing = TRUE;
        }
        dwRetCode = CItemLogic::UseItem(pstUser, pstCity, udwItemId, udwSecondClass, udwItemNum, uddwTargetId, bIsKing);
        if(dwRetCode != 0)
        {
            if(dwRetCode > 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = dwRetCode;
            }
            else
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            }
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ItemUse: item use failed [ret=%d] [seq=%u]", \
                                                     dwRetCode, \
                                                     pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }

        SSpGlobalRes stItemEffect;
        stItemEffect.Reset();
        dwRetCode = CItemLogic::GetItemreward(udwItemId, &stItemEffect);

        // 如果是加速物品，判定是否已经完成
        if(dwRetCode == 0)
        {
            for(TUINT32 udwIdx = 0; udwIdx < stItemEffect.udwTotalNum; ++udwIdx)
            {
                if(stItemEffect.aRewardList[udwIdx].udwType == EN_GLOBALRES_TYPE_SPEEDUP_TIME || stItemEffect.aRewardList[udwIdx].udwType == EN_GLOBALRES_TYPE_SPEEDUP_PERCENT)
                {
                    TUINT32 udwActionType = CActionBase::GetActionTypeBySecondClass(udwSecondClass);
                    TINT32 dwActionIdx = -1;
                    TUINT32 udwActionMainClass = 0;
                    TUINT32 udwActionSecondClass = 0;
                    TUINT32 udwActionEndTime = 0;
                    // 0. 获取action
                    switch(udwActionType)
                    {
                    case EN_ACTION_TYPE_BUFF_NORMAL:
                        dwActionIdx = CActionBase::GetActionIndex(pstUser->m_atbAction, pstUser->m_udwActionNum, uddwTargetId);
                        break;
                    case EN_ACTION_TYPE_AL_CAN_HELP:
                        dwActionIdx = CActionBase::GetAlActionIndex(pstUser->m_atbSelfAlAction, pstUser->m_udwSelfAlActionNum, uddwTargetId);
                        break;
                    case EN_ACTION_TYPE_MARCH:
                        dwActionIdx = CActionBase::GetMarchIndex(pstUser->m_atbMarch, pstUser->m_udwMarchNum, uddwTargetId);
                        break;
                    }
                    if(dwActionIdx >= 0)
                    {
                        switch(udwActionType)
                        {
                        case EN_ACTION_TYPE_BUFF_NORMAL:
                            udwActionMainClass = pstUser->m_atbAction[dwActionIdx].m_nMclass;
                            udwActionSecondClass = pstUser->m_atbAction[dwActionIdx].m_nSclass;
                            udwActionEndTime = pstUser->m_atbAction[dwActionIdx].m_nEtime;
                            break;
                        case EN_ACTION_TYPE_AL_CAN_HELP:
                            udwActionMainClass = pstUser->m_atbSelfAlAction[dwActionIdx].m_nMclass;
                            udwActionSecondClass = pstUser->m_atbSelfAlAction[dwActionIdx].m_nSclass;
                            udwActionEndTime = pstUser->m_atbSelfAlAction[dwActionIdx].m_nEtime;
                            break;
                        case EN_ACTION_TYPE_MARCH:
                            udwActionMainClass = pstUser->m_atbMarch[dwActionIdx].m_nMclass;
                            udwActionSecondClass = pstUser->m_atbMarch[dwActionIdx].m_nSclass;
                            udwActionEndTime = pstUser->m_atbMarch[dwActionIdx].m_nEtime;
                            break;
                        }
                        // 5.cb log
                        snprintf(pstSession->m_stReqParam.m_szKey[5], sizeof(pstSession->m_stReqParam.m_szKey[5]), "%u", udwActionMainClass);
                        snprintf(pstSession->m_stReqParam.m_szKey[6], sizeof(pstSession->m_stReqParam.m_szKey[6]), "%u", udwActionSecondClass);
                        snprintf(pstSession->m_stReqParam.m_szKey[7], sizeof(pstSession->m_stReqParam.m_szKey[7]), "%u", stItemEffect.aRewardList[udwIdx].udwNum);
                    }
                    else
                    {
                        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ACTION_NOT_EXIST;
                        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ItemUse: error action id [second=%u] [seq=%u]", \
                                                                udwActionSecondClass, \
                                                                pstSession->m_stUserInfo.m_udwBSeqNo));
                        return -4;
                    }

                    if(udwActionEndTime <= CTimeUtils::GetUnixTime())
                    {
                        if (udwActionMainClass == EN_ACTION_MAIN_CLASS__BUILDING
                            && pstUser->m_udwTipsNum > 0)
                        {
                            pstUser->m_atbTips[pstUser->m_udwTipsNum - 1].Reset();
                            pstUser->m_udwTipsNum--;
                        }
                        CProcessAction::ActionDone(pstSession, pstUser, pstCity, udwActionMainClass, udwActionSecondClass, dwActionIdx, TRUE);
                        
                        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessCmd_ItemUse: instant finish action! [seq=%u]", \
                                                               pstSession->m_stUserInfo.m_udwBSeqNo));

                    }

                    // 4. wave@push_data
                    switch(udwActionType)
                    {
                    case EN_ACTION_TYPE_BUFF_NORMAL:
                        break;
                    case EN_ACTION_TYPE_AL_CAN_HELP:
                        if(pstUser->m_aucSelfAlActionFlag[dwActionIdx] == EN_TABLE_UPDT_FLAG__DEL)
                        {
                            pstUser->m_ptbPushAlAction = &pstUser->m_atbSelfAlAction[dwActionIdx];
                            pstUser->dwPushAlActionType = pstUser->m_aucSelfAlActionFlag[dwActionIdx];
                        }                        
                        break;
                    case EN_ACTION_TYPE_MARCH:
                        pstUser->m_ptbPushMarchAction = &pstUser->m_atbMarch[dwActionIdx];
                        pstUser->dwPushMarchActionType = pstUser->m_aucMarchFlag[dwActionIdx];
                        break;
                    }
                }
            }
        }

        //如果是对队友的rally reinforce加速，向队友弹tips
        if(dwRetCode == 0 && udwSecondClass == EN_ACTION_SEC_CLASS__RALLY_REINFORCE)
        {
            TbMarch_action* ptbRallyReinforce = NULL;
            TBOOL bNeedToSendTip = FALSE;
            ptbRallyReinforce = CActionBase::GetMarch(pstUser->m_atbMarch, pstUser->m_udwMarchNum, uddwTargetId);
            if(ptbRallyReinforce != NULL && ptbRallyReinforce->m_nSuid != pstUser->m_tbPlayer.m_nUid)
            {
                for(TUINT32 udwIdx = 0; udwIdx < stItemEffect.udwTotalNum; ++udwIdx)
                {
                    if(ptbRallyReinforce != NULL && (stItemEffect.aRewardList[udwIdx].udwType == EN_GLOBALRES_TYPE_SPEEDUP_TIME || stItemEffect.aRewardList[udwIdx].udwType == EN_GLOBALRES_TYPE_SPEEDUP_PERCENT))
                    {
                        bNeedToSendTip = TRUE;
                        break;
                    }
                }

                if(bNeedToSendTip)
                {
                    CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPY__MARCH_TIME_IS_REDUCED, ptbRallyReinforce->m_nSuid, TRUE,
                        udwItemId, 0, 0, pstUser->m_tbPlayer.m_sUin.c_str());

                    //wave@push_data
                    pstUser->m_ptbPushTips = &pstUser->m_atbTips[pstUser->m_udwTipsNum-1];
                    pstUser->m_aucTipsFlag[pstUser->m_udwTipsNum-1] = EN_TABLE_UPDT_FLAG__DEL;
                }
            }
        }
        // 加速rally reinforce，向rally war发起者弹tips
        if (dwRetCode == 0 && udwSecondClass == EN_ACTION_SEC_CLASS__RALLY_REINFORCE)
        {
            TbMarch_action *ptbRallyReinforce = NULL;
            TBOOL bNeedToSendTip = FALSE;
            ptbRallyReinforce = CActionBase::GetMarch(pstUser->m_atbMarch, pstUser->m_udwMarchNum, uddwTargetId);
            if (ptbRallyReinforce != NULL && ptbRallyReinforce->m_nSuid != pstUser->m_tbPlayer.m_nUid)
            {
                for (TUINT32 udwIdx = 0; udwIdx < stItemEffect.udwTotalNum; ++udwIdx)
                {
                    if (ptbRallyReinforce != NULL && (stItemEffect.aRewardList[udwIdx].udwType == EN_GLOBALRES_TYPE_SPEEDUP_TIME || stItemEffect.aRewardList[udwIdx].udwType == EN_GLOBALRES_TYPE_SPEEDUP_PERCENT))
                    {
                        bNeedToSendTip = TRUE;
                        break;
                    }
                }
                if (bNeedToSendTip)
                {
                    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
                    {
                        TbMarch_action *ptbTmpMarch = &pstUser->m_atbMarch[udwIdx];
                        if (ptbTmpMarch->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR && ptbRallyReinforce->m_nTuid == ptbTmpMarch->m_nSuid)
                        {
                            CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPE__SPEED_UP_RALLY_REINFORCE, ptbTmpMarch->m_nSuid, TRUE,
                                ptbPlayer->m_nUid, ptbRallyReinforce->m_nSuid, 0, ptbPlayer->m_sUin.c_str(), ptbRallyReinforce->m_bParam[0].m_szSourceUserName);

                            //wave@push_data
                            pstUser->m_ptbPushTips = &pstUser->m_atbTips[pstUser->m_udwTipsNum - 1];
                            pstUser->m_aucTipsFlag[pstUser->m_udwTipsNum - 1] = EN_TABLE_UPDT_FLAG__DEL;

                            break;
                        }
                    }
                }
            }
        }

        return 0;

    }
    // 水晶箱子和材料箱子的使用逻辑
    else   
    {
        if(EN_COMMAND_STEP__INIT == pstSession->m_udwCommandStep)
        {
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__DATA_CENTER;
            pstSession->ResetDataCenterReq();


            DataCenterReqInfo* pstReq = new DataCenterReqInfo;
            pstReq->m_udwType = EN_REFRESH_DATA_TYPE__CHEST;
            Json::Value rDataReqJson = Json::Value(Json::objectValue);

            rDataReqJson["sid"] = pstUser->m_tbLogin.m_nSid;
            rDataReqJson["uid"] = pstUser->m_tbPlayer.m_nUid;
            rDataReqJson["castle_lv"] = CCityBase::GetBuildingLevelByFuncType(&pstUser->m_stCityInfo, EN_BUILDING_TYPE__CASTLE);
            rDataReqJson["request"] = Json::Value(Json::objectValue);
            rDataReqJson["request"]["chest_id"] = udwItemId;
            rDataReqJson["request"]["open_num"] = udwItemNum;
            
            Json::FastWriter rEventWriter;
            pstReq->m_sReqContent = rEventWriter.write(rDataReqJson);
            pstSession->m_vecDataCenterReq.push_back(pstReq);

            TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_ItemUse: data center req: [type=%u] [uid=%ld][seq=%u] [json=%s]", \
                                                    pstReq->m_udwType, \
                                                    pstUser->m_tbPlayer.m_nUid, \
                                                    pstSession->m_udwSeqNo, \
                                                    pstReq->m_sReqContent.c_str()));
            bNeedResponse = TRUE;
            TINT32 dwRetCode = CBaseProcedure::SendDataCenterRequest(pstSession, EN_SERVICE_TYPE_DATA_CENTER_REQ);
            if (dwRetCode == 0)
            {
                return 0;
            }
            else
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_DATA_CENTER_REQ_ERR;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ItemUse: send request to data center failed. [json=%s] [ret=%d] [seq=%u]", \
                                                        pstReq->m_sReqContent.c_str(), \
                                                        dwRetCode, \
                                                        pstUser->m_udwBSeqNo));
                return -5;
            }

            
        }


        if(EN_COMMAND_STEP__1 == pstSession->m_udwCommandStep)
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
                    if(EN_REFRESH_DATA_TYPE__CHEST == pstDataCenterRsp->m_udwType)
                    {
                        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_ItemUse: data center get rsp: [json=%s] [uid=%ld][seq=%u]",
                                                                pstDataCenterRsp->m_sRspJson.c_str(), \
                                                                pstUser->m_tbPlayer.m_nUid, \
                                                                pstUser->m_udwBSeqNo));

                        if (FALSE == jsonReader.parse(pstDataCenterRsp->m_sRspJson, oRspDataJson))
                        {
                            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_DATA_CENTER_PACKAGE_ERR;
                            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ItemUse: prase rsp from data center failed. [seq=%u]", \
                                                                    pstUser->m_udwBSeqNo));
                            return -6;
                        }
                        TINT32 dwRetCode = stRefreshData.m_stChestRsp.setVal(oRspDataJson);
                        if (0 != dwRetCode)
                        {
                            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__DATA_CENTER_PACKAGE_FORMAT_ERR;
                            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ItemUse: response data format error. [ret=%d][seq=%u]", \
                                                                    dwRetCode, \
                                                                    pstUser->m_udwBSeqNo));
                            return -7;
                        }
                        break;
                    }
                }


                if(MAX_SP_REWARD_ITEM_NUM < stRefreshData.m_stChestRsp.m_vecReward.size()
                    || 0 >= stRefreshData.m_stChestRsp.m_vecReward.size())
                {
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ItemUse: chest reward size is over. [size=%ld] [seq=%u]", \
                                                            stRefreshData.m_stChestRsp.m_vecReward.size(), \
                                                            pstUser->m_udwBSeqNo));
                    return -8;
                }


                SSpGlobalRes stGlobalRes;
                stGlobalRes.Reset();

                for(TINT32 dwIdx = 0; dwIdx < stRefreshData.m_stChestRsp.m_vecReward.size(); ++dwIdx)
                {
                    SOneGlobalRes *pstOneGlobalRes = stRefreshData.m_stChestRsp.m_vecReward[dwIdx];
                    stGlobalRes.aRewardList[dwIdx].udwType = pstOneGlobalRes->ddwType;
                    stGlobalRes.aRewardList[dwIdx].udwId = pstOneGlobalRes->ddwId;
                    stGlobalRes.aRewardList[dwIdx].udwNum = pstOneGlobalRes->ddwNum;
                    ++stGlobalRes.udwTotalNum;
                    dwRetCode = CGlobalResLogic::AddGlobalRes(pstUser, &pstUser->m_stCityInfo, pstOneGlobalRes->ddwType, pstOneGlobalRes->ddwId, pstOneGlobalRes->ddwNum);
                    if (dwRetCode > 0)
                    {
                        pstSession->m_stCommonResInfo.m_dwRetCode = dwRetCode;
                        return -1;
                    }
                    else if (dwRetCode < 0)
                    {
                        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
                        return -2;
                    }
                }
                
                // cost item 
                CItemBase::CostItem(&pstUser->m_tbBackpack, udwItemId);
         
                pstUser->m_udwLotteryChestItemId = udwItemId;
                pstUser->m_udwLotteryChestItemNum = 1;
                pstUser->m_stRewardWindow = stGlobalRes;
                
                Json::Value jTmp = Json::Value(Json::objectValue);
                jTmp["chest"] = Json::Value(Json::objectValue);
                jTmp["chest"]["id"] = udwItemId;
                jTmp["chest"]["num"] = 1;
                
                CCommonBase::AddRewardWindow(pstUser, pstUser->m_tbPlayer.m_nUid, EN_REWARD_WIMDOW_TYPE_CHEST, EN_REWARD_WINDOW_GET_TYPE_OPEN_CHEST,
                                             0, &stGlobalRes, FALSE, jTmp);

                
                pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
                return 0;
                
            }
            else
            {
                pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
                return 0;
            }
        }
        return 0;
    }
}

TINT32 CProcessItem::ProcessCmd_ItemBuy(SSession *pstSession, TBOOL& bNeedResponse)
{
    TINT32 dwRetCode = 0;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;

    TUINT32 udwItemId = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TUINT32 udwItemNum = 1;
    TUINT32 udwPrice = strtoul(pstSession->m_stReqParam.m_szKey[1], NULL, 10);

    pstSession->m_udwGemCost = udwPrice;

    dwRetCode = CItemBase::CheckItemPrice(udwItemId, 1, udwPrice);
    if(dwRetCode != 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__BLACK_ACCOUNT;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__ItemBuy: client price is not match with svr ret[%d] [seq=%u]", dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }

    if(FALSE == CPlayerBase::HasEnoughGem(&pstUser->m_tbLogin, udwPrice))
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GEM_LACK;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__ItemBuy: gem not enough gem_own=%u price=%u [seq=%u]", pstUser->m_tbLogin.Get_Gem(), udwPrice, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -3;
    }

    // 1. buy
    dwRetCode = CItemLogic::BuyItem(pstUser, &pstUser->m_tbBackpack, udwItemId, udwItemNum, udwPrice);
    if(dwRetCode != 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = dwRetCode;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__ItemBuy: BuyItem logic err check CItemLogic::BuyItem ret[%d] [seq=%u]", dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -4;
    }

    return 0;
}

TINT32 CProcessItem::ProcessCmd_ItemBuyAndUse(SSession *pstSession, TBOOL& bNeedResponse)
{
    TINT32 dwRetCode = 0;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;
    
    TUINT32 udwItemId = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TUINT64 uddwTargetId = strtoull(pstSession->m_stReqParam.m_szKey[1], NULL, 10);
    TUINT32 udwPrice = strtoul(pstSession->m_stReqParam.m_szKey[2], NULL, 10);
    TUINT32 udwSecondClass = strtoull(pstSession->m_stReqParam.m_szKey[3], NULL, 10);
    TINT64 ddwRallyWarId = strtoll(pstSession->m_stReqParam.m_szKey[4], NULL, 10);
    TINT32 dwSide = atoi(pstSession->m_stReqParam.m_szKey[5]);

    // @jonathan 下面cblog复用了key5导致值被覆盖，所以占用key8
    strncpy(pstSession->m_stReqParam.m_szKey[8], pstSession->m_stReqParam.m_szKey[5], DEFAULT_PARAM_STR_LEN);

    if(ddwRallyWarId > 0)
    {
        if(dwSide == EN_RALLY_SIDE_ATTACK)
        {
            if(CActionBase::GetMarch(pstUser->m_atbMarch, pstUser->m_udwMarchNum, ddwRallyWarId) == NULL)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__TARGET_NOT_EXIST;
                return -1;
            }
        }
        else
        {
            if(CActionBase::GetMarch(pstUser->m_atbPassiveMarch, pstUser->m_udwPassiveMarchNum, ddwRallyWarId) == NULL)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__TARGET_NOT_EXIST;
                return -2;
            }
        }
    }

    pstSession->m_udwGemCost = udwPrice;
    dwRetCode = CItemBase::CheckItemPrice(udwItemId, 1, udwPrice);
    if(dwRetCode != 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__BLACK_ACCOUNT;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__ItemBuyAndUse: client price is not match with svr ret ret[%d] [seq=%u]", dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -3;
    }

    if(FALSE == CPlayerBase::HasEnoughGem(&pstUser->m_tbLogin, udwPrice))
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GEM_LACK;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__ItemBuy: gem not enough gem_own=%u price=%u [seq=%u]", pstUser->m_tbLogin.Get_Gem(), udwPrice, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -4;
    }

    // 1. buy and use
    TBOOL bIsKing = FALSE;
    if (pstSession->m_tbThrone.m_nAlid > 0 && pstSession->m_tbThrone.m_nOwner_id == ptbPlayer->m_nUid)
    {
        bIsKing = TRUE;
    }
    dwRetCode = CItemLogic::BuyAndUseItem(pstUser, pstCity, udwItemId, udwPrice, udwSecondClass, uddwTargetId, bIsKing);
    if(dwRetCode != 0)
    {
        if(dwRetCode > 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = dwRetCode;
        }
        else
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        }
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__ItemBuyAndUse: ret[%d] [seq=%u]", dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -5;
    }

    SSpGlobalRes stItemEffect;
    stItemEffect.Reset();
    dwRetCode = CItemLogic::GetItemreward(udwItemId, &stItemEffect);

    // 如果是加速物品，判定是否已经完成
    if(dwRetCode==0)
    {
        for(TUINT32 udwIdx = 0; udwIdx < stItemEffect.udwTotalNum; ++udwIdx)
        {
            if(stItemEffect.aRewardList[udwIdx].udwType == EN_GLOBALRES_TYPE_SPEEDUP_TIME || stItemEffect.aRewardList[udwIdx].udwType == EN_GLOBALRES_TYPE_SPEEDUP_PERCENT)
            {
                TUINT32 udwActionType = CActionBase::GetActionTypeBySecondClass(udwSecondClass);
                TINT32 dwActionIdx = -1;
                TUINT32 udwActionMainClass = 0;
                TUINT32 udwActionSecondClass = 0;
                TUINT32 udwActionEndTime = 0;

                // 0. 获取action
                switch(udwActionType)
                {
                case EN_ACTION_TYPE_BUFF_NORMAL:
                    dwActionIdx = CActionBase::GetActionIndex(pstUser->m_atbAction, pstUser->m_udwActionNum, uddwTargetId);
                    break;
                case EN_ACTION_TYPE_AL_CAN_HELP:
                    dwActionIdx = CActionBase::GetAlActionIndex(pstUser->m_atbSelfAlAction, pstUser->m_udwSelfAlActionNum, uddwTargetId);
                    break;
                case EN_ACTION_TYPE_MARCH:
                    dwActionIdx = CActionBase::GetMarchIndex(pstUser->m_atbMarch, pstUser->m_udwMarchNum, uddwTargetId);
                    break;
                }
                if(dwActionIdx >= 0)
                {
                    switch(udwActionType)
                    {
                    case EN_ACTION_TYPE_BUFF_NORMAL:
                        udwActionMainClass = pstUser->m_atbAction[dwActionIdx].m_nMclass;
                        udwActionSecondClass = pstUser->m_atbAction[dwActionIdx].m_nSclass;
                        udwActionEndTime = pstUser->m_atbAction[dwActionIdx].m_nEtime;
                        break;
                    case EN_ACTION_TYPE_AL_CAN_HELP:
                        udwActionMainClass = pstUser->m_atbSelfAlAction[dwActionIdx].m_nMclass;
                        udwActionSecondClass = pstUser->m_atbSelfAlAction[dwActionIdx].m_nSclass;
                        udwActionEndTime = pstUser->m_atbSelfAlAction[dwActionIdx].m_nEtime;
                        break;
                    case EN_ACTION_TYPE_MARCH:
                        udwActionMainClass = pstUser->m_atbMarch[dwActionIdx].m_nMclass;
                        udwActionSecondClass = pstUser->m_atbMarch[dwActionIdx].m_nSclass;
                        udwActionEndTime = pstUser->m_atbMarch[dwActionIdx].m_nEtime;
                        break;
                    }
                    // 5.cb log
                    snprintf(pstSession->m_stReqParam.m_szKey[5], sizeof(pstSession->m_stReqParam.m_szKey[5]), "%u", udwActionMainClass);
                    snprintf(pstSession->m_stReqParam.m_szKey[6], sizeof(pstSession->m_stReqParam.m_szKey[6]), "%u", udwActionSecondClass);
                    snprintf(pstSession->m_stReqParam.m_szKey[7], sizeof(pstSession->m_stReqParam.m_szKey[7]), "%u", stItemEffect.aRewardList[udwIdx].udwNum);
                }
                else
                {
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ACTION_NOT_EXIST;
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__ActionGemSpeedUp: error action id [second=%u] [seq=%u]", udwActionSecondClass, pstSession->m_stUserInfo.m_udwBSeqNo));
                    return -2;
                }

                if(udwActionEndTime <= CTimeUtils::GetUnixTime())
                {
                    //建造或研究完成时..屏蔽使用item的tips
                    if (udwActionMainClass == EN_ACTION_MAIN_CLASS__BUILDING
                        && pstUser->m_udwTipsNum > 0)
                    {
                        pstUser->m_atbTips[pstUser->m_udwTipsNum - 1].Reset();
                        pstUser->m_udwTipsNum--;
                    }
                    CProcessAction::ActionDone(pstSession, pstUser, pstCity, udwActionMainClass, udwActionSecondClass, dwActionIdx, TRUE);
                    TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessCmd__ActionGemSpeedUp: instant finish action! [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                }

                // 4. wave@push_data
                switch(udwActionType)
                {
                case EN_ACTION_TYPE_BUFF_NORMAL:
                    break;
                case EN_ACTION_TYPE_AL_CAN_HELP:
                    if(pstUser->m_aucSelfAlActionFlag[dwActionIdx] == EN_TABLE_UPDT_FLAG__DEL)
                    {
                        pstUser->m_ptbPushAlAction = &pstUser->m_atbSelfAlAction[dwActionIdx];
                        pstUser->dwPushAlActionType = pstUser->m_aucSelfAlActionFlag[dwActionIdx];
                    }                        
                    break;
                case EN_ACTION_TYPE_MARCH:
                    pstUser->m_ptbPushMarchAction = &pstUser->m_atbMarch[dwActionIdx];
                    pstUser->dwPushMarchActionType = pstUser->m_aucMarchFlag[dwActionIdx];
                    break;
                }
            }
        }
    }

    //如果是对队友的rally reinforce加速，向队友弹tips
    if(dwRetCode == 0 && udwSecondClass == EN_ACTION_SEC_CLASS__RALLY_REINFORCE)
    {
        TbMarch_action* ptbRallyReinforce = NULL;
        TBOOL bNeedToSendTip = FALSE;
        ptbRallyReinforce = CActionBase::GetMarch(pstUser->m_atbMarch, pstUser->m_udwMarchNum, uddwTargetId);
        if(ptbRallyReinforce != NULL && ptbRallyReinforce->m_nSuid != pstUser->m_tbPlayer.m_nUid)
        {
            for(TUINT32 udwIdx = 0; udwIdx < stItemEffect.udwTotalNum; ++udwIdx)
            {
                if(ptbRallyReinforce != NULL && (stItemEffect.aRewardList[udwIdx].udwType == EN_GLOBALRES_TYPE_SPEEDUP_TIME || stItemEffect.aRewardList[udwIdx].udwType == EN_GLOBALRES_TYPE_SPEEDUP_PERCENT))
                {
                    bNeedToSendTip = TRUE;
                    break;
                }
            }

            if(bNeedToSendTip)
            {
                CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPY__MARCH_TIME_IS_REDUCED, ptbRallyReinforce->m_nSuid, TRUE,
                    udwItemId, 0, 0, pstUser->m_tbPlayer.m_sUin.c_str());

                //wave@push_data
                pstUser->m_ptbPushTips = &pstUser->m_atbTips[pstUser->m_udwTipsNum-1];
                pstUser->m_aucTipsFlag[pstUser->m_udwTipsNum-1] = EN_TABLE_UPDT_FLAG__DEL;
            }
        }
    }
    // 加速rally reinforce，向rally war发起者弹tips
    if (dwRetCode == 0 && udwSecondClass == EN_ACTION_SEC_CLASS__RALLY_REINFORCE)
    {
        TbMarch_action *ptbRallyReinforce = NULL;
        TBOOL bNeedToSendTip = FALSE;
        ptbRallyReinforce = CActionBase::GetMarch(pstUser->m_atbMarch, pstUser->m_udwMarchNum, uddwTargetId);
        if(ptbRallyReinforce != NULL && ptbRallyReinforce->m_nSuid != pstUser->m_tbPlayer.m_nUid)
        {
            for(TUINT32 udwIdx = 0; udwIdx < stItemEffect.udwTotalNum; ++udwIdx)
            {
                if(ptbRallyReinforce != NULL && (stItemEffect.aRewardList[udwIdx].udwType == EN_GLOBALRES_TYPE_SPEEDUP_TIME || stItemEffect.aRewardList[udwIdx].udwType == EN_GLOBALRES_TYPE_SPEEDUP_PERCENT))
                {
                    bNeedToSendTip = TRUE;
                    break;
                }
            }
            if(bNeedToSendTip)
            {
                for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
                {
                    TbMarch_action *ptbTmpMarch = &pstUser->m_atbMarch[udwIdx];
                    if (ptbTmpMarch->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR && ptbRallyReinforce->m_nTuid == ptbTmpMarch->m_nSuid)
                    {
                        CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPE__SPEED_UP_RALLY_REINFORCE, ptbTmpMarch->m_nSuid, TRUE,
                            ptbPlayer->m_nUid, ptbRallyReinforce->m_nSuid, 0, ptbPlayer->m_sUin.c_str(), ptbRallyReinforce->m_bParam[0].m_szSourceUserName);

                        //wave@push_data
                        pstUser->m_ptbPushTips = &pstUser->m_atbTips[pstUser->m_udwTipsNum - 1];
                        pstUser->m_aucTipsFlag[pstUser->m_udwTipsNum - 1] = EN_TABLE_UPDT_FLAG__DEL;

                        break;
                    }
                }
            }
        }
    }

    return 0;
}

TINT32 CProcessItem::ProcessCmd_RandomMoveCity(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    TbMap& tbTmpMap = pstSession->m_tbTmpMap;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;

    // 0. 输入参数
    TUINT8 ucProvince = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TUINT32 udwOldPos = atoi(pstSession->m_stReqParam.m_szKey[1]);

    // 1. 获取地图信息
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__MAP_SVR;

        dwRetCode = CItemLogic::CanMoveCity(pstUser);
        if (dwRetCode != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = dwRetCode;
            return -1;
        }

        //wave@20131127:添加物品检查保护
        if (CItemBase::HasEnoughItem(&pstSession->m_stUserInfo.m_tbBackpack, EN_ITEM_ID__PORTAL_OF_REFUGE) == FALSE)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ITEM_NOT_ENOUGH;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_RandomMoveCity: item not enough [id=%u] [seq=%u]", EN_ITEM_ID__PORTAL_OF_REFUGE, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }

        for (TUINT32 idx = 0; idx < pstUser->m_udwActionNum; idx++)
        {
            TbAction *ptbAction = &pstUser->m_atbAction[idx];
            if (ptbAction->m_nMclass == EN_ACTION_SEC_CLASS__ATTACK_MOVE
                && ptbAction->m_bParam[0].m_stAttackMove.m_ddwSpecailMoveFlag == 1)
            {
                //pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__BE_KICKING_OUT;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MoveCityPrepare: be kicking out [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
                return 0;
            }
        }

        if (udwOldPos != 0 && udwOldPos != pstUser->m_stCityInfo.m_stTblData.m_nPos)
        {
            //pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__HAS_KICKED_OUT;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MoveCityPrepare: has kicked out [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
            return 0;
        }

        //随机选取野地建城
        pstSession->ResetMapSvrReq();
        MapSvrReqInfo* pMapSvrReq = new MapSvrReqInfo(pstSession->m_stReqParam.m_udwSvrId, "GetNewCityMap");
        Json::FastWriter writer;
        writer.omitEndingLineFeed();

        CJsoncppSeri jSeri;

        Json::Value jTmp = Json::Value(Json::objectValue);
        jTmp["type"] = 1; // 0: new user 1: random move 2: attack move
        jTmp["zone"] = -1;
        jTmp["province"] = ucProvince;

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
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__RandomMoveCity: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }

        return 0;
    }

    // 2. 获取响应
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        tbTmpMap.Reset();
        // a.parse data
        TINT32 dwParseCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[0], &tbTmpMap);
        if (dwParseCode <= 0 || tbTmpMap.m_nId == 0 || tbTmpMap.m_nUid != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__CAN_NOT_FIND_MAP_FOR_RANDOM_MOVE;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__RandomMoveCity: can not find the map for random move [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }

        pstSession->m_udwCommand = EN_CLIENT_REQ_COMMAND__MOVE_CITY;
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;

        strncpy(&pstSession->m_stReqParam.m_szKey[0][0], CCommonFunc::NumToString(pstSession->m_tbTmpMap.m_nId).c_str(), DEFAULT_PARAM_STR_LEN - 1);
        pstSession->m_stReqParam.m_szKey[0][DEFAULT_PARAM_STR_LEN - 1] = 0;
        strncpy(&pstSession->m_stReqParam.m_szKey[1][0], CCommonFunc::NumToString(0).c_str(), DEFAULT_PARAM_STR_LEN - 1);
        pstSession->m_stReqParam.m_szKey[1][DEFAULT_PARAM_STR_LEN - 1] = 0;

        pstSession->m_bGotoOtherCmd = TRUE;
    }
    
    return 0;
}

TINT32 CProcessItem::ProcessCmd_MoveCity(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TbAlliance *pstAlliance = &pstUser->m_tbAlliance;

    // 1. 获取输入参数
    TUINT32 udwPos = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TUINT32 udwGemCost = atoi(pstSession->m_stReqParam.m_szKey[1]);
    //key2 item_id
    TUINT32 udwOldPos = atoi(pstSession->m_stReqParam.m_szKey[3]);

    pstSession->m_udwGemCost = udwGemCost;

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__MAP_SVR;

        dwRetCode = CItemLogic::CanMoveCity(pstUser);
        if (dwRetCode != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = dwRetCode;
            return -1;
        }

        //wave@20131127:添加物品检查保护
        if (CItemBase::HasEnoughItem(&pstUser->m_tbBackpack, EN_ITEM_ID__PORTAL_OF_ORDER) == FALSE)
        {
            if (CPlayerBase::HasEnoughGem(&pstUser->m_tbLogin, udwGemCost) == FALSE)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GEM_LACK;
                return -1;
            }
        }

        for (TUINT32 idx = 0; idx < pstUser->m_udwActionNum; idx++)
        {
            TbAction *ptbAction = &pstUser->m_atbAction[idx];
            if (ptbAction->m_nMclass == EN_ACTION_SEC_CLASS__ATTACK_MOVE
                && ptbAction->m_bParam[0].m_stAttackMove.m_ddwSpecailMoveFlag == 1)
            {
                //pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__BE_KICKING_OUT;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MoveCity: be kicking out [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
                return 0;
            }
        }

        if (udwOldPos != 0 && udwOldPos != pstCity->m_stTblData.m_nPos)
        {
            //pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__HAS_KICKED_OUT;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MoveCity: has kicked out [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
            return 0;
        }

        // set request package
        pstSession->ResetMapSvrReq();

        MapSvrReqInfo* pMapSvrReq = new MapSvrReqInfo(pstSession->m_stReqParam.m_udwSvrId, "GetNewCityMap");
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

    // 3. 获取wild信息
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[0],
            &pstSession->m_tbTmpMap);

        if (dwRetCode <= 0 || pstSession->m_tbTmpMap.m_nId == 0 || pstSession->m_tbTmpMap.m_nUid != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__MOVE_CITY_COORD_INVALID;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MoveCity: wild belong someone else. [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -4;
        }
    }

    // 4. 处理
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        TbMap tbMap;
        tbMap.Set_Sid(pstSession->m_stReqParam.m_udwSvrId);
        tbMap.Set_Id(pstSession->m_stReqParam.m_udwCityId);
        CAwsRequest::GetItem(pstSession, &tbMap, ETbMAP_OPEN_TYPE_PRIMARY, true, true);

        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MoveCity: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
        return 0;
    }

    // 5. update action and map
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__4;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        TbMap tbOldCityMap;
        dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[0],
            &tbOldCityMap);

        if (dwRetCode <= 0 || tbOldCityMap.m_nId == 0)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MoveCity: get old city map failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            pstSession->m_tbTmpMap.Set_Sid(pstSession->m_stReqParam.m_udwSvrId);
        }
        else
        {
            TUINT32 udwSmokingTime = 0;
            TUINT32 udwBurnTime = 0;
            TUINT32 udwCatchHeroFlag = 0;
            Json::Value jCItyInfo;
            jCItyInfo.clear();
            for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwWildNum; ++udwIdx)
            {
                TbMap* ptbMap = &pstUser->m_atbWild[udwIdx];
                if (EN_WILD_TYPE__CITY != ptbMap->m_nType)
                {
                    continue;
                }
                if (ptbMap->m_nUid != pstUser->m_tbPlayer.m_nUid)
                {
                    continue;
                }
                if ((pstUser->m_tbPlayer.m_nStatus & EN_CITY_STATUS__BEEN_ATTACK) &&
                    ptbMap->Get_Smoke_end_time() > CTimeUtils::GetUnixTime())
                {
                    udwSmokingTime = ptbMap->Get_Smoke_end_time();
                }
                if (pstUser->m_tbPlayer.m_nStatus & EN_CITY_STATUS__BEEN_REMOVE &&
                    ptbMap->Get_Burn_end_time() > CTimeUtils::GetUnixTime())
                {
                    udwBurnTime = ptbMap->Get_Burn_end_time();
                }
                udwCatchHeroFlag = ptbMap->m_nPrison_flag;

                jCItyInfo = ptbMap->m_jCity_info;
            }

            TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessCmd_MoveCity:set map info to new city  [uid=%u sid=%u center_pos=%u] [smok_time=%u burn_time=%u catch_flag=%u] [step_2] [seq=%u]",
                pstSession->m_stUserInfo.m_tbPlayer.m_nUid,
                pstSession->m_stReqParam.m_udwSvrId,
                pstSession->m_tbTmpMap.m_nId,
                udwSmokingTime,
                udwBurnTime,
                udwCatchHeroFlag,
                pstSession->m_udwSeqNo));

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
            CCommonBase::SetMapToNewCity(&pstSession->m_tbTmpMap, &pstUser->m_tbPlayer, pstCity, pstAlliance, udwSmokingTime, udwBurnTime, udwCatchHeroFlag);

            //设置保护时间
            pstSession->m_tbTmpMap.Set_Time_end(pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_PEACE_TIME].m_astBuffDetail[EN_BUFF_TYPE_ITEM].m_dwTime);

            pstSession->m_tbTmpMap.Set_City_info(jCItyInfo);
        }

        // set package
        pstSession->ResetAwsReq();
        CAwsRequest::UpdateItem(pstSession, &pstSession->m_tbTmpMap, ExpectedDesc(), 0, true);

        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MoveCity: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -5;
        }
        return 0;
    }

    // 6. 最后更新本地city、city所在map和player信息
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__4)
    {
        if (pstSession->m_tbTmpMap.m_nUid == 0)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MoveCity: get old city map failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PROCESS_CMD_ERROR;
            return -10;
        }

        TUINT32 udwOldCityId = pstSession->m_stReqParam.m_udwCityId;
        TUINT32 udwNewCityId = pstSession->m_tbTmpMap.m_nId;

        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessCmd_MoveCity:old city and new city [uid=%u sid=%u center_pos=%u] [old_city=%u new_city=%u] [step_3] [seq=%u]",
            pstSession->m_stUserInfo.m_tbPlayer.m_nUid,
            pstSession->m_stReqParam.m_udwSvrId,
            pstSession->m_tbTmpMap.m_nId,
            udwOldCityId,
            udwNewCityId,
            pstSession->m_udwSeqNo));

        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__5;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        // set package
        pstSession->ResetAwsReq();

        // 更新assist数据
        for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwAlAssistAllNum; udwIdx++)
        {
            if (pstUser->m_atbAlAssistAll[udwIdx].m_nCid == udwOldCityId)
            {
                pstUser->m_atbAlAssistAll[udwIdx].Set_Cid(udwNewCityId);
                pstUser->m_aucAlAssistAllFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
            }
        }

        // 更新action数据
        for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; udwIdx++)
        {
            TbMarch_action *ptbMarch = &pstUser->m_atbMarch[udwIdx];
            if (ptbMarch->m_nSuid == pstUser->m_tbPlayer.m_nUid)
            {
                if (ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR
                    || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR_THRONE)
                {
                    for (TUINT32 udwIdy = 0; udwIdy < pstUser->m_udwPassiveMarchNum; udwIdy++)
                    {
                        if (pstUser->m_atbPassiveMarch[udwIdy].m_nSclass == EN_ACTION_SEC_CLASS__RALLY_REINFORCE
                            && pstUser->m_atbPassiveMarch[udwIdy].m_nTid == ptbMarch->m_nId)
                        {
                            if (pstUser->m_atbPassiveMarch[udwIdy].m_nStatus == EN_MARCH_STATUS__MARCHING)
                            {
                                CActionBase::ReleaseSlot(ptbMarch, &pstUser->m_atbPassiveMarch[udwIdy], TRUE);
                                CActionBase::UpdateRallyForce(ptbMarch, &pstUser->m_atbPassiveMarch[udwIdy], TRUE);
                                CActionBase::ReturnMarchOnFly(&pstUser->m_atbPassiveMarch[udwIdy]);
                                pstUser->m_aucPassiveMarchFlag[udwIdy] = EN_TABLE_UPDT_FLAG__CHANGE;
                            }
                            else if (pstUser->m_atbPassiveMarch[udwIdy].m_nStatus != EN_MARCH_STATUS__RETURNING)
                            {
                                TINT64 ddwCtime = CToolBase::GetMarchTime(&pstUser->m_atbPassiveMarch[udwIdy], udwNewCityId);
                                pstUser->m_atbPassiveMarch[udwIdy].m_bParam[0].m_ddwMarchingTime = ddwCtime;
                                pstUser->m_atbPassiveMarch[udwIdy].SetFlag(TbMARCH_ACTION_FIELD_PARAM);
                                pstUser->m_atbPassiveMarch[udwIdy].Set_Tpos(udwNewCityId);
                                pstUser->m_atbPassiveMarch[udwIdy].Set_Tbid(CMapBase::GetBlockIdFromPos(udwNewCityId));
                                pstUser->m_aucPassiveMarchFlag[udwIdy] = EN_TABLE_UPDT_FLAG__CHANGE;
                            }
                        }
                    }
                    if (ptbMarch->m_nStatus == EN_MARCH_STATUS__WAITING)
                    {
                        ptbMarch->Set_Etime(CTimeUtils::GetUnixTime());
                    }

                    ptbMarch->Set_Scid(udwNewCityId);
                    ptbMarch->Set_Sbid(CMapBase::GetBlockIdFromPos(udwNewCityId));
                    pstUser->m_aucMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
                }
                else if (ptbMarch->m_nScid == udwOldCityId)
                {
                    ptbMarch->Set_Scid(udwNewCityId);
                    ptbMarch->Set_Sbid(CMapBase::GetBlockIdFromPos(udwNewCityId));
                    pstUser->m_aucMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
                }
            }
        }

        for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwPassiveMarchNum; udwIdx++)
        {
            TBOOL bUpdate = FALSE;
            TbMarch_action *ptbPassiveMarch = &pstUser->m_atbPassiveMarch[udwIdx];

            // 移城后rallywar变化
            if (ptbPassiveMarch->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR
                && ptbPassiveMarch->m_nTuid == pstUser->m_tbPlayer.m_nUid)
            {
                ptbPassiveMarch->m_bParam[0].m_ddwTargetCityId = udwNewCityId;
                ptbPassiveMarch->SetFlag(TbMARCH_ACTION_FIELD_PARAM);
                bUpdate = TRUE;
            }
            else if (ptbPassiveMarch->m_nTpos == udwOldCityId
                && ptbPassiveMarch->m_nTuid == pstUser->m_tbPlayer.m_nUid)
            {
                if (ptbPassiveMarch->m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_NORMAL)
                {
                    if (ptbPassiveMarch->m_nStatus == EN_MARCH_STATUS__MARCHING)
                    {
                        bUpdate = TRUE;
                        CActionBase::ReturnMarchOnFly(ptbPassiveMarch);
                    }
                    else if (ptbPassiveMarch->m_nStatus != EN_MARCH_STATUS__RETURNING)
                    {
                        bUpdate = TRUE;
                        CActionBase::ReturnMarch(ptbPassiveMarch);
                    }
                }
                else if (ptbPassiveMarch->m_nMclass == EN_ACTION_MAIN_CLASS__TIMER 
                    && ptbPassiveMarch->m_nSclass == EN_ACTION_SEC_CLASS__PRISON_TIMER)
                {
                    bUpdate = TRUE;
                    ptbPassiveMarch->Set_Tpos(udwNewCityId);
                    ptbPassiveMarch->Set_Tbid(CMapBase::GetBlockIdFromPos(udwNewCityId));
                }
                else
                {
                    bUpdate = TRUE;
                    ptbPassiveMarch->Set_Tal(0);
                }
            }

            if (bUpdate)
            {
                pstUser->m_aucPassiveMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
            }
        }

        TbMap tbOldCityMap;
        tbOldCityMap.Reset();
        tbOldCityMap.Set_Sid(pstSession->m_stReqParam.m_udwSvrId);
        tbOldCityMap.Set_Id(udwOldCityId);
        tbOldCityMap.Set_Type(EN_WILD_TYPE__CITY);
        CCommonBase::AbandonWild(pstCity, &tbOldCityMap);

        CAwsRequest::UpdateItem(pstSession, &tbOldCityMap, ExpectedDesc(), 0, true);

        //wave@push_data
        CPushDataBasic::PushDataMap_SingleWild(pstSession, &tbOldCityMap);
        CPushDataBasic::PushDataMap_SingleWild(pstSession, &pstSession->m_tbTmpMap);

        // send request
        if (pstSession->m_vecAwsReq.size() > 0)
        {
            bNeedResponse = TRUE;
            dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
            if (dwRetCode < 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MoveCity: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                return -6;
            }
            return 0;
        }
        else
        {
            TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessCmd_MoveCity:not wild and action need updt [uid=%u sid=%u center_pos=%u] [old_city=%u new_city=%u] [step_3] [seq=%u]",
                pstSession->m_stUserInfo.m_tbPlayer.m_nUid,
                pstSession->m_stReqParam.m_udwSvrId,
                pstSession->m_tbTmpMap.m_nId,
                udwOldCityId,
                udwNewCityId,
                pstSession->m_udwSeqNo));
        }
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__5)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;

        TUINT32 udwRawCityId = pstSession->m_stReqParam.m_udwCityId;
        TUINT32 udwNewCityId = pstSession->m_tbTmpMap.m_nId;

        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessCmd_MoveCity:reset old map info [uid=%u sid=%u center_pos=%u] [old_city=%u new_city=%u] [step_4] [seq=%u]",
            pstSession->m_stUserInfo.m_tbPlayer.m_nUid,
            pstSession->m_stReqParam.m_udwSvrId,
            pstSession->m_tbTmpMap.m_nId,
            udwRawCityId,
            udwNewCityId,
            pstSession->m_udwSeqNo));

        // 更新成功之后设置新city数据
        pstSession->m_stReqParam.m_udwCityId = udwNewCityId;

        // 更新city的id
        pstCity->m_stTblData.Set_Pos(udwNewCityId);

        // 更新player数据
        pstUser->m_tbPlayer.Set_Cid(udwNewCityId);

        //禁止 after更新
        pstUser->m_udwWildNum = 0;

        // use item
        if (pstSession->m_stReqParam.m_udwCommandID == EN_CLIENT_REQ_COMMAND__RANDOM_MOVE_CITY)
        {
            CItemBase::CostItem(&pstUser->m_tbBackpack, EN_ITEM_ID__PORTAL_OF_REFUGE);
        }
        else
        {
            if (udwGemCost > 0)
            {
                CPlayerBase::CostGem(pstUser, udwGemCost);
            }
            else
            {
                CItemBase::CostItem(&pstUser->m_tbBackpack, EN_ITEM_ID__PORTAL_OF_ORDER);
            }
        }
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessCmd_MoveCity:move city succeed [uid=%u sid=%u center_pos=%u] [new_pos=%u old_pos=%u] [step_4] [seq=%u]",
            pstSession->m_stUserInfo.m_tbPlayer.m_nUid,
            pstSession->m_stReqParam.m_udwSvrId,
            udwNewCityId,
            udwRawCityId,
            pstSession->m_udwSeqNo));
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessItem::ProcessCmd_OpenAllChest(SSession *pstSession, TBOOL& bNeedResponse)
{
    TINT32 dwRetCode = 0;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;

    TUINT32 udwOpenType = atoi(pstSession->m_stReqParam.m_szKey[0]); //0: use item 1: use gem
    TUINT32 udwChestId = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TUINT32 udwItemId = atoi(pstSession->m_stReqParam.m_szKey[2]);
    TUINT32 udwChestNum = atoi(pstSession->m_stReqParam.m_szKey[3]);
    TUINT32 udwGemCost = atoi(pstSession->m_stReqParam.m_szKey[4]);

    if(!CGameInfo::GetInstance()->m_oJsonRoot["game_item"].isMember(CCommonFunc::NumToString(udwChestId)))
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_OpenAllChest: not such chest id[chest_id=%u] [seq=%u]", udwChestId, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }
    TUINT32 udwItemCategry = CGameInfo::GetInstance()->m_oJsonRoot["game_item"][CCommonFunc::NumToString(udwChestId)]["a1"].asUInt();
    if(EN_ITEM_CATEGORY__CHEST != udwItemCategry)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_OpenAllChest: id not chest [chest_id=%u] [seq=%u]", udwChestId, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -2;
    }

    if (udwOpenType == 0)
    {
        if (FALSE == CItemBase::HasEnoughItem(&pstUser->m_tbBackpack, udwItemId))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ITEM_NOT_ENOUGH;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_OpenAllChest: item not enough [item_id=%u] [seq=%u]", udwItemId, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }
    }
    else if (udwOpenType == 1 && udwGemCost != 0)
    {
        if (FALSE == CPlayerBase::HasEnoughGem(&pstUser->m_tbLogin, udwGemCost))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GEM_LACK;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_OpenAllChest: gem not enough [gem_cur=%ld] [gem_cost=%u] [seq=%u]", 
                pstUser->m_tbLogin.m_nGem, udwGemCost, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }
    }
    else
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_OpenAllChest: open type error type[%u] gem_cost[%u] [seq=%u]", 
            udwOpenType, udwGemCost, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -2;
    }
    
    if (FALSE == CItemBase::HasEnoughItem(&pstUser->m_tbBackpack, udwChestId, udwChestNum))
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ITEM_NOT_ENOUGH;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_OpenAllChest: chest not enough [chest_id=%u num=%u] [seq=%u]", udwChestId, udwChestNum, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -4;
    }
    
    //检测是否箱子
    CGameInfo *poGameInfo = CGameInfo::GetInstance();
    if(!poGameInfo->m_oJsonRoot["game_chest"].isMember(CCommonFunc::NumToString(udwChestId)))
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_OpenAllChest: not such chest  [chest_id=%u] [seq=%u]", udwChestId, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -5;
    }


    // 水晶箱子和材料箱子除外使用物品逻辑
    if((EN_ITEM_FUNC_TYPE_CRYSTAL != CItemBase::GetItemFuncFromGameJson(udwChestId)
       && EN_ITEM_FUNC_TYPE_MATERIALS != CItemBase::GetItemFuncFromGameJson(udwChestId)
       && EN_ITEM_FUNC_TYPE_CHALLENGER != CItemBase::GetItemFuncFromGameJson(udwChestId)
       && EN_ITEM_FUNC_TYPE_MONSTER != CItemBase::GetItemFuncFromGameJson(udwChestId))
       || CItemLogic::IsChestLottery(udwChestId))
    {
        SSpGlobalRes stGlobalRes;
        stGlobalRes.Reset();
        const Json::Value &oChestReward = poGameInfo->m_oJsonRoot["game_chest"][CCommonFunc::NumToString(udwChestId)]["a1"];
        TINT32 dwRewardType = poGameInfo->m_oJsonRoot["game_chest"][CCommonFunc::NumToString(udwChestId)]["a0"].asInt();

        for(TUINT32 udwIdx = 0; udwIdx < udwChestNum;++udwIdx)
        {
            if (CItemLogic::IsChestLottery(udwChestId))
            {
                SSpGlobalRes stGlobalResOne;
                stGlobalResOne.Reset();
                dwRetCode = CItemLogic::GetChestLottery(pstUser, udwChestId, &stGlobalResOne);
                CItemLogic::AppendGlobalRes(&stGlobalRes, &stGlobalResOne);
            }
            else
            {
                dwRetCode = CGlobalResLogic::GetSpGlobalResInfo(oChestReward, dwRewardType, &stGlobalRes);
            }
            if(dwRetCode != 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_OpenAllChest: get reward fail  [chest_id=%u ret=%d] [seq=%u]", udwChestId, dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
                return -6;
                
            }
        }
        //获取奖励
        dwRetCode = CGlobalResLogic::AddSpGlobalRes(pstUser, pstCity, &stGlobalRes);
        if(dwRetCode != 0)
        {
            TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("ProcessCmd_OpenAllChest: GetChest: take reward fail [uid=%ld chestId=%u] [ret=%d]", pstUser->m_tbPlayer.Get_Uid(), udwChestId, dwRetCode));
            return dwRetCode;
        }

        // 抽奖记录
        pstUser->m_udwLotteryChestItemId = udwChestId;
        pstUser->m_stRewardWindow = stGlobalRes;
        pstUser->m_udwLotteryChestItemNum = udwChestNum;

        Json::Value jTmp = Json::Value(Json::objectValue);
        jTmp["chest"] = Json::Value(Json::objectValue);
        jTmp["chest"]["id"] = udwChestId;
        jTmp["chest"]["num"] = udwChestNum;
        
        CCommonBase::AddRewardWindow(pstUser, pstUser->m_tbPlayer.m_nUid, EN_REWARD_WIMDOW_TYPE_CHEST, EN_REWARD_WINDOW_GET_TYPE_OPEN_CHEST,
                                     0, &stGlobalRes, FALSE, jTmp);

        //3 cost item 
        if (udwOpenType == 0)
        {
            CItemBase::CostItem(&pstUser->m_tbBackpack, udwItemId);
        }
        else
        {
            CPlayerBase::CostGem(pstUser, udwGemCost);
        }
        CItemBase::CostItem(&pstUser->m_tbBackpack, udwChestId, udwChestNum);
        return 0;
    }
    // 水晶箱子和材料箱子除外使用物品逻辑
    else
    {
        if(EN_COMMAND_STEP__INIT == pstSession->m_udwCommandStep)
        {
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__DATA_CENTER;
            pstSession->ResetDataCenterReq();


            DataCenterReqInfo* pstReq = new DataCenterReqInfo;
            pstReq->m_udwType = EN_REFRESH_DATA_TYPE__CHEST;
            Json::Value rDataReqJson = Json::Value(Json::objectValue);

            rDataReqJson["sid"] = pstUser->m_tbLogin.m_nSid;
            rDataReqJson["uid"] = pstUser->m_tbPlayer.m_nUid;
            rDataReqJson["castle_lv"] = CCityBase::GetBuildingLevelByFuncType(&pstUser->m_stCityInfo, EN_BUILDING_TYPE__CASTLE);
            rDataReqJson["request"] = Json::Value(Json::objectValue);
            rDataReqJson["request"]["chest_id"] = udwChestId;
            rDataReqJson["request"]["open_num"] = udwChestNum;
            
            Json::FastWriter rEventWriter;
            pstReq->m_sReqContent = rEventWriter.write(rDataReqJson);
            pstSession->m_vecDataCenterReq.push_back(pstReq);

            TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_OpenAllChest: data center req: [json=%s] [type=%u] [uid=%ld] [seq=%u]", \
                                                    pstReq->m_sReqContent.c_str(), \
                                                    pstReq->m_udwType, \
                                                    pstUser->m_tbPlayer.m_nUid, \
                                                    pstSession->m_udwSeqNo));
            bNeedResponse = TRUE;
            TINT32 dwRetCode = CBaseProcedure::SendDataCenterRequest(pstSession, EN_SERVICE_TYPE_DATA_CENTER_REQ);
            if (dwRetCode == 0)
            {
                return 0;
            }
            else
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_DATA_CENTER_REQ_ERR;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_OpenAllChest: send request to data center failed. [json=%s] [ret=%d] [seq=%u]", \
                                                        pstReq->m_sReqContent.c_str(), \
                                                        dwRetCode, \
                                                        pstUser->m_udwBSeqNo));
                return -5;
            }            
        }

        if(EN_COMMAND_STEP__1 == pstSession->m_udwCommandStep)
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
                    if(EN_REFRESH_DATA_TYPE__CHEST == pstDataCenterRsp->m_udwType)
                    {
                        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_OpenAllChest: data center get rsp: [json=%s] [uid=%ld][seq=%u]",
                                                                pstDataCenterRsp->m_sRspJson.c_str(), \
                                                                pstUser->m_tbPlayer.m_nUid, \
                                                                pstUser->m_udwBSeqNo));

                        if (FALSE == jsonReader.parse(pstDataCenterRsp->m_sRspJson, oRspDataJson))
                        {
                            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_DATA_CENTER_PACKAGE_ERR;
                            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ItemUse: prase rsp from data center failed. [seq=%u]", \
                                                                    pstUser->m_udwBSeqNo));
                            return -6;
                        }
                        TINT32 dwRetCode = stRefreshData.m_stChestRsp.setVal(oRspDataJson);
                        if (0 != dwRetCode)
                        {
                            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__DATA_CENTER_PACKAGE_FORMAT_ERR;
                            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_OpenAllChest: response data format error. [ret=%d][seq=%u]", \
                                                                    dwRetCode, \
                                                                    pstUser->m_udwBSeqNo));
                            return -7;
                        }
                        break;
                    }
                }


                if(MAX_SP_REWARD_ITEM_NUM < stRefreshData.m_stChestRsp.m_vecReward.size()
                    || 0 >= stRefreshData.m_stChestRsp.m_vecReward.size())
                {
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_OpenAllChest: chest reward size is over. [size=%ld] [seq=%u]", \
                                                            stRefreshData.m_stChestRsp.m_vecReward.size(), \
                                                            pstUser->m_udwBSeqNo));
                    return -8;
                }


                SSpGlobalRes stGlobalRes;
                stGlobalRes.Reset();

                for(TINT32 dwIdx = 0; dwIdx < stRefreshData.m_stChestRsp.m_vecReward.size(); ++dwIdx)
                {
                    SOneGlobalRes *pstOneGlobalRes = stRefreshData.m_stChestRsp.m_vecReward[dwIdx];
                    TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_OpenAllChest: chest reward [type=%ld] [id=%ld] [num=%ld] [seq=%u]", \
                                                            pstOneGlobalRes->ddwType, \
                                                            pstOneGlobalRes->ddwId, \
                                                            pstOneGlobalRes->ddwNum, \
                                                            pstUser->m_udwBSeqNo));

                
                    stGlobalRes.aRewardList[dwIdx].udwType = pstOneGlobalRes->ddwType;
                    stGlobalRes.aRewardList[dwIdx].udwId = pstOneGlobalRes->ddwId;
                    stGlobalRes.aRewardList[dwIdx].udwNum = pstOneGlobalRes->ddwNum;
                    ++stGlobalRes.udwTotalNum;
                    dwRetCode = CGlobalResLogic::AddGlobalRes(pstUser, pstCity, pstOneGlobalRes->ddwType, pstOneGlobalRes->ddwId, pstOneGlobalRes->ddwNum);
                    if (dwRetCode > 0)
                    {
                        pstSession->m_stCommonResInfo.m_dwRetCode = dwRetCode;
                        return -1;
                    }
                    else if (dwRetCode < 0)
                    {
                        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
                        return -2;
                    }
                }
                
                //3 cost item 
                if (udwOpenType == 0)
                {
                    CItemBase::CostItem(&pstUser->m_tbBackpack, udwItemId);
                }
                else
                {
                    CPlayerBase::CostGem(pstUser, udwGemCost);
                }
                CItemBase::CostItem(&pstUser->m_tbBackpack, udwChestId, udwChestNum);
                
                pstUser->m_udwLotteryChestItemId = udwChestId;
                pstUser->m_udwLotteryChestItemNum = 1;
                pstUser->m_stRewardWindow = stGlobalRes;


                Json::Value jTmp = Json::Value(Json::objectValue);
                jTmp["chest"] = Json::Value(Json::objectValue);
                jTmp["chest"]["id"] = udwChestId;
                jTmp["chest"]["num"] = udwChestNum;
                
                CCommonBase::AddRewardWindow(pstUser, pstUser->m_tbPlayer.m_nUid, EN_REWARD_WIMDOW_TYPE_CHEST, EN_REWARD_WINDOW_GET_TYPE_OPEN_CHEST,
                                             0, &stGlobalRes, FALSE, jTmp);

                
                pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
                return 0;
                
            }
            else
            {
                pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
                return 0;
            }
        }
        return 0;
    }

    
}

TINT32 CProcessItem::ProcessCmd_MoveCityPrepare( SSession *pstSession, TBOOL &bNeedResponse )
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbMarch_action *ptbMarch = NULL;
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    TINT32 dwRetCode = 0;

    // 1. 获取输入参数
    TUINT32 udwItem = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TUINT32 udwGemCost = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TUINT32 udwOldPos = atoi(pstSession->m_stReqParam.m_szKey[2]);

    // 2. check condition
    dwRetCode = CItemLogic::CanMoveCity(pstUser);
    if (dwRetCode != 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = dwRetCode;
        return -1;
    }

    //wave@20131127:添加物品检查保护
    if (CItemBase::HasEnoughItem(&pstUser->m_tbBackpack, udwItem) == FALSE)
    {
        if (CPlayerBase::HasEnoughGem(&pstUser->m_tbLogin, udwGemCost) == FALSE)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GEM_LACK;
            return -1;
        }
    }

    for (TUINT32 idx = 0; idx < pstUser->m_udwActionNum; idx++)
    {
        TbAction *ptbAction = &pstUser->m_atbAction[idx];
        if (ptbAction->m_nMclass == EN_ACTION_SEC_CLASS__ATTACK_MOVE
            && ptbAction->m_bParam[0].m_stAttackMove.m_ddwSpecailMoveFlag == 1)
        {
            //pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__BE_KICKING_OUT;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MoveCityPrepare: be kicking out [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            pstUser->m_dwTimeForMoveCityPrepare = 5;
            return 0;
        }
    }

    if (udwOldPos != 0 && udwOldPos != pstUser->m_stCityInfo.m_stTblData.m_nPos)
    {
        //pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__HAS_KICKED_OUT;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MoveCityPrepare: has kicked out [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
        pstUser->m_dwTimeForMoveCityPrepare = 5;
        return 0;
    }

    //3. 执行敌对到达
    TBOOL bIsAttacked = FALSE;
    TUINT32 udwCount = 0;
    for(TINT32 idx = 0; idx < pstUser->m_udwPassiveMarchNum; idx++)
    {
        ptbMarch = &pstUser->m_atbPassiveMarch[idx];
        if(ptbMarch->m_nTpos == pstUser->m_stCityInfo.m_stTblData.m_nPos)
        {
            if(ptbMarch->m_nMclass == EN_ACTION_MAIN_CLASS__MARCH 
                && (ptbMarch->m_nStatus == EN_MARCH_STATUS__MARCHING || ptbMarch->m_nStatus == EN_MARCH_STATUS__FIGHTING))
            {
                if(ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR
                    || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__ATTACK)
                {
                    bIsAttacked = TRUE;
                    if (ptbMarch->m_nEtime > udwCurTime - 60)
                    {
                        ptbMarch->Set_Etime(udwCurTime - 60); //保证能fighting完...
                        pstUser->m_aucPassiveMarchFlag[idx] = EN_TABLE_UPDT_FLAG__CHANGE;
                    }
                    udwCount++;
                    if (udwCount >= 10)
                    {
                        break;
                    }
                }
            }
        }
    }

    if (bIsAttacked)
    {
        pstUser->m_dwTimeForMoveCityPrepare = 5;
    }

    return 0;
}
