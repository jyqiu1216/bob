#include "process_march.h"
#include "common_func.h"
#include "game_info.h"
#include "common_base.h"
#include "procedure_base.h"
#include "process_diplomacy.h"
#include "sendmessage_base.h"
#include "buffer_base.h"
#include "msg_base.h"
#include "wild_info.h"
#include "common_logic.h"
#include "conf_base.h"
#include "report_logic.h"
#include "city_base.h"
#include "item_base.h"
#include "action_base.h"
#include "tool_base.h"
#include "player_base.h"
#include "map_logic.h"
#include "game_info.h"
#include "report_svr_request.h"
#include "pushdata_action.h"
#include "item_logic.h"
#include "activities_logic.h"
#include "quest_logic.h"

TINT32 CProcessMarch::ProcessCmd_MarchScout(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;
    TbMap *ptbTmpMap = &pstSession->m_tbTmpMap;

    // 0. 输入参数
    TUINT32 udwCostTime = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TUINT32 udwTargetPos = strtoul(pstSession->m_stReqParam.m_szKey[1], NULL, 10);
    TUINT32 udwCostGem = strtoul(pstSession->m_stReqParam.m_szKey[2], NULL, 10);
    TCHAR *pszResource = pstSession->m_stReqParam.m_szKey[3];

    // 1. check param
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        dwRetCode = CheckMarchSelfCondition(pstUser, ptbPlayer, pstCity, "", "", pszResource, udwCostGem, -1, FALSE, TRUE, EN_ACTION_SEC_CLASS__SCOUT);
        if(dwRetCode != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = dwRetCode;
            return -1;
        }
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
    }

    // 2. 获取目标的所属信息——发送请求
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        // get data
        if(0 == udwTargetPos)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MarchScout: get map info failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }
        pstSession->ResetAwsReq();
        CAwsRequest::MapGet(pstSession, udwTargetPos);

        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MarchScout: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }

        return 0;
    }

    // 3. 解析响应，生成相应的action
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;

        // parse city info
        ptbTmpMap->Reset();
        dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[0], ptbTmpMap);
        if(dwRetCode <= 0 || ptbTmpMap->m_nId != udwTargetPos)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MarchScout: get map info failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -4;
        }

        // check param
        dwRetCode = CheckMarchOtherCondition(pstUser, ptbPlayer, ptbTmpMap, EN_ACTION_SEC_CLASS__SCOUT);
        if(dwRetCode != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = dwRetCode;
            return -5;
        }
    }

    // 4. 进行计算
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        // get action param
        GenMarchAction(pstUser, ptbPlayer, pstCity, "", "", pszResource, -1, FALSE, ptbTmpMap, EN_ACTION_SEC_CLASS__SCOUT, udwCostTime);

        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }
    return 0;
}

TINT32 CProcessMarch::ProcessCmd_MarchAttack(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;
    TbMap *ptbTmpMap = &pstSession->m_tbTmpMap;

    // 0. 输入参数
    TUINT32 udwCostTime = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TUINT32 udwTargetPos = strtoul(pstSession->m_stReqParam.m_szKey[1], NULL, 10);
    TINT32	dwKnightId = atoi(pstSession->m_stReqParam.m_szKey[2]) % MAX_KNIGHT_NUM;
    TCHAR *pszTroop = pstSession->m_stReqParam.m_szKey[3];
    TBOOL bIsDragonJoin = atoi(pstSession->m_stReqParam.m_szKey[4]);

    // 1. check param
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        TUINT32 udwTroopNum = EN_TROOP_TYPE__END;
        SCommonTroop stTroop;
        stTroop.Reset();
        CCommonFunc::GetArrayFromString(pszTroop, ':', stTroop.m_addwNum, udwTroopNum);
        SPlayerBuffInfo *pstBuff = &pstUser->m_stPlayerBuffList;
        if (bIsDragonJoin)
        {
            pstBuff = &pstUser->m_stPlayerBuffList;
        }
        else
        {
            pstBuff = &pstUser->m_stBuffWithoutDragon;
        }
        TINT32 dwRealTime = CCommonBase::GetTroopMarchTime(&stTroop, pstSession->m_stReqParam.m_udwCityId, udwTargetPos, pstBuff);
        if (!CCommonBase::IsCorrectTime(udwCostTime, dwRealTime))
        {
            TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("ProcessCmd_MarchAttack: time_cost is incorrect [given_time:%u calc_time:%d] [seq=%u]",
                udwCostTime, dwRealTime, pstUser->m_udwBSeqNo));
            if (pstSession->m_bIsNeedCheck)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__BLACK_ACCOUNT;
                return -1;
            }
        }

        dwRetCode = CheckMarchSelfCondition(pstUser, ptbPlayer, pstCity, pszTroop, "", "", 0, dwKnightId, bIsDragonJoin,
            TRUE, EN_ACTION_SEC_CLASS__ATTACK);
        if(dwRetCode != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = dwRetCode;
            return -1;
        }
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
    }

    // 2. 获取目标的所属信息——发送请求
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        // get data
        if(0 == udwTargetPos)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MarchAttack: get map info failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }
        pstSession->ResetAwsReq();
        CAwsRequest::MapGet(pstSession, udwTargetPos);

        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MarchAttack: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }

        return 0;
    }

    // 3. 解析响应，生成相应的action
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;

        // parse city info
        ptbTmpMap->Reset();
        dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[0], ptbTmpMap);
        if(dwRetCode < 0 || ptbTmpMap->m_nId != udwTargetPos)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MarchAttack: get map info failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -4;
        }

        // check param
        dwRetCode = CheckMarchOtherCondition(pstUser, ptbPlayer, ptbTmpMap, EN_ACTION_SEC_CLASS__ATTACK);
        if(dwRetCode != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = dwRetCode;
            return -5;
        }
    }

    // 4. 进行计算
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        SNoticInfo stNoticInfo;
        stNoticInfo.Reset();
        if(ptbTmpMap->m_nType == EN_WILD_TYPE__CITY)
        {
            stNoticInfo.SetValue(EN_NOTI_ID__COMING_ATTACK_7, ptbPlayer->m_sUin, "", 0, 0, 0, 0, udwCostTime, "", 0);
        }
        else
        {
            stNoticInfo.SetValue(EN_NOTI_ID__OCCUPY_COMING_ATTACK_7, ptbPlayer->m_sUin, "", 0, 0, 0, 0, udwCostTime, "", 0);
        }
        CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), pstSession->m_stReqParam.m_udwSvrId, ptbTmpMap->m_nUid, stNoticInfo);
        GenMarchAction(pstUser, ptbPlayer, pstCity, pszTroop, "", "", dwKnightId, bIsDragonJoin, ptbTmpMap, EN_ACTION_SEC_CLASS__ATTACK, udwCostTime);
        // next procedure
        bNeedResponse = FALSE;
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        
        return 0;
    }

    return 0;
}

TINT32 CProcessMarch::ProcessCmd_MarchTransport(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;
    TbMap *ptbTmpMap = &pstSession->m_tbTmpMap;

    // 0. 输入参数
    TUINT32 udwCostTime = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TUINT32 udwTargetPos = strtoul(pstSession->m_stReqParam.m_szKey[1], NULL, 10);
    TCHAR *szSendResource = pstSession->m_stReqParam.m_szKey[2];
    TCHAR *szTaxResource = pstSession->m_stReqParam.m_szKey[3];
    

    // 1. check param
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        TINT32 dwMarketLevel = CCityBase::GetBuildingLevelByFuncType(pstCity, EN_BUILDING_TYPE__MARKETPLACE);
        if(dwMarketLevel <= 0)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MarchTransport: have no market[lv=%d] [seq=%u]",
                dwMarketLevel, pstSession->m_stUserInfo.m_udwBSeqNo));
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__MARKET_NOT_EXIST;
            return -1;
        }

        TINT32 dwRealTime = CCommonBase::GetTroopTransportTime(pstSession->m_stReqParam.m_udwCityId, udwTargetPos, &pstUser->m_stPlayerBuffList);
        if (!CCommonBase::IsCorrectTime(udwCostTime, dwRealTime))
        {
            TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("ProcessCmd_MarchTransport: time_cost is incorrect [given_time:%u calc_time:%d] [seq=%u]",
                udwCostTime, dwRealTime, pstUser->m_udwBSeqNo));
            if (pstSession->m_bIsNeedCheck)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__BLACK_ACCOUNT;
                return -1;
            }
        }

        dwRetCode = CheckMarchSelfCondition(pstUser, ptbPlayer, pstCity, "", szSendResource, szTaxResource, 0, -1, FALSE,
            FALSE, EN_ACTION_SEC_CLASS__TRANSPORT);
        if(dwRetCode != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = dwRetCode;
            return -2;
        }

        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
    }

    // 2. 获取目标的所属信息——发送请求
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        // get data
        if(0 == udwTargetPos)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MarchTransport: get map info failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }
        pstSession->ResetAwsReq();
        CAwsRequest::MapGet(pstSession, udwTargetPos);

        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MarchTransport: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -4;
        }

        return 0;
    }

    // 3. 解析响应，生成相应的action
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;

        // parse city info
        ptbTmpMap->Reset();
        dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[0], ptbTmpMap);
        if(dwRetCode < 0 || ptbTmpMap->m_nId != udwTargetPos)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MarchTransport: get map info failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -5;
        }

        // check param
        dwRetCode = CheckMarchOtherCondition(pstUser, ptbPlayer, ptbTmpMap, EN_ACTION_SEC_CLASS__TRANSPORT);
        if(dwRetCode != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = dwRetCode;
            return -6;
        }
    }

    // 4. 进行计算
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        // get action param
        GenMarchAction(pstUser, ptbPlayer, pstCity, "", szSendResource, szTaxResource, -1, FALSE, ptbTmpMap, EN_ACTION_SEC_CLASS__TRANSPORT, udwCostTime);

        // next procedure
        bNeedResponse = FALSE;
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    return 0;
}

TINT32 CProcessMarch::ProcessCmd_MarchReinforceNormal(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;
    TbMap *ptbTmpMap = &pstSession->m_tbTmpMap;

    // 0. 输入参数
    TUINT32 udwCostTime = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TUINT32 udwTargetPos = strtoul(pstSession->m_stReqParam.m_szKey[1], NULL, 10);
    TINT32 dwKnightId = atoi(pstSession->m_stReqParam.m_szKey[2]) % MAX_KNIGHT_NUM;
    TCHAR *pszTroop = pstSession->m_stReqParam.m_szKey[3];

    // 1. check param
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        TINT32 dwLevel = CCityBase::GetBuildingLevelByFuncType(pstCity, EN_BUILDING_TYPE__EMBASSY);
        if(dwLevel <= 0)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MarchReinforceNormal: you have no consulate [lv=%d] [seq=%u]",
                dwLevel, pstSession->m_stUserInfo.m_udwBSeqNo));
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__CONSULATE_NOT_EXIST;
            return -6;
        }

        TUINT32 udwTroopNum = EN_TROOP_TYPE__END;
        SCommonTroop stTroop;
        stTroop.Reset();
        CCommonFunc::GetArrayFromString(pszTroop, ':', stTroop.m_addwNum, udwTroopNum);

        TINT32 dwRealTime = CCommonBase::GetTroopReinforceTime(&stTroop, pstSession->m_stReqParam.m_udwCityId, udwTargetPos, &pstUser->m_stPlayerBuffList);
        if (!CCommonBase::IsCorrectTime(udwCostTime, dwRealTime))
        {
            TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("ProcessCmd_MarchReinforceNormal: time_cost is incorrect [given_time:%u calc_time:%d] [seq=%u]",
                udwCostTime, dwRealTime, pstUser->m_udwBSeqNo));
            if (pstSession->m_bIsNeedCheck)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__BLACK_ACCOUNT;
                return -1;
            }
        }

        dwRetCode = CheckMarchSelfCondition(pstUser, ptbPlayer, pstCity, pszTroop, "", "", 0, dwKnightId, FALSE,
            TRUE, EN_ACTION_SEC_CLASS__REINFORCE_NORMAL);
        if(dwRetCode != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = dwRetCode;
            return -1;
        }

        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
    }

    // 2. 获取目标的所属信息——发送请求
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        // get data
        if(0 == udwTargetPos)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MarchReinforceNormal: get map info failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }
        pstSession->ResetAwsReq();
        CAwsRequest::MapGet(pstSession, udwTargetPos);

        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MarchReinforceNormal: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }

        return 0;
    }

    // 3. 解析响应，生成相应的action
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;

        // parse city info
        ptbTmpMap->Reset();
        dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[0], ptbTmpMap);
        if(dwRetCode < 0 || ptbTmpMap->m_nId != udwTargetPos)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MarchReinforceNormal: get map info failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -4;
        }

        // check param
        dwRetCode = CheckMarchOtherCondition(pstUser, ptbPlayer, ptbTmpMap, EN_ACTION_SEC_CLASS__REINFORCE_NORMAL);
        if(dwRetCode != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = dwRetCode;
            return -5;
        }

        TUINT32 udwTroopNum = EN_TROOP_TYPE__END;
        TINT64 addwTroop[EN_TROOP_TYPE__END] = { 0 };
        CCommonFunc::GetArrayFromString(pszTroop, ':', addwTroop, udwTroopNum);
        TINT32 dwTroopNum = CToolBase::GetTroopSumNum(addwTroop);
        if (ptbTmpMap->m_jCity_info["reinforced_limit"].asInt() <= ptbTmpMap->m_jCity_info["reinforced_num"].asInt()
            || ptbTmpMap->m_jCity_info["reinforced_troop_limit"].asInt() < ptbTmpMap->m_jCity_info["reinforced_troop_num"].asInt() + dwTroopNum)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REINFORCE_ARRIVE_LIMIT;
            return -6;
        }
    }

    // 4. 进行计算
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        GenMarchAction(pstUser, ptbPlayer, pstCity, pszTroop, "", "", dwKnightId, FALSE, ptbTmpMap, EN_ACTION_SEC_CLASS__REINFORCE_NORMAL, udwCostTime);

        CMsgBase::RefreshUserInfo(ptbTmpMap->m_nUid);
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        
        return 0;
    }

    return 0;
}

TINT32 CProcessMarch::ProcessCmd_Recall(SSession *pstSession, TBOOL &bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TINT32 dwActionIdx = -1;
    TbMarch_action *ptbMarch = NULL;
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();

    // 0. 请求参数
    TINT64 uddwActionId = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TINT32 dwItemId = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TINT32 dwGem = atoi(pstSession->m_stReqParam.m_szKey[2]);

    dwActionIdx = CActionBase::GetMarchIndex(pstUser->m_atbMarch, pstUser->m_udwMarchNum, uddwActionId);
    if(dwActionIdx < 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__Recall: march id invalid [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }
    ptbMarch = &pstUser->m_atbMarch[dwActionIdx];

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        if (ptbMarch->m_nMclass == EN_ACTION_MAIN_CLASS__MARCH
            && ptbMarch->m_nStatus == EN_MARCH_STATUS__RETURNING)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__Recall: action[%ld] has recall, return succ.. [seq=%u]",
                ptbMarch->m_nId, pstSession->m_stUserInfo.m_udwBSeqNo));
            return 0;
        }

        if(dwItemId > 0 && FALSE == CItemBase::HasEnoughItem(&pstUser->m_tbBackpack, dwItemId, 1))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ITEM_NOT_ENOUGH;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_Recall: item not enough [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }
        if(dwItemId > 0)
        {
            CItemBase::CostItem(&pstUser->m_tbBackpack, dwItemId, 1);
        }

        if(dwGem > 0 && FALSE == CPlayerBase::HasEnoughGem(&pstUser->m_tbLogin, dwGem))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GEM_LACK;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_Recall: gem not enough gem_own=%ld price=%d [seq=%u]", pstUser->m_tbLogin.Get_Gem(), dwGem, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }
        if(dwGem > 0)
        {
            CPlayerBase::CostGem(pstUser, dwGem);
        }
        
        if (ptbMarch->m_nStatus == EN_MARCH_STATUS__MARCHING)
        {
            CActionBase::ReturnMarchOnFly(ptbMarch);
            pstUser->m_aucMarchFlag[dwActionIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
        }
        else if (ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_NORMAL
            && ptbMarch->m_nStatus == EN_MARCH_STATUS__DEFENDING)
        {
            CActionBase::ReturnMarch(ptbMarch);
            pstUser->m_aucMarchFlag[dwActionIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
        }
        else if (ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__OCCUPY
            && ptbMarch->m_nStatus == EN_MARCH_STATUS__LOADING)
        {
            // 更新action信息
            ptbMarch->Set_Etime(udwCurTime);
            ptbMarch->Set_Is_recalled(TRUE);
            pstUser->m_aucMarchFlag[dwActionIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
        }
        else if (ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__CAMP &&
            (ptbMarch->m_nStatus == EN_MARCH_STATUS__CAMPING_WITH_PEACETIME 
            || ptbMarch->m_nStatus == EN_MARCH_STATUS__CAMPING_NORMAL))
        {
            // 更新action信息
            ptbMarch->Set_Etime(udwCurTime);
            ptbMarch->Set_Is_recalled(TRUE);
            pstUser->m_aucMarchFlag[dwActionIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
        }
        else
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -4;
        }

        if (ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_NORMAL)
        {
            CMsgBase::RefreshUserInfo(ptbMarch->m_nTuid);
        }
        else if(ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_REINFORCE)
        {
            TINT64 ddwRallyWarId = ptbMarch->m_nTid;

            TINT32 dwRallyWarIndex = -1;
            TbMarch_action* ptbRallyWar = NULL;
            dwRallyWarIndex = CActionBase::GetMarchIndex(pstUser->m_atbMarch, pstUser->m_udwMarchNum, ddwRallyWarId);
            
            if(dwRallyWarIndex >= 0)
            {
                ptbRallyWar = &pstUser->m_atbMarch[dwRallyWarIndex];
                CActionBase::ReleaseSlot(ptbRallyWar, ptbMarch);
                CActionBase::UpdateRallyForce(ptbRallyWar, ptbMarch, TRUE);

                if (ptbRallyWar->m_nStatus == EN_MARCH_STATUS__WAITING)
                {
                    ptbRallyWar->Set_Etime(CTimeUtils::GetUnixTime());
                }
                pstUser->m_aucMarchFlag[dwRallyWarIndex] = EN_TABLE_UPDT_FLAG__CHANGE;
            }
        }

        //wave@push_data: 对于盟友和目标来说，召回就意味着删除,因为删除了sal字段
        pstUser->m_ptbPushMarchAction = &pstUser->m_atbMarch[dwActionIdx];
        pstUser->dwPushMarchActionType = EN_TABLE_UPDT_FLAG__DEL;

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessMarch::ProcessCmd_MarchOccupy(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;
    TbMap *ptbTmpMap = &pstSession->m_tbTmpMap;

    // 0. 输入参数
    TUINT32 udwCostTime = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TUINT32 udwTargetPos = strtoul(pstSession->m_stReqParam.m_szKey[1], NULL, 10);
    TINT32	dwKnightId = atoi(pstSession->m_stReqParam.m_szKey[2]) % MAX_KNIGHT_NUM;
    TCHAR *pszTroop = pstSession->m_stReqParam.m_szKey[3];
    TBOOL bIsDragonJoin = atoi(pstSession->m_stReqParam.m_szKey[4]);

    // 1. check param
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        TUINT32 udwTroopNum = EN_TROOP_TYPE__END;
        SCommonTroop stTroop;
        stTroop.Reset();
        CCommonFunc::GetArrayFromString(pszTroop, ':', stTroop.m_addwNum, udwTroopNum);
        SPlayerBuffInfo *pstBuff = &pstUser->m_stPlayerBuffList;
        if (bIsDragonJoin)
        {
            pstBuff = &pstUser->m_stPlayerBuffList;
        }
        else
        {
            pstBuff = &pstUser->m_stBuffWithoutDragon;
        }
        TINT32 dwRealTime = CCommonBase::GetTroopMarchTime(&stTroop, pstSession->m_stReqParam.m_udwCityId, udwTargetPos, pstBuff);
        if (!CCommonBase::IsCorrectTime(udwCostTime, dwRealTime))
        {
            TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("ProcessCmd_MarchOccupy: time_cost is incorrect [given_time:%u calc_time:%d] [seq=%u]",
                udwCostTime, dwRealTime, pstUser->m_udwBSeqNo));
            if (pstSession->m_bIsNeedCheck)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__BLACK_ACCOUNT;
                return -1;
            }
        }

        dwRetCode = CheckMarchSelfCondition(pstUser, ptbPlayer, pstCity, pszTroop, "", "", 0, dwKnightId, bIsDragonJoin,
            FALSE, EN_ACTION_SEC_CLASS__OCCUPY);
        if(dwRetCode != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = dwRetCode;
            return -1;
        }

        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
    }

    // 2. 获取目标的所属信息——发送请求
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        // get data
        if(0 == udwTargetPos)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MarchReinforceNormal: get map info failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }
        pstSession->ResetAwsReq();
        CAwsRequest::MapGet(pstSession, udwTargetPos);

        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_March: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }

        return 0;
    }

    // 3. 解析响应，生成相应的action
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;

        // parse city info
        ptbTmpMap->Reset();
        dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[0], ptbTmpMap);
        if(dwRetCode < 0 || ptbTmpMap->m_nId != udwTargetPos)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessReqCommand_March: get map info failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -4;
        }

        // check param
        dwRetCode = CheckMarchOtherCondition(pstUser, ptbPlayer, ptbTmpMap, EN_ACTION_SEC_CLASS__OCCUPY);
        if(dwRetCode != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = dwRetCode;
            return -5;
        }
    }

    // 4. 进行计算
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        GenMarchAction(pstUser, ptbPlayer, pstCity, pszTroop, "", "", dwKnightId, bIsDragonJoin, ptbTmpMap, EN_ACTION_SEC_CLASS__OCCUPY, udwCostTime);
        // next procedure
        bNeedResponse = FALSE;
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        
        return 0;
    }

    return 0;
}

