#include "aws_table_mail_user.h"

TableDesc TbMail_user::oTableDesc;

int TbMail_user::Init(const string& sConfFile, const string strProjectName)
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

AwsTable* TbMail_user::NewObject()
{
	return new TbMail_user;
}

string TbMail_user::GetTableName()
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

TINT32 TbMail_user::GetTableIdx()
{
	 return 0;
}

AwsMap* TbMail_user::OnScanReq(unsigned int udwIdxNo, bool bHasStartKey, bool bReturnConsumedCapacity,
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
		pScan->AddValue("/ExclusiveStartKey/mid/N",m_nMid);
	}
	return pScan;
}

int TbMail_user::OnScanReq(string& sPostData, unsigned int udwIdxNo, bool bHasStartKey, bool bReturnConsumedCapacity,
 unsigned int dwLimit, unsigned int dwSegment, unsigned int dwTotalSegments)
{
	AwsMap* pScan = OnScanReq(udwIdxNo, bHasStartKey, bReturnConsumedCapacity, dwLimit, dwSegment, dwTotalSegments);
	ostringstream oss;
	pScan->Dump(oss);
	sPostData = oss.str();
	delete pScan;
	return 0;
}

int TbMail_user::OnScanRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbMail_user::OnQueryReq(unsigned int udwIdxNo, const CompareDesc& comp_desc,
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
			if(fld_desc.udwFldNo == TbMAIL_USER_FIELD_UID)
			{
				pQuery->AddValue("/KeyConditions/uid/AttributeValueList[0]/N", m_nUid);
				pQuery->AddValue("/KeyConditions/uid/ComparisonOperator", "EQ");
			}
		}
		else if (i == 1) //1只能是rangekey or local index，可以有多种方式
		{
			if (COMPARE_TYPE_EQ<=comp_desc.dwCompareType && comp_desc.dwCompareType<=COMPARE_TYPE_GT)
			{
				if(fld_desc.udwFldNo == TbMAIL_USER_FIELD_MID)
				{
					pQuery->AddValue("/KeyConditions/mid/AttributeValueList[0]/N", m_nMid);
					pQuery->AddValue("/KeyConditions/mid/ComparisonOperator", CompareType2Str(comp_desc.dwCompareType));
				}
			}
			else if (COMPARE_TYPE_BETWEEN == comp_desc.dwCompareType)
			{
				if(fld_desc.udwFldNo == TbMAIL_USER_FIELD_MID)
				{
					pQuery->AddValue("/KeyConditions/mid/AttributeValueList[0]/N" , comp_desc.vecN[0]);
					pQuery->AddValue("/KeyConditions/mid/AttributeValueList[1]/N" , comp_desc.vecN[1]);
					pQuery->AddValue("/KeyConditions/mid/ComparisonOperator", "BETWEEN");
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

int TbMail_user::OnQueryReq(string& sPostData, unsigned int udwIdxNo, const CompareDesc& comp_desc,
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

int TbMail_user::OnQueryRsp(const Json::Value& item)
{
	return OnResponse(item);
}

int TbMail_user::OnCountReq(string& sPostData, unsigned int udwIdxNo, const CompareDesc& comp_desc, 
		bool bConsistentRead, bool bReturnConsumedCapacity, bool bScanIndexForward, unsigned int dwLimit)
{
	AwsMap* pQuery = OnQueryReq(udwIdxNo, comp_desc, bConsistentRead, bReturnConsumedCapacity, bScanIndexForward, dwLimit, true);
	ostringstream oss;
	pQuery->Dump(oss);
	sPostData = oss.str();
	delete pQuery;
	return 0;
}

AwsMap* TbMail_user::OnUpdateItemReq(
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
	pUpdateItem->AddValue("/Key/mid/N", m_nMid);
	for (map<unsigned int, int>::iterator iter = m_mFlag.begin(); iter != m_mFlag.end(); ++iter)
	{
		bUpdateFlag = true;
		if (TbMAIL_USER_FIELD_SUID == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/suid/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/suid/Value/N", m_nSuid);
				pUpdateItem->AddValue("/AttributeUpdates/suid/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMAIL_USER_FIELD_TUID == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/tuid/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/tuid/Value/N", m_nTuid);
				pUpdateItem->AddValue("/AttributeUpdates/tuid/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMAIL_USER_FIELD_TIME == iter->first)
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
		if (TbMAIL_USER_FIELD_STATUS == iter->first)
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
		if (TbMAIL_USER_FIELD_DISPLAY_TYPE == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/display_type/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/display_type/Value/N", m_nDisplay_type);
				pUpdateItem->AddValue("/AttributeUpdates/display_type/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMAIL_USER_FIELD_RECEIVER_AVATAR == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/receiver_avatar/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/receiver_avatar/Value/N", m_nReceiver_avatar);
				pUpdateItem->AddValue("/AttributeUpdates/receiver_avatar/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMAIL_USER_FIELD_RECEIVER_NAME == iter->first)
		{
			if (!m_sReceiver_name.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/receiver_name/Value/S", JsonEncode(m_sReceiver_name, sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/receiver_name/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/receiver_name/Action", "DELETE");
			}
			continue;
		}
		if (TbMAIL_USER_FIELD_HAS_REWARD == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/has_reward/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/has_reward/Value/N", m_nHas_reward);
				pUpdateItem->AddValue("/AttributeUpdates/has_reward/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMAIL_USER_FIELD_RECEIVER_ALNICK == iter->first)
		{
			if (!m_sReceiver_alnick.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/receiver_alnick/Value/S", JsonEncode(m_sReceiver_alnick, sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/receiver_alnick/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/receiver_alnick/Action", "DELETE");
			}
			continue;
		}
		if (TbMAIL_USER_FIELD_IS_AL == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/is_al/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/is_al/Value/N", m_nIs_al);
				pUpdateItem->AddValue("/AttributeUpdates/is_al/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMAIL_USER_FIELD_IS_SINGLE_SVR == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/is_single_svr/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/is_single_svr/Value/N", m_nIs_single_svr);
				pUpdateItem->AddValue("/AttributeUpdates/is_single_svr/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMAIL_USER_FIELD_SID == iter->first)
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
		if (TbMAIL_USER_FIELD_VERSION == iter->first)
		{
			if (!m_sVersion.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/version/Value/S", JsonEncode(m_sVersion, sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/version/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/version/Action", "DELETE");
			}
			continue;
		}
		if (TbMAIL_USER_FIELD_PLATFORM == iter->first)
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
		if (TbMAIL_USER_FIELD_SEQ == iter->first)
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

int TbMail_user::OnUpdateItemReq(string& sPostData,
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

int TbMail_user::OnUpdateItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbMail_user::OnWriteItemReq(int dwActionType)
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
		pItem->AddValue("/mid/N", m_nMid);
		pItem->AddValue("/suid/N", m_nSuid);
		pItem->AddValue("/tuid/N", m_nTuid);
		pItem->AddValue("/time/N", m_nTime);
		pItem->AddValue("/status/N", m_nStatus);
		pItem->AddValue("/display_type/N", m_nDisplay_type);
		pItem->AddValue("/receiver_avatar/N", m_nReceiver_avatar);
		if (!m_sReceiver_name.empty())
		{
			pItem->AddValue("/receiver_name/S", JsonEncode(m_sReceiver_name, sJsonEncode));
		}
		pItem->AddValue("/has_reward/N", m_nHas_reward);
		if (!m_sReceiver_alnick.empty())
		{
			pItem->AddValue("/receiver_alnick/S", JsonEncode(m_sReceiver_alnick, sJsonEncode));
		}
		pItem->AddValue("/is_al/N", m_nIs_al);
		pItem->AddValue("/is_single_svr/N", m_nIs_single_svr);
		pItem->AddValue("/sid/N", m_nSid);
		if (!m_sVersion.empty())
		{
			pItem->AddValue("/version/S", JsonEncode(m_sVersion, sJsonEncode));
		}
		if (!m_sPlatform.empty())
		{
			pItem->AddValue("/platform/S", JsonEncode(m_sPlatform, sJsonEncode));
		}
		pItem->AddValue("/seq/N", m_nSeq);
	}
	else if (WRITE_ACTION_TYPE_DELETE == dwActionType)
	{
		pItem = pWriteItem->GetAwsMap("DeleteRequest")->GetAwsMap("Key");
		pItem->AddValue("/uid/N", m_nUid);
		pItem->AddValue("/mid/N", m_nMid);
	}
	else
	{
		assert(0);
	}
	return pWriteItem;
}

void TbMail_user::OnWriteItemReq(AwsMap* pWriteItem, int dwActionType)
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
		pItem->AddValue("/mid/N", m_nMid);
		pItem->AddValue("/suid/N", m_nSuid);
		pItem->AddValue("/tuid/N", m_nTuid);
		pItem->AddValue("/time/N", m_nTime);
		pItem->AddValue("/status/N", m_nStatus);
		pItem->AddValue("/display_type/N", m_nDisplay_type);
		pItem->AddValue("/receiver_avatar/N", m_nReceiver_avatar);
		if (!m_sReceiver_name.empty())
		{
			pItem->AddValue("/receiver_name/S", JsonEncode(m_sReceiver_name, sJsonEncode));
		}
		pItem->AddValue("/has_reward/N", m_nHas_reward);
		if (!m_sReceiver_alnick.empty())
		{
			pItem->AddValue("/receiver_alnick/S", JsonEncode(m_sReceiver_alnick, sJsonEncode));
		}
		pItem->AddValue("/is_al/N", m_nIs_al);
		pItem->AddValue("/is_single_svr/N", m_nIs_single_svr);
		pItem->AddValue("/sid/N", m_nSid);
		if (!m_sVersion.empty())
		{
			pItem->AddValue("/version/S", JsonEncode(m_sVersion, sJsonEncode));
		}
		if (!m_sPlatform.empty())
		{
			pItem->AddValue("/platform/S", JsonEncode(m_sPlatform, sJsonEncode));
		}
		pItem->AddValue("/seq/N", m_nSeq);
	}
	else if (WRITE_ACTION_TYPE_DELETE == dwActionType)
	{
		pItem = pReqItem->GetAwsMap("DeleteRequest")->GetAwsMap("Key");
		pItem->AddValue("/uid/N", m_nUid);
		pItem->AddValue("/mid/N", m_nMid);
	}
	else
	{
		assert(0);
	}
	pWriteItem->GetAwsMap("RequestItems")->GetAwsList(GetTableName())->SetValue(pReqItem, true);
}

AwsMap* TbMail_user::OnReadItemReq(unsigned int udwIdxNo)
{
	oJsonWriter.omitEndingLineFeed();
	IndexDesc& idx_desc = oTableDesc.mIndexDesc[udwIdxNo];
	assert(idx_desc.sName == "PRIMARY"); //只能通过主键查询
	AwsMap* pReadItem = new AwsMap;
	assert(pReadItem);
	pReadItem->AddValue("/Keys[0]/uid/N", m_nUid);
	pReadItem->AddValue("/Keys[0]/mid/N", m_nMid);
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

void TbMail_user::OnReadItemReq(AwsMap* pReadItem)
{
	assert(pReadItem);
	AwsList* pKeys = pReadItem->GetAwsMap("RequestItems")->GetAwsMap(GetTableName())->GetAwsList("Keys");
	AwsMap* pKey = new AwsMap;
	assert(pKey);
	pKey->AddValue("/uid/N", m_nUid);
	pKey->AddValue("/mid/N", m_nMid);
	pKeys->SetValue(pKey, true);
}

int TbMail_user::OnReadItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbMail_user::OnDeleteItemReq(
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
	pDeleteItem->AddValue("/Key/mid/N", m_nMid);
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

int TbMail_user::OnDeleteItemReq(string& sPostData,
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

int TbMail_user::OnDeleteItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbMail_user::OnGetItemReq(unsigned int udwIdxNo,
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
	pGetItem->AddValue("/Key/mid/N", m_nMid);
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

int TbMail_user::OnGetItemReq(string& sPostData, unsigned int udwIdxNo,
	bool bConsistentRead, bool bReturnConsumedCapacity)
{
	AwsMap* pGetItem = OnGetItemReq(udwIdxNo, bConsistentRead, bReturnConsumedCapacity);
	ostringstream oss;
	pGetItem->Dump(oss);
	sPostData = oss.str();
	delete pGetItem;
	return 0;
}

int TbMail_user::OnGetItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbMail_user::OnPutItemReq(
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
	pPutItem->AddValue("/Item/mid/N", m_nMid);
	pPutItem->AddValue("/Item/suid/N", m_nSuid);
	pPutItem->AddValue("/Item/tuid/N", m_nTuid);
	pPutItem->AddValue("/Item/time/N", m_nTime);
	pPutItem->AddValue("/Item/status/N", m_nStatus);
	pPutItem->AddValue("/Item/display_type/N", m_nDisplay_type);
	pPutItem->AddValue("/Item/receiver_avatar/N", m_nReceiver_avatar);
	if (!m_sReceiver_name.empty())
	{
		pPutItem->AddValue("/Item/receiver_name/S", JsonEncode(m_sReceiver_name, sJsonEncode));
	}
	pPutItem->AddValue("/Item/has_reward/N", m_nHas_reward);
	if (!m_sReceiver_alnick.empty())
	{
		pPutItem->AddValue("/Item/receiver_alnick/S", JsonEncode(m_sReceiver_alnick, sJsonEncode));
	}
	pPutItem->AddValue("/Item/is_al/N", m_nIs_al);
	pPutItem->AddValue("/Item/is_single_svr/N", m_nIs_single_svr);
	pPutItem->AddValue("/Item/sid/N", m_nSid);
	if (!m_sVersion.empty())
	{
		pPutItem->AddValue("/Item/version/S", JsonEncode(m_sVersion, sJsonEncode));
	}
	if (!m_sPlatform.empty())
	{
		pPutItem->AddValue("/Item/platform/S", JsonEncode(m_sPlatform, sJsonEncode));
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

int TbMail_user::OnPutItemReq(string& sPostData,
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

int TbMail_user::OnPutItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

int TbMail_user::OnResponse(const Json::Value& item)
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
		if (vecMembers[i] == "mid")
		{
			m_nMid = strtoll(item["mid"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "suid")
		{
			m_nSuid = strtoll(item["suid"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "tuid")
		{
			m_nTuid = strtoll(item["tuid"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "time")
		{
			m_nTime = strtoll(item["time"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "status")
		{
			m_nStatus = strtoll(item["status"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "display_type")
		{
			m_nDisplay_type = strtoll(item["display_type"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "receiver_avatar")
		{
			m_nReceiver_avatar = strtoll(item["receiver_avatar"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "receiver_name")
		{
			m_sReceiver_name = item["receiver_name"]["S"].asString();
			continue;
		}
		if (vecMembers[i] == "has_reward")
		{
			m_nHas_reward = strtoll(item["has_reward"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "receiver_alnick")
		{
			m_sReceiver_alnick = item["receiver_alnick"]["S"].asString();
			continue;
		}
		if (vecMembers[i] == "is_al")
		{
			m_nIs_al = strtoll(item["is_al"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "is_single_svr")
		{
			m_nIs_single_svr = strtoll(item["is_single_svr"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "sid")
		{
			m_nSid = strtoll(item["sid"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "version")
		{
			m_sVersion = item["version"]["S"].asString();
			continue;
		}
		if (vecMembers[i] == "platform")
		{
			m_sPlatform = item["platform"]["S"].asString();
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

TINT64 TbMail_user::GetSeq()
{
	return m_nSeq;
}

