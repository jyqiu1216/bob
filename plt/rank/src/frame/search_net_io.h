#ifndef _SEARCH_NET_IO_H_
#define _SEARCH_NET_IO_H_

#include "std_header.h"
#include "aws_rsp_info.h"
#include "client_netio.pb.h"

using namespace client_netio_protocol;

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
	TINT32  OnAwsResponse(LTasksGroup *pstTasksGrp, SSession *poSession);
	TINT32	ParseAwsResponse(TUCHAR *pszPack, TUINT32 udwPackLen, AwsRspInfo* pAwsRspInfo);

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

    ClientRequest           *m_pobjClientReq;

    TUINT32 m_udwSeqNo;
};

#endif