#include "game_evaluate_logic.h"
#include "time_utils.h"
#include "city_base.h"
#include "item_logic.h"
#include "common_func.h"
#include "common_base.h"


TBOOL CGameEvaluateLogic::GenGameEvaluateData(SReqInfo *pstReqInfo, SUserInfo *pstUserInfo, SGameEvaluateData *pstGameEvaluateData)
{
    ostringstream ossContent;
    ossContent.str("");
    TINT32 dwCurTimeStamp = CTimeUtils::GetUnixTime();
    SCityInfo *pstCity = &pstUserInfo->m_stCityInfo;
    if(NULL == pstCity)
    {
        return false;
    }

    // key0(sid)
    pstGameEvaluateData->m_ddwSid = pstReqInfo->m_ddwSid;

    // key1(uid)
    pstGameEvaluateData->m_ddwUid = pstUserInfo->m_tbLogin.Get_Uid();
    
    // key2(idfa)
    pstGameEvaluateData->m_strIdfa = pstReqInfo->m_strIdfa; 
    
    // key3(cmd)
    ossContent << pstReqInfo->m_strCommand;
    for(TINT32 dwIdx = 0; dwIdx < MAX_REQ_PARAM_KEY_NUM; ++dwIdx)
    {
        ossContent << "&" << "key" << dwIdx << "=" << pstReqInfo->m_astrKey[dwIdx];
    }
    pstGameEvaluateData->m_strCmd = ossContent.str();
    ossContent.str("");    
    
    // key4(curtimestamp)
    pstGameEvaluateData->m_dwCurTimeStamp = dwCurTimeStamp;

    // key5(castle_lv)
    TINT64 ddwCastleAge = pstUserInfo->m_tbPlayer.Get_Age();
    TUINT32 ucCastleLv = (TUINT32)CCityBase::GetBuildingLevelByFuncType(pstCity, EN_BUILDING_TYPE__CASTLE);
    pstGameEvaluateData->m_addwCastleLv[0] = ddwCastleAge;
    pstGameEvaluateData->m_addwCastleLv[1] = (TINT64)ucCastleLv;

    // key6(lord_lv)
    pstGameEvaluateData->m_ddwLordLv = pstUserInfo->m_tbPlayer.m_nLevel;
    
    // key7(force)
    pstGameEvaluateData->m_ddwForce = pstUserInfo->m_tbPlayer.Get_Might();

    // key8(build_force)
    pstGameEvaluateData->m_ddwBuildForce = pstUserInfo->m_tbPlayer.Get_Building_force();

    // key9(research_force)
    pstGameEvaluateData->m_ddwResearchForce = pstUserInfo->m_tbPlayer.Get_Research_force();

    // key10(train_force)
    pstGameEvaluateData->m_ddwTrainForce = pstUserInfo->m_tbPlayer.Get_Mgain();

    // key11(hero_force)
    pstGameEvaluateData->m_ddwHeroForce = pstUserInfo->m_tbPlayer.Get_Dragon_force();

    // key12(equip_force)
    pstGameEvaluateData->m_ddwEquipForce = pstUserInfo->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_ADD_FORCE].m_astBuffDetail[EN_BUFF_TYPE_EQUIP].m_ddwNum;

    // key13(res_item_value)
    GetItemInfo(pstGameEvaluateData->m_stResItemValueMap, &pstUserInfo->m_tbBackpack, EN_GLOBALRES_TYPE_RESOURCE);

    // key14(time_item_value)
    GetItemInfo(pstGameEvaluateData->m_stTimeItemValueMap, &pstUserInfo->m_tbBackpack, EN_GLOBALRES_TYPE_SPEEDUP_TIME);

    // key15(res0)
    // key16(res1)
    // key17(res2)
    // key18(res3) 
    // key19(res4)
    for(TINT32 dwIdx = 0; dwIdx < EN_RESOURCE_TYPE__END; ++dwIdx)
    {
        pstGameEvaluateData->m_addwRes[dwIdx] = pstCity->m_stTblData.m_bResource[0].m_addwNum[dwIdx];
    }

    // key20(gem_cost_total)
    pstGameEvaluateData->m_ddwGemCostTotal = pstUserInfo->m_tbLogin.Get_Gem_cost();

    // key21(gem_buy_total)
    pstGameEvaluateData->m_ddwGemBuyTotal = pstUserInfo->m_tbLogin.Get_Gem_buy();

    // key22(conn_time)
    pstGameEvaluateData->m_ddwConnTime = 0;

    // key23(packet_size)
    pstGameEvaluateData->m_ddwPacketSize = pstReqInfo->m_ddwFinalPackLength;

    // key24(network_delay)
    pstGameEvaluateData->m_ddwNetworkDelay = pstReqInfo->m_ddwReqCost;

    // key25(f1)
    // key26(f2)
    // key27(f3)
    // key28(f4)
    for(TINT32 dwIdx = 0; dwIdx < F_SIZE; ++dwIdx)
    {
        //pstGameEvaluateData->m_addwF[dwIdx] = 0;
    }

    // key29(event_seq)
    pstGameEvaluateData->m_ddwEventSeq = pstUserInfo->m_uddwCurEventId;

    // key30(build_speed_buff)
    pstGameEvaluateData->m_ddwBuildSpeedBuff = pstUserInfo->m_stPlayerBuffList[EN_BUFFER_INFO_BUILDING_SPEED_UP_PERCENT].m_ddwBuffTotal;

    // key31(research_speed_buff)
    pstGameEvaluateData->m_ddwSearchSpeedBuff = pstUserInfo->m_stPlayerBuffList[EN_BUFFER_INFO_RESEARCH_SPEED_UP_PERCENT].m_ddwBuffTotal;

    // key32(troop_train_heal_speed_buff)
    pstGameEvaluateData->m_ddwTrainTroopSpeedBuff = pstUserInfo->m_stPlayerBuffList[EN_BUFFER_INFO_ALL_TROOP_TRAIN_SPEED].m_ddwBuffTotal;

    // key33(troop_train_reduce_res_buff)
    pstGameEvaluateData->m_ddwTrainTroopRedResBuff = pstUserInfo->m_stPlayerBuffList[EN_BUFFER_INFO_ALL_TROOP_TRAIN_COST].m_ddwBuffTotal;

    // key34(fort_train_speed_buff)
    pstGameEvaluateData->m_ddwTrainFortSpeedBuff = pstUserInfo->m_stPlayerBuffList[EN_BUFFER_INFO_BUILD_WALL_SPEED].m_ddwBuffTotal;

    // key35(equip_compose_speed_buff)
    pstGameEvaluateData->m_ddwComposEquipSepeedBuff = pstUserInfo->m_stPlayerBuffList[EN_BUFFER_INFO_EQUIPMENT_CRAFT_SPEED].m_ddwBuffTotal;

    // key36(equip_compose_reduce_res_buff)
    pstGameEvaluateData->m_ddwCompEquipRedResBuff = pstUserInfo->m_stPlayerBuffList[EN_BUFFER_INFO_EQUIPMENT_CRAFT_COST].m_ddwBuffTotal;

    // key37(create_time)
    pstGameEvaluateData->m_ddwCreateTime = pstUserInfo->m_tbLogin.Get_Ctime();

    // key38(platform)
    pstGameEvaluateData->m_strPlatform = pstUserInfo->m_tbLogin.Get_Platform();    

    // key39(troop_ability)
    GetTroopAbility(pstGameEvaluateData->m_stTroopAbilityVector, pstUserInfo);

    // key40(fort_ability)
    GetFortAbility(pstGameEvaluateData->m_stFortAbilityVector, pstUserInfo);
    
    // key41(own_action_info_no_march)
    GetOwnActionIdSecClassNoMarch(pstGameEvaluateData->m_stOwnActionInfoNoMatchVector, pstUserInfo);

    // key42(aid)
    pstGameEvaluateData->m_ddwAid = pstUserInfo->m_tbAlliance.Get_Aid();
    
    // key43(agift_lv)
    pstGameEvaluateData->m_ddwAlGiftLv = CCommonBase::GetAlGiftLevel(&pstUserInfo->m_tbAlliance);
    
    // key44(aforce)
    pstGameEvaluateData->m_ddwAForce = pstUserInfo->m_tbAlliance.Get_Might();

    // key45(gem_cur)
    pstGameEvaluateData->m_ddwGemCur = pstUserInfo->m_tbLogin.Get_Gem();    

    return true;
}


