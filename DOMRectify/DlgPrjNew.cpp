// DlgPrjNew.cpp : implementation file
//

#include "stdafx.h"
#include "DOMRectify.h"
#include "DlgPrjNew.h"
#include "afxdialogex.h"


// CDlgPrjNew dialog

IMPLEMENT_DYNAMIC(CDlgPrjNew, CDialogEx)

CDlgPrjNew::CDlgPrjNew(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_PRJ_NEW, pParent)
	, m_strDomFolder(_T(""))
	, m_strRefPath(_T(""))
	, m_strPrjPath(_T(""))
	, m_bAutoSelection(FALSE)
{

}

CDlgPrjNew::~CDlgPrjNew()
{
}

void CDlgPrjNew::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_PRJ_PATH, m_EditBrowsePrjPath);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_REF_PATH, m_EditBrowseRefPath);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_DOM_FOLDER, m_EditBrowseDomFolder);
	DDX_Text(pDX, IDC_MFCEDITBROWSE_DOM_FOLDER, m_strDomFolder);
	DDX_Text(pDX, IDC_MFCEDITBROWSE_REF_PATH, m_strRefPath);
	DDX_Text(pDX, IDC_MFCEDITBROWSE_PRJ_PATH, m_strPrjPath);
	DDX_Control(pDX, IDC_LIST_IMAGE, m_listImgInfo);
	DDX_Check(pDX, IDC_CHECK_AUTO_SELECTION, m_bAutoSelection);
	DDX_Control(pDX, IDC_COMBO_DOMFILE_EXT, m_CombDomFileExt);
}

void CDlgPrjNew::ImageGrouping(vector<CString>& vecAllPanPath, vector<CString>& vecAllMuxPath, vector<CString>& vecAllFusPath)
{
}

void CDlgPrjNew::ClearData()
{
	vector<stuImgGroup>().swap(m_vecImgGroups);
}

void CDlgPrjNew::ImageGrouping(vector<CString>& vecAllImgPath)
{
	for (int i = 0; i<vecAllImgPath.size(); i++)
	{
		stuImgGroup group; 
		group.strPanPath = vecAllImgPath[i];
		group.strMuxPath = vecAllImgPath[i];
		group.strFusPath = vecAllImgPath[i];
		group.strPanShp = FunGetFileFolder(group.strPanPath) + "\\" + FunGetFileName(group.strPanPath, false) + ".shp";
		group.strMuxShp = FunGetFileFolder(group.strMuxPath) + "\\" + FunGetFileName(group.strMuxPath, false) + ".shp";
		group.strFusShp = FunGetFileFolder(group.strFusPath) + "\\" + FunGetFileName(group.strFusPath, false) + ".shp";
		m_vecImgGroups.push_back(group);
	}
}


BEGIN_MESSAGE_MAP(CDlgPrjNew, CDialogEx)
	ON_EN_CHANGE(IDC_MFCEDITBROWSE_DOM_FOLDER, &CDlgPrjNew::OnEnChangeMfceditbrowseDomFolder)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST_IMAGE, &CDlgPrjNew::OnGetdispinfoListImginfo)
	ON_BN_CLICKED(IDOK, &CDlgPrjNew::OnBnClickedOk)
	ON_EN_CHANGE(IDC_MFCEDITBROWSE_PRJ_PATH, &CDlgPrjNew::OnEnChangeMfceditbrowsePrjPath)
END_MESSAGE_MAP()


// CDlgPrjNew message handlers


BOOL CDlgPrjNew::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	// TODO:  Add extra initialization here
	CRect rect; DWORD dwStyle; int nListWidth, nListHeight;
	dwStyle = (m_listImgInfo).GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;     //可以选择整行
	dwStyle |= LVS_EX_GRIDLINES;         //格网形式 
	dwStyle |= LVS_OWNERDRAWFIXED;       //可重绘
	dwStyle |= LVS_EX_DOUBLEBUFFER;
	m_listImgInfo.SetExtendedStyle(dwStyle);
	m_listImgInfo.GetClientRect(rect);

	m_listImgInfo.InsertColumn(0, "序号", LVCFMT_LEFT, 40, -1);
	m_listImgInfo.InsertColumn(1, "全色图像路径", LVCFMT_LEFT, 250, -1);
	m_listImgInfo.InsertColumn(2, "多光谱图像路径", LVCFMT_LEFT, 250 - 1);
	m_listImgInfo.InsertColumn(3, "融合图像路径", LVCFMT_LEFT, 250 - 1);

	m_bAutoSelection = true;
	m_CombDomFileExt.SetCurSel(0);

	m_strPrjPath = "E:\\0_G5Group\\02_program\\02_DOM\\test\\===湖南规划院===\\新建文件夹\\1.xml";
	m_strRefPath = "E:\\0_G5Group\\02_program\\02_DOM\\test\\===湖南规划院===\\blockdom.tif";
	UpdateData(false);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}


