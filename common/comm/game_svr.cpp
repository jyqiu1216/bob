#include "game_svr.h"
#include <time.h>
#include "game_info.h"
#include "common_func.h"

CGameSvrInfo* CGameSvrInfo::m_poGameSvrInfo = NULL;


CGameSvrInfo* CGameSvrInfo::GetInstance()
{
	if(m_poGameSvrInfo == NULL)
	{
		m_poGameSvrInfo = new CGameSvrInfo;
	}
	return m_poGameSvrInfo;
}


CGameSvrInfo::CGameSvrInfo()
{
	m_fMinClientVersion = 0.0;
	m_fNewestClientVersion = 0.0;

	m_udwSvrUpdtBegTime = 0;
	m_udwSvrUpdtEndTime = 0;

	m_udwSvrNum = 0;
}

CGameSvrInfo::~CGameSvrInfo()
{
	m_fMinClientVersion = 0.0;
	m_fNewestClientVersion = 0.0;

	m_udwSvrUpdtBegTime = 0;
	m_udwSvrUpdtEndTime = 0;

	m_udwSvrNum = 0;
}

TINT32 CGameSvrInfo::Update( CTseLogger *poLog )
{
	TINT32 dwRetCode = 0;
	CGameSvrInfo *poSvrInfo = new CGameSvrInfo;
	CGameSvrInfo *poTmpSvrInfo = m_poGameSvrInfo;

	dwRetCode = poSvrInfo->Init("../data/client_version.info", "../data/server_updt.info", "../data/game_svr.info", poLog);
	if(dwRetCode != 0)
	{
		TSE_LOG_ERROR(poLog, (" CGameSvrInfo::Update failed[%d]", dwRetCode));
		delete poSvrInfo;
		return -1;
	}

	m_poGameSvrInfo = poSvrInfo;

	if(poTmpSvrInfo != NULL)
	{
	    sleep(5);
		delete poTmpSvrInfo;
	}

	TSE_LOG_INFO(poLog, (" CGameSvrInfo::Update ok", dwRetCode));

	return 0;
}

TINT32 CGameSvrInfo::Init( const TCHAR *pszClientConf, const TCHAR *pszSvrUpdtConf, const TCHAR *pszGameSvrConf, CTseLogger *poLog )
{
	TINT32 dwRetCode = 0;

    for (TUINT32 udwIdx = 0; udwIdx < MAX_GAME_SVR_NUM; udwIdx++)
    {
        m_astSvrInfo[udwIdx].Reset();
    }

	// 设置随机数种子
	srand(time(NULL));

	// 加载client版本
	dwRetCode = LoadClientVersion(pszClientConf, poLog);
	if(dwRetCode != 0)
	{
		return -1;
	}

	// 加载后台维护参数
	dwRetCode = LoadSvrUpdtParam(pszSvrUpdtConf, poLog);
	if(dwRetCode != 0)
	{
		return -2;
	}

	// 加载游戏服务器统计信息，主要是地图相关
	dwRetCode = LoadSvrInfo(pszGameSvrConf, poLog);
	if(dwRetCode != 0)
	{
		return -3;
	}

	// 加载黑名单
	dwRetCode = LoadBlackAccountList(BLACK_ACCOUNT_LIST_FILE, TRUE);
	if(dwRetCode != 0)
	{
		return -4;
	}

    // 加载合服配置文件
    dwRetCode = LoadSvrMergeInfo(SVR_MERGE_FILE, poLog);
    if(dwRetCode != 0)
    {
        return -5;
    }

	return 0;
}

