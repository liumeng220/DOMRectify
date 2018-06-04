#include "DomRectifyPro.h"
#include "stdafx.h"
#include "DomRectifyPro.h"
#include "IdpmImage\IdpmImage.h"
#include "io.h"
#include "stdlib.h"
#include "stdio.h"
#include "conio.h"
#include "float.h"
#include "math.h"


int cmpfun_matchpts_ID(const void *a, const void *b) {
	const CorrespondPt *pt1 = (const CorrespondPt *)a;
	const CorrespondPt *pt2 = (const CorrespondPt *)b;
	return strcmp(pt1->ID, pt2->ID);
}

int cmpfun_matchpts_nrefid(const void *a, const void *b) {
	const CorrespondPt *pt1 = (const CorrespondPt *)a;
	const CorrespondPt *pt2 = (const CorrespondPt *)b;
	return pt1->nrefid - pt2->nrefid;
}

DomRectifyPro::DomRectifyPro(string strdom, string strmatchfile, vector<string> strrefs, string strresult)
{
	m_strdom = strdom; 
	m_strmatchfile = strmatchfile;
	m_strresult = strresult;
	m_strrefs = strrefs;
	m_matchImage = nullptr;
	m_prefImages = nullptr;

	m_matchptlist.clear(); m_matchptlist.reserve(50000); 
	init();
}

DomRectifyPro::DomRectifyPro(CString strdom, CString strmatchfile, vector<CString> strrefs, CString strresult)
{
	m_strdom = strdom;
	m_strmatchfile = strmatchfile;
	m_strresult = strresult;
	for (int i = 0; i < strrefs.size(); i++)
	{
		m_strrefs.push_back(strrefs[i].GetBuffer());
	}
	m_matchImage = nullptr;
	m_prefImages = nullptr;

	m_matchptlist.clear(); m_matchptlist.reserve(50000);
	init();
}

void DomRectifyPro::LoadData(CString strdom, CString strmatchfile, vector<CString> strrefs, CString strresult)
{
	m_strdom = strdom;
	m_strmatchfile = strmatchfile;
	m_strresult = strresult;
	for (int i = 0; i < strrefs.size(); i++)
	{
		m_strrefs.push_back(strrefs[i].GetBuffer());
	}
	m_matchImage = nullptr;
	m_prefImages = nullptr;

	m_matchptlist.clear(); m_matchptlist.reserve(50000);
	init();
}

DomRectifyPro::~DomRectifyPro()
{
	releaseImages();
	m_matchptlist.clear();
	freeTri(&m_tri);
}

void DomRectifyPro::releaseImages()
{
	if (m_matchImage) { delete m_matchImage; m_matchImage = nullptr; }
	if (m_prefImages) { delete[]m_prefImages; m_prefImages = nullptr; }
}

bool DomRectifyPro::init()
{
	if (_access(m_strdom.c_str(), 0)) return false;
	if (_access(m_strmatchfile.c_str(), 0)) return false;
	for (int i = 0; i < m_strrefs.size(); i++) if (_access(m_strdom.c_str(), 0)) return false;

	releaseImages();
	m_matchImage = new CSpVZImageCache; 
	if (!m_matchImage->Open(m_strdom.c_str(), CIdpmImage::modeRead)) { m_matchImage->Close(); releaseImages(); return false; }
	m_nrefImages = m_strrefs.size(); m_prefImages = new CIdpmImage[m_nrefImages];
	for (int i = 0; i < m_strrefs.size(); i++)
		if (!m_prefImages[i].Open(m_strrefs[i].c_str(), CIdpmImage::modeRead)) { m_prefImages[i].Close(); releaseImages();  return false; }

	if(!readmatchfile(m_strmatchfile)) return false;

	m_nrealmatchpts = m_matchptlist.size();

	//右影像重投影到左影像坐标系
	sortmatchpts_nrefid();
	reprojectionToLeftCoordinate(m_matchImage, m_prefImages, m_nrefImages, m_matchptlist.data(), m_matchptlist.size());
	//增加点并构建三角网(都位左影像坐标系)
	expandCorrespondPts_left(m_matchImage->GetRows(), m_matchImage->GetCols(), m_matchptlist);
	//以右影像的点来进行构建三角网
	sortmatchpts_ID();
	constructTin_left(m_matchptlist.data(), m_matchptlist.size(), &m_tri, false);

	return true;
}

