#ifndef _SERVER_RANK_INFO_H_
#define _SERVER_RANK_INFO_H_
#include <vector>
#include "game_define.h"
#include "player_rank_info.h"
#include "alliance_rank_info.h"

#define DEFAULT_PLAYER_NUM (100000)
#define DEFAULT_ALLIANCE_NUM (5000)
#define DEFAULT_RECOMMEND_AL_NUM (500)
#define DEFAULT_TOP_NUM (100)
#define RANK_TYPE_NUM (33)
#define DEFAULT_NUM_ONE_PAGE (20)
#define MAX_NAME_LEN (32)

static const int kRankTypes[] = {
    EN_RANK_TYPE_PLAYER_CHAMPION,
    EN_RANK_TYPE_PLAYER_FORCE_KILL,
    EN_RANK_TYPE_PLAYER_TROOPS_KILL,
    EN_RANK_TYPE_PLAYER_FORCE,
    EN_RANK_TYPE_PLAYER_TROOPS_KDR,
    EN_RANK_TYPE_PLAYER_BATTLE_WON,
    EN_RANK_TYPE_PLAYER_BWLR,  //Battles Won/Lost Ration
    EN_RANK_TYPE_PLAYER_DRAGON_CAPTURED,
    EN_RANK_TYPE_PLAYER_DRAGON_EXECUTED,
    EN_RANK_TYPE_PLAYER_EVIL_TROOP_KILL,
    EN_RANK_TYPE_PLAYER_EVIL_FORCE_KILL,
    EN_RANK_TYPE_PLAYER_GAIN_RESOURCE,
    EN_RANK_TYPE_PLAYER_TRANSPORT_RESOURCE_0,
    EN_RANK_TYPE_PLAYER_TRANSPORT_RESOURCE_1,
    EN_RANK_TYPE_PLAYER_TRANSPORT_RESOURCE_2,
    EN_RANK_TYPE_PLAYER_TRANSPORT_RESOURCE_3,
    EN_RANK_TYPE_PLAYER_TRANSPORT_RESOURCE_4,
    EN_RANK_TYPE_PLAYER_KILL_MONSTER,
    EN_RANK_TYPE_ALLIANCE_CHAMPION,
    EN_RANK_TYPE_ALLIANCE_FORCE_KILL,
    EN_RANK_TYPE_ALLIANCE_TROOPS_KILL,
    EN_RANK_TYPE_ALLIANCE_FORCE,
    EN_RANK_TYPE_ALLIANCE_TROOPS_KDR,
    EN_RANK_TYPE_ALLIANCE_BATTLE_WON,
    EN_RANK_TYPE_ALLIANCE_BWLR,  //Battles Won/Lost Ration
    EN_RANK_TYPE_ALLIANCE_HELP,
    EN_RANK_TYPE_ALLIANCE_FUND,
    EN_RANK_TYPE_ALLIANCE_GIFT_POINT,
    EN_RANK_TYPE_ALLIANCE_DRAGON_CAPTURED,
    EN_RANK_TYPE_ALLIANCE_DRAGON_EXECUTED,
    EN_RANK_TYPE_ALLIANCE_EVIL_TROOP_KILL,
    EN_RANK_TYPE_ALLIANCE_EVIL_FORCE_KILL,
    EN_RANK_TYPE_ALLIANCE_THRONE_OCCUPY,
};