TINT32 CGameSvrInfo::LoadClientVersion( const TCHAR *pszConfFile, CTseLogger *poLog )
{
	FILE *fp = fopen(pszConfFile, "rt");
	if(fp == NULL)
	{
		TSE_LOG_ERROR(poLog, ("LoadClientVersion: fail to open %s", pszConfFile));
		return -1;
	}

	TCHAR szLine[128];
	TCHAR *pCur = NULL;
	TUINT32 udwLen = 0;
	while(fgets(szLine, 128, fp))
	{
		pCur = strstr(szLine, CLIENT_VERSION_NEWEST_STR);
		if(pCur)
		{
			udwLen = strlen(CLIENT_VERSION_NEWEST_STR);
			m_fNewestClientVersion = atof(pCur+udwLen);
		}

		pCur = strstr(szLine, CLIENT_VERSION_MIN_STR);
		if(pCur)
		{
			udwLen = strlen(CLIENT_VERSION_MIN_STR);
			m_fMinClientVersion = atof(pCur+udwLen);
		}
	}
	fclose(fp);

	if(m_fNewestClientVersion > 0 && m_fMinClientVersion > 0)
	{
		TSE_LOG_INFO(poLog, ("LoadClientVersion: newest[%f], min[%f]", m_fNewestClientVersion, m_fMinClientVersion));
		return 0;
	}
	else
	{
		TSE_LOG_ERROR(poLog, ("LoadClientVersion: newest[%f], min[%f]", m_fNewestClientVersion, m_fMinClientVersion));
		return -2;
	}
}

TINT32 CGameSvrInfo::LoadSvrUpdtParam( const TCHAR *pszConfFile, CTseLogger *poLog )
{
	FILE *fp = fopen(pszConfFile, "rt");
	if(fp == NULL)
	{
		TSE_LOG_ERROR(poLog, ("LoadSvrUpdtParam: fail to open %s", pszConfFile));
		return -1;
	}

	TCHAR szLine[128];
	TCHAR *pCur = NULL;
	TUINT32 udwLen = 0;
	TUINT32 udwBegTime = 0, udwEndTime = 0;
	while(fgets(szLine, 128, fp))
	{
		pCur = strstr(szLine, SERVER_UPDT_BEG_TIME);
		if(pCur)
		{
			udwLen = strlen(SERVER_UPDT_BEG_TIME);
			udwBegTime = strtoul(pCur+udwLen, NULL, 10);
		}

		pCur = strstr(szLine, SERVER_UPDT_END_TIME);
		if(pCur)
		{
			udwLen = strlen(SERVER_UPDT_END_TIME);
			udwEndTime = strtoul(pCur+udwLen, NULL, 10);
		}
	}
	fclose(fp);

	if(udwEndTime < udwBegTime)
	{
		TSE_LOG_ERROR(poLog, ("LoadSvrUpdtParam: beg=%u, end=%u, interval=%d", \
			udwBegTime, udwEndTime, udwEndTime - udwBegTime));
		return -1;
	}
	else
	{
		TSE_LOG_INFO(poLog, ("LoadSvrUpdtParam: beg=%u, end=%u, interval=%d", \
			udwBegTime, udwEndTime, udwEndTime - udwBegTime));

		m_udwSvrUpdtBegTime = udwBegTime;
		m_udwSvrUpdtEndTime = udwEndTime;
	}
	
	return 0;
}

