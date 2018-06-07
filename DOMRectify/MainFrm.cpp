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

// MainFrm.cpp : CMainFrame ���ʵ��
//

#include "stdafx.h"
#include "DOMRectify.h"

#include "MainFrm.h"
#include "DlgPrjNew.h"
#include "DlgPrjNew2.h"
#include "ChildFrm.h"
#include "DOMRectifyView.h"
#include "MyMemTxtFile.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame
OGREnvelope g_curDomEnve;

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWndEx)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWndEx)
	ON_WM_CREATE()
	ON_COMMAND(ID_WINDOW_MANAGER, &CMainFrame::OnWindowManager)
	ON_COMMAND_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7, &CMainFrame::OnApplicationLook)
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7, &CMainFrame::OnUpdateApplicationLook)
	ON_WM_SETTINGCHANGE()
	ON_COMMAND(ID_FILE_NEW, &CMainFrame::OnFileNew)
	ON_MESSAGE(WM_SYNC_DISPLAY, &CMainFrame::SyncDisplay)
	ON_MESSAGE(WM_OPEN_MATCH_VIEW, &CMainFrame::OnOpenMatchView)
	ON_MESSAGE(WM_WND_PROGRESS_SHOW, &CMainFrame::OnProgressVisible)
	ON_MESSAGE(WM_WND_PROGRESS_POS, &CMainFrame::OnProgressPosition)
	ON_MESSAGE(WM_CREATE_DOCKPANE, &CMainFrame::OnCreateDockpane)
	ON_MESSAGE(WM_UPDATE_VIEWER, &CMainFrame::OnUpdateViewer)
	ON_MESSAGE(WM_UPDATE_STATUSINFO, &CMainFrame::OnUpdateStatusInfo)
	ON_MESSAGE(WM_NEW_PROJECT, &CMainFrame::OnNewProject)
	ON_MESSAGE(WM_UPDATE_MATCHVIEW, &CMainFrame::OnUpdateMatchView)
	ON_MESSAGE(WM_ADD_MATCHPOINT, &CMainFrame::OnAddMatchPoint)
	ON_MESSAGE(WM_DELETE_MATCHPOINT, &CMainFrame::OnDelMatchPoint)
	ON_MESSAGE(WM_UPDATE_ORTHORIMAGE, &CMainFrame::OnUpdateOrthoImage)

	ON_COMMAND(ID_FILE_OPEN, &CMainFrame::OnFileOpen)
	ON_COMMAND(ID_BUTTON_PRJ_CLOSE, &CMainFrame::OnButtonPrjClose)
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_COMMAND(ID_CHECK_AUTO_SELECT_REF, &CMainFrame::OnCheckAutoSelectRef)
	ON_UPDATE_COMMAND_UI(ID_CHECK_AUTO_SELECT_REF, &CMainFrame::OnUpdateCheckAutoSelectRef)
	ON_COMMAND(ID_COMBO_SELECT_REF, &CMainFrame::OnComboSelectRef)
	ON_UPDATE_COMMAND_UI(ID_COMBO_SELECT_REF, &CMainFrame::OnUpdateComboSelectRef)

	ON_COMMAND(ID_CHECK_VIEW_POINT, &CMainFrame::OnCheckViewPoint)
	ON_UPDATE_COMMAND_UI(ID_CHECK_VIEW_POINT, &CMainFrame::OnUpdateCheckViewPoint)
END_MESSAGE_MAP()

// CMainFrame ����/����

CMainFrame::CMainFrame()
{
	// TODO: �ڴ���ӳ�Ա��ʼ������
	theApp.m_nAppLook = theApp.GetInt(_T("ApplicationLook"), ID_VIEW_APPLOOK_OFF_2007_BLACK);
	m_pMainView = NULL;
	m_pLView = NULL;
	m_pRView = NULL;
	m_pReaderMain = NULL;
	m_pReaderStere = NULL;
	m_pReaderRef = NULL;
	m_LPointsVBO = 0;
	m_RPointsVBO = 0;
	m_LPointsVBOAdd = 0;
	m_RPointsVBOAdd = 0;
	m_ImgGridsVBO = 0;
	m_nImgIdxInMatchView = -1;
	m_eOpenDOMType = PAN;
	ClearData();

	m_bViewPoint = true;


}


CMainFrame::~CMainFrame()
{
	ClearData();
}

void CMainFrame::ReadImage(CString strImagePath, int stcol, int strow, int edcol, int edrow, int memWidth, int memHeight, BYTE*&data)
{
	OrthoImage *pImg = new OrthoImage;
	pImg->LoadDOM(strImagePath.GetBuffer(),"");
	pImg->ReadImage(strImagePath, stcol, strow, edcol, edrow, memWidth, memHeight, data);
}
void CMainFrame::SaveImage(CString strImagePath, int nCols, int nRows, int nBands, BYTE*data, const char*pszFormat)
{
	if (strcmp(pszFormat, "GTiff") == 0)
	{
		GDALDriver *poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);
		if (poDriver == NULL) return ;
		GDALDataset *WriteDataSet = poDriver->Create(strImagePath, nCols, nRows, nBands, GDT_Byte, NULL);
		int *pBandMap = new int[nBands];
		for (int i = 0; i < nBands; i++)
			pBandMap[i] = i + 1;
		if (WriteDataSet->RasterIO(GF_Write, 0, 0, nCols, nRows, data, nCols, nRows, GDT_Byte, nBands, NULL, nBands * 1, nCols*nBands * 1, 1) == CE_Failure)
		{
			return ;
		}
		GDALClose(poDriver);
		GDALClose(WriteDataSet); WriteDataSet = NULL;
		delete[]pBandMap;
		return;
	}
	else
	{
		if (strImagePath.IsEmpty() || data == NULL || nCols < 1 || nRows < 1 || nBands < 1)
		{
			return ;
		}

		GDALDriver *pMemDriver = NULL;
		pMemDriver = GetGDALDriverManager()->GetDriverByName("MEM");
		if (pMemDriver == NULL) { return ; }

		GDALDataset * pMemDataSet = pMemDriver->Create("", nCols, nRows,nBands, GDT_Byte, NULL);
		GDALRasterBand *pBand = NULL;
		int nLineCount = nCols * nBands;
		int *pBandMap = new int[nBands];
		for (int i = 0; i < nBands; i++)
			pBandMap[i] = i + 1;
		CPLErr err = pMemDataSet->RasterIO(GF_Write, 0, 0, nCols, nRows, data, nCols, nRows, GDT_Byte, nBands, pBandMap, nBands * 1, nLineCount, 1);
		if (err == CE_Failure) return ;
		GDALDriver *pDstDriver = NULL;
		pDstDriver = (GDALDriver *)GDALGetDriverByName("jpg");
		if (pDstDriver == NULL) { return ; }
		pDstDriver->CreateCopy(strImagePath, pMemDataSet, FALSE, NULL, NULL, NULL);
		GDALClose(pMemDataSet);
		delete[]pBandMap;
		return;
	}
}

