#include "aws_table_common.h"
#include "encode/src/encode/utf8_util.h"

map<string, AwsTable* (*)()> AwsTable::mNewObjFunc;


/*string AwsTable::GetClassName(const string& sTableName, int dwSvrIdx, int dwTableIdx)
{
	size_t begin = -1;
	if (dwSvrIdx >= 0)
	{
		begin = sTableName.find_first_of('_');
		assert(begin!= string::npos);
	}
	size_t end = sTableName.size();
	if (dwTableIdx >= 0)
	{
		end = sTableName.find_last_of('_');
		assert(begin < end);
	}
	return sTableName.substr(begin+1, end-begin-1);
}
*/

AwsTable* AwsTable::NewObject(const string& sClassName)
{
	return mNewObjFunc[sClassName]();
}

void AwsTable::AddNewObjectFunc(const string& sClassName, AwsTable* (*pFunc)())
{
	mNewObjFunc[sClassName] = pFunc;
}

char* AwsTable::CompareType2Str(int  dwCompareType)
{
	switch(dwCompareType)
	{
		case COMPARE_TYPE_EQ:
			return "EQ";
		case COMPARE_TYPE_LE:
			return "LE";
		case COMPARE_TYPE_LT:
			return "LT";
		case COMPARE_TYPE_GE:
			return "GE";
		case COMPARE_TYPE_GT:
			return "GT";
		case COMPARE_TYPE_BEGINS_WITH:
			return "BEGINS_WITH";
		case COMPARE_TYPE_BETWEEN:
			return "BETWEEN";
		default:
			assert(0);
	}
	return NULL;
	
}

char* AwsTable::DataType2Str(int dwDataType)
{
	switch(dwDataType)
	{
		case FIELD_TYPE_N:
			return "N";
		case FIELD_TYPE_S:
        case FIELD_TYPE_J:
			return "S";
		case FIELD_TYPE_B:
			return "B";
		default:
			assert(0);
	}
	return NULL;
}

char* AwsTable::UpdateActionType2Str(int dwUpdateActionType)
{
	switch(dwUpdateActionType)
	{
		case UPDATE_ACTION_TYPE_PUT:
			return "PUT";
		case UPDATE_ACTION_TYPE_DELETE:
			return "DELETE";
		case UPDATE_ACTION_TYPE_ADD:
			return "ADD";
		default:
			assert(0);
	}
	return NULL;
}

char* AwsTable::ReturnValuesType2Str(int dwReturnValuesType)
{
	switch(dwReturnValuesType)
	{
		case RETURN_VALUES_NONE:
			return "NONE";
		case RETURN_VALUES_ALL_OLD:
			return "ALL_OLD";
		case RETURN_VALUES_UPDATED_OLD:
			return "UPDATED_OLD";
		case RETURN_VALUES_ALL_NEW:
			return "ALL_NEW";
		case RETURN_VALUES_UPDATED_NEW:
			return "UPDATED_NEW";
		default:
			assert(0);
	}
	return NULL;
}

const string& AwsTable::Base64Encode(const string& sSrc, string& sDest)
{
	if (sSrc.empty())
	{	
		sDest = "";
	}
	else
	{
		int dwLen;
        sDest.clear();
		sDest.resize(4*sSrc.size());
		CUtilBase64::encode((char*)sSrc.c_str(), (char*)sSrc.c_str()+sSrc.size(), (char*)sDest.c_str(), dwLen);
		sDest.resize((unsigned int)dwLen);
	}
	return sDest;
}

const string& AwsTable::Base64Encode( char* pszSrc, unsigned int udwSrcLen, string& sDest )
{
    int dwLen;
    sDest.clear();
    sDest.resize(4*udwSrcLen);
    CUtilBase64::encode(pszSrc, pszSrc+udwSrcLen, (char*)sDest.c_str(), dwLen);
    sDest.resize(dwLen);

    return sDest;
}

const string& AwsTable::Base64Decode(const string& sSrc, string& sDest)
{
	if (sSrc.empty())
	{	
		sDest = "";
	}
	else
	{
		int dwLen;
        sDest.clear();
		sDest.resize(sSrc.size());
		CUtilBase64::decode((char*)sSrc.c_str(), (char*)sSrc.c_str()+sSrc.size(), (char*)sDest.c_str(), dwLen);
		sDest.resize((unsigned int)dwLen);
	}
	return sDest;

}

void AwsTable::Base64Decode( const string& sSrc, char* pszDest, int &udwDstLen )
{
    CUtilBase64::decode_bin((char*)sSrc.c_str(), (char*)sSrc.c_str()+sSrc.size(), pszDest, udwDstLen);
}

const string& AwsTable::JsonEncode(const string& sInput, string& sOutPut)
{
    sOutPut.clear();
    sOutPut.resize(sInput.size() * 2 + 512);
    // i指明输入串位置,j指明输出串位置
    size_t i = 0;
    size_t j = 0;
    int dwcharlen = 0;

    while (i < sInput.size())
    {
        // 保证剩余的buf空间至少大于6
        if (sOutPut.size() - j - 1 <= 6)
        {
            sOutPut.resize(j);
            return sOutPut;
        }

        dwcharlen = CUtf8Util::charlen(sInput.c_str()+i);
        if(dwcharlen == 1)
        {
            switch (sInput[i])
            {
            case '"':
                strcpy(const_cast<char *>(sOutPut.c_str()) + j, "\\\"");
                i++;
                j += 2;
                break;
            case '/':
                strcpy(const_cast<char *>(sOutPut.c_str()) + j, "\\/");
                i++;
                j += 2;
                break;
            case '\\':
                strcpy(const_cast<char *>(sOutPut.c_str()) + j, "\\\\");
                i++;
                j += 2;
                break;
            case '\r':
                strcpy(const_cast<char *>(sOutPut.c_str()) + j, "\\r");
                i++;
                j += 2;
                break;
            case '\n':
                strcpy(const_cast<char *>(sOutPut.c_str()) + j, "\\n");
                i++;
                j += 2;
                break;
            case '\t':
                strcpy(const_cast<char *>(sOutPut.c_str()) + j, "\\t");
                i++;
                j += 2;
                break;
            case '\b':
                strcpy(const_cast<char *>(sOutPut.c_str()) + j, "\\b");
                i++;
                j += 2;
                break;
            case '\f':
                strcpy(const_cast<char *>(sOutPut.c_str()) + j, "\\f");
                i++;
                j += 2;
                break;
            default:
                sOutPut[j++] = sInput[i++];
                break;
            }
        }
        else
        {
            memcpy(const_cast<char *>(sOutPut.c_str()) + j, sInput.c_str() + i, dwcharlen);
            i += dwcharlen;
            j += dwcharlen;
        }
    }
    sOutPut.resize(j);
    return sOutPut;
}

int AwsTable::GetTipsTableOffset( TINT64 ddwTime )
{
    TUINT32 month = ddwTime/(1000*1000);
    month = month/(30*24*3600);
    return month%2;
}

int AwsTable::GetMailReportTableOffset( TINT64 ddwId )
{
    return (TUINT16)(ddwId/MAX_MAIL_REPORT_TABLE_SIZE);
}


