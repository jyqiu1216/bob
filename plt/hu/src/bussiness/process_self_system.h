#ifndef _PROCESS_SELF_SYSTEM_H_
#define _PROCESS_SELF_SYSTEM_H_

#include "session.h"

enum EALL_BASE_NEED_OPERATE
{
    EN_ADD_OPERATE = 0,
    EN_CLEAR_OPERATE,
};

// 运营命令字接口除了能被主流程调用之外，不允许被别的模块调用
class CProcessSelfSystem
{


// 自助系统(只提供给自助系统使用)
// 请求url: did=self-system&sid=&uid=&command=&key0=
public:

    // ================================= 基础需求 ======================== //
    
    // *************** 单个 ****************** //
    
    // function ==> 设置宝石     
    // in_value ==> key0:gem数量
    static TINT32 Processcmd_SetGemNum(SSession *pstSession, TBOOL &bNeedResponse);

    // function ==> 设置资源
    // in_value ==> key0:资源类型
    //          ==> key1:数量
    static TINT32 Processcmd_SetResource(SSession *pstSession,TBOOL &bNeedResponse);
    
    // function ==> 设置军队
    // in_value ==> key0:军队类型
    //          ==> key1:数量
    static TINT32 Processcmd_SetTroop(SSession *pstSession,TBOOL &bNeedResponse);
    
    // function ==> 设置fort
    // in_value ==> key0:fort类型
    //          ==> key1:数量
    static TINT32 Processcmd_SetFort(SSession *pstSession,TBOOL &bNeedResponse);//?

    // function ==> 设置物品
    // in_value ==> key0:物品类型
    //          ==> key1:数量
    static TINT32 Processcmd_SetItem(SSession *pstSession,TBOOL &bNeedResponse);
    
    // function ==> 设置用户所在联盟的联盟基金
    // in_value ==> key0:数量
    static TINT32 Processcmd_SetFund(SSession *pstSession,TBOOL &bNeedResponse);//?
    
    // function ==> 设置用户的联盟忠诚度
    // in_value ==> key0:数量
    static TINT32 Processcmd_SetLoyalty(SSession *pstSession,TBOOL &bNeedResponse);
    
    // function ==> 设置建筑等级(没有建筑提升等级的相关逻辑)(pos/type)
    // in_value ==> key0:建筑位置
    //          ==> key1:建筑类型
    //          ==> key2:建筑等级
    static TINT32 Processcmd_SetBuildingLv(SSession *pstSession,TBOOL &bNeedResponse);
    
    // function ==> 设置建筑等级(没有建筑提升等级的相关逻辑)(pos)
    // in_value ==> key0:建筑位置
    //          ==> key1:建筑等级
    static TINT32 Processcmd_SetPosBuildingLv(SSession *pstSession,TBOOL &bNeedResponse);
    
    // function ==> 设置研究等级
    // in_value ==> key0:研究类型
    //          ==> key1:研究等级
    static TINT32 Processcmd_SetResearchLv(SSession *pstSession,TBOOL &bNeedResponse);

    // *************** 全部 ****************** //
    
    // function ==> 增加和清空军队
    // in_value ==> key0:操作类型(0:增加;1:清空)
    static TINT32 ProcessCmd_AddClearTroop(SSession *pstSession, TBOOL &bNeedResponse);
    
    // function ==> 增加和清空fort
	// in_value ==> key0:操作类型(0:增加;1:清空)
    static TINT32 ProcessCmd_AddClearFort(SSession *pstSession, TBOOL &bNeedResponse);
    
	// function ==> 增加和清空资源
	// in_value ==> key0:操作类型(0:增加;1:清空)
    static TINT32 ProcessCmd_AddClearResource(SSession *pstSession, TBOOL &bNeedResponse);

    // function ==> 清空物品
	// in_value ==> key0:操作类型(0:增加;1:清空)
    static TINT32 ProcessCmd_ClearItem(SSession *pstSession, TBOOL &bNeedResponse);


    // function ==> 设置所有研究等级
    // in_value ==> key0:研究等级
    static TINT32 Processcmd_SetAllResearchLv(SSession *pstSession,TBOOL &bNeedResponse);

    // function ==> 设置所有建筑等级
    // in_value ==> key0:建筑等级
    static TINT32 Processcmd_SetAllBuildingLv(SSession *pstSession,TBOOL &bNeedResponse);

    

    // ================================= 特殊需求 ======================== //

    // function ==> 清空无用的装备格子
    static TINT32 Processcmd_CleanEquipGrid(SSession *pstSession, TBOOL &bNeedResponse);

    // function ==> 增加和清空卷轴
    static TINT32 ProcessCmd_AddClearScroll(SSession *pstSession, TBOOL &bNeedResponse);

    
    // function ==> 添加fort到医院
    // in_value ==> key0:军队类型
    //          ==> key1:数量
    static TINT32 ProcessCmd_AddDeadFort(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_ReduceDeadFort(SSession *pstSession, TBOOL &bNeedResponse);

    // function ==> 添加兵到医院
    // in_value ==> key0:军队类型
    //          ==> key1:数量
    static TINT32 ProcessCmd_AddHosTroop(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_ReduceHosTroop(SSession *pstSession, TBOOL &bNeedResponse);

	//打破新手保护
	static TINT32 Processcmd_BreakNewUserProtect(SSession *pstSession, TBOOL &bNeedResponse);

	static TINT32 Processcmd_GetBufferInfo(SSession *pstSession, TBOOL &bNeedResponse);
	// function ==> knight相关

    //设置vip等级
    static TINT32 Processcmd_SetVipLevel(SSession* pstSession, TBOOL &bNeedResponse);

    //设置vip剩余时间
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
    // function ==> 清除新手指引标志位
    // in_value ==> key0:需要清除的guideflag位
    static TINT32 Processcmd_ClearGuideFlag(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 Processcmd_SetGuideFlag(SSession *pstSession, TBOOL &bNeedResponse);

    // function ==> 设置最后一个联盟礼物的过期时间
    static TINT32 Processcmd_SetAlGiftTime(SSession *pstSession, TBOOL &bNeedResponse);
    // function ==> 设置联盟的al gift level
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

    // function ==> 设置本人的帮助神像气泡状态
    // in_value ==> key0:指定的其气泡状态(0:无气泡, 1:有气泡)
    static TINT32 ProcessCmd_SetPersonalHelpBubble(SSession* pstSession, TBOOL& bNeedResponse);

    // function ==> 设置本人的帮助神像气泡超时时间
    // in_value ==> key0:气泡的超时时间（秒）
    static TINT32 ProcessCmd_SetHelpBubbleTimeOut(SSession* pstSession, TBOOL& bNeedResponse);

    // function ==> 清除自动回复时间标记
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