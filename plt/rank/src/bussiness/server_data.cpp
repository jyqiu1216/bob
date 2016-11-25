#include "server_data.h"
#include "time_utils.h"
#include <cstring>
#include "global_serv.h"

ServerData::ServerData()
{

}

ServerData::~ServerData()
{

}

TVOID ServerData::init()
{
    player_infos_.reserve(DEFAULT_PLAYER_NUM);
    alliance_infos_.reserve(DEFAULT_ALLIANCE_NUM);
    recommend_al_infos_.reserve(DEFAULT_RECOMMEND_AL_NUM);

    for(TUINT32 index = 0; index < RANK_TYPE_NUM; ++index)
    {
        rank_top_id_.insert(std::make_pair(kRankTypes[index], top_ids_t()));
        rank_top_id_.at(kRankTypes[index]).reserve(DEFAULT_TOP_NUM);
    }
}

TVOID ServerData::clear()
{
    player_infos_.clear();
    alliance_infos_.clear();
    recommend_al_infos_.clear();
    update_alliance_infos_.clear();
    alliance_index_.clear();
    player_index_.clear();

    for(TUINT32 index = 0; index < RANK_TYPE_NUM; ++index)
    {
        rank_top_id_.at(kRankTypes[index]).clear();
        for(TUINT32 count = 0; count < DEFAULT_TOP_NUM; ++count)
        {
            rank_top_id_.at(kRankTypes[index]).push_back(0);
        }
    }
}

TINT32 ServerData::updateData(const TCHAR* player_file, const TCHAR* al_file, const TCHAR* recommend_file)
{
    clear();
    loadPlayerRank(player_file);
    loadAllianceRank(al_file);
    loadRecommendAl(recommend_file);
    return 0;
}


TINT32 ServerData::updateAlData(std::map<TINT32, AllianceRankInfo> &update_alliance_infos)
{       
    TSE_LOG_INFO(CGlobalServ::m_poServLog, ("ServerData::updateAlData begin size=%d", \
                                            update_alliance_infos.size()));
     
    
    std::map<TINT32, AllianceRankInfo>::iterator it_update = update_alliance_infos.begin();
    while(it_update != update_alliance_infos.end())
    {
    
        TSE_LOG_INFO(CGlobalServ::m_poServLog, ("ServerData::updateAlData aid=%d, join_policy=%d, alnick=%s, alname=%s",\
                                                it_update->first,\ 
                                                it_update->second.join_policy, \
                                                it_update->second.alnick, \
                                                it_update->second.alname));
      
        std::map<TINT32, TINT32>::iterator it_old = alliance_index_.find(it_update->first);
        if(it_old != alliance_index_.end())
        {
            if(0 != it_update->second.udwUpdateTime)
            {
                alliance_infos_[it_old->second].join_policy = it_update->second.join_policy; 
            
                strncpy(alliance_infos_[it_old->second].alnick, it_update->second.alnick, 5);
                alliance_infos_[it_old->second].alnick[4] = '\0';
                
                strncpy(alliance_infos_[it_old->second].alname, it_update->second.alname, MAX_NAME_LEN);
                alliance_infos_[it_old->second].alname[MAX_NAME_LEN - 1] = '\0';    
            }
            else
            {   
                alliance_infos_[it_old->second].udwUpdateTime = 0; 
            }
        }
        else
        {
            if(0 != it_update->second.udwUpdateTime)
            {
                alliance_infos_.push_back(it_update->second);
                alliance_index_.insert(std::make_pair(it_update->first, alliance_infos_.size() - 1));
            }
        }
        ++it_update;
    }
    return 0;
}

TINT32 ServerData::loadPlayerRank(const TCHAR* file_name)
{
    FILE *fp = fopen(file_name, "rt");
    if(fp == NULL)
    {
        return 0;
    }

    TCHAR line[1024];
    while(fgets(line, 1024, fp))
    {
        tmp_player_rank_info_.reset();
        if(tmp_player_rank_info_.load_data(line) != 0)
        {
            continue;
        }

        player_infos_.push_back(tmp_player_rank_info_);
        player_index_.insert(std::make_pair(tmp_player_rank_info_.uid, player_infos_.size() - 1));
        //std::map<TINT32, TINT32>::iterator it = player_index_.find(tmp_player_rank_info_.uid);
        //if(it == player_index_.end())
        //{
        //    player_index_.insert(std::make_pair(tmp_player_rank_info_.uid, player_infos_.size() - 1));
        //}
        //else
        //{
        //    player_index_.at(tmp_player_rank_info_.uid) = player_infos_.size() - 1;
        //}

        updateTop(tmp_player_rank_info_);
    }

    fclose(fp);
    return 0;
}

