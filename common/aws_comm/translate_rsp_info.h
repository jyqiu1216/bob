#ifndef _TRANSLATE_RSP_INFO_H_
#define _TRANSLATE_RSP_INFO_H_

#include <string>
using namespace std;

struct TranslateRspInfo
{
    int m_dwRetCode;
    unsigned int m_udwServiceType;
    string m_strTranslateType;
    string m_strTranslateOperate;
    string m_strTranslateRawLang;
    string m_strTranslateResultLang;
    string m_strTranslateContent;
    string m_strTranslateResult;

    TranslateRspInfo()
    {
        Reset();
    }

    void Reset()
    {
        m_dwRetCode = 0;
        m_udwServiceType = 0;
        m_strTranslateType = "";
        m_strTranslateOperate = "";
        m_strTranslateRawLang = "";
        m_strTranslateResultLang = "";
        m_strTranslateContent = "";
        m_strTranslateResult = "";
    }
};

#endif      

