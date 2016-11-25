#ifndef _WAR_BASE_H_
#define _WAR_BASE_H_

#include "player_info.h"

#define ARMY_DATA_EXPAND_RATIO 10000  // attack health defend 扩大倍数  防止出现小数
#define MAX_ATTACK_TIMES 100

struct BattleBuff
{
    TINT64 atk;
    TINT64 hp;
    TINT64 def;

    BattleBuff()
    {
        Reset();
    }

    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
};

typedef TINT64 ArmyId;
struct OneArmy
{
    ArmyId army_id;

    TINT64 kill_troop_num;
    TINT64 kill_troop_force;
    TINT64 kill_fort_num;
    TINT64 kill_fort_force;

    TINT64 raw_army_num;

    SCommonTroop raw_troop;
    SCommonFort raw_fort;
    SCommonTroop left_troop;
    SCommonFort left_fort;
    SCommonTroop dead_troop;
    SCommonFort dead_fort;
    SCommonTroop wounded_troop;
    SCommonFort wounded_fort;
    SMarchKnightInfo knight;
    TBOOL is_city;

    TINT64 load;
    SCommonResource resource;

    OneArmy()
    {
        memset(this, 0, sizeof(*this));
    }
    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
};

struct BattleUnit
{
    EArmsClass unit_class;
    TINT64 single_force;
    TFLOAT64 single_atk;
    TFLOAT64 single_hp;
    TFLOAT64 single_def;

    TINT64 raw_num;
    TINT64 left_num;

    TINT64 atk;
    TINT64 hp;
    TINT64 def;

    TINT64 raw_atk;
    TINT64 raw_hp;

    TINT64 kill_troop_num;
    TINT64 kill_troop_force;
    TINT64 kill_fort_num;
    TINT64 kill_fort_force;

    TFLOAT64 raw_rate;
    TFLOAT64 raw_atked_rate;
    TFLOAT64 raw_atk_rate;

    BattleUnit()
    {
        memset(this, 0, sizeof(*this));
    }
    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
};

struct Armys
{
    TINT64 raw_total_atk;
    TINT64 raw_total_def;
    TINT64 raw_troop_num;
    TINT64 raw_fort_num;
    TINT64 raw_army_num;
    TINT64 total_load;
    TINT64 total_dead_num;

    TINT64 kill_troop_num;
    TINT64 kill_troop_force;
    TINT64 kill_fort_num;
    TINT64 kill_fort_force;
    TINT64 dead_troop_num;
    TINT64 dead_troop_force;
    TINT64 dead_fort_num;
    TINT64 dead_fort_force;

    BattleBuff troop_buffs[EN_TROOP_TYPE__END];
    BattleBuff fort_buffs[EN_FORT_TYPE__END];
    
    BattleUnit units[EN_ARMS_CLS__END];

    TINT32 valid_units_list[EN_ARMS_CLS__END];
    TINT32 valid_units_num;

    TFLOAT64 tier_num_rate[MAX_ARMY_TIER_LIMIT];
    TFLOAT64 tier_atked_rate[MAX_ARMY_TIER_LIMIT];
    TINT64  tier_health[MAX_ARMY_TIER_LIMIT];

    std::map<ArmyId, OneArmy> actors;

    OneArmy *pstSelectArmyForProcessLeft;

    SCommonResource resource;

    TVOID Reset()
    {
        raw_total_atk = 0;
        raw_total_def = 0;
        raw_troop_num = 0;
        raw_fort_num = 0;
        raw_army_num = 0;
        total_load = 0;
        total_dead_num = 0;

        kill_troop_num = 0;
        kill_troop_force = 0;
        kill_fort_num = 0;
        kill_fort_force = 0;
        dead_troop_num = 0;
        dead_troop_force = 0;
        dead_fort_num = 0;
        dead_fort_force = 0;

        for(TUINT32 udwIdx = 0; udwIdx < EN_TROOP_TYPE__END; ++udwIdx)
        {
            troop_buffs[udwIdx].Reset();
        }
        for(TUINT32 udwIdx = 0; udwIdx < EN_FORT_TYPE__END; ++udwIdx)
        {
            fort_buffs[udwIdx].Reset();
        }
        for(TUINT32 udwIdx = 0; udwIdx < EN_ARMS_CLS__END; ++udwIdx)
        {
            units[udwIdx].Reset();
        }

        valid_units_num = 0;
        for(TUINT32 udwIdx = 0; udwIdx < MAX_ARMY_TIER_LIMIT; ++udwIdx)
        {
            tier_num_rate[udwIdx] = 0;
            tier_health[udwIdx] = 0;
            tier_atked_rate[udwIdx] = 0;
        }

        actors.clear();

        pstSelectArmyForProcessLeft = NULL;

        resource.Reset();
    }
};

