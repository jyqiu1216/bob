#ifndef _GLOBALRES_LOGIC_H_
#define _GLOBALRES_LOGIC_H_
#include "player_info.h"

class CGlobalResLogic
{        
public:
    static TINT32 CostGlobalRes(SUserInfo *pstUser,SCityInfo* pstCity,SGlobalRes *pstGlobalRes);
    static TBOOL HaveEnoughGlobalsRes(SUserInfo *pstUser,SCityInfo* pstCity,SGlobalRes *pstGlobalRes);
    static TINT32 AddGlobalRes(SUserInfo *pstUser, SCityInfo* pstCity, SGlobalRes *pstGlobalRes, TUINT32 udwSecClass = 0, TINT64 ddwTargetId = -1, TUINT32 udwSpItemId = 0);

    static TINT32 AddSpGlobalRes(SUserInfo *pstUser, SCityInfo* pstCity, SSpGlobalRes *pstGlobalRes, TUINT32 udwSecClass = 0,TINT64 dwTargetId = -1, TUINT32 udwSpItemId = 0);
    static TINT32 CostSpGlobalRes(SUserInfo *pstUser, SCityInfo* pstCity, SSpGlobalRes *pstGlobalRes);
    static TBOOL HaveEnoughSpGlobalsRes(SUserInfo *pstUser, SCityInfo* pstCity, SSpGlobalRes *pstGlobalRes);

    static TINT32 GetSpGlobalResInfo(const Json::Value& oJsonRoot,TINT32 dwRewardType,SSpGlobalRes*pstGlobalRes); 
    
    //各种reward 
    //抽奖箱子
    static TINT32 GetLotteryChestGlobalResInfo(TINT32 dwChestId,SSpGlobalRes *pstGlobalRes);

    //任务奖励信息
    static TINT32 GetTimeQuestReward(TUINT32 udwQuestType,TUINT32 udwQuestLv,TUINT32 udwCastleLv,SGlobalRes *pstGlobalRes);
    
    static TINT32 GetGlobalResInfo(const Json::Value& oJsonRoot, TINT32 dwRewardType, SGlobalRes *pstGlobalRes);

    static TINT32 GetBuildingReward(TUINT32 udwBuildingType, TUINT32 udwBuildLv, SSpGlobalRes *pstGlobalRes);
    static TINT32 GetResearchReward(TUINT32 udwResearchType, TUINT32 udwResearchLv, SSpGlobalRes *pstGlobalRes);

    static TINT32 GetObstleReward(TUINT32 udwBuildingType, TUINT32 udwBuildLv, SSpGlobalRes *pstGlobalRes);

    static TINT32 GetAlGiftRewarId(Json::Value& oJsonRoot);

    //rate 配置方式: 0:10000表示1    1:精确到小数点6位
    static TVOID ComputeBuffRewarRate(Json::Value& oJsonRoot, TbMarch_action* ptbMarch, TBOOL bRateType);

public:
    //base
    static TINT32 HaveEnoughGlobalsRes(SUserInfo *pstUser,SCityInfo* pstCity, TUINT32 udwType,TUINT32 udwItemId,TUINT32 udwItemNum);
    static TINT32 AddGlobalRes(SUserInfo *pstUser, SCityInfo* pstCity, TUINT32 udwType, TUINT32 udwItemId,
        TUINT32 udwItemNum, TUINT32 udwSecClas = 0, TINT64 dwTargetId = -1, TUINT32 udwSpItemId = 0, SAttrInfo *pstAttrInfo = NULL);
    static TINT32 CostGlobalRes(SUserInfo *pstUser, SCityInfo* pstCity, TUINT32 udwType, TUINT32 udwItemId, TUINT32 udwItemNum);
    static TINT32 SetGlobalRes(SUserInfo *pstUser, SCityInfo* pstCity, TUINT32 udwType, TUINT32 udwItemId, TUINT32 udwItemNum);
};

#endif
