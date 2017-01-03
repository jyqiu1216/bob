#ifndef _DOCUMENT_H_
#define _DOCUMENT_H_

#include "jsoncpp/json/json.h"
#include "game_define.h"
#include "base/log/wtselogger.h"
#include <set>

using namespace wtse::log;
using namespace std;


#define UPDATE_DOCUMENT_ALL_FLAG_FILE ("../data/document_all_flag")
#define UPDATE_SUB_DOCUMENT_ALL_FLAG_FILE ("../data/sub_document_all_flag")

/*
#define UPDATE_DOCUMENT_ENGLISH_FLAG_FILE ("../data/document_english_flag")
#define UPDATE_DOCUMENT_FRENCH_FLAG_FILE ("../data/document_french_flag")
#define UPDATE_DOCUMENT_GERMAN_FLAG_FILE ("../data/document_german_flag")
#define UPDATE_DOCUMENT_SPAIN_FLAG_FILE ("../data/document_spain_flag")
#define UPDATE_DOCUMENT_RUSSIAN_FLAG_FILE ("../data/document_russian_flag")
#define UPDATE_DOCUMENT_ARABIC_FLAG_FILE ("../data/document_arabic_flag")
#define UPDATE_DOCUMENT_CHINESE_FLAG_FILE ("../data/document_chinese_flag")
#define UPDATE_DOCUMENT_PORTUGAL_FLAG_FILE ("../data/document_portugal_flag")
*/

/*
#define UPDATE_DOCUMENT_ENGLISH_ANDROID_FLAG_FILE ("../data/document_chinese_andriod_flag")
#define UPDATE_DOCUMENT_ARABIC_ANDROID_FLAG_FILE ("../data/document_spain_andriod_flag")
#define UPDATE_DOCUMENT_CHINESE_ANDROID_FLAG_FILE ("../data/document_arabic_andriod_flag")
#define UPDATE_DOCUMENT_FRENCH_ANDROID_FLAG_FILE ("../data/document_english_andriod_flag")
#define UPDATE_DOCUMENT_GERMAN_ANDROID_FLAG_FILE ("../data/document_french_andriod_flag")
#define UPDATE_DOCUMENT_PORTUGAL_ANDROID_FLAG_FILE ("../data/document_german_andriod_flag")
#define UPDATE_DOCUMENT_RUSSIAN_ANDROID_FLAG_FILE ("../data/document_portugal_android_flag")
#define UPDATE_DOCUMENT_SPAIN_ANDROID_FLAG_FILE ("../data/document_russian_andriod_flag")
*/

#define DOCUMENT_ENGLISH_FILE ("../data/document_english.json")
#define DOCUMENT_FRENCH_FILE ("../data/document_french.json")
#define DOCUMENT_GERMAN_FILE ("../data/document_german.json")
#define DOCUMENT_SPAIN_FILE ("../data/document_spain.json")
#define DOCUMENT_RUSSIAN_FILE ("../data/document_russian.json")
#define DOCUMENT_CHINESE_FILE ("../data/document_chinese.json")

#define SUB_DOCUMENT_ENGLISH_FILE ("../data/sub_document_english.json")
#define SUB_DOCUMENT_FRENCH_FILE ("../data/sub_document_french.json")
#define SUB_DOCUMENT_GERMAN_FILE ("../data/sub_document_german.json")
#define SUB_DOCUMENT_SPAIN_FILE ("../data/sub_document_spain.json")
#define SUB_DOCUMENT_RUSSIAN_FILE ("../data/sub_document_russian.json")
#define SUB_DOCUMENT_CHINESE_FILE ("../data/sub_document_chinese.json")
/*
#define DOCUMENT_ARABIC_FILE ("../data/document_arabic.json.de")
#define DOCUMENT_PORTUGAL_FILE ("../data/document_portugal.json.de")
*/

/*
#define DOCUMENT_ENGLISH_ANDROID_FILE ("../data/document_english_andriod.json.de")
#define DOCUMENT_ARABIC_ANDROID_FILE ("../data/document_arabic_andriod.json.de")
#define DOCUMENT_CHINESE_ANDROID_FILE ("../data/document_chinese_andriod.json.de")
#define DOCUMENT_FRENCH_ANDROID_FILE ("../data/document_french_andriod.json.de")
#define DOCUMENT_GERMAN_ANDROID_FILE ("../data/document_german_andriod.json.de")
#define DOCUMENT_PORTUGAL_ANDROID_FILE ("../data/document_portugal_andriod.json.de")
#define DOCUMENT_RUSSIAN_ANDROID_FILE ("../data/document_russian_andriod.json.de")
#define DOCUMENT_SPAIN_ANDROID_FILE ("../data/document_spain_andriod.json.de")
*/

class CDocument
{
public:
	static CDocument* m_poDocument;
	static CDocument* GetInstance();
    //static TINT32 Update(CTseLogger *poLog, string strDocumentFileName);
    static TINT32 Update_All(CTseLogger *poLog);
public:
	TINT32 Init(CTseLogger *poLog, string strDocumentFileName, string strSubDocFileName);
    TINT32 Init_All(CTseLogger *poLog);

public:
	CTseLogger* m_poLog;
    map<string, Json::Value> m_oDocumentMap; 


public:
    TBOOL IsSupportLang(const string &strLang);
    const Json::Value &GetDocumentJsonByLang(const string &strLang);

    TBOOL GetDocumentJsonByLang(string strLang, Json::Value &rDocumentJson);
    static string GetLang(TUINT32 udwLang);
    TINT32 GetSupportLangNum();
    string GetShortLangName(TUINT32 udwLang);

    // 支持的语言id
    string GetLangId(string strShortLangName);


    // 翻译支持的id
    string GetLanguageId(string strShortLangName, TINT32 dwReqLang);
    
    string GetSvrName(TINT32 dwSvrId);
    TINT32 LoadSubDoc(CTseLogger *poLog, string strDocumentFileName, string strSubDocFileName);
};

#endif

