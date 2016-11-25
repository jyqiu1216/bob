#ifndef _AWS_RESPONSE_H_
#define _AWS_RESPONSE_H_

//#include "hs_define.h"
#include "player_info.h"
#include <assert.h>
#include "base/log/wtselogger.h"
#include "aws_table_common.h"
#include "aws_consume_info.h"
#include "auto_vector.h"
#include "aws_rsp_info.h"
#include "jsoncpp_utils.h"
#include <string>
using namespace std;


class CAwsResponse
{
public:
	static int OnQueryRsp(const AwsRspInfo& rspInfo, AwsTable* pAwsTable, unsigned int udwObjSize, unsigned int udwObjMaxNum, AwsConsumeInfo& consumeInfo);
    static int OnQueryRsp(const AwsRspInfo& rspInfo, AwsTable* pAwsTable, unsigned int udwObjSize, unsigned int udwObjMaxNum);

    static int OnBatchGetItemRsp(const AwsRspInfo& rspInfo, AwsTable* pAwsTable, unsigned int udwObjSize, unsigned int udwObjMaxNum);
	
    static int OnGetItemRsp(const AwsRspInfo& rspInfo, AwsTable *pAwsTable, AwsConsumeInfo& consumeInfo);
    static int OnGetItemRsp(const AwsRspInfo& rspInfo, AwsTable *pAwsTable);

	static int OnUpdateItemRsp(const AwsRspInfo& rspInfo, AwsTable *pAwsTable, AwsConsumeInfo& consumeInfo);
    static int OnUpdateItemRsp(const AwsRspInfo& rspInfo, AwsTable *pAwsTable);

public:
	static int GetConsumeInfo(const Json::Value& jsonRoot, AwsConsumeInfo& consumeInfo);
};

#endif


