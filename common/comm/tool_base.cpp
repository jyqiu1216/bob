#include "tool_base.h"
#include "common_func.h"

double CToolBase::GetDistance(TINT32 dwPosA, TINT32 dwPosB)
{
    TINT32  dwXa = dwPosA / MAP_X_Y_POS_COMPUTE_OFFSET;
    TINT32  dwYa = dwPosA % MAP_X_Y_POS_COMPUTE_OFFSET;
    TINT32  dwXb = dwPosB / MAP_X_Y_POS_COMPUTE_OFFSET;
    TINT32  dwYb = dwPosB % MAP_X_Y_POS_COMPUTE_OFFSET;
    return sqrt((dwXa - dwXb) * (dwXa - dwXb) + (dwYa - dwYb) * (dwYa - dwYb));
}

TUINT32 CToolBase::GetRandNumber(TUINT32 udwBegin, TUINT32 udwEnd)
{
    TUINT32 udwNum = udwEnd - udwBegin + 1;
    return rand() % udwNum + udwBegin;
}

TUINT64 CToolBase::GetPlayerNewTaskId(TINT64 ddwPlayId, TUINT32 udwSeq)
{
    TUINT64 uddwTaskId = 0;

    uddwTaskId = (ddwPlayId << 32) + udwSeq;

    return uddwTaskId;
}

TUINT64 CToolBase::GetAllianceNewTaskId(TUINT32 udwAlid)
{
    return ((udwAlid + (1ul << 27)) << 32) + CTimeUtils::GetUnixTime();
}

string CToolBase::ToLower(const string& str)
{
    string strTarget = str;
    transform(strTarget.begin(), strTarget.end(), strTarget.begin(), (int(*)(int)) tolower);
    return strTarget;
}

CURLcode CToolBase::ResFromUrl(TCHAR *szUrl, TCHAR *szRes, TUINT32 udwTimeOut)
{
    CURL *curl = NULL;
    CURLcode res = CURL_LAST;
    TINT32 dwRetry = 0;
    while (!curl && dwRetry < 5)
    {
        curl = curl_easy_init();
        ++dwRetry;
    }

    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, szUrl);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, szRes);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writedata);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, udwTimeOut);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }

    return res;
}

TUINT32 CToolBase::GetTroopCategoryByTroopType(TUINT32 udwTroopType)
{
    CGameInfo *pGameInfo = CGameInfo::GetInstance();
    return pGameInfo->m_oJsonRoot["game_troop"][udwTroopType]["a"]["a5"].asUInt();
}

TUINT32 CToolBase::GetTroopCategoryByFortType(TUINT32 udwFortType)
{
    CGameInfo *pGameInfo = CGameInfo::GetInstance();
    return pGameInfo->m_oJsonRoot["game_fort"][udwFortType]["a"]["a5"].asUInt();
}

TUINT32 CToolBase::GetTroopLvByTroopId(TUINT32 udwTroopType)
{
    CGameInfo *pGameInfo = CGameInfo::GetInstance();
    return pGameInfo->m_oJsonRoot["game_troop"][udwTroopType]["a"]["a6"].asUInt();
}

TUINT32 CToolBase::GetFortLvByFortId(TUINT32 udwFortType)
{
    CGameInfo *pGameInfo = CGameInfo::GetInstance();
    return pGameInfo->m_oJsonRoot["game_fort"][udwFortType]["a"]["a6"].asUInt();
}



TUINT32 CToolBase::GetTroopSingleMight(TUINT32 udwTroopId)
{
    return CGameInfo::GetInstance()->m_oJsonRoot["game_troop"][udwTroopId]["a"]["a9"].asInt();
}

TUINT32 CToolBase::GetFortSingleMight(TUINT32 udwFortId)
{
    return CGameInfo::GetInstance()->m_oJsonRoot["game_fort"][udwFortId]["a"]["a9"].asInt();
}

TINT32 CToolBase::GetTroopSpeed(TUINT32 udwTroopId)
{
    return CGameInfo::GetInstance()->m_oJsonRoot["game_troop"][udwTroopId]["a"]["a3"].asInt();
}

