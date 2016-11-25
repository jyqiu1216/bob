#include "aws_table_backpack.h"

TableDesc TbBackpack::oTableDesc;

int TbBackpack::Init(const string& sConfFile, const string strProjectName)
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

AwsTable* TbBackpack::NewObject()
{
	return new TbBackpack;
}

string TbBackpack::GetTableName()
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

TINT32 TbBackpack::GetTableIdx()
{
	 return 0;
}

AwsMap* TbBackpack::OnScanReq(unsigned int udwIdxNo, bool bHasStartKey, bool bReturnConsumedCapacity,
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

int TbBackpack::OnScanReq(string& sPostData, unsigned int udwIdxNo, bool bHasStartKey, bool bReturnConsumedCapacity,
 unsigned int dwLimit, unsigned int dwSegment, unsigned int dwTotalSegments)
{
	AwsMap* pScan = OnScanReq(udwIdxNo, bHasStartKey, bReturnConsumedCapacity, dwLimit, dwSegment, dwTotalSegments);
	ostringstream oss;
	pScan->Dump(oss);
	sPostData = oss.str();
	delete pScan;
	return 0;
}

int TbBackpack::OnScanRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbBackpack::OnQueryReq(unsigned int udwIdxNo, const CompareDesc& comp_desc,
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
			if(fld_desc.udwFldNo == TbBACKPACK_FIELD_UID)
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

int TbBackpack::OnQueryReq(string& sPostData, unsigned int udwIdxNo, const CompareDesc& comp_desc,
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

int TbBackpack::OnQueryRsp(const Json::Value& item)
{
	return OnResponse(item);
}

int TbBackpack::OnCountReq(string& sPostData, unsigned int udwIdxNo, const CompareDesc& comp_desc, 
		bool bConsistentRead, bool bReturnConsumedCapacity, bool bScanIndexForward, unsigned int dwLimit)
{
	AwsMap* pQuery = OnQueryReq(udwIdxNo, comp_desc, bConsistentRead, bReturnConsumedCapacity, bScanIndexForward, dwLimit, true);
	ostringstream oss;
	pQuery->Dump(oss);
	sPostData = oss.str();
	delete pQuery;
	return 0;
}

AwsMap* TbBackpack::OnUpdateItemReq(
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
		if (TbBACKPACK_FIELD_ITEM == iter->first)
		{
			if (!m_bItem.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/item/Value/B", Base64Encode((char*)&m_bItem.m_astList[0], m_bItem.m_udwNum*sizeof(SPlayerItem), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/item/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/item/Action", "DELETE");
			}
			continue;
		}
		if (TbBACKPACK_FIELD_CRYSTAL == iter->first)
		{
			if (!m_jCrystal.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/crystal/Value/S", JsonEncode(oJsonWriter.write(m_jCrystal), sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/crystal/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/crystal/Action", "DELETE");
			}
			continue;
		}
		if (TbBACKPACK_FIELD_MATERIAL == iter->first)
		{
			if (!m_jMaterial.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/material/Value/S", JsonEncode(oJsonWriter.write(m_jMaterial), sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/material/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/material/Action", "DELETE");
			}
			continue;
		}
		if (TbBACKPACK_FIELD_SOUL == iter->first)
		{
			if (!m_jSoul.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/soul/Value/S", JsonEncode(oJsonWriter.write(m_jSoul), sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/soul/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/soul/Action", "DELETE");
			}
			continue;
		}
		if (TbBACKPACK_FIELD_PARTS == iter->first)
		{
			if (!m_jParts.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/parts/Value/S", JsonEncode(oJsonWriter.write(m_jParts), sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/parts/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/parts/Action", "DELETE");
			}
			continue;
		}
		if (TbBACKPACK_FIELD_SP_CRYSTAL == iter->first)
		{
			if (!m_jSp_crystal.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/sp_crystal/Value/S", JsonEncode(oJsonWriter.write(m_jSp_crystal), sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/sp_crystal/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/sp_crystal/Action", "DELETE");
			}
			continue;
		}
		if (TbBACKPACK_FIELD_SCROLL == iter->first)
		{
			if (!m_jScroll.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/scroll/Value/S", JsonEncode(oJsonWriter.write(m_jScroll), sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/scroll/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/scroll/Action", "DELETE");
			}
			continue;
		}
		if (TbBACKPACK_FIELD_SCROLL_GET_TIME == iter->first)
		{
			if (!m_jScroll_get_time.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/scroll_get_time/Value/S", JsonEncode(oJsonWriter.write(m_jScroll_get_time), sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/scroll_get_time/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/scroll_get_time/Action", "DELETE");
			}
			continue;
		}
		if (TbBACKPACK_FIELD_SEQ == iter->first)
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

