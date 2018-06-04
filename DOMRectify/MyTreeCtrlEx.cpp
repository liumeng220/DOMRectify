// TreeCtrlEx.cpp : implementation file
//

#include "stdafx.h"
#include "DOMRectify.h"
#include "MyTreeCtrlEx.h"
IMPLEMENT_DYNAMIC(CTreeCtrlEx, CTreeCtrl)
IMPLEMENT_DYNAMIC(CDirTreeCtrl, CTreeCtrlEx)
// CDirTreeCtrl message handlers  
CDirTreeCtrl::CDirTreeCtrl()
{
}

CDirTreeCtrl::~CDirTreeCtrl()
{
	m_imgList.Detach();
}

/************************************************************************/
/*                                                                      */
/************************************************************************/

#ifdef _DEBUG  
#define new DEBUG_NEW  
#undef THIS_FILE  
static char THIS_FILE[] = __FILE__;
#endif  

#define TCEX_EDITLABEL 1        // Edit label timer event  

/////////////////////////////////////////////////////////////////////////////  
// CTreeCtrlEx  

BEGIN_MESSAGE_MAP(CTreeCtrlEx, CTreeCtrl)
	//{{AFX_MSG_MAP(CTreeCtrlEx)  
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_KEYDOWN()
	ON_NOTIFY_REFLECT_EX(TVN_ITEMEXPANDING, OnItemexpanding)
	ON_NOTIFY_REFLECT_EX(NM_SETFOCUS, OnSetfocus)
	ON_NOTIFY_REFLECT_EX(NM_KILLFOCUS, OnKillfocus)
	ON_NOTIFY_REFLECT_EX(TVN_SELCHANGED, OnSelchanged)
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP  
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

//IMPLEMENT_DYNAMIC(CTreeCtrlEx, CTreeCtrl)  

BOOL CTreeCtrlEx::Create(DWORD dwStyle, DWORD dwExStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
#if _MFC_VER < 0x0700  
	return CreateEx(dwExStyle, WC_TREEVIEW, NULL, dwStyle,
		rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
		pParentWnd->GetSafeHwnd(), (HMENU)nID);
#else  
	return CTreeCtrl::CreateEx(dwExStyle, dwStyle, rect, pParentWnd, nID);
#endif  
}

BOOL CTreeCtrlEx::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	return CTreeCtrl::Create(dwStyle, rect, pParentWnd, nID);
}


/////////////////////////////////////////////////////////////////////////////  
// CTreeCtrlEx message handlers  


///////////////////////////////////////////////////////////////////////////////  
// The tree control dosn't support multiple selection. However we can simulate   
// it by taking control of the left mouse click and arrow key press before the  
// control gets them, and setting/clearing the TVIS_SELECTED style on the items  

void CTreeCtrlEx::OnLButtonDown(UINT nFlags, CPoint point)
{

	//UINT nHitFlags = 0;
	//HTREEITEM hClickedItem = HitTest(point, &nHitFlags);

	//GetMainFramHand()->m_wndLayerlist.m_wndTree_ShowLayer.NMClickWork(hClickedItem, nHitFlags);

	//// Must invoke label editing explicitly. The base class OnLButtonDown would normally  
	//// do this, but we can't call it here because of the multiple selection...  
	//if (!(nFlags&(MK_CONTROL | MK_SHIFT)) && (GetStyle() & TVS_EDITLABELS) && (nHitFlags & TVHT_ONITEMLABEL))
	//if (hClickedItem == GetSelectedItem())
	//{
	//	// Clear multple selection before label editing  
	//	ClearSelection();
	//	SelectItem(hClickedItem);

	//	// Invoke label editing  
	//	m_bEditLabelPending = TRUE;
	//	m_idTimer = (UINT)SetTimer(TCEX_EDITLABEL, GetDoubleClickTime(), NULL);

	//	return;
	//}

	//m_bEditLabelPending = FALSE;

	//if (nHitFlags & TVHT_ONITEM)
	//{
	//	SetFocus();

	//	m_hClickedItem = hClickedItem;

	//	// Is the clicked item already selected ?  
	//	BOOL bIsClickedItemSelected = GetItemState(hClickedItem, TVIS_SELECTED) & TVIS_SELECTED;

	//	if (bIsClickedItemSelected)
	//	{
	//		// Maybe user wants to drag/drop multiple items!  
	//		// So, wait until OnLButtonUp() to do the selection stuff.   
	//		m_bSelectPending = TRUE;
	//	}
	//	else
	//	{
	//		SelectMultiple(hClickedItem, nFlags, point);
	//		m_bSelectPending = FALSE;
	//	}

	//	m_ptClick = point;
	//}
	//else
	CTreeCtrl::OnLButtonDown(nFlags, point);

}

void CTreeCtrlEx::OnLButtonUp(UINT nFlags, CPoint point)
{
// 	if (m_bSelectPending)
// 	{
// 		// A select has been waiting to be performed here  
// 		SelectMultiple(m_hClickedItem, nFlags, point);
// 		m_bSelectPending = FALSE;
// 	}
// 
// 	m_hClickedItem = NULL;

	CTreeCtrl::OnLButtonUp(nFlags, point);
}


