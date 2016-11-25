#include "aws_table_al_assist.h"

TableDesc TbAl_assist::oTableDesc;

int TbAl_assist::Init(const string& sConfFile, const string strProjectName)
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

AwsTable* TbAl_assist::NewObject()
{
	return new TbAl_assist;
}

string TbAl_assist::GetTableName()
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

TINT32 TbAl_assist::GetTableIdx()
{
	 return 0;
}

AwsMap* TbAl_assist::OnScanReq(unsigned int udwIdxNo, bool bHasStartKey, bool bReturnConsumedCapacity,
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
		pScan->AddValue("/ExclusiveStartKey/aid/N",m_nAid);
		pScan->AddValue("/ExclusiveStartKey/id/N",m_nId);
	}
	return pScan;
}

int TbAl_assist::OnScanReq(string& sPostData, unsigned int udwIdxNo, bool bHasStartKey, bool bReturnConsumedCapacity,
 unsigned int dwLimit, unsigned int dwSegment, unsigned int dwTotalSegments)
{
	AwsMap* pScan = OnScanReq(udwIdxNo, bHasStartKey, bReturnConsumedCapacity, dwLimit, dwSegment, dwTotalSegments);
	ostringstream oss;
	pScan->Dump(oss);
	sPostData = oss.str();
	delete pScan;
	return 0;
}

int TbAl_assist::OnScanRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbAl_assist::OnQueryReq(unsigned int udwIdxNo, const CompareDesc& comp_desc,
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
			if(fld_desc.udwFldNo == TbAL_ASSIST_FIELD_AID)
			{
				pQuery->AddValue("/KeyConditions/aid/AttributeValueList[0]/N", m_nAid);
				pQuery->AddValue("/KeyConditions/aid/ComparisonOperator", "EQ");
			}
		}
		else if (i == 1) //1只能是rangekey or local index，可以有多种方式
		{
			if (COMPARE_TYPE_EQ<=comp_desc.dwCompareType && comp_desc.dwCompareType<=COMPARE_TYPE_GT)
			{
				if(fld_desc.udwFldNo == TbAL_ASSIST_FIELD_ID)
				{
					pQuery->AddValue("/KeyConditions/id/AttributeValueList[0]/N", m_nId);
					pQuery->AddValue("/KeyConditions/id/ComparisonOperator", CompareType2Str(comp_desc.dwCompareType));
				}
			}
			else if (COMPARE_TYPE_BETWEEN == comp_desc.dwCompareType)
			{
				if(fld_desc.udwFldNo == TbAL_ASSIST_FIELD_ID)
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

int TbAl_assist::OnQueryReq(string& sPostData, unsigned int udwIdxNo, const CompareDesc& comp_desc,
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

int TbAl_assist::OnQueryRsp(const Json::Value& item)
{
	return OnResponse(item);
}

int TbAl_assist::OnCountReq(string& sPostData, unsigned int udwIdxNo, const CompareDesc& comp_desc, 
		bool bConsistentRead, bool bReturnConsumedCapacity, bool bScanIndexForward, unsigned int dwLimit)
{
	AwsMap* pQuery = OnQueryReq(udwIdxNo, comp_desc, bConsistentRead, bReturnConsumedCapacity, bScanIndexForward, dwLimit, true);
	ostringstream oss;
	pQuery->Dump(oss);
	sPostData = oss.str();
	delete pQuery;
	return 0;
}

AwsMap* TbAl_assist::OnUpdateItemReq(
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
	pUpdateItem->AddValue("/Key/aid/N", m_nAid);
	pUpdateItem->AddValue("/Key/id/N", m_nId);
	for (map<unsigned int, int>::iterator iter = m_mFlag.begin(); iter != m_mFlag.end(); ++iter)
	{
		bUpdateFlag = true;
		if (TbAL_ASSIST_FIELD_SID == iter->first)
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
		if (TbAL_ASSIST_FIELD_TYPE == iter->first)
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
		if (TbAL_ASSIST_FIELD_UID == iter->first)
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
		if (TbAL_ASSIST_FIELD_UNAME == iter->first)
		{
			if (!m_sUname.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/uname/Value/S", JsonEncode(m_sUname, sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/uname/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/uname/Action", "DELETE");
			}
			continue;
		}
		if (TbAL_ASSIST_FIELD_CID == iter->first)
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
		if (TbAL_ASSIST_FIELD_TIME == iter->first)
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
		if (TbAL_ASSIST_FIELD_PARAM == iter->first)
		{
			if (!m_bParam.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/param/Value/B", Base64Encode((char*)&m_bParam.m_astList[0], m_bParam.m_udwNum*sizeof(SCommonResource), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/param/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/param/Action", "DELETE");
			}
			continue;
		}
		if (TbAL_ASSIST_FIELD_PROGRESS == iter->first)
		{
			if (!m_bProgress.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/progress/Value/B", Base64Encode((char*)&m_bProgress.m_astList[0], m_bProgress.m_udwNum*sizeof(SCommonResource), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/progress/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/progress/Action", "DELETE");
			}
			continue;
		}
		if (TbAL_ASSIST_FIELD_DESC == iter->first)
		{
			if (!m_sDesc.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/desc/Value/S", JsonEncode(m_sDesc, sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/desc/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/desc/Action", "DELETE");
			}
			continue;
		}
		if (TbAL_ASSIST_FIELD_SEQ == iter->first)
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

int TbAl_assist::OnUpdateItemReq(string& sPostData,
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

int TbAl_assist::OnUpdateItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbAl_assist::OnWriteItemReq(int dwActionType)
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
		pItem->AddValue("/type/N", m_nType);
		pItem->AddValue("/uid/N", m_nUid);
		if (!m_sUname.empty())
		{
			pItem->AddValue("/uname/S", JsonEncode(m_sUname, sJsonEncode));
		}
		pItem->AddValue("/cid/N", m_nCid);
		pItem->AddValue("/aid/N", m_nAid);
		pItem->AddValue("/time/N", m_nTime);
		if (!m_bParam.empty())
		{
			pItem->AddValue("/param/B", Base64Encode((char*)&m_bParam.m_astList[0], m_bParam.m_udwNum*sizeof(SCommonResource), sBase64Encode));
		}
		if (!m_bProgress.empty())
		{
			pItem->AddValue("/progress/B", Base64Encode((char*)&m_bProgress.m_astList[0], m_bProgress.m_udwNum*sizeof(SCommonResource), sBase64Encode));
		}
		if (!m_sDesc.empty())
		{
			pItem->AddValue("/desc/S", JsonEncode(m_sDesc, sJsonEncode));
		}
		pItem->AddValue("/seq/N", m_nSeq);
	}
	else if (WRITE_ACTION_TYPE_DELETE == dwActionType)
	{
		pItem = pWriteItem->GetAwsMap("DeleteRequest")->GetAwsMap("Key");
		pItem->AddValue("/aid/N", m_nAid);
		pItem->AddValue("/id/N", m_nId);
	}
	else
	{
		assert(0);
	}
	return pWriteItem;
}