TINT32 ServerData::loadAllianceRank(const TCHAR* file_name)
{
    FILE *fp = fopen(file_name, "rt");
    if(fp == NULL)
    {
        return 0;
    }

    TCHAR line[1024];
    while(fgets(line, 1024, fp))
    {
        tmp_al_rank_info_.reset();
        if(tmp_al_rank_info_.load_data(line) != 0)
        {
            continue;
        }

        alliance_infos_.push_back(tmp_al_rank_info_);
        alliance_index_.insert(std::make_pair(tmp_al_rank_info_.aid, alliance_infos_.size() - 1));
        //std::map<TINT32, TINT32>::iterator it = alliance_index_.find(tmp_al_rank_info_.aid);
        //if(it == alliance_index_.end())
        //{
        //    alliance_index_.insert(std::make_pair(tmp_al_rank_info_.aid, alliance_infos_.size() - 1));
        //}
        //else
        //{
        //    alliance_index_.at(tmp_al_rank_info_.aid) = alliance_infos_.size() - 1;
        //}

        updateTop(tmp_al_rank_info_);
    }
    fclose(fp);
    return 0;
}

TINT32 ServerData::loadRecommendAl(const TCHAR* file_name)
{
    FILE *fp = fopen(file_name, "rt");
    if(fp == NULL)
    {
        return 0;
    }

    TCHAR line[1024];
    while(fgets(line, 1024, fp))
    {
        tmp_recommend_al_info_.reset();
        if(tmp_recommend_al_info_.load_data(line) != 0)
        {
            continue;
        }

        recommend_al_infos_.push_back(tmp_recommend_al_info_);
    }

    fclose(fp);
    return 0;
}

TINT32 ServerData::getPlayerRank(TINT32 uid, PlayerRankInfo* player_rank_info)
{
    std::map<TINT32, TINT32>::iterator it = player_index_.find(uid);
    if(it != player_index_.end())
    {
        *player_rank_info = player_infos_[it->second];
        return 0;
    }
    player_rank_info->reset();
    return -1;
}

TINT32 ServerData::getPlayerRank(TINT32 rank_type, TINT32 uid, PlayerRankInfo* player_rank_info)
{
    std::map<TINT32, TINT32>::iterator it = player_index_.find(uid);
    if(it != player_index_.end())
    {
        *player_rank_info = player_infos_[it->second];
        player_rank_info->rank_type = rank_type;
        switch(rank_type)
        {
        case EN_RANK_TYPE_PLAYER_FORCE_KILL:
            player_rank_info->rank = player_rank_info->rank1;
            player_rank_info->value = player_rank_info->value1;
            break;
        case EN_RANK_TYPE_PLAYER_TROOPS_KILL:
            player_rank_info->rank = player_rank_info->rank2;
            player_rank_info->value = player_rank_info->value2;
            break;
        case EN_RANK_TYPE_PLAYER_FORCE:
            player_rank_info->rank = player_rank_info->rank3;
            player_rank_info->value = player_rank_info->value3;
            break;
        case EN_RANK_TYPE_PLAYER_TROOPS_KDR:
            player_rank_info->rank = player_rank_info->rank4;
            player_rank_info->value = player_rank_info->value4;
            break;
        case EN_RANK_TYPE_PLAYER_BATTLE_WON:
            player_rank_info->rank = player_rank_info->rank5;
            player_rank_info->value = player_rank_info->value5;
            break;
        case EN_RANK_TYPE_PLAYER_BWLR:
            player_rank_info->rank = player_rank_info->rank6;
            player_rank_info->value = player_rank_info->value6;
            break;
        case EN_RANK_TYPE_PLAYER_DRAGON_CAPTURED:
            player_rank_info->rank = player_rank_info->rank7;
            player_rank_info->value = player_rank_info->value7;
            break;
        case EN_RANK_TYPE_PLAYER_DRAGON_EXECUTED:
            player_rank_info->rank = player_rank_info->rank8;
            player_rank_info->value = player_rank_info->value8;
            break;
        case EN_RANK_TYPE_PLAYER_EVIL_TROOP_KILL:
            player_rank_info->rank = player_rank_info->rank9;
            player_rank_info->value = player_rank_info->value9;
            break;
        case EN_RANK_TYPE_PLAYER_EVIL_FORCE_KILL:
            player_rank_info->rank = player_rank_info->rank10;
            player_rank_info->value = player_rank_info->value10;
            break;
        case EN_RANK_TYPE_PLAYER_GAIN_RESOURCE:
            player_rank_info->rank = player_rank_info->rank11;
            player_rank_info->value = player_rank_info->value11;
            break;
        case EN_RANK_TYPE_PLAYER_TRANSPORT_RESOURCE_0:
            player_rank_info->rank = player_rank_info->rank12;
            player_rank_info->value = player_rank_info->value12;
            break;
        case EN_RANK_TYPE_PLAYER_TRANSPORT_RESOURCE_1:
            player_rank_info->rank = player_rank_info->rank13;
            player_rank_info->value = player_rank_info->value13;
            break;
        case EN_RANK_TYPE_PLAYER_TRANSPORT_RESOURCE_2:
            player_rank_info->rank = player_rank_info->rank14;
            player_rank_info->value = player_rank_info->value14;
            break;
        case EN_RANK_TYPE_PLAYER_TRANSPORT_RESOURCE_3:
            player_rank_info->rank = player_rank_info->rank15;
            player_rank_info->value = player_rank_info->value15;
            break;
        case EN_RANK_TYPE_PLAYER_TRANSPORT_RESOURCE_4:
            player_rank_info->rank = player_rank_info->rank16;
            player_rank_info->value = player_rank_info->value16;
            break;
        case EN_RANK_TYPE_PLAYER_KILL_MONSTER:
            player_rank_info->rank = player_rank_info->rank17;
            player_rank_info->value = player_rank_info->value17;
            break;
        default:
            break;
        }
        return 0;
    }
    player_rank_info->reset();
    return -1;
}

