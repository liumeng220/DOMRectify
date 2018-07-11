#include "StdAfx.h"
#include "DOMRectifyPrj.h"
//#include "pugiconfig.hpp"
#include "pugixml.hpp"
#include "MyFunctions.h"
#include "MyMemTxtFile.h"

static bool XmlNodeJudge(pugi::xml_node& Node)
{
	if (Node == NULL)
	{
		FunOutPutLogInfo("工程文件解析错误！");
		return false;
	}
	return true;
}
CDOMRectifyPrj::CDOMRectifyPrj(void)
{
	ClearPrj();
}


CDOMRectifyPrj::~CDOMRectifyPrj(void)
{
	ClearPrj();
}

void CDOMRectifyPrj::ClearPrj()
{
	m_strPrjPath.Empty();
	m_nGroupNum = 0;
	m_nDomNum = 0;
	m_nRefNum = 0;
	vector<CString>().swap(m_vecRefPath);
	vector<stuImgGroup>().swap(m_vecDomGroup);
	ClearPoint();
	m_strCurDomPath.Empty();
	m_strCurRecDomPath.Empty();
}

void CDOMRectifyPrj::NewPrj(CString strPrjPath, vector<CString> vecRefPath, vector<stuImgGroup>& vecDomGroup, int nDomNum)
{
	ClearPrj();
	m_strPrjPath = strPrjPath;
	m_vecRefPath.insert(m_vecRefPath.begin(), vecRefPath.begin(), vecRefPath.end());
	m_vecDomGroup.insert(m_vecDomGroup.begin(), vecDomGroup.begin(), vecDomGroup.end());
	m_nGroupNum = vecDomGroup.size();
	m_nDomNum = nDomNum;
	m_nRefNum = vecRefPath.size();

	CString strMatchFolder = FunGetFileFolder(m_strPrjPath) + "\\ImageMatch\\";
	FunCreateDir4Path(strMatchFolder,true);
	for (int i = 0; i<m_nGroupNum; i++)
	{
		m_vecDomGroup[i].strMatchFilePan = strMatchFolder + FunGetFileName(m_vecDomGroup[i].strPanPath, false) + "_match.txt";
		m_vecDomGroup[i].strMatchFileMux = strMatchFolder + FunGetFileName(m_vecDomGroup[i].strMuxPath, false) + "_match.txt";
		m_vecDomGroup[i].strMatchFileFus = strMatchFolder + FunGetFileName(m_vecDomGroup[i].strFusPath, false) + "_match.txt";
	}
}

bool CDOMRectifyPrj::SavePrj(CString strSavePath)
{
	if (m_nGroupNum == 0 || m_vecRefPath.size() == 0)
	{
		FunOutPutLogInfo("工程影像信息有误，保存失败！==path=" + strSavePath);
		return false;
	}
	pugi::xml_document PrjXmlDoc; char strValue[MAX_SIZE_LINE];
	pugi::xml_node decl = PrjXmlDoc.prepend_child(pugi::node_declaration);
	decl.append_attribute("version") = "1.0";
	decl.append_attribute("encoding") = "gb2312";
	pugi::xml_node RootNode = PrjXmlDoc.append_child("DOMRectify_Project");
	pugi::xml_node Ref = RootNode.append_child("Reference_Image");
	pugi::xml_node Node;
	for (int i = 0; i<m_vecRefPath.size(); i++)
	{
		sprintf(strValue, "Ref_%d", i);
		Node = Ref.append_child(strValue);
		Node.append_child(pugi::node_pcdata).set_value(m_vecRefPath[i]);
	}

	pugi::xml_node DOM = RootNode.append_child("DOM_Image");
    Node = DOM.append_child("Group_Number");
	sprintf(strValue, "%d", m_nGroupNum);
	Node.append_child(pugi::node_pcdata).set_value(strValue);
	Node = DOM.append_child("DOM_Number");
	sprintf(strValue, "%d", m_nDomNum);
	Node.append_child(pugi::node_pcdata).set_value(strValue);
	Node = DOM.append_child("Image_List");
	pugi::xml_node Group, child;
	for (int i = 0; i<m_nGroupNum; i++)
	{
		Group = Node.append_child("Group");
		child = Group.append_child("ID");
		sprintf(strValue, "%d", i);
		child.append_child(pugi::node_pcdata).set_value(strValue);

		child = Group.append_child("Pan");
		child.append_child(pugi::node_pcdata).set_value(m_vecDomGroup[i].strPanPath);

		child = Group.append_child("Mux");
		child.append_child(pugi::node_pcdata).set_value(m_vecDomGroup[i].strMuxPath);
		
		child = Group.append_child("Fus");
		child.append_child(pugi::node_pcdata).set_value(m_vecDomGroup[i].strFusPath);

		child = Group.append_child("MatchFile");
		pugi::xml_node child2 = child.append_child("PAN");
		child2.append_child(pugi::node_pcdata).set_value(m_vecDomGroup[i].strMatchFilePan);
		child2 = child.append_child("MUX");
		child2.append_child(pugi::node_pcdata).set_value(m_vecDomGroup[i].strMatchFileMux);
		child2 = child.append_child("FUS");
		child2.append_child(pugi::node_pcdata).set_value(m_vecDomGroup[i].strMatchFileFus);
	}
	bool ret = PrjXmlDoc.save_file(strSavePath);
	if (ret)
	{
		FunOutPutLogInfo("工程保存成功==path=" + strSavePath);
	}
	else
	{
		FunOutPutLogInfo("工程保存失败==path=" + strSavePath);
	}
	return ret;
}

