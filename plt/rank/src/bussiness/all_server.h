#ifndef _ALL_SERVER_H_
#define _ALL_SERVER_H_

#include "single_server.h"
#include "base/log/wtselogger.h"
#include "base/conf/wtse_ini_configer.h"

#define RANK_CONF_FILE ("../conf/rank.conf")
#define RANK_UPDATE_FLAG ("../data/rank.flag")

using wtse::log::CTseLogger;

class AllServer
{
public:
    ~AllServer();

    static AllServer* init(CTseLogger *logger, const TCHAR *pszConfFile, const std::string strIp);
    static AllServer* instance();

    SingleServer* getSingleServer(TINT32 sid);
    TINT32 updateData();

private:
    AllServer();

    static AllServer* instance_;
    std::map<TINT32, SingleServer*> servers_;
    CTseLogger* logger_;
};


#endif