struct SBattleNode
{
    Armys m_armys;

    //进攻方主action
    TbMarch_action* m_ptbMainAttackAction;

    //防守方支援驻军action
    TbMarch_action* m_pastEncampActionList[MAX_REINFORCE_NUM];
    TUINT32 m_udwEncampNum;

    //rallywar支援action
    TbMarch_action* m_pastRallyReinforceList[MAX_REINFORCE_NUM];
    TUINT32 m_udwRallyReinforceNum;

    //城市防守者
    SCityInfo *m_pstDefenderCity;
    TbPlayer *m_ptbDefendPlayer;
    SCommonTroop m_stDefenderCityWoundedTroop;
    SCommonFort m_stDefenderCityWoundedFort;

    //野地
    TbMap *m_ptbWild;
    TbIdol *m_ptbIdol;

    //leader
    SUserInfo *m_pstUser;
    TbPlayer *m_ptbPlayer;
    SCityInfo *m_pstCity;

    TBOOL m_bIsDragonJoin;
    SMarchDragonInfo m_stDragon;

    SMarchKnightInfo m_stKnight;

    TUINT32 m_udwSeriesId;

    //result
    TBOOL bAllTroopDead;

    SReportBuffer m_stReportBuffer;

    SPlayerBuffInfo m_stBattleBuff;

    TVOID Reset()
    {
        m_ptbMainAttackAction = NULL;
        m_udwEncampNum = 0;
        m_udwRallyReinforceNum = 0;
        m_pstDefenderCity = NULL;
        m_ptbDefendPlayer = NULL;
        m_ptbWild = NULL;
        m_pstUser = NULL;
        m_pstCity = NULL;
        m_bIsDragonJoin = FALSE;
        m_stDragon.Reset();
        m_armys.Reset();
        m_stKnight.Reset();

        m_udwSeriesId = 0;

        bAllTroopDead = FALSE;

        m_stReportBuffer.Reset();

        m_stBattleBuff.Reset();
    }
};

struct SAttackInfo
{
    TINT32 m_dwAttackType;
    TINT32 m_dwMultiAttackNum;
    TINT32 m_dwMultiAttackBuff;
    TINT64 m_ddwAttackDamage;   //表示累计输出
    TINT64 m_ddwRealDamage;
    SAttackInfo()
    {
        Reset();
    }

    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
};

struct SDragonNode
{
    // in
    TINT64 m_ddwAttack;         //基础攻击力
    TINT32 m_dwCanAttackTimes;//可攻打次数
    TINT32 m_dwBeginMultiAttackTimes;//此次攻击连击等级
    TINT32 m_dwMaxMultiAttackTimes; //玩家最大连击次数
    TINT32 m_dwCritChance;      //暴击概率 useless
    TINT32 m_dwCritBuff;        //暴击攻击力加成
    TINT32 m_dwRaidBuff;        //偷袭
    TINT32 m_dwHeavyBuff;       //重击
    TINT32 m_dwDoubleBuff;      //连击
    TINT32 m_dwMonsterDefenceReduce;//破防加成
    TINT32 m_dwEnergyCost;//能量消耗
    TINT32 m_dwBeginAlMultiAttackTimes;
    TINT32 m_dwAlMutiAttackExpire;
    TINT32 m_dwRawExp;
    TINT32 m_dwBMonsterHit;
    TINT32 m_dwBLeaderMonsterKillNum;

