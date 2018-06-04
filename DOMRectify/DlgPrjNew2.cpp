// DlgPrjNew2.cpp : implementation file
//

#include "stdafx.h"
#include "DOMRectify.h"
#include "DlgPrjNew2.h"
#include "afxdialogex.h"


// CDlgPrjNew2 dialog

IMPLEMENT_DYNAMIC(CDlgPrjNew2, CDialogEx)

CDlgPrjNew2::CDlgPrjNew2(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_PRJ_NEW2, pParent)
	, m_bAutoCreatePyramid(FALSE)
	, m_bJpeg(FALSE)
	, m_bJpg(FALSE)
	, m_bTif(FALSE)
	, m_bTiff(FALSE)
	, m_nDomNum(0)
	, m_nRefNum(0)
	, m_nGroupNum(0)
{

}

CDlgPrjNew2::~CDlgPrjNew2()
{
	ClearData();
}

void CDlgPrjNew2::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_AUTO_CREATE_PYRAMID, m_bAutoCreatePyramid);
	DDX_Check(pDX, IDC_CHECK_JPEG, m_bJpeg);
	DDX_Check(pDX, IDC_CHECK_JPG, m_bJpg);
	DDX_Check(pDX, IDC_CHECK_TIF, m_bTif);
	DDX_Check(pDX, IDC_CHECK_TIFF, m_bTiff);
	DDX_Text(pDX, IDC_EDIT_NUM_DOM, m_nDomNum);
	DDX_Text(pDX, IDC_EDIT_NUM_REF, m_nRefNum);
	DDX_Control(pDX, IDC_LIST_IMGINFO, m_ListImgInfo);
}

void CDlgPrjNew2::ClearData()
{
	vector<stuImgGroup>().swap(m_vecDomGroups);
	vector<CString>().swap(m_vecRefPath);
	vector<stuImgList>().swap(m_vecImgListInfo);
	m_nRefNum = 0;
	m_nDomNum = 0;
	m_nGroupNum = 0;
}

void CDlgPrjNew2::ImageGrouping(vector<CString>& vecAllImgPath)
{
	vector<stuImgGroup>().swap(m_vecDomGroups);
	for (int i = 0; i < vecAllImgPath.size(); i++)
	{
		stuImgGroup group;
		group.strMuxPath = vecAllImgPath[i];
		group.strPanPath = vecAllImgPath[i];
		group.strFusPath = vecAllImgPath[i];
// 		group.strPanShp = FunGetFileFolder(group.strPanPath) + "\\" + FunGetFileName(group.strPanPath, false) + ".shp";
// 		group.strMuxShp = FunGetFileFolder(group.strMuxPath) + "\\" + FunGetFileName(group.strMuxPath, false) + ".shp";
// 		group.strFusShp = FunGetFileFolder(group.strFusPath) + "\\" + FunGetFileName(group.strFusPath, false) + ".shp";
// 		group.strMatchFilePan = "E:\\0_G5Group\\02_program\\02_DOM\\test\\===湖南规划院===\\Match_pan.wrp";
// 		group.strMatchFileMux = "E:\\0_G5Group\\02_program\\02_DOM\\test\\===湖南规划院===\\Match_mux.wrp";
// 		group.strMatchFileFus = "E:\\0_G5Group\\02_program\\02_DOM\\test\\===湖南规划院===\\Match_fus.wrp";
		m_vecDomGroups.push_back(group);
	}
	m_nGroupNum = m_vecDomGroups.size();
}

void CDlgPrjNew2::LoadImgListFile(CString strListFile)
{
	//解析Image list xml文件
}

void CDlgPrjNew2::SearchRefImage(CString strFolder)
{
	if (m_bTif)  FunSearchFile(strFolder, "tif", m_vecRefPath);
	if (m_bTiff) FunSearchFile(strFolder, "tiff", m_vecRefPath);
	if (m_bJpg)  FunSearchFile(strFolder, "jpg", m_vecRefPath);
	if (m_bJpeg) FunSearchFile(strFolder, "jpeg", m_vecRefPath);
}

