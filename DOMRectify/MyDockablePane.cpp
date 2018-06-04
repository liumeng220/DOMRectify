// MyDockablePane.cpp : implementation file
//

#include "stdafx.h"
#include "DOMRectify.h"
#include "MyDockablePane.h"


// CMyDockablePane

IMPLEMENT_DYNAMIC(CMyDockablePane, CDockablePane)

CMyDockablePane::CMyDockablePane()
{

}

CMyDockablePane::~CMyDockablePane()
{
}


BEGIN_MESSAGE_MAP(CMyDockablePane, CDockablePane)
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()



// CMyDockablePane message handlers




void CMyDockablePane::OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/)
{
	// TODO: Add your message handler code here
}
