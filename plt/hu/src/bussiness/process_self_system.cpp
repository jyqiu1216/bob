#include "process_self_system.h"
#include "game_info.h"
#include "city_base.h"
#include "action_base.h"
#include "procedure_base.h"
#include "process_alliance.h"
#include "player_base.h"
#include "process_action.h"
#include "common_func.h"
#include "backpack_info.h"
#include "activities_logic.h"
#include "wild_info.h"
#include "common_base.h"
#include "map_logic.h"
#include "map_base.h"
#include "common_logic.h"
#include "globalres_logic.h"
#include "tool_base.h"
#include "sendmessage_base.h"
#include "db_request.h"
#include "msg_base.h"
#include "conf_base.h"
#include "backpack_logic.h"
#include "quest_logic.h"
#include "dragon_trail_control.h"
#include "global_gift.h"
#include "process_throne.h"
#include "item_base.h"
#include "item_logic.h"

// #define SUPPORT_DATABASE_NAME "test"
// #define SUPPORT_TABLE_NAME "user_time"
#define SUPPORT_SQL_FNAME "update.sql"


TINT32 CProcessSelfSystem::Processcmd_CleanEquipGrid(SSession *pstSession, TBOOL &bNeedResponse)
{

    SUserInfo *pstUserInfo = &pstSession->m_stUserInfo;
    
    TUINT32 udwInitEquipGridNum = CGameInfo::GetInstance()->m_oJsonRoot["game_init_data"]["r"]["a5"].asUInt();

    if(pstUserInfo->m_tbUserStat.m_nEquip_gride == udwInitEquipGridNum)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__EQUIP_NUM_MAX;
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("Processcmd_CleanEquipGrid: init grid num[seq=%u] ",
                                                          pstSession->m_udwSeqNo));
        return -1;     
    }
    
    TINT32 dwCanCleanEquipGrid = 0;
    TINT32 dwNeedEquipGridRow = 0;
    if(pstUserInfo->m_udwEquipNum <= udwInitEquipGridNum)
    {
        dwCanCleanEquipGrid = pstUserInfo->m_tbUserStat.m_nEquip_gride - udwInitEquipGridNum;
    }
    else
    {
        dwNeedEquipGridRow = (pstUserInfo->m_udwEquipNum - udwInitEquipGridNum) % 4;
        dwCanCleanEquipGrid = pstUserInfo->m_tbUserStat.m_nEquip_gride - dwNeedEquipGridRow * 4 - udwInitEquipGridNum;
    }
    
    if(dwCanCleanEquipGrid <= 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__EQUIP_NUM_MAX;
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("Processcmd_CleanEquipGrid: not grid will clean [seq=%u] ",
                                                          pstSession->m_udwSeqNo));
        return -2;     
    }
    
    
    pstUserInfo->m_tbUserStat.Set_Equip_gride(pstUserInfo->m_tbUserStat.m_nEquip_gride - dwCanCleanEquipGrid);

    return 0;

}



TINT32 CProcessSelfSystem::Processcmd_SetGemNum(SSession *pstSession, TBOOL &bNeedResponse)
{
    //解析参数
    TINT64 ddwSetGemNum = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TbLogin *pstAccount = &pstSession->m_stUserInfo.m_tbLogin;

    //条件判断
    if(0 > ddwSetGemNum)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_SetGemNum: SetGemNum is illegal [getnum=%ld] [seq=%u]", \
            ddwSetGemNum, \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }

    pstAccount->Set_Gem(ddwSetGemNum);
    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_SetResource(SSession *pstSession, TBOOL &bNeedResponse)
{
    //提取参数
    TINT32 dwType = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TINT64 ddwNum = strtoll(pstSession->m_stReqParam.m_szKey[1], NULL, 10);

    //获取必要的内容
    SUserInfo *pstUserInfo = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUserInfo->m_stCityInfo;

    TSE_LOG_INFO(pstSession->m_poServLog, ("Processcmd_SetResource: start [type=%d] [ddwNum=%ld] [seq=%u]", \
        dwType, \
        ddwNum, \
        pstSession->m_stUserInfo.m_udwBSeqNo));

    //条件判断
    if(NULL == pstCity)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_SetResource: pstCity is null,[seq=%u]", \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }
    if(EN_RESOURCE_TYPE__GOLD > dwType
        || EN_RESOURCE_TYPE__END <= dwType
        || 0 > ddwNum)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("CProcessSelfSystem::Processcmd_SetResource parameter is illegal [type=%d] [ddwNum=%ld] [seq=%u]", \
            dwType, \
            ddwNum, \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -2;
    }

    if(MAX_RESOURCE_LIMIT_NUM < ddwNum)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_SetResource: ddwNum is greater than max resource num [ddwNum=%ld] [seq=%u]", \
            ddwNum, \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -3;
    }

    // set res        
    pstCity->m_stTblData.m_bResource[0].m_addwNum[dwType] = ddwNum;
    pstCity->m_stTblData.SetFlag(TbCITY_FIELD_RESOURCE);

    TSE_LOG_INFO(pstSession->m_poServLog, ("Processcmd_SetResource: end [seq=%u]", \
        pstSession->m_stUserInfo.m_udwBSeqNo));
    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_SetTroop(SSession *pstSession, TBOOL &bNeedResponse)
{ 
    TINT32 dwTroopType = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TINT64 ddwTroopNum = strtoll(pstSession->m_stReqParam.m_szKey[1], NULL, 10);

    SUserInfo *pstUserInfo = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUserInfo->m_stCityInfo;

    TSE_LOG_INFO(pstSession->m_poServLog, ("Processcmd_SetTroop: [dwTroopType=%d] [ddwTroopNum=%ld] [seq=%u]", \
        dwTroopType, \
        ddwTroopNum, \
        pstSession->m_stUserInfo.m_udwBSeqNo));
    //条件判断
    if(NULL == pstCity)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_SetTroop: pstCity is null [seq=%u]", \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }

    if(EN_TROOP_TYPE__T1_INFANTRY > dwTroopType
        || EN_TROOP_TYPE__END <= dwTroopType
        || 0 > ddwTroopNum)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_SetTroop: parameter is illegal [dwTroopType=%d] [ddwTroopNum=%d] [seq=%u]", \
            dwTroopType, \
            ddwTroopNum, \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -2;
    }

    pstCity->m_stTblData.m_bTroop[0].m_addwNum[dwTroopType] = ddwTroopNum;
    pstCity->m_stTblData.SetFlag(TbCITY_FIELD_TROOP);

    TSE_LOG_INFO(pstSession->m_poServLog, ("Processcmd_SetTroop: end [seq=%u]", \
        pstSession->m_stUserInfo.m_udwBSeqNo));
    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_SetFort(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwFortType = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TINT32 ddwFortNum = strtoll(pstSession->m_stReqParam.m_szKey[1], NULL, 10);
    TSE_LOG_INFO(pstSession->m_poServLog, ("Processcmd_SetFort: start [dwFortType=%d] [ddwFortNum=%ld] [seq=%u]", \
        dwFortType, \
        ddwFortNum, \
        pstSession->m_stUserInfo.m_udwBSeqNo));

    SUserInfo *pstUserInfo = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUserInfo->m_stCityInfo;
    TbCity *ptbCity = &pstCity->m_stTblData;
    if(NULL == ptbCity)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_SetFort: ptbCity is null [seq=%u]", \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }
    if(EN_FORT_TYPE__END < dwFortType
        || EN_FORT_TYPE__TRAPS > dwFortType
        || 0 > ddwFortNum)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_SetFort: parameter is illegal [seq=%u]", \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -2;
    }

    ptbCity->m_bFort[0].m_addwNum[dwFortType] = ddwFortNum;
    pstCity->m_stTblData.SetFlag(TbCITY_FIELD_FORT);

    TSE_LOG_INFO(pstSession->m_poServLog, ("Processcmd_SetFort end [seq=%u]", \
        pstSession->m_stUserInfo.m_udwBSeqNo));
    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_SetItem(SSession *pstSession, TBOOL &bNeedResponse)
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

    for(udwIdx = 0; udwIdx < ptbBackPack->m_bItem.m_udwNum; ++udwIdx)
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

TINT32 CProcessSelfSystem::Processcmd_SetFund(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT64 ddwFundNum = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 10);

    TSE_LOG_INFO(pstSession->m_poServLog, ("Processcmd_SetFund: start [ddwFundNum=%ld] [seq=%u]", \
        ddwFundNum, \
        pstSession->m_stUserInfo.m_udwBSeqNo));

    if(0 > ddwFundNum)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_SetFund: parameter is illegal [seq=%u]", \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }
    pstSession->m_stReqParam.m_udwAllianceId = pstSession->m_stUserInfo.m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET;
    if(0 == pstSession->m_stReqParam.m_udwAllianceId || pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos == EN_ALLIANCE_POS__REQUEST)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_SetFund: m_stReqParam.m_udwAllianceId is 0 or not in an alliance[seq=%u]", \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -2;
    }

    TbAlliance *ptbAlliance = &pstSession->m_stUserInfo.m_tbAlliance;
    ptbAlliance->Set_Fund(ddwFundNum); //lucien note:是否要更新al mem表中的字段？

    TSE_LOG_INFO(pstSession->m_poServLog, ("Processcmd_SetFund: end [seq=%u]", \
        pstSession->m_stUserInfo.m_udwBSeqNo));
    return 0;
}


TINT32 CProcessSelfSystem::Processcmd_SetLoyalty(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT64 ddwLoyalty = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TSE_LOG_INFO(pstSession->m_poServLog, ("Processcmd_SetLoyalty: start [dwLoyalty=%d] [seq=%u]", \
        ddwLoyalty, \
        pstSession->m_stUserInfo.m_udwBSeqNo));

    if(0 > ddwLoyalty)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_SetLoyalty: parameter is illegal [seq=%u]", \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }

    pstSession->m_stReqParam.m_udwAllianceId = pstSession->m_stUserInfo.m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET;
    if(0 == pstSession->m_stReqParam.m_udwAllianceId || pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos == EN_ALLIANCE_POS__REQUEST)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_SetLoyalty: m_stReqParam.m_udwAllianceId is 0 or not in an alliance[seq=%u]", \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -2;
    }

    pstSession->m_stUserInfo.m_tbPlayer.Set_Loy_cur(ddwLoyalty); //lucien note:是否要改al mem表中的字段？
    pstSession->m_stUserInfo.m_tbPlayer.Set_Loy_all(ddwLoyalty);

    // next procedure
    TSE_LOG_INFO(pstSession->m_poServLog, ("Processcmd_SetLoyalty: end [seq=%u]", \
        pstSession->m_stUserInfo.m_udwBSeqNo));
    return 0;
}


TINT32 CProcessSelfSystem::Processcmd_SetBuildingLv(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwPosition = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TINT32 dwType = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TINT32 dwLevel = atoi(pstSession->m_stReqParam.m_szKey[2]);

    TSE_LOG_INFO(pstSession->m_poServLog, ("Processcmd_SetBuildingLv: start [dwPosition=%d] [dwType=%d] [dwLevel=%d] [seq=%u]", \
        dwPosition, \
        dwType, \
        dwLevel, \
        pstSession->m_stUserInfo.m_udwBSeqNo));


    SUserInfo *pstUserInfo = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUserInfo->m_stCityInfo;
    TbCity *ptbCity = &pstCity->m_stTblData;

    if(dwLevel < 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_SetBuildingLv: level is illegal [dwLevel=%d] [seq=%u]", \
            dwLevel, \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;

    }

    if(dwLevel == 0)
    {
        CCityBase::DelBuildingAtPos(ptbCity, dwPosition);
        return 0;
    }

    SCityBuildingNode* pstBuildingNode = CCityBase::GetBuildingAtPos(ptbCity, dwPosition);
    if(pstBuildingNode && pstBuildingNode->m_ddwType != dwType)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_SetBuildingLv: current position has building and type is different [dwType=%d] [dwcurType=%d] [seq=%u]", \
            dwType, \
            pstBuildingNode->m_ddwType, \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -3;

    }
    // 5. 执行upgrade
    UActionParam stParam;
    stParam.m_stBuilding.SetValue(dwPosition, dwType, dwLevel, 0, (TCHAR *)pstSession->m_stUserInfo.m_tbPlayer.m_sUin.c_str());
    CProcessAction::BuildingActionDone(pstSession, &pstSession->m_stUserInfo, pstCity, EN_ACTION_SEC_CLASS__BUILDING_UPGRADE, &stParam.m_stBuilding);

    ptbCity->SetFlag(TbCITY_FIELD_BUILDING);

    TSE_LOG_INFO(pstSession->m_poServLog, ("Processcmd_SetBuildingLv: end [seq=%u]", \
        pstSession->m_stUserInfo.m_udwBSeqNo));
    return 0;
}


TINT32 CProcessSelfSystem::Processcmd_SetResearchLv(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwType = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TINT32 dwLevel = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TSE_LOG_INFO(pstSession->m_poServLog, ("Processcmd_SetResearchLv: start [dwType=%d] [dwLevel=%d] [seq=%u]", \
        dwType, \
        dwLevel, \
        pstSession->m_stUserInfo.m_udwBSeqNo));

    SUserInfo *pstUserInfo = &pstSession->m_stUserInfo;
    TbCity *ptbCity = &pstUserInfo->m_stCityInfo.m_stTblData;

    if(dwType < 0
        || dwType > EN_RESEARCH_TYPE__END)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_SetResearchLv: research type error [dwType=%d] [seq=%u]", \
            dwType, \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -2;
    }

    if(dwLevel < 0 || dwLevel > MAX_RESEARCH_LEVEL)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -3;
    }

    ptbCity->m_bResearch[0].m_addwLevel[dwType] = dwLevel;
    ptbCity->SetFlag(TbCITY_FIELD_RESEARCH);

    TSE_LOG_INFO(pstSession->m_poServLog, ("Processcmd_SetResearchLv: end [seq=%u]", \
        pstSession->m_stUserInfo.m_udwBSeqNo));
    return 0;
}

TINT32 CProcessSelfSystem::ProcessCmd_AddClearScroll(SSession *pstSession, TBOOL &bNeedResponse)
{

    TINT64 ddwOperateType = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 10);

    TbBackpack *ptbBackpack = &pstSession->m_stUserInfo.m_tbBackpack;
    CBackpackInfo *poBackpackInfo = CBackpackInfo::GetInstance();


    TUINT32 udwNum = atoi(pstSession->m_stReqParam.m_szKey[1]);

    TbCity *ptbCity = &pstSession->m_stUserInfo.m_stCityInfo.m_stTblData;

    if(EN_ADD_OPERATE == ddwOperateType)
    {
        Json::Value::Members vecMembers = poBackpackInfo->m_oJsonRoot["equip_scroll_type"].getMemberNames();
        if(0 == vecMembers.size())
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AddClearScroll: equip.json scroll is null [seq=%u]", \
                                                    pstSession->m_stUserInfo.m_udwBSeqNo));
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -2;
        }
        else
        {
            for(TUINT32 udwIdx = 0; udwIdx < vecMembers.size(); ++udwIdx)
            {
                ptbBackpack->m_jScroll[vecMembers[udwIdx]] = udwNum;
            }
        }
    }
    else if(EN_CLEAR_OPERATE == ddwOperateType)
    {
        ptbBackpack->m_jScroll.clear();                   
    }
    else
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AddClearScroll: operate type error [seq=%u]", \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -4;
    }

    ptbBackpack->SetFlag(TbBACKPACK_FIELD_SCROLL);

    return 0;

}



TINT32 CProcessSelfSystem::ProcessCmd_AddClearTroop(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT64 ddwOperateType = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    CGameInfo *poGameInfo = CGameInfo::GetInstance();

    TUINT32 udwNum = atoi(pstSession->m_stReqParam.m_szKey[1]);

    TbCity *ptbCity = &pstSession->m_stUserInfo.m_stCityInfo.m_stTblData;

    if(EN_ADD_OPERATE == ddwOperateType)
    {
        TINT64 ddwTroopTotalNum = poGameInfo->GetTroopTypeNum();
        if(0 == ddwTroopTotalNum)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AddClearTroop: game.json troop is null [seq=%u]", \
                pstSession->m_stUserInfo.m_udwBSeqNo));
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -2;
        }
        else
        {
            for(TINT64 ddwIdx = 0; ddwIdx < ddwTroopTotalNum; ++ddwIdx)
            {
                ptbCity->m_bTroop[0].m_addwNum[ddwIdx] = udwNum;
            }
        }
    }
    else if(EN_CLEAR_OPERATE == ddwOperateType)
    {

        TINT64 ddwTroopTotalNum = poGameInfo->GetTroopTypeNum();
        if(0 == ddwTroopTotalNum)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AddClearTroop: game.json troop is null [seq=%u]", \
                pstSession->m_stUserInfo.m_udwBSeqNo));
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -3;
        }
        else
        {
            for(TINT64 ddwIdx = 0; ddwIdx < ddwTroopTotalNum; ++ddwIdx)
            {
                ptbCity->m_bTroop[0].m_addwNum[ddwIdx] = 0;
            }
        }
    }
    else
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AddClearTroop: operate type error [seq=%u]", \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -4;
    }

    pstSession->m_stUserInfo.m_stCityInfo.m_stTblData.SetFlag(TbCITY_FIELD_TROOP);

    return 0;
}

TINT32 CProcessSelfSystem::ProcessCmd_AddClearFort(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT64 ddwOperateType = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    CGameInfo *poGameInfo = CGameInfo::GetInstance();

    TUINT32 udwNum = atoi(pstSession->m_stReqParam.m_szKey[1]);

    TbCity *ptbCity = &pstSession->m_stUserInfo.m_stCityInfo.m_stTblData;

    TINT64 ddwFortTotalNum = poGameInfo->GetFortTypeNum();

    if(EN_ADD_OPERATE == ddwOperateType)
    {
        if(0 == ddwFortTotalNum)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AddClearFort: game.json fort is null [seq=%u]", \
                pstSession->m_stUserInfo.m_udwBSeqNo));
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -2;
        }
        else
        {
            for(TINT64 ddwIdx = 0; ddwIdx < ddwFortTotalNum; ++ddwIdx)
            {
                ptbCity->m_bFort[0].m_addwNum[ddwIdx] = udwNum;
            }
        }
    }
    else if(EN_CLEAR_OPERATE == ddwOperateType)
    {

        if(0 == ddwFortTotalNum)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AddClearFort: game.json fort is null [seq=%u]", \
                pstSession->m_stUserInfo.m_udwBSeqNo));
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -3;
        }
        else
        {
            for(TINT64 ddwIdx = 0; ddwIdx < ddwFortTotalNum; ++ddwIdx)
            {
                ptbCity->m_bFort[0].m_addwNum[ddwIdx] = 0;
            }
        }
    }
    else
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AddClearFort: operate type error [seq=%u]", \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -4;
    }



    ptbCity->SetFlag(TbCITY_FIELD_FORT);

    return 0;
}

