
// MainFrm.h : interface of the CMainFrame class
//

#pragma once
#include "ContainerWnd.h"
#include "BaseBar.h"
#include "PropWnd.h"

class CMainFrame : public CFrameWnd
{
public:
	CMainFrame() noexcept;
protected:
	DECLARE_DYNAMIC(CMainFrame)

// Attributes
public:
	HICON m_hIcon;

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CToolBar          m_wndToolBar;
	CBaseBar          m_wndBaseBar;
	CStatusBar        m_wndStatusBar;
	CContainerWnd*    m_pWndView;
	CPropWnd*         m_pWndProp;

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd *pOldWnd);
	DECLARE_MESSAGE_MAP()
};
