#include "DelaunayTriangleation.h"
#include <windows.h>
#include "gpc.h"
#include "Triangle/Triangle.h"

void DelaunayTriangleation(std::vector<OGRPoint>& vertexs, std::vector<Triangle>& triangles)
{
	if (vertexs.size() == 0)
	{
		return;
	}
	int i = 0, j = 0;
	std::vector<OGRPoint> tempVertex;
	/*-----滤除重复点-----*/
	int iVerticesCount = vertexs.size();
	for (i = 0; i < iVerticesCount; ++i)
	{
		for (j = i + 1; j < iVerticesCount; ++j)
		{
			if (vertexs[i].getX() == vertexs[j].getX() && vertexs[i].getY() == vertexs[j].getY())
			{
				break;
			}
		}
		if (j == iVerticesCount)
		{
			tempVertex.push_back(vertexs[i]);
		}
	}

	iVerticesCount = tempVertex.size();
	if (iVerticesCount < 3)
	{
		return;
	}
	triio in, out;
	in.numberofpoints = iVerticesCount;
	in.pointlist = new double[iVerticesCount * 2];
	memset(in.pointlist, 0, sizeof(double) * 2 * iVerticesCount);
	in.numberofsegments = iVerticesCount;
	in.segmentlist = new int[iVerticesCount * 2];
	memset(in.segmentlist, 0, sizeof(int) * 2 * iVerticesCount);

	for (i = 0; i < iVerticesCount; ++i)
	{
		in.pointlist[i * 2 + 0] = tempVertex[i].getX();
		in.pointlist[i * 2 + 1] = tempVertex[i].getY();

		if (i != iVerticesCount - 1)
		{
			in.segmentlist[i * 2 + 0] = i;
			in.segmentlist[i * 2 + 1] = i + 1;
		}
		else
		{
			in.segmentlist[i * 2 + 0] = i;
			in.segmentlist[i * 2 + 1] = 0;
		}
	}

	triangulate("pzN", &in, &out, NULL);
	int iTriangleCount = out.numberoftriangles;
	triangles.resize(iTriangleCount);
	for (i = 0; i < iTriangleCount; ++i)
	{
		triangles[i].vertices[0].setX(in.pointlist[2 * (out.trianglelist[3 * i + 0]) + 0]);
		triangles[i].vertices[0].setY(in.pointlist[2 * (out.trianglelist[3 * i + 0]) + 1]);
		triangles[i].vertices[1].setX(in.pointlist[2 * (out.trianglelist[3 * i + 1]) + 0]);
		triangles[i].vertices[1].setY(in.pointlist[2 * (out.trianglelist[3 * i + 1]) + 1]);
		triangles[i].vertices[2].setX(in.pointlist[2 * (out.trianglelist[3 * i + 2]) + 0]);
		triangles[i].vertices[2].setY(in.pointlist[2 * (out.trianglelist[3 * i + 2]) + 1]);
	}
	triiofree(&in);
	triiofree(&out);
}

void DelaunayTriangleation(const OGRLineString* linestring, std::vector<Triangle>& triangles)
{
	
}

void DelaunayTriangleation(const OGRLinearRing* linearring, std::vector<Triangle>& triangles)
{
	
}