TINT32 CProcessMarch::ProcessCmd_MarchDragonAttack(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;
    TbMap *ptbTmpMap = &pstSession->m_tbTmpMap;

    // 0. 输入参数
    TUINT32 udwCostTime = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TUINT32 udwTargetPos = strtoul(pstSession->m_stReqParam.m_szKey[1], NULL, 10);
    TBOOL bIsMaxAttack = atoi(pstSession->m_stReqParam.m_szKey[2]);

    // 1. check param
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {

        TINT32 dwBuff = pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_HERO_MARCH_TIME].m_ddwBuffTotal;
        TINT32 dwRealTime = CCommonBase::GetHeroMarchTime(pstSession->m_stReqParam.m_udwCityId, udwTargetPos, dwBuff);

        if (!CCommonBase::IsCorrectTime(udwCostTime, dwRealTime))
        {
            TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("ProcessCmd_MarchHeroAttack: time_cost is incorrect [buff:%d given_time:%u calc_time:%d] [seq=%u]",
                dwBuff, udwCostTime, dwRealTime, pstUser->m_udwBSeqNo));
            if (pstSession->m_bIsNeedCheck)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__BLACK_ACCOUNT;
                return -1;
            }
        }

        dwRetCode = CheckMarchSelfCondition(pstUser, ptbPlayer, pstCity, "", "", "", 0, -1, TRUE,
            FALSE, EN_ACTION_SEC_CLASS__DRAGON_ATTACK);
        if(dwRetCode != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = dwRetCode;
            return -1;
        }
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
    }

    // 2. 获取目标的所属信息——发送请求
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        // get data
        if(0 == udwTargetPos)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MarchHeroAttack: get map info failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }
        pstSession->ResetAwsReq();
        CAwsRequest::MapGet(pstSession, udwTargetPos);

        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MarchHeroAttack: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }

        return 0;
    }

    // 3. 解析响应，生成相应的action
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;

        // parse city info
        ptbTmpMap->Reset();
        dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[0], ptbTmpMap);
        if(dwRetCode < 0 || ptbTmpMap->m_nId != udwTargetPos)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MarchHeroAttack: get map info failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -4;
        }

        // check param
        dwRetCode = CheckMarchOtherCondition(pstUser, ptbPlayer, ptbTmpMap, EN_ACTION_SEC_CLASS__DRAGON_ATTACK);
        if(dwRetCode != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = dwRetCode;
            return -5;
        }
    }

    // 4. 进行计算
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        GenMarchAction(pstUser, ptbPlayer, pstCity, NULL, NULL, NULL, -1, TRUE, ptbTmpMap, EN_ACTION_SEC_CLASS__DRAGON_ATTACK, udwCostTime, bIsMaxAttack);
        // next procedure
        bNeedResponse = FALSE;
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    return 0;
}

TINT32 CProcessMarch::ProcessCmd_RallyAttack(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;
    TbMap *ptbTmpMap = &pstSession->m_tbTmpMap;

    // 0. 输入参数
    TUINT32 udwCostTime = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TUINT32 udwTargetPos = strtoul(pstSession->m_stReqParam.m_szKey[1], NULL, 10);
    TINT32 dwKnightId = atoi(pstSession->m_stReqParam.m_szKey[2]);
    TCHAR *pszTroop = pstSession->m_stReqParam.m_szKey[3];
    TUINT32 udwPrepareTime = strtoul(pstSession->m_stReqParam.m_szKey[4], NULL, 10);

    TBOOL bIsDragonJoin = TRUE;
    if (strlen(pstSession->m_stReqParam.m_szKey[5]) > 0)
    {
        bIsDragonJoin = atoi(pstSession->m_stReqParam.m_szKey[5]);
    }


    // 1. check param
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        TINT32 dwLevel = CCityBase::GetBuildingLevelByFuncType(pstCity, EN_BUILDING_TYPE__EMBASSY);
        if(dwLevel <= 0)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_RallyAttack: you have no consulate [lv=%d] [seq=%u]",
                dwLevel, pstSession->m_stUserInfo.m_udwBSeqNo));
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__CONSULATE_NOT_EXIST;
            return -1;
        }

        TUINT32 udwTroopNum = EN_TROOP_TYPE__END;
        SCommonTroop stTroop;
        stTroop.Reset();
        CCommonFunc::GetArrayFromString(pszTroop, ':', stTroop.m_addwNum, udwTroopNum);
        SPlayerBuffInfo *pstBuff = &pstUser->m_stPlayerBuffList;
        if (bIsDragonJoin)
        {
            pstBuff = &pstUser->m_stPlayerBuffList;
        }
        else
        {
            pstBuff = &pstUser->m_stBuffWithoutDragon;
        }
        TINT32 dwRealTime = CCommonBase::GetTroopRallyWarTime(&stTroop, pstSession->m_stReqParam.m_udwCityId, udwTargetPos, pstBuff);
        if (!CCommonBase::IsCorrectTime(udwCostTime, dwRealTime))
        {
            TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("ProcessCmd_RallyAttack: time_cost is incorrect [given_time:%u calc_time:%d] [seq=%u]",
                udwCostTime, dwRealTime, pstUser->m_udwBSeqNo));
            if (pstSession->m_bIsNeedCheck)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__BLACK_ACCOUNT;
                return -1;
            }
        }

        dwRetCode = CheckMarchSelfCondition(pstUser, ptbPlayer, pstCity, pszTroop, "", "", 0, dwKnightId, bIsDragonJoin,
            TRUE, EN_ACTION_SEC_CLASS__RALLY_WAR);
        if(dwRetCode != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = dwRetCode;
            return -1;
        }

        TINT32 dwCanRallyWar = CheckCanRallyWar(pstUser, udwTargetPos, pstSession->m_stReqParam.m_udwAllianceId);
        if(dwCanRallyWar != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = dwCanRallyWar;
            return -1;
        }

        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
    }

    // 2. 获取目标的所属信息——发送请求
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        // get data
        if(0 == udwTargetPos)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_RallyAttack: get map info failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }
        pstSession->ResetAwsReq();
        CAwsRequest::MapGet(pstSession, udwTargetPos);

        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_RallyAttack: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }

        return 0;
    }

    // 3. 解析响应，生成相应的action
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;

        // parse city info
        ptbTmpMap->Reset();
        dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[0], ptbTmpMap);
        if(dwRetCode < 0 || ptbTmpMap->m_nId != udwTargetPos)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_RallyAttack: get map info failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -4;
        }

        // check param
        dwRetCode = CheckMarchOtherCondition(pstUser, ptbPlayer, ptbTmpMap, EN_ACTION_SEC_CLASS__RALLY_WAR);
        if(dwRetCode != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = dwRetCode;
            return -5;
        }
    }

    // 4. 进行计算
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        string strTarget = ptbTmpMap->m_sUname;
        TINT32 dwType = ptbTmpMap->m_nType;
        if(ptbTmpMap->m_nUid != 0)
        {
            dwType = 0;
        }

        if(ptbTmpMap->m_nUid > 0)
        {
            //tips
            CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPY__BE_RALLY_WARED, ptbTmpMap->m_nUid, TRUE, 0, 0, 0, ptbPlayer->m_sUin.c_str());  //给目标
        }
        CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPY__RALLY_WAR_BEGIN_TO_ASSEMBLE, ptbPlayer->m_nUid, FALSE, dwType, 0, 0, strTarget.c_str());  //给发起者

        SNoticInfo stNoticInfo;
        stNoticInfo.Reset();
        stNoticInfo.SetValue(EN_NOTI_ID__RALLYWAR_START, ptbPlayer->m_sUin, strTarget, 0, 0, 0, 0, udwCostTime + udwPrepareTime, "", 0);
        CMsgBase::SendNotificationAlliance(CConfBase::GetString("tbxml_project"), pstSession->m_stReqParam.m_udwSvrId, ptbPlayer->m_nUid, ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET, stNoticInfo);

        if(ptbTmpMap->m_nUid > 0)
        {
            SNoticInfo stNoticInfo;
            stNoticInfo.Reset();
            stNoticInfo.SetValue(EN_NOTI_ID__RALLYWAR_DEFENSE, ptbTmpMap->m_sUname, ptbPlayer->m_sUin, 0, 0, 0, 0, udwCostTime + udwPrepareTime, "", 0);
            CMsgBase::SendNotificationAlliance(CConfBase::GetString("tbxml_project"), pstSession->m_stReqParam.m_udwSvrId, ptbPlayer->m_nUid, ptbTmpMap->m_nAlid, stNoticInfo);
        }

        TbMarch_action* ptbRallyWar = CProcessMarch::GenRallyAction(pstUser, ptbPlayer, pstCity, pszTroop, dwKnightId, bIsDragonJoin, ptbTmpMap, EN_ACTION_SEC_CLASS__RALLY_WAR, udwCostTime, udwPrepareTime);
        if(!ptbTmpMap->m_jCity_info.empty())
        {
            ptbRallyWar->m_bRally_def_force[0].ddwTotalNum = ptbTmpMap->m_jCity_info["troop_num"].asInt64();
            ptbRallyWar->m_bRally_def_force[0].ddwTotalForce = ptbTmpMap->m_jCity_info["troop_force"].asInt64();
            ptbRallyWar->m_bRally_def_force[0].ddwReinforceNum = ptbTmpMap->m_jCity_info["reinforced_troop_num"].asInt64();
            ptbRallyWar->m_bRally_def_force[0].ddwReinforceTroopLimit = ptbTmpMap->m_jCity_info["reinforced_troop_limit"].asInt64();
            ptbRallyWar->m_bRally_def_force[0].ddwReinforceForce = ptbTmpMap->m_jCity_info["reinforced_troop_force"].asInt64();
            ptbRallyWar->SetFlag(TbMARCH_ACTION_FIELD_RALLY_DEF_FORCE);
            ptbRallyWar->Set_City_info(ptbTmpMap->m_jCity_info);
        }
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_RallyAttack: gen rallywar time[%ld:%ld:%ld] prepare rally atk_slot[%u] def_slot[%u] [seq=%u]",
            ptbRallyWar->m_nBtime, ptbRallyWar->m_nCtime, ptbRallyWar->m_nEtime,
            ptbRallyWar->m_bRally_atk_slot.m_udwNum, ptbRallyWar->m_bRally_def_slot.m_udwNum, pstSession->m_stUserInfo.m_udwBSeqNo));

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    return 0;
}

TINT32 CProcessMarch::ProcessCmd_RallyReinforce(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;
    TbMap *ptbTmpMap = &pstSession->m_tbTmpMap;

    // 0. 输入参数
    TUINT32 udwCostTime = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TUINT32 udwTargetPos = strtoul(pstSession->m_stReqParam.m_szKey[1], NULL, 10);
    TINT32 dwKnightId = atoi(pstSession->m_stReqParam.m_szKey[2]);
    TCHAR *pszTroop = pstSession->m_stReqParam.m_szKey[3];
    TINT64 ddwRallyWarId = strtoull(pstSession->m_stReqParam.m_szKey[4], NULL, 10);
    TINT32 dwSide = atoi(pstSession->m_stReqParam.m_szKey[5]);

    TbMarch_action* ptbRallyWar = NULL;
    TINT32 dwRallyWarIndex = -1;
    
    dwRallyWarIndex = CActionBase::GetMarchIndex(pstUser->m_atbMarch, pstUser->m_udwMarchNum, ddwRallyWarId);
    if (dwRallyWarIndex >= 0)
    {
        ptbRallyWar = &pstUser->m_atbMarch[dwRallyWarIndex];
    }

    // 1. check param
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        TINT32 dwLevel = CCityBase::GetBuildingLevelByFuncType(pstCity, EN_BUILDING_TYPE__EMBASSY);
        if(dwLevel <= 0)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_RallyReinforce: you have no consulate [lv=%d] [seq=%u]",
                dwLevel, pstSession->m_stUserInfo.m_udwBSeqNo));
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__CONSULATE_NOT_EXIST;
            return -1;
        }

        TUINT32 udwTroopNum = EN_TROOP_TYPE__END;
        SCommonTroop stTroop;
        stTroop.Reset();
        CCommonFunc::GetArrayFromString(pszTroop, ':', stTroop.m_addwNum, udwTroopNum);
        TINT32 dwRealTime = CCommonBase::GetTroopReinforceTime(&stTroop, pstSession->m_stReqParam.m_udwCityId, udwTargetPos, &pstUser->m_stBuffWithoutDragon);
        if (!CCommonBase::IsCorrectTime(udwCostTime, dwRealTime))
        {
            TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("ProcessCmd_RallyReinforce: time_cost is incorrect [given_time:%u calc_time:%d] [seq=%u]",
                udwCostTime, dwRealTime, pstUser->m_udwBSeqNo));
            if (pstSession->m_bIsNeedCheck)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__BLACK_ACCOUNT;
                return -1;
            }
        }

        dwRetCode = CheckMarchSelfCondition(pstUser, ptbPlayer, pstCity, pszTroop, "", "", 0, dwKnightId, FALSE,
            TRUE, EN_ACTION_SEC_CLASS__RALLY_REINFORCE);
        if(dwRetCode != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = dwRetCode;
            return -2;
        }

        if(dwRallyWarIndex < 0 || ptbRallyWar == NULL)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__TARGET_NOT_EXIST;
            return -3;
        }
        if(!CActionBase::HasEmptySlot(ptbRallyWar, pstUser->m_tbPlayer.m_nUid))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NO_SLOT;
            return -4;
        }

        // 检查主action 状态
        if (ptbRallyWar->m_nStatus != EN_MARCH_STATUS__PREPARING)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__RALLYING_TIME_UP;
            return -5;
        }

        if(!CActionBase::CanRallyReinforce(ptbRallyWar, pszTroop))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__RALLY_REINFORCE_ARRIVE_LIMIT;
            return -7;
        }

        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
    }

    // 2. 获取目标的所属信息——发送请求
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        // get data
        if(0 == udwTargetPos)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MarchReinforceNormal: get map info failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -8;
        }
        pstSession->ResetAwsReq();
        CAwsRequest::MapGet(pstSession, udwTargetPos);

        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MarchReinforceNormal: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -9;
        }

        return 0;
    }

    // 3. 解析响应，生成相应的action
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;

        // parse city info
        ptbTmpMap->Reset();
        dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[0], ptbTmpMap);
        if(dwRetCode < 0 || ptbTmpMap->m_nId != udwTargetPos)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MarchReinforceNormal: get map info failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -10;
        }

        // check param
        dwRetCode = CheckMarchOtherCondition(pstUser, ptbPlayer, ptbTmpMap, EN_ACTION_SEC_CLASS__RALLY_REINFORCE, ddwRallyWarId);
        if(dwRetCode != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = dwRetCode;
            return -11;
        }
    }

    // 4. 进行计算
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        TbMarch_action* ptbRallyReinforce = CProcessMarch::GenRallyAction(pstUser, ptbPlayer, pstCity, pszTroop, dwKnightId, FALSE, ptbTmpMap, EN_ACTION_SEC_CLASS__RALLY_REINFORCE, udwCostTime, 0);
        if(ptbRallyReinforce == NULL)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -11;
        }

        ptbRallyReinforce->Set_Tid(ptbRallyWar->m_nId);
        CActionBase::HoldSlot(ptbRallyWar, ptbRallyReinforce);
        CActionBase::UpdateRallyForce(ptbRallyWar, ptbRallyReinforce, FALSE);

        pstUser->m_aucMarchFlag[dwRallyWarIndex] = EN_TABLE_UPDT_FLAG__CHANGE;

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    return 0;
}

TINT32 CProcessMarch::ProcessCmd_RallyReinforceRecall(SSession *pstSession, TBOOL &bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();

    // 0. 请求参数
    TINT64 ddwRallyReinforceId = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TINT64 ddwRallyWarId = strtoll(pstSession->m_stReqParam.m_szKey[1], NULL, 10);
    TINT32 dwItemId = atoi(pstSession->m_stReqParam.m_szKey[2]);
    TINT32 dwGem = atoi(pstSession->m_stReqParam.m_szKey[3]);

    if (dwItemId > 0 && FALSE == CItemBase::HasEnoughItem(&pstUser->m_tbBackpack, dwItemId, 1))
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ITEM_NOT_ENOUGH;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_Recall: item not enough [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }
    if(dwItemId > 0)
    {
        CItemBase::CostItem(&pstUser->m_tbBackpack, dwItemId, 1);
    }

    if(dwGem > 0 && FALSE == CPlayerBase::HasEnoughGem(&pstUser->m_tbLogin, dwGem))
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GEM_LACK;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_Recall: gem not enough gem_own=%ld price=%d [seq=%u]", pstUser->m_tbLogin.Get_Gem(), dwGem, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -2;
    }
    if(dwGem > 0)
    {
        CPlayerBase::CostGem(pstUser, dwGem);
    }

    TINT32 dwRallyReinforceIndex = CActionBase::GetMarchIndex(pstUser->m_atbMarch, pstUser->m_udwMarchNum, ddwRallyReinforceId);
    if(dwRallyReinforceIndex < 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_RallyReinforceRecall: action id invalid [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }

    TbMarch_action* ptbRallyReinforce = &pstUser->m_atbMarch[dwRallyReinforceIndex];

    if (ptbRallyReinforce->m_nStatus != EN_MARCH_STATUS__MARCHING)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -4;
    }

    if (ddwRallyWarId > 0)
    {
        TINT32 dwRallyWarIndex = -1;
        TbMarch_action* ptbRallyWar = NULL;

        dwRallyWarIndex = CActionBase::GetMarchIndex(pstUser->m_atbMarch, pstUser->m_udwMarchNum, ddwRallyWarId);
        
        if (dwRallyWarIndex < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_RallyReinforceRecall: action id invalid [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }

        ptbRallyWar = &pstUser->m_atbMarch[dwRallyWarIndex];

        if (ptbRallyReinforce->m_nTid != ptbRallyWar->m_nId)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -3;
        }

        CActionBase::ReleaseSlot(ptbRallyWar, ptbRallyReinforce);
        CActionBase::UpdateRallyForce(ptbRallyWar, ptbRallyReinforce, TRUE);

        if (ptbRallyWar->m_nStatus == EN_MARCH_STATUS__WAITING)
        {
            ptbRallyWar->Set_Etime(CTimeUtils::GetUnixTime());
        }

        pstUser->m_aucMarchFlag[dwRallyWarIndex] = EN_TABLE_UPDT_FLAG__CHANGE;
    }
    
    ptbRallyReinforce->Set_Status(EN_MARCH_STATUS__RETURNING);
    ptbRallyReinforce->Set_Btime(udwCurTime);
    ptbRallyReinforce->Set_Etime(ptbRallyReinforce->m_nCtime + 2 * udwCurTime - ptbRallyReinforce->m_nEtime);
    ptbRallyReinforce->Set_Sal(0, UPDATE_ACTION_TYPE_DELETE);
    ptbRallyReinforce->Set_Tid(0, UPDATE_ACTION_TYPE_DELETE);
    //ptbRallyReinforce->DeleteField(TbMARCH_ACTION_FIELD_SAL);
    //ptbRallyReinforce->DeleteField(TbMARCH_ACTION_FIELD_TID);
    pstUser->m_aucMarchFlag[dwRallyReinforceIndex] = EN_TABLE_UPDT_FLAG__CHANGE;

    //wave@push_data: 对于盟友来说，召回就意味着删除,因为删除了sal字段
    pstUser->m_ptbPushMarchAction = ptbRallyReinforce;
    pstUser->dwPushMarchActionType = EN_TABLE_UPDT_FLAG__DEL;
    
    return 0;
}

