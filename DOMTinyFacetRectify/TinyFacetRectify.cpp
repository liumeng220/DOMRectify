#include "TinyFacetRectify.h"

//利用仿射变换参数进行坐标转换
void photoToground(const double tfw[6], double ix, double iy, double &gx, double &gy)
{
	if (tfw[1] < zero && tfw[2] < zero) {
		gx = tfw[0] * ix + tfw[4];
		gy = tfw[3] * iy + tfw[5];
	}
	else {
		gx = tfw[0] * ix + tfw[1] * iy + tfw[4];
		gy = tfw[2] * ix + tfw[3] * iy + tfw[5];
	}	
}

void groundTophoto(const double tfw[6], double gx, double gy, double &ix, double &iy)
{
	if (tfw[1] < zero && tfw[2] < zero) {
		ix = (gx - tfw[4]) / tfw[0];
		iy = (gy - tfw[5]) / tfw[3];
	}
	else {
		ix = ((gx - tfw[4]) * tfw[3] - (gy - tfw[5]) * tfw[1]) / (tfw[0] * tfw[3] - tfw[2] * tfw[1]);
		iy = ((gx - tfw[4]) * tfw[2] - (gy - tfw[5]) * tfw[0]) / (tfw[1] * tfw[2] - tfw[0] * tfw[3]);
	}	
}

//利用同名点构建三角网
void constructTin(const CorrespondPt *pt, int pn, triangulateio *tri, bool bleft /* = true*/)
{
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
};

int GetTinInRect(double xl, double yb, double xr, double yt, triangulateio& tin, int** pTinIdx)
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

bool IsPointInTin(double px, double py, triangulateio *tin, int *pTinIdx, int nTin)
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
};

