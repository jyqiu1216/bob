#include "aws_table_map.h"

TableDesc TbMap::oTableDesc;

int TbMap::Init(const string& sConfFile, const string strProjectName)
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

AwsTable* TbMap::NewObject()
{
	return new TbMap;
}

string TbMap::GetTableName()
{
	ostringstream oss;
	if(!oTableDesc.m_strProjectName.empty())
	{
	oss << oTableDesc.m_strProjectName << "_";
	}
	oss << "split_";
	oss << oTableDesc.sName;
	oss << "_" << this->GetTableIdx();
	return oss.str();
}

TINT32 TbMap::GetTableIdx()
{
	return this->m_nSid/1;
}

AwsMap* TbMap::OnScanReq(unsigned int udwIdxNo, bool bHasStartKey, bool bReturnConsumedCapacity,
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
		pScan->AddValue("/ExclusiveStartKey/id/N",m_nId);
	}
	return pScan;
}

int TbMap::OnScanReq(string& sPostData, unsigned int udwIdxNo, bool bHasStartKey, bool bReturnConsumedCapacity,
 unsigned int dwLimit, unsigned int dwSegment, unsigned int dwTotalSegments)
{
	AwsMap* pScan = OnScanReq(udwIdxNo, bHasStartKey, bReturnConsumedCapacity, dwLimit, dwSegment, dwTotalSegments);
	ostringstream oss;
	pScan->Dump(oss);
	sPostData = oss.str();
	delete pScan;
	return 0;
}

