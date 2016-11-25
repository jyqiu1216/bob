#ifndef _PROCESS_OPERATE_H_
#define _PROCESS_OPERATE_H_

#include "session.h"
#include "npc_upgrade.h"

#define MAX_LEVEL (20)

// 运营命令字接口除了能被主流程调用之外，不允许被别的模块调用
class CProcessOperate
{
public: 
    static TINT32 ProcessCmd_LoginGet(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_OperateLog(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_GemRecharge(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_AddPersonAlGift(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_AddAllianceAlGift(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_SvrChange(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 ProcessCmd_UserUpdate(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_PlayerName(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_CityName(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_AddTroop(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_AddFort(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_ClearUserResource(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_AddResource(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_GemAdd(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_ItemAdd(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_ItemSet(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_AllianceMemberGet(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_AllianceInfoGet(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_AllianceChangeChancellor(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_ResearchLevelSet(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_BuildingLevelSet(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_IdentityChange(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_StatisticalOpenAlGiftWave(SSession *pstSession, TBOOL &bNeedResponse);
    // 召回流程测试
    static TINT32 ProcessCmd_RecallTest(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 Processcmd_AddRewardList(SSession* pstSession, TBOOL &bNeedResponse);
    static TINT32 Processcmd_AddRewardListNew(SSession* pstSession, TBOOL &bNeedResponse);
    static TINT32 Processcmd_AddAlRewardList(SSession* pstSession, TBOOL &bNeedResponse);
    static TINT32 Processcmd_AddAlRewardListNew(SSession* pstSession, TBOOL &bNeedResponse);
    static TINT32 Processcmd_AddThemeRewardList(SSession* pstSession, TBOOL &bNeedResponse);
    static TINT32 Processcmd_AddThemeAlRewardList(SSession* pstSession, TBOOL &bNeedResponse);

    static TINT32 Processcmd_AddLoyaltyAndFund(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 Processcmd_ListBuildingPos(SSession* pstSession, TBOOL& bNeedResponse);

    //恢复玩家的城市在世界地图上的信息
    static TINT32 ProcessCmd_RecoverPlayerCity(SSession *pstSession, TBOOL &bNeedResponse);
    //只删除map表上多余的数据
    static TINT32 ProcessCmd_CheckPlayerMap(SSession *pstSession, TBOOL &bNeedResponse);
    //指定服务器id 删除因为跨服移城遗留的map
    static TINT32 ProcessCmd_CheckPlayerSvrMap(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 ProcessCmd_ClearUser(SSession* pstSession, TBOOL& bNeedResponse);
    static TINT32 ProcessCmd_BlockHacker(SSession* pstSession, TBOOL& bNeedResponse);

    //只删除map表上多余的数据
    static TVOID CheckPlayerMap(SSession *pstSession, SUserInfo *pstUser, TbMap *ptbWildList, const TUINT32 udwWildNum);
    //恢复玩家的城市在世界地图上的信息
    static TVOID RecoverPlayerCity(SSession *pstSession, SUserInfo *pstUser, TbMap *ptbWildList, const TUINT32 udwWildNum);

    static TINT32 ResetMap(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 ClearNoPlayerMap(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 ProcessCmd_HuPressureMeasure(SSession* pstSession, TBOOL& bNeedResponse);

    static TINT32 ProcessCmd_AuPressureMeasure(SSession* pstSession, TBOOL& bNeedResponse);

    static TINT32 ProcessCmd_CleanAuPressureAction(SSession* pstSession, TBOOL& bNeedResponse);

    static TINT32 Processcmd_AccountOperate(SSession* pstSession, TBOOL& bNeedResponse);
    static TINT32 ProcessCmd_CacheTest(SSession* pstSession, TBOOL& bNeedResponse);
    
    static TINT32 ProcessCmd_WarningTest(SSession *pstSession, TBOOL &bNeedResponse);
    
    static TINT32 ProcessCmd_CreateNewPlayerAlliance(SSession* pstSession, TBOOL& bNeedResponse);

    static TINT32 ProcessCmd_SendRallyPridict(SSession* pstSession, TBOOL& bNeedResponse);

    static TINT32 ProcessCmd_SetHelpBubble(SSession* pstSession, TBOOL& bNeedResponse);
    
    static TINT32 ProcessCmd_TranslateTest(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 ProcessGenTaxAction(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessSetIapKey(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 ProcessCmd_ChangeSvr(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 Processcmd_AddRewardListForOp(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 ProcessCmd_SetDragonFlag(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 ProcessCmd_RecoveryDeadPlayer(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 ProcessCmd_ProSysGetData(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 ProcessCmd_GenAttackMoveAction(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 ProcessCmd_GenIdol(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 ProcessCmd_GenThrone(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 ProcessCmd_UpgradeNpcToLv10(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 ProcessCmd_GetRank(SSession *pstSession, TBOOL &bNeedResponse);

private:
    static TBOOL TbPlayer_Compare(const TbPlayer *pstA, const TbPlayer *pstB);
};

#endif
