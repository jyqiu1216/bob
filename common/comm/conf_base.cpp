#include "conf_base.h"

#include <assert.h>
#include <fstream>
#include <iostream>

using namespace std;
using wtse::conf::CTseIniConfiger;

string CConfBase::m_szModule = "";
string CConfBase::m_szModuleCaps = "";
CTseIniConfiger CConfBase::m_config_obj;

int CConfBase::Init(const string szFilePath, const string szModule)
{
    if (szModule != "")
    {
        m_szModule = szModule;
        m_szModuleCaps = szModule;
    }
    else
    {
        return -1;
    }

    transform(m_szModuleCaps.begin(), m_szModuleCaps.end(), m_szModuleCaps.begin(), (int(*)(int))toupper);
    m_szModuleCaps += "_INFO";

    TBOOL ret_val;
    ret_val = m_config_obj.LoadConfig(szFilePath);
    if (!ret_val)
    {
        return -2;
    }

    return 0;
}

int CConfBase::Update()
{
    CTseIniConfiger tmp;
    TBOOL ret_val;

    ret_val = tmp.LoadConfig("../conf/module.conf");
    if (!ret_val)
    {
        return -1;
    }
    else
    {
        m_config_obj = tmp;
        return 0;
    }
}

string CConfBase::GetString(const string &szKey, const string strSection)
{
    string ret_str = ""; 
    string tmp = "";
    TBOOL ret_val = FALSE;

    if("" != strSection)
    {
        string strSectionCaps = strSection;
        transform(strSectionCaps.begin(), strSectionCaps.end(), strSectionCaps.begin(), (int(*)(int))toupper);
        strSectionCaps += "_INFO";
        
        tmp = strSection + "_" + szKey;
        ret_val = m_config_obj.GetValue(strSectionCaps, tmp, ret_str);
        assert(ret_val == true);
    }
    else
    {
        if (szKey == "project")
        {
            ret_val = m_config_obj.GetValue("PROJECT_INFO", szKey, ret_str);
            assert(ret_val == true);
        }
        else if (szKey == "tbxml_project")
        {
            ret_val = m_config_obj.GetValue("PROJECT_INFO", szKey, ret_str);
            assert(ret_val == true);
        }
        else
        {
            tmp = m_szModule + "_" + szKey;
            ret_val = m_config_obj.GetValue(m_szModuleCaps, tmp, ret_str);
            assert(ret_val == true);
        }        
    }

    return ret_str;
}

int CConfBase::GetInt(const string &szKey, const string strSection)
{
    int ret_int = 0;
    string tmp = "";
    TBOOL ret_val = FALSE;


    if("" != strSection)
    {
        string strSectionCaps = strSection;
        transform(strSectionCaps.begin(), strSectionCaps.end(), strSectionCaps.begin(), (int(*)(int))toupper);
        strSectionCaps += "_INFO";
        
        tmp = strSection + "_" + szKey;
        ret_val = m_config_obj.GetValue(strSectionCaps, tmp, ret_int);
        assert(ret_val == true);
    }
    else
    {
        tmp = m_szModule + "_" + szKey;
        ret_val = m_config_obj.GetValue(m_szModuleCaps, tmp, ret_int);
        assert(ret_val == true);
    }
    return ret_int;


}

float CConfBase::GetFloat(const string &szKey, const string strSection)
{
//    float ret_flo = 0.0;
    float ret_flo = 0;
    string tmp = "";
    TBOOL ret_val = FALSE;


    if("" != strSection)
    {
        string strSectionCaps = strSection;
        transform(strSectionCaps.begin(), strSectionCaps.end(), strSectionCaps.begin(), (int(*)(int))toupper);
        strSectionCaps += "_INFO";
        
        tmp = strSection + "_" + szKey;
        ret_val = m_config_obj.GetValue(strSectionCaps, tmp, ret_flo);
        assert(ret_val == true);
    }
    else
    {
        tmp = m_szModule + "_" + szKey;
        ret_val = m_config_obj.GetValue(m_szModuleCaps, tmp, ret_flo);
        assert(ret_val == true);
    }


    return ret_flo;
}