TINT64 CToolBase::GetTroopSumNum(TINT64 *addwTroop)
{
    if (!addwTroop)
    {
        return 0;
    }
    TINT64 ddwSum = 0;
    for (TUINT32 udwIdx = 0; udwIdx < EN_TROOP_TYPE__END; ++udwIdx)
    {
        ddwSum += addwTroop[udwIdx];
    }
    return ddwSum;
}

TINT64 CToolBase::GetTroopSumNum(const SCommonTroop& stTroop)
{
    TINT64 ddwSum = 0;
    for(TUINT32 udwIdx = 0; udwIdx < EN_TROOP_TYPE__END; ++udwIdx)
    {
        ddwSum += stTroop.m_addwNum[udwIdx];
    }
    return ddwSum;
}

TINT64 CToolBase::GetFortSumNum(const SCommonFort& stFort)
{
    TINT64 ddwSum = 0;
    for(TUINT32 udwIdx = 0; udwIdx < EN_FORT_TYPE__END; ++udwIdx)
    {
        ddwSum += stFort.m_addwNum[udwIdx];
    }
    return ddwSum;
}

TUINT64 CToolBase::GetTroopSumForce(TINT64 *addwTroop)
{
    TUINT64 uddwMightSum = 0;
    for(TUINT32 udwIdx = 0; udwIdx < EN_TROOP_TYPE__END; ++udwIdx)
    {
        if(addwTroop[udwIdx] > 0)
        {
            TUINT64 uddwUnitMight = GetTroopSingleMight(udwIdx);
            uddwMightSum += uddwUnitMight * addwTroop[udwIdx];
        }
    }
    return uddwMightSum;
}

TUINT64 CToolBase::GetTroopSumForce(const SCommonTroop& stTroop)
{
    TUINT64 uddwMightSum = 0;
    for(TUINT32 udwIdx = 0; udwIdx < EN_TROOP_TYPE__END; ++udwIdx)
    {
        if(stTroop.m_addwNum[udwIdx] > 0)
        {
            TUINT64 uddwUnitMight = GetTroopSingleMight(udwIdx);
            uddwMightSum += uddwUnitMight * stTroop.m_addwNum[udwIdx];
        }
    }
    return uddwMightSum;
}

TINT32 CToolBase::GetTroopMarchSpeed(const SCommonTroop& stTroop)
{
    TINT32 dwSlowestSpeed = 2000;
    for(TUINT32 udwIdx = 0; udwIdx < EN_TROOP_TYPE__END; ++udwIdx)
    {
        if(stTroop.m_addwNum[udwIdx] > 0)
        {
            TINT32 dwSpeed = CToolBase::GetTroopSpeed(udwIdx);
            if(dwSpeed < dwSlowestSpeed)
            {
                dwSlowestSpeed = dwSpeed;
            }
        }
    }
    return dwSlowestSpeed;
}

TINT64 CToolBase::GetMarchTime(TbMarch_action* ptbMarch, TINT64 ddwTpos)
{
    double fSpeed = CToolBase::GetDistance(ptbMarch->m_nScid, ptbMarch->m_nTpos) / ptbMarch->m_bParam[0].m_ddwMarchingTime;
    double fDistance = CToolBase::GetDistance(ptbMarch->m_nScid, ddwTpos);
    double fTime = fDistance / fSpeed;
    if(fTime < 30.0f)
    {
        fTime = 30.0f;
    }
    return fTime;
}

TVOID CToolBase::AddTroop(const SCommonTroop& stAdd, SCommonTroop& stTotal)
{
    for(TUINT32 udwIdx = 0; udwIdx < EN_TROOP_TYPE__END; ++udwIdx)
    {
        stTotal.m_addwNum[udwIdx] += stAdd.m_addwNum[udwIdx];
    }
}