int TbMap::OnScanRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbMap::OnQueryReq(unsigned int udwIdxNo, const CompareDesc& comp_desc,
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
			if(fld_desc.udwFldNo == TbMAP_FIELD_SID)
			{
				pQuery->AddValue("/KeyConditions/sid/AttributeValueList[0]/N", m_nSid);
				pQuery->AddValue("/KeyConditions/sid/ComparisonOperator", "EQ");
			}
			if(fld_desc.udwFldNo == TbMAP_FIELD_ID)
			{
				pQuery->AddValue("/KeyConditions/id/AttributeValueList[0]/N", m_nId);
				pQuery->AddValue("/KeyConditions/id/ComparisonOperator", "EQ");
			}
			if(fld_desc.udwFldNo == TbMAP_FIELD_UID)
			{
				pQuery->AddValue("/KeyConditions/uid/AttributeValueList[0]/N", m_nUid);
				pQuery->AddValue("/KeyConditions/uid/ComparisonOperator", "EQ");
			}
		}
		else if (i == 1) //1只能是rangekey or local index，可以有多种方式
		{
			if (COMPARE_TYPE_EQ<=comp_desc.dwCompareType && comp_desc.dwCompareType<=COMPARE_TYPE_GT)
			{
				if(fld_desc.udwFldNo == TbMAP_FIELD_ID)
				{
					pQuery->AddValue("/KeyConditions/id/AttributeValueList[0]/N", m_nId);
					pQuery->AddValue("/KeyConditions/id/ComparisonOperator", CompareType2Str(comp_desc.dwCompareType));
				}
				if(fld_desc.udwFldNo == TbMAP_FIELD_BID)
				{
					pQuery->AddValue("/KeyConditions/bid/AttributeValueList[0]/N", m_nBid);
					pQuery->AddValue("/KeyConditions/bid/ComparisonOperator", CompareType2Str(comp_desc.dwCompareType));
				}
			}
			else if (COMPARE_TYPE_BETWEEN == comp_desc.dwCompareType)
			{
				if(fld_desc.udwFldNo == TbMAP_FIELD_ID)
				{
					pQuery->AddValue("/KeyConditions/id/AttributeValueList[0]/N" , comp_desc.vecN[0]);
					pQuery->AddValue("/KeyConditions/id/AttributeValueList[1]/N" , comp_desc.vecN[1]);
					pQuery->AddValue("/KeyConditions/id/ComparisonOperator", "BETWEEN");
				}
				if(fld_desc.udwFldNo == TbMAP_FIELD_BID)
				{
					pQuery->AddValue("/KeyConditions/bid/AttributeValueList[0]/N" , comp_desc.vecN[0]);
					pQuery->AddValue("/KeyConditions/bid/AttributeValueList[1]/N" , comp_desc.vecN[1]);
					pQuery->AddValue("/KeyConditions/bid/ComparisonOperator", "BETWEEN");
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

int TbMap::OnQueryReq(string& sPostData, unsigned int udwIdxNo, const CompareDesc& comp_desc,
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

int TbMap::OnQueryRsp(const Json::Value& item)
{
	return OnResponse(item);
}

int TbMap::OnCountReq(string& sPostData, unsigned int udwIdxNo, const CompareDesc& comp_desc, 
		bool bConsistentRead, bool bReturnConsumedCapacity, bool bScanIndexForward, unsigned int dwLimit)
{
	AwsMap* pQuery = OnQueryReq(udwIdxNo, comp_desc, bConsistentRead, bReturnConsumedCapacity, bScanIndexForward, dwLimit, true);
	ostringstream oss;
	pQuery->Dump(oss);
	sPostData = oss.str();
	delete pQuery;
	return 0;
}

AwsMap* TbMap::OnUpdateItemReq(
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
	pUpdateItem->AddValue("/Key/id/N", m_nId);
	for (map<unsigned int, int>::iterator iter = m_mFlag.begin(); iter != m_mFlag.end(); ++iter)
	{
		bUpdateFlag = true;
		if (TbMAP_FIELD_SID == iter->first)
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
		if (TbMAP_FIELD_UTIME == iter->first)
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
		if (TbMAP_FIELD_TYPE == iter->first)
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
		if (TbMAP_FIELD_RTYPE == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/rtype/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/rtype/Value/N", m_nRtype);
				pUpdateItem->AddValue("/AttributeUpdates/rtype/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMAP_FIELD_LEVEL == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/level/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/level/Value/N", m_nLevel);
				pUpdateItem->AddValue("/AttributeUpdates/level/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMAP_FIELD_MIGHT == iter->first)
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
		if (TbMAP_FIELD_BID == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/bid/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/bid/Value/N", m_nBid);
				pUpdateItem->AddValue("/AttributeUpdates/bid/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMAP_FIELD_UID == iter->first)
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
		if (TbMAP_FIELD_NAME == iter->first)
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
		if (TbMAP_FIELD_STATUS == iter->first)
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
		if (TbMAP_FIELD_NPC == iter->first)
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
		if (TbMAP_FIELD_CITY_INFO == iter->first)
		{
			if (!m_jCity_info.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/city_info/Value/S", JsonEncode(oJsonWriter.write(m_jCity_info), sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/city_info/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/city_info/Action", "DELETE");
			}
			continue;
		}
		if (TbMAP_FIELD_EM_LV == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/em_lv/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/em_lv/Value/N", m_nEm_lv);
				pUpdateItem->AddValue("/AttributeUpdates/em_lv/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMAP_FIELD_CID == iter->first)
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
		if (TbMAP_FIELD_UNAME == iter->first)
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
		if (TbMAP_FIELD_ULEVEL == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/ulevel/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/ulevel/Value/N", m_nUlevel);
				pUpdateItem->AddValue("/AttributeUpdates/ulevel/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMAP_FIELD_ALID == iter->first)
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
		if (TbMAP_FIELD_ALNAME == iter->first)
		{
			if (!m_sAlname.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/alname/Value/S", JsonEncode(m_sAlname, sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/alname/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/alname/Action", "DELETE");
			}
			continue;
		}
		if (TbMAP_FIELD_AL_NICK == iter->first)
		{
			if (!m_sAl_nick.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/al_nick/Value/S", JsonEncode(m_sAl_nick, sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/al_nick/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/al_nick/Action", "DELETE");
			}
			continue;
		}
		if (TbMAP_FIELD_AL_FLAG == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/al_flag/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/al_flag/Value/N", m_nAl_flag);
				pUpdateItem->AddValue("/AttributeUpdates/al_flag/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMAP_FIELD_VIP_LEVEL == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/vip_level/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/vip_level/Value/N", m_nVip_level);
				pUpdateItem->AddValue("/AttributeUpdates/vip_level/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMAP_FIELD_VIP_ETIME == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/vip_etime/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/vip_etime/Value/N", m_nVip_etime);
				pUpdateItem->AddValue("/AttributeUpdates/vip_etime/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMAP_FIELD_CENTER_POS == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/center_pos/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/center_pos/Value/N", m_nCenter_pos);
				pUpdateItem->AddValue("/AttributeUpdates/center_pos/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMAP_FIELD_NAME_UPDATE_TIME == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/name_update_time/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/name_update_time/Value/N", m_nName_update_time);
				pUpdateItem->AddValue("/AttributeUpdates/name_update_time/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMAP_FIELD_AL_POS == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/al_pos/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/al_pos/Value/N", m_nAl_pos);
				pUpdateItem->AddValue("/AttributeUpdates/al_pos/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMAP_FIELD_AVATAR == iter->first)
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
		if (TbMAP_FIELD_AGE == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/age/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/age/Value/N", m_nAge);
				pUpdateItem->AddValue("/AttributeUpdates/age/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMAP_FIELD_PRISON_FLAG == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/prison_flag/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/prison_flag/Value/N", m_nPrison_flag);
				pUpdateItem->AddValue("/AttributeUpdates/prison_flag/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMAP_FIELD_TIME_END == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/time_end/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/time_end/Value/N", m_nTime_end);
				pUpdateItem->AddValue("/AttributeUpdates/time_end/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMAP_FIELD_PIC_INDEX == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/pic_index/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/pic_index/Value/N", m_nPic_index);
				pUpdateItem->AddValue("/AttributeUpdates/pic_index/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMAP_FIELD_RALLY_TROOP_LIMIT == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/rally_troop_limit/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/rally_troop_limit/Value/N", m_nRally_troop_limit);
				pUpdateItem->AddValue("/AttributeUpdates/rally_troop_limit/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMAP_FIELD_SMOKE_END_TIME == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/smoke_end_time/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/smoke_end_time/Value/N", m_nSmoke_end_time);
				pUpdateItem->AddValue("/AttributeUpdates/smoke_end_time/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMAP_FIELD_BURN_END_TIME == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/burn_end_time/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/burn_end_time/Value/N", m_nBurn_end_time);
				pUpdateItem->AddValue("/AttributeUpdates/burn_end_time/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMAP_FIELD_MARCH_STATUS_TIME == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/march_status_time/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/march_status_time/Value/N", m_nMarch_status_time);
				pUpdateItem->AddValue("/AttributeUpdates/march_status_time/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMAP_FIELD_MOVE_CITY == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/move_city/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/move_city/Value/N", m_nMove_city);
				pUpdateItem->AddValue("/AttributeUpdates/move_city/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMAP_FIELD_RESOURCE == iter->first)
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
		if (TbMAP_FIELD_TROOP == iter->first)
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
		if (TbMAP_FIELD_RES_RATE == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/res_rate/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/res_rate/Value/N", m_nRes_rate);
				pUpdateItem->AddValue("/AttributeUpdates/res_rate/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMAP_FIELD_RES_TIME == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/res_time/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/res_time/Value/N", m_nRes_time);
				pUpdateItem->AddValue("/AttributeUpdates/res_time/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMAP_FIELD_REWARD_LEFT == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/reward_left/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/reward_left/Value/N", m_nReward_left);
				pUpdateItem->AddValue("/AttributeUpdates/reward_left/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMAP_FIELD_SHOWTIME == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/showtime/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/showtime/Value/N", m_nShowtime);
				pUpdateItem->AddValue("/AttributeUpdates/showtime/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMAP_FIELD_WILD_GEN_TIME == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/wild_gen_time/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/wild_gen_time/Value/N", m_nWild_gen_time);
				pUpdateItem->AddValue("/AttributeUpdates/wild_gen_time/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMAP_FIELD_BOSS_LIFE == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/boss_life/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/boss_life/Value/N", m_nBoss_life);
				pUpdateItem->AddValue("/AttributeUpdates/boss_life/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMAP_FIELD_ATTACK_INFO == iter->first)
		{
			if (!m_bAttack_info.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/attack_info/Value/B", Base64Encode((char*)&m_bAttack_info.m_astList[0], m_bAttack_info.m_udwNum*sizeof(SAttackTimesInfo), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/attack_info/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/attack_info/Action", "DELETE");
			}
			continue;
		}
		if (TbMAP_FIELD_TAX_RATE_ID == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/tax_rate_id/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/tax_rate_id/Value/N", m_nTax_rate_id);
				pUpdateItem->AddValue("/AttributeUpdates/tax_rate_id/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMAP_FIELD_FORCE_KILL == iter->first)
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
		if (TbMAP_FIELD_AL_ATTACK_INFO == iter->first)
		{
			if (!m_bAl_attack_info.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/al_attack_info/Value/B", Base64Encode((char*)&m_bAl_attack_info.m_astList[0], m_bAl_attack_info.m_udwNum*sizeof(SAttackTimesInfo), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/al_attack_info/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/al_attack_info/Action", "DELETE");
			}
			continue;
		}
		if (TbMAP_FIELD_LEADER_MONSTER_FLAG == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/leader_monster_flag/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/leader_monster_flag/Value/N", m_nLeader_monster_flag);
				pUpdateItem->AddValue("/AttributeUpdates/leader_monster_flag/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMAP_FIELD_OCCUPY_NUM == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/occupy_num/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/occupy_num/Value/N", m_nOccupy_num);
				pUpdateItem->AddValue("/AttributeUpdates/occupy_num/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMAP_FIELD_OCCUPY_CLEAN_FLAG == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/occupy_clean_flag/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/occupy_clean_flag/Value/N", m_nOccupy_clean_flag);
				pUpdateItem->AddValue("/AttributeUpdates/occupy_clean_flag/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMAP_FIELD_SEQ == iter->first)
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
		if (TbMAP_FIELD_EXPIRE_TIME == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/expire_time/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/expire_time/Value/N", m_nExpire_time);
				pUpdateItem->AddValue("/AttributeUpdates/expire_time/Action", UpdateActionType2Str(iter->second));
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

int TbMap::OnUpdateItemReq(string& sPostData,
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

int TbMap::OnUpdateItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbMap::OnWriteItemReq(int dwActionType)
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
		pItem->AddValue("/utime/N", m_nUtime);
		pItem->AddValue("/type/N", m_nType);
		pItem->AddValue("/rtype/N", m_nRtype);
		pItem->AddValue("/level/N", m_nLevel);
		pItem->AddValue("/might/N", m_nMight);
		pItem->AddValue("/bid/N", m_nBid);
		pItem->AddValue("/uid/N", m_nUid);
		if (!m_sName.empty())
		{
			pItem->AddValue("/name/S", JsonEncode(m_sName, sJsonEncode));
		}
		pItem->AddValue("/status/N", m_nStatus);
		pItem->AddValue("/npc/N", m_nNpc);
		if (!m_jCity_info.empty())
		{
				pItem->AddValue("/AttributeUpdates/city_info/Value/S", JsonEncode(oJsonWriter.write(m_jCity_info), sJsonEncode));
		}
		pItem->AddValue("/em_lv/N", m_nEm_lv);
		pItem->AddValue("/cid/N", m_nCid);
		if (!m_sUname.empty())
		{
			pItem->AddValue("/uname/S", JsonEncode(m_sUname, sJsonEncode));
		}
		pItem->AddValue("/ulevel/N", m_nUlevel);
		pItem->AddValue("/alid/N", m_nAlid);
		if (!m_sAlname.empty())
		{
			pItem->AddValue("/alname/S", JsonEncode(m_sAlname, sJsonEncode));
		}
		if (!m_sAl_nick.empty())
		{
			pItem->AddValue("/al_nick/S", JsonEncode(m_sAl_nick, sJsonEncode));
		}
		pItem->AddValue("/al_flag/N", m_nAl_flag);
		pItem->AddValue("/vip_level/N", m_nVip_level);
		pItem->AddValue("/vip_etime/N", m_nVip_etime);
		pItem->AddValue("/center_pos/N", m_nCenter_pos);
		pItem->AddValue("/name_update_time/N", m_nName_update_time);
		pItem->AddValue("/al_pos/N", m_nAl_pos);
		pItem->AddValue("/avatar/N", m_nAvatar);
		pItem->AddValue("/age/N", m_nAge);
		pItem->AddValue("/prison_flag/N", m_nPrison_flag);
		pItem->AddValue("/time_end/N", m_nTime_end);
		pItem->AddValue("/pic_index/N", m_nPic_index);
		pItem->AddValue("/rally_troop_limit/N", m_nRally_troop_limit);
		pItem->AddValue("/smoke_end_time/N", m_nSmoke_end_time);
		pItem->AddValue("/burn_end_time/N", m_nBurn_end_time);
		pItem->AddValue("/march_status_time/N", m_nMarch_status_time);
		pItem->AddValue("/move_city/N", m_nMove_city);
		if (!m_bResource.empty())
		{
			pItem->AddValue("/resource/B", Base64Encode((char*)&m_bResource.m_astList[0], m_bResource.m_udwNum*sizeof(SCommonResource), sBase64Encode));
		}
		if (!m_bTroop.empty())
		{
			pItem->AddValue("/troop/B", Base64Encode((char*)&m_bTroop.m_astList[0], m_bTroop.m_udwNum*sizeof(SCommonTroop), sBase64Encode));
		}
		pItem->AddValue("/res_rate/N", m_nRes_rate);
		pItem->AddValue("/res_time/N", m_nRes_time);
		pItem->AddValue("/reward_left/N", m_nReward_left);
		pItem->AddValue("/showtime/N", m_nShowtime);
		pItem->AddValue("/wild_gen_time/N", m_nWild_gen_time);
		pItem->AddValue("/boss_life/N", m_nBoss_life);
		if (!m_bAttack_info.empty())
		{
			pItem->AddValue("/attack_info/B", Base64Encode((char*)&m_bAttack_info.m_astList[0], m_bAttack_info.m_udwNum*sizeof(SAttackTimesInfo), sBase64Encode));
		}
		pItem->AddValue("/tax_rate_id/N", m_nTax_rate_id);
		pItem->AddValue("/force_kill/N", m_nForce_kill);
		if (!m_bAl_attack_info.empty())
		{
			pItem->AddValue("/al_attack_info/B", Base64Encode((char*)&m_bAl_attack_info.m_astList[0], m_bAl_attack_info.m_udwNum*sizeof(SAttackTimesInfo), sBase64Encode));
		}
		pItem->AddValue("/leader_monster_flag/N", m_nLeader_monster_flag);
		pItem->AddValue("/occupy_num/N", m_nOccupy_num);
		pItem->AddValue("/occupy_clean_flag/N", m_nOccupy_clean_flag);
		pItem->AddValue("/seq/N", m_nSeq);
		pItem->AddValue("/expire_time/N", m_nExpire_time);
	}
	else if (WRITE_ACTION_TYPE_DELETE == dwActionType)
	{
		pItem = pWriteItem->GetAwsMap("DeleteRequest")->GetAwsMap("Key");
		pItem->AddValue("/id/N", m_nId);
	}
	else
	{
		assert(0);
	}
	return pWriteItem;
}

void TbMap::OnWriteItemReq(AwsMap* pWriteItem, int dwActionType)
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
		pItem->AddValue("/utime/N", m_nUtime);
		pItem->AddValue("/type/N", m_nType);
		pItem->AddValue("/rtype/N", m_nRtype);
		pItem->AddValue("/level/N", m_nLevel);
		pItem->AddValue("/might/N", m_nMight);
		pItem->AddValue("/bid/N", m_nBid);
		pItem->AddValue("/uid/N", m_nUid);
		if (!m_sName.empty())
		{
			pItem->AddValue("/name/S", JsonEncode(m_sName, sJsonEncode));
		}
		pItem->AddValue("/status/N", m_nStatus);
		pItem->AddValue("/npc/N", m_nNpc);
		if (!m_jCity_info.empty())
		{
				pItem->AddValue("/AttributeUpdates/city_info/Value/S", JsonEncode(oJsonWriter.write(m_jCity_info), sJsonEncode));
		}
		pItem->AddValue("/em_lv/N", m_nEm_lv);
		pItem->AddValue("/cid/N", m_nCid);
		if (!m_sUname.empty())
		{
			pItem->AddValue("/uname/S", JsonEncode(m_sUname, sJsonEncode));
		}
		pItem->AddValue("/ulevel/N", m_nUlevel);
		pItem->AddValue("/alid/N", m_nAlid);
		if (!m_sAlname.empty())
		{
			pItem->AddValue("/alname/S", JsonEncode(m_sAlname, sJsonEncode));
		}
		if (!m_sAl_nick.empty())
		{
			pItem->AddValue("/al_nick/S", JsonEncode(m_sAl_nick, sJsonEncode));
		}
		pItem->AddValue("/al_flag/N", m_nAl_flag);
		pItem->AddValue("/vip_level/N", m_nVip_level);
		pItem->AddValue("/vip_etime/N", m_nVip_etime);
		pItem->AddValue("/center_pos/N", m_nCenter_pos);
		pItem->AddValue("/name_update_time/N", m_nName_update_time);
		pItem->AddValue("/al_pos/N", m_nAl_pos);
		pItem->AddValue("/avatar/N", m_nAvatar);
		pItem->AddValue("/age/N", m_nAge);
		pItem->AddValue("/prison_flag/N", m_nPrison_flag);
		pItem->AddValue("/time_end/N", m_nTime_end);
		pItem->AddValue("/pic_index/N", m_nPic_index);
		pItem->AddValue("/rally_troop_limit/N", m_nRally_troop_limit);
		pItem->AddValue("/smoke_end_time/N", m_nSmoke_end_time);
		pItem->AddValue("/burn_end_time/N", m_nBurn_end_time);
		pItem->AddValue("/march_status_time/N", m_nMarch_status_time);
		pItem->AddValue("/move_city/N", m_nMove_city);
		if (!m_bResource.empty())
		{
			pItem->AddValue("/resource/B", Base64Encode((char*)&m_bResource.m_astList[0], m_bResource.m_udwNum*sizeof(SCommonResource), sBase64Encode));
		}
		if (!m_bTroop.empty())
		{
			pItem->AddValue("/troop/B", Base64Encode((char*)&m_bTroop.m_astList[0], m_bTroop.m_udwNum*sizeof(SCommonTroop), sBase64Encode));
		}
		pItem->AddValue("/res_rate/N", m_nRes_rate);
		pItem->AddValue("/res_time/N", m_nRes_time);
		pItem->AddValue("/reward_left/N", m_nReward_left);
		pItem->AddValue("/showtime/N", m_nShowtime);
		pItem->AddValue("/wild_gen_time/N", m_nWild_gen_time);
		pItem->AddValue("/boss_life/N", m_nBoss_life);
		if (!m_bAttack_info.empty())
		{
			pItem->AddValue("/attack_info/B", Base64Encode((char*)&m_bAttack_info.m_astList[0], m_bAttack_info.m_udwNum*sizeof(SAttackTimesInfo), sBase64Encode));
		}
		pItem->AddValue("/tax_rate_id/N", m_nTax_rate_id);
		pItem->AddValue("/force_kill/N", m_nForce_kill);
		if (!m_bAl_attack_info.empty())
		{
			pItem->AddValue("/al_attack_info/B", Base64Encode((char*)&m_bAl_attack_info.m_astList[0], m_bAl_attack_info.m_udwNum*sizeof(SAttackTimesInfo), sBase64Encode));
		}
		pItem->AddValue("/leader_monster_flag/N", m_nLeader_monster_flag);
		pItem->AddValue("/occupy_num/N", m_nOccupy_num);
		pItem->AddValue("/occupy_clean_flag/N", m_nOccupy_clean_flag);
		pItem->AddValue("/seq/N", m_nSeq);
		pItem->AddValue("/expire_time/N", m_nExpire_time);
	}
	else if (WRITE_ACTION_TYPE_DELETE == dwActionType)
	{
		pItem = pReqItem->GetAwsMap("DeleteRequest")->GetAwsMap("Key");
		pItem->AddValue("/id/N", m_nId);
	}
	else
	{
		assert(0);
	}
	pWriteItem->GetAwsMap("RequestItems")->GetAwsList(GetTableName())->SetValue(pReqItem, true);
}

AwsMap* TbMap::OnReadItemReq(unsigned int udwIdxNo)
{
	oJsonWriter.omitEndingLineFeed();
	IndexDesc& idx_desc = oTableDesc.mIndexDesc[udwIdxNo];
	assert(idx_desc.sName == "PRIMARY"); //只能通过主键查询
	AwsMap* pReadItem = new AwsMap;
	assert(pReadItem);
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

void TbMap::OnReadItemReq(AwsMap* pReadItem)
{
	assert(pReadItem);
	AwsList* pKeys = pReadItem->GetAwsMap("RequestItems")->GetAwsMap(GetTableName())->GetAwsList("Keys");
	AwsMap* pKey = new AwsMap;
	assert(pKey);
	pKey->AddValue("/id/N", m_nId);
	pKeys->SetValue(pKey, true);
}

int TbMap::OnReadItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbMap::OnDeleteItemReq(
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

int TbMap::OnDeleteItemReq(string& sPostData,
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

int TbMap::OnDeleteItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbMap::OnGetItemReq(unsigned int udwIdxNo,
	bool bConsistentRead, bool bReturnConsumedCapacity)
{
	oJsonWriter.omitEndingLineFeed();
	assert(oTableDesc.mIndexDesc.find(udwIdxNo)!=oTableDesc.mIndexDesc.end());
	IndexDesc& idx_desc = oTableDesc.mIndexDesc[udwIdxNo];
	assert(idx_desc.sName == "PRIMARY"); //只能通过主键查询
	AwsMap* pGetItem = new AwsMap;
	assert(pGetItem);
	pGetItem->AddValue("/TableName", GetTableName());
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

int TbMap::OnGetItemReq(string& sPostData, unsigned int udwIdxNo,
	bool bConsistentRead, bool bReturnConsumedCapacity)
{
	AwsMap* pGetItem = OnGetItemReq(udwIdxNo, bConsistentRead, bReturnConsumedCapacity);
	ostringstream oss;
	pGetItem->Dump(oss);
	sPostData = oss.str();
	delete pGetItem;
	return 0;
}

int TbMap::OnGetItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbMap::OnPutItemReq(
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
	pPutItem->AddValue("/Item/utime/N", m_nUtime);
	pPutItem->AddValue("/Item/type/N", m_nType);
	pPutItem->AddValue("/Item/rtype/N", m_nRtype);
	pPutItem->AddValue("/Item/level/N", m_nLevel);
	pPutItem->AddValue("/Item/might/N", m_nMight);
	pPutItem->AddValue("/Item/bid/N", m_nBid);
	pPutItem->AddValue("/Item/uid/N", m_nUid);
	if (!m_sName.empty())
	{
		pPutItem->AddValue("/Item/name/S", JsonEncode(m_sName, sJsonEncode));
	}
	pPutItem->AddValue("/Item/status/N", m_nStatus);
	pPutItem->AddValue("/Item/npc/N", m_nNpc);
	if (!m_jCity_info.empty())
	{
				pPutItem->AddValue("/Item/city_info/S", JsonEncode(oJsonWriter.write(m_jCity_info), sJsonEncode));
	}
	pPutItem->AddValue("/Item/em_lv/N", m_nEm_lv);
	pPutItem->AddValue("/Item/cid/N", m_nCid);
	if (!m_sUname.empty())
	{
		pPutItem->AddValue("/Item/uname/S", JsonEncode(m_sUname, sJsonEncode));
	}
	pPutItem->AddValue("/Item/ulevel/N", m_nUlevel);
	pPutItem->AddValue("/Item/alid/N", m_nAlid);
	if (!m_sAlname.empty())
	{
		pPutItem->AddValue("/Item/alname/S", JsonEncode(m_sAlname, sJsonEncode));
	}
	if (!m_sAl_nick.empty())
	{
		pPutItem->AddValue("/Item/al_nick/S", JsonEncode(m_sAl_nick, sJsonEncode));
	}
	pPutItem->AddValue("/Item/al_flag/N", m_nAl_flag);
	pPutItem->AddValue("/Item/vip_level/N", m_nVip_level);
	pPutItem->AddValue("/Item/vip_etime/N", m_nVip_etime);
	pPutItem->AddValue("/Item/center_pos/N", m_nCenter_pos);
	pPutItem->AddValue("/Item/name_update_time/N", m_nName_update_time);
	pPutItem->AddValue("/Item/al_pos/N", m_nAl_pos);
	pPutItem->AddValue("/Item/avatar/N", m_nAvatar);
	pPutItem->AddValue("/Item/age/N", m_nAge);
	pPutItem->AddValue("/Item/prison_flag/N", m_nPrison_flag);
	pPutItem->AddValue("/Item/time_end/N", m_nTime_end);
	pPutItem->AddValue("/Item/pic_index/N", m_nPic_index);
	pPutItem->AddValue("/Item/rally_troop_limit/N", m_nRally_troop_limit);
	pPutItem->AddValue("/Item/smoke_end_time/N", m_nSmoke_end_time);
	pPutItem->AddValue("/Item/burn_end_time/N", m_nBurn_end_time);
	pPutItem->AddValue("/Item/march_status_time/N", m_nMarch_status_time);
	pPutItem->AddValue("/Item/move_city/N", m_nMove_city);
	if (!m_bResource.empty())
	{
		pPutItem->AddValue("/Item/resource/B", Base64Encode((char*)&m_bResource.m_astList[0], m_bResource.m_udwNum*sizeof(SCommonResource), sBase64Encode));
	}
	if (!m_bTroop.empty())
	{
		pPutItem->AddValue("/Item/troop/B", Base64Encode((char*)&m_bTroop.m_astList[0], m_bTroop.m_udwNum*sizeof(SCommonTroop), sBase64Encode));
	}
	pPutItem->AddValue("/Item/res_rate/N", m_nRes_rate);
	pPutItem->AddValue("/Item/res_time/N", m_nRes_time);
	pPutItem->AddValue("/Item/reward_left/N", m_nReward_left);
	pPutItem->AddValue("/Item/showtime/N", m_nShowtime);
	pPutItem->AddValue("/Item/wild_gen_time/N", m_nWild_gen_time);
	pPutItem->AddValue("/Item/boss_life/N", m_nBoss_life);
	if (!m_bAttack_info.empty())
	{
		pPutItem->AddValue("/Item/attack_info/B", Base64Encode((char*)&m_bAttack_info.m_astList[0], m_bAttack_info.m_udwNum*sizeof(SAttackTimesInfo), sBase64Encode));
	}
	pPutItem->AddValue("/Item/tax_rate_id/N", m_nTax_rate_id);
	pPutItem->AddValue("/Item/force_kill/N", m_nForce_kill);
	if (!m_bAl_attack_info.empty())
	{
		pPutItem->AddValue("/Item/al_attack_info/B", Base64Encode((char*)&m_bAl_attack_info.m_astList[0], m_bAl_attack_info.m_udwNum*sizeof(SAttackTimesInfo), sBase64Encode));
	}
	pPutItem->AddValue("/Item/leader_monster_flag/N", m_nLeader_monster_flag);
	pPutItem->AddValue("/Item/occupy_num/N", m_nOccupy_num);
	pPutItem->AddValue("/Item/occupy_clean_flag/N", m_nOccupy_clean_flag);
	pPutItem->AddValue("/Item/seq/N", m_nSeq);
	pPutItem->AddValue("/Item/expire_time/N", m_nExpire_time);
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

int TbMap::OnPutItemReq(string& sPostData,
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

int TbMap::OnPutItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

int TbMap::OnResponse(const Json::Value& item)
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
		if (vecMembers[i] == "utime")
		{
			m_nUtime = strtoll(item["utime"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "type")
		{
			m_nType = strtoll(item["type"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "rtype")
		{
			m_nRtype = strtoll(item["rtype"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "level")
		{
			m_nLevel = strtoll(item["level"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "might")
		{
			m_nMight = strtoll(item["might"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "bid")
		{
			m_nBid = strtoll(item["bid"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "uid")
		{
			m_nUid = strtoll(item["uid"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "name")
		{
			m_sName = item["name"]["S"].asString();
			continue;
		}
		if (vecMembers[i] == "status")
		{
			m_nStatus = strtoll(item["status"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "npc")
		{
			m_nNpc = strtoll(item["npc"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "city_info")
		{
			if (FALSE == oJsonReader.parse(item["city_info"]["S"].asString(),m_jCity_info))
			{
				m_jCity_info = Json::Value(Json::nullValue);
			}
			continue;
		}
		if (vecMembers[i] == "em_lv")
		{
			m_nEm_lv = strtoll(item["em_lv"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "cid")
		{
			m_nCid = strtoll(item["cid"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "uname")
		{
			m_sUname = item["uname"]["S"].asString();
			continue;
		}
		if (vecMembers[i] == "ulevel")
		{
			m_nUlevel = strtoll(item["ulevel"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "alid")
		{
			m_nAlid = strtoll(item["alid"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "alname")
		{
			m_sAlname = item["alname"]["S"].asString();
			continue;
		}
		if (vecMembers[i] == "al_nick")
		{
			m_sAl_nick = item["al_nick"]["S"].asString();
			continue;
		}
		if (vecMembers[i] == "al_flag")
		{
			m_nAl_flag = strtoll(item["al_flag"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "vip_level")
		{
			m_nVip_level = strtoll(item["vip_level"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "vip_etime")
		{
			m_nVip_etime = strtoll(item["vip_etime"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "center_pos")
		{
			m_nCenter_pos = strtoll(item["center_pos"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "name_update_time")
		{
			m_nName_update_time = strtoll(item["name_update_time"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "al_pos")
		{
			m_nAl_pos = strtoll(item["al_pos"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "avatar")
		{
			m_nAvatar = strtoll(item["avatar"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "age")
		{
			m_nAge = strtoll(item["age"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "prison_flag")
		{
			m_nPrison_flag = strtoll(item["prison_flag"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "time_end")
		{
			m_nTime_end = strtoll(item["time_end"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "pic_index")
		{
			m_nPic_index = strtoll(item["pic_index"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "rally_troop_limit")
		{
			m_nRally_troop_limit = strtoll(item["rally_troop_limit"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "smoke_end_time")
		{
			m_nSmoke_end_time = strtoll(item["smoke_end_time"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "burn_end_time")
		{
			m_nBurn_end_time = strtoll(item["burn_end_time"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "march_status_time")
		{
			m_nMarch_status_time = strtoll(item["march_status_time"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "move_city")
		{
			m_nMove_city = strtoll(item["move_city"]["N"].asString().c_str(), NULL, 10);
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
		if (vecMembers[i] == "res_rate")
		{
			m_nRes_rate = strtoll(item["res_rate"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "res_time")
		{
			m_nRes_time = strtoll(item["res_time"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "reward_left")
		{
			m_nReward_left = strtoll(item["reward_left"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "showtime")
		{
			m_nShowtime = strtoll(item["showtime"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "wild_gen_time")
		{
			m_nWild_gen_time = strtoll(item["wild_gen_time"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "boss_life")
		{
			m_nBoss_life = strtoll(item["boss_life"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "attack_info")
		{
			Base64Decode(item["attack_info"]["B"].asString(), (char*)&m_bAttack_info.m_astList[0], dwResLen);
			m_bAttack_info.m_udwNum = dwResLen/sizeof(SAttackTimesInfo);
			continue;
		}
		if (vecMembers[i] == "tax_rate_id")
		{
			m_nTax_rate_id = strtoll(item["tax_rate_id"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "force_kill")
		{
			m_nForce_kill = strtoll(item["force_kill"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "al_attack_info")
		{
			Base64Decode(item["al_attack_info"]["B"].asString(), (char*)&m_bAl_attack_info.m_astList[0], dwResLen);
			m_bAl_attack_info.m_udwNum = dwResLen/sizeof(SAttackTimesInfo);
			continue;
		}
		if (vecMembers[i] == "leader_monster_flag")
		{
			m_nLeader_monster_flag = strtoll(item["leader_monster_flag"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "occupy_num")
		{
			m_nOccupy_num = strtoll(item["occupy_num"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "occupy_clean_flag")
		{
			m_nOccupy_clean_flag = strtoll(item["occupy_clean_flag"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "seq")
		{
			m_nSeq = strtoll(item["seq"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "expire_time")
		{
			m_nExpire_time = strtoll(item["expire_time"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
	}
	return 0;
}

TINT64 TbMap::GetSeq()
{
	return m_nSeq;
}

