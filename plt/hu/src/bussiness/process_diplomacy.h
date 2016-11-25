#ifndef _DIPLOMACY_H_
#define _DIPLOMACY_H_

#include "session.h"

class CDiplomacyProcess
{
public:
    static TINT32 CheckDiplomacy(TbDiplomacy *pstList, TUINT32 udwNum, TUINT32 udwTargetAlliance);

public:
    static TINT32 TableRequest_GetDiplomacyByTwoAid(SSession *pstSession, TUINT32 udwSource, TUINT32 udwDestination);

    static TINT32 TableRequest_DiplomacyChange(SSession *pstSession, TbDiplomacy *pstDiplomacy, TUINT8 ucFlag);
    
public:
    static TVOID TbDiplomacy_SetKey(TbDiplomacy *pstDiplomacy, TUINT32 udwSource, TUINT32 udwDestination, TUINT8 ucType)
    {
        pstDiplomacy->Set_Src_al(udwSource);
        pstDiplomacy->Set_Des_al(udwDestination);
        pstDiplomacy->Set_Type(ucType);
    }

};

#endif