#include "stdafx.h"
#include "GeometryOP.h"
#include <io.h>
#include <algorithm>
#include "IBFS.h"


template<typename T> void Gradient(T* pData, int nCols, int nRows)
{
	if (!pData) return;
	if (nCols < 3 || nRows < 3) return;
	int nLen = nCols * nRows;
	T* pTemp = new T[nLen];
	memset(pTemp, 0, nLen);
	int max = 0, min = 255;
	for (int i = 0; i < nRows; ++i)
	{
		for (int j = 0; j < nCols; ++j)
		{
			int nIdx = i * nCols + j;
			if (i == 0)
			{
				pTemp[nIdx] = abs(pData[nIdx] - pData[nIdx + nCols]);
				continue;
			}
			else if (i == nRows - 1)
			{
				pTemp[nIdx] = abs(pData[nIdx] - pData[nIdx - nCols]);
				continue;
			}
			else if (j == 0)
			{
				pTemp[nIdx] = abs(pData[nIdx] - pData[nIdx + 1]);
				continue;
			}
			else if (j == nCols - 1)
			{
				pTemp[nIdx] = abs(pData[nIdx] - pData[nIdx - 1]);
				continue;
			}
			else
			{
				pTemp[nIdx] = abs(4 * pData[nIdx] - pData[nIdx + nCols] - pData[nIdx - nCols] - pData[nIdx + 1] - pData[nIdx - 1]);
			}
		}
	}
	memcpy(pData, pTemp, nCols * nRows * sizeof(T));
	delete pTemp; pTemp = NULL;
}

