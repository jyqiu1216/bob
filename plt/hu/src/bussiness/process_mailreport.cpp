#include "process_mailreport.h"
#include "statistic.h"
#include "game_info.h"
#include "procedure_base.h"
#include "global_serv.h"
#include "common_base.h"
#include "common_func.h"
#include "process_alliance.h"
#include "msg_base.h"
#include "document.h"
#include "globalres_logic.h"
#include "activities_logic.h"
#include "sendmessage_base.h"
#include "quest_logic.h"
#include "conf_base.h"
#include <iomanip>
#include "common_json.h"
#include <stdio.h> 
#include "db_request.h"
#include "city_base.h"
#include "player_base.h"
#include "tool_base.h"
#include "report_svr_request.h"
#include "pushdata_action.h"

#define SUPPORT_SQL_FNAME "update.sql"

TINT32 CProcessMailReport::ProcessCmd_MailSend(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    TINT64 ddwMailId = 0;
//    TCHAR *pszCustomerService = "customerservice";

    TUINT8 ucSendType = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TCHAR *pszReceiverName = pstSession->m_stReqParam.m_szKey[1];
    TCHAR *pszTitle = pstSession->m_stReqParam.m_szKey[2];
    TCHAR *pszContent = pstSession->m_stReqParam.m_szKey[3];
    SCityInfo* pstCity = &pstSession->m_stUserInfo.m_stCityInfo;
    TbPlayer *ptbPlayer = &pstSession->m_stUserInfo.m_tbPlayer;

    // 1. init
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        // check param
        if(pszTitle[0] == 0 && pszContent[0] == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__MailSend: mail send title and content all empty [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }

        if(ucSendType == EN_MAIL_SEND_TYPE_PLAYER_TO_ALLIANCE)
        {
            if(pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos == 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NOT_IN_ALLIANCE;
                return -2;
            }
        }

        std::vector<string> vecNames;
        string strNames;
        if (ucSendType == EN_MAIL_SEND_TYPE_PLAYER_TO_SUPPORT)
        {
            if (pszContent[0] == 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__MailSend: help mail send content empty [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                return -12;
            }
        }
        else
        {
            if (ucSendType == EN_MAIL_SEND_TYPE_PLAYER_TO_PLAYERS)
            {
                strNames = CToolBase::ToLower(pszReceiverName);
            }
            else
            {
                strNames = pszReceiverName;
            }
            CCommonFunc::GetVectorFromString(strNames.c_str(), ':', vecNames);

            if (vecNames.empty())
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__MailSend: no receiver [seq=%u]", pstSession->m_udwSeqNo));
                return -2;
            }

            if (vecNames.size() > DEFAULT_PERPAGE_NUM)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__MailSend: too many receiver [seq=%u]", pstSession->m_udwSeqNo));
                return -3;
            }

            if (vecNames.size() > 1 && ucSendType == EN_MAIL_SEND_TYPE_PLAYER_TO_ALLIANCE)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__MailSend: too many receiver [seq=%u]", pstSession->m_udwSeqNo));
                return -4;
            }
        }

        pstSession->ResetAwsInfo();
        if(ucSendType == EN_MAIL_SEND_TYPE_PLAYER_TO_PLAYERS)
        {
            for(TUINT32 udwIdx = 0; udwIdx < vecNames.size() && udwIdx < MAX_MAIL_PERPAGE_NUM; udwIdx++)
            {
                CAwsRequest::UserGetByPlayerName(pstSession, vecNames[udwIdx]);
            }
        }
        else if(ucSendType == EN_MAIL_SEND_TYPE_PLAYER_TO_ALLIANCE)
        {
            CAwsRequest::AllianceGetByName(pstSession, vecNames.at(0));
        }
        else if (ucSendType != EN_MAIL_SEND_TYPE_PLAYER_TO_SUPPORT)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -5;
        }


        if (ucSendType != EN_MAIL_SEND_TYPE_PLAYER_TO_SUPPORT)
        {
            // set next procedure
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

            // send request
            bNeedResponse = TRUE;
            dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
            if(dwRetCode < 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__MailSend: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                return -6;
            }
            return 0;
        }
        else
        {
            // set next procedure
            pstSession->m_udwMailSendStep = 0;
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        }
    }

    // 3. 解析响应
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        pstSession->m_udwMailRUidNum = 0;
        if(ucSendType == EN_MAIL_SEND_TYPE_PLAYER_TO_PLAYERS)
        {
            TbUnique_name tbPlayerName;
            for(TUINT32 idx = 0; idx < pstSession->m_vecAwsRsp.size(); idx++)
            {
                dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[idx], &tbPlayerName);
                if(dwRetCode > 0)
                {
                    pstSession->m_atbMailReceiver[pstSession->m_udwMailRUidNum].Reset();
                    pstSession->m_atbMailReceiver[pstSession->m_udwMailRUidNum] = tbPlayerName;
                    pstSession->m_audwMailRUidList[pstSession->m_udwMailRUidNum++] = tbPlayerName.m_nId;
                }
            }
            if(pstSession->m_udwMailRUidNum == 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__MAIL_RECEIVER_INVALID;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MailSend: no such user[%s] [seq=%u]", \
                    pszReceiverName, pstSession->m_stUserInfo.m_udwBSeqNo));
                return -7;
            }
            pstSession->m_dwMailReceiverId = pstSession->m_audwMailRUidList[0];
        }
        else if(ucSendType == EN_MAIL_SEND_TYPE_PLAYER_TO_ALLIANCE)
        {
            TbUnique_name tbAllianceName;
            dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[0], &tbAllianceName);
            if(dwRetCode > 0 && tbAllianceName.m_nId > 0)
            {
                pstSession->m_dwMailReceiverId = -1 * (tbAllianceName.m_nId);//联盟id
            }
            else
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__MAIL_RECEIVER_INVALID;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MailSend: no such user[%s] [seq=%u]", \
                    pszReceiverName, pstSession->m_stUserInfo.m_udwBSeqNo));
                return -8;
            }
        }

        // set next procedure
        pstSession->m_udwMailSendStep = 0;
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
    }

    // 4. 获取mailid——发送请求 // 获取对方黑名单...nemo add v1.3
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        // set package
        pstSession->ResetAwsInfo();
        CAwsRequest::GetGlobalNewId(pstSession, EN_GLOBAL_PARAM__MAIL_ID);

        if (ucSendType == EN_MAIL_SEND_TYPE_PLAYER_TO_PLAYERS)
        {
            CAwsRequest::BlackListGet(pstSession, pstSession->m_audwMailRUidList[pstSession->m_udwMailSendStep], ptbPlayer->m_nUid);
        }

        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MailSend: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -9;
        }
        return 0;
    }

    // 5. 获取mailid——解析结果
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__4;

        // parse data
        TbParam tbParamRes;
        for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecAwsRsp.size(); ++udwIdx)
        {
            AwsRspInfo *pstAwsRspInfo = pstSession->m_vecAwsRsp[udwIdx];
            string strTableRawName = CCommonFunc::GetTableRawName(pstAwsRspInfo->sTableName);
            if (strTableRawName == EN_AWS_TABLE_PARAM)
            {
                dwRetCode = CAwsResponse::OnUpdateItemRsp(*pstAwsRspInfo, &tbParamRes);
                continue;
            }
            if (strTableRawName == EN_AWS_TABLE_BLACKLIST)
            {
                dwRetCode = CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &pstSession->m_tbTmpBlackList);
                continue;
            }
        }
        if (tbParamRes.m_nVal == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_PACKAGE_ERR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("get mail id failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -10;
        }
        ddwMailId = tbParamRes.m_nVal;
        pstSession->m_ddwNewMailId = ddwMailId;
    }

    if(EN_COMMAND_STEP__4 == pstSession->m_udwCommandStep)
    {  
        TINT32 dwRetCode = 0;
        
        // reset req
        pstSession->ResetTranslateReq();
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__5;

        if(ucSendType == EN_MAIL_SEND_TYPE_PLAYER_TO_PLAYERS
           || ucSendType == EN_MAIL_SEND_TYPE_PLAYER_TO_ALLIANCE)
        {
            if(0 != CDocument::GetInstance()->GetSupportLangNum())
            {        
                pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__TRANSLATE;
                // send request
                bNeedResponse = TRUE;
            	Json::FastWriter oJsonWriter;
            	oJsonWriter.omitEndingLineFeed();
            	Json::Value rTranslateJson = Json::Value(Json::objectValue);
                for(TINT32 dwIdx = 0; dwIdx < CDocument::GetInstance()->GetSupportLangNum(); ++dwIdx)
                {
                    rTranslateJson.clear();
                    TranslateReqInfo *pstTranslateReq = new TranslateReqInfo;
                    rTranslateJson["0"]["from"] = "";
                    rTranslateJson["0"]["to"] = CDocument::GetInstance()->GetShortLangName(dwIdx);
                	rTranslateJson["0"]["content"] = pstSession->m_stReqParam.m_szKey[3];
                    pstTranslateReq->SetVal("mail", "translate", oJsonWriter.write(rTranslateJson));       
                    pstSession->m_vecTranslateReq.push_back(pstTranslateReq);

                    TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_MailSend: [TranslateType=%s] [TranslateOperate=%s] [TranslateReqContent=%s] [seq:%u]", \
                                                            pstTranslateReq->m_strTranslateType.c_str(), \
                                                            pstTranslateReq->m_strTranslateOperate.c_str(), \
                                                            pstTranslateReq->m_strTranslateContent.c_str(), \
                                                            pstSession->m_stUserInfo.m_udwBSeqNo));
                }
                
                dwRetCode = CBaseProcedure::SendTranslateRequest(pstSession, EN_SERVICE_TYPE_TRANSLATE_REQ);
                if(dwRetCode != 0)
                {        
                    bNeedResponse = FALSE;
                    pstSession->m_udwCommandStep = EN_COMMAND_STEP__5;
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MailSend: send translate req fail [ret=%d] [seq=%u]", \
                                                            dwRetCode, \
                                                            pstSession->m_stUserInfo.m_udwBSeqNo));
                }
                else
                {
                    return 0;
                }             
            }
        }
    }

    // 6. 邮件发送, 并更新统计信息
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__5)
    {
        TBOOL bTranslateFalg = FALSE;
        if((TUINT32)CDocument::GetInstance()->GetSupportLangNum() == pstSession->m_vecTranslateRsp.size()
           && 0 != pstSession->m_vecTranslateRsp.size())
        {
            bTranslateFalg = TRUE;
        }
        for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecTranslateRsp.size(); ++udwIdx)
        {
            if(0 != pstSession->m_vecTranslateRsp[udwIdx]->m_dwRetCode)
            {
                bTranslateFalg = FALSE;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MailSend: parse translate info fail [seq=%u]", \
                                                         pstSession->m_stUserInfo.m_udwBSeqNo));
            }
            else
            {
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_MailSend: [TranslateType=%s] [TranslateOperate=%s] [TranslateContent=%s] [TranslateResult=%s] [seq=%u]", \
                                                       pstSession->m_vecTranslateRsp[udwIdx]->m_strTranslateType.c_str(), \
                                                       pstSession->m_vecTranslateRsp[udwIdx]->m_strTranslateOperate.c_str(), \
                                                       pstSession->m_vecTranslateRsp[udwIdx]->m_strTranslateContent.c_str(), \
                                                       pstSession->m_vecTranslateRsp[udwIdx]->m_strTranslateResult.c_str(), \
                                                       pstSession->m_stUserInfo.m_udwBSeqNo));
            }
        }
        
        
        TINT32 dwLanguageId = -1;
        string strTranslateContent = "";
        Json::FastWriter oJsonWriter;
        oJsonWriter.omitEndingLineFeed();
        Json::Value rTranslateJson = Json::Value(Json::objectValue);
        if(TRUE == bTranslateFalg)
        {
            for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecTranslateRsp.size(); ++udwIdx)
            {
                Json::Reader jsonReader;
                Json::Value jResultBody;
                if(jsonReader.parse(pstSession->m_vecTranslateRsp[udwIdx]->m_strTranslateResult.c_str(), jResultBody))
                { 
                    if(0 == udwIdx)
                    {
                        dwLanguageId = atoi(CDocument::GetInstance()->GetLanguageId(jResultBody["0"]["from"].asString().c_str(), pstSession->m_stReqParam.m_udwLang).c_str());
                    }
                    Json::Value::Members vecMembers = jResultBody.getMemberNames();
                    for(TUINT32 udwIdy = 0; udwIdy < vecMembers.size(); ++udwIdy)
                    {
                        rTranslateJson[CDocument::GetInstance()->GetLangId(jResultBody[vecMembers[0]]["to"].asString()).c_str()][vecMembers[udwIdy]] = jResultBody[vecMembers[udwIdy]]["content"];
                    }
                }
            }
            strTranslateContent = oJsonWriter.write(rTranslateJson);
        }

        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_MailSend: [dwLanguageId=%d] [strTranslateContent=%s] [seq=%u]", \
                                                dwLanguageId, \
                                                strTranslateContent.c_str(), \
                                                pstSession->m_stUserInfo.m_udwBSeqNo));

        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_MailSend: [bTranslateFalg=%d] [mail_id=%d] [seq=%u]", \
                                                bTranslateFalg, \
                                                pstSession->m_ddwNewMailId, \
                                                pstSession->m_stUserInfo.m_udwBSeqNo));

        pstSession->ResetAwsInfo();
        // mail class
        TbMail tbMail;

        tbMail.Set_Id(pstSession->m_ddwNewMailId);
        tbMail.Set_Time(CTimeUtils::GetUnixTime());
        tbMail.Set_Suid(pstSession->m_stReqParam.m_udwUserId);
        tbMail.Set_Sender(pstSession->m_stUserInfo.m_tbPlayer.m_sUin);
        tbMail.Set_Scid(pstSession->m_stReqParam.m_udwCityId);
        tbMail.Set_Send_type(ucSendType);
        tbMail.Set_Title(pszTitle);
        tbMail.Set_Content(pszContent);
        tbMail.Set_Sender_player_avatar(pstSession->m_stUserInfo.m_tbPlayer.m_nAvatar);
        tbMail.Set_Sender_al_nick(pstSession->m_stUserInfo.m_tbAlliance.m_sAl_nick_name);
        tbMail.Set_Raw_lang(dwLanguageId);
        tbMail.Set_Translate_content(strTranslateContent);
        if(ucSendType == EN_MAIL_SEND_TYPE_PLAYER_TO_PLAYERS)
        {
            tbMail.Set_Display_type(EN_MAIL_DISPLAY_TYPE_NORMAL);
            tbMail.Set_Ruid(pstSession->m_audwMailRUidList[pstSession->m_udwMailSendStep]);
        }
        else if(ucSendType == EN_MAIL_SEND_TYPE_PLAYER_TO_ALLIANCE)
        {
            tbMail.Set_Display_type(EN_MAIL_DISPLAY_TYPE_ALLIANCE);
            tbMail.Set_Ruid(pstSession->m_dwMailReceiverId);
        }
        else if (ucSendType == EN_MAIL_SEND_TYPE_PLAYER_TO_SUPPORT)
        {
            tbMail.Set_Display_type(EN_MAIL_DISPLAY_TYPE_SUPPORT_HELP);
        }

        // set data package - mail
        pstSession->m_stUserInfo.m_tbUserStat.Set_Newest_mailid(pstSession->m_ddwNewMailId);
        CAwsRequest::UpdateItem(pstSession, &tbMail);

        TbMail_user astMailUserItem[DEFAULT_PERPAGE_NUM * 2];
        TUINT32 udwMailUserNum = 0;
        if(ucSendType == EN_MAIL_SEND_TYPE_PLAYER_TO_ALLIANCE)
        {
            if(pstSession->m_dwMailReceiverId == -1 * pstSession->m_stUserInfo.m_tbAlliance.m_nAid)
            {
                pstSession->m_stUserInfo.m_tbUserStat.Set_Unread_mail(pstSession->m_stUserInfo.m_tbUserStat.m_nUnread_mail + 1);
            }

            astMailUserItem[udwMailUserNum].Reset();
            astMailUserItem[udwMailUserNum].Set_Mid(pstSession->m_ddwNewMailId);
            astMailUserItem[udwMailUserNum].Set_Suid(pstSession->m_stUserInfo.m_tbPlayer.m_nUid);
            astMailUserItem[udwMailUserNum].Set_Time(tbMail.m_nTime);
            astMailUserItem[udwMailUserNum].Set_Uid(pstSession->m_dwMailReceiverId);
            astMailUserItem[udwMailUserNum].Set_Tuid(pstSession->m_dwMailReceiverId);
            astMailUserItem[udwMailUserNum].Set_Display_type(EN_MAIL_DISPLAY_TYPE_ALLIANCE);

            //CAwsRequest::UpdateItem(pstSession, &stMailUserItem);
            udwMailUserNum++;

            //wave@push_data
            CPushDataBasic::PushDataAid_Refresh(pstSession, pstSession->m_dwMailReceiverId);
        }
        else if (ucSendType == EN_MAIL_SEND_TYPE_PLAYER_TO_PLAYERS)
        {
            if(pstSession->m_udwMailSendStep < pstSession->m_udwMailRUidNum)
            {
                if(pstSession->m_stUserInfo.m_tbPlayer.m_nUid != pstSession->m_audwMailRUidList[pstSession->m_udwMailSendStep])
                {
                    astMailUserItem[udwMailUserNum].Reset();
                    astMailUserItem[udwMailUserNum].Set_Mid(pstSession->m_ddwNewMailId);
                    astMailUserItem[udwMailUserNum].Set_Suid(pstSession->m_stUserInfo.m_tbPlayer.m_nUid);
                    astMailUserItem[udwMailUserNum].Set_Time(tbMail.m_nTime);
                    astMailUserItem[udwMailUserNum].Set_Uid(pstSession->m_stUserInfo.m_tbPlayer.m_nUid);
                    astMailUserItem[udwMailUserNum].Set_Tuid(pstSession->m_audwMailRUidList[pstSession->m_udwMailSendStep]);
                    astMailUserItem[udwMailUserNum].Set_Display_type(EN_MAIL_DISPLAY_TYPE_NORMAL);
                    astMailUserItem[udwMailUserNum].Set_Status(EN_MAIL_STATUS_READ);
                    if(!pstSession->m_atbMailReceiver[pstSession->m_udwMailSendStep].m_jProfile.empty())
                    {
                        astMailUserItem[udwMailUserNum].Set_Receiver_avatar(pstSession->m_atbMailReceiver[pstSession->m_udwMailSendStep].m_jProfile["avatar"].asInt());
                        astMailUserItem[udwMailUserNum].Set_Receiver_name(pstSession->m_atbMailReceiver[pstSession->m_udwMailSendStep].m_jProfile["uname"].asString());
                        astMailUserItem[udwMailUserNum].Set_Receiver_alnick(pstSession->m_atbMailReceiver[pstSession->m_udwMailSendStep].m_jProfile["al_nick_name"].asString());
                    }

                    //CAwsRequest::UpdateItem(pstSession, &stMailUserItem);
                    udwMailUserNum++;
                }

                if (pstSession->m_stUserInfo.m_tbPlayer.m_nUid == pstSession->m_audwMailRUidList[pstSession->m_udwMailSendStep]
                    || pstSession->m_tbTmpBlackList.m_nTarget_uid != ptbPlayer->m_nUid)
                {
                    astMailUserItem[udwMailUserNum].Reset();
                    astMailUserItem[udwMailUserNum].Set_Mid(pstSession->m_ddwNewMailId);
                    astMailUserItem[udwMailUserNum].Set_Suid(pstSession->m_stUserInfo.m_tbPlayer.m_nUid);
                    astMailUserItem[udwMailUserNum].Set_Time(tbMail.m_nTime);
                    astMailUserItem[udwMailUserNum].Set_Uid(pstSession->m_audwMailRUidList[pstSession->m_udwMailSendStep]);
                    astMailUserItem[udwMailUserNum].Set_Tuid(pstSession->m_audwMailRUidList[pstSession->m_udwMailSendStep]);
                    astMailUserItem[udwMailUserNum].Set_Display_type(EN_MAIL_DISPLAY_TYPE_NORMAL);
                    if (!pstSession->m_atbMailReceiver[pstSession->m_udwMailSendStep].m_jProfile.empty())
                    {
                        astMailUserItem[udwMailUserNum].Set_Receiver_avatar(pstSession->m_atbMailReceiver[pstSession->m_udwMailSendStep].m_jProfile["avatar"].asInt());
                        astMailUserItem[udwMailUserNum].Set_Receiver_name(pstSession->m_atbMailReceiver[pstSession->m_udwMailSendStep].m_jProfile["uname"].asString());
                        astMailUserItem[udwMailUserNum].Set_Receiver_alnick(pstSession->m_atbMailReceiver[pstSession->m_udwMailSendStep].m_jProfile["al_nick_name"].asString());
                    };

                    //CAwsRequest::UpdateItem(pstSession, &stMailUserItem);
                    udwMailUserNum++;

                    //wave@push_data
                    CPushDataBasic::PushDataUid_Refresh(pstSession, pstSession->m_audwMailRUidList[pstSession->m_udwMailSendStep]);
                }

                SNoticInfo stNoticInfo;
                stNoticInfo.Reset();
                stNoticInfo.SetValue(EN_NOTI_ID__MAIL,
                    pstSession->m_stUserInfo.m_tbPlayer.m_sUin, "",
                    0, 0,
                    0, 0,
                    0, "", 0);
                CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), pstSession->m_stReqParam.m_udwSvrId, pstSession->m_audwMailRUidList[pstSession->m_udwMailSendStep], stNoticInfo);

                pstSession->m_udwMailSendStep++;
            }
        }
        else if (ucSendType == EN_MAIL_SEND_TYPE_PLAYER_TO_SUPPORT)
        {
            astMailUserItem[udwMailUserNum].Reset();
            astMailUserItem[udwMailUserNum].Set_Mid(pstSession->m_ddwNewMailId);
            astMailUserItem[udwMailUserNum].Set_Suid(pstSession->m_stUserInfo.m_tbPlayer.m_nUid);
            astMailUserItem[udwMailUserNum].Set_Time(tbMail.m_nTime);
            astMailUserItem[udwMailUserNum].Set_Uid(pstSession->m_stUserInfo.m_tbPlayer.m_nUid);
            astMailUserItem[udwMailUserNum].Set_Tuid(SYSTEM_SUPPORT);  //TODO: 定下客服的代表id
            astMailUserItem[udwMailUserNum].Set_Display_type(EN_MAIL_DISPLAY_TYPE_SUPPORT_HELP);
            astMailUserItem[udwMailUserNum].Set_Status(EN_MAIL_STATUS_READ);

            //CAwsRequest::UpdateItem(pstSession, &stMailUserItem);
            udwMailUserNum++;
        }
        
        pstSession->ResetReportInfo();
        CReportSvrRequest::MailUserPut(pstSession, astMailUserItem, udwMailUserNum);

        // send request
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__6;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MailSend: insert mail or update stat failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -11;
        }
        return 0;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__6)
    {
        if (pstSession->m_udwMailSendStep == pstSession->m_udwMailRUidNum)
        {
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__7;
        }
        else
        {
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        }

        // send request
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__REPORT_SVR;

        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendReportSvrRequest(pstSession);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MailSend: insert mail or update stat failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -11;
        }
        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__7)
    {
        //task count
        if(ucSendType == EN_MAIL_SEND_TYPE_PLAYER_TO_ALLIANCE)
        {
            CQuestLogic::SetTaskCurValue(&pstSession->m_stUserInfo,pstCity, EN_TASK_TYPE_SEND_ALLIANCE_MAIL);
        }
        else if (ucSendType == EN_MAIL_SEND_TYPE_PLAYER_TO_PLAYERS)
        {
            CQuestLogic::SetTaskCurValue(&pstSession->m_stUserInfo,pstCity, EN_TASK_TYPE_SEND_MAIL);
        }

        if (ucSendType == EN_MAIL_SEND_TYPE_PLAYER_TO_PLAYERS)
        {
            Json::Value jTmp = Json::Value(Json::arrayValue);
            Json::FastWriter writer;
            for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_udwMailRUidNum; udwIdx++)
            {
                if (pstSession->m_stUserInfo.m_tbPlayer.m_nUid != pstSession->m_audwMailRUidList[udwIdx])
                {
                    if (pstSession->m_atbMailReceiver[udwIdx].m_jProfile["new_al_status"].asInt() == EN_NEW_PLAYER_AL_STATUS__JOIN
                        && pstSession->m_atbMailReceiver[udwIdx].m_jProfile["alpos"].asInt() == EN_ALLIANCE_POS__CHANCELLOR)
                    {
                        jTmp.append(pstSession->m_atbMailReceiver[udwIdx].m_jProfile["uname"].asString());
                        jTmp.append(pstSession->m_atbMailReceiver[udwIdx].m_jProfile["avatar"].asInt());
                        jTmp.append(pstSession->m_atbMailReceiver[udwIdx].m_jProfile["al_nick_name"].asString());
                        CMsgBase::SendOperateMail(pstSession->m_stUserInfo.m_tbPlayer.m_nUid, EN_MAIL_ID__AUTO_TELL_NPC, pstSession->m_stUserInfo.m_tbPlayer.m_nSid,
                            pstSession->m_atbMailReceiver[udwIdx].m_nId, "", writer.write(jTmp).c_str(), "");
                    }
                }
            }
        }

        if (ucSendType == EN_MAIL_SEND_TYPE_PLAYER_TO_SUPPORT)
        {
            TINT64& ddwUid = pstSession->m_stUserInfo.m_tbPlayer.m_nUid;
            TINT64& ddwSid = pstSession->m_stUserInfo.m_tbPlayer.m_nSid;
            string& sUin = pstSession->m_stUserInfo.m_tbPlayer.m_sUin;            

            
            TCHAR szEncodeBuf[10240];
            CHttpUtils::url_encode(pszContent, szEncodeBuf, 10240);
            string strContent = szEncodeBuf;
            TINT64 &ddwMgiht = pstSession->m_stUserInfo.m_tbPlayer.m_nMight;

            string strDBName = CConfBase::GetString("support_dbname");
            string strTBName = CConfBase::GetString("support_info_tbname");
            string strNewTBName = CConfBase::GetString("support_info_new_tbname");
            string strTimeTBName = CConfBase::GetString("support_time_tbname");

            TUINT32 udwCurTime = CTimeUtils::GetUnixTime();
            TUINT64 uddwId = (ddwUid << 32) + udwCurTime;
            string sFileName = SUPPORT_SQL_FNAME;
            
            stringstream os_help_sql;
            os_help_sql.str("");

            //[id, uid, uin, sid, status, time, get_time, tag]

            os_help_sql << "use " << strDBName << ";\n";
            
            /*
            os_help_sql << "create table if not exists " << strTBName << "(";
            os_help_sql << "id bigint unsigned not null, uid int unsigned not null, uin varchar(20) not null, sid int unsigned not null, ";
            os_help_sql << "status tinyint unsigned not null, time int unsigned not null, get_time int unsigned not null, ";
            os_help_sql << "tag int unsigned not null, recept_id int unsigned not null, primary key(id)";
            os_help_sql << ") default charset = utf8;\n";
            os_help_sql << "insert into " << strTBName << " values (";
            os_help_sql << uddwId << ", " << ddwUid << ", \"" << sUin << "\", " << ddwSid << ", 0, " << udwCurTime << ", 0, 0, 0);\n";
            */

            os_help_sql << "insert into " << strNewTBName << " (uid,uin,sid,might,time,fromid,toid,content,docid,mailid,reply_recept_id,deadflag,status,recept_id,get_time,tag) values (";
            os_help_sql << ddwUid << ", ";
            os_help_sql << "\"" << sUin << "\", ";
            os_help_sql << ddwSid << ", ";
            os_help_sql << ddwMgiht << ", ";
            os_help_sql << udwCurTime << ", ";
            os_help_sql << ddwUid << ", ";
            os_help_sql << SYSTEM_SUPPORT << ", ";
            os_help_sql << "\"" << strContent << "\", ";
            os_help_sql << "0, ";  
            os_help_sql << pstSession->m_ddwNewMailId << ", ";
            os_help_sql << "0, ";
            os_help_sql << "0, ";
            os_help_sql << "0, ";
            os_help_sql << "0, ";
            os_help_sql << "0, ";
            os_help_sql << "0);\n";


            os_help_sql << "create table if not exists " << strTimeTBName << "(";
            os_help_sql << "uid int unsigned not null, last_mail_time int unsigned not null, last_reply_time int unsigned not null, primary key(uid)";
            os_help_sql << ") default charset = utf8;\n";
            os_help_sql << "insert into " << strTimeTBName << " values (";
            os_help_sql << ddwUid << ", " << udwCurTime << ", 0) ON DUPLICATE KEY UPDATE uid=values(uid), last_mail_time=values(last_mail_time);";

            //TSE_LOG_DEBUG(pstSession->m_poServLog, ("MailHelpSend: got sql as => \n%s", os_help_sql.str().c_str()));

            ofstream file_sql(sFileName.c_str());
            file_sql << os_help_sql.str().c_str();
            file_sql.close();

            TCHAR szExecCmd[100];
            sprintf(szExecCmd, "./update_support_table.sh %s", sFileName.c_str());
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("MailHelpSend: got sql as => %s", szExecCmd));
            CMsgBase::SendDelaySystemMsg(szExecCmd);
        }
        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    // set next procedure
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessMailReport::ProcessCmd_MailSendById(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    TINT64 ddwMailId = 0;

    TUINT8 ucSendType = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TINT32 dwTargetId = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TCHAR *pszTitle = pstSession->m_stReqParam.m_szKey[2];
    TCHAR *pszContent = pstSession->m_stReqParam.m_szKey[3];
    SCityInfo* pstCity = &pstSession->m_stUserInfo.m_stCityInfo;
    TbPlayer *ptbPlayer = &pstSession->m_stUserInfo.m_tbPlayer;

    // 1. init
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        // check param
        if(pszTitle[0] == 0 && pszContent[0] == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MailSendByUid: mail send title and content all empty [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }

        if(dwTargetId <= 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -2;
        }

        if(ucSendType == EN_MAIL_SEND_TYPE_PLAYER_TO_ALLIANCE)
        {
            if(pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos == 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NOT_IN_ALLIANCE;
                return -3;
            }
        }

        pstSession->ResetAwsInfo();
        if(ucSendType == EN_MAIL_SEND_TYPE_PLAYER_TO_PLAYERS)
        {
            CAwsRequest::UserGetByUid(pstSession, dwTargetId);
        }
        else if(ucSendType == EN_MAIL_SEND_TYPE_PLAYER_TO_ALLIANCE)
        {
            CAwsRequest::AllianceGetByAid(pstSession, dwTargetId);
        }
        else
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -4;
        }

        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MailSendByUid: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -5;
        }
        return 0;
    }

    // 3. 解析响应
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        pstSession->m_udwMailRUidNum = 0;
        if(ucSendType == EN_MAIL_SEND_TYPE_PLAYER_TO_PLAYERS)
        {
            dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[0], &pstSession->m_tbTmpPlayer);
            if(dwRetCode > 0 && pstSession->m_tbTmpPlayer.m_nUid > 0)
            {
                pstSession->m_audwMailRUidList[pstSession->m_udwMailRUidNum++] = pstSession->m_tbTmpPlayer.m_nUid;
            }
            if(pstSession->m_udwMailRUidNum == 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__MAIL_RECEIVER_INVALID;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MailSendByUid: no such user[uid=%d] [seq=%u]", 
                    dwTargetId, pstSession->m_stUserInfo.m_udwBSeqNo));
                return -6;
            }
            pstSession->m_dwMailReceiverId = pstSession->m_audwMailRUidList[0];
        }
        else if(ucSendType == EN_MAIL_SEND_TYPE_PLAYER_TO_ALLIANCE)
        {
            dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[0], &pstSession->m_tbTmpAlliance);
            if(dwRetCode > 0 && pstSession->m_tbTmpAlliance.m_nAid > 0)
            {
                pstSession->m_dwMailReceiverId = -1 * (pstSession->m_tbTmpAlliance.m_nAid);//联盟id
            }
            else
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__MAIL_RECEIVER_INVALID;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MailSendByUid: no such alliance[aid=%d] [seq=%u]", 
                    dwTargetId, pstSession->m_stUserInfo.m_udwBSeqNo));
                return -7;
            }
        }

        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
    }

    // 4. 获取mailid——发送请求
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        // set package
        pstSession->ResetAwsInfo();
        CAwsRequest::GetGlobalNewId(pstSession, EN_GLOBAL_PARAM__MAIL_ID);
        CAwsRequest::BlackListGet(pstSession, dwTargetId, ptbPlayer->m_nUid);

        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MailSendByUid: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -8;
        }
        return 0;
    }

    // 5. 获取mailid——解析结果
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__4;

        // parse data

        TbParam tbParamRes;
        for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecAwsRsp.size(); ++udwIdx)
        {
            AwsRspInfo *pstAwsRspInfo = pstSession->m_vecAwsRsp[udwIdx];
            string strTableRawName = CCommonFunc::GetTableRawName(pstAwsRspInfo->sTableName);
            if (strTableRawName == EN_AWS_TABLE_PARAM)
            {
                dwRetCode = CAwsResponse::OnUpdateItemRsp(*pstAwsRspInfo, &tbParamRes);
                continue;
            }
            if (strTableRawName == EN_AWS_TABLE_BLACKLIST)
            {
                dwRetCode = CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &pstSession->m_tbTmpBlackList);
                continue;
            }
        }
        if(tbParamRes.m_nVal == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_PACKAGE_ERR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("get mail id failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -9;
        }
        ddwMailId = tbParamRes.m_nVal;
        pstSession->m_ddwNewMailId = ddwMailId;
    }



    if(EN_COMMAND_STEP__4 == pstSession->m_udwCommandStep)
    {  
        TINT32 dwRetCode = 0;
        
        // reset req
        pstSession->ResetTranslateReq();
        // next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__5;

        if(ucSendType == EN_MAIL_SEND_TYPE_PLAYER_TO_PLAYERS
           || ucSendType == EN_MAIL_SEND_TYPE_PLAYER_TO_ALLIANCE)
        {
            if(0 != CDocument::GetInstance()->GetSupportLangNum())
            {        
                pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__TRANSLATE;
                // send request
                bNeedResponse = TRUE;
            	Json::FastWriter oJsonWriter;
            	oJsonWriter.omitEndingLineFeed();
            	Json::Value rTranslateJson = Json::Value(Json::objectValue);
                for(TINT32 dwIdx = 0; dwIdx < CDocument::GetInstance()->GetSupportLangNum(); ++dwIdx)
                {
                    rTranslateJson.clear();
                    TranslateReqInfo *pstTranslateReq = new TranslateReqInfo;
                    rTranslateJson["0"]["from"] = "";
                    rTranslateJson["0"]["to"] = CDocument::GetInstance()->GetShortLangName(dwIdx);
                	rTranslateJson["0"]["content"] = pstSession->m_stReqParam.m_szKey[3];
                    pstTranslateReq->SetVal("mail", "translate", oJsonWriter.write(rTranslateJson));       
                    pstSession->m_vecTranslateReq.push_back(pstTranslateReq);

                    TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_MailSendByUid: [TranslateType=%s] [TranslateOperate=%s] [TranslateReqContent=%s] [seq:%u]", \
                                                            pstTranslateReq->m_strTranslateType.c_str(), \
                                                            pstTranslateReq->m_strTranslateOperate.c_str(), \
                                                            pstTranslateReq->m_strTranslateContent.c_str(), \
                                                            pstSession->m_stUserInfo.m_udwBSeqNo));
                }
                
                dwRetCode = CBaseProcedure::SendTranslateRequest(pstSession, EN_SERVICE_TYPE_TRANSLATE_REQ);
                if(dwRetCode != 0)
                {        
                    bNeedResponse = FALSE;
                    pstSession->m_udwCommandStep = EN_COMMAND_STEP__5;
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MailSendByUid: send translate req fail [ret=%d] [seq=%u]", \
                                                            dwRetCode, \
                                                            pstSession->m_stUserInfo.m_udwBSeqNo));
                }
                else
                {
                    return 0;
                }             
            }
        }
    }

    // 6. 邮件发送, 并更新统计信息
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__5)
    {
        TBOOL bTranslateFalg = FALSE;
        if((TUINT32)CDocument::GetInstance()->GetSupportLangNum() == pstSession->m_vecTranslateRsp.size()
           && 0 != pstSession->m_vecTranslateRsp.size())
        {
            bTranslateFalg = TRUE;
        }
        for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecTranslateRsp.size(); ++udwIdx)
        {
            if(0 != pstSession->m_vecTranslateRsp[udwIdx]->m_dwRetCode)
            {
                bTranslateFalg = FALSE;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MailSendByUid: parse translate info fail [seq=%u]", \
                                                         pstSession->m_stUserInfo.m_udwBSeqNo));
            }
            else
            {
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_MailSendByUid: [TranslateType=%s] [TranslateOperate=%s] [TranslateContent=%s] [TranslateResult=%s] [seq=%u]", \
                                                       pstSession->m_vecTranslateRsp[udwIdx]->m_strTranslateType.c_str(), \
                                                       pstSession->m_vecTranslateRsp[udwIdx]->m_strTranslateOperate.c_str(), \
                                                       pstSession->m_vecTranslateRsp[udwIdx]->m_strTranslateContent.c_str(), \
                                                       pstSession->m_vecTranslateRsp[udwIdx]->m_strTranslateResult.c_str(), \
                                                       pstSession->m_stUserInfo.m_udwBSeqNo));
            }
        }
        
        TINT32 dwLanguageId = -1;
        string strTranslateContent = "";
        Json::FastWriter oJsonWriter;
        oJsonWriter.omitEndingLineFeed();
        Json::Value rTranslateJson = Json::Value(Json::objectValue);
        if(TRUE == bTranslateFalg)
        {
            for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecTranslateRsp.size(); ++udwIdx)
            {
                Json::Reader jsonReader;
                Json::Value jResultBody;
                if(jsonReader.parse(pstSession->m_vecTranslateRsp[udwIdx]->m_strTranslateResult.c_str(), jResultBody))
                { 
                    if(0 == udwIdx)
                    {
                        dwLanguageId = atoi(CDocument::GetInstance()->GetLanguageId(jResultBody["0"]["from"].asString().c_str(), pstSession->m_stReqParam.m_udwLang).c_str());
                    }
                    Json::Value::Members vecMembers = jResultBody.getMemberNames();
                    for(TUINT32 udwIdy = 0; udwIdy < vecMembers.size(); ++udwIdy)
                    {
                        rTranslateJson[CDocument::GetInstance()->GetLangId(jResultBody[vecMembers[0]]["to"].asString()).c_str()][vecMembers[udwIdy]] = jResultBody[vecMembers[udwIdy]]["content"];
                    }
                }
            }
            strTranslateContent = oJsonWriter.write(rTranslateJson);
        }

        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_MailSendByUid: [dwLanguageId=%d] [strTranslateContent=%s] [seq=%u]", \
                                                dwLanguageId, \
                                                strTranslateContent.c_str(), \
                                                pstSession->m_stUserInfo.m_udwBSeqNo));

        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_MailSendByUid: [bTranslateFalg=%d] [mail_id=%d] [seq=%u]", \
                                                bTranslateFalg, \
                                                pstSession->m_ddwNewMailId, \
                                                pstSession->m_stUserInfo.m_udwBSeqNo));

        pstSession->ResetAwsInfo();
        // pstSession->m_ddwNewMailId = ddwMailId;
        // mail class
        TbMail tbMail;

        tbMail.Set_Id(pstSession->m_ddwNewMailId);
        tbMail.Set_Time(CTimeUtils::GetUnixTime());
        tbMail.Set_Suid(pstSession->m_stReqParam.m_udwUserId);
        tbMail.Set_Sender(pstSession->m_stUserInfo.m_tbPlayer.m_sUin);
        tbMail.Set_Scid(pstSession->m_stReqParam.m_udwCityId);
        tbMail.Set_Send_type(ucSendType);
        tbMail.Set_Title(pszTitle);
        tbMail.Set_Content(pszContent);
        tbMail.Set_Sender_player_avatar(pstSession->m_stUserInfo.m_tbPlayer.m_nAvatar);
        tbMail.Set_Sender_al_nick(pstSession->m_stUserInfo.m_tbAlliance.m_sAl_nick_name);
        tbMail.Set_Raw_lang(dwLanguageId);
        tbMail.Set_Translate_content(strTranslateContent);
        if(ucSendType == EN_MAIL_SEND_TYPE_PLAYER_TO_PLAYERS)
        {
            tbMail.Set_Display_type(EN_MAIL_DISPLAY_TYPE_NORMAL);
            tbMail.Set_Ruid(pstSession->m_dwMailReceiverId);
        }
        else if(ucSendType == EN_MAIL_SEND_TYPE_PLAYER_TO_ALLIANCE)
        {
            tbMail.Set_Display_type(EN_MAIL_DISPLAY_TYPE_ALLIANCE);
            tbMail.Set_Ruid(pstSession->m_dwMailReceiverId);
        }

        // set data package - mail
        pstSession->m_stUserInfo.m_tbUserStat.Set_Newest_mailid(pstSession->m_ddwNewMailId);
        CAwsRequest::UpdateItem(pstSession, &tbMail);

        TbMail_user astMailUserItem[DEFAULT_PERPAGE_NUM * 2];
        TUINT32 udwMailUserNum = 0;
        if(ucSendType == EN_MAIL_SEND_TYPE_PLAYER_TO_ALLIANCE)
        {
            if(pstSession->m_dwMailReceiverId == -1 * pstSession->m_stUserInfo.m_tbAlliance.m_nAid)
            {
                pstSession->m_stUserInfo.m_tbUserStat.Set_Unread_mail(pstSession->m_stUserInfo.m_tbUserStat.m_nUnread_mail + 1);
            }

            astMailUserItem[udwMailUserNum].Reset();
            astMailUserItem[udwMailUserNum].Set_Mid(pstSession->m_ddwNewMailId);
            astMailUserItem[udwMailUserNum].Set_Suid(pstSession->m_stUserInfo.m_tbPlayer.m_nUid);
            astMailUserItem[udwMailUserNum].Set_Time(tbMail.m_nTime);
            astMailUserItem[udwMailUserNum].Set_Uid(pstSession->m_dwMailReceiverId);
            astMailUserItem[udwMailUserNum].Set_Tuid(pstSession->m_dwMailReceiverId);
            astMailUserItem[udwMailUserNum].Set_Display_type(EN_MAIL_DISPLAY_TYPE_ALLIANCE);

            //CAwsRequest::UpdateItem(pstSession, &stMailUserItem);
            udwMailUserNum++;
        }
        else
        {
            if (pstSession->m_stUserInfo.m_tbPlayer.m_nUid != pstSession->m_dwMailReceiverId)
            {
                astMailUserItem[udwMailUserNum].Reset();
                astMailUserItem[udwMailUserNum].Set_Mid(pstSession->m_ddwNewMailId);
                astMailUserItem[udwMailUserNum].Set_Suid(pstSession->m_stUserInfo.m_tbPlayer.m_nUid);
                astMailUserItem[udwMailUserNum].Set_Time(tbMail.m_nTime);
                astMailUserItem[udwMailUserNum].Set_Uid(pstSession->m_stUserInfo.m_tbPlayer.m_nUid);
                astMailUserItem[udwMailUserNum].Set_Tuid(pstSession->m_dwMailReceiverId);
                astMailUserItem[udwMailUserNum].Set_Display_type(EN_MAIL_DISPLAY_TYPE_NORMAL);
                astMailUserItem[udwMailUserNum].Set_Status(EN_MAIL_STATUS_READ);

                astMailUserItem[udwMailUserNum].Set_Receiver_avatar(pstSession->m_tbTmpPlayer.m_nAvatar);
                astMailUserItem[udwMailUserNum].Set_Receiver_name(pstSession->m_tbTmpPlayer.m_sUin);
                astMailUserItem[udwMailUserNum].Set_Receiver_alnick(pstSession->m_tbTmpPlayer.m_sAl_nick_name);

                //CAwsRequest::UpdateItem(pstSession, &stMailUserItem);
                udwMailUserNum++;
            }

            if (pstSession->m_tbTmpBlackList.m_nTarget_uid != ptbPlayer->m_nUid)
            {
                astMailUserItem[udwMailUserNum].Reset();
                astMailUserItem[udwMailUserNum].Set_Mid(pstSession->m_ddwNewMailId);
                astMailUserItem[udwMailUserNum].Set_Suid(pstSession->m_stUserInfo.m_tbPlayer.m_nUid);
                astMailUserItem[udwMailUserNum].Set_Time(tbMail.m_nTime);
                astMailUserItem[udwMailUserNum].Set_Uid(pstSession->m_dwMailReceiverId);
                astMailUserItem[udwMailUserNum].Set_Tuid(pstSession->m_dwMailReceiverId);
                astMailUserItem[udwMailUserNum].Set_Display_type(EN_MAIL_DISPLAY_TYPE_NORMAL);

                astMailUserItem[udwMailUserNum].Set_Receiver_avatar(pstSession->m_tbTmpPlayer.m_nAvatar);
                astMailUserItem[udwMailUserNum].Set_Receiver_name(pstSession->m_tbTmpPlayer.m_sUin);
                astMailUserItem[udwMailUserNum].Set_Receiver_alnick(pstSession->m_tbTmpPlayer.m_sAl_nick_name);

                //CAwsRequest::UpdateItem(pstSession, &stMailUserItem);
                udwMailUserNum++;

                //wave@push_data
                CPushDataBasic::PushDataUid_Refresh(pstSession, pstSession->m_dwMailReceiverId);
            }

            if(udwMailUserNum == 0)
            {
                pstSession->m_dwMailReceiverId = 0;
            }
            else
            {
                SNoticInfo stNoticInfo;
                stNoticInfo.Reset();
                stNoticInfo.SetValue(EN_NOTI_ID__MAIL,
                    pstSession->m_stUserInfo.m_tbPlayer.m_sUin, "",
                    0, 0,
                    0, 0,
                    0, "", 0);
                CMsgBase::SendNotificationPerson(CConfBase::GetString("tbxml_project"), pstSession->m_stReqParam.m_udwSvrId, pstSession->m_dwMailReceiverId, stNoticInfo);
            }
        }

        pstSession->ResetReportInfo();
        CReportSvrRequest::MailUserPut(pstSession, astMailUserItem, udwMailUserNum);

        // send request
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__6;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MailSendByUid: insert mail or update stat failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -10;
        }
        return 0;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__6)
    {
        //wave@push_data
        if(pstSession->m_dwMailReceiverId)
        {
            if(ucSendType == EN_MAIL_SEND_TYPE_PLAYER_TO_ALLIANCE)
            {
                CPushDataBasic::PushDataAid_Refresh(pstSession, pstSession->m_dwMailReceiverId);
            }
            else
            {
                CPushDataBasic::PushDataUid_Refresh(pstSession, pstSession->m_dwMailReceiverId);
            }
        }

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__7;
        // send request
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__REPORT_SVR;

        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendReportSvrRequest(pstSession);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_MailSend: insert mail or update stat failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -11;
        }
        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__7)
    {
        //task count
        if(ucSendType == EN_MAIL_SEND_TYPE_PLAYER_TO_ALLIANCE)
        {
            CQuestLogic::SetTaskCurValue(&pstSession->m_stUserInfo,pstCity, EN_TASK_TYPE_SEND_ALLIANCE_MAIL);
        }
        else
        {
            CQuestLogic::SetTaskCurValue(&pstSession->m_stUserInfo,pstCity, EN_TASK_TYPE_SEND_MAIL);
        }

        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    // set next procedure
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessMailReport::ProcessCmd_MailGet(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    TINT32 dwIsSupport = atoi(pstSession->m_stReqParam.m_szKey[0]);  //是否是客服系统拉取

    SMailUserRspInfo *pstMailUserInfo = &pstSession->m_stUserInfo.m_stMailUserInfo;

    // 1. 发送mail stat获取请求
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        // set package
        pstSession->ResetReportInfo();
        CReportSvrRequest::QueryMailIdGet(pstSession, dwIsSupport);
        // send request
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__REPORT_SVR;

        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendReportSvrRequest(pstSession);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("CProcessMailReport::ProcessCmd_MailGet request failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
        return 0;
    }

    // 2. 解析获取的mail的id数据
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecReportRsp.size(); udwIdx++)
        {
            memcpy(pstMailUserInfo, pstSession->m_vecReportRsp[udwIdx]->sRspContent.c_str(),
                pstSession->m_vecReportRsp[udwIdx]->sRspContent.size());
        }

        if (dwIsSupport != 1)
        {
            pstSession->m_stUserInfo.m_tbUserStat.Set_Unread_mail(pstMailUserInfo->m_udwMailUnreadNum);
            if (pstMailUserInfo->m_ddwNewestMid > pstSession->m_stUserInfo.m_tbUserStat.m_nNewest_mailid)
            {
                pstSession->m_stUserInfo.m_tbUserStat.Set_Newest_mailid(pstMailUserInfo->m_ddwNewestMid);
            }
            if (pstSession->m_stUserInfo.m_tbUserStat.m_nNewest_mailid != pstSession->m_stUserInfo.m_tbUserStat.m_nReturn_mailid && dwIsSupport == 0)
            {
                pstSession->m_stUserInfo.m_tbUserStat.Set_Return_mailid(pstSession->m_stUserInfo.m_tbUserStat.m_nNewest_mailid);
            }
        }

        pstSession->ResetAwsInfo();
        TbMail tbMailItem;

        for(TUINT32 udwIdx = 0; udwIdx < pstMailUserInfo->m_udwMailEntryNum; ++udwIdx)
        {
            tbMailItem.Reset();
            tbMailItem.Set_Id(pstMailUserInfo->m_aMailToReturn[udwIdx].ddwMid);
            CAwsRequest::GetItem(pstSession, &tbMailItem, ETbMAIL_OPEN_TYPE_PRIMARY);
        }

        if (pstMailUserInfo->m_udwMailEntryNum == 0)
        {
            pstSession->m_stUserInfo.m_udwMailNum = 0;
            pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_MAIL_LIST;
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
            return 0;
        }
        else
        {
            // set next procedure
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

            bNeedResponse = TRUE;
            // send request
            dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
            if(dwRetCode < 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("CProcessMailReport::ProcessCmd_MailGet send aws request failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                return -2;
            }
            return 0;
        }
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        // parse data
        pstSession->m_stUserInfo.m_udwMailNum = 0;
        for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecAwsRsp.size(); udwIdx++)
        {
            string strTableRawName = CCommonFunc::GetTableRawName((pstSession->m_vecAwsRsp[udwIdx])->sTableName.c_str());
            if(strTableRawName != EN_AWS_TABLE_MAIL)
            {
                continue;
            }
            TbMail& tbMail = pstSession->m_stUserInfo.m_atbMailList[pstSession->m_stUserInfo.m_udwMailNum];
            dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[udwIdx], &tbMail);
            if(dwRetCode > 0)
            {
                pstSession->m_stUserInfo.m_udwMailNum += dwRetCode;
            }
        }

        pstSession->ResetReportInfo();
        for (TUINT32 udwIdx = 0; udwIdx < pstMailUserInfo->m_udwMailEntryNum; ++udwIdx)
        {
            TBOOL bFind = FALSE;
            for(TUINT32 udwTrueMailIndex = 0; udwTrueMailIndex < pstSession->m_stUserInfo.m_udwMailNum; ++udwTrueMailIndex)
            {
                if(pstSession->m_stUserInfo.m_atbMailList[udwTrueMailIndex].m_nId ==
                    pstMailUserInfo->m_aMailToReturn[udwIdx].ddwMid)
                {
                    bFind = TRUE;
                    break;
                }
            }
            if(!bFind)
            {
                CReportSvrRequest::MailUserDelete(pstSession, pstMailUserInfo->m_aMailToReturn[udwIdx].ddwIndexUid,
                    pstMailUserInfo->m_aMailToReturn[udwIdx].ddwMid);
            }
        }

        if(pstSession->m_vecReportReq.size() > 0)
        {
            // set next procedure
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__REPORT_SVR;

            bNeedResponse = TRUE;
            // send request
            dwRetCode = CBaseProcedure::SendReportSvrRequest(pstSession);
            if(dwRetCode < 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("CProcessMailReport::ProcessCmd_MailGet send aws request failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                return -3;
            }
            return 0;
        }

        // set next procedure
        pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_MAIL_LIST;
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_MAIL_LIST;
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_MAIL_LIST;
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessMailReport::ProcessCmd_MailDetailGet(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;

    TINT32 dwDisplayType = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TINT64 ddwSenderUid = strtoull(pstSession->m_stReqParam.m_szKey[1], NULL, 10);
    TINT64 ddwReceiverUid = strtoull(pstSession->m_stReqParam.m_szKey[2], NULL, 10);
    TINT64 ddwMailId = strtoull(pstSession->m_stReqParam.m_szKey[3], NULL, 10);
    TINT32 dwIsSupport = strtoull(pstSession->m_stReqParam.m_szKey[4], NULL, 10);

    SMailUserRspInfo *pstMailUserInfo = &pstSession->m_stUserInfo.m_stMailUserInfo;
    // 1. 发送mail stat获取请求
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        // set package
        pstSession->ResetReportInfo();
        CReportSvrRequest::QueryMailDetailIdGet(pstSession, ddwMailId, dwDisplayType, ddwSenderUid, ddwReceiverUid, dwIsSupport);
        // send request
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__REPORT_SVR;

        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendReportSvrRequest(pstSession);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("CProcessMailReport::ProcessCmd_MailGet request failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
        return 0;
    }

    // 2. 解析获取的mail的id数据
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecReportRsp.size(); udwIdx++)
        {
            memcpy(pstMailUserInfo, pstSession->m_vecReportRsp[udwIdx]->sRspContent.c_str(),
                pstSession->m_vecReportRsp[udwIdx]->sRspContent.size());
        }

        pstSession->ResetAwsInfo();

        TbMail tbMailItem;
        for(TUINT32 udwIdx = 0; udwIdx < pstMailUserInfo->m_udwMailEntryNum; ++udwIdx)
        {
            tbMailItem.Reset();
            tbMailItem.Set_Id(pstMailUserInfo->m_aMailToReturn[udwIdx].ddwMid);
            CAwsRequest::GetItem(pstSession, &tbMailItem, ETbMAIL_OPEN_TYPE_PRIMARY);
        }

        if (pstMailUserInfo->m_udwMailEntryNum == 0)
        {
            pstSession->m_stUserInfo.m_udwMailNum = 0;
            pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_MAIL_LIST;
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
            return 0;
        }
        else
        {
            // set next procedure
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

            bNeedResponse = TRUE;
            // send request
            dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
            if(dwRetCode < 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("CProcessMailReport::ProcessCmd_MailGet send aws request failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                return -2;
            }
            return 0;
        }
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        // parse data
        pstSession->m_stUserInfo.m_udwMailNum = 0;
        for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecAwsRsp.size(); udwIdx++)
        {
            TbMail& tbMail = pstSession->m_stUserInfo.m_atbMailList[pstSession->m_stUserInfo.m_udwMailNum];
            dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[udwIdx], &tbMail);
            if(dwRetCode > 0)
            {
                pstSession->m_stUserInfo.m_udwMailNum += dwRetCode;
            }
        }

        pstSession->ResetReportInfo();
        for (TUINT32 udwIdx = 0; udwIdx < pstMailUserInfo->m_udwMailEntryNum; ++udwIdx)
        {
            TBOOL bFind = FALSE;
            for(TUINT32 udwTrueMailIndex = 0; udwTrueMailIndex < pstSession->m_stUserInfo.m_udwMailNum; ++udwTrueMailIndex)
            {
                if(pstSession->m_stUserInfo.m_atbMailList[udwTrueMailIndex].m_nId ==
                    pstMailUserInfo->m_aMailToReturn[udwIdx].ddwMid)
                {
                    bFind = TRUE;
                    break;
                }
            }
            if(!bFind)
            {
                CReportSvrRequest::MailUserDelete(pstSession, pstMailUserInfo->m_aMailToReturn[udwIdx].ddwIndexUid,
                    pstMailUserInfo->m_aMailToReturn[udwIdx].ddwMid);
            }
        }

        if(pstSession->m_vecReportReq.size() > 0)
        {
            // set next procedure
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__REPORT_SVR;

            bNeedResponse = TRUE;
            // send request
            dwRetCode = CBaseProcedure::SendReportSvrRequest(pstSession);
            if(dwRetCode < 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("CProcessMailReport::ProcessCmd_MailGet send aws request failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                return -3;
            }
            return 0;
        }

        // set next procedure
        pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_MAIL_LIST;
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_MAIL_LIST;
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_MAIL_LIST;
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessMailReport::ProcessCmd_MailRead(SSession *pstSession, TBOOL &bNeedResponse)
{
    return CProcessMailReport::SetMailStatus(EN_MAIL_STATUS_READ, pstSession, bNeedResponse);
}

TINT32 CProcessMailReport::ProcessCmd_MailDel(SSession *pstSession, TBOOL &bNeedResponse)
{
    return CProcessMailReport::SetMailStatus(EN_MAIL_STATUS_DEL, pstSession, bNeedResponse);
}

TINT32 CProcessMailReport::ProcessCmd_MailStar(SSession *pstSession, TBOOL &bNeedResponse)
{
    return CProcessMailReport::SetMailStatus(EN_MAIL_STATUS_MARK, pstSession, bNeedResponse);
}

TINT32 CProcessMailReport::ProcessCmd_MailUnstar(SSession *pstSession, TBOOL &bNeedResponse)
{
    return CProcessMailReport::SetMailStatus(EN_MAIL_STATUS_MARK, pstSession, bNeedResponse, TRUE);
}

TINT32 CProcessMailReport::ProcessCmd_MailRewardCollect(SSession* pstSession, TBOOL& bNeedResponse)
{
    TINT32 dwRetCode = 0;
    TINT64 ddwTargetMailId = strtoull(pstSession->m_stReqParam.m_szKey[0], NULL, 10);

    SCityInfo *pstCity = &pstSession->m_stUserInfo.m_stCityInfo;
    if(NULL == pstCity)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
        TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd__BuildingUpgrade: no city info [seq=%u]",
            pstSession->m_stUserInfo.m_udwBSeqNo));
        return -1;
    }
    SUserInfo* pstUser = &pstSession->m_stUserInfo;

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        pstSession->ResetAwsInfo();
        TbMail tbMail;
        tbMail.Set_Id(ddwTargetMailId);
        CAwsRequest::GetItem(pstSession, &tbMail, ETbMAIL_OPEN_TYPE_PRIMARY);

        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("CProcessMailReport::ProcessCmd_MailRewardCollect send aws request failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }
        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        TbMail tbMail;
        tbMail.Reset();

        for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecAwsRsp.size(); ++udwIdx)
        {
            AwsRspInfo* pstAwsRspInfo = pstSession->m_vecAwsRsp[udwIdx];
            string strTableRawName = CCommonFunc::GetTableRawName(pstAwsRspInfo->sTableName);
            if(strTableRawName == EN_AWS_TABLE_MAIL)
            {
                dwRetCode = CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &tbMail);
                if (dwRetCode <= 0 || tbMail.m_nId != ddwTargetMailId)
                {
                    pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
                    TSE_LOG_ERROR(pstSession->m_poServLog, ("CProcessMailReport::ProcessCmd_MailRewardCollect get target mail failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                    return -3;
                }
                break;
            }
        }

        if (tbMail.m_nMaildocid == EN_MAIL_ID__GIVE_GIFT)
        {
            pstSession->m_bPickUpGift = TRUE;
            pstSession->udwGiftUid = tbMail.m_nSuid;
            Json::Value jTmp;
            Json::Reader reader;
            reader.parse(tbMail.m_sEscs, jTmp);
            if (jTmp.isArray() && jTmp.size() > 0)
            {
                pstSession->udwGiftId = jTmp[0U].asUInt();
            }
        }

        CGlobalResLogic::AddGlobalRes(&pstSession->m_stUserInfo, pstCity, &(tbMail.m_bReward[0]));

        for (TUINT32 udwIdx = 0; udwIdx < tbMail.m_bEx_reward.m_udwNum; udwIdx++)
        {
            CGlobalResLogic::AddGlobalRes(pstUser, pstCity, tbMail.m_bEx_reward[udwIdx].ddwType,
                tbMail.m_bEx_reward[udwIdx].ddwId, tbMail.m_bEx_reward[udwIdx].ddwNum);
        }

        CReportSvrRequest::SetMailRewardCollect(pstSession, ddwTargetMailId);

        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__REPORT_SVR;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendReportSvrRequest(pstSession);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("CProcessMailReport::ProcessCmd_MailRewardCollect send report svr req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -6;
        }
        return 0;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        SMailUserRspInfo stMailUserInfo;
        stMailUserInfo.Reset();
        for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecReportRsp.size(); udwIdx++)
        {
            dwRetCode = pstSession->m_vecReportRsp[udwIdx]->dwRetCode;
            break;
        }

        if (dwRetCode != 200)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("CProcessMailReport::ProcessCmd_MailRewardCollect mail_user status error [seq=%u]", 
                pstSession->m_stUserInfo.m_udwBSeqNo));
            return -3;
        }

        if (pstSession->m_bPickUpGift)
        {
            CMsgBase::PickUpGift(pstUser->m_tbPlayer.m_nUid, pstSession->udwGiftUid, pstSession->udwGiftId, ddwTargetMailId);
        }

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessMailReport::ProcessCmd_GiftSend(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;

    TINT32 dwTargetId = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TINT32 dwGiftId = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TCHAR *pszContent = pstSession->m_stReqParam.m_szKey[2];
    TCHAR *pszReward = pstSession->m_stReqParam.m_szKey[3];

    SCityInfo* pstCity = &pstSession->m_stUserInfo.m_stCityInfo;
    TbPlayer *ptbPlayer = &pstSession->m_stUserInfo.m_tbPlayer;

    // 1. init
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        if (dwTargetId <= 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -2;
        }

        pstSession->ResetAwsInfo();
        CAwsRequest::UserGetByUid(pstSession, dwTargetId);

        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_GiftSend: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -5;
        }
        return 0;
    }

    // 3. 解析响应
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        pstSession->m_udwMailRUidNum = 0;
        dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[0], &pstSession->m_tbTmpPlayer);
        if (dwRetCode > 0 && pstSession->m_tbTmpPlayer.m_nUid == dwTargetId)
        {
            pstSession->m_audwMailRUidList[pstSession->m_udwMailRUidNum++] = pstSession->m_tbTmpPlayer.m_nUid;
        }
        if (pstSession->m_udwMailRUidNum == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__MAIL_RECEIVER_INVALID;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_GiftSend: no such user[uid=%d] [seq=%u]",
                dwTargetId, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
        pstSession->m_dwMailReceiverId = pstSession->m_audwMailRUidList[0];

        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
    }

    // 4. 获取mailid——发送请求
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        // set package
        pstSession->ResetAwsInfo();
        CAwsRequest::GetGlobalNewId(pstSession, EN_GLOBAL_PARAM__MAIL_ID);

        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_GiftSend: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -8;
        }
        return 0;
    }

    // 5. 获取mailid——解析结果
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__4;

        // parse data
        TbParam tbParamRes;
        for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecAwsRsp.size(); ++udwIdx)
        {
            AwsRspInfo *pstAwsRspInfo = pstSession->m_vecAwsRsp[udwIdx];
            string strTableRawName = CCommonFunc::GetTableRawName(pstAwsRspInfo->sTableName);
            if (strTableRawName == EN_AWS_TABLE_PARAM)
            {
                CAwsResponse::OnUpdateItemRsp(*pstAwsRspInfo, &tbParamRes);
                break;
            }
        }
        if (tbParamRes.m_nVal == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_PACKAGE_ERR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_GiftSend: get mail id failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -9;
        }
        pstSession->m_ddwNewMailId = tbParamRes.m_nVal;
    }

    // 6. 邮件发送, 并更新统计信息
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__4)
    {
        pstSession->ResetAwsInfo();

        Json::Reader reader;
        Json::Value jContent;
        TINT32 dwRawLangId = -1;
        TBOOL bHasContent = FALSE;
        if (TRUE == reader.parse(pszContent, jContent))
        {
            bHasContent = TRUE;
            dwRawLangId = jContent["raw_lang"].asInt();
        }

        TbMail tbMail;
        tbMail.Set_Id(pstSession->m_ddwNewMailId);
        tbMail.Set_Time(CTimeUtils::GetUnixTime());
        tbMail.Set_Suid(pstSession->m_stReqParam.m_udwUserId);
        tbMail.Set_Sender(pstSession->m_stUserInfo.m_tbPlayer.m_sUin);
        tbMail.Set_Scid(pstSession->m_stReqParam.m_udwCityId);
        tbMail.Set_Send_type(EN_MAIL_SEND_TYPE_PLAYER_TO_PLAYERS);
        tbMail.Set_Maildocid(EN_MAIL_ID__GIVE_GIFT);
        ostringstream oss;
        oss << "[" << dwGiftId << "]";
        tbMail.Set_Escs(oss.str());
        if (bHasContent)
        {
            tbMail.Set_Content(jContent["raw_content"].asString());
        }
        tbMail.Set_Sender_player_avatar(pstSession->m_stUserInfo.m_tbPlayer.m_nAvatar);
        tbMail.Set_Sender_al_nick(pstSession->m_stUserInfo.m_tbAlliance.m_sAl_nick_name);
        tbMail.Set_Raw_lang(dwRawLangId);
        if (bHasContent && !jContent["translate_info"].empty())
        {
            Json::FastWriter writer;
            writer.omitEndingLineFeed();
            tbMail.Set_Translate_content(writer.write(jContent["translate_info"]));
        }

        tbMail.Set_Display_type(EN_MAIL_DISPLAY_TYPE_NORMAL);
        tbMail.Set_Ruid(pstSession->m_dwMailReceiverId);

        Json::Value jsonReward;
        TBOOL bHasReward = FALSE;
        if (reader.parse(pszReward, jsonReward) == TRUE)
        {
            for (TUINT32 udwIdx = 0; udwIdx < jsonReward.size(); ++udwIdx)
            {
                if (tbMail.m_bEx_reward.m_udwNum >= TBMAIL_EX_REWARD_MAX_NUM)
                {
                    break;
                }

                tbMail.m_bEx_reward[tbMail.m_bEx_reward.m_udwNum++].SetValue(
                    jsonReward[udwIdx][0U].asUInt(),
                    jsonReward[udwIdx][1U].asUInt(),
                    jsonReward[udwIdx][2U].asUInt());
                tbMail.SetFlag(TbMAIL_FIELD_EX_REWARD);
                bHasReward = TRUE;
            }
        }

        // set data package - mail
        pstSession->m_stUserInfo.m_tbUserStat.Set_Newest_mailid(pstSession->m_ddwNewMailId);
        CAwsRequest::UpdateItem(pstSession, &tbMail);

        TbMail_user astMailUserItem[2];
        TUINT32 udwMailUserNum = 0;
        
        astMailUserItem[udwMailUserNum].Reset();
        astMailUserItem[udwMailUserNum].Set_Uid(pstSession->m_stUserInfo.m_tbPlayer.m_nUid);
        astMailUserItem[udwMailUserNum].Set_Mid(pstSession->m_ddwNewMailId);
        astMailUserItem[udwMailUserNum].Set_Suid(pstSession->m_stUserInfo.m_tbPlayer.m_nUid);
        astMailUserItem[udwMailUserNum].Set_Time(tbMail.m_nTime);
        astMailUserItem[udwMailUserNum].Set_Tuid(pstSession->m_dwMailReceiverId);
        astMailUserItem[udwMailUserNum].Set_Display_type(EN_MAIL_DISPLAY_TYPE_NORMAL);
        astMailUserItem[udwMailUserNum].Set_Status(EN_MAIL_STATUS_READ);
        astMailUserItem[udwMailUserNum].Set_Has_reward(bHasReward);
        astMailUserItem[udwMailUserNum].Set_Receiver_avatar(pstSession->m_tbTmpPlayer.m_nAvatar);
        astMailUserItem[udwMailUserNum].Set_Receiver_name(pstSession->m_tbTmpPlayer.m_sUin);
        astMailUserItem[udwMailUserNum].Set_Receiver_alnick(pstSession->m_tbTmpPlayer.m_sAl_nick_name);
        udwMailUserNum++;

        astMailUserItem[udwMailUserNum].Reset();
        astMailUserItem[udwMailUserNum].Set_Uid(pstSession->m_dwMailReceiverId);
        astMailUserItem[udwMailUserNum].Set_Mid(pstSession->m_ddwNewMailId);
        astMailUserItem[udwMailUserNum].Set_Suid(pstSession->m_stUserInfo.m_tbPlayer.m_nUid);
        astMailUserItem[udwMailUserNum].Set_Time(tbMail.m_nTime);
        astMailUserItem[udwMailUserNum].Set_Tuid(pstSession->m_dwMailReceiverId);
        astMailUserItem[udwMailUserNum].Set_Display_type(EN_MAIL_DISPLAY_TYPE_NORMAL);
        astMailUserItem[udwMailUserNum].Set_Has_reward(bHasReward);
        astMailUserItem[udwMailUserNum].Set_Receiver_avatar(pstSession->m_tbTmpPlayer.m_nAvatar);
        astMailUserItem[udwMailUserNum].Set_Receiver_name(pstSession->m_tbTmpPlayer.m_sUin);
        astMailUserItem[udwMailUserNum].Set_Receiver_alnick(pstSession->m_tbTmpPlayer.m_sAl_nick_name);
        udwMailUserNum++;

        pstSession->ResetReportInfo();
        CReportSvrRequest::MailUserPut(pstSession, astMailUserItem, udwMailUserNum);

        // send request
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__5;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_GiftSend: insert mail or update stat failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -10;
        }
        return 0;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__5)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__6;
        // send request
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__REPORT_SVR;

        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendReportSvrRequest(pstSession);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_GiftSend: insert mail or update stat failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -11;
        }
        return 0;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__6)
    {
        //wave@push_data
        CPushDataBasic::PushDataUid_Refresh(pstSession, ptbPlayer->m_nUid);
        if (pstSession->m_dwMailReceiverId)
        {
            CPushDataBasic::PushDataUid_Refresh(pstSession, pstSession->m_dwMailReceiverId);
        }

        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    // set next procedure
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessMailReport::ProcessCmd_GiftPickUp(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;

    TUINT32 udwTargetId = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TUINT32 udwGiftId = atoi(pstSession->m_stReqParam.m_szKey[1]);
    TINT64 ddwMailId = strtoll(pstSession->m_stReqParam.m_szKey[2], NULL, 10);

    SCityInfo* pstCity = &pstSession->m_stUserInfo.m_stCityInfo;
    TbPlayer *ptbPlayer = &pstSession->m_stUserInfo.m_tbPlayer;

    // 1. init
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        if (udwTargetId == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -2;
        }

        pstSession->ResetAwsInfo();
        CAwsRequest::UserGetByUid(pstSession, udwTargetId);
        CAwsRequest::UserStatGet(pstSession, udwTargetId);

        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_GiftSend: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -5;
        }
        return 0;
    }

    // 3. 解析响应
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        pstSession->m_udwMailRUidNum = 0;
        pstSession->m_tbTmpPlayer.Reset();
        pstSession->m_tbTmpUserStat.Reset();
        for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecAwsRsp.size(); udwIdx++)
        {
            AwsRspInfo *pstAwsRspInfo = pstSession->m_vecAwsRsp[udwIdx];
            string strTableRawName = CCommonFunc::GetTableRawName(pstAwsRspInfo->sTableName);
            if (strTableRawName == EN_AWS_TABLE_PLAYER)
            {
                CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &pstSession->m_tbTmpPlayer);
                continue;
            }
            if (strTableRawName == EN_AWS_TABLE_USER_STAT)
            {
                CAwsResponse::OnGetItemRsp(*pstAwsRspInfo, &pstSession->m_tbTmpUserStat);
                continue;
            }
        }
        if (pstSession->m_tbTmpPlayer.m_nUid == udwTargetId
            && pstSession->m_tbTmpUserStat.m_nUid == udwTargetId)
        {
            pstSession->m_audwMailRUidList[pstSession->m_udwMailRUidNum++] = pstSession->m_tbTmpPlayer.m_nUid;
        }
        if (pstSession->m_udwMailRUidNum == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__MAIL_RECEIVER_INVALID;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_GiftSend: no such user[uid=%d] [seq=%u]",
                udwTargetId, pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
        pstSession->m_dwMailReceiverId = pstSession->m_audwMailRUidList[0];

        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
    }

    // 4. 获取mailid——发送请求
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        // set package
        pstSession->ResetAwsInfo();
        CAwsRequest::GetGlobalNewId(pstSession, EN_GLOBAL_PARAM__MAIL_ID);

        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_GiftSend: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -8;
        }
        return 0;
    }

    // 5. 获取mailid——解析结果
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__4;

        // parse data
        TbParam tbParamRes;
        for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecAwsRsp.size(); ++udwIdx)
        {
            AwsRspInfo *pstAwsRspInfo = pstSession->m_vecAwsRsp[udwIdx];
            string strTableRawName = CCommonFunc::GetTableRawName(pstAwsRspInfo->sTableName);
            if (strTableRawName == EN_AWS_TABLE_PARAM)
            {
                CAwsResponse::OnUpdateItemRsp(*pstAwsRspInfo, &tbParamRes);
                break;
            }
        }
        if (tbParamRes.m_nVal == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_PACKAGE_ERR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_GiftSend: get mail id failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -9;
        }
        pstSession->m_ddwNewMailId = tbParamRes.m_nVal;
    }

    // 6. 邮件发送, 并更新统计信息
    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__4)
    {
        pstSession->ResetAwsInfo();

        TbMail tbMail;
        tbMail.Set_Id(pstSession->m_ddwNewMailId);
        tbMail.Set_Time(CTimeUtils::GetUnixTime());
        tbMail.Set_Suid(pstSession->m_stReqParam.m_udwUserId);
        tbMail.Set_Sender(pstSession->m_stUserInfo.m_tbPlayer.m_sUin);
        tbMail.Set_Scid(pstSession->m_stReqParam.m_udwCityId);
        tbMail.Set_Send_type(EN_MAIL_SEND_TYPE_PLAYER_TO_PLAYERS);
        tbMail.Set_Maildocid(EN_MAIL_ID__PICK_UP_GIFT);
        ostringstream oss;
        oss << "[" << udwGiftId << "," << ddwMailId << "]";
        tbMail.Set_Escs(oss.str());

        tbMail.Set_Sender_player_avatar(pstSession->m_stUserInfo.m_tbPlayer.m_nAvatar);
        tbMail.Set_Sender_al_nick(pstSession->m_stUserInfo.m_tbAlliance.m_sAl_nick_name);
        tbMail.Set_Raw_lang(-1);

        tbMail.Set_Display_type(EN_MAIL_DISPLAY_TYPE_NORMAL);
        tbMail.Set_Ruid(pstSession->m_dwMailReceiverId);

        // set data package - mail
        pstSession->m_stUserInfo.m_tbUserStat.Set_Newest_mailid(pstSession->m_ddwNewMailId);
        CAwsRequest::UpdateItem(pstSession, &tbMail);

        TbMail_user astMailUserItem[2];
        TUINT32 udwMailUserNum = 0;

        astMailUserItem[udwMailUserNum].Reset();
        astMailUserItem[udwMailUserNum].Set_Uid(pstSession->m_stUserInfo.m_tbPlayer.m_nUid);
        astMailUserItem[udwMailUserNum].Set_Mid(pstSession->m_ddwNewMailId);
        astMailUserItem[udwMailUserNum].Set_Suid(pstSession->m_stUserInfo.m_tbPlayer.m_nUid);
        astMailUserItem[udwMailUserNum].Set_Time(tbMail.m_nTime);
        astMailUserItem[udwMailUserNum].Set_Tuid(pstSession->m_dwMailReceiverId);
        astMailUserItem[udwMailUserNum].Set_Display_type(EN_MAIL_DISPLAY_TYPE_NORMAL);
        astMailUserItem[udwMailUserNum].Set_Status(EN_MAIL_STATUS_READ);
        astMailUserItem[udwMailUserNum].Set_Receiver_avatar(pstSession->m_tbTmpPlayer.m_nAvatar);
        astMailUserItem[udwMailUserNum].Set_Receiver_name(pstSession->m_tbTmpPlayer.m_sUin);
        astMailUserItem[udwMailUserNum].Set_Receiver_alnick(pstSession->m_tbTmpPlayer.m_sAl_nick_name);
        udwMailUserNum++;

        astMailUserItem[udwMailUserNum].Reset();
        astMailUserItem[udwMailUserNum].Set_Uid(pstSession->m_dwMailReceiverId);
        astMailUserItem[udwMailUserNum].Set_Mid(pstSession->m_ddwNewMailId);
        astMailUserItem[udwMailUserNum].Set_Suid(pstSession->m_stUserInfo.m_tbPlayer.m_nUid);
        astMailUserItem[udwMailUserNum].Set_Time(tbMail.m_nTime);
        astMailUserItem[udwMailUserNum].Set_Tuid(pstSession->m_dwMailReceiverId);
        astMailUserItem[udwMailUserNum].Set_Display_type(EN_MAIL_DISPLAY_TYPE_NORMAL);
        astMailUserItem[udwMailUserNum].Set_Receiver_avatar(pstSession->m_tbTmpPlayer.m_nAvatar);
        astMailUserItem[udwMailUserNum].Set_Receiver_name(pstSession->m_tbTmpPlayer.m_sUin);
        astMailUserItem[udwMailUserNum].Set_Receiver_alnick(pstSession->m_tbTmpPlayer.m_sAl_nick_name);
        udwMailUserNum++;

        pstSession->ResetReportInfo();
        CReportSvrRequest::MailUserPut(pstSession, astMailUserItem, udwMailUserNum);
        CReportSvrRequest::SetMailRewardCollect(pstSession, &pstSession->m_tbTmpPlayer, &pstSession->m_tbTmpUserStat, ddwMailId);

        // send request
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__5;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_GiftSend: insert mail or update stat failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -10;
        }
        return 0;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__5)
    {
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__6;
        // send request
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__REPORT_SVR;

        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendReportSvrRequest(pstSession);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_GiftSend: insert mail or update stat failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -11;
        }
        return 0;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__6)
    {
        //wave@push_data
        CPushDataBasic::PushDataUid_Refresh(pstSession, ptbPlayer->m_nUid);
        if (pstSession->m_dwMailReceiverId)
        {
            CPushDataBasic::PushDataUid_Refresh(pstSession, pstSession->m_dwMailReceiverId);
        }

        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    // set next procedure
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessMailReport::ProcessCmd_ReportGet(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;

    // 1. 发送mail stat获取请求
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        // set package
        pstSession->ResetReportInfo();
        CReportSvrRequest::QueryReportIdGet(pstSession);
        // send request
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__REPORT_SVR;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendReportSvrRequest(pstSession);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("CProcessMailReport::ProcessCmd_ReportGet request failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
        return 0;
    }

    // 2. 解析获取的mail的id数据
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        SReportUserRspInfo *pstReportUserInfo = &pstSession->m_stUserInfo.m_stReportUserInfo;
        for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecReportRsp.size(); udwIdx++)
        {
            memcpy(pstReportUserInfo, pstSession->m_vecReportRsp[udwIdx]->sRspContent.c_str(),
                pstSession->m_vecReportRsp[udwIdx]->sRspContent.size());
        }

        pstSession->m_stUserInfo.m_tbUserStat.Set_Unread_report(pstReportUserInfo->m_udwReportUnreadNum);
        if (pstReportUserInfo->m_ddwNewestRid > 0 && pstReportUserInfo->m_ddwNewestRid > pstSession->m_stUserInfo.m_tbUserStat.m_nNewest_reportid)
        {
            pstSession->m_stUserInfo.m_tbUserStat.Set_Newest_reportid(pstReportUserInfo->m_ddwNewestRid);
        }
        if(pstSession->m_stUserInfo.m_tbUserStat.m_nNewest_reportid != pstSession->m_stUserInfo.m_tbUserStat.m_nReturn_reportid)
        {
            pstSession->m_stUserInfo.m_tbUserStat.Set_Return_reportid(pstSession->m_stUserInfo.m_tbUserStat.m_nNewest_reportid);
        }

        pstSession->ResetAwsInfo();
        TbReport tbReportItem;
        for (TUINT32 udwIdx = 0; udwIdx < pstReportUserInfo->m_udwReportEntryNum; ++udwIdx)
        {
            tbReportItem.Reset();
            tbReportItem.Set_Id(pstReportUserInfo->m_aReportToReturn[udwIdx].ddwRid);
            CAwsRequest::GetItem(pstSession, &tbReportItem, ETbREPORT_OPEN_TYPE_PRIMARY);
        }

        if (pstReportUserInfo->m_udwReportEntryNum == 0)
        {
            pstSession->m_stUserInfo.m_udwReportNum = 0;
            pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_REPORT;
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
            return 0;
        }
        else
        {
            // set next procedure
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

            // send request
            bNeedResponse = TRUE;
            dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
            if(dwRetCode < 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("CProcessMailReport::ProcessCmd_ReportGet send aws request failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                return -2;
            }
            return 0;
        }
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        // parse data
        pstSession->m_stUserInfo.m_udwReportNum = 0;
        for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecAwsRsp.size(); udwIdx++)
        {
            TbReport& tbReport = pstSession->m_stUserInfo.m_atbReportList[pstSession->m_stUserInfo.m_udwReportNum];
            dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[udwIdx], &tbReport);
            if(dwRetCode > 0)
            {
                pstSession->m_stUserInfo.m_udwReportNum += dwRetCode;
            }
        }

        // set next procedure
        pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_REPORT;
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_REPORT;
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;;
}

