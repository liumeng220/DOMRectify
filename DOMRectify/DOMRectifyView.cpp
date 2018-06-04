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

// DOMRectifyView.cpp : CDOMRectifyView 类的实现
//

#include "stdafx.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "DOMRectify.h"
#endif
#include "MyFunctions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDOMRectifyView

IMPLEMENT_DYNCREATE(CDOMRectifyView, ImageView)

BEGIN_MESSAGE_MAP(CDOMRectifyView, ImageView)
	// 	ON_WM_CONTEXTMENU()
	// 	ON_WM_RBUTTONUP()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_COMMAND(ID_BUTTON_POINT_ADD, &CDOMRectifyView::OnButtonPointAdd)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_POINT_ADD, &CDOMRectifyView::OnUpdateButtonPointAdd)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_COMMAND(ID_BUTTON_POINT_DELETE, &CDOMRectifyView::OnButtonPointDelete)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_POINT_DELETE, &CDOMRectifyView::OnUpdateButtonPointDelete)
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_COMMAND(ID_BUTTON_WRAPPER_CHECK, &CDOMRectifyView::OnButtonWrapperCheck)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_WRAPPER_CHECK, &CDOMRectifyView::OnUpdateButtonWrapperCheck)
END_MESSAGE_MAP()

// CDOMRectifyView 构造/析构

CDOMRectifyView::CDOMRectifyView()
{
	// TODO: 在此处添加构造代码
	m_hDC = NULL;
	theApp.m_bPointAdd = false;
	theApp.m_bPointDel = false;
	theApp.m_bWrapperView = false;
	theApp.m_bPtDom = false;
	theApp.m_bPtRef = false;
	theApp.m_nRefIdx = 0;
	m_bStartSelect = false;
	 theApp.m_bWrapperView = false;
	m_bStartWrapper = false;
	m_CheckOri = NONE;
}

CDOMRectifyView::~CDOMRectifyView()
{

}


// CDOMRectifyView 消息处理程序

void CDOMRectifyView::SyncDisplay(const OGREnvelope& enve)
{
	m_GroundRange = enve;
	PostMessage(WM_PAINT);
}

int CDOMRectifyView::GetCurImage()
{
	CMainFrame* pMainFrame = GetMainFramHand();
	CChildFrame* pChildFrame = GetChildFramHand();
	if (!pMainFrame || !pChildFrame)
	{
		return -1;
	}
	CPoint point; GetCursorPos(&point); ScreenToClient(&point);
	double x, y;
	Client2Ground(point.x, point.y, x, y);

	int nidx = 0;
	for (auto it = pMainFrame->m_ImgGridEnves.cbegin(); it != pMainFrame->m_ImgGridEnves.cend(); ++it)
	{
		if (x<it->second.MinX || x>it->second.MaxX || y<it->second.MinY || y>it->second.MaxY)
		{
			nidx++;
			continue;
		}
		return nidx;
	}
	return -1;
}

void CDOMRectifyView::DrawInsideImageFrame()
{
	CMainFrame *pMainFrame = GetMainFramHand();
	GLuint vboid = 0;
	if (!glIsBuffer(vboid))
	{
		glGenBuffers(1, &vboid);
	}
	glBindBuffer(GL_ARRAY_BUFFER, vboid);
	glBufferData(GL_ARRAY_BUFFER, 1 * 64, NULL, GL_DYNAMIC_DRAW);
	float vertex[16] = { 0.0 };
	int idx = GetCurImage();
	if (idx == -1) return;

	vertex[0] = pMainFrame->m_ImgGridEnves[idx].MinX; vertex[1] = pMainFrame->m_ImgGridEnves[idx].MinY;
	vertex[2] = pMainFrame->m_ImgGridEnves[idx].MinX; vertex[3] = pMainFrame->m_ImgGridEnves[idx].MaxY;

	vertex[4] = vertex[2]; vertex[5] = vertex[3];
	vertex[6] = pMainFrame->m_ImgGridEnves[idx].MaxX; vertex[7] = pMainFrame->m_ImgGridEnves[idx].MaxY;

	vertex[8] = vertex[6]; vertex[9] = vertex[7];
	vertex[10] = pMainFrame->m_ImgGridEnves[idx].MaxX; vertex[11] = pMainFrame->m_ImgGridEnves[idx].MinY;

	vertex[12] = vertex[10]; vertex[13] = vertex[11];
	vertex[14] = vertex[0]; vertex[15] = vertex[1];

	glBufferSubData(GL_ARRAY_BUFFER, 0, 64, vertex);

	glDisable(GL_DEPTH_TEST);
	glUniform1i(m_bTextureUniformID, 0);
	glUniform4f(m_GeometryColorUniformID, 0.0, 0.0, 1.0, 1.0);
	glLineWidth(3.0f);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	DrawPolys(1, vboid, m_VAOID);
}