TINT32 CGameSvrInfo::LoadProvinceInfo( const TCHAR *pszConf, TUINT32 udwSvrId, CTseLogger *poLog )
{
	TINT32 dwProvinceId = 0;
	TINT32 dwProvinceNum = 0;
	TCHAR szLine[512];
	TCHAR *pCur = NULL;
	TBOOL bOk = TRUE;
	SProvinceInfo *pstProvince = NULL;

	FILE *fp = fopen(pszConf, "rt");
	if(fp == NULL)
	{
		TSE_LOG_ERROR(poLog, ("LoadProvinceInfo: fail to open %s", pszConf));
		return -1;
	}
	while(fgets(szLine, 512, fp))
	{
		pCur = szLine;
		if(pCur[0] < '0' || pCur[0] > '9')
		{
			bOk = FALSE;
			break;
		}
		dwProvinceId = atoi(pCur);

		if(dwProvinceId != dwProvinceNum)
		{
			bOk = FALSE;
			break;
		}

		pstProvince = &m_aastProvinceInfo[udwSvrId][dwProvinceId];

		pCur = strchr(pCur, '\t');
		pCur++;
		pstProvince->m_udwProbability = strtoul(pCur, NULL, 10);
		pCur = strchr(pCur, '\t');
		pCur++;
		pstProvince->m_udwPlainNum = strtoul(pCur, NULL, 10);
		pCur = strchr(pCur, '\t');
		pCur++;
		pstProvince->m_udwCityNum = strtoul(pCur, NULL, 10);
		
		dwProvinceNum++;
	}
	if(bOk == FALSE || dwProvinceNum != MAX_PROVINCE_NUM)
	{
		TSE_LOG_ERROR(poLog, ("LoadProvinceInfo: %s content is not right, province_num=%u", pszConf, dwProvinceNum));
		return -2;
	}
	else
	{
		TSE_LOG_INFO(poLog, ("LoadProvinceInfo: %s load ok, province_num=%u", pszConf, dwProvinceNum));
	}
	fclose(fp);

    // info for debug
    for (TUINT32 udwIdx=0; udwIdx<MAX_PROVINCE_NUM; ++udwIdx)
    {
		TSE_LOG_INFO(poLog, ("LoadDetailProvinceInfo: [sid=%u,pid=%u,%u,%u,%u]", 
            udwSvrId, udwIdx, 
            m_aastProvinceInfo[udwSvrId][udwIdx].m_udwProbability,
            m_aastProvinceInfo[udwSvrId][udwIdx].m_udwPlainNum,
            m_aastProvinceInfo[udwSvrId][udwIdx].m_udwCityNum));
    }
    
    
	return 0;
}

TUINT32 CGameSvrInfo::GetNewPlayerSvr()
{
	TUINT32 udwNewPlayerSvr = 0;
    TUINT32 udwRank = random()%SVR_SUM_NEW_IN_RANK;

    // 获取当前所有服务器的总概率值
    TUINT32 udwTotalRank = 0;
	for(TUINT32 idx = 0; idx < m_udwSvrNum; idx++)
	{
	    udwTotalRank += m_astSvrInfo[idx].m_udwNewInChance;
	}
    
    //TUINT32 udwRank = random()%SVR_SUM_NEW_IN_RANK;
    if (0<udwTotalRank)
    {
        udwRank = random()%udwTotalRank;
    }
    
	for(TUINT32 idx = 0; idx < m_udwSvrNum; idx++)
	{

        if (udwRank<m_astSvrInfo[idx].m_udwNewInChance)
        {
			udwNewPlayerSvr = idx;
            break;
        }
        udwRank -= m_astSvrInfo[idx].m_udwNewInChance;
	}

    TSE_LOG_INFO(CGameInfo::GetInstance()->m_poLog, ("ProcessCmd_LoginFake:[svr_total_num=%u total_rank=%u rank=%u new_svr_id=%u]", m_udwSvrNum, udwTotalRank, udwRank,udwNewPlayerSvr));

	return udwNewPlayerSvr;
}

