#pragma once
#include "stdafx.h"
#include "IdpmImage\IdpmImage.h"
#include "Triangles.h"
#include "stdlib.h"
#include "stdio.h"
#include "conio.h"
#include "float.h"
#include "math.h"
#include "CorrespondPtIO.h"

/************************************************************************/
/*                           左影像：待纠正影像                           */
/*                           右影像：底图DOM                             */
/************************************************************************/
/*                           lpstrCpPts格式                              */
/*                           待纠正影像路径                               */
/*                           底图DOM路径                                 */
/*                           匹配点数量及在左右影像上对应坐标              */
/************************************************************************/

#define zero 1.0e-10
#define MB	1024*1024

// #ifndef MATCHING_STRUCTURE
// #define MATCHING_STRUCTURE
// typedef struct tagCorrespondPt {
// 	char ID[32];
// 	float lx;
// 	float ly;
// 	float rx;
// 	float ry;
// }CorrespondPt;
// #endif

inline int Mem2Rows(int nMemMB, int rowBufSize) {
	int nRows = int(nMemMB*1024.0*1024.0 / rowBufSize);
	return int(nRows / 8) * 8;
};

template <typename T>
static inline T* GetBiPixelEx(float fx, float fy, int cols, int rows, int bands, T *pBuf, UINT bkGrd = 0xFFFFFFFF)
{
	static T backW[8] = { 0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0 };
	T *buf = backW; int r1, r2, r3, r4, col = int(fx), row = int(fy);
	if (col < 0 || row < 0 || col >= cols - 1 || row >= rows - 1) {
		UINT* pBak = (UINT*)backW;
		*pBak = *(pBak + 1) = bkGrd;
		return buf;
	}

	float dx = fx - col, dy = fy - row; int b;
	T *p, *p0 = pBuf + (row*cols + col)*bands;
	if (dx < 0.001f || dy < 0.001f) return p0;
	p = p0 + 0; r1 = *p; r2 = *(p + bands); p += cols*bands; r3 = *p; r4 = *(p + bands);
	buf[0] = (T)((1 - dx)*(1 - dy)*r1 + dx*(1 - dy)*r2 + (1 - dx)*dy*r3 + dx*dy*r4);
	for (b = 1; b < bands; b++) {
		p = p0 + b; r1 = *p; r2 = *(p + bands); p += cols*bands; r3 = *p; r4 = *(p + bands);
		buf[b] = (T)((1 - dx)*(1 - dy)*r1 + dx*(1 - dy)*r2 + (1 - dx)*dy*r3 + dx*dy*r4);
	}
	return buf;
};

template <typename T>
void ZoomOutImageData(T* pSrc, int w, int h, int x0, int y0, T* pTar, int bx, int by, int w1, int h1, int nPxlBytes, int zoom)
{
	T* p0 = NULL;
	T* p = pSrc + (y0*w + x0)*nPxlBytes;
	T* p1 = pTar + (by*w1 + bx)*nPxlBytes;
	float d = (float)(zoom*zoom);

	for (int i = 0; i < h1; i++) {
		for (int j = 0; j < w1; j++) {
			for (int b = 0; b < nPxlBytes; b++) {
				p0 = p + (i*w + j)*zoom*nPxlBytes + b;

				int all = 0;
				for (int k = 0; k < zoom; k++) {
					for (int l = 0; l < zoom; l++) {
						all += *(p0 + l*nPxlBytes);
					}
					p0 += w*nPxlBytes;
				}
				*(p1 + j*nPxlBytes + b) = (T)(all / d);
			}
		}
		p1 += w1*nPxlBytes;
	}
};

