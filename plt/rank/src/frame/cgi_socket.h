#ifndef _CGI_SOCKET_H_
#define _CGI_SOCKET_H_

#include "base/os/wtsesocket.h"

class CCgiSocket
{
public:
	//dwSendTimeOut的单位：ms
	static T_SOCKET Connect(const TCHAR* pszIp, TUINT16 uwPort, TINT32 dwSendTimeOut)
	{
		T_SOCKET dwSocket = -1;
		//创建socket
		dwSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if(dwSocket == -1)
		{
			return -1;
		}
		//设置该socket为激活状态
		if(0 != tse_socket_setKeepalive(dwSocket))
		{
			tse_socket_close(dwSocket);
			return -2;
		}

		//连接
		struct sockaddr_in serv_addr;
		memset(&serv_addr, 0, sizeof(struct sockaddr_in));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(uwPort);
		serv_addr.sin_addr.s_addr = inet_addr(pszIp);

		if(0 != connect(dwSocket, (struct sockaddr*)&serv_addr, sizeof(sockaddr_in)))
		{
			tse_socket_close(dwSocket);
			return -3;
		}

		//设置send超时时间
		struct timeval tv;
		tv.tv_sec  = dwSendTimeOut/1000;
		tv.tv_usec = (dwSendTimeOut%1000)*1000;
		if(0 != setsockopt(dwSocket, SOL_SOCKET, SO_SNDTIMEO, (TCHAR *)&tv, sizeof(tv)))
		{
			tse_socket_close(dwSocket);
			return -4;
		}

		return dwSocket;
	}

	static TINT32 Close(T_SOCKET dwSocket)
	{
		return tse_socket_close(dwSocket);
	}

	static TINT32 Send(T_SOCKET sock, TCHAR *buf, TINT32 count)
	{
		return tse_socket_writeFull(sock, buf, count);
	}

	static TINT32 Recv(T_SOCKET sock, TCHAR *buf, TINT32 count)
	{
		TINT32 nWillPackLen = 0;
		TBOOL bReadOK = FALSE;
		TINT32 nGotLen = 0;
		while(1)
		{
			TINT32 nRead;
			nRead = read(sock, buf + nGotLen, count - nGotLen);

			if(nRead > 0)
				nGotLen += nRead;
			else
			{
				bReadOK = FALSE;
				break;
			}
			if(nGotLen >= 4)
			{
				memcpy(&nWillPackLen, buf, sizeof(TINT32));
				nWillPackLen = ntohl(nWillPackLen);
				if(nGotLen >= nWillPackLen)
				{
					bReadOK = TRUE;
					break;
				}
			}
		}	

		if(!bReadOK)
		{
			return -1;
		}

		return nGotLen;
	}
};

#endif
