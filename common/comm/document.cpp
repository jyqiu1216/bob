#include "document.h"
#include <math.h>
#include "guard_mutex.h"
#include "game_info.h"
#include "tool_base.h"
#include "common_func.h"

CDocument* CDocument::m_poDocument = NULL;


CDocument* CDocument::GetInstance()
{
    if(m_poDocument == NULL)
    {
        m_poDocument = new CDocument;
    }

    return m_poDocument;
}

TINT32 CDocument::Init_All(CTseLogger *poLog)
{
    TINT32 dwRetCode = 0;

    if(poLog == NULL)
    {
        return -1;
    }
    // 0. set param
    m_poLog = poLog;

    dwRetCode = Init(poLog, DOCUMENT_ENGLISH_FILE);
    if(dwRetCode < 0)
    {
        return -1;
    }
    dwRetCode = Init(poLog, DOCUMENT_FRENCH_FILE);
    if(dwRetCode < 0)
    {
        return -1;
    }
    dwRetCode = Init(poLog, DOCUMENT_GERMAN_FILE);
    if(dwRetCode < 0)
    {
        return -1;
    }
    dwRetCode = Init(poLog, DOCUMENT_SPAIN_FILE);
    if(dwRetCode < 0)
    {
        return -1;
    }    
    dwRetCode = Init(poLog, DOCUMENT_RUSSIAN_FILE);
    if(dwRetCode < 0)
    {
    return -1;
    }

    dwRetCode = Init(poLog, DOCUMENT_CHINESE_FILE);
    if (dwRetCode < 0)
    {
        return -1;
    }

    /*
    dwRetCode = Init(poLog, DOCUMENT_ARABIC_FILE);
    if(dwRetCode < 0)
    {
    return -1;
    }


    dwRetCode = Init(poLog, DOCUMENT_PORTUGAL_FILE);
    if(dwRetCode < 0)
    {
    return -1;
    }

    dwRetCode = Init(poLog, DOCUMENT_ARABIC_ANDROID_FILE);
    if(dwRetCode < 0)
    {
    return -1;
    }
    dwRetCode = Init(poLog, DOCUMENT_CHINESE_ANDROID_FILE);
    if(dwRetCode < 0)
    {
    return -1;
    }
    dwRetCode = Init(poLog, DOCUMENT_ENGLISH_ANDROID_FILE);
    if(dwRetCode < 0)
    {
    return -1;
    }
    dwRetCode = Init(poLog, DOCUMENT_FRENCH_ANDROID_FILE);
    if(dwRetCode < 0)
    {
    return -1;
    }
    dwRetCode = Init(poLog, DOCUMENT_GERMAN_ANDROID_FILE);
    if(dwRetCode < 0)
    {
    return -1;
    }
    dwRetCode = Init(poLog, DOCUMENT_PORTUGAL_ANDROID_FILE);
    if(dwRetCode < 0)
    {
    return -1;
    }
    dwRetCode = Init(poLog, DOCUMENT_RUSSIAN_ANDROID_FILE);
    if(dwRetCode < 0)
    {
    return -1;
    }
    dwRetCode = Init(poLog, DOCUMENT_SPAIN_ANDROID_FILE);
    if(dwRetCode < 0)
    {
    return -1;
    }
    */

    return 0;

}

TINT32 CDocument::Update_All(CTseLogger *poLog)
{
    if(poLog == NULL)
    {
        return -1;
    }

    TINT32 dwRetCode = 0;
    CDocument *poDocument = new CDocument;
    CDocument *poTmpDocument = m_poDocument;

    dwRetCode = poDocument->Init_All(poLog);
    if (dwRetCode != 0)
    {
        TSE_LOG_ERROR(poLog, (" CDocument::Update_All failed[%d]", dwRetCode));
        delete poDocument;
        return -1;
    }

    m_poDocument = poDocument;

    if (poTmpDocument != NULL)
    {
        sleep(3);
        delete poTmpDocument;
    }

    TSE_LOG_INFO(poLog, ("CDocument::Update_All ok", dwRetCode));
    return 0;
}

