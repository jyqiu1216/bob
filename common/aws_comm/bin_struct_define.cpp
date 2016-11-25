#include "bin_struct_define.h"


TVOID SActionBuildingParam::SetValue(TINT64 ddwPos, TINT64 ddwType, TINT64 ddwLevel, TINT64 ddwExp, TCHAR* pszUserName)
{
    m_ddwPos = ddwPos;
    m_ddwType = ddwType;
    m_ddwTargetLevel = ddwLevel;
    m_ddwExp = ddwExp;
    strncpy(m_szUserName, pszUserName, MAX_TABLE_NAME_LEN - 1);
    m_szUserName[MAX_TABLE_NAME_LEN - 1] = '\0';
}

TVOID SActionTrainParam::SetValue(TINT64 ddwType, TINT64 ddwNum, TINT64 ddwExp, TCHAR* pszUserName)
{
    m_ddwType = ddwType;
    m_ddwNum = ddwNum;
    m_ddwExp = ddwExp;
    strncpy(m_szUserName, pszUserName, MAX_TABLE_NAME_LEN - 1);
    m_szUserName[MAX_TABLE_NAME_LEN - 1] = '\0';
}

TVOID SActionItemParam::SetValue(TINT64 ddwBufferId, TINT64 ddwNum, TINT64 ddwTime, TINT64 ddwItemId/* = 0*/)
{
    m_ddwBufferId = ddwBufferId;
    m_ddwNum = ddwNum;
    m_ddwTime = ddwTime;
    m_ddwItemId = ddwItemId;
}

TVOID SActionEquipParam::SetValue(TUINT64 uddwId, TINT64 ddwEid, TINT64 ddwScrollId, TINT64 ddwLevel, TINT64 ddwGoldCost, TCHAR* pszMaterialIdList, TCHAR* pszUserName)
{
    m_ddwEType = ddwEid;
    m_uddwId = uddwId;
    m_ddwScrollId = ddwScrollId;
    strncpy(m_szMaterialIdList, pszMaterialIdList, 63);
    m_szMaterialIdList[63] = '\0';

    strncpy(m_szUserName, pszUserName, MAX_TABLE_NAME_LEN - 1);
    m_szUserName[MAX_TABLE_NAME_LEN - 1] = '\0';

    m_ddwLevel = ddwLevel;

    m_ddwGoldCost = ddwGoldCost;
}