    //out
    TINT64 m_ddwRealDamageNum;
    TINT64 m_ddwTotalDamage;//造成总伤害
    TINT32 m_dwMultiAttackTimes;
    TINT32 m_dwMultiAttackBuff;
    TINT32 m_dwNextMultiAttackBuff;
    TINT32 m_dwAlMultiAttackTimes;
    TINT32 m_dwAlMultiAttackBuff;
    TINT32 m_dwRealAttackTimes;//真正攻击次数
    TINT32 m_dwExp;//增加经验值
    SAttackInfo m_astAttackInfo[MAX_ATTACK_TIMES];
    TINT32 m_dwHitToZero;
    TINT32 m_dwEMonsterHit;
    TINT32 m_dwELeaderMonsterKillNum;

    SGlobalRes m_stChallengerReward;

    SDragonNode()
    {
        Reset();
    }

    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
};


struct SMonsterNode
{
    // in
    TINT32 m_dwType;
    TINT32 m_dwLevel;
    TINT64 m_ddwMaxHp;
    TINT64 m_ddwRawHp;
    TINT64 m_ddwHp;
    TINT64 m_ddwDefence;
    TINT64 m_ddwLeader;

    // out
    TINT64 m_ddwLostHp;
    TBOOL m_bIsDead;

    SMonsterNode()
    {
        Reset();
    }

    TVOID Reset()
    {
        memset(this, 0, sizeof(*this));
    }
};

class CWarBase
{
public:
    //////////////////////////////////////////////////////////////////////////////////
    //获取action_list中在野地udwWildPos位置上驻扎或采集的action的index值，未找到时返回-1
    //////////////////////////////////////////////////////////////////////////////////////
    static TINT32 GetWildAction(TbMarch_action *patbMarchList, TUINT32 udwMarchNum, TUINT32 udwWildPos);

    static TVOID SetBattleBuff(SPlayerBuffInfo *pstBuff, SBattleNode *pstNode, TBOOL bIsCityDefender = FALSE, TBOOL bIsRallyAttacker = FALSE);

    static TVOID AddAttackArmy(TbMarch_action* ptbMarch, SBattleNode* pstNode);
    static TVOID AddCityArmy(SCityInfo* pstCity, SBattleNode* pstNode);
    static TVOID AddReinforceArmy(TbMarch_action* ptbMarch, SBattleNode* pstNode);
    static TVOID AddRallyReinforceArmy(TbMarch_action* ptbMarch, SBattleNode* pstNode);
    static TVOID AddWildArmy(TbMap* ptbWild, SBattleNode* pstNode);
    static TVOID AddWildOccupyArmy(TbMarch_action* ptbMarch, SBattleNode* pstNode);
    static TVOID AddWildCampArmy(TbMarch_action* ptbMarch, SBattleNode* pstNode);
    static TVOID AddIdolArmy(TbIdol *ptbIdol, SBattleNode* pstNode);

    //static TVOID PrepareDefendBattleUnits(SBattleNode* pstDefender);
    //static TVOID PrepareAttackBattleUnits(const SBattleNode* pstDefender, SBattleNode* pstAttacker);

    //static TVOID LoadAttackOrder(AttackOrders& attack_orders);
    //static TVOID LoadTroopAttackOrder(TUINT32 udwSourceTroopId, AttackOrders& attack_orders);
    //static TVOID LoadFortAttackOrder(TUINT32 udwSourceFortId, AttackOrders& attack_orders);

    //static TVOID UnitsFightUnits(const AttackOrders& attack_orders, BattleUnit* atk_units, BattleUnit* def_units);
    //static TVOID FightOneRound(AttackOrders& attack_orders, SBattleNode* pstAttacker, SBattleNode* pstDefender);
    //static TVOID FightOneRoundForDefender(AttackOrders& attack_orders, SBattleNode* pstDefender, SBattleNode* pstAttacker);

    static TVOID ComputeCasualty(SBattleNode* pstNode);
    static TVOID ComputeScore(SBattleNode* pstNode);

public://wave@20160612
    static TVOID PrepareBattleUnits(SBattleNode* pstNode);
    static TVOID ComputeTotalAttack(SBattleNode* pstDefender, SBattleNode* pstAttacker);