TINT32 CProcessMailReport::ProcessCmd_ReportDetailGet(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    TINT32 dwReportType = atoi(pstSession->m_stReqParam.m_szKey[0]);

    // 1. 发送mail stat获取请求
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        // set package
        pstSession->ResetReportInfo();
        CReportSvrRequest::QueryReportDetailIdGet(pstSession, dwReportType);
        // send request
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__REPORT_SVR;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendReportSvrRequest(pstSession);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("CProcessMailReport::ProcessCmd_ReportDetailGet request failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -1;
        }
        return 0;
    }

    // 2. 解析获取的mail的id数据
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        // parse data
        SReportUserRspInfo *pstReportUserInfo = &pstSession->m_stUserInfo.m_stReportUserInfo;
        for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecReportRsp.size(); udwIdx++)
        {
            memcpy(pstReportUserInfo, pstSession->m_vecReportRsp[udwIdx]->sRspContent.c_str(),
                pstSession->m_vecReportRsp[udwIdx]->sRspContent.size());
        }

        pstSession->ResetAwsInfo();
        TbReport tbReportItem;

        for(TUINT32 udwIdx = 0; udwIdx < pstReportUserInfo->m_udwReportEntryNum; ++udwIdx)
        {
            tbReportItem.Reset();
            tbReportItem.Set_Id(pstReportUserInfo->m_aReportToReturn[udwIdx].ddwRid);
            CAwsRequest::GetItem(pstSession, &tbReportItem, ETbREPORT_OPEN_TYPE_PRIMARY);
        }

        if (pstReportUserInfo->m_udwReportEntryNum == 0)
        {
            pstSession->m_stUserInfo.m_udwReportNum = 0;
            pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_REPORT;
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
            return 0;
        }
        else
        {
            // set next procedure
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

            // send request
            bNeedResponse = TRUE;
            dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
            if(dwRetCode < 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("CProcessMailReport::ProcessCmd_ReportGet send aws request failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                return -2;
            }
            return 0;
        }
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        // parse data
        pstSession->m_stUserInfo.m_udwReportNum = 0;
        for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecAwsRsp.size(); udwIdx++)
        {
            TbReport& tbReport = pstSession->m_stUserInfo.m_atbReportList[pstSession->m_stUserInfo.m_udwReportNum];
            dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[udwIdx], &tbReport);
            if(dwRetCode > 0)
            {
                pstSession->m_stUserInfo.m_udwReportNum += dwRetCode;
            }
        }

        // set next procedure
        pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_REPORT;
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_REPORT;
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessMailReport::ProcessCmd_ReportRead(SSession *pstSession, TBOOL &bNeedResponse)
{
    return CProcessMailReport::SetReportStatus(EN_MAIL_STATUS_READ, pstSession, bNeedResponse);
}

