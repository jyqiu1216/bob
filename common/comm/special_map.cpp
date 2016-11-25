#include "special_map.h"
#include "game_define.h"

CSpecailMap *CSpecailMap::m_poIns = NULL;
CSpecailMap* CSpecailMap::m_poBackup = NULL;

TINT32 CSpecailMap::Update()
{
    CSpecailMap *pTmp = new CSpecailMap();

    std::ifstream is;
    is.open(MAP_TYPE_FILE, std::ios::binary);
    if (is.is_open())
    {
        TUCHAR szMapDataBuff[MAX_X * MAX_Y + 1];
        TINT32 dwCurLen = 0;
        while (!is.eof())
        {
            if (dwCurLen == MAX_X * MAX_Y + 1)
            {
                break;
            }
            is.read((char *)szMapDataBuff + dwCurLen, MAX_X * MAX_Y + 1 - dwCurLen);
            dwCurLen += is.gcount();
        }
        if (is.eof())
        {
            TINT32 dwMapPos = 0;
            TINT32 dwType = 0;
            TINT32 dwIndex = 0;
            for (TINT32 dwIdx = 1; dwIdx <= MAX_X; ++dwIdx)
            {
                for (TINT32 dwIdy = 1; dwIdy <= MAX_Y; ++dwIdy)
                {
                    if ((dwIdx + dwIdy) % 2 != 0)
                    {
                        continue;
                    }

                    dwMapPos = dwIdx * LOCATION_X_OFFSET + dwIdy;
                    dwIndex = dwIdx - 1 + (dwIdy - 1) * MAX_X;
                    dwIndex -= (dwIdx + 1) % 2;
                    dwType = szMapDataBuff[dwIndex] >> 4;

                    if (11 == dwType) //11对应后台的 SPECIAL_LAKE(8)
                    {
                        pTmp->specail_map_pos.insert(dwMapPos);
                    }
                }
            }
        }
        else
        {
            assert(0);
        }
        is.close();
    }

    if (m_poBackup != NULL)
    {
        delete(m_poBackup);
        m_poBackup = NULL;
    }

    m_poBackup = m_poIns;

    m_poIns = pTmp;

    return 0;
}

TBOOL CSpecailMap::bIsSpecialMap(TUINT32 udwPos)
{
    return m_poIns->specail_map_pos.count(udwPos) > 0;
}