#include "action_map.h"
#include "time_utils.h"

CComputeMap* CComputeMap::m_pCComputeMap = NULL;

CComputeMap* CComputeMap::GetInstance()
{
    if(m_pCComputeMap == NULL)
    {
        m_pCComputeMap = new CComputeMap;
        m_pCComputeMap->Init();
    }
    return m_pCComputeMap;
}

TVOID CComputeMap::Init()
{
    m_pActionMap = new CAcitonMap;
    m_pActionMap->clear();

    m_pWildMap = new CWildMap;
    m_pWildMap->clear();

    m_pUserMap = new CAcitonMap;
    m_pUserMap->clear();

    int result = 0;
    result = pthread_mutex_init(&m_mtxQue, NULL);
    if(result == -1)
    {
        assert(0);
    }
}

TVOID CComputeMap::InsertIntoMap(TUINT64 uddwActionId, TUINT32 udwSvrId, TUINT32 udwSUid, TUINT32 udwTUid, TUINT32 udwWildId)
{
    TUINT64 uddwSUidKey = udwSvrId * 1000000000L + udwSUid;

    pthread_mutex_lock(&m_mtxQue);

    //TUINT64 uddwCurTime = CTimeUtils::GetCurTimeUs();

    m_pActionMap->insert(make_pair(uddwActionId, 0));

    if(udwSUid)
    {
        m_pUserMap->insert(make_pair(uddwSUidKey, 0));
    }
    /*
    if(udwTUid)
    {
    m_pUserMap->insert(make_pair(uddwTUidKey, 0));
    }

    if(udwWildId)
    {
    m_pWildMap->insert(make_pair(udwWildKey, 0));
    }
    */
    pthread_mutex_unlock(&m_mtxQue);
}

TVOID CComputeMap::InsertIntoMap(TUINT32 udwSvrId, TbAction *pstAction)
{
    // 根据命令字确定首次需要获取的信息
    switch(pstAction->m_nMclass)
    {
    case EN_ACTION_MAIN_CLASS__ITEM:
    case EN_ACTION_MAIN_CLASS__EQUIP:
    case EN_ACTION_CLASS_TASK_NOTIC:
    case EN_ACTION_MAIN_TASK_ATTACK_MOVE:
    case EN_ACTION_MAIN_CLASS__AU_PRESSURE_MEASURE:
    case EN_ACTION_MAIN_CLASS__TRAIN_NEW:
        InsertIntoMap(pstAction->m_nId, udwSvrId, pstAction->m_nSuid, 0, 0);
        break;
    default:
        assert(0);
        break;
    }
}

TVOID CComputeMap::InsertIntoMap(TUINT32 udwSvrId, TbAlliance_action *pstAlAction)
{
    // 根据命令字确定首次需要获取的信息
    switch(pstAlAction->m_nMclass)
    {
    case EN_ACTION_MAIN_CLASS__BUILDING:
    case EN_ACTION_MAIN_CLASS__ITEM:
    case EN_ACTION_MAIN_CLASS__EQUIP:
    case EN_ACTION_CLASS_TASK_NOTIC:
    case EN_ACTION_MAIN_CLASS__TRAIN_NEW:
    case EN_ACTION_MAIN_CLASS__DRAGON:
        InsertIntoMap(pstAlAction->m_nId, udwSvrId, pstAlAction->m_nSuid, 0, 0);
        break;
    default:
        assert(0);
        break;
    }
}

TVOID CComputeMap::InsertIntoMap(TUINT32 udwSvrId, TbMarch_action *ptbMarch)
{
    // 根据命令字确定首次需要获取的信息
    switch(ptbMarch->m_nMclass)
    {
    case EN_ACTION_MAIN_CLASS__MARCH:
    case EN_ACTION_MAIN_CLASS__TIMER:
        if(ptbMarch->m_nStatus == EN_MARCH_STATUS__MARCHING)
        {
            if(ptbMarch->m_nTuid)
            {
                InsertIntoMap(ptbMarch->m_nId, udwSvrId, ptbMarch->m_nSuid, ptbMarch->m_nTuid, 0);
            }
            else
            {
                InsertIntoMap(ptbMarch->m_nId, udwSvrId, ptbMarch->m_nSuid, 0, ptbMarch->m_nTpos);
            }
        }
        else
        {
            InsertIntoMap(ptbMarch->m_nId, udwSvrId, ptbMarch->m_nSuid, 0, 0);
        }
        break;
    default:
        assert(0);
        break;
    }
}

