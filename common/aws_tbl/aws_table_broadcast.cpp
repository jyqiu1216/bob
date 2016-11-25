#include "aws_table_broadcast.h"

TableDesc TbBroadcast::oTableDesc;

int TbBroadcast::Init(const string& sConfFile, const string strProjectName)
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

AwsTable* TbBroadcast::NewObject()
{
	return new TbBroadcast;
}

string TbBroadcast::GetTableName()
{
	ostringstream oss;
	if(!oTableDesc.m_strProjectName.empty())
	{
	oss << oTableDesc.m_strProjectName << "_";
	}
	oss << "global_";
	oss << oTableDesc.sName;
	return oss.str();
}

TINT32 TbBroadcast::GetTableIdx()
{
	 return 0;
}

AwsMap* TbBroadcast::OnScanReq(unsigned int udwIdxNo, bool bHasStartKey, bool bReturnConsumedCapacity,
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
		pScan->AddValue("/ExclusiveStartKey/key/N",m_nKey);
		pScan->AddValue("/ExclusiveStartKey/ctime/N",m_nCtime);
	}
	return pScan;
}

int TbBroadcast::OnScanReq(string& sPostData, unsigned int udwIdxNo, bool bHasStartKey, bool bReturnConsumedCapacity,
 unsigned int dwLimit, unsigned int dwSegment, unsigned int dwTotalSegments)
{
	AwsMap* pScan = OnScanReq(udwIdxNo, bHasStartKey, bReturnConsumedCapacity, dwLimit, dwSegment, dwTotalSegments);
	ostringstream oss;
	pScan->Dump(oss);
	sPostData = oss.str();
	delete pScan;
	return 0;
}