void CTreeCtrlEx::OnMouseMove(UINT nFlags, CPoint point)
{
	// If there is a select pending, check if cursor has moved so much away from the   
	// down-click point that we should cancel the pending select and initiate  
	// a drag/drop operation instead!  

	if (m_hClickedItem)
	{
		CSize sizeMoved = m_ptClick - point;

		if (abs(sizeMoved.cx) > GetSystemMetrics(SM_CXDRAG) || abs(sizeMoved.cy) > GetSystemMetrics(SM_CYDRAG))
		{
			m_bSelectPending = FALSE;

			// Notify parent that he may begin drag operation  
			// Since we have taken over OnLButtonDown(), the default handler doesn't  
			// do the normal work when clicking an item, so we must provide our own  
			// TVN_BEGINDRAG notification for the parent!  

			CWnd* pWnd = GetParent();
			if (pWnd && !(GetStyle() & TVS_DISABLEDRAGDROP))
			{
				NM_TREEVIEW tv;

				tv.hdr.hwndFrom = GetSafeHwnd();
				tv.hdr.idFrom = GetWindowLong(GetSafeHwnd(), GWL_ID);
				tv.hdr.code = TVN_BEGINDRAG;

				tv.itemNew.hItem = m_hClickedItem;
				tv.itemNew.state = GetItemState(m_hClickedItem, 0xffffffff);
				tv.itemNew.lParam = GetItemData(m_hClickedItem);

				tv.ptDrag.x = point.x;
				tv.ptDrag.y = point.y;

				pWnd->SendMessage(WM_NOTIFY, tv.hdr.idFrom, (LPARAM)&tv);
			}

			m_hClickedItem = NULL;
		}
	}

	CTreeCtrl::OnMouseMove(nFlags, point);
}


void CTreeCtrlEx::SelectMultiple(HTREEITEM hClickedItem, UINT nFlags, CPoint point)
{
	// Start preparing an NM_TREEVIEW struct to send a notification after selection is done  
	NM_TREEVIEW tv;
	memset(&tv.itemOld, 0, sizeof(tv.itemOld));

	CWnd* pWnd = GetParent();

	HTREEITEM hOldItem = GetSelectedItem();

	if (hOldItem)
	{
		tv.itemOld.hItem = hOldItem;
		tv.itemOld.state = GetItemState(hOldItem, 0xffffffff);
		tv.itemOld.lParam = GetItemData(hOldItem);
		tv.itemOld.mask = TVIF_HANDLE | TVIF_STATE | TVIF_PARAM;
	}

	// Flag signaling that selection process is NOT complete.  
	// (Will prohibit TVN_SELCHANGED from being sent to parent)  
	m_bSelectionComplete = FALSE;

	// Action depends on whether the user holds down the Shift or Ctrl key  
	if (nFlags & MK_SHIFT)
	{
		// Select from first selected item to the clicked item  
		if (!m_hFirstSelectedItem)
			m_hFirstSelectedItem = GetSelectedItem();

		SelectItems(m_hFirstSelectedItem, hClickedItem);
	}
	else if (nFlags & MK_CONTROL)
	{
		// Find which item is currently selected  
		HTREEITEM hSelectedItem = GetSelectedItem();

		// Is the clicked item already selected ?  
		BOOL bIsClickedItemSelected = GetItemState(hClickedItem, TVIS_SELECTED) & TVIS_SELECTED;
		BOOL bIsSelectedItemSelected = FALSE;
		if (hSelectedItem)
			bIsSelectedItemSelected = GetItemState(hSelectedItem, TVIS_SELECTED) & TVIS_SELECTED;

		// Must synthesize a TVN_SELCHANGING notification  
		if (pWnd)
		{
			tv.hdr.hwndFrom = GetSafeHwnd();
			tv.hdr.idFrom = GetWindowLong(GetSafeHwnd(), GWL_ID);
			tv.hdr.code = TVN_SELCHANGING;

			tv.itemNew.hItem = hClickedItem;
			tv.itemNew.state = GetItemState(hClickedItem, 0xffffffff);
			tv.itemNew.lParam = GetItemData(hClickedItem);

			tv.itemOld.hItem = NULL;
			tv.itemOld.mask = 0;

			tv.action = TVC_BYMOUSE;

			tv.ptDrag.x = point.x;
			tv.ptDrag.y = point.y;

			pWnd->SendMessage(WM_NOTIFY, tv.hdr.idFrom, (LPARAM)&tv);
		}

		// If the previously selected item was selected, re-select it  
		if (bIsSelectedItemSelected)
			SetItemState(hSelectedItem, TVIS_SELECTED, TVIS_SELECTED);

		// We want the newly selected item to toggle its selected state,  
		// so unselect now if it was already selected before  
		if (bIsClickedItemSelected)
			SetItemState(hClickedItem, 0, TVIS_SELECTED);
		else
		{
			SelectItem(hClickedItem);
			SetItemState(hClickedItem, TVIS_SELECTED, TVIS_SELECTED);
		}

		// If the previously selected item was selected, re-select it  
		if (bIsSelectedItemSelected && hSelectedItem != hClickedItem)
			SetItemState(hSelectedItem, TVIS_SELECTED, TVIS_SELECTED);

		// Store as first selected item (if not already stored)  
		if (m_hFirstSelectedItem == NULL)
			m_hFirstSelectedItem = hClickedItem;
	}
	else
	{
		// Clear selection of all "multiple selected" items first  
		ClearSelection();

		// Then select the clicked item  
		SelectItem(hClickedItem);
		SetItemState(hClickedItem, TVIS_SELECTED, TVIS_SELECTED);

		// Store as first selected item  
		m_hFirstSelectedItem = hClickedItem;
	}

	// Selection process is now complete. Since we have 'eaten' the TVN_SELCHANGED   
	// notification provided by Windows' treectrl, we must now produce one ourselves,  
	// so that our parent gets to know about the change of selection.  
	m_bSelectionComplete = TRUE;

	if (pWnd)
	{
		tv.hdr.hwndFrom = GetSafeHwnd();
		tv.hdr.idFrom = GetWindowLong(GetSafeHwnd(), GWL_ID);
		tv.hdr.code = TVN_SELCHANGED;

		tv.itemNew.hItem = m_hClickedItem;
		tv.itemNew.state = GetItemState(m_hClickedItem, 0xffffffff);
		tv.itemNew.lParam = GetItemData(m_hClickedItem);
		tv.itemNew.mask = TVIF_HANDLE | TVIF_STATE | TVIF_PARAM;

		tv.action = TVC_UNKNOWN;

		pWnd->SendMessage(WM_NOTIFY, tv.hdr.idFrom, (LPARAM)&tv);
	}
}

