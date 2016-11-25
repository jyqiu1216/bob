#include "pushdata_action.h"
#include "common_func.h"
#include "base/log/wtselogger.h"
#include "sendmessage_base.h"
#include "action_base.h"
#include "tool_base.h"
#include "global_serv.h"
#include "map_base.h"
#include "map_logic.h"
#include "wild_info.h"
#include "output_conf.h"
#include "player_base.h"
#include "json_result_generator.h"

using namespace wtse::log;

TINT32 CAuPushData::AuPushData( SSession *pstSession )
{
    TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("[wavetest---march]: push_data beg [seq=%u]---------------", pstSession->m_udwSeqNo));

    TINT32 dwRet = 0;

    CAuPushDataNode *pobjNode = &pstSession->m_objAuPushDataNode;
    CPushAlData *pobjSourceAlData = &pobjNode->m_objPushDataSourceAl;
    CPushAlData *pobjTargetAlData = &pobjNode->m_objPushDataTargetAl;
    CPushMapData *pobjMapData = &pobjNode->m_objPushDataMap;

    //将主用户加入更新列表
    AddPushData_AlUid(pobjSourceAlData, pstSession->m_stSourceUser.m_tbPlayer.m_nUid);
    AddPushData_AlUid(pobjSourceAlData, pstSession->m_stTargetUser.m_tbPlayer.m_nUid);

    //计算需要删除的alaction
    //DelUnExistAction(pobjSourceAlData);
    //DelUnExistAction(pobjTargetAlData);

    //更新需要刷新数据的用户列表
    UpdtTotalRefreshUidList(pobjNode);

    //real push data——refresh uid
    if(pobjNode->m_udwTotalUidListNum > 0)
    {
        PushDataBasic_RefreshUid(pstSession, &pobjNode->m_audwTotalUidList[0], pobjNode->m_udwTotalUidListNum);
    }
    if (pobjNode->m_udwTotalAidListNum > 0)
    {
        PushDataAid_Refresh(pstSession, &pobjNode->m_audwTotalAidList[0], pobjNode->m_udwTotalAidListNum, &pobjNode->m_audwTotalUidList[0], pobjNode->m_udwTotalUidListNum);
    }

    //real push data——push aid action
    if (pobjSourceAlData->m_udwAllianceId != 0)
    {
        Json::Value jsonSAlData;
        GetPushData_AlAction(pobjSourceAlData->m_vecActionList, pobjSourceAlData->m_udwAllianceId, jsonSAlData);
        PushDataBasic_TargetAid(pstSession, jsonSAlData, pobjSourceAlData->m_udwAllianceId, 
            &pobjSourceAlData->m_audwUidList[0], pobjSourceAlData->m_udwUidListNum);
    }

    //real push data——push aid action
    if (pobjTargetAlData->m_udwAllianceId != 0)
    {
        Json::Value jsonTAlData;
        GetPushData_AlAction(pobjTargetAlData->m_vecActionList, pobjTargetAlData->m_udwAllianceId, jsonTAlData);
        PushDataBasic_TargetAid(pstSession, jsonTAlData, pobjTargetAlData->m_udwAllianceId, 
            &pobjTargetAlData->m_audwUidList[0], pobjTargetAlData->m_udwUidListNum);
    }

    //real push data——push map(action & wild)
    Json::Value jsonMapData;
    GetPushData_BlockMap(pobjMapData->m_vecActionList, pobjMapData->m_vecWild, jsonMapData, pstSession->m_stSourceUser.m_tbPlayer.m_nSid);
    PushDataBasic_Map(pstSession, jsonMapData, pstSession->m_stSourceUser.m_tbPlayer.m_nSid);

    if (pstSession->bKingChanged == TRUE)
    {
        Json::Value jTitleData = Json::Value(Json::objectValue);
        CCommJson::GenTitleInfo(&pstSession->m_stTitleInfoList, &pstSession->m_tbThrone, jTitleData["svr_title_new"]);
        PushDataBasic_AllSvr(pstSession, jTitleData, pstSession->m_udwReqSvrId, &pobjNode->m_audwTotalUidList[0], pobjNode->m_udwTotalUidListNum);
    }

    TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("[wavetest---march]: push_data end [seq=%u]---------------", pstSession->m_udwSeqNo));

    return 0;
}

TINT32 CAuPushData::InitRawAlInfo(SUserInfo *pstUser, CPushAlData *pobjPushData)
{
    pobjPushData->m_udwUid = pstUser->m_tbPlayer.m_nUid;
    pobjPushData->m_udwAllianceId = CPlayerBase::GetAllianceId(&pstUser->m_tbPlayer);
    if(pobjPushData->m_udwUid == 0)
    {
        return 0;
    }

    TUINT8 ucFlag = EN_TABLE_UPDT_FLAG__UNCHANGE;
    map<TINT64, SPushActionNode>::iterator it;
    for(TINT32 idx = 0; idx < pstUser->m_udwSelfAlActionNum; idx++)
    {
        TbAlliance_action *ptbAlAction = &pstUser->m_atbSelfAlAction[idx];
        if(ptbAlAction->m_nSal > 0 && ptbAlAction->m_nSal == pobjPushData->m_udwAllianceId)
        {
            it = pobjPushData->m_mapAlActionRaw.find(ptbAlAction->m_nId);
            if(it == pobjPushData->m_mapAlActionRaw.end())
            {
                pobjPushData->m_mapAlActionRaw.insert(make_pair(ptbAlAction->m_nId, SPushActionNode(ptbAlAction, EN_AID_ACTION, ucFlag, TRUE)));
            }
        }
    }

    for(TINT32 idx = 0; idx < pstUser->m_udwMarchNum; idx++)
    {
        TbMarch_action *ptbMarch = &pstUser->m_atbMarch[idx];

        //过滤不需要push的action
        if(ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__THRONE_TIMER
            || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__PRISON_TIMER
            || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__NOTI_TIMER
            || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__IDOL_PERIOD
            || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__THRONE_PERIOD
            || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__TAX)
        {
            continue;
        }

        if((ptbMarch->m_nSal > 0 && ptbMarch->m_nSal == pobjPushData->m_udwAllianceId)
            || ptbMarch->m_nSuid == pstUser->m_tbPlayer.m_nUid)
        {
            it = pobjPushData->m_mapAlActionRaw.find(ptbMarch->m_nId);
            if(it == pobjPushData->m_mapAlActionRaw.end())
            {
                pobjPushData->m_mapAlActionRaw.insert(make_pair(ptbMarch->m_nId, SPushActionNode(ptbMarch, EN_MARCH_ACTION, ucFlag, TRUE)));
            }
        }
    }

    for(TINT32 idx = 0; idx < pstUser->m_udwPassiveMarchNum; idx++)
    {
        TbMarch_action *ptbMarch = &pstUser->m_atbPassiveMarch[idx];
        if(ptbMarch->m_nTal != 0)
        {
            it = pobjPushData->m_mapAlActionRaw.find(ptbMarch->m_nId);
            if(it == pobjPushData->m_mapAlActionRaw.end())
            {
                pobjPushData->m_mapAlActionRaw.insert(make_pair(ptbMarch->m_nId, SPushActionNode(ptbMarch, EN_MARCH_ACTION, ucFlag, FALSE)));
            }
        }
    }

    TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("[wavetest] InitRawAlActionList uid=%ld alid=%u, alaction_num=%u", 
        pstUser->m_tbPlayer.m_nUid, pobjPushData->m_udwAllianceId, pobjPushData->m_mapAlActionRaw.size()));

    return 0;
}

