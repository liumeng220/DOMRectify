#pragma once
class CMyMemTxtFile
{
public:
	CMyMemTxtFile();
	~CMyMemTxtFile();
public:
	bool OpenFile(CString strFilePath);
	void CloseFile();
	DWORD GetFileSize() { return m_dwFileSize; }
	char* GetFileHead() { return m_lpbFile; }
private:
	HANDLE m_hFile;
	HANDLE m_hFileMap;
	char* m_lpbFile;
	DWORD m_dwFileSize;
};

inline bool ReadLine(const char* mbuf, char*line, int maxlen)
{
	int len = 0;
	while (len < maxlen - 1 && *(mbuf + len) != '\r')
	{
		*(line + len) = *(mbuf + len);
		len++;
	}
	*(line + len) = '\0';
	if (strlen(line) <= 0) return false;
	//	mbuf+=strlen(line)+2;
	return true;
}