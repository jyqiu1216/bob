#ifndef _PLAYER_RANK_H_
#define _PLAYER_RANK_H_

#include <string>
using namespace std;

typedef __int64_t               TINT64;

enum EPlayerRankFld
{
	EN_PLAYER_RANK_FLD_UID = 0,
	EN_PLAYER_RANK_FLD_UNAME,
	EN_PLAYER_RANK_FLD_LEVEL,
	EN_PLAYER_RANK_FLD_AID,
	EN_PLAYER_RANK_FLD_ALNAME,
	EN_PLAYER_RANK_FLD_CID,
    EN_PLAYER_RANK_FLD_RANK0,
	EN_PLAYER_RANK_FLD_RANK1,
	EN_PLAYER_RANK_FLD_RANK2,
	EN_PLAYER_RANK_FLD_RANK3,
	EN_PLAYER_RANK_FLD_RANK4,
	EN_PLAYER_RANK_FLD_RANK5,
    EN_PLAYER_RANK_FLD_RANK6,
    EN_PLAYER_RANK_FLD_RANK7,
    EN_PLAYER_RANK_FLD_RANK8,
    EN_PLAYER_RANK_FLD_RANK9,
    EN_PLAYER_RANK_FLD_RANK10,
    EN_PLAYER_RANK_FLD_RANK11,
    EN_PLAYER_RANK_FLD_RANK12,
    EN_PLAYER_RANK_FLD_RANK13,
    EN_PLAYER_RANK_FLD_RANK14,
    EN_PLAYER_RANK_FLD_RANK15,
    
    EN_PLAYER_RANK_FLD_VALUE0,
    EN_PLAYER_RANK_FLD_VALUE1,
    EN_PLAYER_RANK_FLD_VALUE2,
    EN_PLAYER_RANK_FLD_VALUE3,
    EN_PLAYER_RANK_FLD_VALUE4,
    EN_PLAYER_RANK_FLD_VALUE5,
    EN_PLAYER_RANK_FLD_VALUE6,
    EN_PLAYER_RANK_FLD_VALUE7,
    EN_PLAYER_RANK_FLD_VALUE8,
    EN_PLAYER_RANK_FLD_VALUE9,
    EN_PLAYER_RANK_FLD_VALUE10,
    EN_PLAYER_RANK_FLD_VALUE11,
    EN_PLAYER_RANK_FLD_VALUE12,
    EN_PLAYER_RANK_FLD_VALUE13,
    EN_PLAYER_RANK_FLD_VALUE14,
    EN_PLAYER_RANK_FLD_VALUE15,
    EN_PLAYER_RANK_FLD_END,
};

class PlayerRank
{
public:
	unsigned int	udwUid;
	string		    sUname;
	unsigned int	udwLevel;
	unsigned int	udwAid;
	string 		    sAlName;
	unsigned int	udwCid;
	unsigned int	udwRank0;
	unsigned int	udwRank1;
	unsigned int	udwRank2;
	unsigned int	udwRank3;
	unsigned int	udwRank4;
	unsigned int	udwRank5;
	unsigned int 	udwRank6;
	unsigned int 	udwRank7;
	unsigned int 	udwRank8;
	unsigned int	 udwRank9;
	unsigned int	 udwRank10;
	unsigned int	 udwRank11;
	unsigned int	 udwRank12;
	unsigned int	 udwRank13;
	unsigned int	 udwRank14;
	unsigned int	 udwRank15;

    
    
	TINT64	 ddwValue0;
	TINT64 	 ddwValue1;
	TINT64	 ddwValue2;
	TINT64 	ddwValue3;
	TINT64	ddwValue4;
	TINT64	ddwValue5;
	TINT64	ddwValue6;
	TINT64	ddwValue7;
	TINT64	ddwValue8;
	TINT64	ddwValue9;
	TINT64	ddwValue10;
	TINT64	ddwValue11;
	TINT64	ddwValue12;
	TINT64	ddwValue13;
	TINT64	ddwValue14;
	TINT64	ddwValue15;
    
    //以上字段和msyql表字段对应

   //以下字段为业务逻辑服务字段
    TINT64		ddwSendHelpNum; //请求帮助数量
    TINT64		ddwRecvHelpNum; //收到帮助的数量

    //
	unsigned int    udwIsNpc;
    unsigned int    audwRank[20];
    unsigned int    udwRank;
    TINT64    addwValue[20];
    TINT64    ddwValue;
    unsigned int udwRankType;
    PlayerRank()
    {
        udwRankType = 0;
    }
};

#endif
