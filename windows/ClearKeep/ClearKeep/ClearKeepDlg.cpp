
// ClearKeepDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ClearKeep.h"
#include "ClearKeepDlg.h"
#include "afxdialogex.h"

#include "wrapper/logging.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CAboutDlg dialog used for App About
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CClearKeepDlg dialog



CClearKeepDlg::CClearKeepDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_CLEARKEEP_DIALOG, pParent)
	, m_strInputMsg(_T(""))
	, m_ChatZoom(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	
}

void CClearKeepDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_CONTACT, m_ContactListCtrl);
	DDX_Text(pDX, IDC_ED_INPUT, m_strInputMsg);
	DDX_Text(pDX, IDC_CHAT_ROOM, m_ChatZoom);
}

BEGIN_MESSAGE_MAP(CClearKeepDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BT_SEND, &CClearKeepDlg::OnBnClickedBtSend)
	ON_BN_CLICKED(IDC_BT_ADD_CT, &CClearKeepDlg::OnBnClickedBtAddCt)
	ON_NOTIFY(NM_CLICK, IDC_LIST_CONTACT, &CClearKeepDlg::OnNMClickListContact)
END_MESSAGE_MAP()


// CClearKeepDlg message handlers

int CClearKeepDlg::InitLogging()
{
	// init log machine for logging information [9/19/2017 Canhnh]
	spdlog_set_level(spdlog_info);
	spdlog_set_pattern("[%Y-%m-%d %H:%M:%S.%e][%L][%t] %v");
	spdlog_basic_logger_mt("CKLog", "log.txt", 0);
	SPD_LOG_INFO("CKLog", "Init application");
	return 0;
}

int CClearKeepDlg::Init()
{
	// Init logging to trace all information
	InitLogging();

	InitTabContact();
	nCurrentIndex = -1; // None peer is displayed

	// Init Themis class
	m_Themis = CThemis::GetSingleTon();
	m_LoginDlg = new CLoginDlg();

	//Setting callback function
	m_Themis->doSetCallbackFunction(this, incommingMessage);
	
	// if login return fail, app will exit or request register
	if (!CallLogin())
	{
		m_bConnected = false;
		EndDialog(0);
		// bat cai flash nao do len chay trong timeout 5s de thuc hien checklog in [9/29/2017 Canhnh]

	}
	else
	{
		m_bIsLogin = true;
	}

	ShowWindow(SW_SHOW);

	return 0;
}

bool CClearKeepDlg::CallLogin()
{
	SPD_LOG_INFO("CKLog", "Request login");
	
	if (m_LoginDlg->DoModal() == IDCANCEL)
	{
		return false;
	}
	
	return true;
}

LOGIN_STATUS CClearKeepDlg::doLogin(const string strName, const string strPassword)
{
	// Use Themis lib to send and check login information with server
	LOGIN_STATUS nResult = m_Themis->doConnnection();
	if (nResult != login_success)
	{
		return nResult;
	}
	
	m_Themis->doLogin(strName, strPassword);

	// - luu gia tri cua Contact Login
	strUsername = strName;
	strPass = strPassword;

	AfxBeginThread(doCheckLogin, this);
	return nResult;
}

BOOL CClearKeepDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	ShowWindow(SW_HIDE);

	// TODO: Add extra initialization here
	Init();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CClearKeepDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CClearKeepDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CClearKeepDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

UINT CClearKeepDlg::doCheckLogin(void * pClass)
{
	CClearKeepDlg* pThis = (CClearKeepDlg*)pClass;
	int n = 0;
	while (n < 1000)
	{
		if (pThis->m_bIsLogin)
		{
			pThis->doLoginProcess();
			return 0;
		}
		Sleep(100);
		n++;
	}
	return 0;
}

