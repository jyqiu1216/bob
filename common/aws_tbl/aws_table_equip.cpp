#include "aws_table_equip.h"

TableDesc TbEquip::oTableDesc;

int TbEquip::Init(const string& sConfFile, const string strProjectName)
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

AwsTable* TbEquip::NewObject()
{
	return new TbEquip;
}

string TbEquip::GetTableName()
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

TINT32 TbEquip::GetTableIdx()
{
	 return 0;
}

AwsMap* TbEquip::OnScanReq(unsigned int udwIdxNo, bool bHasStartKey, bool bReturnConsumedCapacity,
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
		pScan->AddValue("/ExclusiveStartKey/id/N",m_nId);
	}
	return pScan;
}

int TbEquip::OnScanReq(string& sPostData, unsigned int udwIdxNo, bool bHasStartKey, bool bReturnConsumedCapacity,
 unsigned int dwLimit, unsigned int dwSegment, unsigned int dwTotalSegments)
{
	AwsMap* pScan = OnScanReq(udwIdxNo, bHasStartKey, bReturnConsumedCapacity, dwLimit, dwSegment, dwTotalSegments);
	ostringstream oss;
	pScan->Dump(oss);
	sPostData = oss.str();
	delete pScan;
	return 0;
}

int TbEquip::OnScanRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbEquip::OnQueryReq(unsigned int udwIdxNo, const CompareDesc& comp_desc,
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
			if(fld_desc.udwFldNo == TbEQUIP_FIELD_UID)
			{
				pQuery->AddValue("/KeyConditions/uid/AttributeValueList[0]/N", m_nUid);
				pQuery->AddValue("/KeyConditions/uid/ComparisonOperator", "EQ");
			}
		}
		else if (i == 1) //1只能是rangekey or local index，可以有多种方式
		{
			if (COMPARE_TYPE_EQ<=comp_desc.dwCompareType && comp_desc.dwCompareType<=COMPARE_TYPE_GT)
			{
				if(fld_desc.udwFldNo == TbEQUIP_FIELD_ID)
				{
					pQuery->AddValue("/KeyConditions/id/AttributeValueList[0]/N", m_nId);
					pQuery->AddValue("/KeyConditions/id/ComparisonOperator", CompareType2Str(comp_desc.dwCompareType));
				}
				if(fld_desc.udwFldNo == TbEQUIP_FIELD_STATUS)
				{
					pQuery->AddValue("/KeyConditions/status/AttributeValueList[0]/N", m_nStatus);
					pQuery->AddValue("/KeyConditions/status/ComparisonOperator", CompareType2Str(comp_desc.dwCompareType));
				}
			}
			else if (COMPARE_TYPE_BETWEEN == comp_desc.dwCompareType)
			{
				if(fld_desc.udwFldNo == TbEQUIP_FIELD_ID)
				{
					pQuery->AddValue("/KeyConditions/id/AttributeValueList[0]/N" , comp_desc.vecN[0]);
					pQuery->AddValue("/KeyConditions/id/AttributeValueList[1]/N" , comp_desc.vecN[1]);
					pQuery->AddValue("/KeyConditions/id/ComparisonOperator", "BETWEEN");
				}
				if(fld_desc.udwFldNo == TbEQUIP_FIELD_STATUS)
				{
					pQuery->AddValue("/KeyConditions/status/AttributeValueList[0]/N" , comp_desc.vecN[0]);
					pQuery->AddValue("/KeyConditions/status/AttributeValueList[1]/N" , comp_desc.vecN[1]);
					pQuery->AddValue("/KeyConditions/status/ComparisonOperator", "BETWEEN");
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

int TbEquip::OnQueryReq(string& sPostData, unsigned int udwIdxNo, const CompareDesc& comp_desc,
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

int TbEquip::OnQueryRsp(const Json::Value& item)
{
	return OnResponse(item);
}

int TbEquip::OnCountReq(string& sPostData, unsigned int udwIdxNo, const CompareDesc& comp_desc, 
		bool bConsistentRead, bool bReturnConsumedCapacity, bool bScanIndexForward, unsigned int dwLimit)
{
	AwsMap* pQuery = OnQueryReq(udwIdxNo, comp_desc, bConsistentRead, bReturnConsumedCapacity, bScanIndexForward, dwLimit, true);
	ostringstream oss;
	pQuery->Dump(oss);
	sPostData = oss.str();
	delete pQuery;
	return 0;
}

AwsMap* TbEquip::OnUpdateItemReq(
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
	pUpdateItem->AddValue("/Key/id/N", m_nId);
	for (map<unsigned int, int>::iterator iter = m_mFlag.begin(); iter != m_mFlag.end(); ++iter)
	{
		bUpdateFlag = true;
		if (TbEQUIP_FIELD_EQUIP_TYPE == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/equip_type/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/equip_type/Value/N", m_nEquip_type);
				pUpdateItem->AddValue("/AttributeUpdates/equip_type/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbEQUIP_FIELD_STATUS == iter->first)
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
		if (TbEQUIP_FIELD_PUT_ON_TIME == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/put_on_time/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/put_on_time/Value/N", m_nPut_on_time);
				pUpdateItem->AddValue("/AttributeUpdates/put_on_time/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbEQUIP_FIELD_CRYSTAL == iter->first)
		{
			if (!m_bCrystal.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/crystal/Value/B", Base64Encode((char*)&m_bCrystal.m_astList[0], m_bCrystal.m_udwNum*sizeof(SCrystal), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/crystal/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/crystal/Action", "DELETE");
			}
			continue;
		}
		if (TbEQUIP_FIELD_BUFF == iter->first)
		{
			if (!m_jBuff.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/buff/Value/S", JsonEncode(oJsonWriter.write(m_jBuff), sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/buff/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/buff/Action", "DELETE");
			}
			continue;
		}
		if (TbEQUIP_FIELD_PUT_ON_POS == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/put_on_pos/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/put_on_pos/Value/N", m_nPut_on_pos);
				pUpdateItem->AddValue("/AttributeUpdates/put_on_pos/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbEQUIP_FIELD_MISTERY_BUFF == iter->first)
		{
			if (!m_jMistery_buff.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/mistery_buff/Value/S", JsonEncode(oJsonWriter.write(m_jMistery_buff), sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/mistery_buff/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/mistery_buff/Action", "DELETE");
			}
			continue;
		}
		if (TbEQUIP_FIELD_EQUIP_LV == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/equip_lv/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/equip_lv/Value/N", m_nEquip_lv);
				pUpdateItem->AddValue("/AttributeUpdates/equip_lv/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbEQUIP_FIELD_GET_TIME == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/get_time/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/get_time/Value/N", m_nGet_time);
				pUpdateItem->AddValue("/AttributeUpdates/get_time/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbEQUIP_FIELD_SEQ == iter->first)
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

int TbEquip::OnUpdateItemReq(string& sPostData,
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

int TbEquip::OnUpdateItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbEquip::OnWriteItemReq(int dwActionType)
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
		pItem->AddValue("/id/N", m_nId);
		pItem->AddValue("/equip_type/N", m_nEquip_type);
		pItem->AddValue("/status/N", m_nStatus);
		pItem->AddValue("/put_on_time/N", m_nPut_on_time);
		if (!m_bCrystal.empty())
		{
			pItem->AddValue("/crystal/B", Base64Encode((char*)&m_bCrystal.m_astList[0], m_bCrystal.m_udwNum*sizeof(SCrystal), sBase64Encode));
		}
		if (!m_jBuff.empty())
		{
				pItem->AddValue("/AttributeUpdates/buff/Value/S", JsonEncode(oJsonWriter.write(m_jBuff), sJsonEncode));
		}
		pItem->AddValue("/put_on_pos/N", m_nPut_on_pos);
		if (!m_jMistery_buff.empty())
		{
				pItem->AddValue("/AttributeUpdates/mistery_buff/Value/S", JsonEncode(oJsonWriter.write(m_jMistery_buff), sJsonEncode));
		}
		pItem->AddValue("/equip_lv/N", m_nEquip_lv);
		pItem->AddValue("/get_time/N", m_nGet_time);
		pItem->AddValue("/seq/N", m_nSeq);
	}
	else if (WRITE_ACTION_TYPE_DELETE == dwActionType)
	{
		pItem = pWriteItem->GetAwsMap("DeleteRequest")->GetAwsMap("Key");
		pItem->AddValue("/uid/N", m_nUid);
		pItem->AddValue("/id/N", m_nId);
	}
	else
	{
		assert(0);
	}
	return pWriteItem;
}

void TbEquip::OnWriteItemReq(AwsMap* pWriteItem, int dwActionType)
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
		pItem->AddValue("/id/N", m_nId);
		pItem->AddValue("/equip_type/N", m_nEquip_type);
		pItem->AddValue("/status/N", m_nStatus);
		pItem->AddValue("/put_on_time/N", m_nPut_on_time);
		if (!m_bCrystal.empty())
		{
			pItem->AddValue("/crystal/B", Base64Encode((char*)&m_bCrystal.m_astList[0], m_bCrystal.m_udwNum*sizeof(SCrystal), sBase64Encode));
		}
		if (!m_jBuff.empty())
		{
				pItem->AddValue("/AttributeUpdates/buff/Value/S", JsonEncode(oJsonWriter.write(m_jBuff), sJsonEncode));
		}
		pItem->AddValue("/put_on_pos/N", m_nPut_on_pos);
		if (!m_jMistery_buff.empty())
		{
				pItem->AddValue("/AttributeUpdates/mistery_buff/Value/S", JsonEncode(oJsonWriter.write(m_jMistery_buff), sJsonEncode));
		}
		pItem->AddValue("/equip_lv/N", m_nEquip_lv);
		pItem->AddValue("/get_time/N", m_nGet_time);
		pItem->AddValue("/seq/N", m_nSeq);
	}
	else if (WRITE_ACTION_TYPE_DELETE == dwActionType)
	{
		pItem = pReqItem->GetAwsMap("DeleteRequest")->GetAwsMap("Key");
		pItem->AddValue("/uid/N", m_nUid);
		pItem->AddValue("/id/N", m_nId);
	}
	else
	{
		assert(0);
	}
	pWriteItem->GetAwsMap("RequestItems")->GetAwsList(GetTableName())->SetValue(pReqItem, true);
}

AwsMap* TbEquip::OnReadItemReq(unsigned int udwIdxNo)
{
	oJsonWriter.omitEndingLineFeed();
	IndexDesc& idx_desc = oTableDesc.mIndexDesc[udwIdxNo];
	assert(idx_desc.sName == "PRIMARY"); //只能通过主键查询
	AwsMap* pReadItem = new AwsMap;
	assert(pReadItem);
	pReadItem->AddValue("/Keys[0]/uid/N", m_nUid);
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

void TbEquip::OnReadItemReq(AwsMap* pReadItem)
{
	assert(pReadItem);
	AwsList* pKeys = pReadItem->GetAwsMap("RequestItems")->GetAwsMap(GetTableName())->GetAwsList("Keys");
	AwsMap* pKey = new AwsMap;
	assert(pKey);
	pKey->AddValue("/uid/N", m_nUid);
	pKey->AddValue("/id/N", m_nId);
	pKeys->SetValue(pKey, true);
}

int TbEquip::OnReadItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbEquip::OnDeleteItemReq(
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
	pDeleteItem->AddValue("/Key/id/N", m_nId);
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

int TbEquip::OnDeleteItemReq(string& sPostData,
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

int TbEquip::OnDeleteItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbEquip::OnGetItemReq(unsigned int udwIdxNo,
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

int TbEquip::OnGetItemReq(string& sPostData, unsigned int udwIdxNo,
	bool bConsistentRead, bool bReturnConsumedCapacity)
{
	AwsMap* pGetItem = OnGetItemReq(udwIdxNo, bConsistentRead, bReturnConsumedCapacity);
	ostringstream oss;
	pGetItem->Dump(oss);
	sPostData = oss.str();
	delete pGetItem;
	return 0;
}

int TbEquip::OnGetItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbEquip::OnPutItemReq(
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
	pPutItem->AddValue("/Item/id/N", m_nId);
	pPutItem->AddValue("/Item/equip_type/N", m_nEquip_type);
	pPutItem->AddValue("/Item/status/N", m_nStatus);
	pPutItem->AddValue("/Item/put_on_time/N", m_nPut_on_time);
	if (!m_bCrystal.empty())
	{
		pPutItem->AddValue("/Item/crystal/B", Base64Encode((char*)&m_bCrystal.m_astList[0], m_bCrystal.m_udwNum*sizeof(SCrystal), sBase64Encode));
	}
	if (!m_jBuff.empty())
	{
				pPutItem->AddValue("/Item/buff/S", JsonEncode(oJsonWriter.write(m_jBuff), sJsonEncode));
	}
	pPutItem->AddValue("/Item/put_on_pos/N", m_nPut_on_pos);
	if (!m_jMistery_buff.empty())
	{
				pPutItem->AddValue("/Item/mistery_buff/S", JsonEncode(oJsonWriter.write(m_jMistery_buff), sJsonEncode));
	}
	pPutItem->AddValue("/Item/equip_lv/N", m_nEquip_lv);
	pPutItem->AddValue("/Item/get_time/N", m_nGet_time);
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

int TbEquip::OnPutItemReq(string& sPostData,
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

int TbEquip::OnPutItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

int TbEquip::OnResponse(const Json::Value& item)
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
		if (vecMembers[i] == "id")
		{
			m_nId = strtoll(item["id"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "equip_type")
		{
			m_nEquip_type = strtoll(item["equip_type"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "status")
		{
			m_nStatus = strtoll(item["status"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "put_on_time")
		{
			m_nPut_on_time = strtoll(item["put_on_time"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "crystal")
		{
			Base64Decode(item["crystal"]["B"].asString(), (char*)&m_bCrystal.m_astList[0], dwResLen);
			m_bCrystal.m_udwNum = dwResLen/sizeof(SCrystal);
			if (0==m_bCrystal.m_udwNum && dwResLen>0)
			{
				m_bCrystal.m_udwNum = 1;
			}
			continue;
		}
		if (vecMembers[i] == "buff")
		{
			if (FALSE == oJsonReader.parse(item["buff"]["S"].asString(),m_jBuff))
			{
				m_jBuff = Json::Value(Json::nullValue);
			}
			continue;
		}
		if (vecMembers[i] == "put_on_pos")
		{
			m_nPut_on_pos = strtoll(item["put_on_pos"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "mistery_buff")
		{
			if (FALSE == oJsonReader.parse(item["mistery_buff"]["S"].asString(),m_jMistery_buff))
			{
				m_jMistery_buff = Json::Value(Json::nullValue);
			}
			continue;
		}
		if (vecMembers[i] == "equip_lv")
		{
			m_nEquip_lv = strtoll(item["equip_lv"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "get_time")
		{
			m_nGet_time = strtoll(item["get_time"]["N"].asString().c_str(), NULL, 10);
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

TINT64 TbEquip::GetSeq()
{
	return m_nSeq;
}

