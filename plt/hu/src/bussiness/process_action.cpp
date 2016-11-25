#include "process_action.h"
#include "statistic.h"
#include "game_info.h"
#include "procedure_base.h"
#include "global_serv.h"
#include "common_base.h"
#include "city_info.h"
#include "action_base.h"
#include "city_base.h"
#include "sendmessage_base.h"
#include "globalres_logic.h"
#include "hu_cblog.h"
#include "backpack_info.h"
#include "common_func.h"
#include "msg_base.h"
#include "activities_logic.h"
#include "common_logic.h"
#include "quest_logic.h"
#include "player_base.h"
#include "item_base.h"
#include "tool_base.h"
#include "backpack_logic.h"
#include "map_logic.h"

TINT32 CProcessAction::ProcessCmd_ActionGemSpeedUp(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;

    TUINT64 uddwActionId = strtoull(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TUINT32 udwGemNum = strtoul(pstSession->m_stReqParam.m_szKey[1], NULL, 10);
    TUINT32 udwSecondClass = strtoul(pstSession->m_stReqParam.m_szKey[2], NULL, 10);

    TUINT32 udwActionType = CActionBase::GetActionTypeBySecondClass(udwSecondClass);

    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TINT32 dwActionIdx = -1;
    TINT64 ddwCurTime = CTimeUtils::GetUnixTime();

    pstSession->m_udwGemCost = udwGemNum;

    TUINT32 udwActionMainClass = 0;
    TUINT32 udwActionSecondClass = 0;
    TINT64 ddwActionTime = 0;
    TINT64 ddwActionEndTime = 0;
    // 0. 获取action
    switch(udwActionType)
    {
    case EN_ACTION_TYPE_BUFF_NORMAL:
        dwActionIdx = CActionBase::GetActionIndex(pstUser->m_atbAction, pstUser->m_udwActionNum, uddwActionId);
        break;
    case EN_ACTION_TYPE_AL_CAN_HELP:
        dwActionIdx = CActionBase::GetAlActionIndex(pstUser->m_atbSelfAlAction,pstUser->m_udwSelfAlActionNum,uddwActionId);
        break;
    case EN_ACTION_TYPE_MARCH:
        dwActionIdx = CActionBase::GetMarchIndex(pstUser->m_atbMarch,pstUser->m_udwMarchNum,uddwActionId);
        break;
    }
    if(dwActionIdx >= 0)
    {
        switch(udwActionType)
        {
        case EN_ACTION_TYPE_BUFF_NORMAL:
            udwActionMainClass = pstUser->m_atbAction[dwActionIdx].m_nMclass;
            udwActionSecondClass = pstUser->m_atbAction[dwActionIdx].m_nSclass;
            ddwActionTime = pstUser->m_atbAction[dwActionIdx].m_nEtime - ddwCurTime;
            ddwActionEndTime = pstUser->m_atbAction[dwActionIdx].m_nEtime;
            break;
        case EN_ACTION_TYPE_AL_CAN_HELP:
            udwActionMainClass = pstUser->m_atbSelfAlAction[dwActionIdx].m_nMclass;
            udwActionSecondClass = pstUser->m_atbSelfAlAction[dwActionIdx].m_nSclass;
            ddwActionTime = pstUser->m_atbSelfAlAction[dwActionIdx].m_nEtime - ddwCurTime;
            ddwActionEndTime = pstUser->m_atbSelfAlAction[dwActionIdx].m_nEtime;
            break;
        case EN_ACTION_TYPE_MARCH:
            udwActionMainClass = pstUser->m_atbMarch[dwActionIdx].m_nMclass;
            udwActionSecondClass = pstUser->m_atbMarch[dwActionIdx].m_nSclass;
            ddwActionTime = pstUser->m_atbMarch[dwActionIdx].m_nEtime - ddwCurTime;
            ddwActionEndTime = pstUser->m_atbMarch[dwActionIdx].m_nEtime;
            break;
        }
        if(pstCity == NULL)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__ActionGemSpeedUp: error city pos [seq=%u]", dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
    }
    else
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ACTION_NOT_EXIST;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__ActionGemSpeedUp: error action id [aid=%u] [seq=%u]", uddwActionId, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -2;
    }

    
    //cblog:为日志计算key3（id）和key4（level）
    if(EN_ACTION_MAIN_CLASS__BUILDING == udwActionMainClass)
    {
        sprintf(pstSession->m_stReqParam.m_szKey[3], "%ld", pstUser->m_atbSelfAlAction[dwActionIdx].m_bParam[0].m_stBuilding.m_ddwType);
        sprintf(pstSession->m_stReqParam.m_szKey[4], "%ld", pstUser->m_atbSelfAlAction[dwActionIdx].m_bParam[0].m_stBuilding.m_ddwTargetLevel);
    }
    else if (EN_ACTION_MAIN_CLASS__TRAIN_NEW == udwActionMainClass)
    {
        sprintf(pstSession->m_stReqParam.m_szKey[3], "%ld", pstUser->m_atbSelfAlAction[dwActionIdx].m_bParam[0].m_stTrain.m_ddwType);
        sprintf(pstSession->m_stReqParam.m_szKey[4], "%ld", pstUser->m_atbSelfAlAction[dwActionIdx].m_bParam[0].m_stTrain.m_ddwNum);
    }
    else if(EN_ACTION_MAIN_CLASS__EQUIP == udwActionMainClass)
    {
        sprintf(pstSession->m_stReqParam.m_szKey[3], "%ld", pstUser->m_atbSelfAlAction[dwActionIdx].m_bParam[0].m_stEquip.m_uddwId);
        sprintf(pstSession->m_stReqParam.m_szKey[4], "%ld", pstUser->m_atbSelfAlAction[dwActionIdx].m_bParam[0].m_stEquip.m_ddwLevel);
    }

    TINT32 dwSpeedUpTime = ddwActionEndTime - CTimeUtils::GetUnixTime();

    TINT32 dwFreeTime = CCommonBase::GetGameBasicVal(EN_GAME_BASIC_FREE_INSTANT_TIME) +
        pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_FREE_INSTANT_FINISH_TIME].m_ddwBuffTotal;
    TINT32 dwRealTimeCost = dwSpeedUpTime - dwFreeTime;
    if (!CCommonBase::CheckInstantGem(udwGemNum, dwRealTimeCost))
    {
        TINT32 dwRealCostGem = CCommonBase::ComputeSpeedUpGem(dwRealTimeCost);
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__BLACK_ACCOUNT;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__ActionGemSpeedUp: giving gem cost unright realcostgem[%d] givencostgem[%u] [seq=%u]",
            dwRealCostGem, udwGemNum, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -7;
    }

    // 1. 判定资源是否足够
    if(FALSE == CPlayerBase::HasEnoughGem(&pstUser->m_tbLogin, udwGemNum))
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__GEM_LACK;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__ActionGemSpeedUp: gem not enough [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
        return -8;
    }

    // 2. 资源消耗
    // 不需要加速
    if(ddwActionEndTime <= ddwCurTime)
    {
        dwSpeedUpTime = 0;
        udwGemNum = 0;
        pstSession->m_udwGemCost = udwGemNum;
        TSE_LOG_INFO(pstSession->m_poServLog, ("ProcessCmd__ActionGemSpeedUp: no need speed up [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
    }
    CPlayerBase::CostGem(pstUser, udwGemNum);

    // 3. 根据action的main class和sec class分别处理完成的情况
    CProcessAction::ActionDone(pstSession, pstUser, pstCity, udwActionMainClass,udwActionSecondClass, dwActionIdx, TRUE);

    CActivitesLogic::ComputeSpeedUpGemScore(pstUser, dwSpeedUpTime);

    // 4. wave@push_data
    switch(udwActionType)
    {
    case EN_ACTION_TYPE_BUFF_NORMAL:
        break;
    case EN_ACTION_TYPE_AL_CAN_HELP:
        pstUser->m_ptbPushAlAction = &pstUser->m_atbSelfAlAction[dwActionIdx];
        pstUser->dwPushAlActionType = EN_TABLE_UPDT_FLAG__DEL;
        break;
    case EN_ACTION_TYPE_MARCH:
        pstUser->m_ptbPushMarchAction = &pstUser->m_atbMarch[dwActionIdx];
        pstUser->dwPushMarchActionType = EN_TABLE_UPDT_FLAG__DEL;
        break;
    }

    return 0;
}

TINT32 CProcessAction::ProcessCmd_ActionFreeSpeedUp(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TUINT64 uddwActionId = strtoull(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TUINT32 udwSecondClass = strtoul(pstSession->m_stReqParam.m_szKey[1], NULL, 10);
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TINT32 dwActionIdx = -1;
    TUINT32 udwCurtime = CTimeUtils::GetUnixTime();

    TUINT32 udwActionType = CActionBase::GetActionTypeBySecondClass(udwSecondClass);

    TUINT32 udwActionMainClass = 0;
    TUINT32 udwActionSecondClass = 0;
    TUINT32 udwEtime = 0;

    // 0. 获取action
    switch(udwActionType)
    {
    case EN_ACTION_TYPE_BUFF_NORMAL:
        dwActionIdx = CActionBase::GetActionIndex(pstUser->m_atbAction, pstUser->m_udwActionNum, uddwActionId);
        break;
    case EN_ACTION_TYPE_AL_CAN_HELP:
        dwActionIdx = CActionBase::GetAlActionIndex(pstUser->m_atbSelfAlAction, pstUser->m_udwSelfAlActionNum, uddwActionId);
        break;
    case EN_ACTION_TYPE_MARCH:
        dwActionIdx = CActionBase::GetMarchIndex(pstUser->m_atbMarch, pstUser->m_udwMarchNum, uddwActionId);
        break;
    }
    if(dwActionIdx >= 0)
    {
        switch(udwActionType)
        {
        case EN_ACTION_TYPE_BUFF_NORMAL:
            udwActionMainClass = pstUser->m_atbAction[dwActionIdx].m_nMclass;
            udwActionSecondClass = pstUser->m_atbAction[dwActionIdx].m_nSclass;
            udwEtime = pstUser->m_atbAction[dwActionIdx].m_nEtime;
            break;
        case EN_ACTION_TYPE_AL_CAN_HELP:
            udwActionMainClass = pstUser->m_atbSelfAlAction[dwActionIdx].m_nMclass;
            udwActionSecondClass = pstUser->m_atbSelfAlAction[dwActionIdx].m_nSclass;
            udwEtime = pstUser->m_atbSelfAlAction[dwActionIdx].m_nEtime;
            break;
        case EN_ACTION_TYPE_MARCH:
            udwActionMainClass = pstUser->m_atbMarch[dwActionIdx].m_nMclass;
            udwActionSecondClass = pstUser->m_atbMarch[dwActionIdx].m_nSclass;
            udwEtime = pstUser->m_atbMarch[dwActionIdx].m_nEtime;
            break;
        }
        if(pstCity == NULL)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__ActionGemSpeedUp: error city pos [seq=%u]", dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
    }
    else
    {
        //pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ACTION_NOT_EXIST;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__ActionGemSpeedUp: error action id [aid=%u] [seq=%u]", uddwActionId, pstSession->m_stUserInfo.m_udwBSeqNo));
        //return -2;
        return 0; // 不报错...防止在action已经完成的情况下(盟友帮助,倒计时结束)报错...
    }

    TINT32 dwFreeTime = CCommonBase::GetGameBasicVal(EN_GAME_BASIC_FREE_INSTANT_TIME) +
        pstUser->m_stPlayerBuffList[EN_BUFFER_INFO_FREE_INSTANT_FINISH_TIME].m_ddwBuffTotal;
    //只有build，research，hospital 的等待时间可以免费加速
    if ((EN_ACTION_MAIN_CLASS__BUILDING == udwActionMainClass) ||
        (EN_ACTION_MAIN_CLASS__TRAIN_NEW == udwActionMainClass && EN_ACTION_SEC_CLASS__HOS_TREAT == udwActionSecondClass) ||
        (EN_ACTION_MAIN_CLASS__EQUIP == udwActionMainClass && EN_ACTION_SEC_CLASS__EQUIP_UPGRADE == udwActionSecondClass) ||
        (EN_ACTION_MAIN_CLASS__DRAGON == udwActionMainClass))
    {
        if (udwEtime - dwFreeTime > udwCurtime)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__BLACK_ACCOUNT;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__ActionFreeSpeedUp: cannot free speedup yet etime[%u] - free_time[%d] > curtime[%u] [seq=%u]",
                udwEtime, dwFreeTime, udwCurtime, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }
    }
    else
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__ActionFreeSpeedUp: error action type [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
        return -4;
    }

    //cblog:为日志计算key3（id）和key4（level）
    if(EN_ACTION_MAIN_CLASS__BUILDING == udwActionMainClass)
    {
        sprintf(pstSession->m_stReqParam.m_szKey[3], "%ld", pstUser->m_atbSelfAlAction[dwActionIdx].m_bParam[0].m_stBuilding.m_ddwType);
        sprintf(pstSession->m_stReqParam.m_szKey[4], "%ld", pstUser->m_atbSelfAlAction[dwActionIdx].m_bParam[0].m_stBuilding.m_ddwTargetLevel);
    }
    else if (EN_ACTION_MAIN_CLASS__TRAIN_NEW == udwActionMainClass)
    {
        sprintf(pstSession->m_stReqParam.m_szKey[3], "%ld", pstUser->m_atbSelfAlAction[dwActionIdx].m_bParam[0].m_stTrain.m_ddwType);
        sprintf(pstSession->m_stReqParam.m_szKey[4], "%ld", pstUser->m_atbSelfAlAction[dwActionIdx].m_bParam[0].m_stTrain.m_ddwNum);
    }
    else if(EN_ACTION_MAIN_CLASS__EQUIP == udwActionMainClass)
    {
        sprintf(pstSession->m_stReqParam.m_szKey[3], "%ld", pstUser->m_atbSelfAlAction[dwActionIdx].m_bParam[0].m_stEquip.m_uddwId);
        sprintf(pstSession->m_stReqParam.m_szKey[4], "%ld", pstUser->m_atbSelfAlAction[dwActionIdx].m_bParam[0].m_stEquip.m_ddwLevel);
    }

    // 3. 根据action的main class和sec class分别处理完成的情况
    CProcessAction::ActionDone(pstSession, pstUser, pstCity, udwActionMainClass, udwActionSecondClass, dwActionIdx, FALSE);

    // 4. wave@push_data
    switch(udwActionType)
    {
    case EN_ACTION_TYPE_BUFF_NORMAL:
        break;
    case EN_ACTION_TYPE_AL_CAN_HELP:
        pstUser->m_ptbPushAlAction = &pstUser->m_atbSelfAlAction[dwActionIdx];
        pstUser->dwPushAlActionType = EN_TABLE_UPDT_FLAG__DEL;
        break;
    case EN_ACTION_TYPE_MARCH:
        //todo
        break;
    }

    return 0;
}

TINT32 CProcessAction::ProcessCmd_ActionCancel(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TUINT64 uddwActionId = strtoull(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TUINT32 udwSecondClass = strtoul(pstSession->m_stReqParam.m_szKey[1], NULL, 10);
    
    TINT32 dwActionIdx = -1;
    TUINT32 udwActionType = CActionBase::GetActionTypeBySecondClass(udwSecondClass);

    TUINT32 udwActionMainClass = 0;
    TUINT32 udwActionSecondClass = 0;
    // 0. 获取action
    switch(udwActionType)
    {
    case EN_ACTION_TYPE_BUFF_NORMAL:
        dwActionIdx = CActionBase::GetActionIndex(pstUser->m_atbAction, pstUser->m_udwActionNum, uddwActionId);
        break;
    case EN_ACTION_TYPE_AL_CAN_HELP:
        dwActionIdx = CActionBase::GetAlActionIndex(pstUser->m_atbSelfAlAction, pstUser->m_udwSelfAlActionNum, uddwActionId);
        break;
    case EN_ACTION_TYPE_MARCH:
        dwActionIdx = CActionBase::GetMarchIndex(pstUser->m_atbMarch, pstUser->m_udwMarchNum, uddwActionId);
        break;
    }
    if(dwActionIdx >= 0)
    {
        switch(udwActionType)
        {
        case EN_ACTION_TYPE_BUFF_NORMAL:
            udwActionMainClass = pstUser->m_atbAction[dwActionIdx].m_nMclass;
            udwActionSecondClass = pstUser->m_atbAction[dwActionIdx].m_nSclass;
            break;
        case EN_ACTION_TYPE_AL_CAN_HELP:
            udwActionMainClass = pstUser->m_atbSelfAlAction[dwActionIdx].m_nMclass;
            udwActionSecondClass = pstUser->m_atbSelfAlAction[dwActionIdx].m_nSclass;
            break;
        case EN_ACTION_TYPE_MARCH:
            udwActionMainClass = pstUser->m_atbMarch[dwActionIdx].m_nMclass;
            udwActionSecondClass = pstUser->m_atbMarch[dwActionIdx].m_nSclass;
            break;
        }
        if(pstCity == NULL)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__ActionGemSpeedUp: error city pos [seq=%u]", dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
    }
    else
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ACTION_NOT_EXIST;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__ActionGemSpeedUp: error action id [aid=%u] [seq=%u]", uddwActionId, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -2;
    }

    // 根据action的main class和sec class分别处理cancel的资源回收——回收50%
    switch(udwActionMainClass)
    {
    case EN_ACTION_MAIN_CLASS__BUILDING:
        dwRetCode = BuildingActionCancel(pstSession, pstUser, pstCity, (EActionSecClass)udwActionSecondClass, &pstUser->m_atbSelfAlAction[dwActionIdx].m_bParam[0].m_stBuilding);
        if(dwRetCode != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__ActionCancel: BuildingActionCancel: ret[%d] [seq=%u]", \
                dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
        }
        CCommonBase::SetActionDeleteFlag(pstUser, uddwActionId, udwActionSecondClass);
        break;
    case EN_ACTION_MAIN_CLASS__EQUIP:
        dwRetCode = EquipActionCancel(pstSession, pstUser, pstCity, (EActionSecClass)udwActionSecondClass, &pstUser->m_atbSelfAlAction[dwActionIdx].m_bParam[0].m_stEquip);
        if(dwRetCode != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__ActionCancel: equipCancel: ret[%d] [seq=%u]", \
                dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
        }
        CCommonBase::SetActionDeleteFlag(pstUser, uddwActionId, udwActionSecondClass);
        break;
    case EN_ACTION_MAIN_CLASS__TRAIN_NEW:
        dwRetCode = TrainCancelNew(pstSession, pstUser, pstCity, &pstUser->m_atbSelfAlAction[dwActionIdx]);
        if (dwRetCode != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__ActionCancel: TrainCancel: ret[%d] [seq=%u]", \
                dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
        }
        break;
    default:
        dwRetCode = -3;
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__ActionCancel: error main class [seq=%u]", \
            dwRetCode, pstSession->m_stUserInfo.m_udwBSeqNo));
        break;
    }
    if(dwRetCode != 0)
    {
        return dwRetCode;
    }

    // 4. wave@push_data
    switch(udwActionType)
    {
    case EN_ACTION_TYPE_BUFF_NORMAL:
        break;
    case EN_ACTION_TYPE_AL_CAN_HELP:
        pstUser->m_ptbPushAlAction = &pstUser->m_atbSelfAlAction[dwActionIdx];
        pstUser->dwPushAlActionType = EN_TABLE_UPDT_FLAG__DEL;
        break;
    case EN_ACTION_TYPE_MARCH:
        pstUser->m_ptbPushMarchAction = &pstUser->m_atbMarch[dwActionIdx];
        pstUser->dwPushMarchActionType = EN_TABLE_UPDT_FLAG__DEL;
        break;
    }

    return 0;
}

TINT32 CProcessAction::ProcessCmd_TroopCancel(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    if(NULL == pstCity)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__TroopCancel: no city info [seq=%u]", \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }

    TINT64 ddwActionId = strtoull(pstSession->m_stReqParam.m_szKey[0], NULL, 10);

    TbAlliance_action* ptbAlAction = CActionBase::GetAlAction(pstUser->m_atbSelfAlAction, pstUser->m_udwSelfAlActionNum, ddwActionId);
    if(ptbAlAction != NULL)
    {
        dwRetCode = CProcessAction::TrainCancelNew(pstSession, pstUser, pstCity, ptbAlAction);
    }
    else
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ACTION_NOT_EXIST;
        return -2;
    }
    
    if(dwRetCode < 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__TroopCancel: train cancel ret[%d] [seq=%u]", 
            dwRetCode, pstSession->m_udwSeqNo));
        return -3;
    }

    return 0;
}

