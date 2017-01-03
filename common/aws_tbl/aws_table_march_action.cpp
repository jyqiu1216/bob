#include "aws_table_march_action.h"

TableDesc TbMarch_action::oTableDesc;

int TbMarch_action::Init(const string& sConfFile, const string strProjectName)
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

AwsTable* TbMarch_action::NewObject()
{
	return new TbMarch_action;
}

string TbMarch_action::GetTableName()
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

TINT32 TbMarch_action::GetTableIdx()
{
	 return 0;
}

AwsMap* TbMarch_action::OnScanReq(unsigned int udwIdxNo, bool bHasStartKey, bool bReturnConsumedCapacity,
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
		pScan->AddValue("/ExclusiveStartKey/suid/N",m_nSuid);
		pScan->AddValue("/ExclusiveStartKey/id/N",m_nId);
	}
	return pScan;
}

int TbMarch_action::OnScanReq(string& sPostData, unsigned int udwIdxNo, bool bHasStartKey, bool bReturnConsumedCapacity,
 unsigned int dwLimit, unsigned int dwSegment, unsigned int dwTotalSegments)
{
	AwsMap* pScan = OnScanReq(udwIdxNo, bHasStartKey, bReturnConsumedCapacity, dwLimit, dwSegment, dwTotalSegments);
	ostringstream oss;
	pScan->Dump(oss);
	sPostData = oss.str();
	delete pScan;
	return 0;
}