void CTreeCtrlEx::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CWnd* pWnd = GetParent();

	if (nChar == VK_NEXT || nChar == VK_PRIOR)
	{
		if (!(GetKeyState(VK_SHIFT) & 0x8000))
		{
			// User pressed Pg key without holding 'Shift':  
			// Clear multiple selection (if multiple) and let base class do   
			// normal selection work!  
			if (GetSelectedCount() > 1)
				ClearSelection(TRUE);

			CTreeCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
			m_hFirstSelectedItem = GetSelectedItem();
			return;
		}

		// Flag signaling that selection process is NOT complete.  
		// (Will prohibit TVN_SELCHANGED from being sent to parent)  
		m_bSelectionComplete = FALSE;

		// Let base class select the item  
		CTreeCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
		HTREEITEM hSelectedItem = GetSelectedItem();

		// Then select items in between  
		SelectItems(m_hFirstSelectedItem, hSelectedItem);

		// Selection process is now complete. Since we have 'eaten' the TVN_SELCHANGED   
		// notification provided by Windows' treectrl, we must now produce one ourselves,  
		// so that our parent gets to know about the change of selection.  
		m_bSelectionComplete = TRUE;

		if (pWnd)
		{
			NM_TREEVIEW tv;
			memset(&tv.itemOld, 0, sizeof(tv.itemOld));

			tv.hdr.hwndFrom = GetSafeHwnd();
			tv.hdr.idFrom = GetWindowLong(GetSafeHwnd(), GWL_ID);
			tv.hdr.code = TVN_SELCHANGED;

			tv.itemNew.hItem = hSelectedItem;
			tv.itemNew.state = GetItemState(hSelectedItem, 0xffffffff);
			tv.itemNew.lParam = GetItemData(hSelectedItem);
			tv.itemNew.mask = TVIF_HANDLE | TVIF_STATE | TVIF_PARAM;

			tv.action = TVC_UNKNOWN;

			pWnd->SendMessage(WM_NOTIFY, tv.hdr.idFrom, (LPARAM)&tv);
		}
	}
	else if (nChar == VK_UP || nChar == VK_DOWN)
	{
		// Find which item is currently selected  
		HTREEITEM hSelectedItem = GetSelectedItem();

		HTREEITEM hNextItem;
		if (nChar == VK_UP)
			hNextItem = GetPrevVisibleItem(hSelectedItem);
		else
			hNextItem = GetNextVisibleItem(hSelectedItem);

		if (!(GetKeyState(VK_SHIFT) & 0x8000))
		{
			// User pressed arrow key without holding 'Shift':  
			// Clear multiple selection (if multiple) and let base class do   
			// normal selection work!  
			if (GetSelectedCount() > 1)
				ClearSelection(TRUE);

			if (hNextItem)
				CTreeCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
			m_hFirstSelectedItem = GetSelectedItem();
			return;
		}

		if (hNextItem)
		{
			// Flag signaling that selection process is NOT complete.  
			// (Will prohibit TVN_SELCHANGED from being sent to parent)  
			m_bSelectionComplete = FALSE;

			// If the next item is already selected, we assume user is  
			// "moving back" in the selection, and thus we should clear   
			// selection on the previous one  
			BOOL bSelect = !(GetItemState(hNextItem, TVIS_SELECTED) & TVIS_SELECTED);

			// Select the next item (this will also deselect the previous one!)  
			SelectItem(hNextItem);

			// Now, re-select the previously selected item  
			if (bSelect || (!(GetItemState(hSelectedItem, TVIS_SELECTED) & TVIS_SELECTED)))
				SelectItems(m_hFirstSelectedItem, hNextItem);

			// Selection process is now complete. Since we have 'eaten' the TVN_SELCHANGED   
			// notification provided by Windows' treectrl, we must now produce one ourselves,  
			// so that our parent gets to know about the change of selection.  
			m_bSelectionComplete = TRUE;

			if (pWnd)
			{
				NM_TREEVIEW tv;
				memset(&tv.itemOld, 0, sizeof(tv.itemOld));

				tv.hdr.hwndFrom = GetSafeHwnd();
				tv.hdr.idFrom = GetWindowLong(GetSafeHwnd(), GWL_ID);
				tv.hdr.code = TVN_SELCHANGED;

				tv.itemNew.hItem = hNextItem;
				tv.itemNew.state = GetItemState(hNextItem, 0xffffffff);
				tv.itemNew.lParam = GetItemData(hNextItem);
				tv.itemNew.mask = TVIF_HANDLE | TVIF_STATE | TVIF_PARAM;

				tv.action = TVC_UNKNOWN;

				pWnd->SendMessage(WM_NOTIFY, tv.hdr.idFrom, (LPARAM)&tv);
			}
		}

		// Since the base class' OnKeyDown() isn't called in this case,  
		// we must provide our own TVN_KEYDOWN notification to the parent  

		CWnd* pWnd = GetParent();
		if (pWnd)
		{
			NMTVKEYDOWN tvk;

			tvk.hdr.hwndFrom = GetSafeHwnd();
			tvk.hdr.idFrom = GetWindowLong(GetSafeHwnd(), GWL_ID);
			tvk.hdr.code = TVN_KEYDOWN;

			tvk.wVKey = nChar;
			tvk.flags = 0;

			pWnd->SendMessage(WM_NOTIFY, tvk.hdr.idFrom, (LPARAM)&tvk);
		}
	}
	else
		// Behave normally  
		CTreeCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}