TINT32 CProcessMarch::ProcessCmd_RallyDismiss(SSession* pstSession, TBOOL &bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;

    TINT64 ddwRallyWarId = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TINT32 dwItemId = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TINT32 dwGem = atoi(pstSession->m_stReqParam.m_szKey[2]);

    TINT32 dwRallyWarIndex = CActionBase::GetMarchIndex(pstUser->m_atbMarch, pstUser->m_udwMarchNum, ddwRallyWarId);
    if (dwRallyWarIndex < 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_RallyDismiss: action id invalid [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }
    TbMarch_action* ptbRallyWar = &pstUser->m_atbMarch[dwRallyWarIndex];

    if (dwItemId >= 0)
    {
        if (CItemBase::HasEnoughItem(&pstUser->m_tbBackpack, dwItemId, 1))
        {
            CItemBase::CostItem(&pstUser->m_tbBackpack, dwItemId, 1);
        }
        else
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ITEM_NOT_ENOUGH;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_Recall: item not enough [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
    }
    else if (dwGem > 0)
    {
        if (CPlayerBase::HasEnoughGem(&pstUser->m_tbLogin, dwGem))
        {
            CPlayerBase::CostGem(pstUser, dwGem);
        }
        else
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GEM_LACK;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_Recall: gem lack [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }
    }
    else // no item or gem cost
    {
        if (ptbRallyWar->m_nStatus != EN_MARCH_STATUS__PREPARING && ptbRallyWar->m_nStatus != EN_MARCH_STATUS__WAITING)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_Recall: no item_id or gem send [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }
    }

    if (!CActionBase::IsPlayerOwnedAction(pstUser->m_tbPlayer.m_nUid, ptbRallyWar->m_nId))
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -4;
    }
    if (ptbRallyWar->m_nStatus != EN_MARCH_STATUS__PREPARING && ptbRallyWar->m_nStatus != EN_MARCH_STATUS__MARCHING && ptbRallyWar->m_nStatus != EN_MARCH_STATUS__WAITING)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -5;
    }

    for(TUINT32 udwReinIdx = 0; udwReinIdx < pstUser->m_udwPassiveMarchNum; udwReinIdx++)
    {
        if(pstUser->m_aucPassiveMarchFlag[udwReinIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }

        TbMarch_action* ptbRallyReinforce = &pstUser->m_atbPassiveMarch[udwReinIdx];
        if(!CActionBase::IsPlayerOwnedAction(pstUser->m_tbPlayer.m_nUid, ptbRallyReinforce->m_nId)
        && ptbRallyReinforce->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_REINFORCE
        && ptbRallyReinforce->m_nStatus != EN_MARCH_STATUS__RETURNING
        && ptbRallyReinforce->m_nTid == ptbRallyWar->m_nId)
        {
            if(ptbRallyReinforce->m_nStatus == EN_MARCH_STATUS__PREPARING)
            {
                CActionBase::ReturnMarch(ptbRallyReinforce);
            }
            else if(ptbRallyReinforce->m_nStatus == EN_MARCH_STATUS__MARCHING)
            {
                CActionBase::ReturnMarchOnFly(ptbRallyReinforce);
            }
            pstUser->m_aucPassiveMarchFlag[udwReinIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
        }
    }

    if(ptbRallyWar->m_nStatus == EN_MARCH_STATUS__MARCHING)
    {
        ptbRallyWar->Set_Status(EN_MARCH_STATUS__DELING_ON_FLY);
    }
    else
    {
        ptbRallyWar->Set_Status(EN_MARCH_STATUS__DELING);
    }
    ptbRallyWar->m_bParam[0].m_ddwLoadTime = ptbRallyWar->m_nEtime;
    ptbRallyWar->SetFlag(TbMARCH_ACTION_FIELD_PARAM);
    ptbRallyWar->Set_Etime(CTimeUtils::GetUnixTime());
    ptbRallyWar->Set_Sal(0, UPDATE_ACTION_TYPE_DELETE);
    //ptbRallyWar->DeleteField(TbMARCH_ACTION_FIELD_SAL);
    pstUser->m_aucMarchFlag[dwRallyWarIndex] = EN_TABLE_UPDT_FLAG__CHANGE;

    //wave@push_data: 对于盟友来说，召回就意味着删除,因为删除了sal字段
    pstUser->m_ptbPushMarchAction = ptbRallyWar;
    pstUser->dwPushMarchActionType = EN_TABLE_UPDT_FLAG__DEL;

    return 0;
}

TINT32 CProcessMarch::ProcessCmd_RallySlotBuy(SSession* pstSession, TBOOL &bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    // 0. 请求参数
    TINT64 ddwRallyWarId = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TINT32 dwType = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TINT32 dwItemId = atoi(pstSession->m_stReqParam.m_szKey[2]);
    TINT32 dwGemNum = atoi(pstSession->m_stReqParam.m_szKey[3]);
    TBOOL bPrivate = atoi(pstSession->m_stReqParam.m_szKey[4]);
    TINT32 dwSide = atoi(pstSession->m_stReqParam.m_szKey[5]);

    if(dwType == 1)
    {
        if(FALSE == CPlayerBase::HasEnoughGem(&pstUser->m_tbLogin, dwGemNum))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GEM_LACK;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_RallySoltBuy: gem not enough [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
    }
    else if(dwType == 0)
    {
        if (FALSE == CItemBase::HasEnoughItem(&pstUser->m_tbBackpack, dwItemId))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ITEM_NOT_ENOUGH;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_RallySoltBuy: item not enough [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }
    }
    else
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_RallySoltBuy: req item and gem all empty [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
        return -3;
    }

    TINT32 dwRallyWarIndex = -1;
    TbMarch_action* ptbRallyWar = NULL;
    
    dwRallyWarIndex = CActionBase::GetMarchIndex(pstUser->m_atbMarch, pstUser->m_udwMarchNum, ddwRallyWarId);

    if(dwRallyWarIndex < 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_RallySoltBuy: action id invalid [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
        return -4;
    }
    ptbRallyWar = &pstUser->m_atbMarch[dwRallyWarIndex];

    if(ptbRallyWar->m_nStatus != EN_MARCH_STATUS__PREPARING)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -5;
    }

    if(bPrivate)
    {
        if(CActionBase::HasPrivateSlot(ptbRallyWar, pstUser->m_tbPlayer.m_nUid))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -6;
        }
    }

    if (ptbRallyWar->m_bRally_atk_slot.m_udwNum >= CCommonBase::GetGameBasicVal(EN_GAME_BASIC_RALLY_SLOT_LIMIT))
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ADOBE_SAID_NETWORK_ERROR;
        return -7;
    }

    if(bPrivate)
    {
        CActionBase::AddSlot(ptbRallyWar, pstUser->m_tbPlayer.m_nUid, pstUser->m_tbPlayer.m_sUin);
    }
    else
    {
        CActionBase::AddSlot(ptbRallyWar, 0, pstUser->m_tbPlayer.m_sUin);
    }

    pstUser->m_aucMarchFlag[dwRallyWarIndex] = EN_TABLE_UPDT_FLAG__CHANGE;

    if(dwType == 1)
    {
        pstSession->m_udwGemCost = dwGemNum;
        CPlayerBase::CostGem(pstUser, dwGemNum);
    }
    else if(dwType == 0)
    {
        CItemBase::CostItem(&pstSession->m_stUserInfo.m_tbBackpack, dwItemId);
    }


    //wave@push_data: 对于盟友和目标来说，召回就意味着删除,因为删除了sal字段
    pstUser->m_ptbPushMarchAction = ptbRallyWar;
    pstUser->dwPushMarchActionType = EN_TABLE_UPDT_FLAG__CHANGE;

    return 0;
}

TINT32 CProcessMarch::ProcessCmd_RallyInfo(SSession* pstSession, TBOOL &bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;

    TINT64 ddwRallyWarId = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TINT32 dwSide = atoi(pstSession->m_stReqParam.m_szKey[1]);

    TINT32 dwRetCode = 0;
    TbMarch_action *ptbRally = NULL;

    if (dwSide == EN_RALLY_SIDE_ATTACK)
    {
        ptbRally = CActionBase::GetMarch(pstUser->m_atbMarch, pstUser->m_udwMarchNum, ddwRallyWarId);
    }
    else
    {
        ptbRally = CActionBase::GetMarch(pstUser->m_atbPassiveMarch, pstUser->m_udwPassiveMarchNum, ddwRallyWarId);
    }
    if (ptbRally == NULL)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__TARGET_NOT_EXIST;
        return -1;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        pstSession->ResetAwsInfo();
        CAwsRequest::MarchActionQueryByTuid(pstSession, pstSession->m_stReqParam.m_udwSvrId, ptbRally->m_nSuid);
        if (ptbRally->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR)
        {
            CAwsRequest::CityGetByUid(pstSession, ptbRally->m_nTuid);
            CAwsRequest::MarchActionQueryByTuid(pstSession, pstSession->m_stReqParam.m_udwSvrId, ptbRally->m_nTuid);
            CAwsRequest::MapGet(pstSession, ptbRally->m_nTpos);
        }
        else
        {
            CAwsRequest::ThroneGet(pstSession, ptbRally->m_nSid);
        }

        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_RallyInfo: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }

        return 0;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        TbCity tbCity;
        tbCity.Reset();
        TbMap tbMap;
        tbMap.Reset();
        TbThrone tbThrone;
        tbThrone.Reset();

        for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecAwsRsp.size(); ++udwIdx)
        {
            AwsRspInfo *pstAwsRspInfo = pstSession->m_vecAwsRsp[udwIdx];
            string strTableRawName = CCommonFunc::GetTableRawName(pstAwsRspInfo->sTableName);

            if (strTableRawName == EN_AWS_TABLE_MARCH_ACTION)
            {
                dwRetCode = CAwsResponse::OnQueryRsp(*pstAwsRspInfo, pstUser->m_atbRallyMarch + pstUser->m_udwRallyMarchNum, sizeof(TbMarch_action), MAX_USER_MARCH_NUM);
                if (dwRetCode >= 0)
                {
                    pstUser->m_udwRallyMarchNum += dwRetCode;
                }
            }
            if (strTableRawName == EN_AWS_TABLE_CITY)
            {
                CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &tbCity);
                continue;
            }
            if (strTableRawName == EN_AWS_TABLE_MAP)
            {
                CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &tbMap);
                continue;
            }
            if (strTableRawName == EN_AWS_TABLE_THRONE)
            {
                CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &tbThrone);
                continue;
            }
        }

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;

        if (ptbRally->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR)
        {
            if (tbCity.m_nUid == 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__USER_INFO_INCOMPLETE;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_RallyInfo: get player city failed [seq=%u]",
                    pstSession->m_stUserInfo.m_udwBSeqNo));
                return -4;
            }

            CActionBase::ResetRallyDefence(ptbRally, &tbCity);
            SCommonTroop stReinforceTroop;
            stReinforceTroop.Reset();
            TINT64 ddwReinforceTroopLimit = ptbRally->m_bRally_def_force[0].ddwReinforceTroopLimit;
            TINT64 ddwReinforceNum = 0;
            TINT64 ddwReinforceForce = 0;
            TINT64 ddwTotalNum = 0;
            TINT64 ddwTotalForce = 0;
            if (tbMap.m_nUid == ptbRally->m_nTuid && tbMap.m_nType == EN_WILD_TYPE__CITY)
            {
                ddwReinforceTroopLimit = tbMap.m_jCity_info["reinforced_troop_limit"].asInt64();
            }
            for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwRallyMarchNum; ++udwIdx)
            {
                TbMarch_action *ptbReinforce = &pstUser->m_atbRallyMarch[udwIdx];
                if (ptbReinforce->m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_NORMAL &&
                    ptbReinforce->m_nStatus != EN_MARCH_STATUS__RETURNING &&
                    ptbReinforce->m_nTuid == ptbRally->m_nTuid && ptbReinforce->m_nTpos == ptbRally->m_nTpos)
                {
                    if (CActionBase::RallyInfoAddDefence(ptbRally, ptbReinforce))
                    {
                        for (TUINT32 udwIdx = 0; udwIdx < EN_TROOP_TYPE__END; ++udwIdx)
                        {
                            stReinforceTroop[udwIdx] += ptbReinforce->m_bParam[0].m_stTroop.m_addwNum[udwIdx];
                        }
                    }
                }
            }

            for (TUINT32 udwIdx = 0; udwIdx < EN_TROOP_TYPE__END; ++udwIdx)
            {
                ddwReinforceNum += stReinforceTroop[udwIdx];
                ddwTotalNum += tbCity.m_bTroop[0].m_addwNum[udwIdx];
            }
            ddwTotalNum += ddwReinforceNum;
            ddwReinforceForce = CCityBase::CalcTroopMight(stReinforceTroop.m_addwNum);
            ddwTotalForce = ddwReinforceForce + CCityBase::CalcTroopMight(tbCity.m_bTroop[0].m_addwNum);

            TBOOL bIsNeedUpdate = FALSE;
            if (ptbRally->m_bRally_def_force[0].ddwReinforceTroopLimit != ddwReinforceTroopLimit
                || ptbRally->m_bRally_def_force[0].ddwReinforceNum != ddwReinforceNum
                || ptbRally->m_bRally_def_force[0].ddwReinforceForce != ddwReinforceForce
                || ptbRally->m_bRally_def_force[0].ddwTotalNum != ddwTotalNum
                || ptbRally->m_bRally_def_force[0].ddwTotalForce != ddwTotalForce)
            {
                ptbRally->m_bRally_def_force[0].ddwReinforceTroopLimit = ddwReinforceTroopLimit;
                ptbRally->m_bRally_def_force[0].ddwReinforceNum = ddwReinforceNum;
                ptbRally->m_bRally_def_force[0].ddwReinforceForce = ddwReinforceForce;
                ptbRally->m_bRally_def_force[0].ddwTotalNum = ddwTotalNum;
                ptbRally->m_bRally_def_force[0].ddwTotalForce = ddwTotalForce;
                ptbRally->SetFlag(TbMARCH_ACTION_FIELD_RALLY_DEF_FORCE);
                ptbRally->SetFlag(TbMARCH_ACTION_FIELD_DEF_TOTAL_TROOP);
                bIsNeedUpdate = TRUE;
            }

            if (bIsNeedUpdate)
            {
                pstSession->ResetAwsInfo();
                ExpectedDesc expect_desc;
                ExpectedItem expect_item;
                expect_item.SetVal(TbMARCH_ACTION_FIELD_ID, TRUE, ptbRally->m_nId);
                expect_desc.push_back(expect_item);

                CAwsRequest::UpdateItem(pstSession, ptbRally, expect_desc);

                pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

                bNeedResponse = TRUE;
                dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
                if (dwRetCode < 0)
                {
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_RallyInfo: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                    return -2;
                }
            }
        }
        else
        {
            if (tbThrone.m_nPos != ptbRally->m_nTpos)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__USER_INFO_INCOMPLETE;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_RallyInfo: get throne info failed [seq=%u]",
                    pstSession->m_stUserInfo.m_udwBSeqNo));
                return -4;
            }
            ptbRally->m_bRally_def_force[0].ddwReinforceTroopLimit = CCommonBase::GetGameBasicVal(EN_GAME_BASIC_THRONE_TROOP_NUM_LIMIT);
            ptbRally->m_bRally_def_force[0].ddwReinforceNum = tbThrone.m_nDefending_troop_num;
            ptbRally->m_bRally_def_force[0].ddwReinforceForce = tbThrone.m_nDefending_troop_force;
            ptbRally->m_bRally_def_force[0].ddwTotalNum = tbThrone.m_nDefending_troop_num;
            ptbRally->m_bRally_def_force[0].ddwTotalForce = tbThrone.m_nDefending_troop_force;
        }
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_RALLY_WAR_INFO;
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    return 0;
}

TINT32 CProcessMarch::ProcessCmd_RallyHistory(SSession* pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;

    if(pstUser->m_tbPlayer.m_nAlpos == EN_ALLIANCE_POS__REQUEST)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NOT_IN_ALLIANCE;
        return -1;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        pstSession->ResetAwsInfo();
        TbRally_history tbRallyHistory;
        tbRallyHistory.Set_Aid(pstUser->m_tbAlliance.m_nAid);
        tbRallyHistory.Set_Rid(0);

        CompareDesc stCompareInfo;
        stCompareInfo.dwCompareType = COMPARE_TYPE_GT;

        CAwsRequest::Query(pstSession, &tbRallyHistory, ETbRALLYHISTORY_OPEN_TYPE_PRIMARY, stCompareInfo, true, true, false, MAX_RALLY_HISTORY);

        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_RallyHistory:: send aws query req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }
        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        dwRetCode = CAwsResponse::OnQueryRsp(*pstSession->m_vecAwsRsp[0], pstSession->m_atbTmpRallyHistory, sizeof(TbRally_history), MAX_RALLY_HISTORY);
        if(dwRetCode > 0)
        {
            pstSession->m_udwTmpRallyHistoryNum = dwRetCode;
        }
        // next procedure
        pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_RALLY_HISTORY;
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;

        return 0;
    }
    return 0;
}

TINT32 CProcessMarch::ProcessCmd_ManorAbandon(SSession* pstSession, TBOOL &bNeedResponse)
{
    //废弃
    return -1;
}

TINT32 CProcessMarch::ProcessCmd_Repatriate(SSession *pstSession, TBOOL &bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TINT32 dwActionIdx = -1;
    TbMarch_action *ptbMarch = NULL;

    // 0. 请求参数
    TINT64 uddwActionId = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    dwActionIdx = CActionBase::GetMarchIndex(pstUser->m_atbPassiveMarch, pstUser->m_udwPassiveMarchNum, uddwActionId);
    if(dwActionIdx < 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_Repatriate: march id invalid [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    ptbMarch = &pstUser->m_atbPassiveMarch[dwActionIdx];

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
	    if (ptbMarch->m_nMclass == EN_ACTION_MAIN_CLASS__MARCH && ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_NORMAL
            && ptbMarch->m_nStatus == EN_MARCH_STATUS__RETURNING && ptbMarch->m_nTuid == pstUser->m_tbPlayer.m_nUid)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_Repatriate: march returning [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
            return 0;
        }
	
        if(ptbMarch->m_nMclass != EN_ACTION_MAIN_CLASS__MARCH || ptbMarch->m_nSclass != EN_ACTION_SEC_CLASS__REINFORCE_NORMAL
            || ptbMarch->m_nStatus != EN_MARCH_STATUS__DEFENDING || ptbMarch->m_nTuid != pstUser->m_tbPlayer.m_nUid)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ACTION_HAS_BEEN_RECALL;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_Repatriate: wrong param [Mclass=%ld] [Sclass=%ld] [Action Status=%ld] [Tuid=%ld] [seq=%u]",
                ptbMarch->m_nMclass, ptbMarch->m_nSclass, ptbMarch->m_nStatus, ptbMarch->m_nTuid, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }

        CActionBase::ReturnMarch(ptbMarch);
        pstUser->m_aucPassiveMarchFlag[dwActionIdx] = EN_TABLE_UPDT_FLAG__CHANGE;

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessMarch::ProcessCmd_RecallAllReinforce(SSession *pstSession, TBOOL &bNeedResponse)
{
    //没用...
    /*
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;

    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();

    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
    {
        TbMarch_action *ptbMarch = &pstUser->m_atbMarch[udwIdx];

        if (pstUser->m_aucMarchFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }
        if (!CActionBase::IsPlayerOwnedAction(ptbPlayer->m_nUid, ptbMarch->m_nId))
        {
            continue;
        }

        if (ptbMarch->m_nMclass == EN_ACTION_MAIN_CLASS__MARCH && ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_NORMAL)
        {
            ptbMarch->Set_Status(EN_MARCH_STATUS__RETURNING);
            ptbMarch->Set_Btime(udwCurTime);
            ptbMarch->Set_Ctime(ptbMarch->m_bParam[0].m_ddwMarchingTime);
            ptbMarch->Set_Etime(ptbMarch->m_nBtime + ptbMarch->m_nCtime);
            pstUser->m_aucMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;

            TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessCmd_RecallAllReinforce: recall march[%d]: action_id[%ld] [seq=%u]",
                udwIdx, ptbMarch->m_nId, pstSession->m_stUserInfo.m_udwBSeqNo));

            //wave@push_data: 特殊情况，批量处理
            CPushDataProcess::SendPushDataRequest_MarchAction(pstSession, pstSession->m_stReqParam.m_udwUserId, pstSession->m_stReqParam.m_udwAllianceId, ptbMarch, EN_TABLE_UPDT_FLAG__DEL);
        }
    }
    */
    return -1;
}

TINT32 CProcessMarch::ProcessCmd_SendbackAllReinforce(SSession *pstSession, TBOOL &bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;

    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwPassiveMarchNum; ++udwIdx)
    {
        TbMarch_action *ptbMarch = &pstUser->m_atbPassiveMarch[udwIdx];

        if (pstUser->m_aucPassiveMarchFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }
        if (ptbMarch->m_nTuid != ptbPlayer->m_nUid)
        {
            continue;
        }

        if (ptbMarch->m_nMclass == EN_ACTION_MAIN_CLASS__MARCH && ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_NORMAL &&
            ptbMarch->m_nStatus != EN_MARCH_STATUS__RETURNING && ptbMarch->m_nTuid == ptbPlayer->m_nUid)
        {
            if (ptbMarch->m_nStatus == EN_MARCH_STATUS__MARCHING)
            {
                CActionBase::ReturnMarchOnFly(ptbMarch);
            }
            else
            {
                CActionBase::ReturnMarch(ptbMarch);
            }
            pstUser->m_aucPassiveMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;

            TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessCmd_SendbackAllReinforce: sendback p_march[%d]: action_id[%ld] [seq=%u]",
                udwIdx, ptbMarch->m_nId, pstSession->m_stUserInfo.m_udwBSeqNo));
        }
    }

    return 0;
}

