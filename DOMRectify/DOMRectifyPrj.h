#pragma once
#include "GLImageReader/GLImageReader.h"
#include "MyDefine.h"
#include <vector>
using namespace std;
class CDOMRectifyPrj
{
public:
	CDOMRectifyPrj(void);
	~CDOMRectifyPrj(void);
	void ClearPrj();
	void NewPrj(CString strPrjPath, vector<CString> vecRefPath, vector<stuImgGroup>& vecDomGroup, int nDomNum);
	bool SavePrj(CString strSavePath);
	bool OpenPrj(CString strPrjPath);
	
	bool LoadPoint(int nIdx, ImageReader *pReader, eDOMTYPE eDomType);
	void AddPoint(OGRPoint ptDom, OGRPoint ptRef, int nRefIdx);
	void SavePoint(ImageReader * pReader);
/*	void SavePoint();*/

	void SelPoint(OGREnvelope enve);
	void DelPoint();
	void ClearPoint();
	CString GetPrjPath();
	CString GetRefPath(int idx);
	CString GetPanPath(int idx);
	CString GetMuxPath(int idx);
	CString GetFusPath(int idx);
	CString GetMatchFilePath(int idx);
	CString GetCurDomPath();
	CString GetCurRecDomPath();
	CString GetCurMatchPath();
	stuImgGroup GetDomGroup(int idx);
	vector<stuImgGroup> GetDomGroup();
	vector<CString> GetRefPath();
	vector<stuMatchPoint> GetRefPoints();
	vector<stuMatchPoint> GetDomPoints();
	vector<stuMatchPoint> GetRefPoints(int nRefIdx);
	vector<stuMatchPoint> GetDomPoints(int nRefIdx);
	vector<stuMatchPoint> GetSelRefPoints();
	vector<stuMatchPoint> GetSelDomPoints();

	int GetGroupNum();
	int GetDomNum();
	int GetRefNum();
	int GetAllPointNum();
	int GetCurPointNum();
	int GetSelPointNum();
protected:
	CString m_strPrjPath; 
	vector<CString> m_vecRefPath;
	int m_nGroupNum;
	int m_nDomNum;
	int m_nRefNum;
	int m_nPtNum;
	int m_nCurPtNum;
	vector<int> m_vecSelectedPointIdx;
	vector<stuImgGroup> m_vecDomGroup;
	eDOMTYPE m_eDomTyep;
	CString m_strMatchFile;
	CString m_strCurDomPath;
	CString m_strCurRecDomPath;
	vector<stuMatchPoint> m_vecPtRef;
	vector<stuMatchPoint> m_vecPtDom;
};

