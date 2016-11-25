#ifndef _COMMON_HANDLE_AFTER_H_
#define _COMMON_HANDLE_AFTER_H_

#include "session.h"

class CCommonHandleAfter
{
public:
    // function  ===> 处理数据更新前公共命令的接口
    static TINT32 Process_CommonHandleAfter(SSession *pstSession);

private:
    // function  ===> 客户端每次主动拉数据时,更新player表的utime
    static TINT32 UpdatePlayerUtimeAndLoginDay(SSession *pstSession);

    // function  ===> 客户端每次主动拉数据时,更新玩家的vip信息,包括每日登录的奖励
    static TINT32 UpdatePlayerVipInfo(SSession *pstSession);

    // function  ===> 军队的数量监控
    static TINT32 TroopMonitor(SSession *pstSession);

    // function  ===> city信息的计算
    static TINT32 UpdateCityInfo(SSession *pstSession);

    // function  ===> player信息的计算
    static TINT32 UpdatePlayerInfo(SSession *pstSession);

    // function  ===> 计算用户的might值
    static TINT32 UpdateUserMight(SSession *pstSession);

    // function  ===> 新手教学期间的资源保护
    static TINT32 NewPlayerResourceProtection(SSession *pstSession);

    // function  ===> 判定是否要提示用户新手保护已经结束
    static TINT32 CheckPlayerNewProtectionStatus(SSession *pstSession);

    // function  ===> 判定是否对玩家首次加入联盟并成功进行奖励
    static TINT32 CheckPlayerFirstInAllianceReward(SSession *pstSession);

    // function  ===> 特定情况下的不产生tips提示
    static TINT32 CleanUpTipsNum(SSession *pstSession);

    // function  ===> 监控用户数据
    static TINT32 MonitorUserData(SSession *pstSession);

    // function  ===> 同步用户信息到地图
    static TINT32 UpdateUserInfoToMap(SSession *pstSession);

    static TINT32 SyncInfoToMarch(SSession *pstSession);

    // function  ===> 删除等级为0的建筑
    static TINT32 DelEmptytBuildingPos(SSession* pstSession);

    static TINT32 HandleGuideFinishStage(SSession *pstSession);

    // function  ===> 更新特殊推送的定时器
    static TINT32 UpdateNotiTimer(SSession* pstSession);

    // function  ===> 更新用户的al_gift
    static TINT32 ComputeAllianceGift(SSession *pstSession);

    static TBOOL TbAlGift_CompareRe(const TbAl_gift& stA, const TbAl_gift& stB);

    static TBOOL IsExisted(TbAl_gift_reward* patbAlGiftRewardList, TUINT32 udwListSize, TINT64 ddwGiftId);

    static TINT32 CorrectExp(SSession* pstSession);

    static TINT32 CompareLordLevel(SSession *pstSession);

    static TINT32 DelNotiTimer(SSession* pstSession);

    static TINT32 CheckSvrAlRecord(SSession *pstSession);

    static TINT32 CheckTaxAction(SSession *pstSession);
    static TVOID UpdateLastMight(SSession * pstSession);

    static TVOID CheckDragon(SSession *pstSession);

    static TVOID CheckTitle(SSession *pstSession);

    //释放龙
    static TVOID CheckPeaceTime(SSession *pstSession);

    static TVOID GenEventInfo(SSession *pstSession);

    static TINT32 CheckRewardWindow(SSession *pstSession);

    static TINT32 GenEquipInfo(SSession *pstSession);

    static TINT32 CheckUserWild(SSession *pstSession);

    static TINT32 UpdateEventTipsStat(SSession *pstSession);

    static TINT32 UpdateSelfTitle(SSession *pstSession);

    static TINT32 UpdateThroneInfo(SSession *pstSession);

    static TINT32 UpdateBroadcastStat(SSession* pstSession);

    static TINT32 CheckRallyReinforce(SSession* pstSession);

    //自己是国王时...取消peace_time
    static TVOID CancelPeaceTimeWhenBecomeKing(SSession *pstSession);
};



#endif