TINT32 CProcessMarch::ProcessCmd_MarchCamp(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;
    TbMap *ptbTmpMap = &pstSession->m_tbTmpMap;

    // 0. 输入参数
    TUINT32 udwCostTime = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TUINT32 udwTargetPos = strtoul(pstSession->m_stReqParam.m_szKey[1], NULL, 10);
    TINT32	dwKnightId = atoi(pstSession->m_stReqParam.m_szKey[2]) % MAX_KNIGHT_NUM;
    TCHAR *pszTroop = pstSession->m_stReqParam.m_szKey[3];

    TBOOL bIsDragonJoin = FALSE;

    // 暂不开放camp
    //pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__FUNCTION_NOT_OPEN;
    //return -1;

    // 1. check param
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        TUINT32 udwTroopNum = EN_TROOP_TYPE__END;
        SCommonTroop stTroop;
        stTroop.Reset();
        CCommonFunc::GetArrayFromString(pszTroop, ':', stTroop.m_addwNum, udwTroopNum);
        SPlayerBuffInfo *pstBuff = &pstUser->m_stPlayerBuffList;
        if (bIsDragonJoin)
        {
            pstBuff = &pstUser->m_stPlayerBuffList;
        }
        else
        {
            pstBuff = &pstUser->m_stBuffWithoutDragon;
        }
        TINT32 dwRealTime = CCommonBase::GetTroopMarchTime(&stTroop, pstSession->m_stReqParam.m_udwCityId, udwTargetPos, pstBuff);
        if (!CCommonBase::IsCorrectTime(udwCostTime, dwRealTime))
        {
            TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("ProcessCmd_MarchCamp: time_cost is incorrect [given_time:%u calc_time:%d] [seq=%u]",
                udwCostTime, dwRealTime, pstUser->m_udwBSeqNo));
            if (pstSession->m_bIsNeedCheck)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__BLACK_ACCOUNT;
                return -1;
            }
        }

        dwRetCode = CheckMarchSelfCondition(pstUser, ptbPlayer, pstCity, pszTroop, "", "", 0, dwKnightId, bIsDragonJoin,
            FALSE, EN_ACTION_SEC_CLASS__CAMP);
        if (dwRetCode != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = dwRetCode;
            return -1;
        }

        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
    }

    // 2. 获取目标的所属信息——发送请求
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        // get data
        if (0 == udwTargetPos)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MarchCamp: get map info failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }
        pstSession->ResetAwsReq();
        CAwsRequest::MapGet(pstSession, udwTargetPos);

        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MarchCamp: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }

        return 0;
    }

    // 3. 解析响应，生成相应的action
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;

        // parse city info
        ptbTmpMap->Reset();
        dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[0], ptbTmpMap);
        if (dwRetCode < 0 || ptbTmpMap->m_nId != udwTargetPos)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MarchCamp: get map info failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -4;
        }

        // check param
        dwRetCode = CheckMarchOtherCondition(pstUser, ptbPlayer, ptbTmpMap, EN_ACTION_SEC_CLASS__CAMP);
        if (dwRetCode != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = dwRetCode;
            return -5;
        }
    }

    // 4. 进行计算
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        GenMarchAction(pstUser, ptbPlayer, pstCity, pszTroop, "", "", dwKnightId, bIsDragonJoin, ptbTmpMap, EN_ACTION_SEC_CLASS__CAMP, udwCostTime);
        // next procedure
        bNeedResponse = FALSE;
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;

        return 0;
    }

    return 0;
}

TINT32 CProcessMarch::ProcessCmd_IdolAttack(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;
    TbMap *ptbTmpMap = &pstSession->m_tbTmpMap;

    // 0. 输入参数
    TUINT32 udwCostTime = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TUINT32 udwTargetPos = strtoul(pstSession->m_stReqParam.m_szKey[1], NULL, 10);
    TINT32	dwKnightId = atoi(pstSession->m_stReqParam.m_szKey[2]) % MAX_KNIGHT_NUM;
    TCHAR *pszTroop = pstSession->m_stReqParam.m_szKey[3];
    TBOOL bIsDragonJoin = atoi(pstSession->m_stReqParam.m_szKey[4]);

    // 1. check param
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        TUINT32 udwTroopNum = EN_TROOP_TYPE__END;
        SCommonTroop stTroop;
        stTroop.Reset();
        CCommonFunc::GetArrayFromString(pszTroop, ':', stTroop.m_addwNum, udwTroopNum);
//         SPlayerBuffInfo *pstBuff = &pstUser->m_stPlayerBuffList;
//         if (bIsDragonJoin)
//         {
//             pstBuff = &pstUser->m_stPlayerBuffList;
//         }
//         else
//         {
//             pstBuff = &pstUser->m_stBuffWithoutDragon;
//         }
//         TINT32 dwRealTime = CCommonBase::GetTroopMarchTime(&stTroop, pstSession->m_stReqParam.m_udwCityId, udwTargetPos, pstBuff);
//         if (!CCommonBase::IsCorrectTime(udwCostTime, dwRealTime))
//         {
//             TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("ProcessCmd_IdolAttack: time_cost is incorrect [given_time:%u calc_time:%d] [seq=%u]",
//                 udwCostTime, dwRealTime, pstUser->m_udwBSeqNo));
//             if (pstSession->m_bIsNeedCheck)
//             {
//                 pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__BLACK_ACCOUNT;
//                 return -1;
//             }
//         }

        dwRetCode = CheckMarchSelfCondition(pstUser, ptbPlayer, pstCity, pszTroop, "", "", 0, dwKnightId, bIsDragonJoin,
            TRUE, EN_ACTION_SEC_CLASS__ATTACK_IDOL);
        if (dwRetCode != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = dwRetCode;
            return -1;
        }
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
    }

    // 2. 获取目标的所属信息——发送请求
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        // get data
        if (0 == udwTargetPos)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_IdolAttack: get map info failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }
        pstSession->ResetAwsReq();
        CAwsRequest::MapGet(pstSession, udwTargetPos);

        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_IdolAttack: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }

        return 0;
    }

    // 3. 解析响应，生成相应的action
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;

        // parse city info
        ptbTmpMap->Reset();
        dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[0], ptbTmpMap);
        if (dwRetCode < 0 || ptbTmpMap->m_nId != udwTargetPos)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_IdolAttack: get map info failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -4;
        }

        // check param
        dwRetCode = CheckMarchOtherCondition(pstUser, ptbPlayer, ptbTmpMap, EN_ACTION_SEC_CLASS__ATTACK_IDOL);
        if (dwRetCode != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = dwRetCode;
            return -5;
        }
    }

    // 4. 进行计算
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        GenMarchAction(pstUser, ptbPlayer, pstCity, pszTroop, "", "", dwKnightId, bIsDragonJoin, ptbTmpMap, EN_ACTION_SEC_CLASS__ATTACK_IDOL, udwCostTime);
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;

        return 0;
    }

    return 0;
}

TINT32 CProcessMarch::ProcessCmd_ThroneAttack(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;
    TbMap *ptbTmpMap = &pstSession->m_tbTmpMap;

    // 0. 输入参数
    TUINT32 udwCostTime = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TUINT32 udwTargetPos = strtoul(pstSession->m_stReqParam.m_szKey[1], NULL, 10);
    TINT32	dwKnightId = atoi(pstSession->m_stReqParam.m_szKey[2]) % MAX_KNIGHT_NUM;
    TCHAR *pszTroop = pstSession->m_stReqParam.m_szKey[3];
    TBOOL bIsDragonJoin = atoi(pstSession->m_stReqParam.m_szKey[4]);

    // 1. check param
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        TUINT32 udwTroopNum = EN_TROOP_TYPE__END;
        SCommonTroop stTroop;
        stTroop.Reset();
        CCommonFunc::GetArrayFromString(pszTroop, ':', stTroop.m_addwNum, udwTroopNum);

        dwRetCode = CheckMarchSelfCondition(pstUser, ptbPlayer, pstCity, pszTroop, "", "", 0, dwKnightId, bIsDragonJoin,
            TRUE, EN_ACTION_SEC_CLASS__ATTACK_THRONE);
        if (dwRetCode != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = dwRetCode;
            return -1;
        }
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
    }

    // 2. 获取目标的所属信息——发送请求
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        // get data
        if (0 == udwTargetPos || udwTargetPos != pstSession->m_tbThrone.m_nPos)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ThroneAttack: throne_pos[%u, %ld] error [seq=%u]", 
                udwTargetPos, pstSession->m_tbThrone.m_nPos, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }
        pstSession->ResetAwsReq();
        CAwsRequest::MapGet(pstSession, udwTargetPos);

        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ThroneAttack: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }

        return 0;
    }

    // 3. 解析响应，生成相应的action
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;

        // parse city info
        ptbTmpMap->Reset();
        dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[0], ptbTmpMap);
        if (dwRetCode < 0 || ptbTmpMap->m_nId != udwTargetPos)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ThroneAttack: get map info failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -4;
        }

        // check param
        dwRetCode = CheckMarchOtherCondition(pstUser, ptbPlayer, ptbTmpMap, EN_ACTION_SEC_CLASS__ATTACK_THRONE);
        if (dwRetCode != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = dwRetCode;
            return -5;
        }
    }

    // 4. 进行计算
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        SNoticInfo stNoticInfo;
        stNoticInfo.Reset();
        stNoticInfo.SetValue(EN_NOTI_ID__COMING_ATTACK_7, ptbPlayer->m_sUin, "", 0, 0, 0, 0, udwCostTime, "", 0);
        CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), pstSession->m_stReqParam.m_udwSvrId, ptbTmpMap->m_nUid, stNoticInfo);

        GenMarchAction(pstUser, ptbPlayer, pstCity, pszTroop, "", "", dwKnightId, bIsDragonJoin, ptbTmpMap, EN_ACTION_SEC_CLASS__ATTACK_THRONE, udwCostTime);
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;

        return 0;
    }

    return 0;
}

TINT32 CProcessMarch::ProcessCmd_ThroneRallyWar(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;
    TbMap *ptbTmpMap = &pstSession->m_tbTmpMap;

    // 0. 输入参数
    TUINT32 udwCostTime = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TUINT32 udwTargetPos = strtoul(pstSession->m_stReqParam.m_szKey[1], NULL, 10);
    TINT32 dwKnightId = atoi(pstSession->m_stReqParam.m_szKey[2]);
    TCHAR *pszTroop = pstSession->m_stReqParam.m_szKey[3];
    TUINT32 udwPrepareTime = strtoul(pstSession->m_stReqParam.m_szKey[4], NULL, 10);

    TBOOL bIsDragonJoin = TRUE;
    if (strlen(pstSession->m_stReqParam.m_szKey[5]) > 0)
    {
        bIsDragonJoin = atoi(pstSession->m_stReqParam.m_szKey[5]);
    }


    // 1. check param
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        TINT32 dwLevel = CCityBase::GetBuildingLevelByFuncType(pstCity, EN_BUILDING_TYPE__EMBASSY);
        if (dwLevel <= 0)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ThroneRallyWar: you have no consulate [lv=%d] [seq=%u]",
                dwLevel, pstSession->m_stUserInfo.m_udwBSeqNo));
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__CONSULATE_NOT_EXIST;
            return -1;
        }

        dwRetCode = CheckMarchSelfCondition(pstUser, ptbPlayer, pstCity, pszTroop, "", "", 0, dwKnightId, bIsDragonJoin,
            TRUE, EN_ACTION_SEC_CLASS__RALLY_WAR_THRONE);
        if (dwRetCode != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = dwRetCode;
            return -1;
        }

        TINT32 dwCanRallyWar = CheckCanRallyWar(pstUser, udwTargetPos, pstSession->m_stReqParam.m_udwAllianceId);
        if (dwCanRallyWar != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = dwCanRallyWar;
            return -1;
        }

        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
    }

    // 2. 获取目标的所属信息——发送请求
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        // get data
        if (0 == udwTargetPos || udwTargetPos != pstSession->m_tbThrone.m_nPos)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ThroneRallyWar: throne_pos[%u, %ld] error [seq=%u]",
                udwTargetPos, pstSession->m_tbThrone.m_nPos, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }
        pstSession->ResetAwsReq();
        CAwsRequest::MapGet(pstSession, udwTargetPos);

        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ThroneRallyWar: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }

        return 0;
    }

    // 3. 解析响应，生成相应的action
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;

        // parse city info
        ptbTmpMap->Reset();
        dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[0], ptbTmpMap);
        if (dwRetCode < 0 || ptbTmpMap->m_nId != udwTargetPos)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ThroneRallyWar: get map info failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -4;
        }

        // check param
        dwRetCode = CheckMarchOtherCondition(pstUser, ptbPlayer, ptbTmpMap, EN_ACTION_SEC_CLASS__RALLY_WAR_THRONE);
        if (dwRetCode != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = dwRetCode;
            return -5;
        }
    }

    // 4. 进行计算
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        string strTarget = ptbTmpMap->m_sUname;

        if (ptbTmpMap->m_nUid > 0)
        {
            //tips
            CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPY__BE_RALLY_WARED, ptbTmpMap->m_nUid, TRUE, 0, 0, 0, ptbPlayer->m_sUin.c_str());  //给目标
        }
        CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPY__RALLY_WAR_BEGIN_TO_ASSEMBLE, ptbPlayer->m_nUid, FALSE, EN_WILD_TYPE__THRONE_NEW, 0, 0, strTarget.c_str());  //给发起者

        SNoticInfo stNoticInfo;
        stNoticInfo.Reset();
        stNoticInfo.SetValue(EN_NOTI_ID__RALLYWAR_START, ptbPlayer->m_sUin, strTarget, 0, 0, 0, 0, udwCostTime + udwPrepareTime, "", 0);
        CMsgBase::SendNotificationAlliance(CConfBase::GetString("tbxml_project"), pstSession->m_stReqParam.m_udwSvrId, ptbPlayer->m_nUid, ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET, stNoticInfo);

        if (ptbTmpMap->m_nUid > 0)
        {
            SNoticInfo stNoticInfo;
            stNoticInfo.Reset();
            stNoticInfo.SetValue(EN_NOTI_ID__RALLYWAR_DEFENSE, ptbTmpMap->m_sUname, ptbPlayer->m_sUin, 0, 0, 0, 0, udwCostTime + udwPrepareTime, "", 0);
            CMsgBase::SendNotificationAlliance(CConfBase::GetString("tbxml_project"), pstSession->m_stReqParam.m_udwSvrId, ptbPlayer->m_nUid, ptbTmpMap->m_nAlid, stNoticInfo);
        }

        TbMarch_action* ptbRallyWar = CProcessMarch::GenRallyAction(pstUser, ptbPlayer, pstCity, pszTroop, dwKnightId, bIsDragonJoin, ptbTmpMap, EN_ACTION_SEC_CLASS__RALLY_WAR_THRONE, udwCostTime, udwPrepareTime);

        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_ThroneRallyWar: gen rallywar time[%ld:%ld:%ld] prepare rally atk_slot[%u] def_slot[%u] [seq=%u]",
            ptbRallyWar->m_nBtime, ptbRallyWar->m_nCtime, ptbRallyWar->m_nEtime,
            ptbRallyWar->m_bRally_atk_slot.m_udwNum, ptbRallyWar->m_bRally_def_slot.m_udwNum, pstSession->m_stUserInfo.m_udwBSeqNo));

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    return 0;
}

