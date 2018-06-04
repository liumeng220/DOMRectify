#ifdef DOMTINYFACETRECTIFY_EXPORTS
//#define DOMTINYFACETRECTIFY_API __declspec(dllexport)
//#else
//#define DOMTINYFACETRECTIFY_API __declspec(dllimport)
//#endif

#define DOMTINYFACETRECTIFY_API _declspec(dllexport)
#else
#define DOMTINYFACETRECTIFY_API _declspec(dllimport)
#pragma comment(lib, "DOMTinyFacetRectify.lib")
#pragma message("Automatically Linking With DOMTinyFacetRectify.dll")
#endif


#include "CorrespondPtIO.h"
/************************************************************************/
/*                           左影像：待纠正影像                           */
/*                           右影像：底图DOM                             */
/************************************************************************/
/*                           lpstrCpPts格式                             */
/*                           保存影像路径                                */
/************************************************************************/

DOMTINYFACETRECTIFY_API bool DOMTinyFacetRectify_images(const char* lpstrCpPts, const char* lpstrTarImage);

//针对匹配点进行小面元纠正，用于纠正影像内存块（左影像位待纠正影像）
//ltfw， rtfw 左右影像的仿射变换参数
//plindata 待纠正影像块 poutdata 纠正后影像块
//lrows lcols 左影像块的行列
//slrow slcol 左影像的起始行列
//nPxlBytes 波段数量
//CorrespondPt* CpPt, int npt 纠正后的匹配点坐标
DOMTINYFACETRECTIFY_API void DOMminiTinyFacetRectify8bit(const double *ltfw, const double *rtfw, const unsigned char* plindata, int lrows, int lcols, int nPxlBytes, int slrow, int slcol, CorrespondPt* CpPt, int npt, unsigned char* poutdata, int nBkgrdClr = 0);

DOMTINYFACETRECTIFY_API void DOMminiTinyFacetRectify16bit(const double *ltfw, const double *rtfw, const unsigned short* plindata, int lrows, int lcols, int nPxlBytes, int slrow, int slcol, CorrespondPt* CpPt, int npt, unsigned short* poutdata, int nBkgrdClr = 0);