TINT32 ServerData::getTopPlayerRank(TINT32 rank_type, PlayerRankInfo* player_rank_infos, TINT32& player_num)
{
    player_num = 0;
    for(TUINT32 index = 0; index < DEFAULT_TOP_NUM; ++index)
    {
        TINT32 uid = rank_top_id_.at(rank_type)[index];
        if(uid == 0)
        {
            break;
        }

        if(getPlayerRank(rank_type, uid, &player_rank_infos[player_num]) != 0)
        {
            continue;
        }
        
        player_num++;
    }
    return player_num;
}

TINT32 ServerData::searchPlayer(std::string keyword, TINT32 offset, TINT32& player_num, TINT32& total_num, PlayerRankInfo* player_rank_infos)
{
    player_num = 0;
    total_num = 0;
    for(TINT32 index = 0; index < player_infos_.size(); ++index)
    {
        if(player_infos_[index].aid == 0)
        {
            continue;
        }

        if(matchWord(player_infos_[index].uname, keyword.c_str()) == FALSE)
        {
            continue;
        }

        total_num++;

        if(player_num < DEFAULT_NUM_ONE_PAGE && total_num > offset)
        {
            player_rank_infos[player_num++] = player_infos_[index];
        }
    }

    return player_num;
}


TINT32 ServerData::NewsearchPlayer(TINT32 rank_type, std::string keyword, TINT32 offset, TINT32& player_num, TINT32& total_num, PlayerRankInfo* player_rank_infos)
{
    player_num = 0;
    total_num = 0;

    for(TINT32 index = 0; index < player_infos_.size(); ++index)
    {
        if (player_num >= 100)
        {
            break; // 避免过多结果
        }
        if(matchWord(player_infos_[index].uname, keyword.c_str()) == FALSE)
        {
            continue;
        }
        player_rank_infos[player_num] = player_infos_[index];
        if(getPlayerRank(rank_type, player_rank_infos[player_num].uid, &player_rank_infos[player_num]) != 0)
        {
            continue;
        }

        total_num++;
        player_num++;
    }

    return player_num;
}


TINT32 ServerData::getAlRank(TINT32 aid, AllianceRankInfo* al_rank_info)
{
    std::map<TINT32, TINT32>::iterator it = alliance_index_.find(aid);
    if(it != alliance_index_.end())
    {
        *al_rank_info = alliance_infos_[it->second];
        return 0;
    }
    al_rank_info->reset();
    return -1;
}

