#ifndef _DB_STRUCT_DEFINE_H_
#define _DB_STRUCT_DEFINE_H_

#include <string>
using namespace std;

typedef __int64_t               TINT64;

enum EPlayerRecommendFld
{
    EN_PLAYER_RECOMMEND_FLD_AID = 0,
    EN_PLAYER_RECOMMEND_FLD_SID,
    EN_PLAYER_RECOMMEND_FLD_UID,
    EN_PLAYER_RECOMMEND_FLD_UNAME,
    EN_PLAYER_RECOMMEND_FLD_AVATAR,
    EN_PLAYER_RECOMMEND_FLD_HERO_LEVEL,
    EN_PLAYER_RECOMMEND_FLD_CASTLE_LEVEL,
    EN_PLAYER_RECOMMEND_FLD_FORCE,
    EN_PLAYER_RECOMMEND_FLD_INVITED,
    EN_PLAYER_RECOMMEND_FLD_END,
};

class PlayerRecommend
{
public:
    unsigned int    udwAid;
    unsigned int    udwSid;
    unsigned int    udwUid;
    string          sUname;
    unsigned int    udwAvatar;
    unsigned int    udwLordLv;
    unsigned int    udwCastleLv;
    TINT64          ddwForce;
    unsigned int    udwInvited;

    PlayerRecommend()
    {
        udwAid = 0;
        udwSid = 0;
        udwUid = 0;
        sUname = "";
        udwAvatar = 0;
        udwLordLv = 0;
        udwCastleLv = 0;
        ddwForce = 0;
        udwInvited = 0;
    }
    TVOID Reset()
    {
        udwAid = 0;
        udwSid = 0;
        udwUid = 0;
        sUname = "";
        udwAvatar = 0;
        udwLordLv = 0;
        udwCastleLv = 0;
        ddwForce = 0;
        udwInvited = 0;
    }
};

enum EComputeTimeType
{
    EN_COMPUTE_TIME_RANK = 1,
    EN_COMPUTE_TIME_RECOMMEND = 2,
};

enum EComputeTimeFld
{
    EN_COMPUTE_TIME_FLD_TYPE = 0,
    EN_COMPUTE_TIME_FLD_TIME = 1,
};

class ComputeTime
{
public:
    unsigned int	udwType;
    TINT64          ddwTime;

    ComputeTime()
    {
        udwType = EN_COMPUTE_TIME_RANK;
        ddwTime = 0;
    }
    TVOID Reset()
    {
        udwType = EN_COMPUTE_TIME_RANK;
        ddwTime = 0;
    }
};

enum EAllianceRankFld
{
    EN_ALLIANCE_RANK_FLD_AID = 0,
    EN_ALLIANCE_RANK_FLD_ALNAME,
    EN_ALLIANCE_RANK_FLD_OID,
    EN_ALLIANCE_RANK_FLD_ONAME,
    EN_ALLIANCE_RANK_FLD_MIGHT,
    EN_ALLIANCE_RANK_FLD_MEMBER,
    EN_ALLIANCE_RANK_FLD_POLICY,
    EN_ALLIANCE_RANK_FLD_DESC,
    EN_ALLIANCE_RANK_FLD_LANGUAGE,
    EN_ALLIANCE_RANK_FLD_RANK0,
    EN_ALLIANCE_RANK_FLD_RANK1,
    EN_ALLIANCE_RANK_FLD_RANK2,
    EN_ALLIANCE_RANK_FLD_RANK3,
    EN_ALLIANCE_RANK_FLD_RANK4,
    EN_ALLIANCE_RANK_FLD_RANK5,
    EN_ALLIANCE_RANK_FLD_RANK6,
    EN_ALLIANCE_RANK_FLD_RANK7,
    EN_ALLIANCE_RANK_FLD_RANK8,
    EN_ALLIANCE_RANK_FLD_RANK9,
    EN_ALLIANCE_RANK_FLD_RANK10,
    EN_ALLIANCE_RANK_FLD_RANK11,
    EN_ALLIANCE_RANK_FLD_RANK12,
    EN_ALLIANCE_RANK_FLD_RANK13,
    EN_ALLIANCE_RANK_FLD_VALUE0,
    EN_ALLIANCE_RANK_FLD_VALUE1,
    EN_ALLIANCE_RANK_FLD_VALUE2,
    EN_ALLIANCE_RANK_FLD_VALUE3,
    EN_ALLIANCE_RANK_FLD_VALUE4,
    EN_ALLIANCE_RANK_FLD_VALUE5,
    EN_ALLIANCE_RANK_FLD_VALUE6,
    EN_ALLIANCE_RANK_FLD_VALUE7,
    EN_ALLIANCE_RANK_FLD_VALUE8,
    EN_ALLIANCE_RANK_FLD_VALUE9,
    EN_ALLIANCE_RANK_FLD_VALUE10,
    EN_ALLIANCE_RANK_FLD_VALUE11,
    EN_ALLIANCE_RANK_FLD_VALUE12,
    EN_ALLIANCE_RANK_FLD_VALUE13,
    EN_ALLIANCE_RANK_FLD_RANK14,
    EN_ALLIANCE_RANK_FLD_VALUE14,
    EN_ALLIANCE_RANK_FLD_ALNICK,
    EN_ALLIANCE_RANK_FLD_NEWPLAYERALFLAG,
    EN_ALLIANCE_RANK_FLD_END,
};

