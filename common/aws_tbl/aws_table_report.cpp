#include "aws_table_report.h"

TableDesc TbReport::oTableDesc;

int TbReport::Init(const string& sConfFile, const string strProjectName)
{
	CFieldProperty fld_prop;
	if (fld_prop.Init(sConfFile.c_str()) == false)
	{
		assert(0);
		return -1;
	}
	fld_prop.GetTableInfo(&oTableDesc);
	oTableDesc.m_strProjectName = strProjectName;
	AwsTable::AddNewObjectFunc(oTableDesc.sName, NewObject);
	return 0;
}

AwsTable* TbReport::NewObject()
{
	return new TbReport;
}

string TbReport::GetTableName()
{
	ostringstream oss;
	if(!oTableDesc.m_strProjectName.empty())
	{
	oss << oTableDesc.m_strProjectName << "_";
	}
	oss << "split_";
	oss << oTableDesc.sName;
	oss << "_" << this->GetTableIdx();
	return oss.str();
}

TINT32 TbReport::GetTableIdx()
{
	return this->m_nId/1000000;
}

AwsMap* TbReport::OnScanReq(unsigned int udwIdxNo, bool bHasStartKey, bool bReturnConsumedCapacity,
	 unsigned int dwLimit, unsigned int dwSegment, unsigned int dwTotalSegments)
{
	oJsonWriter.omitEndingLineFeed();
	assert(oTableDesc.mIndexDesc.find(udwIdxNo)!=oTableDesc.mIndexDesc.end());
	IndexDesc& idx_desc = oTableDesc.mIndexDesc[udwIdxNo];
	assert(idx_desc.sName == "PRIMARY"); //ֻ��ͨ��������ѯ
	AwsMap* pScan = new AwsMap;
	assert(pScan);
	pScan->AddValue("/TableName", GetTableName());
	if (idx_desc.vecRtnFld.empty())
	{
		pScan->AddValue("/Select", "ALL_ATTRIBUTES");
	}
	else
	{
		ostringstream oss;
		for (unsigned int i = 0; i < idx_desc.vecRtnFld.size(); ++i)
		{
			oss.str("");
			oss << "/AttributesToGet[" << i << "]";
			FieldDesc& fld_desc = oTableDesc.mFieldDesc[idx_desc.vecRtnFld[i]];
			pScan->AddValue(oss.str(),  fld_desc.sName);
		}
	}
	if (dwLimit > 0)
	{
		pScan->AddNumber("/Limit", dwLimit);
	}
	if (bReturnConsumedCapacity)
	{
		pScan->AddValue("/ReturnConsumedCapacity", "TOTAL");
	}
	if (dwTotalSegments > 0)
	{
		pScan->AddNumber("/Segment", dwSegment);
		pScan->AddNumber("/TotalSegments", dwTotalSegments);
	}
	if (bHasStartKey)
	{
		pScan->AddValue("/ExclusiveStartKey/id/N",m_nId);
	}
	return pScan;
}

int TbReport::OnScanReq(string& sPostData, unsigned int udwIdxNo, bool bHasStartKey, bool bReturnConsumedCapacity,
 unsigned int dwLimit, unsigned int dwSegment, unsigned int dwTotalSegments)
{
	AwsMap* pScan = OnScanReq(udwIdxNo, bHasStartKey, bReturnConsumedCapacity, dwLimit, dwSegment, dwTotalSegments);
	ostringstream oss;
	pScan->Dump(oss);
	sPostData = oss.str();
	delete pScan;
	return 0;
}

