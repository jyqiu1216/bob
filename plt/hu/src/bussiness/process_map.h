#ifndef _PROCESS_MAP_H_
#define _PROCESS_MAP_H_


#include "session.h"


class CProcessMap
{

public: 

    // function  ===> 获取地图数据(以地图的pos为最少单位)
    static TINT32 ProcessCmd_MapGet(SSession *pstSession, TBOOL &bNeedResponse);
    
    // function  ===> 获取地图数据(以地图的block为最少单位)(todo: 该特性还有么)
    static TINT32 ProcessCmd_MapBlockGet(SSession *pstSession, TBOOL &bNeedResponse);

    static TINT32 ProcessCmd_ManorInfo(SSession* pstSession, TBOOL& bNeedResponse);

    static TINT32 ProcessCmd_PrisionInfo(SSession* pstSession, TBOOL& bNeedResponse);
    
};



#endif