bool DomRectifyPro::readmatchfile(string strmatchfile)
{
	FILE *pFile = fopen(m_strmatchfile.c_str(), "r");
	if (pFile == NULL) return false;

	char temp[512] = { 0 }; int nMatchNum = 0;
	fgets(temp, 512, pFile); fgets(temp, 512, pFile);
	fgets(temp, 512, pFile); sscanf(temp, "%d\n", &nMatchNum);
	if (nMatchNum < 3)  return false;

	m_matchptlist.clear(); m_matchptlist.resize(nMatchNum); 
	for (int i = 0; i < nMatchNum; i++) {
		fgets(temp, 512, pFile);
		sscanf(temp, "%s %f %f %f %f %d", (m_matchptlist[i].ID), &(m_matchptlist[i].lx), &(m_matchptlist[i].ly), &(m_matchptlist[i].rx), &(m_matchptlist[i].ry), &(m_matchptlist[i].nrefid));
	}
	
	fclose(pFile);
	return true;
}

void DomRectifyPro::sortmatchpts_ID()
{
	qsort(m_matchptlist.data(), m_matchptlist.size(), sizeof(CorrespondPt), cmpfun_matchpts_ID);
}

void DomRectifyPro::sortmatchpts_nrefid()
{
	qsort(m_matchptlist.data(), m_matchptlist.size(), sizeof(CorrespondPt), cmpfun_matchpts_nrefid);
}

int DomRectifyPro::findmatchpt(char * pID)
{
	int nstart = 0, nend = m_nrealmatchpts - 1, nmean = (nstart + nend) / 2;
	int nsuceed = 0;

	while (nstart <= nend)
	{
		if (strcmp(m_matchptlist[nstart].ID, pID) == 0) return nstart;
		if (strcmp(m_matchptlist[nend].ID, pID) == 0) return nend;

		nsuceed = strcmp(m_matchptlist[nmean].ID, pID);
		if (nsuceed == 0) {
			return nmean;
		}
		else if (nsuceed < 0) {
			nstart = nmean + 1; nend--;  if (nstart > nend) return -1;
			nmean = (nstart + nend) / 2;
		}
		else{
			nstart++;; nend = nmean -1;  if (nstart > nend) return -1;
			nmean = (nstart + nend) / 2;
		}
	}
	return -1;
}

void DomRectifyPro::updatetriangulateio()
{
	if (m_deleteptindex.size() > 0 || m_addpt.size() > 0) {
		//先删除，再添加，不能颠倒顺序
		for (int i = 0; i < m_deleteptindex.size(); i++) {
			m_matchptlist.erase(m_matchptlist.begin() + m_deleteptindex[i]); m_nrealmatchpts--;
		}
		m_deleteptindex.clear();
		for (int i = 0; i < m_addpt.size(); i++) {
			m_matchptlist.insert(m_matchptlist.begin() + m_nrealmatchpts, m_addpt[i]); m_nrealmatchpts++;
		}
		m_addpt.clear();

		expandCorrespondPts_left(m_matchImage->GetRows(), m_matchImage->GetCols(), m_matchptlist);
		//以右影像的点来进行构建三角网
		constructTin_left(m_matchptlist.data(), m_matchptlist.size(), &m_tri, false);
	}
	
}

void DomRectifyPro::addmatchpt(CorrespondPt * pt)
{
	reprojectionToLeftCoordinate(m_matchImage, m_prefImages, m_nrefImages, pt, 1);

	//反投影回去
	double lx[4], ly[4], rx[3], ry[3], t[3], a[6];
	int j3, j, k; double tx, ty;
	int tinidx = GetPtWithinTin(pt->lx, pt->ly, m_tri, m_matchptlist.data());
	if (tinidx != -1) {
		j3 = tinidx * 3;
		for (k = 0; k < 3; k++) {
			j = m_tri.triList[j3 + k];
			lx[k] = pt[j].lx; ly[k] = pt[j].ly;
			rx[k] = pt[j].rx; ry[k] = pt[j].ry;
		}
		CalcAffine(rx, ry, lx, ly, a);
		tx = a[0] + pt->lx*a[1] + pt->ly*a[2];
		ty = a[3] + pt->lx*a[4] + pt->ly*a[5];
		pt->lx = tx; pt->ly = ty;
		m_addpt.push_back(*pt);
	}

}

