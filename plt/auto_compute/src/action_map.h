#ifndef _ACTION_MAP_H_
#define _ACTION_MAP_H_

#include "base/common/wtse_std_header.h"
#include <pthread.h>
#include <map>
#include "base/log/wtselogger.h"
#include "player_info.h"

using namespace std;
using namespace wtse::log;

typedef map<TUINT64, TUINT64> CAcitonMap;
typedef map<TUINT32, TUINT64> CWildMap;

class CComputeMap
{
public:
    static CComputeMap* GetInstance();
private:
    static CComputeMap* m_pCComputeMap;

private:
    //互斥锁
    pthread_mutex_t m_mtxQue;

public:
    CAcitonMap*	m_pActionMap;			// 用于防止同样的action被重复计算
    CWildMap *m_pWildMap;				// 用于防止同样的目标被同时march中处理
    CAcitonMap *m_pUserMap;				// 用于防止同一用户的数据被同时处理

public:
    TVOID Init();

    TVOID InsertIntoMap(TUINT64 uddwActionId, TUINT32 udwSvrId, TUINT32 udwSUid, TUINT32 udwTUid, TUINT32 udwWildId);
    TBOOL FindInMap(TUINT64 uddwActionId, TUINT32 udwSvrId, TUINT32 udwSUid, TUINT32 udwTUid, TUINT32 udwWildId);
    TVOID FinishAction(TUINT64 uddwActionId, TUINT32 udwSvrId, TUINT32 udwSUid, TUINT32 udwTUid, TUINT32 udwWildId);

    TVOID ClearTimeoutAction(TUINT32 udwTimeoutMs, CTseLogger *poLog);

    TVOID InsertIntoMap(TUINT32 udwSvrId, TbAction *pstAction);
    TBOOL FindInMap(TUINT32 udwSvrId, TbAction *pstAction);
    TVOID FinishAction(TUINT32 udwSvrId, TbAction *pstAction, TUINT8 ucRawStatus);

    TVOID InsertIntoMap(TUINT32 udwSvrId, TbAlliance_action *pstAlAction);
    TBOOL FindInMap(TUINT32 udwSvrId, TbAlliance_action *pstAlAction);
    TVOID FinishAction(TUINT32 udwSvrId, TbAlliance_action *pstAlAction, TUINT8 ucRawStatus);

    TVOID InsertIntoMap(TUINT32 udwSvrId, TbMarch_action *ptbMarch);
    TBOOL FindInMap(TUINT32 udwSvrId, TbMarch_action *ptbMarch);
    TVOID FinishAction(TUINT32 udwSvrId, TbMarch_action *ptbMarch, TUINT8 ucRawStatus);
};

#endif