void CMainFrame::ClearData()
{
	m_Project.ClearPrj();
	if (m_wndImgData.GetSafeHwnd())
	{
		m_wndImgData.ClearImgTree();
		m_wndImgData.ClearMatchList();
	}
	m_ImgGridEnves.clear();
	ClearAllReader();
	m_strPrjPath.Empty();
	m_bAutoSelectRef = true;
	// 	if (glIsBuffer(m_ImgGridsVBO))   glDeleteBuffers(1, &m_ImgGridsVBO);
	// 	if (glIsBuffer(m_LPointsVBO))    glDeleteBuffers(1, &m_LPointsVBO);
	// 	if (glIsBuffer(m_RPointsVBO))    glDeleteBuffers(1, &m_RPointsVBO);
	// 	if (glIsBuffer(m_LPointsVBOAdd)) glDeleteBuffers(1, &m_LPointsVBOAdd);
	// 	if (glIsBuffer(m_RPointsVBOAdd)) glDeleteBuffers(1, &m_RPointsVBOAdd);
	// 	if (glIsBuffer(m_SelectBoxVBO))  glDeleteBuffers(1, &m_SelectBoxVBO);
}

void CMainFrame::ClearAllReader()
{
	ClearMainReader();
	ClearStereReader();
}

void CMainFrame::ClearMainReader()
{
	if (m_pReaderMain)
	{
		m_pReaderMain->ClearData();
		m_pReaderMain = NULL;
	}
}

void CMainFrame::ClearStereReader()
{
	if (m_pReaderStere)
	{
//		m_Project.SavePoint(m_pReaderRef);
		m_Project.ClearPoint();
		m_pReaderStere->ClearData();
		m_pReaderStere = NULL;
	}
}

void CMainFrame::CloseStereDoc()
{
	ClearStereReader();
	POSITION pos = theApp.GetFirstDocTemplatePosition();
	if (pos)
	{
		CDocTemplate* pDocTemp = theApp.GetNextDocTemplate(pos);
		if (pDocTemp)
		{
			POSITION dPos = pDocTemp->GetFirstDocPosition();
			while (dPos)
			{
				CDOMRectifyDoc*pDoc = (CDOMRectifyDoc*)pDocTemp->GetNextDoc(dPos);
				CString strTitle = pDoc->GetTitle();
				if (strTitle == "ƥ���༭")
				{
					m_pLView = NULL;
					m_pRView = NULL;
					pDoc->OnCloseDocument();
				}
			}
		}
	}
	m_nImgIdxInMatchView = -1;
	HighlightMatchViewImage();
}

void CMainFrame::Switch2StereDoc()
{
	if (m_pLView)
	{
		m_pLView->GetParentFrame()->ActivateFrame();
	}
}




int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	BOOL bNameValid;
	// ���ڳ־�ֵ�����Ӿ�����������ʽ
	OnApplicationLook(theApp.m_nAppLook);

	CMDITabInfo mdiTabParams;
	mdiTabParams.m_style = CMFCTabCtrl::STYLE_3D_ONENOTE; // ����������ʽ...
	mdiTabParams.m_bActiveTabCloseButton = TRUE;      // ����Ϊ FALSE �Ὣ�رհ�ť������ѡ�������Ҳ�
	mdiTabParams.m_bTabIcons = FALSE;    // ����Ϊ TRUE ���� MDI ѡ��������ĵ�ͼ��
	mdiTabParams.m_bAutoColor = TRUE;    // ����Ϊ FALSE ������ MDI ѡ����Զ���ɫ
	mdiTabParams.m_bDocumentMenu = TRUE; // ��ѡ�������ұ�Ե�����ĵ��˵�
	EnableMDITabbedGroups(TRUE, mdiTabParams);

	m_wndRibbonBar.Create(this);
	m_wndRibbonBar.LoadFromResource(IDR_RIBBON);

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("δ�ܴ���״̬��\n");
		return -1;      // δ�ܴ���
	}

	m_wndStatusBar.AddElement(new CMFCRibbonStatusBarPane(ID_STATUSBAR_PANE1, "", TRUE), "");
	m_wndStatusBar.AddElement(new CMFCRibbonStatusBarPane(ID_STATUSBAR_PANE2, "", TRUE), "");
	m_wndStatusBar.AddExtendedElement(new CMFCRibbonStatusBarPane(ID_STATUSBAR_PANE1, FunGetSysTimeStr(), TRUE), FunGetSysTimeStr());

	m_pStatusPaneInfo = (CMFCRibbonStatusBarPane*)m_wndStatusBar.GetElement(0);
	m_pStatusPaneProcessing = (CMFCRibbonStatusBarPane*)m_wndStatusBar.GetElement(1);
	m_pStatusPaneSysTime = (CMFCRibbonStatusBarPane*)m_wndStatusBar.GetExElement(0);


	m_pStatusPaneInfo->SetAlmostLargeText("00000000000000000000000000000000000000000000");
	m_pStatusPaneProcessing->SetAlmostLargeText("00000000000000000000000000000000000");
	m_pStatusPaneSysTime->SetAlmostLargeText("0000-00-00  00-00-00");

	m_pStatusPaneProcessing->SetTextAlign(TA_RIGHT);
	// ���� Visual Studio 2005 ��ʽͣ��������Ϊ
	CDockingManager::SetDockingMode(DT_SMART);
	// ���� Visual Studio 2005 ��ʽͣ�������Զ�������Ϊ
	EnableAutoHidePanes(CBRS_ALIGN_ANY);

	// �������񽫴�������࣬��˽���ʱ��������ͣ��:
	EnableDocking(CBRS_ALIGN_TOP | CBRS_ALIGN_BOTTOM | CBRS_ALIGN_RIGHT);


	// �Ѵ��� Outlook ����Ӧ���������ͣ����
	EnableDocking(CBRS_ALIGN_LEFT);
	EnableAutoHidePanes(CBRS_ALIGN_RIGHT);
	// ������ǿ�Ĵ��ڹ���Ի���
	EnableWindowsDialog(ID_WINDOW_MANAGER, ID_WINDOW_MANAGER, TRUE);

	// ���ĵ�����Ӧ�ó��������ڴ��ڱ������ϵ�˳����н�������
	// ���Ľ��������Ŀ����ԣ���Ϊ��ʾ���ĵ�����������ͼ��
	ModifyStyle(0, FWS_PREFIXTITLE);

	CRect RectStatus, RectSystemTime, RectProgress, RectProgressPyramid;
	m_wndStatusBar.GetClientRect(RectStatus);
	RectSystemTime = m_pStatusPaneSysTime->GetRect();
	RectProgress.right = max(RectStatus.left + 5, RectStatus.right - RectSystemTime.Width() - 3);
	RectProgress.left = max(RectProgress.right - 200, RectStatus.left + 1);
	RectProgress.top = RectSystemTime.top;
	RectProgress.bottom = RectSystemTime.bottom;

	RectProgressPyramid = RectProgress;
	RectProgressPyramid.right = RectProgress.left;
	RectProgressPyramid.left = max(RectProgressPyramid.right - 150, RectStatus.left + 1);

	if (!m_wndProgress.Create(WS_VISIBLE | WS_CHILD | PBS_SMOOTH, RectProgress, &m_wndStatusBar, 1) ||
		!m_wndProgressPyramid.Create(WS_VISIBLE | WS_CHILD | PBS_SMOOTH, RectProgressPyramid, &m_wndStatusBar, 2))
	{
		TRACE0("Failed to create windows progressctrl\n");
		return -1;
	}

	m_wndProgress.ShowWindow(false);
	m_wndProgress.SetStartColor(RGB(255, 0, 0));
	m_wndProgress.SetEndColor(RGB(30, 144, 255));
	m_wndProgress.SetTextColor(RGB(255, 0, 0));
	m_wndProgress.SetRange(0, 100);
	m_wndProgress.SetPos(0);

	m_wndProgressPyramid.ShowWindow(false);
	m_wndProgressPyramid.SetStartColor(RGB(255, 0, 0));
	m_wndProgressPyramid.SetEndColor(RGB(30, 144, 255));
	m_wndProgressPyramid.SetTextColor(RGB(255, 0, 0));
	m_wndProgressPyramid.SetRange(0, 100);
	m_wndProgressPyramid.SetPos(0);


	SetTimer(WM_SYSTEM_TIME, 500, NULL);
	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CMDIFrameWndEx::PreCreateWindow(cs))
		return FALSE;
	// TODO: �ڴ˴�ͨ���޸�
	//  CREATESTRUCT cs ���޸Ĵ��������ʽ
	return TRUE;
}

