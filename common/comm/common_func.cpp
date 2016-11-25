#include "common_func.h"
#include "encode/src/encode/utf8_util.h"

void CCommonFunc::UIntToString(unsigned int key, char*pstr)
{
    const char* pVal = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    char szTmp[20];
    int dwByteNum = 0;
    unsigned int udwIndex = 0;

    while(key)
    {
        udwIndex = key % 36;
        szTmp[dwByteNum++] = pVal[udwIndex];
        key = key / 36;
    }

    for(int idx = 0; idx < dwByteNum; idx++)
    {
        pstr[dwByteNum - idx - 1] = szTmp[idx];
    }
    pstr[dwByteNum] = 0;
}


string CCommonFunc::GetTableRawName(const string& strTableName)
{
    string strRawTableName = "";

    std::size_t begin = strTableName.find("_global_");
    if(begin != std::string::npos)
    {
        strRawTableName = strTableName.substr(begin + strlen("_global_"));
        return strRawTableName;
    }

    begin = strTableName.find("_split_");
    if(begin != std::string::npos)
    {
        strRawTableName = strTableName.substr(begin + strlen("_split_"));
        std::size_t end = strRawTableName.find_last_of('_');
        strRawTableName.resize(end);
        return strRawTableName;
    }

    begin = strTableName.find("_month_");
    if(begin != std::string::npos)
    {
        strRawTableName = strTableName.substr(begin + strlen("_month_"));
        std::size_t end = strRawTableName.find_last_of('_');
        strRawTableName.resize(end);
        return strRawTableName;
    }

    return strRawTableName;
}

TVOID CCommonFunc::ArrayOutput(TCHAR *pszOut, TUINT32 &udwOutLen, TUINT32 *pudwTroop, TUINT32 udwTroopTypeNum, const TCHAR *pszKey, TBOOL bHead)
{
    TUINT32 idx = 0;
    TUINT32 udwCurLen = 0;
    TCHAR *pCur = pszOut;

    if(bHead == TRUE)
    {
        udwCurLen = sprintf(pCur, "\"%s\":[", pszKey);
        pCur += udwCurLen;
    }
    else
    {
        udwCurLen = sprintf(pCur, ",\"%s\":[", pszKey);
        pCur += udwCurLen;
    }

    for(idx = 0; idx < udwTroopTypeNum; idx++)
    {
        if(idx == 0)
        {
            udwCurLen = sprintf(pCur, "%u", pudwTroop[idx]);
        }
        else
        {
            udwCurLen = sprintf(pCur, ",%u", pudwTroop[idx]);
        }
        pCur += udwCurLen;
    }
    udwCurLen = sprintf(pCur, "]");
    pCur += udwCurLen;

    udwOutLen = pCur - pszOut;
}

TVOID CCommonFunc::ArrayOutput( TCHAR *pszOut, TUINT32 &udwOutLen, TUINT8 *pudwTroop, TUINT32 udwTroopTypeNum, const TCHAR *pszKey, TBOOL bHead )
{
    TUINT32 idx = 0;
    TUINT32 udwCurLen = 0;
    TCHAR *pCur = pszOut;

    if(bHead == TRUE)
    {
        udwCurLen = sprintf(pCur, "\"%s\":[", pszKey);
        pCur += udwCurLen;
    }
    else
    {
        udwCurLen = sprintf(pCur, ",\"%s\":[", pszKey);
        pCur += udwCurLen;
    }

    for(idx = 0; idx < udwTroopTypeNum; idx++)
    {
        if(idx == 0)
        {
            udwCurLen = sprintf(pCur, "%u", pudwTroop[idx]);
        }
        else
        {
            udwCurLen = sprintf(pCur, ",%u", pudwTroop[idx]);
        }
        pCur += udwCurLen;
    }
    udwCurLen = sprintf(pCur, "]");
    pCur += udwCurLen;

    udwOutLen = pCur - pszOut;
}

