#ifndef _AU_COMMON_HANDLE_AFTER_H_
#define _AU_COMMON_HANDLE_AFTER_H_

#include "session.h"

class CAuCommonAfter
{
public:
    static TINT32 AuCommonHandleAfter(SSession *pstSession, SUserInfo* pstUser);

private:

    // function  ===> city��Ϣ�ļ���
    static TINT32 UpdateCityInfo(SSession *pstSession, SUserInfo* pstUser);
    static TINT32 UpdatePlayerInfo(SSession *pstSession, SUserInfo* pstUser);

    static TINT32 UpdateUserMight(SSession *pstSession, SUserInfo* pstUser);

    // function  ===> ͬ���û���Ϣ����ͼ
    static TINT32 UpdateUserInfoToMap(SSession *pstSession, SUserInfo* pstUser);
    static TVOID UpdateMapCityInfo(SUserInfo* pstUser, TbMap *ptbWild);

    static TINT32 SyncInfoToMarch(SSession *pstSession, SUserInfo* pstUser);

    // function  ===> ɾ���ȼ�Ϊ0�Ľ���
    static TINT32 DelEmptytBuildingPos(SSession* pstSession, SUserInfo* pstUser);

    // function  ===> �����������͵Ķ�ʱ��
    static TINT32 UpdateNotiTimer(SSession* pstSession, SUserInfo* pstUser);

    static TINT32 DelNotiTimer(SSession* pstSession, SUserInfo* pstUser);
    static TVOID UpdateLastMight(SUserInfo * pstUser, CTseLogger * poServLog);

    static TVOID CheckPeaceTime(SSession *pstSession, SUserInfo* pstUser);

    
};




#endif