BOOL CMainFrame::CreateDockingWindows()
{
	BOOL bNameValid;
	// �����������
	CString strOutputWnd;
	bNameValid = strOutputWnd.LoadString(IDS_OUTPUT_WND);
	ASSERT(bNameValid);
	if (!m_wndOutput.Create(strOutputWnd, this, CRect(0, 0, 100, 100), TRUE, ID_VIEW_OUTPUTWND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_BOTTOM | CBRS_FLOAT_MULTI))
	{
		TRACE0("δ�ܴ����������\n");
		return FALSE; // δ�ܴ���
	}
	DWORD dwNoCloseBarStyle = AFX_DEFAULT_DOCKING_PANE_STYLE & ~AFX_CBRS_CLOSE;  //���عرհ�ť
	if (!m_wndImgData.Create("ͼ���б�", this, CRect(0, 0, 300, 100), TRUE, ID_VIEW_IMGDATAWND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI, AFX_CBRS_REGULAR_TABS, dwNoCloseBarStyle&~AFX_CBRS_RESIZE &~AFX_CBRS_AUTOHIDE))
	{
		TRACE0("δ�ܴ���ͼ���б���\n");
		return false; // δ�ܴ���
	}
	// �������Դ���
	//if (!m_wndProperties.Create("����", this, CRect(0, 0, 300, 100), TRUE, ID_VIEW_PROPERTIESWND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI,AFX_CBRS_REGULAR_TABS,dwNoCloseBarStyle&~AFX_CBRS_RESIZE &~ AFX_CBRS_AUTOHIDE))
	//{
	//	TRACE0("δ�ܴ��������ԡ�����\n");
	//	return false; // δ�ܴ���
	//}
	// 	m_wndImgData.EnableDocking(CBRS_ALIGN_ANY);
	//m_wndProperties.EnableDocking(CBRS_ALIGN_ANY);
	m_wndOutput.EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndImgData);
	//DockPane(&m_wndProperties);
	DockPane(&m_wndOutput);
	//m_wndProperties.DockToWindow(&m_wndImgData, CBRS_BOTTOM);
	m_wndImgData.SetControlBarStyle(AFX_CBRS_RESIZE);   //���ô��ڲ����϶�
														//m_wndProperties.SetControlBarStyle(AFX_CBRS_RESIZE);
	SetDockingWindowIcons(theApp.m_bHiColorIcons);
	return TRUE;
}

void CMainFrame::AddToMatchList()
{
	m_wndImgData.AddToMatchList(m_Project.GetPanPath(m_nImgIdxInMatchView));
}

void CMainFrame::AutoMatch(CString strPath)
{
	//��ο�ͼƥ��
	CString strRunMatch, strMatchExe, strRef, strDom;
	strMatchExe = FunGetFileFolder(FunGetThisExePath()) + "\\match\\SIATMatch.exe";
	for (int i = 0;i <m_Project.GetRefNum(); i++)
	{
		strRef = m_Project.GetRefPath(i);
		strDom = strPath;
		strRunMatch.Format("%s %s %s", strMatchExe, strDom, strRef);
		system(strRunMatch);
		CString strMathchOri, strMatchRef, strMatchRefTmp;
		strMathchOri = FunGetFileFolder(strDom) + "\\" + FunGetFileName(strDom, false) + "_cppts.txt";
		strMatchRefTmp = FunGetFileFolder(strDom) + "\\" + FunGetFileName(strDom, false) + "_cppts";
		strMatchRef.Format("%s_%d.txt", strMatchRefTmp, i); DeleteFile(strMatchRef);
		CopyFile(strMathchOri, strMatchRef, FALSE);
	}
	
//����
	CString strMatch2 = FunGetFileFolder(m_strPrjPath) + "\\ImageMatch\\" + FunGetFileName(strDom, false) + "_match.txt";
	if (!PathFileExists(FunGetFileFolder(strMatch2))) FunCreateDir4Path(strMatch2, false);
	int nRefIdx = 0; nRefIdx = theApp.m_nRefIdx;
// 	for (int i = 0; i < m_Project.GetRefNum(); i++)
// 	{
// 		if (m_Project.GetRefPath(i) == strRef)
// 		{
// 			nRefIdx = i;
// 			break;
// 		}
// 	}
	FILE *pw = fopen(strMatch2, "w"); if (!pw) return;
	int nPtNum = 0;
	for (int i = 0; i < m_Project.GetRefNum(); i++)
	{
		CString  strMatchRef, strMatchRefTmp;
		strMatchRefTmp = FunGetFileFolder(strDom) + "\\" + FunGetFileName(strDom, false) + "_cppts";
		strMatchRef.Format("%s_%d.txt", strMatchRefTmp, i);
		FILE *pr = fopen(strMatchRef, "r"); if (!pr) continue;
		char line[1024]; memset(line, 0, 1024);
		fgets(line, 1024, pr); if (i == 0)fputs(line, pw);
		fgets(line, 1024, pr); if (i == 0)fputs("Ref_Image\n", pw);
		fgets(line, 1024, pr); if (i == 0)fputs(line, pw); nPtNum += atoi(line);
		while (fgets(line, 1024, pr))
		{
			line[strlen(line) - 1] = '\0';
			fprintf(pw, "%s %d\n", line, i);
		}
		fclose(pr);
	}
	fclose(pw);

	//��������ͼ��
	CString strRectifyFolder = FunGetFileFolder(m_strPrjPath) + "\\Rectify\\"; FunCreateDir4Path(strRectifyFolder, true);
	CString strRectifyImage = m_Project.GetCurRecDomPath();

	m_RectifyHander.LoadData(m_Project.GetCurDomPath(), m_Project.GetCurMatchPath(), m_Project.GetRefPath(), strRectifyImage);
	m_RectifyHander.rectifyDomImages();

	::SendMessage(GetMainFramHand()->m_hWnd, WM_UPDATE_ORTHORIMAGE, 0, 1);
	::SendMessage(GetMainFramHand()->m_hWnd, WM_UPDATE_MATCHVIEW, 0, 1);

}

