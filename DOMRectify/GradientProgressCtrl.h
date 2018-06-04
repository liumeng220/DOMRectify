#if !defined(AFX_ENHPROGRESSCTRL_H__12909D73_C393_11D1_9FAE_8192554015AD__INCLUDED_)
#define AFX_ENHPROGRESSCTRL_H__12909D73_C393_11D1_9FAE_8192554015AD__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// EnhProgressCtrl.h : header file
//



// GradientProgressCtrl.h : header file
//
// Written by matt weagle (matt_weagle@hotmail.com)
// Copyright (c) 1998.
//
// This code may be used in compiled form in any way you desire. This
// file may be redistributed unmodified by any means PROVIDING it is 
// not sold for profit without the authors written consent, and 
// providing that this notice and the authors name is included. If 
// the source code in  this file is used in any commercial application 
// then a simple email would be nice.
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability if it causes any damage whatsoever.
//
// Thanks to Chris Maunder (Chris.Maunder@cbr.clw.csiro.au) for the 
// foregoing disclaimer.


#include "MemDC.h"

/////////////////////////////////////////////////////////////////////////////


/**
* @brief 进度条基类
*
* 提供进度条基类接口，来反映当前算法的进度值
*/
class  CProcessBase
{
public:
	/**
	* @brief 构造函数
	*/
	CProcessBase()
	{
		m_dPosition = 0.0;
		m_iStepCount = 100;
		m_iCurStep = 0;
		m_bIsContinue = true;
	}

	/**
	* @brief 析构函数
	*/
	virtual ~CProcessBase() {}

	/**
	* @brief 设置进度信息
	* @param pszMsg         进度信息
	*/
	virtual void SetMessage(const char* pszMsg) = 0;

	/**
	* @brief 设置进度值
	* @param dPosition      进度值
	* @return 返回是否取消的状态，true为不取消，false为取消
	*/
	virtual bool SetPosition(double dPosition) = 0;

	/**
	* @brief 进度条前进一步，返回true表示继续，false表示取消
	* @return 返回是否取消的状态，true为不取消，false为取消
	*/
	virtual int StepIt() = 0;

	/**
	* @brief 设置进度个数
	* @param iStepCount     进度个数
	*/
	virtual void SetStepCount(int iStepCount)
	{
		ReSetProcess();
		m_iStepCount = iStepCount;
	}

	/**
	* @brief 获取进度信息
	* @return 返回当前进度信息
	*/
// 	string GetMessage()
// 	{
// 		return m_strMessage;
// 	}

	/**
	* @brief 获取进度值
	* @return 返回当前进度值
	*/
	double GetPosition()
	{
		return m_dPosition;
	}

	/**
	* @brief 重置进度条
	*/
	void ReSetProcess()
	{
		m_dPosition = 0.0;
		m_iStepCount = 100;
		m_iCurStep = 0;
		m_bIsContinue = true;
	}

// 	/*! 进度信息 */
// 	string m_strMessage;
	/*! 进度值 */
	double m_dPosition;
	/*! 进度个数 */
	int m_iStepCount;
	/*! 进度当前个数 */
	int m_iCurStep;
	/*! 是否取消，值为false时表示计算取消 */
	bool m_bIsContinue;
};

class CConsoleProcess : public CProcessBase
{
public:
	/**
	* @brief 构造函数
	*/
	CConsoleProcess()
	{
		m_dPosition = 0.0;
		m_iStepCount = 100;
		m_iCurStep = 0;
	};

	/**
	* @brief 析构函数
	*/
	~CConsoleProcess()
	{
		//remove(m_pszFile);  
	};

	/**
	* @brief 设置进度信息
	* @param pszMsg         进度信息
	*/
	void SetMessage(const char* pszMsg)
	{
		//m_strMessage = pszMsg;
		printf("%s\n", pszMsg);
	}

