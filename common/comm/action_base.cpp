#include "action_base.h"
#include "time_utils.h"
#include "buffer_base.h"
#include "tool_base.h"
#include "map_base.h"
#include "common_func.h"
#include "common_base.h"
#include "wild_info.h"
#include "map_logic.h"
#include "msg_base.h"

TINT32 CActionBase::GetAlActionIndex(TbAlliance_action* patbAlAction, TUINT32 udwActionNum, TINT64 ddwTargetActionId)
{
    TINT32 dwRetIdx = -1;
    for(TUINT32 udwIdx = 0; udwIdx < udwActionNum; udwIdx++)
    {
        if(patbAlAction[udwIdx].m_nId == ddwTargetActionId)
        {
            dwRetIdx = udwIdx;
            break;
        }
    }
    return dwRetIdx;
}

TbAlliance_action* CActionBase::GetAlAction(TbAlliance_action* patbAlAction, TUINT32 udwActionNum, TINT64 ddwTargetActionId)
{
    TINT32 dwActionIndex = CActionBase::GetAlActionIndex(patbAlAction, udwActionNum, ddwTargetActionId);
    if(dwActionIndex < 0)
    {
        return NULL;
    }
    else
    {
        return &patbAlAction[dwActionIndex];
    }
}

TINT32 CActionBase::AddAlAction(SUserInfo *pstUser, SCityInfo*pstCity, TUINT8 enMainClass, TUINT8 enSecClass, TUINT8 enStatus, TUINT32 udwCostTime, UActionParam *pstParam, TUINT32 udwTargetPos /*= 0*/, TUINT64 uddwNewTaskId /*= 0Lui*/, TINT32 dwSvrId /*= -1*/)
{
    TbAlliance_action *ptbAlAction = &pstUser->m_atbSelfAlAction[pstUser->m_udwSelfAlActionNum];
    ptbAlAction->Reset();

    assert(pstUser->m_tbPlayer.m_nUid);

    // 1. set head
    if (uddwNewTaskId)
    {
        ptbAlAction->Set_Id(uddwNewTaskId);
    }
    else
    {
        ptbAlAction->Set_Id(CToolBase::GetPlayerNewTaskId(pstUser->m_tbPlayer.m_nUid, pstUser->m_tbLogin.m_nSeq));
        pstUser->m_tbLogin.Set_Seq(pstUser->m_tbLogin.m_nSeq + 1); //llt add,防止,同一个用户请求中,生成多个action的情况
    }

    ptbAlAction->Set_Suid(pstUser->m_tbPlayer.m_nUid);
    ptbAlAction->Set_Mclass(enMainClass);
    ptbAlAction->Set_Sclass(enSecClass);
    ptbAlAction->Set_Status(enStatus);

    ptbAlAction->Set_Btime(CTimeUtils::GetUnixTime());
    ptbAlAction->Set_Ctime(udwCostTime);
    ptbAlAction->Set_Etime(ptbAlAction->m_nBtime + udwCostTime);

    ptbAlAction->Set_Sid(pstUser->m_tbPlayer.m_nSid);

    // 2. param
    memcpy((char*)&ptbAlAction->m_bParam.m_astList[0], (char*)pstParam, sizeof(UActionParam));
    ptbAlAction->SetFlag(TbALLIANCE_ACTION_FIELD_PARAM);

    // 3. set flag
    pstUser->m_aucSelfAlActionFlag[pstUser->m_udwSelfAlActionNum] = EN_TABLE_UPDT_FLAG__NEW;

    // 4. auto increase
    pstUser->m_udwSelfAlActionNum++;

    // 5. for log
    pstUser->m_uddwNewActionId = ptbAlAction->m_nId;

    return 0;
}

TbMarch_action* CActionBase::AddMarchAction(SUserInfo *pstUser, SCityInfo*pstCity, TUINT8 enMainClass, TUINT8 enSecClass, TUINT8 enStatus, TUINT32 udwCostTime,
    SActionMarchParam* pstParam, TUINT64 uddwNewTaskId, TUINT32 udwTargetId)
{
    TbMarch_action *ptbMarch = &pstUser->m_atbMarch[pstUser->m_udwMarchNum];
    ptbMarch->Reset();

    assert(pstUser->m_tbPlayer.m_nUid);

    // 1. set head
    ptbMarch->Set_Fixed(0);

    if(uddwNewTaskId)
    {
        ptbMarch->Set_Id(uddwNewTaskId);
    }
    else
    {
        ptbMarch->Set_Id(CToolBase::GetPlayerNewTaskId(pstUser->m_tbPlayer.m_nUid, pstUser->m_tbLogin.m_nSeq));
    }
    TbLogin& tbLogin = pstUser->m_tbLogin;
    tbLogin.Set_Seq(tbLogin.m_nSeq + 1); //llt add,防止,同一个用户请求中,生成多个action的情况
    ptbMarch->Set_Suid(pstUser->m_tbPlayer.m_nUid);
    ptbMarch->Set_Scid(pstCity->m_stTblData.m_nPos);
    ptbMarch->Set_Mclass(enMainClass);
    ptbMarch->Set_Sclass(enSecClass);
    ptbMarch->Set_Status(enStatus);
    ptbMarch->Set_Btime(CTimeUtils::GetUnixTime());
    ptbMarch->Set_Ctime(udwCostTime);
    ptbMarch->Set_Etime(ptbMarch->m_nBtime + udwCostTime);
    ptbMarch->Set_Retry(0);

    ptbMarch->Set_Tuid(pstParam->m_ddwTargetUserId);
    ptbMarch->Set_Tpos(udwTargetId);
    
    ptbMarch->Set_Sid(pstUser->m_tbPlayer.m_nSid);

    ptbMarch->Set_Sbid(CMapBase::GetBlockIdFromPos(ptbMarch->m_nScid));
    ptbMarch->Set_Tbid(CMapBase::GetBlockIdFromPos(udwTargetId));

    ptbMarch->Set_Savatar(pstUser->m_tbPlayer.m_nAvatar);

    switch(enSecClass)
    {
    case EN_ACTION_SEC_CLASS__TRANSPORT:
    case EN_ACTION_SEC_CLASS__ATTACK:
        ptbMarch->Set_Tal(-1 * (TINT64)pstParam->m_ddwTargetUserId);
        if (pstParam->m_ddwTargetAlliance > 0 &&
            EN_WILD_CLASS_MONSTER_NEST == CMapLogic::GetWildClass(pstUser->m_tbPlayer.m_nSid, pstParam->m_ddwTargetType))
        {
            ptbMarch->Set_Tal(pstParam->m_ddwTargetAlliance);
        }
        break;
    case EN_ACTION_SEC_CLASS__RALLY_WAR:
        ptbMarch->Set_Tal(-1 * (TINT64)pstParam->m_ddwTargetUserId);
        if(pstParam->m_ddwTargetAlliance > 0)
        {
            ptbMarch->Set_Tal(pstParam->m_ddwTargetAlliance);
        }
        if(pstParam->m_ddwSourceAlliance)
        {
            ptbMarch->Set_Sal(pstParam->m_ddwSourceAlliance);
        }
        break;
    case EN_ACTION_SEC_CLASS__REINFORCE_NORMAL:
    case EN_ACTION_SEC_CLASS__RALLY_REINFORCE:
        ptbMarch->Set_Tal(-1 * (TINT64)pstParam->m_ddwTargetUserId);
        break;
    case EN_ACTION_SEC_CLASS__ATTACK_THRONE:
        ptbMarch->Set_Tal(CActionBase::GenThroneTargetId(pstUser->m_tbPlayer.m_nSid, udwTargetId));
        break;
    case EN_ACTION_SEC_CLASS__RALLY_WAR_THRONE:
        ptbMarch->Set_Sal(pstParam->m_ddwSourceAlliance);
        ptbMarch->Set_Tal(CActionBase::GenThroneTargetId(pstUser->m_tbPlayer.m_nSid, udwTargetId));
        break;
    case EN_ACTION_SEC_CLASS__REINFORCE_THRONE:
        if (pstParam->m_ddwTargetType == EN_WILD_TYPE__THRONE_NEW)
        {
            ptbMarch->Set_Sal(pstParam->m_ddwSourceAlliance);
        }
        else
        {
            ptbMarch->Set_Tal(-1 * (TINT64)pstParam->m_ddwTargetUserId);
        }
        break;
    case EN_ACTION_SEC_CLASS__ASSIGN_THRONE:
        ptbMarch->Set_Sal(pstParam->m_ddwSourceAlliance);
        break;
    case EN_ACTION_SEC_CLASS__ATTACK_IDOL:
    case EN_ACTION_SEC_CLASS__SCOUT:
    case EN_ACTION_SEC_CLASS__OCCUPY:
        break;
    }

    // 2. set param
    memcpy((char*)&ptbMarch->m_bParam.m_astList[0], (char*)pstParam, sizeof(SActionMarchParam));
    ptbMarch->SetFlag(TbMARCH_ACTION_FIELD_PARAM);


    // 3. set table flag
    pstUser->m_aucMarchFlag[pstUser->m_udwMarchNum] = EN_TABLE_UPDT_FLAG__NEW;

    // 4. auto increase
    pstUser->m_udwMarchNum++;

    // 5. for log
    pstUser->m_uddwNewActionId = ptbMarch->m_nId;

    // 6. for push
    pstUser->m_ptbPushMarchAction = ptbMarch;
    pstUser->dwPushMarchActionType = EN_TABLE_UPDT_FLAG__NEW;

    return ptbMarch;
}