void expandCorrespondPts(int lrow, int lcol, CorrespondPt** pCpt, int *npt)
{
	printf("expand CorrespondPts\n\n");
	
	// 构建三角网
	int i, j, k, j3, srow = 0, trow = 1024;
	int nTin; int *pTinIdx = NULL;
	triangulateio tri; constructTin(*pCpt, *npt, &tri);
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

void expandCorrespondPts(const double *ltfw, const double *rtfw, int lrows, int lcols, int slrow, int slcol, CorrespondPt** pCpt, int *npt)
{
	printf("expand CorrespondPts\n\n");
	
	// 构建三角网
	int i, j, k, j3;
	int nTin; int *pTinIdx = NULL;
	triangulateio tri; constructTin(*pCpt, *npt, &tri);
	//按照格网补点
	int grid = 100;
	double ry = 0.0, rx = 0.0;
	double lx, ly;
	double gx, gy;
	CorrespondPt *gridPt = new CorrespondPt[100000]; int ngridPt = 0;

	for (i = 0; i < lrows; i = i + grid)
	{
		ly = (i + slrow) * 1.0;
		if (i >= lrows - 1) ly = (lrows - 1 + slrow) * 1.0;
		for (j = 0; j < lcols; j = j + grid)
		{
			lx = (j + slrow) * 1.0;
			if (j > lcols - 1) lx = (lcols - 1 + slcol) * 1.0;
			if (IsPointInTin(lx, ly, &tri, pTinIdx, nTin)) continue;

	//		photoToground(ltfw, lx, ly, gx, gy);
	//		groundTophoto(rtfw, gx, gy, rx, ry);

			sprintf(gridPt[ngridPt].ID, "GridPoint%d", ngridPt);
			gridPt[ngridPt].lx = (float)lx; gridPt[ngridPt].ly = (float)ly;
			gridPt[ngridPt].rx = (float)lx; gridPt[ngridPt].ry = (float)ly;
			ngridPt++;
		}
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

void reprojectionToSameCoordinate(const CIdpmImage* plImage, const CIdpmImage* prImage, CorrespondPt *pCpt, int npt)
{
	printf("reprojection To Same Coordinate\n\n");
	double gx, gy, tx, ty, gx1, gy1;
	for (int i=0; i<npt; i++)
	{
		tx = pCpt[i].rx; ty = pCpt[i].ry;
		prImage->ImageToGeoProj(tx, ty, &gx, &gy);
		plImage->GeoProjToImage(gx, gy, &tx, &ty);
		pCpt[i].rx = tx; pCpt[i].ry = ty;
	}
}

void reprojectionToSameCoordinate(const double* ltfw, const double* rtfw, CorrespondPt *pCpt, int npt)
{
	printf("reprojection To Same Coordinate\n\n");
	double gx, gy, tx, ty;
	for (int i = 0; i < npt; i++)
	{
		tx = pCpt[i].rx; ty = pCpt[i].ry;
		photoToground(rtfw, tx, ty, gx, gy);
		groundTophoto(ltfw, gx, gy, tx, ty);
		pCpt[i].rx = tx; pCpt[i].ry = ty;
	}
}

void CalcAffine(double* x1, double* y1, double* x2, double* y2, double* a)
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

bool TinyFacetRectify(const char * lpstrCpPts, const char * lpstrTarImage)
{
	//导入文件
	CorrespondPt* pt = NULL;
	int pn = 0;
	char lpstrRefImage[512] = "", lpstrMatchImage[512] = "";
	if (!ReadFromCptFile((char*)lpstrCpPts, lpstrMatchImage, lpstrRefImage, &pt, &pn)) { return false; }
	if (pn < 20) { return false; }
	for (int i = 0; i < strlen(lpstrMatchImage); i++) { if (lpstrMatchImage[i] == '/')lpstrMatchImage[i] == '\\'; }
	for (int i = 0; i < strlen(lpstrRefImage); i++) { if (lpstrRefImage[i] == '/')lpstrRefImage[i] == '\\'; }

	//读取影像
	int nRows, nCols, nPxlBytes, nDataType;
	CIdpmImage drefImage; if (!drefImage.Open(lpstrRefImage, CIdpmImage::modeRead)) { drefImage.Close(); delete[]pt; pt = NULL; return false; }
	CSpVZImageCache srcImage; if (!srcImage.Open(lpstrMatchImage, CIdpmImage::modeRead)) { srcImage.Close(); delete[]pt; pt = NULL; return false; }
	nRows = srcImage.GetRows(); nCols = srcImage.GetCols();
	nPxlBytes = srcImage.GetBands(); nDataType = srcImage.GetDataType();
	//创建新影像，拷贝地理信息文件
	CIdpmImage desImage; if (!desImage.Open(lpstrTarImage, CIdpmImage::modeCreate)) { srcImage.Close(); drefImage.Close(); delete[]pt; pt = NULL; return false; }
	desImage.SetRows(nRows); desImage.SetCols(nCols);
	desImage.SetBands(nPxlBytes); desImage.SetDataType((CIdpmImage::RASTERDATATYPE)nDataType);
	double *pref = srcImage.GetGeoTransform(NULL); desImage.SetGeoTransform(pref);

	//右影像重投影到左影像
	reprojectionToSameCoordinate(&srcImage, &drefImage, pt, pn);
	drefImage.Close();
	//增加点并构建三角网(都位左影像坐标系)
	expandCorrespondPts(nRows, nCols, &pt, &pn);
	triangulateio tri; constructTin(pt, pn, &tri, false);
	
	//小面元纠正
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
		nTin = GetTinInRect(0, stRow, nCols - 1, stRow + pcRow - 1, tri, &pTinIdx);
		xl = yb = DBL_MAX; xr = yt = -DBL_MAX;
		for (i = 0; i < nTin; i++) {
			j3 = pTinIdx[i] * 3;
			for (k = 0; k < 3; k++) {
				j = tri.triList[j3 + k];
				x = pt[j].lx; y = pt[j].ly;
				xl = min(xl, x); xr = max(xr, x);
				yb = min(yb, y); yt = max(yt, y);
			}
		}
		srcImage.FillCache(int(xl - 32), int(yb - 32), int(xr + 32), int(yt + 32));

		for (i = 0; i < nTin; i++) {
			j3 = pTinIdx[i] * 3;
			xl = yb = DBL_MAX; xr = yt = -DBL_MAX;
			for (k = 0; k < 3; k++) {
				j = tri.triList[j3 + k];
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

			int sR = max(stRow,
				int(floor(yb)));
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
							pS = (unsigned char*)srcImage.GetPxl(float(x), float(y));
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
							pS = (unsigned short*)srcImage.GetPxl(float(x), float(y));
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

	srcImage.Close();

	if (pTinIdx != NULL) { delete[] pTinIdx; pTinIdx = NULL; }
	if (pBuffer != NULL) { delete[] pBuffer; pBuffer = NULL; }
	freeTri(&tri);
	if (pt); {delete[]pt; pt = NULL; }
	desImage.Close();
	return true;
}