TINT32 CProcessSelfSystem::ProcessCmd_AddClearResource(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT64 ddwOperateType = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 10);

    TbCity *ptbCity = &pstSession->m_stUserInfo.m_stCityInfo.m_stTblData;

    if(EN_ADD_OPERATE == ddwOperateType)
    {
        for(TINT64 ddwIdx = 0; ddwIdx < EN_RESOURCE_TYPE__END; ++ddwIdx)
        {
            ptbCity->m_bResource[0].m_addwNum[ddwIdx] = 999999;
        }
    }
    else if(EN_CLEAR_OPERATE == ddwOperateType)
    {

        for(TINT64 ddwIdx = 0; ddwIdx < EN_RESOURCE_TYPE__END; ++ddwIdx)
        {
            ptbCity->m_bResource[0].m_addwNum[ddwIdx] = 0;
        }
    }
    else
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AddClearResource: operate type error [seq=%u]", \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -4;
    }


    ptbCity->SetFlag(TbCITY_FIELD_RESOURCE);

    return 0;
}

TINT32 CProcessSelfSystem::ProcessCmd_ClearItem(SSession *pstSession, TBOOL &bNeedResponse)
{   
    TINT64 ddwOperateType = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    CGameInfo *poGameInfo = CGameInfo::GetInstance();

    TbBackpack *ptBackpack = &pstSession->m_stUserInfo.m_tbBackpack;
    if(0 == ptBackpack->m_nUid)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ClearItem: user_stat data is null [seq=%u]", \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }
    TINT64 ddwItemTotalNum = poGameInfo->GetItemTotalNum();


    if(EN_ADD_OPERATE == ddwOperateType)
    {
        if(0 == ddwItemTotalNum)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ClearItem: game.json item is null [seq=%u]", \
                pstSession->m_stUserInfo.m_udwBSeqNo));
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -2;
        }
        else
        {
            for(TINT64 ddwIdx = 0; ddwIdx < ptBackpack->m_bItem.m_udwNum; ++ddwIdx)  //lucien note:item会是一直连续排列的吗？
            {
                ptBackpack->m_bItem[ddwIdx].m_ddwItemNum = 999999;
            }
        }
    }
    else if(EN_CLEAR_OPERATE == ddwOperateType)
    {

        if(0 == ddwItemTotalNum)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ClearItem: game.json item is null [seq=%u]", \
                pstSession->m_stUserInfo.m_udwBSeqNo));
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -3;
        }
        else
        {
            for(TINT64 ddwIdx = 0; ddwIdx < ptBackpack->m_bItem.m_udwNum; ++ddwIdx)
            {
                ptBackpack->m_bItem[ddwIdx].m_ddwItemNum = 0;
            }
        }
    }
    else
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ClearItem: operate type error [seq=%u]", \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -4;
    }



    ptBackpack->SetFlag(TbBACKPACK_FIELD_ITEM);

    return 0;
}

TINT32 CProcessSelfSystem::ProcessCmd_AddHosTroop(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwTroopId = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TINT32 dwTroopNum = atoi(pstSession->m_stReqParam.m_szKey[1]);

    if(dwTroopNum <= 0
        || dwTroopId < 0
        || dwTroopId >= EN_TROOP_TYPE__END)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AddHosTroop: [dwTroopId=%d] [dwTroopNum=%d] [seq=%u]", \
            dwTroopId, \
            dwTroopNum, \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }

    TbCity *ptbCity = &pstSession->m_stUserInfo.m_stCityInfo.m_stTblData;

    ptbCity->m_bHos_wait[0].m_addwNum[dwTroopId] += dwTroopNum;
    ptbCity->SetFlag(TbCITY_FIELD_HOS_WAIT);

    return 0;
}