void TbAl_assist::OnWriteItemReq(AwsMap* pWriteItem, int dwActionType)
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
		pItem->AddValue("/type/N", m_nType);
		pItem->AddValue("/uid/N", m_nUid);
		if (!m_sUname.empty())
		{
			pItem->AddValue("/uname/S", JsonEncode(m_sUname, sJsonEncode));
		}
		pItem->AddValue("/cid/N", m_nCid);
		pItem->AddValue("/aid/N", m_nAid);
		pItem->AddValue("/time/N", m_nTime);
		if (!m_bParam.empty())
		{
			pItem->AddValue("/param/B", Base64Encode((char*)&m_bParam.m_astList[0], m_bParam.m_udwNum*sizeof(SCommonResource), sBase64Encode));
		}
		if (!m_bProgress.empty())
		{
			pItem->AddValue("/progress/B", Base64Encode((char*)&m_bProgress.m_astList[0], m_bProgress.m_udwNum*sizeof(SCommonResource), sBase64Encode));
		}
		if (!m_sDesc.empty())
		{
			pItem->AddValue("/desc/S", JsonEncode(m_sDesc, sJsonEncode));
		}
		pItem->AddValue("/seq/N", m_nSeq);
	}
	else if (WRITE_ACTION_TYPE_DELETE == dwActionType)
	{
		pItem = pReqItem->GetAwsMap("DeleteRequest")->GetAwsMap("Key");
		pItem->AddValue("/aid/N", m_nAid);
		pItem->AddValue("/id/N", m_nId);
	}
	else
	{
		assert(0);
	}
	pWriteItem->GetAwsMap("RequestItems")->GetAwsList(GetTableName())->SetValue(pReqItem, true);
}

