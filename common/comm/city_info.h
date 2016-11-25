#ifndef _CITY_INFO_H_
#define _CITY_INFO_H_

#include "base/common/wtse_std_header.h"
#include "game_define.h"
#include "bussiness_struct.h"

struct SCityInfo
{
    TbCity  m_stTblData;

    // production
    SResourceProduction m_astResProduction[EN_RESOURCE_TYPE__END];
    // action stat
    SCityActionStat m_stActionStat;

    // knight_res_cost
    TUINT64 m_uddwKnightCostGold;

    ///////////////////////////////////////////////
    TVOID Reset()
    {
        m_stTblData.Reset();

        memset(m_astResProduction, 0, sizeof(m_astResProduction));
        
        memset(&m_stActionStat, 0, sizeof(m_stActionStat));

        m_uddwKnightCostGold = 0;
    }
};

#endif