TINT32 CProcessMailReport::ProcessCmd_ReportDel(SSession *pstSession, TBOOL &bNeedResponse)
{
    return CProcessMailReport::SetReportStatus(EN_MAIL_STATUS_DEL, pstSession, bNeedResponse);
}

TINT32 CProcessMailReport::ProcessCmd_OpMailGet(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;

    // get param
    TINT64 ddwMailId = strtoull(pstSession->m_stReqParam.m_szKey[0], NULL, 10);

    // 1. 由mail id获取mail——请求
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        // get package
        pstSession->ResetAwsReq();

        TbMail tbMailItem;
        tbMailItem.Set_Id(ddwMailId);
        CAwsRequest::GetItem(pstSession, &tbMailItem, ETbMAIL_OPEN_TYPE_PRIMARY);

        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("SendLocalHsRequest: send user mail get request failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }
        return 0;
    }

    // 2. 由mail id获取mail——解析
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        // parse data
        pstSession->m_stUserInfo.m_udwMailNum = 0;
        dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[0], pstSession->m_stUserInfo.m_atbMailList);
        if(dwRetCode > 0)
        {
            pstSession->m_stUserInfo.m_udwMailNum = dwRetCode;
        }

        // set next procedure
        pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_MAIL_LIST;
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    }

    return 0;
}

