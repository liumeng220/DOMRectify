#ifndef  _TRACE_OUTLINE_H_20170816_
#define  _TRACE_OUTLINE_H_20170816_

#include "GeometryInclude.h"

GEOMETRYAPI void DectectionBackgroundColor(const string& orthopath, int& m_BackGroundColor);

GEOMETRYAPI bool TraceOutline(const string& tifPath, const string& shpPath, BYTE* pDataPool = NULL);

GEOMETRYAPI bool CloudTraceOutline(const string& tifPath, const string& shpPath, const vector<BYTE>& targets);

GEOMETRYAPI bool MaskTraceOutline(const string& tifPath, const string& shpPath, BYTE* pDataPool = NULL);

GEOMETRYAPI void OrthoTraceOutLine(const string& orthopath, const string& shppath, int m_BackGroundColor);

#endif