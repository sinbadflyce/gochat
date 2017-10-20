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
#include <secure_session.hpp>  
#include <secure_rand.hpp>

// protobuf
#include "Includes/protobuf/wire.pb.h"
#include "Includes/protobuf/voip.pb.h"

// string
#include <string.h>
using namespace std;

// tinythread
//#include "Includes/tinythread/tinythread.h"
//using namespace tthread;

#include <functional>


enum AccountStatus
{
	status_Online,
	status_Offline,
	status_visible,
	status_away,
	status_busy
};

enum LOGIN_STATUS {
	login_success,
	login_fail_username,
	login_fail_password,
	login_error, //connection error...
	login_unknow_error
};

struct _List_Contact 
{
	_List_Contact() = default;
	_List_Contact(string name, string id, int status, string key)
		: strName(name), strId(id), strPrKey(key), nStatus(status), psession(NULL) {};
	string strName;
	string strId;
	int nStatus;
	string strPrKey;
	themispp::secure_session_t* psession;
};

struct _Messenger_Content
{
	_Messenger_Content() = default;
	_Messenger_Content(string content, string timestamp)
		: strContent(content), strTimestamp(timestamp){};
	string strContent;
	string strTimestamp;
};

struct _Contact_Chat_History
{
	string strName;
	_Messenger_Content _Content;
};


// Callback function pointer.
typedef void(*CallbackFunctionPtr)(LPVOID, int);


class CThemis
{
public:
	CThemis();
	~CThemis();

	// Singleton
	static CThemis *SingleTon;
	static CThemis* GetSingleTon();

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
	// default using ECC - type = 0
	void setTypeOfEnc(int type) { nType = type; };
	void setPort(string port) { strPort = port; };
	void setHost(string host) { strHost = host; };

	// Function for socket
	bool InitWebsocket();
	bool SendMsg(string strMsg);
	static void ProcessCommandThread(void* lpUser);
	static int ProcessCommand(const std::string & message);
	const string currentDateTime();

	// Data
	vector<_List_Contact> m_ListContact;

	// Create chatting session
	void doProcessingPublickey(string strFrom, string plKey);
	int doCreateChatSession(int nContactIndex);
	int doSendPublicKeyToClient(int nContactIndex);
	int doSendPublicKeyResponseToClient(string strTo);
	int doSendMsgToClient(int nContactIndex, string strText);
	int doProcessSendMsg(int nContactIndex, string strText);
	int doReceiveHandShake(string strFrom, string strPayload);
	int doSendHandShake(string strFrom, string session_init_data);
	int doReceivePayload(string strFrom, string strPayload);

	// Send list contact to server
	int doSendContactList();
	int doAddContactToList(Contact pContact);
	int getIndexByName(string strName);
	string getContactNameByIndex(int nIndex);
	_List_Contact* getContactByIndex(int nIndex);
	_Messenger_Content doGetChatContentOfContact(string strContact);

	// Connect Callback Function to handle message incoming
	void doSetCallbackFunction(LPVOID pClassHolder, CallbackFunctionPtr cb_Message) { m_cb_Message = cb_Message; m_pHolder = pClassHolder; };
	
	// store session id
	void setSessionId(string strId) {
		strSessionId = strId;
	};
	const string getSessionId() { return strSessionId; };

	// Create Session_Secure to use
	int doCreateSecureSession(string strPeerID, string strPeerPlkey);

	// create to test
	vector<uint8_t> getPublicKey() { return plKey; };
	vector<uint8_t> getPrivateKey() { return prKey; };
	const string getAccount() { return strAccount; };
	void doCheck(bool bValue);
	
protected:
	std::vector<uint8_t> plKey;
	std::vector<uint8_t> prKey;
	int nType;
	string strHost;
	string strPort;
	bool b_isConnected;
	bool b_initWebsocket;
	bool b_ExitThread;
	DWORD myThreadID;
	HANDLE myHandle;

	// websocket
	WebSocket::pointer ws;
	string strSessionId;
	string strAccount;

	// For callback function
	void *m_pHolder;
	CallbackFunctionPtr m_cb_Message;
	vector<_Contact_Chat_History> m_ChatHistory;
};

#endif