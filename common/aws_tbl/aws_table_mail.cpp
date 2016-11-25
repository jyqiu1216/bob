#include "aws_table_mail.h"

TableDesc TbMail::oTableDesc;

int TbMail::Init(const string& sConfFile, const string strProjectName)
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

AwsTable* TbMail::NewObject()
{
	return new TbMail;
}

string TbMail::GetTableName()
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

TINT32 TbMail::GetTableIdx()
{
	return this->m_nId/1000000;
}

AwsMap* TbMail::OnScanReq(unsigned int udwIdxNo, bool bHasStartKey, bool bReturnConsumedCapacity,
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

int TbMail::OnScanReq(string& sPostData, unsigned int udwIdxNo, bool bHasStartKey, bool bReturnConsumedCapacity,
 unsigned int dwLimit, unsigned int dwSegment, unsigned int dwTotalSegments)
{
	AwsMap* pScan = OnScanReq(udwIdxNo, bHasStartKey, bReturnConsumedCapacity, dwLimit, dwSegment, dwTotalSegments);
	ostringstream oss;
	pScan->Dump(oss);
	sPostData = oss.str();
	delete pScan;
	return 0;
}

int TbMail::OnScanRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbMail::OnQueryReq(unsigned int udwIdxNo, const CompareDesc& comp_desc,
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
			if(fld_desc.udwFldNo == TbMAIL_FIELD_ID)
			{
				pQuery->AddValue("/KeyConditions/id/AttributeValueList[0]/N", m_nId);
				pQuery->AddValue("/KeyConditions/id/ComparisonOperator", "EQ");
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

int TbMail::OnQueryReq(string& sPostData, unsigned int udwIdxNo, const CompareDesc& comp_desc,
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

int TbMail::OnQueryRsp(const Json::Value& item)
{
	return OnResponse(item);
}

int TbMail::OnCountReq(string& sPostData, unsigned int udwIdxNo, const CompareDesc& comp_desc, 
		bool bConsistentRead, bool bReturnConsumedCapacity, bool bScanIndexForward, unsigned int dwLimit)
{
	AwsMap* pQuery = OnQueryReq(udwIdxNo, comp_desc, bConsistentRead, bReturnConsumedCapacity, bScanIndexForward, dwLimit, true);
	ostringstream oss;
	pQuery->Dump(oss);
	sPostData = oss.str();
	delete pQuery;
	return 0;
}

AwsMap* TbMail::OnUpdateItemReq(
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
		if (TbMAIL_FIELD_TIME == iter->first)
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
		if (TbMAIL_FIELD_SUID == iter->first)
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
		if (TbMAIL_FIELD_SENDER == iter->first)
		{
			if (!m_sSender.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/sender/Value/S", JsonEncode(m_sSender, sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/sender/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/sender/Action", "DELETE");
			}
			continue;
		}
		if (TbMAIL_FIELD_SCID == iter->first)
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
		if (TbMAIL_FIELD_RUID == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/ruid/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/ruid/Value/N", m_nRuid);
				pUpdateItem->AddValue("/AttributeUpdates/ruid/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMAIL_FIELD_SEND_TYPE == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/send_type/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/send_type/Value/N", m_nSend_type);
				pUpdateItem->AddValue("/AttributeUpdates/send_type/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMAIL_FIELD_TITLE == iter->first)
		{
			if (!m_sTitle.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/title/Value/S", JsonEncode(m_sTitle, sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/title/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/title/Action", "DELETE");
			}
			continue;
		}
		if (TbMAIL_FIELD_CONTENT == iter->first)
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
		if (TbMAIL_FIELD_LINK == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/link/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/link/Value/N", m_nLink);
				pUpdateItem->AddValue("/AttributeUpdates/link/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMAIL_FIELD_URL == iter->first)
		{
			if (!m_sUrl.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/url/Value/S", JsonEncode(m_sUrl, sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/url/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/url/Action", "DELETE");
			}
			continue;
		}
		if (TbMAIL_FIELD_MAILDOCID == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/maildocid/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/maildocid/Value/N", m_nMaildocid);
				pUpdateItem->AddValue("/AttributeUpdates/maildocid/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMAIL_FIELD_DISPLAY_TYPE == iter->first)
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
		if (TbMAIL_FIELD_ESCS == iter->first)
		{
			if (!m_sEscs.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/escs/Value/S", JsonEncode(m_sEscs, sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/escs/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/escs/Action", "DELETE");
			}
			continue;
		}
		if (TbMAIL_FIELD_SENDER_AL_NICK == iter->first)
		{
			if (!m_sSender_al_nick.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/sender_al_nick/Value/S", JsonEncode(m_sSender_al_nick, sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/sender_al_nick/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/sender_al_nick/Action", "DELETE");
			}
			continue;
		}
		if (TbMAIL_FIELD_SENDER_AL_INFO == iter->first)
		{
			if (!m_sSender_al_info.empty())
			{
				pUpdateItem->AddValue("/AttributeUpdates/sender_al_info/Value/S", JsonEncode(m_sSender_al_info, sJsonEncode));
				pUpdateItem->AddValue("/AttributeUpdates/sender_al_info/Action", "PUT");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/sender_al_info/Action", "DELETE");
			}
			continue;
		}
		if (TbMAIL_FIELD_REWARD == iter->first)
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
		if (TbMAIL_FIELD_SENDER_PLAYER_AVATAR == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/sender_player_avatar/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/sender_player_avatar/Value/N", m_nSender_player_avatar);
				pUpdateItem->AddValue("/AttributeUpdates/sender_player_avatar/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMAIL_FIELD_RAW_LANG == iter->first)
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
		if (TbMAIL_FIELD_TRANSLATE_CONTENT == iter->first)
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
		if (TbMAIL_FIELD_SEQ == iter->first)
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
		if (TbMAIL_FIELD_JMP_TYPE == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/jmp_type/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/jmp_type/Value/N", m_nJmp_type);
				pUpdateItem->AddValue("/AttributeUpdates/jmp_type/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMAIL_FIELD_EVENT_ID == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/event_id/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/event_id/Value/N", m_nEvent_id);
				pUpdateItem->AddValue("/AttributeUpdates/event_id/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMAIL_FIELD_EVENT_TYPE == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/event_type/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/event_type/Value/N", m_nEvent_type);
				pUpdateItem->AddValue("/AttributeUpdates/event_type/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMAIL_FIELD_EVENT_SCORE == iter->first)
		{
			if(UPDATE_ACTION_TYPE_DELETE == iter->second)
			{
				pUpdateItem->AddValue("/AttributeUpdates/event_score/Action", "DELETE");
			}
			else
			{
				pUpdateItem->AddValue("/AttributeUpdates/event_score/Value/N", m_nEvent_score);
				pUpdateItem->AddValue("/AttributeUpdates/event_score/Action", UpdateActionType2Str(iter->second));
			}
			continue;
		}
		if (TbMAIL_FIELD_EX_REWARD == iter->first)
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

int TbMail::OnUpdateItemReq(string& sPostData,
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

int TbMail::OnUpdateItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbMail::OnWriteItemReq(int dwActionType)
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
		pItem->AddValue("/time/N", m_nTime);
		pItem->AddValue("/suid/N", m_nSuid);
		if (!m_sSender.empty())
		{
			pItem->AddValue("/sender/S", JsonEncode(m_sSender, sJsonEncode));
		}
		pItem->AddValue("/scid/N", m_nScid);
		pItem->AddValue("/ruid/N", m_nRuid);
		pItem->AddValue("/send_type/N", m_nSend_type);
		if (!m_sTitle.empty())
		{
			pItem->AddValue("/title/S", JsonEncode(m_sTitle, sJsonEncode));
		}
		if (!m_sContent.empty())
		{
			pItem->AddValue("/content/S", JsonEncode(m_sContent, sJsonEncode));
		}
		pItem->AddValue("/link/N", m_nLink);
		if (!m_sUrl.empty())
		{
			pItem->AddValue("/url/S", JsonEncode(m_sUrl, sJsonEncode));
		}
		pItem->AddValue("/maildocid/N", m_nMaildocid);
		pItem->AddValue("/display_type/N", m_nDisplay_type);
		if (!m_sEscs.empty())
		{
			pItem->AddValue("/escs/S", JsonEncode(m_sEscs, sJsonEncode));
		}
		if (!m_sSender_al_nick.empty())
		{
			pItem->AddValue("/sender_al_nick/S", JsonEncode(m_sSender_al_nick, sJsonEncode));
		}
		if (!m_sSender_al_info.empty())
		{
			pItem->AddValue("/sender_al_info/S", JsonEncode(m_sSender_al_info, sJsonEncode));
		}
		if (!m_bReward.empty())
		{
			pItem->AddValue("/reward/B", Base64Encode((char*)&m_bReward.m_astList[0], m_bReward.m_udwNum*sizeof(SGlobalRes), sBase64Encode));
		}
		pItem->AddValue("/sender_player_avatar/N", m_nSender_player_avatar);
		pItem->AddValue("/raw_lang/N", m_nRaw_lang);
		if (!m_sTranslate_content.empty())
		{
			pItem->AddValue("/translate_content/S", JsonEncode(m_sTranslate_content, sJsonEncode));
		}
		pItem->AddValue("/seq/N", m_nSeq);
		pItem->AddValue("/jmp_type/N", m_nJmp_type);
		pItem->AddValue("/event_id/N", m_nEvent_id);
		pItem->AddValue("/event_type/N", m_nEvent_type);
		pItem->AddValue("/event_score/N", m_nEvent_score);
		if (!m_bEx_reward.empty())
		{
			pItem->AddValue("/ex_reward/B", Base64Encode((char*)&m_bEx_reward.m_astList[0], m_bEx_reward.m_udwNum*sizeof(SOneGlobalRes), sBase64Encode));
		}
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

void TbMail::OnWriteItemReq(AwsMap* pWriteItem, int dwActionType)
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
		pItem->AddValue("/time/N", m_nTime);
		pItem->AddValue("/suid/N", m_nSuid);
		if (!m_sSender.empty())
		{
			pItem->AddValue("/sender/S", JsonEncode(m_sSender, sJsonEncode));
		}
		pItem->AddValue("/scid/N", m_nScid);
		pItem->AddValue("/ruid/N", m_nRuid);
		pItem->AddValue("/send_type/N", m_nSend_type);
		if (!m_sTitle.empty())
		{
			pItem->AddValue("/title/S", JsonEncode(m_sTitle, sJsonEncode));
		}
		if (!m_sContent.empty())
		{
			pItem->AddValue("/content/S", JsonEncode(m_sContent, sJsonEncode));
		}
		pItem->AddValue("/link/N", m_nLink);
		if (!m_sUrl.empty())
		{
			pItem->AddValue("/url/S", JsonEncode(m_sUrl, sJsonEncode));
		}
		pItem->AddValue("/maildocid/N", m_nMaildocid);
		pItem->AddValue("/display_type/N", m_nDisplay_type);
		if (!m_sEscs.empty())
		{
			pItem->AddValue("/escs/S", JsonEncode(m_sEscs, sJsonEncode));
		}
		if (!m_sSender_al_nick.empty())
		{
			pItem->AddValue("/sender_al_nick/S", JsonEncode(m_sSender_al_nick, sJsonEncode));
		}
		if (!m_sSender_al_info.empty())
		{
			pItem->AddValue("/sender_al_info/S", JsonEncode(m_sSender_al_info, sJsonEncode));
		}
		if (!m_bReward.empty())
		{
			pItem->AddValue("/reward/B", Base64Encode((char*)&m_bReward.m_astList[0], m_bReward.m_udwNum*sizeof(SGlobalRes), sBase64Encode));
		}
		pItem->AddValue("/sender_player_avatar/N", m_nSender_player_avatar);
		pItem->AddValue("/raw_lang/N", m_nRaw_lang);
		if (!m_sTranslate_content.empty())
		{
			pItem->AddValue("/translate_content/S", JsonEncode(m_sTranslate_content, sJsonEncode));
		}
		pItem->AddValue("/seq/N", m_nSeq);
		pItem->AddValue("/jmp_type/N", m_nJmp_type);
		pItem->AddValue("/event_id/N", m_nEvent_id);
		pItem->AddValue("/event_type/N", m_nEvent_type);
		pItem->AddValue("/event_score/N", m_nEvent_score);
		if (!m_bEx_reward.empty())
		{
			pItem->AddValue("/ex_reward/B", Base64Encode((char*)&m_bEx_reward.m_astList[0], m_bEx_reward.m_udwNum*sizeof(SOneGlobalRes), sBase64Encode));
		}
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

AwsMap* TbMail::OnReadItemReq(unsigned int udwIdxNo)
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

void TbMail::OnReadItemReq(AwsMap* pReadItem)
{
	assert(pReadItem);
	AwsList* pKeys = pReadItem->GetAwsMap("RequestItems")->GetAwsMap(GetTableName())->GetAwsList("Keys");
	AwsMap* pKey = new AwsMap;
	assert(pKey);
	pKey->AddValue("/id/N", m_nId);
	pKeys->SetValue(pKey, true);
}

int TbMail::OnReadItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbMail::OnDeleteItemReq(
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

int TbMail::OnDeleteItemReq(string& sPostData,
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

int TbMail::OnDeleteItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbMail::OnGetItemReq(unsigned int udwIdxNo,
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

int TbMail::OnGetItemReq(string& sPostData, unsigned int udwIdxNo,
	bool bConsistentRead, bool bReturnConsumedCapacity)
{
	AwsMap* pGetItem = OnGetItemReq(udwIdxNo, bConsistentRead, bReturnConsumedCapacity);
	ostringstream oss;
	pGetItem->Dump(oss);
	sPostData = oss.str();
	delete pGetItem;
	return 0;
}

int TbMail::OnGetItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

AwsMap* TbMail::OnPutItemReq(
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
	pPutItem->AddValue("/Item/time/N", m_nTime);
	pPutItem->AddValue("/Item/suid/N", m_nSuid);
	if (!m_sSender.empty())
	{
		pPutItem->AddValue("/Item/sender/S", JsonEncode(m_sSender, sJsonEncode));
	}
	pPutItem->AddValue("/Item/scid/N", m_nScid);
	pPutItem->AddValue("/Item/ruid/N", m_nRuid);
	pPutItem->AddValue("/Item/send_type/N", m_nSend_type);
	if (!m_sTitle.empty())
	{
		pPutItem->AddValue("/Item/title/S", JsonEncode(m_sTitle, sJsonEncode));
	}
	if (!m_sContent.empty())
	{
		pPutItem->AddValue("/Item/content/S", JsonEncode(m_sContent, sJsonEncode));
	}
	pPutItem->AddValue("/Item/link/N", m_nLink);
	if (!m_sUrl.empty())
	{
		pPutItem->AddValue("/Item/url/S", JsonEncode(m_sUrl, sJsonEncode));
	}
	pPutItem->AddValue("/Item/maildocid/N", m_nMaildocid);
	pPutItem->AddValue("/Item/display_type/N", m_nDisplay_type);
	if (!m_sEscs.empty())
	{
		pPutItem->AddValue("/Item/escs/S", JsonEncode(m_sEscs, sJsonEncode));
	}
	if (!m_sSender_al_nick.empty())
	{
		pPutItem->AddValue("/Item/sender_al_nick/S", JsonEncode(m_sSender_al_nick, sJsonEncode));
	}
	if (!m_sSender_al_info.empty())
	{
		pPutItem->AddValue("/Item/sender_al_info/S", JsonEncode(m_sSender_al_info, sJsonEncode));
	}
	if (!m_bReward.empty())
	{
		pPutItem->AddValue("/Item/reward/B", Base64Encode((char*)&m_bReward.m_astList[0], m_bReward.m_udwNum*sizeof(SGlobalRes), sBase64Encode));
	}
	pPutItem->AddValue("/Item/sender_player_avatar/N", m_nSender_player_avatar);
	pPutItem->AddValue("/Item/raw_lang/N", m_nRaw_lang);
	if (!m_sTranslate_content.empty())
	{
		pPutItem->AddValue("/Item/translate_content/S", JsonEncode(m_sTranslate_content, sJsonEncode));
	}
	pPutItem->AddValue("/Item/seq/N", m_nSeq);
	pPutItem->AddValue("/Item/jmp_type/N", m_nJmp_type);
	pPutItem->AddValue("/Item/event_id/N", m_nEvent_id);
	pPutItem->AddValue("/Item/event_type/N", m_nEvent_type);
	pPutItem->AddValue("/Item/event_score/N", m_nEvent_score);
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

int TbMail::OnPutItemReq(string& sPostData,
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

int TbMail::OnPutItemRsp(const Json::Value& item)
{
	return OnResponse(item);
}

int TbMail::OnResponse(const Json::Value& item)
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
		if (vecMembers[i] == "time")
		{
			m_nTime = strtoll(item["time"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "suid")
		{
			m_nSuid = strtoll(item["suid"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "sender")
		{
			m_sSender = item["sender"]["S"].asString();
			continue;
		}
		if (vecMembers[i] == "scid")
		{
			m_nScid = strtoll(item["scid"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "ruid")
		{
			m_nRuid = strtoll(item["ruid"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "send_type")
		{
			m_nSend_type = strtoll(item["send_type"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "title")
		{
			m_sTitle = item["title"]["S"].asString();
			continue;
		}
		if (vecMembers[i] == "content")
		{
			m_sContent = item["content"]["S"].asString();
			continue;
		}
		if (vecMembers[i] == "link")
		{
			m_nLink = strtoll(item["link"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "url")
		{
			m_sUrl = item["url"]["S"].asString();
			continue;
		}
		if (vecMembers[i] == "maildocid")
		{
			m_nMaildocid = strtoll(item["maildocid"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "display_type")
		{
			m_nDisplay_type = strtoll(item["display_type"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "escs")
		{
			m_sEscs = item["escs"]["S"].asString();
			continue;
		}
		if (vecMembers[i] == "sender_al_nick")
		{
			m_sSender_al_nick = item["sender_al_nick"]["S"].asString();
			continue;
		}
		if (vecMembers[i] == "sender_al_info")
		{
			m_sSender_al_info = item["sender_al_info"]["S"].asString();
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
		if (vecMembers[i] == "sender_player_avatar")
		{
			m_nSender_player_avatar = strtoll(item["sender_player_avatar"]["N"].asString().c_str(), NULL, 10);
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
		if (vecMembers[i] == "jmp_type")
		{
			m_nJmp_type = strtoll(item["jmp_type"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "event_id")
		{
			m_nEvent_id = strtoll(item["event_id"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "event_type")
		{
			m_nEvent_type = strtoll(item["event_type"]["N"].asString().c_str(), NULL, 10);
			continue;
		}
		if (vecMembers[i] == "event_score")
		{
			m_nEvent_score = strtoll(item["event_score"]["N"].asString().c_str(), NULL, 10);
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

TINT64 TbMail::GetSeq()
{
	return m_nSeq;
}