void CDlgPrjNew2::SearchDomImage(CString strFolder)
{
	vector<CString> vecImgPath;
	if (m_bTif)  FunSearchFile(strFolder, "tif", vecImgPath);
	if (m_bTiff) FunSearchFile(strFolder, "tiff", vecImgPath);
	if (m_bJpg)  FunSearchFile(strFolder, "jpg", vecImgPath);
	if (m_bJpeg) FunSearchFile(strFolder, "jpeg", vecImgPath);
	m_nDomNum = vecImgPath.size();
	ImageGrouping(vecImgPath);
	vector<CString>().swap(vecImgPath);
}

void CDlgPrjNew2::DelectImages()
{

}

void CDlgPrjNew2::UpdateImgList()
{
	m_vecImgListInfo.resize(m_vecRefPath.size() + m_vecDomGroups.size());
	for (int i = 0; i<m_vecRefPath.size(); i++)
	{
		m_vecImgListInfo[i].strId.Format("%d", i);
		m_vecImgListInfo[i].strType = "REF";
		m_vecImgListInfo[i].strIdx.Format("%d", i);
		m_vecImgListInfo[i].strPath = m_vecRefPath[i];
	}
	for (int i = 0; i<m_vecDomGroups.size(); i++)
	{
		m_vecImgListInfo[i + m_vecRefPath.size()].strId.Format("%d", i + m_vecRefPath.size());
		m_vecImgListInfo[i + m_vecRefPath.size()].strType = "DOM";
		m_vecImgListInfo[i + m_vecRefPath.size()].strIdx.Format("%d", i);
		m_vecImgListInfo[i + m_vecRefPath.size()].strPath = m_vecDomGroups[i].strPanPath;
	}
	m_ListImgInfo.SetItemCountEx(m_vecImgListInfo.size());
	m_ListImgInfo.Invalidate();
}


BEGIN_MESSAGE_MAP(CDlgPrjNew2, CDialogEx)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST_IMGINFO, &CDlgPrjNew2::OnGetdispinfoListImginfo)
	ON_BN_CLICKED(IDC_BUTTON_LOAD_IMGLIST, &CDlgPrjNew2::OnBnClickedButtonLoadImglist)
	ON_BN_CLICKED(IDC_BUTTON_LOAD_IMG_REF, &CDlgPrjNew2::OnBnClickedButtonLoadImgRef)
	ON_BN_CLICKED(IDC_BUTTON_LOAD_IMG_DOM, &CDlgPrjNew2::OnBnClickedButtonLoadImgDom)
	ON_BN_CLICKED(IDC_BUTTON_DELETE_IMG, &CDlgPrjNew2::OnBnClickedButtonDeleteImg)
END_MESSAGE_MAP()


// CDlgPrjNew2 message handlers


BOOL CDlgPrjNew2::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  Add extra initialization here
	CRect rect; DWORD dwStyle; int nListWidth, nListHeight;
	dwStyle = (m_ListImgInfo).GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;     //可以选择整行
	dwStyle |= LVS_EX_GRIDLINES;         //格网形式 
	dwStyle |= LVS_OWNERDRAWFIXED;       //可重绘
	dwStyle |= LVS_EX_DOUBLEBUFFER;
	m_ListImgInfo.SetExtendedStyle(dwStyle);
	m_ListImgInfo.GetClientRect(rect);

	m_ListImgInfo.InsertColumn(0, "序号", LVCFMT_LEFT, 50, -1);
	m_ListImgInfo.InsertColumn(1, "类型", LVCFMT_LEFT, 50, -1);
	m_ListImgInfo.InsertColumn(2, "索引", LVCFMT_LEFT, 50 - 1);
	m_ListImgInfo.InsertColumn(3, "图像路径", LVCFMT_LEFT, 450 - 1);

	m_bTiff = true;
	m_bTif = true;
	m_bJpg = false;
	m_bJpeg = false;
	m_bAutoCreatePyramid = true;

	UpdateData(false);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgPrjNew2::OnGetdispinfoListImginfo(NMHDR * pNMHDR, LRESULT * pResult)
{
	NMLVDISPINFO *pGetInfoTip = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	LV_ITEM* pItem = &(pGetInfoTip)->item;
	if (m_vecImgListInfo.size() == 0) return;
	if (pItem->mask & LVIF_TEXT)
	{
		switch (pItem->iSubItem)
		{
		case 0: {
			lstrcpyn(pItem->pszText, m_vecImgListInfo[pItem->iItem].strId, pItem->cchTextMax);
			break;
		}
		case 1: {
			lstrcpyn(pItem->pszText, m_vecImgListInfo[pItem->iItem].strType, pItem->cchTextMax);
			break;
		}
		case 2: {
			lstrcpyn(pItem->pszText, m_vecImgListInfo[pItem->iItem].strIdx, pItem->cchTextMax);
			break;
		}
		case 3: {
			lstrcpyn(pItem->pszText, m_vecImgListInfo[pItem->iItem].strPath, pItem->cchTextMax);
			break;
		}
		default:
			break;
		}
	}
	*pResult = 0;
}


