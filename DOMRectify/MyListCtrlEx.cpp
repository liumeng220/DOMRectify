//*************************************************************************
// MyListCtrlEx.cpp : header file
// Version : 1.0
// Date : June 2005
// Author : Chen Hao
// Email :  ch@sothink.com
// Website : http://www.sothink.com
// 
// You are free to use/modify this code but leave this header intact. 
// This class is public domain so you are free to use it any of 
// your applications (Freeware,Shareware,Commercial). All I ask is 
// that you let me know that some of my code is in your app.
//*************************************************************************


#include "stdafx.h"
//#include "TestListCtrl.h"
#include ".\MyListCtrlEx.h"


// CMyListCtrlEx

IMPLEMENT_DYNAMIC(CMyListCtrlEx, CListCtrl)
CMyListCtrlEx::CMyListCtrlEx()
{
	m_nEditCol=-1;
	m_nEditCol=-1;
}

CMyListCtrlEx::~CMyListCtrlEx()
{
}


BEGIN_MESSAGE_MAP(CMyListCtrlEx, CListCtrl)
	ON_NOTIFY(NM_RCLICK, 0, OnNmRclickHeader)	// this map is added manually.
	ON_NOTIFY(HDN_DIVIDERDBLCLICKA, 0, OnHdnDividerdblclick)
	ON_NOTIFY(HDN_DIVIDERDBLCLICKW, 0, OnHdnDividerdblclick)
	ON_NOTIFY(HDN_ENDDRAG, 0, OnHdnEnddrag)
//	ON_NOTIFY_REFLECT(NM_CLICK, &CMyListCtrlEx::OnNMClick)
END_MESSAGE_MAP()



// CMyListCtrlEx message handlers


void CMyListCtrlEx::PreSubclassWindow()
{
	// TODO: Add your specialized code here and/or call the base class

	HWND hWnd = ::GetWindow(m_hWnd, GW_CHILD);
	ASSERT(hWnd);
	if (hWnd)
		m_wndHeader.SubclassWindow(hWnd);

	CListCtrl::PreSubclassWindow();
}

LRESULT CMyListCtrlEx::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: Add your specialized code here and/or call the base class

	switch (message)
	{
		case LVM_INSERTCOLUMN:
		{
			LRESULT ret = CListCtrl::WindowProc(message, wParam, lParam);
			if ((int)ret >= 0)
			{
				LVCOLUMN *pCol = (LVCOLUMN *)lParam;				
				CMyHeaderCtrlEx::CItemData *pData = new CMyHeaderCtrlEx::CItemData(pCol->cx, TRUE, TRUE);
				m_wndHeader.SetItemData((int)ret, (DWORD_PTR)pData);
			}
			return ret;
		}

		case LVM_DELETECOLUMN:
		{
			CMyHeaderCtrlEx::CItemData *pData = (CMyHeaderCtrlEx::CItemData *)m_wndHeader.GetItemData((int)wParam);
			ASSERT(pData);
			LRESULT ret = CListCtrl::WindowProc(message, wParam, lParam);
			if ((BOOL)ret && pData)
				delete pData;
			return ret;
		}
	}

	return CListCtrl::WindowProc(message, wParam, lParam);
}

void CMyListCtrlEx::OnNmRclickHeader(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);

	// TODO: Add your control notification handler code here

#ifdef _DEBUG
	int count = m_wndHeader.GetItemCount();
	int *piArray = new int[count];
	m_wndHeader.GetOrderArray(piArray, count);
	for (int i = 0; i < count; i++)
	{
		TRACE("%d", piArray[i]);
		if (!m_wndHeader.GetVisible(piArray[i]))
			TRACE("*");
		TRACE(" ");
	}
	TRACE("\n");
	delete[] piArray;
