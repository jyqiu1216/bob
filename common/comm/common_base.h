#ifndef _COMMON_BASE_H_
#define _COMMON_BASE_H_

#include "player_info.h"

class CCommonBase
{
public:
    static TVOID CalcMarchActionTroop(SUserInfo *pstUser, TINT64 *addwTroop, TBOOL bIsRawTroop = FALSE);

    static TVOID ProtectGuideResource(SUserInfo *pstUser);

    //根据player 里的vip point 和 vip etime更新VIP action
    static TVOID UpdateVipInfo(SUserInfo *pstUserInfo, SCityInfo *pstCityInfo);

    //ComputeUserMight
    static TVOID ComputeUserMight(SUserInfo *pstUser);

    //GetTroopsNum
    static TINT64 CalcTroopsNum(SUserInfo *pstUser);
    static TINT64 CalcFortsNum(SUserInfo *pstUser);

    //ComputeTotalMight
    static TUINT64 GetTotalMight(SUserInfo *pstUser, TBOOL bNumOrCoeff);

    static TVOID GetInCityTroop(SUserInfo* pstUser, SCityInfo* pstCity, SCommonTroop& stInCityTroop);

    //Reinforce
    static TVOID GetReinforceTroop(SUserInfo* pstUser, SCityInfo* pstCity, SCommonTroop& stReinforceTroop);
    static TINT64 GetReinforcedMarchNum(SUserInfo* pstUser, SCityInfo* pstCity);
    static TINT64 GetReinforcedTroopNum(SUserInfo* pstUser, SCityInfo* pstCity);
    static TINT64 GetReinforceMarchLimit(SUserInfo* pstUser);
    static TINT64 GetReinforceTroopLimit(SUserInfo* pstUser);

    static TINT64 GetMarchTroopNum(TbMarch_action *ptbMarch);

    //OccupyWild
    static TVOID OccupyWild(TbPlayer *pstPlayer, TbMap *pstMap, TbMarch_action *pstAction = NULL);
    static TVOID OccupyWildByMarch(TbMap *pstMap, TbMarch_action *ptbAction);
    static TVOID CampWild(TbPlayer *pstPlayer, TbMap *pstMap, TbMarch_action *pstAction = NULL);

    //AbandonWild
    static TVOID AbandonWild(SCityInfo *pstCity, TbMap *pstMap);

    //AddAction
    static TVOID AddAction(SUserInfo *pstUser, SCityInfo*pstCity, TUINT8 enMainClass, TUINT8 enSecClass, TUINT8 enStatus, TUINT32 udwCostTime,
        UActionParam *pstParam, TUINT64 uddwNewTaskId = 0Lu);

    //UpdateMapToNewCity
    static TVOID SetMapToNewCity(TbMap *pstMapItem, TbPlayer *pstPlayer, SCityInfo *pstCity, TbAlliance *pstAlliance, TUINT32 udwSTime = 0, TUINT32 udwBTime = 0, TUINT32 udwPrisonFlag = 0);

    //UpdatePlayerTipsState
    static TVOID SetPlayerStateTipsTime(SUserInfo *pstUser);

    //MonitorUserInfo
    static TUINT64 PeriodProcMonitorUserInfo(SUserInfo *pstUser);

    //GetMaxMarchNum
    //static TINT32 GetMaxMarchNum(SUserInfo *pstUser);

    static TVOID SetActionDeleteFlag(SUserInfo* pstUser, TINT64 ddwTargetActionId, TUINT32 udwSecondClass);
    static TVOID SetActionDeleteFlag(TbAction* ptbActionList, TUINT8* pActionFlagList, TINT32 dwActionNum, TINT64 ddwTargetActionId);
    static TVOID SetAlActionDeleteFlag(TbAlliance_action* ptbActionList, TUINT8* pActionFlagList, TINT32 dwActionNum, TINT64 ddwTargetActionId);
    static TVOID SetMarchActionDeleteFlag(TbMarch_action* ptbActionList, TUINT8* pActionFlagList, TINT32 dwActionNum, TINT64 ddwTargetActionId);

