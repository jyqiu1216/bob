#include "aws_response.h"
#include "statistic.h"

int CAwsResponse::OnScanRsp(const AwsRspInfo& rspInfo, AwsTable* pAwsTable, unsigned int udwObjSize, unsigned int udwObjMaxNum,
    AwsTable& objLastEvaluatedKey, bool& bLastEvaluatedKey, int& dwScannedCount)
{
    AwsConsumeInfo consumeInfo;
    consumeInfo.sOperName = "Read";
    int dwRet = OnScanRsp(rspInfo, pAwsTable, udwObjSize, udwObjMaxNum, objLastEvaluatedKey, bLastEvaluatedKey, dwScannedCount, consumeInfo);
    CStatistic::Instance()->AddAwsConsume(consumeInfo);
    return dwRet;
}

int CAwsResponse::OnScanRsp(const AwsRspInfo& rspInfo, AwsTable* pAwsTable, unsigned int udwObjSize, unsigned int udwObjMaxNum,
    AwsTable& objLastEvaluatedKey, bool& bLastEvaluatedKey, int& dwScannedCount, AwsConsumeInfo& consumeInfo)
{
    Json::Reader jsonReader;
    Json::Value jsonRoot;
    CJsoncppUnseri jsonUnseri;

    if (rspInfo.dwRetCode != 200)
    {
        return -1;
    }
    if (rspInfo.udwResType == EN_CONTENT_TYPE__STRING)
    {
        if (!jsonReader.parse(rspInfo.sRspContent, jsonRoot))
        {
            return -2;
        }
    }
    else
    {
        if (0 != jsonUnseri.unserializeToDom(rspInfo.sRspContent.c_str(), jsonRoot, UNSERI_MODE_REPLACE))
        {
            return -2;
        }
    }
    if (GetConsumeInfo(jsonRoot, consumeInfo) != 0)
    {
        return -3;
    }
    if (!jsonRoot.isMember("Count"))
    {
        return -4;
    }
    if (!jsonRoot.isMember("ScannedCount"))
    {
        return -5;
    }
    dwScannedCount = jsonRoot["ScannedCount"].asInt();
    bLastEvaluatedKey = false;
    if (jsonRoot.isMember("LastEvaluatedKey"))
    {
        bLastEvaluatedKey = true;
        objLastEvaluatedKey.OnScanRsp(jsonRoot["LastEvaluatedKey"]);
    }
    AwsTable* pTbl = NULL;
    unsigned int udwCnt = jsonRoot["Count"].asUInt();
    for (unsigned int i = 0; i < udwCnt && i < udwObjMaxNum; ++i)
    {
        pTbl = (AwsTable*)((char*)pAwsTable+i*udwObjSize);
        pTbl->OnQueryRsp(jsonRoot["Items"][i]);
    }
    return udwCnt < udwObjMaxNum ? udwCnt : udwObjMaxNum;
}

int CAwsResponse::OnQueryRsp(const AwsRspInfo& rspInfo, AwsTable* pAwsTable, unsigned int udwObjSize, unsigned int udwObjMaxNum)
{
    AwsConsumeInfo consumeInfo;
    consumeInfo.sOperName = "Read";
    int dwRet = OnQueryRsp(rspInfo, pAwsTable, udwObjSize, udwObjMaxNum, consumeInfo);
    CStatistic::Instance()->AddAwsConsume(consumeInfo);
    return dwRet;
}

