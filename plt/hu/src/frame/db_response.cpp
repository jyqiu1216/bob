#include "db_response.h"

int CDbResponse::OnSelectResponse(const DbRspInfo& rspInfo, ComputeTime* pComputeTime, unsigned int udwMaxNum)
{
    unsigned int udwNum = 0;
    unsigned int udwLen = 0;
    unsigned short uwFldNo = 0;
    unsigned int udwItemLen = 0;
    char szItemContent[512];

    for (unsigned int i = 0, j = 0; i < rspInfo.sRspContent.size();)
    {
        ComputeTime& oComputeTime = pComputeTime[udwNum];
        if (i+4 > rspInfo.sRspContent.size())
        {
            return -1;
        }
        memcpy(&udwLen, rspInfo.sRspContent.c_str()+i, 4);
        i += 4;
        if (i+udwLen > rspInfo.sRspContent.size())
        {
            return -2;
        }
        j = i;
        for (; j < i + udwLen;)
        {
            if (j+6 > i+udwLen)
            {
                return -3;
            }
            memcpy(&uwFldNo, rspInfo.sRspContent.c_str()+j, 2);
            j += 2;
            memcpy(&udwItemLen, rspInfo.sRspContent.c_str()+j, 4);
            j += 4;
            if (j+udwItemLen > i+udwLen)
            {
                return -4;
            }
            if (udwItemLen < sizeof(szItemContent))
            {
                memcpy(szItemContent, rspInfo.sRspContent.c_str()+j, udwItemLen);
                szItemContent[udwItemLen] = '\0';
            }
            else
            {
                memcpy(szItemContent, rspInfo.sRspContent.c_str()+j, sizeof(szItemContent)-1);
                szItemContent[sizeof(szItemContent)-1] = '\0';
            }
            j += udwItemLen;
            switch (uwFldNo)
            {
            case EN_COMPUTE_TIME_FLD_TYPE:
                oComputeTime.udwType = atoi(szItemContent);
                break;
            case EN_COMPUTE_TIME_FLD_TIME:
                oComputeTime.ddwTime =  strtoll(szItemContent, NULL, 10);
                break;
            default:
                return -5;
            }
        }
        i += udwLen;
        assert(i == j);
        udwNum++;
        if (udwNum >= udwMaxNum)
        {
            return udwNum;
        }
    }
    return udwNum;
}