void CMainFrame::AutoRectifyOnTIme()
{
	OrthoImage *DomImg = new OrthoImage;
	DomImg->LoadDOM(m_Project.GetCurDomPath().GetBuffer(), "");
	double GsdDom = DomImg->GetGSD();
	OGREnvelope enveDom = DomImg->GetGroundRange();
	DomImg->SetOffset(enveDom.MinX, enveDom.MinY);
	OGREnvelope enveDom2 = DomImg->GetGroundRange();
	OGREnvelope enveRef = m_pReaderRef->GetImage(theApp.m_nRefIdx)->GetGroundRange();
	double GsdRef = m_pReaderRef->GetImage(theApp.m_nRefIdx)->GetGSD();

	tagCorrespondPt cpt;
	sprintf(cpt.ID, "%d", m_Project.GetDomPoints()[m_Project.GetAllPointNum() - 1].nPtIdx);

	cpt.lx = (theApp.m_ptDom.getX() - enveDom2.MinX) / GsdDom;  //������껻�㵽��ƽ������ϵ
	cpt.ly = (theApp.m_ptDom.getY() - enveDom2.MinY) / GsdDom;
	cpt.rx = (theApp.m_ptRef.getX() - enveRef.MinX) / GsdRef;
	cpt.ry = (theApp.m_ptRef.getY() - enveRef.MinY) / GsdRef;;
	cpt.nrefid = theApp.m_nRefIdx;

	float fZoomRate = m_pReaderStere->GetCurZoomRate();
	int stCol, stRow, edCol, edRow; double left, top, right, bottom;

	CRect rectView; m_pLView->GetClientRect(rectView);
	m_pLView->Client2Ground(0, 0, left, top);
	m_pLView->Client2Ground(rectView.right, rectView.bottom, right, bottom);
	stCol = (left - enveDom2.MinX) / GsdDom;
	stRow = (bottom - enveDom2.MinY) / GsdDom;
	edCol = (right - enveDom2.MinX) / GsdDom;
	edRow = (top - enveDom2.MinY) / GsdDom;

	//��ȡӰ��Ҷ�-��������ɴ������ȡ
	BYTE *data=new BYTE[1],*data2=new BYTE[1];
	ReadImage(m_Project.GetCurDomPath(), stCol, stRow, edCol, edRow, rectView.Width(), rectView.Height(),data);
	SaveImage(m_strPrjPath + ".tif", rectView.Width(), rectView.Height(), 4, data, "GTiff");
	/***************************************/
	//�˴�����ʵʱ������غ���
	m_RectifyHander.addmatchpt(&cpt);
//	m_RectifyHander.smallareaTinyFacet(stRow, stCol, fZoomRate, rectView.Height(), rectView.Width(), data, 1, &data2);   //lkb������������⣿��
	/**************************************/
	//�˴��滻����
	m_pReaderStere->UpdateCurTex(data, stCol, stRow, edCol, edRow, fZoomRate);
//	m_pLView->PostMessage(WM_PAINT);
}

void CMainFrame::SetDockingWindowIcons(BOOL bHiColorIcons)
{
	HICON hOutputBarIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_OUTPUT_WND_HC : IDI_OUTPUT_WND), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_wndOutput.SetIcon(hOutputBarIcon, FALSE);

	HICON hPropertiesBarIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_PROPERTIES_WND_HC : IDI_PROPERTIES_WND), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	//	m_wndProperties.SetIcon(hPropertiesBarIcon, FALSE);

	UpdateMDITabbedBarsIcons();
}

// CMainFrame ���

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWndEx::AssertValid();
} 

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWndEx::Dump(dc);
}
#endif //_DEBUG


// CMainFrame ��Ϣ�������

void CMainFrame::OnWindowManager()
{
	ShowWindowsDialog();
}

void CMainFrame::OnApplicationLook(UINT id)
{
	CWaitCursor wait;

	theApp.m_nAppLook = id;

	switch (theApp.m_nAppLook)
	{
	case ID_VIEW_APPLOOK_WIN_2000:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManager));
		m_wndRibbonBar.SetWindows7Look(FALSE);
		break;

	case ID_VIEW_APPLOOK_OFF_XP:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOfficeXP));
		m_wndRibbonBar.SetWindows7Look(FALSE);
		break;

	case ID_VIEW_APPLOOK_WIN_XP:
		CMFCVisualManagerWindows::m_b3DTabsXPTheme = TRUE;
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
		m_wndRibbonBar.SetWindows7Look(FALSE);
		break;

	case ID_VIEW_APPLOOK_OFF_2003:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2003));
		CDockingManager::SetDockingMode(DT_SMART);
		m_wndRibbonBar.SetWindows7Look(FALSE);
		break;

	case ID_VIEW_APPLOOK_VS_2005:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2005));
		CDockingManager::SetDockingMode(DT_SMART);
		m_wndRibbonBar.SetWindows7Look(FALSE);
		break;

	case ID_VIEW_APPLOOK_VS_2008:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2008));
		CDockingManager::SetDockingMode(DT_SMART);
		m_wndRibbonBar.SetWindows7Look(FALSE);
		break;

	case ID_VIEW_APPLOOK_WINDOWS_7:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows7));
		CDockingManager::SetDockingMode(DT_SMART);
		m_wndRibbonBar.SetWindows7Look(TRUE);
		break;

	default:
		switch (theApp.m_nAppLook)
		{
		case ID_VIEW_APPLOOK_OFF_2007_BLUE:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_LunaBlue);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_BLACK:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_SILVER:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Silver);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_AQUA:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Aqua);
			break;
		}

		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
		CDockingManager::SetDockingMode(DT_SMART);
		m_wndRibbonBar.SetWindows7Look(FALSE);
	}

	RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ERASE);

	theApp.WriteInt(_T("ApplicationLook"), theApp.m_nAppLook);
}

void CMainFrame::OnUpdateApplicationLook(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(theApp.m_nAppLook == pCmdUI->m_nID);
}

void CMainFrame::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CMDIFrameWndEx::OnSettingChange(uFlags, lpszSection);
	m_wndOutput.UpdateFonts();
}




void CMainFrame::OnFileNew()
{
	CFileDialog dlg(FALSE,  // TRUE��Open��FALSE����Save As�ļ��Ի���
		".xml",  // Ĭ�ϵĴ��ļ�������
		NULL, // Ĭ�ϴ򿪵��ļ��� 
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR,  // ��ѡ��
		"xml�ļ�(*.xml)|*.xml|�����ļ�(*.*) |*.*||"  // �򿪵��ļ�����
	);
	dlg.m_ofn.lpstrTitle = "�趨�����ļ�����·��";
	if (dlg.DoModal() == IDOK)
	{
		CString strPrjPath = dlg.GetPathName();
		if (FunGetFileExt(strPrjPath) != "xml")  //��֤����Ϊxml�ļ�
		{
			strPrjPath += ".xml";
		}

		if (theApp.GetOpenDocumentCount() == 2)
		{
			CloseStereDoc();
		}
		ClearData();
		m_Project.SavePrj(strPrjPath);
		CDlgPrjNew2 dlg2;
		if (dlg2.DoModal() == IDOK)
		{
			m_strPrjPath = strPrjPath;
			NewPrj(dlg2);
		}
		else
		{
			AfxMessageBox("���̴���ʧ�ܣ�");
		}
	}
}
void CMainFrame::OnFileOpen()
{
	// TODO: Add your command handler code here
	CString FilePathName;
	CFileDialog dlg(TRUE, //TRUEΪOPEN�Ի���FALSEΪSAVE AS�Ի���
		NULL,
		NULL,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		(LPCTSTR)_TEXT("XML Files (*.xml)|*.xml|All Files (*.*)|*.*||"),
		NULL);
	if (dlg.DoModal() == IDOK)
	{
		FilePathName = dlg.GetPathName(); //�ļ�����������FilePathName��
		if (FilePathName == m_Project.GetPrjPath()) return;
		ClearData();
		if (theApp.GetOpenDocumentCount() == 2)
		{
			CloseStereDoc();
		}
		if (!m_Project.OpenPrj(FilePathName))
		{
			ClearData();
			AfxMessageBox("���̴�ʧ�ܣ�==" + FilePathName);
			return;
		}
		OpenPrj();
		m_strPrjPath = FilePathName;
	}

}
void CMainFrame::NewPrj(CDlgPrjNew2 &dlg)
{
	ClearAllReader();
	m_Project.NewPrj(m_strPrjPath, dlg.m_vecRefPath, dlg.m_vecDomGroups, dlg.m_nDomNum);
	if (!m_Project.SavePrj(m_Project.GetPrjPath()))
	{
		AfxMessageBox("���̴���ʧ��==" + m_Project.GetPrjPath());
		m_Project.ClearPrj();
		return;
	}
	if (m_Project.GetGroupNum() == 0)
	{
		AfxMessageBox("DOM��Ϊ0�����̴���ʧ��==" + m_Project.GetPrjPath());
		m_Project.ClearPrj();
		return;
	}

	DWORD id = 0; HANDLE hd;
	hd = CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)MultiThreadBuildPyramid,
		(LPVOID)this, 0, &id);
	InitRefCombo();
}
void CMainFrame::OpenPrj()
{
	DWORD id = 0; HANDLE hd;
	hd = CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)MultiThreadBuildPyramid,
		(LPVOID)this, 0, &id);
	InitRefCombo();
}