TBOOL CGameEvaluateLogic::GenGameEvaluateExData(SUserInfo *pstUserInfo, SGameEvaluateExData *pstGameEvaluateExDataResult, TINT64 ddwExDataUserType)
{

    SGameEvaluateExData *pstGameEvaluateExDataRaw = NULL;
    SGameEvaluateExData *pstGameEvaluateExDataNew = NULL;

    if(EN_EX_DATA_USER_TYPE_SOURCE == ddwExDataUserType)
    {
        pstGameEvaluateExDataRaw = &pstUserInfo->m_astGameEvaluateExDataRaw[0];
        pstGameEvaluateExDataNew = &pstUserInfo->m_astGameEvaluateExDataNew[0];
    }
    else if(EN_EX_DATA_USER_TYPE_TARGET == ddwExDataUserType)
    {
        pstGameEvaluateExDataRaw = &pstUserInfo->m_astGameEvaluateExDataRaw[1];
        pstGameEvaluateExDataNew = &pstUserInfo->m_astGameEvaluateExDataNew[1];
    }
    else
    {
        return false;
    }

    map<TINT64, TINT64>::iterator it_raw;
    map<TINT64, TINT64>::iterator it_new;
    
    map<TINT64, SBuildInfo>::iterator it_build_raw;
    map<TINT64, SBuildInfo>::iterator it_build_new;

    
    for(TINT32 dwIdx = 0; dwIdx < EN_USER_INFO_RECORD_TYPE__END; ++dwIdx)
    {  
        if(EN_USER_INFO_RECORD_TYPE__RESEARCH == dwIdx)
        {
            // 以raw为标准
            for(it_raw = pstGameEvaluateExDataRaw->m_astExDataMap[dwIdx].begin(); it_raw != pstGameEvaluateExDataRaw->m_astExDataMap[dwIdx].end(); it_raw++)
            {
                it_new = pstGameEvaluateExDataNew->m_astExDataMap[dwIdx].find(it_raw->first);
                if(it_new == pstGameEvaluateExDataNew->m_astExDataMap[dwIdx].end())
                {
                    // pstGameEvaluateExDataResult->m_astExDataMap[dwIdx][it_raw->first] = 0 - pstGameEvaluateExDataRaw->m_astExDataMap[dwIdx][it_raw->first];
                    pstGameEvaluateExDataResult->m_astExDataMap[dwIdx].insert(make_pair(it_raw->first, (0 - it_raw->second)));
                }
                else
                {
                    if(0 != (it_new->second - it_raw->second))
                    {
                        pstGameEvaluateExDataResult->m_astExDataMap[dwIdx].insert(make_pair(it_new->first, it_new->second));
                    }
                    /*
                    if(0 != (pstGameEvaluateExDataNew->m_astExDataMap[dwIdx][it_raw->first] - pstGameEvaluateExDataRaw->m_astExDataMap[dwIdx][it_raw->first]))
                    {
                        pstGameEvaluateExDataResult->m_astExDataMap[dwIdx][it_raw->first] = pstGameEvaluateExDataNew->m_astExDataMap[dwIdx][it_raw->first];
                    }
                    */
                }
            }
            
            // 以new为标准
            for(it_new = pstGameEvaluateExDataNew->m_astExDataMap[dwIdx].begin(); it_new != pstGameEvaluateExDataNew->m_astExDataMap[dwIdx].end(); it_new++)
            {
                it_raw = pstGameEvaluateExDataRaw->m_astExDataMap[dwIdx].find(it_new->first);
                if(it_raw == pstGameEvaluateExDataRaw->m_astExDataMap[dwIdx].end())
                {
                    pstGameEvaluateExDataResult->m_astExDataMap[dwIdx].insert(make_pair(it_new->first, it_new->second));
                    // pstGameEvaluateExDataResult->m_astExDataMap[dwIdx][it_new->first] = pstGameEvaluateExDataNew->m_astExDataMap[dwIdx][it_new->first];
                }
            }
        }
        else if(EN_USER_INFO_RECORD_TYPE__BUILDING == dwIdx)
        {
            // 以raw为标准
            for(it_build_raw = pstGameEvaluateExDataRaw->m_astExBuildDataMap.begin(); it_build_raw != pstGameEvaluateExDataRaw->m_astExBuildDataMap.end(); it_build_raw++)
            {
                it_build_new = pstGameEvaluateExDataNew->m_astExBuildDataMap.find(it_build_raw->first);
                if(it_build_new == pstGameEvaluateExDataNew->m_astExBuildDataMap.end())
                {
                    pstGameEvaluateExDataResult->m_astExBuildDataMap.insert(make_pair(it_build_raw->first, it_build_raw->second));
                    pstGameEvaluateExDataResult->m_astExBuildDataMap[it_build_raw->first].m_ddwBuildLevel = -1L * pstGameEvaluateExDataRaw->m_astExBuildDataMap[it_build_raw->first].m_ddwBuildLevel;
                    /*
                    pstGameEvaluateExDataResult->m_astExBuildDataMap[it_build_raw->first] = pstGameEvaluateExDataRaw->m_astExBuildDataMap[it_build_raw->first];
                    pstGameEvaluateExDataResult->m_astExBuildDataMap[it_build_raw->first].m_ddwBuildLevel = -1L * pstGameEvaluateExDataRaw->m_astExBuildDataMap[it_build_raw->first].m_ddwBuildLevel;
                    */
                }
                else
                {
                    if(0 != (it_build_new->second.m_ddwBuildLevel - it_build_raw->second.m_ddwBuildLevel))
                    {
                        pstGameEvaluateExDataResult->m_astExBuildDataMap.insert(make_pair(it_build_new->first, it_build_new->second));
                    }
                    /*
                    if(0 != (pstGameEvaluateExDataNew->m_astExBuildDataMap[it_build_raw->first].m_ddwBuildLevel - pstGameEvaluateExDataRaw->m_astExBuildDataMap[it_build_raw->first].m_ddwBuildLevel))
                    {
                        pstGameEvaluateExDataResult->m_astExBuildDataMap[it_build_raw->first] = pstGameEvaluateExDataNew->m_astExBuildDataMap[it_build_raw->first];
                    }
                    */
                }
            }
            
            // 以new为标准
            for(it_build_new = pstGameEvaluateExDataNew->m_astExBuildDataMap.begin(); it_build_new != pstGameEvaluateExDataNew->m_astExBuildDataMap.end(); it_build_new++)
            {
                it_build_raw = pstGameEvaluateExDataRaw->m_astExBuildDataMap.find(it_build_new->first);
                if(it_build_raw == pstGameEvaluateExDataRaw->m_astExBuildDataMap.end())
                {
                    // pstGameEvaluateExDataResult->m_astExBuildDataMap[it_build_new->first] = pstGameEvaluateExDataNew->m_astExBuildDataMap[it_build_new->first];
                    pstGameEvaluateExDataResult->m_astExBuildDataMap.insert(make_pair(it_build_new->first, it_build_new->second));
                }
            }
        }
        else
        {         
            // 以raw为标准
            for(it_raw = pstGameEvaluateExDataRaw->m_astExDataMap[dwIdx].begin(); it_raw != pstGameEvaluateExDataRaw->m_astExDataMap[dwIdx].end(); it_raw++)
            {
                it_new = pstGameEvaluateExDataNew->m_astExDataMap[dwIdx].find(it_raw->first);
                if(it_new == pstGameEvaluateExDataNew->m_astExDataMap[dwIdx].end())
                {
                    pstGameEvaluateExDataResult->m_astExDataMap[dwIdx].insert(make_pair(it_raw->first, (0 - it_raw->second)));
                    // pstGameEvaluateExDataResult->m_astExDataMap[dwIdx][it_raw->first] = 0 - pstGameEvaluateExDataRaw->m_astExDataMap[dwIdx][it_raw->first];
                }
                else
                {
                    pstGameEvaluateExDataResult->m_astExDataMap[dwIdx].insert(make_pair(it_new->first, (it_new->second - it_raw->second)));
                    // pstGameEvaluateExDataResult->m_astExDataMap[dwIdx][it_raw->first] = pstGameEvaluateExDataNew->m_astExDataMap[dwIdx][it_raw->first] - pstGameEvaluateExDataRaw->m_astExDataMap[dwIdx][it_raw->first];
                }
            }
            
            // 以new为标准
            for(it_new = pstGameEvaluateExDataNew->m_astExDataMap[dwIdx].begin(); it_new != pstGameEvaluateExDataNew->m_astExDataMap[dwIdx].end(); it_new++)
            {
                it_raw = pstGameEvaluateExDataRaw->m_astExDataMap[dwIdx].find(it_new->first);
                if(it_raw == pstGameEvaluateExDataRaw->m_astExDataMap[dwIdx].end())
                {
                    pstGameEvaluateExDataResult->m_astExDataMap[dwIdx].insert(make_pair(it_new->first, it_new->second));
                    // pstGameEvaluateExDataResult->m_astExDataMap[dwIdx][it_new->first] = pstGameEvaluateExDataNew->m_astExDataMap[dwIdx][it_new->first];
                }
            }
        }
        
    
    }
    return true;
}