int CDbResponse::OnSelectResponse(const DbRspInfo& rspInfo, PlayerRecommend* pRecommend, unsigned int udwMaxNum)
{
    unsigned int udwNum = 0;
    unsigned int udwLen = 0;
    unsigned short uwFldNo = 0;
    unsigned int udwItemLen = 0;
    char szItemContent[512];

    for (unsigned int i = 0, j = 0; i < rspInfo.sRspContent.size();)
    {
        PlayerRecommend& oOnePlayer = pRecommend[udwNum];
        if (i+4 > rspInfo.sRspContent.size())
        {
            return -1;
        }
        memcpy(&udwLen, rspInfo.sRspContent.c_str()+i, 4);
        i += 4;
        if (i+udwLen > rspInfo.sRspContent.size())
        {
            return -2;
        }
        j = i;
        for (; j < i + udwLen;)
        {
            if (j+6 > i+udwLen)
            {
                return -3;
            }
            memcpy(&uwFldNo, rspInfo.sRspContent.c_str()+j, 2);
            j += 2;
            memcpy(&udwItemLen, rspInfo.sRspContent.c_str()+j, 4);
            j += 4;
            if (j+udwItemLen > i+udwLen)
            {
                return -4;
            }
            if (udwItemLen < sizeof(szItemContent))
            {
                memcpy(szItemContent, rspInfo.sRspContent.c_str()+j, udwItemLen);
                szItemContent[udwItemLen] = '\0';
            }
            else
            {
                memcpy(szItemContent, rspInfo.sRspContent.c_str()+j, sizeof(szItemContent)-1);
                szItemContent[sizeof(szItemContent)-1] = '\0';
            }
            j += udwItemLen;
            switch (uwFldNo)
            {
            case EN_PLAYER_RECOMMEND_FLD_AID:
                oOnePlayer.udwAid = atoi(szItemContent);
                break;
            case EN_PLAYER_RECOMMEND_FLD_SID:
                oOnePlayer.udwSid = atoi(szItemContent);
                break;
            case EN_PLAYER_RECOMMEND_FLD_UID:
                oOnePlayer.udwUid = atoi(szItemContent);
                break;
            case EN_PLAYER_RECOMMEND_FLD_UNAME:
                oOnePlayer.sUname = szItemContent;
                break;
            case EN_PLAYER_RECOMMEND_FLD_AVATAR:
                oOnePlayer.udwAvatar = atoi(szItemContent);
                break;
            case EN_PLAYER_RECOMMEND_FLD_HERO_LEVEL:
                oOnePlayer.udwLordLv = atoi(szItemContent);
                break;
            case EN_PLAYER_RECOMMEND_FLD_CASTLE_LEVEL:
                oOnePlayer.udwCastleLv = atoi(szItemContent);
                break;
            case EN_PLAYER_RECOMMEND_FLD_FORCE:
                oOnePlayer.ddwForce = strtoll(szItemContent, NULL, 10);
                break;
            case EN_PLAYER_RECOMMEND_FLD_INVITED:
                oOnePlayer.udwInvited = atoi(szItemContent);
                break;
            default:
                return -5;
            }
        }
        i += udwLen;
        assert(i == j);
        udwNum++;
        if (udwNum >= udwMaxNum)
        {
            return udwNum;
        }
    }
    return udwNum;
}

int CDbResponse::OnCountResponse(const DbRspInfo& rspInfo)
{
    unsigned int udwNum = 0;
    unsigned int udwLen = 0;
    unsigned short uwFldNo = 0;
    unsigned int udwItemLen = 0;

    if (rspInfo.sRspContent.size() <= 10)
    {
        return -1;
    }
    memcpy(&udwLen, rspInfo.sRspContent.c_str(), 4);
    memcpy(&uwFldNo, rspInfo.sRspContent.c_str()+4, 2);
    if (uwFldNo != 0)
    {
        return -2;
    }
    memcpy(&udwItemLen, rspInfo.sRspContent.c_str()+6, 4);
    if (udwLen != udwItemLen+6)
    {
        return -3;
    }
    udwNum = atoi(rspInfo.sRspContent.c_str()+10);
    return udwNum;
}

