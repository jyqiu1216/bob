#ifndef _ALLIANCE_RANK_INFO_H_
#define _ALLIANCE_RANK_INFO_H_

#include "base/common/wtse_std_header.h"
#define MAX_NAME_LEN 32

struct AllianceRankInfo
{
    TINT32 aid;
    TINT32 join_policy;

    TINT32 rank_type;

    TINT32 rank1;
    TINT32 rank2;
    TINT32 rank3;
    TINT32 rank4;
    TINT32 rank5;
    TINT32 rank6;
    TINT32 rank7;
    TINT32 rank8;
    TINT32 rank9;
    TINT32 rank10;
    TINT32 rank11;
    TINT32 rank12;
    TINT32 rank13;
    TINT32 rank14;      //EN_RANK_TYPE_ALLIANCE_THRONE_OCCUPY

    TINT32 rank;

    TINT64 value1;
    TINT64 value2;
    TINT64 value3;
    TINT64 value4;
    TINT64 value5;
    TINT64 value6;
    TINT64 value7;
    TINT64 value8;
    TINT64 value9;
    TINT64 value10;
    TINT64 value11;
    TINT64 value12;
    TINT64 value13;
    TINT64 value14;     //EN_RANK_TYPE_ALLIANCE_THRONE_OCCUPY

    TINT64 value;

    TCHAR alnick[5];
    TCHAR alname[MAX_NAME_LEN];

    TUINT32 udwUpdateTime;

    TVOID reset()
    {
        memset((TVOID*)this, 0, sizeof(AllianceRankInfo));
    };

    TINT32 load_data(TCHAR* raw_data)
    {
        if(strlen(raw_data) < 10)
        {
            return -1;
        }

        TCHAR* cur_pos = NULL;

        // last login time
        while(TRUE)
        {
            cur_pos = strtok(raw_data, " \t\r\n");
            if(!cur_pos)
            {
                break;
            }
            aid = atoi(cur_pos);

            cur_pos = strtok(NULL, " \t\r\n");
            if(!cur_pos)
            {
                break;
            }
            join_policy = atoi(cur_pos);

            cur_pos = strtok(NULL, " \t\r\n");
            if(!cur_pos)
            {
                break;
            }
            rank1 = atoi(cur_pos);

            cur_pos = strtok(NULL, " \t\r\n");
            if(!cur_pos)
            {
                break;
            }
            rank2 = atoi(cur_pos);

            cur_pos = strtok(NULL, " \t\r\n");
            if(!cur_pos)
            {
                break;
            }
            rank3 = atoi(cur_pos);

            cur_pos = strtok(NULL, " \t\r\n");
            if(!cur_pos)
            {
                break;
            }
            rank4 = atoi(cur_pos);

            cur_pos = strtok(NULL, " \t\r\n");
            if(!cur_pos)
            {
                break;
            }
            rank5 = atoi(cur_pos);

            cur_pos = strtok(NULL, " \t\r\n");
            if(!cur_pos)
            {
                break;
            }
            rank6 = atoi(cur_pos);

            cur_pos = strtok(NULL, " \t\r\n");
            if(!cur_pos)
            {
                break;
            }
            rank7 = atoi(cur_pos);

            cur_pos = strtok(NULL, " \t\r\n");
            if(!cur_pos)
            {
                break;
            }
            rank8 = atoi(cur_pos);

            cur_pos = strtok(NULL, " \t\r\n");
            if(!cur_pos)
            {
                break;
            }
            rank9 = atoi(cur_pos);

            cur_pos = strtok(NULL, " \t\r\n");
            if (!cur_pos)
            {
                break;
            }
            rank10 = atoi(cur_pos);

            cur_pos = strtok(NULL, " \t\r\n");
            if (!cur_pos)
            {
                break;
            }
            rank11 = atoi(cur_pos);

            cur_pos = strtok(NULL, " \t\r\n");
            if(!cur_pos)
            {
                break;
            }
            value1 = strtoull(cur_pos, NULL, 10);

            cur_pos = strtok(NULL, " \t\r\n");
            if(!cur_pos)
            {
                break;
            }
            value2 = strtoull(cur_pos, NULL, 10);

            cur_pos = strtok(NULL, " \t\r\n");
            if(!cur_pos)
            {
                break;
            }
            value3 = strtoull(cur_pos, NULL, 10);

            cur_pos = strtok(NULL, " \t\r\n");
            if(!cur_pos)
            {
                break;
            }
            value4 = strtoull(cur_pos, NULL, 10);

            cur_pos = strtok(NULL, " \t\r\n");
            if(!cur_pos)
            {
                break;
            }
            value5 = strtoull(cur_pos, NULL, 10);

            cur_pos = strtok(NULL, " \t\r\n");
            if(!cur_pos)
            {
                break;
            }
            value6 = strtoull(cur_pos, NULL, 10);

            cur_pos = strtok(NULL, " \t\r\n");
            if(!cur_pos)
            {
                break;
            }
            value7 = strtoull(cur_pos, NULL, 10);

            cur_pos = strtok(NULL, " \t\r\n");
            if(!cur_pos)
            {
                break;
            }
            value8 = strtoull(cur_pos, NULL, 10);

            cur_pos = strtok(NULL, " \t\r\n");
            if(!cur_pos)
            {
                break;
            }
            value9 = strtoull(cur_pos, NULL, 10);

            cur_pos = strtok(NULL, " \t\r\n");
            if (!cur_pos)
            {
                break;
            }
            value10 = strtoull(cur_pos, NULL, 10);

            cur_pos = strtok(NULL, " \t\r\n");
            if (!cur_pos)
            {
                break;
            }
            value11 = strtoull(cur_pos, NULL, 10);

            cur_pos = strtok(NULL, " \t\r\n");
            if(!cur_pos)
            {
                break;
            }
            strncpy(alnick, cur_pos, 5);
            alnick[4] = '\0';

            cur_pos = strtok(NULL, " \t\r\n");
            if(!cur_pos)
            {
                break;
            }
            strncpy(alname, cur_pos, MAX_NAME_LEN);
            alname[MAX_NAME_LEN - 1] = '\0';

            cur_pos = strtok(NULL, " \t\r\n");
            if(!cur_pos)
            {
                break;
            }
            udwUpdateTime = strtoul(cur_pos, NULL, 10);

            cur_pos = strtok(NULL, " \t\r\n");
            if (!cur_pos)
            {
                return -2;
            }
            rank12 = strtoull(cur_pos, NULL, 10);

            cur_pos = strtok(NULL, " \t\r\n");
            if (!cur_pos)
            {
                return -2;
            }
            value12 = strtoull(cur_pos, NULL, 10);

            cur_pos = strtok(NULL, " \t\r\n");
            if (!cur_pos)
            {
                return -2;
            }
            rank13 = strtoull(cur_pos, NULL, 10);

            cur_pos = strtok(NULL, " \t\r\n");
            if (!cur_pos)
            {
                return -2;
            }
            value13 = strtoull(cur_pos, NULL, 10);

            cur_pos = strtok(NULL, " \t\r\n");
            if (!cur_pos)
            {
                return -2;
            }
            rank14 = strtoull(cur_pos, NULL, 10);

            cur_pos = strtok(NULL, " \t\r\n");
            if (!cur_pos)
            {
                return -2;
            }
            value14 = strtoull(cur_pos, NULL, 10);
            break;
        }

        if(aid == 0)
        {
            return -2;
        }

        return 0;
    };
};

