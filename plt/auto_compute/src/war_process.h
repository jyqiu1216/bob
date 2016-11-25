#ifndef _WAR_PROCESS_H_
#define _WAR_PROCESS_H_

#include "session.h"
#include "player_info.h"
#include "war_base.h"

class CWarProcess
{
public:
    static TINT32 GetBattleType(TINT32 dwSecClass, TINT32 dwWildType, TINT32 dwWildUid, TINT32 dwSid);

    static TINT32 ProcessWar(SSession *pstSession, SBattleNode *pstAttackerNode, SBattleNode *pstDefenderNode, TbReport *ptbMarchReport, TUINT32 udwAttackType = EN_BATTLE_TYPE__NO_OP);

    //设置战斗双方信息
    static TVOID SetBattleNode(SSession *pstSession, SBattleNode *pstAttacker, SBattleNode *pstDefender, TUINT32 udwAttackType);

    static TVOID SetReportBuffer(SSession *pstSession, SBattleNode *pstAttacker, SBattleNode *pstDefender, TUINT32 udwAttackType);
    static TBOOL IsDefenderInCity(TUINT32 udwAttackType);
    static TVOID AddBuffer(SBuffInfo *pstBuff, TUINT32& udwBuffNum, TUINT32 udwMaxBuffNum, TUINT32 udwAddId, TUINT32 udwAddNum);

    //调用warbase 处理战斗
    static TUINT32 ProcessAttack(SBattleNode *pstAttacker, SBattleNode *pstDefender);

    //输出战斗结果
    static TVOID OutputBattleNode(SBattleNode *pstNode);

    //战斗造成的影响
    static TVOID ComputeBattleResult(SSession *pstSession, SBattleNode *pstAttacker, SBattleNode *pstDefender, TUINT32 udwBattleResult, TUINT32 udwAttackType);

public:
    //英雄打怪
    static TINT32 ProcessWar(SSession *pstSession, SDragonNode *pstDragonNode, SMonsterNode *pstMonsterNode);
    static TUINT32 ProcessAttack(SDragonNode *pstDragonNode, SMonsterNode *pstMonsterNode);
    static TVOID ComputeBattleResult(SSession *pstSession, SDragonNode *pstDragonNode, SMonsterNode *pstMonsterNode);
    static TVOID ComputeWarResult(SSession *pstSession, SBattleNode *pstAttacker, SBattleNode *pstDefender, TUINT32 udwResultCode, TUINT32 udwHostNum);

private:
    static TVOID SetBattleNodeForAttackCity(SSession *pstSession, SBattleNode *pstAttacker, SBattleNode *pstDefender);
    static TVOID SetBattleNodeForAttackOccupy(SSession *pstSession, SBattleNode *pstAttacker, SBattleNode *pstDefender);
    static TVOID SetBattleNodeForAttackCamp(SSession *pstSession, SBattleNode *pstAttacker, SBattleNode *pstDefender);
    static TVOID SetBattleNodeForAttackWild(SSession *pstSession, SBattleNode *pstAttacker, SBattleNode *pstDefender);
    static TVOID SetBattleNodeForRallyCity(SSession *pstSession, SBattleNode *pstAttacker, SBattleNode *pstDefender);
    static TVOID SetBattleNodeForRallyThrone(SSession * pstSession, SBattleNode * pstAttacker, SBattleNode * pstDefender);
    static TVOID SetBattleNodeForAttackOccupyLair(SSession *pstSession, SBattleNode *pstAttacker, SBattleNode *pstDefender);

    static TVOID SetBattleNodeForAttackIdol(SSession *pstSession, SBattleNode *pstAttacker, SBattleNode *pstDefender);

    ////////////////////////////////////////////////////////////////////////////////////
    //将请求req_action中的军队信息添加到战斗所需的war_node的army当中，用于战斗数据的组成
    //PS:req_action和reinforce_action的处理不同
    ///////////////////////////////////////////////////////////////////////////////
    static TINT32 SetLeadAttackerArmy(TbMarch_action *ptbReqMarch, SUserInfo *pstUserInfo, SBattleNode *pstAttacker);

    //////////////////////////////////////////////////////////////////////////////////////
    //设置防守方在野地驻扎或采集资源时的军队数据，添加到战斗所需的war_node的army当中
    ///////////////////////////////////////////////////////////////////////////////////////
    static TINT32 SetWildDefenderArmy(SUserInfo *pstUserInfo, SBattleNode *pstDefender, TUINT32 udwWildPos);

    //////////////////////////////////////////////////////////////////////////////////////
    //设置防守方在野地驻扎或采集资源时的军队数据，添加到战斗所需的war_node的army当中
    ///////////////////////////////////////////////////////////////////////////////////////
    static TINT32 SetWildCampArmy(SUserInfo *pstUserInfo, SBattleNode *pstDefender, TUINT32 udwWildPos);

