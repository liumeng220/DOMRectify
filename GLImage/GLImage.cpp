// GLImage.cpp : 定义 DLL 的初始化例程。
//

#include "stdafx.h"

#include "windows.h"
#include "GLImage.h"
#include "GL/GLObjects.h"
#include "Geometry/GeometryIO.h"
#include "Geometry/GeometryOP.h"
#include "Geometry/DelaunayTriangleation.h"
#include "ColorTable.h"


ImageTile::ImageTile()
{
	m_PyrLevel = -1;
	m_pTexture = NULL;
	m_pTexture = NULL;
};

ImageTile::~ImageTile()
{

};

void  ImageTile::FreeTexData(GLTexturePool* pPool)
{
	if (m_pTexture)
	{
		pPool->ReturnTexture(m_pTexture);
		m_pTexture = NULL;
	}
}

void  ImageTile::DrawTex(GLuint vaoid)
{
	if (m_pTexture)
	{
		DrawTexture(m_pTexture->tex, vaoid, m_pTexture->vbo);
	}
}

/************************************************************************/
/*							     普通影像                                */
/************************************************************************/
Image::Image()
{
	m_bShow = false;
	m_OrthoID = -1;
	m_TileCount = 0;
	m_A = 1.0;
	m_B = 0.0;
	m_BandCount = 0;
	InitialColorTable(m_ColorTable);
	m_bColorTableExist = false;
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
}

Image::~Image()
{
	Clear();
}

void   Image::Clear()
{
	Destroy();
}

bool   Image::LoadImage(const std::string& domfilename)
{
	GDALDataset* pDataSet = (GDALDataset *)GDALOpen(domfilename.c_str(), GA_ReadOnly);
	if (!pDataSet)
	{
		return false;
	}
	int nBx = 0, nBy = 0;
	pDataSet->GetRasterBand(1)->GetBlockSize(&nBx, &nBy);
	m_DomName = domfilename;
	m_CtbName = domfilename;
	m_CtbName.erase(m_CtbName.end() - 3, m_CtbName.end());
	m_CtbName.append("ctb");
	m_Cols = pDataSet->GetRasterXSize();
	m_Rows = pDataSet->GetRasterYSize();
	m_BandCount = pDataSet->GetRasterCount();
	int nPixelSize = 0;
	GDALDataType dT = pDataSet->GetRasterBand(1)->GetRasterDataType();
	if (dT == GDT_Byte)
	{
		nPixelSize = 1;
	}
	else if (dT == GDT_UInt16)
	{
		nPixelSize = 2;
	}

	/************************************************************************/
	/*							查看是否包含地理信息                         */
	/************************************************************************/
	CPLErr err = pDataSet->GetGeoTransform(m_GeoTrans);
	if (err == CPLErr::CE_Failure)
	{
		m_GeoTrans[0] = 0.0; m_GeoTrans[1] = 1.0; m_GeoTrans[2] = 0.0;
		m_GeoTrans[3] = m_Rows; m_GeoTrans[4] = 0.0; m_GeoTrans[5] = -1.0;
		m_GSD = 1.0; //按照像素计数
	}
	else
	{
		m_GSD = m_GeoTrans[1];
	}
	m_GroundRange.MinX = m_GeoTrans[0];
	m_GroundRange.MaxX = m_GeoTrans[0] + m_Cols * m_GeoTrans[1];
	m_GroundRange.MinY = m_GeoTrans[3] + m_Rows * m_GeoTrans[5];
	m_GroundRange.MaxY = m_GeoTrans[3];
	/************************************************************************/
	/*							  建立影像金字塔                             */
	/************************************************************************/
	int nOverView[12] = { 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096 };
	int iOverViewCount = pDataSet->GetRasterBand(1)->GetOverviewCount();
	int iNeedOverviewCount = ceil(log(double(max(m_Rows, m_Cols)) / 256) / log(2.0));
	bool bNeedRebuildPyramid = false;
	if (iOverViewCount < iNeedOverviewCount)
	{
		bNeedRebuildPyramid = true;
	}
	if (bNeedRebuildPyramid)
	{
		pDataSet->BuildOverviews("NEAREST", iNeedOverviewCount, nOverView, 0, 0, NULL, NULL);
	}
	/************************************************************************/
	/*                    从金字塔某个层级上计算影像的拉伸系数                */
	/************************************************************************/
	if (dT == GDT_UInt16)
	{
		int nMaxWidth = max(m_Cols, m_Rows);
		int nOverViewLevel = log(nMaxWidth / 4096.0) / log(2.0);
		GDALRasterBand* pBand = pDataSet->GetRasterBand(1)->GetOverview(nOverViewLevel);
		int nOverviewWid = pBand->GetXSize();
		int nOverviewHei = pBand->GetYSize();
		int nPixelCount = nOverviewWid * nOverviewHei;
		BYTE* pOverviewData = new BYTE[nOverviewHei * nOverviewWid * nPixelSize];
		memset(pOverviewData, 0x00, nPixelSize * nOverviewWid * nOverviewHei);
		CPLErr err = pBand->RasterIO(GF_Read, 0, 0, nOverviewWid, nOverviewHei, pOverviewData, nOverviewWid, nOverviewHei, dT, nPixelSize, nPixelSize * nOverviewWid, NULL);
		if (err == CE_None)
		{
			unsigned short usMax = 0x0000, usMin = 0xffff;
			for (int i = 0; i < nPixelCount; ++i)
			{
				unsigned short c = ((unsigned short*)pOverviewData)[i];
				if (c > usMax) usMax = c;
				if (c < usMin) usMin = c;
			}
			delete pOverviewData; pOverviewData = NULL;
			m_A = 255.0 / (usMax - usMin);
			m_B = -usMin * m_A;
		}
	}
	m_Pyramids = pDataSet->GetRasterBand(1)->GetOverviewCount();
	/************************************************************************/
	/*                             载入颜色表                                */
	/************************************************************************/
	if (ReadColorTable(domfilename, m_BandCount, m_ColorTable))
	{
		m_bColorTableExist = true;
	}
	else
	{
		m_bColorTableExist = false;
	}
	/************************************************************************/
	/*							     生成四叉树                              */
	/************************************************************************/
	InitialQuadTree();
	return true;
}