static const TINT32 kAlRankTypeOrder[] = {
    EN_RANK_TYPE_ALLIANCE_FORCE,
    EN_RANK_TYPE_ALLIANCE_FORCE_KILL,
    EN_RANK_TYPE_ALLIANCE_TROOPS_KILL,
    EN_RANK_TYPE_ALLIANCE_TROOPS_KDR,
    EN_RANK_TYPE_ALLIANCE_BATTLE_WON,
    EN_RANK_TYPE_ALLIANCE_BWLR,  //Battles Won/Lost Ration
    EN_RANK_TYPE_ALLIANCE_HELP,
    EN_RANK_TYPE_ALLIANCE_FUND,
    EN_RANK_TYPE_ALLIANCE_GIFT_POINT,
    EN_RANK_TYPE_ALLIANCE_DRAGON_CAPTURED,
    EN_RANK_TYPE_ALLIANCE_DRAGON_EXECUTED,
    //EN_RANK_TYPE_ALLIANCE_EVIL_TROOP_KILL,
    //EN_RANK_TYPE_ALLIANCE_EVIL_FORCE_KILL,
    EN_RANK_TYPE_ALLIANCE_THRONE_OCCUPY,
};

static const TINT32 kPlayerRankTypeOrders[] = {
    EN_RANK_TYPE_PLAYER_FORCE,
    EN_RANK_TYPE_PLAYER_FORCE_KILL,
    EN_RANK_TYPE_PLAYER_TROOPS_KILL,
    EN_RANK_TYPE_PLAYER_TROOPS_KDR,
    EN_RANK_TYPE_PLAYER_BATTLE_WON,
    EN_RANK_TYPE_PLAYER_BWLR,
    EN_RANK_TYPE_PLAYER_DRAGON_CAPTURED,
    EN_RANK_TYPE_PLAYER_DRAGON_EXECUTED,
    EN_RANK_TYPE_PLAYER_EVIL_TROOP_KILL,
    EN_RANK_TYPE_PLAYER_EVIL_FORCE_KILL,
    EN_RANK_TYPE_PLAYER_GAIN_RESOURCE,
    EN_RANK_TYPE_PLAYER_TRANSPORT_RESOURCE_0,
    EN_RANK_TYPE_PLAYER_TRANSPORT_RESOURCE_1,
    EN_RANK_TYPE_PLAYER_TRANSPORT_RESOURCE_2,
    EN_RANK_TYPE_PLAYER_TRANSPORT_RESOURCE_3,
    EN_RANK_TYPE_PLAYER_TRANSPORT_RESOURCE_4,
    EN_RANK_TYPE_PLAYER_KILL_MONSTER,
};

class ServerData
{
public:
    ServerData();
    ~ServerData();
    TVOID init();
    TVOID clear();
    TINT32 updateData(const TCHAR* player_file, const TCHAR* al_file, const TCHAR* recommend_file);
    TINT32 updateAlData(std::map<TINT32, AllianceRankInfo> &update_alliance_infos);
    
    TINT32 loadPlayerRank(const TCHAR* file_name);
    TINT32 loadAllianceRank(const TCHAR* file_name);
    TINT32 loadRecommendAl(const TCHAR* file_name);

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

    PlayerRankInfo getPlayerRank(TINT32 uid);
    std::vector<PlayerRankInfo> getTopPlayerRank(TINT32 rank_type);
    AllianceRankInfo getAlRank(TINT32 aid);
    std::vector<AllianceRankInfo> getTopAlRank(TINT32 rank_type);

    std::map<TINT32, AllianceRankInfo>& getUpdateAlInfo();

private:
    TBOOL matchWord(const TCHAR* str, const TCHAR* substr);

    TINT32 updateTop(const PlayerRankInfo& player_rank_info);
    TINT32 updateTop(const AllianceRankInfo& al_rank_info);
    std::vector<PlayerRankInfo> player_infos_;
    std::map<TINT32, TINT32> player_index_;

    std::vector<AllianceRankInfo> alliance_infos_;
    std::map<TINT32, TINT32> alliance_index_;

    std::map<TINT32, AllianceRankInfo> update_alliance_infos_;

    std::vector<RecommendAllianceInfo> recommend_al_infos_;

    typedef std::vector<TINT32> top_ids_t;
    std::map<TINT32, top_ids_t> rank_top_id_;

    PlayerRankInfo tmp_player_rank_info_;
    AllianceRankInfo tmp_al_rank_info_;
    RecommendAllianceInfo tmp_recommend_al_info_;
};

#endif