TINT32 CProcessSelfSystem::ProcessCmd_ReduceHosTroop(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwTroopId = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TINT32 dwTroopNum = atoi(pstSession->m_stReqParam.m_szKey[1]);

    if (dwTroopNum <= 0
        || dwTroopId < 0
        || dwTroopId >= EN_TROOP_TYPE__END)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_AddHosTroop: [dwTroopId=%d] [dwTroopNum=%d] [seq=%u]", \
            dwTroopId, \
            dwTroopNum, \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }

    TbCity *ptbCity = &pstSession->m_stUserInfo.m_stCityInfo.m_stTblData;

    if (ptbCity->m_bHos_wait[0].m_addwNum[dwTroopId] >= dwTroopNum)
    {
        ptbCity->m_bHos_wait[0].m_addwNum[dwTroopId] -= dwTroopNum;
    }
    else
    {
        ptbCity->m_bHos_wait[0].m_addwNum[dwTroopId] = 0;
    }
    ptbCity->SetFlag(TbCITY_FIELD_HOS_WAIT);

    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_BreakNewUserProtect(SSession *pstSession, TBOOL &bNeedResponse)  //lucien note：确认相关公用方法是否可用
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;

    TbAction *pstAction = CActionBase::GetActionByBufferId(&pstUser->m_atbAction[0], pstUser->m_udwActionNum, EN_BUFFER_INFO_PEACE_TIME);
    if(pstAction)
    {
        // 3. 删除相应的action
        TUINT32 udwIdx = CActionBase::GetActionIndex(&pstUser->m_atbAction[0], pstUser->m_udwActionNum, pstAction->m_nId);
        pstUser->m_aucActionFlag[udwIdx] = EN_TABLE_UPDT_FLAG__DEL;
    }

    pstUser->m_tbPlayer.Set_Status(pstUser->m_tbPlayer.m_nStatus & (~EN_CITY_STATUS__NEW_PROTECTION));
    pstUser->m_tbPlayer.Set_Status(pstUser->m_tbPlayer.m_nStatus & (~EN_CITY_STATUS__AVOID_WAR));
    pstSession->m_bBreakMapEndTime = TRUE;

    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_GetBufferInfo(SSession *pstSession, TBOOL &bNeedResponse)
{
    pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_OPERATE_BUFFER_INFO;

    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_SetVipLevel(SSession* pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwLevel = atoi(pstSession->m_stReqParam.m_szKey[0]);

    pstSession->m_stUserInfo.m_tbPlayer.Set_Vip_point(CPlayerBase::GetRawVipLevelPoint(dwLevel));
    pstSession->m_stUserInfo.m_tbPlayer.Set_Vip_stage(CPlayerBase::GetRawVipStage(dwLevel));
    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_SetVipPoint(SSession* pstSession, TBOOL &bNeedResponse)
{
    TINT64 ddwPoint = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    if (ddwPoint > CPlayerBase::GetMaxVipPoint())
    {
        ddwPoint = CPlayerBase::GetMaxVipPoint();
    }
    pstSession->m_stUserInfo.m_tbPlayer.Set_Vip_point(ddwPoint);

    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_SetVipLeftTime(SSession* pstSession, TBOOL &bNeedResponse)
{
    TUINT32 udwLeftTime = atoi(pstSession->m_stReqParam.m_szKey[0]);

    SCityInfo *pstCity = &pstSession->m_stUserInfo.m_stCityInfo;

    pstSession->m_stUserInfo.m_tbPlayer.Set_Vip_etime(CTimeUtils::GetUnixTime() + udwLeftTime);
    CCommonBase::UpdateVipInfo(&pstSession->m_stUserInfo, pstCity);

    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_SetPlayerLv(SSession* pstSession, TBOOL &bNeedResponse)
{
    TUINT32 udwLv = atoi(pstSession->m_stReqParam.m_szKey[0]);

    const Json::Value &oExpJson = CGameInfo::GetInstance()->m_oJsonRoot["game_lord_exp"];

    if(udwLv > oExpJson.size())
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("CProcessSelfSystem::Processcmd_AddMaterial: lv is out of rang[seq=%u]",
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }
    TUINT32 udwExp = oExpJson[udwLv][0U].asUInt() + 1;

    pstSession->m_stUserInfo.m_tbPlayer.Set_Exp(udwExp);
    pstSession->m_stUserInfo.m_tbPlayer.Set_Level(udwLv);

    return 0;

}

TINT32 CProcessSelfSystem::Processcmd_OpenChest(SSession* pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;

    TUINT32 udwItemId = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TUINT32 udwItemNum = atoi(pstSession->m_stReqParam.m_szKey[1]);

    if (FALSE == CItemBase::HasEnoughItem(&pstUser->m_tbBackpack, udwItemId, udwItemNum))
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__ITEM_NOT_ENOUGH;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_OpenChest: item not enough [seq=%u]", \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }
    if (0 == udwItemNum)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_OpenChest: use item num[%u] [seq=%u]", \
            udwItemNum, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -2;
    }

    if ((EN_ITEM_FUNC_TYPE_CRYSTAL != CItemBase::GetItemFuncFromGameJson(udwItemId)
        && EN_ITEM_FUNC_TYPE_MATERIALS != CItemBase::GetItemFuncFromGameJson(udwItemId)
        && EN_ITEM_FUNC_TYPE_CHALLENGER != CItemBase::GetItemFuncFromGameJson(udwItemId)
        && EN_ITEM_FUNC_TYPE_MONSTER != CItemBase::GetItemFuncFromGameJson(udwItemId))
        || CItemLogic::IsChestLottery(udwItemId))
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_OpenChest: use item id[%u] not chest [seq=%u]", \
            udwItemId, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -3;
    }

    if (EN_COMMAND_STEP__INIT == pstSession->m_udwCommandStep)
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

        TSE_LOG_DEBUG(pstSession->m_poServLog, ("Processcmd_OpenChest: data center req: [json=%s] [type=%u] [uid=%ld][seq=%u]", \
            pstReq->m_sReqContent.c_str(), \
            pstReq->m_udwType, \
            pstUser->m_tbPlayer.m_nUid, \
            pstSession->m_udwSeqNo));
        bNeedResponse = TRUE;
        TINT32 dwRetCode = CBaseProcedure::SendDataCenterRequest(pstSession, EN_SERVICE_TYPE_DATA_CENTER_REQ);
        if (dwRetCode != 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_DATA_CENTER_REQ_ERR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_OpenChest: send request to data center failed. [json=%s] [ret=%d] [seq=%u]", \
                pstReq->m_sReqContent.c_str(), \
                dwRetCode, \
                pstUser->m_udwBSeqNo));
            return -4;
        }

        return 0;
    }

    if (EN_COMMAND_STEP__1 == pstSession->m_udwCommandStep)
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
                if (EN_REFRESH_DATA_TYPE__CHEST == pstDataCenterRsp->m_udwType)
                {
                    TSE_LOG_DEBUG(pstSession->m_poServLog, ("Processcmd_OpenChest: data center get rsp: [json=%s] [uid=%ld][seq=%u]",
                        pstDataCenterRsp->m_sRspJson.c_str(), \
                        pstUser->m_tbPlayer.m_nUid, \
                        pstUser->m_udwBSeqNo));

                    if (FALSE == jsonReader.parse(pstDataCenterRsp->m_sRspJson, oRspDataJson))
                    {
                        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_DATA_CENTER_PACKAGE_ERR;
                        TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_OpenChest: prase rsp from data center failed. [seq=%u]", \
                            pstUser->m_udwBSeqNo));
                        return -5;
                    }
                    TINT32 dwRetCode = stRefreshData.m_stChestRsp.setVal(oRspDataJson);
                    if (0 != dwRetCode)
                    {
                        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__DATA_CENTER_PACKAGE_FORMAT_ERR;
                        TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_OpenChest: response data format error. [ret=%d][seq=%u]", \
                            dwRetCode, \
                            pstUser->m_udwBSeqNo));
                        return -6;
                    }
                    break;
                }
            }

            if (MAX_SP_REWARD_ITEM_NUM < stRefreshData.m_stChestRsp.m_vecReward.size()
                || 0 >= stRefreshData.m_stChestRsp.m_vecReward.size())
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_OpenChest: chest reward size[%ld] is over. [seq=%u]", \
                    stRefreshData.m_stChestRsp.m_vecReward.size(), pstUser->m_udwBSeqNo));
                return -8;
            }

            SSpGlobalRes stGlobalRes;
            stGlobalRes.Reset();

            for (TINT32 dwIdx = 0; dwIdx < stRefreshData.m_stChestRsp.m_vecReward.size(); ++dwIdx)
            {
                SOneGlobalRes *pstOneGlobalRes = stRefreshData.m_stChestRsp.m_vecReward[dwIdx];
                stGlobalRes.aRewardList[dwIdx].udwType = pstOneGlobalRes->ddwType;
                stGlobalRes.aRewardList[dwIdx].udwId = pstOneGlobalRes->ddwId;
                stGlobalRes.aRewardList[dwIdx].udwNum = pstOneGlobalRes->ddwNum;
                ++stGlobalRes.udwTotalNum;
                CGlobalResLogic::AddGlobalRes(pstUser, &pstUser->m_stCityInfo, pstOneGlobalRes->ddwType, pstOneGlobalRes->ddwId, pstOneGlobalRes->ddwNum);
            }

            // cost item 
            CItemBase::CostItem(&pstUser->m_tbBackpack, udwItemId, udwItemNum);

            pstUser->m_udwLotteryChestItemId = udwItemId;
            pstUser->m_udwLotteryChestItemNum = udwItemNum;
            pstUser->m_stRewardWindow = stGlobalRes;

            Json::Value jTmp = Json::Value(Json::objectValue);
            jTmp["chest"] = Json::Value(Json::objectValue);
            jTmp["chest"]["id"] = udwItemId;
            jTmp["chest"]["num"] = udwItemNum;

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

TINT32 CProcessSelfSystem::Processcmd_SetLastLoginTime(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwType = atoi(pstSession->m_stReqParam.m_szKey[0]);
    if(dwType == 0)
    {
        pstSession->m_stUserInfo.m_tbPlayer.Set_Utime(CTimeUtils::GetUnixTime() - 86400 - 100);
    }
    else
    {
        pstSession->m_stUserInfo.m_tbPlayer.Set_Utime(CTimeUtils::GetUnixTime() - 86400 * 2 - 100);
    }

    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_SetCurLoytal(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT64 ddwLoyitv = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    
    TbPlayer& tbPlayer = pstSession->m_stUserInfo.m_tbPlayer;
    TINT64 ddwMaxAlFundGetPerInterval = CGameInfo::GetInstance()->m_oJsonRoot["game_basic"][3U].asInt64();
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    
    if(ddwLoyitv < 0
       || ddwLoyitv > ddwMaxAlFundGetPerInterval)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_SetCurLoytal: ddwFundAdd error [ddwLoyitv=%ld] [ddwMaxAlFundGetPerInterval=%ld] [seq=%u]",
                                                ddwLoyitv, \
                                                ddwMaxAlFundGetPerInterval, \
                                                pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }

    TINT64 ddwLoyitvAdd = 0;
    ddwLoyitvAdd = ddwLoyitv - tbPlayer.m_nLoy_itv;


    pstSession->m_udwLoytalAdd = ddwLoyitvAdd;
    tbPlayer.Set_Loy_itv(ddwLoyitv);
    tbPlayer.Set_Loy_cur(tbPlayer.m_nLoy_cur + ddwLoyitvAdd);
    tbPlayer.Set_Loy_all(tbPlayer.m_nLoy_all + ddwLoyitvAdd);
    tbPlayer.Set_Loy_time(udwCurTime);

    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_SetDragonShard(SSession *pstSession, TBOOL &bNeedResponse)
{
    TbPlayer *ptbPlayer = &pstSession->m_stUserInfo.m_tbPlayer;

    TINT64 ddwShard = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 10);

    if (ddwShard < 0)
    {
        ptbPlayer->Set_Dragon_shard(0);
    }
    else
    {
        ptbPlayer->Set_Dragon_shard(ddwShard);
    }

    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_SetEquipGrideNum(SSession *pstSession, TBOOL &bNeedResponse)
{
    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_SetPosBuildingLv(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwPosition = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TINT32 dwLevel = atoi(pstSession->m_stReqParam.m_szKey[1]);

    TSE_LOG_INFO(pstSession->m_poServLog, ("Processcmd_SetPosBuildingLv: start [dwPosition=%d] [dwLevel=%d] [seq=%u]", \
        dwPosition, \
        dwLevel, \
        pstSession->m_stUserInfo.m_udwBSeqNo));

    SUserInfo *pstUserInfo = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUserInfo->m_stCityInfo;
    TbCity *ptbCity = &pstCity->m_stTblData;

    SCityBuildingNode* pstBuildingNode = CCityBase::GetBuildingAtPos(ptbCity, dwPosition);
    if(!pstBuildingNode)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -2;
    }

    if(dwLevel == 0)
    {
        CCityBase::DelBuildingAtPos(ptbCity, dwPosition);
        return 0;
    }

    // 5. 执行upgrade
    UActionParam stParam;
    stParam.m_stBuilding.SetValue(dwPosition, pstBuildingNode->m_ddwType, dwLevel, 0, (TCHAR *)pstSession->m_stUserInfo.m_tbPlayer.m_sUin.c_str());
    CProcessAction::BuildingActionDone(pstSession, &pstSession->m_stUserInfo, pstCity, EN_ACTION_SEC_CLASS__BUILDING_UPGRADE, &stParam.m_stBuilding);

    ptbCity->SetFlag(TbCITY_FIELD_BUILDING);

    TSE_LOG_INFO(pstSession->m_poServLog, ("Processcmd_SetPosBuildingLv: end [seq=%u]", \
        pstSession->m_stUserInfo.m_udwBSeqNo));
    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_SetAllResearchLv(SSession *pstSession, TBOOL &bNeedResponse)
{   
    TINT32 dwLevel = atoi(pstSession->m_stReqParam.m_szKey[0]);

    SUserInfo *pstUserInfo = &pstSession->m_stUserInfo;
    TbCity *ptbCity = &pstUserInfo->m_stCityInfo.m_stTblData;

    if(dwLevel < 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -3;
    }

    const Json::Value& oJsonResearh = CGameInfo::GetInstance()->m_oJsonRoot["game_research"];
    Json::Value::Members members = oJsonResearh.getMemberNames();
    for(Json::Value::Members::iterator it = members.begin(); it != members.end(); ++it)
    {
        TINT32 dwMaxlv = oJsonResearh[*it]["b"]["b0"].size();
        TINT32 dwType = atoi(it->c_str());
        if(dwLevel > dwMaxlv)
        {
            ptbCity->m_bResearch[0].m_addwLevel[dwType] = dwMaxlv;
        }
        else
        {
            ptbCity->m_bResearch[0].m_addwLevel[dwType] = dwLevel;
        }
    }

    ptbCity->SetFlag(TbCITY_FIELD_RESEARCH);

    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_SetAllBuildingLv(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwLevel = atoi(pstSession->m_stReqParam.m_szKey[0]);

    TSE_LOG_INFO(pstSession->m_poServLog, ("Processcmd_SetAllBuildingLv: start [dwPosition=%d] [dwType=%d] [dwLevel=%d] [seq=%u]", \
        dwLevel, \
        pstSession->m_stUserInfo.m_udwBSeqNo));


    SUserInfo *pstUserInfo = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUserInfo->m_stCityInfo;
    TbCity *ptbCity = &pstCity->m_stTblData;

    if(dwLevel < 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_SetAllBuildingLv: level is illegal [dwLevel=%d] [seq=%u]", \
            dwLevel, \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }

    for(TUINT32 udwIdx = 0; udwIdx < pstCity->m_stTblData.m_bBuilding.m_udwNum; ++udwIdx)
    {
        SCityBuildingNode* pstNode = &pstCity->m_stTblData.m_bBuilding[udwIdx];
        if(0 != pstNode->m_ddwLevel)
        {
            if(dwLevel == 0)
            {
                CCityBase::DelBuildingAtPos(ptbCity, pstNode->m_ddwPos);
                continue;
            }

            TUINT8 ucCurType = pstNode->m_ddwType;
            // 5. 执行upgrade
            UActionParam stParam;
            stParam.m_stBuilding.SetValue(pstNode->m_ddwPos, ucCurType, dwLevel, 0, (TCHAR *)pstSession->m_stUserInfo.m_tbPlayer.m_sUin.c_str());
            CProcessAction::BuildingActionDone(pstSession, &pstSession->m_stUserInfo, pstCity, EN_ACTION_SEC_CLASS__BUILDING_UPGRADE, &stParam.m_stBuilding);

            ptbCity->SetFlag(TbCITY_FIELD_BUILDING);
        }
    }

    TSE_LOG_INFO(pstSession->m_poServLog, ("Processcmd_SetAllBuildingLv: end [seq=%u]", \
        pstSession->m_stUserInfo.m_udwBSeqNo));
    return 0;
}

TINT32 CProcessSelfSystem::ProcessCmd_AddDeadFort(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwFortId = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TINT32 dwFortNum = atoi(pstSession->m_stReqParam.m_szKey[1]);

    if(dwFortNum <= 0
        || dwFortId < 0
        || dwFortId >= EN_FORT_TYPE__END)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }

    TbCity *ptbCity = &pstSession->m_stUserInfo.m_stCityInfo.m_stTblData;

    ptbCity->m_bDead_fort[0].m_addwNum[dwFortId] += dwFortNum;
    ptbCity->SetFlag(TbCITY_FIELD_DEAD_FORT);

    return 0;
}

TINT32 CProcessSelfSystem::ProcessCmd_ReduceDeadFort(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwFortId = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TINT32 dwFortNum = atoi(pstSession->m_stReqParam.m_szKey[1]);

    if (dwFortNum <= 0
        || dwFortId < 0
        || dwFortId >= EN_FORT_TYPE__END)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }

    TbCity *ptbCity = &pstSession->m_stUserInfo.m_stCityInfo.m_stTblData;

    if (ptbCity->m_bDead_fort[0].m_addwNum[dwFortId] >= dwFortNum)
    {
        ptbCity->m_bDead_fort[0].m_addwNum[dwFortId] -= dwFortNum;
    }
    else
    {
        ptbCity->m_bDead_fort[0].m_addwNum[dwFortId] = 0;
    }
    ptbCity->SetFlag(TbCITY_FIELD_DEAD_FORT);

    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_ResetFinishTask(SSession *pstSession, TBOOL &bNeedResponse)
{
    pstSession->m_stUserInfo.m_tbTask.m_bTask_finish[0].Reset();
    pstSession->m_stUserInfo.m_tbTask.SetFlag(TbTASK_FIELD_TASK_FINISH);
    
    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_SetFinishTask(SSession *pstSession, TBOOL &bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbTask *ptbTask = &pstUser->m_tbTask;
    TUINT32 udwTaskId = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);

    //普通任务
    BITSET(ptbTask->m_bTask_finish[0].m_bitTask, udwTaskId);

    pstUser->m_audwTmpFinishTaskId[pstUser->m_udwTmpFinishNum] = udwTaskId;
    pstUser->m_udwTmpFinishNum++;

    ptbTask->SetFlag(TbTASK_FIELD_TASK_FINISH);

    return 0;
}


TINT32 CProcessSelfSystem::Processcmd_RefreshTaskCur(SSession *pstSession, TBOOL &bNeedResponse)
{
    TbTask *ptbTask = &pstSession->m_stUserInfo.m_tbTask;

    ptbTask->m_nTask_refresh_time = CTimeUtils::GetCurTimeUs() - 24 * 60 * 60;
    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_AddWild(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    TUINT32 udwPos = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TUINT32 udwWildType = strtoul(pstSession->m_stReqParam.m_szKey[1], NULL, 10);
    TUINT32 udwWildLv = strtoul(pstSession->m_stReqParam.m_szKey[2], NULL, 10);
    const Json::Value& oWildResJson = CWildInfo::GetWildResInfo(pstSession->m_stReqParam.m_udwSvrId);
    TUINT32 udwXYSize = CMapBase::GetWildBlockNumByType(pstSession->m_stReqParam.m_udwSvrId, udwWildType);

    //拉取地图
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        //获取该地形所占地块数
        if(!oWildResJson.isMember(CCommonFunc::NumToString(udwWildType)) && udwWildType != EN_WILD_TYPE__THRONE_NEW && udwWildType != EN_WILD_TYPE__IDOL)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("Processcmd_AddWild:not such wild type [type=%u] [seq=%u]", udwWildType, pstSession->m_udwSeqNo));
            return -2;
        }

        if (oWildResJson.isMember(CCommonFunc::NumToString(udwWildType)) 
            && (udwWildLv > oWildResJson[CCommonFunc::NumToString(udwWildType)]["a1"].size() || udwWildLv == 0))
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("Processcmd_AddWild:lv err [lv=%u] [seq=%u]", udwWildLv, pstSession->m_udwSeqNo));
            return -1;
        }

        // set request package
        pstSession->ResetAwsInfo();

        TUINT32 udwXPos = udwPos / MAP_X_Y_POS_COMPUTE_OFFSET;
        TUINT32 udwYPos = udwPos % MAP_X_Y_POS_COMPUTE_OFFSET;

        std::set<TINT32> posSet;
        posSet.clear();

        TUINT32 udwCenterPos = CCommonLogic::BuildingPointToPos(udwXPos, udwYPos);
        CCommonLogic::GetBuildingPos(udwCenterPos, udwXYSize, posSet);

        for(set<TINT32>::iterator it = posSet.begin(); it != posSet.end(); ++it)
        {
            BuildingPoint stBuildingPoing = CCommonLogic::BuildingPosToPoint(*it);
            if ((stBuildingPoing.x + stBuildingPoing.y) % 2 != 0)
            {
                continue;
            }
            TUINT32 udwNewPos = stBuildingPoing.x * MAP_X_Y_POS_COMPUTE_OFFSET + stBuildingPoing.y;
            CAwsRequest::MapGet(pstSession, udwNewPos);
        }

        // send request
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("TableRequest_MapGetByCids: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
        return 0;
    }

    //响应 检查地图信息
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        if(pstSession->m_stCommonResInfo.m_dwRetCode == EN_RET_CODE__SUCCESS)
        {
            AwsRspInfo *pstRes = NULL;
            for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecAwsRsp.size(); ++udwIdx)
            {
                pstRes = pstSession->m_vecAwsRsp[udwIdx];

                string strTableRawName = CCommonFunc::GetTableRawName(pstRes->sTableName);
                if(strTableRawName == EN_AWS_TABLE_MAP)
                {
                    dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[udwIdx],
                        &pstSession->m_atbTmpWild[pstSession->m_udwTmpWildNum]);

                    if(dwRetCode > 0)
                    {
                        pstSession->m_udwTmpWildNum++;
                    }
                }
            }   
            for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_udwTmpWildNum; ++udwIdx)
            {
                if(pstSession->m_atbTmpWild[udwIdx].m_nId == udwPos)
                {
                    pstSession->m_tbTmpMap = pstSession->m_atbTmpWild[udwIdx];
                }
            }
            if (pstSession->m_tbTmpMap.m_nUid != 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__MOVE_CITY_COORD_INVALID;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("GetMapInfo: wild belong someone else. [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                return -4;
            }
        }
    }

    // 4. 处理
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {

        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        // 设置新map数据
        TbMap *pstMap = &pstSession->m_tbTmpMap;
        pstMap->Set_Level(udwWildLv);
        pstMap->Set_Type(udwWildType);
        pstMap->Set_Utime(CTimeUtils::GetUnixTime());
        pstMap->Set_Bid(CMapBase::GetBlockIdFromPos(udwPos));
        
        pstMap->Set_Pic_index(1);

        if (oWildResJson.isMember(CCommonFunc::NumToString(udwWildType)))
        {
            pstMap->Set_Showtime(CTimeUtils::GetUnixTime());
            pstMap->Set_Reward_left(10000); //100%
            pstMap->Set_Boss_life(oWildResJson[CCommonFunc::NumToString(udwWildType)]["a11"][udwWildLv - 1][1U].asUInt());
            pstMap->Set_Wild_gen_time(CTimeUtils::GetUnixTime());
            pstMap->Set_Expire_time(pstMap->m_nWild_gen_time + MAX_MONSTER_SHOW_INTERVAL_TIME);
            TINT32 dwFource = 0;
            TINT32 dwSingleFource = 0;
            CGameInfo * pstGameInfo = CGameInfo::GetInstance();
            //troop
            for (TUINT32 udwTroopIdx = 0; udwTroopIdx < oWildResJson[CCommonFunc::NumToString(udwWildType)]["a2"][udwWildLv - 1]["a0"].size(); ++udwTroopIdx)
            {
                pstMap->m_bTroop[0].m_addwNum[udwTroopIdx] = oWildResJson[CCommonFunc::NumToString(udwWildType)]["a2"][udwWildLv - 1]["a0"][udwTroopIdx].asInt();
                if (pstMap->m_bTroop[0].m_addwNum[udwTroopIdx] > 0)
                {
                    dwSingleFource = pstGameInfo->m_oJsonRoot["troop"][udwTroopIdx]["a"]["a9"].asInt();
                    dwFource += dwSingleFource * pstMap->m_bTroop[0].m_addwNum[udwTroopIdx];
                }
            }
            pstMap->Set_Might(dwFource);
            pstMap->Set_Force_kill(0);
            pstMap->SetFlag(TbMAP_FIELD_TROOP);
        }
        
        pstMap->m_bAl_attack_info.Reset();
        pstMap->m_bAttack_info.Reset();
        pstMap->SetFlag(TbMAP_FIELD_AL_ATTACK_INFO);
        pstMap->SetFlag(TbMAP_FIELD_ATTACK_INFO);

        pstMap->Set_Leader_monster_flag(0);

        if (udwWildType == EN_WILD_TYPE__THRONE_NEW)
        {
            pstMap->Set_Status(EN_THRONE_STATUS__PEACE_TIME);
        }
        else if (udwWildType == EN_WILD_TYPE__IDOL)
        {
            pstMap->Set_Status(EN_IDOL_STATUS__THRONE_PEACE_TIME);
        }

        ExpectedDesc desc;
        ExpectedItem item;
        item.SetVal(TbMAP_FIELD_UID, true, 0);//未被人占领
        desc.vecExpectedItem.push_back(item);

        // set package
        pstSession->ResetAwsInfo();
        CAwsRequest::UpdateItem(pstSession, &pstSession->m_tbTmpMap, desc, RETURN_VALUES_ALL_NEW);


        //主坐标外的其余坐标
        dwRetCode = CProcessSelfSystem::SetSidePos(pstSession,
            &pstSession->m_tbTmpMap,
            pstSession->m_atbTmpWild,
            pstSession->m_udwTmpWildNum,
            udwXYSize);

        // send request
        
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("TableRequest_CityChangeId: update side pos failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -6;
        }

        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("TableRequest_CityChangeId: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -5;
        }
        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_AddCrystal(SSession *pstSession, TBOOL &bNeedResponse)
{
    TUINT32 udwId = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TUINT32 udwNum = strtoul(pstSession->m_stReqParam.m_szKey[1], NULL, 10);
    CBackpack::AddCrystal(&pstSession->m_stUserInfo, udwId, udwNum);

    return 0;
}


TINT32 CProcessSelfSystem::Processcmd_AddSpCrystal(SSession *pstSession, TBOOL &bNeedResponse)
{
    TUINT32 udwId = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TUINT32 udwNum = strtoul(pstSession->m_stReqParam.m_szKey[1], NULL, 10);

    CBackpack::AddSpCrystal(&pstSession->m_stUserInfo, udwId, udwNum);
    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_AddMaterial(SSession *pstSession, TBOOL &bNeedResponse)
{
    TUINT32 udwId = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TINT64 ddwNum = strtoll(pstSession->m_stReqParam.m_szKey[1], NULL, 10);

    CBackpack::OpAddMaterial(&pstSession->m_stUserInfo, udwId, ddwNum);
    return 0;

}

TINT32 CProcessSelfSystem::Processcmd_AddParts(SSession *pstSession, TBOOL &bNeedResponse)
{
    TUINT32 udwId = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TUINT32 udwNum = strtoul(pstSession->m_stReqParam.m_szKey[1], NULL, 10);

    CBackpack::AddParts(&pstSession->m_stUserInfo, udwId, udwNum);
    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_AddSoul(SSession *pstSession, TBOOL &bNeedResponse)
{
    TUINT32 udwId = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TUINT32 udwNum = strtoul(pstSession->m_stReqParam.m_szKey[1], NULL, 10);

    CBackpack::AddSoul(&pstSession->m_stUserInfo, udwId, udwNum);
    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_AddNormalEquip(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwType = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TINT32 dwLevel = strtoul(pstSession->m_stReqParam.m_szKey[1], NULL, 10);
    SEquipMentInfo stEquip;
    stEquip.Reset();
    const Json::Value &oEquipJson = CBackpackInfo::GetInstance()->m_oJsonRoot["equip_equipment"];

    if(pstSession->m_stUserInfo.m_tbUserStat.m_nEquip_gride <= pstSession->m_stUserInfo.m_udwEquipNum)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__EQUIP_NUM_MAX;
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("CProcessSelfSystem::Processcmd_AddNormalEquip:not enough gride [grid_num=%ld] [equip_num=%ld] [seq=%u] ",
            pstSession->m_stUserInfo.m_tbUserStat.m_nEquip_gride, \
            pstSession->m_stUserInfo.m_udwEquipNum, \
            pstSession->m_udwSeqNo));
        return -1;
    }
    
    if (dwType < 0
        || dwLevel < 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("CProcessSelfSystem::Processcmd_AddNormalEquip:not enough gride [type=%ld] [level=%ld] [seq=%u] ",
            dwType, dwLevel, pstSession->m_udwSeqNo));
        return -2;
    }
    string szEquipType = CCommonFunc::NumToString(dwType);

    if (!oEquipJson.isMember(szEquipType))
    {
        //未找到该type 和 lv对应的装备id
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("CProcessSelfSystem::Processcmd_AddEquip:can not find [type=%u] equip [seq=%u] ",
            dwType, pstSession->m_udwSeqNo));
        return -3;
    }


    for (TUINT32 udwIdx = 0; udwIdx < oEquipJson[szEquipType]["c"].size(); udwIdx++)
    {
        if (oEquipJson[szEquipType]["c"][udwIdx][0U].asUInt() == EN_GLOBALRES_TYPE_DRAGON_LV)
        {
            if (pstSession->m_stUserInfo.m_tbPlayer.m_nDragon_level < oEquipJson[szEquipType]["c"][udwIdx][1U].asUInt())
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__COMPOSE_DRAGON_LV_NOT_ENOUGH;
                TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("CProcessSelfSystem::Processcmd_AddEquip:can not find [dragon_lv=%ld] [idx=%u] [seq=%u] ",
                    pstSession->m_stUserInfo.m_tbPlayer.m_nDragon_level, \
                    udwIdx, \
                    pstSession->m_udwSeqNo));
                return -4;
            }
        }
    }

    //id
    stEquip.uddwId = CBackpack::GenEquipId(&pstSession->m_stUserInfo.m_tbLogin);
    stEquip.stBaseInfo.udwLv = dwLevel;

    //base info
    CBackpack::GetEquipBaseInfoByEid(dwType, &stEquip);

    //buff
    Json::Value::Members oBuffMember = oEquipJson[szEquipType]["buff"][dwLevel - 1].getMemberNames();
    for (Json::Value::Members::iterator it = oBuffMember.begin(); it != oBuffMember.end(); ++it)
    {
        stEquip.stStatusInfo.astBuffInfo[stEquip.stStatusInfo.udwBufferNum].m_udwId = oEquipJson[szEquipType]["buff"][dwLevel - 1][(*it).c_str()][0U].asUInt();
        stEquip.stStatusInfo.astBuffInfo[stEquip.stStatusInfo.udwBufferNum].m_dwNum = oEquipJson[szEquipType]["buff"][dwLevel - 1][(*it).c_str()][1U].asInt();
        stEquip.stStatusInfo.astBuffInfo[stEquip.stStatusInfo.udwBufferNum].m_udwType = oEquipJson[szEquipType]["buff"][dwLevel - 1][(*it).c_str()][3U].asUInt();
        stEquip.stStatusInfo.udwBufferNum++;
    }
    stEquip.stStatusInfo.udwStatus = EN_EQUIPMENT_STATUS_NORMAL;

    CBackpack::AddEquip(&pstSession->m_stUserInfo, &stEquip);

    return 0;
}


