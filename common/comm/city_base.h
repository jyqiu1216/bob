/************************************************************
�ļ�����: city_base.h
����: daemon
����: 2014.11.05
ģ������: cityģ������ӿڡ�
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
    ����˵�����ж�����Ƿ����㹻��troop����fort
    ����˵����ucType = 0 �ж�Troop �Ƿ��㹻 ucType =1 �ж�fort�Ƿ��㹻
                        paudwTroop Ҫ�жϵ�troop/fort����
                        udwTypeNum paudwTroop���е�Ԫ������
    *************************************************************/
	static TBOOL HasEnoughTroop(SCityInfo *pstCity, TUINT8 ucType, TINT64 *addwTroop, TUINT32 udwTypeNum);

    /************************************************************
    ����˵������ó����е�troop fort
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
    ����˵�����ж�����Ƿ����㹻����Դ
    ����˵����paudwResCost ��Ҫ�жϵ���Դ����
    *************************************************************/
	static TBOOL HasEnoughResource(SCityInfo *pstCity, TINT64 *addwResCost);

    /************************************************************
    ����˵�����ж�����Ƿ����㹻��gold
    *************************************************************/
	static TBOOL HasEnoughGold(SCityInfo *pstCity, TINT64 ddwPrice);

    /************************************************************
    ����˵�������������Դ �ڵ��øú���ʱ һ��Ҫ���ж������Դ�Ƿ��㹻
    ����˵����paudwResCost ��Ҫ���ĵ���Դ����
    *************************************************************/
	static TVOID CostResource(SCityInfo *pstCity, TINT64 *addwResCost);

    /************************************************************
    ����˵��������gold ����ǰ ���ж��Ƿ����㹻��gold
    *************************************************************/
	static TVOID CostGold(SCityInfo *pstCity, TINT64 ddwPrice);

    /************************************************************
    ����˵����������Դ
    ����˵����paudwResouce ���ӵ���Դ����
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
    ����˵��������gold
    *************************************************************/
    static TVOID AddGold(SCityInfo *pstCity, TINT64 ddwNum);

//building
public:
    /************************************************************
    ����˵������ȡ�����ĵȼ� ����ж������� �������ȼ�
    *************************************************************/
    static TUINT8 GetBuildingLevelByFuncType(SCityInfo *pstCity, TUINT32 udwFuncType);

    static TUINT8 GetBuildingIdByFuncType(TbCity *pstCity, TUINT32 udwFuncType);

    /************************************************************
    ����˵������ȡ�����ĵȼ� ����ж������� �������ȼ�
    *************************************************************/
    static TUINT8 GetBuildingLevelById(TbCity *pstCity, TUINT32 udwBuildId);

    /************************************************************
    ����˵������ȡucType���͵Ľ���������
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
    ����˵��������ҽԺ�к������е��˱�����
    *************************************************************/
    static TINT64 GetHosTroopNum(SCityInfo *pstCity, TbAlliance_action* patbAction, TUINT8* paucFlag, TUINT32 udwActionNum);

    /************************************************************
    ����˵��������ҽԺ�к������е��˱�����
    *************************************************************/
    static TINT64 GetHosFortNum(SCityInfo *pstCity, TbAlliance_action* patbAction, TUINT8* paucFlag, TUINT32 udwActionNum);

//troop fort might
public:
	/************************************************************
	����˵��������troop�������mightֵ
	*************************************************************/
	static TINT64 CalcTroopMight(TINT64 *addwTroop);

	/************************************************************
	����˵��������fort�������mightֵ
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