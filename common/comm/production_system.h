/************************************************************
�ļ�����: production_system.h
����: daemon
����: 20014.11.07
ģ������: ����ϵͳ��
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
    ����˵������̬�������������ݣ��������˿� happiness ��Դ
    ����˵����udwTime Ϊ������Ϣ����ʱ��
    *************************************************************/
    static TVOID ComputeProductionSystem(SUserInfo *pstUser, SCityInfo *pstCity, TUINT32 udwTime);

    //����ϵͳ
private:
    /************************************************************
    ����˵����������ҵĻ��������� ���˿ڵ�Ӱ�죨������bonous��
    ���˿ڲ��� ֻ����ǰӵ���˿ڼ���
    *************************************************************/
    static TVOID ComputeBaseProduction(SCityInfo *pstCity,SUserInfo *pstUser);

    /************************************************************
    ����˵�������������udwTimeʱ������������Դ�� udwTime*(production-upkeep)
    *************************************************************/
    static TINT32 ComputeCityResource(SCityInfo *pstCity, SUserInfo *pstUser, TUINT32 udwTime);

    /************************************************************
    ����˵�����������������Դ��
    *************************************************************/
    static TVOID ComputeCityResUpkeep(SCityInfo *pstCity, SUserInfo *pstUser);

    /************************************************************
    ����˵�������������Դ������ (����bonous)
    *************************************************************/
    static TVOID ComputeCityProduction(SCityInfo *pstCity, SUserInfo *pstUser);

    /************************************************************
    ����˵�������㵱ǰ�������� �� ����������+bonous
    *************************************************************/
    static TVOID ComputeTotalProduction(SCityInfo *pstCity,SUserInfo *pstUser);

	static TVOID ComputeCityResCapacity(SCityInfo *pstCity, SUserInfo *pstUser);

//bonus
private:
    /************************************************************
    ����˵��������Ƽ��������ļӳ�
    *************************************************************/
	static TVOID ComputeResearchBonus(SCityInfo *pstCity,SUserInfo *pstUser);

    /************************************************************
    ����˵����������߶������ļӳ�
    *************************************************************/
	static TVOID ComputeItemBonus(SCityInfo *pstCity,SUserInfo *pstUser);

    /************************************************************
    ����˵�������������������ļӳ�
    *************************************************************/
	static TVOID ComputeOtherBonus(SCityInfo *pstCity,SUserInfo *pstUser);

    /************************************************************
    ����˵��������Ӣ��(���ܼ�װ��)�������ļӳ�
    *************************************************************/
    static TVOID ComputeDragonBonus(SCityInfo *pstCity, SUserInfo *pstUser);

    /************************************************************
    ����˵��������vip�������ļӳ�
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