TINT32 CProcessSelfSystem::Processcmd_AddSpEquip(SSession *pstSession, TBOOL &bNeedResponse)
{
    TUINT32 udwSoulId = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    SEquipMentInfo stEquip;
    stEquip.Reset();

    TbLogin *pstLogin = &pstSession->m_stUserInfo.m_tbLogin;
    TINT32 dwRetCode = 0;
    const Json::Value &oComposeJson = CBackpackInfo::GetInstance()->m_oJsonRoot["equip_compose"];

    if(pstSession->m_stUserInfo.m_tbUserStat.m_nEquip_gride <= pstSession->m_stUserInfo.m_udwEquipNum)
    {
        pstSession->m_stUserInfo.m_tbUserStat.Set_Equip_gride(pstSession->m_stUserInfo.m_udwEquipNum + 1);
    }

    SSoulInfo stSoulInfo;
    stSoulInfo.Reset();
    dwRetCode = CBackpack::GetSoulInfoById(udwSoulId, &stSoulInfo);
    if(dwRetCode != 0)
    {
        //不存在该灵魂
        return -1;
    }
    //type 
    TUINT32 udwEquipType = oComposeJson["b"][CCommonFunc::NumToString(stSoulInfo.udwType)].asUInt();

    //lv 
    TUINT32 udwEquipLv = stSoulInfo.udwLv;

    TUINT32 udwEid = CBackpack::GetEquipIdByTypeAndLv(udwEquipType, udwEquipLv);
    if(udwEid == 0)
    {
        //不存在这样的装备
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("CProcessSelfSystem::Processcmd_AddSpEquip:can not find [id=%u] equip [seq=%u] ",
            udwEid, pstSession->m_udwSeqNo));
        return -2;
    }
    //id
    stEquip.uddwId = CBackpack::GenEquipId(pstLogin);

    //base info
    CBackpack::GetEquipBaseInfoByEid(udwEid, &stEquip);

    //soul buff
    const Json::Value &oSoulJson = CBackpackInfo::GetInstance()->m_oJsonRoot["equip_soul"];
    Json::Value::Members oBuffMember = oSoulJson[CCommonFunc::NumToString(udwSoulId)]["buff"].getMemberNames();
    //TODO @terry @daemon
    TUINT32 udwResearchBuffer = pstSession->m_stUserInfo.m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_CORE_EQUIPMENT_LUCK].m_ddwBuffTotal;
    for(Json::Value::Members::iterator it = oBuffMember.begin(); it != oBuffMember.end(); ++it)
    {
        TUINT32 udwMin = oSoulJson[CCommonFunc::NumToString(udwSoulId)]["buff"][(*it).c_str()][1U].asUInt();
        TUINT32 udwMax = oSoulJson[CCommonFunc::NumToString(udwSoulId)]["buff"][(*it).c_str()][2U].asUInt();
        TUINT32 udwId = oSoulJson[CCommonFunc::NumToString(udwSoulId)]["buff"][(*it).c_str()][0U].asUInt();
        TUINT32 udwType = oSoulJson[CCommonFunc::NumToString(udwSoulId)]["buff"][(*it).c_str()][4U].asUInt();

        stEquip.stStatusInfo.astBuffInfo[stEquip.stStatusInfo.udwBufferNum].m_udwId = udwId;
        stEquip.stStatusInfo.astBuffInfo[stEquip.stStatusInfo.udwBufferNum].m_dwNum = CBackpack::GetRandomBuffNum(udwResearchBuffer, udwMin, udwMax);
        stEquip.stStatusInfo.astBuffInfo[stEquip.stStatusInfo.udwBufferNum].m_udwType = udwType;
        stEquip.stStatusInfo.udwBufferNum++;
    }
    stEquip.stStatusInfo.udwStatus = EN_EQUIPMENT_STATUS_NORMAL;

    //add
    CBackpack::AddEquip(&pstSession->m_stUserInfo, &stEquip);

    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_CleanAllCrystal(SSession *pstSession, TBOOL &bNeedResponse)
{
    Json::Value rJson = Json::Value(Json::objectValue);
    pstSession->m_stUserInfo.m_tbBackpack.m_jCrystal = rJson;
    pstSession->m_stUserInfo.m_tbBackpack.SetFlag(TbBACKPACK_FIELD_CRYSTAL);
    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_CleanAllMaterial(SSession *pstSession, TBOOL &bNeedResponse)
{
    Json::Value rJson = Json::Value(Json::objectValue);
    pstSession->m_stUserInfo.m_tbBackpack.m_jMaterial = rJson;
    pstSession->m_stUserInfo.m_tbBackpack.SetFlag(TbBACKPACK_FIELD_MATERIAL);
    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_CleanAllSoul(SSession *pstSession, TBOOL &bNeedResponse)
{
    Json::Value rJson = Json::Value(Json::objectValue);
    pstSession->m_stUserInfo.m_tbBackpack.m_jSoul = rJson;
    pstSession->m_stUserInfo.m_tbBackpack.SetFlag(TbBACKPACK_FIELD_SOUL);
    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_CleanAllParts(SSession *pstSession, TBOOL &bNeedResponse)
{
    Json::Value rJson = Json::Value(Json::objectValue);
    pstSession->m_stUserInfo.m_tbBackpack.m_jParts = rJson;
    pstSession->m_stUserInfo.m_tbBackpack.SetFlag(TbBACKPACK_FIELD_PARTS);
    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_CleanAllEquip(SSession *pstSession, TBOOL &bNeedResponse)
{
    //backpack
    for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_stUserInfo.m_udwEquipNum;++udwIdx)
    {
        pstSession->m_stUserInfo.m_aucEquipFlag[udwIdx] =EN_TABLE_UPDT_FLAG__DEL;
    }

    //action
    for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_stUserInfo.m_udwSelfAlActionNum;++udwIdx)
    {
        if(pstSession->m_stUserInfo.m_atbSelfAlAction[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__EQUIP_UPGRADE)
        {
            pstSession->m_stUserInfo.m_aucSelfAlActionFlag[udwIdx] = EN_TABLE_UPDT_FLAG__DEL;
        }
    }
    return 0;
}

TINT32 CProcessSelfSystem::SetSidePos(SSession *pstSession, TbMap *pstMap, TbMap *patbMap, TUINT32 udwNum, TUINT32 udwTypeBlockNum)
{
    TbMap stMapItem;
    stMapItem.Reset();
    TbMap *pstMapItem = &stMapItem;

    //udwTypeBlockNum 是改地形的地块数 定为 NXN
    TUINT32 udwXpos = pstMap->m_nId / MAP_X_Y_POS_COMPUTE_OFFSET;
    TUINT32 udwYpos = pstMap->m_nId % MAP_X_Y_POS_COMPUTE_OFFSET;
    TUINT32 udwSize = udwTypeBlockNum;
    assert(udwSize != 0);
    const Json::Value &oWildJson = CWildInfo::GetWildResInfo(pstMap->m_nSid);

    std::set<TINT32> posSet;
    posSet.clear();

    TUINT32 udwCenterPos = CCommonLogic::BuildingPointToPos(udwXpos, udwYpos);
    CCommonLogic::GetBuildingPos(udwCenterPos, udwSize, posSet);

    for(set<TINT32>::iterator it = posSet.begin(); it != posSet.end(); ++it)
    {
        BuildingPoint stBuildingPoing = CCommonLogic::BuildingPosToPoint(*it);

        TUINT32 udwNewPos = stBuildingPoing.x * MAP_X_Y_POS_COMPUTE_OFFSET + stBuildingPoing.y;

        if (udwNewPos == pstMap->m_nId || (stBuildingPoing.x + stBuildingPoing.y) % 2 != 0)
        {
            continue;
        }
        //set data
        pstMapItem->Set_Sid(pstMap->m_nSid);
        pstMapItem->Set_Id(udwNewPos);
        pstMapItem->Set_Sid(pstMap->m_nSid);
        pstMapItem->Set_Level(pstMap->m_nLevel);
        pstMapItem->Set_Type(pstMap->m_nType);
        pstMapItem->Set_Utime(CTimeUtils::GetUnixTime());
        pstMapItem->Set_Bid(CMapBase::GetBlockIdFromPos(udwNewPos));
        pstMapItem->Set_Pic_index(2);

        if (oWildJson.isMember(CCommonFunc::NumToString(pstMap->m_nType)))
        {
            pstMapItem->Set_Showtime(CTimeUtils::GetUnixTime());
            pstMapItem->Set_Reward_left(10000); //100%
            TUINT32  udwBollLift = 0;
            const Json::Value &wildTypeJson = oWildJson[CCommonFunc::NumToString(pstMap->m_nType)];
            if (wildTypeJson.isMember("a11"))
            {
                TUINT32 udwLv = pstMap->m_nLevel;
                udwBollLift = wildTypeJson["a11"][udwLv - 1][1U].asUInt();
            }
            pstMap->Set_Boss_life(udwBollLift);
        }

        ExpectedDesc desc;
        ExpectedItem item;
        item.SetVal(TbMAP_FIELD_UID, true, 0);//未被人占领
        desc.vecExpectedItem.push_back(item);

        // set package
        CAwsRequest::UpdateItem(pstSession, pstMapItem, desc, RETURN_VALUES_ALL_NEW);
    }
    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_ClearGuideFlag(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwGuideFlag = atoi(pstSession->m_stReqParam.m_szKey[0]);

    TbLogin *pstLogin = &pstSession->m_stUserInfo.m_tbLogin;

    BITCLEAR(pstLogin->m_bGuide_flag[0].m_bitFlag, dwGuideFlag);
    pstLogin->SetFlag(TbLOGIN_FIELD_GUIDE_FLAG);

    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_SetGuideFlag(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwGuideFlag = atoi(pstSession->m_stReqParam.m_szKey[0]);

    TbLogin *pstLogin = &pstSession->m_stUserInfo.m_tbLogin;

    BITSET(pstLogin->m_bGuide_flag[0].m_bitFlag, dwGuideFlag);
    pstLogin->SetFlag(TbLOGIN_FIELD_GUIDE_FLAG);

    return 0;
}


TINT32 CProcessSelfSystem::Processcmd_CleanSecondBuildingAction(SSession *pstSession, TBOOL &bNeedResponse)
{
    TbAction *pstAction = CActionBase::GetActionByBufferId(&pstSession->m_stUserInfo.m_atbAction[0], pstSession->m_stUserInfo.m_udwActionNum, EN_BUFFER_INFO_SECOND_BUILDING_TASK);
    if(pstAction != NULL)
    {
        TUINT32 udwActionIdx = CActionBase::GetActionIndex(&pstSession->m_stUserInfo.m_atbAction[0], pstSession->m_stUserInfo.m_udwActionNum, pstAction->m_nId);
        pstSession->m_stUserInfo.m_aucActionFlag[udwActionIdx] = EN_TABLE_UPDT_FLAG__DEL;
    }

    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_SetAlGiftTime(SSession *pstSession, TBOOL &bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TINT32 dwRetCode = 0;

    if(0 >= pstUser->m_stAlGifts.m_dwGiftNum)
    {
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("Processcmd_SetAlGiftTime: Gift num error, not enough. [gifu_num=%d][uid=%ld] [seq=%u]",
            pstUser->m_stAlGifts.m_dwGiftNum, pstUser->m_tbPlayer.m_nUid, pstSession->m_udwSeqNo));
        return -1;
    }

    TbAl_gift* ptbAlGift = NULL;
    TbAl_gift_reward* ptbAlGiftReward = NULL;
    TINT64 ddwTime = atoi(pstSession->m_stReqParam.m_szKey[0]);
    ddwTime = ddwTime * 1000 * 1000;
    TINT64 ddwCurTime = CTimeUtils::GetCurTimeUs();

    if(EN_COMMAND_STEP__INIT == pstSession->m_udwCommandStep)
    {
        for(TINT32 dwIdx = pstUser->m_stAlGifts.m_dwGiftNum - 1; dwIdx >= 0; dwIdx--)
        {
            for(TUINT32 udwIdy = 0; udwIdy < pstUser->m_udwAlGiftRewardNum; udwIdy++)
            {
                if(pstUser->m_atbAlGiftReward[udwIdy].m_nGid == pstUser->m_stAlGifts[dwIdx].m_nId)
                {
                    ptbAlGiftReward = &pstUser->m_atbAlGiftReward[udwIdy];
                }
            }

            if(ptbAlGiftReward != NULL && ptbAlGiftReward->m_nStatus != EN_AL_GIFT_STATUS_CLEARED
                && pstUser->m_stAlGifts[dwIdx].m_nCtime + AL_IAP_GIFT_EXPIRE_TIME > ddwCurTime)
            {
                ptbAlGift = &pstSession->m_stUserInfo.m_stAlGifts[dwIdx];
                break;
            }
        }

        if(NULL != ptbAlGift)
        {
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;

            TINT64 ddwNewCTime = ddwCurTime + ddwTime - AL_IAP_GIFT_EXPIRE_TIME;
            ptbAlGift->Set_Ctime(ddwNewCTime);
            CAwsRequest::UpdateItem(pstSession, ptbAlGift);
            bNeedResponse = TRUE;
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
            dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
            if(dwRetCode < 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_SetAlGiftTime: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                return -2;
            }
            return 0;
        }
        else
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_SetAlGiftTime: No suitable al_gift. [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }
    }

    if(EN_COMMAND_STEP__1 == pstSession->m_udwCommandStep)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_SetAlGiftLv(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwLevel = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 10);

    TSE_LOG_INFO(pstSession->m_poServLog, ("Processcmd_SetAlGiftLv: start [dwLevel=%d] [seq=%u]", \
        dwLevel, \
        pstSession->m_stUserInfo.m_udwBSeqNo));

    if(0 >= dwLevel)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_SetAlGiftLv: parameter is illegal, level less equal than 0 [seq=%u]", \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }
    pstSession->m_stReqParam.m_udwAllianceId = pstSession->m_stUserInfo.m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET;
    if(0 == pstSession->m_stReqParam.m_udwAllianceId || pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos == EN_ALLIANCE_POS__REQUEST)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_SetAlGiftLv: m_stReqParam.m_udwAllianceId is 0 or not in an alliance[seq=%u]", \
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -2;
    }

    TINT32 dwGiftPoint = CGameInfo::GetInstance()->m_oJsonRoot["game_al_gift_new"]["a0"][dwLevel-1].asInt();
    pstSession->m_stUserInfo.m_tbAlliance.Set_Gift_point(dwGiftPoint);

    TSE_LOG_INFO(pstSession->m_poServLog, ("Processcmd_SetAlGiftLv: end [seq=%u]", \
        pstSession->m_stUserInfo.m_udwBSeqNo));

    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_SetDragonExcuteTime(SSession* pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;

    TINT32 dwOwnerUid = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TINT32 dwTimeStamp = atoi(pstSession->m_stReqParam.m_szKey[1]);

    TbMarch_action* ptbPrison = NULL;
    TINT32 dwIndex = -1;

    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwPassiveMarchNum; ++udwIdx)
    {
        ptbPrison = &pstUser->m_atbPassiveMarch[udwIdx];

        if(ptbPrison->m_nSclass == EN_ACTION_SEC_CLASS__PRISON_TIMER
            && ptbPrison->m_nSuid == dwOwnerUid)
        {
            dwIndex = udwIdx;
            break;
        }
    }

    if(dwIndex == -1)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }

    TINT64 ddwChangeTime = ptbPrison->m_nEtime - ptbPrison->m_bPrison_param[0].ddwReleaseWait + ptbPrison->m_bPrison_param[0].ddwExcuteWait - dwTimeStamp;
    if (ddwChangeTime < 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -2;
    }

    ptbPrison->Set_Etime(ptbPrison->m_nEtime - ddwChangeTime);
    ptbPrison->m_bPrison_param[0].ddwJoinTimeStamp -= ddwChangeTime;
    ptbPrison->SetFlag(TbMARCH_ACTION_FIELD_PRISON_PARAM);
    pstUser->m_aucPassiveMarchFlag[dwIndex] = EN_TABLE_UPDT_FLAG__CHANGE;

    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_SetDragonAutoReleaseTime(SSession* pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;

    TINT32 dwOwnerId = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TINT32 dwTimeStamp = atoi(pstSession->m_stReqParam.m_szKey[1]);

    TbMarch_action* ptbPrison = NULL;
    TINT32 dwIndex = -1;

    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwPassiveMarchNum; ++udwIdx)
    {
        ptbPrison = &pstUser->m_atbPassiveMarch[udwIdx];

        if(ptbPrison->m_nSclass == EN_ACTION_SEC_CLASS__PRISON_TIMER
            && ptbPrison->m_nSuid == dwOwnerId)
        {
            dwIndex = udwIdx;
            break;
        }
    }

    if(dwIndex == -1)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }

    TINT64 ddwChangeTime = ptbPrison->m_nEtime - dwTimeStamp;
    if (ddwChangeTime < 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -2;
    }

    ptbPrison->m_bPrison_param[0].ddwJoinTimeStamp -= ddwChangeTime;
    ptbPrison->SetFlag(TbMARCH_ACTION_FIELD_PRISON_PARAM);
    ptbPrison->Set_Etime(ptbPrison->m_nEtime - ddwChangeTime);
    pstUser->m_aucPassiveMarchFlag[dwIndex] = EN_TABLE_UPDT_FLAG__CHANGE;

    return 0;

}

TINT32 CProcessSelfSystem::Processcmd_SetThroneTime(SSession* pstSession, TBOOL& bNeedResponse)
{
    TINT32 dwRetCode = 0;

    TINT32 dwPos = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TINT32 dwTimeStamp = atoi(pstSession->m_stReqParam.m_szKey[1]);

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        pstSession->ResetAwsInfo();

        TbMarch_action tbTimer;
        tbTimer.Reset();
        tbTimer.Set_Id(-1 * dwPos + pstSession->m_stReqParam.m_udwSvrId  * -1000000);
        tbTimer.Set_Sid(pstSession->m_stReqParam.m_udwSvrId);
        CAwsRequest::GetItem(pstSession, &tbTimer, ETbMARCH_OPEN_TYPE_PRIMARY);

        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_SetThroneTime:: send aws query req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        TbMarch_action tbTimer;
        tbTimer.Reset();
        dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[0], &tbTimer);
        if(dwRetCode <= 0 || tbTimer.m_nId != (-1 * dwPos))
        {
            tbTimer.Set_Id(-1 * dwPos + pstSession->m_stReqParam.m_udwSvrId  * -100000000);
            tbTimer.Set_Mclass(EN_ACTION_MAIN_CLASS__TIMER);
            tbTimer.Set_Sclass(EN_ACTION_SEC_CLASS__THRONE_TIMER);
            tbTimer.Set_Sid(pstSession->m_stReqParam.m_udwSvrId);
            tbTimer.Set_Tpos(dwPos);
        }

        tbTimer.Set_Etime(dwTimeStamp);

        pstSession->ResetAwsInfo();
        CAwsRequest::UpdateItem(pstSession, &tbTimer);

        TbMap tbWild;
        tbWild.Reset();
        tbWild.Set_Id(dwPos);
        tbWild.Set_Sid(pstSession->m_stReqParam.m_udwSvrId);
        tbWild.Set_March_status_time(dwTimeStamp);
        CAwsRequest::UpdateItem(pstSession, &tbWild);

        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_SetThroneTime:: send aws query req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }
        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_RecoverThroneTime(SSession* pstSession, TBOOL& bNeedResponse)
{
    TINT32 dwRetCode = 0;

    TINT32 dwSid = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TINT32 dwPos = atoi(pstSession->m_stReqParam.m_szKey[1]);

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        pstSession->ResetAwsInfo();

        TbMarch_action tbTimer;
        tbTimer.Reset();
        tbTimer.Set_Id(-1 * dwPos + dwSid * -100000000);
        tbTimer.Set_Sid(dwSid);
        CAwsRequest::GetItem(pstSession, &tbTimer, ETbMARCH_OPEN_TYPE_PRIMARY);

        TbMap tbWild;
        tbWild.Reset();
        tbWild.Set_Id(dwPos);
        tbWild.Set_Sid(dwSid);
        CAwsRequest::GetItem(pstSession, &tbWild, ETbMAP_OPEN_TYPE_PRIMARY);

        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_RecoverThroneTime:: send aws query req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        TbMarch_action tbTimer;
        tbTimer.Reset();
        TbMap tbWild;
        tbWild.Reset();

        for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecAwsRsp.size(); ++udwIdx)
        {
            AwsRspInfo* pstAwsRspInfo = pstSession->m_vecAwsRsp[udwIdx];
            string strTableRawName = CCommonFunc::GetTableRawName(pstAwsRspInfo->sTableName);
            if(strTableRawName == EN_AWS_TABLE_MAP)
            {
                CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &tbWild);
                continue;
            }
            if(strTableRawName == EN_AWS_TABLE_MARCH_ACTION)
            {
                CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &tbTimer);
                continue;
            }
        }

        if(tbWild.m_nId == dwPos)
        {
            if(tbWild.m_nMarch_status_time > 0
            && tbTimer.m_nId == 0)
            {
                tbTimer.Set_Id(-1 * dwPos + dwSid * -100000000);
                tbTimer.Set_Mclass(EN_ACTION_MAIN_CLASS__TIMER);
                tbTimer.Set_Sclass(EN_ACTION_SEC_CLASS__THRONE_TIMER);
                tbTimer.Set_Sid(dwSid);
                tbTimer.Set_Tpos(dwPos);
                tbTimer.Set_Etime(tbWild.m_nMarch_status_time);

                pstSession->ResetAwsInfo();
                CAwsRequest::UpdateItem(pstSession, &tbTimer);
            }
            else if(tbWild.m_nMarch_status_time > 0
                 && tbTimer.m_nId != 0)
            {
                tbTimer.Set_Etime(tbWild.m_nMarch_status_time);
                pstSession->ResetAwsInfo();
                CAwsRequest::UpdateItem(pstSession, &tbTimer);
            }
        }

        if(pstSession->m_vecAwsReq.size() > 0)
        {
            // next procedure
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
            bNeedResponse = TRUE;
            dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
            if(dwRetCode < 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_RecoverThroneTime:: send aws query req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                return -2;
            }
            return 0;
        }

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_CutPrepareTime(SSession* pstSession, TBOOL& bNeedRespons)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;

    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
    {
        TbMarch_action* ptbRallyWar = &pstUser->m_atbMarch[udwIdx];
        if(ptbRallyWar->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR
            && ptbRallyWar->m_nStatus == EN_MARCH_STATUS__PREPARING
            && ptbRallyWar->m_nSuid == pstUser->m_tbPlayer.m_nUid)
        {
            ptbRallyWar->Set_Etime(CTimeUtils::GetUnixTime() + 30);
            pstUser->m_aucMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
        }
    }

    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_SetSmokeTime(SSession* pstSession, TBOOL& bNeedRespons)
{
    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_CaptureDragon(SSession* pstSession, TBOOL& bNeedResponse)
{
    TINT32 dwRetCode = 0;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;

    TINT32 dwCaptorUid = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TINT32 dwCaptorCid = atoi(pstSession->m_stReqParam.m_szKey[1]);

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        if (pstUser->m_tbPlayer.m_nHas_dragon == 0 || pstUser->m_tbPlayer.m_nDragon_status != EN_DRAGON_STATUS_NORMAL)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -1;
        }

        pstSession->ResetAwsInfo();

        TbPlayer tbCaptor;
        tbCaptor.Reset();
        tbCaptor.Set_Uid(dwCaptorUid);
        CAwsRequest::GetItem(pstSession, &tbCaptor, ETbPLAYER_OPEN_TYPE_PRIMARY);

        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_CaptureHero:: send aws query req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }
        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        TbPlayer tbCaptor;
        tbCaptor.Reset();
        dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[0], &tbCaptor);
        if(dwRetCode <= 0 || tbCaptor.m_nUid != dwCaptorUid || tbCaptor.m_nSid != pstUser->m_tbPlayer.m_nSid)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -3;
        }

        TbMarch_action* ptbPrisonTimer = CActionBase::AddNewMarch(pstUser);
        ptbPrisonTimer->Set_Suid(pstUser->m_tbPlayer.m_nUid);
        ptbPrisonTimer->Set_Scid(pstSession->m_stReqParam.m_udwCityId);
        ptbPrisonTimer->Set_Mclass(EN_ACTION_MAIN_CLASS__TIMER);
        ptbPrisonTimer->Set_Sclass(EN_ACTION_SEC_CLASS__PRISON_TIMER);
        ptbPrisonTimer->Set_Status(EN_MARCH_STATUS__DEFENDING);
        ptbPrisonTimer->Set_Btime(CTimeUtils::GetUnixTime());
        ptbPrisonTimer->Set_Ctime(300);

        TUINT32 udwBasicExcuteTime = CCommonBase::GetGameBasicVal(EN_GAME_BASIC_DRAGON_EXECUTED_TIME);
        TUINT32 udwPrisonTime = CCommonBase::GetGameBasicVal(EN_GAME_BASIC_DRAGON_IMPRISON_TIME)
            + pstUser->m_tbPlayer.m_nDragon_level * CCommonBase::GetGameBasicVal(EN_GAME_BASIC_EX_DRAGON_IMPRISON_TIME_PER_LV);

        ptbPrisonTimer->Set_Etime(ptbPrisonTimer->m_nBtime + udwBasicExcuteTime + udwPrisonTime);
        ptbPrisonTimer->m_bPrison_param[0].stDragon.m_ddwCaptured = 0;
        ptbPrisonTimer->m_bPrison_param[0].stDragon.m_ddwLevel = pstUser->m_tbPlayer.m_nDragon_level;
        ptbPrisonTimer->m_bPrison_param[0].stDragon.m_ddwIconId = pstUser->m_tbPlayer.m_nDragon_avatar;
        strncpy(ptbPrisonTimer->m_bPrison_param[0].stDragon.m_szName, pstUser->m_tbPlayer.m_sDragon_name.c_str(), MAX_TABLE_NAME_LEN);
        ptbPrisonTimer->m_bPrison_param[0].stDragon.m_szName[MAX_TABLE_NAME_LEN - 1] = '\0';

        ptbPrisonTimer->m_bPrison_param[0].ddwJoinTimeStamp = ptbPrisonTimer->m_nBtime;
        ptbPrisonTimer->m_bPrison_param[0].ddwReleaseWait = udwBasicExcuteTime + udwPrisonTime;
        ptbPrisonTimer->m_bPrison_param[0].ddwExcuteWait = udwPrisonTime;
        ptbPrisonTimer->m_bPrison_param[0].ddwEscortActionId = 0;
        strncpy(ptbPrisonTimer->m_bPrison_param[0].szSourceUserName, pstUser->m_tbPlayer.m_sUin.c_str(), MAX_TABLE_NAME_LEN);
        ptbPrisonTimer->m_bPrison_param[0].szSourceUserName[MAX_TABLE_NAME_LEN - 1] = '\0';
        strncpy(ptbPrisonTimer->m_bPrison_param[0].szTargetUserName, tbCaptor.m_sUin.c_str(), MAX_TABLE_NAME_LEN);
        ptbPrisonTimer->m_bPrison_param[0].szTargetUserName[MAX_TABLE_NAME_LEN - 1] = '\0';

        ptbPrisonTimer->SetFlag(TbMARCH_ACTION_FIELD_PRISON_PARAM);

        ptbPrisonTimer->Set_Sid(pstUser->m_tbPlayer.m_nSid);
        ptbPrisonTimer->Set_Tal(-1 * dwCaptorUid);
        ptbPrisonTimer->Set_Tbid(CMapBase::GetBlockIdFromPos(dwCaptorCid));
        strncpy(ptbPrisonTimer->m_bPrison_param[0].szSourceAlNick, pstUser->m_tbAlliance.m_sAl_nick_name.c_str(), MAX_TABLE_NAME_LEN);
        ptbPrisonTimer->m_bPrison_param[0].szSourceAlNick[MAX_TABLE_NAME_LEN - 1] = '\0';
        strncpy(ptbPrisonTimer->m_bPrison_param[0].szTargetAlNick, tbCaptor.m_sAl_nick_name.c_str(), MAX_TABLE_NAME_LEN);
        ptbPrisonTimer->m_bPrison_param[0].szTargetAlNick[MAX_TABLE_NAME_LEN - 1] = '\0';
        ptbPrisonTimer->Set_Tuid(dwCaptorUid);
        ptbPrisonTimer->Set_Tpos(dwCaptorCid);

        pstUser->m_tbPlayer.Set_Dragon_status(EN_DRAGON_STATUS_WAIT_KILL);

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_KillSelfDragon(SSession* pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    if(pstUser->m_tbPlayer.m_nDragon_status != EN_DRAGON_STATUS_NORMAL)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }

    pstUser->m_tbPlayer.Set_Dragon_status(EN_DRAGON_STATUS_DEAD);

    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_OpAddGlobalres(SSession* pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;

    TCHAR* pszReward = pstSession->m_stReqParam.m_szKey[0];
    Json::Value jsonReward;
    Json::Reader jsonReader;
    if(jsonReader.parse(pszReward, jsonReward) == FALSE)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -2;
    }

    SSpGlobalRes stGlobalRes;
    stGlobalRes.Reset();
    for(TUINT32 udwIdx = 0; udwIdx < jsonReward.size(); ++udwIdx)
    {
        stGlobalRes.aRewardList[udwIdx].udwType = jsonReward[udwIdx][0U].asUInt();
        stGlobalRes.aRewardList[udwIdx].udwId = jsonReward[udwIdx][1U].asUInt();
        stGlobalRes.aRewardList[udwIdx].udwNum = jsonReward[udwIdx][2U].asUInt();
        stGlobalRes.udwTotalNum++;

        TSE_LOG_DEBUG(pstSession->m_poServLog, ("Processcmd_OpAddGlobalres: [type=%u, id=%u, num=%u][seq=%u]",
            stGlobalRes.aRewardList[udwIdx].udwType,
            stGlobalRes.aRewardList[udwIdx].udwId,
            stGlobalRes.aRewardList[udwIdx].udwNum,
            pstSession->m_udwSeqNo));
    }

    CGlobalResLogic::AddSpGlobalRes(pstUser, pstCity, &stGlobalRes);

    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_HelpSelfAction(SSession* pstSession, TBOOL& bNeddResponse)
{
    TINT32 dwHelpTimes = atoi(pstSession->m_stReqParam.m_szKey[0]);
    SUserInfo *pstUser = &pstSession->m_stUserInfo;

    TbAlliance_action *ptbAlAction = NULL;
    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwAlCanHelpActionNum; ++udwIdx)
    {
        ptbAlAction = pstUser->m_patbAlCanHelpAction[udwIdx];
        if(!CActionBase::IsPlayerOwnedAction(pstUser->m_tbPlayer.m_nUid, ptbAlAction->m_nId))
        {
            continue;
        }

        TINT32 dwRealHelpNum = ptbAlAction->m_nCan_help_num - ptbAlAction->m_nHelped_num;
        if(dwRealHelpNum > dwHelpTimes)
        {
            dwRealHelpNum = dwHelpTimes;
        }

        if(dwRealHelpNum <= 0)
        {
            continue;
        }

        ptbAlAction->Set_Helped_num(ptbAlAction->m_nHelped_num + dwRealHelpNum);
        ptbAlAction->Set_Etime(ptbAlAction->m_nEtime + dwRealHelpNum * CToolBase::Get_AlHelpTime(ptbAlAction->m_nCtime, ptbAlAction->m_nCan_help_num));
        TINT32 dwAcitonIdx = CActionBase::GetAlActionIndex(pstUser->m_atbSelfAlAction, pstUser->m_udwSelfAlActionNum, ptbAlAction->m_nId);
        if(dwAcitonIdx >= 0)
        {
            pstUser->m_aucSelfAlActionFlag[dwAcitonIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
        }
    }

    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_SetAltarBuffTime(SSession* pstSession, TBOOL& bNeedResponse)
{
    TINT32 dwTimeStamp = atoi(pstSession->m_stReqParam.m_szKey[0]);

    SCityInfo *pstCity = &pstSession->m_stUserInfo.m_stCityInfo;

    pstCity->m_stTblData.Set_Altar_buff_etime(dwTimeStamp);

    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_AddVipPoint(SSession* pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;

    TINT32 dwPoint = atoi(pstSession->m_stReqParam.m_szKey[0]);

    CCommonBase::AddVipPoint(pstUser, pstCity, dwPoint);

    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_CleanAllTree(SSession* pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUserInfo = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUserInfo->m_stCityInfo;

    for(TUINT32 idx = 0; idx < pstCity->m_stTblData.m_bBuilding.m_udwNum; idx++)
    {
        SCityBuildingNode *pstBuildingNode = &pstCity->m_stTblData.m_bBuilding[idx];
        if(pstBuildingNode->m_ddwLevel == 0)
        {
            continue;
        }
        TINT32 dwFuncType = CCityBase::GetBuildingFuncType(pstBuildingNode->m_ddwType);
        if(dwFuncType == EN_BUILDING_TYPE__GROVE)
        {
            CCityBase::DelBuildingAtPos(&pstCity->m_stTblData, pstBuildingNode->m_ddwPos);
        }
    }

    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_SetQuestRefresh(SSession* pstSession, TBOOL& bNeedResponse)
{
    TbQuest *pstQuest = &pstSession->m_stUserInfo.m_tbQuest;
    TUINT32 udwType = atoi(pstSession->m_stReqParam.m_szKey[0]);
    //type 1:daily 2:alliance 3: vip
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    TUINT32 udwQuestRefreshTime = 0;
    switch(udwType)
    {
    case EN_TIME_QUEST_TYPE_DAILY:
        udwQuestRefreshTime = udwCurTime + 60;
        pstQuest->m_bDaily_quest[0].m_ddwRTime = udwQuestRefreshTime;
        pstQuest->SetFlag(TbQUEST_FIELD_DAILY_QUEST);
        break;
    case EN_TIME_QUEST_TYPE_ALLIANCE:
        udwQuestRefreshTime = udwCurTime + 60;
        pstQuest->m_bAl_quest[0].m_ddwRTime = udwQuestRefreshTime;
        pstQuest->SetFlag(TbQUEST_FIELD_AL_QUEST);
        break;
    case 3:
        udwQuestRefreshTime = udwCurTime + 60;
        pstQuest->m_bVip_quest[0].m_ddwRTime = udwQuestRefreshTime;
        pstQuest->SetFlag(TbQUEST_FIELD_VIP_QUEST);
        break;
    default:
        break;
    }

    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_ClearAllBuff(SSession* pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;

    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwActionNum; ++udwIdx)
    {
        if(pstUser->m_aucActionFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }
        TbAction* ptbBuff = &pstUser->m_atbAction[udwIdx];

        if(!CActionBase::IsPlayerOwnedAction(pstUser->m_tbPlayer.m_nUid, ptbBuff->m_nId))
        {
            continue;
        }

        if(ptbBuff->m_nSclass != EN_ACTION_SEC_CLASS__ITEM)
        {
            continue;
        }

        if(ptbBuff->m_bParam[0].m_stItem.m_ddwBufferId == EN_BUFFER_INFO_PEACE_TIME)
        {
            continue;
        }

        pstUser->m_aucActionFlag[udwIdx] = EN_TABLE_UPDT_FLAG__DEL;
    }

    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_SetFinishQuest(SSession* pstSession, TBOOL& bNeedResponse)
{
    TbQuest *pstQuest = &pstSession->m_stUserInfo.m_tbQuest;
    TUINT32 udwType = atoi(pstSession->m_stReqParam.m_szKey[0]);
    //type 1:daily 2:alliance 3: vip 4 mistory
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    switch(udwType)
    {
    case EN_TIME_QUEST_TYPE_DAILY:
        for(TUINT32 udwIdx = 0; udwIdx < MAX_TIME_QUEST_NUM;++udwIdx)
        {
            SQuestComm *pstQuestNode = &pstQuest->m_bDaily_quest[0].m_stQuestCom[udwIdx];
            if(pstQuestNode->m_ddwStatus == EN_TIME_QUEST_STATUS_RUNNING)
            {
                pstQuestNode->m_ddwBTime = udwCurTime - pstQuestNode->m_ddwCTime + 20;
            }
        }
        pstQuest->SetFlag(TbQUEST_FIELD_DAILY_QUEST);
        break;
    case EN_TIME_QUEST_TYPE_ALLIANCE:
        for(TUINT32 udwIdx = 0; udwIdx < MAX_TIME_QUEST_NUM; ++udwIdx)
        {
            SQuestComm *pstQuestNode = &pstQuest->m_bAl_quest[0].m_stQuestCom[udwIdx];
            if(pstQuestNode->m_ddwStatus == EN_TIME_QUEST_STATUS_RUNNING)
            {
                pstQuestNode->m_ddwBTime = udwCurTime - pstQuestNode->m_ddwCTime + 20;
            }
        }
        pstQuest->SetFlag(TbQUEST_FIELD_AL_QUEST);
        break;
    case 3:
        for(TUINT32 udwIdx = 0; udwIdx < MAX_TIME_QUEST_NUM; ++udwIdx)
        {
            SQuestComm *pstQuestNode = &pstQuest->m_bVip_quest[0].m_stQuestCom[udwIdx];
            if(pstQuestNode->m_ddwStatus == EN_TIME_QUEST_STATUS_RUNNING)
            {
                pstQuestNode->m_ddwBTime = udwCurTime - pstQuestNode->m_ddwCTime + 20;
            }
        }
        pstQuest->SetFlag(TbQUEST_FIELD_VIP_QUEST);
        break;
    case 4:
        if(TRUE)
        {
            SQuestComm * pstQuestNode = &pstQuest->m_bTimer_gift[0].m_stQuestCom[0];
            if(pstQuestNode->m_ddwStatus == EN_TIME_QUEST_STATUS_RUNNING)
            {
                pstQuestNode->m_ddwBTime = udwCurTime - pstQuestNode->m_ddwCTime + 20;
            }
            pstQuest->SetFlag(TbQUEST_FIELD_TIMER_GIFT);
            break;
        }
        
    default:
        break;
    }
    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_CleanTitle(SSession* pstSession, TBOOL& bNeedResponse)
{
    return -1;
}

TINT32 CProcessSelfSystem::Processcmd_SendBroadcast(SSession* pstSession, TBOOL& bNeedResponse)
{
    TINT32 dwType = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TCHAR *pszReplace = pstSession->m_stReqParam.m_szKey[1];
    TCHAR *pszParam = pstSession->m_stReqParam.m_szKey[2];
    TINT32 dwSid = atoi(pstSession->m_stReqParam.m_szKey[3]);

    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    
    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwBroadcastNum; udwIdx++)
    {
        pstUser->m_atbBroadcast[udwIdx].Reset();
        pstUser->m_aucBroadcastFlag[udwIdx] = EN_TABLE_UPDT_FLAG__UNCHANGE;
    }
    pstUser->m_udwBroadcastNum = 0;
    TINT32 dwRet = CSendMessageBase::SendBroadcast(pstUser, dwSid, pstUser->m_tbPlayer.m_nUid, dwType, pszReplace, pszParam);

    return dwRet;
}

TINT32 CProcessSelfSystem::Processcmd_AddRecommendPlayer(SSession* pstSession, TBOOL& bNeedResponse)
{
    TINT32 dwRetCode = 0;
    if(pstSession->m_stUserInfo.m_tbAlliance.m_nAid == 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }

    TINT32 dwUid = atoi(pstSession->m_stReqParam.m_szKey[0]);

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        CAwsRequest::UserGetByUid(pstSession, dwUid);
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_AddRecommendPlayer:: send aws query req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }
        return 0;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        TbPlayer tbPlayerItem;
        tbPlayerItem.Reset();
        dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[0], &tbPlayerItem);
        if(tbPlayerItem.m_nUid == 0 || tbPlayerItem.m_nSid != pstSession->m_stUserInfo.m_tbAlliance.m_nSid)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -3;
        }

        pstSession->ResetDbInfo();
        CDbRequest::AddRecommendPlayer(pstSession, pstSession->m_stUserInfo.m_tbAlliance.m_nAid, &tbPlayerItem);
        CDbRequest::UpdateRecommendTime(pstSession, CTimeUtils::GetUnixTime());
        // send request
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__DB;
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendDbRequest(pstSession, EN_SERVICE_TYPE_QUERY_MYSQL_REQ);
        if(dwRetCode < 0)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_AddRecommendPlayer: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -4;
        }
        return 0;
    }
    
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_DelInviteRecord(SSession* pstSession, TBOOL& bNeedResponse)
{
    TINT32 dwRetCode = 0;
    if(pstSession->m_stUserInfo.m_tbAlliance.m_nAid == 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }

    TINT32 dwUid = atoi(pstSession->m_stReqParam.m_szKey[0]);
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        TbAl_invite_record tbOneRecord;
        tbOneRecord.Reset();
        tbOneRecord.Set_Aid(pstSession->m_stUserInfo.m_tbAlliance.m_nAid);
        tbOneRecord.Set_Uid(dwUid);
        CAwsRequest::DeleteItem(pstSession, &tbOneRecord);
        // send request
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_DelInviteRecord:: send aws query req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }
        return 0;;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_SetAlStar(SSession* pstSession, TBOOL& bNeedResponse)
{
    if(pstSession->m_stUserInfo.m_tbAlliance.m_nAid == 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }

    TINT32 dwStar = atoi(pstSession->m_stReqParam.m_szKey[0]);
    pstSession->m_stUserInfo.m_tbAlliance.Set_Al_star(dwStar);

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_SetAssistPostTime(SSession* pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;

    TUINT32 udwTimeStamp = atoi(pstSession->m_stReqParam.m_szKey[0]);

    // check param
    if(pstUser->m_tbPlayer.m_nAlpos == EN_ALLIANCE_POS__REQUEST)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NOT_IN_ALLIANCE;
        return -1;
    }
    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwAlAssistAllNum; ++udwIdx)
    {
        if(pstUser->m_atbAlAssistAll[udwIdx].m_nUid == pstUser->m_tbLogin.m_nUid)
        {
            pstUser->m_atbAlAssistAll[udwIdx].Set_Time(udwTimeStamp);
            pstUser->m_aucAlAssistAllFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
        }
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_SetLastUpdateTime(SSession* pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;

    TUINT32 udwTimeStamp = atoi(pstSession->m_stReqParam.m_szKey[0]);

    pstUser->m_tbPlayer.Set_Utime(udwTimeStamp);
    pstUser->m_tbLogin.Set_Utime(udwTimeStamp);

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_ReleaseSelfDragon(SSession* pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;

    TbMarch_action* ptbPrison = NULL;
    TINT32 dwIndex = -1;

    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
    {
        ptbPrison = &pstUser->m_atbMarch[udwIdx];

        if(ptbPrison->m_nSclass == EN_ACTION_SEC_CLASS__PRISON_TIMER
            && ptbPrison->m_nSuid == pstUser->m_tbLogin.m_nUid)
        {
            dwIndex = udwIdx;
            break;
        }
    }

    if(dwIndex == -1)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }

    ptbPrison->Set_Etime(CTimeUtils::GetUnixTime());
    pstUser->m_aucMarchFlag[dwIndex] = EN_TABLE_UPDT_FLAG__CHANGE;

    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_SetThroneTroop(SSession* pstSession, TBOOL& bNeedResponse)
{
    TINT32 dwRetCode = 0;
    TINT32 dwSid = pstSession->m_stReqParam.m_udwSvrId;
    TINT32 dwPos = atoi(pstSession->m_stReqParam.m_szKey[0]);

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        pstSession->ResetAwsInfo();

        TbMap tbWild;
        tbWild.Reset();
        tbWild.Set_Id(dwPos);
        tbWild.Set_Sid(dwSid);
        CAwsRequest::GetItem(pstSession, &tbWild, ETbMAP_OPEN_TYPE_PRIMARY);

        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_SetThroneTroop:: send aws query req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        TbMap tbWild;
        tbWild.Reset();

        for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecAwsRsp.size(); ++udwIdx)
        {
            AwsRspInfo* pstAwsRspInfo = pstSession->m_vecAwsRsp[udwIdx];
            string strTableRawName = CCommonFunc::GetTableRawName(pstAwsRspInfo->sTableName);
            if(strTableRawName == EN_AWS_TABLE_MAP)
            {
                CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &tbWild);
                continue;
            }
        }

        pstSession->ResetAwsInfo();
        if(tbWild.m_nId == dwPos)
        {
            TINT64 ddwTroopTotalNum = CGameInfo::GetInstance()->GetTroopTypeNum();
            for(TUINT32 udwIdx = 0; udwIdx < ddwTroopTotalNum; ++udwIdx)
            {
                tbWild.m_bTroop[0].m_addwNum[udwIdx] = 1000;
            }

            tbWild.SetFlag(TbMAP_FIELD_TROOP);
            CAwsRequest::UpdateItem(pstSession, &tbWild);
        }

        if(pstSession->m_vecAwsReq.size() > 0)
        {
            // next procedure
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
            bNeedResponse = TRUE;
            dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
            if(dwRetCode < 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_SetThroneTroop:: send aws query req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                return -2;
            }
            return 0;
        }

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_SetTradeRefreshTime(SSession* pstSession, TBOOL& bNeedResponse)
{
    return -1;
}

TINT32 CProcessSelfSystem::Processcmd_SetTradeMarchTime(SSession* pstSession, TBOOL& bNeedResponse)
{
    return -1;
}

TINT32 CProcessSelfSystem::Processcmd_SetTradeWaitingTime(SSession* pstSession, TBOOL& bNeedResponse)
{
    return -1;
}

TINT32 CProcessSelfSystem::Processcmd_SetMysteryStoreRefreshTime(SSession* pstSession, TBOOL& bNeedResponse)
{
    return -1;
}

TINT32 CProcessSelfSystem::Processcmd_SetVioletGold(SSession* pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUserInfo = &pstSession->m_stUserInfo;
    TbPlayer *ptbPlayer = &pstUserInfo->m_tbPlayer;

    TINT64 ddwNum = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 10);

    ptbPlayer->Set_Doubloon(ddwNum);

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessSelfSystem::ProcessCmd_SetBountyRefreshTime(SSession *pstSession, TBOOL &bNeedResponse)
{
    TbBounty *pstBounty = &pstSession->m_stUserInfo.m_tbBounty;
    pstBounty->Set_Next_refresh_time(CTimeUtils::GetUnixTime() + 60);
    return 0;
}

TINT32 CProcessSelfSystem::ProcessCmd_SetBountyNodeStarNum(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwId = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TUINT32 dwNum = strtoll(pstSession->m_stReqParam.m_szKey[1], NULL, 10);//num
    
    TbBounty *pstBounty = &pstSession->m_stUserInfo.m_tbBounty;
    Json::Value &jBase = pstBounty->m_jBase;

    if(!jBase.isMember(CCommonFunc::NumToString(dwId)))
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_SetBountyNodeStarNum:: [uid=%u] not such id [id=%u] [seq=%u]",
            pstBounty->m_nUid,dwId, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }
    TUINT32 udwNodwMaxNum = jBase[CCommonFunc::NumToString(dwId)][0U][1U].asUInt();
    
    if(dwNum > udwNodwMaxNum)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_SetBountyNodeStarNum:: [uid=%u] num not right [id=%u max_num=%u num=%u] [id%u] [seq=%u]",
            pstBounty->m_nUid, dwId, udwNodwMaxNum, dwNum,pstSession->m_stUserInfo.m_udwBSeqNo));
        return -2;
    }
    
    TUINT32 udwFinishNum = dwNum;
    
    jBase[CCommonFunc::NumToString(dwId)][0U][0U] = udwFinishNum;
    
    pstBounty->SetFlag(TbBOUNTY_FIELD_BASE);

    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_SetUserCreateTime(SSession* pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUserInfo = &pstSession->m_stUserInfo;
    TbPlayer *ptbPlayer = &pstUserInfo->m_tbPlayer;

    TINT64 ddwTime = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 10);

    ptbPlayer->Set_Ctime(CTimeUtils::GetUnixTime() - ddwTime);

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_SetContinueLoginDay(SSession* pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbUser_stat *pstStat = &pstUser->m_tbUserStat;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;

    TINT32 dwDay = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 10);

    if (dwDay <= 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_SetContinueLoginDay: dwDay[%d] <= 0 [seq=%u]",
            dwDay, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }

    pstStat->Set_Con_login_days(dwDay);

    for (TUINT32 udwIdx = 1; udwIdx <= dwDay; ++udwIdx)
    {
        TINT64 ddwPointAdd = CCommonBase::GetConLoginVipPoint(udwIdx);
        CCommonBase::AddVipPoint(pstUser, pstCity, ddwPointAdd);
    }

    return 0;
}

