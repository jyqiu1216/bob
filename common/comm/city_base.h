/************************************************************
文件名称: city_base.h
作者: daemon
日期: 2014.11.05
模块描述: city模块基础接口。
*************************************************************/

#ifndef _CITY_BASE_H_
#define _CITY_BASE_H_

#include "base/common/wtse_std_header.h"
#include "game_define.h"
#include "aws_table_city.h"
#include "aws_table_action.h"
#include "city_info.h"
#include "player_info.h"

class CCityBase
{
//troop fort
public:
    /************************************************************
    功能说明：判断玩家是否有足够的troop或者fort
    参数说明：ucType = 0 判断Troop 是否足够 ucType =1 判断fort是否足够
                        paudwTroop 要判断的troop/fort数组
                        udwTypeNum paudwTroop含有的元素数量
    *************************************************************/
	static TBOOL HasEnoughTroop(SCityInfo *pstCity, TUINT8 ucType, TINT64 *addwTroop, TUINT32 udwTypeNum);

    /************************************************************
    功能说明：获得城市中的troop fort
    *************************************************************/
    static TVOID CalcCityTroopAndFort(SCityInfo *pstCity, TINT64 *addwTroop, TINT64 *addwFort);

    static TVOID AddTroop(SCityInfo *pstCity, TUINT32 udwTroopIdx, TINT64 ddwTroopNum);
    static TVOID SetTroop(SCityInfo *pstCity, TUINT32 udwTroopIdx, TINT64 ddwTroopNum);

    static TVOID AddFort(SCityInfo *pstCity, TUINT32 udwTroopIdx, TINT64 ddwTroopNum);
    static TVOID SetFort(SCityInfo *pstCity, TUINT32 udwTroopIdx, TINT64 ddwTroopNum);

    static TVOID CostFort(SCityInfo *pstCity, TUINT32 udwTroopId, TINT64 ddwNum);

//resource
public:
    /************************************************************
    功能说明：判断玩家是否有足够的资源
    参数说明：paudwResCost 需要判断的资源数组
    *************************************************************/
	static TBOOL HasEnoughResource(SCityInfo *pstCity, TINT64 *addwResCost);

    /************************************************************
    功能说明：判断玩家是否有足够的gold
    *************************************************************/
	static TBOOL HasEnoughGold(SCityInfo *pstCity, TINT64 ddwPrice);

    /************************************************************
    功能说明：消耗玩家资源 在调用该函数时 一定要先判断玩家资源是否足够
    参数说明：paudwResCost 需要消耗的资源数组
    *************************************************************/
	static TVOID CostResource(SCityInfo *pstCity, TINT64 *addwResCost);

    /************************************************************
    功能说明：消耗gold 调用前 需判定是否有足够的gold
    *************************************************************/
	static TVOID CostGold(SCityInfo *pstCity, TINT64 ddwPrice);

    /************************************************************
    功能说明：增加资源
    参数说明：paudwResouce 增加的资源数组
    *************************************************************/
	static TVOID AddResource(SCityInfo *pstCity, TINT64 *addwResource);
    static TVOID AddFood(SCityInfo *pstCity, TINT64 ddwNum);
    static TVOID AddWood(SCityInfo *pstCity, TINT64 ddwNum);
    static TVOID AddStone(SCityInfo *pstCity, TINT64 ddwNum);
    static TVOID AddOre(SCityInfo *pstCity, TINT64 ddwNum);
    static TVOID AddResource(SCityInfo *pstCity, TUINT32 udwId, TINT64 ddwNum);
    static TVOID SetResource(SCityInfo *pstCity, TUINT32 udwId, TINT64 ddwNum);
    static TVOID CostOneResource(SCityInfo *pstCity, TUINT32 udwResourceId, TINT64 ddwResourceNum);

    /************************************************************
    功能说明：增加gold
    *************************************************************/
    static TVOID AddGold(SCityInfo *pstCity, TINT64 ddwNum);

//building
public:
    /************************************************************
    功能说明：获取建筑的等级 如果有多座建筑 返回最大等级
    *************************************************************/
    static TUINT8 GetBuildingLevelByFuncType(SCityInfo *pstCity, TUINT32 udwFuncType);

    static TUINT8 GetBuildingIdByFuncType(TbCity *pstCity, TUINT32 udwFuncType);

    /************************************************************
    功能说明：获取建筑的等级 如果有多座建筑 返回最大等级
    *************************************************************/
    static TUINT8 GetBuildingLevelById(TbCity *pstCity, TUINT32 udwBuildId);

    /************************************************************
    功能说明：获取ucType类型的建筑的数量
    *************************************************************/
    static TUINT8 GetBuildingNumByFuncType(SCityInfo *pstCity, TUINT32 udwFuncType);

    static TUINT8 GetBuildingNumById(SCityInfo *pstCity, TUINT32 udwBuildId);

    static TINT32 GetBuildingFuncType(TUINT32 udwBuildingId);

    static SCityBuildingNode* GetBuildingAtPos(TbCity* ptbCity, TUINT32 udwPos);

    static TINT32 DelBuildingAtPos(TbCity* ptbCity, TUINT32 udwPos);

    static TINT32 AddBuilding(TUINT32 udwPos, TUINT32 ucType, TUINT8 ucLevel, TbCity& tbCity);

    static TUINT8 GetBuildingNumByLvAndType(SCityInfo *pstCity, TUINT32 udwFuncType, TUINT32 udwLv);

    static TUINT32 GetBuildingLimitLv(TUINT32 ucType);

    static TUINT32 GetBuildingNumByCategory(TbCity *pstCity, TUINT32 udwCategory);
    static TBOOL HasBuildCategoryCapacity(SUserInfo *pstUser, TUINT32 udwBuildId);

//hos
public:
    /************************************************************
    功能说明：计算医院中和治疗中的伤兵数量
    *************************************************************/
    static TINT64 GetHosTroopNum(SCityInfo *pstCity, TbAlliance_action* patbAction, TUINT8* paucFlag, TUINT32 udwActionNum);

    /************************************************************
    功能说明：计算医院中和治疗中的伤兵数量
    *************************************************************/
    static TINT64 GetHosFortNum(SCityInfo *pstCity, TbAlliance_action* patbAction, TUINT8* paucFlag, TUINT32 udwActionNum);

//troop fort might
public:
	/************************************************************
	功能说明：根据troop数组计算might值
	*************************************************************/
	static TINT64 CalcTroopMight(TINT64 *addwTroop);

	/************************************************************
	功能说明：根据fort数组计算might值
	*************************************************************/
	static TINT64 CalcFortMight(TINT64 *addwFort);

//quest
public:
    static TBOOL HasChangeCityName(SCityInfo* pstCity);

    static TVOID CostTroop(SCityInfo *pstCity, TUINT32 udwTroopId, TINT64 ddwNum);

    static TBOOL HasEnoughResource(SCityInfo *pstCity, TUINT32 udwId, TINT64 ddwNum);

    static TBOOL HasEnoughTroop(SCityInfo *pstCity, TUINT8 ucType, TUINT32 udwId, TINT64 ddwNum);

public:
    static TINT64 GetHosCurNum(SCityInfo *pstCity);
    static TVOID ComputeDeadTroopMight(TbMarch_action *ptbMarch, TINT64& ddwDeadMight, TINT64& ddwDeadNum);

public:
    static TINT32 AddKnightExp(SCityInfo* pstCity, TUINT32 udwTargetKnight, TUINT32 udwAddExp);
};

#endif