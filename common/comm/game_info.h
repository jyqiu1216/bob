#ifndef _GAME_INFO_H_
#define _GAME_INFO_H_

#include "game_define.h"
#include "game_data.h"
#include "base/log/wtselogger.h"
#include "jsoncpp/json/json.h"
#include <map>
#include <vector>

using namespace std;
using namespace wtse::log;

#define UPDATE_GAME_JSON_FLAG_FILE      ("../data/update_game_json_flag")

class CGameInfo
{
public:
	static CGameInfo* m_poGameInfo;
	static CGameInfo* GetInstance();
    static  TINT32 Update( const TCHAR *pszFileName, CTseLogger *poLog );
public:
	TINT32			Init(const TCHAR *pszFileName, CTseLogger *poLog);
	TBOOL			GetBuildingInfo(TUINT32 udwType, TUINT32 udwLevel, SBuildingInfo *pstRetInfo);
    TINT64          GetResearchTotalNum();
	TBOOL			GetResearchInfo(TUINT32 udwType, TUINT32 udwLevel, SResearchInfo *pstRetInfo);
    TINT64          GetTroopTypeNum();
	TBOOL			GetTroopInfo(TUINT32 udwType, STroopInfo* pstRetInfo);
    STroopInfo*     GetTroopInfo(TUINT32 udwType);

    TBOOL			GetHealTroopInfo(TUINT32 udwType, STroopInfo* pstRetInfo);
    STroopInfo*     GetHealTroopInfo(TUINT32 udwType);

    TBOOL			GetHealFortInfo(TUINT32 udwType, STroopInfo* pstRetInfo);
    STroopInfo*     GetHealFortInfo(TUINT32 udwType);

    TINT64          GetFortTypeNum();
	TBOOL			GetFortInfo(TUINT32 udwType, STroopInfo *pstRetInfo);
    STroopInfo*     GetFortInfo(TUINT32 udwType);

    STroopInfo*     GetArmyInfo(TUINT32 udwType);

    TINT64          GetItemTotalNum();
	TBOOL			GetItemInfo(TUINT32 udwID, SItemInfo *pstRetInfo);
	TBOOL			GetQuestInfo(TUINT32 udwID, SQuestInfo *pstRetInfo);
	TBOOL			GetWildInfo(TUINT8 ucType, TUINT8 ucLevel, SWildInfo *pstRetInfo);

    TUINT32         ComputeKnightLevelByExp(TINT64 ddwExp);
    TINT64          GetKnightExpByLevel(TUINT32 udwLevel);
    TINT64          GetBasicVal(TUINT32 udwKey);
    
    TINT64          GetTopAlCommentNum();

public:
	Json::Value		m_oJsonRoot;
	CTseLogger*		m_poLog;

public:
    TINT32          InitArmyData();
    TVOID           LoadAttackOrder(AttackOrders& attack_orders);
    TVOID           LoadTroopAttackOrder(TUINT32 udwSourceTroopId, AttackOrders& attack_orders);
    TVOID           LoadFortAttackOrder(TUINT32 udwSourceFortId, AttackOrders& attack_orders);

    map<TINT32, STroopInfo*> m_mArmy;
    STroopInfo m_aTroop[EN_TROOP_TYPE__END];
    STroopInfo m_aFort[EN_FORT_TYPE__END];
    STroopInfo m_aTroopHeal[EN_TROOP_TYPE__END];
    STroopInfo m_aFortHeal[EN_FORT_TYPE__END];

    vector<TINT32>  m_avecTierList[MAX_ARMY_TIER_LIMIT];

    AttackOrders    m_objAttackFactor;

    TVOID LoadTaskList();
    const vector<TINT64>& GetTaskList();
    vector<TINT64> m_vecTaskList;
};

#endif
