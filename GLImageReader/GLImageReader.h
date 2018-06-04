
#ifndef _GLIMAGEREADER_20171109_H_
#define _GLIMAGEREADER_20171109_H_

#ifdef  GLIMAGEREADER
#define GLIMAGEREADERWAPI _declspec(dllexport)
#else
#define GLIMAGEREADERWAPI _declspec(dllimport)
#pragma comment(lib, "GLImageReader.lib")
#pragma message("Automatically Linking With GLImageReader.dll")
#endif

#include "GLImage/GLImage.h"

/************************************************************************/
/*	    创建ImageReader要给定一个OGL环境,以确定朝那个环境输出纹理数据      */
/************************************************************************/
class  GLIMAGEREADERWAPI ImageReader
{
public:

	 ImageReader(const GLContext& context);

	~ImageReader();

	const  GLContext& GetContext();

	void    SetGroundRange(OGREnvelope enve);
	void    SetCurWnd(const OGREnvelope& enve, const OGREnvelope& viewport);
	void	ClearData();
	int		GetCurTileSum();

	int		GetCurTexSum();

	void    FreeBufferTile();

	void	Stop();

	bool	IsStop();

	void    AddImage(Image* pImage);
	void    AddImage(CString strImgPath, OGREnvelope enve);
	void    SetImage(Image* pImage, int idx);
	int		GetImageCount();
	double  GetCurZoomRate();
	OGREnvelope     GetGroundRange();

	Image* GetImage(int i);

	ImageTile*		GetCurTile(int idx);

private:
	void            GetCurTiles();
	void			ClearBufTiles();
	void			Start();
public:
	void			Work();
	void            UpdateCurTex(BYTE*data, int stCol, int stRow, int edCol, int edRow, double zoomRate);

private:
	std::vector<ImageTile*>		   m_CurTiles;
	std::vector<ImageTile*>        m_BufTiles;
	OGREnvelope					   m_CurGround;
	OGREnvelope					   m_CurViewPort;
	OGREnvelope                    m_GroundRange;
	HANDLE						   m_hThread;
	DWORD						   m_dwThdId;
	bool 						   m_bStop;
	GLContext                      m_GLContext;
	/************************************************************************/
	/*					ImageReader绑定的Image(可以是多张)                   */
	/************************************************************************/
	vector<Image*> m_Images;
	vector<CString> m_vecImgPath;
	/************************************************************************/
	/*						两类纹理池（彩色和灰度）                         */
	/************************************************************************/
	GLLUMTexturePool*  m_pLUMPool;
	GLRGBTexturePool*  m_pRGBPool;
	/************************************************************************/
	/* 当前读取图像缩放比例尺，image-client                                  */
	/************************************************************************/
	double m_dZoomRate;
};

#endif