TINT32 CProcessMailReport::ProcessCmd_OpMailSend(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;
    TINT64 ddwMailId = 0;

    TUINT32 udwSize = DEFAULT_PERPAGE_NUM;
    TUINT32 audwTid[DEFAULT_PERPAGE_NUM];

    TUINT8 ucSendType = atoi(pstSession->m_stReqParam.m_szKey[0]);
    TCHAR* szTid = pstSession->m_stReqParam.m_szKey[1];
    TCHAR *pszTitle = pstSession->m_stReqParam.m_szKey[2];
    TINT32 dwDocId = atoi(pstSession->m_stReqParam.m_szKey[3]);
    TCHAR *pszContent = pstSession->m_stReqParam.m_szKey[4];
    TCHAR *pszExtraContent = pstSession->m_stReqParam.m_szKey[5];
    TINT32 dwSendUid = atoi(pstSession->m_stReqParam.m_szKey[6]);
    if(dwSendUid == 0)
    {
        dwSendUid = SYSTEM_ENCOURAGE;
    }
    TCHAR *pszReward = pstSession->m_stReqParam.m_szKey[7];
    TINT32 dwSingleSvr = atoi(pstSession->m_stReqParam.m_szKey[8]);
    TCHAR *pszPlatform = pstSession->m_stReqParam.m_szKey[9];
    TCHAR *pszVersion = pstSession->m_stReqParam.m_szKey[10];
    TCHAR *pszUrl = pstSession->m_stReqParam.m_szKey[11];
    TINT32 dwJmpType = atoi(pstSession->m_stReqParam.m_szKey[12]);

    memset(audwTid, 0, DEFAULT_PERPAGE_NUM);
    CCommonFunc::GetArrayFromString(szTid, ':', audwTid, udwSize);

    // 1. init
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        if(udwSize == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_OpMailSend: no receiver [seq=%u]", pstSession->m_udwSeqNo));
            return -1;
        }

        //只能发给一个联盟
        if(udwSize > 1 && ucSendType == EN_MAIL_SEND_TYPE_SYSTEM_TO_ALLIANCE)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_OpMailSend: too many receiver [seq=%u]", pstSession->m_udwSeqNo));
            return -2;
        }

        if(udwSize > DEFAULT_PERPAGE_NUM)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_OpMailSend: too many receiver [seq=%u]", pstSession->m_udwSeqNo));
            return -3;
        }

        if(ucSendType == EN_MAIL_SEND_TYPE_SYSTEM_TO_PLAYERS)
        {
            pstSession->m_dwMailReceiverId = audwTid[0];  //lucien note:这里只发了一封？
        }
        else
        {
            pstSession->m_dwMailReceiverId = -1 * static_cast<TINT32>(audwTid[0]);
        }
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
    }

    // 4. 获取mailid——发送请求
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        // set package
        pstSession->ResetAwsInfo();
        CAwsRequest::GetGlobalNewId(pstSession, EN_GLOBAL_PARAM__MAIL_ID);

        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_OpMailSend: send req failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -4;
        }
        return 0;
    }

    // 5. 获取mailid——解析结果
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__3;

        // parse data
        TbParam tbParamRes;
        dwRetCode = CAwsResponse::OnUpdateItemRsp(*pstSession->m_vecAwsRsp[0], &tbParamRes);
        if(tbParamRes.m_nVal == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__PARSE_PACKAGE_ERR;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("get mail id failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -5;
        }
        ddwMailId = tbParamRes.m_nVal;
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("ProcessCmd_OpMailSend: [new_mail_id=%ld][seq=%u]", ddwMailId, pstSession->m_stUserInfo.m_udwBSeqNo));
    }

    // 6. 邮件发送, 并更新统计信息
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__3)
    {
        TBOOL bHasReward = FALSE;
        TBOOL bIsAlRelation = FALSE;

        TbMail tbMail;

        //兼容性版本不开放
        switch(dwDocId)
        {
        case EN_MAIL_ID__FIRST_JOIN_ALLIANCE:
        case EN_MAIL_ID__FIRST_CREATE_ALLIANCE:
        case EN_MAIL_ID__JOINED_AN_ALLIANCE:
        case EN_MAIL_ID__CREATE_ALLIANCE:
        case EN_MAIL_ID__REQUEST_IS_DECLINED:
        case EN_MAIL_ID__BE_DISMISS:
        case EN_MAIL_ID__BE_PROMOTED:
        case EN_MAIL_ID__BE_DEMOTED:
        case EN_MAIL_ID__BE_FRIEND:
        case EN_MAIL_ID__CONFIRM_FRIEND:
        case EN_MAIL_ID__CONFIRM_NORMAL:
        case EN_MAIL_ID__CONFIRM_HOSTILE:
        case EN_MAIL_ID__CANCEL_HOSTILE:
        case EN_MAIL_ID__BE_HOSTILE:
            dwSendUid = SYSTEM_DIPLOMACY;
            break;

        case EN_MAIL_ID__SUPPORT_AUTO_REPLY:
            dwSendUid = SYSTEM_SUPPORT;
        default:
            break;
        }

        tbMail.Set_Id(ddwMailId);
        tbMail.Set_Raw_lang(pstSession->m_stReqParam.m_udwLang + 1);
        tbMail.Set_Time(CTimeUtils::GetUnixTime());
        tbMail.Set_Suid(dwSendUid);
        Json::Value jTmp;
        Json::Reader jtmpReader;
        switch(dwSendUid)
        {
        case SYSTEM_NOTICE:
            tbMail.Set_Sender("Update");
            tbMail.Set_Display_type(EN_MAIL_DISPLAY_TYPE_SYSTEM_NOTICE);
            break;
        case SYSTEM_EVENT:
            tbMail.Set_Sender("Event Rewards");
            tbMail.Set_Display_type(EN_MAIL_DISPLAY_TYPE_SYSTEM_EVENT);
            break;
        case SYSTEM_ENCOURAGE:
            tbMail.Set_Sender("Encourage");
            tbMail.Set_Display_type(EN_MAIL_DISPLAY_TYPE_SYSTEM_ENCOURAGE);
            break;
        case SYSTEM_WAR:
            tbMail.Set_Sender("War");
            tbMail.Set_Display_type(EN_MAIL_DISPLAY_TYPE_SYSTEM_ENCOURAGE);
            break;
        case SYSTEM_ACTIVITY:
            tbMail.Set_Sender("Announcement");
            tbMail.Set_Display_type(EN_MAIL_DISPLAY_TYPE_SYSTEM_ACTIVITY);
            break;
        case SYSTEM_INVITE:
            tbMail.Set_Sender("Alliance Invite");
            tbMail.Set_Display_type(EN_MAIL_DISPLAY_TYPE_ALLIANCE_INVITE);
            break;
        case SYSTEM_DIPLOMACY:
            tbMail.Set_Sender("Alliance Diplomacy");
            tbMail.Set_Display_type(EN_MAIL_DISPLAY_TYPE_ALLIANCE_DIPLOMACY);
            break;
        case SYSTEM_NEW_PLAYER_AL:
            jtmpReader.parse(pszExtraContent, jTmp);
            tbMail.Set_Sender(jTmp[1].asString());
            tbMail.Set_Sender_player_avatar(jTmp[2].asInt());
            tbMail.Set_Sender_al_nick(jTmp[3].asString());
            tbMail.Set_Display_type(EN_MAIL_DISPLAY_TYPE_SYSTEM_NEW_PLAYER_AL);
            break;
        case SYSTEM_HOLIDAY:
            tbMail.Set_Sender("War Age Studio");
            tbMail.Set_Display_type(EN_MAIL_DISPLAY_TYPE_SYSTEM_HOLIDAY);
            break;
        case SYSTEM_SUPPORT:
            tbMail.Set_Sender("Support");
            tbMail.Set_Display_type(EN_MAIL_DISPLAY_TYPE_SUPPORT_HELP);
        default:
            break;
        }
        tbMail.Set_Ruid(pstSession->m_dwMailReceiverId);
        tbMail.Set_Send_type(ucSendType);
        tbMail.Set_Title(pszTitle);
        tbMail.Set_Content(pszContent);
        tbMail.Set_Url(pszUrl);
        tbMail.Set_Jmp_type(dwJmpType);
        if (dwSendUid != SYSTEM_SUPPORT || 0 == strcmp(pszContent, ""))
        {
            tbMail.Set_Maildocid(dwDocId);
        }
        if(dwDocId > 0)
        {
            Json::Reader jsonReader;
            Json::Value jsonEscs = Json::Value(Json::arrayValue);
            switch(dwDocId)
            {
            case EN_MAIL_ID__FIRST_CREATE_ALLIANCE:
            case EN_MAIL_ID__FIRST_JOIN_ALLIANCE:
                jsonEscs.append(pszContent);
                jsonEscs.append(CGameInfo::GetInstance()->m_oJsonRoot["game_basic"][EN_GAME_BASIC_FIRST_JOIN_ALLIANCE_GEM_REWARD].asUInt());
                break;
            case EN_MAIL_ID__CREATE_ALLIANCE:
            case EN_MAIL_ID__JOINED_AN_ALLIANCE:
            case EN_MAIL_ID__REQUEST_IS_DECLINED:
            case EN_MAIL_ID__BE_DISMISS:
                jsonEscs.append(pszContent);
                break;
            case EN_MAIL_ID__BE_PROMOTED:
            case EN_MAIL_ID__BE_DEMOTED:
                jsonEscs.append(atoi(pszContent));
                break;
            case EN_MAIL_ID__BE_FRIEND:
            case EN_MAIL_ID__CANCEL_HOSTILE:
                tbMail.Set_Sender_al_info(pszExtraContent);
                jsonEscs.append(pszContent);
                bIsAlRelation = TRUE;
                break;
            case EN_MAIL_ID__CONFIRM_FRIEND:
            case EN_MAIL_ID__CONFIRM_NORMAL:
            case EN_MAIL_ID__CONFIRM_HOSTILE:
            case EN_MAIL_ID__BE_HOSTILE:
                jsonEscs.append(pszContent);
                bIsAlRelation = TRUE;
                break;
            case EN_MAIL_ID__PERSON_GOAL:
            case EN_MAIL_ID__PERSON_RANK:
                jsonReader.parse(pszExtraContent, jsonEscs);
                break;
            case EN_MAIL_ID__ALLIANCE_GOAL:
            case EN_MAIL_ID__ALLIANCE_RANK:
                bIsAlRelation = TRUE;
                jsonReader.parse(pszExtraContent, jsonEscs);
                break;
            case EN_MAIL_ID__DELAY_PURCHASE:
                jsonEscs.append(pszContent);
                jsonEscs.append(pszExtraContent);
                break;
            case EN_MAIL_ID__INVITE:
                tbMail.Set_Sender_al_info(pszExtraContent);
                tbMail.Set_Sender(pszContent);
                jtmpReader.parse(pszExtraContent, jTmp);
                tbMail.Set_Sender_al_nick(jTmp["al_nick"].asString());
                break;
            case EN_MAIL_ID__AUTO_TELL_NPC:
                jsonReader.parse(pszExtraContent, jsonEscs);
                tbMail.Set_Sender(jsonEscs[0].asString());
                tbMail.Set_Display_type(EN_MAIL_DISPLAY_TYPE_NORMAL);
                tbMail.Set_Sender_player_avatar(jsonEscs[1].asInt());
                tbMail.Set_Sender_al_nick(jsonEscs[2].asString());
                tbMail.Set_Send_type(EN_MAIL_SEND_TYPE_PLAYER_TO_PLAYERS);
                break;
            case EN_MAIL_ID__THRONE_HAS_BEEN_OCCUPIED:
                jsonEscs.append(pszContent);
                jsonEscs.append(pszExtraContent);
                dwSingleSvr = 1; //发单svr
                //tbMail.Set_Display_type(EN_MAIL_DISPLAY_TYPE_SYSTEM_ACTIVITY);
                break;
            case EN_MAIL_ID__BE_DUBBED_TITLE:
                jsonReader.parse(pszExtraContent, jsonEscs);
                break;
            case EN_MAIL_ID__EVENT_SEND_SCORE:
                tbMail.Set_Title(CProcessMailReport::GetMailTitle(pstSession, dwDocId));
                jsonReader.parse(pszExtraContent, jsonEscs);
                break;
            default:
                break;
            }

            if (dwSendUid == SYSTEM_NEW_PLAYER_AL)
            {
                jsonReader.parse(pszExtraContent, jsonEscs);
            }

            Json::FastWriter tmpWriter;
            tbMail.Set_Escs(tmpWriter.write(jsonEscs));
        }

        Json::Reader jsonReader;
        Json::Value jsonReward;
        if(jsonReader.parse(pszReward, jsonReward) == TRUE)
        {
            for(TUINT32 udwIdx = 0; udwIdx < jsonReward.size(); ++udwIdx)
            {
                if(tbMail.m_bEx_reward.m_udwNum >= TBMAIL_EX_REWARD_MAX_NUM)
                {
                    break;
                }

                tbMail.m_bEx_reward[tbMail.m_bEx_reward.m_udwNum++].SetValue(
                    jsonReward[udwIdx][0U].asUInt(),
                    jsonReward[udwIdx][1U].asUInt(),
                    jsonReward[udwIdx][2U].asUInt());
                tbMail.SetFlag(TbMAIL_FIELD_EX_REWARD);
                bHasReward = TRUE;
            }
        }

        if(dwDocId > 0)
        {
            string strMailDocId = CCommonFunc::NumToString(dwDocId);
            const Json::Value& oMailJson = CGameInfo::GetInstance()->m_oJsonRoot["game_encourage_mail"];

            if(oMailJson.isMember(strMailDocId))
            {
                for(TUINT32 udwIdx = 0; udwIdx < oMailJson[strMailDocId]["r"]["a1"].size(); ++udwIdx)
                {
                    //if(atoi(strMailDocId.c_str()) == EN_MAIL_ID__ACCOUNT_REGISTER)
                    //{
                    //    continue;
                    //}
                    if(tbMail.m_bEx_reward.m_udwNum >= TBMAIL_EX_REWARD_MAX_NUM)
                    {
                        break;
                    }

                    tbMail.m_bEx_reward[tbMail.m_bEx_reward.m_udwNum++].SetValue(
                        oMailJson[strMailDocId]["r"]["a1"][udwIdx][0U].asUInt(),
                        oMailJson[strMailDocId]["r"]["a1"][udwIdx][1U].asUInt(),
                        oMailJson[strMailDocId]["r"]["a1"][udwIdx][2U].asUInt());
                    tbMail.SetFlag(TbMAIL_FIELD_EX_REWARD);
                    bHasReward = TRUE;
                }
            }
        }

        pstSession->ResetAwsInfo();
        pstSession->m_ddwNewMailId = tbMail.m_nId;
        // set data package - mail
        CAwsRequest::UpdateItem(pstSession, &tbMail);

        TbMail_user astMailUserItem[DEFAULT_PERPAGE_NUM * 2];
        TUINT32 udwMailUserNum = 0;
        if(ucSendType == EN_MAIL_SEND_TYPE_SYSTEM_TO_ALLIANCE)
        {
            astMailUserItem[udwMailUserNum].Reset();
            astMailUserItem[udwMailUserNum].Set_Uid(pstSession->m_dwMailReceiverId);
            astMailUserItem[udwMailUserNum].Set_Mid(ddwMailId);
            astMailUserItem[udwMailUserNum].Set_Suid(dwSendUid);
            astMailUserItem[udwMailUserNum].Set_Tuid(pstSession->m_dwMailReceiverId);
            astMailUserItem[udwMailUserNum].Set_Time(tbMail.m_nTime);
            astMailUserItem[udwMailUserNum].Set_Display_type(tbMail.m_nDisplay_type);
            astMailUserItem[udwMailUserNum].Set_Has_reward(bHasReward);
            astMailUserItem[udwMailUserNum].Set_Is_al(TRUE);
            astMailUserItem[udwMailUserNum].Set_Is_single_svr(dwSingleSvr);
            astMailUserItem[udwMailUserNum].Set_Sid(pstSession->m_stReqParam.m_udwSvrId);
            astMailUserItem[udwMailUserNum].Set_Version(pszVersion);
            astMailUserItem[udwMailUserNum].Set_Platform(pszPlatform);
            //CAwsRequest::PutItem(pstSession, &tbMailUserItem);
            udwMailUserNum++;
        }
        else
        {
            for(TUINT32 idx = 0; idx < udwSize; idx++)
            {
                astMailUserItem[udwMailUserNum].Reset();
                astMailUserItem[udwMailUserNum].Set_Uid(audwTid[idx]);
                astMailUserItem[udwMailUserNum].Set_Mid(ddwMailId);
                astMailUserItem[udwMailUserNum].Set_Suid(dwSendUid);
                astMailUserItem[udwMailUserNum].Set_Tuid(audwTid[idx]);
                astMailUserItem[udwMailUserNum].Set_Time(tbMail.m_nTime);
                astMailUserItem[udwMailUserNum].Set_Display_type(tbMail.m_nDisplay_type);
                astMailUserItem[udwMailUserNum].Set_Has_reward(bHasReward);
                astMailUserItem[udwMailUserNum].Set_Is_al(bIsAlRelation);
                astMailUserItem[udwMailUserNum].Set_Is_single_svr(dwSingleSvr);
                astMailUserItem[udwMailUserNum].Set_Sid(pstSession->m_stReqParam.m_udwSvrId);
                astMailUserItem[udwMailUserNum].Set_Version(pszVersion);
                astMailUserItem[udwMailUserNum].Set_Platform(pszPlatform);
                //CAwsRequest::PutItem(pstSession, &tbMailUserItem);
                udwMailUserNum++;
            }
        }

        pstSession->ResetReportInfo();
        CReportSvrRequest::MailUserPut(pstSession, astMailUserItem, udwMailUserNum);

        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__4;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_OpMailSend: insert mail or update stat failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -6;
        }
        return 0;
    }

    if (pstSession->m_udwCommandStep == EN_COMMAND_STEP__4)
    {
        //wave@push_data
        if(ucSendType == EN_MAIL_SEND_TYPE_SYSTEM_TO_ALLIANCE)
        {   
            CPushDataBasic::PushDataAid_Refresh(pstSession, pstSession->m_dwMailReceiverId);
        }
        else
        {
            CPushDataBasic::PushDataUid_Refresh(pstSession, audwTid, udwSize);
        }

        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__REPORT_SVR;
        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendReportSvrRequest(pstSession);
        if (dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("ProcessCmd_OpMailSend: insert mail or update stat failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -6;
        }
        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__END)
    {
        return 0;
    }

    return 0;
}

