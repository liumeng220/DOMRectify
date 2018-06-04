// 这段 MFC 示例源代码演示如何使用 MFC Microsoft Office Fluent 用户界面 
// (“Fluent UI”)。该示例仅供参考，
// 用以补充《Microsoft 基础类参考》和 
// MFC C++ 库软件随附的相关电子文档。
// 复制、使用或分发 Fluent UI 的许可条款是单独提供的。
// 若要了解有关 Fluent UI 许可计划的详细信息，请访问  
// http://msdn.microsoft.com/officeui。
//
// 版权所有(C) Microsoft Corporation
// 保留所有权利。

// ChildFrm.cpp : CChildFrame 类的实现
//

#include "stdafx.h"
#include "DOMRectify.h"

#include "ChildFrm.h"
#include "MyFunctions.h"
#include "DlgPrjNew.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CChildFrame

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWndEx)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWndEx)
	ON_COMMAND(ID_FILE_NEW, &CChildFrame::OnFileNew)
	ON_WM_CLOSE()
	ON_COMMAND(ID_FILE_OPEN, &CChildFrame::OnFileOpen)
END_MESSAGE_MAP()

// CChildFrame 构造/析构

CChildFrame::CChildFrame()
{
	// TODO: 在此添加成员初始化代码
	m_bSpliter = false;
}


CChildFrame::~CChildFrame()
{
	DWORD lastError = 0;
	if (!wglDeleteContext(m_hRC))
	{
		lastError = GetLastError();
	}
	if (!wglDeleteContext(m_RRC))
	{
		lastError = GetLastError();
	}
	if (!wglDeleteContext(m_LRC))
	{
		lastError = GetLastError();
	}
	
}

BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改 CREATESTRUCT cs 来修改窗口类或样式
	if( !CMDIChildWndEx::PreCreateWindow(cs) )
		return FALSE;
	return TRUE;
}

// CChildFrame 诊断

#ifdef _DEBUG
void CChildFrame::AssertValid() const
{
	CMDIChildWndEx::AssertValid();
}

void CChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWndEx::Dump(dc);
}
#endif //_DEBUG

// CChildFrame 消息处理程序


void CChildFrame::OnFileNew()
{
	// TODO: Add your command handler code here
	GetMainFramHand()->OnFileNew();
}

void CChildFrame::OnFileOpen()
{
	// TODO: Add your command handler code here
	GetMainFramHand()->OnFileOpen();
}


void CChildFrame::OnDestroy()
{
	CMDIChildWndEx::OnDestroy();

	// TODO: Add your message handler code here
}


void CChildFrame::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	if (GetActiveDocument()->GetTitle() == "工程全览")
	{
		GetMainFramHand()->ClearData();
		theApp.CloseAllDocuments(TRUE);	
	}
	else
	{
		GetMainFramHand()->CloseStereDoc();
	}
	m_bSpliter = false;
}


BOOL CChildFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	// TODO: Add your specialized code here and/or call the base class
	if (theApp.m_pDocManager->GetOpenDocumentCount() == 2)
	{
		CRect rt;  GetClientRect(&rt);
		CRect rt_Left;  rt_Left.SetRectEmpty();
		if(GetMainFramHand()->m_wndImgData.GetSafeHwnd())
			GetMainFramHand()->m_wndImgData.GetClientRect(rt_Left);
		if (!m_wndSpliter.CreateStatic(this, 1, 2, WS_CHILD | WS_VISIBLE))
		{
			AfxMessageBox(_T("分割窗口错误."));
			return false;
		}
		m_wndSpliter.CreateView(0, 0, RUNTIME_CLASS(CDOMRectifyView), CSize((rt.Width() + rt_Left.Width()) / 2, rt.Height()), pContext);
		m_wndSpliter.CreateView(0, 1, RUNTIME_CLASS(CDOMRectifyView), CSize((rt.Width() + rt_Left.Width()) / 2, rt.Height()), pContext);
		m_bSpliter = true;
		GetMainFramHand()->m_pLView = (CDOMRectifyView*)m_wndSpliter.GetPane(0, 0);
		GetMainFramHand()->m_pRView = (CDOMRectifyView*)m_wndSpliter.GetPane(0, 1);

		m_hDC = NULL; m_hRC = NULL;
		m_hDC = GetDC()->GetSafeHdc();
		if (m_hDC == NULL) return FALSE;
		if (SetWindowPixelFormat(m_hDC) == FALSE)
		{
			ASSERT(0);
			return FALSE;
		}
		m_hRC = wglCreateContext(m_hDC);
		m_LRC = wglCreateContext(m_hDC);
		m_RRC = wglCreateContext(m_hDC);
		DWORD lastErr = 0;
		if (!wglShareLists(m_hRC, m_LRC))
		{
			lastErr = GetLastError();
		}
		if (!wglShareLists(m_hRC, m_RRC))
		{
			lastErr = GetLastError();
		}
		GetMainFramHand()->m_pLView->SetRC(m_LRC);
		GetMainFramHand()->m_pRView->SetRC(m_RRC);
		wglMakeCurrent(m_hDC, m_hRC);
		GLenum err = glewInit();
		wglMakeCurrent(NULL, NULL);
		return true;
	}
	else
	{
		m_hDC = NULL; m_hRC = NULL;
		m_hDC = GetDC()->GetSafeHdc();
		if (m_hDC == NULL) return FALSE;
		if (SetWindowPixelFormat(m_hDC) == FALSE)
		{
			ASSERT(0);
			return FALSE;
		}
		m_hRC = wglCreateContext(m_hDC);
		wglMakeCurrent(m_hDC, m_hRC);
		GLenum err = glewInit();
		wglMakeCurrent(NULL, NULL);
	}
	return CMDIChildWndEx::OnCreateClient(lpcs, pContext);
}

void CChildFrame::ActivateFrame(int nCmdShow)
{
	// TODO: Add your specialized code here and/or call the base class
	nCmdShow = SW_SHOWMAXIMIZED;
	CMDIChildWndEx::ActivateFrame(nCmdShow);
}

