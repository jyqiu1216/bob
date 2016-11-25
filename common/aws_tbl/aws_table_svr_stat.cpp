#include "aws_table_svr_stat.h"

TableDesc TbSvr_stat::oTableDesc;

int TbSvr_stat::Init(const string& sConfFile, const string strProjectName)
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

AwsTable* TbSvr_stat::NewObject()
{
	return new TbSvr_stat;
}

string TbSvr_stat::GetTableName()
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

TINT32 TbSvr_stat::GetTableIdx()
{
	 return 0;
}

AwsMap* TbSvr_stat::OnScanReq(unsigned int udwIdxNo, bool bHasStartKey, bool bReturnConsumedCapacity,
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
		pScan->AddValue("/ExclusiveStartKey/sid/N",m_nSid);
	}
	return pScan;
}

int TbSvr_stat::OnScanReq(string& sPostData, unsigned int udwIdxNo, bool bHasStartKey, bool bReturnConsumedCapacity,
 unsigned int dwLimit, unsigned int dwSegment, unsigned int dwTotalSegments)
{
	AwsMap* pScan = OnScanReq(udwIdxNo, bHasStartKey, bReturnConsumedCapacity, dwLimit, dwSegment, dwTotalSegments);
	ostringstream oss;
	pScan->Dump(oss);
	sPostData = oss.str();
	delete pScan;
	return 0;
}