int CDbResponse::OnSelectResponse(const DbRspInfo& rspInfo, AllianceRank* pAllianceRank, unsigned int udwMaxNum)
{
    unsigned int udwNum = 0;
    unsigned int udwLen = 0;
    unsigned short uwFldNo = 0;
    unsigned int udwItemLen = 0;
    char szItemContent[512];

    for(unsigned int i = 0, j = 0; i < rspInfo.sRspContent.size();)
    {
        AllianceRank& oAllianceRank = pAllianceRank[udwNum];
        if(i + 4 > rspInfo.sRspContent.size())
        {
            return -1;
        }
        memcpy(&udwLen, rspInfo.sRspContent.c_str() + i, 4);
        i += 4;
        if(i + udwLen > rspInfo.sRspContent.size())
        {
            return -2;
        }
        j = i;
        for(; j < i + udwLen;)
        {
            if(j + 6 > i + udwLen)
            {
                return -3;
            }
            //表中第几项，占两位
            memcpy(&uwFldNo, rspInfo.sRspContent.c_str() + j, 2);
            j += 2;
            //项的长度，占四位
            memcpy(&udwItemLen, rspInfo.sRspContent.c_str() + j, 4);
            j += 4;
            if(j + udwItemLen > i + udwLen)
            {
                return -4;
            }
            if(udwItemLen < sizeof(szItemContent))
            {
                memcpy(szItemContent, rspInfo.sRspContent.c_str() + j, udwItemLen);
                szItemContent[udwItemLen] = '\0';
            }
            else
            {
                memcpy(szItemContent, rspInfo.sRspContent.c_str() + j, sizeof(szItemContent)-1);
                szItemContent[sizeof(szItemContent)-1] = '\0';
            }
            j += udwItemLen;
            switch(uwFldNo)
            {
            case EN_ALLIANCE_RANK_FLD_AID:
                oAllianceRank.udwAid = atoi(szItemContent);
                break;
            case EN_ALLIANCE_RANK_FLD_ALNAME:
                oAllianceRank.sAlName = szItemContent;
                break;
            case EN_ALLIANCE_RANK_FLD_OID:
                oAllianceRank.udwOid = atoi(szItemContent);
                break;
            case EN_ALLIANCE_RANK_FLD_ONAME:
                oAllianceRank.sOname = szItemContent;
                break;
            case EN_ALLIANCE_RANK_FLD_MIGHT:
                oAllianceRank.ddwMight = strtoll(szItemContent, NULL, 10);
                break;
            case EN_ALLIANCE_RANK_FLD_MEMBER:
                oAllianceRank.udwMember = atoi(szItemContent);
                break;
            case EN_ALLIANCE_RANK_FLD_POLICY:
                oAllianceRank.udwPolicy = atoi(szItemContent);
                break;
            case EN_ALLIANCE_RANK_FLD_DESC:
                oAllianceRank.sDesc = szItemContent;
                break;
            case EN_ALLIANCE_RANK_FLD_LANGUAGE:
                oAllianceRank.udwLanguage = atoi(szItemContent);
                break;
            case EN_ALLIANCE_RANK_FLD_RANK0:
            case EN_ALLIANCE_RANK_FLD_RANK1:
            case EN_ALLIANCE_RANK_FLD_RANK2:
            case EN_ALLIANCE_RANK_FLD_RANK3:
            case EN_ALLIANCE_RANK_FLD_RANK4:
            case EN_ALLIANCE_RANK_FLD_RANK5:
            case EN_ALLIANCE_RANK_FLD_RANK6:
            case EN_ALLIANCE_RANK_FLD_RANK7:
            case EN_ALLIANCE_RANK_FLD_RANK8:
            case EN_ALLIANCE_RANK_FLD_RANK9:
            case EN_ALLIANCE_RANK_FLD_RANK10:
            case EN_ALLIANCE_RANK_FLD_RANK11:
            case EN_ALLIANCE_RANK_FLD_RANK12:
            case EN_ALLIANCE_RANK_FLD_RANK13:
                oAllianceRank.audwRank[uwFldNo - EN_ALLIANCE_RANK_FLD_RANK0] = atoi(szItemContent);
                break;
            case EN_ALLIANCE_RANK_FLD_VALUE0:
            case EN_ALLIANCE_RANK_FLD_VALUE1:
            case EN_ALLIANCE_RANK_FLD_VALUE2:
            case EN_ALLIANCE_RANK_FLD_VALUE3:
            case EN_ALLIANCE_RANK_FLD_VALUE4:
            case EN_ALLIANCE_RANK_FLD_VALUE5:
            case EN_ALLIANCE_RANK_FLD_VALUE6:
            case EN_ALLIANCE_RANK_FLD_VALUE7:
            case EN_ALLIANCE_RANK_FLD_VALUE8:
            case EN_ALLIANCE_RANK_FLD_VALUE9:
            case EN_ALLIANCE_RANK_FLD_VALUE10:
            case EN_ALLIANCE_RANK_FLD_VALUE11:
            case EN_ALLIANCE_RANK_FLD_VALUE12:
            case EN_ALLIANCE_RANK_FLD_VALUE13:
                oAllianceRank.addwValue[uwFldNo - EN_ALLIANCE_RANK_FLD_VALUE0] = strtoll(szItemContent, NULL, 10);
                break;
            case EN_ALLIANCE_RANK_FLD_RANK14:
                oAllianceRank.audwRank[13] = atoi(szItemContent);
                break;
            case EN_ALLIANCE_RANK_FLD_VALUE14:
                oAllianceRank.addwValue[13] = strtoll(szItemContent, NULL, 10);
                break;
            case EN_ALLIANCE_RANK_FLD_ALNICK:
                oAllianceRank.sAlNick = szItemContent;
                break;
            case EN_ALLIANCE_RANK_FLD_NEWPLAYERALFLAG:
                oAllianceRank.ddwNewPlayerAlFlag = strtoll(szItemContent, NULL, 10);
                break;
            default:
                return -5;
            }
        }
        i += udwLen;
        assert(i == j);
        udwNum++;
        if(udwNum >= udwMaxNum)
        {
            return udwNum;
        }

    }
    return udwNum;
}