string CGameEvaluateLogic::GenGameEvaluateLog(SGameEvaluateData *pstGameEvaluateDataSource, TBOOL bGetGameEvaluateDataSourceFlag, 
                                              SGameEvaluateData *pstGameEvaluateDataTarget, TBOOL bGetGameEvaluateDataTargetFlag, 
                                              SGameEvaluateExData *pstGameEvaluateExDataSourceResult, TBOOL bGetGameEvaluateExDataSourceResultFlag, 
                                              SGameEvaluateExData *pstGameEvaluateExDataTargetResult, TBOOL bGetGameEvaluateExDataTargetResultFlag,
                                              SGameEvaluateAddData *pstGameEvaluateAddData, TBOOL bGameEvaluateType)
{
    Json::Value jGameEvaluateLogRoot = Json::Value(Json::objectValue);

    SGameEvaluateData *pstGameEvaluateData = NULL;
    TBOOL bGetGameEvaluateDataFlag = false;
    SGameEvaluateExData *pstGameEvaluateExDataResult = NULL;
    TBOOL bGetGameEvaluateExDataResultFlag = false;
    string strBaseKeyName = "s_b";
    string strExKeyName = "s_e";

    pstGameEvaluateData = pstGameEvaluateDataSource;
    bGetGameEvaluateDataFlag = bGetGameEvaluateDataSourceFlag;
    pstGameEvaluateExDataResult = pstGameEvaluateExDataSourceResult;
    bGetGameEvaluateExDataResultFlag = bGetGameEvaluateExDataSourceResultFlag;
    
    for(TINT32 dwIdz = 0; dwIdz < 2; ++dwIdz)
    {

        // *********************************** base_data **************************************** //
        if(true == bGetGameEvaluateDataFlag)
        {
            Json::Value jNodeRoot = Json::Value(Json::arrayValue);
            Json::Value jNodeSubRoot = Json::Value(Json::arrayValue);
            map<TINT64, TINT64>::iterator it;
            
            jGameEvaluateLogRoot[strBaseKeyName] = Json::Value(Json::arrayValue);
            // key0(sid)
            jGameEvaluateLogRoot[strBaseKeyName].append(pstGameEvaluateData->m_ddwSid);
            // key1(uid)
            jGameEvaluateLogRoot[strBaseKeyName].append(pstGameEvaluateData->m_ddwUid);
            // key2(idfa)
            jGameEvaluateLogRoot[strBaseKeyName].append(pstGameEvaluateData->m_strIdfa);
            // key3(cmd)
            jGameEvaluateLogRoot[strBaseKeyName].append(pstGameEvaluateData->m_strCmd);
            // key4(curtimestamp)
            jGameEvaluateLogRoot[strBaseKeyName].append(pstGameEvaluateData->m_dwCurTimeStamp);
            // key5(castle_lv)
            jNodeRoot.resize(0);
            jNodeRoot.append(pstGameEvaluateData->m_addwCastleLv[0]);
            jNodeRoot.append(pstGameEvaluateData->m_addwCastleLv[1]);
            jGameEvaluateLogRoot[strBaseKeyName].append(jNodeRoot);
            // key6(hero_lv)
            jGameEvaluateLogRoot[strBaseKeyName].append(pstGameEvaluateData->m_ddwLordLv);
            // key7(force)
            jGameEvaluateLogRoot[strBaseKeyName].append(pstGameEvaluateData->m_ddwForce);
            // key8(build_force)
            jGameEvaluateLogRoot[strBaseKeyName].append(pstGameEvaluateData->m_ddwBuildForce);
            // key9(research_force)
            jGameEvaluateLogRoot[strBaseKeyName].append(pstGameEvaluateData->m_ddwResearchForce);
            // key10(train_force)
            jGameEvaluateLogRoot[strBaseKeyName].append(pstGameEvaluateData->m_ddwTrainForce);
            // key11(hero_force)
            jGameEvaluateLogRoot[strBaseKeyName].append(pstGameEvaluateData->m_ddwHeroForce);
            // key12(equip_force)
            jGameEvaluateLogRoot[strBaseKeyName].append(pstGameEvaluateData->m_ddwEquipForce);
            // key13(res_item_value)
            jNodeRoot.resize(EN_RESOURCE_TYPE__END);
            for(it = pstGameEvaluateData->m_stResItemValueMap.begin(); it != pstGameEvaluateData->m_stResItemValueMap.end();it++)
            {
                jNodeRoot[(TINT32)it->first] = it->second;                 
            }
            jGameEvaluateLogRoot[strBaseKeyName].append(jNodeRoot);
            // key14(time_item_value)
            jNodeRoot.resize(0);
            for(it = pstGameEvaluateData->m_stTimeItemValueMap.begin(); it != pstGameEvaluateData->m_stTimeItemValueMap.end();it++)
            {
                jNodeSubRoot.resize(0);
                jNodeSubRoot.append(it->first);
                jNodeSubRoot.append(it->second);
                jNodeRoot.append(jNodeSubRoot);                    
            }
            jGameEvaluateLogRoot[strBaseKeyName].append(jNodeRoot);            
            // key15(res0)
            // key16(res1)
            // key17(res2)
            // key18(res3) 
            // key19(res4)
            for(TINT32 dwIdx = 0; dwIdx < EN_RESOURCE_TYPE__END; ++dwIdx)
            {
                jGameEvaluateLogRoot[strBaseKeyName].append(pstGameEvaluateData->m_addwRes[dwIdx]);  
            }
            // key20(gem_cost_total)
            jGameEvaluateLogRoot[strBaseKeyName].append(pstGameEvaluateData->m_ddwGemCostTotal);
            // key21(gem_buy_total)
            jGameEvaluateLogRoot[strBaseKeyName].append(pstGameEvaluateData->m_ddwGemBuyTotal);
            // key22(conn_time)
            jGameEvaluateLogRoot[strBaseKeyName].append(pstGameEvaluateData->m_ddwConnTime);
            // key23(packet_size)
            jGameEvaluateLogRoot[strBaseKeyName].append(pstGameEvaluateData->m_ddwPacketSize);
            // key24(network_delay)
            jGameEvaluateLogRoot[strBaseKeyName].append(pstGameEvaluateData->m_ddwNetworkDelay);
            // key25(f1)
            // key26(f2)
            // key27(f3)
            // key28(f4)
            for(TINT32 dwIdx = 0; dwIdx < F_SIZE; ++dwIdx)
            {
                jGameEvaluateLogRoot[strBaseKeyName].append(pstGameEvaluateData->m_addwF[dwIdx]);  
            }            
            // key29(event_seq)
            jGameEvaluateLogRoot[strBaseKeyName].append(pstGameEvaluateData->m_ddwEventSeq);  

            // key30(build_speed_buff)
            jGameEvaluateLogRoot[strBaseKeyName].append(pstGameEvaluateData->m_ddwBuildSpeedBuff);

            // key31(research_speed_buff)
            jGameEvaluateLogRoot[strBaseKeyName].append(pstGameEvaluateData->m_ddwSearchSpeedBuff);

            // key32(troop_train_heal_speed_buff)
            jGameEvaluateLogRoot[strBaseKeyName].append(pstGameEvaluateData->m_ddwTrainTroopSpeedBuff);

            // key33(troop_train_reduce_res_buff)
            jGameEvaluateLogRoot[strBaseKeyName].append(pstGameEvaluateData->m_ddwTrainTroopRedResBuff);

            // key34(fort_train_speed_buff)
            jGameEvaluateLogRoot[strBaseKeyName].append(pstGameEvaluateData->m_ddwTrainFortSpeedBuff);

            // key35(equip_compose_speed_buff)
            jGameEvaluateLogRoot[strBaseKeyName].append(pstGameEvaluateData->m_ddwComposEquipSepeedBuff);

            // key36(equip_compose_reduce_res_buff)
            jGameEvaluateLogRoot[strBaseKeyName].append(pstGameEvaluateData->m_ddwCompEquipRedResBuff);

            // key37(create_time)
            jGameEvaluateLogRoot[strBaseKeyName].append(pstGameEvaluateData->m_ddwCreateTime);

            // key38(platform)
            jGameEvaluateLogRoot[strBaseKeyName].append(pstGameEvaluateData->m_strPlatform);

            // key39(troop_ability)
            jNodeRoot.resize(0);
            for(TUINT32 udwIdx = 0; udwIdx < pstGameEvaluateData->m_stTroopAbilityVector.size(); ++udwIdx)
            {
                jNodeRoot.append(pstGameEvaluateData->m_stTroopAbilityVector[udwIdx]);
            }
            jGameEvaluateLogRoot[strBaseKeyName].append(jNodeRoot);            
            
            // key40(fort_ability)
            jNodeRoot.resize(0);
            for(TUINT32 udwIdx = 0; udwIdx < pstGameEvaluateData->m_stFortAbilityVector.size(); ++udwIdx)
            {
                jNodeRoot.append(pstGameEvaluateData->m_stFortAbilityVector[udwIdx]);
            }
            jGameEvaluateLogRoot[strBaseKeyName].append(jNodeRoot);  

            // key41(own_action_info_no_march)
            jNodeRoot.resize(0);
            for(TUINT32 udwIdx = 0; udwIdx < pstGameEvaluateData->m_stOwnActionInfoNoMatchVector.size(); ++udwIdx)
            {
                jNodeSubRoot.resize(0);
                jNodeSubRoot.append(pstGameEvaluateData->m_stOwnActionInfoNoMatchVector[udwIdx].m_ddwActionId);
                jNodeSubRoot.append(pstGameEvaluateData->m_stOwnActionInfoNoMatchVector[udwIdx].m_ddwSecClass);
                jNodeSubRoot.append(pstGameEvaluateData->m_stOwnActionInfoNoMatchVector[udwIdx].m_ddwOrginCostTime);
                jNodeSubRoot.append(pstGameEvaluateData->m_stOwnActionInfoNoMatchVector[udwIdx].m_ddwRealCostTime);
                jNodeRoot.append(jNodeSubRoot);                    
            }
            jGameEvaluateLogRoot[strBaseKeyName].append(jNodeRoot);  

            // key42(aid)
            jGameEvaluateLogRoot[strBaseKeyName].append(pstGameEvaluateData->m_ddwAid);

            // key43(agift_lv)
            jGameEvaluateLogRoot[strBaseKeyName].append(pstGameEvaluateData->m_ddwAlGiftLv);

            // key44(aforce)
            jGameEvaluateLogRoot[strBaseKeyName].append(pstGameEvaluateData->m_ddwAForce);
            
            // key45(gem_cur)
             jGameEvaluateLogRoot[strBaseKeyName].append(pstGameEvaluateData->m_ddwGemCur);

        }
        else
        {
            jGameEvaluateLogRoot[strBaseKeyName].resize(0);
        }



        // *********************************** ex_data **************************************** //
        // key30(ex_data)
        if(true == bGetGameEvaluateExDataResultFlag)
        {
            map<TINT64, TINT64>::iterator it;
            map<TINT64, SBuildInfo>::iterator it_build;
            jGameEvaluateLogRoot[strExKeyName] = Json::Value(Json::arrayValue);
            Json::Value jNodeRoot = Json::Value(Json::arrayValue);
            for(TINT32 dwIdx = 0; dwIdx < EN_USER_INFO_RECORD_TYPE__END; ++dwIdx)
            {

                if(EN_USER_INFO_RECORD_TYPE__BUILDING == dwIdx)
                {
                    for(it_build = pstGameEvaluateExDataResult->m_astExBuildDataMap.begin(); it_build != pstGameEvaluateExDataResult->m_astExBuildDataMap.end(); it_build++)
                    {
                        jNodeRoot.append(dwIdx);
                        jNodeRoot.append(it_build->second.m_ddwBuildId);
                        jNodeRoot.append(it_build->second.m_ddwBuildLevel);
                        jGameEvaluateLogRoot[strExKeyName].append(jNodeRoot);
                        jNodeRoot.resize(0);
                    }
                }
                else
                {
                    if(0 == pstGameEvaluateExDataResult->m_astExDataMap[dwIdx].size())
                    {
                        continue;
                    }
                
                    for(it = pstGameEvaluateExDataResult->m_astExDataMap[dwIdx].begin(); it != pstGameEvaluateExDataResult->m_astExDataMap[dwIdx].end(); it++)
                    {
                        if(0 == it->second)
                        {
                            continue;
                        }
                        else
                        {   
                            jNodeRoot.append(dwIdx);
                            jNodeRoot.append(it->first);
                            jNodeRoot.append(it->second);
                            jGameEvaluateLogRoot[strExKeyName].append(jNodeRoot);
                            jNodeRoot.resize(0);
                        }
                    }
                }
                

            }
        }
        else
        {
            jGameEvaluateLogRoot[strExKeyName].resize(0);
        }



        if(true == bGameEvaluateType)
        {
            pstGameEvaluateData = pstGameEvaluateDataTarget;
            bGetGameEvaluateDataFlag = bGetGameEvaluateDataTargetFlag;
            pstGameEvaluateExDataResult = pstGameEvaluateExDataTargetResult;
            bGetGameEvaluateExDataResultFlag = bGetGameEvaluateExDataTargetResultFlag;
            strBaseKeyName = "t_b";
            strExKeyName = "t_e";
        }
        else
        {
            jGameEvaluateLogRoot["t_b"].resize(0);
            jGameEvaluateLogRoot["t_e"].resize(0);
            break;
        }   

    }

    // *********************************** add_data **************************************** //
    map<TINT64, string>::iterator it;
    if(0 != pstGameEvaluateAddData->m_astAddDataMap.size())
    {
        Json::Value jNodeRoot = Json::Value(Json::arrayValue);
        for(it = pstGameEvaluateAddData->m_astAddDataMap.begin(); it != pstGameEvaluateAddData->m_astAddDataMap.end();it++)
        {

           jNodeRoot.append(it->first);
           jNodeRoot.append(it->second);
           jGameEvaluateLogRoot["add"].append(jNodeRoot);
           jNodeRoot.resize(0);

        }
    }
    else
    {
        jGameEvaluateLogRoot["add"].resize(0);
    }

    

    // return ossGameEvaluateLogOut.str();
    Json::FastWriter write;
    string strGameEvaluateLog = write.write(jGameEvaluateLogRoot);
    if(0 < strGameEvaluateLog.size())
    {        
        strGameEvaluateLog.replace(strGameEvaluateLog.length() - 1, strGameEvaluateLog.length(), "\0");
    }
    return strGameEvaluateLog;

}



