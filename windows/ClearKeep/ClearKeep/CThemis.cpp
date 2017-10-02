#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define  GOOGLE_PROTOBUF_NO_STATIC_INITIALIZER

#pragma once
#include "stdafx.h"
//#include <cstdlib>
//#include <deque>
//#include <iostream>
#include "CThemis.h"
//#include "message.h"

#include <string.h>
using namespace std;

// logging
#include "wrapper/logging.h"


const uint8_t client_private_key[] = { 0x52, 0x45, 0x43, 0x32, 0x00, 0x00, 0x00, 0x2d, 0x00, 0xb2, 0x7f, 0x81, 0x00, 0x60, 0x9d, 0xe7, 0x7a, 0x39, 0x93, 0x68, 0xfc, 0x25, 0xd1, 0x79, 0x88, 0x6d, 0xfb, 0xf6, 0x19, 0x35, 0x53, 0x74, 0x10, 0xfc, 0x5b, 0x44, 0xe1, 0xf6, 0xf4, 0x4e, 0x59, 0x8d, 0x94, 0x99, 0x4f };
const uint8_t client_public_key[] = { 0x55, 0x45, 0x43, 0x32, 0x00, 0x00, 0x00, 0x2d, 0x10, 0xf4, 0x68, 0x8c, 0x02, 0x1c, 0xd0, 0x3b, 0x20, 0x84, 0xf2, 0x7a, 0x38, 0xbc, 0xf6, 0x39, 0x74, 0xbf, 0xc3, 0x13, 0xae, 0xb1, 0x00, 0x26, 0x78, 0x07, 0xe1, 0x7f, 0x63, 0xce, 0xe0, 0xb8, 0xac, 0x02, 0x10, 0x40, 0x10 };

const uint8_t server_private_key[] = { 0x52, 0x45, 0x43, 0x32, 0x00, 0x00, 0x00, 0x2d, 0xd0, 0xfd, 0x93, 0xc6, 0x00, 0xae, 0x83, 0xb3, 0xef, 0xef, 0x06, 0x2c, 0x9d, 0x76, 0x63, 0xf2, 0x50, 0xd8, 0xac, 0x32, 0x6e, 0x73, 0x96, 0x60, 0x53, 0x77, 0x51, 0xe4, 0x34, 0x26, 0x7c, 0xf2, 0x9f, 0xb6, 0x96, 0xeb, 0xd8 };
const uint8_t server_public_key[] = { 0x55, 0x45, 0x43, 0x32, 0x00, 0x00, 0x00, 0x2d, 0xa5, 0xb3, 0x9b, 0x9d, 0x03, 0xcd, 0x34, 0xc5, 0xc1, 0x95, 0x6a, 0xb2, 0x50, 0x43, 0xf1, 0x4f, 0xe5, 0x88, 0x3a, 0x0f, 0xb1, 0x11, 0x8c, 0x35, 0x81, 0x82, 0xe6, 0x9e, 0x5c, 0x5a, 0x3e, 0x14, 0x06, 0xc5, 0xb3, 0x7d, 0xdd };

CThemis* CThemis::SingleTon = NULL;

CThemis::CThemis()
{
	nType = 0; // default using EC as algorithm encrypt
	ws = NULL;
	b_initWebsocket = InitWebsocket();
	m_SocketThread = NULL;
	b_ExitThread = false;
	spdlog_set_level(spdlog_info);
	spdlog_set_pattern("[%Y-%m-%d %H:%M:%S.%e][%L][%t] %v");
	SPD_LOG_INFO("CKLog", "Init CThemis class");
}

CThemis::~CThemis()
{
	pLogin.Clear();
	if (b_initWebsocket)
		WSACleanup();
	if (ws)
	{
		ws->close();
		ws->~WebSocket();
	}

	if (m_SocketThread)
	{
		b_ExitThread = true;
		this_thread::sleep_for(tthread::chrono::milliseconds(100));
	}

}


