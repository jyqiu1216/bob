#include "game_info.h"
#include "time_utils.h"
#include "globalres_logic.h"
#include "common_func.h"
#include "common_base.h"
#include "common_logic.h"
#include <cmath>
#include "tool_base.h"
#include "city_base.h"
#include "action_base.h"
#include "msg_base.h"
#include "document.h"
#include "conf_base.h"
#include "func_open.h"
#include "output_conf.h"

TINT32 CCommonLogic::GenAlGiftToPerson(TbPlayer *ptbPlayer, TINT32 dwPackId, SAlGiftList& sAlGiftList, TbLogin* ptbLogin)
{
    CGameInfo* poGameInfo = CGameInfo::GetInstance();
    string strPackId = CCommonFunc::NumToString(dwPackId);

    if(!poGameInfo->m_oJsonRoot["game_al_gift_new"]["pack"].isMember(strPackId))
    {
        return -1;
    }

    TINT32 dwLastGiftIndex = (sAlGiftList.m_dwGiftNum >= MAX_AL_IAP_GIFT_NUM_SVR) ? sAlGiftList.m_dwGiftNum - 1 : sAlGiftList.m_dwGiftNum;
    for(TINT32 dwIdx = dwLastGiftIndex; dwIdx > 0; dwIdx--)
    {
        sAlGiftList.m_atbGifts[dwIdx] = sAlGiftList.m_atbGifts[dwIdx - 1];
        sAlGiftList.m_aucUpdateFlag[dwIdx] = sAlGiftList.m_aucUpdateFlag[dwIdx - 1];
    }
    TINT32 dwCurIndex = 0;
    TbAl_gift *ptbAlGift = &sAlGiftList.m_atbGifts[dwCurIndex];

    TUINT64 uddwCurTime = CTimeUtils::GetCurTimeUs();
    TINT32 dwGiftPoint = poGameInfo->m_oJsonRoot["game_al_gift_new"]["pack"][strPackId]["gift_point"].asInt();

    ptbAlGift->Reset();
    ptbAlGift->Set_Aid(-1 * ptbPlayer->m_nUid);//special
    ptbAlGift->Set_Id(uddwCurTime);
    ptbAlGift->Set_Pack_id(dwPackId);
    ptbAlGift->Set_Gift_point(dwGiftPoint);
    ptbAlGift->Set_Ctime(uddwCurTime);

    sAlGiftList.m_aucUpdateFlag[dwCurIndex] = EN_TABLE_UPDT_FLAG__NEW;
    sAlGiftList.m_dwGiftNum++;

    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("GenPersonAlGift: [aid=%d] [gid=%ld] [pid=%ld] [g_point=%ld] [ctime=%ld]",
        ptbAlGift->m_nAid, ptbAlGift->m_nId, ptbAlGift->m_nPack_id,
        ptbAlGift->m_nGift_point, ptbAlGift->m_nCtime));

    SNoticInfo stNoticInfo;
    stNoticInfo.Reset();
    stNoticInfo.SetValue(EN_NOTI_ID__AL_GIFT,
        "", "",
        0, 0,
        0, 0,
        0, "", 0, CDocument::GetLang(ptbLogin->m_nLang));
    CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), ptbPlayer->m_nSid, ptbPlayer->m_nUid, stNoticInfo);

    return 0;
}

TINT32 CCommonLogic::GenAlGiftToAlliance(TINT64 ddwAid, TINT32 dwPackId, TINT64 ddwSrcUid, SAlGiftList& sAlGiftList, TbLogin* ptbLogin)
{
    CGameInfo* poGameInfo = CGameInfo::GetInstance();
    string strPackId = CCommonFunc::NumToString(dwPackId);

    if(!poGameInfo->m_oJsonRoot["game_al_gift_new"]["pack"].isMember(strPackId))
    {
        return -1;
    }

    TINT32 dwLastGiftIndex = (sAlGiftList.m_dwGiftNum >= MAX_AL_IAP_GIFT_NUM_SVR) ? sAlGiftList.m_dwGiftNum - 1 : sAlGiftList.m_dwGiftNum;
    for(TINT32 dwIdx = dwLastGiftIndex; dwIdx > 0; dwIdx--)
    {
        sAlGiftList.m_atbGifts[dwIdx] = sAlGiftList.m_atbGifts[dwIdx - 1];
        sAlGiftList.m_aucUpdateFlag[dwIdx] = sAlGiftList.m_aucUpdateFlag[dwIdx - 1];
    }
    TINT32 dwCurIndex = 0;
    TbAl_gift *ptbAlGift = &sAlGiftList.m_atbGifts[dwCurIndex];

    TUINT64 uddwCurTime = CTimeUtils::GetCurTimeUs();
    TINT32 dwGiftPoint = poGameInfo->m_oJsonRoot["game_al_gift_new"]["pack"][strPackId]["gift_point"].asInt();

    ptbAlGift->Reset();
    ptbAlGift->Set_Aid(ddwAid);
    ptbAlGift->Set_Id(uddwCurTime);
    ptbAlGift->Set_Pack_id(dwPackId);
    ptbAlGift->Set_Gift_point(dwGiftPoint);
    ptbAlGift->Set_Ctime(uddwCurTime);

    sAlGiftList.m_aucUpdateFlag[dwCurIndex] = EN_TABLE_UPDT_FLAG__NEW;
    sAlGiftList.m_dwGiftNum++;

    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("GenFightAlGift: [aid=%d] [gid=%ld] [pid=%ld] [g_point=%ld] [ctime=%ld]",
        ptbAlGift->m_nAid, ptbAlGift->m_nId, ptbAlGift->m_nPack_id,
        ptbAlGift->m_nGift_point, ptbAlGift->m_nCtime));

    SNoticInfo stNoticInfo;
    stNoticInfo.Reset();
    stNoticInfo.SetValue(EN_NOTI_ID__AL_GIFT,
        "", "",
        0, 0,
        0, 0,
        0, "", 0, CDocument::GetLang(ptbLogin->m_nLang));
    CMsgBase::SendNotificationAlliance(CConfBase::GetString("tbxml_project"), ptbLogin->m_nSid, ddwSrcUid, ddwAid, stNoticInfo);

    return 0;
}

TINT32 CCommonLogic::GenEventAlGift(TbPlayer *ptbPlayer, TINT32 dwPackId, SAlGiftList& sAlGiftList, TUINT32 udwEventType, TbLogin* ptbLogin)
{
    CGameInfo* poGameInfo = CGameInfo::GetInstance();
    string strPackId = CCommonFunc::NumToString(dwPackId);

    if(!poGameInfo->m_oJsonRoot["game_al_gift_new"]["pack"].isMember(strPackId))
    {
        return -1;
    }

    TINT32 dwLastGiftIndex = (sAlGiftList.m_dwGiftNum >= MAX_AL_IAP_GIFT_NUM_SVR) ? sAlGiftList.m_dwGiftNum - 1 : sAlGiftList.m_dwGiftNum;
    for(TINT32 dwIdx = dwLastGiftIndex; dwIdx > 0; dwIdx--)
    {
        sAlGiftList.m_atbGifts[dwIdx] = sAlGiftList.m_atbGifts[dwIdx - 1];
        sAlGiftList.m_aucUpdateFlag[dwIdx] = sAlGiftList.m_aucUpdateFlag[dwIdx - 1];
    }
    TINT32 dwCurIndex = 0;
    TbAl_gift *ptbAlGift = &sAlGiftList.m_atbGifts[dwCurIndex];

    TUINT64 uddwCurTime = CTimeUtils::GetCurTimeUs();
    TINT32 dwGiftPoint = poGameInfo->m_oJsonRoot["game_al_gift_new"]["pack"][strPackId]["gift_point"].asInt();

    ptbAlGift->Reset();
    ptbAlGift->Set_Aid(-1 * ptbPlayer->m_nUid);  //给个人的gift
    ptbAlGift->Set_Id(uddwCurTime);
    ptbAlGift->Set_Pack_id(dwPackId);
    ptbAlGift->Set_Gift_point(dwGiftPoint);
    ptbAlGift->Set_Ctime(uddwCurTime);

    sAlGiftList.m_aucUpdateFlag[dwCurIndex] = EN_TABLE_UPDT_FLAG__NEW;
    sAlGiftList.m_dwGiftNum++;

    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("GenEventAlGift: [aid=%d] [gid=%ld] [pid=%ld] [g_point=%ld] [ctime=%ld]",
        ptbAlGift->m_nAid, ptbAlGift->m_nId, ptbAlGift->m_nPack_id,
        ptbAlGift->m_nGift_point, ptbAlGift->m_nCtime));

    SNoticInfo stNoticInfo;
    stNoticInfo.Reset();
    stNoticInfo.SetValue(EN_NOTI_ID__AL_GIFT,
        "", "",
        0, 0,
        0, 0,
        0, "", 0, CDocument::GetLang(ptbLogin->m_nLang));
    CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), ptbPlayer->m_nSid, ptbPlayer->m_nUid, stNoticInfo);

    return 0;
}

