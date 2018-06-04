#ifndef MY_DEFINE
#define MY_DEFINE
#pragma once

#include <afx.h>  
#include <Shlwapi.h>
#include <algorithm>
#include <vector>
using namespace std;

#define WM_OPEN_MATCH_VIEW       WM_USER+10001  //开启匹配点查看窗口
#define WM_WND_PROGRESS_SHOW     WM_USER+10002  //窗口进度条显示-隐藏
#define WM_WND_PROGRESS_POS      WM_USER+10003  //窗口进度条进度更新
#define WM_SYSTEM_TIME           WM_USER+10004  //系统时间
#define WM_CREATE_DOCKPANE       WM_USER+10005  
#define WM_UPDATE_VIEWER         WM_USER+10006
#define WM_UPDATE_STATUSINFO     WM_USER+10007
#define WM_NEW_PROJECT           WM_USER+10008
#define WM_UPDATE_MATCHVIEW      WM_USER+10009 //更新匹配窗口显示内容:整个匹配双窗口
#define WM_ADD_MATCHPOINT        WM_USER+10010 //更新匹配窗口显示内容:添加匹配点
#define WM_DELETE_MATCHPOINT     WM_USER+10011 //更新匹配窗口显示内容:删除匹配点
#define WM_UPDATE_ORTHORIMAGE    WM_USER+10012 //更新匹配窗口显示内容:纠正后图像

#define ImageGridHeight 300   //图像高度
#define ImageGridGap    5     //图像间隔
#define ImageGridCols   8     //行图像数
#define MAX_SIZE_LINE   1024
#define MAX_SIZE_PATH   1024
#define MAX_SIZE_LOGF  (1024*1024)
struct stuImgGroup
{
	int nIdx;
	CString strPanPath;
	CString strMuxPath;
	CString strFusPath;
	CString strPanShp;
	CString strMuxShp;
	CString strFusShp;
	CString strMatchFilePan;
	CString strMatchFileMux;
	CString strMatchFileFus;
	stuImgGroup()
	{
		nIdx = -1;
		strPanPath.Empty();
		strMuxPath.Empty();
		strFusPath.Empty();
		strPanShp.Empty();
		strMuxShp.Empty();
		strFusShp.Empty();
		strMatchFilePan.Empty();
		strMatchFileMux.Empty();
		strMatchFileFus.Empty();
	}
};



enum eDOMTYPE
{
	PAN = 1,
	MUX = 2,
	FUS = 3
};

#endif