TINT32 CProcessAction::ProcessCmd_FortCancel(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;

    TINT64 ddwActionId = strtoull(pstSession->m_stReqParam.m_szKey[0], NULL, 10);

    TbAlliance_action* ptbAlAction = CActionBase::GetAlAction(pstUser->m_atbSelfAlAction, pstUser->m_udwSelfAlActionNum, ddwActionId);
    if (ptbAlAction != NULL)
    {
        dwRetCode = CProcessAction::TrainCancelNew(pstSession, pstUser, pstCity, ptbAlAction);
    }
    else
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ACTION_NOT_EXIST;
        return -2;
    }

    if(dwRetCode < 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_FortCancel: train cancel ret[%d] [seq=%u]", 
            dwRetCode, pstSession->m_udwSeqNo));
        return -3;
    }

    return 0;
}

TINT32 CProcessAction::ProcessCmd_HospitalTroopTreatCancel(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TbCity& tbCity = pstCity->m_stTblData;

    TINT64 ddwActionId = strtoull(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    SActionTrainParam *pstParam = NULL;
    TbAlliance_action* ptbAlAction = CActionBase::GetAlAction(pstUser->m_atbSelfAlAction, pstUser->m_udwSelfAlActionNum, ddwActionId);
    if (ptbAlAction != NULL)
    {
        pstParam = &ptbAlAction->m_bParam[0].m_stTrain;
        dwRetCode = CProcessAction::TrainCancelNew(pstSession, pstUser, pstCity, ptbAlAction);
    }
    else
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ACTION_NOT_EXIST;
        return -2;
    }

    if(dwRetCode < 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_FortCancel: train cancel ret[%d] [seq=%u]", 
            dwRetCode, pstSession->m_udwSeqNo));
        return -3;
    }

    //补回hos wait
    tbCity.m_bHos_wait[0].m_addwNum[pstParam->m_ddwType] +=
        pstParam->m_ddwNum;
    tbCity.SetFlag(TbCITY_FIELD_HOS_WAIT);

    return 0;
}

