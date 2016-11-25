#ifndef _SVR_LIST_CONF_H_
#define _SVR_LIST_CONF_H_

#include "base/common/wtse_std_header.h"
#include "base/conf/wtse_ini_configer.h"
#include "base/log/logthread.h"
#include "my_define.h"
#include "game_define.h"

using wtse::log::CTseLogger;

#define UPDATE_SVR_LIST_FLAG_FILE       ("../conf/upt_vaild_action_table.conf.flag")

struct SActionTable
{
    TINT32 dwTableType;
    TINT32 dwTableIdx;
//    std::set<TINT32> setTableSid;
    std::string strTableName;

    TVOID Reset()
    {
        dwTableType = EN_UID_ACTION;
        dwTableIdx = 0;
//        setTableSid.clear();
        strTableName = "";
    }
};

class CActionTableConf
{
public:
    static string m_strIp;
    static CActionTableConf* m_poSvrListConf;
    static CActionTableConf* GetInstance();
    static TINT32 Update(const TCHAR *pszConfFile, CTseLogger *poLog);

public:
    // 游戏服务器列表,主要是指本模块需要处理的游戏服务器list
    TUINT32 m_udwTableNum;
    std::vector<SActionTable> m_vecTable;

public:
    TINT32 Init(const TCHAR *pszConfFile, const string strIp = "");
};

#endif