    //增加VIP点数
    static TINT32 AddVipPoint(SUserInfo *pstUserInfo, SCityInfo *pstCityInfo, TUINT32 udwVipPointAdd);

    //增加VIP时间
    static TINT32 AddVipTime(SUserInfo *pstUserInfo, SCityInfo *pstCityInfo, TUINT32 udwVipTimeAdd);

    //获取research 的 cost
    static TINT32 GetResearchCost(TINT32 dwResearchId, TINT32 dwTargetLv, SSpGlobalRes& rCost);

    //************************************
    // Method:    GetAlGiftLevel
    // FullName:  CCommonBase::GetAlGiftLevel
    // Access:    public static 
    // Returns:   TINT32
    // Qualifier: get the alliance's gift level
    // Parameter: TbAlliance * ptbAlliance
    //************************************
    static TINT32 GetAlGiftLevel(TbAlliance* ptbAlliance);

    static TINT64 GetAlGiftPoint(TINT64 ddwPoint);


    //************************************
    // Method:    GetConLoginVipPoint
    // FullName:  CCommonBase::GetConLoginVipPoint
    // Access:    public static 
    // Returns:   TINT64
    // Qualifier: get the reward vip point from gamejson
    // Parameter: TINT64 ddwConLoginDays
    //************************************
    static TINT64 GetConLoginVipPoint(TINT64 ddwConLoginDays);


    //************************************
    // Method:    GetVipLevelUpRewardTime
    // FullName:  CCommonBase::GetVipLevelUpRewardTime
    // Access:    public static 
    // Returns:   TUINT32
    // Qualifier: get the reward vip time form gamejson
    // Parameter: TINT32 dwLevelUp
    //************************************
    static TUINT32 GetVipLevelUpRewardTime(TINT32 dwLevelUp);

    //获取双方联盟关系
    static TbDiplomacy *GetDiplomacy(TbDiplomacy *patbDiplomacyList, TUINT32 udwDipNum, TINT32 dwTargetAlid);

    //获取action中troop总的load值
    static TINT32 GetTroopTotalLoad(TbMarch_action *ptbMarch, TUINT32 udwCurTime = 0);

    static TINT64 GetTroopTotalLoad(SCommonTroop *pstTroop);

    //有没有处于执行中的action
    static TBOOL IsActionDoingOnUser(SUserInfo *pstUserInfo);

    //fund
    static TINT32 AddAllianceFund(TbPlayer *pstPlayer,TbAlliance *pstAlliance,TUINT32 udwNum);
    static TINT32 CostAllianceFund(TbPlayer *pstPlayer, TbAlliance *pstAlliance, TUINT32 udwNum);
    static TINT32 HasEnoughAllianceFund(TbPlayer *pstPlayer, TbAlliance *pstAlliance, TUINT32 udwNum);

    static TINT32 GetRewardBoxUiType(SSpGlobalRes *pstGlobalRes);

    static TUINT64 ComputeTotalMight(SUserInfo *pstUser);

    static TINT32 UpdateResourceHelpInfo(SUserInfo *pstUser, TbMarch_action *ptbTransportMarch, TUINT8 ucAssistType);

    static TINT64 GetGameBasicVal(TINT32 dwKey);

    static TUINT64 ComputeTotalTroopMight(SUserInfo *pstUser, TUINT32 &udwTroopNum);

    static TBOOL ComputeTotalTroop(SUserInfo *pstUser, TINT64 addwTroop[]);

    static TUINT64 ComputeFortMight(SUserInfo *pstUser, TUINT32 &udwFortNum);

    static TVOID SetPlayerStateEventTipsTime(SUserInfo *pstUser, TBOOL bIsOperateCmd, TBOOL bIsLogin);

public:
    static TINT32 GetUserDna(TINT64 ddwUserId, TINT32 dwCastleLv, TINT32 dwGemBuy, TINT32 dwAlGiftLv, TINT32 dwMaxAlGemBuy,
            TUINT32 udwSid, SUserEventGoals* pOutUserEventGoals);

