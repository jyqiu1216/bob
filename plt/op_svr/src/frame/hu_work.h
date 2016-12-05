#ifndef TSHP_HU_HU_WORK_H
#define TSHP_HU_HU_WORK_H

#include "std_header.h"
#include "encode/src/encode/string_utils.h"
#include "key_value.h"
#include "base/arithmatic/md5int.h"
#include "http_utils.h"

class CHuWork
{
public:
    /*
     * ����ǰ������ 1.��ȡ����url 2.��ȡ����������
     */
	static int GetRequestUrl(const char *req_buf, char *url_buf);
    static int GetRequestParam(SSession *session, CTseLogger *serv_log, RequestParam *req_param);

public:
	 /*
     * �������ַ�����ת���Է���json��׼
     */
    static int JsonEncode(char *input_buf, char *output_buf, size_t buf_size);

private:
	// ��ȡhttp��key-value��
	static int GetHttpParam(const TCHAR *url_buf, RequestParam *req_param);
	// ��http��key-value��Ӧ����������
	static int ProcessReqParam(RequestParam *pstReqParam, SSession *pstSession);

public:
	static TUINT64	GetUserMd5(TCHAR *pszStr);
    static TUINT64  GetRawDataMd5(const TUCHAR *pszData, const TUINT32 udwDataLen);
	static TUINT64	GetStringMD5(TCHAR* pszStr);
    static TUINT64 GetStringMD5(const TCHAR* pszStr, TCHAR* pMD5Buffer);
	static TVOID	ProcessString(TCHAR *pszStr);
    
};

#endif