int TbMarch_action::OnScanRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbMarch_action::OnQueryReq(unsigned int udwIdxNo, const CompareDesc& comp_desc,
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
			if(fld_desc.udwFldNo == TbMARCH_ACTION_FIELD_SUID)
			{
				pQuery->AddValue("/KeyConditions/suid/AttributeValueList[0]/N", m_nSuid);
				pQuery->AddValue("/KeyConditions/suid/ComparisonOperator", "EQ");
			}
			if(fld_desc.udwFldNo == TbMARCH_ACTION_FIELD_SID)
			{
				pQuery->AddValue("/KeyConditions/sid/AttributeValueList[0]/N", m_nSid);
				pQuery->AddValue("/KeyConditions/sid/ComparisonOperator", "EQ");
			}
			if(fld_desc.udwFldNo == TbMARCH_ACTION_FIELD_SAL)
			{
				pQuery->AddValue("/KeyConditions/sal/AttributeValueList[0]/N", m_nSal);
				pQuery->AddValue("/KeyConditions/sal/ComparisonOperator", "EQ");
			}
			if(fld_desc.udwFldNo == TbMARCH_ACTION_FIELD_TAL)
			{
				pQuery->AddValue("/KeyConditions/tal/AttributeValueList[0]/N", m_nTal);
				pQuery->AddValue("/KeyConditions/tal/ComparisonOperator", "EQ");
			}
		}
		else if (i == 1) //1只能是rangekey or local index，可以有多种方式
		{
			if (COMPARE_TYPE_EQ<=comp_desc.dwCompareType && comp_desc.dwCompareType<=COMPARE_TYPE_GT)
			{
				if(fld_desc.udwFldNo == TbMARCH_ACTION_FIELD_ID)
				{
					pQuery->AddValue("/KeyConditions/id/AttributeValueList[0]/N", m_nId);
					pQuery->AddValue("/KeyConditions/id/ComparisonOperator", CompareType2Str(comp_desc.dwCompareType));
				}
				if(fld_desc.udwFldNo == TbMARCH_ACTION_FIELD_ETIME)
				{
					pQuery->AddValue("/KeyConditions/etime/AttributeValueList[0]/N", m_nEtime);
					pQuery->AddValue("/KeyConditions/etime/ComparisonOperator", CompareType2Str(comp_desc.dwCompareType));
				}
				if(fld_desc.udwFldNo == TbMARCH_ACTION_FIELD_SBID)
				{
					pQuery->AddValue("/KeyConditions/sbid/AttributeValueList[0]/N", m_nSbid);
					pQuery->AddValue("/KeyConditions/sbid/ComparisonOperator", CompareType2Str(comp_desc.dwCompareType));
				}
				if(fld_desc.udwFldNo == TbMARCH_ACTION_FIELD_TBID)
				{
					pQuery->AddValue("/KeyConditions/tbid/AttributeValueList[0]/N", m_nTbid);
					pQuery->AddValue("/KeyConditions/tbid/ComparisonOperator", CompareType2Str(comp_desc.dwCompareType));
				}
			}
			else if (COMPARE_TYPE_BETWEEN == comp_desc.dwCompareType)
			{
				if(fld_desc.udwFldNo == TbMARCH_ACTION_FIELD_ID)
				{
					pQuery->AddValue("/KeyConditions/id/AttributeValueList[0]/N" , comp_desc.vecN[0]);
					pQuery->AddValue("/KeyConditions/id/AttributeValueList[1]/N" , comp_desc.vecN[1]);
					pQuery->AddValue("/KeyConditions/id/ComparisonOperator", "BETWEEN");
				}
				if(fld_desc.udwFldNo == TbMARCH_ACTION_FIELD_ETIME)
				{
					pQuery->AddValue("/KeyConditions/etime/AttributeValueList[0]/N" , comp_desc.vecN[0]);
					pQuery->AddValue("/KeyConditions/etime/AttributeValueList[1]/N" , comp_desc.vecN[1]);
					pQuery->AddValue("/KeyConditions/etime/ComparisonOperator", "BETWEEN");
				}
				if(fld_desc.udwFldNo == TbMARCH_ACTION_FIELD_SBID)
				{
					pQuery->AddValue("/KeyConditions/sbid/AttributeValueList[0]/N" , comp_desc.vecN[0]);
					pQuery->AddValue("/KeyConditions/sbid/AttributeValueList[1]/N" , comp_desc.vecN[1]);
					pQuery->AddValue("/KeyConditions/sbid/ComparisonOperator", "BETWEEN");
				}
				if(fld_desc.udwFldNo == TbMARCH_ACTION_FIELD_TBID)
				{
					pQuery->AddValue("/KeyConditions/tbid/AttributeValueList[0]/N" , comp_desc.vecN[0]);
					pQuery->AddValue("/KeyConditions/tbid/AttributeValueList[1]/N" , comp_desc.vecN[1]);
					pQuery->AddValue("/KeyConditions/tbid/ComparisonOperator", "BETWEEN");
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

int TbMarch_action::OnQueryReq(string& sPostData, unsigned int udwIdxNo, const CompareDesc& comp_desc,
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

int TbMarch_action::OnQueryRsp(const Json::Value& item)
{
	return OnResponse(item);
}

int TbMarch_action::OnCountReq(string& sPostData, unsigned int udwIdxNo, const CompareDesc& comp_desc, 
		bool bConsistentRead, bool bReturnConsumedCapacity, bool bScanIndexForward, unsigned int dwLimit)
{
	AwsMap* pQuery = OnQueryReq(udwIdxNo, comp_desc, bConsistentRead, bReturnConsumedCapacity, bScanIndexForward, dwLimit, true);
	ostringstream oss;
	pQuery->Dump(oss);
	sPostData = oss.str();
	delete pQuery;
	return 0;
}

AwsMap* TbMarch_action::OnUpdateItemReq(
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
	pUpdateItem->AddValue("/Key/suid/N", m_nSuid);
	pUpdateItem->AddValue("/Key/id/N", m_nId);
	for (map<unsigned int, int>::iterator iter = m_mFlag.begin(); iter != m_mFlag.end(); ++iter)
	{
		bUpdateFlag = true;
		if (TbMARCH_ACTION_FIELD_SCID == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/scid/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/scid/Value/N", m_nScid);
				pUpdateItem->AddValue("/AttributeUpdates/scid/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMARCH_ACTION_FIELD_FIXED == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/fixed/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/fixed/Value/N", m_nFixed);
				pUpdateItem->AddValue("/AttributeUpdates/fixed/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMARCH_ACTION_FIELD_MCLASS == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/mclass/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/mclass/Value/N", m_nMclass);
				pUpdateItem->AddValue("/AttributeUpdates/mclass/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMARCH_ACTION_FIELD_SCLASS == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/sclass/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/sclass/Value/N", m_nSclass);
				pUpdateItem->AddValue("/AttributeUpdates/sclass/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMARCH_ACTION_FIELD_STATUS == iter->first)
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
		if (TbMARCH_ACTION_FIELD_BTIME == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/btime/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/btime/Value/N", m_nBtime);
				pUpdateItem->AddValue("/AttributeUpdates/btime/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMARCH_ACTION_FIELD_CTIME == iter->first)
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
		if (TbMARCH_ACTION_FIELD_ETIME == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/etime/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/etime/Value/N", m_nEtime);
				pUpdateItem->AddValue("/AttributeUpdates/etime/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMARCH_ACTION_FIELD_PARAM == iter->first)
		{
			if (!m_bParam.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/param/Value/B", Base64Encode((char*)&m_bParam.m_astList[0], m_bParam.m_udwNum*sizeof(SActionMarchParam), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/param/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/param/Action", "DELETE");
			}
			continue;
		}
		if (TbMARCH_ACTION_FIELD_RETRY == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/retry/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/retry/Value/N", m_nRetry);
				pUpdateItem->AddValue("/AttributeUpdates/retry/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMARCH_ACTION_FIELD_SID == iter->first)
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
		if (TbMARCH_ACTION_FIELD_NOTIC_TASK_FLAG == iter->first)
		{
			if (!m_bNotic_task_flag.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/notic_task_Flag/Value/B", Base64Encode((char*)&m_bNotic_task_flag.m_astList[0], m_bNotic_task_flag.m_udwNum*sizeof(SNotictaskFlag), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/notic_task_Flag/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/notic_task_Flag/Action", "DELETE");
			}
			continue;
		}
		if (TbMARCH_ACTION_FIELD_SAL == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/sal/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/sal/Value/N", m_nSal);
				pUpdateItem->AddValue("/AttributeUpdates/sal/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMARCH_ACTION_FIELD_TAL == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/tal/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/tal/Value/N", m_nTal);
				pUpdateItem->AddValue("/AttributeUpdates/tal/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMARCH_ACTION_FIELD_TID == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/tid/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/tid/Value/N", m_nTid);
				pUpdateItem->AddValue("/AttributeUpdates/tid/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMARCH_ACTION_FIELD_SBID == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/sbid/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/sbid/Value/N", m_nSbid);
				pUpdateItem->AddValue("/AttributeUpdates/sbid/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMARCH_ACTION_FIELD_TBID == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/tbid/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/tbid/Value/N", m_nTbid);
				pUpdateItem->AddValue("/AttributeUpdates/tbid/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMARCH_ACTION_FIELD_KILL_TROOP_MIGHT == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/kill_troop_might/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/kill_troop_might/Value/N", m_nKill_troop_might);
				pUpdateItem->AddValue("/AttributeUpdates/kill_troop_might/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMARCH_ACTION_FIELD_KILL_FORT_MIGHT == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/kill_fort_might/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/kill_fort_might/Value/N", m_nKill_fort_might);
				pUpdateItem->AddValue("/AttributeUpdates/kill_fort_might/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMARCH_ACTION_FIELD_BUFF == iter->first)
		{
			if (!m_bBuff.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/buff/Value/B", Base64Encode((char*)&m_bBuff.m_astList[0], m_bBuff.m_udwNum*sizeof(SBuffInfo), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/buff/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/buff/Action", "DELETE");
			}
			continue;
		}
		if (TbMARCH_ACTION_FIELD_EXPIRING_BUFF == iter->first)
		{
			if (!m_bExpiring_buff.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/expiring_buff/Value/B", Base64Encode((char*)&m_bExpiring_buff.m_astList[0], m_bExpiring_buff.m_udwNum*sizeof(SBuffInfo), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/expiring_buff/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/expiring_buff/Action", "DELETE");
			}
			continue;
		}
		if (TbMARCH_ACTION_FIELD_REWARD == iter->first)
		{
			if (!m_bReward.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/reward/Value/B", Base64Encode((char*)&m_bReward.m_astList[0], m_bReward.m_udwNum*sizeof(SGlobalRes), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/reward/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/reward/Action", "DELETE");
			}
			continue;
		}
		if (TbMARCH_ACTION_FIELD_SP_REWARD == iter->first)
		{
			if (!m_bSp_reward.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/sp_reward/Value/B", Base64Encode((char*)&m_bSp_reward.m_astList[0], m_bSp_reward.m_udwNum*sizeof(SGlobalRes), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/sp_reward/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/sp_reward/Action", "DELETE");
			}
			continue;
		}
		if (TbMARCH_ACTION_FIELD_TPOS == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/tpos/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/tpos/Value/N", m_nTpos);
				pUpdateItem->AddValue("/AttributeUpdates/tpos/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMARCH_ACTION_FIELD_TUID == iter->first)
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
		if (TbMARCH_ACTION_FIELD_IF_MAX_ATTACK == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/if_max_attack/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/if_max_attack/Value/N", m_nIf_max_attack);
				pUpdateItem->AddValue("/AttributeUpdates/if_max_attack/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMARCH_ACTION_FIELD_RALLY_ATK_FORCE == iter->first)
		{
			if (!m_bRally_atk_force.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/rally_atk_force/Value/B", Base64Encode((char*)&m_bRally_atk_force.m_astList[0], m_bRally_atk_force.m_udwNum*sizeof(SRallyForce), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/rally_atk_force/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/rally_atk_force/Action", "DELETE");
			}
			continue;
		}
		if (TbMARCH_ACTION_FIELD_RALLY_ATK_SLOT == iter->first)
		{
			if (!m_bRally_atk_slot.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/rally_atk_slot/Value/B", Base64Encode((char*)&m_bRally_atk_slot.m_astList[0], m_bRally_atk_slot.m_udwNum*sizeof(SRallySlot), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/rally_atk_slot/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/rally_atk_slot/Action", "DELETE");
			}
			continue;
		}
		if (TbMARCH_ACTION_FIELD_RALLY_DEF_FORCE == iter->first)
		{
			if (!m_bRally_def_force.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/rally_def_force/Value/B", Base64Encode((char*)&m_bRally_def_force.m_astList[0], m_bRally_def_force.m_udwNum*sizeof(SRallyForce), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/rally_def_force/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/rally_def_force/Action", "DELETE");
			}
			continue;
		}
		if (TbMARCH_ACTION_FIELD_RALLY_DEF_SLOT == iter->first)
		{
			if (!m_bRally_def_slot.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/rally_def_slot/Value/B", Base64Encode((char*)&m_bRally_def_slot.m_astList[0], m_bRally_def_slot.m_udwNum*sizeof(SRallySlot), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/rally_def_slot/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/rally_def_slot/Action", "DELETE");
			}
			continue;
		}
		if (TbMARCH_ACTION_FIELD_PRISON_PARAM == iter->first)
		{
			if (!m_bPrison_param.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/prison_param/Value/B", Base64Encode((char*)&m_bPrison_param.m_astList[0], m_bPrison_param.m_udwNum*sizeof(SPrisonParam), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/prison_param/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/prison_param/Action", "DELETE");
			}
			continue;
		}
		if (TbMARCH_ACTION_FIELD_CITY_INFO == iter->first)
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
		if (TbMARCH_ACTION_FIELD_REINFORCE_RESULT == iter->first)
		{
			if (!m_bReinforce_result.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/reinforce_result/Value/B", Base64Encode((char*)&m_bReinforce_result.m_astList[0], m_bReinforce_result.m_udwNum*sizeof(SReinforceResult), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/reinforce_result/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/reinforce_result/Action", "DELETE");
			}
			continue;
		}
		if (TbMARCH_ACTION_FIELD_KILL_TROOP_NUM == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/kill_troop_num/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/kill_troop_num/Value/N", m_nKill_troop_num);
				pUpdateItem->AddValue("/AttributeUpdates/kill_troop_num/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMARCH_ACTION_FIELD_KILL_FORT_NUM == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/kill_fort_num/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/kill_fort_num/Value/N", m_nKill_fort_num);
				pUpdateItem->AddValue("/AttributeUpdates/kill_fort_num/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMARCH_ACTION_FIELD_IS_RECALLED == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/is_recalled/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/is_recalled/Value/N", m_nIs_recalled);
				pUpdateItem->AddValue("/AttributeUpdates/is_recalled/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMARCH_ACTION_FIELD_SAVATAR == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/savatar/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/savatar/Value/N", m_nSavatar);
				pUpdateItem->AddValue("/AttributeUpdates/savatar/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMARCH_ACTION_FIELD_AL_GIFT_RECORD == iter->first)
		{
			if (!m_bAl_gift_record.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/al_gift_record/Value/B", Base64Encode((char*)&m_bAl_gift_record.m_astList[0], m_bAl_gift_record.m_udwNum*sizeof(SGlobalRes), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/al_gift_record/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/al_gift_record/Action", "DELETE");
			}
			continue;
		}
		if (TbMARCH_ACTION_FIELD_ATK_TOTAL_TROOP == iter->first)
		{
			if (!m_bAtk_total_troop.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/atk_total_troop/Value/B", Base64Encode((char*)&m_bAtk_total_troop.m_astList[0], m_bAtk_total_troop.m_udwNum*sizeof(SCommonTroop), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/atk_total_troop/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/atk_total_troop/Action", "DELETE");
			}
			continue;
		}
		if (TbMARCH_ACTION_FIELD_TRADE_INFO == iter->first)
		{
			if (!m_bTrade_info.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/trade_info/Value/B", Base64Encode((char*)&m_bTrade_info.m_astList[0], m_bTrade_info.m_udwNum*sizeof(STradeInfo), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/trade_info/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/trade_info/Action", "DELETE");
			}
			continue;
		}
		if (TbMARCH_ACTION_FIELD_TAX_INFO == iter->first)
		{
			if (!m_jTax_info.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/tax_info/Value/S", JsonEncode(oJsonWriter.write(m_jTax_info), sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/tax_info/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/tax_info/Action", "DELETE");
			}
			continue;
		}
		if (TbMARCH_ACTION_FIELD_TRADE_LIST_IDX == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/trade_list_idx/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/trade_list_idx/Value/N", m_nTrade_list_idx);
				pUpdateItem->AddValue("/AttributeUpdates/trade_list_idx/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMARCH_ACTION_FIELD_MONSTER_INFO == iter->first)
		{
			if (!m_bMonster_info.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/monster_info/Value/B", Base64Encode((char*)&m_bMonster_info.m_astList[0], m_bMonster_info.m_udwNum*sizeof(SMonsterInfo), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/monster_info/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/monster_info/Action", "DELETE");
			}
			continue;
		}
		if (TbMARCH_ACTION_FIELD_DELAY_REPORT_ID == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/delay_report_id/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/delay_report_id/Value/N", m_nDelay_report_id);
				pUpdateItem->AddValue("/AttributeUpdates/delay_report_id/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMARCH_ACTION_FIELD_SEQ == iter->first)
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
		if (TbMARCH_ACTION_FIELD_DEF_TOTAL_TROOP == iter->first)
		{
			if (!m_bDef_total_troop.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/def_total_troop/Value/B", Base64Encode((char*)&m_bDef_total_troop.m_astList[0], m_bDef_total_troop.m_udwNum*sizeof(SCommonTroop), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/def_total_troop/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/def_total_troop/Action", "DELETE");
			}
			continue;
		}
		if (TbMARCH_ACTION_FIELD_EX_REWARD == iter->first)
		{
			if (!m_bEx_reward.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/ex_reward/Value/B", Base64Encode((char*)&m_bEx_reward.m_astList[0], m_bEx_reward.m_udwNum*sizeof(SOneGlobalRes), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/ex_reward/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/ex_reward/Action", "DELETE");
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

int TbMarch_action::OnUpdateItemReq(string& sPostData,
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

int TbMarch_action::OnUpdateItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbMarch_action::OnWriteItemReq(int dwActionType)
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
		pItem->AddValue("/id/N", m_nId);
		pItem->AddValue("/suid/N", m_nSuid);
		pItem->AddValue("/scid/N", m_nScid);
		pItem->AddValue("/fixed/N", m_nFixed);
		pItem->AddValue("/mclass/N", m_nMclass);
		pItem->AddValue("/sclass/N", m_nSclass);
		pItem->AddValue("/status/N", m_nStatus);
		pItem->AddValue("/btime/N", m_nBtime);
		pItem->AddValue("/ctime/N", m_nCtime);
		pItem->AddValue("/etime/N", m_nEtime);
		if (!m_bParam.empty())
		{
			pItem->AddValue("/param/B", Base64Encode((char*)&m_bParam.m_astList[0], m_bParam.m_udwNum*sizeof(SActionMarchParam), sBase64Encode));
		}
		pItem->AddValue("/retry/N", m_nRetry);
		pItem->AddValue("/sid/N", m_nSid);
		if (!m_bNotic_task_flag.empty())
		{
			pItem->AddValue("/notic_task_Flag/B", Base64Encode((char*)&m_bNotic_task_flag.m_astList[0], m_bNotic_task_flag.m_udwNum*sizeof(SNotictaskFlag), sBase64Encode));
		}
		pItem->AddValue("/sal/N", m_nSal);
		pItem->AddValue("/tal/N", m_nTal);
		pItem->AddValue("/tid/N", m_nTid);
		pItem->AddValue("/sbid/N", m_nSbid);
		pItem->AddValue("/tbid/N", m_nTbid);
		pItem->AddValue("/kill_troop_might/N", m_nKill_troop_might);
		pItem->AddValue("/kill_fort_might/N", m_nKill_fort_might);
		if (!m_bBuff.empty())
		{
			pItem->AddValue("/buff/B", Base64Encode((char*)&m_bBuff.m_astList[0], m_bBuff.m_udwNum*sizeof(SBuffInfo), sBase64Encode));
		}
		if (!m_bExpiring_buff.empty())
		{
			pItem->AddValue("/expiring_buff/B", Base64Encode((char*)&m_bExpiring_buff.m_astList[0], m_bExpiring_buff.m_udwNum*sizeof(SBuffInfo), sBase64Encode));
		}
		if (!m_bReward.empty())
		{
			pItem->AddValue("/reward/B", Base64Encode((char*)&m_bReward.m_astList[0], m_bReward.m_udwNum*sizeof(SGlobalRes), sBase64Encode));
		}
		if (!m_bSp_reward.empty())
		{
			pItem->AddValue("/sp_reward/B", Base64Encode((char*)&m_bSp_reward.m_astList[0], m_bSp_reward.m_udwNum*sizeof(SGlobalRes), sBase64Encode));
		}
		pItem->AddValue("/tpos/N", m_nTpos);
		pItem->AddValue("/tuid/N", m_nTuid);
		pItem->AddValue("/if_max_attack/N", m_nIf_max_attack);
		if (!m_bRally_atk_force.empty())
		{
			pItem->AddValue("/rally_atk_force/B", Base64Encode((char*)&m_bRally_atk_force.m_astList[0], m_bRally_atk_force.m_udwNum*sizeof(SRallyForce), sBase64Encode));
		}
		if (!m_bRally_atk_slot.empty())
		{
			pItem->AddValue("/rally_atk_slot/B", Base64Encode((char*)&m_bRally_atk_slot.m_astList[0], m_bRally_atk_slot.m_udwNum*sizeof(SRallySlot), sBase64Encode));
		}
		if (!m_bRally_def_force.empty())
		{
			pItem->AddValue("/rally_def_force/B", Base64Encode((char*)&m_bRally_def_force.m_astList[0], m_bRally_def_force.m_udwNum*sizeof(SRallyForce), sBase64Encode));
		}
		if (!m_bRally_def_slot.empty())
		{
			pItem->AddValue("/rally_def_slot/B", Base64Encode((char*)&m_bRally_def_slot.m_astList[0], m_bRally_def_slot.m_udwNum*sizeof(SRallySlot), sBase64Encode));
		}
		if (!m_bPrison_param.empty())
		{
			pItem->AddValue("/prison_param/B", Base64Encode((char*)&m_bPrison_param.m_astList[0], m_bPrison_param.m_udwNum*sizeof(SPrisonParam), sBase64Encode));
		}
		if (!m_jCity_info.empty())
		{
				pItem->AddValue("/AttributeUpdates/city_info/Value/S", JsonEncode(oJsonWriter.write(m_jCity_info), sJsonEncode));
		}
		if (!m_bReinforce_result.empty())
		{
			pItem->AddValue("/reinforce_result/B", Base64Encode((char*)&m_bReinforce_result.m_astList[0], m_bReinforce_result.m_udwNum*sizeof(SReinforceResult), sBase64Encode));
		}
		pItem->AddValue("/kill_troop_num/N", m_nKill_troop_num);
		pItem->AddValue("/kill_fort_num/N", m_nKill_fort_num);
		pItem->AddValue("/is_recalled/N", m_nIs_recalled);
		pItem->AddValue("/savatar/N", m_nSavatar);
		if (!m_bAl_gift_record.empty())
		{
			pItem->AddValue("/al_gift_record/B", Base64Encode((char*)&m_bAl_gift_record.m_astList[0], m_bAl_gift_record.m_udwNum*sizeof(SGlobalRes), sBase64Encode));
		}
		if (!m_bAtk_total_troop.empty())
		{
			pItem->AddValue("/atk_total_troop/B", Base64Encode((char*)&m_bAtk_total_troop.m_astList[0], m_bAtk_total_troop.m_udwNum*sizeof(SCommonTroop), sBase64Encode));
		}
		if (!m_bTrade_info.empty())
		{
			pItem->AddValue("/trade_info/B", Base64Encode((char*)&m_bTrade_info.m_astList[0], m_bTrade_info.m_udwNum*sizeof(STradeInfo), sBase64Encode));
		}
		if (!m_jTax_info.empty())
		{
				pItem->AddValue("/AttributeUpdates/tax_info/Value/S", JsonEncode(oJsonWriter.write(m_jTax_info), sJsonEncode));
		}
		pItem->AddValue("/trade_list_idx/N", m_nTrade_list_idx);
		if (!m_bMonster_info.empty())
		{
			pItem->AddValue("/monster_info/B", Base64Encode((char*)&m_bMonster_info.m_astList[0], m_bMonster_info.m_udwNum*sizeof(SMonsterInfo), sBase64Encode));
		}
		pItem->AddValue("/delay_report_id/N", m_nDelay_report_id);
		pItem->AddValue("/seq/N", m_nSeq);
		if (!m_bDef_total_troop.empty())
		{
			pItem->AddValue("/def_total_troop/B", Base64Encode((char*)&m_bDef_total_troop.m_astList[0], m_bDef_total_troop.m_udwNum*sizeof(SCommonTroop), sBase64Encode));
		}
		if (!m_bEx_reward.empty())
		{
			pItem->AddValue("/ex_reward/B", Base64Encode((char*)&m_bEx_reward.m_astList[0], m_bEx_reward.m_udwNum*sizeof(SOneGlobalRes), sBase64Encode));
		}
	}
	else if (WRITE_ACTION_TYPE_DELETE == dwActionType)
	{
		pItem = pWriteItem->GetAwsMap("DeleteRequest")->GetAwsMap("Key");
		pItem->AddValue("/suid/N", m_nSuid);
		pItem->AddValue("/id/N", m_nId);
	}
	else
	{
		assert(0);
	}
	return pWriteItem;
}

void TbMarch_action::OnWriteItemReq(AwsMap* pWriteItem, int dwActionType)
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
		pItem->AddValue("/id/N", m_nId);
		pItem->AddValue("/suid/N", m_nSuid);
		pItem->AddValue("/scid/N", m_nScid);
		pItem->AddValue("/fixed/N", m_nFixed);
		pItem->AddValue("/mclass/N", m_nMclass);
		pItem->AddValue("/sclass/N", m_nSclass);
		pItem->AddValue("/status/N", m_nStatus);
		pItem->AddValue("/btime/N", m_nBtime);
		pItem->AddValue("/ctime/N", m_nCtime);
		pItem->AddValue("/etime/N", m_nEtime);
		if (!m_bParam.empty())
		{
			pItem->AddValue("/param/B", Base64Encode((char*)&m_bParam.m_astList[0], m_bParam.m_udwNum*sizeof(SActionMarchParam), sBase64Encode));
		}
		pItem->AddValue("/retry/N", m_nRetry);
		pItem->AddValue("/sid/N", m_nSid);
		if (!m_bNotic_task_flag.empty())
		{
			pItem->AddValue("/notic_task_Flag/B", Base64Encode((char*)&m_bNotic_task_flag.m_astList[0], m_bNotic_task_flag.m_udwNum*sizeof(SNotictaskFlag), sBase64Encode));
		}
		pItem->AddValue("/sal/N", m_nSal);
		pItem->AddValue("/tal/N", m_nTal);
		pItem->AddValue("/tid/N", m_nTid);
		pItem->AddValue("/sbid/N", m_nSbid);
		pItem->AddValue("/tbid/N", m_nTbid);
		pItem->AddValue("/kill_troop_might/N", m_nKill_troop_might);
		pItem->AddValue("/kill_fort_might/N", m_nKill_fort_might);
		if (!m_bBuff.empty())
		{
			pItem->AddValue("/buff/B", Base64Encode((char*)&m_bBuff.m_astList[0], m_bBuff.m_udwNum*sizeof(SBuffInfo), sBase64Encode));
		}
		if (!m_bExpiring_buff.empty())
		{
			pItem->AddValue("/expiring_buff/B", Base64Encode((char*)&m_bExpiring_buff.m_astList[0], m_bExpiring_buff.m_udwNum*sizeof(SBuffInfo), sBase64Encode));
		}
		if (!m_bReward.empty())
		{
			pItem->AddValue("/reward/B", Base64Encode((char*)&m_bReward.m_astList[0], m_bReward.m_udwNum*sizeof(SGlobalRes), sBase64Encode));
		}
		if (!m_bSp_reward.empty())
		{
			pItem->AddValue("/sp_reward/B", Base64Encode((char*)&m_bSp_reward.m_astList[0], m_bSp_reward.m_udwNum*sizeof(SGlobalRes), sBase64Encode));
		}
		pItem->AddValue("/tpos/N", m_nTpos);
		pItem->AddValue("/tuid/N", m_nTuid);
		pItem->AddValue("/if_max_attack/N", m_nIf_max_attack);
		if (!m_bRally_atk_force.empty())
		{
			pItem->AddValue("/rally_atk_force/B", Base64Encode((char*)&m_bRally_atk_force.m_astList[0], m_bRally_atk_force.m_udwNum*sizeof(SRallyForce), sBase64Encode));
		}
		if (!m_bRally_atk_slot.empty())
		{
			pItem->AddValue("/rally_atk_slot/B", Base64Encode((char*)&m_bRally_atk_slot.m_astList[0], m_bRally_atk_slot.m_udwNum*sizeof(SRallySlot), sBase64Encode));
		}
		if (!m_bRally_def_force.empty())
		{
			pItem->AddValue("/rally_def_force/B", Base64Encode((char*)&m_bRally_def_force.m_astList[0], m_bRally_def_force.m_udwNum*sizeof(SRallyForce), sBase64Encode));
		}
		if (!m_bRally_def_slot.empty())
		{
			pItem->AddValue("/rally_def_slot/B", Base64Encode((char*)&m_bRally_def_slot.m_astList[0], m_bRally_def_slot.m_udwNum*sizeof(SRallySlot), sBase64Encode));
		}
		if (!m_bPrison_param.empty())
		{
			pItem->AddValue("/prison_param/B", Base64Encode((char*)&m_bPrison_param.m_astList[0], m_bPrison_param.m_udwNum*sizeof(SPrisonParam), sBase64Encode));
		}
		if (!m_jCity_info.empty())
		{
				pItem->AddValue("/AttributeUpdates/city_info/Value/S", JsonEncode(oJsonWriter.write(m_jCity_info), sJsonEncode));
		}
		if (!m_bReinforce_result.empty())
		{
			pItem->AddValue("/reinforce_result/B", Base64Encode((char*)&m_bReinforce_result.m_astList[0], m_bReinforce_result.m_udwNum*sizeof(SReinforceResult), sBase64Encode));
		}
		pItem->AddValue("/kill_troop_num/N", m_nKill_troop_num);
		pItem->AddValue("/kill_fort_num/N", m_nKill_fort_num);
		pItem->AddValue("/is_recalled/N", m_nIs_recalled);
		pItem->AddValue("/savatar/N", m_nSavatar);
		if (!m_bAl_gift_record.empty())
		{
			pItem->AddValue("/al_gift_record/B", Base64Encode((char*)&m_bAl_gift_record.m_astList[0], m_bAl_gift_record.m_udwNum*sizeof(SGlobalRes), sBase64Encode));
		}
		if (!m_bAtk_total_troop.empty())
		{
			pItem->AddValue("/atk_total_troop/B", Base64Encode((char*)&m_bAtk_total_troop.m_astList[0], m_bAtk_total_troop.m_udwNum*sizeof(SCommonTroop), sBase64Encode));
		}
		if (!m_bTrade_info.empty())
		{
			pItem->AddValue("/trade_info/B", Base64Encode((char*)&m_bTrade_info.m_astList[0], m_bTrade_info.m_udwNum*sizeof(STradeInfo), sBase64Encode));
		}
		if (!m_jTax_info.empty())
		{
				pItem->AddValue("/AttributeUpdates/tax_info/Value/S", JsonEncode(oJsonWriter.write(m_jTax_info), sJsonEncode));
		}
		pItem->AddValue("/trade_list_idx/N", m_nTrade_list_idx);
		if (!m_bMonster_info.empty())
		{
			pItem->AddValue("/monster_info/B", Base64Encode((char*)&m_bMonster_info.m_astList[0], m_bMonster_info.m_udwNum*sizeof(SMonsterInfo), sBase64Encode));
		}
		pItem->AddValue("/delay_report_id/N", m_nDelay_report_id);
		pItem->AddValue("/seq/N", m_nSeq);
		if (!m_bDef_total_troop.empty())
		{
			pItem->AddValue("/def_total_troop/B", Base64Encode((char*)&m_bDef_total_troop.m_astList[0], m_bDef_total_troop.m_udwNum*sizeof(SCommonTroop), sBase64Encode));
		}
		if (!m_bEx_reward.empty())
		{
			pItem->AddValue("/ex_reward/B", Base64Encode((char*)&m_bEx_reward.m_astList[0], m_bEx_reward.m_udwNum*sizeof(SOneGlobalRes), sBase64Encode));
		}
	}
	else if (WRITE_ACTION_TYPE_DELETE == dwActionType)
	{
		pItem = pReqItem->GetAwsMap("DeleteRequest")->GetAwsMap("Key");
		pItem->AddValue("/suid/N", m_nSuid);
		pItem->AddValue("/id/N", m_nId);
	}
	else
	{
		assert(0);
	}
	pWriteItem->GetAwsMap("RequestItems")->GetAwsList(GetTableName())->SetValue(pReqItem, true);
}

AwsMap* TbMarch_action::OnReadItemReq(unsigned int udwIdxNo)
{
	oJsonWriter.omitEndingLineFeed();
	IndexDesc& idx_desc = oTableDesc.mIndexDesc[udwIdxNo];
	assert(idx_desc.sName == "PRIMARY"); //只能通过主键查询
	AwsMap* pReadItem = new AwsMap;
	assert(pReadItem);
	pReadItem->AddValue("/Keys[0]/suid/N", m_nSuid);
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

void TbMarch_action::OnReadItemReq(AwsMap* pReadItem)
{
	assert(pReadItem);
	AwsList* pKeys = pReadItem->GetAwsMap("RequestItems")->GetAwsMap(GetTableName())->GetAwsList("Keys");
	AwsMap* pKey = new AwsMap;
	assert(pKey);
	pKey->AddValue("/suid/N", m_nSuid);
	pKey->AddValue("/id/N", m_nId);
	pKeys->SetValue(pKey, true);
}

int TbMarch_action::OnReadItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbMarch_action::OnDeleteItemReq(
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
	pDeleteItem->AddValue("/Key/suid/N", m_nSuid);
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

int TbMarch_action::OnDeleteItemReq(string& sPostData,
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

int TbMarch_action::OnDeleteItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbMarch_action::OnGetItemReq(unsigned int udwIdxNo,
	bool bConsistentRead, bool bReturnConsumedCapacity)
{
	oJsonWriter.omitEndingLineFeed();
	assert(oTableDesc.mIndexDesc.find(udwIdxNo)!=oTableDesc.mIndexDesc.end());
	IndexDesc& idx_desc = oTableDesc.mIndexDesc[udwIdxNo];
	assert(idx_desc.sName == "PRIMARY"); //只能通过主键查询
	AwsMap* pGetItem = new AwsMap;
	assert(pGetItem);
	pGetItem->AddValue("/TableName", GetTableName());
	pGetItem->AddValue("/Key/suid/N", m_nSuid);
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

int TbMarch_action::OnGetItemReq(string& sPostData, unsigned int udwIdxNo,
	bool bConsistentRead, bool bReturnConsumedCapacity)
{
	AwsMap* pGetItem = OnGetItemReq(udwIdxNo, bConsistentRead, bReturnConsumedCapacity);
	ostringstream oss;
	pGetItem->Dump(oss);
	sPostData = oss.str();
	delete pGetItem;
	return 0;
}

int TbMarch_action::OnGetItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbMarch_action::OnPutItemReq(
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
	pPutItem->AddValue("/Item/id/N", m_nId);
	pPutItem->AddValue("/Item/suid/N", m_nSuid);
	pPutItem->AddValue("/Item/scid/N", m_nScid);
	pPutItem->AddValue("/Item/fixed/N", m_nFixed);
	pPutItem->AddValue("/Item/mclass/N", m_nMclass);
	pPutItem->AddValue("/Item/sclass/N", m_nSclass);
	pPutItem->AddValue("/Item/status/N", m_nStatus);
	pPutItem->AddValue("/Item/btime/N", m_nBtime);
	pPutItem->AddValue("/Item/ctime/N", m_nCtime);
	pPutItem->AddValue("/Item/etime/N", m_nEtime);
	if (!m_bParam.empty())
	{
		pPutItem->AddValue("/Item/param/B", Base64Encode((char*)&m_bParam.m_astList[0], m_bParam.m_udwNum*sizeof(SActionMarchParam), sBase64Encode));
	}
	pPutItem->AddValue("/Item/retry/N", m_nRetry);
	pPutItem->AddValue("/Item/sid/N", m_nSid);
	if (!m_bNotic_task_flag.empty())
	{
		pPutItem->AddValue("/Item/notic_task_Flag/B", Base64Encode((char*)&m_bNotic_task_flag.m_astList[0], m_bNotic_task_flag.m_udwNum*sizeof(SNotictaskFlag), sBase64Encode));
	}
	pPutItem->AddValue("/Item/sal/N", m_nSal);
	pPutItem->AddValue("/Item/tal/N", m_nTal);
	pPutItem->AddValue("/Item/tid/N", m_nTid);
	pPutItem->AddValue("/Item/sbid/N", m_nSbid);
	pPutItem->AddValue("/Item/tbid/N", m_nTbid);
	pPutItem->AddValue("/Item/kill_troop_might/N", m_nKill_troop_might);
	pPutItem->AddValue("/Item/kill_fort_might/N", m_nKill_fort_might);
	if (!m_bBuff.empty())
	{
		pPutItem->AddValue("/Item/buff/B", Base64Encode((char*)&m_bBuff.m_astList[0], m_bBuff.m_udwNum*sizeof(SBuffInfo), sBase64Encode));
	}
	if (!m_bExpiring_buff.empty())
	{
		pPutItem->AddValue("/Item/expiring_buff/B", Base64Encode((char*)&m_bExpiring_buff.m_astList[0], m_bExpiring_buff.m_udwNum*sizeof(SBuffInfo), sBase64Encode));
	}
	if (!m_bReward.empty())
	{
		pPutItem->AddValue("/Item/reward/B", Base64Encode((char*)&m_bReward.m_astList[0], m_bReward.m_udwNum*sizeof(SGlobalRes), sBase64Encode));
	}
	if (!m_bSp_reward.empty())
	{
		pPutItem->AddValue("/Item/sp_reward/B", Base64Encode((char*)&m_bSp_reward.m_astList[0], m_bSp_reward.m_udwNum*sizeof(SGlobalRes), sBase64Encode));
	}
	pPutItem->AddValue("/Item/tpos/N", m_nTpos);
	pPutItem->AddValue("/Item/tuid/N", m_nTuid);
	pPutItem->AddValue("/Item/if_max_attack/N", m_nIf_max_attack);
	if (!m_bRally_atk_force.empty())
	{
		pPutItem->AddValue("/Item/rally_atk_force/B", Base64Encode((char*)&m_bRally_atk_force.m_astList[0], m_bRally_atk_force.m_udwNum*sizeof(SRallyForce), sBase64Encode));
	}
	if (!m_bRally_atk_slot.empty())
	{
		pPutItem->AddValue("/Item/rally_atk_slot/B", Base64Encode((char*)&m_bRally_atk_slot.m_astList[0], m_bRally_atk_slot.m_udwNum*sizeof(SRallySlot), sBase64Encode));
	}
	if (!m_bRally_def_force.empty())
	{
		pPutItem->AddValue("/Item/rally_def_force/B", Base64Encode((char*)&m_bRally_def_force.m_astList[0], m_bRally_def_force.m_udwNum*sizeof(SRallyForce), sBase64Encode));
	}
	if (!m_bRally_def_slot.empty())
	{
		pPutItem->AddValue("/Item/rally_def_slot/B", Base64Encode((char*)&m_bRally_def_slot.m_astList[0], m_bRally_def_slot.m_udwNum*sizeof(SRallySlot), sBase64Encode));
	}
	if (!m_bPrison_param.empty())
	{
		pPutItem->AddValue("/Item/prison_param/B", Base64Encode((char*)&m_bPrison_param.m_astList[0], m_bPrison_param.m_udwNum*sizeof(SPrisonParam), sBase64Encode));
	}
	if (!m_jCity_info.empty())
	{
				pPutItem->AddValue("/Item/city_info/S", JsonEncode(oJsonWriter.write(m_jCity_info), sJsonEncode));
	}
	if (!m_bReinforce_result.empty())
	{
		pPutItem->AddValue("/Item/reinforce_result/B", Base64Encode((char*)&m_bReinforce_result.m_astList[0], m_bReinforce_result.m_udwNum*sizeof(SReinforceResult), sBase64Encode));
	}
	pPutItem->AddValue("/Item/kill_troop_num/N", m_nKill_troop_num);
	pPutItem->AddValue("/Item/kill_fort_num/N", m_nKill_fort_num);
	pPutItem->AddValue("/Item/is_recalled/N", m_nIs_recalled);
	pPutItem->AddValue("/Item/savatar/N", m_nSavatar);
	if (!m_bAl_gift_record.empty())
	{
		pPutItem->AddValue("/Item/al_gift_record/B", Base64Encode((char*)&m_bAl_gift_record.m_astList[0], m_bAl_gift_record.m_udwNum*sizeof(SGlobalRes), sBase64Encode));
	}
	if (!m_bAtk_total_troop.empty())
	{
		pPutItem->AddValue("/Item/atk_total_troop/B", Base64Encode((char*)&m_bAtk_total_troop.m_astList[0], m_bAtk_total_troop.m_udwNum*sizeof(SCommonTroop), sBase64Encode));
	}
	if (!m_bTrade_info.empty())
	{
		pPutItem->AddValue("/Item/trade_info/B", Base64Encode((char*)&m_bTrade_info.m_astList[0], m_bTrade_info.m_udwNum*sizeof(STradeInfo), sBase64Encode));
	}
	if (!m_jTax_info.empty())
	{
				pPutItem->AddValue("/Item/tax_info/S", JsonEncode(oJsonWriter.write(m_jTax_info), sJsonEncode));
	}
	pPutItem->AddValue("/Item/trade_list_idx/N", m_nTrade_list_idx);
	if (!m_bMonster_info.empty())
	{
		pPutItem->AddValue("/Item/monster_info/B", Base64Encode((char*)&m_bMonster_info.m_astList[0], m_bMonster_info.m_udwNum*sizeof(SMonsterInfo), sBase64Encode));
	}
	pPutItem->AddValue("/Item/delay_report_id/N", m_nDelay_report_id);
	pPutItem->AddValue("/Item/seq/N", m_nSeq);
	if (!m_bDef_total_troop.empty())
	{
		pPutItem->AddValue("/Item/def_total_troop/B", Base64Encode((char*)&m_bDef_total_troop.m_astList[0], m_bDef_total_troop.m_udwNum*sizeof(SCommonTroop), sBase64Encode));
	}
	if (!m_bEx_reward.empty())
	{
		pPutItem->AddValue("/Item/ex_reward/B", Base64Encode((char*)&m_bEx_reward.m_astList[0], m_bEx_reward.m_udwNum*sizeof(SOneGlobalRes), sBase64Encode));
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

int TbMarch_action::OnPutItemReq(string& sPostData,
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

int TbMarch_action::OnPutItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

int TbMarch_action::OnResponse(const Json::Value& item)
{
	oJsonWriter.omitEndingLineFeed();
	int dwResLen = 0;
	dwResLen = 0;
	this->Reset();
	Json::Value::Members vecMembers = item.getMemberNames();
	for (unsigned int i = 0; i < vecMembers.size(); ++i)
	{
 		if (vecMembers[i] == "id")
		{
			m_nId = strtoll(item["id"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "suid")
		{
			m_nSuid = strtoll(item["suid"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "scid")
		{
			m_nScid = strtoll(item["scid"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "fixed")
		{
			m_nFixed = strtoll(item["fixed"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "mclass")
		{
			m_nMclass = strtoll(item["mclass"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "sclass")
		{
			m_nSclass = strtoll(item["sclass"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "status")
		{
			m_nStatus = strtoll(item["status"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "btime")
		{
			m_nBtime = strtoll(item["btime"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "ctime")
		{
			m_nCtime = strtoll(item["ctime"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "etime")
		{
			m_nEtime = strtoll(item["etime"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "param")
		{
			Base64Decode(item["param"]["B"].asString(), (char*)&m_bParam.m_astList[0], dwResLen);
			m_bParam.m_udwNum = dwResLen/sizeof(SActionMarchParam);
			if (0==m_bParam.m_udwNum && dwResLen>0)
			{
				m_bParam.m_udwNum = 1;
			}
			continue;
		}
		if (vecMembers[i] == "retry")
		{
			m_nRetry = strtoll(item["retry"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "sid")
		{
			m_nSid = strtoll(item["sid"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "notic_task_Flag")
		{
			Base64Decode(item["notic_task_Flag"]["B"].asString(), (char*)&m_bNotic_task_flag.m_astList[0], dwResLen);
			m_bNotic_task_flag.m_udwNum = dwResLen/sizeof(SNotictaskFlag);
			if (0==m_bNotic_task_flag.m_udwNum && dwResLen>0)
			{
				m_bNotic_task_flag.m_udwNum = 1;
			}
			continue;
		}
		if (vecMembers[i] == "sal")
		{
			m_nSal = strtoll(item["sal"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "tal")
		{
			m_nTal = strtoll(item["tal"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "tid")
		{
			m_nTid = strtoll(item["tid"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "sbid")
		{
			m_nSbid = strtoll(item["sbid"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "tbid")
		{
			m_nTbid = strtoll(item["tbid"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "kill_troop_might")
		{
			m_nKill_troop_might = strtoll(item["kill_troop_might"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "kill_fort_might")
		{
			m_nKill_fort_might = strtoll(item["kill_fort_might"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "buff")
		{
			Base64Decode(item["buff"]["B"].asString(), (char*)&m_bBuff.m_astList[0], dwResLen);
			m_bBuff.m_udwNum = dwResLen/sizeof(SBuffInfo);
			continue;
		}
		if (vecMembers[i] == "expiring_buff")
		{
			Base64Decode(item["expiring_buff"]["B"].asString(), (char*)&m_bExpiring_buff.m_astList[0], dwResLen);
			m_bExpiring_buff.m_udwNum = dwResLen/sizeof(SBuffInfo);
			continue;
		}
		if (vecMembers[i] == "reward")
		{
			Base64Decode(item["reward"]["B"].asString(), (char*)&m_bReward.m_astList[0], dwResLen);
			m_bReward.m_udwNum = dwResLen/sizeof(SGlobalRes);
			if (0==m_bReward.m_udwNum && dwResLen>0)
			{
				m_bReward.m_udwNum = 1;
			}
			continue;
		}
		if (vecMembers[i] == "sp_reward")
		{
			Base64Decode(item["sp_reward"]["B"].asString(), (char*)&m_bSp_reward.m_astList[0], dwResLen);
			m_bSp_reward.m_udwNum = dwResLen/sizeof(SGlobalRes);
			if (0==m_bSp_reward.m_udwNum && dwResLen>0)
			{
				m_bSp_reward.m_udwNum = 1;
			}
			continue;
		}
		if (vecMembers[i] == "tpos")
		{
			m_nTpos = strtoll(item["tpos"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "tuid")
		{
			m_nTuid = strtoll(item["tuid"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "if_max_attack")
		{
			m_nIf_max_attack = strtoll(item["if_max_attack"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "rally_atk_force")
		{
			Base64Decode(item["rally_atk_force"]["B"].asString(), (char*)&m_bRally_atk_force.m_astList[0], dwResLen);
			m_bRally_atk_force.m_udwNum = dwResLen/sizeof(SRallyForce);
			if (0==m_bRally_atk_force.m_udwNum && dwResLen>0)
			{
				m_bRally_atk_force.m_udwNum = 1;
			}
			continue;
		}
		if (vecMembers[i] == "rally_atk_slot")
		{
			Base64Decode(item["rally_atk_slot"]["B"].asString(), (char*)&m_bRally_atk_slot.m_astList[0], dwResLen);
			m_bRally_atk_slot.m_udwNum = dwResLen/sizeof(SRallySlot);
			continue;
		}
		if (vecMembers[i] == "rally_def_force")
		{
			Base64Decode(item["rally_def_force"]["B"].asString(), (char*)&m_bRally_def_force.m_astList[0], dwResLen);
			m_bRally_def_force.m_udwNum = dwResLen/sizeof(SRallyForce);
			if (0==m_bRally_def_force.m_udwNum && dwResLen>0)
			{
				m_bRally_def_force.m_udwNum = 1;
			}
			continue;
		}
		if (vecMembers[i] == "rally_def_slot")
		{
			Base64Decode(item["rally_def_slot"]["B"].asString(), (char*)&m_bRally_def_slot.m_astList[0], dwResLen);
			m_bRally_def_slot.m_udwNum = dwResLen/sizeof(SRallySlot);
			continue;
		}
		if (vecMembers[i] == "prison_param")
		{
			Base64Decode(item["prison_param"]["B"].asString(), (char*)&m_bPrison_param.m_astList[0], dwResLen);
			m_bPrison_param.m_udwNum = dwResLen/sizeof(SPrisonParam);
			if (0==m_bPrison_param.m_udwNum && dwResLen>0)
			{
				m_bPrison_param.m_udwNum = 1;
			}
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
		if (vecMembers[i] == "reinforce_result")
		{
			Base64Decode(item["reinforce_result"]["B"].asString(), (char*)&m_bReinforce_result.m_astList[0], dwResLen);
			m_bReinforce_result.m_udwNum = dwResLen/sizeof(SReinforceResult);
			if (0==m_bReinforce_result.m_udwNum && dwResLen>0)
			{
				m_bReinforce_result.m_udwNum = 1;
			}
			continue;
		}
		if (vecMembers[i] == "kill_troop_num")
		{
			m_nKill_troop_num = strtoll(item["kill_troop_num"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "kill_fort_num")
		{
			m_nKill_fort_num = strtoll(item["kill_fort_num"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "is_recalled")
		{
			m_nIs_recalled = strtoll(item["is_recalled"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "savatar")
		{
			m_nSavatar = strtoll(item["savatar"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "al_gift_record")
		{
			Base64Decode(item["al_gift_record"]["B"].asString(), (char*)&m_bAl_gift_record.m_astList[0], dwResLen);
			m_bAl_gift_record.m_udwNum = dwResLen/sizeof(SGlobalRes);
			if (0==m_bAl_gift_record.m_udwNum && dwResLen>0)
			{
				m_bAl_gift_record.m_udwNum = 1;
			}
			continue;
		}
		if (vecMembers[i] == "atk_total_troop")
		{
			Base64Decode(item["atk_total_troop"]["B"].asString(), (char*)&m_bAtk_total_troop.m_astList[0], dwResLen);
			m_bAtk_total_troop.m_udwNum = dwResLen/sizeof(SCommonTroop);
			if (0==m_bAtk_total_troop.m_udwNum && dwResLen>0)
			{
				m_bAtk_total_troop.m_udwNum = 1;
			}
			continue;
		}
		if (vecMembers[i] == "trade_info")
		{
			Base64Decode(item["trade_info"]["B"].asString(), (char*)&m_bTrade_info.m_astList[0], dwResLen);
			m_bTrade_info.m_udwNum = dwResLen/sizeof(STradeInfo);
			if (0==m_bTrade_info.m_udwNum && dwResLen>0)
			{
				m_bTrade_info.m_udwNum = 1;
			}
			continue;
		}
		if (vecMembers[i] == "tax_info")
		{
			if (FALSE == oJsonReader.parse(item["tax_info"]["S"].asString(),m_jTax_info))
			{
				m_jTax_info = Json::Value(Json::nullValue);
			}
			continue;
		}
		if (vecMembers[i] == "trade_list_idx")
		{
			m_nTrade_list_idx = strtoll(item["trade_list_idx"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "monster_info")
		{
			Base64Decode(item["monster_info"]["B"].asString(), (char*)&m_bMonster_info.m_astList[0], dwResLen);
			m_bMonster_info.m_udwNum = dwResLen/sizeof(SMonsterInfo);
			if (0==m_bMonster_info.m_udwNum && dwResLen>0)
			{
				m_bMonster_info.m_udwNum = 1;
			}
			continue;
		}
		if (vecMembers[i] == "delay_report_id")
		{
			m_nDelay_report_id = strtoll(item["delay_report_id"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "seq")
		{
			m_nSeq = strtoll(item["seq"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "def_total_troop")
		{
			Base64Decode(item["def_total_troop"]["B"].asString(), (char*)&m_bDef_total_troop.m_astList[0], dwResLen);
			m_bDef_total_troop.m_udwNum = dwResLen/sizeof(SCommonTroop);
			if (0==m_bDef_total_troop.m_udwNum && dwResLen>0)
			{
				m_bDef_total_troop.m_udwNum = 1;
			}
			continue;
		}
		if (vecMembers[i] == "ex_reward")
		{
			Base64Decode(item["ex_reward"]["B"].asString(), (char*)&m_bEx_reward.m_astList[0], dwResLen);
			m_bEx_reward.m_udwNum = dwResLen/sizeof(SOneGlobalRes);
			continue;
		}
	}
	return 0;
}

TINT64 TbMarch_action::GetSeq()
{
	return m_nSeq;
}