TUINT64 CToolBase::GetFortSumMight(TINT64 *addwFort)
{
    TUINT64 uddwMightSum = 0;
    for (TUINT32 udwIdx = 0; udwIdx < EN_FORT_TYPE__END; ++udwIdx)
    {
        if (addwFort[udwIdx] > 0)
        {
            TUINT64 udwUnitMight = GetFortSingleMight(udwIdx);
            uddwMightSum += udwUnitMight * addwFort[udwIdx];
        }
    }
    return uddwMightSum;
}

TINT32 CToolBase::Get_AlHelpTime(TUINT32 udwCTime, TUINT32 udwCanHelpNum)
{
    TINT32 dwHelpTime = udwCTime * 0.01;
    if (dwHelpTime < 60)
    {
        dwHelpTime = 60;
    }
    return -1 * dwHelpTime;
}

TINT32 CToolBase::AddUserToMailReceiverList(TINT32 *adwReceiverList, TUINT32 &udwRecieverNum, TINT64 ddwUserId)
{
    if(ddwUserId == 0)
    {
        return 0;
    }
    if (udwRecieverNum < MAX_REINFORCE_NUM * 2)
    {
        TUINT32 udwIdx = 0;
        for (udwIdx = 0; udwIdx < udwRecieverNum; udwIdx++)
        {
            if(adwReceiverList[udwIdx] == ddwUserId)
            {
                break;
            }
        }
        if (udwIdx >= udwRecieverNum)
        {
            adwReceiverList[udwRecieverNum] = ddwUserId;
            udwRecieverNum++;
        }
    }
    return 0;
}

TINT32 CToolBase::GetAllianceUserReportKey(TINT32 dwAllianceId, TBOOL IsAllMember)
{
    if (IsAllMember)
    {
        return -1 * (dwAllianceId + 10000000);
    }
    else
    {
        return -1 * dwAllianceId;
    }
}

TBOOL CToolBase::IsOpCommand(TCHAR* pszCommand)
{
    if(pszCommand == NULL)
    {
        return FALSE;
    }

    if(strncmp(pszCommand, "operate_", strlen("operate_")) == 0)
    {
        return TRUE;
    }
    if(strncmp(pszCommand, "op_", strlen("op_")) == 0)
    {
        return TRUE;
    }
    if(strncmp(pszCommand, "mail_", strlen("mail_")) == 0
        && strcmp(pszCommand, "mail_reward_collect") != 0)
    {
        return TRUE;
    }

    return FALSE;
}

TINT32 CToolBase::GetBuildingSize(TINT32 dwType)
{
    string strBuildingId = CCommonFunc::NumToString(dwType);
    if(CGameInfo::GetInstance()->m_oJsonRoot["game_building"].isMember(strBuildingId))
    {
        return CGameInfo::GetInstance()->m_oJsonRoot["game_building"][strBuildingId]["a"]["a11"].asInt();
    }
    return 1;
}

TBOOL CToolBase::IsObstacle(TINT32 dwType)
{
    TBOOL bIsObstacle = FALSE;
    if(CGameInfo::GetInstance()->m_oJsonRoot["game_building"][CCommonFunc::NumToString(dwType)]["a"]["a12"].asUInt() == 1)
    {
        bIsObstacle = TRUE;
    }
    return bIsObstacle;
}

TBOOL CToolBase::LoadNewJson(Json::Value& rawJson, Json::Value& resultJson)
{
    resultJson = Json::Value(Json::objectValue);
    TUINT32 udwTableCount = rawJson.size();
    for(TUINT32 udwIdx = 0; udwIdx < udwTableCount; ++udwIdx)
    {
        string strKey = rawJson[udwIdx]["key"].asString();
        TUINT32 udwEndPos = strKey.find(":");
        string strTableName = strKey.substr(1, udwEndPos - 1);
        resultJson[strTableName] = rawJson[udwIdx]["data"];
    }

    return TRUE;
}

// =================================================== private ===================================================== //