void CMainFrame::InitRefCombo()
{
	CMFCRibbonComboBox* pCom = NULL;
	pCom = DYNAMIC_DOWNCAST(CMFCRibbonComboBox, GetMainFramHand()->m_wndRibbonBar.FindByID(ID_COMBO_SELECT_REF));
	if (!pCom) return;
	pCom->RemoveAllItems();
	for (int i = 0; i < m_Project.GetRefNum(); i++)
	{
		CString strRef;
		strRef.Format("%-3d %s", i, FunGetFileName(m_Project.GetRefPath(i), true));
		pCom->AddItem(strRef, i);
	}
	pCom->SelectItem(0);
}


void CMainFrame::UpdateViewer()
{
	//��ʱdoc��Ϊ0
	if (theApp.GetOpenDocumentCount() == 0)
	{
		if (m_pMainView)
		{
			m_pMainView = NULL;
		}
		theApp.m_pDocManager->OnFileNew();
	}
	SendMessage(WM_WND_PROGRESS_POS, 1, 20.0);
	CChildFrame *pChildFrame = GetChildFramHand();
	//	m_pMainView->SetRC(pChildFrame->m_hRC);
	m_pMainView->OnInitialUpdate();

	GLContext context;
	context.hDC = m_pMainView->m_hDC;
	context.hRC = m_pMainView->GetShareRC();
	m_pReaderMain = new ImageReader(context);
	m_pMainView->AttachReader(m_pReaderMain);

	LoadImageGrids();
	SendMessage(WM_WND_PROGRESS_POS, 1, 30.0);
	LoadImageTexts();
	OGREnvelope enveAll = m_pReaderMain->GetGroundRange();
	m_pMainView->InitialEnvelope(enveAll);
	SendMessage(WM_WND_PROGRESS_POS, WPARAM(&m_wndProgress), 100.0);
	SendMessage(WM_WND_PROGRESS_SHOW, WPARAM(&m_wndProgress), 0);
	// 	enveAll = m_pReaderMain->GetGroundRange();
	// 	m_pMainView->InitialEnvelope(enveAll);
}
void CMainFrame::LoadImageGrids()
{
	/************************************************************************/
	/* ImageGrids                                                                     */
	/************************************************************************/
	CChildFrame *pChildFrame = GetChildFramHand();
	MultiThreadWGLMakeCurrent(pChildFrame->m_hDC, pChildFrame->m_hRC);
	ImageGridEnve(m_ImgGridEnves);
	GetPolysMapBuffer(m_ImgGridEnves, m_ImgGridsVBO);
	OGREnvelope GridRange = m_ImgGridEnves[0];
	for (int i = 0; i < m_ImgGridEnves.size(); i++)
	{
		GridRange.MinX = min(GridRange.MinX, m_ImgGridEnves[i].MinX);
		GridRange.MaxX = max(GridRange.MaxX, m_ImgGridEnves[i].MaxX);
		GridRange.MinY = min(GridRange.MinY, m_ImgGridEnves[i].MinY);
		GridRange.MaxY = max(GridRange.MaxY, m_ImgGridEnves[i].MaxY);
	}
	m_pReaderMain->SetGroundRange(GridRange);
	MultiThreadWGLMakeCurrent(NULL, NULL);
}
void CMainFrame::LoadImageTexts()
{
	CChildFrame *pChildFrame = GetChildFramHand();
	MultiThreadWGLMakeCurrent(pChildFrame->m_hDC, pChildFrame->m_hRC);
	for (int i = 0; i < m_Project.GetGroupNum(); i++)
	{
		OrthoImage* pImg = new OrthoImage;
		pImg->SetOrthoID(1 + i);
		pImg->LoadDOM(m_Project.GetDomGroup()[i].strPanPath.GetBuffer(), "", m_ImgGridEnves[i]);
		OGREnvelope enve = pImg->GetGroundRange();
		m_pReaderMain->AddImage(pImg);
		SendMessage(WM_WND_PROGRESS_POS, 1, 30.0 + (i + 1) * 70 / m_Project.GetGroupNum());
	}
	MultiThreadWGLMakeCurrent(NULL, NULL);
}
void CMainFrame::LoadMatchPoint()
{
	m_Project.LoadPoint(m_nImgIdxInMatchView, m_pReaderRef, m_eOpenDOMType);
}
void CMainFrame::ImageGridEnve(map<int, OGREnvelope>& imgGridEnve)
{
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
	int nImgNum = m_Project.GetGroupNum();
	int nGridCols = ImageGridCols, nGridRows = ceil(nImgNum*1.0 / ImageGridCols);
	int width, height = ImageGridHeight;
	vector<int> vecWidth(nImgNum);
	vector<int> vecHeight(nImgNum);
	for (int i = 0; i < nImgNum; i++)
	{
		GDALDataset *pDataSet = (GDALDataset *)GDALOpen(m_Project.GetDomGroup()[i].strPanPath, GA_ReadOnly);
		if (!pDataSet) continue;
		int nCols = pDataSet->GetRasterXSize();
		int nRows = pDataSet->GetRasterYSize();
		width = nCols*height / nRows;
		vecWidth[i] = width;
		vecHeight[i] = height;
		GDALClose(pDataSet);
	}
	CRect rect; m_pMainView->GetClientRect(rect);
	int nIdx = 0;
	int top = ImageGridGap;
	int left = ImageGridGap;
	for (int i = 0; i < nGridRows; i++)
	{
		top += (ImageGridHeight + ImageGridGap);
		left = ImageGridGap;
		for (int j = 0; j < nGridCols; j++)
		{
			OGREnvelope enve;
			enve.MinX = left;
			enve.MaxY = rect.Height() - top;
			enve.MaxX = left + vecWidth[nIdx];
			enve.MinY = rect.Height() - (top + vecHeight[nIdx]);
			imgGridEnve.insert(std::make_pair(nIdx, enve));
			left += (vecWidth[nIdx] + ImageGridGap);
			nIdx++;
			if (nIdx == nImgNum) break;
		}
	}
	vector<int>().swap(vecWidth);
	vector<int>().swap(vecHeight);
}
void CMainFrame::OpenMatchView()
{
	// TODO: Add your command handler code here
	ClearStereReader();
	theApp.m_nRefIdx = 0;
	//��֤��doc��Ϊ2�ҵ�ǰΪƥ��㴰��
	if (theApp.GetOpenDocumentCount() == 1)
	{
		theApp.m_pDocManager->OnFileNew();
	}
	else
	{
		Switch2StereDoc();
	}
	CChildFrame *pChildFrame = GetChildFramHand();
	m_pLView->OnInitialUpdate();
	m_pRView->OnInitialUpdate();
	GLContext context;
	context.hDC = pChildFrame->m_hDC;
	context.hRC = pChildFrame->m_hRC;

	m_pReaderStere = new ImageReader(context);
	m_pLView->AttachReader(m_pReaderStere);
	m_pRView->AttachReader(m_pReaderStere);

	MultiThreadWGLMakeCurrent(pChildFrame->m_hDC, pChildFrame->m_hRC);


	string strDom, strRef;
	switch (m_eOpenDOMType)
	{
	case PAN:
		strDom = m_Project.GetDomGroup()[m_nImgIdxInMatchView].strPanPath;
		break;
	case MUX:
		strDom = m_Project.GetDomGroup()[m_nImgIdxInMatchView].strMuxPath;
		break;
	case FUS:
		strDom = m_Project.GetDomGroup()[m_nImgIdxInMatchView].strFusPath;
		break;
	default:
		break;
	}
	OrthoImage* pImgDom = new OrthoImage;
	pImgDom->SetOrthoID(1);
	CString strShp = FunGetFileFolder(strDom.c_str()) + "\\" + FunGetFileName(strDom.c_str(), false) + ".shp";
	pImgDom->LoadDOM(strDom, "");
	OGREnvelope enveDom;
	enveDom = pImgDom->GetGroundRange();
	g_curDomEnve = enveDom;
	pImgDom->SetOffset(enveDom.MinX, enveDom.MinY);
	m_pReaderStere->AddImage(pImgDom);
	for (int i = 0; i < 1/*m_Project.GetRefNum()*/; i++)  //ֻ���һ�Ųο�ͼ��
	{
		strRef = m_Project.GetRefPath(i);
		OrthoImage* pImgRef = new OrthoImage;
		pImgRef->SetOrthoID(2);
		pImgRef->LoadDOM(strRef, "");
		pImgRef->SetOffset(enveDom.MinX, enveDom.MinY);
		m_pReaderStere->AddImage(pImgRef);
	}

	//�������вο�ͼ����
	if (m_pReaderRef)
	{
		m_pReaderRef->ClearData();
	}
	m_pReaderRef = new ImageReader(context);
	for (int i = 0; i < m_Project.GetRefNum(); i++)
	{
		OrthoImage*pImgRef = new OrthoImage;
		pImgRef->LoadDOM(m_Project.GetRefPath(i).GetBuffer(), "");
		pImgRef->SetOffset(enveDom.MinX, enveDom.MinY);
		m_pReaderRef->AddImage(pImgRef);
	}

	MultiThreadWGLMakeCurrent(NULL, NULL);

	/************************************************************************/
	/* points                                                                     */
	/************************************************************************/
//	m_Project.SavePoint();
	LoadMatchPoint();
	MultiThreadWGLMakeCurrent(GetChildFramHand()->m_hDC, GetChildFramHand()->m_hRC);
	GetPointsMapBuffer(m_Project.GetDomPoints(theApp.m_nRefIdx), m_pReaderStere->GetImage(0)->GetGSD(), m_LPointsVBO);
	GetPointsMapBuffer(m_Project.GetRefPoints(theApp.m_nRefIdx), m_pReaderStere->GetImage(1)->GetGSD(), m_RPointsVBO);
	MultiThreadWGLMakeCurrent(NULL, NULL);
	/************************************************************************/
	/* ��ʾ��Χ��ʼ��                                                                     */
	/************************************************************************/
	enveDom = pImgDom->GetGroundRange();
	m_pLView->InitialEnvelope(enveDom);
	m_pRView->InitialEnvelope(enveDom);

	//��ǰrefcombo
	CMFCRibbonComboBox *pCom = DYNAMIC_DOWNCAST(CMFCRibbonComboBox, m_wndRibbonBar.FindByID(ID_COMBO_SELECT_REF));
	if (!pCom) return;
	pCom->SelectItem(theApp.m_nRefIdx);
}

