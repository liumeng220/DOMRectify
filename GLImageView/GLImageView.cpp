// GLImageView.cpp : 定义 DLL 的初始化例程。
//

#include "stdafx.h"
#include "GLImageView.h"

#define FORECAST_TIMER_ID   120
#define FORECAST_TIME       10    

#include "math.h"
#include "gl/vmath.h"
#include "GL/GLConfiguration.h"
#include <fstream>
#include "MosaicShader.h"
#include "GL/GLObjects.h"

IMPLEMENT_DYNCREATE(ImageView, CView)
BEGIN_MESSAGE_MAP(ImageView, CView)
	ON_WM_ERASEBKGND()
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_TIMER()
END_MESSAGE_MAP()


vmath::mat4 translatematrix;
vmath::mat4 scalematrix;
vmath::mat4 transformmatrix;

ImageView::ImageView()
{
	m_hRC = NULL;
	m_hDC = NULL;
	m_pReader = NULL;
	m_hRCShare = NULL;
	m_MainShader = 0;
	m_GeometryColorUniformID = -1;
	m_MVMatrixUniformID = -1;
	m_bTextureUniformID = -1;
	m_bStop = false;
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
}

ImageView::~ImageView()
{
	MultiThreadWGLMakeCurrent(m_hDC, m_hRC);
	if (glIsBuffer(m_ZoomRCVBO))
	{
		glDeleteBuffers(1, &m_ZoomRCVBO);
	}
	if (glIsVertexArray(m_VAOID))
	{
		glDeleteVertexArrays(1, &m_VAOID);
	}
	if (glIsProgram(m_MainShader))
	{
		glDeleteShader(m_MainShader);
	}
	MultiThreadWGLMakeCurrent(NULL, NULL);
}

HGLRC ImageView::GetRC()
{
	return m_hRC;
}

HGLRC ImageView::GetShareRC()
{
	return m_hRCShare;
}

void ImageView::SetRC(HGLRC rc)
{
	m_hRC = rc;
}

void ImageView::OnInitialUpdate()
{
	m_hDC = GetDC()->GetSafeHdc();
	if (m_hDC == NULL) return;
	if (SetWindowPixelFormat(m_hDC) == FALSE)
	{
		ASSERT(0);
		return;
	}
	if (!m_hRC)
	{
		CreateViewGLContext(m_hDC, m_hRC);
	}
	
	if (!m_hRCShare)
	{
		CreateViewGLContext(m_hDC, m_hRCShare);
	}
	if (!wglShareLists(m_hRC, m_hRCShare))
	{
		ASSERT(0);
	}

	m_MainShader = LoadShaders(drawvert, drawfrag, m_hDC, m_hRC);
	MultiThreadWGLMakeCurrent(m_hDC, m_hRC);
	glUseProgram(m_MainShader);
	m_MVMatrixUniformID = glGetUniformLocation(m_MainShader, "mvmatrix");
	m_bTextureUniformID = glGetUniformLocation(m_MainShader, "btex");
	m_GeometryColorUniformID = glGetUniformLocation(m_MainShader, "linecolor");
	glUniform4f(m_GeometryColorUniformID, 1.0, 1.0, 0.0, 1.0);
	glGenVertexArrays(1, &m_VAOID);
	/************************************************************************/
	/*						创建局部放大的矩形VBO                             */
	/************************************************************************/
	glGenBuffers(1, &m_ZoomRCVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_ZoomRCVBO);
	glBufferData(GL_ARRAY_BUFFER, 32, NULL, GL_DYNAMIC_DRAW);
	m_StartPoint.setX(0.0);
	m_StartPoint.setY(0.0);
	m_EndPoint.setX(0.0);
	m_EndPoint.setY(0.0);
	MultiThreadWGLMakeCurrent(NULL, NULL);
	CView::OnInitialUpdate();
}

void ImageView::BeginDraw()
{
	if (m_hDC == NULL || m_hRC == NULL)
	{
		return;
	}
	MultiThreadWGLMakeCurrent(m_hDC, m_hRC);
	translatematrix = vmath::translate<float>(-(m_GroundRange.MinX + m_GroundRange.MaxX) / 2.0, -(m_GroundRange.MinY + m_GroundRange.MaxY) / 2.0, 0.0);
	scalematrix = vmath::scale<float>(2.0 / (m_GroundRange.MaxX - m_GroundRange.MinX), 2.0 / (m_GroundRange.MaxY - m_GroundRange.MinY), 1.0 / 10000.0);
	transformmatrix = scalematrix * translatematrix;
	if (m_MainShader == 0)
	{
		if (!wglMakeCurrent(NULL, NULL))
		{
			ASSERT(0);
		}
		return;
	}
	glUseProgram(m_MainShader);
	glUniformMatrix4fv(m_MVMatrixUniformID, 1, GL_FALSE, transformmatrix);
	glUniform1i(m_bTextureUniformID, 0);
	glViewport(m_ViewPort.MinX, m_ViewPort.MinY, m_ViewPort.MaxX, m_ViewPort.MaxY);
	glDepthRange(0.0, 1.0);
    glEnable(GL_DEPTH_TEST);
	glDrawBuffer(GL_BACK);
//	glClearColor(200.0 / 255.0, 200.0 / 255.0, 200.0 / 255.0, 1.0);
	glClearColor(0,0,0, 1.0);
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void ImageView::DrawTexture()
{

}

void ImageView::DrawGeometry()
{

}
void ImageView::DrawImageGrids()
{

}

void ImageView::DrawImageFrame()
{
}

void ImageView::Draw()
{
	DrawTexture();
	DrawGeometry();
	DrawImageGrids();
	DrawImageFrame();
}

void ImageView::EndDraw()
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glFinish();
	SwapBuffers(m_hDC);
	MultiThreadWGLMakeCurrent(NULL, NULL);
	SetTimer(FORECAST_TIMER_ID, FORECAST_TIME, NULL);
}