TINT32 ServerData::getAlRank(TINT32 rank_type, TINT32 aid, AllianceRankInfo* al_rank_info)
{
    std::map<TINT32, TINT32>::iterator it = alliance_index_.find(aid);
    if(it != alliance_index_.end())
    {
        *al_rank_info = alliance_infos_[it->second];
        al_rank_info->rank_type = rank_type;
        switch(rank_type)
        {
        case EN_RANK_TYPE_ALLIANCE_FORCE_KILL:
            al_rank_info->rank = al_rank_info->rank1;
            al_rank_info->value = al_rank_info->value1;
            break;
        case EN_RANK_TYPE_ALLIANCE_TROOPS_KILL:
            al_rank_info->rank = al_rank_info->rank2;
            al_rank_info->value = al_rank_info->value2;
            break;
        case EN_RANK_TYPE_ALLIANCE_FORCE:
            al_rank_info->rank = al_rank_info->rank3;
            al_rank_info->value = al_rank_info->value3;
            break;
        case EN_RANK_TYPE_ALLIANCE_TROOPS_KDR:
            al_rank_info->rank = al_rank_info->rank4;
            al_rank_info->value = al_rank_info->value4;
            break;
        case EN_RANK_TYPE_ALLIANCE_BATTLE_WON:
            al_rank_info->rank = al_rank_info->rank5;
            al_rank_info->value = al_rank_info->value5;
            break;
        case EN_RANK_TYPE_ALLIANCE_BWLR:
            al_rank_info->rank = al_rank_info->rank6;
            al_rank_info->value = al_rank_info->value6;
            break;
        case EN_RANK_TYPE_ALLIANCE_HELP:
            al_rank_info->rank = al_rank_info->rank7;
            al_rank_info->value = al_rank_info->value7;
            break;
        case EN_RANK_TYPE_ALLIANCE_FUND:
            al_rank_info->rank = al_rank_info->rank8;
            al_rank_info->value = al_rank_info->value8;
            break;
        case EN_RANK_TYPE_ALLIANCE_GIFT_POINT:
            al_rank_info->rank = al_rank_info->rank9;
            al_rank_info->value = al_rank_info->value9;
            break;
        case EN_RANK_TYPE_ALLIANCE_DRAGON_CAPTURED:
            al_rank_info->rank = al_rank_info->rank10;
            al_rank_info->value = al_rank_info->value10;
            break;
        case EN_RANK_TYPE_ALLIANCE_DRAGON_EXECUTED:
            al_rank_info->rank = al_rank_info->rank11;
            al_rank_info->value = al_rank_info->value11;
            break;
        case EN_RANK_TYPE_ALLIANCE_EVIL_TROOP_KILL:
            al_rank_info->rank = al_rank_info->rank12;
            al_rank_info->value = al_rank_info->value12;
            break;
        case EN_RANK_TYPE_ALLIANCE_EVIL_FORCE_KILL:
            al_rank_info->rank = al_rank_info->rank13;
            al_rank_info->value = al_rank_info->value13;
            break;
        case EN_RANK_TYPE_ALLIANCE_THRONE_OCCUPY:
            al_rank_info->rank = al_rank_info->rank14;
            al_rank_info->value = al_rank_info->value14;
            break;
        default:
            break;
        }
        return 0;
    }
    al_rank_info->reset();
    return -1;
}

TINT32 ServerData::delAlRank(TINT32 aid)
{
    std::map<TINT32, AllianceRankInfo>::iterator it = update_alliance_infos_.find(aid);
    if(it != update_alliance_infos_.end())
    {
        update_alliance_infos_[aid].udwUpdateTime = 0;
    }
    else
    {
        AllianceRankInfo stAllianceRankInfo;
        stAllianceRankInfo.reset();
        stAllianceRankInfo.aid = aid;
        stAllianceRankInfo.udwUpdateTime = 0;
        update_alliance_infos_[aid] = stAllianceRankInfo;
    }

    
    std::map<TINT32, TINT32>::iterator it_index = alliance_index_.find(aid);
    if(it_index != alliance_index_.end())
    {
        alliance_infos_[it_index->second].udwUpdateTime = 0;
    }
    
    return 0;
}



TINT32 ServerData::addAlRank(TINT32 aid, TINT32 join_policy, const std::string& alname, const std::string& alnick,
    TINT64 value3, TINT64 value9, TINT32 new_player_flag)
{

    TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("ServerData::addAlRank begin alliance_infos_.size()=%ld",\
                                             alliance_infos_.size()));



    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();

    std::map<TINT32, TINT32>::iterator it = alliance_index_.find(aid);
    if(it != alliance_index_.end())
    {
        return -1;
    }

    tmp_al_rank_info_.reset();

    tmp_al_rank_info_.aid = aid;
    tmp_al_rank_info_.join_policy = join_policy;

    strncpy(tmp_al_rank_info_.alname, alname.c_str(), MAX_NAME_LEN);
    tmp_al_rank_info_.alname[MAX_NAME_LEN - 1] = '\0';
    strncpy(tmp_al_rank_info_.alnick, alnick.c_str(), 5);
    tmp_al_rank_info_.alnick[4] = '\0';

    tmp_al_rank_info_.value3 = value3;
    tmp_al_rank_info_.value9 = value9;

    tmp_al_rank_info_.rank1 = alliance_infos_.size() + 1;
    tmp_al_rank_info_.rank2 = tmp_al_rank_info_.rank1;
    tmp_al_rank_info_.rank3 = tmp_al_rank_info_.rank1;
    tmp_al_rank_info_.rank4 = tmp_al_rank_info_.rank1;
    tmp_al_rank_info_.rank5 = tmp_al_rank_info_.rank1;
    tmp_al_rank_info_.rank6 = tmp_al_rank_info_.rank1;
    tmp_al_rank_info_.rank7 = tmp_al_rank_info_.rank1;
    tmp_al_rank_info_.rank8 = tmp_al_rank_info_.rank1;
    tmp_al_rank_info_.rank9 = tmp_al_rank_info_.rank1;
    tmp_al_rank_info_.rank10 = tmp_al_rank_info_.rank1;
    tmp_al_rank_info_.rank11 = tmp_al_rank_info_.rank1;
    tmp_al_rank_info_.rank12 = tmp_al_rank_info_.rank1;
    tmp_al_rank_info_.rank13 = tmp_al_rank_info_.rank1;
    tmp_al_rank_info_.rank14 = tmp_al_rank_info_.rank1;

    tmp_al_rank_info_.udwUpdateTime = udwCurTime;


    update_alliance_infos_[aid] = tmp_al_rank_info_;

    alliance_infos_.push_back(tmp_al_rank_info_);
    alliance_index_.insert(std::make_pair(tmp_al_rank_info_.aid, alliance_infos_.size() - 1));

    TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("ServerData::addAlRank end alliance_infos_.size()=%ld",\
                                             alliance_infos_.size()));


    if(new_player_flag == 1)
    {
        TBOOL find_in_recommend = FALSE;
        for(TINT32 index = 0; index < recommend_al_infos_.size(); ++index)
        {
            if(recommend_al_infos_[index].aid == aid)
            {
                find_in_recommend = TRUE;
                break;
            }
        }
        if(!find_in_recommend)
        {
            tmp_recommend_al_info_.reset();
            tmp_recommend_al_info_.aid = aid;
            tmp_recommend_al_info_.is_new_player_al = new_player_flag;
            tmp_recommend_al_info_.is_npc = 0;
            tmp_recommend_al_info_.language = 0;
            tmp_recommend_al_info_.member_num = 1;
            tmp_recommend_al_info_.policy = join_policy;
            recommend_al_infos_.push_back(tmp_recommend_al_info_);
        }
    }

    updateTop(tmp_al_rank_info_);

    return 0;
}