void CDOMRectifyView::DrawMatchImageFrame()
{
	CMainFrame *pMainFrame = GetMainFramHand();
	if (pMainFrame->m_nImgIdxInMatchView >= 0)
	{
		GLuint vboid = 0;
		if (!glIsBuffer(vboid))
		{
			glGenBuffers(1, &vboid);
		}
		glBindBuffer(GL_ARRAY_BUFFER, vboid);
		glBufferData(GL_ARRAY_BUFFER, 1 * 64, NULL, GL_DYNAMIC_DRAW);
		float vertex[16] = { 0.0 };
		int idx = pMainFrame->m_nImgIdxInMatchView;
		if (idx == -1) return;

		vertex[0] = pMainFrame->m_ImgGridEnves[idx].MinX; vertex[1] = pMainFrame->m_ImgGridEnves[idx].MinY;
		vertex[2] = pMainFrame->m_ImgGridEnves[idx].MinX; vertex[3] = pMainFrame->m_ImgGridEnves[idx].MaxY;

		vertex[4] = vertex[2]; vertex[5] = vertex[3];
		vertex[6] = pMainFrame->m_ImgGridEnves[idx].MaxX; vertex[7] = pMainFrame->m_ImgGridEnves[idx].MaxY;

		vertex[8] = vertex[6]; vertex[9] = vertex[7];
		vertex[10] = pMainFrame->m_ImgGridEnves[idx].MaxX; vertex[11] = pMainFrame->m_ImgGridEnves[idx].MinY;

		vertex[12] = vertex[10]; vertex[13] = vertex[11];
		vertex[14] = vertex[0]; vertex[15] = vertex[1];

		glBufferSubData(GL_ARRAY_BUFFER, 0, 64, vertex);
		idx++;

		glDisable(GL_DEPTH_TEST);
		glUniform1i(m_bTextureUniformID, 0);
		glUniform4f(m_GeometryColorUniformID, 0.0, 1.0, 0.0, 1.0);
		glLineWidth(4.0f);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		DrawPolys(1, vboid, m_VAOID);
	}
}

void CDOMRectifyView::DrawOverlapBox()
{
	CMainFrame *pMainFrame = GetMainFramHand();
	if (pMainFrame->m_nImgIdxInMatchView >= 0)
	{
		GLuint vboid = 0;
		if (!glIsBuffer(vboid))
		{
			glGenBuffers(1, &vboid);
		}
		glBindBuffer(GL_ARRAY_BUFFER, vboid);
		glBufferData(GL_ARRAY_BUFFER, 1 * 64, NULL, GL_DYNAMIC_DRAW);
		float vertex[16] = { 0.0 };
		int idx = pMainFrame->m_nImgIdxInMatchView;
		if (idx == -1) return;
		OGREnvelope enveDom = pMainFrame->m_pReaderStere->GetImage(0)->GetGroundRange();
		OGREnvelope enveRef = pMainFrame->m_pReaderRef->GetImage(theApp.m_nRefIdx)->GetGroundRange();
		OGREnvelope enveInt = enveDom; enveInt.Intersect(enveRef); 

		vertex[0] =enveInt.MinX; vertex[1] =enveInt.MinY;
		vertex[2] =enveInt.MinX; vertex[3] =enveInt.MaxY;

		vertex[4] = vertex[2]; vertex[5] = vertex[3];
		vertex[6] = enveInt.MaxX; vertex[7] = enveInt.MaxY;

		vertex[8] = vertex[6]; vertex[9] = vertex[7];
		vertex[10] = enveInt.MaxX; vertex[11] = enveInt.MinY;

		vertex[12] = vertex[10]; vertex[13] = vertex[11];
		vertex[14] = vertex[0]; vertex[15] = vertex[1];

		glBufferSubData(GL_ARRAY_BUFFER, 0, 64, vertex);
		idx++;

		glDisable(GL_DEPTH_TEST);
		glUniform1i(m_bTextureUniformID, 0);
		glUniform4f(m_GeometryColorUniformID, 0.0, 1.0, 0.0, 1.0);
		glLineWidth(4.0f);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		DrawPolys(1, vboid, m_VAOID);
	}
}