bool Image::LoadImage(const std::string & domfilename, OGREnvelope enve)
{
	GDALDataset* pDataSet = (GDALDataset *)GDALOpen(domfilename.c_str(), GA_ReadOnly);
	if (!pDataSet)
	{
		return false;
	}
	int nBx = 0, nBy = 0;
	pDataSet->GetRasterBand(1)->GetBlockSize(&nBx, &nBy);
	m_DomName = domfilename;
	m_CtbName = domfilename;
	m_CtbName.erase(m_CtbName.end() - 3, m_CtbName.end());
	m_CtbName.append("ctb");
	m_Cols = pDataSet->GetRasterXSize();
	m_Rows = pDataSet->GetRasterYSize();
	m_BandCount = pDataSet->GetRasterCount();
	int nPixelSize = 0;
	GDALDataType dT = pDataSet->GetRasterBand(1)->GetRasterDataType();
	if (dT == GDT_Byte)
	{
		nPixelSize = 1;
	}
	else if (dT == GDT_UInt16)
	{
		nPixelSize = 2;
	}

	/************************************************************************/
	/*							根据enve设置地理信息                         */
	/************************************************************************/
	// 	CPLErr err = pDataSet->GetGeoTransform(m_GeoTrans);
	// 	if (err == CPLErr::CE_Failure)
	// 	{
	// 		m_GeoTrans[0] = 0.0; m_GeoTrans[1] = 1.0; m_GeoTrans[2] = 0.0;
	// 		m_GeoTrans[3] = m_Rows; m_GeoTrans[4] = 0.0; m_GeoTrans[5] = -1.0;
	// 		m_GSD = 1.0; //按照像素计数
	// 	}
	// 	else
	// 	{
	// 		m_GSD = m_GeoTrans[1];
	// 	}
	m_GSD = (enve.MaxX - enve.MinX) / m_Cols;
	m_GroundRange = enve;
	/************************************************************************/
	/*							  建立影像金字塔                             */
	/************************************************************************/
	int nOverView[12] = { 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096 };
	int iOverViewCount = pDataSet->GetRasterBand(1)->GetOverviewCount();
	int iNeedOverviewCount = ceil(log(double(max(m_Rows, m_Cols)) / 256) / log(2.0));
	bool bNeedRebuildPyramid = false;
	if (iOverViewCount < iNeedOverviewCount)
	{
		bNeedRebuildPyramid = true;
	}
	if (bNeedRebuildPyramid)
	{
		pDataSet->BuildOverviews("NEAREST", iNeedOverviewCount, nOverView, 0, 0, NULL, NULL);
	}
	/************************************************************************/
	/*                    从金字塔某个层级上计算影像的拉伸系数                */
	/************************************************************************/
	if (dT == GDT_UInt16)
	{
		int nMaxWidth = max(m_Cols, m_Rows);
		int nOverViewLevel = log(nMaxWidth / 4096.0) / log(2.0);
		GDALRasterBand* pBand = pDataSet->GetRasterBand(1)->GetOverview(nOverViewLevel);
		int nOverviewWid = pBand->GetXSize();
		int nOverviewHei = pBand->GetYSize();
		int nPixelCount = nOverviewWid * nOverviewHei;
		BYTE* pOverviewData = new BYTE[nOverviewHei * nOverviewWid * nPixelSize];
		memset(pOverviewData, 0x00, nPixelSize * nOverviewWid * nOverviewHei);
		CPLErr err = pBand->RasterIO(GF_Read, 0, 0, nOverviewWid, nOverviewHei, pOverviewData, nOverviewWid, nOverviewHei, dT, nPixelSize, nPixelSize * nOverviewWid, NULL);
		if (err == CE_None)
		{
			unsigned short usMax = 0x0000, usMin = 0xffff;
			for (int i = 0; i < nPixelCount; ++i)
			{
				unsigned short c = ((unsigned short*)pOverviewData)[i];
				if (c > usMax) usMax = c;
				if (c < usMin) usMin = c;
			}
			delete pOverviewData; pOverviewData = NULL;
			m_A = 255.0 / (usMax - usMin);
			m_B = -usMin * m_A;
		}
	}
	m_Pyramids = pDataSet->GetRasterBand(1)->GetOverviewCount();
	/************************************************************************/
	/*                             载入颜色表                                */
	/************************************************************************/
	if (ReadColorTable(domfilename, m_BandCount, m_ColorTable))
	{
		m_bColorTableExist = true;
	}
	else
	{
		m_bColorTableExist = false;
	}
	/************************************************************************/
	/*							     生成四叉树                              */
	/************************************************************************/
	InitialQuadTree();
	return true;
}