eDOMTYPE CMainFrame::HighlightMatchViewImage()
{
	return m_wndImgData.HighlightSelectedItem(m_nImgIdxInMatchView);
}


LRESULT CMainFrame::SyncDisplay(WPARAM wPara, LPARAM lPara)
{
	if ((CDOMRectifyView*)wPara == m_pMainView)
	{
		return 0;
	}
	else
	{
		RECT LRect, RRect;
		CChildFrame *pChildFrame = GetChildFramHand();
		CMainFrame *pMainFrame = GetMainFramHand();
		OGREnvelope Lenve = pMainFrame->m_pLView->GetCurrentGroundRange(), Renve = pMainFrame->m_pRView->GetCurrentGroundRange(), NewEnve;
		::GetClientRect(pMainFrame->m_pLView->GetSafeHwnd(), &LRect);
		::GetClientRect(pMainFrame->m_pRView->GetSafeHwnd(), &RRect);
		if ((CDOMRectifyView*)wPara == pMainFrame->m_pLView) //�󴰿ڷ�������ƽ����Ϣ
		{
			double xzoom = (Lenve.MaxX - Lenve.MinX) / (LRect.right - LRect.left);
			double yzoom = (Lenve.MaxY - Lenve.MinY) / abs(LRect.top - LRect.bottom);
			double RViewXLen = xzoom *  (RRect.right - RRect.left);
			double RViewYLen = yzoom *  abs(RRect.top - RRect.bottom);
			double centerX = (Lenve.MaxX + Lenve.MinX) / 2.0;
			double centerY = (Lenve.MaxY + Lenve.MinY) / 2.0;
			NewEnve.MinX = centerX - RViewXLen / 2.0;
			NewEnve.MaxX = centerX + RViewXLen / 2.0;
			NewEnve.MinY = centerY - RViewYLen / 2.0;
			NewEnve.MaxY = centerY + RViewYLen / 2.0;
			pMainFrame->m_pRView->SyncDisplay(NewEnve);
			if (LRect.right - LRect.left < RRect.right - RRect.left)
			{
				pMainFrame->m_pRView->ResetEnvelope(NewEnve);
			}
		}
		else if ((CDOMRectifyView*)wPara == pMainFrame->m_pRView) //�Ҵ��ڷ�������ƽ����Ϣ
		{
			double xzoom = (Renve.MaxX - Renve.MinX) / (RRect.right - RRect.left);
			double yzoom = (Renve.MaxY - Renve.MinY) / abs(RRect.top - RRect.bottom);
			double LViewXLen = xzoom *  (LRect.right - LRect.left);
			double LViewYLen = yzoom *  abs(LRect.top - LRect.bottom);
			double centerX = (Renve.MaxX + Renve.MinX) / 2.0;
			double centerY = (Renve.MaxY + Renve.MinY) / 2.0;
			NewEnve.MinX = centerX - LViewXLen / 2.0;
			NewEnve.MaxX = centerX + LViewXLen / 2.0;
			NewEnve.MinY = centerY - LViewYLen / 2.0;
			NewEnve.MaxY = centerY + LViewYLen / 2.0;
			pMainFrame->m_pLView->SyncDisplay(NewEnve);
			if (LRect.right - LRect.left > RRect.right - RRect.left)
			{
				pMainFrame->m_pLView->ResetEnvelope(NewEnve);
			}
		}
	}
	return 0;
}

LRESULT CMainFrame::OnOpenMatchView(WPARAM wPara, LPARAM lPara)
{
	m_nImgIdxInMatchView = *((int*)wPara);
	if (m_nImgIdxInMatchView < 0) return 0;
	m_eOpenDOMType = HighlightMatchViewImage();
	OpenMatchView();
	return 1;
}

