#ifndef _EVENT_RSP_INFO_H_
#define _EVENT_RSP_INFO_H_
#include <string>
using namespace std;

struct EventRspInfo
{
    int dwRetCode;
    unsigned int udwIdxNo; //打开方式,用于区分主动和被动action
    unsigned int udwServiceType;
    unsigned int udwCostTime;
    unsigned int udwReqSeq;
    TUINT32 udwReqType;
    string sRspContent;

    EventRspInfo()
    {
        
    }

    void Reset()
    {
        dwRetCode = 0;
        udwIdxNo = 0;
        udwServiceType = 0;
        udwCostTime = 0;
        udwReqSeq = 0;
        udwReqType = 0;
        sRspContent = "";
    }
};

#endif      