TINT32 CProcessMailReport::ProcessCmd_AllianceReportGet(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;

    if(pstSession->m_stUserInfo.m_tbPlayer.m_nAlpos == EN_ALLIANCE_POS__REQUEST)
    {
        pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__NOT_IN_ALLIANCE;
        return -1;
    }

    // 0. 输入参数
    TUINT8 ucGetType = atoi(pstSession->m_stReqParam.m_szKey[0]);

    // 1. 发送report id获取请求
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        // set package
        CReportSvrRequest::AllianceReportIdGet(pstSession, ucGetType);

        // send request
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__REPORT_SVR;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendReportSvrRequest(pstSession);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("SendLocalHsRequest: send alliance report get request failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }
        return 0;
    }

    // 2. 解析获取的report id数据
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        // parse data
        SReportUserRspInfo *pstReportUserInfo = &pstSession->m_stUserInfo.m_stReportUserInfo;
        for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecReportRsp.size(); udwIdx++)
        {
            memcpy(pstReportUserInfo, pstSession->m_vecReportRsp[udwIdx]->sRspContent.c_str(),
                pstSession->m_vecReportRsp[udwIdx]->sRspContent.size());
        }

        // set next procedure
        if (pstReportUserInfo->m_udwReportEntryNum == 0)
        {
            pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_AL_REPORT;
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
            return 0;
        }
        else
        {
            pstSession->ResetAwsInfo();
            TbReport tbReportItem;
            for (TUINT32 udwIdx = 0; udwIdx < pstReportUserInfo->m_udwReportEntryNum; udwIdx++)
            {
                tbReportItem.Reset();
                tbReportItem.Set_Id(pstReportUserInfo->m_aReportToReturn[udwIdx].ddwRid);
                CAwsRequest::GetItem(pstSession, &tbReportItem, ETbREPORT_OPEN_TYPE_PRIMARY);
            }
            // send request
            pstSession->m_udwCommandStep = EN_COMMAND_STEP__2;
            pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;
            bNeedResponse = TRUE;
            dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
            if(dwRetCode < 0)
            {
                pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
                TSE_LOG_ERROR(pstSession->m_poServLog, ("SendLocalHsRequest: send user mail get request failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
                return -3;
            }
            return 0;
        }
    }

    // 3. 解析获取report数据
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__2)
    {
        // parse data
        pstSession->m_stUserInfo.m_udwReportNum = 0;
        for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecAwsRsp.size(); udwIdx++)
        {
            dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[udwIdx], &pstSession->m_stUserInfo.m_atbReportList[pstSession->m_stUserInfo.m_udwReportNum]);
            if(dwRetCode > 0)
            {
                pstSession->m_stUserInfo.m_udwReportNum += dwRetCode;
            }
        }
        for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_stUserInfo.m_udwReportNum; udwIdx++)
        {
            pstSession->m_stUserInfo.m_ptbReport[udwIdx] = &pstSession->m_stUserInfo.m_atbReportList[udwIdx];
        }
        // sort
        std::sort(pstSession->m_stUserInfo.m_ptbReport,
            pstSession->m_stUserInfo.m_ptbReport + pstSession->m_stUserInfo.m_udwReportNum, TbReport_Compare_Reverse); //逆序

        // set next procedure
        pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_AL_REPORT;
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_AL_REPORT;
    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TINT32 CProcessMailReport::ProcessCmd_OpReportGet(SSession *pstSession, TBOOL &bNeedResponse)
{
    TINT32 dwRetCode = 0;

    TUINT32 udwReportId = strtoul(pstSession->m_stReqParam.m_szKey[0], NULL, 10);

    // 1. 获取report——请求
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        // get package
        pstSession->ResetAwsReq();

        TbReport tbReportItem;
        tbReportItem.Set_Id(udwReportId);
        CAwsRequest::GetItem(pstSession, &tbReportItem, ETbREPORT_OPEN_TYPE_PRIMARY);

        // set next procedure
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__AWS;

        // send request
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendAwsRequest(pstSession, EN_SERVICE_TYPE_QUERY_DYNAMODB_REQ);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("SendLocalHsRequest: send user report get request failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }
        return 0;
    }

    // 2. 获取report——解析
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        // parse data
        pstSession->m_stUserInfo.m_udwReportNum = 0;
        dwRetCode = CAwsResponse::OnGetItemRsp(*pstSession->m_vecAwsRsp[0], pstSession->m_stUserInfo.m_atbReportList);
        if(dwRetCode > 0)
        {
            pstSession->m_stUserInfo.m_udwReportNum = dwRetCode;
            pstSession->m_stUserInfo.m_ptbReport[0] = &pstSession->m_stUserInfo.m_atbReportList[0];
        }

        // set next procedure
        pstSession->m_stCommonResInfo.m_ucJsonType = EN_JSON_TYPE_REPORT;
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    }

    return 0;
}

