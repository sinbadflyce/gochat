/*
Filename	: CThemis.h
Purpose		: Using wrapper to use ThemisLib
Author		: Canhnh
Date time	: 14/09/2017
*/

#ifndef THEMISPP_SWAPPER_HPP_
#define THEMISPP_SWAPPER_HPP_

#pragma once
#include <secure_keygen.hpp>
#include <secure_cell.hpp>
#include <secure_message.hpp>
// Current Error [9/18/2017 Canhnh]
//#include <secure_session.hpp>  
#include <secure_rand.hpp>

#include "Includes/protobuf/wire.pb.h"
#include "Includes/protobuf/voip.pb.h"

#include <string.h>
#include "Includes/tcpsocket/TcpSocket.h"

using namespace std;

class CThemis
{
public:
	CThemis();
	~CThemis();

	// Varial of msg [9/20/2017 Canhnh]
	Login pLogin;

	// Create TcpSocket to implement connection with server [9/21/2017 Canhnh]
	int doConnnection();

	// Login [9/20/2017 Canhnh]
	void doLogin(string strUsername, string password);
	// Send message [9/20/2017 Canhnh]
	void doSendMsg();
	// Create Session [9/20/2017 Canhnh]
	void doCreateSession();

	// Generade pair key [9/20/2017 Canhnh]
	void createPairKey();
	void setTypeOfEnc(int type) { nType = type; };
	void setPort(string port) { strPort = port; };
	void setHost(string host) { strHost = host; };

	// Function for socket
	static void OnSocketReceive(CTcpSocket* pSocket, LPVOID pUser);
	static void OnSocketClose(CTcpSocket* pSocket, LPVOID pUser, BOOL bFlag);
	void ProcessSocketReceive();
	static UINT ProcessCommandThread(LPVOID lpUser);
	int ProcessCommand();

protected:
	std::vector<uint8_t> plKey;
	std::vector<uint8_t> prKey;
	int nType;
	string strHost;
	string strPort;
	bool b_isConnected;
	CTcpSocket			m_TcpSocket;
	LPBYTE				m_pBuffer;
	int					m_nBufferIdx;
};

#endif