TINT32 CProcessSelfSystem::ProcessCmd_ClearRecommend(SSession* pstSession, TBOOL& bNeedResponse)
{
    TINT32 dwRetCode = 0;
    if(pstSession->m_stUserInfo.m_tbAlliance.m_nAid == 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        pstSession->ResetDbInfo();
        CDbRequest::DeleteAlPlayerRecommend(pstSession, pstSession->m_stUserInfo.m_tbAlliance.m_nAid, 0);
        // send request
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__DB;
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendDbRequest(pstSession, EN_SERVICE_TYPE_QUERY_MYSQL_REQ);
        if(dwRetCode < 0)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_ClearRecommend: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
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

TINT32 CProcessSelfSystem::ProcessCmd_ClearInvitedCount(SSession* pstSession, TBOOL& bNeedResponse)
{
    pstSession->m_stUserInfo.m_tbPlayer.Set_Invited_num(0);
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessSelfSystem::ProcessCmd_SetPersonalHelpBubble(SSession* pstSession, TBOOL& bNeedResponse)
{
    TINT32 dwTargetStatus = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    if (dwTargetStatus != EN_HELP_BUBBLE_STATUS_ON && dwTargetStatus != EN_HELP_BUBBLE_STATUS_OFF)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_SetPersonalHelpBubble: no such status. [status=%d] [seq=%u]", dwTargetStatus, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }

    if (dwTargetStatus == EN_HELP_BUBBLE_STATUS_ON)
    {
        pstSession->m_stUserInfo.m_tbUserStat.Set_Last_help_bubble_time_out(udwCurTime + 3 * 3600);
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_SetPersonalHelpBubble: set help bubble on success. [uid=%u] [seq=%u]",
            pstSession->m_stUserInfo.m_tbPlayer.m_nUid, pstSession->m_stUserInfo.m_udwBSeqNo));
    }
    else
    {
        pstSession->m_stUserInfo.m_tbUserStat.Set_Last_help_bubble_time_out(udwCurTime - 1);
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessSelfSystem::ProcessCmd_SetHelpBubbleTimeOut(SSession* pstSession, TBOOL& bNeedResponse)
{
    TINT32 dwTimeOut = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 10);

    if (dwTimeOut < 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_SetHelpBubbleTimeOut: time error, must greater than 0. [time=%d] [seq=%u]", dwTimeOut, pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }

    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    pstSession->m_stUserInfo.m_tbUserStat.Set_Last_help_bubble_time_out(udwCurTime + dwTimeOut);

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}


TINT32 CProcessSelfSystem::ProcessCmd_ClearSupportTimeTag(SSession* pstSession, TBOOL& bNeedResponse)
{
    TUINT32 udwUid = (TUINT32)pstSession->m_stUserInfo.m_tbPlayer.m_nUid;

    string sFileName = SUPPORT_SQL_FNAME;
//     string strDBName = SUPPORT_DATABASE_NAME;
//     string strTBName = SUPPORT_TABLE_NAME;

    string strDBName = CConfBase::GetString("support_dbname");
    string strTBName = CConfBase::GetString("support_time_tbname");

    ostringstream os;
    os.str("");
    os << "use " << strDBName << ";\n";
    os << "update " << strTBName << " set last_mail_time = 0, last_reply_time = 0 where uid = " << udwUid << ";";
    
    ofstream file_sql(sFileName.c_str());
    file_sql << os.str().c_str();
    file_sql.close();
    TCHAR szExecCmd[100];

    sprintf(szExecCmd, "./update_support_table.sh %s", sFileName.c_str());
    CMsgBase::SendDelaySystemMsg(szExecCmd);

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessSelfSystem::ProcessCmd_SetTaxTime(SSession* pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;

    TINT32 dwTime = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 10);

    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; udwIdx++)
    {
        if (pstUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__TAX)
        {
            if (pstUser->m_atbMarch[udwIdx].m_nStatus == EN_TAX_STATUS__PREPARING)
            {
                pstUser->m_atbMarch[udwIdx].Set_Etime(CTimeUtils::GetUnixTime() + dwTime);
                if (pstUser->m_atbMarch[udwIdx].m_nBtime > pstUser->m_atbMarch[udwIdx].m_nEtime)
                {
                    pstUser->m_atbMarch[udwIdx].Set_Btime(pstUser->m_atbMarch[udwIdx].m_nBtime - pstUser->m_atbMarch[udwIdx].m_nCtime);
                }
                pstUser->m_aucMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
            }
            break;
        }
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_SetDragonLevel(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT64 ddwDragonLevel = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    

    if(ddwDragonLevel < 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_SetDragonLevel: [ddwDragonLevel=%ld] [seq=%u]", \
                                                ddwDragonLevel, \
                                                pstSession->m_stUserInfo.m_udwBSeqNo));        
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }

    TbPlayer *pstPlayer = &pstSession->m_stUserInfo.m_tbPlayer;

    pstPlayer->Set_Dragon_level(ddwDragonLevel);
    pstPlayer->Set_Dragon_exp(0);

    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_SetPlayerLevel(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT64 ddwPlayerLevel = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    

    if(ddwPlayerLevel < 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_SetPlayerLevel: [ddwPlayerLevel=%ld] [seq=%u]", \
                                                ddwPlayerLevel, \
                                                pstSession->m_stUserInfo.m_udwBSeqNo));        
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }

    TbPlayer *pstPlayer = &pstSession->m_stUserInfo.m_tbPlayer;

    pstPlayer->Set_Level(ddwPlayerLevel);
    
    return 0;

   
}


TINT32 CProcessSelfSystem::Processcmd_SetKnightLevel(SSession *pstSession, TBOOL &bNeedResponse)
{    
    TINT64 ddwKnightId = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TINT32 ddwKnightLevel = atoi(pstSession->m_stReqParam.m_szKey[1]);
    

    if(ddwKnightId < 0 || ddwKnightId > 29)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_SetKnightLevel: [ddwKnightId=%ld] [seq=%u]", \
                                                ddwKnightId, \
                                                pstSession->m_stUserInfo.m_udwBSeqNo));        
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }

    if(ddwKnightLevel < 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_SetKnightLevel: [ddwKnightLevel=%ld] [seq=%u]", \
                                                ddwKnightLevel, \
                                                pstSession->m_stUserInfo.m_udwBSeqNo));        
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -2;
    }


    CGameInfo *poGameInfo = CGameInfo::GetInstance();

    TbCity *pstCity = &pstSession->m_stUserInfo.m_stCityInfo.m_stTblData;
    pstCity->m_bKnight[ddwKnightId].ddwExp = poGameInfo->m_oJsonRoot["game_knight_exp"][ddwKnightLevel][0].asInt64();
    pstCity->SetFlag(TbCITY_FIELD_KNIGHT);
    
    return 0;

}


TINT32 CProcessSelfSystem::Processcmd_SetBuffFailureTime(SSession *pstSession, TBOOL &bNeedResponse)
{    

    /* 物品buff的id
    EN_BUFFER_INFO_PEACE_TIME = 45,
    EN_BUFFER_INFO_ALL_TROOP_ATTACK = 41,
    EN_BUFFER_INFO_ALL_TROOP_LIFE = 42,
    EN_BUFFER_INFO_GOLD_PRODUCTION_PERCENT = 1,
    EN_BUFFER_INFO_FOOD_PRODUCTION_PERCENT = 2,
    EN_BUFFER_INFO_WOOD_PRODUCTION_PERCENT = 3,
    EN_BUFFER_INFO_STON_PRODUCTION_PERCENT = 4,
    EN_BUFFER_INFO_ORE_PRODUCTION_PERCENT = 5,
    EN_BUFFER_INFO_QUEUE_NUM = 44,
    EN_BUFFER_INFO_TROOP_SIZE_PERCENT = 110,
    */


    TINT64 ddwBuffId = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TINT64 ddwFuffFailureTime = strtoll(pstSession->m_stReqParam.m_szKey[1], NULL, 10);
    

    if(ddwBuffId < 0 || ddwBuffId >= EN_BUFFER_INFO_END)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_SetBuffFailureTime: [ddwBuffId=%ld] [seq=%u]", \
                                                ddwBuffId, \
                                                pstSession->m_stUserInfo.m_udwBSeqNo));        
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }


    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbAction *ptbAction = NULL;

    ptbAction = CActionBase::GetActionByBufferId(pstUser->m_atbAction, pstUser->m_udwActionNum, ddwBuffId);

    if (ptbAction != NULL)
    {
        ptbAction->Set_Etime(CTimeUtils::GetUnixTime()+ ddwFuffFailureTime);

        TINT32 dwActionIdx = CActionBase::GetActionIndex(pstUser->m_atbAction, pstUser->m_udwActionNum, ptbAction->m_nId);
        pstUser->m_aucActionFlag[dwActionIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
    }

    return 0;
}


TINT32 CProcessSelfSystem::Processcmd_SetMonsterWildRefreshTime(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    TUINT32 udwPos = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TINT64 ddwRefreshTime = strtoul(pstSession->m_stReqParam.m_szKey[1], NULL, 10);
    const Json::Value& oWildResJson = CWildInfo::GetWildResInfo(pstSession->m_stReqParam.m_udwSvrId);

    //拉取地图
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {

        // set request package
        pstSession->ResetAwsInfo();
        CAwsRequest::MapGet(pstSession, udwPos);

        // send request
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("TableRequest_MapGetByCids: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
        return 0;
    }

    //响应 检查地图信息
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        if(pstSession->m_stCommonResInfo.m_dwRetCode == EN_RET_CODE__SUCCESS)
        {
            AwsRspInfo *pstRes = NULL;
            for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecAwsRsp.size(); ++udwIdx)
            {
                pstRes = pstSession->m_vecAwsRsp[udwIdx];

                string strTableRawName = CCommonFunc::GetTableRawName(pstRes->sTableName);
                if(strTableRawName == EN_AWS_TABLE_MAP)
                {
                    dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[udwIdx],
                        &pstSession->m_atbTmpWild[pstSession->m_udwTmpWildNum]);

                    if(dwRetCode > 0)
                    {
                        pstSession->m_udwTmpWildNum++;
                    }
                }
            }   
            for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_udwTmpWildNum; ++udwIdx)
            {
                if(pstSession->m_atbTmpWild[udwIdx].m_nId == udwPos)
                {
                    pstSession->m_tbTmpMap = pstSession->m_atbTmpWild[udwIdx];
                }
            }
        }
    }

    // 4. 处理
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {

        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        // 设置新map数据
        TbMap *pstMap = &pstSession->m_tbTmpMap;
        pstMap->Set_Id(udwPos);
        pstMap->Set_Expire_time(pstMap->m_nExpire_time - ddwRefreshTime);


        // set package
        pstSession->ResetAwsInfo();
        CAwsRequest::UpdateItem(pstSession, &pstSession->m_tbTmpMap);

        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("TableRequest_CityChangeId: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -5;
        }
        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    return 0;
}


TINT32 CProcessSelfSystem::Processcmd_SetSmokeFireDisappearTime(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    TUINT32 udwPos = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TINT64 ddwSmokeDisappearTime = strtoul(pstSession->m_stReqParam.m_szKey[1], NULL, 10);
    TINT64 ddwFireDisappearTime = strtoul(pstSession->m_stReqParam.m_szKey[2], NULL, 10);
    const Json::Value& oWildResJson = CWildInfo::GetWildResInfo(pstSession->m_stReqParam.m_udwSvrId);

    //拉取地图
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {

        // set request package
        pstSession->ResetAwsInfo();
        CAwsRequest::MapGet(pstSession, udwPos);

        // send request
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("TableRequest_MapGetByCids: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
        return 0;
    }

    //响应 检查地图信息
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        if(pstSession->m_stCommonResInfo.m_dwRetCode == EN_RET_CODE__SUCCESS)
        {
            AwsRspInfo *pstRes = NULL;
            for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecAwsRsp.size(); ++udwIdx)
            {
                pstRes = pstSession->m_vecAwsRsp[udwIdx];

                string strTableRawName = CCommonFunc::GetTableRawName(pstRes->sTableName);
                if(strTableRawName == EN_AWS_TABLE_MAP)
                {
                    dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[udwIdx],
                        &pstSession->m_atbTmpWild[pstSession->m_udwTmpWildNum]);

                    if(dwRetCode > 0)
                    {
                        pstSession->m_udwTmpWildNum++;
                    }
                }
            }   
            for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_udwTmpWildNum; ++udwIdx)
            {
                if(pstSession->m_atbTmpWild[udwIdx].m_nId == udwPos)
                {
                    pstSession->m_tbTmpMap = pstSession->m_atbTmpWild[udwIdx];
                }
            }
        }
    }

    // 4. 处理
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {

        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        // 设置新map数据
        TbMap *pstMap = &pstSession->m_tbTmpMap;
        pstMap->Set_Id(udwPos);
        pstMap->Set_Smoke_end_time(pstMap->Get_Smoke_end_time() - ddwSmokeDisappearTime);
        pstMap->Set_Burn_end_time(pstMap->Get_Burn_end_time() - ddwSmokeDisappearTime);
        
        // set package
        pstSession->ResetAwsInfo();
        CAwsRequest::UpdateItem(pstSession, &pstSession->m_tbTmpMap);

        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("TableRequest_CityChangeId: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -5;
        }
        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    return 0;
}




