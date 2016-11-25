#ifndef _MAP_LOGIC_H_
#define _MAP_LOGIC_H_

#include "game_info.h"
#include "aws_table_map.h"
#include "action_base.h"
#include "game_define.h"
#include "bussiness_struct.h"

#include "time_utils.h"

class CMapLogic
{
public:
    //�ж��Ƿ�Ϊ����ȡ����
    static TBOOL IsOccupyWild(TINT32 dwSvrId, TINT32 dwWildType);

    //�ж��Ƿ�Ϊ����ȡ����
    static TBOOL IsAttackWild(TINT32 dwSvrId, TINT32 dwWildType);

    //�ж��Ƿ�ΪӢ�ۿɹ������
    static TBOOL IsDragonAttackWild(TINT32 dwSvrId, TINT32 dwWildType);

    //�ж��Ƿ�ΪӢ�ۿ���ȡ����
    static TBOOL IsDragonOccupyWild(TINT32 dwSvrId, TINT32 dwWildType);

    //��ȡmap����Դ����
    static TUINT32 GetMapResTotalNum(TbMap *ptbWild);

    //��ȡ��ȡʱ��
    static TINT32 ComputeLoadResTime(TbMarch_action *ptbMarch, TbMap *ptbWild);

    static TINT32 GetWildClass(TINT32 dwSvrId, TINT32 dwWildType);

    //��ȡ��Դ(������ϲ����)
    static TINT32 GetCollectedReward(SUserInfo *pstUser,TbMarch_action *ptbMarch, TbMap *ptbWild, TINT32 dwBeginTime, TINT32 dwLoadTime, TINT64 &ddwLoadNum);

    static TBOOL HaveCollectedOut(SUserInfo *pstUser, TbMarch_action *ptbMainMarch, TbMap *ptbWild);

    static TINT32 GetAttackReward(TbMarch_action *ptbMarch, TbMap *ptbWild);

    static TVOID GenWildItemReward(TbMap* ptbWild, TbMarch_action* ptbReqMarch,TUINT32 udwTime = 1);
    static TBOOL GenConfirmWildItemReward(TbMap* ptbWild, TbMarch_action* ptbReqMarch,TUINT32 udwTime = 1);
    static TBOOL GenRandomWildItemReward(TbMap* ptbWild, TbMarch_action* ptbReqMarch,TUINT32 udwTime = 1);
    static TBOOL GenOneWildItemReward(TbMap* ptbWild, TbMarch_action* ptbReqMarch,TUINT32 udwTime = 1);

    static TVOID GenSurpriseResult(TbMap* ptbWild, TbMarch_action* ptbMarch);
    static TBOOL GenSurpriceConfirmWildItemReward(TbMap* ptbWild, TbMarch_action* ptbReqMarch);
    static TBOOL GenSurpriceRandomWildItemReward(TbMap* ptbWild, TbMarch_action* ptbReqMarch);
    static TBOOL GenSurpriceOneWildItemReward(TbMap* ptbWild, TbMarch_action* ptbReqMarch);

    static TINT32 GetIdxById(TUINT32 udwId, TbMap *patbMap, TUINT32 udwNum);
    static TBOOL IsPosCanMove(TUINT32 udwPos, TbMap *patbMap, TUINT32 udwNum, TUINT32 udwTypeBlockNum, TUINT32 udwSid);
    static TBOOL IsPosCanMove(TbMap *patbMap);

    static TBOOL GenMosterAllianceGift(SUserInfo *pstUser, TbMap *ptbWild, TbMarch_action *ptbReqAction);

    static TVOID GenStageResult(TbMap* ptbWild, TbMarch_action* ptbMarch, TINT32 dwLoadTime);

    static TINT32 GetResTypeByWildType(TUINT32 udwSid, TINT32 dwWildType, TUINT32 udwLv);

    static TBOOL IfPlayerCity(SUserInfo *pstUser, TbMap *ptbWild);

    static TBOOL IsWildNeedToDelete(TbMap *ptbWild);
};
#endif