TINT32 CCommonLogic::GenIapAlGift(TbAlliance* ptbAlliance, TINT32 dwGem, TINT64 ddwSrcUid, SAlGiftList& sAlGiftList, TbLogin* ptbLogin)
{
    const Json::Value &jAlGift = CGameInfo::GetInstance()->m_oJsonRoot["game_al_gift_new"];
    if(!jAlGift["iap"].isMember(CCommonFunc::NumToString(dwGem)))
    {
        return -1;
    }
    TINT32 dwAlGiftLevel = CCommonBase::GetAlGiftLevel(ptbAlliance);
    if(dwAlGiftLevel==0)
    {
        dwAlGiftLevel++;
    }
    TINT32 dwPackId = jAlGift["iap"][CCommonFunc::NumToString(dwGem)][dwAlGiftLevel-1].asInt();

    if(!jAlGift["pack"].isMember(CCommonFunc::NumToString(dwPackId)))
    {
        return -2;
    }

    TINT32 dwLastGiftIndex = (sAlGiftList.m_dwGiftNum >= MAX_AL_IAP_GIFT_NUM_SVR) ? sAlGiftList.m_dwGiftNum - 1 : sAlGiftList.m_dwGiftNum;
    for(TINT32 dwIdx = dwLastGiftIndex; dwIdx > 0; dwIdx--)
    {
        sAlGiftList.m_atbGifts[dwIdx] = sAlGiftList.m_atbGifts[dwIdx - 1];
        sAlGiftList.m_aucUpdateFlag[dwIdx] = sAlGiftList.m_aucUpdateFlag[dwIdx - 1];
    }
    TINT32 dwCurIndex = 0;
    TbAl_gift *ptbAlGift = &sAlGiftList.m_atbGifts[dwCurIndex];

    TUINT64 uddwCurTime = CTimeUtils::GetCurTimeUs();
    TINT32 dwGiftPoint = jAlGift["pack"][CCommonFunc::NumToString(dwPackId)]["gift_point"].asInt();

    ptbAlGift->Reset();
    ptbAlGift->Set_Aid(ptbAlliance->m_nAid);
    ptbAlGift->Set_Id(uddwCurTime);
    ptbAlGift->Set_Pack_id(dwPackId);
    ptbAlGift->Set_Gift_point(dwGiftPoint);
    ptbAlGift->Set_Ctime(uddwCurTime);

    sAlGiftList.m_aucUpdateFlag[dwCurIndex] = EN_TABLE_UPDT_FLAG__NEW;
    sAlGiftList.m_dwGiftNum++;

    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("GenIapAlGift: [aid=%d] [gid=%ld] [pid=%ld] [g_point=%ld] [ctime=%ld]",
        ptbAlGift->m_nAid, ptbAlGift->m_nId, ptbAlGift->m_nPack_id,
        ptbAlGift->m_nGift_point, ptbAlGift->m_nCtime));

    SNoticInfo stNoticInfo;
    stNoticInfo.Reset();
    stNoticInfo.SetValue(EN_NOTI_ID__AL_GIFT,
        "", "",
        0, 0,
        0, 0,
        0, "", 0, CDocument::GetLang(ptbLogin->m_nLang));
    CMsgBase::SendNotificationAlliance(CConfBase::GetString("tbxml_project"), ptbLogin->m_nSid, ddwSrcUid, ptbAlliance->m_nAid, stNoticInfo);

    return 0;
}

TINT32 CCommonLogic::ComputeCanHelpAlAction(SUserInfo * pstUserInfo)
{
    TbPlayer &tbPlayer = pstUserInfo->m_tbPlayer;

    // 计算能帮助的联盟action数
    pstUserInfo->m_udwCanHelpTaskNum = 0;
    pstUserInfo->m_udwAlCanHelpActionNum = 0;
    for(TUINT32 udwIdx = 0; udwIdx < pstUserInfo->m_udwSelfAlActionNum; ++udwIdx)
    {
        TbAlliance_action* ptbHelpAction = &pstUserInfo->m_atbSelfAlAction[udwIdx];
        TbAl_help& tbAl_help = pstUserInfo->m_atbAl_help[ptbHelpAction->m_nId % MAX_AL_HELP_LIST_NUM];
        SAlHelpList& stAlHelpList = tbAl_help.m_bList[0];

        if(ptbHelpAction->m_nCan_help_num == 0)
        {
            continue;
        }

        //只有build，research，hospital的等待时间可以免费加速
        if(EN_ACTION_MAIN_CLASS__BUILDING == ptbHelpAction->m_nMclass)
        {
        }
        else if (EN_ACTION_MAIN_CLASS__TRAIN_NEW == ptbHelpAction->m_nMclass)
        {
        }
        else if(ptbHelpAction->m_nMclass == EN_ACTION_MAIN_CLASS__EQUIP && ptbHelpAction->m_nSclass == EN_ACTION_SEC_CLASS__EQUIP_UPGRADE)
        {
        }
        else if (ptbHelpAction->m_nMclass == EN_ACTION_MAIN_CLASS__DRAGON)
        {
        }
        else
        {
            continue;
        }

        if(stAlHelpList.FindNode(ptbHelpAction->m_nId)) //已经帮助过
        {
            continue;
        }
        //不是自己联盟的任务
        if(ptbHelpAction->m_nSal != tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET)
        {
            continue;
        }
        //不能再接受帮助
        if(ptbHelpAction->m_nHelped_num >= ptbHelpAction->m_nCan_help_num)
        {
            if(ptbHelpAction->m_nSal != 0)
            {
                ptbHelpAction->Set_Sal(0, UPDATE_ACTION_TYPE_DELETE);
                //ptbHelpAction->DeleteField(TbALLIANCE_ACTION_FIELD_SAL);
                pstUserInfo->m_aucSelfAlActionFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
            }
            if(ptbHelpAction->m_nSuid != tbPlayer.m_nUid)//自己的任务帮助满了也要展示
            {
                continue;
            }
        }
        if(ptbHelpAction->m_nSuid != tbPlayer.m_nUid)
        {
            pstUserInfo->m_udwCanHelpTaskNum++;
        }
        pstUserInfo->m_patbAlCanHelpAction[pstUserInfo->m_udwAlCanHelpActionNum] = ptbHelpAction;
        pstUserInfo->m_udwAlCanHelpActionNum++;
    }

   return 0;
}

