/************************************************************
文件名称: production_system.h
作者: daemon
日期: 20014.11.07
模块描述: 生产系统。
*************************************************************/

#ifndef _PRODUCTION_SYSTEM_H_
#define _PRODUCTION_SYSTEM_H_

#include "base/common/wtse_std_header.h"
#include "game_define.h"
#include "city_base.h"
#include "city_info.h"
#include "player_info.h"

class CProductionSystem
{
public:
    /************************************************************
    功能说明：动态计算城市相关数据，包括：人口 happiness 资源
    参数说明：udwTime 为城市信息更新时间
    *************************************************************/
    static TVOID ComputeProductionSystem(SUserInfo *pstUser, SCityInfo *pstCity, TUINT32 udwTime);

    //生产系统
private:
    /************************************************************
    功能说明：计算玩家的基础生产率 受人口的影响（不包含bonous）
    若人口不足 只按当前拥有人口计算
    *************************************************************/
    static TVOID ComputeBaseProduction(SCityInfo *pstCity,SUserInfo *pstUser);

    /************************************************************
    功能说明：计算城市在udwTime时间内生产的资源量 udwTime*(production-upkeep)
    *************************************************************/
    static TINT32 ComputeCityResource(SCityInfo *pstCity, SUserInfo *pstUser, TUINT32 udwTime);

    /************************************************************
    功能说明：计算城市消耗资源量
    *************************************************************/
    static TVOID ComputeCityResUpkeep(SCityInfo *pstCity, SUserInfo *pstUser);

    /************************************************************
    功能说明：计算城市资源生产率 (包含bonous)
    *************************************************************/
    static TVOID ComputeCityProduction(SCityInfo *pstCity, SUserInfo *pstUser);

    /************************************************************
    功能说明：计算当前的生产率 ： 基础生产率+bonous
    *************************************************************/
    static TVOID ComputeTotalProduction(SCityInfo *pstCity,SUserInfo *pstUser);

	static TVOID ComputeCityResCapacity(SCityInfo *pstCity, SUserInfo *pstUser);

//bonus
private:
    /************************************************************
    功能说明：计算科技对生产的加成
    *************************************************************/
	static TVOID ComputeResearchBonus(SCityInfo *pstCity,SUserInfo *pstUser);

    /************************************************************
    功能说明：计算道具对生产的加成
    *************************************************************/
	static TVOID ComputeItemBonus(SCityInfo *pstCity,SUserInfo *pstUser);

    /************************************************************
    功能说明：计算其他对生产的加成
    *************************************************************/
	static TVOID ComputeOtherBonus(SCityInfo *pstCity,SUserInfo *pstUser);

    /************************************************************
    功能说明：计算英雄(技能加装备)对生产的加成
    *************************************************************/
    static TVOID ComputeDragonBonus(SCityInfo *pstCity, SUserInfo *pstUser);

    /************************************************************
    功能说明：计算vip对生产的加成
    *************************************************************/
	static TVOID ComputeVipBonus(SCityInfo *pstCity, SUserInfo *pstUser);

    static TVOID ComputeLordBonus(SCityInfo *pstCity, SUserInfo *pstUser);

private:
    static TVOID ComputeActionStat(SUserInfo *pstUser, SCityInfo *pstCity);

    //static TINT64 GetDragonUpkeep(TINT64 ddwLevel);
public:
    static TUINT32 ComputeKnightUnassignTime(SUserInfo *pstUser, SCityInfo *pstCity);

};

#endif