void CDOMRectifyView::DrawSelectBox()
{
	if (!theApp.m_bPointDel) return;
	GLuint vboid = 0;
	if (!glIsBuffer(vboid))
	{
		glGenBuffers(1, &vboid);
	}
	glBindBuffer(GL_ARRAY_BUFFER, vboid);
	glBufferData(GL_ARRAY_BUFFER, 64, NULL, GL_DYNAMIC_DRAW);
	float vertex[16] = { 0.0 };
	vertex[0] = m_EnveSelectBox.MinX; vertex[1] = m_EnveSelectBox.MinY;
	vertex[2] = m_EnveSelectBox.MinX; vertex[3] = m_EnveSelectBox.MaxY;

	vertex[4] = vertex[2]; vertex[5] = vertex[3];
	vertex[6] = m_EnveSelectBox.MaxX; vertex[7] = m_EnveSelectBox.MaxY;

	vertex[8] = vertex[6]; vertex[9] = vertex[7];
	vertex[10] = m_EnveSelectBox.MaxX; vertex[11] = m_EnveSelectBox.MinY;

	vertex[12] = vertex[10]; vertex[13] = vertex[11];
	vertex[14] = vertex[0]; vertex[15] = vertex[1];

	glBufferSubData(GL_ARRAY_BUFFER, 0, 64, vertex);
	glUniform4f(m_GeometryColorUniformID, 0.8, 0.8, 0.8, 1.0);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	DrawPolys(1, vboid, m_VAOID);
}

void CDOMRectifyView::DrawSelectPoint()
{
	
	if (!theApp.m_bPointDel) return;
	CMainFrame* pMainFrame = GetMainFramHand();
	vector<stuMatchPoint> vecSelPoint;
	if (this == GetMainFramHand()->m_pLView)
	{
		vecSelPoint = pMainFrame->m_Project.GetSelDomPoints();
	}
	else if (this == GetMainFramHand()->m_pRView)
	{
		vecSelPoint = pMainFrame->m_Project.GetSelRefPoints();
	}
	if (vecSelPoint.size() == 0) return;
	GLuint vboid = 0;
	GetPointsMapBuffer(vecSelPoint, m_pReader->GetImage(0)->m_GSD, vboid);
	glDisable(GL_DEPTH_TEST);
	glUniform1i(m_bTextureUniformID, 0);
	glUniform4f(m_GeometryColorUniformID, 0.0, 1.0, 0.0, 1.0);
	glLineWidth(1.0f);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	DrawCross(vecSelPoint.size(), vboid, m_VAOID);
}

void CDOMRectifyView::DrawImageNames()
{
	vector<stuImgGroup> vImgGroup = GetMainFramHand()->m_Project.GetDomGroup();
	for (int ii = 0; ii<GetMainFramHand()->m_Project.GetDomNum(); ii++)
	{
	
		glColor3f(1, 0, 0);
		glRasterPos3f(0,0,0);
		char str[1024]; strcpy(str, FunGetFileName(vImgGroup[ii].strPanPath, true));
		selectFont(120, ANSI_CHARSET, "Comic Sans MS");
		static int isFirstCall = 1;
		static GLuint lists;
		if (isFirstCall)
		{
			isFirstCall = 0;
			lists = glGenLists(128);
			wglUseFontBitmaps(wglGetCurrentDC(), 0, 128, lists);
		}
		for (int i = 0; i < strlen(str); i++)
		{
			glCallList(lists + *(str + i));
		}
	}
}

