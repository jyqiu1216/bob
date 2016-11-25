#include "aws_proxy_conf.h"
using wtse::conf::CTseIniConfiger;

TINT32 CAwsProxyConf::Init( const TCHAR *pszConfFile )
{
	CTseIniConfiger config_obj;
	char key_str[1024];
	bool ret_val = false;
	TUINT32 idx = 0;

	ret_val = config_obj.LoadConfig(pszConfFile);
	assert(ret_val == true);

	ret_val = config_obj.GetValue("AWS_PROXY_INFO", "aws_proxy_num", m_udwDbSvrNum);
	assert(ret_val == true);

	m_pudwDbIdToHsSvrId = new TUINT32[m_udwDbSvrNum];
	m_pszPriorHsSvrIp =	new TCHAR*[m_udwDbSvrNum];

	for(idx = 0; idx < m_udwDbSvrNum; idx++)
	{
		sprintf(key_str, "aws_proxy_id_%u", idx);
		ret_val = config_obj.GetValue("AWS_PROXY_INFO", key_str, m_pudwDbIdToHsSvrId[idx]);
		assert(ret_val == true);

		m_pszPriorHsSvrIp[idx] = new TCHAR[16];
		sprintf(key_str, "aws_proxy_prior_ip_%u", idx);
		ret_val = config_obj.GetValue("AWS_PROXY_INFO", key_str, m_pszPriorHsSvrIp[idx]);
		assert(ret_val == true);
	}

	return 0;
}

TINT32 CAwsProxyConf::GetSvrGroup( TUINT32 udwReqSvrId )
{
	TINT32 dwRet = -1;
	if(udwReqSvrId < m_udwDbSvrNum)
	{
		dwRet = m_pudwDbIdToHsSvrId[udwReqSvrId];
	}
	return dwRet;
}
