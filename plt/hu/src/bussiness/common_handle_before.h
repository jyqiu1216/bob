#ifndef _COMMON_HANDLE_BEFORE_H_
#define _COMMON_HANDLE_BEFORE_H_

#include "session.h"

class CCommonHandleBefore
{
public:
    static TINT32 Process_CommonHandleBefore(SSession *pstSession);

private:
    // function  ===> ���ӵ��������
    static TINT32 TroopMonitor(SSession *pstSession);

    // function  ===> city��Ϣ�ļ���
    static TINT32 UpdateCityInfo(SSession *pstSession);

    // function  ===> �����û���al_gift
    static TINT32 ComputeAllianceGift(SSession *pstSession);

    // function  ===> �����������û���Ԯ��assistance����
    static TINT32 ComputeCanHelpAllianceAssistance(SSession *pstSession);

    // function  ===> ����user-stat��tips_time
    static TINT32 UpdateUserStatTipsTime(SSession *pstSession);

    //************************************
    // Method:    AutoClearMarkedAlStoreItem
    // FullName:  CCommonHandleBefore::AutoClearMarkedAlStoreItem
    // Access:    private static 
    // Returns:   TINT32
    // Qualifier: �Զ����������������ǵ�mark
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
    //����Ƿ���Խ���rating
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

