#ifndef _PROCESS_SELF_SYSTEM_H_
#define _PROCESS_SELF_SYSTEM_H_

#include "session.h"

enum EALL_BASE_NEED_OPERATE
{
    EN_ADD_OPERATE = 0,
    EN_CLEAR_OPERATE,
};

// ��Ӫ�����ֽӿڳ����ܱ������̵���֮�⣬���������ģ�����
class CProcessSelfSystem
{


// ����ϵͳ(ֻ�ṩ������ϵͳʹ��)
// ����url: did=self-system&sid=&uid=&command=&key0=
public:

    // ================================= �������� ======================== //
    
    // *************** ���� ****************** //
    
    // function ==> ���ñ�ʯ     
    // in_value ==> key0:gem����
    static TINT32 Processcmd_SetGemNum(SSession *pstSession, TBOOL &bNeedResponse);

    // function ==> ������Դ
    // in_value ==> key0:��Դ����
    //          ==> key1:����
    static TINT32 Processcmd_SetResource(SSession *pstSession,TBOOL &bNeedResponse);
    
    // function ==> ���þ���
    // in_value ==> key0:��������
    //          ==> key1:����
    static TINT32 Processcmd_SetTroop(SSession *pstSession,TBOOL &bNeedResponse);
    
    // function ==> ����fort
    // in_value ==> key0:fort����
    //          ==> key1:����
    static TINT32 Processcmd_SetFort(SSession *pstSession,TBOOL &bNeedResponse);//?

    // function ==> ������Ʒ
    // in_value ==> key0:��Ʒ����
    //          ==> key1:����
    static TINT32 Processcmd_SetItem(SSession *pstSession,TBOOL &bNeedResponse);
    
    // function ==> �����û��������˵����˻���
    // in_value ==> key0:����
    static TINT32 Processcmd_SetFund(SSession *pstSession,TBOOL &bNeedResponse);//?
    
    // function ==> �����û��������ҳ϶�
    // in_value ==> key0:����
    static TINT32 Processcmd_SetLoyalty(SSession *pstSession,TBOOL &bNeedResponse);
    
    // function ==> ���ý����ȼ�(û�н��������ȼ�������߼�)(pos/type)
    // in_value ==> key0:����λ��
    //          ==> key1:��������
    //          ==> key2:�����ȼ�
    static TINT32 Processcmd_SetBuildingLv(SSession *pstSession,TBOOL &bNeedResponse);
    
    // function ==> ���ý����ȼ�(û�н��������ȼ�������߼�)(pos)
    // in_value ==> key0:����λ��
    //          ==> key1:�����ȼ�
    static TINT32 Processcmd_SetPosBuildingLv(SSession *pstSession,TBOOL &bNeedResponse);
    
    // function ==> �����о��ȼ�
    // in_value ==> key0:�о�����
    //          ==> key1:�о��ȼ�
    static TINT32 Processcmd_SetResearchLv(SSession *pstSession,TBOOL &bNeedResponse);

    // *************** ȫ�� ****************** //
    
    // function ==> ���Ӻ���վ���
    // in_value ==> key0:��������(0:����;1:���)
    static TINT32 ProcessCmd_AddClearTroop(SSession *pstSession, TBOOL &bNeedResponse);
    
    // function ==> ���Ӻ����fort
	// in_value ==> key0:��������(0:����;1:���)
    static TINT32 ProcessCmd_AddClearFort(SSession *pstSession, TBOOL &bNeedResponse);
    
	// function ==> ���Ӻ������Դ
	// in_value ==> key0:��������(0:����;1:���)
    static TINT32 ProcessCmd_AddClearResource(SSession *pstSession, TBOOL &bNeedResponse);

    // function ==> �����Ʒ
	// in_value ==> key0:��������(0:����;1:���)
    static TINT32 ProcessCmd_ClearItem(SSession *pstSession, TBOOL &bNeedResponse);


    // function ==> ���������о��ȼ�
    // in_value ==> key0:�о��ȼ�
    static TINT32 Processcmd_SetAllResearchLv(SSession *pstSession,TBOOL &bNeedResponse);

