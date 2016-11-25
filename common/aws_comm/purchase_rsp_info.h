#ifndef _PURCHASE_RSP_INFO_H_
#define _PURCHASE_RSP_INFO_H_
#include <string>
using namespace std;

struct PurchaseRspInfo
{
    unsigned int udwServiceType;
    int dwRetCode;
    unsigned int udwCostTime;
    unsigned int udwResType;
    string sRspContent;

    PurchaseRspInfo()
    {
        udwServiceType = 0;
        dwRetCode = 0;
        udwCostTime = 0;
        udwResType = 0;
        sRspContent.clear();
    }

    void Reset()
    {
        udwServiceType = 0;
        dwRetCode = 0;
        udwCostTime = 0;
        udwResType = 0;
        sRspContent.clear();
    }
};

#endif      