std::map<TINT32, AllianceRankInfo>&  ServerData::getUpdateAlInfo()
{
    return update_alliance_infos_;
}


TINT32 ServerData::getTopAlRank(TINT32 rank_type, AllianceRankInfo* al_rank_infos, TINT32& al_num)
{
    al_num = 0;
    for(TUINT32 index = 0; index < DEFAULT_TOP_NUM; ++index)
    {
        TINT32 aid = rank_top_id_.at(rank_type)[index];
        if(aid == 0)
        {
            break;
        }

        if(getAlRank(rank_type, aid, &al_rank_infos[al_num]) != 0)
        {
            continue;
        }

        al_num++;
    }
    return al_num;
}

TINT32 ServerData::getAlList(TINT32 join_policy, TINT32 offset, TINT32& al_num, TINT32& total_num, AllianceRankInfo* al_rank_infos)
{
    //Suppose that the alliance_infos_ is in ordered by rank3 force 
    al_num = 0;
    total_num = 0;

    for(TINT32 index = 0; index < alliance_infos_.size(); ++index)
    {
        if(alliance_infos_[index].aid == 0)
        {
            continue;
        }

        if(alliance_infos_[index].udwUpdateTime == 0)
        {
            continue;
        }

        if(join_policy != EN_ALLIANCE_JOIN_BOTH
        && join_policy != alliance_infos_[index].join_policy)
        {
            continue;
        }

        total_num++;

        if(al_num < DEFAULT_NUM_ONE_PAGE && total_num > offset)
        {
            al_rank_infos[al_num++] = alliance_infos_[index];
        }
    }

    return al_num;
}

TINT32 ServerData::searchAl(TINT32 join_policy, std::string keyword, TINT32 offset, TINT32& al_num, TINT32& total_num, AllianceRankInfo* al_rank_infos)
{
    al_num = 0;
    total_num = 0;

    TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("ServerData::searchAl alliance_infos_.size()=%ld", \
                                             alliance_infos_.size()));

    for(TINT32 index = 0; index < alliance_infos_.size(); ++index)
    {
        
        TSE_LOG_DEBUG(CGlobalServ::m_poServLog, ("ServerData::searchAl aid=%ld, udwUpdateTime=%ld, alname=%s, alnick=%s, keyword=%s",\
                                                alliance_infos_[index].aid, \
                                                alliance_infos_[index].udwUpdateTime, \
                                                alliance_infos_[index].alname,\ 
                                                alliance_infos_[index].alnick,\
                                                keyword.c_str()));


    
        if(alliance_infos_[index].aid == 0)
        {
            continue;
        }

        if(alliance_infos_[index].udwUpdateTime == 0)
        {
            continue;
        }

        if(join_policy != EN_ALLIANCE_JOIN_BOTH
        && join_policy != alliance_infos_[index].join_policy)
        {
            continue;
        }

        if(matchWord(alliance_infos_[index].alname, keyword.c_str()) == FALSE
        && matchWord(alliance_infos_[index].alnick, keyword.c_str()) == FALSE)
        {
            continue;
        }

        total_num++;

        if(al_num < DEFAULT_NUM_ONE_PAGE && total_num > offset)
        {
            al_rank_infos[al_num++] = alliance_infos_[index];
        }
    }

    return al_num;
}

