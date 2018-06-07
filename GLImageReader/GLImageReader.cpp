#include "stdafx.h"
#include "GLImageReader.h"
#include <math.h>

static DWORD WINAPI ProcReadBuf(LPVOID lpParameter)
{
	ImageReader* pThd = (ImageReader*)lpParameter;
	if (pThd)
	{
		pThd->Work();
		return 1;
	}
	else {
		return 0;
	}
}


ImageReader::ImageReader(const GLContext& context)
{
	m_hThread = NULL;
	m_dwThdId = 0;
	m_bStop = false;
	m_GLContext = context;
	m_pLUMPool = new GLLUMTexturePool(context);
	m_pRGBPool = new GLRGBTexturePool(context);
	m_pLUMPool->InitialTexturePool(G_TileWidth, 512);
	m_pRGBPool->InitialTexturePool(G_TileWidth, 512);
}

ImageReader::~ImageReader()
{
	ClearData();
	delete m_pLUMPool;  m_pLUMPool = NULL;
	delete m_pRGBPool;  m_pRGBPool = NULL;
}

const GLContext& ImageReader::GetContext()
{
	return m_GLContext;
}

void ImageReader::ClearData()
{
	Stop();
	for (int i = 0; i < m_CurTiles.size(); i++)
	{
		if (m_CurTiles[i]->m_pTexture)
		{
			if (m_CurTiles[i]->m_Color == ImageTile::RGB)
			{
				m_CurTiles[i]->FreeTexData(m_pRGBPool); //不同类型的Tile回归的不同的纹理池中
			}
			else
			{
				m_CurTiles[i]->FreeTexData(m_pLUMPool);
			}
		}

	}
	for (int i = 0; i < m_BufTiles.size(); ++i)
	{
		if (m_BufTiles[i]->m_pTexture)
		{
			if (m_BufTiles[i]->m_Color == ImageTile::RGB)
			{
				m_BufTiles[i]->FreeTexData(m_pRGBPool); //不同类型的Tile回归的不同的纹理池中
			}
			else
			{
				m_BufTiles[i]->FreeTexData(m_pLUMPool);
			}
		}
	}
	/************************************************************************/
	/*					           Image对象清理                            */
	/************************************************************************/
	MultiThreadWGLMakeCurrent(m_GLContext.hDC, m_GLContext.hRC);
	for (int i = 0; i < m_Images.size(); ++i)
	{
		if (m_Images[i])
		{
			m_Images[i]->Clear();
//			delete m_Images[i]; m_Images[i] = NULL;
		}
	}
	MultiThreadWGLMakeCurrent(NULL, NULL);
	std::vector<ImageTile*>().swap(m_CurTiles);
	std::vector<ImageTile*>().swap(m_BufTiles);
	vector<Image*>().swap(m_Images);
	vector<CString>().swap(m_vecImgPath);

	m_dZoomRate = 1;
}



void ImageReader::SetGroundRange(OGREnvelope enve)
{
	m_GroundRange = enve;
}

void ImageReader::SetCurWnd(const OGREnvelope& enve, const OGREnvelope& viewport)
{
	Stop();
	if (m_CurGround.MinX == enve.MinX && m_CurGround.MinY == enve.MinY &&m_CurGround.MaxX == enve.MaxX &&m_CurGround.MaxY == enve.MaxY)
	{
		return;
	}
	m_CurGround = enve;
	m_CurViewPort = viewport;
	/*----更新缓存列表-----*/
	std::vector<ImageTile*> vTempTileList = m_BufTiles;
	if (m_CurTiles.size() > 0)
	{
		int iCurTileCount = m_CurTiles.size();
		int iBufTileCount = m_BufTiles.size();
		/*-----缓存列表无数据,将现有Buf中具有Tex的添加进缓存列表-----*/
		if (iBufTileCount == 0)
		{
			for (int i = 0; i < iCurTileCount; ++i)
			{
				if (m_CurTiles[i]->m_pTexture)
					vTempTileList.push_back(m_CurTiles[i]);
			}
		}
		else
		{
			int i = 0, j = 0;
			for (i = 0; i < iCurTileCount; ++i)
			{
				if (m_CurTiles[i]->m_pTexture == NULL)
					continue;
				for (j = 0; j < iBufTileCount; ++j)
				{
					if (m_CurTiles[i] == m_BufTiles[j])
						break;
				}
				if (j == iBufTileCount)
				{
					if (m_CurTiles[i]->m_pTexture != NULL)
					{
						vTempTileList.push_back(m_CurTiles[i]);
					}
				}
			}
		}
	}
	m_BufTiles = vTempTileList;
	GetCurTiles();
	ClearBufTiles();
	Start();
}