/*
TINT32 CAuPushData::ComputeActionInfo( SUserInfo *pstUser, CPushAlData *pobjPushDataAl, CPushMapData *pobjPushDataMap )
{
    TUINT8 ucFlag = EN_TABLE_UPDT_FLAG__UNCHANGE;
    TUINT32 udwAlId = pobjPushDataAl->m_udwAllianceId;
    map<TINT64, SPushActionNode>::iterator it;

    AddPushData_AlUid(pobjPushDataAl, pstUser->m_tbPlayer.m_nUid);

    for(TINT32 idx = 0; idx < pstUser->m_udwSelfAlActionNum; idx++)
    {
        TbAlliance_action *ptbAlAction = &pstUser->m_atbSelfAlAction[idx];
        TUINT32 udwSuid = 0;
        if(ptbAlAction->m_nSal > 0 && ptbAlAction->m_nSal == udwAlId)
        {
            it = pobjPushDataAl->m_mapAlActionRaw.find(ptbAlAction->m_nId);
            if(it != pobjPushDataAl->m_mapAlActionRaw.end())
            {
                udwSuid = it->second.m_udwSUid;
                pobjPushDataAl->m_mapAlActionRaw.erase(it);
            }

            if(EN_TABLE_UPDT_FLAG__UNCHANGE != pstUser->m_aucSelfAlActionFlag[idx])
            {
                if(pobjPushDataAl->m_setActionId.count(ptbAlAction->m_nId) == 0)
                {
                    pobjPushDataAl->m_vecActionList.push_back(SPushActionNode(ptbAlAction, EN_AID_ACTION, pstUser->m_aucSelfAlActionFlag[idx], TRUE));
                    pobjPushDataAl->m_setActionId.insert(ptbAlAction->m_nId);
                }
            }
        }

        if(EN_TABLE_UPDT_FLAG__UNCHANGE != pstUser->m_aucSelfAlActionFlag[idx])
        {
            AddPushData_AlUid(pobjPushDataAl, udwSuid);
            AddPushData_AlUid(pobjPushDataAl, ptbAlAction->m_nSuid);
        }
    }

    for(TINT32 idx = 0; idx < pstUser->m_udwMarchNum; idx++)
    {
        TbMarch_action *ptbMarch = &pstUser->m_atbMarch[idx];

        //过滤不需要push的action
        if(ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__THRONE_TIMER
            || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__PRISON_TIMER
            || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__NOTI_TIMER
            || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__IDOL_PERIOD
            || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__THRONE_PERIOD)
        {
            continue;
        }

        TUINT32 udwSuid = 0;
        TUINT32 udwTuid = 0;
        if(ptbMarch->m_nSal > 0 && ptbMarch->m_nSal == udwAlId)
        {
            it = pobjPushDataAl->m_mapAlActionRaw.find(ptbMarch->m_nId);
            if(it != pobjPushDataAl->m_mapAlActionRaw.end())
            {
                udwSuid = it->second.m_udwSUid;
                udwTuid = it->second.m_udwTUid;
                pobjPushDataAl->m_mapAlActionRaw.erase(it);
            }

            if(EN_TABLE_UPDT_FLAG__UNCHANGE != pstUser->m_aucMarchFlag[idx])
            {
                if(pobjPushDataAl->m_setActionId.count(ptbMarch->m_nId) == 0)
                {
                    pobjPushDataAl->m_vecActionList.push_back(SPushActionNode(ptbMarch, EN_MARCH_ACTION, pstUser->m_aucMarchFlag[idx], TRUE));
                    pobjPushDataAl->m_setActionId.insert(ptbMarch->m_nId);
                }
            }
        }

        if(EN_TABLE_UPDT_FLAG__UNCHANGE != pstUser->m_aucMarchFlag[idx])
        {
            AddPushData_AlUid(pobjPushDataAl, udwSuid);
            AddPushData_AlUid(pobjPushDataAl, udwTuid);
            AddPushData_AlUid(pobjPushDataAl, ptbMarch->m_nSuid);
            AddPushData_AlUid(pobjPushDataAl, ptbMarch->m_nTuid);

            //add action to map
            AddPushData_MapAction(pobjPushDataMap, ptbMarch, pstUser->m_aucMarchFlag[idx]);            
        }
    }

    for(TINT32 idx = 0; idx < pstUser->m_udwPassiveMarchNum; idx++)
    {
        TbMarch_action *ptbMarch = &pstUser->m_atbPassiveMarch[idx];
        TUINT32 udwSuid = 0;
        TUINT32 udwTuid = 0;

        if(ptbMarch->m_nTal > 0 && ptbMarch->m_nTal == udwAlId)
        {
            it = pobjPushDataAl->m_mapAlActionRaw.find(ptbMarch->m_nId);
            if(it != pobjPushDataAl->m_mapAlActionRaw.end())
            {
                udwSuid = it->second.m_udwSUid;
                udwTuid = it->second.m_udwTUid;
                pobjPushDataAl->m_mapAlActionRaw.erase(it);
            }

            if(EN_TABLE_UPDT_FLAG__UNCHANGE != pstUser->m_aucPassiveMarchFlag[idx])
            {
                if(pobjPushDataAl->m_setActionId.count(ptbMarch->m_nId) == 0)
                {
                    pobjPushDataAl->m_vecActionList.push_back(SPushActionNode(ptbMarch, EN_MARCH_ACTION, pstUser->m_aucPassiveMarchFlag[idx], FALSE));
                    pobjPushDataAl->m_setActionId.insert(ptbMarch->m_nId);
                }
            }
        }

        if(EN_TABLE_UPDT_FLAG__UNCHANGE != pstUser->m_aucPassiveMarchFlag[idx])
        {
            AddPushData_AlUid(pobjPushDataAl, udwSuid);
            AddPushData_AlUid(pobjPushDataAl, udwTuid);
            AddPushData_AlUid(pobjPushDataAl, ptbMarch->m_nSuid);
            AddPushData_AlUid(pobjPushDataAl, ptbMarch->m_nTuid);

            //add action to map
            AddPushData_MapAction(pobjPushDataMap, ptbMarch, pstUser->m_aucPassiveMarchFlag[idx]);
        }
    }

    // 因为数据更改已经不存在于Al中的action需要删除
    for(it = pobjPushDataAl->m_mapAlActionRaw.begin(); it != pobjPushDataAl->m_mapAlActionRaw.end(); it++)
    {
        if(pobjPushDataAl->m_setActionId.count(it->first) == 0)
        {
            pobjPushDataAl->m_vecActionList.push_back(SPushActionNode(it->second.m_ptbAction, it->second.m_dwType, EN_TABLE_UPDT_FLAG__DEL, it->second.m_isActive));
            pobjPushDataAl->m_setActionId.insert(it->first);

            AddPushData_AlUid(pobjPushDataAl, it->second.m_udwSUid);
            AddPushData_AlUid(pobjPushDataAl, it->second.m_udwTUid);
            switch(it->second.m_dwType)
            {
            case EN_UID_ACTION:
                AddPushData_AlUid(pobjPushDataAl, ((TbAction*)it->second.m_ptbAction)->m_nSuid);
                break;
            case EN_AID_ACTION:
                AddPushData_AlUid(pobjPushDataAl, ((TbAlliance_action*)it->second.m_ptbAction)->m_nSuid);
                break;
            case EN_MARCH_ACTION:
                AddPushData_AlUid(pobjPushDataAl, ((TbMarch_action*)it->second.m_ptbAction)->m_nSuid);
                AddPushData_AlUid(pobjPushDataAl, ((TbMarch_action*)it->second.m_ptbAction)->m_nTuid);
                break;
            default:
                break; 
            }
        }
    }

    TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("[wavetest] PushData_ComputeResult uid=%ld alid=%u, alaction_num=%u, uid_num=%u, mapacion_num=%u", 
        pstUser->m_tbPlayer.m_nUid, pobjPushDataAl->m_udwAllianceId, pobjPushDataAl->m_vecActionList.size(), pobjPushDataAl->m_udwUidListNum, pobjPushDataMap->m_vecActionList.size()));

    return 0;
}
*/

TINT32 CAuPushData::DelUnExistAction(CPushAlData *pobjPushDataAl)
{
    if(pobjPushDataAl->m_udwAllianceId == 0)
    {
        return 0;
    }

    // 因为数据更改已经不存在于Al中的action需要删除
    for(map<TINT64, SPushActionNode>::iterator it = pobjPushDataAl->m_mapAlActionRaw.begin(); it != pobjPushDataAl->m_mapAlActionRaw.end(); it++)
    {
        if(pobjPushDataAl->m_setActionId.count(it->first) == 0)
        {
            TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("AuPushData_DelUnExistAction: target=%u action_id=%ld, suid=%u sal=%ld,tuid=%u,tal=%ld", 
                pobjPushDataAl->m_udwAllianceId, it->first, it->second.m_udwSUid, it->second.m_nSal, it->second.m_udwTUid, it->second.m_nTal));

            pobjPushDataAl->m_vecActionList.push_back(SPushActionNode(it->second.m_ptbAction, it->second.m_dwType, EN_TABLE_UPDT_FLAG__DEL, it->second.m_isActive));
            pobjPushDataAl->m_setActionId.insert(it->first);

            AddPushData_AlUid(pobjPushDataAl, it->second.m_udwSUid);
            AddPushData_AlUid(pobjPushDataAl, it->second.m_udwTUid);
            switch(it->second.m_dwType)
            {
            case EN_UID_ACTION:
                AddPushData_AlUid(pobjPushDataAl, ((TbAction*)it->second.m_ptbAction)->m_nSuid);
                break;
            case EN_AID_ACTION:
                AddPushData_AlUid(pobjPushDataAl, ((TbAlliance_action*)it->second.m_ptbAction)->m_nSuid);
                break;
            case EN_MARCH_ACTION:
                AddPushData_AlUid(pobjPushDataAl, ((TbMarch_action*)it->second.m_ptbAction)->m_nSuid);
                AddPushData_AlUid(pobjPushDataAl, ((TbMarch_action*)it->second.m_ptbAction)->m_nTuid);
                break;
            default:
                break; 
            }
        }
    }

    return 0;
}

TINT32 CAuPushData::UpdtTotalRefreshUidList(CAuPushDataNode *pstNode)
{
    pstNode->m_udwTotalUidListNum = 0;

    for(int idx = 0; idx < pstNode->m_objPushDataSourceAl.m_udwUidListNum; idx++)
    {
        pstNode->m_audwTotalUidList[pstNode->m_udwTotalUidListNum++] = pstNode->m_objPushDataSourceAl.m_audwUidList[idx];
    }

    for(int idx = 0; idx < pstNode->m_objPushDataTargetAl.m_udwUidListNum; idx++)
    {
        if(pstNode->m_objPushDataSourceAl.m_setUid.count(pstNode->m_objPushDataTargetAl.m_audwUidList[idx]) == 0)
        {
            pstNode->m_audwTotalUidList[pstNode->m_udwTotalUidListNum++] = pstNode->m_objPushDataTargetAl.m_audwUidList[idx];
        }
    }

    TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("[wavetest] UpdtTotalRefreshUidList suid_num=%u, tuid_num=%u, totaluid_num=%u", 
        pstNode->m_objPushDataSourceAl.m_udwUidListNum, pstNode->m_objPushDataTargetAl.m_udwUidListNum, pstNode->m_udwTotalUidListNum));

    return 0;
}

