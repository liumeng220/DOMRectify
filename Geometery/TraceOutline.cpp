#include "TraceOutline.h"
#include "GeometryOP.h"
#include "GeometryIO.h"
#include <io.h>

enum RGBOrder { RRRGGGBBB, RGBRGBRGB };
template<typename T> void Multichannel2Gray(T* pMutiChannelData, T* pSingleChannelData, int iChannel, int iCol, int iRow, RGBOrder iDataOrder)
{
	if (!pMutiChannelData || !pSingleChannelData)
	{
		return;
	}
	int i, j, k;
	const int iRectLen = iCol * iRow;
	const int iPixelLen = sizeof(T);
	memset(pSingleChannelData, 0, iRectLen * iPixelLen);
	int* pOffset = new int[iChannel];
	for (i = 0; i < iChannel; ++i)
	{
		pOffset[i] = i * iRectLen;
	}

	if (RRRGGGBBB == iDataOrder)
	{
		for (i = 0; i < iRow; ++i)
		{
			for (j = 0; j < iCol; ++j)
			{
				int iOffset = iCol * i + j;
				int sum = 0;
				for (k = 0; k < iChannel; ++k)
					sum += *(pMutiChannelData + pOffset[k] + iOffset);
				pSingleChannelData[iOffset] = sum / iChannel;
			}
		}
	}
	else if (RGBRGBRGB == iDataOrder)
	{
		for (i = 0; i < iRectLen; ++i)
		{
			int iOffset = iChannel * i;
			int sum = 0;
			for (k = 0; k < iChannel; ++k)
				sum += *(pMutiChannelData + iOffset + k);
			pSingleChannelData[i] = sum / iChannel;
		}
	}
	else
	{
		return;
	}

	if (pOffset)
	{
		delete[]pOffset; pOffset = NULL;
	}
}

void Erode(BYTE* pData, int nCols, int nRows, int nIteration)
{
	vector<BYTE> pDataCopy(nCols * nRows, 0);
	
	int nIter = 0;
	while (nIter < nIteration)
	{
		memcpy(pDataCopy.data(), pData, nCols * nRows);
		for (int i = 1; i < nRows - 1; ++i)
		{
			for (int j = 1; j < nCols - 1; ++j)
			{
				int nIdx = i * nCols + j;
				if (pDataCopy[nIdx] == 0x00)
				{
					pData[nIdx - 1] = 0x00;
					pData[nIdx + 1] = 0x00;
					pData[nIdx + nCols] = 0x00;
					pData[nIdx - nCols] = 0x00;
					pData[nIdx + nCols + 1] = 0x00;
					pData[nIdx + nCols - 1] = 0x00;
					pData[nIdx - nCols + 1] = 0x00;
					pData[nIdx - nCols - 1] = 0x00;
				}
			}
		}
		nIter++;
	}


}