void GraphCut(int* pCost, BYTE* pMask, int* pIndex, int nCols, int nRows, BYTE nSrc, BYTE nDst, BYTE nNone, BYTE nSrcDst)
{
	if (!pCost || !pMask || !pIndex)
	{
		return;
	}

	int nDataLen = nCols * nRows;
	for (int i = 0; i < nDataLen; ++i)
	{
		pIndex[i] = -1;
	}

	/************************************************************************/
	/*					 避免出现没有源点或者汇点的情况                       */
	/************************************************************************/
	bool bNoSrc = true;
	bool bNoDst = true;
	for (int i = 0; i < nDataLen; ++i)
	{
		if (pMask[i] == nSrc) bNoSrc = false;
		if (pMask[i] == nDst) bNoDst = false;
	}

	if (bNoDst)
	{
		for (int i = 0; i < nDataLen; ++i)
		{
			if (pMask[i] == nSrcDst) pMask[i] = nSrc;
		}
		return;
	}

	if (bNoSrc)
	{
		for (int i = 0; i < nDataLen; ++i)
		{
			if (pMask[i] == nSrcDst) pMask[i] = nDst;
		}
		return;
	}
	/************************************************************************/
	/*					  计算像素节点数量和边数量                            */
	/************************************************************************/

	int nPixelNodeCount = 0;
	int nPixelEdgeCount = 0;
	int nIdx = 0, nIdx1 = 0/*LEFT*/, nIdx2 = 0/*RIGHT*/, nIdx3 = 0/*TOP*/, nIdx4 = 0/*BOTTOM*/;
	//计算顶点数量
	for (int i = 0; i < nRows; ++i)
	{
		for (int j = 0; j < nCols; ++j)
		{
			nIdx = i * nCols + j;
			nIdx1 = nIdx - 1; if (j == 0) nIdx1 = nIdx;
			nIdx2 = nIdx + 1; if (j == nCols - 1) nIdx2 = nIdx;
			nIdx3 = nIdx - nCols; if (i == 0) nIdx3 = nIdx;
			nIdx4 = nIdx + nCols; if (i == nRows - 1) nIdx4 = nIdx;
			if (pMask[nIdx] == nNone)
			{
				continue;
			}
			else if (pMask[nIdx] == nSrcDst)
			{
				pIndex[nIdx] = nPixelNodeCount;
				nPixelNodeCount++;
			}
			else if (pMask[nIdx1] == nSrcDst ||
				pMask[nIdx2] == nSrcDst ||
				pMask[nIdx3] == nSrcDst ||
				pMask[nIdx4] == nSrcDst)
			{
				pIndex[nIdx] = nPixelNodeCount;
				nPixelNodeCount++;
			}
		}
	}
	// 计算边数量
	for (int i = 0; i < nRows; ++i)
	{
		for (int j = 0; j < nCols; ++j)
		{
			nIdx = i * nCols + j;
			nIdx2 = nIdx + 1;
			nIdx4 = nIdx + nCols;
			if (j < nCols - 1)
			{
				if (pIndex[nIdx] != -1 && pIndex[nIdx2] != -1) nPixelEdgeCount++;
			}
			if (i < nRows - 1)
			{
				if (pIndex[nIdx] != -1 && pIndex[nIdx4] != -1) nPixelEdgeCount++;
			}

		}
	}

	/************************************************************************/
	/*						     构建图                                     */
	/************************************************************************/
	IBFS::IBFSGraph* g = new IBFS::IBFSGraph;
	g->initSize(nPixelNodeCount, nPixelEdgeCount);
	//添加源边和汇边
	int nSrcNodeCount = 0, nDstNodeCount = 0, nMidNodeCount = 0;
	for (int i = 0; i < nRows; ++i)
	{
		for (int j = 0; j < nCols; ++j)
		{
			nIdx = i * nCols + j;
			if (pIndex[nIdx] == -1)
			{
				continue;
			}
			if (pMask[nIdx] == nSrc)
			{
				g->addNode(pIndex[nIdx], FLT_MAX, 0);
				nSrcNodeCount++;
			}
			else if (pMask[nIdx] == nDst)
			{
				g->addNode(pIndex[nIdx], 0, FLT_MAX);
				nDstNodeCount++;
			}
			else
			{
				g->addNode(pIndex[nIdx], 0, 0);
				nMidNodeCount++;
			}

		}
	}

	//添加普通边
	int nCost = 0;
	int nRealEdgeCount = 0;
	for (int i = 0; i < nRows; ++i)
	{
		for (int j = 0; j < nCols; ++j)
		{
			nIdx = i * nCols + j;
			nIdx2 = nIdx + 1;
			nIdx4 = nIdx + nCols;
			if (j < nCols - 1)
			{
				if (pIndex[nIdx] != -1 && pIndex[nIdx2] != -1)
				{
					nCost = pCost[nIdx] + pCost[nIdx2];
					if (nCost <= 0) nCost = 1;
					g->addEdge(pIndex[nIdx], pIndex[nIdx2], nCost, nCost);
					nRealEdgeCount++;
				}
			}
			if (i < nRows - 1)
			{
				if (pIndex[nIdx] != -1 && pIndex[nIdx4] != -1)
				{
					nCost = pCost[nIdx] + pCost[nIdx4];
					if (nCost <= 0) nCost = 1;
					g->addEdge(pIndex[nIdx], pIndex[nIdx4], nCost, nCost);
					nRealEdgeCount++;
				}
			}
		}
	}

	/************************************************************************/
	/*						     图割优化                                    */
	/************************************************************************/
	g->initGraph();
	g->computeMaxFlow();
	/************************************************************************/
	/*							 重新标记                                    */
	/************************************************************************/
	for (int i = 0; i < nRows; ++i)
	{
		for (int j = 0; j < nCols; ++j)
		{
			nIdx = i * nCols + j;
			if (pIndex[nIdx] == -1)
			{
				continue;
			}
			if (pMask[nIdx] == nSrcDst)
			{
				if (g->isNodeOnSrcSide(pIndex[nIdx]))
					pMask[nIdx] = nSrc;
				else
					pMask[nIdx] = nDst;
			}
		}
	}
	delete g; g = NULL;
}

/*-----过滤重复点-----*/
void GeometryFilter(OGRLineString* pGeometry, OGRGeometry*& pFilterGeometry)
{
	if (!pGeometry)
	{
		return;
	}
	pFilterGeometry = (OGRLineString*)OGRGeometryFactory::createGeometry(wkbLineString);
	int i = 0, j = 0;
	for (i = 0; i < pGeometry->getNumPoints(); ++i)
	{
		if (i == 0)
		{
			((OGRLineString*)pFilterGeometry)->addPoint(pGeometry->getX(i), pGeometry->getY(i));
		}
		else
		{
			for (j = 0; j < ((OGRLineString*)pFilterGeometry)->getNumPoints(); ++j)
			{
				if (pGeometry->getX(i) == ((OGRLineString*)pFilterGeometry)->getX(j) && pGeometry->getY(i) == ((OGRLineString*)pFilterGeometry)->getY(j))
					break;
			}
			if (j == ((OGRLineString*)pFilterGeometry)->getNumPoints())
			{
				((OGRLineString*)pFilterGeometry)->addPoint(pGeometry->getX(i), pGeometry->getY(i));
			}
		}
	}
}

