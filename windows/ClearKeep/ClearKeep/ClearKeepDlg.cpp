
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
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	
}

void CClearKeepDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_CONTACT, m_ContactListCtrl);
}

BEGIN_MESSAGE_MAP(CClearKeepDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
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

	

	// Init Themis class
	m_Themis = CThemis::GetSingleTon();
	m_LoginDlg = new CLoginDlg();

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


	// Contact list loading
	InitTabContact();

}

void CClearKeepDlg::InitTabContact()
{
	if (m_Themis->m_ListContact.size() > 0)
	{
		for (unsigned i = 0; i < m_Themis->m_ListContact.size(); i++)
		{
			Contact pCtTemp = m_Themis->m_ListContact.at(i);
			CString strAccName(pCtTemp.name().c_str());
			m_ContactListCtrl.AddString(strAccName);
		}
	}
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
