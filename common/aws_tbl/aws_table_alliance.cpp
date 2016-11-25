#include "aws_table_alliance.h"

TableDesc TbAlliance::oTableDesc;

int TbAlliance::Init(const string& sConfFile, const string strProjectName)
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

AwsTable* TbAlliance::NewObject()
{
	return new TbAlliance;
}

string TbAlliance::GetTableName()
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

TINT32 TbAlliance::GetTableIdx()
{
	 return 0;
}

AwsMap* TbAlliance::OnScanReq(unsigned int udwIdxNo, bool bHasStartKey, bool bReturnConsumedCapacity,
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
	}
	return pScan;
}

int TbAlliance::OnScanReq(string& sPostData, unsigned int udwIdxNo, bool bHasStartKey, bool bReturnConsumedCapacity,
 unsigned int dwLimit, unsigned int dwSegment, unsigned int dwTotalSegments)
{
	AwsMap* pScan = OnScanReq(udwIdxNo, bHasStartKey, bReturnConsumedCapacity, dwLimit, dwSegment, dwTotalSegments);
	ostringstream oss;
	pScan->Dump(oss);
	sPostData = oss.str();
	delete pScan;
	return 0;
}

int TbAlliance::OnScanRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbAlliance::OnQueryReq(unsigned int udwIdxNo, const CompareDesc& comp_desc,
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
			if(fld_desc.udwFldNo == TbALLIANCE_FIELD_AID)
			{
				pQuery->AddValue("/KeyConditions/aid/AttributeValueList[0]/N", m_nAid);
				pQuery->AddValue("/KeyConditions/aid/ComparisonOperator", "EQ");
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

int TbAlliance::OnQueryReq(string& sPostData, unsigned int udwIdxNo, const CompareDesc& comp_desc,
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

int TbAlliance::OnQueryRsp(const Json::Value& item)
{
	return OnResponse(item);
}

int TbAlliance::OnCountReq(string& sPostData, unsigned int udwIdxNo, const CompareDesc& comp_desc, 
		bool bConsistentRead, bool bReturnConsumedCapacity, bool bScanIndexForward, unsigned int dwLimit)
{
	AwsMap* pQuery = OnQueryReq(udwIdxNo, comp_desc, bConsistentRead, bReturnConsumedCapacity, bScanIndexForward, dwLimit, true);
	ostringstream oss;
	pQuery->Dump(oss);
	sPostData = oss.str();
	delete pQuery;
	return 0;
}

AwsMap* TbAlliance::OnUpdateItemReq(
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
	for (map<unsigned int, int>::iterator iter = m_mFlag.begin(); iter != m_mFlag.end(); ++iter)
	{
		bUpdateFlag = true;
		if (TbALLIANCE_FIELD_NAME == iter->first)
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
		if (TbALLIANCE_FIELD_OID == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/oid/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/oid/Value/N", m_nOid);
				pUpdateItem->AddValue("/AttributeUpdates/oid/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbALLIANCE_FIELD_ONAME == iter->first)
		{
			if (!m_sOname.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/oname/Value/S", JsonEncode(m_sOname, sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/oname/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/oname/Action", "DELETE");
			}
			continue;
		}
		if (TbALLIANCE_FIELD_MEMBER == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/member/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/member/Value/N", m_nMember);
				pUpdateItem->AddValue("/AttributeUpdates/member/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbALLIANCE_FIELD_MIGHT == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/might/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/might/Value/N", m_nMight);
				pUpdateItem->AddValue("/AttributeUpdates/might/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbALLIANCE_FIELD_DESC == iter->first)
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
		if (TbALLIANCE_FIELD_NOTICE == iter->first)
		{
			if (!m_sNotice.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/notice/Value/S", JsonEncode(m_sNotice, sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/notice/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/notice/Action", "DELETE");
			}
			continue;
		}
		if (TbALLIANCE_FIELD_SID == iter->first)
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
		if (TbALLIANCE_FIELD_POLICY == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/policy/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/policy/Value/N", m_nPolicy);
				pUpdateItem->AddValue("/AttributeUpdates/policy/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbALLIANCE_FIELD_GIFT_POINT == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/gift_point/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/gift_point/Value/N", m_nGift_point);
				pUpdateItem->AddValue("/AttributeUpdates/gift_point/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbALLIANCE_FIELD_LANGUAGE == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/language/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/language/Value/N", m_nLanguage);
				pUpdateItem->AddValue("/AttributeUpdates/language/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbALLIANCE_FIELD_FUND == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/fund/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/fund/Value/N", m_nFund);
				pUpdateItem->AddValue("/AttributeUpdates/fund/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbALLIANCE_FIELD_AL_STORE_ITEM == iter->first)
		{
			if (!m_bAl_store_item.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/al_store_item/Value/B", Base64Encode((char*)&m_bAl_store_item.m_astList[0], m_bAl_store_item.m_udwNum*sizeof(SAlStoreItem), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/al_store_item/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/al_store_item/Action", "DELETE");
			}
			continue;
		}
		if (TbALLIANCE_FIELD_FORCE_KILL == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/force_kill/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/force_kill/Value/N", m_nForce_kill);
				pUpdateItem->AddValue("/AttributeUpdates/force_kill/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbALLIANCE_FIELD_AVATAR == iter->first)
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
		if (TbALLIANCE_FIELD_AL_NICK_NAME == iter->first)
		{
			if (!m_sAl_nick_name.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/al_nick_name/Value/S", JsonEncode(m_sAl_nick_name, sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/al_nick_name/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/al_nick_name/Action", "DELETE");
			}
			continue;
		}
		if (TbALLIANCE_FIELD_THRONE_POS == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/throne_pos/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/throne_pos/Value/N", m_nThrone_pos);
				pUpdateItem->AddValue("/AttributeUpdates/throne_pos/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbALLIANCE_FIELD_THRONE_STATUS == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/throne_status/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/throne_status/Value/N", m_nThrone_status);
				pUpdateItem->AddValue("/AttributeUpdates/throne_status/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbALLIANCE_FIELD_IS_NPC == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/is_npc/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/is_npc/Value/N", m_nIs_npc);
				pUpdateItem->AddValue("/AttributeUpdates/is_npc/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbALLIANCE_FIELD_MAX_GEM_BUY == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/max_gem_buy/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/max_gem_buy/Value/N", m_nMax_gem_buy);
				pUpdateItem->AddValue("/AttributeUpdates/max_gem_buy/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbALLIANCE_FIELD_AL_STAR == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/al_star/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/al_star/Value/N", m_nAl_star);
				pUpdateItem->AddValue("/AttributeUpdates/al_star/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbALLIANCE_FIELD_HIVE_POS == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/hive_pos/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/hive_pos/Value/N", m_nHive_pos);
				pUpdateItem->AddValue("/AttributeUpdates/hive_pos/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbALLIANCE_FIELD_HIVE_SHOW_FLAG == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/hive_show_flag/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/hive_show_flag/Value/N", m_nHive_show_flag);
				pUpdateItem->AddValue("/AttributeUpdates/hive_show_flag/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbALLIANCE_FIELD_HIVE_SVR == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/hive_svr/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/hive_svr/Value/N", m_nHive_svr);
				pUpdateItem->AddValue("/AttributeUpdates/hive_svr/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbALLIANCE_FIELD_SEQ == iter->first)
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
		if (TbALLIANCE_FIELD_HAS_OCCUPIED_TIME == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/has_occupied_time/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/has_occupied_time/Value/N", m_nHas_occupied_time);
				pUpdateItem->AddValue("/AttributeUpdates/has_occupied_time/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbALLIANCE_FIELD_LAST_OCCUPY_TIME == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/last_occupy_time/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/last_occupy_time/Value/N", m_nLast_occupy_time);
				pUpdateItem->AddValue("/AttributeUpdates/last_occupy_time/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbALLIANCE_FIELD_OWNER_CID == iter->first)
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

int TbAlliance::OnUpdateItemReq(string& sPostData,
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

int TbAlliance::OnUpdateItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbAlliance::OnWriteItemReq(int dwActionType)
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
		pItem->AddValue("/aid/N", m_nAid);
		if (!m_sName.empty())
		{
			pItem->AddValue("/name/S", JsonEncode(m_sName, sJsonEncode));
		}
		pItem->AddValue("/oid/N", m_nOid);
		if (!m_sOname.empty())
		{
			pItem->AddValue("/oname/S", JsonEncode(m_sOname, sJsonEncode));
		}
		pItem->AddValue("/member/N", m_nMember);
		pItem->AddValue("/might/N", m_nMight);
		if (!m_sDesc.empty())
		{
			pItem->AddValue("/desc/S", JsonEncode(m_sDesc, sJsonEncode));
		}
		if (!m_sNotice.empty())
		{
			pItem->AddValue("/notice/S", JsonEncode(m_sNotice, sJsonEncode));
		}
		pItem->AddValue("/sid/N", m_nSid);
		pItem->AddValue("/policy/N", m_nPolicy);
		pItem->AddValue("/gift_point/N", m_nGift_point);
		pItem->AddValue("/language/N", m_nLanguage);
		pItem->AddValue("/fund/N", m_nFund);
		if (!m_bAl_store_item.empty())
		{
			pItem->AddValue("/al_store_item/B", Base64Encode((char*)&m_bAl_store_item.m_astList[0], m_bAl_store_item.m_udwNum*sizeof(SAlStoreItem), sBase64Encode));
		}
		pItem->AddValue("/force_kill/N", m_nForce_kill);
		pItem->AddValue("/avatar/N", m_nAvatar);
		if (!m_sAl_nick_name.empty())
		{
			pItem->AddValue("/al_nick_name/S", JsonEncode(m_sAl_nick_name, sJsonEncode));
		}
		pItem->AddValue("/throne_pos/N", m_nThrone_pos);
		pItem->AddValue("/throne_status/N", m_nThrone_status);
		pItem->AddValue("/is_npc/N", m_nIs_npc);
		pItem->AddValue("/max_gem_buy/N", m_nMax_gem_buy);
		pItem->AddValue("/al_star/N", m_nAl_star);
		pItem->AddValue("/hive_pos/N", m_nHive_pos);
		pItem->AddValue("/hive_show_flag/N", m_nHive_show_flag);
		pItem->AddValue("/hive_svr/N", m_nHive_svr);
		pItem->AddValue("/seq/N", m_nSeq);
		pItem->AddValue("/has_occupied_time/N", m_nHas_occupied_time);
		pItem->AddValue("/last_occupy_time/N", m_nLast_occupy_time);
		pItem->AddValue("/owner_cid/N", m_nOwner_cid);
	}
	else if (WRITE_ACTION_TYPE_DELETE == dwActionType)
	{
		pItem = pWriteItem->GetAwsMap("DeleteRequest")->GetAwsMap("Key");
		pItem->AddValue("/aid/N", m_nAid);
	}
	else
	{
		assert(0);
	}
	return pWriteItem;
}

void TbAlliance::OnWriteItemReq(AwsMap* pWriteItem, int dwActionType)
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
		pItem->AddValue("/aid/N", m_nAid);
		if (!m_sName.empty())
		{
			pItem->AddValue("/name/S", JsonEncode(m_sName, sJsonEncode));
		}
		pItem->AddValue("/oid/N", m_nOid);
		if (!m_sOname.empty())
		{
			pItem->AddValue("/oname/S", JsonEncode(m_sOname, sJsonEncode));
		}
		pItem->AddValue("/member/N", m_nMember);
		pItem->AddValue("/might/N", m_nMight);
		if (!m_sDesc.empty())
		{
			pItem->AddValue("/desc/S", JsonEncode(m_sDesc, sJsonEncode));
		}
		if (!m_sNotice.empty())
		{
			pItem->AddValue("/notice/S", JsonEncode(m_sNotice, sJsonEncode));
		}
		pItem->AddValue("/sid/N", m_nSid);
		pItem->AddValue("/policy/N", m_nPolicy);
		pItem->AddValue("/gift_point/N", m_nGift_point);
		pItem->AddValue("/language/N", m_nLanguage);
		pItem->AddValue("/fund/N", m_nFund);
		if (!m_bAl_store_item.empty())
		{
			pItem->AddValue("/al_store_item/B", Base64Encode((char*)&m_bAl_store_item.m_astList[0], m_bAl_store_item.m_udwNum*sizeof(SAlStoreItem), sBase64Encode));
		}
		pItem->AddValue("/force_kill/N", m_nForce_kill);
		pItem->AddValue("/avatar/N", m_nAvatar);
		if (!m_sAl_nick_name.empty())
		{
			pItem->AddValue("/al_nick_name/S", JsonEncode(m_sAl_nick_name, sJsonEncode));
		}
		pItem->AddValue("/throne_pos/N", m_nThrone_pos);
		pItem->AddValue("/throne_status/N", m_nThrone_status);
		pItem->AddValue("/is_npc/N", m_nIs_npc);
		pItem->AddValue("/max_gem_buy/N", m_nMax_gem_buy);
		pItem->AddValue("/al_star/N", m_nAl_star);
		pItem->AddValue("/hive_pos/N", m_nHive_pos);
		pItem->AddValue("/hive_show_flag/N", m_nHive_show_flag);
		pItem->AddValue("/hive_svr/N", m_nHive_svr);
		pItem->AddValue("/seq/N", m_nSeq);
		pItem->AddValue("/has_occupied_time/N", m_nHas_occupied_time);
		pItem->AddValue("/last_occupy_time/N", m_nLast_occupy_time);
		pItem->AddValue("/owner_cid/N", m_nOwner_cid);
	}
	else if (WRITE_ACTION_TYPE_DELETE == dwActionType)
	{
		pItem = pReqItem->GetAwsMap("DeleteRequest")->GetAwsMap("Key");
		pItem->AddValue("/aid/N", m_nAid);
	}
	else
	{
		assert(0);
	}
	pWriteItem->GetAwsMap("RequestItems")->GetAwsList(GetTableName())->SetValue(pReqItem, true);
}

AwsMap* TbAlliance::OnReadItemReq(unsigned int udwIdxNo)
{
	oJsonWriter.omitEndingLineFeed();
	IndexDesc& idx_desc = oTableDesc.mIndexDesc[udwIdxNo];
	assert(idx_desc.sName == "PRIMARY"); //只能通过主键查询
	AwsMap* pReadItem = new AwsMap;
	assert(pReadItem);
	pReadItem->AddValue("/Keys[0]/aid/N", m_nAid);
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

void TbAlliance::OnReadItemReq(AwsMap* pReadItem)
{
	assert(pReadItem);
	AwsList* pKeys = pReadItem->GetAwsMap("RequestItems")->GetAwsMap(GetTableName())->GetAwsList("Keys");
	AwsMap* pKey = new AwsMap;
	assert(pKey);
	pKey->AddValue("/aid/N", m_nAid);
	pKeys->SetValue(pKey, true);
}

int TbAlliance::OnReadItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbAlliance::OnDeleteItemReq(
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

int TbAlliance::OnDeleteItemReq(string& sPostData,
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

int TbAlliance::OnDeleteItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbAlliance::OnGetItemReq(unsigned int udwIdxNo,
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

int TbAlliance::OnGetItemReq(string& sPostData, unsigned int udwIdxNo,
	bool bConsistentRead, bool bReturnConsumedCapacity)
{
	AwsMap* pGetItem = OnGetItemReq(udwIdxNo, bConsistentRead, bReturnConsumedCapacity);
	ostringstream oss;
	pGetItem->Dump(oss);
	sPostData = oss.str();
	delete pGetItem;
	return 0;
}

int TbAlliance::OnGetItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbAlliance::OnPutItemReq(
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
	pPutItem->AddValue("/Item/aid/N", m_nAid);
	if (!m_sName.empty())
	{
		pPutItem->AddValue("/Item/name/S", JsonEncode(m_sName, sJsonEncode));
	}
	pPutItem->AddValue("/Item/oid/N", m_nOid);
	if (!m_sOname.empty())
	{
		pPutItem->AddValue("/Item/oname/S", JsonEncode(m_sOname, sJsonEncode));
	}
	pPutItem->AddValue("/Item/member/N", m_nMember);
	pPutItem->AddValue("/Item/might/N", m_nMight);
	if (!m_sDesc.empty())
	{
		pPutItem->AddValue("/Item/desc/S", JsonEncode(m_sDesc, sJsonEncode));
	}
	if (!m_sNotice.empty())
	{
		pPutItem->AddValue("/Item/notice/S", JsonEncode(m_sNotice, sJsonEncode));
	}
	pPutItem->AddValue("/Item/sid/N", m_nSid);
	pPutItem->AddValue("/Item/policy/N", m_nPolicy);
	pPutItem->AddValue("/Item/gift_point/N", m_nGift_point);
	pPutItem->AddValue("/Item/language/N", m_nLanguage);
	pPutItem->AddValue("/Item/fund/N", m_nFund);
	if (!m_bAl_store_item.empty())
	{
		pPutItem->AddValue("/Item/al_store_item/B", Base64Encode((char*)&m_bAl_store_item.m_astList[0], m_bAl_store_item.m_udwNum*sizeof(SAlStoreItem), sBase64Encode));
	}
	pPutItem->AddValue("/Item/force_kill/N", m_nForce_kill);
	pPutItem->AddValue("/Item/avatar/N", m_nAvatar);
	if (!m_sAl_nick_name.empty())
	{
		pPutItem->AddValue("/Item/al_nick_name/S", JsonEncode(m_sAl_nick_name, sJsonEncode));
	}
	pPutItem->AddValue("/Item/throne_pos/N", m_nThrone_pos);
	pPutItem->AddValue("/Item/throne_status/N", m_nThrone_status);
	pPutItem->AddValue("/Item/is_npc/N", m_nIs_npc);
	pPutItem->AddValue("/Item/max_gem_buy/N", m_nMax_gem_buy);
	pPutItem->AddValue("/Item/al_star/N", m_nAl_star);
	pPutItem->AddValue("/Item/hive_pos/N", m_nHive_pos);
	pPutItem->AddValue("/Item/hive_show_flag/N", m_nHive_show_flag);
	pPutItem->AddValue("/Item/hive_svr/N", m_nHive_svr);
	pPutItem->AddValue("/Item/seq/N", m_nSeq);
	pPutItem->AddValue("/Item/has_occupied_time/N", m_nHas_occupied_time);
	pPutItem->AddValue("/Item/last_occupy_time/N", m_nLast_occupy_time);
	pPutItem->AddValue("/Item/owner_cid/N", m_nOwner_cid);
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

int TbAlliance::OnPutItemReq(string& sPostData,
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

int TbAlliance::OnPutItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

int TbAlliance::OnResponse(const Json::Value& item)
{
	oJsonWriter.omitEndingLineFeed();
	int dwResLen = 0;
	dwResLen = 0;
	this->Reset();
	Json::Value::Members vecMembers = item.getMemberNames();
	for (unsigned int i = 0; i < vecMembers.size(); ++i)
	{
 		if (vecMembers[i] == "aid")
		{
			m_nAid = strtoll(item["aid"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "name")
		{
			m_sName = item["name"]["S"].asString();
			continue;
		}
		if (vecMembers[i] == "oid")
		{
			m_nOid = strtoll(item["oid"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "oname")
		{
			m_sOname = item["oname"]["S"].asString();
			continue;
		}
		if (vecMembers[i] == "member")
		{
			m_nMember = strtoll(item["member"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "might")
		{
			m_nMight = strtoll(item["might"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "desc")
		{
			m_sDesc = item["desc"]["S"].asString();
			continue;
		}
		if (vecMembers[i] == "notice")
		{
			m_sNotice = item["notice"]["S"].asString();
			continue;
		}
		if (vecMembers[i] == "sid")
		{
			m_nSid = strtoll(item["sid"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "policy")
		{
			m_nPolicy = strtoll(item["policy"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "gift_point")
		{
			m_nGift_point = strtoll(item["gift_point"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "language")
		{
			m_nLanguage = strtoll(item["language"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "fund")
		{
			m_nFund = strtoll(item["fund"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "al_store_item")
		{
			Base64Decode(item["al_store_item"]["B"].asString(), (char*)&m_bAl_store_item.m_astList[0], dwResLen);
			m_bAl_store_item.m_udwNum = dwResLen/sizeof(SAlStoreItem);
			continue;
		}
		if (vecMembers[i] == "force_kill")
		{
			m_nForce_kill = strtoll(item["force_kill"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "avatar")
		{
			m_nAvatar = strtoll(item["avatar"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "al_nick_name")
		{
			m_sAl_nick_name = item["al_nick_name"]["S"].asString();
			continue;
		}
		if (vecMembers[i] == "throne_pos")
		{
			m_nThrone_pos = strtoll(item["throne_pos"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "throne_status")
		{
			m_nThrone_status = strtoll(item["throne_status"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "is_npc")
		{
			m_nIs_npc = strtoll(item["is_npc"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "max_gem_buy")
		{
			m_nMax_gem_buy = strtoll(item["max_gem_buy"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "al_star")
		{
			m_nAl_star = strtoll(item["al_star"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "hive_pos")
		{
			m_nHive_pos = strtoll(item["hive_pos"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "hive_show_flag")
		{
			m_nHive_show_flag = strtoll(item["hive_show_flag"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "hive_svr")
		{
			m_nHive_svr = strtoll(item["hive_svr"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "seq")
		{
			m_nSeq = strtoll(item["seq"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "has_occupied_time")
		{
			m_nHas_occupied_time = strtoll(item["has_occupied_time"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "last_occupy_time")
		{
			m_nLast_occupy_time = strtoll(item["last_occupy_time"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "owner_cid")
		{
			m_nOwner_cid = strtoll(item["owner_cid"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
	}
	return 0;
}

TINT64 TbAlliance::GetSeq()
{
	return m_nSeq;
}

