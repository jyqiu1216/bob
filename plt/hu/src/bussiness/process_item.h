#ifndef _PROCESS_ITEM_H_
#define _PROCESS_ITEM_H_


#include "session.h"

class CProcessItem
{

public: 
    /**************************道具的购买和使用相关操作**********************************/  
    // function  ===> 使用物品(道具特效)
    static TINT32 ProcessCmd_ItemUse(SSession *pstSession, TBOOL& bNeedResponse);
    // function  ===> 购买物品
    static TINT32 ProcessCmd_ItemBuy(SSession *pstSession, TBOOL& bNeedResponse);
    // function  ===> 购买并使用物品
    static TINT32 ProcessCmd_ItemBuyAndUse(SSession *pstSession, TBOOL& bNeedResponse);

    static TINT32 ProcessCmd_OpenAllChest(SSession *pstSession, TBOOL& bNeedResponse);

    /*************************单独接口使用特殊道具相关的操作******************************/  
    // function  ===> 使用随机移城道具(todo: 统一使用以上三个接口是最好的)
    static TINT32 ProcessCmd_RandomMoveCity(SSession *pstSession, TBOOL &bNeedResponse);
    // function  ===> 使用确定位置的移城道具(todo: 统一使用以上三个接口是最好的)
    static TINT32 ProcessCmd_MoveCity(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 ProcessCmd_MoveCityPrepare(SSession *pstSession, TBOOL &bNeedResponse);
};



#endif
