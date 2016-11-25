#ifndef _PLAYER_RANK_INFO_H_
#define _PLAYER_RANK_INFO_H_

#include "base/common/wtse_std_header.h"
#define MAX_NAME_LEN 32

struct PlayerRankInfo
{
    TINT32 uid;
    TINT32 aid;

    TINT32 rank_type;

    TINT32 rank1;// EN_RANK_TYPE_PLAYER_FORCE_KILL
    TINT32 rank2;// EN_RANK_TYPE_PLAYER_TROOPS_KILL
    TINT32 rank3;// EN_RANK_TYPE_PLAYER_FORCE
    TINT32 rank4;// EN_RANK_TYPE_PLAYER_TROOPS_KDR
    TINT32 rank5;// EN_RANK_TYPE_PLAYER_BATTLE_WON
    TINT32 rank6;// EN_RANK_TYPE_PLAYER_BWLR
    TINT32 rank7;// EN_RANK_TYPE_PLAYER_DRAGON_CAPTURED
    TINT32 rank8;// EN_RANK_TYPE_PLAYER_DRAGON_EXECUTED
    TINT32 rank9;//EN_RANK_TYPE_PLAYER_EVIL_TROOP_KILL
    TINT32 rank10;//EN_RANK_TYPE_PLAYER_EVIL_FORCE_KILL
    TINT32 rank11;//EN_RANK_TYPE_PLAYER_GAIN_RESOURCE
    TINT32 rank12;//EN_RANK_TYPE_PLAYER_TRANSPORT_RESOURCE_0
    TINT32 rank13;//EN_RANK_TYPE_PLAYER_TRANSPORT_RESOURCE_1
    TINT32 rank14;//EN_RANK_TYPE_PLAYER_TRANSPORT_RESOURCE_2
    TINT32 rank15;//EN_RANK_TYPE_PLAYER_TRANSPORT_RESOURCE_3
    TINT32 rank16;//EN_RANK_TYPE_PLAYER_TRANSPORT_RESOURCE_4
    TINT32 rank17;//EN_RANK_TYPE_PLAYER_KILL_MONSTER

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
    TINT64 value11;     //EN_RANK_TYPE_PLAYER_GAIN_RESOURCE
    TINT64 value12;     //EN_RANK_TYPE_PLAYER_TRANSPORT_RESOURCE_0 
    TINT64 value13;     //EN_RANK_TYPE_PLAYER_TRANSPORT_RESOURCE_1
    TINT64 value14;     //EN_RANK_TYPE_PLAYER_TRANSPORT_RESOURCE_2
    TINT64 value15;     //EN_RANK_TYPE_PLAYER_TRANSPORT_RESOURCE_3
    TINT64 value16;     //EN_RANK_TYPE_PLAYER_TRANSPORT_RESOURCE_4
    TINT64 value17;     //EN_RANK_TYPE_PLAYER_KILL_MONSTER

    TINT64 value;

    TCHAR alnick[5];
    TCHAR uname[MAX_NAME_LEN];
    TCHAR alname[MAX_NAME_LEN];

    TVOID reset()
    {
        memset((TVOID*)this, 0, sizeof(PlayerRankInfo));
    };