#endif
	
	*pResult = 0;

	CPoint point;
	GetCursorPos(&point);

	CMenu menu;
	if (menu.CreatePopupMenu())
	{
		const int TEXT_LEN = 64;
		const TCHAR TEXT_TAIL[] = "...";

		TCHAR szText[TEXT_LEN + sizeof(TEXT_TAIL) - 1];
		HDITEM hdi;
		hdi.mask = HDI_TEXT;
		hdi.pszText = szText;
		hdi.cchTextMax = TEXT_LEN;
		int count = m_wndHeader.GetItemCount();
		for (int i = 0; i < count; i++)
		{
			if (!m_wndHeader.GetItem(i, &hdi))
				return;

			if (hdi.cchTextMax == TEXT_LEN - 1)
				_tcscat(szText, TEXT_TAIL);

			UINT nFlags = MF_STRING;
			if (!m_wndHeader.GetRemovable(i))
				nFlags |= MF_GRAYED | MF_CHECKED;			
			if (m_wndHeader.GetVisible(i))
				nFlags |= MF_CHECKED;
			if (!menu.AppendMenu(nFlags, i + 1, szText))
				return;
		}

		UINT index = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, point.x, point.y, this);
		if (index > 0)
		{
			index--;
			BOOL bVisible = m_wndHeader.GetVisible(index);
			m_wndHeader.SetVisible(index, !bVisible);
		}
	}
}

void CMyListCtrlEx::OnHdnEnddrag(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);

	// TODO: Add your control notification handler code here

	m_wndHeader.PostMessage(WM_HDN_ENDDRAG, 0, 0L);

	*pResult = 0;
}

void CMyListCtrlEx::OnHdnDividerdblclick(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);

	// TODO: Add your control notification handler code here

	*pResult = 1;

	int nItem = m_wndHeader.FindVisibleItem(phdr->iItem);
	if (nItem >= 0)
	{
		int count = GetItemCount();
		if (count > 0)
		{
			CClientDC dc(this);
			CFont *pOldFont = dc.SelectObject(GetFont());

			int nMaxWidth = 0;
			CString str;
			for (int i = 0; i < count; i++)
			{
				str = GetItemText(i, nItem);
				int w = GetStringWidth(str);
				nMaxWidth = max(nMaxWidth, w + 12);
			}

			CImageList *pImgList = GetImageList(LVSIL_SMALL);
			if (pImgList != NULL && nItem == 0)
			{
				int cx, cy;
				ImageList_GetIconSize(pImgList->m_hImageList, &cx, &cy);
				nMaxWidth += cx;
			}
			SetColumnWidth(nItem, nMaxWidth);

			dc.SelectObject(pOldFont);
		}
	}
}

BOOL CMyListCtrlEx::SaveState()
{
	ASSERT(!m_strKey.IsEmpty());
	if (m_strKey.IsEmpty())
		return FALSE;

	int count = m_wndHeader.GetItemCount();
	AfxGetApp()->WriteProfileInt(m_strKey, "Column Count", count);

	int *nColOrder = new int[count];
	m_wndHeader.GetOrderArray(nColOrder, count);
	AfxGetApp()->WriteProfileBinary(m_strKey, "Column Order", (BYTE *)nColOrder, sizeof(int) * count);
	delete[] nColOrder;

	int *nColWidth = new int[count];
	m_wndHeader.GetWidthArray(nColWidth, count);
	AfxGetApp()->WriteProfileBinary(m_strKey, "Column Width", (BYTE *)nColWidth, sizeof(int) * count);
	delete[] nColWidth;

	int *nColVisible = new int[count];
	m_wndHeader.GetVisibleArray(nColVisible, count);
	AfxGetApp()->WriteProfileBinary(m_strKey, "Column Visible", (BYTE *)nColVisible, sizeof(int) * count);
	delete[] nColVisible;

	return TRUE;
}