class CSpVZImageCache : public CIdpmImage
{
public:
	CSpVZImageCache()
	{
		m_pBuf = NULL;
		m_bufCs = 0;
		m_bufRs = 0;
		m_bufCols = 0;
		m_bufRows = 0;
		m_bufSize = 0;
		m_nBkgrdClr = 0;
		m_nZoomRate = 1;
	};
	~CSpVZImageCache()
	{
		ReleaseCache();
	};
	BOOL Open(LPCSTR lpstrPathName, UINT flag = modeRead, int maxBufSize = -1, CIdpmImage *pImgAcs = NULL)
	{
		cprintf("Open image [%s]\n", lpstrPathName);
		if (stricmp(m_strPathName, lpstrPathName) != 0)
		{
			if (!CIdpmImage::Open(lpstrPathName, modeRead, maxBufSize, pImgAcs))
				return FALSE;

			ReleaseCache();
			m_nZoomRate = 1;
			m_bufRs = m_bufRows = 0;
			m_bufCs = m_bufCols = 0;
			m_pBuf = NULL; m_bufSize = 0;
		}
		return TRUE;
	};
	bool FillCache(int sC, int sR, int eC, int eR, int nZoomRate = 1, int* pNewSize = NULL)
	{
		m_bufRs = sR; if (m_bufRs < 0) m_bufRs = 0;
		m_bufCs = sC; if (m_bufCs < 0) m_bufCs = 0;
		m_bufRows = eR - sR + 1; if (m_bufRs + m_bufRows > m_nRows) m_bufRows = m_nRows - m_bufRs;
		m_bufCols = eC - sC + 1; if (m_bufCs + m_bufCols > m_nCols) m_bufCols = m_nCols - m_bufCs;

		nZoomRate = max(nZoomRate, 1);
		m_bufRs /= nZoomRate;
		m_bufCs /= nZoomRate;
		m_bufRows = m_bufRows / nZoomRate + 1;
		if (m_bufRs + m_bufRows > m_nRows / nZoomRate) m_bufRows = m_nRows / nZoomRate - m_bufRs;
		m_bufCols = m_bufCols / nZoomRate + 1;
		if (m_bufCs + m_bufCols > m_nCols / nZoomRate) m_bufCols = m_nCols / nZoomRate - m_bufCs;

		int newSize = m_bufCols*m_bufRows*m_nBands;  ///?m_nPixelBytes
		if (pNewSize != NULL) *pNewSize = newSize;

		if (newSize < 0) {
			ReleaseCache();
			m_bufSize = 0;
			return false;
		}
		else if (m_bufSize < newSize) {
			m_bufSize = newSize + 128;
			ReleaseCache();
			try {
				if (GetDataType() == RDT_UINT16)
				{
					m_pBuf = new unsigned short[m_bufSize]; memset(m_pBuf, 0, sizeof(unsigned short)*m_bufSize);
				}
				else
				{
					m_pBuf = new unsigned char[m_bufSize]; memset(m_pBuf, 0, m_bufSize);
				}
			}
			catch (...) {
				cprintf("!!Failed to new %d MB memory.\n", m_bufSize / MB);
				m_bufSize = 0;
				return false;
			}
		}

		m_nZoomRate = nZoomRate;
		if (nZoomRate > 1) {
			const int NROWS = max(1, Mem2Rows(48, m_bufCols*m_nBands*nZoomRate*nZoomRate));
			int szBuffer = m_bufCols*NROWS*nZoomRate*nZoomRate*m_nBands;

			switch (GetDataType())
			{
			case RDT_UINT16:
			{
				unsigned short* pBuffer = new unsigned short[szBuffer];
				int stRow = 0, pcRow = NROWS;
				int nRows, nCols = m_bufCols*nZoomRate;
				while (stRow < m_bufRows) {
					pcRow = NROWS;
					if (stRow + pcRow > m_bufRows) pcRow = m_bufRows - stRow;

					memset(pBuffer, 0, sizeof(unsigned short)*szBuffer);
					nRows = pcRow*nZoomRate;
					Read(pBuffer, m_nBands, (m_bufRs + stRow)*nZoomRate, m_bufCs*nZoomRate,
						nRows, nCols, CIdpmImage::RDT_UINT16);
					ZoomOutImageData((unsigned short*)pBuffer, nCols, nRows, 0, 0,
						(unsigned short*)m_pBuf, 0, stRow, m_bufCols, pcRow,
						m_nBands, nZoomRate);

					stRow += NROWS;
				}
				if (pBuffer != NULL) delete[] pBuffer; pBuffer = NULL;
			}
			break;
			case RDT_BYTE:
			{
				unsigned char* pBuffer = new unsigned char[szBuffer];
				int stRow = 0, pcRow = NROWS;
				int nRows, nCols = m_bufCols*nZoomRate;
				while (stRow < m_bufRows) {
					pcRow = NROWS;
					if (stRow + pcRow > m_bufRows) pcRow = m_bufRows - stRow;

					memset(pBuffer, 0, sizeof(unsigned char)*szBuffer);
					nRows = pcRow*nZoomRate;
					Read(pBuffer, m_nBands, (m_bufRs + stRow)*nZoomRate, m_bufCs*nZoomRate,
						nRows, nCols);
					ZoomOutImageData((unsigned char*)pBuffer, nCols, nRows, 0, 0,
						(unsigned char*)m_pBuf, 0, stRow, m_bufCols, pcRow,
						m_nBands, nZoomRate);

					stRow += NROWS;
				}
				if (pBuffer != NULL) delete[] pBuffer; pBuffer = NULL;
			}
			break;
			default:
				break;
			}

		}
		else {
			switch (GetDataType())
			{
			case RDT_UINT16:
				Read(m_pBuf, m_nBands, m_bufRs, m_bufCs, m_bufRows, m_bufCols, RDT_UINT16);
				break;
			case RDT_BYTE:
				Read(m_pBuf, m_nBands, m_bufRs, m_bufCs, m_bufRows, m_bufCols);
				break;
			default:
				break;
			}

		}

		return true;
	};
	void ReleaseCache() {
		if (m_pBuf != NULL) {
			delete[] m_pBuf; m_pBuf = NULL;
		}
	};

