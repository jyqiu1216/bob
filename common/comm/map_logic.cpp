#include "map_logic.h"
#include "wild_info.h"
#include "common_func.h"
#include "tool_base.h"
#include "common_base.h"
#include <math.h>
#include "map_base.h"
#include "common_logic.h"
#include "common_logic.h"
#include "city_base.h"
#include "buffer_base.h"
#include "globalres_logic.h"

TBOOL CMapLogic::IsOccupyWild(TINT32 dwSvrId, TINT32 dwWildType)
{
    const Json::Value& oWildResJson = CWildInfo::GetWildResInfo(dwSvrId);
    if (!oWildResJson.isMember(CCommonFunc::NumToString(dwWildType)))
    {
        return FALSE;
    }
    if (oWildResJson[CCommonFunc::NumToString(dwWildType)]["a0"]["a1"].asInt() == 0)
    {
        return TRUE;
    }
    return FALSE;
}

TBOOL CMapLogic::IsAttackWild(TINT32 dwSvrId, TINT32 dwWildType)
{
    const Json::Value& oWildResJson = CWildInfo::GetWildResInfo(dwSvrId);
    if (!oWildResJson.isMember(CCommonFunc::NumToString(dwWildType)))
    {
        return FALSE;
    }
    if (oWildResJson[CCommonFunc::NumToString(dwWildType)]["a0"]["a1"].asInt() == 1)
    {
        return TRUE;
    }
    return FALSE;
}