void GeometryFilter(OGRLinearRing* pGeometry, OGRGeometry* &pFilterGeometry)
{
	if (!pGeometry)
	{
		return;
	}
	pFilterGeometry = (OGRLinearRing*)OGRGeometryFactory::createGeometry(wkbLinearRing);
	int i = 0, j = 0;
	for (i = 0; i < pGeometry->getNumPoints(); ++i)
	{
		if (i == 0)
		{
			((OGRLinearRing*)pFilterGeometry)->addPoint(pGeometry->getX(i), pGeometry->getY(i));
		}
		else
		{
			for (j = 0; j < ((OGRLinearRing*)pFilterGeometry)->getNumPoints(); ++j)
			{
				if (abs(pGeometry->getX(i) - ((OGRLinearRing*)pFilterGeometry)->getX(j)) < 1e-6 && abs(pGeometry->getY(i) - ((OGRLinearRing*)pFilterGeometry)->getY(j)) < 1e-6)
					break;
			}
			if (j == ((OGRLinearRing*)pFilterGeometry)->getNumPoints())
			{
				((OGRLinearRing*)pFilterGeometry)->addPoint(pGeometry->getX(i), pGeometry->getY(i));
			}
		}
	}
	((OGRLinearRing*)pFilterGeometry)->closeRings();
}

void GeometryFilter(OGRPolygon* pGeometry, OGRGeometry* &pFilterGeometry)
{
	if (!pGeometry)
	{
		return;
	}
	pFilterGeometry = (OGRPolygon*)OGRGeometryFactory::createGeometry(wkbPolygon);
	OGRGeometry* pLinearRing = NULL;
	GeometryFilter(pGeometry->getExteriorRing(), pLinearRing);

	if (pLinearRing)
	{
		((OGRPolygon*)pFilterGeometry)->addRing((OGRLinearRing*)pLinearRing);
	}

	int nInterRingCount = pGeometry->getNumInteriorRings();
	for (int i = 0; i < nInterRingCount; ++i)
	{
		pLinearRing = NULL;
		GeometryFilter(pGeometry->getInteriorRing(i), pLinearRing);
		if (pLinearRing)
		{
			((OGRPolygon*)pFilterGeometry)->addRing((OGRLinearRing*)pLinearRing);
		}
	}
}

void GeometryFilter(OGRMultiPolygon* pGeometry, OGRGeometry* &pFilterGeometry)
{
	if (!pGeometry)
	{
		return;
	}
	pFilterGeometry = (OGRMultiPolygon*)OGRGeometryFactory::createGeometry(wkbMultiPolygon);
	OGRGeometry* pPolygon = NULL;
	int nPolygonCount = pGeometry->getNumGeometries();
	for (int i = 0; i < nPolygonCount; ++i)
	{
		pPolygon = NULL;
		GeometryFilter((OGRPolygon*)pGeometry->getGeometryRef(i), pPolygon);
		if (pPolygon)
		{
			((OGRMultiPolygon*)pFilterGeometry)->addGeometry(pPolygon);
		}
	}
}

void GeometryFilter(OGRMultiPoint* pGeometry, OGRGeometry* &pFilterGeometry)
{
	pFilterGeometry = pGeometry->clone();
}

void GeometryFilter(OGRMultiLineString* pMultiLineString, OGRGeometry* &pFilterMultiLineString)
{
	if (!pMultiLineString)
	{
		return;
	}
	pFilterMultiLineString = (OGRMultiLineString*)OGRGeometryFactory::createGeometry(wkbMultiLineString);
	for (int i = 0; i < pMultiLineString->getNumGeometries(); ++i)
	{
		OGRGeometry* pTemp = NULL;
		GeometryFilter(pMultiLineString->getGeometryRef(i), pTemp);
		((OGRMultiLineString*)pFilterMultiLineString)->addGeometry(pTemp);
	}
}