TINT32 ServerData::changeAlPolicy(TINT32 aid, TINT32 policy)
{
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    std::map<TINT32, TINT32>::iterator it = alliance_index_.find(aid);
    if(it != alliance_index_.end())
    {
        alliance_infos_[it->second].join_policy = policy;

        update_alliance_infos_[aid] = alliance_infos_[it->second];
        update_alliance_infos_[aid].udwUpdateTime = udwCurTime;
     
        return 0;
    }
    return -1;
}

TINT32 ServerData::changeAlName(TINT32 aid, const std::string& alname)
{
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    std::map<TINT32, TINT32>::iterator it = alliance_index_.find(aid);
    if(it != alliance_index_.end())
    {
        strncpy(alliance_infos_[it->second].alname, alname.c_str(), MAX_NAME_LEN);
        alliance_infos_[it->second].alname[MAX_NAME_LEN - 1] = '\0';

        update_alliance_infos_[aid] = alliance_infos_[it->second];
        update_alliance_infos_[aid].udwUpdateTime = udwCurTime;

        return 0;
    }
    return -1;
}

TINT32 ServerData::changeAlNick(TINT32 aid, const std::string& alnick)
{
    TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
    std::map<TINT32, TINT32>::iterator it = alliance_index_.find(aid);
    if(it != alliance_index_.end())
    {
        strncpy(alliance_infos_[it->second].alnick, alnick.c_str(), 5);
        alliance_infos_[it->second].alnick[4] = '\0';
        
        update_alliance_infos_[aid] = alliance_infos_[it->second];
        update_alliance_infos_[aid].udwUpdateTime = udwCurTime;

        return 0;
    }
    return -1;
}

TINT32 ServerData::changeAlLang(TINT32 aid, TINT32 lang)
{
    for(std::vector<RecommendAllianceInfo>::iterator it = recommend_al_infos_.begin(); it != recommend_al_infos_.end(); ++it)
    {
        if(it->aid == aid)
        {
            it->language = lang;
            return 0;
        }
    }
    return -1;
}

TINT32 ServerData::getRecommendAl(TINT32 new_player_flag, TINT32& al_num, RecommendAllianceInfo* recommend_al_infos)
{
    //Suppose that the alliance_infos_ is in ordered by rank3 force 
    al_num = 0;

    for(TINT32 index = 0; index < recommend_al_infos_.size(); ++index)
    {
        if(recommend_al_infos_[index].aid == 0)
        {
            break;
        }

        if(recommend_al_infos_[index].is_new_player_al != new_player_flag)
        {
            continue;
        }

        recommend_al_infos[al_num++] = recommend_al_infos_[index];

        if(al_num >= DEFAULT_TOP_NUM)
        {
            break;
        }
    }

    return al_num;
}

TBOOL ServerData::matchWord(const TCHAR* str, const TCHAR* substr)
{
    TCHAR lower_str[MAX_NAME_LEN];
    TCHAR lower_substr[MAX_NAME_LEN];

    TUINT32 udwStrLen = strlen(str);
    if (udwStrLen >= MAX_NAME_LEN)
    {
        udwStrLen = MAX_NAME_LEN - 1;
    }
    for(TINT32 index = 0; index < strlen(str); ++index)
    {
        lower_str[index] = tolower(str[index]);
    }
    lower_str[udwStrLen] = '\0';

    udwStrLen = strlen(substr);
    if (udwStrLen >= MAX_NAME_LEN)
    {
        udwStrLen = MAX_NAME_LEN - 1;
    }
    for(TINT32 index = 0; index < strlen(substr); ++index)
    {
        lower_substr[index] = tolower(substr[index]);
    }
    lower_substr[udwStrLen] = '\0';

    if(strstr(lower_str, lower_substr) == NULL)
    {
        return FALSE;
    }

    return TRUE;
}