TINT32 CProcessMarch::ProcessCmd_ThroneReinforce(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;
    TbMap *ptbTmpMap = &pstSession->m_tbTmpMap;

    // 0. 输入参数
    TUINT32 udwCostTime = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TUINT32 udwTargetPos = strtoul(pstSession->m_stReqParam.m_szKey[1], NULL, 10);
    TCHAR *pszTroop = pstSession->m_stReqParam.m_szKey[2];
    TINT32 dwSide = EN_RALLY_SIDE_DEFENCE; // atoi(pstSession->m_stReqParam.m_szKey[3]);
    TINT64 ddwRallyWarId = 0; // strtoull(pstSession->m_stReqParam.m_szKey[4], NULL, 10);

    TbMarch_action* ptbRallyWar = NULL;
    TINT32 dwRallyWarIndex = -1;

    if (dwSide == EN_RALLY_SIDE_ATTACK)
    {
        dwRallyWarIndex = CActionBase::GetMarchIndex(pstUser->m_atbMarch, pstUser->m_udwMarchNum, ddwRallyWarId);
        if (dwRallyWarIndex >= 0)
        {
            ptbRallyWar = &pstUser->m_atbMarch[dwRallyWarIndex];
        }
    }
    else
    {
        ddwRallyWarId = 0;
    }

    // 1. check param
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        TINT32 dwLevel = CCityBase::GetBuildingLevelByFuncType(pstCity, EN_BUILDING_TYPE__EMBASSY);
        if (dwLevel <= 0)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ThroneReinforce: you have no consulate [lv=%d] [seq=%u]",
                dwLevel, pstSession->m_stUserInfo.m_udwBSeqNo));
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__CONSULATE_NOT_EXIST;
            return -1;
        }

        dwRetCode = CheckMarchSelfCondition(pstUser, ptbPlayer, pstCity, pszTroop, "", "", 0, -1, FALSE,
            TRUE, EN_ACTION_SEC_CLASS__REINFORCE_THRONE);
        if (dwRetCode != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = dwRetCode;
            return -2;
        }

        if (dwSide == EN_RALLY_SIDE_ATTACK)
        {
            if (dwRallyWarIndex < 0 || ptbRallyWar == NULL)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__TARGET_NOT_EXIST;
                return -3;
            }

            if (!CActionBase::HasEmptySlot(ptbRallyWar, pstUser->m_tbPlayer.m_nUid))
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NO_SLOT;
                return -4;
            }

            // 检查主action 状态
            if (ptbRallyWar->m_nStatus != EN_MARCH_STATUS__PREPARING)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__RALLYING_TIME_UP;
                return -5;
            }

            if (!CActionBase::CanRallyReinforce(ptbRallyWar, pszTroop))
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__RALLY_REINFORCE_ARRIVE_LIMIT;
                return -7;
            }
        }

        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
    }

    // 2. 获取目标的所属信息——发送请求
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        // get data
        if (0 == udwTargetPos || (ddwRallyWarId == 0 && udwTargetPos != pstSession->m_tbThrone.m_nPos))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ThroneReinforce: pos[%u] error [seq=%u]",
                udwTargetPos, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -8;
        }
        pstSession->ResetAwsReq();
        CAwsRequest::MapGet(pstSession, udwTargetPos);

        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ThroneReinforce: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -9;
        }

        return 0;
    }

    // 3. 解析响应，生成相应的action
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;

        // parse city info
        ptbTmpMap->Reset();
        dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[0], ptbTmpMap);
        if (dwRetCode < 0 || ptbTmpMap->m_nId != udwTargetPos)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ThroneReinforce: get map info failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -10;
        }

        // check param
        dwRetCode = CheckMarchOtherCondition(pstUser, ptbPlayer, ptbTmpMap, EN_ACTION_SEC_CLASS__REINFORCE_THRONE, ddwRallyWarId);
        if (dwRetCode != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = dwRetCode;
            return -11;
        }

        if (ddwRallyWarId == 0)
        {
            TUINT32 udwAssignNum = 1;
            if (pstUser->m_tbPlayer.m_nUid != pstUser->m_tbAlliance.m_nOid)
            {
                for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; udwIdx++)
                {
                    if (pstUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__ASSIGN_THRONE
                        && pstUser->m_atbMarch[udwIdx].m_nStatus != EN_MARCH_STATUS__RETURNING)
                    {
                        udwAssignNum = 0;
                        break;
                    }
                }
            }
            else
            {
                udwAssignNum = 0;
            }

            TUINT32 udwTroopNum = EN_TROOP_TYPE__END;
            TINT64 addwTroop[EN_TROOP_TYPE__END] = { 0 };
            CCommonFunc::GetArrayFromString(pszTroop, ':', addwTroop, udwTroopNum);
            TINT32 dwTroopNum = CToolBase::GetTroopSumNum(addwTroop);
            if (CCommonBase::GetGameBasicVal(EN_GAME_BASIC_RALLY_SLOT_LIMIT) - udwAssignNum <= pstSession->m_tbThrone.m_nReinforce_num
                || CCommonBase::GetGameBasicVal(EN_GAME_BASIC_THRONE_TROOP_NUM_LIMIT) < pstSession->m_tbThrone.m_nReinforce_troop_num + dwTroopNum)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REINFORCE_ARRIVE_LIMIT;
                return -6;
            }
        }
    }

    // 4. 进行计算
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        TbMarch_action* ptbRallyReinforce = CProcessMarch::GenRallyAction(pstUser, ptbPlayer, pstCity, pszTroop, -1, FALSE, ptbTmpMap, EN_ACTION_SEC_CLASS__REINFORCE_THRONE, udwCostTime, 0);
        if (ptbRallyReinforce == NULL)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -11;
        }

        if (dwSide == EN_RALLY_SIDE_ATTACK)
        {
            ptbRallyReinforce->Set_Tid(ptbRallyWar->m_nId);
            CActionBase::HoldSlot(ptbRallyWar, ptbRallyReinforce);
            CActionBase::UpdateRallyForce(ptbRallyWar, ptbRallyReinforce, FALSE);
            pstUser->m_aucMarchFlag[dwRallyWarIndex] = EN_TABLE_UPDT_FLAG__CHANGE;
        }

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    return 0;
}

TINT32 CProcessMarch::ProcessCmd_ThroneDispatch(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;
    TbMap *ptbTmpMap = &pstSession->m_tbTmpMap;

    // 0. 输入参数
    TUINT32 udwCostTime = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TUINT32 udwTargetPos = strtoul(pstSession->m_stReqParam.m_szKey[1], NULL, 10);
    TINT32 dwKnightId = atoi(pstSession->m_stReqParam.m_szKey[2]);
    TBOOL bIsDragonJoin = atoi(pstSession->m_stReqParam.m_szKey[3]);
    TCHAR *pszTroop = pstSession->m_stReqParam.m_szKey[4];

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        dwRetCode = CheckMarchSelfCondition(pstUser, ptbPlayer, pstCity, pszTroop, "", "", 0, dwKnightId, bIsDragonJoin,
            TRUE, EN_ACTION_SEC_CLASS__ASSIGN_THRONE);
        if (dwRetCode != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = dwRetCode;
            return -1;
        }

        if (udwTargetPos == 0 || udwTargetPos != pstSession->m_tbThrone.m_nPos)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -2;
        }

        pstSession->ResetAwsInfo();
        CAwsRequest::MapGet(pstSession, udwTargetPos);

        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AssignHero:: send aws query req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -4;
        }
        return 0;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        pstSession->m_tbTmpMap.Reset();
        dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[0], &pstSession->m_tbTmpMap);
        if (dwRetCode < 0 || pstSession->m_tbTmpMap.m_nId != udwTargetPos)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AssignHero:: get map failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -5;
        }

        dwRetCode = CheckMarchOtherCondition(pstUser, ptbPlayer, ptbTmpMap, EN_ACTION_SEC_CLASS__ASSIGN_THRONE);
        if (dwRetCode != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = dwRetCode;
            return -11;
        }

        TUINT32 udwTroopNum = EN_TROOP_TYPE__END;
        TINT64 addwTroop[EN_TROOP_TYPE__END] = { 0 };
        CCommonFunc::GetArrayFromString(pszTroop, ':', addwTroop, udwTroopNum);
        TINT32 dwTroopNum = CToolBase::GetTroopSumNum(addwTroop);
        if (CCommonBase::GetGameBasicVal(EN_GAME_BASIC_RALLY_SLOT_LIMIT) <= pstSession->m_tbThrone.m_nReinforce_num
            || CCommonBase::GetGameBasicVal(EN_GAME_BASIC_THRONE_TROOP_NUM_LIMIT) < pstSession->m_tbThrone.m_nReinforce_troop_num + dwTroopNum)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REINFORCE_ARRIVE_LIMIT;
            return -6;
        }

        GenMarchAction(pstUser, ptbPlayer, pstCity, pszTroop, NULL, NULL, dwKnightId, bIsDragonJoin, ptbTmpMap, EN_ACTION_SEC_CLASS__ASSIGN_THRONE, udwCostTime);
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    return 0;
}

TINT32 CProcessMarch::ProcessCmd_ThroneDismissKnightDragon(SSession *pstSession, TBOOL &bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TINT32 dwActionIdx = -1;
    TbMarch_action *ptbMarch = NULL;

    // 0. 请求参数
    TINT64 uddwActionId = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 10);

    if (pstUser->m_tbPlayer.m_nAlpos != EN_ALLIANCE_POS__CHANCELLOR
        || pstUser->m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET != pstSession->m_tbThrone.m_nAlid)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ThroneDismissKnightDragon: throne_alid=%ld alid=%ld alpos=%ld [seq=%u]", 
            pstSession->m_tbThrone.m_nAlid, pstUser->m_tbPlayer.m_nAlid, pstUser->m_tbPlayer.m_nAlpos, pstSession->m_stUserInfo.m_udwBSeqNo));
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__HAVE_NO_PERMISSION;
        return -1;
    }

    dwActionIdx = CActionBase::GetMarchIndex(pstUser->m_atbMarch, pstUser->m_udwMarchNum, uddwActionId);
    if (dwActionIdx < 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ThroneDismissKnightDragon: march id invalid [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    ptbMarch = &pstUser->m_atbMarch[dwActionIdx];

    if (ptbMarch->m_nMclass == EN_ACTION_MAIN_CLASS__MARCH && ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__ASSIGN_THRONE
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
        pstUser->m_aucMarchFlag[dwActionIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessMarch::ProcessCmd_ThroneDismissReinforce(SSession *pstSession, TBOOL &bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TINT32 dwActionIdx = -1;
    TbMarch_action *ptbMarch = NULL;

    // 0. 请求参数
    TINT64 uddwActionId = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 10);

    if (pstUser->m_tbPlayer.m_nAlpos != EN_ALLIANCE_POS__CHANCELLOR
        || pstUser->m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET != pstSession->m_tbThrone.m_nAlid)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ThroneDismissKnightDragon: throne_alid=%ld alid=%ld alpos=%ld [seq=%u]",
            pstSession->m_tbThrone.m_nAlid, pstUser->m_tbPlayer.m_nAlid, pstUser->m_tbPlayer.m_nAlpos, pstSession->m_stUserInfo.m_udwBSeqNo));
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__HAVE_NO_PERMISSION;
        return -1;
    }

    dwActionIdx = CActionBase::GetMarchIndex(pstUser->m_atbMarch, pstUser->m_udwMarchNum, uddwActionId);
    if (dwActionIdx < 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ThroneDismissKnightDragon: march id invalid [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    ptbMarch = &pstUser->m_atbMarch[dwActionIdx];

    if (ptbMarch->m_nMclass == EN_ACTION_MAIN_CLASS__MARCH && ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_THRONE
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
        pstUser->m_aucMarchFlag[dwActionIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessMarch::ProcessCmd_ThroneDismissAll(SSession *pstSession, TBOOL &bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;

    if (pstUser->m_tbPlayer.m_nAlpos != EN_ALLIANCE_POS__CHANCELLOR
        || pstUser->m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET != pstSession->m_tbThrone.m_nAlid)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ThroneDismissKnightDragon: throne_alid=%ld alid=%ld alpos=%ld [seq=%u]",
            pstSession->m_tbThrone.m_nAlid, pstUser->m_tbPlayer.m_nAlid, pstUser->m_tbPlayer.m_nAlpos, pstSession->m_stUserInfo.m_udwBSeqNo));
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__HAVE_NO_PERMISSION;
        return -1;
    }

    TbMarch_action *ptbMarch = NULL;
    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; udwIdx++)
    {
        ptbMarch = &pstUser->m_atbMarch[udwIdx];

        if (ptbMarch->m_nMclass == EN_ACTION_MAIN_CLASS__MARCH
            && (ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_THRONE
            || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__ASSIGN_THRONE)
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
            pstUser->m_aucMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
        }
    }
    
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessMarch::ProcessCmd_ThroneRecallKnightDragon(SSession *pstSession, TBOOL &bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TINT32 dwActionIdx = -1;
    TbMarch_action *ptbMarch = NULL;

    // 0. 请求参数
    TINT64 uddwActionId = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 10);

    dwActionIdx = CActionBase::GetMarchIndex(pstUser->m_atbMarch, pstUser->m_udwMarchNum, uddwActionId);
    if (dwActionIdx < 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ThroneDismissKnightDragon: march id invalid [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    ptbMarch = &pstUser->m_atbMarch[dwActionIdx];

    if (ptbMarch->m_nSuid != pstUser->m_tbPlayer.m_nUid)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }

    if (ptbMarch->m_nMclass == EN_ACTION_MAIN_CLASS__MARCH && ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__ASSIGN_THRONE
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
        pstUser->m_aucMarchFlag[dwActionIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessMarch::ProcessCmd_ThroneRecallReinforce(SSession *pstSession, TBOOL &bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TINT32 dwActionIdx = -1;
    TbMarch_action *ptbMarch = NULL;

    // 0. 请求参数
    TINT64 uddwActionId = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 10);

    dwActionIdx = CActionBase::GetMarchIndex(pstUser->m_atbMarch, pstUser->m_udwMarchNum, uddwActionId);
    if (dwActionIdx < 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ThroneDismissKnightDragon: march id invalid [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    ptbMarch = &pstUser->m_atbMarch[dwActionIdx];

    if (ptbMarch->m_nSuid != pstUser->m_tbPlayer.m_nUid)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }

    if (ptbMarch->m_nMclass == EN_ACTION_MAIN_CLASS__MARCH && ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_THRONE
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
        pstUser->m_aucMarchFlag[dwActionIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessMarch::ProcessCmd_ThroneReinforceSpeedup(SSession *pstSession, TBOOL &bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;

    TINT64 ddwActionId = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TINT32 dwItemId = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TINT32 dwGemNum = atoi(pstSession->m_stReqParam.m_szKey[2]);

    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();

    TINT32 dwRetCode = 0;
    if (pstUser->m_tbPlayer.m_nAlpos == 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ThroneReinforceSpeedup: not in alliance [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NOT_IN_ALLIANCE;
        return -1;
    }

    if (dwGemNum > 0)
    {
        if (!CPlayerBase::HasEnoughGem(&pstUser->m_tbLogin, dwGemNum))
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ThroneReinforceSpeedup: gem lack [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GEM_LACK;
            return -1;
        }
    }
    else if (dwItemId >= 0)
    {
        if (!CItemBase::HasEnoughItem(&pstUser->m_tbBackpack, dwItemId))
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ThroneReinforceSpeedup: item lack [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ITEM_NOT_ENOUGH;
            return -1;
        }
    }
    else
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ThroneReinforceSpeedup: cost what ? [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }

    SSpGlobalRes stEffectInfo;
    stEffectInfo.Reset();
    if ((dwRetCode = CItemLogic::GetItemreward(dwItemId, &stEffectInfo)) != 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ThroneReinforceSpeedup: get item reward fail [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
        pstSession->m_stCommonResInfo.m_dwRetCode = dwRetCode;
        return -1;
    }
    
    //take effect
    TINT32 dwMarchIdx = CActionBase::GetMarchIndex(pstUser->m_atbMarch, pstUser->m_udwMarchNum, ddwActionId);
    TbMarch_action *ptbMarch = NULL;

    if (dwMarchIdx == -1)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }
    ptbMarch = &pstUser->m_atbMarch[dwMarchIdx];
    pstUser->m_aucMarchFlag[dwMarchIdx] = EN_TABLE_UPDT_FLAG__CHANGE;

    if ((ptbMarch->m_nSclass != EN_ACTION_SEC_CLASS__REINFORCE_THRONE
        && ptbMarch->m_nSclass != EN_ACTION_SEC_CLASS__ASSIGN_THRONE)
        || ptbMarch->m_nStatus != EN_MARCH_STATUS__MARCHING
        || pstSession->m_tbThrone.m_nPos != ptbMarch->m_nTpos)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ThroneReinforceSpeedup: action status[%ld %ld %ld] error [seq=%u]", 
            ptbMarch->m_nSclass, ptbMarch->m_nStatus, ptbMarch->m_nTpos, pstSession->m_stUserInfo.m_udwBSeqNo));
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }

    if (stEffectInfo.aRewardList[0].udwType == EN_GLOBALRES_TYPE_SPEEDUP_PERCENT)
    {
        TUINT32 udwReduceTime = (ptbMarch->m_nEtime - udwCurTime) * stEffectInfo.aRewardList[0].udwNum / 10000;
        ptbMarch->Set_Etime(ptbMarch->m_nEtime - udwReduceTime);
    }
    else
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ThroneReinforceSpeedup: effect type[%u] error [seq=%u]", 
            stEffectInfo.aRewardList[0].udwType, pstSession->m_stUserInfo.m_udwBSeqNo));
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }

    //3 cost item 
    if (dwGemNum > 0)
    {
        CPlayerBase::CostGem(pstUser, dwGemNum);
        pstSession->m_udwGemCost = dwGemNum;
    }
    else if (dwItemId >= 0)
    {
        CItemBase::CostItem(&pstUser->m_tbBackpack, dwItemId);
    }

    //活动积分统计
    CActivitesLogic::ComputeSpeedUpItemScore(pstUser, dwItemId);
    //task count
    CQuestLogic::SetTaskCurValue(pstUser, pstCity, EN_TASK_TYPE_USE_SPEED_UP_ITEMS);

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessMarch::ProcessCmd_ReinforceSpeedup(SSession *pstSession, TBOOL &bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;

    TINT64 ddwActionId = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TINT64 ddwRallyWarId = strtoll(pstSession->m_stReqParam.m_szKey[1], NULL, 10);
    TINT32 dwItemId = atoi(pstSession->m_stReqParam.m_szKey[2]);
    TINT32 dwGemNum = atoi(pstSession->m_stReqParam.m_szKey[3]);

    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();

    TINT32 dwRetCode = 0;

    TINT32 dwRallyWarIndex = -1;
    TbMarch_action *ptbRallyWar = NULL;
    TINT32 dwReinforceIndex = -1;
    TbMarch_action *ptbReinforce = NULL;
    TBOOL bIsSourceAction = FALSE;

    dwRallyWarIndex = CActionBase::GetMarchIndex(pstUser->m_atbMarch, pstUser->m_udwMarchNum, ddwRallyWarId);
    if (dwRallyWarIndex >= 0)
    {
        ptbRallyWar = &pstUser->m_atbMarch[dwRallyWarIndex];
    }
    else
    {
        dwRallyWarIndex = CActionBase::GetMarchIndex(pstUser->m_atbPassiveMarch, pstUser->m_udwPassiveMarchNum, ddwRallyWarId);
        if (dwRallyWarIndex >= 0)
        {
            ptbRallyWar = &pstUser->m_atbPassiveMarch[dwRallyWarIndex];
        }
    }
    if (ptbRallyWar == NULL)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ReinforceSpeedup: cannot find rally war[%ld] [seq=%u]", 
            ddwRallyWarId, pstSession->m_stUserInfo.m_udwBSeqNo));
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        if (dwGemNum > 0)
        {
            if (!CPlayerBase::HasEnoughGem(&pstUser->m_tbLogin, dwGemNum))
            {
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ReinforceSpeedup: gem lack [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GEM_LACK;
                return -1;
            }
        }
        else if (dwItemId >= 0)
        {
            if (!CItemBase::HasEnoughItem(&pstUser->m_tbBackpack, dwItemId))
            {
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ReinforceSpeedup: item lack [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ITEM_NOT_ENOUGH;
                return -1;
            }
        }
        else
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ReinforceSpeedup: cost what? [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -1;
        }

        dwReinforceIndex = CActionBase::GetMarchIndex(pstUser->m_atbMarch, pstUser->m_udwMarchNum, ddwActionId);
        if (dwReinforceIndex >= 0)
        {
            bIsSourceAction = TRUE;
            ptbReinforce = &pstUser->m_atbMarch[dwReinforceIndex];
            pstUser->m_aucMarchFlag[dwReinforceIndex] = EN_TABLE_UPDT_FLAG__CHANGE;
        }
        else
        {
            dwReinforceIndex = CActionBase::GetMarchIndex(pstUser->m_atbPassiveMarch, pstUser->m_udwPassiveMarchNum, ddwActionId);
            if (dwReinforceIndex >= 0)
            {
                ptbReinforce = &pstUser->m_atbPassiveMarch[dwReinforceIndex];
                pstUser->m_aucPassiveMarchFlag[dwReinforceIndex] = EN_TABLE_UPDT_FLAG__CHANGE;
            }
        }

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;

        pstSession->ResetAwsInfo();
        if (ptbReinforce == NULL)
        {
            TbMarch_action tbAction;
            tbAction.Set_Suid(ddwActionId >> 32);
            tbAction.Set_Id(ddwActionId);
            CAwsRequest::GetItem(pstSession, &tbAction, ETbMARCH_OPEN_TYPE_PRIMARY);

            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
            bNeedResponse = TRUE;
            dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
            if (dwRetCode < 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_RallyInfo: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                return -2;
            }
            return 0;
        }
    }
    
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        if (ptbReinforce == NULL)
        {
            if (pstUser->m_udwMarchNum == MAX_USER_MARCH_NUM)
            {
                pstUser->m_udwMarchNum--;
                pstUser->m_atbMarch[pstUser->m_udwMarchNum].Reset();
            }
            dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[0], &pstUser->m_atbMarch[pstUser->m_udwMarchNum]);
            if (dwRetCode > 0 && pstUser->m_atbMarch[pstUser->m_udwMarchNum].m_nId == ddwActionId)
            {
                dwReinforceIndex = pstUser->m_udwMarchNum;
                ptbReinforce = &pstUser->m_atbMarch[pstUser->m_udwMarchNum];
                pstUser->m_aucMarchFlag[dwReinforceIndex] = EN_TABLE_UPDT_FLAG__CHANGE;
                pstUser->m_udwMarchNum++;
            }
        }

        if (ptbReinforce == NULL)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ReinforceSpeedup: cannot find reinforce [%ld] [seq=%u]",
                ddwActionId, pstSession->m_stUserInfo.m_udwBSeqNo));
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -1;
        }

        SSpGlobalRes stEffectInfo;
        stEffectInfo.Reset();
        if ((dwRetCode = CItemLogic::GetItemreward(dwItemId, &stEffectInfo)) != 0)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ReinforceSpeedup: get item reward fail [seq=%u]",
                pstSession->m_stUserInfo.m_udwBSeqNo));
            pstSession->m_stCommonResInfo.m_dwRetCode = dwRetCode;
            return -1;
        }

        if ((ptbReinforce->m_nSclass != EN_ACTION_SEC_CLASS__REINFORCE_THRONE
            && ptbReinforce->m_nSclass != EN_ACTION_SEC_CLASS__RALLY_REINFORCE
            && ptbReinforce->m_nSclass != EN_ACTION_SEC_CLASS__REINFORCE_NORMAL)
            || ptbReinforce->m_nStatus != EN_MARCH_STATUS__MARCHING
            || ptbReinforce->m_bParam[0].m_ddwSourceAlliance != pstUser->m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ReinforceSpeedup: action status1 error[%ld %ld %ld %ld] [seq=%u]",
                ptbReinforce->m_nSclass, ptbReinforce->m_nStatus, 
                ptbReinforce->m_bParam[0].m_ddwSourceAlliance, pstUser->m_tbPlayer.m_nAlid,
                pstSession->m_stUserInfo.m_udwBSeqNo));
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -1;
        }

        if (ptbReinforce->m_nTid != ptbRallyWar->m_nId
            && ptbReinforce->m_nTpos != ptbRallyWar->m_nTpos)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ReinforceSpeedup: action status2 error[%ld %ld] [seq=%u]",
                ptbReinforce->m_nTid, ptbReinforce->m_nTpos, pstSession->m_stUserInfo.m_udwBSeqNo));
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -1;
        }

        if (stEffectInfo.aRewardList[0].udwType == EN_GLOBALRES_TYPE_SPEEDUP_PERCENT)
        {
            TUINT32 udwReduceTime = (ptbReinforce->m_nEtime - udwCurTime) * stEffectInfo.aRewardList[0].udwNum / 10000;
            ptbReinforce->Set_Etime(ptbReinforce->m_nEtime - udwReduceTime);

            //wave@push_data
            pstUser->m_ptbPushMarchAction = ptbReinforce;

            if (bIsSourceAction)
            {
                TINT32 dwActionIdx = CActionBase::GetMarchIndex(pstUser->m_atbPassiveMarch, pstUser->m_udwPassiveMarchNum, ddwActionId);
                if (dwActionIdx >= 0)
                {
                    pstUser->m_atbPassiveMarch[dwActionIdx].m_nEtime = ptbReinforce->m_nEtime;
                }
            }
        }
        else
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ReinforceSpeedup: effect type[%u] error [seq=%u]",
                stEffectInfo.aRewardList[0].udwType, pstSession->m_stUserInfo.m_udwBSeqNo));
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -1;
        }

        //3 cost item 
        if (dwGemNum > 0)
        {
            CPlayerBase::CostGem(pstUser, dwGemNum);
            pstSession->m_udwGemCost = dwGemNum;
        }
        else if (dwItemId >= 0)
        {
            CItemBase::CostItem(&pstUser->m_tbBackpack, dwItemId);
        }

        //活动积分统计
        CActivitesLogic::ComputeSpeedUpItemScore(pstUser, dwItemId);
        //task count
        CQuestLogic::SetTaskCurValue(pstUser, pstCity, EN_TASK_TYPE_USE_SPEED_UP_ITEMS);

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

