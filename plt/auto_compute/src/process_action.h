#ifndef _PROCESS_ACTION_H_
#define _PROCESS_ACTION_H_

#include "session.h"

class CProcessAction
{
public:
    static TINT32 ProcessUidAction(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessAidAction(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessMarchAction(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 SendDataCenterRequest(SSession *pstSession, TUINT16 uwReqServiceType);
    
private:
    static TINT32 ProcessActionTrainNew(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessActionItem(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessActionAttackMove(SSession *pstSession, TBOOL &bNeedResponse);

private:
    static TINT32 ProcessActionBuilding(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessActionEquip(SSession *pstSession, TBOOL &bNeedResponse);

private:
    static TINT32 ProcessActionDragon(SSession *pstSession, TBOOL &bNeedResponse);
private:
    static TINT32 ProcessMarch(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 ProcessMarchReturn(SSession *pstSession);
    static TINT32 ProcessMarchTransport(SSession *pstSession);
    static TINT32 ProcessMarchReinForce(SSession *pstSession);
    static TINT32 ProcessMarchAttack(SSession *pstSession);
    static TINT32 ProcessMarchOccupy(SSession *pstSession);
    static TINT32 ProcessMarchScout(SSession *pstSession);
    static TINT32 ProcessMarchFighting(SSession *pstSession);
    static TINT32 ProcessMarchLoadingRes(SSession *pstSession);
    static TINT32 ProcessMarchSetupCamp(SSession *pstSession);
    static TINT32 ProcessMarchPreLoading(SSession *pstSession);
    static TINT32 ProcessMarchUnLoading(SSession *pstSession);
    static TINT32 ProcessMarchCamp(SSession *pstSession);
    static TINT32 ProcessMarchCampSetupCampPeace(SSession *pstSession);
    static TINT32 ProcessMarchCampSetupCampNormal(SSession *pstSession);
    static TINT32 ProcessMarchCampSetupCampReturn(SSession *pstSession);

    static TINT32 ProcessMarchDragonAttack(SSession *pstSession);
    static TINT32 ProcessMarchDragonFighting(SSession *pstSession);

    static TINT32 ProcessMarchRallyReinforce(SSession* pstSession);
    static TINT32 ProcessMarchRallyAttack(SSession* pstSession);
    static TINT32 ProcessMarchRallyFighting(SSession* pstSession);
    static TINT32 ProcessMarchRallyPreparing(SSession* pstSession);
    static TINT32 ProcessMarchRallyDeling(SSession* pstSession);

    static TINT32 ProcessMarchIdolAttack(SSession *pstSession);
    static TINT32 ProcessMarchIdolFighting(SSession *pstSession);

    static TINT32 ProcessTimer(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 ProcessThroneTimer(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessPrisonTimer(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessNotiTimer(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 ProcessTaxPreparing(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessTaxCollecting(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessTaxFinish(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessTaxTransfer(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 ProcessIdolThronePeaceTimePeriod(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessIdolBuffPeriod(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessIdolQuietPeriod(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessIdolContestPeriod(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 ProcessThronePeaceTimePeriod(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessThroneContestPeriod(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 ProcessMarchThroneAssign(SSession *pstSession);
    static TINT32 ProcessMarchThroneReinforce(SSession *pstSession);
    static TINT32 ProcessMarchThroneAttack(SSession *pstSession);
    static TINT32 ProcessMarchThroneRallyWar(SSession *pstSession);
    static TINT32 ProcessMarchThroneFighting(SSession *pstSession);
    static TINT32 ProcessMarchThroneRallyFighting(SSession *pstSession);
    static TINT32 ProcessMarchRallyThronePreparing(SSession* pstSession);

    static TINT32 ProcessMarchThroneReinforceAttack(SSession *pstSession);
    static TINT32 ProcessMarchThroneReinforceDefend(SSession *pstSession);

    static TINT32 ProcessAuPress(SSession *pstSession, TBOOL &bNeedResponse);
};

#endif //_PROCESS_ACTION_H_