TINT32 CActionBase::GetMarchIndex(TbMarch_action* patbMacrh, TUINT32 udwMarchNum, TINT64 ddwTargetActionId)
{
    TINT32 dwRetIdx = -1;
    for(TUINT32 udwIdx = 0; udwIdx < udwMarchNum; udwIdx++)
    {
        if(patbMacrh[udwIdx].m_nId == ddwTargetActionId)
        {
            dwRetIdx = udwIdx;
            break;
        }
    }
    return dwRetIdx;
}

TbMarch_action* CActionBase::GetMarch(TbMarch_action* patbMacrh, TUINT32 udwMarchNum, TINT64 ddwTargetActionId)
{
    TINT32 dwActionIndex = CActionBase::GetMarchIndex(patbMacrh, udwMarchNum, ddwTargetActionId);
    if(dwActionIndex < 0)
    {
        return NULL;
    }
    else
    {
        return &patbMacrh[dwActionIndex];
    }
}

TINT32 CActionBase::GetActionIndex(TbAction* patbAction, TUINT32 udwActionNum, TINT64 ddwTargetActionId)
{
    TINT32 dwRetIdx = -1;
    for(TUINT32 udwIdx = 0; udwIdx < udwActionNum; udwIdx++)
    {
        if(patbAction[udwIdx].m_nId == ddwTargetActionId)
        {
            dwRetIdx = udwIdx;
            break;
        }
    }
    return dwRetIdx;
}

TbAction* CActionBase::GetAction(TbAction* patbAction, TUINT32 udwActionNum, TINT64 ddwTargetActionId)
{
    TINT32 dwActionIndex = CActionBase::GetActionIndex(patbAction, udwActionNum, ddwTargetActionId);
    if(dwActionIndex < 0)
    {
        return NULL;
    }
    else
    {
        return &patbAction[dwActionIndex];
    }
}