BOOL CMyListCtrlEx::RestoreState()
{
	ASSERT(!m_strKey.IsEmpty());
	if (m_strKey.IsEmpty())
		return FALSE;

	int count = AfxGetApp()->GetProfileInt(m_strKey, "Column Count", 0);
	if (count != m_wndHeader.GetItemCount())
		return FALSE;

	UINT bytes;

	int *nColOrder = NULL;
	BOOL ret = FALSE;
	if ((AfxGetApp()->GetProfileBinary(m_strKey, "Column Order", (BYTE **)&nColOrder, &bytes) && bytes == sizeof(int) * count))
	{
		ret = VerifyOrderArray(nColOrder, count) && m_wndHeader.SetOrderArray(count, nColOrder);
		ASSERT(ret);
	}
	delete[] nColOrder;

	if (!ret)
		return FALSE;

	int *nColWidth = NULL;
	ret = FALSE;
	if ((AfxGetApp()->GetProfileBinary(m_strKey, "Column Width", (BYTE **)&nColWidth, &bytes) && bytes == sizeof(int) * count))
		ret = m_wndHeader.SetWidthArray(count, nColWidth);
	delete[] nColWidth;
	if (!ret)
		return FALSE;

	int *nColVisible = NULL;
	ret = FALSE;
	if ((AfxGetApp()->GetProfileBinary(m_strKey, "Column Visible", (BYTE **)&nColVisible, &bytes) && bytes == sizeof(int) * count))
		ret = m_wndHeader.SetVisibleArray(count, nColVisible);
	delete[] nColVisible;

	return ret;
}

BOOL CMyListCtrlEx::VerifyOrderArray(int *piArray, int count)
{
	for (int i = 0; i < count; i++)
	{
		if (!(piArray[i] >= 0 && piArray[i] <= count - 1))
			return FALSE;

		// compare with items after current one
		for (int j = i + 1; j < count; j++)
			if (piArray[i] == piArray[j])
				return FALSE;
	}
	return TRUE;
}

void CMyListCtrlEx::SetRegistryKey(LPCTSTR lpszKey)
{
	m_strKey = lpszKey;
}

void CMyListCtrlEx::SetEditItem(int nRow, int nCol)
{
	if(nRow>=GetItemCount()||nCol>=m_wndHeader.GetItemCount()) return;
	m_nEditCol=nCol;
	m_nEditRow=nRow;
	if (m_EditItem.m_hWnd == NULL)
	{
		CRect Rect = CRect(0, 0, 100, 20);
		m_EditItem.Create(WS_CHILD | ES_LEFT | WS_BORDER | ES_AUTOHSCROLL | ES_WANTRETURN | ES_MULTILINE, Rect, this, 0);
		m_EditItem.SetFont(GetFont(), FALSE);
	}
	CRect RectSubItem; GetSubItemRect(m_nEditRow, m_nEditCol, LVIR_LABEL, RectSubItem);
	CString strItem = GetItemText(m_nEditRow, m_nEditCol);
	m_EditItem.SetWindowText(strItem);
	m_EditItem.MoveWindow(RectSubItem);
	m_EditItem.ShowWindow(TRUE);
	m_EditItem.SetFocus();
}
void CMyListCtrlEx::UpdateEditItem()
{
//	if(m_nEditCol==-1||m_nEditRow==-1) return;
	m_EditItem.ShowWindow(FALSE);
	CString strEdit; m_EditItem.GetWindowText(strEdit);
	if (strEdit.IsEmpty()) strEdit = "-1";
	SetItemText(m_nEditRow, m_nEditCol, strEdit);
	m_nEditRow = -1;
	m_nEditCol = -1;
	m_EditItem.SetWindowText("");
}
bool CMyListCtrlEx::IsEditItem()
{
	if(m_nEditCol!=-1&&m_nEditRow!=-1) return true;
	else return false;
}


// CMyListCtrlReDraw
IMPLEMENT_DYNAMIC(CMyListCtrlReDraw, CListCtrl)
	CMyListCtrlReDraw::CMyListCtrlReDraw()
{
	LIST_ITEM_HEIGHT = 22;
	LIST_HEAD_HEIGHT = 15;
}
CMyListCtrlReDraw::~CMyListCtrlReDraw()
{
}

BEGIN_MESSAGE_MAP(CMyListCtrlReDraw, CListCtrl)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, &CMyListCtrlReDraw::OnNMCustomdraw)
	ON_WM_MEASUREITEM_REFLECT()
END_MESSAGE_MAP()

