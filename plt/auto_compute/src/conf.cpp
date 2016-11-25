#include "conf.h"

using wtse::conf::CTseIniConfiger;

TINT32 CConf::Init( const TCHAR *pszConfFile )
{
	CTseIniConfiger config_obj;
	bool ret_val = false;

	ret_val = config_obj.LoadConfig(pszConfFile);
	assert(ret_val == true);

    ret_val = config_obj.GetValue("SERV_INFO", "module_group", m_dwModuleGroup);
    assert(ret_val == true);
    ret_val = config_obj.GetValue("SERV_INFO", "module_name", m_szModuleName);
    assert(ret_val == true);
    ret_val = config_obj.GetValue("SERV_INFO", "module_ip", m_szModuleIp);
    assert(ret_val == true);
    ret_val = config_obj.GetValue("SERV_INFO", "module_svr", m_szModuleSvr);
    assert(ret_val == true);
    ret_val = config_obj.GetValue("SERV_INFO", "module_priority", m_dwModulePriority);
    assert(ret_val == true);

    return 0;
}
