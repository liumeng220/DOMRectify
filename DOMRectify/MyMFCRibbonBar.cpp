// MyMFCRibbonBar.cpp : implementation file
//

#include "stdafx.h"
#include "DOMRectify.h"
#include "MyMFCRibbonBar.h"


// CMyMFCRibbonBar

IMPLEMENT_DYNAMIC(CMyMFCRibbonBar, CMFCRibbonBar)

CMyMFCRibbonBar::CMyMFCRibbonBar()
{

}

CMyMFCRibbonBar::~CMyMFCRibbonBar()
{
}


BEGIN_MESSAGE_MAP(CMyMFCRibbonBar, CMFCRibbonBar)
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()



// CMyMFCRibbonBar message handlers




void CMyMFCRibbonBar::OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/)
{
	// TODO: Add your message handler code here
}
