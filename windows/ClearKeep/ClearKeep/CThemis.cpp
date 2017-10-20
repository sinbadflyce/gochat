#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define  GOOGLE_PROTOBUF_NO_STATIC_INITIALIZER

#pragma once
#include "stdafx.h"
//#include <cstdlib>
//#include <deque>
//#include <iostream>
#include <thread>
#include "CThemis.h"
//#include "message.h"

#include <string.h>
using namespace std;

// logging
#include "wrapper/logging.h"


const uint8_t client_priv[] = { 0x52, 0x45, 0x43, 0x32, 0x00, 0x00, 0x00, 0x2d, 0x00, 0xb2, 0x7f, 0x81, 0x00, 0x60, 0x9d, 0xe7, 0x7a, 0x39, 0x93, 0x68, 0xfc, 0x25, 0xd1, 0x79, 0x88, 0x6d, 0xfb, 0xf6, 0x19, 0x35, 0x53, 0x74, 0x10, 0xfc, 0x5b, 0x44, 0xe1, 0xf6, 0xf4, 0x4e, 0x59, 0x8d, 0x94, 0x99, 0x4f };
const uint8_t client_pub[] = { 0x55, 0x45, 0x43, 0x32, 0x00, 0x00, 0x00, 0x2d, 0x10, 0xf4, 0x68, 0x8c, 0x02, 0x1c, 0xd0, 0x3b, 0x20, 0x84, 0xf2, 0x7a, 0x38, 0xbc, 0xf6, 0x39, 0x74, 0xbf, 0xc3, 0x13, 0xae, 0xb1, 0x00, 0x26, 0x78, 0x07, 0xe1, 0x7f, 0x63, 0xce, 0xe0, 0xb8, 0xac, 0x02, 0x10, 0x40, 0x10 };
const uint8_t server_priv[] = { 0x52, 0x45, 0x43, 0x32, 0x00, 0x00, 0x00, 0x2d, 0xd0, 0xfd, 0x93, 0xc6, 0x00, 0xae, 0x83, 0xb3, 0xef, 0xef, 0x06, 0x2c, 0x9d, 0x76, 0x63, 0xf2, 0x50, 0xd8, 0xac, 0x32, 0x6e, 0x73, 0x96, 0x60, 0x53, 0x77, 0x51, 0xe4, 0x34, 0x26, 0x7c, 0xf2, 0x9f, 0xb6, 0x96, 0xeb, 0xd8 };
const uint8_t server_pub[] = { 0x55, 0x45, 0x43, 0x32, 0x00, 0x00, 0x00, 0x2d, 0xa5, 0xb3, 0x9b, 0x9d, 0x03, 0xcd, 0x34, 0xc5, 0xc1, 0x95, 0x6a, 0xb2, 0x50, 0x43, 0xf1, 0x4f, 0xe5, 0x88, 0x3a, 0x0f, 0xb1, 0x11, 0x8c, 0x35, 0x81, 0x82, 0xe6, 0x9e, 0x5c, 0x5a, 0x3e, 0x14, 0x06, 0xc5, 0xb3, 0x7d, 0xdd };

vector<uint8_t> prKey_test;
vector<uint8_t> plKey_test;




static class callback : public themispp::secure_session_callback_interface_t {
public:
	const std::vector<uint8_t> get_pub_key_by_id(const std::vector<uint8_t>& id) {
		CThemis* pThis = CThemis::GetSingleTon();
		std::string id_str(id.begin(), id.end());
			// get public key of peer 
		if (id_str == "server")
		{
			return prKey_test;
		}

		if (id_str == pThis->getAccount())
		{
			// get public key of account
			return pThis->getPublicKey();
		}

		for (int n = 0; n < pThis->m_ListContact.size(); n++)
		{
			if (id_str == pThis->m_ListContact.at(n).strName)
			{
				if (pThis->m_ListContact.at(n).strPrKey.size() > 0)
				{
					vector<uint8_t> peer_public_key(pThis->m_ListContact.at(n).strPrKey.begin(), pThis->m_ListContact.at(n).strPrKey.end());
					return peer_public_key;
				}
			}
		}
		return std::vector<uint8_t>(0);
	}
};
CThemis* CThemis::SingleTon = NULL;