void CDOMRectifyView::DrawTexture()
{
	if (!m_pReader) return;
	if (m_pReader->GetImageCount() > 0)
	{
		/************************************************************************/
		/*						            绘制缓冲区                          */
		/************************************************************************/
		OrthoImage* pOrtho = NULL;
		if (this == GetMainFramHand()->m_pMainView)
		{
			glDisable(GL_DEPTH_TEST);
		}
		else if (this == GetMainFramHand()->m_pLView)
		{
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_ALWAYS);
			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
			if (/*m_CheckOri == NONE||*/ !theApp.m_bWrapperView)
			{
				pOrtho = (OrthoImage*)m_pReader->GetImage(0);
				if (!pOrtho) return;
				if (pOrtho->GetTriangleCount() <= 0) return;
				DrawTriangles(pOrtho->GetTriangleCount(), pOrtho->GetTriangleVBO(), m_VAOID);
			}
			else	//wrapper
			{
				for (int i = m_pReader->GetImageCount() - 1; i >= 0; i--)
				{
					pOrtho = (OrthoImage*)m_pReader->GetImage(i);
					if (!pOrtho) return;
					if (pOrtho->GetTriangleCount() <= 0) return;
					DrawTriangles(pOrtho->GetTriangleCount(), pOrtho->GetTriangleVBO(), m_VAOID);
				}
				if(m_CheckOri!=NONE)
				{
					DrawQuads(1, m_OrinVBO, m_VAOID);
					DrawQuads(1, m_WrapVBO, m_VAOID);
				}
			}
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			glDepthFunc(GL_EQUAL);
		}
		else if (this == GetMainFramHand()->m_pRView)
		{
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_ALWAYS);
			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

			for (int i = 1; i<m_pReader->GetImageCount(); i++)
			{
				pOrtho = (OrthoImage*)m_pReader->GetImage(i);
				if (!pOrtho) return;
				if (pOrtho->GetTriangleCount() <= 0) return;
				DrawTriangles(pOrtho->GetTriangleCount(), pOrtho->GetTriangleVBO(), m_VAOID);
			}
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			glDepthFunc(GL_GEQUAL);

		}
		glUniform1i(m_bTextureUniformID, 1);
		int iTileCount = m_pReader->GetCurTileSum();
		ImageTile* pTile = NULL;
		for (int i = 0; i < iTileCount; ++i)
		{
			pTile = m_pReader->GetCurTile(i);
			if (pTile->m_pTexture)
			{
				pTile->DrawTex(m_VAOID);
			}
		}
	}
}

void CDOMRectifyView::DrawGeometry()
{


	CMainFrame* pMainFrame = GetMainFramHand();
	CChildFrame* pChildFrame = GetChildFramHand();

	if (!pChildFrame) return;
	if (!pMainFrame->m_bViewPoint) return;

	if (!pMainFrame || !pChildFrame)
	{
		return;
	}
	if (this == pMainFrame->m_pLView)
	{
		glDisable(GL_DEPTH_TEST);
		glUniform1i(m_bTextureUniformID, 0);
		glUniform4f(m_GeometryColorUniformID, 1.0, 0.0, 0.0, 1.0);
		glLineWidth(1.0f);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		DrawCross(pMainFrame->m_Project.GetCurPointNum(), pMainFrame->m_LPointsVBO, m_VAOID);
		if (theApp.m_bPtDom || theApp.m_bPtRef)
		{
			glUniform4f(m_GeometryColorUniformID, 1.0, 1.0, 0.0, 1.0);
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			DrawCross(1, pMainFrame->m_LPointsVBOAdd, m_VAOID);
		}
		if(m_bStartSelect)
			DrawSelectBox();
	}
	else if (this == pMainFrame->m_pRView)
	{
		glDisable(GL_DEPTH_TEST);
		glUniform1i(m_bTextureUniformID, 0);
		glUniform4f(m_GeometryColorUniformID, 1.0, 0.0, 0.0, 1.0);
		glLineWidth(1.0f);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		DrawCross(pMainFrame->m_Project.GetCurPointNum(), pMainFrame->m_RPointsVBO, m_VAOID);
		if (theApp.m_bPtRef || theApp.m_bPtDom)
		{
			glUniform4f(m_GeometryColorUniformID, 1.0, 1.0, 0.0, 1.0);
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			DrawCross(1, pMainFrame->m_RPointsVBOAdd, m_VAOID);
		}
	}
	DrawSelectPoint();

}

void CDOMRectifyView::DrawImageGrids()
{
	CMainFrame* pMainFrame = GetMainFramHand();
	CChildFrame* pChildFrame = GetChildFramHand();
	if (!pMainFrame || !pChildFrame)
	{
		return;
	}
	if (this == pMainFrame->m_pMainView)
	{
		glDisable(GL_DEPTH_TEST);
		glUniform1i(m_bTextureUniformID, 0);
		glUniform4f(m_GeometryColorUniformID, 1.0, 0.0, 0.0, 1.0);
		glLineWidth(2.0f);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		DrawPolys(pMainFrame->m_Project.GetGroupNum(), pMainFrame->m_ImgGridsVBO, m_VAOID);
	}
	DrawImageNames();
}

