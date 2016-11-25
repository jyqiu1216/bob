#include "jsoncpp_utils.h"
#include <cstring>
#include <cassert>

// ********** 序列化 **********

CJsoncppSeri::CJsoncppSeri(TUINT32 bufLen) : m_bufLen(bufLen)
{
    m_pSeriBuf = new TCHAR[m_bufLen];
    m_pSeriBufCur = m_pSeriBuf;
}

CJsoncppSeri::~CJsoncppSeri()
{
    delete[] m_pSeriBuf;
}

const TCHAR *CJsoncppSeri::serializeToBuffer(const Value &val, TUINT32 &len)
{
    m_pSeriBufCur = m_pSeriBuf;

    serialize(val);
    len = m_pSeriBufCur - m_pSeriBuf;
    assert(len <= m_bufLen);

    return m_pSeriBuf;
}

void CJsoncppSeri::serialize(const Value &val)
{
    copyValueToBuffer(val);

    switch (val.type())
    {
    case stringValue:
        copyStringToBuffer(val.asString().c_str(), val.asString().length());
        break;

    case arrayValue:
    {
        for (TUINT32 i = 0; i < val.size(); ++i)
        {
           serialize(val[i]);
        }
    }
        break;

    case objectValue:
    {
        Value::Members members(val.getMemberNames());
        for (Value::Members::iterator it = members.begin(); it != members.end(); ++it)
        {
            const string& name = *it;
            TCHAR flag = TYPE_STRING;
            TUINT32 v = name.length();
            memcpy(m_pSeriBufCur, &flag, sizeof(flag));
            m_pSeriBufCur += sizeof(flag);
            memcpy(m_pSeriBufCur, &v, sizeof(v));
            m_pSeriBufCur += sizeof(v);
            copyStringToBuffer(name.c_str(), name.length());

            serialize(val[name]);
        }
    }
        break;

    default:
        // nullValue
        // intValue
        // uintValue
        // readValue
        // booleanValue
        // do nothing
        break;
    }
}

