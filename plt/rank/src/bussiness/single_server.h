#ifndef _SINGLE_SERVER_H_
#define _SINGLE_SERVER_H_

#include "server_data.h"
#include "guard_mutex.h"

class SingleServer
{
public:
    SingleServer(TINT32 _sid);
    ~SingleServer();

    TINT32 updateData(const TCHAR* player_file, const TCHAR* al_file, const TCHAR* recommend_file);

    TINT32 getPlayerRank(TINT32 uid, PlayerRankInfo* player_rank_info);
    TINT32 getPlayerRank(TINT32 rank_type, TINT32 uid, PlayerRankInfo* player_rank_info);
    TINT32 getTopPlayerRank(TINT32 rank_type, PlayerRankInfo* player_rank_infos, TINT32& player_num);
    TINT32 getTopSinglePlayerRank(TINT32 rank_type, PlayerRankInfo* player_rank_info);
    TINT32 searchPlayer(std::string keyword, TINT32 offset, TINT32& player_num, TINT32& total_num, PlayerRankInfo* player_rank_infos);
    TINT32 NewsearchPlayer(TINT32 rank_type, std::string keyword, TINT32 offset, TINT32& player_num, TINT32& total_num, PlayerRankInfo* player_rank_infos);

    TINT32 getAlRank(TINT32 aid, AllianceRankInfo* al_rank_info);
    TINT32 getAlRank(TINT32 rank_type, TINT32 aid, AllianceRankInfo* al_rank_info);
    TINT32 getTopAlRank(TINT32 rank_type, AllianceRankInfo* al_rank_infos, TINT32& al_num);
    TINT32 getTopSingleAlRank(TINT32 rank_type, AllianceRankInfo* al_rank_info);
    TINT32 getAlList(TINT32 join_policy, TINT32 offset, TINT32& al_num, TINT32& total_num, AllianceRankInfo* al_rank_infos);
    TINT32 searchAl(TINT32 join_policy, std::string keyword, TINT32 offset, TINT32& al_num, TINT32& total_num, AllianceRankInfo* al_rank_infos);

    TINT32 changeAlPolicy(TINT32 aid, TINT32 policy);
    TINT32 changeAlName(TINT32 aid, const std::string& alname);
    TINT32 changeAlNick(TINT32 aid, const std::string& alnick);
    TINT32 changeAlLang(TINT32 aid, TINT32 lang);

    TINT32 addAlRank(TINT32 aid, TINT32 join_policy, const std::string& alname, const std::string& alnick,
                     TINT64 value3, TINT64 value9, TINT32 new_player_flag);

    TINT32 delAlRank(TINT32 aid);

    TINT32 getRecommendAl(TINT32 new_player_flag, TINT32& al_num, RecommendAllianceInfo* recommend_al_infos);

    TINT32 sid;
    TUINT32 update_time;
private:

    ServerData data_a_;
    ServerData data_b_;

    ServerData* current_data_;
    ServerData* old_data_;

    pthread_mutex_t lock_;
};

#endif
