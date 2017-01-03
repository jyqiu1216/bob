#include "aws_table_throne.h"

TableDesc TbThrone::oTableDesc;

int TbThrone::Init(const string& sConfFile, const string strProjectName)
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

AwsTable* TbThrone::NewObject()
{
	return new TbThrone;
}

string TbThrone::GetTableName()
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

TINT32 TbThrone::GetTableIdx()
{
	 return 0;
}

AwsMap* TbThrone::OnScanReq(unsigned int udwIdxNo, bool bHasStartKey, bool bReturnConsumedCapacity,
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
		pScan->AddValue("/ExclusiveStartKey/id/N",m_nId);
	}
	return pScan;
}

int TbThrone::OnScanReq(string& sPostData, unsigned int udwIdxNo, bool bHasStartKey, bool bReturnConsumedCapacity,
 unsigned int dwLimit, unsigned int dwSegment, unsigned int dwTotalSegments)
{
	AwsMap* pScan = OnScanReq(udwIdxNo, bHasStartKey, bReturnConsumedCapacity, dwLimit, dwSegment, dwTotalSegments);
	ostringstream oss;
	pScan->Dump(oss);
	sPostData = oss.str();
	delete pScan;
	return 0;
}

int TbThrone::OnScanRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbThrone::OnQueryReq(unsigned int udwIdxNo, const CompareDesc& comp_desc,
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
			if(fld_desc.udwFldNo == TbTHRONE_FIELD_SID)
			{
				pQuery->AddValue("/KeyConditions/sid/AttributeValueList[0]/N", m_nSid);
				pQuery->AddValue("/KeyConditions/sid/ComparisonOperator", "EQ");
			}
		}
		else if (i == 1) //1只能是rangekey or local index，可以有多种方式
		{
			if (COMPARE_TYPE_EQ<=comp_desc.dwCompareType && comp_desc.dwCompareType<=COMPARE_TYPE_GT)
			{
				if(fld_desc.udwFldNo == TbTHRONE_FIELD_ID)
				{
					pQuery->AddValue("/KeyConditions/id/AttributeValueList[0]/N", m_nId);
					pQuery->AddValue("/KeyConditions/id/ComparisonOperator", CompareType2Str(comp_desc.dwCompareType));
				}
			}
			else if (COMPARE_TYPE_BETWEEN == comp_desc.dwCompareType)
			{
				if(fld_desc.udwFldNo == TbTHRONE_FIELD_ID)
				{
					pQuery->AddValue("/KeyConditions/id/AttributeValueList[0]/N" , comp_desc.vecN[0]);
					pQuery->AddValue("/KeyConditions/id/AttributeValueList[1]/N" , comp_desc.vecN[1]);
					pQuery->AddValue("/KeyConditions/id/ComparisonOperator", "BETWEEN");
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

int TbThrone::OnQueryReq(string& sPostData, unsigned int udwIdxNo, const CompareDesc& comp_desc,
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

int TbThrone::OnQueryRsp(const Json::Value& item)
{
	return OnResponse(item);
}

int TbThrone::OnCountReq(string& sPostData, unsigned int udwIdxNo, const CompareDesc& comp_desc, 
		bool bConsistentRead, bool bReturnConsumedCapacity, bool bScanIndexForward, unsigned int dwLimit)
{
	AwsMap* pQuery = OnQueryReq(udwIdxNo, comp_desc, bConsistentRead, bReturnConsumedCapacity, bScanIndexForward, dwLimit, true);
	ostringstream oss;
	pQuery->Dump(oss);
	sPostData = oss.str();
	delete pQuery;
	return 0;
}

AwsMap* TbThrone::OnUpdateItemReq(
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
	pUpdateItem->AddValue("/Key/id/N", m_nId);
	for (map<unsigned int, int>::iterator iter = m_mFlag.begin(); iter != m_mFlag.end(); ++iter)
	{
		bUpdateFlag = true;
		if (TbTHRONE_FIELD_POS == iter->first)
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
		if (TbTHRONE_FIELD_ALID == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/alid/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/alid/Value/N", m_nAlid);
				pUpdateItem->AddValue("/AttributeUpdates/alid/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbTHRONE_FIELD_OWNER_ID == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/owner_id/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/owner_id/Value/N", m_nOwner_id);
				pUpdateItem->AddValue("/AttributeUpdates/owner_id/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbTHRONE_FIELD_INFO == iter->first)
		{
			if (!m_jInfo.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/info/Value/S", JsonEncode(oJsonWriter.write(m_jInfo), sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/info/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/info/Action", "DELETE");
			}
			continue;
		}
		if (TbTHRONE_FIELD_TAX_ID == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/tax_id/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/tax_id/Value/N", m_nTax_id);
				pUpdateItem->AddValue("/AttributeUpdates/tax_id/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbTHRONE_FIELD_STATUS == iter->first)
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
		if (TbTHRONE_FIELD_END_TIME == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/end_time/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/end_time/Value/N", m_nEnd_time);
				pUpdateItem->AddValue("/AttributeUpdates/end_time/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbTHRONE_FIELD_SEQ == iter->first)
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
		if (TbTHRONE_FIELD_DEFENDING_NUM == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/defending_num/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/defending_num/Value/N", m_nDefending_num);
				pUpdateItem->AddValue("/AttributeUpdates/defending_num/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbTHRONE_FIELD_DEFENDING_TROOP_NUM == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/defending_troop_num/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/defending_troop_num/Value/N", m_nDefending_troop_num);
				pUpdateItem->AddValue("/AttributeUpdates/defending_troop_num/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbTHRONE_FIELD_DEFENDING_TROOP_FORCE == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/defending_troop_force/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/defending_troop_force/Value/N", m_nDefending_troop_force);
				pUpdateItem->AddValue("/AttributeUpdates/defending_troop_force/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbTHRONE_FIELD_REINFORCE_NUM == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/reinforce_num/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/reinforce_num/Value/N", m_nReinforce_num);
				pUpdateItem->AddValue("/AttributeUpdates/reinforce_num/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbTHRONE_FIELD_REINFORCE_TROOP_NUM == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/reinforce_troop_num/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/reinforce_troop_num/Value/N", m_nReinforce_troop_num);
				pUpdateItem->AddValue("/AttributeUpdates/reinforce_troop_num/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbTHRONE_FIELD_REINFORCE_TROOP_FORCE == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/reinforce_troop_force/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/reinforce_troop_force/Value/N", m_nReinforce_troop_force);
				pUpdateItem->AddValue("/AttributeUpdates/reinforce_troop_force/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbTHRONE_FIELD_OCCUPY_TIME == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/occupy_time/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/occupy_time/Value/N", m_nOccupy_time);
				pUpdateItem->AddValue("/AttributeUpdates/occupy_time/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbTHRONE_FIELD_OWNER_CID == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/owner_cid/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/owner_cid/Value/N", m_nOwner_cid);
				pUpdateItem->AddValue("/AttributeUpdates/owner_cid/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbTHRONE_FIELD_RANK_INFO == iter->first)
		{
			if (!m_jRank_info.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/rank_info/Value/S", JsonEncode(oJsonWriter.write(m_jRank_info), sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/rank_info/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/rank_info/Action", "DELETE");
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

int TbThrone::OnUpdateItemReq(string& sPostData,
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

int TbThrone::OnUpdateItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbThrone::OnWriteItemReq(int dwActionType)
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
		pItem->AddValue("/id/N", m_nId);
		pItem->AddValue("/pos/N", m_nPos);
		pItem->AddValue("/alid/N", m_nAlid);
		pItem->AddValue("/owner_id/N", m_nOwner_id);
		if (!m_jInfo.empty())
		{
				pItem->AddValue("/AttributeUpdates/info/Value/S", JsonEncode(oJsonWriter.write(m_jInfo), sJsonEncode));
		}
		pItem->AddValue("/tax_id/N", m_nTax_id);
		pItem->AddValue("/status/N", m_nStatus);
		pItem->AddValue("/end_time/N", m_nEnd_time);
		pItem->AddValue("/seq/N", m_nSeq);
		pItem->AddValue("/defending_num/N", m_nDefending_num);
		pItem->AddValue("/defending_troop_num/N", m_nDefending_troop_num);
		pItem->AddValue("/defending_troop_force/N", m_nDefending_troop_force);
		pItem->AddValue("/reinforce_num/N", m_nReinforce_num);
		pItem->AddValue("/reinforce_troop_num/N", m_nReinforce_troop_num);
		pItem->AddValue("/reinforce_troop_force/N", m_nReinforce_troop_force);
		pItem->AddValue("/occupy_time/N", m_nOccupy_time);
		pItem->AddValue("/owner_cid/N", m_nOwner_cid);
		if (!m_jRank_info.empty())
		{
				pItem->AddValue("/AttributeUpdates/rank_info/Value/S", JsonEncode(oJsonWriter.write(m_jRank_info), sJsonEncode));
		}
	}
	else if (WRITE_ACTION_TYPE_DELETE == dwActionType)
	{
		pItem = pWriteItem->GetAwsMap("DeleteRequest")->GetAwsMap("Key");
		pItem->AddValue("/sid/N", m_nSid);
		pItem->AddValue("/id/N", m_nId);
	}
	else
	{
		assert(0);
	}
	return pWriteItem;
}

void TbThrone::OnWriteItemReq(AwsMap* pWriteItem, int dwActionType)
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
		pItem->AddValue("/id/N", m_nId);
		pItem->AddValue("/pos/N", m_nPos);
		pItem->AddValue("/alid/N", m_nAlid);
		pItem->AddValue("/owner_id/N", m_nOwner_id);
		if (!m_jInfo.empty())
		{
				pItem->AddValue("/AttributeUpdates/info/Value/S", JsonEncode(oJsonWriter.write(m_jInfo), sJsonEncode));
		}
		pItem->AddValue("/tax_id/N", m_nTax_id);
		pItem->AddValue("/status/N", m_nStatus);
		pItem->AddValue("/end_time/N", m_nEnd_time);
		pItem->AddValue("/seq/N", m_nSeq);
		pItem->AddValue("/defending_num/N", m_nDefending_num);
		pItem->AddValue("/defending_troop_num/N", m_nDefending_troop_num);
		pItem->AddValue("/defending_troop_force/N", m_nDefending_troop_force);
		pItem->AddValue("/reinforce_num/N", m_nReinforce_num);
		pItem->AddValue("/reinforce_troop_num/N", m_nReinforce_troop_num);
		pItem->AddValue("/reinforce_troop_force/N", m_nReinforce_troop_force);
		pItem->AddValue("/occupy_time/N", m_nOccupy_time);
		pItem->AddValue("/owner_cid/N", m_nOwner_cid);
		if (!m_jRank_info.empty())
		{
				pItem->AddValue("/AttributeUpdates/rank_info/Value/S", JsonEncode(oJsonWriter.write(m_jRank_info), sJsonEncode));
		}
	}
	else if (WRITE_ACTION_TYPE_DELETE == dwActionType)
	{
		pItem = pReqItem->GetAwsMap("DeleteRequest")->GetAwsMap("Key");
		pItem->AddValue("/sid/N", m_nSid);
		pItem->AddValue("/id/N", m_nId);
	}
	else
	{
		assert(0);
	}
	pWriteItem->GetAwsMap("RequestItems")->GetAwsList(GetTableName())->SetValue(pReqItem, true);
}

AwsMap* TbThrone::OnReadItemReq(unsigned int udwIdxNo)
{
	oJsonWriter.omitEndingLineFeed();
	IndexDesc& idx_desc = oTableDesc.mIndexDesc[udwIdxNo];
	assert(idx_desc.sName == "PRIMARY"); //只能通过主键查询
	AwsMap* pReadItem = new AwsMap;
	assert(pReadItem);
	pReadItem->AddValue("/Keys[0]/sid/N", m_nSid);
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

void TbThrone::OnReadItemReq(AwsMap* pReadItem)
{
	assert(pReadItem);
	AwsList* pKeys = pReadItem->GetAwsMap("RequestItems")->GetAwsMap(GetTableName())->GetAwsList("Keys");
	AwsMap* pKey = new AwsMap;
	assert(pKey);
	pKey->AddValue("/sid/N", m_nSid);
	pKey->AddValue("/id/N", m_nId);
	pKeys->SetValue(pKey, true);
}

int TbThrone::OnReadItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbThrone::OnDeleteItemReq(
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

int TbThrone::OnDeleteItemReq(string& sPostData,
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

int TbThrone::OnDeleteItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbThrone::OnGetItemReq(unsigned int udwIdxNo,
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

int TbThrone::OnGetItemReq(string& sPostData, unsigned int udwIdxNo,
	bool bConsistentRead, bool bReturnConsumedCapacity)
{
	AwsMap* pGetItem = OnGetItemReq(udwIdxNo, bConsistentRead, bReturnConsumedCapacity);
	ostringstream oss;
	pGetItem->Dump(oss);
	sPostData = oss.str();
	delete pGetItem;
	return 0;
}

int TbThrone::OnGetItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbThrone::OnPutItemReq(
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
	pPutItem->AddValue("/Item/id/N", m_nId);
	pPutItem->AddValue("/Item/pos/N", m_nPos);
	pPutItem->AddValue("/Item/alid/N", m_nAlid);
	pPutItem->AddValue("/Item/owner_id/N", m_nOwner_id);
	if (!m_jInfo.empty())
	{
				pPutItem->AddValue("/Item/info/S", JsonEncode(oJsonWriter.write(m_jInfo), sJsonEncode));
	}
	pPutItem->AddValue("/Item/tax_id/N", m_nTax_id);
	pPutItem->AddValue("/Item/status/N", m_nStatus);
	pPutItem->AddValue("/Item/end_time/N", m_nEnd_time);
	pPutItem->AddValue("/Item/seq/N", m_nSeq);
	pPutItem->AddValue("/Item/defending_num/N", m_nDefending_num);
	pPutItem->AddValue("/Item/defending_troop_num/N", m_nDefending_troop_num);
	pPutItem->AddValue("/Item/defending_troop_force/N", m_nDefending_troop_force);
	pPutItem->AddValue("/Item/reinforce_num/N", m_nReinforce_num);
	pPutItem->AddValue("/Item/reinforce_troop_num/N", m_nReinforce_troop_num);
	pPutItem->AddValue("/Item/reinforce_troop_force/N", m_nReinforce_troop_force);
	pPutItem->AddValue("/Item/occupy_time/N", m_nOccupy_time);
	pPutItem->AddValue("/Item/owner_cid/N", m_nOwner_cid);
	if (!m_jRank_info.empty())
	{
				pPutItem->AddValue("/Item/rank_info/S", JsonEncode(oJsonWriter.write(m_jRank_info), sJsonEncode));
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

int TbThrone::OnPutItemReq(string& sPostData,
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

int TbThrone::OnPutItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

int TbThrone::OnResponse(const Json::Value& item)
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
		if (vecMembers[i] == "id")
		{
			m_nId = strtoll(item["id"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "pos")
		{
			m_nPos = strtoll(item["pos"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "alid")
		{
			m_nAlid = strtoll(item["alid"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "owner_id")
		{
			m_nOwner_id = strtoll(item["owner_id"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "info")
		{
			if (FALSE == oJsonReader.parse(item["info"]["S"].asString(),m_jInfo))
			{
				m_jInfo = Json::Value(Json::nullValue);
			}
			continue;
		}
		if (vecMembers[i] == "tax_id")
		{
			m_nTax_id = strtoll(item["tax_id"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "status")
		{
			m_nStatus = strtoll(item["status"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "end_time")
		{
			m_nEnd_time = strtoll(item["end_time"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "seq")
		{
			m_nSeq = strtoll(item["seq"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "defending_num")
		{
			m_nDefending_num = strtoll(item["defending_num"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "defending_troop_num")
		{
			m_nDefending_troop_num = strtoll(item["defending_troop_num"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "defending_troop_force")
		{
			m_nDefending_troop_force = strtoll(item["defending_troop_force"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "reinforce_num")
		{
			m_nReinforce_num = strtoll(item["reinforce_num"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "reinforce_troop_num")
		{
			m_nReinforce_troop_num = strtoll(item["reinforce_troop_num"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "reinforce_troop_force")
		{
			m_nReinforce_troop_force = strtoll(item["reinforce_troop_force"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "occupy_time")
		{
			m_nOccupy_time = strtoll(item["occupy_time"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "owner_cid")
		{
			m_nOwner_cid = strtoll(item["owner_cid"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "rank_info")
		{
			if (FALSE == oJsonReader.parse(item["rank_info"]["S"].asString(),m_jRank_info))
			{
				m_jRank_info = Json::Value(Json::nullValue);
			}
			continue;
		}
	}
	return 0;
}

TINT64 TbThrone::GetSeq()
{
	return m_nSeq;
}