int TbBackpack::OnUpdateItemReq(string& sPostData,
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

int TbBackpack::OnUpdateItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbBackpack::OnWriteItemReq(int dwActionType)
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
		if (!m_bItem.empty())
		{
			pItem->AddValue("/item/B", Base64Encode((char*)&m_bItem.m_astList[0], m_bItem.m_udwNum*sizeof(SPlayerItem), sBase64Encode));
		}
		if (!m_jCrystal.empty())
		{
				pItem->AddValue("/AttributeUpdates/crystal/Value/S", JsonEncode(oJsonWriter.write(m_jCrystal), sJsonEncode));
		}
		if (!m_jMaterial.empty())
		{
				pItem->AddValue("/AttributeUpdates/material/Value/S", JsonEncode(oJsonWriter.write(m_jMaterial), sJsonEncode));
		}
		if (!m_jSoul.empty())
		{
				pItem->AddValue("/AttributeUpdates/soul/Value/S", JsonEncode(oJsonWriter.write(m_jSoul), sJsonEncode));
		}
		if (!m_jParts.empty())
		{
				pItem->AddValue("/AttributeUpdates/parts/Value/S", JsonEncode(oJsonWriter.write(m_jParts), sJsonEncode));
		}
		if (!m_jSp_crystal.empty())
		{
				pItem->AddValue("/AttributeUpdates/sp_crystal/Value/S", JsonEncode(oJsonWriter.write(m_jSp_crystal), sJsonEncode));
		}
		if (!m_jScroll.empty())
		{
				pItem->AddValue("/AttributeUpdates/scroll/Value/S", JsonEncode(oJsonWriter.write(m_jScroll), sJsonEncode));
		}
		if (!m_jScroll_get_time.empty())
		{
				pItem->AddValue("/AttributeUpdates/scroll_get_time/Value/S", JsonEncode(oJsonWriter.write(m_jScroll_get_time), sJsonEncode));
		}
		pItem->AddValue("/seq/N", m_nSeq);
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

void TbBackpack::OnWriteItemReq(AwsMap* pWriteItem, int dwActionType)
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
		if (!m_bItem.empty())
		{
			pItem->AddValue("/item/B", Base64Encode((char*)&m_bItem.m_astList[0], m_bItem.m_udwNum*sizeof(SPlayerItem), sBase64Encode));
		}
		if (!m_jCrystal.empty())
		{
				pItem->AddValue("/AttributeUpdates/crystal/Value/S", JsonEncode(oJsonWriter.write(m_jCrystal), sJsonEncode));
		}
		if (!m_jMaterial.empty())
		{
				pItem->AddValue("/AttributeUpdates/material/Value/S", JsonEncode(oJsonWriter.write(m_jMaterial), sJsonEncode));
		}
		if (!m_jSoul.empty())
		{
				pItem->AddValue("/AttributeUpdates/soul/Value/S", JsonEncode(oJsonWriter.write(m_jSoul), sJsonEncode));
		}
		if (!m_jParts.empty())
		{
				pItem->AddValue("/AttributeUpdates/parts/Value/S", JsonEncode(oJsonWriter.write(m_jParts), sJsonEncode));
		}
		if (!m_jSp_crystal.empty())
		{
				pItem->AddValue("/AttributeUpdates/sp_crystal/Value/S", JsonEncode(oJsonWriter.write(m_jSp_crystal), sJsonEncode));
		}
		if (!m_jScroll.empty())
		{
				pItem->AddValue("/AttributeUpdates/scroll/Value/S", JsonEncode(oJsonWriter.write(m_jScroll), sJsonEncode));
		}
		if (!m_jScroll_get_time.empty())
		{
				pItem->AddValue("/AttributeUpdates/scroll_get_time/Value/S", JsonEncode(oJsonWriter.write(m_jScroll_get_time), sJsonEncode));
		}
		pItem->AddValue("/seq/N", m_nSeq);
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

AwsMap* TbBackpack::OnReadItemReq(unsigned int udwIdxNo)
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

void TbBackpack::OnReadItemReq(AwsMap* pReadItem)
{
	assert(pReadItem);
	AwsList* pKeys = pReadItem->GetAwsMap("RequestItems")->GetAwsMap(GetTableName())->GetAwsList("Keys");
	AwsMap* pKey = new AwsMap;
	assert(pKey);
	pKey->AddValue("/uid/N", m_nUid);
	pKeys->SetValue(pKey, true);
}

int TbBackpack::OnReadItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbBackpack::OnDeleteItemReq(
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

int TbBackpack::OnDeleteItemReq(string& sPostData,
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

int TbBackpack::OnDeleteItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbBackpack::OnGetItemReq(unsigned int udwIdxNo,
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

int TbBackpack::OnGetItemReq(string& sPostData, unsigned int udwIdxNo,
	bool bConsistentRead, bool bReturnConsumedCapacity)
{
	AwsMap* pGetItem = OnGetItemReq(udwIdxNo, bConsistentRead, bReturnConsumedCapacity);
	ostringstream oss;
	pGetItem->Dump(oss);
	sPostData = oss.str();
	delete pGetItem;
	return 0;
}

int TbBackpack::OnGetItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbBackpack::OnPutItemReq(
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
	if (!m_bItem.empty())
	{
		pPutItem->AddValue("/Item/item/B", Base64Encode((char*)&m_bItem.m_astList[0], m_bItem.m_udwNum*sizeof(SPlayerItem), sBase64Encode));
	}
	if (!m_jCrystal.empty())
	{
				pPutItem->AddValue("/Item/crystal/S", JsonEncode(oJsonWriter.write(m_jCrystal), sJsonEncode));
	}
	if (!m_jMaterial.empty())
	{
				pPutItem->AddValue("/Item/material/S", JsonEncode(oJsonWriter.write(m_jMaterial), sJsonEncode));
	}
	if (!m_jSoul.empty())
	{
				pPutItem->AddValue("/Item/soul/S", JsonEncode(oJsonWriter.write(m_jSoul), sJsonEncode));
	}
	if (!m_jParts.empty())
	{
				pPutItem->AddValue("/Item/parts/S", JsonEncode(oJsonWriter.write(m_jParts), sJsonEncode));
	}
	if (!m_jSp_crystal.empty())
	{
				pPutItem->AddValue("/Item/sp_crystal/S", JsonEncode(oJsonWriter.write(m_jSp_crystal), sJsonEncode));
	}
	if (!m_jScroll.empty())
	{
				pPutItem->AddValue("/Item/scroll/S", JsonEncode(oJsonWriter.write(m_jScroll), sJsonEncode));
	}
	if (!m_jScroll_get_time.empty())
	{
				pPutItem->AddValue("/Item/scroll_get_time/S", JsonEncode(oJsonWriter.write(m_jScroll_get_time), sJsonEncode));
	}
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

int TbBackpack::OnPutItemReq(string& sPostData,
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

int TbBackpack::OnPutItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

int TbBackpack::OnResponse(const Json::Value& item)
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
		if (vecMembers[i] == "item")
		{
			Base64Decode(item["item"]["B"].asString(), (char*)&m_bItem.m_astList[0], dwResLen);
			m_bItem.m_udwNum = dwResLen/sizeof(SPlayerItem);
			continue;
		}
		if (vecMembers[i] == "crystal")
		{
			if (FALSE == oJsonReader.parse(item["crystal"]["S"].asString(),m_jCrystal))
			{
				m_jCrystal = Json::Value(Json::nullValue);
			}
			continue;
		}
		if (vecMembers[i] == "material")
		{
			if (FALSE == oJsonReader.parse(item["material"]["S"].asString(),m_jMaterial))
			{
				m_jMaterial = Json::Value(Json::nullValue);
			}
			continue;
		}
		if (vecMembers[i] == "soul")
		{
			if (FALSE == oJsonReader.parse(item["soul"]["S"].asString(),m_jSoul))
			{
				m_jSoul = Json::Value(Json::nullValue);
			}
			continue;
		}
		if (vecMembers[i] == "parts")
		{
			if (FALSE == oJsonReader.parse(item["parts"]["S"].asString(),m_jParts))
			{
				m_jParts = Json::Value(Json::nullValue);
			}
			continue;
		}
		if (vecMembers[i] == "sp_crystal")
		{
			if (FALSE == oJsonReader.parse(item["sp_crystal"]["S"].asString(),m_jSp_crystal))
			{
				m_jSp_crystal = Json::Value(Json::nullValue);
			}
			continue;
		}
		if (vecMembers[i] == "scroll")
		{
			if (FALSE == oJsonReader.parse(item["scroll"]["S"].asString(),m_jScroll))
			{
				m_jScroll = Json::Value(Json::nullValue);
			}
			continue;
		}
		if (vecMembers[i] == "scroll_get_time")
		{
			if (FALSE == oJsonReader.parse(item["scroll_get_time"]["S"].asString(),m_jScroll_get_time))
			{
				m_jScroll_get_time = Json::Value(Json::nullValue);
			}
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

TINT64 TbBackpack::GetSeq()
{
	return m_nSeq;
}

