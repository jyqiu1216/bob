#ifndef _PROCESS_MARCH_H_
#define _PROCESS_MARCH_H_


#include "session.h"

class CProcessMarch
{
public: 
    // function  ===> scout
    static TINT32 ProcessCmd_MarchScout(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ===> 发起普通的attack
    static TINT32 ProcessCmd_MarchAttack(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ===> transport
    static TINT32 ProcessCmd_MarchTransport(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ===> 普通的reinforce
    static TINT32 ProcessCmd_MarchReinforceNormal(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ===> 把驻扎在其他地方的兵recall回来
    static TINT32 ProcessCmd_Recall(SSession *pstSession, TBOOL &bNeedResponse);
    
    // function  ===> occupy
    static TINT32 ProcessCmd_MarchOccupy( SSession *pstSession, TBOOL &bNeedResponse );
    
    static TINT32 ProcessCmd_MarchDragonAttack(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ===> 发起rally war
    static TINT32 ProcessCmd_RallyAttack(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ===> 发起rally war reinforce
    static TINT32 ProcessCmd_RallyReinforce(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 ProcessCmd_RallyReinforceRecall(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 ProcessCmd_RallyDismiss(SSession* pstSession, TBOOL &bNeedResponse);

    static TINT32 ProcessCmd_RallySlotBuy(SSession* pstSession, TBOOL &bNeedResponse);

    static TINT32 ProcessCmd_RallyInfo(SSession* pstSession, TBOOL &bNeedResponse);

    static TINT32 ProcessCmd_RallyHistory(SSession* pstSession, TBOOL &bNeedResponse);

    static TINT32 ProcessCmd_ManorAbandon(SSession* pstSession, TBOOL &bNeedResponse);

    static TINT32 ProcessCmd_Repatriate(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 ProcessCmd_RecallAllReinforce(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 ProcessCmd_SendbackAllReinforce(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 ProcessCmd_MarchCamp(SSession *pstSession, TBOOL &bNeedResponse);

public:
    static TINT32 ProcessCmd_IdolAttack(SSession *pstSession, TBOOL &bNeedResponse);

public:
    static TINT32 ProcessCmd_ThroneAttack(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_ThroneRallyWar(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_ThroneReinforce(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_ThroneDispatch(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_ThroneDismissKnightDragon(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_ThroneDismissReinforce(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_ThroneDismissAll(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_ThroneRecallKnightDragon(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_ThroneRecallReinforce(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_ThroneReinforceSpeedup(SSession *pstSession, TBOOL &bNeedResponse);

public:
    static TINT32 ProcessCmd_ReinforceSpeedup(SSession *pstSession, TBOOL &bNeedResponse);


private:
    //check
    static TINT32 CheckMarchSelfCondition(SUserInfo *pstUser, TbPlayer *ptbPlayer, SCityInfo *pstCity, TCHAR *szTroop, 
        TCHAR *szSendResource, TCHAR *szTaxResource, TINT32 dwGem, TINT32 dwKnightId, TBOOL bIsDragonJoin, TBOOL bNeedBreakPeaceTime, TINT32 dwMarchType);

    static TINT32 CheckMarchOtherCondition(SUserInfo *pstUser, TbPlayer *ptbPlayer, TbMap *ptbTargetMap, TINT32 dwMarchType, TINT64 ddwRallyId = 0);



    // gen action
    static TbMarch_action* GenMarchAction(SUserInfo *pstUser, TbPlayer *ptbPlayer, SCityInfo *pstCity, TCHAR *szTroop,
        TCHAR *szSendResource, TCHAR *szTaxResource, TINT32 dwKnightId, TBOOL bIsDragonJoin, TbMap *ptbTargetMap, 
        TINT32 dwMarchType, TUINT32 udwCostTime, TBOOL bIfMaxAttack = FALSE);

    // gen rally action
    static TbMarch_action* GenRallyAction(SUserInfo *pstUser, TbPlayer *ptbPlayer, SCityInfo *pstCity, TCHAR *szTroop, TINT32 dwKnightId,
        TBOOL bIsDragonJoin, TbMap *ptbTargetMap, TINT32 dwMarchType, TUINT32 udwCostTime, TUINT32 udwPrepareTime);

    // CHECK rally war
    static TINT32 CheckCanRallyWar(SUserInfo *pstUser, TUINT32 udwTargetPos, TUINT32 udwAlid);
};



#endif