void DomRectifyPro::deletematchpts(vector<char*> pIDlist)
{
	for (int i = 0; i < pIDlist.size(); i++)
	{
		char id[16]; strcpy(id, pIDlist[i]);
		int nfind = findmatchpt(id);
		if (nfind != -1) {
		//	m_matchptlist.erase(m_matchptlist.begin() + nfind); m_nrealmatchpts--;
			m_deleteptindex.push_back(nfind);
		}
		else {
			for (int j = 0; j < m_addpt.size(); j++){
				if (strcmp(m_addpt[j].ID, id) == 0) {
					m_addpt.erase(m_addpt.begin() + j); break;
				}
			}
		}
	}

}

void DomRectifyPro::reprojectionToLeftCoordinate(CIdpmImage *matchimages, CIdpmImage *refimages, int nref, CorrespondPt *pCpt, int npt)
{
	int currefid = 0; double gx, gy, tx, ty, gx1, gy1;
	CIdpmImage *refImage = refimages + currefid;
	for (int i = 0; i < npt; i++){
		if (pCpt[i].nrefid != currefid) {
			currefid = pCpt[i].nrefid;
			refImage = refimages + currefid;
		}

		tx = pCpt[i].rx; ty = pCpt[i].ry;
		refImage->ImageToGeoProj(tx, ty, &gx, &gy);
		matchimages->GeoProjToImage(gx, gy, &tx, &ty);
		pCpt[i].rx = tx; pCpt[i].ry = ty;
	}
}

void DomRectifyPro::constructTin_left(const CorrespondPt * pt, int pn, triangulateio * tri, bool bleft)
{
	//if(tri) freeTri(tri);

	// 构建三角网
	memset(tri, 0, sizeof(tri));

	int i;
	REAL* pXY = new REAL[pn * 2];
	memset(pXY, 0, sizeof(REAL)*pn * 2);
	REAL* ptr = pXY;
	for (i = 0; i < pn; i++, ptr += 2) {
		if (bleft) {
			*(ptr + 0) = pt[i].lx;
			*(ptr + 1) = pt[i].ly;
		}
		else {
			*(ptr + 0) = pt[i].rx;
			*(ptr + 1) = pt[i].ry;
		}
	}

	triangulateio in, tmp;
	memset(&in, 0, sizeof(in)); *tri = tmp = in;
	in.numOfPoints = pn;
	in.pointList = pXY; pXY = NULL;

	triangulate("pczAenVQ", &in, tri, &tmp);
	freeTri(&tmp);
	freeTri(&in);
}

void DomRectifyPro::expandCorrespondPts_left(int lrow, int lcol, CorrespondPt ** pCpt, int * npt)
{
	// 构建三角网
	int i, j, k, j3, srow = 0, trow = 1024;
	int nTin; int *pTinIdx = NULL;
	triangulateio tri; constructTin_left(*pCpt, *npt, &tri);
	//按照格网补点
	int grid = 500;
	double ry = 0.0, rx = 0.0;
	double lx, ly;
	double gx, gy;
	CorrespondPt *gridPt = new CorrespondPt[100000]; int ngridPt = 0;

	while (srow < lrow)
	{
		if (srow + trow > lrow) { trow = lrow - srow; }
		nTin = GetTinInRect(0, srow, lcol - 1, srow + trow - 1, tri, &pTinIdx);

		for (i = 0; i < trow; i = i + grid)
		{
			ly = (i + srow) * 1.0;
			if (i >= trow - 1) ly = (trow - 1 + srow) * 1.0;
			for (j = 0; j < lcol; j = j + grid)
			{
				lx = j * 1.0;
				if (j > lcol - 1) lx = (lcol - 1) * 1.0;
				if (IsPointInTin(lx, ly, &tri, pTinIdx, nTin)) continue;

				sprintf(gridPt[ngridPt].ID, "GridPoint%d", ngridPt);
				gridPt[ngridPt].lx = (float)lx; gridPt[ngridPt].ly = (float)ly;
				gridPt[ngridPt].rx = (float)lx; gridPt[ngridPt].ry = (float)ly;
				ngridPt++;
			}
		}

		srow += trow;
	}

	if (ngridPt > 0) {
		CorrespondPt* pTemp = *pCpt;
		*pCpt = new CorrespondPt[*npt + ngridPt];
		memcpy(*pCpt, pTemp, sizeof(CorrespondPt)*(*npt));
		memcpy(*pCpt + *npt, gridPt, sizeof(CorrespondPt)*ngridPt);
		delete[]pTemp; pTemp = NULL;
	}
	freeTri(&tri);
	*npt = ngridPt + *npt;
	delete[]gridPt; gridPt = NULL;
}