TBOOL CCommonLogic::CheckBuildingCollision(TbCity* ptbCity)
{
    set<TINT32> setPosSet;
    TBOOL bConflict = FALSE;
    for(TUINT32 udwIdx = 0; udwIdx < ptbCity->m_bBuilding.m_udwNum; ++udwIdx)
    {
        SCityBuildingNode* pstNode = &ptbCity->m_bBuilding[udwIdx];
        if(pstNode->m_ddwLevel == 0)
        {
            continue;
        }

        TINT32 dwCenterPos = pstNode->m_ddwPos;
        TINT32 dwSize = CToolBase::GetBuildingSize(pstNode->m_ddwType);
        bConflict = CCommonLogic::AddBuildingPos(dwCenterPos, dwSize, setPosSet);
        if(bConflict)
        {
            return bConflict;
        }
    }
    return bConflict;;
}

TBOOL CCommonLogic::CheckBuildingCollision(TbCity* ptbCity, SCityBuildingNode stNewBuilding)
{
    set<TINT32> setPosSet;
    TBOOL bConflict = FALSE;
    for(TUINT32 udwIdx = 0; udwIdx < ptbCity->m_bBuilding.m_udwNum; ++udwIdx)
    {
        SCityBuildingNode* pstNode = &ptbCity->m_bBuilding[udwIdx];
        if(pstNode->m_ddwLevel == 0)
        {
            continue;
        }

        TINT32 dwCenterPos = pstNode->m_ddwPos;
        TINT32 dwSize = CToolBase::GetBuildingSize(pstNode->m_ddwType);
        TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("CCommonLogic::CheckBuildingCollision: building conflit[pos=%u,id=%u,size=%u]",
                                                          dwCenterPos, pstNode->m_ddwType, dwSize));
        bConflict = CCommonLogic::AddBuildingPos(dwCenterPos, dwSize, setPosSet);
        if(bConflict)
        {
            return bConflict;
        }
    }

    TINT32 dwCenterPos = stNewBuilding.m_ddwPos;
    TINT32 dwSize = CToolBase::GetBuildingSize(stNewBuilding.m_ddwType);
    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("CCommonLogic::CheckBuildingCollision: building conflit[pos=%u,id=%u,size=%u]",
                                                      dwCenterPos, stNewBuilding.m_ddwType, dwSize));
    bConflict = CCommonLogic::AddBuildingPos(dwCenterPos, dwSize, setPosSet);
    return bConflict;
}

TBOOL CCommonLogic::CheckBuildingCollision(TbCity* ptbCity, std::vector<SCityBuildingNode>& vecNewBuilding, TUINT32 udwSeq)
{
    set<TINT32> setPosSet;
    TBOOL bConflict = FALSE;
    for(TUINT32 udwIdx = 0; udwIdx < ptbCity->m_bBuilding.m_udwNum; ++udwIdx)
    {
        SCityBuildingNode* pstNode = &ptbCity->m_bBuilding[udwIdx];
        if(pstNode->m_ddwLevel == 0)
        {
            continue;
        }

        TINT32 dwCenterPos = pstNode->m_ddwPos;
        TINT32 dwSize = CToolBase::GetBuildingSize(pstNode->m_ddwType);
        bConflict = CCommonLogic::AddBuildingPos(dwCenterPos, dwSize, setPosSet);
        if(bConflict)
        {
            TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("CheckBuildingCollision: building conflit[pos=%u,id=%u,size=%u][seq=%u]",
                dwCenterPos, pstNode->m_ddwType, dwSize, udwSeq));
            return bConflict;
        }
    }

    for(TUINT32 udwIdx = 0; udwIdx < vecNewBuilding.size(); ++udwIdx)
    {
        TINT32 dwCenterPos = vecNewBuilding[udwIdx].m_ddwPos;
        TINT32 dwSize = CToolBase::GetBuildingSize(vecNewBuilding[udwIdx].m_ddwType);
        bConflict = CCommonLogic::AddBuildingPos(dwCenterPos, dwSize, setPosSet);
        if(bConflict)
        {
            TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("CheckBuildingCollision: building conflit[pos=%u,id=%u,size=%u][seq=%u]",
                dwCenterPos, vecNewBuilding[udwIdx].m_ddwType, dwSize, udwSeq));
            return bConflict;
        }
    }

    return bConflict;
}

TBOOL CCommonLogic::GetBuildingPos(TINT32 dwCenterPos, TINT32 dwSize, std::set<TINT32>& posSet)
{
    TBOOL bConflict = FALSE;
    if(dwSize == 1)
    {
        if(posSet.count(dwCenterPos))
        {
            bConflict = TRUE;
        }
        posSet.insert(dwCenterPos);
        return bConflict;
    }

    BuildingPoint CenterPoint = CCommonLogic::BuildingPosToPoint(dwCenterPos);
    TINT32 dwLeft = 0;
    if(dwSize % 2 == 0)
    {
        dwLeft = CenterPoint.x - (dwSize / 2 - 1);
    }
    else
    {
        dwLeft = CenterPoint.x - (dwSize / 2);
    }

    for(TINT32 dwI = dwSize; dwI > 0; dwI--)
    {
        TINT32 dwOffset = (CenterPoint.y % 2 == 0) ? ceil((dwSize - dwI) / 2.0f) : (dwSize - dwI) / 2;
        TINT32 dwStartX = dwLeft + dwOffset;

        TINT32 dwY = CenterPoint.y - (dwSize - dwI);
        for(TINT32 dwJ = 0; dwJ < dwI; dwJ++)
        {
            TINT32 dwPos = CCommonLogic::BuildingPointToPos(dwStartX + dwJ, dwY);
            if(posSet.count(dwPos))
            {
                bConflict = TRUE;
            }
            posSet.insert(dwPos);
        }

        if((dwSize - dwI) != 0)
        {
            dwY = CenterPoint.y + (dwSize - dwI);
            for(TINT32 dwJ = 0; dwJ < dwI; dwJ++)
            {
                TINT32 dwPos = CCommonLogic::BuildingPointToPos(dwStartX + dwJ, dwY);
                if(posSet.count(dwPos))
                {
                    bConflict = TRUE;
                }
                posSet.insert(dwPos);
            }
        }
    }
    return bConflict;
}

BuildingPoint CCommonLogic::BuildingPosToPoint(TINT32 dwPos)
{
    BuildingPoint point;
    point.x = dwPos / (MAX_BUILDING_SIZE * 10);
    point.y = dwPos % (MAX_BUILDING_SIZE * 10);

    point.x = (point.x > MAX_BUILDING_SIZE) ? (MAX_BUILDING_SIZE - point.x) : point.x;
    point.y = (point.y > MAX_BUILDING_SIZE) ? (MAX_BUILDING_SIZE - point.y) : point.y;

    return point;
}

TINT32 CCommonLogic::BuildingPosToMapPos(TINT32 dwBpos)
{
    TINT32 dwX = dwBpos / (MAX_BUILDING_SIZE * 10);
    TINT32 dwY = dwBpos % (MAX_BUILDING_SIZE * 10);

    dwX = (dwX > MAX_BUILDING_SIZE) ? (MAX_BUILDING_SIZE - dwX) : dwX;
    dwY = (dwY > MAX_BUILDING_SIZE) ? (MAX_BUILDING_SIZE - dwY) : dwY;

    return dwX * 1000 + dwY;
}

TINT32 CCommonLogic::BuildingPointToPos(BuildingPoint point)
{
    point.x = (point.x < 0) ? ((MAX_BUILDING_SIZE - point.x) * MAX_BUILDING_SIZE * 10) : (point.x * MAX_BUILDING_SIZE * 10);
    point.y = (point.y < 0) ? (MAX_BUILDING_SIZE - point.y) : point.y;
    return point.x + point.y;
}