bool CProcessMailReport::TbReport_Compare_Reverse(const TbReport *pstA, const TbReport *pstB)
{
    if (pstA->m_nId > pstB->m_nId)
    {
        return true;
    }
    return false;
}

TINT64 CProcessMailReport::GetReportDisplayClass(const TbReport_user& tbReportUser)
{
    if(tbReportUser.m_nReport_type == EN_REPORT_TYPE_OCCUPY)
    {
        return -1 * tbReportUser.m_nReport_type;
    }
    else if (tbReportUser.m_nReport_type == EN_REPORT_TYPE_TRADE)
    {
        return -1 * tbReportUser.m_nReport_type;
    }
    else if (tbReportUser.m_nReport_type == EN_REPORT_TYPE_DRAGON_MONSTER)
    {
        return -1 * tbReportUser.m_nReport_type;
    }
    else if (tbReportUser.m_nReport_type == EN_REPORT_TYPE_TRANSPORT)
    {
        return -1 * tbReportUser.m_nReport_type;
    }
    return tbReportUser.m_nRid;
}

TINT32 CProcessMailReport::SetReportStatus(TINT32 dwStatus, SSession* pstSession, TBOOL& bNeedResponse, TBOOL bClear /*= FALSE*/)
{
    TINT32 dwRetCode = 0;
    TUINT32 udwOpCount = 0;
    TUINT32 audwOptype[MAX_MAIL_PERPAGE_NUM * 2];
    TINT64 addwReportId[MAX_MAIL_PERPAGE_NUM * 2];
    memset((TVOID*)audwOptype, 0, sizeof(TUINT32)*MAX_MAIL_PERPAGE_NUM * 2);
    memset((TVOID*)addwReportId, 0, sizeof(TINT64)*MAX_MAIL_PERPAGE_NUM * 2);

    const TCHAR ucPair = ':';
    const TCHAR ucEntry = ',';

    const TCHAR* pCurPos = pstSession->m_stReqParam.m_szKey[0];
    
    // 1. 发送mail stat获取请求
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        while (pCurPos && *pCurPos)
        {
            audwOptype[udwOpCount] = atoi(pCurPos);
            pCurPos = strchr(pCurPos, ucPair);
            if (pCurPos != NULL)
            {
                pCurPos++;
                addwReportId[udwOpCount] = strtoull(pCurPos, NULL, 10);
                udwOpCount++;
                if (udwOpCount == MAX_MAIL_PERPAGE_NUM * 2)
                {
                    break;
                }
            }
            else
            {
                break;
            }
            pCurPos = strchr(pCurPos, ucEntry);
            if (pCurPos != NULL)
            {
                pCurPos++;
            }
        }

        if (udwOpCount == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -1;
        }

        // set package
        pstSession->ResetReportInfo();
        CReportSvrRequest::UpdateReportUserReq(pstSession, dwStatus, udwOpCount, audwOptype, addwReportId);

        // send request
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__REPORT_SVR;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendReportSvrRequest(pstSession);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("CProcessMailReport::ProcessCmd_ReportGet request failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }
        return 0;
    }

    // 2. 解析获取的mail的id数据
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        SReportUserRspInfo *pstReportUserInfo = &pstSession->m_stUserInfo.m_stReportUserInfo;
        for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecReportRsp.size(); udwIdx++)
        {
            memcpy(pstReportUserInfo, pstSession->m_vecReportRsp[udwIdx]->sRspContent.c_str(),
                pstSession->m_vecReportRsp[udwIdx]->sRspContent.size());
        }
        if (pstReportUserInfo->m_udwReportUnreadNum != pstSession->m_stUserInfo.m_tbUserStat.m_nUnread_report)
        {
            pstSession->m_stUserInfo.m_tbUserStat.Set_Unread_report(pstReportUserInfo->m_udwReportUnreadNum);
        }
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

