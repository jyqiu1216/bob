#include "player_info.h"
#include "game_evaluate_logic.h"

SUserInfo::SUserInfo()
{
    Reset();
}

TVOID SUserInfo::Reset()
{
    m_udwBSeqNo = 0;
    m_strErrMsg = "";

    m_udwLotteryChestItemId = 0;
    m_udwLotteryChestItemNum = 0;

    // login info
    m_tbLogin.Reset();
    m_tbApns_token.Reset();
    m_tbPlayer.Reset();
    m_jPlayerRankInfo.clear();
    m_tbUserStat.Reset();

    m_dwErrTroop = 0;
    m_dwErrFort = 0;
    m_dwErrGem = 0;
    m_dwErrProdBonus = 0;
    m_dwErrMarchBonus = 0;
    m_dwErrProd = 0;
    m_dwErrItem = 0;
    m_dwErrBuildLv = 0;
    m_dwShowGemFlag = 0;
    m_dwShowItemFlag= 0;
    m_dwShowChestFlag= 0;
    m_dwShowRemoveFlag = 0;

    udwRewardWinGetType = 0;
    m_tbBounty.Reset();

    m_udwTmpFinishBountyGoal = 0;
    m_vTmpFinishBountyBase.clear();

    // city
    m_stCityInfo.Reset();
    
    // action-active
    m_udwActionNum = 0;
    for (unsigned int i = 0; i < sizeof(m_atbAction)/sizeof(m_atbAction[0]); ++i)
    {
        if (m_atbAction[i].m_nId)
        {
            m_atbAction[i].Reset();
        }
    }
    // match_action-active
    m_udwMarchNum = 0;
    for(unsigned int i = 0; i < sizeof(m_atbMarch) / sizeof(m_atbMarch[0]); ++i)
    {
        if(m_atbMarch[i].m_nId)
        {
            m_atbMarch[i].Reset();
        }
    }
    // match_action-passive
    m_udwPassiveMarchNum = 0;
    for(unsigned int i = 0; i < sizeof(m_atbPassiveMarch) / sizeof(m_atbPassiveMarch[0]); ++i)
    {
        if(m_atbPassiveMarch[i].m_nId)
        {
            m_atbPassiveMarch[i].Reset();
        }
    }
    // al action
    m_udwSelfAlActionNum = 0;
    for(unsigned int i = 0; i < sizeof(m_atbSelfAlAction) / sizeof(m_atbSelfAlAction[0]); ++i)
    {
        if(m_atbSelfAlAction[i].m_nId)
        {
            m_atbSelfAlAction[i].Reset();
        }
    }

    // rally_info defence action
    m_udwRallyMarchNum = 0;
    for (unsigned int i = 0; i < sizeof(m_atbRallyMarch) / sizeof(m_atbRallyMarch[0]); ++i)
    {
        if (m_atbRallyMarch[i].m_nId)
        {
            m_atbRallyMarch[i].Reset();
        }
    }

    m_udwAlCanHelpActionNum = 0;

    // alliance member
    m_tbAlliance.Reset();
    m_udwCurFindMemberNum = 0;
    for (unsigned int i = 0; i < sizeof(m_atbAllianceMember)/sizeof(m_atbAllianceMember[0]); ++i)
    {
        if (m_atbAllianceMember[i].m_nUid)
        {
            m_atbAllianceMember[i].Reset();
        }
    }

    // alliance request
    m_udwAllianceReqTotalNum = 0;
    m_udwAllianceReqCurFindNum = 0;
    for (unsigned int i = 0; i < sizeof(m_atbAllianceReqPlayer)/sizeof(m_atbAllianceReqPlayer[0]); ++i)
    {
        if (m_atbAllianceReqPlayer[i].m_nUid)
        {
            m_atbAllianceReqPlayer[i].Reset();
        }
    }

    for (unsigned int i = 0; i < sizeof(m_atbAl_help)/sizeof(m_atbAl_help[0]); ++i)
    {
        if (m_atbAl_help[i].m_nUid)
        {
             m_atbAl_help[i].Reset();
        }
    }

    m_udwCanHelpTaskNum = 0;
    m_udwAllianceReqCurFindNum = 0;
    m_udwAllianceCanAssistNum = 0;
    m_udwNewAlWallMsgNum = 0;

    m_udwMailNum = 0;
    m_udwReportNum = 0;

    m_pstReqParam = NULL;

    // bookmark
    m_udwBookmarkNum = 0;
    for (unsigned int i = 0; i < MAX_BOOKMARK_NUM; ++i)
    {
        if (m_atbBookmark[i].m_nUid)
        {
            m_atbBookmark[i].Reset();
        }
    }

    // mail
    m_udwMailNum = 0;
    for (unsigned int i = 0; i < MAX_MAIL_PERPAGE_NUM; ++i)
    {
        if (m_atbMailList[i].m_nId)
        {
            m_atbMailList[i].Reset();
        }
    }
    m_stMailUserInfo.Reset();

    // report
    m_udwReportNum = 0;
    for (unsigned int i = 0; i < MAX_REPORT_PERPAGE_NUM; ++i)
    {
        if (m_atbReportList[i].m_nId)
        {
            m_atbReportList[i].Reset();
        }
        m_ptbReport[i] = NULL;
    }
    m_stReportUserInfo.Reset();

    // wild
    m_udwWildNum = 0;
    for (unsigned int i = 0; i < MAX_WILD_RETURN_NUM; ++i)
    {
        if (m_atbWild[i].m_nId)
        {
            m_atbWild[i].Reset();
        }
        m_aucWildFlag[i] = EN_TABLE_UPDT_FLAG__CHANGE;
    }

    // diplomacy
    m_udwDiplomacyNum = 0;
    m_udwDiplomacyDelNum = 0;
    for(unsigned int i = 0; i < MAX_DIPLOMACY_NUM; ++i)
    {
        if(m_atbDiplomacy[i].m_nSrc_al)
        {
            m_atbDiplomacy[i].Reset();
        }
    }
    memset(m_audwDiplomacyTypeNum, 0, sizeof(m_audwDiplomacyTypeNum));
    memset(m_aucDiplomacyFlag, 0, sizeof(m_aucDiplomacyFlag));
    m_tbTargetDiplomacy.Reset();
    m_ucTargetDiplomacyFlag = 0;

    // wall
    m_udwWallNum = 0;
    for(unsigned int i = 0; i < MAX_WALL_MSG_NUM; ++i)
    {
        if(m_atbWall[i].m_nUid)
        {
            m_atbWall[i].Reset();
        }
    }
    memset(m_aucWallFlag, 0, sizeof(m_aucWallFlag));
    m_udwWallDelNum = 0;

    m_tbTmpWall.Reset();
    m_ucTmpWallFlag = 0;

    // al assist
    m_udwAlAssistAllNum = 0;
    for(unsigned int i = 0; i < MAX_AL_ASSIST_NUM; ++i)
    {
        if(m_atbAlAssistAll[i].m_nUid)
        {
            m_atbAlAssistAll[i].Reset();
        }
    }
    memset(m_aucAlAssistAllFlag, 0, sizeof(m_aucAlAssistAllFlag));

    // al store
    m_udwAlConsumeNum = 0;
    for(unsigned int i = 0; i < sizeof(m_atbAl_store_consume) / sizeof(m_atbAl_store_consume[0]); ++i)
    {
        if(m_atbAl_store_consume[i].m_nId)
        {
            m_atbAl_store_consume[i].Reset();
        }
    }

    // tips
    m_udwTipsNum = 0;
    memset(m_aucTipsFlag, 0, sizeof(m_aucTipsFlag));

    //event tips
    m_udwEventTipsNum = 0;
    memset(m_aucEventTipsFlag, 0, sizeof(m_aucEventTipsFlag));
    for(TUINT32 udwIdx = 0; udwIdx < MAX_EVENT_TIPS_NUM * 2;++udwIdx)
    {
        m_atbEventTips[udwIdx].Reset();
    }

	//backpack
	m_tbBackpack.Reset();
    m_stRewardWindow.Reset();

    m_szIapDesc.clear();
    ///////////////
    m_uddwNewActionId = 0;
    m_bNewBreakFlag = false;
    m_bFirstJoinAlReward = false;

    m_ucAccountFlag = 0;
    m_ucPlayerFlag = 0;
    memset(m_aucActionFlag, 0, sizeof(m_aucActionFlag));
    memset(m_aucMarchFlag, 0, sizeof(m_aucMarchFlag));
    memset(m_aucPassiveMarchFlag, 0, sizeof(m_aucPassiveMarchFlag));
    memset(m_aucSelfAlActionFlag, 0 , sizeof(m_aucSelfAlActionFlag));
    m_ucAllianceFlag = 0;
    memset((TCHAR*)&m_aucBookMarkFlag[0], 0, sizeof(TUINT8)*MAX_BOOKMARK_NUM);
    memset(m_aucAlHelpFlag, 0, sizeof(m_aucAlHelpFlag));

    m_tbQuest.Reset();
    m_tbTask.Reset();

    //buffer info 
    m_stPlayerBuffList.Reset();
    m_stBuffWithoutDragon.Reset();

    m_stScore.Reset();
    
    m_stAlGifts.Reset();
    for(TUINT32 idx = 0; idx < MAX_AL_IAP_GIFT_NUM * 2; idx++)
    {
        if(m_atbAlGiftReward[idx].m_nUid)
        {
            m_atbAlGiftReward[idx].Reset();
        }
    }
    m_udwAlGiftRewardNum = 0;

    m_ddwRawVipPoint = 0;
    m_bTodayFirstLogin = FALSE;

    m_udwMailSendNum = 0;
    m_udwBeforAlid = 0;
    m_udwBeforPlayerExp = 0;

    memset(m_adwMailSendUidList, 0, sizeof(m_adwMailSendUidList));

    m_udwTmpFinishNum = 0;
    memset(m_audwTmpFinishTaskId, 0, sizeof(m_audwTmpFinishTaskId));

    m_ucSelfAlmemberFlag = 0;
    m_tbSelfAlmember.Reset();
    m_tbSelfName.Reset();

    m_udwCostResource = 0;
    m_bUpdtPlayerUtime = 0;
    m_stScore.Reset();

    m_bRatingSwitch = FALSE;
    m_udwRatingGem = 0;

    for(TINT32 dwIdx = 0; dwIdx < 2; ++dwIdx)
    {
        m_astGameEvaluateExDataRaw[dwIdx].Reset();
        m_astGameEvaluateExDataNew[dwIdx].Reset();
    }
    m_bGameEvaluateType = false;
    m_uddwCurEventId = 0;
    m_udwMoveNewCityId = 0;

    m_vCheckTaskId.clear();
    m_vecTaskList.clear();

    m_stUserEventGoals.Reset();
    m_bIsSendEventReq = FALSE;

    //broadcast
    m_udwBroadcastNum = 0;
    for(TINT32 dwIdx = 0; dwIdx < MAX_BROADCAST_NUM_TOTAL; dwIdx++)
    {
        m_atbBroadcast[dwIdx].Reset();
        m_aucBroadcastFlag[dwIdx] = EN_TABLE_UPDT_FLAG__UNCHANGE;
    }

    m_udwBlackListNum = 0;
    for (TUINT32 udwIdx = 0; udwIdx < MAX_BLACKLIST_NUM; udwIdx++)
    {
        m_atbBlackList[udwIdx].Reset();
        m_aucBlackListFlag[udwIdx] = 0;
    }

    m_aucSvrAlFlag = 0;
    m_tbSvrAl.Reset();

    m_uddwLastMight = 0;
    m_sLastTroopAndFort = "";
    m_uddwCurMight = 0;
    m_sCurTroopAndFort = "";

    m_udwEquipNum = 0;
    for (unsigned int i = 0; i < MAX_USER_EQUIP_NUM; ++i)
    {
        if (m_atbEquip[i].m_nId)
        {
            m_atbEquip[i].Reset();
        }
        m_aucEquipFlag[i] = 0;
    }

    ptbThrone = NULL;

    m_dwRefreshDailyQuestNum = 0;
    m_dwRefreshAllianceQuestNum = 0;
    m_dwRefreshVipQuestNum = 0;
    m_dwRefreshpQuesType = 0;
    
    m_setInsertTask.clear();

    m_dwRewardWindowNum = 0;
    for (TUINT32 udwIdx = 0; udwIdx < MAX_REWARD_WINDOW_NUM; udwIdx++)
    {
        m_aucRewardWindowFlag[udwIdx] = EN_TABLE_UPDT_FLAG__DEL;
    }

    m_udwRandomRewardNum = 0;
    memset(m_aucRandomRewardFlag, 0, sizeof(m_aucRandomRewardFlag));

    m_bTaskClearCmd = 0;
    m_bTaskUpdate = 0;

    m_dwTimeForMoveCityPrepare = 0;
	
    m_tbDataOutput.Reset();


    m_ptbPushMarchAction = NULL;
    dwPushMarchActionType = EN_TABLE_UPDT_FLAG__UNCHANGE;
    m_ptbPushAlAction = NULL;
    dwPushAlActionType = EN_TABLE_UPDT_FLAG__UNCHANGE;
    m_ptbPushTips = NULL;
}