	void* GetCache(int& bufSize)
	{
		bufSize = m_bufSize; return m_pBuf;
	};

	void* GetPxl(float x, float y)
	{
		if (m_pBuf == NULL) return (unsigned char*)&m_nBkgrdClr;

		x /= m_nZoomRate;
		y /= m_nZoomRate;
		if (GetDataType() == RDT_BYTE)
			return (unsigned char*)GetBiPixelEx(x - m_bufCs, y - m_bufRs, m_bufCols, m_bufRows, m_nBands, (unsigned char*)m_pBuf, m_nBkgrdClr);
		else
			return (unsigned short*)GetBiPixelEx(x - m_bufCs, y - m_bufRs, m_bufCols, m_bufRows, m_nBands, (unsigned short*)m_pBuf, m_nBkgrdClr);
	};

	void SetBackColor(int nClrIndex = 0)
	{
		m_nBkgrdClr = (nClrIndex == 0) ? 0 : 0xFFFFFFFF;
	};

	inline BOOL IsInRange(float x, float y)
	{
		x /= m_nZoomRate;
		y /= m_nZoomRate;
		return (x >= m_bufCs && x < m_bufCs + m_bufCols && y >= m_bufRs && y < m_bufRs + m_bufRows);
	};

protected:
	int   m_bufCs, m_bufRs, m_bufCols, m_bufRows, m_bufSize;
	void* m_pBuf;
	UINT  m_nBkgrdClr;
	int	  m_nZoomRate;
};

//坐标转换
inline void photoToground(const double tfw[6], double ix, double iy, double &gx, double &gy);
inline void groundTophoto(const double tfw[6], double gx, double gy, double &ix, double &iy);

//利用同名点构建三角网
void constructTin(const CorrespondPt *pt, int pn, triangulateio *tri, bool bleft = true);

//扩展匹配点,对左影像进行格网加点，再投影到右影像
void expandCorrespondPts(int lrow, int lcol, CorrespondPt** pCpt, int *npt);
void expandCorrespondPts(const double *ltfw, const double *rtfw, int lrows, int lcols, int slrow, int slcol, CorrespondPt** pCpt, int *npt);

//将右影像的点投影到左影像坐标系下
void reprojectionToSameCoordinate(const CIdpmImage* plImage, const CIdpmImage* prImage, CorrespondPt *pCpt, int npt);
void reprojectionToSameCoordinate(const double* ltfw, const double* rtfw, CorrespondPt *pCpt, int npt);

//计算仿射变换参数
void CalcAffine(double* x1, double* y1, double* x2, double* y2, double* a);

