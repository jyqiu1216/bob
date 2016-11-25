#ifndef _AWS_PROXY_CONF_H_
#define _AWS_PROXY_CONF_H_

#include "base/common/wtse_std_header.h"
#include "base/conf/wtse_ini_configer.h"

class CAwsProxyConf
{
public:
	TINT32 Init(const TCHAR *pszConfFile);
	TINT32 GetSvrGroup(TUINT32 udwReqSvrId);

public:
	TUINT32 m_udwDbSvrNum;
	TUINT32 *m_pudwDbIdToHsSvrId;
	TCHAR **m_pszPriorHsSvrIp;
};

#endif