#include "process_diplomacy.h"
#include "aws_request.h"

TINT32 CDiplomacyProcess::TableRequest_GetDiplomacyByTwoAid(SSession *pstSession, TUINT32 udwSource, TUINT32 udwDestination)
{
	TbDiplomacy tbDiplomacyItem;
    tbDiplomacyItem.Set_Src_al(udwSource);
    tbDiplomacyItem.Set_Des_al(udwDestination);
	return CAwsRequest::GetItem(pstSession, &tbDiplomacyItem, ETbDIPLOMACY_OPEN_TYPE_PRIMARY);
}


TINT32 CDiplomacyProcess::TableRequest_DiplomacyChange(SSession *pstSession, TbDiplomacy *pstDiplomacy, TUINT8 ucFlag)
{
    if(ucFlag == EN_TABLE_UPDT_FLAG__CHANGE)
    {
        return CAwsRequest::UpdateItem(pstSession, pstDiplomacy);
    }
    else if(ucFlag == EN_TABLE_UPDT_FLAG__NEW)
    {
        return CAwsRequest::UpdateItem(pstSession, pstDiplomacy);
    }
    else if(ucFlag == EN_TABLE_UPDT_FLAG__DEL)
    {
        return CAwsRequest::DeleteItem(pstSession, pstDiplomacy);
    }

    return 0;
}

TINT32 CDiplomacyProcess::CheckDiplomacy( TbDiplomacy *pstList, TUINT32 udwNum, TUINT32 udwTargetAlliance )
{
	if(udwTargetAlliance == 0)
	{
		return 0;
	}

	TINT32 dwRetCode = 0;
	for(TUINT32 idx = 0; idx < udwNum; idx++)
	{
        if(pstList[idx].m_nDes_al == udwTargetAlliance)
		{
			if(pstList[idx].m_nType == EN_DIPLOMACY_TYPE__FRIENDLY)
			{
				dwRetCode = -1;
				break;
			}
		}
	}

    return dwRetCode;
}
