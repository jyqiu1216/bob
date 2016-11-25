#include "backpack_logic.h"
#include "game_info.h"
#include "player_info.h"
#include "cmath"
#include "backpack_info.h"
#include "time_utils.h"
#include <vector>
#include "service_key.h"
#include "common_func.h"
#include "jsoncpp/json/json.h"
#include "player_info.h"
#include "sendmessage_base.h"
#include "activities_logic.h"
#include "common_base.h"
#include <functional>

//id
TUINT64 CBackpack::GenEquipId(TbLogin *ptbLogin)
{
    TINT64 ddwEquipId = ptbLogin->m_nUid;
    ddwEquipId = (ddwEquipId << 32) + ptbLogin->m_nSeq;
    ptbLogin->Set_Seq(ptbLogin->m_nSeq + 1);
    return ddwEquipId;
}

//gen equip
TINT32 CBackpack::ComposeNormalEquip(SUserInfo *pstUser, TUINT32 udwEquipType, TUINT32 udwScrollId, const vector<TUINT32> &vMaterial,
    SEquipMentInfo *pstEquip, TBOOL bInstallFinish, TINT32 dwLv)
{
    pstEquip->Reset();

    TbLogin *pstLogin = &pstUser->m_tbLogin;

    const Json::Value &jEquip = CBackpackInfo::GetInstance()->m_oJsonRoot;

    vector<TUINT32> vecMaterialLv;
    vector<TUINT32> vecMaterialType;
    vecMaterialLv.clear();
    vecMaterialType.clear();

    for(TUINT32 udwIdx = 0; udwIdx < vMaterial.size(); ++udwIdx)
    {
        if (!jEquip["equip_material"].isMember(CCommonFunc::NumToString(vMaterial[udwIdx])))
        {
            TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("CBackpack::ComposeNormalEquip:error material [id=%u] [seq=%u]",
                vMaterial[udwIdx], pstUser->m_udwBSeqNo));
            return -1;
        }
        vecMaterialType.push_back(jEquip["equip_material"][CCommonFunc::NumToString(vMaterial[udwIdx])]["b"][0U].asUInt());
        vecMaterialLv.push_back(jEquip["equip_material"][CCommonFunc::NumToString(vMaterial[udwIdx])]["b"][1U].asUInt());
    }
    
    string szScrollId = CCommonFunc::NumToString(udwScrollId);
    if (!jEquip["equip_scroll"].isMember(szScrollId))
    {
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("ProcessCmd_ComposeEquip: scroll_id error [id:%u] [seq=%u]",
            udwScrollId, pstUser->m_udwBSeqNo));
        return -2;
    }

    string szEquipType = CCommonFunc::NumToString(udwEquipType);
    if (!jEquip["equip_equipment"].isMember(szEquipType))
    {
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("ProcessCmd_ComposeEquip: equip_type error [id:%u] [seq=%u]",
            udwEquipType, pstUser->m_udwBSeqNo));
        return -2;
    }


    //等级
    //TINT32 dwLv = GetNewNormalEquipLv(vecMaterialLv); //废弃，从DC获取等级

    if (dwLv <= 0)
    {
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("ProcessCmd_ComposeEquip: get_equip_lv[%d] error [seq=%u]",
            dwLv, pstUser->m_udwBSeqNo));
        return -2;
    }

    //type
    set<TUINT32> setIdx;
    for (TUINT32 udwIdx = 0; udwIdx < jEquip["equip_equipment"][szEquipType]["c"].size(); udwIdx++)
    {
        TBOOL bIsFind = FALSE;
        if (jEquip["equip_equipment"][szEquipType]["c"][udwIdx][0U].asUInt() == EN_GLOBALRES_TYPE_DRAGON_LV)
        {
            if (pstUser->m_tbPlayer.m_nDragon_level < jEquip["equip_equipment"][szEquipType]["c"][udwIdx][1U].asUInt())
            {
                //该返回码不能修改
                return EN_RET_CODE__COMPOSE_DRAGON_LV_NOT_ENOUGH;
            }
            continue;
        }
        else if (jEquip["equip_equipment"][szEquipType]["c"][udwIdx][0U].asUInt() != EN_GLOBALRES_TYPE_MATERIAL_TYPE)
        {
            continue;
        }

        for (TUINT32 udwIdy = 0; udwIdy < vecMaterialType.size(); udwIdy++)
        {
            if (jEquip["equip_equipment"][szEquipType]["c"][udwIdx][1U].asUInt() == vecMaterialType[udwIdy]
                && setIdx.count(udwIdy) == 0)
            {
                bIsFind = TRUE;
                setIdx.insert(udwIdy);
                break;
            }
        }
        if (bIsFind == FALSE)
        {
            TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("ProcessCmd_ComposeEquip: materail error [seq=%u]",
                pstUser->m_udwBSeqNo));
            return -2;
        }
    }

    //id
    pstEquip->uddwId = GenEquipId(pstLogin);
    pstEquip->stBaseInfo.udwLv = dwLv;
    //base info
    GetEquipBaseInfoByEid(udwEquipType, pstEquip);

    if (pstEquip->stBaseInfo.ddwScrollId != udwScrollId)
    {
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("ProcessCmd_ComposeEquip: scroll id error[%ld:%u] [seq=%u]",
            pstEquip->stBaseInfo.ddwScrollId, udwScrollId, pstUser->m_udwBSeqNo));
        return -2;
    }

    //buff
    const Json::Value &oEquipJson = CBackpackInfo::GetInstance()->m_oJsonRoot["equip_equipment"];
    Json::Value::Members oBuffMember = oEquipJson[szEquipType]["buff"][dwLv - 1].getMemberNames();
    for(Json::Value::Members::iterator it = oBuffMember.begin(); it != oBuffMember.end();++it)
    {
        pstEquip->stStatusInfo.astBuffInfo[pstEquip->stStatusInfo.udwBufferNum].m_udwId = oEquipJson[szEquipType]["buff"][dwLv - 1][(*it).c_str()][0U].asUInt();
        pstEquip->stStatusInfo.astBuffInfo[pstEquip->stStatusInfo.udwBufferNum].m_dwNum = oEquipJson[szEquipType]["buff"][dwLv - 1][(*it).c_str()][1U].asInt();
        pstEquip->stStatusInfo.astBuffInfo[pstEquip->stStatusInfo.udwBufferNum].m_udwType = oEquipJson[szEquipType]["buff"][dwLv - 1][(*it).c_str()][3U].asUInt();
        pstEquip->stStatusInfo.udwBufferNum++;
    }

    if(bInstallFinish)
    {
        //活动积分统计
        CActivitesLogic::ComputeEquipUpgradeScore(pstUser, dwLv);

        pstEquip->stStatusInfo.udwStatus = EN_EQUIPMENT_STATUS_NORMAL;

        //返回给客户端
        SGlobalRes stReward;
        stReward.ddwTotalNum = 1;
        stReward[0].ddwType = EN_GLOBALRES_TYPE_EQUIP;
        stReward[0].ddwId = pstEquip->uddwId;
        stReward[0].ddwNum = 1;

        Json::Value jTmp = Json::Value(Json::objectValue);
        jTmp["equip"] = Json::Value(Json::arrayValue);
        jTmp["equip"][0U] = pstEquip->stBaseInfo.udwLv;

        CCommonBase::AddRewardWindow(pstUser, pstUser->m_tbPlayer.m_nUid, EN_REWARD_WINDOW_TYPE_ONE_EQUIP, EN_REWARD_WINDOW_GET_TYPE_COMPOSE, 0, &stReward, FALSE, jTmp);
    }
    else
    {
        pstEquip->stStatusInfo.udwStatus = EN_EQUIPMENT_STATUS_UPGRADING;
    }
    
    //cost material 
    for(TUINT32 udwIdx = 0; udwIdx < vMaterial.size(); ++udwIdx)
    {
        CostMaterial(pstUser, vMaterial[udwIdx]);
    }
    CostScroll(pstUser, udwScrollId);

    AddEquip(pstUser, pstEquip);

    return 0;
}