    // function ==> �������н����ȼ�
    // in_value ==> key0:�����ȼ�
    static TINT32 Processcmd_SetAllBuildingLv(SSession *pstSession,TBOOL &bNeedResponse);

    

    // ================================= �������� ======================== //

    // function ==> ������õ�װ������
    static TINT32 Processcmd_CleanEquipGrid(SSession *pstSession, TBOOL &bNeedResponse);

    // function ==> ���Ӻ���վ���
    static TINT32 ProcessCmd_AddClearScroll(SSession *pstSession, TBOOL &bNeedResponse);

    
    // function ==> ���fort��ҽԺ
    // in_value ==> key0:��������
    //          ==> key1:����
    static TINT32 ProcessCmd_AddDeadFort(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_ReduceDeadFort(SSession *pstSession, TBOOL &bNeedResponse);

    // function ==> ��ӱ���ҽԺ
    // in_value ==> key0:��������
    //          ==> key1:����
    static TINT32 ProcessCmd_AddHosTroop(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_ReduceHosTroop(SSession *pstSession, TBOOL &bNeedResponse);

	//�������ֱ���
	static TINT32 Processcmd_BreakNewUserProtect(SSession *pstSession, TBOOL &bNeedResponse);

	static TINT32 Processcmd_GetBufferInfo(SSession *pstSession, TBOOL &bNeedResponse);
	// function ==> knight���

    //����vip�ȼ�
    static TINT32 Processcmd_SetVipLevel(SSession* pstSession, TBOOL &bNeedResponse);

    //����vipʣ��ʱ��
    static TINT32 Processcmd_SetVipLeftTime(SSession* pstSession, TBOOL &bNeedResponse);

    static TINT32 Processcmd_SetPlayerLv(SSession* pstSession, TBOOL &bNeedResponse);
    static TINT32 Processcmd_OpenChest(SSession* pstSession, TBOOL &bNeedResponse);

    static TINT32 Processcmd_SetLastLoginTime(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 Processcmd_SetCurLoytal(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 Processcmd_SetDragonShard(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 Processcmd_SetEquipGrideNum(SSession *pstSession, TBOOL &bNeedResponse);
    
    static TINT32 Processcmd_ResetFinishTask(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 Processcmd_SetFinishTask(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 Processcmd_RefreshTaskCur(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 Processcmd_AddWild(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 Processcmd_AddCrystal(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 Processcmd_AddSpCrystal(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 Processcmd_AddMaterial(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 Processcmd_AddSoul(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 Processcmd_AddParts(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 Processcmd_AddNormalEquip(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 Processcmd_AddSpEquip(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 Processcmd_CleanAllCrystal(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 Processcmd_CleanAllMaterial(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 Processcmd_CleanAllSoul(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 Processcmd_CleanAllParts(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 Processcmd_CleanAllEquip(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 SetSidePos(SSession *pstSession, TbMap *pstMap, TbMap *patbMap, TUINT32 udwNum, TUINT32 udwTypeBlockNum);

    static TINT32 Processcmd_CleanSecondBuildingAction(SSession *pstSession, TBOOL &bNeedResponse);
    // function ==> �������ָ����־λ
    // in_value ==> key0:��Ҫ�����guideflagλ
    static TINT32 Processcmd_ClearGuideFlag(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 Processcmd_SetGuideFlag(SSession *pstSession, TBOOL &bNeedResponse);

    // function ==> �������һ����������Ĺ���ʱ��
    static TINT32 Processcmd_SetAlGiftTime(SSession *pstSession, TBOOL &bNeedResponse);
    // function ==> �������˵�al gift level
    static TINT32 Processcmd_SetAlGiftLv(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 Processcmd_SetDragonExcuteTime(SSession* pstSession, TBOOL& bNeedResponse);
    static TINT32 Processcmd_SetDragonAutoReleaseTime(SSession* pstSession, TBOOL& bNeedResponse);

    static TINT32 Processcmd_SetThroneTime(SSession* pstSession, TBOOL& bNeedResponse);
    static TINT32 Processcmd_RecoverThroneTime(SSession* pstSession, TBOOL& bNeedResponse);

    static TINT32 Processcmd_CutPrepareTime(SSession* pstSession, TBOOL& bNeedResponse);

    static TINT32 Processcmd_SetSmokeTime(SSession* pstSession, TBOOL& bNeedResponse);

    static TINT32 Processcmd_CaptureDragon(SSession* pstSession, TBOOL& bNeedResponse);
    static TINT32 Processcmd_KillSelfDragon(SSession* pstSession, TBOOL& bNeedResponse);

    static TINT32 Processcmd_OpAddGlobalres(SSession* pstSession, TBOOL& bNeedResponse);


    static TINT32 Processcmd_HelpSelfAction(SSession* pstSession, TBOOL& bNeddResponse);

    static TINT32 Processcmd_SetAltarBuffTime(SSession* pstSession, TBOOL& bNeedResponse);

    static TINT32 Processcmd_AddVipPoint(SSession* pstSession, TBOOL& bNeedResponse);

    static TINT32 Processcmd_CleanAllTree(SSession* pstSession, TBOOL& bNeedResponse);

    static TINT32 Processcmd_SetQuestRefresh(SSession* pstSession, TBOOL& bNeedResponse);

    static TINT32 Processcmd_ClearAllBuff(SSession* pstSession, TBOOL& bNeedResponse);

    static TINT32 Processcmd_SetFinishQuest(SSession* pstSession, TBOOL& bNeedResponse);

    static TINT32 Processcmd_CleanTitle(SSession* pstSession, TBOOL& bNeedResponse);

    static TINT32 Processcmd_SendBroadcast(SSession* pstSession, TBOOL& bNeedResponse);

    static TINT32 Processcmd_AddRecommendPlayer(SSession* pstSession, TBOOL& bNeedResponse);

    static TINT32 Processcmd_DelInviteRecord(SSession* pstSession, TBOOL& bNeedResponse);

    static TINT32 Processcmd_SetAlStar(SSession* pstSession, TBOOL& bNeedResponse);

    static TINT32 Processcmd_SetAssistPostTime(SSession* pstSession, TBOOL& bNeedResponse);

    static TINT32 Processcmd_SetLastUpdateTime(SSession* pstSession, TBOOL& bNeedResponse);

    static TINT32 Processcmd_ReleaseSelfDragon(SSession* pstSession, TBOOL& bNeedResponse);

    static TINT32 Processcmd_SetThroneTroop(SSession* pstSession, TBOOL& bNeedResponse);

    static TINT32 Processcmd_SetTradeRefreshTime(SSession* pstSession, TBOOL& bNeedResponse);
    static TINT32 Processcmd_SetTradeMarchTime(SSession* pstSession, TBOOL& bNeedResponse);
    static TINT32 Processcmd_SetTradeWaitingTime(SSession* pstSession, TBOOL& bNeedResponse);
    static TINT32 Processcmd_SetMysteryStoreRefreshTime(SSession* pstSession, TBOOL& bNeedResponse);
    static TINT32 Processcmd_SetVioletGold(SSession* pstSession, TBOOL& bNeedResponse);
    static TINT32 Processcmd_SetUserCreateTime(SSession* pstSession, TBOOL& bNeedResponse);
    static TINT32 Processcmd_SetContinueLoginDay(SSession* pstSession, TBOOL& bNeedResponse);
    static TINT32 ProcessCmd_SetBountyRefreshTime(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_SetBountyNodeStarNum(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 ProcessCmd_ClearRecommend(SSession* pstSession, TBOOL& bNeedResponse);

    static TINT32 ProcessCmd_ClearInvitedCount(SSession* pstSession, TBOOL& bNeedResponse);

    // function ==> ���ñ��˵İ�����������״̬
    // in_value ==> key0:ָ����������״̬(0:������, 1:������)
    static TINT32 ProcessCmd_SetPersonalHelpBubble(SSession* pstSession, TBOOL& bNeedResponse);

    // function ==> ���ñ��˵İ����������ݳ�ʱʱ��
    // in_value ==> key0:���ݵĳ�ʱʱ�䣨�룩
    static TINT32 ProcessCmd_SetHelpBubbleTimeOut(SSession* pstSession, TBOOL& bNeedResponse);

    // function ==> ����Զ��ظ�ʱ����
    static TINT32 ProcessCmd_ClearSupportTimeTag(SSession* pstSession, TBOOL& bNeedResponse);

    static TINT32 ProcessCmd_SetTaxTime(SSession* pstSession, TBOOL& bNeedResponse);


    static TINT32 Processcmd_SetDragonLevel(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 Processcmd_SetPlayerLevel(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 Processcmd_SetKnightLevel(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 Processcmd_SetBuffFailureTime(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 Processcmd_SetMonsterWildRefreshTime(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 Processcmd_SetSmokeFireDisappearTime(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 Processcmd_SetConsecutiveLogin(SSession* pstSession, TBOOL& bNeedResponse);

    static TINT32 ProcessCmd_SetAlWallNewestTime( SSession* pstSession, TBOOL& bNeedResponse );

    //wave@20160425
    static TINT32 ProcessCmd_SetTaskInShowList(SSession* pstSession, TBOOL& bNeedResponse);
    static TINT32 ProcessCmd_SetPersonGuideFlag(SSession* pstSession, TBOOL& bNeedResponse);

    static TINT32 ProcessCmd_ResetMonsterHit(SSession* pstSession, TBOOL& bNeedResponse);

    static TINT32 ProcessCmd_SetSystemTime(SSession* pstSession, TBOOL& bNeedResponse);

    static TINT32 ProcessCmd_SetMap(SSession* pstSession, TBOOL& bNeedResponse);
    static TINT32 ProcessCmd_SetMapExpire(SSession* pstSession, TBOOL& bNeedResponse);
    static TINT32 ProcessCmd_SetRallyTime(SSession* pstSession, TBOOL& bNeedResponse);
    static TINT32 ProcessCmd_SetCampProtectTime(SSession* pstSession, TBOOL& bNeedResponse);
    static TINT32 ProcessCmd_SetTrialLock(SSession* pstSession, TBOOL& bNeedResponse);
    static TINT32 ProcessCmd_AddAlGiftPoint(SSession* pstSession, TBOOL& bNeedResponse);
    static TINT32 ProcessCmd_AddAlLoyaltyAndFund(SSession* pstSession, TBOOL& bNeedResponse);
    static TINT32 ProcessCmd_SetAttackMoveTime(SSession* pstSession, TBOOL& bNeedResponse);
    static TINT32 ProcessCmd_SetRallyAttackSlotAllOpen(SSession* pstSession, TBOOL& bNeedResponse);
    static TINT32 ProcessCmd_SetAlGiftDisappearTime(SSession* pstSession, TBOOL& bNeedResponse);

    static TINT32 Processcmd_OpChangeGlobalres(SSession* pstSession, TBOOL& bNeedResponse);
    static TINT32 Processcmd_OpSetGlobalres(SSession* pstSession, TBOOL& bNeedResponse);

    static TINT32 Processcmd_SetIdolTime(SSession* pstSession, TBOOL& bNeedResponse);
    static TINT32 Processcmd_FillIdolRank(SSession* pstSession, TBOOL& bNeedResponse);

    static TINT32 Processcmd_SetTitleExpireTime(SSession* pstSession, TBOOL& bNeedResponse);
    static TINT32 Processcmd_SetTaxBeginTime(SSession* pstSession, TBOOL& bNeedResponse);
    static TINT32 Processcmd_SetTaxIntervalTime(SSession* pstSession, TBOOL& bNeedResponse);

    static TINT32 Processcmd_SetThroneTimeNew(SSession* pstSession, TBOOL& bNeedResponse);

    static TINT32 Processcmd_GenPeaceTime(SSession* pstSession, TBOOL& bNeedResponse);
};

#endif