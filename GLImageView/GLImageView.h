#ifndef _GLIMAGEVIEW_20171109_H_
#define _GLIMAGEVIEW_20171109_H_

#include <vector>
#include <string>
#include "GLImageReader/GLImageReader.h"

#ifdef GLIMAGEVIEW
#define GLIMAGEVIEWAPI _declspec(dllexport)
#else
#define GLIMAGEVIEWAPI _declspec(dllimport)
#pragma comment(lib, "GLImageView.lib")
#pragma message("Automatically Linking With GLImageView.dll")
#endif

using namespace std;
#define WM_SYNC_DISPLAY WM_USER + 1001 //向主窗口发出同步窗口消息
/************************************************************************/
/*							   ImageView                                */
/************************************************************************/
class GLIMAGEVIEWAPI ImageView : public CView
{
public:
	DECLARE_DYNCREATE(ImageView)
	DECLARE_MESSAGE_MAP()
public:
	ImageView();
	~ImageView();

	void	ZoomIn();
	void	Zoomout();
	void	ZoomFit();
	void	ZoomOrigin();
	void	InitialEnvelope(const OGREnvelope& enve);
	void	ResetEnvelope(const OGREnvelope& enve);
	void	AttachReader(ImageReader* pReader) { m_pReader = pReader; }
	HGLRC  GetRC();
	HGLRC  GetShareRC();
	void   SetRC(HGLRC rc);
	OGREnvelope& GetCurrentGroundRange() { return m_GroundRange; }
protected:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	virtual void OnInitialUpdate();
	virtual void OnDraw(CDC* pDC);

protected:
	virtual  void   BeginDraw();
	virtual  void	Draw();
	virtual  void   DrawTexture();
	virtual  void   DrawGeometry();
	virtual  void   DrawImageGrids();
	virtual  void   DrawImageFrame();
	virtual  void   EndDraw();
public:
	void	Client2Ground(int screenx, int screeny, double& groundx, double& groundy);
	void	Ground2Client(double groundx, double groundy, int& screenx, int& screeny);

public:

	OGREnvelope m_GroundRange;
	OGREnvelope m_ViewPort;
	CPoint		m_MButtonPos;
	GLfloat		m_SeamLineColor[4];
	GLuint		m_MVMatrixUniformID;
	GLuint		m_GeometryColorUniformID;
	GLuint		m_bTextureUniformID;
	GLuint		m_MainShader;
	/*-----渲染环境-----*/
	HDC         m_hDC;
	HGLRC       m_hRC;
	HGLRC       m_hRCShare;
	/*-----VAO-----*/
	GLuint		m_VAOID;
	bool		m_bStop;
	/*-----窗口局部放大-----*/
	GLuint	   m_ZoomRCVBO;
	bool	   m_bStartDrag;
	OGRPoint   m_StartPoint;
	OGRPoint   m_EndPoint;

	/************************************************************************/
	/*						    绑定的影像读取器                             */
	/************************************************************************/
	ImageReader *m_pReader;
};

#endif