/************************************************private**********************************************/
TINT32 CProcessMarch::CheckMarchSelfCondition(SUserInfo *pstUser, TbPlayer *ptbPlayer, SCityInfo *pstCity, TCHAR *szTroop, 
    TCHAR *szSendResource, TCHAR *szTaxResource, TINT32 dwGem, TINT32 dwKnightId, TBOOL bIsDragonJoin, TBOOL bNeedBreakPeaceTime, TINT32 dwMarchType)
{
    CGameInfo *poGameInfo = CGameInfo::GetInstance();

    //是否新手保护
    if(bNeedBreakPeaceTime && (ptbPlayer->m_nStatus & EN_CITY_STATUS__NEW_PROTECTION))
    {
        TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchSelfCondition: uid[%ld] cid[%ld] you are under new_player_protection [seq=%u]",
            ptbPlayer->m_nUid, pstCity->m_stTblData.m_nPos, pstUser->m_udwBSeqNo));
        return EN_RET_CODE__YOURSELF_UNDER_PROTECT;
    }

    //军队数量是否足够
    TUINT32 udwTroopNum = EN_TROOP_TYPE__END;
    TINT64 addwTroop[EN_TROOP_TYPE__END] = {0};
    CCommonFunc::GetArrayFromString(szTroop, ':', addwTroop, udwTroopNum);
    if(!CCityBase::HasEnoughTroop(pstCity, 0, addwTroop, EN_TROOP_TYPE__END))
    {
        TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchSelfCondition: uid[%ld] cid[%ld] the troop isn't enough [seq=%u]",
            ptbPlayer->m_nUid, pstCity->m_stTblData.m_nPos, pstUser->m_udwBSeqNo));
        return EN_RET_CODE__TROOP_LACK;
    }

    //资源数量是否足够
    TUINT32 udwResourceNum = EN_RESOURCE_TYPE__END;
    TINT64 addwTotalResource[EN_RESOURCE_TYPE__END] = {0};
    CCommonFunc::GetArrayFromString(szSendResource, ':', addwTotalResource, udwResourceNum);

    udwResourceNum = EN_RESOURCE_TYPE__END;
    TINT64 addwTaxResource[EN_RESOURCE_TYPE__END] = { 0 };
    CCommonFunc::GetArrayFromString(szTaxResource, ':', addwTaxResource, udwResourceNum);

    if (dwMarchType == EN_ACTION_SEC_CLASS__TRANSPORT)
    {
        //TODO 校验税收是否合理
    }

    for (TUINT32 udwIdx = 0; udwIdx < EN_RESOURCE_TYPE__END; udwIdx++)
    {
        addwTotalResource[udwIdx] += addwTaxResource[udwIdx];
    }

    if(!CCityBase::HasEnoughResource(pstCity, addwTotalResource))
    {
        TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchSelfCondition: uid[%ld] cid[%ld] the resource isn't enough [seq=%u]",
            ptbPlayer->m_nUid, pstCity->m_stTblData.m_nPos, pstUser->m_udwBSeqNo));
        return EN_RET_CODE__RESOURCE_LACK;
    }

    //Gem数量是否足够
    if(!CPlayerBase::HasEnoughGem(&pstUser->m_tbLogin, dwGem))
    {
        TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchSelfCondition: uid[%ld] cid[%ld] the gem isn't enough [seq=%u]",
            ptbPlayer->m_nUid, pstCity->m_stTblData.m_nPos, pstUser->m_udwBSeqNo));
        return EN_RET_CODE__GEM_LACK;
    }

    //行军数量
    if (dwMarchType == EN_ACTION_SEC_CLASS__TRANSPORT)
    {
        if (pstCity->m_stActionStat.m_ucDoingTransportNum >= pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_TRANSPORT_QUEUE_NUM].m_ddwBuffTotal)
        {
            TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchSelfCondition: uid[%ld] cid[%ld] march slot lack CurMarchNum[%u] MaxTransportNum[%ld] [seq=%u]",
                ptbPlayer->m_nUid, pstCity->m_stTblData.m_nPos, pstCity->m_stActionStat.m_ucDoingTransportNum, 
                pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_TRANSPORT_QUEUE_NUM].m_ddwBuffTotal, pstUser->m_udwBSeqNo));
            return EN_RET_CODE__TRANSPORT_SLOT_LACK;
        }
    }
    else
    {
        if (pstCity->m_stActionStat.m_ucDoingMarchNum >= pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_QUEUE_NUM].m_ddwBuffTotal)
        {
            TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchSelfCondition: uid[%ld] cid[%ld] march slot lack CurMarchNum[%u] MaxMarchNum[%ld] [seq=%u]",
                ptbPlayer->m_nUid, pstCity->m_stTblData.m_nPos, pstCity->m_stActionStat.m_ucDoingMarchNum,
                pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_QUEUE_NUM].m_ddwBuffTotal, pstUser->m_udwBSeqNo));
            return EN_RET_CODE__MARCH_SLOT_LACK;
        }
    }

    //knight状态
    if(dwKnightId >= 0)
    {
        if(pstCity->m_stTblData.m_bKnight[dwKnightId].ddwStatus != EN_KNIGHT_STATUS__NORMAL)
        {
            TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchSelfCondition: uid[%ld] cid[%ld] knight status[%d] [seq=%u]",
                ptbPlayer->m_nUid, pstCity->m_stTblData.m_nPos, pstCity->m_stTblData.m_bKnight[dwKnightId].ddwStatus, pstUser->m_udwBSeqNo));
            return EN_RET_CODE__KNIGHT_UNVAILABEL;
        }
    }    

    //hero状态
    if(bIsDragonJoin && ptbPlayer->m_nDragon_status != EN_DRAGON_STATUS_NORMAL)
    {
        TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchSelfCondition: uid[%ld] cid[%ld] dragon not normarl [status=%ld] [seq=%u]",
            ptbPlayer->m_nUid, pstCity->m_stTblData.m_nPos, ptbPlayer->m_nDragon_status, pstUser->m_udwBSeqNo));
        return EN_RET_CODE__HERO_NOT_IN_CITY;
    }

    // hero attack rally war时 是否有联盟
    if((dwMarchType == EN_ACTION_SEC_CLASS__DRAGON_ATTACK
        || dwMarchType == EN_ACTION_SEC_CLASS__RALLY_WAR
        || dwMarchType == EN_ACTION_SEC_CLASS__RALLY_REINFORCE
        || dwMarchType == EN_ACTION_SEC_CLASS__REINFORCE_NORMAL
        || dwMarchType == EN_ACTION_SEC_CLASS__ATTACK_IDOL
        || dwMarchType == EN_ACTION_SEC_CLASS__ATTACK_THRONE
        || dwMarchType == EN_ACTION_SEC_CLASS__RALLY_WAR_THRONE
        || dwMarchType == EN_ACTION_SEC_CLASS__ASSIGN_THRONE
        || dwMarchType == EN_ACTION_SEC_CLASS__REINFORCE_THRONE)
        && ptbPlayer->m_nAlpos == 0)
    {
        TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchSelfCondition: uid[%ld] is not in alliance [seq=%u]",
            ptbPlayer->m_nUid, pstUser->m_udwBSeqNo));
        return EN_RET_CODE__NOT_IN_ALLIANCE;
    }

    if(bNeedBreakPeaceTime && (ptbPlayer->m_nStatus & EN_CITY_STATUS__AVOID_WAR))
    {
        TSE_LOG_INFO(poGameInfo->m_poLog, ("CheckMarchSelfCondition: uid[%ld] cid[%ld] need to break the peace_time [seq=%u]",
            ptbPlayer->m_nUid, pstCity->m_stTblData.m_nPos, pstUser->m_udwBSeqNo));
        // 修改状态
        ptbPlayer->Set_Status(ptbPlayer->m_nStatus & (~EN_CITY_STATUS__AVOID_WAR));
        // 删除peacetime的action
        TbAction *pstAction = CActionBase::GetActionByBufferId(pstUser->m_atbAction, pstUser->m_udwActionNum, EN_BUFFER_INFO_PEACE_TIME);
        if(pstAction)
        {
            TINT32 dwActionIdx = CActionBase::GetActionIndex(pstUser->m_atbAction, pstUser->m_udwActionNum, pstAction->m_nId);
            if(dwActionIdx >= 0)
            {
                pstUser->m_aucActionFlag[dwActionIdx] = EN_TABLE_UPDT_FLAG__DEL;
            }
        }
    }

    return 0;
}