    TINT32 load_data(TCHAR* raw_data)
    {
        if(strlen(raw_data) < 10)
        {
            return -1;
        }

        TCHAR* cur_pos = NULL;


        //cur_pos = strtok(raw_data, " \t\r\n");
        cur_pos = raw_data;
        uid = atoi(cur_pos);

        cur_pos = strchr((char*)cur_pos, '\t');
        if (!cur_pos)
        {
            return -2;
        }
        cur_pos++;
        aid = atoi(cur_pos);

        cur_pos = strchr((char*)cur_pos, '\t');
        if (!cur_pos)
        {
            return -2;
        }
        cur_pos++;
        rank1 = atoi(cur_pos);

        cur_pos = strchr((char*)cur_pos, '\t');
        if (!cur_pos)
        {
            return -2;
        }
        cur_pos++;
        rank2 = atoi(cur_pos);

        cur_pos = strchr((char*)cur_pos, '\t');
        if (!cur_pos)
        {
            return -2;
        }
        cur_pos++;
        rank3 = atoi(cur_pos);

        cur_pos = strchr((char*)cur_pos, '\t');
        if (!cur_pos)
        {
            return -2;
        }
        cur_pos++;
        rank4 = atoi(cur_pos);

        cur_pos = strchr((char*)cur_pos, '\t');
        if (!cur_pos)
        {
            return -2;
        }
        cur_pos++;
        rank5 = atoi(cur_pos);

        cur_pos = strchr((char*)cur_pos, '\t');
        if (!cur_pos)
        {
            return -2;
        }
        cur_pos++;
        rank6 = atoi(cur_pos);

        cur_pos = strchr((char*)cur_pos, '\t');
        if (!cur_pos)
        {
            return -2;
        }
        cur_pos++;
        rank7 = atoi(cur_pos);

        cur_pos = strchr((char*)cur_pos, '\t');
        if (!cur_pos)
        {
            return -2;
        }
        cur_pos++;
        rank8 = atoi(cur_pos);

        cur_pos = strchr((char*)cur_pos, '\t');
        if (!cur_pos)
        {
            return -2;
        }
        cur_pos++;
        value1 = strtoull(cur_pos, NULL, 10);

        cur_pos = strchr((char*)cur_pos, '\t');
        if (!cur_pos)
        {
            return -2;
        }
        cur_pos++;
        value2 = strtoull(cur_pos, NULL, 10);

        cur_pos = strchr((char*)cur_pos, '\t');
        if (!cur_pos)
        {
            return -2;
        }
        cur_pos++;
        value3 = strtoull(cur_pos, NULL, 10);

        cur_pos = strchr((char*)cur_pos, '\t');
        if (!cur_pos)
        {
            return -2;
        }
        cur_pos++;
        value4 = strtoull(cur_pos, NULL, 10);

        cur_pos = strchr((char*)cur_pos, '\t');
        if (!cur_pos)
        {
            return -2;
        }
        cur_pos++;
        value5 = strtoull(cur_pos, NULL, 10);

        cur_pos = strchr((char*)cur_pos, '\t');
        if (!cur_pos)
        {
            return -2;
        }
        cur_pos++;
        value6 = strtoull(cur_pos, NULL, 10);

        cur_pos = strchr((char*)cur_pos, '\t');
        if (!cur_pos)
        {
            return -2;
        }
        cur_pos++;
        value7 = strtoull(cur_pos, NULL, 10);

        cur_pos = strchr((char*)cur_pos, '\t');
        if (!cur_pos)
        {
            return -2;
        }
        cur_pos++;
        value8 = strtoull(cur_pos, NULL, 10);

        cur_pos = strchr((char*)cur_pos, '\t');
        if (!cur_pos)
        {
            return -2;
        }
        cur_pos++;
        strncpy(alnick, cur_pos, 5);
        alnick[4] = '\0';
        str_split(alnick, 5, '\t');

        cur_pos = strchr((char*)cur_pos, '\t');
        if (!cur_pos)
        {
            return -2;
        }
        cur_pos++;
        strncpy(uname, cur_pos, MAX_NAME_LEN);
        uname[MAX_NAME_LEN - 1] = '\0';
        str_split(uname, MAX_NAME_LEN, '\t');

        cur_pos = strchr((char*)cur_pos, '\t');
        if (!cur_pos)
        {
            return -2;
        }
        cur_pos++;
        strncpy(alname, cur_pos, MAX_NAME_LEN);
        alname[MAX_NAME_LEN - 1] = '\0';
        str_split(alname, MAX_NAME_LEN, '\t');

        cur_pos = strchr((char*)cur_pos, '\t');
        if (!cur_pos)
        {
            return -2;
        }
        cur_pos++;
        //compute_time

        cur_pos = strchr((char*)cur_pos, '\t');
        if (!cur_pos)
        {
            return -2;
        }
        cur_pos++;
        rank9 = atoi(cur_pos);

        cur_pos = strchr((char*)cur_pos, '\t');
        if (!cur_pos)
        {
            return -2;
        }
        cur_pos++;
        value9 = strtoull(cur_pos, NULL, 10);

        cur_pos = strchr((char*)cur_pos, '\t');
        if (!cur_pos)
        {
            return -2;
        }
        cur_pos++;
        rank10 = atoi(cur_pos);

        cur_pos = strchr((char*)cur_pos, '\t');
        if (!cur_pos)
        {
            return -2;
        }
        cur_pos++;
        value10 = strtoull(cur_pos, NULL, 10);

        cur_pos = strchr((char*)cur_pos, '\t');
        if (!cur_pos)
        {
            return -2;
        }
        cur_pos++;
        rank11 = atoi(cur_pos);

        cur_pos = strchr((char*)cur_pos, '\t');
        if (!cur_pos)
        {
            return -2;
        }
        cur_pos++;
        value11 = strtoull(cur_pos, NULL, 10);

        cur_pos = strchr((char*)cur_pos, '\t');
        if (!cur_pos)
        {
            return -2;
        }
        cur_pos++;
        rank12 = atoi(cur_pos);

        cur_pos = strchr((char*)cur_pos, '\t');
        if (!cur_pos)
        {
            return -2;
        }
        cur_pos++;
        value12 = strtoull(cur_pos, NULL, 10);

        cur_pos = strchr((char*)cur_pos, '\t');
        if (!cur_pos)
        {
            return -2;
        }
        cur_pos++;
        rank13 = atoi(cur_pos);

        cur_pos = strchr((char*)cur_pos, '\t');
        if (!cur_pos)
        {
            return -2;
        }
        cur_pos++;
        value13 = strtoull(cur_pos, NULL, 10);

        cur_pos = strchr((char*)cur_pos, '\t');
        if (!cur_pos)
        {
            return -2;
        }
        cur_pos++;
        rank14 = atoi(cur_pos);

        cur_pos = strchr((char*)cur_pos, '\t');
        if (!cur_pos)
        {
            return -2;
        }
        cur_pos++;
        value14 = strtoull(cur_pos, NULL, 10);

        cur_pos = strchr((char*)cur_pos, '\t');
        if (!cur_pos)
        {
            return -2;
        }
        cur_pos++;
        rank15 = atoi(cur_pos);

        cur_pos = strchr((char*)cur_pos, '\t');
        if (!cur_pos)
        {
            return -2;
        }
        cur_pos++;
        value15 = strtoull(cur_pos, NULL, 10);

        cur_pos = strchr((char*)cur_pos, '\t');
        if (!cur_pos)
        {
            return -2;
        }
        cur_pos++;
        rank16 = atoi(cur_pos);

        cur_pos = strchr((char*)cur_pos, '\t');
        if (!cur_pos)
        {
            return -2;
        }
        cur_pos++;
        value16 = strtoull(cur_pos, NULL, 10);

        cur_pos = strchr((char*)cur_pos, '\t');
        if (!cur_pos)
        {
            return -2;
        }
        cur_pos++;
        rank17 = atoi(cur_pos);

        cur_pos = strchr((char*)cur_pos, '\t');
        if (!cur_pos)
        {
            return -2;
        }
        cur_pos++;
        value17 = strtoull(cur_pos, NULL, 10);
        if(uid == 0)
        {
            return -2;
        }

        return 0;
    };

    TVOID str_split(TCHAR *pszStr, TUINT32 udwLen, TCHAR split)
    {
        for (TUINT32 udwIdx = 0; udwIdx < udwLen; ++udwIdx)
        {
            if (pszStr[udwIdx] == split)
            {
                pszStr[udwIdx] = '\0';
                return;
            }
        }
    }
};

#undef MAX_NAME_LEN
#endif