void   Image::Destroy()
{
	for (int i = 0; i < 4; ++i)
	{
		if (m_Root.children[i])
		{
			DestroyNode(m_Root.children[i]);
			m_Root.children[i] = NULL;
		}
	}
}

void   Image::LoadCurrentTexData(GLTexturePool* pTexturePool, bool bForce, bool* bStop)
{
	int iCurTexCount = m_CurTiles.size();
	GDALDataset* pDataSet = (GDALDataset *)GDALOpen(m_DomName.c_str(), GA_ReadOnly);
	int bandsmap[3] = { 1, 2, 3 };
	if (!pDataSet)
	{
		return;
	}
	int nPixelSize = 0;
	GDALDataType dT = pDataSet->GetRasterBand(1)->GetRasterDataType();
	if (dT == GDT_Byte)
	{
		nPixelSize = 1;
	}
	else if (dT == GDT_UInt16)
	{
		nPixelSize = 2;
	}
	const int datalen = 256 * 256 * 3 * 2;
	BYTE data[datalen];
	int nCols = pDataSet->GetRasterXSize();
	int nRows = pDataSet->GetRasterYSize();
	int nPixelCount = 256 * 256 * 3;
	int nBandCount = pDataSet->GetRasterCount();
	for (int i = 0; i < iCurTexCount; ++i)
	{
		memset(data, 0xc8, datalen);
		if (*bStop)
		{
			GDALClose(pDataSet); return;
		}
		ImageTile* pCurTile = m_CurTiles[i];
		if (pCurTile->m_pTexture != NULL && bForce == false)
		{
			continue;
		}
		pCurTile->m_pTexture = pTexturePool->GetTexture();
		int iZoom = 1 << pCurTile->m_PyrLevel;

		int nRealWid = 0, nRealHei = 0;
		if (pCurTile->m_ImgRange.MinX + G_TileWidth * iZoom <= nCols)
		{
			nRealWid = G_TileWidth * iZoom;
		}
		else
		{
			nRealWid = nCols - pCurTile->m_ImgRange.MinX;
		}

		if (pCurTile->m_ImgRange.MinY + G_TileWidth * iZoom <= nRows)
		{
			nRealHei = G_TileWidth * iZoom;
		}
		else
		{
			nRealHei = nRows - pCurTile->m_ImgRange.MinY;
		}
		CPLErr err = pDataSet->RasterIO(GF_Read, pCurTile->m_ImgRange.MinX, pCurTile->m_ImgRange.MinY, nRealWid, nRealHei, data, nRealWid / iZoom, nRealHei / iZoom, dT, nBandCount, bandsmap, nBandCount * nPixelSize, nBandCount * G_TileWidth * nPixelSize, nPixelSize);

		if (dT == GDT_UInt16)
		{
			if (m_bColorTableExist)
			{
				Trans16To8(data, 256, 256, 3, &m_ColorTable);
			}
			else
			{
				Trans16To8(data, 256, 256, 3);
			}
			if (nBandCount == 4)
			{
				BGR2RGB(data, 256, 256, nBandCount);
			}
		}
		else if (dT == GDT_Byte)
		{
			if (m_bColorTableExist)
			{
				Trans8To8(data, 256, 256, 3, &m_ColorTable);
			}
			if (nBandCount == 4)
			{
				BGR2RGB(data, 256, 256, nBandCount); //RGBA
			}
		}

		if (err == CE_Failure)
		{
			GDALClose(pDataSet);
			return;
		}
		/*-----在前一个版本中将wglMakeCurrent放在OrthoSet之中，这样会照成问题：前一个线程尚未结束实，下一个线程可能已经开始，这样造成RC正在使用无法更改-----*/
		/*-----向纹理传输数据-----*/
		MultiThreadWGLMakeCurrent(pTexturePool->GetDC(), pTexturePool->GetRC());
		glBindTexture(GL_TEXTURE_2D, pCurTile->m_pTexture->tex);
		if (m_BandCount == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, G_TileWidth, G_TileWidth, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		else
			glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, G_TileWidth, G_TileWidth, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data);
		/*-----向VBO传输数据-----*/
		OGREnvelope	m_TileRange;
		m_TileRange.MinX = m_GroundRange.MinX + pCurTile->m_ImgRange.MinX * m_GSD;
		m_TileRange.MaxX = m_GroundRange.MinX + pCurTile->m_ImgRange.MaxX * m_GSD;
		m_TileRange.MinY = m_GroundRange.MaxY - pCurTile->m_ImgRange.MaxY * m_GSD;
		m_TileRange.MaxY = m_GroundRange.MaxY - pCurTile->m_ImgRange.MinY * m_GSD;
		float vertexcoord[24] = {
			m_TileRange.MinX, m_TileRange.MaxY, m_OrthoID, 1.0f,
			m_TileRange.MaxX, m_TileRange.MaxY, m_OrthoID, 1.0f,
			m_TileRange.MaxX, m_TileRange.MinY, m_OrthoID, 1.0f,
			m_TileRange.MinX, m_TileRange.MinY, m_OrthoID, 1.0f,
			0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0 };
		glBindBuffer(GL_ARRAY_BUFFER, pCurTile->m_pTexture->vbo);
		glBufferData(GL_ARRAY_BUFFER, 96, vertexcoord, GL_DYNAMIC_DRAW);
		MultiThreadWGLMakeCurrent(NULL, NULL);
	}
	GDALClose(pDataSet);
}


