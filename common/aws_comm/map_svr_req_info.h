#ifndef _MAP_SVR_REQ_INFO_H_
#define _MAP_SVR_REQ_INFO_H_
#include <string>
using namespace std;

struct MapSvrReqInfo
{
    int dwSid;
	string sOperator;
	string sReqContent;
    MapSvrReqInfo(int sid = -1, const string& sOp = "", const string& sReq = "") :dwSid(sid), sOperator(sOp), sReqContent(sReq)
	{}

    MapSvrReqInfo()
    {
        dwSid = -1;
        sOperator.clear();
        sReqContent.clear();
    }

    void SetVal(int sid = -1, const string& sOp = "", const string& sReq = "")
    {
        dwSid = sid;
        sOperator = sOp;
        sReqContent = sReq;
    }
};

#endif