int CCommonFunc::JsonEncode( const char *input_buf, char *output_buf, size_t buf_size )
{
    // i指明输入串位置,j指明输出串位置 
    size_t i = 0;
    size_t j = 0;
    int dwcharlen = 0;

    while (input_buf[i] != '\0')
    {
        // 保证剩余的buf空间至少大于6
        if (buf_size - j - 1 <= 6)
        {
            output_buf[j] = '\0';
            return -1;
        }

        dwcharlen = CUtf8Util::charlen(input_buf+i);
        if(dwcharlen == 1)
        {
            switch (input_buf[i])
            {
            case '"':
                //strcpy(output_buf + j, "\\\'");
                //i++;
                //j += 2;
                output_buf[j] = '\'';
                i++;
                j++;
                break;
            case '/':
                strcpy(output_buf + j, "\\/");
                i++;
                j += 2;
                break;
            case '\\':
                strcpy(output_buf + j, "\\\\");
                i++;
                j += 2;
                break;
            case '\r':
                strcpy(output_buf + j, "\\r");
                i++;
                j += 2;
                break;
            case '\n':
                strcpy(output_buf + j, "\\n");
                i++;
                j += 2;
                break;
            case '\t':
                strcpy(output_buf + j, "\\t");
                i++;
                j += 2;
                break;
            case '\b':
                strcpy(output_buf + j, "\\b");
                i++;
                j += 2;
                break;
            case '\f':
                strcpy(output_buf + j, "\\f");
                i++;
                j += 2;
                break;
            default:
                output_buf[j++] = input_buf[i++];
                break;
            }
        }
        else
        {
            memcpy(output_buf + j, input_buf + i, dwcharlen);
            i += dwcharlen;
            j += dwcharlen;
        }
    }
    output_buf[j] = '\0';

    return 0;
}

int CCommonFunc::SqlEncode(const char *input_buf, char *output_buf, size_t buf_size)
{
    // i指明输入串位置,j指明输出串位置
    size_t i = 0;
    size_t j = 0;
    int dwcharlen = 0;

    while (input_buf[i] != '\0')
    {
        // 保证剩余的buf空间至少大于6
        if (buf_size - j - 1 <= 6)
        {
            output_buf[j] = '\0';
            return -1;
        }

        dwcharlen = CUtf8Util::charlen(input_buf+i);
        if(dwcharlen == 1)
        {
            switch (input_buf[i])
            {
            case '\'':
		        strcpy(output_buf + j, "\\\'");
		        i++;
		        j += 2;
		        break;
	            case '!':
		        strcpy(output_buf + j, "\\!");
		        i++;
		        j += 2;
		        break;
            case '"':
                strcpy(output_buf + j, "\\\"");
                i++;
                j += 2;
                break;
            case '/':
                strcpy(output_buf + j, "\\/");
                i++;
                j += 2;
                break;
            case '\\':
                strcpy(output_buf + j, "\\\\");
                i++;
                j += 2;
                break;
            case '\r':
                strcpy(output_buf + j, "\\r");
                i++;
                j += 2;
                break;
            case '\n':
                strcpy(output_buf + j, "\\n");
                i++;
                j += 2;
                break;
            case '\t':
                strcpy(output_buf + j, "\\t");
                i++;
                j += 2;
                break;
            case '\b':
                strcpy(output_buf + j, "\\b");
                i++;
                j += 2;
                break;
            case '\f':
                strcpy(output_buf + j, "\\f");
                i++;
                j += 2;
                break;
            default:
                output_buf[j++] = input_buf[i++];
                break;
            }
        }
        else
        {
            memcpy(output_buf + j, input_buf + i, dwcharlen);
            i += dwcharlen;
            j += dwcharlen;
        }
    }
    output_buf[j] = '\0';

    return 0;
}


int CCommonFunc::ShellEncode(const char *input_buf, char *output_buf, size_t buf_size)
{
    // i???????λ??,j??????λ??
    size_t i = 0;
    size_t j = 0;
    int dwcharlen = 0;

    while (input_buf[i] != '\0')
    {
        // ???????buf??????????6
        if (buf_size - j - 1 <= 6)
        {
            output_buf[j] = '\0';
            return -1;
        }

        dwcharlen = CUtf8Util::charlen(input_buf + i);
        if (dwcharlen == 1)
        {
            switch (input_buf[i])
            {
            case '"':
                strcpy(output_buf + j, "\\\"");
                i++;
                j += 2;
                break;
            default:
                output_buf[j++] = input_buf[i++];
                break;
            }
        }
        else
        {
            memcpy(output_buf + j, input_buf + i, dwcharlen);
            i += dwcharlen;
            j += dwcharlen;
        }
    }
    output_buf[j] = '\0';

    return 0;
}

