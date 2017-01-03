#include "hu_cblog.h"
#include "city_base.h"
#include "player_base.h"
#include "global_serv.h"
#include "action_base.h"
#include "dc_log.h"

TVOID CHuCBLog::OutOperateLogCbLog(SSession *pstSession, string strCommand /*= ""*/, string strKey /*= ""*/)
{
    //TODO 需要重新整理
    if(EN_RET_CODE__SUCCESS != pstSession->m_stCommonResInfo.m_dwRetCode)
    {
        return;
    }

    SReqParam *pstReqParam = &pstSession->m_stReqParam;
    
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    ostringstream ossCbLogOut;
    ostringstream ossCbLogkeyContent;
    ossCbLogOut.str("");
    ossCbLogkeyContent.str("");

    // ======================================================
    // key2(version)
    SetCbLogKey(ossCbLogOut, pstReqParam->m_udwVersion);

    // ======================================================
    // key3(timestamp)
    SetCbLogKey(ossCbLogOut, udwCurTime);

    // ======================================================
    // key4(device_id)
    SetCbLogKey(ossCbLogOut, pstReqParam->m_szDevice);

    // ======================================================
    // key5(command)
    string strCurCommand = "";
    if("" != strCommand)
    {
        strCurCommand = strCommand;
    }
    else
    {
        strCurCommand = pstReqParam->m_szCommand;
    }
    SetCbLogKey(ossCbLogOut, strCurCommand);

    // ======================================================
    // key6(uid)
    TUINT32 udwUid = pstReqParam->m_udwUserId;
    SetCbLogKey(ossCbLogOut, udwUid);

    // ======================================================
    // key7(sid)
    TINT32 dwSid = (TINT32)pstReqParam->m_udwSvrId;
    SetCbLogKey(ossCbLogOut, dwSid);

    // ======================================================
    // key8(cid)
    TINT64 ddwCityPos = 0;
    SetCbLogKey(ossCbLogOut, ddwCityPos);

    // ======================================================
    // key9(keys)
    // "key0|key1|key2|key3|key4|key5|..."
    ossCbLogkeyContent.str("");
    if("" != strKey)
    {
    ossCbLogkeyContent << strKey;
    }
    else
    {
        for(TINT32 dwIdx = 0; dwIdx < MAX_REQ_PARAM_KEY_NUM; ++dwIdx)
        {
            if(0 == dwIdx)
            {
                ossCbLogkeyContent << pstReqParam->m_szKey[dwIdx];
            }
            else
            {
                ossCbLogkeyContent << "|" << pstReqParam->m_szKey[dwIdx];
            }
        }
    }
    SetCbLogKey(ossCbLogOut, ossCbLogkeyContent.str().c_str());
    
    // output
    TSE_LOG_HOUR(CGlobalServ::m_poFormatLog, ("%s", ossCbLogOut.str().c_str()));

}



