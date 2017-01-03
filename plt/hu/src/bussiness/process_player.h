#ifndef _PROCESS_PLAYER_H_
#define _PROCESS_PLAYER_H_

#include "session.h"
#include "city_info.h"

class CProcessPlayer
{
public: 
    /******************************用户帐号相关的操作***********************************/
    // function  ===> 定时或强制login_get
    static TINT32 ProcessCmd_LoginGet(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_NewLoginCreate(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_LoginFake(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_GuideFinish(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_GuideFinishStage(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_UserInfoCreate(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_SvrChange(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 ProcessCmd_BuildingUpgrade(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_BuildingRemove(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_BuildingMove(SSession* pstSession, TBOOL& bNeedResponse);
    static TINT32 ProcessCmd_BuildingEdit(SSession* pstSession, TBOOL& bNeedResponse);
    static TINT32 ProcessCmd_AgeUpgrade(SSession* pstSession, TBOOL& bNeedResponse);
    static TINT32 ProcessCmd_ResearchUpgrade(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_RemoveObstacle(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 ProcessCmd_TroopTrain(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_FortTrain(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_TroopDismiss(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_FortDismiss(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_DeadFortHeal(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_DeadFortAbandon(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_HospitalTroopTreat(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_AbandWoundTroop(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 ProcessCmd_PlayerInfoGet(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_ChangePlayerName(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_ChangeBaseName(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 ProcessCmd_QuestStart(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_QuestRewardCollect(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_RefreshTimeQuest(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_GetTimerGift(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_SetGuideFlag(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_TaskClearFlag(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_TaskOperate(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_RandomReward(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_OpenTaskWinDow(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 ProcessCmd_PlayerAvatarChange(SSession* pstSession, TBOOL& bNeedResponse);
    static TINT32 ProcessCmd_DragonNameChange(SSession* pstSession, TBOOL& bNeedResponse);
    static TINT32 ProcessCmd_DragonAvatarChange(SSession* pstSession, TBOOL& bNeedResponse);

    static TINT32 ProcessCmd_LordSkillUpgrade(SSession* pstSession, TBOOL& bNeedResponse);
    static TINT32 ProcessCmd_LordSkillUpgradeNew(SSession* pstSession, TBOOL& bNeedResponse);
    static TINT32 ProcessCmd_LordSkillReset(SSession* pstSession, TBOOL& bNeedResponse);
    static TINT32 ProcessCmd_DragonSkillUpgrade(SSession* pstSession, TBOOL& bNeedResponse);
    static TINT32 ProcessCmd_DragonSkillUpgradeNew(SSession* pstSession, TBOOL& bNeedResponse);
    static TINT32 ProcessCmd_DragonSkillReset(SSession* pstSession, TBOOL& bNeedResponse);
    static TINT32 ProcessCmd_DragonMonsterSkillUpgrade(SSession* pstSession, TBOOL& bNeedResponse);
    static TINT32 ProcessCmd_DragonMonsterSkillUpgradeNew(SSession* pstSession, TBOOL& bNeedResponse);
    static TINT32 ProcessCmd_DragonMonsterSkillReset(SSession* pstSession, TBOOL& bNeedResponse);

    static TINT32 ProcessCmd_QuestClaim(SSession *pstSession, TBOOL& bNeedResponse);
    static TINT32 ProcessCmd_LordLevelUp(SSession *pstSession, TBOOL& bNeedResponse);
    static TINT32 ProcessCmd_DragonLevelUp(SSession *pstSession, TBOOL& bNeedResponse);
	
    static TINT32 ProcessCmd_ActiveSecondBuildAction(SSession *pstSession, TBOOL &bNeedResponse);

    //equip 
    static TINT32 ProcessCmd_ComposeEquip(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_ComposeMaterial(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_ComposeSoul(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_ComposeParts(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_ComposeCrystal(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_ComposeSpCrystal(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_AddEquipGride(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_CrystalInsert(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_CrystalRemove(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_PutOnEquip(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_PutOffEquip(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_DestroyEquip(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_BuyScroll(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_BuyScrollNew(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_DropScroll(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 ProcessCmd_ReleaseDragon(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_KillDragon(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_ReviveDragon(SSession *pstSession, TBOOL &bNeedResponse);

    //rating
    static TINT32 ProcessCmd_RatingRewardCollect(SSession *pstSession, TBOOL& bNeedResponse);

    static TINT32 ProcessCmd_MysteryStoreBuy(SSession *pstSession, TBOOL& bNeedResponse);
    static TINT32 ProcessCmd_MysteryStorePass(SSession *pstSession, TBOOL& bNeedResponse);

    static TINT32 ProcessCmd_BlackUserAdd(SSession *pstSession, TBOOL& bNeedResponse);
    static TINT32 ProcessCmd_BlackUserDel(SSession *pstSession, TBOOL& bNeedResponse);

    static TINT32 ProcessCmd_ClearHelpBubble(SSession *pstSession, TBOOL& bNeedResponse);

    static TINT32 ProcessCmd_RecoveryMergeUser(SSession *pstSession, TBOOL& bNeedResponse);

    static TINT32 ProcessCmd_KnightAssign(SSession *pstSession, TBOOL& bNeedResponse);

    static TINT32 ProcessCmd_PersonGuideClaim(SSession *pstSession, TBOOL& bNeedResponse);

    static TINT32 ProcessCmd_DragonUnlock(SSession *pstSession, TBOOL& bNeedResponse);

    static TINT32 ProcessCmd_AddRandomReward(SSession *pstSession, TBOOL& bNeesResponse);
    static TINT32 ProcessCmd_GetRandomReward(SSession *pstSession, TBOOL& bNeesResponse);

    static TINT32 ProcessCmd_ClaimRandomReward(SSession *pstSession, TBOOL& bNeesResponse);

    static TINT32 ProcessCmd_TrialInit(SSession *pstSession, TBOOL& bNeesResponse);
    static TINT32 ProcessCmd_TrialRageMode(SSession *pstSession, TBOOL& bNeesResponse);
    static TINT32 ProcessCmd_TrialNormalMode(SSession *pstSession, TBOOL& bNeesResponse);
    static TINT32 ProcessCmd_TrialAttack(SSession *pstSession, TBOOL& bNeesResponse);
    static TINT32 ProcessCmd_TrialLuckyBagNormal(SSession *pstSession, TBOOL& bNeesResponse);
    static TINT32 ProcessCmd_TrialLuckyBagRage(SSession *pstSession, TBOOL& bNeesResponse);
    static TINT32 ProcessCmd_TrialGiftCollect(SSession *pstSession, TBOOL& bNeesResponse);
    static TINT32 ProcessCmd_FinishGuide(SSession *pstSession, TBOOL& bNeesResponse);

    static TINT32 ProcessCmd_BuildDecoration(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_OpenDecorationList(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_DeleteDecoration(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_GiveGift(SSession *pstSession, TBOOL& bNeedResponse);

private:
    static TINT32 Relive(SUserInfo *pstUser, SCityInfo *pstCity, TUINT8 ucTargetType, 
        TINT64 ddwTargetId, TUINT32 udwCostTime, TUINT32 udwNum /*= 1*/, TUINT32 udwExp /*= 0*/);

    static TINT32 Train(SUserInfo *pstUser, SCityInfo *pstCity, TUINT8 ucType, TUINT8 ucId, TUINT32 udwNum, TUINT32 udwCostTime, TUINT32 udwExp, TUINT32 udwNeedResource, TUINT32 udwGemNum, TBOOL bIsNeedCheck);
};

#endif
