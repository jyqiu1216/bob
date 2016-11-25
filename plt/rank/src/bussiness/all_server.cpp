#include "all_server.h"

AllServer* AllServer::instance_ = NULL;

AllServer* AllServer::init(CTseLogger *logger, const TCHAR *conf, const std::string self_ip)
{
    if(instance_ == NULL)
    {
        instance_ = new AllServer();
    }
    instance_->logger_ = logger;

    using wtse::conf::CTseIniConfiger;
    CTseIniConfiger config_obj;
    TINT32 svr_num = 0;
    TCHAR key_name[128];
    TBOOL ret = FALSE;

    ret = config_obj.LoadConfig(conf);
    assert(ret == TRUE);

    ret = config_obj.GetValue("RANK_CONF", "ServerNum", svr_num);
    assert(ret == TRUE);
    for(TUINT32 idx = 0; idx < svr_num; idx++)
    {
        string target_ip;
        sprintf(key_name, "server_%u_ip", idx);
        ret = config_obj.GetValue("RANK_CONF", key_name, target_ip);
        assert(ret == TRUE);
        if(target_ip != self_ip)
        {
            continue;
        }

        TINT32 sid = 0;
        sprintf(key_name, "server_%u_id", idx);
        ret = config_obj.GetValue("RANK_CONF", key_name, sid);
        assert(ret == TRUE);

        SingleServer* single_server = new SingleServer(sid);
        instance_->servers_.insert(make_pair(sid, single_server));
    }
    return instance_;
}

AllServer* AllServer::instance()
{
    return instance_;
}

SingleServer* AllServer::getSingleServer(TINT32 sid)
{
    std::map<TINT32, SingleServer*>::iterator it = servers_.find(sid);
    if(it != servers_.end())
    {
        return it->second;
    }
    return NULL;
}

TINT32 AllServer::updateData()
{
    for(std::map<TINT32, SingleServer*>::iterator it = servers_.begin(); it != servers_.end(); ++it)
    {
        TINT32 sid = it->second->sid;

        TCHAR player_file[128];
        TCHAR al_file[128];
        TCHAR recommend_file[128];

        sprintf(player_file, "../data/player_sid_%d.file", sid);
        sprintf(al_file, "../data/alliance_sid_%d.file", sid);
        sprintf(recommend_file, "../data/recommend_sid_%d.file", sid);

        if(access(player_file, F_OK) != 0)
        {
            TSE_LOG_INFO(logger_, ("update server data failed!No player data file![sid=%d]", sid));
            continue;
        }
        if(access(al_file, F_OK) != 0)
        {
            TSE_LOG_INFO(logger_, ("update server data failed!No alliance data file![sid=%d]", sid));
            continue;
        }
        if(access(recommend_file, F_OK) != 0)
        {
            TSE_LOG_INFO(logger_, ("update server data failed!No recommend data file![sid=%d]", sid));
            continue;
        }
        
        it->second->updateData(player_file, al_file, recommend_file);
        TSE_LOG_INFO(logger_, ("update server data ok![sid=%d]", sid));
    }

    return 0;
}

AllServer::~AllServer()
{
    for(std::map<TINT32, SingleServer*>::iterator it = servers_.begin(); it != servers_.end(); ++it)
    {
        if(it->second != NULL)
        {
            delete (it->second);
            it->second = NULL;
        }
    }
}

AllServer::AllServer()
{
    logger_ = NULL;
}