TBOOL CComputeMap::FindInMap(TUINT64 uddwActionId, TUINT32 udwSvrId, TUINT32 udwSUid, TUINT32 udwTUid, TUINT32 udwWildId)
{
    //TUINT32 uddwWildKey = udwSvrId*1000000 +  udwWildId;
    TUINT64 uddwSUidKey = udwSvrId * 1000000000L + udwSUid;
    //TUINT64 uddwTUidKey = udwSvrId*1000000000L + udwTUid;

    CAcitonMap::iterator it;
    CWildMap::iterator itWild;

    pthread_mutex_lock(&m_mtxQue);

    it = m_pActionMap->find(uddwActionId);
    if(it != m_pActionMap->end())
    {
        pthread_mutex_unlock(&m_mtxQue);
        return TRUE;
    }

    if(udwSUid)
    {
        it = m_pUserMap->find(uddwSUidKey);
        if(it != m_pUserMap->end())
        {
            pthread_mutex_unlock(&m_mtxQue);
            return TRUE;
        }
    }

    pthread_mutex_unlock(&m_mtxQue);
    return FALSE;
}

TBOOL CComputeMap::FindInMap(TUINT32 udwSvrId, TbAction *pstAction)
{
    TBOOL bRet = FALSE;
    // 根据命令字确定首次需要获取的信息
    switch(pstAction->m_nMclass)
    {
    case EN_ACTION_MAIN_CLASS__BUILDING:
    case EN_ACTION_MAIN_CLASS__ITEM:
    case EN_ACTION_MAIN_CLASS__EQUIP:
    case EN_ACTION_CLASS_TASK_NOTIC:
    case EN_ACTION_MAIN_TASK_ATTACK_MOVE:
    case EN_ACTION_MAIN_CLASS__TRAIN_NEW:
        bRet = FindInMap(pstAction->m_nId, udwSvrId, pstAction->m_nSuid, 0, 0);
        break;
    default:
        break;
    }

    return bRet;
}

TBOOL CComputeMap::FindInMap(TUINT32 udwSvrId, TbAlliance_action *pstAlAction)
{
    TBOOL bRet = FALSE;
    // 根据命令字确定首次需要获取的信息
    switch(pstAlAction->m_nMclass)
    {
    case EN_ACTION_MAIN_CLASS__BUILDING:
    case EN_ACTION_MAIN_CLASS__ITEM:
    case EN_ACTION_MAIN_CLASS__EQUIP:
    case EN_ACTION_CLASS_TASK_NOTIC:
    case EN_ACTION_MAIN_CLASS__AU_PRESSURE_MEASURE:
    case EN_ACTION_MAIN_CLASS__TRAIN_NEW:
    case EN_ACTION_MAIN_CLASS__DRAGON:
        bRet = FindInMap(pstAlAction->m_nId, udwSvrId, pstAlAction->m_nSuid, 0, 0);
        break;
    default:
        break;
    }

    return bRet;
}

TBOOL CComputeMap::FindInMap(TUINT32 udwSvrId, TbMarch_action *ptbMarch)
{
    TBOOL bRet = FALSE;
    // 根据命令字确定首次需要获取的信息
    switch(ptbMarch->m_nMclass)
    {
    case EN_ACTION_MAIN_CLASS__MARCH:
    case EN_ACTION_MAIN_CLASS__TIMER:
        if(ptbMarch->m_nStatus == EN_MARCH_STATUS__MARCHING)
        {
            if(ptbMarch->m_nTuid)
            {
                bRet = FindInMap(ptbMarch->m_nId, udwSvrId, ptbMarch->m_nSuid, ptbMarch->m_nTuid, 0);
            }
            else
            {
                bRet = FindInMap(ptbMarch->m_nId, udwSvrId, ptbMarch->m_nSuid, 0, ptbMarch->m_nTpos);
            }
        }
        else
        {
            bRet = FindInMap(ptbMarch->m_nId, udwSvrId, ptbMarch->m_nSuid, 0, 0);
        }
        break;
    default:
        break;
    }

    return bRet;
}

TVOID CComputeMap::FinishAction(TUINT64 uddwActionId, TUINT32 udwSvrId, TUINT32 udwSUid, TUINT32 udwTUid, TUINT32 udwWildId)
{
    TUINT64 uddwCurTime = 0;
    TUINT64 uddwSUidKey = udwSvrId * 1000000000L + udwSUid;

    CAcitonMap::iterator it;
    CWildMap::iterator itWild;

    pthread_mutex_lock(&m_mtxQue);

    uddwCurTime = CTimeUtils::GetCurTimeUs();

    it = m_pActionMap->find(uddwActionId);
    if(it != m_pActionMap->end())
    {
        it->second = uddwCurTime;
    }
    else
    {
        m_pActionMap->insert(make_pair(uddwActionId, uddwCurTime));
    }

    if(udwSUid)
    {
        m_pUserMap->erase(uddwSUidKey);
    }
    pthread_mutex_unlock(&m_mtxQue);
}

