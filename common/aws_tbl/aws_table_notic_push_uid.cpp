#include "aws_table_notic_push_uid.h"

TableDesc TbNotic_push_uid::oTableDesc;

int TbNotic_push_uid::Init(const string& sConfFile, const string strProjectName)
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

AwsTable* TbNotic_push_uid::NewObject()
{
	return new TbNotic_push_uid;
}

string TbNotic_push_uid::GetTableName()
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

TINT32 TbNotic_push_uid::GetTableIdx()
{
	 return 0;
}

AwsMap* TbNotic_push_uid::OnScanReq(unsigned int udwIdxNo, bool bHasStartKey, bool bReturnConsumedCapacity,
	 unsigned int dwLimit, unsigned int dwSegment, unsigned int dwTotalSegments)
{
	oJsonWriter.omitEndingLineFeed();
	assert(oTableDesc.mIndexDesc.find(udwIdxNo)!=oTableDesc.mIndexDesc.end());
	IndexDesc& idx_desc = oTableDesc.mIndexDesc[udwIdxNo];
	assert(idx_desc.sName == "PRIMARY"); //只能通过主键查询
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
		pScan->AddValue("/ExclusiveStartKey/uid/N",m_nUid);
	}
	return pScan;
}

int TbNotic_push_uid::OnScanReq(string& sPostData, unsigned int udwIdxNo, bool bHasStartKey, bool bReturnConsumedCapacity,
 unsigned int dwLimit, unsigned int dwSegment, unsigned int dwTotalSegments)
{
	AwsMap* pScan = OnScanReq(udwIdxNo, bHasStartKey, bReturnConsumedCapacity, dwLimit, dwSegment, dwTotalSegments);
	ostringstream oss;
	pScan->Dump(oss);
	sPostData = oss.str();
	delete pScan;
	return 0;
}

