#ifndef _GTCP_SOCKET_H_
#define _GTCP_SOCKET_H_
#pragma once
#include <winsock2.h>
#include <Ws2tcpip.h>
#define NETWORK_EVENT	0
#define CLOSE_EVENT		1
#define TOTAL_EVENT		2
class GTCPSocket
{
public:
	GTCPSocket(void);
	virtual ~GTCPSocket(void);
	
	SOCKET m_sSocket;
	
	int Accept(GTCPSocket& GSocket);
	int Connect(int nIp, int nPort);
	int Connect(const char* szIP, int nPort);
	int Close();
	int Send(void* buffer, int size);
	int Listen(int nPort, int nBackLog = 5);
	int Receive(void* buffer, int size);

	int	GetPeerName();
	int GetPeerName(BYTE& field0, BYTE& field1, BYTE& field2, BYTE& field3);
	bool GetPeerName(int& nIp);

protected:
	virtual void OnClose(int nErrorCode);
	virtual void OnAccept(int nErrorCode);
	virtual void OnReceive(int nErrorCode);
	virtual void OnConnect(int nErrorCode);
	virtual void OnSend(int nErrorCode);

	
private:		
	WSAEVENT m_event[TOTAL_EVENT];
	WSANETWORKEVENTS NetworkEvents;
	HANDLE	m_hMainThread;

private:
	bool Run();	
	GTCPSocket& operator=(GTCPSocket&);
	static unsigned long __stdcall MainThread(void* param);
	unsigned long Main();
};
#endif