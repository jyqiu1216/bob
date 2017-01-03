#ifndef _PROCESS_ACCOUNT_H_
#define _PROCESS_ACCOUNT_H_

#include "session.h"

class CProcessAccount
{
public: 

    /***********************************��ֵ��صĲ���************************************/  
    // function  ==> �ͻ������������ò���
    static TINT32 ProcessCmd_GemRecharge(SSession *pstSession, TBOOL &bNeedResponse); 

    /***********************************��Ϸ������صĲ���************************************/  
    // function  ==> ��ȡ��Ϸ������Ϣ
    static TINT32 ProcessCmd_SvrInfoGet(SSession *pstSession, TBOOL &bNeedResponse);
    
    /***********************************token��صĲ���************************************/  
    // function  ==> ����token�������Ϣ
    static TINT32 ProcessCmd_ApnsUpdate(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ==> �°����token����Ϣ
    static TINT32 ProcessCmd_ApnsToken(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ==> �°�������Ϳ���
    static TINT32 ProcessCmd_ApnsSwitch(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 AddPromoteReward(SSession *pstSession, const Json::Value &jReward);
};

#endif
