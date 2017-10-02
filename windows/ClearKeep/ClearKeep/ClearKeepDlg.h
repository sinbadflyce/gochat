
// ClearKeepDlg.h : header file
//

#pragma once
#include "CThemis.h"
#include "LoginDlg.h"
#include "afxcmn.h"
#include "afxwin.h"


// CClearKeepDlg dialog
class CClearKeepDlg : public CDialogEx
{
// Construction
public:
	CClearKeepDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CLEARKEEP_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

public:
// Khai bao cac bien de su dung [9/19/2017 Canhnh]
	CThemis* m_Themis;
	string strUsername;
	string strPass;
	bool m_bConnected;
	
	// Init all varial and function [9/19/2017 Canhnh]
	int InitLogging();
	int Init();

	// Work with login dialog [9/19/2017 Canhnh]
	bool CallLogin();

	// Connect server to login [9/19/2017 Canhnh]
	// Values return: 
	LOGIN_STATUS doLogin(const string strName, const string strPassword);


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	// - thread check login
	static UINT doCheckLogin(void * pClass);
	bool m_bIsLogin;
	void doLoginProcess();
	CLoginDlg *m_LoginDlg;

	void InitTabContact();
public:
	afx_msg void OnClose();
	CListBox m_ContactListCtrl;
};