TINT32 CBackpack::ComposeSpecialEquip(SUserInfo *pstUser, TINT32 dwResearchParam, TUINT32 udwSoulId,
    vector<TUINT32> *pvParts, SEquipMentInfo *pstEquip, TBOOL bInstallFinish,TBOOL bMistery)
{
    pstEquip->Reset();

    TbLogin *pstLogin = &pstUser->m_tbLogin;
    TINT32 dwRetCode = 0;
    const Json::Value &oComposeJson = CBackpackInfo::GetInstance()->m_oJsonRoot["equip_compose"];

    SSoulInfo stSoulInfo;
    stSoulInfo.Reset();
    dwRetCode = GetSoulInfoById(udwSoulId, &stSoulInfo);
    if(dwRetCode != 0)
    {
        //不存在该灵魂
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("CBackpack::ComposeSpecialEquip:can not find soul [soul_id=%u] uid=%u [seq=%u]",
            udwSoulId, pstLogin->m_nUid, pstUser->m_udwBSeqNo));
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
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("CBackpack::ComposeSpecialEquip:can not find equip [soul_id=%u equip_type=%u equip_lv] uid=%u [seq=%u]",
            udwSoulId, udwEquipType,udwEquipLv, pstLogin->m_nUid, pstUser->m_udwBSeqNo));
        return -2;
    }
    //id
    pstEquip->uddwId = GenEquipId(pstLogin);

    //base info
    GetEquipBaseInfoByEid(udwEid, pstEquip);

    if (pstUser->m_tbPlayer.m_nDragon_level < pstEquip->stBaseInfo.udwOnLv)
    {
        //该返回码不能修改
        return EN_RET_CODE__COMPOSE_DRAGON_LV_NOT_ENOUGH;
    }

    //soul buff
    const Json::Value &oSoulJson = CBackpackInfo::GetInstance()->m_oJsonRoot["equip_soul"];
    Json::Value::Members oBuffMember = oSoulJson[CCommonFunc::NumToString(udwSoulId)]["buff"].getMemberNames();

    for(Json::Value::Members::iterator it = oBuffMember.begin(); it != oBuffMember.end(); ++it)
    {
        TINT32 dwMin = oSoulJson[CCommonFunc::NumToString(udwSoulId)]["buff"][(*it).c_str()][1U].asInt();
        TINT32 dwMax = oSoulJson[CCommonFunc::NumToString(udwSoulId)]["buff"][(*it).c_str()][2U].asInt();
        TUINT32 udwId = oSoulJson[CCommonFunc::NumToString(udwSoulId)]["buff"][(*it).c_str()][0U].asUInt();
        TUINT32 udwType = oSoulJson[CCommonFunc::NumToString(udwSoulId)]["buff"][(*it).c_str()][4U].asUInt();

        pstEquip->stStatusInfo.astBuffInfo[pstEquip->stStatusInfo.udwBufferNum].m_udwId = udwId;
        pstEquip->stStatusInfo.astBuffInfo[pstEquip->stStatusInfo.udwBufferNum].m_dwNum = GetRandomBuffNum(dwResearchParam, dwMin, dwMax);
        pstEquip->stStatusInfo.astBuffInfo[pstEquip->stStatusInfo.udwBufferNum].m_udwType = udwType;
        pstEquip->stStatusInfo.udwBufferNum++;
    }
    if(bMistery)
    {
        const Json::Value &oEqiupJson = CBackpackInfo::GetInstance()->m_oJsonRoot["equip_equipment"];
        Json::Value::Members oBuffMember = oEqiupJson[CCommonFunc::NumToString(udwEid)]["mystery_buff"].getMemberNames();

        for(Json::Value::Members::iterator it = oBuffMember.begin(); it != oBuffMember.end(); ++it)
        {
            TINT32 dwMin = oEqiupJson[CCommonFunc::NumToString(udwEid)]["mystery_buff"][(*it).c_str()][1U].asInt();
            TINT32 dwMax = oEqiupJson[CCommonFunc::NumToString(udwEid)]["mystery_buff"][(*it).c_str()][2U].asInt();
            TUINT32 udwId = oEqiupJson[CCommonFunc::NumToString(udwEid)]["mystery_buff"][(*it).c_str()][0U].asUInt();
            TUINT32 udwType = oEqiupJson[CCommonFunc::NumToString(udwEid)]["mystery_buff"][(*it).c_str()][4U].asUInt();
            TINT32 dwLastBuffNum = GetRandomBuffNum(dwResearchParam, dwMin, dwMax);

            
            pstEquip->stStatusInfo.astMisteryBuffInfo[pstEquip->stStatusInfo.udwMistoryBufferNum].m_udwId = udwId;
            pstEquip->stStatusInfo.astMisteryBuffInfo[pstEquip->stStatusInfo.udwMistoryBufferNum].m_dwNum = dwLastBuffNum;
            pstEquip->stStatusInfo.astMisteryBuffInfo[pstEquip->stStatusInfo.udwMistoryBufferNum].m_udwType = udwType;
            pstEquip->stStatusInfo.udwMistoryBufferNum++;
            
        }
    }
    //parts buff
    for(TUINT32 udwIdx = 0; udwIdx < pvParts->size(); ++udwIdx)
    {
        SSoulInfo stSoulInfo;
        stSoulInfo.Reset();
        dwRetCode = GetPartsInfoById((*pvParts)[udwIdx], &stSoulInfo);
        if(dwRetCode != 0)
        {
            //存在非法parts 
            TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("CBackpack::ComposeSpecialEquip:can not find parts [parts_id=%u] uid=%u [seq=%u]",
                (*pvParts)[udwIdx], pstLogin->m_nUid, pstUser->m_udwBSeqNo));
            return -3;
        }
        for(TUINT32 udwPartsBuffIdx = 0; udwPartsBuffIdx < stSoulInfo.udwBufferNum;++udwPartsBuffIdx)
        {
            TINT32 dwMin = stSoulInfo.astBuffInfo[udwPartsBuffIdx].m_dwMinNum;
            TINT32 dwMax = stSoulInfo.astBuffInfo[udwPartsBuffIdx].m_dwMaxNum;
            TINT32 dwLastBuffNum = GetRandomBuffNum(dwResearchParam, dwMin, dwMax);

            TBOOL  bExist = FALSE;
            for(TUINT32 udwEquipBuffIdx = 0; udwEquipBuffIdx < pstEquip->stStatusInfo.udwBufferNum;++udwEquipBuffIdx)
            {
                if(pstEquip->stStatusInfo.astBuffInfo[udwEquipBuffIdx].m_udwId == stSoulInfo.astBuffInfo[udwPartsBuffIdx].m_udwId)
                {
                    bExist = TRUE;
                    pstEquip->stStatusInfo.astBuffInfo[udwEquipBuffIdx].m_dwNum += dwLastBuffNum;
                    break;
                }
            }
            if(!bExist)
            {
                pstEquip->stStatusInfo.astBuffInfo[pstEquip->stStatusInfo.udwBufferNum].m_udwId = stSoulInfo.astBuffInfo[udwPartsBuffIdx].m_udwId;
                pstEquip->stStatusInfo.astBuffInfo[pstEquip->stStatusInfo.udwBufferNum].m_dwNum = dwLastBuffNum;
                pstEquip->stStatusInfo.astBuffInfo[pstEquip->stStatusInfo.udwBufferNum].m_udwType = stSoulInfo.astBuffInfo[udwPartsBuffIdx].m_udwType;
                pstEquip->stStatusInfo.udwBufferNum++;
            }
        }
    }

    if(bInstallFinish)
    {
        //活动积分统计
        CActivitesLogic::ComputeEquipUpgradeScore(pstUser, udwEquipLv);

        pstEquip->stStatusInfo.udwStatus = EN_EQUIPMENT_STATUS_NORMAL;

        //返回给客户端
//         pstUser->udwRewardWinType = EN_REWARD_WINDOW_TYPE_ONE_EQUIP;
//         pstUser->udwRewardWinGetType = EN_REWARD_WINDOW_GET_TYPE_COMPOSE;
//         pstUser->m_stRewardWindow.aRewardList[pstUser->m_stRewardWindow.udwTotalNum].udwType = EN_GLOBALRES_TYPE_EQUIP;
//         pstUser->m_stRewardWindow.aRewardList[pstUser->m_stRewardWindow.udwTotalNum].udwId = pstEquip->uddwId;
//         pstUser->m_stRewardWindow.aRewardList[pstUser->m_stRewardWindow.udwTotalNum].udwNum = 1;
//         pstUser->m_stRewardWindow.udwTotalNum++;
    }
    else
    {
        pstEquip->stStatusInfo.udwStatus = EN_EQUIPMENT_STATUS_UPGRADING;
    }
    //cost 
    for(TUINT32 udwIdx = 0; udwIdx < pvParts->size(); ++udwIdx)
    {
        CostParts(pstUser, (*pvParts)[udwIdx]);
    }
    CostSoul(pstUser, udwSoulId);

    //add
    AddEquip(pstUser, pstEquip);
    return 0;
}

