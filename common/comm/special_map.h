#ifndef _SPECIAL_MAP_H_
#define _SPECIAL_MAP_H_

#include <stdio.h>
#include <set>
#include "base/common/wtse_std_header.h"

#define UPDATE_MAP_TYPE_FLAG ("../data/update_map_type_flag")
#define MAP_TYPE_FILE ("../data/map_type.file")

#define MAX_X 500
#define MAX_Y 1200
#define LOCATION_X_OFFSET 10000

class CSpecailMap
{
private:
    static CSpecailMap* m_poIns;
    static CSpecailMap* m_poBackup;

    CSpecailMap()
    {
        specail_map_pos.clear();
    }

public:
    ~CSpecailMap()
    {
        specail_map_pos.clear();
    }

    static TINT32 Update();

public:
    static TBOOL bIsSpecialMap(TUINT32 udwPos);

private:
    std::set<TUINT32> specail_map_pos;
};

#endif