TINT32 CCommonLogic::BuildingPointToPos(TINT32 x, TINT32 y)
{
    x = (x < 0) ? ((MAX_BUILDING_SIZE - x) * MAX_BUILDING_SIZE * 10) : (x * MAX_BUILDING_SIZE * 10);
    y = (y < 0) ? (MAX_BUILDING_SIZE - y) : y;
    return x + y;
}

TBOOL CCommonLogic::AddBuildingPos(TINT32 dwCenterPos, TINT32 dwSize, std::set<TINT32>& posSet)
{
    TBOOL bConflict = FALSE;
    if(dwSize == 1)
    {
        if(posSet.count(dwCenterPos))
        {
            bConflict = TRUE;
            return bConflict;
        }
        posSet.insert(dwCenterPos);
        return bConflict;
    }

    BuildingPoint CenterPoint = CCommonLogic::BuildingPosToPoint(dwCenterPos);
    TINT32 dwLeft = 0;
    if(dwSize % 2 == 0)
    {
        dwLeft = CenterPoint.x - (dwSize / 2 - 1);
    }
    else
    {
        dwLeft = CenterPoint.x - (dwSize / 2);
    }

    for(TINT32 dwI = dwSize; dwI > 0; dwI--)
    {
        TINT32 dwOffset = (CenterPoint.y % 2 == 0) ? ceil((dwSize - dwI) / 2.0f) : (dwSize - dwI) / 2;
        TINT32 dwStartX = dwLeft + dwOffset;

        TINT32 dwY = CenterPoint.y - (dwSize - dwI);
        for(TINT32 dwJ = 0; dwJ < dwI; dwJ++)
        {
            TINT32 dwPos = CCommonLogic::BuildingPointToPos(dwStartX + dwJ, dwY);
            if(posSet.count(dwPos))
            {
                bConflict = TRUE;
                return bConflict;
            }
            posSet.insert(dwPos);
        }

        if((dwSize - dwI) != 0)
        {
            dwY = CenterPoint.y + (dwSize - dwI);
            for(TINT32 dwJ = 0; dwJ < dwI; dwJ++)
            {
                TINT32 dwPos = CCommonLogic::BuildingPointToPos(dwStartX + dwJ, dwY);
                if(posSet.count(dwPos))
                {
                    bConflict = TRUE;
                    return bConflict;
                }
                posSet.insert(dwPos);
            }
        }
    }
    return bConflict;
}

TBOOL CCommonLogic::GetWildPos(TINT32 dwCenterPos, TINT32 dwSize, std::set<TINT32>& posSet)
{
    int c_x = dwCenterPos / MAP_X_Y_POS_COMPUTE_OFFSET;
    int c_y = dwCenterPos % MAP_X_Y_POS_COMPUTE_OFFSET;

    int c_r = dwSize;

    int x, y;

//     if (c_r % 2 == 0)
//     {
//         c_x += 1;
//     }

    for (int i = -1 * c_r; i <= c_r; i++)
    {
        x = c_x + i;
        for (int j = -1 * c_r; j <= c_r; j++)
        {
            y = c_y + j;
            if (j <= 0 && i <= 0 && -1 * (i + j) <= c_r)
            {
                //ok
            }
            else if (j <= 0 && i >= 0 && i - j <= c_r)
            {
                //ok
            }
            else if (j >= 0 && i >= 0 && i + j <= c_r)
            {
                //ok
            }
            else if (j >= 0 && i <= 0 && j - i <= c_r)
            {
                //ok
            }
            else
            {
                continue;
            }

            if ((x + y) % 2 == 0)
            {
                posSet.insert(x * MAP_X_Y_POS_COMPUTE_OFFSET + y);
            }
        }
    }

    return TRUE;
}

TINT32 CCommonLogic::GenObstle(TbCity* ptbCity, TUINT32 udwId, std::vector<TINT32> *pstInSet)
{
    TUINT32 udwSize = CGameInfo::GetInstance()->m_oJsonRoot["game_building"][CCommonFunc::NumToString(udwId)]["a"]["a11"].asUInt();

    std::set<TINT32> posSet;
    std::vector<TINT32> posCenterSet;
    posCenterSet.clear();

    for(vector<TINT32>::iterator it = pstInSet->begin(); it != pstInSet->end(); ++it)
    {
        posSet.clear();
        TBOOL bIsCenterPos = TRUE;
        CCommonLogic::GetBuildingPos(*it, udwSize, posSet);
        //检查建筑的每个坐标是否在InSet里
        for(set<TINT32>::iterator itTmp = posSet.begin(); itTmp != posSet.end(); ++itTmp)
        {
            TBOOL bInSet = FALSE;
            for(vector<TINT32>::iterator tmpIt = pstInSet->begin(); tmpIt != pstInSet->end(); ++tmpIt)
            {
                if(*tmpIt == *itTmp)
                {
                    bInSet = true;
                }
            }
            if(bInSet == FALSE)
            {
                bIsCenterPos = FALSE;
            }
        }
        if(!bIsCenterPos)
        {
            continue;
        }
        posCenterSet.push_back(*it);
    }
    if(posCenterSet.size() == 0)
    {
        //坐标集中没有可用的坐标来生成障碍物
        return 0;
    }
    //从可作为center的坐标中随机选取一个出来 作为中心坐标
    TUINT32 udwIdx = rand() % posCenterSet.size();
    TUINT32 udwCentPos = posCenterSet[udwIdx];

    posSet.clear();
    CCommonLogic::GetBuildingPos(udwCentPos, udwSize, posSet);
    CCityBase::AddBuilding(udwCentPos, udwId, 1, *ptbCity);

    //从坐标集中删掉已经占用的坐标
    for(vector<TINT32>::iterator it = pstInSet->begin(); it != pstInSet->end();)
    {
        TBOOL bRemove = FALSE;
        for(set<TINT32>::iterator setIt = posSet.begin(); setIt != posSet.end(); ++setIt)
        {
            if(*setIt == *it)
            {
                bRemove = TRUE;
                break;
            }
        }
        if(bRemove)
        {
            it = pstInSet->erase(it);
        }
        else
        {
            it++;
        }
    }
    return 0;
}

TINT32 CCommonLogic::AbandonThrone(TbAlliance* ptbAlliance, TbThrone *ptbThrone, TbMap* ptbWild)
{
    if (ptbThrone->m_nAlid != ptbAlliance->m_nAid)
    {
        return 0;
    }

    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    if (ptbAlliance->m_nLast_occupy_time != 0)
    {
        ptbAlliance->Set_Has_occupied_time(ptbAlliance->m_nHas_occupied_time + udwCurTime - ptbAlliance->m_nLast_occupy_time + 1); //后台认为..占领也算1s...
        ptbAlliance->Set_Last_occupy_time(0);
    }

    if (ptbThrone->m_nStatus == EN_THRONE_STATUS__CONTEST_PERIOD && ptbThrone->m_jRank_info.isArray())
    {
        for (TUINT32 udwIdx = 0; udwIdx < ptbThrone->m_jRank_info.size(); udwIdx++)
        {
            if (ptbThrone->m_jRank_info[udwIdx][0U].asInt() == ptbThrone->m_nAlid)
            {
                if (ptbThrone->m_jRank_info[udwIdx][1U].asString() != ptbAlliance->m_sAl_nick_name)
                {
                    ptbThrone->m_jRank_info[udwIdx][1U] = ptbAlliance->m_sAl_nick_name;
                }
                if (ptbThrone->m_jRank_info[udwIdx][2U].asString() != ptbAlliance->m_sName)
                {
                    ptbThrone->m_jRank_info[udwIdx][2U] = ptbAlliance->m_sName;
                }
                ptbThrone->m_jRank_info[udwIdx][3U] = ptbThrone->m_jRank_info[udwIdx][3U].asUInt() + udwCurTime - ptbThrone->m_jRank_info[udwIdx][4U].asUInt();
                if (ptbThrone->m_jRank_info[udwIdx][3U].asUInt() >= CCommonBase::GetGameBasicVal(EN_GAME_BASIC_THRONE_FIGHT_TIME))
                {
                    ptbThrone->m_jRank_info[udwIdx][3U] = CCommonBase::GetGameBasicVal(EN_GAME_BASIC_THRONE_FIGHT_TIME) - 1; //做保护...
                }
                ptbThrone->m_jRank_info[udwIdx][4U] = 0;
                ptbThrone->SetFlag(TbTHRONE_FIELD_RANK_INFO);
                break;
            }
        }
    }
    else if (ptbThrone->m_nStatus == EN_THRONE_STATUS__PEACE_TIME)
    {
        ptbThrone->Set_Rank_info(Json::Value(Json::arrayValue));
    }

    ptbThrone->Set_Alid(-1);
    ptbThrone->Set_Owner_id(0);
    ptbThrone->Set_Owner_cid(0);
    ptbThrone->Set_Tax_id(0);
    ptbThrone->Set_Defending_num(0);
    ptbThrone->Set_Defending_troop_num(0);
    ptbThrone->Set_Defending_troop_force(0);
    ptbThrone->Set_Reinforce_num(0);
    ptbThrone->Set_Reinforce_troop_num(0);
    ptbThrone->Set_Reinforce_troop_force(0);
    ptbThrone->Set_Occupy_time(udwCurTime);

    ptbWild->Set_Uid(0);
    ptbWild->Set_Uname("");
    ptbWild->Set_Alid(0);
    ptbWild->Set_Alname("");
    ptbWild->Set_Al_nick("");
    ptbWild->Set_Al_flag(0);
    ptbWild->Set_Name_update_time(udwCurTime);

    return 0;
}