void DomRectifyPro::expandCorrespondPts_left(int lrow, int lcol, vector<CorrespondPt>& ptlist)
{
	if (m_nrealmatchpts < ptlist.size()) ptlist.erase(ptlist.begin() + m_nrealmatchpts, ptlist.end());

	// 构建三角网
	int i, j, k, j3, srow = 0, trow = 1024;
	int nTin; int *pTinIdx = NULL;
	triangulateio tri; constructTin_left(ptlist.data(), ptlist.size(), &tri);
	//按照格网补点
	int grid = 100;
	double ry = 0.0, rx = 0.0;
	double lx, ly;
	double gx, gy;
	CorrespondPt *gridPt = new CorrespondPt[100000]; int ngridPt = 0;

	while (srow < lrow)
	{
		if (srow + trow > lrow) { trow = lrow - srow; }
		nTin = GetTinInRect(0, srow, lcol - 1, srow + trow - 1, tri, &pTinIdx);

		for (i = 0; i < trow; i = i + grid)
		{
			ly = (i + srow) * 1.0;
			if (i >= trow - 1) ly = (trow - 1 + srow) * 1.0;
			for (j = 0; j < lcol; j = j + grid)
			{
				lx = j * 1.0;
				if (j > lcol - 1) lx = (lcol - 1) * 1.0;
				if (IsPointInTin(lx, ly, &tri, pTinIdx, nTin)) continue;

				sprintf(gridPt[ngridPt].ID, "CSDGridPoint%04d", ngridPt); //在匹配点后面补
				gridPt[ngridPt].lx = (float)lx; gridPt[ngridPt].ly = (float)ly;
				gridPt[ngridPt].rx = (float)lx; gridPt[ngridPt].ry = (float)ly;
				gridPt[ngridPt].nrefid = no_referimage;
				ngridPt++;
			}
		}

		srow += trow;
	}

	ptlist.insert(ptlist.end(), gridPt, gridPt + ngridPt);
	delete[]gridPt; gridPt = NULL;
}

int DomRectifyPro::GetTinInRect(double xl, double yb, double xr, double yt, triangulateio & tin, int ** pTinIdx)
{
	{
		if (*pTinIdx != NULL) {
			delete[] * pTinIdx; *pTinIdx = NULL;
		}

		int i, j, k, j3;
		REAL x, y;
		int inNum = 0;
		int nMaxSize = 1024, nTin = 0;
		int* pIdx = new int[nMaxSize];
		memset(pIdx, 0, sizeof(int)*nMaxSize);

		for (i = 0; i < tin.numOfTriangles; i++) {
			j3 = i * 3;
			inNum = 0;
			for (k = 0; k < 3; k++) {
				j = tin.triList[j3 + k];
				x = tin.pointList[j * 2 + 0];
				y = tin.pointList[j * 2 + 1];
				if (x >= xl && x <= xr && y >= yb && y <= yt) {
					inNum++;
				}
			}

			if (inNum == 0) {
				continue;
			}

			if (nTin >= nMaxSize) {
				nMaxSize += 1024;
				int* po = pIdx;
				pIdx = new int[nMaxSize];
				memset(pIdx, 0, sizeof(int)*nMaxSize);
				memcpy(pIdx, po, sizeof(int)*nTin);
				delete[] po; po = NULL;
			}

			pIdx[nTin++] = i;
		}

		*pTinIdx = pIdx;
		return nTin;
	}
}

