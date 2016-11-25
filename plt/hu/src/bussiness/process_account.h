#ifndef _PROCESS_ACCOUNT_H_
#define _PROCESS_ACCOUNT_H_

#include "session.h"

class CProcessAccount
{
public: 

    /***********************************充值相关的操作************************************/  
    // function  ==> 客户端正常的重置操作
    static TINT32 ProcessCmd_GemRecharge(SSession *pstSession, TBOOL &bNeedResponse); 

    /***********************************游戏服的相关的操作************************************/  
    // function  ==> 获取游戏服的信息
    static TINT32 ProcessCmd_SvrInfoGet(SSession *pstSession, TBOOL &bNeedResponse);
    
    /***********************************token相关的操作************************************/  
    // function  ==> 更新token的相关信息
    static TINT32 ProcessCmd_ApnsUpdate(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ==> 新版更新token的信息
    static TINT32 ProcessCmd_ApnsToken(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ==> 新版更新推送开关
    static TINT32 ProcessCmd_ApnsSwitch(SSession *pstSession, TBOOL &bNeedResponse);



    static TINT32 AddPromoteReward(SSession *pstSession, const Json::Value &jReward);

    static TUINT32 GetPurchaseAbility(TUINT32 udwGemNum);
    static TUINT32 GetIapPay(string sItemId);
    static TUINT32 GetIapPayCent(string sItemId);
};

#endif