void ImageView::OnDraw(CDC* pDC)
{
	BeginDraw();
	Draw();
	EndDraw();
}

void ImageView::InitialEnvelope(const OGREnvelope& enve)
{
	CRect rt;
	GetClientRect(&rt);
	OGREnvelope winenve;
	m_ViewPort.MinX = rt.left;
	m_ViewPort.MaxX = rt.right;
	m_ViewPort.MinY = rt.top;
	m_ViewPort.MaxY = rt.bottom;
	double iScreenW = m_ViewPort.MaxX - m_ViewPort.MinX;
	double iScreenH = m_ViewPort.MaxY - m_ViewPort.MinY;
	double GroundW = enve.MaxX - enve.MinX;
	double GroundH = enve.MaxY - enve.MinY;
	double zoomx = GroundW / iScreenW;
	double zoomy = GroundH / iScreenH;
	double maxzoom = zoomx > zoomy ? zoomx : zoomy;
	double xrange = maxzoom * iScreenW;
	double yrange = maxzoom * iScreenH;
	double xoffset = (xrange - GroundW) / 2.0;
	double yoffset = (yrange - GroundH) / 2.0;
	winenve.MinX = enve.MinX - xoffset;
	winenve.MinY = enve.MinY - yoffset;
	winenve.MaxX = winenve.MinX + xrange;
	winenve.MaxY = winenve.MinY + yrange;
	ResetEnvelope(winenve);
}

void ImageView::ResetEnvelope(const OGREnvelope& enve)
{
	if (!m_pReader) return;
	m_GroundRange = enve;
	CRect rt; GetClientRect(&rt);
	m_ViewPort.MinX = rt.left;
	m_ViewPort.MaxX = rt.right;
	m_ViewPort.MinY = rt.top;
	m_ViewPort.MaxY = rt.bottom;
	m_pReader->SetCurWnd(m_GroundRange, m_ViewPort);
	PostMessage(WM_PAINT);
}


void ImageView::Client2Ground(int screenx, int screeny, double& groundx, double& groundy)
{
	CRect rt;
	GetClientRect(&rt);
	groundx = (screenx / (double)rt.Width()) * (m_GroundRange.MaxX - m_GroundRange.MinX) + m_GroundRange.MinX;
	groundy = ((rt.Height() - screeny) / ((double)rt.Height())) * (m_GroundRange.MaxY - m_GroundRange.MinY) + m_GroundRange.MinY;
}

void ImageView::Ground2Client(double groundx, double groundy, int& screenx, int& screeny)
{

}

void ImageView::OnLButtonDown(UINT nFlags, CPoint point)
{
	GetCursorPos(&point);
	ScreenToClient(&point);
	OGRPoint pt; double x, y; Client2Ground(point.x, point.y, x, y); pt.setX(x); pt.setY(y);
	CView::OnLButtonDown(nFlags, point);
}

void ImageView::OnLButtonUp(UINT nFlags, CPoint point)
{

	CView::OnLButtonUp(nFlags, point);
}

void ImageView::OnRButtonDown(UINT nFlags, CPoint point)
{
	GetCursorPos(&point); ScreenToClient(&point);
	OGRPoint pt; double x, y; Client2Ground(point.x, point.y, x, y); pt.setX(x); pt.setY(y);
	CView::OnRButtonDown(nFlags, point);
}

void ImageView::OnMButtonDown(UINT nFlags, CPoint point)
{
	SetCapture();
	m_MButtonPos = point;
	CView::OnMButtonDown(nFlags, point);
}

void ImageView::OnMButtonUp(UINT nFlags, CPoint point)
{
	ReleaseCapture();
	CView::OnMButtonUp(nFlags, point);
}