CThemis* CThemis::GetSingleTon()
{
	if (SingleTon == NULL)
	{
		SingleTon = new CThemis();
	}

	return SingleTon;
}

LOGIN_STATUS CThemis::doConnnection()
{
	//Process with socket
	ws = WebSocket::from_url("ws://localhost:8000/ws"); // localhost
	//ws = WebSocket::from_url("ws://localhost:8080/ws"); // localhost - echo server
	//ws = WebSocket::from_url("ws://192.168.2.185:8000/ws"); // MAC on VMW

	if (ws == NULL)
	{
		return LOGIN_STATUS::login_error;
	}
	
	//thread t(ProcessCommandThread, 0);
	m_SocketThread = new thread(ProcessCommandThread, this);
	return LOGIN_STATUS::login_success;
}

void CThemis::doLogin(string strUsername, string password)
{
	// Create pair key
	createPairKey();
	Wire m_Wire;
	Login* m_pLogin = new Login();

	// processing with raw login message through protobuf [9/20/2017 Canhnh]
	m_pLogin->set_username(strUsername.c_str());
	m_pLogin->set_type(1);
	m_pLogin->set_platform("window");
	m_pLogin->set_authentoken("");
	m_pLogin->set_devicetoken("");

	pWire.set_allocated_login(m_pLogin);
	pWire.set_which(Wire_Which_LOGIN);

	string strRawData;
	pWire.SerializeToString(&strRawData);
	
	SendMsg(strRawData);

	// send or connect to server

}

void CThemis::createPairKey()
{
	if (!nType)
	{
		themispp::secure_key_pair_generator_t<themispp::EC> g;
		prKey = g.get_priv();
		plKey = g.get_pub();
	}
	else
	{
		themispp::secure_key_pair_generator_t<themispp::RSA> g;
		prKey = g.get_priv();
		plKey = g.get_pub();
	}
}

bool CThemis::InitWebsocket()
{
	INT rc;
	WSADATA wsaData;

	rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (rc) {
		printf("WSAStartup Failed.\n");
		return false;
	}
	return true;
}

bool CThemis::SendMsg(string strMsg)
{
	if (ws->getReadyState() != WebSocket::CLOSED)
	{
		ws->send(strMsg);
		ws->poll();
		return true;
	}
	return false;
}

void handle_message(const std::string & message)
{
	printf(">>> %s\n", message.c_str());
	if (message == "world") {  }
}

void CThemis::ProcessCommandThread(void* lpUser)
{
	CThemis* pThis = (CThemis*)lpUser;
	
	while (pThis->ws->getReadyState() != WebSocket::CLOSED)
	{
		pThis->ws->poll();
		pThis->ws->dispatch(ProcessCommand);
		this_thread::sleep_for(tthread::chrono::milliseconds(100));
	}
	return;
}

int CThemis::ProcessCommand(const std::string & message)
{
	CThemis* pThis = CThemis::GetSingleTon();
	Wire pResWire;
	Login pResLogin;

	int nLeng = message.size();
	pResWire.ParseFromString(message);
	
	Wire::Which m_ResWhich = pResWire.which();

	//if(m_ResWhich == Wire::CONTACTS)
	switch (m_ResWhich)
	{
	case Wire::LOGIN:
		break;
	case Wire::CONTACTS:
	{
		//Contact *pContact = new Contact();
		int nSize = pResWire.contacts_size();
		for (int n = 0; n < nSize; n++)
		{
			pThis->m_ListContact.push_back(pResWire.contacts(n));
		}
		pThis->b_isConnected = true;
	}
		break;
	case Wire::PRESENCE:
		break;
	case Wire::STORE:
		break;
	case Wire::LOAD:
		break;
	case Wire::PUBLIC_KEY:
		break;
	case Wire::PUBLIC_KEY_RESPONSE:
		break;
	case Wire::HANDSHAKE:
		break;
	case Wire::PAYLOAD:
		break;
	case Wire::LOGIN_RESPONSE:
		break;
	case Wire::PLAIN_TEXT:
		break;
	default:
		break;
	}


	return 0;
}

