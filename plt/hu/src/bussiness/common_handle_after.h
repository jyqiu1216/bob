#ifndef _COMMON_HANDLE_AFTER_H_
#define _COMMON_HANDLE_AFTER_H_

#include "session.h"

class CCommonHandleAfter
{
public:
    // function  ===> �������ݸ���ǰ��������Ľӿ�
    static TINT32 Process_CommonHandleAfter(SSession *pstSession);

private:
    // function  ===> �ͻ���ÿ������������ʱ,����player���utime
    static TINT32 UpdatePlayerUtimeAndLoginDay(SSession *pstSession);

    // function  ===> �ͻ���ÿ������������ʱ,������ҵ�vip��Ϣ,����ÿ�յ�¼�Ľ���
    static TINT32 UpdatePlayerVipInfo(SSession *pstSession);

    // function  ===> ���ӵ��������
    static TINT32 TroopMonitor(SSession *pstSession);

    // function  ===> city��Ϣ�ļ���
    static TINT32 UpdateCityInfo(SSession *pstSession);

    // function  ===> player��Ϣ�ļ���
    static TINT32 UpdatePlayerInfo(SSession *pstSession);

    // function  ===> �����û���mightֵ
    static TINT32 UpdateUserMight(SSession *pstSession);

    // function  ===> ���ֽ�ѧ�ڼ����Դ����
    static TINT32 NewPlayerResourceProtection(SSession *pstSession);

    // function  ===> �ж��Ƿ�Ҫ��ʾ�û����ֱ����Ѿ�����
    static TINT32 CheckPlayerNewProtectionStatus(SSession *pstSession);

    // function  ===> �ж��Ƿ������״μ������˲��ɹ����н���
    static TINT32 CheckPlayerFirstInAllianceReward(SSession *pstSession);

    // function  ===> �ض�����µĲ�����tips��ʾ
    static TINT32 CleanUpTipsNum(SSession *pstSession);

    // function  ===> ����û�����
    static TINT32 MonitorUserData(SSession *pstSession);

    // function  ===> ͬ���û���Ϣ����ͼ
    static TINT32 UpdateUserInfoToMap(SSession *pstSession);

    static TINT32 SyncInfoToMarch(SSession *pstSession);

    // function  ===> ɾ���ȼ�Ϊ0�Ľ���
    static TINT32 DelEmptytBuildingPos(SSession* pstSession);

    static TINT32 HandleGuideFinishStage(SSession *pstSession);

    // function  ===> �����������͵Ķ�ʱ��
    static TINT32 UpdateNotiTimer(SSession* pstSession);

    // function  ===> �����û���al_gift
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

    //�ͷ���
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

    //�Լ��ǹ���ʱ...ȡ��peace_time
    static TVOID CancelPeaceTimeWhenBecomeKing(SSession *pstSession);
};



#endif