CThemis::CThemis()
{
	nType = 0; // default using EC as algorithm encrypt
	ws = NULL;
	b_initWebsocket = InitWebsocket();
	b_ExitThread = false;
	spdlog_set_level(spdlog_info);
	spdlog_set_pattern("[%Y-%m-%d %H:%M:%S.%e][%L][%t] %v");
	SPD_LOG_INFO("CKLog", "Init CThemis class");
}

CThemis::~CThemis()
{
	if (b_initWebsocket)
		WSACleanup();
	if (ws)
	{
		ws->close();
		ws->~WebSocket();
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
	string host("ws://" + strHost + ":" + strPort + "/ws");
	//"ws://127.0.0.1:8000/ws"
	ws = WebSocket::from_url(host); 
	if (ws == NULL)
	{
		return LOGIN_STATUS::login_error;
	}
	
	//thread t(ProcessCommandThread, 0);
	std::thread ThreadConsumer(ProcessCommandThread, this);
	ThreadConsumer.detach();

	return LOGIN_STATUS::login_success;
}

void CThemis::doLogin(string strUsername, string password)
{
	// Create pair key
	createPairKey();

	// store account
	strAccount = strUsername;

	// package the message
	Wire m_Wire;
	Login *m_pLogin = new Login;

	// processing with raw login message through protobuf [9/20/2017 Canhnh]
	m_pLogin->set_username(strUsername.c_str());
	m_pLogin->set_type(1);
	m_pLogin->set_platform("window");
	m_pLogin->set_authentoken("");
	m_pLogin->set_devicetoken("");

	m_Wire.set_allocated_login(m_pLogin);
	m_Wire.set_which(Wire_Which_LOGIN);

	string strRawData;
	m_Wire.SerializeToString(&strRawData);
	
	SendMsg(strRawData);
}

void CThemis::createPairKey()
{
	if (!nType)
	{
		themispp::secure_key_pair_generator_t<themispp::EC> g;
		//g.gen();
		prKey = g.get_priv();
		plKey = g.get_pub();
	}
	else
	{
		themispp::secure_key_pair_generator_t<themispp::RSA> g;
		//g.gen();
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


void CThemis::ProcessCommandThread(void* lpUser)
{
	CThemis* pThis = (CThemis*)lpUser;
	
	while (!pThis->b_ExitThread)
	{
		if (pThis->ws->getReadyState() == WebSocket::CLOSED)
			return;

		pThis->ws->poll();
		pThis->ws->dispatch(ProcessCommand);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	return;
}

int CThemis::ProcessCommand(const string & message)
{
	CThemis* pThis = CThemis::GetSingleTon();

	// revert Wire message
	Wire* pResWire = new Wire;
	pResWire->ParseFromString(message);

	// store session ID
	if(pResWire->sessionid().size() > 0)
		pThis->setSessionId(pResWire->sessionid());
	
	Wire::Which m_ResWhich = pResWire->which();
	switch (m_ResWhich)
	{
	case Wire::LOGIN:
		
		break;
	case Wire::CONTACTS:
	case Wire::PRESENCE:
	{
		int nSize = pResWire->contacts_size();
		for (int n = 0; n < nSize; n++)
		{
			pThis->doAddContactToList(pResWire->contacts(n));
		}
	}
		break;
	case Wire::STORE:

		break;
	case Wire::LOAD:
		break;
	case Wire::PUBLIC_KEY:
	{
		// 1. store public key into contact list
		pThis->doProcessingPublickey(pResWire->from(), pResWire->payload());
		
		// 2. (default accept) Send PUBLIC_KEY_RESPONSE  
		pThis->doSendPublicKeyResponseToClient(pResWire->from());
	}
		break;
	case Wire::PUBLIC_KEY_RESPONSE:
	{
		// Create new session then get data = session.init() and send to HANSHAKE
		pThis->doCreateSecureSession(pResWire->from(), pResWire->payload());
	}
		break;
	case Wire::HANDSHAKE:
	{
		// 1. Get data of Handshake then do processing 
		pThis->doReceiveHandShake(pResWire->from(), pResWire->payload());
	}
		break;
	case Wire::PAYLOAD:
	{
		pThis->doReceivePayload(pResWire->from(), pResWire->payload());
		break;
	}
	case Wire::LOGIN_RESPONSE:
		break;
	case Wire::PLAIN_TEXT:
		break;
	default:
		break;
	}

	pThis->m_cb_Message(pThis->m_pHolder, m_ResWhich);

	return 0;
}

const string CThemis::currentDateTime()
{
	// Get current date/time, format is YYYY-MM-DD.HH:mm:ss
	time_t rawtime;
	struct tm * timeinfo;
	char buffer[80];

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(buffer, sizeof(buffer), "%d-%m-%Y %I:%M:%S", timeinfo);
	std::string str(buffer);

	return str;
}

int CThemis::doCreateChatSession(int nContactIndex)
{

	return 0;
}

int CThemis::doSendPublicKeyToClient(int nContactIndex)
{
	// Send public key to index of contact to chat
	Wire* pWire = new Wire();
	
	pWire->set_which(Wire_Which_PUBLIC_KEY);
	string strPlKey(plKey.begin(), plKey.end());
	//pWire->set_payload(strPlKey.c_str());
	pWire->set_allocated_payload(&strPlKey);
	pWire->set_to(m_ListContact.at(nContactIndex).strName);
	pWire->set_sessionid(strSessionId);
	pWire->set_from(strAccount);

	string strPayload = pWire->payload();
	string strRawData;
	pWire->SerializeToString(&strRawData);

	SendMsg(strRawData);

	pWire->Clear();

	return 0;
}

int CThemis::doSendPublicKeyResponseToClient(string strTo)
{
	// Send public key to index of contact to chat
	Wire* pWire = new Wire();

	pWire->set_which(Wire_Which_PUBLIC_KEY_RESPONSE);
	string strPlKey(plKey.begin(), plKey.end());
	pWire->set_allocated_payload(&strPlKey);
	pWire->set_to(strTo);
	pWire->set_sessionid(strSessionId);
	pWire->set_from(strAccount);

	string strRawData;
	pWire->SerializeToString(&strRawData);

	SendMsg(strRawData);

	pWire->Clear();

	return 0;
}

int CThemis::doSendMsgToClient(int nContactIndex, string strText)
{
	// Send plain text to index of contact to chat
	Wire* pWire = new Wire();

	pWire->set_which(Wire_Which_PAYLOAD);
	pWire->set_allocated_payload(&strText);
	pWire->set_to(m_ListContact.at(nContactIndex).strName);
	pWire->set_sessionid(strSessionId);
	pWire->set_from(strAccount);

	string strRawData;
	pWire->SerializeToString(&strRawData);

	SendMsg(strRawData);

	pWire->Clear();
	return 0;
}

int CThemis::doProcessSendMsg(int nContactIndex, string strText)
{
	if (!getContactByIndex(nContactIndex)->psession)
	{
		doSendPublicKeyToClient(nContactIndex);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	

	if (getContactByIndex(nContactIndex)->psession)
	{
		if (getContactByIndex(nContactIndex)->psession->is_established())
		{
			doSendMsgToClient(nContactIndex, strText);
		}
	}
	else
	{
		return 1;
	}

	return 0;
}

int CThemis::doReceiveHandShake(string strFrom, string strPayload)
{
	// when receive HandShake will send back unwrap data if session has not established
	
	// 1. Check session of From: exist or not
	size_t nContactSize = m_ListContact.size();
	for (int n = 0; n < nContactSize; n++)
	{
		if (strFrom == m_ListContact.at(n).strName)
		{
			if (m_ListContact.at(n).psession == NULL) // 1.1 Create new secure_session now
			{
				doCreateSecureSession(strFrom, strPayload); // -unwrap this Init_data
			}
			else // 1.2 Check if session has created
			{
				if (!m_ListContact.at(n).psession->is_established())
				{
					vector<uint8_t> data(strPayload.begin(), strPayload.end());
					vector<uint8_t> data_unwrap = m_ListContact.at(n).psession->unwrap(data);
					string strDataUnwrap(data_unwrap.begin(), data_unwrap.end());
					doSendHandShake(strFrom, strDataUnwrap);
				}
				else
				{
					bool bChatNow = true;
				}
			}
		}
	}
	return 0;
}

int CThemis::doSendHandShake(string strFrom, string session_init_data)
{
	Wire* pWire = new Wire;
	pWire->set_which(Wire_Which_HANDSHAKE);
	pWire->set_allocated_payload(&session_init_data);
	pWire->set_sessionid(strSessionId);
	pWire->set_from(strAccount);
	pWire->set_to(strFrom);
	
	string strRawData;
	pWire->SerializeToString(&strRawData);

	SendMsg(strRawData);

	pWire->Clear();

	return 0;
}

int CThemis::doReceivePayload(string strFrom, string strPayload)
{
	// do store contact chat to database 
	/*
	for (int n =0; n < m_ChatHistory.size(); n++)
	{
		if (strFrom == m_ChatHistory.at(n).strName)
		{
			// ADD not REPLACE
			_Messenger_Content _content;
			_content.strContent = strPayload;
			_content.strTimestamp = currentDateTime();
			m_ChatHistory.at(n)._Content = _content;
		}
	}
	*/

	for (int n = 0; n < m_ChatHistory.size(); n++)
	{
		if (strFrom == m_ChatHistory.at(n).strName)
		{
			// ADD not REPLACE
			_Messenger_Content _content;
			_content.strContent = strPayload;
			_content.strTimestamp = currentDateTime();
			m_ChatHistory.at(n)._Content = _content;
			return 0;
		}
	}


	return 0;
}

int CThemis::doSendContactList()
{
	Wire* pWire = new Wire;

	pWire->set_which(Wire_Which_CONTACTS);
	pWire->set_sessionid(strSessionId);
	pWire->set_from(strAccount);

	size_t nContactSize = m_ListContact.size();
	if (nContactSize <= 0)
		return -1;

	for (int n = 0; n < nContactSize; n++)
	{
		Contact* pTempContact = pWire->add_contacts();
		pTempContact->set_name(m_ListContact.at(n).strName);
		pTempContact->set_id(m_ListContact.at(n).strId);
		pTempContact->set_online(false);
	}
	
	string strRawData;
	pWire->SerializeToString(&strRawData);

	SendMsg(strRawData);

	pWire->Clear();

	return 0;
}

int CThemis::doAddContactToList(Contact pContact)
{
	for (int n=0; n < m_ListContact.size(); n++)
	{
		if (pContact.name() == m_ListContact.at(n).strName)
		{ 
			// Set status with new
			if (pContact.online() != m_ListContact.at(n).nStatus)
			{
				m_ListContact.at(n).nStatus = pContact.online();
				m_ListContact.at(n).strPrKey = "";
				if (!m_ListContact.at(n).nStatus && m_ListContact.at(n).psession)
				{
					delete m_ListContact.at(n).psession; // - release secure_session if created
					m_ListContact.at(n).psession = NULL;
				}
			}
			return 1;
		}
	}
	_List_Contact pNewContact;
	pNewContact.nStatus = pContact.online();
	pNewContact.strName = pContact.name();
	pNewContact.strId = pContact.id();
	pNewContact.nStatus = pContact.online();
	pNewContact.psession = NULL;
	m_ListContact.push_back(pNewContact);

	_Contact_Chat_History pNewChat;
	pNewChat.strName = pContact.name();
	_Messenger_Content pContent("", "");
	pNewChat._Content = pContent;
	m_ChatHistory.push_back(pNewChat);

	return 0;
}

int CThemis::getIndexByName(string strName)
{
	int nIndex;
	for (nIndex = 0; nIndex < m_ListContact.size(); nIndex++)
	{
		if (strName == m_ListContact.at(nIndex).strName)
			return nIndex;
	}
	return -1;
}

std::string CThemis::getContactNameByIndex(int nIndex)
{
	if (nIndex >= m_ListContact.size())
	{
		return "";
	}

	return m_ListContact.at(nIndex).strName;
}

_List_Contact* CThemis::getContactByIndex(int nIndex)
{
	_List_Contact* pTemp = NULL;
	if (m_ListContact.size() >= 0)
	{
		pTemp = &m_ListContact.at(nIndex);
	}
	return pTemp;
}

_Messenger_Content CThemis::doGetChatContentOfContact(string strContact)
{
	_Messenger_Content content_temp("", "");
	for (int n=0; n < m_ChatHistory.size(); n++)
	{
		if (strContact == m_ChatHistory.at(n).strName)
		{
			content_temp = m_ChatHistory.at(n)._Content;
		}
	}
	return content_temp;
}

void CThemis::doProcessingPublickey(string strFrom, string plKey_peer)
{
	// 1. do need check session id here???

	// 2. Check and set plkey to tokenDevice fields in ContactList
	for (int n = 0; n < m_ListContact.size(); n++)
	{
		if (strFrom == m_ListContact.at(n).strName)
			m_ListContact.at(n).strPrKey = plKey_peer;
	}
}


class callback_test : public themispp::secure_session_callback_interface_t {
public:
	const vector<uint8_t> get_pub_key_by_id(const vector<uint8_t>& id)
	{
		//return std::vector<uint8_t>(server_pub, server_pub + sizeof(server_pub));
		CThemis* pThis = CThemis::GetSingleTon();
		return pThis->getPublicKey();
	}
};

int CThemis::doCreateSecureSession(string strPeerID, string strPeerPlkey)
{
	// NOTE [10/17/2017 Canhnh]
	// - split that function later 
	if (m_ListContact.at(getIndexByName(strPeerID)).strPrKey.length() > 0)
	{
		/*
		// Test [10/18/2017 Canhnh]
		callback callbacks_outgoing;
		themispp::secure_session_t* session_t = new themispp::secure_session_t(
			std::vector<uint8_t>(strAccount.c_str(), strAccount.c_str() + strAccount.length()),
			getPrivateKey(),
			&callbacks_outgoing);
		vector<uint8_t> data = session_t->init();
		//------------------------------------------------
		*/

		// 2. Create new secure_session for incoming invite
		// - unwrap init_data received then send back to peer
		callback* callbacks_incoming = new callback;
		themispp::secure_session_t* session = new themispp::secure_session_t(
			std::vector<uint8_t>(strAccount.c_str(), strAccount.c_str() + strAccount.length()),
			getPrivateKey(),
			callbacks_incoming);

		vector<uint8_t> data(strPeerPlkey.begin(), strPeerPlkey.end());
		vector<uint8_t> wrap_data = session->unwrap(data);
		string strData(wrap_data.begin(), wrap_data.end());

		// 3. Send first HandShake <data_init_session> to peer
		doSendHandShake(strPeerID, strData);

		// 4. Store session of peer to list
		for (int n = 0; n < m_ListContact.size(); n++)
		{
			if (strPeerID == m_ListContact.at(n).strName)
			{
				m_ListContact.at(n).psession = session;
				return 0;
			}
		}
	}
	else
	{
		m_ListContact.at(getIndexByName(strPeerID)).strPrKey = strPeerPlkey;
		// 2. Create new secure_session for outgoing 
		// create init_data then send to peer
		callback* callbacks_outgoing = new callback;
		themispp::secure_session_t* session = new themispp::secure_session_t(
			std::vector<uint8_t>(strAccount.c_str(), strAccount.c_str() + strAccount.length()),
			getPrivateKey(),
			callbacks_outgoing);
		vector<uint8_t> data = session->init();
	
		/*
		// Test [10/18/2017 Canhnh]
		themispp::secure_key_pair_generator_t<themispp::EC> t;
		prKey_test = t.get_priv();
		plKey_test = t.get_pub();
		callback_test* callbacks_t = new callback_test; // to get plkey of account
		string server_id("server");
		themispp::secure_session_t* sv_session = new themispp::secure_session_t(
			std::vector<uint8_t>(server_id.c_str(), server_id.c_str() + server_id.length()),
			prKey_test,
			//std::vector<uint8_t>(server_priv, server_priv + sizeof(server_priv)),
			callbacks_t);
		vector<uint8_t> t_unwrap_data = sv_session->unwrap(data); // need plkey of canh

		vector<uint8_t> unwrap_data_1 =  session->unwrap(t_unwrap_data);

		vector<uint8_t> unwrap_data_2 = sv_session->unwrap(unwrap_data_1);

		vector<uint8_t> unwrap_data_3 = session->unwrap(unwrap_data_2);
		//------------------------------------------------
			*/	
		
		// 3. Send first HandShake <data_init_session> to peer
		string strData(data.begin(), data.end());
		doSendHandShake(strPeerID, strData);

		// 4. Store session of peer to list
		for (int n = 0; n < m_ListContact.size(); n++)
		{
			if (strPeerID == m_ListContact.at(n).strName)
			{
				m_ListContact.at(n).psession = session;
				return 0;
			}
		}
	}

	return 0;
}

void CThemis::doCheck(bool bValue)
{
	if (bValue)
	{
		bool nSuccess = bValue;
	}
}