void CJsoncppSeri::copyValueToBuffer(const Value &val)
{
    TCHAR flag;

    switch (val.type())
    {
    case intValue:
    {
        TINT64 v = val.asLargestInt();

        if (static_cast<TINT8>(v) == v)
        {
            flag = TYPE_INT8;
            TINT8 v8 = static_cast<TINT8>(v);
            memcpy(m_pSeriBufCur, &flag, sizeof(flag));
            m_pSeriBufCur += sizeof(flag);
            memcpy(m_pSeriBufCur, &v8, sizeof(v8));
            m_pSeriBufCur += sizeof(v8);
        }
        else if (static_cast<TINT16>(v) == v)
        {
            flag = TYPE_INT16;
            TINT16 v16 = static_cast<TINT16>(v);
            memcpy(m_pSeriBufCur, &flag, sizeof(flag));
            m_pSeriBufCur += sizeof(flag);
            memcpy(m_pSeriBufCur, &v16, sizeof(v16));
            m_pSeriBufCur += sizeof(v16);
        }
        else if (static_cast<TINT32>(v) == v)
        {
            flag = TYPE_INT32;
            TINT32 v32 = static_cast<TINT32>(v);
            memcpy(m_pSeriBufCur, &flag, sizeof(flag));
            m_pSeriBufCur += sizeof(flag);
            memcpy(m_pSeriBufCur, &v32, sizeof(v32));
            m_pSeriBufCur += sizeof(v32);
        }
        else
        {
            flag = TYPE_INT64;
            memcpy(m_pSeriBufCur, &flag, sizeof(flag));
            m_pSeriBufCur += sizeof(flag);
            memcpy(m_pSeriBufCur, &v, sizeof(v));
            m_pSeriBufCur += sizeof(v);
        }
    }
        break;

    case uintValue:
    {
        TUINT64 v = val.asLargestUInt();

        if (static_cast<TUINT8>(v) == v)
        {
            flag = TYPE_UINT8;
            TUINT8 v8 = static_cast<TUINT8>(v);
            memcpy(m_pSeriBufCur, &flag, sizeof(flag));
            m_pSeriBufCur += sizeof(flag);
            memcpy(m_pSeriBufCur, &v8, sizeof(v8));
            m_pSeriBufCur += sizeof(v8);
        }
        else if (static_cast<TUINT16>(v) == v)
        {
            flag = TYPE_UINT16;
            TUINT16 v16 = static_cast<TUINT16>(v);
            memcpy(m_pSeriBufCur, &flag, sizeof(flag));
            m_pSeriBufCur += sizeof(flag);
            memcpy(m_pSeriBufCur, &v16, sizeof(v16));
            m_pSeriBufCur += sizeof(v16);
        }
        else if (static_cast<TUINT32>(v) == v)
        {
            flag = TYPE_UINT32;
            TUINT32 v32 = static_cast<TUINT32>(v);
            memcpy(m_pSeriBufCur, &flag, sizeof(flag));
            m_pSeriBufCur += sizeof(flag);
            memcpy(m_pSeriBufCur, &v32, sizeof(v32));
            m_pSeriBufCur += sizeof(v32);
        }
        else
        {
            flag = TYPE_UINT64;
            memcpy(m_pSeriBufCur, &flag, sizeof(flag));
            m_pSeriBufCur += sizeof(flag);
            memcpy(m_pSeriBufCur, &v, sizeof(v));
            m_pSeriBufCur += sizeof(v);
        }
    }
        break;

    case realValue:
    {
        flag = TYPE_DOUBLE;
        TFLOAT64 v = val.asDouble();
        memcpy(m_pSeriBufCur, &flag, sizeof(flag));
        m_pSeriBufCur += sizeof(flag);
        memcpy(m_pSeriBufCur, &v, sizeof(v));
        m_pSeriBufCur += sizeof(v);
    }
        break;

    case nullValue:
    {
        flag = TYPE_NULL;
        memcpy(m_pSeriBufCur, &flag, sizeof(flag));
        m_pSeriBufCur += sizeof(flag);
    }
        break;

    case booleanValue:
    {
        if (val.asBool())
        {
            flag = TYPE_TRUE;
            memcpy(m_pSeriBufCur, &flag, sizeof(flag));
            m_pSeriBufCur += sizeof(flag);
        }
        else
        {
            flag = TYPE_FALSE;
            memcpy(m_pSeriBufCur, &flag, sizeof(flag));
            m_pSeriBufCur += sizeof(flag);
        }
    }
        break;

    case stringValue:
    {
        flag = TYPE_STRING;
        TUINT32 v = val.asString().length();
        memcpy(m_pSeriBufCur, &flag, sizeof(flag));
        m_pSeriBufCur += sizeof(flag);
        memcpy(m_pSeriBufCur, &v, sizeof(v));
        m_pSeriBufCur += sizeof(v);
    }
        break;

    case arrayValue:
    {
        flag = TYPE_ARRAY;
        TUINT32 v = val.size();
        memcpy(m_pSeriBufCur, &flag, sizeof(flag));
        m_pSeriBufCur += sizeof(flag);
        memcpy(m_pSeriBufCur, &v, sizeof(v));
        m_pSeriBufCur += sizeof(v);
    }
        break;

    case objectValue:
    {
        flag = TYPE_OBJECT;
        TUINT32 v = val.getMemberNames().size();
        memcpy(m_pSeriBufCur, &flag, sizeof(flag));
        m_pSeriBufCur += sizeof(flag);
        memcpy(m_pSeriBufCur, &v, sizeof(v));
        m_pSeriBufCur += sizeof(v);
    }
        break;

    }
}

TINT32 CJsoncppSeri::copyStringToBuffer(const TCHAR *pst, TUINT32 len)
{
    if (NULL == pst)
    {
        return -1;
    }

    memcpy(m_pSeriBufCur, pst, len);
    m_pSeriBufCur += len;

    return 0;
}


// ********** 反序列化 **********

CJsoncppUnseri::CJsoncppUnseri() : m_pUnseriBufCur(0)
{
    // do nothing
}

CJsoncppUnseri::~CJsoncppUnseri()
{
    // do nothing
}

TINT32 CJsoncppUnseri::unserializeToDom(const TCHAR *buf, Value &root, TINT32 mode)
{
    if (NULL == buf || (mode != UNSERI_MODE_REPLACE && mode != UNSERI_MODE_UPDATE))
    {
        return -1;
    }

    m_pUnseriBufCur = buf;

    if (UNSERI_MODE_REPLACE == mode || (UNSERI_MODE_UPDATE == mode && !root.isObject()))
    {
        root = Value(objectValue);
    }

    // 清空stack
    while (!m_stack.empty())
    {
        m_stack.pop();
    }

    m_stack.push(&root);
    unserialize();
    m_stack.pop();

    return 0;
}