void GeometryFilter(OGRGeometry* pGeometry, OGRGeometry* &pFilterGeometry)
{
	if (!pGeometry)
	{
		return;
	}
	OGRwkbGeometryType type = pGeometry->getGeometryType();
	if (type == wkbPoint)
	{
		return;
	}
	else if (type == wkbLineString)
	{
		GeometryFilter((OGRLineString*)pGeometry, pFilterGeometry);
	}
	else if (type == wkbMultiLineString)
	{
		int nLineStringCount = ((OGRMultiLineString*)pGeometry)->getNumGeometries();
		GeometryFilter((OGRMultiLineString*)pGeometry, pFilterGeometry);
	}
	else if (type == wkbLinearRing)
	{
		GeometryFilter((OGRLinearRing*)pGeometry, pFilterGeometry);
	}
	else if (type == wkbPolygon)
	{
		GeometryFilter((OGRPolygon*)pGeometry, pFilterGeometry);
	}
	else if (type == wkbPolygon25D)
	{
		GeometryFilter((OGRPolygon*)pGeometry, pFilterGeometry);
	}
	else if (type == wkbMultiPolygon)
	{
		GeometryFilter((OGRMultiPolygon*)pGeometry, pFilterGeometry);
	}
	else if (type == wkbMultiPoint25D)
	{
		GeometryFilter((OGRMultiPoint*)pGeometry, pFilterGeometry);
	}
}


/*-----几何体缩放-----*/
void GeometryZoom(const string& srcfilename, const string& dstfilename, double scale)
{
	/*-----文件不存在-----*/
	if (_access(srcfilename.c_str(), 0) == -1)
	{
		return;
	}

	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");

	GDALDriver* pSHPDriver = GetGDALDriverManager()->GetDriverByName("ESRI Shapefile");
	if (pSHPDriver == NULL)
	{
		return;
	}

	string tempdstfilename = dstfilename;
	if (dstfilename == srcfilename)
	{
		string temp = "_Temp";
		tempdstfilename.insert(tempdstfilename.end() - 4, temp.begin(), temp.end());
	}

	GDALDataset* pSrcDataSet = (GDALDataset*)GDALOpenEx(srcfilename.c_str(), GA_ReadOnly, NULL, NULL, NULL);
	if (pSrcDataSet == NULL)
	{
		return;
	}
	GDALDataset* pDstDataSet = (GDALDataset*)GDALCreate(pSHPDriver, tempdstfilename.c_str(), 0, 0, 0, GDT_Unknown, 0);
	if (pDstDataSet == NULL)
	{
		return;
	}

	int iLayerCount = pSrcDataSet->GetLayerCount();
	int i = 0, j = 0, k = 0, q = 0;
	for (i = 0; i < iLayerCount; ++i)
	{
		OGRLayer* pSrcLayer = pSrcDataSet->GetLayer(i);
		OGRLayer* pDstLayer = pDstDataSet->CreateLayer("_", pSrcLayer->GetSpatialRef(), wkbUnknown);
		int iFeatureCount = pSrcLayer->GetFeatureCount();
		for (j = 0; j < iFeatureCount; ++j)
		{
			OGRFeature* pFeature = pSrcLayer->GetFeature(j);
			OGRGeometry* pGeometry = pFeature->GetGeometryRef();
			if (!pGeometry)
			{
				continue;
			}
			OGRPoint centerPoint;
			pGeometry->Centroid(&centerPoint);

			if (pGeometry->getGeometryType() == wkbPolygon)
			{
				OGRPolygon* pPolygon = (OGRPolygon*)pGeometry;
				OGRLinearRing* pExterRing = pPolygon->getExteriorRing();
				if (pExterRing)
				{
					int iExterPointCount = pExterRing->getNumPoints();
					for (k = 0; k < iExterPointCount; ++k)
					{
						pExterRing->setPoint(k, (pExterRing->getX(k) - centerPoint.getX()) * scale + centerPoint.getX(), (pExterRing->getY(k) - centerPoint.getY()) * scale + centerPoint.getY());
					}
				}
				int iInnerRingCount = pPolygon->getNumInteriorRings();
				for (k = 0; k < iInnerRingCount; ++k)
				{
					OGRLinearRing* pInnerRing = pPolygon->getInteriorRing(k);
					int iInnerPointCount = pInnerRing->getNumPoints();
					if (!pInnerRing)  continue;
					for (q = 0; q < iInnerPointCount; ++q)
					{
						pInnerRing->setPoint(q, (pInnerRing->getX(q) - centerPoint.getX()) * scale + centerPoint.getX(), (pInnerRing->getY(q) - centerPoint.getY()) * scale + centerPoint.getY());
					}
				}
			}
			else
			{
				return;
			}
			pDstLayer->CreateFeature(pFeature);
		}
	}
	GDALClose(pSrcDataSet);
	GDALClose(pDstDataSet);

	/*-----名称替换-----*/
	if (srcfilename == dstfilename)
	{
		pSHPDriver->Delete(srcfilename.c_str());
		string srcshxfilename = srcfilename; srcshxfilename.erase(srcshxfilename.end() - 3, srcshxfilename.end()); srcshxfilename.append("shx");
		string srcdbffilename = srcfilename; srcdbffilename.erase(srcdbffilename.end() - 3, srcdbffilename.end()); srcdbffilename.append("dbf");
		string tempdstshxfilename = tempdstfilename; tempdstshxfilename.erase(tempdstshxfilename.end() - 3, tempdstshxfilename.end()); tempdstshxfilename.append("shx");
		string tempdstdbffilename = tempdstfilename; tempdstdbffilename.erase(tempdstdbffilename.end() - 3, tempdstdbffilename.end()); tempdstdbffilename.append("dbf");
		rename(tempdstfilename.c_str(), srcfilename.c_str());
		rename(tempdstshxfilename.c_str(), srcshxfilename.c_str());
		rename(tempdstdbffilename.c_str(), srcdbffilename.c_str());
	}
}


