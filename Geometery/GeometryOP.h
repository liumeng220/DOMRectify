#ifndef  _GEOMETRY_H_
#define  _GEOMETRY_H_

#include "GeometryInclude.h"
#include "DelaunayTriangleation.h"

GEOMETRYAPI OGREnvelope GetRasterEnvelope(GDALDataset* pDataSet);

GEOMETRYAPI void   GeometryCompress(OGRGeometry* pGeometry, OGRGeometry*& pCompressGeometry); //无损压缩

GEOMETRYAPI void   GeometryFilter(OGRGeometry* pGeometry, OGRGeometry*& pFilterGeometry);     //过滤重复点

GEOMETRYAPI void   GeometryZoom(const string& srcfilename, const string& dstfilename, double scale); //缩放

GEOMETRYAPI void   GeometryTranslate(OGRGeometry* pGeometry, double x, double y); //平移

GEOMETRYAPI bool   Intersect(const OGREnvelope& e1, const OGREnvelope& e2, OGREnvelope& cross);

GEOMETRYAPI bool   Intersect(const OGREnvelope& e1, const OGREnvelope& e2);

GEOMETRYAPI void   Union(const OGREnvelope& e1, const OGREnvelope& e2, OGREnvelope& e);

GEOMETRYAPI OGRPolygon*    EnvelopeToPolygon(const OGREnvelope& enve);

GEOMETRYAPI OGRLinearRing* LineString2LinearRing(OGRLineString* pLinearRing);

GEOMETRYAPI void GraphCut(int* pCost, BYTE* pMask, int* pIndex, int nCols, int nRows, BYTE nSrc, BYTE nDst, BYTE nNone, BYTE nSrcDst);

#endif