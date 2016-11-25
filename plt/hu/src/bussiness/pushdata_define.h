#ifndef _PUSHDATA_DEFINE_H_
#define _PUSHDATA_DEFINE_H_

#include "base/common/wtsetypedef.h"
#include "aws_table_include.h"

using namespace std;

#define MAX_PUSH_UID_LIST_SIZE  (100)

struct SPushActionNode
{
    TVOID* m_ptbAction;
    TINT32 m_dwType;
    TUINT8 m_ucFlag;
    TBOOL m_isActive;

    TUINT32 m_udwSUid;
    TUINT32 m_udwTUid;
    TINT64  m_nSal;
    TINT64  m_nTal;

    SPushActionNode(TVOID *ptbAction, TINT32 dwType, TUINT8 ucFlag, TBOOL isActive = TRUE)
    {
        m_ptbAction = ptbAction;
        m_dwType = dwType;
        m_ucFlag = ucFlag;

        m_isActive = isActive;

        m_udwSUid = 0;
        m_udwTUid = 0;

        m_nSal = 0;
        m_nTal = 0;

        switch(dwType)
        {
        case EN_UID_ACTION:
            m_udwSUid = ((TbAction*)ptbAction)->m_nSuid;
            break;
        case EN_AID_ACTION:
            m_udwSUid = ((TbAlliance_action*)ptbAction)->m_nSuid;
            m_nSal = ((TbAlliance_action*)ptbAction)->m_nSal;
            break;
        case EN_MARCH_ACTION:
            m_udwSUid = ((TbMarch_action*)ptbAction)->m_nSuid;
            m_udwTUid = ((TbMarch_action*)ptbAction)->m_nTuid;
            m_nSal = ((TbMarch_action*)ptbAction)->m_nSal;
            m_nTal = ((TbMarch_action*)ptbAction)->m_nTal;
            break;
        default:
            break;
        }
    }

    TVOID Reset()
    {
        m_ptbAction = NULL;
        m_dwType = 0;
        m_ucFlag = 0;
        m_isActive = TRUE;

        m_udwSUid = 0;
        m_udwTUid = 0;

        m_nSal = 0;
        m_nTal = 0;
    }
};

struct SPushWildNode
{
    TbMap *m_ptbWild;

    SPushWildNode(TbMap *ptbWild)
    {
        m_ptbWild = ptbWild;
    }

    TVOID Reset()
    {
        m_ptbWild = NULL;
    }
};

class CPushAlData
{
public:
    map<TINT64, SPushActionNode> m_mapAlActionRaw;
    vector<SPushActionNode> m_vecActionList;
    set<TINT64> m_setActionId;
    set<TINT64> m_setUid;

    TUINT32 m_audwUidList[MAX_PUSH_UID_LIST_SIZE];
    TUINT32 m_udwUidListNum;

    TUINT32 m_udwAllianceId;

public:
    TVOID Reset()
    {
        m_mapAlActionRaw.clear();
        m_vecActionList.clear();
        m_setActionId.clear();
        m_setUid.clear();
        m_udwUidListNum = 0;

        m_udwAllianceId = 0;
    }
};

class CPushMapData
{
public:
    vector<SPushActionNode> m_vecActionList;
    set<TINT64> m_setAction;

    vector<SPushWildNode> m_vecWild;
    set<TINT64> m_setWild;

public:
    TVOID Reset()
    {
        m_vecActionList.clear();
        m_setAction.clear();

        m_vecWild.clear();
        m_setWild.clear();
    }
};

class CAuPushDataNode
{
public:    
    CPushAlData m_objPushDataSourceAl;
    CPushAlData m_objPushDataTargetAl;

    CPushMapData m_objPushDataMap;

    // need refresh uid
    TUINT32 m_audwTotalUidList[MAX_PUSH_UID_LIST_SIZE];
    TUINT32 m_udwTotalUidListNum;

public:
    TVOID Reset()
    {
        m_objPushDataSourceAl.Reset();
        m_objPushDataTargetAl.Reset();
        m_objPushDataMap.Reset();
        m_udwTotalUidListNum = 0;
    }
};

class CHuPushDataNode
{
public:
    CPushAlData m_objPushDataSourceAl;
    CPushMapData m_objPushDataMap;

public:
    TVOID Reset()
    {
        m_objPushDataSourceAl.Reset();
        m_objPushDataMap.Reset();
    }
};

#endif