bool CDOMRectifyPrj::OpenPrj(CString strPrjPath)
{
	pugi::xml_document PrjXmlDoc;
	pugi::xml_parse_result ret = PrjXmlDoc.load_file(strPrjPath, pugi::parse_full, pugi::encoding_utf8);
	if (ret.status != pugi::status_ok)
	{
		FunOutPutLogInfo("工程打开失败==path=" + strPrjPath);
		return false;
	}
	pugi::xml_node NodeRoot = PrjXmlDoc.child("DOMRectify_Project");
	if(!XmlNodeJudge(NodeRoot)) return false;
	pugi::xml_node Ref = NodeRoot.child("Reference_Image");
	if(!XmlNodeJudge(Ref)) return false;
	for (pugi::xml_node Node1 = Ref.first_child(); Node1 != NULL; Node1 = Node1.next_sibling())
	{
		CString NodeLabel = Node1.name();
		if (NodeLabel.Left(3) == "Ref")
		{
			m_vecRefPath.push_back(Node1.text().as_string());
		}
	}
	pugi::xml_node DOM = NodeRoot.child("DOM_Image");
	if (!XmlNodeJudge(DOM)) return false;
	for (pugi::xml_node Node1 = DOM.first_child(); Node1 != NULL; Node1 = Node1.next_sibling())
	{
		CString NodeLabel = Node1.name();
		if (NodeLabel == "Group_Number")
		{
			m_nGroupNum = Node1.text().as_int();
		}
		else if (NodeLabel == "DOM_Number")
		{
			m_nDomNum = Node1.text().as_int();
		}
		else if (NodeLabel == "Image_List")
		{
			for (pugi::xml_node Node2 = Node1.first_child(); Node2 != NULL; Node2 = Node2.next_sibling())
			{
				CString NodeLabel2 = Node2.name();
				if (NodeLabel2 == "Group")
				{
					stuImgGroup imgGroup; bool bGroup = false;
					for (pugi::xml_node Node3 = Node2.first_child(); Node3 != NULL; Node3 = Node3.next_sibling())
					{
						CString NodeLabel3 = Node3.name();
						if (NodeLabel3 == "ID")
						{
							imgGroup.nIdx = Node3.text().as_int();
							bGroup = true;
						}
						else if (NodeLabel3 == "Pan")
						{
							imgGroup.strPanPath = Node3.text().as_string();
							bGroup = true;
						}
						else if (NodeLabel3 == "Mux")
						{
							imgGroup.strMuxPath = Node3.text().as_string();
							bGroup = true;
						}
						else if (NodeLabel3 == "Fus")
						{
							imgGroup.strFusPath = Node3.text().as_string();
							bGroup = true;
						}
						else if (NodeLabel3 == "MatchFile")
						{
							for (pugi::xml_node Node4 = Node3.first_child(); Node4 != NULL; Node4 = Node4.next_sibling())
							{
								CString NodeLabel4 = Node4.name();
								if (NodeLabel4 == "PAN")
								{
									imgGroup.strMatchFilePan = Node4.text().as_string();
								}
								else if (NodeLabel4 == "MUX")
								{
									imgGroup.strMatchFileMux = Node4.text().as_string();
								}
								else if (NodeLabel4 == "FUS")
								{
									imgGroup.strMatchFileFus = Node4.text().as_string();
								}
							}
						}
					}
					if (bGroup)
					{
						m_vecDomGroup.push_back(imgGroup);
					}
				}

			}
		}
	}
	if (m_nGroupNum != m_vecDomGroup.size())
	{
		FunOutPutLogInfo("工程内影像数与影像列表冲突，工程打开失败==path=" + strPrjPath);
		ClearPrj();
		return false;
	}
	m_nRefNum = m_vecRefPath.size();
	m_strPrjPath = strPrjPath;
	FunOutPutLogInfo("工程打开成功==path=" + m_strPrjPath);
	return true;
}

