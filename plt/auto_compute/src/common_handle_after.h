#ifndef _AU_COMMON_HANDLE_AFTER_H_
#define _AU_COMMON_HANDLE_AFTER_H_

#include "session.h"

class CAuCommonAfter
{
public:
    static TINT32 AuCommonHandleAfter(SSession *pstSession, SUserInfo* pstUser);

private:

    // function  ===> city信息的计算
    static TINT32 UpdateCityInfo(SSession *pstSession, SUserInfo* pstUser);
    static TINT32 UpdatePlayerInfo(SSession *pstSession, SUserInfo* pstUser);

    static TINT32 UpdateUserMight(SSession *pstSession, SUserInfo* pstUser);

    // function  ===> 同步用户信息到地图
    static TINT32 UpdateUserInfoToMap(SSession *pstSession, SUserInfo* pstUser);
    static TVOID UpdateMapCityInfo(SUserInfo* pstUser, TbMap *ptbWild);

    static TINT32 SyncInfoToMarch(SSession *pstSession, SUserInfo* pstUser);

    // function  ===> 删除等级为0的建筑
    static TINT32 DelEmptytBuildingPos(SSession* pstSession, SUserInfo* pstUser);

    // function  ===> 更新特殊推送的定时器
    static TINT32 UpdateNotiTimer(SSession* pstSession, SUserInfo* pstUser);

    static TINT32 DelNotiTimer(SSession* pstSession, SUserInfo* pstUser);
    static TVOID UpdateLastMight(SUserInfo * pstUser, CTseLogger * poServLog);

    static TVOID CheckPeaceTime(SSession *pstSession, SUserInfo* pstUser);

    
};




#endif