void ImageReader::ClearBufTiles()
{
	int iBufTileCount = m_BufTiles.size();
	int iCurTileCount = m_CurTiles.size();
	ImageTile* pTile = NULL;
	int nUnloadRGBCount = 0;
	int nUnloadRrayCount = 0;
	/*-----如果未载入数据并生成纹理,视为Unload-----*/
	for (int i = 0; i < m_CurTiles.size(); ++i)
	{
		if (m_CurTiles[i]->m_pTexture == NULL)
		{
			if (m_CurTiles[i]->m_Color == ImageTile::RGB)
				++nUnloadRGBCount;
			else
				++nUnloadRrayCount;
		}
	}
	while (nUnloadRGBCount > m_pRGBPool->GetSize())
	{
		m_pRGBPool->ExpandTexturePool(256);
	}
	while (nUnloadRrayCount > m_pLUMPool->GetSize())
	{
		m_pLUMPool->ExpandTexturePool(256);
	}

	int nNeedDelGrayCount = nUnloadRrayCount - m_pLUMPool->GetResidualCount();
	int nNeedDelRGBCount = nUnloadRGBCount - m_pRGBPool->GetResidualCount();
	if (nNeedDelGrayCount <= 0 && nNeedDelRGBCount <= 0) return;

	/************************************************************************/
	/*			此处暂时没有区分RGB和GRAY,默认同时只打开一种类型数据           */
	/************************************************************************/
	if (nNeedDelGrayCount > 0)
	{
		ImageTile* pTile = NULL;
		int iSwapCount = 0;
		while (iSwapCount < nNeedDelGrayCount)
		{
			int i = 0, j = 0;
			for (i = 0; i < iBufTileCount; ++i)
			{
				for (j = 0; j < iCurTileCount; ++j)
				{
					if (m_BufTiles[i] == m_CurTiles[j]) break;
				}
				if (j == iCurTileCount)
				{
					pTile = m_BufTiles[iSwapCount];
					m_BufTiles[iSwapCount] = m_BufTiles[i];
					m_BufTiles[i] = pTile;
					iSwapCount++;
				}
			}
		}
		/*-----从m_BufTiles中剔除那些不再需要的-----*/
		while (nNeedDelGrayCount > 0 && m_BufTiles.size() > 0)
		{
			pTile = m_BufTiles[0];
			if (pTile->m_pTexture) --nNeedDelGrayCount;
			pTile->FreeTexData(m_pLUMPool);
			pTile->m_pTexture = NULL;
			m_BufTiles.erase(m_BufTiles.begin());
		}
	}
	if (nNeedDelRGBCount > 0)
	{
		ImageTile* pTile = NULL;
		int iSwapCount = 0;
		while (iSwapCount < nNeedDelRGBCount)
		{
			int i = 0, j = 0;
			for (i = 0; i < iBufTileCount; ++i)
			{
				for (j = 0; j < iCurTileCount; ++j)
				{
					if (m_BufTiles[i] == m_CurTiles[j]) break;
				}
				if (j == iCurTileCount)
				{
					pTile = m_BufTiles[iSwapCount];
					m_BufTiles[iSwapCount] = m_BufTiles[i];
					m_BufTiles[i] = pTile;
					iSwapCount++;
				}
			}
		}
		/*-----从m_BufTiles中剔除那些不再需要的-----*/
		while (nNeedDelRGBCount > 0 && m_BufTiles.size() > 0)
		{
			pTile = m_BufTiles[0];
			if (pTile->m_pTexture) --nNeedDelRGBCount;
			pTile->FreeTexData(m_pRGBPool);
			pTile->m_pTexture = NULL;
			m_BufTiles.erase(m_BufTiles.begin());
		}
	}
}

void ImageReader::FreeBufferTile()
{
	for (int i = 0; i < m_BufTiles.size(); ++i)
	{
		if (m_BufTiles[i]->m_Color == ImageTile::RGB)
		{
			m_BufTiles[i]->FreeTexData(m_pRGBPool);
		}
		else
		{
			m_BufTiles[i]->FreeTexData(m_pLUMPool);
		}
	}
}

int	 ImageReader::GetCurTileSum()
{
	return m_CurTiles.size();
}

int	 ImageReader::GetCurTexSum()
{
	int iCurTileCount = m_CurTiles.size();
	int iCurTexCount = 0;
	for (int i = 0; i < iCurTileCount; ++i)
	{
		if (m_CurTiles[i]->m_pTexture)
		{
			iCurTexCount++;
		}
	}
	return iCurTexCount;
}

