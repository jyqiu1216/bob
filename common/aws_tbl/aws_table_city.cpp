#include "aws_table_city.h"

TableDesc TbCity::oTableDesc;

int TbCity::Init(const string& sConfFile, const string strProjectName)
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

AwsTable* TbCity::NewObject()
{
	return new TbCity;
}

string TbCity::GetTableName()
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

TINT32 TbCity::GetTableIdx()
{
	 return 0;
}

AwsMap* TbCity::OnScanReq(unsigned int udwIdxNo, bool bHasStartKey, bool bReturnConsumedCapacity,
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

int TbCity::OnScanReq(string& sPostData, unsigned int udwIdxNo, bool bHasStartKey, bool bReturnConsumedCapacity,
 unsigned int dwLimit, unsigned int dwSegment, unsigned int dwTotalSegments)
{
	AwsMap* pScan = OnScanReq(udwIdxNo, bHasStartKey, bReturnConsumedCapacity, dwLimit, dwSegment, dwTotalSegments);
	ostringstream oss;
	pScan->Dump(oss);
	sPostData = oss.str();
	delete pScan;
	return 0;
}

int TbCity::OnScanRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbCity::OnQueryReq(unsigned int udwIdxNo, const CompareDesc& comp_desc,
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
			if(fld_desc.udwFldNo == TbCITY_FIELD_UID)
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

int TbCity::OnQueryReq(string& sPostData, unsigned int udwIdxNo, const CompareDesc& comp_desc,
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

int TbCity::OnQueryRsp(const Json::Value& item)
{
	return OnResponse(item);
}

int TbCity::OnCountReq(string& sPostData, unsigned int udwIdxNo, const CompareDesc& comp_desc, 
		bool bConsistentRead, bool bReturnConsumedCapacity, bool bScanIndexForward, unsigned int dwLimit)
{
	AwsMap* pQuery = OnQueryReq(udwIdxNo, comp_desc, bConsistentRead, bReturnConsumedCapacity, bScanIndexForward, dwLimit, true);
	ostringstream oss;
	pQuery->Dump(oss);
	sPostData = oss.str();
	delete pQuery;
	return 0;
}

AwsMap* TbCity::OnUpdateItemReq(
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
		if (TbCITY_FIELD_POS == iter->first)
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
		if (TbCITY_FIELD_NAME == iter->first)
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
		if (TbCITY_FIELD_RESOURCE == iter->first)
		{
			if (!m_bResource.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/resource/Value/B", Base64Encode((char*)&m_bResource.m_astList[0], m_bResource.m_udwNum*sizeof(SCommonResource), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/resource/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/resource/Action", "DELETE");
			}
			continue;
		}
		if (TbCITY_FIELD_BUILDING == iter->first)
		{
			if (!m_bBuilding.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/building/Value/B", Base64Encode((char*)&m_bBuilding.m_astList[0], m_bBuilding.m_udwNum*sizeof(SCityBuildingNode), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/building/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/building/Action", "DELETE");
			}
			continue;
		}
		if (TbCITY_FIELD_TROOP == iter->first)
		{
			if (!m_bTroop.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/troop/Value/B", Base64Encode((char*)&m_bTroop.m_astList[0], m_bTroop.m_udwNum*sizeof(SCommonTroop), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/troop/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/troop/Action", "DELETE");
			}
			continue;
		}
		if (TbCITY_FIELD_FORT == iter->first)
		{
			if (!m_bFort.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/fort/Value/B", Base64Encode((char*)&m_bFort.m_astList[0], m_bFort.m_udwNum*sizeof(SCommonFort), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/fort/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/fort/Action", "DELETE");
			}
			continue;
		}
		if (TbCITY_FIELD_HOS_WAIT == iter->first)
		{
			if (!m_bHos_wait.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/hos_wait/Value/B", Base64Encode((char*)&m_bHos_wait.m_astList[0], m_bHos_wait.m_udwNum*sizeof(SCommonTroop), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/hos_wait/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/hos_wait/Action", "DELETE");
			}
			continue;
		}
		if (TbCITY_FIELD_DEAD_FORT == iter->first)
		{
			if (!m_bDead_fort.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/dead_fort/Value/B", Base64Encode((char*)&m_bDead_fort.m_astList[0], m_bDead_fort.m_udwNum*sizeof(SCommonFort), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/dead_fort/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/dead_fort/Action", "DELETE");
			}
			continue;
		}
		if (TbCITY_FIELD_RESEARCH == iter->first)
		{
			if (!m_bResearch.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/research/Value/B", Base64Encode((char*)&m_bResearch.m_astList[0], m_bResearch.m_udwNum*sizeof(SCommonResearch), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/research/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/research/Action", "DELETE");
			}
			continue;
		}
		if (TbCITY_FIELD_UTIME == iter->first)
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
		if (TbCITY_FIELD_UNLOCK_BLOCK == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/unlock_block/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/unlock_block/Value/N", m_nUnlock_block);
				pUpdateItem->AddValue("/AttributeUpdates/unlock_block/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbCITY_FIELD_ALTAR_BUFF == iter->first)
		{
			if (!m_bAltar_buff.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/altar_buff/Value/B", Base64Encode((char*)&m_bAltar_buff.m_astList[0], m_bAltar_buff.m_udwNum*sizeof(SBuffInfo), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/altar_buff/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/altar_buff/Action", "DELETE");
			}
			continue;
		}
		if (TbCITY_FIELD_ALTAR_BUFF_BTIME == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/altar_buff_btime/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/altar_buff_btime/Value/N", m_nAltar_buff_btime);
				pUpdateItem->AddValue("/AttributeUpdates/altar_buff_btime/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbCITY_FIELD_ALTAR_BUFF_ETIME == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/altar_buff_etime/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/altar_buff_etime/Value/N", m_nAltar_buff_etime);
				pUpdateItem->AddValue("/AttributeUpdates/altar_buff_etime/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbCITY_FIELD_ALTAR_DRAGON_LV == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/altar_dragon_lv/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/altar_dragon_lv/Value/N", m_nAltar_dragon_lv);
				pUpdateItem->AddValue("/AttributeUpdates/altar_dragon_lv/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbCITY_FIELD_ALTAR_DRAGON_NAME == iter->first)
		{
			if (!m_sAltar_dragon_name.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/altar_dragon_name/Value/S", JsonEncode(m_sAltar_dragon_name, sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/altar_dragon_name/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/altar_dragon_name/Action", "DELETE");
			}
			continue;
		}
		if (TbCITY_FIELD_KNIGHT == iter->first)
		{
			if (!m_bKnight.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/knight/Value/B", Base64Encode((char*)&m_bKnight.m_astList[0], m_bKnight.m_udwNum*sizeof(SKnightInfo), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/knight/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/knight/Action", "DELETE");
			}
			continue;
		}
		if (TbCITY_FIELD_SEQ == iter->first)
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

int TbCity::OnUpdateItemReq(string& sPostData,
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

int TbCity::OnUpdateItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbCity::OnWriteItemReq(int dwActionType)
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
		pItem->AddValue("/pos/N", m_nPos);
		if (!m_sName.empty())
		{
			pItem->AddValue("/name/S", JsonEncode(m_sName, sJsonEncode));
		}
		if (!m_bResource.empty())
		{
			pItem->AddValue("/resource/B", Base64Encode((char*)&m_bResource.m_astList[0], m_bResource.m_udwNum*sizeof(SCommonResource), sBase64Encode));
		}
		if (!m_bBuilding.empty())
		{
			pItem->AddValue("/building/B", Base64Encode((char*)&m_bBuilding.m_astList[0], m_bBuilding.m_udwNum*sizeof(SCityBuildingNode), sBase64Encode));
		}
		if (!m_bTroop.empty())
		{
			pItem->AddValue("/troop/B", Base64Encode((char*)&m_bTroop.m_astList[0], m_bTroop.m_udwNum*sizeof(SCommonTroop), sBase64Encode));
		}
		if (!m_bFort.empty())
		{
			pItem->AddValue("/fort/B", Base64Encode((char*)&m_bFort.m_astList[0], m_bFort.m_udwNum*sizeof(SCommonFort), sBase64Encode));
		}
		if (!m_bHos_wait.empty())
		{
			pItem->AddValue("/hos_wait/B", Base64Encode((char*)&m_bHos_wait.m_astList[0], m_bHos_wait.m_udwNum*sizeof(SCommonTroop), sBase64Encode));
		}
		if (!m_bDead_fort.empty())
		{
			pItem->AddValue("/dead_fort/B", Base64Encode((char*)&m_bDead_fort.m_astList[0], m_bDead_fort.m_udwNum*sizeof(SCommonFort), sBase64Encode));
		}
		if (!m_bResearch.empty())
		{
			pItem->AddValue("/research/B", Base64Encode((char*)&m_bResearch.m_astList[0], m_bResearch.m_udwNum*sizeof(SCommonResearch), sBase64Encode));
		}
		pItem->AddValue("/utime/N", m_nUtime);
		pItem->AddValue("/unlock_block/N", m_nUnlock_block);
		if (!m_bAltar_buff.empty())
		{
			pItem->AddValue("/altar_buff/B", Base64Encode((char*)&m_bAltar_buff.m_astList[0], m_bAltar_buff.m_udwNum*sizeof(SBuffInfo), sBase64Encode));
		}
		pItem->AddValue("/altar_buff_btime/N", m_nAltar_buff_btime);
		pItem->AddValue("/altar_buff_etime/N", m_nAltar_buff_etime);
		pItem->AddValue("/altar_dragon_lv/N", m_nAltar_dragon_lv);
		if (!m_sAltar_dragon_name.empty())
		{
			pItem->AddValue("/altar_dragon_name/S", JsonEncode(m_sAltar_dragon_name, sJsonEncode));
		}
		if (!m_bKnight.empty())
		{
			pItem->AddValue("/knight/B", Base64Encode((char*)&m_bKnight.m_astList[0], m_bKnight.m_udwNum*sizeof(SKnightInfo), sBase64Encode));
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

void TbCity::OnWriteItemReq(AwsMap* pWriteItem, int dwActionType)
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
		pItem->AddValue("/pos/N", m_nPos);
		if (!m_sName.empty())
		{
			pItem->AddValue("/name/S", JsonEncode(m_sName, sJsonEncode));
		}
		if (!m_bResource.empty())
		{
			pItem->AddValue("/resource/B", Base64Encode((char*)&m_bResource.m_astList[0], m_bResource.m_udwNum*sizeof(SCommonResource), sBase64Encode));
		}
		if (!m_bBuilding.empty())
		{
			pItem->AddValue("/building/B", Base64Encode((char*)&m_bBuilding.m_astList[0], m_bBuilding.m_udwNum*sizeof(SCityBuildingNode), sBase64Encode));
		}
		if (!m_bTroop.empty())
		{
			pItem->AddValue("/troop/B", Base64Encode((char*)&m_bTroop.m_astList[0], m_bTroop.m_udwNum*sizeof(SCommonTroop), sBase64Encode));
		}
		if (!m_bFort.empty())
		{
			pItem->AddValue("/fort/B", Base64Encode((char*)&m_bFort.m_astList[0], m_bFort.m_udwNum*sizeof(SCommonFort), sBase64Encode));
		}
		if (!m_bHos_wait.empty())
		{
			pItem->AddValue("/hos_wait/B", Base64Encode((char*)&m_bHos_wait.m_astList[0], m_bHos_wait.m_udwNum*sizeof(SCommonTroop), sBase64Encode));
		}
		if (!m_bDead_fort.empty())
		{
			pItem->AddValue("/dead_fort/B", Base64Encode((char*)&m_bDead_fort.m_astList[0], m_bDead_fort.m_udwNum*sizeof(SCommonFort), sBase64Encode));
		}
		if (!m_bResearch.empty())
		{
			pItem->AddValue("/research/B", Base64Encode((char*)&m_bResearch.m_astList[0], m_bResearch.m_udwNum*sizeof(SCommonResearch), sBase64Encode));
		}
		pItem->AddValue("/utime/N", m_nUtime);
		pItem->AddValue("/unlock_block/N", m_nUnlock_block);
		if (!m_bAltar_buff.empty())
		{
			pItem->AddValue("/altar_buff/B", Base64Encode((char*)&m_bAltar_buff.m_astList[0], m_bAltar_buff.m_udwNum*sizeof(SBuffInfo), sBase64Encode));
		}
		pItem->AddValue("/altar_buff_btime/N", m_nAltar_buff_btime);
		pItem->AddValue("/altar_buff_etime/N", m_nAltar_buff_etime);
		pItem->AddValue("/altar_dragon_lv/N", m_nAltar_dragon_lv);
		if (!m_sAltar_dragon_name.empty())
		{
			pItem->AddValue("/altar_dragon_name/S", JsonEncode(m_sAltar_dragon_name, sJsonEncode));
		}
		if (!m_bKnight.empty())
		{
			pItem->AddValue("/knight/B", Base64Encode((char*)&m_bKnight.m_astList[0], m_bKnight.m_udwNum*sizeof(SKnightInfo), sBase64Encode));
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

AwsMap* TbCity::OnReadItemReq(unsigned int udwIdxNo)
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

void TbCity::OnReadItemReq(AwsMap* pReadItem)
{
	assert(pReadItem);
	AwsList* pKeys = pReadItem->GetAwsMap("RequestItems")->GetAwsMap(GetTableName())->GetAwsList("Keys");
	AwsMap* pKey = new AwsMap;
	assert(pKey);
	pKey->AddValue("/uid/N", m_nUid);
	pKeys->SetValue(pKey, true);
}

int TbCity::OnReadItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbCity::OnDeleteItemReq(
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

int TbCity::OnDeleteItemReq(string& sPostData,
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

int TbCity::OnDeleteItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbCity::OnGetItemReq(unsigned int udwIdxNo,
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

int TbCity::OnGetItemReq(string& sPostData, unsigned int udwIdxNo,
	bool bConsistentRead, bool bReturnConsumedCapacity)
{
	AwsMap* pGetItem = OnGetItemReq(udwIdxNo, bConsistentRead, bReturnConsumedCapacity);
	ostringstream oss;
	pGetItem->Dump(oss);
	sPostData = oss.str();
	delete pGetItem;
	return 0;
}

int TbCity::OnGetItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbCity::OnPutItemReq(
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
	pPutItem->AddValue("/Item/pos/N", m_nPos);
	if (!m_sName.empty())
	{
		pPutItem->AddValue("/Item/name/S", JsonEncode(m_sName, sJsonEncode));
	}
	if (!m_bResource.empty())
	{
		pPutItem->AddValue("/Item/resource/B", Base64Encode((char*)&m_bResource.m_astList[0], m_bResource.m_udwNum*sizeof(SCommonResource), sBase64Encode));
	}
	if (!m_bBuilding.empty())
	{
		pPutItem->AddValue("/Item/building/B", Base64Encode((char*)&m_bBuilding.m_astList[0], m_bBuilding.m_udwNum*sizeof(SCityBuildingNode), sBase64Encode));
	}
	if (!m_bTroop.empty())
	{
		pPutItem->AddValue("/Item/troop/B", Base64Encode((char*)&m_bTroop.m_astList[0], m_bTroop.m_udwNum*sizeof(SCommonTroop), sBase64Encode));
	}
	if (!m_bFort.empty())
	{
		pPutItem->AddValue("/Item/fort/B", Base64Encode((char*)&m_bFort.m_astList[0], m_bFort.m_udwNum*sizeof(SCommonFort), sBase64Encode));
	}
	if (!m_bHos_wait.empty())
	{
		pPutItem->AddValue("/Item/hos_wait/B", Base64Encode((char*)&m_bHos_wait.m_astList[0], m_bHos_wait.m_udwNum*sizeof(SCommonTroop), sBase64Encode));
	}
	if (!m_bDead_fort.empty())
	{
		pPutItem->AddValue("/Item/dead_fort/B", Base64Encode((char*)&m_bDead_fort.m_astList[0], m_bDead_fort.m_udwNum*sizeof(SCommonFort), sBase64Encode));
	}
	if (!m_bResearch.empty())
	{
		pPutItem->AddValue("/Item/research/B", Base64Encode((char*)&m_bResearch.m_astList[0], m_bResearch.m_udwNum*sizeof(SCommonResearch), sBase64Encode));
	}
	pPutItem->AddValue("/Item/utime/N", m_nUtime);
	pPutItem->AddValue("/Item/unlock_block/N", m_nUnlock_block);
	if (!m_bAltar_buff.empty())
	{
		pPutItem->AddValue("/Item/altar_buff/B", Base64Encode((char*)&m_bAltar_buff.m_astList[0], m_bAltar_buff.m_udwNum*sizeof(SBuffInfo), sBase64Encode));
	}
	pPutItem->AddValue("/Item/altar_buff_btime/N", m_nAltar_buff_btime);
	pPutItem->AddValue("/Item/altar_buff_etime/N", m_nAltar_buff_etime);
	pPutItem->AddValue("/Item/altar_dragon_lv/N", m_nAltar_dragon_lv);
	if (!m_sAltar_dragon_name.empty())
	{
		pPutItem->AddValue("/Item/altar_dragon_name/S", JsonEncode(m_sAltar_dragon_name, sJsonEncode));
	}
	if (!m_bKnight.empty())
	{
		pPutItem->AddValue("/Item/knight/B", Base64Encode((char*)&m_bKnight.m_astList[0], m_bKnight.m_udwNum*sizeof(SKnightInfo), sBase64Encode));
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

int TbCity::OnPutItemReq(string& sPostData,
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

int TbCity::OnPutItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

int TbCity::OnResponse(const Json::Value& item)
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
		if (vecMembers[i] == "pos")
		{
			m_nPos = strtoll(item["pos"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "name")
		{
			m_sName = item["name"]["S"].asString();
			continue;
		}
		if (vecMembers[i] == "resource")
		{
			Base64Decode(item["resource"]["B"].asString(), (char*)&m_bResource.m_astList[0], dwResLen);
			m_bResource.m_udwNum = dwResLen/sizeof(SCommonResource);
			if (0==m_bResource.m_udwNum && dwResLen>0)
			{
				m_bResource.m_udwNum = 1;
			}
			continue;
		}
		if (vecMembers[i] == "building")
		{
			Base64Decode(item["building"]["B"].asString(), (char*)&m_bBuilding.m_astList[0], dwResLen);
			m_bBuilding.m_udwNum = dwResLen/sizeof(SCityBuildingNode);
			continue;
		}
		if (vecMembers[i] == "troop")
		{
			Base64Decode(item["troop"]["B"].asString(), (char*)&m_bTroop.m_astList[0], dwResLen);
			m_bTroop.m_udwNum = dwResLen/sizeof(SCommonTroop);
			if (0==m_bTroop.m_udwNum && dwResLen>0)
			{
				m_bTroop.m_udwNum = 1;
			}
			continue;
		}
		if (vecMembers[i] == "fort")
		{
			Base64Decode(item["fort"]["B"].asString(), (char*)&m_bFort.m_astList[0], dwResLen);
			m_bFort.m_udwNum = dwResLen/sizeof(SCommonFort);
			if (0==m_bFort.m_udwNum && dwResLen>0)
			{
				m_bFort.m_udwNum = 1;
			}
			continue;
		}
		if (vecMembers[i] == "hos_wait")
		{
			Base64Decode(item["hos_wait"]["B"].asString(), (char*)&m_bHos_wait.m_astList[0], dwResLen);
			m_bHos_wait.m_udwNum = dwResLen/sizeof(SCommonTroop);
			if (0==m_bHos_wait.m_udwNum && dwResLen>0)
			{
				m_bHos_wait.m_udwNum = 1;
			}
			continue;
		}
		if (vecMembers[i] == "dead_fort")
		{
			Base64Decode(item["dead_fort"]["B"].asString(), (char*)&m_bDead_fort.m_astList[0], dwResLen);
			m_bDead_fort.m_udwNum = dwResLen/sizeof(SCommonFort);
			if (0==m_bDead_fort.m_udwNum && dwResLen>0)
			{
				m_bDead_fort.m_udwNum = 1;
			}
			continue;
		}
		if (vecMembers[i] == "research")
		{
			Base64Decode(item["research"]["B"].asString(), (char*)&m_bResearch.m_astList[0], dwResLen);
			m_bResearch.m_udwNum = dwResLen/sizeof(SCommonResearch);
			if (0==m_bResearch.m_udwNum && dwResLen>0)
			{
				m_bResearch.m_udwNum = 1;
			}
			continue;
		}
		if (vecMembers[i] == "utime")
		{
			m_nUtime = strtoll(item["utime"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "unlock_block")
		{
			m_nUnlock_block = strtoll(item["unlock_block"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "altar_buff")
		{
			Base64Decode(item["altar_buff"]["B"].asString(), (char*)&m_bAltar_buff.m_astList[0], dwResLen);
			m_bAltar_buff.m_udwNum = dwResLen/sizeof(SBuffInfo);
			continue;
		}
		if (vecMembers[i] == "altar_buff_btime")
		{
			m_nAltar_buff_btime = strtoll(item["altar_buff_btime"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "altar_buff_etime")
		{
			m_nAltar_buff_etime = strtoll(item["altar_buff_etime"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "altar_dragon_lv")
		{
			m_nAltar_dragon_lv = strtoll(item["altar_dragon_lv"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "altar_dragon_name")
		{
			m_sAltar_dragon_name = item["altar_dragon_name"]["S"].asString();
			continue;
		}
		if (vecMembers[i] == "knight")
		{
			Base64Decode(item["knight"]["B"].asString(), (char*)&m_bKnight.m_astList[0], dwResLen);
			m_bKnight.m_udwNum = dwResLen/sizeof(SKnightInfo);
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

TINT64 TbCity::GetSeq()
{
	return m_nSeq;
}