TINT32 CAuPushData::AddPushData_Action(SSession *pstSession, CPushAlData *pobjPushDataAl, CPushMapData *pobjPushDataMap, void *ptbAction, TINT32 dwType, TUINT8 ucFlag, TBOOL isActive)
{
    TUINT32 udwSuid = 0;
    TUINT32 udwTuid = 0;
    map<TINT64, SPushActionNode>::iterator it;
    TbAlliance_action *ptbAlAction = NULL;
    TbMarch_action *ptbMarch = NULL;
    TbAction *ptbUidAction = NULL;

    SPushActionNode *pstNode = NULL;

    if(EN_TABLE_UPDT_FLAG__UNCHANGE == ucFlag)
    {
        return 0;
    }

    if(pobjPushDataAl->m_udwUid == 0)
    {
        return 0;
    }

    switch (dwType)
    {
    case EN_UID_ACTION:
        //不需要uid的action
        break;
    case EN_AID_ACTION:
        ptbAlAction = (TbAlliance_action *)ptbAction;
        it = pobjPushDataAl->m_mapAlActionRaw.find(ptbAlAction->m_nId);
        if(it != pobjPushDataAl->m_mapAlActionRaw.end())
        {
            pstNode = &it->second;
            TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("AddPushData_Action[alliance--rawinfo]:target=%u action_id=%ld, suid=%u sal=%ld [seq=%u]", 
                pobjPushDataAl->m_udwAllianceId, ptbAlAction->m_nId, pstNode->m_udwSUid, pstNode->m_nSal, pstSession->m_udwSeqNo));
        }

        if (ptbAlAction->m_nSal > 0 && ptbAlAction->m_nSal == pobjPushDataAl->m_udwAllianceId)
        {
            if(pobjPushDataAl->m_setActionId.count(ptbAlAction->m_nId) == 0)
            {
                TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("AddPushData_Action[alliance---update]:target=%u action_id=%ld, suid=%ld sal=%ld, [main=%ld,sec=%ld,status=%d] [seq=%u]", 
                    pobjPushDataAl->m_udwAllianceId, ptbAlAction->m_nId, ptbAlAction->m_nSuid, ptbAlAction->m_nSal, 
                    ptbAlAction->m_nMclass, ptbAlAction->m_nSclass, ptbAlAction->m_nStatus, pstSession->m_udwSeqNo));

                pobjPushDataAl->m_vecActionList.push_back(SPushActionNode(ptbAlAction, EN_AID_ACTION, ucFlag, isActive));
                pobjPushDataAl->m_setActionId.insert(ptbAlAction->m_nId);
            }
        }
        else
        {    
            if(pstNode)
            {
                if(it->second.m_nSal > 0)
                {
                    if(it->second.m_nSal == pobjPushDataAl->m_udwAllianceId)
                    {
                        if(pobjPushDataAl->m_setActionId.count(ptbAlAction->m_nId) == 0)
                        {
                            TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("AddPushData_Action[alliance--delete]:target=%u action_id=%ld, suid=%ld sal=%ld, [main=%ld,sec=%ld,status=%d] [seq=%u]", 
                                pobjPushDataAl->m_udwAllianceId, ptbAlAction->m_nId, ptbAlAction->m_nSuid, ptbAlAction->m_nSal, 
                                ptbAlAction->m_nMclass, ptbAlAction->m_nSclass, ptbAlAction->m_nStatus, pstSession->m_udwSeqNo));

                            pobjPushDataAl->m_vecActionList.push_back(SPushActionNode(ptbAlAction, EN_AID_ACTION, EN_TABLE_UPDT_FLAG__DEL, isActive));
                            pobjPushDataAl->m_setActionId.insert(ptbAlAction->m_nId);
                        }                        
                    }
                    else
                    {
                        TSE_LOG_ERROR(CGlobalServ::m_poServLog, ("AddPushData_Action[alliance--error]:target=%u action_id=%ld, suid=%ld sal=%ld, [main=%ld,sec=%ld,status=%d] [seq=%u]", 
                            pobjPushDataAl->m_udwAllianceId, ptbAlAction->m_nId, ptbAlAction->m_nSuid, ptbAlAction->m_nSal, 
                            ptbAlAction->m_nMclass, ptbAlAction->m_nSclass, ptbAlAction->m_nStatus, pstSession->m_udwSeqNo));
                    }
                }
            }
        }

        if(pstNode)
        {
            AddPushData_AlUid(pobjPushDataAl, pstNode->m_udwSUid);
        }        
        AddPushData_AlUid(pobjPushDataAl, ptbAlAction->m_nSuid);
        break;
    case EN_MARCH_ACTION:
        ptbMarch = (TbMarch_action *)ptbAction;
        //过滤不需要push的action
        if (ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__THRONE_TIMER
            || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__PRISON_TIMER
            || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__NOTI_TIMER
            || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__IDOL_PERIOD
            || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__THRONE_PERIOD
            || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__TAX)
        {
            break;
        }

        it = pobjPushDataAl->m_mapAlActionRaw.find(ptbMarch->m_nId);
        if(it != pobjPushDataAl->m_mapAlActionRaw.end())
        {
            pstNode = &it->second;
            TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("AddPushData_Action[march--rawinfo]: target=%u action_id=%ld, suid=%u sal=%ld,tuid=%u,tal=%ld [seq=%u]", 
                    pobjPushDataAl->m_udwAllianceId, ptbMarch->m_nId, pstNode->m_udwSUid, pstNode->m_nSal, pstNode->m_udwTUid, pstNode->m_nTal, pstSession->m_udwSeqNo));
        }

        // 处理sal的情况
        if(ptbMarch->m_nSal > 0 && ptbMarch->m_nSal == pobjPushDataAl->m_udwAllianceId)
        {
            if(pobjPushDataAl->m_setActionId.count(ptbMarch->m_nId) == 0)
            {
                TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("AddPushData_Action[march--update]:target=%u action_id=%ld, suid=%ld sal=%ld,tuid=%ld,tal=%ld [main=%ld,sec=%ld,status=%d] [seq=%u]", 
                    pobjPushDataAl->m_udwAllianceId, ptbMarch->m_nId, ptbMarch->m_nSuid, ptbMarch->m_nSal, ptbMarch->m_nTuid, ptbMarch->m_nTal,
                    ptbMarch->m_nMclass, ptbMarch->m_nSclass, ptbMarch->m_nStatus, pstSession->m_udwSeqNo));

                pobjPushDataAl->m_vecActionList.push_back(SPushActionNode(ptbMarch, EN_MARCH_ACTION, ucFlag, isActive));
                pobjPushDataAl->m_setActionId.insert(ptbMarch->m_nId);
            }
        }
        
        //处理sal变化的情况
        if(pstNode && pstNode->m_nSal != ptbMarch->m_nSal)
        {
            if(pstNode->m_nSal > 0)
            {
                if(pstNode->m_nSal == pobjPushDataAl->m_udwAllianceId)
                {
                    if(pobjPushDataAl->m_setActionId.count(ptbMarch->m_nId) == 0)
                    {
                        TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("AddPushData_Action[march--delete--sal]:sal change!!!. target=%u action_id=%ld, suid=%ld sal=%ld,tuid=%ld,tal=%ld [main=%ld,sec=%ld,status=%d] [seq=%u]", 
                            pobjPushDataAl->m_udwAllianceId, ptbMarch->m_nId, ptbMarch->m_nSuid, ptbMarch->m_nSal, ptbMarch->m_nTuid, ptbMarch->m_nTal,
                            ptbMarch->m_nMclass, ptbMarch->m_nSclass, ptbMarch->m_nStatus, pstSession->m_udwSeqNo));

                        pobjPushDataAl->m_vecActionList.push_back(SPushActionNode(ptbMarch, EN_MARCH_ACTION, EN_TABLE_UPDT_FLAG__DEL, isActive));
                        pobjPushDataAl->m_setActionId.insert(ptbMarch->m_nId);
                    }
                }
                else
                {
                    TSE_LOG_ERROR(CGlobalServ::m_poServLog, ("AddPushData_Action[march--error--sal]:sal change!!!. target=%u action_id=%ld, suid=%ld sal=%ld,tuid=%ld,tal=%ld [main=%ld,sec=%ld,status=%d] [seq=%u]", 
                        pobjPushDataAl->m_udwAllianceId, ptbMarch->m_nId, ptbMarch->m_nSuid, ptbMarch->m_nSal, ptbMarch->m_nTuid, ptbMarch->m_nTal,
                        ptbMarch->m_nMclass, ptbMarch->m_nSclass, ptbMarch->m_nStatus, pstSession->m_udwSeqNo));
                }
            }
        }

        //处理tal的情况
        if(ptbMarch->m_nTal > 0)
        {
            if(ptbMarch->m_nTal == pobjPushDataAl->m_udwAllianceId)
            {
                if(pobjPushDataAl->m_setActionId.count(ptbMarch->m_nId) == 0)
                {
                    TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("AddPushData_Action[march--update--tal]:target=%u action_id=%ld, suid=%ld sal=%ld,tuid=%ld,tal=%ld [main=%ld,sec=%ld,status=%d] [seq=%u]", 
                        pobjPushDataAl->m_udwAllianceId, ptbMarch->m_nId, ptbMarch->m_nSuid, ptbMarch->m_nSal, ptbMarch->m_nTuid, ptbMarch->m_nTal,
                        ptbMarch->m_nMclass, ptbMarch->m_nSclass, ptbMarch->m_nStatus, pstSession->m_udwSeqNo));

                    pobjPushDataAl->m_vecActionList.push_back(SPushActionNode(ptbMarch, EN_MARCH_ACTION, ucFlag, isActive));
                    pobjPushDataAl->m_setActionId.insert(ptbMarch->m_nId);
                }
            }
            else if (ptbMarch->m_nTal == CActionBase::GenThroneTargetId(pstSession->m_tbThrone.m_nSid, pstSession->m_tbThrone.m_nPos))
            {
                if (pstSession->m_tbThrone.m_nAlid > 0)
                {
                    TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("AddPushData_Action[march--update--tal_throne]:target=%ld action_id=%ld, suid=%ld sal=%ld,tuid=%ld,tal=%ld [main=%ld,sec=%ld,status=%d] [seq=%u]",
                        pstSession->m_tbThrone.m_nAlid, ptbMarch->m_nId, ptbMarch->m_nSuid, ptbMarch->m_nSal, ptbMarch->m_nTuid, ptbMarch->m_nTal,
                        ptbMarch->m_nMclass, ptbMarch->m_nSclass, ptbMarch->m_nStatus, pstSession->m_udwSeqNo));
                    PushDataAid_March(pstSession, ptbMarch, EN_TABLE_UPDT_FLAG__CHANGE, "svr_al_p_action_list", pstSession->m_tbThrone.m_nAlid);
                }
                if (pstSession->bKingChanged && pstSession->dwRawThroneAlid)
                {
                    TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("AddPushData_Action[march--delete--tal_throne]:target=%d action_id=%ld, suid=%ld sal=%ld,tuid=%ld,tal=%ld [main=%ld,sec=%ld,status=%d] [seq=%u]",
                        pstSession->dwRawThroneAlid, ptbMarch->m_nId, ptbMarch->m_nSuid, ptbMarch->m_nSal, ptbMarch->m_nTuid, ptbMarch->m_nTal,
                        ptbMarch->m_nMclass, ptbMarch->m_nSclass, ptbMarch->m_nStatus, pstSession->m_udwSeqNo));
                    PushDataAid_March(pstSession, ptbMarch, EN_TABLE_UPDT_FLAG__DEL, "svr_al_p_action_list", pstSession->dwRawThroneAlid);
                }
            }
            else
            {
                TSE_LOG_ERROR(CGlobalServ::m_poServLog, ("AddPushData_Action[march--error--tal]:target=%u action_id=%ld, suid=%ld sal=%ld,tuid=%ld,tal=%ld [main=%ld,sec=%ld,status=%d] [seq=%u]", 
                    pobjPushDataAl->m_udwAllianceId, ptbMarch->m_nId, ptbMarch->m_nSuid, ptbMarch->m_nSal, ptbMarch->m_nTuid, ptbMarch->m_nTal,
                    ptbMarch->m_nMclass, ptbMarch->m_nSclass, ptbMarch->m_nStatus, pstSession->m_udwSeqNo));
            }
        }
        //处理tal变化的情况
        if(pstNode && pstNode->m_nTal != ptbMarch->m_nTal)
        {
            if(pstNode->m_nTal > 0)
            {
                if(pstNode->m_nTal == pobjPushDataAl->m_udwAllianceId)
                {
                    if(pobjPushDataAl->m_setActionId.count(ptbMarch->m_nId) == 0)
                    {
                        TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("AddPushData_Action[march--delete--rawtal_is_us]:. target=%u action_id=%ld, suid=%ld sal=%ld,tuid=%ld,tal=%ld [main=%ld,sec=%ld,status=%d] [seq=%u]", 
                            pobjPushDataAl->m_udwAllianceId, ptbMarch->m_nId, ptbMarch->m_nSuid, ptbMarch->m_nSal, ptbMarch->m_nTuid, ptbMarch->m_nTal,
                            ptbMarch->m_nMclass, ptbMarch->m_nSclass, ptbMarch->m_nStatus, pstSession->m_udwSeqNo));

                        pobjPushDataAl->m_vecActionList.push_back(SPushActionNode(ptbMarch, EN_MARCH_ACTION, EN_TABLE_UPDT_FLAG__DEL, isActive));
                        pobjPushDataAl->m_setActionId.insert(ptbMarch->m_nId);
                    }
                }
                else if (pstNode->m_nTal == CActionBase::GenThroneTargetId(pstSession->m_tbThrone.m_nSid, pstSession->m_tbThrone.m_nPos))
                {
                    if (!pstSession->bKingChanged && pstSession->m_tbThrone.m_nAlid > 0)
                    {
                        TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("AddPushData_Action[march--delete--tal_throne]:target=%ld action_id=%ld, suid=%ld sal=%ld,tuid=%ld,tal=%ld [main=%ld,sec=%ld,status=%d] [seq=%u]",
                            pstSession->m_tbThrone.m_nAlid, ptbMarch->m_nId, ptbMarch->m_nSuid, ptbMarch->m_nSal, ptbMarch->m_nTuid, ptbMarch->m_nTal,
                            ptbMarch->m_nMclass, ptbMarch->m_nSclass, ptbMarch->m_nStatus, pstSession->m_udwSeqNo));
                        PushDataAid_March(pstSession, ptbMarch, EN_TABLE_UPDT_FLAG__DEL, "svr_al_p_action_list", pstSession->m_tbThrone.m_nAlid);
                    }
                    else if (pstSession->bKingChanged && pstSession->dwRawThroneAlid)
                    {
                        TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("AddPushData_Action[march--delete--tal_throne]:target=%d action_id=%ld, suid=%ld sal=%ld,tuid=%ld,tal=%ld [main=%ld,sec=%ld,status=%d] [seq=%u]",
                            pstSession->dwRawThroneAlid, ptbMarch->m_nId, ptbMarch->m_nSuid, ptbMarch->m_nSal, ptbMarch->m_nTuid, ptbMarch->m_nTal,
                            ptbMarch->m_nMclass, ptbMarch->m_nSclass, ptbMarch->m_nStatus, pstSession->m_udwSeqNo));
                        PushDataAid_March(pstSession, ptbMarch, EN_TABLE_UPDT_FLAG__DEL, "svr_al_p_action_list", pstSession->dwRawThroneAlid);
                    }
                }
                else
                {
                    TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("AddPushData_Action[march--delete--rawtal_not_us]:target=%u action_id=%ld, suid=%ld sal=%ld,tuid=%ld,tal=%ld [main=%ld,sec=%ld,status=%d] [seq=%u]", 
                        pobjPushDataAl->m_udwAllianceId, ptbMarch->m_nId, ptbMarch->m_nSuid, ptbMarch->m_nSal, ptbMarch->m_nTuid, ptbMarch->m_nTal,
                        ptbMarch->m_nMclass, ptbMarch->m_nSclass, ptbMarch->m_nStatus, pstSession->m_udwSeqNo));
                    //pobjPushDataAl->m_vecActionList.push_back(SPushActionNode(ptbMarch, EN_MARCH_ACTION, EN_TABLE_UPDT_FLAG__DEL, isActive));
                    //pobjPushDataAl->m_setActionId.insert(ptbMarch->m_nId);
                    PushDataAid_March(pstSession, ptbMarch, EN_TABLE_UPDT_FLAG__DEL, "svr_al_p_action_list", pstNode->m_nTal);

                }
            }
        }

        //if(EN_TABLE_UPDT_FLAG__UNCHANGE != ucFlag)
        {
            if(pstNode)
            {
                AddPushData_AlUid(pobjPushDataAl, pstNode->m_udwSUid);
                AddPushData_AlUid(pobjPushDataAl, pstNode->m_udwTUid);
            }
            AddPushData_AlUid(pobjPushDataAl, ptbMarch->m_nSuid);
            AddPushData_AlUid(pobjPushDataAl, ptbMarch->m_nTuid);

            //add action to map
            AddPushData_MapAction(pobjPushDataMap, ptbMarch, ucFlag);
        }
        break;
    default:
        break;
    }

    return 0;
}
TINT32 CAuPushData::AddPushData_AlUid( CPushAlData *pobjPushDataAl, TUINT32 udwId )
{
    if(udwId && pobjPushDataAl->m_setUid.count(udwId) == 0)
    {
        pobjPushDataAl->m_setUid.insert(udwId);
        pobjPushDataAl->m_audwUidList[pobjPushDataAl->m_udwUidListNum++] = udwId;
    }
    return 0;
}

