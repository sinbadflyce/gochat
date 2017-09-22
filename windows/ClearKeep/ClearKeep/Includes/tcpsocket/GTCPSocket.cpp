#include "GTCPSocket.h"
#pragma warning(disable:4996)

GTCPSocket::GTCPSocket(void)
{
	m_sSocket = INVALID_SOCKET;
	m_event[CLOSE_EVENT] = WSACreateEvent();
	m_event[NETWORK_EVENT] = WSACreateEvent();
	m_hMainThread = INVALID_HANDLE_VALUE;
}

GTCPSocket::~GTCPSocket(void)
{
	Close();
	WSACloseEvent(m_event[NETWORK_EVENT]);
	WSACloseEvent(m_event[CLOSE_EVENT]);
}

int GTCPSocket::Close()
{
	int nResult = -1;
	if(m_sSocket != INVALID_SOCKET)
	{				
		nResult = closesocket(m_sSocket);
		m_sSocket = INVALID_SOCKET;		
		WSASetEvent(m_event[CLOSE_EVENT]);
	}		
	return nResult;
}
bool GTCPSocket::Run()
{	
	WSAResetEvent(m_event[NETWORK_EVENT]);
	WSAResetEvent(m_event[CLOSE_EVENT]);
	WSAEventSelect(m_sSocket, m_event[NETWORK_EVENT], FD_READ | FD_WRITE | FD_CLOSE | FD_CONNECT);
	unsigned long dwThreadId;
	m_hMainThread = CreateThread(NULL, 0, MainThread, this, 0, &dwThreadId);
	if(m_hMainThread == NULL)
	{
		closesocket(m_sSocket);
		m_sSocket = INVALID_SOCKET;
		return false;
	}
	return true;	
}
int GTCPSocket::Accept(GTCPSocket& GSocket)
{
	if(GSocket.m_sSocket != INVALID_SOCKET)
		return false;
	GSocket.m_sSocket = accept(m_sSocket, NULL, NULL);
	if(GSocket.m_sSocket != INVALID_SOCKET)
	{		
		return GSocket.Run();			
	}
	return false;
	
}

int GTCPSocket::Send(void* buffer, int size)
{
	if(m_sSocket == INVALID_SOCKET)
	{
		return -1;
	}
	return send(m_sSocket, (char *)buffer, size, 0);
}
int GTCPSocket::Listen(int nPort, int nBackLog /* = 5 */)
{
	if(m_sSocket != INVALID_SOCKET)
		return 0;

	m_sSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(m_sSocket == SOCKET_ERROR)
	{
		closesocket(m_sSocket);
		m_sSocket = INVALID_SOCKET;
		return 0;
	}

	WSAResetEvent(m_event[NETWORK_EVENT]);
	WSAResetEvent(m_event[CLOSE_EVENT]);
	sockaddr_in internet_address;
	
	internet_address.sin_family = AF_INET;
	internet_address.sin_port = htons(nPort);
	internet_address.sin_addr.s_addr = htonl(INADDR_ANY);

	if(SOCKET_ERROR == bind(m_sSocket, (sockaddr *)&internet_address, sizeof(internet_address)))
	{
		closesocket(m_sSocket);
		m_sSocket = INVALID_SOCKET;
		return 0;
	}

	WSAEventSelect(m_sSocket, m_event[NETWORK_EVENT], FD_ACCEPT | FD_CLOSE);
	listen(m_sSocket, nBackLog);	
	
	unsigned long dwThreadId;
	m_hMainThread = CreateThread(NULL, 0, MainThread, this, 0, &dwThreadId);
	if(!m_hMainThread)
	{
		closesocket(m_sSocket);
		m_sSocket = INVALID_SOCKET;
		return 0;
	}
	return 1;
}

int GTCPSocket::Receive(void* buffer, int size)
{
	if(m_sSocket == INVALID_SOCKET)
		return -1;
	return recv(m_sSocket, (char *)buffer, size, 0);
}
int GTCPSocket::Connect(int nIp, int nPort)
{
	if(m_sSocket != INVALID_SOCKET)
		return 0;

	m_sSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	sockaddr_in server;
	server.sin_port = htons(nPort);
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = nIp;

	if(connect(m_sSocket, (sockaddr *)&server, sizeof(server)) != SOCKET_ERROR)
	{
		return Run();
	}
	closesocket(m_sSocket);
	m_sSocket = INVALID_SOCKET;
	return 0;
}