void CClearKeepDlg::doLoginProcess()
{
	// Main Account loading
	CString strName(strUsername.c_str());
	SetDlgItemText(IDC_MAIN_USERNAME, strName);
	CComboBox* pStatus = (CComboBox*)GetDlgItem(IDC_MAIN_STATUS);
	pStatus->SetCurSel(AccountStatus::status_Online);
	// set avatar nua

	// Hide button and peer avatar, status
	doShowHideChatBox(SW_HIDE); //will show when contact peer is selected
}

void CClearKeepDlg::InitTabContact()
{
	LVCOLUMN lvColumn;

	lvColumn.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
	lvColumn.fmt = LVCFMT_LEFT;
	lvColumn.cx = 100;
	lvColumn.pszText = _T("Contact Name");
	m_ContactListCtrl.InsertColumn(0, &lvColumn);

	lvColumn.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
	lvColumn.fmt = LVCFMT_CENTER;
	lvColumn.cx = 70;
	lvColumn.pszText = _T("Status");
	m_ContactListCtrl.InsertColumn(1, &lvColumn);
}

void CClearKeepDlg::doShowHideChatBox(int nOpt)
{
	GetDlgItem(IDC_BT_ACALL)->ShowWindow(nOpt);
	GetDlgItem(IDC_BT_VCALL)->ShowWindow(nOpt);
	GetDlgItem(IDC_PEER_AVATAR)->ShowWindow(nOpt);
	GetDlgItem(IDC_PEER_NAME)->ShowWindow(nOpt);
	GetDlgItem(IDC_PEER_STATUS)->ShowWindow(nOpt);
	GetDlgItem(IDC_BT_SEND)->ShowWindow(nOpt);
}

void CClearKeepDlg::LoadPeerInfo(int nIndex)
{
	if (nIndex >= m_Themis->m_ListContact.size()) // Error if index greater than size of list contact
	{
		return;
	}
	CString strPeerName(m_Themis->m_ListContact.at(nIndex).strName.c_str());
	SetDlgItemText(IDC_PEER_NAME, strPeerName);
	bool bIsOnline = m_Themis->m_ListContact.at(nIndex).nStatus;
	CComboBox* pStatus = (CComboBox*)GetDlgItem(IDC_PEER_STATUS);
	if (bIsOnline)
		pStatus->SetCurSel(AccountStatus::status_Online);
	else
		pStatus->SetCurSel(AccountStatus::status_Offline);

	doShowHideChatBox(SW_SHOW);
	// Set avatar nua
}

void CClearKeepDlg::doLoadContactToList()
{
	m_ContactListCtrl.DeleteAllItems();

	if (m_Themis->m_ListContact.size() > 0)
	{
		for (unsigned i = 0; i < m_Themis->m_ListContact.size(); i++)
		{
			CString strAccName(m_Themis->m_ListContact.at(i).strName.c_str());
			m_ContactListCtrl.InsertItem(i, strAccName);
			m_ContactListCtrl.SetItemText(i, 0, strAccName);
			CString strStatus = _T("status");
			if (m_Themis->m_ListContact.at(i).nStatus)
			{
				strStatus = _T("online");
			}
			else
			{
				strStatus = _T("offline");
			}
			m_ContactListCtrl.SetItemText(i, 1, strStatus);
		}
	}
}

