#include "aws_table_al_wall.h"

TableDesc TbAl_wall::oTableDesc;

int TbAl_wall::Init(const string& sConfFile, const string strProjectName)
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

AwsTable* TbAl_wall::NewObject()
{
	return new TbAl_wall;
}

string TbAl_wall::GetTableName()
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

TINT32 TbAl_wall::GetTableIdx()
{
	 return 0;
}

AwsMap* TbAl_wall::OnScanReq(unsigned int udwIdxNo, bool bHasStartKey, bool bReturnConsumedCapacity,
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
		pScan->AddValue("/ExclusiveStartKey/alid/N",m_nAlid);
		pScan->AddValue("/ExclusiveStartKey/wall_id/N",m_nWall_id);
	}
	return pScan;
}

int TbAl_wall::OnScanReq(string& sPostData, unsigned int udwIdxNo, bool bHasStartKey, bool bReturnConsumedCapacity,
 unsigned int dwLimit, unsigned int dwSegment, unsigned int dwTotalSegments)
{
	AwsMap* pScan = OnScanReq(udwIdxNo, bHasStartKey, bReturnConsumedCapacity, dwLimit, dwSegment, dwTotalSegments);
	ostringstream oss;
	pScan->Dump(oss);
	sPostData = oss.str();
	delete pScan;
	return 0;
}

