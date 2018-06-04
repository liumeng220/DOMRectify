#pragma once
#include "afxeditbrowsectrl.h"
#include "afxcmn.h"
#include "MyMFCEditBrowseCtrl.h"
#include "MyListCtrlEx.h"
#include "MyFunctions.h"
#include "MyDefine.h"
#include "afxwin.h"
// CDlgPrjNew dialog

class CDlgPrjNew : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgPrjNew)

public:
	CDlgPrjNew(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgPrjNew();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_PRJ_NEW };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	void ClearData();
	void ImageGrouping(vector<CString>& vecAllImgPath);
	void ImageGrouping(vector<CString>& vecAllPanPath, vector<CString>& vecAllMuxPath, vector<CString>& vecAllFusPath);
public:
	CMFCEditBrowseCtrl m_EditBrowsePrjPath;
	CMFCEditBrowseCtrl m_EditBrowseRefPath;
	CMyMFCEditBrowseCtrl m_EditBrowseDomFolder;
	CString m_strPrjPath;
	CString m_strRefPath;
	CString m_strDomFolder;
	CMyListCtrlEx m_listImgInfo;
	BOOL m_bAutoSelection;

	vector<stuImgGroup> m_vecImgGroups;

	virtual BOOL OnInitDialog();
	afx_msg void OnEnChangeMfceditbrowseDomFolder();
	afx_msg void OnGetdispinfoListImginfo(NMHDR *pNMHDR, LRESULT *pResult);
	CComboBox m_CombDomFileExt;
	afx_msg void OnBnClickedOk();
	afx_msg void OnEnChangeMfceditbrowsePrjPath();
};
