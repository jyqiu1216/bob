#ifndef _PALYER_BASE_H_
#define _PALYER_BASE_H_


#include "game_info.h"
#include "aws_table_include.h"
#include "time_utils.h"
#include <math.h>
#include "player_info.h"


class CPlayerBase
{
public:
    // function  ===> 判断用户是否有足够的gem
    // in_value  ===> ptbLogin: 待操作的login表
    //           ===> udwGemNum: 是否存在这个数量值的gem
    // out_value ===> 返回是否足够gem(true:足够;false:不足够)
    static TBOOL HasEnoughGem(TbLogin *ptbLogin, TUINT32 udwGemNum);

    // function  ===> 消耗用户的gem
    // in_value  ===> ptbLogin: 待操作的login表
    //           ===> udwGemNum: 待消耗的gem数量
    static TVOID CostGem(SUserInfo *pstUser, TUINT32 udwGemNum);

    // function  ===> 增加用户的gem
    // in_value  ===> ptbLogin: 待操作的login表
    //           ===> udwGemNum: 待增加的gem数量
    static TVOID AddGem(TbLogin *ptbLogin, TUINT32 udwGemNum);

    // function  ===> 设置用户的gem
    // in_value  ===> ptbLogin: 待操作的login表
    //           ===> udwGemNum: 待设置的gem数量
    static TVOID SetGem(TbLogin *ptbLogin, TUINT32 udwGemNum);

    // function  ===> 检测用户是否是死用户
    // in_value  ===> ptbPlayer: 待操作的player表
    //           ===> udwTime: 当前时间
    // out_value ===> 返回是否是死用户的判断（true:死用户;false:活跃用户）
    static TBOOL IsDeadPlayer(TbPlayer *ptbPlayer, TUINT32 udwTime);

    // function  ===> 判断用户是否已经改过名字
    // in_value  ===> ptbPlayer: 待操作的player表
    // out_value ===> 返会用户是否已经改过名字的状态（true:改过;false:没改）
    static TBOOL HasChangePlayerName(const TbPlayer* ptbPlayer);

    // function  ===> 增加用户的国王的exp
    // in_value  ===> ptbPlayer: 待操作的player表
    //           ===> udwExp: 需要增加的国王exp
    static TVOID AddLordExp(SUserInfo *pstUser, SCityInfo *pstCity, TUINT32 udwExp, TBOOL bNeedBuff = TRUE);

    static TVOID AddDragonExp(SUserInfo *pstUser, TUINT32 udwExp, TBOOL bNeedBuff = TRUE);
    // function  ===> 增加用户头像
    // in_value  ===> ptbLordImage: 待操作的LordImage表
    //           ===> udwImageId: 增加的头像id
    static TINT32 AddLordImage(TbLord_image *ptbLord_image, TUINT32 udwImageId);
    // function  ===> 增加用户装饰
    // in_value  ===> ptbDecoration: 待操作的Decoration表
    //           ===> udwDecorationId: 增加的装饰id
    static TINT32 AddDecoration(TbDecoration *ptbDecoration, TUINT32 udwDecoId, TUINT32 udwItemNum = 1);
    static TINT32 SetDecoration(TbDecoration *ptbDecoration, string strDecoId, TUINT32 udwItemNum);
    static TINT32 HasEnoughDecoration(TbDecoration *ptbDecoration, TUINT32 udwDecoId, TUINT32 udwNum = 1);
    static TINT32 CostDecoration(TbDecoration *ptbDecoration, TUINT32 udwDecoId);
    static TINT32 PickUpDecoration(TbDecoration *ptbDecoration, TUINT32 udwDecoId);

    static TINT64 GetRawVipLevelPoint(TINT32 dwLevel);
    static TINT32 GetRawVipLevel(TbPlayer* ptbPlayer, TINT64 ddwVipPoint);
    static TINT32 GetRawVipStage(TINT32 dwLevel);
    static TINT64 GetMaxVipPoint(TbPlayer* ptbPlayer);
    static TINT64 GetMaxVipPoint();
    static TINT32 GetMaxVipStage();
    static TINT32 GetVipLevel(TbPlayer* ptbPlayer);
    static TINT32 ComputePlayerLevel(TbPlayer* ptbPlayer);
    static TINT32 ComputeDragonLevel(TINT64 ddwExp);