TUINT32 CMapLogic::GetMapResTotalNum(TbMap *ptbWild)
{
    const Json::Value& oWildResJson = CWildInfo::GetWildResInfo(ptbWild->m_nSid);
    if (!oWildResJson.isMember(CCommonFunc::NumToString(ptbWild->m_nType)))
    {
        return 0;
    }
    TUINT32 udwTotalNum = 0;
    TINT32 dwTmpNum = ptbWild->m_nLevel - 1;
    if (oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a0"]["a2"].asInt() != 0)
    {
        udwTotalNum = 0;
    }
    else
    {
        const Json::Value& oRewardJson = oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a1"][dwTmpNum]["a1"];
        for (TUINT32 udwIdx = 0; udwIdx < oRewardJson.size(); ++udwIdx)
        {
            if (oRewardJson[udwIdx][0U].asInt() == EN_GLOBALRES_TYPE_RESOURCE)
            {
                udwTotalNum += oRewardJson[udwIdx][2U].asUInt();
            }
            else if (oRewardJson[udwIdx][0U].asInt() == EN_GLOBALRES_TYPE_GEM)
            {
                udwTotalNum += oRewardJson[udwIdx][2U].asUInt() * 10000;
            }
        }
        udwTotalNum = 1.0 * udwTotalNum / 10000 * ptbWild->m_nReward_left;
    }
    return udwTotalNum;
}

TINT32 CMapLogic::ComputeLoadResTime(TbMarch_action *ptbMarch, TbMap *ptbWild)
{
    const Json::Value& oWildResJson = CWildInfo::GetWildResInfo(ptbMarch->m_nSid);

    if (!oWildResJson.isMember(CCommonFunc::NumToString(ptbWild->m_nType)))
    {
        return 1; //保护
    }

    if (ptbWild->m_nReward_left == 0)
    {
        ptbWild->Set_Reward_left(10000);
    }

    TINT32 dwResTime = 0;
    TINT32 dwMaxNum = 0;
    TINT32 dwMinNum = 0;
    double dFactor = 0.0;
    TINT32 dwTotalNum = 0;
    TINT32 dwTmpNum = ptbWild->m_nLevel - 1;
    if (oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a4"]["a2"].asInt() == 1)
    {
        for (TUINT32 udwIdx = 0; udwIdx < EN_TROOP_TYPE__END; ++udwIdx)
        {
            dwTotalNum += ptbMarch->m_bParam[0].m_stTroop[udwIdx];
        }
    }
    else if (oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a4"]["a2"].asInt() == 0)
    {
        dwTotalNum = ptbMarch->m_bParam[0].m_stDragon.m_ddwLevel;
    }

    dwResTime = oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a4"]["a0"][dwTmpNum].asInt() * 1.0 / 10000 * ptbWild->m_nReward_left;
    dwMaxNum = oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a3"][2U]["a2"].asInt();
    dwMinNum = oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a3"][2U]["a1"].asInt();
    dFactor = oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a4"]["a1"].asDouble();

    if (dwTotalNum > dwMaxNum)
    {
        dwTotalNum = dwMaxNum;
    }
    if (dwTotalNum < dwMinNum)
    {
        dwTotalNum -= dwMinNum;
    }
    else
    {
        dwTotalNum = 0;
    }
    dFactor *= 1.0 * dwTotalNum / dwMaxNum;

    TUINT64 uddwResBonus = 0;
    if (oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a0"]["a1"].asInt() == 0
        && oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a0"]["a2"].asInt() == 0)
    {
        switch (oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a0"]["a6"].asInt())
        {
        case EN_RESOURCE_TYPE__GOLD:
            uddwResBonus = CActionBase::GetUserBuffById(ptbMarch, EN_BUFFER_INFO_GOLD_COLLECT_SPEED);
            break;
        case EN_RESOURCE_TYPE__FOOD:
            uddwResBonus = CActionBase::GetUserBuffById(ptbMarch, EN_BUFFER_INFO_FOOD_COLLECT_SPEED);
            break;
        case EN_RESOURCE_TYPE__ORE:
            uddwResBonus = CActionBase::GetUserBuffById(ptbMarch, EN_BUFFER_INFO_ORE_COLLECT_SPEED);
            break;
        case EN_RESOURCE_TYPE__STONE:
            uddwResBonus = CActionBase::GetUserBuffById(ptbMarch, EN_BUFFER_INFO_STONE_COLLECT_SPEED);
            break;
        case EN_RESOURCE_TYPE__WOOD:
            uddwResBonus = CActionBase::GetUserBuffById(ptbMarch, EN_BUFFER_INFO_WOOD_COLLECT_SPEED);
            break;
        default:
            break;
        }
    }
    
    uddwResBonus += CActionBase::GetUserBuffById(ptbMarch, EN_BUFFER_INFO_COLLECT_SPEED);

    dwResTime /= (1 + dFactor) * (1.0 * (10000 + uddwResBonus) / 10000);

    return dwResTime < 1 ? 1 :dwResTime;
}

TINT32 CMapLogic::GetWildClass(TINT32 dwSvrId, TINT32 dwWildType)
{
    TUINT32 udwWildClass = EN_WILD_CLASS_NORMAL;
    const Json::Value& oWildResJson = CWildInfo::GetWildResInfo(dwSvrId);
    if(oWildResJson.isMember(CCommonFunc::NumToString(dwWildType)))
    {
        udwWildClass = oWildResJson[CCommonFunc::NumToString(dwWildType)]["a0"]["a0"].asUInt();
    }
    else
    {
        switch(dwWildType)
        {
        case EN_WILD_TYPE__NORMAL:
        case EN_WILD_TYPE__LAKE:
            udwWildClass = EN_WILD_CLASS_NORMAL;
            break;
        case EN_WILD_TYPE__CITY:
            udwWildClass = EN_WILD_CLASS_CITY;
            break;
        case EN_WILD_TYPE__CAMP:
            udwWildClass = EN_WILD_CLASS_CAMP;
            break;
        case EN_WILD_TYPE__THRONE_NEW:
            udwWildClass = EN_WILD_CLASS_THRONE;
            break;
        case EN_WILD_TYPE__IDOL:
            udwWildClass = EN_WILD_CLASS_IDOL;
            break;
        }
    }
    return udwWildClass;

}

TINT32 CMapLogic::GetCollectedReward(SUserInfo *pstUser,TbMarch_action *ptbMarch, TbMap *ptbWild, TINT32 dwBeginTime, TINT32 dwLoadTime, TINT64 &ddwLoadNum)
{
    SActionMarchParam *pstParam = &ptbMarch->m_bParam[0];
    const Json::Value& oWildResJson = CWildInfo::GetWildResInfo(ptbWild->m_nSid);
    TUINT32 udwCurtime = CTimeUtils::GetUnixTime();

    if (!oWildResJson.isMember(CCommonFunc::NumToString(ptbWild->m_nType)))
    {
        return 0;
    }
    TINT32 dwTmpNum = ptbWild->m_nLevel - 1;
    const Json::Value& oRewardJson = oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a1"][dwTmpNum]["a1"];
    if (ptbWild->m_nReward_left == 0)
    {
        ptbWild->Set_Reward_left(10000);
    }

    if(dwLoadTime == 0)
    {
        return 0;
    }
    TINT32 dwHasLoadRes = ceil((ptbMarch->m_nEtime - pstParam->m_ddwBeginLoadTime) * 1.0 / dwLoadTime * ptbWild->m_nReward_left);
    TSE_LOG_INFO(CGameInfo::GetInstance()->m_poLog, ("GetCollectedReward: has_load_res[%d] etime[%ld] begin_load_time[%ld] load_time[%d] reward[%ld]",
        dwHasLoadRes, ptbMarch->m_nEtime, pstParam->m_ddwBeginLoadTime, dwLoadTime, ptbWild->m_nReward_left));

    if (dwHasLoadRes > ptbWild->m_nReward_left)
    {
        dwHasLoadRes = ptbWild->m_nReward_left;
    }
    if (dwHasLoadRes < 0)
    {
        dwHasLoadRes = 0;
    }
    TSE_LOG_INFO(CGameInfo::GetInstance()->m_poLog, ("GetCollectedReward: has_load_res[%d]", dwHasLoadRes));

    TUINT32 udwClassType = CMapLogic::GetWildClass(ptbWild->m_nSid,ptbWild->m_nType);
    TINT32 udwLoad = CCommonBase::GetTroopTotalLoad(ptbMarch, dwBeginTime);

    TSE_LOG_INFO(CGameInfo::GetInstance()->m_poLog, ("GetCollectedReward: [uid=%u action_id=%lld map_id=%u troop_load_num=%u rewar_left=%u load_percent=%u",
        ptbMarch->m_nSuid, ptbMarch->m_nId, ptbWild->m_nId, udwLoad, ptbWild->m_nReward_left, dwHasLoadRes));

    if (oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a0"]["a2"].asInt() == 0)
    {
        for (TUINT32 udwIdx = 0; udwIdx < oRewardJson.size(); ++udwIdx)
        {
            if (oRewardJson[udwIdx][0U].asInt() == EN_GLOBALRES_TYPE_RESOURCE)
            {
                TINT64 ddwCollectNum = oRewardJson[udwIdx][2U].asInt64() * dwHasLoadRes / 10000;
                if (static_cast<TINT32>(ddwCollectNum) > udwLoad)
                {
                    ddwCollectNum = udwLoad;
                }
                udwLoad -= ddwCollectNum;
                pstParam->m_stResource[oRewardJson[udwIdx][1U].asUInt()] += ddwCollectNum;
                ddwLoadNum = pstParam->m_stResource[oRewardJson[udwIdx][1U].asUInt()];
            }
            else if (oRewardJson[udwIdx][0U].asInt() == EN_GLOBALRES_TYPE_GEM)
            {
                TINT64 ddwNum = oRewardJson[udwIdx][2U].asInt64();
                TINT32 dwCollectedNum = ddwNum * ptbWild->m_nReward_left / 10000
                    - ddwNum * (ptbWild->m_nReward_left - dwHasLoadRes) / 10000;
                if (dwCollectedNum > udwLoad / 10000)
                {
                    dwCollectedNum = udwLoad / 10000;
                }
                udwLoad -= dwCollectedNum * 10000;
                pstParam->m_ddwLoadGem += dwCollectedNum;
                ddwLoadNum = pstParam->m_stResource[oRewardJson[udwIdx][1U].asUInt()];
            }
        }
        ptbMarch->SetFlag(TbMARCH_ACTION_FIELD_PARAM);

        //针对资源地 阶段掉落机制
        // GenStageResult(ptbWild, ptbMarch, dwLoadTime);

        ptbWild->Set_Reward_left(ptbWild->m_nReward_left - dwHasLoadRes);
//         if (ptbWild->m_nReward_left == 0)
//         {
//             for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; ++udwIdx)
//             {
//                 TbMarch_action tbAction = pstUser->m_atbMarch[udwIdx];
//                 if (tbAction.m_nMclass == EN_ACTION_MAIN_CLASS__MARCH && tbAction.m_nSclass == EN_ACTION_SEC_CLASS__OCCUPY &&
//                     tbAction.m_nTpos == ptbWild->m_nId)
//                 {
//                     pstUser->m_atbMarch[udwIdx].Set_Etime(udwCurtime);
//                     pstUser->m_atbMarch[udwIdx].Set_Status(EN_MARCH_STATUS__RETURNING);
//                 }
//             }
//         }
    }
//     else
//     {
//         if (ptbWild->m_nReward_left - dwHasLoadRes == 0)
//         {
//             GenWildItemReward(ptbWild, ptbMarch);
//         }
//         ptbWild->Set_Reward_left(ptbWild->m_nReward_left - dwHasLoadRes);
//     }

//     if(udwClassType != EN_WILD_CLASS_RES && ptbWild->m_nReward_left == 0)
//     {
//         GenSurpriseResult(ptbWild, ptbMarch);
//     }
    return 0;
}

TBOOL CMapLogic::HaveCollectedOut(SUserInfo *pstUser, TbMarch_action *ptbMainMarch, TbMap *ptbWild)
{
    TbMarch_action *ptbMarch = NULL;
    TUINT32 udwLoadRes = 0;
    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; udwIdx++)
    {
        ptbMarch = &pstUser->m_atbMarch[udwIdx];
        if (ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__OCCUPY
            && ptbMarch->m_nStatus == EN_MARCH_STATUS__LOADING
            && ptbMarch->m_nTpos == ptbWild->m_nId)
        {
            udwLoadRes += ceil((ptbMarch->m_nEtime - ptbMarch->m_bParam[0].m_ddwBeginLoadTime) * 1.0 / ptbMarch->m_bParam[0].m_ddwTotalLoadTime * ptbWild->m_nReward_left);
            if (ptbMarch->m_nEtime > ptbMainMarch->m_nEtime)
            {
                return FALSE;
            }
        }
    }

    if (udwLoadRes >= ptbWild->m_nReward_left)
    {
        return TRUE;
    }

    return FALSE;
}

TINT32 CMapLogic::GetAttackReward(TbMarch_action *ptbMarch, TbMap *ptbWild)
{
    const Json::Value& oWildResJson = CWildInfo::GetWildResInfo(ptbWild->m_nSid);
    if (!oWildResJson.isMember(CCommonFunc::NumToString(ptbWild->m_nType)))
    {
        return 0;
    }

    GenWildItemReward(ptbWild, ptbMarch);
    GenSurpriseResult(ptbWild, ptbMarch);

    return 0;
}

//普通奖励
TVOID CMapLogic::GenWildItemReward(TbMap* ptbWild, TbMarch_action* ptbReqMarch, TUINT32 udwTime /*= 1*/)
{
    TINT32 dwRewardType = 0;
    const Json::Value& oWildResJson = CWildInfo::GetWildResInfo(ptbWild->Get_Sid());
    dwRewardType = oWildResJson[CCommonFunc::NumToString(ptbWild->Get_Type())]["a0"]["a3"].asInt();

    if (dwRewardType == EREWARD_TYPE_ALL)
    {
        CMapLogic::GenConfirmWildItemReward(ptbWild, ptbReqMarch, udwTime);
    }
    else if (dwRewardType == EREWARD_TYPE_SOME)
    {
        CMapLogic::GenRandomWildItemReward(ptbWild, ptbReqMarch, udwTime);
    }
    else if (dwRewardType == EREWARD_TYPE_ONE)
    {
        CMapLogic::GenOneWildItemReward(ptbWild, ptbReqMarch,udwTime);
    }
    return;
}

TBOOL CMapLogic::GenConfirmWildItemReward(TbMap* ptbWild, TbMarch_action* ptbMarch, TUINT32 udwTime /*= 1*/)
{
    ptbMarch->m_bReward.Reset();
    const Json::Value& oWildResJson = CWildInfo::GetWildResInfo(ptbMarch->m_nSid);
    if(!oWildResJson.isMember(CCommonFunc::NumToString(ptbWild->m_nType)))
    {
        return FALSE;
    }
    TINT32 dwTmpNum = ptbWild->m_nLevel - 1;
    const Json::Value& oReward = oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a1"][dwTmpNum]["a1"];
    for(TUINT32 udwTimeIdx = 0; udwTimeIdx < udwTime; ++udwTimeIdx)
    {
        for(TUINT32 udwIdx = 0; udwIdx < oReward.size(); ++udwIdx)
        {
            if(oReward[udwIdx][0U].asUInt() >= EN_GLOBALRES_TYPE_END)
            {
                continue;
            }
            if(ptbMarch->m_bReward[0].ddwTotalNum >= MAX_REWARD_ITEM_NUM)
            {
                continue;
            }
            TUINT32 udwType = oReward[udwIdx][0U].asUInt();
            TUINT32 udwId = oReward[udwIdx][1U].asInt();
            TUINT32 udwNum = oReward[udwIdx][2U].asUInt();
            TBOOL bExit = FALSE;
            for(TUINT32 udwOwnIdx = 0; udwOwnIdx < ptbMarch->m_bReward[0].ddwTotalNum;++udwOwnIdx)
            {
                if(ptbMarch->m_bReward[0].aRewardList[udwOwnIdx].ddwType == udwType &&
                    ptbMarch->m_bReward[0].aRewardList[udwOwnIdx].ddwId == udwId)
                {
                    ptbMarch->m_bReward[0].aRewardList[udwOwnIdx].ddwNum += udwNum;
                    bExit = TRUE;
                }
            }
            if(!bExit)
            {
                ptbMarch->m_bReward[0].aRewardList[ptbMarch->m_bReward[0].ddwTotalNum].ddwType = udwType;
                ptbMarch->m_bReward[0].aRewardList[ptbMarch->m_bReward[0].ddwTotalNum].ddwId = udwId;
                ptbMarch->m_bReward[0].aRewardList[ptbMarch->m_bReward[0].ddwTotalNum].ddwNum = udwNum;
                ++ptbMarch->m_bReward[0].ddwTotalNum;
            }
            ptbMarch->SetFlag(TbMARCH_ACTION_FIELD_REWARD);
        }
    }
    return TRUE;
}

TBOOL CMapLogic::GenRandomWildItemReward(TbMap* ptbWild, TbMarch_action* ptbMarch, TUINT32 udwTime /*= 1*/)
{
    ptbMarch->m_bReward.Reset();
    const Json::Value& oWildResJson = CWildInfo::GetWildResInfo(ptbMarch->m_nSid);
    if (!oWildResJson.isMember(CCommonFunc::NumToString(ptbWild->m_nType)))
    {
        return FALSE;
    }

    TINT32 dwTmpNum = ptbWild->m_nLevel - 1;
    
    Json::Value oReward = oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a1"][dwTmpNum]["a1"];

    CGlobalResLogic::ComputeBuffRewarRate(oReward, ptbMarch, 1);

    for(TUINT32 udwTimeIdx = 0; udwTimeIdx < udwTime;++udwTimeIdx)
    {
        for(TUINT32 udwIdx = 0; udwIdx < oReward.size(); ++udwIdx)
        {
            if(oReward[udwIdx][0U].asUInt() >= EN_GLOBALRES_TYPE_END)
            {
                continue;
            }

            //运营此处给的数值最小单位为0.000001
            double dSingleRate = oReward[udwIdx][3U].asDouble();
            double dRand = (double)rand() / RAND_MAX;
            if(dRand - dSingleRate <= 0.000001)
            {
                TUINT32 udwNum = CToolBase::GetRandNumber(oReward[udwIdx][4U].asUInt(), oReward[udwIdx][5U].asUInt());

                if(ptbMarch->m_bReward[0].ddwTotalNum >= MAX_REWARD_ITEM_NUM)
                {
                    continue;
                }
                TUINT32 udwType = oReward[udwIdx][0U].asUInt();
                TUINT32 udwId = oReward[udwIdx][1U].asInt();
                
                TBOOL bExit = FALSE;
                for(TUINT32 udwOwnIdx = 0; udwOwnIdx < ptbMarch->m_bReward[0].ddwTotalNum; ++udwOwnIdx)
                {
                    if(ptbMarch->m_bReward[0].aRewardList[udwOwnIdx].ddwType == udwType &&
                        ptbMarch->m_bReward[0].aRewardList[udwOwnIdx].ddwId == udwId)
                    {
                        ptbMarch->m_bReward[0].aRewardList[udwOwnIdx].ddwNum += udwNum;
                        bExit = TRUE;
                    }
                }
                if(!bExit)
                {
                    ptbMarch->m_bReward[0].aRewardList[ptbMarch->m_bReward[0].ddwTotalNum].ddwType = udwType;
                    ptbMarch->m_bReward[0].aRewardList[ptbMarch->m_bReward[0].ddwTotalNum].ddwId = udwId;
                    ptbMarch->m_bReward[0].aRewardList[ptbMarch->m_bReward[0].ddwTotalNum].ddwNum = udwNum;
                    ++ptbMarch->m_bReward[0].ddwTotalNum;
                }
                
                ptbMarch->SetFlag(TbMARCH_ACTION_FIELD_REWARD);
            }
        }
    }
    
    return TRUE;
}

TBOOL CMapLogic::GenOneWildItemReward(TbMap* ptbWild, TbMarch_action* ptbMarch, TUINT32 udwTime /*= 1*/)
{
    ptbMarch->m_bReward.Reset();
    const Json::Value& oWildResJson = CWildInfo::GetWildResInfo(ptbMarch->m_nSid);
    if (!oWildResJson.isMember(CCommonFunc::NumToString(ptbWild->m_nType)))
    {
        return FALSE;
    }
    TINT32 dwTmpNum = ptbWild->m_nLevel - 1;
    Json::Value oReward = oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a1"][dwTmpNum]["a1"];

    CGlobalResLogic::ComputeBuffRewarRate(oReward, ptbMarch, 0);

    TUINT64 uddwTotalRate = 0;
    for (TUINT32 udwIdx = 0; udwIdx < oReward.size(); ++udwIdx)
    {
        uddwTotalRate += oReward[udwIdx][3U].asInt();
    }

    if (uddwTotalRate == 0)
    {
        return FALSE;
    }
    for(TUINT32 udwTimeIdx = 0; udwTimeIdx < udwTime;++udwTimeIdx)
    {
        TUINT64 uddwRand = (double)rand() / RAND_MAX * uddwTotalRate;

        TUINT64 uddwTmpRand = 0;
        for(TUINT32 udwIdx = 0; udwIdx < oReward.size(); ++udwIdx)
        {
            // 存在隐患，如果随机奖励中配置了资源奖励，则会多获取物品
            uddwTmpRand += oReward[udwIdx][3U].asInt();
            if(oReward[udwIdx][0U].asUInt() >= EN_GLOBALRES_TYPE_END)
            {
                continue;
            }

            if(uddwTmpRand > uddwRand)
            {
                TUINT32 udwNum = CToolBase::GetRandNumber(oReward[udwIdx][4U].asUInt(), oReward[udwIdx][5U].asUInt());
                if(ptbMarch->m_bReward[0].ddwTotalNum >= MAX_REWARD_ITEM_NUM)
                {
                    continue;
                }
                TUINT32 udwType = oReward[udwIdx][0U].asUInt();
                TUINT32 udwId = oReward[udwIdx][1U].asInt();
                
                TBOOL bExit = FALSE;
                for(TUINT32 udwOwnIdx = 0; udwOwnIdx < ptbMarch->m_bReward[0].ddwTotalNum; ++udwOwnIdx)
                {
                    if(ptbMarch->m_bReward[0].aRewardList[udwOwnIdx].ddwType == udwType &&
                        ptbMarch->m_bReward[0].aRewardList[udwOwnIdx].ddwId == udwId)
                    {
                        ptbMarch->m_bReward[0].aRewardList[udwOwnIdx].ddwNum += udwNum;
                        bExit = TRUE;
                    }
                }
                if(!bExit)
                {
                    ptbMarch->m_bReward[0].aRewardList[ptbMarch->m_bReward[0].ddwTotalNum].ddwType = udwType;
                    ptbMarch->m_bReward[0].aRewardList[ptbMarch->m_bReward[0].ddwTotalNum].ddwId = udwId;
                    ptbMarch->m_bReward[0].aRewardList[ptbMarch->m_bReward[0].ddwTotalNum].ddwNum = udwNum;
                    ++ptbMarch->m_bReward[0].ddwTotalNum;
                }

                ptbMarch->SetFlag(TbMARCH_ACTION_FIELD_REWARD);
                break;
            }
        }
    }
    return TRUE;
}


//惊喜奖励
TVOID CMapLogic::GenSurpriseResult(TbMap* ptbWild, TbMarch_action* ptbMarch)
{
    TINT32 dwRewardType = 0;
    const Json::Value& oWildResJson = CWildInfo::GetWildResInfo(ptbMarch->m_nSid);
    if (!oWildResJson.isMember(CCommonFunc::NumToString(ptbWild->m_nType)))
    {
        return ;
    }

    dwRewardType = oWildResJson[CCommonFunc::NumToString(ptbWild->Get_Type())]["a0"]["a4"].asInt();
    TINT32 dwTmpNum = ptbWild->m_nLevel - 1;
    double dSurpriseRate = oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a1"][dwTmpNum]["a0"].asDouble();
    double dRand = (double)rand() / RAND_MAX;
    if (dRand - dSurpriseRate <= 0.000001)
    {
        if (dwRewardType == EREWARD_TYPE_ALL)
        {
            GenSurpriceConfirmWildItemReward(ptbWild, ptbMarch);
        }
        else if (dwRewardType == EREWARD_TYPE_SOME)
        {
            GenSurpriceRandomWildItemReward(ptbWild, ptbMarch);
        }
        else if (dwRewardType == EREWARD_TYPE_ONE)
        {
            GenSurpriceOneWildItemReward(ptbWild, ptbMarch);
        }
    }
}

TBOOL CMapLogic::GenSurpriceConfirmWildItemReward(TbMap* ptbWild, TbMarch_action* ptbMarch)
{
    ptbMarch->m_bSp_reward.Reset();
    const Json::Value& oWildResJson = CWildInfo::GetWildResInfo(ptbMarch->m_nSid);
    if(!oWildResJson.isMember(CCommonFunc::NumToString(ptbWild->m_nType)))
    {
        return FALSE;
    }
    TINT32 dwTmpNum = ptbWild->m_nLevel - 1;
    const Json::Value& oReward = oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a1"][dwTmpNum]["a2"];
    for(TUINT32 udwIdx = 0; udwIdx < oReward.size(); ++udwIdx)
    {
        if(oReward[udwIdx][0U].asUInt() >= EN_GLOBALRES_TYPE_END)
        {
            continue;
        }

        if(ptbMarch->m_bSp_reward[0].ddwTotalNum >= MAX_REWARD_ITEM_NUM)
        {
            continue;
        }

        ptbMarch->m_bSp_reward[0].aRewardList[ptbMarch->m_bSp_reward[0].ddwTotalNum].ddwType = oReward[udwIdx][0U].asUInt();
        ptbMarch->m_bSp_reward[0].aRewardList[ptbMarch->m_bSp_reward[0].ddwTotalNum].ddwId = oReward[udwIdx][1U].asInt();
        ptbMarch->m_bSp_reward[0].aRewardList[ptbMarch->m_bSp_reward[0].ddwTotalNum].ddwNum = oReward[udwIdx][2U].asUInt();
        ++ptbMarch->m_bSp_reward[0].ddwTotalNum;
        ptbMarch->SetFlag(TbMARCH_ACTION_FIELD_SP_REWARD);
    }
    return TRUE;
}

TBOOL CMapLogic::GenSurpriceRandomWildItemReward(TbMap* ptbWild, TbMarch_action* ptbMarch)
{
    ptbMarch->m_bSp_reward.Reset();
    const Json::Value& oWildResJson = CWildInfo::GetWildResInfo(ptbMarch->m_nSid);
    if (!oWildResJson.isMember(CCommonFunc::NumToString(ptbWild->m_nType)))
    {
        return FALSE;
    }
    TINT32 dwTmpNum = ptbWild->m_nLevel - 1;

    Json::Value oReward = oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a1"][dwTmpNum]["a2"];

    CGlobalResLogic::ComputeBuffRewarRate(oReward, ptbMarch, 1);

    for (TUINT32 udwIdx = 0; udwIdx < oReward.size(); ++udwIdx)
    {
        if (oReward[udwIdx][0U].asUInt() >= EN_GLOBALRES_TYPE_END)
        {
            continue;
        }

        if(ptbMarch->m_bSp_reward[0].ddwTotalNum >= MAX_REWARD_ITEM_NUM)
        {
            continue;
        }

        //运营此处给的数值最小单位为0.000001
        double dSingleRate = oReward[udwIdx][3U].asDouble();
        double dRand = (double)rand() / RAND_MAX;
        if (dRand - dSingleRate <= 0.000001)
        {
            TUINT32 udwNum = CToolBase::GetRandNumber(oReward[udwIdx][4U].asUInt(), oReward[udwIdx][5U].asUInt());
            ptbMarch->m_bSp_reward[0].aRewardList[ptbMarch->m_bSp_reward[0].ddwTotalNum].ddwType = oReward[udwIdx][0U].asUInt();
            ptbMarch->m_bSp_reward[0].aRewardList[ptbMarch->m_bSp_reward[0].ddwTotalNum].ddwId = oReward[udwIdx][1U].asInt();
            ptbMarch->m_bSp_reward[0].aRewardList[ptbMarch->m_bSp_reward[0].ddwTotalNum].ddwNum = udwNum;
            ++ptbMarch->m_bSp_reward[0].ddwTotalNum;
            ptbMarch->SetFlag(TbMARCH_ACTION_FIELD_SP_REWARD);
        }
    }
    return TRUE;
}

TBOOL CMapLogic::GenSurpriceOneWildItemReward(TbMap* ptbWild, TbMarch_action* ptbMarch)
{
    ptbMarch->m_bSp_reward.Reset();
    const Json::Value& oWildResJson = CWildInfo::GetWildResInfo(ptbMarch->m_nSid);
    if (!oWildResJson.isMember(CCommonFunc::NumToString(ptbWild->m_nType)))
    {
        return FALSE;
    }
    TINT32 dwTmpNum = ptbWild->m_nLevel - 1;
    
    Json::Value oReward = oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a1"][dwTmpNum]["a2"];
    CGlobalResLogic::ComputeBuffRewarRate(oReward, ptbMarch, 0);

    TUINT64 uddwTotalRate = 0;
    for (TUINT32 udwIdx = 0; udwIdx < oReward.size(); ++udwIdx)
    {
        uddwTotalRate += oReward[udwIdx][3U].asInt();
    }

    if (0 == uddwTotalRate)
    {
        return FALSE;
    }
    TUINT64 uddwRand = (double)rand() / RAND_MAX * uddwTotalRate;
    TUINT64 uddwTmpRand = 0;
    for (TUINT32 udwIdx = 0; udwIdx < oReward.size(); ++udwIdx)
    {
        if (oReward[udwIdx][0U].asUInt() >= EN_GLOBALRES_TYPE_END)
        {
            continue;
        }

        if(ptbMarch->m_bSp_reward[0].ddwTotalNum >= MAX_REWARD_ITEM_NUM)
        {
            continue;
        }

        uddwTmpRand += oReward[udwIdx][3U].asInt();
        if (uddwTmpRand > uddwRand)
        {
            TUINT32 udwNum = CToolBase::GetRandNumber(oReward[udwIdx][4U].asUInt(), oReward[udwIdx][5U].asUInt());
            ptbMarch->m_bSp_reward[0].aRewardList[ptbMarch->m_bSp_reward[0].ddwTotalNum].ddwType = oReward[udwIdx][0U].asUInt();
            ptbMarch->m_bSp_reward[0].aRewardList[ptbMarch->m_bSp_reward[0].ddwTotalNum].ddwId = oReward[udwIdx][1U].asInt();
            ptbMarch->m_bSp_reward[0].aRewardList[ptbMarch->m_bSp_reward[0].ddwTotalNum].ddwNum = udwNum;
            ++ptbMarch->m_bSp_reward[0].ddwTotalNum;
            ptbMarch->SetFlag(TbMARCH_ACTION_FIELD_SP_REWARD);
            break;
        }
    }
    return FALSE;
}

TINT32 CMapLogic::GetIdxById(TUINT32 udwId, TbMap *patbMap,TUINT32 udwNum)
{
    for(TUINT32 udwIdx = 0; udwIdx < udwNum;++udwIdx)
    {
        if(patbMap[udwIdx].m_nId == udwId)
        {
            return udwIdx;
        }
    }
    return -1;
}

TBOOL CMapLogic::IsPosCanMove(TUINT32 udwPos, TbMap *patbMap, TUINT32 udwNum,TUINT32 udwTypeBlockNum,TUINT32 udwSid)
{
    /*
    //udwTypeBlockNum 是改地形的地块数 定为 NXN
    TUINT32 udwXpos = udwPos / MAP_X_Y_POS_COMPUTE_OFFSET;
    TUINT32 udwYpos = udwPos % MAP_X_Y_POS_COMPUTE_OFFSET;
    TUINT32 udwSize = udwTypeBlockNum;
    assert(udwSize != 0);
    const Json::Value &oWildJson = CWildInfo::GetWildResInfo(udwSid);

    std::set<TINT32> posSet;
    posSet.clear();

    TUINT32 udwCenterPos = CCommonLogic::BuildingPointToPos(udwXpos, udwYpos);
    CCommonLogic::GetBuildingPos(udwCenterPos, udwSize, posSet);

    for(set<TINT32>::iterator it = posSet.begin(); it != posSet.end(); ++it)
    {
        BuildingPoint stBuildingPoing = CCommonLogic::BuildingPosToPoint(*it);

        TUINT32 udwNewPos = stBuildingPoing.x * MAP_X_Y_POS_COMPUTE_OFFSET + stBuildingPoing.y;

        TINT32 dwDataIdx = CMapLogic::GetIdxById(udwNewPos, patbMap, udwNum);
        if(dwDataIdx == -1)
        {
            //数据不足
            TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("IsPosCanMove: [pos=%u side pos info not in tmp_map]",udwNewPos));
            return FALSE;
        }
        
        if(patbMap[dwDataIdx].m_nUid != 0 || 
            patbMap[dwDataIdx].m_nRtype == EN_WILD_TYPE__LAKE ||
            patbMap[dwDataIdx].m_nType == EN_WILD_TYPE__CITY )
        {
            TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("IsPosCanMove: [pos=%u uid=%u type=%u smoe condition not right]", udwNewPos, patbMap[dwDataIdx].m_nUid, patbMap[dwDataIdx].m_nType));
            return FALSE;
        }
        if(oWildJson.isMember(CCommonFunc::NumToString(patbMap[dwDataIdx].m_nType)))
        {
            std::set<TINT32> sProvinceSet;
            CMapLogic::GetProvincePoints(sProvinceSet);
            std::set<TINT32> sThronSet;
            CMapLogic::GetThronePoints(sThronSet);
            TUINT32 udwWildClass = CMapLogic::GetWildClass(patbMap[dwDataIdx].m_nSid, patbMap[dwDataIdx].m_nType);
            if ((EN_WILD_CLASS_HERO_MOSTER == udwWildClass || EN_WILD_CLASS_LEADER_MONSTER == udwWildClass) &&
                patbMap[dwDataIdx].m_nWild_gen_time != 0 && 
                (patbMap[dwDataIdx].m_nWild_gen_time + 3600 * 3 > CTimeUtils::GetUnixTime()))
            {
                TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("IsPosCanMove: [pos=%u type=%u end_time=%u smoe condition not right]",
                    udwNewPos, patbMap[dwDataIdx].m_nType, patbMap[dwDataIdx].m_nWild_gen_time + 3600 * 3));
                return FALSE;
            }
            if(EN_WILD_CLASS_HERO_COLLECT == udwWildClass)
            {
                TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("IsPosCanMove: [pos=%u type=%u  smoe condition not right]",
                    udwNewPos,  patbMap[dwDataIdx].m_nType));
                return FALSE;
            }
            if(udwWildClass == EN_WILD_CLASS_PROVINCE && sProvinceSet.find(CCommonLogic::BuildingPointToPos(udwPos / MAP_X_Y_POS_COMPUTE_OFFSET, udwPos % MAP_X_Y_POS_COMPUTE_OFFSET)) == sProvinceSet.end())
            {
                continue;
            }
            if(udwWildClass == EN_WILD_CLASS_THRONE && sThronSet.find(CCommonLogic::BuildingPointToPos(udwPos / MAP_X_Y_POS_COMPUTE_OFFSET, udwPos % MAP_X_Y_POS_COMPUTE_OFFSET)) == sThronSet.end())
            {
                continue;
            }

            TUINT32 udwTmpBlockNum = CMapBase::GetWildBlockNumByType(udwSid, patbMap[dwDataIdx].m_nType);
            if(udwTmpBlockNum != 1)
            {
                TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("IsPosCanMove: [pos=%u type=%u block_num=%u smoe condition not right]", 
                    udwNewPos, patbMap[dwDataIdx].m_nType, udwTmpBlockNum));
                return FALSE;
            }
        }
    }
    return TRUE;
    */

    return FALSE;
}

TBOOL CMapLogic::IsPosCanMove( TbMap *ptbMap )
{
    /*
    if(ptbMap->m_nUid != 0 || 
        ptbMap->m_nRtype == EN_WILD_TYPE__LAKE)
    {
        return FALSE;
    }

    TUINT32 udwWildClass = GetWildClass(ptbMap->m_nSid, ptbMap->m_nType);
    if(udwWildClass >= EN_WILD_CLASS_HERO_MOSTER)
    {
        if((EN_WILD_CLASS_HERO_MOSTER == udwWildClass || EN_WILD_CLASS_LEADER_MONSTER == udwWildClass) && 
            (ptbMap->m_nWild_gen_time + MAX_MONSTER_SHOW_INTERVAL_TIME < CTimeUtils::GetUnixTime()))
        {
            return TRUE;
        }
        else if (EN_WILD_CLASS_HERO_COLLECT == udwWildClass 
            && ptbMap->m_nExpire_time < CTimeUtils::GetUnixTime())
        {
            return TRUE;
        }

        return FALSE;
    }

    return TRUE;
    */

    return FALSE;
}


TBOOL CMapLogic::IsDragonAttackWild(TINT32 dwSvrId, TINT32 dwWildType)
{
    const Json::Value& oWildResJson = CWildInfo::GetWildResInfo(dwSvrId);
    if(!oWildResJson.isMember(CCommonFunc::NumToString(dwWildType)))
    {
        return FALSE;
    }
    if(oWildResJson[CCommonFunc::NumToString(dwWildType)]["a0"]["a1"].asInt() == 1
        && (oWildResJson[CCommonFunc::NumToString(dwWildType)]["a0"]["a0"].asInt() == EN_WILD_CLASS_MONSTER
        || oWildResJson[CCommonFunc::NumToString(dwWildType)]["a0"]["a0"].asInt() == EN_WILD_CLASS_LEADER_MONSTER))
    {
        return TRUE;
    }
    return FALSE;
}

TBOOL CMapLogic::IsDragonOccupyWild(TINT32 dwSvrId, TINT32 dwWildType)
{
    const Json::Value& oWildResJson = CWildInfo::GetWildResInfo(dwSvrId);
    if(!oWildResJson.isMember(CCommonFunc::NumToString(dwWildType)))
    {
        return FALSE;
    }
    if(oWildResJson[CCommonFunc::NumToString(dwWildType)]["a0"]["a1"].asInt() == 0
        && oWildResJson[CCommonFunc::NumToString(dwWildType)]["a0"]["a0"].asInt() == EN_WILD_CLASS_MONSTER_NEST)
    {
        return TRUE;
    }
    return FALSE;
}


TBOOL CMapLogic::GenMosterAllianceGift(SUserInfo *pstUser, TbMap *ptbWild, TbMarch_action *ptbReqAction)
{
    const Json::Value& oWildResJson = CWildInfo::GetWildResInfo(ptbWild->m_nSid);
    if(!oWildResJson.isMember(CCommonFunc::NumToString(ptbWild->m_nType)))
    {
        return FALSE;
    }

    ptbReqAction->m_bAl_gift_record.Reset();
    TINT32 dwTmpNum = ptbWild->m_nLevel - 1;
    const Json::Value& oReward = oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a1"][dwTmpNum]["a2"];
    for(TUINT32 udwIdx = 0; udwIdx < oReward.size(); ++udwIdx)
    {
        if(oReward[udwIdx][0U].asUInt() == EN_GLOBALRES_TYPE_AL_GIFT)
        {
            TUINT32 udwPackId = oReward[udwIdx][1U].asUInt();
            TINT32 dwPackNum = oReward[udwIdx][2U].asInt();
            for(TINT32 dwIdx = 0; dwIdx < dwPackNum; dwIdx++)
            {
                CCommonLogic::GenAlGiftToAlliance(pstUser->m_tbAlliance.m_nAid, udwPackId, pstUser->m_tbPlayer.m_nUid, pstUser->m_stAlGifts, &pstUser->m_tbLogin);
            }

            // 将奖励信息记录下来
            if(ptbReqAction->m_bAl_gift_record[0].ddwTotalNum >= MAX_REWARD_ITEM_NUM)
            {
                continue;
            }
            ptbReqAction->m_bAl_gift_record[0].aRewardList[ptbReqAction->m_bAl_gift_record[0].ddwTotalNum].ddwType = oReward[udwIdx][0U].asUInt();
            ptbReqAction->m_bAl_gift_record[0].aRewardList[ptbReqAction->m_bAl_gift_record[0].ddwTotalNum].ddwId = udwPackId;
            ptbReqAction->m_bAl_gift_record[0].aRewardList[ptbReqAction->m_bAl_gift_record[0].ddwTotalNum].ddwNum = dwPackNum;
            ++ptbReqAction->m_bAl_gift_record[0].ddwTotalNum;
            ptbReqAction->SetFlag(TbMARCH_ACTION_FIELD_AL_GIFT_RECORD);
        }
    }
    return TRUE;
}

//阶段性奖励
TVOID CMapLogic::GenStageResult(TbMap* ptbWild, TbMarch_action* ptbMarch, TINT32 dwLoadTime)
{
    SActionMarchParam *pstParam = &ptbMarch->m_bParam[0];
    const Json::Value& oWildResJson = CWildInfo::GetWildResInfo(ptbWild->m_nSid);
    if(!oWildResJson.isMember(CCommonFunc::NumToString(ptbWild->m_nType)))
    {
        return;
    }
    if(EN_WILD_CLASS_RES != oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)]["a0"]["a0"].asUInt())
    {
        return;
    }

    TUINT32 dwHasLoadRes = ceil((ptbMarch->m_nEtime - pstParam->m_ddwBeginLoadTime) * 1.0 / dwLoadTime * ptbWild->m_nReward_left);

    if(dwHasLoadRes > ptbWild->m_nReward_left)
    {
        dwHasLoadRes = ptbWild->m_nReward_left;
    }

    TINT32 dwTmpNum = ptbWild->m_nLevel - 1;
    const Json::Value &oWildTypeInfo = oWildResJson[CCommonFunc::NumToString(ptbWild->m_nType)];
    if(!oWildTypeInfo["a1"][dwTmpNum].isMember("a3"))
    {
        return;
    }
    if(oWildTypeInfo["a1"][dwTmpNum]["a3"]["a0"].size() == 0||
        oWildTypeInfo["a1"][dwTmpNum]["a3"]["a1"].size() == 0 ||
        oWildTypeInfo["a1"][dwTmpNum]["a3"]["a2"].size() == 0)
    {
        return;
    }
    TINT32 dwRewardStage = 0;
    const Json::Value &oStageReward = oWildTypeInfo["a1"][dwTmpNum]["a3"];
    //阶段
    for(TUINT32 dwIdx = 0; dwIdx < oStageReward["a0"].size();++dwIdx)
    {
        if(dwHasLoadRes <= oStageReward["a0"][dwIdx].asUInt())
        {
            dwRewardStage = dwIdx + 1;
            break;
        }
    }
    Json::Value oReward = oStageReward["a2"];
    CGlobalResLogic::ComputeBuffRewarRate(oReward, ptbMarch, 0);

    for(TINT32 dwIdx = 0; dwIdx < dwRewardStage;++dwIdx)
    {
        TINT32 dwRewardRate = oStageReward["a1"][dwIdx].asInt();
        TINT32 dwRand = rand() % 10000;

        if(dwRand < dwRewardRate)
        {

            //获得奖励
            TUINT32 udwTotalRate = 0;
            TINT32 dwRewardIdx = 0;
            for(TUINT32 dwRewardIdx = 0; dwRewardIdx < oReward.size(); ++dwRewardIdx)
            {
                udwTotalRate += oReward[dwRewardIdx][3U].asUInt();
            }

            if(udwTotalRate == 0)
            {
                break;
            }
            TUINT32 dwRewardRand = rand() % udwTotalRate;

            udwTotalRate = 0;
            for(TUINT32 Idx = 0; Idx < oReward.size(); ++Idx)
            {
                udwTotalRate += oReward[Idx][3U].asUInt();
                if(dwRewardRand < udwTotalRate)
                {
                    dwRewardIdx = Idx;
                    break;
                }
            }
            
            //add reward
            TUINT32 udwRewardType = oReward[dwRewardIdx][0U].asUInt();
            TUINT32 udwRewardId = oReward[dwRewardIdx][1U].asUInt();
            TUINT32 udwRewardNum = oReward[dwRewardIdx][2U].asUInt();

            if(udwRewardType >= EN_GLOBALRES_TYPE_END)
            {
                continue;
            }

            if(ptbMarch->m_bReward[0].ddwTotalNum >= MAX_REWARD_ITEM_NUM)
            {
                continue;
            }
            ptbMarch->m_bReward[0].aRewardList[ptbMarch->m_bReward[0].ddwTotalNum].ddwType = udwRewardType;
            ptbMarch->m_bReward[0].aRewardList[ptbMarch->m_bReward[0].ddwTotalNum].ddwId = udwRewardId;
            ptbMarch->m_bReward[0].aRewardList[ptbMarch->m_bReward[0].ddwTotalNum].ddwNum = udwRewardNum;
            ++ptbMarch->m_bReward[0].ddwTotalNum;
            ptbMarch->SetFlag(TbMARCH_ACTION_FIELD_REWARD);

        }
    }
    return ;
}

TINT32 CMapLogic::GetResTypeByWildType(TUINT32 udwSid,TINT32 dwWildType,TUINT32 udwLv)
{
    const Json::Value& oWildResJson = CWildInfo::GetWildResInfo(udwSid);

    if(!oWildResJson.isMember(CCommonFunc::NumToString(dwWildType)))
    {
        return -1;
    }
    TINT32 dwTmpNum = udwLv > 0 ? udwLv - 1 : 0;
    if(oWildResJson[CCommonFunc::NumToString(dwWildType)]["a0"]["a2"].asInt() != 0)
    {
        return -1;
    }

    const Json::Value& oRewardJson = oWildResJson[CCommonFunc::NumToString(dwWildType)]["a1"][dwTmpNum]["a1"];
    for(TUINT32 udwIdx = 0; udwIdx < oRewardJson.size(); ++udwIdx)
    {
        if(oRewardJson[udwIdx][0U].asInt() == EN_GLOBALRES_TYPE_RESOURCE)
        {
            return oRewardJson[udwIdx][1U].asInt();
        }
    }
    return -1;
}

TBOOL CMapLogic::IfPlayerCity(SUserInfo *pstUser, TbMap *ptbWild)
{
    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwWildNum; ++udwIdx)
    {
        if (pstUser->m_atbWild[udwIdx].m_nId == ptbWild->m_nId &&
            pstUser->m_tbPlayer.m_nUid == ptbWild->m_nUid &&
            ptbWild->m_nType == EN_WILD_TYPE__CITY)
        {
            return TRUE;
        }
    }
    return FALSE;
}

TBOOL CMapLogic::IsWildNeedToDelete(TbMap *ptbWild)
{
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    if (udwCurTime < ptbWild->m_nShowtime)
    {
        return TRUE;
    }

    TUINT32 udwWildClass = CMapLogic::GetWildClass(ptbWild->m_nSid, ptbWild->m_nType);
    if (udwWildClass == EN_WILD_CLASS_NORMAL)
    {
        return TRUE;
    }

    if ((udwWildClass == EN_WILD_CLASS_MONSTER || udwWildClass == EN_WILD_CLASS_LEADER_MONSTER) &&
        ptbWild->m_nExpire_time < udwCurTime)
    {
        return TRUE;
    }

    if (udwWildClass == EN_WILD_CLASS_MONSTER_NEST
        && ptbWild->m_nExpire_time < udwCurTime)
    {
        return TRUE;
    }

    return FALSE;
}