TINT32 ServerData::updateTop(const PlayerRankInfo& player_rank_info)
{
    if(player_rank_info.rank1 <= DEFAULT_TOP_NUM && player_rank_info.rank1 > 0 && player_rank_info.value1 > 0)
    {
        rank_top_id_.at(kRankTypes[1])[player_rank_info.rank1 - 1] = player_rank_info.uid;
    }

    if (player_rank_info.rank2 <= DEFAULT_TOP_NUM && player_rank_info.rank2 > 0 && player_rank_info.value2 > 0)
    {
        rank_top_id_.at(kRankTypes[2])[player_rank_info.rank2 - 1] = player_rank_info.uid;
    }

    if (player_rank_info.rank3 <= DEFAULT_TOP_NUM && player_rank_info.rank3 > 0 && player_rank_info.value3 > 0)
    {
        rank_top_id_.at(kRankTypes[3])[player_rank_info.rank3 - 1] = player_rank_info.uid;
    }

    if (player_rank_info.rank4 <= DEFAULT_TOP_NUM && player_rank_info.rank4 > 0 && player_rank_info.value4 > 0)
    {
        rank_top_id_.at(kRankTypes[4])[player_rank_info.rank4 - 1] = player_rank_info.uid;
    }

    if (player_rank_info.rank5 <= DEFAULT_TOP_NUM && player_rank_info.rank5 > 0 && player_rank_info.value5 > 0)
    {
        rank_top_id_.at(kRankTypes[5])[player_rank_info.rank5 - 1] = player_rank_info.uid;
    }

    if (player_rank_info.rank6 <= DEFAULT_TOP_NUM && player_rank_info.rank6 > 0 && player_rank_info.value6 > 0)
    {
        rank_top_id_.at(kRankTypes[6])[player_rank_info.rank6 - 1] = player_rank_info.uid;
    }

    if (player_rank_info.rank7 <= DEFAULT_TOP_NUM && player_rank_info.rank7 > 0 && player_rank_info.value7 > 0)
    {
        rank_top_id_.at(kRankTypes[7])[player_rank_info.rank7 - 1] = player_rank_info.uid;
    }

    if (player_rank_info.rank8 <= DEFAULT_TOP_NUM && player_rank_info.rank8 > 0 && player_rank_info.value8 > 0)
    {
        rank_top_id_.at(kRankTypes[8])[player_rank_info.rank8 - 1] = player_rank_info.uid;
    }

    if (player_rank_info.rank9 <= DEFAULT_TOP_NUM && player_rank_info.rank9 > 0 && player_rank_info.value9 > 0)
    {
        rank_top_id_.at(kRankTypes[9])[player_rank_info.rank9 - 1] = player_rank_info.uid;
    }

    if (player_rank_info.rank10 <= DEFAULT_TOP_NUM && player_rank_info.rank10 > 0 && player_rank_info.value10 > 0)
    {
        rank_top_id_.at(kRankTypes[10])[player_rank_info.rank10 - 1] = player_rank_info.uid;
    }

    if (player_rank_info.rank11 <= DEFAULT_TOP_NUM && player_rank_info.rank11 > 0 && player_rank_info.value11 > 0)
    {
        rank_top_id_.at(kRankTypes[11])[player_rank_info.rank11 - 1] = player_rank_info.uid;
    }

    if (player_rank_info.rank12 <= DEFAULT_TOP_NUM && player_rank_info.rank12 > 0 && player_rank_info.value12 > 0)
    {
        rank_top_id_.at(kRankTypes[12])[player_rank_info.rank12 - 1] = player_rank_info.uid;
    }

    if (player_rank_info.rank13 <= DEFAULT_TOP_NUM && player_rank_info.rank13 > 0 && player_rank_info.value13 > 0)
    {
        rank_top_id_.at(kRankTypes[13])[player_rank_info.rank13 - 1] = player_rank_info.uid;
    }

    if (player_rank_info.rank14 <= DEFAULT_TOP_NUM && player_rank_info.rank14 > 0 && player_rank_info.value14 > 0)
    {
        rank_top_id_.at(kRankTypes[14])[player_rank_info.rank14 - 1] = player_rank_info.uid;
    }

    if (player_rank_info.rank15 <= DEFAULT_TOP_NUM && player_rank_info.rank15 > 0 && player_rank_info.value15 > 0)
    {
        rank_top_id_.at(kRankTypes[15])[player_rank_info.rank15 - 1] = player_rank_info.uid;
    }

    if (player_rank_info.rank16 <= DEFAULT_TOP_NUM && player_rank_info.rank16 > 0 && player_rank_info.value16 > 0)
    {
        rank_top_id_.at(kRankTypes[16])[player_rank_info.rank16 - 1] = player_rank_info.uid;
    }

    if (player_rank_info.rank17 <= DEFAULT_TOP_NUM && player_rank_info.rank17 > 0 && player_rank_info.value17 > 0)
    {
        rank_top_id_.at(kRankTypes[17])[player_rank_info.rank17 - 1] = player_rank_info.uid;
    }

    return 0;
}