TVOID CCommonLogic::GenPrisonReport(TbMarch_action* ptbPrison, TbPlayer* ptbSaver, TINT32 dwReportType, TINT32 dwReportResult, TbReport* ptbReport)
{
    ptbReport->Set_Type(dwReportType);
    ptbReport->Set_Result(dwReportResult);
    ptbReport->Set_Time(CTimeUtils::GetUnixTime());
    ptbReport->Set_Sid(ptbPrison->m_nSid);

    Json::Value jsonContent = Json::Value(Json::objectValue);

    jsonContent["captor"] = Json::Value(Json::objectValue);
    jsonContent["captor"]["uid"] = ptbPrison->m_nTuid;
    jsonContent["captor"]["al_nick"] = ptbPrison->m_bPrison_param[0].szTargetAlNick;
    jsonContent["captor"]["uname"] = ptbPrison->m_bPrison_param[0].szTargetUserName;

    if(ptbSaver)
    {
        jsonContent["saver"] = Json::Value(Json::objectValue);
        jsonContent["saver"]["uid"] = ptbSaver->m_nUid;
        jsonContent["saver"]["al_nick"] = ptbSaver->m_sAl_nick_name;
        jsonContent["saver"]["uname"] = ptbSaver->m_sUin;
    }
    else
    {
        jsonContent["saver"] = Json::Value(Json::objectValue);
        jsonContent["saver"]["uid"] = 0;
        jsonContent["saver"]["al_nick"] = "";
        jsonContent["saver"]["uname"] = "";
    }

    jsonContent["dragon_list"] = Json::Value(Json::objectValue);

    Json::Value jsonOne = Json::Value(Json::objectValue);
    jsonOne["al_nick"] = ptbPrison->m_bPrison_param[0].szSourceAlNick;
    jsonOne["uname"] = ptbPrison->m_bPrison_param[0].szSourceUserName;
    jsonOne["dragon_lv"] = ptbPrison->m_bPrison_param[0].stDragon.m_ddwLevel;
    jsonOne["dragon_name"] = ptbPrison->m_bPrison_param[0].stDragon.m_szName;

    jsonContent["dragon_list"][CCommonFunc::NumToString(ptbPrison->m_nSuid)] = jsonOne;

    Json::FastWriter jsonWriter;
    jsonWriter.omitEndingLineFeed();
    ptbReport->Set_Content(jsonWriter.write(jsonContent));
}

TUINT32 CCommonLogic::GetPurchaseAbility(TUINT32 udwGemNum)
{
    TUINT32 udwAbility = EN_PURCHASE_ABILITY__LV0;
    if (udwGemNum >= 500)
    {
        udwAbility = EN_PURCHASE_ABILITY__LV1;
    }
    if (udwGemNum >= 7500)
    {
        udwAbility = EN_PURCHASE_ABILITY__LV2;
    }
    if (udwGemNum >= 18000)
    {
        udwAbility = EN_PURCHASE_ABILITY__LV3;
    }
    return udwAbility;
}

TUINT32 CCommonLogic::GetIapPay(const string& strItemId)
{
    TUINT32 udwPay = 0;

    if (strcmp(strItemId.c_str(), "50gems9") == 0)
    {
        udwPay = 5;
    }
    else if (strcmp(strItemId.c_str(), "100gems9") == 0)
    {
        udwPay = 10;
    }
    else if (strcmp(strItemId.c_str(), "240gems9") == 0)
    {
        udwPay = 20;
    }
    else if (strcmp(strItemId.c_str(), "665gems9") == 0)
    {
        udwPay = 50;
    }
    else if (strcmp(strItemId.c_str(), "1600gems9") == 0)
    {
        udwPay = 100;
    }

    return udwPay;
}

TUINT32 CCommonLogic::GetIapPayCent(const string& strItemId)
{
    TUINT32 udwPay = 0;

    if (strcmp(strItemId.c_str(), "50gems9") == 0)
    {
        udwPay = 499;
    }
    else if (strcmp(strItemId.c_str(), "100gems9") == 0)
    {
        udwPay = 999;
    }
    else if (strcmp(strItemId.c_str(), "240gems9") == 0)
    {
        udwPay = 1999;
    }
    else if (strcmp(strItemId.c_str(), "665gems9") == 0)
    {
        udwPay = 4999;
    }
    else if (strcmp(strItemId.c_str(), "1600gems9") == 0)
    {
        udwPay = 9999;
    }

    return udwPay;
}

TVOID CCommonLogic::AddBookMark(SUserInfo *pstUser, TUINT64 udwBookmarkPos, TUINT8 ucBookmarkType, string pszBookmarkNick)
{
    if(pstUser->m_udwBookmarkNum >= MAX_BOOKMARK_NUM)
    {
        return;
    }
    TbBookmark *pstBookmark = &pstUser->m_atbBookmark[pstUser->m_udwBookmarkNum];
    // 2. add into list

    TUINT64 uddwPos = pstUser->m_tbPlayer.m_nSid;
    uddwPos = (uddwPos << 32) + udwBookmarkPos;

    pstBookmark->Reset();
    pstBookmark->Set_Uid(pstUser->m_tbPlayer.m_nUid);
    pstBookmark->Set_Pos(uddwPos);
    pstBookmark->Set_Flag(ucBookmarkType);
    pstBookmark->Set_Nick(pszBookmarkNick);
    pstBookmark->Set_Time(CTimeUtils::GetUnixTime());
    pstUser->m_aucBookMarkFlag[pstUser->m_udwBookmarkNum] = EN_TABLE_UPDT_FLAG__NEW;
    pstUser->m_udwBookmarkNum++;

    return;
}