struct RecommendAllianceInfo
{
    TBOOL is_npc;
    TBOOL is_new_player_al;

    TINT32 aid;
    TINT32 member_num;
    TINT32 policy;
    TINT32 language;

    TVOID reset()
    {
        memset((TVOID*)this, 0, sizeof(RecommendAllianceInfo));
    };

    TINT32 load_data(TCHAR* raw_data)
    {
        if(strlen(raw_data) < 10)
        {
            return -1;
        }

        TCHAR* cur_pos = NULL;

        // last login time
        while(TRUE)
        {
            cur_pos = strtok(raw_data, " \t\r\n");
            if(!cur_pos)
            {
                break;
            }
            aid = atoi(cur_pos);

            cur_pos = strtok(NULL, " \t\r\n");
            if(!cur_pos)
            {
                break;
            }
            member_num = atoi(cur_pos);

            cur_pos = strtok(NULL, " \t\r\n");
            if(!cur_pos)
            {
                break;
            }
            policy = atoi(cur_pos);

            cur_pos = strtok(NULL, " \t\r\n");
            if(!cur_pos)
            {
                break;
            }
            language = atoi(cur_pos);

            cur_pos = strtok(NULL, " \t\r\n");
            if(!cur_pos)
            {
                break;
            }
            is_npc = atoi(cur_pos);

            cur_pos = strtok(NULL, " \t\r\n");
            if(!cur_pos)
            {
                break;
            }
            is_new_player_al = atoi(cur_pos);

            break;
        }

        if(aid == 0)
        {
            return -2;
        }

        return 0;
    };
};


#undef MAX_NAME_LEN


#endif

