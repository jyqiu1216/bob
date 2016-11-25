#ifndef _ACTIVITES_LOGIC_H_
#define _ACTIVITES_LOGIC_H_

#include "player_info.h"

class CActivitesLogic
{
public:
    //活动积分逻辑

    static TINT32 ComputeTroopKillScore(SUserInfo *pstUser, TUINT32 udwTroopKillForce, TUINT32 udwFortKillForce, TINT64 ddwTUid, 
                                        TINT64 ddwUid, TUINT32 udwSid, TUINT64 uddwAlid, string strUname);
    static TINT32 ComputeTroopKillScore(SUserInfo *pstUser, TUINT32 udwTroopKillForce, TUINT32 udwFortKillForce, TINT64 ddwTUid);

    static TINT32 ComputeTrainTroopScore(SUserInfo *pstUser, TUINT32 udwType, TUINT32 udwNum);
    static TINT32 ComputeTrainFortScore(SUserInfo *pstUser, TUINT32 udwType, TUINT32 udwNum);

    static TINT32 ComputeBuildUpgradeScore(SUserInfo *pstUser, TUINT32 udwBuildType, TUINT32 udwBuildLv);
    static TINT32 ComputeResearchUpgradeScore(SUserInfo *pstUser, TUINT32 udwType, TUINT32 udwLv);

    static TINT32 ComputeHealTroopScore(SUserInfo *pstUser, TUINT32 udwType, TUINT32 udwNum);
    static TINT32 ComputeHealFortScore(SUserInfo *pstUser, TUINT32 udwType, TUINT32 udwNum);

    static TINT32 ComputeHeroExpMarchScore(SUserInfo *pstUser, TUINT32 udwExp);
    static TINT32 ComputeHeroExpItemScore(SUserInfo *pstUser, TUINT32 udwItemId);
    static TINT32 ComputeHeroExpResearchScore(SUserInfo *pstUser, TUINT32 udwExp);
    static TINT32 ComputeHeroExpBuildingScore(SUserInfo *pstUser, TUINT32 udwExp);

    static TINT32 ComputeSpeedUpItemScore(SUserInfo *pstUser, TUINT32 udwItemId);
    static TINT32 ComputeSpeedUpGemScore(SUserInfo *pstUser, TUINT32 udwTime);

    static TINT32 ComputeBuyGemScore(SUserInfo *pstUser, TUINT32 udwGemNum);
    static TINT32 ComputeCostGemScore(SUserInfo *pstUser, TUINT32 udwGemNum);

    static TINT32 ComputeEquipUpgradeScore(SUserInfo *pstUser, TUINT32 udwTargetLv);

    static TINT32 ComputeTrialAttackScore(SUserInfo *pstUser, TUINT32 udwTrialMode, TUINT32 udwAtkTime);
    static TINT32 ComputeBuyIapScore(SUserInfo *pstUser, TUINT32 udwPayCent);

    static TINT32 ComputeAttackMonsterScore(SUserInfo *pstUser, TINT32 udwMonsterId, TINT32 udwMonsterLv, TINT32 dwAttackNum,
                                            TINT64 ddwUid, TUINT32 udwSid, TUINT64 uddwAlid, string strUname);
    static TINT32 ComputeKillMonsterScore(SUserInfo *pstUser, TINT32 udwMonsterId, TINT32 udwMonsterLv,
                                            TINT64 ddwUid, TUINT32 udwSid, TUINT64 uddwAlid, string strUname);

public:
    static TBOOL IfCalcScore(SUserInfo *pstUser);

private:
    static TUINT32 ComputeStructScoreList(SUserInfo *pstUser, TUINT32 udwScoreType, TUINT32 udwScoreId, TUINT32 udwBaseScore,
                                            TINT64 ddwUid, TUINT32 udwSid, TUINT64 uddwAlid, string strUname);
};
#endif