// CMyListCtrlReDraw message handlers 
void CMyListCtrlReDraw::OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVCUSTOMDRAW lpnmcd = (LPNMLVCUSTOMDRAW)pNMHDR;
	if (lpnmcd->nmcd.dwDrawStage == CDDS_PREPAINT)
	{
		*pResult = CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYPOSTPAINT;
		return;
	}
	else if (lpnmcd->nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
	{
		*pResult = CDRF_NOTIFYSUBITEMDRAW;
		return;
	}
	else if (lpnmcd->nmcd.dwDrawStage == CDDS_POSTPAINT)
	{
		DrawRemainSpace(lpnmcd);
		*pResult = CDRF_SKIPDEFAULT;
		return;
	}
	//else if (lpnmcd->nmcd.dwDrawStage == (CDDS_SUBITEM | CDDS_ITEMPREPAINT))
	//{
	//	int iItem = lpnmcd->nmcd.dwItemSpec;
	//	int iSubItem = lpnmcd->iSubItem;
	//	if (iItem >= 0 && iSubItem >= 0)
	//	{
	//		CRect rSubItem;
	//		HDC hDC = lpnmcd->nmcd.hdc;
	//		GetSubItemRect(iItem, iSubItem, LVIR_LABEL, rSubItem);
	//		if (iSubItem == 0)
	//		{
	//			rSubItem.left = 0;
	//		}
	//		bool bSelected = false;
	//		if (GetItemState(iItem, LVIS_SELECTED))
	//		{
	//			bSelected = true;
	//		}
	//		bool bFocus = false;
	//		CWnd *pWndFocus = GetFocus();
	//		if (IsChild(pWndFocus) || pWndFocus == this)
	//		{
	//			bFocus = true;
	//		}
	//		rSubItem.NormalizeRect();
	//		CDC dc;
	//		dc.Attach(lpnmcd->nmcd.hdc);
	//		DrawSubItem(&dc, iItem, iSubItem, rSubItem, bSelected, bFocus);
	//		dc.Detach();
	//		*pResult = CDRF_SKIPDEFAULT;
	//		return;
	//	}
	//}


	*pResult = 0;
}
// overwrite:  
void CMyListCtrlReDraw::DrawSubItem(CDC *pDC, int nItem, int nSubItem, CRect &rSubItem, bool bSelected, bool bFocus)
{
	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextColor(RGB(0, 0, 0));
	CFont font;
	font.CreateFont(13,   // nHeight  
		0,                         // nWidth  
		0,                         // nEscapement  
		0,                         // nOrientation  
		FW_NORMAL,                 // nWeight  
		FALSE,                     // bItalic  
		FALSE,                     // bUnderline  
		0,                         // cStrikeOut  
		ANSI_CHARSET,              // nCharSet  
		OUT_DEFAULT_PRECIS,        // nOutPrecision  
		CLIP_DEFAULT_PRECIS,       // nClipPrecision  
		DEFAULT_QUALITY,           // nQuality  
		DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily  
		_T("宋体"));
	pDC->SelectObject(&font);
	CString strText;
	strText = GetItemText(nItem, nSubItem);
	draw_row_bg(pDC, rSubItem, bSelected, bFocus, nItem);
	pDC->DrawText(strText, strText.GetLength(), &rSubItem, DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS);

}
// 画剩余部分  
void CMyListCtrlReDraw::DrawRemainSpace(LPNMLVCUSTOMDRAW lpnmcd)
{
	int nTop = lpnmcd->nmcd.rc.top;
	int nCount = GetItemCount();
	if (nCount > 0)
	{
		CRect rcItem;
		GetItemRect(nCount - 1, &rcItem, LVIR_LABEL);
		nTop = rcItem.bottom;
	}
	CRect rectClient;
	GetClientRect(&rectClient);
	if (nTop < lpnmcd->nmcd.rc.bottom) // 有剩余  
	{
		CRect rcRemain = lpnmcd->nmcd.rc;
		rcRemain.top = nTop;
		rcRemain.right = rectClient.right;
		int nRemainItem = rcRemain.Height() / LIST_ITEM_HEIGHT;
		if (rcRemain.Height() % LIST_ITEM_HEIGHT)
		{
			nRemainItem++;
		}
		int pos = GetScrollPos(SB_HORZ);
		CDC dc;
		dc.Attach(lpnmcd->nmcd.hdc);
		for (int i = 0; i < nRemainItem; ++i)
		{
			CRect rcItem;
			rcItem.top = rcRemain.top + i * LIST_ITEM_HEIGHT;
			rcItem.left = rcRemain.left;
			rcItem.right = rcRemain.right;
			rcItem.bottom = rcItem.top + LIST_ITEM_HEIGHT;
			int nColumnCount = GetHeaderCtrl()->GetItemCount();
			CRect  rcSubItem;
			for (int j = 0; j < nColumnCount; ++j)
			{
				GetHeaderCtrl()->GetItemRect(j, &rcSubItem);
				rcSubItem.top = rcItem.top;
				rcSubItem.bottom = rcItem.bottom;
				rcSubItem.OffsetRect(-pos, 0);
				if (rcSubItem.right < rcRemain.left || rcSubItem.left > rcRemain.right)
					continue;
				draw_row_bg(&dc, rcSubItem, false, false, i + nCount);
			}
		}
		dc.Detach();
	}
}
void CMyListCtrlReDraw::draw_row_bg(CDC *pDC, RECT rc, bool bSelected, bool bFocus, int nRow)
{
	bool bOdd = (nRow % 2 == 0 ? true : false);
	CRect rect = rc;
	if (rect.Width() == 0)
	{
		return;
	}

	draw_row_bg(pDC, rc, bSelected, bFocus, bOdd);
}
void CMyListCtrlReDraw::draw_row_bg(CDC *pDC, RECT rc, bool bSelected, bool bFocus, bool bOdd)
{
	CRect rect = rc;
	if (rect.Width() == 0)
	{
		return;
	}
	int nSave = pDC->SaveDC();
	if (bSelected)
	{
		if (bFocus)
		{
			CBrush selectBrush;
			selectBrush.CreateSolidBrush(RGB(203, 223, 239));
			pDC->FillRect(&rc, &selectBrush);
		}
		else
		{
			CBrush selectNoFocusBrush;
			selectNoFocusBrush.CreateSolidBrush(RGB(206, 206, 206));
			pDC->FillRect(&rc, &selectNoFocusBrush);
		}
	}
	else if (bOdd)
	{
		CBrush oddBrush;
		oddBrush.CreateSolidBrush(RGB(255, 255, 255));
		pDC->FillRect(&rc, &oddBrush);
	}
	else
	{
		CBrush normalBrush;
		normalBrush.CreateSolidBrush(RGB(243, 243, 243));
		pDC->FillRect(&rc, &normalBrush);
	}

	// 画竖线  
	CPen pen;
	pen.CreatePen(PS_SOLID, 1, RGB(218, 218, 218));
	pDC->SelectObject(&pen);
	pDC->MoveTo(rc.right - 1, rc.top);
	pDC->LineTo(rc.right - 1, rc.bottom);
	// 画选中的底部分割线  
	if (bSelected)
	{
		CPen bottomPen;
		bottomPen.CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
		pDC->SelectObject(&bottomPen);
		pDC->MoveTo(rc.left, rc.bottom - 1);
		pDC->LineTo(rc.right, rc.bottom - 1);
	}
	pDC->RestoreDC(nSave);
}
void CMyListCtrlReDraw::InitCustomdraw()
{
	LOGFONT logfont;
	memset(&logfont, 0, sizeof(logfont));
	logfont.lfWeight = FW_NORMAL;
	logfont.lfCharSet = GB2312_CHARSET;
	_tcscpy_s(logfont.lfFaceName, LF_FACESIZE, _T("宋体"));
	logfont.lfHeight = -(LIST_HEAD_HEIGHT - 1);
	CFont font;
	font.CreateFontIndirect(&logfont);
	SetFont(&font);
	font.Detach();
}
void CMyListCtrlReDraw::OnLvnItemchanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	if (pNMLV->uChanged & LVIF_STATE)
	{
		if (((pNMLV->uOldState & LVIS_SELECTED) != (pNMLV->uNewState & LVIS_SELECTED))
			|| ((pNMLV->uOldState & LVIS_STATEIMAGEMASK) != (pNMLV->uNewState & LVIS_STATEIMAGEMASK)))
		{
			InvalidateItem(pNMLV->iItem);
		}
	}
	*pResult = 0;
}
void CMyListCtrlReDraw::InvalidateItem(int nItem)
{
	CRect rcClient;
	GetClientRect(&rcClient);
	CRect rcItem;
	GetItemRect(nItem, &rcItem, LVIR_BOUNDS);
	rcItem.left = rcClient.left;
	rcItem.right = rcClient.right;
	InvalidateRect(&rcItem, FALSE);
}
void CMyListCtrlReDraw::InitDrawItem()
{
	CRect rcwin;
	GetWindowRect(rcwin);
	WINDOWPOS wp;
	wp.hwnd = m_hWnd;
	wp.cx = rcwin.Width();
	wp.cy = rcwin.Height();
	wp.flags = SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER;
	SendMessage(WM_WINDOWPOSCHANGED, 0, (LPARAM)&wp);
}
void CMyListCtrlReDraw::PreSubclassWindow()
{
	InitCustomdraw();
	InitDrawItem();
	CListCtrl::PreSubclassWindow();
}