//info
TINT32 CBackpack::GetEquipInfoById(TbEquip *atbEquip, TUINT32 udwEquipNum,TUINT64 uddwEquipId, SEquipMentInfo *pstEquip)
{
    pstEquip->Reset();
    pstEquip->uddwId = uddwEquipId;

    const Json::Value &oEquipJson = CBackpackInfo::GetInstance()->m_oJsonRoot["equip_equipment"];

    TINT32 dwEquipIdx = -1;
    for(TUINT32 udwIdx = 0; udwIdx < udwEquipNum; ++udwIdx)
    {
        if(atbEquip[udwIdx].m_nId == static_cast<TINT64>(uddwEquipId))
        {
            dwEquipIdx = udwIdx;
            break;
        }
    }
    if(dwEquipIdx == -1)
    {
        //玩家不存在该装备
        return -1;
    }
    TUINT32 udwEType = atbEquip[dwEquipIdx].m_nEquip_type;
    if(!oEquipJson.isMember(CCommonFunc::NumToString(udwEType)))
    {
        //该装备id 未在配置里
        return -2;
    }
    pstEquip->stBaseInfo.udwLv = atbEquip[dwEquipIdx].m_nEquip_lv;
    pstEquip->stStatusInfo.udwStatus = atbEquip[dwEquipIdx].m_nStatus;
    pstEquip->stStatusInfo.udwEquipmentPutOnTime = atbEquip[dwEquipIdx].m_nPut_on_time;
    pstEquip->stStatusInfo.udwPotOnPos = atbEquip[dwEquipIdx].m_nPut_on_pos;
    //crystal
    for(TUINT32 udwIdx = 0; udwIdx < MAX_CRYSTAL_IN_EQUIPMENT; ++udwIdx)
    {
        if(atbEquip[dwEquipIdx].m_bCrystal[0].m_addwNum[udwIdx] == 0)
        {
            pstEquip->stStatusInfo.audwSlot[udwIdx] = 0;
            continue;
        }
        pstEquip->stStatusInfo.audwSlot[udwIdx] = atbEquip[dwEquipIdx].m_bCrystal[0].m_addwNum[udwIdx];
    }
    //base info 
    GetEquipBaseInfoByEid(udwEType, pstEquip);

    //buff
    Json::Value::Members jBuffer = atbEquip[dwEquipIdx].m_jBuff.getMemberNames();
    for(Json::Value::Members::iterator it = jBuffer.begin(); it != jBuffer.end(); ++it)
    {
        if(atbEquip[dwEquipIdx].m_jBuff[(*it).c_str()][0U].asUInt() != 0 &&
            atbEquip[dwEquipIdx].m_jBuff[(*it).c_str()][1U].asInt() != 0)
        {
            pstEquip->stStatusInfo.astBuffInfo[pstEquip->stStatusInfo.udwBufferNum].m_udwId = atbEquip[dwEquipIdx].m_jBuff[(*it).c_str()][0U].asUInt();
            pstEquip->stStatusInfo.astBuffInfo[pstEquip->stStatusInfo.udwBufferNum].m_dwNum = atbEquip[dwEquipIdx].m_jBuff[(*it).c_str()][1U].asInt();
            pstEquip->stStatusInfo.astBuffInfo[pstEquip->stStatusInfo.udwBufferNum].m_udwType = atbEquip[dwEquipIdx].m_jBuff[(*it).c_str()][2U].asUInt();
            pstEquip->stStatusInfo.udwBufferNum++;
        }
    }

    //mistory buff
    Json::Value::Members jMistoryBuffer = atbEquip[dwEquipIdx].m_jMistery_buff.getMemberNames();
    for(Json::Value::Members::iterator it = jMistoryBuffer.begin(); it != jMistoryBuffer.end(); ++it)
    {
        if(atbEquip[dwEquipIdx].m_jMistery_buff[(*it).c_str()][0U].asUInt() != 0 &&
            atbEquip[dwEquipIdx].m_jMistery_buff[(*it).c_str()][1U].asUInt() != 0)
        {
            pstEquip->stStatusInfo.astMisteryBuffInfo[pstEquip->stStatusInfo.udwMistoryBufferNum].m_udwId = atbEquip[dwEquipIdx].m_jMistery_buff[(*it).c_str()][0U].asUInt();
            pstEquip->stStatusInfo.astMisteryBuffInfo[pstEquip->stStatusInfo.udwMistoryBufferNum].m_dwNum = atbEquip[dwEquipIdx].m_jMistery_buff[(*it).c_str()][1U].asInt();
            pstEquip->stStatusInfo.astMisteryBuffInfo[pstEquip->stStatusInfo.udwMistoryBufferNum].m_udwType = atbEquip[dwEquipIdx].m_jMistery_buff[(*it).c_str()][2U].asUInt();
            pstEquip->stStatusInfo.udwMistoryBufferNum++;
        }
    }

    TINT32 dwRetCode = 0;
    for(TUINT32 udwIdx = 0; udwIdx < MAX_CRYSTAL_IN_EQUIPMENT;++udwIdx)
    {
        if(pstEquip->stStatusInfo.audwSlot[udwIdx] == 0)
        {
            continue;
        }
        SCrystalInfo stCrystal;
        stCrystal.Reset();
        dwRetCode = CBackpack::GetCrystalInfoById(pstEquip->stStatusInfo.audwSlot[udwIdx], &stCrystal);
        if(dwRetCode != 0)
        {
            break;
        }
        for(TUINT32 udwCrystalBuffIdx = 0; udwCrystalBuffIdx < stCrystal.udwBufferNum;++udwCrystalBuffIdx)
        {
            TBOOL bBuffExist = FALSE;
            for(TUINT32 udwEquipBuffIdx = 0; udwEquipBuffIdx < pstEquip->stStatusInfo.udwBufferNum; ++udwEquipBuffIdx)
            {
                if(stCrystal.astBuffInfo[udwCrystalBuffIdx].m_udwId == pstEquip->stStatusInfo.astBuffInfo[udwEquipBuffIdx].m_udwId)
                {
                    pstEquip->stStatusInfo.astBuffInfo[udwEquipBuffIdx].m_dwNum += stCrystal.astBuffInfo[udwCrystalBuffIdx].m_dwNum;
                    bBuffExist = TRUE;
                    break;
                }
            }
            if(!bBuffExist)
            {
                pstEquip->stStatusInfo.astBuffInfo[pstEquip->stStatusInfo.udwBufferNum].m_udwId = stCrystal.astBuffInfo[udwCrystalBuffIdx].m_udwId;
                pstEquip->stStatusInfo.astBuffInfo[pstEquip->stStatusInfo.udwBufferNum].m_dwNum = stCrystal.astBuffInfo[udwCrystalBuffIdx].m_dwNum;
                pstEquip->stStatusInfo.astBuffInfo[pstEquip->stStatusInfo.udwBufferNum].m_udwType = stCrystal.astBuffInfo[udwCrystalBuffIdx].m_udwType;
                pstEquip->stStatusInfo.udwBufferNum++;
            }
        }
    }
    if(dwRetCode != 0)
    {
        //存在非法水晶
        return -3;
    }
    return 0;
}

TINT32 CBackpack::GetEquipBaseInfoByEid(TUINT32 udwEquipId, SEquipMentInfo *pstEquip)
{
    const Json::Value &oEquipJson = CBackpackInfo::GetInstance()->m_oJsonRoot["equip_equipment"];
    if(!oEquipJson.isMember(CCommonFunc::NumToString(udwEquipId)))
    {
        //该装备id 未在配置里
        return -1;
    }
    //base info 
    const Json::Value &jOneEquipJson = oEquipJson[CCommonFunc::NumToString(udwEquipId)]["b"];
    pstEquip->stBaseInfo.udwEType = udwEquipId;
    pstEquip->stBaseInfo.udwPos = jOneEquipJson[0U].asUInt();
    pstEquip->stBaseInfo.udwOnLv = jOneEquipJson[1U].asUInt();
    pstEquip->stBaseInfo.ddwScrollId = jOneEquipJson[8U].asUInt();

    return 0;

}

TINT32 CBackpack::GetCrystalInfoById(TUINT32 udwCrystalId, SCrystalInfo *pstCrystal)
{
    pstCrystal->Reset();
    pstCrystal->udwId = udwCrystalId;

    const Json::Value &oCrystalJson = CBackpackInfo::GetInstance()->m_oJsonRoot["equip_crystal"];

    if(!oCrystalJson.isMember(CCommonFunc::NumToString(udwCrystalId)))
    {
        //不存在该crystal
        return -1;
    }
    const Json::Value &oOneCrystal = oCrystalJson[CCommonFunc::NumToString(udwCrystalId)];
    pstCrystal->udwType = oOneCrystal["b"][0U].asUInt();
    pstCrystal->udwLv = oOneCrystal["b"][1U].asUInt();
    
    Json::Value::Members jBuffer = oOneCrystal["buff"].getMemberNames();
    for(Json::Value::Members::iterator it = jBuffer.begin(); it != jBuffer.end(); ++it)
    {
        if(oOneCrystal["buff"][(*it).c_str()][0U].asUInt() != 0 &&
            oOneCrystal["buff"][(*it).c_str()][1U].asInt() != 0)
        {
            pstCrystal->astBuffInfo[pstCrystal->udwBufferNum].m_udwId = oOneCrystal["buff"][(*it).c_str()][0U].asUInt();
            pstCrystal->astBuffInfo[pstCrystal->udwBufferNum].m_dwNum = oOneCrystal["buff"][(*it).c_str()][1U].asInt();
            pstCrystal->astBuffInfo[pstCrystal->udwBufferNum].m_udwType = oOneCrystal["buff"][(*it).c_str()][4U].asUInt();
            pstCrystal->udwBufferNum++;
        }
    }
    return 0;
}

TINT32 CBackpack::GetMaterialInfoById(TUINT32 udwMaterialId, SMaterialInfo *pstCrystal)
{
    pstCrystal->Reset();
    pstCrystal->udwId = udwMaterialId;

    const Json::Value &oMaterialJson = CBackpackInfo::GetInstance()->m_oJsonRoot["equip_material"];

    if(!oMaterialJson.isMember(CCommonFunc::NumToString(udwMaterialId)))
    {
        //不存在该crystal
        return -1;
    }
    const Json::Value &oOneCrystal = oMaterialJson[CCommonFunc::NumToString(udwMaterialId)];
    pstCrystal->udwType = oOneCrystal["b"][0U].asUInt();
    pstCrystal->udwLv = oOneCrystal["b"][1U].asUInt();
    return 0;
}


TINT32 CBackpack::GetSoulInfoById(TUINT32 udwSoulId, SSoulInfo *pstSoul)
{
    pstSoul->Reset();
    pstSoul->udwId = udwSoulId;

    const Json::Value &oSoulJson = CBackpackInfo::GetInstance()->m_oJsonRoot["equip_soul"];

    if(!oSoulJson.isMember(CCommonFunc::NumToString(udwSoulId)))
    {
        //不存在该crystal
        return -1;
    }
    const Json::Value &oOneSoul = oSoulJson[CCommonFunc::NumToString(udwSoulId)];
    pstSoul->udwType = oOneSoul["b"][0U].asUInt();
    pstSoul->udwLv = oOneSoul["b"][1U].asUInt();

    Json::Value::Members jBuffer = oOneSoul["buff"].getMemberNames();
    for(Json::Value::Members::iterator it = jBuffer.begin(); it != jBuffer.end(); ++it)
    {
        if(oOneSoul["buff"][(*it).c_str()][0U].asUInt() != 0 &&
            oOneSoul["buff"][(*it).c_str()][1U].asInt() != 0)
        {
            pstSoul->astBuffInfo[pstSoul->udwBufferNum].m_udwId = oOneSoul["buff"][(*it).c_str()][0U].asUInt();
            pstSoul->astBuffInfo[pstSoul->udwBufferNum].m_dwMinNum = oOneSoul["buff"][(*it).c_str()][1U].asInt();
            pstSoul->astBuffInfo[pstSoul->udwBufferNum].m_dwMaxNum = oOneSoul["buff"][(*it).c_str()][2U].asInt();
            pstSoul->astBuffInfo[pstSoul->udwBufferNum].m_udwType = oOneSoul["buff"][(*it).c_str()][4U].asUInt();
            pstSoul->udwBufferNum++;
        }
    }
    
    return 0;
}

