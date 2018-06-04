// MyDockPaneWndData.cpp : implementation file
//

#include "stdafx.h"
#include "DOMRectify.h"
#include "MyDockPaneWndData.h"
#include "MainFrm.h"
#include "MyFunctions.h"
// CMyDockPaneWndData

IMPLEMENT_DYNAMIC(CMyDockPaneWndData, CMyDockablePane)

CMyDockPaneWndData::CMyDockPaneWndData()
{
	ClearImgTree();
}

CMyDockPaneWndData::~CMyDockPaneWndData()
{
	ClearImgTree();
}

#define  UnSelectedImgIdx 7
#define  SelectedImgIdx 4
#define  CloseGroupImgIdx 2
#define  OpenGroupImgIdx 1
void CMyDockPaneWndData::FillImgTree(vector<stuImgGroup>& vecDomGroup, vector<CString>& vecRefPath)
{
	ClearImgTree();
	HTREEITEM hRootRef = m_ImgTree.InsertItem("参考影像");
	m_ImgTree.SetItemData(hRootRef, -1);
	for (int i = 0; i<vecRefPath.size(); i++)
	{
		HTREEITEM hRef = m_ImgTree.InsertItem(vecRefPath[i], hRootRef);
		m_ImgTree.SetItemData(hRef, -1);
	}
	HTREEITEM hRootGroup = m_ImgTree.InsertItem("正射产品");
	m_ImgTree.SetItemData(hRootGroup, -1);

	for (int i = 0; i<vecDomGroup.size(); i++)
	{
		CString strGroup; strGroup.Format("Group %d", i + 1);
		HTREEITEM hGroup = m_ImgTree.InsertItem(strGroup, CloseGroupImgIdx, CloseGroupImgIdx, hRootGroup);
		m_ImgTree.SetItemData(hGroup, i);
		CString strImg; HTREEITEM hDom;
		hDom = m_ImgTree.InsertItem("Pan = " + vecDomGroup[i].strPanPath, UnSelectedImgIdx, UnSelectedImgIdx, hGroup);
		m_ImgTree.SetItemData(hDom, (i + 1)*vecDomGroup.size() + 0);

		hDom = m_ImgTree.InsertItem("Mux = " + vecDomGroup[i].strMuxPath, UnSelectedImgIdx, UnSelectedImgIdx, hGroup);
		m_ImgTree.SetItemData(hDom, (i + 1)*vecDomGroup.size() + 1);

		hDom = m_ImgTree.InsertItem("Fus = " + vecDomGroup[i].strFusPath, UnSelectedImgIdx, UnSelectedImgIdx, hGroup);
		m_ImgTree.SetItemData(hDom, (i + 1)*vecDomGroup.size() + 2);
		m_vecGroupItem.push_back(hGroup);
	}
	m_ImgTree.Expand(hRootGroup, TVE_EXPAND);
}

void CMyDockPaneWndData::ClearImgTree()
{
	if (m_ImgTree.GetSafeHwnd())
	{
		m_ImgTree.DeleteAllItems();
		vector<HTREEITEM>().swap(m_vecGroupItem);
	}
	m_nHighlight = -1;

//	m_hRoot = m_ImgTree.InsertItem("影像列表");
}

void CMyDockPaneWndData::AddToMatchList(CString strPath)
{
	bool bExist = false;
	for (int i = 0; i < m_vecImgPathInList.size(); i++)
	{
		if (strPath == m_vecImgPathInList[i])
		{
			bExist = true;
			break;
		}
	}
	if (!bExist)
	{
		m_vecImgPathInList.push_back(strPath);
		m_MatchList.InsertItem(m_vecImgPathInList.size() - 1, FunGetFileName(strPath,false));
	}
}

void CMyDockPaneWndData::ClearMatchList()
{
	vector<CString>().swap(m_vecImgPathInList);
	m_MatchList.DeleteAllItems();
}

eDOMTYPE CMyDockPaneWndData::HighlightSelectedItem(int nItemData)
{
	if (m_nHighlight >= 0)
	{
	//	m_ImgTree.SetItemState(m_vecGroupItem[m_nHighlight], 0, TVGN_DROPHILITE);
		m_ImgTree.SetItemImage(m_vecGroupItem[m_nHighlight], CloseGroupImgIdx, CloseGroupImgIdx);
		m_nHighlight = -1;
	}
	if (nItemData >= 0 && nItemData < m_vecGroupItem.size())
	{
	//	m_ImgTree.SetItemState(m_vecGroupItem[nItemData], TVGN_DROPHILITE, TVGN_DROPHILITE);
		m_ImgTree.SetItemImage(m_vecGroupItem[nItemData], OpenGroupImgIdx, OpenGroupImgIdx);
		m_nHighlight = nItemData;
		HTREEITEM hChild = m_ImgTree.GetChildItem(m_vecGroupItem[nItemData]);
		int nItemImgIdx;
		while (hChild)
		{
			m_ImgTree.GetItemImage(hChild, nItemImgIdx, nItemImgIdx);
			if (nItemImgIdx == SelectedImgIdx)
			{
				CString strText = m_ImgTree.GetItemText(hChild);
				int nItemData = m_ImgTree.GetItemData(hChild);
				int nChildData = nItemData%m_vecGroupItem.size() + 1;
				return eDOMTYPE(nChildData);
			}
			hChild = m_ImgTree.GetNextSiblingItem(hChild);
		}
		m_ImgTree.SetItemImage(m_ImgTree.GetChildItem(m_vecGroupItem[nItemData]), SelectedImgIdx, SelectedImgIdx);
	//	theApp.m_nRefIdx = 0;
	}
	return PAN;
}