/*-----平移几何体-----*/
void GeometryTranslate(OGRPolygon* ppolygon, double x, double y)
{
	if (ppolygon == NULL) return;
	OGRLinearRing* pExterRing = ppolygon->getExteriorRing();
	if (pExterRing)
	{
		int iExtervertexCount = pExterRing->getNumPoints();
		for (int i = 0; i < iExtervertexCount; ++i) pExterRing->setPoint(i, pExterRing->getX(i) + x, pExterRing->getY(i) + y);
	}
	int iInnerRingCount = ppolygon->getNumInteriorRings();
	for (int i = 0; i < iInnerRingCount; ++i)
	{
		OGRLinearRing* pInnerRing = ppolygon->getInteriorRing(i);
		if (pInnerRing)
		{
			int iInnerVerticesCount = pInnerRing->getNumPoints();
			for (int j = 0; j < iInnerVerticesCount; ++j) pInnerRing->setPoint(j, pInnerRing->getX(j) + x, pInnerRing->getY(j) + y);
		}
	}
}

void GeometryTranslate(OGRMultiPolygon* ppolygon, double x, double y)
{
	if (ppolygon == NULL)return;
	int iPolygonCount = ppolygon->getNumGeometries();
	for (int i = 0; i < iPolygonCount; ++i) GeometryTranslate((OGRPolygon*)ppolygon->getGeometryRef(i), x, y);
}

void GeometryTranslate(OGRLineString* plinestring, double x, double y)
{
	int iPointCount = plinestring->getNumPoints();
	for (int i = 0; i < iPointCount; ++i)
	{
		plinestring->setPoint(i, plinestring->getX(i) + x, plinestring->getY(i) + y);
	}
}

void GeometryTranslate(OGRGeometry* pGeometry, double x, double y)
{
	if (pGeometry == NULL)
	{
		return;
	}

	OGRwkbGeometryType ogrType = pGeometry->getGeometryType();
	if (ogrType == wkbPoint)
	{
		OGRPoint* pPoint = (OGRPoint *)pGeometry;
		pPoint->setX(pPoint->getX() + x);
		pPoint->setY(pPoint->getY() + y);
	}
	else if (ogrType == wkbLineString)
	{
		GeometryTranslate((OGRLineString *)pGeometry, x, y);
	}
	else if (ogrType == wkbPolygon)
	{
		GeometryTranslate((OGRPolygon*)pGeometry, x, y);
	}
	else if (ogrType == wkbMultiPolygon)
	{
		GeometryTranslate((OGRMultiPolygon*)pGeometry, x, y);
	}
	else if (ogrType == wkbLinearRing)
	{
	}
	else if (ogrType == wkbMultiPoint)
	{
	}
	else if (ogrType == wkbMultiPolygon)
	{

	}
	else if (ogrType == wkbGeometryCollection)
	{
		OGRGeometryCollection* pGeometryCollection = (OGRGeometryCollection*)pGeometry;
		int iGeometryCount = pGeometryCollection->getNumGeometries();
		for (int i = 0; i < iGeometryCount; ++i)
		{
			GeometryTranslate(pGeometryCollection->getGeometryRef(i), x, y);
		}
	}
}