    //////////////////////////////////////////////////////////////////////////////////////
    //设置防守方为野地（tribe，battle_field等）时的军队数据，添加到战斗所需的war_node的army当中
    ///////////////////////////////////////////////////////////////////////////////////////
    static TINT32 SetWildDefenderNode(TbMap *ptbWildDefender, SBattleNode *pstDefender);

    //////////////////////////////////////////////////////////////////////////////////
    //将防守方主城里面的军队信息添加到defender_node中，并同时添加到war_node，用于战斗数据的组成
    //////////////////////////////////////////////////////////////////////////////////////
    static TINT32 SetCityDefenderArmy(SUserInfo *pstUserInfo, SBattleNode *pstDefender);

    //////////////////////////////////////////////////////////////////////////////////
    //在城市防守时，将普通reinforce的军队加入到defender_node的encamp中
    //////////////////////////////////////////////////////////////////////////////////////
    static TINT32 SetCityReinforceArmy(SUserInfo *pstUserInfo, SBattleNode *pstDefender);

    //////////////////////////////////////////////////////////////////////////////////
    //在王座防守时，将驻扎的军队加入到defender_node的encamp中
    //////////////////////////////////////////////////////////////////////////////////////
    static TINT32 SetThroneReinforceArmy(SUserInfo *pstUserInfo, SBattleNode *pstDefender, TUINT32 udwWildPos);

    //////////////////////////////////////////////////////////////////////////////////
    //在occupy巢穴时，将驻扎occupy的军队加入到defender_node中
    //////////////////////////////////////////////////////////////////////////////////////
    static TINT32 SetOccupyLairDefenderArmy(SUserInfo *pstUser, SBattleNode *pstDefender, TbMarch_action *ptbReqMarch, TbMap *ptbWild);

    //////////////////////////////////////////////////////////////////////////////////
    //在rallywar时，将进攻reinforce的军队加入到attacker_node中
    //////////////////////////////////////////////////////////////////////////////////////
    static TINT32 SetRallyReinforceArmy(SUserInfo *pstUserInfo, TbMarch_action* ptbRallyWar, SBattleNode *pstBattleNode);

    //////////////////////////////////////////////////////////////////////////////////
    //在王座防守时，将驻扎的英雄加入defender_node中
    //////////////////////////////////////////////////////////////////////////////////////
    static TINT32 SetLeaderAssign(SUserInfo *pstUserInfo, SBattleNode *pstDefender, TUINT32 udwWildPos);

    static TINT32 SetIdolDefenderNode(TbIdol *ptbIdolDefender, SBattleNode *pstDefender);

private:
    static TVOID OutputBattleNode(SBattleNode *pstNode, TbMarch_action *ptbMarch);
    static TVOID OutputBattleNode(SBattleNode *pstNode, SCityInfo *pstCity, TbPlayer *ptbPlayer);
    static TVOID OutputBattleNode(SBattleNode *pstNode, TbMap *ptbWild);

private:
    static TVOID ComputeAttackCityResult(SSession *pstSession, SBattleNode *pstAttacker, SBattleNode *pstDefender, TUINT32 udwBattleResult);
    static TVOID ComputeAttackOccupyResult(SSession *pstSession, SBattleNode *pstAttacker, SBattleNode *pstDefender, TUINT32 udwBattleResult);
    static TVOID ComputeAttackCampResult(SSession *pstSession, SBattleNode *pstAttacker, SBattleNode *pstDefender, TUINT32 udwBattleResult);
    static TVOID ComputeAttackWildResult(SSession *pstSession, SBattleNode *pstAttacker, SBattleNode *pstDefender, TUINT32 udwBattleResult);
    static TVOID ComputeRallyCityResult(SSession *pstSession, SBattleNode *pstAttacker, SBattleNode *pstDefender, TUINT32 udwBattleResult);
    static TVOID ComputeRallyThroneResult(SSession *pstSession, SBattleNode *pstAttacker, SBattleNode *pstDefender, TUINT32 udwBattleResult);
    static TVOID ComputeAttackOccupyLairResult(SSession *pstSession, SBattleNode *pstAttacker, SBattleNode *pstDefender, TUINT32 udwBattleResult);

    static TVOID ComputeAttackIdolResult(SSession *pstSession, SBattleNode *pstAttacker, SBattleNode *pstDefender, TUINT32 udwBattleResult);

    static TVOID AddDragonExp(SUserInfo *pstUser, SCityInfo *pstCity, SBattleNode *pstAttacker, TUINT32 udwBattleResult);

    static TINT32 ComputeCityDead(SUserInfo *pstUserInfo, SBattleNode *pstDefender, TINT64 ddwAlterId);
    static TINT32 RobResourceFromCity(TbMarch_action *ptbReqMarch, SBattleNode *pstAttacker, SBattleNode *pstDefender, TUINT32 udwBattleResult);
    static TINT32 ReturnEncampAction(SBattleNode *pstDefender, TBOOL bIsForceReturn = FALSE);

