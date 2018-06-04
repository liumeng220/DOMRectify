#ifndef  _GEOMETRY_IO_H_20170816_
#define  _GEOMETRY_IO_H_20170816_

#include "GeometryInclude.h"

GEOMETRYAPI void SaveSingleGeometryAsFile(const std::string &strPath, OGRGeometry* pObj);

GEOMETRYAPI void SavePolygonsAsLineString(const string& filename, vector<OGRGeometry*>& pgeometrys);

GEOMETRYAPI OGRGeometry* ReadSHP(const std::string& filePath);

GEOMETRYAPI void ReadSHP(const string& filename, vector<OGRGeometry*>& pgeometrys);

GEOMETRYAPI void SaveDXF(const string& filename, vector<OGRGeometry*>& pgeometrys);

#endif