BEGIN_MESSAGE_MAP(CMyDockPaneWndData, CMyDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_WND_IMAGE_DATA, &CMyDockPaneWndData::OnDBClickImgTree)
	ON_NOTIFY(NM_CLICK, IDC_LIST_WND_IMAGE_DATA, &CMyDockPaneWndData::OnClickImgTree)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_MATCH, &CMyDockPaneWndData::OnDBClickMatchList)
END_MESSAGE_MAP()



// CMyDockPaneWndData message handlers




int CMyDockPaneWndData::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMyDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	CRect rectDummy; rectDummy.SetRectEmpty();
	DWORD dwTreeStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS /*| TVS_CHECKBOXES*/;
	m_ImgTree.Create(dwTreeStyle, rectDummy, this, IDC_LIST_WND_IMAGE_DATA);
	CImageList *imagelist = new CImageList;
	imagelist->Create(16, 16, ILC_COLOR32 | ILC_MASK, 7, 7);//16*16的图标  
	CBitmap cBmp; cBmp.LoadBitmap(IDB_BITMAP1);
	imagelist->Add(&cBmp, RGB(255, 255, 255));
	m_ImgTree.SetImageList(imagelist, TVSIL_NORMAL);

	CRect rect; rect.SetRectEmpty();
	DWORD 	dwStyle = LVS_ALIGNTOP | LVS_REPORT | WS_CHILD | WS_VISIBLE | WS_BORDER;
	if (!m_MatchList.Create(dwStyle, rect, this, IDC_LIST_MATCH))
	{
		AfxMessageBox("待匹配影像列表构建失败！");
		return -1;
	}
	dwStyle |= LVS_EX_FULLROWSELECT;//选中某行使整行高亮（只适用与report风格的listctrl）
									//	dwStyle |= LVS_EX_GRIDLINES;//网格线（只适用与report风格的listctrl）
	dwStyle |= LVS_SHOWSELALWAYS;//一直选中item
	m_MatchList.SetExtendedStyle(dwStyle);
	m_MatchList.InsertColumn(0, "待匹配影像", LVCFMT_CENTER, rect.Width(), -1);

	return 0;
}


void CMyDockPaneWndData::OnSize(UINT nType, int cx, int cy)
{
	CMyDockablePane::OnSize(nType, cx, cy);
	m_ImgTree.SetWindowPos(NULL, -1, -1, cx, cy*3/5, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
	m_MatchList.SetWindowPos(NULL, -1, cy * 3 / 5, cx, cy *2 / 5, SWP_NOACTIVATE | SWP_NOZORDER);
	m_MatchList.SetColumnWidth(0, cx);
	// TODO: Add your message handler code here
}

void CMyDockPaneWndData::OnDBClickImgTree(NMHDR * pNMHDR, LRESULT * pResult)
{
	CPoint point; GetCursorPos(&point);//获得鼠标点击的位置
	m_ImgTree.ScreenToClient(&point);//转化为客户坐标
	UINT uFlags;  HTREEITEM CurrentItem = m_ImgTree.HitTest(point, &uFlags);//获得当前点击节点的ITEM
	if (!CurrentItem) return;
	int nItemData = m_ImgTree.GetItemData(CurrentItem);
	if (nItemData >= 0 && nItemData < m_vecGroupItem.size())
	{
	//	m_ImgTree.Expand(CurrentItem, TVE_COLLAPSE);
		::SendMessage(GetMainFramHand()->m_hWnd, WM_OPEN_MATCH_VIEW, (WPARAM)&nItemData, 0);
	}
}

void CMyDockPaneWndData::OnClickImgTree(NMHDR * pNMHDR, LRESULT * pResult)
{
	CPoint point; GetCursorPos(&point);//获得鼠标点击的位置
	m_ImgTree.ScreenToClient(&point);//转化为客户坐标
	UINT uFlags;  HTREEITEM CurrentItem = m_ImgTree.HitTest(point, &uFlags);//获得当前点击节点的ITEM
	if (!CurrentItem) return;
	HTREEITEM hChild = m_ImgTree.GetChildItem(CurrentItem);
	if (!hChild)
	{
		int nImgIdx; m_ImgTree.GetItemImage(CurrentItem, nImgIdx, nImgIdx);
		if (nImgIdx == SelectedImgIdx)  //点击已选中DOM
		{
			return;
		}
		else  //切换选中DOM
		{
			HTREEITEM hParent = m_ImgTree.GetParentItem(CurrentItem);
			//设置hParent子节点不选中
			HTREEITEM hChild2 = m_ImgTree.GetChildItem(hParent);

			while (hChild2)
			{
				m_ImgTree.SetItemImage(hChild2, UnSelectedImgIdx, UnSelectedImgIdx);
				hChild2 = m_ImgTree.GetNextSiblingItem(hChild2);
			}
			//选中当前节点
			m_ImgTree.SetItemImage(CurrentItem, SelectedImgIdx, SelectedImgIdx);

			if (theApp.GetOpenDocumentCount() == 2)  //已打开匹配窗口
			{
				int nGroupIdx = m_ImgTree.GetItemData(hParent);
				if (nGroupIdx == m_nHighlight) //当前group
				{
					int nCurrentData = m_ImgTree.GetItemData(CurrentItem);
		//			GetMainFramHand()->m_Project.SavePoint();
					::SendMessage(GetMainFramHand()->m_hWnd, WM_UPDATE_MATCHVIEW, nCurrentData%m_vecGroupItem.size() + 1, 0);
				}
			}
		}
	}

}

void CMyDockPaneWndData::OnDBClickMatchList(NMHDR * pNMHDR, LRESULT * pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	int item = pNMItemActivate->iItem;
	if (item < 0) return;
	GetMainFramHand()->AutoMatch(m_vecImgPathInList[item]);
}