/*-----2D布尔运算-----*/
bool Intersect(const OGREnvelope& e1, const OGREnvelope& e2, OGREnvelope& cross)
{
	/*-----两个矩形没有交集-----*/
	if (e1.MinX >= e2.MaxX || e2.MinX >= e1.MaxX)
	{
		return false;
	}
	if (e1.MinY >= e2.MaxY || e2.MinY >= e1.MaxY)
	{
		return false;
	}
	/*-----矩形存在交集-----*/
	cross.MaxX = e1.MaxX > e2.MaxX ? e2.MaxX : e1.MaxX;
	cross.MinX = e1.MinX > e2.MinX ? e1.MinX : e2.MinX;
	cross.MaxY = e1.MaxY > e2.MaxY ? e2.MaxY : e1.MaxY;
	cross.MinY = e1.MinY > e2.MinY ? e1.MinY : e2.MinY;
	return true;
}

bool Intersect(const OGREnvelope& e1, const OGREnvelope& e2)
{
	/*-----两个矩形没有交集-----*/
	if (e1.MinX >= e2.MaxX || e2.MinX >= e1.MaxX)
	{
		return false;
	}
	if (e1.MinY >= e2.MaxY || e2.MinY >= e1.MaxY)
	{
		return false;
	}
	return true;
}

void Union(const OGREnvelope& e1, const OGREnvelope& e2, OGREnvelope& e)
{
	e.MinX = e1.MinX > e2.MinX ? e2.MinX : e1.MinX;
	e.MaxX = e1.MaxX > e2.MaxX ? e1.MaxX : e2.MaxX;
	e.MinY = e1.MinY > e2.MinY ? e2.MinY : e1.MinY;
	e.MaxY = e1.MaxY > e2.MaxY ? e1.MaxY : e2.MaxY;
}

bool compare(OGRPolygon* pPolygon1, OGRPolygon* pPolygon2)
{
	if (pPolygon1 == NULL || pPolygon2 == NULL)
	{
		return true;
	}
	if (pPolygon1->getExteriorRing()->getNumPoints() > pPolygon2->getExteriorRing()->getNumPoints())
	{
		return true;
	}
	return false;
}

void GeometryCompress(OGRLinearRing* pLinearRing, OGRLinearRing*& pCompressLinearRing)
{
	if (!pLinearRing)
		return;
	int nVerticesCount = pLinearRing->getNumPoints();
	std::vector<bool> bDel(nVerticesCount, false);
	for (int i = 1; i < nVerticesCount - 1; ++i)
	{
		OGRPoint pt0, pt1, pt2;
		pLinearRing->getPoint(i - 1, &pt0);
		pLinearRing->getPoint(i + 0, &pt1);
		pLinearRing->getPoint(i + 1, &pt2);
		if ((pt1.getX() == pt0.getX() && pt1.getX() == pt2.getX()) || (pt1.getY() == pt0.getY() && pt1.getY() == pt2.getY()))
		{
			bDel[i] = true;
		}
	}
	pCompressLinearRing = (OGRLinearRing*)OGRGeometryFactory::createGeometry(wkbLinearRing);
	for (int i = 0; i < nVerticesCount; ++i)
	{
		if (!bDel[i])
			pCompressLinearRing->addPoint(pLinearRing->getX(i), pLinearRing->getY(i));
	}
	pCompressLinearRing->closeRings();
}

void GeometryCompress(OGRPolygon* pPolygon, OGRPolygon*& pCompressPolygon)
{
	if (!pPolygon)
		return;
	pCompressPolygon = (OGRPolygon *)OGRGeometryFactory::createGeometry(wkbPolygon);
	OGRLinearRing* pExRing = pPolygon->getExteriorRing();
	OGRLinearRing* pCompressExLinearRing = NULL;
	GeometryCompress(pExRing, pCompressExLinearRing);
	pCompressPolygon->addRing(pCompressExLinearRing);
	OGRGeometryFactory::destroyGeometry(pCompressExLinearRing);
	int nInRingCount = pPolygon->getNumInteriorRings();
	for (int i = 0; i < nInRingCount; ++i)
	{
		OGRLinearRing* pInRing = pPolygon->getInteriorRing(i);
		OGRLinearRing* pCompressInLinearRing = NULL;
		GeometryCompress(pInRing, pCompressInLinearRing);
		pCompressPolygon->addRing(pCompressInLinearRing);
		OGRGeometryFactory::destroyGeometry(pCompressInLinearRing);
	}
}