void TraceOutline(BYTE* pData, int iCol, int iRow, int seedx, int seedy, OGRLinearRing& seq)
{
	bool bFindStartPoint = false;
	int iCurX = seedx;
	int iCurY = seedy;
	int iStartX = 0;
	int iStartY = 0;
	const int iDataLen = iCol * iRow;
	/*
	* 标志某点已经位于轮廓线上
	*/
	const int IN_OUTLINE = 2;
	/*
	* iDirect有四种状态:
	* → 为1方向
	* ↓ 为2方向
	* ← 为3方向
	* ↑ 为4方向
	*/
	int iDirect = 0;

	/*
	* 判断该种子点的初始方向
	*/
	if (iCurX == 0)
	{
		iDirect = 4;
		seq.addPoint(iCurX, iCurY + 1);
		seq.addPoint(iCurX, iCurY);
	}
	else if (iCurX == iCol - 1)
	{
		iDirect = 2;
		seq.addPoint(iCurX + 1, iCurY);
		seq.addPoint(iCurX + 1, iCurY + 1);
	}
	else if (iCurY == 0)
	{
		iDirect = 1;
		seq.addPoint(iCurX, iCurY);
		seq.addPoint(iCurX + 1, iCurY);
	}
	else if (iCurY == iRow - 1)
	{
		iDirect = 3;
		seq.addPoint(iCurX + 1, iCurY + 1);
		seq.addPoint(iCurX, iCurY + 1);
	}
	else if (pData[(iCurY - 1) * iCol + iCurX] == 0x00)
	{
		iDirect = 1;
		seq.addPoint(iCurX, iCurY);
		seq.addPoint(iCurX + 1, iCurY);
	}
	else if (pData[(iCurY + 1) * iCol + iCurX] == 0x00)
	{
		iDirect = 3;
		seq.addPoint(iCurX + 1, iCurY + 1);
		seq.addPoint(iCurX, iCurY + 1);
	}
	else if (pData[iCurY * iCol + iCurX - 1] == 0x00)
	{
		iDirect = 4;
		seq.addPoint(iCurX, iCurY + 1);
		seq.addPoint(iCurX, iCurY);
	}
	else if (pData[iCurY * iCol + iCurX + 1] == 0x00)
	{
		iDirect = 2;
		seq.addPoint(iCurX + 1, iCurY);
		seq.addPoint(iCurX + 1, iCurY + 1);
	}

	pData[iCurY * iCol + iCurX] = IN_OUTLINE;
	while (!bFindStartPoint)
	{
		if (iDirect == 1)
		{
			if (iCurY - 1 >= 0 && iCurX + 1 < iCol && pData[(iCurY - 1) * iCol + iCurX + 1] != 0) //右上方有点
			{
				iDirect = 4;
				seq.addPoint(iCurX + 1, iCurY - 1);
				if (seq.get_IsClosed())bFindStartPoint = true;
				iCurX += 1;
				iCurY -= 1;
				pData[iCurY * iCol + iCurX] = IN_OUTLINE;
			}
			else if (iCurX + 1 < iCol && pData[iCurY * iCol + iCurX + 1] != 0) //右侧有点
			{
				iDirect = 1;
				seq.addPoint(iCurX + 2, iCurY);
				if (seq.get_IsClosed())bFindStartPoint = true;
				iCurX += 1;
				pData[iCurY * iCol + iCurX] = IN_OUTLINE;
			}
			else if (iCurY + 1 < iRow && iCurX + 1 < iCol && pData[(iCurY + 1) * iCol + iCurX + 1] != 0)
			{
				iDirect = 1;
				seq.addPoint(iCurX + 1, iCurY + 1);
				if (seq.get_IsClosed())bFindStartPoint = true;
				if (!bFindStartPoint)
				{
					seq.addPoint(iCurX + 2, iCurY + 1);
					if (seq.get_IsClosed())bFindStartPoint = true;
				}
				iCurX += 1; iCurY += 1;
				pData[iCurY * iCol + iCurX] = IN_OUTLINE;
			}
			else if (iCurY + 1 < iRow && pData[(iCurY + 1) * iCol + iCurX] != 0)
			{
				iDirect = 2;
				seq.addPoint(iCurX + 1, iCurY + 1);
				if (seq.get_IsClosed())bFindStartPoint = true;
				if (!bFindStartPoint)
				{
					seq.addPoint(iCurX + 1, iCurY + 2);
					if (seq.get_IsClosed())bFindStartPoint = true;
				}
				iCurY += 1;
				pData[iCurY * iCol + iCurX] = IN_OUTLINE;
			}
			else if (iCurY + 1 < iRow && iCurX - 1 >= 0 && pData[(iCurY + 1) * iCol + iCurX - 1] != 0)
			{
				iDirect = 2;
				seq.addPoint(iCurX + 1, iCurY + 1);
				if (seq.get_IsClosed())bFindStartPoint = true;
				if (!bFindStartPoint)
				{
					seq.addPoint(iCurX, iCurY + 1);
					if (seq.get_IsClosed())bFindStartPoint = true;
				}
				if (!bFindStartPoint)
				{
					seq.addPoint(iCurX, iCurY + 2);
					if (seq.get_IsClosed())bFindStartPoint = true;
				}
				iCurX -= 1; iCurY += 1;
				pData[iCurY * iCol + iCurX] = IN_OUTLINE;
			}
			else if (iCurX - 1 >= 0 && pData[iCurY * iCol + iCurX - 1] != 0)
			{
				iDirect = 3;
				seq.addPoint(iCurX + 1, iCurY + 1);
				if (seq.get_IsClosed())bFindStartPoint = true;
				if (!bFindStartPoint)
				{
					seq.addPoint(iCurX, iCurY + 1);
					if (seq.get_IsClosed())bFindStartPoint = true;
				}
				if (!bFindStartPoint)
				{
					seq.addPoint(iCurX, iCurY + 1);
					if (seq.get_IsClosed())bFindStartPoint = true;
				}
				if (!bFindStartPoint)
				{
					seq.addPoint(iCurX - 1, iCurY + 1);
					if (seq.get_IsClosed())bFindStartPoint = true;
				}
				iCurX -= 1;
				pData[iCurY * iCol + iCurX] = IN_OUTLINE;
			}
			else if (iCurY - 1 >= 0 && iCurX - 1 >= 0 && pData[(iCurY - 1) * iCol + iCurX - 1] != 0)
			{
				iDirect = 3;
				seq.addPoint(iCurX + 1, iCurY + 1);
				if (seq.get_IsClosed())bFindStartPoint = true;
				if (!bFindStartPoint)
				{
					seq.addPoint(iCurX, iCurY + 1);
					if (seq.get_IsClosed())bFindStartPoint = true;
				}
				if (!bFindStartPoint)
				{
					seq.addPoint(iCurX - 1, iCurY);
					if (seq.get_IsClosed()) bFindStartPoint = true;
				}
				iCurX -= 1; iCurY -= 1;
				pData[iCurY * iCol + iCurX] = IN_OUTLINE;
			}
			else
			{
				seq.addPoint(iCurX + 1, iCurY + 1);
				if (seq.get_IsClosed())bFindStartPoint = true;
				if (!bFindStartPoint)
				{
					seq.addPoint(iCurX, iCurY + 1);
					if (seq.get_IsClosed())bFindStartPoint = true;
				}

				return;
			}
		}
		else if (iDirect == 2)
		{
			if (iCurY + 1 < iRow && iCurX + 1 < iCol && pData[(iCurY + 1) * iCol + iCurX + 1] != 0)
			{
				seq.addPoint(iCurX + 2, iCurY + 1);
				if (seq.get_IsClosed()) bFindStartPoint = true;
				iDirect = 1;
				iCurX += 1; iCurY += 1;
				pData[iCurY * iCol + iCurX] = IN_OUTLINE;
			}
			else if (iCurY + 1 < iRow && pData[(iCurY + 1) * iCol + iCurX] != 0)
			{
				seq.addPoint(iCurX + 1, iCurY + 2);
				if (seq.get_IsClosed()) bFindStartPoint = true;
				iDirect = 2;
				iCurY += 1;
				pData[iCurY * iCol + iCurX] = IN_OUTLINE;
			}
			else if (iCurY + 1 < iRow && iCurX - 1 >= 0 && pData[(iCurY + 1) * iCol + iCurX - 1] != 0)
			{
				seq.addPoint(iCurX, iCurY + 1);
				if (seq.get_IsClosed()) bFindStartPoint = true;
				if (!bFindStartPoint)
				{
					seq.addPoint(iCurX, iCurY + 2);
					if (seq.get_IsClosed()) bFindStartPoint = true;
				}
				iDirect = 2;
				iCurX -= 1; iCurY += 1;
				pData[iCurY * iCol + iCurX] = IN_OUTLINE;
			}
			else if (iCurX - 1 >= 0 && pData[iCurY * iCol + iCurX - 1] != 0)
			{
				seq.addPoint(iCurX, iCurY + 1);
				if (seq.get_IsClosed()) bFindStartPoint = true;
				if (!bFindStartPoint)
				{
					seq.addPoint(iCurX - 1, iCurY + 1);
					if (seq.get_IsClosed()) bFindStartPoint = true;
				}
				iDirect = 3;
				iCurX -= 1;
				pData[iCurY * iCol + iCurX] = IN_OUTLINE;
			}
			else if (iCurY - 1 >= 0 && iCurX - 1 >= 0 && pData[(iCurY - 1) * iCol + iCurX - 1] != 0)
			{
				seq.addPoint(iCurX, iCurY + 1);
				if (seq.get_IsClosed()) bFindStartPoint = true;
				if (!bFindStartPoint)
				{
					seq.addPoint(iCurX, iCurY);
					if (seq.get_IsClosed()) bFindStartPoint = true;
				}
				if (!bFindStartPoint)
				{
					seq.addPoint(iCurX - 1, iCurY);
					if (seq.get_IsClosed()) bFindStartPoint = true;
				}
				iDirect = 3;
				iCurX -= 1; iCurY -= 1;
				pData[iCurY * iCol + iCurX] = IN_OUTLINE;
			}
			else if (iCurY - 1 >= 0 && pData[(iCurY - 1) * iCol + iCurX] != 0)
			{
				seq.addPoint(iCurX, iCurY + 1);
				if (seq.get_IsClosed()) bFindStartPoint = true;
				if (!bFindStartPoint)
				{
					seq.addPoint(iCurX, iCurY);
					if (seq.get_IsClosed()) bFindStartPoint = true;
				}
				if (!bFindStartPoint)
				{
					seq.addPoint(iCurX, iCurY - 1);
					if (seq.get_IsClosed()) bFindStartPoint = true;
				}
				iDirect = 4;
				iCurY -= 1;
				pData[iCurY * iCol + iCurX] = IN_OUTLINE;
			}
			else if (iCurY - 1 >= 0 && iCurX + 1 < iCol && pData[(iCurY - 1) * iCol + iCurX + 1] != 0)
			{
				seq.addPoint(iCurX, iCurY + 1);
				if (seq.get_IsClosed()) bFindStartPoint = true;
				if (!bFindStartPoint)
				{
					seq.addPoint(iCurX, iCurY);
					if (seq.get_IsClosed()) bFindStartPoint = true;
				}
				if (!bFindStartPoint)
				{
					seq.addPoint(iCurX + 1, iCurY);
					if (seq.get_IsClosed()) bFindStartPoint = true;
				}
				if (!bFindStartPoint)
				{
					seq.addPoint(iCurX + 1, iCurY - 1);
					if (seq.get_IsClosed()) bFindStartPoint = true;
				}
				iDirect = 4;
				iCurX += 1; iCurY -= 1;
				pData[iCurY * iCol + iCurX] = IN_OUTLINE;
			}
			else
			{
				seq.addPoint(iCurX, iCurY + 1);
				if (seq.get_IsClosed()) bFindStartPoint = true;
				if (!bFindStartPoint)
				{
					seq.addPoint(iCurX, iCurY);
					if (seq.get_IsClosed()) bFindStartPoint = true;
				}
				return;
			}
		}
		else if (iDirect == 3)
		{
			if (iCurX - 1 >= 0 && iCurY + 1 < iRow && pData[(iCurY + 1) * iCol + iCurX - 1] != 0)
			{
				seq.addPoint(iCurX, iCurY + 2);
				if (seq.get_IsClosed()) bFindStartPoint = true;
				iDirect = 2;
				iCurX -= 1; iCurY += 1;
				pData[iCurY * iCol + iCurX] = IN_OUTLINE;
			}
			else if (iCurX - 1 >= 0 && pData[iCurY * iCol + iCurX - 1] != 0)
			{
				seq.addPoint(iCurX - 1, iCurY + 1);
				if (seq.get_IsClosed()) bFindStartPoint = true;
				iDirect = 3;
				iCurX -= 1;
				pData[iCurY * iCol + iCurX] = IN_OUTLINE;
			}
			else if (iCurX - 1 >= 0 && iCurY - 1 >= 0 && pData[(iCurY - 1) * iCol + iCurX - 1] != 0)
			{
				seq.addPoint(iCurX, iCurY);
				if (seq.get_IsClosed()) bFindStartPoint = true;
				if (!bFindStartPoint)
				{
					seq.addPoint(iCurX - 1, iCurY);
					if (seq.get_IsClosed())bFindStartPoint = true;
				}
				iDirect = 3;
				iCurX -= 1; iCurY -= 1;
				pData[iCurY * iCol + iCurX] = IN_OUTLINE;
			}
			else if (iCurY - 1 >= 0 && pData[(iCurY - 1) * iCol + iCurX] != 0)
			{
				seq.addPoint(iCurX, iCurY);
				if (seq.get_IsClosed()) bFindStartPoint = true;
				if (!bFindStartPoint)
				{
					seq.addPoint(iCurX, iCurY - 1);
					if (seq.get_IsClosed()) bFindStartPoint = true;
				}
				iDirect = 4;
				iCurY -= 1;
				pData[iCurY * iCol + iCurX] = IN_OUTLINE;
			}
			else if (iCurX + 1 < iCol && iCurY - 1 >= 0 && pData[(iCurY - 1)* iCol + iCurX + 1] != 0)
			{
				seq.addPoint(iCurX, iCurY);
				if (seq.get_IsClosed()) bFindStartPoint = true;
				if (!bFindStartPoint)
				{
					seq.addPoint(iCurX + 1, iCurY);
					if (seq.get_IsClosed()) bFindStartPoint = true;
				}
				if (!bFindStartPoint)
				{
					seq.addPoint(iCurX + 1, iCurY - 1);
					if (seq.get_IsClosed()) bFindStartPoint = true;
				}
				iDirect = 4;
				iCurX += 1; iCurY -= 1;
				pData[iCurY * iCol + iCurX] = IN_OUTLINE;
			}
			else if (iCurX + 1 < iCol && pData[iCurY * iCol + iCurX + 1] != 0)
			{
				seq.addPoint(iCurX, iCurY);
				if (seq.get_IsClosed()) bFindStartPoint = true;
				if (!bFindStartPoint)
				{
					seq.addPoint(iCurX + 1, iCurY);
					if (seq.get_IsClosed()) bFindStartPoint = true;
				}
				if (!bFindStartPoint)
				{
					seq.addPoint(iCurX + 2, iCurY);
					if (seq.get_IsClosed()) bFindStartPoint = true;
				}

				iDirect = 1;
				iCurX += 1;
				pData[iCurY * iCol + iCurX] = IN_OUTLINE;
			}
			else if (iCurX + 1 < iCol && iCurY + 1 < iRow && pData[(iCurY + 1)* iCol + iCurX + 1] != 0)
			{
				seq.addPoint(iCurX, iCurY);
				if (seq.get_IsClosed()) bFindStartPoint = true;
				if (!bFindStartPoint)
				{
					seq.addPoint(iCurX + 1, iCurY);
					if (seq.get_IsClosed()) bFindStartPoint = true;
				}
				if (!bFindStartPoint)
				{
					seq.addPoint(iCurX + 1, iCurY + 1);
					if (seq.get_IsClosed()) bFindStartPoint = true;
				}
				if (!bFindStartPoint)
				{
					seq.addPoint(iCurX + 2, iCurY + 1);
					if (seq.get_IsClosed()) bFindStartPoint = true;
				}
				iDirect = 1;
				iCurX += 1; iCurY += 1;
				pData[iCurY * iCol + iCurX] = IN_OUTLINE;
			}
			else
			{
				seq.addPoint(iCurX, iCurY);
				if (seq.get_IsClosed()) bFindStartPoint = true;
				if (!bFindStartPoint)
				{
					seq.addPoint(iCurX + 1, iCurY);
					if (seq.get_IsClosed()) bFindStartPoint = true;
				}
				return;
			}
		}
		else if (iDirect == 4)
		{
			if (iCurX - 1 >= 0 && iCurY - 1 >= 0 && pData[(iCurY - 1) * iCol + iCurX - 1] != 0)
			{
				iDirect = 3;
				seq.addPoint(iCurX - 1, iCurY);
				if (seq.get_IsClosed()) bFindStartPoint = true;
				iCurX -= 1; iCurY -= 1;
				pData[iCurY * iCol + iCurX] = IN_OUTLINE;
			}
			else if (iCurY - 1 >= 0 && pData[(iCurY - 1) * iCol + iCurX] != 0)
			{
				seq.addPoint(iCurX, iCurY - 1);
				if (seq.get_IsClosed()) bFindStartPoint = true;
				iDirect = 4;
				iCurY -= 1;
				pData[iCurY * iCol + iCurX] = IN_OUTLINE;
			}
			else if (iCurX + 1 < iCol && iCurY - 1 >= 0 && pData[(iCurY - 1) * iCol + iCurX + 1] != 0)
			{
				iDirect = 4;
				seq.addPoint(iCurX + 1, iCurY);
				if (seq.get_IsClosed()) bFindStartPoint = true;
				if (!bFindStartPoint)
				{
					seq.addPoint(iCurX + 1, iCurY - 1);
					if (seq.get_IsClosed()) bFindStartPoint = true;
				}
				iCurX += 1;  iCurY -= 1;
				pData[iCurY * iCol + iCurX] = IN_OUTLINE;
			}
			else if (iCurX + 1 < iCol && pData[iCurY * iCol + iCurX + 1] != 0)
			{
				seq.addPoint(iCurX + 1, iCurY);
				if (seq.get_IsClosed()) bFindStartPoint = true;
				if (!bFindStartPoint)
				{
					seq.addPoint(iCurX + 2, iCurY);
					if (seq.get_IsClosed()) bFindStartPoint = true;
				}
				iDirect = 1;
				iCurX += 1;
				pData[iCurY * iCol + iCurX] = IN_OUTLINE;
			}
			else if (iCurY + 1 < iRow && iCurX + 1 < iCol && pData[(iCurY + 1)* iCol + iCurX + 1] != 0)
			{
				seq.addPoint(iCurX + 1, iCurY);
				if (seq.get_IsClosed()) bFindStartPoint = true;
				if (!bFindStartPoint)
				{
					seq.addPoint(iCurX + 1, iCurY + 1);
					if (seq.get_IsClosed()) bFindStartPoint = true;
				}
				if (!bFindStartPoint)
				{
					seq.addPoint(iCurX + 2, iCurY + 1);
					if (seq.get_IsClosed()) bFindStartPoint = true;
				}
				iDirect = 1;
				iCurX += 1; iCurY += 1;
				pData[iCurY * iCol + iCurX] = IN_OUTLINE;
			}
			else if (iCurY + 1 < iRow && pData[(iCurY + 1) * iCol + iCurX] != 0)
			{
				seq.addPoint(iCurX + 1, iCurY);
				if (seq.get_IsClosed()) bFindStartPoint = true;
				if (!bFindStartPoint)
				{
					seq.addPoint(iCurX + 1, iCurY + 1);
					if (seq.get_IsClosed()) bFindStartPoint = true;
				}
				if (!bFindStartPoint)
				{
					seq.addPoint(iCurX + 1, iCurY + 2);
					if (seq.get_IsClosed()) bFindStartPoint = true;
				}
				iDirect = 2;
				iCurY += 1;
				pData[iCurY * iCol + iCurX] = IN_OUTLINE;
			}
			else if (iCurX - 1 >= 0 && iCurY + 1 < iRow && pData[(iCurY + 1) * iCol + iCurX - 1] != 0)
			{
				seq.addPoint(iCurX + 1, iCurY);
				if (seq.get_IsClosed()) bFindStartPoint = true;
				if (!bFindStartPoint)
				{
					seq.addPoint(iCurX + 1, iCurY + 1);
					if (seq.get_IsClosed()) bFindStartPoint = true;
				}
				if (!bFindStartPoint)
				{
					seq.addPoint(iCurX, iCurY + 1);
					if (seq.get_IsClosed()) bFindStartPoint = true;
				}
				if (!bFindStartPoint)
				{
					seq.addPoint(iCurX, iCurY + 2);
					if (seq.get_IsClosed()) bFindStartPoint = true;
				}
				iDirect = 2;
				iCurX -= 1; iCurY += 1;
				pData[iCurY * iCol + iCurX] = IN_OUTLINE;
			}
			else
			{
				seq.addPoint(iCurX + 1, iCurY);
				if (seq.get_IsClosed()) bFindStartPoint = true;
				if (!bFindStartPoint)
				{
					seq.addPoint(iCurX + 1, iCurY + 1);
					if (seq.get_IsClosed()) bFindStartPoint = true;
				}
				return;
			}
		}
	}
}