TINT32 CProcessSelfSystem::Processcmd_SetConsecutiveLogin(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT64 ddwDayNum = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 10);


    if(ddwDayNum < 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("Processcmd_SetConsecutiveLogin: [ddwDayNum=%ld] [seq=%u]", \
                                                ddwDayNum, \
                                                pstSession->m_stUserInfo.m_udwBSeqNo));        
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }

    TbUser_stat *pstUserStat = &pstSession->m_stUserInfo.m_tbUserStat;

    pstUserStat->Set_Con_login_days(ddwDayNum);

    return 0;
}

TINT32 CProcessSelfSystem::ProcessCmd_SetAlWallNewestTime( SSession* pstSession, TBOOL& bNeedResponse )
{
    TUINT32 udwSetTime = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);

    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    if(pstUser->m_udwWallNum)
    {
        pstUser->m_atbWall[0].Set_Time(udwSetTime);
        pstUser->m_aucWallFlag[0] == EN_TABLE_UPDT_FLAG__CHANGE;
    }
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessSelfSystem::ProcessCmd_SetTaskInShowList( SSession* pstSession, TBOOL& bNeedResponse )
{
    TUINT32 udwTaskId = atoi(pstSession->m_stReqParam.m_szKey[0]);

    TbTask *ptbTask = &pstSession->m_stUserInfo.m_tbTask;
    TUINT32 idx = 0;

    if(ptbTask->m_bTask_normal.m_udwNum >= TBTASK_TASK_NORMAL_MAX_NUM)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }

    if(BITTEST(ptbTask->m_bTask_finish[0].m_bitTask, udwTaskId))
    {
        BITCLEAR(ptbTask->m_bTask_finish[0].m_bitTask, udwTaskId);
        ptbTask->SetFlag(TbTASK_FIELD_TASK_FINISH);
    }

    for(idx = 0; idx < ptbTask->m_bTask_normal.m_udwNum; idx++)
    {
        if(ptbTask->m_bTask_normal[idx].m_ddwId == udwTaskId)
        {
            break;
        }
    }
    if(idx < ptbTask->m_bTask_normal.m_udwNum)
    {
        return 0;
    }

    CQuestLogic::InsertQuest(&pstSession->m_stUserInfo, &pstSession->m_stUserInfo.m_stCityInfo, udwTaskId, ptbTask->m_bTask_normal.m_udwNum);
    ptbTask->m_bTask_normal.m_udwNum++;

    ptbTask->SetFlag(TbTASK_FIELD_TASK_NORMAL);

    return 0;
}