bool CDOMRectifyPrj::LoadPoint(int nIdx, ImageReader * pReader, eDOMTYPE eType)
{
	ClearPoint();
	if (nIdx<0 || nIdx>m_nGroupNum) return false;
	m_eDomTyep = eType;
	switch (m_eDomTyep)
	{
	case PAN:
		m_strMatchFile = m_vecDomGroup[nIdx].strMatchFilePan;
		m_strCurDomPath = m_vecDomGroup[nIdx].strPanPath;
		break;
	case MUX:
		if (!PathFileExists(m_vecDomGroup[nIdx].strMatchFileMux))
			CopyFile(m_vecDomGroup[nIdx].strMatchFilePan, m_vecDomGroup[nIdx].strMatchFileMux, FALSE);
		m_strMatchFile = m_vecDomGroup[nIdx].strMatchFileMux;
		m_strCurDomPath = m_vecDomGroup[nIdx].strMuxPath;
		break;
	case FUS:
		if (!PathFileExists(m_vecDomGroup[nIdx].strMatchFileFus))
			CopyFile(m_vecDomGroup[nIdx].strMatchFilePan, m_vecDomGroup[nIdx].strMatchFileFus, FALSE);
		m_strMatchFile = m_vecDomGroup[nIdx].strMatchFileFus;
		m_strCurDomPath = m_vecDomGroup[nIdx].strFusPath;
		break;
	default:
		break;
	}
	m_strCurRecDomPath = FunGetFileFolder(m_strPrjPath) + "\\Rectify\\" + FunGetFileName(m_strCurDomPath, false) + "_rec.tif";
	FunCreateDir4Path(m_strCurRecDomPath,false);

	CMyMemTxtFile txt;
	if (!txt.OpenFile(m_strMatchFile)) return false;
	char line[MAX_SIZE_LINE]; memset(line, 0, MAX_SIZE_LINE);
	const char* tmp = txt.GetFileHead();
	if (!ReadLine(tmp, line, MAX_SIZE_LINE)) return false;
	tmp += strlen(line) + 2;
	if (!ReadLine(tmp, line, MAX_SIZE_LINE)) return false;
	tmp += strlen(line) + 2;
	if (!ReadLine(tmp, line, MAX_SIZE_LINE)) return false;
	tmp += strlen(line) + 2;
	m_nPtNum = atoi(line);
	m_vecPtRef.resize(m_nPtNum);
	m_vecPtDom.resize(m_nPtNum);
	//解析匹配点
	int nMatchPtIndex = 0, nRefIdx;
	double lfDomX = 0.0, lfDomY = 0.0, lfRefX = 0.0, lfRefY = 0.0;

	OrthoImage *DomImg = new OrthoImage;
	DomImg->LoadDOM(m_strCurDomPath.GetBuffer(), "");

	OGREnvelope enveDom = DomImg->GetGroundRange();
	double GsdDom = DomImg->GetGSD();
//	DomImg->Clear(); if (DomImg) delete DomImg;

	for (int i = 0; i < m_nPtNum; i++)
	{
		if (!ReadLine(tmp, line, MAX_SIZE_LINE)) continue;
		tmp += strlen(line) + 2;
		sscanf(line, "%d %lf %lf %lf %lf %d", &nMatchPtIndex, &lfDomX, &lfDomY, &lfRefX, &lfRefY, &nRefIdx);
	//ref point	
		m_vecPtRef[i].nRefIdx = nRefIdx;
		m_vecPtRef[i].nPtIdx = nMatchPtIndex;
// 		OrthoImage *RefImg = new OrthoImage;
// 		RefImg->LoadDOM(m_vecRefPath[m_vecPtRef[i].nRefIdx].GetBuffer(), "");
// 		double GsdRef = RefImg->GetGSD();
// 		RefImg->SetOffset(enveDom.MinX, enveDom.MinY);
// 		OGREnvelope enveRef = RefImg->GetGroundRange();
		OGREnvelope enveRef = pReader->GetImage(m_vecPtRef[i].nRefIdx)->GetGroundRange();
		double GsdRef = pReader->GetImage(m_vecPtRef[i].nRefIdx)->GetGSD();
//		if (RefImg) RefImg->Clear(); delete RefImg;
		m_vecPtRef[i].pt.setX(lfRefX * GsdRef + enveRef.MinX);
		m_vecPtRef[i].pt.setY(lfRefY * GsdRef + enveRef.MinY);
   //pan point
		m_vecPtDom[i].nRefIdx = 0;
		m_vecPtDom[i].nPtIdx = nMatchPtIndex;
		m_vecPtDom[i].pt.setX(lfDomX* GsdDom + 0 /*enveRef.MinX*/);
		m_vecPtDom[i].pt.setY(lfDomY* GsdDom + 0 /*enveRef.MinY*/);
	}
	txt.CloseFile();
	return true;
}