TVOID CHuCBLog::OutLogCbLog(SSession *pstSession, string strCommand /*= ""*/, string strKey /*= ""*/)
{
    if(EN_RET_CODE__SUCCESS != pstSession->m_stCommonResInfo.m_dwRetCode)
    {
        return;
    }

    SReqParam *pstReqParam = &pstSession->m_stReqParam;
    SUserInfo *pstUser = &pstSession->m_stUserInfo;

    TbLogin *pstLogin = &pstUser->m_tbLogin;
    TbPlayer *ptbPlayer = &pstUser->m_tbPlayer;
    SCityInfo *pstCityinfo = &pstUser->m_stCityInfo;
    TbCity *ptbCity = NULL;
    if (pstCityinfo)
    {
        ptbCity = &pstCityinfo->m_stTblData;
    }
    else
    {
        return;
    }
    //SPlayerMightInfo *pstPlayerMight = &pstSession->m_stUserInfo.m_stPlayerMight;
    TbUser_stat* ptbStat = &pstUser->m_tbUserStat;

    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    ostringstream ossCbLogOut;
    ostringstream ossCbLogkeyContent;
    ossCbLogOut.str("");
    ossCbLogkeyContent.str("");

    // 适配
    if((0 == strcmp(pstSession->m_stReqParam.m_szCommand, "gem_recharge"))
        || (0 == strcmp(pstSession->m_stReqParam.m_szCommand, "operate_gem_recharge")))
    {
        if(pstSession->m_ucFakeRecharge)
        {
            strcpy(pstSession->m_stReqParam.m_szKey[6], "1");
        }
    }

    TBOOL bHead = TRUE;

    // ======================================================
    // key2(version)
    SetCbLogKey(ossCbLogOut, pstReqParam->m_udwVersion);

    // ======================================================
    // key3(timestamp)
    SetCbLogKey(ossCbLogOut, udwCurTime);

    // ======================================================
    // key4(device_id)
    SetCbLogKey(ossCbLogOut, pstReqParam->m_szDevice);

    // ======================================================
    // key5(command)
    string strCurCommand = "";
    if("" != strCommand)
    {
        strCurCommand = strCommand;
    }
    else
    {
        strCurCommand = pstReqParam->m_szCommand;
    }
    SetCbLogKey(ossCbLogOut, strCurCommand);

    // ======================================================
    // key6(uid)
    TUINT32 udwUid = pstReqParam->m_udwUserId;
    if(0 == udwUid)
    {
        udwUid = ptbPlayer->m_nUid;
    }
    SetCbLogKey(ossCbLogOut, udwUid);

    // ======================================================
    // key7(sid)
    TINT32 dwSid = (TINT32)pstReqParam->m_udwSvrId;
    SetCbLogKey(ossCbLogOut, dwSid);

    // ======================================================
    // key8(cid)
    TINT64 ddwCityPos = ptbCity->Get_Pos();
    SetCbLogKey(ossCbLogOut, ddwCityPos);

    // special command
    if(EN_CLIENT_REQ_COMMAND__ITEM_USE == pstSession->m_stReqParam.m_udwCommandID ||
       EN_CLIENT_REQ_COMMAND__ITEM_BUY_AND_USE == pstSession->m_stReqParam.m_udwCommandID)
    {
        snprintf(pstSession->m_stReqParam.m_szKey[3], DEFAULT_PARAM_STR_LEN, "%u",
            pstSession->m_stUserInfo.m_udwLotteryChestItemId);
    }

    // ======================================================
    // key9(keys)
    // "key0|key1|key2|key3|key4|key5|..."
    ossCbLogkeyContent.str("");
    if("" != strKey)
    {
        ossCbLogkeyContent << strKey;
    }
    else
    {
        for(TINT32 dwIdx = 0; dwIdx < MAX_REQ_PARAM_KEY_NUM; ++dwIdx)
        {
            // mail_send:key3(content)及以后直接丢弃，内容太大
            if(EN_CLIENT_REQ_COMMAND__MAIL_SEND == pstSession->m_stReqParam.m_udwCommandID
            || EN_CLIENT_REQ_COMMAND__MAIL_SEND_BY_ID == pstSession->m_stReqParam.m_udwCommandID
            || EN_CLIENT_OPERATE_COMMAND__MAIL_SEND == pstSession->m_stReqParam.m_udwCommandID)
            {
                if(3 <= dwIdx)
                {
                    continue;
                }
            }
            if(0 == strcmp(pstSession->m_stReqParam.m_szCommand, "mail_send")
                && 3 <= dwIdx)
            {
                continue;
            }

            if(4 == dwIdx
                && 0 == strcmp(pstSession->m_stReqParam.m_szCommand, "guide_finish")
                && EN_RET_CODE__SUCCESS == pstSession->m_stCommonResInfo.m_dwRetCode)
            {
                ossCbLogkeyContent << pstLogin->Get_Uid();
            }
            else if(0 == dwIdx)
            {
                ossCbLogkeyContent << pstReqParam->m_szKey[dwIdx];
            }
            else
            {
                ossCbLogkeyContent << "|" << pstReqParam->m_szKey[dwIdx];
            }
        }
    }
    SetCbLogKey(ossCbLogOut, ossCbLogkeyContent.str().c_str());

    // ======================================================
    // key10(seqno)
    SetCbLogKey(ossCbLogOut, pstSession->m_udwSeqNo);

    // ======================================================
    // key11(ret_code)
    SetCbLogKey(ossCbLogOut, pstSession->m_stCommonResInfo.m_dwRetCode);

    // ======================================================
    // key12(timecost)
    SetCbLogKey(ossCbLogOut, pstSession->m_stCommonResInfo.m_uddwCostTime);

    // ======================================================
    // key13(gid)
    SetCbLogKey(ossCbLogOut, pstReqParam->m_szGameCenterID);

    // ======================================================
    // key14(aid)
    SetCbLogKey(ossCbLogOut, pstReqParam->m_udwAllianceId);

    // ======================================================
    // key15(flag(hu/au))
    SetCbLogKey(ossCbLogOut, 1U);

    // ======================================================
    // key16(options)
    // "new_player|npc|guide|loytal_add|tid"
    ossCbLogkeyContent.str("");
    ossCbLogkeyContent << (TUINT32)pstUser->m_ucPlayerFlag << "|";
    ossCbLogkeyContent << pstLogin->Get_Npc() << "|";
    ossCbLogkeyContent << 0 << "|";
    ossCbLogkeyContent << pstSession->m_udwLoytalAdd << "|";
    ossCbLogkeyContent << pstUser->m_uddwNewActionId;
    SetCbLogKey(ossCbLogOut, ossCbLogkeyContent.str().c_str());

    // ======================================================
    // key17(level)
    SetCbLogKey(ossCbLogOut, ptbPlayer->m_nLevel);

    // ======================================================
    // key18(force)
    // "total_force|build_force|research_force|troop_force|trap_force|knight_force|hero_force"
    ossCbLogkeyContent.str("");
    ossCbLogkeyContent << ptbPlayer->m_nMight << "|";
    ossCbLogkeyContent << pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_ADD_FORCE].m_astBuffDetail[EN_BUFF_TYPE_BUILDING].m_ddwNum << "|";
    ossCbLogkeyContent << pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_ADD_FORCE].m_astBuffDetail[EN_BUFF_TYPE_RESEARCH].m_ddwNum << "|";
    ossCbLogkeyContent << pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_ADD_FORCE].m_astBuffDetail[EN_BUFF_TYPE_TROOP].m_ddwNum << "|";
    ossCbLogkeyContent << pstUser->m_stPlayerBuffList.m_astPlayerBuffInfo[EN_BUFFER_INFO_ADD_FORCE].m_astBuffDetail[EN_BUFF_TYPE_FORT].m_ddwNum << "|";
    ossCbLogkeyContent << 0u << "|";
    ossCbLogkeyContent << 0u;
    SetCbLogKey(ossCbLogOut, ossCbLogkeyContent.str().c_str());

    // ======================================================
    // key19(gem)
    // "gem_now|gem_seq|gem_buy|gem_cost|sandbox|gem_add|is_iap|iap_type"
    ossCbLogkeyContent.str("");
    TINT64 ddwGemCost = 0;
    TINT32 dwSandBox = 1;
    TINT64 ddwGemAdd = 0;

    if("" == strCommand)
    {
        dwSandBox = pstReqParam->m_ucIsSandBox;
        if(pstSession->m_udwCommand != EN_CLIENT_REQ_COMMAND__LOGIN_FAKE &&
           pstSession->m_stCommonResInfo.m_dwRetCode == EN_RET_CODE__SUCCESS &&
           pstSession->m_bAccoutUpt)
        {
            ddwGemAdd = pstSession->m_stUserInfo.m_tbLogin.m_nGem - pstSession->m_ddwGemBegin;
        }
        if(ddwGemAdd < 0)
        {
            pstSession->m_udwGemCost = ddwGemAdd * (-1);
            ddwGemCost = pstSession->m_udwGemCost;
        }
        
    }
    TUINT32 udwGemNow = pstSession->m_stUserInfo.m_tbLogin.m_nGem;
    if(!pstSession->m_bAccoutUpt)
    {
        //如果gem不需要更新,但login_in中gem值发生变化 则还原gem值
        if(pstSession->m_stUserInfo.m_tbLogin.m_nGem != pstSession->m_ddwGemBegin)
        {
            udwGemNow = pstSession->m_ddwGemBegin;
        }
    }

    ossCbLogkeyContent << udwGemNow << "|";
    ossCbLogkeyContent << pstLogin->Get_Gem_seq() << "|";
    ossCbLogkeyContent << pstLogin->Get_Gem_buy() << "|";
    ossCbLogkeyContent << ddwGemCost << "|";
    ossCbLogkeyContent << dwSandBox << "|";
    ossCbLogkeyContent << ddwGemAdd << "|";
    ossCbLogkeyContent << (TUINT32)pstSession->m_ucPromoteReward << "|";
    ossCbLogkeyContent << pstSession->m_dwPromoteTpye;
    SetCbLogKey(ossCbLogOut, ossCbLogkeyContent.str().c_str());

    // ======================================================
    // key20(basic)
    // "user_name|city_name|alliance_name|create_time|updt_time|tax|happiness|tax_utime
    // |tax_population|tax_idle_population|status|population_capacity_limit|hospital_capacity|base_training_speed|game_create_time"
    ossCbLogkeyContent.str("");
    ossCbLogkeyContent << ptbPlayer->Get_Uin() << "|";
    ossCbLogkeyContent << ptbCity->Get_Name() << "|";
    ossCbLogkeyContent << ptbPlayer->Get_Alname() << "|";
    ossCbLogkeyContent << ptbPlayer->Get_Ctime() << "|";
    ossCbLogkeyContent << ptbPlayer->Get_Utime() << "|";
    ossCbLogkeyContent << 0U << "|";
    ossCbLogkeyContent << 0U << "|";
    ossCbLogkeyContent << 0U << "|";
    ossCbLogkeyContent << 0U << "|";
    ossCbLogkeyContent << 0U << "|";
    ossCbLogkeyContent << ptbPlayer->Get_Status() << "|";
    ossCbLogkeyContent << 0U << "|";
    ossCbLogkeyContent << 0U << "|";
    ossCbLogkeyContent << 0U << "|";
    ossCbLogkeyContent << pstLogin->m_nCtime;
    SetCbLogKey(ossCbLogOut, ossCbLogkeyContent.str().c_str());

    // ======================================================
    // key21(stat)
    // "might_gain|might_kill|dtroop|ktroop|dfort|kfort|bat|bat_suc
    // |war|war_suc|help_req|help_rc|help_num|sh_res|sh_might|dcity|res_col
    // |loy_time|loy_itv|loy_cur|loy_all|feats_type|feats_level|tq_n|aq_n|dq_n
    // |pstPlayer->m_nVip_point|pstPlayer->m_nVip_level|pstPlayer->m_nCon_login_days|pstPlayer->m_nShtime"    
    ossCbLogkeyContent.str("");
    ossCbLogkeyContent << ptbPlayer->Get_Mgain() << "|";
    ossCbLogkeyContent << ptbPlayer->Get_Mkill() << "|";
    ossCbLogkeyContent << ptbPlayer->m_nDtroop << "|";
    ossCbLogkeyContent << ptbPlayer->m_nKtroop << "|";
    ossCbLogkeyContent << ptbPlayer->m_nDfort << "|";
    ossCbLogkeyContent << ptbPlayer->m_nKfort << "|";
    ossCbLogkeyContent << ptbPlayer->m_nBat << "|";
    ossCbLogkeyContent << ptbPlayer->Get_Bat_suc() << "|";
    ossCbLogkeyContent << 0U << "|";
    ossCbLogkeyContent << 0U << "|";
    ossCbLogkeyContent << 0U << "|";
    ossCbLogkeyContent << 0U << "|";
    ossCbLogkeyContent << 0U << "|";
    ossCbLogkeyContent << 0U << "|";
    ossCbLogkeyContent << 0U << "|";
    ossCbLogkeyContent << ptbPlayer->m_nDcity << "|";
    ossCbLogkeyContent << 0U << "|";
    ossCbLogkeyContent << ptbPlayer->m_nLoy_time << "|";
    ossCbLogkeyContent << ptbPlayer->m_nLoy_itv << "|";
    ossCbLogkeyContent << ptbPlayer->m_nLoy_cur << "|";
    ossCbLogkeyContent << ptbPlayer->m_nLoy_all << "|";
    ossCbLogkeyContent << 0 << "|";
    ossCbLogkeyContent << 0 << "|";
    ossCbLogkeyContent << 0 << "|";
    ossCbLogkeyContent << 0 << "|";
    ossCbLogkeyContent << 0 << "|";
    ossCbLogkeyContent << ptbPlayer->m_nVip_point << "|";
    ossCbLogkeyContent << CPlayerBase::GetVipLevel(ptbPlayer) << "|";
    ossCbLogkeyContent << 0U << "|";
    ossCbLogkeyContent << 0U;
    SetCbLogKey(ossCbLogOut, ossCbLogkeyContent.str().c_str());

    // ======================================================
    // key22(resource)
    // "fuel|oil|iron|cash|uranium|titannium|..."
    bHead = TRUE;
    ossCbLogkeyContent.str("");
    SCommonResource *pstCommonResource = &ptbCity->m_bResource[0];
    for(TINT32 dwIdx = 0; dwIdx < EN_RESOURCE_TYPE__END; ++dwIdx)
    {
        if(TRUE == bHead)
        {
            bHead = FALSE;
        }
        else
        {
            ossCbLogkeyContent << "|";
        }
        ossCbLogkeyContent << pstCommonResource->m_addwNum[dwIdx];
    }
    SetCbLogKey(ossCbLogOut, ossCbLogkeyContent.str().c_str());


    // "base_produce|logistician_bonus|research_bonus|item_bonus|king_bonus|other_bonus|total_bonus
    // |total_bonus_produce|capacity|cur_production|upkeep|vip"	
    for(TUINT32 udwIdx = 0; udwIdx < EN_RESOURCE_TYPE__END; ++udwIdx)
    {
        ossCbLogkeyContent.str("");
        CHuCBLog::GenProductionCBlog(&pstCityinfo->m_astResProduction[udwIdx], ossCbLogkeyContent);
        SetCbLogKey(ossCbLogOut, ossCbLogkeyContent.str().c_str());
    }

    // ======================================================
    // key29(troop)
    bHead = TRUE;
    ossCbLogkeyContent.str("");
    for(TINT32 dwIdx = 0; dwIdx < EN_TROOP_TYPE__END; ++dwIdx)
    {
        if(TRUE == bHead)
        {
            bHead = FALSE;
        }
        else
        {
            ossCbLogkeyContent << "|";
        }
        ossCbLogkeyContent << ptbCity->m_bTroop[0].m_addwNum[dwIdx];
    }
    SetCbLogKey(ossCbLogOut, ossCbLogkeyContent.str().c_str());

    // ======================================================
    // key30(wound_troop)
    bHead = TRUE;
    ossCbLogkeyContent.str("");
    for(TINT32 dwIdx = 0; dwIdx < EN_TROOP_TYPE__END; ++dwIdx)
    {
        if(TRUE == bHead)
        {
            bHead = FALSE;
        }
        else
        {
            ossCbLogkeyContent << "|";
        }
        ossCbLogkeyContent << ptbCity->m_bHos_wait[0].m_addwNum[dwIdx];
    }
    SetCbLogKey(ossCbLogOut, ossCbLogkeyContent.str().c_str());

    // ======================================================
    // key31(dead_troop)
    bHead = TRUE;
    ossCbLogkeyContent.str("");

    SetCbLogKey(ossCbLogOut, ossCbLogkeyContent.str().c_str());

    TbAlliance_action* ptbAlTrainTroopAction = NULL;
    TbAlliance_action* ptbAlHealTroopAction = NULL;
    TbAlliance_action* ptbAlTrainFortAction = NULL;
    //TbAction* ptbHealFortAction = NULL;
    for (TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwSelfAlActionNum; ++udwIdx)
    {
        if (pstUser->m_aucSelfAlActionFlag[udwIdx] == EN_TABLE_UPDT_FLAG__DEL)
        {
            continue;
        }
        if (!CActionBase::IsPlayerOwnedAction(pstUser->m_tbPlayer.m_nUid, pstUser->m_atbSelfAlAction[udwIdx].m_nId) ||
            pstUser->m_atbSelfAlAction[udwIdx].m_nMclass != EN_ACTION_MAIN_CLASS__TRAIN_NEW)
        {
            continue;
        }

        switch (pstUser->m_atbSelfAlAction[udwIdx].m_nSclass)
        {
        case EN_ACTION_SEC_CLASS__TROOP:
            ptbAlTrainTroopAction = &pstUser->m_atbSelfAlAction[udwIdx];
            break;
        case EN_ACTION_SEC_CLASS__HOS_TREAT:
            ptbAlHealTroopAction = &pstUser->m_atbSelfAlAction[udwIdx];
            break;
        case EN_ACTION_SEC_CLASS__FORT:
            ptbAlTrainFortAction = &pstUser->m_atbSelfAlAction[udwIdx];
            break;
        default:
            break;
        }
    }
    // ======================================================
    // key32(training_troop)
    bHead = TRUE;
    ossCbLogkeyContent.str("");
    if(ptbAlTrainTroopAction)
    {
        if(bHead)
        {
            bHead = FALSE;
        }
        else
        {
            ossCbLogkeyContent << "|";
        }
        ossCbLogkeyContent << (TUINT32)ptbAlTrainTroopAction->m_bParam[0].m_stTrain.m_ddwType << "#";
        ossCbLogkeyContent << ptbAlTrainTroopAction->m_bParam[0].m_stTrain.m_ddwNum << "#";
        ossCbLogkeyContent << ptbAlTrainTroopAction->m_nCtime;
    }
    SetCbLogKey(ossCbLogOut, ossCbLogkeyContent.str().c_str());

    // ======================================================
    // key33(healing_troop)
    bHead = TRUE;
    ossCbLogkeyContent.str("");
    if (ptbAlHealTroopAction)
    {
        if (bHead)
        {
            bHead = FALSE;
        }
        else
        {
            ossCbLogkeyContent << "|";
        }
        ossCbLogkeyContent << (TUINT32)ptbAlHealTroopAction->m_bParam[0].m_stTrain.m_ddwType << "#";
        ossCbLogkeyContent << ptbAlHealTroopAction->m_bParam[0].m_stTrain.m_ddwNum << "#";
        ossCbLogkeyContent << ptbAlHealTroopAction->m_nCtime;
    }
    
    SetCbLogKey(ossCbLogOut, ossCbLogkeyContent.str().c_str());

    // ======================================================
    // key34(fort)
    bHead = TRUE;
    ossCbLogkeyContent.str("");
    for(TINT32 dwIdx = 0; dwIdx < EN_FORT_TYPE__END; ++dwIdx)
    {
        if(TRUE == bHead)
        {
            bHead = FALSE;
        }
        else
        {
            ossCbLogkeyContent << "|";
        }
        ossCbLogkeyContent << ptbCity->m_bFort[0].m_addwNum[dwIdx];
    }
    SetCbLogKey(ossCbLogOut, ossCbLogkeyContent.str().c_str());

 
    // ======================================================
    // key35(training_fort)
    bHead = TRUE;
    ossCbLogkeyContent.str("");
    if(ptbAlTrainFortAction)
    {
        if(bHead)
        {
            bHead = FALSE;
        }
        else
        {
            ossCbLogkeyContent << "|";
        }
        ossCbLogkeyContent << (TUINT32)ptbAlTrainFortAction->m_bParam[0].m_stTrain.m_ddwType << "#";
        ossCbLogkeyContent << ptbAlTrainFortAction->m_bParam[0].m_stTrain.m_ddwNum << "#";
        ossCbLogkeyContent << ptbAlTrainFortAction->m_nCtime;
    }
    SetCbLogKey(ossCbLogOut, ossCbLogkeyContent.str().c_str());

    // ======================================================
    // key36(knight)
    bHead = TRUE;
    ossCbLogkeyContent.str("");
    SetCbLogKey(ossCbLogOut, ossCbLogkeyContent.str().c_str());

    // ======================================================
    // key37(healing_hero)// no reliving
    SetCbLogKey(ossCbLogOut, "");

    // ======================================================
    // key38(build_level)