TINT32 CAuPushData::AddPushData_Aid(SSession *pstSession, TUINT32 udwId)
{
    if (udwId > 0)
    {
        for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_objAuPushDataNode.m_udwTotalAidListNum; udwIdx++)
        {
            if (pstSession->m_objAuPushDataNode.m_audwTotalAidList[udwIdx] == udwId)
            {
                return 0;
            }
        }
        pstSession->m_objAuPushDataNode.m_audwTotalAidList[pstSession->m_objAuPushDataNode.m_udwTotalAidListNum++] == udwId;
    }
    return 0;
}

TINT32 CAuPushData::AddPushData_Wild( CPushMapData *pobjPushDataMap, TbMap *ptbWild )
{
    if(ptbWild->m_nId == 0)
    {
        return 0;
    }

    TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("AddPushData_Wild: push_back_to_map id=%ld, count=%u", 
        ptbWild->m_nId, pobjPushDataMap->m_setWild.count(ptbWild->m_nId)));

    if(pobjPushDataMap->m_setWild.count(ptbWild->m_nId) == 0)
    {
        pobjPushDataMap->m_vecWild.push_back(SPushWildNode(ptbWild));
        pobjPushDataMap->m_setWild.insert(ptbWild->m_nId);
    }

    return 0;
}

TINT32 CAuPushData::AddPushData_MapAction( CPushMapData *pobjPushDataMap, TbMarch_action *ptbMarch, TINT32 dwTableFlag )
{
    TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("AddPushData_MapAction: push_back_to_map id=%ld, flag=%d, count=%u", 
        ptbMarch->m_nId, dwTableFlag, pobjPushDataMap->m_setAction.count(ptbMarch->m_nId)));

    if(pobjPushDataMap->m_setAction.count(ptbMarch->m_nId) == 0)
    {
        pobjPushDataMap->m_vecActionList.push_back(SPushActionNode(ptbMarch, EN_MARCH_ACTION, dwTableFlag));
        pobjPushDataMap->m_setAction.insert(ptbMarch->m_nId);
    }
    return 0;
}

TINT32 CAuPushData::GetPushData_AlAction( vector<SPushActionNode> &vecActionList, TINT64 ddwAid, Json::Value &jsonAlAction )
{
    TINT32 dwActionNum = 0;
    jsonAlAction.clear();
    if(ddwAid == 0)
    {
        return 0;
    }

    SPushActionNode *pstNode = NULL;
    for(TINT32 idx = 0; idx < vecActionList.size(); idx++)
    {
        pstNode = &vecActionList[idx];
        if(pstNode->m_dwType == EN_AID_ACTION)
        {
            CCommJson::GenAlActionInfoForPush((TbAlliance_action*)pstNode->m_ptbAction, jsonAlAction["svr_al_action_list"], pstNode->m_ucFlag);
            dwActionNum++;
        }
        else if(pstNode->m_dwType == EN_MARCH_ACTION)
        {
            TbMarch_action *ptbAction = (TbMarch_action*)pstNode->m_ptbAction;

            if(pstNode->m_ucFlag == EN_TABLE_UPDT_FLAG__DEL)
            {
                CCommJson::GenMarchInfoForPush(ptbAction, jsonAlAction["svr_al_action_list"], pstNode->m_ucFlag);
                CCommJson::GenMarchInfoForPush(ptbAction, jsonAlAction["svr_al_p_action_list"], pstNode->m_ucFlag);
            }
            else
            {
                if(ptbAction->m_nSal == ddwAid)
                {
                    CCommJson::GenMarchInfoForPush(ptbAction, jsonAlAction["svr_al_action_list"], pstNode->m_ucFlag);
                    dwActionNum++;
                }

                if(ptbAction->m_nTal == ddwAid)
                {
                    CCommJson::GenMarchInfoForPush(ptbAction, jsonAlAction["svr_al_p_action_list"], pstNode->m_ucFlag);
                    dwActionNum++;
                }
            }
        }
        else
        {
            continue;
        }
    }
    return dwActionNum;
}

