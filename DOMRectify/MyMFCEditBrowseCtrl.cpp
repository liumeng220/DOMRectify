// MyMFCEdutVriwseCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "DOMRectify.h"
#include "MyMFCEditBrowseCtrl.h"


// CMyMFCEdutVriwseCtrl

IMPLEMENT_DYNAMIC(CMyMFCEditBrowseCtrl, CMFCEditBrowseCtrl)

CMyMFCEditBrowseCtrl::CMyMFCEditBrowseCtrl()
{
	m_browserType = eFolder;
}

CMyMFCEditBrowseCtrl::~CMyMFCEditBrowseCtrl()
{
}


BEGIN_MESSAGE_MAP(CMyMFCEditBrowseCtrl, CMFCEditBrowseCtrl)
END_MESSAGE_MAP()

void CMyMFCEditBrowseCtrl::OnBrowse()
{
	switch(m_browserType)
	{
	case eFile:
		{
			CFileDialog dlg(TRUE,NULL,NULL,NULL,NULL,NULL,0,TRUE);
			(dlg.m_ofn).lpstrFile="Open File";
			dlg.DoModal();
			break;
		}
	case eFolder:
		{
			CFolderPickerDialog dlg;
			if(dlg.DoModal()==IDOK)
			{
				SetWindowText(dlg.GetPathName());
			}
			break;
		}
	}
}

// CMyMFCEdutVriwseCtrl message handlers


