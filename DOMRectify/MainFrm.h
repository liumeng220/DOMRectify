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

// MainFrm.h : CMainFrame 类的接口
//

#pragma once
#include "OutputWnd.h"
#include "PropertiesWnd.h"
#include "Resource.h"
#include "MyMFCRibbonBar.h"
#include "MyDockablePane.h"
#include "MyDockPaneWndData.h"
#include "DOMRectifyPrj.h"
#include "GradientProgressCtrl.h"
#include "DomRectifyPro.h"
class CChildFrame;
class CDlgPrjNew;
class CDlgPrjNew2;
class CDOMRectifyView;


class COutlookBar : public CMFCOutlookBar
{
	virtual BOOL AllowShowOnPaneMenu() const { return TRUE; }
	virtual void GetPaneName(CString& strName) const { BOOL bNameValid = strName.LoadString(IDS_OUTLOOKBAR); ASSERT(bNameValid); if (!bNameValid) strName.Empty(); }
};

class CMainFrame : public CMDIFrameWndEx
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();
	virtual ~CMainFrame();
	ImageReader *m_pReaderMain;
	ImageReader *m_pReaderStere;
	ImageReader *m_pReaderRef;
	DomRectifyPro m_RectifyHander;
	bool m_bAutoSelectRef;
	bool m_bViewPoint;
	bool m_bAutoRectify;
	void ReadImage(CString strImagePath, int stcol, int strow, int edcol, int edrow, int memWidth, int memHeight, BYTE*&data);
	void SaveImage(CString strImagePath, int nCols, int nRows, int nBands, BYTE*data, const char*pszFormat);

	// 特性
public:
	BOOL CreateDockingWindows();
	void AddToMatchList();
	void AutoMatch(CString strPath);
	void AutoRectifyOnTIme();

	void NewPrj(CDlgPrjNew2 &dlg);
	void OpenPrj();
	void InitRefCombo();
	void UpdateViewer();
	void LoadImageGrids();
	void LoadImageTexts();
	void LoadMatchPoint();
	void ImageGridEnve(map<int, OGREnvelope>& imgGridEnve);
	void ClearData();
	void ClearAllReader();
	void ClearMainReader();
	void ClearStereReader();
	void CloseStereDoc();
	void Switch2StereDoc();
	CString m_strPrjPath;
	CDOMRectifyPrj m_Project;;
	CDOMRectifyView* m_pMainView;
	CDOMRectifyView* m_pLView;
	CDOMRectifyView* m_pRView;
	GLuint m_LPointsVBO;
	GLuint m_RPointsVBO;
	GLuint m_LPointsVBOAdd;
	GLuint m_RPointsVBOAdd;
	GLuint m_ImgGridsVBO;
	map<int, OGREnvelope> m_ImgGridEnves;

	eDOMTYPE m_eOpenDOMType;
	int m_nImgIdxInMatchView;
// 操作
public:

// 重写
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// 实现
public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

public:  // 控件条嵌入成员
	CMyMFCRibbonBar     m_wndRibbonBar;
	CMFCRibbonApplicationButton m_MainButton;
	CMFCToolBarImages m_PanelImages;
	CMFCRibbonStatusBar  m_wndStatusBar;
	COutputWnd        m_wndOutput;
	CPropertiesWnd    m_wndProperties;
	CMyDockPaneWndData m_wndImgData;

	CMFCRibbonStatusBarPane *m_pStatusPaneInfo;
	CMFCRibbonStatusBarPane *m_pStatusPaneProcessing;
	CMFCRibbonStatusBarPane *m_pStatusPaneSysTime;
	CGradientProgressCtrl m_wndProgress;
	CGradientProgressCtrl m_wndProgressPyramid;
// 生成的消息映射函数
protected:
	afx_msg LRESULT SyncDisplay(WPARAM wPara, LPARAM lPara);
	afx_msg LRESULT OnOpenMatchView(WPARAM wPara, LPARAM lPara);
	afx_msg LRESULT OnProgressVisible(WPARAM wPara, LPARAM lPara);
	afx_msg LRESULT OnProgressPosition(WPARAM wPara, LPARAM lPara);
	afx_msg LRESULT OnCreateDockpane(WPARAM wPara, LPARAM lPara);
	afx_msg LRESULT OnUpdateViewer(WPARAM wPara, LPARAM lPara);
	afx_msg LRESULT OnUpdateStatusInfo(WPARAM wPara, LPARAM lPara);
	afx_msg LRESULT OnNewProject(WPARAM wPara, LPARAM lPara);
	afx_msg LRESULT OnUpdateMatchView(WPARAM wPara, LPARAM lPara);
	afx_msg LRESULT OnUpdateOrthoImage(WPARAM wPara, LPARAM lPara);
	afx_msg LRESULT OnAddMatchPoint(WPARAM wPara, LPARAM lPara);
	afx_msg LRESULT OnDelMatchPoint(WPARAM wPara, LPARAM lPara);

	afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnWindowManager();
	afx_msg void OnApplicationLook(UINT id);
	afx_msg void OnUpdateApplicationLook(CCmdUI* pCmdUI);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	DECLARE_MESSAGE_MAP()

	void SetDockingWindowIcons(BOOL bHiColorIcons);
	void OpenMatchView();
	eDOMTYPE HighlightMatchViewImage();

public:
	afx_msg void OnFileNew();
//	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	afx_msg void OnFileOpen();
	afx_msg void OnButtonPrjClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);


	afx_msg void OnCheckAutoSelectRef();
	afx_msg void OnUpdateCheckAutoSelectRef(CCmdUI *pCmdUI);
	afx_msg void OnComboSelectRef();
	afx_msg void OnUpdateComboSelectRef(CCmdUI *pCmdUI);
	afx_msg void OnCheckViewPoint();
	afx_msg void OnUpdateCheckViewPoint(CCmdUI *pCmdUI);
	afx_msg void OnCheckAutoRectifyAfterMatch();
	afx_msg void OnUpdateCheckAutoRectifyAfterMatch(CCmdUI *pCmdUI);
};