TINT32 CProcessMarch::CheckMarchOtherCondition(SUserInfo *pstUser, TbPlayer *ptbPlayer, TbMap *ptbTargetMap, TINT32 dwMarchType, TINT64 ddwRallyId/* = 0*/)
{
    CGameInfo *poGameInfo = CGameInfo::GetInstance();

    const Json::Value& oWildResJson = CWildInfo::GetWildResInfo(ptbTargetMap->m_nSid);
    TUINT32 udwMapType = CMapLogic::GetWildClass(ptbTargetMap->m_nSid, ptbTargetMap->m_nType);
    TUINT32 udwCurtime = CTimeUtils::GetUnixTime();

    switch (dwMarchType)
    {
    case EN_ACTION_SEC_CLASS__ATTACK:
        if (ptbTargetMap->m_nUid == 0)
        {
            TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: uid[%ld] target_map[%ld] the target is cannot be attacked[type:%ld] [seq=%u]",
                ptbPlayer->m_nUid, ptbTargetMap->m_nId, ptbTargetMap->m_nType, pstUser->m_udwBSeqNo));
            return EN_RET_CODE__TARGET_LEFT;
        }
        if (ptbTargetMap->m_nUid == ptbPlayer->m_nUid)
        {
            TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: uid[%ld] target_map[%ld] the target is self[uid:%ld] [seq=%u]",
                ptbPlayer->m_nUid, ptbTargetMap->m_nId, ptbTargetMap->m_nUid, pstUser->m_udwBSeqNo));
            return EN_RET_CODE__ATTACK_COORD_INVALID;
        }
        if (ptbTargetMap->m_nAlid != 0 && ptbTargetMap->m_nAlid == ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET && ptbPlayer->m_nAlpos != 0)
        {
            TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: uid[%ld] target_map[%ld] the target is in the same alliance[aid:%ld] [seq=%u]",
                ptbPlayer->m_nUid, ptbTargetMap->m_nId, ptbTargetMap->m_nAlid, pstUser->m_udwBSeqNo));
            return EN_RET_CODE__CANNOT_ATTACT_FRIEND;
        }
        if (CDiplomacyProcess::CheckDiplomacy(pstUser->m_atbDiplomacy,
            pstUser->m_udwDiplomacyNum, ptbTargetMap->m_nAlid) != 0)
        {
            TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: uid[%ld] target_map[%ld] the target is in the frend alliance[aid:%ld] [seq=%u]",
                ptbPlayer->m_nUid, ptbTargetMap->m_nId, ptbTargetMap->m_nAlid, pstUser->m_udwBSeqNo));
            return EN_RET_CODE__CANNOT_ATTACT_FRIEND;
        }
        if ((ptbTargetMap->m_nStatus & EN_CITY_STATUS__AVOID_WAR) || (ptbTargetMap->m_nStatus & EN_CITY_STATUS__NEW_PROTECTION) ||
            ptbTargetMap->m_nTime_end >= CTimeUtils::GetUnixTime())
        {
            TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: uid[%ld] target_map[%ld] the target is under protection [seq=%u]",
                ptbPlayer->m_nUid, ptbTargetMap->m_nId, pstUser->m_udwBSeqNo));
            return EN_RET_CODE__CITY_UNDER_PROTECT;
        }
        if (ptbTargetMap->m_nType == EN_WILD_TYPE__THRONE_NEW || ptbTargetMap->m_nType == EN_WILD_TYPE__IDOL)
        {
            TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: uid[%ld] target_map[%ld] type[%ld] [seq=%u]",
                ptbPlayer->m_nUid, ptbTargetMap->m_nId, ptbTargetMap->m_nType, pstUser->m_udwBSeqNo));
            return EN_RET_CODE__REQ_PARAM_ERROR;
        }
        break;
    case EN_ACTION_SEC_CLASS__SCOUT:
        if (ptbTargetMap->m_nUid == 0 && ptbTargetMap->m_nType != EN_WILD_TYPE__THRONE_NEW && ptbTargetMap->m_nType != EN_WILD_TYPE__IDOL)
        {
            TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: uid[%ld] target_map[%ld] the target is cannot be attacked[type:%ld] [seq=%u]",
                ptbPlayer->m_nUid, ptbTargetMap->m_nId, ptbTargetMap->m_nType, pstUser->m_udwBSeqNo));
            return EN_RET_CODE__ATTACK_COORD_INVALID;
        }
        if (ptbTargetMap->m_nUid == ptbPlayer->m_nUid)
        {
            TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: uid[%ld] target_map[%ld] the target is self[uid:%ld] [seq=%u]",
                ptbPlayer->m_nUid, ptbTargetMap->m_nId, ptbTargetMap->m_nUid, pstUser->m_udwBSeqNo));
            return EN_RET_CODE__ATTACK_COORD_INVALID;
        }
        if (ptbTargetMap->m_nAlid != 0 && ptbTargetMap->m_nAlid == ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET && ptbPlayer->m_nAlpos != 0)
        {
            TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: uid[%ld] target_map[%ld] the target is in the same alliance[aid:%ld] [seq=%u]",
                ptbPlayer->m_nUid, ptbTargetMap->m_nId, ptbTargetMap->m_nAlid, pstUser->m_udwBSeqNo));
            return EN_RET_CODE__CANNOT_ATTACT_FRIEND;
        }
        if (CDiplomacyProcess::CheckDiplomacy(pstUser->m_atbDiplomacy,
            pstUser->m_udwDiplomacyNum, ptbTargetMap->m_nAlid) != 0)
        {
            TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: uid[%ld] target_map[%ld] the target is in the frend alliance[aid:%ld] [seq=%u]",
                ptbPlayer->m_nUid, ptbTargetMap->m_nId, ptbTargetMap->m_nAlid, pstUser->m_udwBSeqNo));
            return EN_RET_CODE__CANNOT_ATTACT_FRIEND;
        }
        if (ptbTargetMap->m_nType != EN_WILD_TYPE__THRONE_NEW && ptbTargetMap->m_nType != EN_WILD_TYPE__IDOL
            && ((ptbTargetMap->m_nStatus & EN_CITY_STATUS__AVOID_WAR) || (ptbTargetMap->m_nStatus & EN_CITY_STATUS__NEW_PROTECTION)))
        {
            TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: uid[%ld] target_map[%ld] the target is under protection [seq=%u]",
                ptbPlayer->m_nUid, ptbTargetMap->m_nId, pstUser->m_udwBSeqNo));
            return EN_RET_CODE__CITY_UNDER_PROTECT;
        }
        if (ptbTargetMap->m_nType == EN_WILD_TYPE__THRONE_NEW && ptbTargetMap->m_nStatus == EN_THRONE_STATUS__PEACE_TIME)
        {
            TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: uid[%ld] target_map[%ld] the target is under protection [seq=%u]",
                ptbPlayer->m_nUid, ptbTargetMap->m_nId, pstUser->m_udwBSeqNo));
            return EN_RET_CODE__CITY_UNDER_PROTECT;
        }
        if (ptbTargetMap->m_nType == EN_WILD_TYPE__IDOL && ptbTargetMap->m_nStatus != EN_IDOL_STATUS__CONTEST_PERIOD)
        {
            TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: uid[%ld] target_map[%ld] the target is under protection [seq=%u]",
                ptbPlayer->m_nUid, ptbTargetMap->m_nId, pstUser->m_udwBSeqNo));
            return EN_RET_CODE__CITY_UNDER_PROTECT;
        }
        if ((ptbTargetMap->m_nType == EN_WILD_TYPE__THRONE_NEW && ptbTargetMap->m_nType == EN_WILD_TYPE__IDOL)
            && ptbPlayer->m_nAlpos == 0)
        {
            TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: target_type[%ld] player_alid[%ld] [seq=%u]",
                ptbTargetMap->m_nType, ptbPlayer->m_nAlid, pstUser->m_udwBSeqNo));
            return EN_RET_CODE__NEED_TO_JOIN_ALLIANCE;
        }
        break;
    case EN_ACTION_SEC_CLASS__TRANSPORT:
        if(ptbTargetMap->m_nType != EN_WILD_TYPE__CITY)
        {
            TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: uid[%ld] target_map[%ld] the target is not a city[type:%ld] [seq=%u]",
                ptbPlayer->m_nUid, ptbTargetMap->m_nId, ptbTargetMap->m_nType, pstUser->m_udwBSeqNo));
            return EN_RET_CODE__OVER_MAX_MARCH_DISTACE;
        }
        if(ptbPlayer->m_nAlpos == 0 || ptbTargetMap->m_nAlid == 0 || ptbTargetMap->m_nAlid != ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET)
        {
            TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: uid[%ld] target_map[%ld] the target is not your ally[aid:%ld] [seq=%u]",
                ptbPlayer->m_nUid, ptbTargetMap->m_nId, ptbTargetMap->m_nAlid, pstUser->m_udwBSeqNo));
            return EN_RET_CODE__TARGET_NOT_YOUR_ALLY;
        }
        break;
    case EN_ACTION_SEC_CLASS__REINFORCE_NORMAL:
        if (ptbTargetMap->m_nType != EN_WILD_TYPE__CITY)
        {
            TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: uid[%ld] target_map[%ld] the target is not city, cannot be reinforced[type:%ld] [seq=%u]",
                ptbPlayer->m_nUid, ptbTargetMap->m_nId, ptbTargetMap->m_nType, pstUser->m_udwBSeqNo));
            return EN_RET_CODE__OVER_MAX_MARCH_DISTACE;
        }
        else if (ptbTargetMap->m_jCity_info.isMember("em_lv") && ptbTargetMap->m_jCity_info["em_lv"].asInt() == 0)
        {
            TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: target_map[%ld] city_info->em_lv[%ld] no embassy [seq=%u]",
                ptbTargetMap->m_nId, ptbTargetMap->m_jCity_info["em_lv"].asInt(), pstUser->m_udwBSeqNo));
            return EN_RET_CODE__TARGET_CONSULATE_NOT_EXIST;
        }
        if(ptbPlayer->m_nAlpos == 0 || ptbTargetMap->m_nAlid == 0 || ptbTargetMap->m_nAlid != ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET)
        {
            TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: uid[%ld] target_map[%ld] the target is not your ally[aid:%ld] [seq=%u]",
                ptbPlayer->m_nUid, ptbTargetMap->m_nId, ptbTargetMap->m_nAlid, pstUser->m_udwBSeqNo));
            return EN_RET_CODE__TARGET_NOT_YOUR_ALLY;
        }
        for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
        {
            TbMarch_action *ptbAction = &pstUser->m_atbMarch[udwIdx];
            if (ptbAction->m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_NORMAL && ptbAction->m_nStatus != EN_MARCH_STATUS__RETURNING &&
                ptbAction->m_nSuid == ptbPlayer->Get_Uid() && ptbAction->m_nTuid == ptbTargetMap->m_nUid)
            {
                return EN_RET_CODE__REINFORCE_SAME_PEOPLE;
            }
        }
        break;
    case EN_ACTION_SEC_CLASS__RALLY_REINFORCE:
        if (ptbTargetMap->m_nType != EN_WILD_TYPE__CITY)
        {
            TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: uid[%ld] target_map[%ld] the target is not city, cannot be reinforced[type:%ld] [seq=%u]",
                ptbPlayer->m_nUid, ptbTargetMap->m_nId, ptbTargetMap->m_nType, pstUser->m_udwBSeqNo));
            return EN_RET_CODE__REQ_PARAM_ERROR;
        }
        else if (ptbTargetMap->m_jCity_info.isMember("em_lv") && ptbTargetMap->m_jCity_info["em_lv"].asInt() == 0)
        {
            TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: target_map[%ld] city_info->em_lv[%ld] no embassy [seq=%u]",
                ptbTargetMap->m_nId, ptbTargetMap->m_jCity_info["em_lv"].asInt(), pstUser->m_udwBSeqNo));
            return EN_RET_CODE__TARGET_CONSULATE_NOT_EXIST;
        }
        if (ptbPlayer->m_nAlpos == 0 || ptbTargetMap->m_nAlid == 0 || ptbTargetMap->m_nAlid != ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET)
        {
            TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: uid[%ld] target_map[%ld] the target is not your ally[aid:%ld] [seq=%u]",
                ptbPlayer->m_nUid, ptbTargetMap->m_nId, ptbTargetMap->m_nAlid, pstUser->m_udwBSeqNo));
            return EN_RET_CODE__TARGET_NOT_YOUR_ALLY;
        }
        for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
        {
            TbMarch_action *ptbAction = &pstUser->m_atbMarch[udwIdx];
            if (ptbAction->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_REINFORCE && ptbAction->m_nStatus != EN_MARCH_STATUS__RETURNING 
                && ptbAction->m_nTid == ddwRallyId && ptbAction->m_nSuid == ptbPlayer->Get_Uid() && ptbAction->m_nTuid == ptbTargetMap->m_nUid)
            {
                return EN_RET_CODE__REINFORCE_SAME_PEOPLE;
            }
        }
        break;
    case EN_ACTION_SEC_CLASS__OCCUPY:
        if (EN_WILD_CLASS_RES == udwMapType)
        {
            if (0 != ptbTargetMap->m_nUid)
            {
                TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: uid[%ld] target_map[%ld] the target has occupied by uid[%ld] [seq=%u]",
                    ptbPlayer->m_nUid, ptbTargetMap->m_nId, ptbTargetMap->m_nUid, pstUser->m_udwBSeqNo));
                return EN_RET_CODE__OCCUPY_COORD_INVALID;
            }
            if (!CMapLogic::IsOccupyWild(ptbTargetMap->m_nSid, ptbTargetMap->m_nType))
            {
                TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: uid[%ld] target_map[%ld] the target is not a occupy_wild[type:%ld] [seq=%u]",
                    ptbPlayer->m_nUid, ptbTargetMap->m_nId, ptbTargetMap->m_nType, pstUser->m_udwBSeqNo));
                return EN_RET_CODE__OCCUPY_COORD_INVALID;
            }
        }
        else if (EN_WILD_CLASS_MONSTER_NEST == udwMapType)
        {
            if (0 != ptbTargetMap->m_nUid)
            {
                if (ptbTargetMap->m_nExpire_time > 0 && udwCurtime >= ptbTargetMap->m_nExpire_time)
                {
                    TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: target_map[%ld] reach disappear time[%ld], [seq=%u]",
                        ptbTargetMap->m_nId, ptbTargetMap->m_nExpire_time, pstUser->m_udwBSeqNo));
                    return EN_RET_CODE__OCCUPY_COORD_INVALID;
                }
                if (ptbTargetMap->m_nUid == ptbPlayer->m_nUid)
                {
                    TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: uid[%ld] target_map[%ld] the target is self[uid:%ld] [seq=%u]",
                        ptbPlayer->m_nUid, ptbTargetMap->m_nId, ptbTargetMap->m_nUid, pstUser->m_udwBSeqNo));
                    return EN_RET_CODE__OCCUPY_COORD_INVALID;
                }
                if (pstUser->m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET != ptbTargetMap->m_nAlid)
                {
                    TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: uid[%ld] target_map[%ld] alid not match user_alid[%ld] map_alid[%ld] [seq=%u]",
                        ptbPlayer->m_nUid, ptbTargetMap->m_nId, pstUser->m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET, ptbTargetMap->m_nAlid, pstUser->m_udwBSeqNo));
                    return EN_RET_CODE__OCCUPY_COORD_INVALID;
                }
                if (ptbTargetMap->m_nOccupy_num >= oWildResJson[CCommonFunc::NumToString(ptbTargetMap->m_nType)]["a0"]["a11"][(TUINT32)ptbTargetMap->m_nLevel - 1].asUInt())
                {
                    TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: uid[%ld] target_map[%ld] occupy user num exceed occupy_num[%ld] occupy_limit[%u] [seq=%u]",
                        ptbPlayer->m_nUid, ptbTargetMap->m_nId, ptbTargetMap->m_nOccupy_num,
                        oWildResJson[CCommonFunc::NumToString(ptbTargetMap->m_nType)]["a0"]["a11"][(TUINT32)ptbTargetMap->m_nLevel-1].asUInt(), pstUser->m_udwBSeqNo));
                    return EN_RET_CODE__OCCUPY_COORD_INVALID;
                }
            }
        }
        else
        {
            TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: uid[%ld] target_map[%ld] the target map type[%u] cannot be occupied [seq=%u]",
                ptbPlayer->m_nUid, ptbTargetMap->m_nId, udwMapType, pstUser->m_udwBSeqNo));
            return EN_RET_CODE__OCCUPY_COORD_INVALID;
        }
        break;
    case EN_ACTION_SEC_CLASS__CAMP:
        if (ptbTargetMap->m_nUid != 0)
        {
            TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: uid[%ld] target_map[%ld] the target has occupied by uid[%ld] [seq=%u]",
                ptbPlayer->m_nUid, ptbTargetMap->m_nId, ptbTargetMap->m_nUid, pstUser->m_udwBSeqNo));
            return EN_RET_CODE__CAMP_COORD_INVALID;
        }
        if (ptbTargetMap->m_nType != EN_WILD_TYPE__NORMAL)
        {
            TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: target map[%ld] is not normal_type[type:%ld] [seq=%u]",
                ptbTargetMap->m_nId, ptbTargetMap->m_nType, pstUser->m_udwBSeqNo));
            return EN_RET_CODE__CAMP_COORD_INVALID;
        }
        break;
    case EN_ACTION_SEC_CLASS__DRAGON_ATTACK:
        if (!CMapLogic::IsDragonAttackWild(ptbTargetMap->m_nSid, ptbTargetMap->m_nType))
        {
            TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: uid[%ld] target_map[%ld] the target cannot be attacked by hero[type:%ld] [seq=%u]",
                ptbPlayer->m_nUid, ptbTargetMap->m_nId, ptbTargetMap->m_nType, pstUser->m_udwBSeqNo));
            return EN_RET_CODE__ATTACK_COORD_INVALID;
        }
        break;
    case EN_ACTION_SEC_CLASS__RALLY_WAR:
        if(ptbTargetMap->m_nType == EN_WILD_TYPE__NORMAL)
        {
            TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: uid[%ld] target_map[%ld] the target is cannot be rallywared[type:%ld] [seq=%u]",
                ptbPlayer->m_nUid, ptbTargetMap->m_nId, ptbTargetMap->m_nType, pstUser->m_udwBSeqNo));
            return EN_RET_CODE__TARGET_LEFT;
        }
        else if(ptbTargetMap->m_nType == EN_WILD_TYPE__CITY)
        {
            if(ptbTargetMap->m_nLevel < CCommonBase::GetGameBasicVal(EN_GAME_BASIC_RALLY_TARGET_LV_LIMIT))
            {
                TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: uid[%ld] target_map[%ld] the target param is invalid [type:%ld] [castle_lv:%ld] [seq=%u]",
                    ptbPlayer->m_nUid, ptbTargetMap->m_nId, ptbTargetMap->m_nType, ptbTargetMap->m_nLevel, pstUser->m_udwBSeqNo));
                return EN_RET_CODE__ATTACK_COORD_INVALID;
            }
            if(ptbTargetMap->m_nAlid != 0 && ptbTargetMap->m_nAlid == ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET && ptbPlayer->m_nAlpos != 0)
            {
                TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: uid[%ld] target_map[%ld] the target is in the same alliance[aid:%ld] [seq=%u]",
                    ptbPlayer->m_nUid, ptbTargetMap->m_nId, ptbTargetMap->m_nAlid, pstUser->m_udwBSeqNo));
                return EN_RET_CODE__CANNOT_ATTACT_FRIEND;
            }
            if(CDiplomacyProcess::CheckDiplomacy(pstUser->m_atbDiplomacy,
                pstUser->m_udwDiplomacyNum, ptbTargetMap->m_nAlid) != 0)
            {
                TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: uid[%ld] target_map[%ld] the target is in the frend alliance[aid:%ld] [seq=%u]",
                    ptbPlayer->m_nUid, ptbTargetMap->m_nId, ptbTargetMap->m_nAlid, pstUser->m_udwBSeqNo));
                return EN_RET_CODE__CANNOT_ATTACT_FRIEND;
            }
            if((ptbTargetMap->m_nStatus & EN_CITY_STATUS__AVOID_WAR) || (ptbTargetMap->m_nStatus & EN_CITY_STATUS__NEW_PROTECTION) ||
                ptbTargetMap->m_nTime_end >= CTimeUtils::GetUnixTime())
            {
                TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: uid[%ld] target_map[%ld] the target is under protection [seq=%u]",
                    ptbPlayer->m_nUid, ptbTargetMap->m_nId, pstUser->m_udwBSeqNo));
                return EN_RET_CODE__CITY_UNDER_PROTECT;
            }
            if(ptbPlayer->m_nAlpos == 0)
            {
                TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: uid[%ld] target_map[%ld] the target need alliance[seq=%u]",
                    ptbPlayer->m_nUid, ptbTargetMap->m_nId, pstUser->m_udwBSeqNo));
                return EN_RET_CODE__NEED_TO_JOIN_ALLIANCE;
            }
        }
        else
        {
            TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: uid[%ld] target_map[%ld] the target param is invalid [type:%ld] [castle_lv:%ld] [seq=%u]",
                ptbPlayer->m_nUid, ptbTargetMap->m_nId, ptbTargetMap->m_nType, ptbTargetMap->m_nLevel, pstUser->m_udwBSeqNo));
            return EN_RET_CODE__ATTACK_COORD_INVALID;
        }
        break;
    case EN_ACTION_SEC_CLASS__ATTACK_IDOL:
        if (ptbTargetMap->m_nType != EN_WILD_TYPE__IDOL)
        {
            TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: uid[%ld] target_map[%ld] type[%ld] [seq=%u]",
                ptbPlayer->m_nUid, ptbTargetMap->m_nId, ptbTargetMap->m_nType, pstUser->m_udwBSeqNo));
            return EN_RET_CODE__REQ_PARAM_ERROR;
        }
        if (ptbPlayer->m_nAlpos == 0)
        {
            TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: uid[%ld] target_map[%ld] type[%ld] the target need alliance[seq=%u]",
                ptbPlayer->m_nUid, ptbTargetMap->m_nId, ptbTargetMap->m_nType, pstUser->m_udwBSeqNo));
            return EN_RET_CODE__NEED_TO_JOIN_ALLIANCE;
        }
        if (ptbTargetMap->m_nStatus != EN_IDOL_STATUS__CONTEST_PERIOD)
        {
            TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: uid[%ld] target_map[%ld] type[%ld] status[%ld] [seq=%u]",
                ptbPlayer->m_nUid, ptbTargetMap->m_nId, ptbTargetMap->m_nType, ptbTargetMap->m_nStatus, pstUser->m_udwBSeqNo));
            return EN_RET_CODE__CITY_UNDER_PROTECT;
        }
        break;
    case EN_ACTION_SEC_CLASS__ATTACK_THRONE:
    case EN_ACTION_SEC_CLASS__RALLY_WAR_THRONE:
        if (ptbTargetMap->m_nType != EN_WILD_TYPE__THRONE_NEW)
        {
            TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: uid[%ld] target_map[%ld] type[%ld] [seq=%u]",
                ptbPlayer->m_nUid, ptbTargetMap->m_nId, ptbTargetMap->m_nType, pstUser->m_udwBSeqNo));
            return EN_RET_CODE__REQ_PARAM_ERROR;
        }
        if (ptbPlayer->m_nAlpos == 0)
        {
            TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: uid[%ld] target_map[%ld] type[%ld] the target need alliance[seq=%u]",
                ptbPlayer->m_nUid, ptbTargetMap->m_nId, ptbTargetMap->m_nType, pstUser->m_udwBSeqNo));
            return EN_RET_CODE__NEED_TO_JOIN_ALLIANCE;
        }
        if (ptbPlayer->m_nAlpos != 0 && ptbTargetMap->m_nAlid == ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET)
        {
            TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: uid[%ld] target_map[%ld] the target is in the same alliance[aid:%ld] [seq=%u]",
                ptbPlayer->m_nUid, ptbTargetMap->m_nId, ptbTargetMap->m_nAlid, pstUser->m_udwBSeqNo));
            return EN_RET_CODE__CANNOT_ATTACT_FRIEND;
        }
        if (ptbTargetMap->m_nStatus == EN_THRONE_STATUS__PEACE_TIME)
        {
            TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: uid[%ld] target_map[%ld] type[%ld] status[%ld] [seq=%u]",
                ptbPlayer->m_nUid, ptbTargetMap->m_nId, ptbTargetMap->m_nType, ptbTargetMap->m_nStatus, pstUser->m_udwBSeqNo));
            return EN_RET_CODE__CITY_UNDER_PROTECT;
        }
        break;
    case EN_ACTION_SEC_CLASS__REINFORCE_THRONE:
//         if (ddwRallyId > 0)
//         {
//             if (ptbTargetMap->m_nType != EN_WILD_TYPE__CITY)
//             {
//                 TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: uid[%ld] target_map[%ld] the target is not city, cannot be reinforced[type:%ld] [seq=%u]",
//                     ptbPlayer->m_nUid, ptbTargetMap->m_nId, ptbTargetMap->m_nType, pstUser->m_udwBSeqNo));
//                 return EN_RET_CODE__REQ_PARAM_ERROR;
//             }
//             else if (ptbTargetMap->m_jCity_info.isMember("em_lv") && ptbTargetMap->m_jCity_info["em_lv"].asInt() == 0)
//             {
//                 TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: target_map[%ld] city_info->em_lv[%ld] no embassy [seq=%u]",
//                     ptbTargetMap->m_nId, ptbTargetMap->m_jCity_info["em_lv"].asInt(), pstUser->m_udwBSeqNo));
//                 return EN_RET_CODE__TARGET_CONSULATE_NOT_EXIST;
//             }
//             if (ptbPlayer->m_nAlpos == 0 || ptbTargetMap->m_nAlid == 0 || ptbTargetMap->m_nAlid != ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET)
//             {
//                 TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: uid[%ld] target_map[%ld] the target is not your ally[aid:%ld] [seq=%u]",
//                     ptbPlayer->m_nUid, ptbTargetMap->m_nId, ptbTargetMap->m_nAlid, pstUser->m_udwBSeqNo));
//                 return EN_RET_CODE__TARGET_NOT_YOUR_ALLY;
//             }
//             for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
//             {
//                 TbMarch_action *ptbAction = &pstUser->m_atbMarch[udwIdx];
//                 if (ptbAction->m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_THRONE && ptbAction->m_nStatus != EN_MARCH_STATUS__RETURNING
//                     && ptbAction->m_nTid == ddwRallyId && ptbAction->m_nSuid == ptbPlayer->m_nUid && ptbAction->m_nTuid == ptbTargetMap->m_nUid)
//                 {
//                     return EN_RET_CODE__REINFORCE_SAME_PEOPLE;
//                 }
//             }
//         }
//         else
//         {
            if (ptbTargetMap->m_nType != EN_WILD_TYPE__THRONE_NEW)
            {
                TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: uid[%ld] target_map[%ld] the target is not throne, cannot be reinforced[type:%ld] [seq=%u]",
                    ptbPlayer->m_nUid, ptbTargetMap->m_nId, ptbTargetMap->m_nType, pstUser->m_udwBSeqNo));
                return EN_RET_CODE__REQ_PARAM_ERROR;
            }
            if (ptbPlayer->m_nAlpos == 0 || ptbTargetMap->m_nAlid != ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET)
            {
                TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: uid[%ld] target_map[%ld] the target is not your ally[aid:%ld] [seq=%u]",
                    ptbPlayer->m_nUid, ptbTargetMap->m_nId, ptbTargetMap->m_nAlid, pstUser->m_udwBSeqNo));
                return EN_RET_CODE__THRONE_NOT_YOUR_ALLY;
            }
//         }
        break;
    case EN_ACTION_SEC_CLASS__ASSIGN_THRONE:
        if (ptbTargetMap->m_nType != EN_WILD_TYPE__THRONE_NEW)
        {
            TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: uid[%ld] target_map[%ld] the target is not throne, cannot be reinforced[type:%ld] [seq=%u]",
                ptbPlayer->m_nUid, ptbTargetMap->m_nId, ptbTargetMap->m_nType, pstUser->m_udwBSeqNo));
            return EN_RET_CODE__REQ_PARAM_ERROR;
        }
        if (ptbPlayer->m_nAlpos == 0 || ptbTargetMap->m_nAlid != ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET)
        {
            TSE_LOG_ERROR(poGameInfo->m_poLog, ("CheckMarchOtherCondition: uid[%ld] target_map[%ld] the target is not your ally[aid:%ld] [seq=%u]",
                ptbPlayer->m_nUid, ptbTargetMap->m_nId, ptbTargetMap->m_nAlid, pstUser->m_udwBSeqNo));
            return EN_RET_CODE__THRONE_NOT_YOUR_ALLY;
        }
        for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
        {
            if (pstUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__ASSIGN_THRONE
                && pstUser->m_atbMarch[udwIdx].m_nStatus != EN_MARCH_STATUS__RETURNING)
            {
                return EN_RET_CODE__REQ_PARAM_ERROR;
            }
        }
        break;
    default:
        break;
    }
    return 0;
}

