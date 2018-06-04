#ifndef  _DELAYNAY_TRIANGLEATION_H_20170815_
#define  _DELAYNAY_TRIANGLEATION_H_20170815_

#include "GeometryInclude.h"

using namespace std;

struct Triangle
{
	OGRPoint vertices[3];
};

GEOMETRYAPI void  DelaunayTriangleation(std::vector<OGRPoint>& vertexs, std::vector<Triangle>& triangles);

GEOMETRYAPI void  DelaunayTriangleation(OGRGeometry* pGeometry, std::vector<Triangle>& triangles);

#endif