#ifndef _TRANSLATE_REQ_INFO_H_
#define _TRANSLATE_REQ_INFO_H_

#include <string>
using namespace std;


struct TranslateReqInfo
{
    string m_strTranslateType;          // 对哪种源数据进行翻译
    string m_strTranslateOperate;       // 翻译服务锁对应的接口
    string m_strTranslateContent;       // 翻译的请求内容

    TranslateReqInfo(const string &strTranslateType = "", const string &strTranslateOperate = "", const string &strTranslateContent = ""):m_strTranslateType(strTranslateType), m_strTranslateOperate(strTranslateOperate), m_strTranslateContent(strTranslateContent)
    {
    
    }

    TVOID SetVal(string strTranslateType, string strTranslateOperate, string strTranslateContent)
    {
        m_strTranslateType = strTranslateType;
        m_strTranslateOperate = strTranslateOperate;
        m_strTranslateContent = strTranslateContent;

    }

};

#endif