void CClearKeepDlg::incommingMessage(LPVOID p, int nMsgType)
{
	CClearKeepDlg* pThis = (CClearKeepDlg*)p;
	Wire::Which nWhich = (Wire_Which)nMsgType;
	switch (nWhich)
	{
		case Wire::LOGIN:
			break;
		case Wire::CONTACTS:
		{
			pThis->doLoadContactToList();
		}
		break;
		case Wire::PRESENCE:
		{
			pThis->doLoadContactToList(); 
		}
		break;
		case Wire::STORE:
			AfxMessageBox(_T("STORE"), IDOK);
			break;
		case Wire::LOAD:
			AfxMessageBox(_T("LOAD"), IDOK);
			break;
		case Wire::PUBLIC_KEY:
			//AfxMessageBox(_T("PUBLIC_KEY"), IDOK);
			break;
		case Wire::PUBLIC_KEY_RESPONSE:
			//AfxMessageBox(_T("PUBLIC_KEY_RESPONSE"), IDOK);
			break;
		case Wire::HANDSHAKE:
			//AfxMessageBox(_T("HANDSHAKE"), IDOK);
			break;
		case Wire::PAYLOAD:
		{
			//AfxMessageBox(_T("PAYLOAD"), IDOK);
			// Load chat history from database store
			pThis->doLoadChatHistory();
			break;
		}
		case Wire::LOGIN_RESPONSE:
			AfxMessageBox(_T("LOGIN_RESPONSE"), IDOK);
			break;
		case Wire::PLAIN_TEXT:
			AfxMessageBox(_T("PLAIN_TEXT"), IDOK);
			break;
		default:
			break;
	}
}


int CClearKeepDlg::doLoadChatHistory()
{
	// load chat history of current index
	_Messenger_Content _CurrentContent;
	_CurrentContent = m_Themis->doGetChatContentOfContact((m_Themis->getContactNameByIndex(nCurrentIndex)));
	CString strFrom(m_Themis->getContactNameByIndex(nCurrentIndex).c_str());
	CString strNewline(_CurrentContent.strContent.c_str());
	CString strTime(_CurrentContent.strTimestamp.c_str());
	m_ChatZoom += _T("\r\n") + strFrom + _T(" < ") + strTime + _T(" >: ") + strNewline;
	
	SetDlgItemText(IDC_CHAT_ROOM, m_ChatZoom);

	return 0;
}

void CClearKeepDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	if (m_Themis)
		delete m_Themis;
	if (m_LoginDlg)
	{
		delete m_LoginDlg;
	}

	CDialogEx::OnClose();
}


void CClearKeepDlg::OnBnClickedBtSend()
{
	// check if offline
	if (m_Themis->getContactByIndex(nCurrentIndex)->nStatus != 1)
	{
		return;
	}

	
	// Make new session chat or send text message
	
	CT2CA pszMsg(m_strInputMsg);
	string strMsg(pszMsg);
	m_Themis->doProcessSendMsg(nCurrentIndex, strMsg);

	CString strName(strUsername.c_str());
	CString strTime(m_Themis->currentDateTime().c_str());
	m_ChatZoom += _T("\r\n") + strName + _T(" < ") + strTime + _T(" >: ") + m_strInputMsg;

	SetDlgItemText(IDC_CHAT_ROOM, m_ChatZoom);

	UpdateData();

	SetDlgItemText(IDC_ED_INPUT, _T(""));
}


void CClearKeepDlg::OnBnClickedBtAddCt()
{
	CContactDlg pContactDlg;
	
	if (pContactDlg.DoModal() == IDOK)
	{
		CT2CA pszName(pContactDlg.m_strName);
		CT2CA pszId(pContactDlg.m_strId);
		string strNewCt(pszName);
		string strIdct(pszId);
		Contact pTemp;
		pTemp.set_name(strNewCt);
		pTemp.set_id(strIdct);

		if (m_Themis->doAddContactToList(pTemp) == 0)
		{
			doLoadContactToList();
		}
		m_Themis->doSendContactList();
	}
}


void CClearKeepDlg::OnNMClickListContact(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	if (m_Themis->m_ListContact.size() > 0) // Contact list is zero
	{
		int nIndex = m_ContactListCtrl.GetNextItem(-1, LVNI_SELECTED);
		if (nIndex == -1)
		{
			nIndex = m_ContactListCtrl.GetItemCount() - 1;
			m_ContactListCtrl.SetItemState(nIndex, LVIS_SELECTED, LVIS_SELECTED);
		}

		if (nCurrentIndex != nIndex)
		{
			LoadPeerInfo(nIndex);
			nCurrentIndex = nIndex;
		}
	}
	*pResult = 0;
}