    static TINT32 GetLeftLordSkillPoint(SUserInfo *pstUserInfo);
    static TINT32 GetLeftDragonSkillPoint(SUserInfo *pstUserInfo);
    static TINT32 GetLeftDragonMonsterSkillPoint(SUserInfo *pstUserInfo);

    static TINT32 GetLeftLordSkillPoint(TbPlayer* ptbPlayer, TbUser_stat* ptbUserStat);
    static TINT32 GetUsedLoadSkillPoint(TbUser_stat* ptbUserStat);
    static TINT32 GetLordSkillPointLimit(TINT32 dwSkillId);

    static TINT32 GetLeftDragonSkillPoint(TbPlayer* ptbPlayer, TbUser_stat* ptbUserStat);
    static TINT32 GetDragonSkillPointLimit(TINT32 dwSkillId);
    static TINT32 GetLeftDragonMonsterSkillPoint(TbPlayer* ptbPlayer, TbUser_stat* ptbUserStat);
    static TINT32 GetDragonMonsterSkillPointLimit(TINT32 dwSkillId);
    static TBOOL IsMeetLordSkillReliance(SUserInfo* pstUser, TINT32 dwSkillId, TINT32 dwSKillLevel);
    static TBOOL IsMeetDragonSkillReliance(SUserInfo* pstUser, TINT32 dwSkillId, TINT32 dwSKillLevel);
    static TBOOL IsMeetDragonMonsterSkillReliance(SUserInfo* pstUser, TINT32 dwSkillId, TINT32 dwSKillLevel);

    static TINT32 AddLoyality(TbPlayer *pstPlayer,TUINT32 udwNum);
    static TINT32 CostLoyality(TbPlayer *pstPlayer, TUINT32 udwNum);
    static TINT32 HasEnoughLoyality(TbPlayer *pstPlayer, TUINT32 udwNum);

    static TINT32 CheckUpdtDragonEnergy(SUserInfo *pstUser);
    static TINT32 AddDragonEnergy(SUserInfo *pstUser, TUINT32 udwNum);

    static TINT32 AddDragonShard(SUserInfo *pstUser, TUINT32 udwNum);
    static TINT32 SetDragonShard(SUserInfo *pstUser, TUINT32 udwNum);
    static TINT32 CostDragonShard(SUserInfo *pstUser, TUINT32 udwNum);
    static TBOOL HasEnoughDragonShard(SUserInfo *pstUser, TUINT32 udwNum);
    static TVOID GetTrialAttackCost(SUserInfo *pstUser, TUINT32 udwAtkNum, TUINT32 &udwItemNum, TUINT32 &udwShardNum);

    static TINT64 GetMaxDragonExp(TINT64 ddwMaxLv = 0);
    static TINT64 GetMaxLordExp();


    static TUINT32 GetDragonExcuteTime(TUINT32 udwLevel);

    static TUINT32 GetAltarBuffTime(TUINT32 udwHerolv);

    static TINT64 GetCurDragonMaxEnergy(SUserInfo *pstUser);

    static TUINT32 GetAllianceId(TbPlayer *ptbPlayer);

public:
    static TVOID CheckAndUpdtKnightInfo(SUserInfo *pstUser);
    static TVOID AutoUnassignKnight(SUserInfo *pstUser);

public:
    static TINT32 GetCurPersonGuide(SUserInfo *pstUser);

public:
    static TINT32 AddDragon(TbPlayer *ptbPlayer, TINT64 ddwMaxEnergy);

private:
    
    static TBOOL CheckPersonGuideVIP(SUserInfo *pstUser, TUINT32 udwKey);

    static TBOOL CheckPersonGuidePlayerSkill(SUserInfo *pstUser, TUINT32 udwKey);

    static TBOOL CheckPersonGuideBuilding(SUserInfo *pstUser, TUINT32 udwKey);
    
    static TBOOL CheckPersonGuideChest(SUserInfo *pstUser, TUINT32 udwKey);
    
    static TBOOL CheckPersonGuideJoinAlliance(SUserInfo *pstUser, TUINT32 udwKey);

    static TBOOL CheckPersonGuideNeverUseSpeedItem(SUserInfo *pstUser, TUINT32 udwKey);

public:
    static TBOOL CheckDeadPlayerBaseCondForClear(SUserInfo *pstUser);
};



#endif
