#ifndef _CONF_BASE_H_
#define _CONF_BASE_H_

#include <string>
#include <algorithm>
#include "base/common/wtse_std_header.h"
#include "base/conf/wtse_ini_configer.h"

using namespace std;
using wtse::conf::CTseIniConfiger;

#define DEFAULT_CONF_UPDATE_FLAG ("../conf/conf_update_flag")

class CConfBase
{
public:
    static string GetString(const string &szKey, const string strSection = "");
    static int GetInt(const string &szKey, const string strSection = "");
    static float GetFloat(const string &szKey, const string strSection = "");

    static int Init(const string szFilePath, const string szModule);
    static int Update();

private:

    static string m_szModule;
    static string m_szModuleCaps;

    static CTseIniConfiger m_config_obj;
};

#endif // !_CONF_BASE_H_

