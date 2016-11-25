#ifndef _SEARCH_NET_IO_H_
#define _SEARCH_NET_IO_H_

#include "std_header.h"
#include "aws_rsp_info.h"

class CSearchNetIO : public ITasksGroupCallBack
{
public:
	// 初始化
	TINT32 Init( CConf *pobjConf, CTseLogger *pLog, CTseLogger *pRegLog, CTaskQueue *poTaskQueue );

	// 网络IO线程驱动函数
	static TVOID * RoutineNetIO(void *pParam);

	// 网络消息回调
	virtual void OnTasksFinishedCallBack(LTasksGroup *pTasksgrp);

	// 网络消息请求
	virtual void OnUserRequest(LongConnHandle hSession, const TUCHAR *pData, TUINT32 uLen, BOOL &bWillResponse);

private:
	// 处理响应消息
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
	//  创建监听socket
	SOCKET CreateListenSocket(TCHAR *pszListenHost, TUINT16 uwPort);

	// 关闭监听socket
	TINT32 CloseListenSocket();

	// 获取ip和端口
	TINT32 GetIp2PortByHandle(LongConnHandle stHandle, TUINT16 *puwPort, TCHAR **ppszIp);

public:
	// 长连接对象
	ILongConn               *m_pLongConn;
	// 日志对象
	CTseLogger				*m_poServLog;
	CTseLogger				*m_poRegLog;
	// session队列
	CTaskQueue				*m_poTaskQueue;
	// 配置
	CConf					*m_poConf;
	// 打包/解包工具
	CBaseProtocolPack       *m_pPackTool;
	CBaseProtocolUnpack     *m_pUnPackTool;
	// 监听socket
	TINT32                  m_hListenSock;
};

#endif
