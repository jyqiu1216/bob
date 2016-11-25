#include "single_server.h"
#include "time_utils.h"

SingleServer::SingleServer(TINT32 _sid)
{
    pthread_mutex_init(&lock_, NULL);
    update_time = CTimeUtils::GetUnixTime();
    sid = _sid;
    data_a_.init();
    data_b_.init();
    current_data_ = &data_a_;
    old_data_ = &data_b_;
}

SingleServer::~SingleServer()
{
    pthread_mutex_destroy(&lock_);
}

TINT32 SingleServer::updateData(const TCHAR* player_file, const TCHAR* al_file, const TCHAR* recommend_file)
{
    GuardMutex Guard(lock_);
    old_data_->updateData(player_file, al_file, recommend_file);
    old_data_->updateAlData(current_data_->getUpdateAlInfo());
    std::swap(current_data_, old_data_);
    update_time = CTimeUtils::GetUnixTime();
    return 0;
}

TINT32 SingleServer::getPlayerRank(TINT32 uid, PlayerRankInfo* player_rank_info)
{
    return current_data_->getPlayerRank(uid, player_rank_info);
}

TINT32 SingleServer::getPlayerRank(TINT32 rank_type, TINT32 uid, PlayerRankInfo* player_rank_info)
{
    return current_data_->getPlayerRank(rank_type, uid, player_rank_info);
}

TINT32 SingleServer::getTopPlayerRank(TINT32 rank_type, PlayerRankInfo* player_rank_infos, TINT32& player_num)
{
    return current_data_->getTopPlayerRank(rank_type, player_rank_infos, player_num);
}

TINT32 SingleServer::getTopSinglePlayerRank(TINT32 rank_type, PlayerRankInfo* player_rank_info)
{
    return current_data_->getTopSinglePlayerRank(rank_type, player_rank_info);
}


TINT32 SingleServer::NewsearchPlayer(TINT32 rank_type, std::string keyword, TINT32 offset, TINT32& player_num, TINT32& total_num, PlayerRankInfo* player_rank_infos)
{
    return current_data_->NewsearchPlayer(rank_type, keyword, offset, player_num, total_num, player_rank_infos);
}


TINT32 SingleServer::searchPlayer(std::string keyword, TINT32 offset, TINT32& player_num, TINT32& total_num, PlayerRankInfo* player_rank_infos)
{
    return current_data_->searchPlayer(keyword, offset, player_num, total_num, player_rank_infos);
}

TINT32 SingleServer::getAlRank(TINT32 aid, AllianceRankInfo* al_rank_info)
{
    return current_data_->getAlRank(aid, al_rank_info);
}

TINT32 SingleServer::getAlRank(TINT32 rank_type, TINT32 aid, AllianceRankInfo* al_rank_info)
{
    return current_data_->getAlRank(rank_type, aid, al_rank_info);
}

TINT32 SingleServer::getTopAlRank(TINT32 rank_type, AllianceRankInfo* al_rank_infos, TINT32& al_num)
{
    return current_data_->getTopAlRank(rank_type, al_rank_infos, al_num);
}

TINT32 SingleServer::getTopSingleAlRank(TINT32 rank_type, AllianceRankInfo* al_rank_info)
{
    return current_data_->getTopSingleAlRank(rank_type, al_rank_info);
}

TINT32 SingleServer::getAlList(TINT32 join_policy, TINT32 offset, TINT32& al_num, TINT32& total_num, AllianceRankInfo* al_rank_infos)
{
    return current_data_->getAlList(join_policy, offset, al_num, total_num, al_rank_infos);
}

TINT32 SingleServer::searchAl(TINT32 join_policy, std::string keyword, TINT32 offset, TINT32& al_num, TINT32& total_num, AllianceRankInfo* al_rank_infos)
{
    return current_data_->searchAl(join_policy, keyword, offset, al_num, total_num, al_rank_infos);
}

TINT32 SingleServer::changeAlPolicy(TINT32 aid, TINT32 policy)
{
    GuardMutex Guard(lock_);
    return current_data_->changeAlPolicy(aid, policy);
}

TINT32 SingleServer::changeAlName(TINT32 aid, const std::string& alname)
{
    GuardMutex Guard(lock_);
    return current_data_->changeAlName(aid, alname);
}

TINT32 SingleServer::changeAlNick(TINT32 aid, const std::string& alnick)
{
    GuardMutex Guard(lock_);
    return current_data_->changeAlNick(aid, alnick);
}

TINT32 SingleServer::changeAlLang(TINT32 aid, TINT32 lang)
{
    GuardMutex Guard(lock_);
    return current_data_->changeAlLang(aid, lang);
}

TINT32 SingleServer::addAlRank(TINT32 aid, TINT32 join_policy, const std::string& alname, const std::string& alnick, TINT64 value3, TINT64 value9, TINT32 new_player_flag)
{
    GuardMutex Guard(lock_);
    return current_data_->addAlRank(aid, join_policy, alname, alnick, value3, value9, new_player_flag);
}


TINT32 SingleServer::delAlRank(TINT32 aid)
{
    GuardMutex Guard(lock_);
    return current_data_->delAlRank(aid);

}


TINT32 SingleServer::getRecommendAl(TINT32 new_player_flag, TINT32& al_num, RecommendAllianceInfo* recommend_al_infos)
{
    return current_data_->getRecommendAl(new_player_flag, al_num, recommend_al_infos);
}