TINT32 CBackpack::GetPartsInfoById(TUINT32 udwpartsId, SSoulInfo *pstParts)
{
    pstParts->Reset();
    pstParts->udwId = udwpartsId;

    const Json::Value &oSoulJson = CBackpackInfo::GetInstance()->m_oJsonRoot["equip_parts"];

    if(!oSoulJson.isMember(CCommonFunc::NumToString(udwpartsId)))
    {
        //不存在该crystal
        return -1;
    }
    const Json::Value &oOneSoul = oSoulJson[CCommonFunc::NumToString(udwpartsId)];
    pstParts->udwType = oOneSoul["b"][0U].asUInt();
    pstParts->udwLv = oOneSoul["b"][1U].asUInt();

    
    Json::Value::Members jBuffer = oOneSoul["buff"].getMemberNames();
    for(Json::Value::Members::iterator it = jBuffer.begin(); it != jBuffer.end(); ++it)
    {
        if(oOneSoul["buff"][(*it).c_str()][0U].asUInt() != 0 &&
            oOneSoul["buff"][(*it).c_str()][1U].asInt() != 0)
        {
            pstParts->astBuffInfo[pstParts->udwBufferNum].m_udwId = oOneSoul["buff"][(*it).c_str()][0U].asUInt();
            pstParts->astBuffInfo[pstParts->udwBufferNum].m_dwMinNum = oOneSoul["buff"][(*it).c_str()][1U].asInt();
            pstParts->astBuffInfo[pstParts->udwBufferNum].m_dwMaxNum = oOneSoul["buff"][(*it).c_str()][2U].asInt();
            pstParts->astBuffInfo[pstParts->udwBufferNum].m_udwType = oOneSoul["buff"][(*it).c_str()][4U].asUInt();
            pstParts->udwBufferNum++;
        }
    }
    
    return 0;
}

TINT32 CBackpack::GetSpCrystalInfoById(TUINT32 udwCrystalId, SCrystalInfo *pstCrystal)
{
    pstCrystal->Reset();
    pstCrystal->udwId = udwCrystalId;

    const Json::Value &oCrystalJson = CBackpackInfo::GetInstance()->m_oJsonRoot["equip_special_crystal"];

    if(!oCrystalJson.isMember(CCommonFunc::NumToString(udwCrystalId)))
    {
        //不存在该crystal
        return -1;
    }
    const Json::Value &oOneCrystal = oCrystalJson[CCommonFunc::NumToString(udwCrystalId)];
    pstCrystal->udwType = oOneCrystal["b"][0U].asUInt();
    pstCrystal->udwLv = oOneCrystal["b"][1U].asUInt();

    Json::Value::Members jBuffer = oOneCrystal["buff"].getMemberNames();
    for(Json::Value::Members::iterator it = jBuffer.begin(); it != jBuffer.end(); ++it)
    {
        if(oOneCrystal["buff"][(*it).c_str()][0U].asUInt() != 0 &&
            oOneCrystal["buff"][(*it).c_str()][1U].asInt() != 0)
        {
            pstCrystal->astBuffInfo[pstCrystal->udwBufferNum].m_udwId = oOneCrystal["buff"][(*it).c_str()][0U].asUInt();
            pstCrystal->astBuffInfo[pstCrystal->udwBufferNum].m_dwNum = oOneCrystal["buff"][(*it).c_str()][1U].asInt();
            pstCrystal->astBuffInfo[pstCrystal->udwBufferNum].m_udwType = oOneCrystal["buff"][(*it).c_str()][4U].asUInt();
            pstCrystal->udwBufferNum++;
        }
    }
    return 0;
}

//add
TVOID CBackpack::AddEquip(SUserInfo *pstUser, SEquipMentInfo *pstEquip)
{
    TbEquip *ptbEquip = &pstUser->m_atbEquip[pstUser->m_udwEquipNum];
    ptbEquip->Reset();

    assert(pstUser->m_tbPlayer.m_nUid);

    ptbEquip->Set_Id(pstEquip->uddwId);
    ptbEquip->Set_Equip_type(pstEquip->stBaseInfo.udwEType);
    ptbEquip->Set_Equip_lv(pstEquip->stBaseInfo.udwLv);
    ptbEquip->Set_Put_on_time(pstEquip->stStatusInfo.udwEquipmentPutOnTime);
    ptbEquip->Set_Status(pstEquip->stStatusInfo.udwStatus);
    ptbEquip->Set_Uid(pstUser->m_tbPlayer.m_nUid);
    if (pstEquip->stStatusInfo.udwStatus == EN_EQUIPMENT_STATUS_NORMAL)
    {
        ptbEquip->Set_Get_time(CTimeUtils::GetUnixTime());
    }
    for(TUINT32 udwIdx = 0; udwIdx < MAX_CRYSTAL_IN_EQUIPMENT; ++udwIdx)
    {
        ptbEquip->m_bCrystal[0].m_addwNum[udwIdx] = pstEquip->stStatusInfo.audwSlot[udwIdx];
    }
    ptbEquip->SetFlag(TbEQUIP_FIELD_CRYSTAL);

    for(TUINT32 udwIdx = 0; udwIdx < pstEquip->stStatusInfo.udwBufferNum; ++udwIdx)
    {
        TUINT32 udwBuffId = pstEquip->stStatusInfo.astBuffInfo[udwIdx].m_udwId;
        TINT32 dwBuffNum = pstEquip->stStatusInfo.astBuffInfo[udwIdx].m_dwNum;
        TUINT32 udwBuffType = pstEquip->stStatusInfo.astBuffInfo[udwIdx].m_udwType;

        ptbEquip->m_jBuff[CCommonFunc::NumToString(udwBuffId)] = Json::Value(Json::arrayValue);

        ptbEquip->m_jBuff[CCommonFunc::NumToString(udwBuffId)].append(udwBuffId);
        ptbEquip->m_jBuff[CCommonFunc::NumToString(udwBuffId)].append(dwBuffNum);
        ptbEquip->m_jBuff[CCommonFunc::NumToString(udwBuffId)].append(udwBuffType);
    }
    ptbEquip->SetFlag(TbEQUIP_FIELD_BUFF);

    for(TUINT32 udwIdx = 0; udwIdx < pstEquip->stStatusInfo.udwMistoryBufferNum; ++udwIdx)
    {
        TUINT32 udwBuffId = pstEquip->stStatusInfo.astMisteryBuffInfo[udwIdx].m_udwId;
        TINT32 dwBuffNum = pstEquip->stStatusInfo.astMisteryBuffInfo[udwIdx].m_dwNum;
        TUINT32 udwBuffType = pstEquip->stStatusInfo.astMisteryBuffInfo[udwIdx].m_udwType;

        ptbEquip->m_jMistery_buff[CCommonFunc::NumToString(udwBuffId)] = Json::Value(Json::arrayValue);

        ptbEquip->m_jMistery_buff[CCommonFunc::NumToString(udwBuffId)].append(udwBuffId);
        ptbEquip->m_jMistery_buff[CCommonFunc::NumToString(udwBuffId)].append(dwBuffNum);
        ptbEquip->m_jMistery_buff[CCommonFunc::NumToString(udwBuffId)].append(udwBuffType);
    }
    ptbEquip->SetFlag(TbEQUIP_FIELD_MISTERY_BUFF);

    // 3. set flag
    pstUser->m_aucEquipFlag[pstUser->m_udwEquipNum] = EN_TABLE_UPDT_FLAG__NEW;
    // 4. auto increase
    pstUser->m_udwEquipNum++;

}

TINT32 CBackpack::AddNormalEquip(SUserInfo *pstUser, TUINT32 udwEquipType, TUINT32 udwEquipLv, TINT32 udwNum)
{
    TINT32 dwCost = 0;
    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwEquipNum; ++udwIdx)
    {
        if (pstUser->m_aucEquipFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }
        dwCost++;
    }
    if (pstUser->m_tbUserStat.m_nEquip_gride < dwCost + udwNum)
    {
        return EN_RET_CODE__EQUIP_GRIDE_NOT_ENOUGH;
    }
    SEquipMentInfo stEquip;
    stEquip.Reset();

    TbLogin *pstLogin = &pstUser->m_tbLogin;

    const Json::Value &jEquip = CBackpackInfo::GetInstance()->m_oJsonRoot;

    string szEquipType = CCommonFunc::NumToString(udwEquipType);
    if (!jEquip["equip_equipment"].isMember(szEquipType))
    {
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("AddNormalEquip: equip_type error [id:%u] [seq=%u]",
            udwEquipType, pstUser->m_udwBSeqNo));
        return EN_RET_CODE__REQ_PARAM_ERROR;
    }

    if (udwEquipLv <= 0)
    {
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("AddNormalEquip: equip_lv[%d] error [seq=%u]",
            udwEquipLv, pstUser->m_udwBSeqNo));
        return EN_RET_CODE__REQ_PARAM_ERROR;
    }

    //base info
    stEquip.stBaseInfo.udwLv = udwEquipLv;
    GetEquipBaseInfoByEid(udwEquipType, &stEquip);

    //buff
    const Json::Value &oEquipJson = CBackpackInfo::GetInstance()->m_oJsonRoot["equip_equipment"];
    Json::Value::Members oBuffMember = oEquipJson[szEquipType]["buff"][udwEquipLv - 1].getMemberNames();
    for (Json::Value::Members::iterator it = oBuffMember.begin(); it != oBuffMember.end(); ++it)
    {
        stEquip.stStatusInfo.astBuffInfo[stEquip.stStatusInfo.udwBufferNum].m_udwId = oEquipJson[szEquipType]["buff"][udwEquipLv - 1][(*it).c_str()][0U].asUInt();
        stEquip.stStatusInfo.astBuffInfo[stEquip.stStatusInfo.udwBufferNum].m_dwNum = oEquipJson[szEquipType]["buff"][udwEquipLv - 1][(*it).c_str()][1U].asInt();
        stEquip.stStatusInfo.astBuffInfo[stEquip.stStatusInfo.udwBufferNum].m_udwType = oEquipJson[szEquipType]["buff"][udwEquipLv - 1][(*it).c_str()][3U].asUInt();
        stEquip.stStatusInfo.udwBufferNum++;
    }

    stEquip.stStatusInfo.udwStatus = EN_EQUIPMENT_STATUS_NORMAL;

    for (TUINT32 udwIdx = 0; udwIdx < udwNum; udwIdx++)
    {
        stEquip.uddwId = GenEquipId(pstLogin);
        AddEquip(pstUser, &stEquip);
    }

    return 0;
}

