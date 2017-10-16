// MyMfcListCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "ClearKeep.h"
#include "MyMfcListCtrl.h"


// CMyMfcListCtrl

IMPLEMENT_DYNAMIC(CMyMfcListCtrl, CMFCListCtrl)

CMyMfcListCtrl::CMyMfcListCtrl()
{
#ifndef _WIN32_WCE
	EnableActiveAccessibility();
#endif

}

CMyMfcListCtrl::~CMyMfcListCtrl()
{
}


COLORREF CMyMfcListCtrl::OnGetCellTextColor(int nRow, int nColumn)
{
	//CMyClass* pMyClass = (CMyClass*)GetItemData(nRow);
	//if (pMyClass && pMyClass->m_bDeleted)
	//	return RGB(255, 0, 0);

	return CMyMfcListCtrl::OnGetCellTextColor(nRow, nColumn);
}

BEGIN_MESSAGE_MAP(CMyMfcListCtrl, CMFCListCtrl)
END_MESSAGE_MAP()



// CMyMfcListCtrl message handlers


