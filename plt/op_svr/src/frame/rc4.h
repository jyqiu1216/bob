#ifndef _ENCRYPT_RC4_H_
#define _ENCRYPT_RC4_H_

#include <cstdlib>
#include <cstdio>
#include <string.h>

#define MAX_RC4_KEY_LEN	(32)

class CEncryptCR4
{
private:
	unsigned char m_state[256];

public:
	float	m_fVersion;
	char	m_szkey[MAX_RC4_KEY_LEN];

public:
	int init(const char* pszKey, const float fVersion);

	int prepare_key(const char* pszKey, int dwKeyLen);

	int rc4(unsigned char *buffer_ptr, int buffer_len);
	int rc4(const unsigned char *buffer_ptr, int buffer_len, unsigned char *out_ptr, int &out_len);
};

class CEncryptCR4Mgr
{
public:
	static CEncryptCR4Mgr* get_instance();
	static CEncryptCR4Mgr *m_pobjEncryptCR4Mgr;

public:
	CEncryptCR4 *m_pobjEncryptList;
	int m_dwEncryptListNum;

public:
	int Init();
	CEncryptCR4* GetEncrypt(const float fVersion);
};

#endif