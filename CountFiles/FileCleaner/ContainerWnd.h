
// ContainerWnd.h : interface of the CContainerWnd class
//


#pragma once

#include "MainView.h"
#include "HeadBar.h"

// CContainerWnd window

class CContainerWnd : public CWnd
{
	DECLARE_DYNAMIC(CContainerWnd)

// Construction
public:
	CContainerWnd();

// Attributes
public:

// Operations
public:
	void Relayout();

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

// Implementation
public:
	virtual ~CContainerWnd();

	CHeadBar* m_pHeadBar;
	CMainView* m_pWndView;

// Generated message map functions
protected:
	virtual void PostNcDestroy();

	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd *pOldWnd);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