void CDlgPrjNew2::OnBnClickedButtonLoadImglist()
{
	// TODO: Add your control notification handler code here
	CString FilePathName;
	CFileDialog dlg(TRUE, //TRUE为OPEN对话框，FALSE为SAVE AS对话框
		NULL,
		NULL,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		(LPCTSTR)_TEXT("XML Files (*.xml)|*.xml|All Files (*.*)|*.*||"),
		NULL);
	if (dlg.DoModal() == IDOK)
	{
		LoadImgListFile(dlg.GetPathName());
		UpdateImgList();
	}
}


void CDlgPrjNew2::OnBnClickedButtonLoadImgRef()
{
	// TODO: Add your control notification handler code here
	CFolderPickerDialog dlg;
	if (dlg.DoModal() == IDOK)
	{
		SearchRefImage(dlg.GetPathName());
		UpdateImgList();
		m_nRefNum = m_vecRefPath.size();
		UpdateData(false);
	}
}


void CDlgPrjNew2::OnBnClickedButtonLoadImgDom()
{
	// TODO: Add your control notification handler code here
	CFolderPickerDialog dlg;
	if (dlg.DoModal() == IDOK)
	{
		SearchDomImage(dlg.GetPathName());
		UpdateImgList();
		m_nGroupNum = m_vecDomGroups.size();
		UpdateData(false);
	}
}


void CDlgPrjNew2::OnBnClickedButtonDeleteImg()
{
	// TODO: Add your control notification handler code here
	POSITION pos = m_ListImgInfo.GetFirstSelectedItemPosition();
	vector<int> vecDomIdx2Delete,vecRefIdx2Delete;
	while (pos)
	{
		int nItemIdx = m_ListImgInfo.GetNextSelectedItem(pos);
		char strType[10], strImgIdx[10]; int nImgIdx;
		strcpy(strType, m_ListImgInfo.GetItemText(nItemIdx, 1));//类型
		strcpy(strImgIdx, m_ListImgInfo.GetItemText(nItemIdx, 2));//索引
		nImgIdx = atoi(strImgIdx);
		if (strcmp(strType, "DOM") == 0)
		{
			vecDomIdx2Delete.push_back(nImgIdx);
		}
		else
		{
			vecRefIdx2Delete.push_back(nImgIdx);
		}
		m_ListImgInfo.SetItemState(nItemIdx, 0, -1);//取消选择：0-取消选择，-1取消高亮
	}

	for (int i = 0; i<vecDomIdx2Delete.size(); i++)
	{
		vecDomIdx2Delete[i] -= i;
	}
	for (int i = 0; i < vecRefIdx2Delete.size(); i++)
	{
		vecRefIdx2Delete[i] -= i;
	}
	while (vecDomIdx2Delete.size())
	{
		m_vecDomGroups.erase(m_vecDomGroups.begin() + vecDomIdx2Delete[0]);
		vecDomIdx2Delete.erase(vecDomIdx2Delete.begin());
	}
	while (vecRefIdx2Delete.size())
	{
		m_vecRefPath.erase(m_vecRefPath.begin() + vecRefIdx2Delete[0]);
		vecRefIdx2Delete.erase(vecRefIdx2Delete.begin());
	}
	UpdateImgList();
	m_nRefNum = m_vecRefPath.size();
	m_nDomNum = m_vecDomGroups.size();
	UpdateData(false);
}