    static TVOID FightOneRound(SBattleNode* pstAttacker, SBattleNode* pstDefender);
    static TVOID UnitsFightUnits(SBattleNode* pstAttacker, SBattleNode* pstDefender);

public:
    static TVOID RecordBattleNode(SBattleNode* pstNode, Json::Value& record);
    static TVOID RecordArmy(OneArmy* army, Json::Value& record);
    static TVOID RecordUnits(BattleUnit* units, TUINT32 unit_num, Json::Value& record);
    static TVOID RecordBuff(BattleBuff* buffs, TUINT32 buff_num, Json::Value& record);

    template<typename T>
    static TVOID ArrayOutput(T *NumNode, TUINT32 udwNodeNum, Json::Value &jNode);

    static TVOID GetTotalTroop(const SBattleNode &stNode, SCommonTroop *pTroop);
    static TVOID GetTotalFort(const SBattleNode &stNode, SCommonFort *pFort);
    static TVOID GetRawTotalTroop(const SBattleNode &stNode, SCommonTroop *pRawTroop);
    static TVOID GetRawTotalFort(const SBattleNode &stNode, SCommonFort *pRawFort);

    static TVOID SelectBattleNode(TUINT32 udwTotalSelectNum, SCommonTroop *pstRawTroop, SCommonFort* pstRawFort, SCommonTroop *pstSelectTroop, SCommonFort *pstSelectFort);

    static TVOID ComputeTotalLoad(SBattleNode *pstNode);
    static TVOID DistributeResource(SBattleNode *pstNode, SCommonResource *pstResource);

public:
    //英雄 打怪 相关
    static TVOID SetDragonNode(SUserInfo *pstUser, TbMarch_action *ptbReqAction, TbMap *ptbWild, SDragonNode *pstHeroNode);
    static TVOID SetMonsterNode(TbMap *ptbWild, SMonsterNode *pstMonsterNode);

    static TINT32 GetEnergyCost(SUserInfo *pstUser, TbMarch_action *ptbReqAction, TbMap *ptbWild);

    static TVOID DragonNodeOutput(SDragonNode *pstNode, Json::Value &jNode);
    static TVOID MonsterNodeOutput(SMonsterNode *pstNode, Json::Value &jNode);
    static TVOID MonsterRewardOutput(TbMarch_action *ptbReqAction, Json::Value &jNode);

    static TINT32 SetDragonResult(TbMarch_action *ptbReqAction, SUserInfo *pstUser, TbMap *ptbWild, SDragonNode *pstHeroNode, SMonsterNode *pstMonsterNode);

    static TINT32 SetMonsterResult(TbMarch_action *ptbReqAction, TbPlayer *ptbPlayer, TbPlayer *ptbChallenger, TbMap *ptbWild, SDragonNode *pstHeroNode, SMonsterNode *pstMonsterNode);

    static TINT32 SetMonsterRewardResult(SUserInfo *pstUser, TbMarch_action *ptbReqAction, TbMap *ptbWild, TbAl_gift *ptbAlGift, SMonsterNode *pstMonsterNode, SDragonNode *pstHero);

    static TINT32 SetChallengerResult(SUserInfo *pstUser, SMonsterNode *pstMonsterNode, SDragonNode *pstDragon);

    static TINT32 SetMarchReward(TbMarch_action *ptbReqAction, const Json::Value& jReward);
    
    static TINT32 NewSetMarchReward(TbMarch_action *ptbReqAction, const vector<SOneGlobalRes *> &vecReward);
    static TINT32 NewSetChallengerReward(SUserInfo *pstChallenger, SDragonNode *pstDragon, const vector<SOneGlobalRes *> &vecReward);
    static TVOID GetDropRewardResult(SMonsterNode *pstMonsterNode, TBOOL &bNormalReward, TBOOL &bEliteReward);

public:
    static TFLOAT64 m_atk2def[EN_ARMS_CLS__END][EN_ARMS_CLS__END];
};

#endif //_WAR_BASE_H_
