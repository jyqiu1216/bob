#ifndef _DB_REQUEST_H_
#define _DB_REQUEST_H_

#include "time_utils.h"
#include <string>
#include <vector>
#include "aws_table_player.h"
//#include "player_info.h"
//#include "session.h"

using namespace std;


struct SSession;
struct SReqParam;
class AllianceRank;
class TradeCityInfo;

class CDbRequest
{
public:
	static int Select(SSession *pstSession, const string& sSql, const string& sTableName, const string& sOperName);
    static int UpdateAllianceRank(SSession *pstSession, AllianceRank* pAllianceRank);
    static int UpdateAlliancePolicy(SSession *pstSession, AllianceRank* pAllianceRank);
    static int UpdateAllianceName(SSession *pstSession, AllianceRank* pAllianceRank);
    static int UpdateAllianceNick(SSession *pstSession, AllianceRank* pAllianceRank);
    static int DeleteAllianceRank(SSession *pstSession);
    static int SelectAllianceRank(SSession *pstSession, TUINT32 udwAid);
    static int DeletePlayerRank(SSession *pstSession);
    static int UpdatePlayerName(SSession *pstSession, const string& sName);

    static int SelectRecommendCount(SSession *pstSession, TUINT32 udwAid);
    static int SelectRecommendTime(SSession *pstSession);
    static int DeletePlayerRecommend(SSession* pstSession, TUINT32 udwAid, TINT32 dwUid);
    static int DeleteAlPlayerRecommend(SSession* pstSession, TUINT32 udwAid, TINT32 dwUid);
    static int UpdatePlayerRecommendToInvited(SSession* pstSession, TUINT32 udwAid, TINT32 dwUid);
    static int SelectRecommendPlayer(SSession *pstSession, TUINT32 udwAid);
    static int AddRecommendPlayer(SSession *pstSession, TUINT32 udwAid, TbPlayer* ptbPlayer);
    static int UpdateRecommendTime(SSession *pstSession, TUINT32 udwTime);

    static int SelectTradeCityInfoInRange(SSession *pstSession, TINT32 dwSid, TINT32 dwX, TINT32 dwY, TINT32 dwX1, TINT32 dwX2, TINT32 dwY1, TINT32 dwY2, TUINT32 udwNum);
    static int SelectTradeCityInfoInRandom(SSession *pstSession, TINT32 dwSid, TUINT32 udwNum);
};

#endif