TINT32 CProcessAction::ProcessCmd_FortRepairCancel(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TbCity& tbCity = pstCity->m_stTblData;

    TINT64 ddwActionId = strtoull(pstSession->m_stReqParam.m_szKey[0], NULL, 10);

    SActionTrainParam *pstParam = NULL;
    TbAlliance_action* ptbAlAction = CActionBase::GetAlAction(pstUser->m_atbSelfAlAction, pstUser->m_udwSelfAlActionNum, ddwActionId);
    if (ptbAlAction != NULL)
    {
        pstParam = &ptbAlAction->m_bParam[0].m_stTrain;
        dwRetCode = CProcessAction::TrainCancelNew(pstSession, pstUser, pstCity, ptbAlAction);
    }
    else
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ACTION_NOT_EXIST;
        return -2;
    }

    if(dwRetCode < 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_FortRepairCancel: heal cancel ret[%d] [seq=%u]", 
            dwRetCode, pstSession->m_udwSeqNo));
        return -3;
    }

    //补回dead_fort
    tbCity.m_bDead_fort[0][pstParam->m_ddwType] += pstParam->m_ddwNum;
    tbCity.SetFlag(TbCITY_FIELD_DEAD_FORT);

    return 0;
}

/************************************************private**********************************************/
//EN_ACTION_MAIN_CLASS__BUILDING
TINT32 CProcessAction::BuildingActionCancel(SSession *pstSession, SUserInfo *pstUser, SCityInfo *pstCity, EActionSecClass enSecClass, SActionBuildingParam *pstParam)
{
    TINT32 dwRetCode = 0;
    CGameInfo *poGameInfo = CGameInfo::GetInstance();
    TINT64 addwResource[EN_BASE_COST_TYPE__TIME];
    TUINT32 idx = 0;
    SBuildingInfo stBuildInfo;
    SSpGlobalRes stResearchCost;
    stResearchCost.Reset();

    // 1. 获取资源
    switch(enSecClass)
    {
    case EN_ACTION_SEC_CLASS__BUILDING_UPGRADE:
    case EN_ACTION_SEC_CLASS__REMOVE_OBSTACLE:
        if(FALSE == poGameInfo->GetBuildingInfo(pstParam->m_ddwType, pstParam->m_ddwTargetLevel, &stBuildInfo))
        {
            dwRetCode = -1;
            break;
        }
        memcpy((char*)&addwResource[0], (char*)&stBuildInfo.m_addwBaseCost[0], EN_BASE_COST_TYPE__TIME * sizeof(TINT64));
        break;
    case EN_ACTION_SEC_CLASS__RESEARCH_UPGRADE:
        if(CCommonBase::GetResearchCost(pstParam->m_ddwType, pstParam->m_ddwTargetLevel, stResearchCost) != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("Can not get research [seq=%u]", pstSession->m_udwSeqNo));
            dwRetCode = -2;
        }
        else
        {
            for(TUINT32 udwIdx = 0; udwIdx < stResearchCost.udwTotalNum; ++udwIdx)
            {
                if(stResearchCost[udwIdx].udwType == EN_GLOBALRES_TYPE_RESOURCE)
                {
                    addwResource[stResearchCost[udwIdx].udwId] = stResearchCost[udwIdx].udwNum;
                    stResearchCost[udwIdx].udwNum /= 2;
                }
            }
        }
        break;
    default:
        memset((char*)&addwResource[0], 0, EN_BASE_COST_TYPE__TIME*sizeof(TINT64));
        break;
    }
    if(dwRetCode != 0)
    {
        return dwRetCode;
    }

    // 2. 计算资源
    for(idx = 0; idx < EN_BASE_COST_TYPE__TIME; idx++)
    {
        addwResource[idx] = 0.5*addwResource[idx];
    }

    if(enSecClass == EN_ACTION_SEC_CLASS__BUILDING_UPGRADE ||
        EN_ACTION_SEC_CLASS__REMOVE_OBSTACLE == enSecClass)
    {
        // 3. 补回资源
        CCityBase::AddResource(pstCity, &addwResource[0]);
        // 补回item
        if(stBuildInfo.m_ucUpradeNeedItem == TRUE)
        {
            CItemBase::AddItem(&pstSession->m_stUserInfo.m_tbBackpack, stBuildInfo.m_stSpecialItem.m_udwId, stBuildInfo.m_stSpecialItem.m_udwNum);
        }
    }
    else if(enSecClass == EN_ACTION_SEC_CLASS__RESEARCH_UPGRADE)
    {
        CGlobalResLogic::AddSpGlobalRes(pstUser, pstCity, &stResearchCost);
    }

    // 4 cb log key
    TBOOL bHead = TRUE;
    TUINT32 udwLen = 0;
    for(idx = 0; idx < EN_BASE_COST_TYPE__SPACE; ++idx)
    {
        if(bHead)
        {
            udwLen = snprintf(pstSession->m_stReqParam.m_szKey[1], 1024, "%ld", addwResource[idx]);
            bHead = FALSE;
        }
        else
        {
            udwLen += snprintf(pstSession->m_stReqParam.m_szKey[1] + udwLen, 1024 - udwLen, ":%ld", addwResource[idx]);
        }
    }

    return 0;
}

