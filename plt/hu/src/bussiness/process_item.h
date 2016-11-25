#ifndef _PROCESS_ITEM_H_
#define _PROCESS_ITEM_H_


#include "session.h"

class CProcessItem
{

public: 
    /**************************���ߵĹ����ʹ����ز���**********************************/  
    // function  ===> ʹ����Ʒ(������Ч)
    static TINT32 ProcessCmd_ItemUse(SSession *pstSession, TBOOL& bNeedResponse);
    // function  ===> ������Ʒ
    static TINT32 ProcessCmd_ItemBuy(SSession *pstSession, TBOOL& bNeedResponse);
    // function  ===> ����ʹ����Ʒ
    static TINT32 ProcessCmd_ItemBuyAndUse(SSession *pstSession, TBOOL& bNeedResponse);

    static TINT32 ProcessCmd_OpenAllChest(SSession *pstSession, TBOOL& bNeedResponse);

    /*************************�����ӿ�ʹ�����������صĲ���******************************/  
    // function  ===> ʹ������Ƴǵ���(todo: ͳһʹ�����������ӿ�����õ�)
    static TINT32 ProcessCmd_RandomMoveCity(SSession *pstSession, TBOOL &bNeedResponse);
    // function  ===> ʹ��ȷ��λ�õ��Ƴǵ���(todo: ͳһʹ�����������ӿ�����õ�)
    static TINT32 ProcessCmd_MoveCity(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 ProcessCmd_MoveCityPrepare(SSession *pstSession, TBOOL &bNeedResponse);
};



#endif