TVOID CActionBase::UpdtActionWhenLeaveAlliance(SUserInfo *pstUser, TINT64 ddwSid, TINT64 ddwSrcUid)
{
    const Json::Value& oWildResJson = CWildInfo::GetWildResInfo(ddwSid);
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();

    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
    {
        if (pstUser->m_aucMarchFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }

        //召回驻扎在盟友的普通reinforce
        if (CActionBase::IsPlayerOwnedAction(ddwSrcUid, pstUser->m_atbMarch[udwIdx].m_nId)
            && pstUser->m_atbMarch[udwIdx].m_nMclass == EN_ACTION_MAIN_CLASS__MARCH
            && pstUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_NORMAL
            && pstUser->m_atbMarch[udwIdx].m_nStatus != EN_MARCH_STATUS__RETURNING)
        {
            CMsgBase::RefreshUserInfo(pstUser->m_atbMarch[udwIdx].m_nTuid);
            CActionBase::ReturnMarch(&pstUser->m_atbMarch[udwIdx]);
            pstUser->m_aucMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
            continue;
        }

        // 召回怪物巢穴拉取
        if (CActionBase::IsPlayerOwnedAction(ddwSrcUid, pstUser->m_atbMarch[udwIdx].m_nId)
            && pstUser->m_atbMarch[udwIdx].m_nMclass == EN_ACTION_MAIN_CLASS__MARCH
            && pstUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__OCCUPY)
        {
            if (EN_WILD_CLASS_MONSTER_NEST == oWildResJson[CCommonFunc::NumToString(pstUser->m_atbMarch[udwIdx].m_bParam[0].m_ddwTargetType)]["a0"]["a0"].asUInt())
            {
                //更换野地主防守...
                if (pstUser->m_atbMarch[udwIdx].m_nStatus == EN_MARCH_STATUS__LOADING
                    || pstUser->m_atbMarch[udwIdx].m_nStatus == EN_MARCH_STATUS__PRE_LOADING)
                {
                    TbMap *ptbWild = NULL;
                    for (TUINT32 udwMapIdx = 0; udwMapIdx < pstUser->m_udwWildNum; udwMapIdx++)
                    {
                        if (pstUser->m_atbWild[udwMapIdx].m_nId == pstUser->m_atbMarch[udwIdx].m_nTpos
                            && pstUser->m_atbWild[udwMapIdx].m_nUid == ddwSrcUid)
                        {
                            ptbWild = &pstUser->m_atbWild[udwMapIdx];
                            if (pstUser->m_aucWildFlag[udwMapIdx] == EN_TABLE_UPDT_FLAG__UNCHANGE)
                            {
                                pstUser->m_aucWildFlag[udwMapIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
                            }
                        }
                        if (ptbWild != NULL)
                        {
                            CCommonBase::UpdateOccupyLairStat(pstUser, ptbWild, pstUser->m_atbMarch[udwIdx].m_nId);
                        }
                    }
                }
                if (pstUser->m_atbMarch[udwIdx].m_nStatus == EN_MARCH_STATUS__LOADING)
                {
                    pstUser->m_atbMarch[udwIdx].Set_Etime(udwCurTime);
                    pstUser->m_aucMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
                    continue;
                }
            }
        }

        //让AU删除自己发起的普通rallywar
        if (CActionBase::IsPlayerOwnedAction(ddwSrcUid, pstUser->m_atbMarch[udwIdx].m_nId)
            && pstUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR
            && pstUser->m_atbMarch[udwIdx].m_nStatus != EN_MARCH_STATUS__RETURNING)
        {
            TbMarch_action* ptbRallyWar = &pstUser->m_atbMarch[udwIdx];
            for (TUINT32 udwReinIdx = 0; udwReinIdx < pstUser->m_udwPassiveMarchNum; udwReinIdx++)
            {
                TbMarch_action* ptbRallyReinforce = &pstUser->m_atbPassiveMarch[udwReinIdx];
                if (!CActionBase::IsPlayerOwnedAction(ddwSrcUid, ptbRallyReinforce->m_nId)
                    && ptbRallyReinforce->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_REINFORCE
                    && ptbRallyReinforce->m_nStatus != EN_MARCH_STATUS__RETURNING
                    && ptbRallyReinforce->m_nTid == ptbRallyWar->m_nId)
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

            if (ptbRallyWar->m_nStatus == EN_MARCH_STATUS__MARCHING)
            {
                ptbRallyWar->Set_Status(EN_MARCH_STATUS__DELING_ON_FLY);
            }
            else
            {
                ptbRallyWar->Set_Status(EN_MARCH_STATUS__DELING);
            }
            ptbRallyWar->m_bParam[0].m_ddwLoadTime = ptbRallyWar->m_nEtime;
            ptbRallyWar->SetFlag(TbMARCH_ACTION_FIELD_PARAM);
            ptbRallyWar->Set_Etime(udwCurTime);
            ptbRallyWar->Set_Sal(0, UPDATE_ACTION_TYPE_DELETE);
            ptbRallyWar->Set_Tal(0, UPDATE_ACTION_TYPE_DELETE);
            //ptbRallyWar->DeleteField(TbMARCH_ACTION_FIELD_SAL);
            //ptbRallyWar->DeleteField(TbMARCH_ACTION_FIELD_TAL);
            pstUser->m_aucMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
            continue;
        }

        //自己的对盟友rallywar的支援(普通/王座)
        if (CActionBase::IsPlayerOwnedAction(ddwSrcUid, pstUser->m_atbMarch[udwIdx].m_nId)
            && pstUser->m_atbMarch[udwIdx].m_nMclass == EN_ACTION_MAIN_CLASS__MARCH
            && pstUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__RALLY_REINFORCE
            && pstUser->m_atbMarch[udwIdx].m_nStatus != EN_MARCH_STATUS__RETURNING)
        {
            TbMarch_action* ptbRallyReinforce = &pstUser->m_atbMarch[udwIdx];
            TbMarch_action* ptbRallyWar = NULL;
            TINT32 dwRallyWarIndex = -1;
            dwRallyWarIndex = CActionBase::GetMarchIndex(pstUser->m_atbMarch, pstUser->m_udwMarchNum, ptbRallyReinforce->m_nTid);
            if (dwRallyWarIndex >= 0)
            {
                ptbRallyWar = &pstUser->m_atbMarch[dwRallyWarIndex];
                pstUser->m_aucMarchFlag[dwRallyWarIndex] = EN_TABLE_UPDT_FLAG__CHANGE;

                if (ptbRallyWar->m_nStatus == EN_MARCH_STATUS__WAITING
                    && ptbRallyReinforce->m_nStatus == EN_MARCH_STATUS__MARCHING)
                {
                    ptbRallyWar->Set_Etime(udwCurTime);
                }

                CActionBase::ReleaseSlot(ptbRallyWar, ptbRallyReinforce, TRUE);
                CActionBase::UpdateRallyForce(ptbRallyWar, ptbRallyReinforce, TRUE);
            }

            if (ptbRallyReinforce->m_nStatus == EN_MARCH_STATUS__PREPARING
                || ptbRallyReinforce->m_nStatus == EN_MARCH_STATUS__DEFENDING) //为了处理某个bug遗留的无法召回的action
            {
                CActionBase::ReturnMarch(ptbRallyReinforce);
            }
            else if (ptbRallyReinforce->m_nStatus == EN_MARCH_STATUS__MARCHING)
            {
                CActionBase::ReturnMarchOnFly(ptbRallyReinforce);
            }

            pstUser->m_aucMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
            continue;
        }


        //召回王座相关的reinforce
        if (CActionBase::IsPlayerOwnedAction(ddwSrcUid, pstUser->m_atbMarch[udwIdx].m_nId)
            && pstUser->m_atbMarch[udwIdx].m_nMclass == EN_ACTION_MAIN_CLASS__MARCH
            && pstUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_THRONE
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
            continue;
        }

        //取消对王座的rally_war
        if (CActionBase::IsPlayerOwnedAction(ddwSrcUid, pstUser->m_atbMarch[udwIdx].m_nId)
            && pstUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR_THRONE
            && pstUser->m_atbMarch[udwIdx].m_nStatus != EN_MARCH_STATUS__RETURNING)
        {
            TbMarch_action* ptbRallyWar = &pstUser->m_atbMarch[udwIdx];
            for (TUINT32 udwReinIdx = 0; udwReinIdx < pstUser->m_udwPassiveMarchNum; udwReinIdx++)
            {
                TbMarch_action* ptbRallyReinforce = &pstUser->m_atbPassiveMarch[udwReinIdx];
                if (!CActionBase::IsPlayerOwnedAction(ddwSrcUid, ptbRallyReinforce->m_nId)
                    && ptbRallyReinforce->m_nSclass == EN_ACTION_SEC_CLASS__RALLY_REINFORCE
                    && ptbRallyReinforce->m_nStatus != EN_MARCH_STATUS__RETURNING
                    && ptbRallyReinforce->m_nTid == ptbRallyWar->m_nId)
                {
                    if (ptbRallyReinforce->m_nStatus == EN_MARCH_STATUS__PREPARING)
                    {
                        CActionBase::ReturnMarch(ptbRallyReinforce);
                    }
                    else if (ptbRallyReinforce->m_nStatus == EN_MARCH_STATUS__MARCHING
                        || ptbRallyReinforce->m_nStatus == EN_MARCH_STATUS__FIGHTING)
                    {
                        CActionBase::ReturnMarchOnFly(ptbRallyReinforce);
                    }
                    pstUser->m_aucPassiveMarchFlag[udwReinIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
                }
            }

            if (ptbRallyWar->m_nStatus == EN_MARCH_STATUS__MARCHING)
            {
                ptbRallyWar->Set_Status(EN_MARCH_STATUS__DELING_ON_FLY);
            }
            else
            {
                ptbRallyWar->Set_Status(EN_MARCH_STATUS__DELING);
            }
            ptbRallyWar->m_bParam[0].m_ddwLoadTime = ptbRallyWar->m_nEtime;
            ptbRallyWar->SetFlag(TbMARCH_ACTION_FIELD_PARAM);
            ptbRallyWar->Set_Etime(udwCurTime);
            ptbRallyWar->Set_Sal(0, UPDATE_ACTION_TYPE_DELETE);
            ptbRallyWar->Set_Tal(0, UPDATE_ACTION_TYPE_DELETE);
            //ptbRallyWar->DeleteField(TbMARCH_ACTION_FIELD_SAL);
            //ptbRallyWar->DeleteField(TbMARCH_ACTION_FIELD_TAL);
            pstUser->m_aucMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
            continue;
        }

        //召回对盟友的王座rally_war的reinforce
//         if (CActionBase::IsPlayerOwnedAction(ddwSrcUid, pstUser->m_atbMarch[udwIdx].m_nId)
//             && pstUser->m_atbMarch[udwIdx].m_nMclass == EN_ACTION_MAIN_CLASS__MARCH
//             && pstUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_THRONE
//             && pstUser->m_atbMarch[udwIdx].m_nTid != 0
//             && pstUser->m_atbMarch[udwIdx].m_nStatus != EN_MARCH_STATUS__RETURNING)
//         {
//             TbMarch_action* ptbRallyReinforce = &pstUser->m_atbMarch[udwIdx];
//             TbMarch_action* ptbRallyWar = NULL;
//             TINT32 dwRallyWarIndex = -1;
//             dwRallyWarIndex = CActionBase::GetMarchIndex(pstUser->m_atbMarch, pstUser->m_udwMarchNum, ptbRallyReinforce->m_nTid);
//             if (dwRallyWarIndex >= 0)
//             {
//                 ptbRallyWar = &pstUser->m_atbMarch[dwRallyWarIndex];
//                 pstUser->m_aucMarchFlag[dwRallyWarIndex] = EN_TABLE_UPDT_FLAG__CHANGE;
// 
//                 if (ptbRallyWar->m_nStatus == EN_MARCH_STATUS__WAITING
//                     && ptbRallyReinforce->m_nStatus == EN_MARCH_STATUS__MARCHING)
//                 {
//                     ptbRallyWar->Set_Etime(udwCurTime);
//                 }
// 
//                 CActionBase::ReleaseSlot(ptbRallyWar, ptbRallyReinforce, TRUE);
//                 CActionBase::UpdateRallyForce(ptbRallyWar, ptbRallyReinforce, TRUE);
//             }
// 
//             if (ptbRallyReinforce->m_nStatus == EN_MARCH_STATUS__PREPARING)
//             {
//                 CActionBase::ReturnMarch(ptbRallyReinforce);
//             }
//             else if (ptbRallyReinforce->m_nStatus == EN_MARCH_STATUS__MARCHING)
//             {
//                 CActionBase::ReturnMarchOnFly(ptbRallyReinforce);
//             }
// 
//             pstUser->m_aucMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
//             continue;
//         }

        //召回自己派遣在王座的英雄
        if (CActionBase::IsPlayerOwnedAction(ddwSrcUid, pstUser->m_atbMarch[udwIdx].m_nId)
            && pstUser->m_atbMarch[udwIdx].m_nMclass == EN_ACTION_MAIN_CLASS__MARCH
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
            continue;
        }

        //召回对王座/神像发起的普通进攻
        if (CActionBase::IsPlayerOwnedAction(ddwSrcUid, pstUser->m_atbMarch[udwIdx].m_nId)
            && pstUser->m_atbMarch[udwIdx].m_nMclass == EN_ACTION_MAIN_CLASS__MARCH
            && (pstUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__ATTACK_IDOL
            || pstUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__ATTACK_THRONE)
            && pstUser->m_atbMarch[udwIdx].m_nStatus != EN_MARCH_STATUS__RETURNING)
        {
            CActionBase::ReturnMarchOnFly(&pstUser->m_atbMarch[udwIdx]);
            pstUser->m_aucMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
            continue;
        }
    }

    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwPassiveMarchNum; udwIdx++)
    {
        //踢回盟友对自己城市的支援
        if (pstUser->m_atbPassiveMarch[udwIdx].m_nMclass == EN_ACTION_MAIN_CLASS__MARCH
            && pstUser->m_atbPassiveMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_NORMAL
            && pstUser->m_atbPassiveMarch[udwIdx].m_nStatus == EN_MARCH_STATUS__DEFENDING
            && pstUser->m_atbPassiveMarch[udwIdx].m_nTuid == ddwSrcUid)
        {
            CActionBase::ReturnMarch(&pstUser->m_atbPassiveMarch[udwIdx]);
            pstUser->m_aucPassiveMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
            continue;
        }

        if (pstUser->m_atbPassiveMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__RALLY_WAR
            && pstUser->m_atbPassiveMarch[udwIdx].m_nTuid == ddwSrcUid)
        {
            TbMarch_action* ptbRallyWar = &pstUser->m_atbPassiveMarch[udwIdx];
            ptbRallyWar->m_bParam[0].m_ddwTargetAlliance = 0;
            ptbRallyWar->m_bParam[0].m_szTargetAlliance[0] = '\0';
            ptbRallyWar->SetFlag(TbMARCH_ACTION_FIELD_PARAM);
            ptbRallyWar->Set_Tal(-1 * ptbRallyWar->m_nTuid);
            pstUser->m_aucPassiveMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
        }
    }

    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwSelfAlActionNum; udwIdx++)
    {
        if (!CActionBase::IsPlayerOwnedAction(ddwSrcUid, pstUser->m_atbSelfAlAction[udwIdx].m_nId))
        {
            continue;
        }
        //取消自己申请的联盟帮助
        if (pstUser->m_atbSelfAlAction[udwIdx].m_nSal != 0)
        {
            pstUser->m_atbSelfAlAction[udwIdx].Set_Sal(0, UPDATE_ACTION_TYPE_DELETE);
            //pstUser->m_atbSelfAlAction[udwIdx].DeleteField(TbMARCH_ACTION_FIELD_SAL);
            pstUser->m_aucSelfAlActionFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
        }
    }

    if (pstUser->m_tbPlayer.m_nUid == ddwSrcUid)
    {
        pstUser->m_udwAlCanHelpActionNum = 0;
    }
}

TbAction* CActionBase::GetActionByBufferId(TbAction* patbAction, TUINT32 udwActionNum, TUINT32 udwBufferId)
{
    TINT32 dwRetIdx = -1;
    for(TUINT32 udwIdx = 0; udwIdx < udwActionNum; udwIdx++)
    {
        if(patbAction[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__ITEM && patbAction[udwIdx].m_bParam[0].m_stItem.m_ddwBufferId == udwBufferId)
        {
            dwRetIdx = udwIdx;
            break;
        }
    }
    if(dwRetIdx != -1)
    {
        return &patbAction[dwRetIdx];
    }

    return NULL;
}

TBOOL CActionBase::IsPlayerOwnedAction(TINT64 ddwUserId, TINT64 ddwActionId)
{
    if((ddwActionId >> 32) == ddwUserId)
    {
        return TRUE;
    }
    return FALSE;
}

TINT32 CActionBase::GetUserBuffById(TbMarch_action *ptbMarch, TINT32 dwBuffId, TUINT32 udwTime/* = 0*/)
{
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    if (udwTime != 0)
    {
        udwCurTime = udwTime;
    }
    TINT32 dwNum = 0;
    for (TUINT32 udwIdx = 0; udwIdx < ptbMarch->m_bBuff.m_udwNum; ++udwIdx)
    {
        if (ptbMarch->m_bBuff[udwIdx].ddwBuffId == dwBuffId)
        {
            dwNum += ptbMarch->m_bBuff[udwIdx].ddwBuffNum;
        }
    }
    for (TUINT32 udwIdx = 0; udwIdx < ptbMarch->m_bExpiring_buff.m_udwNum; ++udwIdx)
    {
        if (ptbMarch->m_bExpiring_buff[udwIdx].ddwBuffId == dwBuffId
            && ptbMarch->m_bExpiring_buff[udwIdx].ddwBuffExpiredTime < static_cast<TINT32>(udwCurTime))
        {
            dwNum -= ptbMarch->m_bExpiring_buff[udwIdx].ddwBuffNum;
        }
    }
    if (dwNum < 0)
    {
        dwNum = 0;
    }

    return dwNum;
}

TINT64 CActionBase::GetActionTypeBySecondClass(TUINT32 udwSecondClass)
{
    TUINT32 udwActionType = 0;

    switch (udwSecondClass)
    {
    case EN_ACTION_SEC_CLASS__BUILDING_UPGRADE:
    case EN_ACTION_SEC_CLASS__BUILDING_REMOVE:
    case EN_ACTION_SEC_CLASS__RESEARCH_UPGRADE:
    case EN_ACTION_SEC_CLASS__EQUIP_UPGRADE:
    case EN_ACTION_SEC_CLASS__REMOVE_OBSTACLE:
    case EN_ACTION_SEC_CLASS__TROOP:
    case EN_ACTION_SEC_CLASS__FORT:
    case EN_ACTION_SEC_CLASS__HOS_TREAT:
    case EN_ACTION_SEC_CLASS__FORT_REPAIR:
    case EN_ACTION_SEC_CLASS__UNLOCK_DRAGON:
        udwActionType = EN_ACTION_TYPE_AL_CAN_HELP;
        break;
    case EN_ACTION_SEC_CLASS__ITEM:
    case EN_ACTION_SEC_CLASS__ATTACK_MOVE:
        udwActionType = EN_ACTION_TYPE_BUFF_NORMAL;
        break;
    case EN_ACTION_SEC_CLASS__TRANSPORT:
    case EN_ACTION_SEC_CLASS__REINFORCE_NORMAL:
    case EN_ACTION_SEC_CLASS__SCOUT:
    case EN_ACTION_SEC_CLASS__ATTACK:
    case EN_ACTION_SEC_CLASS__OCCUPY:
    case EN_ACTION_SEC_CLASS__CAMP:
    case EN_ACTION_SEC_CLASS__RALLY_WAR:
    case EN_ACTION_SEC_CLASS__RALLY_REINFORCE:
    case EN_ACTION_SEC_CLASS__DRAGON_ATTACK:
    case EN_ACTION_SEC_CLASS__CATCH_DRAGON:
    case EN_ACTION_SEC_CLASS__RELEASE_DRAGON:
    case EN_ACTION_SEC_CLASS__ATTACK_THRONE:
    case EN_ACTION_SEC_CLASS__RALLY_WAR_THRONE:
    case EN_ACTION_SEC_CLASS__REINFORCE_THRONE:
    case EN_ACTION_SEC_CLASS__ASSIGN_THRONE:
    case EN_ACTION_SEC_CLASS__ATTACK_IDOL:
        udwActionType = EN_ACTION_TYPE_MARCH;
        break;
    default:
        break;
    }

    return udwActionType;
}

TBOOL CActionBase::HasEmptySlot(TbMarch_action* ptbRallyWar, TINT64 ddwUid)
{
    SRallySlot* pstSlot = NULL;
    TUINT32 udwSlotNum = 0;
    
    pstSlot = &ptbRallyWar->m_bRally_atk_slot[0];
    udwSlotNum = ptbRallyWar->m_bRally_atk_slot.m_udwNum;

    TBOOL bHas = FALSE;
    for(TUINT32 udwIdx = 0; udwIdx < udwSlotNum; ++udwIdx)
    {
        if(pstSlot[udwIdx].ddwMarchId == 0 &&
            (pstSlot[udwIdx].ddwUid == 0 || pstSlot[udwIdx].ddwUid == ddwUid))
        {
            bHas = TRUE;
            break;
        }
    }
    return bHas;
}

TVOID CActionBase::HoldSlot(TbMarch_action* ptbRallyWar, TbMarch_action* ptbRallyReinforce)
{
    SRallySlot* pstSlot = NULL;
    TUINT32 udwSlotNum = 0;

    pstSlot = &ptbRallyWar->m_bRally_atk_slot[0];
    udwSlotNum = ptbRallyWar->m_bRally_atk_slot.m_udwNum;

    TINT32 dwSlotIndex = -1;
    for(TUINT32 udwIdx = 0; udwIdx < udwSlotNum; ++udwIdx)
    {
        if(pstSlot[udwIdx].ddwMarchId > 0)
        {
            continue;
        }
        if(pstSlot[udwIdx].ddwUid == ptbRallyReinforce->m_nSuid)
        {
            dwSlotIndex = udwIdx;
            break;
        }
        if(pstSlot[udwIdx].ddwUid == 0 && dwSlotIndex == -1)
        {
            dwSlotIndex = udwIdx;
            continue;
        }
    }

    if(dwSlotIndex != -1)
    {
        pstSlot[dwSlotIndex].ddwMarchId = ptbRallyReinforce->m_nId;
        pstSlot[dwSlotIndex].ddwUid = ptbRallyReinforce->m_nSuid;
        strncpy(pstSlot[dwSlotIndex].szUserName, ptbRallyReinforce->m_bParam[0].m_szSourceUserName, MAX_TABLE_NAME_LEN);
        pstSlot[dwSlotIndex].szUserName[MAX_TABLE_NAME_LEN - 1] = '\0';
        ptbRallyWar->SetFlag(TbMARCH_ACTION_FIELD_RALLY_ATK_SLOT);
    }
}

TVOID CActionBase::ReleaseSlot(TbMarch_action* ptbRallyWar, TbMarch_action* ptbRallyReinforce, TBOOL bForceRelease)
{
    SRallySlot* pstSlot = NULL;
    TUINT32 udwSlotNum = 0;
    
    pstSlot = &ptbRallyWar->m_bRally_atk_slot[0];
    udwSlotNum = ptbRallyWar->m_bRally_atk_slot.m_udwNum;

    for(TUINT32 udwIdx = 0; udwIdx < udwSlotNum; ++udwIdx)
    {
        if(pstSlot[udwIdx].ddwMarchId == ptbRallyReinforce->m_nId)
        {
            //wave@20160127: release slot时需要考虑私人槽位问题
            if(pstSlot[udwIdx].bPrivate == TRUE)
            {
                pstSlot[udwIdx].ddwMarchId = 0;
            }
            else
            {
                pstSlot[udwIdx].Reset();
                strncpy(pstSlot[udwIdx].szUserName, "empty", MAX_TABLE_NAME_LEN); // jonathan: 填些东西防止action_svr把字段清空
            }
            
            ptbRallyWar->SetFlag(TbMARCH_ACTION_FIELD_RALLY_ATK_SLOT);
            break;
        }
    }
}

TVOID CActionBase::AddSlot(TbMarch_action* ptbRallyWar, TINT64 ddwUid, const string& strUserName)
{
    SRallySlot* pstSlot = NULL;
    TUINT32 udwSlotNum = 0;

    pstSlot = &ptbRallyWar->m_bRally_atk_slot[0];
    udwSlotNum = ptbRallyWar->m_bRally_atk_slot.m_udwNum;

    pstSlot[udwSlotNum].Reset();
    strncpy(pstSlot[udwSlotNum].szUserName, "empty", MAX_TABLE_NAME_LEN);
    pstSlot[udwSlotNum].ddwUid = ddwUid;
    if(ddwUid > 0)
    {
        pstSlot[udwSlotNum].bPrivate = TRUE;
        strncpy(pstSlot[udwSlotNum].szUserName, strUserName.c_str(), MAX_TABLE_NAME_LEN);
        pstSlot[udwSlotNum].szUserName[MAX_TABLE_NAME_LEN - 1] = '\0';
    }

    ptbRallyWar->m_bRally_atk_slot.m_udwNum++;
    ptbRallyWar->SetFlag(TbMARCH_ACTION_FIELD_RALLY_ATK_SLOT);
}

TBOOL CActionBase::HasPrivateSlot(TbMarch_action* ptbRallyWar, TINT64 ddwUid)
{
    TUINT32 udwSlotNum = 0;
    SRallySlot* pstSlot = NULL;
    
    pstSlot = &ptbRallyWar->m_bRally_atk_slot[0];
    udwSlotNum = ptbRallyWar->m_bRally_atk_slot.m_udwNum;

    for(TUINT32 udwIdx = 0; udwIdx < udwSlotNum; ++udwIdx)
    {
        if(pstSlot[udwIdx].bPrivate)
        {
            if(pstSlot[udwIdx].ddwUid == ddwUid)
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

TVOID CActionBase::UpdateRallyForce(TbMarch_action* ptbRallyWar, TbMarch_action* ptbRallyReinforce, TBOOL bRecall /*= FALSE*/)
{
    SRallyForce* pstRallyForce = NULL;
    
    pstRallyForce = &ptbRallyWar->m_bRally_atk_force[0];

    if(bRecall)
    {
        pstRallyForce->ddwReinforceNum -= ptbRallyReinforce->m_bParam[0].m_ddwTroopNum;
        pstRallyForce->ddwReinforceForce -= ptbRallyReinforce->m_bParam[0].m_ddwForce;
    }
    else
    {
        pstRallyForce->ddwReinforceNum += ptbRallyReinforce->m_bParam[0].m_ddwTroopNum;
        pstRallyForce->ddwReinforceForce += ptbRallyReinforce->m_bParam[0].m_ddwForce;
    }

    if(pstRallyForce->ddwReinforceNum < 0)
    {
        pstRallyForce->ddwReinforceNum = 0;
    }
    if(pstRallyForce->ddwReinforceForce < 0)
    {
        pstRallyForce->ddwReinforceForce = 0;
    }

    pstRallyForce->ddwTotalNum = ptbRallyWar->m_bParam[0].m_ddwTroopNum + pstRallyForce->ddwReinforceNum;
    pstRallyForce->ddwTotalForce = ptbRallyWar->m_bParam[0].m_ddwForce + pstRallyForce->ddwReinforceForce;
    ptbRallyWar->SetFlag(TbMARCH_ACTION_FIELD_RALLY_ATK_FORCE);

    if (bRecall)
    {
        for (TUINT32 udwIdx = 0; udwIdx < EN_TROOP_TYPE__END; ++udwIdx)
        {
            if (ptbRallyWar->m_bAtk_total_troop[0].m_addwNum[udwIdx] >= ptbRallyReinforce->m_bParam[0].m_stTroop.m_addwNum[udwIdx])
            {
                ptbRallyWar->m_bAtk_total_troop[0].m_addwNum[udwIdx] -= ptbRallyReinforce->m_bParam[0].m_stTroop.m_addwNum[udwIdx];
            }
            else
            {
                ptbRallyWar->m_bAtk_total_troop[0].m_addwNum[udwIdx] = 0;
            }
        }
    }
    else
    {
        for (TUINT32 udwIdx = 0; udwIdx < EN_TROOP_TYPE__END; ++udwIdx)
        {
            ptbRallyWar->m_bAtk_total_troop[0].m_addwNum[udwIdx] += ptbRallyReinforce->m_bParam[0].m_stTroop.m_addwNum[udwIdx];
        }
    }
    ptbRallyWar->SetFlag(TbMARCH_ACTION_FIELD_ATK_TOTAL_TROOP);
}

TBOOL CActionBase::CanRallyReinforce(TbMarch_action* ptbRallyWar, const TCHAR* pszTroop)
{
    SCommonTroop stTroop;
    stTroop.Reset();
    TUINT32 udwTroopNum = EN_TROOP_TYPE__END;
    CCommonFunc::GetArrayFromString(pszTroop, ':', &stTroop.m_addwNum[0], udwTroopNum);

    udwTroopNum = CToolBase::GetTroopSumNum(stTroop);

    SRallyForce* pstRallyForce = NULL;

    pstRallyForce = &ptbRallyWar->m_bRally_atk_force[0];

    if(udwTroopNum + pstRallyForce->ddwReinforceNum > pstRallyForce->ddwReinforceTroopLimit)
    {
        return FALSE;
    }

    return TRUE;
}

TBOOL CActionBase::IsEmptyMarch(TbMarch_action* ptbMarch)
{
    TBOOL bEmpty = TRUE;
    for(TUINT32 udwIdx = 0; udwIdx < EN_TROOP_TYPE__END; ++udwIdx)
    {
        if(ptbMarch->m_bParam[0].m_stTroop[udwIdx] > 0)
        {
            bEmpty = FALSE;
            break;
        }
    }

    if(ptbMarch->m_bParam[0].m_stDragon.m_ddwLevel > 0)
    {
        bEmpty = FALSE;
    }

    if(ptbMarch->m_bPrison_param[0].stDragon.m_ddwLevel > 0)
    {
        bEmpty = FALSE;
    }

    if (ptbMarch->m_bParam[0].m_stKnight.ddwLevel > 0)
    {
        bEmpty = FALSE;
    }
    return bEmpty;
}

TBOOL CActionBase::IsMarchHasTroop(TbMarch_action* ptbMarch)
{
    TBOOL bHas = FALSE;
    for(TUINT32 udwIdx = 0; udwIdx < EN_TROOP_TYPE__END; ++udwIdx)
    {
        if(ptbMarch->m_bParam[0].m_stTroop[udwIdx] > 0)
        {
            bHas = TRUE;
            break;
        }
    }

    return bHas;
}

TVOID CActionBase::DeleteMarch(TbMarch_action *ptbMarch)
{
    ptbMarch->Set_Status(EN_MARCH_STATUS__RETURNING);
    ptbMarch->Set_Etime(CTimeUtils::GetUnixTime());
    ptbMarch->Set_Sal(0, UPDATE_ACTION_TYPE_DELETE);
    ptbMarch->Set_Tal(0, UPDATE_ACTION_TYPE_DELETE);
    //ptbMarch->DeleteField(TbMARCH_ACTION_FIELD_SAL);
    //ptbMarch->DeleteField(TbMARCH_ACTION_FIELD_TAL);
}

TVOID CActionBase::ReturnMarch(TbMarch_action *ptbMarch, TINT64 ddwWildPos/* = 0*/, TINT64 ddwWildType/* = -1*/)
{
    ptbMarch->Set_Status(EN_MARCH_STATUS__RETURNING);
    ptbMarch->Set_Btime(CTimeUtils::GetUnixTime());
    ptbMarch->Set_Ctime(ptbMarch->m_bParam[0].m_ddwMarchingTime);
    ptbMarch->Set_Etime(ptbMarch->m_nBtime + ptbMarch->m_nCtime);

    if(ddwWildPos != 0)
    {
        TINT64 ddwCtime = CToolBase::GetMarchTime(ptbMarch, ddwWildPos);
        ptbMarch->Set_Tpos(ddwWildPos);
        ptbMarch->Set_Tbid(CMapBase::GetBlockIdFromPos(ddwWildPos));
        ptbMarch->Set_Ctime(ddwCtime);
        ptbMarch->Set_Etime(ptbMarch->m_nBtime + ddwCtime);
    }

    if (ptbMarch->m_bParam[0].m_ddwTargetType != ddwWildType && ddwWildType != -1)
    {
        ptbMarch->m_bParam[0].m_ddwTargetType = ddwWildType;
        ptbMarch->SetFlag(TbMARCH_ACTION_FIELD_PARAM);
    }
    ptbMarch->Set_Sal(0, UPDATE_ACTION_TYPE_DELETE);
    ptbMarch->Set_Tal(0, UPDATE_ACTION_TYPE_DELETE);
    ptbMarch->Set_Tid(0, UPDATE_ACTION_TYPE_DELETE);

}

TVOID CActionBase::ReturnMarchOnFly(TbMarch_action *ptbMarch)
{
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    ptbMarch->Set_Status(EN_MARCH_STATUS__RETURNING);
    ptbMarch->Set_Btime(udwCurTime);
    ptbMarch->Set_Ctime(ptbMarch->m_bParam[0].m_ddwMarchingTime);
    ptbMarch->Set_Etime(ptbMarch->m_nCtime + 2 * udwCurTime - ptbMarch->m_nEtime);

    ptbMarch->Set_Sal(0, UPDATE_ACTION_TYPE_DELETE);
    ptbMarch->Set_Tal(0, UPDATE_ACTION_TYPE_DELETE);
    ptbMarch->Set_Tid(0, UPDATE_ACTION_TYPE_DELETE);
}

TVOID CActionBase::ReinforceToThrone(TbMarch_action* ptbMarch, TINT64 ddwWildPos, TINT64 ddwWildType)
{
    if (ddwWildPos != ptbMarch->m_nTpos)
    {
        TINT64 ddwCtime = CToolBase::GetMarchTime(ptbMarch, ddwWildPos);
        ptbMarch->m_bParam[0].m_ddwMarchingTime = ddwCtime;
        ptbMarch->SetFlag(TbMARCH_ACTION_FIELD_PARAM);
    }

    ptbMarch->Set_Sclass(EN_ACTION_SEC_CLASS__REINFORCE_THRONE);
    ptbMarch->Set_Status(EN_MARCH_STATUS__DEFENDING);
    ptbMarch->Set_Btime(CTimeUtils::GetUnixTime());
    ptbMarch->Set_Etime(INT64_MAX);
    ptbMarch->Set_Tpos(ddwWildPos);
    ptbMarch->Set_Tbid(CMapBase::GetBlockIdFromPos(ddwWildPos));

    ptbMarch->Set_Sal(ptbMarch->m_bParam[0].m_ddwSourceAlliance);
    ptbMarch->Set_Tid(0, UPDATE_ACTION_TYPE_DELETE);
    ptbMarch->Set_Tuid(0, UPDATE_ACTION_TYPE_DELETE);
    ptbMarch->Set_Tal(0, UPDATE_ACTION_TYPE_DELETE);
    if (ptbMarch->m_bParam[0].m_ddwTargetType != ddwWildType)
    {
        ptbMarch->m_bParam[0].m_ddwTargetType = ddwWildType;
        ptbMarch->SetFlag(TbMARCH_ACTION_FIELD_PARAM);
    }
}

TVOID CActionBase::RallyWarToThroneAssign(TbMarch_action* ptbMarch)
{
    ptbMarch->Set_Sclass(EN_ACTION_SEC_CLASS__ASSIGN_THRONE);
    ptbMarch->Set_Status(EN_MARCH_STATUS__DEFENDING);
    ptbMarch->Set_Btime(CTimeUtils::GetUnixTime());
    ptbMarch->Set_Etime(INT64_MAX);

    ptbMarch->Set_Sal(ptbMarch->m_bParam[0].m_ddwSourceAlliance);

    ptbMarch->Set_Tal(0, UPDATE_ACTION_TYPE_DELETE);
    ptbMarch->Set_Tid(0, UPDATE_ACTION_TYPE_DELETE);
    ptbMarch->Set_Tuid(0, UPDATE_ACTION_TYPE_DELETE);
}

TVOID CActionBase::PrisonToMarch(TbMarch_action* ptbPrisonTimer, TUINT32 udwScid)
{
    ptbPrisonTimer->Set_Mclass(EN_ACTION_MAIN_CLASS__MARCH);
    ptbPrisonTimer->Set_Sclass(EN_ACTION_SEC_CLASS__RELEASE_DRAGON);
    ptbPrisonTimer->Set_Status(EN_MARCH_STATUS__RETURNING);
    ptbPrisonTimer->Set_Btime(CTimeUtils::GetUnixTime());
    ptbPrisonTimer->Set_Etime(ptbPrisonTimer->m_nBtime + ptbPrisonTimer->m_nCtime);
    ptbPrisonTimer->Set_Tal(0, UPDATE_ACTION_TYPE_DELETE);
    //ptbPrisonTimer->DeleteField(TbMARCH_ACTION_FIELD_TAL);

    if (udwScid && ptbPrisonTimer->m_nScid != udwScid)
    {
        ptbPrisonTimer->Set_Scid(udwScid);
    }

    ptbPrisonTimer->Set_Sbid(CMapBase::GetBlockIdFromPos(ptbPrisonTimer->m_nScid));
    ptbPrisonTimer->Set_Tbid(CMapBase::GetBlockIdFromPos(ptbPrisonTimer->m_nTpos));
}

TINT32 CActionBase::CheckSeq(SUserInfo* pstUser)
{
    if(pstUser->m_tbLogin.m_nUid == 0)
    {
        return 0;
    }

    TINT64 ddwBiggestActionId = 0;

    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwActionNum; ++udwIdx)
    {
        if(!CActionBase::IsPlayerOwnedAction(pstUser->m_tbLogin.m_nUid, pstUser->m_atbAction[udwIdx].m_nId))
        {
            continue;
        }
        if(pstUser->m_atbAction[udwIdx].m_nId > ddwBiggestActionId)
        {
            ddwBiggestActionId = pstUser->m_atbAction[udwIdx].m_nId;
        }
    }

    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
    {
        if(!CActionBase::IsPlayerOwnedAction(pstUser->m_tbLogin.m_nUid, pstUser->m_atbMarch[udwIdx].m_nId))
        {
            continue;
        }
        if(pstUser->m_atbMarch[udwIdx].m_nId > ddwBiggestActionId)
        {
            ddwBiggestActionId = pstUser->m_atbMarch[udwIdx].m_nId;
        }
    }

    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwSelfAlActionNum; ++udwIdx)
    {
        if(!CActionBase::IsPlayerOwnedAction(pstUser->m_tbLogin.m_nUid, pstUser->m_atbSelfAlAction[udwIdx].m_nId))
        {
            continue;
        }
        if(pstUser->m_atbSelfAlAction[udwIdx].m_nId > ddwBiggestActionId)
        {
            ddwBiggestActionId = pstUser->m_atbSelfAlAction[udwIdx].m_nId;
        }
    }

    TINT64 ddwActionSeq = (ddwBiggestActionId << 32) >> 32;
    if(ddwActionSeq >= pstUser->m_tbLogin.m_nSeq)
    {
        pstUser->m_tbLogin.Set_Seq(ddwActionSeq + 10);
    }

    return 0;
}

TVOID CActionBase::SyncDragonStatus(SUserInfo* pstUser)
{
    if(pstUser->m_tbPlayer.m_nUid == 0)
    {
        return;
    }

    TbMarch_action* ptbPrison = NULL;
    TbMarch_action* ptbMarch = NULL;
    TBOOL bBeCaptured = FALSE;
    TBOOL bInMarch = FALSE;
    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
    {
        if(pstUser->m_aucMarchFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }
        if(!CActionBase::IsPlayerOwnedAction(pstUser->m_tbPlayer.m_nUid, pstUser->m_atbMarch[udwIdx].m_nId))
        {
            continue;
        }
        if(pstUser->m_atbMarch[udwIdx].m_nSclass == EN_ACTION_SEC_CLASS__PRISON_TIMER)
        {
            ptbPrison = &pstUser->m_atbMarch[udwIdx];
            bBeCaptured = TRUE;
            continue;
        }
        if(pstUser->m_atbMarch[udwIdx].m_bParam[0].m_stDragon.m_ddwLevel > 0)
        {
            ptbMarch = &pstUser->m_atbMarch[udwIdx];
            bInMarch = TRUE;
            continue;
        }
        if(pstUser->m_atbMarch[udwIdx].m_bPrison_param[0].stDragon.m_ddwLevel > 0)
        {
            ptbMarch = &pstUser->m_atbMarch[udwIdx];
            bInMarch = TRUE;
            continue;
        }
    }
    if(bBeCaptured == TRUE)
    {
        TUINT32 udwDragonStatus = EN_DRAGON_STATUS_WAIT_KILL;
        if(ptbPrison->m_bPrison_param[0].ddwEscortActionId > 0)
        {
            udwDragonStatus = EN_DRAGON_STATUS_BEING_ESCORT;
        }
        else if (CTimeUtils::GetUnixTime() > ptbPrison->m_nEtime - ptbPrison->m_bPrison_param[0].ddwReleaseWait + ptbPrison->m_bPrison_param[0].ddwExcuteWait
            || ptbPrison->m_bPrison_param[0].stDragon.m_ddwCaptured != 0)
        {
            udwDragonStatus = EN_DRAGON_STATUS_WAIT_RELEASE;
        }
        if(pstUser->m_tbPlayer.m_nDragon_status != udwDragonStatus)
        {
            pstUser->m_tbPlayer.Set_Dragon_status(udwDragonStatus);
        }
        if(pstUser->m_tbPlayer.m_nDragon_tid != 0)
        {
            pstUser->m_tbPlayer.Set_Dragon_tid(0);
        }
    }
    else if(bInMarch == TRUE)
    {
        if (pstUser->m_tbPlayer.m_nDragon_status != EN_DRAGON_STATUS_MARCH
            && pstUser->m_tbPlayer.m_nDragon_status != EN_DRAGON_STATUS_DEAD)
        {
            pstUser->m_tbPlayer.Set_Dragon_status(EN_DRAGON_STATUS_MARCH);
            pstUser->m_tbPlayer.Set_Dragon_tid(ptbMarch->m_nId);
        }
    }
    else if(bBeCaptured == FALSE && bInMarch == FALSE)
    {
        if (pstUser->m_tbPlayer.m_nDragon_status == EN_DRAGON_STATUS_WAIT_RELEASE
            || pstUser->m_tbPlayer.m_nDragon_status == EN_DRAGON_STATUS_WAIT_KILL
            || pstUser->m_tbPlayer.m_nDragon_status == EN_DRAGON_STATUS_BEING_ESCORT
            || pstUser->m_tbPlayer.m_nDragon_status == EN_DRAGON_STATUS_MARCH)
        {
            pstUser->m_tbPlayer.Set_Dragon_status(EN_DRAGON_STATUS_NORMAL);
            pstUser->m_tbPlayer.Set_Dragon_tid(0);
        }
     }
}

TbMarch_action* CActionBase::AddNewMarch(SUserInfo* pstUser)
{
    if(pstUser->m_udwMarchNum >=  MAX_USER_MARCH_NUM)
    {
        return NULL;
    }

    TbMarch_action* ptbNewMarch = &pstUser->m_atbMarch[pstUser->m_udwMarchNum];
    pstUser->m_aucMarchFlag[pstUser->m_udwMarchNum] = EN_TABLE_UPDT_FLAG__NEW;
    pstUser->m_udwMarchNum++;

    ptbNewMarch->Set_Id(CToolBase::GetPlayerNewTaskId(pstUser->m_tbPlayer.m_nUid, pstUser->m_tbLogin.m_nSeq));
    pstUser->m_tbLogin.Set_Seq(pstUser->m_tbLogin.m_nSeq + 1);
    pstUser->m_uddwNewActionId = ptbNewMarch->m_nId;

    return ptbNewMarch;
}

TbMarch_action* CActionBase::GenTaxAction(SUserInfo *pstUser, TINT64 ddwThronePos)
{
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;
    TbMarch_action *ptbTax = AddNewMarch(pstUser);

    if (!ptbTax)
    {
        return NULL;
    }

    ptbTax->Set_Sid(ptbPlayer->m_nSid);
    ptbTax->Set_Suid(ptbPlayer->m_nUid);
    ptbTax->Set_Scid(pstCity->m_stTblData.m_nPos);
    ptbTax->Set_Mclass(EN_ACTION_MAIN_CLASS__TIMER);
    ptbTax->Set_Sclass(EN_ACTION_SEC_CLASS__TAX);
    ptbTax->Set_Status(EN_TAX_STATUS__PREPARING);
    ptbTax->Set_Ctime(CCommonBase::GetGameBasicVal(EN_GAME_BASIC_TAX_INTERVAL));
    ptbTax->Set_Btime(CTimeUtils::GetUnixTime() + ptbTax->m_nCtime);
    ptbTax->Set_Etime(ptbTax->m_nBtime);
    ptbTax->Set_Tpos(ddwThronePos);

    return ptbTax;
}

TINT32 CActionBase::GetActionDisplayClass( TINT32 dwMainClass )
{
    TINT32 dwDisplayClass = EN_ACTION_DISPLAY_CLASS_NONE;
    
    if (dwMainClass == EN_ACTION_MAIN_CLASS__BUILDING
        || dwMainClass == EN_ACTION_MAIN_CLASS__EQUIP
        || dwMainClass == EN_ACTION_MAIN_CLASS__DRAGON)
    {
        dwDisplayClass = EN_ACTION_DISPLAY_CLASS_BUILD;
    }
    else if (dwMainClass == EN_ACTION_MAIN_CLASS__TRAIN_NEW)
    {
        dwDisplayClass = EN_ACTION_DISPLAY_CLASS_TRAIN;
    }
    else if (dwMainClass == EN_ACTION_MAIN_CLASS__MARCH)
    {
        dwDisplayClass = EN_ACTION_DISPLAY_CLASS_MARCH;
    }

    return dwDisplayClass;
}

TVOID CActionBase::ResetRallyDefence(TbMarch_action *ptbRally, TbCity *ptbCity)
{
    for (TUINT32 udwIdx = 0; udwIdx < EN_TROOP_TYPE__END; ++udwIdx)
    {
        ptbRally->m_bDef_total_troop[0].m_addwNum[udwIdx] = ptbCity->m_bTroop[0].m_addwNum[udwIdx];
    }

    for (TUINT32 udwIdx = 0; udwIdx < ptbRally->m_bRally_def_slot.m_udwNum; ++udwIdx)
    {
        ptbRally->m_bRally_def_slot[udwIdx].Reset();
        strncpy(ptbRally->m_bRally_def_slot[udwIdx].szUserName, "empty", MAX_TABLE_NAME_LEN);
        ptbRally->m_bRally_def_slot[udwIdx].szUserName[MAX_TABLE_NAME_LEN - 1] = '\0';
    }
}

TBOOL CActionBase::RallyInfoAddDefence(TbMarch_action *ptbRally, TbMarch_action *ptbDefence)
{
    for (TUINT32 udwIdx = 0; udwIdx < ptbRally->m_bRally_def_slot.m_udwNum; ++udwIdx)
    {
        if (ptbRally->m_bRally_def_slot[udwIdx].ddwUid == 0)
        {
            for (TUINT32 udwIdy = 0; udwIdy < EN_TROOP_TYPE__END; ++udwIdy)
            {
                ptbRally->m_bDef_total_troop[0].m_addwNum[udwIdy] += ptbDefence->m_bParam[0].m_stTroop.m_addwNum[udwIdy];
            }
            ptbRally->m_bRally_def_slot[udwIdx].ddwUid = ptbDefence->m_nSuid;
            ptbRally->m_bRally_def_slot[udwIdx].ddwMarchId = ptbDefence->m_nId;
            strncpy(ptbRally->m_bRally_def_slot[udwIdx].szUserName, ptbDefence->m_bParam[0].m_szSourceUserName, MAX_TABLE_NAME_LEN);
            ptbRally->m_bRally_def_slot[udwIdx].szUserName[MAX_TABLE_NAME_LEN - 1] = '\0';
            return TRUE;
        }
    }

    return FALSE;
}

TVOID CActionBase::UpdtPassiveActionWhenAbandonWild( SUserInfo *pstUser, TbMap *pstWild )
{
    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwPassiveMarchNum; udwIdx++)
    {
        //踢回盟友对自己城市的支援
        if (pstUser->m_atbPassiveMarch[udwIdx].m_nMclass == EN_ACTION_MAIN_CLASS__MARCH
            && pstUser->m_atbPassiveMarch[udwIdx].m_nStatus != EN_MARCH_STATUS__RETURNING
            && pstUser->m_atbPassiveMarch[udwIdx].m_nTpos == pstWild->m_nId
            && pstUser->m_atbPassiveMarch[udwIdx].m_nTal < 0
            && pstUser->m_atbPassiveMarch[udwIdx].m_nTuid == pstUser->m_tbPlayer.m_nUid)
        {
            pstUser->m_atbPassiveMarch[udwIdx].Set_Tal(0, UPDATE_ACTION_TYPE_DELETE);
            pstUser->m_aucPassiveMarchFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
            continue;
        }
    }
}

TINT64 CActionBase::GenMapActionId(TINT64 ddwSvrId, TINT64 ddwPos)
{
    return -1 * (ddwSvrId * 1000 * 10000 + ddwPos);
}

TINT64 CActionBase::GenThroneTargetId(TINT64 ddwSvrId, TINT64 ddwPos)
{
    return (1L << 36) + (ddwSvrId * 1000 * 10000 + ddwPos);
}