int CAwsResponse::OnQueryRsp(const AwsRspInfo& rspInfo, AwsTable* pAwsTable, unsigned int udwObjSize, unsigned int udwObjMaxNum, AwsConsumeInfo& consumeInfo)
{
	Json::Reader jsonReader;
	Json::Value jsonRoot;
    CJsoncppUnseri jsonUnseri;

	if (rspInfo.dwRetCode != 200)
	{
		return -1;
	}
    if (rspInfo.udwResType == EN_CONTENT_TYPE__STRING)
    {
        if (!jsonReader.parse(rspInfo.sRspContent, jsonRoot))
        {
            return -2;
        }
    }
    else
    {
        if (0 != jsonUnseri.unserializeToDom(rspInfo.sRspContent.c_str(), jsonRoot, UNSERI_MODE_REPLACE))
        {
            return -2;
        }
    }
	if (GetConsumeInfo(jsonRoot, consumeInfo) != 0)
	{
		return -3;
	}
	if (!jsonRoot.isMember("Count"))
	{
		return -4;
	}
	AwsTable* pTbl = NULL;
	unsigned int udwCnt = jsonRoot["Count"].asUInt();
	for (unsigned int i = 0; i < udwCnt && i < udwObjMaxNum; ++i)
	{
		pTbl = (AwsTable*)((char*)pAwsTable+i*udwObjSize);
		pTbl->OnQueryRsp(jsonRoot["Items"][i]);
	}
	return udwCnt < udwObjMaxNum ? udwCnt : udwObjMaxNum;
}

int CAwsResponse::OnQueryCountRsp(const AwsRspInfo& rspInfo, AwsConsumeInfo& consumeInfo)
{
    Json::Reader jsonReader;
	Json::Value jsonRoot;
    CJsoncppUnseri jsonUnseri;

	if (rspInfo.dwRetCode != 200)
	{
		return -1;
	}
    if (rspInfo.udwResType == EN_CONTENT_TYPE__STRING)
    {
        if (!jsonReader.parse(rspInfo.sRspContent, jsonRoot))
        {
            return -2;
        }
    }
    else
    {
        if (0 != jsonUnseri.unserializeToDom(rspInfo.sRspContent.c_str(), jsonRoot, UNSERI_MODE_REPLACE))
        {
            return -2;
        }
    }
	if (GetConsumeInfo(jsonRoot, consumeInfo) != 0)
	{
		return -3;
	}
	if (!jsonRoot.isMember("Count"))
	{
		return -4;
	}
    unsigned int udwCnt = jsonRoot["Count"].asUInt();
    return udwCnt;
}

int CAwsResponse::OnQueryCountRsp(const AwsRspInfo& rspInfo)
{
    AwsConsumeInfo consumeInfo;
    consumeInfo.sOperName = "Read";
    int dwRet = OnQueryCountRsp(rspInfo, consumeInfo);
    CStatistic::Instance()->AddAwsConsume(consumeInfo);
    return dwRet;
}

int CAwsResponse::OnBatchGetItemRsp(const AwsRspInfo& rspInfo, AwsTable* pAwsTable, unsigned int udwObjSize, unsigned int udwObjMaxNum)
{
    Json::Reader jsonReader;
    Json::Value jsonRoot;
    CJsoncppUnseri jsonUnseri;

    if (rspInfo.dwRetCode != 200)
    {
        return -1;
    }
    if (rspInfo.udwResType == EN_CONTENT_TYPE__STRING)
    {
        if (!jsonReader.parse(rspInfo.sRspContent, jsonRoot))
        {
            return -2;
        }
    }
    else
    {
        if (0 != jsonUnseri.unserializeToDom(rspInfo.sRspContent.c_str(), jsonRoot, UNSERI_MODE_REPLACE))
        {
            return -2;
        }
    }
    AwsConsumeInfo consumeInfo;
    if (GetConsumeInfo(jsonRoot, consumeInfo) != 0)
    {
        return -3;
    }
    CStatistic::Instance()->AddAwsConsume(consumeInfo);
    if (!jsonRoot.isMember("Responses"))
    {
        return -4;
    }
    Json::Value& jsonResponses = jsonRoot["Responses"];
    if (!jsonResponses.isMember(rspInfo.sTableName))
    {
        return 0; //这里应该返回0还是-5?
    }
    AwsTable* pTbl = NULL;
    unsigned int udwCnt = jsonRoot[rspInfo.sTableName].size();
    for(unsigned int i = 0; i < udwCnt && i < udwObjMaxNum; ++i)
    {
        pTbl = (AwsTable*)((char*)pAwsTable+i*udwObjSize);
        pTbl->OnResponse(jsonRoot[rspInfo.sTableName][i]);
    }
    return udwCnt < udwObjMaxNum ? udwCnt : udwObjMaxNum;
}

