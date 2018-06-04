#pragma once
#include "MyDockablePane.h"
#include "MyDefine.h"
#include "MyTreeCtrlEx.h"
#include <vector>
using namespace std;
// CMyDockPaneWndData

class CMyDockPaneWndData : public CMyDockablePane
{
	DECLARE_DYNAMIC(CMyDockPaneWndData)

public:
	CMyDockPaneWndData();
	virtual ~CMyDockPaneWndData();
	void FillImgTree(vector<stuImgGroup>& vecDomGroup, vector<CString>& vecRefPath);
	void ClearImgTree();
	void AddToMatchList(CString strPath);
	void ClearMatchList();
	eDOMTYPE HighlightSelectedItem(int nItemData);

	vector<CString> m_vecImgPathInList;
protected:
	CTreeCtrl  m_ImgTree;
	CListCtrl  m_MatchList;
	vector<HTREEITEM> m_vecGroupItem;
	int m_nHighlight;
//	HTREEITEM m_hRoot;
	
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDBClickImgTree(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnClickImgTree(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDBClickMatchList(NMHDR *pNMHDR, LRESULT *pResult);
};