void CDOMRectifyView::DrawImageFrame()
{
	CMainFrame* pMainFrame = GetMainFramHand();
	CChildFrame* pChildFrame = GetChildFramHand();
	if (!pMainFrame || !pChildFrame)
	{
		return;
	}
	if (this == pMainFrame->m_pMainView)
	{
		DrawInsideImageFrame();
		DrawMatchImageFrame();
	}
	else
	{
		DrawOverlapBox();
	}
}

int CDOMRectifyView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (ImageView::OnCreate(lpCreateStruct) == -1)
		return -1;
	// 	if (!m_hDC)
	// 	{
	// 		CDC* pDC = new CClientDC(this);
	// 		m_hDC = pDC->GetSafeHdc();
	// 		if (!SetWindowPixelFormat(m_hDC))
	// 		{
	// 			AfxMessageBox("OpenGL环境创建错误！");
	// 			ASSERT(0);
	// 			return -1;
	// 		}
	// 		m_hRC = wglCreateContext(m_hDC);
	// 		m_LRC = wglCreateContext(m_hDC);
	// 		m_RRC = wglCreateContext(m_hDC);
	// 
	// 		DWORD lastErr = 0;
	// 		if (!wglShareLists(m_hRC, m_LRC))
	// 		{
	// 			lastErr = GetLastError();
	// 		}
	// 		if (!wglShareLists(m_hRC, m_RRC))
	// 		{
	// 			lastErr = GetLastError();
	// 		}
	// 		//初始化GLEW
	// 		wglMakeCurrent(m_hDC, m_hRC);
	// 		GLenum err = glewInit();
	// 		wglMakeCurrent(NULL, NULL);
	// 	}


	CChildFrame *pChildFrame = GetChildFramHand();
	if (pChildFrame)
	{
		CMainFrame *pMainFrame = GetMainFramHand();
		if (!pMainFrame->m_pMainView)
		{
			pMainFrame->m_pMainView = this;
		}
	}

	// TODO:  Add your specialized creation code here

	return 0;
}


void CDOMRectifyView::OnSize(UINT nType, int cx, int cy)
{
	ImageView::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
}

void CDOMRectifyView::OnInitialUpdate()
{
	ImageView::OnInitialUpdate();
	// TODO: Add your specialized code here and/or call the base class
}


void CDOMRectifyView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (this == GetMainFramHand()->m_pMainView)
	{
		int idx = GetCurImage(); if (idx < 0) return;
		GetMainFramHand()->SendMessage(WM_OPEN_MATCH_VIEW, (WPARAM)&idx, 0);
	}
	ImageView::OnLButtonDblClk(nFlags, point);
}


void CDOMRectifyView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (this == GetMainFramHand()->m_pLView&& theApp.m_bWrapperView)
	{
		m_bStartWrapper = true;
	}

	if (theApp.m_bPointAdd)
	{
		double x, y; Client2Ground(point.x, point.y, x, y);
		if (this == GetMainFramHand()->m_pLView)
		{
			theApp.m_bPtDom = true;
			theApp.m_ptDom.setX(x);
			theApp.m_ptDom.setY(y);
			::SendMessage(GetMainFramHand()->m_hWnd, WM_ADD_MATCHPOINT, 1, 0);
			if (!theApp.m_bPtRef)
			{
				theApp.m_ptRef = theApp.m_ptDom;
				::SendMessage(GetMainFramHand()->m_hWnd, WM_ADD_MATCHPOINT, 0, 1);
			}
		}
		else if (this == GetMainFramHand()->m_pRView&&theApp.m_bPtDom)
		{
//			theApp.m_bPtRef = true;
//			theApp.m_nRefIdx = 0;  //设置当前刺点参考影像
			theApp.m_ptRef.setX(x);
			theApp.m_ptRef.setY(y);
			::SendMessage(GetMainFramHand()->m_hWnd, WM_ADD_MATCHPOINT, 0, 1);
		}
	}
	else if (this == GetMainFramHand()->m_pLView&&theApp.m_bPointDel)
	{
		m_EnveSelectBox = OGREnvelope(); 
		double x, y; Client2Ground(point.x, point.y, x, y);
		m_EnveSelectBox.MinX = x;
		m_EnveSelectBox.MinY = y;
	}
	ImageView::OnLButtonDown(nFlags, point);
}