int CDbResponse::OnSelectResponse(const DbRspInfo& rspInfo, TradeCityInfo* pTradeCityInfo, unsigned int udwMaxNum)
{
    unsigned int udwNum = 0;
    unsigned int udwLen = 0;
    unsigned short uwFldNo = 0;
    unsigned int udwItemLen = 0;
    char szItemContent[512];

    for (unsigned int i = 0, j = 0; i < rspInfo.sRspContent.size();)
    {
        TradeCityInfo& oTradeCityInfo = pTradeCityInfo[udwNum];
        if (i + 4 > rspInfo.sRspContent.size())
        {
            return -1;
        }
        memcpy(&udwLen, rspInfo.sRspContent.c_str() + i, 4);
        i += 4;
        if (i + udwLen > rspInfo.sRspContent.size())
        {
            return -2;
        }
        j = i;
        for (; j < i + udwLen;)
        {
            if (j + 6 > i + udwLen)
            {
                return -3;
            }
            memcpy(&uwFldNo, rspInfo.sRspContent.c_str() + j, 2);
            j += 2;
            memcpy(&udwItemLen, rspInfo.sRspContent.c_str() + j, 4);
            j += 4;
            if (j + udwItemLen > i + udwLen)
            {
                return -4;
            }
            if (udwItemLen < sizeof(szItemContent))
            {
                memcpy(szItemContent, rspInfo.sRspContent.c_str() + j, udwItemLen);
                szItemContent[udwItemLen] = '\0';
            }
            else
            {
                memcpy(szItemContent, rspInfo.sRspContent.c_str() + j, sizeof(szItemContent)-1);
                szItemContent[sizeof(szItemContent)-1] = '\0';
            }
            j += udwItemLen;
            switch (uwFldNo)
            {
            case EN_TRADE_CITY_INFO_FLD_SID:
                oTradeCityInfo.dwSid = atoi(szItemContent);
                break;
            case EN_TRADE_CITY_INFO_FLD_UID:
                oTradeCityInfo.dwUid = atoi(szItemContent);
                break;
            case EN_TRADE_CITY_INFO_FLD_X:
                oTradeCityInfo.dwX = atoi(szItemContent);
                break;
            case EN_TRADE_CITY_INFO_FLD_Y:
                oTradeCityInfo.dwY = atoi(szItemContent);
                break;
            case EN_TRADE_CITY_INFO_FLD_CITYNAME:
                oTradeCityInfo.sCityName = szItemContent;
                break;
            case EN_TRADE_CITY_INFO_FLD_UTIME:
                oTradeCityInfo.udwUTime = atoi(szItemContent);
                break;
            default:
                return -5;
            }
        }
        i += udwLen;
        assert(i == j);
        udwNum++;
        if (udwNum >= udwMaxNum)
        {
            return udwNum;
        }
    }
    return udwNum;
}