/*    bHead = TRUE;
    ossCbLogkeyContent.str("");
	
    for(TINT32 dwIdx = 0; dwIdx < EN_BUILDING_TYPE__END; ++dwIdx)
    {
        if(TRUE == bHead)
        {
            bHead = FALSE;
        }
        else
        {
            ossCbLogkeyContent << "|";
        }
        TINT32 dwBuildLevel = CCityBase::GetBuildingLevelByFuncType(pstCityinfo, dwIdx);
        ossCbLogkeyContent << dwBuildLevel;
    }

*/
/************************************************************************/
/* new       andy@20160603: optimize speed                                                              */
/************************************************************************/
	bHead = TRUE;
	ossCbLogkeyContent.str("");
	TINT32 dwTopLevel[EN_BUILDING_TYPE__END];
	memset( dwTopLevel, 0, sizeof(TINT32)*EN_BUILDING_TYPE__END );
	for (TUINT32 idx = 0; idx < pstCityinfo->m_stTblData.m_bBuilding.m_udwNum; idx++)
	{
		SCityBuildingNode *pstBuildingNode = &pstCityinfo->m_stTblData.m_bBuilding[idx];
		if(pstBuildingNode->m_ddwLevel == 0)
		{
			continue;
		}
		TINT32 dwFuncType = CCityBase::GetBuildingFuncType(pstBuildingNode->m_ddwType);
        if (dwFuncType >= EN_BUILDING_TYPE__END)
        {
            continue;
        }
		if (pstBuildingNode->m_ddwLevel > dwTopLevel[dwFuncType] )
		{
			dwTopLevel[dwFuncType] = pstBuildingNode->m_ddwLevel;
		}
	}

	for(TINT32 dwIdx = 0; dwIdx < EN_BUILDING_TYPE__END; ++dwIdx)
	{
		if(TRUE == bHead)
		{
			bHead = FALSE;
		}
		else
		{
			ossCbLogkeyContent << "|";
		}
		ossCbLogkeyContent << dwTopLevel[dwIdx];
	}
    SetCbLogKey(ossCbLogOut, ossCbLogkeyContent.str().c_str());

    // ======================================================
    // key39(building)
    bHead = TRUE;
    ossCbLogkeyContent.str("");
    for(TUINT32 udwIdx = 0; udwIdx < ptbCity->m_bBuilding.m_udwNum; ++udwIdx)
    {
        SCityBuildingNode* pstNode = &ptbCity->m_bBuilding[udwIdx];
        if(pstNode->m_ddwLevel == 0)
        {
            continue;
        }

        if(bHead)
        {
            bHead = FALSE;
        }
        else
        {
            ossCbLogkeyContent << "|";
        }

        ossCbLogkeyContent << pstNode->m_ddwPos << "#";
        ossCbLogkeyContent << (TUINT32)pstNode->m_ddwType << "#";
        ossCbLogkeyContent << (TUINT32)pstNode->m_ddwLevel;
    }
    SetCbLogKey(ossCbLogOut, ossCbLogkeyContent.str().c_str());

    // ======================================================
    // key40(research)
    bHead = TRUE;
    ossCbLogkeyContent.str("");
    for(TINT32 dwIdx = 0; dwIdx < EN_RESEARCH_TYPE__END; ++dwIdx)
    {
        if(TRUE == bHead)
        {
            bHead = FALSE;
        }
        else
        {
            ossCbLogkeyContent << "|";
        }
        ossCbLogkeyContent << pstCityinfo->m_stTblData.m_bResearch[0].m_addwLevel[dwIdx];
    }
    SetCbLogKey(ossCbLogOut, ossCbLogkeyContent.str().c_str());

    // ======================================================
    // key41(skill)
    bHead = TRUE;
    ossCbLogkeyContent.str("");
    for(TUINT32 udwIdx = 0; udwIdx < EN_SKILL_TYPE__END; ++udwIdx)
    {
        if(TRUE == bHead)
        {
            bHead = FALSE;
        }
        else
        {
            ossCbLogkeyContent << "|";
        }
        ossCbLogkeyContent << ptbStat->m_bLord_skill[0].m_addwLevel[udwIdx];
    }
    SetCbLogKey(ossCbLogOut, ossCbLogkeyContent.str().c_str());

    // ======================================================
    // key42(quest)
    bHead = TRUE;
    ossCbLogkeyContent.str("");
    for(TINT32 dwIdx = 0; dwIdx < EN_TOP_QUEST_NUM_LIMIT; ++dwIdx)
    {
        if(BITTEST(ptbStat->m_bTop_quest[0].m_bitQuest, dwIdx) == 0)
        {
            continue;
        }

        if(bHead == TRUE)
        {
            bHead = FALSE;
        }
        else
        {
            ossCbLogkeyContent << "|";
        }
        ossCbLogkeyContent << dwIdx;
    }
    SetCbLogKey(ossCbLogOut, ossCbLogkeyContent.str().c_str());

    // ======================================================
    // key43(item)
    bHead = TRUE;
    ossCbLogkeyContent.str("");
    for(TUINT32 dwIdx = 0; dwIdx < pstUser->m_tbBackpack.m_bItem.m_udwNum; ++dwIdx)
    {
        if (0 == pstUser->m_tbBackpack.m_bItem[dwIdx].m_ddwItemNum)
        {
            continue;
        }
        if(bHead == TRUE)
        {
            bHead = FALSE;
        }
        else
        {
            ossCbLogkeyContent << "|";
        }
        ossCbLogkeyContent << pstUser->m_tbBackpack.m_bItem[dwIdx].m_ddwItemId << "#";
        ossCbLogkeyContent << pstUser->m_tbBackpack.m_bItem[dwIdx].m_ddwItemNum;
    }
    SetCbLogKey(ossCbLogOut, ossCbLogkeyContent.str().c_str());

    // ======================================================
    // key44(mark_item)
    bHead = TRUE;
    ossCbLogkeyContent.str("");
    for(TUINT32 udwIdx = 0; udwIdx < ptbStat->m_bMark.m_udwNum; ++udwIdx)
    {
        if(bHead == TRUE)
        {
            bHead = FALSE;
        }
        else
        {
            ossCbLogkeyContent << "|";
        }
        ossCbLogkeyContent << ptbStat->m_bMark[udwIdx].ddwItemId;
    }
    SetCbLogKey(ossCbLogOut, ossCbLogkeyContent.str().c_str());

    // ======================================================
    // key45(dragon_material)
    SetCbLogKey(ossCbLogOut, "");

    // ======================================================
    // key46(dragon_crystal)
    SetCbLogKey(ossCbLogOut, "");

    // ======================================================
    // key47(card_material)
    SetCbLogKey(ossCbLogOut, "");

    // ======================================================
    // key48(card_crystal)
    SetCbLogKey(ossCbLogOut, "");

    // ======================================================
    // key49(req_url)
    SetCbLogKey(ossCbLogOut, pstSession->m_stReqParam.m_szReqUrl);

    // output
    TSE_LOG_HOUR(CGlobalServ::m_poFormatLog, ("%s", ossCbLogOut.str().c_str()));

    // send to dc
    CDcUdpLog::SendFormatLogToDc(ossCbLogOut.str(), CGlobalServ::m_poServLog);
}

