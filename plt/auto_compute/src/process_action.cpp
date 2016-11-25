#include "process_action.h"
#include "common_base.h"
#include "sendmessage_base.h"
#include "aws_request.h"
#include "aws_response.h"
#include "war_process.h"
#include "wild_info.h"
#include "common_func.h"
#include "process_report.h"
#include "globalres_logic.h"
#include "backpack_info.h"
#include "msg_base.h"
#include "activities_logic.h"
#include "document.h"
#include "common_logic.h"
#include "buffer_base.h"
#include "quest_logic.h"
#include "conf_base.h"
#include "db_struct_define.h"
#include "db_request.h"
#include "map_logic.h"
#include "city_base.h"
#include "map_base.h"
#include "tool_base.h"
#include "player_base.h"
#include "backpack_logic.h"
#include "std_header.h"
#include "global_serv.h"
#include "base/os/wtsesocket.h"
#include "report_svr_request.h"


TINT32 CProcessAction::SendDataCenterRequest(SSession *pstSession, TUINT16 uwReqServiceType)
{
    TUCHAR *pszPack = NULL;
    TUINT32 udwPackLen = 0;
    CDownMgr *pobjDownMgr = CDownMgr::Instance();
    LTasksGroup	stTasksGroup;
    vector<DataCenterReqInfo*>& vecReq = pstSession->m_vecDataCenterReq;

    if (vecReq.size() == 0)
    {
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("SendDataCenterRequest: not translate req [seq=%u]", \
                                                pstSession->m_udwSeqNo));
        return -1;
    }


    // 1. 获取下游——对于一个session来说，每次只需获取一次
    if (pstSession->m_bDataCenterProxyExist == FALSE)
    {
        pstSession->m_pstDataCenterProxyNode = NULL;
        if (S_OK == pobjDownMgr->zk_GetNode(DOWN_NODE_TYPE__DATA_CENTER_PROXY, &pstSession->m_pstDataCenterProxyNode))
        {
            pstSession->m_bDataCenterProxyExist = TRUE;
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("SendDataCenterRequest: Get DataCenterProxy node succ [seq=%u]", \
                                                    pstSession->m_udwSeqNo));
        }
        else
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("SendDataCenterRequest: Get DataCenterProxy node fail [seq=%u]", \
                                                    pstSession->m_udwSeqNo));
            return -2;
        }
    }

    // 2. 封装请求、打包、设置task
    for (TUINT32 udwIdx = 0; udwIdx < vecReq.size(); ++udwIdx)
    {
        srand(time(0));
        DataCenterReqInfo *pstDataCenterReq = vecReq[udwIdx];
        CBaseProtocolPack* pPack = pstSession->m_ppPackTool[udwIdx];
        pPack->ResetContent();
        pPack->SetServiceType(uwReqServiceType);
        TUINT32 udwHandleSeq = CGlobalServ::GenerateHsReqSeq();
        pPack->SetSeq(udwHandleSeq);
        pPack->SetKey(EN_DATA_CENTER__REQ_SEQ, pstSession->m_udwSeqNo);
        pPack->SetKey(EN_DATA_CENTER__REQ_TYPE, pstDataCenterReq->m_udwType);
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("SendDataCenterRequest: send data: [handleseq=%u][seq=%u][type=%u] [seq=%u]", \
                                                udwHandleSeq, \
                                                pstSession->m_udwSeqNo, \
                                                pstDataCenterReq->m_udwType, \
                                                pstSession->m_udwSeqNo));
        pPack->SetKey(EN_DATA_CENTER__REQ_JSON, (unsigned char*)pstDataCenterReq->m_sReqContent.c_str(), pstDataCenterReq->m_sReqContent.size());
        ///打包请求到下游
        pPack->GetPackage(&pszPack, &udwPackLen);

        if (NULL == pstSession->m_pstDataCenterProxyNode->m_stDownHandle.handle)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("SendDataCenterRequest: handle [task_id=%u] [handlevalue=0x%p] [seq=%u]", \
                                                    udwIdx, \
                                                    pstSession->m_pstDataCenterProxyNode->m_stDownHandle.handle, \
                                                    pstSession->m_udwSeqNo));
            return -3;
        }

        stTasksGroup.m_Tasks[udwIdx].SetConnSession(pstSession->m_pstDataCenterProxyNode->m_stDownHandle);
        stTasksGroup.m_Tasks[udwIdx].SetSendData(pszPack, udwPackLen);
        stTasksGroup.m_Tasks[udwIdx].SetNeedResponse(1);
    }

    // c. 设置task
    stTasksGroup.m_UserData1.ptr = pstSession;
    stTasksGroup.m_UserData2.u32 = pstSession->m_udwSeqNo;//wave@20160104:为解决主程序已处理，但后续有回包返回时的情况

    stTasksGroup.SetValidTasks(vecReq.size());
    stTasksGroup.m_uTaskTimeOutVal = DOWN_SEARCH_NODE_TIMEOUT_MS;

    pstSession->ResetDataCenterReq();

    // 3. 发送数据
    pstSession->m_bProcessing = FALSE;
    if (!pstSession->m_poLongConn->SendData(&stTasksGroup))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("SendDataCenterRequest: send data failed! [seq=%u]", \
                                                pstSession->m_udwSeqNo));
        return -4;
    }

    return 0;
}



TINT32 CProcessAction::ProcessUidAction(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;

    switch(pstSession->m_stReqAction.m_nMclass)
    {
    case EN_ACTION_MAIN_CLASS__ITEM:
        dwRetCode = CProcessAction::ProcessActionItem(pstSession, bNeedResponse);
        break;
    case EN_ACTION_MAIN_TASK_ATTACK_MOVE:
        dwRetCode = CProcessAction::ProcessActionAttackMove(pstSession,bNeedResponse);
        break;
    case EN_ACTION_MAIN_CLASS__AU_PRESSURE_MEASURE:
        dwRetCode = CProcessAction::ProcessAuPress(pstSession, bNeedResponse);
        break;
    default:
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ERROR_COMMAND;
        dwRetCode = -1;
        break;
    }
    TSE_LOG_INFO(pstSession->m_poServLog, ("process_by_cmd: step[%u] main=%ld,sec=%ld,status=%ld,ret=%d, bNeedResponse[%d] [seq=%u]",
        pstSession->m_udwProcessSeq, pstSession->m_stReqAction.m_nMclass, pstSession->m_stReqAction.m_nSclass,
        pstSession->m_stReqAction.m_nStatus, dwRetCode, bNeedResponse, pstSession->m_udwSeqNo));

    return dwRetCode;
}

TINT32 CProcessAction::ProcessAidAction(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;

    switch(pstSession->m_stReqAlAction.m_nMclass)
    {
    case EN_ACTION_MAIN_CLASS__BUILDING:
        dwRetCode = CProcessAction::ProcessActionBuilding(pstSession, bNeedResponse);
        break;
    case EN_ACTION_MAIN_CLASS__EQUIP:
        dwRetCode = CProcessAction::ProcessActionEquip(pstSession, bNeedResponse);
        break;
    case EN_ACTION_MAIN_CLASS__TRAIN_NEW:
        dwRetCode = CProcessAction::ProcessActionTrainNew(pstSession, bNeedResponse);
        break;
    case EN_ACTION_MAIN_CLASS__DRAGON:
        dwRetCode = CProcessAction::ProcessActionDragon(pstSession, bNeedResponse);
        break;
    default:
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ERROR_COMMAND;
        dwRetCode = -1;
        break;
    }
    TSE_LOG_INFO(pstSession->m_poServLog, ("process_by_cmd: step[%u] main=%ld,sec=%ld,status=%ld,ret=%d, bNeedResponse[%d] [seq=%u]",
        pstSession->m_udwProcessSeq, pstSession->m_stReqAlAction.m_nMclass, pstSession->m_stReqAlAction.m_nSclass,
        pstSession->m_stReqAlAction.m_nStatus, dwRetCode, bNeedResponse, pstSession->m_udwSeqNo));

    return dwRetCode;
}

TINT32 CProcessAction::ProcessMarchAction(SSession *pstSession, TBOOL &bNeedResponse)
{
    switch(pstSession->m_stReqMarch.m_nMclass)
    {
    case EN_ACTION_MAIN_CLASS__MARCH:
        return CProcessAction::ProcessMarch(pstSession, bNeedResponse);
        break;
    case EN_ACTION_MAIN_CLASS__TIMER:
        return CProcessAction::ProcessTimer(pstSession, bNeedResponse);
        break;
    default:
        break;
    }
    return 0;
}

TINT32 CProcessAction::ProcessMarch(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;

    // process
    if(pstSession->m_stReqMarch.m_nStatus == EN_MARCH_STATUS__RETURNING)
    {
        dwRetCode = ProcessMarchReturn(pstSession);
    }
    else if(pstSession->m_stReqMarch.m_nStatus == EN_MARCH_STATUS__MARCHING)
    {
        switch(pstSession->m_stReqMarch.m_nSclass)
        {
        case EN_ACTION_SEC_CLASS__TRANSPORT:
            dwRetCode = ProcessMarchTransport(pstSession);
            break;
        case EN_ACTION_SEC_CLASS__REINFORCE_NORMAL:
            dwRetCode = ProcessMarchReinForce(pstSession);
            break;
        case EN_ACTION_SEC_CLASS__ATTACK:
            dwRetCode = ProcessMarchAttack(pstSession);
            break;
        case EN_ACTION_SEC_CLASS__OCCUPY:
            dwRetCode = ProcessMarchOccupy(pstSession);
            break;
        case EN_ACTION_SEC_CLASS__SCOUT:
            dwRetCode = ProcessMarchScout(pstSession);
            break;
        case EN_ACTION_SEC_CLASS__DRAGON_ATTACK:
            dwRetCode = ProcessMarchDragonAttack(pstSession);
            break;
        case EN_ACTION_SEC_CLASS__RALLY_REINFORCE:
            dwRetCode = ProcessMarchRallyReinforce(pstSession);
            break;
        case EN_ACTION_SEC_CLASS__RALLY_WAR:
            dwRetCode = ProcessMarchRallyAttack(pstSession);
            break;
        case EN_ACTION_SEC_CLASS__CAMP:
            //dwRetCode = ProcessMarchCamp(pstSession);
            dwRetCode = ProcessMarchCampSetupCampPeace(pstSession);
            break;
        case EN_ACTION_SEC_CLASS__ATTACK_IDOL:
            dwRetCode = ProcessMarchIdolAttack(pstSession);
            break;
        case EN_ACTION_SEC_CLASS__ASSIGN_THRONE:
            dwRetCode = ProcessMarchThroneAssign(pstSession);
            break;
        case EN_ACTION_SEC_CLASS__REINFORCE_THRONE:
            dwRetCode = ProcessMarchThroneReinforce(pstSession);
            break;
        case EN_ACTION_SEC_CLASS__ATTACK_THRONE:
            dwRetCode = ProcessMarchThroneAttack(pstSession);
            break;
        case EN_ACTION_SEC_CLASS__RALLY_WAR_THRONE:
            dwRetCode = ProcessMarchThroneRallyWar(pstSession);
            break;
        default:
            return -1;
            assert(0);
            break;
        }
    }
    else if(pstSession->m_stReqMarch.m_nStatus == EN_MARCH_STATUS__FIGHTING)
    {
        switch(pstSession->m_stReqMarch.m_nSclass)
        {
        case EN_ACTION_SEC_CLASS__ATTACK:
            dwRetCode = ProcessMarchFighting(pstSession);
            break;
        case EN_ACTION_SEC_CLASS__DRAGON_ATTACK:
            dwRetCode = ProcessMarchDragonFighting(pstSession);
            break;
        case EN_ACTION_SEC_CLASS__RALLY_WAR:
            dwRetCode = ProcessMarchRallyFighting(pstSession);
            break;
        case EN_ACTION_SEC_CLASS__ATTACK_IDOL:
            dwRetCode = ProcessMarchIdolFighting(pstSession);
            break;
        case EN_ACTION_SEC_CLASS__ATTACK_THRONE:
            dwRetCode = ProcessMarchThroneFighting(pstSession);
            break;
        case EN_ACTION_SEC_CLASS__RALLY_WAR_THRONE:
            dwRetCode = ProcessMarchThroneRallyFighting(pstSession);
            break;
        default:
            return -1;
            assert(0);
            break;
        }
    }
    else if(pstSession->m_stReqMarch.m_nStatus == EN_MARCH_STATUS__LOADING)
    {
        switch(pstSession->m_stReqMarch.m_nSclass)
        {
        case EN_ACTION_SEC_CLASS__OCCUPY:
            dwRetCode = ProcessMarchLoadingRes(pstSession);
            break;
        default:
            return -1;
            assert(0);
            break;
        }
    }
    else if(pstSession->m_stReqMarch.m_nStatus == EN_MARCH_STATUS__SETUP_CAMP)
    {
        switch (pstSession->m_stReqMarch.m_nSclass)
        {
        case EN_ACTION_SEC_CLASS__REINFORCE_NORMAL:
            dwRetCode = ProcessMarchSetupCamp(pstSession);
            break;
        default:
            return -1;
            assert(0);
            break;
        }
    }
    else if (pstSession->m_stReqMarch.m_nStatus == EN_MARCH_STATUS__CAMPING_WITH_PEACETIME)
    {
        switch (pstSession->m_stReqMarch.m_nSclass)
        {
        case EN_ACTION_SEC_CLASS__CAMP:
            dwRetCode = ProcessMarchCampSetupCampNormal(pstSession);
            break;
        default:
            return -1;
            assert(0);
            break;
        }
    }
    else if (pstSession->m_stReqMarch.m_nStatus == EN_MARCH_STATUS__CAMPING_NORMAL)
    {
        switch (pstSession->m_stReqMarch.m_nSclass)
        {
        case EN_ACTION_SEC_CLASS__CAMP:
            dwRetCode = ProcessMarchCampSetupCampReturn(pstSession);
            break;
        default:
            return -1;
            assert(0);
            break;
        }
    }
    else if(pstSession->m_stReqMarch.m_nStatus == EN_MARCH_STATUS__PRE_LOADING)
    {
        switch(pstSession->m_stReqMarch.m_nSclass)
        {
        case EN_ACTION_SEC_CLASS__OCCUPY:
            dwRetCode = ProcessMarchPreLoading(pstSession);
            break;
        default:
            return -1;
            assert(0);
            break;
        }
    }
    else if(pstSession->m_stReqMarch.m_nStatus == EN_MARCH_STATUS__UN_LOADING)
    {
        switch(pstSession->m_stReqMarch.m_nSclass)
        {
        case EN_ACTION_SEC_CLASS__TRANSPORT:
            dwRetCode = ProcessMarchUnLoading(pstSession);
            break;
        default:
            return -1;
            assert(0);
            break;
        }
    }
    else if(pstSession->m_stReqMarch.m_nStatus == EN_MARCH_STATUS__PREPARING
        || pstSession->m_stReqMarch.m_nStatus == EN_MARCH_STATUS__WAITING)
    {
        switch(pstSession->m_stReqMarch.m_nSclass)
        {
        case EN_ACTION_SEC_CLASS__RALLY_WAR:
            dwRetCode = ProcessMarchRallyPreparing(pstSession);
            break;
        case EN_ACTION_SEC_CLASS__RALLY_WAR_THRONE:
            dwRetCode = ProcessMarchRallyThronePreparing(pstSession);
            break;
        default:
            return -2;
            break;
        }
    }
    else if(pstSession->m_stReqMarch.m_nStatus == EN_MARCH_STATUS__DELING
         || pstSession->m_stReqMarch.m_nStatus == EN_MARCH_STATUS__DELING_ON_FLY)
    {
        dwRetCode = ProcessMarchRallyDeling(pstSession);
    }
    else
    {
        return -3;
        assert(0);
    }

    TSE_LOG_INFO(pstSession->m_poServLog, ("process_by_cmd: step[%u] main=%ld,sec=%ld,status=%ld,ret=%d, bNeedResponse[%d] [seq=%u]",
        pstSession->m_udwProcessSeq, pstSession->m_stReqMarch.m_nMclass, pstSession->m_stReqMarch.m_nSclass,
        pstSession->m_stReqMarch.m_nStatus, dwRetCode, bNeedResponse, pstSession->m_udwSeqNo));
    return dwRetCode;
}

TINT32 CProcessAction::ProcessActionBuilding(SSession *pstSession, TBOOL &bNeedResponse)
{
    SUserInfo *pstSUser = &pstSession->m_stSourceUser;
    TbPlayer *pstPlayer = &pstSession->m_stSourceUser.m_tbPlayer;
    //SCityInfo *pstCity = CCityBase::GetCityInfo(pstSession->m_stSourceUser.m_astCityInfo, pstSession->m_stSourceUser.m_udwCityNum, pstSession->m_stReqAlAction.m_nScid);
	SCityInfo *pstCity = &pstSession->m_stSourceUser.m_stCityInfo;
	SActionBuildingParam *pstParam = &pstSession->m_stReqAlAction.m_bParam[0].m_stBuilding;

    if(pstCity == NULL)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessActionBuilding:null city [seq=%u]", pstSession->m_stSourceUser.m_udwBSeqNo));
        return -1;
    }
    // 1. 根据action命令字分别进行计算
    switch(pstSession->m_stReqAlAction.m_nSclass)
    {
    case EN_ACTION_SEC_CLASS__BUILDING_UPGRADE:
        if(TRUE)
        {
            SCityBuildingNode* pstNode = CCityBase::GetBuildingAtPos(&pstCity->m_stTblData, pstParam->m_ddwPos);
            if(pstNode)
            {
                pstNode->m_ddwType = pstParam->m_ddwType;
                pstNode->m_ddwLevel = pstParam->m_ddwTargetLevel;
                pstCity->m_stTblData.SetFlag(TbCITY_FIELD_BUILDING);
            }
            else
            {
                CCityBase::AddBuilding(pstParam->m_ddwPos, pstParam->m_ddwType, pstParam->m_ddwTargetLevel, pstCity->m_stTblData);
            }
        }
        // 主城的等级信息
        if(CCityBase::GetBuildingFuncType(pstParam->m_ddwType) == EN_BUILDING_TYPE__CASTLE)
        {
            if (CMapLogic::IfPlayerCity(pstSUser, &pstSession->m_stMapItem))
            {
                pstSession->m_stMapItem.Set_Sid(pstPlayer->m_nSid);
                pstSession->m_stMapItem.Set_Id(pstCity->m_stTblData.m_nPos);
                pstSession->m_stMapItem.Set_Level(pstParam->m_ddwTargetLevel);
            }

            // 主城5级取消新手保护
            TUINT32 udwBreakProtectLv = CGameInfo::GetInstance()->m_oJsonRoot["game_basic"][29U].asUInt();
            //TUINT32 udwBreadProtectAge = CGameInfo::GetInstance()->m_oJsonRoot["game_basic"][54U].asUInt();
            if(pstParam->m_ddwTargetLevel >= udwBreakProtectLv /* && pstPlayer->m_nAge >= udwBreadProtectAge*/)
            {
                if(pstPlayer->m_nStatus & EN_CITY_STATUS__NEW_PROTECTION)
                {
                    pstPlayer->Set_Status(pstPlayer->m_nStatus & (~EN_CITY_STATUS__NEW_PROTECTION));
                    pstPlayer->Set_Status(pstPlayer->m_nStatus | EN_CITY_STATUS__NEW_PROTECTION_CANCEL);

                    pstSession->m_stMapItem.Set_Status(pstPlayer->m_nStatus);

                    // 删除action
                    TbAction* ptbAction = CActionBase::GetActionByBufferId(pstSession->m_stSourceUser.m_atbAction, pstSession->m_stSourceUser.m_udwActionNum, EN_BUFFER_INFO_PEACE_TIME);
                    if(ptbAction != NULL)
                    {
                        TINT32 dwActionIdx = CActionBase::GetActionIndex(pstSession->m_stSourceUser.m_atbAction, pstSession->m_stSourceUser.m_udwActionNum, ptbAction->m_nId);
                        pstSession->m_stSourceUser.m_aucActionFlag[dwActionIdx] = EN_TABLE_UPDT_FLAG__DEL;
                    }
                }
            }

            if(pstParam->m_ddwTargetLevel >= CCityBase::GetBuildingLimitLv(pstParam->m_ddwType))
            {
                SNoticInfo stNoticInfo;
                stNoticInfo.Reset();
                stNoticInfo.SetValue(EN_NOTI_ID__NEW_AGE,
                    "", "",
                    0, 0,
                    0, 0,
                    0, "", 0);
                CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), pstSession->m_stSourceUser.m_tbPlayer.m_nSid, pstSession->m_stSourceUser.m_tbPlayer.m_nUid, stNoticInfo);
            }
        }
        else if(CCityBase::GetBuildingFuncType(pstParam->m_ddwType) == EN_BUILDING_TYPE__EMBASSY)
        {
            if (CMapLogic::IfPlayerCity(pstSUser, &pstSession->m_stMapItem))
            {
                pstSession->m_stMapItem.Set_Sid(pstSession->m_udwReqSvrId);
                pstSession->m_stMapItem.Set_Id(pstCity->m_stTblData.m_nPos);
                pstSession->m_stMapItem.Set_Em_lv(pstParam->m_ddwTargetLevel);
            }
        }
        // tips
        CSendMessageBase::AddTips(&pstSession->m_stSourceUser, EN_TIPS_TYPE__BUILDING_OK, pstPlayer->m_nUid, TRUE, pstParam->m_ddwType, pstParam->m_ddwTargetLevel);

        if(pstParam->m_ddwTargetLevel == 1)
        {
            CQuestLogic::SetTaskCurValue(&pstSession->m_stSourceUser, pstCity, EN_TASK_TYPE_ING_BUILDING_NUM);
        }
        CQuestLogic::SetTaskCurValue(&pstSession->m_stSourceUser, pstCity, EN_TASK_TYPE_ING_BUILD_X_LV_BUILDING, 1, 0, pstParam->m_ddwTargetLevel);

        break;
    case EN_ACTION_SEC_CLASS__BUILDING_REMOVE:
        CCityBase::DelBuildingAtPos(&pstCity->m_stTblData, pstParam->m_ddwPos);
        //llt add,大使馆拆除,修改地图信息
        if(EN_BUILDING_TYPE__EMBASSY == CCityBase::GetBuildingFuncType(pstParam->m_ddwType))
        {
            if (CMapLogic::IfPlayerCity(pstSUser, &pstSession->m_stMapItem))
            {
                TbMap& tbTmpMap = pstSession->m_stMapItem;
                tbTmpMap.Reset();
                tbTmpMap.Set_Sid(pstSession->m_udwReqSvrId);
                tbTmpMap.Set_Id(pstCity->m_stTblData.m_nPos);
                tbTmpMap.Set_Em_lv(0);
            }
        }

        // tips
        CSendMessageBase::AddTips(&pstSession->m_stSourceUser, EN_TIPS_TYPE__REMOVE_BUILDING_OK, pstPlayer->m_nUid, TRUE, pstParam->m_ddwType, pstParam->m_ddwTargetLevel);

        break;
    case EN_ACTION_SEC_CLASS__RESEARCH_UPGRADE:
        pstSession->m_stSourceUser.m_stCityInfo.m_stTblData.m_bResearch[0].m_addwLevel[pstParam->m_ddwType] = pstParam->m_ddwTargetLevel;
        pstSession->m_stSourceUser.m_stCityInfo.m_stTblData.SetFlag(TbCITY_FIELD_RESEARCH);

        // tips
        CSendMessageBase::AddTips(&pstSession->m_stSourceUser, EN_TIPS_TYPE__RESEARCH_OK, pstPlayer->m_nUid, TRUE, pstParam->m_ddwType, pstParam->m_ddwTargetLevel);

        CQuestLogic::SetTaskCurValue(&pstSession->m_stSourceUser, pstCity, EN_TASK_TYPE_ING_RESEARCH_TIME);
        break;
    case EN_ACTION_SEC_CLASS__REMOVE_OBSTACLE:
        if(TRUE)
        {
            std::set<TINT32> posSet;
            posSet.clear();

            TUINT32 udwOldSize = CGameInfo::GetInstance()->m_oJsonRoot["game_building"][CCommonFunc::NumToString(pstParam->m_ddwType)]["a"]["a11"].asUInt();
            CCommonLogic::GetBuildingPos(pstParam->m_ddwPos, udwOldSize, posSet);
            //是否5*5障碍物
            if(posSet.size() == 5 * 5)
            {
                pstCity->m_stTblData.Set_Unlock_block(pstCity->m_stTblData.m_nUnlock_block + 1);
            }

            std::vector<TINT32> povVec;
            povVec.clear();
            for(std::set<TINT32>::iterator it = posSet.begin(); it != posSet.end(); ++it)
            {
                povVec.push_back(*it);
            }

            CCityBase::DelBuildingAtPos(&pstCity->m_stTblData, pstParam->m_ddwPos);
            const Json::Value &oBuildingTypeReward = CGameInfo::GetInstance()->m_oJsonRoot["game_building"][CCommonFunc::NumToString(pstParam->m_ddwType)]["b"]["b2"][0U];

            for(TUINT32 udwIdx = 0; udwIdx < oBuildingTypeReward.size(); ++udwIdx)
            {
                if(oBuildingTypeReward[udwIdx][0U].asUInt() == EN_GLOBALRES_TYPE_BUILDING_OBSTACLE)
                {
                    TUINT32 udwBuildingId = oBuildingTypeReward[udwIdx][1U].asUInt();
                    TUINT32 udwBuildingNum = oBuildingTypeReward[udwIdx][2U].asUInt();
                    TUINT32 udwBuildingRandNum = rand() % udwBuildingNum;
                    for(TUINT32 udwNumIdx = 0; udwNumIdx < udwBuildingRandNum; ++udwNumIdx)
                    {
                        CCommonLogic::GenObstle(&pstCity->m_stTblData, udwBuildingId, &povVec);
                    }
                }
            }
        }// tips
        CSendMessageBase::AddTips(&pstSession->m_stSourceUser, EN_TIPS_TYPE__REMOVE_OBSTCLE, pstPlayer->m_nUid, TRUE, pstParam->m_ddwType, 0);

        break;
    default:
        return -1;
        break;
    }
    //add_reward
    SSpGlobalRes stGlobalRes;
    stGlobalRes.Reset();
    TINT32 dwRetCode = 0;
    if(EN_ACTION_SEC_CLASS__BUILDING_UPGRADE == pstSession->m_stReqAlAction.m_nSclass)
    {
        dwRetCode = CGlobalResLogic::GetBuildingReward(pstParam->m_ddwType, pstParam->m_ddwTargetLevel, &stGlobalRes);

        //活动积分统计
        TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("ProcessActionBuilding_Au: Building Score."));
        CActivitesLogic::ComputeBuildUpgradeScore(&pstSession->m_stSourceUser, pstParam->m_ddwType, pstParam->m_ddwTargetLevel);
        CActivitesLogic::ComputeHeroExpBuildingScore(&pstSession->m_stSourceUser, pstParam->m_ddwExp);

    }
    if(EN_ACTION_SEC_CLASS__RESEARCH_UPGRADE == pstSession->m_stReqAlAction.m_nSclass)
    {
        dwRetCode = CGlobalResLogic::GetResearchReward(pstParam->m_ddwType, pstParam->m_ddwTargetLevel, &stGlobalRes);
        //活动积分统计
        TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("ProcessActionBuilding_Au: Research Score."));
        CActivitesLogic::ComputeResearchUpgradeScore(&pstSession->m_stSourceUser, pstParam->m_ddwType, pstParam->m_ddwTargetLevel);
        CActivitesLogic::ComputeHeroExpResearchScore(&pstSession->m_stSourceUser, pstParam->m_ddwExp);

    }
    if(EN_ACTION_SEC_CLASS__REMOVE_OBSTACLE == pstSession->m_stReqAlAction.m_nSclass)
    {
        dwRetCode = CGlobalResLogic::GetObstleReward(pstParam->m_ddwType, 1, &stGlobalRes);
    }
    if(dwRetCode != 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("BuildingActionDone:get  reward fail ret_code=%d type=%u lv=%u [seq=%u]",
            dwRetCode,
            pstParam->m_ddwType,
            pstParam->m_ddwTargetLevel,
            pstSession->m_udwSeqNo));
    }

    dwRetCode = CGlobalResLogic::AddSpGlobalRes(&pstSession->m_stSourceUser, pstCity, &stGlobalRes);
    if(dwRetCode != 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("BuildingActionDone:add  reward fail ret_code=%d type=%u lv=%u [seq=%u]",
            dwRetCode,
            pstParam->m_ddwType,
            pstParam->m_ddwTargetLevel,
            pstSession->m_udwSeqNo));
    }

    if(pstSession->m_stSourceUser.m_tbPlayer.m_nBuilding_force != CBufferBase::ComputeBuildingForce(pstCity, &pstSession->m_stSourceUser))
    {
        pstSession->m_stSourceUser.m_tbPlayer.Set_Building_force(CBufferBase::ComputeBuildingForce(pstCity, &pstSession->m_stSourceUser));
    }
    if(pstSession->m_stSourceUser.m_tbPlayer.m_nResearch_force != CBufferBase::ComputeResearchForce(pstCity, &pstSession->m_stSourceUser))
    {
        pstSession->m_stSourceUser.m_tbPlayer.Set_Research_force(CBufferBase::ComputeResearchForce(pstCity, &pstSession->m_stSourceUser));
    }

    TUINT32 udwNotiId = 0;
    switch(pstSession->m_stReqAlAction.m_nSclass)
    {
    case EN_ACTION_SEC_CLASS__BUILDING_UPGRADE:
        udwNotiId = EN_NOTI_ID__BUILDING_UPGRADE_OK;
        break;
    case EN_ACTION_SEC_CLASS__BUILDING_REMOVE:
        udwNotiId = EN_NOTI_ID__BUILDING_REMOVE_OK;
        break;
    case EN_ACTION_SEC_CLASS__RESEARCH_UPGRADE:
        udwNotiId = EN_NOTI_ID__RESEARCH_UPGRADE_OK;
        break;
    default:
        break;
    }

    if(udwNotiId > 0)
    {
        string strId = CCommonFunc::NumToString(pstSession->m_stReqAlAction.m_bParam[0].m_stBuilding.m_ddwType);
        TUINT32 udwLevel = pstSession->m_stReqAlAction.m_bParam[0].m_stBuilding.m_ddwTargetLevel;
        string strName = "";
        TBOOL bIsExistDocument = CDocument::GetInstance()->IsSupportLang("english");
        if(bIsExistDocument)
        {
            const Json::Value &stDocumentJson = CDocument::GetInstance()->GetDocumentJsonByLang("english");
            if(pstSession->m_stReqAlAction.m_nSclass == EN_ACTION_SEC_CLASS__RESEARCH_UPGRADE)
            {
                for(TUINT32 udwIdx = 0; udwIdx < stDocumentJson["doc_research"].size(); ++udwIdx)
                {
                    if(stDocumentJson["doc_research"][udwIdx]["id"].asString() == strId)
                    {
                        strName = stDocumentJson["doc_research"][udwIdx]["name"].asString();
                    }
                }
                if(strName == "")
                {
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("Noti error doc[research_id=%d][seq=%u]", pstSession->m_stReqAlAction.m_bParam[0].m_stBuilding.m_ddwType,
                        pstSession->m_udwSeqNo));
                }
            }
            else
            {
                if(stDocumentJson.isMember("doc_building")
                    && stDocumentJson["doc_building"].isMember(strId)
                    && stDocumentJson["doc_building"][strId].isMember("name"))
                {
                    strName = stDocumentJson["doc_building"][strId]["name"].asString();
                }
                else
                {
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("Noti error doc[building_id=%d][seq=%u]", pstSession->m_stReqAlAction.m_bParam[0].m_stBuilding.m_ddwType,
                        pstSession->m_udwSeqNo));
                }
            }
        }

        SNoticInfo stNoticInfo;
        stNoticInfo.Reset();
        stNoticInfo.SetValue(udwNotiId,
            "", "",
            0, 0,
            0, 0,
            0, strName, udwLevel);
        CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), pstSession->m_stSourceUser.m_tbPlayer.m_nSid, pstSession->m_stSourceUser.m_tbPlayer.m_nUid, stNoticInfo);
    }

    pstSession->m_dwMarchResult = EN_REPORT_RESULT_WIN;

    // 处理完成，设置删除当前action
    pstSession->m_ucReqAlActionFlag = EN_TABLE_UPDT_FLAG__DEL;

    return 0;
}

TINT32 CProcessAction::ProcessActionItem(SSession *pstSession, TBOOL &bNeedResponse)
{
    SActionItemParam *pstParam = &pstSession->m_stReqAction.m_bParam[0].m_stItem;

    // 根据action命令字分别进行计算
    if(pstParam->m_ddwBufferId == EN_BUFFER_INFO_PEACE_TIME)
    {
        if(pstSession->m_stSourceUser.m_tbPlayer.m_nStatus & EN_CITY_STATUS__NEW_PROTECTION)
        {
            //新手保护期间
            pstSession->m_stSourceUser.m_tbPlayer.Set_Status(pstSession->m_stSourceUser.m_tbPlayer.m_nStatus & (~EN_CITY_STATUS__NEW_PROTECTION));
            pstSession->m_stSourceUser.m_tbPlayer.Set_Status(pstSession->m_stSourceUser.m_tbPlayer.m_nStatus | EN_CITY_STATUS__NEW_PROTECTION_CANCEL);// 表示刚刚打破新手保护
        }
        else if(pstSession->m_stSourceUser.m_tbPlayer.m_nStatus & EN_CITY_STATUS__AVOID_WAR)
        {
            //peacetime
            pstSession->m_stSourceUser.m_tbPlayer.Set_Status(pstSession->m_stSourceUser.m_tbPlayer.m_nStatus & (~EN_CITY_STATUS__AVOID_WAR));
        }
    }

    if(pstParam->m_ddwBufferId == EN_BUFFER_INFO_VIP_ACTIVATE)
    {
        //tips
        CSendMessageBase::AddTips(&pstSession->m_stSourceUser, EN_TIPS_TYPY__VIP_INVALID, pstSession->m_stSourceUser.m_tbPlayer.m_nUid, TRUE, 0, 0, 0);
    }

    pstSession->m_dwMarchResult = EN_REPORT_RESULT_WIN;
    // 处理完成，设置删除当前action
    pstSession->m_ucReqActionFlag = EN_TABLE_UPDT_FLAG__DEL;

    return 0;
}

TINT32 CProcessAction::ProcessActionEquip(SSession *pstSession, TBOOL &bNeedResponse)
{
	SCityInfo *pstCity = &pstSession->m_stSourceUser.m_stCityInfo;
    if(pstCity == NULL)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessActionEquip:null city [seq=%u]", pstSession->m_stSourceUser.m_udwBSeqNo));
        return -1;
    }
    SActionEquipParam *pstParam = &pstSession->m_stReqAlAction.m_bParam[0].m_stEquip;
    SUserInfo *pstUser = &pstSession->m_stSourceUser;
    TINT64 ddwId = pstParam->m_uddwId;
    TINT32 dwEquipIdx = -1;
    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwEquipNum; ++udwIdx)
    {
        if(pstUser->m_aucEquipFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }
        if (pstUser->m_atbEquip[udwIdx].m_nId == ddwId)
        {
            dwEquipIdx = udwIdx;
            break;
        }
    }
    if(dwEquipIdx == -1)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("EquipUpgradeActionDone:player backpack have not such equip id[id=%ld e_id=%u] [seq=%u]",
            pstParam->m_uddwId, pstParam->m_ddwEType, pstSession->m_stSourceUser.m_udwBSeqNo));
        return -1;
    }
    pstUser->m_atbEquip[dwEquipIdx].Set_Status(EN_EQUIPMENT_STATUS_NORMAL);
    pstUser->m_atbEquip[dwEquipIdx].Set_Get_time(CTimeUtils::GetUnixTime());
    pstUser->m_aucEquipFlag[dwEquipIdx] = EN_TABLE_UPDT_FLAG__CHANGE;

    //活动积分统计
    SEquipMentInfo stEquip;
    stEquip.Reset();

    CBackpack::GetEquipBaseInfoByEid(pstParam->m_ddwEType, &stEquip);
    
    CActivitesLogic::ComputeEquipUpgradeScore(pstUser, pstParam->m_ddwLevel);

    string stQuanlity = "";
    string stTypeName = "";
    TBOOL bIsExistDocument = CDocument::GetInstance()->IsSupportLang("english");
    if (bIsExistDocument)
    {
        const Json::Value &stDocumentJson = CDocument::GetInstance()->GetDocumentJsonByLang("english");
        Json::Value::Members jQuanlityMember = stDocumentJson["doc_equipment_quality"].getMemberNames();
        for(Json::Value::Members::iterator it = jQuanlityMember.begin(); it != jQuanlityMember.end(); ++it)
        {
            if(stEquip.stBaseInfo.udwLv == (TUINT32)atoi((*it).c_str()))
            {
                stQuanlity = stDocumentJson["doc_equipment_quality"][(*it)].asString();
                break;
            }
        }
        
        if(stQuanlity == "")
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("Noti error doc[equip_lv=%d][seq=%u]", stEquip.stBaseInfo.udwLv,
                pstSession->m_udwSeqNo));
        }

        Json::Value::Members jTypeMember = stDocumentJson["doc_equipment"].getMemberNames();
        for(Json::Value::Members::iterator it = jTypeMember.begin(); it != jTypeMember.end(); ++it)
        {
            if (stEquip.stBaseInfo.udwEType == (TUINT32)atoi((*it).c_str()))
            {
                stTypeName = stDocumentJson["doc_equipment"][(*it)]["name"].asString();
                break;
            }
        }

        if(stTypeName == "")
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("Noti error doc[equip_type=%d][seq=%u]", stEquip.stBaseInfo.udwEType,
                pstSession->m_udwSeqNo));
        }
    }

    SGlobalRes stReward;
    stReward.ddwTotalNum = 1;
    stReward[0].ddwType = EN_GLOBALRES_TYPE_EQUIP;
    stReward[0].ddwId = pstParam->m_uddwId;
    stReward[0].ddwNum = 1;

    Json::Value jTmp = Json::Value(Json::objectValue);
    jTmp["equip"] = Json::Value(Json::arrayValue);
    jTmp["equip"][0U] = pstParam->m_ddwLevel;

    CCommonBase::AddRewardWindow(pstUser, pstUser->m_tbPlayer.m_nUid, EN_REWARD_WINDOW_TYPE_ONE_EQUIP, EN_REWARD_WINDOW_GET_TYPE_COMPOSE, 0, &stReward, TRUE, jTmp);

    SNoticInfo stNoticInfo;
    stNoticInfo.Reset();
    stNoticInfo.m_strKeyName = stTypeName;
    stNoticInfo.m_strValueName = stQuanlity;
    stNoticInfo.m_dwNoticId = EN_NOTI_ID__FORGE_EQUIP_SUCC;
    stNoticInfo.m_strLang = CDocument::GetLang(pstSession->m_stSourceUser.m_tbLogin.m_nLang);
    
    CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), pstSession->m_stSourceUser.m_tbPlayer.m_nSid, pstSession->m_stSourceUser.m_tbPlayer.m_nUid, stNoticInfo);

    CQuestLogic::SetTaskCurValue(&pstSession->m_stSourceUser, pstCity, EN_TASK_TYPE_ING_COMPOSE_EQUIP);

    CMsgBase::SendEncourageMail(&pstUser->m_tbUserStat, pstUser->m_tbPlayer.m_nSid, EN_MAIL_ID__FIRST_COMPOSE_EQUIP, 0, CCommonFunc::NumToString(stEquip.stBaseInfo.udwEType));

    pstSession->m_dwMarchResult = EN_REPORT_RESULT_WIN;
    // 处理完成，设置删除当前action
    pstSession->m_ucReqAlActionFlag = EN_TABLE_UPDT_FLAG__DEL;
    return 0;
}

TINT32 CProcessAction::ProcessActionAttackMove(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;

    SUserInfo *pstUser = &pstSession->m_stSourceUser;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TbAlliance *pstAlliance = &pstSession->m_stSourceUser.m_tbAlliance;
    SActionAttackMoveParam *pstParam = &pstSession->m_stReqAction.m_bParam[0].m_stAttackMove;

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        if (pstCity->m_stTblData.m_nPos != pstParam->m_ddwCityId)
        {
            // 处理完成，设置删除当前action 玩家在被动移城期间 已经主动移过城
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
            pstSession->m_ucReqActionFlag = EN_TABLE_UPDT_FLAG__DEL;
            return 0;
        }

        if (!pstCity)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessActionAttackMove: city id error [seq=%u]", pstSession->m_stSourceUser.m_udwBSeqNo));
            return -10;
        }

        //所有攻击即时到达...
        TBOOL bIsAttacked = FALSE;
        TBOOL bHasMarching = FALSE;
        TbMarch_action *ptbMarch = NULL;
        TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
        TUINT32 udwCount = 0;
        for (TUINT32 idx = 0; idx < pstUser->m_udwPassiveMarchNum; idx++)
        {
            ptbMarch = &pstUser->m_atbPassiveMarch[idx];
            if (ptbMarch->m_nTpos == pstUser->m_stCityInfo.m_stTblData.m_nPos)
            {
                if (ptbMarch->m_nMclass == EN_ACTION_MAIN_CLASS__MARCH
                    && (ptbMarch->m_nStatus == EN_MARCH_STATUS__MARCHING || ptbMarch->m_nStatus == EN_MARCH_STATUS__FIGHTING))
                {
                    if (ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR
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

        if (bIsAttacked == TRUE)
        {
            pstSession->m_stReqAction.Set_Etime(udwCurTime + 5);
            pstSession->m_ucReqActionFlag = EN_TABLE_UPDT_FLAG__CHANGE;
            return 0;
        }

        if (pstParam->m_ddwSpecailMoveFlag == 1)
        {
            for (TUINT32 idx = 0; idx < pstUser->m_udwMarchNum; idx++)
            {
                TbMarch_action *ptbMarch = &pstUser->m_atbMarch[idx];
                if (ptbMarch->m_nSuid == pstUser->m_tbPlayer.m_nUid
                    && ptbMarch->m_nMclass == EN_ACTION_MAIN_CLASS__MARCH)
                {
                    if (ptbMarch->m_nSclass != EN_ACTION_SEC_CLASS__RALLY_WAR
                        && ptbMarch->m_nSclass != EN_ACTION_SEC_CLASS__RALLY_WAR_THRONE)
                    {
                        if (ptbMarch->m_nStatus == EN_MARCH_STATUS__RETURNING)
                        {
                            bHasMarching = TRUE;
                            ptbMarch->Set_Etime(udwCurTime);
                            pstUser->m_aucMarchFlag[idx] = EN_TABLE_UPDT_FLAG__CHANGE;
                        }
                        else if (ptbMarch->m_nStatus == EN_MARCH_STATUS__MARCHING
                            || ptbMarch->m_nStatus == EN_MARCH_STATUS__FIGHTING
                            || ptbMarch->m_nStatus == EN_MARCH_STATUS__SETUP_CAMP
                            || ptbMarch->m_nStatus == EN_MARCH_STATUS__PRE_LOADING
                            || ptbMarch->m_nStatus == EN_MARCH_STATUS__UN_LOADING)
                        {
                            bHasMarching = TRUE;
                            ptbMarch->Set_Status(EN_MARCH_STATUS__RETURNING);
                            ptbMarch->Set_Etime(udwCurTime);
                            pstUser->m_aucMarchFlag[idx] = EN_TABLE_UPDT_FLAG__CHANGE;
                        }
                    }
                    else
                    {
                        if (ptbMarch->m_nStatus == EN_MARCH_STATUS__RETURNING)
                        {
                            ptbMarch->Set_Etime(udwCurTime);
                        }
                        else
                        {
                            for (TUINT32 udwReinIdx = 0; udwReinIdx < pstUser->m_udwPassiveMarchNum; udwReinIdx++)
                            {
                                TbMarch_action* ptbRallyReinforce = &pstUser->m_atbPassiveMarch[udwReinIdx];
                                if (ptbRallyReinforce->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_REINFORCE
                                    && ptbRallyReinforce->m_nStatus != EN_MARCH_STATUS__RETURNING
                                    && ptbRallyReinforce->m_nTid == ptbMarch->m_nId)
                                {
                                    if (ptbRallyReinforce->m_nStatus == EN_MARCH_STATUS__PREPARING)
                                    {
                                        CActionBase::ReturnMarch(ptbRallyReinforce);
                                    }
                                    else if (ptbRallyReinforce->m_nStatus == EN_MARCH_STATUS__MARCHING)
                                    {
                                        CActionBase::ReturnMarchOnFly(ptbRallyReinforce);
                                    }
                                    pstUser->m_aucPassiveMarchFlag[udwReinIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
                                }
                            }

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
                        }
                        bHasMarching = TRUE;
                        pstUser->m_aucMarchFlag[idx] = EN_TABLE_UPDT_FLAG__CHANGE;
                    }
                }
            }
        }
        
        if (bHasMarching == TRUE)
        {
            pstSession->m_stReqAction.Set_Etime(udwCurTime + 5);
            pstSession->m_ucReqActionFlag = EN_TABLE_UPDT_FLAG__CHANGE;
            return 0;
        }

        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;

        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__MAP_SVR;
        pstSession->ResetMapSvrReq();
        MapSvrReqInfo* pMapSvrReq = new MapSvrReqInfo(pstSession->m_udwReqSvrId, "GetNewCityMap");
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

        return 0;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;

        dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[0],
            &pstSession->m_tbTmpMap);

        if (dwRetCode <= 0 || pstSession->m_tbTmpMap.m_nId == 0 || pstSession->m_tbTmpMap.m_nUid != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__MOVE_CITY_COORD_INVALID;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessActionAttackMove: wild belong someone else. [seq=%u]", pstSession->m_stSourceUser.m_udwBSeqNo));
            return -4;
        }
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        TUINT32 udwSmokingTime = 0;
        TUINT32 udwBurnTime = 0;
        TUINT32 udwCatchHeroFlag = 0;
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
                ptbMap->m_nSmoke_end_time > CTimeUtils::GetUnixTime())
            {
                udwSmokingTime = ptbMap->m_nSmoke_end_time;
            }
            if (pstUser->m_tbPlayer.m_nStatus & EN_CITY_STATUS__BEEN_REMOVE &&
                ptbMap->m_nBurn_end_time > CTimeUtils::GetUnixTime())
            {
                udwBurnTime = ptbMap->m_nBurn_end_time;
            }
            udwCatchHeroFlag = ptbMap->m_nPrison_flag;
            pstSession->m_tbTmpMap.Set_City_info(ptbMap->m_jCity_info);
            break;
        }

        // 设置新map数据
        if (pstParam->m_ddwSpecailMoveFlag != 1)
        {
            pstUser->m_tbPlayer.Set_Status(pstUser->m_tbPlayer.m_nStatus & (~EN_CITY_STATUS__BEEN_REMOVE));
            udwSmokingTime = 0;
            udwBurnTime = 0;
        }

        pstUser->m_tbPlayer.Set_Status(pstUser->m_tbPlayer.m_nStatus & (~EN_CITY_STATUS__ON_SPECIAL_WILD));

        CCommonBase::SetMapToNewCity(&pstSession->m_tbTmpMap, &pstUser->m_tbPlayer, pstCity, pstAlliance, udwSmokingTime, udwBurnTime, udwCatchHeroFlag);

        // set package
        pstSession->ResetAwsReq();
        CAwsRequest::UpdateItem(pstSession, &pstSession->m_tbTmpMap, ExpectedDesc(), 0, true);

        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessActionAttackMove: move new city [id=%u] [seq=%u]", pstSession->m_tbTmpMap.m_nId, pstSession->m_stSourceUser.m_udwBSeqNo));

        //wave@push_data
        CAuPushData::PushData_WildToMap(pstSession, &pstSession->m_tbTmpMap);

        return 0;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        TUINT32 udwOldCityId = pstSession->m_stReqAction.m_bParam[0].m_stAttackMove.m_ddwCityId;
        TUINT32 udwNewCityId = pstSession->m_tbTmpMap.m_nId;

        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessActionAttackMove:old=%u new=%u [seq=%u]",
            udwOldCityId, udwNewCityId, pstSession->m_stSourceUser.m_udwBSeqNo));
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__4;
        //pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

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
        TUINT32 idx = 0;
        for (idx = 0; idx < pstUser->m_udwActionNum; idx++)
        {
            TbAction *ptbAction = &pstUser->m_atbAction[idx];
            //是否存在被动移城action
            if (ptbAction->m_nSclass == EN_ACTION_SEC_CLASS__ATTACK_MOVE)
            {
                pstUser->m_aucActionFlag[idx] = EN_TABLE_UPDT_FLAG__DEL;
                continue;
            }
        }

        for (idx = 0; idx < pstUser->m_udwSelfAlActionNum; idx++)
        {
            TbAlliance_action *ptbAlAction = &pstUser->m_atbSelfAlAction[idx];
            if (ptbAlAction->m_nScid == udwOldCityId)
            {
                ptbAlAction->Set_Scid(udwNewCityId);
                //CAwsRequest::UpdateItem(pstSession, ptbAlAction);
                pstUser->m_aucSelfAlActionFlag[idx] = EN_TABLE_UPDT_FLAG__CHANGE;
            }
        }

        for (idx = 0; idx < pstUser->m_udwMarchNum; idx++)
        {
            TbMarch_action *ptbMarch = &pstUser->m_atbMarch[idx];
            if (ptbMarch->m_nScid == udwOldCityId)
            {
                ptbMarch->Set_Scid(udwNewCityId);
                ptbMarch->Set_Sbid(CMapBase::GetBlockIdFromPos(udwNewCityId));
                //CAwsRequest::UpdateItem(pstSession, ptbMarch);
                pstUser->m_aucMarchFlag[idx] = EN_TABLE_UPDT_FLAG__CHANGE;
            }
        }

        for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwPassiveMarchNum; udwIdx++)
        {
            TbMarch_action *ptbPassiveMarch = &pstUser->m_atbPassiveMarch[udwIdx];

            // 移城后rallywar变化
            if (ptbPassiveMarch->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR
                && ptbPassiveMarch->m_nTuid == pstUser->m_tbPlayer.m_nUid)
            {
                ptbPassiveMarch->m_bParam[0].m_ddwTargetCityId = udwNewCityId;
                ptbPassiveMarch->SetFlag(TbMARCH_ACTION_FIELD_PARAM);
                pstUser->m_aucPassiveMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
            }
            else if (ptbPassiveMarch->m_nTpos == udwOldCityId
                && ptbPassiveMarch->m_nTuid == pstUser->m_tbPlayer.m_nUid)
            {
                pstUser->m_aucPassiveMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
                if (ptbPassiveMarch->m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_NORMAL
                    && ptbPassiveMarch->m_nStatus == EN_MARCH_STATUS__DEFENDING)
                {
                    TINT64 ddwCtime = CToolBase::GetMarchTime(ptbPassiveMarch, udwNewCityId);
                    ptbPassiveMarch->m_bParam[0].m_ddwMarchingTime = ddwCtime;
                    ptbPassiveMarch->SetFlag(TbMARCH_ACTION_FIELD_PARAM);
                    ptbPassiveMarch->Set_Tpos(udwNewCityId);
                    ptbPassiveMarch->Set_Tbid(CMapBase::GetBlockIdFromPos(udwNewCityId));
                }
                else if (ptbPassiveMarch->m_nSclass == EN_ACTION_SEC_CLASS__PRISON_TIMER)
                {
                    ptbPassiveMarch->Set_Tpos(udwNewCityId);
                    ptbPassiveMarch->Set_Tbid(CMapBase::GetBlockIdFromPos(udwNewCityId));

                    //将被抓龙者加入更新列表
                    CAuPushDataNode *pobjNode = &pstSession->m_objAuPushDataNode;
                    CPushAlData *pobjSourceAlData = &pobjNode->m_objPushDataSourceAl;
                    CAuPushData::AddPushData_AlUid(pobjSourceAlData, ptbPassiveMarch->m_nSuid);
                }
                else
                {
                    ptbPassiveMarch->Set_Tal(0);
                }
            }
        }

        // send request
//         if (pstSession->m_vecAwsReq.size() > 0)
//         {
//             return 0;
//         }
//         else
//         {
//             TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessActionAttackMove: no action and wild need update [seq=%u]", pstSession->m_stSourceUser.m_udwBSeqNo));
//         }
    }

    // 6. 最后更新本地city、city所在map和player信息
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__4)
    {
        TUINT32 udwRawCityId = pstSession->m_stReqAction.m_bParam[0].m_stAttackMove.m_ddwCityId;
        TUINT32 udwNewCityId = pstSession->m_tbTmpMap.m_nId;

        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__5;

        // 更新city的id
        pstCity->m_stTblData.Set_Pos(udwNewCityId);
        pstUser->m_udwMoveNewCityId = udwNewCityId;

        // 更新player数据
        pstUser->m_tbPlayer.Set_Cid(udwNewCityId);

        if (pstParam->m_ddwSpecailMoveFlag == 0)
        {
            pstUser->m_tbUserStat.Set_Remove_flag(1);
        }
        else if (pstParam->m_ddwSpecailMoveFlag == 1)
        {
            pstUser->m_tbUserStat.Set_Remove_flag(2);
        }

        // 更新原始map数据
        pstSession->m_ucTmpMapItemFlag = EN_TABLE_UPDT_FLAG__CHANGE;
        pstSession->m_tbTmpMap.Reset();
        pstSession->m_tbTmpMap.Set_Sid(pstSession->m_stSourceUser.m_tbPlayer.m_nSid);
        pstSession->m_tbTmpMap.Set_Id(udwRawCityId);
        pstSession->m_tbTmpMap.Set_Type(EN_WILD_TYPE__CITY);
        CCommonBase::AbandonWild(pstCity, &pstSession->m_tbTmpMap);
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__5)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;

        //禁止 after更新
        pstUser->m_udwWildNum = 0;

        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessActionAttackMove:finish [seq=%u]",
            pstSession->m_stSourceUser.m_udwBSeqNo));
    }
    
    pstSession->m_dwMarchResult = EN_REPORT_RESULT_WIN;
    // 处理完成，设置删除当前action
    pstSession->m_ucReqActionFlag = EN_TABLE_UPDT_FLAG__DEL;

    return 0;
}

TINT32 CProcessAction::ProcessMarchReturn(SSession *pstSession)
{
    TbMarch_action *ptbReqMarch = &pstSession->m_stReqMarch;
    SUserInfo *pstUser = &pstSession->m_stSourceUser;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;
	SCityInfo *pstCity = &pstUser->m_stCityInfo;

    SActionMarchParam *pstParam = &ptbReqMarch->m_bParam[0];

    TBOOL bIsDragonReturn = FALSE;
    TBOOL bIsKnightReturn = FALSE;
    TBOOL bIsTroopReturn = FALSE;

    //troop
    for(TUINT32 udwIdx = 0; udwIdx < EN_TROOP_TYPE__END; ++udwIdx)
    {
        pstCity->m_stTblData.m_bTroop[0].m_addwNum[udwIdx] += pstParam->m_stTroop[udwIdx];
        if (!bIsTroopReturn && pstParam->m_stTroop[udwIdx])
        {
            bIsTroopReturn = TRUE;
        }
    }
    pstCity->m_stTblData.SetFlag(TbCITY_FIELD_TROOP);

    //resource 
    TUINT32 udwTotalResAmount = 0;
    for(TUINT32 udwIdx = 0; udwIdx < EN_RESOURCE_TYPE__END; ++udwIdx)
    {
        pstCity->m_stTblData.m_bResource[0].m_addwNum[udwIdx] += pstParam->m_stResource[udwIdx];
        udwTotalResAmount += pstParam->m_stResource[udwIdx];
        if(pstCity->m_stTblData.m_bResource[0].m_addwNum[udwIdx] > MAX_RESOURCE_LIMIT_NUM)
        {
            pstCity->m_stTblData.m_bResource[0].m_addwNum[udwIdx] = MAX_RESOURCE_LIMIT_NUM;
        }
    }
    pstCity->m_stTblData.SetFlag(TbCITY_FIELD_RESOURCE);

    if (ptbReqMarch->m_nSclass == EN_ACTION_SEC_CLASS__CATCH_DRAGON)
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("catch_dragon: return a catch_dragon"));
    }
    else
    {
        if (pstParam->m_stDragon.m_ddwLevel > 0 && pstParam->m_stDragon.m_ddwCaptured < 0)
        {
            ptbPlayer->Set_Dragon_tid(0);
            ptbPlayer->Set_Dragon_status(EN_DRAGON_STATUS_NORMAL);
            bIsDragonReturn = TRUE;
        }
        else if (ptbReqMarch->m_bPrison_param[0].stDragon.m_ddwLevel > 0)
        {
            ptbPlayer->Set_Dragon_tid(0);
            ptbPlayer->Set_Dragon_status(EN_DRAGON_STATUS_NORMAL);
            bIsDragonReturn = TRUE;
        }
    }

    if(ptbReqMarch->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR)
    {
        for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
        {
            if(pstUser->m_aucMarchFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
            {
                continue;
            }

            TbMarch_action* ptbRallyReinforce = &pstUser->m_atbMarch[udwIdx];
            if(ptbRallyReinforce->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_REINFORCE
            && ptbRallyReinforce->m_nStatus == EN_MARCH_STATUS__PREPARING
            && ptbRallyReinforce->m_nTid == ptbReqMarch->m_nId)
            {
                ptbRallyReinforce->Set_Status(EN_MARCH_STATUS__RETURNING);
                ptbRallyReinforce->Set_Btime(ptbReqMarch->m_nEtime);
                ptbRallyReinforce->Set_Etime(ptbRallyReinforce->m_nBtime + ptbRallyReinforce->m_nCtime);
            }
        }
    }

    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwPassiveMarchNum; ++udwIdx)
    {
        if(pstUser->m_aucPassiveMarchFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }

        if(pstUser->m_atbPassiveMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__PRISON_TIMER
        && pstUser->m_atbPassiveMarch[udwIdx].m_nTid == ptbReqMarch->m_nId)
        {
            pstUser->m_atbPassiveMarch[udwIdx].Set_Status(EN_MARCH_STATUS__DEFENDING);
            pstUser->m_atbPassiveMarch[udwIdx].Set_Etime(CTimeUtils::GetUnixTime() +
                pstUser->m_atbPassiveMarch[udwIdx].m_bPrison_param[0].ddwReleaseWait);

            pstUser->m_atbPassiveMarch[udwIdx].m_bPrison_param[0].ddwEscortActionId = 0;
            pstUser->m_atbPassiveMarch[udwIdx].SetFlag(TbMARCH_ACTION_FIELD_PRISON_PARAM);
            //pstUser->m_atbPassiveMarch[udwIdx].DeleteField(TbMARCH_ACTION_FIELD_TID);
            pstUser->m_atbPassiveMarch[udwIdx].Set_Tid(0, UPDATE_ACTION_TYPE_DELETE);
        }
    }

    //reward
    CGlobalResLogic::AddGlobalRes(pstUser, pstCity, &ptbReqMarch->m_bReward[0]);
    CGlobalResLogic::AddGlobalRes(pstUser, pstCity, &ptbReqMarch->m_bSp_reward[0]);

    //dead troop
    TINT64 ddwDeadNum = 0;
    TINT64 ddwDeadMight = 0;
    CCityBase::ComputeDeadTroopMight(ptbReqMarch, ddwDeadMight, ddwDeadNum);

    //knight
    if(pstParam->m_stKnight.ddwId >= 0)
    {
        TUINT32 udwLvBefore = CGameInfo::GetInstance()->ComputeKnightLevelByExp(pstCity->m_stTblData.m_bKnight[pstParam->m_stKnight.ddwId].ddwExp);
        pstCity->m_stTblData.m_bKnight[pstParam->m_stKnight.ddwId].ddwStatus = EN_KNIGHT_STATUS__NORMAL;
        CCityBase::AddKnightExp(pstCity, pstParam->m_stKnight.ddwId, pstParam->m_stKnight.ddwExpAdd);
        pstCity->m_stTblData.m_bKnight[pstParam->m_stKnight.ddwId].ddwTid = 0;
        pstCity->m_stTblData.SetFlag(TbCITY_FIELD_KNIGHT);
        TUINT32 udwLvAfter = CGameInfo::GetInstance()->ComputeKnightLevelByExp(pstCity->m_stTblData.m_bKnight[pstParam->m_stKnight.ddwId].ddwExp);

        if (udwLvAfter > udwLvBefore)
        {
            CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPE__KNIGHT_LV_UP, ptbPlayer->m_nUid, TRUE, pstParam->m_stKnight.ddwId);
        }

        bIsKnightReturn = TRUE;
    }

    //player related
    if (ptbReqMarch->m_nSclass != EN_ACTION_SEC_CLASS__ATTACK_IDOL)
    {
        ptbPlayer->Set_Ktroop(ptbPlayer->m_nKtroop + ptbReqMarch->m_nKill_troop_might);
        ptbPlayer->Set_Kfort(ptbPlayer->m_nKfort + ptbReqMarch->m_nKill_fort_might);
        ptbPlayer->Set_Dtroop(ptbPlayer->m_nDtroop + ddwDeadMight);
        ptbPlayer->Set_Mkill(ptbPlayer->m_nMkill + ptbReqMarch->m_nKill_troop_might + ptbReqMarch->m_nKill_fort_might);
    }
    else
    {
        ptbPlayer->Set_Evil_force_kill(ptbPlayer->m_nEvil_force_kill + ptbReqMarch->m_nKill_troop_might + ptbReqMarch->m_nKill_fort_might);
        ptbPlayer->Set_Evil_troop_kill(ptbPlayer->m_nEvil_troop_kill + ptbReqMarch->m_nKill_troop_num);
    }

    TUINT32 udwUseless;
    ptbPlayer->Set_Cur_troop_might(CCommonBase::ComputeFortMight(pstUser, udwUseless));
    ptbPlayer->Set_Cur_fort_might(CCommonBase::ComputeTotalTroopMight(pstUser, udwUseless));

    if(ptbReqMarch->m_nSclass == EN_ACTION_SEC_CLASS__ATTACK
    || ptbReqMarch->m_nSclass == EN_ACTION_SEC_CLASS__OCCUPY)
    {
        ptbPlayer->Set_Res_col(ptbPlayer->m_nRes_col + udwTotalResAmount);
    }

    //玩家信息统计
    if(ptbReqMarch->m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_NORMAL
        || ptbReqMarch->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_REINFORCE
        || ptbReqMarch->m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_THRONE)
    {
        ptbPlayer->m_bWar_statistics[0].ddwDfnWin += ptbReqMarch->m_bReinforce_result[0].ddwReinforceDefendWin;
        ptbPlayer->m_bWar_statistics[0].ddwDfnFail += ptbReqMarch->m_bReinforce_result[0].ddwReinforceDefendFail;
        ptbPlayer->m_bWar_statistics[0].ddwAtkWin += ptbReqMarch->m_bReinforce_result[0].ddwReinforceAtkWin;
        ptbPlayer->m_bWar_statistics[0].ddwAtkFail += ptbReqMarch->m_bReinforce_result[0].ddwReinforceAtkFail;

        ptbPlayer->m_bWar_statistics[0].ddwHurtEnemyTroopNum += 0;

        ptbPlayer->m_bWar_statistics[0].ddwMyTroopDamagedNum += ddwDeadNum;
        ptbPlayer->m_bWar_statistics[0].ddwDamageTroopNum += ptbReqMarch->m_nKill_troop_num;
        ptbPlayer->m_bWar_statistics[0].ddwDamageFortNum += ptbReqMarch->m_nKill_fort_num;
        ptbPlayer->m_bWar_statistics[0].ddwForceKilled += ptbReqMarch->m_nKill_troop_might + ptbReqMarch->m_nKill_fort_might;
        ptbPlayer->SetFlag(TbPLAYER_FIELD_WAR_STATISTICS);

        CQuestLogic::SetTaskCurValue(pstUser, pstCity, EN_TASK_TYPE_ING_KILL_TROP_NUM, ptbReqMarch->m_nKill_troop_num);
        CQuestLogic::SetTaskCurValue(pstUser, pstCity, EN_TASK_TYPE_ING_KILL_FORT_NUM, ptbReqMarch->m_nKill_fort_num);

        TUINT32 udwTmpUid = 0;
        if(ptbReqMarch->m_bParam[0].m_ddwTargetType == EN_WILD_TYPE__CITY || 
            ptbReqMarch->m_bParam[0].m_ddwTargetType == EN_WILD_TYPE__THRONE_NEW)
        {
            udwTmpUid = 1;
        }
        
        /*CActivitesLogic::ComputeTroopKillScore(&pstSession->m_stSourceUser,
            ptbReqMarch->m_nKill_troop_might,
            ptbReqMarch->m_nKill_fort_might,
            udwTmpUid);*/
    }
    else if(ptbReqMarch->m_nSclass == EN_ACTION_SEC_CLASS__OCCUPY)
    {
        ptbPlayer->m_bWar_statistics[0].ddwDfnWin += ptbReqMarch->m_bReinforce_result[0].ddwReinforceDefendWin;
        ptbPlayer->m_bWar_statistics[0].ddwDfnFail += ptbReqMarch->m_bReinforce_result[0].ddwReinforceDefendFail;
        ptbPlayer->m_bWar_statistics[0].ddwAtkWin += ptbReqMarch->m_bReinforce_result[0].ddwReinforceAtkWin;
        ptbPlayer->m_bWar_statistics[0].ddwAtkFail += ptbReqMarch->m_bReinforce_result[0].ddwReinforceAtkFail;

        ptbPlayer->m_bWar_statistics[0].ddwMyTroopDamagedNum += ddwDeadNum;
        ptbPlayer->m_bWar_statistics[0].ddwDamageTroopNum += ptbReqMarch->m_nKill_troop_num;
        ptbPlayer->m_bWar_statistics[0].ddwDamageFortNum += ptbReqMarch->m_nKill_fort_num;
        ptbPlayer->m_bWar_statistics[0].ddwForceKilled += ptbReqMarch->m_nKill_troop_might + ptbReqMarch->m_nKill_fort_might;
        ptbPlayer->SetFlag(TbPLAYER_FIELD_WAR_STATISTICS);

        CQuestLogic::SetTaskCurValue(pstUser, pstCity, EN_TASK_TYPE_ING_KILL_TROP_NUM, ptbReqMarch->m_nKill_troop_num);
        CQuestLogic::SetTaskCurValue(pstUser, pstCity, EN_TASK_TYPE_ING_KILL_FORT_NUM, ptbReqMarch->m_nKill_fort_num);

//         CActivitesLogic::ComputeTroopKillScore(&pstSession->m_stSourceUser,
//             ptbReqMarch->m_nKill_troop_might,
//             ptbReqMarch->m_nKill_fort_might,
//             1);
    }

    if (bIsDragonReturn)
    {
        SNoticInfo stNoticInfo;
        stNoticInfo.Reset();
        stNoticInfo.SetValue(EN_NOTI_ID__HERO_RETURN,
            "", "",
            0, 0,
            0, 0,
            0, "", 0, CDocument::GetLang(pstUser->m_tbLogin.m_nLang));
        CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), pstUser->m_tbLogin.m_nSid, pstUser->m_tbLogin.m_nUid, stNoticInfo);
    }
    else if (bIsTroopReturn)
    {
        SNoticInfo stNoticInfo;
        stNoticInfo.Reset();
        stNoticInfo.SetValue(EN_NOTI_ID__TROOP_RETURN,
            "", "",
            0, 0,
            0, 0,
            0, "", 0, CDocument::GetLang(pstUser->m_tbLogin.m_nLang));
        CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), pstUser->m_tbLogin.m_nSid, pstUser->m_tbLogin.m_nUid, stNoticInfo);
    }

    if (ptbReqMarch->m_nSclass != EN_ACTION_SEC_CLASS__DRAGON_ATTACK)
    {
        if (bIsKnightReturn || bIsTroopReturn || bIsDragonReturn)
        {
            CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPE__TROOP_RETURN, ptbPlayer->m_nUid, TRUE);
        }
    }

    //del action
    pstSession->m_dwMarchResult = EN_REPORT_RESULT_WIN;
    pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__DEL;

    return 0;
}

TINT32 CProcessAction::ProcessMarchTransport(SSession *pstSession)
{
    SCityInfo *pstTCity = NULL;
    TbMarch_action *ptbReqMarch = &pstSession->m_stReqMarch;
    TbMap *ptbWild = &pstSession->m_stMapItem;
    SUserInfo *pstSUser = &pstSession->m_stSourceUser;
    TbPlayer *ptbSPlayer = &pstSUser->m_tbPlayer;

    TBOOL bSuccess = TRUE;
    TINT32 dwReason = EN_REPORT_RESULT_TARGET_CHANGE;

    // a. 任务合法性检查
    if(bSuccess && (ptbWild->m_nType != EN_WILD_TYPE__CITY || ptbWild->m_nUid != ptbReqMarch->m_nTuid || pstSession->m_bCheckValidFlag == FALSE))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchTransport: check_valid=%d target[type:%ld] not city or target[uid:%ld,%ld] has changed, just return... [seq=%u]",
            pstSession->m_bCheckValidFlag, ptbWild->m_nType, ptbReqMarch->m_nTuid, ptbWild->m_nUid, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_CHANGE;
    }
    pstTCity = &pstSession->m_stTargetUser.m_stCityInfo;
    if(bSuccess && pstTCity->m_stTblData.m_nPos == 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchTransport: no such city[%lld], just return... [seq=%u]",
            ptbReqMarch->m_nTpos, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_CHANGE;
    }
    if(bSuccess && (ptbSPlayer->m_nAlpos == 0 || ptbSPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET != ptbWild->m_nAlid))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchTransport: target[salpos:%u,sal:%u,tal:%u] not right, just return... [seq=%u]",
            ptbSPlayer->m_nAlpos, ptbSPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET, ptbWild->m_nAlid, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_DIFF_ALLIANCE;
    }

    // b. 进行资源卸载
    if(bSuccess == TRUE)
    {
        ptbReqMarch->Set_Status(EN_MARCH_STATUS__UN_LOADING);
        ptbReqMarch->Set_Etime(ptbReqMarch->m_nEtime + 3);
        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;
    }
    else
    {
        CProcessReport::GenInValidReport(pstSession, EN_REPORT_TYPE_TRANSPORT, dwReason, &pstSession->m_tbReport);
        // c. 更新action状态
        ptbReqMarch->Set_Status(EN_MARCH_STATUS__RETURNING);
        ptbReqMarch->Set_Btime(ptbReqMarch->m_nEtime);
        ptbReqMarch->Set_Etime(ptbReqMarch->m_nBtime + ptbReqMarch->m_nCtime);
        //ptbReqMarch->DeleteField(TbMARCH_ACTION_FIELD_TAL);
        ptbReqMarch->Set_Tal(0, UPDATE_ACTION_TYPE_DELETE);
        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;
    }

    return 0;
}

TINT32 CProcessAction::ProcessMarchReinForce(SSession *pstSession)
{
    TbMarch_action *ptbReqMarch = &pstSession->m_stReqMarch;
    SUserInfo *pstSUser = &pstSession->m_stSourceUser;
    TbPlayer *ptbSPlayer = &pstSUser->m_tbPlayer;
    SUserInfo *pstTUser = &pstSession->m_stTargetUser;
    SCityInfo *pstTCity = NULL;
    TbMap *ptbWild = &pstSession->m_stMapItem;

    TBOOL bSuccess = TRUE;
    TINT32 dwReason = EN_REPORT_RESULT_TARGET_CHANGE;

    // 任务合法性检查
    if(ptbWild->m_nType != EN_WILD_TYPE__CITY || ptbWild->m_nUid != ptbReqMarch->m_nTuid || pstSession->m_bCheckValidFlag == FALSE)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchReinForce: check_valid=%d target[type:%ld] not city or target[uid:%ld,%ld] has changed, just return... [seq=%u]",
            pstSession->m_bCheckValidFlag, ptbWild->m_nType, ptbReqMarch->m_nTuid, ptbWild->m_nUid, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_CHANGE;
    }

    if(ptbWild->m_nType == EN_WILD_TYPE__CITY)
    {
        pstTCity = &pstTUser->m_stCityInfo;
        if(bSuccess && pstTCity->m_stTblData.m_nPos == 0)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchReinForce: no such city[%lld], just return... [seq=%u]",
                ptbReqMarch->m_nTpos, pstSession->m_udwSeqNo));
            bSuccess = FALSE;
            dwReason = EN_REPORT_RESULT_TARGET_CHANGE;
        }
        if (bSuccess && CCityBase::GetBuildingLevelByFuncType(pstTCity, EN_BUILDING_TYPE__EMBASSY) <= 0)
        {
            bSuccess = FALSE;
            dwReason = EN_REPORT_RESULT_NO_EMBASSY;
        }
    }

    if(bSuccess && (ptbSPlayer->m_nAlpos == 0 || ptbSPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET != ptbWild->m_nAlid))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchReinForce: target[salpos:%ld,sal:%ld,tal:%ld] not right, just return... [seq=%u]",
            ptbSPlayer->m_nAlpos, ptbSPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET, ptbWild->m_nAlid, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_DIFF_ALLIANCE;
    }

    // 更新action状态, 将部队设置为encamp
    if(bSuccess == TRUE)
    {
        ptbReqMarch->Set_Status(EN_MARCH_STATUS__SETUP_CAMP);
        ptbReqMarch->Set_Btime(ptbReqMarch->m_nEtime);
        ptbReqMarch->Set_Etime(ptbReqMarch->m_nBtime + 3);
        if(ptbWild->m_nType == EN_WILD_TYPE__CITY)
        {
            ptbReqMarch->Set_Tal(-1 * ptbReqMarch->m_nTuid);
        }
        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;
    }
    else//部队直接放回
    {
        CProcessReport::GenInValidReport(pstSession, EN_REPORT_TYPE_REINFORCE, dwReason, &pstSession->m_tbReport);

        ptbReqMarch->Set_Status(EN_MARCH_STATUS__RETURNING);
        ptbReqMarch->Set_Btime(ptbReqMarch->m_nEtime);
        ptbReqMarch->Set_Etime(ptbReqMarch->m_nBtime + ptbReqMarch->m_nCtime);
        //ptbReqMarch->DeleteField(TbMARCH_ACTION_FIELD_TAL);
        ptbReqMarch->Set_Tal(0, UPDATE_ACTION_TYPE_DELETE);
        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;
    }

    return 0;
}

TINT32 CProcessAction::ProcessMarchAttack(SSession *pstSession)
{
    TbMarch_action *ptbReqMarch = &pstSession->m_stReqMarch;
    SActionMarchParam *pstParam = &ptbReqMarch->m_bParam[0];
    TbMap *ptbWild = &pstSession->m_stMapItem;
    TBOOL bSuccess = TRUE;
    TINT32 dwReason = EN_REPORT_RESULT_TARGET_CHANGE;
    SUserInfo *pstSUser = &pstSession->m_stSourceUser;
    TbDiplomacy *ptbDip = NULL;
    TbPlayer *ptbSPlayer = &pstSUser->m_tbPlayer;

    TUINT32 udwMapType = CMapLogic::GetWildClass(ptbWild->m_nSid, ptbWild->m_nType);

    if(ptbWild->m_nAlid != 0 && ptbSPlayer->m_nAlpos != 0)
    {
        ptbDip = CCommonBase::GetDiplomacy(pstSession->m_stSourceUser.m_atbDiplomacy, pstSession->m_stSourceUser.m_udwDiplomacyNum,
            ptbWild->m_nAlid);
    }

    // 任务合法性检查
    if ((ptbSPlayer->m_nStatus & EN_CITY_STATUS__NEW_PROTECTION)
        || (ptbSPlayer->m_nStatus & EN_CITY_STATUS__AVOID_WAR))
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchAttack: srcPlayer[id=%ld] is in peace time, just return... [seq=%u]",
            ptbSPlayer->m_nUid, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_SELF_PEACETIME;
    }

    if (bSuccess && pstSession->m_bCheckValidFlag == FALSE)
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchAttack: check_valid=%d suid[%ld] tuid[%ld] wild_type[%ld], just return... [seq=%u]",
            pstSession->m_bCheckValidFlag, ptbReqMarch->m_nSuid, ptbReqMarch->m_nTuid, ptbWild->m_nType, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_CHANGE;
    }

    if(bSuccess && udwMapType != EN_WILD_CLASS_MONSTER_NEST && ptbReqMarch->m_nTuid != ptbWild->m_nUid)
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchAttack: suid[%ld] tuid[%ld] wild_type[%ld], just return... [seq=%u]",
            ptbReqMarch->m_nSuid, ptbReqMarch->m_nTuid, ptbWild->m_nType, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_CHANGE;
    }

    if (bSuccess && udwMapType == EN_WILD_CLASS_MONSTER_NEST && ptbReqMarch->m_nTal != ptbWild->m_nAlid)
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchAttack: attack lair: req_tal[%lld] wild_alid[%ld], just return... [seq=%u]",
            ptbReqMarch->m_nTal, ptbWild->m_nAlid, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_CHANGE;
    }

    if(bSuccess && ptbSPlayer->m_nAlpos != 0 && ptbSPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET == ptbWild->m_nAlid)
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchAttack: sal[%u]=tal[%u], just return... [seq=%u]",
            pstParam->m_ddwSourceAlliance, pstParam->m_ddwTargetAlliance, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_SAME_ALLIANCE;
    }

    if(bSuccess && ptbDip && ptbDip->m_nType == EN_DIPLOMACY_TYPE__FRIENDLY)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchAttack: sal[%u] and tal[%u] is friend, just return... [seq=%u]",
            ptbSPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET, ptbWild->m_nAlid, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_IS_FRIEND;
    }

    if(bSuccess && ((ptbWild->m_nStatus & EN_CITY_STATUS__AVOID_WAR) || (ptbWild->m_nStatus & EN_CITY_STATUS__NEW_PROTECTION)))
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchAttack: target[%ld] is in peace time, just return... [seq=%u]",
            ptbWild->m_nId, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_PEACETIME;
    }

    if (bSuccess && (ptbWild->m_nType == EN_WILD_TYPE__THRONE_NEW || ptbWild->m_nType == EN_WILD_TYPE__IDOL))
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchAttack: srcPlayer[id=%ld] cannot attack throne, just return... [seq=%u]",
            ptbSPlayer->m_nUid, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_CHANGE;
    }

    if(bSuccess)
    {
        ptbReqMarch->Set_Status(EN_MARCH_STATUS__FIGHTING);
        ptbReqMarch->Set_Btime(ptbReqMarch->m_nEtime);
        ptbReqMarch->Set_Etime(ptbReqMarch->m_nBtime + 5);
        if (ptbReqMarch->m_nEtime < CTimeUtils::GetUnixTime())
        {
            ptbReqMarch->m_nEtime = CTimeUtils::GetUnixTime();
        }
        if(ptbWild->m_nUid == 0)
        {
            CSendMessageBase::AddTips(&pstSession->m_stSourceUser, EN_TIPS_TYPE__ATTACKING, ptbSPlayer->m_nUid, TRUE,
                ptbWild->m_nId, ptbWild->m_nType, ptbWild->m_nLevel, ptbWild->m_sUname.c_str());
        }
        else
        {
            CSendMessageBase::AddTips(&pstSession->m_stSourceUser, EN_TIPS_TYPE__ATTACKING, ptbSPlayer->m_nUid, TRUE,
                ptbWild->m_nId, ptbWild->m_nType, ptbWild->m_nLevel, pstSession->m_stTargetUser.m_tbPlayer.m_sUin.c_str());
        }

        if (udwMapType == EN_WILD_CLASS_MONSTER_NEST &&
            ptbReqMarch->m_nTuid != ptbWild->m_nUid)
        {
            ptbReqMarch->Set_Tuid(ptbWild->m_nUid);
            ptbReqMarch->m_bParam[0].m_ddwTargetUserId = ptbWild->m_nUid;
            strncpy(ptbReqMarch->m_bParam[0].m_szTargetUserName, ptbWild->m_sUname.c_str(), MAX_TABLE_NAME_LEN);
            ptbReqMarch->SetFlag(TbMARCH_ACTION_FIELD_PARAM);
        }
    }
    else
    {
        CProcessReport::GenInValidReport(pstSession, EN_REPORT_TYPE_ATTACK, dwReason, &pstSession->m_tbReport);

        ptbReqMarch->Set_Status(EN_MARCH_STATUS__RETURNING);
        ptbReqMarch->Set_Btime(ptbReqMarch->m_nEtime);
        ptbReqMarch->Set_Etime(ptbReqMarch->m_nBtime + ptbReqMarch->m_nCtime);
        //ptbReqMarch->DeleteField(TbMARCH_ACTION_FIELD_TAL);
        ptbReqMarch->Set_Tal(0, UPDATE_ACTION_TYPE_DELETE);
    }

    // 更新action状态
    pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;

    return 0;
}

TINT32 CProcessAction::ProcessMarchOccupy(SSession *pstSession)
{
    TbMarch_action *ptbReqMarch = &pstSession->m_stReqMarch;
    SUserInfo *pstUser = &pstSession->m_stSourceUser;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;
    TbMap *ptbWild = &pstSession->m_stMapItem;
    TBOOL bSuccess = TRUE;
    TINT32 dwReason = EN_REPORT_RESULT_TARGET_CHANGE;

    const Json::Value& oWildResJson = CWildInfo::GetWildResInfo(ptbWild->m_nSid);
    TUINT32 udwMapType = CMapLogic::GetWildClass(ptbWild->m_nSid, ptbWild->m_nType);
    TUINT32 udwCurtime = CTimeUtils::GetUnixTime();

    // 任务合法性检查 - 资源地
    if (bSuccess && EN_WILD_CLASS_RES == udwMapType &&
        (ptbWild->m_nType != pstSession->m_stRawReqMarch.m_bParam[0].m_ddwTargetType || !CMapLogic::IsOccupyWild(pstSession->m_udwReqSvrId, ptbWild->m_nType)))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchOccupy: target[type:%ld,%u] has changed, just return... [seq=%u]",
            ptbWild->m_nType, pstSession->m_stRawReqMarch.m_bParam[0].m_ddwTargetType, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_COLLECTED_OUT;
    }
    if (bSuccess && EN_WILD_CLASS_RES == udwMapType &&
        ptbWild->m_nUid != 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchOccupy: target[uid:%ld] has been occupied, just return... [seq=%u]",
            ptbWild->m_nUid, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_HAS_OCCUPIED;
    }

    // 任务合法性检查 - 巢穴
    if (bSuccess && EN_WILD_CLASS_MONSTER_NEST == udwMapType &&
        (ptbWild->m_nExpire_time > 0 && udwCurtime >= ptbWild->m_nExpire_time))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchOccupy: wild[%ld] reach disappear time[%ld], just return... [seq=%u]",
            ptbWild->m_nId, ptbWild->m_nExpire_time, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_COLLECTED_OUT;
    }
    if (bSuccess && EN_WILD_CLASS_MONSTER_NEST == udwMapType &&
        (ptbPlayer->m_nAlid == 0))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchOccupy: user[%ld] not in alliance, just return... [seq=%u]",
            ptbPlayer->m_nUid, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_OCCUPY_LAIR_NO_AL;
    }
    if (bSuccess && EN_WILD_CLASS_MONSTER_NEST == udwMapType &&
        ptbWild->m_nOccupy_clean_flag == 1)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchOccupy: wild[%ld] occupy clean flag[%ld], just return... [seq=%u]",
            ptbWild->m_nId, ptbWild->m_nOccupy_clean_flag, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_COLLECTED_OUT;
    }
    if (bSuccess && EN_WILD_CLASS_MONSTER_NEST == udwMapType &&
        (ptbWild->m_nUid != 0 && ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET != ptbWild->m_nAlid))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchOccupy: user[%ld] not in same alliance, user_alid[%ld] wild_alid[%ld], just return... [seq=%u]",
            ptbPlayer->m_nUid, ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET, ptbWild->m_nAlid, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_OCCUPY_LAIR_WRONG_AL;
    }
    if (bSuccess && EN_WILD_CLASS_MONSTER_NEST == udwMapType &&
        ptbWild->m_nUid != 0)
    {
        for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
        {
            TbMarch_action tbAction = pstUser->m_atbMarch[udwIdx];
            if (tbAction.m_nMclass == EN_ACTION_MAIN_CLASS__MARCH && tbAction.m_nSclass == EN_ACTION_SEC_CLASS__OCCUPY &&
                tbAction.m_nSuid == ptbPlayer->m_nUid && tbAction.m_nTpos == ptbWild->m_nId && tbAction.m_nId != ptbReqMarch->m_nId &&
                (tbAction.m_nStatus == EN_MARCH_STATUS__PRE_LOADING || tbAction.m_nStatus == EN_MARCH_STATUS__LOADING))
            {
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchOccupy: user[%ld] is now loading wild[%ld], just return... [seq=%u]",
                    ptbPlayer->m_nUid, ptbWild->m_nId, pstSession->m_udwSeqNo));
                bSuccess = FALSE;
                dwReason = EN_REPORT_RESULT_TARGET_HAS_OCCUPIED;
            }
        }
    }
    if (bSuccess && EN_WILD_CLASS_MONSTER_NEST == udwMapType &&
        (ptbWild->m_nUid != 0 && ptbWild->m_nOccupy_num >= oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a0"]["a11"][(TUINT32)ptbWild->m_nLevel - 1].asUInt()))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchOccupy: wild[%ld] occupying num[%ld] limit num[%u], just return... [seq=%u]",
            ptbWild->m_nId, ptbWild->m_nOccupy_num, oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a0"]["a11"][(TUINT32)ptbWild->m_nLevel - 1].asUInt(), pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_OCCUPY_LAIR_NUM_LIMIT;
    }

    // 更新action状态
    if(bSuccess == TRUE)
    {
        if (EN_WILD_CLASS_RES == udwMapType)
        {
            ptbReqMarch->Set_Status(EN_MARCH_STATUS__PRE_LOADING);
            ptbReqMarch->Set_Btime(ptbReqMarch->m_nEtime);
            ptbReqMarch->Set_Etime(ptbReqMarch->m_nBtime + 3);
            pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;

            CCommonBase::OccupyWild(ptbPlayer, ptbWild, ptbReqMarch);
            pstSession->m_ucMapItemFlag = EN_TABLE_UPDT_FLAG__CHANGE;
        }
        else if (EN_WILD_CLASS_MONSTER_NEST == udwMapType)
        {
            ptbReqMarch->Set_Status(EN_MARCH_STATUS__PRE_LOADING);
            ptbReqMarch->Set_Btime(ptbReqMarch->m_nEtime);
            ptbReqMarch->Set_Ctime(3);
            ptbReqMarch->Set_Etime(ptbReqMarch->m_nBtime + 3);
            ptbReqMarch->Set_Sal(ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET);
            pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;

            if (ptbWild->m_nUid == 0)
            {
                CCommonBase::OccupyWild(ptbPlayer, ptbWild, ptbReqMarch);
                ptbWild->Set_Occupy_num(1);
            }
            else
            {
                CWarProcess::UpdateOccupyLairMap(pstSession, &pstSession->m_stSourceUser, ptbWild);
            }
            pstSession->m_ucMapItemFlag = EN_TABLE_UPDT_FLAG__CHANGE;
        }
        else//部队直接放回
        {
            CProcessReport::GenInValidReport(pstSession, EN_REPORT_TYPE_OCCUPY, dwReason, &pstSession->m_tbReport);

            ptbReqMarch->Set_Status(EN_MARCH_STATUS__RETURNING);
            ptbReqMarch->Set_Btime(ptbReqMarch->m_nEtime);
            ptbReqMarch->Set_Etime(ptbReqMarch->m_nBtime + ptbReqMarch->m_nCtime);
            pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;
        }
    }
    else//部队直接放回
    {
        CProcessReport::GenInValidReport(pstSession, EN_REPORT_TYPE_OCCUPY, dwReason, &pstSession->m_tbReport);

        ptbReqMarch->Set_Status(EN_MARCH_STATUS__RETURNING);
        ptbReqMarch->Set_Btime(ptbReqMarch->m_nEtime);
        ptbReqMarch->Set_Etime(ptbReqMarch->m_nBtime + ptbReqMarch->m_nCtime);
        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;
    }

    return 0;
}

TINT32 CProcessAction::ProcessMarchScout(SSession *pstSession)
{
    TbMarch_action *ptbReqMarch = &pstSession->m_stReqMarch;
    SActionMarchParam *pstParam = &ptbReqMarch->m_bParam[0];
    TbMap *ptbWild = &pstSession->m_stMapItem;
    TBOOL bSuccess = TRUE;
    TINT32 dwReason = EN_REPORT_RESULT_TARGET_CHANGE;
    SUserInfo *pstSUser = &pstSession->m_stSourceUser;
    TbDiplomacy *ptbDip = NULL;
    TbPlayer *ptbSPlayer = &pstSUser->m_tbPlayer;
    SCityInfo *pstCity = &pstSUser->m_stCityInfo;
    TbThrone *ptbThrone = &pstSession->m_tbThrone;

    if(ptbWild->m_nAlid != 0 && ptbSPlayer->m_nAlpos != 0)
    {
        ptbDip = CCommonBase::GetDiplomacy(pstSession->m_stSourceUser.m_atbDiplomacy, pstSession->m_stSourceUser.m_udwDiplomacyNum,
            ptbWild->m_nAlid);
    }
    // 任务合法性检查
    if ((ptbSPlayer->m_nStatus & EN_CITY_STATUS__NEW_PROTECTION)
        || (ptbSPlayer->m_nStatus & EN_CITY_STATUS__AVOID_WAR))
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchScout: srcPlayer[id=%ld] is in peace time, just return... [seq=%u]",
            ptbSPlayer->m_nUid, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_SELF_PEACETIME;
    }

    if (bSuccess && pstSession->m_bCheckValidFlag == FALSE)
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchScout: check_valid=%d suid[%lld] tuid[%lld], just return... [seq=%u]",
            pstSession->m_bCheckValidFlag, ptbReqMarch->m_nSuid, ptbReqMarch->m_nTuid, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_CHANGE;
    }

    if(bSuccess && ptbReqMarch->m_nTuid != ptbWild->m_nUid && ptbWild->m_nType != EN_WILD_TYPE__THRONE_NEW && ptbWild->m_nType != EN_WILD_TYPE__IDOL)
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchScout: suid[%lld] tuid[%lld], just return... [seq=%u]",
            ptbReqMarch->m_nSuid, ptbReqMarch->m_nTuid, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_CHANGE;
    }

    if(bSuccess && ptbSPlayer->m_nAlpos != 0 && ptbSPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET == ptbWild->m_nAlid)
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchScout: sal[%u]=tal[%u], just return... [seq=%u]",
            pstParam->m_ddwSourceAlliance, pstParam->m_ddwTargetAlliance, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_SAME_ALLIANCE;
    }

    if(bSuccess && ptbDip && ptbDip->m_nType == EN_DIPLOMACY_TYPE__FRIENDLY)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchScout: sal[%u] and tal[%u] is friend, just return... [seq=%u]",
            ptbSPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET, ptbWild->m_nAlid, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_IS_FRIEND;
    }

    if (bSuccess && ptbWild->m_nType != EN_WILD_TYPE__THRONE_NEW && ptbWild->m_nType != EN_WILD_TYPE__IDOL 
        && ((ptbWild->m_nStatus & EN_CITY_STATUS__AVOID_WAR) || (ptbWild->m_nStatus & EN_CITY_STATUS__NEW_PROTECTION)))
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchScout: target[%ld] is in peace time, just return... [seq=%u]",
            ptbWild->m_nId, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_PEACETIME;
    }

    if (bSuccess && ptbWild->m_nType == EN_WILD_TYPE__IDOL)
    {
        if (ptbWild->m_nStatus != EN_IDOL_STATUS__CONTEST_PERIOD)
        {
            TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchScout: idol[%ld][type:%ld] is in peace time, just return... [seq=%u]",
                ptbWild->m_nId, ptbWild->m_nType, pstSession->m_udwSeqNo));
            bSuccess = FALSE;
            dwReason = EN_REPORT_RESULT_TARGET_PEACETIME;
        }
        else
        {
            TBOOL bIsFind = FALSE;
            for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_udwIdolNum; udwIdx++)
            {
                if (pstSession->m_atbIdol[udwIdx].m_nPos == ptbWild->m_nId)
                {
                    bIsFind = TRUE;
                    break;
                }
            }
            if (bIsFind == FALSE)
            {
                TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchScout: there is no such idol on pos[%ld], just return... [seq=%u]",
                    ptbWild->m_nId, pstSession->m_udwSeqNo));
                bSuccess = FALSE;
                dwReason = EN_REPORT_RESULT_TARGET_CHANGE;
            }
        }
    }

    if (bSuccess && ptbWild->m_nType == EN_WILD_TYPE__THRONE_NEW)
    {
        if (ptbWild->m_nStatus == EN_THRONE_STATUS__PEACE_TIME)
        {
            TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchScout: throne[%ld][type:%ld] is in peace time, just return... [seq=%u]",
                ptbWild->m_nId, ptbWild->m_nType, pstSession->m_udwSeqNo));
            bSuccess = FALSE;
            dwReason = EN_REPORT_RESULT_TARGET_PEACETIME;
        }
        else if (ptbWild->m_nId != ptbThrone->m_nPos)
        {
            TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchScout: there is no such throne on pos[%ld], just return... [seq=%u]",
                ptbWild->m_nId, pstSession->m_udwSeqNo));
            bSuccess = FALSE;
            dwReason = EN_REPORT_RESULT_TARGET_CHANGE;
        }
    }

    SUserInfo *pstTUser = &pstSession->m_stTargetUser;
    if (bSuccess && ptbWild->m_nType == EN_WILD_TYPE__CITY && pstTUser->m_stPlayerBuffList[EN_BUFFER_INFO_SCOUT_PREVENT].m_astBuffDetail[EN_BUFF_TYPE_ITEM].m_dwTime > CTimeUtils::GetUnixTime())
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchScout: scout prevent[%d], just return... [seq=%u]",
            pstTUser->m_stPlayerBuffList[EN_BUFFER_INFO_SCOUT_PREVENT].m_astBuffDetail[EN_BUFF_TYPE_ITEM].m_dwTime, 
            pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_SCOUT_BE_PREVENT;
    }

    if(bSuccess)
    {
        if(ptbWild->m_nUid > 0 && ptbWild->m_nType == EN_WILD_TYPE__CITY)
        {
            CProcessReport::GenScoutCityReport(pstSession, &pstSession->m_tbReport);

            CQuestLogic::SetTaskCurValue(pstSUser, pstCity, EN_TASK_TYPE_ING_SCOUT_CITY_NUM);
        }
        else if(ptbWild->m_nType == EN_WILD_TYPE__THRONE_NEW)
        {
            if (pstSession->m_stTargetUser.m_tbAlliance.m_nAid > 0)
            {
                ptbReqMarch->m_bParam[0].m_ddwTargetUserId = pstSession->m_stTargetUser.m_tbAlliance.m_nOid;
                strncpy(ptbReqMarch->m_bParam[0].m_szTargetUserName, pstSession->m_stTargetUser.m_tbAlliance.m_sOname.c_str(), MAX_TABLE_NAME_LEN);
                ptbReqMarch->m_bParam[0].m_szTargetUserName[MAX_TABLE_NAME_LEN - 1] = 0;
                strncpy(ptbReqMarch->m_bParam[0].m_szTargetAlliance, pstSession->m_stTargetUser.m_tbAlliance.m_sName.c_str(), MAX_TABLE_NAME_LEN);
                ptbReqMarch->m_bParam[0].m_szTargetAlliance[MAX_TABLE_NAME_LEN - 1] = 0;
                strncpy(ptbReqMarch->m_bParam[0].m_szTargetAlNick, pstSession->m_stTargetUser.m_tbAlliance.m_sAl_nick_name.c_str(), MAX_TABLE_NAME_LEN);
                ptbReqMarch->m_bParam[0].m_szTargetAlNick[MAX_TABLE_NAME_LEN - 1] = 0;
            }
            ptbReqMarch->m_bParam[0].m_ddwSourceAlliance = pstSUser->m_tbAlliance.m_nAid;
            CProcessReport::GenScoutThroneReport(pstSession, &pstSession->m_tbReport);
        }
        else if (ptbWild->m_nType == EN_WILD_TYPE__IDOL)
        {
            CProcessReport::GenScoutIdolReport(pstSession, &pstSession->m_tbReport);
        }
        else
        {
            //TODO
            CProcessReport::GenScoutWildReport(pstSession, &pstSession->m_tbReport);
        }

        CSendMessageBase::AddTips(&pstSession->m_stSourceUser, EN_TIPS_TYPE__SCOUT_SUCC, pstSession->m_stSourceUser.m_tbPlayer.m_nUid, TRUE, ptbWild->m_nId);

        if(ptbWild->m_nUid)
        {
            SNoticInfo stNoticInfo;
            stNoticInfo.Reset();
            stNoticInfo.m_dwNoticId = EN_NOTI_ID__SCOUT_SUCC;
            stNoticInfo.m_strTName = pstSession->m_stTargetUser.m_tbPlayer.m_sUin;
            stNoticInfo.m_strLang = CDocument::GetLang(pstSUser->m_tbLogin.m_nLang);
            CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), pstSession->m_udwReqSvrId, ptbSPlayer->m_nUid, stNoticInfo);
            stNoticInfo.Reset();
            stNoticInfo.m_dwNoticId = EN_NOTI_ID__BE_SCOUT;
            stNoticInfo.m_strSName = ptbSPlayer->m_sUin;
            stNoticInfo.m_strLang = CDocument::GetLang(pstSession->m_stTargetUser.m_tbLogin.m_nLang);;
            CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), pstSession->m_udwReqSvrId, pstSession->m_stTargetUser.m_tbPlayer.m_nUid, stNoticInfo);

            //tips
            CSendMessageBase::AddTips(&pstSession->m_stTargetUser, EN_TIPS_TYPY__BE_SCOUTED, pstSession->m_stTargetUser.m_tbPlayer.m_nUid, TRUE, ptbWild->m_nId, ptbWild->m_nType, 0, ptbSPlayer->m_sUin.c_str());
        }

        //玩家信息统计
        pstSUser->m_tbPlayer.m_bWar_statistics[0].ddwScoutNum++;
        pstSUser->m_tbPlayer.SetFlag(TbPLAYER_FIELD_WAR_STATISTICS);
    }
    else
    {
        if (dwReason == EN_REPORT_RESULT_SCOUT_BE_PREVENT)
        {
            CSendMessageBase::AddTips(&pstSession->m_stSourceUser, EN_TIPS_TYPE__SCOUT_BE_PREVENTED, pstSession->m_stSourceUser.m_tbPlayer.m_nUid, TRUE, 
                ptbWild->m_nId, ptbWild->m_nUid, 0, pstSession->m_stTargetUser.m_tbPlayer.m_sUin.c_str());
            CSendMessageBase::AddTips(&pstSession->m_stTargetUser, EN_TIPS_TYPE__PREVENT_SCOUT, pstSession->m_stTargetUser.m_tbPlayer.m_nUid, TRUE, 
                ptbSPlayer->m_nUid, 0, 0, ptbSPlayer->m_sUin.c_str());

            SNoticInfo stNoticInfo;
            stNoticInfo.Reset();
            stNoticInfo.m_dwNoticId = EN_NOTI_ID__SCOUT_BE_PREVENTED;
            stNoticInfo.m_strTName = pstSession->m_stTargetUser.m_tbPlayer.m_sUin;
            stNoticInfo.m_strTPosX = CCommonFunc::NumToString(ptbWild->m_nId / MAP_X_Y_POS_COMPUTE_OFFSET);
            stNoticInfo.m_strTPosY = CCommonFunc::NumToString(ptbWild->m_nId % MAP_X_Y_POS_COMPUTE_OFFSET);
            stNoticInfo.m_strLang = CDocument::GetLang(pstSUser->m_tbLogin.m_nLang);
            CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), pstSession->m_udwReqSvrId, ptbSPlayer->m_nUid, stNoticInfo);
            stNoticInfo.Reset();
            stNoticInfo.m_dwNoticId = EN_NOTI_ID__PREVENT_SCOUT;
            stNoticInfo.m_strSName = ptbSPlayer->m_sUin;
            stNoticInfo.m_strLang = CDocument::GetLang(pstSession->m_stTargetUser.m_tbLogin.m_nLang);;
            CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), pstSession->m_udwReqSvrId, pstSession->m_stTargetUser.m_tbPlayer.m_nUid, stNoticInfo);
            CProcessReport::GenScoutPreventReport(pstSession, EN_REPORT_TYPE_SCOUT, dwReason, &pstSession->m_tbReport);
        }
        else
        {
            CSendMessageBase::AddTips(&pstSession->m_stSourceUser, EN_TIPS_TYPE__SCOUT_FAIL, pstSession->m_stSourceUser.m_tbPlayer.m_nUid, TRUE, ptbWild->m_nId);
            CProcessReport::GenInValidReport(pstSession, EN_REPORT_TYPE_SCOUT, dwReason, &pstSession->m_tbReport);
        }
    }

    // 更新action状态
    pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__DEL;

    return 0;
}

TINT32 CProcessAction::ProcessMarchFighting(SSession *pstSession)
{
    TbMarch_action *ptbReqMarch = &pstSession->m_stReqMarch;
    TbMap *ptbWild = &pstSession->m_stMapItem;
    TBOOL bSuccess = TRUE;
    TINT32 dwReason = EN_REPORT_RESULT_TARGET_CHANGE;
    SUserInfo *pstSUser = &pstSession->m_stSourceUser;
    TbPlayer *ptbSPlayer = &pstSUser->m_tbPlayer;
    SCityInfo *pstCity = &pstSUser->m_stCityInfo;
    TUINT32 udwWildRawUId = ptbWild->m_nUid;

    TUINT32 udwMapType = CMapLogic::GetWildClass(ptbWild->m_nSid, ptbWild->m_nType);

    // 任务合法性检查
    if (bSuccess && pstSession->m_bCheckValidFlag == FALSE)
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchAttack: check_valid=%d suid[%lld] tuid[%lld] wild_type[%ld], just return... [seq=%u]",
            pstSession->m_bCheckValidFlag, ptbReqMarch->m_nSuid, ptbReqMarch->m_nTuid, ptbWild->m_nType, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_CHANGE;
    }

    if (bSuccess && udwMapType != EN_WILD_CLASS_MONSTER_NEST
        && ptbReqMarch->m_nTuid != ptbWild->m_nUid)
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchAttack: suid[%lld] tuid[%lld] wild_type[%ld], just return... [seq=%u]",
            ptbReqMarch->m_nSuid, ptbReqMarch->m_nTuid, ptbWild->m_nType, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_CHANGE;
    }
    if (bSuccess && udwMapType == EN_WILD_CLASS_MONSTER_NEST
        && ptbReqMarch->m_nTal != ptbWild->m_nAlid)
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchAttack: attack lair: req_tal[%lld] wild_alid[%ld], just return... [seq=%u]",
            ptbReqMarch->m_nTal, ptbWild->m_nAlid, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_CHANGE;
    }

    if(bSuccess)
    {
        if (udwMapType == EN_WILD_CLASS_MONSTER_NEST &&
            ptbReqMarch->m_nTuid != ptbWild->m_nUid)
        {
            ptbReqMarch->Set_Tuid(ptbWild->m_nUid);
            ptbReqMarch->m_bParam[0].m_ddwTargetUserId = ptbWild->m_nUid;
            strncpy(ptbReqMarch->m_bParam[0].m_szTargetUserName, ptbWild->m_sUname.c_str(), MAX_TABLE_NAME_LEN);
            ptbReqMarch->SetFlag(TbMARCH_ACTION_FIELD_PARAM);
        }

        SBattleNode stDefend;
        SBattleNode stAttack;

        TUINT32 udwBeforUserHosNum = 0;
        SCityInfo *pstTCity = NULL;
        if(ptbReqMarch->m_nTuid != 0 && ptbReqMarch->m_nTpos != 0)
        {
            pstTCity = &pstSession->m_stTargetUser.m_stCityInfo;
            udwBeforUserHosNum = CCityBase::GetHosCurNum(pstTCity);
        }

        pstSession->m_dwMarchResult = CWarProcess::ProcessWar(pstSession, &stAttack, &stDefend, &pstSession->m_tbReport);

        TUINT32 udwAfterUserHosNum = CCityBase::GetHosCurNum(pstTCity);
        TUINT32 udwHosNum = udwAfterUserHosNum - udwBeforUserHosNum;

        //player info 统计 不统计支援类action的兵力情况
        CWarProcess::ComputeWarResult(pstSession, &stAttack, &stDefend, pstSession->m_dwMarchResult, udwHosNum);

        TINT32 dwReportResult = pstSession->m_dwMarchResult;

        CProcessReport::GenAttackReport(pstSession, EN_REPORT_TYPE_ATTACK, dwReportResult, &stAttack, &stDefend, &pstSession->m_tbReport);
        
        ptbReqMarch->Set_Status(EN_MARCH_STATUS__RETURNING);
        ptbReqMarch->Set_Btime(CTimeUtils::GetUnixTime());
        ptbReqMarch->Set_Etime(ptbReqMarch->m_nBtime + ptbReqMarch->m_nCtime);
        //ptbReqMarch->DeleteField(TbMARCH_ACTION_FIELD_TAL);
        ptbReqMarch->Set_Tal(0, UPDATE_ACTION_TYPE_DELETE);

        if (udwMapType == EN_WILD_CLASS_MONSTER_NEST)
        {
            CWarProcess::UpdateOccupyLairAction(pstSession, &pstSession->m_stTargetUser, ptbWild);
            CWarProcess::UpdateOccupyLairMap(pstSession, &pstSession->m_stTargetUser, ptbWild);
        }

        //设置抓龙和战果标志
        if (stAttack.m_stDragon.m_ddwCaptured >= 0 || stDefend.m_stDragon.m_ddwCaptured >= 0)
        {
            ptbReqMarch->m_bParam[0].m_ddwCaptureDragonFlag = 1;
            ptbReqMarch->m_bParam[0].m_ddwWinFlag = pstSession->m_dwMarchResult == EN_REPORT_RESULT_WIN ? 1 : 0;
            ptbReqMarch->SetFlag(TbMARCH_ACTION_FIELD_PARAM);
        }

        //Tips
        ostringstream oss;
        oss.str("");
        for(TUINT32 udwIdx = 0; udwIdx < EN_RESOURCE_TYPE__END; ++udwIdx)
        {
            if(udwIdx > 0)
            {
                oss << "|";
            }
            oss << ptbReqMarch->m_bParam[0].m_stResource[udwIdx];
        }
        oss << "|" << ptbReqMarch->m_bParam[0].m_ddwLoadGem;

        string sResourceInfo = oss.str();

        TUINT32 udwWinOrLose = pstSession->m_dwMarchResult == EN_REPORT_RESULT_WIN ? 0 : 1;
        CSendMessageBase::AddTips(&pstSession->m_stSourceUser, EN_TIPS_TYPE__WAR_RESULT, ptbSPlayer->m_nUid, TRUE,
            ptbWild->m_nId, ptbWild->m_nUid, udwWinOrLose, ptbWild->m_sUname.c_str(), sResourceInfo.c_str());

        //防守方打输了 也要弹tips
        if(udwWildRawUId)
        {
            TUINT32 udwTargetWinOrLose = pstSession->m_dwMarchResult == EN_REPORT_RESULT_LOSE ? 2 : 3;
            CSendMessageBase::AddTips(&pstSession->m_stTargetUser, EN_TIPS_TYPE__WAR_RESULT, udwWildRawUId, TRUE,
                ptbWild->m_nId, ptbSPlayer->m_nUid, udwTargetWinOrLose,
                ptbSPlayer->m_sUin.c_str(), sResourceInfo.c_str());

            for(TUINT32 udwIdx = 0; udwIdx < EN_TROOP_TYPE__END; ++udwIdx)
            {
                pstSUser->m_tbUserStat.m_nKill_troop_num += ptbReqMarch->m_bParam[0].m_stTroopRaw[udwIdx] - ptbReqMarch->m_bParam[0].m_stTroop[udwIdx];
            }
            pstSUser->m_tbUserStat.SetFlag(TbUSER_STAT_FIELD_KILL_TROOP_NUM);

            SNoticInfo stNoticInfo;
            if(pstSession->m_dwMarchResult == EN_REPORT_RESULT_WIN)
            {
                //推送
                stNoticInfo.Reset();
                stNoticInfo.m_dwNoticId = EN_NOTI_ID__ATTACK_SUCC;
                stNoticInfo.m_strTName = pstSession->m_stTargetUser.m_tbPlayer.m_sUin;
                stNoticInfo.m_strLang = CDocument::GetLang(pstSUser->m_tbLogin.m_nLang);;
                CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), ptbSPlayer->m_nSid, ptbSPlayer->m_nUid, stNoticInfo);
                stNoticInfo.Reset();
                stNoticInfo.m_dwNoticId = EN_NOTI_ID__DEFENSE_FAIL;
                stNoticInfo.m_strSName = ptbSPlayer->m_sUin;
                stNoticInfo.m_strLang = CDocument::GetLang(pstSession->m_stTargetUser.m_tbLogin.m_nLang);;
                CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), pstSession->m_stTargetUser.m_tbPlayer.m_nSid, pstSession->m_stTargetUser.m_tbPlayer.m_nUid, stNoticInfo);

                //冒烟和冒火逻辑
                if(ptbWild->m_nType == EN_WILD_TYPE__CITY)
                {
                    CQuestLogic::SetTaskCurValue(pstSUser, pstCity, EN_TASK_TYPE_ING_ATTACK_CITY_NUM);

                    CWarProcess::GenMoveCityAction(pstSession, ptbWild, &stDefend);
                    pstSession->m_stTargetUser.m_tbPlayer.Set_Status(pstSession->m_stTargetUser.m_tbPlayer.m_nStatus | EN_CITY_STATUS__BEEN_ATTACK);

                    ptbWild->Set_Smoke_end_time(CTimeUtils::GetUnixTime() + 30 * 60);
                    ptbWild->Set_Status(pstSession->m_stTargetUser.m_tbPlayer.Get_Status());
                    pstSession->m_ucMapItemFlag = EN_TABLE_UPDT_FLAG__CHANGE;
                } 
            }
            else
            {
                stNoticInfo.Reset();
                stNoticInfo.m_dwNoticId = EN_NOTI_ID__ATTACK_FAIL;
                stNoticInfo.m_strTName = pstSession->m_stTargetUser.m_tbPlayer.m_sUin;
                stNoticInfo.m_strLang = CDocument::GetLang(pstSUser->m_tbLogin.m_nLang);
                CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), ptbSPlayer->m_nSid, ptbSPlayer->m_nUid, stNoticInfo);
                stNoticInfo.Reset();
                stNoticInfo.m_dwNoticId = EN_NOTI_ID__DEFENSE_SUCC;
                stNoticInfo.m_strSName = ptbSPlayer->m_sUin;
                stNoticInfo.m_strLang = CDocument::GetLang(pstSession->m_stTargetUser.m_tbLogin.m_nLang);
                CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), pstSession->m_stTargetUser.m_tbPlayer.m_nSid, pstSession->m_stTargetUser.m_tbPlayer.m_nUid, stNoticInfo);

                //冒烟和冒火逻辑
                if(ptbWild->m_nType == EN_WILD_TYPE__CITY)
                {
                    CQuestLogic::SetTaskCurValue(pstSUser, pstCity, EN_TASK_TYPE_ING_ATTACK_CITY_NUM);

                    pstSession->m_stTargetUser.m_tbPlayer.Set_Status(pstSession->m_stTargetUser.m_tbPlayer.m_nStatus | EN_CITY_STATUS__BEEN_ATTACK);

                    ptbWild->Set_Smoke_end_time(CTimeUtils::GetUnixTime() + 30 * 60);
                    ptbWild->Set_Status(pstSession->m_stTargetUser.m_tbPlayer.Get_Status());
                    pstSession->m_ucMapItemFlag = EN_TABLE_UPDT_FLAG__CHANGE;
                }
            }

            CMsgBase::SendEncourageMail(&pstSession->m_stTargetUser.m_tbUserStat, pstSession->m_udwReqSvrId, EN_MAIL_ID__FIRST_BE_ATK);

            if (stAttack.m_stDragon.m_ddwCaptured >= 0)
            {
                CMsgBase::SendEncourageMail(&pstSession->m_stTargetUser.m_tbUserStat, pstSession->m_udwReqSvrId, EN_MAIL_ID__FIRST_CAPTURE_HERO);
            }
            else if (stDefend.m_stDragon.m_ddwCaptured >= 0)
            {
                CMsgBase::SendEncourageMail(&pstSession->m_stSourceUser.m_tbUserStat, pstSession->m_udwReqSvrId, EN_MAIL_ID__FIRST_CAPTURE_HERO);
            }
        }
    }
    else
    {
        CProcessReport::GenInValidReport(pstSession, EN_REPORT_TYPE_ATTACK, dwReason, &pstSession->m_tbReport);

        ptbReqMarch->Set_Status(EN_MARCH_STATUS__RETURNING);
        ptbReqMarch->Set_Btime(ptbReqMarch->m_nEtime);
        ptbReqMarch->Set_Etime(ptbReqMarch->m_nBtime + ptbReqMarch->m_nCtime);
        //ptbReqMarch->DeleteField(TbMARCH_ACTION_FIELD_TAL);
        ptbReqMarch->Set_Tal(0, UPDATE_ACTION_TYPE_DELETE);
    }

    TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessMarchAttack: suid[%lld] tuid[%lld] wild_type[%ld] [user_status=%u wild_id=%u wild_status=%u] [seq=%u]",
        ptbReqMarch->m_nSuid, ptbReqMarch->m_nTuid, ptbWild->m_nType, pstSession->m_stTargetUser.m_tbPlayer.m_nStatus, ptbWild->m_nId,ptbWild->m_nStatus, pstSession->m_udwSeqNo));

    // 更新action状态
    pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;

    return 0;
}

TINT32 CProcessAction::ProcessMarchLoadingRes(SSession *pstSession)
{
    TbMarch_action *ptbReqMarch = &pstSession->m_stReqMarch;
    TbMap *ptbWild = &pstSession->m_stMapItem;
    SUserInfo *pstUser = &pstSession->m_stSourceUser;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;

    TUINT32 udwMapType = CMapLogic::GetWildClass(ptbWild->m_nSid, ptbWild->m_nType);
    TUINT32 udwCurtime = CTimeUtils::GetUnixTime();

    if(EN_COMMAND_STEP__INIT == pstSession->m_udwCommandStep)
    {
        TSE_LOG_INFO(CGameInfo::GetInstance()->m_poLog, ("ProcessMarchLoadingRes: beg: reward_left[%ld] [seq=%u]",
                                                          ptbWild->m_nReward_left, pstSession->m_udwSeqNo));

        SPlayerBuffInfo stOccupyBuff;
        if (ptbReqMarch->m_bParam[0].m_stDragon.m_ddwLevel != 0)
        {
            CBufferBase::GenOccupyBuff(&pstUser->m_stPlayerBuffList, &stOccupyBuff);
        }
        else
        {
            CBufferBase::GenOccupyBuff(&pstUser->m_stBuffWithoutDragon, &stOccupyBuff);
        }
        CBufferBase::SetOccupyBuff(&stOccupyBuff, ptbReqMarch);

        if (ptbWild->m_nOccupy_clean_flag == 0 && CMapLogic::HaveCollectedOut(pstUser, ptbReqMarch, ptbWild))
        {
            ptbWild->Set_Occupy_clean_flag(1);
        }

        TINT64 ddwLoadNum = 0;
        CMapLogic::GetCollectedReward(pstUser, ptbReqMarch, ptbWild, ptbReqMarch->m_bParam[0].m_ddwBeginLoadTime, \
                                      ptbReqMarch->m_bParam[0].m_ddwTotalLoadTime, ddwLoadNum);    

        TSE_LOG_INFO(CGameInfo::GetInstance()->m_poLog, ("ProcessMarchLoadingRes: [sid=%u uid=%lld action_id=%ld pos=%u] action_info[second_class=%u status=%u,begin_time=%u,end_time=%u,cost_time=%u] map_info[type=%u lv=%u reward_left=%u] result_info[cur_time=%u begin_load_time=%u total_load_time=%u load_time=%u load_num=%u load_rate=%u] [seq=%u]",
                                                          ptbWild->m_nSid, pstUser->m_tbPlayer.m_nUid, ptbReqMarch->m_nId,
                                                          ptbWild->m_nId, ptbReqMarch->m_nSclass, ptbReqMarch->m_nStatus,
                                                          ptbReqMarch->m_nBtime, ptbReqMarch->m_nEtime, ptbReqMarch->m_nCtime,
                                                          ptbWild->m_nType, ptbWild->m_nLevel, ptbWild->m_nReward_left,
                                                          CTimeUtils::GetUnixTime(),
                                                          ptbReqMarch->m_bParam[0].m_ddwBeginLoadTime,
                                                          ptbReqMarch->m_bParam[0].m_ddwTotalLoadTime,
                                                          ptbReqMarch->m_bParam[0].m_ddwLoadTime,
                                                          ptbReqMarch->m_bParam[0].m_ddwLoadResTotalNum,
                                                          ptbReqMarch->m_bParam[0].m_ddwLoadRate,
                                                          pstSession->m_udwSeqNo));

        TBOOL bIsRequestDataCenter = FALSE;

        if(EN_WILD_CLASS_RES == udwMapType
            || EN_WILD_CLASS_MONSTER_NEST == udwMapType)
        {
            bIsRequestDataCenter = TRUE;  
        }
        else
        {
            bIsRequestDataCenter = FALSE;  
        }

        if(TRUE == bIsRequestDataCenter)
        {
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__DATA_CENTER;
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;


            pstSession->ResetDataCenterReq();
            DataCenterReqInfo* pstReq = new DataCenterReqInfo;
            pstReq->m_udwType = EN_REFRESH_DATA_TYPE__OCCUPY;
            Json::Value rDataReqJson = Json::Value(Json::objectValue);

            rDataReqJson["sid"] = pstUser->m_tbLogin.m_nSid;
            rDataReqJson["uid"] = pstUser->m_tbPlayer.m_nUid;
            rDataReqJson["castle_lv"] = CCityBase::GetBuildingLevelByFuncType(&pstUser->m_stCityInfo, EN_BUILDING_TYPE__CASTLE);
            rDataReqJson["request"] = Json::Value(Json::objectValue);
            rDataReqJson["request"]["wild_id"] = ptbWild->m_nType;
            rDataReqJson["request"]["wild_lv"] = ptbWild->m_nLevel;
            rDataReqJson["request"]["load_num"] = ddwLoadNum;
            rDataReqJson["request"]["flag"] = ptbWild->m_nOccupy_clean_flag > 0 ? 1 : 0;
            rDataReqJson["request"]["res_id"] = 0;
            for (TUINT32 udwIdx = 0; udwIdx < EN_RESOURCE_TYPE__END; ++udwIdx)
            {
                if (ptbReqMarch->m_bParam[0].m_stResource[udwIdx] > 0)
                {
                    rDataReqJson["request"]["res_id"] = udwIdx;
                }
            }

            Json::FastWriter rEventWriter;
            pstReq->m_sReqContent = rEventWriter.write(rDataReqJson);
            pstSession->m_vecDataCenterReq.push_back(pstReq);
            
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessMarchLoadingRes: data center req: [ExpectProcedure=%ld] [type=%u] [uid=%ld] [seq=%u] [json=%s]", \
                                                    pstSession->m_udwExpectProcedure, pstReq->m_udwType, pstUser->m_tbPlayer.m_nUid,
                                                    pstSession->m_udwSeqNo, pstReq->m_sReqContent.c_str()));

            // return 0;
            TINT32 dwRetCode = SendDataCenterRequest(pstSession, EN_SERVICE_TYPE_DATA_CENTER_REQ);
            if(0 == dwRetCode)
            {
                return 0;
            }
            else
            {
                pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchLoadingRes: send request to data center failed. [json=%s] [ret=%d] [seq=%u]", \
                                                        pstReq->m_sReqContent.c_str(), \
                                                        dwRetCode, \
                                                        pstUser->m_udwBSeqNo));
            }
        }
        else
        {
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        } 
    }
    
    if(EN_COMMAND_STEP__1 == pstSession->m_udwCommandStep)
    {
        SRefreshData stRefreshData;
        vector<DataCenterRspInfo*>& vecRsp = pstSession->m_vecDataCenterRsp;
        DataCenterRspInfo *pstDataCenterRsp = NULL;
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessMarchLoadingRes: data center rsp size: [size=%d] [seq=%u]",
                                                vecRsp.size(), \
                                                pstUser->m_udwBSeqNo))
        
        if(vecRsp.size() > 0)
        {
            stRefreshData.Reset();
            Json::Reader jsonReader;
            Json::Value oRspDataJson;
            for (TUINT32 udwIdx = 0; udwIdx < vecRsp.size(); ++udwIdx)
            {
                pstDataCenterRsp = vecRsp[udwIdx];
                if(EN_REFRESH_DATA_TYPE__OCCUPY == pstDataCenterRsp->m_udwType)
                {
                    TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessMarchLoadingRes: data center get rsp: [json=%s] [uid=%ld][seq=%u]",
                                                            pstDataCenterRsp->m_sRspJson.c_str(), \
                                                            pstUser->m_tbPlayer.m_nUid, \
                                                            pstUser->m_udwBSeqNo));
                    if (FALSE == jsonReader.parse(pstDataCenterRsp->m_sRspJson, oRspDataJson))
                    {
                        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_DATA_CENTER_PACKAGE_ERR;
                        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchLoadingRes: prase rsp from data center failed. [seq=%u]", \
                                                                pstUser->m_udwBSeqNo));
                        return -2;
                    }
                    TINT32 dwRetCode = stRefreshData.m_stOccupyRsp.setVal(oRspDataJson);
                    if (0 != dwRetCode)
                    {
                        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__DATA_CENTER_PACKAGE_FORMAT_ERR;
                        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchLoadingRes: response data format error. [ret=%d][seq=%u]", \
                                                                dwRetCode, \
                                                                pstUser->m_udwBSeqNo));
                        return -3;
                    }
                    break;
                }
            }


            if(MAX_REWARD_ITEM_NUM < stRefreshData.m_stOccupyRsp.m_vecReward.size()
                || 0 >= stRefreshData.m_stOccupyRsp.m_vecReward.size())
            {
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchLoadingRes: occupy reward size is over. [size=%ld] [seq=%u]", \
                                                        stRefreshData.m_stOccupyRsp.m_vecReward.size(), \
                                                        pstUser->m_udwBSeqNo));
            }



            for(TUINT32 udwIdx = 0; udwIdx < stRefreshData.m_stOccupyRsp.m_vecReward.size(); ++udwIdx)
            {
                SOneGlobalRes *pstOneGlobalRes = stRefreshData.m_stOccupyRsp.m_vecReward[udwIdx];

                ptbReqMarch->m_bReward[0].aRewardList[ptbReqMarch->m_bReward[0].ddwTotalNum].ddwType = pstOneGlobalRes->ddwType;
                ptbReqMarch->m_bReward[0].aRewardList[ptbReqMarch->m_bReward[0].ddwTotalNum].ddwId = pstOneGlobalRes->ddwId;
                ptbReqMarch->m_bReward[0].aRewardList[ptbReqMarch->m_bReward[0].ddwTotalNum].ddwNum = pstOneGlobalRes->ddwNum;
                ++ptbReqMarch->m_bReward[0].ddwTotalNum;
                ptbReqMarch->SetFlag(TbMARCH_ACTION_FIELD_REWARD);
            }
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        }
        else
        {
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        }
    }
   

    if(EN_COMMAND_STEP__2 == pstSession->m_udwCommandStep)
    {
        TSE_LOG_INFO(CGameInfo::GetInstance()->m_poLog, ("ProcessMarchLoadingRes: btime[%ld] ctime[%ld] etime[%ld] load_rate[%ld], load_time[%ld] [seq=%u]",
            ptbReqMarch->m_nBtime, ptbReqMarch->m_nCtime, ptbReqMarch->m_nEtime, ptbReqMarch->m_bParam[0].m_ddwLoadRate, ptbReqMarch->m_bParam[0].m_ddwLoadTime,
            pstSession->m_udwSeqNo));


        //active quest ||task
        for(TUINT32 udwIdx = 0; udwIdx < EN_RESOURCE_TYPE__END; ++udwIdx)
        {
            switch(udwIdx)
            {
            case EN_RESOURCE_TYPE__GOLD:
                pstUser->m_tbPlayer.m_bGain_resource[0].m_addwNum[EN_RESOURCE_TYPE__GOLD] += ptbReqMarch->m_bParam[0].m_stResource[udwIdx];
                break;
            case EN_RESOURCE_TYPE__FOOD:
                pstUser->m_tbPlayer.m_bGain_resource[0].m_addwNum[EN_RESOURCE_TYPE__FOOD] += ptbReqMarch->m_bParam[0].m_stResource[udwIdx];
                break;
            case EN_RESOURCE_TYPE__WOOD:
                pstUser->m_tbPlayer.m_bGain_resource[0].m_addwNum[EN_RESOURCE_TYPE__WOOD] += ptbReqMarch->m_bParam[0].m_stResource[udwIdx];
                break;
            case EN_RESOURCE_TYPE__STONE:
                pstUser->m_tbPlayer.m_bGain_resource[0].m_addwNum[EN_RESOURCE_TYPE__STONE] += ptbReqMarch->m_bParam[0].m_stResource[udwIdx];
                break;
            case EN_RESOURCE_TYPE__ORE:
                pstUser->m_tbPlayer.m_bGain_resource[0].m_addwNum[EN_RESOURCE_TYPE__ORE] += ptbReqMarch->m_bParam[0].m_stResource[udwIdx];
                break;
            }
            CQuestLogic::SetTaskCurValue(pstUser, pstCity, EN_TASK_TYPE_ING_GET_RESOURCE, ptbReqMarch->m_bParam[0].m_stResource[udwIdx]);
            pstUser->m_tbPlayer.SetFlag(TbPLAYER_FIELD_GAIN_RESOURCE);
        }

        if(ptbReqMarch->m_nIs_recalled == FALSE)
        {
            //tips
            CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPY__LOAD_FINISHED, pstUser->m_tbPlayer.m_nUid, TRUE, ptbWild->m_nType, 0, 0);
        }

        CProcessReport::GenOccupyReport(pstSession, &pstSession->m_tbReport);

        SNoticInfo stNoticInfo;
        stNoticInfo.Reset();
        stNoticInfo.m_dwNoticId = EN_NOTI_ID__LOAD_SUCC;
        stNoticInfo.m_strLang = CDocument::GetLang(pstUser->m_tbLogin.m_nLang);
        CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), pstSession->m_udwReqSvrId, ptbReqMarch->m_nSuid, stNoticInfo);

        ptbReqMarch->Set_Status(EN_MARCH_STATUS__RETURNING);
        if (EN_WILD_CLASS_MONSTER_NEST == udwMapType)
        {
            CWarProcess::UpdateOccupyLairAction(pstSession, &pstSession->m_stTargetUser, ptbWild);
            CWarProcess::UpdateOccupyLairMap(pstSession, &pstSession->m_stTargetUser, ptbWild);

            if (ptbWild->m_nOccupy_clean_flag == 1 && ptbWild->m_nOccupy_num == 0)
            {
                CActionBase::UpdtPassiveActionWhenAbandonWild(pstUser, ptbWild);
                CMapBase::ResetMap(ptbWild);
            }
            pstSession->m_ucMapItemFlag = EN_TABLE_UPDT_FLAG__CHANGE;
        }
        else if (EN_WILD_CLASS_RES == udwMapType)
        {
            CActionBase::UpdtPassiveActionWhenAbandonWild(pstUser, ptbWild);
            CCommonBase::AbandonWild(pstCity, ptbWild);
            if (ptbWild->m_nReward_left == 0)
            {
                CMapBase::ResetMap(ptbWild);
            }
            pstSession->m_ucMapItemFlag = EN_TABLE_UPDT_FLAG__CHANGE;
        }

        ptbReqMarch->Set_Btime(ptbReqMarch->m_nEtime < udwCurtime ? ptbReqMarch->m_nEtime : udwCurtime);
        ptbReqMarch->Set_Ctime(ptbReqMarch->m_bParam[0].m_ddwMarchingTime);
        ptbReqMarch->Set_Etime(ptbReqMarch->m_nBtime + ptbReqMarch->m_nCtime);
        //ptbReqMarch->DeleteField(TbMARCH_ACTION_FIELD_SAL);
        ptbReqMarch->Set_Sal(0, UPDATE_ACTION_TYPE_DELETE);
        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;

        return 0;
    }

    return 0;
}

TINT32 CProcessAction::ProcessMarchSetupCamp(SSession *pstSession)
{
    TbMarch_action *ptbReqMarch = &pstSession->m_stReqMarch;
    SActionMarchParam *pstParam = &ptbReqMarch->m_bParam[0];
    SUserInfo *pstSUser = &pstSession->m_stSourceUser;
    TbPlayer *ptbSPlayer = &pstSUser->m_tbPlayer;
    SUserInfo *pstTUser = &pstSession->m_stTargetUser;
    TbPlayer *ptbTPlayer = &pstTUser->m_tbPlayer;
    SCityInfo *pstTCity = NULL;
    TbMap *ptbWild = &pstSession->m_stMapItem;

    TBOOL bSuccess = TRUE;
    TINT32 dwReason = EN_REPORT_RESULT_TARGET_CHANGE;
    TUINT64 uddwShareMight = 0;

    SCityInfo *pstCity = &pstSUser->m_stCityInfo;

    // 任务合法性检查
    if(bSuccess && (ptbWild->m_nType != EN_WILD_TYPE__CITY || ptbWild->m_nUid != ptbReqMarch->m_nTuid || pstSession->m_bCheckValidFlag == FALSE))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchSetupCamp: check_valid=%d target[type:%ld] not city or target[uid:%ld,%ld] has changed, just return... [seq=%u]",
            pstSession->m_bCheckValidFlag, ptbWild->m_nType, ptbReqMarch->m_nTuid, ptbWild->m_nUid, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_CHANGE;
    }
    if(pstTUser->m_stCityInfo.m_stTblData.m_nPos == ptbReqMarch->m_nTpos)
    {
        pstTCity = &pstTUser->m_stCityInfo;
    }    
    if(bSuccess && pstTCity == NULL)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchSetupCamp: no such city[%lld], just return... [seq=%u]",
            ptbReqMarch->m_nTpos, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_CHANGE;
    }
    if(bSuccess && (ptbSPlayer->m_nAlpos == 0 || ptbSPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET != ptbWild->m_nAlid))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchSetupCamp: target[salpos:%ld,sal:%ld,tal:%ld] not right, just return... [seq=%u]",
            ptbSPlayer->m_nAlpos, ptbSPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET, ptbWild->m_nAlid, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_DIFF_ALLIANCE;
    }

    if (ptbWild->m_nType == EN_WILD_TYPE__CITY)
    {
        pstTCity = &pstTUser->m_stCityInfo;
        if (bSuccess && pstTCity->m_stTblData.m_nPos == 0)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchReinForce: no such city[%lld], just return... [seq=%u]",
                ptbReqMarch->m_nTpos, pstSession->m_udwSeqNo));
            bSuccess = FALSE;
            dwReason = EN_REPORT_RESULT_TARGET_CHANGE;
        }
        if (bSuccess && CCityBase::GetBuildingLevelByFuncType(pstTCity, EN_BUILDING_TYPE__EMBASSY) <= 0)
        {
            bSuccess = FALSE;
            dwReason = EN_REPORT_RESULT_NO_EMBASSY;
        }

        TUINT32 udwReinforceMarchLimit = CCommonBase::GetReinforceMarchLimit(pstTUser);
        TUINT32 udwReinforceTroopLimit = CCommonBase::GetReinforceTroopLimit(pstTUser);
        TUINT32 udwReinforceMarch = 0;
        TUINT32 udwReinforceTroop = 0;
        for (TUINT32 udwIdx = 0; udwIdx < pstTUser->m_udwPassiveMarchNum; ++udwIdx)
        {
            if (pstTUser->m_aucPassiveMarchFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
            {
                continue;
            }

            if (pstTUser->m_atbPassiveMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_NORMAL
                && pstTUser->m_atbPassiveMarch[udwIdx].m_nStatus == EN_MARCH_STATUS__DEFENDING
                && pstTUser->m_atbPassiveMarch[udwIdx].m_nTpos == pstTCity->m_stTblData.m_nPos)
            {
                udwReinforceTroop += CCommonBase::GetMarchTroopNum(&pstTUser->m_atbPassiveMarch[udwIdx]);
                udwReinforceMarch++;
            }
        }

        udwReinforceTroop += CCommonBase::GetMarchTroopNum(ptbReqMarch);
        udwReinforceMarch++;

        if (bSuccess && udwReinforceMarch > udwReinforceMarchLimit)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchReinForce: reinforce march num exceed[max:%u,cur:%u], just return... [seq=%u]",
                udwReinforceMarchLimit, udwReinforceMarch, pstSession->m_udwSeqNo));
            bSuccess = FALSE;
            dwReason = EN_REPORT_RESULT_TARGET_REINFORCE_FULL;
        }
        if (bSuccess && udwReinforceTroopLimit > 0 && udwReinforceTroop > udwReinforceTroopLimit)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchReinForce: reinforce troop num exceed[max:%u,cur:%u], just return... [seq=%u]",
                udwReinforceTroopLimit, udwReinforceTroop, pstSession->m_udwSeqNo));
            bSuccess = FALSE;
            dwReason = EN_REPORT_RESULT_TARGET_REINFORCE_FULL;
        }
    }

    // 更新action状态, 将部队设置为encamp
    if(bSuccess == TRUE)
    {
        ptbReqMarch->Set_Status(EN_MARCH_STATUS__DEFENDING);
        ptbReqMarch->Set_Btime(CTimeUtils::GetUnixTime());
        ptbReqMarch->Set_Etime(INT64_MAX);
        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;

        CProcessReport::GenReinforceReport(pstSession, &pstSession->m_tbReport);

        // 增加贡献度
        uddwShareMight = CCityBase::CalcTroopMight(pstParam->m_stTroop.m_addwNum);
        ptbSPlayer->Set_Sh_might(ptbSPlayer->m_nSh_might + uddwShareMight);

        CSendMessageBase::AddTips(&pstSession->m_stSourceUser, EN_TIPS_TYPE__REINFORCE_NORMAL_OK, ptbSPlayer->m_nUid, TRUE,
            pstSession->m_stTargetUser.m_stCityInfo.m_stTblData.m_nPos, ptbTPlayer->m_nUid, 0, ptbTPlayer->m_sUin.c_str());
        CSendMessageBase::AddTips(&pstSession->m_stTargetUser, EN_TIPS_TYPE__BE_REINFORCE_NORMAL_OK, ptbTPlayer->m_nUid, TRUE,
            pstSession->m_stSourceUser.m_stCityInfo.m_stTblData.m_nPos, ptbSPlayer->m_nUid, 0, ptbSPlayer->m_sUin.c_str());

        SNoticInfo stNoticInfo;
        stNoticInfo.Reset();
        stNoticInfo.m_dwNoticId = EN_NOTI_ID__REINFORCE_SUCC;
        stNoticInfo.m_strTName = pstSession->m_stTargetUser.m_tbPlayer.m_sUin;
        stNoticInfo.m_strLang = CDocument::GetLang(pstSUser->m_tbLogin.m_nLang);
        CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), ptbSPlayer->m_nSid, ptbSPlayer->m_nUid, stNoticInfo);
        stNoticInfo.Reset();
        stNoticInfo.m_dwNoticId = EN_NOTI_ID__BE_REINFORCE_SUCC;
        stNoticInfo.m_strSName = ptbSPlayer->m_sUin;
        stNoticInfo.m_strLang = CDocument::GetLang(pstSession->m_stTargetUser.m_tbLogin.m_nLang);
        CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), pstSession->m_stTargetUser.m_tbPlayer.m_nSid, pstSession->m_stTargetUser.m_tbPlayer.m_nUid, stNoticInfo);

        CQuestLogic::SetTaskCurValue(pstSUser, pstCity, EN_TASK_TYPE_ING_REINFORCE_NUM);

        for(TUINT32 udwIdx = 0; udwIdx < EN_TROOP_TYPE__END; ++udwIdx)
        {
            pstSUser->m_tbUserStat.m_nReinforce_num += pstParam->m_stTroop.m_addwNum[udwIdx];
        }

        CMsgBase::SendEncourageMail(&pstSUser->m_tbUserStat, pstSUser->m_tbPlayer.m_nSid, EN_MAIL_ID__FIRST_REINFORCE);
    }
    else//部队直接放回
    {
        CProcessReport::GenInValidReport(pstSession, EN_REPORT_TYPE_REINFORCE, dwReason, &pstSession->m_tbReport);

        ptbReqMarch->Set_Status(EN_MARCH_STATUS__RETURNING);
        ptbReqMarch->Set_Btime(ptbReqMarch->m_nEtime);
        ptbReqMarch->Set_Etime(ptbReqMarch->m_nBtime + ptbReqMarch->m_nCtime);
        //ptbReqMarch->DeleteField(TbMARCH_ACTION_FIELD_TAL);
        ptbReqMarch->Set_Tal(0, UPDATE_ACTION_TYPE_DELETE);
        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;
    }

    return 0;
}

TINT32 CProcessAction::ProcessMarchCampSetupCampPeace(SSession *pstSession)
{
    TbMarch_action *ptbReqMarch = &pstSession->m_stReqMarch;
    SUserInfo *pstUser = &pstSession->m_stSourceUser;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;
    TbMap *ptbWild = &pstSession->m_stMapItem;

    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    TBOOL bSuccess = TRUE;
    TINT32 dwReason = EN_REPORT_RESULT_TARGET_CHANGE;

    if (bSuccess && ptbWild->m_nType != EN_WILD_TYPE__NORMAL)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchCamp: target[%ld] type[%ld] is not normal_type, just return... [seq=%u]",
            ptbWild->m_nId, ptbWild->m_nType, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_WILD_NOT_NORMAL;
    }
    if (bSuccess && ptbWild->m_nUid != 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchCamp: target[%ld] is not camp by user[%ld], get by new user[%ld], just return... [seq=%u]",
            ptbWild->m_nId, ptbPlayer->m_nUid, ptbWild->m_nUid, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_WILD_NOT_NORMAL;
    }

    // 更新action状态
    if (bSuccess == TRUE)
    {
        TUINT32 udwPeaceEtime = udwCurTime + CCommonBase::GetGameBasicVal(EN_GAME_BASIC_CAMP_PROTECT_TIME);
        ptbReqMarch->Set_Status(EN_MARCH_STATUS__CAMPING_WITH_PEACETIME);
        ptbReqMarch->Set_Btime(udwCurTime);
        ptbReqMarch->Set_Etime(udwPeaceEtime);
        ptbReqMarch->Set_Ctime(ptbReqMarch->m_nEtime - ptbReqMarch->m_nBtime);
        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;

        CProcessReport::GenCampReport(pstSession, &pstSession->m_tbReport);

        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessMarchCampSetupCampPeace: mapItem[%ld] uid[%ld] type[%ld] [seq=%u]",
            ptbWild->m_nId, ptbWild->m_nUid, ptbWild->m_nType, pstSession->m_udwSeqNo));
        CCommonBase::CampWild(ptbPlayer, ptbWild, ptbReqMarch);
        ptbWild->Set_Status(ptbWild->m_nStatus | EN_CITY_STATUS__AVOID_WAR);
        ptbWild->Set_Time_end(udwPeaceEtime);
        pstSession->m_ucMapItemFlag = EN_TABLE_UPDT_FLAG__CHANGE;
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessMarchCampSetupCampPeace: set camp to map[%ld] uid[%ld] [seq=%u]",
            ptbWild->m_nId, ptbWild->m_nUid, pstSession->m_udwSeqNo));

        CSendMessageBase::AddTips(&pstSession->m_stSourceUser, EN_TIPS_TYPE__CAMP_SUCC, ptbPlayer->m_nUid, TRUE);
    }
    else
    {
        CProcessReport::GenInValidReport(pstSession, EN_REPORT_TYPE_CAMP, dwReason, &pstSession->m_tbReport);

        ptbReqMarch->Set_Status(EN_MARCH_STATUS__RETURNING);
        ptbReqMarch->Set_Btime(ptbReqMarch->m_nEtime);
        ptbReqMarch->Set_Etime(ptbReqMarch->m_nBtime + ptbReqMarch->m_bParam[0].m_ddwMarchingTime);
        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;
    }

    return 0;
}

TINT32 CProcessAction::ProcessMarchCampSetupCampNormal(SSession *pstSession)
{
    TbMarch_action *ptbReqMarch = &pstSession->m_stReqMarch;
    SUserInfo *pstUser = &pstSession->m_stSourceUser;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;
    TbMap *ptbWild = &pstSession->m_stMapItem;

    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();

    if (ptbReqMarch->m_nIs_recalled == 0)
    {
        ptbReqMarch->Set_Status(EN_MARCH_STATUS__CAMPING_NORMAL);
        ptbReqMarch->Set_Btime(ptbReqMarch->m_nEtime);
        ptbReqMarch->Set_Etime(INT64_MAX);
        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;

        ptbWild->Set_Status(ptbWild->m_nStatus & (~EN_CITY_STATUS__AVOID_WAR));
        ptbWild->Set_Time_end(udwCurTime);
        pstSession->m_ucMapItemFlag = EN_TABLE_UPDT_FLAG__CHANGE;

        //推送
        SNoticInfo stNoticInfo;
        stNoticInfo.Reset();
        stNoticInfo.m_dwNoticId = EN_NOTI_ID__CAMP_PROTECT_EXPIRE;
        stNoticInfo.m_strLang = CDocument::GetLang(pstUser->m_tbLogin.m_nLang);
        stNoticInfo.m_strTPosX = CCommonFunc::NumToString(ptbReqMarch->m_nTpos / MAP_X_Y_POS_COMPUTE_OFFSET);
        stNoticInfo.m_strTPosY = CCommonFunc::NumToString(ptbReqMarch->m_nTpos % MAP_X_Y_POS_COMPUTE_OFFSET);;
        CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), ptbPlayer->m_nSid, ptbPlayer->m_nUid, stNoticInfo);
    }
    else
    {
        ptbReqMarch->Set_Status(EN_MARCH_STATUS__RETURNING);
        ptbReqMarch->Set_Btime(ptbReqMarch->m_nEtime);
        ptbReqMarch->Set_Ctime(ptbReqMarch->m_bParam[0].m_ddwMarchingTime);
        ptbReqMarch->Set_Etime(ptbReqMarch->m_nBtime + ptbReqMarch->m_nCtime);
        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;

        ptbWild->Set_Status(ptbWild->m_nStatus & (~EN_CITY_STATUS__AVOID_WAR));
        ptbWild->Set_Time_end(udwCurTime);
        CCommonBase::AbandonWild(pstCity, ptbWild);
        CActionBase::UpdtPassiveActionWhenAbandonWild(pstUser, ptbWild);
        pstSession->m_ucMapItemFlag = EN_TABLE_UPDT_FLAG__CHANGE;
    }

    return 0;
}

TINT32 CProcessAction::ProcessMarchCampSetupCampReturn(SSession *pstSession)
{
    TbMarch_action *ptbReqMarch = &pstSession->m_stReqMarch;
    SUserInfo *pstUser = &pstSession->m_stSourceUser;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TbMap *ptbWild = &pstSession->m_stMapItem;

    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();

    ptbReqMarch->Set_Status(EN_MARCH_STATUS__RETURNING);
    ptbReqMarch->Set_Btime(ptbReqMarch->m_nEtime);
    ptbReqMarch->Set_Ctime(ptbReqMarch->m_bParam[0].m_ddwMarchingTime);
    ptbReqMarch->Set_Etime(ptbReqMarch->m_nBtime + ptbReqMarch->m_nCtime);
    pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;

    ptbWild->Set_Status(ptbWild->m_nStatus & (~EN_CITY_STATUS__AVOID_WAR));
    ptbWild->Set_Time_end(udwCurTime);
    CCommonBase::AbandonWild(pstCity, ptbWild);
    CActionBase::UpdtPassiveActionWhenAbandonWild(pstUser, ptbWild);
    pstSession->m_ucMapItemFlag = EN_TABLE_UPDT_FLAG__CHANGE;

    return 0;
}

TINT32 CProcessAction::ProcessMarchPreLoading(SSession *pstSession)
{
    TbMarch_action *ptbReqMarch = &pstSession->m_stReqMarch;
    SActionMarchParam *pstParam = &ptbReqMarch->m_bParam[0];
    SUserInfo *pstUser = &pstSession->m_stSourceUser;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;
    TbMap *ptbWild = &pstSession->m_stMapItem;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TBOOL bSuccess = TRUE;
    TINT32 dwReason = EN_REPORT_RESULT_TARGET_CHANGE;

    TUINT32 udwMapType = CMapLogic::GetWildClass(ptbWild->m_nSid, ptbWild->m_nType);
    TUINT32 udwCurtime = CTimeUtils::GetUnixTime();

    // 任务合法性检查 - 资源地
    if (bSuccess && EN_WILD_CLASS_RES == udwMapType &&
        (ptbWild->m_nType != pstSession->m_stRawReqMarch.m_bParam[0].m_ddwTargetType || !CMapLogic::IsOccupyWild(ptbWild->m_nSid, ptbWild->m_nType)))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchPreLoading: target[type:%ld,%u] has changed, just return... [seq=%u]",
            ptbWild->m_nType, pstSession->m_stRawReqMarch.m_bParam[0].m_ddwTargetType, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_COLLECTED_OUT;
    }
    if (bSuccess && EN_WILD_CLASS_RES == udwMapType &&
        ptbWild->m_nUid != ptbPlayer->m_nUid)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchPreLoading: target[uid:%ld,%ld] has been occupied by others, just return... [seq=%u]",
            ptbWild->m_nUid, ptbPlayer->m_nUid, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_HAS_OCCUPIED;
    }

    // 任务合法性检查 - 巢穴
    if (bSuccess && EN_WILD_CLASS_MONSTER_NEST == udwMapType &&
        (ptbWild->m_nExpire_time > 0 && udwCurtime >= ptbWild->m_nExpire_time))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchPreLoading: wild[%ld] reach disappear time[%ld], just return... [seq=%u]",
            ptbWild->m_nId, ptbWild->m_nExpire_time, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_COLLECTED_OUT;
    }
    if (bSuccess && EN_WILD_CLASS_MONSTER_NEST == udwMapType &&
        (ptbPlayer->m_nAlid == 0))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchPreLoading: user[%ld] not in alliance, just return... [seq=%u]",
            ptbPlayer->m_nUid, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_OCCUPY_LAIR_NO_AL;
    }
    if (bSuccess && EN_WILD_CLASS_MONSTER_NEST == udwMapType &&
        ptbWild->m_nOccupy_clean_flag == 1)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchPreLoading: wild[%ld] occupy clean flag[%ld], just return... [seq=%u]",
            ptbWild->m_nId, ptbWild->m_nOccupy_clean_flag, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_COLLECTED_OUT;
    }
    if (bSuccess && EN_WILD_CLASS_MONSTER_NEST == udwMapType &&
        (ptbWild->m_nUid != 0 && ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET != ptbWild->m_nAlid))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchPreLoading: user[%ld] not in same alliance, user_alid[%ld] wild_alid[%ld], just return... [seq=%u]",
            ptbPlayer->m_nUid, ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET, ptbWild->m_nAlid, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_OCCUPY_LAIR_WRONG_AL;
    }
    if (bSuccess && EN_WILD_CLASS_MONSTER_NEST == udwMapType &&
        ptbWild->m_nUid != 0)
    {
        for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
        {
            TbMarch_action tbAction = pstUser->m_atbMarch[udwIdx];
            if (tbAction.m_nMclass == EN_ACTION_MAIN_CLASS__MARCH && tbAction.m_nSclass == EN_ACTION_SEC_CLASS__OCCUPY &&
                tbAction.m_nSuid == ptbPlayer->m_nUid && tbAction.m_nTpos == ptbWild->m_nId && tbAction.m_nId != ptbReqMarch->m_nId &&
                (tbAction.m_nStatus == EN_MARCH_STATUS__PRE_LOADING || tbAction.m_nStatus == EN_MARCH_STATUS__LOADING))
            {
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchPreLoading: user[%ld] is now loading wild[%ld], just return... [seq=%u]",
                    ptbPlayer->m_nUid, ptbWild->m_nId, pstSession->m_udwSeqNo));
                bSuccess = FALSE;
                dwReason = EN_REPORT_RESULT_TARGET_HAS_OCCUPIED;
            }
        }
    }

    // 更新action状态
    if(bSuccess == TRUE)
    {
        //是否带英雄 会影响buff
        SPlayerBuffInfo stOccupyBuff;
        if (ptbReqMarch->m_bParam[0].m_stDragon.m_ddwLevel != 0)
        {
            CBufferBase::GenOccupyBuff(&pstUser->m_stPlayerBuffList, &stOccupyBuff);
        }
        else
        {
            CBufferBase::GenOccupyBuff(&pstUser->m_stBuffWithoutDragon, &stOccupyBuff);
        }
        CBufferBase::SetOccupyBuff(&stOccupyBuff, ptbReqMarch);

        pstParam->m_ddwLoadTime = CMapLogic::ComputeLoadResTime(ptbReqMarch, ptbWild);
        pstParam->m_ddwTotalLoadTime = pstParam->m_ddwLoadTime;
        pstParam->m_ddwBeginLoadTime = ptbReqMarch->m_nEtime;
        pstParam->m_ddwLoadResTotalNum = CMapLogic::GetMapResTotalNum(ptbWild);

        pstParam->m_ddwLoadRate = pstParam->m_ddwLoadResTotalNum * 1.0 / pstParam->m_ddwLoadTime * 3600;
        TUINT32 udwTotalLoad = CCommonBase::GetTroopTotalLoad(ptbReqMarch);
        if (pstParam->m_ddwLoadResTotalNum > udwTotalLoad)
        {
            pstParam->m_ddwLoadResTotalNum = udwTotalLoad;
            pstParam->m_ddwLoadTime = ceil(pstParam->m_ddwLoadResTotalNum * 1.0 / pstParam->m_ddwLoadRate * 3600);
        }
        if (udwMapType == EN_WILD_CLASS_MONSTER_NEST)
        {
            if (udwCurtime + pstParam->m_ddwLoadTime > ptbWild->m_nExpire_time)
            {
                pstParam->m_ddwLoadTime = ptbWild->m_nExpire_time - udwCurtime;
            }
        }

        //wave@20160504: occupy对骑士不增加经验
        /*
        if (pstParam->m_stKnight.ddwLevel < CGameInfo::GetInstance()->GetBasicVal(EN_GAME_BASIC_KNIGHT_TOP_LEVEL))
        {
        pstParam->m_stKnight.ddwExpAdd = CGameInfo::GetInstance()->m_oJsonRoot["game_knight"]["a"]["a2"].asInt64();
        }*/
        ptbReqMarch->SetFlag(TbMARCH_ACTION_FIELD_PARAM);

        ptbReqMarch->Set_Status(EN_MARCH_STATUS__LOADING);
        ptbReqMarch->Set_Btime(ptbReqMarch->m_nEtime);
        ptbReqMarch->Set_Ctime(pstParam->m_ddwLoadTime);
        ptbReqMarch->Set_Etime(ptbReqMarch->m_nBtime + pstParam->m_ddwLoadTime);
        
        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchPreLoading: set reqmarch: btime[%ld] ctime[%ld] etime[%ld] stat[%ld] [seq=%u]",
            ptbReqMarch->m_nBtime, ptbReqMarch->m_nCtime, ptbReqMarch->m_nEtime, ptbReqMarch->m_nStatus, pstSession->m_udwSeqNo));

        if (EN_WILD_CLASS_MONSTER_NEST == udwMapType)
        {
            CWarProcess::UpdateOccupyLairAction(pstSession, &pstSession->m_stSourceUser, ptbWild);
        }
        TSE_LOG_INFO(CGameInfo::GetInstance()->m_poLog, ("ProcessMarchPreLoading: btime[%ld] ctime[%ld] etime[%ld] load_rate[%ld], load_time[%ld] [seq=%u]",
            ptbReqMarch->m_nBtime, ptbReqMarch->m_nCtime, ptbReqMarch->m_nEtime, ptbReqMarch->m_bParam[0].m_ddwLoadRate, ptbReqMarch->m_bParam[0].m_ddwLoadTime,
            pstSession->m_udwSeqNo));

        CSendMessageBase::AddTips(&pstSession->m_stSourceUser, EN_TIPS_TYPE__OCCUPY_OK, ptbPlayer->m_nUid, TRUE,
            ptbWild->m_nId, ptbWild->m_nType, ptbWild->m_nLevel);

        TSE_LOG_INFO(CGameInfo::GetInstance()->m_poLog, ("ProcessMarchPreLoading:[sid=%u uid=%lld action_id=%lld pos=%u] action_info[second_class=%u status=%u,begin_time=%u,end_time=%u,cost_time=%u] map_info[type=%u lv=%u reward_left=%u] player_info[speed_buff:gold=%u food=%u ore=%u stone=%u wood=%u all=%u troop_load:%u] result_info[cur_time=%u load_time=%u load_num=%u load_rate=%u total_load_time=%u] [seq=%u]",
            ptbWild->m_nSid, pstUser->m_tbPlayer.m_nUid, ptbReqMarch->m_nId, ptbWild->m_nId,
            ptbReqMarch->m_nSclass, ptbReqMarch->m_nStatus,
            ptbReqMarch->m_nBtime, ptbReqMarch->m_nEtime, ptbReqMarch->m_nCtime,
            ptbWild->m_nType, ptbWild->m_nLevel, ptbWild->m_nReward_left,
            CActionBase::GetUserBuffById(ptbReqMarch, EN_BUFFER_INFO_GOLD_COLLECT_SPEED),
            CActionBase::GetUserBuffById(ptbReqMarch, EN_BUFFER_INFO_FOOD_COLLECT_SPEED),
            CActionBase::GetUserBuffById(ptbReqMarch, EN_BUFFER_INFO_ORE_COLLECT_SPEED),
            CActionBase::GetUserBuffById(ptbReqMarch, EN_BUFFER_INFO_STONE_COLLECT_SPEED),
            CActionBase::GetUserBuffById(ptbReqMarch, EN_BUFFER_INFO_WOOD_COLLECT_SPEED),
            CActionBase::GetUserBuffById(ptbReqMarch, EN_BUFFER_INFO_COLLECT_SPEED),
            udwTotalLoad,
            CTimeUtils::GetUnixTime(), pstParam->m_ddwLoadTime, pstParam->m_ddwLoadResTotalNum, pstParam->m_ddwLoadRate,
            pstParam->m_ddwTotalLoadTime,
            pstSession->m_udwSeqNo));
    }
    else//部队直接放回
    {
        CProcessReport::GenInValidReport(pstSession, EN_REPORT_TYPE_OCCUPY, dwReason, &pstSession->m_tbReport);

        ptbReqMarch->Set_Status(EN_MARCH_STATUS__RETURNING);
        ptbReqMarch->Set_Btime(ptbReqMarch->m_nEtime);
        ptbReqMarch->Set_Etime(ptbReqMarch->m_nBtime + ptbReqMarch->m_nCtime);
		ptbReqMarch->Set_Sal(0, UPDATE_ACTION_TYPE_DELETE);
        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;

        if (EN_WILD_CLASS_RES == udwMapType && ptbWild->m_nUid == ptbPlayer->m_nUid)
        {
            CCommonBase::AbandonWild(pstCity, ptbWild);
        }
        if (EN_WILD_CLASS_MONSTER_NEST == udwMapType)
        {
            CWarProcess::UpdateOccupyLairMap(pstSession, &pstSession->m_stTargetUser, ptbWild);
            if (ptbWild->m_nOccupy_num == 0 && ptbWild->m_nUid == ptbPlayer->m_nUid)
            {
                CCommonBase::AbandonWild(pstCity, ptbWild);
            }
        }

        pstSession->m_ucMapItemFlag = EN_TABLE_UPDT_FLAG__CHANGE;
    }

    return 0;
}

TINT32 CProcessAction::ProcessMarchUnLoading(SSession *pstSession)
{
    SCityInfo *pstTCity = NULL;
    TbMarch_action *ptbReqMarch = &pstSession->m_stReqMarch;
    TbMap *ptbWild = &pstSession->m_stMapItem;
    SUserInfo *pstSUser = &pstSession->m_stSourceUser;
    TbPlayer *ptbSPlayer = &pstSUser->m_tbPlayer;

    SCityInfo *pstCity = &pstSUser->m_stCityInfo;

    TBOOL bSuccess = TRUE;
    TUINT64 uddwTransResource = 0;
    TINT32 dwReason = EN_REPORT_RESULT_TARGET_CHANGE;

    // a. 任务合法性检查
    if(bSuccess && (ptbWild->m_nType != EN_WILD_TYPE__CITY || ptbWild->m_nUid != ptbReqMarch->m_nTuid))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchUnLoading: target[type:%ld] not city or target[uid:%ld,%ld] has changed, just return... [seq=%u]",
            ptbWild->m_nType, ptbReqMarch->m_nTuid, ptbWild->m_nUid, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_CHANGE;
    }
    //pstTCity = CCityBase::GetCityInfo(pstSession->m_stTargetUser.m_astCityInfo, pstSession->m_stTargetUser.m_udwCityNum, ptbReqMarch->m_nTcid);
    if(pstSession->m_stTargetUser.m_stCityInfo.m_stTblData.m_nPos == ptbReqMarch->m_nTpos)
    {
        pstTCity = &pstSession->m_stTargetUser.m_stCityInfo;
    }
    if(bSuccess && pstTCity == NULL)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchUnLoading: no such city[%lld], just return... [seq=%u]",
            ptbReqMarch->m_nTpos, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_CHANGE;
    }

    // b. 进行资源卸载
    if(bSuccess == TRUE)
    {
        // 更新用户的联盟请求信息
        CCommonBase::UpdateResourceHelpInfo(&pstSession->m_stTargetUser, ptbReqMarch, EN_ASSIST_TYPE__RES_REQ);
        CCommonBase::UpdateResourceHelpInfo(&pstSession->m_stSourceUser, ptbReqMarch, EN_ASSIST_TYPE__RES_SHARE);

        CProcessReport::GenTransportReport(pstSession, &pstSession->m_tbReport);

        // 查找成功则卸载资源
        for(TUINT32 udwIdx = EN_RESOURCE_TYPE__GOLD; udwIdx < EN_RESOURCE_TYPE__END; ++udwIdx)
        {
            uddwTransResource += ptbReqMarch->m_bParam[0].m_stResource.m_addwNum[udwIdx];

            pstTCity->m_stTblData.m_bResource[0].m_addwNum[udwIdx] += ptbReqMarch->m_bParam[0].m_stResource.m_addwNum[udwIdx];
            pstSUser->m_tbPlayer.m_bTransport_resource[0].m_addwNum[udwIdx] += ptbReqMarch->m_bParam[0].m_stResource.m_addwNum[udwIdx];

            TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessMarchUnLoading: transport [action resource idx=%u num=%u] [player resource idx=%u num=%u] [seq=%u]",
                udwIdx, ptbReqMarch->m_bParam[0].m_stResource.m_addwNum[udwIdx],
                udwIdx, pstSUser->m_tbPlayer.m_bTransport_resource[0].m_addwNum[udwIdx],
                pstSession->m_udwSeqNo));

            ptbReqMarch->m_bParam[0].m_stResource.m_addwNum[udwIdx] = 0;
            if(pstTCity->m_stTblData.m_bResource[0].m_addwNum[udwIdx] > MAX_RESOURCE_LIMIT_NUM)
            {
                pstTCity->m_stTblData.m_bResource[0].m_addwNum[udwIdx] = MAX_RESOURCE_LIMIT_NUM;
            }

            pstSUser->m_tbPlayer.SetFlag(TbPLAYER_FIELD_TRANSPORT_RESOURCE);

        }
        pstTCity->m_stTblData.SetFlag(TbCITY_FIELD_RESOURCE);
        ptbReqMarch->SetFlag(TbMARCH_ACTION_FIELD_PARAM);

        // 增加贡献度
        ptbSPlayer->Set_Sh_res(ptbSPlayer->m_nSh_res + uddwTransResource);

        CSendMessageBase::AddTips(&pstSession->m_stSourceUser, EN_TIPS_TYPE__TRANSPORT_OK, ptbSPlayer->m_nUid, TRUE,
            ptbWild->m_nId, ptbWild->m_nUid, 0, ptbWild->m_sUname.c_str());

        SNoticInfo stNoticInfo;
        stNoticInfo.Reset();
        stNoticInfo.m_dwNoticId = EN_NOTI_ID__TRANSPORT_SUCC;
        stNoticInfo.m_strTName = pstSession->m_stTargetUser.m_tbPlayer.m_sUin;
        stNoticInfo.m_strLang = CDocument::GetLang(pstSUser->m_tbLogin.m_nLang);
        CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), ptbSPlayer->m_nSid, ptbSPlayer->m_nUid, stNoticInfo);
        stNoticInfo.Reset();
        stNoticInfo.m_dwNoticId = EN_NOTI_ID__BE_TRANSPORT_SUCC;
        stNoticInfo.m_strSName = ptbSPlayer->m_sUin;
        stNoticInfo.m_strLang = CDocument::GetLang(pstSession->m_stTargetUser.m_tbLogin.m_nLang);
        CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), pstSession->m_stTargetUser.m_tbPlayer.m_nSid, pstSession->m_stTargetUser.m_tbPlayer.m_nUid, stNoticInfo);

        CQuestLogic::SetTaskCurValue(pstSUser, pstCity, EN_TASK_TYPE_ING_TRANSPORT_NUM);

        CMsgBase::SendEncourageMail(&pstSUser->m_tbUserStat, pstSUser->m_tbPlayer.m_nSid, EN_MAIL_ID__FIRST_TRANSPORT);

    }
    else
    {
        CProcessReport::GenInValidReport(pstSession, EN_REPORT_TYPE_TRANSPORT, dwReason, &pstSession->m_tbReport);
    }

    // c. 更新action状态
    /*
    ptbReqMarch->Set_Status(EN_MARCH_STATUS__RETURNING);
    ptbReqMarch->Set_Btime(ptbReqMarch->m_nEtime);
    ptbReqMarch->Set_Etime(ptbReqMarch->m_nBtime + ptbReqMarch->m_nCtime);
    //ptbReqMarch->DeleteField(TbMARCH_ACTION_FIELD_TAL);
    ptbReqMarch->Set_Tal(0, UPDATE_ACTION_TYPE_DELETE);
    pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;
    */
    
    // 卸载后不需要返回
    pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__DEL;

    return 0;
}

TINT32 CProcessAction::ProcessMarchDragonAttack(SSession *pstSession)
{
    TbMarch_action *ptbReqAction = &pstSession->m_stReqMarch;
    TbMap *ptbWild = &pstSession->m_stMapItem;
    TBOOL bSuccess = TRUE;
    TINT32 dwReason = 0;
    SUserInfo *pstSUser = &pstSession->m_stSourceUser;
    CGameInfo *poGameInfo = CGameInfo::GetInstance();

    if(EN_COMMAND_STEP__INIT == pstSession->m_udwCommandStep)
    {

        // 任务合法性检查
        if(bSuccess && !CMapLogic::IsDragonAttackWild(ptbWild->m_nSid, ptbWild->m_nType))
        {
            TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchDragonAttack: suid[%ld] wild_uid[%ld] wild_type[%ld], just return... [seq=%u]",
                                                    ptbReqAction->m_nSuid, ptbWild->m_nUid, ptbWild->m_nType, pstSession->m_udwSeqNo));
            bSuccess = FALSE;
            dwReason = EN_REPORT_RESULT_TARGET_CHANGE;
        }
        
        if(!bSuccess)
        {
            CProcessReport::GenInValidReport(pstSession, EN_REPORT_TYPE_DRAGON_MONSTER, dwReason, &pstSession->m_tbReport);
        
            ptbReqAction->Set_Status(EN_MARCH_STATUS__RETURNING);
            ptbReqAction->Set_Btime(ptbReqAction->m_nEtime);
            ptbReqAction->Set_Etime(ptbReqAction->m_nBtime + ptbReqAction->m_nCtime);
            //ptbReqAction->DeleteField(TbMARCH_ACTION_FIELD_TAL);
            ptbReqAction->Set_Tal(0, UPDATE_ACTION_TYPE_DELETE);

            // 更新action状态
            pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
            
            return 0;
        }

        //attack
        CWarProcess::ProcessWar(pstSession, &pstSession->m_stDragonNode, &pstSession->m_stMonsterNode);

        TBOOL bNormalReward = FALSE;
        TBOOL bEliteReward = FALSE;
        CWarBase::GetDropRewardResult(&pstSession->m_stMonsterNode, bNormalReward, bEliteReward);

        if(FALSE == bNormalReward
           && FALSE == bEliteReward)
        {
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        }
        else
        {
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__DATA_CENTER;
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
            
            
            pstSession->ResetDataCenterReq();
            DataCenterReqInfo* pstReq = new DataCenterReqInfo;
            pstReq->m_udwType = EN_REFRESH_DATA_TYPE__MONSTER;
            Json::Value rDataReqJson = Json::Value(Json::objectValue);
            
            rDataReqJson["sid"] = pstSUser->m_tbLogin.m_nSid;
            rDataReqJson["aid"] = pstSUser->m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET;
            rDataReqJson["uid"] = pstSUser->m_tbPlayer.m_nUid;
            rDataReqJson["castle_lv"] = CCityBase::GetBuildingLevelByFuncType(&pstSUser->m_stCityInfo, EN_BUILDING_TYPE__CASTLE);
            rDataReqJson["request"] = Json::Value(Json::objectValue);
            rDataReqJson["request"]["monster_id"] = pstSession->m_stMonsterNode.m_dwType;
            rDataReqJson["request"]["monster_lv"] = pstSession->m_stMonsterNode.m_dwLevel;
            if(FALSE == pstSession->m_stMonsterNode.m_bIsDead)
            {
                rDataReqJson["request"]["kill"] = 0U;
            }
            else
            {
                rDataReqJson["request"]["kill"] = 1U;
            }
            rDataReqJson["request"]["owner"] = ptbReqAction->m_bMonster_info[0].ddwChallengerId;
            rDataReqJson["request"]["attack_list"] = Json::Value(Json::arrayValue);
            for(TINT32 dwIdx = 0; dwIdx < pstSession->m_stDragonNode.m_dwCanAttackTimes; ++dwIdx)
            {
                rDataReqJson["request"]["attack_list"].append(pstSession->m_stDragonNode.m_astAttackInfo[dwIdx].m_dwAttackType);
            }
            
            Json::FastWriter rEventWriter;
            pstReq->m_sReqContent = rEventWriter.write(rDataReqJson);
            pstSession->m_vecDataCenterReq.push_back(pstReq);
            
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessMarchDragonAttack: data center req: [json=%s] [type=%u] [uid=%ld] [seq=%u]", \
                                                    pstReq->m_sReqContent.c_str(), \
                                                    pstReq->m_udwType, \
                                                    pstSUser->m_tbPlayer.m_nUid, \
                                                    pstSession->m_udwSeqNo));

            // return 0;
            TINT32 dwRetCode = SendDataCenterRequest(pstSession, EN_SERVICE_TYPE_DATA_CENTER_REQ);
            if(0 == dwRetCode)
            {
                return 0;
            }
            else
            {
                pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchDragonAttack: send request to data center failed. [json=%s] [ret=%d] [seq=%u]", \
                                                        pstReq->m_sReqContent.c_str(), \
                                                        dwRetCode, \
                                                        pstSUser->m_udwBSeqNo));
            }
 
        }
    }
    
    if(EN_COMMAND_STEP__1 == pstSession->m_udwCommandStep)
    {

        SRefreshData stRefreshData;
        vector<DataCenterRspInfo*>& vecRsp = pstSession->m_vecDataCenterRsp;
        DataCenterRspInfo *pstDataCenterRsp = NULL;
        if(vecRsp.size() > 0)
        {
            stRefreshData.Reset();
            Json::Reader jsonReader;
            Json::Value oRspDataJson;
            for (TUINT32 udwIdx = 0; udwIdx < vecRsp.size(); ++udwIdx)
            {
                pstDataCenterRsp = vecRsp[udwIdx];
                if(EN_REFRESH_DATA_TYPE__MONSTER == pstDataCenterRsp->m_udwType)
                {
                    TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessMarchDragonAttack: data center get rsp: [json=%s] [uid=%ld] [seq=%u]",
                                                            pstDataCenterRsp->m_sRspJson.c_str(), \
                                                            pstSUser->m_tbPlayer.m_nUid, \
                                                            pstSUser->m_udwBSeqNo));

                    if (FALSE == jsonReader.parse(pstDataCenterRsp->m_sRspJson, oRspDataJson))
                    {
                        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_DATA_CENTER_PACKAGE_ERR;
                        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchDragonAttack: prase rsp from data center failed. [seq=%u]", \
                                                                pstSUser->m_udwBSeqNo));
                        return -2;
                    }
                    TINT32 dwRetCode = stRefreshData.m_stMonsterRsp.setVal(oRspDataJson);
                    if (0 != dwRetCode)
                    {
                        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__DATA_CENTER_PACKAGE_FORMAT_ERR;
                        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchDragonAttack: response data format error. [ret=%d][seq=%u]", \
                                                                dwRetCode, \
                                                                pstSUser->m_udwBSeqNo));
                        return -3;
                    }
                    break;
                }
            }

            if(0 == stRefreshData.m_stMonsterRsp.m_vecAttackReward.size()
               && 0 == stRefreshData.m_stMonsterRsp.m_vecEliteReward.size())
            {
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchDragonAttack: attack moster reward size is zero. [normalsize=%ld] [elitesize=%ld] [seq=%u]", \
                                                        stRefreshData.m_stMonsterRsp.m_vecAttackReward.size(), \
                                                        stRefreshData.m_stMonsterRsp.m_vecEliteReward.size(), \
                                                        pstSUser->m_udwBSeqNo));
            }
            
            if(MAX_REWARD_ITEM_NUM < stRefreshData.m_stMonsterRsp.m_vecAttackReward.size()
               || 0 >= stRefreshData.m_stMonsterRsp.m_vecAttackReward.size())
            {
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchDragonAttack: attack normal moster reward size is over. [size=%ld] [seq=%u]", \
                                                        stRefreshData.m_stMonsterRsp.m_vecAttackReward.size(), \
                                                        pstSUser->m_udwBSeqNo));
            }
            
            if(MAX_REWARD_ITEM_NUM < stRefreshData.m_stMonsterRsp.m_vecEliteReward.size()
              || 0 >= stRefreshData.m_stMonsterRsp.m_vecEliteReward.size())
            {
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchDragonAttack: attack elite moster reward size is over. [size=%ld] [seq=%u]", \
                                                        stRefreshData.m_stMonsterRsp.m_vecEliteReward.size(), \
                                                        pstSUser->m_udwBSeqNo));
            }


            
            if(0 != stRefreshData.m_stMonsterRsp.m_vecAttackReward.size())
            {
                CWarBase::NewSetMarchReward(ptbReqAction, stRefreshData.m_stMonsterRsp.m_vecAttackReward);
            }
            
            SUserInfo *pstChallenger = NULL;
            if(0 != stRefreshData.m_stMonsterRsp.m_vecEliteReward.size()
               && pstSession->m_stMonsterNode.m_bIsDead
               && pstSession->m_stMonsterNode.m_ddwLeader)
            {           
                if (ptbWild->m_nUid != pstSUser->m_tbPlayer.m_nUid)
                {
                    pstChallenger = &pstSession->m_stTargetUser;
                }
                else
                {
                    pstChallenger = pstSUser;
                }
                CWarBase::NewSetChallengerReward(pstChallenger, &pstSession->m_stDragonNode, stRefreshData.m_stMonsterRsp.m_vecEliteReward);
            }
            
            
            Json::Value jMonsterReward;
            Json::FastWriter write;
            CWarBase::MonsterRewardOutput(&pstSession->m_stReqMarch, jMonsterReward);
            TSE_LOG_INFO(pstSession->m_poServLog, ("[seq=%u] MonsterReward : %s", \
                                                    pstSession->m_udwSeqNo, \
                                                    write.write(jMonsterReward).c_str()));

            pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        }
        else
        {
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        }        
    }

    if(EN_COMMAND_STEP__2 == pstSession->m_udwCommandStep)
    {
        //report
        CProcessReport::GenDragonAttackReport(pstSession, EN_REPORT_RESULT_WIN, &pstSession->m_stDragonNode, &pstSession->m_stMonsterNode, &pstSession->m_tbReport);

        TUINT32 udwFightingTime = 0;
        TUINT32 udwBDragonLv = CPlayerBase::ComputeDragonLevel(pstSession->m_stDragonNode.m_dwRawExp);
        TUINT32 udwEDragonLv = CPlayerBase::ComputeDragonLevel(pstSession->m_stDragonNode.m_dwRawExp + pstSession->m_stDragonNode.m_dwExp);
        if (udwEDragonLv > udwBDragonLv && udwEDragonLv > pstSUser->m_tbPlayer.m_nDragon_level)
        {
            udwFightingTime = poGameInfo->m_oJsonRoot["game_dragon"]["a15"].asInt();
        }
        else if (pstSession->m_stMonsterNode.m_bIsDead)
        {
            udwFightingTime = poGameInfo->m_oJsonRoot["game_dragon"]["a14"].asInt();
        }
        else
        {
            udwFightingTime = poGameInfo->m_oJsonRoot["game_dragon"]["a13"].asInt();
        }

        if (pstSession->m_stMonsterNode.m_bIsDead 
            && pstSession->m_stMonsterNode.m_ddwLeader)
        {
            CProcessReport::GenChallengerReport(pstSession, ptbReqAction->m_bMonster_info[0].ddwChallengerId, &pstSession->m_stDragonNode, &pstSession->m_stMonsterNode, &pstSession->m_tbChallengerReport);
            pstSession->m_dwReportFlag |= EN_REPORT_FLAG_CHALLENGER_REWARD;
        
            TINT64 ddwShowTime = ptbReqAction->m_nEtime + udwFightingTime;
            if (ptbReqAction->m_nSuid == ptbReqAction->m_bMonster_info[0].ddwChallengerId)
            {
                ddwShowTime = CTimeUtils::GetUnixTime() - 1;
            }
            CCommonBase::AddRewardWindow(pstSUser, ptbReqAction->m_bMonster_info[0].ddwChallengerId, EN_REWARD_WINDOW_TYPE_ELITE_MONSTER_KILLED, 
                                         EN_REWARD_WINDOW_GET_TYPE_ELITE_MONSTER, ddwShowTime, &pstSession->m_stDragonNode.m_stChallengerReward, TRUE);
        }
        
        ptbReqAction->Set_Status(EN_MARCH_STATUS__FIGHTING);
        ptbReqAction->Set_Btime(ptbReqAction->m_nEtime);
        ptbReqAction->Set_Etime(ptbReqAction->m_nBtime + udwFightingTime);


        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;
        
        return 0;
    }

    return 0;
}

TINT32 CProcessAction::ProcessMarchDragonFighting(SSession *pstSession)
{
    TbMarch_action *ptbReqAction = &pstSession->m_stReqMarch;
    SUserInfo *pstUser = &pstSession->m_stSourceUser;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;

    SMonsterInfo *pstMonster = &ptbReqAction->m_bMonster_info[0];

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        //Tips
        if (ptbReqAction->m_bReward[0].ddwTotalNum != 0)
        {
            ostringstream oss;
            oss.str("");
            oss << "[";
            for (TUINT32 udwIdx = 0; udwIdx < ptbReqAction->m_bReward[0].ddwTotalNum; ++udwIdx)
            {
                if (udwIdx > 0)
                {
                    oss << ",";
                }
                oss << "[" << ptbReqAction->m_bReward[0].aRewardList[udwIdx].ddwType << ",";
                oss << ptbReqAction->m_bReward[0].aRewardList[udwIdx].ddwId << ",";
                oss << ptbReqAction->m_bReward[0].aRewardList[udwIdx].ddwNum << "]";

            }
            oss << "]";
            string sResourceInfo = oss.str();
            CSendMessageBase::AddTips(pstUser, pstUser->m_tbPlayer.m_nUid, TRUE, EN_TIPS_TYPE__DRAGON_ATTACK_MONSTER_GET_REWARD, sResourceInfo);
        }
        if (pstMonster->ddwDeadFlag)
        {
            //玩家信息统计
            pstUser->m_tbPlayer.m_bWar_statistics[0].ddwKillMonsterNum++;
            pstUser->m_tbPlayer.SetFlag(TbPLAYER_FIELD_WAR_STATISTICS);

            //if (pstMonster->ddwLead)
            //{
            //    CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPE__DRAGON_KILL_LEADER_MONSTER_GET_REWARD, pstUser->m_tbPlayer.m_nUid, TRUE, pstMonster->ddwType, pstMonster->ddwLv);
            //}
            //else
            //{
                CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPE__HERO_KILL_MONSTER_OK, pstUser->m_tbPlayer.m_nUid, TRUE, pstMonster->ddwType, pstMonster->ddwLv);
            //}
            CQuestLogic::SetTaskCurValue(pstUser, pstCity, EN_TASK_TYPE_ING_KILL_MONSTER);
        }

        //Notification
        SNoticInfo stNoticInfo;
        stNoticInfo.Reset();
        string stMonsterLv = CCommonFunc::NumToString(pstMonster->ddwLv);
        string stMonsterName = "";
        TBOOL bIsExistDocument = CDocument::GetInstance()->IsSupportLang("english");
        if (bIsExistDocument)
        {
            const Json::Value &stDocumentJson = CDocument::GetInstance()->GetDocumentJsonByLang("english");
            Json::Value::Members jTileMember = stDocumentJson["doc_tile"].getMemberNames();
            for (Json::Value::Members::iterator it = jTileMember.begin(); it != jTileMember.end(); ++it)
            {
                if (pstMonster->ddwType == atoi((*it).c_str()))
                {
                    stMonsterName = stDocumentJson["doc_tile"][(*it)]["name"].asString();
                    break;
                }
            }

            if (stMonsterName == "")
            {
                TSE_LOG_ERROR(pstSession->m_poServLog, ("Noti error doc[wild_type=%d][seq=%u]", pstMonster->ddwType,
                    pstSession->m_udwSeqNo));
            }
        }
        if (pstMonster->ddwDeadFlag)
        {
            stNoticInfo.m_dwNoticId = EN_NOTI_ID__HERO_ATTACK_SUCC_AND_AL_GIFT;
        }
        else
        {
            stNoticInfo.m_dwNoticId = EN_NOTI_ID__HERO_ATTACK_SUCC;
        }
        stNoticInfo.m_strLang = CDocument::GetLang(pstUser->m_tbLogin.m_nLang);
        stNoticInfo.m_strKeyName = stMonsterName;
        stNoticInfo.m_strValueName = stMonsterLv;

        CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), pstSession->m_stSourceUser.m_tbPlayer.m_nSid, pstSession->m_stSourceUser.m_tbPlayer.m_nUid, stNoticInfo);

        CMsgBase::SendEncourageMail(&pstUser->m_tbUserStat, pstUser->m_tbPlayer.m_nSid, EN_MAIL_ID__FIRST_HERO_ATTACK);

        CActivitesLogic::ComputeHeroExpMarchScore(pstUser, pstMonster->ddwExpGet);        

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        if (ptbReqAction->m_nDelay_report_id > 0)
        {
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__REPORT_SVR;

            pstSession->ResetReportInfo();
            CReportSvrRequest::UserReportPut(pstSession, pstUser->m_tbPlayer.m_nUid, 
                ptbReqAction->m_nDelay_report_id, EN_REPORT_TYPE_DRAGON_MONSTER, EN_ALLIANCE_REPORT_GET_TYPE__OUT);

            if (ptbReqAction->m_bMonster_info[0].ddwRid > 0)
            {
                CReportSvrRequest::UserReportPut(pstSession, ptbReqAction->m_bMonster_info[0].ddwChallengerId,
                    ptbReqAction->m_bMonster_info[0].ddwRid, EN_REPORT_TYPE_DRAGON_MONSTER, EN_ALLIANCE_REPORT_GET_TYPE__OUT);
            }
            return 0;
        }
    }
    
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        ptbReqAction->Set_Status(EN_MARCH_STATUS__RETURNING);
        ptbReqAction->Set_Btime(ptbReqAction->m_nEtime);
        ptbReqAction->Set_Etime(ptbReqAction->m_nBtime + ptbReqAction->m_nCtime);
        //ptbReqAction->DeleteField(TbMARCH_ACTION_FIELD_TAL);
        ptbReqAction->Set_Tal(0, UPDATE_ACTION_TYPE_DELETE);

        // 更新action状态
        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    }

    return 0;
}

TINT32 CProcessAction::ProcessMarchRallyReinforce(SSession* pstSession)
{
    TbMarch_action *ptbRallyReinforce = &pstSession->m_stReqMarch;
    SUserInfo *pstSUser = &pstSession->m_stSourceUser;
    SUserInfo *pstTUser = &pstSession->m_stTargetUser;
    TbPlayer *ptbSPlayer = &pstSUser->m_tbPlayer;
    TbMap *ptbWild = &pstSession->m_stMapItem;

    TbMarch_action* ptbRallyWar = NULL;
    TINT32 dwRallyWarIndex = -1;
    dwRallyWarIndex = CActionBase::GetMarchIndex(pstSUser->m_atbMarch, pstSUser->m_udwMarchNum, ptbRallyReinforce->m_nTid);
    if (dwRallyWarIndex >= 0)
    {
        ptbRallyWar = &pstSUser->m_atbMarch[dwRallyWarIndex];
    }

    SCityInfo *pstCity = &pstSUser->m_stCityInfo;

    TBOOL bSuccess = TRUE;
    TINT32 dwReason = EN_REPORT_RESULT_TARGET_CHANGE;

    // 任务合法性检查
    if(bSuccess && (ptbWild->m_nType != EN_WILD_TYPE__CITY || ptbWild->m_nUid != ptbRallyReinforce->m_nTuid 
		|| pstSession->m_bCheckValidFlag == FALSE))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchRallyReinforce: check_valid=%d target[type:%ld] not city or target[uid:%ld,%ld] has changed, just return... [seq=%u]",
            pstSession->m_bCheckValidFlag, ptbWild->m_nType, ptbRallyReinforce->m_nTuid, ptbWild->m_nUid, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_CHANGE;
    }
    if(bSuccess && (ptbSPlayer->m_nAlpos == 0 || ptbSPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET != ptbWild->m_nAlid))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchRallyReinforce: target[salpos:%ld,sal:%ld,tal:%ld] not right, just return... [seq=%u]",
            ptbSPlayer->m_nAlpos, ptbSPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET, ptbWild->m_nAlid, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_DIFF_ALLIANCE;
    }
    if(bSuccess && ptbRallyWar == NULL)
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchReinForce: main action[id=%ld] not existed or go, just return... [seq=%u]",
            ptbRallyReinforce->m_nTid, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_RALLY_WAR_END;
    }
    if (bSuccess && ptbRallyWar->m_nStatus != EN_MARCH_STATUS__PREPARING && ptbRallyWar->m_nStatus != EN_MARCH_STATUS__WAITING)
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchReinForce: main status[%d] just return... [seq=%u]", ptbRallyWar->m_nStatus, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_RALLY_WAR_END;
    }

    // 更新action状态
    if(bSuccess == TRUE)
    {
        ptbRallyReinforce->Set_Status(EN_MARCH_STATUS__PREPARING);
        ptbRallyReinforce->Set_Etime(INT64_MAX);
        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;

        CQuestLogic::SetTaskCurValue(pstSUser, pstCity, EN_TASK_TYPE_ING_REINFORCE_NUM);

        if(ptbRallyWar->m_nStatus == EN_MARCH_STATUS__WAITING)
        {
            TBOOL bNeedWait = FALSE;
            TINT64 ddwEtime = 0;
            for (TUINT32 udwIdx = 0; udwIdx < pstTUser->m_udwPassiveMarchNum; ++udwIdx)
            {
                if (pstTUser->m_atbPassiveMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__RALLY_REINFORCE
                    && pstTUser->m_atbPassiveMarch[udwIdx].m_nTid == ptbRallyWar->m_nId
                    && pstTUser->m_atbPassiveMarch[udwIdx].m_nId != ptbRallyReinforce->m_nId
                    && pstTUser->m_atbPassiveMarch[udwIdx].m_nStatus == EN_MARCH_STATUS__MARCHING)
                {
                    bNeedWait = TRUE;
                    if (ddwEtime < pstSUser->m_atbPassiveMarch[udwIdx].m_nEtime)
                    {
                        ddwEtime = pstSUser->m_atbPassiveMarch[udwIdx].m_nEtime;
                    }
                }
            }
            if (bNeedWait)
            {
                ptbRallyWar->Set_Etime(ddwEtime);
            }
            else
            {
                ptbRallyWar->Set_Etime(CTimeUtils::GetUnixTime());
            }
        }

        CMsgBase::SendEncourageMail(&pstSUser->m_tbUserStat, pstSUser->m_tbPlayer.m_nSid, EN_MAIL_ID__FIRST_REINFORCE);
    }
    else//部队直接放回
    {
        CProcessReport::GenInValidReport(pstSession, EN_REPORT_TYPE_RALLY_REINFORCE, dwReason, &pstSession->m_tbReport);

        CActionBase::ReturnMarch(ptbRallyReinforce);
        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;

        if(ptbRallyWar != NULL && ptbRallyWar->m_nStatus == EN_MARCH_STATUS__WAITING)
        {
            ptbRallyWar->Set_Etime(CTimeUtils::GetUnixTime());
        }
    }

    return 0;
}

TINT32 CProcessAction::ProcessMarchRallyAttack(SSession* pstSession)
{
    TbMarch_action *ptbRallyWar = &pstSession->m_stReqMarch;
    SActionMarchParam *pstParam = &ptbRallyWar->m_bParam[0];
    TbMap *ptbWild = &pstSession->m_stMapItem;
    TBOOL bSuccess = TRUE;
    TINT32 dwReason = EN_REPORT_RESULT_TARGET_CHANGE;
    SUserInfo *pstSUser = &pstSession->m_stSourceUser;
    SUserInfo *pstTUser = &pstSession->m_stTargetUser;
    TbDiplomacy *ptbDip = NULL;
    TbPlayer *ptbSPlayer = &pstSUser->m_tbPlayer;
    if(ptbWild->m_nAlid != 0 && ptbSPlayer->m_nAlpos != 0)
    {
        ptbDip = CCommonBase::GetDiplomacy(pstSession->m_stSourceUser.m_atbDiplomacy, pstSession->m_stSourceUser.m_udwDiplomacyNum,
            ptbWild->m_nAlid);
    }

    // 任务合法性检查
    if ((ptbSPlayer->m_nStatus & EN_CITY_STATUS__NEW_PROTECTION)
        || (ptbSPlayer->m_nStatus & EN_CITY_STATUS__AVOID_WAR))
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchRallyAttack: srcPlayer[id=%ld] is in peace time, just return... [seq=%u]",
            ptbSPlayer->m_nUid, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_SELF_PEACETIME;
    }

    if (bSuccess && pstSession->m_bCheckValidFlag == FALSE)
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchRallyAttack: check_valid=%d suid[%ld] tuid[%ld] wild_type[%ld], just return... [seq=%u]",
            pstSession->m_bCheckValidFlag, ptbRallyWar->m_nSuid, ptbRallyWar->m_nTuid, ptbWild->m_nType, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_CHANGE;
    }

    if (bSuccess && ptbRallyWar->m_nTuid != ptbWild->m_nUid)
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchRallyAttack: suid[%ld] tuid[%ld] wild_type[%ld], just return... [seq=%u]",
            ptbRallyWar->m_nSuid, ptbRallyWar->m_nTuid, ptbWild->m_nType, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_CHANGE;
    }

    if (bSuccess && (ptbWild->m_nType == EN_WILD_TYPE__THRONE_NEW || ptbWild->m_nType == EN_WILD_TYPE__IDOL))
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchRallyAttack: normal rally war throne: wild_type[%ld], just return... [seq=%u]",
            ptbWild->m_nType, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_CHANGE;
    }

    if(bSuccess && ptbSPlayer->m_nAlpos != 0 && ptbSPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET == ptbWild->m_nAlid)
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchRallyAttack: sal[%u]=tal[%u], just return... [seq=%u]",
            pstParam->m_ddwSourceAlliance, pstParam->m_ddwTargetAlliance, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_SAME_ALLIANCE;
    }

    if(bSuccess && ptbDip && ptbDip->m_nType == EN_DIPLOMACY_TYPE__FRIENDLY)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchRallyAttack: sal[%u] and tal[%u] is friend, just return... [seq=%u]",
            ptbSPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET, ptbWild->m_nAlid, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_IS_FRIEND;
    }

    if(bSuccess && ((ptbWild->m_nStatus & EN_CITY_STATUS__AVOID_WAR) || (ptbWild->m_nStatus & EN_CITY_STATUS__NEW_PROTECTION)))
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchRallyAttack: target[%ld] is in peace time, just return... [seq=%u]",
            ptbWild->m_nId, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_PEACETIME;
    }

    if(bSuccess)
    {
        TUINT32 udwFightTime = 5;

        ptbRallyWar->Set_Status(EN_MARCH_STATUS__FIGHTING);
        ptbRallyWar->Set_Btime(ptbRallyWar->m_nEtime);
        ptbRallyWar->Set_Ctime(udwFightTime);
        ptbRallyWar->Set_Etime(ptbRallyWar->m_nBtime + udwFightTime);
        //给发起者tips
        CSendMessageBase::AddTips(&pstSession->m_stSourceUser, EN_TIPS_TYPE__ATTACKING, ptbSPlayer->m_nUid, TRUE,
            ptbWild->m_nId, ptbWild->m_nType, ptbWild->m_nLevel, ptbWild->m_sUname.c_str());
        //给支援者tips
        for(TUINT32 udwIdx = 0; udwIdx < pstSUser->m_udwMarchNum; ++udwIdx)
        {
            if(pstSUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__RALLY_REINFORCE
                && pstSUser->m_atbMarch[udwIdx].m_nTid == ptbRallyWar->m_nId
                && pstSUser->m_atbMarch[udwIdx].m_nStatus == EN_MARCH_STATUS__PREPARING)
            {
                CSendMessageBase::AddTips(&pstSession->m_stSourceUser, EN_TIPS_TYPE__ATTACKING, pstSUser->m_atbMarch[udwIdx].m_nSuid, TRUE,
                    ptbWild->m_nId, ptbWild->m_nType, ptbWild->m_nLevel, ptbWild->m_sUname.c_str());
            }
        }
    }
    else
    {
        CProcessReport::GenInValidReport(pstSession, EN_REPORT_TYPE_RALLY_WAR, dwReason, &pstSession->m_tbReport);

        ptbRallyWar->Set_Status(EN_MARCH_STATUS__RETURNING);
        ptbRallyWar->Set_Btime(ptbRallyWar->m_nEtime);
        ptbRallyWar->Set_Etime(ptbRallyWar->m_nBtime + ptbRallyWar->m_nCtime);
        //ptbRallyWar->DeleteField(TbMARCH_ACTION_FIELD_TAL);
        ptbRallyWar->Set_Tal(0, UPDATE_ACTION_TYPE_DELETE);

        for(TUINT32 udwIdx = 0; udwIdx < pstSUser->m_udwMarchNum; ++udwIdx)
        {
            TbMarch_action* ptbRallyReinforce = &pstSUser->m_atbMarch[udwIdx];
            if(ptbRallyReinforce->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_REINFORCE
            && ptbRallyReinforce->m_nStatus == EN_MARCH_STATUS__PREPARING
            && ptbRallyReinforce->m_nTid == ptbRallyWar->m_nId)
            {
                CActionBase::ReturnMarch(ptbRallyReinforce, ptbRallyWar->m_nTpos, ptbRallyWar->m_bParam[0].m_ddwTargetType);
                // 添加盟友idlist，用于发送report
                CToolBase::AddUserToMailReceiverList(pstSUser->m_adwMailSendUidList, pstSUser->m_udwMailSendNum, ptbRallyReinforce->m_nSuid);
            }
        }

        for(TUINT32 udwIdx = 0; udwIdx < pstTUser->m_udwMarchNum; ++udwIdx)
        {
            TbMarch_action* ptbRallyReinforce = &pstTUser->m_atbMarch[udwIdx];
            if(ptbRallyReinforce->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_REINFORCE
            && ptbRallyReinforce->m_nStatus == EN_MARCH_STATUS__PREPARING
            && ptbRallyReinforce->m_nTid == ptbRallyWar->m_nId)
            {
                CActionBase::ReturnMarch(ptbRallyReinforce);
                // 添加盟友idlist，用于发送report
                CToolBase::AddUserToMailReceiverList(pstSUser->m_adwMailSendUidList, pstSUser->m_udwMailSendNum, ptbRallyReinforce->m_nSuid);
            }
            if(ptbRallyReinforce->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_REINFORCE
            && ptbRallyReinforce->m_nStatus == EN_MARCH_STATUS__MARCHING
            && ptbRallyReinforce->m_nTid == ptbRallyWar->m_nId)
            {
                CActionBase::ReturnMarchOnFly(ptbRallyReinforce);
                // 添加盟友idlist，用于发送report
                CToolBase::AddUserToMailReceiverList(pstSUser->m_adwMailSendUidList, pstSUser->m_udwMailSendNum, ptbRallyReinforce->m_nSuid);
            }
        }
    }
    
    // 更新action状态
    pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;
    return 0;
}

TINT32 CProcessAction::ProcessMarchRallyFighting(SSession* pstSession)
{
    TbMarch_action *ptbRallyWar = &pstSession->m_stReqMarch;
    TbMap *ptbWild = &pstSession->m_stMapItem;
    TBOOL bSuccess = TRUE;
    TINT32 dwReason = EN_REPORT_RESULT_TARGET_CHANGE;
    SUserInfo *pstSUser = &pstSession->m_stSourceUser;
    TbPlayer *ptbSPlayer = &pstSUser->m_tbPlayer; 
    SUserInfo *pstTUser = &pstSession->m_stTargetUser;
    TUINT32 udwWildRawUId = ptbWild->m_nUid;

    // 任务合法性检查
    if (bSuccess && ptbRallyWar->m_nTuid != ptbWild->m_nUid)
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchRallyFighting: suid[%ld] tuid[%ld] wild_type[%ld], just return... [seq=%u]",
            ptbRallyWar->m_nSuid, ptbRallyWar->m_nTuid, ptbWild->m_nType, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_CHANGE;
    }

    if(bSuccess)
    {
        SBattleNode stDefend;
        SBattleNode stAttack;

        TUINT32 udwBeforUserHosNum = 0;
        TUINT32 udwAfterUserHosNum = 0;
        SCityInfo *pstCity = NULL;
        if(ptbRallyWar->m_nTuid !=0 && ptbRallyWar->m_nTpos != 0)
        {
            pstCity = &pstSession->m_stTargetUser.m_stCityInfo;
            udwBeforUserHosNum = CCityBase::GetHosCurNum(pstCity);
        }

        pstSession->m_dwMarchResult = CWarProcess::ProcessWar(pstSession, &stAttack, &stDefend, &pstSession->m_tbReport);

        udwAfterUserHosNum = CCityBase::GetHosCurNum(pstCity);
        TUINT32 udwHosNum = udwAfterUserHosNum - udwBeforUserHosNum;

        //player info 统计 不统计支援类action的兵力情况
        CWarProcess::ComputeWarResult(pstSession, &stAttack, &stDefend, pstSession->m_dwMarchResult, udwHosNum);

        TINT32 dwReportResult = pstSession->m_dwMarchResult;
        CProcessReport::GenAttackReport(pstSession, EN_REPORT_TYPE_RALLY_WAR, dwReportResult, &stAttack, &stDefend, &pstSession->m_tbReport);

        //若是进攻王座 则 rallywar 已经处理
        if(ptbWild->m_nType == EN_WILD_TYPE__CITY)
        {
            ptbRallyWar->Set_Status(EN_MARCH_STATUS__RETURNING);
            ptbRallyWar->Set_Btime(CTimeUtils::GetUnixTime());
            ptbRallyWar->Set_Ctime(ptbRallyWar->m_bParam[0].m_ddwMarchingTime);
            ptbRallyWar->Set_Etime(ptbRallyWar->m_nBtime + ptbRallyWar->m_bParam[0].m_ddwMarchingTime);
            //ptbRallyWar->DeleteField(TbMARCH_ACTION_FIELD_SAL);
            //ptbRallyWar->DeleteField(TbMARCH_ACTION_FIELD_TAL);
            ptbRallyWar->Set_Sal(0, UPDATE_ACTION_TYPE_DELETE);
            ptbRallyWar->Set_Tal(0, UPDATE_ACTION_TYPE_DELETE);
        }

        //设置抓龙和战果标志
        if (stAttack.m_stDragon.m_ddwCaptured >= 0 || stDefend.m_stDragon.m_ddwCaptured >= 0)
        {
            ptbRallyWar->m_bParam[0].m_ddwCaptureDragonFlag = 1;
            ptbRallyWar->m_bParam[0].m_ddwWinFlag = pstSession->m_dwMarchResult == EN_REPORT_RESULT_WIN ? 1 : 0;
            ptbRallyWar->SetFlag(TbMARCH_ACTION_FIELD_PARAM);
        }

        //Tips
        ostringstream oss;
        oss.str("");
        for(TUINT32 udwIdx = 0; udwIdx < EN_RESOURCE_TYPE__END; ++udwIdx)
        {
            if(udwIdx > 0)
            {
                oss << "|";
            }
            oss << ptbRallyWar->m_bParam[0].m_stResource[udwIdx];
        }
        oss << "|" << ptbRallyWar->m_bParam[0].m_ddwLoadGem;

        string sResourceInfo = oss.str();

        TUINT32 udwWinOrLose = pstSession->m_dwMarchResult == EN_REPORT_RESULT_WIN ? 0 : 1;

        //给发起者tips
        CSendMessageBase::AddTips(&pstSession->m_stSourceUser, EN_TIPS_TYPE__WAR_RESULT, ptbSPlayer->m_nUid, TRUE,
            ptbWild->m_nId, ptbWild->m_nUid, udwWinOrLose, ptbWild->m_sUname.c_str(), sResourceInfo.c_str());
        //给支援者tips
        for(TUINT32 udwIdx = 0; udwIdx < stAttack.m_udwRallyReinforceNum; ++udwIdx)
        {
            //CSendMessageBase::AddTips(&pstSession->m_stSourceUser, EN_TIPS_TYPE__WAR_RESULT, stAttack.m_pastRallyReinforceList[udwIdx]->m_nSuid, TRUE,
            //    ptbWild->m_nId, ptbWild->m_nUid, udwWinOrLose, ptbWild->m_sUname.c_str(), sResourceInfo.c_str());
        }

        CMsgBase::SendEncourageMail(&pstSUser->m_tbUserStat, pstSUser->m_tbPlayer.m_nSid, EN_MAIL_ID__FIRST_RALLY_WAR_FINISH);

        if(udwWildRawUId)
        {
            //防守方打输了 也要弹tips
            TUINT32 udwTargetWinOrLose = pstSession->m_dwMarchResult == EN_REPORT_RESULT_LOSE ? 2 : 4;
            //给防守者tips
            CSendMessageBase::AddTips(&pstSession->m_stTargetUser, EN_TIPS_TYPE__WAR_RESULT, udwWildRawUId, TRUE,
                ptbWild->m_nId, ptbSPlayer->m_nUid, udwTargetWinOrLose,
                ptbSPlayer->m_sUin.c_str(), sResourceInfo.c_str());
            //给支援者tips
            for(TUINT32 udwIdx = 0; udwIdx < stDefend.m_udwEncampNum; udwIdx++)
            {
                if(stDefend.m_pastEncampActionList[udwIdx]->m_nSclass == EN_ACTION_SEC_CLASS__OCCUPY)
                {
                    continue;
                }
                if(stDefend.m_pastEncampActionList[udwIdx]->m_nSuid == udwWildRawUId)
                {
                    continue;
                }
                //CSendMessageBase::AddTips(&pstSession->m_stTargetUser, EN_TIPS_TYPE__WAR_RESULT, stDefend.m_pastEncampActionList[udwIdx]->m_nSuid, TRUE,
                //    ptbWild->m_nId, ptbSPlayer->m_nUid, udwTargetWinOrLose,
                //    ptbSPlayer->m_sUin.c_str(), sResourceInfo.c_str());
            }
            //for(TUINT32 udwIdx = 0; udwIdx < stDefend.m_udwRallyReinforceNum; ++udwIdx)
            //{
                //CSendMessageBase::AddTips(&pstSession->m_stTargetUser, EN_TIPS_TYPE__WAR_RESULT, stDefend.m_pastRallyReinforceList[udwIdx]->m_nSuid, TRUE,
                //    ptbWild->m_nId, ptbSPlayer->m_nUid, udwTargetWinOrLose,
                //    ptbSPlayer->m_sUin.c_str(), sResourceInfo.c_str());
            //}

            for(TUINT32 udwIdx = 0; udwIdx < EN_TROOP_TYPE__END; ++udwIdx)
            {
                pstSUser->m_tbUserStat.m_nKill_troop_num += ptbRallyWar->m_bParam[0].m_stTroopRaw[udwIdx] - ptbRallyWar->m_bParam[0].m_stTroop[udwIdx];
            }
            pstSUser->m_tbUserStat.SetFlag(TbUSER_STAT_FIELD_KILL_TROOP_NUM);

            SNoticInfo stNoticInfo;
            if(pstSession->m_dwMarchResult == EN_REPORT_RESULT_WIN)
            {
                stNoticInfo.Reset();
                stNoticInfo.m_dwNoticId = EN_NOTI_ID__RALLY_ATTACK_SUCC;
                stNoticInfo.m_strTName = pstSession->m_stTargetUser.m_tbPlayer.m_sUin;
                stNoticInfo.m_strLang = CDocument::GetLang(pstSUser->m_tbLogin.m_nLang);
                if(ptbSPlayer->m_nAlid > 0 && ptbSPlayer->m_nAlpos > EN_ALLIANCE_POS__REQUEST)
                {
                    for (TUINT32 udwIdx = 0; udwIdx < pstSUser->m_udwMailSendNum; udwIdx++)
                    {
                        CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), ptbSPlayer->m_nSid, pstSUser->m_adwMailSendUidList[udwIdx], stNoticInfo);
                    }
                }
                else
                {
                    CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), ptbSPlayer->m_nSid, ptbSPlayer->m_nUid, stNoticInfo);
                }

                stNoticInfo.Reset();
                stNoticInfo.m_dwNoticId = EN_NOTI_ID__RALLY_DEFENSE_FAIL;
                stNoticInfo.m_strTName = ptbSPlayer->m_sUin;
                stNoticInfo.m_strLang = CDocument::GetLang(pstSession->m_stTargetUser.m_tbLogin.m_nLang);
                if(pstSession->m_stTargetUser.m_tbPlayer.m_nAlid > 0 && pstSession->m_stTargetUser.m_tbPlayer.m_nAlpos > EN_ALLIANCE_POS__REQUEST)
                {
//                     CMsgBase::SendNotificationAlliance(CConfBase::GetString("project"),
//                         pstSession->m_stTargetUser.m_tbPlayer.m_nSid,
//                         pstSession->m_stTargetUser.m_tbPlayer.m_nUid,
//                         pstSession->m_stTargetUser.m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET,
//                         stNoticInfo);
                    for (TUINT32 udwIdx = 0; udwIdx < pstTUser->m_udwMailSendNum; udwIdx++)
                    {
                        CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), pstTUser->m_tbPlayer.m_nSid, pstTUser->m_adwMailSendUidList[udwIdx], stNoticInfo);
                    }
                }
                else
                {
                    CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"),
                        pstSession->m_stTargetUser.m_tbPlayer.m_nSid,
                        pstSession->m_stTargetUser.m_tbPlayer.m_nUid,
                        stNoticInfo);
                }

                //冒烟和冒火逻辑
                if(ptbWild->m_nType == EN_WILD_TYPE__CITY)
                {
                    CQuestLogic::SetTaskCurValue(pstSUser, pstCity, EN_TASK_TYPE_ING_ATTACK_CITY_NUM);

                    CWarProcess::GenMoveCityAction(pstSession, ptbWild, &stDefend);
                    pstSession->m_stTargetUser.m_tbPlayer.Set_Status(pstSession->m_stTargetUser.m_tbPlayer.m_nStatus | EN_CITY_STATUS__BEEN_ATTACK);

                    ptbWild->Set_Smoke_end_time(CTimeUtils::GetUnixTime() + 30 * 60);
                    ptbWild->Set_Status(pstSession->m_stTargetUser.m_tbPlayer.Get_Status());
                    pstSession->m_ucMapItemFlag = EN_TABLE_UPDT_FLAG__CHANGE;
                }

            }
            else
            {
                stNoticInfo.Reset();
                stNoticInfo.m_dwNoticId = EN_NOTI_ID__RALLY_ATTACK_FAIL;
                stNoticInfo.m_strTName = pstSession->m_stTargetUser.m_tbPlayer.m_sUin;
                if(pstSession->m_stTargetUser.m_tbPlayer.m_nUid == 0)
                {
                }
                stNoticInfo.m_strLang = CDocument::GetLang(pstSUser->m_tbLogin.m_nLang);
                if(ptbSPlayer->m_nAlid > 0 && ptbSPlayer->m_nAlpos > EN_ALLIANCE_POS__REQUEST)
                {
                    for (TUINT32 udwIdx = 0; udwIdx < pstSUser->m_udwMailSendNum; udwIdx++)
                    {
                        CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), ptbSPlayer->m_nSid, pstSUser->m_adwMailSendUidList[udwIdx], stNoticInfo);
                    }
                }
                else
                {
                    CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), ptbSPlayer->m_nSid, ptbSPlayer->m_nUid, stNoticInfo);
                }

                stNoticInfo.Reset();
                stNoticInfo.m_dwNoticId = EN_NOTI_ID__RALLY_DEFENSE_SUCC;
                stNoticInfo.m_strTName = ptbSPlayer->m_sUin;
                stNoticInfo.m_strLang = CDocument::GetLang(pstSession->m_stTargetUser.m_tbLogin.m_nLang);
                if(pstSession->m_stTargetUser.m_tbPlayer.m_nAlid > 0 && pstSession->m_stTargetUser.m_tbPlayer.m_nAlpos > EN_ALLIANCE_POS__REQUEST)
                {
                    for (TUINT32 udwIdx = 0; udwIdx < pstTUser->m_udwMailSendNum; udwIdx++)
                    {
                        CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), pstTUser->m_tbPlayer.m_nSid, pstTUser->m_adwMailSendUidList[udwIdx], stNoticInfo);
                    }
                }
                else
                {
                    CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"),
                        pstSession->m_stTargetUser.m_tbPlayer.m_nSid,
                        pstSession->m_stTargetUser.m_tbPlayer.m_nUid,
                        stNoticInfo);
                }

                //冒烟和冒火逻辑
                if(ptbWild->m_nType == EN_WILD_TYPE__CITY)
                {
                    CQuestLogic::SetTaskCurValue(pstSUser, pstCity, EN_TASK_TYPE_ING_ATTACK_CITY_NUM);

                    pstSession->m_stTargetUser.m_tbPlayer.Set_Status(pstSession->m_stTargetUser.m_tbPlayer.m_nStatus | EN_CITY_STATUS__BEEN_ATTACK);

                    //daemon TODO 修改时间 方便测试
                    ptbWild->Set_Smoke_end_time(CTimeUtils::GetUnixTime() + 30 * 60);
                    ptbWild->Set_Status(pstSession->m_stTargetUser.m_tbPlayer.Get_Status());
                    pstSession->m_ucMapItemFlag = EN_TABLE_UPDT_FLAG__CHANGE;
                }
            }
        }
    }
    else
    {
        CProcessReport::GenInValidReport(pstSession, EN_REPORT_TYPE_RALLY_WAR, dwReason, &pstSession->m_tbReport);

        ptbRallyWar->Set_Status(EN_MARCH_STATUS__RETURNING);
        ptbRallyWar->Set_Btime(ptbRallyWar->m_nEtime);
        ptbRallyWar->Set_Etime(ptbRallyWar->m_nBtime + ptbRallyWar->m_bParam[0].m_ddwMarchingTime);
        //ptbRallyWar->DeleteField(TbMARCH_ACTION_FIELD_SAL);
        //ptbRallyWar->DeleteField(TbMARCH_ACTION_FIELD_TAL);
        ptbRallyWar->Set_Sal(0, UPDATE_ACTION_TYPE_DELETE);
        ptbRallyWar->Set_Tal(0, UPDATE_ACTION_TYPE_DELETE);

        for(TUINT32 udwIdx = 0; udwIdx < pstSUser->m_udwMarchNum; ++udwIdx)
        {
            TbMarch_action* ptbRallyReinforce = &pstSUser->m_atbMarch[udwIdx];
            if(ptbRallyReinforce->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_REINFORCE
                && ptbRallyReinforce->m_nStatus == EN_MARCH_STATUS__PREPARING
                && ptbRallyReinforce->m_nTid == ptbRallyWar->m_nId)
            {
                CActionBase::ReturnMarch(ptbRallyReinforce, ptbRallyWar->m_nTpos, ptbRallyWar->m_bParam[0].m_ddwTargetType);
                // 添加盟友idlist，用于发送report
                CToolBase::AddUserToMailReceiverList(pstSUser->m_adwMailSendUidList, pstSUser->m_udwMailSendNum, ptbRallyReinforce->m_nSuid);
            }
        }

        for(TUINT32 udwIdx = 0; udwIdx < pstTUser->m_udwMarchNum; ++udwIdx)
        {
            TbMarch_action* ptbRallyReinforce = &pstTUser->m_atbMarch[udwIdx];
            if(ptbRallyReinforce->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_REINFORCE
            && ptbRallyReinforce->m_nStatus == EN_MARCH_STATUS__PREPARING
            && ptbRallyReinforce->m_nTid == ptbRallyWar->m_nId)
            {
                CActionBase::ReturnMarch(ptbRallyReinforce);
                // 添加盟友idlist，用于发送report
                CToolBase::AddUserToMailReceiverList(pstSUser->m_adwMailSendUidList, pstSUser->m_udwMailSendNum, ptbRallyReinforce->m_nSuid);
            }
            if(ptbRallyReinforce->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_REINFORCE
            && ptbRallyReinforce->m_nStatus == EN_MARCH_STATUS__MARCHING
            && ptbRallyReinforce->m_nTid == ptbRallyWar->m_nId)
            {
                CActionBase::ReturnMarchOnFly(ptbRallyReinforce);
                // 添加盟友idlist，用于发送report
                CToolBase::AddUserToMailReceiverList(pstSUser->m_adwMailSendUidList, pstSUser->m_udwMailSendNum, ptbRallyReinforce->m_nSuid);
            }
        }
    }

    //添加rallywar history
    pstSession->m_bNeedRallyHistory = TRUE;
    pstSession->m_atbTmpRallyHistory[0].Set_Aid(pstSUser->m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET);
    if(pstTUser->m_tbPlayer.m_nAlpos > 0)
    {
        pstSession->m_atbTmpRallyHistory[1].Set_Aid(pstTUser->m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET);
    }
    else
    {
        pstSession->m_atbTmpRallyHistory[1].Set_Aid(-1 * pstTUser->m_tbPlayer.m_nUid);
    }
    Json::Value jsonContent = Json::Value(Json::objectValue);
    jsonContent["attacker"] = Json::Value(Json::arrayValue);
    jsonContent["attacker"].append(pstSUser->m_tbAlliance.m_nAid);
    jsonContent["attacker"].append(pstSUser->m_tbAlliance.m_sAl_nick_name);
    jsonContent["attacker"].append(pstSUser->m_tbPlayer.m_sUin);
    jsonContent["attacker"].append(pstSession->m_dwMarchResult == EN_REPORT_RESULT_WIN ? EN_REPORT_RESULT_WIN : EN_REPORT_RESULT_LOSE);
    jsonContent["attacker"].append(EN_WILD_TYPE__CITY);
    jsonContent["defender"] = Json::Value(Json::arrayValue);
    jsonContent["defender"].append(pstTUser->m_tbAlliance.m_nAid);
    jsonContent["defender"].append(pstTUser->m_tbAlliance.m_sAl_nick_name);
    jsonContent["defender"].append(pstTUser->m_tbPlayer.m_sUin);
    jsonContent["defender"].append(pstSession->m_dwMarchResult == EN_REPORT_RESULT_WIN ? EN_REPORT_RESULT_LOSE : EN_REPORT_RESULT_WIN);
    jsonContent["defender"].append(ptbWild->m_nType);
    jsonContent["time"] = CTimeUtils::GetUnixTime();
    for(TUINT32 udwIdx = 0; udwIdx < 2; ++udwIdx)
    {
        pstSession->m_atbTmpRallyHistory[udwIdx].Set_Content(jsonContent);
    }

    // 更新action状态
    pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;
    return 0;
}

TINT32 CProcessAction::ProcessMarchRallyPreparing(SSession* pstSession)
{
    TbMarch_action *ptbRallyWar = &pstSession->m_stReqMarch;
    SUserInfo *pstSUser = &pstSession->m_stSourceUser;
    SCityInfo *pstCity = &pstSUser->m_stCityInfo;

    TBOOL bNeedWait = FALSE;
    TINT64 ddwEtime = 0;
    for(TUINT32 udwIdx = 0; udwIdx < pstSUser->m_udwPassiveMarchNum; ++udwIdx)
    {
        if(pstSUser->m_atbPassiveMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__RALLY_REINFORCE
            && pstSUser->m_atbPassiveMarch[udwIdx].m_nTid == ptbRallyWar->m_nId
            && pstSUser->m_atbPassiveMarch[udwIdx].m_nStatus == EN_MARCH_STATUS__MARCHING)
        {
            bNeedWait = TRUE;
            if (ddwEtime < pstSUser->m_atbPassiveMarch[udwIdx].m_nEtime)
            {
                ddwEtime = pstSUser->m_atbPassiveMarch[udwIdx].m_nEtime;
            }
        }
    }

    if(bNeedWait == TRUE)
    {
        ptbRallyWar->Set_Status(EN_MARCH_STATUS__WAITING);
        ptbRallyWar->Set_Etime(ddwEtime);

        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchRallyPreparing: rallywar need wait[id=%ld][seq=%u]",
            ptbRallyWar->m_nId,
            pstSession->m_udwSeqNo));
    }
    else
    {
        if (ptbRallyWar->m_bParam[0].m_ddwSourceCityId != ptbRallyWar->m_nScid
            || ptbRallyWar->m_nTpos != ptbRallyWar->m_bParam[0].m_ddwTargetCityId)
        {
            if (ptbRallyWar->m_bParam[0].m_stDragon.m_ddwLevel)
            {
                ptbRallyWar->m_bParam[0].m_ddwMarchingTime = CCommonBase::GetTroopRallyWarTimeNew(&ptbRallyWar->m_bParam[0].m_stTroop,
                    ptbRallyWar->m_nScid, ptbRallyWar->m_bParam[0].m_ddwTargetCityId, &pstSUser->m_stPlayerBuffList);
            }
            else
            {
                ptbRallyWar->m_bParam[0].m_ddwMarchingTime = CCommonBase::GetTroopRallyWarTimeNew(&ptbRallyWar->m_bParam[0].m_stTroop,
                    ptbRallyWar->m_nScid, ptbRallyWar->m_bParam[0].m_ddwTargetCityId, &pstSUser->m_stBuffWithoutDragon);
            }
            ptbRallyWar->SetFlag(TbMARCH_ACTION_FIELD_PARAM);

            if (ptbRallyWar->m_nScid != ptbRallyWar->m_bParam[0].m_ddwSourceCityId)
            {
                ptbRallyWar->m_bParam[0].m_ddwSourceCityId = ptbRallyWar->m_nScid;
            }

            if (ptbRallyWar->m_nTpos != ptbRallyWar->m_bParam[0].m_ddwTargetCityId)
            {
                ptbRallyWar->Set_Tpos(ptbRallyWar->m_bParam[0].m_ddwTargetCityId);
                ptbRallyWar->Set_Tbid(CMapBase::GetBlockIdFromPos(ptbRallyWar->m_bParam[0].m_ddwTargetCityId));
            }
        }

        ptbRallyWar->Set_Status(EN_MARCH_STATUS__MARCHING);
        ptbRallyWar->Set_Btime(ptbRallyWar->m_nEtime);
        ptbRallyWar->Set_Ctime(ptbRallyWar->m_bParam[0].m_ddwMarchingTime);
        ptbRallyWar->Set_Etime(ptbRallyWar->m_nBtime + ptbRallyWar->m_bParam[0].m_ddwMarchingTime);

        for(TUINT32 udwIdx = 0; udwIdx < pstSUser->m_udwMarchNum; udwIdx++)
        {
            if((pstSUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__RALLY_REINFORCE && pstSUser->m_atbMarch[udwIdx].m_nTid == ptbRallyWar->m_nId)
                || (pstSUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR && pstSUser->m_atbMarch[udwIdx].m_nId == ptbRallyWar->m_nId))
            {
                CSendMessageBase::AddTips(&pstSession->m_stSourceUser, EN_TIPS_TYPY__RALLY_WAR_BEGIN, pstSUser->m_atbMarch[udwIdx].m_nSuid, TRUE,
                    pstSession->m_stMapItem.m_nType, pstSession->m_stMapItem.m_nLevel, 0, pstSession->m_stTargetUser.m_tbPlayer.m_sUin.c_str());
            }
        }

        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchRallyPreparing: rallywar start march[id=%ld][seq=%u]",
            ptbRallyWar->m_nId,
            pstSession->m_udwSeqNo));
    }

    // 更新action状态
    pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;

    return 0;
}

TINT32 CProcessAction::ProcessMarchRallyThronePreparing(SSession* pstSession)
{
    TbMarch_action *ptbRallyWar = &pstSession->m_stReqMarch;
    SUserInfo *pstSUser = &pstSession->m_stSourceUser;

    TBOOL bNeedWait = FALSE;
    TINT64 ddwEtime = 0;
    for (TUINT32 udwIdx = 0; udwIdx < pstSUser->m_udwPassiveMarchNum; ++udwIdx)
    {
        if (pstSUser->m_atbPassiveMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__RALLY_REINFORCE
            && pstSUser->m_atbPassiveMarch[udwIdx].m_nTid == ptbRallyWar->m_nId
            && pstSUser->m_atbPassiveMarch[udwIdx].m_nStatus == EN_MARCH_STATUS__MARCHING)
        {
            bNeedWait = TRUE;
            if (ddwEtime < pstSUser->m_atbPassiveMarch[udwIdx].m_nEtime)
            {
                ddwEtime = pstSUser->m_atbPassiveMarch[udwIdx].m_nEtime;
            }
        }
    }

    if (bNeedWait == TRUE)
    {
        ptbRallyWar->Set_Status(EN_MARCH_STATUS__WAITING);
        ptbRallyWar->Set_Etime(ddwEtime);

        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchRallyThronePreparing: rallywar need wait[id=%ld][seq=%u]",
            ptbRallyWar->m_nId,
            pstSession->m_udwSeqNo));
    }
    else
    {
        ptbRallyWar->Set_Status(EN_MARCH_STATUS__MARCHING);
        ptbRallyWar->Set_Btime(ptbRallyWar->m_nEtime);
        ptbRallyWar->Set_Ctime(ptbRallyWar->m_bParam[0].m_ddwMarchingTime);
        ptbRallyWar->Set_Etime(ptbRallyWar->m_nBtime + ptbRallyWar->m_bParam[0].m_ddwMarchingTime);

        for (TUINT32 udwIdx = 0; udwIdx < pstSUser->m_udwMarchNum; udwIdx++)
        {
            if ((pstSUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__RALLY_REINFORCE && pstSUser->m_atbMarch[udwIdx].m_nTid == ptbRallyWar->m_nId)
                || (pstSUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR_THRONE && pstSUser->m_atbMarch[udwIdx].m_nId == ptbRallyWar->m_nId))
            {
                CSendMessageBase::AddTips(&pstSession->m_stSourceUser, EN_TIPS_TYPY__RALLY_WAR_BEGIN, pstSUser->m_atbMarch[udwIdx].m_nSuid, TRUE,
                    pstSession->m_stMapItem.m_nType, pstSession->m_stMapItem.m_nLevel, 0, pstSession->m_stTargetUser.m_tbPlayer.m_sUin.c_str());
            }
        }

        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchRallyThronePreparing: rallywar start march[id=%ld][seq=%u]",
            ptbRallyWar->m_nId,
            pstSession->m_udwSeqNo));
    }

    // 更新action状态
    pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;

    return 0;
}

TINT32 CProcessAction::ProcessMarchRallyDeling(SSession* pstSession)
{
    TbMarch_action *ptbRallyWar = &pstSession->m_stReqMarch;
    SUserInfo *pstTUser = &pstSession->m_stTargetUser;

    if(ptbRallyWar->m_nStatus == EN_MARCH_STATUS__DELING_ON_FLY)
    {
        ptbRallyWar->m_nEtime = ptbRallyWar->m_bParam[0].m_ddwLoadTime;
        CActionBase::ReturnMarchOnFly(ptbRallyWar);
    }
    else
    {
        CActionBase::ReturnMarch(ptbRallyWar);
        ptbRallyWar->Set_Etime(CTimeUtils::GetUnixTime());
    }

    pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;

    return 0;
}

TINT32 CProcessAction::ProcessTimer(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = -1;

    if(pstSession->m_stReqMarch.m_nSclass == EN_ACTION_SEC_CLASS__THRONE_TIMER)
    {
        dwRetCode = ProcessThroneTimer(pstSession, bNeedResponse);
    }
    else if(pstSession->m_stReqMarch.m_nSclass == EN_ACTION_SEC_CLASS__PRISON_TIMER)
    {
        dwRetCode = ProcessPrisonTimer(pstSession, bNeedResponse);
    }
    else if(pstSession->m_stReqMarch.m_nSclass == EN_ACTION_SEC_CLASS__NOTI_TIMER)
    {
        dwRetCode = ProcessNotiTimer(pstSession, bNeedResponse);
    }
    else if (pstSession->m_stReqMarch.m_nSclass == EN_ACTION_SEC_CLASS__TAX)
    {
        if (pstSession->m_stReqMarch.m_nStatus == EN_TAX_STATUS__PREPARING)
        {
            dwRetCode = ProcessTaxPreparing(pstSession, bNeedResponse);
        }
        else if (pstSession->m_stReqMarch.m_nStatus == EN_TAX_STATUS__COLLECTING)
        {
            dwRetCode = ProcessTaxCollecting(pstSession, bNeedResponse);
        }
        else if (pstSession->m_stReqMarch.m_nStatus == EN_TAX_STATUS__FINISH)
        {
            dwRetCode = ProcessTaxFinish(pstSession, bNeedResponse);
        }
        else if (pstSession->m_stReqMarch.m_nStatus == EN_TAX_STATUS__TRANSFER)
        {
            dwRetCode = ProcessTaxTransfer(pstSession, bNeedResponse);
        }
    }
    else if (pstSession->m_stReqMarch.m_nSclass == EN_ACTION_SEC_CLASS__IDOL_PERIOD)
    {
        if (pstSession->m_stReqMarch.m_nStatus == EN_TITMER_IDOL_PERIOD_STATUS__THRONE_PEACE_TIME)
        {
            dwRetCode = ProcessIdolThronePeaceTimePeriod(pstSession, bNeedResponse);
        }
        else if (pstSession->m_stReqMarch.m_nStatus == EN_TITMER_IDOL_PERIOD_STATUS__BUFF)
        {
            dwRetCode = ProcessIdolBuffPeriod(pstSession, bNeedResponse);
        }
        else if (pstSession->m_stReqMarch.m_nStatus == EN_TITMER_IDOL_PERIOD_STATUS__QUIET)
        {
            dwRetCode = ProcessIdolQuietPeriod(pstSession, bNeedResponse);
        }
        else if (pstSession->m_stReqMarch.m_nStatus == EN_TITMER_IDOL_PERIOD_STATUS__CONTEST)
        {
            dwRetCode = ProcessIdolContestPeriod(pstSession, bNeedResponse);
        }
    }
    else if (pstSession->m_stReqMarch.m_nSclass == EN_ACTION_SEC_CLASS__THRONE_PERIOD)
    {
        if (pstSession->m_stReqMarch.m_nStatus == EN_TITMER_THRONE_PERIOD_STATUS__PEACE_TIME)
        {
            dwRetCode = ProcessThronePeaceTimePeriod(pstSession, bNeedResponse);
        }
        else if (pstSession->m_stReqMarch.m_nStatus == EN_TITMER_THRONE_PERIOD_STATUS__CONTEST)
        {
            dwRetCode = ProcessThroneContestPeriod(pstSession, bNeedResponse);
        }
    }

    TSE_LOG_INFO(pstSession->m_poServLog, ("process_by_cmd: step[%u] main=%ld,sec=%ld,status=%ld,ret=%d, bNeedResponse[%d] [seq=%u]",
        pstSession->m_udwProcessSeq, pstSession->m_stReqMarch.m_nMclass, pstSession->m_stReqMarch.m_nSclass,
        pstSession->m_stReqMarch.m_nStatus, dwRetCode, bNeedResponse, pstSession->m_udwSeqNo));

    return dwRetCode;
}

TINT32 CProcessAction::ProcessThroneTimer(SSession *pstSession, TBOOL &bNeedResponse)
{
    //TODO
    return -1;
}

TINT32 CProcessAction::ProcessPrisonTimer(SSession *pstSession, TBOOL &bNeedResponse)
{
    TbMarch_action *ptbReqMarch = &pstSession->m_stReqMarch;
    SUserInfo *pstSUser = &pstSession->m_stSourceUser;
    TbPlayer* ptbSPlayer = &pstSUser->m_tbPlayer;
    SUserInfo *pstTUser = &pstSession->m_stTargetUser;
    TbPlayer* ptbTPlayer = &pstTUser->m_tbPlayer;
    TSE_LOG_INFO(pstSession->m_poServLog, ("prisontime: result[%ld] [seq=%u]",
        ptbReqMarch->m_bPrison_param[0].ddwResult, pstSession->m_udwSeqNo));
    if(ptbReqMarch->m_bPrison_param[0].ddwResult == EN_PRISON_RESULT_BE_KILLED)
    {
        pstSUser->m_tbPlayer.Set_Dragon_status(EN_DRAGON_STATUS_DEAD);
        pstSUser->m_tbPlayer.Set_Dragon_tid(0);

        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__DEL;
        return 0;
    }
    else if(ptbReqMarch->m_bPrison_param[0].ddwResult == EN_PRISON_RESULT_ESCAPE)
    {
        pstSUser->m_tbPlayer.Set_Dragon_status(EN_DRAGON_STATUS_MARCH);
        pstSUser->m_tbPlayer.Set_Dragon_tid(ptbReqMarch->m_nId);

        CCommonLogic::GenPrisonReport(ptbReqMarch, ptbSPlayer, EN_REPORT_TYPE_DRAGON_RELEASE, EN_REPORT_RESULT_DRAGON_ESCAPE, &pstSession->m_tbPrisonReport);
        pstSession->m_dwReportFlag |= EN_REPORT_FLAG_PRISON;
        pstSession->m_vecPrisonReportReceivers.insert(ptbReqMarch->m_nSuid);
        pstSession->m_vecPrisonReportReceivers.insert(ptbReqMarch->m_nTuid);

        CActionBase::PrisonToMarch(ptbReqMarch, pstSUser->m_stCityInfo.m_stTblData.m_nPos);
        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;

        //被抓的人增加逃跑记录
        ptbSPlayer->m_bDragon_statistics[0].ddwMyDragonEscapedNum++;
        ptbSPlayer->SetFlag(TbPLAYER_FIELD_DRAGON_STATISTICS);
        //抓捕的人增加放跑英雄记录
        ptbTPlayer->m_bDragon_statistics[0].ddwEscapeDragonNum++;
        ptbTPlayer->SetFlag(TbPLAYER_FIELD_DRAGON_STATISTICS);
        //发给被抓者tips
        CSendMessageBase::AddTips(&pstSession->m_stSourceUser, EN_TIPS_TYPY__HERO_IS_RELEASED, ptbSPlayer->m_nUid, TRUE, 0, 0, 0, ptbTPlayer->m_sUin.c_str());
        return 0;
    }
    else if(ptbReqMarch->m_bPrison_param[0].ddwResult == EN_PRISON_RESULT_BE_RELEASED)
    {
        pstSUser->m_tbPlayer.Set_Dragon_status(EN_DRAGON_STATUS_MARCH);
        pstSUser->m_tbPlayer.Set_Dragon_tid(ptbReqMarch->m_nId);
        CActionBase::PrisonToMarch(ptbReqMarch, pstSUser->m_stCityInfo.m_stTblData.m_nPos);
        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;

        //发给被抓者tips 不发....别处释放的时候已经发过了....
        //CSendMessageBase::AddTips(&pstSession->m_stSourceUser, EN_TIPS_TYPY__HERO_IS_RELEASED, ptbSPlayer->m_nUid, TRUE, 0, 0, 0, ptbTPlayer->m_sUin.c_str());

        return 0;
    }

    return 0;
}

TINT32 CProcessAction::ProcessNotiTimer(SSession *pstSession, TBOOL &bNeedResponse)
{
    TbMarch_action *ptbNotiTimer = &pstSession->m_stReqMarch;
    SUserInfo *pstUser = &pstSession->m_stSourceUser;
    TbQuest *ptbQuest = &pstUser->m_tbQuest;
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();

    if (pstUser->m_tbPlayer.m_nUtime < udwCurTime - 60 * 60 * 24 * 7)
    {
        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__DEL;
        return 0;
    }

    //building research 相关
    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwSelfAlActionNum; ++udwIdx)
    {
        if (pstUser->m_aucSelfAlActionFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }
        TbAlliance_action* ptbCanFreeAction = &pstUser->m_atbSelfAlAction[udwIdx];

        if (ptbCanFreeAction->m_nNoti_flag == TRUE)
        {
            continue;
        }

        if (ptbCanFreeAction->m_nSclass != EN_ACTION_SEC_CLASS__BUILDING_UPGRADE
            && ptbCanFreeAction->m_nSclass != EN_ACTION_SEC_CLASS__BUILDING_REMOVE
            && ptbCanFreeAction->m_nSclass != EN_ACTION_SEC_CLASS__RESEARCH_UPGRADE)
        {
            continue;
        }

        TINT64 ddwCanFreeTimeStamp = ptbCanFreeAction->m_nEtime -
            CCommonBase::GetGameBasicVal(EN_GAME_BASIC_FREE_INSTANT_TIME) -
            pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_FREE_INSTANT_FINISH_TIME].m_ddwBuffTotal;
        if (udwCurTime >= ddwCanFreeTimeStamp)
        {
            // 原本没有发notification
            ptbCanFreeAction->Set_Noti_flag(TRUE);
        }
    }

    //peacetime 相关
    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwActionNum; ++udwIdx)
    {
        if(pstUser->m_aucActionFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }
        TbAction* ptbPeacetime = &pstUser->m_atbAction[udwIdx];

        if(ptbPeacetime->m_nSclass != EN_ACTION_SEC_CLASS__ITEM)
        {
            continue;
        }

        if(ptbPeacetime->m_nNoti_flag == TRUE)
        {
            continue;
        }

        TUINT32 udwNotiId = EN_NOTI_ID__END;
        TUINT32 udwNeedNotiTimeStamp = ptbPeacetime->m_nEtime - 3600;
        switch(ptbPeacetime->m_bParam[0].m_stItem.m_ddwBufferId)
        {
        case EN_BUFFER_INFO_PEACE_TIME:
            udwNotiId = EN_NOTI_ID__PEACETIME_END;
            if(pstUser->m_tbPlayer.m_nStatus & EN_CITY_STATUS__NEW_PROTECTION)
            {
                udwNotiId = EN_NOTI_ID__NEW_PLAYER_END;
            }
            break;
        case EN_BUFFER_INFO_VIP_ACTIVATE:
            udwNotiId = EN_NOTI_ID__VIP_EXPIRED;
            break;
        case EN_BUFFER_INFO_ALL_TROOP_ATTACK:
        case EN_BUFFER_INFO_ALL_TROOP_LIFE:
        case EN_BUFFER_INFO_GOLD_PRODUCTION:
        case EN_BUFFER_INFO_FOOD_PRODUCTION:
        case EN_BUFFER_INFO_WOOD_PRODUCTION:
        case EN_BUFFER_INFO_STONE_PROTECTION:
        case EN_BUFFER_INFO_ORE_PROTECTION:
        case EN_BUFFER_INFO_QUEUE_NUM:
        case EN_BUFFER_INFO_TROOP_SIZE:
            udwNotiId = EN_NOTI_ID__BUFF_EXPIRED;
            break;
        default:
            break;
        }

        if(udwNotiId == EN_NOTI_ID__END)
        {
            continue;
        }

        if(udwCurTime > udwNeedNotiTimeStamp)
        {
            TINT64 ddwCostTime = ptbPeacetime->m_nEtime - udwCurTime;
            string strBuffName = CMsgBase::GetBuffInfoName(ptbPeacetime->m_bParam[0].m_stItem.m_ddwBufferId);
            SNoticInfo stNoticInfo;
            stNoticInfo.Reset();
            stNoticInfo.SetValue(udwNotiId,
                "", "",
                0, 0,
                0, 0,
                ddwCostTime, strBuffName.c_str(), 0);
            CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), pstUser->m_tbPlayer.m_nSid, pstUser->m_tbPlayer.m_nUid, stNoticInfo);

            ptbPeacetime->Set_Noti_flag(TRUE);
        }
    }

    //top quest
    //if(0 == BITTEST(ptbNotiTimer->m_bNotic_task_flag[0].m_bitFlag, EN_TOP_QUEST))
    //{
    //    TBOOL IsTopQuestClaim = FALSE;
    //    CQuestLogic::CheckTopQuestCanClaim(pstUser, IsTopQuestClaim);
    //    if(TRUE == IsTopQuestClaim)
    //    {
    //        TUINT32 udwNotiId = EN_NOTI_ID__QUEST_COMPLETE;
    //        SNoticInfo stNoticInfo;
    //        stNoticInfo.Reset();
    //        stNoticInfo.SetValue(udwNotiId,
    //            "", "",
    //            0, 0,
    //            0, 0,
    //            0, "", 0);
    //        CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), pstUser->m_tbPlayer.m_nSid, pstUser->m_tbPlayer.m_nUid, stNoticInfo);
    //        BITSET(ptbNotiTimer->m_bNotic_task_flag[0].m_bitFlag, EN_TOP_QUEST);
    //        ptbNotiTimer->SetFlag(TbMARCH_ACTION_FIELD_NOTIC_TASK_FLAG);
    //    }
    //}
  
    TBOOL IsQuestFinish = FALSE;

    //daily quest 
    if(0 == BITTEST(ptbNotiTimer->m_bNotic_task_flag[0].m_bitFlag, EN_DAILY_QUEST))
    {
        CQuestLogic::HasFinishQuestNode(&ptbQuest->m_bDaily_quest[0], IsQuestFinish);
        if(TRUE == IsQuestFinish)
        {
            TUINT32 udwNotiId = EN_NOTI_ID__QUEST_COMPLETE;
            SNoticInfo stNoticInfo;
            stNoticInfo.Reset();
            stNoticInfo.SetValue(udwNotiId,
                "", "",
                0, 0,
                0, 0,
                0, "", 0);
            CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), pstUser->m_tbPlayer.m_nSid, pstUser->m_tbPlayer.m_nUid, stNoticInfo);
            BITSET(ptbNotiTimer->m_bNotic_task_flag[0].m_bitFlag, EN_DAILY_QUEST);
            ptbNotiTimer->SetFlag(TbMARCH_ACTION_FIELD_NOTIC_TASK_FLAG);

            //tips
            CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPE__DAILY_QUEST_OK, pstUser->m_tbPlayer.m_nUid, TRUE, 0, 0, 0);
        }
    }
    //alliance quest 
    if(0 == BITTEST(ptbNotiTimer->m_bNotic_task_flag[0].m_bitFlag, EN_ALLIANCE_QUEST))
    {
        CQuestLogic::HasFinishQuestNode(&ptbQuest->m_bAl_quest[0],  IsQuestFinish);
        if(TRUE == IsQuestFinish)
        {
            TUINT32 udwNotiId = EN_NOTI_ID__QUEST_COMPLETE;
            SNoticInfo stNoticInfo;
            stNoticInfo.Reset();
            stNoticInfo.SetValue(udwNotiId,
                "", "",
                0, 0,
                0, 0,
                0, "", 0);
            CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), pstUser->m_tbPlayer.m_nSid, pstUser->m_tbPlayer.m_nUid, stNoticInfo);
            BITSET(ptbNotiTimer->m_bNotic_task_flag[0].m_bitFlag, EN_ALLIANCE_QUEST);
            ptbNotiTimer->SetFlag(TbMARCH_ACTION_FIELD_NOTIC_TASK_FLAG);

            //tips
            CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPE__ALLIANCE_QUEST_OK, pstUser->m_tbPlayer.m_nUid, TRUE, 0, 0, 0);
        }
    }
    //vip quest 
    if(0 == BITTEST(ptbNotiTimer->m_bNotic_task_flag[0].m_bitFlag, EN_VIP_QUEST))
    {
        CQuestLogic::HasFinishQuestNode(&ptbQuest->m_bVip_quest[0], IsQuestFinish);
        if(TRUE == IsQuestFinish)
        {
            TUINT32 udwNotiId = EN_NOTI_ID__QUEST_COMPLETE;
            SNoticInfo stNoticInfo;
            stNoticInfo.Reset();
            stNoticInfo.SetValue(udwNotiId,
                "", "",
                0, 0,
                0, 0,
                0, "", 0);
            CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), pstUser->m_tbPlayer.m_nSid, pstUser->m_tbPlayer.m_nUid, stNoticInfo);
            BITSET(ptbNotiTimer->m_bNotic_task_flag[0].m_bitFlag, EN_VIP_QUEST);
            ptbNotiTimer->SetFlag(TbMARCH_ACTION_FIELD_NOTIC_TASK_FLAG);

            //tips
            CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPE__VIP_QUEST_OK, pstUser->m_tbPlayer.m_nUid, TRUE, 0, 0, 0);
        }
    }
    //mistery quest 
    if(0 == BITTEST(ptbNotiTimer->m_bNotic_task_flag[0].m_bitFlag, EN_MISTERY_QUEST))
    {
        CQuestLogic::HasFinishQuestNode(&ptbQuest->m_bTimer_gift[0], IsQuestFinish);
        if(TRUE == IsQuestFinish)
        {
            TUINT32 udwNotiId = EN_NOTI_ID__MYSTERY_GIFT;
            SNoticInfo stNoticInfo;
            stNoticInfo.Reset();
            stNoticInfo.SetValue(udwNotiId,
                "", "",
                0, 0,
                0, 0,
                0, "", 0);
            CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), pstUser->m_tbPlayer.m_nSid, pstUser->m_tbPlayer.m_nUid, stNoticInfo);
            BITSET(ptbNotiTimer->m_bNotic_task_flag[0].m_bitFlag, EN_MISTERY_QUEST);
            ptbNotiTimer->SetFlag(TbMARCH_ACTION_FIELD_NOTIC_TASK_FLAG);
        }
    }

    ptbNotiTimer->Set_Etime(INT64_MAX);
    pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;
    return 0;
}

TINT32 CProcessAction::ProcessAuPress(SSession *pstSession, TBOOL &bNeedResponse)
{
    //wave@20160402:TODO
    /*
    TbMap& tbTmpMap = pstSession->m_tbTmpMap;

    TbAction *ptbAction = &pstSession->m_stReqAction;
    SUserInfo *pstUser = &pstSession->m_stSourceUser;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TbAlliance *pstAlliance = &pstSession->m_stSourceUser.m_tbAlliance;
    
    TUINT32 udwRandStatus = rand() % 5;
    // process
    if(ptbAction->m_nStatus == EN_MARCH_STATUS__MARCHING)
    {
        //修改城市名
        pstCity->m_stTblData.Set_Name("Xesiafa");

        ptbAction->Set_Status(udwRandStatus);
        ptbAction->Set_Btime(CTimeUtils::GetUnixTime());
        ptbAction->Set_Etime(CTimeUtils::GetUnixTime());
        pstSession->m_ucReqActionFlag = EN_TABLE_UPDT_FLAG__CHANGE;
        
    }
    else if(ptbAction->m_nStatus == EN_MARCH_STATUS__FIGHTING)
    {
        //backpack 
        CBackpack::AddCrystal(pstUser,11);

        ptbAction->Set_Status(udwRandStatus);
        ptbAction->Set_Btime(CTimeUtils::GetUnixTime());
        ptbAction->Set_Etime(CTimeUtils::GetUnixTime());
        pstSession->m_ucReqActionFlag = EN_TABLE_UPDT_FLAG__CHANGE;
    }
    else if(ptbAction->m_nStatus == EN_MARCH_STATUS__RETURNING)
    {
        //login
        CPlayerBase::AddGem(&pstUser->m_tbLogin,1);

        ptbAction->Set_Status(udwRandStatus);
        ptbAction->Set_Btime(CTimeUtils::GetUnixTime());
        ptbAction->Set_Etime(CTimeUtils::GetUnixTime());
        pstSession->m_ucReqActionFlag = EN_TABLE_UPDT_FLAG__CHANGE;
    }
    else if(ptbAction->m_nStatus == EN_MARCH_STATUS__DEFENDING)
    {
        //quest 
        pstUser->m_tbQuest.Set_Task_refresh_time(CTimeUtils::GetUnixTime() - 10);

        ptbAction->Set_Status(udwRandStatus);
        ptbAction->Set_Btime(CTimeUtils::GetUnixTime());
        ptbAction->Set_Etime(CTimeUtils::GetUnixTime());
        pstSession->m_ucReqActionFlag = EN_TABLE_UPDT_FLAG__CHANGE;
    }
    else if(ptbAction->m_nStatus == EN_MARCH_STATUS__LOADING)
    {
        //修改玩家体力值
        pstUser->m_tbPlayer.Set_Cur_energy(100 + rand() % 100);

        ptbAction->Set_Status(udwRandStatus);
        ptbAction->Set_Btime(CTimeUtils::GetUnixTime());
        ptbAction->Set_Etime(CTimeUtils::GetUnixTime());
        pstSession->m_ucReqActionFlag = EN_TABLE_UPDT_FLAG__CHANGE;
    }
    else
    {
        ptbAction->Set_Status(udwRandStatus);
        ptbAction->Set_Btime(CTimeUtils::GetUnixTime());
        ptbAction->Set_Etime(CTimeUtils::GetUnixTime());
        pstSession->m_ucReqActionFlag = EN_TABLE_UPDT_FLAG__CHANGE;
    }

    TSE_LOG_INFO(pstSession->m_poServLog, ("process_by_cmd: step[%u] main=%ld,sec=%ld,status=%ld,ret=%d, bNeedResponse[%d] [seq=%u]",
        pstSession->m_udwProcessSeq, ptbAction->m_nMclass, ptbAction->m_nSclass,
        ptbAction->m_nStatus, 0, bNeedResponse, pstSession->m_udwSeqNo));
    */
    return 0;
}

TINT32 CProcessAction::ProcessMarchCamp(SSession *pstSession)
{
    TbMarch_action *ptbReqMarch = &pstSession->m_stReqMarch;
    TbMap *ptbWild = &pstSession->m_stMapItem;
    TBOOL bSuccess = TRUE;
    TINT32 dwReason = EN_REPORT_RESULT_TARGET_HAS_OCCUPIED;

    if (bSuccess && ptbWild->m_nType != EN_WILD_TYPE__NORMAL)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchCamp: target[%ld] type[%ld] is not normal_type, just return... [seq=%u]",
            ptbWild->m_nId, ptbWild->m_nType, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_WILD_NOT_NORMAL;
    }
    if (bSuccess && ptbWild->m_nUid != 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchCamp: target[%ld] is get by user[%ld], just return... [seq=%u]",
            ptbWild->m_nId, ptbWild->m_nUid, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_WILD_NOT_NORMAL;
    }

    // 更新action状态
    if (bSuccess == TRUE)
    {
        ptbReqMarch->Set_Status(EN_MARCH_STATUS__SETUP_CAMP);
        ptbReqMarch->Set_Btime(ptbReqMarch->m_nEtime);
        ptbReqMarch->Set_Etime(ptbReqMarch->m_nBtime + 3);
        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;
    }
    else
    {
        CProcessReport::GenInValidReport(pstSession, EN_REPORT_TYPE_CAMP, dwReason, &pstSession->m_tbReport);

        ptbReqMarch->Set_Status(EN_MARCH_STATUS__RETURNING);
        ptbReqMarch->Set_Btime(ptbReqMarch->m_nEtime);
        ptbReqMarch->Set_Etime(ptbReqMarch->m_nBtime + ptbReqMarch->m_nCtime);
        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;
    }

    return 0;
}

TINT32 CProcessAction::ProcessTaxPreparing(SSession *pstSession, TBOOL &bNeedResponse)
{
    TbMarch_action *ptbTax = &pstSession->m_stReqMarch;
    SUserInfo *pstUser = &pstSession->m_stSourceUser;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;
    TbAlliance *ptbAlliance = &pstUser->m_tbAlliance;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        TINT64 ddwAlid = 0;
        if (ptbTax->m_jTax_info.isObject() && ptbTax->m_jTax_info.isMember("self_alid"))
        {
            ddwAlid = ptbTax->m_jTax_info["self_alid"].asInt64();
        }
        if (ddwAlid != 0 && pstSession->m_tbThrone.m_nAlid == ddwAlid
            && pstSession->m_tbThrone.m_nOwner_id != ptbPlayer->m_nUid)
        {
            ptbTax->Set_Tuid(pstSession->m_tbThrone.m_nOwner_id);
            ptbTax->Set_Status(EN_TAX_STATUS__TRANSFER);
            ptbTax->Set_Etime(CTimeUtils::GetUnixTime());

            pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;
            return 0;
        }

        if (pstSession->m_tbThrone.m_nAlid != ptbAlliance->m_nAid
            || pstSession->m_tbThrone.m_nOwner_id != ptbPlayer->m_nUid)
        {
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
            pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__DEL;
            return 0;
        }

        //if (!CCommonLogic::CheckFuncOpen(pstUser, pstCity, EN_FUNC_OPEN_TYPE_TAX))
        //{
        //    ptbTax->Set_Btime(ptbTax->m_nBtime + ptbTax->m_nCtime);
        //    ptbTax->Set_Etime(ptbTax->m_nBtime);
        //
        //    pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;
        //
        //    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        //    return 0;
        //}

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        CAwsRequest::SvrAlQuery(pstSession, pstSession->m_udwReqSvrId);
        bNeedResponse = TRUE;
        return 0;
    }
    
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        TbSvr_al atbTmpSvrAl[2048];
        TINT32 dwSvrAlNum = 0;
        dwSvrAlNum = CAwsResponse::OnQueryRsp(*pstSession->m_vecAwsRsp[0], atbTmpSvrAl, sizeof(TbSvr_al), 2048);
        if (dwSvrAlNum > 0)
        {
            set<TINT64> setUser;
            Json::Value jTmpTax = Json::Value(Json::objectValue);
            if (pstSession->m_tbThrone.m_nTax_id == 0)
            {
                jTmpTax["tax_rate"] = CGameInfo::GetInstance()->m_oJsonRoot["game_throne_tax"]["default"].asInt();
            }
            else
            {
                jTmpTax["tax_rate"] = pstSession->m_tbThrone.m_nTax_id;
            }
            jTmpTax["total_num"] = dwSvrAlNum;
            jTmpTax["cur_idx"] = 0;
            jTmpTax["resource"] = Json::Value(Json::arrayValue);
            for (TUINT32 dwIdx = 0; dwIdx < EN_RESOURCE_TYPE__END; dwIdx++)
            {
                jTmpTax["resource"].append(0);
            }
            jTmpTax["al_owner_list"] = Json::Value(Json::arrayValue);
            for (TINT32 dwIdx = 0; dwIdx < dwSvrAlNum; dwIdx++)
            {
                if (atbTmpSvrAl[dwIdx].m_nOwner_uid > 0 && setUser.count(atbTmpSvrAl[dwIdx].m_nOwner_uid) == 0)
                {
                    jTmpTax["al_owner_list"].append(atbTmpSvrAl[dwIdx].m_nOwner_uid);
                }
            }
            jTmpTax["tax_detail"] = Json::Value(Json::arrayValue);

            ptbTax->Set_Tax_info(jTmpTax);
            ptbTax->Set_Tuid(jTmpTax["al_owner_list"][jTmpTax["cur_idx"].asInt()].asInt());
            ptbTax->Set_Tpos(0);

            ptbTax->Set_Status(EN_TAX_STATUS__COLLECTING);
            ptbTax->Set_Etime(CTimeUtils::GetUnixTime() + 1 + rand() % 2);

            pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;
        }
        else
        {
            Json::Value jTmpTax = Json::Value(Json::objectValue);
            if (pstSession->m_tbThrone.m_nTax_id == 0)
            {
                jTmpTax["tax_rate"] = CGameInfo::GetInstance()->m_oJsonRoot["game_throne_tax"]["default"].asInt();
            }
            else
            {
                jTmpTax["tax_rate"] = pstSession->m_tbThrone.m_nTax_id;
            }
            jTmpTax["total_num"] = dwSvrAlNum;
            jTmpTax["cur_idx"] = 0;
            jTmpTax["resource"] = Json::Value(Json::arrayValue);
            for (TUINT32 udwIdx = 0; udwIdx < EN_RESOURCE_TYPE__END; udwIdx++)
            {
                jTmpTax["resource"].append(0);
            }
            jTmpTax["al_owner_list"] = Json::Value(Json::arrayValue);
            jTmpTax["tax_detail"] = Json::Value(Json::arrayValue);

            ptbTax->Set_Tax_info(jTmpTax);
            ptbTax->Set_Status(EN_TAX_STATUS__FINISH);
            ptbTax->Set_Etime(CTimeUtils::GetUnixTime() + 1 + rand() % 2);

            pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;
        }
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessAction::ProcessTaxCollecting(SSession *pstSession, TBOOL &bNeedResponse)
{
    TbMarch_action *ptbTax = &pstSession->m_stReqMarch;
    SUserInfo *pstSUser = &pstSession->m_stSourceUser;
    TbPlayer *ptbSPlayer = &pstSUser->m_tbPlayer;
    TbAlliance *ptbSAlliance = &pstSUser->m_tbAlliance;

    SUserInfo *pstTUser = &pstSession->m_stTargetUser;
    TbPlayer *ptbTPlayer = &pstTUser->m_tbPlayer;
    SCityInfo *pstTCity = &pstTUser->m_stCityInfo;

    if (pstSession->m_tbThrone.m_nAlid != ptbSAlliance->m_nAid
        || ptbSAlliance->m_nOid != ptbSPlayer->m_nUid)
    {
        ptbTax->Set_Tuid(0);
        ptbTax->Set_Status(EN_TAX_STATUS__FINISH); 
        ptbTax->Set_Etime(CTimeUtils::GetUnixTime() + 1 + rand() % 2);

        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;
        return 0;
    }

    if (ptbTPlayer->m_nAlpos != EN_ALLIANCE_POS__CHANCELLOR || ptbTPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET == 0
        || ptbTPlayer->m_nUid == ptbTax->m_nSuid)
    {
        ptbTax->m_jTax_info["cur_idx"] = ptbTax->m_jTax_info["cur_idx"].asInt() + 1;

        if (ptbTax->m_jTax_info["cur_idx"].asInt() >= ptbTax->m_jTax_info["total_num"].asInt())
        {
            ptbTax->Set_Tuid(0);
            ptbTax->Set_Status(EN_TAX_STATUS__FINISH);
        }
        else
        {
            ptbTax->Set_Tuid(ptbTax->m_jTax_info["al_owner_list"][ptbTax->m_jTax_info["cur_idx"].asInt()].asInt());
        }
        ptbTax->SetFlag(TbMARCH_ACTION_FIELD_TAX_INFO);
        ptbTax->Set_Etime(CTimeUtils::GetUnixTime() + 1 + rand() % 2);

        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;
        return 0;
    }

    TUINT64 uddwResNum = pstTCity->m_stTblData.m_bResource[0].m_addwNum[EN_RESOURCE_TYPE__GOLD];
    TUINT32 udwTaxRate = ptbTax->m_jTax_info["tax_rate"].asUInt();

    udwTaxRate = CGameInfo::GetInstance()->m_oJsonRoot["game_throne_tax"]["a"][CCommonFunc::NumToString(udwTaxRate)].asInt();

    TUINT32 udwTaxGoldNum = uddwResNum * (1.0 * udwTaxRate / 10000);

    if (pstTCity->m_stTblData.m_bResource.m_astList[0].m_addwNum[EN_RESOURCE_TYPE__GOLD] > udwTaxGoldNum)
    {
        pstTCity->m_stTblData.m_bResource.m_astList[0].m_addwNum[EN_RESOURCE_TYPE__GOLD] -= udwTaxGoldNum;
    }
    else
    {
        udwTaxGoldNum = pstTCity->m_stTblData.m_bResource.m_astList[0].m_addwNum[EN_RESOURCE_TYPE__GOLD];
        pstTCity->m_stTblData.m_bResource.m_astList[0].m_addwNum[EN_RESOURCE_TYPE__GOLD] = 0;
    }
    pstTCity->m_stTblData.SetFlag(TbCITY_FIELD_RESOURCE);

    TUINT32 audwResouce[EN_RESOURCE_TYPE__END] = { 0 };
    audwResouce[EN_RESOURCE_TYPE__GOLD] = udwTaxGoldNum;
    CProcessReport::GenPayTaxReport(pstSession, audwResouce, &pstSession->m_tbReport);

    SNoticInfo stNoticInfo;
    stNoticInfo.Reset();
    stNoticInfo.SetValue(EN_NOTI_ID__PAY_TAX,
        "", "",
        0, 0,
        0, 0,
        0, "", udwTaxGoldNum, CDocument::GetLang(pstTUser->m_tbLogin.m_nLang));
    CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), pstTUser->m_tbLogin.m_nSid, pstTUser->m_tbLogin.m_nUid, stNoticInfo);

    ptbTax->m_jTax_info["resource"][EN_RESOURCE_TYPE__GOLD] = ptbTax->m_jTax_info["resource"][EN_RESOURCE_TYPE__GOLD].asUInt() + udwTaxGoldNum;
    Json::Value jTmp = Json::Value(Json::arrayValue);
    jTmp[0] = ptbTPlayer->m_nUid;
    jTmp[1] = udwTaxGoldNum;
    ptbTax->m_jTax_info["tax_detail"].append(jTmp);

    ptbTax->m_jTax_info["cur_idx"] = ptbTax->m_jTax_info["cur_idx"].asInt() + 1;

    if (ptbTax->m_jTax_info["cur_idx"].asInt() >= ptbTax->m_jTax_info["total_num"].asInt())
    {
        ptbTax->Set_Tuid(0);
        ptbTax->Set_Status(EN_TAX_STATUS__FINISH);
    }
    else
    {
        ptbTax->Set_Tuid(ptbTax->m_jTax_info["al_owner_list"][ptbTax->m_jTax_info["cur_idx"].asInt()].asInt());
    }
    ptbTax->SetFlag(TbMARCH_ACTION_FIELD_TAX_INFO);
    ptbTax->Set_Etime(CTimeUtils::GetUnixTime() + 1 + rand() % 2);

    pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;

    return 0;
}

TINT32 CProcessAction::ProcessTaxFinish(SSession *pstSession, TBOOL &bNeedResponse)
{
    TbMarch_action *ptbTax = &pstSession->m_stReqMarch;
    SUserInfo *pstUser = &pstSession->m_stSourceUser;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;
    TbAlliance *ptbAlliance = &pstUser->m_tbAlliance;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;

    TINT64 ddwResource[EN_RESOURCE_TYPE__END] = { 0 };
    for (TUINT32 udwIdx = 0; udwIdx < EN_RESOURCE_TYPE__END; udwIdx++)
    {
        ddwResource[udwIdx] = ptbTax->m_jTax_info["resource"][udwIdx].asUInt();
    }
    CCityBase::AddResource(pstCity, ddwResource);

    CProcessReport::GenCollectTaxReport(pstSession, &pstSession->m_tbReport);
    SNoticInfo stNoticInfo;
    stNoticInfo.Reset();
    stNoticInfo.SetValue(EN_NOTI_ID__COLLECT_TAX,
        "", "",
        0, 0,
        0, 0,
        0, "", ddwResource[EN_RESOURCE_TYPE__GOLD], CDocument::GetLang(pstUser->m_tbLogin.m_nLang));
    CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), pstUser->m_tbLogin.m_nSid, pstUser->m_tbLogin.m_nUid, stNoticInfo);

    TINT64 ddwAlid = 0;
    if (ptbTax->m_jTax_info.isObject() && ptbTax->m_jTax_info.isMember("self_alid"))
    {
        ddwAlid = ptbTax->m_jTax_info["self_alid"].asInt64();
    }
    if (ddwAlid != 0 && pstSession->m_tbThrone.m_nAlid == ddwAlid
        && pstSession->m_tbThrone.m_nOwner_id != ptbPlayer->m_nUid)
    {
        ptbTax->Set_Tuid(pstSession->m_tbThrone.m_nOwner_id);
        ptbTax->Set_Status(EN_TAX_STATUS__TRANSFER);
        ptbTax->Set_Etime(CTimeUtils::GetUnixTime());
        ptbTax->Set_Btime(ptbTax->m_nBtime + ptbTax->m_nCtime);
        TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
        if (ptbTax->m_nBtime < udwCurTime)
        {
            ptbTax->Set_Btime(udwCurTime + ptbTax->m_nCtime);
        }

        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;
        return 0;
    }
    else if (pstSession->m_tbThrone.m_nAlid != ptbAlliance->m_nAid
        || ptbAlliance->m_nOid != ptbPlayer->m_nUid)
    {
        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__DEL;
        return 0;
    }
    else
    {
        ptbTax->DeleteField(TbMARCH_ACTION_FIELD_TAX_INFO);
        ptbTax->Set_Tpos(pstSession->m_tbThrone.m_nPos);

        ptbTax->Set_Status(EN_TAX_STATUS__PREPARING);
        if (ptbTax->m_nCtime == 0)
        {
            ptbTax->Set_Ctime(CCommonBase::GetGameBasicVal(EN_GAME_BASIC_TAX_INTERVAL));
        }
        ptbTax->Set_Btime(ptbTax->m_nBtime + ptbTax->m_nCtime);
        ptbTax->Set_Etime(ptbTax->m_nBtime);

        TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
        if (ptbTax->m_nEtime < udwCurTime)
        {
            ptbTax->Set_Btime(udwCurTime + ptbTax->m_nCtime);
            ptbTax->Set_Etime(ptbTax->m_nBtime);
        }

        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;
    }

    return 0;
}

TINT32 CProcessAction::ProcessTaxTransfer(SSession *pstSession, TBOOL &bNeedResponse)
{
    TbMarch_action *ptbTax = &pstSession->m_stReqMarch;
    SUserInfo *pstUser = &pstSession->m_stSourceUser;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;
    TbAlliance *ptbAlliance = &pstUser->m_tbAlliance;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;

    SUserInfo *pstTUser = &pstSession->m_stTargetUser;
    TbPlayer *ptbTPlayer = &pstTUser->m_tbPlayer;
    SCityInfo *pstTCity = &pstTUser->m_stCityInfo;

    TINT64 ddwAlid = 0;
    if (ptbTax->m_jTax_info.isObject() && ptbTax->m_jTax_info.isMember("self_alid"))
    {
        ddwAlid = ptbTax->m_jTax_info["self_alid"].asInt64();
    }
    if (ddwAlid != 0 && pstSession->m_tbThrone.m_nAlid == ddwAlid)
    {
        // 国王还是自己
        if (pstSession->m_tbThrone.m_nOwner_id == ptbPlayer->m_nUid)
        {
            ptbTax->DeleteField(TbMARCH_ACTION_FIELD_TAX_INFO);
            ptbTax->Set_Tpos(pstSession->m_tbThrone.m_nPos);

            ptbTax->Set_Status(EN_TAX_STATUS__PREPARING);
            
            ptbTax->Set_Etime(ptbTax->m_nBtime);

            pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;
            return 0;
        }
        // 国王和target_user不同...
        else if (pstSession->m_tbThrone.m_nOwner_id != ptbTPlayer->m_nUid)
        {
            ptbTax->Set_Tuid(pstSession->m_tbThrone.m_nOwner_id);
            ptbTax->Set_Etime(CTimeUtils::GetUnixTime());

            pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;
            return 0;
        }
        // 国王和target_user相同...
        else if (pstSession->m_tbThrone.m_nOwner_id == ptbTPlayer->m_nUid)
        {
            TbMarch_action *ptbNewTax = CActionBase::GenTaxAction(pstTUser, pstSession->m_tbThrone.m_nPos);
            if (ptbNewTax == NULL)
            {
                ptbTax->Set_Etime(CTimeUtils::GetUnixTime());
                pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;
                return 0;
            }
            else
            {
                ptbNewTax->Set_Btime(ptbTax->m_nBtime);
                ptbNewTax->Set_Etime(ptbTax->m_nBtime);
            }
        }
    }

    pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__DEL;
    return 0;
}

TINT32 CProcessAction::ProcessActionTrainNew(SSession *pstSession, TBOOL &bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stSourceUser;
    TbPlayer *pstPlayer = &pstSession->m_stSourceUser.m_tbPlayer;
    SCityInfo *pstCity = &pstSession->m_stSourceUser.m_stCityInfo;
    SActionTrainParam *pstParam = &pstSession->m_stReqAlAction.m_bParam[0].m_stTrain;
    TUINT64 uddwGainedMight = 0;

    if (pstCity == NULL)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessActionTrainNew:no city info[seq=%u]", pstSession->m_udwSeqNo));
        return -2;
    }

    SCommonTroop *pstTroop = &pstCity->m_stTblData.m_bTroop[0];
    SCommonFort *pstFort = &pstCity->m_stTblData.m_bFort[0];

    SNoticInfo stNoticInfo;
    // 1. set data
    switch (pstSession->m_stReqAlAction.m_nSclass)
    {
    case EN_ACTION_SEC_CLASS__TROOP:
        // tips
        CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPE__TRAIN_TROOP_OK, pstPlayer->m_nUid, TRUE, pstParam->m_ddwType, pstParam->m_ddwNum);
        stNoticInfo.Reset();
        stNoticInfo.m_dwNoticId = EN_NOTI_ID__TRAIN_SUCC;
        stNoticInfo.m_strKeyName = "doc_troop";
        stNoticInfo.m_strValueName = CCommonFunc::NumToString(pstParam->m_ddwType);
        stNoticInfo.m_strLang = CDocument::GetLang(pstUser->m_tbLogin.m_nLang);
        CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), pstSession->m_udwReqSvrId, pstPlayer->m_nUid, stNoticInfo);

        //活动积分统计
        CActivitesLogic::ComputeTrainTroopScore(pstUser, (TUINT32)pstParam->m_ddwType, pstParam->m_ddwNum);
        CQuestLogic::SetTaskCurValue(pstUser, pstCity, EN_TASK_TYPE_ING_TROOP_TRAIN_NUM, pstParam->m_ddwNum, 0, CToolBase::GetTroopLvByTroopId(pstParam->m_ddwType));

        break;
    case EN_ACTION_SEC_CLASS__FORT:
        // tips
        CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPE__TRAIN_FORT_OK, pstPlayer->m_nUid, TRUE, pstParam->m_ddwType, pstParam->m_ddwNum);
        stNoticInfo.Reset();
        stNoticInfo.m_dwNoticId = EN_NOTI_ID__FORT_TRAIN_SUCC;
        stNoticInfo.m_strKeyName = "doc_fort";
        stNoticInfo.m_strValueName = CCommonFunc::NumToString(pstParam->m_ddwType);
        stNoticInfo.m_strLang = CDocument::GetLang(pstUser->m_tbLogin.m_nLang);
        CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), pstSession->m_udwReqSvrId, pstPlayer->m_nUid, stNoticInfo);
        CQuestLogic::SetTaskCurValue(pstUser, pstCity, EN_TASK_TYPE_ING_FORT_TRAIN_NUM, pstParam->m_ddwNum, 0, CToolBase::GetFortLvByFortId(pstParam->m_ddwType));

        //活动积分统计
        CActivitesLogic::ComputeTrainFortScore(pstUser, (TUINT32)pstParam->m_ddwType, pstParam->m_ddwNum);

        break;
    case EN_ACTION_SEC_CLASS__HOS_TREAT:
        // tips
        CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPE__HEAL_TROOP_OK, pstPlayer->m_nUid, TRUE, pstParam->m_ddwType, pstParam->m_ddwNum);
        stNoticInfo.Reset();
        stNoticInfo.m_dwNoticId = EN_NOTI_ID__HEAL_SUCC;
        stNoticInfo.m_strKeyName = "doc_troop";
        stNoticInfo.m_strValueName = CCommonFunc::NumToString(pstParam->m_ddwType);
        stNoticInfo.m_strLang = CDocument::GetLang(pstUser->m_tbLogin.m_nLang);
        CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), pstSession->m_udwReqSvrId, pstPlayer->m_nUid, stNoticInfo);

        //活动积分统计
        CActivitesLogic::ComputeHealTroopScore(pstUser, (TUINT32)pstParam->m_ddwType, pstParam->m_ddwNum);

        break;
    case EN_ACTION_SEC_CLASS__FORT_REPAIR:
        // tips
        CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPE__HEAL_FORT_OK, pstPlayer->m_nUid, TRUE, pstParam->m_ddwType, pstParam->m_ddwNum);
        stNoticInfo.Reset();
        stNoticInfo.m_dwNoticId = EN_NOTI_ID__FORT_REPAIR_SUCC;
        stNoticInfo.m_strKeyName = "doc_fort";
        stNoticInfo.m_strValueName = CCommonFunc::NumToString(pstParam->m_ddwType);
        stNoticInfo.m_strLang = CDocument::GetLang(pstUser->m_tbLogin.m_nLang);
        CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), pstSession->m_udwReqSvrId, pstPlayer->m_nUid, stNoticInfo);

        //活动积分统计
        CActivitesLogic::ComputeHealFortScore(pstUser, (TUINT32)pstParam->m_ddwType, pstParam->m_ddwNum);
        break;
    default:
        return -1;
        assert(0);
        break;
    }

    // 2. compute
    SActionTrainParam& stTrainParam = pstSession->m_stReqAlAction.m_bParam[0].m_stTrain;
    switch (pstSession->m_stReqAlAction.m_nSclass)
    {
    case EN_ACTION_SEC_CLASS__TROOP:
        pstTroop->m_addwNum[stTrainParam.m_ddwType] += stTrainParam.m_ddwNum;
        pstCity->m_stTblData.m_mFlag[TbCITY_FIELD_TROOP] = UPDATE_ACTION_TYPE_PUT;
        // add might
        uddwGainedMight = pstParam->m_ddwNum * CToolBase::GetTroopSingleMight(stTrainParam.m_ddwType);
        pstPlayer->Set_Might(pstPlayer->m_nMight + uddwGainedMight);
        pstPlayer->Set_Mgain(pstPlayer->m_nMgain + uddwGainedMight);
        break;
    case EN_ACTION_SEC_CLASS__FORT:
        pstFort->m_addwNum[stTrainParam.m_ddwType] += stTrainParam.m_ddwNum;
        pstCity->m_stTblData.m_mFlag[TbCITY_FIELD_FORT] = UPDATE_ACTION_TYPE_PUT;
        // add might
        uddwGainedMight = pstParam->m_ddwNum * CToolBase::GetFortSingleMight(stTrainParam.m_ddwType);
        pstPlayer->Set_Might(pstPlayer->m_nMight + uddwGainedMight);
        pstPlayer->Set_Mgain(pstPlayer->m_nMgain + uddwGainedMight);
        break;
    case EN_ACTION_SEC_CLASS__HOS_TREAT:
        pstTroop->m_addwNum[stTrainParam.m_ddwType] += stTrainParam.m_ddwNum;
        pstCity->m_stTblData.m_mFlag[TbCITY_FIELD_TROOP] = UPDATE_ACTION_TYPE_PUT;
        // add might
        uddwGainedMight = pstParam->m_ddwNum * CToolBase::GetTroopSingleMight(stTrainParam.m_ddwType);
        pstPlayer->Set_Might(pstPlayer->m_nMight + uddwGainedMight);
        break;
    case EN_ACTION_SEC_CLASS__FORT_REPAIR:
        pstFort->m_addwNum[stTrainParam.m_ddwType] += stTrainParam.m_ddwNum;
        pstCity->m_stTblData.m_mFlag[TbCITY_FIELD_FORT] = UPDATE_ACTION_TYPE_PUT;
        // add might
        uddwGainedMight = pstParam->m_ddwNum * CToolBase::GetFortSingleMight(stTrainParam.m_ddwType);
        pstPlayer->Set_Might(pstPlayer->m_nMight + uddwGainedMight);
        break;
    }
    // 用户exp增加
    CPlayerBase::AddLordExp(pstUser, pstCity, stTrainParam.m_ddwExp);

    pstSession->m_dwMarchResult = EN_REPORT_RESULT_WIN;
    pstSession->m_ucReqAlActionFlag = EN_TABLE_UPDT_FLAG__DEL;

    return 0;
}

TINT32 CProcessAction::ProcessActionDragon(SSession *pstSession, TBOOL &bNeedResponse)
{
    TbPlayer *ptbPlayer = &pstSession->m_stSourceUser.m_tbPlayer;

    if (ptbPlayer->m_nHas_dragon > 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessActionDragon: has own dragon[%ld], just del... [seq=%u]",
            ptbPlayer->m_nHas_dragon, pstSession->m_udwSeqNo));
    }
    else
    {
        TINT64 ddwMaxEnergy = CPlayerBase::GetCurDragonMaxEnergy(&pstSession->m_stSourceUser);
        CPlayerBase::AddDragon(ptbPlayer, ddwMaxEnergy);
        CSendMessageBase::AddTips(&pstSession->m_stSourceUser, EN_TIPS_TYPE__UNLOCK_DRAGON, ptbPlayer->m_nUid, TRUE);
        // 发送邮件
        CMsgBase::SendOperateMail(ptbPlayer->m_nUid, EN_MAIL_ID__DRAGON_UNLOCK, pstSession->m_stRawReqAction.m_nSid,
            SYSTEM_ENCOURAGE, "", "", "");

        SNoticInfo stNoticInfo;
        stNoticInfo.Reset();
        stNoticInfo.SetValue(EN_NOTI_ID__HERO_UNLOCKED,
            "", "",
            0, 0,
            0, 0,
            0, "", 0);
        CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), pstSession->m_stSourceUser.m_tbPlayer.m_nSid, pstSession->m_stSourceUser.m_tbPlayer.m_nUid, stNoticInfo);
    }

    pstSession->m_ucReqAlActionFlag = EN_TABLE_UPDT_FLAG__DEL;
    return 0;
}

TINT32 CProcessAction::ProcessIdolThronePeaceTimePeriod(SSession *pstSession, TBOOL &bNeedResponse)
{
    CGameInfo *poGameInfo = CGameInfo::GetInstance();

    TbMarch_action *ptbReqAction = &pstSession->m_stReqMarch;
    TbMap *ptbMapItem = &pstSession->m_stMapItem;

    TbIdol *ptbIdol = NULL;
    TUINT32 udwIdolIdx = 0;
    for (udwIdolIdx = 0; udwIdolIdx < pstSession->m_udwIdolNum; udwIdolIdx++)
    {
        if (pstSession->m_atbIdol[udwIdolIdx].m_nPos == ptbReqAction->m_nTpos)
        {
            ptbIdol = &pstSession->m_atbIdol[udwIdolIdx];
            break;
        }
    }

    if (ptbIdol == NULL)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessIdolThronePeaceTimePeriod: there is no idol in pos[%ld], just del action... [seq=%u]",
            ptbReqAction->m_nTpos, pstSession->m_udwSeqNo));

        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__DEL;
        return 0;
    }

    if (ptbMapItem->m_nType != EN_WILD_TYPE__IDOL)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessIdolThronePeaceTimePeriod: pos[%ld] type[%ld], just del idol... [seq=%u]",
            ptbMapItem->m_nId, ptbMapItem->m_nType, pstSession->m_udwSeqNo));

        pstSession->m_ucIdolFlag[udwIdolIdx] = EN_TABLE_UPDT_FLAG__DEL;
        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__DEL;
        return 0;
    }

    if (!poGameInfo->m_oJsonRoot["game_guardian"].isMember(CCommonFunc::NumToString(ptbIdol->m_nId)))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessIdolThronePeaceTimePeriod: there is no such idol[id=%ld], just del idol... [seq=%u]",
            ptbIdol->m_nId, pstSession->m_udwSeqNo));

        pstSession->m_ucIdolFlag[udwIdolIdx] = EN_TABLE_UPDT_FLAG__DEL;
        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__DEL;
        return 0;
    }

    pstSession->m_ucIdolFlag[udwIdolIdx] = EN_TABLE_UPDT_FLAG__CHANGE;

    if (pstSession->m_tbThrone.m_nPos != 0 && pstSession->m_tbThrone.m_nStatus != EN_THRONE_STATUS__CONTEST_PERIOD)
    {
        ptbIdol->Set_Alid(-1);
        ptbIdol->m_jRank.clear();
        ptbIdol->m_jRank = Json::Value(Json::arrayValue);
        ptbIdol->SetFlag(TbIDOL_FIELD_RANK);
        ptbIdol->m_bTroop.Reset();
        ptbIdol->SetFlag(TbIDOL_FIELD_TROOP);
        ptbIdol->m_jInfo.clear();
        ptbIdol->SetFlag(TbIDOL_FIELD_INFO);
        ptbIdol->Set_Status(EN_IDOL_STATUS__THRONE_PEACE_TIME);
        ptbIdol->Set_End_time(0);
        ptbIdol->Set_Last_time(0);
        ptbIdol->Set_Active(0);

        ptbMapItem->Set_Status(ptbIdol->m_nStatus);
        ptbMapItem->Set_Time_end(ptbIdol->m_nEnd_time);
        ptbMapItem->Set_Alid(0);
        ptbMapItem->Set_Alname("");
        ptbMapItem->Set_Al_nick("");
        pstSession->m_ucMapItemFlag = EN_TABLE_UPDT_FLAG__CHANGE;

        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__DEL;
        return 0;
    }

    ptbIdol->Set_Status(EN_IDOL_STATUS__QUIET_PERIOD);

    ptbIdol->m_jInfo.clear();
    ptbIdol->m_jInfo = poGameInfo->m_oJsonRoot["game_guardian"][CCommonFunc::NumToString(ptbIdol->m_nId)];
    ptbIdol->SetFlag(TbIDOL_FIELD_INFO);

    ptbIdol->Set_Last_time(ptbIdol->m_jInfo["a"][1U].asInt64());
    ptbIdol->Set_End_time(ptbReqAction->m_nEtime + ptbIdol->m_nLast_time);

    ptbIdol->m_bTroop.Reset();
    ptbIdol->SetFlag(TbIDOL_FIELD_TROOP);
    TUINT32 udwTotalNum = ptbIdol->m_jInfo["t"][3U].asUInt();
    TUINT32 udwTroopSize = ptbIdol->m_jInfo["t"][4U].size();
    set<TUINT32> setTroopIdx;
    setTroopIdx.clear();
    TUINT32 udwTroopId = 0;
    TUINT32 udwTroopNum = 0;
    for (TUINT32 udwIdx = 0; udwIdx < udwTroopSize; udwIdx++)
    {
        if (udwTotalNum == 0)
        {
            break;
        }
        udwTroopId = rand() % udwTroopSize;
        while (setTroopIdx.count(udwTroopId))
        {
            udwTroopId = (udwTroopId + 1) % udwTroopSize;
        }
        setTroopIdx.insert(udwTroopId);
        udwTroopId = ptbIdol->m_jInfo["t"][4U][udwTroopId].asUInt();
        if (udwIdx == udwTroopSize - 1)
        {
            udwTroopNum = udwTotalNum;
            udwTotalNum = 0;
        }
        else
        {
            udwTroopNum = CToolBase::GetRandNumber(ptbIdol->m_jInfo["t"][5U][udwIdx][0U].asUInt(), ptbIdol->m_jInfo["t"][5U][udwIdx][1U].asUInt());
            udwTroopNum = 1.0 * udwTroopNum / 10000 * udwTotalNum;
            if (udwTotalNum > udwTroopNum)
            {
                udwTotalNum -= udwTroopNum;
            }
            else
            {
                udwTroopNum = udwTotalNum;
                udwTotalNum = 0;
            }
        }
        
        ptbIdol->m_bTroop[0].m_addwNum[udwTroopId] = udwTroopNum;
    }

    ptbReqAction->Set_Status(EN_TITMER_IDOL_PERIOD_STATUS__QUIET);
    ptbReqAction->Set_Btime(ptbReqAction->m_nEtime);
    ptbReqAction->Set_Ctime(ptbIdol->m_nLast_time);
    ptbReqAction->Set_Etime(ptbIdol->m_nEnd_time);

    ptbMapItem->Set_Status(ptbIdol->m_nStatus);
    ptbMapItem->Set_Time_end(ptbIdol->m_nEnd_time);
    pstSession->m_ucMapItemFlag = EN_TABLE_UPDT_FLAG__CHANGE;

    pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;
    return 0;
}

TINT32 CProcessAction::ProcessIdolBuffPeriod(SSession *pstSession, TBOOL &bNeedResponse)
{
    CGameInfo *poGameInfo = CGameInfo::GetInstance();

    TbMarch_action *ptbReqAction = &pstSession->m_stReqMarch;
    TbMap *ptbMapItem = &pstSession->m_stMapItem;

    TbIdol *ptbIdol = NULL;
    TUINT32 udwIdolIdx = 0;
    for (udwIdolIdx = 0; udwIdolIdx < pstSession->m_udwIdolNum; udwIdolIdx++)
    {
        if (pstSession->m_atbIdol[udwIdolIdx].m_nPos == ptbReqAction->m_nTpos)
        {
            ptbIdol = &pstSession->m_atbIdol[udwIdolIdx];
            break;
        }
    }

    if (ptbIdol == NULL)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessIdolThronePeaceTimePeriod: there is no idol in pos[%ld], just del action... [seq=%u]",
            ptbReqAction->m_nTpos, pstSession->m_udwSeqNo));

        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__DEL;
        return 0;
    }

    if (ptbMapItem->m_nType != EN_WILD_TYPE__IDOL)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessIdolThronePeaceTimePeriod: pos[%ld] type[%ld], just del idol... [seq=%u]",
            ptbMapItem->m_nId, ptbMapItem->m_nType, pstSession->m_udwSeqNo));

        pstSession->m_ucIdolFlag[udwIdolIdx] = EN_TABLE_UPDT_FLAG__DEL;
        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__DEL;
        return 0;
    }

    if (!poGameInfo->m_oJsonRoot["game_guardian"].isMember(CCommonFunc::NumToString(ptbIdol->m_nId)))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessIdolThronePeaceTimePeriod: there is no such idol[id=%ld], just del idol... [seq=%u]",
            ptbIdol->m_nId, pstSession->m_udwSeqNo));

        pstSession->m_ucIdolFlag[udwIdolIdx] = EN_TABLE_UPDT_FLAG__DEL;
        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__DEL;
        return 0;
    }

    pstSession->m_ucIdolFlag[udwIdolIdx] = EN_TABLE_UPDT_FLAG__CHANGE;

    if (pstSession->m_tbThrone.m_nPos != 0 && pstSession->m_tbThrone.m_nStatus != EN_THRONE_STATUS__CONTEST_PERIOD)
    {
        if (ptbIdol->m_nAlid > 0)
        {
            CAuPushData::AddPushData_Aid(pstSession, ptbIdol->m_nAlid);
        }

        ptbIdol->Set_Alid(-1);
        ptbIdol->m_jRank.clear();
        ptbIdol->m_jRank = Json::Value(Json::arrayValue);
        ptbIdol->SetFlag(TbIDOL_FIELD_RANK);
        ptbIdol->m_bTroop.Reset();
        ptbIdol->SetFlag(TbIDOL_FIELD_TROOP);
        ptbIdol->m_jInfo.clear();
        ptbIdol->SetFlag(TbIDOL_FIELD_INFO);
        ptbIdol->Set_Status(EN_IDOL_STATUS__THRONE_PEACE_TIME);
        ptbIdol->Set_End_time(0);
        ptbIdol->Set_Last_time(0);
        ptbIdol->Set_Active(0);

        ptbMapItem->Set_Status(ptbIdol->m_nStatus);
        ptbMapItem->Set_Time_end(ptbIdol->m_nEnd_time);
        ptbMapItem->Set_Alid(0);
        ptbMapItem->Set_Alname("");
        ptbMapItem->Set_Al_nick("");
        pstSession->m_ucMapItemFlag = EN_TABLE_UPDT_FLAG__CHANGE;

        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__DEL;
        return 0;
    }

    ptbIdol->Set_Status(EN_IDOL_STATUS__QUIET_PERIOD);

    ptbIdol->Set_Alid(-1);
    ptbIdol->m_jInfo.clear();
    ptbIdol->m_jInfo = poGameInfo->m_oJsonRoot["game_guardian"][CCommonFunc::NumToString(ptbIdol->m_nId)];
    ptbIdol->SetFlag(TbIDOL_FIELD_INFO);

    ptbIdol->m_jRank.clear();
    ptbIdol->m_jRank = Json::Value(Json::arrayValue);
    ptbIdol->SetFlag(TbIDOL_FIELD_RANK);

    ptbIdol->Set_Last_time(ptbIdol->m_jInfo["a"][4U].asInt64());
    ptbReqAction->Set_Btime(ptbIdol->m_nEnd_time);
    ptbIdol->Set_End_time(ptbIdol->m_nEnd_time + ptbIdol->m_nLast_time);

    ptbIdol->m_bTroop.Reset();
    ptbIdol->SetFlag(TbIDOL_FIELD_TROOP);
    TUINT32 udwTotalNum = ptbIdol->m_jInfo["t"][3U].asUInt();
    TUINT32 udwTroopSize = ptbIdol->m_jInfo["t"][4U].size();
    set<TUINT32> setTroopIdx;
    setTroopIdx.clear();
    TUINT32 udwTroopId = 0;
    TUINT32 udwTroopNum = 0;
    for (TUINT32 udwIdx = 0; udwIdx < udwTroopSize; udwIdx++)
    {
        if (udwTotalNum == 0)
        {
            break;
        }
        udwTroopId = rand() % udwTroopSize;
        while (setTroopIdx.count(udwTroopId))
        {
            udwTroopId = (udwTroopId + 1) % udwTroopSize;
        }
        setTroopIdx.insert(udwTroopId);
        udwTroopId = ptbIdol->m_jInfo["t"][4U][udwTroopId].asUInt();
        if (udwIdx == udwTroopSize - 1)
        {
            udwTroopNum = udwTotalNum;
            udwTotalNum = 0;
        }
        else
        {
            udwTroopNum = CToolBase::GetRandNumber(ptbIdol->m_jInfo["t"][5U][udwIdx][0U].asUInt(), ptbIdol->m_jInfo["t"][5U][udwIdx][1U].asUInt());
            if (udwTotalNum > udwTroopNum)
            {
                udwTotalNum -= udwTroopNum;
            }
            else
            {
                udwTroopNum = udwTotalNum;
                udwTotalNum = 0;
            }
        }

        ptbIdol->m_bTroop[0].m_addwNum[udwTroopId] = udwTroopNum;
    }

    ptbReqAction->Set_Status(EN_TITMER_IDOL_PERIOD_STATUS__QUIET);
    ptbReqAction->Set_Ctime(ptbIdol->m_nLast_time);
    ptbReqAction->Set_Etime(ptbIdol->m_nEnd_time);

    ptbMapItem->Set_Status(ptbIdol->m_nStatus);
    ptbMapItem->Set_Time_end(ptbIdol->m_nEnd_time);
    ptbMapItem->Set_Alid(0);
    ptbMapItem->Set_Alname("");
    ptbMapItem->Set_Al_nick("");
    pstSession->m_ucMapItemFlag = EN_TABLE_UPDT_FLAG__CHANGE;

    pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;
    return 0;
}

TINT32 CProcessAction::ProcessIdolQuietPeriod(SSession *pstSession, TBOOL &bNeedResponse)
{
    TbMarch_action *ptbReqAction = &pstSession->m_stReqMarch;
    TbMap *ptbMapItem = &pstSession->m_stMapItem;

    TbIdol *ptbIdol = NULL;
    TUINT32 udwIdolIdx = 0;
    TBOOL bFirstActive = TRUE;
    for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_udwIdolNum; udwIdx++)
    {
        if (pstSession->m_atbIdol[udwIdx].m_nPos == ptbReqAction->m_nTpos)
        {
            ptbIdol = &pstSession->m_atbIdol[udwIdx];
            udwIdolIdx = udwIdx;
        }
        if (pstSession->m_atbIdol[udwIdx].m_nActive == 1)
        {
            bFirstActive = FALSE;
        }
    }

    if (ptbIdol == NULL)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessIdolThronePeaceTimePeriod: there is no idol in pos[%ld], just del action... [seq=%u]",
            ptbReqAction->m_nTpos, pstSession->m_udwSeqNo));

        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__DEL;
        return 0;
    }

    if (ptbMapItem->m_nType != EN_WILD_TYPE__IDOL)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessIdolThronePeaceTimePeriod: pos[%ld] type[%ld], just del idol... [seq=%u]",
            ptbMapItem->m_nId, ptbMapItem->m_nType, pstSession->m_udwSeqNo));

        pstSession->m_ucIdolFlag[udwIdolIdx] = EN_TABLE_UPDT_FLAG__DEL;
        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__DEL;
        return 0;
    }

    pstSession->m_ucIdolFlag[udwIdolIdx] = EN_TABLE_UPDT_FLAG__CHANGE;

    if (pstSession->m_tbThrone.m_nPos != 0 && pstSession->m_tbThrone.m_nStatus != EN_THRONE_STATUS__CONTEST_PERIOD)
    {
        ptbIdol->Set_Alid(-1);
        ptbIdol->m_jRank.clear();
        ptbIdol->m_jRank = Json::Value(Json::arrayValue);
        ptbIdol->SetFlag(TbIDOL_FIELD_RANK);
        ptbIdol->m_bTroop.Reset();
        ptbIdol->SetFlag(TbIDOL_FIELD_TROOP);
        ptbIdol->m_jInfo.clear();
        ptbIdol->SetFlag(TbIDOL_FIELD_INFO);
        ptbIdol->Set_Status(EN_IDOL_STATUS__THRONE_PEACE_TIME);
        ptbIdol->Set_End_time(0);
        ptbIdol->Set_Last_time(0);
        ptbIdol->Set_Active(0);

        ptbMapItem->Set_Status(ptbIdol->m_nStatus);
        ptbMapItem->Set_Time_end(ptbIdol->m_nEnd_time);
        ptbMapItem->Set_Alid(0);
        ptbMapItem->Set_Alname("");
        ptbMapItem->Set_Al_nick("");
        pstSession->m_ucMapItemFlag = EN_TABLE_UPDT_FLAG__CHANGE;

        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__DEL;
        return 0;
    }

    if (ptbIdol->m_nActive == 0)
    {
        ptbIdol->Set_Active(1);
    }
    ptbIdol->Set_Status(EN_IDOL_STATUS__CONTEST_PERIOD);
    ptbIdol->Set_Last_time(ptbIdol->m_jInfo["a"][2U].asInt64());

    ptbReqAction->Set_Btime(ptbIdol->m_nEnd_time);
    ptbIdol->Set_End_time(ptbIdol->m_nEnd_time + ptbIdol->m_nLast_time);

    ptbReqAction->Set_Status(EN_TITMER_IDOL_PERIOD_STATUS__CONTEST);
    ptbReqAction->Set_Ctime(ptbIdol->m_nLast_time);
    ptbReqAction->Set_Etime(ptbIdol->m_nEnd_time);

    ptbMapItem->Set_Status(ptbIdol->m_nStatus);
    ptbMapItem->Set_Time_end(ptbIdol->m_nEnd_time);
    pstSession->m_ucMapItemFlag = EN_TABLE_UPDT_FLAG__CHANGE;

    pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;
    return 0;
}

TINT32 CProcessAction::ProcessIdolContestPeriod(SSession *pstSession, TBOOL &bNeedResponse)
{
    TbMarch_action *ptbReqAction = &pstSession->m_stReqMarch;
    TbMap *ptbMapItem = &pstSession->m_stMapItem;

    TbIdol *ptbIdol = NULL;
    TUINT32 udwIdolIdx = 0;
    for (udwIdolIdx = 0; udwIdolIdx < pstSession->m_udwIdolNum; udwIdolIdx++)
    {
        if (pstSession->m_atbIdol[udwIdolIdx].m_nPos == ptbReqAction->m_nTpos)
        {
            ptbIdol = &pstSession->m_atbIdol[udwIdolIdx];
            break;
        }
    }

    if (ptbIdol == NULL)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessIdolThronePeaceTimePeriod: there is no idol in pos[%ld], just del action... [seq=%u]",
            ptbReqAction->m_nTpos, pstSession->m_udwSeqNo));

        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__DEL;
        return 0;
    }

    if (ptbMapItem->m_nType != EN_WILD_TYPE__IDOL)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessIdolThronePeaceTimePeriod: pos[%ld] type[%ld], just del idol... [seq=%u]",
            ptbMapItem->m_nId, ptbMapItem->m_nType, pstSession->m_udwSeqNo));

        pstSession->m_ucIdolFlag[udwIdolIdx] = EN_TABLE_UPDT_FLAG__DEL;
        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__DEL;
        return 0;
    }

    pstSession->m_ucIdolFlag[udwIdolIdx] = EN_TABLE_UPDT_FLAG__CHANGE;

    if (pstSession->m_tbThrone.m_nPos != 0 && pstSession->m_tbThrone.m_nStatus != EN_THRONE_STATUS__CONTEST_PERIOD)
    {
        ptbIdol->Set_Alid(-1);
        ptbIdol->m_jRank.clear(); 
        ptbIdol->m_jRank = Json::Value(Json::arrayValue);
        ptbIdol->SetFlag(TbIDOL_FIELD_RANK);
        ptbIdol->m_bTroop.Reset();
        ptbIdol->SetFlag(TbIDOL_FIELD_TROOP);
        ptbIdol->m_jInfo.clear();
        ptbIdol->SetFlag(TbIDOL_FIELD_INFO);
        ptbIdol->Set_Status(EN_IDOL_STATUS__THRONE_PEACE_TIME);
        ptbIdol->Set_End_time(0);
        ptbIdol->Set_Last_time(0);
        ptbIdol->Set_Active(0);

        ptbMapItem->Set_Status(ptbIdol->m_nStatus);
        ptbMapItem->Set_Time_end(ptbIdol->m_nEnd_time);
        ptbMapItem->Set_Alid(0);
        ptbMapItem->Set_Alname("");
        ptbMapItem->Set_Al_nick("");
        pstSession->m_ucMapItemFlag = EN_TABLE_UPDT_FLAG__CHANGE;

        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__DEL;
        return 0;
    }

    //rank
    Json::Value jTmp = Json::Value(Json::arrayValue);
    vector<SIdolRank> vecRank;
    vecRank.clear();
    SIdolRank stTmpRank;
    for (TUINT32 udwIdx = 0; udwIdx < ptbIdol->m_jRank.size(); udwIdx++)
    {
        stTmpRank.Reset();
        stTmpRank.udwIdx = udwIdx;
        stTmpRank.udwPoint = ptbIdol->m_jRank[udwIdx]["point"].asUInt();
        stTmpRank.ddwTime = ptbIdol->m_jRank[udwIdx]["time"].asInt64();
        vecRank.push_back(stTmpRank);
    }

    std::sort(vecRank.begin(), vecRank.end(), SIdolRank::Comp);
    for (TUINT32 udwIdx = 0; udwIdx < vecRank.size(); udwIdx++)
    {
        jTmp[udwIdx] = ptbIdol->m_jRank[vecRank[udwIdx].udwIdx];
        jTmp[udwIdx]["rank"] = udwIdx + 1;
    }

    ptbIdol->m_jRank.clear();
    ptbIdol->m_jRank = jTmp;
    ptbIdol->SetFlag(TbIDOL_FIELD_RANK);

    if (ptbIdol->m_jRank.size() > 0)
    {
        ptbIdol->Set_Alid(ptbIdol->m_jRank[0U]["alid"].asInt64());
    }
    else
    {
        ptbIdol->Set_Alid(-1);
    }

    ptbIdol->Set_Status(EN_IDOL_STATUS__BUFF_PERIOD);
    ptbIdol->Set_Last_time(ptbIdol->m_jInfo["a"][3U].asInt64());

    ptbReqAction->Set_Btime(ptbIdol->m_nEnd_time);
    ptbIdol->Set_End_time(ptbIdol->m_nEnd_time + ptbIdol->m_nLast_time);

    ptbReqAction->Set_Status(EN_TITMER_IDOL_PERIOD_STATUS__BUFF);
    ptbReqAction->Set_Ctime(ptbIdol->m_nLast_time);
    ptbReqAction->Set_Etime(ptbIdol->m_nEnd_time);

    ptbMapItem->Set_Status(ptbIdol->m_nStatus);
    ptbMapItem->Set_Time_end(ptbIdol->m_nEnd_time);

    if (ptbIdol->m_nAlid > 0)
    {
        ptbMapItem->Set_Alid(ptbIdol->m_nAlid);
        ptbMapItem->Set_Alname(ptbIdol->m_jRank[0U]["al_name"].asString());
        ptbMapItem->Set_Al_nick(ptbIdol->m_jRank[0U]["al_nick"].asString());
    }

    pstSession->m_ucMapItemFlag = EN_TABLE_UPDT_FLAG__CHANGE;

    if (ptbIdol->m_nAlid > 0)
    {
        SNoticInfo stNoticInfo;
        stNoticInfo.Reset();
        stNoticInfo.m_dwNoticId = EN_NOTI_ID__OCCUPY_IDOL;
        stNoticInfo.m_strLang = "english";
        CMsgBase::SendNotificationAlliance(CConfBase::GetString("tbxml_project"),
            ptbIdol->m_nSid, 0, ptbIdol->m_nAlid, stNoticInfo);
    }

    //TODO
//     if (ptbIdol->m_nAlid > 0)
//     {
//         string szTmp = ptbIdol->m_jRank[0U]["al_name"].asString() + "#" + CCommonFunc::NumToString(ptbIdol->m_nId) + "#" 
//             + CCommonFunc::NumToString(ptbIdol->m_nSid) + "#" + CCommonFunc::NumToString(ptbIdol->m_nPos);
//         CSendMessageBase::SendBroadcast(&pstSession->m_stTargetUser, ptbIdol->m_nSid, 0, EN_BROADCAST_CONTENT_ID__IDOL_BUFF_TIME, szTmp);
//     }

    pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;
    return 0;
}

TINT32 CProcessAction::ProcessMarchIdolAttack(SSession *pstSession)
{
    TbMarch_action *ptbReqMarch = &pstSession->m_stReqMarch;
    TbMap *ptbWild = &pstSession->m_stMapItem;
    TBOOL bSuccess = TRUE;
    TINT32 dwReason = EN_REPORT_RESULT_TARGET_CHANGE;
    SUserInfo *pstSUser = &pstSession->m_stSourceUser;
    TbPlayer *ptbSPlayer = &pstSUser->m_tbPlayer;

    // 任务合法性检查
    if ((ptbSPlayer->m_nStatus & EN_CITY_STATUS__NEW_PROTECTION)
        || (ptbSPlayer->m_nStatus & EN_CITY_STATUS__AVOID_WAR))
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchIdolAttack: srcPlayer[id=%ld] is in peace time, just return... [seq=%u]",
            ptbSPlayer->m_nUid, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_SELF_PEACETIME;
    }

    if (bSuccess && ptbWild->m_nType != EN_WILD_TYPE__IDOL)
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchIdolAttack: wild_type[%ld], just return... [seq=%u]",
            ptbWild->m_nType, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_CHANGE;
    }

    if (bSuccess && ptbWild->m_nStatus != EN_IDOL_STATUS__CONTEST_PERIOD)
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchIdolAttack: target[%ld] type[%ld] is in peace time, just return... [seq=%u]",
            ptbWild->m_nId, ptbWild->m_nStatus, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_IDOL_PEACE_TIME;
        if (ptbWild->m_nStatus == EN_IDOL_STATUS__THRONE_PEACE_TIME)
        {
            dwReason = EN_REPORT_RESULT_IDOL_PEACE_TIME_FOR_THRONE;
        }
    }

    if (bSuccess && ptbSPlayer->m_nAlpos == 0)
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchIdolAttack: not in alliance, just return... [seq=%u]",
            ptbWild->m_nId, ptbWild->m_nStatus, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_NOT_IN_ALLIANCE;
    }

    TBOOL bIsFind = FALSE;
    for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_udwIdolNum; udwIdx++)
    {
        if (ptbWild->m_nId == pstSession->m_atbIdol[udwIdx].m_nPos)
        {
            bIsFind = TRUE;
        }
    }

    if (bSuccess && !bIsFind)
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchIdolAttack: cannot find this idol[pos: %ld], just return... [seq=%u]",
            ptbWild->m_nId, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_PEACETIME;
    }

    if (bSuccess)
    {
        ptbReqMarch->Set_Status(EN_MARCH_STATUS__FIGHTING);
        ptbReqMarch->Set_Btime(ptbReqMarch->m_nEtime);
        ptbReqMarch->Set_Etime(ptbReqMarch->m_nBtime + 5);
        if (ptbReqMarch->m_nEtime < CTimeUtils::GetUnixTime())
        {
            ptbReqMarch->m_nEtime = CTimeUtils::GetUnixTime();
        }
        CSendMessageBase::AddTips(&pstSession->m_stSourceUser, EN_TIPS_TYPE__ATTACKING, ptbSPlayer->m_nUid, TRUE,
            ptbWild->m_nId, ptbWild->m_nType, ptbWild->m_nLevel, ptbWild->m_sUname.c_str());
    }
    else
    {
        CProcessReport::GenInValidReport(pstSession, EN_REPORT_TYPE_IDOL_ATTACK, dwReason, &pstSession->m_tbReport);

        ptbReqMarch->Set_Status(EN_MARCH_STATUS__RETURNING);
        ptbReqMarch->Set_Btime(ptbReqMarch->m_nEtime);
        ptbReqMarch->Set_Etime(ptbReqMarch->m_nBtime + ptbReqMarch->m_nCtime);
        //ptbReqMarch->DeleteField(TbMARCH_ACTION_FIELD_TAL);
        ptbReqMarch->Set_Tal(0, UPDATE_ACTION_TYPE_DELETE);
    }

    // 更新action状态
    pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;

    return 0;
}

TINT32 CProcessAction::ProcessMarchIdolFighting(SSession *pstSession)
{
    TbMarch_action *ptbReqMarch = &pstSession->m_stReqMarch;
    TbMap *ptbWild = &pstSession->m_stMapItem;
    TBOOL bSuccess = TRUE;
    TINT32 dwReason = EN_REPORT_RESULT_TARGET_CHANGE;
    SUserInfo *pstSUser = &pstSession->m_stSourceUser;
    TbPlayer *ptbSPlayer = &pstSUser->m_tbPlayer;

    // 任务合法性检查
    if ((ptbSPlayer->m_nStatus & EN_CITY_STATUS__NEW_PROTECTION)
        || (ptbSPlayer->m_nStatus & EN_CITY_STATUS__AVOID_WAR))
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchIdolFighting: srcPlayer[id=%ld] is in peace time, just return... [seq=%u]",
            ptbSPlayer->m_nUid, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_SELF_PEACETIME;
    }

    if (bSuccess && ptbWild->m_nType != EN_WILD_TYPE__IDOL)
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchIdolFighting: wild_type[%ld], just return... [seq=%u]",
            ptbWild->m_nType, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_CHANGE;
    }

    if (bSuccess && ptbWild->m_nStatus != EN_IDOL_STATUS__CONTEST_PERIOD)
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchIdolFighting: target[%ld] type[%ld] is in peace time, just return... [seq=%u]",
            ptbWild->m_nId, ptbWild->m_nStatus, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_IDOL_PEACE_TIME;
        if (ptbWild->m_nStatus == EN_IDOL_STATUS__THRONE_PEACE_TIME)
        {
            dwReason = EN_REPORT_RESULT_IDOL_PEACE_TIME_FOR_THRONE;
        }
    }

    if (bSuccess && ptbSPlayer->m_nAlpos == 0)
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchIdolFighting: not in alliance, just return... [seq=%u]",
            ptbWild->m_nId, ptbWild->m_nStatus, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_NOT_IN_ALLIANCE;
    }

    TBOOL bIsFind = FALSE;
    for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_udwIdolNum; udwIdx++)
    {
        if (ptbWild->m_nId == pstSession->m_atbIdol[udwIdx].m_nPos)
        {
            bIsFind = TRUE;
        }
    }

    if (bSuccess && !bIsFind)
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchIdolFighting: cannot find this idol[pos: %ld], just return... [seq=%u]",
            ptbWild->m_nId, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_PEACETIME;
    }

    if (bSuccess)
    {
        SBattleNode stDefend;
        SBattleNode stAttack;

        pstSession->m_dwMarchResult = CWarProcess::ProcessWar(pstSession, &stAttack, &stDefend, &pstSession->m_tbReport);

        TINT32 dwReportResult = pstSession->m_dwMarchResult;

        CProcessReport::GenAttackReport(pstSession, EN_REPORT_TYPE_IDOL_ATTACK, dwReportResult, &stAttack, &stDefend, &pstSession->m_tbReport);

        for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_udwIdolNum; udwIdx++)
        {
            if (ptbWild->m_nId == pstSession->m_atbIdol[udwIdx].m_nPos)
            {
                pstSession->m_ucIdolFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
                break;
            }
        }

        ptbReqMarch->Set_Status(EN_MARCH_STATUS__RETURNING);
        ptbReqMarch->Set_Btime(ptbReqMarch->m_nEtime);
        ptbReqMarch->Set_Etime(ptbReqMarch->m_nBtime + ptbReqMarch->m_nCtime);

        //Tips
        //TODO
        TUINT32 udwWinOrLose = pstSession->m_dwMarchResult == EN_REPORT_RESULT_WIN ? 0 : 1;
        CSendMessageBase::AddTips(&pstSession->m_stSourceUser, EN_TIPS_TYPE__WAR_RESULT, ptbSPlayer->m_nUid, TRUE,
            ptbWild->m_nId, ptbWild->m_nUid, udwWinOrLose, ptbWild->m_sUname.c_str(), "0|0|0|0|0|0");
    }
    else
    {
        CProcessReport::GenInValidReport(pstSession, EN_REPORT_TYPE_IDOL_ATTACK, dwReason, &pstSession->m_tbReport);

        ptbReqMarch->Set_Status(EN_MARCH_STATUS__RETURNING);
        ptbReqMarch->Set_Btime(ptbReqMarch->m_nEtime);
        ptbReqMarch->Set_Etime(ptbReqMarch->m_nBtime + ptbReqMarch->m_nCtime);
        //ptbReqMarch->DeleteField(TbMARCH_ACTION_FIELD_TAL);
        ptbReqMarch->Set_Tal(0, UPDATE_ACTION_TYPE_DELETE);
    }

    // 更新action状态
    pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;

    return 0;
}

TINT32 CProcessAction::ProcessThronePeaceTimePeriod(SSession *pstSession, TBOOL &bNeedResponse)
{
    TbMarch_action *ptbReqAction = &pstSession->m_stReqMarch;
    TbMap *ptbMapItem = &pstSession->m_stMapItem;

    TbThrone *ptbThrone = &pstSession->m_tbThrone;

    if (ptbMapItem->m_nType != EN_WILD_TYPE__THRONE_NEW
        || ptbThrone->m_nPos != ptbMapItem->m_nId)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessIdolThronePeaceTimePeriod: map_pos[%ld] type[%ld], throne_pos[%ld] just del action... [seq=%u]",
            ptbMapItem->m_nId, ptbMapItem->m_nType, ptbThrone->m_nPos, pstSession->m_udwSeqNo));

        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__DEL;
        return 0;
    }

    //王座状态不对...修复...
    if (ptbThrone->m_nStatus != EN_THRONE_STATUS__PEACE_TIME)
    {
        ptbThrone->Set_Status(EN_THRONE_STATUS__QUIET_PERIOD);
        ptbThrone->Set_End_time(0);
        ptbMapItem->Set_Status(ptbThrone->m_nStatus);
        ptbMapItem->Set_Time_end(ptbThrone->m_nEnd_time);
        pstSession->m_ucMapItemFlag = EN_TABLE_UPDT_FLAG__CHANGE;

        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__DEL;
        return 0;
    }

    // 无联盟占领  修正
    if (0 == ptbThrone->m_nAlid)
    {
        ptbThrone->Set_Status(EN_THRONE_STATUS__QUIET_PERIOD);
        ptbThrone->Set_End_time(0);
        ptbThrone->m_jInfo = Json::Value(Json::objectValue);
        ptbThrone->m_jInfo["buff"] = CGameInfo::GetInstance()->m_oJsonRoot["game_throne_buff"];
        ptbThrone->SetFlag(TbTHRONE_FIELD_INFO);
        ptbThrone->Set_Occupy_time(CTimeUtils::GetUnixTime());

        ptbMapItem->Set_Status(ptbThrone->m_nStatus);
        ptbMapItem->Set_Time_end(ptbThrone->m_nEnd_time);
        pstSession->m_ucMapItemFlag = EN_TABLE_UPDT_FLAG__CHANGE;

        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__DEL;
        return 0;
    }

    // 时间不对  修正
    if (CTimeUtils::GetUnixTime() < ptbThrone->m_nEnd_time)
    {
        ptbReqAction->Set_Etime(ptbThrone->m_nEnd_time);
        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;
        return 0;
    }

    ptbThrone->Set_Status(EN_THRONE_STATUS__QUIET_PERIOD);
    ptbThrone->Set_End_time(0);

    ptbMapItem->Set_Status(ptbThrone->m_nStatus);
    ptbMapItem->Set_Time_end(ptbThrone->m_nEnd_time);
    pstSession->m_ucMapItemFlag = EN_TABLE_UPDT_FLAG__CHANGE;

    CSendMessageBase::SendBroadcast(&pstSession->m_stTargetUser, ptbThrone->m_nSid, 0, EN_BROADCAST_CONTENT_ID__THRONE_PEACE_TIME_END);

    pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__DEL;
    return 0;
}

TINT32 CProcessAction::ProcessThroneContestPeriod(SSession *pstSession, TBOOL &bNeedResponse)
{
    TbMarch_action *ptbReqAction = &pstSession->m_stReqMarch;
    TbMap *ptbMapItem = &pstSession->m_stMapItem;

    TbThrone *ptbThrone = &pstSession->m_tbThrone;

    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();

    if (ptbMapItem->m_nType != EN_WILD_TYPE__THRONE_NEW
        || ptbThrone->m_nPos != ptbMapItem->m_nId)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessIdolThronePeaceTimePeriod: map_pos[%ld] type[%ld], throne_pos[%ld] status[%ld], just del action... [seq=%u]",
            ptbMapItem->m_nId, ptbMapItem->m_nType, ptbThrone->m_nPos, ptbThrone->m_nStatus, pstSession->m_udwSeqNo));

        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__DEL;
        return 0;
    }

    //王座状态不对...修复...
    if (ptbThrone->m_nStatus != EN_THRONE_STATUS__CONTEST_PERIOD)
    {
        ptbThrone->Set_Status(EN_THRONE_STATUS__QUIET_PERIOD);
        ptbThrone->Set_End_time(0);
        ptbMapItem->Set_Status(ptbThrone->m_nStatus);
        ptbMapItem->Set_Time_end(ptbThrone->m_nEnd_time);
        pstSession->m_ucMapItemFlag = EN_TABLE_UPDT_FLAG__CHANGE;

        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__DEL;
        return 0;
    }

    // 无联盟占领  修正
    if (0 == ptbThrone->m_nAlid)
    {
        ptbThrone->Set_Status(EN_THRONE_STATUS__QUIET_PERIOD);
        ptbThrone->Set_End_time(0);
        ptbThrone->m_jInfo = Json::Value(Json::objectValue);
        ptbThrone->m_jInfo["buff"] = CGameInfo::GetInstance()->m_oJsonRoot["game_throne_buff"];
        ptbThrone->SetFlag(TbTHRONE_FIELD_INFO);
        ptbThrone->Set_Occupy_time(CTimeUtils::GetUnixTime());

        ptbMapItem->Set_Status(ptbThrone->m_nStatus);
        ptbMapItem->Set_Time_end(ptbThrone->m_nEnd_time);
        pstSession->m_ucMapItemFlag = EN_TABLE_UPDT_FLAG__CHANGE;

        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__DEL;
        return 0;
    }

    // 时间不对  修正
    if (udwCurTime < ptbThrone->m_nEnd_time)
    {
        ptbReqAction->Set_Etime(ptbThrone->m_nEnd_time);
        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;
        return 0;
    }

    ptbThrone->Set_Status(EN_THRONE_STATUS__PEACE_TIME);
    ptbThrone->Set_End_time(udwCurTime + CCommonBase::GetGameBasicVal(EN_GAME_BASIC_THRONE_PEACE_TIME));

    ptbMapItem->Set_Status(ptbThrone->m_nStatus);
    ptbMapItem->Set_Time_end(ptbThrone->m_nEnd_time);
    pstSession->m_ucMapItemFlag = EN_TABLE_UPDT_FLAG__CHANGE;

    //TODO
//     if (ptbThrone->m_nAlid > 0)
//     {
//         TbReport tbTmpReport;
//         Json::Value jTmp = Json::Value(Json::objectValue);
//         jTmp["last_time"] = CCommonBase::GetGameBasicVal(EN_GAME_BASIC_THRONE_PEACE_TIME);
// 
//         tbTmpReport.Reset();
//         CProcessReport::GenThroneStatusReport(pstSession, EN_REPORT_TYPE_THRONE_PEACE_TIME, jTmp, &tbTmpReport);
//         pstSession->m_vecAlReport.push_back(tbTmpReport);
//         pstSession->m_vecReportAlReceiver.push_back(ptbThrone->m_nAlid);
//     }

    ptbReqAction->Set_Btime(udwCurTime);
    ptbReqAction->Set_Etime(ptbThrone->m_nEnd_time);
    ptbReqAction->Set_Status(EN_TITMER_THRONE_PERIOD_STATUS__PEACE_TIME);

    for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_udwIdolNum; udwIdx++)
    {
        pstSession->m_audwTmpMarchFlag[pstSession->m_udwTmpMarchNum] = EN_TABLE_UPDT_FLAG__NEW;
        TbMarch_action *ptbAction = &pstSession->m_atbTmpMarch[pstSession->m_udwTmpMarchNum];
        ptbAction->Set_Suid(0);
        ptbAction->Set_Id(CActionBase::GenMapActionId(pstSession->m_atbIdol[udwIdx].m_nSid, pstSession->m_atbIdol[udwIdx].m_nPos));
        ptbAction->Set_Sid(pstSession->m_atbIdol[udwIdx].m_nSid);
        ptbAction->Set_Tpos(pstSession->m_atbIdol[udwIdx].m_nPos);
        ptbAction->Set_Mclass(EN_ACTION_MAIN_CLASS__TIMER);
        ptbAction->Set_Sclass(EN_ACTION_SEC_CLASS__IDOL_PERIOD);
        ptbAction->Set_Status(EN_TITMER_IDOL_PERIOD_STATUS__THRONE_PEACE_TIME);
        ptbAction->Set_Etime(udwCurTime);
        pstSession->m_udwTmpMarchNum++;
    }

    if (pstSession->m_stTargetUser.m_tbAlliance.m_nAid == ptbThrone->m_nAlid)
    {
        string szTmp = pstSession->m_stTargetUser.m_tbAlliance.m_sName;
        CSendMessageBase::SendBroadcast(&pstSession->m_stTargetUser, ptbThrone->m_nSid, 0, EN_BROADCAST_CONTENT_ID__OCCUPY_THRONE_ENTIRE, szTmp);
        szTmp = pstSession->m_stTargetUser.m_tbAlliance.m_sOname;
        CSendMessageBase::SendBroadcast(&pstSession->m_stTargetUser, ptbThrone->m_nSid, 0, EN_BROADCAST_CONTENT_ID__BECOME_KING, szTmp);

        CMsgBase::SendOperateMail(0, EN_MAIL_ID__THRONE_HAS_BEEN_OCCUPIED, ptbThrone->m_nSid, 0, pstSession->m_stTargetUser.m_tbAlliance.m_sName.c_str(), 
            pstSession->m_stTargetUser.m_tbAlliance.m_sOname.c_str(), "");
    }

    pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;
    return 0;
}

TINT32 CProcessAction::ProcessMarchThroneAssign(SSession *pstSession)
{
    TbMarch_action *ptbReqMarch = &pstSession->m_stReqMarch;
    SUserInfo *pstSUser = &pstSession->m_stSourceUser;
    TbPlayer *ptbSPlayer = &pstSUser->m_tbPlayer;
    TbMap *ptbWild = &pstSession->m_stMapItem;

    TBOOL bSuccess = TRUE;
    TINT32 dwReason = EN_REPORT_RESULT_TARGET_CHANGE;

    // 任务合法性检查
    if (ptbWild->m_nType != EN_WILD_TYPE__THRONE_NEW || ptbWild->m_nId != pstSession->m_tbThrone.m_nPos)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchThroneAssign: target[pos:%ld type:%ld] [throne_pos:%ld] has changed, just return... [seq=%u]",
            ptbWild->m_nId, ptbWild->m_nType, pstSession->m_tbThrone.m_nPos, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_CHANGE;
    }

    if (bSuccess && (ptbSPlayer->m_nAlpos == 0 || ptbSPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET != ptbWild->m_nAlid))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchThroneAssign: target[salpos:%ld,sal:%ld,tal:%ld] not right, just return... [seq=%u]",
            ptbSPlayer->m_nAlpos, ptbSPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET, ptbWild->m_nAlid, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_DIFF_ALLIANCE;
    }

    if (bSuccess && ((ptbSPlayer->m_nStatus & EN_CITY_STATUS__NEW_PROTECTION)
        || (ptbSPlayer->m_nStatus & EN_CITY_STATUS__AVOID_WAR)))
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchThroneAssign: srcPlayer[id=%ld] is in peace time, just return... [seq=%u]",
            ptbSPlayer->m_nUid, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_SELF_PEACETIME;
    }

    if (bSuccess)
    {
        for (TUINT32 udwIdx = 0; udwIdx < pstSUser->m_udwMarchNum; ++udwIdx)
        {
            if (pstSUser->m_aucMarchFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
            {
                continue;
            }
            TbMarch_action* ptbThroneAssign = &pstSUser->m_atbMarch[udwIdx];
            if (ptbThroneAssign->m_nSclass == EN_ACTION_SEC_CLASS__ASSIGN_THRONE
                && ptbThroneAssign->m_nStatus == EN_MARCH_STATUS__DEFENDING)
            {
                TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchThroneAssign: exist hero assign, just return... [seq=%u]", pstSession->m_udwSeqNo));
                bSuccess = FALSE;
                dwReason = EN_REPORT_RESULT_EXIST_DRAGON;
                break;
            }
        }
    }

    if (bSuccess == TRUE)
    {
        CProcessReport::GenThroneAssignReport(pstSession, &pstSession->m_tbReport);

        ptbReqMarch->Set_Status(EN_MARCH_STATUS__DEFENDING);
        ptbReqMarch->Set_Btime(CTimeUtils::GetUnixTime());
        ptbReqMarch->Set_Etime(INT64_MAX);
        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;

        if (ptbReqMarch->m_bParam[0].m_stDragon.m_ddwLevel > 0)
        {
            CSendMessageBase::AddTips(&pstSession->m_stSourceUser, EN_TIPS_TYPY__ASSIGN_HERO_ARRIVE, ptbSPlayer->m_nUid, TRUE,
                ptbWild->m_nId, ptbWild->m_nType, ptbWild->m_nLevel);
        }
        else
        {
            CSendMessageBase::AddTips(&pstSession->m_stSourceUser, EN_TIPS_TYPE__ASSIGN_KNIGHT_ARRIVE, ptbSPlayer->m_nUid, TRUE,
                ptbWild->m_nId, ptbWild->m_nType, ptbWild->m_nLevel);
        }

        CCommonLogic::UpdateThroneInfo(pstSUser, &pstSession->m_tbThrone);
    }
    else//部队直接放回
    {
        CProcessReport::GenInValidReport(pstSession, EN_REPORT_TYPE_THRONE_DISPATCH, dwReason, &pstSession->m_tbReport);

        ptbReqMarch->Set_Status(EN_MARCH_STATUS__RETURNING);
        ptbReqMarch->Set_Btime(ptbReqMarch->m_nEtime);
        ptbReqMarch->Set_Etime(ptbReqMarch->m_nBtime + ptbReqMarch->m_nCtime);
        //ptbReqMarch->DeleteField(TbMARCH_ACTION_FIELD_SAL);
        ptbReqMarch->Set_Sal(0, UPDATE_ACTION_TYPE_DELETE);
        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;
    }

    return 0;
}

TINT32 CProcessAction::ProcessMarchThroneReinforce(SSession *pstSession)
{
    TINT32 dwRetCode = 0;
//     if (pstSession->m_stReqMarch.m_nTid > 0)
//     {
//         dwRetCode = ProcessMarchThroneReinforceAttack(pstSession);
//     }
//     else
//     {
        dwRetCode = ProcessMarchThroneReinforceDefend(pstSession);
//     }

    return dwRetCode;
}

TINT32 CProcessAction::ProcessMarchThroneReinforceAttack(SSession *pstSession)
{
    TbMarch_action *ptbRallyReinforce = &pstSession->m_stReqMarch;
    SUserInfo *pstSUser = &pstSession->m_stSourceUser;
    SUserInfo *pstTUser = &pstSession->m_stTargetUser;
    TbPlayer *ptbSPlayer = &pstSUser->m_tbPlayer;
    TbMap *ptbWild = &pstSession->m_stMapItem;

    TbMarch_action* ptbRallyWar = NULL;
    TINT32 dwRallyWarIndex = -1;
    dwRallyWarIndex = CActionBase::GetMarchIndex(pstSUser->m_atbMarch, pstSUser->m_udwMarchNum, ptbRallyReinforce->m_nTid);
    if (dwRallyWarIndex >= 0)
    {
        ptbRallyWar = &pstSUser->m_atbMarch[dwRallyWarIndex];
    }

    SCityInfo *pstCity = &pstSUser->m_stCityInfo;

    TBOOL bSuccess = TRUE;
    TINT32 dwReason = EN_REPORT_RESULT_TARGET_CHANGE;

    // 任务合法性检查
    if (bSuccess && (ptbWild->m_nType != EN_WILD_TYPE__CITY || ptbWild->m_nUid != ptbRallyReinforce->m_nTuid
        || pstSession->m_bCheckValidFlag == FALSE))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchThroneReinforceAttack: check_valid=%d target[type:%ld] not city or target[uid:%ld,%ld] has changed, just return... [seq=%u]",
            pstSession->m_bCheckValidFlag, ptbWild->m_nType, ptbRallyReinforce->m_nTuid, ptbWild->m_nUid, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_CHANGE;
    }
    if (bSuccess && (ptbSPlayer->m_nAlpos == 0 || ptbSPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET != ptbWild->m_nAlid))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchThroneReinforceAttack: target[salpos:%ld,sal:%ld,tal:%ld] not right, just return... [seq=%u]",
            ptbSPlayer->m_nAlpos, ptbSPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET, ptbWild->m_nAlid, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_DIFF_ALLIANCE;
    }
    if (bSuccess && ptbRallyWar == NULL)
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchThroneReinforceAttack: main action[id=%ld] not existed or go, just return... [seq=%u]",
            ptbRallyReinforce->m_nTid, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_RALLY_WAR_END;
    }
    if (bSuccess && ptbRallyWar->m_nStatus != EN_MARCH_STATUS__PREPARING && ptbRallyWar->m_nStatus != EN_MARCH_STATUS__WAITING)
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchThroneReinforceAttack: main status[%d] just return... [seq=%u]", ptbRallyWar->m_nStatus, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_RALLY_WAR_END;
    }

    // 更新action状态
    if (bSuccess == TRUE)
    {
        ptbRallyReinforce->Set_Status(EN_MARCH_STATUS__PREPARING);
        ptbRallyReinforce->Set_Etime(INT64_MAX);
        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;

        CQuestLogic::SetTaskCurValue(pstSUser, pstCity, EN_TASK_TYPE_ING_REINFORCE_NUM);

        if (ptbRallyWar->m_nStatus == EN_MARCH_STATUS__WAITING)
        {
            TBOOL bNeedWait = FALSE;
            TINT64 ddwEtime = 0;
            for (TUINT32 udwIdx = 0; udwIdx < pstTUser->m_udwPassiveMarchNum; ++udwIdx)
            {
                if (pstTUser->m_atbPassiveMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_THRONE
                    && pstTUser->m_atbPassiveMarch[udwIdx].m_nTid == ptbRallyWar->m_nId
                    && pstTUser->m_atbPassiveMarch[udwIdx].m_nId != ptbRallyReinforce->m_nId
                    && pstTUser->m_atbPassiveMarch[udwIdx].m_nStatus == EN_MARCH_STATUS__MARCHING)
                {
                    bNeedWait = TRUE;
                    if (ddwEtime < pstSUser->m_atbPassiveMarch[udwIdx].m_nEtime)
                    {
                        ddwEtime = pstSUser->m_atbPassiveMarch[udwIdx].m_nEtime;
                    }
                }
            }
            if (bNeedWait)
            {
                ptbRallyWar->Set_Etime(ddwEtime);
            }
            else
            {
                ptbRallyWar->Set_Etime(CTimeUtils::GetUnixTime());
            }
        }

        CMsgBase::SendEncourageMail(&pstSUser->m_tbUserStat, pstSUser->m_tbPlayer.m_nSid, EN_MAIL_ID__FIRST_REINFORCE);
    }
    else//部队直接放回
    {
        CProcessReport::GenInValidReport(pstSession, EN_REPORT_TYPE_THRONE_REINFORCE, dwReason, &pstSession->m_tbReport);

        CActionBase::ReturnMarch(ptbRallyReinforce);
        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;

        if (ptbRallyWar != NULL && ptbRallyWar->m_nStatus == EN_MARCH_STATUS__WAITING)
        {
            ptbRallyWar->Set_Etime(CTimeUtils::GetUnixTime());
        }
    }

    return 0;
}

TINT32 CProcessAction::ProcessMarchThroneReinforceDefend(SSession *pstSession)
{
    TbMarch_action *ptbReqMarch = &pstSession->m_stReqMarch;
    SActionMarchParam *pstParam = &ptbReqMarch->m_bParam[0];
    SUserInfo *pstSUser = &pstSession->m_stSourceUser;
    TbPlayer *ptbSPlayer = &pstSUser->m_tbPlayer;
    TbMap *ptbWild = &pstSession->m_stMapItem;
    TbThrone *ptbThrone = &pstSession->m_tbThrone;

    TBOOL bSuccess = TRUE;
    TINT32 dwReason = EN_REPORT_RESULT_TARGET_CHANGE;
    TUINT64 uddwShareMight = 0;

    SCityInfo *pstCity = &pstSUser->m_stCityInfo;

    // 任务合法性检查
    if (ptbWild->m_nType != EN_WILD_TYPE__THRONE_NEW || ptbWild->m_nId != pstSession->m_tbThrone.m_nPos)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchThroneReinforce: target[pos:%ld type:%ld] [throne_pos:%ld] has changed, just return... [seq=%u]",
            ptbWild->m_nId, ptbWild->m_nType, pstSession->m_tbThrone.m_nPos, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_CHANGE;
    }

    if (bSuccess && (ptbSPlayer->m_nAlpos == 0 || ptbSPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET != ptbWild->m_nAlid))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchThroneReinforce: target[salpos:%ld,sal:%ld,tal:%ld] not right, just return... [seq=%u]",
            ptbSPlayer->m_nAlpos, ptbSPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET, ptbWild->m_nAlid, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_DIFF_ALLIANCE;
    }

    if (bSuccess && ((ptbSPlayer->m_nStatus & EN_CITY_STATUS__NEW_PROTECTION)
        || (ptbSPlayer->m_nStatus & EN_CITY_STATUS__AVOID_WAR)))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchThroneReinforce: srcPlayer[id=%ld] is in peace time, just return... [seq=%u]",
            ptbSPlayer->m_nUid, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_SELF_PEACETIME;
    }

    if (bSuccess)
    {
        TUINT32 udwReinforceLimit = CCommonBase::GetGameBasicVal(EN_GAME_BASIC_RALLY_SLOT_LIMIT);
        TUINT32 udwTroopLimit = CCommonBase::GetGameBasicVal(EN_GAME_BASIC_THRONE_TROOP_NUM_LIMIT);
        TUINT32 udwTotalReinforce = 0;
        TUINT32 udwTroopNum = CToolBase::GetTroopSumNum(ptbReqMarch->m_bParam[0].m_stTroop);
        TUINT32 udwTotalTroopNum = 0;
        for (TUINT32 udwIdx = 0; udwIdx < pstSUser->m_udwMarchNum; udwIdx++)
        {
            if (pstSUser->m_aucMarchFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
            {
                continue;
            }
            if (pstSUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_THRONE
                && pstSUser->m_atbMarch[udwIdx].m_nTpos == ptbThrone->m_nPos
                && pstSUser->m_atbMarch[udwIdx].m_nStatus == EN_MARCH_STATUS__DEFENDING)
            {
                udwTotalReinforce++;
                udwTotalTroopNum += CToolBase::GetTroopSumNum(pstSUser->m_atbMarch[udwIdx].m_bParam[0].m_stTroop);
            }
        }

        if (udwTotalReinforce + 1 > udwReinforceLimit || udwTotalTroopNum + udwTroopNum > udwTroopLimit)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchThroneReinforce: throne reinforce arrive limit[%u %u %u %u %u], just return... [seq=%u]",
                udwTotalReinforce, udwReinforceLimit, udwTotalTroopNum, udwTroopNum, udwTroopLimit, pstSession->m_udwSeqNo));
            bSuccess = FALSE;
            dwReason = EN_REPORT_RESULT_TARGET_REINFORCE_FULL;
        }
    }

    // 更新action状态, 将部队设置为encamp
    if (bSuccess == TRUE)
    {
        ptbReqMarch->Set_Status(EN_MARCH_STATUS__DEFENDING);
        ptbReqMarch->Set_Btime(CTimeUtils::GetUnixTime());
        ptbReqMarch->Set_Etime(INT64_MAX);
        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;

        CProcessReport::GenThroneReinforceReport(pstSession, &pstSession->m_tbReport);

        CQuestLogic::SetTaskCurValue(pstSUser, pstCity, EN_TASK_TYPE_ING_REINFORCE_NUM);

        CMsgBase::SendEncourageMail(&pstSUser->m_tbUserStat, pstSUser->m_tbPlayer.m_nSid, EN_MAIL_ID__FIRST_REINFORCE);

        CCommonLogic::UpdateThroneInfo(pstSUser, ptbThrone);
    }
    else//部队直接放回
    {
        CProcessReport::GenInValidReport(pstSession, EN_REPORT_TYPE_THRONE_REINFORCE, dwReason, &pstSession->m_tbReport);

        ptbReqMarch->Set_Status(EN_MARCH_STATUS__RETURNING);
        ptbReqMarch->Set_Btime(ptbReqMarch->m_nEtime);
        ptbReqMarch->Set_Etime(ptbReqMarch->m_nBtime + ptbReqMarch->m_nCtime);
        //ptbReqMarch->DeleteField(TbMARCH_ACTION_FIELD_SAL);
        ptbReqMarch->Set_Sal(0, UPDATE_ACTION_TYPE_DELETE);
        pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;
    }

    return 0;
}

TINT32 CProcessAction::ProcessMarchThroneAttack(SSession *pstSession)
{
    TbMarch_action *ptbReqMarch = &pstSession->m_stReqMarch;
    SActionMarchParam *pstParam = &ptbReqMarch->m_bParam[0];
    TbMap *ptbWild = &pstSession->m_stMapItem;
    TBOOL bSuccess = TRUE;
    TINT32 dwReason = EN_REPORT_RESULT_TARGET_CHANGE;
    SUserInfo *pstSUser = &pstSession->m_stSourceUser;
    TbDiplomacy *ptbDip = NULL;
    TbPlayer *ptbSPlayer = &pstSUser->m_tbPlayer;
    SUserInfo *pstTUser = &pstSession->m_stTargetUser;

    if (ptbWild->m_nAlid != 0 && ptbSPlayer->m_nAlpos != 0)
    {
        ptbDip = CCommonBase::GetDiplomacy(pstSession->m_stSourceUser.m_atbDiplomacy, pstSession->m_stSourceUser.m_udwDiplomacyNum,
            ptbWild->m_nAlid);
    }

    // 任务合法性检查
    if ((ptbSPlayer->m_nStatus & EN_CITY_STATUS__NEW_PROTECTION)
        || (ptbSPlayer->m_nStatus & EN_CITY_STATUS__AVOID_WAR))
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchThroneAttack: srcPlayer[id=%ld] is in peace time, just return... [seq=%u]",
            ptbSPlayer->m_nUid, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_SELF_PEACETIME;
    }

    if (bSuccess && (ptbWild->m_nType != EN_WILD_TYPE__THRONE_NEW || ptbWild->m_nId != pstSession->m_tbThrone.m_nPos))
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchThroneAttack: wild_type[%ld], just return... [seq=%u]",
            ptbReqMarch->m_nSuid, ptbReqMarch->m_nTuid, ptbWild->m_nType, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_CHANGE;
    }

    if (bSuccess && ptbSPlayer->m_nAlpos != 0 && ptbSPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET == ptbWild->m_nAlid)
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchThroneAttack: sal[%u]=tal[%u], just return... [seq=%u]",
            pstParam->m_ddwSourceAlliance, pstParam->m_ddwTargetAlliance, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_SAME_ALLIANCE;
    }

    if (bSuccess && ptbDip && ptbDip->m_nType == EN_DIPLOMACY_TYPE__FRIENDLY)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchThroneAttack: sal[%u] and tal[%u] is friend, just return... [seq=%u]",
            ptbSPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET, ptbWild->m_nAlid, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_IS_FRIEND;
    }

    if (bSuccess && ptbWild->m_nStatus == EN_THRONE_STATUS__PEACE_TIME)
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchThroneAttack: target[%ld] is in peace time, just return... [seq=%u]",
            ptbWild->m_nId, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_PEACETIME;
    }

//     if (bSuccess)
//     {
//         ptbReqMarch->Set_Status(EN_MARCH_STATUS__FIGHTING);
//         ptbReqMarch->Set_Btime(ptbReqMarch->m_nEtime);
//         ptbReqMarch->Set_Etime(ptbReqMarch->m_nBtime + 5);
//         if (ptbReqMarch->m_nEtime < CTimeUtils::GetUnixTime())
//         {
//             ptbReqMarch->m_nEtime = CTimeUtils::GetUnixTime();
//         }
//     }
//     else
//     {
//         CProcessReport::GenInValidReport(pstSession, EN_REPORT_TYPE_THRONE_ATTACK, dwReason, &pstSession->m_tbReport);
// 
//         ptbReqMarch->Set_Status(EN_MARCH_STATUS__RETURNING);
//         ptbReqMarch->Set_Btime(ptbReqMarch->m_nEtime);
//         ptbReqMarch->Set_Etime(ptbReqMarch->m_nBtime + ptbReqMarch->m_nCtime);
//         //ptbReqMarch->DeleteField(TbMARCH_ACTION_FIELD_TAL);
//         ptbReqMarch->Set_Tal(0, UPDATE_ACTION_TYPE_DELETE);
//     }
// 
//     // 更新action状态
//     pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;
// 
//     return 0;

    if (bSuccess)
    {
        ptbReqMarch->Set_Status(EN_MARCH_STATUS__FIGHTING);
        ptbReqMarch->Set_Btime(ptbReqMarch->m_nEtime);
        ptbReqMarch->Set_Etime(ptbReqMarch->m_nBtime + 5);
        if (ptbReqMarch->m_nEtime < CTimeUtils::GetUnixTime())
        {
            ptbReqMarch->m_nEtime = CTimeUtils::GetUnixTime();
        }
        if (ptbWild->m_nUid == 0)
        {
            CSendMessageBase::AddTips(&pstSession->m_stSourceUser, EN_TIPS_TYPE__ATTACKING, ptbSPlayer->m_nUid, TRUE,
                ptbWild->m_nId, ptbWild->m_nType, ptbWild->m_nLevel, ptbWild->m_sUname.c_str());
        }
        else
        {
            CSendMessageBase::AddTips(&pstSession->m_stSourceUser, EN_TIPS_TYPE__ATTACKING, ptbSPlayer->m_nUid, TRUE,
                ptbWild->m_nId, ptbWild->m_nType, ptbWild->m_nLevel, pstSession->m_stTargetUser.m_tbPlayer.m_sUin.c_str());
        }
    }
    else
    {
        CProcessReport::GenInValidReport(pstSession, EN_REPORT_TYPE_THRONE_ATTACK, dwReason, &pstSession->m_tbReport);

        ptbReqMarch->Set_Status(EN_MARCH_STATUS__RETURNING);
        ptbReqMarch->Set_Btime(ptbReqMarch->m_nEtime);
        ptbReqMarch->Set_Ctime(pstParam->m_ddwMarchingTime);
        ptbReqMarch->Set_Etime(ptbReqMarch->m_nBtime + pstParam->m_ddwMarchingTime);
        ptbReqMarch->Set_Tal(0, UPDATE_ACTION_TYPE_DELETE);
    }

    TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessMarchAttack: suid[%lld] tuid[%lld] wild_type[%ld] [user_status=%u wild_id=%u wild_status=%u] [seq=%u]",
        ptbReqMarch->m_nSuid, ptbReqMarch->m_nTuid, ptbWild->m_nType, pstSession->m_stTargetUser.m_tbPlayer.m_nStatus, ptbWild->m_nId, ptbWild->m_nStatus, pstSession->m_udwSeqNo));

    // 更新action状态
    pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;

    return 0;
}

TINT32 CProcessAction::ProcessMarchThroneRallyWar(SSession *pstSession)
{
    TbMarch_action *ptbRallyWar = &pstSession->m_stReqMarch;
    SActionMarchParam *pstParam = &ptbRallyWar->m_bParam[0];
    TbMap *ptbWild = &pstSession->m_stMapItem;
    TBOOL bSuccess = TRUE;
    TINT32 dwReason = EN_REPORT_RESULT_TARGET_CHANGE;
    SUserInfo *pstSUser = &pstSession->m_stSourceUser;
    SUserInfo *pstTUser = &pstSession->m_stTargetUser;
    TbDiplomacy *ptbDip = NULL;
    TbPlayer *ptbSPlayer = &pstSUser->m_tbPlayer;
    if (ptbWild->m_nAlid != 0 && ptbSPlayer->m_nAlpos != 0)
    {
        ptbDip = CCommonBase::GetDiplomacy(pstSession->m_stSourceUser.m_atbDiplomacy, pstSession->m_stSourceUser.m_udwDiplomacyNum,
            ptbWild->m_nAlid);
    }

    // 任务合法性检查
    if ((ptbSPlayer->m_nStatus & EN_CITY_STATUS__NEW_PROTECTION)
        || (ptbSPlayer->m_nStatus & EN_CITY_STATUS__AVOID_WAR))
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchThroneRallyWar: srcPlayer[id=%ld] is in peace time, just return... [seq=%u]",
            ptbSPlayer->m_nUid, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_SELF_PEACETIME;
    }

    if (bSuccess && (ptbWild->m_nType != EN_WILD_TYPE__THRONE_NEW || ptbWild->m_nId != pstSession->m_tbThrone.m_nPos))
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchThroneRallyWar: rally war throne: wild_type[%ld], just return... [seq=%u]",
            ptbWild->m_nType, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_CHANGE;
    }

    if (bSuccess && ptbSPlayer->m_nAlpos != 0 && ptbSPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET == ptbWild->m_nAlid)
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchThroneRallyWar: sal[%u]=tal[%u], just return... [seq=%u]",
            pstParam->m_ddwSourceAlliance, pstParam->m_ddwTargetAlliance, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_SAME_ALLIANCE;
    }

    if (bSuccess && ptbDip && ptbDip->m_nType == EN_DIPLOMACY_TYPE__FRIENDLY)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchThroneRallyWar: sal[%u] and tal[%u] is friend, just return... [seq=%u]",
            ptbSPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET, ptbWild->m_nAlid, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_IS_FRIEND;
    }

    if (bSuccess && ptbWild->m_nStatus == EN_THRONE_STATUS__PEACE_TIME)
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchThroneRallyWar: target[%ld] is in peace time, just return... [seq=%u]",
            ptbWild->m_nId, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_PEACETIME;
    }

//     if (bSuccess)
//     {
//         TUINT32 udwFightTime = 5;
// 
//         ptbRallyWar->Set_Status(EN_MARCH_STATUS__FIGHTING);
//         ptbRallyWar->Set_Btime(ptbRallyWar->m_nEtime);
//         ptbRallyWar->Set_Ctime(udwFightTime);
//         ptbRallyWar->Set_Etime(ptbRallyWar->m_nBtime + udwFightTime);
//     }
//     else
//     {
//         CProcessReport::GenInValidReport(pstSession, EN_REPORT_TYPE_THRONE_RALLY_WAR, dwReason, &pstSession->m_tbReport);
// 
//         ptbRallyWar->Set_Status(EN_MARCH_STATUS__RETURNING);
//         ptbRallyWar->Set_Btime(ptbRallyWar->m_nEtime);
//         ptbRallyWar->Set_Etime(ptbRallyWar->m_nBtime + ptbRallyWar->m_nCtime);
//         //ptbRallyWar->DeleteField(TbMARCH_ACTION_FIELD_TAL);
//         //ptbRallyWar->DeleteField(TbMARCH_ACTION_FIELD_SAL);
//         ptbRallyWar->Set_Sal(0, UPDATE_ACTION_TYPE_DELETE);
//         ptbRallyWar->Set_Tal(0, UPDATE_ACTION_TYPE_DELETE);
// 
//         for (TUINT32 udwIdx = 0; udwIdx < pstSUser->m_udwMarchNum; ++udwIdx)
//         {
//             TbMarch_action* ptbRallyReinforce = &pstSUser->m_atbMarch[udwIdx];
//             if (ptbRallyReinforce->m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_THRONE
//                 && ptbRallyReinforce->m_nStatus == EN_MARCH_STATUS__PREPARING
//                 && ptbRallyReinforce->m_nTid == ptbRallyWar->m_nId)
//             {
//                 CActionBase::ReturnMarch(ptbRallyReinforce, ptbRallyWar->m_nTpos, ptbRallyWar->m_bParam[0].m_ddwTargetType);
//                 // 添加盟友idlist，用于发送report
//                 CToolBase::AddUserToMailReceiverList(pstSUser->m_adwMailSendUidList, pstSUser->m_udwMailSendNum, ptbRallyReinforce->m_nSuid);
//             }
//         }
//     }
// 
//     // 更新action状态
//     pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;
//     return 0;

    if (bSuccess)
    {
        TUINT32 udwFightTime = 5;

        ptbRallyWar->Set_Status(EN_MARCH_STATUS__FIGHTING);
        ptbRallyWar->Set_Btime(ptbRallyWar->m_nEtime);
        ptbRallyWar->Set_Ctime(udwFightTime);
        ptbRallyWar->Set_Etime(ptbRallyWar->m_nBtime + udwFightTime);
        //给发起者tips
        CSendMessageBase::AddTips(&pstSession->m_stSourceUser, EN_TIPS_TYPE__ATTACKING, ptbSPlayer->m_nUid, TRUE,
            ptbWild->m_nId, ptbWild->m_nType, ptbWild->m_nLevel, ptbWild->m_sUname.c_str());
        //给支援者tips
        for (TUINT32 udwIdx = 0; udwIdx < pstSUser->m_udwMarchNum; ++udwIdx)
        {
            if (pstSUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__RALLY_REINFORCE
                && pstSUser->m_atbMarch[udwIdx].m_nTid == ptbRallyWar->m_nId
                && pstSUser->m_atbMarch[udwIdx].m_nStatus == EN_MARCH_STATUS__PREPARING)
            {
                CSendMessageBase::AddTips(&pstSession->m_stSourceUser, EN_TIPS_TYPE__ATTACKING, pstSUser->m_atbMarch[udwIdx].m_nSuid, TRUE,
                    ptbWild->m_nId, ptbWild->m_nType, ptbWild->m_nLevel, ptbWild->m_sUname.c_str());
            }
        }
    }
    else
    {
        CProcessReport::GenInValidReport(pstSession, EN_REPORT_TYPE_THRONE_RALLY_WAR, dwReason, &pstSession->m_tbReport);

        ptbRallyWar->Set_Status(EN_MARCH_STATUS__RETURNING);
        ptbRallyWar->Set_Btime(ptbRallyWar->m_nEtime);
        ptbRallyWar->Set_Ctime(pstParam->m_ddwMarchingTime);
        ptbRallyWar->Set_Etime(ptbRallyWar->m_nBtime + ptbRallyWar->m_nCtime);
        ptbRallyWar->Set_Sal(0, UPDATE_ACTION_TYPE_DELETE);
        ptbRallyWar->Set_Tal(0, UPDATE_ACTION_TYPE_DELETE);

        for (TUINT32 udwIdx = 0; udwIdx < pstSUser->m_udwMarchNum; ++udwIdx)
        {
            TbMarch_action* ptbRallyReinforce = &pstSUser->m_atbMarch[udwIdx];
            if (ptbRallyReinforce->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_REINFORCE
                && ptbRallyReinforce->m_nStatus == EN_MARCH_STATUS__PREPARING
                && ptbRallyReinforce->m_nTid == ptbRallyWar->m_nId)
            {
                CActionBase::ReturnMarch(ptbRallyReinforce, ptbRallyWar->m_nTpos, ptbRallyWar->m_bParam[0].m_ddwTargetType);
                // 添加盟友idlist，用于发送report
                CToolBase::AddUserToMailReceiverList(pstSUser->m_adwMailSendUidList, pstSUser->m_udwMailSendNum, ptbRallyReinforce->m_nSuid);
            }
        }
    }

    // 更新action状态
    pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;
    return 0;
}

TINT32 CProcessAction::ProcessMarchThroneFighting(SSession *pstSession)
{
    TbMarch_action *ptbReqMarch = &pstSession->m_stReqMarch;
    SActionMarchParam *pstParam = &ptbReqMarch->m_bParam[0];
    TbMap *ptbWild = &pstSession->m_stMapItem;
    TBOOL bSuccess = TRUE;
    TINT32 dwReason = EN_REPORT_RESULT_TARGET_CHANGE;
    SUserInfo *pstSUser = &pstSession->m_stSourceUser;
    TbDiplomacy *ptbDip = NULL;
    TbPlayer *ptbSPlayer = &pstSUser->m_tbPlayer;
    SUserInfo *pstTUser = &pstSession->m_stTargetUser;

    if (ptbWild->m_nAlid != 0 && ptbSPlayer->m_nAlpos != 0)
    {
        ptbDip = CCommonBase::GetDiplomacy(pstSession->m_stSourceUser.m_atbDiplomacy, pstSession->m_stSourceUser.m_udwDiplomacyNum,
            ptbWild->m_nAlid);
    }

    // 任务合法性检查
    if ((ptbSPlayer->m_nStatus & EN_CITY_STATUS__NEW_PROTECTION)
        || (ptbSPlayer->m_nStatus & EN_CITY_STATUS__AVOID_WAR))
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchThroneFighting: srcPlayer[id=%ld] is in peace time, just return... [seq=%u]",
            ptbSPlayer->m_nUid, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_SELF_PEACETIME;
    }

    if (bSuccess && (ptbWild->m_nType != EN_WILD_TYPE__THRONE_NEW || ptbWild->m_nId != pstSession->m_tbThrone.m_nPos))
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchThroneFighting: wild_type[%ld], just return... [seq=%u]",
            ptbReqMarch->m_nSuid, ptbReqMarch->m_nTuid, ptbWild->m_nType, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_CHANGE;
    }

    if (bSuccess && ptbSPlayer->m_nAlpos != 0 && ptbSPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET == ptbWild->m_nAlid)
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchThroneFighting: sal[%u]=tal[%u], just return... [seq=%u]",
            pstParam->m_ddwSourceAlliance, pstParam->m_ddwTargetAlliance, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_SAME_ALLIANCE;
    }

    if (bSuccess && ptbDip && ptbDip->m_nType == EN_DIPLOMACY_TYPE__FRIENDLY)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchThroneFighting: sal[%u] and tal[%u] is friend, just return... [seq=%u]",
            ptbSPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET, ptbWild->m_nAlid, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_IS_FRIEND;
    }

    if (bSuccess && ptbWild->m_nStatus == EN_THRONE_STATUS__PEACE_TIME)
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchThroneFighting: target[%ld] is in peace time, just return... [seq=%u]",
            ptbWild->m_nId, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_PEACETIME;
    }

    //     if (bSuccess)
    //     {
    //         ptbReqMarch->Set_Status(EN_MARCH_STATUS__FIGHTING);
    //         ptbReqMarch->Set_Btime(ptbReqMarch->m_nEtime);
    //         ptbReqMarch->Set_Etime(ptbReqMarch->m_nBtime + 5);
    //         if (ptbReqMarch->m_nEtime < CTimeUtils::GetUnixTime())
    //         {
    //             ptbReqMarch->m_nEtime = CTimeUtils::GetUnixTime();
    //         }
    //     }
    //     else
    //     {
    //         CProcessReport::GenInValidReport(pstSession, EN_REPORT_TYPE_THRONE_ATTACK, dwReason, &pstSession->m_tbReport);
    // 
    //         ptbReqMarch->Set_Status(EN_MARCH_STATUS__RETURNING);
    //         ptbReqMarch->Set_Btime(ptbReqMarch->m_nEtime);
    //         ptbReqMarch->Set_Etime(ptbReqMarch->m_nBtime + ptbReqMarch->m_nCtime);
    //         //ptbReqMarch->DeleteField(TbMARCH_ACTION_FIELD_TAL);
    //         ptbReqMarch->Set_Tal(0, UPDATE_ACTION_TYPE_DELETE);
    //     }
    // 
    //     // 更新action状态
    //     pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;
    // 
    //     return 0;

    if (bSuccess)
    {
        SBattleNode stDefend;
        SBattleNode stAttack;

        if (pstTUser->m_tbAlliance.m_nAid > 0)
        {
            ptbReqMarch->m_bParam[0].m_ddwTargetUserId = pstTUser->m_tbAlliance.m_nOid;
            strncpy(ptbReqMarch->m_bParam[0].m_szTargetUserName, pstSession->m_stTargetUser.m_tbAlliance.m_sOname.c_str(), MAX_TABLE_NAME_LEN);
            ptbReqMarch->m_bParam[0].m_szTargetUserName[MAX_TABLE_NAME_LEN - 1] = 0;
            strncpy(ptbReqMarch->m_bParam[0].m_szTargetAlliance, pstSession->m_stTargetUser.m_tbAlliance.m_sName.c_str(), MAX_TABLE_NAME_LEN);
            ptbReqMarch->m_bParam[0].m_szTargetAlliance[MAX_TABLE_NAME_LEN - 1] = 0;
            strncpy(ptbReqMarch->m_bParam[0].m_szTargetAlNick, pstSession->m_stTargetUser.m_tbAlliance.m_sAl_nick_name.c_str(), MAX_TABLE_NAME_LEN);
            ptbReqMarch->m_bParam[0].m_szTargetAlNick[MAX_TABLE_NAME_LEN - 1] = 0;
        }
        ptbReqMarch->m_bParam[0].m_ddwSourceAlliance = pstSUser->m_tbAlliance.m_nAid;

        pstSession->m_dwMarchResult = CWarProcess::ProcessWar(pstSession, &stAttack, &stDefend, &pstSession->m_tbReport);

        //player info 统计 不统计支援类action的兵力情况
        CWarProcess::ComputeWarResult(pstSession, &stAttack, &stDefend, pstSession->m_dwMarchResult, 0);

        TINT32 dwReportResult = pstSession->m_dwMarchResult;

        CProcessReport::GenAttackReport(pstSession, EN_REPORT_TYPE_THRONE_ATTACK, dwReportResult, &stAttack, &stDefend, &pstSession->m_tbReport);

        //TODO
        //         TbReport tbTmpReport;
        //         if (dwReportResult == EN_REPORT_RESULT_WIN)
        //         {
        //             Json::Value jTmp = Json::Value(Json::objectValue);
        //             jTmp["last_time"] = CCommonBase::GetGameBasicVal(EN_GAME_BASIC_THRONE_FIGHT_TIME);
        //             if (pstTUser->m_tbAlliance.m_nAid > 0)
        //             {
        //                 tbTmpReport.Reset();
        //                 CProcessReport::GenThroneStatusReport(pstSession, EN_REPORT_TYPE_LOST_THRONE, jTmp, &tbTmpReport);
        //                 pstSession->m_vecAlReport.push_back(tbTmpReport);
        //                 pstSession->m_vecReportAlReceiver.push_back(pstTUser->m_tbAlliance.m_nAid);
        //             }
        // 
        //             tbTmpReport.Reset();
        //             CProcessReport::GenThroneStatusReport(pstSession, EN_REPORT_TYPE_OCCUPY_THRONE, jTmp, &tbTmpReport);
        //             pstSession->m_vecAlReport.push_back(tbTmpReport);
        //             pstSession->m_vecReportAlReceiver.push_back(pstSUser->m_tbAlliance.m_nAid);
        //         }

        if (pstSession->m_dwMarchResult == EN_REPORT_RESULT_WIN)
        {
            CSendMessageBase::SendBroadcast(pstSUser, ptbReqMarch->m_nSid, 0, EN_BROADCAST_CONTENT_ID__BECOME_KING, pstSUser->m_tbAlliance.m_sOname);
        }

        TUINT32 udwWinOrLose = pstSession->m_dwMarchResult == EN_REPORT_RESULT_WIN ? 0 : 1;
        CSendMessageBase::AddTips(&pstSession->m_stSourceUser, EN_TIPS_TYPE__WAR_RESULT, ptbSPlayer->m_nUid, TRUE,
            ptbWild->m_nId, ptbWild->m_nUid, udwWinOrLose, ptbWild->m_sUname.c_str(), "0|0|0|0|0|0");

        if (pstTUser->m_tbPlayer.m_nUid)
        {
            TUINT32 udwTargetWinOrLose = pstSession->m_dwMarchResult == EN_REPORT_RESULT_LOSE ? 2 : 3;
            CSendMessageBase::AddTips(&pstSession->m_stTargetUser, EN_TIPS_TYPE__WAR_RESULT, pstTUser->m_tbPlayer.m_nUid, TRUE,
                ptbWild->m_nId, ptbSPlayer->m_nUid, udwTargetWinOrLose,
                ptbSPlayer->m_sUin.c_str(), "0|0|0|0|0|0");

            SNoticInfo stNoticInfo;
            if (pstSession->m_dwMarchResult == EN_REPORT_RESULT_WIN)
            {
                //推送
                stNoticInfo.Reset();
                stNoticInfo.m_dwNoticId = EN_NOTI_ID__ATTACK_SUCC;
                stNoticInfo.m_strTName = pstSession->m_stTargetUser.m_tbPlayer.m_sUin;
                stNoticInfo.m_strLang = CDocument::GetLang(pstSUser->m_tbLogin.m_nLang);;
                CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), ptbSPlayer->m_nSid, ptbSPlayer->m_nUid, stNoticInfo);
                stNoticInfo.Reset();
                stNoticInfo.m_dwNoticId = EN_NOTI_ID__DEFENSE_FAIL;
                stNoticInfo.m_strSName = ptbSPlayer->m_sUin;
                stNoticInfo.m_strLang = CDocument::GetLang(pstSession->m_stTargetUser.m_tbLogin.m_nLang);;
                CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), pstSession->m_stTargetUser.m_tbPlayer.m_nSid, pstSession->m_stTargetUser.m_tbPlayer.m_nUid, stNoticInfo);
            }
            else
            {
                stNoticInfo.Reset();
                stNoticInfo.m_dwNoticId = EN_NOTI_ID__ATTACK_FAIL;
                stNoticInfo.m_strTName = pstSession->m_stTargetUser.m_tbPlayer.m_sUin;
                stNoticInfo.m_strLang = CDocument::GetLang(pstSUser->m_tbLogin.m_nLang);
                CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), ptbSPlayer->m_nSid, ptbSPlayer->m_nUid, stNoticInfo);
                stNoticInfo.Reset();
                stNoticInfo.m_dwNoticId = EN_NOTI_ID__DEFENSE_SUCC;
                stNoticInfo.m_strSName = ptbSPlayer->m_sUin;
                stNoticInfo.m_strLang = CDocument::GetLang(pstSession->m_stTargetUser.m_tbLogin.m_nLang);
                CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), pstSession->m_stTargetUser.m_tbPlayer.m_nSid, pstSession->m_stTargetUser.m_tbPlayer.m_nUid, stNoticInfo);
            }

            CMsgBase::SendEncourageMail(&pstSession->m_stTargetUser.m_tbUserStat, pstSession->m_udwReqSvrId, EN_MAIL_ID__FIRST_BE_ATK);
        }

        SNoticInfo stNoticInfo;
        if (dwReportResult == EN_REPORT_RESULT_WIN)
        {
            if (pstTUser->m_tbAlliance.m_nAid > 0)
            {
                stNoticInfo.Reset();
                stNoticInfo.m_dwNoticId = EN_NOTI_ID__THRONE_BE_ROBED;
                stNoticInfo.m_strLang = "english";
                CMsgBase::SendNotificationAlliance(CConfBase::GetString("tbxml_project"),
                    pstTUser->m_tbAlliance.m_nSid, pstTUser->m_tbAlliance.m_nOid,
                    pstTUser->m_tbAlliance.m_nAid, stNoticInfo);
            }

            stNoticInfo.Reset();
            stNoticInfo.m_dwNoticId = EN_NOTI_ID__OCCUPY_THRONE;
            stNoticInfo.m_strLang = "english";
            CMsgBase::SendNotificationAlliance(CConfBase::GetString("tbxml_project"),
                pstSUser->m_tbAlliance.m_nSid, pstSUser->m_tbAlliance.m_nOid,
                pstSUser->m_tbAlliance.m_nAid, stNoticInfo);
        }

        if (stAttack.m_stDragon.m_ddwCaptured >= 0)
        {
            CMsgBase::SendEncourageMail(&pstSession->m_stTargetUser.m_tbUserStat, pstSession->m_udwReqSvrId, EN_MAIL_ID__FIRST_CAPTURE_HERO);
        }
        else if (stDefend.m_stDragon.m_ddwCaptured >= 0)
        {
            CMsgBase::SendEncourageMail(&pstSession->m_stSourceUser.m_tbUserStat, pstSession->m_udwReqSvrId, EN_MAIL_ID__FIRST_CAPTURE_HERO);
        }
    }
    else
    {
        CProcessReport::GenInValidReport(pstSession, EN_REPORT_TYPE_THRONE_ATTACK, dwReason, &pstSession->m_tbReport);

        ptbReqMarch->Set_Status(EN_MARCH_STATUS__RETURNING);
        ptbReqMarch->Set_Btime(ptbReqMarch->m_nEtime);
        ptbReqMarch->Set_Ctime(pstParam->m_ddwMarchingTime);
        ptbReqMarch->Set_Etime(ptbReqMarch->m_nBtime + pstParam->m_ddwMarchingTime);
        ptbReqMarch->Set_Tal(0, UPDATE_ACTION_TYPE_DELETE);
    }

    TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessMarchThroneFighting: suid[%lld] tuid[%lld] wild_type[%ld] [user_status=%u wild_id=%u wild_status=%u] [seq=%u]",
        ptbReqMarch->m_nSuid, ptbReqMarch->m_nTuid, ptbWild->m_nType, pstSession->m_stTargetUser.m_tbPlayer.m_nStatus, ptbWild->m_nId, ptbWild->m_nStatus, pstSession->m_udwSeqNo));

    // 更新action状态
    pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;

    return 0;
}

TINT32 CProcessAction::ProcessMarchThroneRallyFighting(SSession *pstSession)
{
    TbMarch_action *ptbRallyWar = &pstSession->m_stReqMarch;
    SActionMarchParam *pstParam = &ptbRallyWar->m_bParam[0];
    TbMap *ptbWild = &pstSession->m_stMapItem;
    TBOOL bSuccess = TRUE;
    TINT32 dwReason = EN_REPORT_RESULT_TARGET_CHANGE;
    SUserInfo *pstSUser = &pstSession->m_stSourceUser;
    SUserInfo *pstTUser = &pstSession->m_stTargetUser;
    TbDiplomacy *ptbDip = NULL;
    TbPlayer *ptbSPlayer = &pstSUser->m_tbPlayer;
    if (ptbWild->m_nAlid != 0 && ptbSPlayer->m_nAlpos != 0)
    {
        ptbDip = CCommonBase::GetDiplomacy(pstSession->m_stSourceUser.m_atbDiplomacy, pstSession->m_stSourceUser.m_udwDiplomacyNum,
            ptbWild->m_nAlid);
    }

    // 任务合法性检查
    if ((ptbSPlayer->m_nStatus & EN_CITY_STATUS__NEW_PROTECTION)
        || (ptbSPlayer->m_nStatus & EN_CITY_STATUS__AVOID_WAR))
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchThroneRallyFighting: srcPlayer[id=%ld] is in peace time, just return... [seq=%u]",
            ptbSPlayer->m_nUid, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_SELF_PEACETIME;
    }

    if (bSuccess && (ptbWild->m_nType != EN_WILD_TYPE__THRONE_NEW || ptbWild->m_nId != pstSession->m_tbThrone.m_nPos))
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchThroneRallyFighting: rally war throne: wild_type[%ld], just return... [seq=%u]",
            ptbWild->m_nType, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_CHANGE;
    }

    if (bSuccess && ptbSPlayer->m_nAlpos != 0 && ptbSPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET == ptbWild->m_nAlid)
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchThroneRallyFighting: sal[%u]=tal[%u], just return... [seq=%u]",
            pstParam->m_ddwSourceAlliance, pstParam->m_ddwTargetAlliance, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_SAME_ALLIANCE;
    }

    if (bSuccess && ptbDip && ptbDip->m_nType == EN_DIPLOMACY_TYPE__FRIENDLY)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessMarchThroneRallyWar: sal[%u] and tal[%u] is friend, just return... [seq=%u]",
            ptbSPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET, ptbWild->m_nAlid, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_IS_FRIEND;
    }

    if (bSuccess && ptbWild->m_nStatus == EN_THRONE_STATUS__PEACE_TIME)
    {
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessMarchThroneRallyFighting: target[%ld] is in peace time, just return... [seq=%u]",
            ptbWild->m_nId, pstSession->m_udwSeqNo));
        bSuccess = FALSE;
        dwReason = EN_REPORT_RESULT_TARGET_PEACETIME;
    }

    //     if (bSuccess)
    //     {
    //         TUINT32 udwFightTime = 5;
    // 
    //         ptbRallyWar->Set_Status(EN_MARCH_STATUS__FIGHTING);
    //         ptbRallyWar->Set_Btime(ptbRallyWar->m_nEtime);
    //         ptbRallyWar->Set_Ctime(udwFightTime);
    //         ptbRallyWar->Set_Etime(ptbRallyWar->m_nBtime + udwFightTime);
    //     }
    //     else
    //     {
    //         CProcessReport::GenInValidReport(pstSession, EN_REPORT_TYPE_THRONE_RALLY_WAR, dwReason, &pstSession->m_tbReport);
    // 
    //         ptbRallyWar->Set_Status(EN_MARCH_STATUS__RETURNING);
    //         ptbRallyWar->Set_Btime(ptbRallyWar->m_nEtime);
    //         ptbRallyWar->Set_Etime(ptbRallyWar->m_nBtime + ptbRallyWar->m_nCtime);
    //         //ptbRallyWar->DeleteField(TbMARCH_ACTION_FIELD_TAL);
    //         //ptbRallyWar->DeleteField(TbMARCH_ACTION_FIELD_SAL);
    //         ptbRallyWar->Set_Sal(0, UPDATE_ACTION_TYPE_DELETE);
    //         ptbRallyWar->Set_Tal(0, UPDATE_ACTION_TYPE_DELETE);
    // 
    //         for (TUINT32 udwIdx = 0; udwIdx < pstSUser->m_udwMarchNum; ++udwIdx)
    //         {
    //             TbMarch_action* ptbRallyReinforce = &pstSUser->m_atbMarch[udwIdx];
    //             if (ptbRallyReinforce->m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_THRONE
    //                 && ptbRallyReinforce->m_nStatus == EN_MARCH_STATUS__PREPARING
    //                 && ptbRallyReinforce->m_nTid == ptbRallyWar->m_nId)
    //             {
    //                 CActionBase::ReturnMarch(ptbRallyReinforce, ptbRallyWar->m_nTpos, ptbRallyWar->m_bParam[0].m_ddwTargetType);
    //                 // 添加盟友idlist，用于发送report
    //                 CToolBase::AddUserToMailReceiverList(pstSUser->m_adwMailSendUidList, pstSUser->m_udwMailSendNum, ptbRallyReinforce->m_nSuid);
    //             }
    //         }
    //     }
    // 
    //     // 更新action状态
    //     pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;
    //     return 0;

    if (bSuccess)
    {
        SBattleNode stDefend;
        SBattleNode stAttack;

        if (pstTUser->m_tbAlliance.m_nAid > 0)
        {
            ptbRallyWar->m_bParam[0].m_ddwTargetUserId = pstTUser->m_tbAlliance.m_nOid;
            strncpy(ptbRallyWar->m_bParam[0].m_szTargetUserName, pstTUser->m_tbAlliance.m_sOname.c_str(), MAX_TABLE_NAME_LEN);
            ptbRallyWar->m_bParam[0].m_szTargetUserName[MAX_TABLE_NAME_LEN - 1] = 0;
            strncpy(ptbRallyWar->m_bParam[0].m_szTargetAlliance, pstTUser->m_tbAlliance.m_sName.c_str(), MAX_TABLE_NAME_LEN);
            ptbRallyWar->m_bParam[0].m_szTargetAlliance[MAX_TABLE_NAME_LEN - 1] = 0;
            strncpy(ptbRallyWar->m_bParam[0].m_szTargetAlNick, pstTUser->m_tbAlliance.m_sAl_nick_name.c_str(), MAX_TABLE_NAME_LEN);
            ptbRallyWar->m_bParam[0].m_szTargetAlNick[MAX_TABLE_NAME_LEN - 1] = 0;
        }
        ptbRallyWar->m_bParam[0].m_ddwSourceAlliance = pstSUser->m_tbAlliance.m_nAid;

        pstSession->m_dwMarchResult = CWarProcess::ProcessWar(pstSession, &stAttack, &stDefend, &pstSession->m_tbReport);

        //player info 统计 不统计支援类action的兵力情况
        CWarProcess::ComputeWarResult(pstSession, &stAttack, &stDefend, pstSession->m_dwMarchResult, 0);

        TINT32 dwReportResult = pstSession->m_dwMarchResult;

        CProcessReport::GenAttackReport(pstSession, EN_REPORT_TYPE_THRONE_RALLY_WAR, dwReportResult, &stAttack, &stDefend, &pstSession->m_tbReport);

        if (pstSession->m_dwMarchResult == EN_REPORT_RESULT_WIN)
        {
            CSendMessageBase::SendBroadcast(pstSUser, ptbRallyWar->m_nSid, 0, EN_BROADCAST_CONTENT_ID__BECOME_KING, pstSUser->m_tbAlliance.m_sOname);
        }

        //Tips
        TUINT32 udwWinOrLose = pstSession->m_dwMarchResult == EN_REPORT_RESULT_WIN ? 0 : 1;

        //给发起者tips
        CSendMessageBase::AddTips(&pstSession->m_stSourceUser, EN_TIPS_TYPE__WAR_RESULT, ptbSPlayer->m_nUid, TRUE,
            ptbWild->m_nId, ptbWild->m_nUid, udwWinOrLose, pstTUser->m_tbPlayer.m_sUin.c_str(), "0|0|0|0|0|0");

        CMsgBase::SendEncourageMail(&pstSUser->m_tbUserStat, pstSUser->m_tbPlayer.m_nSid, EN_MAIL_ID__FIRST_RALLY_WAR_FINISH);

        if (pstTUser->m_tbAlliance.m_nAid > 0)
        {
            //防守方打输了 也要弹tips
            TUINT32 udwTargetWinOrLose = pstSession->m_dwMarchResult == EN_REPORT_RESULT_LOSE ? 2 : 4;
            //给防守者tips
            CSendMessageBase::AddTips(&pstSession->m_stTargetUser, EN_TIPS_TYPE__WAR_RESULT, pstTUser->m_tbPlayer.m_nUid, TRUE,
                ptbWild->m_nId, ptbSPlayer->m_nUid, udwTargetWinOrLose,
                ptbSPlayer->m_sUin.c_str(), "0|0|0|0|0|0");

            SNoticInfo stNoticInfo;
            if (pstSession->m_dwMarchResult == EN_REPORT_RESULT_WIN)
            {
                stNoticInfo.Reset();
                stNoticInfo.m_dwNoticId = EN_NOTI_ID__RALLY_ATTACK_SUCC;
                stNoticInfo.m_strTName = pstSession->m_stTargetUser.m_tbPlayer.m_sUin;
                stNoticInfo.m_strLang = "english";
                for (TUINT32 udwIdx = 0; udwIdx < pstSUser->m_udwMailSendNum; udwIdx++)
                {
                    CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), ptbSPlayer->m_nSid, pstSUser->m_adwMailSendUidList[udwIdx], stNoticInfo);
                }

                stNoticInfo.Reset();
                stNoticInfo.m_dwNoticId = EN_NOTI_ID__RALLY_DEFENSE_FAIL;
                stNoticInfo.m_strTName = ptbSPlayer->m_sUin;
                stNoticInfo.m_strLang = "english";
                CMsgBase::SendNotificationAlliance(CConfBase::GetString("project"),
                    pstSession->m_stTargetUser.m_tbPlayer.m_nSid,
                    pstSession->m_stTargetUser.m_tbPlayer.m_nUid,
                    pstSession->m_stTargetUser.m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET,
                    stNoticInfo);
            }
            else
            {
                stNoticInfo.Reset();
                stNoticInfo.m_dwNoticId = EN_NOTI_ID__RALLY_ATTACK_FAIL;
                stNoticInfo.m_strTName = pstSession->m_stTargetUser.m_tbPlayer.m_sUin;
                stNoticInfo.m_strLang = "english";
                for (TUINT32 udwIdx = 0; udwIdx < pstSUser->m_udwMailSendNum; udwIdx++)
                {
                    CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), ptbSPlayer->m_nSid, pstSUser->m_adwMailSendUidList[udwIdx], stNoticInfo);
                }

                stNoticInfo.Reset();
                stNoticInfo.m_dwNoticId = EN_NOTI_ID__RALLY_DEFENSE_SUCC;
                stNoticInfo.m_strTName = ptbSPlayer->m_sUin;
                stNoticInfo.m_strLang = "english";
                CMsgBase::SendNotificationAlliance(CConfBase::GetString("project"),
                    pstSession->m_stTargetUser.m_tbPlayer.m_nSid,
                    pstSession->m_stTargetUser.m_tbPlayer.m_nUid,
                    pstSession->m_stTargetUser.m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET,
                    stNoticInfo);
            }
        }

        SNoticInfo stNoticInfo;
        if (dwReportResult == EN_REPORT_RESULT_WIN)
        {
            if (pstTUser->m_tbAlliance.m_nAid > 0)
            {
                stNoticInfo.Reset();
                stNoticInfo.m_dwNoticId = EN_NOTI_ID__THRONE_BE_ROBED;
                stNoticInfo.m_strLang = "english";
                CMsgBase::SendNotificationAlliance(CConfBase::GetString("tbxml_project"),
                    pstTUser->m_tbAlliance.m_nSid, pstTUser->m_tbAlliance.m_nOid,
                    pstTUser->m_tbAlliance.m_nAid, stNoticInfo);
            }

            stNoticInfo.Reset();
            stNoticInfo.m_dwNoticId = EN_NOTI_ID__OCCUPY_THRONE;
            stNoticInfo.m_strLang = "english";
            CMsgBase::SendNotificationAlliance(CConfBase::GetString("tbxml_project"),
                pstSUser->m_tbAlliance.m_nSid, pstSUser->m_tbAlliance.m_nOid,
                pstSUser->m_tbAlliance.m_nAid, stNoticInfo);
        }

        CMsgBase::SendEncourageMail(&pstSUser->m_tbUserStat, pstSUser->m_tbPlayer.m_nSid, EN_MAIL_ID__FIRST_RALLY_WAR_FINISH);
        if (stAttack.m_stDragon.m_ddwCaptured >= 0)
        {
            CMsgBase::SendEncourageMail(&pstSession->m_stTargetUser.m_tbUserStat, pstSession->m_udwReqSvrId, EN_MAIL_ID__FIRST_CAPTURE_HERO);
        }
        else if (stDefend.m_stDragon.m_ddwCaptured >= 0)
        {
            CMsgBase::SendEncourageMail(&pstSession->m_stSourceUser.m_tbUserStat, pstSession->m_udwReqSvrId, EN_MAIL_ID__FIRST_CAPTURE_HERO);
        }
    }
    else
    {
        CProcessReport::GenInValidReport(pstSession, EN_REPORT_TYPE_THRONE_RALLY_WAR, dwReason, &pstSession->m_tbReport);

        ptbRallyWar->Set_Status(EN_MARCH_STATUS__RETURNING);
        ptbRallyWar->Set_Btime(ptbRallyWar->m_nEtime);
        ptbRallyWar->Set_Ctime(pstParam->m_ddwMarchingTime);
        ptbRallyWar->Set_Etime(ptbRallyWar->m_nBtime + ptbRallyWar->m_nCtime);
        ptbRallyWar->Set_Sal(0, UPDATE_ACTION_TYPE_DELETE);
        ptbRallyWar->Set_Tal(0, UPDATE_ACTION_TYPE_DELETE);

        for (TUINT32 udwIdx = 0; udwIdx < pstSUser->m_udwMarchNum; ++udwIdx)
        {
            TbMarch_action* ptbRallyReinforce = &pstSUser->m_atbMarch[udwIdx];
            if (ptbRallyReinforce->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_REINFORCE
                && ptbRallyReinforce->m_nStatus == EN_MARCH_STATUS__PREPARING
                && ptbRallyReinforce->m_nTid == ptbRallyWar->m_nId)
            {
                CActionBase::ReturnMarch(ptbRallyReinforce, ptbRallyWar->m_nTpos, ptbRallyWar->m_bParam[0].m_ddwTargetType);
                // 添加盟友idlist，用于发送report
                CToolBase::AddUserToMailReceiverList(pstSUser->m_adwMailSendUidList, pstSUser->m_udwMailSendNum, ptbRallyReinforce->m_nSuid);
            }
        }
    }

    // 更新action状态
    pstSession->m_ucReqMarchFlag = EN_TABLE_UPDT_FLAG__CHANGE;
    return 0;
}