TVOID CBackpack::AddCrystal(SUserInfo *pstUser, TUINT32 udwCrystalId, TUINT32 udwNum)
{
    TbBackpack *pstBackpack = &pstUser->m_tbBackpack;
    if(pstBackpack->m_jCrystal.isMember(CCommonFunc::NumToString(udwCrystalId)))
    {
        pstBackpack->m_jCrystal[CCommonFunc::NumToString(udwCrystalId)] = pstBackpack->m_jCrystal[CCommonFunc::NumToString(udwCrystalId)].asUInt() + udwNum;
    }
    else
    {
        pstBackpack->m_jCrystal[CCommonFunc::NumToString(udwCrystalId)] = udwNum;
    }
    pstBackpack->SetFlag(TbBACKPACK_FIELD_CRYSTAL);
}

TVOID CBackpack::SetCrystal(SUserInfo *pstUser, TUINT32 udwCrystalId, TUINT32 udwNum)
{
    TbBackpack *pstBackpack = &pstUser->m_tbBackpack;
    if (udwNum > 0)
    {
        pstBackpack->m_jCrystal[CCommonFunc::NumToString(udwCrystalId)] = udwNum;
    }
    else
    {
        pstBackpack->m_jCrystal.removeMember(CCommonFunc::NumToString(udwCrystalId));
    }
    pstBackpack->SetFlag(TbBACKPACK_FIELD_CRYSTAL);
}

TVOID CBackpack::AddSoul(SUserInfo *pstUser, TUINT32 udwSoulId, TUINT32 udwNum)
{
    TbBackpack *pstBackpack = &pstUser->m_tbBackpack;
    if(pstBackpack->m_jSoul.isMember(CCommonFunc::NumToString(udwSoulId)))
    {
        pstBackpack->m_jSoul[CCommonFunc::NumToString(udwSoulId)] = pstBackpack->m_jSoul[CCommonFunc::NumToString(udwSoulId)].asUInt() + udwNum;
    }
    else
    {
        pstBackpack->m_jSoul[CCommonFunc::NumToString(udwSoulId)] = udwNum;
    }
    pstBackpack->SetFlag(TbBACKPACK_FIELD_SOUL);
}

TVOID CBackpack::AddParts(SUserInfo *pstUser, TUINT32 udwPartsId, TUINT32 udwNum)
{
    TbBackpack *pstBackpack = &pstUser->m_tbBackpack;
    if(pstBackpack->m_jParts.isMember(CCommonFunc::NumToString(udwPartsId)))
    {
        pstBackpack->m_jParts[CCommonFunc::NumToString(udwPartsId)] = pstBackpack->m_jParts[CCommonFunc::NumToString(udwPartsId)].asUInt() + udwNum;
    }
    else
    {
        pstBackpack->m_jParts[CCommonFunc::NumToString(udwPartsId)] = udwNum;
    }
    pstBackpack->SetFlag(TbBACKPACK_FIELD_PARTS);
}

TVOID CBackpack::OpAddMaterial(SUserInfo *pstUser, TUINT32 udwMaterialId, TINT64 ddwNum)
{

    TbBackpack *pstBackpack = &pstUser->m_tbBackpack;
    if(pstBackpack->m_jMaterial.isMember(CCommonFunc::NumToString(udwMaterialId)))
    {
        if(pstBackpack->m_jMaterial[CCommonFunc::NumToString(udwMaterialId)].asUInt() + ddwNum <= 0)            
        {   
            Json::Value &MaterialJson = pstBackpack->m_jMaterial;
            MaterialJson.removeMember(CCommonFunc::NumToString(udwMaterialId));
        }
        else
        {
            pstBackpack->m_jMaterial[CCommonFunc::NumToString(udwMaterialId)] = pstBackpack->m_jMaterial[CCommonFunc::NumToString(udwMaterialId)].asUInt() + ddwNum;
        }
    }
    else
    {
        if(ddwNum >= 0)
        {
            pstBackpack->m_jMaterial[CCommonFunc::NumToString(udwMaterialId)] = ddwNum;            
        }
        else
        {
            Json::Value &MaterialJson = pstBackpack->m_jMaterial;
            MaterialJson.removeMember(CCommonFunc::NumToString(udwMaterialId));
        }
    }
    pstBackpack->SetFlag(TbBACKPACK_FIELD_MATERIAL);
}

TVOID CBackpack::OpSetMaterial(SUserInfo *pstUser, TUINT32 udwMaterialId, TUINT32 udwNum)
{
    TbBackpack *pstBackpack = &pstUser->m_tbBackpack;
    if (pstBackpack->m_jMaterial.isMember(CCommonFunc::NumToString(udwMaterialId)))
    {
        if (udwNum == 0)
        {
            pstBackpack->m_jMaterial.removeMember(CCommonFunc::NumToString(udwMaterialId));
        }
        else
        {
            pstBackpack->m_jMaterial[CCommonFunc::NumToString(udwMaterialId)] = udwNum;
        }
    }
    else
    {
        if (udwNum > 0)
        {
            pstBackpack->m_jMaterial[CCommonFunc::NumToString(udwMaterialId)] = udwNum;
        }
        else
        {
            pstBackpack->m_jMaterial.removeMember(CCommonFunc::NumToString(udwMaterialId));
        }
    }
    pstBackpack->SetFlag(TbBACKPACK_FIELD_MATERIAL);
}

TVOID CBackpack::AddMaterial(SUserInfo *pstUser, TUINT32 udwMaterialId, TUINT32 udwNum)
{
    TbBackpack *pstBackpack = &pstUser->m_tbBackpack;
    if(pstBackpack->m_jMaterial.isMember(CCommonFunc::NumToString(udwMaterialId)))
    {
        pstBackpack->m_jMaterial[CCommonFunc::NumToString(udwMaterialId)] = pstBackpack->m_jMaterial[CCommonFunc::NumToString(udwMaterialId)].asUInt() + udwNum;
    }
    else
    {
        pstBackpack->m_jMaterial[CCommonFunc::NumToString(udwMaterialId)] = udwNum;
    }
    pstBackpack->SetFlag(TbBACKPACK_FIELD_MATERIAL);
}


TVOID CBackpack::AddSpCrystal(SUserInfo *pstUser, TUINT32 udwCrystalId, TUINT32 udwNum)
{
    TbBackpack *pstBackpack = &pstUser->m_tbBackpack;
    if(pstBackpack->m_jSp_crystal.isMember(CCommonFunc::NumToString(udwCrystalId)))
    {
        pstBackpack->m_jSp_crystal[CCommonFunc::NumToString(udwCrystalId)] = pstBackpack->m_jSp_crystal[CCommonFunc::NumToString(udwCrystalId)].asUInt() + udwNum;
    }
    else
    {
        pstBackpack->m_jSp_crystal[CCommonFunc::NumToString(udwCrystalId)] = udwNum;
    }
    pstBackpack->SetFlag(TbBACKPACK_FIELD_SP_CRYSTAL);
}

TVOID CBackpack::AddScroll(SUserInfo *pstUser, TUINT32 udwScrollId, TUINT32 udwNum)
{
    TbBackpack *ptbBackpack = &pstUser->m_tbBackpack;
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    if (ptbBackpack->m_jScroll.isMember(CCommonFunc::NumToString(udwScrollId)))
    {
        ptbBackpack->m_jScroll[CCommonFunc::NumToString(udwScrollId)] = ptbBackpack->m_jScroll[CCommonFunc::NumToString(udwScrollId)].asUInt() + udwNum;
    }
    else
    {
        ptbBackpack->m_jScroll[CCommonFunc::NumToString(udwScrollId)] = udwNum;
    }
    ptbBackpack->SetFlag(TbBACKPACK_FIELD_SCROLL);

    ptbBackpack->m_jScroll_get_time[CCommonFunc::NumToString(udwScrollId)] = udwCurTime;
    ptbBackpack->SetFlag(TbBACKPACK_FIELD_SCROLL_GET_TIME);
}