//EN_ACTION_MAIN_CLASS__BUILDING
TINT32 CProcessAction::BuildingActionDone(SSession *pstSession, SUserInfo *pstUser, SCityInfo *pstCity,
    EActionSecClass enSecClass, SActionBuildingParam *pstParam, TBOOL bCbLog)
{
    TbPlayer& tbPlayer = pstUser->m_tbPlayer;
    TbCity& tbCity = pstCity->m_stTblData;
    TCHAR szCmd[1024];
    szCmd[0] = '\0';

    switch((int)enSecClass)
    {
    case EN_ACTION_SEC_CLASS__BUILDING_UPGRADE:
        if(TRUE)
        {
            SCityBuildingNode* pstNode = CCityBase::GetBuildingAtPos(&tbCity, pstParam->m_ddwPos);
            if(pstNode)
            {
                pstNode->m_ddwType = pstParam->m_ddwType;
                pstNode->m_ddwLevel = pstParam->m_ddwTargetLevel;
                tbCity.SetFlag(TbCITY_FIELD_BUILDING);
            }
            else
            {
                CCityBase::AddBuilding(pstParam->m_ddwPos, pstParam->m_ddwType, pstParam->m_ddwTargetLevel, tbCity);
            }
            //活动积分统计
            CActivitesLogic::ComputeBuildUpgradeScore(pstUser, pstParam->m_ddwType, pstParam->m_ddwTargetLevel);
            CActivitesLogic::ComputeHeroExpBuildingScore(pstUser, pstParam->m_ddwExp);
        }

        if(CCityBase::GetBuildingFuncType(pstParam->m_ddwType) == EN_BUILDING_TYPE__CASTLE)// 修改map中的主城等级
        {
            TbMap& tbTmpMap = pstSession->m_tbTmpMap;
            if (CMapLogic::IfPlayerCity(pstUser, &pstSession->m_tbTmpMap))
            {
                tbTmpMap.Reset();
                tbTmpMap.Set_Sid(pstSession->m_stReqParam.m_udwSvrId);
                tbTmpMap.Set_Id(tbCity.m_nPos);
                tbTmpMap.Set_Level(pstParam->m_ddwTargetLevel);
                pstSession->m_ucTmpMapItemFlag = EN_TABLE_UPDT_FLAG__CHANGE;
            }

            // 主城5级取消新手保护
            TUINT32 udwBreakProtectLv = CGameInfo::GetInstance()->m_oJsonRoot["game_basic"][29U].asUInt();
            //TUINT32 udwBreadProtectAge = CGameInfo::GetInstance()->m_oJsonRoot["game_basic"][54U].asUInt();
            if(pstParam->m_ddwTargetLevel >= udwBreakProtectLv)
            {
                if(tbPlayer.m_nStatus & EN_CITY_STATUS__NEW_PROTECTION)
                {
                    tbPlayer.Set_Status(tbPlayer.m_nStatus & (~EN_CITY_STATUS__NEW_PROTECTION));
                    if (CMapLogic::IfPlayerCity(pstUser, &pstSession->m_tbTmpMap))
                    {
                        tbTmpMap.Set_Status(tbPlayer.m_nStatus);
                    }

                    // 删除action
                    TbAction* ptbAction = CActionBase::GetActionByBufferId(pstUser->m_atbAction, pstUser->m_udwActionNum, EN_BUFFER_INFO_PEACE_TIME);
                    if(ptbAction != NULL)
                    {
                        TINT32 dwActionIdx = CActionBase::GetActionIndex(pstUser->m_atbAction, pstUser->m_udwActionNum, ptbAction->m_nId);
                        pstUser->m_aucActionFlag[dwActionIdx] = EN_TABLE_UPDT_FLAG__DEL;
                    }
                    // 设置通知标记
                    //tbPlayer.Set_Protect(1);
                    tbPlayer.Set_Status(tbPlayer.m_nStatus | EN_CITY_STATUS__NEW_PROTECTION_CANCEL);
                }
            }
        }
        else if(CCityBase::GetBuildingFuncType(pstParam->m_ddwType) == EN_BUILDING_TYPE__EMBASSY)
        {
            if (CMapLogic::IfPlayerCity(pstUser, &pstSession->m_tbTmpMap))
            {
                TbMap& tbTmpMap = pstSession->m_tbTmpMap;
                tbTmpMap.Reset();
                tbTmpMap.Set_Sid(pstSession->m_stReqParam.m_udwSvrId);
                tbTmpMap.Set_Id(tbCity.m_nPos);
                tbTmpMap.Set_Em_lv(pstParam->m_ddwTargetLevel);
                pstSession->m_ucTmpMapItemFlag = EN_TABLE_UPDT_FLAG__CHANGE;
            }
        }

        // tips
        CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPE__BUILDING_OK, tbPlayer.m_nUid, FALSE, pstParam->m_ddwType, pstParam->m_ddwTargetLevel);

        if(pstParam->m_ddwTargetLevel == 1)
        {
            CQuestLogic::SetTaskCurValue(&pstSession->m_stUserInfo, pstCity, EN_TASK_TYPE_ING_BUILDING_NUM);
        }
        CQuestLogic::SetTaskCurValue(&pstSession->m_stUserInfo, pstCity, EN_TASK_TYPE_ING_BUILD_X_LV_BUILDING, 1, 0,  pstParam->m_ddwTargetLevel);

        //cmd
        strncpy(szCmd, "construct_finish", 1024);
        szCmd[1023] = '\0';
        break;
    case EN_ACTION_SEC_CLASS__BUILDING_REMOVE:
        CCityBase::DelBuildingAtPos(&tbCity, pstParam->m_ddwPos);
        //llt add,大使馆拆除,修改地图信息
        if(EN_BUILDING_TYPE__EMBASSY == CCityBase::GetBuildingFuncType(pstParam->m_ddwType))
        {
            if (CMapLogic::IfPlayerCity(pstUser, &pstSession->m_tbTmpMap))
            {
                TbMap& tbTmpMap = pstSession->m_tbTmpMap;
                tbTmpMap.Reset();
                tbTmpMap.Set_Sid(pstSession->m_stReqParam.m_udwSvrId);
                tbTmpMap.Set_Id(tbCity.m_nPos);
                tbTmpMap.Set_Em_lv(0);
                pstSession->m_ucTmpMapItemFlag = EN_TABLE_UPDT_FLAG__CHANGE;
            }
        }
        // tips
        CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPE__REMOVE_BUILDING_OK, tbPlayer.m_nUid, FALSE, pstParam->m_ddwType, pstParam->m_ddwTargetLevel);

        strncpy(szCmd, "deconstruct_finish", 1024);
        szCmd[1023] = '\0';

        break;
    case EN_ACTION_SEC_CLASS__RESEARCH_UPGRADE:
        tbCity.m_bResearch[0].m_addwLevel[pstParam->m_ddwType] = pstParam->m_ddwTargetLevel;
        tbCity.SetFlag(TbCITY_FIELD_RESEARCH);

        // tips
        CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPE__RESEARCH_OK, pstUser->m_tbUserStat.m_nUid, FALSE, pstParam->m_ddwType, pstParam->m_ddwTargetLevel);

        strncpy(szCmd, "research_finish", 1024);
        szCmd[1023] = '\0';

        //活动积分统计
        CActivitesLogic::ComputeResearchUpgradeScore(pstUser, pstParam->m_ddwType, pstParam->m_ddwTargetLevel);
        CActivitesLogic::ComputeHeroExpResearchScore(pstUser, pstParam->m_ddwExp);

        CQuestLogic::SetTaskCurValue(&pstSession->m_stUserInfo, pstCity, EN_TASK_TYPE_ING_RESEARCH_TIME);

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
            CCityBase::DelBuildingAtPos(&tbCity, pstParam->m_ddwPos);
            TINT32 dwObstacleLv = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_OBSTACLE_LEVEL].m_ddwBuffTotal;
            if(dwObstacleLv == 0)
            {
                dwObstacleLv = 1;
            }

            const Json::Value &oBuildingTypeReward = CGameInfo::GetInstance()->m_oJsonRoot["game_building"][CCommonFunc::NumToString(pstParam->m_ddwType)]["b"]["b2"][dwObstacleLv - 1U];

            for(TUINT32 udwIdx = 0; udwIdx < oBuildingTypeReward.size(); ++udwIdx)
            {
                if(oBuildingTypeReward[udwIdx][0U].asUInt() == EN_GLOBALRES_TYPE_BUILDING_OBSTACLE)
                {
                    TUINT32 udwBuildingId = oBuildingTypeReward[udwIdx][1U].asUInt();
                    TUINT32 udwBuildingNum = oBuildingTypeReward[udwIdx][2U].asUInt();

                    TUINT32 udwBuildingMaxNumRate = oBuildingTypeReward[udwIdx][3U].asUInt();
                    TUINT32 udwRandNum = rand() % 10000;

                    TUINT32 udwFinalBuildNum = 0;
                    if(udwRandNum < udwBuildingMaxNumRate)
                    {
                        udwFinalBuildNum = udwBuildingNum;
                    }
                    else if(udwBuildingNum == 0 || udwBuildingNum == 1)
                    {
                        udwFinalBuildNum = 0;
                    }
                    else
                    {
                        udwFinalBuildNum = rand() % (udwBuildingNum - 1);
                    }
                    for(TUINT32 udwNumIdx = 0; udwNumIdx < udwFinalBuildNum; ++udwNumIdx)
                    {
                        CCommonLogic::GenObstle(&tbCity, udwBuildingId, &povVec);
                    }
                }
            }
        }

        // tips
        CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPE__REMOVE_OBSTCLE, tbPlayer.m_nUid, FALSE, pstParam->m_ddwType, 0);

        strncpy(szCmd, "deconstruct_finish", 1024);
        szCmd[1023] = '\0';
        break;
    default:
        break;
    }

    //add_reward
    SSpGlobalRes stGlobalRes;
    stGlobalRes.Reset();
    TINT32 dwRetCode = 0;
    if(EN_ACTION_SEC_CLASS__RESEARCH_UPGRADE == (int)enSecClass)
    {
        dwRetCode = CGlobalResLogic::GetResearchReward(pstParam->m_ddwType, pstParam->m_ddwTargetLevel, &stGlobalRes);
    }
    if(EN_ACTION_SEC_CLASS__BUILDING_UPGRADE == (int)enSecClass)
    {
        dwRetCode = CGlobalResLogic::GetBuildingReward(pstParam->m_ddwType, pstParam->m_ddwTargetLevel, &stGlobalRes);
    }
    if(EN_ACTION_SEC_CLASS__REMOVE_OBSTACLE == (int)enSecClass)
    {
        dwRetCode = CGlobalResLogic::GetObstleReward(pstParam->m_ddwType, 1, &stGlobalRes);
    }
    if(dwRetCode != 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("BuildingActionDone:get  reward fail ret_code=%d type=%ld lv=%ld [seq=%u]",
            dwRetCode,
            pstParam->m_ddwType,
            pstParam->m_ddwTargetLevel,
            pstSession->m_udwSeqNo));
    }

    dwRetCode = CGlobalResLogic::AddSpGlobalRes(pstUser, pstCity, &stGlobalRes);
    if(dwRetCode != 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("BuildingActionDone:add  reward fail ret_code=%d type=%ld lv=%ld [seq=%u]",
            dwRetCode,
            pstParam->m_ddwType,
            pstParam->m_ddwTargetLevel,
            pstSession->m_udwSeqNo));
    }

    // 4.1 cb log key
    TCHAR szKey[1024];
    snprintf(szKey, 1024, "%ld|%ld|%ld|%ld",
        pstParam->m_ddwPos,
        pstParam->m_ddwType,
        pstParam->m_ddwTargetLevel,
        pstParam->m_ddwExp);

    // 4.2 cb log
    if((TRUE == bCbLog) && (0 != strcmp(szCmd, "")))
    {
        CHuCBLog::OutLogCbLog(pstSession, szCmd, szKey);
    }

    return 0;
}

