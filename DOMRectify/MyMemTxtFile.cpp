#include "StdAfx.h"
#include "MyMemTxtFile.h"


CMyMemTxtFile::CMyMemTxtFile()
{

}
CMyMemTxtFile::~CMyMemTxtFile()
{
	CloseFile();
}
bool CMyMemTxtFile::OpenFile(CString strFilePath)
{
	m_hFile = INVALID_HANDLE_VALUE;
	m_hFileMap = NULL;
	m_lpbFile = NULL;
	m_dwFileSize = 0;
	if (!PathFileExists(strFilePath)) return false;
	m_hFile = CreateFile(strFilePath, //路径
		GENERIC_READ,//读
		FILE_SHARE_READ, //共享只读
		NULL,//安全特性
		OPEN_EXISTING,//文件必须存在
		FILE_FLAG_SEQUENTIAL_SCAN,//针对连续访问对文件缓冲进行优化
		NULL);  //如果不为0，则指定一个文件句柄
	if (INVALID_HANDLE_VALUE == m_hFile) return false;
	m_dwFileSize = ::GetFileSize(m_hFile, NULL);
	m_hFileMap = CreateFileMapping(m_hFile,
		NULL,//安全对象
		PAGE_READONLY,//只读方式打开映射
		0,
		0,//用磁盘文件实际长度
		NULL);//指定文件映射对象的名字
	if (NULL == m_hFileMap) return false;
	m_lpbFile = (char*)MapViewOfFile(
		m_hFileMap,
		FILE_MAP_READ,
		0,
		0,//映射整个文件映射对象
		0);//0表示允许Windows寻找地址
	if (NULL == m_lpbFile) return false;
	return true;
}
void CMyMemTxtFile::CloseFile()
{
	if (m_lpbFile != NULL) UnmapViewOfFile(m_lpbFile);
	if (m_hFileMap != NULL) CloseHandle(m_hFileMap);
	if (m_hFile != INVALID_HANDLE_VALUE) CloseHandle(m_hFile);
	m_hFile = INVALID_HANDLE_VALUE;
	m_hFileMap = NULL;
	m_lpbFile = NULL;
	m_dwFileSize = 0;
}