void CDOMRectifyPrj::AddPoint(OGRPoint ptDom, OGRPoint ptRef, int nRefIdx)
{
	stuMatchPoint stuPtDom, stuPtRef;
	if (m_nPtNum > 0)
		stuPtDom.nPtIdx = m_vecPtDom[m_nPtNum - 1].nPtIdx + 1;
	else
		stuPtDom.nPtIdx = 9000000;
	stuPtDom.nRefIdx = nRefIdx;
	stuPtDom.pt = ptDom;
	stuPtRef = stuPtDom;
	stuPtRef.pt = ptRef;

	m_vecPtDom.push_back(stuPtDom);
	m_vecPtRef.push_back(stuPtRef);
	m_nPtNum++;
	m_nCurPtNum++;
}

// void CDOMRectifyPrj::SavePoint(ImageReader * pReader)
// {
// 	/*FILE *pf = fopen(m_strMatchFile, "w");
// 	if (!pf) return;
// 
// 	double lfDomX = 0.0, lfDomY = 0.0, lfRefX = 0.0, lfRefY = 0.0;
// 
// 	fprintf(pf, "%s\n", "image1");
// 	fprintf(pf, "%s\n", "image2");
// 	fprintf(pf, "%d\n", m_nPtNum);
// 	double GsdDom = pReader->GetImage(0)->GetGSD();
// 	OGREnvelope enveDom = pReader->GetImage(0)->GetGroundRange();
// 	for (int i = 0; i < m_nPtNum; i++)
// 	{
// 		double GsdRef = pReader->GetImage(m_vecPtRef[i].nRefIdx + 1)->GetGSD();
// 		OGREnvelope enveRef = pReader->GetImage(m_vecPtRef[i].nRefIdx + 1)->GetGroundRange();
// 
// 		lfDomX = (m_vecPtDom[i].pt.getX() - enveDom.MinX) / GsdDom;
// 		lfDomY = (m_vecPtDom[i].pt.getY() - enveDom.MinY) / GsdDom;
// 		lfRefX = (m_vecPtRef[i].pt.getX() - enveRef.MinX) / GsdRef;
// 		lfRefY = (m_vecPtRef[i].pt.getY() - enveRef.MinY) / GsdRef;
// 
// 		fprintf(pf, "%-8d %-8.3lf %-8.3lf %-8.3lf %-8.3lf  %-4d\n",
// 			m_vecPtDom[i].nPtIdx, lfDomX, lfDomY, lfRefX, lfRefY, m_vecPtRef[i].nRefIdx);
// 	}
// 	fclose(pf); pf = NULL;*/
// }