void CDOMRectifyView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (m_bStartWrapper&& theApp.m_bWrapperView)
	{
		if (GetAsyncKeyState(VK_LBUTTON))
		{
			m_CheckOri = LEFT_RIGHT;
		}
		else if (GetAsyncKeyState(VK_RBUTTON))
		{
			m_CheckOri = BOTTOM_UP;
		}
		else
		{
			return;
		}
		float vertex_orin[12] = { 0.0 };
		float vertex_wrap[12] = { 0.0 };
		double gx = 0.0, gy = 0.0;
		Client2Ground(point.x, point.y, gx, gy);
		CMainFrame* pFrm = (CMainFrame*)theApp.GetMainWnd();
		Image* pWrap = pFrm->m_pReaderStere->GetImage(0);
		if (!pWrap)
			return;
		OGREnvelope enve = pWrap->GetGroundRange();


		if (m_CheckOri == BOTTOM_UP)
		{
			vertex_orin[0] = enve.MinX; vertex_orin[1] = enve.MinY;  vertex_orin[2] = 1.0;
			vertex_orin[3] = enve.MinX; vertex_orin[4] = enve.MaxY;  vertex_orin[5] = 1.0;
			vertex_orin[6] = gx;        vertex_orin[7] = enve.MaxY;  vertex_orin[8] = 1.0;
			vertex_orin[9] = gx;        vertex_orin[10] = enve.MinY; vertex_orin[11] = 1.0;

			vertex_wrap[0] = gx;        vertex_wrap[1] = enve.MinY;  vertex_wrap[2] = 2.0;
			vertex_wrap[3] = gx;        vertex_wrap[4] = enve.MaxY;  vertex_wrap[5] = 2.0;
			vertex_wrap[6] = enve.MaxX; vertex_wrap[7] = enve.MaxY;  vertex_wrap[8] = 2.0;
			vertex_wrap[9] = enve.MaxX; vertex_wrap[10] = enve.MinY; vertex_wrap[11] = 2.0;
		}
		else
		{
			vertex_wrap[0] = enve.MinX; vertex_wrap[1] = enve.MinY;  vertex_wrap[2] = 2.0;
			vertex_wrap[3] = enve.MinX; vertex_wrap[4] = gy;         vertex_wrap[5] = 2.0;
			vertex_wrap[6] = enve.MaxX; vertex_wrap[7] = gy;         vertex_wrap[8] = 2.0;
			vertex_wrap[9] = enve.MaxX; vertex_wrap[10] = enve.MinY; vertex_wrap[11] = 2.0;

			vertex_orin[0] = enve.MinX; vertex_orin[1] = gy;        vertex_orin[2] = 1.0;
			vertex_orin[3] = enve.MinX; vertex_orin[4] = enve.MaxY; vertex_orin[5] = 1.0;
			vertex_orin[6] = enve.MaxX; vertex_orin[7] = enve.MaxY; vertex_orin[8] = 1.0;
			vertex_orin[9] = enve.MaxX; vertex_orin[10] = gy;       vertex_orin[11] = 1.0;
		}
		MultiThreadWGLMakeCurrent(m_hDC, m_hRC);
		glBindBuffer(GL_ARRAY_BUFFER, m_OrinVBO);
		glBufferData(GL_ARRAY_BUFFER, 48, vertex_orin, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, m_WrapVBO);
		glBufferData(GL_ARRAY_BUFFER, 48, vertex_wrap, GL_DYNAMIC_DRAW);
		MultiThreadWGLMakeCurrent(NULL, NULL);
	}

	if (this == GetMainFramHand()->m_pLView && theApp.m_bPointDel && nFlags == MK_LBUTTON)
	{
		double x, y; Client2Ground(point.x, point.y, x, y);
		m_EnveSelectBox.MaxX = x;
		m_EnveSelectBox.MaxY = y;
		GetMainFramHand()->m_Project.SelPoint(m_EnveSelectBox);
		m_bStartSelect = true;

	}
	ImageView::OnMouseMove(nFlags, point);
}