///////////////////////////////////////////////////////////////////////////////  
// I want clicking on an item with the right mouse button to select the item,  
// but not if there is currently a multiple selection  

void CTreeCtrlEx::OnRButtonDown(UINT nFlags, CPoint point)
{
	UINT nHitFlags = 0;
	HTREEITEM hClickedItem = HitTest(point, &nHitFlags);

	if (nHitFlags&TVHT_ONITEM)
	if (GetSelectedCount() < 2)
		SelectItem(hClickedItem);

	CTreeCtrl::OnRButtonDown(nFlags, point);
}


///////////////////////////////////////////////////////////////////////////////  
// Get number of selected items  

UINT CTreeCtrlEx::GetSelectedCount() const
{
	// Only visible items should be selected!  
	UINT uCount = 0;
	for (HTREEITEM hItem = GetRootItem(); hItem != NULL; hItem = GetNextVisibleItem(hItem))
	if (GetItemState(hItem, TVIS_SELECTED) & TVIS_SELECTED)
		uCount++;

	return uCount;
}


///////////////////////////////////////////////////////////////////////////////  
// Overloaded to catch our own special code  

HTREEITEM CTreeCtrlEx::GetNextItem(HTREEITEM hItem, UINT nCode)
{
	if (nCode == TVGN_EX_ALL)
	{
		// This special code lets us iterate through ALL tree items regardless   
		// of their parent/child relationship (very handy)  
		HTREEITEM hNextItem;

		// If it has a child node, this will be the next item  
		hNextItem = GetChildItem(hItem);
		if (hNextItem)
			return hNextItem;

		// Otherwise, see if it has a next sibling item  
		hNextItem = GetNextSiblingItem(hItem);
		if (hNextItem)
			return hNextItem;

		// Finally, look for next sibling to the parent item  
		HTREEITEM hParentItem = hItem;
		while (!hNextItem && hParentItem)
		{
			// No more children: Get next sibling to parent  
			hParentItem = GetParentItem(hParentItem);
			hNextItem = GetNextSiblingItem(hParentItem);
		}

		return hNextItem; // will return NULL if no more parents  
	}
	else
		return CTreeCtrl::GetNextItem(hItem, nCode);    // standard processing  
}

///////////////////////////////////////////////////////////////////////////////  
// Helpers to list out selected items. (Use similar to GetFirstVisibleItem(),   
// GetNextVisibleItem() and GetPrevVisibleItem()!)  

HTREEITEM CTreeCtrlEx::GetFirstSelectedItem()
{
	for (HTREEITEM hItem = GetRootItem(); hItem != NULL; hItem = GetNextVisibleItem(hItem))
	if (GetItemState(hItem, TVIS_SELECTED) & TVIS_SELECTED)
		return hItem;

	return NULL;
}

HTREEITEM CTreeCtrlEx::GetNextSelectedItem(HTREEITEM hItem)
{
	for (hItem = GetNextVisibleItem(hItem); hItem != NULL; hItem = GetNextVisibleItem(hItem))
	if (GetItemState(hItem, TVIS_SELECTED) & TVIS_SELECTED)
		return hItem;

	return NULL;
}

HTREEITEM CTreeCtrlEx::GetPrevSelectedItem(HTREEITEM hItem)
{
	for (hItem = GetPrevVisibleItem(hItem); hItem != NULL; hItem = GetPrevVisibleItem(hItem))
	if (GetItemState(hItem, TVIS_SELECTED) & TVIS_SELECTED)
		return hItem;

	return NULL;
}


///////////////////////////////////////////////////////////////////////////////  
// Select/unselect item without unselecting other items  