TINT32 CAuPushData::GetPushData_BlockMap( vector<SPushActionNode> &vecActionList, vector<SPushWildNode> &vecWildList, Json::Value &jsonMap, TINT32 dwSid )
{
    jsonMap.clear();

    Json::Value jsonTmp;
    TUINT32 udwSBlockId = 0, udwTBlockId = 0;

    SPushActionNode *pstActionNode = NULL;
    TbMarch_action *ptbAction = NULL;
    for(TINT32 idx = 0; idx < vecActionList.size(); idx++)
    {
        pstActionNode = &vecActionList[idx];
        if(pstActionNode->m_dwType != EN_MARCH_ACTION)
        {
            continue;
        }

        if(IsMarchActionNeedToPushMap(pstActionNode) == FALSE)
        {
            TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("[wavetest-filter] march action: block_map gen json. flag=%d", 
                pstActionNode->m_ucFlag));
            continue;
        }

        jsonTmp.clear();
        ptbAction = (TbMarch_action*)pstActionNode->m_ptbAction;

        TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("[wavetest] march action: block_map gen json id=%ld, flag=%d", 
            ptbAction->m_nId, pstActionNode->m_ucFlag));

        CCommJson::GenMarchInfoForPush(ptbAction, jsonTmp, pstActionNode->m_ucFlag);

        udwSBlockId = CMapBase::GetBlockIdFromPos(ptbAction->m_nScid);
        udwTBlockId = CMapBase::GetBlockIdFromPos(ptbAction->m_nTpos);
        jsonMap[CCommonFunc::NumToString(udwSBlockId)]["svr_map_inc"]["map_action_list"][CCommonFunc::NumToString(ptbAction->m_nId)] = jsonTmp[CCommonFunc::NumToString(ptbAction->m_nId)];
        jsonMap[CCommonFunc::NumToString(udwTBlockId)]["svr_map_inc"]["map_action_list"][CCommonFunc::NumToString(ptbAction->m_nId)] = jsonTmp[CCommonFunc::NumToString(ptbAction->m_nId)];
        jsonMap[CCommonFunc::NumToString(udwSBlockId)]["svr_map_inc"]["svr_id"] = dwSid;
    }

    TbMap *ptbWild = NULL;
    for(TINT32 idx = 0; idx < vecWildList.size(); idx++)
    {
        ptbWild = vecWildList[idx].m_ptbWild;

        if(CMapLogic::IsWildNeedToDelete(ptbWild))
        {
            udwSBlockId = CMapBase::GetBlockIdFromPos(ptbWild->m_nId);
            jsonMap[CCommonFunc::NumToString(udwSBlockId)]["svr_map_inc"]["list"][CCommonFunc::NumToString(ptbWild->m_nId)] = Json::Value(Json::nullValue);
            jsonMap[CCommonFunc::NumToString(udwSBlockId)]["svr_map_inc"]["svr_id"] = dwSid;
        }
        else
        {
            jsonTmp.clear();
            CCommJson::PushData_GenMapJson(ptbWild->m_nSid, ptbWild, jsonTmp);

            udwSBlockId = CMapBase::GetBlockIdFromPos(ptbWild->m_nId);
            jsonMap[CCommonFunc::NumToString(udwSBlockId)]["svr_map_inc"]["list"][CCommonFunc::NumToString(ptbWild->m_nId)] = jsonTmp;
            jsonMap[CCommonFunc::NumToString(udwSBlockId)]["svr_map_inc"]["svr_id"] = dwSid;
        }
    }

    return 0;

}

TBOOL CAuPushData::IsMarchActionNeedToPushMap(SPushActionNode *pstNode)
{
    TbMarch_action* ptbAction = (TbMarch_action*)pstNode->m_ptbAction;
    if(pstNode->m_ucFlag != EN_TABLE_UPDT_FLAG__DEL)
    {
        if(ptbAction->CheckFlag(TbMARCH_ACTION_FIELD_STATUS) == false
            && ptbAction->CheckFlag(TbMARCH_ACTION_FIELD_SCID) == false
            && ptbAction->CheckFlag(TbMARCH_ACTION_FIELD_TPOS) == false) //状态未发生改变的action不用更新
        {
            return FALSE;
        }
    }
    return TRUE;
}

TINT32 CAuPushData::PushDataBasic_Map(SSession *pstSession, Json::Value& rJson, TINT32 dwSid)
{
    CDownMgr *pobjDownMgr = CDownMgr::Instance();
    TUCHAR *pucPackage = NULL;
    TUINT32 udwPackageLen = 0;
    CBaseProtocolPack *pobjPack = pstSession->m_ppPackTool[0];
    TBOOL bRet = FALSE;
    TINT32 dwCostTime = 1;
    TINT32 dwTargetType = EN_PUSH_DATA_TYPE__MAP;
    TINT32 dwListSize = 1;
    LTasksGroup        stTasks;
    CJsonResultGenerator *pJsonGenerator = (CJsonResultGenerator*)pstSession->m_pJsonGenerator;

    TUINT8 ucCompressFlag = 0;

    Json::Value::Members jsonDataKeys = rJson.getMemberNames();
    if(jsonDataKeys.size() == 0)
    {
        return 0;
    }

    // 1. get down node
    if (pstSession->m_bUserLinkerNodeExist == FALSE)
    {
        pstSession->m_pstUserLinkerNode = NULL;
        if (S_OK == pobjDownMgr->zk_GetNode(DOWN_NODE_TYPE__USER_LINKER, &pstSession->m_pstUserLinkerNode))
        {
            pstSession->m_bUserLinkerNodeExist = TRUE;
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("Get UserLinker node succ [seq=%u]", pstSession->m_udwSeqNo));
        }
        else
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("Get UserLinker node fail [seq=%u]", pstSession->m_udwSeqNo));
            return -1;
        }

        if (NULL == pstSession->m_pstUserLinkerNode->m_stDownHandle.handle)
        {
            pstSession->m_bUserLinkerNodeExist = FALSE;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("PushDataBasic_Map: handle [handlevalue=0x%p] [seq=%u]", \
                pstSession->m_pstAwsProxyNode->m_stDownHandle.handle, pstSession->m_udwSeqNo));
            return -2;
        }
    }

    
    
    TUINT32 udwTargetBlockId = 0;

    for (TINT32 idx = 0; idx < jsonDataKeys.size(); idx++)
    {
        udwTargetBlockId = atoi(jsonDataKeys[idx].c_str());
        pobjPack = pstSession->m_ppPackTool[idx];

        // 2. get data
        pJsonGenerator->GenPushData_Pb(pstSession, EN_SERVICE_TYPE__CLIENT__DATA_PUSH_RSP, rJson[jsonDataKeys[idx]]);

        // 3. get package
        pobjPack->ResetContent();
        pobjPack->SetServiceType(EN_SERVICE_TYPE__CLIENT__DATA_PUSH_RSP);
        pobjPack->SetSeq(CGlobalServ::GenerateHsReqSeq());
        pobjPack->SetKey(EN_GLOBAL_KEY__REQ_SEQ, pstSession->m_udwSeqNo);
        pobjPack->SetKey(EN_GLOBAL_KEY__TARGET_TYPE, dwTargetType);
        pobjPack->SetKey(EN_GLOBAL_KEY__TARGET_SID, dwSid);
        dwListSize = 1;
        pobjPack->SetKey(EN_GLOBAL_KEY__TARGET_LIST_SIZE, dwListSize);
        pobjPack->SetKey(EN_GLOBAL_KEY__TARGET_LIST, (TUCHAR*)&udwTargetBlockId, sizeof(udwTargetBlockId));
        pobjPack->SetKey(EN_GLOBAL_KEY__RES_BUF_COMPRESS_FLAG, ucCompressFlag);
        pobjPack->SetKey(EN_GLOBAL_KEY__REQ_BUF, (TUCHAR*)&pstSession->m_pTmpBuf[0], pstSession->m_udwTmpBufLen);

        pobjPack->GetPackage(&pucPackage, &udwPackageLen);

        TSE_LOG_DEBUG(pstSession->m_poServLog, ("PushDataBasic_Map: service=%u, package_len=%u [blockid=%u] [seq=%u]", 
            EN_SERVICE_TYPE__CLIENT__DATA_PUSH_RSP, udwPackageLen, udwTargetBlockId, pstSession->m_udwSeqNo));

        // 4. send request
        
        stTasks.m_Tasks[idx].SetConnSession(pstSession->m_pstUserLinkerNode->m_stDownHandle);
        stTasks.m_Tasks[idx].SetSendData(pucPackage, udwPackageLen);
        stTasks.m_Tasks[idx].SetNeedResponse(0);
    }
    
    stTasks.SetValidTasks(jsonDataKeys.size());
    if (!pstSession->m_poLongConn->SendData(&stTasks))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("PushDataBasic_Map: send request failed [seq=%u]", pstSession->m_udwSeqNo));
        return -3;
    }
    return 0;
}

TINT32 CAuPushData::PushDataBasic_AllSvr(SSession *pstSession, Json::Value& rJson, TUINT32 udwSid, TUINT32 *pudwExceptUidList = NULL, TUINT32 udwExceptListNum = 0)
{
    CDownMgr *pobjDownMgr = CDownMgr::Instance();
    TUCHAR *pucPackage = NULL;
    TUINT32 udwPackageLen = 0;

    TUINT8 ucCompressFlag = 0;
    TINT32 dwTargetType = EN_PUSH_DATA_TYPE__SID;
    TINT32 dwListSize = 1;

    LTasksGroup stTasks;
    CJsonResultGenerator *pJsonGenerator = (CJsonResultGenerator*)pstSession->m_pJsonGenerator;

    // 1. get down node
    if (pstSession->m_bUserLinkerNodeExist == FALSE)
    {
        pstSession->m_pstUserLinkerNode = NULL;
        if (S_OK == pobjDownMgr->zk_GetNode(DOWN_NODE_TYPE__USER_LINKER, &pstSession->m_pstUserLinkerNode))
        {
            pstSession->m_bUserLinkerNodeExist = TRUE;
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("PushDataBasic_AllSvr: Get UserLinker node succ [seq=%u]", pstSession->m_udwSeqNo));
        }
        else
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("PushDataBasic_AllSvr: Get UserLinker node fail [seq=%u]", pstSession->m_udwSeqNo));
            return -1;
        }

        if (NULL == pstSession->m_pstUserLinkerNode->m_stDownHandle.handle)
        {
            pstSession->m_bUserLinkerNodeExist = FALSE;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("PushDataBasic_AllSvr: handle [handlevalue=0x%p] [seq=%u]", 
                pstSession->m_pstUserLinkerNode->m_stDownHandle.handle, pstSession->m_udwSeqNo));
            return -2;
        }
    }

    // 2. get data
    pJsonGenerator->GenPushData_Pb(pstSession, EN_SERVICE_TYPE__CLIENT__DATA_PUSH_RSP, rJson);

    // 3. get package
    CBaseProtocolPack *pobjPack = pstSession->m_ppPackTool[0];
    pobjPack->ResetContent();
    pobjPack->SetServiceType(EN_SERVICE_TYPE__CLIENT__DATA_PUSH_RSP);
    pobjPack->SetSeq(CGlobalServ::GenerateHsReqSeq());

    pobjPack->SetKey(EN_GLOBAL_KEY__REQ_SEQ, pstSession->m_udwSeqNo);
    pobjPack->SetKey(EN_GLOBAL_KEY__TARGET_TYPE, dwTargetType);

    pobjPack->SetKey(EN_GLOBAL_KEY__TARGET_LIST_SIZE, dwListSize);
    pobjPack->SetKey(EN_GLOBAL_KEY__TARGET_LIST, (TUCHAR*)&udwSid, sizeof(udwSid));

    dwListSize = udwExceptListNum;
    pobjPack->SetKey(EN_GLOBAL_KEY__EXCEPT_LIST_SIZE, dwListSize);
    if (dwListSize > 0)
    {
        dwTargetType = EN_PUSH_DATA_TYPE__UID;
        pobjPack->SetKey(EN_GLOBAL_KEY__EXCEPT_TYPE, dwTargetType);
        pobjPack->SetKey(EN_GLOBAL_KEY__EXCEPT_LIST, (TUCHAR*)pudwExceptUidList, sizeof(TUINT32)*dwListSize);
    }

    pobjPack->SetKey(EN_GLOBAL_KEY__RES_BUF_COMPRESS_FLAG, ucCompressFlag);
    pobjPack->SetKey(EN_GLOBAL_KEY__REQ_BUF, (TUCHAR*)&pstSession->m_pTmpBuf[0], pstSession->m_udwTmpBufLen);

    pobjPack->GetPackage(&pucPackage, &udwPackageLen);

    TSE_LOG_DEBUG(pstSession->m_poServLog, ("PushDataBasic_AllSvr: service=%u, package_len=%u [seq=%u]",
        EN_SERVICE_TYPE__CLIENT__DATA_PUSH_RSP, udwPackageLen, pstSession->m_udwSeqNo));

    // 4. send request
    stTasks.m_Tasks[0].SetConnSession(pstSession->m_pstUserLinkerNode->m_stDownHandle);
    stTasks.m_Tasks[0].SetSendData(pucPackage, udwPackageLen);
    stTasks.m_Tasks[0].SetNeedResponse(0);

    stTasks.SetValidTasks(1);
    if (!pstSession->m_poLongConn->SendData(&stTasks))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("PushDataBasic_AllSvr: send request failed [seq=%u]", pstSession->m_udwSeqNo));
        return -3;
    }
    return 0;
}