TINT32 ServerData::updateTop(const AllianceRankInfo& al_rank_info)
{
    const int offset = 18;

    if(al_rank_info.rank1 <= DEFAULT_TOP_NUM && al_rank_info.rank1 > 0 && al_rank_info.value1 > 0)
    {
        rank_top_id_.at(kRankTypes[1 + offset])[al_rank_info.rank1 - 1] = al_rank_info.aid;
    }

    if (al_rank_info.rank2 <= DEFAULT_TOP_NUM && al_rank_info.rank2 > 0 && al_rank_info.value2 > 0)
    {
        rank_top_id_.at(kRankTypes[2 + offset])[al_rank_info.rank2 - 1] = al_rank_info.aid;
    }

    if (al_rank_info.rank3 <= DEFAULT_TOP_NUM && al_rank_info.rank3 > 0 && al_rank_info.value3 > 0)
    {
        rank_top_id_.at(kRankTypes[3 + offset])[al_rank_info.rank3 - 1] = al_rank_info.aid;
    }

    if (al_rank_info.rank4 <= DEFAULT_TOP_NUM && al_rank_info.rank4 > 0 && al_rank_info.value4 > 0)
    {
        rank_top_id_.at(kRankTypes[4 + offset])[al_rank_info.rank4 - 1] = al_rank_info.aid;
    }

    if (al_rank_info.rank5 <= DEFAULT_TOP_NUM && al_rank_info.rank5 > 0 && al_rank_info.value5 > 0)
    {
        rank_top_id_.at(kRankTypes[5 + offset])[al_rank_info.rank5 - 1] = al_rank_info.aid;
    }

    if (al_rank_info.rank6 <= DEFAULT_TOP_NUM && al_rank_info.rank6 > 0 && al_rank_info.value6 > 0)
    {
        rank_top_id_.at(kRankTypes[6 + offset])[al_rank_info.rank6 - 1] = al_rank_info.aid;
    }

    if (al_rank_info.rank7 <= DEFAULT_TOP_NUM && al_rank_info.rank7 > 0 && al_rank_info.value7 > 0)
    {
        rank_top_id_.at(kRankTypes[7 + offset])[al_rank_info.rank7 - 1] = al_rank_info.aid;
    }

    if (al_rank_info.rank8 <= DEFAULT_TOP_NUM && al_rank_info.rank8 > 0 && al_rank_info.value8 > 0)
    {
        rank_top_id_.at(kRankTypes[8 + offset])[al_rank_info.rank8 - 1] = al_rank_info.aid;
    }

    if (al_rank_info.rank9 <= DEFAULT_TOP_NUM && al_rank_info.rank9 > 0 && al_rank_info.value9 > 0)
    {
        rank_top_id_.at(kRankTypes[9 + offset])[al_rank_info.rank9 - 1] = al_rank_info.aid;
    }

    if (al_rank_info.rank10 <= DEFAULT_TOP_NUM && al_rank_info.rank10 > 0 && al_rank_info.value10 > 0)
    {
        rank_top_id_.at(kRankTypes[10 + offset])[al_rank_info.rank10 - 1] = al_rank_info.aid;
    }

    if (al_rank_info.rank11 <= DEFAULT_TOP_NUM && al_rank_info.rank11 > 0 && al_rank_info.value11 > 0)
    {
        rank_top_id_.at(kRankTypes[11 + offset])[al_rank_info.rank11 - 1] = al_rank_info.aid;
    }

    if (al_rank_info.rank12 <= DEFAULT_TOP_NUM && al_rank_info.rank12 > 0 && al_rank_info.value12 > 0)
    {
        rank_top_id_.at(kRankTypes[12 + offset])[al_rank_info.rank12 - 1] = al_rank_info.aid;
    }

    if (al_rank_info.rank13 <= DEFAULT_TOP_NUM && al_rank_info.rank13 > 0 && al_rank_info.value13 > 0)
    {
        rank_top_id_.at(kRankTypes[13 + offset])[al_rank_info.rank13 - 1] = al_rank_info.aid;
    }
    if (al_rank_info.rank14 <= DEFAULT_TOP_NUM && al_rank_info.rank14 > 0 && al_rank_info.value14 > 0)
    {
        rank_top_id_.at(kRankTypes[14 + offset])[al_rank_info.rank14 - 1] = al_rank_info.aid;
    }
    return 0;
}

TINT32 ServerData::getTopSingleAlRank(TINT32 rank_type, AllianceRankInfo* al_rank_info)
{
    al_rank_info->reset();
    TINT32 aid = rank_top_id_.at(rank_type)[0];
    if(aid == 0)
    {
        return -1;
    }

    if(getAlRank(rank_type, aid, al_rank_info) == 0)
    {
        return 0;
    }

    return -2;
}

TINT32 ServerData::getTopSinglePlayerRank(TINT32 rank_type, PlayerRankInfo* player_rank_info)
{
    player_rank_info->reset();
    std::vector<TINT32> tmp = rank_top_id_.at(rank_type);
    TINT32 uid = rank_top_id_.at(rank_type)[0];
    if(uid == 0)
    {
        return -1;
    }

    if(getPlayerRank(rank_type, uid, player_rank_info) == 0)
    {
        return 0;
    }

    return -2;
}
