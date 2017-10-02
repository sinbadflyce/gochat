/*
Filename	: CThemis.h
Purpose		: Using wrapper to use ThemisLib
Author		: Canhnh
Date time	: 14/09/2017
*/

#ifndef THEMISPP_SWAPPER_HPP_
#define THEMISPP_SWAPPER_HPP_

#pragma once
// websocket
#include "Includes/easyclient/easywsclient.hpp"
using easywsclient::WebSocket;
#ifdef _WIN32
#pragma comment( lib, "ws2_32" )
#include <WinSock2.h>
#endif

// themis
#include <secure_keygen.hpp>
#include <secure_cell.hpp>
#include <secure_message.hpp>
// Current Error [9/18/2017 Canhnh]
#include <secure_session.hpp>  
#include <secure_rand.hpp>

// protobuf
#include "Includes/protobuf/wire.pb.h"
#include "Includes/protobuf/voip.pb.h"

// string
#include <string.h>
using namespace std;

// tinythread
#include "Includes/tinythread/tinythread.h"
using namespace tthread;

enum AccountStatus
{
	status_busy,
	status_away,
	status_visible,
	status_Offline,
	status_Online,
};

enum LOGIN_STATUS {
	login_success,
	login_fail_username,
	login_fail_password,
	login_error, //connection error...
	login_unknow_error
};



class CThemis
{
public:
	CThemis();
	~CThemis();

	// Singleton
	static CThemis *SingleTon;
	static CThemis* GetSingleTon();

	// Varial of msg [9/20/2017 Canhnh]
	Login pLogin;
	Wire pWire;

	// Create TcpSocket to implement connection with server [9/21/2017 Canhnh]
	LOGIN_STATUS doConnnection();

	// Login [9/20/2017 Canhnh]
	void doLogin(string strUsername, string password);
	// Send message [9/20/2017 Canhnh]
	void doSendMsg(string strTo, string strContent);
	// Create Session [9/20/2017 Canhnh]
	void doCreateSession();

	// Generade pair key [9/20/2017 Canhnh]
	void createPairKey();
	void setTypeOfEnc(int type) { nType = type; };
	void setPort(int port) { nPort = port; };
	void setHost(int host) { nHost = host; };

	// Function for socket
	bool InitWebsocket();
	bool SendMsg(string strMsg);
	static void ProcessCommandThread(void* lpUser);
	static int ProcessCommand(const std::string & message);
	thread* m_SocketThread;

	// Data
	vector<Contact> m_ListContact;

protected:
	std::vector<uint8_t> plKey;
	std::vector<uint8_t> prKey;
	int nType;
	int nHost;
	int nPort;
	bool b_isConnected;
	bool b_initWebsocket;
	bool b_ExitThread;
	DWORD myThreadID;
	HANDLE myHandle;

	//websocket
	WebSocket::pointer ws;
};

#endif