TVOID CBackpack::SetScroll(SUserInfo *pstUser, TUINT32 udwScrollId, TUINT32 udwNum)
{
    TbBackpack *ptbBackpack = &pstUser->m_tbBackpack;
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    if (udwNum > 0)
    {
        ptbBackpack->m_jScroll[CCommonFunc::NumToString(udwScrollId)] = udwNum;
        ptbBackpack->SetFlag(TbBACKPACK_FIELD_SCROLL);

        ptbBackpack->m_jScroll_get_time[CCommonFunc::NumToString(udwScrollId)] = udwCurTime;
        ptbBackpack->SetFlag(TbBACKPACK_FIELD_SCROLL_GET_TIME);
    }
    else
    {
        ptbBackpack->m_jScroll.removeMember(CCommonFunc::NumToString(udwScrollId));
        ptbBackpack->SetFlag(TbBACKPACK_FIELD_SCROLL);
        ptbBackpack->m_jScroll_get_time.removeMember(CCommonFunc::NumToString(udwScrollId));
        ptbBackpack->SetFlag(TbBACKPACK_FIELD_SCROLL_GET_TIME);
    }
}

//has enough
TBOOL CBackpack::HasEnoughCrystal(SUserInfo *pstUser, TUINT32 udwCrystalId, TUINT32 udwNum)
{
    TbBackpack *pstBackpack = &pstUser->m_tbBackpack;
    TBOOL bEnough = FALSE;
    if(pstBackpack->m_jCrystal.isMember(CCommonFunc::NumToString(udwCrystalId)))
    {
        if(pstBackpack->m_jCrystal[CCommonFunc::NumToString(udwCrystalId)].asUInt() >= udwNum)
        {
            bEnough = TRUE;
        }
    }
    return bEnough;
}

TBOOL CBackpack::HasEnoughSoul(SUserInfo *pstUser, TUINT32 udwSoulId, TUINT32 udwNum)
{
    TbBackpack *pstBackpack = &pstUser->m_tbBackpack;
    TBOOL bEnough = FALSE;
    if(pstBackpack->m_jSoul.isMember(CCommonFunc::NumToString(udwSoulId)))
    {
        if(pstBackpack->m_jSoul[CCommonFunc::NumToString(udwSoulId)].asUInt() >= udwNum)
        {
            bEnough = TRUE;
        }
    }
    return bEnough;
}

TBOOL CBackpack::HasEnoughParts(SUserInfo *pstUser, TUINT32 udwPartsId, TUINT32 udwNum)
{
    TbBackpack *pstBackpack = &pstUser->m_tbBackpack;
    TBOOL bEnough = FALSE;
    if(pstBackpack->m_jParts.isMember(CCommonFunc::NumToString(udwPartsId)))
    {
        if(pstBackpack->m_jParts[CCommonFunc::NumToString(udwPartsId)].asUInt() >= udwNum)
        {
            bEnough = TRUE;
        }
    }
    return bEnough;
}

TBOOL CBackpack::HasEnoughMaterial(SUserInfo *pstUser, TUINT32 udwMaterialId, TUINT32 udwNum)
{
    TbBackpack *pstBackpack = &pstUser->m_tbBackpack;
    TBOOL bEnough = FALSE;
    if(pstBackpack->m_jMaterial.isMember(CCommonFunc::NumToString(udwMaterialId)))
    {
        if(pstBackpack->m_jMaterial[CCommonFunc::NumToString(udwMaterialId)].asUInt() >= udwNum)
        {
            bEnough = TRUE;
        }
    }
    return bEnough;
}

TBOOL CBackpack::HasEnoughSpCrystal(SUserInfo *pstUser, TUINT32 udwCrystalId, TUINT32 udwNum)
{
    TbBackpack *pstBackpack = &pstUser->m_tbBackpack;
    TBOOL bEnough = FALSE;
    if(pstBackpack->m_jSp_crystal.isMember(CCommonFunc::NumToString(udwCrystalId)))
    {
        if(pstBackpack->m_jSp_crystal[CCommonFunc::NumToString(udwCrystalId)].asUInt() >= udwNum)
        {
            bEnough = TRUE;
        }
    }
    return bEnough;
}

TBOOL CBackpack::HasEnoughScroll(SUserInfo *pstUser, TUINT32 udwScrollId, TUINT32 udwNum)
{
    TbBackpack *ptbBackpack = &pstUser->m_tbBackpack;
    TBOOL bEnough = FALSE;
    if (ptbBackpack->m_jScroll.isMember(CCommonFunc::NumToString(udwScrollId)))
    {
        if (ptbBackpack->m_jScroll[CCommonFunc::NumToString(udwScrollId)].asUInt() >= udwNum)
        {
            bEnough = TRUE;
        }
    }
    return bEnough;
}

//cost
TVOID CBackpack::CostCrystal(SUserInfo *pstUser, TUINT32 udwCrystalId, TUINT32 udwNum)
{
    TbBackpack *pstBackpack = &pstUser->m_tbBackpack;
    if(pstBackpack->m_jCrystal.isMember(CCommonFunc::NumToString(udwCrystalId)))
    {
        if(pstBackpack->m_jCrystal[CCommonFunc::NumToString(udwCrystalId)].asUInt() > udwNum)
        {
            pstBackpack->m_jCrystal[CCommonFunc::NumToString(udwCrystalId)] = pstBackpack->m_jCrystal[CCommonFunc::NumToString(udwCrystalId)].asUInt() - udwNum;
        }
        else
        {
            Json::Value &CrystalJson = pstBackpack->m_jCrystal;
            CrystalJson.removeMember(CCommonFunc::NumToString(udwCrystalId));
        }
        pstBackpack->SetFlag(TbBACKPACK_FIELD_CRYSTAL);
    }
}

TVOID CBackpack::CostSoul(SUserInfo *pstUser, TUINT32 udwSoulId, TUINT32 udwNum)
{
    TbBackpack *pstBackpack = &pstUser->m_tbBackpack;
    if(pstBackpack->m_jSoul.isMember(CCommonFunc::NumToString(udwSoulId)))
    {
        if(pstBackpack->m_jSoul[CCommonFunc::NumToString(udwSoulId)].asUInt() > udwNum)
        {
            pstBackpack->m_jSoul[CCommonFunc::NumToString(udwSoulId)] = pstBackpack->m_jSoul[CCommonFunc::NumToString(udwSoulId)].asUInt() - udwNum;
        }
        else
        {
            Json::Value &SoulJson = pstBackpack->m_jSoul;
            SoulJson.removeMember(CCommonFunc::NumToString(udwSoulId));
        }
        pstBackpack->SetFlag(TbBACKPACK_FIELD_SOUL);
    }
}

TVOID CBackpack::CostParts(SUserInfo *pstUser, TUINT32 udwPartsId, TUINT32 udwNum)
{
    TbBackpack *pstBackpack = &pstUser->m_tbBackpack;
    if(pstBackpack->m_jParts.isMember(CCommonFunc::NumToString(udwPartsId)))
    {
        if(pstBackpack->m_jParts[CCommonFunc::NumToString(udwPartsId)].asUInt() > udwNum)
        {
            pstBackpack->m_jParts[CCommonFunc::NumToString(udwPartsId)] = pstBackpack->m_jParts[CCommonFunc::NumToString(udwPartsId)].asUInt() - udwNum;
        }
        else
        {
            Json::Value &PartsJson = pstBackpack->m_jParts;
            PartsJson.removeMember(CCommonFunc::NumToString(udwPartsId));
        }
        pstBackpack->SetFlag(TbBACKPACK_FIELD_PARTS);
    }
}

TVOID CBackpack::CostMaterial(SUserInfo *pstUser, TUINT32 udwMaterialId, TUINT32 udwNum)
{
    TbBackpack *pstBackpack = &pstUser->m_tbBackpack;
    if(pstBackpack->m_jMaterial.isMember(CCommonFunc::NumToString(udwMaterialId)))
    {
        if(pstBackpack->m_jMaterial[CCommonFunc::NumToString(udwMaterialId)].asUInt() > udwNum)
        {
            pstBackpack->m_jMaterial[CCommonFunc::NumToString(udwMaterialId)] = pstBackpack->m_jMaterial[CCommonFunc::NumToString(udwMaterialId)].asUInt() - udwNum;
        }
        else
        {
            Json::Value &MaterialJson = pstBackpack->m_jMaterial;
            MaterialJson.removeMember(CCommonFunc::NumToString(udwMaterialId));
        }
        pstBackpack->SetFlag(TbBACKPACK_FIELD_MATERIAL);
    }
}

TVOID CBackpack::CostSpCrystal(SUserInfo *pstUser, TUINT32 udwCrystalId, TUINT32 udwNum)
{
    TbBackpack *pstBackpack = &pstUser->m_tbBackpack;
    if(pstBackpack->m_jSp_crystal.isMember(CCommonFunc::NumToString(udwCrystalId)))
    {
        if(pstBackpack->m_jSp_crystal[CCommonFunc::NumToString(udwCrystalId)].asUInt() > udwNum)
        {
            pstBackpack->m_jSp_crystal[CCommonFunc::NumToString(udwCrystalId)] = pstBackpack->m_jSp_crystal[CCommonFunc::NumToString(udwCrystalId)].asUInt() - udwNum;
        }
        else
        {
            Json::Value &CrystalJson = pstBackpack->m_jSp_crystal;
            CrystalJson.removeMember(CCommonFunc::NumToString(udwCrystalId));
        }
        pstBackpack->SetFlag(TbBACKPACK_FIELD_SP_CRYSTAL);
    }
}