int TbNotic_push_uid::OnScanRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbNotic_push_uid::OnQueryReq(unsigned int udwIdxNo, const CompareDesc& comp_desc,
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
		if (i == 0) //0只能是hash key，HASH KEY只能是EQ方式
		{
			if(fld_desc.udwFldNo == TbNOTIC_PUSH_UID_FIELD_UID)
			{
				pQuery->AddValue("/KeyConditions/uid/AttributeValueList[0]/N", m_nUid);
				pQuery->AddValue("/KeyConditions/uid/ComparisonOperator", "EQ");
			}
		}
		else if (i == 1) //1只能是rangekey or local index，可以有多种方式
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

int TbNotic_push_uid::OnQueryReq(string& sPostData, unsigned int udwIdxNo, const CompareDesc& comp_desc,
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

int TbNotic_push_uid::OnQueryRsp(const Json::Value& item)
{
	return OnResponse(item);
}

int TbNotic_push_uid::OnCountReq(string& sPostData, unsigned int udwIdxNo, const CompareDesc& comp_desc, 
		bool bConsistentRead, bool bReturnConsumedCapacity, bool bScanIndexForward, unsigned int dwLimit)
{
	AwsMap* pQuery = OnQueryReq(udwIdxNo, comp_desc, bConsistentRead, bReturnConsumedCapacity, bScanIndexForward, dwLimit, true);
	ostringstream oss;
	pQuery->Dump(oss);
	sPostData = oss.str();
	delete pQuery;
	return 0;
}

AwsMap* TbNotic_push_uid::OnUpdateItemReq(
	const ExpectedDesc& expected_desc, int dwReturnValuesType, bool bReturnConsumedCapacity)
{
	oJsonWriter.omitEndingLineFeed();
	if (m_mFlag.size() == 0) //没有变化
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
	pUpdateItem->AddValue("/Key/uid/N", m_nUid);
	for (map<unsigned int, int>::iterator iter = m_mFlag.begin(); iter != m_mFlag.end(); ++iter)
	{
		bUpdateFlag = true;
		if (TbNOTIC_PUSH_UID_FIELD_SEQ == iter->first)
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
		if (TbNOTIC_PUSH_UID_FIELD_DID == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/did/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/did/Value/N", m_nDid);
				pUpdateItem->AddValue("/AttributeUpdates/did/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbNOTIC_PUSH_UID_FIELD_DE == iter->first)
		{
			if (!m_sDe.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/de/Value/S", JsonEncode(m_sDe, sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/de/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/de/Action", "DELETE");
			}
			continue;
		}
		if (TbNOTIC_PUSH_UID_FIELD_APNS_TOKEN == iter->first)
		{
			if (!m_sApns_token.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/apns_token/Value/S", JsonEncode(m_sApns_token, sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/apns_token/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/apns_token/Action", "DELETE");
			}
			continue;
		}
		if (TbNOTIC_PUSH_UID_FIELD_APNS_NUM == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/apns_num/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/apns_num/Value/N", m_nApns_num);
				pUpdateItem->AddValue("/AttributeUpdates/apns_num/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbNOTIC_PUSH_UID_FIELD_UTIME == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/utime/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/utime/Value/N", m_nUtime);
				pUpdateItem->AddValue("/AttributeUpdates/utime/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbNOTIC_PUSH_UID_FIELD_IDFA == iter->first)
		{
			if (!m_sIdfa.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/idfa/Value/S", JsonEncode(m_sIdfa, sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/idfa/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/idfa/Action", "DELETE");
			}
			continue;
		}
		if (TbNOTIC_PUSH_UID_FIELD_PLATFORM == iter->first)
		{
			if (!m_sPlatform.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/platform/Value/S", JsonEncode(m_sPlatform, sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/platform/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/platform/Action", "DELETE");
			}
			continue;
		}
		if (TbNOTIC_PUSH_UID_FIELD_LANG == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/lang/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/lang/Value/N", m_nLang);
				pUpdateItem->AddValue("/AttributeUpdates/lang/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbNOTIC_PUSH_UID_FIELD_APNS_SWITCH == iter->first)
		{
			if (!m_jApns_switch.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/apns_switch/Value/S", JsonEncode(oJsonWriter.write(m_jApns_switch), sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/apns_switch/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/apns_switch/Action", "DELETE");
			}
			continue;
		}
		if (TbNOTIC_PUSH_UID_FIELD_ARN == iter->first)
		{
			if (!m_sArn.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/arn/Value/S", JsonEncode(m_sArn, sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/arn/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/arn/Action", "DELETE");
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
		if (item.bExists) //Exists为true时才需要Value
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
			//FIELD_TYPE_B型不能expected
			assert(0);
		}
	}
	return pUpdateItem;
}

int TbNotic_push_uid::OnUpdateItemReq(string& sPostData,
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

int TbNotic_push_uid::OnUpdateItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbNotic_push_uid::OnWriteItemReq(int dwActionType)
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
		pItem->AddValue("/uid/N", m_nUid);
		pItem->AddValue("/seq/N", m_nSeq);
		pItem->AddValue("/did/N", m_nDid);
		if (!m_sDe.empty())
		{
			pItem->AddValue("/de/S", JsonEncode(m_sDe, sJsonEncode));
		}
		if (!m_sApns_token.empty())
		{
			pItem->AddValue("/apns_token/S", JsonEncode(m_sApns_token, sJsonEncode));
		}
		pItem->AddValue("/apns_num/N", m_nApns_num);
		pItem->AddValue("/utime/N", m_nUtime);
		if (!m_sIdfa.empty())
		{
			pItem->AddValue("/idfa/S", JsonEncode(m_sIdfa, sJsonEncode));
		}
		if (!m_sPlatform.empty())
		{
			pItem->AddValue("/platform/S", JsonEncode(m_sPlatform, sJsonEncode));
		}
		pItem->AddValue("/lang/N", m_nLang);
		if (!m_jApns_switch.empty())
		{
				pItem->AddValue("/AttributeUpdates/apns_switch/Value/S", JsonEncode(oJsonWriter.write(m_jApns_switch), sJsonEncode));
		}
		if (!m_sArn.empty())
		{
			pItem->AddValue("/arn/S", JsonEncode(m_sArn, sJsonEncode));
		}
	}
	else if (WRITE_ACTION_TYPE_DELETE == dwActionType)
	{
		pItem = pWriteItem->GetAwsMap("DeleteRequest")->GetAwsMap("Key");
		pItem->AddValue("/uid/N", m_nUid);
	}
	else
	{
		assert(0);
	}
	return pWriteItem;
}

void TbNotic_push_uid::OnWriteItemReq(AwsMap* pWriteItem, int dwActionType)
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
		pItem->AddValue("/uid/N", m_nUid);
		pItem->AddValue("/seq/N", m_nSeq);
		pItem->AddValue("/did/N", m_nDid);
		if (!m_sDe.empty())
		{
			pItem->AddValue("/de/S", JsonEncode(m_sDe, sJsonEncode));
		}
		if (!m_sApns_token.empty())
		{
			pItem->AddValue("/apns_token/S", JsonEncode(m_sApns_token, sJsonEncode));
		}
		pItem->AddValue("/apns_num/N", m_nApns_num);
		pItem->AddValue("/utime/N", m_nUtime);
		if (!m_sIdfa.empty())
		{
			pItem->AddValue("/idfa/S", JsonEncode(m_sIdfa, sJsonEncode));
		}
		if (!m_sPlatform.empty())
		{
			pItem->AddValue("/platform/S", JsonEncode(m_sPlatform, sJsonEncode));
		}
		pItem->AddValue("/lang/N", m_nLang);
		if (!m_jApns_switch.empty())
		{
				pItem->AddValue("/AttributeUpdates/apns_switch/Value/S", JsonEncode(oJsonWriter.write(m_jApns_switch), sJsonEncode));
		}
		if (!m_sArn.empty())
		{
			pItem->AddValue("/arn/S", JsonEncode(m_sArn, sJsonEncode));
		}
	}
	else if (WRITE_ACTION_TYPE_DELETE == dwActionType)
	{
		pItem = pReqItem->GetAwsMap("DeleteRequest")->GetAwsMap("Key");
		pItem->AddValue("/uid/N", m_nUid);
	}
	else
	{
		assert(0);
	}
	pWriteItem->GetAwsMap("RequestItems")->GetAwsList(GetTableName())->SetValue(pReqItem, true);
}