void CDlgPrjNew::OnEnChangeMfceditbrowseDomFolder()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	UpdateData(true);
	vector<CString> vecAllImgPath;
	int nExtIdx = m_CombDomFileExt.GetCurSel();
	const int nAllIdx = 0;
	switch (nExtIdx)
	{
	case 5: //all above
		FunSearchFile(m_strDomFolder, ".tif", vecAllImgPath);
		FunSearchFile(m_strDomFolder, ".tiff", vecAllImgPath);
		FunSearchFile(m_strDomFolder, ".jpg", vecAllImgPath);
		FunSearchFile(m_strDomFolder, ".jpeg", vecAllImgPath);
		break;
	default:
		CString strExt; m_CombDomFileExt.GetLBText(nExtIdx, strExt);
		FunSearchFile(m_strDomFolder, strExt, vecAllImgPath);
		break;
	}
	ImageGrouping(vecAllImgPath);
	m_listImgInfo.SetItemCountEx(m_vecImgGroups.size());
	m_listImgInfo.Invalidate();
}

void CDlgPrjNew::OnGetdispinfoListImginfo(NMHDR * pNMHDR, LRESULT * pResult)
{
	NMLVDISPINFO *pGetInfoTip = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	LV_ITEM* pItem = &(pGetInfoTip)->item;
	if (m_vecImgGroups.size() == 0) return;
	if (pItem->mask & LVIF_TEXT)
	{
// 		SSWstuPosInfo sswPos; SSWstuCmrInfo sswCmr;
// 		int nPosIdx = m_vecImgIdxToPosIdx[pItem->iItem];
// 		int nCmrIdx = m_nCmrIdxSelected;
// 		if (nPosIdx == InvalidValue) sswPos = SSWstuPosInfo();
// 		else sswPos = m_vecAllPosInfo[nPosIdx];
// 		if (nCmrIdx == InvalidValue || m_vecAllCmrInfo.size() == 0) sswCmr = SSWstuCmrInfo();
// 		else sswCmr = m_vecAllCmrInfo[nCmrIdx];
// 		CString strImgPath = m_vecAllImgPath[pItem->iItem];
		switch (pItem->iSubItem)
		{
		case 0: {
			char strId[10]; sprintf(strId, "%d", pItem->iItem);
			lstrcpyn(pItem->pszText, strId, pItem->cchTextMax);
			break;
		}
		case 1: {
			lstrcpyn(pItem->pszText, m_vecImgGroups[pItem->iItem].strPanPath, pItem->cchTextMax);
			break;
		}
		case 2: {
			lstrcpyn(pItem->pszText, m_vecImgGroups[pItem->iItem].strPanPath, pItem->cchTextMax);
			break;
		}
		case 3: {
			lstrcpyn(pItem->pszText, m_vecImgGroups[pItem->iItem].strPanPath, pItem->cchTextMax);
			break;
		}
		default:
			break;
		}
	}
	*pResult = 0;
}


void CDlgPrjNew::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
	if (m_strPrjPath.IsEmpty())
	{
		AfxMessageBox("请设置工程文件路径！");
		return;
	}
	if (FunGetFileExt(m_strPrjPath) != "xml")
	{
		m_strPrjPath += ".xml";
		UpdateData(FALSE);
	}
	if (m_strRefPath.IsEmpty())
	{
		AfxMessageBox("请选择参考图像文件！");
		return;
	}
	if (m_vecImgGroups.size() == 0)
	{
		AfxMessageBox("请选择待纠正正射影像！");
		return;
	}
	CDialogEx::OnOK();
}


void CDlgPrjNew::OnEnChangeMfceditbrowsePrjPath()
{
	UpdateData(true);
	if (!m_strPrjPath.IsEmpty()&&FunGetFileExt(m_strPrjPath) != "xml")
	{
		m_strPrjPath += ".xml";
		UpdateData(FALSE);
	}
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}
