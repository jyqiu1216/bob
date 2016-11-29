#ifndef _SSC_CONF_H_
#define _SSC_CONF_H_

#include "base/common/wtse_std_header.h"
#include "base/conf/wtse_ini_configer.h"
#include "my_define.h"
#include <string>

class CConf
{
public:
    TINT32 m_dwModuleGroup;
    TCHAR m_szModuleName[DEFAULT_NAME_STR_LEN];
    TCHAR m_szModuleIp[DEFAULT_IP_LEN];
    TCHAR m_szModuleSvr[DEFAULT_SVR_LIST_STR_LEN];
    TINT32 m_dwModulePriority;
    TCHAR m_szHuOpUrlPre[DEFAULT_NAME_STR_LEN];
    TCHAR m_szChatUrlPre[DEFAULT_NAME_STR_LEN];
    TCHAR m_szRankUrlPre[DEFAULT_NAME_STR_LEN];

public:
    TINT32 Init(const TCHAR *pszConfFile);
};

#endif