LRESULT CMainFrame::OnProgressVisible(WPARAM wPara, LPARAM lPara)
{
	if ((CGradientProgressCtrl*)wPara == &m_wndProgress)
		m_wndProgress.ShowWindow(int(lPara));
	else if ((CGradientProgressCtrl*)wPara == &m_wndProgressPyramid)
		m_wndProgressPyramid.ShowWindow(int(lPara));
	return 1;
}

LRESULT CMainFrame::OnProgressPosition(WPARAM wPara, LPARAM lPara)
{
	if ((CGradientProgressCtrl*)wPara == &m_wndProgress)
		m_wndProgress.SetPos(max(float(1), float(lPara)));
	else if ((CGradientProgressCtrl*)wPara == &m_wndProgressPyramid)
		m_wndProgressPyramid.SetPos(float(lPara));
	return 1;
}

LRESULT CMainFrame::OnCreateDockpane(WPARAM wPara, LPARAM lPara)
{
	SendMessage(WM_WND_PROGRESS_POS, 1, 5.0);
	if (!m_wndImgData.GetSafeHwnd())
	{
		if (!CreateDockingWindows())
		{
			TRACE0("δ�ܴ���ͣ������\n");
			return -1;
		}
	}
	m_wndImgData.FillImgTree(m_Project.GetDomGroup(), m_Project.GetRefPath());
	SendMessage(WM_WND_PROGRESS_POS, 1, 10.0);

	return 1;
}

LRESULT CMainFrame::OnUpdateViewer(WPARAM wPara, LPARAM lPara)
{
	UpdateViewer();
	return 1;
}

LRESULT CMainFrame::OnUpdateStatusInfo(WPARAM wPara, LPARAM lPara)
{
	CMFCRibbonStatusBarPane*pStatus = (CMFCRibbonStatusBarPane*)wPara;
	CString strInfo;
	if (lPara != 0) strInfo = *(CString*)lPara;
	else strInfo.Empty();
	if (pStatus)
	{
		pStatus->SetText(strInfo);
		pStatus->Redraw();
	}
	return 1;
}

LRESULT CMainFrame::OnNewProject(WPARAM wPara, LPARAM lPara)
{
	SendMessage(WM_WND_PROGRESS_SHOW, WPARAM(&m_wndProgress), 1);
	SendMessage(WM_WND_PROGRESS_POS, WPARAM(&m_wndProgress), 1.0);

	/************************************************************************/
	/* �������ڣ����߳�                                                 */
	/************************************************************************/
	DWORD id = 0; HANDLE hd;
	hd = CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)MultiThreadCreateDockPanes,
		(LPVOID)this, 0, &id);
	/************************************************************************/
	/* ������ʾ���ݣ����߳�                                                 */
	/************************************************************************/
	hd = CreateThread(NULL, 0,
		(LPTHREAD_START_ROUTINE)MultiThreadUpdateViewer,
		(LPVOID)this, 0, &id);
	return 1;
}


LRESULT CMainFrame::OnUpdateMatchView(WPARAM wPara, LPARAM lPara)
{
	CChildFrame* pChildFrame = GetChildFramHand();
	if (lPara == 0)   //����pan,mux,fus
		m_eOpenDOMType = eDOMTYPE(wPara);
	else
		m_eOpenDOMType = HighlightMatchViewImage();
	MultiThreadWGLMakeCurrent(pChildFrame->m_hDC, pChildFrame->m_hRC);
	/************************************************************************/
	/* Update Image                                                         */
	/************************************************************************/
	if (lPara == 0)  //�л�DOM
	{
		string strDom;
		switch (m_eOpenDOMType)
		{
		case PAN:
			strDom = m_Project.GetDomGroup()[m_nImgIdxInMatchView].strPanPath;
			break;
		case MUX:
			strDom = m_Project.GetDomGroup()[m_nImgIdxInMatchView].strMuxPath;
			break;
		case FUS:
			strDom = m_Project.GetDomGroup()[m_nImgIdxInMatchView].strFusPath;
			break;
		default:
			break;
		}
		OrthoImage* pImgDom = new OrthoImage;
		pImgDom->SetOrthoID(1);
		pImgDom->LoadDOM(strDom, "");
		g_curDomEnve = pImgDom->GetGroundRange();
		OGREnvelope enve = g_curDomEnve;
		pImgDom->SetOffset(enve.MinX, enve.MinY);
		m_pReaderStere->SetImage(pImgDom, 0);
		MultiThreadWGLMakeCurrent(NULL, NULL);
		/************************************************************************/
		/* Update Points                                                        */
		/************************************************************************/
	//	m_Project.SavePoint();

		LoadMatchPoint();
		MultiThreadWGLMakeCurrent(GetChildFramHand()->m_hDC, GetChildFramHand()->m_hRC);
		GetPointsMapBuffer(m_Project.GetDomPoints(theApp.m_nRefIdx), m_pReaderStere->GetImage(0)->GetGSD(), m_LPointsVBO);
		GetPointsMapBuffer(m_Project.GetRefPoints(theApp.m_nRefIdx), m_pReaderStere->GetImage(1)->GetGSD(), m_RPointsVBO);
		MultiThreadWGLMakeCurrent(NULL, NULL);

		/************************************************************************/
		/* Initial envelope                                                     */
		/************************************************************************/
		m_pLView->InitialEnvelope(pImgDom->GetGroundRange());
	}
	else //�л��ο�ͼ
	{
		string strRef = m_Project.GetRefPath(theApp.m_nRefIdx);
		OrthoImage* pImgRef = new OrthoImage;
		pImgRef->SetOrthoID(2);
		pImgRef->LoadDOM(strRef, "");


		OGREnvelope enve = g_curDomEnve;
		pImgRef->SetOffset(enve.MinX, enve.MinY);
		m_pReaderStere->SetImage(pImgRef, 1);
//		m_Project.SavePoint(m_pReaderRef);

		LoadMatchPoint();
		MultiThreadWGLMakeCurrent(GetChildFramHand()->m_hDC, GetChildFramHand()->m_hRC);
 		glDeleteBuffers(1, &m_LPointsVBO);
 		glDeleteBuffers(1, &m_RPointsVBO);
		GetPointsMapBuffer(m_Project.GetDomPoints(theApp.m_nRefIdx), m_pReaderStere->GetImage(0)->GetGSD(), m_LPointsVBO);
		GetPointsMapBuffer(m_Project.GetRefPoints(theApp.m_nRefIdx), m_pReaderStere->GetImage(1)->GetGSD(), m_RPointsVBO);
		MultiThreadWGLMakeCurrent(NULL, NULL);

		/************************************************************************/
		/* Initial envelope                                                     */
		/************************************************************************/
		m_pLView->InitialEnvelope(pImgRef->GetGroundRange());
		m_pRView->InitialEnvelope(pImgRef->GetGroundRange());
	}

	return 1;
}

LRESULT CMainFrame::OnUpdateOrthoImage(WPARAM wPara, LPARAM lPara)
{
	CChildFrame* pChildFrame = GetChildFramHand();

	MultiThreadWGLMakeCurrent(pChildFrame->m_hDC, pChildFrame->m_hRC);
	///************************************************************************/
	///* Update Image                                                         */
	///************************************************************************/
	
	OrthoImage* pImgDom = new OrthoImage;
	pImgDom->SetOrthoID(1);
	pImgDom->LoadDOM(m_Project.GetCurRecDomPath().GetBuffer(), "");
	g_curDomEnve = pImgDom->GetGroundRange();
	OGREnvelope enve = g_curDomEnve;
	pImgDom->SetOffset(enve.MinX, enve.MinY);
	m_pReaderStere->SetImage(pImgDom, 0);
	MultiThreadWGLMakeCurrent(NULL, NULL);

	return 1;
}


