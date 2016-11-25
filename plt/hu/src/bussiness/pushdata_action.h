#ifndef _PUSH_DATA_PROCESS_H_
#define _PUSH_DATA_PROCESS_H_

#include "pushdata_basic.h"

class CPushDataProcess
{
public:
    static TINT32 SendPushDataRequest_ForHuActionComm(SSession *pstSession);

    static TINT32 InitRawInfo(SUserInfo *pstUser, CPushAlData *pobjPushData);
    static TINT32 PushData_HuActionProcess(SSession *pstSession, SUserInfo *pstUser, CPushAlData *pobjPushDataAl, CPushMapData *pobjPushDataMap);

public:
    static TINT32 AddPushData_Wild(CPushMapData *pobjPushDataMap, TbMap *ptbMap);  
    static TINT32 AddPushData_MapAction(CPushMapData *pobjPushDataMap, TbMarch_action *ptbMarch, TINT32 dwTableFlag);

public:
    static TINT32 SendUserLinkerRequest_DataRefreshUid(SSession *pstSession, TUINT16 uwReqServiceType, TUINT32 udwTargetUid, const TCHAR *pKey = NULL);
    static TINT32 SendUserLinkerRequest_DataRefreshAid(SSession *pstSession, TUINT16 uwReqServiceType, TUINT32 udwTargetAid, TUINT32 *pudwExceptUidList = NULL, TUINT32 udwExceptListNum = 0, const TCHAR *pKey = NULL);

public:
    static TINT32 SendPushDataRequest_MarchAction(SSession *pstSession, TUINT32 udwActiveUid, TUINT32 udwActiveAid, TbMarch_action* ptbAction, TINT32 dwUpdtType, SPushActionNode *pstRawActionInfo = NULL);

public:
    static TINT32 SendPushDataRequest_AlAction(SSession *pstSession, TUINT32 udwActiveUid, TbAlliance_action* ptbAction, TINT32 dwUpdtType, SPushActionNode *pstRawActionInfo = NULL);
    static TINT32 SendPushDataRequest_AlActionAid(SSession *pstSession, TbAlliance_action* ptbAction, TINT32 dwUpdtType, const TCHAR* pszTable, TUINT32 udwTargetAid, TUINT32 udwExceptUid = 0);

public:
    static TINT32 SendPushDataRequest_AlHelpReq(SSession *pstSession, TbAlliance_action* ptbAlAction);
    static TINT32 SendPushDataRequest_AlTaskHelpSpeedUp(SSession *pstSession, TbAlliance_action* ptbAlAction, TINT32 dwHelpNum, TINT32 dwHelpTime);
    static TINT32 GetPushTasks_AlTaskHelpSpeedUpAll(SSession *pstSession, LTasksGroup &stTasks);
    static TINT32 SetOneTask_AlTaskHelpSpeedUp(SSession *pstSession, LTask& stTask, TINT32 dwTaskIdx, TUINT16 uwReqServiceType, Json::Value& rJson, TUINT32 udwTargetUid);
};

#endif
