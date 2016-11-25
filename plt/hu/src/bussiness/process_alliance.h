#ifndef _PROCESS_ALLIANCE_H_
#define _PROCESS_ALLIANCE_H_


#include "session.h"



class CProcessAlliance
{
public:

    // function  ===> 创建联盟
    static TINT32 ProcessCmd_AllianceCreate(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ===> 请求加入联盟
    static TINT32 ProcessCmd_AllianceRequestJoin(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ===> 联盟成员主动离开联盟
    static TINT32 ProcessCmd_AllianceLeave(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ===> 联盟成员被踢出
    static TINT32 ProcessCmd_AllianceKickout(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ===> 获取联盟信息
	static TINT32 ProcessCmd_AllianceGetInfo(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ===> 盟主或者副盟主查看联盟请求列表
    static TINT32 ProcessCmd_AllianceRequestGet(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_AllianceRequestAllow(SSession *pstSession, TBOOL &bNeedResponse);
    static TINT32 ProcessCmd_AllianceRequestReject(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ===> 获取联盟成员信息
    static TINT32 ProcessCmd_AllianceMemberGet(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ===> 联盟成员职位变更
    static TINT32 ProcessCmd_AlliancePosChange(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ===> 修改联盟加入是否需要验证
    static TINT32 ProcessCmd_AllianceChangePolicy(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ===> 修改联盟描述
    static TINT32 ProcessCmd_AllianceChangeDesc(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ===> 修改联盟描述
    static TINT32 ProcessCmd_AllianceChangeNotic(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ===> 修改联盟的语言类型
    static TINT32 ProcessCmd_AllianceChangeLanguage(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ===> 置顶联盟评论
    static TINT32 ProcessCmd_AllianceWallMsgTop(SSession *pstSession, TBOOL &bNeedResponse);
    // function  ===> 获取联盟评论
    static TINT32 ProcessCmd_AllianceWallMagGet(SSession *pstSession, TBOOL &bNeedResponse);
    // function  ===> 添加alliance_wall
    static TINT32 ProcessCmd_WallInsert(SSession *pstSession, TBOOL &bNeedResponse);
    // function  ===> 删除alliance_wall
    static TINT32 ProcessCmd_WallDelete(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ===> 申请联盟assistance
    static TINT32 ProcessCmd_AlAssistSend(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ===> 拉取联盟assistance
    static TINT32 ProcessCmd_AlAssistGet(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ===> 拉取联盟assistance
    static TINT32 ProcessCmd_AlAssistDel(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ===> 请求联盟帮助(action)
    static TINT32 ProcessCmd_AlTaskHelpReq(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ===> 获取可以帮助加速的联盟帮助action列表
    static TINT32 ProcessCmd_AlTaskHelpGet(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ===> 帮助加速单个联盟帮助action
    static TINT32 ProcessCmd_AlTaskHelpSpeedUp(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ===> 帮助加速自己列表中能看到的全部联盟帮助action
    static TINT32 ProcessCmd_AlTaskHelpSpeedUpAll(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ===> 开启联盟礼包
    static TINT32 ProcessCmd_AlGiftOpen(SSession *pstSession, TBOOL &bNeedResponse);
    
    // function  ===> 获取联盟礼包列表
    static TINT32 ProcessCmd_AlGiftGet(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ===> 删除过期礼包
    static TINT32 ProcessCmd_AlGiftDel(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ===> 删除全部过期礼包
    static TINT32 ProcessCmd_AlGiftDelAll(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ==> //盟友兑换items(使用忠诚度兑换)
    static TINT32 ProcessCmd_AlItemExchange(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ==> //购买Store items(盟主和副盟主操作)
    static TINT32 ProcessCmd_AlItemBuy(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ==> //给Store items打星(所有联盟成员都能操作)
    static TINT32 ProcessCmd_AlItemMark(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ==> ///取消打星(所有联盟成员都能操作)
    static TINT32 ProcessCmd_AlItemUnmark(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ==> //删除Store items打星记录(盟主和副盟主操作)
    static TINT32 ProcessCmd_AlItemMarkClear(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ==> //拉取联盟间关系的详尽联盟信息
    static TINT32 ProcessCmd_AlDiplomacyGet(SSession *pstSession, TBOOL &bNeedResponse);

    // function  ==> //设置联盟间关系(盟主和副盟主操作)
    static TINT32 ProcessCmd_AlDiplomacySet(SSession *pstSession, TBOOL &bNeedResponse);

	// function ==> //设置联盟旗帜(盟主和副盟主操作)
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
    // function  ==> 用户离开联盟时,同步相关的action信息
    static TVOID PlayerLeaveAllianceUpdtAction(SSession *pstSession, TINT64 ddwSrcUid);

    // function  ==> 通过外交关系获取目标alliance的aid
    static TINT32 GetAidArrayByDiplomacy(SSession *pstSession, TUINT32 udwSvrId, TbDiplomacy *pstList, TUINT32 udwListSize, TUINT8 *pucFlag, TUINT8 ucDiplomacy, vector<TUINT32>& vecAid);

    // function  ==> 由src_to_des字段解析aid
    static TINT32 ParseAid(TUINT64 src_to_des, TINT32 type);

    // function  ==> 由两个aid得到src_to_des字段
    static TUINT64 GetSrcToDes(TUINT32 source, TUINT32 destination);

    // function  ==> 由两个aid得到des_to_src字段
    static TUINT64 GetDesToSrc(TUINT32 source, TUINT32 destination);

    //根据操作者看对方的关系状态调用
    static TINT32 ProcessDiplomacyIfHeIsNormal(TbDiplomacy* ptbMyDiplomacy, TbDiplomacy* ptbHisDiplomacy, TUINT8 ucTargetDiplomacy, TbAlliance& tbMyAlliance, TbAlliance& tbHisAlliance, TUINT8& ucMyDipFlag, TUINT8& ucHisDipFlag, SSession *pstSession);
    static TINT32 ProcessDiplomacyIfHeIsFriendly(TbDiplomacy* ptbMyDiplomacy, TbDiplomacy* ptbHisDiplomacy, TUINT8 ucTargetDiplomacy, TbAlliance& tbMyAlliance, TbAlliance& tbHisAlliance, TUINT8& ucMyDipFlag, TUINT8& ucHisDipFlag, SSession *pstSession);
    static TINT32 ProcessDiplomacyIfHeIsHostile(TbDiplomacy* ptbMyDiplomacy, TbDiplomacy* ptbHisDiplomacy, TUINT8 ucTargetDiplomacy, TbAlliance& tbMyAlliance, TbAlliance& tbHisAlliance, TUINT8& ucMyDipFlag, TUINT8& ucHisDipFlag, SSession *pstSession);
    static TINT32 ProcessDiplomacyIfHeIsPedding(TbDiplomacy* ptbMyDiplomacy, TbDiplomacy* ptbHisDiplomacy, TUINT8 ucTargetDiplomacy, TbAlliance& tbMyAlliance, TbAlliance& tbHisAlliance, TUINT8& ucMyDipFlag, TUINT8& ucHisDipFlag, SSession *pstSession);

    static TINT32 FakeOpenAlGift(TbAl_gift_reward *ptbAlGiftReward, TbAl_gift* ptbAlGift);  //处理过期gift以供客户端展示
};



#endif