    static TINT32 ProcessOccupyWild(SUserInfo *pstSUser, SUserInfo *pstTarget, TbMarch_action *ptbReqMarch, SBattleNode *pstAttacker, SBattleNode *pstDefender, TbMap *ptbWild, TUINT32 udwBattleResult);
    static TINT32 ProcessOccupyLair(SUserInfo *pstSUser, SUserInfo *pstTarget, TbMarch_action *ptbReqMarch, SBattleNode *pstAttacker, SBattleNode *pstDefender, TbMap *ptbWild, TUINT32 udwBattleResult);
    static TINT32 ComputeWildResult(TbMap *ptbWild, SBattleNode *pstDefender, TUINT32 udwBattleResult);
    static TINT32 ComputeWildResult(TbMarch_action *ptbMarch, TbMap *ptbWild, TUINT32 udwBattleResult);
    static TINT32 GenWildReward(TbMarch_action *ptbMarch, TbMap *ptbWild, TUINT32 udwBattleResult);
    static TVOID CalcAttackCampAction(SSession *pstSession, SBattleNode *pstAttacker, SBattleNode *pstDefender, TUINT32 udwBattleResult);

    static TVOID SetThroneOnFight(SSession* pstSession, TbMap* ptbWild);
    static TVOID ActiveIdol(SSession* pstSession, TUINT32 udwActiveTime);
    static TVOID ControlThrone(TbAlliance* ptbSAlliance, TbThrone *ptbThrone, TbMap* ptbWild);

    static TINT32 ProcessRallyReinforce(SBattleNode *pstNode, TINT64 ddwWildPos, TINT64 ddwWildType, TBOOL bIsForceReturn = FALSE, TBOOL bToControl = FALSE);
    static TVOID ReturnThroneAssign(SUserInfo* pstTUser, TbMap * ptbWild);
    static TINT32 ReturnNoTroopReinforce(SBattleNode *pstNode, TINT64 ddwWildPos, TINT64 ddwWildType);

    static TINT32 SetKnightResult(TbMarch_action *ptbMarchAction, SBattleNode *pstNode, TUINT32 udwBattleResult);
    static TINT32 SetDefenderKnightResult(SUserInfo *pstUser, SBattleNode *pstNode, TUINT32 udwBattleResult, TUINT32 udwWildPos);

    static TINT32 SetIdolResult(TbIdol *ptbIdol, TbPlayer *ptbPlayer, TUINT32 udwBattleResult);

private:
    //英雄抓捕逻辑
    static TVOID RescueDragon(SSession *pstSession, SUserInfo* pstCaptor, TbPlayer* ptbSaver);
    static TVOID CaptureDragon(SSession *pstSession, SBattleNode *pstAttacker, SBattleNode *pstDefender, TUINT32 udwBattleResult);
    static TVOID CaptureDragonWild(SSession *pstSession, SBattleNode *pstAttacker, SBattleNode *pstDefender, TUINT32 udwBattleResult);
    static TINT32 IsCaptureSucc(SUserInfo* pstUserA, SBattleNode *pstWinner, SBattleNode *pstLoser);//-1表示没抓到, 0表示能够处死, 1表示自身等级不足, 2表示对方等级不足

    static TVOID CaptureDragonThrone(SSession *pstSession, SBattleNode *pstAttacker, SBattleNode *pstDefender, TUINT32 udwBattleResult);
private:
    static TVOID GenWarLog(SSession *pstSession, SBattleNode *pstAttacker, SBattleNode *pstDefender, const TCHAR *pszCmd, TUINT8 ucType, TUINT32 udwResultCode);
    static TVOID GenDefenderKeys(SBattleNode *pstAttacker, SBattleNode *pstDefender, TbMarch_action *ptbMarch, TCHAR *pszOut, TUINT32 &udwOutLen, TUINT32 udwResultCode);
    static TVOID GenAttackerKeys(SBattleNode *pstAttacker, SBattleNode *pstDefender, TbMarch_action *ptbMarch, TCHAR *pszOut, TUINT32 &udwOutLen, TUINT32 udwResultCode);
    static TVOID GenAllyEncampActionLog(SSession *pstSession, SBattleNode *pstNode, const TCHAR *pszCmd, TUINT32 udwResultCode);
    static TVOID GenAllyEncampActionKeys(SBattleNode *pstNode, TUINT32 idx, TCHAR *pszOut, TUINT32 &udwOutLen);

public:
    static TVOID GenMoveCityAction(SSession *pstSession, TbMap *ptbWild, SBattleNode *pstDefend);

    // 更新巢穴拉取的队列etime
    static TVOID UpdateOccupyLairAction(SSession *pstSession, SUserInfo *pstUser, TbMap *ptbWild);

    // 选择最高级龙/骑士的用户作为巢穴occupy主用户
    static TVOID UpdateOccupyLairMap(SSession *pstSession, SUserInfo *pstUser, TbMap *ptbWild);
};

#endif //_WAR_PROCESS_H_