BOOL CTreeCtrlEx::SelectItemEx(HTREEITEM hItem, BOOL bSelect/*=TRUE*/)
{
	HTREEITEM hSelItem = GetSelectedItem();

	if (hItem == hSelItem)
	{
		if (!bSelect)
		{
			SelectItem(NULL);
			return TRUE;
		}

		return FALSE;
	}

	SelectItem(hItem);
	m_hFirstSelectedItem = hItem;

	// Reselect previous "real" selected item which was unselected byt SelectItem()  
	if (hSelItem)
		SetItemState(hSelItem, TVIS_SELECTED, TVIS_SELECTED);

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////  
// Select visible items between specified 'from' and 'to' item (including these!)  
// If the 'to' item is above the 'from' item, it traverses the tree in reverse   
// direction. Selection on other items is cleared!  

BOOL CTreeCtrlEx::SelectItems(HTREEITEM hFromItem, HTREEITEM hToItem)
{
	// Determine direction of selection   
	// (see what item comes first in the tree)  
	HTREEITEM hItem = GetRootItem();

	while (hItem && hItem != hFromItem && hItem != hToItem)
		hItem = GetNextVisibleItem(hItem);

	if (!hItem)
		return FALSE;   // Items not visible in tree  

	BOOL bReverse = hItem == hToItem;

	// "Really" select the 'to' item (which will deselect   
	// the previously selected item)  

	SelectItem(hToItem);

	// Go through all visible items again and select/unselect  

	hItem = GetRootItem();
	BOOL bSelect = FALSE;

	while (hItem)
	{
		if (hItem == (bReverse ? hToItem : hFromItem))
			bSelect = TRUE;

		if (bSelect)
		{
			if (!(GetItemState(hItem, TVIS_SELECTED) & TVIS_SELECTED))
				SetItemState(hItem, TVIS_SELECTED, TVIS_SELECTED);
		}
		else
		{
			if (GetItemState(hItem, TVIS_SELECTED) & TVIS_SELECTED)
				SetItemState(hItem, 0, TVIS_SELECTED);
		}

		if (hItem == (bReverse ? hFromItem : hToItem))
			bSelect = FALSE;

		hItem = GetNextVisibleItem(hItem);
	}

	return TRUE;
}

BOOL CTreeCtrlEx::IsItemSelected(HTREEITEM hItem)
{
	return GetItemState(hItem, TVIS_SELECTED) & TVIS_SELECTED;
}


///////////////////////////////////////////////////////////////////////////////  
// Clear selected state on all visible items  

void CTreeCtrlEx::ClearSelection(BOOL bMultiOnly/*=FALSE*/)
{
	//  if ( !bMultiOnly )  
	//      SelectItem( NULL );  

	for (HTREEITEM hItem = GetRootItem(); hItem != NULL; hItem = GetNextVisibleItem(hItem))
	if (GetItemState(hItem, TVIS_SELECTED) & TVIS_SELECTED)
		SetItemState(hItem, 0, TVIS_SELECTED);
}

void CTreeCtrlEx::ClearSelection(HTREEITEM hItem)
{
	if (GetItemState(hItem, TVIS_SELECTED) & TVIS_SELECTED)
		SetItemState(hItem, 0, TVIS_SELECTED);
}


///////////////////////////////////////////////////////////////////////////////  
// If a node is collapsed, we should clear selections of its child items   

BOOL CTreeCtrlEx::OnItemexpanding(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	if (pNMTreeView->action == TVE_COLLAPSE)
	{
		HTREEITEM hItem = GetChildItem(pNMTreeView->itemNew.hItem);

		while (hItem)
		{
			if (GetItemState(hItem, TVIS_SELECTED) & TVIS_SELECTED)
				SetItemState(hItem, 0, TVIS_SELECTED);

			// Get the next node: First see if current node has a child  
			HTREEITEM hNextItem = GetChildItem(hItem);
			if (!hNextItem)
			{
				// No child: Get next sibling item  
				if (!(hNextItem = GetNextSiblingItem(hItem)))
				{
					HTREEITEM hParentItem = hItem;
					while (!hNextItem)
					{
						// No more children: Get parent  
						if (!(hParentItem = GetParentItem(hParentItem)))
							break;

						// Quit when parent is the collapsed node  
						// (Don't do anything to siblings of this)  
						if (hParentItem == pNMTreeView->itemNew.hItem)
							break;

						// Get next sibling to parent  
						hNextItem = GetNextSiblingItem(hParentItem);
					}

					// Quit when parent is the collapsed node  
					if (hParentItem == pNMTreeView->itemNew.hItem)
						break;
				}
			}

			hItem = hNextItem;
		}
	}

	*pResult = 0;
	return FALSE;   // Allow parent to handle this notification as well  
}


///////////////////////////////////////////////////////////////////////////////  
// Intercept TVN_SELCHANGED and pass it only to the parent window of the  
// selection process is finished  

BOOL CTreeCtrlEx::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	// Return TRUE if selection is not complete. This will prevent the   
	// notification from being sent to parent.  
	return !m_bSelectionComplete;
}


///////////////////////////////////////////////////////////////////////////////  
// Ensure the multiple selected items are drawn correctly when loosing/getting  
// the focus  

BOOL CTreeCtrlEx::OnSetfocus(NMHDR* pNMHDR, LRESULT* pResult)
{
	Invalidate();
	*pResult = 0;
	return FALSE;
}

BOOL CTreeCtrlEx::OnKillfocus(NMHDR* pNMHDR, LRESULT* pResult)
{
	Invalidate();
	*pResult = 0;
	return FALSE;
}

// void CTreeCtrlEx::OnLButtonDblClk(UINT nFlags, CPoint point)
// {
// 	// We stop label editing.
// 	m_bEditLabelPending = FALSE;
// 	CTreeCtrl::OnLButtonDblClk(nFlags, point);
// }

void CTreeCtrlEx::OnTimer(/*UINT*/UINT_PTR nIDEvent)
{
	if (nIDEvent == TCEX_EDITLABEL)
	{
		// Stop the timer.  
		KillTimer(m_idTimer);

		// Invoke label editing.  
		if (m_bEditLabelPending)
			EditLabel(GetSelectedItem());

		m_bEditLabelPending = FALSE;
		return;
	}

	CTreeCtrl::OnTimer(nIDEvent);
}

///////////////////////////////////////////////////////////////////////////////  
// Retreives a tree ctrl item given the item's data  

