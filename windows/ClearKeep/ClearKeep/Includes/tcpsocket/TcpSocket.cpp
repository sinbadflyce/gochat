#include "TcpSocket.h"

CTcpSocket::CTcpSocket()
{
	m_pOnReceive=NULL;
	m_pOnAccept=NULL;
	m_pOnClose=NULL;
}

CTcpSocket::~CTcpSocket()
{
}

void CTcpSocket::OnAccept(int nErrorCode) 
{
	if(m_pOnAccept)
		m_pOnAccept(this,m_UserData);
	else
		GTCPSocket::OnAccept(nErrorCode);
}

void CTcpSocket::OnClose(int nErrorCode) 
{
	if(m_pOnClose)
		m_pOnClose(this,m_UserData,FALSE);
	else
		GTCPSocket::OnClose(nErrorCode);
}

void CTcpSocket::OnReceive(int nErrorCode) 
{
	if(m_pOnReceive)
		m_pOnReceive(this,m_UserData);
	else
		GTCPSocket::OnReceive(nErrorCode);
}

int CTcpSocket::Listen(int port)
{
	BOOL bOptVal = TRUE;
	int bOptLen = sizeof(BOOL);
	if (setsockopt(m_sSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&bOptVal, bOptLen) != SOCKET_ERROR) 
	{
		OutputDebugString(LPCTSTR("Set SO_KEEPALIVE: ON\n"));
	}

	if(!GTCPSocket::Listen(port))
	{
		int err=GetLastError();
		Close();
		return err;
	}

	return 0;
}

int CTcpSocket::Connect(int ip, int port)
{
	BOOL bOptVal = TRUE;
	int bOptLen = sizeof(BOOL);
	if (setsockopt(m_sSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&bOptVal, bOptLen) != SOCKET_ERROR) 
	{
		OutputDebugString(LPCTSTR("Set SO_KEEPALIVE: ON\n"));
	}
		
	if(!GTCPSocket::Connect(ip, port))
	{
		int err=GetLastError();
		Close();
		//return err;
		return -1; //if connect fail
	}

	return 0;
}

void CTcpSocket::SetCallbackFunc(void (*OnAccept)(CTcpSocket*,LPVOID),void (*OnReceive)(CTcpSocket*,LPVOID),void (*OnClose)(CTcpSocket*,LPVOID,BOOL),LPVOID UserData)
{
	m_pOnAccept=OnAccept;
	m_pOnReceive=OnReceive;
	m_pOnClose=OnClose;

	m_UserData=UserData;
}