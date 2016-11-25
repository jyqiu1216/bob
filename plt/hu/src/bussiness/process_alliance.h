#ifndef _PROCESS_ALLIANCE_H_
#define _PROCESS_ALLIANCE_H_


#include "session.h"



class CProcessAlliance
{
public:

    // function  ===> ��������
    static TINT32 ProcessCmd_AllianceCreate(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ===> �����������
    static TINT32 ProcessCmd_AllianceRequestJoin(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ===> ���˳�Ա�����뿪����
    static TINT32 ProcessCmd_AllianceLeave(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ===> ���˳�Ա���߳�
    static TINT32 ProcessCmd_AllianceKickout(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ===> ��ȡ������Ϣ
	static TINT32 ProcessCmd_AllianceGetInfo(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ===> �������߸������鿴���������б�
    static TINT32 ProcessCmd_AllianceRequestGet(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_AllianceRequestAllow(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_AllianceRequestReject(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ===> ��ȡ���˳�Ա��Ϣ
    static TINT32 ProcessCmd_AllianceMemberGet(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ===> ���˳�Աְλ���
    static TINT32 ProcessCmd_AlliancePosChange(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ===> �޸����˼����Ƿ���Ҫ��֤
    static TINT32 ProcessCmd_AllianceChangePolicy(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ===> �޸���������
    static TINT32 ProcessCmd_AllianceChangeDesc(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ===> �޸���������
    static TINT32 ProcessCmd_AllianceChangeNotic(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ===> �޸����˵���������
    static TINT32 ProcessCmd_AllianceChangeLanguage(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ===> �ö���������
    static TINT32 ProcessCmd_AllianceWallMsgTop(SSession *pstSession, TBOOL &bNeedResponse);
    // function  ===> ��ȡ��������
    static TINT32 ProcessCmd_AllianceWallMagGet(SSession *pstSession, TBOOL &bNeedResponse);
    // function  ===> ���alliance_wall
    static TINT32 ProcessCmd_WallInsert(SSession *pstSession, TBOOL &bNeedResponse);
    // function  ===> ɾ��alliance_wall
    static TINT32 ProcessCmd_WallDelete(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ===> ��������assistance
    static TINT32 ProcessCmd_AlAssistSend(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ===> ��ȡ����assistance
    static TINT32 ProcessCmd_AlAssistGet(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ===> ��ȡ����assistance
    static TINT32 ProcessCmd_AlAssistDel(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ===> �������˰���(action)
    static TINT32 ProcessCmd_AlTaskHelpReq(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ===> ��ȡ���԰������ٵ����˰���action�б�
    static TINT32 ProcessCmd_AlTaskHelpGet(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ===> �������ٵ������˰���action
    static TINT32 ProcessCmd_AlTaskHelpSpeedUp(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ===> ���������Լ��б����ܿ�����ȫ�����˰���action
    static TINT32 ProcessCmd_AlTaskHelpSpeedUpAll(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ===> �����������
    static TINT32 ProcessCmd_AlGiftOpen(SSession *pstSession, TBOOL &bNeedResponse);
    
    // function  ===> ��ȡ��������б�
    static TINT32 ProcessCmd_AlGiftGet(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ===> ɾ���������
    static TINT32 ProcessCmd_AlGiftDel(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ===> ɾ��ȫ���������
    static TINT32 ProcessCmd_AlGiftDelAll(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ==> //���Ѷһ�items(ʹ���ҳ϶ȶһ�)
    static TINT32 ProcessCmd_AlItemExchange(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ==> //����Store items(�����͸���������)
    static TINT32 ProcessCmd_AlItemBuy(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ==> //��Store items����(�������˳�Ա���ܲ���)
    static TINT32 ProcessCmd_AlItemMark(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ==> ///ȡ������(�������˳�Ա���ܲ���)
    static TINT32 ProcessCmd_AlItemUnmark(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ==> //ɾ��Store items���Ǽ�¼(�����͸���������)
    static TINT32 ProcessCmd_AlItemMarkClear(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ==> //��ȡ���˼��ϵ���꾡������Ϣ
    static TINT32 ProcessCmd_AlDiplomacyGet(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ==> //�������˼��ϵ(�����͸���������)
    static TINT32 ProcessCmd_AlDiplomacySet(SSession *pstSession, TBOOL &bNeedResponse);

	// function ==> //������������(�����͸���������)
	static TINT32 ProcessCmd_AlAvatarChange(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 ProcessCmd_AllianceNickChange(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_AllianceNameChange(SSession *pstSession, TBOOL &bNeedResponse);


    static TINT32 ProcessCmd_DubTitle(SSession* pstSession, TBOOL& bNeedResponse);

    static TINT32 ProcessCmd_Invite(SSession* pstSession, TBOOL& bNeedResponse);
    static TINT32 ProcessCmd_PlayerRecommendGet(SSession* pstSession, TBOOL& bNeedResponse);

    static TINT32 ProcessCmd_InvitedJoin(SSession *pstSession, TBOOL &bNeedResponse);


    static TINT32 ProcessCmd_AlHivePosShow(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 ProcessCmd_AlSetHivePos(SSession *pstSession, TBOOL &bNeedResponse);



private:
    // function  ==> �û��뿪����ʱ,ͬ����ص�action��Ϣ
    static TVOID PlayerLeaveAllianceUpdtAction(SSession *pstSession, TINT64 ddwSrcUid);

    // function  ==> ͨ���⽻��ϵ��ȡĿ��alliance��aid
    static TINT32 GetAidArrayByDiplomacy(SSession *pstSession, TUINT32 udwSvrId, TbDiplomacy *pstList, TUINT32 udwListSize, TUINT8 *pucFlag, TUINT8 ucDiplomacy, vector<TUINT32>& vecAid);

    // function  ==> ��src_to_des�ֶν���aid
    static TINT32 ParseAid(TUINT64 src_to_des, TINT32 type);

    // function  ==> ������aid�õ�src_to_des�ֶ�
    static TUINT64 GetSrcToDes(TUINT32 source, TUINT32 destination);

    // function  ==> ������aid�õ�des_to_src�ֶ�
    static TUINT64 GetDesToSrc(TUINT32 source, TUINT32 destination);

    //���ݲ����߿��Է��Ĺ�ϵ״̬����
    static TINT32 ProcessDiplomacyIfHeIsNormal(TbDiplomacy* ptbMyDiplomacy, TbDiplomacy* ptbHisDiplomacy, TUINT8 ucTargetDiplomacy, TbAlliance& tbMyAlliance, TbAlliance& tbHisAlliance, TUINT8& ucMyDipFlag, TUINT8& ucHisDipFlag, SSession *pstSession);
    static TINT32 ProcessDiplomacyIfHeIsFriendly(TbDiplomacy* ptbMyDiplomacy, TbDiplomacy* ptbHisDiplomacy, TUINT8 ucTargetDiplomacy, TbAlliance& tbMyAlliance, TbAlliance& tbHisAlliance, TUINT8& ucMyDipFlag, TUINT8& ucHisDipFlag, SSession *pstSession);
    static TINT32 ProcessDiplomacyIfHeIsHostile(TbDiplomacy* ptbMyDiplomacy, TbDiplomacy* ptbHisDiplomacy, TUINT8 ucTargetDiplomacy, TbAlliance& tbMyAlliance, TbAlliance& tbHisAlliance, TUINT8& ucMyDipFlag, TUINT8& ucHisDipFlag, SSession *pstSession);
    static TINT32 ProcessDiplomacyIfHeIsPedding(TbDiplomacy* ptbMyDiplomacy, TbDiplomacy* ptbHisDiplomacy, TUINT8 ucTargetDiplomacy, TbAlliance& tbMyAlliance, TbAlliance& tbHisAlliance, TUINT8& ucMyDipFlag, TUINT8& ucHisDipFlag, SSession *pstSession);

    static TINT32 FakeOpenAlGift(TbAl_gift_reward *ptbAlGiftReward, TbAl_gift* ptbAlGift);  //�������gift�Թ��ͻ���չʾ
};



#endif