TBOOL CCommonLogic::CheckFuncOpen(SUserInfo *pstUser, SCityInfo *pstCity, TUINT32 udwType)
{
    if(!pstUser || !pstCity)
    {
        return FALSE;
    }

    TUINT32 udwSid = pstUser->m_tbLogin.m_nSid;
    TBOOL bSvrOpen = CCommonLogic::CheckSvrOpen(udwSid, udwType);
    if(!bSvrOpen)
    {
        return FALSE;
    }
    const Json::Value &jFuncOpen = CGameInfo::GetInstance()->m_oJsonRoot["game_func_open"];
    if(!jFuncOpen.isMember(CCommonFunc::NumToString(udwType)))
    {
        return FALSE;
    }

    TBOOL bAllConfirmCon = CCommonLogic::CheckAllConfirmCon(pstUser, pstCity, udwType);
    TBOOL bOneConfirmCon = CCommonLogic::CheckOneConfirmCon(pstUser, pstCity, udwType);

    if(bAllConfirmCon || bOneConfirmCon)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
    return FALSE;
}

TBOOL CCommonLogic::CheckAllConfirmCon(SUserInfo *pstUser, SCityInfo *pstCity, TUINT32 udwType)
{
    if(!pstUser || !pstCity)
    {
        return FALSE;
    }

    const Json::Value &jFuncOpen = CGameInfo::GetInstance()->m_oJsonRoot["game_func_open"];
    if(!jFuncOpen.isMember(CCommonFunc::NumToString(udwType)))
    {
        return FALSE;
    }

    TBOOL bAllConfirmCon = TRUE;
    for(TUINT32 udwIdx = 0; udwIdx < jFuncOpen[CCommonFunc::NumToString(udwType)]["r0"].size(); ++udwIdx)
    {
        if (jFuncOpen[CCommonFunc::NumToString(udwType)]["r0"][udwIdx].size() == 0)
        {
            continue;
        }
        TUINT32 udwConType = jFuncOpen[CCommonFunc::NumToString(udwType)]["r0"][udwIdx][0U].asUInt();
        TUINT32 udwId = jFuncOpen[CCommonFunc::NumToString(udwType)]["r0"][udwIdx][1U].asUInt();
        TUINT32 udwNum = jFuncOpen[CCommonFunc::NumToString(udwType)]["r0"][udwIdx][2U].asUInt();

        switch(udwConType)
        {
        case EN_GLOBALRES_TYPE_BUILDING_LV:
            if(udwNum > CCityBase::GetBuildingLevelById(&pstCity->m_stTblData, udwId))
            {
                bAllConfirmCon = FALSE;
            }
            break;
        case EN_GLOBALRES_TYPE_LORD_LV:
            if(udwNum > pstUser->m_tbPlayer.m_nLevel)
            {
                bAllConfirmCon = FALSE;
            }
            break;
        case EN_GLOBALRES_TYPE_AGE:
            if(udwNum > pstUser->m_tbPlayer.m_nAge)
            {
                bAllConfirmCon = FALSE;
            }
            break;
        case EN_GLOBALRES_TYPE_REGISTER_TIME:
            if(udwNum > (CTimeUtils::GetUnixTime() - pstUser->m_tbLogin.m_nCtime))
            {
                bAllConfirmCon = FALSE;
            }
            break;
        default:
            bAllConfirmCon = FALSE;
            break;
        }
    }

    return bAllConfirmCon;
}

TBOOL CCommonLogic::CheckOneConfirmCon(SUserInfo *pstUser, SCityInfo *pstCity, TUINT32 udwType)
{
    if(!pstUser || !pstCity)
    {
        return FALSE;
    }

    const Json::Value &jFuncOpen = CGameInfo::GetInstance()->m_oJsonRoot["game_func_open"];
    if(!jFuncOpen.isMember(CCommonFunc::NumToString(udwType)))
    {
        return FALSE;
    }
    TBOOL bOneConfirmCon = FALSE;
    for(TUINT32 udwIdx = 0; udwIdx < jFuncOpen[CCommonFunc::NumToString(udwType)]["r1"].size(); ++udwIdx)
    {
        TUINT32 udwConType = jFuncOpen[CCommonFunc::NumToString(udwType)]["r1"][udwIdx][0U].asUInt();
        TUINT32 udwId = jFuncOpen[CCommonFunc::NumToString(udwType)]["r1"][udwIdx][1U].asUInt();
        TUINT32 udwNum = jFuncOpen[CCommonFunc::NumToString(udwType)]["r1"][udwIdx][2U].asUInt();
        switch(udwConType)
        {
        case EN_GLOBALRES_TYPE_BUILDING_LV:
            if(udwNum <= CCityBase::GetBuildingLevelById(&pstCity->m_stTblData, udwId))
            {
                bOneConfirmCon = TRUE;
            }
            break;
        case EN_GLOBALRES_TYPE_LORD_LV:
            if(udwNum <= pstUser->m_tbPlayer.m_nLevel)
            {
                bOneConfirmCon = TRUE;
            }
            break;
        case EN_GLOBALRES_TYPE_AGE:
            if(udwNum <= pstUser->m_tbPlayer.m_nAge)
            {
                bOneConfirmCon = TRUE;
            }
            break;
        case EN_GLOBALRES_TYPE_REGISTER_TIME:
            if(udwNum <= (CTimeUtils::GetUnixTime() - pstUser->m_tbLogin.m_nCtime))
            {
                bOneConfirmCon = TRUE;
            }
            break;
        default:
            bOneConfirmCon = FALSE;
            break;
        }
    }
    return bOneConfirmCon;
}

TBOOL CCommonLogic::CheckSvrOpen(TUINT32 udwSid, TUINT32 udwType)
{
    const Json::Value &JOpenFun = CFuncOpen::GetInstance()->m_oJsonRoot;
    for(TUINT32 udwIdx = 0; udwIdx < JOpenFun.size();++udwIdx)
    {
        string sConType = JOpenFun[udwIdx]["function_id"].asString();
        TUINT32 udwConType = atoi(sConType.c_str());
        if(udwConType == udwType)
        {
            string sSwitch = JOpenFun[udwIdx]["default_switch_type"].asString();
            TUINT32 udwSwitch = atoi(sSwitch.c_str());
            const Json::Value &jList = JOpenFun[udwIdx]["svr_list"];
            if(udwSwitch == 0)
            {
                //默认开放
                if(jList[0].asString() == "null")
                {
                    return TRUE;
                }
                else
                {
                    TBOOL bOpen = TRUE;
                    for(TUINT32 udwSvrIdx = 0; udwSvrIdx < jList.size(); ++udwSvrIdx)
                    {
                        string sSvrClose = jList[udwSvrIdx].asString();
                        TUINT32 udwSvrClose = atoi(sSvrClose.c_str());
                        if(udwSvrClose == udwSid)
                        {
                            bOpen = FALSE;
                            break;
                        }
                    }
                    return bOpen;
                }
            }
            else
            {
                //默认关闭
                if(jList[0].asString() == "null")
                {
                    return FALSE;
                }
                else
                {
                    TBOOL bOpen = FALSE;
                    for(TUINT32 udwSvrIdx = 0; udwSvrIdx < jList.size(); ++udwSvrIdx)
                    {
                        string sSvrOpen = jList[udwSvrIdx].asString();
                        TUINT32 udwSvrOpen = atoi(sSvrOpen.c_str());
                        if(udwSvrOpen == udwSid)
                        {
                            bOpen = TRUE;
                            break;
                        }
                    }
                    return bOpen;
                }
            }
        }
    }
    return FALSE;
}

