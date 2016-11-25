#ifndef _SEARCH_NET_IO_H_
#define _SEARCH_NET_IO_H_

#include "std_header.h"
#include "aws_rsp_info.h"

class CSearchNetIO : public ITasksGroupCallBack
{
public:
	// ��ʼ��
	TINT32 Init( CConf *pobjConf, CTseLogger *pLog, CTseLogger *pRegLog, CTaskQueue *poTaskQueue );

	// ����IO�߳���������
	static TVOID * RoutineNetIO(void *pParam);

	// ������Ϣ�ص�
	virtual void OnTasksFinishedCallBack(LTasksGroup *pTasksgrp);

	// ������Ϣ����
	virtual void OnUserRequest(LongConnHandle hSession, const TUCHAR *pData, TUINT32 uLen, BOOL &bWillResponse);

private:
	// ������Ӧ��Ϣ
    TINT32 OnDbResponse(LTasksGroup *pstTasksGrp, SSession *poSessio);
	TINT32 OnAwsResponse(LTasksGroup *pstTasksGrp, SSession *poSession);
    TINT32 OnEventResponse(LTasksGroup *pstTasksGrp, SSession *poSession);
    TINT32 OnCacheResponse(LTasksGroup *pstTasksGrp, SSession *poSession);
    TINT32 OnTranslateResponse(LTasksGroup *pstTasksGrp, SSession *poSession);
    TINT32 OnDataCenterResponse(LTasksGroup *pstTasksGrp, SSession *poSession);
    TINT32 OnIapSvrResponse(LTasksGroup *pstTasksGrp, SSession *poSession);
    TINT32 OnMapSvrResponse(LTasksGroup *pstTasksGrp, SSession *poSession);
    TINT32 OnActionSvrResponse(LTasksGroup *pstTasksGrp, SSession *poSession);
    TINT32 OnReportSvrResponse(LTasksGroup *pstTasksGrp, SSession *poSession);
    TINT32 OnPurchaseCheckResponse(LTasksGroup *pstTasksGrp, SSession *poSession);
    TINT32 OnRankSvrResponse(LTasksGroup *pstTasksGrp, SSession *poSession);

	//TINT32	ParseHsResponse(TUCHAR *pszPack, TUINT32 udwPackLen, SHsResInfo *pstHsResInfo);
    TINT32 ParseDbResponse(TUCHAR *pszPack, TUINT32 udwPackLen,  DbRspInfo* pDbRspInfo);
	TINT32 ParseAwsResponse(TUCHAR *pszPack, TUINT32 udwPackLen, AwsRspInfo* pAwsRspInfo);
    TINT32 ParseEventResponse(TUCHAR *pszPack, TUINT32 udwPackLen, EventRspInfo* pAwsRspInfo);
    TINT32 ParseCacheResponse(TUCHAR *pszPack, TUINT32 udwPackLen, CacheRspInfo* pCacheRspInfo);
    TINT32 ParseTranslateResponse(TUCHAR *pszPack, TUINT32 udwPackLen, TranslateRspInfo* pTranslateRspInfo);
    TINT32 ParseDataCenterResponse(TUCHAR *pszPack, TUINT32 udwPackLen, DataCenterRspInfo* pDataCenterRspInfo);
    TINT32 ParseIapSvrResponse(TUCHAR *pszPack, TUINT32 udwPackLen, IapSvrRspInfo* pIapSvrRspInfo);
    TINT32 ParseMapSvrResponse(TUCHAR *pszPack, TUINT32 udwPackLen, AwsRspInfo* pAwsRspInfo);
    TINT32 ParseActionSvrResponse(TUCHAR *pszPack, TUINT32 udwPackLen, AwsRspInfo* pAwsRspInfo);
    TINT32 ParseReportSvrResponse(TUCHAR *pszPack, TUINT32 udwPackLen, ReportRspInfo* pReportRspInfo);
    TINT32 ParsePurchaseCheckResponse(TUCHAR *pszPack, TUINT32 udwPackLen, PurchaseRspInfo* pPurchaseRspInfo);
    TINT32 ParseRankSvrResponse(TUCHAR *pszPack, TUINT32 udwPackLen, RankSvrRspInfo* pRankRspInfo);

	TINT32 OnLockResponse(LTasksGroup *pstTasksGrp, SSession *poSession);

private:
	//  ��������socket
	SOCKET CreateListenSocket(TCHAR *pszListenHost, TUINT16 uwPort);

	// �رռ���socket
	TINT32 CloseListenSocket();

	// ��ȡip�Ͷ˿�
	TINT32 GetIp2PortByHandle(LongConnHandle stHandle, TUINT16 *puwPort, TCHAR **ppszIp);

public:
	// �����Ӷ���
	ILongConn               *m_pLongConn;
	// ��־����
	CTseLogger				*m_poServLog;
	CTseLogger				*m_poRegLog;
	// session����
	CTaskQueue				*m_poTaskQueue;
	// ����
	CConf					*m_poConf;
	// ���/�������
	CBaseProtocolPack       *m_pPackTool;
	CBaseProtocolUnpack     *m_pUnPackTool;
	// ����socket
	TINT32                  m_hListenSock;
};

#endif