void DelaunayTriangleation(const OGRPolygon* polygon, std::vector<Triangle>& triangles)
{
	if (!polygon)
	{
		return;
	}

	int ninnerringcount = polygon->getNumInteriorRings();
	gpc_polygon gpcpolygon;
	gpcpolygon.num_contours = ninnerringcount + 1;
	gpcpolygon.hole = new int[gpcpolygon.num_contours];
	gpcpolygon.contour = new gpc_vertex_list[gpcpolygon.num_contours];
	/************************************************************************/
	/*					            加入外环                                */
	/************************************************************************/
	const OGRLinearRing* pexterring = polygon->getExteriorRing();
	gpcpolygon.contour[0].num_vertices = pexterring->getNumPoints();
	gpcpolygon.contour[0].vertex = new gpc_vertex[gpcpolygon.contour[0].num_vertices];
	for (int i = 0; i < gpcpolygon.contour[0].num_vertices; ++i)
	{
		gpcpolygon.contour[0].vertex[i].x = pexterring->getX(i);
		gpcpolygon.contour[0].vertex[i].y = pexterring->getY(i);
	}
	gpcpolygon.hole[0] = 0;
	/************************************************************************/
	/*								加入内环                                */
	/************************************************************************/
	for (int i = 0; i < ninnerringcount; ++i)
	{
		const OGRLinearRing* pinnerring = polygon->getInteriorRing(i);
		gpcpolygon.contour[i + 1].num_vertices = pinnerring->getNumPoints();
		gpcpolygon.contour[i + 1].vertex = new gpc_vertex[gpcpolygon.contour[i + 1].num_vertices];
		for (int j = 0; j < gpcpolygon.contour[i + 1].num_vertices; ++j)
		{
			gpcpolygon.contour[i + 1].vertex[j].x = pinnerring->getX(j);
			gpcpolygon.contour[i + 1].vertex[j].y = pinnerring->getY(j);
		}
		gpcpolygon.hole[i + 1] = 1;
	}

	gpc_tristrip tris;
	gpc_polygon_to_tristrip(&gpcpolygon, &tris);

	Triangle tri;
	for (int i = 0; i < tris.num_strips; ++i)
	{
		for (int j = 0; j < tris.strip[i].num_vertices - 2; ++j)
		{
			tri.vertices[0].setX(tris.strip[i].vertex[j + 0].x);
			tri.vertices[0].setY(tris.strip[i].vertex[j + 0].y);

			tri.vertices[1].setX(tris.strip[i].vertex[j + 1].x);
			tri.vertices[1].setY(tris.strip[i].vertex[j + 1].y);

			tri.vertices[2].setX(tris.strip[i].vertex[j + 2].x);
			tri.vertices[2].setY(tris.strip[i].vertex[j + 2].y);

			triangles.push_back(tri);
		}
	}

	gpc_free_polygon(&gpcpolygon);
	gpc_free_tristrip(&tris);
}

void DelaunayTriangleation(const std::vector<OGRPolygon*>& polygons, std::vector<Triangle>& triangles)
{
	int iPolygonCount = (int)polygons.size();
	for (int i = 0; i < iPolygonCount; ++i)
	{
		DelaunayTriangleation(polygons[i], triangles);
	}
}

void DelaunayTriangleation(OGRGeometry* pGeometry, std::vector<Triangle>& triangles)
{
	if (pGeometry == NULL) return;
	OGRwkbGeometryType type = pGeometry->getGeometryType();
	if (type == wkbPolygon)
	{
		DelaunayTriangleation((OGRPolygon *)pGeometry, triangles);
	}
	else if (type == wkbPolygon25D)
	{
		DelaunayTriangleation((OGRPolygon *)pGeometry, triangles);
	}
	else if (type == wkbMultiPolygon)
	{
		OGRMultiPolygon* pMultiPolygon = (OGRMultiPolygon *)pGeometry;
		int iPolygonCount = pMultiPolygon->getNumGeometries();
		for (int i = 0; i < iPolygonCount; ++i)
		{
			DelaunayTriangleation((OGRPolygon *)pMultiPolygon->getGeometryRef(i), triangles);
		}
	}
	else if (type == wkbLinearRing)
	{
		DelaunayTriangleation((OGRLinearRing *)pGeometry, triangles);
	}
	else if (type == wkbLineString)
	{
		DelaunayTriangleation((OGRLineString *)pGeometry, triangles);
	}
}

