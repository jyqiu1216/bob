#pragma once

#ifndef _JSONCPP_UTILS_H_
#define _JSONCPP_UTILS_H_

#include <stack>
#include "base/common/wtsetypedef.h"
#include "jsoncpp/json/json.h"

using namespace std;
using namespace Json;

enum
{
    UNSERI_MODE_REPLACE = 0,
    UNSERI_MODE_UPDATE
};

enum
{
    TYPE_INT8 = 0,
    TYPE_UINT8,
    TYPE_INT16,
    TYPE_UINT16,
    TYPE_INT32,
    TYPE_UINT32,
    TYPE_INT64,
    TYPE_UINT64,
    TYPE_FLOAT,
    TYPE_DOUBLE,
    TYPE_TRUE,
    TYPE_FALSE,
    TYPE_NULL,
    TYPE_STRING,
    TYPE_ARRAY,
    TYPE_OBJECT
};

// ���л���
class CJsoncppSeri
{
public:
    CJsoncppSeri(TUINT32 bufLen = MAX_SERI_BUF_LEN);
    ~CJsoncppSeri();

    const TCHAR *serializeToBuffer(const Value &val, TUINT32 &len);

private:
    // ������������ֵ
    CJsoncppSeri(const CJsoncppSeri &);
    const CJsoncppSeri & operator=(const CJsoncppSeri &);

    static const TUINT32 MAX_SERI_BUF_LEN = 2 * 1024 * 1024; // 2M ���л�������Ĭ�ϴ�С

    const TUINT32 m_bufLen;                                  // ���л���������С
    TCHAR *m_pSeriBuf;                                       // ���л�����
    TCHAR *m_pSeriBufCur;                                    // ���л���������ǰָ��

    void serialize(const Value &val);
    void copyValueToBuffer(const Value &val);
    TINT32 copyStringToBuffer(const TCHAR *pst, TUINT32 len);
};


// �����л���
class CJsoncppUnseri
{
public:
    CJsoncppUnseri();
    ~CJsoncppUnseri();

    TINT32 unserializeToDom(const TCHAR *buf, Value &root, TINT32 mode);

private:
    // ������������ֵ
    CJsoncppUnseri(const CJsoncppUnseri &);
    const CJsoncppUnseri & operator=(const CJsoncppUnseri &);

    const TCHAR *m_pUnseriBufCur;                            // �����л���������ǰָ��
    stack<Value *> m_stack;

    void unserialize();
    void updateValue(Value &old, Value &newKey, Value &newVal);
};

#endif // _RAPID_JSON_UTILS_H_