TINT32 CProcessSelfSystem::ProcessCmd_SetPersonGuideFlag( SSession* pstSession, TBOOL& bNeedResponse )
{
    TINT32 dwGuideFlag = atoi(pstSession->m_stReqParam.m_szKey[0]);

    TbPlayer *ptbPlayer = &pstSession->m_stUserInfo.m_tbPlayer;

    BITCLEAR(ptbPlayer->m_bPerson_guide[0].m_bitGuide, dwGuideFlag);
    ptbPlayer->SetFlag(TbPLAYER_FIELD_PERSON_GUIDE);

    return 0;
}

TINT32 CProcessSelfSystem::ProcessCmd_ResetMonsterHit(SSession* pstSession, TBOOL& bNeedResponse)
{
    TINT32 dwLv = atoi(pstSession->m_stReqParam.m_szKey[0]);

    TbPlayer *ptbPlayer = &pstSession->m_stUserInfo.m_tbPlayer;

    if (dwLv == 0)
    {
        ptbPlayer->m_bMonster_hit[0].Reset();
        ptbPlayer->m_bLeader_monster_gen[0].Reset();
        ptbPlayer->m_bLeader_monster_kill[0].Reset();
    }
    else
    {
        ptbPlayer->m_bMonster_hit[0].addwNum[dwLv] = 0;
        ptbPlayer->m_bLeader_monster_gen[0].addwNum[dwLv] = 0;
        ptbPlayer->m_bLeader_monster_kill[0].addwNum[dwLv] = 0;
    }

    ptbPlayer->SetFlag(TbPLAYER_FIELD_MONSTER_HIT);
    ptbPlayer->SetFlag(TbPLAYER_FIELD_LEADER_MONSTER_GEN);
    ptbPlayer->SetFlag(TbPLAYER_FIELD_LEADER_MONSTER_KILL);


    pstSession->m_stUserInfo.m_tbUserStat.m_bDragon_level_finish.Reset();
    pstSession->m_stUserInfo.m_tbUserStat.SetFlag(TbUSER_STAT_FIELD_DRAGON_LEVEL_FINISH);

    return 0;
}

TINT32 CProcessSelfSystem::ProcessCmd_SetSystemTime(SSession* pstSession, TBOOL& bNeedResponse)
{
    TINT32 dwTime = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 10);

    ostringstream oss;
    oss.str("");
    oss << "cd ../upload; ./change_time.sh " << dwTime;

    system(oss.str().c_str());

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessSelfSystem::ProcessCmd_SetMap(SSession* pstSession, TBOOL& bNeedResponse)
{
    enum E_Map_Field
    {
        EN_MAP_FIELD_LEVEL,
        EN_MAP_FIELD_REWARD_LEFT,
        EN_MAP_FIELD_EXPIRE_TIME,
        EN_MAP_FIELD_TYPE,
    };
    TINT32 dwRetCode = 0;

    TUINT32 udwPos = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);

    TUINT32 udwFieldId = strtoul(pstSession->m_stReqParam.m_szKey[1], NULL, 10);
    TINT64 ddwVal = strtoll(pstSession->m_stReqParam.m_szKey[2], NULL, 10);

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        pstSession->ResetAwsInfo();
        CAwsRequest::MapGet(pstSession, udwPos);
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_SetMap: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
        return 0;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        if (pstSession->m_stCommonResInfo.m_dwRetCode == EN_RET_CODE__SUCCESS)
        {
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
            }
            for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_udwTmpWildNum; ++udwIdx)
            {
                if (pstSession->m_atbTmpWild[udwIdx].m_nId == udwPos)
                {
                    pstSession->m_tbTmpMap = pstSession->m_atbTmpWild[udwIdx];
                }
            }
        }
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        TbMap *pstMap = &pstSession->m_tbTmpMap;

        switch (udwFieldId)
        {
        case EN_MAP_FIELD_LEVEL:
            pstMap->Set_Level(ddwVal);
            break;
        case EN_MAP_FIELD_REWARD_LEFT:
            pstMap->Set_Reward_left(ddwVal);
            break;
        case EN_MAP_FIELD_EXPIRE_TIME:
            pstMap->Set_Expire_time(ddwVal);
            break;
        case EN_MAP_FIELD_TYPE:
            pstMap->Set_Type(ddwVal);
            break;
        default:
            break;
        }

        pstSession->ResetAwsInfo();
        CAwsRequest::UpdateItem(pstSession, &pstSession->m_tbTmpMap);
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

TINT32 CProcessSelfSystem::ProcessCmd_SetMapExpire(SSession* pstSession, TBOOL& bNeedResponse)
{
    TINT32 dwRetCode = 0;

    TUINT32 udwPos = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);
    TUINT32 udwExpireLeft = strtoul(pstSession->m_stReqParam.m_szKey[1], NULL, 10);

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        pstSession->ResetAwsInfo();
        CAwsRequest::MapGet(pstSession, udwPos);
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_SetMapExpire: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
        return 0;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        if (pstSession->m_stCommonResInfo.m_dwRetCode == EN_RET_CODE__SUCCESS)
        {
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
            }
            for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_udwTmpWildNum; ++udwIdx)
            {
                if (pstSession->m_atbTmpWild[udwIdx].m_nId == udwPos)
                {
                    pstSession->m_tbTmpMap = pstSession->m_atbTmpWild[udwIdx];
                }
            }
        }
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        TbMap *pstMap = &pstSession->m_tbTmpMap;

        if (pstMap->m_nId == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_SetMapExpire: EN_RET_CODE__PARSE_PACKAGE_ERR [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }

        TUINT32 udwMapType = CMapLogic::GetWildClass(pstMap->m_nSid, pstMap->m_nType);

        if (udwMapType == EN_WILD_CLASS_RES || udwMapType == EN_WILD_CLASS_MONSTER_NEST ||
            udwMapType == EN_WILD_CLASS_MONSTER || udwMapType == EN_WILD_CLASS_LEADER_MONSTER)
        {
            pstMap->Set_Expire_time(CTimeUtils::GetUnixTime() + udwExpireLeft);
        }
        else
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_SetMapExpire: map_type[%ld] map_class[%u] error [seq=%u]",
                pstMap->m_nType, udwMapType, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }

        pstSession->ResetAwsInfo();
        CAwsRequest::UpdateItem(pstSession, &pstSession->m_tbTmpMap);
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_SetMapExpire: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
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

TINT32 CProcessSelfSystem::ProcessCmd_SetRallyTime(SSession* pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();

    TUINT32 udwPrepareTime = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TUINT32 udwMarchTime = atoi(pstSession->m_stReqParam.m_szKey[1]);

    TINT32 dwActionIdx = -1;
    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
    {
        TbMarch_action *ptbAction = &pstUser->m_atbMarch[udwIdx];
        if ((ptbAction->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR 
            || ptbAction->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR_THRONE)
            && ptbAction->m_nSuid == pstUser->m_tbPlayer.m_nUid)
        {
            dwActionIdx = udwIdx;
            break;
        }
    }

    if (udwPrepareTime <= 0 || udwMarchTime <= 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_SetRallyTime: prepare[%u] march[%u] should not be 0 [seq=%u]",
            udwPrepareTime, udwMarchTime, pstSession->m_udwSeqNo));
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }

    if (dwActionIdx < 0)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_SetRallyTime: user[%ld] no rallywar [seq=%u]",
            pstUser->m_tbPlayer.m_nUid, pstSession->m_udwSeqNo));
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -2;
    }
    else
    {
        TbMarch_action *ptbRally = &pstUser->m_atbMarch[dwActionIdx];

        ptbRally->m_bParam[0].m_ddwPrepareTime = udwPrepareTime;
        ptbRally->m_bParam[0].m_ddwMarchingTime = udwMarchTime;
        ptbRally->SetFlag(TbMARCH_ACTION_FIELD_PARAM);

        switch (ptbRally->m_nStatus)
        {
        case EN_MARCH_STATUS__PREPARING:
            ptbRally->Set_Ctime(udwCurTime - ptbRally->m_nBtime + udwPrepareTime);
            ptbRally->Set_Etime(ptbRally->m_nBtime + ptbRally->m_nCtime);
            break;
        case EN_MARCH_STATUS__WAITING:
            break;
        case EN_MARCH_STATUS__MARCHING:
            ptbRally->Set_Ctime(udwCurTime - ptbRally->m_nBtime + udwMarchTime);
            ptbRally->Set_Etime(ptbRally->m_nBtime + ptbRally->m_nCtime);
            break;
        case EN_MARCH_STATUS__FIGHTING:
            break;
        case EN_MARCH_STATUS__RETURNING:
            ptbRally->Set_Ctime(udwCurTime - ptbRally->m_nBtime + udwMarchTime);
            ptbRally->Set_Etime(ptbRally->m_nBtime + ptbRally->m_nCtime);
            break;
        default:
            break;
        }

        pstUser->m_aucMarchFlag[dwActionIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
    }

    return 0;
}

