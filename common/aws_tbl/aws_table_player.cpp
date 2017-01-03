#include "aws_table_player.h"

TableDesc TbPlayer::oTableDesc;

int TbPlayer::Init(const string& sConfFile, const string strProjectName)
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

AwsTable* TbPlayer::NewObject()
{
	return new TbPlayer;
}

string TbPlayer::GetTableName()
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

TINT32 TbPlayer::GetTableIdx()
{
	 return 0;
}

AwsMap* TbPlayer::OnScanReq(unsigned int udwIdxNo, bool bHasStartKey, bool bReturnConsumedCapacity,
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

int TbPlayer::OnScanReq(string& sPostData, unsigned int udwIdxNo, bool bHasStartKey, bool bReturnConsumedCapacity,
 unsigned int dwLimit, unsigned int dwSegment, unsigned int dwTotalSegments)
{
	AwsMap* pScan = OnScanReq(udwIdxNo, bHasStartKey, bReturnConsumedCapacity, dwLimit, dwSegment, dwTotalSegments);
	ostringstream oss;
	pScan->Dump(oss);
	sPostData = oss.str();
	delete pScan;
	return 0;
}

int TbPlayer::OnScanRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbPlayer::OnQueryReq(unsigned int udwIdxNo, const CompareDesc& comp_desc,
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
			if(fld_desc.udwFldNo == TbPLAYER_FIELD_UID)
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

int TbPlayer::OnQueryReq(string& sPostData, unsigned int udwIdxNo, const CompareDesc& comp_desc,
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

int TbPlayer::OnQueryRsp(const Json::Value& item)
{
	return OnResponse(item);
}

int TbPlayer::OnCountReq(string& sPostData, unsigned int udwIdxNo, const CompareDesc& comp_desc, 
		bool bConsistentRead, bool bReturnConsumedCapacity, bool bScanIndexForward, unsigned int dwLimit)
{
	AwsMap* pQuery = OnQueryReq(udwIdxNo, comp_desc, bConsistentRead, bReturnConsumedCapacity, bScanIndexForward, dwLimit, true);
	ostringstream oss;
	pQuery->Dump(oss);
	sPostData = oss.str();
	delete pQuery;
	return 0;
}

AwsMap* TbPlayer::OnUpdateItemReq(
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
		if (TbPLAYER_FIELD_SID == iter->first)
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
		if (TbPLAYER_FIELD_UIN == iter->first)
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
		if (TbPLAYER_FIELD_AVATAR == iter->first)
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
		if (TbPLAYER_FIELD_AGE == iter->first)
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
		if (TbPLAYER_FIELD_CID == iter->first)
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
		if (TbPLAYER_FIELD_CTIME == iter->first)
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
		if (TbPLAYER_FIELD_UTIME == iter->first)
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
		if (TbPLAYER_FIELD_NPC == iter->first)
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
		if (TbPLAYER_FIELD_STATUS == iter->first)
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
		if (TbPLAYER_FIELD_DEAD_FLAG == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/dead_flag/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/dead_flag/Value/N", m_nDead_flag);
				pUpdateItem->AddValue("/AttributeUpdates/dead_flag/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_ALID == iter->first)
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
		if (TbPLAYER_FIELD_ALNAME == iter->first)
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
		if (TbPLAYER_FIELD_ALPOS == iter->first)
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
		if (TbPLAYER_FIELD_AL_TIME == iter->first)
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
		if (TbPLAYER_FIELD_AL_NICK_NAME == iter->first)
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
		if (TbPLAYER_FIELD_REQ_AL == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/req_al/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/req_al/Value/N", m_nReq_al);
				pUpdateItem->AddValue("/AttributeUpdates/req_al/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_JOIN_ALLIANCE == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/join_alliance/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/join_alliance/Value/N", m_nJoin_alliance);
				pUpdateItem->AddValue("/AttributeUpdates/join_alliance/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_LOY_TIME == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/loy_time/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/loy_time/Value/N", m_nLoy_time);
				pUpdateItem->AddValue("/AttributeUpdates/loy_time/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_LOY_ITV == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/loy_itv/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/loy_itv/Value/N", m_nLoy_itv);
				pUpdateItem->AddValue("/AttributeUpdates/loy_itv/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_LOY_CUR == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/loy_cur/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/loy_cur/Value/N", m_nLoy_cur);
				pUpdateItem->AddValue("/AttributeUpdates/loy_cur/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_LOY_ALL == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/loy_all/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/loy_all/Value/N", m_nLoy_all);
				pUpdateItem->AddValue("/AttributeUpdates/loy_all/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_INVITE_MAIL_TIME == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/invite_mail_time/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/invite_mail_time/Value/N", m_nInvite_mail_time);
				pUpdateItem->AddValue("/AttributeUpdates/invite_mail_time/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_INVITED_NUM == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/invited_num/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/invited_num/Value/N", m_nInvited_num);
				pUpdateItem->AddValue("/AttributeUpdates/invited_num/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_INVITED_TIME == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/invited_time/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/invited_time/Value/N", m_nInvited_time);
				pUpdateItem->AddValue("/AttributeUpdates/invited_time/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_DOUBLOON == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/doubloon/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/doubloon/Value/N", m_nDoubloon);
				pUpdateItem->AddValue("/AttributeUpdates/doubloon/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_VIP_POINT == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/vip_point/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/vip_point/Value/N", m_nVip_point);
				pUpdateItem->AddValue("/AttributeUpdates/vip_point/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_VIP_ETIME == iter->first)
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
		if (TbPLAYER_FIELD_DRAGON_STATISTICS == iter->first)
		{
			if (!m_bDragon_statistics.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/dragon_statistics/Value/B", Base64Encode((char*)&m_bDragon_statistics.m_astList[0], m_bDragon_statistics.m_udwNum*sizeof(SDragonStatistics), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/dragon_statistics/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/dragon_statistics/Action", "DELETE");
			}
			continue;
		}
		if (TbPLAYER_FIELD_WAR_STATISTICS == iter->first)
		{
			if (!m_bWar_statistics.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/war_statistics/Value/B", Base64Encode((char*)&m_bWar_statistics.m_astList[0], m_bWar_statistics.m_udwNum*sizeof(SWarStatistics), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/war_statistics/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/war_statistics/Action", "DELETE");
			}
			continue;
		}
		if (TbPLAYER_FIELD_TRANSPORT_RESOURCE == iter->first)
		{
			if (!m_bTransport_resource.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/transport_resource/Value/B", Base64Encode((char*)&m_bTransport_resource.m_astList[0], m_bTransport_resource.m_udwNum*sizeof(SCommonResource), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/transport_resource/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/transport_resource/Action", "DELETE");
			}
			continue;
		}
		if (TbPLAYER_FIELD_GAIN_RESOURCE == iter->first)
		{
			if (!m_bGain_resource.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/gain_resource/Value/B", Base64Encode((char*)&m_bGain_resource.m_astList[0], m_bGain_resource.m_udwNum*sizeof(SCommonResource), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/gain_resource/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/gain_resource/Action", "DELETE");
			}
			continue;
		}
		if (TbPLAYER_FIELD_CUR_TROOP_MIGHT == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/cur_troop_might/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/cur_troop_might/Value/N", m_nCur_troop_might);
				pUpdateItem->AddValue("/AttributeUpdates/cur_troop_might/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_CUR_FORT_MIGHT == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/cur_fort_might/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/cur_fort_might/Value/N", m_nCur_fort_might);
				pUpdateItem->AddValue("/AttributeUpdates/cur_fort_might/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_SEND_AL_HELP_NUM == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/send_al_help_num/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/send_al_help_num/Value/N", m_nSend_al_help_num);
				pUpdateItem->AddValue("/AttributeUpdates/send_al_help_num/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_BUILDING_FORCE == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/building_force/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/building_force/Value/N", m_nBuilding_force);
				pUpdateItem->AddValue("/AttributeUpdates/building_force/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_RESEARCH_FORCE == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/research_force/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/research_force/Value/N", m_nResearch_force);
				pUpdateItem->AddValue("/AttributeUpdates/research_force/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_DRAGON_FORCE == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/dragon_force/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/dragon_force/Value/N", m_nDragon_force);
				pUpdateItem->AddValue("/AttributeUpdates/dragon_force/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_ALNAME_UPDATE_TIME == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/alname_update_time/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/alname_update_time/Value/N", m_nAlname_update_time);
				pUpdateItem->AddValue("/AttributeUpdates/alname_update_time/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_HAS_CHANGE_SVR == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/has_change_svr/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/has_change_svr/Value/N", m_nHas_change_svr);
				pUpdateItem->AddValue("/AttributeUpdates/has_change_svr/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_SVR_CHANGE_TIME == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/svr_change_time/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/svr_change_time/Value/N", m_nSvr_change_time);
				pUpdateItem->AddValue("/AttributeUpdates/svr_change_time/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_CHEST_LOTTERY == iter->first)
		{
			if (!m_bChest_lottery.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/chest_lottery/Value/B", Base64Encode((char*)&m_bChest_lottery.m_astList[0], m_bChest_lottery.m_udwNum*sizeof(SChestLottery), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/chest_lottery/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/chest_lottery/Action", "DELETE");
			}
			continue;
		}
		if (TbPLAYER_FIELD_DRAGON == iter->first)
		{
			if (!m_bDragon.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/dragon/Value/B", Base64Encode((char*)&m_bDragon.m_astList[0], m_bDragon.m_udwNum*sizeof(SDragonInfo), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/dragon/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/dragon/Action", "DELETE");
			}
			continue;
		}
		if (TbPLAYER_FIELD_MIGHT == iter->first)
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
		if (TbPLAYER_FIELD_DTROOP == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/dtroop/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/dtroop/Value/N", m_nDtroop);
				pUpdateItem->AddValue("/AttributeUpdates/dtroop/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_KTROOP == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/ktroop/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/ktroop/Value/N", m_nKtroop);
				pUpdateItem->AddValue("/AttributeUpdates/ktroop/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_DFORT == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/dfort/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/dfort/Value/N", m_nDfort);
				pUpdateItem->AddValue("/AttributeUpdates/dfort/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_KFORT == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/kfort/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/kfort/Value/N", m_nKfort);
				pUpdateItem->AddValue("/AttributeUpdates/kfort/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_BAT == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/bat/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/bat/Value/N", m_nBat);
				pUpdateItem->AddValue("/AttributeUpdates/bat/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_BAT_SUC == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/bat_suc/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/bat_suc/Value/N", m_nBat_suc);
				pUpdateItem->AddValue("/AttributeUpdates/bat_suc/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_DCITY == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/dcity/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/dcity/Value/N", m_nDcity);
				pUpdateItem->AddValue("/AttributeUpdates/dcity/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_MKILL == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/mkill/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/mkill/Value/N", m_nMkill);
				pUpdateItem->AddValue("/AttributeUpdates/mkill/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_MGAIN == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/mgain/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/mgain/Value/N", m_nMgain);
				pUpdateItem->AddValue("/AttributeUpdates/mgain/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_EXP == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/exp/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/exp/Value/N", m_nExp);
				pUpdateItem->AddValue("/AttributeUpdates/exp/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_LEVEL == iter->first)
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
		if (TbPLAYER_FIELD_LAST_MIGHT == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/last_might/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/last_might/Value/N", m_nLast_might);
				pUpdateItem->AddValue("/AttributeUpdates/last_might/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_LAST_TROOP_FORT == iter->first)
		{
			if (!m_jLast_troop_fort.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/last_troop_fort/Value/S", JsonEncode(oJsonWriter.write(m_jLast_troop_fort), sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/last_troop_fort/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/last_troop_fort/Action", "DELETE");
			}
			continue;
		}
		if (TbPLAYER_FIELD_SH_RES == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/sh_res/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/sh_res/Value/N", m_nSh_res);
				pUpdateItem->AddValue("/AttributeUpdates/sh_res/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_SH_MIGHT == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/sh_might/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/sh_might/Value/N", m_nSh_might);
				pUpdateItem->AddValue("/AttributeUpdates/sh_might/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_RES_COL == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/res_col/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/res_col/Value/N", m_nRes_col);
				pUpdateItem->AddValue("/AttributeUpdates/res_col/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_DRAGON_NAME == iter->first)
		{
			if (!m_sDragon_name.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/dragon_name/Value/S", JsonEncode(m_sDragon_name, sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/dragon_name/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/dragon_name/Action", "DELETE");
			}
			continue;
		}
		if (TbPLAYER_FIELD_DRAGON_LEVEL == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/dragon_level/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/dragon_level/Value/N", m_nDragon_level);
				pUpdateItem->AddValue("/AttributeUpdates/dragon_level/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_DRAGON_EXP == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/dragon_exp/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/dragon_exp/Value/N", m_nDragon_exp);
				pUpdateItem->AddValue("/AttributeUpdates/dragon_exp/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_DRAGON_AVATAR == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/dragon_avatar/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/dragon_avatar/Value/N", m_nDragon_avatar);
				pUpdateItem->AddValue("/AttributeUpdates/dragon_avatar/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_DRAGON_STATUS == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/dragon_status/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/dragon_status/Value/N", m_nDragon_status);
				pUpdateItem->AddValue("/AttributeUpdates/dragon_status/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_DRAGON_TID == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/dragon_tid/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/dragon_tid/Value/N", m_nDragon_tid);
				pUpdateItem->AddValue("/AttributeUpdates/dragon_tid/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_DRAGON_CUR_ENERGY == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/dragon_cur_energy/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/dragon_cur_energy/Value/N", m_nDragon_cur_energy);
				pUpdateItem->AddValue("/AttributeUpdates/dragon_cur_energy/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_DRAGON_RECOVERY_TIME == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/dragon_recovery_time/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/dragon_recovery_time/Value/N", m_nDragon_recovery_time);
				pUpdateItem->AddValue("/AttributeUpdates/dragon_recovery_time/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_DRAGON_RECOVERY_COUNT == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/dragon_recovery_count/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/dragon_recovery_count/Value/N", m_nDragon_recovery_count);
				pUpdateItem->AddValue("/AttributeUpdates/dragon_recovery_count/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_HAS_DRAGON == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/has_dragon/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/has_dragon/Value/N", m_nHas_dragon);
				pUpdateItem->AddValue("/AttributeUpdates/has_dragon/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_DRAGON_BEGIN_RECOVERY_TIME == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/dragon_begin_recovery_time/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/dragon_begin_recovery_time/Value/N", m_nDragon_begin_recovery_time);
				pUpdateItem->AddValue("/AttributeUpdates/dragon_begin_recovery_time/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_DRAGON_MAX_ENERGY == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/dragon_max_energy/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/dragon_max_energy/Value/N", m_nDragon_max_energy);
				pUpdateItem->AddValue("/AttributeUpdates/dragon_max_energy/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_PERSON_GUIDE == iter->first)
		{
			if (!m_bPerson_guide.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/person_guide/Value/B", Base64Encode((char*)&m_bPerson_guide.m_astList[0], m_bPerson_guide.m_udwNum*sizeof(SPersonGuideFinish), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/person_guide/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/person_guide/Action", "DELETE");
			}
			continue;
		}
		if (TbPLAYER_FIELD_DRAGON_MAX_LV == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/dragon_max_lv/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/dragon_max_lv/Value/N", m_nDragon_max_lv);
				pUpdateItem->AddValue("/AttributeUpdates/dragon_max_lv/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_MONSTER_HIT == iter->first)
		{
			if (!m_bMonster_hit.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/monster_hit/Value/B", Base64Encode((char*)&m_bMonster_hit.m_astList[0], m_bMonster_hit.m_udwNum*sizeof(SMonsterHit), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/monster_hit/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/monster_hit/Action", "DELETE");
			}
			continue;
		}
		if (TbPLAYER_FIELD_LEADER_MONSTER_KILL == iter->first)
		{
			if (!m_bLeader_monster_kill.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/leader_monster_kill/Value/B", Base64Encode((char*)&m_bLeader_monster_kill.m_astList[0], m_bLeader_monster_kill.m_udwNum*sizeof(SLeaderMonsterKill), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/leader_monster_kill/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/leader_monster_kill/Action", "DELETE");
			}
			continue;
		}
		if (TbPLAYER_FIELD_LEADER_MONSTER_GEN == iter->first)
		{
			if (!m_bLeader_monster_gen.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/leader_monster_gen/Value/B", Base64Encode((char*)&m_bLeader_monster_gen.m_astList[0], m_bLeader_monster_gen.m_udwNum*sizeof(SLeaderMonsterGen), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/leader_monster_gen/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/leader_monster_gen/Action", "DELETE");
			}
			continue;
		}
		if (TbPLAYER_FIELD_DRAGON_EQUIP == iter->first)
		{
			if (!m_jDragon_equip.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/dragon_equip/Value/S", JsonEncode(oJsonWriter.write(m_jDragon_equip), sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/dragon_equip/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/dragon_equip/Action", "DELETE");
			}
			continue;
		}
		if (TbPLAYER_FIELD_SEQ == iter->first)
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
		if (TbPLAYER_FIELD_LORD_FORCE == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/lord_force/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/lord_force/Value/N", m_nLord_force);
				pUpdateItem->AddValue("/AttributeUpdates/lord_force/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_TRIAL_INIT == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/trial_init/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/trial_init/Value/N", m_nTrial_init);
				pUpdateItem->AddValue("/AttributeUpdates/trial_init/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_DRAGON_SHARD == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/dragon_shard/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/dragon_shard/Value/N", m_nDragon_shard);
				pUpdateItem->AddValue("/AttributeUpdates/dragon_shard/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_TRIAL_RAGE_OPEN == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/trial_rage_open/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/trial_rage_open/Value/N", m_nTrial_rage_open);
				pUpdateItem->AddValue("/AttributeUpdates/trial_rage_open/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_TRIAL_RAGE_MODE == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/trial_rage_mode/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/trial_rage_mode/Value/N", m_nTrial_rage_mode);
				pUpdateItem->AddValue("/AttributeUpdates/trial_rage_mode/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_TRIAL_MONSTER_NORMAL == iter->first)
		{
			if (!m_bTrial_monster_normal.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/trial_monster_normal/Value/B", Base64Encode((char*)&m_bTrial_monster_normal.m_astList[0], m_bTrial_monster_normal.m_udwNum*sizeof(STrialMonster), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/trial_monster_normal/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/trial_monster_normal/Action", "DELETE");
			}
			continue;
		}
		if (TbPLAYER_FIELD_TRIAL_MONSTER_RAGE == iter->first)
		{
			if (!m_bTrial_monster_rage.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/trial_monster_rage/Value/B", Base64Encode((char*)&m_bTrial_monster_rage.m_astList[0], m_bTrial_monster_rage.m_udwNum*sizeof(STrialMonster), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/trial_monster_rage/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/trial_monster_rage/Action", "DELETE");
			}
			continue;
		}
		if (TbPLAYER_FIELD_TRIAL_LUCKY_BAG_NORMAL == iter->first)
		{
			if (!m_bTrial_lucky_bag_normal.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/trial_lucky_bag_normal/Value/B", Base64Encode((char*)&m_bTrial_lucky_bag_normal.m_astList[0], m_bTrial_lucky_bag_normal.m_udwNum*sizeof(STrialLuckyBagNormal), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/trial_lucky_bag_normal/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/trial_lucky_bag_normal/Action", "DELETE");
			}
			continue;
		}
		if (TbPLAYER_FIELD_TRIAL_LUCKY_BAG_RAGE == iter->first)
		{
			if (!m_bTrial_lucky_bag_rage.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/trial_lucky_bag_rage/Value/B", Base64Encode((char*)&m_bTrial_lucky_bag_rage.m_astList[0], m_bTrial_lucky_bag_rage.m_udwNum*sizeof(STrialLuckyBagRage), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/trial_lucky_bag_rage/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/trial_lucky_bag_rage/Action", "DELETE");
			}
			continue;
		}
		if (TbPLAYER_FIELD_TRIAL_GIFT_LAST_ETIME == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/trial_gift_last_etime/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/trial_gift_last_etime/Value/N", m_nTrial_gift_last_etime);
				pUpdateItem->AddValue("/AttributeUpdates/trial_gift_last_etime/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_FINISH_GUIDE_LIST == iter->first)
		{
			if (!m_bFinish_guide_list.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/finish_guide_list/Value/B", Base64Encode((char*)&m_bFinish_guide_list.m_astList[0], m_bFinish_guide_list.m_udwNum*sizeof(SFinishGuideList), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/finish_guide_list/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/finish_guide_list/Action", "DELETE");
			}
			continue;
		}
		if (TbPLAYER_FIELD_REINFORCE_LIMIT == iter->first)
		{
			if (!m_bReinforce_limit.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/reinforce_limit/Value/B", Base64Encode((char*)&m_bReinforce_limit.m_astList[0], m_bReinforce_limit.m_udwNum*sizeof(SReinforceLimit), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/reinforce_limit/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/reinforce_limit/Action", "DELETE");
			}
			continue;
		}
		if (TbPLAYER_FIELD_EVIL_FORCE_KILL == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/evil_force_kill/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/evil_force_kill/Value/N", m_nEvil_force_kill);
				pUpdateItem->AddValue("/AttributeUpdates/evil_force_kill/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_EVIL_TROOP_KILL == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/evil_troop_kill/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/evil_troop_kill/Value/N", m_nEvil_troop_kill);
				pUpdateItem->AddValue("/AttributeUpdates/evil_troop_kill/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbPLAYER_FIELD_VIP_STAGE == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/vip_stage/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/vip_stage/Value/N", m_nVip_stage);
				pUpdateItem->AddValue("/AttributeUpdates/vip_stage/Action", UpdateActionType2Str(iter->second));
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

int TbPlayer::OnUpdateItemReq(string& sPostData,
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

int TbPlayer::OnUpdateItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbPlayer::OnWriteItemReq(int dwActionType)
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
		if (!m_sUin.empty())
		{
			pItem->AddValue("/uin/S", JsonEncode(m_sUin, sJsonEncode));
		}
		pItem->AddValue("/avatar/N", m_nAvatar);
		pItem->AddValue("/age/N", m_nAge);
		pItem->AddValue("/cid/N", m_nCid);
		pItem->AddValue("/ctime/N", m_nCtime);
		pItem->AddValue("/utime/N", m_nUtime);
		pItem->AddValue("/npc/N", m_nNpc);
		pItem->AddValue("/status/N", m_nStatus);
		pItem->AddValue("/dead_flag/N", m_nDead_flag);
		pItem->AddValue("/alid/N", m_nAlid);
		if (!m_sAlname.empty())
		{
			pItem->AddValue("/alname/S", JsonEncode(m_sAlname, sJsonEncode));
		}
		pItem->AddValue("/alpos/N", m_nAlpos);
		pItem->AddValue("/al_time/N", m_nAl_time);
		if (!m_sAl_nick_name.empty())
		{
			pItem->AddValue("/al_nick_name/S", JsonEncode(m_sAl_nick_name, sJsonEncode));
		}
		pItem->AddValue("/req_al/N", m_nReq_al);
		pItem->AddValue("/join_alliance/N", m_nJoin_alliance);
		pItem->AddValue("/loy_time/N", m_nLoy_time);
		pItem->AddValue("/loy_itv/N", m_nLoy_itv);
		pItem->AddValue("/loy_cur/N", m_nLoy_cur);
		pItem->AddValue("/loy_all/N", m_nLoy_all);
		pItem->AddValue("/invite_mail_time/N", m_nInvite_mail_time);
		pItem->AddValue("/invited_num/N", m_nInvited_num);
		pItem->AddValue("/invited_time/N", m_nInvited_time);
		pItem->AddValue("/doubloon/N", m_nDoubloon);
		pItem->AddValue("/vip_point/N", m_nVip_point);
		pItem->AddValue("/vip_etime/N", m_nVip_etime);
		if (!m_bDragon_statistics.empty())
		{
			pItem->AddValue("/dragon_statistics/B", Base64Encode((char*)&m_bDragon_statistics.m_astList[0], m_bDragon_statistics.m_udwNum*sizeof(SDragonStatistics), sBase64Encode));
		}
		if (!m_bWar_statistics.empty())
		{
			pItem->AddValue("/war_statistics/B", Base64Encode((char*)&m_bWar_statistics.m_astList[0], m_bWar_statistics.m_udwNum*sizeof(SWarStatistics), sBase64Encode));
		}
		if (!m_bTransport_resource.empty())
		{
			pItem->AddValue("/transport_resource/B", Base64Encode((char*)&m_bTransport_resource.m_astList[0], m_bTransport_resource.m_udwNum*sizeof(SCommonResource), sBase64Encode));
		}
		if (!m_bGain_resource.empty())
		{
			pItem->AddValue("/gain_resource/B", Base64Encode((char*)&m_bGain_resource.m_astList[0], m_bGain_resource.m_udwNum*sizeof(SCommonResource), sBase64Encode));
		}
		pItem->AddValue("/cur_troop_might/N", m_nCur_troop_might);
		pItem->AddValue("/cur_fort_might/N", m_nCur_fort_might);
		pItem->AddValue("/send_al_help_num/N", m_nSend_al_help_num);
		pItem->AddValue("/building_force/N", m_nBuilding_force);
		pItem->AddValue("/research_force/N", m_nResearch_force);
		pItem->AddValue("/dragon_force/N", m_nDragon_force);
		pItem->AddValue("/alname_update_time/N", m_nAlname_update_time);
		pItem->AddValue("/has_change_svr/N", m_nHas_change_svr);
		pItem->AddValue("/svr_change_time/N", m_nSvr_change_time);
		if (!m_bChest_lottery.empty())
		{
			pItem->AddValue("/chest_lottery/B", Base64Encode((char*)&m_bChest_lottery.m_astList[0], m_bChest_lottery.m_udwNum*sizeof(SChestLottery), sBase64Encode));
		}
		if (!m_bDragon.empty())
		{
			pItem->AddValue("/dragon/B", Base64Encode((char*)&m_bDragon.m_astList[0], m_bDragon.m_udwNum*sizeof(SDragonInfo), sBase64Encode));
		}
		pItem->AddValue("/might/N", m_nMight);
		pItem->AddValue("/dtroop/N", m_nDtroop);
		pItem->AddValue("/ktroop/N", m_nKtroop);
		pItem->AddValue("/dfort/N", m_nDfort);
		pItem->AddValue("/kfort/N", m_nKfort);
		pItem->AddValue("/bat/N", m_nBat);
		pItem->AddValue("/bat_suc/N", m_nBat_suc);
		pItem->AddValue("/dcity/N", m_nDcity);
		pItem->AddValue("/mkill/N", m_nMkill);
		pItem->AddValue("/mgain/N", m_nMgain);
		pItem->AddValue("/exp/N", m_nExp);
		pItem->AddValue("/level/N", m_nLevel);
		pItem->AddValue("/last_might/N", m_nLast_might);
		if (!m_jLast_troop_fort.empty())
		{
				pItem->AddValue("/AttributeUpdates/last_troop_fort/Value/S", JsonEncode(oJsonWriter.write(m_jLast_troop_fort), sJsonEncode));
		}
		pItem->AddValue("/sh_res/N", m_nSh_res);
		pItem->AddValue("/sh_might/N", m_nSh_might);
		pItem->AddValue("/res_col/N", m_nRes_col);
		if (!m_sDragon_name.empty())
		{
			pItem->AddValue("/dragon_name/S", JsonEncode(m_sDragon_name, sJsonEncode));
		}
		pItem->AddValue("/dragon_level/N", m_nDragon_level);
		pItem->AddValue("/dragon_exp/N", m_nDragon_exp);
		pItem->AddValue("/dragon_avatar/N", m_nDragon_avatar);
		pItem->AddValue("/dragon_status/N", m_nDragon_status);
		pItem->AddValue("/dragon_tid/N", m_nDragon_tid);
		pItem->AddValue("/dragon_cur_energy/N", m_nDragon_cur_energy);
		pItem->AddValue("/dragon_recovery_time/N", m_nDragon_recovery_time);
		pItem->AddValue("/dragon_recovery_count/N", m_nDragon_recovery_count);
		pItem->AddValue("/has_dragon/N", m_nHas_dragon);
		pItem->AddValue("/dragon_begin_recovery_time/N", m_nDragon_begin_recovery_time);
		pItem->AddValue("/dragon_max_energy/N", m_nDragon_max_energy);
		if (!m_bPerson_guide.empty())
		{
			pItem->AddValue("/person_guide/B", Base64Encode((char*)&m_bPerson_guide.m_astList[0], m_bPerson_guide.m_udwNum*sizeof(SPersonGuideFinish), sBase64Encode));
		}
		pItem->AddValue("/dragon_max_lv/N", m_nDragon_max_lv);
		if (!m_bMonster_hit.empty())
		{
			pItem->AddValue("/monster_hit/B", Base64Encode((char*)&m_bMonster_hit.m_astList[0], m_bMonster_hit.m_udwNum*sizeof(SMonsterHit), sBase64Encode));
		}
		if (!m_bLeader_monster_kill.empty())
		{
			pItem->AddValue("/leader_monster_kill/B", Base64Encode((char*)&m_bLeader_monster_kill.m_astList[0], m_bLeader_monster_kill.m_udwNum*sizeof(SLeaderMonsterKill), sBase64Encode));
		}
		if (!m_bLeader_monster_gen.empty())
		{
			pItem->AddValue("/leader_monster_gen/B", Base64Encode((char*)&m_bLeader_monster_gen.m_astList[0], m_bLeader_monster_gen.m_udwNum*sizeof(SLeaderMonsterGen), sBase64Encode));
		}
		if (!m_jDragon_equip.empty())
		{
				pItem->AddValue("/AttributeUpdates/dragon_equip/Value/S", JsonEncode(oJsonWriter.write(m_jDragon_equip), sJsonEncode));
		}
		pItem->AddValue("/seq/N", m_nSeq);
		pItem->AddValue("/lord_force/N", m_nLord_force);
		pItem->AddValue("/trial_init/N", m_nTrial_init);
		pItem->AddValue("/dragon_shard/N", m_nDragon_shard);
		pItem->AddValue("/trial_rage_open/N", m_nTrial_rage_open);
		pItem->AddValue("/trial_rage_mode/N", m_nTrial_rage_mode);
		if (!m_bTrial_monster_normal.empty())
		{
			pItem->AddValue("/trial_monster_normal/B", Base64Encode((char*)&m_bTrial_monster_normal.m_astList[0], m_bTrial_monster_normal.m_udwNum*sizeof(STrialMonster), sBase64Encode));
		}
		if (!m_bTrial_monster_rage.empty())
		{
			pItem->AddValue("/trial_monster_rage/B", Base64Encode((char*)&m_bTrial_monster_rage.m_astList[0], m_bTrial_monster_rage.m_udwNum*sizeof(STrialMonster), sBase64Encode));
		}
		if (!m_bTrial_lucky_bag_normal.empty())
		{
			pItem->AddValue("/trial_lucky_bag_normal/B", Base64Encode((char*)&m_bTrial_lucky_bag_normal.m_astList[0], m_bTrial_lucky_bag_normal.m_udwNum*sizeof(STrialLuckyBagNormal), sBase64Encode));
		}
		if (!m_bTrial_lucky_bag_rage.empty())
		{
			pItem->AddValue("/trial_lucky_bag_rage/B", Base64Encode((char*)&m_bTrial_lucky_bag_rage.m_astList[0], m_bTrial_lucky_bag_rage.m_udwNum*sizeof(STrialLuckyBagRage), sBase64Encode));
		}
		pItem->AddValue("/trial_gift_last_etime/N", m_nTrial_gift_last_etime);
		if (!m_bFinish_guide_list.empty())
		{
			pItem->AddValue("/finish_guide_list/B", Base64Encode((char*)&m_bFinish_guide_list.m_astList[0], m_bFinish_guide_list.m_udwNum*sizeof(SFinishGuideList), sBase64Encode));
		}
		if (!m_bReinforce_limit.empty())
		{
			pItem->AddValue("/reinforce_limit/B", Base64Encode((char*)&m_bReinforce_limit.m_astList[0], m_bReinforce_limit.m_udwNum*sizeof(SReinforceLimit), sBase64Encode));
		}
		pItem->AddValue("/evil_force_kill/N", m_nEvil_force_kill);
		pItem->AddValue("/evil_troop_kill/N", m_nEvil_troop_kill);
		pItem->AddValue("/vip_stage/N", m_nVip_stage);
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

void TbPlayer::OnWriteItemReq(AwsMap* pWriteItem, int dwActionType)
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
		if (!m_sUin.empty())
		{
			pItem->AddValue("/uin/S", JsonEncode(m_sUin, sJsonEncode));
		}
		pItem->AddValue("/avatar/N", m_nAvatar);
		pItem->AddValue("/age/N", m_nAge);
		pItem->AddValue("/cid/N", m_nCid);
		pItem->AddValue("/ctime/N", m_nCtime);
		pItem->AddValue("/utime/N", m_nUtime);
		pItem->AddValue("/npc/N", m_nNpc);
		pItem->AddValue("/status/N", m_nStatus);
		pItem->AddValue("/dead_flag/N", m_nDead_flag);
		pItem->AddValue("/alid/N", m_nAlid);
		if (!m_sAlname.empty())
		{
			pItem->AddValue("/alname/S", JsonEncode(m_sAlname, sJsonEncode));
		}
		pItem->AddValue("/alpos/N", m_nAlpos);
		pItem->AddValue("/al_time/N", m_nAl_time);
		if (!m_sAl_nick_name.empty())
		{
			pItem->AddValue("/al_nick_name/S", JsonEncode(m_sAl_nick_name, sJsonEncode));
		}
		pItem->AddValue("/req_al/N", m_nReq_al);
		pItem->AddValue("/join_alliance/N", m_nJoin_alliance);
		pItem->AddValue("/loy_time/N", m_nLoy_time);
		pItem->AddValue("/loy_itv/N", m_nLoy_itv);
		pItem->AddValue("/loy_cur/N", m_nLoy_cur);
		pItem->AddValue("/loy_all/N", m_nLoy_all);
		pItem->AddValue("/invite_mail_time/N", m_nInvite_mail_time);
		pItem->AddValue("/invited_num/N", m_nInvited_num);
		pItem->AddValue("/invited_time/N", m_nInvited_time);
		pItem->AddValue("/doubloon/N", m_nDoubloon);
		pItem->AddValue("/vip_point/N", m_nVip_point);
		pItem->AddValue("/vip_etime/N", m_nVip_etime);
		if (!m_bDragon_statistics.empty())
		{
			pItem->AddValue("/dragon_statistics/B", Base64Encode((char*)&m_bDragon_statistics.m_astList[0], m_bDragon_statistics.m_udwNum*sizeof(SDragonStatistics), sBase64Encode));
		}
		if (!m_bWar_statistics.empty())
		{
			pItem->AddValue("/war_statistics/B", Base64Encode((char*)&m_bWar_statistics.m_astList[0], m_bWar_statistics.m_udwNum*sizeof(SWarStatistics), sBase64Encode));
		}
		if (!m_bTransport_resource.empty())
		{
			pItem->AddValue("/transport_resource/B", Base64Encode((char*)&m_bTransport_resource.m_astList[0], m_bTransport_resource.m_udwNum*sizeof(SCommonResource), sBase64Encode));
		}
		if (!m_bGain_resource.empty())
		{
			pItem->AddValue("/gain_resource/B", Base64Encode((char*)&m_bGain_resource.m_astList[0], m_bGain_resource.m_udwNum*sizeof(SCommonResource), sBase64Encode));
		}
		pItem->AddValue("/cur_troop_might/N", m_nCur_troop_might);
		pItem->AddValue("/cur_fort_might/N", m_nCur_fort_might);
		pItem->AddValue("/send_al_help_num/N", m_nSend_al_help_num);
		pItem->AddValue("/building_force/N", m_nBuilding_force);
		pItem->AddValue("/research_force/N", m_nResearch_force);
		pItem->AddValue("/dragon_force/N", m_nDragon_force);
		pItem->AddValue("/alname_update_time/N", m_nAlname_update_time);
		pItem->AddValue("/has_change_svr/N", m_nHas_change_svr);
		pItem->AddValue("/svr_change_time/N", m_nSvr_change_time);
		if (!m_bChest_lottery.empty())
		{
			pItem->AddValue("/chest_lottery/B", Base64Encode((char*)&m_bChest_lottery.m_astList[0], m_bChest_lottery.m_udwNum*sizeof(SChestLottery), sBase64Encode));
		}
		if (!m_bDragon.empty())
		{
			pItem->AddValue("/dragon/B", Base64Encode((char*)&m_bDragon.m_astList[0], m_bDragon.m_udwNum*sizeof(SDragonInfo), sBase64Encode));
		}
		pItem->AddValue("/might/N", m_nMight);
		pItem->AddValue("/dtroop/N", m_nDtroop);
		pItem->AddValue("/ktroop/N", m_nKtroop);
		pItem->AddValue("/dfort/N", m_nDfort);
		pItem->AddValue("/kfort/N", m_nKfort);
		pItem->AddValue("/bat/N", m_nBat);
		pItem->AddValue("/bat_suc/N", m_nBat_suc);
		pItem->AddValue("/dcity/N", m_nDcity);
		pItem->AddValue("/mkill/N", m_nMkill);
		pItem->AddValue("/mgain/N", m_nMgain);
		pItem->AddValue("/exp/N", m_nExp);
		pItem->AddValue("/level/N", m_nLevel);
		pItem->AddValue("/last_might/N", m_nLast_might);
		if (!m_jLast_troop_fort.empty())
		{
				pItem->AddValue("/AttributeUpdates/last_troop_fort/Value/S", JsonEncode(oJsonWriter.write(m_jLast_troop_fort), sJsonEncode));
		}
		pItem->AddValue("/sh_res/N", m_nSh_res);
		pItem->AddValue("/sh_might/N", m_nSh_might);
		pItem->AddValue("/res_col/N", m_nRes_col);
		if (!m_sDragon_name.empty())
		{
			pItem->AddValue("/dragon_name/S", JsonEncode(m_sDragon_name, sJsonEncode));
		}
		pItem->AddValue("/dragon_level/N", m_nDragon_level);
		pItem->AddValue("/dragon_exp/N", m_nDragon_exp);
		pItem->AddValue("/dragon_avatar/N", m_nDragon_avatar);
		pItem->AddValue("/dragon_status/N", m_nDragon_status);
		pItem->AddValue("/dragon_tid/N", m_nDragon_tid);
		pItem->AddValue("/dragon_cur_energy/N", m_nDragon_cur_energy);
		pItem->AddValue("/dragon_recovery_time/N", m_nDragon_recovery_time);
		pItem->AddValue("/dragon_recovery_count/N", m_nDragon_recovery_count);
		pItem->AddValue("/has_dragon/N", m_nHas_dragon);
		pItem->AddValue("/dragon_begin_recovery_time/N", m_nDragon_begin_recovery_time);
		pItem->AddValue("/dragon_max_energy/N", m_nDragon_max_energy);
		if (!m_bPerson_guide.empty())
		{
			pItem->AddValue("/person_guide/B", Base64Encode((char*)&m_bPerson_guide.m_astList[0], m_bPerson_guide.m_udwNum*sizeof(SPersonGuideFinish), sBase64Encode));
		}
		pItem->AddValue("/dragon_max_lv/N", m_nDragon_max_lv);
		if (!m_bMonster_hit.empty())
		{
			pItem->AddValue("/monster_hit/B", Base64Encode((char*)&m_bMonster_hit.m_astList[0], m_bMonster_hit.m_udwNum*sizeof(SMonsterHit), sBase64Encode));
		}
		if (!m_bLeader_monster_kill.empty())
		{
			pItem->AddValue("/leader_monster_kill/B", Base64Encode((char*)&m_bLeader_monster_kill.m_astList[0], m_bLeader_monster_kill.m_udwNum*sizeof(SLeaderMonsterKill), sBase64Encode));
		}
		if (!m_bLeader_monster_gen.empty())
		{
			pItem->AddValue("/leader_monster_gen/B", Base64Encode((char*)&m_bLeader_monster_gen.m_astList[0], m_bLeader_monster_gen.m_udwNum*sizeof(SLeaderMonsterGen), sBase64Encode));
		}
		if (!m_jDragon_equip.empty())
		{
				pItem->AddValue("/AttributeUpdates/dragon_equip/Value/S", JsonEncode(oJsonWriter.write(m_jDragon_equip), sJsonEncode));
		}
		pItem->AddValue("/seq/N", m_nSeq);
		pItem->AddValue("/lord_force/N", m_nLord_force);
		pItem->AddValue("/trial_init/N", m_nTrial_init);
		pItem->AddValue("/dragon_shard/N", m_nDragon_shard);
		pItem->AddValue("/trial_rage_open/N", m_nTrial_rage_open);
		pItem->AddValue("/trial_rage_mode/N", m_nTrial_rage_mode);
		if (!m_bTrial_monster_normal.empty())
		{
			pItem->AddValue("/trial_monster_normal/B", Base64Encode((char*)&m_bTrial_monster_normal.m_astList[0], m_bTrial_monster_normal.m_udwNum*sizeof(STrialMonster), sBase64Encode));
		}
		if (!m_bTrial_monster_rage.empty())
		{
			pItem->AddValue("/trial_monster_rage/B", Base64Encode((char*)&m_bTrial_monster_rage.m_astList[0], m_bTrial_monster_rage.m_udwNum*sizeof(STrialMonster), sBase64Encode));
		}
		if (!m_bTrial_lucky_bag_normal.empty())
		{
			pItem->AddValue("/trial_lucky_bag_normal/B", Base64Encode((char*)&m_bTrial_lucky_bag_normal.m_astList[0], m_bTrial_lucky_bag_normal.m_udwNum*sizeof(STrialLuckyBagNormal), sBase64Encode));
		}
		if (!m_bTrial_lucky_bag_rage.empty())
		{
			pItem->AddValue("/trial_lucky_bag_rage/B", Base64Encode((char*)&m_bTrial_lucky_bag_rage.m_astList[0], m_bTrial_lucky_bag_rage.m_udwNum*sizeof(STrialLuckyBagRage), sBase64Encode));
		}
		pItem->AddValue("/trial_gift_last_etime/N", m_nTrial_gift_last_etime);
		if (!m_bFinish_guide_list.empty())
		{
			pItem->AddValue("/finish_guide_list/B", Base64Encode((char*)&m_bFinish_guide_list.m_astList[0], m_bFinish_guide_list.m_udwNum*sizeof(SFinishGuideList), sBase64Encode));
		}
		if (!m_bReinforce_limit.empty())
		{
			pItem->AddValue("/reinforce_limit/B", Base64Encode((char*)&m_bReinforce_limit.m_astList[0], m_bReinforce_limit.m_udwNum*sizeof(SReinforceLimit), sBase64Encode));
		}
		pItem->AddValue("/evil_force_kill/N", m_nEvil_force_kill);
		pItem->AddValue("/evil_troop_kill/N", m_nEvil_troop_kill);
		pItem->AddValue("/vip_stage/N", m_nVip_stage);
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

AwsMap* TbPlayer::OnReadItemReq(unsigned int udwIdxNo)
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

void TbPlayer::OnReadItemReq(AwsMap* pReadItem)
{
	assert(pReadItem);
	AwsList* pKeys = pReadItem->GetAwsMap("RequestItems")->GetAwsMap(GetTableName())->GetAwsList("Keys");
	AwsMap* pKey = new AwsMap;
	assert(pKey);
	pKey->AddValue("/uid/N", m_nUid);
	pKeys->SetValue(pKey, true);
}

int TbPlayer::OnReadItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbPlayer::OnDeleteItemReq(
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

int TbPlayer::OnDeleteItemReq(string& sPostData,
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

int TbPlayer::OnDeleteItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbPlayer::OnGetItemReq(unsigned int udwIdxNo,
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

int TbPlayer::OnGetItemReq(string& sPostData, unsigned int udwIdxNo,
	bool bConsistentRead, bool bReturnConsumedCapacity)
{
	AwsMap* pGetItem = OnGetItemReq(udwIdxNo, bConsistentRead, bReturnConsumedCapacity);
	ostringstream oss;
	pGetItem->Dump(oss);
	sPostData = oss.str();
	delete pGetItem;
	return 0;
}

int TbPlayer::OnGetItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbPlayer::OnPutItemReq(
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
	if (!m_sUin.empty())
	{
		pPutItem->AddValue("/Item/uin/S", JsonEncode(m_sUin, sJsonEncode));
	}
	pPutItem->AddValue("/Item/avatar/N", m_nAvatar);
	pPutItem->AddValue("/Item/age/N", m_nAge);
	pPutItem->AddValue("/Item/cid/N", m_nCid);
	pPutItem->AddValue("/Item/ctime/N", m_nCtime);
	pPutItem->AddValue("/Item/utime/N", m_nUtime);
	pPutItem->AddValue("/Item/npc/N", m_nNpc);
	pPutItem->AddValue("/Item/status/N", m_nStatus);
	pPutItem->AddValue("/Item/dead_flag/N", m_nDead_flag);
	pPutItem->AddValue("/Item/alid/N", m_nAlid);
	if (!m_sAlname.empty())
	{
		pPutItem->AddValue("/Item/alname/S", JsonEncode(m_sAlname, sJsonEncode));
	}
	pPutItem->AddValue("/Item/alpos/N", m_nAlpos);
	pPutItem->AddValue("/Item/al_time/N", m_nAl_time);
	if (!m_sAl_nick_name.empty())
	{
		pPutItem->AddValue("/Item/al_nick_name/S", JsonEncode(m_sAl_nick_name, sJsonEncode));
	}
	pPutItem->AddValue("/Item/req_al/N", m_nReq_al);
	pPutItem->AddValue("/Item/join_alliance/N", m_nJoin_alliance);
	pPutItem->AddValue("/Item/loy_time/N", m_nLoy_time);
	pPutItem->AddValue("/Item/loy_itv/N", m_nLoy_itv);
	pPutItem->AddValue("/Item/loy_cur/N", m_nLoy_cur);
	pPutItem->AddValue("/Item/loy_all/N", m_nLoy_all);
	pPutItem->AddValue("/Item/invite_mail_time/N", m_nInvite_mail_time);
	pPutItem->AddValue("/Item/invited_num/N", m_nInvited_num);
	pPutItem->AddValue("/Item/invited_time/N", m_nInvited_time);
	pPutItem->AddValue("/Item/doubloon/N", m_nDoubloon);
	pPutItem->AddValue("/Item/vip_point/N", m_nVip_point);
	pPutItem->AddValue("/Item/vip_etime/N", m_nVip_etime);
	if (!m_bDragon_statistics.empty())
	{
		pPutItem->AddValue("/Item/dragon_statistics/B", Base64Encode((char*)&m_bDragon_statistics.m_astList[0], m_bDragon_statistics.m_udwNum*sizeof(SDragonStatistics), sBase64Encode));
	}
	if (!m_bWar_statistics.empty())
	{
		pPutItem->AddValue("/Item/war_statistics/B", Base64Encode((char*)&m_bWar_statistics.m_astList[0], m_bWar_statistics.m_udwNum*sizeof(SWarStatistics), sBase64Encode));
	}
	if (!m_bTransport_resource.empty())
	{
		pPutItem->AddValue("/Item/transport_resource/B", Base64Encode((char*)&m_bTransport_resource.m_astList[0], m_bTransport_resource.m_udwNum*sizeof(SCommonResource), sBase64Encode));
	}
	if (!m_bGain_resource.empty())
	{
		pPutItem->AddValue("/Item/gain_resource/B", Base64Encode((char*)&m_bGain_resource.m_astList[0], m_bGain_resource.m_udwNum*sizeof(SCommonResource), sBase64Encode));
	}
	pPutItem->AddValue("/Item/cur_troop_might/N", m_nCur_troop_might);
	pPutItem->AddValue("/Item/cur_fort_might/N", m_nCur_fort_might);
	pPutItem->AddValue("/Item/send_al_help_num/N", m_nSend_al_help_num);
	pPutItem->AddValue("/Item/building_force/N", m_nBuilding_force);
	pPutItem->AddValue("/Item/research_force/N", m_nResearch_force);
	pPutItem->AddValue("/Item/dragon_force/N", m_nDragon_force);
	pPutItem->AddValue("/Item/alname_update_time/N", m_nAlname_update_time);
	pPutItem->AddValue("/Item/has_change_svr/N", m_nHas_change_svr);
	pPutItem->AddValue("/Item/svr_change_time/N", m_nSvr_change_time);
	if (!m_bChest_lottery.empty())
	{
		pPutItem->AddValue("/Item/chest_lottery/B", Base64Encode((char*)&m_bChest_lottery.m_astList[0], m_bChest_lottery.m_udwNum*sizeof(SChestLottery), sBase64Encode));
	}
	if (!m_bDragon.empty())
	{
		pPutItem->AddValue("/Item/dragon/B", Base64Encode((char*)&m_bDragon.m_astList[0], m_bDragon.m_udwNum*sizeof(SDragonInfo), sBase64Encode));
	}
	pPutItem->AddValue("/Item/might/N", m_nMight);
	pPutItem->AddValue("/Item/dtroop/N", m_nDtroop);
	pPutItem->AddValue("/Item/ktroop/N", m_nKtroop);
	pPutItem->AddValue("/Item/dfort/N", m_nDfort);
	pPutItem->AddValue("/Item/kfort/N", m_nKfort);
	pPutItem->AddValue("/Item/bat/N", m_nBat);
	pPutItem->AddValue("/Item/bat_suc/N", m_nBat_suc);
	pPutItem->AddValue("/Item/dcity/N", m_nDcity);
	pPutItem->AddValue("/Item/mkill/N", m_nMkill);
	pPutItem->AddValue("/Item/mgain/N", m_nMgain);
	pPutItem->AddValue("/Item/exp/N", m_nExp);
	pPutItem->AddValue("/Item/level/N", m_nLevel);
	pPutItem->AddValue("/Item/last_might/N", m_nLast_might);
	if (!m_jLast_troop_fort.empty())
	{
				pPutItem->AddValue("/Item/last_troop_fort/S", JsonEncode(oJsonWriter.write(m_jLast_troop_fort), sJsonEncode));
	}
	pPutItem->AddValue("/Item/sh_res/N", m_nSh_res);
	pPutItem->AddValue("/Item/sh_might/N", m_nSh_might);
	pPutItem->AddValue("/Item/res_col/N", m_nRes_col);
	if (!m_sDragon_name.empty())
	{
		pPutItem->AddValue("/Item/dragon_name/S", JsonEncode(m_sDragon_name, sJsonEncode));
	}
	pPutItem->AddValue("/Item/dragon_level/N", m_nDragon_level);
	pPutItem->AddValue("/Item/dragon_exp/N", m_nDragon_exp);
	pPutItem->AddValue("/Item/dragon_avatar/N", m_nDragon_avatar);
	pPutItem->AddValue("/Item/dragon_status/N", m_nDragon_status);
	pPutItem->AddValue("/Item/dragon_tid/N", m_nDragon_tid);
	pPutItem->AddValue("/Item/dragon_cur_energy/N", m_nDragon_cur_energy);
	pPutItem->AddValue("/Item/dragon_recovery_time/N", m_nDragon_recovery_time);
	pPutItem->AddValue("/Item/dragon_recovery_count/N", m_nDragon_recovery_count);
	pPutItem->AddValue("/Item/has_dragon/N", m_nHas_dragon);
	pPutItem->AddValue("/Item/dragon_begin_recovery_time/N", m_nDragon_begin_recovery_time);
	pPutItem->AddValue("/Item/dragon_max_energy/N", m_nDragon_max_energy);
	if (!m_bPerson_guide.empty())
	{
		pPutItem->AddValue("/Item/person_guide/B", Base64Encode((char*)&m_bPerson_guide.m_astList[0], m_bPerson_guide.m_udwNum*sizeof(SPersonGuideFinish), sBase64Encode));
	}
	pPutItem->AddValue("/Item/dragon_max_lv/N", m_nDragon_max_lv);
	if (!m_bMonster_hit.empty())
	{
		pPutItem->AddValue("/Item/monster_hit/B", Base64Encode((char*)&m_bMonster_hit.m_astList[0], m_bMonster_hit.m_udwNum*sizeof(SMonsterHit), sBase64Encode));
	}
	if (!m_bLeader_monster_kill.empty())
	{
		pPutItem->AddValue("/Item/leader_monster_kill/B", Base64Encode((char*)&m_bLeader_monster_kill.m_astList[0], m_bLeader_monster_kill.m_udwNum*sizeof(SLeaderMonsterKill), sBase64Encode));
	}
	if (!m_bLeader_monster_gen.empty())
	{
		pPutItem->AddValue("/Item/leader_monster_gen/B", Base64Encode((char*)&m_bLeader_monster_gen.m_astList[0], m_bLeader_monster_gen.m_udwNum*sizeof(SLeaderMonsterGen), sBase64Encode));
	}
	if (!m_jDragon_equip.empty())
	{
				pPutItem->AddValue("/Item/dragon_equip/S", JsonEncode(oJsonWriter.write(m_jDragon_equip), sJsonEncode));
	}
	pPutItem->AddValue("/Item/seq/N", m_nSeq);
	pPutItem->AddValue("/Item/lord_force/N", m_nLord_force);
	pPutItem->AddValue("/Item/trial_init/N", m_nTrial_init);
	pPutItem->AddValue("/Item/dragon_shard/N", m_nDragon_shard);
	pPutItem->AddValue("/Item/trial_rage_open/N", m_nTrial_rage_open);
	pPutItem->AddValue("/Item/trial_rage_mode/N", m_nTrial_rage_mode);
	if (!m_bTrial_monster_normal.empty())
	{
		pPutItem->AddValue("/Item/trial_monster_normal/B", Base64Encode((char*)&m_bTrial_monster_normal.m_astList[0], m_bTrial_monster_normal.m_udwNum*sizeof(STrialMonster), sBase64Encode));
	}
	if (!m_bTrial_monster_rage.empty())
	{
		pPutItem->AddValue("/Item/trial_monster_rage/B", Base64Encode((char*)&m_bTrial_monster_rage.m_astList[0], m_bTrial_monster_rage.m_udwNum*sizeof(STrialMonster), sBase64Encode));
	}
	if (!m_bTrial_lucky_bag_normal.empty())
	{
		pPutItem->AddValue("/Item/trial_lucky_bag_normal/B", Base64Encode((char*)&m_bTrial_lucky_bag_normal.m_astList[0], m_bTrial_lucky_bag_normal.m_udwNum*sizeof(STrialLuckyBagNormal), sBase64Encode));
	}
	if (!m_bTrial_lucky_bag_rage.empty())
	{
		pPutItem->AddValue("/Item/trial_lucky_bag_rage/B", Base64Encode((char*)&m_bTrial_lucky_bag_rage.m_astList[0], m_bTrial_lucky_bag_rage.m_udwNum*sizeof(STrialLuckyBagRage), sBase64Encode));
	}
	pPutItem->AddValue("/Item/trial_gift_last_etime/N", m_nTrial_gift_last_etime);
	if (!m_bFinish_guide_list.empty())
	{
		pPutItem->AddValue("/Item/finish_guide_list/B", Base64Encode((char*)&m_bFinish_guide_list.m_astList[0], m_bFinish_guide_list.m_udwNum*sizeof(SFinishGuideList), sBase64Encode));
	}
	if (!m_bReinforce_limit.empty())
	{
		pPutItem->AddValue("/Item/reinforce_limit/B", Base64Encode((char*)&m_bReinforce_limit.m_astList[0], m_bReinforce_limit.m_udwNum*sizeof(SReinforceLimit), sBase64Encode));
	}
	pPutItem->AddValue("/Item/evil_force_kill/N", m_nEvil_force_kill);
	pPutItem->AddValue("/Item/evil_troop_kill/N", m_nEvil_troop_kill);
	pPutItem->AddValue("/Item/vip_stage/N", m_nVip_stage);
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

int TbPlayer::OnPutItemReq(string& sPostData,
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

int TbPlayer::OnPutItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

int TbPlayer::OnResponse(const Json::Value& item)
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
		if (vecMembers[i] == "uin")
		{
			m_sUin = item["uin"]["S"].asString();
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
		if (vecMembers[i] == "cid")
		{
			m_nCid = strtoll(item["cid"]["N"].asString().c_str(), NULL, 10);
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
		if (vecMembers[i] == "npc")
		{
			m_nNpc = strtoll(item["npc"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "status")
		{
			m_nStatus = strtoll(item["status"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "dead_flag")
		{
			m_nDead_flag = strtoll(item["dead_flag"]["N"].asString().c_str(), NULL, 10);
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
		if (vecMembers[i] == "alpos")
		{
			m_nAlpos = strtoll(item["alpos"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "al_time")
		{
			m_nAl_time = strtoll(item["al_time"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "al_nick_name")
		{
			m_sAl_nick_name = item["al_nick_name"]["S"].asString();
			continue;
		}
		if (vecMembers[i] == "req_al")
		{
			m_nReq_al = strtoll(item["req_al"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "join_alliance")
		{
			m_nJoin_alliance = strtoll(item["join_alliance"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "loy_time")
		{
			m_nLoy_time = strtoll(item["loy_time"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "loy_itv")
		{
			m_nLoy_itv = strtoll(item["loy_itv"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "loy_cur")
		{
			m_nLoy_cur = strtoll(item["loy_cur"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "loy_all")
		{
			m_nLoy_all = strtoll(item["loy_all"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "invite_mail_time")
		{
			m_nInvite_mail_time = strtoll(item["invite_mail_time"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "invited_num")
		{
			m_nInvited_num = strtoll(item["invited_num"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "invited_time")
		{
			m_nInvited_time = strtoll(item["invited_time"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "doubloon")
		{
			m_nDoubloon = strtoll(item["doubloon"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "vip_point")
		{
			m_nVip_point = strtoll(item["vip_point"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "vip_etime")
		{
			m_nVip_etime = strtoll(item["vip_etime"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "dragon_statistics")
		{
			Base64Decode(item["dragon_statistics"]["B"].asString(), (char*)&m_bDragon_statistics.m_astList[0], dwResLen);
			m_bDragon_statistics.m_udwNum = dwResLen/sizeof(SDragonStatistics);
			if (0==m_bDragon_statistics.m_udwNum && dwResLen>0)
			{
				m_bDragon_statistics.m_udwNum = 1;
			}
			continue;
		}
		if (vecMembers[i] == "war_statistics")
		{
			Base64Decode(item["war_statistics"]["B"].asString(), (char*)&m_bWar_statistics.m_astList[0], dwResLen);
			m_bWar_statistics.m_udwNum = dwResLen/sizeof(SWarStatistics);
			if (0==m_bWar_statistics.m_udwNum && dwResLen>0)
			{
				m_bWar_statistics.m_udwNum = 1;
			}
			continue;
		}
		if (vecMembers[i] == "transport_resource")
		{
			Base64Decode(item["transport_resource"]["B"].asString(), (char*)&m_bTransport_resource.m_astList[0], dwResLen);
			m_bTransport_resource.m_udwNum = dwResLen/sizeof(SCommonResource);
			if (0==m_bTransport_resource.m_udwNum && dwResLen>0)
			{
				m_bTransport_resource.m_udwNum = 1;
			}
			continue;
		}
		if (vecMembers[i] == "gain_resource")
		{
			Base64Decode(item["gain_resource"]["B"].asString(), (char*)&m_bGain_resource.m_astList[0], dwResLen);
			m_bGain_resource.m_udwNum = dwResLen/sizeof(SCommonResource);
			if (0==m_bGain_resource.m_udwNum && dwResLen>0)
			{
				m_bGain_resource.m_udwNum = 1;
			}
			continue;
		}
		if (vecMembers[i] == "cur_troop_might")
		{
			m_nCur_troop_might = strtoll(item["cur_troop_might"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "cur_fort_might")
		{
			m_nCur_fort_might = strtoll(item["cur_fort_might"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "send_al_help_num")
		{
			m_nSend_al_help_num = strtoll(item["send_al_help_num"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "building_force")
		{
			m_nBuilding_force = strtoll(item["building_force"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "research_force")
		{
			m_nResearch_force = strtoll(item["research_force"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "dragon_force")
		{
			m_nDragon_force = strtoll(item["dragon_force"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "alname_update_time")
		{
			m_nAlname_update_time = strtoll(item["alname_update_time"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "has_change_svr")
		{
			m_nHas_change_svr = strtoll(item["has_change_svr"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "svr_change_time")
		{
			m_nSvr_change_time = strtoll(item["svr_change_time"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "chest_lottery")
		{
			Base64Decode(item["chest_lottery"]["B"].asString(), (char*)&m_bChest_lottery.m_astList[0], dwResLen);
			m_bChest_lottery.m_udwNum = dwResLen/sizeof(SChestLottery);
			continue;
		}
		if (vecMembers[i] == "dragon")
		{
			Base64Decode(item["dragon"]["B"].asString(), (char*)&m_bDragon.m_astList[0], dwResLen);
			m_bDragon.m_udwNum = dwResLen/sizeof(SDragonInfo);
			if (0==m_bDragon.m_udwNum && dwResLen>0)
			{
				m_bDragon.m_udwNum = 1;
			}
			continue;
		}
		if (vecMembers[i] == "might")
		{
			m_nMight = strtoll(item["might"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "dtroop")
		{
			m_nDtroop = strtoll(item["dtroop"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "ktroop")
		{
			m_nKtroop = strtoll(item["ktroop"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "dfort")
		{
			m_nDfort = strtoll(item["dfort"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "kfort")
		{
			m_nKfort = strtoll(item["kfort"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "bat")
		{
			m_nBat = strtoll(item["bat"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "bat_suc")
		{
			m_nBat_suc = strtoll(item["bat_suc"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "dcity")
		{
			m_nDcity = strtoll(item["dcity"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "mkill")
		{
			m_nMkill = strtoll(item["mkill"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "mgain")
		{
			m_nMgain = strtoll(item["mgain"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "exp")
		{
			m_nExp = strtoll(item["exp"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "level")
		{
			m_nLevel = strtoll(item["level"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "last_might")
		{
			m_nLast_might = strtoll(item["last_might"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "last_troop_fort")
		{
			if (FALSE == oJsonReader.parse(item["last_troop_fort"]["S"].asString(),m_jLast_troop_fort))
			{
				m_jLast_troop_fort = Json::Value(Json::nullValue);
			}
			continue;
		}
		if (vecMembers[i] == "sh_res")
		{
			m_nSh_res = strtoll(item["sh_res"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "sh_might")
		{
			m_nSh_might = strtoll(item["sh_might"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "res_col")
		{
			m_nRes_col = strtoll(item["res_col"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "dragon_name")
		{
			m_sDragon_name = item["dragon_name"]["S"].asString();
			continue;
		}
		if (vecMembers[i] == "dragon_level")
		{
			m_nDragon_level = strtoll(item["dragon_level"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "dragon_exp")
		{
			m_nDragon_exp = strtoll(item["dragon_exp"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "dragon_avatar")
		{
			m_nDragon_avatar = strtoll(item["dragon_avatar"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "dragon_status")
		{
			m_nDragon_status = strtoll(item["dragon_status"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "dragon_tid")
		{
			m_nDragon_tid = strtoll(item["dragon_tid"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "dragon_cur_energy")
		{
			m_nDragon_cur_energy = strtoll(item["dragon_cur_energy"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "dragon_recovery_time")
		{
			m_nDragon_recovery_time = strtoll(item["dragon_recovery_time"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "dragon_recovery_count")
		{
			m_nDragon_recovery_count = strtoll(item["dragon_recovery_count"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "has_dragon")
		{
			m_nHas_dragon = strtoll(item["has_dragon"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "dragon_begin_recovery_time")
		{
			m_nDragon_begin_recovery_time = strtoll(item["dragon_begin_recovery_time"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "dragon_max_energy")
		{
			m_nDragon_max_energy = strtoll(item["dragon_max_energy"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "person_guide")
		{
			Base64Decode(item["person_guide"]["B"].asString(), (char*)&m_bPerson_guide.m_astList[0], dwResLen);
			m_bPerson_guide.m_udwNum = dwResLen/sizeof(SPersonGuideFinish);
			if (0==m_bPerson_guide.m_udwNum && dwResLen>0)
			{
				m_bPerson_guide.m_udwNum = 1;
			}
			continue;
		}
		if (vecMembers[i] == "dragon_max_lv")
		{
			m_nDragon_max_lv = strtoll(item["dragon_max_lv"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "monster_hit")
		{
			Base64Decode(item["monster_hit"]["B"].asString(), (char*)&m_bMonster_hit.m_astList[0], dwResLen);
			m_bMonster_hit.m_udwNum = dwResLen/sizeof(SMonsterHit);
			if (0==m_bMonster_hit.m_udwNum && dwResLen>0)
			{
				m_bMonster_hit.m_udwNum = 1;
			}
			continue;
		}
		if (vecMembers[i] == "leader_monster_kill")
		{
			Base64Decode(item["leader_monster_kill"]["B"].asString(), (char*)&m_bLeader_monster_kill.m_astList[0], dwResLen);
			m_bLeader_monster_kill.m_udwNum = dwResLen/sizeof(SLeaderMonsterKill);
			if (0==m_bLeader_monster_kill.m_udwNum && dwResLen>0)
			{
				m_bLeader_monster_kill.m_udwNum = 1;
			}
			continue;
		}
		if (vecMembers[i] == "leader_monster_gen")
		{
			Base64Decode(item["leader_monster_gen"]["B"].asString(), (char*)&m_bLeader_monster_gen.m_astList[0], dwResLen);
			m_bLeader_monster_gen.m_udwNum = dwResLen/sizeof(SLeaderMonsterGen);
			if (0==m_bLeader_monster_gen.m_udwNum && dwResLen>0)
			{
				m_bLeader_monster_gen.m_udwNum = 1;
			}
			continue;
		}
		if (vecMembers[i] == "dragon_equip")
		{
			if (FALSE == oJsonReader.parse(item["dragon_equip"]["S"].asString(),m_jDragon_equip))
			{
				m_jDragon_equip = Json::Value(Json::nullValue);
			}
			continue;
		}
		if (vecMembers[i] == "seq")
		{
			m_nSeq = strtoll(item["seq"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "lord_force")
		{
			m_nLord_force = strtoll(item["lord_force"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "trial_init")
		{
			m_nTrial_init = strtoll(item["trial_init"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "dragon_shard")
		{
			m_nDragon_shard = strtoll(item["dragon_shard"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "trial_rage_open")
		{
			m_nTrial_rage_open = strtoll(item["trial_rage_open"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "trial_rage_mode")
		{
			m_nTrial_rage_mode = strtoll(item["trial_rage_mode"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "trial_monster_normal")
		{
			Base64Decode(item["trial_monster_normal"]["B"].asString(), (char*)&m_bTrial_monster_normal.m_astList[0], dwResLen);
			m_bTrial_monster_normal.m_udwNum = dwResLen/sizeof(STrialMonster);
			if (0==m_bTrial_monster_normal.m_udwNum && dwResLen>0)
			{
				m_bTrial_monster_normal.m_udwNum = 1;
			}
			continue;
		}
		if (vecMembers[i] == "trial_monster_rage")
		{
			Base64Decode(item["trial_monster_rage"]["B"].asString(), (char*)&m_bTrial_monster_rage.m_astList[0], dwResLen);
			m_bTrial_monster_rage.m_udwNum = dwResLen/sizeof(STrialMonster);
			if (0==m_bTrial_monster_rage.m_udwNum && dwResLen>0)
			{
				m_bTrial_monster_rage.m_udwNum = 1;
			}
			continue;
		}
		if (vecMembers[i] == "trial_lucky_bag_normal")
		{
			Base64Decode(item["trial_lucky_bag_normal"]["B"].asString(), (char*)&m_bTrial_lucky_bag_normal.m_astList[0], dwResLen);
			m_bTrial_lucky_bag_normal.m_udwNum = dwResLen/sizeof(STrialLuckyBagNormal);
			if (0==m_bTrial_lucky_bag_normal.m_udwNum && dwResLen>0)
			{
				m_bTrial_lucky_bag_normal.m_udwNum = 1;
			}
			continue;
		}
		if (vecMembers[i] == "trial_lucky_bag_rage")
		{
			Base64Decode(item["trial_lucky_bag_rage"]["B"].asString(), (char*)&m_bTrial_lucky_bag_rage.m_astList[0], dwResLen);
			m_bTrial_lucky_bag_rage.m_udwNum = dwResLen/sizeof(STrialLuckyBagRage);
			if (0==m_bTrial_lucky_bag_rage.m_udwNum && dwResLen>0)
			{
				m_bTrial_lucky_bag_rage.m_udwNum = 1;
			}
			continue;
		}
		if (vecMembers[i] == "trial_gift_last_etime")
		{
			m_nTrial_gift_last_etime = strtoll(item["trial_gift_last_etime"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "finish_guide_list")
		{
			Base64Decode(item["finish_guide_list"]["B"].asString(), (char*)&m_bFinish_guide_list.m_astList[0], dwResLen);
			m_bFinish_guide_list.m_udwNum = dwResLen/sizeof(SFinishGuideList);
			if (0==m_bFinish_guide_list.m_udwNum && dwResLen>0)
			{
				m_bFinish_guide_list.m_udwNum = 1;
			}
			continue;
		}
		if (vecMembers[i] == "reinforce_limit")
		{
			Base64Decode(item["reinforce_limit"]["B"].asString(), (char*)&m_bReinforce_limit.m_astList[0], dwResLen);
			m_bReinforce_limit.m_udwNum = dwResLen/sizeof(SReinforceLimit);
			if (0==m_bReinforce_limit.m_udwNum && dwResLen>0)
			{
				m_bReinforce_limit.m_udwNum = 1;
			}
			continue;
		}
		if (vecMembers[i] == "evil_force_kill")
		{
			m_nEvil_force_kill = strtoll(item["evil_force_kill"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "evil_troop_kill")
		{
			m_nEvil_troop_kill = strtoll(item["evil_troop_kill"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "vip_stage")
		{
			m_nVip_stage = strtoll(item["vip_stage"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
	}
	return 0;
}

TINT64 TbPlayer::GetSeq()
{
	return m_nSeq;
}

