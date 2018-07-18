#pragma once
#include <string>
#include <vector>
#include <map>
#include "Triangles.h"
#include "TinyFacetRectify.h"
using namespace std;

#define no_referimage 9999

#ifndef MATCHING_STRUCTURE
#define MATCHING_STRUCTURE
typedef struct tagCorrespondPt {
	char ID[16];
	int nrefid;
	float lx;
	float ly;
	float rx;
	float ry;

	tagCorrespondPt() {
		memset(ID, 0, sizeof(char)*16);
		nrefid = -1; 
		lx = ly = rx = ry = 0.0;
	}

}CorrespondPt;
#endif

class DomRectifyPro
{
public:
	DomRectifyPro() {};
	DomRectifyPro(string strdom,  string strmatchfile, vector<string> strrefs, string strresult);
	DomRectifyPro(CString strdom, CString strmatchfile, vector<CString> strrefs, CString strresult);
	void LoadData(CString strdom, CString strmatchfile, vector<CString> strrefs, CString strresult);
	~DomRectifyPro();

private:
	bool readmatchfile(string strmatchfile);  //导入连接点
	void sortmatchpts_ID();                   //按匹配点id进行排序
	void sortmatchpts_nrefid();               //按底图id排序
	int findmatchpt(char *pID);               //按匹配点id进行二分法找点
	void updatetriangulateio();               //更新三角网
	bool init();                              //初始化，重投影，扩展点，构建三角网
	void reprojectionToLeftCoordinate(CIdpmImage *matchimages, CIdpmImage *refimages, int nref, CorrespondPt *pCpt, int npt); //匹配点的底图坐标重投影到dom影像坐标系
	void constructTin_left(const CorrespondPt *pt, int pn, triangulateio *tri, bool bleft = true); //利用左影像坐标建立三角网
	void expandCorrespondPts_left(int lrow, int lcol, CorrespondPt** pCpt, int *npt);    //扩展匹配点，覆盖全区域
	void expandCorrespondPts_left(int lrow, int lcol, vector<CorrespondPt> &ptlist);    //扩展匹配点，覆盖全区域
	int GetTinInRect(double xl, double yb, double xr, double yt, triangulateio& tin, int** pTinIdx);	//获取某一区域的三角网
	int GetTinInRect_right(double xl, double yb, double xr, double yt, triangulateio& tin, int** pTinIdx, CorrespondPt *pt);	//获取某一区域的三角网
	int GetPtWithinTin(double x, double y, triangulateio& tri, CorrespondPt *pt); //获取点所在TIN
	bool IsPointInTin(double px, double py, triangulateio *tin, int *pTinIdx, int nTin); //判断某点在某一区域三角网中				   
	void CalcAffine(double* x1, double* y1, double* x2, double* y2, double* a);//计算仿射变换参数
	void releaseImages();

public:	
	void addmatchpt(CorrespondPt *pt, bool borgphoto = false);        //增加匹配点，该点的id比已有点更大
	void deletematchpts(vector<char *> pIDlist);  //删除匹配点

	void rectifyDomImages(); //纠正影像,直接调用
	//影像块纠正
	template <typename T>
	//利用原始影像纠正
	void smallareaTinyFacet(int slrow, int slcol, float fzoom, int lrows, int lcols, T* plindata, T** poutdata, int nBkgrdClr = 0);

public:
	string m_strdom;
	string m_strmatchfile;
	string m_strresult;
	vector<string> m_strrefs;

	vector<CorrespondPt> m_matchptlist;
	vector<CorrespondPt> m_addpt;
	vector<int> m_deleteptindex;
	int m_nrealmatchpts; //匹配点+手工刺点的数量
	triangulateio m_tri;

	CSpVZImageCache *m_matchImage;
	CIdpmImage *m_prefImages;
	int m_nrefImages;
};

template<typename T>
inline void DomRectifyPro::smallareaTinyFacet(int slrow, int slcol, float fzoom, int lrows, int lcols, T * plindata,T ** poutdata, int nBkgrdClr)
{
	int nPxlBytes = m_matchImage->GetBands();
	//保存
	CIdpmImage inphoto;
	inphoto.Open("C:\\Users\\Dabo\\Desktop\\indata.tif", CIdpmImage::modeCreate);
	inphoto.SetBands(nPxlBytes); inphoto.SetRows(lrows); inphoto.SetCols(lcols);
	double *at = m_matchImage->GetGeoTransform();
	inphoto.SetGeoTransform(at); 
	for (int i = 0; i < lrows; i++){
		inphoto.Write(plindata + i * lcols * nPxlBytes, i);
	}
	inphoto.Close();
	
	updatetriangulateio();

	int nrange = 254;
	if (sizeof(T) == sizeof(unsigned short)) nrange = 65535;
	//小面元纠正
	//if (*poutdata) { delete[](*poutdata); *poutdata = NULL; }
	T *pdata = new T[lrows*lcols*nPxlBytes];
	memcpy(pdata, plindata, sizeof(T)*lrows*lcols*nPxlBytes);

	//将匹配点进行转换
	int npt = m_matchptlist.size(); CorrespondPt *pt = new CorrespondPt[npt];
	memcpy(pt, m_matchptlist.data(), sizeof(CorrespondPt)*npt);
	CorrespondPt *pttemp = pt; 
	for (int i = 0; i < npt; i++, pttemp++){
		//pttemp->lx = (pttemp->lx - slrow) * fzoom; pttemp->ly = (pttemp->ly - slcol) * fzoom;
		//pttemp->rx = (pttemp->rx - slrow) * fzoom; pttemp->ry = (pttemp->ry - slcol) * fzoom;
		pttemp->lx = pttemp->lx  * fzoom; pttemp->ly = pttemp->ly * fzoom;
		pttemp->rx = pttemp->rx  * fzoom; pttemp->ry = pttemp->ry * fzoom;
	}

	int* pTinIdx = NULL;
	int nTin = GetTinInRect_right(0, 0, lrows, lcols, m_tri, &pTinIdx, pt);

	double lx[4], ly[4], rx[3], ry[3], t[3], a[6];
	double dx[3], dy[3], area;
	for (int i = 0; i < nTin; i++)
	{
		for (int k = 0; k < 3; k++) {
			int j = m_tri.triList[i * 3 + k];
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

		T* pRow = (T*)pdata;
		double xc[4], yc, xi, x, y;
		//for (int yi = 0; yi <= lrows; yi++, pRow += lcols*nPxlBytes) {
		for (int yi = 0; yi < lrows; yi++, pRow += lcols*nPxlBytes) {
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
						*pS = max(1, min(nrange, (int)*pS));
					}
					memcpy(pD, pS, sizeof(T)*nPxlBytes);
				}
			}
		}
	}

	//if (pTinIdx != NULL) { delete[] pTinIdx; pTinIdx = NULL; }
	if (pt) { delete[]pt; pt = NULL; }
	*poutdata = pdata;

	//保存
	CIdpmImage outphoto;
	outphoto.Open("C:\\Users\\Dabo\\Desktop\\oudata.tif", CIdpmImage::modeCreate);
	outphoto.SetBands(nPxlBytes); outphoto.SetRows(lrows); outphoto.SetCols(lcols);
	outphoto.SetGeoTransform(at);
	for (int i = 0; i < lrows; i++){
		outphoto.Write(pdata + i * lcols * nPxlBytes, i);
	}
	outphoto.Close();
}