    static bool BaseCompare(pair<TINT32, TINT32> i, pair<TINT32, TINT32> j)
    {
        return i.second == j.second ? i.first < j.first : i.second > j.second;
    }
    static TINT32 ComputeSpeedUpGem(TINT32 dwSpeedUpSec);
    static TINT32 GetTimeOptimizeGem(TINT32 dwTime, TINT32 dwIdx, vector<pair<TINT32, TINT32> > &vecList);

    static TBOOL IsCorrectTime(TINT32 dwGivenTime, TINT32 dwCalcTime);
    static TINT32 CheckInstantGem(TINT32 dwGivenCost, TINT32 dwSpeedUpTime);

    static TINT32 GetTroopMarchTime(SCommonTroop *pstTroop, TINT32 dwFrom, TINT32 dwTo, SPlayerBuffInfo *pstBuff);
    static TINT32 GetTroopReinforceTime(SCommonTroop *pstTroop, TINT32 dwFrom, TINT32 dwTo, SPlayerBuffInfo *pstBuff);
    static TINT32 GetTroopTransportTime(TINT32 dwFrom, TINT32 dwTo, SPlayerBuffInfo *pstBuff);
    static TINT32 GetTroopRallyWarTime(SCommonTroop *pstTroop, TINT32 dwFrom, TINT32 dwTo, SPlayerBuffInfo *pstBuff);
    static TINT32 GetScoutMarchTime(TINT32 dwFrom, TINT32 dwTo, TUINT32 udwBuff);
    static TINT32 GetHeroMarchTime(TINT32 dwFrom, TINT32 dwTo, TUINT32 udwBuff);

public:
    static TUINT32 GetPlayerAllianceId(TUINT32 udwAlId, TUINT8 ucAlPos);

    static TBOOL HasEnoughDoubloon(TbPlayer *ptbPlayer, TINT32 dwPrice);
    static TINT32 CostDoubloon(TbPlayer *ptbPlayer, TINT32 dwPrice);
    static TBOOL IsUnlockTroop(TINT32 dwResearchId);

    static TUINT64 GetTotalTroopAndFort(SUserInfo *pstUser, Json::Value &jsonRet);

    static TUINT64 GenLockIdByLockInfo(TUINT64 uddwKey, TINT32 dwType, TUINT32 udwSid);

private:
    static TBOOL IsSpUiType(TUINT32 *audwTypeNum, TINT32 *adwListTypeNum, TUINT32 udwNum);
    static TBOOL IsNormalUiType(TUINT32 *audwTypeNum, TINT32 *adwListTypeNum, TUINT32 udwNum);

public:
    static TINT32 AddRewardWindow(SUserInfo *pstUser, TINT64 ddwTargetId, TUINT32 udwRewardType, TUINT32 udwRewardGetType, TINT64 ddwShowTime, SGlobalRes *pstReward, TBOOL bIsNeedPut, const Json::Value& jInfo = Json::Value(Json::objectValue));
    static TINT32 AddRewardWindow(SUserInfo *pstUser, TINT64 ddwTargetId, TUINT32 udwRewardType, TUINT32 udwRewardGetType, TINT64 ddwShowTime, SSpGlobalRes *pstReward, TBOOL bIsNeedPut, const Json::Value& jInfo = Json::Value(Json::objectValue));

public:
    static TINT32 UpdatePlayerStatusOnSpecailWild(SUserInfo *pstUser);

    static TVOID FindTrialOpenTime(SUserInfo *pstUser, TUINT32 udwTime, TUINT32& udwOpen, TUINT32& udwBtime, TUINT32& udwEtime);
    static TVOID GetTrialGift(TbPlayer *ptbPlayer, TUINT32 udwTime, TUINT32& udwHasGift, TUINT32& udwEndTime, SOneGlobalRes& sReward);

public:
    static TVOID UpdateOccupyLairStat(SUserInfo *pstUser, TbMap *ptbWild, TINT64 ddwExceptActionId = 0);

public:
    static TVOID GetRoadList(TUINT32 udwSourcePos, TUINT32 udwTargetPos, vector<TUINT32> &vecRoadList);

    static TINT32 GetTroopRallyWarTimeNew(SCommonTroop *pstTroop, TINT32 dwFrom, TINT32 dwTo, SPlayerBuffInfo *pstBuff);

};

#endif //_COMMON_BASE_H_