TVOID CProcessMailReport::AutoSendEncourageMail(SSession *pstSession)
{
    SUserInfo* pstUser = &pstSession->m_stUserInfo;
    TbPlayer* ptbPlayer = &pstUser->m_tbPlayer;
    TbUser_stat* ptbStat = &pstUser->m_tbUserStat;
    SCityInfo *pstCity = &pstUser->m_stCityInfo;

    if(ptbPlayer->m_nUid == 0)
    {
        return;
    }

    const Json::Value& oMailJson = CGameInfo::GetInstance()->m_oJsonRoot["game_encourage_mail"];
    Json::Value::Members member = oMailJson.getMemberNames();

    for(Json::Value::Members::iterator iter = member.begin(); iter != member.end(); ++iter)
    {
        TBOOL bNeedSend = FALSE;
        TINT32 dwCurValue = -1;
        TINT32 dwTargetType = 0, dwTargetId = 0, dwTargtValue = 0;
        string strContent;

        TUINT32 udwMailId = atoi((*iter).c_str());
        dwTargetType = oMailJson[(*iter).c_str()]["o"]["a0"].asInt();
        dwTargetId = oMailJson[(*iter).c_str()]["o"]["a1"].asInt();
        dwTargtValue = oMailJson[(*iter).c_str()]["o"]["a2"].asInt();

        if(BITTEST(ptbStat->m_bMail_flag[0].m_bitFlag, udwMailId) != 0)
        {
            continue;
        }

        switch(dwTargetType)
        {
        case EN_MAIL_TYPE_BUILDING_UPGRADE:
            dwCurValue = CCityBase::GetBuildingLevelById(&pstCity->m_stTblData, dwTargetId);
            break;
        case EN_MAIL_TYPE_AGE:
            dwCurValue = ptbPlayer->m_nAge;
            dwTargtValue = dwTargetId;
            break;
        case EN_MAIL_TYPE_DRAGON_LV:
            dwCurValue = ptbPlayer->m_nDragon_level;
            break;
        case EN_MAIL_TYPE_VIP_LV:
            dwCurValue = CPlayerBase::GetVipLevel(ptbPlayer);
            break;
        case EN_MAIL_TYPE_NOTHING:
            if(udwMailId == EN_MAIL_ID__WELCOME_TO_GAME)
            {
                if(BITTEST(ptbStat->m_bMail_flag[0].m_bitFlag, EN_MAIL_ID__WELCOME_TO_GAME) == 0)
                {
                    bNeedSend = TRUE;
                }
            }
            else if(udwMailId == EN_MAIL_ID__FIRST_JOIN_ALLIANCE)
            {
                //第一次加入联盟
                if(ptbPlayer->m_nAlid != 0
                && ptbPlayer->m_nAlpos > EN_ALLIANCE_POS__REQUEST
                && pstUser->m_tbLogin.m_nAl_time == 0
                && (0 != strcmp(pstSession->m_stReqParam.m_szCommand, "al_create")))
                {
                    TUINT32 udwTime = CTimeUtils::GetUnixTime();
                    if(ptbPlayer->m_nAl_time > 0)
                    {
                        udwTime = ptbPlayer->m_nAl_time;
                    }
                    pstUser->m_tbLogin.Set_Al_time(udwTime);
                    strContent = pstUser->m_tbPlayer.m_sAlname;
                    bNeedSend = TRUE;
                }
            }
            else if(udwMailId == EN_MAIL_ID__FIRST_CREATE_ALLIANCE)
            {
                //第一次创建联盟
                if(ptbPlayer->m_nAlpos == EN_ALLIANCE_POS__CHANCELLOR
                && (0 == strcmp(pstSession->m_stReqParam.m_szCommand, "al_create"))
                && pstUser->m_tbLogin.m_nAl_time == 0)
                {
                    TUINT32 udwTime = CTimeUtils::GetUnixTime();
                    if(ptbPlayer->m_nAl_time > 0)
                    {
                        udwTime = ptbPlayer->m_nAl_time;
                    }
                    pstUser->m_tbLogin.Set_Al_time(udwTime);
                    strContent = pstUser->m_tbPlayer.m_sAlname;
                    bNeedSend = TRUE;
                }
            }
            else if(udwMailId == EN_MAIL_ID__PROTECTION_MODE_EXPIRED)
            {
                if((ptbPlayer->m_nStatus & EN_CITY_STATUS__NEW_PROTECTION) == 0
                    && BITTEST(ptbStat->m_bMail_flag[0].m_bitFlag, EN_MAIL_ID__PROTECTION_MODE_EXPIRED) == 0)
                {
                    bNeedSend = TRUE;
                }
            }
            else if (udwMailId == EN_MAIL_ID__FIRST_T3_TROOP_UNLOCK)
            {
                if (pstCity->m_stTblData.m_bResearch[0].m_addwLevel[19] > 0)
                {
                    bNeedSend = TRUE;
                    strContent = "8";
                }
                else if (pstCity->m_stTblData.m_bResearch[0].m_addwLevel[37] > 0)
                {
                    bNeedSend = TRUE;
                    strContent = "9";
                }
                else if (pstCity->m_stTblData.m_bResearch[0].m_addwLevel[55] > 0)
                {
                    bNeedSend = TRUE;
                    strContent = "10";
                }
                else if (pstCity->m_stTblData.m_bResearch[0].m_addwLevel[74] > 0)
                {
                    bNeedSend = TRUE;
                    strContent = "11";
                }
            }
            else if (udwMailId == EN_MAIL_ID__FIRST_T4_TROOP_UNLOCK)
            {
                if (pstCity->m_stTblData.m_bResearch[0].m_addwLevel[20] > 0)
                {
                    bNeedSend = TRUE;
                    strContent = "12";
                }
                else if (pstCity->m_stTblData.m_bResearch[0].m_addwLevel[38] > 0)
                {
                    bNeedSend = TRUE;
                    strContent = "13";
                }
                else if (pstCity->m_stTblData.m_bResearch[0].m_addwLevel[56] > 0)
                {
                    bNeedSend = TRUE;
                    strContent = "14";
                }
                else if (pstCity->m_stTblData.m_bResearch[0].m_addwLevel[75] > 0)
                {
                    bNeedSend = TRUE;
                    strContent = "15";
                }
            }
            else if (udwMailId == EN_MAIL_ID__FIRST_T4_FORT_UNLOCK)
            {
                if (pstCity->m_stTblData.m_bResearch[0].m_addwLevel[90] > 0)
                {
                    bNeedSend = TRUE;
                    strContent = "10";
                }
                else if (pstCity->m_stTblData.m_bResearch[0].m_addwLevel[91] > 0)
                {
                    bNeedSend = TRUE;
                    strContent = "11";
                }
                else if (pstCity->m_stTblData.m_bResearch[0].m_addwLevel[92] > 0)
                {
                    bNeedSend = TRUE;
                    strContent = "9";
                }
            }
            else if (udwMailId == EN_MAIL_ID__RESEARCH_ALL_ECONOMICS)
            {
                bNeedSend = TRUE;
                const Json::Value &jResearch = CGameInfo::GetInstance()->m_oJsonRoot["game_research"];
                Json::Value::Members members = jResearch.getMemberNames();
                for (Json::Value::Members::iterator it = members.begin(); it != members.end(); it++)
                {
                    if (jResearch[*it]["a"]["a2"].asInt() == 1)
                    {
                        if (pstCity->m_stTblData.m_bResearch[0].m_addwLevel[atoi(it->c_str())] == 0)
                        {
                            bNeedSend = FALSE;
                            break;
                        }
                    }
                }
            }
            else if (udwMailId == EN_MAIL_ID__RESEARCH_ALL_HERO)
            {
                bNeedSend = TRUE;
                const Json::Value &jResearch = CGameInfo::GetInstance()->m_oJsonRoot["game_research"];
                Json::Value::Members members = jResearch.getMemberNames();
                for (Json::Value::Members::iterator it = members.begin(); it != members.end(); it++)
                {
                    if (jResearch[*it]["a"]["a2"].asInt() == 7)
                    {
                        if (pstCity->m_stTblData.m_bResearch[0].m_addwLevel[atoi(it->c_str())] == 0)
                        {
                            bNeedSend = FALSE;
                            break;
                        }
                    }
                }
            }
            break;
        default:
            bNeedSend = FALSE;
            break;
        }

        if(dwCurValue >= dwTargtValue)
        {
            bNeedSend = TRUE;
        }

        if(bNeedSend)
        {
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("AutoSendEncourageMail:uid=%u mail_id=%u type=%u id=%u value=%u cur_value=%d [seq=%u]",
                ptbPlayer->m_nUid, udwMailId, dwTargetType, dwTargetId, dwTargtValue, dwCurValue, pstUser->m_udwBSeqNo));

            CMsgBase::SendEncourageMail(ptbStat, ptbPlayer->m_nSid, udwMailId, 0, strContent);
        }
    }
}

TINT32 CProcessMailReport::GetEventMailId(TUINT32 udwEventType, TUINT32 udwRankOrGoal, TUINT32 udwPersonOrAlliance)
{
    //udwPersonOrAlliance :0 个人 :1联盟
    //udwRankOrGoal :0 goal 1:rank
    TINT32 dwMailId = -1;

    if(udwPersonOrAlliance == 0)
    {
        switch(udwEventType)
        {
        case EN_EVENT_TYPE__PERSONAL_2:
            dwMailId = udwRankOrGoal == 0 ? EN_MAIL_ID__PERSON_GOAL : EN_MAIL_ID__PERSON_RANK;
            break;
        case EN_EVENT_TYPE__PERSONAL_1:
            dwMailId = udwRankOrGoal == 0 ? EN_MAIL_ID__HARD_PERSON_GOAL : EN_MAIL_ID__HARD_PERSON_RANK;
            break;
        case EN_EVENT_TYPE__THEME:
            dwMailId = udwRankOrGoal == 0 ? EN_MAIL_ID__THEME_EVENT_GOAL : EN_MAIL_ID__THEME_EVENT_RANK;
            break;
        default:
            break;
        }
    }
    else if(udwPersonOrAlliance == 1)
    {
        switch(udwEventType)
        {
        case EN_EVENT_TYPE__ALLIANCE:
            dwMailId = udwRankOrGoal == 0 ? EN_MAIL_ID__ALLIANCE_GOAL : EN_MAIL_ID__ALLIANCE_RANK;
            break;
        case EN_EVENT_TYPE__THEME:
            dwMailId = udwRankOrGoal == 0 ? EN_MAIL_ID__THEME_EVENT_GOAL : EN_MAIL_ID__THEME_EVENT_RANK;
            break;
        default:
            break;
        }

    }
    return dwMailId;
}

TINT32 CProcessMailReport::GenEventMail(TINT32 dwSvrId, TUINT64 uddwUid, TUINT32 udwEventType, TUINT32 udwRankOrGoal, TINT32 dwKey,
    TUINT32 udwPersonOrAlliance, string sMsgAdd1, string sMsgAdd2, TBOOL bIsClaim, TUINT32 udwPoint,
    /*SGlobalRes *pstGlobalRes, */
    SOneGlobalRes *astRewardList, TUINT32 udwRewardNum, 
    string sLang, TINT64 ddwMailId, TINT64 ddwEventType, TINT64 ddwEventScore,
    TbMail *ptbMail, TbMail_user *ptbMailUser, const string sEventInfo)
{
    TUINT32 udwKingdoms = 0;    //是否跨服活动
    TUINT32 udwEventUI = 0;
    TUINT64 uddwEventId = 0;
    string strEventTitle;   //活动标题
    if (sLang == "")
    {
        sLang = "english";
    }

    TINT32 dwMialId = GetEventMailId(udwEventType, udwRankOrGoal, udwPersonOrAlliance);

    TBOOL bIsExistDocument = CDocument::GetInstance()->IsSupportLang(sLang);
    if (FALSE == bIsExistDocument)
    {
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("SendItemMail: do not have this language[%s] document", sLang.c_str()));

        bIsExistDocument = CDocument::GetInstance()->IsSupportLang("english");
        if (FALSE == bIsExistDocument)
        {
            TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("SendItemMail: do not have english document"));
            return -1;
        }
        else
        {
            sLang = "english";
        }
    }

    const Json::Value &stDocumentJson = CDocument::GetInstance()->GetDocumentJsonByLang(sLang);

    string sRankInfo = "";
    string sItemList = "";
    string sMailContext = CMsgBase::GetEventMailContentByEventId(dwMialId, stDocumentJson);
    string sMailTitle = CMsgBase::GetEventMailTitleByEventId(dwMialId, stDocumentJson);

    ostringstream oss;
    //udwRankOrGoal :0 goal 1:rank
    if (udwRankOrGoal == 1)
    {
        std::vector<string> vecOneRankName;
        std::vector<string> vecRankSvrList;
        CCommonFunc::GetVectorFromString(sMsgAdd1.c_str(), ':', vecOneRankName);
        CCommonFunc::GetVectorFromString(sMsgAdd2.c_str(), ':', vecRankSvrList);

        sRankInfo += "\n";
        for (TUINT32 udwIdx = 0; udwIdx < vecOneRankName.size() && udwIdx < vecRankSvrList.size() && udwIdx < 3/*产品需求, 只展示前三名*/; ++udwIdx)
        {
            sRankInfo += "Rank " + CCommonFunc::NumToString(udwIdx + 1) + " " + vecOneRankName[udwIdx] + " ("
                + CMsgBase::GetSvrNameBySvrId(vecRankSvrList[udwIdx], stDocumentJson) + ")\n";
        }
        sMailContext += sRankInfo;
    }

    oss.str("");
    string strPoint = CProcessMailReport::NumToSplitString(udwPoint);
    CMsgBase::StringReplace(sMailContext, "STRING0", strPoint);

    oss.str("");
    oss << dwKey;
    CMsgBase::StringReplace(sMailContext, "STRING1", oss.str());

    TBOOL bHasReward = FALSE;
    TBOOL bIsAlRelation = FALSE;

    Json::Reader reader;
    Json::Value jEventInfo;
    jEventInfo.clear();
    TBOOL bPase = reader.parse(sEventInfo.c_str(), jEventInfo);
    if (bPase)
    {
        uddwEventId = jEventInfo["e_id"].asUInt64();
    }

    ptbMail->Set_Id(ddwMailId);
    ptbMail->Set_Raw_lang(1);
    ptbMail->Set_Time(CTimeUtils::GetUnixTime());
    ptbMail->Set_Suid(SYSTEM_EVENT);
    ptbMail->Set_Sender("Event Rewards");
    ptbMail->Set_Ruid(uddwUid);
    ptbMail->Set_Send_type(EN_MAIL_SEND_TYPE_SYSTEM_TO_PLAYERS);
    ptbMail->Set_Title(sMailTitle);
    ptbMail->Set_Content(sMailContext);
    ptbMail->Set_Maildocid(0);
    ptbMail->Set_Event_id(uddwEventId);
    ptbMail->Set_Event_type(ddwEventType);
    ptbMail->Set_Event_score(ddwEventScore);
    ptbMail->Set_Display_type(EN_MAIL_DISPLAY_TYPE_SYSTEM_EVENT);
    if (udwEventType == EN_EVENT_TYPE__THEME && bPase)
    {
        Json::FastWriter tmpWriter;
        Json::Value jsonEscs = Json::Value(Json::arrayValue);
        Json::Value jLocalization = jEventInfo["localization"];
        jsonEscs.append(jLocalization);
        jsonEscs.append(jEventInfo["kingdoms"].asUInt());
        jsonEscs.append(jEventInfo["event_ui"].asUInt());
        ptbMail->Set_Escs(tmpWriter.write(jsonEscs));
        TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("GenEventMail: [ESCS=%s]", ptbMail->m_sEscs.c_str()));
    }