TUINT32 CGameSvrInfo::GetNewPlayerProvince( TUINT32 udwSvrId, TUINT8 &ucProvince, TUINT32 &udwSkip )
{
    return 0;
    /*
	TUINT32 udwSum = 0;
	TUINT32 udwRand = rand();
	TUINT8 idx = 0;
	SProvinceInfo *pstProvince = NULL;

	ucProvince = rand()%MAX_PROVINCE_NUM;
	udwSkip = 0;

	// 计算sum
	for(idx = 0; idx < MAX_PROVINCE_NUM; idx++)
	{
		pstProvince = &m_aastProvinceInfo[udwSvrId][idx];
		if(pstProvince->m_udwProbability == 0 || pstProvince->m_udwPlainNum <= 500)
		{
			continue;
		}
		udwSum += pstProvince->m_udwProbability;
	}

	// 获取province
	udwRand %= udwSum;
	for(idx = 0; idx < MAX_PROVINCE_NUM; idx++)
	{
		pstProvince = &m_aastProvinceInfo[udwSvrId][idx];
		if(pstProvince->m_udwProbability == 0 || pstProvince->m_udwPlainNum <=500)
		{
			continue;
		}
		
		if(udwRand < pstProvince->m_udwProbability)
		{
			ucProvince = idx;
			if(pstProvince->m_udwPlainNum > 1000)
			{
				udwSkip = rand()%(pstProvince->m_udwPlainNum - 300);
			}
			else if(pstProvince->m_udwPlainNum > 100)
			{
				udwSkip = rand()%(pstProvince->m_udwPlainNum - 30);
			}
			else if(pstProvince->m_udwPlainNum > 10)
			{
				udwSkip = 0;
			}
			else
			{
				continue;
			}
			break;
		}
		udwRand -= pstProvince->m_udwProbability;
	}

	return 0;
    */
}

TINT32 CGameSvrInfo::LoadSvrInfo( const TCHAR *pszConf, CTseLogger *poLog )
{
	FILE *fp = fopen(pszConf, "rt");
	if(fp == NULL)
	{
		TSE_LOG_ERROR(poLog, ("LoadSvrInfo: open %s failed", pszConf));
		return -1;
	}

	TINT32 dwRetCode = 0;
	TCHAR szLine[512];
	TCHAR *pCur = NULL;
	SGameSvrInfo *pstSvr = NULL;
	TUINT32 udwSvrId = 0;
	TUINT32 udwLineNum = 0;

	while(fgets(szLine, 512, fp))
	{
		udwLineNum++;
		if(strlen(szLine) < 10)
		{
			TSE_LOG_ERROR(poLog, ("LoadSvrInfo: line[%u] not right", udwLineNum));
			dwRetCode = -2;
			break;
		}

		// id
		pCur = strtok(szLine, "\t\r\n");
		udwSvrId = strtoul(pCur, NULL, 10);
		if(udwSvrId >= MAX_GAME_SVR_NUM)
		{
			TSE_LOG_ERROR(poLog, ("LoadSvrInfo: svrid over thrd. line[%u]", udwLineNum));
			dwRetCode = -3;
			break;
		}
		pstSvr = &m_astSvrInfo[udwSvrId];
		pstSvr->m_udwId = udwSvrId;

		// name
		pCur = strtok(NULL, "\t\r\n");
		strncpy(pstSvr->m_szName, pCur, MAX_TABLE_NAME_LEN-1);
		pstSvr->m_szName[MAX_TABLE_NAME_LEN-1] = 0;

		// language
		pCur = strtok(NULL, "\t\r\n");
		strncpy(pstSvr->m_szLanguage, pCur, MAX_TABLE_NAME_LEN-1);
		pstSvr->m_szLanguage[MAX_TABLE_NAME_LEN-1] = 0;

		// opentime
		pCur = strtok(NULL, "\t\r\n");
		pstSvr->m_udwOpenTime = strtoul(pCur, NULL, 10);

		// status
		pCur = strtok(NULL, "\t\r\n");
		pstSvr->m_dwStatus = atoi(pCur);

		// plain num
		pCur = strtok(NULL, "\t\r\n");
		pstSvr->m_udwPlainNum = strtoul(pCur, NULL, 10);

		// city num
		pCur = strtok(NULL, "\t\r\n");
		pstSvr->m_udwCityNum = strtoul(pCur, NULL, 10);

		// thrd
		pCur = strtok(NULL, "\t\r\n");
		pstSvr->m_fThrdRate = atof(pCur);

		// valid flag
        pCur = strtok(NULL, "\t\r\n");
		pstSvr->m_bValidFlag = atoi(pCur);

        // new player in ratio
        pCur = strtok(NULL, "\t\r\n");
        pstSvr->m_udwNewInChance = atoi(pCur);

        // server pos
        pCur = strtok(NULL, "\t\r\n");
        pstSvr->m_udwSvrPos = atoi(pCur);

        // server avatar
        pCur = strtok(NULL, "\t\r\n");
        pstSvr->m_udwSvrAvatar = atoi(pCur);


		// svr num
		m_udwSvrNum = m_udwSvrNum < udwSvrId+1 ? udwSvrId+1 : m_udwSvrNum;

		TSE_LOG_INFO(poLog, ("LoadSvrInfo: svrid[%u] load ok", udwSvrId));
	}
	fclose(fp);
	if(dwRetCode < 0)
	{
		return dwRetCode;
	}

	//// 加载各服务器中province的信息
	//for(TUINT32 idx = 0; idx < m_udwSvrNum; idx++)
	//{
	//	pstSvr = &m_astSvrInfo[idx];
	//	if(pstSvr->m_bValidFlag == TRUE)
	//	{
	//		sprintf(szLine, "../data/province_%u.info", idx);
	//		dwRetCode = LoadProvinceInfo(szLine, idx, poLog);
	//		if(dwRetCode != 0)
	//		{
	//			dwRetCode = -3;
	//			TSE_LOG_ERROR(poLog, ("LoadProvinceInfo: fail to load %s", szLine));
	//			break;
	//		}
	//	}
	//}
	return dwRetCode;
}

