#include "rc4.h"
#include "conf.h"
#include "global_serv.h"
#include "conf_base.h"

CEncryptCR4Mgr* CEncryptCR4Mgr::m_pobjEncryptCR4Mgr = NULL;

int CEncryptCR4::prepare_key( const char* key, int key_len )
{
	unsigned char t[256];
	int i=0, j=0;
	for(i=0;i<256;i++)//////////////给字节状态矢量和可变长的密钥数组赋值
	{
		m_state[i] = i;
		t[i] = key[i % key_len];
	}

	for(i=0;i<256;i++) //////使用可变长的密钥数组初始化字节状态矢量数组s
	{
		unsigned char temp;
		j=(j+m_state[i]+t[i])%256;
		temp=m_state[i];
		m_state[i]=m_state[j];
		m_state[j]=temp;
	}
	return 0;
}

int CEncryptCR4::rc4( unsigned char *buffer_ptr, int buffer_len )
{
	unsigned char state[256];
	int i=0;

	memcpy(&state[0], &m_state[0], 256);

	int m,n,q;
	m=n=0;
	for(i=0;i<buffer_len;i++)/////////////由字节状态矢量数组变换生成密钥流并对明文字符进行加密
	{
		int temp;
		m=(m+1)% 256;
		n=(n+state[n])% 256;
		temp=state[m];
		state[m]=state[n];
		state[n]=temp;
		q=(state[m]+state[n])%256;

		buffer_ptr[i] ^= state[q];
	}

	buffer_ptr[i] = 0;

	return 0;
}

int CEncryptCR4::rc4( const unsigned char *buffer_ptr, int buffer_len, unsigned char *out_ptr, int &out_len )
{
	unsigned char state[256];
	int i=0;

	memcpy(&state[0], &m_state[0], 256);

	int m,n,q;
	m=n=0;
	for(i=0;i<buffer_len;i++)/////////////由字节状态矢量数组变换生成密钥流并对明文字符进行加密
	{
		int temp;
		m=(m+1)% 256;
		n=(n+state[n])% 256;
		temp=state[m];
		state[m]=state[n];
		state[n]=temp;
		q=(state[m]+state[n])%256;

		out_ptr[i] = buffer_ptr[i]^state[q];
	}
	out_ptr[i] = 0;
	out_len = i;
	return 0;
}

int CEncryptCR4::init( const char* pszKey, const float fVersion )
{
	m_fVersion = fVersion;
	strncpy(m_szkey, pszKey, MAX_RC4_KEY_LEN-1);
	m_szkey[MAX_RC4_KEY_LEN-1] = 0;

	return prepare_key(pszKey, strlen(pszKey));
}


CEncryptCR4Mgr* CEncryptCR4Mgr::get_instance()
{
	if(m_pobjEncryptCR4Mgr == NULL)
	{
		m_pobjEncryptCR4Mgr = new CEncryptCR4Mgr;
		m_pobjEncryptCR4Mgr->Init();
	}
	return m_pobjEncryptCR4Mgr;
}

int CEncryptCR4Mgr::Init()
{
	
	int ret = 0;

	m_dwEncryptListNum = CConfBase::GetInt("m_version_num", "other");
	m_pobjEncryptList = new CEncryptCR4[m_dwEncryptListNum];
    string szVersion;
    string szVersionKey;
    ostringstream oss;
    for (TINT32 idx = 0; idx < CConfBase::GetInt("m_version_num", "other"); idx++)
	{
        oss.str("");
        oss << "m_version_" << idx;
        szVersion = oss.str();
        oss.str("");
        oss << "m_version_key_" << idx;
        szVersionKey = oss.str();
        ret = m_pobjEncryptList[idx].init(CConfBase::GetString(szVersionKey, "other").c_str(), CConfBase::GetFloat(szVersion, "other"));
		assert(ret == 0);
	}

	return 0;
}

CEncryptCR4* CEncryptCR4Mgr::GetEncrypt( const float fVersion )
{
	int idx = 0;
	CEncryptCR4 *poEncrypt = NULL;

	for(idx = 0; idx < m_dwEncryptListNum; idx++)
	{
		if(fabs(m_pobjEncryptList[idx].m_fVersion - fVersion) < 0.0001)
		{
			poEncrypt = &m_pobjEncryptList[idx];
			break;
		}
	}
	if (NULL == poEncrypt)
	{
		poEncrypt = &m_pobjEncryptList[0];
	}

	return poEncrypt;
}
