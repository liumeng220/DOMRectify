

#include <vector>
#include <string>
#include <fstream>
#include <Windows.h>

#include "GDAL/ogr_core.h"
#include "GDAL/gdal.h"
#include "GDAL/gdal_priv.h"
#include "GDAL/ogr_geometry.h"
#include "GDAL/ogr_feature.h"
#include "GDAL/ogrsf_frmts.h"
#include "GDAL/cpl_string.h"
#include "GDAL/ogr_core.h"

using namespace std;

#pragma comment(lib, "gdal_i.lib")

#ifdef GEOMETRY
#define GEOMETRYAPI _declspec(dllexport)
#else 
#define GEOMETRYAPI _declspec(dllimport)
#pragma comment(lib, "Geometry.lib")
#pragma message("Linking With Geometry.dll.")
#endif