TINT32 CProcessAction::TrainActionDoneNew(SSession *pstSession, SUserInfo *pstUser, SCityInfo *pstCity, EActionSecClass enSecClass, TUINT32 udwActionIdx)
{
    STroopInfo stTroopInfo;
    CGameInfo *poGameInfo = CGameInfo::GetInstance();
    TbPlayer& tbPlayer = pstUser->m_tbPlayer;
    TbAlliance_action& tbAction = pstUser->m_atbSelfAlAction[udwActionIdx];
    SActionTrainParam *pstParam = &tbAction.m_bParam[0].m_stTrain;
    TbCity& tbCity = pstCity->m_stTblData;
    SCommonTroop& stTroop = tbCity.m_bTroop[0];
    SCommonFort& stFort = tbCity.m_bFort[0];

    UActionParam stActionParam;
    TCHAR szCmd[1024];
    szCmd[0] = '\0';

    // 1. get fort info
    if (enSecClass == EN_ACTION_SEC_CLASS__FORT
        || enSecClass == EN_ACTION_SEC_CLASS__FORT_REPAIR)
    {
        if (FALSE == poGameInfo->GetFortInfo(pstParam->m_ddwType, &stTroopInfo))
        {
            assert(0);
            return -1;
        }
    }
    else if (enSecClass == EN_ACTION_SEC_CLASS__TROOP
        || enSecClass == EN_ACTION_SEC_CLASS__HOS_TREAT)
    {
        if (FALSE == poGameInfo->GetTroopInfo(pstParam->m_ddwType, &stTroopInfo))
        {
            assert(0);
            return -2;
        }
    }
    else
    {
        stTroopInfo.Reset();  //lucien note:这里需要直接返回吗？
    }

    // 2. add troop or fort
    switch ((int)enSecClass)
    {
    case EN_ACTION_SEC_CLASS__TROOP:
        // add troop
        stTroop.m_addwNum[pstParam->m_ddwType] += pstParam->m_ddwNum;
        tbCity.SetFlag(TbCITY_FIELD_TROOP);
        // tips
        CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPE__TRAIN_TROOP_OK, tbPlayer.m_nUid, FALSE, pstParam->m_ddwType, pstParam->m_ddwNum);
        strncpy(szCmd, "troop_train_finish", 1024);
        szCmd[1023] = '\0';

        //task count
        CQuestLogic::SetTaskCurValue(pstUser, pstCity, EN_TASK_TYPE_ING_TROOP_TRAIN_NUM, pstParam->m_ddwNum, 0, CToolBase::GetTroopLvByTroopId(pstParam->m_ddwType));

        //活动积分统计
        CActivitesLogic::ComputeTrainTroopScore(pstUser, (TUINT32)pstParam->m_ddwType, pstParam->m_ddwNum);

        break;
    case EN_ACTION_SEC_CLASS__FORT:
        // add fort
        stFort.m_addwNum[pstParam->m_ddwType] += pstParam->m_ddwNum;
        tbCity.SetFlag(TbCITY_FIELD_FORT);
        // tips
        CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPE__TRAIN_FORT_OK, tbPlayer.m_nUid, FALSE, pstParam->m_ddwType, pstParam->m_ddwNum);

        strncpy(szCmd, "fort_train_finish", 1024);
        szCmd[1023] = '\0';

        //task count
        CQuestLogic::SetTaskCurValue(pstUser, pstCity, EN_TASK_TYPE_ING_FORT_TRAIN_NUM, pstParam->m_ddwNum, 0, CToolBase::GetFortLvByFortId(pstParam->m_ddwType));

        //活动积分统计
        CActivitesLogic::ComputeTrainFortScore(pstUser, (TUINT32)pstParam->m_ddwType, pstParam->m_ddwNum);

        break;
    case EN_ACTION_SEC_CLASS__HOS_TREAT:
        // add troop
        stTroop.m_addwNum[pstParam->m_ddwType] += pstParam->m_ddwNum;
        tbCity.SetFlag(TbCITY_FIELD_TROOP);
        // tips
        CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPE__HEAL_TROOP_OK, tbPlayer.m_nUid, FALSE, pstParam->m_ddwType, pstParam->m_ddwNum);
        strncpy(szCmd, "troop_heal_finish", 1024);
        szCmd[1023] = '\0';

        //活动积分统计
        CActivitesLogic::ComputeHealTroopScore(pstUser, (TUINT32)pstParam->m_ddwType, pstParam->m_ddwNum);

        break;
    case EN_ACTION_SEC_CLASS__FORT_REPAIR:
        // add fort
        stFort.m_addwNum[pstParam->m_ddwType] += pstParam->m_ddwNum;
        tbCity.SetFlag(TbCITY_FIELD_FORT);
        // tips
        CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPE__HEAL_FORT_OK, tbPlayer.m_nUid, FALSE, pstParam->m_ddwType, pstParam->m_ddwNum);

        //活动积分统计
        CActivitesLogic::ComputeHealFortScore(pstUser, (TUINT32)pstParam->m_ddwType, pstParam->m_ddwNum);

        strncpy(szCmd, "fort_heal_finish", 1024);
        szCmd[1023] = '\0';


        break;
    }

    // 4. 用户exp增加
    CPlayerBase::AddLordExp(pstUser, pstCity, pstParam->m_ddwExp);

    // 5. updt stat
    if (enSecClass == EN_ACTION_SEC_CLASS__FORT
        || enSecClass == EN_ACTION_SEC_CLASS__TROOP)
    {
        TUINT64 uddwGainedMight = pstParam->m_ddwNum * stTroopInfo.m_dwMight;
        tbPlayer.Set_Mgain(tbPlayer.m_nMgain + uddwGainedMight);
    }

    // 6.1 cb log key
    TCHAR szKey[1024];
    snprintf(szKey, 1024, "%ld|%ld|%ld", pstParam->m_ddwType,
        pstParam->m_ddwNum,
        pstParam->m_ddwExp);

    // 6.2 cb log
    if (0 != strcmp(szCmd, ""))
    {
        CHuCBLog::OutLogCbLog(pstSession, szCmd, szKey);
    }

    return 0;
}