int TbReport::OnScanRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbReport::OnQueryReq(unsigned int udwIdxNo, const CompareDesc& comp_desc,
	bool bConsistentRead, bool bReturnConsumedCapacity, bool bScanIndexForward, unsigned int dwLimit, bool bCount)
{
	oJsonWriter.omitEndingLineFeed();
	assert(oTableDesc.mIndexDesc.find(udwIdxNo)!=oTableDesc.mIndexDesc.end());
	IndexDesc& idx_desc = oTableDesc.mIndexDesc[udwIdxNo];
	AwsMap* pQuery = new AwsMap;
	assert(pQuery);
	pQuery->AddValue("/TableName", GetTableName());
	pQuery->AddBoolean("/ConsistentRead", bConsistentRead);
	if (!bScanIndexForward)
	{
		pQuery->AddBoolean("/ScanIndexForward", bScanIndexForward);
	}
	if (idx_desc.sName != "PRIMARY")
	{
		pQuery->AddValue("/IndexName", idx_desc.sName);
	}
	if (bCount)
	{
		pQuery->AddValue("/Select", "COUNT");
	}
	else if (idx_desc.vecRtnFld.empty())
	{
		pQuery->AddValue("/Select", "ALL_ATTRIBUTES");
	}
	else
	{
		ostringstream oss;
		for (unsigned int i = 0; i < idx_desc.vecRtnFld.size(); ++i)
		{
			oss.str("");
			oss << "/AttributesToGet[" << i << "]";
			FieldDesc& fld_desc = oTableDesc.mFieldDesc[idx_desc.vecRtnFld[i]];
			pQuery->AddValue(oss.str(),  fld_desc.sName);
		}
	}
	if (dwLimit > 0)
	{
		pQuery->AddNumber("/Limit", dwLimit);
	}
	if (bReturnConsumedCapacity)
	{
		pQuery->AddValue("/ReturnConsumedCapacity", "TOTAL");
	}
	for (unsigned int i = 0; i < idx_desc.vecIdxFld.size(); ++i)
	{
		FieldDesc& fld_desc = oTableDesc.mFieldDesc[idx_desc.vecIdxFld[i]];
		if (i == 0) //0ֻ����hash key��HASH KEYֻ����EQ��ʽ
		{
			if(fld_desc.udwFldNo == TbREPORT_FIELD_ID)
			{
				pQuery->AddValue("/KeyConditions/id/AttributeValueList[0]/N", m_nId);
				pQuery->AddValue("/KeyConditions/id/ComparisonOperator", "EQ");
			}
		}
		else if (i == 1) //1ֻ����rangekey or local index�������ж��ַ�ʽ
		{
			if (COMPARE_TYPE_EQ<=comp_desc.dwCompareType && comp_desc.dwCompareType<=COMPARE_TYPE_GT)
			{
			}
			else if (COMPARE_TYPE_BETWEEN == comp_desc.dwCompareType)
			{
			}
			else
			{
				assert(0);
			}
		}
		else
		{
			assert(0);
		}
	}
	return pQuery;
}

int TbReport::OnQueryReq(string& sPostData, unsigned int udwIdxNo, const CompareDesc& comp_desc,
	bool bConsistentRead, bool bReturnConsumedCapacity, bool bScanIndexForward, unsigned int dwLimit)
{
	oJsonWriter.omitEndingLineFeed();
	AwsMap* pQuery = OnQueryReq(udwIdxNo, comp_desc, bConsistentRead, bReturnConsumedCapacity, bScanIndexForward, dwLimit);
	ostringstream oss;
	pQuery->Dump(oss);
	sPostData = oss.str();
	delete pQuery;
	return 0;
}

int TbReport::OnQueryRsp(const Json::Value& item)
{
	return OnResponse(item);
}

int TbReport::OnCountReq(string& sPostData, unsigned int udwIdxNo, const CompareDesc& comp_desc, 
		bool bConsistentRead, bool bReturnConsumedCapacity, bool bScanIndexForward, unsigned int dwLimit)
{
	AwsMap* pQuery = OnQueryReq(udwIdxNo, comp_desc, bConsistentRead, bReturnConsumedCapacity, bScanIndexForward, dwLimit, true);
	ostringstream oss;
	pQuery->Dump(oss);
	sPostData = oss.str();
	delete pQuery;
	return 0;
}

