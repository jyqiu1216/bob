#ifndef _PUSHDATA_BASIC_H_
#define _PUSHDATA_BASIC_H_

#include "pushdata_define.h"
#include "session.h"

class CPushDataBasic
{
public: //for uid(s)
    static TINT32 PushDataUid_Refresh(SSession *pstSession, TUINT32 udwTargetUid);
    static TINT32 PushDataUid_Refresh(SSession *pstSession, TUINT32 *pudwUidList, TUINT32 udwUidListNum);
    static TINT32 PushDataUid_Tips(SSession *pstSession, TUINT32 udwTargetUid, TbTips *ptbTips);
    static TINT32 PushDataUid_AlAction(SSession *pstSession, TUINT32 udwTargetUid, TbAlliance_action* ptbAlAction, TINT32 dwUpdtType);
    static TINT32 PushDataUid_MarchActionSourceUid(SSession *pstSession, TUINT32 udwSourceUid, TbMarch_action* ptbAction, TINT32 dwUpdtType);
    static TINT32 PushDataUid_MarchActionTargetUid(SSession *pstSession, TUINT32 udwTargetUid, TbMarch_action* ptbAction, TINT32 dwUpdtType);

public: //for aid
    //push
    static TINT32 PushDataAid_Refresh(SSession *pstSession, TUINT32 udwTargetAid, TUINT32 *pudwExceptUidList = NULL, TUINT32 udwExceptListNum = 0);
    static TINT32 PushDataAid_KeyRefresh(SSession *pstSession, TUINT32 udwTargetAid, TUINT32 *pudwExceptUidList = NULL, TUINT32 udwExceptListNum = 0, const TCHAR *pKey = NULL);
    static TINT32 PushDataAid_Normal(SSession *pstSession, CPushAlData *pobjPushData, TUINT32 *pudwExceptUidList = NULL, TUINT32 udwExceptListNum = 0);
    static TINT32 PushDataAid_March(SSession *pstSession, TbMarch_action* ptbAction, TINT32 dwUpdtType, const TCHAR* pszTable, TUINT32 udwTargetAid, TUINT32 *pudwExceptUidList = NULL, TUINT32 udwExceptListNum = 0);
    static TINT32 PushDataAid_AlAction(SSession *pstSession, TUINT32 udwTargetAid, TbAlliance_action* ptbAlAction, TINT32 dwUpdtType, TUINT32 *pudwExceptUidList = NULL, TUINT32 udwExceptListNum = 0);

    //get data
    static TINT32 GetPushData_AlAction(vector<SPushActionNode> &vecActionList, TINT64 ddwAid, Json::Value &jsonAlAction);

public: //for map
    //push
    static TINT32 PushDataMap_SingleWild(SSession *pstSession, TbMap *ptbWild);
    static TINT32 PushDataMap_SingleAction(SSession *pstSession, TbMarch_action *ptbMarch, TUINT8 ucFlag);
    static TINT32 PushDataMap_Normal(SSession *pstSession, vector<SPushActionNode> &vecActionList, vector<SPushWildNode> &vecWildList, TINT32 dwSid);

    //get data
    static TINT32 GetPushData_BlockMap(vector<SPushActionNode> &vecActionList, vector<SPushWildNode> &vecWildList, Json::Value &jsonMap, TINT32 dwSid);

public://basic
    static TINT32 PushDataBasic_Map(SSession *pstSession, TUINT16 uwReqServiceType, Json::Value& rJson, TINT32 dwSid);
    static TINT32 PushDataBasic_TargetUid(SSession *pstSession, TUINT16 uwReqServiceType, Json::Value& rJson, TUINT32 udwTargetUid);
    static TINT32 PushDataBasic_TargetUid(SSession *pstSession, TUINT16 uwReqServiceType, Json::Value& rJson, TUINT32 *pudwTargetUidList, TUINT32 udwTargetNum = 1);
    static TINT32 PushDataBasic_TargetAid(SSession *pstSession, TUINT16 uwReqServiceType, Json::Value& rJson, TUINT32 udwTargetAid, TUINT32 *pudwExceptUidList = NULL, TUINT32 udwExceptListNum = 0);

    static TINT32 PushDataBasic_Sid(SSession *pstSession, TUINT16 uwReqServiceType, Json::Value& rJson, TUINT32 udwSid, TUINT32 *pudwExceptUidList = NULL, TUINT32 udwExceptListNum = 0);

private:
    static TINT32 GetDownNode(SSession *pstSession, SDownNode* &pstNode, TBOOL &bExist, TINT32 dwNodeType);
};


#endif