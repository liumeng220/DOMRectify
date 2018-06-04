#pragma once


// CMyMFCRibbonBar

class CMyMFCRibbonBar : public CMFCRibbonBar
{
	DECLARE_DYNAMIC(CMyMFCRibbonBar)

public:
	CMyMFCRibbonBar();
	virtual ~CMyMFCRibbonBar();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
};