enum EAlListType
{
    EN_AL_LIST_TYPE_APPLY_JOIN = 0,
    EN_AL_LIST_TYPE_NO_APPLY_JOIN = 1,
    EN_AL_LIST_TYPE_ALL = 2,
};

class AllianceRank
{
public:
    unsigned int    udwAid;
    string          sAlName;
    unsigned int    udwOid; //ÃËÖ÷id
    string          sOname; //ÃËÖ÷
    TINT64          ddwMight;
    unsigned int    udwMember;
    unsigned int    udwPolicy;
    unsigned int    udwLanguage;
    string          sDesc;
    unsigned int    udwuage;
    unsigned int    udwRank0;
    unsigned int    udwRank1;
    unsigned int    udwRank2;
    unsigned int    udwRank3;
    unsigned int    udwRank4;
    unsigned int    udwRank5;
    unsigned int    udwRank6;
    unsigned int    udwRank7;
    unsigned int    udwRank8;
    unsigned int    udwRank9;
    unsigned int    udwRank10;
    unsigned int    udwRank11;
    unsigned int    udwRank12;
    unsigned int    udwRank13;
    unsigned int    udwRank14;
    TINT64          ddwValue0;
    TINT64          ddwValue1;
    TINT64          ddwValue2;
    TINT64          ddwValue3;
    TINT64          ddwValue4;
    TINT64          ddwValue5;
    TINT64          ddwValue6;
    TINT64          ddwValue7;
    TINT64          ddwValue8;
    TINT64          ddwValue9;
    TINT64          ddwValue10;
    TINT64          ddwValue11;
    TINT64          ddwValue12;
    TINT64          ddwValue13;
    TINT64          ddwValue14;
    string          sAlNick;
    TINT64          ddwNewPlayerAlFlag;
    unsigned int    udwIsNpc;
    unsigned int    audwRank[15];
    TINT64          addwValue[15];
};


enum ETradeCityInfoFld
{
    EN_TRADE_CITY_INFO_FLD_SID = 0,
    EN_TRADE_CITY_INFO_FLD_UID,
    EN_TRADE_CITY_INFO_FLD_X,
    EN_TRADE_CITY_INFO_FLD_Y,
    EN_TRADE_CITY_INFO_FLD_CITYNAME,
    EN_TRADE_CITY_INFO_FLD_UTIME,
};
class TradeCityInfo
{
public:
    TINT32 dwSid;
    TINT32 dwUid;
    TINT32 dwX;
    TINT32 dwY;
    string sCityName;
    TUINT32 udwUTime;

    TVOID Reset()
    {
        dwSid = 0;
        dwUid = 0;
        dwX = 0;
        dwY = 0;
        sCityName.clear();
        udwUTime = 0;
    }
};

#endif