TVOID CBackpack::CostScroll(SUserInfo *pstUser, TUINT32 udwScrollId, TUINT32 udwNum)
{
    TbBackpack *ptbBackpack = &pstUser->m_tbBackpack;
    if (ptbBackpack->m_jScroll.isMember(CCommonFunc::NumToString(udwScrollId)))
    {
        if (ptbBackpack->m_jScroll[CCommonFunc::NumToString(udwScrollId)].asUInt() > udwNum)
        {
            ptbBackpack->m_jScroll[CCommonFunc::NumToString(udwScrollId)] = ptbBackpack->m_jScroll[CCommonFunc::NumToString(udwScrollId)].asUInt() - udwNum;
        }
        else
        {
            ptbBackpack->m_jScroll.removeMember(CCommonFunc::NumToString(udwScrollId));
            ptbBackpack->m_jScroll_get_time.removeMember(CCommonFunc::NumToString(udwScrollId));
            ptbBackpack->SetFlag(TbBACKPACK_FIELD_SCROLL_GET_TIME);
        }
        ptbBackpack->SetFlag(TbBACKPACK_FIELD_SCROLL);
    }
}

TVOID CBackpack::DropScroll(SUserInfo *pstUser, TUINT32 udwScrollId)
{
    TbBackpack *ptbBackpack = &pstUser->m_tbBackpack;

    if (ptbBackpack->m_jScroll.isMember(CCommonFunc::NumToString(udwScrollId)))
    {
        ptbBackpack->m_jScroll.removeMember(CCommonFunc::NumToString(udwScrollId));
        ptbBackpack->m_jScroll_get_time.removeMember(CCommonFunc::NumToString(udwScrollId));
        ptbBackpack->SetFlag(TbBACKPACK_FIELD_SCROLL);
        ptbBackpack->SetFlag(TbBACKPACK_FIELD_SCROLL_GET_TIME);

    }
}

//get num
TINT32 CBackpack::GetCrystalNumById(SUserInfo *pstUser, TUINT32 udwId)
{
    TbBackpack *pstBackpack = &pstUser->m_tbBackpack;
    return pstBackpack->m_jCrystal.isMember(CCommonFunc::NumToString(udwId)) ? pstBackpack->m_jCrystal[CCommonFunc::NumToString(udwId)].asUInt() : 0;
}

TINT32 CBackpack::GetSoulNumById(SUserInfo *pstUser, TUINT32 udwId)
{
    TbBackpack *pstBackpack = &pstUser->m_tbBackpack;
    return pstBackpack->m_jSoul.isMember(CCommonFunc::NumToString(udwId)) ? pstBackpack->m_jSoul[CCommonFunc::NumToString(udwId)].asUInt() : 0;
}

TINT32 CBackpack::GetPartsNumById(SUserInfo *pstUser, TUINT32 udwId)
{
    TbBackpack *pstBackpack = &pstUser->m_tbBackpack;
    return pstBackpack->m_jParts.isMember(CCommonFunc::NumToString(udwId)) ? pstBackpack->m_jParts[CCommonFunc::NumToString(udwId)].asUInt() : 0;
}

TINT32 CBackpack::GetMaterialNumById(SUserInfo *pstUser, TUINT32 udwId)
{
    TbBackpack *pstBackpack = &pstUser->m_tbBackpack;
    return pstBackpack->m_jMaterial.isMember(CCommonFunc::NumToString(udwId)) ? pstBackpack->m_jMaterial[CCommonFunc::NumToString(udwId)].asUInt() : 0;
}

TINT32 CBackpack::GetSpCrystalNumById(SUserInfo *pstUser, TUINT32 udwId)
{
    TbBackpack *pstBackpack = &pstUser->m_tbBackpack;
    return pstBackpack->m_jSp_crystal.isMember(CCommonFunc::NumToString(udwId)) ? pstBackpack->m_jSp_crystal[CCommonFunc::NumToString(udwId)].asUInt() : 0;
}

//destroy equip function
TINT32 CBackpack::GetMaterialIdByTypeAndLv(TUINT32 udwType, TUINT32 udwLv)
{
    TUINT32 udwMaterialId = 0;
    const Json::Value &oMaterialJson = CBackpackInfo::GetInstance()->m_oJsonRoot["equip_material"];
    Json::Value::Members oMaterialMember = oMaterialJson.getMemberNames();
    for(Json::Value::Members::iterator it = oMaterialMember.begin(); it != oMaterialMember.end();++it)
    {
        if(oMaterialJson[(*it).c_str()]["b"][0U].asUInt() == udwType && 
            oMaterialJson[(*it).c_str()]["b"][1U].asUInt() == udwLv)
        {
            udwMaterialId = atoi((*it).c_str());
            break;
        }
    }
    return udwMaterialId;
}

TINT32 CBackpack::GetMaterialTypeByEquipType(TUINT32 udwEquipType)
{
    vector<TUINT32> vecMaterialType;
    const Json::Value &oComposeJson = CBackpackInfo::GetInstance()->m_oJsonRoot["equip_equipment"][CCommonFunc::NumToString(udwEquipType)]["c"];
    for (TUINT32 udwIdx = 0; udwIdx < oComposeJson.size(); udwIdx++)
    {
        if (oComposeJson[udwIdx][0U].asInt() == EN_GLOBALRES_TYPE_MATERIAL_TYPE)
        {
            vecMaterialType.push_back(oComposeJson[udwIdx][1U].asUInt());
        }
    }

    assert(vecMaterialType.size() != 0);
    TUINT32 udwRand = rand() % vecMaterialType.size();

    return vecMaterialType[udwRand];

}

TINT32 CBackpack::CheckSoulEquip(SUserInfo *pstUser, TbPlayer *pstPlayer)
{
    if(pstPlayer == NULL)
    {
        return 0;
    }
    SEquipMentInfo stEquip;
    TINT32 dwRetCode = 0;

    TINT64 udwBuffEffect = pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_CORE_EQUIPMENT_DURATION].m_ddwBuffTotal;
    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwEquipNum;++udwIdx)
    {
        stEquip.Reset();
        TUINT64 uddwEquipId = pstUser->m_atbEquip[udwIdx].m_nId;
        if(pstUser ->m_aucEquipFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL 
            || uddwEquipId == 0)
        {
            continue;
        }
        dwRetCode = CBackpack::GetEquipInfoById(pstUser->m_atbEquip, pstUser->m_udwEquipNum, uddwEquipId, &stEquip);
        if(0 != dwRetCode)
        {
            TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("ChectEquipTimeEffect:get equip info fail [uid=%u id=%ld ][ret=%d] [seq=%u]", 
                pstPlayer->m_nUid, uddwEquipId, dwRetCode, pstUser->m_udwBSeqNo));
            return -1;
        }
        if(stEquip.stBaseInfo.udwCategory == EN_EQUIP_CATEGORY_NORMAL)
        {
            continue;
        }
        TFLOAT64 ffRate = 0.0001;
        TUINT32 udwBuffEffectTime = stEquip.stBaseInfo.udwEffectTime * (1.0 + ffRate*udwBuffEffect);
        TUINT32 udwEffectEndTime = stEquip.stStatusInfo.udwEquipmentPutOnTime + udwBuffEffectTime;
        if(stEquip.stStatusInfo.udwEquipmentPutOnTime != 0 && 
            udwEffectEndTime < CTimeUtils::GetUnixTime())
        {
            //crystal
            for(TUINT32 udwCrystalIdx = 0; udwCrystalIdx < MAX_CRYSTAL_IN_EQUIPMENT; ++udwCrystalIdx)
            {
                if(stEquip.stStatusInfo.audwSlot[udwCrystalIdx] == 0)
                {
                    continue;
                }
                CBackpack::AddCrystal(pstUser, stEquip.stStatusInfo.audwSlot[udwCrystalIdx]);
            }

            pstUser->m_aucEquipFlag[udwIdx] = EN_TABLE_UPDT_FLAG__DEL;
            //tips
            CSendMessageBase::AddTips(pstUser, EN_TIPS_TYPY__EQUIP_ABATE, pstPlayer->m_nUid, FALSE, stEquip.stBaseInfo.udwEType, 0, 0);
        }
    }
    return 0;
}
//=======================private========================
TINT32 CBackpack::GetEquipIdByTypeAndLv(TUINT32 udwType, TUINT32 udwLv)
{
    const Json::Value &oEquipJson = CBackpackInfo::GetInstance()->m_oJsonRoot["equip_equipment"];
    
    Json::Value::Members oEquipId = oEquipJson.getMemberNames();
    for(Json::Value::Members::iterator it = oEquipId.begin(); it != oEquipId.end();++it)
    {
        if(oEquipJson[(*it).c_str()]["b"][3U].asUInt() == udwType &&
            oEquipJson[(*it).c_str()]["b"][2U].asUInt() == udwLv)
        {
            return atoi((*it).c_str());
        }
    }
    return 0;
}

TINT32 CBackpack::GetNormalEquipType(vector<TUINT32> vMaterialType)
{
    const Json::Value &oComposeJson = CBackpackInfo::GetInstance()->m_oJsonRoot["equip_compose"];
    
    sort(vMaterialType.begin(), vMaterialType.end());

    TCHAR szBuffMsg[64] = {'\0'};
    TBOOL bHead = TRUE;
    TUINT32 udwLen = 0;
    for (TUINT32 udwIdx = 0; udwIdx < vMaterialType.size(); ++udwIdx)
    {
        if(bHead)
        {
            udwLen = snprintf(szBuffMsg, 64, "%u", vMaterialType[udwIdx]);
            bHead = FALSE;
        }
        else
        {
            udwLen += snprintf(szBuffMsg + udwLen, 64 - udwLen, "-%u", vMaterialType[udwIdx]);
        }
    }

    if(!oComposeJson["a"].isMember(szBuffMsg))
    {
        //无合成方案
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("CBackpack::GetNormalEquipType:can not find [type=%s] ",
            szBuffMsg));
        return -1;
    }
    return oComposeJson["a"][szBuffMsg].asInt();
}