void CDOMRectifyPrj::SavePoint(ImageReader * pReader)
{
	FILE *pf = fopen(m_strMatchFile, "w");
	if (!pf) return;

	double lfDomX = 0.0, lfDomY = 0.0, lfRefX = 0.0, lfRefY = 0.0;

	fprintf(pf, "%s\n", m_strCurDomPath);
	fprintf(pf, "%s\n", "Ref_Image");
	fprintf(pf, "%d\n", m_nPtNum);
	OrthoImage *DomImg = new OrthoImage;
	DomImg->LoadDOM(m_strCurDomPath.GetBuffer(), "");
	double GsdDom = DomImg->GetGSD();
	OGREnvelope enveDom = DomImg->GetGroundRange();
	DomImg->SetOffset(enveDom.MinX, enveDom.MinY);
	OGREnvelope enveDom2 = DomImg->GetGroundRange();
//	DomImg->Clear(); if (DomImg) delete DomImg;
	for (int i = 0; i < m_nPtNum; i++)
	{
// 		OrthoImage *RefImg = new OrthoImage;
// 		RefImg->LoadDOM(m_vecRefPath[m_vecPtRef[i].nRefIdx].GetBuffer(), "");
// 		RefImg->SetOffset(enveDom.MinX, enveDom.MinY);
		
		OGREnvelope enveRef = pReader->GetImage(m_vecPtRef[i].nRefIdx)->GetGroundRange();
		double GsdRef = pReader->GetImage(m_vecPtRef[i].nRefIdx)->GetGSD();
		//		RefImg->Clear(); if (RefImg) delete RefImg;
		lfDomX = (m_vecPtDom[i].pt.getX() - enveDom2.MinX) / GsdDom;
		lfDomY = (m_vecPtDom[i].pt.getY() - enveDom2.MinY) / GsdDom;
		lfRefX = (m_vecPtRef[i].pt.getX() - enveRef.MinX) / GsdRef;
		lfRefY = (m_vecPtRef[i].pt.getY() - enveRef.MinY) / GsdRef;

		fprintf(pf, "%-8d %-8.3lf %-8.3lf %-8.3lf %-8.3lf  %-4d\n",
			m_vecPtDom[i].nPtIdx, lfDomX, lfDomY, lfRefX, lfRefY, m_vecPtRef[i].nRefIdx);
	}
	fclose(pf); pf = NULL;
	DomImg->Clear();
	//if (DomImg) delete DomImg;
}

void CDOMRectifyPrj::SelPoint(OGREnvelope enve)
{
	vector<int>().swap(m_vecSelectedPointIdx);
	for (int i = 0; i<m_nPtNum; i++)
	{
		double x = m_vecPtDom[i].pt.getX();
		double y = m_vecPtDom[i].pt.getY();
		if ((x - enve.MinX)*(x - enve.MaxX) <= 0 &&
			(y - enve.MinY)*(y - enve.MaxY) <= 0)
		{
			m_vecSelectedPointIdx.push_back(i);
		}
	}
}

void CDOMRectifyPrj::DelPoint()
{
	for (int i = 0; i < m_vecSelectedPointIdx.size(); i++)
	{
		m_vecPtDom[m_vecSelectedPointIdx[i]].nPtValid = -1;
		m_vecPtRef[m_vecSelectedPointIdx[i]].nPtValid = -1;
	}
	sort(m_vecPtDom.begin(), m_vecPtDom.end());
	sort(m_vecPtRef.begin(), m_vecPtRef.end());
	m_vecPtDom.erase(m_vecPtDom.begin(), m_vecPtDom.begin() + m_vecSelectedPointIdx.size());
	m_vecPtRef.erase(m_vecPtRef.begin(), m_vecPtRef.begin() + m_vecSelectedPointIdx.size());
	m_nPtNum -= m_vecSelectedPointIdx.size();
	m_nCurPtNum -= m_vecSelectedPointIdx.size();
	vector<int>().swap(m_vecSelectedPointIdx);
}

void CDOMRectifyPrj::ClearPoint()
{
	vector<stuMatchPoint>().swap(m_vecPtDom);
	vector<stuMatchPoint>().swap(m_vecPtRef);
	m_nPtNum = 0;
	m_nCurPtNum = 0;
	m_eDomTyep = PAN;
	m_strMatchFile.Empty();
}

CString CDOMRectifyPrj::GetPrjPath()
{
	return m_strPrjPath;
}
CString CDOMRectifyPrj::GetRefPath(int idx)
{
	if (idx >= m_vecRefPath.size())
	{
		return 0;
	}
	else
	{
		return m_vecRefPath[idx];
	}
}
CString CDOMRectifyPrj::GetPanPath(int idx)
{
	if(idx>=m_vecDomGroup.size())
	{
		return 0;
	}
	else
	{
		return m_vecDomGroup[idx].strPanPath;
	}
}
CString CDOMRectifyPrj::GetMuxPath(int idx)
{
	if(idx>= m_vecDomGroup.size())
	{
		return 0;
	}
	else
	{
		return m_vecDomGroup[idx].strMuxPath;
	}
}
CString CDOMRectifyPrj::GetFusPath(int idx)
{
	if(idx>= m_vecDomGroup.size())
	{
		return 0;
	}
	else
	{
		return m_vecDomGroup[idx].strFusPath;
	}
}
CString CDOMRectifyPrj::GetMatchFilePath(int idx)
{
	if(idx>=m_vecDomGroup.size())
	{
		return 0;
	}
	else
	{
		switch (m_eDomTyep)
		{
		case PAN:
			return m_vecDomGroup[idx].strMatchFilePan;
			break;
		case MUX:
			return m_vecDomGroup[idx].strMatchFileMux;
			break;
		case FUS:
			return m_vecDomGroup[idx].strMatchFileFus;
			break;
		default:
			break;
		}
	}
}

