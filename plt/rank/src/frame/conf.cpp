#include "conf.h"

using wtse::conf::CTseIniConfiger;

TINT32 CConf::Init( const TCHAR *pszConfFile )
{
    CTseIniConfiger config_obj;
    bool ret_val = false;

    ret_val = config_obj.LoadConfig(pszConfFile);
    assert(ret_val == true);

    ret_val = config_obj.GetValue("SERV_INFO", "module_name", m_szModule);
    assert(ret_val == true);
    ret_val = config_obj.GetValue("SERV_INFO", "module_group", m_dwModuleGroup);
    assert(ret_val == true);
    ret_val = config_obj.GetValue("SERV_INFO", "module_ip", m_szModuleIp);
    assert(ret_val == true);
    ret_val = config_obj.GetValue("SERV_INFO", "module_svr", m_szModuleSvr);
    assert(ret_val == true);
    ret_val = config_obj.GetValue("SERV_INFO", "module_priority", m_dwModulePriority);
    assert(ret_val == true);

    return 0;
}

TVOID CConf::TrimStr(TCHAR *szSrc, TCHAR cChar)
{
    TINT32 szSrcLen = strlen(szSrc);
    TCHAR *szTmp = new TCHAR[szSrcLen + 1];
    strcpy(szTmp, szSrc);
    szTmp[szSrcLen - 1] = '\0';
    memmove(szSrc, szTmp + 1, szSrcLen - 1);
    delete[]szTmp;
}