	/**
	* @brief 设置进度值
	* @param dPosition      进度值
	* @return 返回是否取消的状态，true为不取消，false为取消
	*/
	bool SetPosition(double dPosition)
	{
		m_dPosition = dPosition;
		TermProgress(m_dPosition);
		m_bIsContinue = true;
		return true;
	}

	/**
	* @brief 进度条前进一步
	* @return 返回是否取消的状态，true为不取消，false为取消
	*/
	int StepIt()
	{
		m_iCurStep++;
		m_dPosition = m_iCurStep*1.0 / m_iStepCount;

		TermProgress(m_dPosition);
		m_bIsContinue = true;
		return true;
	}

private:
	void TermProgress(double dfComplete)
	{
		static int nLastTick = -1;
		int nThisTick = (int)(dfComplete * 40.0);

		nThisTick = min(40, max(0, nThisTick));

		// Have we started a new progress run?    
		if (nThisTick < nLastTick && nLastTick >= 39)
			nLastTick = -1;

		if (nThisTick <= nLastTick)
			return;

		while (nThisTick > nLastTick)
		{
			nLastTick++;
			if (nLastTick % 4 == 0)
				fprintf(stdout, "%d", (nLastTick / 4) * 10);
			else
				fprintf(stdout, ".");
		}

		if (nThisTick == 40)
			fprintf(stdout, " - done.\n");
		else
			fflush(stdout);
	}
};


// CGradientProgressCtrl window

class CGradientProgressCtrl : public CProgressCtrl
{
// Construction
public:
	CGradientProgressCtrl();

protected:
	void DrawGradient(CPaintDC *pDC, const RECT &rectClient, const int &nMaxWidth, const BOOL &bVertical);	
	int m_nLower, m_nUpper, m_nStep, m_nCurrentPosition;
	COLORREF m_clrStart, m_clrEnd, m_clrBkGround, m_clrText;
	BOOL m_bShowPercent;

// Attributes
public:
// Attributes
	
	void SetRange(int nLower, int nUpper);
	void SetRange32(int nLower, int nUpper);
	int SetPos(int nPos);
	int SetStep(int nStep);
	int StepIt(void);
// Operations
public:
	
	// Set Functions
	void SetTextColor(COLORREF color)	{m_clrText = color;}
	void SetBkColor(COLORREF color)		{m_clrBkGround = color;}
	void SetStartColor(COLORREF color)	{m_clrStart = color;}
	void SetEndColor(COLORREF color)	{m_clrEnd = color;}

	// Show the percent caption
	void ShowPercent(BOOL bShowPercent = TRUE)	{m_bShowPercent = bShowPercent;}
	
	// Get Functions
	COLORREF GetTextColor(void)	{return m_clrText;}
	COLORREF GetBkColor(void)	{return m_clrBkGround;}
	COLORREF GetStartColor(void){return m_clrStart;}
	COLORREF GetEndColor(void)	{return m_clrEnd;}


// Overrides
	// ClaMyizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGradientProgressCtrl)
	protected:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CGradientProgressCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CGradientProgressCtrl)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

//CMFCGdalProgressCtrl
class CMFCGdalProgressCtrl :
	public CProcessBase,
	public CGradientProgressCtrl
{
public:
	CMFCGdalProgressCtrl();
	~CMFCGdalProgressCtrl();
	/**
	* @brief 设置进度信息
	* @param pszMsg         进度信息
	*/
	void SetMessage(const char* pszMsg){};

	/**
	* @brief 设置进度值
	* @param dPosition      进度值
	*/
	bool SetPosition(double dPosition);

	/**
	* @brief 进度条前进一步
	*/
	int StepIt();

public:
	
private:

};

#ifndef STD_API  
#define STD_API __stdcall  
#endif  
int STD_API updateProgress(double, const char *, void*);

#endif // !defined(AFX_ENHPROGRESSCTRL_H__12909D73_C393_11D1_9FAE_8192554015AD__INCLUDED_)