AwsMap* TbReport::OnUpdateItemReq(
	const ExpectedDesc& expected_desc, int dwReturnValuesType, bool bReturnConsumedCapacity)
{
	oJsonWriter.omitEndingLineFeed();
	if (m_mFlag.size() == 0) //û�б仯
	{
		return NULL;
	}
	AwsMap* pUpdateItem = new AwsMap;
	assert(pUpdateItem);
	string sBase64Encode;
	string sJsonEncode;
	bool bUpdateFlag = false;
	pUpdateItem->AddValue("/TableName", GetTableName());
	if (bReturnConsumedCapacity)
	{
		pUpdateItem->AddValue("/ReturnConsumedCapacity", "TOTAL");
	}
	if (dwReturnValuesType > RETURN_VALUES_NONE)
	{
		pUpdateItem->AddValue("/ReturnValues", ReturnValuesType2Str(dwReturnValuesType));
	}
	pUpdateItem->AddValue("/Key/id/N", m_nId);
	for (map<unsigned int, int>::iterator iter = m_mFlag.begin(); iter != m_mFlag.end(); ++iter)
	{
		bUpdateFlag = true;
		if (TbREPORT_FIELD_TIME == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/time/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/time/Value/N", m_nTime);
				pUpdateItem->AddValue("/AttributeUpdates/time/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbREPORT_FIELD_TYPE == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/type/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/type/Value/N", m_nType);
				pUpdateItem->AddValue("/AttributeUpdates/type/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbREPORT_FIELD_FROM == iter->first)
		{
			if (!m_bFrom.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/from/Value/B", Base64Encode((char*)&m_bFrom.m_astList[0], m_bFrom.m_udwNum*sizeof(SReportUserInfo), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/from/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/from/Action", "DELETE");
			}
			continue;
		}
		if (TbREPORT_FIELD_TO == iter->first)
		{
			if (!m_bTo.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/to/Value/B", Base64Encode((char*)&m_bTo.m_astList[0], m_bTo.m_udwNum*sizeof(SReportUserInfo), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/to/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/to/Action", "DELETE");
			}
			continue;
		}
		if (TbREPORT_FIELD_CONTENT == iter->first)
		{
			if (!m_sContent.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/content/Value/S", JsonEncode(m_sContent, sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/content/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/content/Action", "DELETE");
			}
			continue;
		}
		if (TbREPORT_FIELD_RESULT == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/result/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/result/Value/N", m_nResult);
				pUpdateItem->AddValue("/AttributeUpdates/result/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbREPORT_FIELD_SID == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/sid/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/sid/Value/N", m_nSid);
				pUpdateItem->AddValue("/AttributeUpdates/sid/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbREPORT_FIELD_SEQ == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/seq/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/seq/Value/N", m_nSeq);
				pUpdateItem->AddValue("/AttributeUpdates/seq/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		assert(0);
	}
	if(bUpdateFlag)
	{
		++m_nSeq;
		pUpdateItem->AddValue("/AttributeUpdates/seq/Value/N", m_nSeq);
		pUpdateItem->AddValue("/AttributeUpdates/seq/Action", "PUT");
	}
	ostringstream oss;
	for (unsigned int i = 0; i < expected_desc.vecExpectedItem.size(); ++i)
	{
		const ExpectedItem& item = expected_desc.vecExpectedItem[i];
		FieldDesc& fld_desc = oTableDesc.mFieldDesc[item.udwFldNo];
		oss.str("");
		oss << "/Expected/" << fld_desc.sName << "/Exists";
		pUpdateItem->AddBoolean(oss.str(), item.bExists);
		if (item.bExists) //ExistsΪtrueʱ����ҪValue
		{
			oss.str("");
			oss << "/Expected/" << fld_desc.sName << "/Value/" << DataType2Str(fld_desc.dwType);
			if (FIELD_TYPE_N == fld_desc.dwType)
			{
				pUpdateItem->AddValue(oss.str(), item.nValue);
				continue;
			}
			if (FIELD_TYPE_S == fld_desc.dwType)
			{
				pUpdateItem->AddValue(oss.str(), item.sValue);
				continue;
			}
			//FIELD_TYPE_B�Ͳ���expected
			assert(0);
		}
	}
	return pUpdateItem;
}

int TbReport::OnUpdateItemReq(string& sPostData,
	const ExpectedDesc& expected_desc, int dwReturnValuesType, bool bReturnConsumedCapacity)
{
	AwsMap* pUpdateItem = OnUpdateItemReq(expected_desc, dwReturnValuesType, bReturnConsumedCapacity);
	if (!pUpdateItem)
	{
		sPostData.clear();
		return 0;
	}
	ostringstream oss;
	pUpdateItem->Dump(oss);
	sPostData = oss.str();
	delete pUpdateItem;
	return 0;
}

int TbReport::OnUpdateItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbReport::OnWriteItemReq(int dwActionType)
{
	++m_nSeq;
	oJsonWriter.omitEndingLineFeed();
	AwsMap* pWriteItem = new AwsMap;
	assert(pWriteItem);
	string sBase64Encode;
	string sJsonEncode;
	AwsMap* pItem = NULL;
	if (WRITE_ACTION_TYPE_PUT == dwActionType)
	{
		pItem = pWriteItem->GetAwsMap("PutRequest")->GetAwsMap("Item");
		pItem->AddValue("/id/N", m_nId);
		pItem->AddValue("/time/N", m_nTime);
		pItem->AddValue("/type/N", m_nType);
		if (!m_bFrom.empty())
		{
			pItem->AddValue("/from/B", Base64Encode((char*)&m_bFrom.m_astList[0], m_bFrom.m_udwNum*sizeof(SReportUserInfo), sBase64Encode));
		}
		if (!m_bTo.empty())
		{
			pItem->AddValue("/to/B", Base64Encode((char*)&m_bTo.m_astList[0], m_bTo.m_udwNum*sizeof(SReportUserInfo), sBase64Encode));
		}
		if (!m_sContent.empty())
		{
			pItem->AddValue("/content/S", JsonEncode(m_sContent, sJsonEncode));
		}
		pItem->AddValue("/result/N", m_nResult);
		pItem->AddValue("/sid/N", m_nSid);
		pItem->AddValue("/seq/N", m_nSeq);
	}
	else if (WRITE_ACTION_TYPE_DELETE == dwActionType)
	{
		pItem = pWriteItem->GetAwsMap("DeleteRequest")->GetAwsMap("Key");
		pItem->AddValue("/id/N", m_nId);
	}
	else
	{
		assert(0);
	}
	return pWriteItem;
}

void TbReport::OnWriteItemReq(AwsMap* pWriteItem, int dwActionType)
{
	++m_nSeq;
	assert(pWriteItem);
	string sBase64Encode;
	string sJsonEncode;
	AwsMap* pReqItem = new AwsMap;
	assert(pReqItem);
	AwsMap* pItem = NULL;
	if (WRITE_ACTION_TYPE_PUT == dwActionType)
	{
		pItem = pReqItem->GetAwsMap("PutRequest")->GetAwsMap("Item");
		pItem->AddValue("/id/N", m_nId);
		pItem->AddValue("/time/N", m_nTime);
		pItem->AddValue("/type/N", m_nType);
		if (!m_bFrom.empty())
		{
			pItem->AddValue("/from/B", Base64Encode((char*)&m_bFrom.m_astList[0], m_bFrom.m_udwNum*sizeof(SReportUserInfo), sBase64Encode));
		}
		if (!m_bTo.empty())
		{
			pItem->AddValue("/to/B", Base64Encode((char*)&m_bTo.m_astList[0], m_bTo.m_udwNum*sizeof(SReportUserInfo), sBase64Encode));
		}
		if (!m_sContent.empty())
		{
			pItem->AddValue("/content/S", JsonEncode(m_sContent, sJsonEncode));
		}
		pItem->AddValue("/result/N", m_nResult);
		pItem->AddValue("/sid/N", m_nSid);
		pItem->AddValue("/seq/N", m_nSeq);
	}
	else if (WRITE_ACTION_TYPE_DELETE == dwActionType)
	{
		pItem = pReqItem->GetAwsMap("DeleteRequest")->GetAwsMap("Key");
		pItem->AddValue("/id/N", m_nId);
	}
	else
	{
		assert(0);
	}
	pWriteItem->GetAwsMap("RequestItems")->GetAwsList(GetTableName())->SetValue(pReqItem, true);
}

AwsMap* TbReport::OnReadItemReq(unsigned int udwIdxNo)
{
	oJsonWriter.omitEndingLineFeed();
	IndexDesc& idx_desc = oTableDesc.mIndexDesc[udwIdxNo];
	assert(idx_desc.sName == "PRIMARY"); //ֻ��ͨ��������ѯ
	AwsMap* pReadItem = new AwsMap;
	assert(pReadItem);
	pReadItem->AddValue("/Keys[0]/id/N", m_nId);
	ostringstream oss;
	for (unsigned int i = 0; i < idx_desc.vecRtnFld.size(); ++i)
	{
		oss.str("");
		oss << "/AttributesToGet[" << i << "]";
		FieldDesc& fld_desc = oTableDesc.mFieldDesc[idx_desc.vecRtnFld[i]];
		pReadItem->AddValue(oss.str(),  fld_desc.sName);
	}
	return pReadItem;
}

void TbReport::OnReadItemReq(AwsMap* pReadItem)
{
	assert(pReadItem);
	AwsList* pKeys = pReadItem->GetAwsMap("RequestItems")->GetAwsMap(GetTableName())->GetAwsList("Keys");
	AwsMap* pKey = new AwsMap;
	assert(pKey);
	pKey->AddValue("/id/N", m_nId);
	pKeys->SetValue(pKey, true);
}

int TbReport::OnReadItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbReport::OnDeleteItemReq(
	const ExpectedDesc& expected_desc, int dwReturnValuesType, bool bReturnConsumedCapacity)
{
	oJsonWriter.omitEndingLineFeed();
	AwsMap* pDeleteItem = new AwsMap;
	assert(pDeleteItem);
	pDeleteItem->AddValue("/TableName", GetTableName());
	if (bReturnConsumedCapacity)
	{
		pDeleteItem->AddValue("/ReturnConsumedCapacity", "TOTAL");
	}
	if (dwReturnValuesType > RETURN_VALUES_NONE)
	{
		pDeleteItem->AddValue("/ReturnValues", ReturnValuesType2Str(dwReturnValuesType));
	}
	pDeleteItem->AddValue("/Key/id/N", m_nId);
	ostringstream oss;
	for (unsigned int i = 0; i < expected_desc.vecExpectedItem.size(); ++i)
	{
		const ExpectedItem& item = expected_desc.vecExpectedItem[i];
		FieldDesc& fld_desc = oTableDesc.mFieldDesc[item.udwFldNo];
		oss.str("");
		oss << "/Expected/" << fld_desc.sName << "/Exists";
		pDeleteItem->AddBoolean(oss.str(), item.bExists);
		if (item.bExists) //ExistsΪtrueʱ����ҪValue
		{
			oss.str("");
			oss << "/Expected/" << fld_desc.sName << "/Value/" << DataType2Str(fld_desc.dwType);
			if (FIELD_TYPE_N == fld_desc.dwType)
			{
				pDeleteItem->AddValue(oss.str(), item.nValue);
				continue;
			}
			if (FIELD_TYPE_S == fld_desc.dwType)
			{
				pDeleteItem->AddValue(oss.str(), item.sValue);
				continue;
			}
			//FIELD_TYPE_B�Ͳ���expected
			assert(0);
		}
	}
	return pDeleteItem;
}

int TbReport::OnDeleteItemReq(string& sPostData,
	const ExpectedDesc& expected_desc, int dwReturnValuesType, bool bReturnConsumedCapacity)
{
	++m_nSeq;
	AwsMap* pDeleteItem = OnDeleteItemReq(expected_desc, dwReturnValuesType, bReturnConsumedCapacity);
	ostringstream oss;
	pDeleteItem->Dump(oss);
	sPostData = oss.str();
	delete pDeleteItem;
	return 0;
}

int TbReport::OnDeleteItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbReport::OnGetItemReq(unsigned int udwIdxNo,
	bool bConsistentRead, bool bReturnConsumedCapacity)
{
	oJsonWriter.omitEndingLineFeed();
	assert(oTableDesc.mIndexDesc.find(udwIdxNo)!=oTableDesc.mIndexDesc.end());
	IndexDesc& idx_desc = oTableDesc.mIndexDesc[udwIdxNo];
	assert(idx_desc.sName == "PRIMARY"); //ֻ��ͨ��������ѯ
	AwsMap* pGetItem = new AwsMap;
	assert(pGetItem);
	pGetItem->AddValue("/TableName", GetTableName());
	pGetItem->AddValue("/Key/id/N", m_nId);
	ostringstream oss;
	for (unsigned int i = 0; i < idx_desc.vecRtnFld.size(); ++i)
	{
		oss.str("");
		oss << "/AttributesToGet[" << i << "]";
		FieldDesc& fld_desc = oTableDesc.mFieldDesc[idx_desc.vecRtnFld[i]];
		pGetItem->AddValue(oss.str(),  fld_desc.sName);
	}
	pGetItem->AddBoolean("/ConsistentRead", bConsistentRead);
	if (bReturnConsumedCapacity)
	{
		pGetItem->AddValue("/ReturnConsumedCapacity", "TOTAL");
	}
	return pGetItem;
}

int TbReport::OnGetItemReq(string& sPostData, unsigned int udwIdxNo,
	bool bConsistentRead, bool bReturnConsumedCapacity)
{
	AwsMap* pGetItem = OnGetItemReq(udwIdxNo, bConsistentRead, bReturnConsumedCapacity);
	ostringstream oss;
	pGetItem->Dump(oss);
	sPostData = oss.str();
	delete pGetItem;
	return 0;
}

int TbReport::OnGetItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbReport::OnPutItemReq(
	const ExpectedDesc& expected_desc, int dwReturnValuesType, bool bReturnConsumedCapacity)
{
	oJsonWriter.omitEndingLineFeed();
	AwsMap* pPutItem = new AwsMap;
	assert(pPutItem);
	string sBase64Encode;
	string sJsonEncode;
	pPutItem->AddValue("/TableName", GetTableName());
	if (bReturnConsumedCapacity)
	{
		pPutItem->AddValue("/ReturnConsumedCapacity", "TOTAL");
	}
	if (dwReturnValuesType > RETURN_VALUES_NONE)
	{
		pPutItem->AddValue("/ReturnValues", ReturnValuesType2Str(dwReturnValuesType));
	}
	pPutItem->AddValue("/Item/id/N", m_nId);
	pPutItem->AddValue("/Item/time/N", m_nTime);
	pPutItem->AddValue("/Item/type/N", m_nType);
	if (!m_bFrom.empty())
	{
		pPutItem->AddValue("/Item/from/B", Base64Encode((char*)&m_bFrom.m_astList[0], m_bFrom.m_udwNum*sizeof(SReportUserInfo), sBase64Encode));
	}
	if (!m_bTo.empty())
	{
		pPutItem->AddValue("/Item/to/B", Base64Encode((char*)&m_bTo.m_astList[0], m_bTo.m_udwNum*sizeof(SReportUserInfo), sBase64Encode));
	}
	if (!m_sContent.empty())
	{
		pPutItem->AddValue("/Item/content/S", JsonEncode(m_sContent, sJsonEncode));
	}
	pPutItem->AddValue("/Item/result/N", m_nResult);
	pPutItem->AddValue("/Item/sid/N", m_nSid);
	pPutItem->AddValue("/Item/seq/N", m_nSeq);
	ostringstream oss;
	for (unsigned int i = 0; i < expected_desc.vecExpectedItem.size(); ++i)
	{
		const ExpectedItem& item = expected_desc.vecExpectedItem[i];
		assert(oTableDesc.mFieldDesc.find(item.udwFldNo)!=oTableDesc.mFieldDesc.end());
		FieldDesc& fld_desc = oTableDesc.mFieldDesc[item.udwFldNo];
		oss.str("");
		oss << "/Expected/" << fld_desc.sName << "/Exists";
		pPutItem->AddBoolean(oss.str(), item.bExists);
		if (item.bExists) //ExistsΪtrueʱ����ҪValue
		{
			oss.str("");
			oss << "/Expected/" << fld_desc.sName << "/Value/" << DataType2Str(fld_desc.dwType);
			if (FIELD_TYPE_N == fld_desc.dwType)
			{
				pPutItem->AddValue(oss.str(), item.nValue);
				continue;
			}
			if (FIELD_TYPE_S == fld_desc.dwType)
			{
				pPutItem->AddValue(oss.str(), item.sValue);
				continue;
			}
			//FIELD_TYPE_B�Ͳ���expected
			assert(0);
		}
	}
	return pPutItem;
}

int TbReport::OnPutItemReq(string& sPostData,
	const ExpectedDesc& expected_desc, int dwReturnValuesType, bool bReturnConsumedCapacity)
{
	++m_nSeq;
	AwsMap* pPutItem = OnPutItemReq(expected_desc, dwReturnValuesType, bReturnConsumedCapacity);
	ostringstream oss;
	pPutItem->Dump(oss);
	sPostData = oss.str();
	delete pPutItem;
	return 0;
}

int TbReport::OnPutItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

int TbReport::OnResponse(const Json::Value& item)
{
	oJsonWriter.omitEndingLineFeed();
	int dwResLen = 0;
	dwResLen = 0;
	this->Reset();
	Json::Value::Members vecMembers = item.getMemberNames();
	for (unsigned int i = 0; i < vecMembers.size(); ++i)
	{
 		if (vecMembers[i] == "id")
		{
			m_nId = strtoll(item["id"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "time")
		{
			m_nTime = strtoll(item["time"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "type")
		{
			m_nType = strtoll(item["type"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "from")
		{
			Base64Decode(item["from"]["B"].asString(), (char*)&m_bFrom.m_astList[0], dwResLen);
			m_bFrom.m_udwNum = dwResLen/sizeof(SReportUserInfo);
			if (0==m_bFrom.m_udwNum && dwResLen>0)
			{
				m_bFrom.m_udwNum = 1;
			}
			continue;
		}
		if (vecMembers[i] == "to")
		{
			Base64Decode(item["to"]["B"].asString(), (char*)&m_bTo.m_astList[0], dwResLen);
			m_bTo.m_udwNum = dwResLen/sizeof(SReportUserInfo);
			if (0==m_bTo.m_udwNum && dwResLen>0)
			{
				m_bTo.m_udwNum = 1;
			}
			continue;
		}
		if (vecMembers[i] == "content")
		{
			m_sContent = item["content"]["S"].asString();
			continue;
		}
		if (vecMembers[i] == "result")
		{
			m_nResult = strtoll(item["result"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "sid")
		{
			m_nSid = strtoll(item["sid"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "seq")
		{
			m_nSeq = strtoll(item["seq"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
	}
	return 0;
}

TINT64 TbReport::GetSeq()
{
	return m_nSeq;
}