TbMarch_action* CProcessMarch::GenMarchAction(SUserInfo *pstUser, TbPlayer *ptbPlayer, SCityInfo *pstCity, TCHAR *szTroop,
    TCHAR *szSendResource, TCHAR *szTaxResource, TINT32 dwKnightId, TBOOL bIsDragonJoin, TbMap *ptbTargetMap, 
    TINT32 dwMarchType, TUINT32 udwCostTime, TBOOL bIfMaxAttack)
{
    TbMarch_action* ptbMarchAction = NULL;
    SActionMarchParam stMarchParam;
    stMarchParam.Reset();

    //source user
    stMarchParam.m_ddwSourceUserId = ptbPlayer->m_nUid;
    strncpy(stMarchParam.m_szSourceUserName, ptbPlayer->m_sUin.c_str(), MAX_TABLE_NAME_LEN);
    stMarchParam.m_ddwSourceCityId = pstCity->m_stTblData.m_nPos;
    strncpy(stMarchParam.m_szSourceCityName, pstCity->m_stTblData.m_sName.c_str(), MAX_TABLE_NAME_LEN);
    stMarchParam.m_ddwSourceAlliance = ptbPlayer->m_nAlpos ? ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET : 0;
    if(stMarchParam.m_ddwSourceAlliance)
    {
        strncpy(stMarchParam.m_szSourceAlliance, ptbPlayer->m_sAlname.c_str(), MAX_TABLE_NAME_LEN);
        strncpy(stMarchParam.m_szSourceAlNick, ptbPlayer->m_sAl_nick_name.c_str(), MAX_TABLE_NAME_LEN);
    }

    //target user
    if (ptbTargetMap->m_nType != EN_WILD_TYPE__THRONE_NEW)
    {
        stMarchParam.m_ddwTargetUserId = ptbTargetMap->m_nUid;
        strncpy(stMarchParam.m_szTargetUserName, ptbTargetMap->m_sUname.c_str(), MAX_TABLE_NAME_LEN);
        if (ptbTargetMap->m_nType == EN_WILD_TYPE__CITY)
        {
            stMarchParam.m_ddwTargetCityId = ptbTargetMap->m_nId;
        }
        else
        {
            stMarchParam.m_ddwTargetCityId = ptbTargetMap->m_nCid;
        }
        strncpy(stMarchParam.m_szTargetCityName, ptbTargetMap->m_sName.c_str(), MAX_TABLE_NAME_LEN);
        stMarchParam.m_ddwTargetAlliance = ptbTargetMap->m_nAlid;
        strncpy(stMarchParam.m_szTargetAlliance, ptbTargetMap->m_sAlname.c_str(), MAX_TABLE_NAME_LEN);
        strncpy(stMarchParam.m_szTargetAlNick, ptbTargetMap->m_sAl_nick.c_str(), MAX_TABLE_NAME_LEN);
    }
    stMarchParam.m_ddwTargetType = ptbTargetMap->m_nType;
    stMarchParam.m_ddwTargetLevel = ptbTargetMap->m_nLevel;

    //march time
    stMarchParam.m_ddwMarchingTime = udwCostTime;

    //knight
    stMarchParam.m_stKnight.ddwId = dwKnightId;
    if(dwKnightId >= 0)
    {
        stMarchParam.m_stKnight.ddwLevel = CGameInfo::GetInstance()->ComputeKnightLevelByExp(pstCity->m_stTblData.m_bKnight[dwKnightId].ddwExp);
        stMarchParam.m_stKnight.ddwExpAdd = 0;
    }
    

    //dragon
    if(bIsDragonJoin)
    {
        stMarchParam.m_stDragon.m_ddwLevel = ptbPlayer->m_nDragon_level;
        stMarchParam.m_stDragon.m_ddwIconId = ptbPlayer->m_nDragon_avatar;
        strncpy(stMarchParam.m_stDragon.m_szName, ptbPlayer->m_sDragon_name.c_str(), MAX_TABLE_NAME_LEN);
        stMarchParam.m_stDragon.m_szName[MAX_TABLE_NAME_LEN - 1] = '\0';
    }

    if (szSendResource || szTaxResource)
    {
        TUINT32 udwResourceNum = EN_RESOURCE_TYPE__END;
        CCommonFunc::GetArrayFromString(szSendResource, ':', stMarchParam.m_stResource.m_addwNum, udwResourceNum);

        udwResourceNum = EN_RESOURCE_TYPE__END;
        TINT64 addwTaxResource[EN_RESOURCE_TYPE__END] = { 0 };
        CCommonFunc::GetArrayFromString(szTaxResource, ':', addwTaxResource, udwResourceNum);

        // 更新 resource
        for(TUINT32 udwIdx = 0; udwIdx < EN_RESOURCE_TYPE__END; udwIdx++)
        {
            if (stMarchParam.m_stResource.m_addwNum[udwIdx] + addwTaxResource[udwIdx] == 0)
            {
                continue;
            }
            if (pstCity->m_stTblData.m_bResource[0].m_addwNum[udwIdx] > stMarchParam.m_stResource.m_addwNum[udwIdx] + addwTaxResource[udwIdx])
            {
                pstCity->m_stTblData.m_bResource[0].m_addwNum[udwIdx] -= stMarchParam.m_stResource.m_addwNum[udwIdx] + addwTaxResource[udwIdx];
                pstUser->m_udwCostResource += stMarchParam.m_stResource.m_addwNum[udwIdx] + addwTaxResource[udwIdx];
            }
            else
            {
                pstCity->m_stTblData.m_bResource[0].m_addwNum[udwIdx] = 0;
                pstUser->m_udwCostResource += pstCity->m_stTblData.m_bResource[0].m_addwNum[udwIdx];
            }
            pstCity->m_stTblData.SetFlag(TbCITY_FIELD_RESOURCE);
        }
    }
    // troop
    if(szTroop)
    {
        TUINT32 udwTroopNum = EN_TROOP_TYPE__END;
        CCommonFunc::GetArrayFromString(szTroop, ':', stMarchParam.m_stTroop.m_addwNum, udwTroopNum);
        // 保留原始
        stMarchParam.m_stTroopRaw = stMarchParam.m_stTroop;

        // 更新 troop
        for(TUINT32 udwIdx = 0; udwIdx < EN_TROOP_TYPE__END; udwIdx++)
        {
            if(stMarchParam.m_stTroop.m_addwNum[udwIdx] == 0)
            {
                continue;
            }
            if(pstCity->m_stTblData.m_bTroop[0].m_addwNum[udwIdx] >= stMarchParam.m_stTroop.m_addwNum[udwIdx])
            {
                pstCity->m_stTblData.m_bTroop[0].m_addwNum[udwIdx] -= stMarchParam.m_stTroop.m_addwNum[udwIdx];
            }
            else
            {
                stMarchParam.m_stTroop.m_addwNum[udwIdx] = pstCity->m_stTblData.m_bTroop[0].m_addwNum[udwIdx];
                pstCity->m_stTblData.m_bTroop[0].m_addwNum[udwIdx] = 0;
            }
            pstCity->m_stTblData.SetFlag(TbCITY_FIELD_TROOP);
        }
        stMarchParam.m_ddwTroopNum = CToolBase::GetTroopSumNum(stMarchParam.m_stTroopRaw);
        stMarchParam.m_ddwForce = CToolBase::GetTroopSumForce(stMarchParam.m_stTroopRaw);
    }

    // research
    if(dwMarchType == EN_ACTION_SEC_CLASS__SCOUT)
    {
        stMarchParam.m_ddwScoutLevel = pstCity->m_stTblData.m_bResearch[0].m_addwLevel[EN_RESEARCH_TYPE__SCOUT];

        if(stMarchParam.m_ddwMarchingTime <= 1)
        {
            stMarchParam.m_ddwScoutLevel = 10;
        }
    }

    //add action
   ptbMarchAction = CActionBase::AddMarchAction(pstUser, pstCity, EN_ACTION_MAIN_CLASS__MARCH, dwMarchType, EN_MARCH_STATUS__MARCHING,
        udwCostTime, &stMarchParam, 0, ptbTargetMap->m_nId);
    
    if(bIsDragonJoin)
    {
        ptbPlayer->Set_Dragon_tid(pstUser->m_uddwNewActionId);
        ptbPlayer->Set_Dragon_status(EN_DRAGON_STATUS_MARCH);
    }

    // knight updt to city
    if(dwKnightId >= 0)
    {
        pstCity->m_stTblData.m_bKnight[dwKnightId].ddwStatus = EN_KNIGHT_STATUS__MARCHING;
        pstCity->m_stTblData.m_bKnight[dwKnightId].ddwTid = pstUser->m_uddwNewActionId;
        pstCity->m_stTblData.SetFlag(TbCITY_FIELD_KNIGHT);
    }

    // buff
    if (dwMarchType == EN_ACTION_SEC_CLASS__ATTACK) //对方警戒塔展示用
    {
        if (bIsDragonJoin)
        {
            CBufferBase::GenMarchBuff(&pstUser->m_stPlayerBuffList, ptbMarchAction);
        }
        else
        {
            CBufferBase::GenMarchBuff(&pstUser->m_stBuffWithoutDragon, ptbMarchAction);
        }
    }
    else if (dwMarchType == EN_ACTION_SEC_CLASS__DRAGON_ATTACK)
    {
        ptbMarchAction->Set_If_max_attack(bIfMaxAttack);
    }
    //occupy 以到达时为基准计算
//     else if (dwMarchType == EN_ACTION_SEC_CLASS__OCCUPY)
//     {
//         SPlayerBuffInfo stOccupyBuff;
//         if (bIsDragonJoin)
//         {
//             CBufferBase::GenOccupyBuff(&pstUser->m_stPlayerBuffList, &stOccupyBuff);
//         }
//         else
//         {
//             CBufferBase::GenOccupyBuff(&pstUser->m_stBuffWithoutDragon, &stOccupyBuff);
//         }
//         CBufferBase::SetOccupyBuff(&stOccupyBuff, ptbMarchAction);
//     }
    return ptbMarchAction;
}

TbMarch_action* CProcessMarch::GenRallyAction(SUserInfo *pstUser, TbPlayer *ptbPlayer, SCityInfo *pstCity, TCHAR *szTroop, TINT32 dwKnightId, TBOOL bIsDragonJoin, TbMap *ptbTargetMap, TINT32 dwMarchType, TUINT32 udwCostTime, TUINT32 udwPrepareTime)
{
    TbMarch_action *ptbRallyMarch = NULL;
    SActionMarchParam stMarchParam;
    stMarchParam.Reset();
    //source user
    stMarchParam.m_ddwSourceUserId = ptbPlayer->m_nUid;
    strncpy(stMarchParam.m_szSourceUserName, ptbPlayer->m_sUin.c_str(), MAX_TABLE_NAME_LEN);
    stMarchParam.m_ddwSourceCityId = pstCity->m_stTblData.m_nPos;
    strncpy(stMarchParam.m_szSourceCityName, pstCity->m_stTblData.m_sName.c_str(), MAX_TABLE_NAME_LEN);
    stMarchParam.m_ddwSourceAlliance = ptbPlayer->m_nAlpos ? ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET : 0;
    if(stMarchParam.m_ddwSourceAlliance)
    {
        strncpy(stMarchParam.m_szSourceAlliance, ptbPlayer->m_sAlname.c_str(), MAX_TABLE_NAME_LEN);
        strncpy(stMarchParam.m_szSourceAlNick, ptbPlayer->m_sAl_nick_name.c_str(), MAX_TABLE_NAME_LEN);
    }

    //target user
    if (ptbTargetMap->m_nType != EN_WILD_TYPE__THRONE_NEW)
    {
        stMarchParam.m_ddwTargetUserId = ptbTargetMap->m_nUid;
        strncpy(stMarchParam.m_szTargetUserName, ptbTargetMap->m_sUname.c_str(), MAX_TABLE_NAME_LEN);
        if (ptbTargetMap->m_nType == EN_WILD_TYPE__CITY)
        {
            stMarchParam.m_ddwTargetCityId = ptbTargetMap->m_nId;
        }
        else
        {
            stMarchParam.m_ddwTargetCityId = ptbTargetMap->m_nCid;
        }
        strncpy(stMarchParam.m_szTargetCityName, ptbTargetMap->m_sName.c_str(), MAX_TABLE_NAME_LEN);
        stMarchParam.m_ddwTargetAlliance = ptbTargetMap->m_nAlid;
        strncpy(stMarchParam.m_szTargetAlliance, ptbTargetMap->m_sAlname.c_str(), MAX_TABLE_NAME_LEN);
        strncpy(stMarchParam.m_szTargetAlNick, ptbTargetMap->m_sAl_nick.c_str(), MAX_TABLE_NAME_LEN);
    }
    stMarchParam.m_ddwTargetType = ptbTargetMap->m_nType;
    stMarchParam.m_ddwTargetLevel = ptbTargetMap->m_nLevel;

    //march time
    stMarchParam.m_ddwMarchingTime = udwCostTime;
    stMarchParam.m_ddwPrepareTime = udwPrepareTime;

    //dragon
    if(bIsDragonJoin)
    {
        stMarchParam.m_stDragon.m_ddwLevel = ptbPlayer->m_nDragon_level;
        stMarchParam.m_stDragon.m_ddwIconId = ptbPlayer->m_nDragon_avatar;
        strncpy(stMarchParam.m_stDragon.m_szName, ptbPlayer->m_sDragon_name.c_str(), MAX_TABLE_NAME_LEN);
        stMarchParam.m_stDragon.m_szName[MAX_TABLE_NAME_LEN - 1] = '\0';
    }

    // troop
    if(szTroop)
    {
        TUINT32 udwTroopNum = EN_TROOP_TYPE__END;
        CCommonFunc::GetArrayFromString(szTroop, ':', stMarchParam.m_stTroop.m_addwNum, udwTroopNum);
        // 保留原始
        stMarchParam.m_stTroopRaw = stMarchParam.m_stTroop;

        // 更新 troop
        for(TUINT32 udwIdx = 0; udwIdx < EN_TROOP_TYPE__END; udwIdx++)
        {
            if(stMarchParam.m_stTroop.m_addwNum[udwIdx] == 0)
            {
                continue;
            }
            if(pstCity->m_stTblData.m_bTroop[0].m_addwNum[udwIdx] >= stMarchParam.m_stTroop.m_addwNum[udwIdx])
            {
                pstCity->m_stTblData.m_bTroop[0].m_addwNum[udwIdx] -= stMarchParam.m_stTroop.m_addwNum[udwIdx];
            }
            else
            {
                stMarchParam.m_stTroop.m_addwNum[udwIdx] = pstCity->m_stTblData.m_bTroop[0].m_addwNum[udwIdx];
                pstCity->m_stTblData.m_bTroop[0].m_addwNum[udwIdx] = 0;
            }
            pstCity->m_stTblData.SetFlag(TbCITY_FIELD_TROOP);
        }

        stMarchParam.m_ddwTroopNum = CToolBase::GetTroopSumNum(stMarchParam.m_stTroopRaw);
        stMarchParam.m_ddwForce = CToolBase::GetTroopSumForce(stMarchParam.m_stTroopRaw);
    }

    //knight
    stMarchParam.m_stKnight.ddwId = dwKnightId;
    if (dwKnightId >= 0)
    {
        stMarchParam.m_stKnight.ddwLevel = CGameInfo::GetInstance()->ComputeKnightLevelByExp(pstCity->m_stTblData.m_bKnight[dwKnightId].ddwExp);
        stMarchParam.m_stKnight.ddwExpAdd = 0;
    }

    //add action
    ptbRallyMarch = CActionBase::AddMarchAction(pstUser, pstCity, EN_ACTION_MAIN_CLASS__MARCH, dwMarchType, EN_MARCH_STATUS__MARCHING,
        udwCostTime, &stMarchParam, 0, ptbTargetMap->m_nId);

    if (bIsDragonJoin)
    {
        ptbPlayer->Set_Dragon_tid(pstUser->m_uddwNewActionId);
        ptbPlayer->Set_Dragon_status(EN_DRAGON_STATUS_MARCH);
    }

    if (dwKnightId >= 0)
    {
        pstCity->m_stTblData.m_bKnight[dwKnightId].ddwStatus = EN_KNIGHT_STATUS__MARCHING;
        pstCity->m_stTblData.m_bKnight[dwKnightId].ddwTid = pstUser->m_uddwNewActionId;
        pstCity->m_stTblData.SetFlag(TbCITY_FIELD_KNIGHT);
    }

    if(ptbRallyMarch)
    {
        if(dwMarchType == EN_ACTION_SEC_CLASS__RALLY_WAR
            || dwMarchType == EN_ACTION_SEC_CLASS__RALLY_WAR_THRONE) //对方警戒塔展示
        {
            if (bIsDragonJoin)
            {
                CBufferBase::GenMarchBuff(&pstUser->m_stPlayerBuffList, ptbRallyMarch, TRUE);
            }
            else
            {
                CBufferBase::GenMarchBuff(&pstUser->m_stBuffWithoutDragon, ptbRallyMarch, TRUE);
            }

            ptbRallyMarch->m_bRally_atk_slot.m_udwNum = 3;
            for(TUINT32 udwIdx = 0; udwIdx < ptbRallyMarch->m_bRally_atk_slot.m_udwNum; ++udwIdx)
            {
                ptbRallyMarch->m_bRally_atk_slot[udwIdx].Reset();
            }
            strncpy(ptbRallyMarch->m_bRally_atk_slot[0].szUserName, "empty", MAX_TABLE_NAME_LEN);
            ptbRallyMarch->m_bRally_atk_slot[0].szUserName[MAX_TABLE_NAME_LEN - 1] = '\0';
            ptbRallyMarch->SetFlag(TbMARCH_ACTION_FIELD_RALLY_ATK_SLOT);

            ptbRallyMarch->m_bRally_def_slot.m_udwNum = CCommonBase::GetGameBasicVal(EN_GAME_BASIC_RALLY_SLOT_LIMIT); // 默认全部解锁

            for(TUINT32 udwIdx = 0; udwIdx < ptbRallyMarch->m_bRally_def_slot.m_udwNum; ++udwIdx)
            {
                ptbRallyMarch->m_bRally_def_slot[udwIdx].Reset();
            }
            strncpy(ptbRallyMarch->m_bRally_def_slot[0].szUserName, "empty", MAX_TABLE_NAME_LEN);
            ptbRallyMarch->m_bRally_def_slot[0].szUserName[MAX_TABLE_NAME_LEN - 1] = '\0';
            ptbRallyMarch->SetFlag(TbMARCH_ACTION_FIELD_RALLY_DEF_SLOT);

            ptbRallyMarch->m_bRally_atk_force[0].ddwTotalNum = ptbRallyMarch->m_bParam[0].m_ddwTroopNum;
            ptbRallyMarch->m_bRally_atk_force[0].ddwTotalForce = ptbRallyMarch->m_bParam[0].m_ddwForce;
            ptbRallyMarch->m_bRally_atk_force[0].ddwReinforceTroopLimit = pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_RALLY_TROOP_LIMIT].m_ddwBuffTotal;
            ptbRallyMarch->SetFlag(TbMARCH_ACTION_FIELD_RALLY_ATK_FORCE);

            ptbRallyMarch->Set_Ctime(udwPrepareTime);
            ptbRallyMarch->Set_Etime(ptbRallyMarch->m_nBtime + udwPrepareTime);
            ptbRallyMarch->Set_Status(EN_MARCH_STATUS__PREPARING);

            ptbRallyMarch->m_bAtk_total_troop[0] = ptbRallyMarch->m_bParam[0].m_stTroop;
            ptbRallyMarch->SetFlag(TbMARCH_ACTION_FIELD_ATK_TOTAL_TROOP);
        }
    }

    return ptbRallyMarch;
}

TINT32 CProcessMarch::CheckCanRallyWar( SUserInfo *pstUser, TUINT32 udwTargetPos, TUINT32 udwAlid )
{
    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
    {
        TbMarch_action *ptbAction = &pstUser->m_atbMarch[udwIdx];
        if (ptbAction->m_nMclass == EN_ACTION_MAIN_CLASS__MARCH 
            && (ptbAction->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR 
            || ptbAction->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR_THRONE)
            && ptbAction->m_nStatus != EN_MARCH_STATUS__RETURNING)
        {
            if (ptbAction->m_nSuid == pstUser->m_tbPlayer.m_nUid)
            {
                return EN_RET_CODE__RALLY_WAR_ONLY_ONE_MARCH;
            }
            if (ptbAction->m_nSal == udwAlid && ptbAction->m_nTpos == udwTargetPos)
            {
                return EN_RET_CODE__RALLY_WAR_ONE_POS_ONLY_ONE_MARCH;
            }
        }
    }
    return 0;
}