TINT32 CAuPushData::PushDataBasic_RefreshUid(SSession *pstSession, TUINT32 *pudwUidList, TUINT32 udwUidListNum)
{
    CDownMgr *pobjDownMgr = CDownMgr::Instance();
    TUCHAR *pucPackage = NULL;
    TUINT32 udwPackageLen = 0;
    CBaseProtocolPack *pobjPack = pstSession->m_ppPackTool[0];
    TBOOL bRet = FALSE;
    TINT32 dwCostTime = 1;
    TINT32 dwTargetType = EN_PUSH_DATA_TYPE__UID;
    TINT32 dwListSize = 1;
    TUINT8 ucCompressFlag = 0;
    CJsonResultGenerator *pJsonGenerator = (CJsonResultGenerator*)pstSession->m_pJsonGenerator;

    if(udwUidListNum == 0)
    {
        return 0;
    }

    // 1. get down node
    if(pstSession->m_bUserLinkerNodeExist == FALSE)
    {
        pstSession->m_pstUserLinkerNode = NULL;
        if (S_OK == pobjDownMgr->zk_GetNode(DOWN_NODE_TYPE__USER_LINKER, &pstSession->m_pstUserLinkerNode))
        {
            pstSession->m_bUserLinkerNodeExist = TRUE;
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("Get UserLinker node succ [seq=%u]", pstSession->m_udwSeqNo));
        }
        else
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("Get UserLinker node fail [seq=%u]", pstSession->m_udwSeqNo));
            return -1;
        }

        if(NULL == pstSession->m_pstUserLinkerNode->m_stDownHandle.handle)
        {
            pstSession->m_bUserLinkerNodeExist = FALSE;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("PushDataBasic_RefreshUid:handle [handlevalue=0x%p] [seq=%u]", \
                pstSession->m_pstAwsProxyNode->m_stDownHandle.handle, pstSession->m_udwSeqNo));
            return -2;
        }
    }

    // 2. get data
    Json::Value rJson(Json::objectValue);
    pJsonGenerator->GenPushData_Pb(pstSession, EN_SERVICE_TYPE__CLIENT__INC_REFRESH_RSP, rJson);

    // 3. get package
    pobjPack->ResetContent();
    pobjPack->SetServiceType(EN_SERVICE_TYPE__CLIENT__INC_REFRESH_RSP);
    pobjPack->SetSeq(CGlobalServ::GenerateHsReqSeq());
    pobjPack->SetKey(EN_GLOBAL_KEY__REQ_SEQ, pstSession->m_udwSeqNo);
    pobjPack->SetKey(EN_GLOBAL_KEY__TARGET_TYPE, dwTargetType);
    pobjPack->SetKey(EN_GLOBAL_KEY__TARGET_LIST_SIZE, udwUidListNum);
    pobjPack->SetKey(EN_GLOBAL_KEY__TARGET_LIST, (TUCHAR*)pudwUidList, sizeof(TUINT32)*udwUidListNum);
    dwListSize = 0;
    pobjPack->SetKey(EN_GLOBAL_KEY__EXCEPT_LIST_SIZE, dwListSize);
    pobjPack->SetKey(EN_GLOBAL_KEY__RES_BUF_COMPRESS_FLAG, ucCompressFlag);
    pobjPack->SetKey(EN_GLOBAL_KEY__REQ_BUF, (TUCHAR*)&pstSession->m_pTmpBuf[0], pstSession->m_udwTmpBufLen);

    pobjPack->GetPackage(&pucPackage, &udwPackageLen);

    TSE_LOG_DEBUG(pstSession->m_poServLog, ("PushDataBasic_RefreshUid: service=%u, package_len=%u, target_uid=%u num=%u [seq=%u]", 
        EN_SERVICE_TYPE__CLIENT__INC_REFRESH_RSP, udwPackageLen, pudwUidList[0], udwUidListNum, pstSession->m_udwSeqNo));

    // 4. send request
    LTasksGroup        stTasks;
    stTasks.m_Tasks[0].SetConnSession(pstSession->m_pstUserLinkerNode->m_stDownHandle);
    stTasks.m_Tasks[0].SetSendData(pucPackage, udwPackageLen);
    stTasks.m_Tasks[0].SetNeedResponse(0);
    stTasks.SetValidTasks(1);
    if(!pstSession->m_poLongConn->SendData(&stTasks))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("PushDataBasic_RefreshUid: send request failed [seq=%u]", pstSession->m_udwSeqNo));
        return -3;
    }
    return 0;
}

TINT32 CAuPushData::PushDataBasic_TargetAid( SSession *pstSession, Json::Value& rJson, TUINT32 udwTargetAid, TUINT32 *pudwExceptUidList, TUINT32 udwExceptListNum )
{
    CDownMgr *pobjDownMgr = CDownMgr::Instance();
    TUCHAR *pucPackage = NULL;
    TUINT32 udwPackageLen = 0;
    CBaseProtocolPack *pobjPack = pstSession->m_ppPackTool[0];
    TBOOL bRet = FALSE;
    TINT32 dwCostTime = 1;
    TINT32 dwTargetType = EN_PUSH_DATA_TYPE__AID;
    TINT32 dwListSize = 1;
    TUINT8 ucCompressFlag = 0;
    CJsonResultGenerator *pJsonGenerator = (CJsonResultGenerator*)pstSession->m_pJsonGenerator;

    // 1. get down node
    if(pstSession->m_bUserLinkerNodeExist == FALSE)
    {
        pstSession->m_pstUserLinkerNode = NULL;
        if (S_OK == pobjDownMgr->zk_GetNode(DOWN_NODE_TYPE__USER_LINKER, &pstSession->m_pstUserLinkerNode))
        {
            pstSession->m_bUserLinkerNodeExist = TRUE;
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("Get UserLinker node succ [seq=%u]", pstSession->m_udwSeqNo));
        }
        else
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("Get UserLinker node fail [seq=%u]", pstSession->m_udwSeqNo));
            return -1;
        }

        if(NULL == pstSession->m_pstUserLinkerNode->m_stDownHandle.handle)
        {
            pstSession->m_bUserLinkerNodeExist = FALSE;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("PushDataBasic_TargetAid: handle [handlevalue=0x%p] [seq=%u]", \
                pstSession->m_pstAwsProxyNode->m_stDownHandle.handle, pstSession->m_udwSeqNo));
            return -2;
        }
    }

    // 2. get data
    pJsonGenerator->GenPushData_Pb(pstSession, EN_SERVICE_TYPE__CLIENT__DATA_PUSH_RSP, rJson);

    // 3. get package
    pobjPack->ResetContent();
    pobjPack->SetServiceType(EN_SERVICE_TYPE__CLIENT__DATA_PUSH_RSP);
    pobjPack->SetSeq(CGlobalServ::GenerateHsReqSeq());
    pobjPack->SetKey(EN_GLOBAL_KEY__REQ_SEQ, pstSession->m_udwSeqNo);
    pobjPack->SetKey(EN_GLOBAL_KEY__TARGET_TYPE, dwTargetType);
    dwListSize = 1;
    pobjPack->SetKey(EN_GLOBAL_KEY__TARGET_LIST_SIZE, dwListSize);
    pobjPack->SetKey(EN_GLOBAL_KEY__TARGET_LIST, (TUCHAR*)&udwTargetAid, sizeof(udwTargetAid));
    pobjPack->SetKey(EN_GLOBAL_KEY__EXCEPT_LIST_SIZE, udwExceptListNum);
    if(udwExceptListNum)
    {
        dwTargetType = EN_PUSH_DATA_TYPE__UID;
        pobjPack->SetKey(EN_GLOBAL_KEY__EXCEPT_TYPE, dwTargetType);
        pobjPack->SetKey(EN_GLOBAL_KEY__EXCEPT_LIST, (TUCHAR*)pudwExceptUidList, sizeof(TUINT32) * udwExceptListNum);
    }
    pobjPack->SetKey(EN_GLOBAL_KEY__RES_BUF_COMPRESS_FLAG, ucCompressFlag);
    pobjPack->SetKey(EN_GLOBAL_KEY__REQ_BUF, (TUCHAR*)&pstSession->m_pTmpBuf[0], pstSession->m_udwTmpBufLen);

    pobjPack->GetPackage(&pucPackage, &udwPackageLen);

    TSE_LOG_DEBUG(pstSession->m_poServLog, ("PushDataBasic_TargetAid: service=%u, package_len=%u, target_aid=%u,  [seq=%u]", 
        EN_SERVICE_TYPE__CLIENT__DATA_PUSH_RSP, udwPackageLen, udwTargetAid, pstSession->m_udwSeqNo));

    // 4. send request
    LTasksGroup        stTasks;
    stTasks.m_Tasks[0].SetConnSession(pstSession->m_pstUserLinkerNode->m_stDownHandle);
    stTasks.m_Tasks[0].SetSendData(pucPackage, udwPackageLen);
    stTasks.m_Tasks[0].SetNeedResponse(0);
    stTasks.SetValidTasks(1);
    if(!pstSession->m_poLongConn->SendData(&stTasks))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("PushDataBasic_TargetAid: send request failed [seq=%u]", pstSession->m_udwSeqNo));
        return -3;
    }
    return 0;
}

