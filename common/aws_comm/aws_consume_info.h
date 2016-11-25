#ifndef _AWS_CONSUME_INFO_H_
#define _AWS_CONSUME_INFO_H_

#include <string>
using namespace std;

class AwsConsumeInfo
{
public:
	string sTableName;
    string sOperName;
    double dConsumeUnit;
	//unsigned int udwConsumeUnit;
	//AwsConsumeInfo(const string& sTable="", unsigned int udwUnit=0) : sTableName(sTable), udwConsumeUnit(udwUnit)
    AwsConsumeInfo(const string& sTable="", const string& sOper="", double dUnit=0.0) : sTableName(sTable), sOperName(sOper), dConsumeUnit(dUnit)
	{}
};

#endif