void GeometryCompress(OGRMultiPolygon* pMultiPolygon, OGRMultiPolygon*& pCompressMultiPolygon)
{
	if (!pMultiPolygon)
	{
		return;
	}
	int nGeometryCount = pMultiPolygon->getNumGeometries();
	pCompressMultiPolygon = (OGRMultiPolygon*)OGRGeometryFactory::createGeometry(wkbMultiPolygon);
	for (int i = 0; i < nGeometryCount; ++i)
	{
		OGRPolygon* pPolygon = (OGRPolygon*)pMultiPolygon->getGeometryRef(i);
		OGRPolygon* pCompressPolygon = NULL;
		GeometryCompress(pPolygon, pCompressPolygon);
		OGRErr err = pCompressMultiPolygon->addGeometry(pCompressPolygon);
		OGRGeometryFactory::destroyGeometry(pCompressPolygon);
	}
}

void GeometryCompress(OGRLineString* pLineString, OGRLineString*& pCompressLineString)
{

}

void GeometryCompress(OGRGeometry* pGeometry, OGRGeometry*& pCompressGeometry)
{
	if (!pGeometry)
	{
		return;
	}
	const char* pName = pGeometry->getGeometryName();
	if (strcmp(pName, "LINEARRING") == 0)
	{
		GeometryCompress((OGRLinearRing*)pGeometry, (OGRLinearRing*&)pCompressGeometry);
	}
	else if (strcmp(pName, "LINESTRING") == 0)
	{
		GeometryCompress((OGRLineString*)pGeometry, (OGRLineString*&)pCompressGeometry);
	}
	else if (strcmp(pName, "POLYGON") == 0)
	{
		GeometryCompress((OGRPolygon*)pGeometry, (OGRPolygon*&)pCompressGeometry);
	}
	else if (strcmp(pName, "MULTIPOLYGON") == 0)
	{
		GeometryCompress((OGRMultiPolygon*)pGeometry, (OGRMultiPolygon*&)pCompressGeometry);
	}
}

OGRPolygon* EnvelopeToPolygon(const OGREnvelope& enve)
{
	OGRLinearRing* pRing = (OGRLinearRing*)OGRGeometryFactory::createGeometry(wkbLinearRing);
	pRing->addPoint(enve.MinX, enve.MinY);
	pRing->addPoint(enve.MaxX, enve.MinY);
	pRing->addPoint(enve.MaxX, enve.MaxY);
	pRing->addPoint(enve.MinX, enve.MaxY);
	pRing->closeRings();
	OGRPolygon* pPolygon = (OGRPolygon*)OGRGeometryFactory::createGeometry(wkbPolygon);
	pPolygon->addRing(pRing);
	return pPolygon;
}

OGRLinearRing* LineString2LinearRing(OGRLineString* pLineString)
{
	if (!pLineString)
	{
		return NULL;
	}
	OGRLinearRing* pLinearRing = (OGRLinearRing*)OGRGeometryFactory::createGeometry(wkbLinearRing);
	int nPointCount = pLineString->getNumPoints();
	OGRPoint pt;
	for (int i = 0; i < nPointCount; ++i)
	{
		pLineString->getPoint(1, &pt);
		pLinearRing->addPoint(&pt);
	}
	pLinearRing->closeRings();
	return pLinearRing;
}

OGREnvelope GetRasterEnvelope(GDALDataset* pDataSet)
{
	OGREnvelope enve;
	enve.MinX = enve.MaxX = enve.MinY = enve.MaxY = 0.0;
	if (!pDataSet)
	{
		return enve;
	}
	int nCols = pDataSet->GetRasterXSize();
	int nRows = pDataSet->GetRasterYSize();
	double GeoTrans[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
	pDataSet->GetGeoTransform(GeoTrans);
	enve.MinX = GeoTrans[0];
	enve.MaxX = GeoTrans[0] + GeoTrans[1] * nCols;
	enve.MaxY = GeoTrans[3];
	enve.MinY = GeoTrans[3] + GeoTrans[5] * nRows;
	return enve;
}