TINT32 CProcessSelfSystem::ProcessCmd_SetCampProtectTime(SSession* pstSession, TBOOL& bNeedResponse)
{
    TUINT32 udwPos = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TUINT32 udwTime = atoi(pstSession->m_stReqParam.m_szKey[1]);

    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;

    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();

    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
    {
        TbMarch_action *ptbAction = &pstUser->m_atbMarch[udwIdx];
        if (ptbAction->m_nSclass == EN_ACTION_SEC_CLASS__CAMP && ptbAction->m_nStatus == EN_MARCH_STATUS__CAMPING_WITH_PEACETIME &&
            ptbAction->m_nSuid == ptbPlayer->m_nUid && ptbAction->m_nTpos == udwPos)
        {
            ptbAction->Set_Etime(udwCurTime + udwTime);
            ptbAction->Set_Ctime(ptbAction->m_nEtime - ptbAction->m_nBtime);
            pstUser->m_aucMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
        }
    }

    return 0;
}

TINT32 CProcessSelfSystem::ProcessCmd_SetTrialLock(SSession* pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;

    if (ptbPlayer->m_nTrial_init == 0)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }

    if (ptbPlayer->m_bTrial_monster_normal[0].udwAtkTime == MAX_TRIAL_ATK_TIME ||
        ptbPlayer->m_bTrial_monster_rage[0].udwAtkTime == MAX_TRIAL_ATK_TIME)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -2;
    }

    ptbPlayer->Set_Trial_rage_open(0);
    ptbPlayer->Set_Trial_rage_mode(0);

    return 0;
}

TINT32 CProcessSelfSystem::ProcessCmd_AddAlGiftPoint(SSession* pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbAlliance* ptbAlliance = &pstUser->m_tbAlliance;

    TINT32 dwPoint = atoi(pstSession->m_stReqParam.m_szKey[0]);

    if (dwPoint > 0)
    {
        ptbAlliance->Set_Gift_point(ptbAlliance->m_nGift_point += dwPoint);
    }

    return 0;
}

TINT32 CProcessSelfSystem::ProcessCmd_AddAlLoyaltyAndFund(SSession* pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbAlliance* ptbAlliance = &pstUser->m_tbAlliance;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();

    TINT32 dwPoint = atoi(pstSession->m_stReqParam.m_szKey[0]);

    if (dwPoint > 0)
    {
        if (ptbPlayer->m_nLoy_time / AL_FUND_GET_INTERVAL != udwCurTime / AL_FUND_GET_INTERVAL)
        {
            ptbPlayer->Set_Loy_itv(0);
        }
        ptbPlayer->Set_Loy_time(udwCurTime);
        ptbPlayer->Set_Loy_itv(ptbPlayer->m_nLoy_itv + dwPoint);
        ptbPlayer->Set_Loy_cur(ptbPlayer->m_nLoy_cur + dwPoint);
        ptbPlayer->Set_Loy_all(ptbPlayer->m_nLoy_all + dwPoint);
        ptbAlliance->Set_Fund(ptbAlliance->m_nFund + dwPoint);
        ptbPlayer->Set_Send_al_help_num(ptbPlayer->m_nSend_al_help_num + dwPoint / AL_FUND_ADD_PER_TIME);
    }

    return 0;
}

TINT32 CProcessSelfSystem::ProcessCmd_SetAttackMoveTime(SSession* pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();

    TUINT32 udwTime = atoi(pstSession->m_stReqParam.m_szKey[0]);

    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwActionNum; ++udwIdx)
    {
        TbAction *ptbAction = &pstUser->m_atbAction[udwIdx];
        if (ptbAction->m_nMclass == EN_ACTION_MAIN_TASK_ATTACK_MOVE &&
            ptbAction->m_nSclass == EN_ACTION_SEC_CLASS__ATTACK_MOVE)
        {
            ptbAction->Set_Etime(udwCurTime + udwTime);
            ptbAction->Set_Ctime(ptbAction->m_nEtime - ptbAction->m_nBtime);
            pstUser->m_aucActionFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;

            for (TUINT32 udwIdy = 0; udwIdy < pstUser->m_udwWildNum; ++udwIdy)
            {
                TbMap *ptbWild = &pstUser->m_atbWild[udwIdy];
                if (ptbWild->m_nType == EN_WILD_TYPE__CITY && ptbWild->m_nUid == ptbPlayer->m_nUid)
                {
                    ptbWild->Set_Burn_end_time(udwCurTime + udwTime);
                    pstUser->m_aucWildFlag[udwIdy] = EN_TABLE_UPDT_FLAG__CHANGE;
                }
            }
        }
    }

    return 0;
}

TINT32 CProcessSelfSystem::ProcessCmd_SetRallyAttackSlotAllOpen(SSession* pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;

    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
    {
        TbMarch_action *ptbAction = &pstUser->m_atbMarch[udwIdx];
        if (ptbAction->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR &&
            ptbAction->m_nSuid == ptbPlayer->m_nUid)
        {
            ptbAction->m_bRally_atk_slot.m_udwNum = CCommonBase::GetGameBasicVal(EN_GAME_BASIC_RALLY_SLOT_LIMIT);
            ptbAction->SetFlag(TbMARCH_ACTION_FIELD_RALLY_ATK_SLOT);
            pstUser->m_aucMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
        }
    }

    return 0;
}


TINT32 CProcessSelfSystem::ProcessCmd_SetAlGiftDisappearTime(SSession* pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SAlGiftList* pstAlGifts = &pstUser->m_stAlGifts;
    TINT64 ddwCurTime = CTimeUtils::GetCurTimeUs();
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();

    TUINT32 udwTime = atoi(pstSession->m_stReqParam.m_szKey[0]);

    for (TINT32 dwIdx = 0; dwIdx < pstAlGifts->m_dwGiftNum && dwIdx < MAX_AL_IAP_GIFT_NUM; ++dwIdx)
    {
        TbAl_gift* ptbAlGift = &((*pstAlGifts)[dwIdx]);
        TbAl_gift_reward *ptbAlGiftReward = NULL;
        TINT32 dwStatus = EN_AL_GIFT_STATUS_NORMAL;

        for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwAlGiftRewardNum; udwIdx++)
        {
            if (pstUser->m_atbAlGiftReward[udwIdx].m_nGid == ptbAlGift->m_nId)
            {
                ptbAlGiftReward = &pstUser->m_atbAlGiftReward[udwIdx];
                break;
            }
        }
        if (NULL == ptbAlGiftReward)
        {
            dwStatus = EN_AL_GIFT_STATUS_NORMAL;
        }
        else
        {
            dwStatus = ptbAlGiftReward->m_nStatus;
        }

        if (dwStatus == EN_AL_GIFT_STATUS_OPENED || dwStatus == EN_AL_GIFT_STATUS_EXPIRED)
        {
            TINT64 ddwTimeDiff = udwTime;
            ptbAlGift->Set_Id(ddwCurTime + ddwTimeDiff * 1000000 - AL_IAP_GIFT_EXPIRE_TIME * 2);
            ptbAlGift->Set_Ctime(ptbAlGift->m_nId);
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("moondo test: ProcessCmd_SetAlGiftDisappearTime: curtime[%u] udwtime[%u] id[%ld] ctime[%ld]",
                udwCurTime, udwTime, ptbAlGift->m_nId, ptbAlGift->m_nCtime));
            pstAlGifts->m_aucUpdateFlag[dwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
        }
    }

    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_OpChangeGlobalres(SSession *pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;

    TCHAR* pszReward = pstSession->m_stReqParam.m_szKey[0];

    Json::Value jsonReward;
    Json::Reader jsonReader;
    if (jsonReader.parse(pszReward, jsonReward) == FALSE)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }

    SSpGlobalRes stGlobalRes;
    stGlobalRes.Reset();
    for (TUINT32 udwIdx = 0; udwIdx < jsonReward.size(); ++udwIdx)
    {
        TUINT32 udwType = jsonReward[udwIdx][0U].asUInt();
        TUINT32 udwId = jsonReward[udwIdx][1U].asUInt();
        TINT32 dwNum = jsonReward[udwIdx][2U].asInt();
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("Processcmd_OpChangeGlobalres: [type=%u, id=%u, num=%d] [seq=%u]",
            udwType, udwId, dwNum, pstSession->m_udwSeqNo));

        if (dwNum > 0)
        {
            CGlobalResLogic::AddGlobalRes(pstUser, pstCity, udwType, udwId, dwNum);
        }
        else if (dwNum < 0)
        {
            CGlobalResLogic::CostGlobalRes(pstUser, pstCity, udwType, udwId, -1 * dwNum);
        }
    }

    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_OpSetGlobalres(SSession *pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;

    TCHAR* pszReward = pstSession->m_stReqParam.m_szKey[0];

    Json::Value jsonReward;
    Json::Reader jsonReader;
    if (jsonReader.parse(pszReward, jsonReward) == FALSE)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        return -1;
    }

    SSpGlobalRes stGlobalRes;
    stGlobalRes.Reset();
    for (TUINT32 udwIdx = 0; udwIdx < jsonReward.size(); ++udwIdx)
    {
        TUINT32 udwType = jsonReward[udwIdx][0U].asUInt();
        TUINT32 udwId = jsonReward[udwIdx][1U].asUInt();
        TUINT32 udwNum = jsonReward[udwIdx][2U].asUInt();
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("Processcmd_OpChangeGlobalres: [type=%u, id=%u, num=%u] [seq=%u]",
            udwType, udwId, udwNum, pstSession->m_udwSeqNo));

        CGlobalResLogic::SetGlobalRes(pstUser, pstCity, udwType, udwId, udwNum);
    }

    return 0;
}


TINT32 CProcessSelfSystem::Processcmd_SetIdolTime(SSession* pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();

    TINT32 dwPos = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TINT32 dwLeftTime = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TINT32 dwStatus = atoi(pstSession->m_stReqParam.m_szKey[2]);

    for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_udwIdolNum; udwIdx++)
    {
        if (pstSession->m_atbIdol[udwIdx].m_nPos == dwPos
            && pstSession->m_atbIdol[udwIdx].m_nStatus == dwStatus)
        {
            //idol
            pstSession->m_atbIdol[udwIdx].Set_End_time(udwCurTime + dwLeftTime);
            pstSession->m_ucIdolFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;

            //map
            pstSession->m_tbTmpMap.Set_Sid(pstSession->m_atbIdol[udwIdx].m_nSid);
            pstSession->m_tbTmpMap.Set_Id(pstSession->m_atbIdol[udwIdx].m_nPos);
            pstSession->m_tbTmpMap.Set_Time_end(pstSession->m_atbIdol[udwIdx].m_nEnd_time);
            pstSession->m_ucTmpMapItemFlag = EN_TABLE_UPDT_FLAG__CHANGE;

            //action
            TbMarch_action *ptbAction = &pstUser->m_atbMarch[pstUser->m_udwMarchNum];
            ptbAction->Set_Suid(0);
            ptbAction->Set_Id(CActionBase::GenMapActionId(pstSession->m_stReqParam.m_udwSvrId, dwPos));
            ptbAction->Set_Sid(pstSession->m_stReqParam.m_udwSvrId);
            ptbAction->Set_Btime(pstSession->m_atbIdol[udwIdx].m_nEnd_time - pstSession->m_atbIdol[udwIdx].m_nLast_time);
            ptbAction->Set_Ctime(pstSession->m_atbIdol[udwIdx].m_nLast_time);
            ptbAction->Set_Etime(pstSession->m_atbIdol[udwIdx].m_nEnd_time);
            pstUser->m_aucMarchFlag[pstUser->m_udwMarchNum] = EN_TABLE_UPDT_FLAG__CHANGE;
            pstUser->m_udwMarchNum++;

            break;
        }
    }

    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_FillIdolRank(SSession* pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();

    TINT32 dwPos = atoi(pstSession->m_stReqParam.m_szKey[0]);
    string szContent = pstSession->m_stReqParam.m_szKey[1];

    for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_udwIdolNum; udwIdx++)
    {
        if (pstSession->m_atbIdol[udwIdx].m_nPos == dwPos
            && pstSession->m_atbIdol[udwIdx].m_nStatus == EN_IDOL_STATUS__CONTEST_PERIOD)
        {
            //idol
            TbIdol *ptbIdol = &pstSession->m_atbIdol[udwIdx];
            for (TUINT32 udwIdy = 0; udwIdy < 50; udwIdy++)
            {
                TUINT32 udwAlid = 100000 + udwIdy;
                Json::Value jTmp = Json::Value(Json::objectValue);
                jTmp["alid"] = udwAlid;
                jTmp["point"] = 1;
                jTmp["al_nick"] = szContent + CCommonFunc::NumToString(udwIdy);
                jTmp["al_name"] = szContent + CCommonFunc::NumToString(udwIdy);
                jTmp["time"] = CTimeUtils::GetUnixTime();
                jTmp["rank"] = 0;
                ptbIdol->m_jRank.append(jTmp);

            }
            ptbIdol->SetFlag(TbIDOL_FIELD_RANK);
            pstSession->m_ucIdolFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
            break;
        }
    }

    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_SetTitleExpireTime(SSession* pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();

    TINT32 dwTitleId = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TINT32 dwLeftTime = atoi(pstSession->m_stReqParam.m_szKey[1]);

    const Json::Value &jTitle = CGameInfo::GetInstance()->m_oJsonRoot["game_title"];

    ostringstream oss;
    for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_stTitleInfoList.udwNum; udwIdx++)
    {
        TbTitle *ptbTitle = &pstSession->m_stTitleInfoList.atbTitle[udwIdx];
        if (ptbTitle->m_nId == dwTitleId)
        {
            oss.str("");
            oss << pstSession->m_stTitleInfoList.atbTitle[udwIdx].m_nId;
            TINT32 dwExpireTime = 0;
            if (jTitle.isMember(oss.str()))
            {
                dwExpireTime = jTitle[oss.str()]["time"].asInt();
            }
            ptbTitle->Set_Dub_time(udwCurTime - dwExpireTime + dwLeftTime);
            pstSession->m_stTitleInfoList.aucFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
            if (pstSession->m_tbThrone.m_nOccupy_time >= ptbTitle->m_nDub_time)
            {
                pstSession->m_tbThrone.Set_Occupy_time(ptbTitle->m_nDub_time - 1);
            }
            break;
        }
    }

    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_SetTaxBeginTime(SSession* pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();

    TINT32 dwTime = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 10);

    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; udwIdx++)
    {
        if (pstUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__TAX)
        {
            if (pstUser->m_atbMarch[udwIdx].m_nStatus == EN_TAX_STATUS__PREPARING)
            {
                pstUser->m_atbMarch[udwIdx].Set_Etime(CTimeUtils::GetUnixTime() + dwTime);
                
                pstUser->m_atbMarch[udwIdx].Set_Btime(pstUser->m_atbMarch[udwIdx].m_nEtime);

                pstUser->m_aucMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
            }
            break;
        }
    }

    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_SetTaxIntervalTime(SSession* pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();

    TINT32 dwTime = strtoll(pstSession->m_stReqParam.m_szKey[0], NULL, 10);

    if (dwTime < 3600)
    {
        return -1;
    }

    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; udwIdx++)
    {
        if (pstUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__TAX)
        {
            if (pstUser->m_atbMarch[udwIdx].m_nStatus == EN_TAX_STATUS__PREPARING)
            {
                pstUser->m_atbMarch[udwIdx].Set_Ctime(dwTime);

                pstUser->m_aucMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
            }
            break;
        }
    }

    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_SetThroneTimeNew(SSession* pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();

    TINT32 dwLeftTime = atoi(pstSession->m_stReqParam.m_szKey[0]);

    if (pstSession->m_tbThrone.m_nStatus == EN_THRONE_STATUS__PEACE_TIME
        || pstSession->m_tbThrone.m_nStatus == EN_THRONE_STATUS__CONTEST_PERIOD)
    {
        if (pstSession->m_tbThrone.m_nEnd_time > 0)
        {
            pstSession->m_tbThrone.Set_End_time(udwCurTime + dwLeftTime);

            //action
            TbMarch_action *ptbAction = &pstUser->m_atbMarch[pstUser->m_udwMarchNum];
            ptbAction->Set_Suid(0);
            ptbAction->Set_Id(CActionBase::GenMapActionId(pstSession->m_stReqParam.m_udwSvrId, pstSession->m_tbThrone.m_nPos));
            ptbAction->Set_Sid(pstSession->m_stReqParam.m_udwSvrId);
            ptbAction->Set_Etime(pstSession->m_tbThrone.m_nEnd_time);
            pstUser->m_aucMarchFlag[pstUser->m_udwMarchNum] = EN_TABLE_UPDT_FLAG__CHANGE;
            pstUser->m_udwMarchNum++;
        }
    }

    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_GenPeaceTime(SSession* pstSession, TBOOL& bNeedResponse)
{
    SUserInfo *pstUser = &pstSession->m_stUserInfo;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;

    pstCity->m_stActionStat.m_bCanPeaceTime = TRUE;
    TINT64 ddwTmpStatus = pstUser->m_tbPlayer.m_nStatus;
    if (pstUser->m_tbPlayer.m_nStatus & EN_CITY_STATUS__ON_SPECIAL_WILD)
    {
        pstUser->m_tbPlayer.m_nStatus = pstUser->m_tbPlayer.m_nStatus ^ EN_CITY_STATUS__ON_SPECIAL_WILD;
    }
    CItemLogic::UseItem(pstUser, pstCity, 375, -1, 1, -1, FALSE);
    pstUser->m_tbPlayer.Set_Status(ddwTmpStatus | EN_CITY_STATUS__AVOID_WAR);

    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwWildNum; udwIdx++)
    {
        TbMap *ptbWild = &pstUser->m_atbWild[udwIdx];
        if (ptbWild->m_nUid == ptbPlayer->m_nUid
            && ptbWild->m_nType == EN_WILD_TYPE__CAMP)
        {
            ptbWild->Set_Status(ptbWild->m_nStatus | EN_CITY_STATUS__AVOID_WAR);
            ptbWild->Set_Time_end(CTimeUtils::GetUnixTime() + 3600 * 24 * 5);
            pstUser->m_aucWildFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
        }
    }

    return 0;
}

TINT32 CProcessSelfSystem::Processcmd_SetIapPromoteNum(SSession* pstSession, TBOOL& bNeedResponse)
{
    TUINT32 udwNum = atoi(pstSession->m_stReqParam.m_szKey[0]);

    pstSession->m_stUserInfo.m_tbLogin.Set_Iap_promote_gem_num(udwNum);

    return 0;
}