TINT64 CCommonLogic::GetTableSeq( string sTableName, SUserInfo *pstUser )
{
    COutputConf *poOutput = COutputConf::GetInstace();
    TINT32 dwSeqCheckType = poOutput->GetTableSeqCheckType(sTableName.c_str());
    if(dwSeqCheckType != EN_TABLE_SEQCHECK_TYPE__TABLE)
    {
        return 0;
    }
    
    if(sTableName == "svr_player") return pstUser->m_tbPlayer.m_nSeq;
    if(sTableName == "svr_alliance") return pstUser->m_tbAlliance.m_nSeq;

    assert(0);
    return 0;

    //if(sTableName == "svr_rally_war_info") return pstUser->m_tbBounty.m_nSeq;//----------------------从4张action表中获取
    //if(sTableName == "svr_map") return pstUser->m_tbBounty.m_nSeq;//----------------------不做check，增量更新

    //复杂的表数据——二级表数据
    //if(sTableName == "svr_action_list") return pstUser->m_tbPlayer.m_nSeq;
    //if(sTableName == "svr_p_action_list") return pstUser->m_tbPlayer.m_nSeq;
    //if(sTableName == "svr_al_action_list") return pstUser->m_tbPlayer.m_nSeq;
    //if(sTableName == "svr_al_p_action_list") return pstUser->m_tbPlayer.m_nSeq;

    
    //其他
    //if(sTableName == "svr_login") return pstUser->m_tbLogin.m_nSeq;
    //if(sTableName == "svr_research") return pstUser->m_stCityInfo.m_stTblData.m_nSeq;
    //if(sTableName == "svr_lord_skill") return pstUser->m_tbUserStat.m_nSeq;
    //if(sTableName == "svr_dragon_skill") return pstUser->m_tbUserStat.m_nSeq;
    //if(sTableName == "svr_dragon_monster_skill") return pstUser->m_tbUserStat.m_nSeq;
    //if(sTableName == "svr_daily_login") return pstUser->m_tbUserStat.m_nSeq;
    //if(sTableName == "svr_knight_list") return pstUser->m_stCityInfo.m_stTblData.m_nSeq;
    //if(sTableName == "svr_city_list") return pstUser->m_stCityInfo.m_stTblData.m_nSeq;
    //if(sTableName == "svr_bag") return pstUser->m_tbBackpack.m_nSeq;
    //if(sTableName == "svr_blacklist") return pstUser->m_tbUserStat.m_nSeq;
    //if(sTableName == "svr_bookmark_list") return pstUser->m_tbPlayer.m_nSeq;
    //if(sTableName == "svr_stat") return pstUser->m_tbUserStat.m_nSeq;
    
    //if(sTableName == "svr_tips") return pstUser->m_tbPlayer.m_nSeq;
    //if(sTableName == "svr_title") return pstUser->m_tbPlayer.m_nSeq;
    //if(sTableName == "svr_diplomacy_list") return pstUser->m_tbPlayer.m_nSeq;
    //if(sTableName == "svr_al_store") return pstUser->m_bMark.m_nSeq;
    //if(sTableName == "svr_client_show_flag") return 0; //----tmp
    //if(sTableName == "svr_reward_window") return 0; //----tmp
    //if(sTableName == "svr_buff") return 0;    //-----compute
    //if(sTableName == "svr_buff_without_dragon") return 0;//-----compute
    //if(sTableName == "svr_trade_info") return pstUser->m_tbTrade.m_nSeq;
    //if(sTableName == "svr_mystery_store") return pstUser->m_tbMysteryStore.m_nSeq;
    //if(sTableName == "svr_time_quest") return pstUser->m_tbQuest.m_nSeq;
    //if(sTableName == "svr_equip_xxx") return pstUser->m_tbPlayer.m_nSeq;
    //if(sTableName == "svr_task") return pstUser->m_tbTask.m_nSeq;
    //if(sTableName == "svr_event_reward_window") return pstUser->m_tbPlayer.m_nSeq; //-----tmp
    //if(sTableName == "svr_list") return pstUser->m_tbPlayer.m_nSeq;
    
    //if(sTableName == "svr_broadcast") return pstUser->m_tbPlayer.m_nSeq; //----tmp
    //if(sTableName == "svr_throne_info") return pstUser->m_tbThrone.m_nSeq;//--------------------
    //if(sTableName == "svr_event_info") return pstUser->m_tbPlayer.m_nSeq;//------------------------
    //if(sTableName == "svr_reward_window_new") return pstUser->m_tbPlayer.m_nSeq;//----tmp
    //if(sTableName == "svr_random_reward_info") return pstUser->m_tbPlayer.m_nSeq;//----tmp
    //if(sTableName == "svr_monster_info") return pstUser->m_tbPlayer.m_nSeq;
    //if(sTableName == "svr_trial") return pstUser->m_tbPlayer.m_nSeq;
    //if(sTableName == "svr_compute_res") return pstUser->m_tbPlayer.m_nSeq;//----tmp

    //wall_json
    //if(sTableName == "svr_wall_msg") return pstUser->m_tbPlayer.m_nSeq;

    //report_json
    //svr_report_total_list
    //svr_report_detail_list

    //svr_recommend_player

    //svr_rally_history

    //svr_al_member_list
    //svr_al_request_list
    //svr_player_info_list

    //svr_manor_info
    //svr_prison_info

    //svr_mail_total_list
    //svr_mail_detail_list

    //svr_buffer_info

    //svr_al_assist_list

    //svr_alliance_friend
    //svr_alliance_hostile
    //svr_al_info

    //svr_al_report
}

TBOOL CCommonLogic::HasTitle(TINT64 ddwUid, TbThrone *ptbThrone, STitleInfoList *pstTitle)
{
    TINT64 ddwCurTime = CTimeUtils::GetUnixTime();
    const Json::Value &jTitle = CGameInfo::GetInstance()->m_oJsonRoot["game_title"];

    ostringstream oss;
    for (TUINT32 udwIdx = 0; udwIdx < pstTitle->udwNum; udwIdx++)
    {
        if (pstTitle->aucFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }

        oss.str("");
        oss << pstTitle->atbTitle[udwIdx].m_nId;
        TINT32 dwExpireTime = 0;
        if (jTitle.isMember(oss.str()))
        {
            dwExpireTime = jTitle[oss.str()]["time"].asInt();
        }

        if (ptbThrone->m_nOccupy_time >= pstTitle->atbTitle[udwIdx].m_nDub_time
            || ddwCurTime - pstTitle->atbTitle[udwIdx].m_nDub_time > dwExpireTime)
        {
            continue;
        }

        if (pstTitle->atbTitle[udwIdx].m_nUid == ddwUid)
        {
            return TRUE;
        }
    }

    return FALSE;
}

