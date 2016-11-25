#include "aws_table_user_stat.h"

TableDesc TbUser_stat::oTableDesc;

int TbUser_stat::Init(const string& sConfFile, const string strProjectName)
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

AwsTable* TbUser_stat::NewObject()
{
	return new TbUser_stat;
}

string TbUser_stat::GetTableName()
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

TINT32 TbUser_stat::GetTableIdx()
{
	 return 0;
}

AwsMap* TbUser_stat::OnScanReq(unsigned int udwIdxNo, bool bHasStartKey, bool bReturnConsumedCapacity,
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

int TbUser_stat::OnScanReq(string& sPostData, unsigned int udwIdxNo, bool bHasStartKey, bool bReturnConsumedCapacity,
 unsigned int dwLimit, unsigned int dwSegment, unsigned int dwTotalSegments)
{
	AwsMap* pScan = OnScanReq(udwIdxNo, bHasStartKey, bReturnConsumedCapacity, dwLimit, dwSegment, dwTotalSegments);
	ostringstream oss;
	pScan->Dump(oss);
	sPostData = oss.str();
	delete pScan;
	return 0;
}

int TbUser_stat::OnScanRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbUser_stat::OnQueryReq(unsigned int udwIdxNo, const CompareDesc& comp_desc,
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
			if(fld_desc.udwFldNo == TbUSER_STAT_FIELD_UID)
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

int TbUser_stat::OnQueryReq(string& sPostData, unsigned int udwIdxNo, const CompareDesc& comp_desc,
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

int TbUser_stat::OnQueryRsp(const Json::Value& item)
{
	return OnResponse(item);
}

int TbUser_stat::OnCountReq(string& sPostData, unsigned int udwIdxNo, const CompareDesc& comp_desc, 
		bool bConsistentRead, bool bReturnConsumedCapacity, bool bScanIndexForward, unsigned int dwLimit)
{
	AwsMap* pQuery = OnQueryReq(udwIdxNo, comp_desc, bConsistentRead, bReturnConsumedCapacity, bScanIndexForward, dwLimit, true);
	ostringstream oss;
	pQuery->Dump(oss);
	sPostData = oss.str();
	delete pQuery;
	return 0;
}

AwsMap* TbUser_stat::OnUpdateItemReq(
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
		if (TbUSER_STAT_FIELD_NEWEST_REPORTID == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/newest_reportid/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/newest_reportid/Value/N", m_nNewest_reportid);
				pUpdateItem->AddValue("/AttributeUpdates/newest_reportid/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbUSER_STAT_FIELD_UNREAD_REPORT == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/unread_report/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/unread_report/Value/N", m_nUnread_report);
				pUpdateItem->AddValue("/AttributeUpdates/unread_report/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbUSER_STAT_FIELD_NEWEST_MAILID == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/newest_mailid/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/newest_mailid/Value/N", m_nNewest_mailid);
				pUpdateItem->AddValue("/AttributeUpdates/newest_mailid/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbUSER_STAT_FIELD_UNREAD_MAIL == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/unread_mail/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/unread_mail/Value/N", m_nUnread_mail);
				pUpdateItem->AddValue("/AttributeUpdates/unread_mail/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbUSER_STAT_FIELD_AS_GET_T == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/as_get_t/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/as_get_t/Value/N", m_nAs_get_t);
				pUpdateItem->AddValue("/AttributeUpdates/as_get_t/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbUSER_STAT_FIELD_BIRTH_R == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/birth_r/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/birth_r/Value/N", m_nBirth_r);
				pUpdateItem->AddValue("/AttributeUpdates/birth_r/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbUSER_STAT_FIELD_BIRTH_M == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/birth_m/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/birth_m/Value/N", m_nBirth_m);
				pUpdateItem->AddValue("/AttributeUpdates/birth_m/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbUSER_STAT_FIELD_TIPS == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/tips/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/tips/Value/N", m_nTips);
				pUpdateItem->AddValue("/AttributeUpdates/tips/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbUSER_STAT_FIELD_WALL_GET_T == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/wall_get_t/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/wall_get_t/Value/N", m_nWall_get_t);
				pUpdateItem->AddValue("/AttributeUpdates/wall_get_t/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbUSER_STAT_FIELD_MARK == iter->first)
		{
			if (!m_bMark.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/mark/Value/B", Base64Encode((char*)&m_bMark.m_astList[0], m_bMark.m_udwNum*sizeof(SMarkItem), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/mark/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/mark/Action", "DELETE");
			}
			continue;
		}
		if (TbUSER_STAT_FIELD_STAR_MAIL == iter->first)
		{
			if (!m_bStar_mail.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/star_mail/Value/B", Base64Encode((char*)&m_bStar_mail.m_astList[0], m_bStar_mail.m_udwNum*sizeof(TINT64), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/star_mail/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/star_mail/Action", "DELETE");
			}
			continue;
		}
		if (TbUSER_STAT_FIELD_RETURN_MAILID == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/return_mailid/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/return_mailid/Value/N", m_nReturn_mailid);
				pUpdateItem->AddValue("/AttributeUpdates/return_mailid/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbUSER_STAT_FIELD_RETURN_REPORTID == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/return_reportid/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/return_reportid/Value/N", m_nReturn_reportid);
				pUpdateItem->AddValue("/AttributeUpdates/return_reportid/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbUSER_STAT_FIELD_REINFORCE_NUM == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/reinforce_num/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/reinforce_num/Value/N", m_nReinforce_num);
				pUpdateItem->AddValue("/AttributeUpdates/reinforce_num/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbUSER_STAT_FIELD_KILL_TROOP_NUM == iter->first)
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
		if (TbUSER_STAT_FIELD_LAST_EVENT_WIN_ID == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/last_event_win_id/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/last_event_win_id/Value/N", m_nLast_event_win_id);
				pUpdateItem->AddValue("/AttributeUpdates/last_event_win_id/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbUSER_STAT_FIELD_AL_EVENT_TIPS_TIME == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/al_event_tips_time/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/al_event_tips_time/Value/N", m_nAl_event_tips_time);
				pUpdateItem->AddValue("/AttributeUpdates/al_event_tips_time/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbUSER_STAT_FIELD_PLAYER_RECOMMEND_TIME == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/player_recommend_time/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/player_recommend_time/Value/N", m_nPlayer_recommend_time);
				pUpdateItem->AddValue("/AttributeUpdates/player_recommend_time/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbUSER_STAT_FIELD_BROADCAST_TIME == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/broadcast_time/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/broadcast_time/Value/N", m_nBroadcast_time);
				pUpdateItem->AddValue("/AttributeUpdates/broadcast_time/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbUSER_STAT_FIELD_MAIL_FLAG == iter->first)
		{
			if (!m_bMail_flag.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/mail_flag/Value/B", Base64Encode((char*)&m_bMail_flag.m_astList[0], m_bMail_flag.m_udwNum*sizeof(SMailFlag), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/mail_flag/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/mail_flag/Action", "DELETE");
			}
			continue;
		}
		if (TbUSER_STAT_FIELD_TOP_QUEST == iter->first)
		{
			if (!m_bTop_quest.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/top_quest/Value/B", Base64Encode((char*)&m_bTop_quest.m_astList[0], m_bTop_quest.m_udwNum*sizeof(STopQuest), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/top_quest/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/top_quest/Action", "DELETE");
			}
			continue;
		}
		if (TbUSER_STAT_FIELD_TOP_QUEST_FINISH == iter->first)
		{
			if (!m_bTop_quest_finish.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/top_quest_finish/Value/B", Base64Encode((char*)&m_bTop_quest_finish.m_astList[0], m_bTop_quest_finish.m_udwNum*sizeof(STopQuest), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/top_quest_finish/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/top_quest_finish/Action", "DELETE");
			}
			continue;
		}
		if (TbUSER_STAT_FIELD_LORD_LEVEL_FINISH == iter->first)
		{
			if (!m_bLord_level_finish.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/lord_level_finish/Value/B", Base64Encode((char*)&m_bLord_level_finish.m_astList[0], m_bLord_level_finish.m_udwNum*sizeof(SLevelQuest), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/lord_level_finish/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/lord_level_finish/Action", "DELETE");
			}
			continue;
		}
		if (TbUSER_STAT_FIELD_LAST_HELP_BUBBLE_SET_TIME == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/last_help_bubble_set_time/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/last_help_bubble_set_time/Value/N", m_nLast_help_bubble_set_time);
				pUpdateItem->AddValue("/AttributeUpdates/last_help_bubble_set_time/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbUSER_STAT_FIELD_LAST_HELP_BUBBLE_TIME_OUT == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/last_help_bubble_time_out/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/last_help_bubble_time_out/Value/N", m_nLast_help_bubble_time_out);
				pUpdateItem->AddValue("/AttributeUpdates/last_help_bubble_time_out/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbUSER_STAT_FIELD_DAILY_QUEST_FINISH_NUM == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/daily_quest_finish_num/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/daily_quest_finish_num/Value/N", m_nDaily_quest_finish_num);
				pUpdateItem->AddValue("/AttributeUpdates/daily_quest_finish_num/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbUSER_STAT_FIELD_AL_QUEST_FINISH_NUM == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/al_quest_finish_num/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/al_quest_finish_num/Value/N", m_nAl_quest_finish_num);
				pUpdateItem->AddValue("/AttributeUpdates/al_quest_finish_num/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbUSER_STAT_FIELD_VIP_QUEST_FINISH_NUM == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/vip_quest_finish_num/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/vip_quest_finish_num/Value/N", m_nVip_quest_finish_num);
				pUpdateItem->AddValue("/AttributeUpdates/vip_quest_finish_num/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbUSER_STAT_FIELD_LAST_MIGHT == iter->first)
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
		if (TbUSER_STAT_FIELD_LAST_TROOP_FORT == iter->first)
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
		if (TbUSER_STAT_FIELD_CON_LOGIN_DAYS == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/con_login_days/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/con_login_days/Value/N", m_nCon_login_days);
				pUpdateItem->AddValue("/AttributeUpdates/con_login_days/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbUSER_STAT_FIELD_MAX_DEFEAT_MONSTER_LV == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/max_defeat_monster_lv/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/max_defeat_monster_lv/Value/N", m_nMax_defeat_monster_lv);
				pUpdateItem->AddValue("/AttributeUpdates/max_defeat_monster_lv/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbUSER_STAT_FIELD_EQUIP_GRIDE == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/equip_gride/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/equip_gride/Value/N", m_nEquip_gride);
				pUpdateItem->AddValue("/AttributeUpdates/equip_gride/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbUSER_STAT_FIELD_REMOVE_FLAG == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/remove_flag/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/remove_flag/Value/N", m_nRemove_flag);
				pUpdateItem->AddValue("/AttributeUpdates/remove_flag/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbUSER_STAT_FIELD_LORD_SKILL == iter->first)
		{
			if (!m_bLord_skill.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/lord_skill/Value/B", Base64Encode((char*)&m_bLord_skill.m_astList[0], m_bLord_skill.m_udwNum*sizeof(SSkill), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/lord_skill/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/lord_skill/Action", "DELETE");
			}
			continue;
		}
		if (TbUSER_STAT_FIELD_DRAGON_SKILL == iter->first)
		{
			if (!m_bDragon_skill.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/dragon_skill/Value/B", Base64Encode((char*)&m_bDragon_skill.m_astList[0], m_bDragon_skill.m_udwNum*sizeof(SSkill), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/dragon_skill/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/dragon_skill/Action", "DELETE");
			}
			continue;
		}
		if (TbUSER_STAT_FIELD_DRAGON_MONSTER_SKILL == iter->first)
		{
			if (!m_bDragon_monster_skill.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/dragon_monster_skill/Value/B", Base64Encode((char*)&m_bDragon_monster_skill.m_astList[0], m_bDragon_monster_skill.m_udwNum*sizeof(SSkill), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/dragon_monster_skill/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/dragon_monster_skill/Action", "DELETE");
			}
			continue;
		}
		if (TbUSER_STAT_FIELD_DRAGON_LEVEL_FINISH == iter->first)
		{
			if (!m_bDragon_level_finish.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/dragon_level_finish/Value/B", Base64Encode((char*)&m_bDragon_level_finish.m_astList[0], m_bDragon_level_finish.m_udwNum*sizeof(SLevelQuest), sBase64Encode));
				pUpdateItem->AddValue("/AttributeUpdates/dragon_level_finish/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/dragon_level_finish/Action", "DELETE");
			}
			continue;
		}
		if (TbUSER_STAT_FIELD_REWARD_WINDOW_TIME == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/reward_window_time/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/reward_window_time/Value/N", m_nReward_window_time);
				pUpdateItem->AddValue("/AttributeUpdates/reward_window_time/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbUSER_STAT_FIELD_SEQ == iter->first)
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

int TbUser_stat::OnUpdateItemReq(string& sPostData,
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

int TbUser_stat::OnUpdateItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbUser_stat::OnWriteItemReq(int dwActionType)
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
		pItem->AddValue("/newest_reportid/N", m_nNewest_reportid);
		pItem->AddValue("/unread_report/N", m_nUnread_report);
		pItem->AddValue("/newest_mailid/N", m_nNewest_mailid);
		pItem->AddValue("/unread_mail/N", m_nUnread_mail);
		pItem->AddValue("/as_get_t/N", m_nAs_get_t);
		pItem->AddValue("/birth_r/N", m_nBirth_r);
		pItem->AddValue("/birth_m/N", m_nBirth_m);
		pItem->AddValue("/tips/N", m_nTips);
		pItem->AddValue("/wall_get_t/N", m_nWall_get_t);
		if (!m_bMark.empty())
		{
			pItem->AddValue("/mark/B", Base64Encode((char*)&m_bMark.m_astList[0], m_bMark.m_udwNum*sizeof(SMarkItem), sBase64Encode));
		}
		if (!m_bStar_mail.empty())
		{
			pItem->AddValue("/star_mail/B", Base64Encode((char*)&m_bStar_mail.m_astList[0], m_bStar_mail.m_udwNum*sizeof(TINT64), sBase64Encode));
		}
		pItem->AddValue("/return_mailid/N", m_nReturn_mailid);
		pItem->AddValue("/return_reportid/N", m_nReturn_reportid);
		pItem->AddValue("/reinforce_num/N", m_nReinforce_num);
		pItem->AddValue("/kill_troop_num/N", m_nKill_troop_num);
		pItem->AddValue("/last_event_win_id/N", m_nLast_event_win_id);
		pItem->AddValue("/al_event_tips_time/N", m_nAl_event_tips_time);
		pItem->AddValue("/player_recommend_time/N", m_nPlayer_recommend_time);
		pItem->AddValue("/broadcast_time/N", m_nBroadcast_time);
		if (!m_bMail_flag.empty())
		{
			pItem->AddValue("/mail_flag/B", Base64Encode((char*)&m_bMail_flag.m_astList[0], m_bMail_flag.m_udwNum*sizeof(SMailFlag), sBase64Encode));
		}
		if (!m_bTop_quest.empty())
		{
			pItem->AddValue("/top_quest/B", Base64Encode((char*)&m_bTop_quest.m_astList[0], m_bTop_quest.m_udwNum*sizeof(STopQuest), sBase64Encode));
		}
		if (!m_bTop_quest_finish.empty())
		{
			pItem->AddValue("/top_quest_finish/B", Base64Encode((char*)&m_bTop_quest_finish.m_astList[0], m_bTop_quest_finish.m_udwNum*sizeof(STopQuest), sBase64Encode));
		}
		if (!m_bLord_level_finish.empty())
		{
			pItem->AddValue("/lord_level_finish/B", Base64Encode((char*)&m_bLord_level_finish.m_astList[0], m_bLord_level_finish.m_udwNum*sizeof(SLevelQuest), sBase64Encode));
		}
		pItem->AddValue("/last_help_bubble_set_time/N", m_nLast_help_bubble_set_time);
		pItem->AddValue("/last_help_bubble_time_out/N", m_nLast_help_bubble_time_out);
		pItem->AddValue("/daily_quest_finish_num/N", m_nDaily_quest_finish_num);
		pItem->AddValue("/al_quest_finish_num/N", m_nAl_quest_finish_num);
		pItem->AddValue("/vip_quest_finish_num/N", m_nVip_quest_finish_num);
		pItem->AddValue("/last_might/N", m_nLast_might);
		if (!m_jLast_troop_fort.empty())
		{
				pItem->AddValue("/AttributeUpdates/last_troop_fort/Value/S", JsonEncode(oJsonWriter.write(m_jLast_troop_fort), sJsonEncode));
		}
		pItem->AddValue("/con_login_days/N", m_nCon_login_days);
		pItem->AddValue("/max_defeat_monster_lv/N", m_nMax_defeat_monster_lv);
		pItem->AddValue("/equip_gride/N", m_nEquip_gride);
		pItem->AddValue("/remove_flag/N", m_nRemove_flag);
		if (!m_bLord_skill.empty())
		{
			pItem->AddValue("/lord_skill/B", Base64Encode((char*)&m_bLord_skill.m_astList[0], m_bLord_skill.m_udwNum*sizeof(SSkill), sBase64Encode));
		}
		if (!m_bDragon_skill.empty())
		{
			pItem->AddValue("/dragon_skill/B", Base64Encode((char*)&m_bDragon_skill.m_astList[0], m_bDragon_skill.m_udwNum*sizeof(SSkill), sBase64Encode));
		}
		if (!m_bDragon_monster_skill.empty())
		{
			pItem->AddValue("/dragon_monster_skill/B", Base64Encode((char*)&m_bDragon_monster_skill.m_astList[0], m_bDragon_monster_skill.m_udwNum*sizeof(SSkill), sBase64Encode));
		}
		if (!m_bDragon_level_finish.empty())
		{
			pItem->AddValue("/dragon_level_finish/B", Base64Encode((char*)&m_bDragon_level_finish.m_astList[0], m_bDragon_level_finish.m_udwNum*sizeof(SLevelQuest), sBase64Encode));
		}
		pItem->AddValue("/reward_window_time/N", m_nReward_window_time);
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

void TbUser_stat::OnWriteItemReq(AwsMap* pWriteItem, int dwActionType)
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
		pItem->AddValue("/newest_reportid/N", m_nNewest_reportid);
		pItem->AddValue("/unread_report/N", m_nUnread_report);
		pItem->AddValue("/newest_mailid/N", m_nNewest_mailid);
		pItem->AddValue("/unread_mail/N", m_nUnread_mail);
		pItem->AddValue("/as_get_t/N", m_nAs_get_t);
		pItem->AddValue("/birth_r/N", m_nBirth_r);
		pItem->AddValue("/birth_m/N", m_nBirth_m);
		pItem->AddValue("/tips/N", m_nTips);
		pItem->AddValue("/wall_get_t/N", m_nWall_get_t);
		if (!m_bMark.empty())
		{
			pItem->AddValue("/mark/B", Base64Encode((char*)&m_bMark.m_astList[0], m_bMark.m_udwNum*sizeof(SMarkItem), sBase64Encode));
		}
		if (!m_bStar_mail.empty())
		{
			pItem->AddValue("/star_mail/B", Base64Encode((char*)&m_bStar_mail.m_astList[0], m_bStar_mail.m_udwNum*sizeof(TINT64), sBase64Encode));
		}
		pItem->AddValue("/return_mailid/N", m_nReturn_mailid);
		pItem->AddValue("/return_reportid/N", m_nReturn_reportid);
		pItem->AddValue("/reinforce_num/N", m_nReinforce_num);
		pItem->AddValue("/kill_troop_num/N", m_nKill_troop_num);
		pItem->AddValue("/last_event_win_id/N", m_nLast_event_win_id);
		pItem->AddValue("/al_event_tips_time/N", m_nAl_event_tips_time);
		pItem->AddValue("/player_recommend_time/N", m_nPlayer_recommend_time);
		pItem->AddValue("/broadcast_time/N", m_nBroadcast_time);
		if (!m_bMail_flag.empty())
		{
			pItem->AddValue("/mail_flag/B", Base64Encode((char*)&m_bMail_flag.m_astList[0], m_bMail_flag.m_udwNum*sizeof(SMailFlag), sBase64Encode));
		}
		if (!m_bTop_quest.empty())
		{
			pItem->AddValue("/top_quest/B", Base64Encode((char*)&m_bTop_quest.m_astList[0], m_bTop_quest.m_udwNum*sizeof(STopQuest), sBase64Encode));
		}
		if (!m_bTop_quest_finish.empty())
		{
			pItem->AddValue("/top_quest_finish/B", Base64Encode((char*)&m_bTop_quest_finish.m_astList[0], m_bTop_quest_finish.m_udwNum*sizeof(STopQuest), sBase64Encode));
		}
		if (!m_bLord_level_finish.empty())
		{
			pItem->AddValue("/lord_level_finish/B", Base64Encode((char*)&m_bLord_level_finish.m_astList[0], m_bLord_level_finish.m_udwNum*sizeof(SLevelQuest), sBase64Encode));
		}
		pItem->AddValue("/last_help_bubble_set_time/N", m_nLast_help_bubble_set_time);
		pItem->AddValue("/last_help_bubble_time_out/N", m_nLast_help_bubble_time_out);
		pItem->AddValue("/daily_quest_finish_num/N", m_nDaily_quest_finish_num);
		pItem->AddValue("/al_quest_finish_num/N", m_nAl_quest_finish_num);
		pItem->AddValue("/vip_quest_finish_num/N", m_nVip_quest_finish_num);
		pItem->AddValue("/last_might/N", m_nLast_might);
		if (!m_jLast_troop_fort.empty())
		{
				pItem->AddValue("/AttributeUpdates/last_troop_fort/Value/S", JsonEncode(oJsonWriter.write(m_jLast_troop_fort), sJsonEncode));
		}
		pItem->AddValue("/con_login_days/N", m_nCon_login_days);
		pItem->AddValue("/max_defeat_monster_lv/N", m_nMax_defeat_monster_lv);
		pItem->AddValue("/equip_gride/N", m_nEquip_gride);
		pItem->AddValue("/remove_flag/N", m_nRemove_flag);
		if (!m_bLord_skill.empty())
		{
			pItem->AddValue("/lord_skill/B", Base64Encode((char*)&m_bLord_skill.m_astList[0], m_bLord_skill.m_udwNum*sizeof(SSkill), sBase64Encode));
		}
		if (!m_bDragon_skill.empty())
		{
			pItem->AddValue("/dragon_skill/B", Base64Encode((char*)&m_bDragon_skill.m_astList[0], m_bDragon_skill.m_udwNum*sizeof(SSkill), sBase64Encode));
		}
		if (!m_bDragon_monster_skill.empty())
		{
			pItem->AddValue("/dragon_monster_skill/B", Base64Encode((char*)&m_bDragon_monster_skill.m_astList[0], m_bDragon_monster_skill.m_udwNum*sizeof(SSkill), sBase64Encode));
		}
		if (!m_bDragon_level_finish.empty())
		{
			pItem->AddValue("/dragon_level_finish/B", Base64Encode((char*)&m_bDragon_level_finish.m_astList[0], m_bDragon_level_finish.m_udwNum*sizeof(SLevelQuest), sBase64Encode));
		}
		pItem->AddValue("/reward_window_time/N", m_nReward_window_time);
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

AwsMap* TbUser_stat::OnReadItemReq(unsigned int udwIdxNo)
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

void TbUser_stat::OnReadItemReq(AwsMap* pReadItem)
{
	assert(pReadItem);
	AwsList* pKeys = pReadItem->GetAwsMap("RequestItems")->GetAwsMap(GetTableName())->GetAwsList("Keys");
	AwsMap* pKey = new AwsMap;
	assert(pKey);
	pKey->AddValue("/uid/N", m_nUid);
	pKeys->SetValue(pKey, true);
}

int TbUser_stat::OnReadItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbUser_stat::OnDeleteItemReq(
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

int TbUser_stat::OnDeleteItemReq(string& sPostData,
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

int TbUser_stat::OnDeleteItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbUser_stat::OnGetItemReq(unsigned int udwIdxNo,
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

int TbUser_stat::OnGetItemReq(string& sPostData, unsigned int udwIdxNo,
	bool bConsistentRead, bool bReturnConsumedCapacity)
{
	AwsMap* pGetItem = OnGetItemReq(udwIdxNo, bConsistentRead, bReturnConsumedCapacity);
	ostringstream oss;
	pGetItem->Dump(oss);
	sPostData = oss.str();
	delete pGetItem;
	return 0;
}

int TbUser_stat::OnGetItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbUser_stat::OnPutItemReq(
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
	pPutItem->AddValue("/Item/newest_reportid/N", m_nNewest_reportid);
	pPutItem->AddValue("/Item/unread_report/N", m_nUnread_report);
	pPutItem->AddValue("/Item/newest_mailid/N", m_nNewest_mailid);
	pPutItem->AddValue("/Item/unread_mail/N", m_nUnread_mail);
	pPutItem->AddValue("/Item/as_get_t/N", m_nAs_get_t);
	pPutItem->AddValue("/Item/birth_r/N", m_nBirth_r);
	pPutItem->AddValue("/Item/birth_m/N", m_nBirth_m);
	pPutItem->AddValue("/Item/tips/N", m_nTips);
	pPutItem->AddValue("/Item/wall_get_t/N", m_nWall_get_t);
	if (!m_bMark.empty())
	{
		pPutItem->AddValue("/Item/mark/B", Base64Encode((char*)&m_bMark.m_astList[0], m_bMark.m_udwNum*sizeof(SMarkItem), sBase64Encode));
	}
	if (!m_bStar_mail.empty())
	{
		pPutItem->AddValue("/Item/star_mail/B", Base64Encode((char*)&m_bStar_mail.m_astList[0], m_bStar_mail.m_udwNum*sizeof(TINT64), sBase64Encode));
	}
	pPutItem->AddValue("/Item/return_mailid/N", m_nReturn_mailid);
	pPutItem->AddValue("/Item/return_reportid/N", m_nReturn_reportid);
	pPutItem->AddValue("/Item/reinforce_num/N", m_nReinforce_num);
	pPutItem->AddValue("/Item/kill_troop_num/N", m_nKill_troop_num);
	pPutItem->AddValue("/Item/last_event_win_id/N", m_nLast_event_win_id);
	pPutItem->AddValue("/Item/al_event_tips_time/N", m_nAl_event_tips_time);
	pPutItem->AddValue("/Item/player_recommend_time/N", m_nPlayer_recommend_time);
	pPutItem->AddValue("/Item/broadcast_time/N", m_nBroadcast_time);
	if (!m_bMail_flag.empty())
	{
		pPutItem->AddValue("/Item/mail_flag/B", Base64Encode((char*)&m_bMail_flag.m_astList[0], m_bMail_flag.m_udwNum*sizeof(SMailFlag), sBase64Encode));
	}
	if (!m_bTop_quest.empty())
	{
		pPutItem->AddValue("/Item/top_quest/B", Base64Encode((char*)&m_bTop_quest.m_astList[0], m_bTop_quest.m_udwNum*sizeof(STopQuest), sBase64Encode));
	}
	if (!m_bTop_quest_finish.empty())
	{
		pPutItem->AddValue("/Item/top_quest_finish/B", Base64Encode((char*)&m_bTop_quest_finish.m_astList[0], m_bTop_quest_finish.m_udwNum*sizeof(STopQuest), sBase64Encode));
	}
	if (!m_bLord_level_finish.empty())
	{
		pPutItem->AddValue("/Item/lord_level_finish/B", Base64Encode((char*)&m_bLord_level_finish.m_astList[0], m_bLord_level_finish.m_udwNum*sizeof(SLevelQuest), sBase64Encode));
	}
	pPutItem->AddValue("/Item/last_help_bubble_set_time/N", m_nLast_help_bubble_set_time);
	pPutItem->AddValue("/Item/last_help_bubble_time_out/N", m_nLast_help_bubble_time_out);
	pPutItem->AddValue("/Item/daily_quest_finish_num/N", m_nDaily_quest_finish_num);
	pPutItem->AddValue("/Item/al_quest_finish_num/N", m_nAl_quest_finish_num);
	pPutItem->AddValue("/Item/vip_quest_finish_num/N", m_nVip_quest_finish_num);
	pPutItem->AddValue("/Item/last_might/N", m_nLast_might);
	if (!m_jLast_troop_fort.empty())
	{
				pPutItem->AddValue("/Item/last_troop_fort/S", JsonEncode(oJsonWriter.write(m_jLast_troop_fort), sJsonEncode));
	}
	pPutItem->AddValue("/Item/con_login_days/N", m_nCon_login_days);
	pPutItem->AddValue("/Item/max_defeat_monster_lv/N", m_nMax_defeat_monster_lv);
	pPutItem->AddValue("/Item/equip_gride/N", m_nEquip_gride);
	pPutItem->AddValue("/Item/remove_flag/N", m_nRemove_flag);
	if (!m_bLord_skill.empty())
	{
		pPutItem->AddValue("/Item/lord_skill/B", Base64Encode((char*)&m_bLord_skill.m_astList[0], m_bLord_skill.m_udwNum*sizeof(SSkill), sBase64Encode));
	}
	if (!m_bDragon_skill.empty())
	{
		pPutItem->AddValue("/Item/dragon_skill/B", Base64Encode((char*)&m_bDragon_skill.m_astList[0], m_bDragon_skill.m_udwNum*sizeof(SSkill), sBase64Encode));
	}
	if (!m_bDragon_monster_skill.empty())
	{
		pPutItem->AddValue("/Item/dragon_monster_skill/B", Base64Encode((char*)&m_bDragon_monster_skill.m_astList[0], m_bDragon_monster_skill.m_udwNum*sizeof(SSkill), sBase64Encode));
	}
	if (!m_bDragon_level_finish.empty())
	{
		pPutItem->AddValue("/Item/dragon_level_finish/B", Base64Encode((char*)&m_bDragon_level_finish.m_astList[0], m_bDragon_level_finish.m_udwNum*sizeof(SLevelQuest), sBase64Encode));
	}
	pPutItem->AddValue("/Item/reward_window_time/N", m_nReward_window_time);
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

int TbUser_stat::OnPutItemReq(string& sPostData,
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

int TbUser_stat::OnPutItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

int TbUser_stat::OnResponse(const Json::Value& item)
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
		if (vecMembers[i] == "newest_reportid")
		{
			m_nNewest_reportid = strtoll(item["newest_reportid"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "unread_report")
		{
			m_nUnread_report = strtoll(item["unread_report"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "newest_mailid")
		{
			m_nNewest_mailid = strtoll(item["newest_mailid"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "unread_mail")
		{
			m_nUnread_mail = strtoll(item["unread_mail"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "as_get_t")
		{
			m_nAs_get_t = strtoll(item["as_get_t"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "birth_r")
		{
			m_nBirth_r = strtoll(item["birth_r"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "birth_m")
		{
			m_nBirth_m = strtoll(item["birth_m"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "tips")
		{
			m_nTips = strtoll(item["tips"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "wall_get_t")
		{
			m_nWall_get_t = strtoll(item["wall_get_t"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "mark")
		{
			Base64Decode(item["mark"]["B"].asString(), (char*)&m_bMark.m_astList[0], dwResLen);
			m_bMark.m_udwNum = dwResLen/sizeof(SMarkItem);
			continue;
		}
		if (vecMembers[i] == "star_mail")
		{
			Base64Decode(item["star_mail"]["B"].asString(), (char*)&m_bStar_mail.m_astList[0], dwResLen);
			m_bStar_mail.m_udwNum = dwResLen/sizeof(TINT64);
			continue;
		}
		if (vecMembers[i] == "return_mailid")
		{
			m_nReturn_mailid = strtoll(item["return_mailid"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "return_reportid")
		{
			m_nReturn_reportid = strtoll(item["return_reportid"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "reinforce_num")
		{
			m_nReinforce_num = strtoll(item["reinforce_num"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "kill_troop_num")
		{
			m_nKill_troop_num = strtoll(item["kill_troop_num"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "last_event_win_id")
		{
			m_nLast_event_win_id = strtoll(item["last_event_win_id"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "al_event_tips_time")
		{
			m_nAl_event_tips_time = strtoll(item["al_event_tips_time"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "player_recommend_time")
		{
			m_nPlayer_recommend_time = strtoll(item["player_recommend_time"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "broadcast_time")
		{
			m_nBroadcast_time = strtoll(item["broadcast_time"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "mail_flag")
		{
			Base64Decode(item["mail_flag"]["B"].asString(), (char*)&m_bMail_flag.m_astList[0], dwResLen);
			m_bMail_flag.m_udwNum = dwResLen/sizeof(SMailFlag);
			if (0==m_bMail_flag.m_udwNum && dwResLen>0)
			{
				m_bMail_flag.m_udwNum = 1;
			}
			continue;
		}
		if (vecMembers[i] == "top_quest")
		{
			Base64Decode(item["top_quest"]["B"].asString(), (char*)&m_bTop_quest.m_astList[0], dwResLen);
			m_bTop_quest.m_udwNum = dwResLen/sizeof(STopQuest);
			if (0==m_bTop_quest.m_udwNum && dwResLen>0)
			{
				m_bTop_quest.m_udwNum = 1;
			}
			continue;
		}
		if (vecMembers[i] == "top_quest_finish")
		{
			Base64Decode(item["top_quest_finish"]["B"].asString(), (char*)&m_bTop_quest_finish.m_astList[0], dwResLen);
			m_bTop_quest_finish.m_udwNum = dwResLen/sizeof(STopQuest);
			if (0==m_bTop_quest_finish.m_udwNum && dwResLen>0)
			{
				m_bTop_quest_finish.m_udwNum = 1;
			}
			continue;
		}
		if (vecMembers[i] == "lord_level_finish")
		{
			Base64Decode(item["lord_level_finish"]["B"].asString(), (char*)&m_bLord_level_finish.m_astList[0], dwResLen);
			m_bLord_level_finish.m_udwNum = dwResLen/sizeof(SLevelQuest);
			if (0==m_bLord_level_finish.m_udwNum && dwResLen>0)
			{
				m_bLord_level_finish.m_udwNum = 1;
			}
			continue;
		}
		if (vecMembers[i] == "last_help_bubble_set_time")
		{
			m_nLast_help_bubble_set_time = strtoll(item["last_help_bubble_set_time"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "last_help_bubble_time_out")
		{
			m_nLast_help_bubble_time_out = strtoll(item["last_help_bubble_time_out"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "daily_quest_finish_num")
		{
			m_nDaily_quest_finish_num = strtoll(item["daily_quest_finish_num"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "al_quest_finish_num")
		{
			m_nAl_quest_finish_num = strtoll(item["al_quest_finish_num"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "vip_quest_finish_num")
		{
			m_nVip_quest_finish_num = strtoll(item["vip_quest_finish_num"]["N"].asString().c_str(), NULL, 10);
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
		if (vecMembers[i] == "con_login_days")
		{
			m_nCon_login_days = strtoll(item["con_login_days"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "max_defeat_monster_lv")
		{
			m_nMax_defeat_monster_lv = strtoll(item["max_defeat_monster_lv"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "equip_gride")
		{
			m_nEquip_gride = strtoll(item["equip_gride"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "remove_flag")
		{
			m_nRemove_flag = strtoll(item["remove_flag"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "lord_skill")
		{
			Base64Decode(item["lord_skill"]["B"].asString(), (char*)&m_bLord_skill.m_astList[0], dwResLen);
			m_bLord_skill.m_udwNum = dwResLen/sizeof(SSkill);
			if (0==m_bLord_skill.m_udwNum && dwResLen>0)
			{
				m_bLord_skill.m_udwNum = 1;
			}
			continue;
		}
		if (vecMembers[i] == "dragon_skill")
		{
			Base64Decode(item["dragon_skill"]["B"].asString(), (char*)&m_bDragon_skill.m_astList[0], dwResLen);
			m_bDragon_skill.m_udwNum = dwResLen/sizeof(SSkill);
			if (0==m_bDragon_skill.m_udwNum && dwResLen>0)
			{
				m_bDragon_skill.m_udwNum = 1;
			}
			continue;
		}
		if (vecMembers[i] == "dragon_monster_skill")
		{
			Base64Decode(item["dragon_monster_skill"]["B"].asString(), (char*)&m_bDragon_monster_skill.m_astList[0], dwResLen);
			m_bDragon_monster_skill.m_udwNum = dwResLen/sizeof(SSkill);
			if (0==m_bDragon_monster_skill.m_udwNum && dwResLen>0)
			{
				m_bDragon_monster_skill.m_udwNum = 1;
			}
			continue;
		}
		if (vecMembers[i] == "dragon_level_finish")
		{
			Base64Decode(item["dragon_level_finish"]["B"].asString(), (char*)&m_bDragon_level_finish.m_astList[0], dwResLen);
			m_bDragon_level_finish.m_udwNum = dwResLen/sizeof(SLevelQuest);
			if (0==m_bDragon_level_finish.m_udwNum && dwResLen>0)
			{
				m_bDragon_level_finish.m_udwNum = 1;
			}
			continue;
		}
		if (vecMembers[i] == "reward_window_time")
		{
			m_nReward_window_time = strtoll(item["reward_window_time"]["N"].asString().c_str(), NULL, 10);
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

TINT64 TbUser_stat::GetSeq()
{
	return m_nSeq;
}

