#ifndef _PROCESS_ACTION_H_
#define _PROCESS_ACTION_H_


#include "session.h"


class CProcessAction
{
public: 
    /******************************ͨ��action�����صĲ���************************************/  
    // function  ===> action��gem���ٲ���
    static TINT32 ProcessCmd_ActionGemSpeedUp(SSession *pstSession, TBOOL &bNeedResponse);
    // function  ===> action��free���ٲ���
    static TINT32 ProcessCmd_ActionFreeSpeedUp(SSession *pstSession, TBOOL &bNeedResponse);
    // function  ===> action��ȡ������
    static TINT32 ProcessCmd_ActionCancel(SSession *pstSession, TBOOL &bNeedResponse);

    /*********************************troop��fort����صĲ���************************************/    
    // function  ===> ȡ��troop��ѵ��
    static TINT32 ProcessCmd_TroopCancel(SSession *pstSession, TBOOL &bNeedResponse);
    // function  ===> ȡ��fort��ѵ��
    static TINT32 ProcessCmd_FortCancel(SSession *pstSession, TBOOL &bNeedResponse); 
    // function  ===> ȡ��ҽԺ��troop������
    static TINT32 ProcessCmd_HospitalTroopTreatCancel(SSession *pstSession, TBOOL &bNeedResponse);
    // function  ===> ȡ��ҽԺ��fort������
    static TINT32 ProcessCmd_FortRepairCancel(SSession *pstSession, TBOOL &bNeedResponse);

    /***********************************�����ͿƼ�����ز���***************************************/
    // function  ===> �����ͿƼ���ȡ����������
    //EN_ACTION_MAIN_CLASS__BUILDING
    static TINT32 BuildingActionCancel(SSession *pstSession, SUserInfo *pstUser, SCityInfo *pstCity, EActionSecClass enSecClass, SActionBuildingParam *pstParam);
    // function  ===> �����ͿƼ���������ɴ���(todo:�Ż��ɻ�������)
    //EN_ACTION_MAIN_CLASS__BUILDING
    static TINT32 BuildingActionDone(SSession *pstSession, SUserInfo *pstUser, SCityInfo *pstCity, EActionSecClass enSecClass, SActionBuildingParam *pstParam, TBOOL bCbLog = FALSE);

    /******************************ѵ��troop��fort����ز���*********************************/
    // function  ===> ѵ��troop��fort��������ɴ���(todo:�Ż��ɻ�������)
    static TINT32 TrainActionDoneNew(SSession *pstSession, SUserInfo *pstUser, SCityInfo *pstCity, EActionSecClass enSecClass, TUINT32 udwActionIdx);

    /********************************ѵ��troop��fort����ز���***********************************/
    // function  ===> troop��fort�ĵ�ȡ��ѵ������
    static TINT32 TrainCancelNew(SSession *pstSession, SUserInfo *pstUser, SCityInfo *pstCity, TbAlliance_action* ptbAction);

    /********************************action����ز���*****************************************/  
    // function  ==> action�����(todo: �д���ȡ)
    static TVOID ActionDone(SSession *pstSession, SUserInfo *pstUser, SCityInfo *pstCity, TUINT32 udwMainClass, TUINT32 udwSecClass, TINT32 dwActionIdx, TBOOL bCbLog);

    static TINT32 EquipUpgradeActionDone(SSession *pstSession, SUserInfo *pstUser, SCityInfo *pstCity, SActionEquipParam *pstParam, TBOOL bCbLog);

    static TINT32 EquipActionCancel(SSession *pstSession, SUserInfo *pstUser, SCityInfo *pstCity, EActionSecClass enSecClass, SActionEquipParam *pstParam);

};



#endif