TINT32 CProcessAction::TrainCancelNew(SSession *pstSession, SUserInfo *pstUser, SCityInfo *pstCity, TbAlliance_action* ptbAction)
{
    CGameInfo *poGameInfo = CGameInfo::GetInstance();
    STroopInfo stTrainInfo;
    TINT64 addwResource[EN_RESOURCE_TYPE__END];
    TUINT32 idx = 0;
    TFLOAT32 fResRate = 1.0;


    SActionTrainParam& stTrainParam = ptbAction->m_bParam[0].m_stTrain;

    // 1. 准备参数
    switch (ptbAction->m_nSclass)
    {
    case EN_ACTION_SEC_CLASS__TROOP:
        if (FALSE == poGameInfo->GetTroopInfo(stTrainParam.m_ddwType, &stTrainInfo))
        {
            return -1;
        }
        break;
    case EN_ACTION_SEC_CLASS__FORT:
        if (FALSE == poGameInfo->GetFortInfo(stTrainParam.m_ddwType, &stTrainInfo))
        {
            return -2;
        }
        break;
    case EN_ACTION_SEC_CLASS__HOS_TREAT:
        if (FALSE == poGameInfo->GetHealTroopInfo(stTrainParam.m_ddwType, &stTrainInfo))
        {
            return -3;
        }
        break;
    case EN_ACTION_SEC_CLASS__FORT_REPAIR:
        if (FALSE == poGameInfo->GetHealFortInfo(stTrainParam.m_ddwType, &stTrainInfo))
        {
            return -4;
        }
        break;
    default:
        break;
    }

    // 计算资源
    for (idx = 0; idx < EN_RESOURCE_TYPE__END; idx++)
    {
        addwResource[idx] = 0.5*fResRate*stTrainInfo.m_audwBaseCost[idx] * stTrainParam.m_ddwNum;
    }

    TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("original resource [nowresource=%ld:%ld:%ld:%ld:%ld] [makeupresource=%ld:%ld:%ld:%ld:%ld] [seq=%u]", 
        pstCity->m_stTblData.m_bResource[0].m_addwNum[EN_RESOURCE_TYPE__GOLD],
        pstCity->m_stTblData.m_bResource[0].m_addwNum[EN_RESOURCE_TYPE__FOOD],
        pstCity->m_stTblData.m_bResource[0].m_addwNum[EN_RESOURCE_TYPE__WOOD],
        pstCity->m_stTblData.m_bResource[0].m_addwNum[EN_RESOURCE_TYPE__STONE],
        pstCity->m_stTblData.m_bResource[0].m_addwNum[EN_RESOURCE_TYPE__ORE],
        addwResource[EN_RESOURCE_TYPE__GOLD], addwResource[EN_RESOURCE_TYPE__FOOD],
        addwResource[EN_RESOURCE_TYPE__WOOD], addwResource[EN_RESOURCE_TYPE__STONE],
        addwResource[EN_RESOURCE_TYPE__ORE], pstUser->m_udwBSeqNo));

    // 补回资源
    CCityBase::AddResource(pstCity, &addwResource[0]);

    TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("final resource [nowresource=%ld:%ld:%ld:%ld:%ld] [seq=%u]", \
        pstCity->m_stTblData.m_bResource[0].m_addwNum[EN_RESOURCE_TYPE__GOLD],
        pstCity->m_stTblData.m_bResource[0].m_addwNum[EN_RESOURCE_TYPE__FOOD],
        pstCity->m_stTblData.m_bResource[0].m_addwNum[EN_RESOURCE_TYPE__WOOD],
        pstCity->m_stTblData.m_bResource[0].m_addwNum[EN_RESOURCE_TYPE__STONE],
        pstCity->m_stTblData.m_bResource[0].m_addwNum[EN_RESOURCE_TYPE__ORE],
        pstUser->m_udwBSeqNo));

    // 从action中删除
    CCommonBase::SetActionDeleteFlag(pstUser, ptbAction->m_nId, ptbAction->m_nSclass);

    // 为运营添加log key
    TBOOL bHead = TRUE;
    TUINT32 udwLen = 0;
    for (idx = 0; idx < EN_RESOURCE_TYPE__END; ++idx)
    {
        if (bHead)
        {
            udwLen = snprintf(pstSession->m_stReqParam.m_szKey[1], 1024, "%ld", addwResource[idx]);
            bHead = FALSE;
        }
        else
        {
            udwLen += snprintf(pstSession->m_stReqParam.m_szKey[1] + udwLen, 1024 - udwLen, ":%ld", addwResource[idx]);
        }
    }

    return 0;
}

