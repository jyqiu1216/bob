#ifndef _COMMON_FUNC_H_
#define _COMMON_FUNC_H_

#include "base/common/wtse_std_header.h"
//#include "bin_struct_define.h"
#include "bussiness_struct.h"
#include <sstream>
#include <string>
#include <vector>

using namespace std;

class CCommonFunc
{
public:
    static void UIntToString(unsigned int key, char*pstr);
    static string GetTableRawName(const string& strTableName);
public:
    static TVOID ArrayOutput(TCHAR *pszOut, TUINT32 &udwOutLen, TUINT32 *pudwTroop, 
        TUINT32 udwTroopTypeNum, const TCHAR *pszKey, TBOOL bHead);

    static TVOID ArrayOutput(TCHAR *pszOut, TUINT32 &udwOutLen, TUINT8 *pudwTroop, 
        TUINT32 udwTroopTypeNum, const TCHAR *pszKey, TBOOL bHead);
public:
    static TVOID CbArrayOutput(TCHAR *pszOut, TUINT32 &udwOutLen, TUINT32 *pudwTroop, 
        TUINT32 udwTroopTypeNum, const TCHAR *pszKey, TBOOL bHead);

    static TVOID CbArrayOutput(TCHAR *pszOut, TUINT32 &udwOutLen, TUINT8 *pudwTroop, 
        TUINT32 udwTroopTypeNum, const TCHAR *pszKey, TBOOL bHead);

    static TVOID CbArrayOutput(TCHAR *pszOut, TUINT32 &udwOutLen, TINT64 *pddwTroop, 
        TUINT32 udwTroopTypeNum, const TCHAR *pszKey, TBOOL bHead);
    
public:
    static int JsonEncode(const char *input_buf, char *output_buf, size_t buf_size);
    static int SqlEncode(const char *input_buf, char *output_buf, size_t buf_size);
    static int ShellEncode(const char *input_buf, char *output_buf, size_t buf_size);

public:
    template <typename N>
    static void GetArrayFromString(const TCHAR *pszStr, const TCHAR ch, N *numList, TUINT32 &udwNum)
    {
        TUINT32 udwMaxNum = udwNum;
        const TCHAR *pCur = pszStr;

        memset((TCHAR*)numList, 0, sizeof(N)*udwMaxNum);

        udwNum = 0;

        while (pCur && *pCur && udwNum < udwMaxNum)
        {
            numList[udwNum++] = strtoul(pCur, NULL, 10);
            pCur = strchr(pCur, ch);
            if (pCur)
            {
                pCur++;
            }
        }
    }

    static void GetTINT64ArrayFromString(const TCHAR *pszStr, const TCHAR ch, TINT64 *pddwList, TUINT32 &udwNum);
    static void GetVectorFromString(const char* pszBuf, const char ch, vector<TUINT32>& vecList);
    static void GetVectorFromString(const char* pszBuf, const char ch, vector<string>& vecList);

    static bool isNum(string str)  // 判断字符串是不是数字
    {
        stringstream sin(str);
        double d;
        char c;
        if (!(sin >> d))
            return false;
        if (sin >> c)
            return false;
        return true;
    }

    template<typename TNumber>
    static string NumToString(TNumber Input)
    {
        ostringstream oss;
        oss << Input;
        return oss.str();
    }
    static string NameToStar(string name)
    {
        string res;
        res.clear();

        /*
        for(int i = 0; i < name.size(); i++)
        {
            if(i >= 3)
            {
                res.append("*");
            }
            else
            {
                res = res + name.c_str()[i];
            }
        }   
        */
        for(unsigned int i = 0; i < name.size(); i++)
        {
            res.append("*");
        }
        
        return res;
    }

};

template<typename T>
class AutoDel
{
public:
    AutoDel(T* object, TBOOL bIsArray);
    ~AutoDel();

private:
    TBOOL _bIsArray;
    T* _obejct;
};

template<typename T>
AutoDel<T>::~AutoDel()
{
    if(_obejct != NULL)
    {
        if(_bIsArray)
        {
            delete[] _obejct;
        }
        else
        {
            delete _obejct;
        }
    }
}

template<typename T>
AutoDel<T>::AutoDel(T* object, TBOOL bIsArray)
{
    _obejct = object;
    _bIsArray = bIsArray;
}

#endif