void   Image::LoadCurrentTexData2(GLTexturePool* pTexturePool, bool bForce, bool* bStop)
{
	int iCurTexCount = m_CurTiles.size();
	GDALDataset* pDataSet = (GDALDataset *)GDALOpen(m_DomName.c_str(), GA_ReadOnly);
	int bandsmap[4] = { 1, 2, 3, 4 };
	if (!pDataSet)
	{
		return;
	}
	int nPixelSize = 0;
	GDALDataType dT = pDataSet->GetRasterBand(1)->GetRasterDataType();
	if (dT == GDT_Byte)
	{
		nPixelSize = 1;
	}
	else if (dT == GDT_UInt16)
	{
		nPixelSize = 2;
	}
	int nCols = pDataSet->GetRasterXSize();
	int nRows = pDataSet->GetRasterYSize();
	int nBandCount = pDataSet->GetRasterCount();

	int datalen = 256 * 256 * 4 * 2;
	BYTE *data = new BYTE[datalen];
	int nPixelCount = 256 * 256 * nBandCount;

	for (int i = 0; i < iCurTexCount; ++i)
	{
		memset(data, 0, datalen);
		if (*bStop)
		{
			GDALClose(pDataSet); return;
		}
		ImageTile* pCurTile = m_CurTiles[i];
		if (pCurTile->m_pTexture != NULL && bForce == false)
		{
			continue;
		}
		pCurTile->m_pTexture = pTexturePool->GetTexture();
		int iZoom = 1 << pCurTile->m_PyrLevel;

		int nRealWid = 0, nRealHei = 0;
		if (pCurTile->m_ImgRange.MinX + G_TileWidth * iZoom <= nCols)
		{
			nRealWid = G_TileWidth * iZoom;
		}
		else
		{
			nRealWid = nCols - pCurTile->m_ImgRange.MinX;
		}

		if (pCurTile->m_ImgRange.MinY + G_TileWidth * iZoom <= nRows)
		{
			nRealHei = G_TileWidth * iZoom;
		}
		else
		{
			nRealHei = nRows - pCurTile->m_ImgRange.MinY;
		}
		CPLErr err = pDataSet->RasterIO(GF_Read, pCurTile->m_ImgRange.MinX, pCurTile->m_ImgRange.MinY, nRealWid, nRealHei, data, nRealWid / iZoom, nRealHei / iZoom, dT, nBandCount, bandsmap, nBandCount * nPixelSize, nBandCount * G_TileWidth* nPixelSize, nPixelSize);

		if (dT == GDT_UInt16)
		{
			if (m_bColorTableExist)
			{
				Trans16To8(data, 256, 256, nBandCount, &m_ColorTable);
			}
			else
			{
				Trans16To8(data, 256, 256, nBandCount);
			}
			// 			if (nBandCount == 4)
			// 			{
			// 				BGR2RGB(data, 256, 256, nBandCount);
			// 			}
		}
		else if (dT == GDT_Byte)
		{
			if (m_bColorTableExist)
			{
				Trans8To8(data, 256, 256, nBandCount, &m_ColorTable);
			}
			if (nBandCount == 4)
			{
				BGR2RGB(data, 256, 256, nBandCount); //RGBA
			}
		}

		if (err == CE_Failure)
		{
			GDALClose(pDataSet);
			return;
		}
		/*-----在前一个版本中将wglMakeCurrent放在OrthoSet之中，这样会照成问题：前一个线程尚未结束实，下一个线程可能已经开始，这样造成RC正在使用无法更改-----*/
		/*-----向纹理传输数据-----*/
		MultiThreadWGLMakeCurrent(pTexturePool->GetDC(), pTexturePool->GetRC());
		if (pCurTile->m_pTexture)
			glBindTexture(GL_TEXTURE_2D, pCurTile->m_pTexture->tex);
		if (nBandCount == 4)
		{
			//for (int i = 0; i<256*256; i++)
			//{
			//	if (data[i * 4] == 0 && data[i * 4 + 1] == 0 && data[i * 4 + 2] == 0)
			//		data[i * 4 + 3] = 0;
			//	else data[i * 4 + 3] = 1;
			//}
			//glEnable(GL_ALPHA_TEST);
			//glAlphaFunc(GL_EQUAL,1);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, G_TileWidth, G_TileWidth, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

		}
		else if (m_BandCount == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, G_TileWidth, G_TileWidth, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		else
			glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, G_TileWidth, G_TileWidth, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data);
		/*-----向VBO传输数据-----*/
		OGREnvelope	m_TileRange;
		m_TileRange.MinX = m_GroundRange.MinX + pCurTile->m_ImgRange.MinX * m_GSD;
		m_TileRange.MaxX = m_GroundRange.MinX + pCurTile->m_ImgRange.MaxX * m_GSD;
		m_TileRange.MinY = m_GroundRange.MaxY - pCurTile->m_ImgRange.MaxY * m_GSD;
		m_TileRange.MaxY = m_GroundRange.MaxY - pCurTile->m_ImgRange.MinY * m_GSD;
		float vertexcoord[24] = {
			m_TileRange.MinX, m_TileRange.MaxY, m_OrthoID, 1.0f,
			m_TileRange.MaxX, m_TileRange.MaxY, m_OrthoID, 1.0f,
			m_TileRange.MaxX, m_TileRange.MinY, m_OrthoID, 1.0f,
			m_TileRange.MinX, m_TileRange.MinY, m_OrthoID, 1.0f,
			0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0 };
		glBindBuffer(GL_ARRAY_BUFFER, pCurTile->m_pTexture->vbo);
		glBufferData(GL_ARRAY_BUFFER, 96, vertexcoord, GL_DYNAMIC_DRAW);
		MultiThreadWGLMakeCurrent(NULL, NULL);
	}
	GDALClose(pDataSet);
	if (data) delete[]data;
}