int DomRectifyPro::GetTinInRect_right(double xl, double yb, double xr, double yt, triangulateio & tin, int ** pTinIdx, CorrespondPt * pt)
{
	if (*pTinIdx != NULL) {
		delete[] * pTinIdx; *pTinIdx = NULL;
	}

	int i, j, k, j3;
	REAL x, y;
	int inNum = 0;
	int nMaxSize = 1024, nTin = 0;
	int* pIdx = new int[nMaxSize];
	memset(pIdx, 0, sizeof(int)*nMaxSize);

	for (i = 0; i < tin.numOfTriangles; i++) {
		j3 = i * 3;
		inNum = 0;
		for (k = 0; k < 3; k++) {
			j = tin.triList[j3 + k];
			x = pt[j].rx;
			y = pt[j].ry;
			if (x >= xl && x <= xr && y >= yb && y <= yt) {
				inNum++;
			}
		}

		if (inNum == 0) {
			continue;
		}

		if (nTin >= nMaxSize) {
			nMaxSize += 1024;
			int* po = pIdx;
			pIdx = new int[nMaxSize];
			memset(pIdx, 0, sizeof(int)*nMaxSize);
			memcpy(pIdx, po, sizeof(int)*nTin);
			delete[] po; po = NULL;
		}

		pIdx[nTin++] = i;
	}

	*pTinIdx = pIdx;
	return nTin;
}

int DomRectifyPro::GetPtWithinTin(double x, double y, triangulateio & tri, CorrespondPt *pt)
{
	int *pTinIdx = NULL;
	int nTin = GetTinInRect(x-10, y -10, x + 10, y + 10, tri, &pTinIdx);

	double rx[3], ry[3], t[3]; int j3;
	double x0[4], y0[4];
	for (int i = 0; i < nTin; i++){
		j3 = pTinIdx[i] * 3;
		for (int k = 0; k < 3; k++) {
			int j = tri.triList[j3 + k];
			rx[k] = pt[j].rx;
			ry[k] = pt[j].ry;
		}
	
		//判断点是否在三角形内部
		for (int j = 0; j < 3; j++){
			x0[j] = rx[j] - x; y0[j] = ry[j] - y;
		}
		x0[3] = x0[0]; y0[3] = y0[0];
		for (int j = 0; j < 3; j++) {
			t[j] = x0[j] * y0[j+1] - x0[j+1] * y0[j];
		}
		if (t[0] * t[1] >= zero && t[0] * t[2] >= zero) {
			if (pTinIdx) { delete[]pTinIdx; pTinIdx = NULL; }
			return pTinIdx[i];
		}
	}
	if (pTinIdx) { delete[]pTinIdx; pTinIdx = NULL; }
	return -1;
}

bool DomRectifyPro::IsPointInTin(double px, double py, triangulateio * tin, int * pTinIdx, int nTin)
{
	int i, j, k, j3;
	double x[3], y[3], dx[3], dy[3], t1, t2, t3;
	for (i = 0; i < nTin; i++) {
		j3 = pTinIdx[i];
		for (k = 0; k < 3; k++) {
			j = tin->triList[j3 + k];
			x[k] = tin->pointList[j * 2 + 0];
			y[k] = tin->pointList[j * 2 + 1];
			dx[k] = px - x[k];
			dy[k] = py - y[k];

			if (fabs(dx[k]) < 5. && fabs(dy[k]) < 5.) return true; //避免点与三角网挨得太近
		}
		t1 = (x[1] - x[0]) * dy[0] - (y[1] - y[0]) * dx[0];
		t2 = (x[2] - x[1]) * dy[1] - (y[2] - y[1]) * dx[1];
		t3 = (x[0] - x[2]) * dy[2] - (y[0] - y[2]) * dx[2];
		if (t1 >= zero && t2 >= zero && t3 >= zero) return true;
	}
	return false;
}

void DomRectifyPro::CalcAffine(double * x1, double * y1, double * x2, double * y2, double * a)
{
	double t, r[9];
	a[0] = 0.0f; a[1] = 1.0f; a[2] = 0.0f;
	a[3] = 0.0f; a[4] = 0.0f; a[5] = 1.0f;

	r[0] = x1[1] * y1[2] - x1[2] * y1[1];
	r[1] = x1[2] * y1[0] - x1[0] * y1[2];
	r[2] = x1[0] * y1[1] - y1[0] * x1[1];
	r[3] = y1[1] - y1[2];
	r[4] = y1[2] - y1[0];
	r[5] = y1[0] - y1[1];
	r[6] = x1[2] - x1[1];
	r[7] = x1[0] - x1[2];
	r[8] = x1[1] - x1[0];
	t = r[0] + r[1] + r[2];
	a[0] = (r[0] * x2[0] + r[1] * x2[1] + r[2] * x2[2]) / t;
	a[1] = (r[3] * x2[0] + r[4] * x2[1] + r[5] * x2[2]) / t;
	a[2] = (r[6] * x2[0] + r[7] * x2[1] + r[8] * x2[2]) / t;
	a[3] = (r[0] * y2[0] + r[1] * y2[1] + r[2] * y2[2]) / t;
	a[4] = (r[3] * y2[0] + r[4] * y2[1] + r[5] * y2[2]) / t;
	a[5] = (r[6] * y2[0] + r[7] * y2[1] + r[8] * y2[2]) / t;
}

