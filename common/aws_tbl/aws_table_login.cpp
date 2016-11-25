#include "aws_table_login.h"

TableDesc TbLogin::oTableDesc;

int TbLogin::Init(const string& sConfFile, const string strProjectName)
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

AwsTable* TbLogin::NewObject()
{
	return new TbLogin;
}

string TbLogin::GetTableName()
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

TINT32 TbLogin::GetTableIdx()
{
	 return 0;
}

AwsMap* TbLogin::OnScanReq(unsigned int udwIdxNo, bool bHasStartKey, bool bReturnConsumedCapacity,
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

int TbLogin::OnScanReq(string& sPostData, unsigned int udwIdxNo, bool bHasStartKey, bool bReturnConsumedCapacity,
 unsigned int dwLimit, unsigned int dwSegment, unsigned int dwTotalSegments)
{
	AwsMap* pScan = OnScanReq(udwIdxNo, bHasStartKey, bReturnConsumedCapacity, dwLimit, dwSegment, dwTotalSegments);
	ostringstream oss;
	pScan->Dump(oss);
	sPostData = oss.str();
	delete pScan;
	return 0;
}

int TbLogin::OnScanRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbLogin::OnQueryReq(unsigned int udwIdxNo, const CompareDesc& comp_desc,
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
			if(fld_desc.udwFldNo == TbLOGIN_FIELD_UID)
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

int TbLogin::OnQueryReq(string& sPostData, unsigned int udwIdxNo, const CompareDesc& comp_desc,
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

int TbLogin::OnQueryRsp(const Json::Value& item)
{
	return OnResponse(item);
}

int TbLogin::OnCountReq(string& sPostData, unsigned int udwIdxNo, const CompareDesc& comp_desc, 
		bool bConsistentRead, bool bReturnConsumedCapacity, bool bScanIndexForward, unsigned int dwLimit)
{
	AwsMap* pQuery = OnQueryReq(udwIdxNo, comp_desc, bConsistentRead, bReturnConsumedCapacity, bScanIndexForward, dwLimit, true);
	ostringstream oss;
	pQuery->Dump(oss);
	sPostData = oss.str();
	delete pQuery;
	return 0;
}

AwsMap* TbLogin::OnUpdateItemReq(
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
		if (TbLOGIN_FIELD_SID == iter->first)
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
		if (TbLOGIN_FIELD_SEQ == iter->first)
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
		if (TbLOGIN_FIELD_DID == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/did/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/did/Value/N", m_nDid);
				pUpdateItem->AddValue("/AttributeUpdates/did/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbLOGIN_FIELD_CTIME == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/ctime/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/ctime/Value/N", m_nCtime);
				pUpdateItem->AddValue("/AttributeUpdates/ctime/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbLOGIN_FIELD_UTIME == iter->first)
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
		if (TbLOGIN_FIELD_GEM == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/gem/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/gem/Value/N", m_nGem);
				pUpdateItem->AddValue("/AttributeUpdates/gem/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbLOGIN_FIELD_GEM_SEQ == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/gem_seq/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/gem_seq/Value/N", m_nGem_seq);
				pUpdateItem->AddValue("/AttributeUpdates/gem_seq/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbLOGIN_FIELD_GEM_BUY == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/gem_buy/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/gem_buy/Value/N", m_nGem_buy);
				pUpdateItem->AddValue("/AttributeUpdates/gem_buy/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbLOGIN_FIELD_NPC == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/npc/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/npc/Value/N", m_nNpc);
				pUpdateItem->AddValue("/AttributeUpdates/npc/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbLOGIN_FIELD_DE == iter->first)
		{
			if (!m_sDe.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/de/Value/S", JsonEncode(m_sDe, sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/de/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/de/Action", "DELETE");
			}
			continue;
		}
		if (TbLOGIN_FIELD_APNS_TOKEN == iter->first)
		{
			if (!m_sApns_token.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/apns_token/Value/S", JsonEncode(m_sApns_token, sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/apns_token/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/apns_token/Action", "DELETE");
			}
			continue;
		}
		if (TbLOGIN_FIELD_APNS_NUM == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/apns_num/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/apns_num/Value/N", m_nApns_num);
				pUpdateItem->AddValue("/AttributeUpdates/apns_num/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbLOGIN_FIELD_AL_TIME == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/al_time/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/al_time/Value/N", m_nAl_time);
				pUpdateItem->AddValue("/AttributeUpdates/al_time/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbLOGIN_FIELD_LAST_LG_TIME == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/last_lg_time/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/last_lg_time/Value/N", m_nLast_lg_time);
				pUpdateItem->AddValue("/AttributeUpdates/last_lg_time/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbLOGIN_FIELD_IDFA == iter->first)
		{
			if (!m_sIdfa.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/idfa/Value/S", JsonEncode(m_sIdfa, sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/idfa/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/idfa/Action", "DELETE");
			}
			continue;
		}
		if (TbLOGIN_FIELD_GEM_COST == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/gem_cost/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/gem_cost/Value/N", m_nGem_cost);
				pUpdateItem->AddValue("/AttributeUpdates/gem_cost/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbLOGIN_FIELD_GUIDE_FLAG == iter->first)
		{
			if (!m_bGuide_flag.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/guide_flag/Value/B", Base64Encode((char*)&m_bGuide_flag.m_astList[0], m_bGuide_flag.m_udwNum*sizeof(SBitFlag), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/guide_flag/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/guide_flag/Action", "DELETE");
			}
			continue;
		}
		if (TbLOGIN_FIELD_PLATFORM == iter->first)
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
		if (TbLOGIN_FIELD_LANG == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/lang/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/lang/Value/N", m_nLang);
				pUpdateItem->AddValue("/AttributeUpdates/lang/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbLOGIN_FIELD_ABILITY == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/ability/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/ability/Value/N", m_nAbility);
				pUpdateItem->AddValue("/AttributeUpdates/ability/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbLOGIN_FIELD_RATING_TIME == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/rating_time/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/rating_time/Value/N", m_nRating_time);
				pUpdateItem->AddValue("/AttributeUpdates/rating_time/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbLOGIN_FIELD_WEEK_GEM_RECHARGE == iter->first)
		{
			if (!m_jWeek_gem_recharge.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/week_gem_recharge/Value/S", JsonEncode(oJsonWriter.write(m_jWeek_gem_recharge), sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/week_gem_recharge/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/week_gem_recharge/Action", "DELETE");
			}
			continue;
		}
		if (TbLOGIN_FIELD_APNS_SWITCH == iter->first)
		{
			if (!m_jApns_switch.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/apns_switch/Value/S", JsonEncode(oJsonWriter.write(m_jApns_switch), sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/apns_switch/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/apns_switch/Action", "DELETE");
			}
			continue;
		}
		if (TbLOGIN_FIELD_MAX_BUY == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/max_buy/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/max_buy/Value/N", m_nMax_buy);
				pUpdateItem->AddValue("/AttributeUpdates/max_buy/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbLOGIN_FIELD_LAST_BUY == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/last_buy/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/last_buy/Value/N", m_nLast_buy);
				pUpdateItem->AddValue("/AttributeUpdates/last_buy/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbLOGIN_FIELD_LAST_BUY_TIME == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/last_buy_time/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/last_buy_time/Value/N", m_nLast_buy_time);
				pUpdateItem->AddValue("/AttributeUpdates/last_buy_time/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbLOGIN_FIELD_TOTAL_PAY == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/total_pay/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/total_pay/Value/N", m_nTotal_pay);
				pUpdateItem->AddValue("/AttributeUpdates/total_pay/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbLOGIN_FIELD_MAX_PAY == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/max_pay/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/max_pay/Value/N", m_nMax_pay);
				pUpdateItem->AddValue("/AttributeUpdates/max_pay/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbLOGIN_FIELD_LAST_PAY == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/last_pay/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/last_pay/Value/N", m_nLast_pay);
				pUpdateItem->AddValue("/AttributeUpdates/last_pay/Action", UpdateActionType2Str(iter->second));
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

int TbLogin::OnUpdateItemReq(string& sPostData,
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

int TbLogin::OnUpdateItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbLogin::OnWriteItemReq(int dwActionType)
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
		pItem->AddValue("/uid/N", m_nUid);
		pItem->AddValue("/seq/N", m_nSeq);
		pItem->AddValue("/did/N", m_nDid);
		pItem->AddValue("/ctime/N", m_nCtime);
		pItem->AddValue("/utime/N", m_nUtime);
		pItem->AddValue("/gem/N", m_nGem);
		pItem->AddValue("/gem_seq/N", m_nGem_seq);
		pItem->AddValue("/gem_buy/N", m_nGem_buy);
		pItem->AddValue("/npc/N", m_nNpc);
		if (!m_sDe.empty())
		{
			pItem->AddValue("/de/S", JsonEncode(m_sDe, sJsonEncode));
		}
		if (!m_sApns_token.empty())
		{
			pItem->AddValue("/apns_token/S", JsonEncode(m_sApns_token, sJsonEncode));
		}
		pItem->AddValue("/apns_num/N", m_nApns_num);
		pItem->AddValue("/al_time/N", m_nAl_time);
		pItem->AddValue("/last_lg_time/N", m_nLast_lg_time);
		if (!m_sIdfa.empty())
		{
			pItem->AddValue("/idfa/S", JsonEncode(m_sIdfa, sJsonEncode));
		}
		pItem->AddValue("/gem_cost/N", m_nGem_cost);
		if (!m_bGuide_flag.empty())
		{
			pItem->AddValue("/guide_flag/B", Base64Encode((char*)&m_bGuide_flag.m_astList[0], m_bGuide_flag.m_udwNum*sizeof(SBitFlag), sBase64Encode));
		}
		if (!m_sPlatform.empty())
		{
			pItem->AddValue("/platform/S", JsonEncode(m_sPlatform, sJsonEncode));
		}
		pItem->AddValue("/lang/N", m_nLang);
		pItem->AddValue("/ability/N", m_nAbility);
		pItem->AddValue("/rating_time/N", m_nRating_time);
		if (!m_jWeek_gem_recharge.empty())
		{
				pItem->AddValue("/AttributeUpdates/week_gem_recharge/Value/S", JsonEncode(oJsonWriter.write(m_jWeek_gem_recharge), sJsonEncode));
		}
		if (!m_jApns_switch.empty())
		{
				pItem->AddValue("/AttributeUpdates/apns_switch/Value/S", JsonEncode(oJsonWriter.write(m_jApns_switch), sJsonEncode));
		}
		pItem->AddValue("/max_buy/N", m_nMax_buy);
		pItem->AddValue("/last_buy/N", m_nLast_buy);
		pItem->AddValue("/last_buy_time/N", m_nLast_buy_time);
		pItem->AddValue("/total_pay/N", m_nTotal_pay);
		pItem->AddValue("/max_pay/N", m_nMax_pay);
		pItem->AddValue("/last_pay/N", m_nLast_pay);
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

void TbLogin::OnWriteItemReq(AwsMap* pWriteItem, int dwActionType)
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
		pItem->AddValue("/uid/N", m_nUid);
		pItem->AddValue("/seq/N", m_nSeq);
		pItem->AddValue("/did/N", m_nDid);
		pItem->AddValue("/ctime/N", m_nCtime);
		pItem->AddValue("/utime/N", m_nUtime);
		pItem->AddValue("/gem/N", m_nGem);
		pItem->AddValue("/gem_seq/N", m_nGem_seq);
		pItem->AddValue("/gem_buy/N", m_nGem_buy);
		pItem->AddValue("/npc/N", m_nNpc);
		if (!m_sDe.empty())
		{
			pItem->AddValue("/de/S", JsonEncode(m_sDe, sJsonEncode));
		}
		if (!m_sApns_token.empty())
		{
			pItem->AddValue("/apns_token/S", JsonEncode(m_sApns_token, sJsonEncode));
		}
		pItem->AddValue("/apns_num/N", m_nApns_num);
		pItem->AddValue("/al_time/N", m_nAl_time);
		pItem->AddValue("/last_lg_time/N", m_nLast_lg_time);
		if (!m_sIdfa.empty())
		{
			pItem->AddValue("/idfa/S", JsonEncode(m_sIdfa, sJsonEncode));
		}
		pItem->AddValue("/gem_cost/N", m_nGem_cost);
		if (!m_bGuide_flag.empty())
		{
			pItem->AddValue("/guide_flag/B", Base64Encode((char*)&m_bGuide_flag.m_astList[0], m_bGuide_flag.m_udwNum*sizeof(SBitFlag), sBase64Encode));
		}
		if (!m_sPlatform.empty())
		{
			pItem->AddValue("/platform/S", JsonEncode(m_sPlatform, sJsonEncode));
		}
		pItem->AddValue("/lang/N", m_nLang);
		pItem->AddValue("/ability/N", m_nAbility);
		pItem->AddValue("/rating_time/N", m_nRating_time);
		if (!m_jWeek_gem_recharge.empty())
		{
				pItem->AddValue("/AttributeUpdates/week_gem_recharge/Value/S", JsonEncode(oJsonWriter.write(m_jWeek_gem_recharge), sJsonEncode));
		}
		if (!m_jApns_switch.empty())
		{
				pItem->AddValue("/AttributeUpdates/apns_switch/Value/S", JsonEncode(oJsonWriter.write(m_jApns_switch), sJsonEncode));
		}
		pItem->AddValue("/max_buy/N", m_nMax_buy);
		pItem->AddValue("/last_buy/N", m_nLast_buy);
		pItem->AddValue("/last_buy_time/N", m_nLast_buy_time);
		pItem->AddValue("/total_pay/N", m_nTotal_pay);
		pItem->AddValue("/max_pay/N", m_nMax_pay);
		pItem->AddValue("/last_pay/N", m_nLast_pay);
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

AwsMap* TbLogin::OnReadItemReq(unsigned int udwIdxNo)
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

void TbLogin::OnReadItemReq(AwsMap* pReadItem)
{
	assert(pReadItem);
	AwsList* pKeys = pReadItem->GetAwsMap("RequestItems")->GetAwsMap(GetTableName())->GetAwsList("Keys");
	AwsMap* pKey = new AwsMap;
	assert(pKey);
	pKey->AddValue("/uid/N", m_nUid);
	pKeys->SetValue(pKey, true);
}

int TbLogin::OnReadItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbLogin::OnDeleteItemReq(
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

int TbLogin::OnDeleteItemReq(string& sPostData,
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

int TbLogin::OnDeleteItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbLogin::OnGetItemReq(unsigned int udwIdxNo,
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

int TbLogin::OnGetItemReq(string& sPostData, unsigned int udwIdxNo,
	bool bConsistentRead, bool bReturnConsumedCapacity)
{
	AwsMap* pGetItem = OnGetItemReq(udwIdxNo, bConsistentRead, bReturnConsumedCapacity);
	ostringstream oss;
	pGetItem->Dump(oss);
	sPostData = oss.str();
	delete pGetItem;
	return 0;
}

int TbLogin::OnGetItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbLogin::OnPutItemReq(
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
	pPutItem->AddValue("/Item/uid/N", m_nUid);
	pPutItem->AddValue("/Item/seq/N", m_nSeq);
	pPutItem->AddValue("/Item/did/N", m_nDid);
	pPutItem->AddValue("/Item/ctime/N", m_nCtime);
	pPutItem->AddValue("/Item/utime/N", m_nUtime);
	pPutItem->AddValue("/Item/gem/N", m_nGem);
	pPutItem->AddValue("/Item/gem_seq/N", m_nGem_seq);
	pPutItem->AddValue("/Item/gem_buy/N", m_nGem_buy);
	pPutItem->AddValue("/Item/npc/N", m_nNpc);
	if (!m_sDe.empty())
	{
		pPutItem->AddValue("/Item/de/S", JsonEncode(m_sDe, sJsonEncode));
	}
	if (!m_sApns_token.empty())
	{
		pPutItem->AddValue("/Item/apns_token/S", JsonEncode(m_sApns_token, sJsonEncode));
	}
	pPutItem->AddValue("/Item/apns_num/N", m_nApns_num);
	pPutItem->AddValue("/Item/al_time/N", m_nAl_time);
	pPutItem->AddValue("/Item/last_lg_time/N", m_nLast_lg_time);
	if (!m_sIdfa.empty())
	{
		pPutItem->AddValue("/Item/idfa/S", JsonEncode(m_sIdfa, sJsonEncode));
	}
	pPutItem->AddValue("/Item/gem_cost/N", m_nGem_cost);
	if (!m_bGuide_flag.empty())
	{
		pPutItem->AddValue("/Item/guide_flag/B", Base64Encode((char*)&m_bGuide_flag.m_astList[0], m_bGuide_flag.m_udwNum*sizeof(SBitFlag), sBase64Encode));
	}
	if (!m_sPlatform.empty())
	{
		pPutItem->AddValue("/Item/platform/S", JsonEncode(m_sPlatform, sJsonEncode));
	}
	pPutItem->AddValue("/Item/lang/N", m_nLang);
	pPutItem->AddValue("/Item/ability/N", m_nAbility);
	pPutItem->AddValue("/Item/rating_time/N", m_nRating_time);
	if (!m_jWeek_gem_recharge.empty())
	{
				pPutItem->AddValue("/Item/week_gem_recharge/S", JsonEncode(oJsonWriter.write(m_jWeek_gem_recharge), sJsonEncode));
	}
	if (!m_jApns_switch.empty())
	{
				pPutItem->AddValue("/Item/apns_switch/S", JsonEncode(oJsonWriter.write(m_jApns_switch), sJsonEncode));
	}
	pPutItem->AddValue("/Item/max_buy/N", m_nMax_buy);
	pPutItem->AddValue("/Item/last_buy/N", m_nLast_buy);
	pPutItem->AddValue("/Item/last_buy_time/N", m_nLast_buy_time);
	pPutItem->AddValue("/Item/total_pay/N", m_nTotal_pay);
	pPutItem->AddValue("/Item/max_pay/N", m_nMax_pay);
	pPutItem->AddValue("/Item/last_pay/N", m_nLast_pay);
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

int TbLogin::OnPutItemReq(string& sPostData,
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

int TbLogin::OnPutItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

int TbLogin::OnResponse(const Json::Value& item)
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
		if (vecMembers[i] == "uid")
		{
			m_nUid = strtoll(item["uid"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "seq")
		{
			m_nSeq = strtoll(item["seq"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "did")
		{
			m_nDid = strtoll(item["did"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "ctime")
		{
			m_nCtime = strtoll(item["ctime"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "utime")
		{
			m_nUtime = strtoll(item["utime"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "gem")
		{
			m_nGem = strtoll(item["gem"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "gem_seq")
		{
			m_nGem_seq = strtoll(item["gem_seq"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "gem_buy")
		{
			m_nGem_buy = strtoll(item["gem_buy"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "npc")
		{
			m_nNpc = strtoll(item["npc"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "de")
		{
			m_sDe = item["de"]["S"].asString();
			continue;
		}
		if (vecMembers[i] == "apns_token")
		{
			m_sApns_token = item["apns_token"]["S"].asString();
			continue;
		}
		if (vecMembers[i] == "apns_num")
		{
			m_nApns_num = strtoll(item["apns_num"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "al_time")
		{
			m_nAl_time = strtoll(item["al_time"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "last_lg_time")
		{
			m_nLast_lg_time = strtoll(item["last_lg_time"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "idfa")
		{
			m_sIdfa = item["idfa"]["S"].asString();
			continue;
		}
		if (vecMembers[i] == "gem_cost")
		{
			m_nGem_cost = strtoll(item["gem_cost"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "guide_flag")
		{
			Base64Decode(item["guide_flag"]["B"].asString(), (char*)&m_bGuide_flag.m_astList[0], dwResLen);
			m_bGuide_flag.m_udwNum = dwResLen/sizeof(SBitFlag);
			if (0==m_bGuide_flag.m_udwNum && dwResLen>0)
			{
				m_bGuide_flag.m_udwNum = 1;
			}
			continue;
		}
		if (vecMembers[i] == "platform")
		{
			m_sPlatform = item["platform"]["S"].asString();
			continue;
		}
		if (vecMembers[i] == "lang")
		{
			m_nLang = strtoll(item["lang"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "ability")
		{
			m_nAbility = strtoll(item["ability"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "rating_time")
		{
			m_nRating_time = strtoll(item["rating_time"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "week_gem_recharge")
		{
			if (FALSE == oJsonReader.parse(item["week_gem_recharge"]["S"].asString(),m_jWeek_gem_recharge))
			{
				m_jWeek_gem_recharge = Json::Value(Json::nullValue);
			}
			continue;
		}
		if (vecMembers[i] == "apns_switch")
		{
			if (FALSE == oJsonReader.parse(item["apns_switch"]["S"].asString(),m_jApns_switch))
			{
				m_jApns_switch = Json::Value(Json::nullValue);
			}
			continue;
		}
		if (vecMembers[i] == "max_buy")
		{
			m_nMax_buy = strtoll(item["max_buy"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "last_buy")
		{
			m_nLast_buy = strtoll(item["last_buy"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "last_buy_time")
		{
			m_nLast_buy_time = strtoll(item["last_buy_time"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "total_pay")
		{
			m_nTotal_pay = strtoll(item["total_pay"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "max_pay")
		{
			m_nMax_pay = strtoll(item["max_pay"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "last_pay")
		{
			m_nLast_pay = strtoll(item["last_pay"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
	}
	return 0;
}

TINT64 TbLogin::GetSeq()
{
	return m_nSeq;
}

