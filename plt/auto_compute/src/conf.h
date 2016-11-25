#ifndef _SSC_CONF_H_
#define _SSC_CONF_H_

#include "base/common/wtse_std_header.h"
#include "base/conf/wtse_ini_configer.h"
#include "my_define.h"
//#include "hs_svr_conf.h"
//#include "aws_proxy_conf.h"

#define MAX_HS_SVR_NUM			(5)

class CConf
{
public:
    TINT32 m_dwModuleGroup;
    TCHAR m_szModuleName[DEFAULT_NAME_STR_LEN];
    TCHAR m_szModuleIp[DEFAULT_IP_LEN];
    TCHAR m_szModuleSvr[DEFAULT_SVR_LIST_STR_LEN];
    TINT32 m_dwModulePriority;




// public:
//     TCHAR m_szProject[DEFAULT_NAME_STR_LEN];
// 
// 	// �����̷�����ip�Ͷ˿�
// 	TCHAR m_szServerIP[DEFAULT_IP_LEN];
// 	TUINT16 m_uwServerPort;
// 	TUINT16 m_uwRegPort;
// 
// 	TUINT32 m_udwMirrorNum;
// 	TUINT32 m_udwCurIdx;
// 
// 	// ������м������߳����
// 	TUINT32 m_udwTaskQueueSize;
// 	TUINT32 m_udwTaskThreadNum;
// 
// 	// ��ʱʱ������
// 	TUINT32	m_udwHsTimeoutMs;
// 
// 	// ��ȡaction��Ϣ��ʱ����
// 	TUINT32 m_udwIntervalMs;
// 	TUINT32 m_udwActionTimeoutMs;
// 
// 	TCHAR m_szPriorLockSvrIp[DEFAULT_IP_LEN];
// 
// 	// �澯����
// 	TCHAR	m_project_name[DEFAULT_NAME_STR_LEN];
// 	TUINT32 m_stat_interval_time;
// 	TUINT32 m_need_send_message;
// 	TUINT32 m_error_num_threshold;
// 	TUINT32 m_error_rate_threshold;

public:
	TINT32 Init(const TCHAR *pszConfFile);

};

#endif
