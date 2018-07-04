#ifndef _GLIMAGE_20171109_H_
#define _GLIMAGE_20171109_H_

#ifdef GLIMAGE
#define GLIMAGEWAPI _declspec(dllexport)
#else
#define GLIMAGEWAPI _declspec(dllimport)
#pragma comment(lib, "GLImage.lib")
#pragma message("Automatically Linking With GLImage.dll")
#endif

#include "GL/GLConfiguration.h"
#include "GL/GLObjects.h"
#include "Geometry/GeometryInclude.h"

#include <fstream>
#include <vector>
#include <list>
#include <math.h>

using namespace std;

const int G_TileWidth = 256;

class ImageTile;
class Image;
class OrthoImage;



class GLIMAGEWAPI ImageTile
{
public:
	enum TILE_COLOR
	{
		GRAY,
		RGB
	};
	ImageTile();

	~ImageTile();

	void FreeTexData(GLTexturePool* pPool);

	void DrawTex(GLuint vaoid);

public:
	TILE_COLOR			  m_Color;
	int					  m_PyrLevel;
	OGREnvelope			  m_ImgRange;
	GLTexture*		      m_pTexture;
};

class GLIMAGEWAPI Image
{
	struct QuadNode
	{
		 QuadNode() { memset(children, 0, sizeof(void*) * 4); }
		~QuadNode(){}
		 ImageTile  tile;
		 QuadNode   *children[4];
	};
public:
	 Image();
	 virtual ~Image();

	virtual void Clear();

	bool LoadImage(const std::string& domfilename);

	bool LoadImage(const std::string& domfilename, OGREnvelope enve);

	void Search(const OGREnvelope& enve, int iLevel, std::vector<ImageTile*>& ResultItem);

	void LoadCurrentTexData(GLTexturePool* pTexturePool, bool bForce, bool* bStop);

	void LoadCurrentTexData2(GLTexturePool* pTexturePool, bool bForce, bool* bStop);

	void UpdateCurrentTexData(GLTexturePool* pTexturePool, bool bForce, bool* bStop, BYTE *data, int stCol, int stRow, int edCol, int edRow, double zoomRate, int nDataX, int nDataY);

	OGREnvelope GetGroundRange() { return m_GroundRange; };

	bool GetShowState() const;

	int  GetCols() const;

	int  GetRows() const;

	int  GetBandCount() const;

	int	 GetOverviewCount() const;

	double GetGSD() const;

	void SetOrthoID(int id);

	int  GetOrthoID() const;

	const std::string& GetDOMFileName();

	int  GetTileCount();

	void InitialQuadTree();

	vector<vector<unsigned short>>& GetColorTable();

	int GetiZoom(GLTexturePool* pTexturePool);

protected:
	void Destroy();

private:


	void CreateQuadBranch(int iDepth, const OGREnvelope& img, QuadNode** ppNode);

	void SearchQuadTree(QuadNode* pStartNode, const OGREnvelope& Env, int iLevel, std::vector<ImageTile*>& ResultItem);

	void SplitImgEnvelope(const OGREnvelope& Env, OGREnvelope* pChildEnv);

	void DestroyNode(QuadNode* pNode);

public:
	QuadNode								m_Root;
	int										m_Cols;
	int										m_Rows;
	double									m_GeoTrans[6];
	double									m_GSD;
	int										m_Pyramids;
	int                                     m_BandCount;
	int										m_Depth;
	int										m_OrthoID;
	bool									m_bShow;
	OGREnvelope								m_GroundRange;
	std::string								m_DomName;
	std::string								m_CtbName;
	int										m_TileCount;
	//当前要显示的Tile
	std::vector<ImageTile*>					m_CurTiles;
	/************************************************************************/
	/*				   影像的拉伸系数:f(x)=m_A*x+m_B                        */
	/************************************************************************/
	double m_A;
	double m_B;
	/************************************************************************/
	/*							   颜色表                                   */
	/************************************************************************/
	vector<vector<unsigned short>> m_ColorTable;
	bool m_bColorTableExist;
};

class GLIMAGEWAPI OrthoImage : public Image
{
public:
	OrthoImage();
	virtual ~OrthoImage();

	virtual void Clear();
	bool LoadDOM(const std::string& domfilename, const std::string& bonfilename);
	bool LoadDOM(const std::string& domfilename, const std::string& bonfilename, OGREnvelope enve);
	void SetOffset(double x, double y);
	void InitialTriangle();

	OGRGeometry* GetBoundry();

	int GetTriangleCount();

	int GetTriangleVBO();

	void ReadImage(CString strImagePath, int stcol, int strow, int edcol, int edrow, int memWidth, int memHeight, BYTE*&data);
public:
	string                              m_Imgname;
	string							    m_BonName;
	vector<Triangle>				    m_Triangles;
	OGRGeometry*						m_Boundry;
	double								m_OffsetX;
	double								m_OffsetY;
	vector<GLuint>						m_BonVBOID;
	vector<int>							m_BonPointCount;
	GLuint								m_TriangleVBOID;
};

#endif