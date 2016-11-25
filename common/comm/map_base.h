#ifndef _MAP_BASE_H_
#define _MAP_BASE_H_

#include "game_info.h"
#include "aws_table_map.h"
#include "game_define.h"
#include "bussiness_struct.h"

#include "time_utils.h"


/*
    �����ͼ�����궨�壨ԭ������Ϊ(1,1)��(��������Ϊ800*800)(province�Ĵ�СΪ200*200)(block�Ĵ�СΪ5*5)

    ------ x
    |
    |   
    y

    province:
    -----------------------------------------------------
    | Province_0 | Province_1 | Province_2 | Province_3 |
    -----------------------------------------------------
    | Province_4 | Province_5 | Province_6 | Province_7 |
    -----------------------------------------------------
    | Province_8 | Province_9 | Province_10| Province_11|
    -----------------------------------------------------
    | Province_12| Province_13| Province_14| Province_15|
    -----------------------------------------------------

    block:
    ----------------------------------------
    |  block_1    | ... | ... | block_160   |
    -----------------------------------------
    | block_201   | ... | ... | block_360   |
    -----------------------------------------
    | block_401   | ... | ... | block_560   |
    -----------------------------------------
    |   ...       | ... | ... |   ...       |
    -----------------------------------------
    | block_31801 | ... | ... | block_31960 |
    -----------------------------------------



    map_get������
                    7
                    |
                    |
                    |
            3       |
            |       |
            |       |
            |       |      
            2<--.-->2
            |       |
            |       |
            |       |
            |       3
            |
            |
            |
            7        
                     
*/
class CMapBase
{
public:
    // function  ===> ����һ��map������troop���͵���mightֵ(totaltroopmight=sum(coefficient*singletroopnum))
    // in_value  ===> pTbMap: ��������map
    // out_value ===> map��troop��mightֵ
    // ComputeBossWildMight
    static TINT64 GetMapMight(TbMap *ptbMap);

    // function  ===> ����һ��map����Ϣ������ԭ��map��ԭʼ����r_type������map�ĸ���ʱ��
    // in_value  ===> pTbMap: ��������map  
    // ResetWild
    static TVOID ResetMap(TbMap *ptbMap);

    // function  ===> ��ȡ���Խ������еĵ�ͼ���
    // in_value  ===> dwMapType: map������
    //                dwMapPos: map��posֵ  
    // out_value ===> ����map��pfalg
    // GetPflag
    static TINT64 GetPflag(TINT64 dwMapType, TINT64 dwMapPos);

    // function  ===> ��ȡ���Խ������еĵ�ͼ���
    // in_value  ===> ptbMap: ��������map
    // ComputeWildInfo
    static TVOID ComputeWildInfo(TbMap *ptbMap);

    // function  ===> ��ȡmap����block��idx
    // in_value  ===> ddwMapPos: map��posֵ
    // GetBlockIdFromPos
    static TINT64 GetBlockIdFromPos(TINT64 ddwMapPos);

    static TVOID SetMonseterNestMap(TbMap *ptbMap);

    static TVOID SetLeaderMonsterMap(TbMap *ptbMap);

    static TUINT32 GetWildBlockNumByType(TUINT32 udwSid, TUINT32 udwType);

private:
    // function  ===> ����map��pos��ȡ��pos���ڵ�ʡ��
    // in_value  ===> ddwMapPos: map��posֵ       
    // out_value ===> ���ظ�pos����province��idx(0-15)
    // GetProvinceFromPos
    static TINT64 GetProvinceFromPos(TINT64 ddwMapPos);



};
#endif