bool TraceOutline(const string& tifPath, const string& shpPath, BYTE* pDataPool)
{
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
	GDALDataset* pDataset = (GDALDataset*)GDALOpen(tifPath.c_str(), GA_ReadOnly);

	if (!pDataset)
	{
		return false;
	}

	int nCols = pDataset->GetRasterXSize(), nRows = pDataset->GetRasterYSize();

	BYTE* pData = pDataPool;

	if (pData == NULL)
	{
		pData = new BYTE[nCols * nRows];
	}
	int nDataLen = nCols * nRows;

	int nBandList[3] = { 1, 2, 3 };

	pDataset->RasterIO(GF_Read, 0, 0, nCols, nRows, pData, nCols, nRows, GDT_Byte, 1, nBandList, 0, 0, 0);

	for (int i = 0; i < nDataLen; ++i)
	{
		if (pData[i] != 0xff)
		{
			pData[i] = 0x00;
		}
	}


	double dTrans[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

	pDataset->GetGeoTransform(dTrans);

	GDALClose(pDataset);

	///************************************************************************/
	///*			                    去自相交                                 */
	///************************************************************************/
	//while (DeSelfIntersect<BYTE>(pData, nCols, nRows, 1, 0) > 0)
	//{
	//	continue;
	//}

	/************************************************************************/
	/*						     跟踪出所有边界                              */
	/************************************************************************/
	vector<OGRLinearRing> seqs;
	for (int i = 0; i < nRows; ++i)
	{
		for (int j = 0; j < nCols; ++j)
		{
			int idx = i * nCols + j;
			if (pData[idx] == 0xff)
			{
				if (i == 0 || i == nRows - 1 || j == 0 || j == nCols - 1 || pData[idx - 1] == 0 || pData[idx + 1] == 0 || pData[idx - nCols] == 0 || pData[idx + nCols] == 0)
				{
					OGRLinearRing seq;
					TraceOutline(pData, nCols, nRows, j, i, seq);
					seqs.push_back(seq);
				}
			}
		}
	}

	if (seqs.size() <= 0)
	{
		if (pDataPool == NULL)
		{
			delete pData; pData = NULL;
		}
		return true;
	}

	/************************************************************************/
	/*							  计算节点地理坐标                           */
	/************************************************************************/
	for (int i = 0; i < seqs.size(); ++i)
	{
		for (int j = 0; j < seqs[i].getNumPoints(); ++j)
		{
			seqs[i].setPoint(j, seqs[i].getX(j) * dTrans[1] + seqs[i].getY(j) * dTrans[2] + dTrans[0], seqs[i].getX(j) * dTrans[4] + seqs[i].getY(j) * dTrans[5] + dTrans[3]);
		}
	}
	/************************************************************************/
	/*							      写成文件                               */
	/************************************************************************/
	/* 2017/07/26 测试                                                      */
	/* 测试结果显示,当给OGRPolygon赋予多个OGRLinearRing时,OGRPolygon自动转化为 */
	/* OGRMultiPolygon,并且在加OGRLinearRing时,各个OGRLinearRing之间的关系是  */
	/* 自动判断的                                                            */
	/************************************************************************/
	OGRPolygon* pPolygon = (OGRPolygon*)OGRGeometryFactory::createGeometry(wkbPolygon);
	for (int i = 0; i < seqs.size(); ++i)
	{
		OGRGeometry* pLineRing = NULL;
		GeometryCompress(&seqs[i], pLineRing);
		((OGRLinearRing*)pLineRing)->closeRings();
		pPolygon->addRing(((OGRLinearRing*)pLineRing));
	}
	SaveSingleGeometryAsFile(shpPath, pPolygon);
	if (pDataPool == NULL)
	{
		delete pData; pData = NULL;
	}
	return true;
}

void DectectionBackgroundColor(const string& orthopath, int& m_BackGroundColor)
{
	GDALDataset* pDataset = (GDALDataset*)GDALOpen(orthopath.c_str(), GA_ReadOnly);
	if (pDataset == NULL)
	{
		return;
	}
	GDALDataType dT = pDataset->GetRasterBand(1)->GetRasterDataType();
	int nCols = pDataset->GetRasterXSize();
	int nRows = pDataset->GetRasterYSize();
	int nPixelCount = nCols * nRows;
	int nPixelSize = 0;
	if (dT == GDT_Byte)
	{
		nPixelSize = 1;
	}
	else if (dT == GDT_UInt16)
	{
		nPixelSize = 2;
	}
	int nZoom = 1 << (int)(log((double)max(nCols, nRows)) / log(4096.0));
	int nProperCols = nCols / nZoom;
	int nProperRows = nRows / nZoom;
	int nProperLens = nProperCols * nProperRows * nPixelSize;
	int nBand = 1;
	BYTE* pData = new BYTE[nProperLens];
	pDataset->RasterIO(GF_Read, 0, 0, nCols, nRows, pData, nProperCols, nProperRows, dT, 1, &nBand, nPixelSize, nProperCols * nPixelSize, nProperLens);

	int iWhitePointsCount = 0;
	int iBlackPointsCount = 0;

	if (dT == GDT_Byte)
	{
		BYTE CornerColor[4] = { 0x0f, 0x0f, 0x0f, 0x0f };
		CornerColor[0] = pData[0];
		CornerColor[1] = pData[nProperCols - 1];
		CornerColor[2] = pData[(nProperRows - 1) * nProperCols];
		CornerColor[3] = pData[nProperCols * nProperRows - 1];
		for (int i = 0; i < 4; ++i)
		{
			if (CornerColor[i] == 0x00) iBlackPointsCount++;
			if (CornerColor[i] == 0xff) iWhitePointsCount++;
		}
	}
	else if (dT == GDT_UInt16)
	{
		short CornerColor[4] = { 0, 0, 0, 0 };
		CornerColor[0] = ((unsigned short*)pData)[0];
		CornerColor[1] = ((unsigned short*)pData)[nProperCols - 1];
		CornerColor[2] = ((unsigned short*)pData)[(nProperRows - 1) * nProperCols];
		CornerColor[3] = ((unsigned short*)pData)[nProperCols * nProperRows - 1];
		for (int i = 0; i < 4; ++i)
		{
			if (CornerColor[i] == 0x0000) iBlackPointsCount++;
			if (CornerColor[i] == 0xffff) iWhitePointsCount++;
		}
	}

	if (iBlackPointsCount > iWhitePointsCount)
	{
		m_BackGroundColor = 0;
	}
	else if (iWhitePointsCount > iBlackPointsCount)
	{
		m_BackGroundColor = 1;
	}
	else
	{
		m_BackGroundColor = -1;
	}
	delete pData; pData = NULL;
	GDALClose(pDataset);
}

bool MaskTraceOutline(const string& tifPath, const string& shpPath, BYTE* pDataPool)
{
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
	GDALDataset* pDataset = (GDALDataset*)GDALOpen(tifPath.c_str(), GA_ReadOnly);
	if (!pDataset)
	{
		return false;
	}

	int nCols = pDataset->GetRasterXSize(), nRows = pDataset->GetRasterYSize();

	BYTE* pData = pDataPool;

	if (pData == NULL)
	{
		pData = new BYTE[nCols * nRows];
	}

	int nBandList[3] = { 1, 2, 3 };

	pDataset->RasterIO(GF_Read, 0, 0, nCols, nRows, pData, nCols, nRows, GDT_Byte, 1, nBandList, 0, 0, 0);

	double dTrans[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

	pDataset->GetGeoTransform(dTrans);

	GDALClose(pDataset);

	/************************************************************************/
	/*						     跟踪出所有边界                              */
	/************************************************************************/
	vector<OGRLinearRing> seqs;
	for (int i = 0; i < nRows; ++i)
	{
		for (int j = 0; j < nCols; ++j)
		{
			int idx = i * nCols + j;
			if (pData[idx] == 0xff)
			{
				if (i == 0 || i == nRows - 1 || j == 0 || j == nCols - 1 || pData[idx - 1] == 0 || pData[idx + 1] == 0 || pData[idx - nCols] == 0 || pData[idx + nCols] == 0)
				{
					OGRLinearRing seq;
					TraceOutline(pData, nCols, nRows, j, i, seq);
					seqs.push_back(seq);
				}
			}
		}
	}

	if (seqs.size() <= 0)
	{
		if (pDataPool == NULL)
		{
			delete pData; pData = NULL;
		}
		remove(shpPath.c_str());
		return true;
	}

	/************************************************************************/
	/*							  计算节点地理坐标                           */
	/************************************************************************/
	for (int i = 0; i < seqs.size(); ++i)
	{
		for (int j = 0; j < seqs[i].getNumPoints(); ++j)
		{
			seqs[i].setPoint(j, seqs[i].getX(j) * dTrans[1] + seqs[i].getY(j) * dTrans[2] + dTrans[0], seqs[i].getX(j) * dTrans[4] + seqs[i].getY(j) * dTrans[5] + dTrans[3]);
		}
	}

	OGRPolygon* pPolygon = (OGRPolygon*)OGRGeometryFactory::createGeometry(wkbPolygon);
	for (int i = 0; i < seqs.size(); ++i)
	{
		OGRGeometry* pLineRing = NULL;
		GeometryCompress(&seqs[i], pLineRing);
		pPolygon->addRing((OGRLinearRing*)pLineRing);
	}
	SaveSingleGeometryAsFile(shpPath, pPolygon);
	if (pDataPool == NULL)
	{
		delete pData; pData = NULL;
	}
	return true;
}

bool CloudTraceOutline(const string& tifPath, const string& shpPath, const vector<BYTE>& targets)
{
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
	GDALDataset* pDataset = (GDALDataset*)GDALOpen(tifPath.c_str(), GA_ReadOnly);
	if (!pDataset)
	{
		return false;
	}
	int nCols = pDataset->GetRasterXSize();
	int nRows = pDataset->GetRasterYSize();
	vector<BYTE> Data1(nCols * nRows, 0);
	vector<BYTE> Data2(nCols * nRows, 0);
	int nBandList[3] = { 1, 2, 3 };
	pDataset->RasterIO(GF_Read, 0, 0, nCols, nRows, Data1.data(), nCols, nRows, GDT_Byte, 1, nBandList, 0, 0, 0);
	double dTrans[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
	pDataset->GetGeoTransform(dTrans);
	GDALClose(pDataset);

	GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName("ESRI Shapefile");
	if (!poDriver)
		return false;
	poDriver->Delete(shpPath.c_str());
	GDALDataset* poDS = poDriver->Create(shpPath.c_str(), 0, 0, 0, GDT_Unknown, NULL);
	if (!poDS)
		return false;
	OGRLayer* poLayer = poDS->CreateLayer("polygon");
	if (!poLayer)
		return false;

	OGRFieldDefn oFeild("WHU_RS", OFTInteger);
	if (poLayer->CreateField(&oFeild) != OGRERR_NONE)
	{
		printf("Create WHU_RS Field Failed.\n");
		return false;
	}

	for (int i = 0; i < targets.size(); ++i)
	{
		int nBandLen = nCols * nRows;
		memcpy(Data2.data(), Data1.data(), nBandLen);
		for (int j = 0; j < nBandLen; ++j)
		{
			if (Data2[j] == targets[i])
				Data2[j] = 0xff;
			else
				Data2[j] = 0x00;
		}
		//while (DeSelfIntersect<BYTE>(Data2.data(), nCols, nRows, 0xff, 0x00) > 0)
		//{
		//	continue;
		//}
		vector<OGRLinearRing*> seqs;
		for (int i = 0; i < nRows; ++i)
		{
			for (int j = 0; j < nCols; ++j)
			{
				int idx = i * nCols + j;
				if (Data2[idx] == 0xff)
				{
					if (i == 0 || i == nRows - 1 || j == 0 || j == nCols - 1 || Data2[idx - 1] == 0 || Data2[idx + 1] == 0 || Data2[idx - nCols] == 0 || Data2[idx + nCols] == 0)
					{
						OGRLinearRing *seq = (OGRLinearRing *)OGRGeometryFactory::createGeometry(wkbLinearRing);
						TraceOutline(Data2.data(), nCols, nRows, j, i, *seq);
						seqs.push_back(seq);
					}
				}
			}
		}

		if (seqs.size() <= 0)
		{
			continue;
		}

		for (int i = 0; i < seqs.size(); ++i)
		{
			int nPointNum = seqs[i]->getNumPoints();
			for (int j = 0; j < nPointNum; ++j)
			{
				seqs[i]->setPoint(j, seqs[i]->getX(j) * dTrans[1] + seqs[i]->getY(j) * dTrans[2] + dTrans[0], seqs[i]->getX(j) * dTrans[4] + seqs[i]->getY(j) * dTrans[5] + dTrans[3]);
			}
		}

		OGRPolygon* pPolygon = (OGRPolygon*)OGRGeometryFactory::createGeometry(wkbPolygon);
		for (int i = 0; i < seqs.size(); ++i)
		{
			OGRGeometry* pLineRing = NULL;
			GeometryCompress(seqs[i], pLineRing);
			pPolygon->addRing((OGRLinearRing*)pLineRing);
		}

		if (pPolygon->getGeometryType() == wkbPolygon)
		{
			OGRFeature *poFeature = OGRFeature::CreateFeature(poLayer->GetLayerDefn());
			if (!poFeature)
				return false;
			poFeature->SetField("WHU_RS", targets[i]);
			poFeature->SetGeometry(pPolygon);
			poLayer->CreateFeature(poFeature);
			OGRFeature::DestroyFeature(poFeature);
		}
		else if (pPolygon->getGeometryType() == wkbMultiPolygon)
		{
			int nPolygonCount = ((OGRMultiPolygon*)pPolygon)->getNumGeometries();
			for (int i = 0; i < nPolygonCount; ++i)
			{
				OGRFeature *poFeature = OGRFeature::CreateFeature(poLayer->GetLayerDefn());
				if (!poFeature)
					return false;
				poFeature->SetField("WHU_RS", targets[i]);
				poFeature->SetGeometry(((OGRMultiPolygon*)pPolygon)->getGeometryRef(i));
				poLayer->CreateFeature(poFeature);
				OGRFeature::DestroyFeature(poFeature);
			}
		}
	}
	GDALClose(poDS);
	return true;
}

void CloudOrthoTraceOutLine(const string& orthopath, const string& shppath, int m_BackGroundColor)
{
	/*-----文件不存在-----*/
	if (_access(orthopath.c_str(), 0) == -1)
	{
		return;
	}
	int i = 0, j = 0;
	/*-----读取TIFF数据-----*/
	GDALDataset* pTifDataSet = (GDALDataset*)GDALOpen(orthopath.c_str(), GA_ReadOnly);
	int nProperLevel = 0; //合适的层级为原始层级
	int nRealCols = pTifDataSet->GetRasterXSize();
	int nRealRows = pTifDataSet->GetRasterYSize();
	int nMaxWidth = max(nRealRows, nRealCols);
	if (nMaxWidth <= 4096)
	{
		nProperLevel = 0;
	}
	else
	{
		nProperLevel = log(nMaxWidth / 4096.0) / log(2.0);
	}
	int nBandCount = pTifDataSet->GetRasterCount();
	int iCols = nRealCols / pow(2.0, nProperLevel);
	int iRows = nRealRows / pow(2.0, nProperLevel);
	int iDataLen = iCols * iRows;
	int iZoom = pow(2.0, nProperLevel);
	double GeoTrans[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
	pTifDataSet->GetGeoTransform(GeoTrans);
	GeoTrans[1] *= iZoom;
	GeoTrans[2] *= iZoom;
	GeoTrans[4] *= iZoom;
	GeoTrans[5] *= iZoom;

	vector<BYTE> Data(iDataLen, 0);

	int nBandMap[4] = { 1, 2, 3, 4 };
	if (pTifDataSet->RasterIO(GF_Read, 0, 0, nRealCols, nRealRows, Data.data(), iCols, iRows, GDT_Byte, nBandCount, nBandMap, 1, iCols, iCols * iRows, 0) != CE_None)
	{
		return;
	}

	for (i = 0; i < iDataLen; ++i)
	{
		if (Data[i] != 0 && Data[i] != 0xff)
		{
			Data[i] = 0xff;
		}
		else
		{
			Data[i] = 0x00;
		}
	}
	Erode(Data.data(), iCols, iRows, 10);
	//while (DeSelfIntersect<BYTE>(Data.data(), iCols, iRows) > 0)
	//{
	//	continue;
	//}
	vector<OGRLinearRing> seqs;
	for (i = 0; i < iRows; ++i)
	{
		for (j = 0; j < iCols; ++j)
		{
			int idx = i * iCols + j;
			if (Data[idx] == 0xff)
			{
				if (i == 0 || i == iRows - 1 || j == 0 || j == iCols - 1 || Data[idx - 1] == 0 || Data[idx + 1] == 0 || Data[idx - iCols] == 0 || Data[idx + iCols] == 0)
				{
					OGRLinearRing seq;
					TraceOutline(Data.data(), iCols, iRows, j, i, seq);
					seq.closeRings();
					seqs.push_back(seq);
				}
			}
		}
	}
	if (seqs.size() == 0)
	{
		GDALClose(pTifDataSet);
		return;
	}
	for (int i = 0; i < seqs.size(); ++i)
	{
		for (int j = 0; j < seqs[i].getNumPoints(); ++j)
		{
			seqs[i].setPoint(j, seqs[i].getX(j) * GeoTrans[1] + seqs[i].getY(j) * GeoTrans[2] + GeoTrans[0], seqs[i].getX(j) * GeoTrans[4] + seqs[i].getY(j) * GeoTrans[5] + GeoTrans[3]);
		}
	}
	OGRPolygon* pPolygon = (OGRPolygon*)OGRGeometryFactory::createGeometry(wkbPolygon);
	for (int i = 0; i < seqs.size(); ++i)
	{
		OGRGeometry* pLineRing = NULL;
		GeometryCompress(&seqs[i], pLineRing);
		pPolygon->addRing((OGRLinearRing*)pLineRing);
	}
	SaveSingleGeometryAsFile(shppath, pPolygon);
	OGRGeometryFactory::destroyGeometry(pPolygon);
}

void OrthoTraceOutLine(const string& orthopath, const string& shppath, int m_BackGroundColor)
{
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
	if (_access(orthopath.c_str(), 0) == -1)
	{
		return;
	}
	string cldname = orthopath;
	cldname.erase(cldname.begin() + cldname.find_last_of("."), cldname.end());
	cldname.append("_cld.tif");
	if (_access(cldname.c_str(), 0) != -1)
	{
		CloudOrthoTraceOutLine(cldname, shppath, m_BackGroundColor);
		return;
	}
	int i = 0, j = 0;
	GDALDataset* pTifDataSet = (GDALDataset*)GDALOpen(orthopath.c_str(), GA_ReadOnly);
	int nProperLevel = 0; //合适的层级为原始层级
	int nRealCols = pTifDataSet->GetRasterXSize();
	int nRealRows = pTifDataSet->GetRasterYSize();
	int nPixelCount = 0;
	int nPixelSize = 0;
	GDALDataType dT = pTifDataSet->GetRasterBand(1)->GetRasterDataType();
	if (dT == GDT_Byte)
	{
		nPixelSize = 1;
	}
	else if (dT == GDT_UInt16)
	{
		nPixelSize = 2;
	}
	int nMaxWidth = max(nRealRows, nRealCols);
	if (nMaxWidth <= 4096)
	{
		nProperLevel = 0;
	}
	else
	{
		nProperLevel = log(nMaxWidth / 4096.0) / log(2.0);
	}

	int nBandCount = pTifDataSet->GetRasterCount();
	GDALRasterBand* pPorperBand = NULL;
	GDALRasterBand* pBand = pTifDataSet->GetRasterBand(1);
	if (nProperLevel == 0)
	{
		pPorperBand = pBand;
	}
	else
	{
		pPorperBand = pBand->GetOverview(nProperLevel - 1);
	}

	int iCols = 0, iRows = 0, iDataLen = 0, iZoom = 0;
	if (pPorperBand != NULL)
	{
		iZoom = floor(pBand->GetXSize() / (float)pPorperBand->GetXSize() + 0.6);
	}
	else
	{
		iZoom = 1;
	}
	iCols = pPorperBand->GetXSize();
	iRows = pPorperBand->GetYSize();
	iDataLen = iCols * iRows * nPixelSize;   //每个波段数据长度
	nPixelCount = iCols * iRows;             //像素数量

	double GeoTrans[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
	pTifDataSet->GetGeoTransform(GeoTrans);

	GeoTrans[1] *= iZoom;
	GeoTrans[2] *= iZoom;
	GeoTrans[4] *= iZoom;
	GeoTrans[5] *= iZoom;

	vector<BYTE> RGBData(iDataLen * nBandCount, 0);
	vector<BYTE> GRYData(iDataLen, 0);

	int nBandMap[4] = { 1, 2, 3, 4 };
	pTifDataSet->RasterIO(GF_Read, 0, 0, nRealCols, nRealRows, RGBData.data(), iCols, iRows, dT, nBandCount, nBandMap, nPixelSize, iCols * nPixelSize, iCols * iRows * nPixelSize, 0);

	if (dT == GDT_Byte)
	{
		Multichannel2Gray<BYTE>(RGBData.data(), GRYData.data(), nBandCount, iCols, iRows, RRRGGGBBB);
		BYTE ucBGColor = 0;
		if (m_BackGroundColor == 1)
		{
			ucBGColor = 0xff;
		}
		for (i = 0; i < nPixelCount; ++i)
		{
			if (GRYData[i] == ucBGColor)
			{
				GRYData[i] = 0x00;
			}
			else GRYData[i] = 0xff;
		}
	}
	else if (dT == GDT_UInt16)
	{
		Multichannel2Gray<unsigned short>((unsigned short*)RGBData.data(), (unsigned short*)GRYData.data(), nBandCount, iCols, iRows, RRRGGGBBB);
		short stBGColor = 0;
		if (m_BackGroundColor == 1)
		{
			stBGColor = 0xffff;
		}
		for (i = 0; i < nPixelCount; ++i)
		{
			if (((unsigned short*)GRYData.data())[i] == stBGColor)
			{
				GRYData[i] = 0x00;
			}
			else
			{
				GRYData[i] = 0xff;
			}
		}
	}
	
	Erode(GRYData.data(), iCols, iRows, 10);
	//while (DeSelfIntersect<BYTE>(GRYData.data(), iCols, iRows) > 0)
	//{
	//	continue;
	//}
	/*-----边缘跟踪-----*/
	vector<OGRLinearRing> seqs;
	for (i = 0; i < iRows; ++i)
	{
		for (j = 0; j < iCols; ++j)
		{
			int idx = i * iCols + j;
			if (GRYData[idx] == 0xff)
			{
				if (i == 0 || i == iRows - 1 || j == 0 || j == iCols - 1 || GRYData[idx - 1] == 0 || GRYData[idx + 1] == 0 || GRYData[idx - iCols] == 0 || GRYData[idx + iCols] == 0)
				{
					OGRLinearRing seq;
					TraceOutline(GRYData.data(), iCols, iRows, j, i, seq);
					seqs.push_back(seq);
				}
			}
		}
	}
	/*-----未找出任何边界-----*/
	if (seqs.size() == 0)
	{
		GDALClose(pTifDataSet);
		return;
	}

	/*-----找出边界-----*/
	int iSeqCount = seqs.size();
	int iMaxPointCount = -INT_MAX;
	int iMaxPointCountIndex = -1;
	for (i = 0; i < iSeqCount; ++i)
	{
		if (seqs[i].getNumPoints() > iMaxPointCount)
		{
			iMaxPointCount = seqs[i].getNumPoints();
			iMaxPointCountIndex = i;
		}
	}
	/*-----计算地理坐标-----*/
	int iBoundryPointCount = seqs[iMaxPointCountIndex].getNumPoints();
	for (i = 0; i < iBoundryPointCount; ++i)
	{
		seqs[iMaxPointCountIndex].setPoint(i, seqs[iMaxPointCountIndex].getX(i) * GeoTrans[1] + seqs[iMaxPointCountIndex].getY(i) * GeoTrans[2] + GeoTrans[0], seqs[iMaxPointCountIndex].getX(i) * GeoTrans[4] + seqs[iMaxPointCountIndex].getY(i) * GeoTrans[5] + GeoTrans[3]);
	}

	OGRPolygon* pPolygon = (OGRPolygon*)OGRGeometryFactory::createGeometry(wkbPolygon);
	seqs[iMaxPointCountIndex].closeRings();
	pPolygon->addRing(&seqs[iMaxPointCountIndex]);
	//double SimplifyThread = 40 * GeoTrans[1];
	//OGRGeometry* pPolygonSimplfy = pPolygon->SimplifyPreserveTopology(SimplifyThread);

	SaveSingleGeometryAsFile(shppath, pPolygon);

	GDALClose(pTifDataSet);
	OGRGeometryFactory::destroyGeometry(pPolygon);
	//OGRGeometryFactory::destroyGeometry(pPolygonSimplfy);
}