int TbBroadcast::OnScanRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbBroadcast::OnQueryReq(unsigned int udwIdxNo, const CompareDesc& comp_desc,
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
			if(fld_desc.udwFldNo == TbBROADCAST_FIELD_KEY)
			{
				pQuery->AddValue("/KeyConditions/key/AttributeValueList[0]/N", m_nKey);
				pQuery->AddValue("/KeyConditions/key/ComparisonOperator", "EQ");
			}
		}
		else if (i == 1) //1ֻ����rangekey or local index�������ж��ַ�ʽ
		{
			if (COMPARE_TYPE_EQ<=comp_desc.dwCompareType && comp_desc.dwCompareType<=COMPARE_TYPE_GT)
			{
				if(fld_desc.udwFldNo == TbBROADCAST_FIELD_CTIME)
				{
					pQuery->AddValue("/KeyConditions/ctime/AttributeValueList[0]/N", m_nCtime);
					pQuery->AddValue("/KeyConditions/ctime/ComparisonOperator", CompareType2Str(comp_desc.dwCompareType));
				}
			}
			else if (COMPARE_TYPE_BETWEEN == comp_desc.dwCompareType)
			{
				if(fld_desc.udwFldNo == TbBROADCAST_FIELD_CTIME)
				{
					pQuery->AddValue("/KeyConditions/ctime/AttributeValueList[0]/N" , comp_desc.vecN[0]);
					pQuery->AddValue("/KeyConditions/ctime/AttributeValueList[1]/N" , comp_desc.vecN[1]);
					pQuery->AddValue("/KeyConditions/ctime/ComparisonOperator", "BETWEEN");
				}
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

int TbBroadcast::OnQueryReq(string& sPostData, unsigned int udwIdxNo, const CompareDesc& comp_desc,
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

int TbBroadcast::OnQueryRsp(const Json::Value& item)
{
	return OnResponse(item);
}

int TbBroadcast::OnCountReq(string& sPostData, unsigned int udwIdxNo, const CompareDesc& comp_desc, 
		bool bConsistentRead, bool bReturnConsumedCapacity, bool bScanIndexForward, unsigned int dwLimit)
{
	AwsMap* pQuery = OnQueryReq(udwIdxNo, comp_desc, bConsistentRead, bReturnConsumedCapacity, bScanIndexForward, dwLimit, true);
	ostringstream oss;
	pQuery->Dump(oss);
	sPostData = oss.str();
	delete pQuery;
	return 0;
}

AwsMap* TbBroadcast::OnUpdateItemReq(
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
	pUpdateItem->AddValue("/Key/key/N", m_nKey);
	pUpdateItem->AddValue("/Key/ctime/N", m_nCtime);
	for (map<unsigned int, int>::iterator iter = m_mFlag.begin(); iter != m_mFlag.end(); ++iter)
	{
		bUpdateFlag = true;
		if (TbBROADCAST_FIELD_TYPE == iter->first)
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
		if (TbBROADCAST_FIELD_CID == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/cid/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/cid/Value/N", m_nCid);
				pUpdateItem->AddValue("/AttributeUpdates/cid/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbBROADCAST_FIELD_REPLACE_DATA == iter->first)
		{
			if (!m_sReplace_data.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/replace_data/Value/S", JsonEncode(m_sReplace_data, sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/replace_data/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/replace_data/Action", "DELETE");
			}
			continue;
		}
		if (TbBROADCAST_FIELD_PARAM == iter->first)
		{
			if (!m_sParam.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/param/Value/S", JsonEncode(m_sParam, sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/param/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/param/Action", "DELETE");
			}
			continue;
		}
		if (TbBROADCAST_FIELD_STRATEGY == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/strategy/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/strategy/Value/N", m_nStrategy);
				pUpdateItem->AddValue("/AttributeUpdates/strategy/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbBROADCAST_FIELD_PRIORITY == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/priority/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/priority/Value/N", m_nPriority);
				pUpdateItem->AddValue("/AttributeUpdates/priority/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbBROADCAST_FIELD_SEQ == iter->first)
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

int TbBroadcast::OnUpdateItemReq(string& sPostData,
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

int TbBroadcast::OnUpdateItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbBroadcast::OnWriteItemReq(int dwActionType)
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
		pItem->AddValue("/key/N", m_nKey);
		pItem->AddValue("/ctime/N", m_nCtime);
		pItem->AddValue("/type/N", m_nType);
		pItem->AddValue("/cid/N", m_nCid);
		if (!m_sReplace_data.empty())
		{
			pItem->AddValue("/replace_data/S", JsonEncode(m_sReplace_data, sJsonEncode));
		}
		if (!m_sParam.empty())
		{
			pItem->AddValue("/param/S", JsonEncode(m_sParam, sJsonEncode));
		}
		pItem->AddValue("/strategy/N", m_nStrategy);
		pItem->AddValue("/priority/N", m_nPriority);
		pItem->AddValue("/seq/N", m_nSeq);
	}
	else if (WRITE_ACTION_TYPE_DELETE == dwActionType)
	{
		pItem = pWriteItem->GetAwsMap("DeleteRequest")->GetAwsMap("Key");
		pItem->AddValue("/key/N", m_nKey);
		pItem->AddValue("/ctime/N", m_nCtime);
	}
	else
	{
		assert(0);
	}
	return pWriteItem;
}

void TbBroadcast::OnWriteItemReq(AwsMap* pWriteItem, int dwActionType)
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
		pItem->AddValue("/key/N", m_nKey);
		pItem->AddValue("/ctime/N", m_nCtime);
		pItem->AddValue("/type/N", m_nType);
		pItem->AddValue("/cid/N", m_nCid);
		if (!m_sReplace_data.empty())
		{
			pItem->AddValue("/replace_data/S", JsonEncode(m_sReplace_data, sJsonEncode));
		}
		if (!m_sParam.empty())
		{
			pItem->AddValue("/param/S", JsonEncode(m_sParam, sJsonEncode));
		}
		pItem->AddValue("/strategy/N", m_nStrategy);
		pItem->AddValue("/priority/N", m_nPriority);
		pItem->AddValue("/seq/N", m_nSeq);
	}
	else if (WRITE_ACTION_TYPE_DELETE == dwActionType)
	{
		pItem = pReqItem->GetAwsMap("DeleteRequest")->GetAwsMap("Key");
		pItem->AddValue("/key/N", m_nKey);
		pItem->AddValue("/ctime/N", m_nCtime);
	}
	else
	{
		assert(0);
	}
	pWriteItem->GetAwsMap("RequestItems")->GetAwsList(GetTableName())->SetValue(pReqItem, true);
}

AwsMap* TbBroadcast::OnReadItemReq(unsigned int udwIdxNo)
{
	oJsonWriter.omitEndingLineFeed();
	IndexDesc& idx_desc = oTableDesc.mIndexDesc[udwIdxNo];
	assert(idx_desc.sName == "PRIMARY"); //ֻ��ͨ��������ѯ
	AwsMap* pReadItem = new AwsMap;
	assert(pReadItem);
	pReadItem->AddValue("/Keys[0]/key/N", m_nKey);
	pReadItem->AddValue("/Keys[0]/ctime/N", m_nCtime);
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

void TbBroadcast::OnReadItemReq(AwsMap* pReadItem)
{
	assert(pReadItem);
	AwsList* pKeys = pReadItem->GetAwsMap("RequestItems")->GetAwsMap(GetTableName())->GetAwsList("Keys");
	AwsMap* pKey = new AwsMap;
	assert(pKey);
	pKey->AddValue("/key/N", m_nKey);
	pKey->AddValue("/ctime/N", m_nCtime);
	pKeys->SetValue(pKey, true);
}

int TbBroadcast::OnReadItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbBroadcast::OnDeleteItemReq(
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
	pDeleteItem->AddValue("/Key/key/N", m_nKey);
	pDeleteItem->AddValue("/Key/ctime/N", m_nCtime);
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

int TbBroadcast::OnDeleteItemReq(string& sPostData,
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

int TbBroadcast::OnDeleteItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbBroadcast::OnGetItemReq(unsigned int udwIdxNo,
	bool bConsistentRead, bool bReturnConsumedCapacity)
{
	oJsonWriter.omitEndingLineFeed();
	assert(oTableDesc.mIndexDesc.find(udwIdxNo)!=oTableDesc.mIndexDesc.end());
	IndexDesc& idx_desc = oTableDesc.mIndexDesc[udwIdxNo];
	assert(idx_desc.sName == "PRIMARY"); //ֻ��ͨ��������ѯ
	AwsMap* pGetItem = new AwsMap;
	assert(pGetItem);
	pGetItem->AddValue("/TableName", GetTableName());
	pGetItem->AddValue("/Key/key/N", m_nKey);
	pGetItem->AddValue("/Key/ctime/N", m_nCtime);
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

int TbBroadcast::OnGetItemReq(string& sPostData, unsigned int udwIdxNo,
	bool bConsistentRead, bool bReturnConsumedCapacity)
{
	AwsMap* pGetItem = OnGetItemReq(udwIdxNo, bConsistentRead, bReturnConsumedCapacity);
	ostringstream oss;
	pGetItem->Dump(oss);
	sPostData = oss.str();
	delete pGetItem;
	return 0;
}

int TbBroadcast::OnGetItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbBroadcast::OnPutItemReq(
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
	pPutItem->AddValue("/Item/key/N", m_nKey);
	pPutItem->AddValue("/Item/ctime/N", m_nCtime);
	pPutItem->AddValue("/Item/type/N", m_nType);
	pPutItem->AddValue("/Item/cid/N", m_nCid);
	if (!m_sReplace_data.empty())
	{
		pPutItem->AddValue("/Item/replace_data/S", JsonEncode(m_sReplace_data, sJsonEncode));
	}
	if (!m_sParam.empty())
	{
		pPutItem->AddValue("/Item/param/S", JsonEncode(m_sParam, sJsonEncode));
	}
	pPutItem->AddValue("/Item/strategy/N", m_nStrategy);
	pPutItem->AddValue("/Item/priority/N", m_nPriority);
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

int TbBroadcast::OnPutItemReq(string& sPostData,
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

int TbBroadcast::OnPutItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

int TbBroadcast::OnResponse(const Json::Value& item)
{
	oJsonWriter.omitEndingLineFeed();
	int dwResLen = 0;
	dwResLen = 0;
	this->Reset();
	Json::Value::Members vecMembers = item.getMemberNames();
	for (unsigned int i = 0; i < vecMembers.size(); ++i)
	{
 		if (vecMembers[i] == "key")
		{
			m_nKey = strtoll(item["key"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "ctime")
		{
			m_nCtime = strtoll(item["ctime"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "type")
		{
			m_nType = strtoll(item["type"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "cid")
		{
			m_nCid = strtoll(item["cid"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "replace_data")
		{
			m_sReplace_data = item["replace_data"]["S"].asString();
			continue;
		}
		if (vecMembers[i] == "param")
		{
			m_sParam = item["param"]["S"].asString();
			continue;
		}
		if (vecMembers[i] == "strategy")
		{
			m_nStrategy = strtoll(item["strategy"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "priority")
		{
			m_nPriority = strtoll(item["priority"]["N"].asString().c_str(), NULL, 10);
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

TINT64 TbBroadcast::GetSeq()
{
	return m_nSeq;
}