void ImageView::OnMouseMove(UINT nFlags, CPoint point)
{
	GetCursorPos(&point); ScreenToClient(&point);
	OGRPoint pt;
	double x, y; Client2Ground(point.x, point.y, x, y); pt.setX(x); pt.setY(y);
	if (nFlags == MK_MBUTTON)
	{
		double dx = (point.x - m_MButtonPos.x);
		double dy = -(point.y - m_MButtonPos.y);
		double zr = (m_GroundRange.MaxX - m_GroundRange.MinX) / (m_ViewPort.MaxX - m_ViewPort.MinX);
		double dxrange = dx * zr;
		double dyrange = dy * zr;
		m_GroundRange.MaxX -= dxrange;
		m_GroundRange.MinX -= dxrange;
		m_GroundRange.MaxY -= dyrange;
		m_GroundRange.MinY -= dyrange;
		m_MButtonPos = point;
		ResetEnvelope(m_GroundRange);
		CWnd* pFrm = AfxGetMainWnd(); //向主框架发送窗口同步消息
		if (pFrm) pFrm->SendMessage(WM_SYNC_DISPLAY, (WPARAM) this);
	}
	PostMessage(WM_PAINT);
	CView::OnMouseMove(nFlags, point);
}

BOOL ImageView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	int d = (zDelta > 0 ? 1 : -1);
	GetCursorPos(&pt); ScreenToClient(&pt);
	double groundx, groundy;
	Client2Ground(pt.x, pt.y, groundx, groundy);
	OGREnvelope enve;
	if (d > 0)
	{
		double lx = (groundx - m_GroundRange.MinX) / 1.1;
		double rx = (m_GroundRange.MaxX - groundx) / 1.1;
		double by = (groundy - m_GroundRange.MinY) / 1.1;
		double ty = (m_GroundRange.MaxY - groundy) / 1.1;
		enve.MinX = groundx - lx; enve.MaxX = groundx + rx;
		enve.MinY = groundy - by; enve.MaxY = groundy + ty;
	}
	else if (d < 0)
	{
		double lx = (groundx - m_GroundRange.MinX) * 1.1;
		double rx = (m_GroundRange.MaxX - groundx) * 1.1;
		double by = (groundy - m_GroundRange.MinY) * 1.1;
		double ty = (m_GroundRange.MaxY - groundy) * 1.1;
		enve.MinX = groundx - lx; enve.MaxX = groundx + rx;
		enve.MinY = groundy - by; enve.MaxY = groundy + ty;
	}
	ResetEnvelope(enve);
	CWnd* pFrm = AfxGetMainWnd(); //向主框架发送窗口同步消息
	if (pFrm) pFrm->SendMessage(WM_SYNC_DISPLAY, (WPARAM) this);
	return CView::OnMouseWheel(nFlags, zDelta, pt);
}

void ImageView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);
	CRect rt;

	OGREnvelope oldViewPort = m_ViewPort;
	GetClientRect(&rt);
	m_ViewPort.MinX = 0.0;
	m_ViewPort.MaxX = rt.Width();
	m_ViewPort.MinY = 0.0;
	m_ViewPort.MaxY = rt.Height();

	if (oldViewPort.IsInit())
	{
		double zoomx = (m_ViewPort.MaxX - m_ViewPort.MinX) / (double)(oldViewPort.MaxX - oldViewPort.MinX);
		double zoomy = (m_ViewPort.MaxY - m_ViewPort.MinY) / (double)(oldViewPort.MaxY - oldViewPort.MinY);

		double centerx = (m_GroundRange.MinX + m_GroundRange.MaxX) / 2.0;
		double centery = (m_GroundRange.MinY + m_GroundRange.MaxY) / 2.0;

		m_GroundRange.MaxX = (m_GroundRange.MaxX - centerx) * zoomx + centerx;
		m_GroundRange.MinX = centerx - (centerx - m_GroundRange.MinX) * zoomx;
		m_GroundRange.MaxY = (m_GroundRange.MaxY - centery) * zoomy + centery;
		m_GroundRange.MinY = centery - (centery - m_GroundRange.MinY) * zoomy;
	}
}

void ImageView::ZoomIn()
{
	OGREnvelope enve = m_GroundRange;
	double centx = (enve.MinX + enve.MaxX) / 2.0, centy = (enve.MinY + enve.MaxY) / 2.0;
	double lx = (centx - m_GroundRange.MinX) * 0.95;
	double rx = (m_GroundRange.MaxX - centx) * 0.95;
	double by = (centy - m_GroundRange.MinY) * 0.95;
	double ty = (m_GroundRange.MaxY - centy) * 0.95;
	enve.MinX = centx - lx; enve.MaxX = centx + rx; enve.MinY = centy - by; enve.MaxY = centy + ty;
	ResetEnvelope(enve);
}

void ImageView::Zoomout()
{
	OGREnvelope enve = m_GroundRange;
	double centx = (enve.MinX + enve.MaxX) / 2.0, centy = (enve.MinY + enve.MaxY) / 2.0;
	double lx = (centx - m_GroundRange.MinX) * 1.05;
	double rx = (m_GroundRange.MaxX - centx) * 1.05;
	double by = (centy - m_GroundRange.MinY) * 1.05;
	double ty = (m_GroundRange.MaxY - centy) * 1.05;
	enve.MinX = centx - lx; enve.MaxX = centx + rx; enve.MinY = centy - by; enve.MaxY = centy + ty;
	ResetEnvelope(enve);
}

void ImageView::ZoomFit()
{

}

void ImageView::ZoomOrigin()
{

}

void ImageView::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == FORECAST_TIMER_ID)
		PostMessage(WM_PAINT);
	CView::OnTimer(nIDEvent);
}