HTREEITEM CTreeCtrlEx::ItemFromData(DWORD dwData, HTREEITEM hStartAtItem/*=NULL*/) const
{
	// Traverse all items in tree control  
	HTREEITEM hItem;
	if (hStartAtItem)
		hItem = hStartAtItem;
	else
		hItem = GetRootItem();

	while (hItem)
	{
		if (dwData == (DWORD)GetItemData(hItem))
			return hItem;

		// Get first child node  
		HTREEITEM hNextItem = GetChildItem(hItem);

		if (!hNextItem)
		{
			// Get next sibling child  
			hNextItem = GetNextSiblingItem(hItem);

			if (!hNextItem)
			{
				HTREEITEM hParentItem = hItem;
				while (!hNextItem && hParentItem)
				{
					// No more children: Get next sibling to parent  
					hParentItem = GetParentItem(hParentItem);
					hNextItem = GetNextSiblingItem(hParentItem);
				}
			}
		}

		hItem = hNextItem;
	}

	return NULL;
}


/////////////////////////////////////////////////////////////////////////////  

HTREEITEM GetTreeItemFromData(CTreeCtrl& treeCtrl, DWORD dwData, HTREEITEM hStartAtItem /*=NULL*/)
{
	// Traverse from given item (or all items if hFromItem is NULL)  
	HTREEITEM hItem;
	if (hStartAtItem)
		hItem = hStartAtItem;
	else
		hItem = treeCtrl.GetRootItem();

	while (hItem)
	{
		if (dwData == (DWORD)treeCtrl.GetItemData(hItem))
			return hItem;

		// Get first child node  
		HTREEITEM hNextItem = treeCtrl.GetChildItem(hItem);

		if (!hNextItem)
		{
			// Get next sibling child  
			hNextItem = treeCtrl.GetNextSiblingItem(hItem);

			if (!hNextItem)
			{
				HTREEITEM hParentItem = hItem;
				while (!hNextItem && hParentItem)
				{
					// No more children: Get next sibling to parent  
					hParentItem = treeCtrl.GetParentItem(hParentItem);
					hNextItem = treeCtrl.GetNextSiblingItem(hParentItem);
				}
			}
		}
		hItem = hNextItem;
	}
	return NULL;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/

BEGIN_MESSAGE_MAP(CDirTreeCtrl, CTreeCtrlEx)
	//{{AFX_MSG_MAP(CDirTreeCtrl)  
	// NOTE - the ClassWizard will add and remove mapping macros here.  
	//}}AFX_MSG_MAP  
	//ON_NOTIFY_REFLECT(NM_CLICK, &CDirTreeCtrl::OnNMClick)  
	ON_NOTIFY_REFLECT(TVN_ITEMEXPANDING, &CDirTreeCtrl::OnTvnItemexpanding)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////  
// CDirTreeCtrl message handlers  
//程序调用的接口，显示一棵目录树，  
//strRoot为根目录路径  
BOOL CDirTreeCtrl::DisplayTree(LPCTSTR strRoot)
{
	SetItemHeight(20);
	SetTextColor(RGB(0X0, 0X0, 0XFF));
	DeleteAllItems();
	SetDirTreeStyle();
	if (!AttachImgList()) return FALSE;

	DisplayDrives();

	return TRUE;
}
//设置目录树属性  
void CDirTreeCtrl::SetDirTreeStyle()
{
	DWORD dwStyle = GetWindowLong(m_hWnd, GWL_STYLE);
	dwStyle |= TVS_HASBUTTONS |
		TVS_HASLINES | TVS_LINESATROOT |
		/*TVS_CHECKBOXES |*/
		WS_BORDER | WS_TABSTOP;
	m_treeStyle = dwStyle;
	SetWindowLong(m_hWnd, GWL_STYLE, dwStyle);
}
//获取系统图标  
BOOL CDirTreeCtrl::AttachImgList()
{
	SHFILEINFOW shFinfo;
	HIMAGELIST hImgList = NULL;

	if (GetImageList(TVSIL_NORMAL)) m_imgList.Detach();

	//char *szName="C:\\";  
	//USES_CONVERSION; // 这个宏一定要加上,否则会出一堆错误    
	//LPCWSTR pName=T2W(szName); // tchar ---> wchar   
	hImgList = (HIMAGELIST)SHGetFileInfoW(
		/*pName*/L"C:\\",
		0,
		&shFinfo,
		sizeof(shFinfo),
		SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
	if (!hImgList)
	{
		m_strError = _T("无法得到系统图标文件!");
		return FALSE;
	}
	m_imgList.m_hImageList = hImgList;

	SetImageList(&m_imgList, TVSIL_NORMAL);
	return TRUE;
}
//显示系统盘符  
BOOL CDirTreeCtrl::DisplayDrives()
{
	DeleteAllItems();

	TCHAR  szDrives[260];
	TCHAR* pDrive = NULL;
	if (!GetLogicalDriveStrings(sizeof(szDrives), szDrives))
	{
		m_strError = _T("驱动信息获取失败！");
		return FALSE;
	}

	pDrive = szDrives;    //szDrives 中的字符串格式：_T("C:\\0D:\\0D:\\0E:\\0")   

	m_hDirTreeRoot = InsertItem(_T("计算机"), 15, 25);//15 和是计算机的两个图标，前一个是没选中时的，后一个是选中时的  
	//CStringArray strDrives;  
	//CString strDrive;  
	//while( *pDrive!=0 )  
	//{  
	//  strDrives.Add(pDrive);  
	//  pDrive += _tcslen( pDrive ) + 1;  
	//}  
	//for (int n=0; n<strDrives.GetCount(); n++)  
	//{  
	//  strDrive=strDrives.GetAt(n);  
	//  strDrive.SetAt(strDrive.GetLength()-1,_T('\0'));  //试验表明：uincode 的"\0"字符同样适用！！！  
	//  HTREEITEM hParent = AddItem( m_hDirTreeRoot,strDrive  );  
	//  if ( FindSubDir( strDrive ))  InsertItem(_T("dummy"),0,0,hParent);  
	//    
	//}  

	// 去掉盘符后的"\",下面的思路经验证也可行，而且更加简练  
	int len;
	while (*pDrive != 0)
	{
		len = (int)_tcslen(pDrive);
		pDrive[len - 1] = _T('\0');
		HTREEITEM hParent = AddItem(m_hDirTreeRoot, pDrive);
		if (FindSubDir(pDrive))  AddSubDirAsItem(hParent);
		// 一个技巧先加入下一级子目录项，  
		// 然后再点击该项后，首先去掉所有子项，然后再加入，  
		// 这样的好处是在前面路径的方框中会有一个+号，  
		// 因为如果把全部的目录一次加入，大约需要半个小时时间，所以采取点击时先去掉所有项，然后再增加子目录  
		pDrive += len + 1;
	}

	Expand(m_hDirTreeRoot, TVE_EXPAND);
	return TRUE;
}


//是否有子目录可展开  
BOOL CDirTreeCtrl::FindSubDir(LPCTSTR strPath)
{
	CFileFind find;
	CString   strTemp = strPath;
	BOOL      bFind;

	if (strTemp.Right(1) == _T('\\'))        strTemp += _T("*.*");
	else        strTemp += _T("\\*.*");

	bFind = find.FindFile(strTemp);
	while (bFind)
	{
		bFind = find.FindNextFile();
		if (find.IsDirectory() && !find.IsDots())
		{
			return TRUE;
		}
		if (!find.IsDirectory()/* && m_bShowFiles*/)
			return TRUE;
	}
	return FALSE;
}

//获取全目录  
CString CDirTreeCtrl::GetFullPath(HTREEITEM hItem)
{
	CString strReturn;
	CString strTemp;
	HTREEITEM hParent = hItem;

	strReturn = "";

	while (hParent)
	{

		strTemp = GetItemText(hParent);
		if (strTemp != _T("计算机"))
		{
			if (strTemp.Right(1) != _T("\\"))     strTemp += _T("\\");
			strReturn = strTemp + strReturn;
		}
		hParent = GetParentItem(hParent);
	}

	return strReturn;
}




// 需要在此节点上，加入它的整个子目录和下一级的子目录，作为迭代加入  
BOOL CDirTreeCtrl::AddSubDirAsItem(HTREEITEM hParent)
{
	CString strPath, strFileName;
	HTREEITEM hChild;

	//---------------------去除该父项下所有的子项------------  
	// 因为有dummy项，并且有时子目录再次打开，或子目录会刷新等，因此必须去除。  
	while (ItemHasChildren(hParent))
	{
		hChild = GetChildItem(hParent);
		DeleteItem(hChild);

	}

	//-----------------------装入该父项下所有子项--------------  
	strPath = GetFullPath(hParent);  // 从本节点开始到根的路径  
	CString strSearchCmd = strPath;
	if (strSearchCmd.Right(1) != _T("\\")) strSearchCmd += _T("\\");
	strSearchCmd += _T("*.*");
	CFileFind find;
	BOOL bContinue = find.FindFile(strSearchCmd);
	while (bContinue)
	{
		bContinue = find.FindNextFile();
		strFileName = find.GetFileName();

		if (!find.IsHidden() && !find.IsDots() && find.IsDirectory())
		{
			hChild = AddItem(hParent, strFileName);
			if (FindSubDir(GetFullPath(hChild)))  AddSubDirAsItem1(hChild);
			// 一个技巧：先加入下一级子目录项，  
			// 然后再点击该项后，再先去掉所有子项，然后再加入，  
			// 这样的好处是在前面路径的方框中会有一个+号，  
			// 因为如果把全部的目录一次加入，大约需要半个小时时间，所以采取点击时先去掉所有项，然后再增加子目录  

		}
		if (!find.IsHidden() && !find.IsDots() && !find.IsDirectory())
		{
			InsertItem(strFileName, 0, 0, hParent);
		}

	}

	//////////////////////////////////////////////////  


	return TRUE;

}


//仅仅装入下一级子目录  
BOOL CDirTreeCtrl::AddSubDirAsItem1(HTREEITEM hParent)
{
	CString strPath, strFileName;
	HTREEITEM hChild;

	//---------------------去除该父项下所有的子项------------  
	// 因为有dummy项，并且有时子目录再次打开，或子目录会刷新等，因此必须去除。  
	while (ItemHasChildren(hParent))
	{
		hChild = GetChildItem(hParent);
		DeleteItem(hChild);
	}

	//-----------------------装入该父项下所有子项--------------  
	strPath = GetFullPath(hParent);  // 从本节点开始到根的路径  
	CString strSearchCmd = strPath;
	if (strSearchCmd.Right(1) != _T("\\")) strSearchCmd += _T("\\");
	strSearchCmd += _T("*.*");
	CFileFind find;
	BOOL bContinue = find.FindFile(strSearchCmd);
	while (bContinue)
	{
		bContinue = find.FindNextFile();
		strFileName = find.GetFileName();

		if (!find.IsHidden() && !find.IsDots() && find.IsDirectory())
		{
			hChild = AddItem(hParent, strFileName);
		}
		if (!find.IsHidden() && !find.IsDots() && !find.IsDirectory())
		{
			InsertItem(strFileName, 0, 0, hParent);
		}

	}

	//////////////////////////////////////////////////  


	return TRUE;

}

//添加项  
HTREEITEM CDirTreeCtrl::AddItem(HTREEITEM hParent, LPCTSTR strName)
{
	// 获取路径  
	CString strPath = GetFullPath(hParent);
	CString strTemp = strPath + CString(strName);

	if (strTemp.Right(1) != _T("\\"))     strTemp += _T("\\");

	SHFILEINFO shFinfo;
	int iIcon, iIconSel;


	if (!SHGetFileInfo(strTemp,
		0,
		&shFinfo,
		sizeof(shFinfo),
		SHGFI_ICON |
		SHGFI_SMALLICON))
	{
		m_strError = _T("系统图表获取失败!");
		return NULL;
	}

	iIcon = shFinfo.iIcon;

	// we only need the index from the system image list  
	DestroyIcon(shFinfo.hIcon);

	if (!SHGetFileInfo(strTemp,
		0,
		&shFinfo,
		sizeof(shFinfo),
		SHGFI_ICON | SHGFI_OPENICON |
		SHGFI_SMALLICON))
	{
		m_strError = _T("系统图表获取失败!");
		return NULL;
	}

	iIconSel = shFinfo.iIcon;

	// we only need the index of the system image list  
	DestroyIcon(shFinfo.hIcon);

	return InsertItem(strName, iIcon, iIconSel, hParent);
}


//这里利用反射实现此函数，好处是可以把消息的处理完全封闭到类内，有利于使用  
//父窗口去掉对此消息的处理即可，没有处理，消息自然返回  
//void CDirTreeCtrl::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult)  
//{  
//  // TODO: Add your control notification handler code here  
//  
//  
//  CPoint myPoint;  
//  UINT uFlag;  
//  
//  GetCursorPos(&myPoint);  
//  ScreenToClient(&myPoint);  
//  HTREEITEM hItem = HitTest(myPoint, &uFlag);  
//  if(NULL == hItem ) return;  
//  
//  if (_T("计算机")==GetItemText(hItem))  return;  
//  
//  // 用户点选了一个目录项  
//  // (TVHT_ONITEM & uFlag)的原因：如果不添加，即使在离图标或标识的较远的地方点一下左键（没在正上面），也会有有树的展开和收缩动作  
//  // (0x0010&uFlag) 如果不加此项，则点击+号时，AddSubDirAsItem不会被调用，下一级的有子目录的文件夹前面就没有+号  
//  // #define TVHT_ONITEMBUTTON   0x0010  我是通过跟踪试验找到这个值的   
//  if ((TVHT_ONITEM & uFlag)||(0x0010&uFlag))  
//  {  
//      AddSubDirAsItem(hItem);  
//  }  
//  else return;  
//  Expand( hItem, TVE_EXPAND );  
//  
//  
//  
//  BOOL bCheck;  
//  if(hItem && (TVHT_ONITEMSTATEICON & uFlag))  
//  {  
//         bCheck = GetCheck(hItem);  
//         SetChildCheck(hItem, !bCheck);  
//  }  
//  
//  
//  
//  *pResult = 0;  
//}  

//这里也是利用反射，好处是可以把消息的处理完全封闭到类内，实现封闭  
void CDirTreeCtrl::OnTvnItemexpanding(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here  
	//HTREEITEM hItem = GetSelectedItem();  // 注意：用这个函数不行，在+号打开时，子目录总是没有+号，尽管子目录中有子项  
	TV_ITEM tvi = pNMTreeView->itemNew;
	HTREEITEM hItem = tvi.hItem;

	if (NULL == hItem) return;

	if (_T("计算机") == GetItemText(hItem))  return;

	AddSubDirAsItem(hItem);



	*pResult = 0;
}

void CTreeCtrlEx::OnRButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
//	if (GetSelectedCount() == 0) return;
//// 	CPoint point;
//// 	GetCursorPos(&point);//获得鼠标点击的位置
//	CMenu menu;
//	VERIFY(menu.LoadMenu(IDR_MENU_RBUTTON_POPUP));
//	if (GetSelectedCount() > 1)
//	{
//		menu.GetSubMenu(0)->EnableMenuItem(3, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
//		menu.GetSubMenu(0)->EnableMenuItem(4, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
//		menu.GetSubMenu(0)->EnableMenuItem(6, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
//	}
//	if (GetSelectedCount() == 1)
//	{
//		HTREEITEM hItemSelect = GetFirstSelectedItem();
//		int n = GetItemData(hItemSelect); if (n < 0) return;
//		if (GetViewHand()->m_vecImgRenderInfo[GetItemData(hItemSelect)].GetImgType() != VECTOR)
//		{
//			menu.GetSubMenu(0)->EnableMenuItem(3, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
//		}
//	}
//	menu.GetSubMenu(0)->GetSubMenu(5)->EnableMenuItem(0, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
//	CMenu* popup = menu.GetSubMenu(0);
//	ASSERT(popup != NULL);
//	popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
	CTreeCtrl::OnRButtonUp(nFlags, point);
}

