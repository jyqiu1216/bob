#ifndef _DB_REQ_INFO_H_
#define _DB_REQ_INFO_H_
#include <string>
using namespace std;

struct DbReqInfo
{
    unsigned int udwDbIdx; //svrid
	string sTableName;
	string sOperatorName;
	string sReqContent;
    unsigned int udwIdxNo; 
	DbReqInfo(unsigned int udwDb = 0, const string& sTable="", const string& sOperator="", const string& sReq="", unsigned int udwIdx = 0): udwDbIdx(udwDb),
        sTableName(sTable),sOperatorName(sOperator),sReqContent(sReq),udwIdxNo(udwIdx)
	{}

    void SetVal(unsigned int udwDb = 0, const string sTable = "", const string sOperator = "", unsigned int udwIndex = 0, const string &sReq="")
    {
        udwDbIdx = udwDb;
        sTableName = sTable;
        sOperatorName = sOperator;
        udwIdxNo = udwIndex;
        sReqContent = sReq;
    }	
};

#endif