TBOOL CGameEvaluateLogic::SaveGameEvaluateExData(SReqInfo *pstReqInfo, SUserInfo *pstUserInfo, TINT64 ddwExDataUserType, TINT64 ddwExDataType)
{
    SGameEvaluateExData *pstGameEvaluateExData = NULL;

    if(EN_EX_DATA_USER_TYPE_SOURCE > ddwExDataUserType \
       && EN_EX_DATA_USER_TYPE_TARGET < ddwExDataUserType)
    {
        return false;
    }
    SCityInfo *pstCity = &pstUserInfo->m_stCityInfo;
    if(NULL == pstCity)
    {
        return false;
    }

    if(EN_EX_DATA_TYPE_RAW == ddwExDataType)
    {
        pstGameEvaluateExData = &pstUserInfo->m_astGameEvaluateExDataRaw[ddwExDataUserType];
    }
    else if(EN_EX_DATA_TYPE_NEW == ddwExDataType)
    {
        pstGameEvaluateExData = &pstUserInfo->m_astGameEvaluateExDataNew[ddwExDataUserType];
    }
    else
    {
        return false;
    }

    pstGameEvaluateExData->Reset();

    TINT64 ddwExDataId = 0;
    TINT64 ddwExDataNum = 0;    
    Json::Value::Members vecMembers;
    TINT64 addwTroop[EN_TROOP_TYPE__END];
    memset((TCHAR*)addwTroop, 0, sizeof(addwTroop));

    
    for(TINT32 dwIdx = 0; dwIdx < EN_USER_INFO_RECORD_TYPE__END; ++dwIdx)
    {
        switch (dwIdx)
        {
            case EN_USER_INFO_RECORD_TYPE__GEM:            
                ddwExDataId = 0;
                ddwExDataNum = pstUserInfo->m_tbLogin.Get_Gem();
                // pstGameEvaluateExData->m_astExDataMap[dwIdx][ddwExDataId] = ddwExDataNum;
                pstGameEvaluateExData->m_astExDataMap[dwIdx].insert(make_pair(ddwExDataId, ddwExDataNum));
                break;
            case EN_USER_INFO_RECORD_TYPE__ITEM:
                for(TUINT32 udwIdy = 0; udwIdy < pstUserInfo->m_tbBackpack.m_bItem.m_udwNum; ++udwIdy)
                {
                    ddwExDataId = pstUserInfo->m_tbBackpack.m_bItem[udwIdy].m_ddwItemId;
                    ddwExDataNum = pstUserInfo->m_tbBackpack.m_bItem[udwIdy].m_ddwItemNum;
                    pstGameEvaluateExData->m_astExDataMap[dwIdx].insert(make_pair(ddwExDataId, ddwExDataNum));
                    // pstGameEvaluateExData->m_astExDataMap[dwIdx][ddwExDataId] = ddwExDataNum;
                }
                break;
            case EN_USER_INFO_RECORD_TYPE__RESOURCE:
                for(TINT32 dwIdy = 0; dwIdy < EN_RESOURCE_TYPE__END; ++dwIdy)
                {                    
                    ddwExDataId = dwIdy;
                    ddwExDataNum = pstCity->m_stTblData.m_bResource[0].m_addwNum[dwIdy];
                    // pstGameEvaluateExData->m_astExDataMap[dwIdx][ddwExDataId] = ddwExDataNum;
                    pstGameEvaluateExData->m_astExDataMap[dwIdx].insert(make_pair(ddwExDataId, ddwExDataNum));
                }
                break;
            case EN_USER_INFO_RECORD_TYPE__BUILDING:
                for(TUINT32 udwIdy = 0; udwIdy < pstCity->m_stTblData.m_bBuilding.m_udwNum; ++udwIdy)
                {                  
                    SBuildInfo stBuildInfo;
                    stBuildInfo.Reset();
                    stBuildInfo.SetValue(pstCity->m_stTblData.m_bBuilding[udwIdy].m_ddwType, pstCity->m_stTblData.m_bBuilding[udwIdy].m_ddwLevel);
                    // pstGameEvaluateExData->m_astExBuildDataMap[pstCity->m_stTblData.m_bBuilding[udwIdy].m_udwPos] = stBuildInfo;
                    pstGameEvaluateExData->m_astExBuildDataMap.insert(make_pair(pstCity->m_stTblData.m_bBuilding[udwIdy].m_ddwPos, stBuildInfo));
                }
                break;
            case EN_USER_INFO_RECORD_TYPE__RESEARCH:
                for(TINT32 dwIdy = 0; dwIdy < EN_RESEARCH_TYPE__LIMIT; ++dwIdy)
                {
                    ddwExDataId = dwIdy;
                    ddwExDataNum = pstCity->m_stTblData.m_bResearch[0].m_addwLevel[dwIdy];
                    if(ddwExDataNum > 0)
                    {
                        // pstGameEvaluateExData->m_astExDataMap[dwIdx][ddwExDataId] = ddwExDataNum;
                        pstGameEvaluateExData->m_astExDataMap[dwIdx].insert(make_pair(ddwExDataId, ddwExDataNum));
                    }                    
                }
                break;
            case EN_USER_INFO_RECORD_TYPE__TROOP:
                CCommonBase::ComputeTotalTroop(pstUserInfo, addwTroop);
                for(TINT32 dwIdy = 0; dwIdy < EN_TROOP_TYPE__END; ++dwIdy)
                {                    
                    ddwExDataId = dwIdy;
                    ddwExDataNum = addwTroop[dwIdy];
                    // pstGameEvaluateExData->m_astExDataMap[dwIdx][ddwExDataId] = ddwExDataNum;
                    pstGameEvaluateExData->m_astExDataMap[dwIdx].insert(make_pair(ddwExDataId, ddwExDataNum));
                }
                break;
            case EN_USER_INFO_RECORD_TYPE__FORT:
                for(TINT32 dwIdy = 0; dwIdy < EN_FORT_TYPE__END; ++dwIdy)
                {                    
                    ddwExDataId = dwIdy;
                    ddwExDataNum = pstCity->m_stTblData.m_bFort[0].m_addwNum[dwIdy];
                    // pstGameEvaluateExData->m_astExDataMap[dwIdx][ddwExDataId] = ddwExDataNum;
                    pstGameEvaluateExData->m_astExDataMap[dwIdx].insert(make_pair(ddwExDataId, ddwExDataNum));
                }
                break;
            case EN_USER_INFO_RECORD_TYPE__MATERIAL:
                vecMembers = pstUserInfo->m_tbBackpack.m_jMaterial.getMemberNames();
	            for(TUINT32 udwIdy = 0; udwIdy < vecMembers.size(); ++udwIdy)
                {
                    ddwExDataId = atoi(vecMembers[udwIdy].c_str());;
                    ddwExDataNum = pstUserInfo->m_tbBackpack.m_jMaterial[vecMembers[udwIdy]].asInt();
                    // pstGameEvaluateExData->m_astExDataMap[dwIdx][ddwExDataId] = ddwExDataNum;
                    pstGameEvaluateExData->m_astExDataMap[dwIdx].insert(make_pair(ddwExDataId, ddwExDataNum));
                }
                break;
            case EN_USER_INFO_RECORD_TYPE__CRYSTAL:
                vecMembers = pstUserInfo->m_tbBackpack.m_jCrystal.getMemberNames();
	            for(TUINT32 udwIdy = 0; udwIdy < vecMembers.size(); ++udwIdy)
                {
                    ddwExDataId = atoi(vecMembers[udwIdy].c_str());;
                    ddwExDataNum = pstUserInfo->m_tbBackpack.m_jCrystal[vecMembers[udwIdy]].asInt();
                    // pstGameEvaluateExData->m_astExDataMap[dwIdx][ddwExDataId] = ddwExDataNum;
                    pstGameEvaluateExData->m_astExDataMap[dwIdx].insert(make_pair(ddwExDataId, ddwExDataNum));
                }
                break;
            case EN_USER_INFO_RECORD_TYPE__SP_CRYSTAL:
                vecMembers = pstUserInfo->m_tbBackpack.m_jSp_crystal.getMemberNames();
	            for(TUINT32 udwIdy = 0; udwIdy < vecMembers.size(); ++udwIdy)
                {
                    ddwExDataId = atoi(vecMembers[udwIdy].c_str());;
                    ddwExDataNum = pstUserInfo->m_tbBackpack.m_jSp_crystal[vecMembers[udwIdy]].asInt();
                    // pstGameEvaluateExData->m_astExDataMap[dwIdx][ddwExDataId] = ddwExDataNum;
                    pstGameEvaluateExData->m_astExDataMap[dwIdx].insert(make_pair(ddwExDataId, ddwExDataNum));
                }
                break;
            case EN_USER_INFO_RECORD_TYPE__SOUL:
                vecMembers = pstUserInfo->m_tbBackpack.m_jSoul.getMemberNames();
	            for(TUINT32 udwIdy = 0; udwIdy < vecMembers.size(); ++udwIdy)
                {
                    ddwExDataId = atoi(vecMembers[udwIdy].c_str());;
                    ddwExDataNum = pstUserInfo->m_tbBackpack.m_jSoul[vecMembers[udwIdy]].asInt();
                    // pstGameEvaluateExData->m_astExDataMap[dwIdx][ddwExDataId] = ddwExDataNum;
                    pstGameEvaluateExData->m_astExDataMap[dwIdx].insert(make_pair(ddwExDataId, ddwExDataNum));
                }
                break;
            case EN_USER_INFO_RECORD_TYPE__PATRS:
                vecMembers = pstUserInfo->m_tbBackpack.m_jParts.getMemberNames();
	            for(TUINT32 udwIdy = 0; udwIdy < vecMembers.size(); ++udwIdy)
                {
                    ddwExDataId = atoi(vecMembers[udwIdy].c_str());;
                    ddwExDataNum = pstUserInfo->m_tbBackpack.m_jParts[vecMembers[udwIdy]].asInt();
                    // pstGameEvaluateExData->m_astExDataMap[dwIdx][ddwExDataId] = ddwExDataNum;
                    pstGameEvaluateExData->m_astExDataMap[dwIdx].insert(make_pair(ddwExDataId, ddwExDataNum));
                }
                break;
            case EN_USER_INFO_RECORD_TYPE__EQUIP:
                for(TUINT32 udwIdy = 0; udwIdy < pstUserInfo->m_udwEquipNum; ++udwIdy)
                {
                    if(EN_TABLE_UPDT_FLAG__DEL == pstUserInfo->m_aucEquipFlag[udwIdy]
                        || 0 == pstUserInfo->m_atbEquip[udwIdy].m_nId)
                    {
                        continue;
                    }
                    ddwExDataId = pstUserInfo->m_atbEquip[udwIdy].m_nEquip_type;
                    ddwExDataNum = 1;
                    // pstGameEvaluateExData->m_astExDataMap[dwIdx][ddwExDataId] = ddwExDataNum;
                    pstGameEvaluateExData->m_astExDataMap[dwIdx].insert(make_pair(ddwExDataId, ddwExDataNum));
                }
                break;
            case EN_USER_INFO_RECORD_TYPE__MIGHT:
                ddwExDataId = 0;
                ddwExDataNum = pstUserInfo->m_tbPlayer.Get_Might();
                // pstGameEvaluateExData->m_astExDataMap[dwIdx][ddwExDataId] = ddwExDataNum;
                pstGameEvaluateExData->m_astExDataMap[dwIdx].insert(make_pair(ddwExDataId, ddwExDataNum));
                break;
            case EN_USER_INFO_RECORD_TYPE__HOSPITAL_TROOP:
                for(TINT32 dwIdy = 0; dwIdy < EN_TROOP_TYPE__END; ++dwIdy)
                {                    
                    ddwExDataId = dwIdy;
                    ddwExDataNum = pstCity->m_stTblData.m_bHos_wait[0].m_addwNum[dwIdy];
                    // pstGameEvaluateExData->m_astExDataMap[dwIdx][ddwExDataId] = ddwExDataNum;
                    pstGameEvaluateExData->m_astExDataMap[dwIdx].insert(make_pair(ddwExDataId, ddwExDataNum));
                }
                break;
            case EN_USER_INFO_RECORD_TYPE__ALTER_TROOP:
                break;
            case EN_USER_INFO_RECORD_TYPE__VIP_POINT:
                ddwExDataId = 0;
                ddwExDataNum = pstUserInfo->m_tbPlayer.Get_Vip_point();
                // pstGameEvaluateExData->m_astExDataMap[dwIdx][ddwExDataId] = ddwExDataNum;
                pstGameEvaluateExData->m_astExDataMap[dwIdx].insert(make_pair(ddwExDataId, ddwExDataNum));
                break;
            case EN_USER_INFO_RECORD_TYPE__LORD_EXP:
                ddwExDataId = 0;
                ddwExDataNum = pstUserInfo->m_tbPlayer.m_nExp;
                // pstGameEvaluateExData->m_astExDataMap[dwIdx][ddwExDataId] = ddwExDataNum;
                pstGameEvaluateExData->m_astExDataMap[dwIdx].insert(make_pair(ddwExDataId, ddwExDataNum));
                break;
            case EN_USER_INFO_RECORD_TYPE__TIME_ITEM_VALUE:
                ddwExDataId = 0;
                ddwExDataNum = CGameEvaluateLogic::GetTotalTimeItemValue(&pstUserInfo->m_tbBackpack);
                // pstGameEvaluateExData->m_astExDataMap[dwIdx][ddwExDataId] = ddwExDataNum;
                pstGameEvaluateExData->m_astExDataMap[dwIdx].insert(make_pair(ddwExDataId, ddwExDataNum));
                break;
            case EN_USER_INFO_RECORD_TYPE__BUILD_FORCE:
                ddwExDataId = 0;
                ddwExDataNum = pstUserInfo->m_tbPlayer.Get_Building_force();
                // pstGameEvaluateExData->m_astExDataMap[dwIdx][ddwExDataId] = ddwExDataNum;
                pstGameEvaluateExData->m_astExDataMap[dwIdx].insert(make_pair(ddwExDataId, ddwExDataNum));
                break;
            case EN_USER_INFO_RECORD_TYPE__RESEARCH_FORCE:
                ddwExDataId = 0;
                ddwExDataNum = pstUserInfo->m_tbPlayer.Get_Research_force();
                // pstGameEvaluateExData->m_astExDataMap[dwIdx][ddwExDataId] = ddwExDataNum;
                pstGameEvaluateExData->m_astExDataMap[dwIdx].insert(make_pair(ddwExDataId, ddwExDataNum));
                break;
            case EN_USER_INFO_RECORD_TYPE__TRAIN_FORCE:
                ddwExDataId = 0;
                ddwExDataNum = pstUserInfo->m_tbPlayer.Get_Mgain();
                // pstGameEvaluateExData->m_astExDataMap[dwIdx][ddwExDataId] = ddwExDataNum;
                pstGameEvaluateExData->m_astExDataMap[dwIdx].insert(make_pair(ddwExDataId, ddwExDataNum));
                break;
            case EN_USER_INFO_RECORD_TYPE__HERO_FORCE:
                ddwExDataId = 0;
                ddwExDataNum = pstUserInfo->m_tbPlayer.Get_Dragon_force();
                // pstGameEvaluateExData->m_astExDataMap[dwIdx][ddwExDataId] = ddwExDataNum;
                pstGameEvaluateExData->m_astExDataMap[dwIdx].insert(make_pair(ddwExDataId, ddwExDataNum));
                break;
            default:          
                TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("CGameEvaluateLogic::SaveGameEvaluateExData [type=%ld] [seq=%u]", \
                                                                  dwIdx, pstUserInfo->m_udwBSeqNo));
                break;
        }
    }
    
    return true;
}