void Image::UpdateCurrentTexData(GLTexturePool* pTexturePool, bool bForce, bool* bStop, BYTE *data, int stCol, int stRow, int edCol, int edRow, double zoomRate, int nDataX, int nDataY)
{
	int iCurTexCount = m_CurTiles.size();
	GDALDataset* pDataSet = (GDALDataset *)GDALOpen(m_DomName.c_str(), GA_ReadOnly);
	int bandsmap[4] = { 1, 2, 3, 4 };
	if (!pDataSet)
	{
		return;
	}
	int nPixelSize = 0;
	GDALDataType dT = pDataSet->GetRasterBand(1)->GetRasterDataType();
	if (dT == GDT_Byte)
	{
		nPixelSize = 1;
	}
	else if (dT == GDT_UInt16)
	{
		nPixelSize = 2;
	}
	int nCols = pDataSet->GetRasterXSize();
	int nRows = pDataSet->GetRasterYSize();
	int nBandCount = pDataSet->GetRasterCount();


	int nPixelCount = 256 * 256 * nBandCount;






	for (int i = 0; i < iCurTexCount; ++i)
	{


		if (*bStop)
		{
			GDALClose(pDataSet); return;
		}
		ImageTile* pCurTile = m_CurTiles[i];


		// 		if (pCurTile->m_pTexture != NULL && bForce == false)
		// 		{
		// 			continue;
		// 		}
		pCurTile->m_pTexture = pTexturePool->GetTexture();
		int iZoom = 1 << pCurTile->m_PyrLevel;

		int datalen = 256 * 256 * iZoom;
		double *data3 = new double[datalen];
		BYTE *data2 = new BYTE[datalen];



		int nRealWid = 0, nRealHei = 0;
		if (pCurTile->m_ImgRange.MinX + G_TileWidth * iZoom <= nCols)
		{
			nRealWid = G_TileWidth * iZoom;
		}
		else
		{
			nRealWid = nCols - pCurTile->m_ImgRange.MinX;
		}

		if (pCurTile->m_ImgRange.MinY + G_TileWidth * iZoom <= nRows)
		{
			nRealHei = G_TileWidth * iZoom;
		}
		else
		{
			nRealHei = nRows - pCurTile->m_ImgRange.MinY;
		}
		memset(data3, 255.0, datalen);

		//CPLErr err = pDataSet->RasterIO(GF_Read, pCurTile->m_ImgRange.MinX, pCurTile->m_ImgRange.MinY, nRealWid, nRealHei, data2, nRealWid / iZoom, nRealHei / iZoom, dT, nBandCount, bandsmap, nBandCount * nPixelSize, nBandCount * nRealWid / iZoom * nPixelSize, nPixelSize);
		CPLErr err = pDataSet->RasterIO(GF_Read, pCurTile->m_ImgRange.MinX, pCurTile->m_ImgRange.MinY, nRealWid, nRealHei, data3, nRealWid / iZoom, nRealHei / iZoom, GDT_Float64, nBandCount, bandsmap, 0, 0, 0);
		//替换纠正数据
		//将纠正结果重采样至当前纹理尺度
		//nDataX->(edCol-stCol)->nRealWid/iZoom

		//pDataSet->RasterIO(GF_Read, stcol, strow, edcol - stcol + 1, edrow - strow + 1, datatemp1, memWidth, memHeight, GDT_Float64, 1, 0, 0, 0, 0);




		memset(data2, 0, datalen);



		for (unsigned i = 0; i < datalen; i++)
		{
			data2[i] = data3[i];

		}



		int stX = max(stCol*1., pCurTile->m_ImgRange.MinX);
		int stY = max(stRow*1., pCurTile->m_ImgRange.MinY);
		int edX = min(edCol*1., pCurTile->m_ImgRange.MinX + nRealWid);
		int edY = min(edRow*1., pCurTile->m_ImgRange.MinY + nRealHei);


		for (int i = stY; i <= edY; i++)
		{
			for (int j = stX; j <= edX; j++)
			{
				for (int k = 0; k<nBandCount; k++)
				{
					int iTex = min(255., max(0., (i - pCurTile->m_ImgRange.MinY) / iZoom));
					int jTex = min(255., max(0., (j - pCurTile->m_ImgRange.MinX) / iZoom));
					int iDat = min(nDataY - 1, max(0, (i - stRow) / iZoom));
					int jDat = min(nDataX - 1, max(0, (j - stCol) / iZoom));

					int nIdxTex = iTex*nRealWid / iZoom*nBandCount + jTex*nBandCount + k;
					int nIdxDat = iDat*nDataX*nBandCount + jDat*nBandCount + k;

					data2[nIdxTex] = data[nIdxDat];
				}
			}
		}



		for (unsigned i = 0; i < datalen; i++)
		{
			data2[i] = data2[i] * 0.9;

		}

		if (dT == GDT_UInt16)
		{
			if (m_bColorTableExist)
			{
				//Trans16To8(data2, nDataX, nDataY, nBandCount, &m_ColorTable);
			}
			else
			{
				//Trans16To8(data2, 256, 256, nBandCount);
			}
		}
		else if (dT == GDT_Byte)
		{
			if (m_bColorTableExist)
			{
				Trans8To8(data2, 256, 256, nBandCount, &m_ColorTable);
			}
			if (nBandCount == 4)
			{
				BGR2RGB(data2, 256, 256, nBandCount); //RGBA
			}
		}

		if (err == CE_Failure)
		{
			GDALClose(pDataSet);
			return;
		}
		/*-----在前一个版本中将wglMakeCurrent放在OrthoSet之中，这样会照成问题：前一个线程尚未结束实，下一个线程可能已经开始，这样造成RC正在使用无法更改-----*/
		/*-----向纹理传输数据-----*/


		MultiThreadWGLMakeCurrent(pTexturePool->GetDC(), pTexturePool->GetRC());
		glBindTexture(GL_TEXTURE_2D, pCurTile->m_pTexture->tex);
		if (nBandCount == 4)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, G_TileWidth, G_TileWidth, 0, GL_RGBA, GL_UNSIGNED_BYTE, data2);
		}
		else if (m_BandCount == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, G_TileWidth, G_TileWidth, 0, GL_RGB, GL_UNSIGNED_BYTE, data2);
		else
			glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, G_TileWidth, G_TileWidth, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data2);
		/*-----向VBO传输数据-----*/
		OGREnvelope	m_TileRange;
		m_TileRange.MinX = m_GroundRange.MinX + pCurTile->m_ImgRange.MinX * m_GSD;
		m_TileRange.MaxX = m_GroundRange.MinX + pCurTile->m_ImgRange.MaxX * m_GSD;
		m_TileRange.MinY = m_GroundRange.MaxY - pCurTile->m_ImgRange.MaxY * m_GSD;
		m_TileRange.MaxY = m_GroundRange.MaxY - pCurTile->m_ImgRange.MinY * m_GSD;
		float vertexcoord[24] = {
			m_TileRange.MinX, m_TileRange.MaxY, m_OrthoID, 1.0f,
			m_TileRange.MaxX, m_TileRange.MaxY, m_OrthoID, 1.0f,
			m_TileRange.MaxX, m_TileRange.MinY, m_OrthoID, 1.0f,
			m_TileRange.MinX, m_TileRange.MinY, m_OrthoID, 1.0f,
			0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0 };
		glBindBuffer(GL_ARRAY_BUFFER, pCurTile->m_pTexture->vbo);
		glBufferData(GL_ARRAY_BUFFER, 96, vertexcoord, GL_DYNAMIC_DRAW);
		MultiThreadWGLMakeCurrent(NULL, NULL);


		if (data2) delete[]data2;

		if (data3) delete[]data3;


	}
	GDALClose(pDataSet);
	//if (data2) delete[]data2;




}