TVOID CProcessAction::ActionDone(SSession *pstSession, SUserInfo *pstUser, SCityInfo *pstCity, TUINT32 udwMainClass,TUINT32 udwSecClass, TINT32 dwActionIdx, TBOOL bCbLog)
{
    TINT64 ddwMaxEnergy = 0;
    TUINT32 ddwActionEndTime = CTimeUtils::GetUnixTime();
    switch(udwMainClass)
    {
    case EN_ACTION_MAIN_CLASS__BUILDING:
        CProcessAction::BuildingActionDone(pstSession, pstUser, pstCity, (EActionSecClass)udwSecClass, &pstUser->m_atbSelfAlAction[dwActionIdx].m_bParam[0].m_stBuilding, bCbLog);
        CCommonBase::SetActionDeleteFlag(pstUser, pstUser->m_atbSelfAlAction[dwActionIdx].m_nId, udwSecClass);
        break;
    case EN_ACTION_MAIN_CLASS__EQUIP:
        CProcessAction::EquipUpgradeActionDone(pstSession, pstUser, pstCity, &pstUser->m_atbSelfAlAction[dwActionIdx].m_bParam[0].m_stEquip, bCbLog);
        CCommonBase::SetActionDeleteFlag(pstUser, pstUser->m_atbSelfAlAction[dwActionIdx].m_nId, udwSecClass);
        break;
    case EN_ACTION_MAIN_CLASS__TRAIN_NEW:
        CProcessAction::TrainActionDoneNew(pstSession, pstUser, pstCity, (EActionSecClass)udwSecClass, dwActionIdx);
        CCommonBase::SetActionDeleteFlag(pstUser, pstUser->m_atbSelfAlAction[dwActionIdx].m_nId, udwSecClass);
        break;
    case EN_ACTION_MAIN_CLASS__DRAGON:
        if (pstUser->m_atbSelfAlAction[dwActionIdx].m_nSclass == EN_ACTION_SEC_CLASS__UNLOCK_DRAGON)
        {
            ddwMaxEnergy = CPlayerBase::GetCurDragonMaxEnergy(pstUser);
            CPlayerBase::AddDragon(&pstUser->m_tbPlayer, ddwMaxEnergy);
            CCommonBase::SetActionDeleteFlag(pstUser, pstUser->m_atbSelfAlAction[dwActionIdx].m_nId, udwSecClass);
            CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPE__UNLOCK_DRAGON, pstUser->m_tbPlayer.m_nUid, FALSE);
            // 发送邮件
            CMsgBase::SendOperateMail(pstUser->m_tbPlayer.m_nUid, EN_MAIL_ID__DRAGON_UNLOCK, pstSession->m_stReqParam.m_udwSvrId,
                SYSTEM_ENCOURAGE, "", "", "");
        }
        break;
    default:
        if(udwMainClass == EN_ACTION_MAIN_CLASS__MARCH)
        {
            TbMarch_action *ptbAction = &pstUser->m_atbMarch[dwActionIdx];
            switch(ptbAction->m_nStatus)
            {
            case EN_MARCH_STATUS__DEFENDING:
            case EN_MARCH_STATUS__LOADING:
                break;
            default:
                // 更新主动列表
                ptbAction->Set_Etime(ddwActionEndTime);
                pstUser->m_aucMarchFlag[dwActionIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
                // 如果是主动进攻方的rally reinforce, 更新rallywar的状态

                // 更新被动列表中的内容——不用更新到下游
                dwActionIdx = CActionBase::GetMarchIndex(pstUser->m_atbPassiveMarch, pstUser->m_udwPassiveMarchNum, ptbAction->Get_Id());
                if(dwActionIdx >= 0)
                {
                    pstUser->m_atbPassiveMarch[dwActionIdx].m_nEtime = ddwActionEndTime;
                }
                break;
            }
        }
        break;
    }

    return;
}

