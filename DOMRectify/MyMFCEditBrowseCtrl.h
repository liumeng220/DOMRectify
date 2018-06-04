#pragma once


// CMyMFCEdutVriwseCtrl
enum BrowserType
{
	eFile = 0,
	eFolder = 1
};

class CMyMFCEditBrowseCtrl : public CMFCEditBrowseCtrl
{
	DECLARE_DYNAMIC(CMyMFCEditBrowseCtrl)

public:
	CMyMFCEditBrowseCtrl();
	virtual ~CMyMFCEditBrowseCtrl();
	
	BrowserType m_browserType;

protected:
	DECLARE_MESSAGE_MAP()
	void OnBrowse();
};


