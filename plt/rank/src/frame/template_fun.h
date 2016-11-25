#ifndef _TEMPLATE_FUN_H_
#define _TEMPLATE_FUN_H_

template<class UserInfoType>
static void SetKeyVal(TUCHAR *pucData, TUINT32 &udwCurLen, TUINT32 type, TUINT32 num, UserInfoType *pUsrData)
{
	udwCurLen = 0;

	if(0 == num)
	{
		return;
	}

	*(TUINT32*)pucData = type;
	pucData += sizeof(TUINT32);
	udwCurLen += sizeof(TUINT32);

	*(TUINT32*)pucData = num;
	pucData += sizeof(TUINT32);
	udwCurLen += sizeof(TUINT32);

	memcpy(pucData, (char*)pUsrData, sizeof(UserInfoType)*num);
	pucData += sizeof(UserInfoType)*num;
	udwCurLen += sizeof(UserInfoType)*num;
}

template <class UserInfoType>
static void GetKeyVal(TUCHAR *pucData, TUINT32 &udwCurLen, TUINT32 num, UserInfoType *pUsrData)
{
	udwCurLen = sizeof(UserInfoType)*num;
	memcpy(pUsrData, pucData, udwCurLen);
}

template <class UserInfoType>
static UserInfoType* GetItemInfoByBinAsc(TUINT32 udwDstID, UserInfoType *pList, TUINT32 udwListSize, TINT32 &dwItemIndex)
{
	if(udwListSize == 0 || NULL == pList)
	{
		return NULL;
	}

	// 优先使用dwItemIndex来判定是否就是要找的目标
	if(dwItemIndex < udwListSize && dwItemIndex >= 0)
	{
		if(udwDstID == pList[dwItemIndex].m_udwID)
		{
			return &pList[dwItemIndex];
		}
	}

	// 二分查找目标
	UserInfoType *pItem = NULL;
	dwItemIndex = -1;

	TINT32 dwLow = 0, dwHigh = udwListSize-1, dwMid = 0;
	TINT32 dwCount = 0;

	while (dwLow <= dwHigh)
	{
		dwMid = (dwLow + dwHigh)/2;
		dwCount++;

		if(udwDstID == pList[dwMid].m_udwID)
		{
			dwItemIndex = dwMid;
			pItem = &pList[dwMid];
			break;
		}
		else if(udwDstID > pList[dwMid].m_udwID)
		{
			dwLow = dwMid + 1;
		}
		else
		{
			dwHigh = dwMid - 1;
		}
	}

	return pItem;
}

template <class UserInfoType>
static UserInfoType* GetItemInfoByBinDes(TUINT32 udwDstID, UserInfoType *pList, TUINT32 udwListSize, TINT32 &dwItemIndex)
{
	if(udwListSize == 0 || NULL == pList)
	{
		return NULL;
	}

	// 二分查找目标
	UserInfoType *pItem = NULL;
	dwItemIndex = -1;

	TINT32 dwLow = 0, dwHigh = udwListSize-1, dwMid = 0;
	TINT32 dwCount = 0;

	while (dwLow <= dwHigh)
	{
		dwMid = (dwLow + dwHigh)/2;
		dwCount++;

		if(udwDstID == pList[dwMid].m_udwID)
		{
			dwItemIndex = dwMid;
			pItem = &pList[dwMid];
			break;
		}
		else if(udwDstID > pList[dwMid].m_udwID)
		{
			dwHigh = dwMid - 1;
		}
		else
		{
			dwLow = dwMid + 1;
		}
	}

	return pItem;
}

template <class UserInfoType>
static UserInfoType* GetItemInfoBySeq(TUINT32 udwDstID, UserInfoType *pList, TUINT32 udwListSize, TINT32 &dwItemIndex)
{
	// 优先使用dwItemIndex来判定是否就是要找的目标
	if(dwItemIndex < udwListSize && dwItemIndex >= 0)
	{
		if(udwDstID == pList[dwItemIndex].m_udwID)
		{
			return &pList[dwItemIndex];
		}
	}

	// 顺序查找
	UserInfoType *pItem = NULL;
	dwItemIndex = -1;

	for(TINT32 idx = 0; idx < udwListSize; idx++)
	{
		if(udwDstID == pList[idx].m_udwID)
		{
			dwItemIndex = idx;
			pItem = &pList[idx];
			break;
		}
	}

	return pItem;
}

#endif