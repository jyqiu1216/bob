#ifndef _DB_RESPONSE_H_
#define _DB_RESPONSE_H_

#include <assert.h>
#include "base/log/wtselogger.h"
#include "db_rsp_info.h"
#include <string>
#include "db_struct_define.h"
using namespace std;


class CDbResponse
{
public:
    //static int OnSelectResponse(const DbRspInfo& rspInfo, AllianceRank* pAllianceRank, unsigned int udwMaxNum);
    static int OnSelectResponse(const DbRspInfo& rspInfo, PlayerRecommend* pRecommend, unsigned int udwMaxNum);
    static int OnSelectResponse(const DbRspInfo& rspInfo, ComputeTime* pComputeTime, unsigned int udwMaxNum);
    static int OnSelectResponse(const DbRspInfo& rspInfo, AllianceRank* pAlRank, unsigned int udwMaxNum);
    static int OnSelectResponse(const DbRspInfo& rspInfo, TradeCityInfo* pTradeCityInfo, unsigned int udwMaxNum);

    static int OnCountResponse(const DbRspInfo& rspInfo);
};

#endif


