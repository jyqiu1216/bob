#ifndef _MAP_BASE_H_
#define _MAP_BASE_H_

#include "game_info.h"
#include "aws_table_map.h"
#include "game_define.h"
#include "bussiness_struct.h"

#include "time_utils.h"


/*
    世界地图的坐标定义（原点坐标为(1,1)）(坐标总数为800*800)(province的大小为200*200)(block的大小为5*5)

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



    map_get的区域
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
    // function  ===> 计算一块map上所有troop类型的总might值(totaltroopmight=sum(coefficient*singletroopnum))
    // in_value  ===> pTbMap: 待操作的map
    // out_value ===> map上troop的might值
    // ComputeBossWildMight
    static TINT64 GetMapMight(TbMap *ptbMap);

    // function  ===> 重置一块map的信息，保留原来map的原始类型r_type，更新map的更新时间
    // in_value  ===> pTbMap: 待操作的map  
    // ResetWild
    static TVOID ResetMap(TbMap *ptbMap);

    // function  ===> 获取可以建立城市的地图标记
    // in_value  ===> dwMapType: map的类型
    //                dwMapPos: map的pos值  
    // out_value ===> 返回map的pfalg
    // GetPflag
    static TINT64 GetPflag(TINT64 dwMapType, TINT64 dwMapPos);

    // function  ===> 获取可以建立城市的地图标记
    // in_value  ===> ptbMap: 待操作的map
    // ComputeWildInfo
    static TVOID ComputeWildInfo(TbMap *ptbMap);

    // function  ===> 获取map所属block的idx
    // in_value  ===> ddwMapPos: map的pos值
    // GetBlockIdFromPos
    static TINT64 GetBlockIdFromPos(TINT64 ddwMapPos);

    static TVOID SetMonseterNestMap(TbMap *ptbMap);

    static TVOID SetLeaderMonsterMap(TbMap *ptbMap);

    static TUINT32 GetWildBlockNumByType(TUINT32 udwSid, TUINT32 udwType);

private:
    // function  ===> 根据map的pos获取该pos所在的省份
    // in_value  ===> ddwMapPos: map的pos值       
    // out_value ===> 返回该pos所在province的idx(0-15)
    // GetProvinceFromPos
    static TINT64 GetProvinceFromPos(TINT64 ddwMapPos);



};
#endif