AwsMap* TbNotic_push_uid::OnReadItemReq(unsigned int udwIdxNo)
{
	oJsonWriter.omitEndingLineFeed();
	IndexDesc& idx_desc = oTableDesc.mIndexDesc[udwIdxNo];
	assert(idx_desc.sName == "PRIMARY"); //只能通过主键查询
	AwsMap* pReadItem = new AwsMap;
	assert(pReadItem);
	pReadItem->AddValue("/Keys[0]/uid/N", m_nUid);
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

void TbNotic_push_uid::OnReadItemReq(AwsMap* pReadItem)
{
	assert(pReadItem);
	AwsList* pKeys = pReadItem->GetAwsMap("RequestItems")->GetAwsMap(GetTableName())->GetAwsList("Keys");
	AwsMap* pKey = new AwsMap;
	assert(pKey);
	pKey->AddValue("/uid/N", m_nUid);
	pKeys->SetValue(pKey, true);
}

int TbNotic_push_uid::OnReadItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbNotic_push_uid::OnDeleteItemReq(
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
	pDeleteItem->AddValue("/Key/uid/N", m_nUid);
	ostringstream oss;
	for (unsigned int i = 0; i < expected_desc.vecExpectedItem.size(); ++i)
	{
		const ExpectedItem& item = expected_desc.vecExpectedItem[i];
		FieldDesc& fld_desc = oTableDesc.mFieldDesc[item.udwFldNo];
		oss.str("");
		oss << "/Expected/" << fld_desc.sName << "/Exists";
		pDeleteItem->AddBoolean(oss.str(), item.bExists);
		if (item.bExists) //Exists为true时才需要Value
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
			//FIELD_TYPE_B型不能expected
			assert(0);
		}
	}
	return pDeleteItem;
}