//     ptbMail->m_bReward[0].ddwTotalNum = pstGlobalRes->ddwTotalNum;
//     for (TUINT32 udwIdx = 0; udwIdx < pstGlobalRes->ddwTotalNum; ++udwIdx)
//     {
//         ptbMail->m_bReward[0].aRewardList[udwIdx].ddwType = pstGlobalRes->aRewardList[udwIdx].ddwType;
//         ptbMail->m_bReward[0].aRewardList[udwIdx].ddwId = pstGlobalRes->aRewardList[udwIdx].ddwId;
//         ptbMail->m_bReward[0].aRewardList[udwIdx].ddwNum = pstGlobalRes->aRewardList[udwIdx].ddwNum;
//     }
// 
//     ptbMail->SetFlag(TbMAIL_FIELD_REWARD);

    for (TUINT32 udwIdx = 0; udwIdx < udwRewardNum; ++udwIdx)
    {
        ptbMail->m_bEx_reward[udwIdx].ddwType = astRewardList[udwIdx].ddwType;
        ptbMail->m_bEx_reward[udwIdx].ddwId = astRewardList[udwIdx].ddwId;
        ptbMail->m_bEx_reward[udwIdx].ddwNum = astRewardList[udwIdx].ddwNum;
    }
    ptbMail->m_bEx_reward.m_udwNum = udwRewardNum;
    ptbMail->SetFlag(TbMAIL_FIELD_EX_REWARD);

    ptbMailUser->Set_Uid(uddwUid);
    ptbMailUser->Set_Tuid(uddwUid);
    ptbMailUser->Set_Suid(SYSTEM_EVENT);
    ptbMailUser->Set_Mid(ddwMailId);
    ptbMailUser->Set_Time(ptbMail->m_nTime);
    ptbMailUser->Set_Display_type(ptbMail->m_nDisplay_type);
//    ptbMailUser->Set_Has_reward(pstGlobalRes->ddwTotalNum > 0 ? TRUE : FALSE);
    ptbMailUser->Set_Has_reward(ptbMail->m_bEx_reward.m_udwNum > 0 ? TRUE : FALSE);
    ptbMailUser->Set_Sid(dwSvrId);

    return 0;
}

TINT32 CProcessMailReport::SendEventMail(TINT32 dwSvrId, TUINT64 uddwUid, TUINT32 udwEventType, TUINT32 udwRankOrGoal, TINT32 dwKey,
    TUINT32 udwPersonOrAlliance, string sMsgAdd1, string sMsgAdd2, TBOOL bIsClaim,TUINT32 udwPoint,
    SGlobalRes *pstGlobalRes /*= NULL */, string sLang/* = english*/)
{
    TINT32 dwMialId = GetEventMailId(udwEventType, udwRankOrGoal, udwPersonOrAlliance);

    TBOOL bIsExistDocument = CDocument::GetInstance()->IsSupportLang(sLang);
    if(FALSE == bIsExistDocument)
    {
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("SendItemMail: do not have this language[%s] document", sLang.c_str()));

        bIsExistDocument = CDocument::GetInstance()->IsSupportLang("english");
        if(FALSE == bIsExistDocument)
        {
            TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("SendItemMail: do not have english document"));
            return -1;
        }
        else
        {
            sLang = "english";
        }
    }

    const Json::Value &stDocumentJson = CDocument::GetInstance()->GetDocumentJsonByLang(sLang);

    string sRankInfo = "";
    string sItemList = "";
    string sMailContext = CMsgBase::GetEventMailContentByEventId(dwMialId, stDocumentJson);
    string sMailTitle = CMsgBase::GetEventMailTitleByEventId(dwMialId, stDocumentJson);

    ostringstream oss;
    //udwRankOrGoal :0 goal 1:rank
    if(udwRankOrGoal == 1)
    {
        std::vector<string> vecOneRankName;
        std::vector<string> vecRankSvrList;
        CCommonFunc::GetVectorFromString(sMsgAdd1.c_str(), ':', vecOneRankName);
        CCommonFunc::GetVectorFromString(sMsgAdd2.c_str(), ':', vecRankSvrList);

        sRankInfo += "\n";
        for(TUINT32 udwIdx = 0; udwIdx < vecOneRankName.size() && udwIdx < vecRankSvrList.size() && udwIdx < 3/*产品需求, 只展示前三名*/; ++udwIdx)
        {
            sRankInfo += "Rank " + CCommonFunc::NumToString(udwIdx + 1) + " " + vecOneRankName[udwIdx] + " ("
                + CMsgBase::GetSvrNameBySvrId(vecRankSvrList[udwIdx], stDocumentJson) + ")\n";
        }
        sMailContext += sRankInfo;
    }
    
    oss.str("");
    string strPoint = CProcessMailReport::NumToSplitString(udwPoint);
    CMsgBase::StringReplace(sMailContext, "STRING0", strPoint);
    
    oss.str("");
    oss << dwKey;
    CMsgBase::StringReplace(sMailContext, "STRING1", oss.str());

    oss.str("");
    if(pstGlobalRes!=NULL)
    {
        oss << "[";
        for(TUINT32 udwIdx = 0; udwIdx < pstGlobalRes->ddwTotalNum; ++udwIdx)
        {
            if(udwIdx != 0)
            {
                oss << ",";
            }
            oss << "[" << pstGlobalRes->aRewardList[udwIdx].ddwType << "," << pstGlobalRes->aRewardList[udwIdx].ddwId << "," << pstGlobalRes->aRewardList[udwIdx].ddwNum << "]";
        }
        oss << "]";
    }
   
    const TINT32 k_tmp_len = 4096;
    TCHAR szScriptStr[k_tmp_len];

    TCHAR szTmpStr[k_tmp_len];
    CCommonFunc::ShellEncode(sMailContext.c_str(), szTmpStr, k_tmp_len);

    sprintf(szScriptStr, "./send_operate_mail.sh \"%u\" \"%d\" \"%ld\" \"%s\" \"%d\" \"%s\" \"%s\" \"%u\" \"%s\"",
        dwSvrId,
        EN_MAIL_SEND_TYPE_SYSTEM_TO_PLAYERS,
        uddwUid,
        sMailTitle.c_str(),
        0,
        szTmpStr,
        "",
        SYSTEM_EVENT,
        oss.str().c_str());
    CMsgBase::SendDelaySystemMsg(szScriptStr);

    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("SendEventMail:: szScriptStr=%s ", szScriptStr));

    return 0;
}

TINT32 CProcessMailReport::SendEventMailToAlliance(TINT32 dwSvrId, TUINT64 uddAid, TUINT32 udwEventType, TUINT32 udwRankOrGoal, TINT32 dwKey, TUINT32 udwPersonOrAlliance, string sMsgAdd1, string sMsgAdd2, TBOOL bIsClaim, TUINT32 udwPoint, SGlobalRes *pstGlobalRes /*= NULL*/, string sLang /*= "english"*/)
{
    TINT32 dwMialId = GetEventMailId(udwEventType, udwRankOrGoal, udwPersonOrAlliance);

    TBOOL bIsExistDocument = CDocument::GetInstance()->IsSupportLang(sLang);
    if(FALSE == bIsExistDocument)
    {
        TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("SendItemMail: do not have this language[%s] document", sLang.c_str()));

        bIsExistDocument = CDocument::GetInstance()->IsSupportLang("english");
        if(FALSE == bIsExistDocument)
        {
            TSE_LOG_ERROR(CGameInfo::GetInstance()->m_poLog, ("SendItemMail: do not have english document"));
            return -1;
        }
        else
        {
            sLang = "english";
        }
    }

    const Json::Value &stDocumentJson = CDocument::GetInstance()->GetDocumentJsonByLang(sLang);

    string sRankInfo = "";
    string sItemList = "";
    string sMailContext = CMsgBase::GetEventMailContentByEventId(dwMialId, stDocumentJson);
    string sMailTitle = CMsgBase::GetEventMailTitleByEventId(dwMialId, stDocumentJson);

    ostringstream oss;
    //udwRankOrGoal :0 goal 1:rank
    if(udwRankOrGoal == 1)
    {
        std::vector<string> vecOneRankName;
        std::vector<string> vecRankSvrList;
        CCommonFunc::GetVectorFromString(sMsgAdd1.c_str(), ':', vecOneRankName);
        CCommonFunc::GetVectorFromString(sMsgAdd2.c_str(), ':', vecRankSvrList);

        sRankInfo += "\n";
        for(TUINT32 udwIdx = 0; udwIdx < vecOneRankName.size() && udwIdx < vecRankSvrList.size(); ++udwIdx)
        {
            sRankInfo += "Rank " + CCommonFunc::NumToString(udwIdx + 1) + " " + vecOneRankName[udwIdx] + " ("
                + CMsgBase::GetSvrNameBySvrId(vecRankSvrList[udwIdx], stDocumentJson) + ")\n";
        }
        sMailContext += sRankInfo;
    }

    oss.str("");
    string strPoint = CProcessMailReport::NumToSplitString(udwPoint);
    CMsgBase::StringReplace(sMailContext, "STRING0", strPoint);

    oss.str("");
    oss << dwKey;
    CMsgBase::StringReplace(sMailContext, "STRING1", oss.str());

    oss.str("");
    if(pstGlobalRes != NULL)
    {
        oss << "[";
        for(TUINT32 udwIdx = 0; udwIdx < pstGlobalRes->ddwTotalNum; ++udwIdx)
        {
            if(udwIdx != 0)
            {
                oss << ",";
            }
            oss << "[" << pstGlobalRes->aRewardList[udwIdx].ddwType << "," << pstGlobalRes->aRewardList[udwIdx].ddwId << "," << pstGlobalRes->aRewardList[udwIdx].ddwNum << "]";
        }
        oss << "]";
    }

    const TINT32 k_tmp_len = 4096;
    TCHAR szScriptStr[k_tmp_len];

    TCHAR szTmpStr[k_tmp_len];
    CCommonFunc::ShellEncode(sMailContext.c_str(), szTmpStr, k_tmp_len);

    sprintf(szScriptStr, "./send_operate_mail.sh \"%u\" \"%d\" \"%ld\" \"%s\" \"%d\" \"%s\" \"%s\" \"%u\" \"%s\"",
        dwSvrId,
        EN_MAIL_SEND_TYPE_SYSTEM_TO_ALLIANCE,
        uddAid,
        sMailTitle.c_str(),
        0,
        szTmpStr,
        "",
        SYSTEM_EVENT,
        oss.str().c_str());
    CMsgBase::SendDelaySystemMsg(szScriptStr);

    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("SendEventMailToAlliance:: szScriptStr=%s ", szScriptStr));

    return 0;
}

/************************************************private**********************************************/

TINT64 CProcessMailReport::GetMailDisplayClass(const TbMail_user& tbMailUser)
{
    if(tbMailUser.m_nDisplay_type == EN_MAIL_DISPLAY_TYPE_ALLIANCE)//(-1000000, -1]
    {
        return tbMailUser.m_nTuid;
    }
    else if(tbMailUser.m_nDisplay_type == EN_MAIL_DISPLAY_TYPE_NORMAL)//[1, +oo)
    {
        if(tbMailUser.m_nUid == tbMailUser.m_nSuid)
        {
            return tbMailUser.m_nTuid;
        }
        return tbMailUser.m_nSuid;
    }
    else if(tbMailUser.m_nDisplay_type == EN_MAIL_DISPLAY_TYPE_ALLIANCE_INVITE
         || tbMailUser.m_nDisplay_type == EN_MAIL_DISPLAY_TYPE_ALLIANCE_DIPLOMACY)
    {
        return tbMailUser.m_nSuid;
    }
    else if(tbMailUser.m_nDisplay_type == EN_MAIL_DISPLAY_TYPE_SYSTEM_ENCOURAGE)//(-oo, -2000000)
    {
        return (-1 * tbMailUser.m_nMid) + SYSTEM_END;
    }
    else if (tbMailUser.m_nDisplay_type == EN_MAIL_DISPLAY_TYPE_SUPPORT_HELP)
    {
        return SYSTEM_SUPPORT;  //-1000010
    }
    return tbMailUser.m_nSuid;
}

TINT32 CProcessMailReport::SetMailStatus(TINT32 dwStatus, SSession* pstSession, TBOOL& bNeedResponse, TBOOL bClear)
{
    TINT32 dwRetCode = 0;
    
    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__INIT)
    {
        TUINT32 udwOpCount = 0;
        TUINT32 audwOptype[MAX_MAIL_PERPAGE_NUM * 2];
        TINT64 addwMailId[MAX_MAIL_PERPAGE_NUM * 2];
        memset((TVOID*)audwOptype, 0, sizeof(TUINT32)*MAX_MAIL_PERPAGE_NUM * 2);
        memset((TVOID*)addwMailId, 0, sizeof(TINT64)*MAX_MAIL_PERPAGE_NUM * 2);

        const TCHAR ucPair = ':';
        const TCHAR ucEntry = ',';

        const TCHAR* pCurPos = pstSession->m_stReqParam.m_szKey[0];
        while (pCurPos && *pCurPos)
        {
            audwOptype[udwOpCount] = atoi(pCurPos);
            pCurPos = strchr(pCurPos, ucPair);
            if (pCurPos != NULL)
            {
                pCurPos++;
                addwMailId[udwOpCount] = strtoull(pCurPos, NULL, 10);
                udwOpCount++;
                if (udwOpCount == MAX_MAIL_PERPAGE_NUM * 2)
                {
                    break;
                }
            }
            else
            {
                break;
            }
            pCurPos = strchr(pCurPos, ucEntry);
            if (pCurPos != NULL)
            {
                pCurPos++;
            }
        }

        if (udwOpCount == 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__REQ_PARAM_ERROR;
            return -1;
        }

        TSE_LOG_DEBUG(pstSession->m_poServLog, ("CProcessMailReport::SetMailStatus key=[%s][seq=%u]",
            pstSession->m_stReqParam.m_szKey[0], pstSession->m_udwSeqNo));

        for(TUINT32 udwIdx = 0; udwIdx < udwOpCount; ++udwIdx)
        {
            TSE_LOG_DEBUG(pstSession->m_poServLog, ("CProcessMailReport::SetMailStatus key=[%u,%ld][seq=%u]",
                audwOptype[udwIdx], addwMailId[udwIdx], pstSession->m_udwSeqNo));
        }

        // set package
        pstSession->ResetReportInfo();
        CReportSvrRequest::SetMailStatus(pstSession, dwStatus, bClear, pstSession->m_stReqParam.m_szKey[0]);

        // send request
        pstSession->m_udwCommandStep = EN_COMMAND_STEP__1;
        pstSession->m_udwExpectProcedure = EN_EXPECT_PROCEDURE__REPORT_SVR;
        bNeedResponse = TRUE;
        dwRetCode = CBaseProcedure::SendReportSvrRequest(pstSession);
        if(dwRetCode < 0)
        {
            pstSession->m_stCommonResInfo.m_dwRetCode = EN_RET_CODE__SEND_REQ_FAILED;
            TSE_LOG_ERROR(pstSession->m_poServLog, ("CProcessMailReport::ProcessCmd_MailGet request failed [seq=%u]", pstSession->m_stUserInfo.m_udwBSeqNo));
            return -2;
        }
        return 0;
    }

    if(pstSession->m_udwCommandStep == EN_COMMAND_STEP__1)
    {
        SMailUserRspInfo *pstMailUserInfo = &pstSession->m_stUserInfo.m_stMailUserInfo;
        for (TUINT32 udwIdx = 0; udwIdx < pstSession->m_vecReportRsp.size(); udwIdx++)
        {
            dwRetCode = pstSession->m_vecReportRsp[udwIdx]->dwRetCode;
            memcpy(pstMailUserInfo, pstSession->m_vecReportRsp[udwIdx]->sRspContent.c_str(),
                pstSession->m_vecReportRsp[udwIdx]->sRspContent.size());
        }

        if (pstMailUserInfo->m_udwMailUnreadNum != 0)
        {
            pstSession->m_stUserInfo.m_tbUserStat.Set_Unread_mail(pstSession->m_stUserInfo.m_tbUserStat.m_nUnread_mail - pstMailUserInfo->m_udwMailUnreadNum);
            if (pstSession->m_stUserInfo.m_tbUserStat.m_nUnread_mail < 0)
            {
                pstSession->m_stUserInfo.m_tbUserStat.m_nUnread_mail = 0;
            }
        }

        if (dwStatus == EN_MAIL_STATUS_DEL)
        {
            TINT32 dwTipsType = EN_TIPS_TYPE__DEL_MAIL_OK;
            if (dwRetCode != 200)
            {
                dwTipsType = EN_TIPS_TYPE__DEL_MAIL_FAIL;
            }
            CSendMessageBase::AddTips(&pstSession->m_stUserInfo, dwTipsType, pstSession->m_stReqParam.m_udwUserId, FALSE);
        }

        pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
        return 0;
    }

    pstSession->m_udwCommandStep = EN_COMMAND_STEP__END;
    return 0;
}

string CProcessMailReport::NumToSplitString(TINT64 ddwNum)
{
    ostringstream oss;
    oss.str("");
    //三位一个逗号
    TUINT64 uddwNumber = ddwNum;
    TINT32 dwCount = 0;
    do
    {
        dwCount++;
        uddwNumber /= 1000;
    } while(uddwNumber);
    uddwNumber = ddwNum;
    TBOOL bBegin = TRUE;
    for(TINT32 dwIdx = dwCount - 1; dwIdx > 0; dwIdx--)
    {
        TUINT64 uddwTimes = 1;
        for(TINT32 dwIdy = 0; dwIdy < dwIdx; dwIdy++)
        {
            uddwTimes *= 1000;
        }
        if(bBegin)
        {
            oss << (uddwNumber / uddwTimes);
        }
        else
        {
            oss << setw(3) << setfill('0') << (uddwNumber / uddwTimes);
        }
        bBegin = FALSE;
        oss << ",";
        uddwNumber %= uddwTimes;
    }
    if(bBegin)
    {
        oss << uddwNumber;
    }
    else
    {
        oss << setw(3) << setfill('0') << uddwNumber;
    }

    return oss.str();
}

string CProcessMailReport::GetMailTitle(SSession* pstSession, TINT32 dwDocId)
{
    string sLang = CDocument::GetLang(pstSession->m_stUserInfo.m_tbLogin.m_nLang);
    if (sLang == "")
    {
        sLang = "english";
    }

    TBOOL bIsExistDocument = CDocument::GetInstance()->IsSupportLang(sLang);
    if (FALSE == bIsExistDocument)
    {
        TSE_LOG_ERROR(pstSession->m_poServLog, ("SendItemMail: do not have this language[%s] document", sLang.c_str()));

        bIsExistDocument = CDocument::GetInstance()->IsSupportLang("english");
        if (FALSE == bIsExistDocument)
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("SendItemMail: do not have english document"));
            return;
        }
        else
        {
            sLang = "english";
        }
    }

    const Json::Value &stDocumentJson = CDocument::GetInstance()->GetDocumentJsonByLang(sLang);

    string sRankInfo = "";
    string sItemList = "";
    string sMailTitle = CMsgBase::GetEventMailTitleByEventId(dwDocId, stDocumentJson);
    return sMailTitle;
}