void DomRectifyPro::rectifyDomImages()
{
	updatetriangulateio();

	//小面元纠正
	CorrespondPt* pt = m_matchptlist.data();
	int nRows, nCols, nPxlBytes, nDataType;
	nRows = m_matchImage->GetRows(); nCols = m_matchImage->GetCols();
	nPxlBytes = m_matchImage->GetBands(); nDataType = m_matchImage->GetDataType();

	//创建新影像，拷贝地理信息文件
	CIdpmImage desImage; if (!desImage.Open(m_strresult.c_str(), CIdpmImage::modeCreate)) { desImage.Close(); return; }
	desImage.SetRows(nRows); desImage.SetCols(nCols);
	desImage.SetBands(nPxlBytes); desImage.SetDataType((CIdpmImage::RASTERDATATYPE)nDataType);
	double *pref = m_matchImage->GetGeoTransform(NULL); desImage.SetGeoTransform(pref);

	const int NROWS = min(nRows, Mem2Rows(64, nCols*nPxlBytes));
	int stRow = 0, pcRow = NROWS;
	int* pTinIdx = NULL;
	int nTin = 0;
	double xl, xr, yb, yt, x, y;
	double lx[4], ly[4], rx[3], ry[3], t[3], a[6];
	double dx[3], dy[3], area;
	int yi, xi;
	double xc[4], yc;
	int n;
	int i, j, k, j3;
	void* pBuffer = NULL;
	switch (nDataType)
	{
	case CIdpmImage::RDT_BYTE:
		pBuffer = new unsigned char[nCols*NROWS*nPxlBytes];
		break;
	case CIdpmImage::RDT_UINT16:
		pBuffer = new unsigned char[nCols*NROWS*nPxlBytes];
		break;
	default:
		break;
	}

	while (stRow < nRows) {
		pcRow = NROWS;
		if (stRow + pcRow > nRows) pcRow = nRows - stRow;

		//初始化,获取相应三角网以及相应的影像块
		memset(pBuffer, 0, sizeof(unsigned char)*nCols*NROWS*nPxlBytes);
		nTin = GetTinInRect(0, stRow, nCols - 1, stRow + pcRow - 1, m_tri, &pTinIdx);
		xl = yb = DBL_MAX; xr = yt = -DBL_MAX;
		for (i = 0; i < nTin; i++) {
			j3 = pTinIdx[i] * 3;
			for (k = 0; k < 3; k++) {
				j = m_tri.triList[j3 + k];
				x = pt[j].lx; y = pt[j].ly;
				xl = min(xl, x); xr = max(xr, x);
				yb = min(yb, y); yt = max(yt, y);
			}
		}
		m_matchImage->FillCache(int(xl - 32), int(yb - 32), int(xr + 32), int(yt + 32));

		for (i = 0; i < nTin; i++) {
			j3 = pTinIdx[i] * 3;
			xl = yb = DBL_MAX; xr = yt = -DBL_MAX;
			for (k = 0; k < 3; k++) {
				j = m_tri.triList[j3 + k];
				lx[k] = pt[j].lx; ly[k] = pt[j].ly;
				rx[k] = pt[j].rx; ry[k] = pt[j].ry;
				xl = min(xl, lx[k]); xr = max(xr, lx[k]);
				yb = min(yb, ly[k]); yt = max(yt, ly[k]);
			}

			for (j = 0; j < 2; j++) {
				dx[j] = lx[j + 1] - lx[j];
				dy[j] = ly[j + 1] - ly[j];
			}
			area = dx[0] * dy[1] - dx[1] * dy[0];
			if (fabs(area) < zero) { continue; }

			CalcAffine(rx, ry, lx, ly, a);

			lx[3] = lx[0]; ly[3] = ly[0];
			for (j = 0; j < 3; j++) {
				t[j] = ly[j + 1] - ly[j];
				if (fabs(t[j]) > zero) {
					t[j] = (lx[j + 1] - lx[j]) / t[j];
				}
				else {
					t[j] = 0;
				}
			}

			int sR = max(stRow, int(floor(yb)));
			int eR = min(stRow + pcRow - 1, int(ceil(yt)));
			if (nDataType == CIdpmImage::RDT_BYTE)
			{
				unsigned char* pRow = (unsigned char*)pBuffer + nCols*nPxlBytes*(sR - stRow);
				for (yi = sR; yi <= eR; yi++, pRow += nCols*nPxlBytes) {
					n = 0;
					yc = yi;
					for (j = 0; j < 3; j++) {
						if (fabs(ly[j] - yc) < zero) {
							xc[n++] = lx[j];
							continue;
						}
						else if ((ly[j] - yc)*(ly[j + 1] - yc) < 0) {
							xc[n++] = lx[j] + (yc - ly[j])*t[j];
						}
					}

					if (n >= 2) {
						if (xc[0] > xc[1]) {
							double tmp = xc[0];
							xc[0] = xc[1];
							xc[1] = tmp;
						}
						int sC = max(0, int(floor(xc[0])));
						int eC = min(nCols - 1, int(ceil(xc[1])));
						unsigned char* pD = pRow + nPxlBytes*sC;
						unsigned char* pS = NULL;
						for (xi = sC; xi <= eC; xi++, pD += nPxlBytes) {
							x = a[0] + xi*a[1] + yi*a[2];
							y = a[3] + xi*a[4] + yi*a[5];
							pS = (unsigned char*)m_matchImage->GetPxl(float(x), float(y));
							// 影像有效范围为(1,254)
							for (j = 0; j < nPxlBytes; j++) {
								*pS = max(1, min(254, *pS));
							}
							memcpy(pD, pS, sizeof(unsigned char)*nPxlBytes);
						}
					}
				}
			}
			else
			{
				unsigned short* pRow = (unsigned short*)pBuffer + nCols*nPxlBytes*(sR - stRow);
				for (yi = sR; yi <= eR; yi++, pRow += nCols*nPxlBytes) {
					n = 0;
					yc = yi;
					for (j = 0; j < 3; j++) {
						if (fabs(ly[j] - yc) < zero) {
							xc[n++] = lx[j];
							continue;
						}
						else if ((ly[j] - yc)*(ly[j + 1] - yc) < 0) {
							xc[n++] = lx[j] + (yc - ly[j])*t[j];
						}
					}

					if (n >= 2) {
						if (xc[0] > xc[1]) {
							double tmp = xc[0];
							xc[0] = xc[1];
							xc[1] = tmp;
						}
						int sC = max(0, int(floor(xc[0])));
						int eC = min(nCols - 1, int(ceil(xc[1])));
						unsigned short* pD = pRow + nPxlBytes*sC;
						unsigned short* pS = NULL;
						for (xi = sC; xi <= eC; xi++, pD += nPxlBytes) {
							x = a[0] + xi*a[1] + yi*a[2];
							y = a[3] + xi*a[4] + yi*a[5];
							pS = (unsigned short*)m_matchImage->GetPxl(float(x), float(y));
							// 影像有效范围为(1,65535)
							for (j = 0; j < nPxlBytes; j++) {
								*pS = max(1, min(65536, *pS));
							}
							memcpy(pD, pS, sizeof(unsigned short)*nPxlBytes);
						}
					}
				}
			}
		}

		for (i = 0; i < pcRow; i++) {
			if (nDataType == CIdpmImage::RDT_BYTE)
				desImage.Write((unsigned char*)pBuffer + nCols*nPxlBytes*i, stRow + i);
			else
				desImage.Write((unsigned short*)pBuffer + nCols*nPxlBytes*i, stRow + i);
		}

		stRow += NROWS;
	}

	if (pTinIdx != NULL) { delete[] pTinIdx; pTinIdx = NULL; }
	if (pBuffer != NULL) { delete[] pBuffer; pBuffer = NULL; }
}

