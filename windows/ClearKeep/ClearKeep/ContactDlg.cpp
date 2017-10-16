// ContactDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ClearKeep.h"
#include "ContactDlg.h"
#include "afxdialogex.h"


// CContactDlg dialog

IMPLEMENT_DYNAMIC(CContactDlg, CDialogEx)

CContactDlg::CContactDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_CONTACT_DLG, pParent)
	, m_strName(_T(""))
	, m_strId(_T(""))
{

}

CContactDlg::~CContactDlg()
{
}

void CContactDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_ED_NAME, m_strName);
	DDX_Text(pDX, IDC_ED_ID, m_strId);
}


BEGIN_MESSAGE_MAP(CContactDlg, CDialogEx)
END_MESSAGE_MAP()


// CContactDlg message handlers


BOOL CContactDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	GetDlgItem(IDC_ED_NAME)->SetFocus();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}