TVOID CGameEvaluateLogic::GetOwnActionIdSecClassNoMarch(vector<SActionInfo> &m_stOwnActionInfoNoMatchVector, SUserInfo *pstUserInfo)
{
    ostringstream ossContent;
    ossContent.str("");
    SActionInfo stActionInfo;
    stActionInfo.Reset();

    for(TUINT32 udwIdx = 0; udwIdx < pstUserInfo->m_udwActionNum; ++udwIdx)
    {
        if(EN_ACTION_MAIN_CLASS__MARCH == pstUserInfo->m_atbAction[udwIdx].Get_Mclass()
           || EN_ACTION_CLASS_TASK_NOTIC == pstUserInfo->m_atbAction[udwIdx].Get_Mclass()
           || EN_ACTION_MAIN_CLASS__TIMER == pstUserInfo->m_atbAction[udwIdx].Get_Mclass()
           || EN_ACTION_MAIN_CLASS__AU_PRESSURE_MEASURE == pstUserInfo->m_atbAction[udwIdx].Get_Mclass()
           || EN_TABLE_UPDT_FLAG__DEL != pstUserInfo->m_aucActionFlag[udwIdx])
        {
            continue;
        }
        
        stActionInfo.Reset();
        stActionInfo.SetValue(pstUserInfo->m_atbAction[udwIdx].Get_Id(), pstUserInfo->m_atbAction[udwIdx].Get_Sclass(), \
                              pstUserInfo->m_atbAction[udwIdx].Get_Ctime(), pstUserInfo->m_atbAction[udwIdx].Get_Etime() - pstUserInfo->m_atbAction[udwIdx].Get_Btime());
        m_stOwnActionInfoNoMatchVector.push_back(stActionInfo);

    }

    for(TUINT32 udwIdx = 0; udwIdx < pstUserInfo->m_udwMarchNum; ++udwIdx)
    {
        if(EN_ACTION_MAIN_CLASS__MARCH == pstUserInfo->m_atbMarch[udwIdx].Get_Mclass()
           || EN_ACTION_CLASS_TASK_NOTIC == pstUserInfo->m_atbMarch[udwIdx].Get_Mclass()
           || EN_ACTION_MAIN_CLASS__TIMER == pstUserInfo->m_atbMarch[udwIdx].Get_Mclass()
           || EN_ACTION_MAIN_CLASS__AU_PRESSURE_MEASURE == pstUserInfo->m_atbMarch[udwIdx].Get_Mclass()
           || EN_TABLE_UPDT_FLAG__DEL != pstUserInfo->m_aucMarchFlag[udwIdx])
        {
            continue;
        }
        stActionInfo.Reset();
        stActionInfo.SetValue(pstUserInfo->m_atbMarch[udwIdx].Get_Id(), pstUserInfo->m_atbMarch[udwIdx].Get_Sclass(), \
                              pstUserInfo->m_atbMarch[udwIdx].Get_Ctime(), pstUserInfo->m_atbMarch[udwIdx].Get_Etime() - pstUserInfo->m_atbMarch[udwIdx].Get_Btime());
        m_stOwnActionInfoNoMatchVector.push_back(stActionInfo);

    }

    
    for(TUINT32 udwIdx = 0; udwIdx < pstUserInfo->m_udwSelfAlActionNum; ++udwIdx)
    {
        if(EN_ACTION_MAIN_CLASS__MARCH == pstUserInfo->m_atbSelfAlAction[udwIdx].Get_Mclass()
           || EN_ACTION_CLASS_TASK_NOTIC == pstUserInfo->m_atbSelfAlAction[udwIdx].Get_Mclass()
           || EN_ACTION_MAIN_CLASS__TIMER == pstUserInfo->m_atbSelfAlAction[udwIdx].Get_Mclass()
           || EN_ACTION_MAIN_CLASS__AU_PRESSURE_MEASURE == pstUserInfo->m_atbSelfAlAction[udwIdx].Get_Mclass()
           || EN_TABLE_UPDT_FLAG__DEL != pstUserInfo->m_aucSelfAlActionFlag[udwIdx])
        {
            continue;
        }
        stActionInfo.Reset();
        stActionInfo.SetValue(pstUserInfo->m_atbSelfAlAction[udwIdx].Get_Id(), pstUserInfo->m_atbSelfAlAction[udwIdx].Get_Sclass(), \
                              pstUserInfo->m_atbSelfAlAction[udwIdx].Get_Ctime(), pstUserInfo->m_atbSelfAlAction[udwIdx].Get_Etime() - pstUserInfo->m_atbSelfAlAction[udwIdx].Get_Btime());
        m_stOwnActionInfoNoMatchVector.push_back(stActionInfo);

    }
    
}