LRESULT CMainFrame::OnAddMatchPoint(WPARAM wPara, LPARAM lPara)
{
	MultiThreadWGLMakeCurrent(GetChildFramHand()->m_hDC, GetChildFramHand()->m_hRC);

	if (wPara && !lPara) //��ʾ�����
	{
		vector<stuMatchPoint> vecMatchPoints(1);
		vecMatchPoints[0].nPtIdx = 0;
		vecMatchPoints[0].nRefIdx = theApp.m_nRefIdx;
		vecMatchPoints[0].pt = theApp.m_ptDom;
		GetPointsMapBuffer(vecMatchPoints, m_pReaderStere->GetImage(0)->GetGSD(), m_LPointsVBOAdd);
		vector<stuMatchPoint>().swap(vecMatchPoints);

	}
	else if (!wPara && lPara)//��ʾ�����
	{
		vector<stuMatchPoint> vecMatchPoints(1);
		vecMatchPoints[0].nPtIdx = 0;
		vecMatchPoints[0].nRefIdx = theApp.m_nRefIdx;
		vecMatchPoints[0].pt = theApp.m_ptRef;
		GetPointsMapBuffer(vecMatchPoints, m_pReaderStere->GetImage(1/*+theApp.m_nRefIdx*/)->GetGSD(), m_RPointsVBOAdd);
		vector<stuMatchPoint>().swap(vecMatchPoints);
	}
	else if (wPara && lPara)//��ɴ̵㣬���ͬ����
	{
		m_Project.AddPoint(theApp.m_ptDom, theApp.m_ptRef, theApp.m_nRefIdx);
		m_Project.SavePoint(m_pReaderRef);
		glDeleteBuffers(1, &m_LPointsVBO);
		glDeleteBuffers(1, &m_RPointsVBO);
		GetPointsMapBuffer(m_Project.GetDomPoints(theApp.m_nRefIdx), m_pReaderStere->GetImage(0)->GetGSD(), m_LPointsVBO);
		GetPointsMapBuffer(m_Project.GetRefPoints(theApp.m_nRefIdx), m_pReaderStere->GetImage(1/*+theApp.m_nRefIdx*/)->GetGSD(), m_RPointsVBO);
//		AddPointMapBuffer(m_Project.GetCurPointNum()-1, m_Project.GetDomPoints()[m_Project.GetAllPointNum()-1], m_pReaderStere->GetImage(0)->GetGSD(), m_LPointsVBO);
//		AddPointMapBuffer(m_Project.GetCurPointNum()-1, m_Project.GetRefPoints()[m_Project.GetAllPointNum()-1], m_pReaderStere->GetImage(1/*+theApp.m_nRefIdx*/)->GetGSD(), m_RPointsVBO);

		AutoRectifyOnTIme();
	}
	MultiThreadWGLMakeCurrent(NULL, NULL);

	//����ͬ����vbo

	return 1;
}

LRESULT CMainFrame::OnDelMatchPoint(WPARAM wPara, LPARAM lPara)
{
	m_Project.DelPoint();
	MultiThreadWGLMakeCurrent(GetChildFramHand()->m_hDC, GetChildFramHand()->m_hRC);
	glDeleteBuffers(1, &m_LPointsVBO);
	glDeleteBuffers(1, &m_RPointsVBO);
	GetPointsMapBuffer(m_Project.GetDomPoints(theApp.m_nRefIdx), m_pReaderStere->GetImage(0)->GetGSD(), m_LPointsVBO);
	GetPointsMapBuffer(m_Project.GetRefPoints(theApp.m_nRefIdx), m_pReaderStere->GetImage(1/*+theApp.m_nRefIdx*/)->GetGSD(), m_RPointsVBO);
	MultiThreadWGLMakeCurrent(NULL, NULL);
	m_Project.SavePoint(m_pReaderRef);
	return 1;
}




void CMainFrame::OnButtonPrjClose()
{
	// TODO: Add your command handler code here
	ClearData();
	theApp.CloseAllDocuments(TRUE);
}


void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
	CMDIFrameWndEx::OnSize(nType, cx, cy);
	if (m_wndProgress.GetSafeHwnd())
	{
		CRect RectStatus, RectSystemTime, RectProgress, RectProgressPyramid, RectInfo, RectProcess;
		m_wndStatusBar.GetClientRect(RectStatus);
		RectSystemTime = m_pStatusPaneSysTime->GetRect();
		RectProgress.right = max(RectStatus.left + 5, RectSystemTime.left - 10);
		RectProgress.left = max(RectProgress.right - 250, RectStatus.left + 1);
		RectProgress.top = RectSystemTime.top;
		RectProgress.bottom = RectSystemTime.bottom;

		RectProgressPyramid = RectProgress;
		RectProgressPyramid.right = RectProgress.left - 10;
		RectProgressPyramid.left = max(RectProgressPyramid.right - 150, RectStatus.left + 1);

		m_wndProgress.MoveWindow(RectProgress);
		m_wndProgressPyramid.MoveWindow(RectProgressPyramid);

		RectProcess = RectProgressPyramid;
		RectProcess.right = RectProgressPyramid.left - 5;
		RectProcess.left = max(RectProcess.right - 250, RectStatus.left + 1);
		m_pStatusPaneProcessing->SetRect(RectProcess);

		m_wndStatusBar.SetRedraw();
		m_pStatusPaneProcessing->Redraw();
	}

	CMFCVisualManager::GetInstance()->OnSetWindowRegion(this, CSize(cx, cy));
	// TODO: Add your message handler code here
}


void CMainFrame::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	switch (nIDEvent)
	{
	case WM_SYSTEM_TIME:
		m_pStatusPaneSysTime->SetText(FunGetSysTimeStr());
		m_pStatusPaneSysTime->Redraw();
		break;
	default:
		break;
	}
	CMDIFrameWndEx::OnTimer(nIDEvent);
}



void CMainFrame::OnCheckAutoSelectRef()
{
	// TODO: Add your command handler code here
	m_bAutoSelectRef = !m_bAutoSelectRef;
}


void CMainFrame::OnUpdateCheckAutoSelectRef(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	if (m_Project.GetRefNum() == 0)
	{
		pCmdUI->Enable(FALSE);
		return;
	}
	pCmdUI->Enable(TRUE);
	pCmdUI->SetCheck(m_bAutoSelectRef);
}


void CMainFrame::OnComboSelectRef()
{
	// TODO: Add your command handler code here
	CMFCRibbonComboBox* pCom = NULL;
	pCom = DYNAMIC_DOWNCAST(CMFCRibbonComboBox, m_wndRibbonBar.FindByID(ID_COMBO_SELECT_REF));
	if (!pCom) return;
	theApp.m_nRefIdx = pCom->GetCurSel();
//	m_Project.SavePoint(m_pReaderRef);
	if (theApp.GetOpenDocumentCount() == 2)
		::SendMessage(GetMainFramHand()->m_hWnd, WM_UPDATE_MATCHVIEW, 0, 1);
}


void CMainFrame::OnUpdateComboSelectRef(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(m_Project.GetRefNum());
}


void CMainFrame::OnCheckViewPoint()
{
	// TODO: Add your command handler code here
	m_bViewPoint = !m_bViewPoint;

}


void CMainFrame::OnUpdateCheckViewPoint(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
// 	if (theApp.GetOpenDocumentCount() != 2)
// 	{
// 		pCmdUI->Enable(FALSE);
// 		return;
// 	}
	pCmdUI->SetCheck(m_bViewPoint);
}