CString CDOMRectifyPrj::GetCurDomPath()
{
	return m_strCurDomPath;
}

CString CDOMRectifyPrj::GetCurRecDomPath()
{
	return m_strCurRecDomPath;
}

CString CDOMRectifyPrj::GetCurMatchPath()
{
	return m_strMatchFile;
}

stuImgGroup CDOMRectifyPrj::GetDomGroup(int idx)
{
	if (idx >= m_vecDomGroup.size())
	{
		return stuImgGroup();
	}
	else
	{
		return m_vecDomGroup[idx];
	}
}


vector<stuImgGroup> CDOMRectifyPrj::GetDomGroup()
{
	return m_vecDomGroup;
}

vector<CString> CDOMRectifyPrj::GetRefPath()
{
	return m_vecRefPath;
}

// vector<CString> CDOMRectifyPrj::GetPanPath()
// {
// 	return m_vecPanPath;
// }
// vector<CString> CDOMRectifyPrj::GetMuxPath()
// {
// 	return m_vecMuxPath;
// }
// vector<CString> CDOMRectifyPrj::GetFusPath()
// {
// 	return m_vecFusPath;
// }
// vector<CString> CDOMRectifyPrj::GetMatchFilePath()
// {
// 	return m_vecMatchFilePath;
// }
vector<stuMatchPoint> CDOMRectifyPrj::GetRefPoints()
{
	return m_vecPtRef;
}
vector<stuMatchPoint> CDOMRectifyPrj::GetDomPoints()
{
	return m_vecPtDom;
}
vector<stuMatchPoint> CDOMRectifyPrj::GetRefPoints(int nRefIdx)
{
	vector<stuMatchPoint> vecPtRef;
	for (int i = 0; i < m_nPtNum; i++)
	{
		if (m_vecPtRef[i].nRefIdx == nRefIdx)
			vecPtRef.push_back(m_vecPtRef[i]);
	}
	m_nCurPtNum = vecPtRef.size();
	return vecPtRef;
	//return m_vecPtRef;
}
vector<stuMatchPoint> CDOMRectifyPrj::GetDomPoints(int nRefIdx)
{
	vector<stuMatchPoint> vecPtDom;
	for (int i = 0; i<m_nPtNum; i++)
	{
		if (m_vecPtRef[i].nRefIdx == nRefIdx)
			vecPtDom.push_back(m_vecPtDom[i]);
	}
	m_nCurPtNum = vecPtDom.size();
	return vecPtDom;
}

vector<stuMatchPoint> CDOMRectifyPrj::GetSelRefPoints()
{
	vector<stuMatchPoint> vecSelRefPoint(m_vecSelectedPointIdx.size());
	for (int i = 0; i<m_vecSelectedPointIdx.size(); i++)
	{
		vecSelRefPoint[i] = m_vecPtRef[m_vecSelectedPointIdx[i]];
	}
	return vecSelRefPoint;
}

vector<stuMatchPoint> CDOMRectifyPrj::GetSelDomPoints()
{
	vector<stuMatchPoint> vecSelDomPoint(m_vecSelectedPointIdx.size());
	for (int i = 0; i < m_vecSelectedPointIdx.size(); i++)
	{
		vecSelDomPoint[i] = m_vecPtDom[m_vecSelectedPointIdx[i]];
	}
	return vecSelDomPoint;
}

int CDOMRectifyPrj::GetGroupNum()
{
	return m_nGroupNum;
}

int CDOMRectifyPrj::GetDomNum()
{
	return m_nDomNum;
}


int CDOMRectifyPrj::GetRefNum()
{
	return m_nRefNum;
}

int CDOMRectifyPrj::GetAllPointNum()
{
	return m_nPtNum;
}

int CDOMRectifyPrj::GetCurPointNum()
{
	return m_nCurPtNum;
}

int CDOMRectifyPrj::GetSelPointNum()
{
	return m_vecSelectedPointIdx.size();
}