TINT32 CBackpack::GetNormalEquipLv(TUINT32 udwMaterialLvTotal)
{
    const Json::Value &oCraftJson = CBackpackInfo::GetInstance()->m_oJsonRoot["equip_craft"];

    if(!oCraftJson.isMember(CCommonFunc::NumToString(udwMaterialLvTotal)))
    {
        //无合成等级方案
        return -1;
    }

    TUINT32 udwRate = rand() % 10000;
    TUINT32 udwRateTotal = 0;
    TUINT32 udwLv = 0;
    for(TUINT32 udwIdx = 0; udwIdx < oCraftJson[CCommonFunc::NumToString(udwMaterialLvTotal)]["p"].size() && udwIdx < MAX_EQUIP_LV; ++udwIdx)
    {
        udwRateTotal += oCraftJson[CCommonFunc::NumToString(udwMaterialLvTotal)]["p"][udwIdx].asUInt();
        if(udwRate <= udwRateTotal)
        {
            udwLv = udwIdx + 1;
            break;
        }
    }
    assert(udwLv != 0);
    return udwLv;
}

TINT32 CBackpack::GetNewNormalEquipLv(vector<TUINT32> vMaterialLv)
{
    const Json::Value &oCraftJson = CBackpackInfo::GetInstance()->m_oJsonRoot["equip_craft_new"];

    sort(vMaterialLv.begin(), vMaterialLv.end());

    TCHAR szBuffMsg[64] = {'\0'};
    TBOOL bHead = TRUE;
    TUINT32 udwLen = 0;
    for (TUINT32 udwIdx = 0; udwIdx < vMaterialLv.size(); udwIdx++)
    {
        if(bHead)
        {
            udwLen = snprintf(szBuffMsg, 64, "%u", vMaterialLv[udwIdx]);
            bHead = FALSE;
        }
        else
        {
            udwLen += snprintf(szBuffMsg + udwLen, 64 - udwLen, "-%u", vMaterialLv[udwIdx]);
        }
    }

    if(!CBackpackInfo::GetInstance()->m_oJsonRoot["equip_craft_new"].isMember(szBuffMsg))
    {
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("CBackpack::ComposeNormalEquip:conf not exit compose_info [equip_craft_new] [%s]", szBuffMsg));
        return -1;
    }

    TUINT32 udwRate = rand() % 10000;
    TUINT32 udwRateTotal = 0;
    TUINT32 dwLv = 0;
    for(TUINT32 udwIdx = 0; udwIdx < oCraftJson[szBuffMsg]["p"].size() && udwIdx < MAX_EQUIP_LV; ++udwIdx)
    {
        udwRateTotal += oCraftJson[CCommonFunc::NumToString(szBuffMsg)]["p"][udwIdx].asUInt();
        if(udwRate <= udwRateTotal)
        {
            dwLv = udwIdx + 1;
            break;
        }
    }
    assert(dwLv != 0);
    return dwLv;
}

TVOID CBackpack::GetMaterialLvStr(vector<TUINT32> vMaterialLv, string& sBuff)
{
    const Json::Value &oCraftJson = CBackpackInfo::GetInstance()->m_oJsonRoot["equip_craft_new"];

    sort(vMaterialLv.begin(), vMaterialLv.end());

    ostringstream oss;

    for (TUINT32 udwIdx = 0; udwIdx < vMaterialLv.size(); udwIdx++)
    {
        if (udwIdx > 0)
        {
            oss << "-";
        }
        oss << vMaterialLv[udwIdx];
    }
    sBuff = oss.str();

    return;
}

TINT32 CBackpack::GetEquipPos(TUINT32 udwEquipType)
{
    const Json::Value &oEquipJson = CBackpackInfo::GetInstance()->m_oJsonRoot["equip_equipment"];

    TCHAR szBuffMsg[MAX_TABLE_NAME_LEN] = { '\0' };
    snprintf(szBuffMsg, MAX_TABLE_NAME_LEN, "%u", udwEquipType);

    if (!oEquipJson.isMember(szBuffMsg))
    {
        return -1;
    }

    TINT32 dwPos = oEquipJson[szBuffMsg]["b"][0U].asInt();
    return dwPos;
}

TINT32 CBackpack::GenRandomValue(TINT32 dwInputNum,TUINT32 *audwOutputValue,TUINT32 udwOutPutNum)
{
    //reset
    memset(audwOutputValue, 0, sizeof(TUINT32)* udwOutPutNum);
    
    const Json::Value &oSoulCraftJson = CBackpackInfo::GetInstance()->m_oJsonRoot["equip_soul_craft"];
    TUINT32 udwValueA = oSoulCraftJson[0U].asUInt();
    TUINT32 udwValueB = oSoulCraftJson[1U].asUInt();
    TUINT32 udwValueC = oSoulCraftJson[2U].asUInt();

    double Denominator = 0;

    //分母
    TUINT32 udwIdx = 0;
    for(double Idx = 0; udwIdx < MAX_RANDOM_VALUE_NUM; Idx += 0.01, udwIdx++)
    {
        Denominator += pow(Idx, udwValueA*(1 + udwValueC *(dwInputNum / 10000)) - 1) * pow(1 - Idx, udwValueB - 1) * 1.0;
    }
    //分子及结果
    udwIdx = 0;
    for(double Idx = 0; Idx <= 1 && udwIdx < udwOutPutNum; Idx += 0.01, udwIdx++)
    {
        double Numerator = pow(Idx, udwValueA * (1 + udwValueC *(dwInputNum / 10000)) - 1) * pow(1 - Idx, udwValueB - 1) * 1.0;
        
        TINT64 udwTmp = (Numerator / Denominator) * 100000;
        audwOutputValue[udwIdx] = udwTmp % 100000;
    }
    return 0;
}

TINT32 CBackpack::GetRandomBuffNum(TINT32 dwResearchNum, TINT32 dwMinNum, TINT32 dwMaxNum)
{
    TUINT32 audwRandomValue[MAX_RANDOM_VALUE_NUM] = {0};
    CBackpack::GenRandomValue(dwResearchNum, audwRandomValue, MAX_RANDOM_VALUE_NUM);

    TUINT64 uddwTotalValue = 0;
    for(TUINT32 udwIdx = 0; udwIdx < MAX_RANDOM_VALUE_NUM;++udwIdx)
    {
        uddwTotalValue += audwRandomValue[udwIdx];
    }

    TUINT64 uddwRand = rand() % uddwTotalValue;
    TUINT32 udwTmpTotal = 0;
    double Value = 0;
    for(TUINT32 udwIdx = 0; udwIdx < MAX_RANDOM_VALUE_NUM; ++udwIdx, Value+=0.01)
    {
        udwTmpTotal += audwRandomValue[udwIdx];
        if(uddwRand <= udwTmpTotal)
        {
            break;
        }
    }
    return dwMinNum + Value * (dwMaxNum - dwMinNum);
}

TINT32 CBackpack::GetNormalEquipCostTime(TUINT32 udwEid, SMaterialInfo *aMaterialInfo, TUINT32 udwMaterialNum, TUINT64 uddwBuffNum)
{
    const Json::Value &oEquipJson = CBackpackInfo::GetInstance()->m_oJsonRoot["equip_equipment"];
    TUINT32 udwBaseCostTime = oEquipJson[CCommonFunc::NumToString(udwEid)]["b"][12U].asUInt();

    vector<SMaterialInfo> vMaterial;
    for(TUINT32 udwIdx = 0; udwIdx < udwMaterialNum;++udwIdx)
    {
        SMaterialInfo stCrystal;
        CBackpack::GetMaterialInfoById(aMaterialInfo[udwIdx].udwId, &stCrystal);
        vMaterial.push_back(stCrystal);
    }
    sort(vMaterial.begin(), vMaterial.end(), CBackpack::CompareMaterial);

    TCHAR szBuffMsg[64] = {'\0'};
    TBOOL bHead = TRUE;
    TUINT32 udwLen = 0;
    for(vector<SMaterialInfo>::iterator it = vMaterial.begin(); it < vMaterial.end();++it)
    {
        if(bHead)
        {
            udwLen = snprintf(szBuffMsg, 64, "%u", it->udwLv);
            bHead = FALSE;
        }
        else
        {
            udwLen += snprintf(szBuffMsg + udwLen, 64 - udwLen, "-%u", it->udwLv);
        }
    }
     
    if(!CBackpackInfo::GetInstance()->m_oJsonRoot["equip_craft_new"].isMember(szBuffMsg))
    {
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("CBackpack::ComposeNormalEquip:conf not exit compose_info [equip_craft_new] [%s]", szBuffMsg));
        return -1;
    }

    TUINT32 udwDif = CBackpackInfo::GetInstance()->m_oJsonRoot["equip_craft_new"][szBuffMsg]["t"].asUInt();

    TUINT32 udwReturnNum = (udwBaseCostTime * udwDif) / (1.0 * uddwBuffNum / 10000);
    TSE_LOG_INFO(CGameInfo::GetInstance()->m_poLog, ("CBackpack::ComposeNormalEquip: [real_cost_time=%u eid=%u time_buff=%lld] [base=%u dif=%u]",
        udwReturnNum, udwEid, uddwBuffNum, udwBaseCostTime, udwDif));

    return udwReturnNum;

}

TBOOL CBackpack::CompareMaterial(SMaterialInfo stMaterialA, SMaterialInfo stMaterialB)
{
    return stMaterialA.udwLv < stMaterialB.udwLv;
}
