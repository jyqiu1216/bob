#ifndef _GAME_DATA_STRUCTURE_H_
#define _GAME_DATA_STRUCTURE_H_

#include "base/common/wtse_std_header.h"
#include "game_define.h"
#include <string>

using namespace std;

#define	MAX_BUILDING_SPECAIL_UPGRADE_NUM	(1)
#define MAX_PROP_PARAM_NUM					(3)
#define MAX_REWARD_TROOP_TYPE_NUM			(2)
#define MAX_REWARD_PROP_TYPE_NUM			(3)

///////////////////////////////////////////////////////////////////
// 基础数据结构
///////////////////////////////////////////////////////////////////
struct SUpgradeSpecialProp
{
    TUINT32 m_udwId;
    TUINT32 m_udwNum;
};

struct SItemProperty
{
    TUINT32 m_udwPrice;
    TUINT32 m_udwCategory;
    TUINT32 m_udwFuction;
};

struct SRewardInfo
{
    // resource
    TUINT32 m_audwResource[EN_RESOURCE_TYPE__END];

    // troop
    TUINT32 m_udwTroopTypeNum;
    TUINT32 m_audwTroopId[MAX_REWARD_TROOP_TYPE_NUM];
    TUINT32 m_audwTroopNum[MAX_REWARD_TROOP_TYPE_NUM];

    // item
    TUINT32 m_udwItemTypeNum;
    TUINT32 m_audwItemId[MAX_REWARD_PROP_TYPE_NUM];
    TUINT32 m_audwItemNum[MAX_REWARD_PROP_TYPE_NUM];

    // special
    TUINT32 m_udwGemNum;
    TUINT32 m_udwMight;
    TUINT32 m_udwUserLevel;
};

/////////////////////////////////////////////////

//TODO  所有相关的都修改掉!!!
struct SBuildingInfo
{
    // 升级需要的基础消耗
    TINT64 m_addwBaseCost[EN_BASE_COST_TYPE__END];
    // 升级需要的特殊物品
    TUINT8 m_ucUpradeNeedItem;
    SUpgradeSpecialProp m_stSpecialItem;

    SBuildingInfo()
    {
        Reset();
    }

    TVOID Reset()
    {
        memset((char*)this, 0, sizeof(*this));
    }
};

struct SResearchInfo
{
    // 升级需要的基础消耗
    TUINT32 m_audwBaseCost[EN_BASE_COST_TYPE__END];
    TVOID Reset()
    {
        memset((char*)this, 0, sizeof(*this));
    }
};

struct STroopInfo
{
    // 需要的基础消耗
    TUINT32 m_audwBaseCost[EN_BASE_COST_TYPE__END];
    // 需要消耗的物品类型数
    TUINT32 m_udwRelyNum;
    // 需要消耗的物品id和数量
    SUpgradeSpecialProp m_astItemCost[MAX_TRAIN_TROOP_RELY_ITEM_TYPE];

    //wave@20160612
    // flag
    TBOOL m_bValid;
    // info
    TINT32  m_dwId;
    TFLOAT64  m_fHealth;
    TFLOAT64  m_fAttack;
    TFLOAT64  m_fDefense;
    TINT32  m_dwSpeed;
    TINT32  m_dwLoad;
    TINT32  m_dwCategory;
    TINT32  m_dwTier;
    TBOOL   m_bTrainable;
    TINT32  m_dwUpkeep;
    TINT32  m_dwMight;

    TVOID Reset()
    {
        memset((char*)this, 0, sizeof(*this));
    }
};

struct SItemInfo
{
    // 基本属性
    TUINT32 m_audwProperty[EN_PROP_STATUS__END];
    TVOID Reset()
    {
        memset((char*)this, 0, sizeof(*this));
    }
};

struct SQuestInfo
{
    // 完成奖励
    SRewardInfo m_stReward;
    TVOID Reset()
    {
        memset((char*)this, 0, sizeof(*this));
    }
};

struct SWildInfo
{
    // resource
    TUINT32 m_audwResource[EN_RESOURCE_TYPE__END];

    // troop
    TUINT32 m_audwTroop[EN_TROOP_TYPE__END];

    // research
    TUINT8	m_aucResearch[EN_RESEARCH_TYPE__END];
};

struct Restrain
{
    double atk_factor;
};

struct AttackOrders
{
    Restrain orders[EN_ARMS_CLS__END][EN_ARMS_CLS__END];
};

#endif
