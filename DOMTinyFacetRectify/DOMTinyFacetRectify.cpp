// DOMTinyFacetRectify.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "DOMTinyFacetRectify.h"
#include "TinyFacetRectify.h"

DOMTINYFACETRECTIFY_API bool DOMTinyFacetRectify_images(const char* lpstrCpPts, const char* lpstrTarImage)
{
	return TinyFacetRectify(lpstrCpPts, lpstrTarImage);
}

DOMTINYFACETRECTIFY_API void DOMminiTinyFacetRectify8bit(const double *ltfw, const double *rtfw, const unsigned char* plindata, int lrows, int lcols, int nPxlBytes, int slrow, int slcol, CorrespondPt* CpPt, int npt, unsigned char* poutdata, int nBkgrdClr /*= 0*/)
{
	smallareaTinyFacet(ltfw, rtfw, (unsigned char*)plindata, lrows, lcols, nPxlBytes, slrow, slcol, CpPt, npt, (unsigned char*)poutdata, nBkgrdClr);
}

DOMTINYFACETRECTIFY_API void DOMminiTinyFacetRectify16bit(const double *ltfw, const double *rtfw, const unsigned short* plindata, int lrows, int lcols, int nPxlBytes, int slrow, int slcol, CorrespondPt* CpPt, int npt, unsigned short* poutdata, int nBkgrdClr /*= 0*/)
{
	smallareaTinyFacet(ltfw, rtfw, (unsigned short*)plindata, lrows, lcols, nPxlBytes, slrow, slcol, CpPt, npt, (unsigned short*)poutdata, nBkgrdClr);
}