TINT32 CProcessAction::EquipUpgradeActionDone(SSession *pstSession, SUserInfo *pstUser, SCityInfo *pstCity, SActionEquipParam *pstParam, TBOOL bCbLog)
{
    TUINT64 uddwId = pstParam->m_uddwId;
    TCHAR szCmd[1024];
    szCmd[0] = '\0';
    TINT32 dwEquipIdx = -1;

    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwEquipNum;++udwIdx)
    {
        if(pstUser->m_aucEquipFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }
        if(pstUser->m_atbEquip[udwIdx].m_nId == static_cast<TINT64>(uddwId))
        {
            dwEquipIdx = udwIdx;
            break;
        }
    }
    if(dwEquipIdx == -1)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("EquipUpgradeActionDone:player backpack have not such equip id[id=%ld e_id=%u] [seq=%u]",
             pstParam->m_uddwId, pstParam->m_ddwEType, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }
    pstUser->m_atbEquip[dwEquipIdx].Set_Status(EN_EQUIPMENT_STATUS_NORMAL);
    pstUser->m_atbEquip[dwEquipIdx].Set_Get_time(CTimeUtils::GetUnixTime());
    pstUser->m_aucEquipFlag[dwEquipIdx] = EN_TABLE_UPDT_FLAG__CHANGE;

    strncpy(szCmd, "compose_equip", 1024);
    szCmd[1023] = '\0';


    // cb log key
    TCHAR szKey[1024];
    snprintf(szKey, 1024, "%lu|%ld",
        pstParam->m_uddwId,
        pstParam->m_ddwEType);

    //cb log
    if((TRUE == bCbLog)
        && (0 != strcmp(szCmd, "")))
    {
        CHuCBLog::OutLogCbLog(pstSession, szCmd, szKey);
    }
    SEquipMentInfo stEquip;
    stEquip.Reset();

    CBackpack::GetEquipBaseInfoByEid(pstParam->m_ddwEType, &stEquip);
    //活动积分统计
    CActivitesLogic::ComputeEquipUpgradeScore(pstUser, stEquip.stBaseInfo.udwLv);

    CQuestLogic::SetTaskCurValue(&pstSession->m_stUserInfo, pstCity, EN_TASK_TYPE_ING_COMPOSE_EQUIP);

    CMsgBase::SendEncourageMail(&pstUser->m_tbUserStat, pstUser->m_tbPlayer.m_nSid, EN_MAIL_ID__FIRST_COMPOSE_EQUIP, 0, CCommonFunc::NumToString(stEquip.stBaseInfo.udwEType));

    //返回给客户端
    SGlobalRes stReward;
    stReward.ddwTotalNum = 1;
    stReward[0].ddwType = EN_GLOBALRES_TYPE_EQUIP;
    stReward[0].ddwId = pstParam->m_uddwId;
    stReward[0].ddwNum = 1;

    Json::Value jTmp = Json::Value(Json::objectValue);
    jTmp["equip"] = Json::Value(Json::arrayValue);
    jTmp["equip"][0U] = pstParam->m_ddwLevel;

    CCommonBase::AddRewardWindow(pstUser, pstUser->m_tbPlayer.m_nUid, EN_REWARD_WINDOW_TYPE_ONE_EQUIP, EN_REWARD_WINDOW_GET_TYPE_COMPOSE, 0, &stReward, FALSE, jTmp);

    return 0;
}

TINT32 CProcessAction::EquipActionCancel(SSession *pstSession, SUserInfo *pstUser, SCityInfo *pstCity, EActionSecClass enSecClass, SActionEquipParam *pstParam)
{
    const Json::Value &oMaterialJson = CBackpackInfo::GetInstance()->m_oJsonRoot["equip_material"];
    const Json::Value &oScrollJson = CBackpackInfo::GetInstance()->m_oJsonRoot["equip_scroll"];

    vector<TUINT32> vecMaterialId;
    CCommonFunc::GetVectorFromString(pstParam->m_szMaterialIdList, ':', vecMaterialId);

    vector<TUINT32> vecPartsId;
    CCommonFunc::GetVectorFromString(pstParam->m_szPartsIdList, ':', vecPartsId);

    TUINT32 udwScrollId = pstParam->m_ddwScrollId;

    TUINT32 udwCostGold = pstParam->m_ddwGoldCost;

    //返还各种材料
    for(TUINT32 udwIdx = 0; udwIdx < vecMaterialId.size(); ++udwIdx)
    {
        if(vecMaterialId[udwIdx] == 0)
        {
            continue;
        }
        
        if(oMaterialJson.isMember(CCommonFunc::NumToString(vecMaterialId[udwIdx])))
        {
            CBackpack::AddMaterial(pstUser, vecMaterialId[udwIdx]);
        }
    }
    if(udwScrollId != 0)
    {
        if(oScrollJson.isMember(CCommonFunc::NumToString(udwScrollId)))
        {
            CBackpack::AddScroll(pstUser, udwScrollId);
        }
    }

    CCityBase::AddGold(pstCity, udwCostGold / 2);

    //清除背包数据
    TINT64 ddwEquipId = pstParam->m_uddwId;
    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwEquipNum;++udwIdx)
    {
        if(pstUser->m_atbEquip[udwIdx].m_nId == ddwEquipId)
        {
            pstUser->m_aucEquipFlag[udwIdx] = EN_TABLE_UPDT_FLAG__DEL;
        }
    }

    return 0;
}