/************************************************** private **************************************************/



// function ==> 获取玩家训练troop的能力
TVOID CGameEvaluateLogic::GetTroopAbility(vector<TINT64> &stTroopAbilityVector, SUserInfo *pstUserInfo)
{
    TINT64 addwTroop[EN_TROOP_TYPE__END];    
    CCommonBase::ComputeTotalTroop(pstUserInfo, addwTroop);
    for(TINT32 dwIdx = 0; dwIdx < EN_TROOP_TYPE__END; ++dwIdx)
    {
        if(0 != addwTroop[dwIdx])
        {
            stTroopAbilityVector.push_back(dwIdx);
        }
    }
}

// function ==> 获取玩家训练fort的能力
TVOID CGameEvaluateLogic::GetFortAbility(vector<TINT64> &stFortAbilityVector, SUserInfo *pstUserInfo)
{
    for(TINT32 dwIdx = 0; dwIdx < EN_FORT_TYPE__END; ++dwIdx)
    {
        if(0 != pstUserInfo->m_stCityInfo.m_stTblData.m_bFort[0].m_addwNum[dwIdx])
        {
            stFortAbilityVector.push_back(dwIdx);
        }
    }
}




TINT64 CGameEvaluateLogic::GetTotalTimeItemValue(TbBackpack *ptbBackpack)
{   
    CGameInfo *poGameInfo = CGameInfo::GetInstance();
    SSpGlobalRes stEffectInfo;
    TUINT32 udwItemId = 0;
    TUINT32 udwItemNum = 0;
    string strItemId = "";
    TINT64 ddwTotalTimeItemValue = 0;
    
    for(TUINT32 udwIdx = 0; udwIdx < ptbBackpack->m_bItem.m_udwNum; ++udwIdx)
    {
        udwItemId = ptbBackpack->m_bItem[udwIdx].m_ddwItemId;
        udwItemNum = ptbBackpack->m_bItem[udwIdx].m_ddwItemNum;
        strItemId = CCommonFunc::NumToString(udwItemId);
        if(poGameInfo->m_oJsonRoot["game_item"].isMember(strItemId))
        {
            stEffectInfo.Reset();
            CItemLogic::GetItemreward(udwItemId, &stEffectInfo);
            for(TUINT32 udwIdy = 0; udwIdy < stEffectInfo.udwTotalNum; ++udwIdy)
            {
                if(EN_GLOBALRES_TYPE_SPEEDUP_TIME == stEffectInfo.aRewardList[udwIdy].udwType)
                {
                    ddwTotalTimeItemValue += udwItemNum * stEffectInfo.aRewardList[udwIdy].udwNum;
                }
            }
        }
    }
    return ddwTotalTimeItemValue;
}





