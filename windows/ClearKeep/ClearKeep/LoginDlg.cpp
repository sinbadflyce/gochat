// LoginDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ClearKeep.h"
#include "LoginDlg.h"
#include "afxdialogex.h"

#include "ClearKeepDlg.h"

// CLoginDlg dialog

IMPLEMENT_DYNAMIC(CLoginDlg, CDialogEx)

CLoginDlg::CLoginDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_LOGIN, pParent)
{
	pMainDlg = (CClearKeepDlg*)AfxGetMainWnd();

	//SetDlgItemText(IDC_ED_USERNAME, _T("minh"));
	//SetDlgItemText(IDC_ED_PW, _T("matkhau"));
}

CLoginDlg::~CLoginDlg()
{
}

void CLoginDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ED_USERNAME, m_UserNameCtrl);
	DDX_Control(pDX, IDC_ED_PW, m_PasswordCtrl);
}


BOOL CLoginDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	
	SetDlgItemText(IDC_ED_USERNAME, _T("minh"));
	SetDlgItemText(IDC_ED_PW, _T("xxx"));
	return TRUE;
}

BEGIN_MESSAGE_MAP(CLoginDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CLoginDlg::OnBnClickedOk)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BT_REGISTER, &CLoginDlg::OnBnClickedBtRegister)
	ON_BN_CLICKED(IDCANCEL, &CLoginDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CLoginDlg message handlers


void CLoginDlg::OnBnClickedOk()
{
	// check validate of username and password format
	if (ChectValidate())
	{
		MessageBox(_T("User name or Password is too short!"), _T("Login Fail"));
		showNotifyCheck(true);
		m_UserNameCtrl.SetSel(0, -1, FALSE);
		m_UserNameCtrl.SetFocus();
		return;
	}

	// Convert a TCHAR string to a LPCSTR
	CT2CA pszConvertedAnsiString1(strUn);
	// construct a std::string using the LPCSTR input
	std::string strUserName(pszConvertedAnsiString1);

	// Convert a TCHAR string to a LPCSTR
	CT2CA pszConvertedAnsiString2(strPw);
	// construct a std::string using the LPCSTR input
	std::string strPassword(pszConvertedAnsiString2);

	if (pMainDlg)
	{
		LOGIN_STATUS res = pMainDlg->doLogin(strUserName, strPassword);
		switch (res)
		{
		case login_success:
			CDialogEx::OnOK();
			break;
		case login_fail_username:
			GetDlgItem(IDC_STAR1)->ShowWindow(SW_SHOW);
			m_UserNameCtrl.SetSel(0, -1, FALSE);
			m_UserNameCtrl.SetFocus();
			MessageBox(_T("User name or Password is invalid!"), _T("Login Fail"));
			break;
		case login_fail_password:
			m_UserNameCtrl.SetSel(0, -1, FALSE);
			m_UserNameCtrl.SetFocus();
			GetDlgItem(IDC_STAR1)->ShowWindow(SW_SHOW);
			MessageBox(_T("User name or Password is invalid!"), _T("Login Fail"));
			break;
		case login_error:
			m_UserNameCtrl.SetSel(0, -1, FALSE);
			m_UserNameCtrl.SetFocus();
			MessageBox(_T("Please check connection!"), _T("Login Fail"));
			break;
		case login_unknow_error:
			m_UserNameCtrl.SetSel(0, -1, FALSE);
			m_UserNameCtrl.SetFocus();
			MessageBox(_T("Unknow Error!"), _T("Login Fail"));
			break;
		default:
			break;
		}
	}
}


int CLoginDlg::ChectValidate()
{
	GetDlgItemText(IDC_ED_USERNAME, strUn);
	GetDlgItemText(IDC_ED_PW, strPw);

	if (strUn.GetLength() < 4 || strPw.GetLength() < 1)
	{
		return -1;
	}

	return 0;
}


void CLoginDlg::showNotifyCheck(bool b_show)
{
	if (b_show)
	{
		GetDlgItem(IDC_STAR1)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_STAR2)->ShowWindow(SW_SHOW);
	}
	else
	{
		GetDlgItem(IDC_STAR1)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_STAR2)->ShowWindow(SW_HIDE);
	}
}

HBRUSH CLoginDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
	// conflict when use gdi32.lib to draw that. NOT solved it YET [9/25/2017 Canhnh]
	
	switch (nCtlColor)
	{
	case CTLCOLOR_STATIC:
		if (pWnd->GetSafeHwnd() == GetDlgItem(IDC_STAR1)->GetSafeHwnd() ||
			pWnd->GetSafeHwnd() == GetDlgItem(IDC_STAR2)->GetSafeHwnd())
		{
			pDC->SetTextColor(RGB(255, 0, 0));
		}
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	default:
		return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	}
	
	return hbr;
}


void CLoginDlg::OnBnClickedBtRegister()
{
	MessageBox(_T("Not finish yet!"), _T("Warning"));
}


void CLoginDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnCancel();
}
