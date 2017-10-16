#pragma once


// CContactDlg dialog

class CContactDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CContactDlg)

public:
	CContactDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CContactDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CONTACT_DLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_strName;
	CString m_strId;
	virtual BOOL OnInitDialog();
};
