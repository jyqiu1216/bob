#ifndef _DB_RSP_INFO_H_
#define _DB_RSP_INFO_H_
#include <string>
using namespace std;

struct DbRspInfo
{
    int dwRetCode;
    unsigned int udwIdxNo; //�򿪷�ʽ,�������������ͱ���action
    unsigned int udwServiceType;
    unsigned int udwCostTime;
    string sRspContent;
    string sTableName;
    string sOperatorName;
    DbRspInfo()
    {
        Reset();
    }
    void Reset()
    {
        dwRetCode = 0;
        sRspContent = "";
        sTableName = "";
        sOperatorName = "";
        udwIdxNo = 0;
        udwServiceType = 0;
        udwCostTime = 0;
    }
};


#endif      