TINT32 CGameSvrInfo::LoadBlackList( const TCHAR *pszFile )
{
	m_objBlackList.clear();

	FILE *fp = fopen(pszFile, "rt");
	if(fp == NULL)
	{
		return 0;
	}

	TCHAR szLine[256];
	TUINT32 udwUserId = 0;
	while(fgets(szLine, 256, fp))
	{
		udwUserId = strtoul(szLine, NULL, 10);
		if(udwUserId)
		{
			m_objBlackList.insert(udwUserId);
		}
	}

	fclose(fp);
	return 0;
}

TBOOL CGameSvrInfo::CheckSvrStatus( TUINT32 udwSvrId )
{
	if(udwSvrId < m_udwSvrNum)
	{
		TFLOAT32 fCurRate = m_astSvrInfo[udwSvrId].m_udwCityNum*1.0/(m_astSvrInfo[udwSvrId].m_udwCityNum + m_astSvrInfo[udwSvrId].m_udwPlainNum);
		if(fCurRate < SVR_FULL_THRD)
		{
			return TRUE;
		}
	}

	return FALSE;
}

TINT32 CGameSvrInfo::LoadPlainList( TUINT32 udwSvrId )
{
    // todo
    return 0;
}

TUINT32 CGameSvrInfo::GetEmptyPlain( TUINT32 udwSvrId, TUINT8 ucProvince )
{
    // todo
    return 1001+((time(NULL))%50);
}

TINT32 CGameSvrInfo::LoadBlackAccountList(const TCHAR *pszFile, TBOOL bIsInit /*= FALSE*/)
{
    if(bIsInit)
    {
        m_pobjMasterBlackAccount = &m_objBlackAccountListA;
        m_pobjBufferBlackAccount = &m_objBlackAccountListB;
    }

    FILE *fp = fopen(pszFile, "rt");
    if(fp == NULL)
    {
        return 0;
    }

    string strBlackAccount;
    m_pobjBufferBlackAccount->clear();
    TCHAR szLine[512];

    while(fgets(szLine, 512, fp))
    {
        TINT32 dwLn = strlen(szLine) - 1;
        if(szLine[dwLn] == '\n')
        {
            szLine[dwLn] = '\0';
        }

        strBlackAccount = szLine;
        
        if(strBlackAccount.length() > 0)
        {
            m_pobjBufferBlackAccount->insert(strBlackAccount);
        }
    }

    set<string>* pobjTmp = m_pobjMasterBlackAccount;
    m_pobjMasterBlackAccount = m_pobjBufferBlackAccount;
    m_pobjBufferBlackAccount = pobjTmp;

    fclose(fp);
    return 0;
}

