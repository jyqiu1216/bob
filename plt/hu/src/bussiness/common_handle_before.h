#ifndef _COMMON_HANDLE_BEFORE_H_
#define _COMMON_HANDLE_BEFORE_H_

#include "session.h"

class CCommonHandleBefore
{
public:
    static TINT32 Process_CommonHandleBefore(SSession *pstSession);

private:
    // function  ===> 军队的数量监控
    static TINT32 TroopMonitor(SSession *pstSession);

    // function  ===> city信息的计算
    static TINT32 UpdateCityInfo(SSession *pstSession);

    // function  ===> 更新用户的al_gift
    static TINT32 ComputeAllianceGift(SSession *pstSession);

    // function  ===> 计算联盟里用户可援助assistance数量
    static TINT32 ComputeCanHelpAllianceAssistance(SSession *pstSession);

    // function  ===> 更新user-stat的tips_time
    static TINT32 UpdateUserStatTipsTime(SSession *pstSession);

    //************************************
    // Method:    AutoClearMarkedAlStoreItem
    // FullName:  CCommonHandleBefore::AutoClearMarkedAlStoreItem
    // Access:    private static 
    // Returns:   TINT32
    // Qualifier: 自动清除被盟主清除打星的mark
    // Parameter: SSession * pstSession
    //************************************
    static TINT32 AutoClearMarkedAlStoreItem(SSession* pstSession);


    //************************************
    // Method:    TbAlGift_CompareRe
    // FullName:  CCommonHandleBefore::TbAlGift_CompareRe
    // Access:    private static 
    // Returns:   TBOOL
    // Qualifier: sort the al_gift
    // Parameter: const TbAl_gift & stA
    // Parameter: const TbAl_gift & stB
    //************************************
    static TBOOL TbAlGift_CompareRe(const TbAl_gift& stA, const TbAl_gift& stB);

    static TBOOL IsExisted(TbAl_gift_reward* patbAlGiftRewardList, TUINT32 udwListSize, TINT64 ddwGiftId);

    static TVOID CheckSvrChangeItem(SSession *pstSession);

    /*
    //检查是否可以进行rating
    //wave@20151209
    */
    static TVOID CheckRating(SSession* pstSession);

    static TVOID CheckWeekIap(SSession* pstSession);

    static TINT32 GetUserDna(SSession* pstSession);

	static TINT32 CheckPeaceTime(SSession *pstSession);

    static TINT32 UpdateNewApnsSwitch(SSession* pstSession);
    static TVOID GetLastTotalMight(SSession * pstSession);

    static TINT32 ComputePlayerPriorityForKf(SSession * pstSession);
};

#endif

