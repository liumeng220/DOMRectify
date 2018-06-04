#pragma once  

/************************************************************************/
/*                                                                      */
/************************************************************************/

#ifndef __TREECTRLEX_H  
#define __TREECTRLEX_H  

#define TVGN_EX_ALL         0x000F  

/////////////////////////////////////////////////////////////////////////////  
// CTreeCtrlEx window  

class CTreeCtrlEx : public CTreeCtrl
{
	DECLARE_DYNAMIC(CTreeCtrlEx)

	// Construction  
public:
	CTreeCtrlEx() : m_bSelectPending(FALSE), m_hClickedItem(NULL), m_hFirstSelectedItem(NULL), m_bSelectionComplete(TRUE), m_bEditLabelPending(FALSE) {}
	BOOL Create(DWORD dwStyle, DWORD dwExStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
	BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

	// Attributes  
public:
	UINT GetSelectedCount() const;
	HTREEITEM GetNextItem(HTREEITEM hItem, UINT nCode);
	HTREEITEM GetFirstSelectedItem();
	HTREEITEM GetNextSelectedItem(HTREEITEM hItem);
	HTREEITEM GetPrevSelectedItem(HTREEITEM hItem);
	HTREEITEM ItemFromData(DWORD dwData, HTREEITEM hStartAtItem = NULL) const;

	BOOL SelectItemEx(HTREEITEM hItem, BOOL bSelect = TRUE);
	BOOL SelectItems(HTREEITEM hFromItem, HTREEITEM hToItem);
	BOOL IsItemSelected(HTREEITEM hItem);
	void ClearSelection(BOOL bMultiOnly = FALSE);
	void ClearSelection(HTREEITEM hItem);

protected:
	void SelectMultiple(HTREEITEM hClickedItem, UINT nFlags, CPoint point);

private:
	BOOL        m_bSelectPending;
	CPoint      m_ptClick;
	HTREEITEM   m_hClickedItem;
	HTREEITEM   m_hFirstSelectedItem;
	BOOL        m_bSelectionComplete;
	BOOL        m_bEditLabelPending;
	UINT        m_idTimer;

	// Operations  
public:

	// Overrides  
	// ClassWizard generated virtual function overrides  
	//{{AFX_VIRTUAL(CTreeCtrlEx)  
	//}}AFX_VIRTUAL  

	// Implementation  
public:
	virtual ~CTreeCtrlEx() {}

	// Generated message map functions  
protected:
	//{{AFX_MSG(CTreeCtrlEx)  
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnItemexpanding(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL OnSetfocus(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL OnKillfocus(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
//	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnTimer(/*UINT*/UINT_PTR nIDEvent);
	//}}AFX_MSG  

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
};

/************************************************************************/
/*                                                                      */
/************************************************************************/

class CDirTreeCtrl : public CTreeCtrlEx
{
	DECLARE_DYNAMIC(CDirTreeCtrl)

	//data  
private:
	BOOL AddSubDirAsItem(HTREEITEM hParent);
	BOOL AddSubDirAsItem1(HTREEITEM hParent);

	BOOL FindSubDir(LPCTSTR strPath);
	CString GetFullPath(HTREEITEM hItem);
	BOOL DisplayDrives();
	BOOL AttachImgList();
	HTREEITEM AddItem(HTREEITEM hParent, LPCTSTR strName);

	CString        m_strError;
	CImageList     m_imgList;
	HTREEITEM m_hDirTreeRoot;
	DWORD m_treeStyle;

public:
	CDirTreeCtrl();
	void SetDirTreeStyle();
	BOOL DisplayTree(LPCTSTR strRoot);
	virtual ~CDirTreeCtrl();

	// Generated message map functions  
protected:
	//{{AFX_MSG(CDirTreeCtrl)  
	// NOTE - the ClassWizard will add and remove member functions here.  
	//}}AFX_MSG  
	DECLARE_MESSAGE_MAP()

public:
	//afx_msg void OnNMClick(NMHDR *pNMHDR, LRESULT *pResult);   // 用这个作为鼠标消息响应也可以，但是下面的一个更好  
	afx_msg void OnTvnItemexpanding(NMHDR *pNMHDR, LRESULT *pResult);
};

HTREEITEM GetTreeItemFromData(CTreeCtrl& treeCtrl, DWORD dwData, HTREEITEM hStartAtItem = NULL);

#endif  