TVOID CGameEvaluateLogic::GetItemInfo(map<TINT64, TINT64> &ItemInfoMap, TbBackpack *ptbBackpack, TINT64 ddwItemType)
{
    CGameInfo *poGameInfo = CGameInfo::GetInstance();
    map<TINT64, TINT64>::iterator it;
    SSpGlobalRes stEffectInfo;
    TUINT32 udwItemId = 0;
    TUINT32 udwItemNum = 0;
    string strItemId = "";
    ostringstream ossContent;

    if(EN_GLOBALRES_TYPE_RESOURCE == ddwItemType)
    {
        for(TINT32 dwIdx = 0; dwIdx < EN_RESOURCE_TYPE__END; ++dwIdx)
        {
            ItemInfoMap[dwIdx] = 0;
        }
    }
    

    for(TUINT32 udwIdx = 0; udwIdx < ptbBackpack->m_bItem.m_udwNum; ++udwIdx)
    {
        udwItemId = ptbBackpack->m_bItem[udwIdx].m_ddwItemId;
        udwItemNum = ptbBackpack->m_bItem[udwIdx].m_ddwItemNum;
        strItemId = CCommonFunc::NumToString(udwItemId);
        if(poGameInfo->m_oJsonRoot["game_item"].isMember(strItemId))
        {
            stEffectInfo.Reset();
            CItemLogic::GetItemreward(udwItemId, &stEffectInfo);
            for(TUINT32 udwIdy = 0; udwIdy < stEffectInfo.udwTotalNum; ++udwIdy)
            {
                if(EN_GLOBALRES_TYPE_SPEEDUP_TIME == ddwItemType)
                {
                    if(ddwItemType == stEffectInfo.aRewardList[udwIdy].udwType)
                    {
                        it = ItemInfoMap.find(udwItemId);
                        if(it != ItemInfoMap.end())
                        {
                            ItemInfoMap[udwItemId] += udwItemNum * stEffectInfo.aRewardList[udwIdy].udwNum;
                        }
                        else
                        {
                            ItemInfoMap[udwItemId] = udwItemNum * stEffectInfo.aRewardList[udwIdy].udwNum;
                        }
                    }
                }
                if(EN_GLOBALRES_TYPE_RESOURCE == ddwItemType)
                {
                    if(ddwItemType == stEffectInfo.aRewardList[udwIdy].udwType)
                    {
                        it = ItemInfoMap.find(stEffectInfo.aRewardList[udwIdy].udwId);
                        if(it != ItemInfoMap.end())
                        {
                            ItemInfoMap[stEffectInfo.aRewardList[udwIdy].udwId] += udwItemNum * stEffectInfo.aRewardList[udwIdy].udwNum;
                        }
                        else
                        {
                            ItemInfoMap[stEffectInfo.aRewardList[udwIdy].udwId] = udwItemNum * stEffectInfo.aRewardList[udwIdy].udwNum;
                        }
                    }
                }
            }
        }
    }
   
}


template<class T>
TVOID CGameEvaluateLogic::SetGameEvaluateLogKey(ostringstream &oss, const T &tVariable)
{
    oss << "\t" << tVariable;
}


template<class T1, class T2>
TVOID CGameEvaluateLogic::SetGameEvaluateSubLogKey(ostringstream &oss, const T1 &tVariable0, const T2 &tVariable1)
{
    oss << "|" << tVariable0 << ":" << tVariable1;
}

template<class T1, class T2, class T3>
TVOID CGameEvaluateLogic::SetGameEvaluateSubLogKey(ostringstream &oss, const T1 &tVariable0, const T2 &tVariable1, const T3 &tVariable2)
{
    oss << "|" << tVariable0 << ":" << tVariable1 << ":" << tVariable2;
}



