#pragma once


// CMyMfcListCtrl

class CMyMfcListCtrl : public CMFCListCtrl
{
	DECLARE_DYNAMIC(CMyMfcListCtrl)

public:
	CMyMfcListCtrl();
	virtual ~CMyMfcListCtrl();

protected:
	DECLARE_MESSAGE_MAP()
	virtual COLORREF OnGetCellTextColor(int nRow, int nColumn);
};