void CCommonFunc::GetTINT64ArrayFromString(const TCHAR *pszStr, const TCHAR ch, TINT64 *pddwList, TUINT32 &udwNum)
{
    TUINT32 udwMaxNum = udwNum;
    const TCHAR *pCur = pszStr;

    memset((TCHAR*)pddwList, 0, sizeof(TINT64)*udwMaxNum);

    udwNum = 0;

    while(pCur && *pCur && udwNum < udwMaxNum)
    {
        pddwList[udwNum++] = strtoll(pCur, NULL, 10);
        pCur = strchr(pCur, ch);
        if(pCur)
        {
            pCur++;
        }
    }
}

void CCommonFunc::GetVectorFromString(const char* pszBuf, const char ch, vector<TUINT32>& vecList)
{
    vecList.clear();
    char* p = const_cast<char*>(pszBuf);
    char* q = strchr(p, ch);
    TUINT32 udwTmp;
    while(p && *p)
    {
        udwTmp = strtoul(p, NULL, 10);
        vecList.push_back(udwTmp);

        if (q)
        {
            p = q + 1;
            q = strchr(p, ch);
        }
        else
        {
            p = NULL;
        }
    };

}

void CCommonFunc::GetVectorFromString(const char* pszBuf, const char ch, vector<string>& vecList)
{
    vecList.clear();
    char* p = const_cast<char*>(pszBuf);
    char* q = strchr(p, ch);
    string sTmp;
    while(p && *p)
    {
        sTmp.clear();
        if (q)
        {
            sTmp.append(p, q-p);
        }
        else
        {
            sTmp.append(p);
        }
        vecList.push_back(sTmp);

        if (q)
        {
            p = q + 1;
            q = strchr(p, ch);
        }
        else
        {
            p = NULL;
        }
    };
}

TVOID CCommonFunc::CbArrayOutput( TCHAR *pszOut, TUINT32 &udwOutLen, TUINT32 *pudwTroop, TUINT32 udwTroopTypeNum, const TCHAR *pszKey, TBOOL bHead )
{
    TUINT32 idx = 0;
    TUINT32 udwCurLen = 0;
    TCHAR *pCur = pszOut;

    for(idx = 0; idx < udwTroopTypeNum; idx++)
    {
        if(idx == 0)
        {
            udwCurLen = sprintf(pCur, "%s%u", pszKey, pudwTroop[idx]);
        }
        else
        {
            if(pszKey[0] == '\t')
            {
                udwCurLen = sprintf(pCur, "|%u", pudwTroop[idx]);
            }
            else
            {
                udwCurLen = sprintf(pCur, "#%u", pudwTroop[idx]);
            }
        }
        pCur += udwCurLen;
    }

    udwOutLen = pCur - pszOut;
}

TVOID CCommonFunc::CbArrayOutput( TCHAR *pszOut, TUINT32 &udwOutLen, TUINT8 *pudwTroop, TUINT32 udwTroopTypeNum, const TCHAR *pszKey, TBOOL bHead )
{
    TUINT32 idx = 0;
    TUINT32 udwCurLen = 0;
    TCHAR *pCur = pszOut;

    for(idx = 0; idx < udwTroopTypeNum; idx++)
    {
        if(idx == 0)
        {
            udwCurLen = sprintf(pCur, "%s%u", pszKey, pudwTroop[idx]);
        }
        else
        {
            udwCurLen = sprintf(pCur, "#%u", pudwTroop[idx]);
        }
        pCur += udwCurLen;
    }
    udwOutLen = pCur - pszOut;
}

TVOID CCommonFunc::CbArrayOutput( TCHAR *pszOut, TUINT32 &udwOutLen, TINT64 *pddwTroop, TUINT32 udwTroopTypeNum, const TCHAR *pszKey, TBOOL bHead )
{
    TUINT32 idx = 0;
    TUINT32 udwCurLen = 0;
    TCHAR *pCur = pszOut;

    for(idx = 0; idx < udwTroopTypeNum; idx++)
    {
        if(idx == 0)
        {
            udwCurLen = sprintf(pCur, "%s%ld", pszKey, pddwTroop[idx]);
        }
        else
        {
            udwCurLen = sprintf(pCur, "#%ld", pddwTroop[idx]);
        }
        pCur += udwCurLen;
    }
    udwOutLen = pCur - pszOut;
}

template<>
//TUINT8 is not Number!Must be specialization!
string CCommonFunc::NumToString<TUINT8>(TUINT8 Input)
{
    ostringstream oss;
    oss << (TUINT32)Input;
    return oss.str();
}