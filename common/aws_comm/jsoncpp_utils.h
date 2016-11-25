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

// 序列化类
class CJsoncppSeri
{
public:
    CJsoncppSeri(TUINT32 bufLen = MAX_SERI_BUF_LEN);
    ~CJsoncppSeri();

    const TCHAR *serializeToBuffer(const Value &val, TUINT32 &len);

private:
    // 不允许拷贝及赋值
    CJsoncppSeri(const CJsoncppSeri &);
    const CJsoncppSeri & operator=(const CJsoncppSeri &);

    static const TUINT32 MAX_SERI_BUF_LEN = 2 * 1024 * 1024; // 2M 序列化缓冲区默认大小

    const TUINT32 m_bufLen;                                  // 序列化缓冲区大小
    TCHAR *m_pSeriBuf;                                       // 序列化数据
    TCHAR *m_pSeriBufCur;                                    // 序列化缓冲区当前指针

    void serialize(const Value &val);
    void copyValueToBuffer(const Value &val);
    TINT32 copyStringToBuffer(const TCHAR *pst, TUINT32 len);
};


// 反序列化类
class CJsoncppUnseri
{
public:
    CJsoncppUnseri();
    ~CJsoncppUnseri();

    TINT32 unserializeToDom(const TCHAR *buf, Value &root, TINT32 mode);

private:
    // 不允许拷贝及赋值
    CJsoncppUnseri(const CJsoncppUnseri &);
    const CJsoncppUnseri & operator=(const CJsoncppUnseri &);

    const TCHAR *m_pUnseriBufCur;                            // 反序列化缓冲区当前指针
    stack<Value *> m_stack;

    void unserialize();
    void updateValue(Value &old, Value &newKey, Value &newVal);
};

#endif // _RAPID_JSON_UTILS_H_
