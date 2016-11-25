#ifndef _PUSH_DATA_PROCESS_H_
#define _PUSH_DATA_PROCESS_H_
#include "pushdata_define.h"
#include "common_json.h"
#include "session.h"
#include <vector>
#include <map>
#include <set>
using namespace std;


class CAuPushData
{
public:
    static TINT32 InitRawAlInfo(SUserInfo *pstUser, CPushAlData *pobjPushData);
    static TINT32 AuPushData(SSession *pstSession);

public:
    static TINT32 PushData_WildToMap(SSession *pstSession, TbMap *ptbWild);
    static TINT32 PushDataAid_March(SSession *pstSession, TbMarch_action* ptbAction, TINT32 dwUpdtType, const TCHAR* pszTable, 
        TUINT32 udwTargetAid, TUINT32 *pudwExceptUidList = NULL, TUINT32 udwExceptListNum = 0);

public:
    static TINT32 AddPushData_Aid(SSession *pstSession, TUINT32 udwId);

public:
    //static TINT32 ComputeActionInfo(SUserInfo *pstUser, CPushAlData *pobjPushDataAl, CPushMapData *pobjPushDataMap);
    
    static TINT32 AddPushData_Action(SSession *pstSession, CPushAlData *pobjPushDataAl, CPushMapData *pobjPushDataMap, void *ptbAction, TINT32 dwType, TUINT8 ucFlag, TBOOL isActive = TRUE);
    static TINT32 AddPushData_ReqMarchAction(SSession *pstSession, TbMarch_action *ptbMarch, TUINT8 ucFlag);
    static TINT32 AddPushData_AlUid(CPushAlData *pobjPushDataAl, TUINT32 udwId);
    static TINT32 AddPushData_Wild(CPushMapData *pobjPushDataMap, TbMap *ptbMap);  
    static TINT32 AddPushData_MapAction(CPushMapData *pobjPushDataMap, TbMarch_action *ptbMarch, TINT32 dwTableFlag);
    
private:
    static TINT32 GetPushData_AlAction(vector<SPushActionNode> &vecActionList, TINT64 ddwAid, Json::Value &jsonAlAction);
    static TINT32 GetPushData_BlockMap(vector<SPushActionNode> &vecActionList, vector<SPushWildNode> &vecWildList, Json::Value &jsonMap, TINT32 dwSid);
    static TBOOL IsMarchActionNeedToPushMap(SPushActionNode *pstNode);	

    static TINT32 UpdtTotalRefreshUidList(CAuPushDataNode *pstNode);
    static TINT32 DelUnExistAction(CPushAlData *pobjPushDataAl);

private:
    static TINT32 PushDataBasic_Map(SSession *pstSession, Json::Value& rJson, TINT32 dwSid);
    static TINT32 PushDataBasic_RefreshUid(SSession *pstSession, TUINT32 *pudwUidList, TUINT32 udwUidListNum);
    static TINT32 PushDataBasic_TargetAid(SSession *pstSession, Json::Value& rJson, TUINT32 udwTargetAid, TUINT32 *pudwExceptUidList = NULL, TUINT32 udwExceptListNum = 0);

private:
    static TINT32 PushDataAid_Refresh(SSession *pstSession, TUINT32 *pudwAidList, TUINT32 udwAidListNum, TUINT32 *pudwExceptUidList = NULL, TUINT32 udwExceptListNum = 0);

private:
    static TINT32 PushDataBasic_AllSvr(SSession *pstSession, Json::Value& rJson, TUINT32 udwSid, TUINT32 *pudwExceptUidList = NULL, TUINT32 udwExceptListNum = 0);
    
};

#endif
