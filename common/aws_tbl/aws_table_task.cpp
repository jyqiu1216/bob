#include "aws_table_task.h"

TableDesc TbTask::oTableDesc;

int TbTask::Init(const string& sConfFile, const string strProjectName)
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

AwsTable* TbTask::NewObject()
{
	return new TbTask;
}

string TbTask::GetTableName()
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

TINT32 TbTask::GetTableIdx()
{
	 return 0;
}

AwsMap* TbTask::OnScanReq(unsigned int udwIdxNo, bool bHasStartKey, bool bReturnConsumedCapacity,
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

int TbTask::OnScanReq(string& sPostData, unsigned int udwIdxNo, bool bHasStartKey, bool bReturnConsumedCapacity,
 unsigned int dwLimit, unsigned int dwSegment, unsigned int dwTotalSegments)
{
	AwsMap* pScan = OnScanReq(udwIdxNo, bHasStartKey, bReturnConsumedCapacity, dwLimit, dwSegment, dwTotalSegments);
	ostringstream oss;
	pScan->Dump(oss);
	sPostData = oss.str();
	delete pScan;
	return 0;
}

int TbTask::OnScanRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbTask::OnQueryReq(unsigned int udwIdxNo, const CompareDesc& comp_desc,
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
			if(fld_desc.udwFldNo == TbTASK_FIELD_UID)
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

int TbTask::OnQueryReq(string& sPostData, unsigned int udwIdxNo, const CompareDesc& comp_desc,
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

int TbTask::OnQueryRsp(const Json::Value& item)
{
	return OnResponse(item);
}

int TbTask::OnCountReq(string& sPostData, unsigned int udwIdxNo, const CompareDesc& comp_desc, 
		bool bConsistentRead, bool bReturnConsumedCapacity, bool bScanIndexForward, unsigned int dwLimit)
{
	AwsMap* pQuery = OnQueryReq(udwIdxNo, comp_desc, bConsistentRead, bReturnConsumedCapacity, bScanIndexForward, dwLimit, true);
	ostringstream oss;
	pQuery->Dump(oss);
	sPostData = oss.str();
	delete pQuery;
	return 0;
}

AwsMap* TbTask::OnUpdateItemReq(
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
		if (TbTASK_FIELD_TASK_REFRESH_TIME == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/task_refresh_time/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/task_refresh_time/Value/N", m_nTask_refresh_time);
				pUpdateItem->AddValue("/AttributeUpdates/task_refresh_time/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbTASK_FIELD_TASK_FINISH == iter->first)
		{
			if (!m_bTask_finish.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/task_finish/Value/B", Base64Encode((char*)&m_bTask_finish.m_astList[0], m_bTask_finish.m_udwNum*sizeof(STaskFinsh), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/task_finish/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/task_finish/Action", "DELETE");
			}
			continue;
		}
		if (TbTASK_FIELD_TASK_NORMAL == iter->first)
		{
			if (!m_bTask_normal.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/task_normal/Value/B", Base64Encode((char*)&m_bTask_normal.m_astList[0], m_bTask_normal.m_udwNum*sizeof(STaskNodeNow), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/task_normal/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/task_normal/Action", "DELETE");
			}
			continue;
		}
		if (TbTASK_FIELD_TASK_TIME == iter->first)
		{
			if (!m_bTask_time.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/task_time/Value/B", Base64Encode((char*)&m_bTask_time.m_astList[0], m_bTask_time.m_udwNum*sizeof(STaskNodeNow), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/task_time/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/task_time/Action", "DELETE");
			}
			continue;
		}
		if (TbTASK_FIELD_TASK_STATUS == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/task_status/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/task_status/Value/N", m_nTask_status);
				pUpdateItem->AddValue("/AttributeUpdates/task_status/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbTASK_FIELD_TASK_OPEN_WINDOW_FLAG == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/task_open_window_flag/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/task_open_window_flag/Value/N", m_nTask_open_window_flag);
				pUpdateItem->AddValue("/AttributeUpdates/task_open_window_flag/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbTASK_FIELD_TASK_CHECK_ID == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/task_check_id/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/task_check_id/Value/N", m_nTask_check_id);
				pUpdateItem->AddValue("/AttributeUpdates/task_check_id/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbTASK_FIELD_RECOMMAND_TASK_ID == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/recommand_task_id/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/recommand_task_id/Value/N", m_nRecommand_task_id);
				pUpdateItem->AddValue("/AttributeUpdates/recommand_task_id/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbTASK_FIELD_SEQ == iter->first)
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

int TbTask::OnUpdateItemReq(string& sPostData,
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

int TbTask::OnUpdateItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbTask::OnWriteItemReq(int dwActionType)
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
		pItem->AddValue("/task_refresh_time/N", m_nTask_refresh_time);
		if (!m_bTask_finish.empty())
		{
			pItem->AddValue("/task_finish/B", Base64Encode((char*)&m_bTask_finish.m_astList[0], m_bTask_finish.m_udwNum*sizeof(STaskFinsh), sBase64Encode));
		}
		if (!m_bTask_normal.empty())
		{
			pItem->AddValue("/task_normal/B", Base64Encode((char*)&m_bTask_normal.m_astList[0], m_bTask_normal.m_udwNum*sizeof(STaskNodeNow), sBase64Encode));
		}
		if (!m_bTask_time.empty())
		{
			pItem->AddValue("/task_time/B", Base64Encode((char*)&m_bTask_time.m_astList[0], m_bTask_time.m_udwNum*sizeof(STaskNodeNow), sBase64Encode));
		}
		pItem->AddValue("/task_status/N", m_nTask_status);
		pItem->AddValue("/task_open_window_flag/N", m_nTask_open_window_flag);
		pItem->AddValue("/task_check_id/N", m_nTask_check_id);
		pItem->AddValue("/recommand_task_id/N", m_nRecommand_task_id);
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

void TbTask::OnWriteItemReq(AwsMap* pWriteItem, int dwActionType)
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
		pItem->AddValue("/task_refresh_time/N", m_nTask_refresh_time);
		if (!m_bTask_finish.empty())
		{
			pItem->AddValue("/task_finish/B", Base64Encode((char*)&m_bTask_finish.m_astList[0], m_bTask_finish.m_udwNum*sizeof(STaskFinsh), sBase64Encode));
		}
		if (!m_bTask_normal.empty())
		{
			pItem->AddValue("/task_normal/B", Base64Encode((char*)&m_bTask_normal.m_astList[0], m_bTask_normal.m_udwNum*sizeof(STaskNodeNow), sBase64Encode));
		}
		if (!m_bTask_time.empty())
		{
			pItem->AddValue("/task_time/B", Base64Encode((char*)&m_bTask_time.m_astList[0], m_bTask_time.m_udwNum*sizeof(STaskNodeNow), sBase64Encode));
		}
		pItem->AddValue("/task_status/N", m_nTask_status);
		pItem->AddValue("/task_open_window_flag/N", m_nTask_open_window_flag);
		pItem->AddValue("/task_check_id/N", m_nTask_check_id);
		pItem->AddValue("/recommand_task_id/N", m_nRecommand_task_id);
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

AwsMap* TbTask::OnReadItemReq(unsigned int udwIdxNo)
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

void TbTask::OnReadItemReq(AwsMap* pReadItem)
{
	assert(pReadItem);
	AwsList* pKeys = pReadItem->GetAwsMap("RequestItems")->GetAwsMap(GetTableName())->GetAwsList("Keys");
	AwsMap* pKey = new AwsMap;
	assert(pKey);
	pKey->AddValue("/uid/N", m_nUid);
	pKeys->SetValue(pKey, true);
}

int TbTask::OnReadItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbTask::OnDeleteItemReq(
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

int TbTask::OnDeleteItemReq(string& sPostData,
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

int TbTask::OnDeleteItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbTask::OnGetItemReq(unsigned int udwIdxNo,
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

int TbTask::OnGetItemReq(string& sPostData, unsigned int udwIdxNo,
	bool bConsistentRead, bool bReturnConsumedCapacity)
{
	AwsMap* pGetItem = OnGetItemReq(udwIdxNo, bConsistentRead, bReturnConsumedCapacity);
	ostringstream oss;
	pGetItem->Dump(oss);
	sPostData = oss.str();
	delete pGetItem;
	return 0;
}

int TbTask::OnGetItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbTask::OnPutItemReq(
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
	pPutItem->AddValue("/Item/task_refresh_time/N", m_nTask_refresh_time);
	if (!m_bTask_finish.empty())
	{
		pPutItem->AddValue("/Item/task_finish/B", Base64Encode((char*)&m_bTask_finish.m_astList[0], m_bTask_finish.m_udwNum*sizeof(STaskFinsh), sBase64Encode));
	}
	if (!m_bTask_normal.empty())
	{
		pPutItem->AddValue("/Item/task_normal/B", Base64Encode((char*)&m_bTask_normal.m_astList[0], m_bTask_normal.m_udwNum*sizeof(STaskNodeNow), sBase64Encode));
	}
	if (!m_bTask_time.empty())
	{
		pPutItem->AddValue("/Item/task_time/B", Base64Encode((char*)&m_bTask_time.m_astList[0], m_bTask_time.m_udwNum*sizeof(STaskNodeNow), sBase64Encode));
	}
	pPutItem->AddValue("/Item/task_status/N", m_nTask_status);
	pPutItem->AddValue("/Item/task_open_window_flag/N", m_nTask_open_window_flag);
	pPutItem->AddValue("/Item/task_check_id/N", m_nTask_check_id);
	pPutItem->AddValue("/Item/recommand_task_id/N", m_nRecommand_task_id);
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

int TbTask::OnPutItemReq(string& sPostData,
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

int TbTask::OnPutItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

int TbTask::OnResponse(const Json::Value& item)
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
		if (vecMembers[i] == "task_refresh_time")
		{
			m_nTask_refresh_time = strtoll(item["task_refresh_time"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "task_finish")
		{
			Base64Decode(item["task_finish"]["B"].asString(), (char*)&m_bTask_finish.m_astList[0], dwResLen);
			m_bTask_finish.m_udwNum = dwResLen/sizeof(STaskFinsh);
			if (0==m_bTask_finish.m_udwNum && dwResLen>0)
			{
				m_bTask_finish.m_udwNum = 1;
			}
			continue;
		}
		if (vecMembers[i] == "task_normal")
		{
			Base64Decode(item["task_normal"]["B"].asString(), (char*)&m_bTask_normal.m_astList[0], dwResLen);
			m_bTask_normal.m_udwNum = dwResLen/sizeof(STaskNodeNow);
			continue;
		}
		if (vecMembers[i] == "task_time")
		{
			Base64Decode(item["task_time"]["B"].asString(), (char*)&m_bTask_time.m_astList[0], dwResLen);
			m_bTask_time.m_udwNum = dwResLen/sizeof(STaskNodeNow);
			continue;
		}
		if (vecMembers[i] == "task_status")
		{
			m_nTask_status = strtoll(item["task_status"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "task_open_window_flag")
		{
			m_nTask_open_window_flag = strtoll(item["task_open_window_flag"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "task_check_id")
		{
			m_nTask_check_id = strtoll(item["task_check_id"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "recommand_task_id")
		{
			m_nRecommand_task_id = strtoll(item["recommand_task_id"]["N"].asString().c_str(), NULL, 10);
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

TINT64 TbTask::GetSeq()
{
	return m_nSeq;
}

