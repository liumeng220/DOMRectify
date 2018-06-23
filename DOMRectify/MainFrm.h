// ��� MFC ʾ��Դ������ʾ���ʹ�� MFC Microsoft Office Fluent �û����� 
// (��Fluent UI��)����ʾ�������ο���
// ���Բ��䡶Microsoft ������ο����� 
// MFC C++ ������渽����ص����ĵ���
// ���ơ�ʹ�û�ַ� Fluent UI ����������ǵ����ṩ�ġ�
// ��Ҫ�˽��й� Fluent UI ��ɼƻ�����ϸ��Ϣ�������  
// http://msdn.microsoft.com/officeui��
//
// ��Ȩ����(C) Microsoft Corporation
// ��������Ȩ����

// MainFrm.h : CMainFrame ��Ľӿ�
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

	// ����
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
// ����
public:

// ��д
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// ʵ��
public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

public:  // �ؼ���Ƕ���Ա
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
// ���ɵ���Ϣӳ�亯��
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


