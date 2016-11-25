#ifndef _PROCESS_ACTION_H_
#define _PROCESS_ACTION_H_


#include "session.h"


class CProcessAction
{
public: 
    /******************************通用action类的相关的操作************************************/  
    // function  ===> action的gem加速操作
    static TINT32 ProcessCmd_ActionGemSpeedUp(SSession *pstSession, TBOOL &bNeedResponse);
    // function  ===> action的free加速操作
    static TINT32 ProcessCmd_ActionFreeSpeedUp(SSession *pstSession, TBOOL &bNeedResponse);
    // function  ===> action的取消操作
    static TINT32 ProcessCmd_ActionCancel(SSession *pstSession, TBOOL &bNeedResponse);

    /*********************************troop和fort的相关的操作************************************/    
    // function  ===> 取消troop的训练
    static TINT32 ProcessCmd_TroopCancel(SSession *pstSession, TBOOL &bNeedResponse);
    // function  ===> 取消fort的训练
    static TINT32 ProcessCmd_FortCancel(SSession *pstSession, TBOOL &bNeedResponse); 
    // function  ===> 取消医院里troop的治疗
    static TINT32 ProcessCmd_HospitalTroopTreatCancel(SSession *pstSession, TBOOL &bNeedResponse);
    // function  ===> 取消医院里fort的治疗
    static TINT32 ProcessCmd_FortRepairCancel(SSession *pstSession, TBOOL &bNeedResponse);

    /***********************************建筑和科技的相关操作***************************************/
    // function  ===> 建筑和科技的取消升级处理
    //EN_ACTION_MAIN_CLASS__BUILDING
    static TINT32 BuildingActionCancel(SSession *pstSession, SUserInfo *pstUser, SCityInfo *pstCity, EActionSecClass enSecClass, SActionBuildingParam *pstParam);
    // function  ===> 建筑和科技的升级完成处理(todo:优化成基础函数)
    //EN_ACTION_MAIN_CLASS__BUILDING
    static TINT32 BuildingActionDone(SSession *pstSession, SUserInfo *pstUser, SCityInfo *pstCity, EActionSecClass enSecClass, SActionBuildingParam *pstParam, TBOOL bCbLog = FALSE);

    /******************************训练troop和fort的相关操作*********************************/
    // function  ===> 训练troop和fort的升级完成处理(todo:优化成基础函数)
    static TINT32 TrainActionDoneNew(SSession *pstSession, SUserInfo *pstUser, SCityInfo *pstCity, EActionSecClass enSecClass, TUINT32 udwActionIdx);

    /********************************训练troop和fort的相关操作***********************************/
    // function  ===> troop和fort的的取消训练处理
    static TINT32 TrainCancelNew(SSession *pstSession, SUserInfo *pstUser, SCityInfo *pstCity, TbAlliance_action* ptbAction);

    /********************************action的相关操作*****************************************/  
    // function  ==> action的完成(todo: 有待提取)
    static TVOID ActionDone(SSession *pstSession, SUserInfo *pstUser, SCityInfo *pstCity, TUINT32 udwMainClass, TUINT32 udwSecClass, TINT32 dwActionIdx, TBOOL bCbLog);

    static TINT32 EquipUpgradeActionDone(SSession *pstSession, SUserInfo *pstUser, SCityInfo *pstCity, SActionEquipParam *pstParam, TBOOL bCbLog);

    static TINT32 EquipActionCancel(SSession *pstSession, SUserInfo *pstUser, SCityInfo *pstCity, EActionSecClass enSecClass, SActionEquipParam *pstParam);

};



#endif