TBOOL CCommonLogic::HasTitleDub(TINT64 ddwTitleId, TbThrone *ptbThrone, STitleInfoList *pstTitle)
{
    TINT64 ddwCurTime = CTimeUtils::GetUnixTime();

    const Json::Value &jTitle = CGameInfo::GetInstance()->m_oJsonRoot["game_title"];

    ostringstream oss;
    for (TUINT32 udwIdx = 0; udwIdx < pstTitle->udwNum; udwIdx++)
    {
        if (pstTitle->atbTitle[udwIdx].m_nId == ddwTitleId
            && pstTitle->atbTitle[udwIdx].m_nUid > 0
            && pstTitle->aucFlag[udwIdx] != EN_TABLE_UPDT_FLAG__DEL
            && pstTitle->atbTitle[udwIdx].m_nDub_time >= ptbThrone->m_nOccupy_time)
        {
            oss.str("");
            oss << pstTitle->atbTitle[udwIdx].m_nId;
            TINT32 dwExpireTime = 0;
            if (jTitle.isMember(oss.str()))
            {
                dwExpireTime = jTitle[oss.str()]["time"].asInt();
            }
            if (ddwCurTime - pstTitle->atbTitle[udwIdx].m_nDub_time <= dwExpireTime)
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}

TVOID CCommonLogic::AddTitle(TbPlayer *ptbPlayer, TINT64 ddwTitleId, STitleInfoList *pstTitle)
{
    TbTitle *ptbTitle = NULL;
    for (TUINT32 udwIdx = 0; udwIdx < pstTitle->udwNum; udwIdx++)
    {
        if (pstTitle->atbTitle[udwIdx].m_nId == ddwTitleId)
        {
            ptbTitle = &pstTitle->atbTitle[udwIdx];
            pstTitle->aucFlag[udwIdx] = EN_TABLE_UPDT_FLAG__NEW;
            break;
        }
    }

    if (ptbTitle == NULL)
    {
        ptbTitle = &pstTitle->atbTitle[pstTitle->udwNum];
        pstTitle->aucFlag[pstTitle->udwNum] = EN_TABLE_UPDT_FLAG__NEW;
        pstTitle->udwNum++;
        ptbTitle->Set_Sid(ptbPlayer->m_nSid);
        ptbTitle->Set_Id(ddwTitleId);
    }

    ptbTitle->Set_Uid(ptbPlayer->m_nUid);
    ptbTitle->Set_Alid(ptbPlayer->m_nAlpos ? ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET : 0);
    ptbTitle->Set_Name(ptbPlayer->m_sUin);
    ptbTitle->Set_Alnick(ptbPlayer->m_sAl_nick_name);
    ptbTitle->Set_Dub_time(CTimeUtils::GetUnixTime());
    ptbTitle->Set_Cid(ptbPlayer->m_nCid);
}

TVOID CCommonLogic::RemoveTitle(TINT64 ddwUid, TINT64 ddwTitleId, STitleInfoList *pstTitle)
{
    for (TUINT32 udwIdx = 0; udwIdx < pstTitle->udwNum; udwIdx++)
    {
        if (pstTitle->atbTitle[udwIdx].m_nId == ddwTitleId
            && pstTitle->atbTitle[udwIdx].m_nUid == ddwUid
            && pstTitle->aucFlag[udwIdx] != EN_TABLE_UPDT_FLAG__DEL)
        {
            pstTitle->aucFlag[udwIdx] = EN_TABLE_UPDT_FLAG__DEL;
            break;
        }
    }
}

TVOID CCommonLogic::RemoveTitle(TINT64 ddwUid, STitleInfoList *pstTitle)
{
    for (TUINT32 udwIdx = 0; udwIdx < pstTitle->udwNum; udwIdx++)
    {
        if (pstTitle->atbTitle[udwIdx].m_nUid == ddwUid
            && pstTitle->aucFlag[udwIdx] != EN_TABLE_UPDT_FLAG__DEL)
        {
            pstTitle->atbTitle[udwIdx].Set_Dub_time(0);
            pstTitle->aucFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
        }
    }
}

TVOID CCommonLogic::UpdateTitle(TbPlayer *ptbPlayer, STitleInfoList *pstTitle)
{
    TbTitle *ptbTitle = NULL;
    TINT64 ddwCurTime = CTimeUtils::GetUnixTime();

    const Json::Value &jTitle = CGameInfo::GetInstance()->m_oJsonRoot["game_title"];

    ostringstream oss;
    for (TUINT32 udwIdx = 0; udwIdx < pstTitle->udwNum; udwIdx++)
    {
        if (pstTitle->aucFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }

        oss.str("");
        oss << pstTitle->atbTitle[udwIdx].m_nId;
        TINT32 dwExpireTime = 0;
        if (jTitle.isMember(oss.str()))
        {
            dwExpireTime = jTitle[oss.str()]["time"].asInt();
        }
        if (pstTitle->atbTitle[udwIdx].m_nDub_time + dwExpireTime < ddwCurTime)
        {
            continue;
        }

        if (pstTitle->atbTitle[udwIdx].m_nUid == ptbPlayer->m_nUid)
        {
            ptbTitle = &pstTitle->atbTitle[udwIdx];
            TINT64 ddwAlid = ptbPlayer->m_nAlpos ? ptbPlayer->m_nAlid / PLAYER_ALLIANCE_ID_OFFSET : 0;
            if (ptbTitle->m_nAlid != ddwAlid || ptbTitle->m_sName != ptbPlayer->m_sUin 
                || ptbTitle->m_sAlnick != ptbPlayer->m_sAl_nick_name || ptbTitle->m_nCid != ptbPlayer->m_nCid)
            {
                ptbTitle->Set_Alid(ddwAlid);
                ptbTitle->Set_Name(ptbPlayer->m_sUin);
                ptbTitle->Set_Alnick(ptbPlayer->m_sAl_nick_name);
                ptbTitle->Set_Cid(ptbPlayer->m_nCid);
                if (pstTitle->aucFlag[udwIdx] != EN_TABLE_UPDT_FLAG__NEW)
                {
                    pstTitle->aucFlag[udwIdx] = EN_TABLE_UPDT_FLAG__CHANGE;
                }
            }
        }
    }
}

TVOID CCommonLogic::UpdateThroneInfo(SUserInfo *pstUser, TbThrone *ptbThrone)
{
    if (ptbThrone->m_nAlid != 0 && pstUser->m_tbPlayer.m_nAlpos != 0
        && ptbThrone->m_nAlid == pstUser->m_tbPlayer.m_nAlid / PLAYER_ALLIANCE_ID_OFFSET)
    {
        if (pstUser->m_tbPlayer.m_nAlpos == EN_ALLIANCE_POS__CHANCELLOR)
        {
            if (ptbThrone->m_nOwner_id != pstUser->m_tbPlayer.m_nUid)
            {
                ptbThrone->Set_Owner_id(pstUser->m_tbPlayer.m_nUid);
            }
            if (ptbThrone->m_nOwner_cid != pstUser->m_tbPlayer.m_nCid)
            {
                ptbThrone->Set_Owner_cid(pstUser->m_tbPlayer.m_nCid);
            }
        }
        TINT64 ddwDefenceNum = 0;
        TINT64 ddwDefenceTroopNum = 0;
        TINT64 ddwDefenceTroopForce = 0;
        SCommonTroop stDefenceTroop;
        stDefenceTroop.Reset();
        TbMarch_action *ptbMarch = NULL;
        for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwMarchNum; udwIdx++)
        {
            if (pstUser->m_aucMarchFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
            {
                continue;
            }
            ptbMarch = &pstUser->m_atbMarch[udwIdx];
            if (ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__REINFORCE_THRONE
                && ptbMarch->m_nTpos == ptbThrone->m_nPos
                && ptbMarch->m_nStatus == EN_MARCH_STATUS__DEFENDING)
            {
                ddwDefenceNum++;
                for (TUINT32 udwIdy = 0; udwIdy < EN_TROOP_TYPE__END; udwIdy++)
                {
                    stDefenceTroop[udwIdy] += ptbMarch->m_bParam[0].m_stTroop[udwIdy];
                }
            }
            else if (ptbMarch->m_nSclass == EN_ACTION_SEC_CLASS__ASSIGN_THRONE
                && ptbMarch->m_nTpos == ptbThrone->m_nPos
                && ptbMarch->m_nStatus == EN_MARCH_STATUS__DEFENDING)
            {
                ddwDefenceNum++;
                for (TUINT32 udwIdy = 0; udwIdy < EN_TROOP_TYPE__END; udwIdy++)
                {
                    stDefenceTroop[udwIdy] += ptbMarch->m_bParam[0].m_stTroop[udwIdy];
                }
            }
        }
        ddwDefenceTroopNum = CToolBase::GetTroopSumNum(stDefenceTroop);
        ddwDefenceTroopForce = CToolBase::GetTroopSumForce(stDefenceTroop);

        if (ptbThrone->m_nDefending_num != ddwDefenceNum)
        {
            ptbThrone->Set_Defending_num(ddwDefenceNum);
        }
        if (ptbThrone->m_nDefending_troop_num != ddwDefenceTroopNum)
        {
            ptbThrone->Set_Defending_troop_num(ddwDefenceTroopNum);
        }
        if (ptbThrone->m_nDefending_troop_force != ddwDefenceTroopForce)
        {
            ptbThrone->Set_Defending_troop_force(ddwDefenceTroopForce);
        }
    }
}