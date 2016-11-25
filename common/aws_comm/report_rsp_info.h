#ifndef _REPORT_RSP_INFO_H_
#define _REPORT_RSP_INFO_H_
#include <string>
using namespace std;

struct ReportRspInfo
{
    int dwRetCode;
    unsigned int udwServiceType;
    int dwRspType; // 0: report 1: mail
    string sRspContent;
    ReportRspInfo()
    {
        dwRetCode = 0;
        udwServiceType = 0;
        dwRspType = 0;
        sRspContent = "";
    }

    void Reset()
    {
        dwRetCode = 0;
        udwServiceType = 0;
        dwRspType = 0;
        sRspContent = "";
    }
};

#endif      