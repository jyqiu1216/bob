#include "conf.h"

using wtse::conf::CTseIniConfiger;


TINT32 CConf::Init(const TCHAR *pszConfFile)
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

    std::string ModuleConfFile = "../conf/";
    ModuleConfFile = ModuleConfFile + m_szModuleName + ".conf";

    ret_val = config_obj.LoadConfig(ModuleConfFile);
    assert(ret_val == true);

    ret_val = config_obj.GetValue("URL_INFO", "hu_op_url_pre", m_szHuOpUrlPre);
    assert(ret_val == true);

    ret_val = config_obj.GetValue("URL_INFO", "chat_http_url_pre", m_szChatUrlPre);
    assert(ret_val == true);

    ret_val = config_obj.GetValue("URL_INFO", "rank_url_pre", m_szRankUrlPre);
    assert(ret_val == true);

    return 0;
}