int GTCPSocket::Connect(const char* szIP, int nPort)
{
	if(m_sSocket != INVALID_SOCKET)
		return 0;

	m_sSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	sockaddr_in server;
	server.sin_port = htons(nPort);
	server.sin_family = AF_INET;
	
	//InetPton(AF_INET, szIP, &server.sin_addr.s_addr)
	server.sin_addr.s_addr = inet_addr(szIP);

	if(connect(m_sSocket, (sockaddr *)&server, sizeof(server)) != SOCKET_ERROR)
	{
		return Run();
	}
	closesocket(m_sSocket);
	m_sSocket = INVALID_SOCKET;
	return 0;
}

int GTCPSocket::GetPeerName()
{	
	SOCKADDR_IN addr;
	int  len = sizeof(addr);
	if(m_sSocket != INVALID_SOCKET)
	{
		if(SOCKET_ERROR == getpeername(m_sSocket, (struct sockaddr *)&addr, &len))
			return 0;
		return (int)addr.sin_addr.s_addr;		
	}
	return 0;	
}

bool GTCPSocket::GetPeerName(int& nIp)
{
	SOCKADDR_IN addr;
	int  len = sizeof(addr);
	nIp = 0;
	if(m_sSocket != INVALID_SOCKET)
	{
		if(SOCKET_ERROR == getpeername(m_sSocket, (struct sockaddr *)&addr, &len))
			return false;
		nIp = addr.sin_addr.s_addr;		
		return true;
	}
	return false;	
}
int GTCPSocket::GetPeerName(BYTE& field0, BYTE& field1, BYTE& field2, BYTE& field3)
{
	int nIp;
	GetPeerName(nIp);

	BYTE* pIp = (BYTE *)&nIp;
	field0 = pIp[0];
	field1 = pIp[1];
	field2 = pIp[2];
	field3 = pIp[3];
	
	return nIp;
}
unsigned long __stdcall GTCPSocket::MainThread(void* param)
{
	unsigned long dwExitCode = ((GTCPSocket *)param)->Main();
	return dwExitCode;
}

unsigned long GTCPSocket::Main()
{
	unsigned long index;
	while(m_sSocket != INVALID_SOCKET)
	{
		index = WSAWaitForMultipleEvents(TOTAL_EVENT, m_event, false, WSA_INFINITE, false);
		if(index == NETWORK_EVENT)
		{
			WSAEnumNetworkEvents(m_sSocket, m_event[NETWORK_EVENT], &NetworkEvents);

			if(NetworkEvents.lNetworkEvents & FD_ACCEPT)
			{
				OnAccept(NetworkEvents.iErrorCode[FD_ACCEPT_BIT]);			
			}		
			else if(NetworkEvents.lNetworkEvents & FD_CONNECT)
			{
				OnConnect(NetworkEvents.iErrorCode[FD_CONNECT_BIT]);
			}
			else if(NetworkEvents.lNetworkEvents & FD_CLOSE)
			{
				OnClose(NetworkEvents.iErrorCode[FD_CLOSE_BIT]);
				if(m_sSocket == INVALID_SOCKET)
					goto L1;
			}
			else if(NetworkEvents.lNetworkEvents & FD_READ)
			{
				OnReceive(NetworkEvents.iErrorCode[FD_READ_BIT]);
			}		
			else if(NetworkEvents.lNetworkEvents & FD_WRITE)
			{
				OnSend(NetworkEvents.iErrorCode[FD_WRITE_BIT]);
			}
		}
		else
			break;
	}
L1: 
	return 0;
}

void GTCPSocket::OnAccept(int nErrorCode)
{
}

void GTCPSocket::OnClose(int nErrorCode)
{
}
void GTCPSocket::OnConnect(int nErrorCode)	
{
}

void GTCPSocket::OnReceive(int nErrorCode)
{
}
void GTCPSocket::OnSend(int nErrorCode)
{
}