void   Image::Search(const OGREnvelope& enve, int iLevel, std::vector<ImageTile*>& ResultItem)
{
	m_CurTiles.clear();
	if (Intersect(enve, m_GroundRange) != true) return;
	if (iLevel < 0) iLevel = 0;
	else if (m_Depth - 1 < iLevel) iLevel = m_Depth - 1;
	OGREnvelope localenv;
	localenv.MinX = ((enve.MinX - m_GroundRange.MinX) / m_GSD);
	localenv.MaxX = ((enve.MaxX - m_GroundRange.MinX) / m_GSD);
	localenv.MinY = ((m_GroundRange.MaxY - enve.MaxY) / m_GSD);
	localenv.MaxY = ((m_GroundRange.MaxY - enve.MinY) / m_GSD);
	SearchQuadTree(&m_Root, localenv, iLevel, m_CurTiles);
	ResultItem.insert(ResultItem.end(), m_CurTiles.begin(), m_CurTiles.end());
}

void   Image::CreateQuadBranch(int iDepth, const OGREnvelope& img, QuadNode** ppNode)
{
	if (iDepth >= 0)
	{
		OGREnvelope childimg[4];
		SplitImgEnvelope(img, childimg);
		for (int i = 0; i < 4; ++i)
		{
			if (childimg[i].MinX >= m_Cols || childimg[i].MinY >= m_Rows) ppNode[i] = NULL;
			else
			{
				ppNode[i] = new QuadNode;
				ppNode[i]->tile.m_ImgRange = childimg[i];
				ppNode[i]->tile.m_PyrLevel = iDepth;
				if (m_BandCount == 3)
				{
					ppNode[i]->tile.m_Color = ImageTile::RGB;
				}
				else if (m_BandCount == 1)
				{
					ppNode[i]->tile.m_Color = ImageTile::GRAY;
				}
				m_TileCount++;
				CreateQuadBranch(iDepth - 1, childimg[i], ppNode[i]->children);
			}
		}
	}
}

void   Image::SearchQuadTree(QuadNode* pStartNode, const OGREnvelope& Env, int iLevel, std::vector<ImageTile*>& ResultItem)
{
	if (!pStartNode)
	{
		return;
	}
	if (Intersect(pStartNode->tile.m_ImgRange, Env) && pStartNode->tile.m_PyrLevel == iLevel)
	{
		ResultItem.push_back(&pStartNode->tile);
		return;
	}
	else if (pStartNode->tile.m_PyrLevel <= iLevel)
	{
		return;
	}
	else
	{
		for (int i = 0; i < 4; ++i)
		{
			SearchQuadTree(pStartNode->children[i], Env, iLevel, ResultItem);
		}
	}
}