TVOID CComputeMap::FinishAction(TUINT32 udwSvrId, TbAction *pstAction, TUINT8 ucRawStatus)
{
    // 根据命令字确定首次需要获取的信息
    switch(pstAction->m_nMclass)
    {
    case EN_ACTION_MAIN_CLASS__BUILDING:
    case EN_ACTION_MAIN_CLASS__ITEM:
    case EN_ACTION_MAIN_CLASS__EQUIP:
    case EN_ACTION_CLASS_TASK_NOTIC:
    case EN_ACTION_MAIN_TASK_ATTACK_MOVE:
    case EN_ACTION_MAIN_CLASS__AU_PRESSURE_MEASURE:
    case EN_ACTION_MAIN_CLASS__TRAIN_NEW:
        FinishAction(pstAction->m_nId, udwSvrId, pstAction->m_nSuid, 0, 0);
        break;
    default:
        assert(0);
        break;
    }
}

TVOID CComputeMap::FinishAction(TUINT32 udwSvrId, TbAlliance_action *pstAlAction, TUINT8 ucRawStatus)
{
    // 根据命令字确定首次需要获取的信息
    switch(pstAlAction->m_nMclass)
    {
    case EN_ACTION_MAIN_CLASS__BUILDING:
    case EN_ACTION_MAIN_CLASS__ITEM:
    case EN_ACTION_MAIN_CLASS__EQUIP:
    case EN_ACTION_CLASS_TASK_NOTIC:
    case EN_ACTION_MAIN_CLASS__TRAIN_NEW:
    case EN_ACTION_MAIN_CLASS__DRAGON:
        FinishAction(pstAlAction->m_nId, udwSvrId, pstAlAction->m_nSuid, 0, 0);
        break;
    default:
        assert(0);
        break;
    }
}

TVOID CComputeMap::FinishAction(TUINT32 udwSvrId, TbMarch_action *ptbMarch, TUINT8 ucRawStatus)
{
    // 根据命令字确定首次需要获取的信息
    switch(ptbMarch->m_nMclass)
    {
    case EN_ACTION_MAIN_CLASS__MARCH:
    case EN_ACTION_MAIN_CLASS__TIMER:
        if(ucRawStatus == EN_MARCH_STATUS__MARCHING)
        {
            if(ptbMarch->m_nTuid)
            {
                FinishAction(ptbMarch->m_nId, udwSvrId, ptbMarch->m_nSuid, ptbMarch->m_nTuid, 0);
            }
            else
            {
                FinishAction(ptbMarch->m_nId, udwSvrId, ptbMarch->m_nSuid, 0, ptbMarch->m_nTpos);
            }
        }
        else
        {
            FinishAction(ptbMarch->m_nId, udwSvrId, ptbMarch->m_nSuid, 0, 0);
        }
        break;
    default:
        assert(0);
        break;
    }
}

TVOID CComputeMap::ClearTimeoutAction(TUINT32 udwTimeOutMs, CTseLogger *poLog)
{
    CAcitonMap::iterator it;
    CWildMap::iterator itWild;
    TUINT64 uddwCurTime = 0;
    TUINT32 udwDelNum = 0;
    TUINT64 audwDelKey[10000];
    TUINT64 uddwTimeoutUs = udwTimeOutMs * 1000;
    TUINT32 idx = 0;

    pthread_mutex_lock(&m_mtxQue);

    uddwCurTime = CTimeUtils::GetCurTimeUs();

    // clear action action
    for(it = m_pActionMap->begin(); it != m_pActionMap->end(); it++)
    {
        if(it->second == 0)
        {
            continue;
        }
        if(uddwCurTime < it->second)
        {
            continue;
        }
        if(uddwCurTime - it->second > uddwTimeoutUs)
        {
            audwDelKey[udwDelNum++] = it->first;
            if(udwDelNum >= 10000)
            {
                break;
            }
        }
    }
    for(idx = 0; idx < udwDelNum; idx++)
    {
        m_pActionMap->erase(audwDelKey[idx]);
        TSE_LOG_DEBUG(poLog, ("CActionMap: map erase[%u]", audwDelKey[idx]));
    }
    // log
    TSE_LOG_DEBUG(poLog, ("CActionMap: size[%u],del[%u]", m_pActionMap->size(), udwDelNum));

    pthread_mutex_unlock(&m_mtxQue);
}
