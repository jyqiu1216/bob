#ifndef _ACTION_BASE_H_
#define _ACTION_BASE_H_


#include "game_data.h"
#include "aws_table_include.h"
#include "bussiness_struct.h"
#include "player_info.h"

class CActionBase
{
public:
    static TINT32 GetAlActionIndex(TbAlliance_action* patbAlAction, TUINT32 udwActionNum, TINT64 ddwTargetActionId);
    static TbAlliance_action* GetAlAction(TbAlliance_action* patbAlAction, TUINT32 udwActionNum, TINT64 ddwTargetActionId);
    static TINT32 AddAlAction(SUserInfo *pstUser, SCityInfo*pstCity, TUINT8 enMainClass, TUINT8 enSecClass, TUINT8 enStatus, TUINT32 udwCostTime,
        UActionParam *pstParam, TUINT32 udwTargetPos = 0, TUINT64 uddwNewTaskId = 0Lu, TINT32 dwSvrId = -1);

    static TINT32 GetMarchIndex(TbMarch_action* patbMacrh, TUINT32 udwMarchNum, TINT64 ddwTargetActionId);
    static TbMarch_action* GetMarch(TbMarch_action* patbMacrh, TUINT32 udwMarchNum, TINT64 ddwTargetActionId);
    static TbMarch_action* AddMarchAction(SUserInfo *pstUser, SCityInfo*pstCity, TUINT8 enMainClass, TUINT8 enSecClass, TUINT8 enStatus, TUINT32 udwCostTime,
        SActionMarchParam* pstParam, TUINT64 uddwNewTaskId, TUINT32 udwTargetId);

    static TINT32 GetActionIndex(TbAction* patbAction, TUINT32 udwActionNum, TINT64 ddwTargetActionId);
    static TbAction* GetAction(TbAction* patbAction, TUINT32 udwActionNum, TINT64 ddwTargetActionId);

    static TbAction* GetActionByBufferId(TbAction* patbAction, TUINT32 udwActionNum, TUINT32 udwBufferId);

    static TBOOL IsPlayerOwnedAction(TINT64 ddwUserId, TINT64 ddwActionId);

    static TVOID UpdtActionWhenLeaveAlliance(SUserInfo *pstUser, TINT64 ddwSid, TINT64 ddwSrcUid);

    static TVOID UpdtPassiveActionWhenAbandonWild(SUserInfo *pstUser, TbMap *pstWild);

public:
    static TINT32 GetUserBuffById(TbMarch_action *ptbMarch, TINT32 dwBuffId, TUINT32 udwTime = 0);

    static TINT64 GetActionTypeBySecondClass(TUINT32 udwSecondClass);

    //rally war
    static TBOOL HasEmptySlot(TbMarch_action* ptbRallyWar, TINT64 ddwUid);
    static TVOID HoldSlot(TbMarch_action* ptbRallyWar, TbMarch_action* ptbRallyReinforce);
    static TVOID ReleaseSlot(TbMarch_action* ptbRallyWar, TbMarch_action* ptbRallyReinforce, TBOOL bForceRelease = FALSE);
    static TVOID AddSlot(TbMarch_action* ptbRallyWar, TINT64 ddwUid, const string& strUserName);
    static TBOOL HasPrivateSlot(TbMarch_action* ptbRallyWar, TINT64 ddwUid);

    static TVOID UpdateRallyForce(TbMarch_action* ptbRallyWar, TbMarch_action* ptbRallyReinforce, TBOOL bRecall = FALSE);
    static TBOOL CanRallyReinforce(TbMarch_action* ptbRallyWar, const TCHAR* pszTroop);

    static TBOOL IsEmptyMarch(TbMarch_action* ptbMarch);
    static TBOOL IsMarchHasTroop(TbMarch_action* ptbMarch);
    static TVOID DeleteMarch(TbMarch_action *ptbMarch);
    static TVOID ReturnMarch(TbMarch_action *ptbMarch, TINT64 ddwWildPos = 0, TINT64 ddwWildType = -1);
    static TVOID ReturnMarchOnFly(TbMarch_action *ptbMarch);
    static TVOID ReinforceToThrone(TbMarch_action* ptbMarch, TINT64 ddwWildPos, TINT64 ddwWildType);
    static TVOID RallyWarToThroneAssign(TbMarch_action* ptbMarch);

    static TVOID PrisonToMarch(TbMarch_action* ptbPrisonTimer, TUINT32 udwScid = 0);

    static TINT32 CheckSeq(SUserInfo* pstUser);
    static TVOID SyncDragonStatus(SUserInfo* pstUser);

    static TbMarch_action* AddNewMarch(SUserInfo* pstUser);

    static TbMarch_action* GenTaxAction(SUserInfo *pstUser, TINT64 ddwThronePos);

    static TINT32 GetActionDisplayClass(TINT32 dwMainClass);

    static TVOID ResetRallyDefence(TbMarch_action *ptbRally, TbCity *ptbCity);
    static TBOOL RallyInfoAddDefence(TbMarch_action *ptbRally, TbMarch_action *ptbDefence);

public:
    static TINT64 GenMapActionId(TINT64 ddwSvrId, TINT64 ddwPos);
    static TINT64 GenThroneTargetId(TINT64 ddwSvrId, TINT64 ddwPos);
};
#endif