void CMyListCtrlReDraw::DrawItem(LPDRAWITEMSTRUCT lpMeasureItemStruct)
{
	CDC* pDC = CDC::FromHandle(lpMeasureItemStruct->hDC);
	LVITEM lvi = { 0 };
	lvi.mask = LVIF_STATE;//|LVIF_IMAGE; 
	lvi.stateMask = LVIS_FOCUSED | LVIS_SELECTED;
	lvi.iItem = lpMeasureItemStruct->itemID;
	BOOL bGet = GetItem(&lvi);
	//高亮显示
	BOOL bHighlight = ((lvi.state & LVIS_DROPHILITED) || ((lvi.state & LVIS_SELECTED) &&
		((GetFocus() == this) || (GetStyle() & LVS_SHOWSELALWAYS))));
	// 画文本背景 
	CRect rcBack = lpMeasureItemStruct->rcItem;
	pDC->SetBkMode(TRANSPARENT);
	if (bHighlight) //如果被选中
	{
		pDC->SetTextColor(RGB(255, 255, 255)); //文本为白色
		pDC->FillRect(rcBack, &CBrush(RGB(90, 162, 0)));

		CPen bottomPen;
		bottomPen.CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
		pDC->SelectObject(&bottomPen);
		pDC->MoveTo(rcBack.left, rcBack.bottom - 1);
		pDC->LineTo(rcBack.right, rcBack.bottom - 1);

	}
	else if (lvi.iItem%2)// RGB(203, 223, 239) RGB(206, 206, 206)
	{
		pDC->SetTextColor(RGB(0, 0, 0));       //文本为黑色
		pDC->FillRect(rcBack, &CBrush(RGB(243, 243, 243)));
	}
	else
	{
		pDC->SetTextColor(RGB(0, 0, 0));       //文本为黑色
		pDC->FillRect(rcBack, &CBrush(RGB(255, 255, 255)));

	}
	if (lpMeasureItemStruct->itemAction & ODA_DRAWENTIRE)
	{
		//写文本 
		CString szText;
		int nCollumn = GetHeaderCtrl()->GetItemCount();//列数
		for (int i = 0; i < GetHeaderCtrl()->GetItemCount(); i++)
		{ //循环得到文本 
			CRect rcItem;
			if (!GetSubItemRect(lpMeasureItemStruct->itemID, i, LVIR_LABEL, rcItem))
				continue;
			szText = GetItemText(lpMeasureItemStruct->itemID, i);
			rcItem.left += 5; rcItem.right -= 1;
			pDC->DrawText(szText, lstrlen(szText), &rcItem, DT_LEFT | DT_VCENTER | DT_NOPREFIX | DT_SINGLELINE);
		}
	}
}
void CMyListCtrlReDraw::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	lpMeasureItemStruct->itemHeight = LIST_ITEM_HEIGHT;
}