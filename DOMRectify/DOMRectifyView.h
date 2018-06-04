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

// DOMRectifyView.h : CDOMRectifyView 类的接口
//

#pragma once
#include "GLImageView/GLImageView.h"
#include "GLImageReader/GLImageReader.h"

inline void selectFont(int size, int charset, const char* face) {
	HFONT hFont = CreateFontA(size, 0, 0, 0, FW_MEDIUM, 0, 0, 0,
		charset, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, face);
	HFONT hOldFont = (HFONT)SelectObject(wglGetCurrentDC(), hFont);
	DeleteObject(hOldFont);
}

class CDOMRectifyView : public ImageView
{
protected: // 仅从序列化创建
	CDOMRectifyView();
	DECLARE_DYNCREATE(CDOMRectifyView)

// 特性
public:
	//CDOMRectifyDoc* GetDocument() const;
	enum CHECK_ORIENTATION
	{
		LEFT_RIGHT,
		BOTTOM_UP,
		NONE
	};
	bool m_bStartSelect;
	bool m_bStartWrapper;
	CHECK_ORIENTATION m_CheckOri;
	GLuint m_OrinVBO;
	GLuint m_WrapVBO;
// 操作
public:

	OGREnvelope m_EnveSelectBox;
// 实现
public:
	virtual ~CDOMRectifyView();
	DECLARE_MESSAGE_MAP()
public:
	void SyncDisplay(const OGREnvelope& enve);
	int GetCurImage();
	void DrawInsideImageFrame();
	void DrawMatchImageFrame();
	void DrawOverlapBox();
	void DrawSelectBox();
	void DrawSelectPoint();
	void DrawImageNames();
	virtual void DrawTexture();
	virtual void DrawGeometry();
	virtual void DrawImageGrids();
	virtual void DrawImageFrame();
	
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual void OnInitialUpdate();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);

	afx_msg void OnButtonPointAdd();
	afx_msg void OnUpdateButtonPointAdd(CCmdUI *pCmdUI);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnButtonPointDelete();
	afx_msg void OnUpdateButtonPointDelete(CCmdUI *pCmdUI);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnButtonWrapperCheck();
	afx_msg void OnUpdateButtonWrapperCheck(CCmdUI *pCmdUI);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};

