#ifndef _TCP_SOCKET_H_
#define _TCP_SOCKET_H_

#pragma once
#include "GTCPSocket.h"

#define BUFFER_SIZE 65536

class CTcpSocket : public GTCPSocket

{
public:

	CTcpSocket();
	virtual ~CTcpSocket();

	LPVOID m_UserData;
	void SetCallbackFunc(void (*OnAccept)(CTcpSocket*,LPVOID),void (*OnReceive)(CTcpSocket*,LPVOID),void (*OnClose)(CTcpSocket*,LPVOID,BOOL),LPVOID UserData);
	int Connect(int ip,int port);
	int Listen(int port);
	
	virtual void OnAccept(int nErrorCode);
	virtual void OnClose(int nErrorCode);
	virtual void OnReceive(int nErrorCode);

	void (*m_pOnAccept)(CTcpSocket*,LPVOID);
	void (*m_pOnClose)(CTcpSocket*,LPVOID,BOOL);
	void (*m_pOnReceive)(CTcpSocket*,LPVOID);
};

#endif
