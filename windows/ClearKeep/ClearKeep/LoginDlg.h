#pragma once
#include "afxwin.h"

class CClearKeepDlg;

// CLoginDlg dialog

class CLoginDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CLoginDlg)

public:
	CLoginDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CLoginDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LOGIN };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	// get main dialog [9/19/2017 Canhnh]
	CClearKeepDlg* pMainDlg;
	
	CString strUn;
	CString strPw;

	// check validate of user name and password [9/19/2017 Canhnh]
	int ChectValidate();
	void showNotifyCheck(bool b_show);

	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedBtRegister();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
	CEdit m_UserNameCtrl;
	CEdit m_PasswordCtrl;
};