int CAwsResponse::OnGetItemRsp(const AwsRspInfo& rspInfo, AwsTable *pAwsTable, AwsConsumeInfo& consumeInfo)
{
	Json::Reader jsonReader;
	Json::Value jsonRoot;
    CJsoncppUnseri jsonUnseri;

	if (rspInfo.dwRetCode != 200)
	{
		return -1;
	}
    if (rspInfo.udwResType == EN_CONTENT_TYPE__STRING)
    {
        if (!jsonReader.parse(rspInfo.sRspContent, jsonRoot))
        {
            return -2;
        }
    }
    else
    {
        if (0 != jsonUnseri.unserializeToDom(rspInfo.sRspContent.c_str(), jsonRoot, UNSERI_MODE_REPLACE))
        {
            return -2;
        }
    }
	if (GetConsumeInfo(jsonRoot, consumeInfo) != 0)
	{
		return -3;
	}
    if (!jsonRoot.isMember("Item"))
    {
        return 0;
    }
	pAwsTable->OnGetItemRsp(jsonRoot["Item"]);
    return 1;
}

int CAwsResponse::OnGetItemRsp(const AwsRspInfo& rspInfo, AwsTable *pAwsTable)
{
    AwsConsumeInfo consumeInfo;
    consumeInfo.sOperName = "Read";
    int dwRet = OnGetItemRsp(rspInfo, pAwsTable, consumeInfo);
    CStatistic::Instance()->AddAwsConsume(consumeInfo);
    return dwRet;
}

int CAwsResponse::OnUpdateItemRsp(const AwsRspInfo& rspInfo, AwsTable *pAwsTable, AwsConsumeInfo& consumeInfo)
{
    if (201 == rspInfo.dwRetCode)
    {
        return 0;
    }
	Json::Reader jsonReader;
    Json::Value jsonRoot;
    CJsoncppUnseri jsonUnseri;

	if (rspInfo.dwRetCode != 200)
	{
		return -1;
	}
    if (rspInfo.udwResType == EN_CONTENT_TYPE__STRING)
    {
        if (!jsonReader.parse(rspInfo.sRspContent, jsonRoot))
        {
            return -2;
        }
    }
    else
    {
        if (0 != jsonUnseri.unserializeToDom(rspInfo.sRspContent.c_str(), jsonRoot, UNSERI_MODE_REPLACE))
        {
            return -2;
        }
    }
	if (GetConsumeInfo(jsonRoot, consumeInfo) != 0)
	{
		return -3;
	}
    if (!jsonRoot.isMember("Attributes"))
    {
        return 0;
    }
	pAwsTable->OnUpdateItemRsp(jsonRoot["Attributes"]);
    return 1;
}

int CAwsResponse::OnUpdateItemRsp(const AwsRspInfo& rspInfo, AwsTable *pAwsTable)
{
    AwsConsumeInfo consumeInfo;
    consumeInfo.sOperName = "Write";
    int dwRet = OnUpdateItemRsp(rspInfo, pAwsTable, consumeInfo);
    CStatistic::Instance()->AddAwsConsume(consumeInfo);
    return dwRet;
}

int CAwsResponse::GetConsumeInfo(const Json::Value& jsonRoot, AwsConsumeInfo& consumeInfo)
{
	if (!jsonRoot.isMember("ConsumedCapacity"))
	{
		return -1;
	}
	const Json::Value& jsonConsumed = jsonRoot["ConsumedCapacity"];
	consumeInfo.sTableName = jsonConsumed["TableName"].asString();
	consumeInfo.dConsumeUnit = jsonConsumed["CapacityUnits"].asDouble();
	if (consumeInfo.sTableName.empty() || consumeInfo.dConsumeUnit<0.0)
	{
		return -2;
	}
	return 0;
}
