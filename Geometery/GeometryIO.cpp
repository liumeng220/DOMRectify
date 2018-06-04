

#include "GeometryIO.h"

void Polygon2LineString(OGRPolygon* pPolygon, OGRMultiLineString* pLineString)
{
	if (!pPolygon || !pLineString)
	{
		return;
	}
	OGRLineString* pExterLineString = (OGRLineString *)OGRGeometryFactory::createGeometry(wkbLineString);
	OGRLinearRing* pExterLinearRing = pPolygon->getExteriorRing();
	OGRPoint pt;
	for (int i = 0; i < pExterLinearRing->getNumPoints(); ++i)
	{
		pExterLinearRing->getPoint(i, &pt);
		pExterLineString->addPoint(&pt);
	}
	pExterLinearRing->getPoint(0, &pt);
	pExterLineString->addPoint(&pt);
	pLineString->addGeometry(pExterLineString);

	for (int i = 0; i < pPolygon->getNumInteriorRings(); ++i)
	{
		pExterLinearRing = pPolygon->getInteriorRing(i);
		pExterLineString = (OGRLineString *)OGRGeometryFactory::createGeometry(wkbLineString);
		for (int j = 0; j < pExterLinearRing->getNumPoints(); ++j)
		{
			pExterLinearRing->getPoint(j, &pt);
			pExterLineString->addPoint(&pt);
		}
		pExterLinearRing->getPoint(0, &pt);
		pExterLineString->addPoint(&pt);
		pLineString->addGeometry(pExterLineString);
	}
}

void SaveSingleGeometryAsFile(const std::string &strPath, OGRGeometry* pObj)
{
	if (!pObj)
	{
		return;
	}
	GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName("ESRI Shapefile");
	if (!poDriver)
		return;
	poDriver->Delete(strPath.c_str());
	GDALDataset* poDS = poDriver->Create(strPath.c_str(), 0, 0, 0, GDT_Unknown, NULL);
	if (!poDS)
		return;
	OGRLayer* poLayer = poDS->CreateLayer("polygon");
	if (!poLayer)
		return;
	OGRFeature *poFeature = OGRFeature::CreateFeature(poLayer->GetLayerDefn());
	if (!poFeature)
		return;
	poFeature->SetGeometry(pObj);
	poLayer->CreateFeature(poFeature);
	OGRFeature::DestroyFeature(poFeature);
	GDALClose(poDS);
}

void SavePolygonsAsLineString(const string& filename, vector<OGRGeometry*>& pgeometrys)
{
	OGRMultiLineString* pMultiLineString = (OGRMultiLineString*)OGRGeometryFactory::createGeometry(wkbMultiLineString);
	for (int i = 0; i < pgeometrys.size(); ++i)
	{
		if (pgeometrys[i] == NULL)
		{
			continue;
		}
		if (pgeometrys[i]->getGeometryType() == wkbPolygon)
		{
			Polygon2LineString((OGRPolygon*)pgeometrys[i], pMultiLineString);
		}
		else if (pgeometrys[i]->getGeometryType() == wkbMultiPolygon)
		{
			int nPolygonCount = ((OGRMultiPolygon*)pgeometrys[i])->getNumGeometries();
			for (int j = 0; j < nPolygonCount; ++j)
			{
				Polygon2LineString((OGRPolygon*)((OGRMultiPolygon*)pgeometrys[i])->getGeometryRef(j), pMultiLineString);
			}
		}
	}
	SaveSingleGeometryAsFile(filename, pMultiLineString);
}

OGRGeometry* ReadSHP(const std::string& filePath)
{
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
	GDALDataset* poDS = (GDALDataset*)GDALOpenEx(filePath.c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL);
	if (!poDS)return NULL;
	int iLayerCount = poDS->GetLayerCount();
	if (iLayerCount > 1)return NULL;
	OGRLayer *poLayer = poDS->GetLayer(0);
	if (!poLayer)return NULL;
	OGRFeatureDefn* poFeatureDef = poLayer->GetLayerDefn();
	OGRFeature* poFeature = NULL;
	poLayer->ResetReading();
	int iShapeCount = poLayer->GetFeatureCount();
	OGRGeometry* pGeometry = NULL;
	if (iShapeCount == 1)
	{
		if ((poFeature = poLayer->GetNextFeature()) != NULL)
		{
			pGeometry = poFeature->GetGeometryRef();
		}
	}
	else
	{
		pGeometry = (OGRMultiPolygon*)OGRGeometryFactory::createGeometry(wkbMultiPolygon);
		for (int i = 0; i < iShapeCount; ++i)
		{
			poFeature = poLayer->GetFeature(i);
			OGRGeometry* pTempGeometry = poFeature->GetGeometryRef();
			if (pTempGeometry->getGeometryType() != wkbPolygon)
			{
				GDALClose(poDS);  return NULL;
			}
			((OGRMultiPolygon*)pGeometry)->addGeometry(pTempGeometry);
		}
	}
	GDALClose(poDS);
	return pGeometry;
}

void  SaveDXF(const string& filename, vector<OGRGeometry*>& pgeometrys)
{
	char **papszOptions = (char **)CPLCalloc(sizeof(char *), 3);
	papszOptions[0] = "HEADER=header.dxf";
	papszOptions[1] = "TRAILER=trailer.dxf";
	GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName("DXF");
	if (!poDriver)
		return;
	poDriver->Delete(filename.c_str());
	GDALDataset* poDS = poDriver->Create(filename.c_str(), 0, 0, 0, GDT_Unknown, papszOptions);
	if (!poDS)
		return;
	OGRLayer* poLayer = poDS->CreateLayer("");
	if (!poLayer)
		return;
	for (int i = 0; i < pgeometrys.size(); ++i)
	{
		if (!pgeometrys[i])
		{
			continue;
		}

		OGRFeature *poFeature = OGRFeature::CreateFeature(poLayer->GetLayerDefn());
		if (!poFeature)
			return;
		poFeature->SetGeometry(pgeometrys[i]);
		poLayer->CreateFeature(poFeature);
		OGRFeature::DestroyFeature(poFeature);
	}
	GDALClose(poDS);
}

void  ReadSHP(const string& filename, vector<OGRGeometry*>& pgeometrys)
{
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");

	GDALDataset* poDS = (GDALDataset*)GDALOpenEx(filename.c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL);
	if (!poDS)
	{
		return;
	}
	int i = 0, j = 0;
	int iLayerCount = poDS->GetLayerCount();

	for (i = 0; i < iLayerCount; ++i)
	{
		OGRLayer *poLayer = poDS->GetLayer(i);
		if (!poLayer)
		{
			continue;
		}
		int iFeatureCount = poLayer->GetFeatureCount();
		for (j = 0; j < iFeatureCount; ++j)
		{
			OGRFeature* poFeature = poLayer->GetFeature(j);
			OGRGeometry* pGeometry = poFeature->GetGeometryRef();
			if (pGeometry)
			{
				pgeometrys.push_back(pGeometry);
			}
		}
	}
	GDALClose(poDS);
}