int TbNotic_push_uid::OnDeleteItemReq(string& sPostData,
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

int TbNotic_push_uid::OnDeleteItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbNotic_push_uid::OnGetItemReq(unsigned int udwIdxNo,
	bool bConsistentRead, bool bReturnConsumedCapacity)
{
	oJsonWriter.omitEndingLineFeed();
	assert(oTableDesc.mIndexDesc.find(udwIdxNo)!=oTableDesc.mIndexDesc.end());
	IndexDesc& idx_desc = oTableDesc.mIndexDesc[udwIdxNo];
	assert(idx_desc.sName == "PRIMARY"); //只能通过主键查询
	AwsMap* pGetItem = new AwsMap;
	assert(pGetItem);
	pGetItem->AddValue("/TableName", GetTableName());
	pGetItem->AddValue("/Key/uid/N", m_nUid);
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

int TbNotic_push_uid::OnGetItemReq(string& sPostData, unsigned int udwIdxNo,
	bool bConsistentRead, bool bReturnConsumedCapacity)
{
	AwsMap* pGetItem = OnGetItemReq(udwIdxNo, bConsistentRead, bReturnConsumedCapacity);
	ostringstream oss;
	pGetItem->Dump(oss);
	sPostData = oss.str();
	delete pGetItem;
	return 0;
}

int TbNotic_push_uid::OnGetItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbNotic_push_uid::OnPutItemReq(
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
	pPutItem->AddValue("/Item/uid/N", m_nUid);
	pPutItem->AddValue("/Item/seq/N", m_nSeq);
	pPutItem->AddValue("/Item/did/N", m_nDid);
	if (!m_sDe.empty())
	{
		pPutItem->AddValue("/Item/de/S", JsonEncode(m_sDe, sJsonEncode));
	}
	if (!m_sApns_token.empty())
	{
		pPutItem->AddValue("/Item/apns_token/S", JsonEncode(m_sApns_token, sJsonEncode));
	}
	pPutItem->AddValue("/Item/apns_num/N", m_nApns_num);
	pPutItem->AddValue("/Item/utime/N", m_nUtime);
	if (!m_sIdfa.empty())
	{
		pPutItem->AddValue("/Item/idfa/S", JsonEncode(m_sIdfa, sJsonEncode));
	}
	if (!m_sPlatform.empty())
	{
		pPutItem->AddValue("/Item/platform/S", JsonEncode(m_sPlatform, sJsonEncode));
	}
	pPutItem->AddValue("/Item/lang/N", m_nLang);
	if (!m_jApns_switch.empty())
	{
				pPutItem->AddValue("/Item/apns_switch/S", JsonEncode(oJsonWriter.write(m_jApns_switch), sJsonEncode));
	}
	if (!m_sArn.empty())
	{
		pPutItem->AddValue("/Item/arn/S", JsonEncode(m_sArn, sJsonEncode));
	}
	ostringstream oss;
	for (unsigned int i = 0; i < expected_desc.vecExpectedItem.size(); ++i)
	{
		const ExpectedItem& item = expected_desc.vecExpectedItem[i];
		assert(oTableDesc.mFieldDesc.find(item.udwFldNo)!=oTableDesc.mFieldDesc.end());
		FieldDesc& fld_desc = oTableDesc.mFieldDesc[item.udwFldNo];
		oss.str("");
		oss << "/Expected/" << fld_desc.sName << "/Exists";
		pPutItem->AddBoolean(oss.str(), item.bExists);
		if (item.bExists) //Exists为true时才需要Value
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
			//FIELD_TYPE_B型不能expected
			assert(0);
		}
	}
	return pPutItem;
}

int TbNotic_push_uid::OnPutItemReq(string& sPostData,
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

int TbNotic_push_uid::OnPutItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

int TbNotic_push_uid::OnResponse(const Json::Value& item)
{
	oJsonWriter.omitEndingLineFeed();
	int dwResLen = 0;
	dwResLen = 0;
	this->Reset();
	Json::Value::Members vecMembers = item.getMemberNames();
	for (unsigned int i = 0; i < vecMembers.size(); ++i)
	{
 		if (vecMembers[i] == "uid")
		{
			m_nUid = strtoll(item["uid"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "seq")
		{
			m_nSeq = strtoll(item["seq"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "did")
		{
			m_nDid = strtoll(item["did"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "de")
		{
			m_sDe = item["de"]["S"].asString();
			continue;
		}
		if (vecMembers[i] == "apns_token")
		{
			m_sApns_token = item["apns_token"]["S"].asString();
			continue;
		}
		if (vecMembers[i] == "apns_num")
		{
			m_nApns_num = strtoll(item["apns_num"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "utime")
		{
			m_nUtime = strtoll(item["utime"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "idfa")
		{
			m_sIdfa = item["idfa"]["S"].asString();
			continue;
		}
		if (vecMembers[i] == "platform")
		{
			m_sPlatform = item["platform"]["S"].asString();
			continue;
		}
		if (vecMembers[i] == "lang")
		{
			m_nLang = strtoll(item["lang"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "apns_switch")
		{
			if (FALSE == oJsonReader.parse(item["apns_switch"]["S"].asString(),m_jApns_switch))
			{
				m_jApns_switch = Json::Value(Json::nullValue);
			}
			continue;
		}
		if (vecMembers[i] == "arn")
		{
			m_sArn = item["arn"]["S"].asString();
			continue;
		}
	}
	return 0;
}

TINT64 TbNotic_push_uid::GetSeq()
{
	return m_nSeq;
}