size_t CToolBase::writedata(void *buffer, size_t size, size_t nmemb, void *userp)
{
    if (size * nmemb + strlen((char *)userp) >= MAX_JSON_LEN)
    {
        return 0;
    }
    memcpy((char *)userp + strlen((char *)userp), (char *)buffer, size * nmemb);
    *((char *)userp + strlen((char *)userp) + size * nmemb) = '\0';
    return size * nmemb;
}

TBOOL CToolBase::IsScoutShowBuff(TINT32 dwBufferId)
{
    TBOOL bIsBattleBuffer = FALSE;
    if(CGameInfo::GetInstance()->m_oJsonRoot["game_buff_func_info"][CCommonFunc::NumToString(dwBufferId)]["a4"].asUInt() == 0
    || CGameInfo::GetInstance()->m_oJsonRoot["game_buff_func_info"][CCommonFunc::NumToString(dwBufferId)]["a4"].asUInt() == 4)
    {
        bIsBattleBuffer = TRUE;
    }
    return bIsBattleBuffer;
}

TBOOL CToolBase::IsWatchTowerShowBuff(TINT32 dwBufferId)
{
    TBOOL bIsBattleBuffer = FALSE;
    if(CGameInfo::GetInstance()->m_oJsonRoot["game_buff_func_info"][CCommonFunc::NumToString(dwBufferId)]["a4"].asUInt() == 0)
    {
        bIsBattleBuffer = TRUE;
    }
    return bIsBattleBuffer;
}

TUINT32 CToolBase::GetArmyClsByTroopId(TUINT32 udwTroopId)
{
    TUINT32 udwCls = EN_ARMS_CLS__SUPPLY_TROOPS + udwTroopId;
    assert(udwCls < EN_TROOP_CLS__END);

    return udwCls;
}

TUINT32 CToolBase::GetArmyClsByFortId(TUINT32 udwFortId)
{
    TUINT32 udwCls = EN_ARMS_CLS__TRAPS + udwFortId;
    assert(udwCls < EN_FORT_CLS__END);

    return udwCls;
}

TUINT32 CToolBase::GetTroopIdByArmyCls(TUINT32 udwArmyCls)
{
    return udwArmyCls - EN_ARMS_CLS__SUPPLY_TROOPS;
}

TUINT32 CToolBase::GetFortIdByArmyCls(TUINT32 udwArmyCls)
{
    assert(udwArmyCls >= EN_ARMS_CLS__TRAPS);
    return udwArmyCls - EN_ARMS_CLS__TRAPS;
}

TBOOL CToolBase::IsArmyClsTroop(TUINT32 udwArmyCls)
{
    return udwArmyCls < EN_TROOP_CLS__END;
}

TBOOL CToolBase::IsArmyClsFort(TUINT32 udwArmyCls)
{
    return ((udwArmyCls >= EN_ARMS_CLS__TRAPS) && (udwArmyCls < EN_FORT_CLS__END));
}

TBOOL CToolBase::IsValidName(const string& strName, const TINT32 dwType)
{
    TBOOL bIsValid = TRUE;;
    string strTarget = CToolBase::ToLower(strName);
    if(strTarget.find("blazeofbattle") != string::npos)
    {
        return FALSE;
    }
    if(strTarget.find("system") == 0)
    {
        return FALSE;
    }
    if (strTarget.find("support") == 0)
    {
        return FALSE;
    }
    switch(dwType)
    {
    case EN_ALLIANCE_NAME:
        break;
    case EN_ALLIANCE_NICK_NAME:
        break;
    case EN_PLAYER_NAME:
        if(strTarget.find("lord_") == 0)
        {
            bIsValid = FALSE;
        }
        break;
    case EN_DRAGON_NAME:
        break;
    case EN_CITY_NAME:
        break;
    default:
        break;
    }
    return bIsValid;
}

TUINT64 CToolBase::GetClientBuildTaskId(TINT64 ddwUid, TUINT32 udwClientSeq)
{
    TUINT64 uddwTaskId = 0;

    uddwTaskId = (ddwUid << 32) + CLIENT_ACTION_ID_OFFSET + udwClientSeq;

    return uddwTaskId;
}