TINT32 CAuPushData::PushData_WildToMap(SSession *pstSession, TbMap *ptbWild)
{
    TUINT32 udwSBlockId = 0;
    TINT32 dwSid = ptbWild->m_nSid;
    Json::Value jsonMap;
    Json::Value jsonTmp;
    if (CMapLogic::IsWildNeedToDelete(ptbWild))
    {
        udwSBlockId = CMapBase::GetBlockIdFromPos(ptbWild->m_nId);
        jsonMap[CCommonFunc::NumToString(udwSBlockId)]["svr_map_inc"]["list"][CCommonFunc::NumToString(ptbWild->m_nId)] = Json::Value(Json::nullValue);
        jsonMap[CCommonFunc::NumToString(udwSBlockId)]["svr_map_inc"]["svr_id"] = dwSid;
    }
    else
    {
        jsonTmp.clear();
        CCommJson::PushData_GenMapJson(ptbWild->m_nSid, ptbWild, jsonTmp);

        udwSBlockId = CMapBase::GetBlockIdFromPos(ptbWild->m_nId);
        jsonMap[CCommonFunc::NumToString(udwSBlockId)]["svr_map_inc"]["list"][CCommonFunc::NumToString(ptbWild->m_nId)] = jsonTmp;
        jsonMap[CCommonFunc::NumToString(udwSBlockId)]["svr_map_inc"]["svr_id"] = dwSid;
    }

    return PushDataBasic_Map(pstSession, jsonMap, dwSid);
}

TINT32 CAuPushData::PushDataAid_March( SSession *pstSession, TbMarch_action* ptbAction, TINT32 dwUpdtType, const TCHAR* pszTable, 
    TUINT32 udwTargetAid, TUINT32 *pudwExceptUidList /*= NULL*/, TUINT32 udwExceptListNum /*= 0*/ )
{
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("PushDataAid_March: target=%u action_id=%ld, suid=%ld sal=%ld tal=%ld tid=%ld, [main=%ld,sec=%ld,status=%d] [seq=%u]", 
        udwTargetAid, ptbAction->m_nId, ptbAction->m_nSuid, ptbAction->m_nSal, ptbAction->m_nTal, ptbAction->m_nTid, 
        ptbAction->m_nMclass, ptbAction->m_nSclass, ptbAction->m_nStatus, pstSession->m_udwSeqNo));

    // action
    Json::Value rjsonResult(Json::objectValue);
    CCommJson::GenMarchInfoForPush(ptbAction, rjsonResult[pszTable], dwUpdtType);

    // send push data
    return PushDataBasic_TargetAid(pstSession, rjsonResult, udwTargetAid, pudwExceptUidList, udwExceptListNum);
}

TINT32 CAuPushData::AddPushData_ReqMarchAction( SSession *pstSession, TbMarch_action *ptbMarch, TUINT8 ucFlag )
{
    TUINT32 udwSuid = 0;
    TUINT32 udwTuid = 0;
    map<TINT64, SPushActionNode>::iterator it;

    SPushActionNode *pstNode = NULL;
    CPushMapData *pobjPushDataMap = &pstSession->m_objAuPushDataNode.m_objPushDataMap;
    CPushAlData *pobjPushDataAl = &pstSession->m_objAuPushDataNode.m_objPushDataSourceAl;
    TBOOL isActive = TRUE;

    if(EN_TABLE_UPDT_FLAG__UNCHANGE == ucFlag)
    {
        return 0;
    }

    //过滤不需要push的action
    if (ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__THRONE_TIMER
        || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__PRISON_TIMER
        || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__NOTI_TIMER
        || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__IDOL_PERIOD
        || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__THRONE_PERIOD
        || ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__TAX)
    {
        return 0;
    }

    it = pobjPushDataAl->m_mapAlActionRaw.find(ptbMarch->m_nId);
    if(it != pobjPushDataAl->m_mapAlActionRaw.end())
    {
        pstNode = &it->second;
        TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("AddPushData_ReqMarchAction[march--rawinfo]: target=%u action_id=%ld, suid=%u sal=%ld,tuid=%u,tal=%ld [seq=%u]", 
            pobjPushDataAl->m_udwAllianceId, ptbMarch->m_nId, pstNode->m_udwSUid, pstNode->m_nSal, pstNode->m_udwTUid, pstNode->m_nTal, pstSession->m_udwSeqNo));
    }

    // 处理sal的情况
    if(ptbMarch->m_nSal > 0 && ptbMarch->m_nSal == pobjPushDataAl->m_udwAllianceId)
    {
        if(pobjPushDataAl->m_setActionId.count(ptbMarch->m_nId) == 0)
        {
            TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("AddPushData_ReqMarchAction[march--update]:target=%u action_id=%ld, suid=%ld sal=%ld,tuid=%ld,tal=%ld [main=%ld,sec=%ld,status=%d] [seq=%u]", 
                pobjPushDataAl->m_udwAllianceId, ptbMarch->m_nId, ptbMarch->m_nSuid, ptbMarch->m_nSal, ptbMarch->m_nTuid, ptbMarch->m_nTal,
                ptbMarch->m_nMclass, ptbMarch->m_nSclass, ptbMarch->m_nStatus, pstSession->m_udwSeqNo));

            pobjPushDataAl->m_vecActionList.push_back(SPushActionNode(ptbMarch, EN_MARCH_ACTION, ucFlag, isActive));
            pobjPushDataAl->m_setActionId.insert(ptbMarch->m_nId);
        }
    }
    //处理sal变化的情况
    if(pstNode && pstNode->m_nSal != ptbMarch->m_nSal)
    {
        if(pstNode->m_nSal > 0)
        {
            if(pstNode->m_nSal == pobjPushDataAl->m_udwAllianceId)
            {
                if(pobjPushDataAl->m_setActionId.count(ptbMarch->m_nId) == 0)
                {
                    TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("AddPushData_ReqMarchAction[march--delete--sal]:sal change!!!. target=%u action_id=%ld, suid=%ld sal=%ld,tuid=%ld,tal=%ld [main=%ld,sec=%ld,status=%d] [seq=%u]", 
                        pobjPushDataAl->m_udwAllianceId, ptbMarch->m_nId, ptbMarch->m_nSuid, ptbMarch->m_nSal, ptbMarch->m_nTuid, ptbMarch->m_nTal,
                        ptbMarch->m_nMclass, ptbMarch->m_nSclass, ptbMarch->m_nStatus, pstSession->m_udwSeqNo));

                    pobjPushDataAl->m_vecActionList.push_back(SPushActionNode(ptbMarch, EN_MARCH_ACTION, EN_TABLE_UPDT_FLAG__DEL, isActive));
                    pobjPushDataAl->m_setActionId.insert(ptbMarch->m_nId);
                }
            }
            else
            {
                TSE_LOG_ERROR(CGlobalServ::m_poServLog, ("AddPushData_ReqMarchAction[march--error--sal]:sal change!!!. target=%u action_id=%ld, suid=%ld sal=%ld,tuid=%ld,tal=%ld [main=%ld,sec=%ld,status=%d] [seq=%u]", 
                    pobjPushDataAl->m_udwAllianceId, ptbMarch->m_nId, ptbMarch->m_nSuid, ptbMarch->m_nSal, ptbMarch->m_nTuid, ptbMarch->m_nTal,
                    ptbMarch->m_nMclass, ptbMarch->m_nSclass, ptbMarch->m_nStatus, pstSession->m_udwSeqNo));

                PushDataAid_March(pstSession, ptbMarch, EN_TABLE_UPDT_FLAG__DEL, "svr_al_action_list", ptbMarch->m_nTal);
            }
        }
    }

    //处理tal的情况
	if(ptbMarch->m_nTal > 0)
    {
        if(ptbMarch->m_nTal == pobjPushDataAl->m_udwAllianceId)
        {
            if(pobjPushDataAl->m_setActionId.count(ptbMarch->m_nId) == 0)
            {
                TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("AddPushData_ReqMarchAction[march--update--tal]:target=%u action_id=%ld, suid=%ld sal=%ld,tuid=%ld,tal=%ld [main=%ld,sec=%ld,status=%d] [seq=%u]", 
                    pobjPushDataAl->m_udwAllianceId, ptbMarch->m_nId, ptbMarch->m_nSuid, ptbMarch->m_nSal, ptbMarch->m_nTuid, ptbMarch->m_nTal,
                    ptbMarch->m_nMclass, ptbMarch->m_nSclass, ptbMarch->m_nStatus, pstSession->m_udwSeqNo));

                pobjPushDataAl->m_vecActionList.push_back(SPushActionNode(ptbMarch, EN_MARCH_ACTION, ucFlag, isActive));
                pobjPushDataAl->m_setActionId.insert(ptbMarch->m_nId);
            }
        }
        else if (ptbMarch->m_nTal == CActionBase::GenThroneTargetId(pstSession->m_tbThrone.m_nSid, pstSession->m_tbThrone.m_nPos))
        {
            if (pstSession->m_tbThrone.m_nAlid > 0)
            {
                TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("AddPushData_ReqMarchAction[march--update--tal_throne]:target=%ld action_id=%ld, suid=%ld sal=%ld,tuid=%ld,tal=%ld [main=%ld,sec=%ld,status=%d] [seq=%u]",
                    pstSession->m_tbThrone.m_nAlid, ptbMarch->m_nId, ptbMarch->m_nSuid, ptbMarch->m_nSal, ptbMarch->m_nTuid, ptbMarch->m_nTal,
                    ptbMarch->m_nMclass, ptbMarch->m_nSclass, ptbMarch->m_nStatus, pstSession->m_udwSeqNo));
                PushDataAid_March(pstSession, ptbMarch, EN_TABLE_UPDT_FLAG__CHANGE, "svr_al_p_action_list", pstSession->m_tbThrone.m_nAlid);
            }
            if (pstSession->bKingChanged && pstSession->dwRawThroneAlid)
            {
                TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("AddPushData_ReqMarchAction[march--delete--tal_throne]:target=%d action_id=%ld, suid=%ld sal=%ld,tuid=%ld,tal=%ld [main=%ld,sec=%ld,status=%d] [seq=%u]",
                    pstSession->dwRawThroneAlid, ptbMarch->m_nId, ptbMarch->m_nSuid, ptbMarch->m_nSal, ptbMarch->m_nTuid, ptbMarch->m_nTal,
                    ptbMarch->m_nMclass, ptbMarch->m_nSclass, ptbMarch->m_nStatus, pstSession->m_udwSeqNo));
                PushDataAid_March(pstSession, ptbMarch, EN_TABLE_UPDT_FLAG__DEL, "svr_al_p_action_list", pstSession->dwRawThroneAlid);
            }
        }
        else
        {
            TSE_LOG_ERROR(CGlobalServ::m_poServLog, ("AddPushData_ReqMarchAction[march--error--tal]:target=%u action_id=%ld, suid=%ld sal=%ld,tuid=%ld,tal=%ld [main=%ld,sec=%ld,status=%d] [seq=%u]", 
                pobjPushDataAl->m_udwAllianceId, ptbMarch->m_nId, ptbMarch->m_nSuid, ptbMarch->m_nSal, ptbMarch->m_nTuid, ptbMarch->m_nTal,
                ptbMarch->m_nMclass, ptbMarch->m_nSclass, ptbMarch->m_nStatus, pstSession->m_udwSeqNo));
            PushDataAid_March(pstSession, ptbMarch, ucFlag, "svr_al_p_action_list", ptbMarch->m_nTal);
        }
    }
    //处理tal变化的情况
    if(pstNode && pstNode->m_nTal != ptbMarch->m_nTal)
    {
        if(pstNode->m_nTal > 0)
        {
            if(pstNode->m_nTal == pobjPushDataAl->m_udwAllianceId)
            {
                if(pobjPushDataAl->m_setActionId.count(ptbMarch->m_nId) == 0)
                {
                    TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("AddPushData_ReqMarchAction[march--delete--rawtal_is_us]:. target=%u action_id=%ld, suid=%ld sal=%ld,tuid=%ld,tal=%ld [main=%ld,sec=%ld,status=%d] [seq=%u]", 
                        pobjPushDataAl->m_udwAllianceId, ptbMarch->m_nId, ptbMarch->m_nSuid, ptbMarch->m_nSal, ptbMarch->m_nTuid, ptbMarch->m_nTal,
                        ptbMarch->m_nMclass, ptbMarch->m_nSclass, ptbMarch->m_nStatus, pstSession->m_udwSeqNo));

                    pobjPushDataAl->m_vecActionList.push_back(SPushActionNode(ptbMarch, EN_MARCH_ACTION, EN_TABLE_UPDT_FLAG__DEL, isActive));
                    pobjPushDataAl->m_setActionId.insert(ptbMarch->m_nId);
                }
            }
            else if (pstNode->m_nTal == CActionBase::GenThroneTargetId(pstSession->m_tbThrone.m_nSid, pstSession->m_tbThrone.m_nPos))
            {
                if (!pstSession->bKingChanged && pstSession->m_tbThrone.m_nAlid > 0)
                {
                    TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("AddPushData_ReqMarchAction[march--delete--tal_throne]:target=%ld action_id=%ld, suid=%ld sal=%ld,tuid=%ld,tal=%ld [main=%ld,sec=%ld,status=%d] [seq=%u]",
                        pstSession->m_tbThrone.m_nAlid, ptbMarch->m_nId, ptbMarch->m_nSuid, ptbMarch->m_nSal, ptbMarch->m_nTuid, ptbMarch->m_nTal,
                        ptbMarch->m_nMclass, ptbMarch->m_nSclass, ptbMarch->m_nStatus, pstSession->m_udwSeqNo));
                    PushDataAid_March(pstSession, ptbMarch, EN_TABLE_UPDT_FLAG__DEL, "svr_al_p_action_list", pstSession->m_tbThrone.m_nAlid);
                }
                else if (pstSession->bKingChanged && pstSession->dwRawThroneAlid)
                {
                    TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("AddPushData_ReqMarchAction[march--delete--tal_throne]:target=%d action_id=%ld, suid=%ld sal=%ld,tuid=%ld,tal=%ld [main=%ld,sec=%ld,status=%d] [seq=%u]",
                        pstSession->dwRawThroneAlid, ptbMarch->m_nId, ptbMarch->m_nSuid, ptbMarch->m_nSal, ptbMarch->m_nTuid, ptbMarch->m_nTal,
                        ptbMarch->m_nMclass, ptbMarch->m_nSclass, ptbMarch->m_nStatus, pstSession->m_udwSeqNo));
                    PushDataAid_March(pstSession, ptbMarch, EN_TABLE_UPDT_FLAG__DEL, "svr_al_p_action_list", pstSession->dwRawThroneAlid);
                }
            }
            else
            {
                TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("AddPushData_ReqMarchAction[march--delete--rawtal_not_us]:target=%u action_id=%ld, suid=%ld sal=%ld,tuid=%ld,tal=%ld [main=%ld,sec=%ld,status=%d] [seq=%u]", 
                    pobjPushDataAl->m_udwAllianceId, ptbMarch->m_nId, ptbMarch->m_nSuid, ptbMarch->m_nSal, ptbMarch->m_nTuid, ptbMarch->m_nTal,
                    ptbMarch->m_nMclass, ptbMarch->m_nSclass, ptbMarch->m_nStatus, pstSession->m_udwSeqNo));
                PushDataAid_March(pstSession, ptbMarch, EN_TABLE_UPDT_FLAG__DEL, "svr_al_p_action_list", pstNode->m_nTal);
            }
        }
    }

    //if(EN_TABLE_UPDT_FLAG__UNCHANGE != ucFlag)
    {
        if(pstNode)
        {
            AddPushData_AlUid(pobjPushDataAl, pstNode->m_udwSUid);
            AddPushData_AlUid(pobjPushDataAl, pstNode->m_udwTUid);
        }
        AddPushData_AlUid(pobjPushDataAl, ptbMarch->m_nSuid);
        AddPushData_AlUid(pobjPushDataAl, ptbMarch->m_nTuid);

        //add action to map
        AddPushData_MapAction(pobjPushDataMap, ptbMarch, ucFlag);
    }
    return 0;
}