//void DelaunayTriangleation(std::vector<OGRPoint>& vertexs, std::vector<Triangle>& triangles)
//{
//	if (vertexs.size() == 0)
//	{
//		return;
//	}
//	int i = 0, j = 0;
//	std::vector<OGRPoint> tempVertex;
//	/*-----滤除重复点-----*/
//	int iVerticesCount = vertexs.size();
//	for (i = 0; i < iVerticesCount; ++i)
//	{
//		for (j = i + 1; j < iVerticesCount; ++j)
//		{
//			if (vertexs[i].getX() == vertexs[j].getX() && vertexs[i].getY() == vertexs[j].getY())
//			{
//				break;
//			}
//		}
//		if (j == iVerticesCount)
//		{
//			tempVertex.push_back(vertexs[i]);
//		}
//	}
//
//	iVerticesCount = tempVertex.size();
//	if (iVerticesCount < 3)
//	{
//		return;
//	}
//	triio in, out;
//	in.numberofpoints = iVerticesCount;
//	in.pointlist = new double[iVerticesCount * 2];
//	memset(in.pointlist, 0, sizeof(double)* 2 * iVerticesCount);
//	in.numberofsegments = iVerticesCount;
//	in.segmentlist = new int[iVerticesCount * 2];
//	memset(in.segmentlist, 0, sizeof(int)* 2 * iVerticesCount);
//
//	for (i = 0; i < iVerticesCount; ++i)
//	{
//		in.pointlist[i * 2 + 0] = tempVertex[i].getX();
//		in.pointlist[i * 2 + 1] = tempVertex[i].getY();
//
//		if (i != iVerticesCount - 1)
//		{
//			in.segmentlist[i * 2 + 0] = i;
//			in.segmentlist[i * 2 + 1] = i + 1;
//		}
//		else
//		{
//			in.segmentlist[i * 2 + 0] = i;
//			in.segmentlist[i * 2 + 1] = 0;
//		}
//	}
//
//	triangulate("pzN", &in, &out, NULL);
//	int iTriangleCount = out.numberoftriangles;
//	triangles.resize(iTriangleCount);
//	for (i = 0; i < iTriangleCount; ++i)
//	{
//		triangles[i].vertices[0].setX(in.pointlist[2 * (out.trianglelist[3 * i + 0]) + 0]);
//		triangles[i].vertices[0].setY(in.pointlist[2 * (out.trianglelist[3 * i + 0]) + 1]);
//		triangles[i].vertices[1].setX(in.pointlist[2 * (out.trianglelist[3 * i + 1]) + 0]);
//		triangles[i].vertices[1].setY(in.pointlist[2 * (out.trianglelist[3 * i + 1]) + 1]);
//		triangles[i].vertices[2].setX(in.pointlist[2 * (out.trianglelist[3 * i + 2]) + 0]);
//		triangles[i].vertices[2].setY(in.pointlist[2 * (out.trianglelist[3 * i + 2]) + 1]);
//	}
//	triiofree(&in);
//	triiofree(&out);
//}

