#ifndef MY_FUNCTIONS
#define MY_FUNCTIONS
#pragma once

#include "MainFrm.h"
#include "ChildFrm.h"
#include "DOMRectifyDoc.h"
#include "DOMRectifyView.h"
class CMainFrame;
class CDOMRectifyDoc;
class CDOMRectifyView;
CMainFrame*      GetMainFramHand();
CChildFrame*     GetChildFramHand();
CDOMRectifyDoc*  GetDocHand();
CDOMRectifyView* GetViewHand();

/*g5 global functions*/
//获取exe路径
CString FunGetThisExePath();

//获取系统时间short sTime[8]
void	FunGetSysTimeDat(short* dTime);

CString	FunGetSysTimeStr();
//获取文件大小
long  	FunGetFileSize(CString strFilePath);
//获取文件目录
CString FunGetFileFolder(CString strFullPath);
//获取文件名
CString FunGetFileName(CString strFullPath, bool bExt);
//获取文件后缀
CString FunGetFileExt(CString strFullPath);
//分割字符串
vector<CString> FunStrTok(CString str, CString strDot);
//创建目录
void FunCreateDir4Path(CString strPath, bool bThisPathisFolder);
//检索文件
bool FunSearchFile(CString strFolderPath, CString strExt, vector<CString>& vecFilePath);

//获取日志文件路径
inline CString	FunGetSyslogPath()
{
	CString strLogPath = FunGetThisExePath();
	int np1 = strLogPath.ReverseFind('.');
	strLogPath = strLogPath.Left(np1);
	strLogPath += ".log";
	return strLogPath;
}
//输出日志信息
inline void FunOutPutLogInfo(CString strLogInfo)
{
	CString strLogFilePath = FunGetSyslogPath();
	CString strTime = FunGetSysTimeStr();
	long nFileSize = FunGetFileSize(strLogFilePath);
	if (nFileSize >= MAX_SIZE_LOGF) DeleteFile(strLogFilePath);
	FILE *pfW = fopen(strLogFilePath, "at+");
	if (pfW != NULL)
	{
		if (nFileSize > 0 && strLogInfo == "\n") fprintf(pfW, "\n");
		else if (strLogInfo != "\n") fprintf(pfW, "%s ===> %s\n", strTime, strLogInfo);
		fclose(pfW); pfW = NULL;
	}
}
/************************************************************************/
/* 影像金字塔建立与回调                                                   */
/************************************************************************/
enum MenmResampleMode
{
	RESMODE_NEARST = 0,
	RESMODE_GUASS = 1,
	RESMODE_CUBIC = 2,
	RESMODE_AVAERAGE = 3
};
typedef int(*pCallBackProFun)(double dStp, const char *pStrInfo, void *pProgressCtrl);
int CallBakPrjPro(double dPos, const char* strInfo, void *pHand);
bool BuildPryamid(CString strImgPath, bool bReBuild = false, MenmResampleMode eMode= RESMODE_NEARST, 
	int nBottomSize = 64, pCallBackProFun pFun = NULL, void *pProgressCtrl = NULL);


/************************************************************************/
/* 多线程函数                                                                     */
/************************************************************************/
void WINAPI MultiThreadBuildPyramid(LPVOID p);
void WINAPI MultiThreadCreateDockPanes(LPVOID p);
void WINAPI MultiThreadUpdateViewer(LPVOID p);
#endif
