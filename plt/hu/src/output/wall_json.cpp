#include "wall_json.h"
#include "game_info.h"
#include "common_base.h"
#include "common_json.h"
#include "common_func.h"
#include "quest_logic.h"
#include "document.h"

CWallJson::CWallJson()
{

}

CWallJson::~CWallJson()
{

}

TVOID CWallJson::GenDataJson(SSession* pstSession, Json::Value& rJson)
{
    //"svr_wall_msg":
    //[
    //    [
    //        1473447638348728,   // msg id 64 int
    //        1405189169,         // unix time
    //        10,                 // aid
    //        5,                  // alpos
    //        275384,             // uid
    //        "Drageda",          // uname
    //        "Content",          // content
    //        0                   // hero avatar
    //    ]
    //]
    Json::Value& rJsonWallMsg = rJson["svr_wall_msg"];
    rJsonWallMsg = Json::Value(Json::arrayValue);
    Json::Value rJsonWallMsgTranslate = Json::Value(Json::objectValue);
    
    // 记录al_comment的删除信息
    map<TINT64, TINT32> mapAlComment;
    for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_stUserInfo.m_udwWallNum; ++udwIdx)
    {
        mapAlComment.insert(make_pair(pstSession->m_stUserInfo.m_atbWall[udwIdx].m_nWall_id, pstSession->m_stUserInfo.m_aucWallFlag[udwIdx]));
    }
    

    // 对al_comment进行排序
    for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_stUserInfo.m_udwWallNum; ++udwIdx)
    {
        
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("CWallJson_begin: msgid=%lld, seq=%u", \
                                                pstSession->m_stUserInfo.m_atbWall[udwIdx].m_nWall_id, \
                                                pstSession->m_udwSeqNo));
    }
    
    std::sort(pstSession->m_stUserInfo.m_atbWall, pstSession->m_stUserInfo.m_atbWall + pstSession->m_stUserInfo.m_udwWallNum, CWallJson::TbAlComment_Compare);

    for(TUINT32 udwIdx = 0; udwIdx < pstSession->m_stUserInfo.m_udwWallNum; ++udwIdx)
    {
        
        TSE_LOG_DEBUG(pstSession->m_poServLog, ("CWallJson_end: msgid=%lld, seq=%u", \
                                                pstSession->m_stUserInfo.m_atbWall[udwIdx].m_nWall_id, \
                                                pstSession->m_udwSeqNo));
    }
    
    
    ostringstream ossTranslateContent;
    ossTranslateContent.str("");

    map<TINT64, TINT32>::iterator it_AlComment;
    for(TUINT32 udwIdx = 0, udwJsonIndex = 0; udwIdx < pstSession->m_stUserInfo.m_udwWallNum; ++udwIdx)
    {
        it_AlComment = mapAlComment.find(pstSession->m_stUserInfo.m_atbWall[udwIdx].m_nWall_id);
        if(it_AlComment == mapAlComment.end())
        {
            TSE_LOG_ERROR(pstSession->m_poServLog, ("CWallJson: can not find the al_comment=%lld, seq=%u", \
                                                    pstSession->m_stUserInfo.m_atbWall[udwIdx].m_nWall_id, \
                                                    pstSession->m_udwSeqNo));
            continue;
        }        
        else
        {
            if(EN_TABLE_UPDT_FLAG__DEL == it_AlComment->second)
            {   
                TSE_LOG_DEBUG(pstSession->m_poServLog, ("CWallJson: delete_msgid=%lld, seq=%u", \
                                                        pstSession->m_stUserInfo.m_atbWall[udwIdx].m_nWall_id, \
                                                        pstSession->m_udwSeqNo));
                continue;
            }
        }
    
            
        rJsonWallMsgTranslate.clear();
        
        rJsonWallMsg[udwJsonIndex] = Json::Value(Json::arrayValue);
        rJsonWallMsg[udwJsonIndex].append(pstSession->m_stUserInfo.m_atbWall[udwIdx].m_nWall_id);
        rJsonWallMsg[udwJsonIndex].append(pstSession->m_stUserInfo.m_atbWall[udwIdx].m_nTime);
        rJsonWallMsg[udwJsonIndex].append(pstSession->m_stUserInfo.m_atbWall[udwIdx].m_nAlid);
        rJsonWallMsg[udwJsonIndex].append(pstSession->m_stUserInfo.m_atbWall[udwIdx].m_nAlpos);
        rJsonWallMsg[udwJsonIndex].append(pstSession->m_stUserInfo.m_atbWall[udwIdx].m_nUid);
        rJsonWallMsg[udwJsonIndex].append(pstSession->m_stUserInfo.m_atbWall[udwIdx].m_sUin);
        rJsonWallMsg[udwJsonIndex].append(pstSession->m_stUserInfo.m_atbWall[udwIdx].m_sContent);
        rJsonWallMsg[udwJsonIndex].append(pstSession->m_stUserInfo.m_atbWall[udwIdx].m_nAvatar);
        rJsonWallMsg[udwJsonIndex].append(pstSession->m_stUserInfo.m_atbWall[udwIdx].m_nTopflag);


        TBOOL bIsExistDocument = CDocument::GetInstance()->IsSupportLang(CDocument::GetLang(0));
        
        if(-1 != pstSession->m_stUserInfo.m_atbWall[udwIdx].m_nRaw_lang
           && TRUE == bIsExistDocument)
        {
            rJsonWallMsgTranslate["raw_lang"] = pstSession->m_stUserInfo.m_atbWall[udwIdx].m_nRaw_lang;            
            if(TRUE == bIsExistDocument)
            {
                const Json::Value &stDocumentJson = CDocument::GetInstance()->GetDocumentJsonByLang(CDocument::GetLang(0));
                for(TUINT32 udwIdy = 0; udwIdy < stDocumentJson["doc_language"].size(); ++udwIdy)
                {
                    if(1 == stDocumentJson["doc_language"][CCommonFunc::NumToString(udwIdy)]["status"].asInt())
                    {
                        Json::Reader jsonReader;
                        Json::Value jResultBody;
                        if(jsonReader.parse(pstSession->m_stUserInfo.m_atbWall[udwIdx].m_sTranslate_content.c_str(), jResultBody))
                        {          
                            ossTranslateContent.str("");
                            ossTranslateContent << jResultBody[CCommonFunc::NumToString(udwIdy)]["0"].asString();
                            
                            rJsonWallMsgTranslate[CCommonFunc::NumToString(udwIdy)] = ossTranslateContent.str();                            
                            
                        }                    
                    }
                }
            }
            rJsonWallMsg[udwJsonIndex].append(rJsonWallMsgTranslate);
        }
        
        ++udwJsonIndex;
    }

    SUserInfo* pstUser = &pstSession->m_stUserInfo;
    TbAlliance* ptbAlliance = &pstUser->m_tbAlliance;

    Json::Value& rjsonAlliance = rJson["svr_alliance"];
    rjsonAlliance = Json::Value(Json::objectValue);
    rjsonAlliance["base"] = Json::Value(Json::arrayValue);
    rjsonAlliance["base"].append(ptbAlliance->m_nAid);
    rjsonAlliance["base"].append(ptbAlliance->m_sName);
    rjsonAlliance["base"].append(ptbAlliance->m_nOid);
    rjsonAlliance["base"].append(ptbAlliance->m_sOname);
    rjsonAlliance["base"].append(ptbAlliance->m_sDesc);
    rjsonAlliance["base"].append(ptbAlliance->m_sNotice);
    rjsonAlliance["base"].append(ptbAlliance->m_nMember);
    rjsonAlliance["base"].append(ptbAlliance->m_nMight);
    rjsonAlliance["base"].append(ptbAlliance->m_nPolicy / ALLIANCE_POLICY_OFFSET);
    rjsonAlliance["base"].append(ptbAlliance->m_nLanguage);
    rjsonAlliance["base"].append(CCommonBase::GetAlGiftPoint(ptbAlliance->m_nGift_point));
    rjsonAlliance["base"].append(ptbAlliance->m_nFund);
    rjsonAlliance["base"].append(ptbAlliance->m_nAvatar);
    rjsonAlliance["base"].append(ptbAlliance->m_sAl_nick_name);

    rjsonAlliance["can_assist_num"] = pstUser->m_udwAllianceCanAssistNum;
    rjsonAlliance["al_join_req_num"] = pstUser->m_udwAllianceReqCurFindNum;
    rjsonAlliance["can_help_action_num"] = pstUser->m_udwCanHelpTaskNum;
    rjsonAlliance["can_help_action_list"] = Json::Value(Json::arrayValue);
    for(TUINT32 udwIdx = 0; udwIdx < pstUser->m_udwAlCanHelpActionNum; ++udwIdx)
    {
        rjsonAlliance["can_help_action_list"].append(pstUser->m_patbAlCanHelpAction[udwIdx]->m_nId);
    }

    rjsonAlliance["wall_msg_num"] = 0;

    rjsonAlliance["throne_pos"] = ptbAlliance->m_nThrone_pos;
    rjsonAlliance["throne_status"] = ptbAlliance->m_nThrone_status;
    rjsonAlliance["al_star"] = ptbAlliance->m_nAl_star;

    CCommJson::GenAlGiftJson(pstUser, rjsonAlliance);

    CCommJson::GenTaskInfo(&pstSession->m_stUserInfo, rJson);
}

TBOOL CWallJson::TbAlComment_Compare(const TbAl_wall tbWall_A, const TbAl_wall tbWall_B)
{
    // 不管是置顶的还是非置顶的, 都以置顶时间和发送时间进行升序排序
    // 非置顶的最新在上面
    // 置顶的按置顶时间,最近置顶的在上面

    if(tbWall_A.m_nTopflag== tbWall_B.m_nTopflag)
    {
        if(1 == tbWall_A.m_nTopflag)
        {
            if(tbWall_A.m_nToptime > tbWall_B.m_nToptime)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            if(tbWall_A.m_nTime> tbWall_B.m_nTime)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }
    else if(tbWall_A.m_nTopflag > tbWall_B.m_nTopflag)
    {
        return true;
    }
    else
    {
        return false;
    }
}