//void DelaunayTriangleation(const OGRLineString* linestring, std::vector<Triangle>& triangles)
//{
//	if (linestring == NULL) return;
//	int iVerticesCount = linestring->getNumPoints();
//	if (iVerticesCount < 3) return;
//	triio in, out;
//	in.numberofpoints = iVerticesCount;
//	in.pointlist = new double[iVerticesCount * 2];
//	memset(in.pointlist, 0, sizeof(double)* 2 * iVerticesCount);
//	in.numberofsegments = iVerticesCount;
//	in.segmentlist = new int[iVerticesCount * 2];
//	memset(in.segmentlist, 0, sizeof(int)* 2 * iVerticesCount);
//
//	for (int i = 0; i < iVerticesCount; ++i)
//	{
//		in.pointlist[i * 2 + 0] = linestring->getX(i);
//		in.pointlist[i * 2 + 1] = linestring->getY(i);
//
//		if (i != iVerticesCount - 1)
//		{
//			in.segmentlist[i * 2 + 0] = i;
//			in.segmentlist[i * 2 + 1] = i + 1;
//		}
//		else
//		{
//			in.segmentlist[i * 2 + 0] = i;
//			in.segmentlist[i * 2 + 1] = 0;
//		}
//	}
//
//	triangulate("pzN", &in, &out, NULL);
//	int iTriangleCount = out.numberoftriangles;
//	triangles.resize(iTriangleCount);
//	for (int i = 0; i < iTriangleCount; ++i)
//	{
//		triangles[i].vertices[0].setX(in.pointlist[2 * (out.trianglelist[3 * i + 0]) + 0]);
//		triangles[i].vertices[0].setY(in.pointlist[2 * (out.trianglelist[3 * i + 0]) + 1]);
//		triangles[i].vertices[1].setX(in.pointlist[2 * (out.trianglelist[3 * i + 1]) + 0]);
//		triangles[i].vertices[1].setY(in.pointlist[2 * (out.trianglelist[3 * i + 1]) + 1]);
//		triangles[i].vertices[2].setX(in.pointlist[2 * (out.trianglelist[3 * i + 2]) + 0]);
//		triangles[i].vertices[2].setY(in.pointlist[2 * (out.trianglelist[3 * i + 2]) + 1]);
//	}
//	triiofree(&in);
//	triiofree(&out);
//}
//
//void DelaunayTriangleation(const OGRLinearRing* linearring, std::vector<Triangle>& triangles)
//{
//	if (linearring == NULL) return;
//	int iVerticesCount = linearring->getNumPoints();
//	if (linearring->getX(0) == linearring->getX(iVerticesCount - 1) && linearring->getY(0) == linearring->getY(iVerticesCount - 1))
//	{
//		iVerticesCount -= 1;
//	}
//	if (iVerticesCount < 3) return;
//	triio in, out;
//	in.numberofpoints = iVerticesCount;
//	in.pointlist = new double[iVerticesCount * 2];
//	memset(in.pointlist, 0, sizeof(double)* 2 * iVerticesCount);
//	in.numberofsegments = iVerticesCount;
//	in.segmentlist = new int[iVerticesCount * 2];
//	memset(in.segmentlist, 0, sizeof(int)* 2 * iVerticesCount);
//
//	for (int i = 0; i < iVerticesCount; ++i)
//	{
//		in.pointlist[i * 2 + 0] = linearring->getX(i);
//		in.pointlist[i * 2 + 1] = linearring->getY(i);
//
//		if (i != iVerticesCount - 1)
//		{
//			in.segmentlist[i * 2 + 0] = i;
//			in.segmentlist[i * 2 + 1] = i + 1;
//		}
//		else
//		{
//			in.segmentlist[i * 2 + 0] = i;
//			in.segmentlist[i * 2 + 1] = 0;
//		}
//	}
//
//	triangulate("pzN", &in, &out, NULL);
//	int iTriangleCount = out.numberoftriangles;
//	triangles.resize(iTriangleCount);
//	for (int i = 0; i < iTriangleCount; ++i)
//	{
//		triangles[i].vertices[0].setX(in.pointlist[2 * (out.trianglelist[3 * i + 0]) + 0]);
//		triangles[i].vertices[0].setY(in.pointlist[2 * (out.trianglelist[3 * i + 0]) + 1]);
//		triangles[i].vertices[1].setX(in.pointlist[2 * (out.trianglelist[3 * i + 1]) + 0]);
//		triangles[i].vertices[1].setY(in.pointlist[2 * (out.trianglelist[3 * i + 1]) + 1]);
//		triangles[i].vertices[2].setX(in.pointlist[2 * (out.trianglelist[3 * i + 2]) + 0]);
//		triangles[i].vertices[2].setY(in.pointlist[2 * (out.trianglelist[3 * i + 2]) + 1]);
//	}
//	triiofree(&in);
//	triiofree(&out);
//}
//
//void DelaunayTriangleation(const OGRPolygon* polygon, std::vector<Triangle>& triangles)
//{
//	int iRingCount = polygon->getNumInteriorRings() + 1;
//	std::vector<int> iPointsCount; iPointsCount.reserve(iRingCount);
//	std::vector<double> points;
//	/*-----外环-----*/
//	const OGRLinearRing* ring = polygon->getExteriorRing();
//	int iExterVerticesCount = ring->getNumPoints();
//	if (ring->getX(0) == ring->getX(iExterVerticesCount - 1) && ring->getY(0) == ring->getY(iExterVerticesCount - 1))
//	{
//		iExterVerticesCount -= 1;
//	}
//	/*-----过滤掉相邻的重复点-----*/
//	iPointsCount.push_back(iExterVerticesCount);
//	for (int i = 0; i < iExterVerticesCount; ++i)
//	{
//		points.push_back(ring->getX(i));
//		points.push_back(ring->getY(i));
//	}
//	/*-----内环-----*/
//	const int iInnerRingCount = polygon->getNumInteriorRings();
//	int iHoleCount = iInnerRingCount;
//	std::vector<double> holes; holes.reserve(iHoleCount * 2);
//	for (int i = 0; i < iInnerRingCount; ++i)
//	{
//		ring = polygon->getInteriorRing(i);
//		int iInnerVerticesCount = ring->getNumPoints();
//		if (ring->getX(0) == ring->getX(iInnerVerticesCount - 1) && ring->getY(0) == ring->getY(iInnerVerticesCount - 1))
//		{
//			iInnerVerticesCount -= 1;
//		}
//		std::vector<Triangle> tris;
//		DelaunayTriangleation(ring, tris);
//		if (tris.size() < 1) continue;
//		Triangle tri = tris[0];
//		double meanx = ((tri.vertices[0].getX() + tri.vertices[1].getX()) / 2.0 + tri.vertices[2].getX()) / 2.0;
//		double meany = ((tri.vertices[0].getY() + tri.vertices[1].getY()) / 2.0 + tri.vertices[2].getY()) / 2.0;
//		holes.push_back(meanx); holes.push_back(meany);
//		iPointsCount.push_back(iInnerVerticesCount);
//		for (int j = 0; j < iInnerVerticesCount; ++j)
//		{
//			points.push_back(ring->getX(j));
//			points.push_back(ring->getY(j));
//		}
//	}
//	/*-----赋值-----*/
//	triio in, out;
//	in.pointlist = new double[points.size()];
//	memcpy(in.pointlist, points.data(), points.size() * sizeof(double));
//	in.numberofpoints = points.size() / 2;
//	in.segmentlist = new int[points.size()];
//	memset(in.segmentlist, 0, points.size() * sizeof(int));
//	in.numberofsegments = points.size() / 2;
//	in.numberofholes = iHoleCount;
//	in.holelist = new double[2 * iHoleCount];
//	memcpy(in.holelist, holes.data(), iHoleCount * 2 * sizeof(double));
//	int iCurSeg = 0;
//	for (int i = 0; i < iRingCount; ++i)
//	{
//		int iCurVerticesCount = iPointsCount[i];
//		for (int j = 0; j < iCurVerticesCount; ++j)
//		{
//			if (j != iCurVerticesCount - 1)
//			{
//				in.segmentlist[2 * iCurSeg] = iCurSeg;
//				in.segmentlist[2 * iCurSeg + 1] = iCurSeg + 1;
//			}
//			else
//			{
//				in.segmentlist[2 * iCurSeg] = iCurSeg;
//				in.segmentlist[2 * iCurSeg + 1] = iCurSeg - iCurVerticesCount + 1;
//			}
//			iCurSeg++;
//		}
//	}
//	triangulate("pzePNB", &in, &out, NULL);
//	int iTriangleCount = out.numberoftriangles;
//	Triangle tri;
//	for (int i = 0; i < iTriangleCount; ++i)
//	{
//		tri.vertices[0].setX(in.pointlist[2 * (out.trianglelist[3 * i + 0]) + 0]);
//		tri.vertices[0].setY(in.pointlist[2 * (out.trianglelist[3 * i + 0]) + 1]);
//		tri.vertices[1].setX(in.pointlist[2 * (out.trianglelist[3 * i + 1]) + 0]);
//		tri.vertices[1].setY(in.pointlist[2 * (out.trianglelist[3 * i + 1]) + 1]);
//		tri.vertices[2].setX(in.pointlist[2 * (out.trianglelist[3 * i + 2]) + 0]);
//		tri.vertices[2].setY(in.pointlist[2 * (out.trianglelist[3 * i + 2]) + 1]);
//		triangles.emplace_back(tri);
//	}
//	triiofree(&in);
//	triiofree(&out);
//}
//
//void DelaunayTriangleation(const OGRMultiPolygon* polygons, std::vector<Triangle>& triangles)
//{
//	int iPolygonCount = polygons->getNumGeometries();
//	for (int i = 0; i < iPolygonCount; ++i)
//	{
//		OGRPolygon* ppolygon = (OGRPolygon *)polygons->getGeometryRef(i);
//		DelaunayTriangleation(ppolygon, triangles);
//	}
//}
//
//void DelaunayTriangleation(const std::vector<OGRPolygon*>& polygons, std::vector<Triangle>& triangles)
//{
//	int iPolygonCount = (int)polygons.size();
//	for (int i = 0; i < iPolygonCount; ++i)
//	{
//		DelaunayTriangleation(polygons[i], triangles);
//	}
//}
//
//void DelaunayTriangleation(OGRGeometry* pGeometry, std::vector<Triangle>& triangles)
//{
//	if (pGeometry == NULL) return;
//	OGRwkbGeometryType type = pGeometry->getGeometryType();
//	if (type == wkbPolygon)
//	{
//		DelaunayTriangleation((OGRPolygon *)pGeometry, triangles);
//	}
//	else if (type == wkbPolygon25D)
//	{
//		DelaunayTriangleation((OGRPolygon *)pGeometry, triangles);
//	}
//	else if (type == wkbMultiPolygon)
//	{
//		OGRMultiPolygon* pMultiPolygon = (OGRMultiPolygon *)pGeometry;
//		int iPolygonCount = pMultiPolygon->getNumGeometries();
//		for (int i = 0; i < iPolygonCount; ++i)
//		{
//			DelaunayTriangleation((OGRPolygon *)pMultiPolygon->getGeometryRef(i), triangles);
//		}
//	}
//	else if (type == wkbLinearRing)
//	{
//		DelaunayTriangleation((OGRLinearRing *)pGeometry, triangles);
//	}
//	else if (type == wkbLineString)
//	{
//		DelaunayTriangleation((OGRLineString *)pGeometry, triangles);
//	}
//}