int TbAl_wall::OnScanRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbAl_wall::OnQueryReq(unsigned int udwIdxNo, const CompareDesc& comp_desc,
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
			if(fld_desc.udwFldNo == TbAL_WALL_FIELD_ALID)
			{
				pQuery->AddValue("/KeyConditions/alid/AttributeValueList[0]/N", m_nAlid);
				pQuery->AddValue("/KeyConditions/alid/ComparisonOperator", "EQ");
			}
		}
		else if (i == 1) //1只能是rangekey or local index，可以有多种方式
		{
			if (COMPARE_TYPE_EQ<=comp_desc.dwCompareType && comp_desc.dwCompareType<=COMPARE_TYPE_GT)
			{
				if(fld_desc.udwFldNo == TbAL_WALL_FIELD_WALL_ID)
				{
					pQuery->AddValue("/KeyConditions/wall_id/AttributeValueList[0]/N", m_nWall_id);
					pQuery->AddValue("/KeyConditions/wall_id/ComparisonOperator", CompareType2Str(comp_desc.dwCompareType));
				}
				if(fld_desc.udwFldNo == TbAL_WALL_FIELD_TOPFLAG)
				{
					pQuery->AddValue("/KeyConditions/topflag/AttributeValueList[0]/N", m_nTopflag);
					pQuery->AddValue("/KeyConditions/topflag/ComparisonOperator", CompareType2Str(comp_desc.dwCompareType));
				}
			}
			else if (COMPARE_TYPE_BETWEEN == comp_desc.dwCompareType)
			{
				if(fld_desc.udwFldNo == TbAL_WALL_FIELD_WALL_ID)
				{
					pQuery->AddValue("/KeyConditions/wall_id/AttributeValueList[0]/N" , comp_desc.vecN[0]);
					pQuery->AddValue("/KeyConditions/wall_id/AttributeValueList[1]/N" , comp_desc.vecN[1]);
					pQuery->AddValue("/KeyConditions/wall_id/ComparisonOperator", "BETWEEN");
				}
				if(fld_desc.udwFldNo == TbAL_WALL_FIELD_TOPFLAG)
				{
					pQuery->AddValue("/KeyConditions/topflag/AttributeValueList[0]/N" , comp_desc.vecN[0]);
					pQuery->AddValue("/KeyConditions/topflag/AttributeValueList[1]/N" , comp_desc.vecN[1]);
					pQuery->AddValue("/KeyConditions/topflag/ComparisonOperator", "BETWEEN");
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

int TbAl_wall::OnQueryReq(string& sPostData, unsigned int udwIdxNo, const CompareDesc& comp_desc,
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

int TbAl_wall::OnQueryRsp(const Json::Value& item)
{
	return OnResponse(item);
}

int TbAl_wall::OnCountReq(string& sPostData, unsigned int udwIdxNo, const CompareDesc& comp_desc, 
		bool bConsistentRead, bool bReturnConsumedCapacity, bool bScanIndexForward, unsigned int dwLimit)
{
	AwsMap* pQuery = OnQueryReq(udwIdxNo, comp_desc, bConsistentRead, bReturnConsumedCapacity, bScanIndexForward, dwLimit, true);
	ostringstream oss;
	pQuery->Dump(oss);
	sPostData = oss.str();
	delete pQuery;
	return 0;
}

AwsMap* TbAl_wall::OnUpdateItemReq(
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
	pUpdateItem->AddValue("/Key/alid/N", m_nAlid);
	pUpdateItem->AddValue("/Key/wall_id/N", m_nWall_id);
	for (map<unsigned int, int>::iterator iter = m_mFlag.begin(); iter != m_mFlag.end(); ++iter)
	{
		bUpdateFlag = true;
		if (TbAL_WALL_FIELD_TIME == iter->first)
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
		if (TbAL_WALL_FIELD_ALPOS == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/alpos/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/alpos/Value/N", m_nAlpos);
				pUpdateItem->AddValue("/AttributeUpdates/alpos/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbAL_WALL_FIELD_UID == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/uid/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/uid/Value/N", m_nUid);
				pUpdateItem->AddValue("/AttributeUpdates/uid/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbAL_WALL_FIELD_UIN == iter->first)
		{
			if (!m_sUin.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/uin/Value/S", JsonEncode(m_sUin, sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/uin/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/uin/Action", "DELETE");
			}
			continue;
		}
		if (TbAL_WALL_FIELD_CONTENT == iter->first)
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
		if (TbAL_WALL_FIELD_AVATAR == iter->first)
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
		if (TbAL_WALL_FIELD_TOPFLAG == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/topflag/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/topflag/Value/N", m_nTopflag);
				pUpdateItem->AddValue("/AttributeUpdates/topflag/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbAL_WALL_FIELD_TOPTIME == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/toptime/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/toptime/Value/N", m_nToptime);
				pUpdateItem->AddValue("/AttributeUpdates/toptime/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbAL_WALL_FIELD_RAW_LANG == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/raw_lang/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/raw_lang/Value/N", m_nRaw_lang);
				pUpdateItem->AddValue("/AttributeUpdates/raw_lang/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbAL_WALL_FIELD_TRANSLATE_CONTENT == iter->first)
		{
			if (!m_sTranslate_content.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/translate_content/Value/S", JsonEncode(m_sTranslate_content, sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/translate_content/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/translate_content/Action", "DELETE");
			}
			continue;
		}
		if (TbAL_WALL_FIELD_SEQ == iter->first)
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

int TbAl_wall::OnUpdateItemReq(string& sPostData,
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

int TbAl_wall::OnUpdateItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbAl_wall::OnWriteItemReq(int dwActionType)
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
		pItem->AddValue("/alid/N", m_nAlid);
		pItem->AddValue("/wall_id/N", m_nWall_id);
		pItem->AddValue("/time/N", m_nTime);
		pItem->AddValue("/alpos/N", m_nAlpos);
		pItem->AddValue("/uid/N", m_nUid);
		if (!m_sUin.empty())
		{
			pItem->AddValue("/uin/S", JsonEncode(m_sUin, sJsonEncode));
		}
		if (!m_sContent.empty())
		{
			pItem->AddValue("/content/S", JsonEncode(m_sContent, sJsonEncode));
		}
		pItem->AddValue("/avatar/N", m_nAvatar);
		pItem->AddValue("/topflag/N", m_nTopflag);
		pItem->AddValue("/toptime/N", m_nToptime);
		pItem->AddValue("/raw_lang/N", m_nRaw_lang);
		if (!m_sTranslate_content.empty())
		{
			pItem->AddValue("/translate_content/S", JsonEncode(m_sTranslate_content, sJsonEncode));
		}
		pItem->AddValue("/seq/N", m_nSeq);
	}
	else if (WRITE_ACTION_TYPE_DELETE == dwActionType)
	{
		pItem = pWriteItem->GetAwsMap("DeleteRequest")->GetAwsMap("Key");
		pItem->AddValue("/alid/N", m_nAlid);
		pItem->AddValue("/wall_id/N", m_nWall_id);
	}
	else
	{
		assert(0);
	}
	return pWriteItem;
}

void TbAl_wall::OnWriteItemReq(AwsMap* pWriteItem, int dwActionType)
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
		pItem->AddValue("/alid/N", m_nAlid);
		pItem->AddValue("/wall_id/N", m_nWall_id);
		pItem->AddValue("/time/N", m_nTime);
		pItem->AddValue("/alpos/N", m_nAlpos);
		pItem->AddValue("/uid/N", m_nUid);
		if (!m_sUin.empty())
		{
			pItem->AddValue("/uin/S", JsonEncode(m_sUin, sJsonEncode));
		}
		if (!m_sContent.empty())
		{
			pItem->AddValue("/content/S", JsonEncode(m_sContent, sJsonEncode));
		}
		pItem->AddValue("/avatar/N", m_nAvatar);
		pItem->AddValue("/topflag/N", m_nTopflag);
		pItem->AddValue("/toptime/N", m_nToptime);
		pItem->AddValue("/raw_lang/N", m_nRaw_lang);
		if (!m_sTranslate_content.empty())
		{
			pItem->AddValue("/translate_content/S", JsonEncode(m_sTranslate_content, sJsonEncode));
		}
		pItem->AddValue("/seq/N", m_nSeq);
	}
	else if (WRITE_ACTION_TYPE_DELETE == dwActionType)
	{
		pItem = pReqItem->GetAwsMap("DeleteRequest")->GetAwsMap("Key");
		pItem->AddValue("/alid/N", m_nAlid);
		pItem->AddValue("/wall_id/N", m_nWall_id);
	}
	else
	{
		assert(0);
	}
	pWriteItem->GetAwsMap("RequestItems")->GetAwsList(GetTableName())->SetValue(pReqItem, true);
}

AwsMap* TbAl_wall::OnReadItemReq(unsigned int udwIdxNo)
{
	oJsonWriter.omitEndingLineFeed();
	IndexDesc& idx_desc = oTableDesc.mIndexDesc[udwIdxNo];
	assert(idx_desc.sName == "PRIMARY"); //只能通过主键查询
	AwsMap* pReadItem = new AwsMap;
	assert(pReadItem);
	pReadItem->AddValue("/Keys[0]/alid/N", m_nAlid);
	pReadItem->AddValue("/Keys[0]/wall_id/N", m_nWall_id);
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

void TbAl_wall::OnReadItemReq(AwsMap* pReadItem)
{
	assert(pReadItem);
	AwsList* pKeys = pReadItem->GetAwsMap("RequestItems")->GetAwsMap(GetTableName())->GetAwsList("Keys");
	AwsMap* pKey = new AwsMap;
	assert(pKey);
	pKey->AddValue("/alid/N", m_nAlid);
	pKey->AddValue("/wall_id/N", m_nWall_id);
	pKeys->SetValue(pKey, true);
}

int TbAl_wall::OnReadItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbAl_wall::OnDeleteItemReq(
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
	pDeleteItem->AddValue("/Key/alid/N", m_nAlid);
	pDeleteItem->AddValue("/Key/wall_id/N", m_nWall_id);
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

int TbAl_wall::OnDeleteItemReq(string& sPostData,
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

int TbAl_wall::OnDeleteItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbAl_wall::OnGetItemReq(unsigned int udwIdxNo,
	bool bConsistentRead, bool bReturnConsumedCapacity)
{
	oJsonWriter.omitEndingLineFeed();
	assert(oTableDesc.mIndexDesc.find(udwIdxNo)!=oTableDesc.mIndexDesc.end());
	IndexDesc& idx_desc = oTableDesc.mIndexDesc[udwIdxNo];
	assert(idx_desc.sName == "PRIMARY"); //只能通过主键查询
	AwsMap* pGetItem = new AwsMap;
	assert(pGetItem);
	pGetItem->AddValue("/TableName", GetTableName());
	pGetItem->AddValue("/Key/alid/N", m_nAlid);
	pGetItem->AddValue("/Key/wall_id/N", m_nWall_id);
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

int TbAl_wall::OnGetItemReq(string& sPostData, unsigned int udwIdxNo,
	bool bConsistentRead, bool bReturnConsumedCapacity)
{
	AwsMap* pGetItem = OnGetItemReq(udwIdxNo, bConsistentRead, bReturnConsumedCapacity);
	ostringstream oss;
	pGetItem->Dump(oss);
	sPostData = oss.str();
	delete pGetItem;
	return 0;
}

int TbAl_wall::OnGetItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbAl_wall::OnPutItemReq(
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
	pPutItem->AddValue("/Item/alid/N", m_nAlid);
	pPutItem->AddValue("/Item/wall_id/N", m_nWall_id);
	pPutItem->AddValue("/Item/time/N", m_nTime);
	pPutItem->AddValue("/Item/alpos/N", m_nAlpos);
	pPutItem->AddValue("/Item/uid/N", m_nUid);
	if (!m_sUin.empty())
	{
		pPutItem->AddValue("/Item/uin/S", JsonEncode(m_sUin, sJsonEncode));
	}
	if (!m_sContent.empty())
	{
		pPutItem->AddValue("/Item/content/S", JsonEncode(m_sContent, sJsonEncode));
	}
	pPutItem->AddValue("/Item/avatar/N", m_nAvatar);
	pPutItem->AddValue("/Item/topflag/N", m_nTopflag);
	pPutItem->AddValue("/Item/toptime/N", m_nToptime);
	pPutItem->AddValue("/Item/raw_lang/N", m_nRaw_lang);
	if (!m_sTranslate_content.empty())
	{
		pPutItem->AddValue("/Item/translate_content/S", JsonEncode(m_sTranslate_content, sJsonEncode));
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

int TbAl_wall::OnPutItemReq(string& sPostData,
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

int TbAl_wall::OnPutItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

int TbAl_wall::OnResponse(const Json::Value& item)
{
	oJsonWriter.omitEndingLineFeed();
	int dwResLen = 0;
	dwResLen = 0;
	this->Reset();
	Json::Value::Members vecMembers = item.getMemberNames();
	for (unsigned int i = 0; i < vecMembers.size(); ++i)
	{
 		if (vecMembers[i] == "alid")
		{
			m_nAlid = strtoll(item["alid"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "wall_id")
		{
			m_nWall_id = strtoll(item["wall_id"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "time")
		{
			m_nTime = strtoll(item["time"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "alpos")
		{
			m_nAlpos = strtoll(item["alpos"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "uid")
		{
			m_nUid = strtoll(item["uid"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "uin")
		{
			m_sUin = item["uin"]["S"].asString();
			continue;
		}
		if (vecMembers[i] == "content")
		{
			m_sContent = item["content"]["S"].asString();
			continue;
		}
		if (vecMembers[i] == "avatar")
		{
			m_nAvatar = strtoll(item["avatar"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "topflag")
		{
			m_nTopflag = strtoll(item["topflag"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "toptime")
		{
			m_nToptime = strtoll(item["toptime"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "raw_lang")
		{
			m_nRaw_lang = strtoll(item["raw_lang"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "translate_content")
		{
			m_sTranslate_content = item["translate_content"]["S"].asString();
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

TINT64 TbAl_wall::GetSeq()
{
	return m_nSeq;
}