TCHAR* CGameSvrInfo::GetSvrNameBySid(TUINT32 udwSvrId)
{
    return m_astSvrInfo[udwSvrId].m_szName;
}

TUINT32 CGameSvrInfo::GetSvrNewChanceInBySid( TUINT32 udwSvrId )
{
    return m_astSvrInfo[udwSvrId].m_udwNewInChance;
}

TINT32 CGameSvrInfo::LoadSvrMergeInfo( const TCHAR *pszConf, CTseLogger *poLog )
{
    FILE *fp = fopen(pszConf, "rt");
    if(fp == NULL)
    {
        TSE_LOG_ERROR(poLog, ("LoadSvrMergeInfo: open %s failed", pszConf));
        return 0;
    }

    TINT32 dwRetCode = 0;
    TCHAR szLine[512];
    TCHAR *pCur = NULL;
    SGameSvrInfo *pstSvr = NULL;
    TINT32 dwTargetSvrId = 0;
    TINT32 dwSidList[MAX_GAME_SVR_NUM];
    TUINT32 udwSidListSize = MAX_GAME_SVR_NUM;
    TUINT32 udwLineNum = 0;
    TINT32 dwStatus = 0;
    const TCHAR *pszRawSidList = NULL;

    while(fgets(szLine, 512, fp))
    {
        udwLineNum++;
        if(strlen(szLine) < 5)
        {
            TSE_LOG_ERROR(poLog, ("LoadSvrMergeInfo: line[%u] not right", udwLineNum));
            dwRetCode = -2;
            break;
        }

        // target sid
        pCur = strtok(szLine, "\t\r\n");
        dwTargetSvrId = atoi(pCur);
        if((TUINT32)dwTargetSvrId != m_astSvrInfo[dwTargetSvrId].m_udwId)
        {
            TSE_LOG_ERROR(poLog, ("LoadSvrMergeInfo: line[%u] not right, not exist target sid=%d", udwLineNum, dwTargetSvrId));
            dwRetCode = -3;
            break;
        }

        // raw sid list
        pCur = strtok(NULL, "\t\r\n");
        CCommonFunc::GetArrayFromString(pCur, ',', dwSidList, udwSidListSize);
        pszRawSidList = pCur;

        // status
        pCur = strtok(NULL, "\t\r\n");
        dwStatus = atoi(pCur);

        //process
        for(unsigned int idx = 0; idx < udwSidListSize; idx++)
        {
            pstSvr = &m_astSvrInfo[dwSidList[idx]];
            pstSvr->m_dwMergeStatus = dwStatus;
            pstSvr->m_dwMergeTargetSid = dwTargetSvrId;
        }

        TSE_LOG_INFO(poLog, ("LoadSvrMergeInfo: tsid=%d, raw_sidlist=%s, status=%d", dwTargetSvrId, pszRawSidList, dwStatus));
    }
    fclose(fp);

    return dwRetCode;
}

TINT32 CGameSvrInfo::GetTargeSid( TUINT32 udwRawSid )
{
    TINT32 dwRealSid = -1;
    if(m_astSvrInfo[udwRawSid].m_dwMergeTargetSid >= 0 && m_astSvrInfo[udwRawSid].m_dwMergeStatus == EN_SVR_MERGE_STATUS__DONE)
    {
        dwRealSid = m_astSvrInfo[udwRawSid].m_dwMergeTargetSid;
    }
    else
    {
        dwRealSid = udwRawSid;
    }

    return dwRealSid;
}

SGameSvrInfo* CGameSvrInfo::GetSidInfo( TUINT32 udwSid )
{
    SGameSvrInfo *pstInfo = &m_astSvrInfo[udwSid]; 
    return pstInfo;
}