void   Image::SplitImgEnvelope(const OGREnvelope& Env, OGREnvelope* pChildEnv)
{
	pChildEnv[0].MinX = Env.MinX;
	pChildEnv[0].MinY = Env.MinY;
	pChildEnv[0].MaxX = (Env.MinX + Env.MaxX) / 2;
	pChildEnv[0].MaxY = (Env.MinY + Env.MaxY) / 2;
	pChildEnv[1].MinX = (Env.MinX + Env.MaxX) / 2;
	pChildEnv[1].MinY = Env.MinY;
	pChildEnv[1].MaxX = Env.MaxX;
	pChildEnv[1].MaxY = (Env.MinY + Env.MaxY) / 2;
	pChildEnv[2].MinX = Env.MinX;
	pChildEnv[2].MinY = (Env.MinY + Env.MaxY) / 2;
	pChildEnv[2].MaxX = (Env.MinX + Env.MaxX) / 2;
	pChildEnv[2].MaxY = Env.MaxY;
	pChildEnv[3].MinX = (Env.MinX + Env.MaxX) / 2;
	pChildEnv[3].MinY = (Env.MinY + Env.MaxY) / 2;
	pChildEnv[3].MaxX = Env.MaxX;
	pChildEnv[3].MaxY = Env.MaxY;
}

void   Image::DestroyNode(QuadNode* pNode)
{
	if (pNode == NULL) return;
	else
	{
		for (int i = 0; i < 4; ++i)
		{
			DestroyNode(pNode->children[i]);
		}
		delete pNode; pNode = NULL;
	}
}

bool   Image::GetShowState() const
{
	return m_bShow;
}

int    Image::GetCols() const
{
	return m_Cols;
}

int    Image::GetRows() const
{
	return m_Rows;
}

int    Image::GetBandCount() const
{
	return m_BandCount;
}

int	   Image::GetOverviewCount() const
{
	return m_Pyramids;
}

double Image::GetGSD() const
{
	return m_GSD;
}

void   Image::SetOrthoID(int id)
{
	m_OrthoID = id;
}

int    Image::GetOrthoID() const
{
	return m_OrthoID;
}

int    Image::GetTileCount()
{
	return m_TileCount;
}

void   Image::InitialQuadTree()
{
	int iMaxWidth = m_Cols > m_Rows ? m_Cols : m_Rows;
	int iPyrLevel = ceil(log(double(iMaxWidth) / G_TileWidth) / log(2.0));
	m_Depth = iPyrLevel + 1;
	m_Root.tile.m_ImgRange.MinX = 0;
	m_Root.tile.m_ImgRange.MinY = 0;
	m_Root.tile.m_ImgRange.MaxX = G_TileWidth << iPyrLevel;
	m_Root.tile.m_ImgRange.MaxY = G_TileWidth << iPyrLevel;
	m_Root.tile.m_PyrLevel = iPyrLevel;
	m_TileCount++;
	CreateQuadBranch(iPyrLevel - 1, m_Root.tile.m_ImgRange, m_Root.children);
}

const string& Image::GetDOMFileName()
{
	return m_DomName;
}

vector<vector<unsigned short>>& Image::GetColorTable()
{
	return m_ColorTable;
}

int Image::GetiZoom(GLTexturePool* pTexturePool)
{
	int iZoom = 0;
	if (m_CurTiles.size() > 0 && pTexturePool)
	{
		ImageTile* pCurTile = m_CurTiles[0];
		pCurTile->m_pTexture = pTexturePool->GetTexture();
		iZoom = 1 << pCurTile->m_PyrLevel;
	}
	return iZoom;
}

/************************************************************************/
/*						       OrthoImage                               */
/************************************************************************/

OrthoImage::OrthoImage()
{
	m_Boundry = NULL;
}

OrthoImage::~OrthoImage()
{
	Clear();
}

void OrthoImage::Clear()
{
	Destroy();
	if (glIsBuffer(m_TriangleVBOID))
	{
		glDeleteBuffers(1, &m_TriangleVBOID);
		m_TriangleVBOID = 0;
	}
	for (int i = 0; i < m_BonVBOID.size(); ++i)
	{
		if (glIsBuffer(m_BonVBOID[i]))
		{
			glDeleteBuffers(1, &m_BonVBOID[i]);
		}
	}
	m_BonVBOID.clear();
	OGRGeometryFactory::destroyGeometry(m_Boundry);

	vector<Triangle>().swap(m_Triangles);
	vector<GLuint>().swap(m_BonVBOID);
	vector<int>().swap(m_BonPointCount);
}

bool OrthoImage::LoadDOM(const std::string & domfilename, const std::string & bonfilename)
{
	Clear();
	Image::LoadImage(domfilename);
	m_Imgname = domfilename;
	m_BonName = bonfilename;
	m_Boundry = ReadSHP(m_BonName);
	InitialTriangle();
	return true;
}

bool OrthoImage::LoadDOM(const std::string& domfilename, const std::string& bonfilename, OGREnvelope enve)
{
	Clear();
	Image::LoadImage(domfilename, enve);
	m_Imgname = domfilename;
	m_BonName = bonfilename;
	m_Boundry = ReadSHP(m_BonName);
	InitialTriangle();
	return true;
}

void OrthoImage::SetOffset(double x, double y)
{
	m_OffsetX = x;
	m_OffsetY = y;
	m_GroundRange.MinX -= m_OffsetX;
	m_GroundRange.MaxX -= m_OffsetX;
	m_GroundRange.MinY -= m_OffsetY;
	m_GroundRange.MaxY -= m_OffsetY;
	GeometryTranslate(m_Boundry, -m_OffsetX, -m_OffsetY);
	InitialTriangle();
}