void CDOMRectifyView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (this == GetMainFramHand()->m_pLView&&m_bStartWrapper&& theApp.m_bWrapperView)
	{
		m_bStartWrapper = false;
		m_CheckOri = NONE;
	}
	if (this == GetMainFramHand()->m_pLView && theApp.m_bPointDel)
	{
		m_bStartSelect = false;
		m_EnveSelectBox = OGREnvelope(); 
		::SendMessage(GetMainFramHand()->m_hWnd, WM_DELETE_MATCHPOINT, 0, 0);
	}
	ImageView::OnLButtonUp(nFlags, point);
}


 void CDOMRectifyView::OnButtonPointAdd()
 {
 	// TODO: Add your command handler code here
	 theApp.m_bPointAdd = !theApp.m_bPointAdd;
	 if (!theApp.m_bPointAdd)
	 {
		 theApp.m_bPtDom = false;
		 theApp.m_bPtRef = false;
	 }
	 else
	 {
		 theApp.m_bPointDel = false;
		 theApp.m_bWrapperView = false;
	 }
 }


 void CDOMRectifyView::OnUpdateButtonPointAdd(CCmdUI *pCmdUI)
 {
	 // TODO: Add your command update UI handler code here
// 	 if (theApp.GetOpenDocumentCount() != 2)
// 	 {
// 		 pCmdUI->Enable(FALSE);
// 	 }
// 	 else
// 	 {
		 pCmdUI->Enable(true);
		 pCmdUI->SetCheck(theApp.m_bPointAdd);
//	 }
 }




 void CDOMRectifyView::OnButtonPointDelete()
 {
	 // TODO: Add your command handler code here
	 theApp.m_bPointDel = !theApp.m_bPointDel;
	 if (!theApp.m_bPointDel)
	 {
		 theApp.m_bPtDom = false;
		 theApp.m_bPtRef = false;
		 theApp.m_bPointAdd = false;
	 }
	 else
	 {
		 theApp.m_bWrapperView = false;
		 theApp.m_bPointAdd = false;
	 }

 }


 void CDOMRectifyView::OnUpdateButtonPointDelete(CCmdUI *pCmdUI)
 {
	 // TODO: Add your command update UI handler code here
// 	 if (theApp.GetOpenDocumentCount() != 2)
// 	 {
// 		 pCmdUI->Enable(FALSE);
// 	 }
// 	 else
// 	 {
		 pCmdUI->Enable(true);
		 pCmdUI->SetCheck(theApp.m_bPointDel);
//	 }

 }


 void CDOMRectifyView::OnRButtonDown(UINT nFlags, CPoint point)
 {
	 // TODO: Add your message handler code here and/or call default
	 if (this == GetMainFramHand()->m_pLView&&theApp.m_bWrapperView)
	 {
		 m_bStartWrapper = true;
	 }
	 if (theApp.m_bPtDom||theApp.m_bPtRef)
	 {
		 ::SendMessage(GetMainFramHand()->m_hWnd, WM_ADD_MATCHPOINT, 1, 1);
		 theApp.m_bPtRef = false;
		 theApp.m_bPtDom = false;
	 }
	 ImageView::OnRButtonDown(nFlags, point);
 }


 void CDOMRectifyView::OnRButtonUp(UINT nFlags, CPoint point)
 {
	 // TODO: Add your message handler code here and/or call default
	 if (this == GetMainFramHand()->m_pLView&&m_bStartWrapper&&theApp.m_bWrapperView)
	 {
		 m_bStartWrapper = false;
		 m_CheckOri = NONE;
	 }
	 ImageView::OnRButtonUp(nFlags, point);
 }


 void CDOMRectifyView::OnButtonWrapperCheck()
 {
	 // TODO: Add your command handler code here
	  theApp.m_bWrapperView = ! theApp.m_bWrapperView;
	 if ( theApp.m_bWrapperView)
	 {
		 theApp.m_bPointAdd = false;
		 theApp.m_bPointDel = false;
	 }
 }


 void CDOMRectifyView::OnUpdateButtonWrapperCheck(CCmdUI *pCmdUI)
 {
	 // TODO: Add your command update UI handler code here
	 pCmdUI->SetCheck( theApp.m_bWrapperView);
 }


 BOOL CDOMRectifyView::PreTranslateMessage(MSG* pMsg)
 {
	 // TODO: Add your specialized code here and/or call the base class
	 if (pMsg->message == WM_KEYDOWN)
	 {
		 switch (pMsg->wParam)
		 {
		 case 'a'-32:  //加入匹配列表
			 if (theApp.GetOpenDocumentCount() == 2)
			 {
				 GetMainFramHand()->AddToMatchList();
			 }
			 break;
		 default:
			 break;
		 }
	 }
	 return ImageView::PreTranslateMessage(pMsg);
 }