template<class T>
TVOID CHuCBLog::SetCbLogKey(ostringstream &oss, const T &tVariable)
{
    oss << "\t" << tVariable;
}

TVOID CHuCBLog::GenProductionCBlog(SResourceProduction* pstProduction, ostringstream& CBlog)
{
    // "base_produce|logistician_bonus|research_bonus|item_bonus|king_bonus|other_bonus|total_bonus
    // |total_bonus_produce|capacity|cur_production|upkeep|vip"	
    CBlog << pstProduction->m_ddwBaseProduction;
    CBlog << "|" << pstProduction->m_addwBonus[EN_BONUS_TYPE__KNIGHT];
    CBlog << "|" << pstProduction->m_addwBonus[EN_BONUS_TYPE__RESEARCH];
    CBlog << "|" << pstProduction->m_addwBonus[EN_BONUS_TYPE__ITEM];
    CBlog << "|" << pstProduction->m_addwBonus[EN_BONUS_TYPE__DRAGON];
    CBlog << "|" << pstProduction->m_addwBonus[EN_BONUS_TYPE__OTHER];
    CBlog << "|" << pstProduction->m_ddwTotalBonus;
    CBlog << "|" << pstProduction->m_ddwTotalBonusAmount;
    CBlog << "|" << pstProduction->m_ddwCapacity;
    CBlog << "|" << pstProduction->m_ddwCurProduction;
    CBlog << "|" << pstProduction->m_uddwUpkeep;
    CBlog << "|" << pstProduction->m_addwBonus[EN_BONUS_TYPE__VIP];
}
