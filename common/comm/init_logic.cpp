#include "init_logic.h"
#include "game_info.h"
#include "globalres_logic.h"
#include "city_base.h"

TVOID CInitLogic::InitCityInfo(SCityInfo *pstCity, SUserInfo *pstUser,TBOOL bFake /* = FALSE*/)
{
    CGameInfo *pstGameInfo = CGameInfo::GetInstance();
    Json::Value oInfoJson ;
    if(bFake)
    {
        oInfoJson = pstGameInfo->m_oJsonRoot["game_init_data"]["fake"]["city"];
    }
    else
    {
        oInfoJson = pstGameInfo->m_oJsonRoot["game_init_data"]["r"]["city"];
    }
    
    TUINT32 udwPos = 0, udwId = 0, udwLv = 0;
    for(TUINT32 udwIdx = 0; udwIdx < oInfoJson["a1"].size(); ++udwIdx)
    {
        udwPos = oInfoJson["a1"][udwIdx][0U].asUInt();
        udwId = oInfoJson["a1"][udwIdx][1U].asUInt();
        udwLv = oInfoJson["a1"][udwIdx][2U].asUInt();

        SCityBuildingNode& stBuildingNode = pstCity->m_stTblData.m_bBuilding[pstCity->m_stTblData.m_bBuilding.m_udwNum];
        stBuildingNode.m_ddwPos = udwPos;
        stBuildingNode.m_ddwType = udwId;
        stBuildingNode.m_ddwLevel = udwLv;
        pstCity->m_stTblData.m_bBuilding.m_udwNum++;
        pstCity->m_stTblData.SetFlag(TbCITY_FIELD_BUILDING);
    }

    SSpGlobalRes stCityItem;
    stCityItem.Reset();

    for(TUINT32 udwIdx = 0; udwIdx < oInfoJson["a2"].size(); ++udwIdx)
    {
        stCityItem.aRewardList[stCityItem.udwTotalNum].udwType = oInfoJson["a2"][udwIdx][0U].asUInt();
        stCityItem.aRewardList[stCityItem.udwTotalNum].udwId = oInfoJson["a2"][udwIdx][1U].asUInt();
        stCityItem.aRewardList[stCityItem.udwTotalNum].udwNum = oInfoJson["a2"][udwIdx][2U].asUInt();
        stCityItem.udwTotalNum++;
    }
    CGlobalResLogic::AddSpGlobalRes(pstUser, pstCity, &stCityItem);

    
}

TVOID CInitLogic::InitPlayerInfo(SCityInfo *pstCity, SUserInfo *pstUser, TBOOL bFake /* = FALSE*/)
{
    CGameInfo *pstGameInfo = CGameInfo::GetInstance();
    Json::Value oInfoJson;
    if(bFake)
    {
        oInfoJson = pstGameInfo->m_oJsonRoot["game_init_data"]["fake"]["player"];
    }
    else
    {
        oInfoJson = pstGameInfo->m_oJsonRoot["game_init_data"]["r"]["player"];
    }

    SSpGlobalRes stCityItem;
    stCityItem.Reset();

    for(TUINT32 udwIdx = 0; udwIdx < oInfoJson["a1"].size(); ++udwIdx)
    {
        stCityItem.aRewardList[stCityItem.udwTotalNum].udwType = oInfoJson["a1"][udwIdx][0U].asUInt();
        stCityItem.aRewardList[stCityItem.udwTotalNum].udwId = oInfoJson["a1"][udwIdx][1U].asUInt();
        stCityItem.aRewardList[stCityItem.udwTotalNum].udwNum = oInfoJson["a1"][udwIdx][2U].asUInt();
        stCityItem.udwTotalNum++;
    }

    CGlobalResLogic::AddSpGlobalRes(pstUser, pstCity, &stCityItem);

    pstUser->m_tbUserStat.m_bTop_quest_finish[0].Reset();
    pstUser->m_tbUserStat.m_bTop_quest[0].Reset();

    for(TUINT32 udwIdx = 0; udwIdx < oInfoJson["a2"].size(); ++udwIdx)
    {
        BITSET(pstUser->m_tbUserStat.m_bTop_quest_finish[0].m_bitQuest, oInfoJson["a2"][udwIdx].asUInt());

    }
    pstUser->m_tbUserStat.SetFlag(TbUSER_STAT_FIELD_TOP_QUEST);
    pstUser->m_tbUserStat.SetFlag(TbUSER_STAT_FIELD_TOP_QUEST_FINISH);
}