//同名点文件的读写
//bool ReadFromCptFile(const char* CptFileName, char pImglPath[512], char pImgrPath[512], CorrespondPt** CpPt, int* nMatchNum);

//影像级小面元纠正
//输出：左右影像路径/左右影像纠正后的坐标
bool TinyFacetRectify(const char* lpstrCpPts, const char* lpstrTarImage);

//影像块的小面元纠正
//外围一圈同名点的地面坐标完全一致
template <typename T>
void smallareaTinyFacet(const double *ltfw, const double *rtfw ,const T* plindata, int lrows, int lcols, int nPxlBytes, int slrow, int slcol, CorrespondPt* CpPt, int npt, T* poutdata, int nBkgrdClr = 0)
{
	CorrespondPt *pt = new CorrespondPt[npt];
	memcpy(pt, CpPt, sizeof(CorrespondPt)*npt);
	int pn = npt;

	//将右影像坐标投影到左影像，并利用左影像坐标构建三角网
	reprojectionToSameCoordinate(ltfw, rtfw, pt, pn);
	//增加点
	expandCorrespondPts(ltfw, rtfw, lrows, lcols, slrow, slcol, &pt, &pn);
	
	for (int i=0; i<pn; i++){
		pt[i].lx -= slcol; pt[i].rx -= slcol;
		pt[i].ly -= slrow; pt[i].ry -= slrow;
	}
	triangulateio tri; constructTin(pt, pn, &tri, false);

	int nrange = 254;
	if (sizeof(T) == sizeof(unsigned short)) nrange = 65535;
	//小面元纠正
	memcpy(poutdata, plindata, sizeof(T)*lrows*lcols*nPxlBytes);
	int nTin = tri.numOfTriangles;
	double lx[4], ly[4], rx[3], ry[3], t[3], a[6];
	double dx[3], dy[3], area;
	for (int i = 0; i < nTin; i++)
	{
		for (int k = 0; k < 3; k++) {
			int j = tri.triList[i * 3 + k];
			lx[k] = pt[j].lx;
			ly[k] = pt[j].ly;
			rx[k] = pt[j].rx;
			ry[k] = pt[j].ry;
		}

		for (int j = 0; j < 2; j++) {
			dx[j] = lx[j + 1] - lx[j];
			dy[j] = ly[j + 1] - ly[j];
		}
		area = dx[0] * dy[1] - dx[1] * dy[0];
		if (fabs(area) < zero) {
			continue;
		}

		CalcAffine(rx, ry, lx, ly, a);

		lx[3] = lx[0]; ly[3] = ly[0];
		for (int j = 0; j < 3; j++) {
			t[j] = ly[j + 1] - ly[j];
			if (fabs(t[j]) > zero) {
				t[j] = (lx[j + 1] - lx[j]) / t[j];
			}
			else {
				t[j] = 0;
			}
		}

		T* pRow = (T*)plindata;
		double xc[4], yc, xi, x, y;
		for (int yi = 0; yi <= lrows; yi++, pRow += lcols*nPxlBytes) {
			int n = 0;
			yc = yi;
			for (int j = 0; j < 3; j++) {
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
				int eC = min(lcols - 1, int(ceil(xc[1])));
				T* pD = pRow + nPxlBytes*sC;
				T* pS = NULL;
				for (xi = sC; xi <= eC; xi++, pD += nPxlBytes) {
					x = a[0] + xi*a[1] + yi*a[2];
					y = a[3] + xi*a[4] + yi*a[5];
					pS = (T*)GetBiPixelEx(float(x), float(y), lcols, lrows, nPxlBytes, (T*)plindata, nBkgrdClr);
					// 影像有效范围为(1,254)
					for (int j = 0; j < nPxlBytes; j++) {
						*pS = max(1, min(nrange, *pS));
					}
					memcpy(pD, pS, sizeof(T)*nPxlBytes);
				}
			}
		}
	}

	freeTri(&tri);

	for (int i=0; i<npt; i++)
	{
		CpPt[i].lx = pt[i].rx + slcol; 
		CpPt[i].ly = pt[i].ry + slrow;
	}
	if (pt) { delete[]pt; pt = NULL; }
}