TINT32 CDocument::Init(CTseLogger *poLog, string strDocumentFileName)
{
    if(poLog == NULL)
    {
        return -1;
    }

    // 0. set param
    m_poLog = poLog;

    // 1.判断配置文件，必须存在
    if(0 != access(strDocumentFileName.c_str(), F_OK))
    {
        assert(0);
    }

    Json::Reader reader;
    std::ifstream is;
    is.open(strDocumentFileName.c_str(), std::ios::binary);
    Json::Value tmpRawJson;
    Json::Value oJsonTmp;
    if(reader.parse(is, tmpRawJson) == false)
    {
        TSE_LOG_ERROR(m_poLog, ("CDocument::Init: parse file[%s] failed.", strDocumentFileName.c_str()));
        is.close();
        return -2;
    }
    else
    {
        oJsonTmp.clear();
        //oJsonTmp = Json::Value(Json::objectValue);
        //CToolBase::LoadNewJson(tmpRawJson, oJsonTmp);
        oJsonTmp = tmpRawJson;
        TSE_LOG_INFO(m_poLog, ("CDocument::Init: parse file[%s] success.", strDocumentFileName.c_str()));
        is.close();
    }

    // 初始化document_map
    string strFind = ".json";
    string strReplace = "";
    string::size_type pos = 0;
    string::size_type FindSize = strFind.size();
    string::size_type ReplaceSize = strReplace.size();
    while((pos = strDocumentFileName.find(strFind, pos)) != string::npos)
    {
        strDocumentFileName.replace(pos, FindSize, strReplace);
        pos += ReplaceSize;
    }
    strFind = "../data/document_";
    strReplace = "";
    pos = 0;
    FindSize = strFind.size();
    ReplaceSize = strReplace.size();
    while((pos = strDocumentFileName.find(strFind, pos)) != string::npos)
    {
        strDocumentFileName.replace(pos, FindSize, strReplace);
        pos += ReplaceSize;
    }

    TSE_LOG_INFO(m_poLog, ("[language=%s]", strDocumentFileName.c_str()));
    m_oDocumentMap[strDocumentFileName] = oJsonTmp;

    return 0;
}

/*
TINT32 CDocument::Update(CTseLogger *poLog, string strDocumentFileName)
{
    TINT32 dwRetCode = 0;

    dwRetCode = m_poDocument->Init(poLog, strDocumentFileName);
    if(dwRetCode != 0)
    {
        TSE_LOG_ERROR(poLog, ("CDocument::Update failed [filename=%s] [ret=%d]", \
                              strDocumentFileName.c_str(), \
                              dwRetCode));
        return -1;
    }

    sleep(3);

    TSE_LOG_INFO(poLog, ("CDocument::Update ok [filename=%s]", \
                         strDocumentFileName.c_str()));

    return 0;
}
*/
TBOOL CDocument::GetDocumentJsonByLang(string strLang, Json::Value &rDocumentJson)
{
    map<string, Json::Value>::iterator it;
    it = m_oDocumentMap.find(strLang);
    if(it != m_oDocumentMap.end())
    {
        rDocumentJson = it->second;
        return TRUE;
    }

    return FALSE;
}

TBOOL CDocument::IsSupportLang(const string &strLang)
{
    if (m_oDocumentMap.find(strLang) != m_oDocumentMap.end())
    {
        return TRUE;
    }
    return FALSE;
}

const Json::Value &CDocument::GetDocumentJsonByLang(const string &strLang)
{
    if (m_oDocumentMap.find(strLang) == m_oDocumentMap.end())
    {
        assert(0);
    }
    return m_oDocumentMap[strLang];
}

string CDocument::GetLang(TUINT32 udwLang)
{
    string strlang = "";
    switch(udwLang) 
    {
        case 0:
            strlang = "english";
            break;
        case 1:
            strlang = "german";
            break;
        case 2:
            strlang = "french";
            break;
         case 3:
            strlang = "portugal";
            break;
        case 4:
            strlang = "spain";
            break;
        case 5:
            strlang = "russian";
            break;
        case 6:
            strlang = "chinese";
            break;       
         case 7:
            strlang = "arabic";
            break;                  
        default:
            strlang = "english";
    }

    TSE_LOG_INFO(CGameInfo::GetInstance()->m_poLog, ("CDocument::GetLang [lang=%s]", \
                                                    strlang.c_str()));
    return strlang;
}


