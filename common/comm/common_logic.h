#ifndef _COMMON_LOGIC_H_
#define _COMMON_LOGIC_H_
#include "aws_table_include.h"

class CCommonLogic
{
public:
    static TINT64 GetTableSeq(string sTableName, SUserInfo *pstUser);

    static TINT32 GenAlGiftToPerson(TbPlayer *ptbPlayer, TINT32 dwPackId, SAlGiftList& sAlGiftList, TbLogin* ptbLogin);

    static TINT32 GenIapAlGift(TbAlliance* ptbAlliance, TINT32 dwGem, TINT64 ddwSrcUid, SAlGiftList& sAlGiftList, TbLogin* ptbLogin); //生成IAP的al gift
    static TINT32 GenAlGiftToAlliance(TINT64 ddwAid, TINT32 dwPackId, TINT64 ddwSrcUid, SAlGiftList& sAlGiftList, TbLogin* ptbLogin); //生成打仗的al gift

    static TINT32 GenEventAlGift(TbPlayer *ptbPlayer, TINT32 dwPackId, SAlGiftList& sAlGiftList, TUINT32 udwEventType, TbLogin* ptbLogin);

    //************************************
    // Method:    ComputeCanHelpAlAction
    // FullName:  CCommonLogic::ComputeCanHelpAlAction
    // Access:    public static 
    // Returns:   TINT32
    // Qualifier: 计算可以帮助的联盟任务
    // Parameter: SUserInfo * pstUserInfo
    //************************************
    static TINT32 ComputeCanHelpAlAction(SUserInfo * pstUserInfo);


    //************************************
    // Method:    CheckBuildingCollision
    // FullName:  CCommonLogic::CheckBuildingCollision
    // Access:    public static 
    // @param:	  TbCity * pstCity
    // @return:   TBOOL
    //************************************
    static TBOOL CheckBuildingCollision(TbCity* ptbCity);
    static TBOOL CheckBuildingCollision(TbCity* ptbCity, SCityBuildingNode stNewBuilding);
    static TBOOL CheckBuildingCollision(TbCity* ptbCity, std::vector<SCityBuildingNode>& vecNewBuilding, TUINT32 udwSeq);
    static TBOOL GetBuildingPos(TINT32 dwCenterPos, TINT32 dwSize, std::set<TINT32>& posSet);

    static BuildingPoint BuildingPosToPoint(TINT32 dwPos);
    static TINT32 BuildingPosToMapPos(TINT32 dwBpos);
    static TINT32 BuildingPointToPos(BuildingPoint point);
    static TINT32 BuildingPointToPos(TINT32 x, TINT32 y);
    static TBOOL AddBuildingPos(TINT32 dwCenterPos, TINT32 dwSize, std::set<TINT32>& posSet);

    static TBOOL GetWildPos(TINT32 dwCenterPos, TINT32 dwSize, std::set<TINT32>& posSet);

    static TINT32 GenObstle(TbCity* ptbCity, TUINT32 udwId, std::vector<TINT32> *pstSet);

    //不处理驻军action
    static TINT32 AbandonThrone(TbAlliance* ptbAlliance, TbThrone *ptbThrone, TbMap* ptbWild);

    static TVOID GenPrisonReport(TbMarch_action* ptbPrison, TbPlayer* ptbSaver, TINT32 dwReportType, TINT32 dwReportResult, TbReport* ptbReport);

    static TFLOAT32 GetIapNumByRechargeGem(const TUINT32 udwGemNum);

    static TVOID AddBookMark(SUserInfo *pstUser, TUINT64 uddwPos, TUINT8 ucBookmarkType, string pszBookmarkNick);

    static TBOOL CheckFuncOpen(SUserInfo *pstUser, SCityInfo *pstCity , TUINT32 udwType);

    static TBOOL CheckAllConfirmCon(SUserInfo *pstUser, SCityInfo *pstCity, TUINT32 udwType);

    static TBOOL CheckOneConfirmCon(SUserInfo *pstUser, SCityInfo *pstCity, TUINT32 udwType);

    static TBOOL CheckSvrOpen(TUINT32 udwSid, TUINT32 udwType);

    static TBOOL HasTitle(TINT64 ddwUid, TbThrone *ptbThrone, STitleInfoList *pstTitle);
    static TBOOL HasTitleDub(TINT64 ddwTitleId, TbThrone *ptbThrone, STitleInfoList *pstTitle);
    static TVOID AddTitle(TbPlayer *ptbPlayer, TINT64 ddwTitleId, STitleInfoList *pstTitle);
    static TVOID RemoveTitle(TINT64 ddwUid, TINT64 ddwTitleId, STitleInfoList *pstTitle);
    static TVOID RemoveTitle(TINT64 ddwUid, STitleInfoList *pstTitle);
    static TVOID UpdateTitle(TbPlayer *ptbPlayer, STitleInfoList *pstTitle);

    static TVOID UpdateThroneInfo(SUserInfo *pstUser, TbThrone *ptbThrone);
};

#endif // !_COMMON_LOGIC_H_