TINT32 CAuPushData::PushDataAid_Refresh(SSession *pstSession, TUINT32 *pudwAidList, TUINT32 udwAidListNum, TUINT32 *pudwExceptUidList = NULL, TUINT32 udwExceptListNum = 0)
{
    TSE_LOG_DEBUG(pstSession->m_poServLog, ("PushDataAid_Refresh: target=%u [seq=%u]",
        pudwAidList[0], pstSession->m_udwSeqNo));

    CDownMgr *pobjDownMgr = CDownMgr::Instance();
    TUCHAR *pucPackage = NULL;
    TUINT32 udwPackageLen = 0;
    CBaseProtocolPack *pobjPack = pstSession->m_ppPackTool[0];
    TBOOL bRet = FALSE;
    TINT32 dwCostTime = 1;
    TINT32 dwTargetType = EN_PUSH_DATA_TYPE__AID;
    TINT32 dwListSize = 1;
    TUINT8 ucCompressFlag = 0;
    CJsonResultGenerator *pJsonGenerator = (CJsonResultGenerator*)pstSession->m_pJsonGenerator;

    if (udwAidListNum == 0)
    {
        return 0;
    }

    // 1. get down node
    if (pstSession->m_bUserLinkerNodeExist == FALSE)
    {
        pstSession->m_pstUserLinkerNode = NULL;
        if (S_OK == pobjDownMgr->zk_GetNode(DOWN_NODE_TYPE__USER_LINKER, &pstSession->m_pstUserLinkerNode))
        {
            pstSession->m_bUserLinkerNodeExist = TRUE;
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("Get UserLinker node succ [seq=%u]", pstSession->m_udwSeqNo));
        }
        else
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("Get UserLinker node fail [seq=%u]", pstSession->m_udwSeqNo));
            return -1;
        }

        if (NULL == pstSession->m_pstUserLinkerNode->m_stDownHandle.handle)
        {
            pstSession->m_bUserLinkerNodeExist = FALSE;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("PushDataAid_Refresh:handle [handlevalue=0x%p] [seq=%u]", 
                pstSession->m_pstUserLinkerNode->m_stDownHandle.handle, pstSession->m_udwSeqNo));
            return -2;
        }
    }

    // 2. get data
    Json::Value rJson(Json::objectValue);
    pJsonGenerator->GenPushData_Pb(pstSession, EN_SERVICE_TYPE__CLIENT__INC_REFRESH_RSP, rJson);

    // 3. get package
    pobjPack->ResetContent();
    pobjPack->SetServiceType(EN_SERVICE_TYPE__CLIENT__INC_REFRESH_RSP);
    pobjPack->SetSeq(CGlobalServ::GenerateHsReqSeq());
    pobjPack->SetKey(EN_GLOBAL_KEY__REQ_SEQ, pstSession->m_udwSeqNo);
    pobjPack->SetKey(EN_GLOBAL_KEY__TARGET_TYPE, dwTargetType);
    pobjPack->SetKey(EN_GLOBAL_KEY__TARGET_LIST_SIZE, udwAidListNum);
    pobjPack->SetKey(EN_GLOBAL_KEY__TARGET_LIST, (TUCHAR*)pudwAidList, sizeof(TUINT32)*udwAidListNum);
    
    pobjPack->SetKey(EN_GLOBAL_KEY__EXCEPT_LIST_SIZE, udwExceptListNum);
    if (udwExceptListNum)
    {
        dwTargetType = EN_PUSH_DATA_TYPE__UID;
        pobjPack->SetKey(EN_GLOBAL_KEY__EXCEPT_TYPE, dwTargetType);
        pobjPack->SetKey(EN_GLOBAL_KEY__EXCEPT_LIST, (TUCHAR*)pudwExceptUidList, sizeof(TUINT32)* udwExceptListNum);
    }

    pobjPack->SetKey(EN_GLOBAL_KEY__RES_BUF_COMPRESS_FLAG, ucCompressFlag);
    pobjPack->SetKey(EN_GLOBAL_KEY__REQ_BUF, (TUCHAR*)&pstSession->m_pTmpBuf[0], pstSession->m_udwTmpBufLen);

    pobjPack->GetPackage(&pucPackage, &udwPackageLen);

    TSE_LOG_DEBUG(pstSession->m_poServLog, ("PushDataAid_Refresh: service=%u, package_len=%u, target_aid=%u num=%u [seq=%u]",
        EN_SERVICE_TYPE__CLIENT__INC_REFRESH_RSP, udwPackageLen, pudwAidList[0], udwAidListNum, pstSession->m_udwSeqNo));

    // 4. send request
    LTasksGroup        stTasks;
    stTasks.m_Tasks[0].SetConnSession(pstSession->m_pstUserLinkerNode->m_stDownHandle);
    stTasks.m_Tasks[0].SetSendData(pucPackage, udwPackageLen);
    stTasks.m_Tasks[0].SetNeedResponse(0);
    stTasks.SetValidTasks(1);
    if (!pstSession->m_poLongConn->SendData(&stTasks))
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("PushDataAid_Refresh: send request failed [seq=%u]", pstSession->m_udwSeqNo));
        return -3;
    }
    return 0;
}