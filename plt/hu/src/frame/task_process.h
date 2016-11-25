#ifndef _SSC_TASK_PROCESS_H_
#define _SSC_TASK_PROCESS_H_

#include "std_header.h"
#include "des.h"
#include "util_base64.h"
#include "npc_upgrade.h"
#include "process_base.h"
#include "json_result_generator.h"

class CTaskProcess
{
public:
	// ��ʼ��
	TINT32 Init(CConf *poConf, ILongConn *poSearchLongConn, ILongConn *poQueryLongConn, 
		CTaskQueue *pTaskQueue, CTseLogger *poServLog, CTseLogger *poDbLog);
	// �߳���ں���
	static void *Start(void *pParam);
	// ���ԴӶ������ȡ����
	TINT32 WorkRoutine();

public:
	// function  ===>����client������(����ʱ��Ҫ��֤session���ͷźͿͻ��˵ķ���)
	TINT32 ProcessClientRequest(SSession *pstSession);

    // function  ===> normal/special���̵�ѡȡ
	TINT32 ProcessCommand(SSession *pstSession);
    
    // function  ===> normal�����ִ����������(��Ҫ��֤session���ͷźͿͻ��˵ķ���)
    // in_value  ===> pstSession: ҵ��session
    //           ===> pBaseCmd: �����ֵĴ�����Ļ���,ָ�������������
    TVOID NormalMainProdure(SSession *pstSession, const TINT32 dwCmdType);

    // function  ===> special�����ִ����������(��Ҫ��֤session���ͷźͿͻ��˵ķ���)
    // in_value  ===> pstSession: ҵ��session
    //           ===> pBaseCmd: �����ֵĴ�����Ļ���,ָ�������������
    TVOID SpecialMainProdure(SSession *pstSession, const TINT32 dwCmdType);
    
private:
    TINT32 SendBackResult(SSession *pstSession);
    TINT32 SendBackResult_Http(SSession *pstSession);
    TINT32 SendBackResult_Binary(SSession *pstSession);

    TVOID ResetSessionTmpParam(SSession *pstSession);
    TVOID PreprocessReq(SSession *pstSession);
    TVOID GetLockedTasked(SSession *pstSession);
    const char* GetStr(const char* pszStr); 
    TVOID StoreJson(SSession* pstSession,TBOOL bRefresh = FALSE, TBOOL bToFilter = TRUE);
    TVOID StoreJson_String(SSession* pstSession,TBOOL bRefresh, TBOOL bToFilter);
    TVOID StoreJson_Binary(SSession* pstSession,TBOOL bRefresh, TBOOL bToFilter);

    //wave@20160709: for ��������
    TINT32 UpdateDataOutputType(SSession* pstSession);

private:
	/*****************************************
	���ܽ������
	*****************************************/
	bool EncryptUrl( char *pszIn, char *pszOut, int &dwOutLen, float fVersion);
	bool DecryptUrl( char *pszIn, char *pszOut, int &dwOutLen, float fVersion);
	CDes				*m_pobjEncrypt;
	TCHAR				m_szEncryptBuf[MAX_HTTP_REQ_LEN];
	TCHAR				m_szEncryptUrl[MAX_HTTP_REQ_LEN];

public:
	// �����ļ�����
	CConf               *m_poConf;
	ILongConn           *m_poSearchLongConn;
	ILongConn			*m_poQueryLongConn;

	// ��־����
	CTseLogger          *m_poServLog;
	CTseLogger			*m_poDayLog;

	// �������
	CTaskQueue          *m_pTaskQueue;

	// ���/�����
	CBaseProtocolPack   *m_pPackTool[MAX_AWS_REQ_TASK_NUM];
    //CBaseProtocolPack   *m_pPackTool;
	CBaseProtocolUnpack *m_pUnPackTool;

    // Json������
    CJsonResultGenerator m_oJsonResultGenerator;

	// hs������Ͱ���
	//SHsReqNode			m_astHsReqNode[MAX_LOCAL_HS_TASK_NUM];
	TUCHAR				m_szReqBuf[MAX_REQ_BUF_LEN];
	// mail
	//STableMail			m_astMail[MAX_MAIL_PERPAGE_NUM];
	// report
	//STableReport		m_astReport[MAX_REPORT_PERPAGE_NUM];

private:
	//�м�ʹ�õı���////////////////////////////////////
	// http��k-v��
	RequestParam		m_stHttpParam;
	// tmp buf
	TCHAR				*m_pTmpBuf;
	TCHAR				*m_pMd5Buf;

public:
	CNpcUpgrade *m_pobjNpcUpgrade;

private:
    
};

#endif