void ImageReader::GetCurTiles()
{
	m_CurTiles.clear();
	double iScreenX = m_CurViewPort.MaxX - m_CurViewPort.MinX;
	double iScreenY = m_CurViewPort.MaxY - m_CurViewPort.MinY;
	for (int i = 0; i < m_Images.size(); ++i)
	{
		double zr = ((m_CurGround.MaxX - m_CurGround.MinX) / m_Images[i]->GetGSD()) / iScreenX;
		int iSerachLevel = int(floor(log(zr) / log(2.0)));
		m_Images[i]->Search(m_CurGround, iSerachLevel, m_CurTiles);
		m_dZoomRate = zr;
	}
}

ImageTile* ImageReader::GetCurTile(int idx)
{
	return m_CurTiles[idx];
}

void ImageReader::Start()
{
	if (m_hThread)
	{
		Stop();
	}
	m_bStop = FALSE;
	m_hThread = ::CreateThread(NULL, 0, ProcReadBuf, this, 0, &m_dwThdId);
	::SetThreadPriority(m_hThread, THREAD_PRIORITY_HIGHEST);
}

bool ImageReader::IsStop()
{
	return m_bStop;
}

void ImageReader::Work()
{
	if (m_bStop != true)
	{
		for (int i = 0; i < m_Images.size(); ++i)
		{
			if (m_Images[i]->GetBandCount() > 3)
			{
				m_Images[i]->LoadCurrentTexData2(m_pRGBPool, false, &m_bStop);
			}
			else
			{
				m_Images[i]->LoadCurrentTexData2(m_pLUMPool, false, &m_bStop);
			}
		}
		m_bStop = true;
	}
}

void ImageReader::UpdateCurTex(BYTE * data, int stCol, int stRow, int edCol, int edRow, double zoomRate)
{
////	Stop();
//	m_bStop = false;
// 	//if (m_bStop != true)
// 	//{
//		if(m_Images.size()>0)
//		{
//			m_Images[0]->UpdateCurrentTexData(m_pRGBPool, false, &m_bStop, data, stCol, stRow, edCol, edRow, zoomRate);
//		}
// 		m_bStop = true;
// 	//}
}

void        ImageReader::Stop()
{
	if (m_hThread)
	{
		m_bStop = TRUE;
		DWORD nStatus = WaitForSingleObject(m_hThread, 10L);
		while (nStatus != WAIT_OBJECT_0)
		{
			nStatus = WaitForSingleObject(m_hThread, 10L);
		}
		m_hThread = NULL;
		m_dwThdId = 0;
	}
}

void        ImageReader::AddImage(Image* pImage)
{
	if (pImage)
	{
		m_Images.push_back(pImage);
		if (m_Images.size() == 1)
		{
			m_GroundRange = pImage->GetGroundRange();
		}
		else
		{
			OGREnvelope enve = pImage->GetGroundRange();
			m_GroundRange.MinX = min(m_GroundRange.MinX, enve.MinX);
			m_GroundRange.MaxX = max(m_GroundRange.MaxX, enve.MaxX);
			m_GroundRange.MinY = min(m_GroundRange.MinY, enve.MinY);
			m_GroundRange.MaxY = max(m_GroundRange.MaxY, enve.MaxY);
		}
	}
}

void ImageReader::AddImage(CString strImgPath, OGREnvelope enve)
{
	if (!strImgPath.IsEmpty())
	{
		m_vecImgPath.push_back(strImgPath);
		if (m_vecImgPath.size() == 1)
		{
	//		m_GroundRange = enve;
			m_GroundRange.MinX = 0;
			m_GroundRange.MinY = 0;
			m_GroundRange.MaxX = enve.MaxX;
			m_GroundRange.MaxY = enve.MaxY;
		}
		else
		{
			m_GroundRange.MinX = min(m_GroundRange.MinX, enve.MinX);
			m_GroundRange.MaxX = max(m_GroundRange.MaxX, enve.MaxX);
			m_GroundRange.MinY = min(m_GroundRange.MinY, enve.MinY);
			m_GroundRange.MaxY = max(m_GroundRange.MaxY, enve.MaxY);
		}
	}
}

void ImageReader::SetImage(Image * pImage, int idx)
{
	if (idx < 0 || idx >= m_Images.size()) return;
	if (m_Images[idx])
	{
//  		m_Images[idx]->Clear();
//  		delete m_Images[idx];
		m_Images[idx] = pImage;
	}

}

int			ImageReader::GetImageCount()
{
	return m_Images.size();
}

double ImageReader::GetCurZoomRate()
{
	return m_dZoomRate;
}

OGREnvelope ImageReader::GetGroundRange()
{
	return m_GroundRange;
}

Image*      ImageReader::GetImage(int i)
{
	if (i < 0 || i >= m_Images.size())
	{
		return NULL;
	}
	else
	{
		return m_Images[i];
	}
}