TINT32 CDocument::GetSupportLangNum()
{
    TINT32 dwSupportLangNum = 0;

    TBOOL bIsExistDocument = IsSupportLang(GetLang(0));
    if (FALSE == bIsExistDocument)
    {
        dwSupportLangNum = 0;
        TSE_LOG_INFO(CGameInfo::GetInstance()->m_poLog, ("CDocument::GetSupportLangNum [SupportLangNum=%d]", \
            dwSupportLangNum));
        return dwSupportLangNum;
    }

    const Json::Value &stDocumentJson = GetDocumentJsonByLang(GetLang(0));
    
    dwSupportLangNum = stDocumentJson["doc_language"].size();
    TSE_LOG_INFO(CGameInfo::GetInstance()->m_poLog, ("CDocument::GetSupportLangNum [SupportLangNum=%d]", \
        dwSupportLangNum));
    return dwSupportLangNum;
}


string CDocument::GetShortLangName(TUINT32 udwLang)
{
    string strShortLangName = "";
    switch(udwLang) 
    {
        case 0:
            strShortLangName = "en";
            break;
        case 1:
            strShortLangName = "de";
            break;
        case 2:
            strShortLangName = "fr";
            break;
         case 3:
            strShortLangName = "pt";
            break;
        case 4:
            strShortLangName = "es";
            break;
        case 5:
            strShortLangName = "ru";
            break;
        case 6:
            strShortLangName = "zh-CHS";
            break;       
         case 7:
            strShortLangName = "ar";
            break;                  
        default:
            strShortLangName = "";
    }

    TSE_LOG_INFO(CGameInfo::GetInstance()->m_poLog, ("CDocument::GetShortLangName [ShortLangName=%s]", \
                                                     strShortLangName.c_str()));
    return strShortLangName;
}

string CDocument::GetLangId(string strShortLangName)
{
    string strLangId = "";
    if("en" == strShortLangName)
    {
        strLangId = "0";
    }
    else if("de" == strShortLangName)
    {
        strLangId = "1";
    }
    else if("fr" == strShortLangName)
    {
        strLangId = "2";
    }
    else if("pt" == strShortLangName)
    {
        strLangId = "3";
    }
    else if("es" == strShortLangName)
    {
        strLangId = "4";
    }
    else if("ru" == strShortLangName)
    {
        strLangId = "5";
    }
    else if("zh-CHS" == strShortLangName)
    {
        strLangId = "6";
    }
    else if("ar" == strShortLangName)
    {
        strLangId = "7";
    }
    else
    {
        strLangId = "";
    }


    TSE_LOG_INFO(CGameInfo::GetInstance()->m_poLog, ("CDocument::GetLangId [LangId=%s]", \
                                                     strLangId.c_str()));
    return strLangId;
}



string CDocument::GetLanguageId(string strShortLangName, TINT32 dwReqLang)
{ 
    TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("CDocument::GetLanguageId [strShortLangName=%s]", \
                                                      strShortLangName.c_str()));

    TBOOL bIsExistDocument = IsSupportLang(GetLang(dwReqLang));
    if (FALSE == bIsExistDocument)
    {
        return "-1";
    }
    else
    {
        const Json::Value &stDocumentJson = GetDocumentJsonByLang(GetLang(dwReqLang));
        Json::Value::Members vecMembers = stDocumentJson["doc_lang"].getMemberNames();
        for(TUINT32 udwIdx = 0; udwIdx < vecMembers.size(); ++udwIdx)
        {
            TSE_LOG_DEBUG(CGameInfo::GetInstance()->m_poLog, ("CDocument::GetLanguageId [strShortLangName=%s] [doc_ShortLangName=%s]", \
                                                              strShortLangName.c_str(), \
                                                              stDocumentJson["doc_lang"][vecMembers[udwIdx]]["brief"].asString().c_str()));
            if(strShortLangName == stDocumentJson["doc_lang"][vecMembers[udwIdx]]["brief"].asString())
            {
                return vecMembers[udwIdx];
            }
        }
        return "-1";
    }
    

}

string CDocument::GetSvrName(TINT32 dwSvrId)
{
    TBOOL bIsExistDocument = IsSupportLang("english");
    if (FALSE == bIsExistDocument)
    {
        string str = "K";
        str += CCommonFunc::NumToString(dwSvrId);
        return str;
    }

    const Json::Value &stDocumentJson = GetDocumentJsonByLang("english");

    if (!stDocumentJson["doc_world"].isMember(CCommonFunc::NumToString(dwSvrId)))
    {
        string str = "K";
        str += CCommonFunc::NumToString(dwSvrId);
        return str;
    }

    return stDocumentJson["doc_world"][CCommonFunc::NumToString(dwSvrId)]["svr_name"].asString();
}