void OrthoImage::InitialTriangle()
{
	if (m_Boundry)
	{
		m_Triangles.clear();
		GetGeometryBuffer(m_Boundry, m_BonVBOID, m_BonPointCount);
		DelaunayTriangleation(m_Boundry, m_Triangles);
		int iTriangleCount = m_Triangles.size();
		for (int i = 0; i < iTriangleCount; ++i)
		{
			m_Triangles[i].vertices[0].setZ(m_OrthoID);
			m_Triangles[i].vertices[1].setZ(m_OrthoID);
			m_Triangles[i].vertices[2].setZ(m_OrthoID);
		}
		GetTriangleBuffer(m_Triangles, m_TriangleVBOID);
	}
	else
	{
		m_Triangles.clear();
		Triangle tri;
		tri.vertices[0].setX(m_GroundRange.MinX); tri.vertices[0].setY(m_GroundRange.MinY); tri.vertices[0].setZ(m_OrthoID);
		tri.vertices[1].setX(m_GroundRange.MinX); tri.vertices[1].setY(m_GroundRange.MaxY); tri.vertices[1].setZ(m_OrthoID);
		tri.vertices[2].setX(m_GroundRange.MaxX); tri.vertices[2].setY(m_GroundRange.MaxY); tri.vertices[2].setZ(m_OrthoID);
		m_Triangles.push_back(tri);
		tri.vertices[0].setX(m_GroundRange.MinX); tri.vertices[0].setY(m_GroundRange.MinY); tri.vertices[0].setZ(m_OrthoID);
		tri.vertices[1].setX(m_GroundRange.MaxX); tri.vertices[1].setY(m_GroundRange.MaxY); tri.vertices[1].setZ(m_OrthoID);
		tri.vertices[2].setX(m_GroundRange.MaxX); tri.vertices[2].setY(m_GroundRange.MinY); tri.vertices[2].setZ(m_OrthoID);
		m_Triangles.push_back(tri);
		GetTriangleBuffer(m_Triangles, m_TriangleVBOID);
	}
}

int  OrthoImage::GetTriangleCount()
{
	return m_Triangles.size();
}

int  OrthoImage::GetTriangleVBO()
{
	return m_TriangleVBOID;
}


void OrthoImage::ReadImage(CString strImagePath, int stcol, int strow, int edcol, int edrow, int memWidth, int memHeight, unsigned int *&data)
{
	//	BYTE*data2 = *data;

	GDALAllRegister();

	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
	GDALDataset* pDataSet = (GDALDataset *)GDALOpen(strImagePath, GA_ReadOnly);
	if (!pDataSet)
	{
		return;
	}
	int nPixelSize = 0;
	GDALDataType dT = pDataSet->GetRasterBand(1)->GetRasterDataType();
	if (dT == GDT_Byte)
	{
		nPixelSize = 1;
	}
	else if (dT == GDT_UInt16)
	{
		nPixelSize = 2;
	}
	int nCols = pDataSet->GetRasterXSize();
	int nRows = pDataSet->GetRasterYSize();
	int nBandCount = pDataSet->GetRasterCount();
	int*bandsmap = new int[nBandCount];
	for (int i = 0; i<nBandCount; i++)
	{
		bandsmap[i] = i + 1;
	}

	int nPixelCount = 256 * 256 * 3;
	data = new unsigned int[memWidth*memHeight*nBandCount];
	memset(data, 0, memWidth*memHeight*nBandCount);



	unsigned int* datatemp = new unsigned int[memWidth*memHeight*nBandCount];


	double* datatemp1 = new double[memWidth*memHeight*nBandCount];

	CPLErr err = pDataSet->RasterIO(GF_Read, stcol, strow, edcol - stcol + 1, edrow - strow + 1, data, memWidth, memHeight, GDT_UInt32, nBandCount, bandsmap, nBandCount * nPixelSize, nBandCount * memWidth * nPixelSize, nPixelSize);

	//CPLErr err1 = pDataSet->RasterIO(GF_Read, stcol, strow, edcol - stcol + 1, edrow - strow + 1, datatemp1, memWidth, memHeight, GDT_Float64, nBandCount, bandsmap, nBandCount * nPixelSize, nBandCount * memWidth * nPixelSize, nPixelSize);


	pDataSet->RasterIO(GF_Read, stcol, strow, edcol - stcol + 1, edrow - strow + 1, datatemp1, memWidth, memHeight, GDT_Float64, 1, bandsmap, 0, 0, 0);

	for (unsigned i = 0; i < memWidth*memHeight*nBandCount; i++)
	{
		data[i] = (unsigned int)datatemp1[i];

	}





	if (dT == GDT_UInt16)
	{
		m_bColorTableExist = false;
		if (m_bColorTableExist)
		{
			//Trans16To8(data, 256, 256, 3, &m_ColorTable);
		}
		else
		{
			Trans16To8I(data, memWidth, memHeight, 1);
		}
		if (nBandCount == 4)
		{
			//BGR2RGB(data, 256, 256, nBandCount);
		}
	}
	else if (dT == GDT_Byte)
	{
		// 		if (m_bColorTableExist)
		// 		{
		// 			Trans8To8(data2, 256, 256, 3, &m_ColorTable);
		// 		}
		// 		if (nBandCount == 4)
		// 		{
		// 			BGR2RGB(data2, 256, 256, nBandCount); //RGBA
		// 		}
	}
	GDALClose(pDataSet);
}

OGRGeometry* OrthoImage::GetBoundry()
{
	return m_Boundry;
}


