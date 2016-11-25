#ifndef _PURCHASE_REQ_INFO_H_
#define _PURCHASE_REQ_INFO_H_
#include <string>

using namespace std;

struct PurchaseReqInfo
{
	string sReqContent;

    PurchaseReqInfo()
    {
        Reset();
    }

    void Reset()
    {
        sReqContent.clear();
    }
};

#endif
