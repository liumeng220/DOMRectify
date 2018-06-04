#pragma once
#include "afxcmn.h"

#include "MyListCtrlEx.h"
#include "MyFunctions.h"
#include "MyDefine.h"
// CDlgPrjNew2 dialog

class CDlgPrjNew2 : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgPrjNew2)

public:
	CDlgPrjNew2(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgPrjNew2();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_PRJ_NEW2 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	struct stuImgList
	{
		CString strId;
		CString strType;
		CString strIdx;
		CString strPath;
	};
	DECLARE_MESSAGE_MAP()
public:
	void ClearData();
	void ImageGrouping(vector<CString> &vecAllImgPath);
	void LoadImgListFile(CString strListFile);
	void SearchRefImage(CString strFolder);
	void SearchDomImage(CString strFolder);
	void DelectImages();
	void UpdateImgList();
public:
	BOOL m_bAutoCreatePyramid;
	BOOL m_bJpeg;
	BOOL m_bJpg;
	BOOL m_bTif;
	BOOL m_bTiff;
	int m_nDomNum;
	int m_nGroupNum;
	int m_nRefNum;
	CMyListCtrlEx m_ListImgInfo;

	vector<stuImgGroup> m_vecDomGroups;
	vector<CString> m_vecRefPath;
	vector<stuImgList> m_vecImgListInfo;

	virtual BOOL OnInitDialog();
	afx_msg void OnGetdispinfoListImginfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonLoadImglist();
	afx_msg void OnBnClickedButtonLoadImgRef();
	afx_msg void OnBnClickedButtonLoadImgDom();
	afx_msg void OnBnClickedButtonDeleteImg();
};