void CJsoncppUnseri::unserialize()
{
    Value *pVal = m_stack.top();

    const TCHAR *pFlag = (const TCHAR *)m_pUnseriBufCur;
    m_pUnseriBufCur += sizeof(TCHAR);
    switch (*pFlag)
    {
    case TYPE_INT8:
    {
        TINT8 val = *(TINT8*)m_pUnseriBufCur;
        m_pUnseriBufCur += sizeof(TINT8);

        Value tmp = val;
        (*pVal) = tmp;
    }
        break;

    case TYPE_UINT8:
    {
        TUINT8 val = *(TUINT8*)m_pUnseriBufCur;
        m_pUnseriBufCur += sizeof(TUINT8);

        Value tmp = val;
        (*pVal) = tmp;
    }
        break;

    case TYPE_INT16:
    {
        TINT16 val = *(TINT16*)m_pUnseriBufCur;
        m_pUnseriBufCur += sizeof(TINT16);

        Value tmp = val;
        (*pVal) = tmp;
    }
        break;

    case TYPE_UINT16:
    {
        TUINT16 val = *(TUINT16*)m_pUnseriBufCur;
        m_pUnseriBufCur += sizeof(TUINT16);

        Value tmp = val;
        (*pVal) = tmp;
    }
        break;

    case TYPE_INT32:
    {
        TINT32 val = *(TINT32*)m_pUnseriBufCur;
        m_pUnseriBufCur += sizeof(TINT32);

        Value tmp = val;
        (*pVal) = tmp;
    }
        break;

    case TYPE_UINT32:
    {
        TUINT32 val = *(TUINT32*)m_pUnseriBufCur;
        m_pUnseriBufCur += sizeof(TUINT32);

        Value tmp = val;
        (*pVal) = tmp;
    }
        break;

    case TYPE_INT64:
    {
        TINT64 val = *(TINT64*)m_pUnseriBufCur;
        m_pUnseriBufCur += sizeof(TINT64);

        Value tmp = val;
        (*pVal) = tmp;
    }
        break;

    case TYPE_UINT64:
    {
        TUINT64 val = *(TUINT64*)m_pUnseriBufCur;
        m_pUnseriBufCur += sizeof(TUINT64);

        Value tmp = val;
        (*pVal) = tmp;
    }
        break;

    case TYPE_DOUBLE:
    {
        double val = *(double*)m_pUnseriBufCur;
        m_pUnseriBufCur += sizeof(double);

        Value tmp = val;
        (*pVal) = tmp;
    }
        break;

    case TYPE_NULL:
    {
        Value tmp;
        (*pVal) = tmp;
    }
        break;

    case TYPE_TRUE:
    {
        (*pVal) = true;
    }
        break;

    case TYPE_FALSE:
    {
        (*pVal) = false;
    }
        break;

    case TYPE_STRING:
    {
        TUINT32 len = *((TUINT32*)m_pUnseriBufCur);
        m_pUnseriBufCur += sizeof(TUINT32);

        Value tmp(m_pUnseriBufCur, m_pUnseriBufCur + len);
        m_pUnseriBufCur += len;
        (*pVal) = tmp;
    }
        break;

    case TYPE_ARRAY:
    {
        TUINT32 num = *((TUINT32*)m_pUnseriBufCur);
        m_pUnseriBufCur += sizeof(TUINT32);

        (*pVal) = Value(arrayValue);
        for (TUINT32 i = 0; i < num; ++i)
        {
            Value& v = (*pVal)[i];
            m_stack.push(&v);
            unserialize();
            m_stack.pop();
        }
    }
        break;

    case TYPE_OBJECT:
    {
        TUINT32 num = *((TUINT32*)m_pUnseriBufCur);
        m_pUnseriBufCur += sizeof(TUINT32);

        // 需要判断，否则在更新模式下会把原数据清除
        if (!((*pVal).isObject()))
        {
            (*pVal) = Value(objectValue);
        }

        for (TUINT32 i = 0; i < num; ++i)
        {
            m_pUnseriBufCur += sizeof(TCHAR);
            TUINT32 len = *((TUINT32*)m_pUnseriBufCur);
            m_pUnseriBufCur += sizeof(TUINT32);
            string key(m_pUnseriBufCur, len);
            m_pUnseriBufCur += len;

            Value& v = (*pVal)[key];
            m_stack.push(&v);
            unserialize();
            m_stack.pop();
        }
    }
        break;
    }
}
