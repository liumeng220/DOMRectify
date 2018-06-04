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

// DOMRectifyDoc.cpp : CDOMRectifyDoc 类的实现
//

#include "stdafx.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "DOMRectify.h"
#endif

#include "DOMRectifyDoc.h"

#include <propkey.h>
#include "MyFunctions.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CDOMRectifyDoc

IMPLEMENT_DYNCREATE(CDOMRectifyDoc, CDocument)

BEGIN_MESSAGE_MAP(CDOMRectifyDoc, CDocument)

END_MESSAGE_MAP()


// CDOMRectifyDoc 构造/析构

CDOMRectifyDoc::CDOMRectifyDoc()
{
	// TODO: 在此添加一次性构造代码
}

CDOMRectifyDoc::~CDOMRectifyDoc()
{
}

BOOL CDOMRectifyDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	if (theApp.GetOpenDocumentCount() == 1)
	{
		SetTitle("工程全览");
	}
	else
	{
		SetTitle("匹配点编辑");
	}
	// TODO: 在此添加重新初始化代码
	// (SDI 文档将重用该文档)

	return TRUE;
}




// CDOMRectifyDoc 序列化

void CDOMRectifyDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: 在此添加存储代码
	}
	else
	{
		// TODO: 在此添加加载代码
	}
}

#ifdef SHARED_HANDLERS

// 缩略图的支持
void CDOMRectifyDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// 修改此代码以绘制文档数据
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// 搜索处理程序的支持
void CDOMRectifyDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// 从文档数据设置搜索内容。
	// 内容部分应由“;”分隔

	// 例如:  strSearchContent = _T("point;rectangle;circle;ole object;")；
	SetSearchContent(strSearchContent);
}

void CDOMRectifyDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CDOMRectifyDoc 诊断

#ifdef _DEBUG
void CDOMRectifyDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CDOMRectifyDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CDOMRectifyDoc 命令

