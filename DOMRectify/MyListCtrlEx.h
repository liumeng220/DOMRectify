//*************************************************************************
// MyListCtrlEx.h : header file
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

#pragma once

#include "MyHeaderCtrlEx.h"

// CMyListCtrlReDrawEx

class CMyListCtrlEx : public CListCtrl
{
	DECLARE_DYNAMIC(CMyListCtrlEx)

public:
	CMyListCtrlEx();
	virtual ~CMyListCtrlEx();

	CMyHeaderCtrlEx *GetHeaderCtrl() {return &m_wndHeader;}

protected:
	DECLARE_MESSAGE_MAP()

protected:
	CString m_strKey;
	CMyHeaderCtrlEx m_wndHeader;
	virtual void PreSubclassWindow();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

public:
	void SetRegistryKey(LPCTSTR lpszKey);
	BOOL SaveState();
	BOOL RestoreState();

	void SetEditItem(int nRow,int nCol);
	void UpdateEditItem();
	bool IsEditItem();
private:
	CEdit m_EditItem;
	int m_nEditRow;
	int m_nEditCol;
protected:
	BOOL VerifyOrderArray(int *piArray, int count);

public:
	afx_msg void OnNmRclickHeader(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnHdnDividerdblclick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnHdnEnddrag(NMHDR *pNMHDR, LRESULT *pResult);
};


// CMyListCtrlReDraw
class CMyListCtrlReDraw : public CListCtrl
{
	DECLARE_DYNAMIC(CMyListCtrlReDraw)
public:
	unsigned int LIST_ITEM_HEIGHT;
	unsigned int LIST_HEAD_HEIGHT;
public:
	CMyListCtrlReDraw();
	virtual ~CMyListCtrlReDraw();
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult);
public:
protected:
	void InitCustomdraw();
	void InitDrawItem();
	virtual void DrawSubItem(CDC *pDC, int nItem, int nSubItem, CRect &rSubItem, bool bSelected, bool bFocus);
	virtual void DrawRemainSpace(LPNMLVCUSTOMDRAW lpnmcd);
	virtual void draw_row_bg(CDC *pDC, RECT rc, bool bSelected, bool bFocus, int nRow);
	virtual void draw_row_bg(CDC *pDC, RECT rc, bool bSelected, bool bFocus, bool bOdd);
	void InvalidateItem(int nItem);
public:
	afx_msg void OnLvnItemchanged(NMHDR *pNMHDR, LRESULT *pResult);
	virtual void PreSubclassWindow();
	virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
	void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
};