int TbSvr_stat::OnScanRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbSvr_stat::OnQueryReq(unsigned int udwIdxNo, const CompareDesc& comp_desc,
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
			if(fld_desc.udwFldNo == TbSVR_STAT_FIELD_SID)
			{
				pQuery->AddValue("/KeyConditions/sid/AttributeValueList[0]/N", m_nSid);
				pQuery->AddValue("/KeyConditions/sid/ComparisonOperator", "EQ");
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

int TbSvr_stat::OnQueryReq(string& sPostData, unsigned int udwIdxNo, const CompareDesc& comp_desc,
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

int TbSvr_stat::OnQueryRsp(const Json::Value& item)
{
	return OnResponse(item);
}

int TbSvr_stat::OnCountReq(string& sPostData, unsigned int udwIdxNo, const CompareDesc& comp_desc, 
		bool bConsistentRead, bool bReturnConsumedCapacity, bool bScanIndexForward, unsigned int dwLimit)
{
	AwsMap* pQuery = OnQueryReq(udwIdxNo, comp_desc, bConsistentRead, bReturnConsumedCapacity, bScanIndexForward, dwLimit, true);
	ostringstream oss;
	pQuery->Dump(oss);
	sPostData = oss.str();
	delete pQuery;
	return 0;
}

AwsMap* TbSvr_stat::OnUpdateItemReq(
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
	pUpdateItem->AddValue("/Key/sid/N", m_nSid);
	for (map<unsigned int, int>::iterator iter = m_mFlag.begin(); iter != m_mFlag.end(); ++iter)
	{
		bUpdateFlag = true;
		if (TbSVR_STAT_FIELD_NAME == iter->first)
		{
			if (!m_sName.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/name/Value/S", JsonEncode(m_sName, sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/name/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/name/Action", "DELETE");
			}
			continue;
		}
		if (TbSVR_STAT_FIELD_LANGUAGE == iter->first)
		{
			if (!m_sLanguage.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/language/Value/S", JsonEncode(m_sLanguage, sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/language/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/language/Action", "DELETE");
			}
			continue;
		}
		if (TbSVR_STAT_FIELD_OPEN_TIME == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/open_time/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/open_time/Value/N", m_nOpen_time);
				pUpdateItem->AddValue("/AttributeUpdates/open_time/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbSVR_STAT_FIELD_STATUS == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/status/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/status/Value/N", m_nStatus);
				pUpdateItem->AddValue("/AttributeUpdates/status/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbSVR_STAT_FIELD_PLAIN_NUM == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/plain_num/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/plain_num/Value/N", m_nPlain_num);
				pUpdateItem->AddValue("/AttributeUpdates/plain_num/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbSVR_STAT_FIELD_CITY_NUM == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/city_num/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/city_num/Value/N", m_nCity_num);
				pUpdateItem->AddValue("/AttributeUpdates/city_num/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbSVR_STAT_FIELD_THRESHOLD == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/threshold/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/threshold/Value/N", m_nThreshold);
				pUpdateItem->AddValue("/AttributeUpdates/threshold/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbSVR_STAT_FIELD_SWITCH == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/switch/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/switch/Value/N", m_nSwitch);
				pUpdateItem->AddValue("/AttributeUpdates/switch/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbSVR_STAT_FIELD_SCALE == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/scale/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/scale/Value/N", m_nScale);
				pUpdateItem->AddValue("/AttributeUpdates/scale/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbSVR_STAT_FIELD_POS == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/pos/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/pos/Value/N", m_nPos);
				pUpdateItem->AddValue("/AttributeUpdates/pos/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbSVR_STAT_FIELD_AVATAR == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/avatar/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/avatar/Value/N", m_nAvatar);
				pUpdateItem->AddValue("/AttributeUpdates/avatar/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbSVR_STAT_FIELD_BLOCK_SIZE == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/block_size/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/block_size/Value/N", m_nBlock_size);
				pUpdateItem->AddValue("/AttributeUpdates/block_size/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbSVR_STAT_FIELD_SHOW_FLAG == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/show_flag/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/show_flag/Value/N", m_nShow_flag);
				pUpdateItem->AddValue("/AttributeUpdates/show_flag/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbSVR_STAT_FIELD_DRAGON_FLAG == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/dragon_flag/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/dragon_flag/Value/N", m_nDragon_flag);
				pUpdateItem->AddValue("/AttributeUpdates/dragon_flag/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbSVR_STAT_FIELD_SEQ == iter->first)
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

int TbSvr_stat::OnUpdateItemReq(string& sPostData,
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

int TbSvr_stat::OnUpdateItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbSvr_stat::OnWriteItemReq(int dwActionType)
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
		pItem->AddValue("/sid/N", m_nSid);
		if (!m_sName.empty())
		{
			pItem->AddValue("/name/S", JsonEncode(m_sName, sJsonEncode));
		}
		if (!m_sLanguage.empty())
		{
			pItem->AddValue("/language/S", JsonEncode(m_sLanguage, sJsonEncode));
		}
		pItem->AddValue("/open_time/N", m_nOpen_time);
		pItem->AddValue("/status/N", m_nStatus);
		pItem->AddValue("/plain_num/N", m_nPlain_num);
		pItem->AddValue("/city_num/N", m_nCity_num);
		pItem->AddValue("/threshold/N", m_nThreshold);
		pItem->AddValue("/switch/N", m_nSwitch);
		pItem->AddValue("/scale/N", m_nScale);
		pItem->AddValue("/pos/N", m_nPos);
		pItem->AddValue("/avatar/N", m_nAvatar);
		pItem->AddValue("/block_size/N", m_nBlock_size);
		pItem->AddValue("/show_flag/N", m_nShow_flag);
		pItem->AddValue("/dragon_flag/N", m_nDragon_flag);
		pItem->AddValue("/seq/N", m_nSeq);
	}
	else if (WRITE_ACTION_TYPE_DELETE == dwActionType)
	{
		pItem = pWriteItem->GetAwsMap("DeleteRequest")->GetAwsMap("Key");
		pItem->AddValue("/sid/N", m_nSid);
	}
	else
	{
		assert(0);
	}
	return pWriteItem;
}

void TbSvr_stat::OnWriteItemReq(AwsMap* pWriteItem, int dwActionType)
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
		pItem->AddValue("/sid/N", m_nSid);
		if (!m_sName.empty())
		{
			pItem->AddValue("/name/S", JsonEncode(m_sName, sJsonEncode));
		}
		if (!m_sLanguage.empty())
		{
			pItem->AddValue("/language/S", JsonEncode(m_sLanguage, sJsonEncode));
		}
		pItem->AddValue("/open_time/N", m_nOpen_time);
		pItem->AddValue("/status/N", m_nStatus);
		pItem->AddValue("/plain_num/N", m_nPlain_num);
		pItem->AddValue("/city_num/N", m_nCity_num);
		pItem->AddValue("/threshold/N", m_nThreshold);
		pItem->AddValue("/switch/N", m_nSwitch);
		pItem->AddValue("/scale/N", m_nScale);
		pItem->AddValue("/pos/N", m_nPos);
		pItem->AddValue("/avatar/N", m_nAvatar);
		pItem->AddValue("/block_size/N", m_nBlock_size);
		pItem->AddValue("/show_flag/N", m_nShow_flag);
		pItem->AddValue("/dragon_flag/N", m_nDragon_flag);
		pItem->AddValue("/seq/N", m_nSeq);
	}
	else if (WRITE_ACTION_TYPE_DELETE == dwActionType)
	{
		pItem = pReqItem->GetAwsMap("DeleteRequest")->GetAwsMap("Key");
		pItem->AddValue("/sid/N", m_nSid);
	}
	else
	{
		assert(0);
	}
	pWriteItem->GetAwsMap("RequestItems")->GetAwsList(GetTableName())->SetValue(pReqItem, true);
}

AwsMap* TbSvr_stat::OnReadItemReq(unsigned int udwIdxNo)
{
	oJsonWriter.omitEndingLineFeed();
	IndexDesc& idx_desc = oTableDesc.mIndexDesc[udwIdxNo];
	assert(idx_desc.sName == "PRIMARY"); //只能通过主键查询
	AwsMap* pReadItem = new AwsMap;
	assert(pReadItem);
	pReadItem->AddValue("/Keys[0]/sid/N", m_nSid);
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

void TbSvr_stat::OnReadItemReq(AwsMap* pReadItem)
{
	assert(pReadItem);
	AwsList* pKeys = pReadItem->GetAwsMap("RequestItems")->GetAwsMap(GetTableName())->GetAwsList("Keys");
	AwsMap* pKey = new AwsMap;
	assert(pKey);
	pKey->AddValue("/sid/N", m_nSid);
	pKeys->SetValue(pKey, true);
}

int TbSvr_stat::OnReadItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbSvr_stat::OnDeleteItemReq(
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
	pDeleteItem->AddValue("/Key/sid/N", m_nSid);
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

int TbSvr_stat::OnDeleteItemReq(string& sPostData,
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

int TbSvr_stat::OnDeleteItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbSvr_stat::OnGetItemReq(unsigned int udwIdxNo,
	bool bConsistentRead, bool bReturnConsumedCapacity)
{
	oJsonWriter.omitEndingLineFeed();
	assert(oTableDesc.mIndexDesc.find(udwIdxNo)!=oTableDesc.mIndexDesc.end());
	IndexDesc& idx_desc = oTableDesc.mIndexDesc[udwIdxNo];
	assert(idx_desc.sName == "PRIMARY"); //只能通过主键查询
	AwsMap* pGetItem = new AwsMap;
	assert(pGetItem);
	pGetItem->AddValue("/TableName", GetTableName());
	pGetItem->AddValue("/Key/sid/N", m_nSid);
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

int TbSvr_stat::OnGetItemReq(string& sPostData, unsigned int udwIdxNo,
	bool bConsistentRead, bool bReturnConsumedCapacity)
{
	AwsMap* pGetItem = OnGetItemReq(udwIdxNo, bConsistentRead, bReturnConsumedCapacity);
	ostringstream oss;
	pGetItem->Dump(oss);
	sPostData = oss.str();
	delete pGetItem;
	return 0;
}

int TbSvr_stat::OnGetItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbSvr_stat::OnPutItemReq(
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
	pPutItem->AddValue("/Item/sid/N", m_nSid);
	if (!m_sName.empty())
	{
		pPutItem->AddValue("/Item/name/S", JsonEncode(m_sName, sJsonEncode));
	}
	if (!m_sLanguage.empty())
	{
		pPutItem->AddValue("/Item/language/S", JsonEncode(m_sLanguage, sJsonEncode));
	}
	pPutItem->AddValue("/Item/open_time/N", m_nOpen_time);
	pPutItem->AddValue("/Item/status/N", m_nStatus);
	pPutItem->AddValue("/Item/plain_num/N", m_nPlain_num);
	pPutItem->AddValue("/Item/city_num/N", m_nCity_num);
	pPutItem->AddValue("/Item/threshold/N", m_nThreshold);
	pPutItem->AddValue("/Item/switch/N", m_nSwitch);
	pPutItem->AddValue("/Item/scale/N", m_nScale);
	pPutItem->AddValue("/Item/pos/N", m_nPos);
	pPutItem->AddValue("/Item/avatar/N", m_nAvatar);
	pPutItem->AddValue("/Item/block_size/N", m_nBlock_size);
	pPutItem->AddValue("/Item/show_flag/N", m_nShow_flag);
	pPutItem->AddValue("/Item/dragon_flag/N", m_nDragon_flag);
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

int TbSvr_stat::OnPutItemReq(string& sPostData,
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

int TbSvr_stat::OnPutItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

int TbSvr_stat::OnResponse(const Json::Value& item)
{
	oJsonWriter.omitEndingLineFeed();
	int dwResLen = 0;
	dwResLen = 0;
	this->Reset();
	Json::Value::Members vecMembers = item.getMemberNames();
	for (unsigned int i = 0; i < vecMembers.size(); ++i)
	{
 		if (vecMembers[i] == "sid")
		{
			m_nSid = strtoll(item["sid"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "name")
		{
			m_sName = item["name"]["S"].asString();
			continue;
		}
		if (vecMembers[i] == "language")
		{
			m_sLanguage = item["language"]["S"].asString();
			continue;
		}
		if (vecMembers[i] == "open_time")
		{
			m_nOpen_time = strtoll(item["open_time"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "status")
		{
			m_nStatus = strtoll(item["status"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "plain_num")
		{
			m_nPlain_num = strtoll(item["plain_num"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "city_num")
		{
			m_nCity_num = strtoll(item["city_num"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "threshold")
		{
			m_nThreshold = strtoll(item["threshold"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "switch")
		{
			m_nSwitch = strtoll(item["switch"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "scale")
		{
			m_nScale = strtoll(item["scale"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "pos")
		{
			m_nPos = strtoll(item["pos"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "avatar")
		{
			m_nAvatar = strtoll(item["avatar"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "block_size")
		{
			m_nBlock_size = strtoll(item["block_size"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "show_flag")
		{
			m_nShow_flag = strtoll(item["show_flag"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "dragon_flag")
		{
			m_nDragon_flag = strtoll(item["dragon_flag"]["N"].asString().c_str(), NULL, 10);
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

TINT64 TbSvr_stat::GetSeq()
{
	return m_nSeq;
}