AwsMap* TbAl_assist::OnReadItemReq(unsigned int udwIdxNo)
{
	oJsonWriter.omitEndingLineFeed();
	IndexDesc& idx_desc = oTableDesc.mIndexDesc[udwIdxNo];
	assert(idx_desc.sName == "PRIMARY"); //只能通过主键查询
	AwsMap* pReadItem = new AwsMap;
	assert(pReadItem);
	pReadItem->AddValue("/Keys[0]/aid/N", m_nAid);
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

void TbAl_assist::OnReadItemReq(AwsMap* pReadItem)
{
	assert(pReadItem);
	AwsList* pKeys = pReadItem->GetAwsMap("RequestItems")->GetAwsMap(GetTableName())->GetAwsList("Keys");
	AwsMap* pKey = new AwsMap;
	assert(pKey);
	pKey->AddValue("/aid/N", m_nAid);
	pKey->AddValue("/id/N", m_nId);
	pKeys->SetValue(pKey, true);
}

int TbAl_assist::OnReadItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbAl_assist::OnDeleteItemReq(
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
	pDeleteItem->AddValue("/Key/aid/N", m_nAid);
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

int TbAl_assist::OnDeleteItemReq(string& sPostData,
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

int TbAl_assist::OnDeleteItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbAl_assist::OnGetItemReq(unsigned int udwIdxNo,
	bool bConsistentRead, bool bReturnConsumedCapacity)
{
	oJsonWriter.omitEndingLineFeed();
	assert(oTableDesc.mIndexDesc.find(udwIdxNo)!=oTableDesc.mIndexDesc.end());
	IndexDesc& idx_desc = oTableDesc.mIndexDesc[udwIdxNo];
	assert(idx_desc.sName == "PRIMARY"); //只能通过主键查询
	AwsMap* pGetItem = new AwsMap;
	assert(pGetItem);
	pGetItem->AddValue("/TableName", GetTableName());
	pGetItem->AddValue("/Key/aid/N", m_nAid);
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

int TbAl_assist::OnGetItemReq(string& sPostData, unsigned int udwIdxNo,
	bool bConsistentRead, bool bReturnConsumedCapacity)
{
	AwsMap* pGetItem = OnGetItemReq(udwIdxNo, bConsistentRead, bReturnConsumedCapacity);
	ostringstream oss;
	pGetItem->Dump(oss);
	sPostData = oss.str();
	delete pGetItem;
	return 0;
}

int TbAl_assist::OnGetItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbAl_assist::OnPutItemReq(
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
	pPutItem->AddValue("/Item/type/N", m_nType);
	pPutItem->AddValue("/Item/uid/N", m_nUid);
	if (!m_sUname.empty())
	{
		pPutItem->AddValue("/Item/uname/S", JsonEncode(m_sUname, sJsonEncode));
	}
	pPutItem->AddValue("/Item/cid/N", m_nCid);
	pPutItem->AddValue("/Item/aid/N", m_nAid);
	pPutItem->AddValue("/Item/time/N", m_nTime);
	if (!m_bParam.empty())
	{
		pPutItem->AddValue("/Item/param/B", Base64Encode((char*)&m_bParam.m_astList[0], m_bParam.m_udwNum*sizeof(SCommonResource), sBase64Encode));
	}
	if (!m_bProgress.empty())
	{
		pPutItem->AddValue("/Item/progress/B", Base64Encode((char*)&m_bProgress.m_astList[0], m_bProgress.m_udwNum*sizeof(SCommonResource), sBase64Encode));
	}
	if (!m_sDesc.empty())
	{
		pPutItem->AddValue("/Item/desc/S", JsonEncode(m_sDesc, sJsonEncode));
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

int TbAl_assist::OnPutItemReq(string& sPostData,
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

int TbAl_assist::OnPutItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

int TbAl_assist::OnResponse(const Json::Value& item)
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
		if (vecMembers[i] == "type")
		{
			m_nType = strtoll(item["type"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "uid")
		{
			m_nUid = strtoll(item["uid"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "uname")
		{
			m_sUname = item["uname"]["S"].asString();
			continue;
		}
		if (vecMembers[i] == "cid")
		{
			m_nCid = strtoll(item["cid"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "aid")
		{
			m_nAid = strtoll(item["aid"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "time")
		{
			m_nTime = strtoll(item["time"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "param")
		{
			Base64Decode(item["param"]["B"].asString(), (char*)&m_bParam.m_astList[0], dwResLen);
			m_bParam.m_udwNum = dwResLen/sizeof(SCommonResource);
			if (0==m_bParam.m_udwNum && dwResLen>0)
			{
				m_bParam.m_udwNum = 1;
			}
			continue;
		}
		if (vecMembers[i] == "progress")
		{
			Base64Decode(item["progress"]["B"].asString(), (char*)&m_bProgress.m_astList[0], dwResLen);
			m_bProgress.m_udwNum = dwResLen/sizeof(SCommonResource);
			if (0==m_bProgress.m_udwNum && dwResLen>0)
			{
				m_bProgress.m_udwNum = 1;
			}
			continue;
		}
		if (vecMembers[i] == "desc")
		{
			m_sDesc = item["desc"]["S"].asString();
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

TINT64 TbAl_assist::GetSeq()
{
	return m_nSeq;
}

