#ifndef _TRANSLATE_REQ_INFO_H_
#define _TRANSLATE_REQ_INFO_H_

#include <string>
using namespace std;


struct TranslateReqInfo
{
    string m_strTranslateType;          // ������Դ���ݽ��з���
    string m_strTranslateOperate;       // �����������Ӧ�Ľӿ�
    string m_strTranslateContent;       // �������������

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

